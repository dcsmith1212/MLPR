package com.example.derek.opencv_display;

import android.os.Bundle;
        import android.support.v7.app.AppCompatActivity;
        import android.util.Log;
        import android.view.MenuItem;
        import android.view.Surface;
        import android.view.SurfaceView;
        import android.view.WindowManager;

        import org.opencv.android.JavaCameraView;
        import org.opencv.android.BaseLoaderCallback;
        import org.opencv.android.CameraBridgeViewBase;
        import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
        import org.opencv.android.LoaderCallbackInterface;
        import org.opencv.android.OpenCVLoader;
        import org.opencv.core.Core;
        import org.opencv.core.CvType;
        import org.opencv.core.Mat;
        import org.opencv.core.Scalar;
        import org.opencv.core.Size;
        import org.opencv.imgproc.Imgproc;

        import static java.lang.Math.round;

// OpenCV Classes

public class MainActivity_show_camera extends AppCompatActivity implements CvCameraViewListener2 {

    // Used for logging success or failure messages
    private static final String TAG = "OCVSample::Activity";

    // Loads camera view of OpenCV for us to use. This lets us see using OpenCV
    private CameraBridgeViewBase mOpenCvCameraView;

    // These variables are used (at the moment) to fix camera orientation from 270degree to 0degree
    Mat mRgba;
    Mat mRgbaF;
    Mat mRgbaT;


    // For onCameraFrame function
    int rawWidth;
    int IDEAL_WIDTH = 512;
    Mat mGray;
    Mat sobel1;
    Mat sobel2;
    Mat outFrame;
    Scalar seVal = new Scalar(1);
    Mat SE1;
    double scaleFactor;
    Size newSize;
    Mat resizedMat;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");
                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    public MainActivity_show_camera() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main_show_camera);

        mOpenCvCameraView = (JavaCameraView) findViewById(R.id.show_camera_activity_java_surface_view);

        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);

        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
        System.out.println("HERE'S YER SIZE: " + width + " x " + height);

        newSize = new Size( (double)IDEAL_WIDTH, (int)round( (double)height * (double)IDEAL_WIDTH / (double)width ));
        mRgba = new Mat(height, width, CvType.CV_8UC4);
        mRgbaF = new Mat(height, width, CvType.CV_8UC4);
        mRgbaT = new Mat(width, width, CvType.CV_8UC4);
    }

    public void onCameraViewStopped() {
        mRgba.release();
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {

        // TODO Auto-generated method stub
        mRgba = inputFrame.rgba();
        /*
        // Rotate mRgba 90 degrees
        Core.transpose(mRgba, mRgbaT);
        Imgproc.resize(mRgbaT, mRgbaF, mRgbaF.size(), 0,0, 0);
        Core.flip(mRgbaF, mRgba, 1 );
        */

        // Fix this location
        SE1 = new Mat(8, 16, CvType.CV_8U, seVal);

        /*
        resizedMat = new Mat();
        rawWidth = mRgba.cols();
        if (rawWidth > IDEAL_WIDTH) {
            scaleFactor = (double)IDEAL_WIDTH / rawWidth;
            Imgproc.resize(mRgba, resizedMat, newSize, 0, 0, 0);
        }
        else scaleFactor = 1;

        System.out.println("Resized size: " + resizedMat.cols() + " x " + resizedMat.rows());

        resizedMat.convertTo(resizedMat, CvType.CV_32FC1, 1.0/255.0);
        */
        mRgba.convertTo(mRgba, CvType.CV_32FC1, 1.0/255.0);


        mGray = mRgba;
        sobel1 = mRgba;
        sobel2 = mRgba;
        outFrame = mRgba;

        Imgproc.cvtColor(mRgba, mGray, Imgproc.COLOR_BGR2GRAY);
        Imgproc.Sobel(mGray, sobel1, -1, 1, 0, 3, 1.0/4.0, 0.0);
        Imgproc.Sobel(mGray, sobel2, -1, 0, 1, 3, 1.0/4.0, 0.0);
        Core.magnitude(sobel1, sobel2, outFrame);
        Imgproc.morphologyEx(outFrame, outFrame, Imgproc.MORPH_CLOSE, SE1);
        
        outFrame.convertTo(outFrame, CvType.CV_8UC1, 255.0);
        return outFrame;
    }
}