/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.stereo3dwallpaper;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.WindowManager;

import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageWallpaperService;

public class Stereo3DWallpaperService extends StageWallpaperService {
    private static final String TAG = "Stereo3DWallpaperService";
    private static final int WALLPAPER_TAG = 0;
    private static final float CAMERA_Z = -1111;
    private static final float ORIGIN_X = 0;
    private static final float ORIGIN_Y = 0;
    private static final float LOOK_AT_Z = 300;
    private static final float FOCAL_LENGTH = 1111f;
    private static final float MIN_HEIGHT = 1600;
    private static final float LEFT_BOUND = 0.15f;
    private static final float RIGHT_BOUND = 0.85f;
    private static final float DEFAULT_Z_NEAR = 1000;
    private static final float DEFAULT_Z_FAR = 2000;

    public Stereo3DWallpaperService() {
        super();
        Stereo3DLog.log(TAG, "Use a3m engine");
    }

    public Engine onCreateEngine() {
        return new Stereo3DWallpaperEngine();
    }

    class Stereo3DWallpaperEngine extends StageEngine {
        private Stereo3DWallpaperEngine mEngine;
        private final Stage mStage;
        private Container mContainer;
        private WallpaperObserver mReceiver;
        private SharedPreferences mPreferences;
        private Image mWallpaper;
        private float mNewPosX;
        private float mNewPosY;
        private float mLastXOffset;
        private float mLastYOffset;
        private float mWallpaperWidth;
        private float mWallpaperHeight;
        private float mSurfaceWidth;
        private float mSurfaceHeight;
        private float mScreenWidth;
        private float mScreenHeight;
        private int mWallpaperId = R.drawable.wallpaper_01;

        class WallpaperObserver extends BroadcastReceiver {
            public void onReceive(Context context, Intent intent) {

                // for auto test
                if (intent.getAction().equals(Stereo3DWallpaperChooserFragment.VISIBILITY_CHANGED)) {
                    Bundle extras = intent.getExtras();
                    boolean visible = extras.getBoolean("visible");

                    Stereo3DLog.log(TAG, "Receive visibility changed intent: " + visible);
                    onVisibilityChanged(visible);
                    return;
                }
            }
        }

        public Stereo3DWallpaperEngine() {
            super();

            mStage = getStage();
            mEngine = this;
        }

        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            super.onCreate(surfaceHolder);
            Stereo3DLog.log(TAG, "onCreate(): " + this);

            mReceiver = new WallpaperObserver();

            // register intent filter
            IntentFilter visibilityFilter = new IntentFilter(
                    Stereo3DWallpaperChooserFragment.VISIBILITY_CHANGED);
            registerReceiver(mReceiver, visibilityFilter);

            // get share preference
            mPreferences = PreferenceManager.getDefaultSharedPreferences(Stereo3DWallpaperService.this);

            // set wallpaper
            setStage();
            mContainer = new Container();
            setWallpaper(getPrefsWallpaperId());
            mStage.add(mContainer);
            setTouchEventsEnabled(true);
        }

        @Override
        public void onResume() {
            super.onResume();
            Stereo3DLog.log(TAG, "onResume(): " + this);

            mEngine = this;
        }

        @Override
        public void onPause() {
            super.onPause();
            Stereo3DLog.log(TAG, "onPause(): " + this);
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            Stereo3DLog.log(TAG, "onDestroy(): " + this);

            unregisterReceiver(mReceiver);
        }

        /**
         * This method updates the wallpaper.
         * @param id the wallpaper id
         */
        private void updateWallpaper(int id) {
            Stereo3DLog.log(TAG, "Update wallpaper: " + id);

            mContainer.remove(mWallpaper);
            setWallpaper(id);
        }

