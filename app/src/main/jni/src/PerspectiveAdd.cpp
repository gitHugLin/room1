//
// Created by linqi on 16-4-5.
//

#include "include/PerspectiveAdd.h"



PerspectiveAdd::PerspectiveAdd()
{
    work_begin = 0;
    work_end = 0;
    checkInitOpenGLES = false;

    //initOpenGLES();
}

PerspectiveAdd::~PerspectiveAdd()
{
    for (int i=0; i<6; i++)
    {
        mGraphicBuffer[i] = NULL;
        eglDestroyImageKHR(display, mEGLImage[i]);
        mEGLImage[i] = EGL_NO_IMAGE_KHR;
    }
    mTargetGraphicBuffer = NULL;
    eglDestroyImageKHR(display, mTargetEGLImage);
    mTargetEGLImage = EGL_NO_IMAGE_KHR;

    glUseProgram(0);
    glDeleteProgram(programObject);
    glDeleteTextures(1,&textureID1);
    glDeleteTextures(1,&textureID2);
    glDeleteTextures(1,&textureID3);
    glDeleteTextures(1,&textureID4);
    glDeleteTextures(1,&textureID5);
    glDeleteTextures(1,&textureID6);
    DestroyEGL();
}


bool PerspectiveAdd::setMode(int homoMethod )
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


int PerspectiveAdd::Progress(Mat & _outMat)
{
    if(checkInitOpenGLES == false)
    {
        assert("Please initOpenGLES first!");
        return 0;
    }

    vector <HomIntMat> homIntMatVec;
    workBegin();
    MutGetHomography MTFeature(m_grays);
    MTFeature.setMode(HomoMethod);
    MTFeature.process(homIntMatVec);
    workEnd("MutGetHomography");

    fHomography fhom;
    vector <fHomography> HomVec;
    HomVec.clear();
    float prtHomography[9] = {1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0};
    memcpy(fhom.Homography,prtHomography, sizeof(prtHomography));
    HomVec.push_back(fhom);
    for(int k = 0; k < 5; k++)
    {
        int index = k;
        if(k > 1)
            index = k + 1;
        for(int i = 0; i < 5; i++)
        {
            if(homIntMatVec[i].index == index )
            {
                LOGE("Homography = %d",index);
                double *temp = (double *)homIntMatVec[i].Homography.data;
                for(int j = 0;j < 9;j++)
                {
                    fhom.Homography[j] = *(double*)(temp+j);
                    LOGE("Mat = %lf",fhom.Homography[j]);
                }
                HomVec.push_back(fhom);
                break;
            }
            else if(homIntMatVec[i].index == -1)
            {
                /* memcpy(fhom.Homography,prtHomography, sizeof(prtHomography));
               HomVec.push_back(fhom);*/
                _outMat = m_images[3];
                cvtColor(_outMat, _outMat, CV_BGR2RGB);
                return -1;
            }
        }
    }
    //workBegin();
    perspectiveAndAdd(HomVec,_outMat);
    //workEnd("perspectiveAndAdd");
    //imwrite("/mnt/obb/Capture/gles.jpg",_outMat);
    cvtColor(_outMat,_outMat,CV_RGB2BGR);
    return 1;
}

