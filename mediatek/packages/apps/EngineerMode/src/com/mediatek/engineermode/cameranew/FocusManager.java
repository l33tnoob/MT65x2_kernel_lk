/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera.Area;
import android.hardware.Camera.Parameters;
import android.os.Handler;
import android.os.Message;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

// A class that handles everything about focus in still picture mode.
// This also handles the metering area because it is the same as focus area.
public class FocusManager {
    private static final String TAG = "test/FocusManager";

    private static final int RESET_TOUCH_FOCUS = 0;
    private static final int RESET_FOCUS_FRAME = 1;
    private static final int FOCUS_BEEP_VOLUME = 100;
    private static final int RESET_TOUCH_FOCUS_DELAY = 3000;

    private int mState = STATE_IDLE;
    private static final int STATE_IDLE = 0; // Focus is not active.
    private static final int STATE_FOCUSING = 1; // Focus is in progress.
    // Focus is in progress and the camera should take a picture after focus
    // finishes.
    private static final int STATE_FOCUSING_SNAP_ON_FINISH = 2;
    private static final int STATE_SUCCESS = 3; // Focus finishes and succeeds.
    private static final int STATE_FAIL = 4; // Focus finishes and fails.

    private boolean mInitialized;
    private boolean mFocusAreaSupported;
    private boolean mLockAeAwbNeeded;
    private boolean mAeAwbLock;
    private Matrix mMatrix;
    private View mFocusIndicatorRotateLayout;
    private FocusIndicatorView mFocusIndicator;
    private View mPreviewFrame;
    private List<Area> mFocusArea; // focus area in driver format
    private List<Area> mRealFocusArea; // focus area in driver format
    private List<Area> mMeteringArea; // metering area in driver format
    private String mFocusMode;
    private String[] mDefaultFocusModes;
    private String mOverrideFocusMode;
    private Parameters mParameters;
    private Handler mHandler;
    Listener mListener;
    private boolean mIsTouchFocus = false;
    
    private boolean mEnableFaceBeauty;
    private static final int CONTINUOUS_FOCUSING = 0;
    private static final int CONTINUOUS_FOCUS_SUCCESS = 1;
    private static final int CONTINUOUS_FOCUS_FAIL = 2;

    private static final int FOCUS_FRAME_DELAY = 1000;
    private static final int RESET_FOCUS_FRAME_DELAY = 800;

    public interface Listener {
        void autoFocus();

        void cancelAutoFocus();

        void onAutoFocusDone();

        void setFocusParameters();
    }

