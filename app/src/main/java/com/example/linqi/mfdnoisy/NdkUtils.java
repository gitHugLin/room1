package com.example.linqi.mfdnoisy;


public class NdkUtils {

    double time;
    NdkUtils() {
        time = 0;
    }

    public native float[] calHomography(long grayMatPtr,boolean first);
    public native void updateTexture(long bitMatPtr);
    public native boolean gray(int[] dstImage,int w,int h);
    public native long processing();
    public native void grayImage(int[][] srcImage,int[] dstImage,int width,int height);
    public native void initOpenGLES(char[] path,int length);
    public native long addPicture();

    static {
        System.loadLibrary("opencv_java");
        System.loadLibrary("AddPictureLib");   //defaultConfig.ndk.moduleName
    }
}
