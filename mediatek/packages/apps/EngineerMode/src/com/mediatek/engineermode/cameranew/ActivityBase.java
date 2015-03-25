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

import android.app.Activity;
import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.os.Bundle;

import com.mediatek.engineermode.Elog;

/**
 * Superclass of Camera and VideoCamera activities.
 */
public abstract class ActivityBase extends Activity {
    protected Camera mCameraDevice;
    private static final String TAG = "ActivityBase";
    private int mResultCodeForTesting;
    private boolean mOnResumePending;
    private Intent mResultDataForTesting;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        Elog.v(TAG, "onWindowFocusChanged.hasFocus=" + hasFocus + ".mOnResumePending=" + mOnResumePending);
        if (hasFocus && mOnResumePending) {
            doOnResume();
            mOnResumePending = false;
        }
    }

    @Override
    public boolean onSearchRequested() {
        return false;
    }

    public int getResultCode() {
        return mResultCodeForTesting;
    }

    public Intent getResultData() {
        return mResultDataForTesting;
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Don't grab the camera if in use by lockscreen. For example, face
        // unlock may be using the camera. Camera may be already opened in
        // onCreate. doOnResume should continue if mCameraDevice != null.
        // Suppose camera app is in the foreground. If users turn off and turn
        // on the screen very fast, camera app can still have the focus when the
        // lock screen shows up. The keyguard takes input focus, so the caemra
        // app will lose focus when it is displayed.
        Elog.v(TAG, "onResume. hasWindowFocus()=" + hasWindowFocus());
        if (mCameraDevice == null && isKeyguardLocked()) {
            Elog.v(TAG, "onResume. mOnResumePending=true");
            mOnResumePending = true;
        } else {
            Elog.v(TAG, "onResume. mOnResumePending=false");
            doOnResume();
            mOnResumePending = false;
        }
    }

    @Override
    protected void onPause() {
        Elog.v(TAG, "onPause");
        super.onPause();
        mOnResumePending = false;
    }

    // Put the code of onResume in this method.
    protected abstract void doOnResume();

    protected void setResultEx(int resultCode) {
        mResultCodeForTesting = resultCode;
        setResult(resultCode);
    }

    protected void setResultEx(int resultCode, Intent data) {
        mResultCodeForTesting = resultCode;
        mResultDataForTesting = data;
        setResult(resultCode, data);
    }

    @Override
    protected void onDestroy() {
        // PopupManager.removeInstance(this);
        super.onDestroy();
    }

    private boolean isKeyguardLocked() {
        KeyguardManager kgm = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        if (kgm != null) {
            Elog.v(TAG,
                    "kgm.isKeyguardLocked()=" + kgm.isKeyguardLocked() + ". kgm.isKeyguardSecure()="
                            + kgm.isKeyguardSecure());
        }
        // isKeyguardSecure excludes the slide lock case.
        return (kgm != null) && kgm.isKeyguardLocked() && kgm.isKeyguardSecure();
    }
}