int PerspectiveAdd::initOpenGLES(Mat *images,Mat *grays)
{
    checkInitOpenGLES = true;

    for(size_t i = 0; i < 6; i++)
    {
        m_images[i] = images[i];
        m_grays[i] = grays[i];
    }

    Width = images[0].size().width;
    Height = images[0].size().height;

    const char gPerspectiveVertexShader[] =
            "attribute vec4 a_position;\n"
                    "uniform mat3 uMVPMatrix1;\n"
                    "uniform mat3 uMVPMatrix2;\n"
                    "uniform mat3 uMVPMatrix3;\n"
                    "uniform mat3 uMVPMatrix4;\n"
                    "uniform mat3 uMVPMatrix5;\n"
                    "uniform mat3 uMVPMatrix6;\n"
                    "uniform vec2 textureSize;\n"
                    "attribute vec2 a_texCoord;\n"
                    "varying vec2 v_texCoord1;\n"
                    "varying vec2 v_texCoord2;\n"
                    "varying vec2 v_texCoord3;\n"
                    "varying vec2 v_texCoord4;\n"
                    "varying vec2 v_texCoord5;\n"
                    "varying vec2 v_texCoord6;\n"
                    "void main() {\n"
                    "  float width = textureSize.x;\n"
                    "  float height = textureSize.y;\n"
                    "  vec3 sPoint = vec3(a_texCoord.x*width,a_texCoord.y*height,1.0);\n"
                    "  vec3 dPoint = uMVPMatrix1*sPoint;\n"
                    "  float X = dPoint.x/(dPoint.z*width);\n"
                    "  float Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord1 = vec2(X,Y);\n"
                    "  dPoint = uMVPMatrix2*sPoint;\n"
                    "  X = dPoint.x/(dPoint.z*width);\n"
                    "  Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord2 = vec2(X,Y);\n"
                    "  dPoint = uMVPMatrix3*sPoint;\n"
                    "  X = dPoint.x/(dPoint.z*width);\n"
                    "  Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord3 = vec2(X,Y);\n"
                    "  dPoint = uMVPMatrix4*sPoint;\n"
                    "  X = dPoint.x/(dPoint.z*width);\n"
                    "  Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord4 = vec2(X,Y);\n"
                    "  dPoint = uMVPMatrix5*sPoint;\n"
                    "  X = dPoint.x/(dPoint.z*width);\n"
                    "  Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord5 = vec2(X,Y);\n"
                    "  dPoint = uMVPMatrix6*sPoint;\n"
                    "  X = dPoint.x/(dPoint.z*width);\n"
                    "  Y = dPoint.y/(dPoint.z*height);\n"
                    "  v_texCoord6 = vec2(X,Y);\n"
                    "  gl_Position = a_position;\n"
                    "}\n";

    const char gPerspectiveFragmentShader[] =
            "precision highp float;\n"
                    "varying vec2 v_texCoord1;\n"
                    "varying vec2 v_texCoord2;\n"
                    "varying vec2 v_texCoord3;\n"
                    "varying vec2 v_texCoord4;\n"
                    "varying vec2 v_texCoord5;\n"
                    "varying vec2 v_texCoord6;\n"
                    "uniform sampler2D u_samplerTexture1;\n"
                    "uniform sampler2D u_samplerTexture2;\n"
                    "uniform sampler2D u_samplerTexture3;\n"
                    "uniform sampler2D u_samplerTexture4;\n"
                    "uniform sampler2D u_samplerTexture5;\n"
                    "uniform sampler2D u_samplerTexture6;\n"
                    "void main() {\n"
                    //"  gl_FragColor = texture2D(u_samplerTexture1,v_texCoord1)*0.5;\n"
                    "  gl_FragColor += texture2D(u_samplerTexture2,v_texCoord2)*1.0;\n"
                    //"  gl_FragColor += texture2D(u_samplerTexture3,v_texCoord3)*0.167;\n"
                    //"  gl_FragColor += texture2D(u_samplerTexture4,v_texCoord4)*0.167;\n"
                    //"  gl_FragColor += texture2D(u_samplerTexture5,v_texCoord5)*0.167;\n"
                    //"  gl_FragColor += texture2D(u_samplerTexture6,v_texCoord6)*0.167;\n"
                    "}\n";
    // Init EGL display, surface and context
    if(!InitEGL())
    {
        LOGE("Init EGL fail\n");
        return GL_FALSE;
    }
    programObject = LoadProgram(gPerspectiveVertexShader,gPerspectiveFragmentShader);
    glUseProgram(programObject);

    //get attribution unit index
    vPositionHandle = glGetAttribLocation(programObject, "a_position" );
    vTexCoordHandle = glGetAttribLocation(programObject, "a_texCoord" );
    vHomograyHandle1 = glGetUniformLocation(programObject, "uMVPMatrix1");
    vHomograyHandle2 = glGetUniformLocation(programObject, "uMVPMatrix2");
    vHomograyHandle3 = glGetUniformLocation(programObject, "uMVPMatrix3");
    vHomograyHandle4 = glGetUniformLocation(programObject, "uMVPMatrix4");
    vHomograyHandle5 = glGetUniformLocation(programObject, "uMVPMatrix5");
    vHomograyHandle6 = glGetUniformLocation(programObject, "uMVPMatrix6");
    vSizeHandle = glGetUniformLocation(programObject, "textureSize");
    // Set the sampler texture unit index
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture1"), 0);
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture2"), 1);
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture3"), 2);
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture4"), 3);
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture5"), 4);
    glUniform1i(glGetUniformLocation(programObject, "u_samplerTexture6"), 5);

    GLuint targetTexId;
    initializeTmpResEGLImage((int) Width, (int) Height, &targetTexId, &fboTargetHandle, 6);

    //create texture object
    glGenTextures(1, &textureID1);
    glGenTextures(1, &textureID2);
    glGenTextures(1, &textureID3);
    glGenTextures(1, &textureID4);
    glGenTextures(1, &textureID5);
    glGenTextures(1, &textureID6);

    createEGLImageTexture(textureID1, 0 , NULL, Width, Height, 3);
    createEGLImageTexture(textureID2, 1 , NULL, Width, Height, 3);
    createEGLImageTexture(textureID3, 2 , NULL, Width, Height, 3);
    createEGLImageTexture(textureID4, 3 , NULL, Width, Height, 3);
    createEGLImageTexture(textureID5, 4 , NULL, Width, Height, 3);
    createEGLImageTexture(textureID6, 5 , NULL, Width, Height, 3);

    //the third frame is the standard frame,we set it ID for textureID0
    updateEGLImageTexture(textureID1, 0, Width, Height, m_images[2].data);
    updateEGLImageTexture(textureID2, 1, Width, Height, m_images[0].data);
    updateEGLImageTexture(textureID3, 2, Width, Height, m_images[1].data);
    updateEGLImageTexture(textureID4, 3, Width, Height, m_images[3].data);
    updateEGLImageTexture(textureID5, 4, Width, Height, m_images[4].data);
    updateEGLImageTexture(textureID6, 5, Width, Height, m_images[5].data);

    float gSize[2] = {Width,Height};
    glUniform2fv(vSizeHandle,1,gSize);
    return GL_TRUE;
}

