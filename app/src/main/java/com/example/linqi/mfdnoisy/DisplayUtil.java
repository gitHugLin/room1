package com.example.linqi.mfdnoisy;

import android.content.Context;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.view.WindowManager;

/**
 * Created by linqi on 16-5-25.
 */
public class DisplayUtil {
    public static Point getScreenMetrics(Context context){
        final DisplayMetrics displayMetrics = new DisplayMetrics();
        ((WindowManager)context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getMetrics(displayMetrics);
        return new Point(displayMetrics.widthPixels,displayMetrics.heightPixels);
    }
    public static int dip2px(Context context, float dipValue){
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(dipValue * scale + 0.5f);
    }
    public static int px2dip(Context context, float pxValue){
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(pxValue / scale + 0.5f);
    }
    public static float getScreenRate(Context context){
        Point P = getScreenMetrics(context);
        float H = P.y;
        float W = P.x;
        return (H/W);
    }
	/*public static Point getScreenMetrics(Context context){
		DisplayMetrics dm =context.getResources().getDisplayMetrics();
		int w_screen = dm.widthPixels;
		int h_screen = dm.heightPixels;
		return new Point(w_screen, h_screen);
	}*/
    /**
     * Generate the center rectangle of the picture using dip dimension
     * @param w
     * @param h
     * @return
     */
    public static Point createCenterPictureRect(int w, int h,int wScreen,int hScreen,int wSavePicture,int hSavePicture){

        float wRate = (float)(wSavePicture) / (float)(wScreen);
        float hRate = (float)(hSavePicture) / (float)(hScreen);
        float rate = (wRate <= hRate) ? wRate : hRate;//you can use the minimum ratio

        int wRectPicture = (int)( w * wRate);
        int hRectPicture = (int)( h * hRate);
        return new Point(wRectPicture, hRectPicture);

    }
    private Rect calculateMaskRect(int w, int h, int wScreen, int hScreen, int wSavePicture, int hSavePicture){

        float wRate = (float)(wScreen)/(float)(wSavePicture);
        float hRate = (float)(hScreen)/(float)(hSavePicture);
        // calculate the dip size of rectangle to show
        int x1 = (int)(wScreen/ 2 - wRate*w / 2);
        int y1 = (int)(hScreen/ 2 - hRate*h / 2);
        int x2 = (int)(x1 + wRate*w);
        int y2 = (int)(y1 + hRate*h);
        return new Rect(x1, y1, x2, y2);
    }
}
