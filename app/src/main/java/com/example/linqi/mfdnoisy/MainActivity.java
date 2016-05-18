package com.example.linqi.mfdnoisy;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.Button;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.List;



public class MainActivity extends Activity
        implements View.OnClickListener, SurfaceHolder.Callback,
        Camera.PictureCallback , Camera.PreviewCallback {

    public static final String TAG = MainActivity.class.getSimpleName();
    public static final String CAMERA_ID_KEY = "camera_id";
    public static final String CAMERA_FLASH_KEY = "flash_mode";
    public static final String PREVIEW_HEIGHT_KEY = "preview_height";

    private static final int PICTURE_SIZE_MAX_WIDTH = 1280;
    private static final int PREVIEW_SIZE_MAX_WIDTH = 640;


    private RecyclerView recyclerView;
    private SquareCameraPreview preview;
    private Button btnChange, btnTake, btnFlash;
    private MyRecyclerAdapter mAdapter;

    private int cameraID;
    private String flashMode;
    private Camera mCamera;
    private SurfaceHolder mHolder;

    private int mDisplayOrientation;
    private int mLayoutOrientation;

    //    private int mCoverHeight;
    private int mPreviewHeight;
    private CameraOrientationListener mOrientationListener;

    float mHomography[] = new float[9];
    static int textureIndex = 0;
    boolean isReading = false;
    NdkUtils MFDenoisy = new NdkUtils();

    public void updatePreview() {
        //mCamera.setOneShotPreviewCallback(this);
        mCamera.setPreviewCallback(this);
    }

    public void decodeToBitMap(byte[] data, Camera _camera) {
        Camera.Size size = mCamera.getParameters().getPreviewSize();
        try {
            YuvImage image = new YuvImage(data, ImageFormat.NV21, size.width,
                    size.height, null);
            Log.w("wwwwwwwww", size.width + " " + size.height);
            if (image != null) {
                ByteArrayOutputStream stream = new ByteArrayOutputStream();
                image.compressToJpeg(new Rect(0, 0, size.width, size.height),
                        100, stream);
                Bitmap bmp = BitmapFactory.decodeByteArray(stream.toByteArray(), 0, stream.size());
/*                Log.w("wwwwwwwww", bmp.getWidth() + " " + bmp.getHeight());
                Log.w("wwwwwwwww",
                        (bmp.getPixel(100, 100) & 0xff) + "  "
                                + ((bmp.getPixel(100, 100) >> 8) & 0xff) + "  "
                                + ((bmp.getPixel(100, 100) >> 16) & 0xff));*/

                stream.close();
                Mat Ychannel = new Mat();
                Utils.bitmapToMat(bmp, Ychannel);
                long matAddr = Ychannel.getNativeObjAddr();
                MFDenoisy.updateTexture(matAddr);
                Log.e("matAddr", "Ychannel.getNativeObjAddr:= " + matAddr);
            }
        } catch (Exception ex) {
            Log.e("Sys", "Error:" + ex.getMessage());
        }
    }

    int mWidth = 0;
    int mHeight = 0;
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Camera.Parameters parameters = camera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        /*int Height = size.height + size.height/2;
        int Width = size.width;*/
        //Log.i("onPreviewFrame", "PreviewSize = " + data.length );
        //Log.i("onPreviewFrame", "PreviewSize = ("+ size.width + "," + size.height +")" );
        if (data.length != 0) {
            if(!isReading) {
                //decodeToBitMap(data,camera);
                Mat YUV420SP = new Mat(mHeight, mWidth, CvType.CV_8UC1, new Scalar(0));
                YUV420SP.put(0, 0, data);
                Mat Ychannel = new Mat(size.height, size.width, CvType.CV_8UC1, new Scalar(0));
                Ychannel.put(0, 0, data);
                long yuvAddr = YUV420SP.getNativeObjAddr();
                long yAddr = Ychannel.getNativeObjAddr();
                MFDenoisy.updateTextures(yuvAddr,yAddr);
/*                if (textureIndex == 0)
                    mHomography = MFDenoisy.calHomography(yAddr, true);
                else
                    mHomography = MFDenoisy.calHomography(yAddr, false);
                textureIndex++;
                if (textureIndex == 6)
                {
                    textureIndex = 0;
                }
                for(int i = 0; i< 9; i++) {
                    Log.i("mHomography", "mHomography" +"["+i+"] = " + mHomography[i] );
                }*/
            }

            /*Camera.Parameters parameters = camera.getParameters();
            Camera.Size size = parameters.getPreviewSize();
            //Log.i("onPreviewFrame", "PreviewSize = ("+ size.width + "," + size.height +")" );
            Log.i("onPreviewFrame", "PreviewSize = " + data.length );
            //Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, size.height * size.width);
            Mat Ychannel = new Mat(size.height, size.width, CvType.CV_8UC1, new Scalar(0));
            Ychannel.put(0, 0, data);
            //Utils.bitmapToMat(bitmap, Ychannel);
            long yAddr = Ychannel.getNativeObjAddr();*/

           // Log.i("onPreviewFrame", "onPreviewFrame is running!");
        } else {
            Log.e("onPreviewFrame", "onPreviewFrame error!");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mOrientationListener = new CameraOrientationListener(this);

        if (savedInstanceState == null) {
            cameraID = getBackCameraID();
            flashMode = Camera.Parameters.FLASH_MODE_AUTO;
        } else {
            cameraID = savedInstanceState.getInt(CAMERA_ID_KEY);
            flashMode = savedInstanceState.getString(CAMERA_FLASH_KEY);
            mPreviewHeight = savedInstanceState.getInt(PREVIEW_HEIGHT_KEY);
        }

        mOrientationListener.enable();

        preview = (SquareCameraPreview) findViewById(R.id.surfaceView);
        preview.getHolder().addCallback(this);


        //btnChange = (Button) findViewById(R.id.button_change_picture);
        btnTake = (Button) findViewById(R.id.button_take_picture);
        //btnFlash = (Button) findViewById(R.id.button_flash_picture);

        //btnChange.setOnClickListener(this);
        btnTake.setOnClickListener(this);
        //btnFlash.setOnClickListener(this);


        recyclerView = (RecyclerView) findViewById(R.id.recyclerView);
        recyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false));

        mAdapter = new MyRecyclerAdapter(this);

        recyclerView.setAdapter(mAdapter);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putInt(CAMERA_ID_KEY, cameraID);
        outState.putString(CAMERA_FLASH_KEY, flashMode);
        outState.putInt(PREVIEW_HEIGHT_KEY, mPreviewHeight);
        super.onSaveInstanceState(outState);
    }

    private void getCamera(int cameraID) {
        Log.d(TAG, "get camera with id " + cameraID);
        try {
            mCamera = Camera.open(cameraID);
            preview.setCamera(mCamera);
        } catch (Exception e) {
            Log.e(TAG, "Can't open camera with id " + cameraID);
            e.printStackTrace();
        }
        //mCamera.getParameters().setPreviewFormat(ImageFormat.JPEG);
    }

    /**
     * Start the camera preview
     */
    private void startCameraPreview() {
        determineDisplayOrientation();
        setupCamera();

        try {
            mCamera.setPreviewDisplay(mHolder);
            mCamera.startPreview();
        } catch (IOException e) {
            Log.d(TAG, "Can't start camera preview due to IOException " + e);
            e.printStackTrace();
        }
        mCamera.setPreviewCallback(this);
        Camera.Parameters parameters = mCamera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        mHeight = size.height + size.height/2;
        mWidth = size.width;
    }

    /**
     * Stop the camera preview
     */
    private void stopCameraPreview() {
        // Nulls out callbacks, stops face detection
        mCamera.stopPreview();
        preview.setCamera(null);
    }

    /**
     * Determine the current display orientation and rotate the camera preview
     * accordingly
     */
    private void determineDisplayOrientation() {
        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraID, cameraInfo);

        int rotation = getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;

        switch (rotation) {
            case Surface.ROTATION_0: {
                degrees = 0;
                break;
            }
            case Surface.ROTATION_90: {
                degrees = 90;
                break;
            }
            case Surface.ROTATION_180: {
                degrees = 180;
                break;
            }
            case Surface.ROTATION_270: {
                degrees = 270;
                break;
            }
        }

        int displayOrientation;

        // Camera direction
        if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            // Orientation is angle of rotation when facing the camera for
            // the camera image to match the natural orientation of the device
            displayOrientation = (cameraInfo.orientation + degrees) % 360;
            displayOrientation = (360 - displayOrientation) % 360;
        } else {
            displayOrientation = (cameraInfo.orientation - degrees + 360) % 360;
        }

        mDisplayOrientation = (cameraInfo.orientation - degrees + 360) % 360;
        mLayoutOrientation = degrees;

        mCamera.setDisplayOrientation(displayOrientation);
    }


    /**
     * Setup the camera parameters
     */
    private void setupCamera() {
        // Never keep a global parameters
        Camera.Parameters parameters = mCamera.getParameters();

        Camera.Size bestPreviewSize = determineBestPreviewSize(parameters);
        Camera.Size bestPictureSize = determineBestPictureSize(parameters);

        //parameters.setPreviewSize(3264, 2448);
        parameters.setPreviewSize(bestPreviewSize.width, bestPreviewSize.height);
        parameters.setPictureSize(bestPictureSize.width, bestPictureSize.height);


        // Set continuous picture focus, if it's supported
        if (parameters.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        }

        //final Button btnFlashMode = (Button) findViewById(R.id.button_flash_picture);
        //List<String> flashModes = parameters.getSupportedFlashModes();
        //if (flashModes != null && flashModes.contains(flashMode)) {
            //parameters.setFlashMode(flashMode);
            //btnFlashMode.setVisibility(View.VISIBLE);
        //} else {
            //btnFlashMode.setVisibility(View.INVISIBLE);
        //}

        // Lock in the changes
        mCamera.setParameters(parameters);
    }

    private Camera.Size determineBestPreviewSize(Camera.Parameters parameters) {
        return determineBestSize(parameters.getSupportedPreviewSizes(), PREVIEW_SIZE_MAX_WIDTH);
    }

    private Camera.Size determineBestPictureSize(Camera.Parameters parameters) {
        return determineBestSize(parameters.getSupportedPictureSizes(), PICTURE_SIZE_MAX_WIDTH);
    }

    private Camera.Size determineBestSize(List<Camera.Size> sizes, int widthThreshold) {
        Camera.Size bestSize = null;
        Camera.Size size;
        int numOfSizes = sizes.size();
        for (int i = 0; i < numOfSizes; i++) {
            size = sizes.get(i);
            boolean isDesireRatio = (size.width / 4) == (size.height / 3);
            boolean isBetterSize = (bestSize == null) || size.width > bestSize.width;

            if (isDesireRatio && isBetterSize) {
                bestSize = size;
            }
        }

        if (bestSize == null) {
            Log.d(TAG, "cannot find the best camera size");
            return sizes.get(sizes.size() - 1);
        }

        return bestSize;
    }

    private void restartPreview() {
        stopCameraPreview();
        mCamera.release();

        getCamera(cameraID);
        startCameraPreview();
    }

    private int getFrontCameraID() {
        PackageManager pm = getPackageManager();
        if (pm.hasSystemFeature(PackageManager.FEATURE_CAMERA_FRONT)) {
            return Camera.CameraInfo.CAMERA_FACING_FRONT;
        }

        return getBackCameraID();
    }

    private int getBackCameraID() {
        return Camera.CameraInfo.CAMERA_FACING_BACK;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        //if (id == R.id.button_change_picture) {
            //if (cameraID == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                //cameraID = getBackCameraID();
            //} else {
                //cameraID = getFrontCameraID();
            //}
            //restartPreview();
        //}
        if (id == R.id.button_take_picture) {
            isReading = true;
            Bitmap mFinalBitmap = Bitmap.createBitmap(1280 , 960,Bitmap.Config.ARGB_8888);
            long address = MFDenoisy.processing();
            Mat outMat = new Mat(address);
            Utils.matToBitmap(outMat, mFinalBitmap); //convert mat to bitmap
            savePicture( mFinalBitmap);
            isReading = false;
            //takePicture();
        }/* else if (id == R.id.button_flash_picture) {
            if (flashMode.equalsIgnoreCase(Camera.Parameters.FLASH_MODE_AUTO)) {
                flashMode = Camera.Parameters.FLASH_MODE_ON;
                btnFlash.setText(getResources().getString(R.string.flash_on));

            } else if (flashMode.equalsIgnoreCase(Camera.Parameters.FLASH_MODE_ON)) {
                flashMode = Camera.Parameters.FLASH_MODE_OFF;
                btnFlash.setText(getResources().getString(R.string.flash_off));

            } else if (flashMode.equalsIgnoreCase(Camera.Parameters.FLASH_MODE_OFF)) {
                flashMode = Camera.Parameters.FLASH_MODE_AUTO;
                btnFlash.setText(getResources().getString(R.string.flash_auto));

            }
            setupCamera();
        }*/
    }

    private void takePicture() {
        mOrientationListener.rememberOrientation();

        // Shutter callback occurs after the image is captured. This can
        // be used to trigger a sound to let the user know that image is taken
        Camera.ShutterCallback shutterCallback = null;

        // Raw callback occurs when the raw image data is available
        Camera.PictureCallback raw = null;

        // postView callback occurs when a scaled, fully processed
        // postView image is available.
        Camera.PictureCallback postView = null;

        // jpeg callback occurs when the compressed image is available
        mCamera.takePicture(shutterCallback, raw, postView, this);
    }

    @Override
    public void onStop() {
        mOrientationListener.disable();

        // stop the preview
        stopCameraPreview();
        mCamera.release();
        super.onStop();
    }

    public void removeFile(String path){
        File f = new File(path);
        if(f.exists()){
            f.delete();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mHolder = holder;

        getCamera(cameraID);
        startCameraPreview();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

    @Override
    public void onPictureTaken(byte[] data, Camera camera) {
        int rotation = (
                mDisplayOrientation
                        + mOrientationListener.getRememberedNormalOrientation()
                        + mLayoutOrientation
        ) % 360;

        savePicture(rotatePicture(rotation, data));
    }

    private Bitmap rotatePicture(int rotation, byte[] data) {
        Bitmap bitmap = ImageUtility.decodeSampledBitmapFromByte(this, data);
//        Log.d(TAG, "original bitmap width " + bitmap.getWidth() + " height " + bitmap.getHeight());
        if (rotation != 0) {
            Bitmap oldBitmap = bitmap;

            Matrix matrix = new Matrix();
            matrix.postRotate(rotation);

            bitmap = Bitmap.createBitmap(
                    oldBitmap, 0, 0, oldBitmap.getWidth(), oldBitmap.getHeight(), matrix, false
            );

            oldBitmap.recycle();
        }
//        photoImageView.setImageBitmap(bitmap);
        return bitmap;
    }

    private void savePicture(Bitmap bitmap) {
//        ImageView photoImageView = (ImageView) getView().findViewById(R.id.photo);
//        Bitmap bitmap = ((BitmapDrawable) photoImageView.getDrawable()).getBitmap();
        Uri photoUri = ImageUtility.savePicture(this, bitmap);
        mAdapter.add(photoUri.getPath(), mAdapter.getItemCount());
        recyclerView.smoothScrollToPosition(mAdapter.getItemCount()-1);

//        ((CameraActivity) getActivity()).returnPhotoUri(photoUri);
    }


    /**
     * When orientation changes, onOrientationChanged(int) of the listener will be called
     * Reference : https://github.com/boxme/SquareCamera
     */
    private static class CameraOrientationListener extends OrientationEventListener {

        private int mCurrentNormalizedOrientation;
        private int mRememberedNormalOrientation;

        public CameraOrientationListener(Context context) {
            super(context, SensorManager.SENSOR_DELAY_NORMAL);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            if (orientation != ORIENTATION_UNKNOWN) {
                mCurrentNormalizedOrientation = normalize(orientation);
            }
        }

        private int normalize(int degrees) {
            if (degrees > 315 || degrees <= 45) {
                return 0;
            }

            if (degrees > 45 && degrees <= 135) {
                return 90;
            }

            if (degrees > 135 && degrees <= 225) {
                return 180;
            }

            if (degrees > 225 && degrees <= 315) {
                return 270;
            }

            throw new RuntimeException("The physics as we know them are no more. Watch out for anomalies.");
        }

        public void rememberOrientation() {
            mRememberedNormalOrientation = mCurrentNormalizedOrientation;
        }

        public int getRememberedNormalOrientation() {
            return mRememberedNormalOrientation;
        }
    }

}