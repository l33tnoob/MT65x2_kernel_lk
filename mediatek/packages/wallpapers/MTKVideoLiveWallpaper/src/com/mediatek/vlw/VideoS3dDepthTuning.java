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

package com.mediatek.vlw;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.WallpaperManager;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.os.storage.StorageVolume;
import android.service.wallpaper.IWallpaperConnection;
import android.service.wallpaper.IWallpaperEngine;
import android.service.wallpaper.IWallpaperService;
import android.service.wallpaper.WallpaperService;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.SeekBar;

import com.mediatek.xlog.Xlog;

public class VideoS3dDepthTuning extends Activity implements OnSharedPreferenceChangeListener {
    private static final String TAG = "VideoS3dDepthTuning";
    public static final int CONVERGENCE_MAX = 255;

    static private boolean mIsFolderMode;
    private Intent mWallpaperIntent;
    private WallpaperManager mWallpaperManager;
    private WallpaperConnection mWallpaperConnection;
    private SeekBar mDepthSlider;
    private View mView;

    private SharedPreferences mSharedPref;
    private Uri mUri;
    private String mBucketID;
    private int mConvValueBkup;
    private int mConvValue;
    private int mS3dType = -2; // -1 unknow

    private final BroadcastReceiver mSDCardReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            final StorageVolume sv = (StorageVolume) intent.getExtra(StorageVolume.EXTRA_STORAGE_VOLUME);
            String path = null;
            if (sv != null) {
                path = sv.getPath();
            }
            if (path == null) {
                Uri data = intent.getData();
                if (data != null && data.getScheme().equals("file")) {
                    path = data.getPath();
                }
            }
            // Need to use '/' at the end to verify sdcard or sdcard2.
            path += "/";
            Xlog.i(TAG, "Receive intent action=" + action + " path=" + path);