void PerspectiveAdd::initializeTmpResEGLImage(int fboWidth, int fboHeight, GLuint *tex,
                                        GLuint * fbo, GLuint texGroup)
{
    glGenTextures(1, tex);
    glActiveTexture(texGroup);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mTargetGraphicBuffer = new GraphicBuffer(fboWidth, fboHeight, HAL_PIXEL_FORMAT_RGB_888,
                                             GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_WRITE_RARELY);

    EGLClientBuffer clientBuffer = (EGLClientBuffer)mTargetGraphicBuffer->getNativeBuffer();
    mTargetEGLImage = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                        clientBuffer, 0);

    //glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureid);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)mTargetEGLImage);

    glGenFramebuffers(1, fbo);
    LOGD("generate tex/fbo for target tex id: %d, fbo id: %d, w-h: %d-%d", *tex, *fbo, fboWidth, fboHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOGD("framebuffer statuc check fail: %d", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//render in here
int PerspectiveAdd::perspectiveAndAdd(const vector <fHomography> & HomographyVec, Mat &dstImage)
{

    //LOGE("Time of perspectiveAndAdd");
    //workBegin();
    glUniformMatrix3fv(vHomograyHandle1,1,GL_FALSE,HomographyVec[0].Homography);
    glUniformMatrix3fv(vHomograyHandle2,1,GL_FALSE,HomographyVec[1].Homography);
    glUniformMatrix3fv(vHomograyHandle3,1,GL_FALSE,HomographyVec[2].Homography);
    glUniformMatrix3fv(vHomograyHandle4,1,GL_FALSE,HomographyVec[3].Homography);
    glUniformMatrix3fv(vHomograyHandle5,1,GL_FALSE,HomographyVec[4].Homography);
    glUniformMatrix3fv(vHomograyHandle6,1,GL_FALSE,HomographyVec[5].Homography);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //begin to render
    glViewport(0, 0, Width, Height);
    const GLfloat vVertices[] = {
            // X,  Y, Z, W,    U, V
            1,  1, 0, 1,    1, 1, //Top Right
            -1,  1, 0, 1,    0, 1, //Top Left
            -1, -1, 0, 1,    0, 0, // Bottom Left
            1, -1, 0, 1,    1, 0 //Bottom Right
    };

    GLushort indices[] = { 0, 3, 2, 2, 1, 0 };
    GLsizei stride = 6*sizeof(GLfloat);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fboTargetHandle);

    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE1);
    glActiveTexture(GL_TEXTURE2);
    glActiveTexture(GL_TEXTURE3);
    glActiveTexture(GL_TEXTURE4);
    glActiveTexture(GL_TEXTURE5);

    glBindTexture(GL_TEXTURE_2D,textureID1);
    glBindTexture(GL_TEXTURE_2D,textureID2);
    glBindTexture(GL_TEXTURE_2D,textureID3);
    glBindTexture(GL_TEXTURE_2D,textureID4);
    glBindTexture(GL_TEXTURE_2D,textureID5);
    glBindTexture(GL_TEXTURE_2D,textureID6);
    // Load the vertex position
    glVertexAttribPointer(vPositionHandle, 4, GL_FLOAT, GL_FALSE, stride,
                          vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(vTexCoordHandle, 2, GL_FLOAT, GL_FALSE, stride,
                          &vVertices[4]);
    glEnableVertexAttribArray(vPositionHandle);
    glEnableVertexAttribArray(vTexCoordHandle);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glFinish();
    glDisableVertexAttribArray(vPositionHandle);
    glDisableVertexAttribArray(vTexCoordHandle);
    //workEnd();

    int err = mTargetGraphicBuffer->lock(GRALLOC_USAGE_SW_READ_RARELY, (void **) (&mTargetGraphicBufferAddr));
    if (err != 0 || mTargetGraphicBufferAddr == NULL)
    {
        LOGD("mYUVTexBuffer->lock(...) failed: %d\n", err);
        return -1;
    }
    Mat result(Height, Width, CV_8UC3, mTargetGraphicBufferAddr);

    dstImage = result.clone();
    //dstImage = result;
    err = mTargetGraphicBuffer->unlock();
    if (err != 0)
    {
        LOGD("mYUVTexBuffer->unlock() failed: %d\n", err);
        return -1;
    }

    //workEnd("COMPOSITION");

    return GL_TRUE;
}

int PerspectiveAdd::DestroyEGL()
{
    //Typical egl cleanup
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    //eglDestroySurface(display, mSurface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    eglReleaseThread();
}

int PerspectiveAdd::InitEGL()
{

    NativeWindowType eglWindow = NULL;
    EGLSurface surface = NULL;

    EGLConfig configs[2];
    EGLBoolean eRetStatus;
    EGLint majorVer, minorVer;
    EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLint numConfigs;
    EGLint cfg_attribs[] = {EGL_BUFFER_SIZE,    EGL_DONT_CARE,
                            EGL_DEPTH_SIZE,     8,
                            EGL_RED_SIZE,       8,
                            EGL_GREEN_SIZE,     8,
                            EGL_BLUE_SIZE,      8,
                            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL_NONE};

    // Get default display connection
    display = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    if ( display == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS )
    {
        LOGE("ERROR: eglGetDisplay return EGL_NO_DISPLAY");
        return EGL_FALSE;
    }

    // Initialize EGL display connection
    eRetStatus = eglInitialize(display, &majorVer, &minorVer);
    if( eRetStatus != EGL_TRUE || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglInitialize return EGL_FALSE");
        return EGL_FALSE;
    }

    //Get a list of all EGL frame buffer configurations for a display
    eRetStatus = eglGetConfigs (display, configs, 2, &numConfigs);
    if( eRetStatus != EGL_TRUE || eglGetError() != EGL_SUCCESS )
    {
        LOGE("ERROR: eglGetConfigs return EGL_FALSE");
        return EGL_FALSE;
    }

    // Get a list of EGL frame buffer configurations that match specified attributes
    eRetStatus = eglChooseConfig (display, cfg_attribs, configs, 2, &numConfigs);
    if( eRetStatus != EGL_TRUE  || !numConfigs || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglChooseConfig return EGL_FALSE");
        return EGL_FALSE;
    }

    sp <IGraphicBufferProducer> producer;
    sp <IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    sp <GLConsumer> mST = new GLConsumer(consumer, 123, GLConsumer::TEXTURE_EXTERNAL, true, false);
    mST->setDefaultBufferSize(64, 64);
    mST->setDefaultBufferFormat(HAL_PIXEL_FORMAT_RGBA_8888);
    sp <Surface> mSTC = new Surface(producer);
    sp <ANativeWindow> window = mSTC.get();
    eglWindow = window.get();

    //LOGE("eglCreateWindowSurface");
    // Create a new EGL window surface
    surface = eglCreateWindowSurface(display, configs[0], eglWindow, NULL);
    if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglCreateWindowSurface return EGL_FALSE");
        return EGL_FALSE;
    }

    // Set the current rendering API (EGL_OPENGL_API, EGL_OPENGL_ES_API,EGL_OPENVG_API)
    eRetStatus = eglBindAPI(EGL_OPENGL_ES_API);
    if (eRetStatus != EGL_TRUE || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglBindAPI return EGL_FALSE");
        return EGL_FALSE;
    }

    // Create a new EGL rendering context
    context = eglCreateContext (display, configs[0], EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglCreateContext return EGL_FALSE");
        return EGL_FALSE;
    }

    // Attach an EGL rendering context to EGL surfaces
    eRetStatus = eglMakeCurrent(display, surface, surface, context);
    if( eRetStatus != EGL_TRUE || eglGetError() != EGL_SUCCESS)
    {
        LOGE("ERROR: eglMakeCurrent return EGL_FALSE");
        return EGL_FALSE;
    }
    //If interval is set to a value of 0, buffer swaps are not synchronized to a video frame, and the swap happens as soon as the render is complete.
    eglSwapInterval(display,0);

    return EGL_TRUE;
}

