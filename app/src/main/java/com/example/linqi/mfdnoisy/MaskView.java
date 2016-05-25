package com.example.linqi.mfdnoisy;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.FrameLayout;
import android.widget.ImageView;

/**
 * Created by linqi on 16-5-25.
 */
public class MaskView extends ImageView {
    private static final String TAG = "MaskView";
    private Paint mLinePaint;
    private Paint mAreaPaint;
    private Rect mCenterRect = null;
    private Context mContext;

    public MaskView(Context context){
        super(context);
        FrameLayout.LayoutParams params= new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT);
        this.setLayoutParams(params);
        mContext = context;
        widthScreen = DisplayUtil.getScreenMetrics(mContext).x;
        heightScreen = DisplayUtil.getScreenMetrics(mContext).y;
    }

    public MaskView(Context context, AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
        initPaint();
        mContext = context;
        widthScreen = DisplayUtil.getScreenMetrics(mContext).x;
        heightScreen = DisplayUtil.getScreenMetrics(mContext).y;
    }

    private void initPaint(){
        //draw transparent area outline in the center
        mLinePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mLinePaint.setColor(Color.RED);
        mLinePaint.setStyle(Paint.Style.STROKE);
        mLinePaint.setStrokeWidth(2f);
        //mLinePaint.setAlpha(255);

    }
    /*private float startX, startY, stopX, stopY;
    private Rect createCenterScreenRect(int w, int h){
        int x1 = DisplayUtil.getScreenMetrics(this).x / 2 - w / 2;
        int y1 = DisplayUtil.getScreenMetrics(this).y / 2 - h / 2;
        int x2 = x1 + w;
        int y2 = y1 + h;
        return new Rect(x1, y1, x2, y2);
    }*/

    //public Rect(float startX, float startY, float stopX, float stopY)
/*    {

        //added comments
        this.startX = startX;
        this.startY = startY;
        this.stopX = stopX;
        this.stopY = stopY;
    }*/
    public void setCenterRect(Rect r){
        Log.d(TAG, "setCenterRect w:"+r.width()+" h:"+r.height());
        this.mCenterRect = r;
        postInvalidate();
    }
    public void clearCenterRect(Rect r){
        this.mCenterRect = null;
    }

    int widthScreen, heightScreen;
    @Override
    protected void onDraw(Canvas canvas) {
        // TODO Auto-generated method stub
        Log.i(TAG, "onDraw...");
        //if(mCenterRect == null)
            //return;

        //draw the transparent area
        canvas.drawRect(mCenterRect, mLinePaint);
        /*Paint paint = new Paint();
        paint.setStyle(Paint.Style.STROKE);
        // 消除锯齿
        paint.setAntiAlias(true);
        // 设置画笔的颜色
        paint.setColor(Color.RED);
        // 设置paint的外框宽度
        paint.setStrokeWidth(2);
        canvas.drawCircle(1000, 1000, 100, paint);*/
        super.onDraw(canvas);
    }

}