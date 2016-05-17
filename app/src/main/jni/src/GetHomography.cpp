//
// Created by linqi on 16-4-5.
//

#include "include/GetHomography.h"



GetHomography::~GetHomography()
{
    pthread_mutex_destroy( &g_mutex );
}

GetHomography::GetHomography(Mat& srcImage,Mat& dstImage)
{
    pthread_mutex_init( &g_mutex, NULL );
    if(!srcImage.empty() || !dstImage.empty())
        assert("Error in Homography: input image can not be empty!");
    assert(srcImage.channels() == 1);
    assert(dstImage.channels() == 1);

    if(srcImage.size().width*srcImage.size().height < 5500000)
        m_patch.createPatch(15,2,16,21);
    else
        //m_patch.createPatch(15,2,16,31);
        m_patch.createPatch(20,3,16,31);

    m_queryPoints.clear();
    m_trainPoints.clear();

    trainPattern = srcImage;
    queryPattern = dstImage;

    //scaleSize 是在pyrDown里缩放的大小，这个不能随便改动
    HomoMethod = RANSAC;
    scaleSize = 1;
    work_begin = 0;
    work_end = 0;
}


//开始计时
void GetHomography::workBegin()
{
    work_begin = getTickCount();
}

//结束计时
void GetHomography::workEnd()
{
    work_end = getTickCount() - work_begin;
    double gTime = work_end /((double)getTickFrequency() )* 1000.0;
    LOGE("TIME = %lf ms \n",gTime);
}

void GetHomography::savePoints(vector<Point2f>& _queryPoints,vector<Point2f>& _trainPoints)
{
    pthread_mutex_lock( &g_mutex );
    vector<Point2f>::iterator query,train;
    for(query = _queryPoints.begin();query != _queryPoints.end();query++)
    {
/*        query->x = query->x*scaleSize;
        query->y = query->y*scaleSize;*/
        m_queryPoints.push_back(*query);
    }
    for(train = _trainPoints.begin();train != _trainPoints.end();train++)
    {
/*        train->x = train->x*scaleSize;
        train->y = train->y*scaleSize;*/
        m_trainPoints.push_back(*train);
    }
    pthread_mutex_unlock( &g_mutex );
    return ;
}

