package com.example.linqi.mfdnoisy;


public class NdkUtils {

    double time;
    NdkUtils() {
        initNDK();
        time = 0;
    }

    public native void initNDK();
    public native void setTextureSize(int width,int height);
    public native float[] calHomography(long grayMatPtr,boolean first);
    public native void updateTextures(long yuvPtr,long yPtr);
    public native boolean gray(int[] dstImage,int w,int h);
    public native long processing();
    public native void grayImage(int[][] srcImage,int[] dstImage,int width,int height);
    public native void initOpenGLES(char[] path,int length);
    public native long addPicture();

    static {
        System.loadLibrary("opencv_java3");
        System.loadLibrary("AddPictureLib");   //defaultConfig.ndk.moduleName
    }
}