GLuint PerspectiveAdd::LoadShader( GLenum type, const char *shaderSrc )
{
    GLuint shader;
    GLint compiled;

    // Create an empty shader object, which maintain the source code strings that define a shader
    shader = glCreateShader ( type );

    if ( shader == 0 )
        return 0;

    // Replaces the source code in a shader object
    glShaderSource ( shader, 1, &shaderSrc, NULL );
    // Compile the shader object
    glCompileShader ( shader );

    // Check the shader object compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    if ( !compiled )
    {
        GLint infoLen = 0;
        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = (char*)malloc (sizeof(char) * infoLen );
            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            LOGE ( "Error compiling shader:\n%s\n", infoLog );
            free ( infoLog );
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

GLuint PerspectiveAdd::LoadProgram( const char *vShaderStr, const char *fShaderStr )
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
    fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

    // Create the program object
    programObject = glCreateProgram( );
    if ( programObject == 0 )
        return 0;

    // Attaches a shader object to a program object
    glAttachShader ( programObject, vertexShader );
    glAttachShader ( programObject, fragmentShader );

    // Link the program object
    glLinkProgram ( programObject );

    // Check the link status
    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

    if ( !linked )
    {
        GLint infoLen = 0;
        glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = (char*)malloc (sizeof(char) * infoLen );
            glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
            LOGE ( "Error linking program:\n%s\n", infoLog );
            free ( infoLog );
        }
        glDeleteProgram ( programObject );
        return GL_FALSE;
    }

    // Free no longer needed shader resources
    glDeleteShader ( vertexShader );
    glDeleteShader ( fragmentShader );

    return programObject;
}

