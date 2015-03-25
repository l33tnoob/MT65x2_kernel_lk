/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.world3d;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.widget.FrameLayout;

import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.UiHandler;

import com.mediatek.ngin3d.animation.AnimationLoader;

public class World3D extends Activity {
    private Stage mStage;
    private RotationManager mSceneManager;
    private GestureDetector mGestureDetector;

    private void setupBackground() {
        Image backgroundImage = Image.createFromResource(getResources(), R.drawable.bg_stereo);
        backgroundImage.setPosition(new Point(0.5f, 0.5f, 600f, true));
        backgroundImage.setScale(new Scale(2.5f, 2.5f));
        mStage.add(backgroundImage);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.world3d);

        mStage = new Stage(new TinyUiHandler());
        mStage.setProjection(Stage.UI_PERSPECTIVE_LHC, 2.0f, 3000.0f, -2111.0f);

        WorldStageView customView = new WorldStageView(this, mStage);
        customView.activateStereo3D(true);

        FrameLayout layout = (FrameLayout)findViewById(R.id.stage_root);
        layout.addView(customView);

        AnimationLoader.setCacheDir(getCacheDir());
        mGestureDetector = new GestureDetector(this, new TinyGestureListener());

        setupBackground();

        mSceneManager = new RotationManager(this, mStage);
        mSceneManager.init();
        mSceneManager.startTitle();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mIsOnPausedCalled = false;
    }

    @Override
    protected void onPause() {
        mIsOnPausedCalled = true;
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        return super.onTouchEvent(e);
    }
    @Override
    public boolean dispatchTouchEvent(MotionEvent e) {
        return super.dispatchTouchEvent(e) || mGestureDetector.onTouchEvent(e);
    }

    private class TinyGestureListener extends GestureDetector.SimpleOnGestureListener {
        private final Point mHitPoint = new Point(0, 0);
        private boolean isPositive(float val) { return (val < 0); }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent event) {
            if (event.getAction() != MotionEvent.ACTION_DOWN) {
                return false;
            }

            mHitPoint.set(event.getX(), event.getY(), 0);
            mSceneManager.hit(mStage.hitTest(new Point(event.getX(), event.getY())), mHitPoint);
            return true;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            float absVelX = Math.abs(velocityX);
            float absVelY = Math.abs(velocityY);
            final int flingThreshold = 200;
            if (absVelX > absVelY) {
                if (absVelX < flingThreshold) {
                    return false;
                }
                mSceneManager.rotate(isPositive(velocityX));
            }
            return true;
        }
    }

    private static class TinyUiHandler implements UiHandler {
        private final Handler mHandler = new Handler();
        public void post(Runnable runnable) {
            mHandler.post(runnable);
        }
    }

    // For Testing.
    public RotateItem getCurrentFocusItem() {
        return mSceneManager.getFocus();
    }

    // For Testing.
    volatile boolean mIsOnPausedCalled;

    // For Testing.
    public boolean isOnPausedCalled() {
        return mIsOnPausedCalled;
    }

    private static final String N3D_NOT_SUPPORT = "0";
    private static final String N3D_SUPPORT = "1";
    public static Boolean isN3DSupported() {
        String option = SystemProperties.get("Camera.Stereo.3D.Support", N3D_NOT_SUPPORT);
        Log.v("World3D", "Camera N3D support value : " + option + ", equals : " + N3D_SUPPORT.equals(option));
        return N3D_SUPPORT.equals(option);
    }
}