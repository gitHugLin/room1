#include "myjni.h"
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/PerspectiveAdd.h"
#include <dirent.h>
#include <sys/stat.h>


static double work_begin = 0;
static double work_end = 0;
static double gTime = 0;
static int gWidth  = 0;
static int gHeight = 0;
//开始计时
static void workBegin()
{
    work_begin = getTickCount();
}
//结束计时
static void workEnd()
{
    work_end = getTickCount() - work_begin;
    gTime = work_end /((double)getTickFrequency() )* 1000.0;
    LOGE("MFDnoisy TIME = %lf ms \n",gTime);
}

static void getImageUnderDir( char *path, char *suffix);
static Mat g_picVec[6];
static Mat g_grayVec[6];
static int texIndex = 0;
static Mat standardMat;
static Mat normalMat;
static PerspectiveAdd g_APUnit;

JNIEXPORT jfloatArray JNICALL calHomography(JNIEnv *env, jobject obj,jlong grayMatPtr,jboolean first)
{
    jfloatArray mHomography;
    mHomography = env->NewFloatArray(9);
    float prtHomography[9] = {1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0};
    Mat *texture = (Mat *)grayMatPtr;

    if(true == first){
        standardMat = *texture;
        env->SetFloatArrayRegion(mHomography, 0, 9, prtHomography);
        return mHomography;
    }
    else{
        normalMat = *texture;
        GetHomography homography(normalMat,standardMat);
        Mat Homography = homography.getHomography();
        if(!Homography.empty()){
            double *prt = (double *)Homography.data;
            for(int j = 0;j < 9; j++)
            {
                float date = *(double*)(prt + j);
                prtHomography[j] = date;
            }
        }
        else{
            prtHomography[0] = -1;
        }
/*        LOGE("Homography data analysis :\n");
        for(int j = 0;j < 9; j++)
            LOGE("Homography: Homography = %lf\n", prtHomography[j]);*/
        //imwrite("/mnt/internal_sd/APCamera/gray.jpg",*texture );
        env->SetFloatArrayRegion(mHomography, 0, 9, prtHomography);
        return mHomography;
    }
}

static void getImageUnderDir( const char *path, const char *suffix,const char *dstPath)
{
    int i = 0;
    struct dirent* ent = NULL;
    DIR *pDir;
    char dir[512];
    char tSuffix[8];
    struct stat statbuf;
    if( (pDir = opendir(path)) == NULL )
    {
        LOGE("getFileList:Cannot open directory:%s\n", path);
        return;
    }
    while( (ent = readdir(pDir)) != NULL )
    {
        //得到读取文件的绝对路径名
        snprintf( dir, 512,"%s/%s", path, ent->d_name );
        //得到文件信息
        lstat(dir, &statbuf);
        //判断是目录还是文件
        if( S_ISDIR(statbuf.st_mode) )
        {
            //排除当前目录和上级目录
            if(strcmp( ".",ent->d_name) == 0 || strcmp( "..",ent->d_name) == 0)
                continue;
            //如果是子目录,递归调用函数本身,实现子目录中文件遍历
            //递归调用,遍历子目录中文件
        } else {
            //LOGE("后缀名:%s",ent->d_name + strlen(ent->d_name) - strlen(suffix));
            //排除后缀名不是指定的　suffix 名的文件
            if(strcmp( suffix,ent->d_name + strlen(ent->d_name) - strlen(suffix)) != 0)
                continue;
            LOGE("绝对路径名:%s",dir);

            Mat rgb,yv12;
            Mat bayer = imread(dir,0);
            cvtColor(bayer, g_picVec[i], CV_BayerBG2RGB);
            rgb = g_picVec[i];
            cvtColor(rgb, yv12, COLOR_RGB2YUV_YV12);
            //矩阵构造函数传入data参数不分配矩阵数据，它们只是初始化矩阵头指向指定的数据
            Mat Ychannel(rgb.rows, rgb.cols, yv12.type(), yv12.data);
            //LOGE("yv12.type() = :%d",yv12.type());
            g_grayVec[i] = Ychannel.clone();

            i++;
            if(i == 3)
            {
                int name = getTickCount();
                char c[32];
                sprintf(c, "%05X", name);
                string path = "/mnt/internal_sd/APCamera/";
                string b = ".jpg";
                string fileName = path + c + b;
                vector<int> compression_params;
                compression_params.push_back(IMWRITE_JPEG_QUALITY);
                compression_params.push_back(100);
                imwrite(fileName, rgb, compression_params);
                imwrite(dstPath, rgb, compression_params);
            }
        }
    }

    closedir(pDir);
}

static pthread_mutex_t g_mutex;

JNIEXPORT void JNICALL initNDK(JNIEnv *env, jobject obj){
    pthread_mutex_init( &g_mutex, NULL );
    int HomMethod = LMEDS; // RHO   RANSAC LMEDS
    g_APUnit.setMode(HomMethod);
}