GLuint PerspectiveAdd::createSimpleTexture2D(GLuint _textureid, GLint _textureIndex ,GLubyte* pixels,
                                       int width, int height, int channels)
{
    // Bind the texture
    switch (_textureIndex) {
        case 0:
            glActiveTexture(GL_TEXTURE0);
            break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            break;
        default:
            glActiveTexture(GL_TEXTURE0);
            break;
    }
    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, _textureid);

    GLenum format;
    switch (channels) {
        case 3:
            format = GL_RGB;
            break;
        case 1:
            format = GL_LUMINANCE;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }
    // Load the texture(changeable)
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, pixels);
    // Set the filtering mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // 线形滤波
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // 线形滤波

    return _textureid;
}

GLuint PerspectiveAdd::createEGLImageTexture(GLuint _textureid, GLint _textureIndex ,GLubyte* pixels,
                                       int width, int height, int channels)
{
    workBegin();

    GLenum format;
    switch (channels) {
        case 3:
            format = GL_RGB;
            break;
        case 1:
            format = GL_LUMINANCE;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }

    mGraphicBuffer[_textureIndex] = new GraphicBuffer(width, height, HAL_PIXEL_FORMAT_RGB_888,
                                                      GraphicBuffer::USAGE_HW_TEXTURE);// | GraphicBuffer::USAGE_SW_WRITE_RARELY);

    EGLClientBuffer clientBuffer = (EGLClientBuffer) mGraphicBuffer[_textureIndex] ->getNativeBuffer();
    mEGLImage[_textureIndex] = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                                 clientBuffer, 0);
    workEnd("TEX EGL");

    // Bind the texture
    switch (_textureIndex) {
        case 0:
            glActiveTexture(GL_TEXTURE0);
            break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            break;
        default:
            glActiveTexture(GL_TEXTURE3);
            break;
    }
    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, _textureid);

    // Set the filtering mode
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 线形滤波
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 线形滤波
}

