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

package com.mediatek.vlw;

import android.os.Bundle;
import android.service.wallpaper.WallpaperService;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

public class VideoLiveWallpaper extends WallpaperService {
    private static final String TAG = "VideoLiveWallpaper";
    
    private static final boolean DEBUG = true;
    
    private static final String HIDE = "hide";
    private static final String SHOW = "show";

    public Engine onCreateEngine() {
        Engine engine = new VLWEngine();
        return engine;
    }

    /**
     * To dynamically switch 2d/3d mode
     * @param flags
     * @param mask
     */
    void setFlagsEx(int flags, int mask, boolean isPreview) {
        ArrayList<Engine> activeEngines = getActiveEngines();
        Engine engine = null;
        for (int i=0; i<activeEngines.size(); i++) {
            engine = activeEngines.get(i);
            if (isPreview && engine.isPreview()) {
                break;
            }
        }
        
        if (FeatureOption.MTK_S3D_SUPPORT) {
            if (engine != null) {
                engine.setFlagsEx(flags, mask);
            }
        }
    }

    private class VLWEngine extends Engine {
        private VideoScene mRenderer;
        private boolean mVisible;
        
        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            super.onCreate(surfaceHolder);
            if (DEBUG) {
                Xlog.i(TAG, "create wallpaper engine");
            }
            setTouchEventsEnabled(true);
            surfaceHolder.setSizeFromLayout();
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            if (DEBUG) {
                Xlog.i(TAG, "destroy wallpaper engine");
            }
            if (mRenderer != null) {
                mRenderer.destroy();
            }
            mRenderer = null;
        }

        @Override
        public void onDesiredSizeChanged(int desiredWidth, int desiredHeight) {
            if (DEBUG) {
                Xlog.i(TAG, String.format(
                        "The desired size is: desiredWidth=%d, desiredHeight=%d",
                        desiredWidth, desiredHeight));
            }
        }

        @Override
        public void onVisibilityChanged(boolean visible) {
            super.onVisibilityChanged(visible);
            
            boolean visibleChange = visible != mVisible;
            if (DEBUG) {
                Xlog.i(TAG, "visibleChange=" + visibleChange 
                        + ", visible=" + visible);
            }
            if (mRenderer != null && visibleChange) {
                mVisible = visible;
                mRenderer.setVisibility(visible);
                if (visible) {
                    mRenderer.start();
                } else {
                    mRenderer.pause();
                }
            }
        }

        @Override
        public void onSurfaceChanged(SurfaceHolder holder, int format,
                int width, int height) {
            super.onSurfaceChanged(holder, format, width, height);
            if (DEBUG) {
                Xlog.i(TAG, String.format(
                        "surface changed width=%d, height=%d", width, height));
            }
            if (mRenderer != null) {
                mRenderer.resize(holder, width, height);
            }
        }

        @Override
        public void onSurfaceCreated(SurfaceHolder holder) {
            super.onSurfaceCreated(holder);

            Surface surface = null;
            // Here can get a surface anyway and the time will not very long. 
            while (surface == null) {
                surface = holder.getSurface();
            }
            if (DEBUG) {
                Xlog.i(TAG, "onSurfaceCreated(), surface: " + surface);
            }
            if (mRenderer == null) {
                mRenderer = new VideoScene(VideoLiveWallpaper.this,
                        holder, isPreview());
                mRenderer.init(holder);
                mRenderer.start();
            }
        }

        @Override
        public void onSurfaceDestroyed(SurfaceHolder holder) {
            super.onSurfaceDestroyed(holder);
            if (DEBUG) {
                Xlog.i(TAG, "onSurfaceDestroyed()");
            }
            if (mRenderer != null) {
                mRenderer.destroy();
                mRenderer = null;
            }
        }

        @Override
        public Bundle onCommand(String action, int x, int y, int z,
                Bundle extras, boolean resultRequested) {
            if (HIDE.equals(action)) {
                if (mRenderer != null) {
                    mRenderer.pause();
                }
            } else if (SHOW.equals(action)) {
                if (mRenderer != null) {
                    mRenderer.start();
                }
            }
            
            if (mRenderer != null) {
                return mRenderer.doCommand(action, x, y, z, extras, resultRequested);
            }
            
            return null;
        }
    }
}