            // Finish activity if the SDCard containing current video is gone
            if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)
                || Intent.ACTION_MEDIA_BAD_REMOVAL.equals(action)
                || Intent.ACTION_MEDIA_REMOVED.equals(action)
                || Intent.ACTION_MEDIA_EJECT.equals(action)) {
                String videoPath = Utils.getVideoPath(context, mUri);
                if (videoPath == null) {
                    videoPath = mUri.getPath();
                }
                if (videoPath != null && path != null && videoPath.contains(path)) {
                	if (mWallpaperConnection != null && mWallpaperConnection.mConnected) {
                        mWallpaperConnection.disconnect();
                    }
                    onBtnCancel(mView);
                }
            }
        }
    };

    private final BroadcastReceiver mShutdownReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.v(TAG, "mShutdownReceiver intent = " + action);
            if (Intent.ACTION_SHUTDOWN.equals(action)
                || Intent.ACTION_REBOOT.equals(action)) {
                onBtnCancel(mView);
            }
        }
    };

    static void launchActivity(Activity activity, int code, Intent intent) {
        Xlog.w(TAG, "launchActivity()");

        mIsFolderMode = (boolean)intent.getBooleanExtra("isFolder", false);
        Intent launchIntent = new Intent(activity, VideoS3dDepthTuning.class);
        launchIntent.putExtra(Utils.EXTRA_VLW_S3D_DEPTH_TUNING_INTENT, intent);
        activity.startActivityForResult(launchIntent, code);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mWallpaperIntent = (Intent)this.getIntent().getExtra(Utils.EXTRA_VLW_S3D_DEPTH_TUNING_INTENT);
        if (mWallpaperIntent == null) {
            onBtnCancel(mView);
        }

        mSharedPref = getSharedPreferences(VideoScene.SHARED_PREFS_FILE + ActivityManager.getCurrentUser(),
                Context.MODE_PRIVATE);
        mSharedPref.registerOnSharedPreferenceChangeListener(this);

        if (mIsFolderMode) {
            mBucketID = mWallpaperIntent.getStringExtra("bucketId");
            mUri = Utils.queryUrisFromBucketId(this, mBucketID).get(0);
            Xlog.d(TAG, "onCreate() mBucketID: " + mBucketID + ", mUri: " + mUri);
        } else {
            mUri = mWallpaperIntent.getData();
        }
        updateDepthSlider(mUri);

        Xlog.d(TAG, "onCreate(), intent: "+mWallpaperIntent);
            
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.depth_tuning);
        mView = (View)findViewById(R.id.layoutRoot);
        mDepthSlider = (SeekBar)findViewById(R.id.seekBar);
        mDepthSlider.setOnSeekBarChangeListener(mSeekBarListener);
        mDepthSlider.setMax(CONVERGENCE_MAX);

        if (mSharedPref != null) {
            mConvValue = (int)mSharedPref.getLong(VideoScene.CONVERGENCE_VALUE, CONVERGENCE_MAX/2);
            mDepthSlider.setProgress(mConvValue);
            mConvValueBkup = mConvValue;
        }
        
        mWallpaperManager = WallpaperManager.getInstance(this);
        mWallpaperConnection = new WallpaperConnection();

        if (mSDCardReceiver != null) {
            IntentFilter filter = new IntentFilter();
            // When sdcard is unavailable.
            filter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
            filter.addAction(Intent.ACTION_MEDIA_REMOVED);
            filter.addAction(Intent.ACTION_MEDIA_EJECT);
            filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            filter.addDataScheme("file");
            registerReceiver(mSDCardReceiver, filter);
        }
        
        if (mShutdownReceiver != null) {
            IntentFilter filter = new IntentFilter();
            // Save Settings when shut down.
            filter.addAction(Intent.ACTION_SHUTDOWN);
            filter.addAction(Intent.ACTION_REBOOT);
            registerReceiver(mShutdownReceiver, filter);
        }
    }

    float wx = 0.25f;
    private void setWallpaperVisibility(boolean visible) {
        Xlog.d(TAG, "setWallpaperVisibility(), visible:"+visible);

        if (mWallpaperConnection != null && mWallpaperConnection.mConnected) {
            try {
                mWallpaperConnection.mEngine.setVisibility(visible);
            } catch (RemoteException e) {
                // Ignore
            }    
        }

        /*if (visible == true && mWallpaperConnection != null && mWallpaperConnection.mConnected) {
            mWallpaperManager.setWallpaperOffsetSteps(1.0f, 0.0f);
            mWallpaperManager.setWallpaperOffsets(mView.getWindowToken(), wx, 0);
            wx = (wx == 0.25f ? 0.75f : 0.25f);
        }*/
    }

    @Override
    public void onAttachedToWindow() {
        Xlog.d(TAG, "onAttachedToWindow()");
        super.onAttachedToWindow();

        showLoading();

        mView.post(new Runnable() {
            public void run() {
                if (mWallpaperConnection != null
                        && !mWallpaperConnection.connect()) {
                    mWallpaperConnection = null;
                }
            }
        });
    }

    @Override
    public void onDetachedFromWindow() {
        Xlog.d(TAG, "onDetachedFromWindow()");
        super.onDetachedFromWindow();
        
        //if (mDialog != null) mDialog.dismiss();
        
        if (mWallpaperConnection != null && mWallpaperConnection.mConnected) {
            mWallpaperConnection.disconnect();
        }
        mWallpaperConnection = null;
    }

    private void showLoading() {
        // show loading message
    }

    @Override
    protected void onPause() {
        Xlog.d(TAG, "onPause()");
        super.onPause();

        setWallpaperVisibility(false);

        if (mConvValueBkup != mConvValue) {
            // leaving activity and do not confirm yet, restore to previous convergence value
            setConvergence(mConvValueBkup);
        }
    }

    @Override
    protected void onResume() {
        Xlog.d(TAG, "onResume()");
        super.onResume();

        setWallpaperVisibility(true);

        if (mConvValueBkup != mConvValue) {
            // back to activity, apply new convergence value
            setConvergence(mConvValue);
        }
    }

    @Override
    protected void onDestroy() {
        if (mSDCardReceiver != null) {
            unregisterReceiver(mSDCardReceiver);
        }
        if (mShutdownReceiver != null) {
            unregisterReceiver(mShutdownReceiver);
        }
        super.onDestroy();
    }

    // button hook
    public void onBtnCancel(View v) {
        Xlog.d(TAG, "onBtnCancel()");
        setResult(RESULT_CANCELED);
        finish();
    }

    // button hook
    public void onBtnConfirm(View v) {
        Xlog.d(TAG, "onBtnConfirm()");
        mConvValueBkup = mConvValue;
        saveConvergenceSettings();
        // bypass original intent
        setResult(RESULT_OK, mWallpaperIntent);
        finish();
    }

    SeekBar.OnSeekBarChangeListener mSeekBarListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            Xlog.d(TAG, "mSeekBarListener.onProgressChanged(), progress:"+progress);
            mConvValue = progress;
            
            setConvergence(mConvValue);
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            Xlog.d(TAG, "mSeekBarListener.onStartTrackingTouch()");
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            Xlog.d(TAG, "mSeekBarListener.onStopTrackingTouch()");
        }
    };

    private void saveUriSettings() {
        if (mSharedPref != null && mUri != null) {
            Editor edit = mSharedPref.edit();
            edit.putString(VideoScene.PREVIEW_BUCKET_ID, mBucketID);
            edit.putString(VideoScene.PREVIEW_WALLPAPER_URI, mUri.toString());
            edit.commit();
        }
    }

    private void saveConvergenceSettings() {
        if (mSharedPref != null) {
            Editor edit = mSharedPref.edit();
            edit.putLong(VideoScene.CONVERGENCE_VALUE, (long)mConvValue);
            edit.commit();
        }
    }

    private void setConvergence(int convergence) {
        if (mWallpaperConnection != null && mWallpaperConnection.mEngine != null) {
            Xlog.d(TAG, "setConvergence(), "+convergence);
            try {
                mWallpaperConnection.mEngine.setFlagsEx(convergence, Utils.FLAG_EX_S3D_CONVERGENCE);
            } catch (RemoteException e) {
                // Ignore
            }
        }
    }

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        Xlog.d(TAG, "onSharedPreferenceChanged(), key:"+key);
        if (mSharedPref != null && key == VideoScene.STEREO_TYPE_CHANGE) {
            int s3dType = (int) mSharedPref.getLong(VideoScene.STEREO_TYPE, -1);
            if (mS3dType != s3dType) {
                updateDepthSlider(mUri);
                mS3dType = s3dType;
            }
            return;
        }
        if (mSharedPref != null && key == VideoScene.PREVIEW_WALLPAPER_URI) {
            Uri uri = Uri.parse(mSharedPref.getString(VideoScene.PREVIEW_WALLPAPER_URI, null));
            if (mUri != uri) {
                Xlog.d(TAG, "onSharedPreferenceChanged(), uri change:" + mUri + " change to " + uri);
                mUri = uri;
                updateDepthSlider(mUri);
            }
            return;
        }
        if (mSharedPref != null && key == VideoScene.STEREO_TYPE) {
            Xlog.d(TAG, "onSharedPreferenceChanged(), stereo type change: "
                    + mSharedPref.getLong(VideoScene.STEREO_TYPE, -1));
            updateDepthSlider(mUri);
        }
    }

    private void updateDepthSlider(Uri uri) {
        if (mDepthSlider != null) {            
            boolean isStereo = (uri != null && Utils.isStereoVideo(this, uri));
            Xlog.d(TAG, "mDepthSlider.setEnabled: " + isStereo);
            mDepthSlider.setEnabled(isStereo);
        }
    }    

    class WallpaperConnection extends IWallpaperConnection.Stub implements ServiceConnection {
        final Intent mIntent;
        IWallpaperService mService;
        IWallpaperEngine mEngine;
        boolean mConnected;

        WallpaperConnection() {
            mIntent = new Intent(WallpaperService.SERVICE_INTERFACE);
            mIntent.setClassName(Utils.VIDEO_LIVE_WALLPAPER_PACKAGE, Utils.VIDEO_LIVE_WALLPAPER_CLASS);
        }

        public boolean connect() {
            Xlog.w(TAG, "connect()");
            synchronized (this) {
                if (!bindService(mIntent, this, Context.BIND_AUTO_CREATE)) {
                    Xlog.w(TAG, "fail to bindService");        
                    return false;
                }
                
                mConnected = true;
                return true;
            }
        }
        
        public void disconnect() {
            Xlog.w(TAG, "disconnect()");
            synchronized (this) {
                mConnected = false;
                if (mEngine != null) {
                    try {
                        mEngine.destroy();
                    } catch (RemoteException e) {
                        // Ignore
                    }
                    mEngine = null;
                }
                unbindService(this);
                mService = null;
            }
        }
        
        public void onServiceConnected(ComponentName name, IBinder service) {
            Xlog.w(TAG, "onServiceConnected()");
            if (mWallpaperConnection == this) {
                mService = IWallpaperService.Stub.asInterface(service);
                try {
                    final View root = mView.getRootView();
                    mService.attach(this, root.getWindowToken(),
                            WindowManager.LayoutParams.TYPE_APPLICATION_MEDIA_OVERLAY,
                            true, root.getWidth(), root.getHeight());
                } catch (RemoteException e) {
                    Xlog.w(TAG, "Failed attaching wallpaper; clearing", e);
                }
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Xlog.w(TAG, "onServiceDisconnected()");
            mService = null;
            mEngine = null;
            if (mWallpaperConnection == this) {
                Xlog.w(TAG, "Wallpaper service gone: " + name);
            }
        }
        
        public void attachEngine(IWallpaperEngine engine) {
            synchronized (this) {
                if (mConnected) {
                    mEngine = engine;
                    saveUriSettings();
                    try {
                        engine.setVisibility(true);
                        engine.setAlignCenter(mView.getWidth());
                    } catch (RemoteException e) {
                        // Ignore
                    }
                } else {
                    try {
                        engine.destroy();
                    } catch (RemoteException e) {
                        // Ignore
                    }
                }
            }
        }
        
        public ParcelFileDescriptor setWallpaper(String name) {
            return null;
        }

        @Override
        public void engineShown(IWallpaperEngine engine) throws RemoteException {
        }
    }
}