bool GetHomography::detectPoints(const int _patchSize,const int keyPointThread,const int robEdgeThreshold)
{
    bool ret = true;
    int patchSzie = _patchSize;
    //robEdgeThread mutiply 2 side of each patch and the scale size of pyramid
    int roiPWidth = patchSzie - robEdgeThreshold*2*4;

    for(size_t patch_index = 0;patch_index < 4;patch_index++)
    {
        int iKeyPoints = 0;//the sum of min keyPoints between query and train
        int cols_index = 0;//cols index
        int rows_index = 0;//rows index

        switch(patch_index)
        {
            case 0://left top corner
            {
                int timeCount = 0;
                //LOGE("CORNER 0");
                while(iKeyPoints <= keyPointThread)
                {
                    //LOGE("right top corner!");
                    timeCount++;
                    //int moveRange = cols_index*patchSzie;
                    int moveRange = cols_index*roiPWidth;
                    if( moveRange > queryPattern.cols/2/* - patchSzie */)//row changed
                    {
                        rows_index++;
                        cols_index = 0;
                        moveRange = 0;
                        if(roiPWidth*rows_index > queryPattern.rows - patchSzie ||
                           roiPWidth*rows_index > trainPattern.rows - patchSzie )
                        {
                            LOGE("points in left top corner are too little!\n");
                            LOGE("left top corner KeyPoints = %d",iKeyPoints);
                            ret = false;
                            return ret;
                            break;
                        }
                        //                    assert(patchSzie*rows_index < queryPattern.rows - patchSzie );
                        //                    assert(patchSzie*rows_index < trainPattern.rows - patchSzie );
                    }
                    Point2f Origin(roiPWidth*cols_index,roiPWidth*rows_index);
                    Rect ROI(roiPWidth*cols_index,roiPWidth*rows_index,patchSzie,patchSzie);
                    Mat roiQueryArea(queryPattern,ROI);
                    Mat roiTrainArea(trainPattern,ROI);

                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiTrainArea,roiTrainArea);
                    pyrDown(roiTrainArea,roiTrainArea);

                    m_patch.setPatch(Origin,roiQueryArea,roiTrainArea);

                    vector <Point2f> queryPoints,trainPoints;
                    m_patch.getPointsMatchers(queryPoints,trainPoints);
                    savePoints(queryPoints,trainPoints);

                    iKeyPoints += queryPoints.size();
                    cols_index++;
                }
                LOGE("left top corner timeCount = %d",timeCount);
            }
                break;
            case 1://right top corner
            {
                //LOGE("CORNER 1");
                int timeCount = 0;
                while(iKeyPoints <= keyPointThread)
                {
                    //LOGE("right top corner!");
                    timeCount++;
                    int moveRange = rows_index*roiPWidth;
                    if( moveRange > queryPattern.rows/2/* - patchSzie */)//cols changed
                    {
                        cols_index++;
                        rows_index = 0;
                        moveRange = 0;
                        if(roiPWidth*cols_index > queryPattern.cols - patchSzie ||
                           roiPWidth*cols_index > trainPattern.cols - patchSzie )
                        {
                            LOGE("points in right top corner are too little!\n");
                            LOGE("right top corner KeyPoints = %d",iKeyPoints);
                            ret = false;
                            return ret;
                            break;
                        }
                        //                    assert(patchSzie*cols_index < queryPattern.cols - patchSzie );
                        //                    assert(patchSzie*cols_index < trainPattern.cols - patchSzie );
                    }

                    Point2f Origin(trainPattern.cols - patchSzie - cols_index*roiPWidth,roiPWidth*rows_index);

                    Rect ROI(trainPattern.cols - patchSzie - cols_index*roiPWidth
                            ,roiPWidth*rows_index,patchSzie,patchSzie);
                    Mat roiQueryArea(queryPattern,ROI);
                    Mat roiTrainArea(trainPattern,ROI);

                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiTrainArea,roiTrainArea);
                    pyrDown(roiTrainArea,roiTrainArea);

                    m_patch.setPatch(Origin,roiQueryArea,roiTrainArea);

                    vector <Point2f> queryPoints,trainPoints;
                    m_patch.getPointsMatchers(queryPoints,trainPoints);
                    savePoints(queryPoints,trainPoints);

                    iKeyPoints += queryPoints.size();
                    rows_index++;
                }
                LOGE("right top corner timeCount = %d",timeCount);
            }
                break;
            case 2://right bottom corner
            {
                //LOGE("CORNER 2");
                int timeCount = 0;
                while(iKeyPoints <= keyPointThread)
                {
                    //LOGE("left bottom corner!");
                    timeCount++;
                    int moveRange = cols_index*roiPWidth;
                    if(moveRange > queryPattern.cols/2/* - patchSzie */)//row changed
                    {
                        rows_index++;
                        cols_index = 0;
                        moveRange = 0;
                        if(roiPWidth*rows_index > queryPattern.rows - patchSzie ||
                           roiPWidth*rows_index > trainPattern.rows - patchSzie )
                        {
                            LOGE("points in right bottom corner are too little!\n");
                            LOGE("right bottom corner KeyPoints = %d",iKeyPoints);
                            ret = false;
                            return ret;
                            break;
                        }
                        //                    assert(patchSzie*rows_index < queryPattern.rows - patchSzie );
                        //                    assert(patchSzie*rows_index < trainPattern.rows - patchSzie );
                    }
                    Point2f Origin(queryPattern.cols - patchSzie - roiPWidth*cols_index,
                                   queryPattern.rows - patchSzie - roiPWidth*rows_index);

                    Rect ROI(queryPattern.cols - patchSzie - roiPWidth*cols_index,
                             queryPattern.rows - patchSzie - roiPWidth*rows_index,patchSzie,patchSzie);
                    Mat roiQueryArea(queryPattern,ROI);
                    Mat roiTrainArea(trainPattern,ROI);

                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiTrainArea,roiTrainArea);
                    pyrDown(roiTrainArea,roiTrainArea);

                    m_patch.setPatch(Origin,roiQueryArea,roiTrainArea);

                    vector <Point2f> queryPoints,trainPoints;
                    m_patch.getPointsMatchers(queryPoints,trainPoints);
                    savePoints(queryPoints,trainPoints);

                    iKeyPoints += queryPoints.size();
                    cols_index++;
                }
                LOGE("right bottom corner timeCount = %d",timeCount);
            }
                break;
            case 3://left bottom corner
            {
                //LOGE("CORNER 3");
                int timeCount = 0;
                while(iKeyPoints <= keyPointThread)
                {
                    timeCount++;
                    //LOGE("right bottom corner!");
                    int moveRange = rows_index*roiPWidth;
                    if(moveRange > queryPattern.rows/2/* - patchSzie */)//col changed
                    {
                        cols_index++;
                        rows_index = 0;
                        moveRange = 0;

                        if(roiPWidth*cols_index > queryPattern.cols - patchSzie ||
                           roiPWidth*cols_index >  trainPattern.cols - patchSzie )
                        {
                            LOGE("points in left bottom corner are too little!\n");
                            LOGE("left bottom corner KeyPoints = %d",iKeyPoints);
                            ret = false;
                            return ret;
                            break;
                        }
                        //                    assert(patchSzie*cols_index <  queryPattern.cols - patchSzie );
                        //                    assert(patchSzie*cols_index <  trainPattern.cols - patchSzie );
                    }

                    Point2f Origin(roiPWidth*cols_index,queryPattern.rows
                                                        - patchSzie - roiPWidth*rows_index);

                    Rect ROI(roiPWidth*cols_index,queryPattern.rows
                                                  - patchSzie - roiPWidth*rows_index,patchSzie,patchSzie);
                    Mat roiQueryArea(queryPattern,ROI);
                    Mat roiTrainArea(trainPattern,ROI);

                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiQueryArea,roiQueryArea);
                    pyrDown(roiTrainArea,roiTrainArea);
                    pyrDown(roiTrainArea,roiTrainArea);

                    m_patch.setPatch(Origin,roiQueryArea,roiTrainArea);

                    vector <Point2f> queryPoints,trainPoints;
                    m_patch.getPointsMatchers(queryPoints,trainPoints);
                    savePoints(queryPoints,trainPoints);

                    iKeyPoints += queryPoints.size();
                    rows_index++;
                }
                LOGE("left bottom corner timeCount = %d",timeCount);
            }
                break;
            default:
                break;
        }
    }

    return ret;
}

