//
// Created by linqi on 16-4-5.
//

#ifndef MY_JNI_ORBPATCH_H
#define MY_JNI_ORBPATCH_H



#include "iostream"
#include "vector"
#include "string"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class OrbPatch
{
public:
    OrbPatch();
    OrbPatch(Point2f _origin,const Mat _queryImage,const Mat _trainImage);
    ~OrbPatch();
private:
    void detectKeypoints(void);   // 检测特征点
    void extractDescriptors(void);   // 提取特征向量
    void detectAndCompute(void);
    void knnMatch(void); // K近邻匹配

private:
    void saveKeypoints(const Mat& image,
                       const vector<KeyPoint>& keypoints, Mat& outImage);  // 保存特征点
    void saveMatches(const Mat& queryImage,
                     const vector<KeyPoint>& queryKeypoints,
                     const Mat& trainImage,
                     const vector<KeyPoint>& trainKeypoints,
                     const vector<DMatch>& matches,
                     Mat& outImage);   // 保存匹配结果到图片中
public:
    // nFeatures = 15, nlevels = 2
    void createPatch(const int nFeatures = 15,const int nlevels = 2
            ,const int edgeThreshold = 16,int _descriptorSize = 31);

    void setPatch(Point2f _origin,const Mat _queryImage,const Mat _trainImage);
    int getPointsMatchers(vector <Point2f> & _queryPoints,vector <Point2f> & _trainPoints);

public:
    void workBegin();
    void workEnd();

public:
    vector < Point2f > qPointsVec;
    vector < Point2f > tPointsVec;
private:
    Mat qImage;
    vector<KeyPoint> qKeypoints;
    Mat qDescriptor;

    Mat tImage;
    vector<KeyPoint> tKeypoints;
    Mat tDescriptor;
private:
    //FeatureDetector是关键点检测类的抽象基类
    Ptr< Feature2D > m_detector;
    //DescriptorMatcher是匹配器的抽象基类
    Ptr<DescriptorMatcher> m_matcher;
private:
    vector<DMatch>  stable_DMatch;
    Point2f origin;//patch原点

    int64 work_begin;
    int64 work_end;
};



#endif //MY_JNI_ORBPATCH_H
