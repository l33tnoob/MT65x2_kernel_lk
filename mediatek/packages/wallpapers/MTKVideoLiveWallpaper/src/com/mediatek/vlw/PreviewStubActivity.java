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

import android.app.Activity;
import android.app.WallpaperManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.service.wallpaper.IWallpaperConnection;
import android.service.wallpaper.IWallpaperEngine;
import android.service.wallpaper.IWallpaperService;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.LinearLayout;

import com.mediatek.xlog.Xlog;


public class PreviewStubActivity extends Activity {
    private static final boolean DEBUG = false;
    private static final String LOG_TAG = "PreviewStubActivity";

    private WallpaperConnection mWallpaperConnection;
    
    // auto test condition
    private boolean mServiceConnected;
    private boolean mEngineConnected;
    private boolean mAttachedToWindow;

    public boolean serviceConnected() {
        return mServiceConnected;
    }
    
    public boolean engineConnected() {
        return mEngineConnected;
    }
    
    public boolean attachedToWindow() {
        return mAttachedToWindow;
    }

    public boolean detachedFromWindow() {
        return !mAttachedToWindow;
    }
    
    public void sendWallpaperCommand(String command) {
        WallpaperManager.getInstance(this).sendWallpaperCommand(
                getWindow().getDecorView().getWindowToken(), command, 0, 0, 0, null);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onCreate() ");
        }
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        LinearLayout layout = new LinearLayout(this);
        LayoutParams params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        layout.setLayoutParams(params);
        setContentView(layout);
        
        Intent vlw = new Intent(getApplicationContext(), VideoLiveWallpaper.class);
        mWallpaperConnection = new WallpaperConnection(vlw);
    }

    @Override
    public void onResume() {
        super.onResume();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onResume() ");
        }
        if (mWallpaperConnection != null && mWallpaperConnection.mEngine != null) {
            try {
                mWallpaperConnection.mEngine.setVisibility(true);
            } catch (RemoteException e) {
                Xlog.w(LOG_TAG, "Failed set mEngine visible ", e);
            }
        }
    }
    
    @Override
    public void onPause() {
        super.onPause();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onPause() ");
        }
        if (mWallpaperConnection != null && mWallpaperConnection.mEngine != null) {
            try {
                mWallpaperConnection.mEngine.setVisibility(false);
            } catch (RemoteException e) {
                Xlog.w(LOG_TAG, "Failed set mEngine invisible ", e);
            }
        }
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onAttachedToWindow() ");
        }
        mAttachedToWindow = true;
        Handler handler = new Handler();
        handler.post(new Runnable() {
            public void run() {
                if (!mWallpaperConnection.connect()) {
                    mWallpaperConnection = null;
                }
            }
        });
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (DEBUG) {
            Xlog.d(LOG_TAG, "onDetachedFromWindow() ");
        }
        mAttachedToWindow = false;
        if (mWallpaperConnection != null) {
            mWallpaperConnection.disconnect();
        }
        mWallpaperConnection = null;
    }
    
    class WallpaperConnection extends IWallpaperConnection.Stub implements ServiceConnection {
        final Intent mIntent;
        IWallpaperService mService;
        IWallpaperEngine mEngine;
        boolean mConnected;

        WallpaperConnection(Intent intent) {
            mIntent = intent;
        }

        public boolean connect() {
            synchronized (this) {
                if (!bindService(mIntent, this, Context.BIND_AUTO_CREATE)) {
                    Xlog.e(LOG_TAG, "connect() bindService failed!!");
                    return false;
                }
                
                mConnected = true;
                return true;
            }
        }
        
        public void disconnect() {
            if (DEBUG) {
                Xlog.d(LOG_TAG, "disconnect() disconnecting...");
            }
            synchronized (this) {
                mConnected = false;
                if (mEngine != null) {
                    try {
                        mEngine.destroy();
                    } catch (RemoteException e) {
                        Xlog.w(LOG_TAG, "Failed destory mEngine ", e);
                    }
                    mEngine = null;
                }
                unbindService(this);
                mService = null;
                
            }
        }
        
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (mWallpaperConnection == this) {
                Xlog.d(LOG_TAG, "connect() successfully");
                mServiceConnected = true;
                
                mService = IWallpaperService.Stub.asInterface(service);
                try {
                    final View decorView = getWindow().getDecorView();
                    mService.attach(this, decorView.getWindowToken(),
                            WindowManager.LayoutParams.TYPE_APPLICATION_MEDIA_OVERLAY,
                            true, decorView.getWidth(), decorView.getHeight());
                } catch (RemoteException e) {
                    Xlog.w(LOG_TAG, "Failed attaching wallpaper; clearing", e);
                }
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            mServiceConnected = false;
            mEngineConnected = false;
            mService = null;
            mEngine = null;
            if (mWallpaperConnection == this) {
                Xlog.w(LOG_TAG, "Wallpaper service gone: " + name);
            }
        }
        
        public void attachEngine(IWallpaperEngine engine) {
            if (DEBUG) {
                Xlog.d(LOG_TAG, "attachEngine()");
            }
            synchronized (this) {
                if (mConnected) {
                    mEngineConnected = true;
                    mEngine = engine;
                    try {
                        engine.setVisibility(true);
                    } catch (RemoteException e) {
                        Xlog.w(LOG_TAG, "Failed set engine visible ", e);
                    }
                } else {
                    try {
                        mEngineConnected = false;
                        engine.destroy();
                    } catch (RemoteException e) {
                        Xlog.w(LOG_TAG, "Failed destory engine ", e);
                    }
                }
            }
        }

        public void engineShown(IWallpaperEngine engine) {

        }

        public ParcelFileDescriptor setWallpaper(String name) {
            return null;
        }
    }
}
