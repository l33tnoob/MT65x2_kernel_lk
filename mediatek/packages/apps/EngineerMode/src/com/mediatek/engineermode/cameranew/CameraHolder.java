/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.engineermode.cameranew;

import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.io.IOException;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 * 
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in {@code android.hardware.Camera}. The difference
 * is if {@code keep()} is called before {@code release()}, CameraHolder will try to hold the {@code android.hardware.Camera}
 * instance for a while, so if {@code open()} is called soon after, we can avoid the cost of {@code open()} in
 * {@code android.hardware.Camera}.
 * 
 * <p>
 * This is used in switching between {@code Camera} and {@code VideoCamera} activities.
 */
public class CameraHolder {
    private static final String TAG = "CameraHolder";
    private android.hardware.Camera mCameraDevice;
    private long mKeepBeforeTime = 0; // Keep the Camera before this time.
    private final Handler mHandler;
    private int mUsers = 0; // number of open() - number of release()
    private int mNumberOfCameras;
    private int mCameraId = -1; // current camera id
    private int mBackCameraId = -1;
    private int mFrontCameraId = -1;
    private CameraInfo[] mInfo;

    private boolean mNeedReopen = false;

    // We store the camera parameters when we actually open the device,
    // so we can restore them in the subsequent open() requests by the user.
    // This prevents the parameters set by the Camera activity used by
    // the VideoCamera activity inadvertently.
    private Parameters mParameters;

    // Use a singleton.
    private static CameraHolder sHolder;

    public static synchronized CameraHolder instance() {
        if (sHolder == null) {
            sHolder = new CameraHolder();
        }
        return sHolder;
    }

    private static final int RELEASE_CAMERA = 1;

    private class MyHandler extends Handler {
        MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case RELEASE_CAMERA:
                    synchronized (CameraHolder.this) {
                        // In 'CameraHolder.open', the 'RELEASE_CAMERA' message
                        // will be removed if it is found in the queue. However,
                        // there is a chance that this message has been handled
                        // before being removed. So, we need to add a check
                        // here:
                        if (CameraHolder.this.mUsers == 0 && mCameraDevice != null) {
                            releaseCamera();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private CameraHolder() {
        HandlerThread ht = new HandlerThread("CameraHolder");
        ht.start();
        mHandler = new MyHandler(ht.getLooper());
        mNumberOfCameras = android.hardware.Camera.getNumberOfCameras();
        mInfo = new CameraInfo[mNumberOfCameras];
        for (int i = 0; i < mNumberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            android.hardware.Camera.getCameraInfo(i, mInfo[i]);
            if (mBackCameraId == -1 && mInfo[i].facing == CameraInfo.CAMERA_FACING_BACK) {
                mBackCameraId = i;
            }
            if (mFrontCameraId == -1 && mInfo[i].facing == CameraInfo.CAMERA_FACING_FRONT) {
                mFrontCameraId = i;
            }
        }
    }

    public int getNumberOfCameras() {
        return mNumberOfCameras;
    }

    public CameraInfo[] getCameraInfo() {
        return mInfo;
    }

    public synchronized android.hardware.Camera open(int cameraId) throws CameraHardwareException {
//        aAssert(mUsers == 0);
        if (mCameraDevice != null && (mCameraId != cameraId || mNeedReopen)) {
            mCameraDevice.release();
            mCameraDevice = null;
            mCameraId = -1;
        }
        if (mCameraDevice == null) {
            try {
                Log.v(TAG, "open camera " + cameraId);
                mCameraDevice = android.hardware.Camera.open(cameraId);
                mCameraId = cameraId;
            } catch (RuntimeException e) {
                Log.e(TAG, "fail to connect Camera", e);
                throw new CameraHardwareException(e);
            }
            mParameters = mCameraDevice.getParameters();
        } else {
            try {
                mCameraDevice.reconnect();
            } catch (IOException e) {
                Log.e(TAG, "reconnect failed.");
                throw new CameraHardwareException(e);
            }
            mCameraDevice.setParameters(mParameters);
        }
        ++mUsers;
        mHandler.removeMessages(RELEASE_CAMERA);
        mKeepBeforeTime = 0;
        return mCameraDevice;
    }

    public synchronized void release() {
//        aAssert(mUsers == 1);
        --mUsers;
        mCameraDevice.stopPreview();
        releaseCamera();
    }

    private synchronized void releaseCamera() {
//        aAssert(mUsers == 0);
//        aAssert(mCameraDevice != null);
        long now = System.currentTimeMillis();
        if (now < mKeepBeforeTime) {
            mHandler.sendEmptyMessageDelayed(RELEASE_CAMERA, mKeepBeforeTime - now);
            return;
        }
        mCameraDevice.release();
        mCameraDevice = null;
        // We must set this to null because it has a reference to Camera.
        // Camera has references to the listeners.
        mParameters = null;
        mCameraId = -1;
    }

    // must be called in synchronized(sHolder) {} block;
    public boolean isSameCameraDevice(android.hardware.Camera c) {
        return mCameraDevice == c;
    }

    public int getBackCameraId() {
        return mBackCameraId;
    }

    public int getFrontCameraId() {
        return mFrontCameraId;
    }
}
