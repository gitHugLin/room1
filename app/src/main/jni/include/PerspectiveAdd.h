//
// Created by linqi on 16-4-5.
//

#ifndef MY_JNI_PERSPECTIVEADD_H
#define MY_JNI_PERSPECTIVEADD_H



#include "opencv2/opencv.hpp"
#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <ui/GraphicBuffer.h>
#include "log.h"
#include "include/MyStruct.h"
#include "include/MutGetHomography.h"

using namespace mystruct;
using namespace cv;

using android::GraphicBuffer;
using android::sp;


class PerspectiveAdd
{
public:
    PerspectiveAdd();
    ~PerspectiveAdd();

public:
    //int initOpenGLES(const vector <Mat> & images,const vector <Mat> & grays);
    int initOpenGLES(Mat *images,Mat *grays);
    int Progress(Mat & _outMat);
    bool setMode(int homoMethod = RANSAC);
    int perspectiveAndAdd(const vector <fHomography> & HomographyVec, Mat &dstImage);
private:

    Mat m_images[6];
    Mat m_grays[6];

    double work_begin;
    double work_end;
    unsigned char *gData;
    float Width;
    float Height;
    int HomoMethod;
    bool checkInitOpenGLES;
    //OpenGLES data
    //GLuint fboHandle;
    GLuint vPositionHandle;
    GLuint vTexCoordHandle;
    GLuint vHomograyHandle1;
    GLuint vHomograyHandle2;
    GLuint vHomograyHandle3;
    GLuint vHomograyHandle4;
    GLuint vHomograyHandle5;
    GLuint vHomograyHandle6;

    GLuint vSizeHandle;

    GLuint programObject;
    // texture
    GLuint textureID1;
    GLuint textureID2;
    GLuint textureID3;
    GLuint textureID4;
    GLuint textureID5;
    GLuint textureID6;

    EGLContext context;
    EGLDisplay display;
    EGLImageKHR mEGLImage[6];
    sp <GraphicBuffer> mGraphicBuffer[6];
    char* mGraphicBufferAddr[6];

    GLuint fboTargetHandle;
    EGLImageKHR mTargetEGLImage;
    sp <GraphicBuffer> mTargetGraphicBuffer;
    char* mTargetGraphicBufferAddr;

private:
    int InitEGL();
    int DestroyEGL();
    void checkFBO();
    void workBegin();
    void workEnd(char* module_name = "null");
    GLuint LoadShader( GLenum type, const char *shaderSrc );
    GLuint LoadProgram( const char *vShaderStr, const char *fShaderStr );
    GLuint createSimpleTexture2D(GLuint _textureid, GLint _textureIndex ,GLubyte* pixels,
                                 int width, int height, int channels);

    void initializeTmpResEGLImage(int fboWidth, int fboHeight, GLuint *tex,
                                  GLuint * fbo, GLuint texGroup);
    GLuint createEGLImageTexture(GLuint _textureid, GLint _textureIndex ,GLubyte* pixels,
                                 int width, int height, int channels);
    GLuint updateEGLImageTexture(GLuint _textureid, int _textureIndex, int width, int height, GLubyte* pixels);
};







#endif //MY_JNI_PERSPECTIVEADD_H
