//
// Created by linqi on 16-4-5.
//

#include "include/OrbPatch.h"


//#define  _DEBUG_MODE_
#define  _SUM_TIME_


OrbPatch::OrbPatch()
{
    origin = Point2f(0,0);
}

OrbPatch::~OrbPatch()
{

}

//开始计时
void OrbPatch::workBegin()
{
    work_begin = getTickCount();
}

//结束计时
void OrbPatch::workEnd()
{
    work_end = getTickCount() - work_begin;
    cout<<"time = "<<work_end /((double)getTickFrequency() )* 1000.0<<"ms"<<endl;
}

void OrbPatch::createPatch(const int nFeatures,const int nlevels,const int edgeThreshold,int _descriptorSize)
{
    //m_detector = ORB::create(10,1.2,1,31,0,2,ORB::HARRIS_SCORE,31,10);//now
    //m_detector = ORB::create(nFeatures,1.15,2,9,0,2,ORB::HARRIS_SCORE,31,5); //stable1 580ms
    //m_detector = ORB::create(nFeatures,1.2,nlevels,6,0,2,ORB::HARRIS_SCORE,31,10);
//    m_detector = ORB::create(nFeatures,1.15,nlevels,6,0,2,ORB::HARRIS_SCORE,21,5);
    m_detector = ORB::create(nFeatures,1.15,nlevels,edgeThreshold,0,2,ORB::HARRIS_SCORE,_descriptorSize,10);
    if(!m_detector)
        assert("Can not create Detector of ORB!");
    m_matcher = DescriptorMatcher::create("BruteForce-Hamming");

    if(!m_matcher)
        assert("Can not create Matcher of BruteForce-Hamming!");

    return ;
}

OrbPatch::OrbPatch(Point2f _origin,const Mat _queryImage,const Mat _trainImage )
{
    origin = _origin;
    qImage = _queryImage;
    tImage = _trainImage;
}

void OrbPatch::setPatch(const Point2f _origin,const Mat _queryImage,const Mat _trainImage)
{
    if(!m_detector)
        assert("Please Create Patch frist!");
    origin = _origin;
    qImage = _queryImage;
    tImage = _trainImage;
    return;
}

void OrbPatch::detectKeypoints(void)
{
    assert(qImage.channels() == 1);
    assert(tImage.channels() == 1);
    qKeypoints.clear();
    tKeypoints.clear();
#ifndef _SUM_TIME_
    cout<<" ******** detect time counts ! ******** "<<endl;
    workBegin();
#endif
//        m_detector->clear();
    m_detector->detect(qImage, qKeypoints);
    m_detector->detect(tImage, tKeypoints);
//    cout<<"the num of qureyKeyPoints = "<<qKeypoints.size()<<endl;
//    cout<<"the num of trainKeyPoints = "<<tKeypoints.size()<<endl;
#ifndef _SUM_TIME_
    workEnd();
#endif
    return ;
}

void OrbPatch::extractDescriptors(void)
{
//    assert(!qKeypoints.empty());
//    assert(!tKeypoints.empty());
    if(qKeypoints.empty() ||tKeypoints.empty())
        return;
#ifndef _SUM_TIME_
    cout<<" ******** extract time counts ! ******** "<<endl;
    workBegin();
#endif
    //    m_detector->clear();
    m_detector->compute(qImage, qKeypoints,qDescriptor);
    m_detector->compute(tImage, tKeypoints, tDescriptor);
#ifndef _SUM_TIME_
    workEnd();
#endif
    return ;
}

void OrbPatch::detectAndCompute(void)
{
    detectKeypoints();
    extractDescriptors();
    return ;
}

