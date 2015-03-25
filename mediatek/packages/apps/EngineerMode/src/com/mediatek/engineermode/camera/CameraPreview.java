/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.camera;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.view.Display;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/** Camera preview activity for capturing different type image. */
public class CameraPreview extends Activity implements SurfaceHolder.Callback {

    private int mMode;
    private boolean mIssdcardExist = false;

    private static final int PREVIEW_WIDTH = 640;
    private static final int PREVIEW_HIGTH = 480;
    private static final int PICTURE_8M_WID = 3264;
    private static final int PICTURE_8M_HIG = 2448;
    public static final int CAPTURE_ID = Menu.FIRST;
    private static final String TAG = "EM/Camera";
    private static final String AUTO_STR = "0";
    private Camera mCamera;
    private Camera.Parameters mCameraParam;
    private int mIsPreviewing = 0;
    private int mIsFocused = 0;
    private int mIsCapture = 0;

    private final AutoFocusCallback mAutoFocCalback = new AutoFocusCallback();
    private final ShutterCallback mShutterCalback = new ShutterCallback();
    private final RawPictureCallback mRawPicCalback = new RawPictureCallback();

    private StorageManager mStorageManager = null;
    private String mImgBucketName = "";
    private String mModeName = "";
    private String mIsoName = "ISO";
    private int mAFEngMode = 0;
    private String mISO = AUTO_STR;

    private static final int DIALOG_PROGRESS = 1000;
    private final ProgressDlgHandler mProgressDlgHdl = new ProgressDlgHandler();
    private static final int FULL_SCAN_START = 100;
    private static final int FULL_SCAN_COMPLET = 101;
    private static final int COMPLETE_CAPTURE = 102;
    private static final int START_CAPTURE = 103;
    private static final int WAIT_FOCUES = 104;
    private static final int FOCUES_COMPLETE = 105;
    private static final int MSG_AF_MODE1 = 1001;
    private static final int MSG_AF_MODE3 = 1003;
    private static final int MSG_AF_MODE4 = 1004;
    private static final int MSG_AF_MODE5 = 1005;
    private static final int MSG_RENEW_SHOTNUM = 1006;
    private static final int MSG_REPEAT_COMPLET = 1007;
    private static final int SLEEP_TIME = 200;
    private static final int SLEEP_TIME1 = 2000;
    private static final int SLEEP_TIME_100 = 100;
    private static final int SLEEP_TIMES = 20;
    private static final int JPEG_CAPTURE_MODE = 3;
    private static final int FINAL_ROTATION_0 = 0;
    private static final int FINAL_ROTATION_90 = 90;
    private static final int FINAL_ROTATION_180 = 180;
    private static final int FINAL_ROTATION_270 = 270;
    private static final int ROTATION_360 = 360;
    private static final int POSITION_VALUE = 50;
    private static final String AF_MANUAL = "manual";
    private static final String AF_FULLSCAN = "fullscan";
    private static final String AF_AUTO = "auto";
    private static final int ITEM_MODE_3 = 3;
    private static final int ITEM_MODE_4 = 4;
    private static final int ITEM_MODE_5 = 5;
    private static final int MGIC_NUM_24 = 24;
    private final int mCameraId = android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK;
    private boolean mIsTest = false;
    private boolean mProgresDlgExist = false;

    private boolean mIsRawCapture = false;
    private String mRawCptFileName;

    private boolean mIsOnPause = false;
    private int mPos = 0;
    /** jump steps each shot. */
    private int mStep = 1;
    /** used for AF mode five. */
    private int mStage = 0;
    private TextView mShotNum;
    private AFMode3Thread mMode3Thread;
    private AFMode4Thread mMode4Thread;
    private AFMode5Thread mMode5Thread;
    public static boolean sCanBack = true;

    private Button mCaptureBtn;

    private final Rect mPreviewRect = new Rect();
    /** avoid waiting for when first time auto focus. */
    private boolean mFocusFlag = true;
    /** This handles everything about focus. */
    private FocusManager mFocusManager;
    private boolean mAeLockSupported;
    private boolean mAwbLockSupported;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (!judgeSdcard()) {
                return;
            }
            switch (msg.what) {
                case MSG_AF_MODE1:
                    AFMode1Thread mode1Thread = new AFMode1Thread();
                    mode1Thread.start();
                    break;
                case MSG_AF_MODE3:
                    mMode3Thread = new AFMode3Thread();
                    mMode3Thread.start();
                    break;
                case MSG_AF_MODE4:
                    mMode4Thread = new AFMode4Thread();
                    mMode4Thread.start();
                    break;
                case MSG_AF_MODE5:
                    mMode5Thread = new AFMode5Thread();
                    mMode5Thread.start();
                    break;
                case MSG_RENEW_SHOTNUM:
                    onHandleShotNum();
                    break;
                case MSG_REPEAT_COMPLET:
                    mShotNum.setText("");
                    mShotNum.setVisibility(View.GONE);
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mStorageManager == null) {
            mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        }
        mImgBucketName = StorageManagerEx.getDefaultPath() + "/DCIM/CameraEM/";
        Elog.v(TAG, "onCreate start...path is " + mImgBucketName);

        mFocusManager = new FocusManager();

        setContentView(R.layout.camera_preview);
        Elog.v(TAG, "onCreate start after setContentView");

