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
import android.os.Bundle;
import android.service.wallpaper.WallpaperService;
import android.view.SurfaceHolder;
import android.view.WindowManager;

public class Stereo3DPairWallpaperService extends WallpaperService {
    private static final String TAG = "Stereo3DPairWallpaperService";

    @Override
    public Engine onCreateEngine() {
        return new Stereo3DPairWallpaperEngine();
    }

    class Stereo3DPairWallpaperEngine extends Engine {
        private Stereo3DPairWallpaperEngine mEngine;
        private Stereo3DSource mSource;
        private Stereo3DRenderer mRenderer;
        private SurfaceHolder mSurfaceHolder;
        private WallpaperObserver mReceiver;

        class WallpaperObserver extends BroadcastReceiver {
            public void onReceive(Context context, Intent intent) {

                // for auto test
                if (intent.getAction().equals(Stereo3DWallpaperManagerService.VISIBILITY_CHANGED)) {
                    Bundle extras = intent.getExtras();
                    boolean visible = extras.getBoolean("visible");

                    Stereo3DLog.log(TAG, "Receive visibility changed intent: " + visible);
                    onVisibilityChanged(visible);
                    return;
                }
            }
        }

        public Stereo3DPairWallpaperEngine() {
            super();
        }

        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            super.onCreate(surfaceHolder);
            Stereo3DLog.log(TAG, "onCreate(): " + this);

            mReceiver = new WallpaperObserver();

            // register intent filter
            IntentFilter visibilityFilter = new IntentFilter(
                Stereo3DWallpaperManagerService.VISIBILITY_CHANGED);
            registerReceiver(mReceiver, visibilityFilter);

            mRenderer = new Stereo3DRenderer();
            mEngine = this;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            Stereo3DLog.log(TAG, "onDestroy(): " + this);

            unregisterReceiver(mReceiver);
            Stereo3DUtility.setScreenDimension(0, 0);

            if (mSource != null) {
                mSource.release();
            }
        }

        /**
         * This method sets the wallpaper.
         */
        private void setWallpaper() {
            if (mSource == null) {
                mSource = new Stereo3DSource();
            }

            mSource.getPairBitmaps();

            if (mSource.isValid()) {
                if (mRenderer != null) {
                    mRenderer.setSource(mSource);
                    mRenderer.render(mSurfaceHolder);
                }
            }
        }

        /**
         * This method updates the wallpaper.
         */
        private void updateWallpaper() {
            mSource.release();
            setWallpaper();
        }

        @Override
        public void onSurfaceCreated(SurfaceHolder holder) {
            super.onSurfaceCreated(holder);
            Stereo3DLog.log(TAG, "onSurfaceCreated");
            mSurfaceHolder = holder;
        }

        @Override
        public void onSurfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            super.onSurfaceChanged(holder, format, width, height);

            Stereo3DLog.log(TAG, "onSurfaceChanged - width: " + width + "; height: " + height);

            if (width == Stereo3DUtility.sScreenWidth && height == Stereo3DUtility.sScreenHeight) {
                if (mSource == null) {
                    Stereo3DLog.log(TAG, "Create source");
                    setWallpaper();
                }
            } else {
                Stereo3DUtility.setScreenDimension(width, height);
                mEngine.setFlagsEx(WindowManager.LayoutParams.FLAG_EX_S3D_3D |
                        WindowManager.LayoutParams.FLAG_EX_S3D_SIDE_BY_SIDE,
                        WindowManager.LayoutParams.FLAG_EX_S3D_MASK);
                setWallpaper();
            }
        }

        @Override
        public void onVisibilityChanged(boolean visible) {
            Stereo3DLog.log(TAG, "onVisibilityChanged(): " + visible + "; id: " + this);

            if (visible) {
                Stereo3DLog.log(TAG, "onVisibilityChanged - 3D mode: SBS");
                mEngine.setFlagsEx(WindowManager.LayoutParams.FLAG_EX_S3D_3D |
                        WindowManager.LayoutParams.FLAG_EX_S3D_SIDE_BY_SIDE,
                        WindowManager.LayoutParams.FLAG_EX_S3D_MASK);
            } else {
                Stereo3DLog.log(TAG, "onVisibilityChanged() - 3D mode: DISABLE");
                mEngine.setFlagsEx(WindowManager.LayoutParams.FLAG_EX_S3D_2D,
                        WindowManager.LayoutParams.FLAG_EX_S3D_MASK);
            }

            // render 2D image after switching 3D mode off.
            if (mRenderer != null) {
                mRenderer.set3DMode(visible);
                mRenderer.render(mSurfaceHolder);
            }

            super.onVisibilityChanged(visible);
        }

        @Override
        public void onOffsetsChanged(float xOffset, float yOffset,
                                     float xOffsetStep, float yOffsetStep,
                                     int xPixels, int yPixels) {

            Stereo3DLog.log(TAG, "onOffsetsChanged - xOffset: " + xOffset + "; yOffset: " + yOffset);

            if (mRenderer != null) {
                mRenderer.onOffsetsChanged(xOffset, yOffset);
                mRenderer.render(mSurfaceHolder);
            }
        }
    }
}