    private class MainHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case RESET_TOUCH_FOCUS:
                    cancelAutoFocus();
                    break;
                case RESET_FOCUS_FRAME:
                    mFocusIndicator.clear();
                    break;
                default:
                    break;
            }
        }
    }

    public FocusManager(String[] defaultFocusModes) {
        mDefaultFocusModes = defaultFocusModes;
        mHandler = new MainHandler();
        mMatrix = new Matrix();
    }

    // This has to be initialized before initialize().
    public void initializeParameters(Parameters parameters) {
        mParameters = parameters;
        mFocusAreaSupported = (mParameters.getMaxNumFocusAreas() > 0 && isSupported(Parameters.FOCUS_MODE_AUTO,
                mParameters.getSupportedFocusModes()));
        mLockAeAwbNeeded = (mParameters.isAutoExposureLockSupported() || mParameters.isAutoWhiteBalanceLockSupported());
    }

    public void initialize(View focusIndicatorRotate, View previewFrame, Listener listener, boolean mirror,
            int displayOrientation) {
        mFocusIndicatorRotateLayout = focusIndicatorRotate;
        mFocusIndicator = (FocusIndicatorView) focusIndicatorRotate.findViewById(R.id.focus_indicator);
        mPreviewFrame = previewFrame;
        mListener = listener;

        Matrix matrix = new Matrix();
        Util.prepareMatrix(matrix, mirror, displayOrientation, previewFrame.getWidth(), previewFrame.getHeight());
        // In face detection, the matrix converts the driver coordinates to UI
        // coordinates. In tap focus, the inverted matrix converts the UI
        // coordinates to driver coordinates.
        matrix.invert(mMatrix);

        if (mParameters != null) {
            mInitialized = true;
        } else {
            Elog.e(TAG, "mParameters is not initialized.");
        }
        initRealFocusArea();
    }

    private void initRealFocusArea() {
        if (mRealFocusArea == null) {
            mRealFocusArea = new ArrayList<Area>();
            mRealFocusArea.add(new Area(new Rect(), 1));
        }
        int focusWidth = mFocusIndicatorRotateLayout.getWidth();
        int focusHeight = mFocusIndicatorRotateLayout.getHeight();
        if (focusWidth == 0 || focusHeight == 0) {
            Elog.i(TAG, "UI Component not initialized, cancel this touch");
            return;
        }
        int previewWidth = mPreviewFrame.getWidth();
        int previewHeight = mPreviewFrame.getHeight();
        int x = previewWidth / 2;
        int y = previewHeight / 2;
        Elog.i(TAG, "real area.x = " + x + " real area.y = " + y);
        calculateTapArea(focusWidth, focusHeight, 1f, x, y, previewWidth, previewHeight, mRealFocusArea.get(0).rect);
    }

    public void onShutterDown() {
        if (!mInitialized) {
            return;
        }
        // Lock AE and AWB so users can half-press shutter and recompose.
        if (mLockAeAwbNeeded && !mAeAwbLock) {
            mAeAwbLock = true;
            mListener.setFocusParameters();
        }
        
        if (needAutoFocusCall() && !isTouchFocusMode()) {
            // Do not focus if touch focus has been triggered.
            if (mState != STATE_SUCCESS && mState != STATE_FAIL) {
                autoFocus();
            }
        } else {
            mListener.onAutoFocusDone();
        }
    }

    public void onShutterUp() {
        if (!mInitialized) {
            return;
        }
        if (needAutoFocusCall()) {
            // User releases half-pressed focus key.
            if (mState == STATE_FOCUSING || mState == STATE_SUCCESS || mState == STATE_FAIL) {
                cancelAutoFocus();
            }
        }

        // Unlock AE and AWB after cancelAutoFocus. Camera API does not
        // guarantee setParameters can be called during autofocus.
        if (mLockAeAwbNeeded && mAeAwbLock && (mState != STATE_FOCUSING_SNAP_ON_FINISH)) {
            mAeAwbLock = false;
            mListener.setFocusParameters();
        }
        mIsTouchFocus = false;
    }

    public void onAutoFocus(boolean focused) {
        Elog.i(TAG, "onAutoFocus, mState = " + String.valueOf(mState));
        if (mState == STATE_FOCUSING_SNAP_ON_FINISH) {
            // Take the picture no matter focus succeeds or fails. No need
            // to play the AF sound if we're about to play the shutter
            // sound.
            if (focused) {
                mState = STATE_SUCCESS;
            } else {
                mState = STATE_FAIL;
            }
            updateFocusUI();
            autoFocusDoneHanlder(focused);
        } else if (mState == STATE_FOCUSING) {
            // This happens when (1) user is half-pressing the focus key or
            // (2) touch focus is triggered. Play the focus tone. Do not
            // take the picture now.
            if (focused) {
                mState = STATE_SUCCESS;
            } else {
                mState = STATE_FAIL;
            }
            updateFocusUI();
            // If this is triggered by touch focus, cancel focus after a
            // while.
            if (mFocusArea != null) {
                mHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS, FOCUS_FRAME_DELAY);
            }
        } else {
            autoFocusDoneHanlder(focused);
        }
    }
    
    private void autoFocusDoneHanlder(boolean focused) {
        if (mIsTouchFocus) {
            mHandler.sendEmptyMessageDelayed(RESET_FOCUS_FRAME, RESET_FOCUS_FRAME_DELAY);
        }
        mListener.onAutoFocusDone();
    }

    public void doSnap() {
        if (!mInitialized) {
            return;
        }
        if (mState == STATE_FOCUSING) {
            // Half pressing the shutter (i.e. the focus button event) will
            // already have requested AF for us, so just request capture on
            // focus here.
            mState = STATE_FOCUSING_SNAP_ON_FINISH;
        }
    }

    public boolean onTouch(MotionEvent e) {
        if (!mInitialized || mState == STATE_FOCUSING_SNAP_ON_FINISH) {
            return false;
        }
        Elog.d(TAG, "onTouch ");

        // Initialize variables.
        int x = Math.round(e.getX());
        int y = Math.round(e.getY());
        int focusWidth = mFocusIndicatorRotateLayout.getWidth();
        int focusHeight = mFocusIndicatorRotateLayout.getHeight();
        if (focusWidth == 0 || focusHeight == 0) {
            Elog.i(TAG, "UI Component not initialized, cancel this touch");
            return false;
        }
        Elog.i(TAG, "TouchFocus: touch.x = " + x + " touch.y = " + y);
        int previewWidth = mPreviewFrame.getWidth();
        int previewHeight = mPreviewFrame.getHeight();
        if (mFocusArea == null) {
            mFocusArea = new ArrayList<Area>();
            mFocusArea.add(new Area(new Rect(), 1));
            mMeteringArea = new ArrayList<Area>();
            mMeteringArea.add(new Area(new Rect(), 1));
        }

        // Convert the coordinates to driver format.
        // AE area is bigger because exposure is sensitive and
        // easy to over- or underexposure if area is too small.
        calculateTapArea(focusWidth, focusHeight, 1f, x, y, previewWidth, previewHeight, mFocusArea.get(0).rect);
        calculateTapArea(focusWidth, focusHeight, 1.5f, x, y, previewWidth, previewHeight, mMeteringArea.get(0).rect);

        // Use margin to set the focus indicator to the touched area.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mFocusIndicatorRotateLayout.getLayoutParams();
        int left = Util.clamp(x - focusWidth / 2, 0, previewWidth - focusWidth);
        int top = Util.clamp(y - focusHeight / 2, 0, previewHeight - focusHeight);
        p.setMargins(left, top, 0, 0);
        // Disable "center" rule because we no longer want to put it in the
        // center.
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = 0;
        mFocusIndicatorRotateLayout.requestLayout();

        // Set the focus area and metering area.
        mListener.setFocusParameters();
        if (mFocusAreaSupported && (e.getAction() == MotionEvent.ACTION_UP)) {
            autoFocus();
        } else { // Just show the indicator in all other cases.
            updateFocusUI();
            // Reset the metering area in 3 seconds.
            mHandler.removeMessages(RESET_TOUCH_FOCUS);
            mHandler.sendEmptyMessageDelayed(RESET_TOUCH_FOCUS, RESET_TOUCH_FOCUS_DELAY);
        }

        return true;
    }

    public void onPreviewStarted() {
        mState = STATE_IDLE;
    }

    public void onPreviewStopped() {
        mState = STATE_IDLE;
        resetTouchFocus();
        // If auto focus was in progress, it would have been canceled.
        updateFocusUI();
    }

    public void onCameraReleased() {
        onPreviewStopped();
    }

    private void autoFocus() {
        Elog.v(TAG, "Start autofocus.");
        mListener.autoFocus();
        mState = STATE_FOCUSING;
        updateFocusUI();
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    private void cancelAutoFocus() {
        Elog.v(TAG, "Cancel autofocus.");

        // Reset the tap area before calling mListener.cancelAutofocus.
        // Otherwise, focus mode stays at auto and the tap area passed to the
        // driver is not reset.
        resetTouchFocus();
        mListener.cancelAutoFocus();
        // if (mFaceView != null)
        // mFaceView.resume();
        mState = STATE_IDLE;
        updateFocusUI();
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    public void capture() {
        Elog.d(TAG, "capture()");
        // if (mListener.capture()) {
        mState = STATE_IDLE;
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
        // }
    }

    // This can only be called after mParameters is initialized.
    public String getFocusMode() {
        // if (mOverrideFocusMode != null)
        return mOverrideFocusMode;
    }

    public List<Area> getFocusAreas() {
        return mFocusArea;
    }

    public List<Area> getRealFocusAreas() {
        return mRealFocusArea;
    }

    public List<Area> getMeteringAreas() {
        return mMeteringArea;
    }

    public void updateFocusUI() {
        Elog.d(TAG, "updateFocusUI()");
        if (!mInitialized) {
            return;
        }
        // Set the length of focus indicator according to preview frame size.
        int len = Math.min(mPreviewFrame.getWidth(), mPreviewFrame.getHeight()) / 4;
        ViewGroup.LayoutParams layout = mFocusIndicator.getLayoutParams();
        layout.width = len;
        layout.height = len;

        // Show only focus indicator or face indicator.
        FocusIndicator focusIndicator = mFocusIndicator;

        if (mState == STATE_IDLE) {
            if (mFocusArea == null) {
                focusIndicator.clear();
            } else {
                // Users touch on the preview and the indicator represents the
                // metering area. Either focus area is not supported or
                // autoFocus call is not required.
                focusIndicator.showStart();
            }
        } else if (mState == STATE_FOCUSING || mState == STATE_FOCUSING_SNAP_ON_FINISH) {
            focusIndicator.showStart();
        } else {
            if (mState == STATE_SUCCESS) {
                focusIndicator.showSuccess();
            } else if (mState == STATE_FAIL) {
                focusIndicator.showFail();
            }
        }
    }

    public void resetTouchFocus() {
        if (!mInitialized) {
            return;
        }
        // Put focus indicator to the center.
        RelativeLayout.LayoutParams p = (RelativeLayout.LayoutParams) mFocusIndicatorRotateLayout.getLayoutParams();
        int[] rules = p.getRules();
        rules[RelativeLayout.CENTER_IN_PARENT] = RelativeLayout.TRUE;
        p.setMargins(0, 0, 0, 0);

        mFocusArea = null;
        mMeteringArea = null;
        mIsTouchFocus = false;
    }

    public void calculateTapArea(int focusWidth, int focusHeight, float areaMultiple, int x, int y, int previewWidth,
            int previewHeight, Rect rect) {
        int areaWidth = (int) (focusWidth * areaMultiple);
        int areaHeight = (int) (focusHeight * areaMultiple);
        int left = Util.clamp(x - areaWidth / 2, 0, previewWidth - areaWidth);
        int top = Util.clamp(y - areaHeight / 2, 0, previewHeight - areaHeight);

        RectF rectF = new RectF(left, top, left + areaWidth, top + areaHeight);
        mMatrix.mapRect(rectF);
        Util.rectFToRect(rectF, rect);
    }

    public boolean isFocusCompleted() {
        return mState == STATE_SUCCESS || mState == STATE_FAIL;
    }

    public boolean isFocusingSnapOnFinish() {
        return mState == STATE_FOCUSING_SNAP_ON_FINISH;
    }

    public void removeMessages() {
        mHandler.removeMessages(RESET_TOUCH_FOCUS);
    }

    public void overrideFocusMode(String focusMode) {
        mOverrideFocusMode = focusMode;
    }

    public void setAeAwbLock(boolean lock) {
        mAeAwbLock = lock;
    }

    public boolean getAeAwbLock() {
        return mAeAwbLock;
    }

    private static boolean isSupported(String value, List<String> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private boolean needAutoFocusCall() {
        String focusMode = getFocusMode();
        if (focusMode == null) {
            return false;
        }
        return !(focusMode.equals(Parameters.FOCUS_MODE_INFINITY) || focusMode.equals(Parameters.FOCUS_MODE_FIXED)
                || focusMode.equals(Parameters.FOCUS_MODE_EDOF) || focusMode.equals(Parameters.FOCUS_MODE_MANUAL));
    }
    
    public boolean onSingleTapUpPreview(View view, MotionEvent me) {
        if (getFocusMode() != Parameters.FOCUS_MODE_AUTO) {
            return false;
        }
        mHandler.removeMessages(RESET_FOCUS_FRAME);
        // Let users be able to cancel previous touch focus.
        if ((mFocusArea != null) && (mState == STATE_FOCUSING || mState == STATE_SUCCESS || mState == STATE_FAIL)) {
            cancelAutoFocus();
        }
        mIsTouchFocus = true;
        return onTouch(me);
    }
    
    public boolean isTouchFocusMode() {
        return mIsTouchFocus;
    }
    
}
