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

package com.mediatek.engineermode.cameranew;

import android.app.AlertDialog;
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
import android.media.CameraProfile;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
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
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
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
public class Camera extends ActivityBase implements FocusManager.Listener, View.OnTouchListener, SurfaceHolder.Callback {

    static final String TAG = "test/camera";
    static final int AF_MODE_AUTO = 0;
    static final int AF_MODE_BRACKET = 1;
    static final int AF_MODE_FULL_SCAN = 2;
    static final int AF_MODE_THROUGH_FOCUS = 3;
    static final int AF_MODE_CONTINUOUS = 4;
    
    private static final int MENU_ID_AE = 10;
    private static final int MENU_ID_EV = 11;
    private static final int MENU_ID_STROBE_DUTY = 12;
    
    private static final int INTERVAL_MS_UPDATE_CAMERA_INFO = 200;
    
    // Storage
    private static final long UNAVAILABLE = -1L;
    private static final long PREPARING = -2L;
    private static final long UNKNOWN_SIZE = -3L;
    private static final long FULL_SDCARD = -4L;
    private static final long LOW_STORAGE_THRESHOLD = 50000000;
    private static final long PICTURE_SIZE = 1500000;
    // handler msg
    private static final int FIRST_TIME_INIT = 2;
    private static final int CLEAR_SCREEN_DELAY = 3;
    private static final int CHECK_DISPLAY_ROTATION = 5;
    private static final int UPDATE_STORAGE = 8;
    private static final int PICTURES_SAVING_DONE = 9;
    private static final int EVENT_COMPLETE_CAPTURE = 102;
    private static final int EVENT_START_CAPTURE = 103;
    private static final int EVENT_CAPTURE_ACTION = 106;
    private static final int EVENT_PREVIEW_RAW_DUMP = 107;
    private static final int EVENT_WAITING_DONE = 201;
    private static final int MSG_ID_UPDATE_CAMERA_INFO = 301;
    private static final int MSG_ID_SHUTTER_DELAY_DONE = 302;
    // The subset of parameters we need to update in setCameraParameters().
    private static final int UPDATE_PARAM_INITIALIZE = 1;
    private static final int UPDATE_PARAM_PREFERENCE = 4;
    private static final int UPDATE_PARAM_ALL = -1;
    // App Eng mode key and value
    private static final String ROPERTY_KEY_CLIENT_APPMODE = "client.appmode";
    private static final String APP_MODE_NAME_MTK_ENG = "MtkEng";
    // Raw save mode
    private static final String KEY_RAW_SAVE_MODE = "rawsave-mode";
    // private static final int RAW_SAVE_PREVIEW = 1;
    // private static final int RAW_SAVE_CAPTURE = 2;
    private static final int RAW_SAVE_JPEG = 3;
    private static final int RAW_SAVE_VIDEO = 4;
    // Isp mode
    private static final String KEY_ISP_MODE = "isp-mode";
    // private static final int ISP_MODE_PROCESSED = 0;
    // private static final int ISP_MODE_PURE = 1;
    // Raw path and name
    private static final String KEY_RAW_PATH = "rawfname";
    private static final String KEY_FLASH_MODE = "flash-mode";
    private static final int SCREEN_DELAY = 2 * 60 * 1000;
    private static final long RAW_JPEG_MAX_SIZE = 17 * 1024 * 1024;
    private static final long JPEG_MAX_SIZE = 1 * 1024 * 1024;
    private Parameters mParameters;
    private Parameters mInitialParams;
    private boolean mFocusAreaSupported;
    private boolean mMeteringAreaSupported;
    private boolean mAeLockSupported;
    private boolean mAwbLockSupported;

    private MyOrientationEventListener mOrientationListener;
    // The degrees of the device rotated clockwise from its natural orientation.
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    // The orientation compensation for icons and thumbnails. Ex: if the value
    // is 90, the UI components should be rotated 90 degrees counter-clockwise.
    private int mOrientationCompensation = 0;

    private SurfaceHolder mSurfaceHolder = null;
    private Button mShutterButton;
    private GestureDetector mPopupGestureDetector;
    private boolean mOpenCameraFail = false;
    private boolean mCameraDisabled = false;

    private View mPreviewPanel; // The container of PreviewFrameLayout.
    private PreviewFrameLayout mPreviewFrameLayout;
    private View mPreviewFrame; // Preview frame area.
    // private FaceView mFaceView;
    private RotateLayout mFocusAreaIndicator;

    // A view group that contains all the small indicators.
    private Rotatable mOnScreenIndicators;

    // The display rotation in degrees. This is only valid when mCameraState is
    // not PREVIEW_STOPPED.
    private int mDisplayRotation;
    // The value for android.hardware.Camera.setDisplayOrientation.
    private int mDisplayOrientation;
    private boolean mPausing;
    private boolean mFirstTimeInitialized;

    private static final int PREVIEW_STOPPED = 0;
    private static final int IDLE = 1; // preview is active
    // Focus is in progress. The exact focus state is in Focus.java.
    private static final int FOCUSING = 2;
    private static final int SNAPSHOT_IN_PROGRESS = 3;
    private static final int SELFTIMER_COUNTING = 4;
    private static final int SAVING_PICTURES = 5;
    private int mCameraState = PREVIEW_STOPPED;
    private boolean mSnapshotOnIdle = false;
    private boolean mDidRegister = false;

    private final ShutterCallback mShutterCallback = new ShutterCallback();
    private final RawPictureCallback mRawPictureCallback = new RawPictureCallback();
    private final AutoFocusCallback mAutoFocusCallback = new AutoFocusCallback();
    private final CameraErrorCallback mErrorCallback = new CameraErrorCallback();

    private final PreviewRawDumpCallback mPreviewRawDumpCallback = new PreviewRawDumpCallback();

    private long mFocusStartTime;
    private long mCaptureStartTime;
    private long mShutterCallbackTime;
    private long mRawPictureCallbackTime;
    private long mOnResumeTime;
    private long mPicturesRemaining;
    private byte[] mJpegImageData;

    // These latency time are for the CameraLatency test.
    private long mAutoFocusTime;
    private long mShutterLag;

    // This handles everything about focus.
    /* package */FocusManager mFocusManager;

    /* package */final Handler mHandler = new MainHandler();
    private int mCameraId;

    private boolean mSettingRetrieved = false;
    private static boolean sIsAutoFocusCallback = false;
    // new
    // private int mIso = 0;
    private int mCaptureMode = 0;
    private int mCaptureSize = 0;
    private int mCaptureType = 0;
    private int mCaptureNumber = 0;
    private int mVideoResolution = 0;
    private String mFlickerString = "50hz";
    // Use for 2nd MP(JB2)
    private String mStrobeMode = "auto";
    private static final String[] STROBE_MODE = { "auto", "on", "off" };
    private int mPreFlashLevel = 1;
    private int mMainFlashLevel = 1;
    private int mAfMode = AF_MODE_AUTO;
    private int mAfBracketRange = 0;
    private int mAfBracketIntervel = 1;
    private int mAfThroughIntervel = 1;
    private int mAfThroughDirect = 0;
    // Through focus: Manual configure
    private int mThroughFocusStartPos = 0;
    private int mThroughFocusEndPos = 1023;
    private int mAfCaptureTimes = 0;
    private int mTripleCount = 0;
    private String mIsoValue = "";
    private ArrayList<String> mIsoValues = null;
    private String mRawCaptureFileName;
    private boolean mTakePicDone = true;
    private StorageManager mStorageManager = null;
    private String mCameraImageName = "";
    // For AF bracket and Through focus
    private int mBracketBestPos;
    private int mBracketMaxPos;
    private int mBracketMinPos;
    private int mPosValue; // real position
    private int mBracketTimes;
    private boolean mIsBracketAddPos = true; // if true, capture from best
                                             // position to max position or from
                                             // min to max
    private int mAfFullScanFrameInterval;
    private int mAfThroughRepeat;
	private int mAfThroughRepearPointer;
	private boolean mIsNeedWait;
	private TextView mTvCameraInfo;
	private int mCaptureDoneNumber = 0;
	private boolean mEnableAeAwbLock = true;
	private String mStrobeDuty = "-1";
	private boolean mManualLockAe = false;
	private String mCustomParameters = null;
	private int mShutterDelay = 0;
	private boolean mEnableShutterDelay;
	private int mShadingTable;
	private int mOutputSdblk;
	private int mIsoValIndex;
	private int mEvCalibration = 0;
    /**
     * This Handler is used to post message back onto the main thread of the
     * application
     */
    private class MainHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            // Elog.d(TAG, "MainHandler msg.what " + msg.what);
            switch (msg.what) {
                case CLEAR_SCREEN_DELAY:
                    getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    break;
                case FIRST_TIME_INIT:
                    initializeFirstTime();
                    break;
                case CHECK_DISPLAY_ROTATION:
                    if (Util.getDisplayRotation(Camera.this) != mDisplayRotation) {
                        setDisplayOrientation();
                    }
                    if (SystemClock.uptimeMillis() - mOnResumeTime < 5000) {
                        mHandler.sendEmptyMessageDelayed(CHECK_DISPLAY_ROTATION, 100);
                    }
                    break;
                case UPDATE_STORAGE:
                    checkStorage();
                    break;
                case PICTURES_SAVING_DONE:
                    if (!mPausing) {
                        if (mCameraState == SAVING_PICTURES) {
                            setCameraState(IDLE);
                            checkStorage();
                            mFocusManager.capture();
                            onShutterButtonFocus(false);
                            startPreview();
                        }
                    }
                    break;
                case EVENT_COMPLETE_CAPTURE:
                    mShutterButton.setEnabled(true);
                    mAfCaptureTimes = 0;
                    mTripleCount = 0;
                    // Init For next capture
                    mBracketTimes = 0;
                    mIsBracketAddPos = true;
                    if (isLockAeAwbAfMode(mAfMode) && !mEnableAeAwbLock) {
                        mEnableAeAwbLock = true;
                        mFocusManager.setAeAwbLock(false);
                    }
                    startPreview();
                    Elog.v(TAG, "Enabled mCaptureBtn");
                    break;
                case EVENT_START_CAPTURE:
                    mShutterButton.setEnabled(false);
                    Elog.v(TAG, "Disabled mCaptureBtn");
                    resetPrivateParams();
                    break;
                case EVENT_CAPTURE_ACTION:
                    Elog.v(TAG, "EVENT_CAPTURE_ACTION af mode " + mAfMode);
                    if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
                        Elog.d(TAG, "EVENT_CAPTURE_ACTION return1");
                        return;
                    }
                    