        /**
         * This method initializes the stage
         */
        private void setStage() {
            Display display = ((WindowManager) getBaseContext().
                               getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

            mScreenWidth = (float)display.getWidth();
            mScreenHeight = (float)display.getHeight();

            Stereo3DLog.log(TAG, "Screen width: " + mScreenWidth);
            Stereo3DLog.log(TAG, "Screen height: " + mScreenHeight);

            // set the stage
            mStage.setCamera(new Point(mScreenWidth / 2.0f, mScreenHeight / 2.0f, CAMERA_Z),
                             new Point(mScreenWidth / 2.0f, mScreenHeight / 2.0f, LOOK_AT_Z));

            // The graphics engine now uses a conventional right-handed system so we
            // use a 'special' projection to compensate for this.
            mStage.setProjection(Stage.UI_PERSPECTIVE_LHC, DEFAULT_Z_NEAR, DEFAULT_Z_FAR, CAMERA_Z);

            mStage.setBackgroundColor(Color.TRANSPARENT);
        }

        /**
         * This method sets the wallpaper.
         * @param id the wallpaper id
         */
        private void setWallpaper(int id) {
            mWallpaper = Image.createFromResource(getResources(), id);
            mWallpaper.setAnchorPoint(new Point(ORIGIN_X, ORIGIN_Y));

            // get wallpaper width and height
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeResource(getResources(), id, options);
            mWallpaperWidth = options.outWidth;
            mWallpaperHeight = options.outHeight;

            // must enlarge size because the photo will shrink when setting to 3D mode
            float scale = MIN_HEIGHT / mWallpaperHeight;

            mWallpaper.setScale(new Scale(scale, scale));
            mWallpaper.setTag(WALLPAPER_TAG);
            mWallpaperId = id;

            mWallpaperWidth = options.outWidth * scale;
            mWallpaperHeight = options.outHeight * scale;
            Stereo3DLog.log(TAG, "Wallpaper width: " + mWallpaperWidth);
            Stereo3DLog.log(TAG, "Wallpaper height: " + mWallpaperHeight);

            if (mSurfaceWidth > 0 && mSurfaceHeight > 0) {
                float remainingW = mSurfaceWidth - mWallpaperWidth;
                float remainingH = mSurfaceHeight - mWallpaperHeight;

                // center the wallpaper
                if (mLastXOffset < LEFT_BOUND) {
                    mLastXOffset = 0.5f;
                    mLastYOffset = 0.5f;
                }

                mNewPosX = remainingW < 0 ? (remainingW * mLastXOffset) : (remainingW / 2);
                mNewPosY = remainingH < 0 ? (remainingH * mLastYOffset) : (remainingH / 2);
            } else {
                Stereo3DLog.log(TAG, "mScreenWidth: " + mScreenWidth);
                Stereo3DLog.log(TAG, "mScreenHeight: " + mScreenHeight);

                mNewPosX = (mScreenWidth - mWallpaperWidth) / 2.0f;
                mNewPosY = (mScreenHeight - mWallpaperHeight) / 2.0f;
            }

            mWallpaper.setPosition(new Point(mNewPosX, mNewPosY, LOOK_AT_Z));
            mContainer.add(mWallpaper);
        }

        /**
         * This method gets the wallpaper id from the share preference
         * @return the wallpaper id
         */
        private int getPrefsWallpaperId() {
            return mPreferences.getInt("wallpaper_id", mWallpaperId);
        }

        @Override
        public void onSurfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            super.onSurfaceChanged(holder, format, width, height);

            mSurfaceWidth = (float)width;
            mSurfaceHeight = (float)height;
            mScreenWidth = (float)width;
            mScreenHeight = (float)height;

            Stereo3DLog.log(TAG, "onSurfaceChanged - reset camera");
            mStage.setCamera(new Point(mScreenWidth / 2.0f, mScreenHeight / 2.0f, CAMERA_Z),
                             new Point(mScreenWidth / 2.0f, mScreenHeight / 2.0f, LOOK_AT_Z));
        }

        @Override
        public void onVisibilityChanged(boolean visible) {
            Stereo3DLog.log(TAG, "onVisibilityChanged(): " + visible + "; id: " + this);

            if (visible) {
                Stereo3DLog.log(TAG, "onVisibilityChanged - 3D mode: SBS");
                mEngine.setFlagsEx(WindowManager.LayoutParams.FLAG_EX_S3D_3D |
                        WindowManager.LayoutParams.FLAG_EX_S3D_SIDE_BY_SIDE,
                        WindowManager.LayoutParams.FLAG_EX_S3D_MASK);
                mStage.setStereo3D(true, FOCAL_LENGTH);
            } else {
                Stereo3DLog.log(TAG, "onVisibilityChanged() - 3D mode: DISABLE");
                mEngine.setFlagsEx(WindowManager.LayoutParams.FLAG_EX_S3D_2D,
                        WindowManager.LayoutParams.FLAG_EX_S3D_MASK);
                mStage.setStereo3D(false, 0);
            }

            // when the visibility is set to false, super must be called at the end
            // because 3D mode have to be turned off before engine pauses
            super.onVisibilityChanged(visible);
        }

        @Override
        public void onOffsetsChanged(float xOffset, float yOffset,
                                     float xOffsetStep, float yOffsetStep,
                                     int xPixels, int yPixels) {

            Stereo3DLog.log(TAG, "onOffsetsChanged - xOffset: " + xOffset + "; yOffset: " + yOffset);

            float x = xOffset;
            float y = yOffset;
            float remainingW = mSurfaceWidth - mWallpaperWidth;
            float remainingH = mSurfaceHeight - mWallpaperHeight;

            // limit the bounds due to parallax
            // if not, may see parallax view on both sides of the image
            if (xOffset < LEFT_BOUND) {
                x = LEFT_BOUND;
            } else if (xOffset > RIGHT_BOUND) {
                x = RIGHT_BOUND;
            }

            mNewPosX = remainingW < 0 ? (remainingW * x) : (remainingW / 2);
            mNewPosY = remainingH < 0 ? (remainingH * y) : (remainingH / 2);
            mLastXOffset = x;
            mLastYOffset = y;
            mWallpaper.setPosition(new Point(mNewPosX, mNewPosY, LOOK_AT_Z));
        }
    }
}
