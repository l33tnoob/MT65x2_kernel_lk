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
/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cta.camera;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.LoaderManager.LoaderCallbacks;
import android.app.ProgressDialog;
import android.app.admin.DevicePolicyManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.media.CameraProfile;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.storage.StorageManager;
import android.text.format.DateFormat;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.Surface;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import com.mediatek.cta.R;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/** The Camera activity which can preview and take pictures. */
public class Camera extends ActivityBase implements View.OnClickListener, SurfaceHolder.Callback {
    private static final String TAG = "CtaTestCamera";
    private static final int MSG_START_PREVIEW = 0;
    private static final int MSG_START_SUCCESS = 1;
    private static final int MSG_START_FAILED = 2;
    private static final int MSG_STOP_PREVIEW = 3;
    private static final int MSG_START_CAPTURE = 4;
    private static final int MSG_START_RECORD = 5;
    private static final int MSG_RECORD_STARTED = 6;
    private static final int MSG_STOP_RECORD = 7;
    private static final int MSG_PICTURES_SAVING_DONE = 8;
    private static final int MSG_SURFACE_CHANGED = 9;
    private static final int MSG_SURFACE_DESTROY = 10;
    private static final int MSG_RECORD_START_FAILED = 11;

    private static final int DIALOG_WAIT = 0;
    private static final int DIALOG_ERROR = 1;
    private static final int DIALOG_DISABLE_WIFI = 2;
    private static final int DIALOG_DISABLE_BT = 3;

    private static final String KEY_RAW_SAVE_MODE = "rawsave-mode";
    private static final int RAW_SAVE_JPEG = 3;

    private static final int PREVIEW_STOPPED = 0;
    private static final int IDLE = 1; // preview is active
    private static final int SAVING_PICTURES = 5;

    private Button mShutterButton;
    private Button mRecorderButton;
    private Toast mToast = null;
    private ProgressDialog mDialog = null;

    MediaRecorder mRecorder = null;
    private Parameters mParameters;
    private SurfaceHolder mSurfaceHolder = null;
    private boolean mOpenCameraFail = false;
    private boolean mCameraDisabled = false;
    private boolean mPausing;
    private int mCameraState = PREVIEW_STOPPED;
    private int mDisplayRotation;
    private int mDisplayOrientation;

    private final HandlerThread mHandlerThread = new HandlerThread("async_handler");
    private Handler mWorkHandler;