GLuint PerspectiveAdd::updateEGLImageTexture(GLuint _textureid, int _textureIndex, int width, int height, GLubyte* pixels)
{
    LOGD("updateEGLImageTexture, index=%d, w-h: %d-%d, pixels:%.8x", _textureIndex, width, height, pixels);
    //if (mGraphicBufferAddr[_textureIndex] == NULL)
    {
        int err = mGraphicBuffer[_textureIndex]->lock(GRALLOC_USAGE_SW_WRITE_RARELY,
                                                      (void **) (&mGraphicBufferAddr[_textureIndex]));
        if (err != 0 || mGraphicBufferAddr[_textureIndex] == NULL) {
            LOGD("mYUVTexBuffer->lock(...) failed: %d\n", err);
            return -1;
        }
        memcpy(mGraphicBufferAddr[_textureIndex], pixels, width * height * 3);

        err = mGraphicBuffer[_textureIndex]->unlock();
        if (err != 0) {
            LOGD("mYUVTexBuffer->unlock() failed: %d\n", err);
            return -1;
        }
    }
    /*else {
        memcpy(mGraphicBufferAddr, pixels, width * height * 3);
    }*/
    glBindTexture(GL_TEXTURE_2D, _textureid);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)mEGLImage[_textureIndex]);

    return 0;
}

void PerspectiveAdd::checkFBO()
{
    // FBO status check
    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            LOGE("fbo complete");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            LOGE("fbo unsupported");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LOGE("绑定的FBO绑定了无效的纹理或RBO");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LOGE("绑定的FBO没有没有关联任何图像");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG:
            LOGE("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG");
            break;
        default:
            LOGE("Framebuffer Error");
            break;
    }
}
//开始计时

void PerspectiveAdd::workBegin()
{
    work_begin = getTickCount();
}

//结束计时
void PerspectiveAdd::workEnd(char* module_name)
{
    work_end = getTickCount() - work_begin;
    double Time = work_end /((double)getTickFrequency() )* 1000.0;
    LOGE("[%s] TIME = %lf ms \n", module_name, Time);
}