bool GetHomography::setMode(int homoMethod)
{
    bool ret = true;
    switch (homoMethod) {
        case RANSAC:
            HomoMethod = RANSAC;
            break;
        case RHO:
            HomoMethod = RHO;
            break;
        case LMEDS:
            HomoMethod = LMEDS;
            break;
        default:
            HomoMethod = RANSAC;
            ret = false;
            break;
    }
    return ret;
}

Mat GetHomography::getHomography(double reprojectionThreshold)
{
    workBegin();
    bool ret = detectPoints();
    LOGE("DetectPoints time count!");
    workEnd();
    if(ret)
    {
        /*RHO
        *RANSAC 和 LMEDS  , 使用坐标点对生成了很多不同的随机组合子集（每四对一组），
        *使用这些子集和一个简单的最小二乘法来估计变换矩阵，然后计算出单应性的质量，
        *最好的子集被用来产生初始单应性的估计和掩码。
        *RANSAC方法几乎可以处理任何异常，但是需要一个阈值，
        *LMEDS 方法不需要任何阈值，但是只有在inliers大于50%时才能计算正确，
        *最后，如果没有outliers和噪音非常小，则可以使用默认的方法。*/
        /*ransacReprojThreshod 仅在RANSAC方法中使用，一个点对被认为是内层围值（非异常值）所允许的最大投影误差。即如果：
        那么点i被认为是异常值。如果srcPoints和dstPoints单位是像素，通常意味着在某些情况下这个参数的范围在1到10之间*/
        workBegin();
        LOGE("findHomography time count!");
        //vector<uchar> inliersMask(m_trainPoints.size());
        //LOGE("total num of Points = %d",m_trainPoints.size());
        Mat homography = findHomography(m_queryPoints,m_trainPoints, HomoMethod, reprojectionThreshold, noArray(), 2000, 0.995);
        //Mat homography = findHomography( m_queryPoints, m_trainPoints, noArray(),method,reprojectionThreshold);
        //需要转置处理，因为GLSL里默认是以列主序读取数组的，而opencv默认的是行主序
        m_Homography = homography.t();
        workEnd();
    }

    //workEnd();
    return m_Homography;

}