    class WorkHandler extends Handler {
        public WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_START_PREVIEW:
                if (mCameraState == PREVIEW_STOPPED) {
                    openCamera();
                    startPreview();
                    if (mOpenCameraFail || mCameraDisabled) {
                        mHandler.sendEmptyMessage(MSG_START_FAILED);
                    } else {
                        mHandler.sendEmptyMessage(MSG_START_SUCCESS);
                    }
                }
                break;
            case MSG_STOP_PREVIEW:
                stopPreview();
                closeCamera();
                break;
            case MSG_START_CAPTURE:
                capture();
                break;
            case MSG_START_RECORD:
                record();
                break;
            case MSG_STOP_RECORD:
                stopRecord();
                break;
            case MSG_SURFACE_CHANGED:
                Log.v(TAG, "s1");
                mSurfaceHolder = (SurfaceHolder) msg.obj;
                if (mCameraDevice == null || mPausing || Camera.this.isFinishing()) {
                    Log.v(TAG, "s2");
                    return;
                }
                if (mCameraState != PREVIEW_STOPPED) {
                    Log.v(TAG, "s3");
                    if (getDisplayRotation() != mDisplayRotation) {
                        setDisplayOrientation();
                    }
                    setPreviewDisplay(mSurfaceHolder);
                }
                break;
            case MSG_SURFACE_DESTROY:
                stopRecord();
                stopPreview();
                mSurfaceHolder = null;
                break;
            }
        }
    };

    /**
     * This Handler is used to post message back onto the main thread of the
     * application
     */
    final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_PICTURES_SAVING_DONE:
                    if (!mPausing) {
                        startPreview();
                    }
                    mShutterButton.setEnabled(true);
                    mRecorderButton.setEnabled(true);
                    break;
                case MSG_RECORD_STARTED:
                    mRecorderButton.setEnabled(true);
                    break;
                case MSG_RECORD_START_FAILED:
                    mRecorderButton.setText(getString(R.string.cta_record_video));
                    break;
                case MSG_START_SUCCESS:
                    if (mDialog != null) {
                        mDialog.dismiss();
                    }
                    mShutterButton.setEnabled(true);
                    mRecorderButton.setEnabled(true);
                    break;
                case MSG_START_FAILED:
                    showDialog(DIALOG_ERROR);
                    break;
                default:
                    break;
            }
        }
    };

    private void capture() {
        Log.i(TAG, "capture()");
        if (mCameraDevice == null || mPausing || mCameraState != IDLE) {
            Log.d(TAG, "capture() return1");
            return;
        }
        mCameraDevice.takePicture(null, null, new JpegPictureCallback());
    }

    private void record() {
        Log.i(TAG, "record()");
        if (mCameraDevice == null || mPausing || mCameraState != IDLE) {
            Log.d(TAG, "record() return1");
            return;
        }

        try {
            mCameraDevice.unlock();
            if (mRecorder == null) {
                mRecorder = new MediaRecorder();
            }
            mRecorder.setCamera(mCameraDevice);
            mRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
            mRecorder.setOutputFormat(MediaRecorder.OutputFormat.DEFAULT);
            mRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.DEFAULT);
            mRecorder.setOutputFile("/sdcard/cta_test_camera.3gpp");
            mRecorder.prepare();
            mRecorder.start();
        } catch (Exception e) {
            mHandler.sendEmptyMessage(MSG_RECORD_START_FAILED);
            stopRecord();
            return;
        }
        setCameraState(SAVING_PICTURES);
        mHandler.sendEmptyMessage(MSG_RECORD_STARTED);
    }

    private void stopRecord() {
        Log.i(TAG, "stopRecord() stop/reset/release");
        try {
            if (mRecorder != null) {
                mRecorder.stop();
                mRecorder.reset();
                mRecorder.release();
            }
        } catch (RuntimeException e) {
        } finally {
            mRecorder = null;
        }

        Log.i(TAG, "stopRecord() stopped");
        if (mCameraDevice == null || mPausing) {
            Log.d(TAG, "stopRecord() return1");
            return;
        }

        Log.i(TAG, "stopRecord() reconnect");
        try {
            mCameraDevice.reconnect();
        } catch (IOException e) {
        }
        Log.i(TAG, "stopRecord() reconnected");
        mHandler.sendEmptyMessage(MSG_PICTURES_SAVING_DONE);
        Log.i(TAG, "stopRecord() end");
    }

    private final class JpegPictureCallback implements PictureCallback {
        @Override
        public void onPictureTaken(byte[] jpegData, android.hardware.Camera camera) {
            if (mPausing) {
                return;
            }

            setCameraState(SAVING_PICTURES);
            long dateTaken = System.currentTimeMillis();
            String jpegName = DateFormat.format("yyyyMMdd-kkmmss", dateTaken).toString() + ".jpg";
            Log.v(TAG, "Jpeg name is " + jpegName);

            File fHandle = new File("/sdcard/cta_test_" + jpegName);
            try {
                OutputStream bos = new FileOutputStream(fHandle);
                bos.write(jpegData);
                bos.close();
            } catch (IOException ex) {
                fHandle.delete();
            }
            mHandler.sendEmptyMessage(MSG_PICTURES_SAVING_DONE);
        }
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.cta_camera);

        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.camera_preview);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        mShutterButton = (Button) findViewById(R.id.capture_btn);
        mShutterButton.setOnClickListener(this);
        mRecorderButton = (Button) findViewById(R.id.record_btn);
        mRecorderButton.setOnClickListener(this);

//        mShutterButton.setEnabled(false);
//        mRecorderButton.setEnabled(false);

        mHandlerThread.start();
        mWorkHandler= new WorkHandler(mHandlerThread.getLooper());