        mCaptureBtn = (Button) findViewById(R.id.capture_btn);
        mCaptureBtn.setOnTouchListener(new View.OnTouchListener() {

            public boolean onTouch(View view, MotionEvent event) {

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    Elog.v(TAG, "mCaptureBtn key down!");
                    if (mIsTest) {
                        return false;
                    }
                }
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    Elog.v(TAG, "mCaptureBtn key up!");

                    mProgressDlgHdl.sendEmptyMessage(WAIT_FOCUES);

                }
                return false;
            }
        });
        Elog.v(TAG, "onCreate end");
        final Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            final File file = new File(mImgBucketName);
            try {
                if (!file.isDirectory()) {
                    file.mkdirs();
                }
            } catch (SecurityException e) {
                Elog.v(TAG, "create direct error");
            }
            mIssdcardExist = true;
        } else {
            mIssdcardExist = false;
        }

        if (!mIssdcardExist) {
            Toast.makeText(this, R.string.sdcard_error_tip, Toast.LENGTH_LONG).show();
            this.finish();
            return;
        }
        mShotNum = (TextView) findViewById(R.id.current_shot_num);
        mShotNum.setVisibility(View.GONE);
    }

    @Override
    public void onResume() {
        super.onResume();
        Elog.v(TAG, "onResume ");
        if (mProgresDlgExist) {
            return;
        }
        if (mStorageManager == null) {
            mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        }
        mImgBucketName = StorageManagerEx.getDefaultPath() + "/DCIM/CameraEM/";
        Elog.v(TAG, "onResume...path is " + mImgBucketName);
        mIsPreviewing = 0;
        mIsFocused = 0;
        mIsCapture = 0;

        VideoPreview mVideoPreview = (VideoPreview) findViewById(R.id.camera_preview);
        mVideoPreview.setAspectRatio(PREVIEW_WIDTH, PREVIEW_HIGTH);
        final SurfaceHolder holder = mVideoPreview.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        mIsTest = false;
        mIsOnPause = false;
    }

    @Override
    public void onPause() {
        Elog.v(TAG, "super onPause.");
        if (mProgresDlgExist) {
            // mProgressDlgExists = false;
            mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
            // return;
        }
        mIsOnPause = true;
        if (1 == mIsPreviewing) {
            stopPreview();
        }
        closeCamera();
        mIsPreviewing = 0;
        mIsOnPause = false;
        this.finish();
        Elog.v(TAG, "super onPause end.");
        super.onPause();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_PROGRESS) {
            final ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getResources().getString(R.string.full_scan_tip));
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setCancelable(false);
            return dialog;
        }
        return null;
    }

    private void onHandleShotNum() {
        switch (mMode) {
            case 1:
                mShotNum.setText("Mode 1:" + (mPos + 1) + "/50");
                break;
            case 2:
                break;
            case ITEM_MODE_3:
                mShotNum.setText("Mode 3:" + (mPos + 1) + "/50");
                break;
            case ITEM_MODE_4:
                mShotNum.setText("Mode 4:" + (mPos + 1) + "/50");
                break;
            case ITEM_MODE_5:
                if (mStage == 2) {
                    mShotNum.setText("Mode 5_Full:" + (mPos + 1) + "/50");
                } else if (mStage == 1) {
                    mShotNum.setText("Mode 5_Auto:" + (mPos + 1) + "/50");
                }
                break;
            default:
                break;
        }
    }

    // remove from mt6589
    // private void setFocusRectangle(int isDraw) {
    // if (mCameraParam == null) {
    // return;
    // }
    // Elog.v(TAG, "startPreview()mCameraParam.setFocusDrawMode(1)");
    // // mCameraParam.setFocusDrawMode(isDraw);
    // mCameraParam.setFocusDrawMode(isDraw);
    // updatePreviewRectToCamera();
    // if (mCamera == null || mCameraParam == null || mIsOnPause) {
    // return;
    // }
    // mCamera.setParameters(mCameraParam);
    // }
    //
    // private void updatePreviewRectToCamera() {
    // // get preview offset and update
    // View preview = findViewById(R.id.camera_preview);
    // final int[] loc = new int[2];
    // preview.getLocationOnScreen(loc);
    // mPreviewRect.set(loc[0], loc[1], loc[0] + preview.getWidth(), loc[1] +
    // preview.getHeight());
    // // for FD
    // Elog.v(TAG, "preview: " + rectToCameraString(mPreviewRect));
    // if (mCameraParam == null || mIsOnPause) {
    // return;
    // }
    // mCameraParam.setDisplayRegion(mPreviewRect.left, mPreviewRect.top,
    // mPreviewRect.width(), mPreviewRect.height(),
    // getWindowManager().getDefaultDisplay().getRotation());
    // }

    private String rectToCameraString(Rect rect) {
        final StringBuilder stringBuilder = new StringBuilder(32);

        stringBuilder.append(rect.left);
        stringBuilder.append("x");
        stringBuilder.append(rect.top);
        stringBuilder.append("x");
        stringBuilder.append(rect.right);
        stringBuilder.append("x");
        stringBuilder.append(rect.bottom);
        return stringBuilder.toString();
    }

    private class ProgressDlgHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case FULL_SCAN_START:
                    showDialog(DIALOG_PROGRESS);
                    mProgresDlgExist = true;
                    break;
                case FULL_SCAN_COMPLET:
                    dismissDialog(DIALOG_PROGRESS);
                    mProgresDlgExist = false;
                    break;
                case COMPLETE_CAPTURE:
                    mCaptureBtn.setEnabled(true);
                    Elog.v(TAG, "Enabled mCaptureBtn");
                    Elog.v(TAG, "After Enabled mCaptureBtn mFocusFlag = " + mFocusFlag);
                    break;
                case START_CAPTURE:
                    mCaptureBtn.setEnabled(false);
                    Elog.v(TAG, "Disabled mCaptureBtn");
                    break;
                case WAIT_FOCUES:
                    Elog.v(TAG, "EVENT_WAIT_FOCUES");
                    new WaitFocusThread().start();
                    break;
                case FOCUES_COMPLETE:
                    Elog.v(TAG, "After while mFocusFlag = " + mFocusFlag);
                    capturePicture();
                    mFocusFlag = false;
                    Elog.v(TAG, "After CapturePicture = " + mFocusFlag);
                    mProgressDlgHdl.sendEmptyMessage(START_CAPTURE);
                    Elog.v(TAG, "EVENT_FOCUES_COMPLETE");
                    break;
                default:
                    break;
            }
        }
    }

    class WaitFocusThread extends Thread {

        @Override
        public void run() {
            int sleepTimes = 0;
            Elog.v(TAG, "Before while mFocusFlag = " + mFocusFlag);
            while (!mFocusFlag) {
                Elog.v(TAG, "Waiting for focus!");
                mySleep(SLEEP_TIME);
                sleepTimes++;
                if (sleepTimes >= SLEEP_TIMES) {
                    break;
                }
                Elog.v(TAG, "Waiting for focus! i = " + sleepTimes);
            }
            mProgressDlgHdl.sendEmptyMessage(FOCUES_COMPLETE);
            Elog.v(TAG, "WaitFocusThread");
        }

    }

    /**
     * camera preview surface changed.
     * 
     * @param holder
     *            : surface holder, used to handle surface
     * @param format
     *            : format
     * @param w
     *            : weight of surface
     * @param h
     *            : height of surface
     */
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        if (mProgresDlgExist) {
            return;
        }
        // mDispW = w;
        // mDispH = h;
        Elog.v(TAG, "surfaceChanged width is : " + w);
        Elog.v(TAG, "surfaceChanged height is : " + h);

        // mCameraParam = mCamera.getParameters();
        // mCameraParam.set("focus-mode", AF_AUTO);
        // mCamera.setParameters(mCameraParam);
        Elog.v(TAG, "before startPreview ");
        initializeCapabilities();
        startPreview();

        Elog.v(TAG, "after startPreview ");
    }

    /**
     * used to handle when the surface first create
     * 
     * @param holder
     *            : surface holder, used to handle surface
     */
    public void surfaceCreated(SurfaceHolder holder) {
        Elog.v(TAG, "surfaceCreated start");
        if (mProgresDlgExist) {
            return;
        }
        try {
            openCamera();
        } catch (CameraHardwareException e) {
            showErrorAndFinish(this, R.string.cannot_connect_camera);
        }
        try {
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setPreviewDisplay(holder);
            setDisplayOrientation();
        } catch (IOException exception) {
            closeCamera();
            Elog.v(TAG, "surfaceCreated closeCamera ");
        }
        Elog.v(TAG, "surfaceCreated end");
    }

    /**
     * used to handle when the surface destoryed
     * 
     * @param arg0
     *            : surface holder, used to handle surface
     */
    public void surfaceDestroyed(SurfaceHolder arg0) {
        Elog.v(TAG, "enter surfaceDestroyed ");
        if (mProgresDlgExist) {
            mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
            // mProgressDlgExists = false;
            // return;
        }
        stopPreview();
        closeCamera();
        Elog.v(TAG, "surfaceDestroyed closeCamera ");
    }

    private void openCamera() throws CameraHardwareException {
        if (mCamera == null && !mIsOnPause) {
            // try {
            mCamera = Camera.open();
            // } catch (RuntimeException e) {
            // Elog.e(TAG, "fail to connect Camera.");
            // throw new CameraHardwareException(e);
            // }
            if (mCamera != null) {
                mCameraParam = mCamera.getParameters();
            }
            final Intent intent = getIntent();
            mISO = intent.getStringExtra("ISO");
            if (TextUtils.isEmpty(mISO)) {
                mISO = AUTO_STR;
            }
            Elog.v(TAG, "intent mISO = " + mISO);

            if (AUTO_STR.equals(mISO) || "1600".equals(mISO)) {
                mIsoName = mIsoName + mISO;
            } else {
                mIsoName = mIsoName + "0" + mISO;
            }
            Elog.v(TAG, "Enter openCamera to init the mCamera.");
            if (null == mCamera) {
                Elog.v(TAG, "init the mCamera is null.");
                showErrorAndFinish(this, R.string.cannot_connect_camera);
            }
        }
    }

    private boolean judgedSupportedSize(int width, int height) {
        List<Size> supprortSizeList = mCameraParam.getSupportedPictureSizes();
        if (supprortSizeList != null && !supprortSizeList.isEmpty()) {
            for (Size size : supprortSizeList) {
                if (size.width == width && size.height == height) {
                    return true;
                }
            }
        }
        return false;
    }

    private void closeCamera() {
        Elog.v(TAG, "closeCamera() start!");
        if (null != mCamera) {
            mCamera.cancelAutoFocus();
            mCamera.setZoomChangeListener(null);
            mCamera.release();
            mCamera = null;
        }
        Elog.v(TAG, "closeCamera() end!");
    }

    private void startPreview() {
        Elog.v(TAG, "startPreview() start!");
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCameraParam = mCamera.getParameters();
        if (mCameraParam == null) {
            return;
        }
        // Unlock AE and AWB.
        setAeAwlock(false);
        setDisplayOrientation();
        mCameraParam.set("fps-mode", 0); // Frame rate is normal
        mCameraParam.set("mtk-cam-mode", 0); // Cam mode is preview
        Intent intent = getIntent();
        int rawCaptureMode = intent.getIntExtra("RawCaptureMode", 1);
        Elog.v(TAG, "intent get Raw capture mode is " + rawCaptureMode);
        int rawCaptureType = intent.getIntExtra("RawType", 0);
        Elog.v(TAG, "intent get Raw Type  is " + rawCaptureType);
        String antiFlicker = intent.getStringExtra("AntiFlicker");
        if (TextUtils.isEmpty(antiFlicker)) {
            antiFlicker = "50";
        }
        Elog.v(TAG, "intent get antiFlicker = " + antiFlicker);
        mCameraParam.setAntibanding(antiFlicker);
        mMode = intent.getIntExtra("AFMode", -1);
        mStep = intent.getIntExtra("AFStep", 1);
        Elog.v(TAG, "The value of AFMode is :" + mMode);
        Elog.v(TAG, "The value of AFStep is :" + mStep);
        handleRawType(rawCaptureMode, rawCaptureType);

        Size size = mCameraParam.getPictureSize();
        if (size == null) {
            return;
        }
        Elog.v(TAG, "Picturesize.width is " + size.width);
        Elog.v(TAG, "Picturesize.height is " + size.height);
        PreviewFrameLayout frameLayout = (PreviewFrameLayout) findViewById(R.id.frame_layout);
        frameLayout.setAspectRatio((double) size.width / size.height);
        List<Size> sizes = mCameraParam.getSupportedPreviewSizes();
        Size optimalSize = null;
        if (size.height != 0) {
            optimalSize = getOptimalPreviewSize(sizes, (double) size.width / size.height);
        }
        if (optimalSize != null) {
            Elog.v(TAG, "optimalSize.width is " + optimalSize.width);
            Elog.v(TAG, "optimalSize.height is " + optimalSize.height);
            mCameraParam.setPreviewSize(optimalSize.width, optimalSize.height);
        }
        if (judgedSupportedSize(PICTURE_8M_WID, PICTURE_8M_HIG)) {
            Elog.v(TAG, "Support 8M picture size!");
            mCameraParam.setPictureSize(PICTURE_8M_WID, PICTURE_8M_HIG);
        }
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        // remove from mt6589
        // int isDraw = 1;
        // setFocusRectangle(isDraw);
        if (mCamera == null || mIsOnPause) {
            return;
        }
        // try {
        mCamera.startPreview();
        // } catch (RuntimeException ex) {
        // closeCamera();
        // showErrorAndFinish(this, R.string.start_preview_failed);
        // return;
        // }
        mIsPreviewing = 1;
        Elog.v(TAG, "startPreview() end!");
    }

    private void setAeAwlock(boolean locked) {
        mFocusManager.setAeAwbLock(locked);
        if (mAeLockSupported) {
            mCameraParam.setAutoExposureLock(mFocusManager.getAeAwbLock());
            Elog.v(TAG, "mFocusManager.getAeAwbLock() is " + mFocusManager.getAeAwbLock());
        }
        if (mAwbLockSupported) {
            mCameraParam.setAutoWhiteBalanceLock(mFocusManager.getAeAwbLock());
            Elog.v(TAG, "mFocusManager.getAeAwbLock() is " + mFocusManager.getAeAwbLock());
        }
    }

    private void handleRawType(int rawCaptureMode, int rawCaptureType) {
        if (rawCaptureMode != JPEG_CAPTURE_MODE) { // not JPEG Only
            mIsRawCapture = true;
            if (rawCaptureMode == 1) {
                mModeName = "Preview";
            } else if (rawCaptureMode == 2) {
                mModeName = "Image";
            }
            mCameraParam.set("rawsave-mode", rawCaptureMode);
            mCameraParam.set("isp-mode", rawCaptureType);
            mCameraParam.setISOSpeed(mISO);
            Elog.v(TAG, "Set iso speed is " + mISO);
            long dateTaken = System.currentTimeMillis();
            mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
            mCameraParam.set("rawfname", mRawCptFileName + ".raw");
            Elog.v(TAG, "Set raw name success!");
        }
    }

    private void initializeCapabilities() {
        Camera.Parameters mInitialParams = mCamera.getParameters();
        mFocusManager.initializeParameters(mInitialParams);
        mAeLockSupported = mInitialParams.isAutoExposureLockSupported();
        mAwbLockSupported = mInitialParams.isAutoWhiteBalanceLockSupported();
    }

    private void setDisplayOrientation() {
        int mDisplayRotation = getDisplayRotation(this);
        Elog.v(TAG, "setDisplayOrientation() mDisplayRotation is !" + mDisplayRotation);
        int mDisplayOrientation = getDisplayOrientation(mDisplayRotation, mCameraId);
        Elog.v(TAG, "setDisplayOrientation() mDisplayOrientation is !" + mDisplayOrientation);
        mCamera.setDisplayOrientation(mDisplayOrientation);

    }

    /**
     * Get the current camera preview's rotation
     * 
     * @Param activity: this activity
     * @return rotation
     */
    public int getDisplayRotation(Activity activity) {
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        Elog.v(TAG, "getDisplayRotation() rotation is !" + rotation);
        int finalRotation = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                finalRotation = FINAL_ROTATION_0;
                break;
            case Surface.ROTATION_90:
                finalRotation = FINAL_ROTATION_90;
                break;
            case Surface.ROTATION_180:
                finalRotation = FINAL_ROTATION_180;
                break;
            case Surface.ROTATION_270:
                finalRotation = FINAL_ROTATION_270;
                break;
            default:
                finalRotation = FINAL_ROTATION_0;
                break;
        }
        return finalRotation;
    }

    /**
     * Get camera display orentation
     * 
     * @param degrees
     *            : degrees
     * @param cameraId
     *            : 0
     * @return orientation
     */
    public int getDisplayOrientation(int degrees, int cameraId) {
        // See android.hardware.Camera.setDisplayOrientation for
        // documentation.
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % ROTATION_360;
            result = (ROTATION_360 - result) % ROTATION_360; // compensate the
            // mirror
        } else { // back-facing
            result = (info.orientation - degrees + ROTATION_360) % ROTATION_360;
        }
        Elog.v(TAG, "getDisplayOrientation() result is !" + result);
        return result;
    }

    private void stopPreview() {
        Elog.v(TAG, "stopPreview() start!");
        if (null != mCamera) {
            mCamera.stopPreview();
        }
        mIsPreviewing = 0;
        Elog.v(TAG, "stopPreview() end!");
        // clearFocusState();
        // int isDraw = 0;
        // setFocusRectangle(isDraw);
    }

    private Size getOptimalPreviewSize(List<Size> sizes, double targetRatio) {
        final double aspectTolerance = 0.05;
        if (sizes == null) {
            return null;
        }
        Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        // Because of bugs of overlay and layout, we sometimes will try to
        // layout the viewfinder in the portrait orientation and thus get the
        // wrong size of mSurfaceView. When we change the preview size, the
        // new overlay will be created before the old one closed, which causes
        // an exception. For now, just get the screen size
        Display display = getWindowManager().getDefaultDisplay();
        int targetHeight = Math.min(display.getHeight(), display.getWidth());
        if (targetHeight <= 0) {
            // We don't know the size of SurefaceView, use screen height
            WindowManager windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
            targetHeight = windowManager.getDefaultDisplay().getHeight();
        }
        // try to find a size larger but closet to the desired preview size
        for (Size size : sizes) {
            if (targetHeight > size.height) {
                continue;
            }
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > aspectTolerance) {
                continue;
            }
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }
        // not found, apply origional policy.
        if (optimalSize == null) {
            // Try to find an size match aspect ratio and size
            for (Size size : sizes) {
                double ratio = (double) size.width / size.height;
                if (Math.abs(ratio - targetRatio) > aspectTolerance) {
                    continue;
                }
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        // Cannot find the one match the aspect ratio, ignore the requirement
        if (optimalSize == null) {
            Elog.i(TAG, "No preview size match the aspect ratio");
            minDiff = Double.MAX_VALUE;
            for (Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        return optimalSize;
    }

    private boolean judgeSdcard() {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            mIssdcardExist = true;
        } else {
            mIssdcardExist = false;
        }

        if (mIssdcardExist) {
            return true;
        } else {
            if (mIsTest) {
                mIsTest = false;
                mHandler.sendEmptyMessage(MSG_REPEAT_COMPLET);
            }
            if (null != mCameraParam) {
                // remove from mt6589
                // int isDraw = 0;
                // setFocusRectangle(isDraw);
                Elog.v(TAG, "judgeSdcard()  mCameraParam.setFocusDrawMode(0)");

            }
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("SD Card not available");
            builder.setMessage("Please insert an SD Card.");
            builder.setPositiveButton("OK", null);
            builder.create().show();
            return false;
        }
    }

    private void capturePicture() {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_SHARED)) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("sdcard is busy");
            builder.setMessage("Sorry, your SD card is busy.");
            builder.setPositiveButton("OK", null);
            builder.create().show();
            return;
        }
        Elog.v(TAG, "CapturePicture()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        if (mIsTest) {
            Toast.makeText(this, "It is in capturing, can not repeat capture.", Toast.LENGTH_LONG)
                    .show();
            if (mProgresDlgExist) {
                showDialog(DIALOG_PROGRESS);
            }
            return;
        }
        handleCaptureMode();
    }

    private void handleCaptureMode() {
        Elog.v(TAG, "mMode = " + mMode);
        switch (mMode) {
            case 0:
                captureMode0();
                break;
            case 1:
                if (isSupportaf(mCameraParam, AF_MANUAL)) {
                    mShotNum.setText("");
                    mShotNum.setVisibility(View.VISIBLE);
                }
                captureMode1();
                break;
            case 2:
                captureMode2();
                break;
            case ITEM_MODE_3:
                if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
                    mShotNum.setText("");
                    mShotNum.setVisibility(View.VISIBLE);
                }
                captureMode3();
                break;
            case ITEM_MODE_4:
                if (isSupportaf(mCameraParam, AF_AUTO)) {
                    mShotNum.setText("");
                    mShotNum.setVisibility(View.VISIBLE);
                }
                captureMode4();
                break;
            case ITEM_MODE_5:
                if (isSupportaf(mCameraParam, AF_FULLSCAN) && isSupportaf(mCameraParam, AF_AUTO)) {
                    mShotNum.setText("");
                    mShotNum.setVisibility(View.VISIBLE);
                }
                captureMode5();
                break;
            default:
                break;
        }
    }

    private void captureMode0() {
        Elog.v(TAG, "Enter captureMode0 function.");
        Elog.v(TAG, "captureMode0()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        AFMode0Thread mode0Thread = new AFMode0Thread();
        mode0Thread.start();
    }

    class AFMode0Thread extends Thread {
        public void run() {
            Elog.v(TAG, "AFMode0Thread");
            mIsTest = true;
            // lock AE
            setAeAwlock(true);
            if (isSupportaf(mCameraParam, AF_AUTO)) {
                mCameraParam.setFocusEngMode(Camera.Parameters.FOCUS_ENG_MODE_NONE);
                mCameraParam.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
                // mCameraParam.set("focus-mode", AF_AUTO);
                mCameraParam.set("focus-meter", "spot");
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 0;
                mCamera.autoFocus(mAutoFocCalback);
            } else {
                Elog.v(TAG, "AFMode0Thread does not support auto mode.");
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 1;
            }
            sCanBack = false;
            takePicture();
            startPreview();
            mIsTest = false;
            sCanBack = true;
            mySleep(SLEEP_TIME1);
            Elog.v(TAG, "mAFMode0FirstThread after Sleep(2000) mMode = " + mMode);
            mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
            Elog.v(TAG, "mAFMode1FirstThread finish.");
            Elog.v(TAG, "mAFMode1FirstThread mCanBack = " + sCanBack);
        }
    }

    private void captureMode1() {
        Elog.v(TAG, "Enter captureMode1 function.");
        Elog.v(TAG, "captureMode1()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        if (isSupportaf(mCameraParam, AF_MANUAL) && mMode != 0) {
            mShotNum.setText("One AF");
        }
        AFMode1FirstThread threadFirst = new AFMode1FirstThread();
        threadFirst.start();
    }

    class AFMode1FirstThread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode1FirstThread");
            mIsTest = true;
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_NONE;
            // lock AE
            setAeAwlock(true);
            if (isSupportaf(mCameraParam, AF_AUTO)) {
                mCameraParam.setFocusEngMode(mAFEngMode);
                mCameraParam.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
                // mCameraParam.set("focus-mode", AF_AUTO);
                mCameraParam.set("focus-meter", "spot");
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 0;
                mCamera.autoFocus(mAutoFocCalback);
            } else {
                Elog.v(TAG, "AFMode1Thread does not support auto mode.");
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 1;
            }
            sCanBack = false;
            takePicture();
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_BRACKET;
            // mCameraParam.set("afeng-mode", mAFEngMode);
            if (isSupportaf(mCameraParam, AF_MANUAL)) {
                mCameraParam.setFocusEngMode(mAFEngMode);
                mCameraParam.set("focus-mode", AF_MANUAL);
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mPos = 0;
                // mCameraParam.set("afeng-pos", mPos);
                mCameraParam.setFocusEngStep(-MGIC_NUM_24 * mStep);
                mCamera.setParameters(mCameraParam);
            } else {
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                Elog.v(TAG, "AFMode1Thread does not support manual mode.");
            }
            startPreview();
            sCanBack = true;
            mySleep(SLEEP_TIME1);
            mHandler.sendEmptyMessage(MSG_AF_MODE1);
            Elog.v(TAG, "mAFMode1FirstThread finish.");
        }
    }

    @Override
    public void onBackPressed() {
        Elog.v(TAG, "onBackPressed() mCanBack = " + sCanBack);
        if (!sCanBack) {
            return;
        }
        super.onBackPressed();
    }

    class AFMode1Thread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode1Thread");
            if (mIsOnPause) {
                mHandler.removeMessages(MSG_AF_MODE1);
                return;
            }
            // lock AE
            setAeAwlock(true);
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            sCanBack = false;
            if (isSupportaf(mCameraParam, AF_MANUAL)) {
                mHandler.sendEmptyMessage(MSG_RENEW_SHOTNUM);
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mIsFocused = 0;
                mCamera.autoFocus(mAutoFocCalback);
            } else {
                mIsFocused = 1;
            }
            takePicture();
            mPos++;
            if (isSupportaf(mCameraParam, AF_MANUAL)) {
                mCameraParam.setFocusEngMode(Camera.Parameters.FOCUS_ENG_MODE_BRACKET);
                mCameraParam.setFocusEngStep((mPos - MGIC_NUM_24) * mStep);
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
            }
            startPreview();
            long dateTaken = System.currentTimeMillis();
            mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
            mCameraParam.set("rawfname", mRawCptFileName + ".raw");
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            sCanBack = true;
            mySleep(SLEEP_TIME1);
            if (!mIsOnPause && mPos < POSITION_VALUE) {
                mHandler.sendEmptyMessage(MSG_AF_MODE1);
            }
            if (mPos >= POSITION_VALUE) {
                mIsTest = false;
                if (isSupportaf(mCameraParam, AF_MANUAL)) {
                    mHandler.sendEmptyMessage(MSG_REPEAT_COMPLET);
                }
                mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
            }
        }
    }

    private void captureMode2() {
        Elog.v(TAG, "Enter captureMode2 function.");
        Elog.v(TAG, "captureMode2()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        AFMode2Thread mode2Thread = new AFMode2Thread();
        mode2Thread.start();
    }

    class AFMode2Thread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode2Thread");
            // lock AE
            setAeAwlock(true);
            mIsTest = true;
            sCanBack = true;
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_FULLSCAN;
            // mCameraParam.set("afeng-mode", mAFEngMode);
            if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
                mCameraParam.setFocusEngMode(mAFEngMode);
                mCameraParam.setFocusEngStep(mStep); // variable step for
                mCameraParam.set("focus-mode", AF_FULLSCAN);
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 0;
                mCamera.autoFocus(mAutoFocCalback);
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
            } else {
                Elog.v(TAG, "AFMode2Thread does not support fullscan mode.");
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.setParameters(mCameraParam);
                mIsFocused = 1;
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
            }
            sCanBack = false;
            takePicture();
            startPreview();
            sCanBack = true;
            mIsTest = false;
            mySleep(SLEEP_TIME1);
            mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
        }
    }

    private void captureMode3() {
        Elog.v(TAG, "Enter captureMode3 function.");
        Elog.v(TAG, "captureMode3()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        mPos = 0;
        mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_FULLSCAN_REPEAT;
        if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
            // mCameraParam.set("afeng-mode", mAFEngMode);
            mCameraParam.setFocusEngMode(mAFEngMode);
            mCameraParam.setFocusEngStep(mStep); // variable step for fullscan
            mCameraParam.set("focus-mode", AF_FULLSCAN);
        }
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        mMode3Thread = new AFMode3Thread();
        mMode3Thread.start();
    }

    class AFMode3Thread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode3Thread");
            if (mIsOnPause) {
                mHandler.removeMessages(MSG_AF_MODE3);
                return;
            }
            mIsTest = true;
            mIsFocused = 0;
            // lock AE
            setAeAwlock(true);
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.autoFocus(mAutoFocCalback);
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
                mHandler.sendEmptyMessage(MSG_RENEW_SHOTNUM);
            } else {
                Elog.v(TAG, "AFMode3Thread does not support fullscan mode.");
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
                mIsFocused = 1;
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
            }
            sCanBack = false;
            takePicture();
            mPos++;
            startPreview();
            long dateTaken = System.currentTimeMillis();
            mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
            mCameraParam.set("rawfname", mRawCptFileName + ".raw");
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            sCanBack = true;
            mySleep(SLEEP_TIME1);
            if (!mIsOnPause && mPos < POSITION_VALUE) {
                mHandler.sendEmptyMessage(MSG_AF_MODE3);
            }
            if (mPos >= POSITION_VALUE) {
                mIsTest = false;
                if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
                    mHandler.sendEmptyMessage(MSG_REPEAT_COMPLET);
                }
                mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
            }

        }
    }

    private void captureMode4() {
        Elog.v(TAG, "Enter captureMode4 function.");
        Elog.v(TAG, "captureMode4()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        mPos = 0;
        // mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_REPEAT;
        if (isSupportaf(mCameraParam, AF_AUTO)) {
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_REPEAT;
            // mCameraParam.set("afeng-mode", mAFEngMode);
            mCameraParam.setFocusEngMode(mAFEngMode);
            mCameraParam.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
            // mCameraParam.set("focus-mode", AF_AUTO);
            mCameraParam.set("focus-meter", "spot");
        }
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        mMode4Thread = new AFMode4Thread();
        mMode4Thread.start();
    }

    class AFMode4Thread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode4Thread");
            if (mIsOnPause) {
                mHandler.removeMessages(MSG_AF_MODE4);
                return;
            }
            Elog.v(TAG, "mAFMode4Thread->judgeSdcard()");
            // lock AE
            setAeAwlock(true);
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            if (!judgeSdcard()) {
                Elog.v(TAG, "No SdCard in AFMode4Thread!");
                this.stop();
            }
            mIsTest = true;
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_REPEAT;
            if (isSupportaf(mCameraParam, AF_AUTO)) {
                mIsFocused = 0;
                if (mCamera == null || mIsOnPause) {
                    return;
                }
                mCamera.autoFocus(mAutoFocCalback);
            } else {
                Elog.v(TAG, "AFMode4Thread does not support auto mode.");
                mIsFocused = 1;
            }
            sCanBack = false;
            if (isSupportaf(mCameraParam, AF_AUTO)) {
                mHandler.sendEmptyMessage(MSG_RENEW_SHOTNUM);
            }
            takePicture();
            mPos++;
            startPreview();
            long dateTaken = System.currentTimeMillis();
            mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
            mCameraParam.set("rawfname", mRawCptFileName + ".raw");
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);

            sCanBack = true;
            mySleep(SLEEP_TIME1);

            if (!mIsOnPause && mPos < POSITION_VALUE) {
                mHandler.sendEmptyMessage(MSG_AF_MODE4);
            }
            if (mPos >= POSITION_VALUE) {
                mIsTest = false;
                if (isSupportaf(mCameraParam, AF_AUTO)) {
                    mHandler.sendEmptyMessage(MSG_REPEAT_COMPLET);
                }
                mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
            }
        }
    }

    private void captureMode5() {
        Elog.v(TAG, "Enter captureMode5 function.");
        Elog.v(TAG, "captureMode5()->judgeSdcard()");
        if (!judgeSdcard()) {
            return;
        }
        mPos = 0;
        mStage = 1;
        mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_REPEAT;
        if (isSupportaf(mCameraParam, AF_AUTO)) {
            // mCameraParam.set("afeng-mode", mAFEngMode);
            mCameraParam.setFocusEngMode(mAFEngMode);
            // mCameraParam.setFocusEngStep(0);
            mCameraParam.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
            // mCameraParam.set("focus-mode", AF_AUTO);
            mCameraParam.set("focus-meter", "spot");
        }
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        mMode5Thread = new AFMode5Thread();
        mMode5Thread.start();
    }

    class AFMode5Thread extends Thread {
        public void run() {
            Elog.v(TAG, "mAFMode5Thread");
            if (mIsOnPause) {
                mHandler.removeMessages(MSG_AF_MODE5);
                return;
            }
            // lock AE
            setAeAwlock(true);
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            mIsTest = true;
            if (mStage == 1) {
                handleMode5Stage1();
            } else if (mStage == 2) {
                handleMode5Stage2();
            }
        }
    }

    private void handleMode5Stage1() {
        mIsFocused = 0;
        if (isSupportaf(mCameraParam, AF_AUTO)) {
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.autoFocus(mAutoFocCalback);
        } else {
            Elog.v(TAG, "AFMode5Thread does not support auto mode.");
            mIsFocused = 1;
        }
        sCanBack = false;
        if (isSupportaf(mCameraParam, AF_FULLSCAN) && isSupportaf(mCameraParam, AF_AUTO)) {
            mHandler.sendEmptyMessage(MSG_RENEW_SHOTNUM);
        }
        takePicture();
        mPos++;
        startPreview();
        long dateTaken = System.currentTimeMillis();
        mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
        mCameraParam.set("rawfname", mRawCptFileName + ".raw");
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        sCanBack = true;
        mySleep(SLEEP_TIME1);
        if (!mIsOnPause && mPos < POSITION_VALUE) {
            mHandler.sendEmptyMessage(MSG_AF_MODE5);
        }
        if (mPos >= POSITION_VALUE) {
            mStage = 2;
            mPos = 0;
            mAFEngMode = Camera.Parameters.FOCUS_ENG_MODE_FULLSCAN_REPEAT;
            if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
                mCameraParam.setFocusEngMode(mAFEngMode);
                mCameraParam.setFocusEngStep(mStep); // variable step
                mCameraParam.set("focus-mode", AF_FULLSCAN);
            }
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.setParameters(mCameraParam);
            mHandler.sendEmptyMessage(MSG_AF_MODE5);
        }
    }

    private void handleMode5Stage2() {
        mIsFocused = 0;
        if (isSupportaf(mCameraParam, AF_FULLSCAN)) {
            if (mCamera == null || mIsOnPause) {
                return;
            }
            mCamera.autoFocus(mAutoFocCalback);
            mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
        } else {
            Elog.v(TAG, "AFMode5Thread does not support fullscan mode.");
            mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_START);
            mIsFocused = 1;
            mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
        }
        if (isSupportaf(mCameraParam, AF_FULLSCAN) && isSupportaf(mCameraParam, AF_AUTO)) {
            mHandler.sendEmptyMessage(MSG_RENEW_SHOTNUM);
        }
        sCanBack = false;
        takePicture();
        mPos++;
        startPreview();
        long dateTaken = System.currentTimeMillis();
        mRawCptFileName = mImgBucketName + mModeName + createName(dateTaken) + mIsoName;
        mCameraParam.set("rawfname", mRawCptFileName + ".raw");
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.setParameters(mCameraParam);
        sCanBack = true;
        mySleep(SLEEP_TIME1);
        if (!mIsOnPause && mPos < POSITION_VALUE) {
            mHandler.sendEmptyMessage(MSG_AF_MODE5);
        }
        if (mPos >= POSITION_VALUE) {
            mIsTest = false;
            mStage = 0;
            if (isSupportaf(mCameraParam, AF_FULLSCAN) && isSupportaf(mCameraParam, AF_AUTO)) {
                mHandler.sendEmptyMessage(MSG_REPEAT_COMPLET);
            }
            mProgressDlgHdl.sendEmptyMessage(COMPLETE_CAPTURE);
        }
    }

    private final class AutoFocusCallback implements android.hardware.Camera.AutoFocusCallback {
        public void onAutoFocus(boolean focused, android.hardware.Camera camera) {
            // mFocusCallbackTime = System.currentTimeMillis();
            mIsFocused = 1;
            Elog.v(TAG, "mAFEngMode value is " + mAFEngMode);
            if (Camera.Parameters.FOCUS_ENG_MODE_FULLSCAN == mAFEngMode
                    || Camera.Parameters.FOCUS_ENG_MODE_FULLSCAN_REPEAT == mAFEngMode) {
                Elog.v(TAG, "AutoFocusCallback send EVENT_FULL_SCAN_COMPLETE message ");
                mProgressDlgHdl.sendEmptyMessage(FULL_SCAN_COMPLET);
            }
            Elog.v(TAG, "In mAutoFocusCallback before set CapturePicture mFocusFlag = "
                    + mFocusFlag);
            mFocusFlag = focused;
            Elog
                    .v(TAG, "In mAutoFocusCallback after set CapturePicture mFocusFlag = "
                            + mFocusFlag);
            // int isDraw = 1;
            // setFocusRectangle(isDraw);
        }
    }

    private final class ShutterCallback implements android.hardware.Camera.ShutterCallback {
        public void onShutter() {
            Elog.v(TAG, "ShutterCallback");
            // mShutterCallbackTime = System.currentTimeMillis();
            // int isDraw = 0;
            // setFocusRectangle(isDraw);
        }
    }

    private final class RawPictureCallback implements PictureCallback {
        public void onPictureTaken(byte[] rawData, android.hardware.Camera camera) {
            Elog.v(TAG, "RawPictureCallback");
            // mRawPictureCallbackTime = System.currentTimeMillis();
        }
    }

    private final class JpegPictureCallback implements PictureCallback {
        public void onPictureTaken(byte[] jpegData, android.hardware.Camera camera) {
            // mJpegPictureCallbackTime = System.currentTimeMillis();
            Elog.v(TAG, "JpegPictureCallback");
            if (jpegData != null) {
                Elog.v(TAG, "jpegData != null");
                storeImage(jpegData);
            }
            mIsCapture = 0;
        }
    }

    private void takePicture() {
        Elog.v(TAG, "takePicture() start");
        Elog.v(TAG, "takePicture()-> judgeSdcard()");
        boolean isSDCard = judgeSdcard();
        if (!isSDCard) {
            Elog.v(TAG, "No SdCard!");
            return;
        }

        while (mIsFocused == 0) {
            Elog.v(TAG, "takePicture()->sleep 1");
            mySleep(SLEEP_TIME_100);
        }
        mIsCapture = 1;
        // mCaptureStartTime = System.currentTimeMillis();
        if (mCamera == null || mIsOnPause) {
            return;
        }
        mCamera.takePicture(mShutterCalback, mRawPicCalback, new JpegPictureCallback());
        // mIsCapture = 0;
        while (mIsCapture == 1) {
            // Elog.v(TAG, "takePicture()->sleep 2");
            mySleep(SLEEP_TIME_100);
        }
        Elog.v(TAG, "takePicture() end");
    }

    private void mySleep(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void storeImage(byte[] jpegData) {
        Elog.v(TAG, "storeImage()");
        // long time;
        long dateTaken = System.currentTimeMillis();

        StringBuffer name = new StringBuffer(mImgBucketName);
        // mode 5,make different name for picture to distinguish between
        // fullscan and auto mode.
        if ((mMode == ITEM_MODE_5 && mStage == 1) || mMode == ITEM_MODE_4 || mMode == 0) {
            name.append("AF_");
        } else if ((mMode == ITEM_MODE_5 && mStage == 2) || mMode == 2 || mMode == ITEM_MODE_3) {
            name.append("Fullscan_");
        } else if (mMode == 1) {
            name.append("Bracket_");
        }
        name.append(createNameJpeg(dateTaken)).append(".jpg");

        if (mIsRawCapture) {
            name = new StringBuffer(mRawCptFileName).append(".jpg");
        }
        Elog.v(TAG, "Jpeg name is " + name);
        File fHandle = new File(name.toString());
        OutputStream bos = null;
        try {
            bos = new FileOutputStream(fHandle);
            bos.write(jpegData);
            // time = System.currentTimeMillis();
        } catch (FileNotFoundException ex) {
            fHandle.delete();
        } catch (IOException ex) {
            fHandle.delete();
        } finally {
            if (null != bos) {
                try {
                    bos.close();
                } catch (IOException e) {
                    Elog.w(TAG, e.getMessage());
                }
            }
        }

    }

    private static String createName(long dateTaken) {
        return DateFormat.format("ddkkmmss", dateTaken).toString();
    }

    private static String createNameJpeg(long dateTaken) {
        return DateFormat.format("yyyy-MM-dd kk.mm.ss", dateTaken).toString();
    }

    /**
     * If camera error, will call this method and show the error message
     * 
     * @param activity
     *            : this activity
     * @param msgId
     *            : error message id
     */
    public static void showErrorAndFinish(final Activity activity, int msgId) {
        DialogInterface.OnClickListener buttonListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                activity.finish();
            }
        };
        new AlertDialog.Builder(activity).setCancelable(false)
                .setTitle(R.string.camera_error_title).setMessage(msgId).setNeutralButton(
                        R.string.dialog_ok, buttonListener).show();
    }

    // private boolean isTbModel() {
    // Elog.v(TAG, "Device model is " + DEVICE_MODEL);
    // if (DEVICE_MODEL.contains("Lenovo75_a2_tb")
    // || DEVICE_MODEL.contains("tablet")) {
    // return true;
    // } else {
    // return false;
    // }
    // }

    private boolean isSupportaf(Camera.Parameters mParam, String afMode) {
        ArrayList<String> supportFocusMode = (ArrayList<String>) mParam.getSupportedFocusModes();
        if (supportFocusMode == null) {
            return false;
        } else if (supportFocusMode.isEmpty()) {
            return false;
        }
        for (String fm : supportFocusMode) {
            if (fm.equals(afMode)) {
                return true;
            }
        }
        return false;
    }
}