                    if (!isStorageEnough4Capture()) {
                		mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                		Toast.makeText(Camera.this, "storage space is not enough or unavailable", Toast.LENGTH_LONG).show();
                		Elog.d(TAG, "[FOR_NATA_CAMERA_NO_AVAILABLE_STORAGE]");
                		return;
                	}
                    Elog.d(TAG, "mEnableShutterDelay:" + mEnableShutterDelay + " mShutterDelay:" + mShutterDelay);
                    if (mEnableShutterDelay && mShutterDelay > 0) {
                        sendEmptyMessageDelayed(MSG_ID_SHUTTER_DELAY_DONE, mShutterDelay * 1000);
                        return;
                    }
                    
                    mParameters = mCameraDevice.getParameters();
                    if (mAfMode == AF_MODE_BRACKET) {
                        if (mAfCaptureTimes != 0) { // Auto is best step
                            if (mAfCaptureTimes == 1) {
                                mBracketTimes = 0;
                                mBracketBestPos = mParameters.getBestFocusStep();
                                mBracketMaxPos = mParameters.getMaxFocusStep();
                                mBracketMinPos = mParameters.getMinFocusStep();
                                Elog.i(TAG, "bracket max " + mBracketMaxPos + " bracket min " + mBracketMinPos
                                        + " bracket best " + mBracketBestPos);
                            }
                            mBracketTimes++;
                            if (mIsBracketAddPos) {
                                mPosValue = mBracketBestPos + mAfBracketIntervel * mBracketTimes;
                                if (mPosValue > mBracketMaxPos) {
                                    mIsBracketAddPos = false;
                                    mBracketTimes = 0;
                                    mAfCaptureTimes++; // prevent if First
                                                       // mPosValue > max
                                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                                    return;
                                }
                            } else {
                                mPosValue = mBracketBestPos - mAfBracketIntervel * mBracketTimes;
                                if (mPosValue < mBracketMinPos || mBracketTimes > mAfBracketRange) {
                                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                                    return;
                                }
                            }
                            Elog.d(TAG, "bracket position " + mPosValue);
                            mParameters.setFocusEngStep(mPosValue);
                            mCameraDevice.setParameters(mParameters);
                            try { // set step need wait 100ms before capture
                                Thread.sleep(100);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                        mAfCaptureTimes++;
                    }
                    if (mAfMode == AF_MODE_THROUGH_FOCUS) {
                        if (mAfCaptureTimes == 0) {
                            mBracketTimes = 0;
                            mBracketMaxPos = mParameters.getMaxFocusStep();
                            mBracketMinPos = mParameters.getMinFocusStep();
                            Elog.i(TAG, "through max " + mBracketMaxPos + " through min " + mBracketMinPos);
                            if (mAfThroughDirect == 0) {
                                mPosValue = mBracketMaxPos;
                                mIsBracketAddPos = false;
                            } else if (mAfThroughDirect == 1) {
                                mPosValue = mBracketMinPos;
                                mIsBracketAddPos = true;
                            } else { // Manual configure
                                if (mThroughFocusStartPos <= mThroughFocusEndPos) {
                                    mIsBracketAddPos = true;
                                    if (mThroughFocusStartPos >= mBracketMinPos) {
                                        mPosValue = mThroughFocusStartPos;
                                    } else {
                                        mPosValue = mBracketMinPos;
                                        setThroughFocusManualPos(R.string.auto_clibr_key_through_manual_start_pos,
                                                mBracketMinPos);
                                    }
                                    if (mThroughFocusEndPos <= mBracketMaxPos) {
                                        mBracketMaxPos = mThroughFocusEndPos;
                                    } else {
                                        setThroughFocusManualPos(R.string.auto_clibr_key_through_manual_end_pos,
                                                mBracketMaxPos);
                                    }
                                } else if (mThroughFocusStartPos > mThroughFocusEndPos) {
                                    mIsBracketAddPos = false;
                                    if (mThroughFocusStartPos <= mBracketMaxPos) {
                                        mPosValue = mThroughFocusStartPos;
                                    } else {
                                        mPosValue = mBracketMaxPos;
                                        setThroughFocusManualPos(R.string.auto_clibr_key_through_manual_start_pos,
                                                mBracketMaxPos);
                                    }
                                    if (mThroughFocusEndPos >= mBracketMinPos) {
                                        mBracketMinPos = mThroughFocusEndPos;
                                    } else {
                                        setThroughFocusManualPos(R.string.auto_clibr_key_through_manual_end_pos,
                                                mBracketMinPos);
                                    }
                                }
                            }
                        } else {
                            mBracketTimes++;
                            if (mIsBracketAddPos) {
                                //mPosValue = mBracketMinPos + mAfThroughIntervel * mBracketTimes;
                            	if (mAfThroughRepearPointer == mAfThroughRepeat) {
                                    mPosValue = mPosValue + mAfThroughIntervel;
                                    mAfThroughRepearPointer = 0;
                            	}
                                if (mPosValue > mBracketMaxPos) {
                                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                                    return;
                                }
                            } else {
                                //mPosValue = mBracketMaxPos - mAfThroughIntervel * mBracketTimes;
                                if (mAfThroughRepearPointer == mAfThroughRepeat) {
                                    mPosValue = mPosValue - mAfThroughIntervel;
                                    mAfThroughRepearPointer = 0;
                                }
                            	if (mPosValue < mBracketMinPos) {
                                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                                    return;
                                }
                            }
                        }
                        Elog.d(TAG, "through position " + mPosValue);
                        mParameters.setFocusEngStep(mPosValue);
                        mCameraDevice.setParameters(mParameters);
                        try { // set step need wait 100ms before capture
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        mAfThroughRepearPointer++;
                        mAfCaptureTimes++;
                    }
                    capture();
                    break;
                case EVENT_PREVIEW_RAW_DUMP:
                    Elog.e(TAG, "EVENT_PREVIEW_RAW_DUMP");
                    onShutterButtonFocus(true);
                    break;
                case EVENT_WAITING_DONE:
//                    Elog.d(TAG, "EVENT_WAITING_DONE: at:" + System.currentTimeMillis());
//                    mIsNeedWait = false;
//                    sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    break;
                case MSG_ID_UPDATE_CAMERA_INFO:
                    if (!mPausing) {
                        updateUiCameraInfo();
                        sendEmptyMessageDelayed(MSG_ID_UPDATE_CAMERA_INFO, INTERVAL_MS_UPDATE_CAMERA_INFO);
                    }
                    break;
                case MSG_ID_SHUTTER_DELAY_DONE:
                    if (!mPausing) {
                        mEnableShutterDelay = false;
                        sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    private void updateUiCameraInfo() {
        mParameters = mCameraDevice.getParameters();
        float previewFPS = mParameters.getEngPreviewFrameIntervalInUS();
        if (previewFPS > 0) {
            previewFPS = 1000000 / previewFPS;
        }
        else {
            previewFPS = 0;
        }

        String format = "AEid:%d PSS:%d: PSG:%d PISPG:%d CSS:%d CSG:%d CISPG:%d PFPS:%.1f";
        String info = String.format(format, mParameters.getEngPreviewAEIndex(),
                mParameters.getEngPreviewShutterSpeed(),
                mParameters.getEngPreviewSensorGain(),
                mParameters.getEngPreviewISPGain(),
                mParameters.getEngCaptureShutterSpeed(),
                mParameters.getEngCaptureSensorGain(),
                mParameters.getEngCaptureISPGain(),
                previewFPS);
        mTvCameraInfo.setText(info);
        Elog.d(TAG, "updateUiCameraInfo():" + info);
    }

	private void resetPrivateParams() {
	    mAfThroughRepearPointer = 0;
	    mCaptureDoneNumber = 0;
	    mEnableShutterDelay = true;
	}

    private void setThroughFocusManualPos(int keyId, int pos) {
        final SharedPreferences preferences = getSharedPreferences(AutoCalibration.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putInt(getString(keyId), pos);
        editor.commit();
    }

    private void capture() {
        Elog.i(TAG, "capture()");
        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
            Elog.d(TAG, "capture() return1");
            return;
        }
        onShutterButtonFocus(true);
    }
    
    private boolean isStorageEnough4Capture() {
       	Elog.d(TAG, "isStorageEnough() captureType:" + mCaptureType);
    	long available = getAvailableSpace();
    	
    	if (mCaptureType == 2) { // JPEG_ONLY
    		if (available > JPEG_MAX_SIZE) {
    			return true;
    		} else {
    			return false;
    		}
    	} else {
    		if (available > RAW_JPEG_MAX_SIZE) {
    			return true;
    		} else {
    			return false;
    		}
    	}
    }

    // Capture image need wait auto focus done
    @Override
    public void onAutoFocusDone() {
        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
            Elog.d(TAG, "onAutoFocusDone() return1");
            return;
        }
        Elog.i(TAG, "onAutoFocusDone()");
        // when touch focus, but no click shutter button, only do auto focus
        if (mFocusManager.isTouchFocusMode() && mShutterButton.isEnabled()) {
            return;
        }
        if (mAfMode == AF_MODE_CONTINUOUS) {
            Elog.d(TAG, "onAutoFocusDone RETURN FOR AF_MODE_CONTINUOUS");
            return;
        }
        
        if (mCaptureMode == 0) {
            AfCaptureThread afAutoThread = new AfCaptureThread();
            afAutoThread.start();
        } else if (mCaptureMode == 2) {
            mParameters.setRawDumpFlag(true);
            mCameraDevice.setParameters(mParameters);
        }
    }

    class AfCaptureThread extends Thread {
        public void run() {
            Elog.v(TAG, "AfCaptureThread start.");
            mTakePicDone = false;
            takePicture();
            while (!mTakePicDone) {
                try {
                    sleep(100);
                } catch (InterruptedException e) {
                    Elog.e(TAG, e.getMessage());
                }
            }
            if (mAfMode == AF_MODE_AUTO) { // auto
                if (mAfCaptureTimes > mIsoValues.size()) { // count
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                } else {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
            } else if (mAfMode == AF_MODE_BRACKET) { // bracket
                if (mBracketTimes == mAfBracketRange) {
                    if (mIsBracketAddPos) {
                        mIsBracketAddPos = false;
                        mBracketTimes = 0;
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    } else {
                        mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    }
                } else {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
            } else if (mAfMode == AF_MODE_FULL_SCAN) { // full scan
                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
            } else if (mAfMode == AF_MODE_THROUGH_FOCUS) { // through focus
                mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
            } else if (mAfMode == AF_MODE_CONTINUOUS) {
                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
            }
            Elog.v(TAG, "AfCaptureThread finish.");
        }
    }

    private void takePicture() {
        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
            Elog.d(TAG, "takePicture() return1");
            return;
        }
        Elog.v(TAG, "takePicture() start");
        mCaptureStartTime = System.currentTimeMillis();
        mJpegImageData = null;
        // Set rotation data.
        Util.setRotationParameter(mParameters, mCameraId, mOrientation);
        mCameraDevice.setParameters(mParameters);
        mCameraDevice.takePicture(mShutterCallback, mRawPictureCallback, new JpegPictureCallback());
    }

    // Snapshots can only be taken after this is called. It should be called
    // once only. We could have done these things in onCreate() but we want to
    // make preview screen appear as soon as possible.
    private void initializeFirstTime() {
        Elog.d(TAG, "initializeFirstTime()");
        if (mFirstTimeInitialized) {
            Elog.d(TAG, "initializeFirstTime() return 1");
            return;
        }
        // Create orientation listenter. This should be done first because it
        // takes some time to get first orientation.
        mOrientationListener = new MyOrientationEventListener(Camera.this);
        mOrientationListener.enable();
        checkStorage();
        // Initialize shutter button.
        mShutterButton.setOnTouchListener(this);
        // Initialize focus UI.
        mPreviewFrame = findViewById(R.id.camera_preview);
        mPreviewFrame.setOnTouchListener(this);
        mFocusAreaIndicator = (RotateLayout) findViewById(R.id.focus_indicator_rotate_layout);
        CameraInfo info = CameraHolder.instance().getCameraInfo()[mCameraId];
        boolean mirror = (info.facing == CameraInfo.CAMERA_FACING_FRONT);
        mFocusManager.initialize(mFocusAreaIndicator, mPreviewFrame, this, mirror, mDisplayOrientation);
        Util.initializeScreenBrightness(getWindow(), getContentResolver());
        installIntentFilter();
        mFirstTimeInitialized = true;
        long switchEndTime = System.currentTimeMillis();
        Elog.i(TAG, "[Performance test][Camera][Camera] camera switch Main To Sub end [" + switchEndTime + "]");
    }

    // If the activity is paused and resumed, this method will be called in
    // onResume.
    private void initializeSecondTime() {
        Elog.d(TAG, "initializeSecondTime()");
        // Start orientation listener as soon as possible because it takes
        // some time to get first orientation.
        mOrientationListener.enable();
        installIntentFilter();
        checkStorage();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent m) {
        // Elog.d(TAG, "dispatchTouchEvent()");
        // Check if the popup window should be dismissed first.
        if (mPopupGestureDetector != null && mPopupGestureDetector.onTouchEvent(m)) {
            return true;
        }
        return super.dispatchTouchEvent(m);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.d(TAG, "Received intent action=" + action);
            if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
                checkStorage();
            } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED) || action.equals(Intent.ACTION_MEDIA_CHECKING)) {
                checkStorage();
            } else if (action.equals(Intent.ACTION_MEDIA_SCANNER_FINISHED)) {
                checkStorage();
            }
        }
    };

