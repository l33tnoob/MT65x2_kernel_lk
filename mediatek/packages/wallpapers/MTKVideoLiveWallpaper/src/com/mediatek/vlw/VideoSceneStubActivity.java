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

import android.app.Activity;
import android.graphics.Rect;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;

import com.mediatek.xlog.Xlog;

public class VideoSceneStubActivity extends Activity implements SurfaceHolder.Callback {
    static final String LOG_TAG = "VideoSceneStubActivity";
    static final boolean DEBUG = true;

    private VideoScene mRenderer;
    private SurfaceHolder mSurfaceHolder;
    private Handler mHandler;
    private Rect mSurfaceFrame = new Rect();
    private boolean mCompletionCalled;
    
    public VideoScene getRenderer() {
        return mRenderer;
    }

    public SurfaceHolder getSurfaceHolder() {
        return mSurfaceHolder;
    }
    
    public Rect getSurfaceFrame() {
        return mSurfaceFrame;
    }
    
    public boolean getCompletionState() {
        return mCompletionCalled;
    }
    
    public void setFixedSize(final int width, final int height) {
        if (mSurfaceHolder != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mSurfaceHolder.setFixedSize(width, height);
                }
            });
        }
    }
    
    public void setVisibility(final boolean visible) {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.setVisibility(visible);
                }
            });
        }
    }
    
    public void startPlayback() {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.startPlayback();
                }
            });
        }
    }
    
    public void stopPlayback() {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.stopPlayback();
                }
            });
        }
    }
    
    public void startPlayer() {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.start();
                }
            });
        }
    }
    
    public void pausePlayer() {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.pause();
                }
            });
        }
    }
    
    public void seekPlayer(final long pos) {
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.seekTo(pos);
                }
            });
        }
    }

   public void hanleInvlaid(){
        if (mRenderer != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRenderer.handleInvalid();
                }
            });
        }
    }
   
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        LinearLayout layout = new LinearLayout(this);
        LayoutParams params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        layout.setLayoutParams(params);
        SurfaceView surface = new SurfaceView(this);
        layout.addView(surface);
        setContentView(layout);
        
        SurfaceHolder holder = surface.getHolder();
        holder.addCallback(this);
        holder.setSizeFromLayout();
        mSurfaceHolder = holder;
        mHandler = new Handler();
    }

    public void surfaceChanged(SurfaceHolder holder, int format,
            int width, int height) {
        if (DEBUG) {
            Xlog.i(LOG_TAG, String.format(
                    "surfaceChanged() width=%d, height=%d", width, height));
        }
        mSurfaceHolder = holder;
        Rect frame = holder.getSurfaceFrame();
        if (frame.width() != width || frame.height() != height) {
            mSurfaceFrame = new Rect(0, 0, width, height);
        } else {
            mSurfaceFrame = frame;
        }
        
        if (mRenderer != null) {
            mRenderer.resize(holder, width, height);

        }
    }

    public void surfaceCreated(SurfaceHolder holder) {
        if (DEBUG) {
            Xlog.i(LOG_TAG, "surfaceCreated() " + holder);
        }
        mSurfaceHolder = holder;
        
        Surface surface = null;
        while (surface == null) {
            surface = holder.getSurface();
        }
        if (mRenderer == null) {
            mRenderer = new VideoScene(this, holder, false);
            mRenderer.init(holder);
            mRenderer.start();
            
            mRenderer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                @Override
                public void onCompletion(MediaPlayer mp) {
                    // TODO Auto-generated method stub
                    mCompletionCalled = true;
                }
            });
        }
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        if (DEBUG) {
            Xlog.i(LOG_TAG, "surfaceDestroyed() " + holder);
        }
        mSurfaceHolder = null;
        
        if (mRenderer != null) {
            mRenderer.destroy();
            mRenderer = null;
        }
    }

}
