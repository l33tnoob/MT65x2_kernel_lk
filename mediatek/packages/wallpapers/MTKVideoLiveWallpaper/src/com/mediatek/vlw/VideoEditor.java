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

package com.mediatek.vlw;
import android.app.ActivityManager;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.WallpaperInfo;
import android.app.WallpaperManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ComponentInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.hardware.display.DisplayManager;
import android.media.MediaMetadataRetriever;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.storage.StorageVolume;
import android.provider.MediaStore;
import android.service.wallpaper.WallpaperService;
import android.text.Html;
import android.text.Spanned;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.VideoView;
import android.os.storage.StorageManager;
import android.app.ProgressDialog;

import org.xmlpull.v1.XmlPullParserException;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.drm.OmaDrmStore;
import com.mediatek.vlw.Utils.LoopMode;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class VideoEditor extends Activity {
    private static final String TAG = "VideoEditor";
    
    private static final boolean DEBUG = true;
    
    private static final int PICK_VIDEO_REQUEST = 0;
    private static final int PICK_FOLDER_REQUEST = 1;
    private static final int PICK_CAMERA_REQUEST = 2;
    private static final int DIALOG_SELECT_VIDEO = 1;
    private static final int S3D_FILE_DEPTH_TUNING_REQUEST = 3;
    private static final int S3D_FOLDER_DEPTH_TUNING_REQUEST = 4;
    
    // states we should take care of
    private static final int STATE_ERROR              = -1;
    private static final int STATE_IDLE               = 0;
    private static final int STATE_PLAYING            = 1;
    private static final int STATE_PAUSED             = 2;
    private static final int STATE_PREPARED           = 3;
    private static final int STATE_PLAYBACK_COMPLETED = 4;
    
    private static final String FILE = "file";
    private static final String HTML_BOLD = "<b>";
    private static final String LEFT_BRACKET = "(";
    private static final String RIGHT_BRACKET = ")";
    private static final String SEPARATOR = "/";
    private static final String TYPE = "video/*";
    private static final String ACTION_PICK_FOLDER = "com.mediatek.action.PICK_VIDEO_FOLDER";
    private static final String ACTION_VIDEO_CAPTURE = "android.media.action.VIDEO_CAPTURE";
    
    private static final int DEFAULT_MODE = 1;
    
    private Intent mWallpaperIntent;
    private VideoView mPlayer;
    private VLWMediaController mMediaController;
    private TextView mFolderInfo;
    private TextView mVideoTitle;
    private ImageButton mPlayPause;
    private Button mSetWallpaper;
    
    // Restore the data when sdcard unmounted.
    private Uri mPrevUri;
    private String mPrevBucketId;
    private int mPrevStartTime;
    private int mPrevEndTime;
    private int mPrevCurrentPos;
    //Mark have received the sdcard remove message.
    private boolean mUnMounted;
    //Mark have saved the video Data before sdcard unmounted.
    private boolean mHavaUnmountedData;
    private ProgressDialog mProgressDialog;
    
    // settings information
    private SharedPreferences mSharedPref;
    private int mStartTime;
    private int mEndTime;
    private int mCurrentPos;
    private Uri mUri;
    private ArrayList<Uri> mUriList;
    private ArrayList<Uri> mUriInvalid;
    private String mBucketId;
    private int mMode;
    private LoopMode mLoopMode = LoopMode.ALL;
    private boolean mIsOpening;
    private boolean mClosed;
    private int mCurrentState = STATE_IDLE;
    private int mTargetState = STATE_IDLE;

    private final Handler mHandler = new Handler();
    /// M: for s3d type video
    private int mStereoType;
    private int mPrevStereoType;
    private int mFlagsEx;
    private boolean mIsVisible;
    private boolean mSmartBookPlug;

    private final VLWMediaController.Callback mCallback = new VLWMediaController.Callback() {
        @Override
        public void updateUI(boolean isPlaying) {
            updatePausePlay(isPlaying);
        }

        @Override
        public void updateState(int start, int end) {
            mStartTime = start;
            mEndTime = end;
            // Seek to start whenever in edit mode
            mCurrentPos = start;
        }
    };

    private final OnInfoListener mOnInfoListener = new OnInfoListener() {
        
        @Override
        public boolean onInfo(MediaPlayer mp, int what, int extra) {
            Xlog.d(TAG, "3D Info Listener, what: " + what + ", s3d type: " + extra);
            if (FeatureOption.MTK_S3D_SUPPORT) {
                if (what == Utils.MEDIA_INFO_3D) {
                    if (mStereoType != extra && mStereoType == Utils.STEREO_TYPE_UNKNOWN) {
                        mStereoType = extra;
                        setFlagsEx(true);
                    }
                }
            }
            
            return true;
        }
    };

    private final OnErrorListener mOnErrorListener = new OnErrorListener() {

        @Override
        public boolean onError(MediaPlayer mp, int framworkErr, int implErr) {
            // Handle all errors, play another video or revert to default video.
            if (mBucketId != null) {
                // Update mUriList.
                ArrayList<Uri> newUriList = Utils.queryUrisFromBucketId(VideoEditor.this, mBucketId);
                if (newUriList.contains(mUri)) {
                    if (mUriInvalid == null) {
                        mUriInvalid = new ArrayList<Uri>();
                    }
                    if (!mUriInvalid.contains(mUri)) {
                        mUriInvalid.add(mUri);
                    }
                }
                mUriList = newUriList;
                mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                if (mMode >= 0) {
                    mUri = mUriList.get(mMode);
                    mStereoType = Utils.queryStereoType(VideoEditor.this, mUri);
                } else {
                    Xlog.w(TAG, "Error: No valid videos, play default video");
                    clear(false, true, true);
                    mMode = 0;
                }
                // @{ ALPS00740162 when current video can't play , reset the current position.
                mCurrentPos = VideoScene.DEFAULT_START;
                // @}
            } else {
                Xlog.w(TAG, "errors, play default video");
                clear(false, false);
            }
            Utils.showInfo(VideoEditor.this,R.string.VideoScene_error_text_unknown, true);
            if (mIsVisible) {
                startPlayback();
            }
            
            return true;
        }
    };

    private final OnPreparedListener mOnPreparedListener = new OnPreparedListener() {

        @Override
        public void onPrepared(MediaPlayer mp) {
            mCurrentState = STATE_PREPARED;
            int duration = mPlayer.getDuration();
            int height = mp.getVideoHeight();
            int width = mp.getVideoWidth();
            if (DEBUG) {
                Xlog.d(TAG, "onPrepared, mp = " + mp + ", height = " + height + ", width = " + width);
            }
            mPlayer.setVisibility(View.VISIBLE);
            mPlayPause.setVisibility(View.VISIBLE);
            if (mIsOpening) {
                mEndTime = (mEndTime == VideoScene.DEFAULT_END && duration > 0) ? duration
                        : mEndTime;
                if (mMediaController != null) {
                    mMediaController.initControllerState(mStartTime, mEndTime, duration);
                }
                if (mProgressDialog == null) {
                    //seek to the right position when turn off airplane mode.
                    if ( mPlayer != null && mCurrentPos != 0) {
                        mPlayer.seekTo(mCurrentPos);
                        Xlog.d(TAG, "onPrepared , seekTo " + mCurrentPos);
                    }
                    play();
                }         
                if (height == 0 && width == 0) {
                    if (mUriInvalid == null) {
                        mUriInvalid = new ArrayList<Uri>();
                    }
                    if (!mUriInvalid.contains(mUri)) {
                        mUriInvalid.add(mUri);
                    }
                    Xlog.w(TAG, "onPrepared() warning: " + mUri
                            + " is invalid:" + " w=" + width + ",h=" + height);
                }
                mIsOpening = false;
            } else if (mMediaController != null) {
                mMediaController.traceBack(mCurrentPos);
            } else {
                if (DEBUG) {
                    Xlog.d("VideoEditor", "traceBack curPos=" + mCurrentPos);
                }
                mPlayer.seekTo(mCurrentPos);
            }
        }
    };

    private final OnCompletionListener mOnCompletionListener = new OnCompletionListener() {
        // If we set endTime, the media controller will take care of this, but
        // if not, catch it here.
        @Override
        public void onCompletion(final MediaPlayer mp) {
            mCurrentState = STATE_PLAYBACK_COMPLETED;
            // Loop mode
            final int duration = mPlayer.getDuration();
            if (DEBUG) {
                Xlog.d(TAG, "onCompletion mCurrentState = " + mCurrentState
                        + ",duration = " + duration + ",mTargetState = "
                        + mTargetState + ",mBucketId = " + mBucketId
                        + ",mMode = " + mMode + ",mLoopMode = " + mLoopMode
                        + ",uris = " + (mUriList != null ? mUriList.size() : 0) 
                        + ",invalidUris = " 
                        + (mUriInvalid != null ? mUriInvalid.size() : 0));
            }

            if (mTargetState == STATE_PLAYING) {
                if (mBucketId != null) {
                    mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                    if (mMode >= 0) {
                        Uri oldUri = mUri;
                        mUri = mUriList.get(mMode);
                        if (oldUri.equals(mUri)) {
                            mHandler.post(new Runnable() {

                                public void run() {
                                    // TODO Auto-generated method stub
                                    play();
                                    Xlog.v(TAG, "Play single video in folder mode.");
                                }
                            });
                        } else {
                            oldUri = mUri;
                            clear(false, false);
                            mUri = oldUri;
                            mStereoType = Utils.queryStereoType(VideoEditor.this, mUri);
                            // Make sure MediaPlayerService has disconnected
                            // from SurfaceTexture
                            mHandler.post(new Runnable() {

                                public void run() {
                                    stopPlayback();
                                    startPlayback();
                                    }
                                });
                        }
                    } else {
                        mHandler.post(new Runnable() {

                            public void run() {
                                mp.seekTo(0);
                                mp.start();
                                }
                            });
                    }

                } else if (mMediaController != null) {
                    mHandler.post(new Runnable() {

                        public void run() {
                            // TODO Auto-generated method stub
                            mMediaController.initControllerState(mStartTime,
                                    mEndTime, duration);
                            mMediaController.play();
                            mTargetState = STATE_PLAYING;
                            Xlog.v(TAG, "Play the same video in single video mode.");
                        }
                    });
                }
            }
        }
    };
    
    private final BroadcastReceiver mVideoEditorReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_SMARTBOOK_PLUG.equals(action)) {
                boolean smartBookPlug = intent.getBooleanExtra(Intent.EXTRA_SMARTBOOK_PLUG_STATE, false);
                Xlog.d(TAG, "Receive EXTRA_SMARTBOOK_PLUG_STATE , Smart Book plug in: " + smartBookPlug);
                if (mSmartBookPlug != smartBookPlug) {
                    mSmartBookPlug = smartBookPlug;
                    if (mSmartBookPlug) {
                        mSharedPref = context.getSharedPreferences(VideoScene.SMARTBOOK_SHARED_PREFS_FILE,
                                Context.MODE_PRIVATE);
                    } else {
                        mSharedPref = context.getSharedPreferences(VideoScene.SHARED_PREFS_FILE, Context.MODE_PRIVATE);
                    }
                    loadSettings();
                    startPlayback();
                }
            }
        }
    };
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        // When sdcard is removed, notify user and then play the default video.
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            final StorageVolume sv = (StorageVolume) intent
                    .getExtra(StorageVolume.EXTRA_STORAGE_VOLUME);
            String path = null;
            if (sv != null) {
                path = sv.getPath();
            }
            if (path == null) {
                Uri data = intent.getData();
                if (data != null && data.getScheme().equals(FILE)) {
                    path = data.getPath();
                }
            }
            // Need to use '/' at the end to verify sdcard or sdcard2.
            path += "/";
            Xlog.i(TAG, " mReceive intent action=" + action + " path=" + path
                    + " mUri=" + mUri);
            if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                if (mPrevUri != null && mHavaUnmountedData) {
                    if (mPrevBucketId == null) {
                        restoreMountedData();
                        if (mIsVisible) {
                            startPlayback();
                        }
                    } else {
                        if (mProgressDialog == null) {
                            mProgressDialog = new ProgressDialog(VideoEditor.this);
                            mProgressDialog
                                    .setProgressStyle(ProgressDialog.STYLE_SPINNER);
                            mProgressDialog
                                    .setMessage(getString(R.string.VideoScene_reload_after_mount));
                            mProgressDialog.show();
                            Xlog.d(TAG, "create dialog"); 
                        }                       
                    }
                }
            } else if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                if (mPrevUri != null && mPrevBucketId != null
                        && mHavaUnmountedData && mUnMounted) {
                    mBucketId = mPrevBucketId;
                    Xlog.d(TAG,
                            "Receive ACTION_MEDIA_SCANNER_FINISHED , mBucketId"
                                    + mBucketId);
                    mUriList = Utils.queryUrisFromBucketId(VideoEditor.this,
                            mBucketId);
                    restoreMountedData();
                    if (mProgressDialog != null) {
                        mProgressDialog.dismiss();
                    }
                    mProgressDialog = null;
                    if (mIsVisible) {
                        startPlayback();
                    }
                    mHavaUnmountedData = false;
                    mUnMounted = false;
                }
                // Update it if needed when it is coming
                updateInfo();
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)
                    || Intent.ACTION_MEDIA_BAD_REMOVAL.equals(action)
                    || Intent.ACTION_MEDIA_REMOVED.equals(action)
                    || Intent.ACTION_MEDIA_EJECT.equals(action)) {
                if(Intent.ACTION_MEDIA_UNMOUNTED.equals(action)){
                    mUnMounted = true;
                }
                String videoPath = mUri.getPath();
                if (videoPath != null && videoPath.contains(path)) {
                    Xlog.w(TAG, "action: " + action
                                    + " revert to default video. sdcard path: "
                                    + path + " absolute path: " + videoPath
                                    + " mUri: " + mUri);
                    saveUnmountedData();
                    clear(false, true, true);
                    if (mIsVisible) {
                        startPlayback();
                    }
                }
            }
        }
    };
    
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.editor);
        mWallpaperIntent = null;
        mSmartBookPlug = isSmartBookPluggedIn();
        if (mSmartBookPlug) {
            Xlog.i(TAG, "oncreate(),Smart book is plug in");
            mSharedPref = getSharedPreferences(VideoScene.SMARTBOOK_SHARED_PREFS_FILE, Context.MODE_PRIVATE);
        } else {
            mSharedPref = getSharedPreferences(VideoScene.SHARED_PREFS_FILE + ActivityManager.getCurrentUser(),
                    Context.MODE_PRIVATE);
        }
        mPlayer = (VLWVideoView) findViewById(R.id.player);
        mPlayer.setOnPreparedListener(mOnPreparedListener);
        mPlayer.setOnErrorListener(mOnErrorListener);
        mPlayer.setOnCompletionListener(mOnCompletionListener);
        mPlayer.setOnInfoListener(mOnInfoListener);
        mVideoTitle = (TextView) findViewById(R.id.video_title);
        mPlayPause = (ImageButton) findViewById(R.id.play_pause);
        mPlayPause.requestFocus();
        mSetWallpaper = (Button) findViewById(R.id.set_wallpaper);

        loadSettings();
        
        // Restore saved state if needed.
        if (savedInstanceState != null) {
            restoreSavedInstance(savedInstanceState);
        }
        
        startPlayback();

        if (mReceiver != null) {
            Xlog.i(TAG, "onCreate() register receiver: " + mReceiver);
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            // When external storage scanning complete.
            filter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
            filter.addAction(Intent.ACTION_MEDIA_REMOVED);
            filter.addAction(Intent.ACTION_MEDIA_EJECT);
            filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            filter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
            filter.addDataScheme(FILE);
            registerReceiver(mReceiver, filter);
        }

        if (FeatureOption.MTK_SMARTBOOK_SUPPORT && mVideoEditorReceiver != null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SMARTBOOK_PLUG);
            registerReceiver(mVideoEditorReceiver, filter);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        mIsVisible = false;
        pause();

        // Calling this to avoid flashing preview video image when wake up if
        // vlw has set as wallpaper and suspend from preview Activity.
        // Side-effect: editor bar jumping when onResume() without onStart()
        // every time.
        mPlayer.setVisibility(View.INVISIBLE);

        // Avoid mPlayPause button has black background when the activity is gone.
        mPlayPause.setVisibility(View.GONE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mIsVisible = true;
        mPlayer.setVisibility(View.VISIBLE);

        if (mMediaController != null) {
            mMediaController.updateTrimTime();
        }

        // Avoid mPlayPause button has black background when the activity is gone.
        mPlayPause.setVisibility(View.VISIBLE);
        
        //When the sdcard unmounted and the display the progressDialog. 
        if (mProgressDialog != null) {
            pause();
        }
        // If it is playing when Activity.onPause, just let user to start it.
        // This will avoid some issue when wake up but still on lockscreen
        // update TextView displayed count
        resetInfoPanel();
        updateInfo();
    }

    /* (non-Javadoc)
     * @see android.app.Activity#onDestroy()
     */
    @Override
    protected void onDestroy() {
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
            Xlog.i(TAG, "onDestroy() unregister receiver: " + mReceiver);
        }
        // /To support Smart Book @{
        if (FeatureOption.MTK_SMARTBOOK_SUPPORT && mVideoEditorReceiver != null) {
            unregisterReceiver(mVideoEditorReceiver);
            Xlog.i(TAG, "onDestroy() unregister mVideoEditorReceiver: " + mVideoEditorReceiver);
        }
        // /@}
        // must stop playback of this video, avoid same video file access conflict
        if (isInPlaybackState()) {
            stopPlayback();
        }       
        super.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        // After onPause() return, Media is stopped and this will always get zero.
        int pos = mPlayer.getCurrentPosition();
        mCurrentPos = (pos != 0) ? pos : mCurrentPos;
        // /ALPS00744591 when player is in edit mode, set current position to start time @{
        if (mMediaController != null && mMediaController.isEditMode()) {
            Xlog.i(TAG, "Player in edit mode");
            mCurrentPos = mStartTime;
        }
        // @}
        outState.putString(VideoScene.WALLPAPER_URI, mUri.toString());
        outState.putInt(VideoScene.START_TIME, mStartTime);
        outState.putInt(VideoScene.END_TIME, mEndTime);
        outState.putInt(VideoScene.CURRENT_POSITION, mCurrentPos);
        outState.putString(VideoScene.BUCKET_ID, mBucketId);
        /// M: for s3d video
        outState.putInt(VideoScene.STEREO_TYPE, mStereoType);
        if (DEBUG) {
            Xlog.d(TAG, "onSaveInstanceState() mUri=" + mUri + ", mStartTime="
                    + mStartTime + ", mEndTime=" + mEndTime + ", mCurrentPos="
                    + mCurrentPos);
        }
    }
    
    private void restoreSavedInstance(Bundle savedInstanceState) {
        String uriString = savedInstanceState.getString(VideoScene.WALLPAPER_URI);
        if (uriString != null) {
            Uri uri = Uri.parse(uriString);
            if (uri != null && !uri.equals(mUri)) {
                mUri = uri;
                if (DEBUG) {
                    Xlog.d(TAG, "onCreate() restore saved uri=" + uri);
                }
            }
        }
        String BucketId = savedInstanceState.getString(VideoScene.BUCKET_ID);
        mBucketId = BucketId;
        if (DEBUG) {
            Xlog.d(TAG, "onCreate() restore saved BucketId=" + BucketId);
        }
        int pos = 0;
        pos = savedInstanceState.getInt(VideoScene.START_TIME);
        if (mStartTime != pos && pos != VideoScene.DEFAULT_START) {
            mStartTime = pos;
            if (DEBUG) {
                Xlog.d(TAG, "onCreate() restore saved start time=" + pos);
            }
        }
        pos = savedInstanceState.getInt(VideoScene.END_TIME);
        if (mEndTime != pos && pos != VideoScene.DEFAULT_END) {
            mEndTime = pos;
            if (DEBUG) {
                Xlog.d(TAG, "onCreate() restore saved end time=" + pos);
            }
        }
        // Video start from scratch
        mCurrentPos = mStartTime;
        /// M: for s3d video
        pos = savedInstanceState.getInt(VideoScene.STEREO_TYPE);
        if (mStereoType != pos) {
            mStereoType = pos;
            if (DEBUG) {
                Xlog.d(TAG, "onCreate() restore s3d type=" + pos);
            }
        }
    }
    
    private void saveUnmountedData() {
        mPrevBucketId = mBucketId;
        Xlog.w(TAG, "saveUnmountedData, mPrevBucketId" + mPrevBucketId);
        mPrevUri = mUri;
        mPrevStartTime = mStartTime;
        mPrevEndTime = mEndTime;
        mPrevCurrentPos = mCurrentPos;
        mHavaUnmountedData = true;
        /// M: for s3d video
        mPrevStereoType = mStereoType;
    }
    
    private void restoreMountedData() {
        mUri = mPrevUri;
        mStartTime = mPrevStartTime;
        mEndTime = mPrevEndTime;
        mCurrentPos = mPrevCurrentPos;
        /// M: for s3d video
        mStereoType = mPrevStereoType;
    }

    private void resetInfoPanel() {
        if (!mSetWallpaper.isEnabled()) {
            mSetWallpaper.setEnabled(true);
        }
        if (mBucketId != null) {
            Xlog.w(TAG, "resetInfoPanel, show folder info");
            // Detach media controller to VideoView if necessary.
            if (mMediaController != null) {
                mMediaController.setVisibility(View.GONE);
                mMediaController = null;
                mPlayer.setMediaController(mMediaController);
            }

            if (mFolderInfo == null) {
                mFolderInfo = (TextView) findViewById(R.id.folder_info);
                mFolderInfo.setVisibility(View.VISIBLE);
            }
        } else {
            Xlog.w(TAG, "resetInfoPanel, show media controller");
            if (mFolderInfo != null) {
                mFolderInfo.setVisibility(View.GONE);
                mFolderInfo = null;
            }
            // Attach media controller to VideoView if necessary.
            if (mMediaController == null) {
                mMediaController = (VLWMediaController) findViewById(R.id.media_controller);
                mMediaController.setVisibility(View.VISIBLE);
                mMediaController.setMediaPlayer(mPlayer);
                mMediaController.setAnchorView(mPlayer);
                mMediaController.addCallback(mCallback);
                mPlayer.setMediaController(mMediaController);
            }
        }
    }
    
    private void updatePausePlay(boolean isPlaying) {
        if (isPlaying) {
            mCurrentState = STATE_PLAYING;
            mTargetState = STATE_PLAYING;
            if (mPlayPause != null) {
                mPlayPause.setImageResource(R.drawable.pause);
            }
        } else {
            mCurrentState = STATE_PAUSED;
            mTargetState = STATE_PAUSED;
            if (mPlayPause != null) {
                mPlayPause.setImageResource(R.drawable.play);
            }
        }
    }
    
    private void pause() {
        if (DEBUG) {
            Xlog.d(TAG, "pause() mCurrentPos = " + mCurrentPos
                    + ",mCurrentState = " + mCurrentState + ",mTargetState = "
                    + mTargetState);
        }

        int pos = mPlayer.getCurrentPosition();
        mCurrentPos = (pos != 0) ? pos : mCurrentPos;
        if (mMediaController != null) {
            if (mCurrentState == STATE_PLAYING) {
                mTargetState = STATE_PAUSED;
                mMediaController.pause();
            }           
        } else {
            if (mCurrentState == STATE_PLAYING) {
                mTargetState = STATE_PAUSED;
                mPlayer.pause();
            }
            updatePausePlay(false);
        }
    }

    private void play() {
        if (DEBUG) {
            Xlog.d(TAG, "play() mCurrentPos = " + mCurrentPos
                    + ",mCurrentState = " + mCurrentState + ",mTargetState = "
                    + mTargetState);
        }

        mPlayer.setVisibility(View.VISIBLE);
        mPlayPause.setVisibility(View.VISIBLE);

        if (mMediaController != null) {
            //TODO: ALPS00416061, may optimize in furture.
            if (mCurrentState == STATE_PLAYING) {
                if (!mMediaController.isPlaying()) {
                    mTargetState = STATE_PLAYING;
                    mHandler.postDelayed((new Runnable() {
                        @Override
                        public void run() {
                            mMediaController.play();
                        }
                    }),1000);
                }
            } else {
                mTargetState = STATE_PLAYING;
                mMediaController.play();
            }
        } else {
            if (mCurrentState != STATE_PLAYING) {
                mTargetState = STATE_PLAYING;
                mPlayer.start();
            }
            updatePausePlay(true);
        }
    }
    
    private boolean isInPlaybackState() {
        return (mPlayer != null && mCurrentState != STATE_ERROR && mCurrentState != STATE_IDLE);
    }
    
    private void startPlayback() {
        mPlayer.setVisibility(View.INVISIBLE);
        mPlayPause.setVisibility(View.GONE);        
        
        // Use thumbnail bitmap to check invalid video.
        mUri = checkThumbnailBitmap(VideoEditor.this, mUri);
        // Switch 2d/3d surface layout if needed
        if (FeatureOption.MTK_S3D_SUPPORT) {
            setFlagsEx(false);
        }
        // Update info panel state.
        resetInfoPanel();
        if (mUri != null && mPlayer != null) {
            mIsOpening = true;
            // This must be done in UI thread
            final SurfaceHolder hd = mPlayer.getHolder();
            mPlayer.post(new Runnable() {
                public void run() {
                    mPlayer.setVideoURI(mUri);
                    Xlog.d(TAG, "VideoView set video URI: " + mUri);
                    play();
                }
            });
        }
        updateInfo();
    }
    
    private void stopPlayback() {
        if (mPlayer != null) {
            mPlayer.stopPlayback();
        }
        mCurrentState = STATE_IDLE;
        mTargetState = STATE_IDLE;
    }
    
    private String queryTitle(String bucketId) {
        Xlog.i(TAG, "queryTitle, bucketId=" + bucketId);
        String title = null;
        title = Utils.queryFolderInfo(this, bucketId);
        if (title != null) {
            if (mMode >= 0) {
                title = title + LEFT_BRACKET + (mMode + 1) + SEPARATOR
                + mUriList.size() + RIGHT_BRACKET;
            } else {
                title = title + LEFT_BRACKET + DEFAULT_MODE + SEPARATOR
                        + mUriList.size() + RIGHT_BRACKET;
            }
        }
        return title;
    }

    /**
     * Get video title from the media database.
     * 
     * @param uri
     * @return
     */
    private String queryTitle(Uri uri) {
        if (uri == null) {
            Xlog.w(TAG, "Uri is null, return null");
            return null;
        }
        String title = null;
        Cursor cursor = null;
        try {
            // Video from SDCARD.
            String[] proj = { MediaStore.Video.Media.DISPLAY_NAME };
            ContentResolver cr = this.getContentResolver();
            cursor = cr.query(uri, proj, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                int displayIndex = cursor
                        .getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME);
                title = cursor.getString(displayIndex);
            } else {
                // Video from resource.
                title = uri.getLastPathSegment();
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return title;
    }

    /**
     * Use thumbnail bitmap to check invalid video, if normal, just return the
     * uri; otherwise, return next normal one or null;
     */
    private Uri checkThumbnailBitmap(Context context, Uri uri) {
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        Bitmap bitmap = null;
        try {
            retriever.setDataSource(context, uri);
            bitmap = retriever.getFrameAtTime();
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file.
            Xlog.e(TAG, "corrupt video file ", ex);
        } catch (SecurityException ex) {
            // Assume this is a corrupt video file.
            Xlog.e(TAG, "corrupt video file ", ex);
        } catch (IllegalStateException ex) {
            // Assume this is a corrupt video file.
            Xlog.e(TAG, "corrupt video file ", ex);
        } catch (RuntimeException ex) {
            // Assume this is a corrupt video file.
            Xlog.d(TAG, "error: ", ex);
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                Xlog.d(TAG, "error when release retriver");
            }
        }
        
        boolean findInvalidVideo = false;
        if (bitmap == null) {
            Xlog.v(TAG, "thumbnail bitmap == null");
            findInvalidVideo = true;
        } else {
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            Xlog.v(TAG, "thumbnail bitmap.getWidth() = "
                    + bitmap.getWidth() + ",bitmap.getHeight()="
                    + bitmap.getHeight());
            if (width == 0 && height == 0) {
                findInvalidVideo = true;
            }
        }
        if (findInvalidVideo) {
            if (mUriInvalid == null) {
                mUriInvalid = new ArrayList<Uri>();
            }
            if (!mUriInvalid.contains(mUri)) {
                mUriInvalid.add(mUri);
            }
            Xlog.w(TAG, "thumbnail find unsuport video: " + mUri);
            // If folder mode, then play next video.
            if (mBucketId != null) {
                // Update mUriList.
                mUriList = Utils.queryUrisFromBucketId(VideoEditor.this, mBucketId);
                mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                if (mMode >= 0) {
                    mUri = mUriList.get(mMode);
                    mStereoType = Utils.queryStereoType(this, mUri);
                    return checkThumbnailBitmap(context, mUri);
                } else {
                    Xlog .w(TAG, "Error: No valid videos, the folder cann't be set as wallpaper");
                    return uri;
                }
            }
        }
        return uri;
    }

    /**
     * Get title of video from video's media metadata.
     * @param context
     * @param uri
     * @return
     */
    private String queryTitle(Context context, Uri uri) {
        if (uri == null) {
            Xlog.w(TAG, "Uri is null, return null");
            return null;
        }
        String title = null;
        if (DEBUG) {
            Xlog.i(TAG, "query Uri " + uri);
        }
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(context, uri);
            title = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
            Xlog.e(TAG, "corrupt video file ", ex);
        } catch (RuntimeException ex) {
            // Assume this is a corrupt video file
            Xlog.d(TAG, "error: ", ex);
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                Xlog.d(TAG, "error when release retriver");
            }
        }
        if (title == null) {
            title = uri.getLastPathSegment();
        }
        return title;
    }
    
    /**
     * Get state info from shared preference. if no video URI is set then use
     * the default video URI.
     */
    private void loadSettings() {
        Uri uri = Uri.parse(getResources().getString(
                R.string.default_video_path));

        if (mSharedPref == null) {
            Xlog.w(TAG, "has no SharedPreferences, use default");
            mUri = uri;
            mStartTime = VideoScene.DEFAULT_START;
            mEndTime = VideoScene.DEFAULT_END;
            mCurrentPos = VideoScene.DEFAULT_START;
            /// M: for s3d video
            mStereoType = VideoScene.DEFAULT_STEREO_TYPE;
        } else {
            mBucketId = mSharedPref.getString(VideoScene.BUCKET_ID, null);
            String uriString = mSharedPref.getString(VideoScene.WALLPAPER_URI,
                    uri.toString());
            mUri = Uri.parse(uriString);
            mStartTime = (int) mSharedPref.getLong(VideoScene.START_TIME,
                    VideoScene.DEFAULT_START);
            mEndTime = (int) mSharedPref.getLong(VideoScene.END_TIME,
                    VideoScene.DEFAULT_END);
            // Video start from scratch
            mCurrentPos = mStartTime;

            // mCurrentPos = (int) mSharedPref.getLong(
            //        VideoScene.CURRENT_POSITION, VideoScene.DEFAULT_START);
            /// M: for s3d video
            mStereoType = (int) mSharedPref.getLong(VideoScene.STEREO_TYPE, 
                    VideoScene.DEFAULT_STEREO_TYPE);
        }
        if (DEBUG) {
            Xlog.i(TAG, String.format(
                    "restore from preference, bucket id %s, Uri %s, start time %d, "
                            + "end time %d, paused position %d, stereo %d", mBucketId,
                    mUri, mStartTime, mEndTime, mCurrentPos, mStereoType));
        }
        if (mBucketId != null) {
            mUriList = Utils.queryUrisFromBucketId(this, mBucketId);
            for (int index = 0; index < mUriList.size(); index++) {
                if (mUriList.get(index).equals(mUri)) {
                    mMode = index;
                    break;
                }
            }
        }
    }

    /**
     * If this func is called, we know user change the video so we must update
     * all infos especially must reset the CURRENT_POSITION to START_TIME.
     * 
     * @hide
     */
    private void saveSettings() {
        Editor edit = mSharedPref.edit();
        edit.putString(VideoScene.BUCKET_ID, mBucketId);
        edit.putString(VideoScene.WALLPAPER_URI, mUri.toString());
        edit.putLong(VideoScene.START_TIME, (long)mStartTime);
        edit.putLong(VideoScene.END_TIME, (long)mEndTime);
        edit.putLong(VideoScene.CURRENT_POSITION, (long)mStartTime);
        /// M: for s3d video
        edit.putLong(VideoScene.STEREO_TYPE, mStereoType);
        edit.commit();

        if (DEBUG) {
            Xlog.i(TAG, String.format(
                "save settings, bucketId %s, Uri %s, start time %d, end time" +
                " %d, paused position %d, stereo %d", mBucketId, mUri, 
                mStartTime, mEndTime, mCurrentPos, mStereoType));
        }
    }
    
    private void clear(boolean clearPrefs, boolean clearBucketId, boolean clearList) {
        if (clearList) {
            if (mUriList != null) {
                mUriList.clear();
            }
            if (mUriInvalid != null) {
                mUriInvalid.clear();
            }
        }
        clear(clearPrefs, clearBucketId);
    }

    /**
     * Clear all current state info, let VideoScene take care of shared preference.
     */
    private void clear(boolean clearPrefs, boolean clearBucketId) {
        if (clearBucketId) {
            mBucketId = null;
        }

        mUri = Uri.parse(getResources().getString(R.string.default_video_path));
        mStartTime = VideoScene.DEFAULT_START;
        mEndTime = VideoScene.DEFAULT_END;
        mCurrentPos = VideoScene.DEFAULT_START;
        /// M: for s3d video
        mStereoType = VideoScene.DEFAULT_STEREO_TYPE;
        if (clearPrefs) {
            if (mSharedPref == null) {
                if (DEBUG) {
                    Xlog.e(TAG, "we lost the shared preferences");
                }
                return;
            }
            Editor edit = mSharedPref.edit();
            edit.putString(VideoScene.BUCKET_ID, mBucketId);
            edit.putString(VideoScene.WALLPAPER_URI, mUri.toString());
            edit.putLong(VideoScene.START_TIME, mStartTime);
            edit.putLong(VideoScene.END_TIME, mEndTime);
            edit.putLong(VideoScene.CURRENT_POSITION, mCurrentPos);
            /// M: for s3d video
            edit.putLong(VideoScene.STEREO_TYPE, mStereoType);
            edit.commit();
            if (DEBUG) {
                Xlog.i(TAG, "clear(), reset the default state into shared_prefs");
            }
        }
    }
    
    // TODO: THIS SHOULD HAPPEN IN AN ASYNCTASK
    private void findLiveWallpaper() {
        if (mWallpaperIntent != null) {
            ComponentName vlw = mWallpaperIntent.getComponent();
            if (vlw != null
                    && vlw.getPackageName().equals(Utils.VIDEO_LIVE_WALLPAPER_PACKAGE)
                    && vlw.getClassName().equals(Utils.VIDEO_LIVE_WALLPAPER_CLASS)) {
                return;
            }
        }
        PackageManager pkgmgr = getPackageManager();
        List<ResolveInfo> list = pkgmgr.queryIntentServices(new Intent(
                WallpaperService.SERVICE_INTERFACE),
                PackageManager.GET_META_DATA);

        int listSize = list.size();
        ResolveInfo resolveInfo = null;
        ComponentInfo ci = null;
        WallpaperInfo info = null;
        String packageName = null;
        String className = null;
        for (int i = 0; i < listSize; i++) {
            resolveInfo = list.get(i);
            ci = resolveInfo.serviceInfo;
            try {
                info = new WallpaperInfo(this, resolveInfo);
            } catch (XmlPullParserException e) {
                Xlog.w(TAG, "Skipping wallpaper " + ci, e);
                continue;
            } catch (IOException e) {
                Xlog.w(TAG, "Skipping wallpaper " + ci, e);
                continue;
            }

            packageName = info.getPackageName();
            className = info.getServiceName();
            if (packageName.equals(Utils.VIDEO_LIVE_WALLPAPER_PACKAGE)
                    && className.equals(Utils.VIDEO_LIVE_WALLPAPER_CLASS)) {
                mWallpaperIntent = new Intent(WallpaperService.SERVICE_INTERFACE);
                mWallpaperIntent.setClassName(packageName, className);
                break;
            }
        }
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (DEBUG) {
            Xlog.d(TAG, "onActivityResult request code = " + requestCode
                    + ", resultCode = " + resultCode + ",data = " + data);
        }
        if (resultCode == Activity.RESULT_OK && data != null) {
            switch (requestCode) {
            case PICK_VIDEO_REQUEST:
                if (FeatureOption.MTK_S3D_SUPPORT && Utils.isStereoVideo(this, data.getData())) {
                    launchS3dDepthTuning(data, false);
                } else {
                    onVideoSelected(data);
                }
                break;

            case PICK_FOLDER_REQUEST:
                if (FeatureOption.MTK_S3D_SUPPORT && Utils.isStereoFolder(this, data.getStringExtra("bucketId"))) {
                    launchS3dDepthTuning(data, true);
                } else {
                    onVideoFolderSelected(data, true);
                }
                break;

            case PICK_CAMERA_REQUEST:
                Uri uri = data.getData();
                String videoPath = uri != null ? Utils.getVideoPath(this, uri) 
                        : null;
                if (uri != null && videoPath != null) {
                    // Reset current state.
                    clear(false, true);
                    mUri = Uri.fromFile(new File(videoPath));
                    mStereoType = Utils.queryStereoType(this, mUri);
                    Xlog.d(TAG, "PICK_CAMERA_REQUEST, uri=" + uri + ", mUri="
                            + mUri);
                    startPlayback();
                }
                break;

            case S3D_FILE_DEPTH_TUNING_REQUEST:
                onVideoSelected(data);

            case S3D_FOLDER_DEPTH_TUNING_REQUEST:    
                onVideoFolderSelected(data, false);

            default:
                Xlog.e(TAG, "unknown request");
                break;
            }
        }
    }
    private void onVideoSelected(Intent data) {
        if (DEBUG) {
            Xlog.d(TAG, "onVideoSelected, data:"+data);
        }
        Uri uri = data.getData();
        if (uri != null) {
            Uri realUri = null;
            if (!Utils.isDefaultVideo(uri)) {
                String videoPath = Utils.getVideoPath(this, uri);
                if (videoPath != null) {
                    realUri = Uri.fromFile(new File(videoPath));
                } else {
                    realUri = uri;
                }
            } else {
                realUri = uri;
            }

            // If come here from folder selected session, change session
            // and select this single video anyway.
            if (!realUri.equals(mUri) || mBucketId != null) {
             // Reset current state.
                clear(false, true, true);
                mUri = realUri;
                mStereoType = Utils.queryStereoType(this, mUri);
                Xlog.i(TAG, "PICK_VIDEO_REQUEST, mUri=" + uri + " mUri="
                        + mUri + ", mStereoType=" + mStereoType);
                startPlayback();
            }

            updateConvergenceValue();
        }
    }

    private void onVideoFolderSelected(Intent data, boolean startFromFirst) {
        if (DEBUG) {
            Xlog.d(TAG, "onVideoFolderSelected, data:"+data+", startFromFirst:"+startFromFirst);
        }
        String bucketId = data.getStringExtra("bucketId");
        if (bucketId != null && !bucketId.equals(mBucketId)) {
            mBucketId = bucketId;
            mUriList = Utils.queryUrisFromBucketId(this, bucketId);
            Uri uri = null;
            if (!mUriList.isEmpty()) {
                if (startFromFirst) {
                    uri = mUriList.get(0);
                    // Reset this to update the title.
                    mMode = 0;
                } else {
                    if (mSharedPref != null) {
                        uri = Uri.parse(mSharedPref.getString(VideoScene.PREVIEW_WALLPAPER_URI, null));
                        if (mUriList == null) {
                            mUriList = Utils.queryUrisFromBucketId(this, mBucketId);
                        }
                        for (int index = 0; index < mUriList.size(); index++) {
                            if (mUriList.get(index).equals(uri)) {
                                mMode = index;
                                break;
                            }
                        }    
                    }
                }                
            }
            if (uri != null) {
                // Reset current state.
                clear(false, false);
                if (mUriInvalid != null) {
                    mUriInvalid.clear();
                }
                mUri = uri;
                mStereoType = Utils.queryStereoType(this, mUri);
                Xlog.i(TAG, "PICK_FOLDER_REQUEST,  " + "bucketId="
                        + bucketId + ", " + mUriList.size()
                        + " videos selected, mUri=" + mUri 
                        + ", mStereoType=" + mStereoType);
                
                startPlayback();
            }

            updateConvergenceValue();
        }
    }

    private void updateConvergenceValue() {
        //update new convergence value
        if (mPlayer != null) {
            int convergence = (int)mSharedPref.getLong(VideoScene.CONVERGENCE_VALUE, 
                VideoS3dDepthTuning.CONVERGENCE_MAX/2);
            mPlayer.setFlagsEx(convergence, Utils.FLAG_EX_S3D_CONVERGENCE);
            if (DEBUG) {
                Xlog.d(TAG, "setFlagsEx(): 0x00" + Integer.toHexString(convergence) +  ",mask: " + Integer.toHexString(Utils.FLAG_EX_S3D_CONVERGENCE) + "  [CONVERGENCE]");
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_SELECT_VIDEO) {
            return new SelectVideo().createDialog();
        }

        return super.onCreateDialog(id);
    }

    /**
     * Displays the select sdcard video dialog and launches, if necessary, the
     * appropriate activity.
     */
    private class SelectVideo implements DialogInterface.OnClickListener,
            DialogInterface.OnCancelListener,
            DialogInterface.OnDismissListener, DialogInterface.OnShowListener {

        private AddAdapter mAdapter;

        Dialog createDialog() {
            mAdapter = new AddAdapter(VideoEditor.this);

            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    VideoEditor.this);
            builder.setTitle(getString(R.string.menu_item_add_item));
            builder.setAdapter(mAdapter, this);
            builder.setInverseBackgroundForced(true);

            AlertDialog dialog = builder.create();
            dialog.setOnCancelListener(this);
            dialog.setOnDismissListener(this);
            dialog.setOnShowListener(this);

            return dialog;
        }

        public void onCancel(DialogInterface dialog) {
            cleanup();
        }

        public void onDismiss(DialogInterface dialog) {
            // TODO
        }

        private void cleanup() {
            dismissDialog(DIALOG_SELECT_VIDEO);
        }

        /**
         * Handle the action clicked in the "select video" dialog.
         */
        public void onClick(DialogInterface dialog, int which) {
            cleanup();
            
            AddAdapter.ListItem listItem = (AddAdapter.ListItem) mAdapter.getItem(which);
            Intent pickIntent = null;
            Intent wrapperIntent = null;
            switch (listItem.mActionTag) {
                case AddAdapter.ITEM_VIDEO:
                    pickIntent = new Intent(Intent.ACTION_PICK);
                    pickIntent.setType(TYPE);
                    pickIntent.putExtra(OmaDrmStore.DrmExtra.EXTRA_DRM_LEVEL,
                            OmaDrmStore.DrmExtra.DRM_LEVEL_FL);
                    wrapperIntent = Intent.createChooser(pickIntent, null);
                    startActivityForResult(wrapperIntent, PICK_VIDEO_REQUEST);
                    break;

                case AddAdapter.ITEM_FOLDER: 
                    pickIntent = new Intent(ACTION_PICK_FOLDER);
                    pickIntent.setType(TYPE);
                    pickIntent.putExtra(OmaDrmStore.DrmExtra.EXTRA_DRM_LEVEL,
                            OmaDrmStore.DrmExtra.DRM_LEVEL_FL);
                    wrapperIntent = Intent.createChooser(pickIntent, null);
                    startActivityForResult(wrapperIntent, PICK_FOLDER_REQUEST);
                    break;
                    
                default:
                    Xlog.i(TAG, "unknown item actionTag: " + listItem.mActionTag);
                    break;
            }
        }

        public void onShow(DialogInterface dialog) {
            // TODO
        }
    }
    
    // button hook
    @SuppressWarnings({ "UnusedDeclaration" })
    public void setLiveWallpaper(View v) {
        if (checkUri()) {
            Utils.showInfo(VideoEditor.this, R.string.VideoScene_error_be_set, true);
            // Stop it from being pressed so crazy.
            mSetWallpaper.setEnabled(false);
            return;
        }
        // Save state info.
        saveSettings();
        // Set live wallpaper.
        findLiveWallpaper();
        if (mWallpaperIntent == null) {
            Xlog.e(TAG, "Can not find Video Live Wallpaper package.");
            return;
        }
        // Must stop playback of this video, avoid same video file access conflict.
        stopPlayback();

        try {
            Xlog.i(TAG, "Set Video Live Wallpaper.");
            WallpaperManager wpm = WallpaperManager.getInstance(this);
            wpm.getIWallpaperManager().setWallpaperComponent(
                    mWallpaperIntent.getComponent());
            wpm.setWallpaperOffsetSteps(0.5f, 0.0f);
            wpm.setWallpaperOffsets(v.getRootView().getWindowToken(), 0.5f,
                    0.0f);
            setResult(RESULT_OK);
        } catch (RemoteException e) {
            Xlog.w(TAG, "Failure setting wallpaper", e);
        }
        finish();
        mClosed = true;
    }

    private boolean checkUri() {
        // Check invalid.
        if (mUriList != null && !mUriList.isEmpty() && mUriInvalid != null
                && mUriInvalid.size() != mUriList.size()) {
            return false;
        } else if (mUriInvalid != null && mUriInvalid.size() > 0
                && mUriInvalid.contains(mUri)) {
            return true;
        }
        return false;
    }

    private void updateInfo() {
        if (mBucketId != null) {
            // Updata displayed video count.
            mUriList = Utils.queryUrisFromBucketId(VideoEditor.this, mBucketId);
            // Update folder info any way.
            String info = Utils.queryFolderInfo(this, mBucketId);
            int count = 0;
            if (mUriList != null) {
                count = mUriList.size();
            }
            info = HTML_BOLD + count + HTML_BOLD
                    + getResources().getString(R.string.folder_info) + HTML_BOLD
                    + info + HTML_BOLD;
            final Spanned span = Html.fromHtml(info);
            // Update video title any way
            final String title = queryTitle(mBucketId);
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    if (title != null) {
                        mVideoTitle.setText(title);
                    }
                    // post this may NullPoint JE when change from folder mode to default video.
                    if (mFolderInfo != null) {
                        mFolderInfo.setText(span);
                    }
                }
            });
        } else {
            final String title = queryTitle(mUri);
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    if (title != null) {
                        mVideoTitle.setText(title);
                    }
                }
            });
        }
    }
    
    /// M: for s3d video
    private void setFlagsEx(boolean updateDatabase) {
        if (FeatureOption.MTK_S3D_SUPPORT) {
            final int flagsEx = Utils.getSurfaceStereoMode(Utils.isStereo(mStereoType))
                    | Utils.getSurfaceLayout(mStereoType);
            Xlog.i(TAG, "setFlagsEx(): 0x00" + Integer.toHexString(flagsEx));
            if (mPlayer != null) {
                mPlayer.setFlagsEx(flagsEx, Utils.FLAG_EX_S3D_MASK);
                mFlagsEx = flagsEx;
                Xlog.i(TAG, "setFlagsEx(): 0x00" + Integer.toHexString(flagsEx) + ", mask: " + Integer.toHexString(Utils.FLAG_EX_S3D_MASK));
            }
            if (updateDatabase) {
                Xlog.i(TAG, "setFlagsEx() Update MediaStore database: " 
                        + mStereoType + ", " + mUri);
                Utils.updateMediaInfoToDatabase(this, mUri, mStereoType, System.currentTimeMillis());
            }
        }
    }
    
    // Button hook.
    @SuppressWarnings({ "UnusedDeclaration" })
    public void selectVideo(View v) {
        if (!mClosed) {
            showDialog(DIALOG_SELECT_VIDEO);
        }
    }
    
    // Button hook.
    @SuppressWarnings({ "UnusedDeclaration" })
    public void selectDefaultVideo(View v) {
        Intent pickIntent = new Intent(this, VideoChooser.class);
        pickIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivityForResult(pickIntent, PICK_VIDEO_REQUEST);
    }
    
    // Button hook.
    @SuppressWarnings({ "UnusedDeclaration" })
    public void captureVideo(View v) {
        float ratio = Utils.queryResolutionRatio(this);
        Xlog.i(TAG, "To captureVideo, ratio=" + ratio);
        
        Intent pickIntent = new Intent(ACTION_VIDEO_CAPTURE);
        pickIntent.putExtra("identity", "com.mediatek.vlw");
        pickIntent.putExtra("ratio", ratio);
        Intent wrapperIntent = Intent.createChooser(pickIntent, null);
        startActivityForResult(wrapperIntent, PICK_CAMERA_REQUEST);
    }
    
    // Button hook.
    @SuppressWarnings({ "UnusedDeclaration" })
    public void updatePausePlay(View v) {
        if (mPlayer != null) {
            if (mCurrentState == STATE_PLAYING) {
                pause();
            } else {
                play();
            }
        }
    }

    /// M: for s3d video
    private void launchS3dDepthTuning(Intent data, boolean isFolder) {
        data.putExtra("isFolder", isFolder);
        VideoS3dDepthTuning.launchActivity(this, 
            isFolder ? S3D_FOLDER_DEPTH_TUNING_REQUEST : S3D_FILE_DEPTH_TUNING_REQUEST
            , data);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // do nothing. override to resolve issue caused by
        // capturing video in landscape mode, when it returns,
        // this activity will be destroyed and restart
    }

    /**
     * Used for AutoTest
     * @param runnable
     */
    public void runInUIThread(Runnable runnable) {
        mHandler.post(runnable);
    }

    // /To support Smart Book @{
    private static final String ACTION_SMARTBOOK_PLUG = "android.intent.action.SMARTBOOK_PLUG";
    private static final String EXTRA_SMARTBOOK_PLUG_STATE = "state";

    private boolean isSmartBookPluggedIn() {
        DisplayManager displayManager = (DisplayManager) this.getSystemService(Context.DISPLAY_SERVICE);
        return displayManager.isSmartBookPluggedIn();
    }
    // /@}
}