    private final class ShutterCallback implements android.hardware.Camera.ShutterCallback {
        public void onShutter() {
            mShutterCallbackTime = System.currentTimeMillis();
            mShutterLag = mShutterCallbackTime - mCaptureStartTime;
            Elog.v(TAG, "mShutterLag = " + mShutterLag + "ms");
            // mFocusManager.onShutter();
        }
    }

    private final class RawPictureCallback implements PictureCallback {
        public void onPictureTaken(byte[] rawData, android.hardware.Camera camera) {
            mRawPictureCallbackTime = System.currentTimeMillis();
            Elog.v(TAG, "mShutterToRawCallbackTime = " + (mRawPictureCallbackTime - mShutterCallbackTime) + "ms");
        }
    }

    private final class JpegPictureCallback implements PictureCallback {

        @Override
        public void onPictureTaken(byte[] jpegData, android.hardware.Camera camera) {
            if (mPausing) {
                return;
            }
            mJpegImageData = jpegData;
            doAttach();
        }
    }

    private void doAttach() {
        Elog.v(TAG, "doAttach()");
        setCameraState(SAVING_PICTURES);
        String jpegName = mRawCaptureFileName;
        // ISO is Auto, get real value
        if (mIsoValues != null && "0".equals(mIsoValues.get(mIsoValIndex))) {
            mParameters = mCameraDevice.getParameters();
            jpegName = jpegName + mParameters.getEngCaptureISO();
        } 
        jpegName = jpegName + ".jpg";
        Elog.v(TAG, "Jpeg name is " + jpegName);
        File fHandle = new File(jpegName);
        try {
            OutputStream bos = new FileOutputStream(fHandle);
            bos.write(mJpegImageData);
            bos.close();
        } catch (IOException ex) {
            fHandle.delete();
        }
        mHandler.sendEmptyMessage(PICTURES_SAVING_DONE);
        mCaptureDoneNumber++;
    }

    private static String createNameJpeg(long dateTaken) {
        return DateFormat.format("yyyy-MM-dd kk.mm.ss", dateTaken).toString();
    }

    private final class AutoFocusCallback implements android.hardware.Camera.AutoFocusCallback {
        public void onAutoFocus(boolean focused, android.hardware.Camera camera) {
            if (mPausing) {
                return;
            }

            mAutoFocusTime = System.currentTimeMillis() - mFocusStartTime;
            Elog.v(TAG,
                    "mAutoFocusTime = " + mAutoFocusTime + "ms" + ", camera State = " + String.valueOf(mCameraState));
            if (mCameraState == FOCUSING) {
                setCameraState(IDLE);
            }
            mFocusManager.doSnap();
            mFocusManager.onAutoFocus(focused);
            sIsAutoFocusCallback = true;
        }
    }

    private final class PreviewRawDumpCallback implements android.hardware.Camera.PreviewRawDumpCallback {
        public void onNotify(int code) {
            Elog.e(TAG, "onNotify code " + code);
            mParameters.setRawDumpFlag(false);
            mCameraDevice.setParameters(mParameters);
            mFocusManager.capture();
            onShutterButtonFocus(false);
            startPreview();
            mShutterButton.setEnabled(true);
        }
    }

    private void setCameraState(int state) {
        Elog.d(TAG, "setCameraState() state " + state);
        mCameraState = state;
    }

    @Override
    public void setFocusParameters() {
        Elog.d(TAG, "setFocusParameters()");
        if (mCameraDevice == null) {
            return;
        }
        mParameters = mCameraDevice.getParameters();
        if ((mFocusAreaSupported) && (!sIsAutoFocusCallback)) {
            mParameters.setFocusAreas(mFocusManager.getFocusAreas());
        }
        if ((mMeteringAreaSupported) && (!sIsAutoFocusCallback)) {
            mParameters.setMeteringAreas(mFocusManager.getMeteringAreas());
        }
        mCameraDevice.setParameters(mParameters);
        sIsAutoFocusCallback = false;
    }

    Thread mCameraOpenThread = new Thread(new Runnable() {
        @Override
        public void run() {
            try {
                Elog.d(TAG, "mCameraOpenThread start");
                // set Eng mode before Camera open
                android.hardware.Camera.setProperty(ROPERTY_KEY_CLIENT_APPMODE, APP_MODE_NAME_MTK_ENG);
                mCameraDevice = Util.openCamera(Camera.this, mCameraId);
            } catch (CameraHardwareException e) {
                mOpenCameraFail = true;
            } catch (CameraDisabledException e) {
                mCameraDisabled = true;
            }
        }
    });