//        mWorkHandler.sendEmptyMessage(MSG_START_PREVIEW);
        showDialog(DIALOG_WAIT);
    }

    @Override
    protected void doOnResume() {
        mPausing = false;
        Log.i(TAG, "doOnResume() Camera State = " + String.valueOf(mCameraState));
        Log.i(TAG, "doOnresume end");
    }

    @Override
    protected void onStart() {
        Log.i(TAG, "onStart()");
        super.onStart();
        mShutterButton.setEnabled(false);
        mRecorderButton.setEnabled(false);

        // Start the preview if it is not started.
        mWorkHandler.sendEmptyMessage(MSG_START_PREVIEW);
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause()");
        mPausing = true;
        super.onPause();
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "onStop()");
        if (mRecorderButton.getText().toString().equals(getString(R.string.cta_test_stop))) {
            mRecorderButton.setText(getString(R.string.cta_record_video));
            mWorkHandler.sendEmptyMessage(MSG_STOP_RECORD);
        }
        mWorkHandler.sendEmptyMessage(MSG_STOP_PREVIEW);
        super.onStop();
    }

    @Override
    public void onClick(View v) {
        if (mPausing || mCameraDevice == null) {
            Log.d(TAG, "onTouch return1");
            return;
        }
        if (v.equals(mShutterButton)) {
            Log.v(TAG, "mCaptureBtn key up!");
            mShutterButton.setEnabled(false);
            mRecorderButton.setEnabled(false);
            mWorkHandler.sendEmptyMessage(MSG_START_CAPTURE);
        }
        if (v.equals(mRecorderButton)) {
            Log.v(TAG, "mRecorderButton key up!");
            mShutterButton.setEnabled(false);
            mRecorderButton.setEnabled(false);
            if (mRecorderButton.getText().toString().equals(getString(R.string.cta_record_video))) {
                mRecorderButton.setText(getString(R.string.cta_test_stop));
                mWorkHandler.sendEmptyMessage(MSG_START_RECORD);
            } else {
                mRecorderButton.setText(getString(R.string.cta_record_video));
                mWorkHandler.sendEmptyMessage(MSG_STOP_RECORD);
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_WAIT:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage("Open Camera");
            mDialog.setCancelable(false);
            return mDialog;
        case DIALOG_ERROR:
            DialogInterface.OnClickListener buttonListener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    Camera.this.finish();
                }
            };
            return new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle("")
                    .setMessage(R.string.cannot_connect_camera)
                    .setNeutralButton(R.string.dialog_ok, buttonListener)
                    .show();
        default:
            return super.onCreateDialog(id);
        }
    }

    @Override
    public void onBackPressed() {
        if (mCameraState != IDLE) {
            return;
        }
        super.onBackPressed();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (holder.getSurface() == null) {
            Log.d(TAG, "holder.getSurface() == null");
            return;
        }
        Log.v(TAG, "surfaceChanged. w=" + width + ". h=" + height);
        Message msg = mWorkHandler.obtainMessage(MSG_SURFACE_CHANGED, holder);
        mWorkHandler.sendMessage(msg);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated.");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mWorkHandler.sendEmptyMessage(MSG_SURFACE_DESTROY);
    }

    private void openCamera() {
        Log.d(TAG, "mCameraOpenThread start");

        // Check if device policy has disabled the camera.
        DevicePolicyManager dpm = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm.getCameraDisabled(null)) {
            mCameraDisabled = true;
            return;
        }

        if (mCameraDevice == null) {
            try {
                Log.v(TAG, "open camera ");
                mCameraDevice = android.hardware.Camera.open(0);
            } catch (RuntimeException e) {
                Log.e(TAG, "fail to connect Camera" + e);
                mOpenCameraFail = true;
                return;
            }
            if (mCameraDevice == null) {
                mOpenCameraFail = true;
                return;
            }
            mParameters = mCameraDevice.getParameters();
        } else {
            try {
                mCameraDevice.reconnect();
            } catch (IOException e) {
                Log.e(TAG, "reconnect failed.");
                mOpenCameraFail = true;
                return;
            }
            mCameraDevice.setParameters(mParameters);
        }
    }

    private void closeCamera() {
        if (mCameraDevice != null) {
            mCameraDevice.release();
            mCameraDevice = null;
            setCameraState(PREVIEW_STOPPED);
        }
    }

    private void startPreview() {
        Log.d(TAG, "startPreview()");
        if (mPausing || isFinishing() || mCameraDevice == null) {
            Log.d(TAG, "startPreview() return1");
            return;
        }

        try {
            if (mCameraState != PREVIEW_STOPPED) {
                stopPreview();
            }
            if (mSurfaceHolder == null) {
                Log.d(TAG, "startPreview() mSurfaceHolder == null");
            }
            setPreviewDisplay(mSurfaceHolder);
            setDisplayOrientation();
            setCameraParameters(); // set paramters

            Log.v(TAG, "startPreview");
            mCameraDevice.startPreview();
        } catch (Throwable ex) {
            closeCamera();
            Log.v(TAG, "startPreview exception." + ex.getMessage());
            return;
        }
        setCameraState(IDLE);
    }

    private void stopPreview() {
        if (mCameraDevice != null && mCameraState != PREVIEW_STOPPED) {
            Log.v(TAG, "stopPreview");
            mCameraDevice.stopPreview();
        }
        setCameraState(PREVIEW_STOPPED);
    }

    private void setPreviewDisplay(SurfaceHolder holder) {
        try {
            mCameraDevice.setPreviewDisplay(holder);
        } catch (IOException ex) {
            closeCamera();
            return;
        }
    }

    private void setCameraParameters() {
        mParameters = mCameraDevice.getParameters();
        mParameters.setCameraMode(Parameters.CAMERA_MODE_NORMAL);
        mParameters.set(KEY_RAW_SAVE_MODE, RAW_SAVE_JPEG);
        mCameraDevice.setParameters(mParameters);
    }

    private void setCameraState(int state) {
        Log.d(TAG, "setCameraState() state " + state);
        mCameraState = state;
    }

    private void setDisplayOrientation() {
        mDisplayRotation = getDisplayRotation();
        mDisplayOrientation = getDisplayOrientation(mDisplayRotation);
        mCameraDevice.setDisplayOrientation(mDisplayOrientation);
    }

    public int getDisplayRotation() {
        int rotation = getWindowManager().getDefaultDisplay().getRotation();
        switch (rotation) {
            case Surface.ROTATION_0:
                return 0;
            case Surface.ROTATION_90:
                return 90;
            case Surface.ROTATION_180:
                return 180;
            case Surface.ROTATION_270:
                return 270;
            default:
                break;
        }
        return 0;
    }

    public int getDisplayOrientation(int degrees) {
        // See android.hardware.Camera.setDisplayOrientation for
        // documentation.
        CameraInfo info = new CameraInfo();
        android.hardware.Camera.getCameraInfo(0, info);
        int result;
        if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else { // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }
}