JNIEXPORT void JNICALL setTextureSize(JNIEnv *env, jobject obj,jint width,jint height)
{
    LOGE("SetTextureSize : WIDTH = %d , HEIGHT = %d",width,height);
    gWidth = width;
    gHeight = height;
    g_APUnit.initOpenGLES(width,height);
}

JNIEXPORT void JNICALL sendTextures(JNIEnv *env, jobject obj,jbyteArray yuvBuffer,jlong yPtr)
{
    unsigned char *pBuffer = NULL;
    pBuffer = (unsigned char *)env->GetByteArrayElements(yuvBuffer, NULL);
    if(pBuffer == NULL ) {
        LOGE("sendTextures ERROR : Can not get byteArray form JAVA!");
        return;
    }
    //workBegin();
    if(texIndex == 6)
        texIndex = 0;
    g_APUnit.updateEGLTextures(texIndex, pBuffer, *((Mat *)yPtr));
    texIndex++;
    env->ReleaseByteArrayElements(yuvBuffer, (jbyte *)pBuffer,0);
    //LOGE("updateTextures TIME COUNT");
    //workEnd();
}

JNIEXPORT void JNICALL initOpenGLES(JNIEnv *env, jobject obj,jcharArray path,jint length)
{
    jchar *array = NULL;
    char buf[255];
    int i;
    array = env->GetCharArrayElements( path, NULL);//复制数组元素到array内存空间
    if(array == NULL){
        LOGE("initOpenGLES: GetCharArrayElements error.");
    }

    memset(buf,0,length*sizeof(char));
    //开辟jboolean类型的内存空间，jboolean对应的c++类型为unsigned char
    if(buf == NULL){
        LOGE("initOpenGLES: calloc error.");
        return ;
    }
    for(i=0; i < length; i++){
        //把jcharArray的数据元素复制到buf所指的内存空间
        *(buf + i) = (char)(*(array + i));
        //LOGD("buf[%d]=%c\n",i,*(buf+i));
    }

    const char dir[] = "/data/isptune";
    const char suffix[] = "pgm";
    //getImageUnderDir(dir,suffix,buf);
    //g_APUnit.initOpenGLES(g_picVec,g_grayVec);

    env->ReleaseCharArrayElements(path, array, 0);//释放资源
    return ;
}


JNIEXPORT jlong JNICALL processing(JNIEnv *env, jobject obj)
{
    //pthread_mutex_lock( &g_mutex );
    jfieldID  nameFieldId ;
    jclass cls = env->GetObjectClass(obj);  //获得Java层该对象实例的类引用，即HelloJNI类引用
    nameFieldId = env->GetFieldID(cls ,"time", "D"); //获得属性句柄
    if(nameFieldId == NULL)
    {
        LOGE("LOGE: 没有得到 TIME 的句柄ID \n");
    }

    //g_APUnit.initOpenGLES(g_picVec,g_grayVec);
    workBegin();
    Mat outMat;
    g_APUnit.Progress(outMat);
    //outMat = g_grayVec[0];
    Mat *imgData = new Mat(outMat);
    LOGE("SUM TIME COUNT");
    workEnd();
    //sdcard/DCIM/Camera
    //imwrite("/mnt/obb/testRGB.jpg", outMat);
    env->SetDoubleField(obj,nameFieldId ,gTime); // 设置该字段的值
    return (jlong)imgData;
    //pthread_mutex_unlock( &g_mutex );
}


static const char *className = "com/example/linqi/mfdnoisy/NdkUtils";

//定义方法隐射关系
static JNINativeMethod methods[] = {
        {"sendTextures","([BJ)V",(void*)sendTextures},
        {"setTextureSize","(II)V",(void*)setTextureSize},
        {"initNDK","()V",(void*)initNDK},
        {"calHomography","(JZ)[F",(void*)calHomography},
        {"processing","()J",(void*)processing},
        {"initOpenGLES","([CI)V",(void*)initOpenGLES},
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
//声明变量
    jint result = JNI_ERR;
    JNIEnv* env = NULL;
    jclass clazz;
    int methodsLenght;

//获取JNI环境对象
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        return JNI_ERR;
    }
    assert(env != NULL);

//注册本地方法.Load 目标类
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_ERR;
    }

//建立方法隐射关系
//取得方法长度
    methodsLenght = sizeof(methods) / sizeof(methods[0]);
    if (env->RegisterNatives(clazz, methods, methodsLenght) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_ERR;
    }

    result = JNI_VERSION_1_4;
    return result;
}

jint JNI_Unload(JavaVM* vm,void* reserved)
{
    jint result = JNI_ERR;
    JNIEnv* env = NULL;

    LOGI("JNI_OnUnload!");
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        return JNI_ERR;
    }
    result = JNI_VERSION_1_4;
    return result;
}