    Thread mCameraPreviewThread = new Thread(new Runnable() {
        @Override
        public void run() {
            Elog.d(TAG, "mCameraPreviewThread start");
            initializeCapabilities();
            startPreview();
        }
    });

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Elog.i(TAG, "onCreate Bundle = " + String.valueOf(icicle));
        String[] defaultFocusModes = getResources().getStringArray(R.array.pref_camera_focusmode_default_array);
        mFocusManager = new FocusManager(defaultFocusModes);
        getSettingsFromPref();
        /*
         * To reduce startup time, we start the camera open and preview threads.
         * We make sure the preview is started at the end of onCreate.
         */
        mCameraOpenThread.start();
        setContentView(R.layout.mt6589_camera);
        // don't set mSurfaceHolder here. We have it set ONLY within
        // surfaceChanged / surfaceDestroyed, other parts of the code
        // assume that when it is set, the surface is also set.
        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.camera_preview);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        // Make sure camera device is opened.
        try {
            mCameraOpenThread.join();
            mCameraOpenThread = null;
            if (mOpenCameraFail) {
                Util.showErrorAndFinish(this, R.string.cannot_connect_camera);
                return;
            } else if (mCameraDisabled) {
                Util.showErrorAndFinish(this, R.string.camera_disabled);
                return;
            }
        } catch (InterruptedException ex) {
            Elog.w(TAG, ex.getMessage());
        }
        mCameraPreviewThread.start();
        // mOnScreenIndicators = (Rotatable)
        // findViewById(R.id.on_screen_indicators);

        mShutterButton = (Button) findViewById(R.id.capture_btn);
        Intent intent = getIntent();
        mEvCalibration = intent.getIntExtra(getString(R.string.camera_key_ev_calibration), 
                AutoCalibration.ID_EV_CALIBRATION_NONE);
        if (mEvCalibration == AutoCalibration.ID_EV_CALIBRATION_ACTION) {
            mShutterButton.setText(getString(R.string.camera_ev_calibration));
        }
        // Wait until the camera settings are retrieved.
        synchronized (mCameraPreviewThread) {
            try {
                if (!mSettingRetrieved) {
                    mCameraPreviewThread.wait();
                }
            } catch (InterruptedException ex) {
                Elog.w(TAG, ex.getMessage());
            }
        }

        // Make sure preview is started.
        try {
            mCameraPreviewThread.join();
        } catch (InterruptedException ex) {
            Elog.w(TAG, ex.getMessage());
        }
        mCameraPreviewThread = null;
        
        mTvCameraInfo = (TextView)findViewById(R.id.camear_info_tv);
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    private void getSettingsFromPref() {
        Elog.d(TAG, "getSettingsFromPref");
        final SharedPreferences preferences = getSharedPreferences(AutoCalibration.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        mCaptureMode = preferences.getInt(getString(R.string.auto_clibr_key_capture_mode), 0);
        if (mCaptureMode == 0) {
            mCaptureSize = preferences.getInt(getString(R.string.auto_clibr_key_capture_size), 0);
            mCaptureType = preferences.getInt(getString(R.string.auto_clibr_key_capture_type), 0);
            mCaptureNumber = preferences.getInt(getString(R.string.auto_clibr_key_capture_number), 0);
        } else if (mCaptureMode == 1) {
            mCaptureNumber = preferences.getInt(getString(R.string.auto_clibr_key_capture_number), 0);
        } else {
            mVideoResolution = preferences.getInt(getString(R.string.auto_clibr_key_capture_resolution), 0);
        }

        if (preferences.getInt(getString(R.string.auto_clibr_key_flicker), 0) == 0) {
            mFlickerString = "50hz";
        } else {
            mFlickerString = "60hz";
        }
        mStrobeMode = STROBE_MODE[preferences.getInt(getString(R.string.auto_clibr_key_led_flash), 0)];
        mPreFlashLevel = preferences.getInt(getString(R.string.auto_clibr_key_pre_flash), 1);
        mMainFlashLevel = preferences.getInt(getString(R.string.auto_clibr_key_main_flash), 1);
        mAfMode = preferences.getInt(getString(R.string.auto_clibr_key_af_mode), 0);
        if (mAfMode == AF_MODE_BRACKET) {
            mAfBracketIntervel = preferences.getInt(getString(R.string.auto_clibr_key_branket_interval), 1);
            mAfBracketRange = preferences.getInt(getString(R.string.auto_clibr_key_branket_range), 0);
        } else if (mAfMode == AF_MODE_THROUGH_FOCUS) {
            mAfThroughDirect = preferences.getInt(getString(R.string.auto_clibr_key_through_focus_dirct), 0);
            if (mAfThroughDirect == 2) {
                mThroughFocusStartPos = preferences.getInt(getString(R.string.auto_clibr_key_through_manual_start_pos),
                        0);
                mThroughFocusEndPos = preferences.getInt(getString(R.string.auto_clibr_key_through_manual_end_pos),
                        1023);
            }
            mAfThroughIntervel = preferences.getInt(getString(R.string.auto_clibr_key_through_focus_interval), 1);
        }
        getIsoValue(mAfMode, preferences.getString(getString(R.string.auto_clibr_key_iso_speed), "0"));
        
        mAfFullScanFrameInterval = preferences.getInt(getString(R.string.auto_clibr_key_full_frame_interval), 0);
        mAfThroughRepeat = preferences.getInt(getString(R.string.auto_clibr_key_through_repeat), 1);
        mCustomParameters = preferences.getString(getString(R.string.camera_key_custom_parameter), "ERROR_INVALID_PARAM");
        Elog.d(TAG, "mCustomParameters: " + mCustomParameters);
        mCameraId = preferences.getInt(getString(R.string.camera_key_selected_sensor_id), 0);
        mShutterDelay = preferences.getInt(getString(R.string.camera_key_shutter_delay), 0);
        mShadingTable = preferences.getInt(getString(R.string.camera_key_shading_table), 0);
        mOutputSdblk = preferences.getInt(getString(R.string.camera_key_output_sdblk), 0);
    }
    
    // get custom parameter by index, which start from 0
    String getCustomParameter(int index) {
        String[] parameters = mCustomParameters.split(";", -1);
        if (index < 0 || index >= parameters.length) {
            throw new IllegalArgumentException("index must be 0 to " + (parameters.length - 1) + "; current is " + index);
        }
        return parameters[index];
    }

    private void getIsoValue(int afMode, String iso) {
        Elog.d(TAG, "getIsoValue iso " + iso + " af mode " + afMode);
        if (afMode != 0) {
            mIsoValue = iso;
        } else {
            String[] tempStrings = iso.split(",");
            if (tempStrings != null) {
                if (mIsoValues == null) {
                    mIsoValues = new ArrayList<String>();
                }
                for (int i = 0; i < tempStrings.length; i++) {
                    Elog.d(TAG, "get is : " + tempStrings[i]);
                }
                mIsoValues.addAll(Arrays.asList(tempStrings));
            } else {
                Elog.d(TAG, "mIsoValues == null");
            }
        }
    }

    private class MyOrientationEventListener extends OrientationEventListener {
        public MyOrientationEventListener(Context context) {
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            // We keep the last known orientation. So if the user first orient
            // the camera then point the camera to floor or sky, we still have
            // the correct orientation.
            if (orientation == ORIENTATION_UNKNOWN) {
                return;
            }
            mOrientation = Util.roundOrientation(orientation, mOrientation);
            // When the screen is unlocked, display rotation may change. Always
            // calculate the up-to-date orientationCompensation.
            int orientationCompensation = mOrientation + Util.getDisplayRotation(Camera.this);
            if (mOrientationCompensation != orientationCompensation) {
                mOrientationCompensation = orientationCompensation;
                setOrientationIndicator(mOrientationCompensation);
            }
        }
    }

    private void setOrientationIndicator(int orientation) {
        Elog.d(TAG, "setOrientationIndicator() orientation " + orientation);
        Rotatable[] indicators = { mFocusAreaIndicator };
        for (Rotatable indicator : indicators) {
            if (indicator != null) {
                indicator.setOrientation(orientation);
            }
        }

        if (mOnScreenIndicators != null) {
            mOnScreenIndicators.setOrientation(orientation);
        }
    }

    private void checkStorage() {
        Elog.d(TAG, "checkStorage()");
        mPicturesRemaining = getAvailableSpace();
        if (mPicturesRemaining == PREPARING) {
            toastOnUiThread(R.string.preparing_sd);
            return;
            //Toast.makeText(this, R.string.preparing_sd, Toast.LENGTH_LONG).show();
        } else if (mPicturesRemaining == UNAVAILABLE) {
            toastOnUiThread(R.string.auto_clibr_sdcard_tips);
            return;
            //Toast.makeText(this, R.string.auto_clibr_sdcard_tips, Toast.LENGTH_LONG).show();
        }
        if (mPicturesRemaining > LOW_STORAGE_THRESHOLD) {
            long pictureSize = PICTURE_SIZE;
            mPicturesRemaining = (mPicturesRemaining - LOW_STORAGE_THRESHOLD) / pictureSize;
        } else if (mPicturesRemaining > 0) {
            mPicturesRemaining = 0;
        }
        if (mCameraState == IDLE) {
            if (mPicturesRemaining < 0) {
                toastOnUiThread(R.string.not_enough_space);
                //Toast.makeText(this, R.string.not_enough_space, Toast.LENGTH_LONG).show();
            } else {
                Elog.w(TAG, "can take " + mPicturesRemaining + " photos.");
                // mRemainPictureView.setText(String.valueOf(mPicturesRemaining));
            }
        }
    }
    
    private void toastOnUiThread(final int strId) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(Camera.this, strId, Toast.LENGTH_LONG).show();
            }
            
        });
    }

    private long getAvailableSpace() {
        String state;
        // get default sdcard path
        if (mStorageManager == null) {
            mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        }
        mCameraImageName = StorageManagerEx.getDefaultPath() + "/DCIM/CameraEM/";
        state = mStorageManager.getVolumeState(StorageManagerEx.getDefaultPath());
        Elog.d(TAG, "External storage state=" + state + ", mount point = " + StorageManagerEx.getDefaultPath());
        if (Environment.MEDIA_CHECKING.equals(state)) {
            return PREPARING;
        }
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            return UNAVAILABLE;
        }

        File dir = new File(mCameraImageName);
        dir.mkdirs();
        if (!dir.isDirectory() || !dir.canWrite()) {
            return FULL_SDCARD;
        }
        // try {
        StatFs stat = new StatFs(mCameraImageName);
        return stat.getAvailableBlocks() * (long) stat.getBlockSize();
        // } catch (Exception e) {
        // Elog.i(TAG, "Fail to access external storage" + e.getMessage());
        // }
        // return UNKNOWN_SIZE;
    }

    // @Override before auto focus set to true, after set to false
    public void onShutterButtonFocus(boolean pressed) {
        if (mPausing || mCameraState == SNAPSHOT_IN_PROGRESS) {
            return;
        }
        // Do not do focus if there is not enough storage.
        if (pressed && !canTakePicture()) {
            return;
        }
        Elog.i(TAG, "onShutterButtonFocus pressed = " + String.valueOf(pressed));
        if (pressed) {
            mFocusManager.onShutterDown();
            if (mCaptureMode == 0 && mAfMode == AF_MODE_CONTINUOUS) {
                Elog.d(TAG, "onShutterButtonFocus AF_MODE_CONTINUOUS");
                AfCaptureThread afAutoThread = new AfCaptureThread();
                afAutoThread.start();
            }
        } else {
            mFocusManager.onShutterUp();
        }
    }

    // install an intent filter to receive SD card related events.
    private void installIntentFilter() {
        Elog.v(TAG, "installIntentFilter()");
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        registerReceiver(mReceiver, intentFilter);
        mDidRegister = true;
    }

    @Override
    protected void doOnResume() {
        if (mOpenCameraFail || mCameraDisabled) {
            return;
        }
        mPausing = false;
        Elog.i(TAG, "doOnResume() Camera State = " + String.valueOf(mCameraState));
        // Start the preview if it is not started.
        if (mCameraState == PREVIEW_STOPPED) {
            try {
                // set Eng mode before Camera open
                android.hardware.Camera.setProperty(ROPERTY_KEY_CLIENT_APPMODE, APP_MODE_NAME_MTK_ENG);
                mCameraDevice = Util.openCamera(this, mCameraId);
                initializeCapabilities();
                startPreview();
            } catch (CameraHardwareException e) {
                Util.showErrorAndFinish(this, R.string.cannot_connect_camera);
                return;
            } catch (CameraDisabledException e) {
                Util.showErrorAndFinish(this, R.string.camera_disabled);
                return;
            }
        }

        if (mSurfaceHolder != null) {
            // If first time initialization is not finished, put it in the
            // message queue.
            if (!mFirstTimeInitialized) {
                mHandler.sendEmptyMessage(FIRST_TIME_INIT);
            } else {
                initializeSecondTime();
            }
        }
        keepScreenOnAwhile();
        if (mCameraState == IDLE) {
            mOnResumeTime = SystemClock.uptimeMillis();
            mHandler.sendEmptyMessageDelayed(CHECK_DISPLAY_ROTATION, 100);
        }
        updateUiCameraInfo();
        mHandler.sendEmptyMessageDelayed(MSG_ID_UPDATE_CAMERA_INFO, INTERVAL_MS_UPDATE_CAMERA_INFO);
        Elog.i(TAG, "doOnresume end");
    }

    @Override
    protected void onPause() {
        Elog.i(TAG, "onPause()");
        mPausing = true;
        mSnapshotOnIdle = false;

        stopPreview();
        // Close the camera now because other activities may need to use it.
        closeCamera();
        resetScreenOn();

        // Clear UI.
        if (mFirstTimeInitialized) {
            mOrientationListener.disable();
        }

        if (mDidRegister) {
            unregisterReceiver(mReceiver);
            mDidRegister = false;
        }
        // If we are in an image capture intent and has taken
        // a picture, we just clear it in onPause.
        mJpegImageData = null;

        // Remove the messages in the event queue.
        mHandler.removeMessages(FIRST_TIME_INIT);
        mHandler.removeMessages(CHECK_DISPLAY_ROTATION);
        mFocusManager.removeMessages();

        // Mediatek merge begin
        sIsAutoFocusCallback = false;
        // Mediatek merge end
        if (mShutterButton.isEnabled()) {
            mAfCaptureTimes = 0;
            mTripleCount = 0;
            // Init For next capture
            mBracketTimes = 0;
            mIsBracketAddPos = true;
        }
        super.onPause();
    }

    protected boolean canTakePicture() {
        boolean retVal = isCameraIdle();
        // retVal = retVal && (mPicturesRemaining > 0);
        Elog.i(TAG, "canTakePicture() " + retVal);
        return retVal;
    }

    @Override
    public void autoFocus() {
        Elog.i(TAG, "autoFocus");
        mFocusStartTime = System.currentTimeMillis();
        mCameraDevice.autoFocus(mAutoFocusCallback);
        setCameraState(FOCUSING);
    }

    @Override
    public void cancelAutoFocus() {
        Elog.i(TAG, "cancelAutoFocus");
        mCameraDevice.cancelAutoFocus();
        if (mCameraState != SELFTIMER_COUNTING && mCameraState != SNAPSHOT_IN_PROGRESS) {
            setCameraState(IDLE);
        }
        setFocusParameters();
    }
    
    private void showMsgDialog(String title, String msg) {
        AlertDialog dialog = new AlertDialog.Builder(this).setTitle(title).
                setMessage(msg).setPositiveButton(android.R.string.ok, null).create();
        dialog.show();
    }
    
    private void showEvCalibrationDlg() {
        String title = getString(R.string.camera_ev_calibr_input_tip);
        showSingleInputDlg(title, new InputDialogOnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which, String inputStr) {
                int value = 0;
                boolean validInput = true;
                try {
                    value = Integer.parseInt(inputStr);
                } catch (NumberFormatException e) {
                    validInput = false;
                }
                if (validInput) {
                    Parameters param = mCameraDevice.getParameters();
                    int offset = param.getEngEVCalOffset() - value + 50;

                    if (offset < -30 || offset > 30)
                    {
                        Toast.makeText(Camera.this, "Offset must be -30 ~ 30", Toast.LENGTH_SHORT).show();
                    }
                    else
                    {
                        String msg = "EV Offset:" + offset;
                        showMsgDialog(getString(R.string.camera_ev_calibration), msg);
                        dialog.dismiss();
                    }
                } else {
                    Toast.makeText(Camera.this, "Invalid LV", Toast.LENGTH_SHORT).show();
                }              
            }
        }, "");
    }

    // Preview area is touched. Handle touch focus.
    @Override
    public boolean onTouch(View v, MotionEvent e) {
        // Elog.d(TAG, "onTouch ");
        if (mPausing || mCameraDevice == null || !mFirstTimeInitialized || mCameraState == SNAPSHOT_IN_PROGRESS
                || mCameraState == PREVIEW_STOPPED || mCameraState == SAVING_PICTURES) {
            Elog.d(TAG, "onTouch return1");
            return false;
        }
        if (v.equals(mShutterButton)) {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                if (mEvCalibration == AutoCalibration.ID_EV_CALIBRATION_ACTION) {
                    showEvCalibrationDlg();
                    return true;
                }
                Elog.v(TAG, "mCaptureBtn key up!");
                mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                if (mCaptureMode == 0) {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                } else if (mCaptureMode == 2) { // video raw dump
                    mHandler.sendEmptyMessage(EVENT_PREVIEW_RAW_DUMP);
                }
            }
            return true;
        }
        if (v.getId() == R.id.camera_preview) {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                if (mShutterButton.isEnabled()) {
                    Elog.v(TAG, "surfaceView touch up!");
                    return mFocusManager.onSingleTapUpPreview(v, e);
                }
            }
        }
        return true;
        // now no need support touch foucs
        // else {
        // // Check if metering area or focus area is supported.
        // if (!mFocusAreaSupported && !mMeteringAreaSupported) {
        // Elog.d(TAG, "onTouch return2");
        // return false;
        // }
        //
        // String focusMode = mParameters.getFocusMode();
        // if (focusMode == null ||
        // Parameters.FOCUS_MODE_INFINITY.equals(focusMode)) {
        // Elog.d(TAG, "onTouch return3");
        // return false;
        // }
        // return mFocusManager.onTouch(e);
        // }
    }

    @Override
    public void onBackPressed() {
        if (!isCameraIdle()) {
            return;
        }
        super.onBackPressed();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // Make sure we have a surface in the holder before proceeding.
        if (holder.getSurface() == null) {
            Elog.d(TAG, "holder.getSurface() == null");
            return;
        }
        Elog.v(TAG, "surfaceChanged. w=" + width + ". h=" + height);

        // We need to save the holder for later use, even when the mCameraDevice
        // is null. This could happen if onResume() is invoked after this
        // function.
        if (holder == null) {
            Elog.v(TAG, "surfaceChanged. holder == null");
        }
        mSurfaceHolder = holder;

        // The mCameraDevice will be null if it fails to connect to the camera
        // hardware. In this case we will show a dialog and then finish the
        // activity, so it's OK to ignore it.
        if (mCameraDevice == null) {
            return;
        }

        // Sometimes surfaceChanged is called after onPause or before onResume.
        // Ignore it.
        if (mPausing || isFinishing()) {
            return;
        }

        // Set preview display if the surface is being created. Preview was
        // already started. Also restart the preview if display rotation has
        // changed. Sometimes this happens when the device is held in portrait
        // and camera app is opened. Rotation animation takes some time and
        // display rotation in onCreate may not be what we want.
        if (mCameraState == PREVIEW_STOPPED) {
            startPreview();
        } else {
            if (Util.getDisplayRotation(this) != mDisplayRotation) {
                setDisplayOrientation();
            }
            if (holder.isCreating()) {
                // Set preview display if the surface is being created and
                // preview
                // was already started. That means preview display was set to
                // null
                // and we need to set it now.
                setPreviewDisplay(holder);
            }
        }

        // If first time initialization is not finished, send a message to do
        // it later. We want to finish surfaceChanged as soon as possible to let
        // user see preview first.
        if (!mFirstTimeInitialized) {
            mHandler.sendEmptyMessage(FIRST_TIME_INIT);
        } else {
            initializeSecondTime();
        }
    }

    public void surfaceCreated(SurfaceHolder holder) {
        Elog.v(TAG, "surfaceCreated.");
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        Elog.v(TAG, "surfaceDestroyed()");
        stopPreview();
        mSurfaceHolder = null;
    }

    private void closeCamera() {
        if (mCameraDevice != null) {
            CameraHolder.instance().release();
            mCameraDevice.setErrorCallback(null);
            mCameraDevice.setPreviewRawDumpCallback(null);
            // mFocusManager.unRegisterCAFCallback(mCameraDevice);
            mCameraDevice = null;
            setCameraState(PREVIEW_STOPPED);
            mFocusManager.onCameraReleased();
        }
    }

    private void setPreviewDisplay(SurfaceHolder holder) {
        try {
            mCameraDevice.setPreviewDisplay(holder);
        } catch (IOException ex) {
            closeCamera();
            throw new RuntimeException("setPreviewDisplay failed", ex);
        }
    }

    private void setDisplayOrientation() {
        mDisplayRotation = Util.getDisplayRotation(this);
        mDisplayOrientation = Util.getDisplayOrientation(mDisplayRotation, mCameraId);
        mCameraDevice.setDisplayOrientation(mDisplayOrientation);
    }

    private void startPreview() {
        Elog.d(TAG, "startPreview()");
        if (mPausing || isFinishing()) {
            Elog.d(TAG, "startPreview() return1");
            return;
        }
        mFocusManager.resetTouchFocus();
        mCameraDevice.setErrorCallback(mErrorCallback);
        mCameraDevice.setPreviewRawDumpCallback(mPreviewRawDumpCallback);
        // If we're previewing already, stop the preview first (this will blank
        // the screen).
        if (mCameraState != PREVIEW_STOPPED) {
            stopPreview();
        }
        if (mSurfaceHolder == null) {
            Elog.d(TAG, "startPreview() mSurfaceHolder == null");
        }
        setPreviewDisplay(mSurfaceHolder);
        setDisplayOrientation();
        if (!mSnapshotOnIdle) {
            // If the focus mode is continuous autofocus, call cancelAutoFocus
            // to
            // resume it because it may have been paused by autoFocus call.
            if (Parameters.FOCUS_MODE_CONTINUOUS_PICTURE.equals(mFocusManager.getFocusMode())) {
                mCameraDevice.cancelAutoFocus();
            }
            mFocusManager.setAeAwbLock(false); // Unlock AE and AWB.
        }
        setCameraParameters(UPDATE_PARAM_ALL); // set paramters

        // Inform the mainthread to go on the UI initialization.
        if (mCameraPreviewThread != null) {
            synchronized (mCameraPreviewThread) {
                mSettingRetrieved = true;
                mCameraPreviewThread.notify();
            }
        }

        try {
            Elog.v(TAG, "startPreview");
            mCameraDevice.startPreview();
        } catch (Throwable ex) {
            closeCamera();
            Elog.v(TAG, "startPreview exception." + ex.getMessage());
            throw new RuntimeException("startPreview failed", ex);
        }
        setCameraState(IDLE);
        mFocusManager.onPreviewStarted();

        // notify again to make sure main thread is wake-up.
        if (mCameraPreviewThread != null) {
            synchronized (mCameraPreviewThread) {
                mCameraPreviewThread.notify();
            }
        }
        if (!mTakePicDone) {
            mTakePicDone = true;
        }
    }

    private void stopPreview() {
        if (mCameraDevice != null && mCameraState != PREVIEW_STOPPED) {
            Elog.v(TAG, "stopPreview");
            // maybe stop capture(stop3DShot) is ongoing,then it is not allowed
            // to stopPreview.
            mCameraDevice.cancelAutoFocus(); // Reset the focus.
            mCameraDevice.stopPreview();
        }
        setCameraState(PREVIEW_STOPPED);
        mFocusManager.onPreviewStopped();
    }

    private static boolean isSupported(String value, List<String> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private void updateCameraParametersInitialize() {
        Elog.v(TAG, "updateCameraParametersInitialize()");
        // Reset preview frame rate to the maximum because it may be lowered by
        // video camera application.
        List<Integer> frameRates = mParameters.getSupportedPreviewFrameRates();
        if (frameRates != null) {
            Integer max = Collections.max(frameRates);
            mParameters.setPreviewFrameRate(max);
        }
        mParameters.setRecordingHint(false);
    }

    private void updateCameraParametersPreference(int updataSet) {
        Elog.v(TAG, "updateCameraParametersPreference() updataSet " + updataSet);
        String sceneMode = getString(R.string.pref_camera_scenemode_default); // is
        // auto
        if (isSupported(sceneMode, mParameters.getSupportedSceneModes())) {
            if (!mParameters.getSceneMode().equals(sceneMode)) {
                mParameters.setSceneMode(sceneMode);
                mCameraDevice.setParameters(mParameters);
                // Setting scene mode will change the settings of flash mode,
                // white balance, and focus mode. Here we read back the
                // parameters, so we can know those settings.
                mParameters = mCameraDevice.getParameters();
            }
        } else {
            sceneMode = mParameters.getSceneMode();
            if (sceneMode == null) {
                sceneMode = Parameters.SCENE_MODE_AUTO;
            }
        }

        if (mAeLockSupported && mAwbLockSupported) {
            Elog.d(TAG, "[EMCamera]mCaptureDoneNumber:" + mCaptureDoneNumber);
            if (mCaptureDoneNumber == 1 && isLockAeAwbAfMode(mAfMode)) {
                mFocusManager.setAeAwbLock(true);
                mEnableAeAwbLock = true;
            }      
            if (mEnableAeAwbLock && !mManualLockAe) {
                mParameters.setAutoExposureLock(mFocusManager.getAeAwbLock());
            }

            if (mEnableAeAwbLock) {
                mParameters.setAutoWhiteBalanceLock(mFocusManager.getAeAwbLock());
            }
            if (mCaptureDoneNumber == 1 && isLockAeAwbAfMode(mAfMode)) {
                mEnableAeAwbLock = false;
            }
        }

        int camOri = CameraHolder.instance().getCameraInfo()[mCameraId].orientation;
        Elog.i(TAG, " Sensor[" + mCameraId + "]'s orientation is " + camOri);
        // EM : set picture size
        setPictureSize(camOri);
        // Set the preview frame aspect ratio according to the picture size.
        Size size = mParameters.getPictureSize();
        Elog.v(TAG, "getPictureSize is " + size.width + "x" + size.height);
        /*
         * Tricky code here, should really careful here. picture size has been
         * swap in camOri == 0 || camOri == 180 case here size is restore again
         * to maximize preview.
         */
        double aspectWtoH = 0.0;
        if (size.height > size.width) {
            aspectWtoH = (double) size.height / size.width;
        } else {
            aspectWtoH = (double) size.width / size.height;
        }
        mPreviewPanel = findViewById(R.id.frame_layout);
        mPreviewFrameLayout = (PreviewFrameLayout) findViewById(R.id.frame);
        Elog.v(TAG, "setAspectRatio " + aspectWtoH);
        mPreviewFrameLayout.setAspectRatio(aspectWtoH);

        // Set a preview size that is closest to the viewfinder height and has
        // the right aspect ratio.
        List<Size> sizes = mParameters.getSupportedPreviewSizes();
        Size optimalSize = Util.getOptimalPreviewSize(this, sizes, aspectWtoH);
        Size original = mParameters.getPreviewSize();

        if (!original.equals(optimalSize)) {
            /*
             * Tricky code here, should really careful here. swap width and
             * height when camera orientation = 0 or 180
             */
            if (camOri == 0 || camOri == 180) {
                mParameters.setPreviewSize(optimalSize.height, optimalSize.width);
            } else {
                mParameters.setPreviewSize(optimalSize.width, optimalSize.height);
            }

            // Zoom related settings will be changed for different preview
            // sizes, so set and read the parameters to get lastest values
            mCameraDevice.setParameters(mParameters);
            mParameters = mCameraDevice.getParameters();
        }
        Elog.v(TAG, "Preview size is " + optimalSize.width + "x" + optimalSize.height);

        // Set JPEG quality.
        int jpegQuality = CameraProfile.getJpegEncodingQualityParameter(mCameraId, CameraProfile.QUALITY_HIGH);
        mParameters.setJpegQuality(jpegQuality);

        if (updataSet == UPDATE_PARAM_ALL) { // just start preview do this
            setEmCameraParam();
        }
    }
    
    private boolean isLockAeAwbAfMode(int afMode) {
        if (afMode == AF_MODE_BRACKET) {
            return true;
        }
        if (afMode == AF_MODE_THROUGH_FOCUS) {
            return true;
        }
        return false;
    }

    private void setPictureSize(int camOri) {
        Elog.v(TAG, "setPictureSize camOri " + camOri);
        List<Size> pictureSizes = mParameters.getSupportedPictureSizes();
        Size maxSize = pictureSizes.get(0);
        for (int i = 1; i < pictureSizes.size(); i++) {
            if ((pictureSizes.get(i).height > maxSize.height) || (pictureSizes.get(i).width > maxSize.width)) {
                    maxSize = pictureSizes.get(i);
            }
        }
        Elog.v(TAG, "Max picture size is " + maxSize.width + "x" + maxSize.height);
        if (mCaptureMode == 0) {
            if (mCaptureSize == 1 || mCaptureSize == 3) { // capture
                if (camOri == 0 || camOri == 180) {
                    mParameters.setPictureSize(maxSize.height, maxSize.width);
                } else {
                    mParameters.setPictureSize(maxSize.width, maxSize.height);
                }
            } else if (mCaptureSize == 0) { // preview
                Size previewSize = pictureSizes.get(0);
                int x = maxSize.width / 2;
                int y = maxSize.height / 2;
                int xDiff = Math.abs(x - pictureSizes.get(0).width);
                int yDiff = Math.abs(y - pictureSizes.get(0).height);
                for (int i = 1; i < pictureSizes.size(); i++) {
                    if ((xDiff > Math.abs(x - pictureSizes.get(i).width))
                            || (yDiff > Math.abs(y - pictureSizes.get(i).height))) {
                        previewSize = pictureSizes.get(i);
                        xDiff = Math.abs(x - pictureSizes.get(i).width);
                        yDiff = Math.abs(y - pictureSizes.get(i).height);
                    }
                }
                Elog.v(TAG, "preview size is " + previewSize.width + "x" + previewSize.height);
                if (camOri == 0 || camOri == 180) {
                    mParameters.setPictureSize(previewSize.height, previewSize.width);
                } else {
                    mParameters.setPictureSize(previewSize.width, previewSize.height);
                }
            } else { // video
                List<Size> videoSizes = mParameters.getSupportedVideoSizes();
                for (int i = 0; i < videoSizes.size(); i++) { 
                    // Modify video size to 16-aligned because Picture Size (Both width & height must be 16-aligned)
                    videoSizes.get(i).width = (((videoSizes.get(i).width + 15) >> 4) << 4);
                    videoSizes.get(i).height = (((videoSizes.get(i).height + 15) >> 4) << 4);
                }
                for (int i = 0; i < videoSizes.size(); i++) {
                    if (!isSupportSize(pictureSizes, videoSizes.get(i))) {
                        videoSizes.remove(i);
                    }
                }
                Size maxVideoSize = videoSizes.get(0);
                for (int i = 1; i < videoSizes.size(); i++) {
                    if ((videoSizes.get(i).height > maxVideoSize.height)
                            || (videoSizes.get(i).width > maxVideoSize.width)) {
                        maxVideoSize = videoSizes.get(i);
                    }
                }
                Elog.v(TAG, "video size is " + maxVideoSize.width + "x" + maxVideoSize.height);
                if (camOri == 0 || camOri == 180) {
                    mParameters.setPictureSize(maxVideoSize.height, maxVideoSize.width);
                } else {
                    mParameters.setPictureSize(maxVideoSize.width, maxVideoSize.height);
                }
            }
        }
    }

    boolean isSupportSize(List<Size> supported, Size s) {
        for (Size size : supported) {
            if (size.width == s.width && size.height == s.height) {
                return true;
            }
        }
        return false;
    }

    private boolean isSupportCameraPictureSize(Size size) {
        // When launching the camera app first time, we will set the picture
        // size to the first one in the list defined in "arrays.xml" and is also
        // supported by the driver.
        for (String candidate : getResources().getStringArray(R.array.pref_camera_picturesize_entryvalues)) {
            int index = candidate.indexOf('x');
            if (index == -1) {
                continue;
            }
            int width = Integer.parseInt(candidate.substring(0, index));
            int height = Integer.parseInt(candidate.substring(index + 1));
            if (size.width == width && size.height == height) {
                return true;
            }
        }
        Elog.e(TAG, "No supported picture size found " + size.width + "x" + size.height);
        return false;
    }

    private void setEmCameraParam() {
        Elog.v(TAG, "setEmCameraParam() set EM parameters.");
        // Set CaptureMode
        // mCameraParam.set("rawsave-mode", value);
        // 1 Preview Mode
        // 2 Capture Mode
        // 3 JPEG Only
        // 4 Video Mode
        // SetOutput type
        // mCameraParam.set("isp-mode", value)
        // 0: "Processed Raw¡°
        // 1: "Pure Raw¡°
        long dateTaken = System.currentTimeMillis();
        if (mCaptureMode == 0) {
            mParameters.setCameraMode(Parameters.CAMERA_MODE_NORMAL);
            if (mCaptureType == 2) {
                mParameters.set(KEY_RAW_SAVE_MODE, RAW_SAVE_JPEG);
                mRawCaptureFileName = "JpegOnly";
            } else {
                if (mCaptureSize == 2) {
                    mParameters.set(KEY_RAW_SAVE_MODE, RAW_SAVE_VIDEO);
                    mRawCaptureFileName = "Video";
                } else {
                    if (mCaptureSize == 0) {
                        mRawCaptureFileName = "Preview";
                    } else {
                        mRawCaptureFileName = "Capture";
                    }
                    if (mCaptureSize == 3) {
                        mParameters.setEngZSDEnable(1);
                        mParameters.set(KEY_RAW_SAVE_MODE, 2); // Force save mode is capture size
                    } else {
                        mParameters.set(KEY_RAW_SAVE_MODE, mCaptureSize + 1);
                    }
                    
                }
                mParameters.set(KEY_ISP_MODE, mCaptureType);
            }
        } else if (mCaptureMode == 2) {
            mParameters.setCameraMode(Parameters.CAMERA_MODE_MTK_PRV);
            mRawCaptureFileName = "VideoClip";
            mParameters.set(KEY_RAW_SAVE_MODE, 1);
            if (mVideoResolution == 0) {
                mParameters.setPreviewRawDumpResolution(Parameters.PREVIEW_DUMP_RESOLUTION_NORMAL);
            } else {
                mParameters.setPreviewRawDumpResolution(Parameters.PREVIEW_DUMP_RESOLUTION_CROP);
            }
        }
        mRawCaptureFileName = mCameraImageName + mRawCaptureFileName + createName(dateTaken);
        // set focus mode
        if (isSupportAf(mParameters)) {
            String focusMode = "";
            if (mAfMode == AF_MODE_AUTO) {
                focusMode = Parameters.FOCUS_MODE_AUTO;
            } else if (mAfMode == AF_MODE_BRACKET) {
                if (mAfCaptureTimes == 0) {
                    focusMode = Parameters.FOCUS_MODE_AUTO;
                } else {
                    focusMode = Parameters.FOCUS_MODE_MANUAL;
                }
            } else if (mAfMode == AF_MODE_FULL_SCAN) {
                focusMode = Parameters.FOCUS_MODE_FULLSCAN;
                Elog.d(TAG, "setEngFocusFullScanFrameInterval(" + mAfFullScanFrameInterval + ")");
                mParameters.setEngFocusFullScanFrameInterval(mAfFullScanFrameInterval);
            } else if (mAfMode == AF_MODE_THROUGH_FOCUS) {
                focusMode = Parameters.FOCUS_MODE_MANUAL;
            } else if (mAfMode == AF_MODE_CONTINUOUS) {
                focusMode = Parameters.FOCUS_MODE_CONTINUOUS_PICTURE;
            } else {
                Elog.w(TAG, "Unknown mAfMode:" + mAfMode);
            }
            mParameters.setFocusMode(focusMode);
            mFocusManager.overrideFocusMode(focusMode);
            if (mFocusAreaSupported) {
                mParameters.setFocusAreas(mFocusManager.getRealFocusAreas());
            }
        } else {
            mFocusManager.overrideFocusMode(Parameters.FOCUS_MODE_INFINITY); 
        }
        // set flicker
        mParameters.setAntibanding(mFlickerString);
        if (mAfMode == AF_MODE_AUTO) { // AF auto has different iso value
            if (mCaptureMode == 2) {
                mParameters.setISOSpeed(mIsoValues.get(0));
                if (mIsoValues.get(0).equals("0")) {
                    // parameter "0" is iso auto
                    mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
                } else {
                    mRawCaptureFileName = mRawCaptureFileName + "ISO" + mIsoValues.get(mAfCaptureTimes);
                }
                mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            } else {
                if (mAfCaptureTimes < mIsoValues.size()) {
                    mIsoValIndex = mAfCaptureTimes;
                    mParameters.setISOSpeed(mIsoValues.get(mAfCaptureTimes));
                    Elog.v(TAG, "EVENT_CAPTURE_ACTION iso speed " + mIsoValues.get(mAfCaptureTimes));
                    if (mIsoValues.get(mAfCaptureTimes).equals("0")) {
                        // parameter "0" is iso auto
                        mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
                    } else {
                        mRawCaptureFileName = mRawCaptureFileName + "ISO" + mIsoValues.get(mAfCaptureTimes);
                    }
                    if (mCaptureType != 2) {
                        mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
                    }
                    Elog.v(TAG, "mTripleCount " + mTripleCount + " mAfCaptureTimes " + mAfCaptureTimes);
                    if (mTripleCount == mCaptureNumber) {
                        mTripleCount = 0;
                        mAfCaptureTimes++;
                    } else {
                        mTripleCount++;
                    }
                    Elog.v(TAG, "mTripleCount " + mTripleCount + " mAfCaptureTimes " + mAfCaptureTimes);
                } else {
                    mAfCaptureTimes++;
                }
            }
        } else { // AF full scan, bracket, through focus just has one iso value
            Elog.v(TAG, "EVENT_CAPTURE_ACTION mAfMode != 0 ");
            mParameters.setISOSpeed(mIsoValue);
            if (mIsoValue.equals("0")) {
                mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
            } else {
                mRawCaptureFileName = mRawCaptureFileName + "ISO" + mIsoValue;
            }
            if (mCaptureType != 2) {
                mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            }
        }
        // set strobe mode
        if (isStrobeSupported(mParameters)) {
            mParameters.set(KEY_FLASH_MODE, mStrobeMode);
        }
        Elog.d(TAG, "setEmCameraParam mRawCaptureFileName " + mRawCaptureFileName);
        Elog.d(TAG, "setEmCameraParam mShadingTable:" + mShadingTable + " mOutputSdblk:" + mOutputSdblk);
        mParameters.setEngShadingTable(mShadingTable);
        mParameters.setEngSaveShadingTable(mOutputSdblk);
    }

    private String createName(long dateTaken) {
        return DateFormat.format("yyyyMMdd-kkmmss", dateTaken).toString();
    }

    // We separate the parameters into several subsets, so we can update only
    // the subsets actually need updating. The PREFERENCE set needs extra
    // locking because the preference can be changed from GLThread as well.
    private void setCameraParameters(int updateSet) {
        Elog.v(TAG, "setCameraParameters() updateSet " + updateSet);
        mParameters = mCameraDevice.getParameters();
        if ((updateSet & UPDATE_PARAM_INITIALIZE) != 0) {
            updateCameraParametersInitialize();
        }
        if ((updateSet & UPDATE_PARAM_PREFERENCE) != 0) {
            // if (mFirstTimeInitialized) {
            checkStorage();
            // }
            updateCameraParametersPreference(updateSet);
            sIsAutoFocusCallback = false;
        }
        mCameraDevice.setParameters(mParameters);
    }

    private boolean isCameraIdle() {
        return (mCameraState == IDLE) || (mFocusManager.isFocusCompleted());
    }

    @Override
    public void onUserInteraction() {
        super.onUserInteraction();
        keepScreenOnAwhile();
    }

    private void resetScreenOn() {
        mHandler.removeMessages(CLEAR_SCREEN_DELAY);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void keepScreenOnAwhile() {
        mHandler.removeMessages(CLEAR_SCREEN_DELAY);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // mHandler.sendEmptyMessageDelayed(CLEAR_SCREEN_DELAY, SCREEN_DELAY);
    }

    private void initializeCapabilities() {
        Elog.d(TAG, "initializeCapabilities()");
        mInitialParams = mCameraDevice.getParameters();
        mFocusManager.initializeParameters(mInitialParams);
        mFocusAreaSupported = (mInitialParams.getMaxNumFocusAreas() > 0 && isSupported(Parameters.FOCUS_MODE_AUTO,
                mInitialParams.getSupportedFocusModes()));
        mMeteringAreaSupported = (mInitialParams.getMaxNumMeteringAreas() > 0);
        mAeLockSupported = mInitialParams.isAutoExposureLockSupported();
        mAwbLockSupported = mInitialParams.isAutoWhiteBalanceLockSupported();
    }

    public void resumePreview() {
        if ((mCameraState != IDLE)) {
            startPreview();
        }
        checkStorage();
    }

    public int getCameraId() {
        return mCameraId;
    }

    public long getPictureRemaining() {
        return mPicturesRemaining;
    }

    protected Parameters getCameraParameters() {
        return mParameters;
    }

    public PreviewFrameLayout getPreviewFrameLayout() {
        return mPreviewFrameLayout;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public int getOrientation() {
        return mOrientation;
    }

    public long getRemainPictures() {
        return mPicturesRemaining;
    }

    public boolean checkCameraState() {
        Elog.i(TAG, "Check camera state in ModeActor");
        if (mPausing) {
            return false;
        }

        // If the user wants to do a snapshot while the previous one is still
        // in progress, remember the fact and do it after we finish the previous
        // one and re-start the preview. Snapshot in progress also includes the
        // state that autofocus is focusing and a picture will be taken when
        // focus callback arrives.
        if (mFocusManager.isFocusingSnapOnFinish() || mCameraState == SNAPSHOT_IN_PROGRESS
                || mCameraState == SAVING_PICTURES) {
            return false;
        }
        mSnapshotOnIdle = false;
        return true;
    }
    
    static boolean isSupportAf(Parameters parameters) {
        boolean isSupport = true;
        List<String> list = parameters.getSupportedFocusModes();

        if (list.size() == 1 && 
                (list.get(0).equals(Parameters.FOCUS_MODE_FIXED) || 
                        list.get(0).equals(Parameters.FOCUS_MODE_INFINITY))) {
            isSupport = false;
        }
        return isSupport;
    }
    
    static boolean isStrobeSupported(Parameters parameters) {
        boolean isSupport = true;
        List<String> list = parameters.getSupportedFlashModes();
        if (null == list) {
            isSupport = false;
        } else {
            if (list.size() == 1 && list.get(0).equals(Parameters.FLASH_MODE_OFF)) {
                isSupport = false;
            }
        }
        return isSupport;
    }

    static boolean isRawSensor(int facing, Parameters parameters) {
        boolean isRaw = false;
        
        int sensorType = parameters.getSensorType();
        if (facing == CameraInfo.CAMERA_FACING_BACK) {
            if ((sensorType & 1) == 0) {
                isRaw = true;
            }
        } else if (facing == CameraInfo.CAMERA_FACING_FRONT) {
            if ((((sensorType >> 1) & 1) == 0)) {
                isRaw = true;
            }
        }
        
        return isRaw;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        
        if (mParameters != null) {
            if (mParameters.isAutoExposureLockSupported()) {
                menu.add(Menu.NONE, MENU_ID_AE, Menu.NONE, "Lock " + getString(R.string.camera_ae));
            }
            
            if (!(mParameters.getMinExposureCompensation() == 0 && 
                    mParameters.getMinExposureCompensation() == 0)) {
                menu.add(Menu.NONE, MENU_ID_EV, Menu.NONE, getString(R.string.camera_ev));
            }
            
            if (isStrobeSupported(mParameters)) {
                menu.add(Menu.NONE, MENU_ID_STROBE_DUTY, Menu.NONE, getString(R.string.camera_strobe_duty));
            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        CharSequence title;
        switch (id) {
        case MENU_ID_AE:
            title = item.getTitle();
            if (title.equals("Lock " + getString(R.string.camera_ae))) {
                mParameters.setAutoExposureLock(true);
                mManualLockAe = true;
            } else {
                mParameters.setAutoExposureLock(false);
                mManualLockAe = false;
            }
            mCameraDevice.setParameters(mParameters);
            break;
        case MENU_ID_EV:
            title = item.getTitle() + " setting";
            showSingleInputDlg(title,  new InputDialogOnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which, String inputStr) {
                    int ev = 0;
                    boolean validInput = true;
                    int minev = mParameters.getMinExposureCompensation();
                    int maxev = mParameters.getMaxExposureCompensation();
                    
                    try {
                        ev = Integer.valueOf(inputStr);
                    } catch (NumberFormatException e) {
                        validInput = false;
                    }
                    if (!validInput || ev < minev || ev > maxev) {
                        Toast.makeText(Camera.this, "invalid ev, must range from " + minev + " to " + maxev, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    mParameters.setExposureCompensation(ev);
                    mCameraDevice.setParameters(mParameters);
                    dialog.dismiss();
                }
            }, String.valueOf(mParameters.getExposureCompensation()));
            break;
        case MENU_ID_STROBE_DUTY:
            title = item.getTitle() + " setting";
            showSingleInputDlg(title,  new InputDialogOnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which, String inputStr) {
                    int minDuty = mParameters.getEngFlashDutyMin();
                    int maxDuty = mParameters.getEngFlashDutyMax();
                    boolean validInput = true;
                    int duty = -1;
                    try {
                        duty = Integer.valueOf(inputStr);
                    } catch (NumberFormatException e) {
                        validInput = false;
                    }
                    if (!validInput || duty < minDuty || duty > maxDuty) {
                        Toast.makeText(Camera.this, 
                                "invalid strobe duty, must range from " + minDuty + " to " + maxDuty, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    mStrobeDuty = inputStr;
                    mParameters.setEngFlashDuty(duty);
                    mCameraDevice.setParameters(mParameters);
                    dialog.dismiss();
                }
            }, mStrobeDuty);
            break;
        default:
            Elog.w(TAG, "onOptionsItemSelected unknown menu item id:" + id);
            break;
        }
        return true;
    }
    
    private interface InputDialogOnClickListener {
        public void onClick(DialogInterface dialog, int which, String inputStr);
    }
    
    private void showSingleInputDlg(CharSequence title, final InputDialogOnClickListener listener, String defVal) {
        View inputView = getLayoutInflater().inflate(R.layout.em_single_input_layout, null);
        Button okBtn = (Button)inputView.findViewById(R.id.em_single_input_ok_btn);
        final EditText inputEdit = (EditText)inputView.findViewById(R.id.em_single_input_edit);
        if (defVal != null) {
            inputEdit.setText(defVal);
        }
        final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                true).setTitle(title).setView(inputView).create();
        okBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                listener.onClick(dialog, DialogInterface.BUTTON_POSITIVE, inputEdit.getText().toString());
            }
        });
        dialog.show();
    }
    
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem aeItem = menu.findItem(MENU_ID_AE);
        if (aeItem != null) {
            if (mParameters.getAutoExposureLock()) {
                aeItem.setTitle("Unlock " + getString(R.string.camera_ae));
            } else {
                aeItem.setTitle("Lock " + getString(R.string.camera_ae));
            }
        }
        return super.onPrepareOptionsMenu(menu);
    }
}