void OrbPatch::knnMatch(void)
{
//    assert(!qDescriptor.empty());
//    assert(!tDescriptor.empty());
    if(qDescriptor.empty() ||tDescriptor.empty())
        return ;
    const int k = 2;
#ifndef _SUM_TIME_
    cout<<" ******** match time counts! ******** "<<endl;
    workBegin();
#endif

    vector< vector<DMatch> > knnMatches;
    if(!m_matcher)
        assert("Please Create Patch frist!");
    m_matcher->clear();
    m_matcher->add(vector<Mat>(1, tDescriptor));
    m_matcher->train();
    m_matcher->knnMatch(qDescriptor, knnMatches, k);
#ifndef _SUM_TIME_
    workEnd();
#endif
    stable_DMatch.clear();
    for (size_t matchIndex = 0; matchIndex < knnMatches.size(); matchIndex++)
    {
        const DMatch& bestMatch = knnMatches[matchIndex][0];
        const DMatch& betterMatch = knnMatches[matchIndex][1];

        //bestMatch.distance = 25
        if(betterMatch.distance - bestMatch.distance > 5 && bestMatch.distance < 25)
        {
            double dErrorY = qKeypoints[bestMatch.queryIdx].pt.y
                             - tKeypoints[bestMatch.trainIdx].pt.y;
            double dErrorX = qKeypoints[bestMatch.queryIdx].pt.x
                             - tKeypoints[bestMatch.trainIdx].pt.x;
            if(fabs(dErrorY) < 12 && fabs(dErrorX) < 12 )
                //if(fabs(dErrorY) < 50 && fabs(dErrorX) < 50)
                stable_DMatch.push_back(bestMatch);
        }
    }
#ifdef _DEBUG_MODE_
    Mat dst;
    saveMatches(qImage,qKeypoints,tImage,tKeypoints,stable_DMatch,dst);
    imwrite("matches.jpg",dst);
#endif

    Point2f queryPoints;
    Point2f trainPoints;
    qPointsVec.clear();
    tPointsVec.clear();
    size_t iDMatches = stable_DMatch.size();
    for(size_t index = 0;index < iDMatches;index++)
    {
        //because of pyrDown,we need to mutilply both x and y with 4
        queryPoints.x = qKeypoints[stable_DMatch[index].queryIdx].pt.x*4 + origin.x;
        queryPoints.y = qKeypoints[stable_DMatch[index].queryIdx].pt.y*4 + origin.y;

        trainPoints.x = tKeypoints[stable_DMatch[index].trainIdx].pt.x*4 + origin.x;
        trainPoints.y = tKeypoints[stable_DMatch[index].trainIdx].pt.y*4 + origin.y;

        qPointsVec.push_back(queryPoints);
        tPointsVec.push_back(trainPoints);
    }
//    cout<<"pointsMatches = "<<tPointsVec.size()<<endl;
    return ;
}

int OrbPatch::getPointsMatchers(vector <Point2f> & _queryPoints,vector <Point2f> & _trainPoints)
{
    if(!qImage.empty() ||!tImage.empty())
        assert("Please set Patch frist!");

    int ret = qPointsVec.size();
    _queryPoints.clear();
    _trainPoints.clear();

    detectAndCompute();
    knnMatch();
    _queryPoints = qPointsVec;
    _trainPoints = tPointsVec;

    return ret;
}

//保存特征点，在图上体现出来
void OrbPatch::saveKeypoints(const Mat& image, const vector<KeyPoint>& keypoints, Mat& outImage)
{
    assert(!keypoints.empty());

    cv::drawKeypoints(image, keypoints, outImage, /*Scalar(255,255,0),*/Scalar::all(-1),
                      DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

}

//保存匹配的特征点，在图上体现出来
void OrbPatch::saveMatches(const Mat& queryImage,
                        const vector<KeyPoint>& queryKeypoints,
                        const Mat& trainImage,
                        const vector<KeyPoint>& trainKeypoints,
                        const vector<DMatch>& matches,
                        Mat& outImage)
{
    assert(!queryKeypoints.empty());
    assert(!trainKeypoints.empty());

    cv::drawMatches(queryImage, queryKeypoints, trainImage, trainKeypoints, matches, outImage,
                    Scalar::all(-1), Scalar::all(-1),vector<char>(),  DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

}
