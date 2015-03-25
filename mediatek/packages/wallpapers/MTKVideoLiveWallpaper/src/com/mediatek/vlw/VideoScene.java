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
import static android.os.FileObserver.DELETE;
import static android.os.FileObserver.DELETE_SELF;
import static android.os.FileObserver.MOVED_FROM;
import static android.os.FileObserver.MOVED_TO;
import static android.os.FileObserver.MOVE_SELF;
import android.app.WallpaperManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.hardware.display.DisplayManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.Metadata;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.WindowManager;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.vlw.Utils.LoopMode;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Map;

public class VideoScene {
    private static final String TAG = "VideoScene";
    
    static final String SHARED_PREFS_FILE = "vlw";
    static final String SMARTBOOK_SHARED_PREFS_FILE = "smartbook_vlw";
    // Video edit control.
    static final int DEFAULT_START = 0;
    // 10 min.
    static final int DEFAULT_END = 600000;
    static final String WALLPAPER_URI = "uri";
    static final String START_TIME = "start";
    static final String END_TIME = "end";
    static final String CURRENT_POSITION = "pos";
    static final String BUCKET_ID = "bucketId";
    /// M: for s3d video
    static final String PREVIEW_WALLPAPER_URI = "preview_uri";
    static final String PREVIEW_CURRENT_POSITION = "preview_pos";
    static final String PREVIEW_BUCKET_ID = "preview_bucketId";
    /// M: for s3d video
    static final String STEREO_TYPE = "stereo";
    static final String STEREO_TYPE_PREV = "stereo_prev";
    static final int DEFAULT_STEREO_TYPE = Utils.STEREO_TYPE_UNKNOWN;
    static final String CONVERGENCE_VALUE = "convergence";
    static final String STEREO_TYPE_CHANGE = "stereo_type_change";
    private boolean mStereoTypeChange;
    
    private static final boolean DEBUG = true;
    private static final int COMPLETE_PRECENT = 100;
    private static final String FILE = "file";
    private static final String SCHEMA_RTSP = "rtsp";
    private static final String SCHEMA_HTTP = "http";
    private static final String URI_END = "sdp";
    private static final String ACTION_BOOT_IPO = "android.intent.action.ACTION_BOOT_IPO";
    private static final String ACTION_PRE_SHUTDOWN = "android.intent.action.ACTION_PRE_SHUTDOWN";
    
    // Previous video info.
    private static final String WALLPAPER_URI_PREV = "uri_prev";
    private static final String START_TIME_PREV = "start_prev";
    private static final String END_TIME_PREV = "end_prev";
    private static final String CURRENT_POSITION_PREV = "pos_prev";
    private static final String BUCKET_ID_PREV = "bucketId_prev";

    // SD card mount point
    private static final String ICS_DEFAULT_SDCARD_PATH = "/mnt/sdcard/";
    private static final String ICS_EXTERNAL_SDCARD_PATH = "/mnt/sdcard2/";
    
    // All possible internal states.
    private static final int STATE_ERROR = -1;
    private static final int STATE_IDLE = 0;
    private static final int STATE_PREPARING = 1;
    private static final int STATE_PREPARED = 2;
    private static final int STATE_PLAYING = 3;
    private static final int STATE_PAUSED = 4;
    private static final int STATE_PLAYBACK_COMPLETED = 5;
    
    private static final int MSG_MONITOR_POSITION = 1;
    private static final int MSG_RELEASE_TIMER = 3;
    private static final int MSG_RELOAD_VIDEO = 4;
    private static final int MSG_CLEAR = 5;
    private static final int MSG_INVALIDVIDEO = 0;
    // Every 1 sec to check if play to end.
    private static final int TIME_OUT = 1000;
    private static final int RELOAD_TIME_OUT = 1000;
    private static final int CLEAR_TIME_OUT = 30000;
    // If 5 min has no playing, then release all media resources.
    private static final int RELEASE_TIME_OUT = 300000; 
    private static final float WALLPAPER_SCREENS_SPAN = 1.25f;
    private static final float WALLPAPER_MAX_SCALE_PERCENT = 2.0f;
    
    private static final int STREAMING_HTTP = 1;
    private static final int STREAMING_RTSP = 2;
    private static final int STREAMING_SDP = 3;
    
    private static final int MEDIA_CANGETMETADATA = 803;

    private final Context mContext;
    private final boolean mPreview;
    private int mStartTime;
    private int mEndTime;
    private int mCurrentPos;
    
    private final WallpaperManager mWallpaperManager;
    private final WindowManager mWindowManager;
    
    // Settable by the client.
    private Uri mUri;
    
    // Restore the video selected last time when sdcard is ready if needed.
    private Uri mPrevUri;
    private String mPrevBucketId;
    private int mPrevStartTime;
    private int mPrevEndTime;
    private int mPrevCurrentPos;
    
    private Map<String, String> mHeaders;
    private int mDuration;
    private SharedPreferences mSharedPref;
    
    // mCurrentState is a VideoScene object's current state.
    // mTargetState is the state that a method caller intends to reach.
    // For instance, regardless the VideoScene object's current state,
    // calling pause() intends to bring the object to a target state
    // of STATE_PAUSED.
    private int mCurrentState = STATE_IDLE;
    private int mTargetState = STATE_IDLE;

    // All the stuff we need for playing and showing a video.
    private SurfaceHolder mSurfaceHolder;
    private MediaPlayer mMediaPlayer;
    // Key and value for enable ClearMotion
    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;	
    private int mVideoWidth;
    private int mVideoHeight;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    // Original wallpaper size, restore this when exit vlw if it is changed
    private int mOriginWallpaperWidth = -1;
    private int mOriginWallpaperHeight = -1;
    
    private OnCompletionListener mOnCompletionListener;
    private OnPreparedListener mOnPreparedListener;
    private OnBufferingUpdateListener mOnBufferingUpdateListener;
    private int mCurrentBufferPercentage;
    private OnErrorListener mOnErrorListener;
    // Recording the seek position while preparing.
    private long mSeekWhenPrepared; 
 
    private boolean mCanPause;
    private boolean mCanSeekBack;
    private boolean mCanSeekForward;
    // State before calling suspend().
    private int mStateWhenSuspended; 
    
    // Judge and support special streaming type.
    private int mStreamingType = -1;
    private int mBufferWaitTimes = 0;
    private boolean mCanGetMetaData;
    private boolean mHaveGetPreparedCallBack;
    private boolean mWaitingReload;
    private boolean mHasShutdown;
    private boolean mStartFromBoot = true;
    private int mMode;
    private LoopMode mLoopMode = LoopMode.ALL;
    private ArrayList<Uri> mUriList;
    private ArrayList<Uri> mUriInvalid;
    private String mBucketId;
    private boolean mVisible;
    private ArrayList<String> mStoragesList;
    
    // Release the media resource immediately after the video file cannot be
    // accessed any more.
    private FileObserver mFileObserver;
    private String mObserverPath;
    private String mMovingFile;
    
    private int iSavedUserID;

    /// M: for s3d video
    private int mStereoType;
    private int mPrevStereoType;
    
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_MONITOR_POSITION:
                if (!mPreview) {
                    long pos = getCurrentPosition();
                    long duration = getDuration();
                    if (duration == -1) {
                        Message message = obtainMessage(MSG_MONITOR_POSITION);
                        sendMessageDelayed(message, TIME_OUT);
                    } else if (mEndTime < duration) {
                        if (pos > mEndTime) {
                            seekTo(mStartTime);
                        }
                        Message message = obtainMessage(MSG_MONITOR_POSITION);
                        sendMessageDelayed(message, TIME_OUT);
                    } else {
                        removeMessages(MSG_MONITOR_POSITION);
                    }
                }
                break;

            case MSG_RELEASE_TIMER:
                // So long as RELEASE_TIME_OUT no playing, so release all resources.
                release(false);
                removeMessages(MSG_RELEASE_TIMER);
                break;
                
            case MSG_RELOAD_VIDEO:
                String videoPath = mUri.getPath();
                boolean videoExistNot = !Utils.isExternalFileExists(videoPath);
                Xlog.d(TAG, "MSG_RELOAD_VIDEO, videoExistNot = " + videoExistNot + ", mHasShutdown = " + mHasShutdown);
                if (videoExistNot || mHasShutdown) {
                    Message message = obtainMessage(MSG_RELOAD_VIDEO);
                    sendMessageDelayed(message, RELOAD_TIME_OUT);
                    Xlog.w(TAG, "Cannot query video path, reload it in "
                            + RELOAD_TIME_OUT / 1000 + "sec");
                } else {
                    mWaitingReload = false;
                    removeMessages(MSG_RELOAD_VIDEO);
                    removeMessages(MSG_CLEAR);
                    // Seek to the right position when we leave.
                    // We should not do this in preview mode.
                    if (!mPreview) {
                        mSeekWhenPrepared = mCurrentPos;
                    }
                    openVideo();
                    if (mVisible) {
                        start();
                    }
                }
                break;
                
            case MSG_CLEAR:
                removeMessages(MSG_CLEAR);
                removeMessages(MSG_RELOAD_VIDEO);
                // Avoid delayed boot broadcast message and others.
                if (!mWaitingReload) {
                    return;
                }
                Xlog.w(TAG, "MSG_CLEAR sdcard removed, play default video");
                mWaitingReload = false;
                Utils.showInfo(mContext,
                        R.string.VideoScene_error_text_unknown, true);
                release(false);
                clear(true, true);
                openVideo();
                if (mVisible) {
                    start();
                }
                break;

            case MSG_INVALIDVIDEO:
                Xlog.d(TAG, "find the invalid video");
                handleInvalid();
                removeMessages(MSG_INVALIDVIDEO);
                break;

            default:
                Xlog.w(TAG, "unknown message " + msg.what);
                break;
            }
        }
    };

    private final BroadcastReceiver mShutdownReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "mShutdownReceiver intent = " + intent.getAction());
            if (Intent.ACTION_SHUTDOWN.equals(intent.getAction())
                    || Intent.ACTION_REBOOT.equals(intent.getAction())
                    || ACTION_PRE_SHUTDOWN.equals(intent.getAction())) {
                if (DEBUG) {
                    Xlog.i(TAG, "shutdown: ACTION_SHUTDOWN, mUri = " + mUri
                            + ", intent.getAction() == " + intent.getAction());
                }
                // IPO reboot issue. After shutdown we would not response to
                // MSG_CLEAR and MSG_VIDEO_RELOAD.
                // We may miss ACTION_IPO_BOOT.
                mHandler.removeMessages(MSG_RELOAD_VIDEO);
                mHasShutdown = true;
                // IPO issue, init() is missing.
                mStartFromBoot = true;
                
                // Save current state and try to restart video wallpaper when
                // next start up.
                saveSettings();
                release(true);
                // Must restart FileObserver if needed.
                stopAndReleaseVideoObserver();

            } else if (ACTION_BOOT_IPO.equals(intent.getAction())) {
                // IPO issue, init() is missing.
                mStartFromBoot = true;
                mHasShutdown = false;
                // ALPS00772474 Make sure to start video. 
                if (mVisible && mCurrentState == STATE_IDLE) {
                    Xlog.v(TAG, "mShutdownReceiver start video .");
                    start();
                }
                if (!Utils.isDefaultVideo(mUri) && !isInPlaybackState()) {
                    // Only when first time ACTION_MEDIA_MOUNTED that
                    // MSG_RELOAD_VIDEO will be sent.
                    if (!mHandler.hasMessages(MSG_CLEAR)) {
                        mHandler.sendEmptyMessageDelayed(MSG_CLEAR, CLEAR_TIME_OUT);
                    }
                }
            } else if (Intent.ACTION_SMARTBOOK_PLUG.equals(intent.getAction())) {
                boolean smartBookPlug = intent.getBooleanExtra(Intent.EXTRA_SMARTBOOK_PLUG_STATE, false);
                Xlog.d(TAG, "Receive EXTRA_SMARTBOOK_PLUG_STATE , Smart Book plug in: " + smartBookPlug);
                saveSettings();
                if (smartBookPlug) {
                    mSharedPref = context.getSharedPreferences(SMARTBOOK_SHARED_PREFS_FILE, Context.MODE_PRIVATE);
                } else {
                    mSharedPref = context.getSharedPreferences(SHARED_PREFS_FILE, Context.MODE_PRIVATE);
                }
                Xlog.d(TAG, "smart book swithing done!!!");
            }
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        // When sdcard is removed, play the default video.
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
            Xlog.i(TAG, "Receive intent action=" + action + " path=" + path);
            if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                // if Mota update from ICS to JB, correct the video URI.
                correctUriIfNeeded(path);
                if (mStoragesList == null) {
                    mStoragesList = new ArrayList<String>(2);
                }
                if (path != null && !mStoragesList.contains(path)) {
                    mStoragesList.add(path);
                }

                if ((mMediaPlayer == null || !mMediaPlayer.isPlaying())
                        && mStartFromBoot && path != null
                        && mUri.getPath().contains(path)) {
                    // Only when first time ACTION_MEDIA_MOUNTED that
                    // MSG_RELOAD_VIDEO will be sent.
                    if (!Utils.isDefaultVideo(mUri)) {
                        if (!mHandler.hasMessages(MSG_RELOAD_VIDEO)) {
                            mHandler.sendEmptyMessage(MSG_RELOAD_VIDEO);
                        }
                        mWaitingReload = true;
                    }

                } else if (mPrevUri != null) {
                    String videoPath = mPrevUri.getPath();
                    String swapPath = Utils.swapSdcardUri(mPrevUri).getPath();
                    
                    // TODO: need to translate the BucketId when sdcard-swap happen
                    if (mPrevBucketId != null) {
                        String bucketPath = videoPath.substring(0,videoPath.lastIndexOf("/"));
                        String swapBucketPath = swapPath.substring(0,swapPath.lastIndexOf("/"));
                        if (!Utils.isExternalFileExists(bucketPath) && 
                                !Utils.isExternalFileExists(swapBucketPath)) {
                            Xlog.w(TAG, "The video belonging sdcard unmounted");
                            return;
                        }
                        mBucketId = mPrevBucketId;
                        if (Utils.isExternalFileExists(videoPath)) {
                            // MediaScanner is scanning now, cannot query videos
                            // just play mPrevUri and query it again later
                            mUri = mPrevUri;
                            mStartTime = mPrevStartTime;
                            mEndTime = mPrevEndTime;
                            mCurrentPos = mPrevCurrentPos;
                            /// M: for s3d video
                            mStereoType = mPrevStereoType;
                        } else if (Utils.isExternalFileExists(swapPath)) {
                            // Needs to waiting for MEDIA_SCAN_FINISHED and 
                            // query the new bucketId again
                            //TODO: Maybe the next video queried from current
                            // bucketId is a wrong video or null
                            mUri = Uri.fromFile(new File(swapPath));
                            mStartTime = mPrevStartTime;
                            mEndTime = mPrevEndTime;
                            mCurrentPos = mPrevCurrentPos;
                            mStereoType = mPrevStereoType;
                        } else {
                            // Cannot query videos from mPrevBucketId and
                            // mPrevUri doesn't exist, cannot wait
                            Xlog.w(TAG,
                                    "cannot reload videos selected last time");
                            return;
                        }

                    } else if (Utils.isExternalFileExists(videoPath)) {
                        mUri = mPrevUri;
                        mBucketId = null;
                        mStartTime = mPrevStartTime;
                        mEndTime = mPrevEndTime;
                        mCurrentPos = mPrevCurrentPos;
                        mStereoType = mPrevStereoType;
                    } else if (Utils.isExternalFileExists(swapPath)) {
                        mUri = Uri.fromFile(new File(swapPath));
                        mBucketId = null;
                        mStartTime = mPrevStartTime;
                        mEndTime = mPrevEndTime;
                        mCurrentPos = mPrevCurrentPos;
                        mStereoType = mPrevStereoType;
                    } else {
                        Xlog.w(TAG, 
                                "video file selected last time does not exists");
                        return;
                    }
                    Xlog.i(TAG, "Restore the video last time. mPrevUri="
                            + mPrevUri + " mPrevBucketId=" + mPrevBucketId
                            + " mPrevStartTime=" + mPrevStartTime
                            + " mPrevEndTime=" + mPrevEndTime
                            + " mPrevCurrentPos=" + mPrevCurrentPos);
                    // Clean up and save current state.
                    mPrevUri = null;
                    mPrevBucketId = null;
                    Editor edit = mSharedPref.edit();
                    edit.putString(BUCKET_ID, mBucketId);
                    edit.putString(WALLPAPER_URI, mUri.toString());
                    edit.putLong(CURRENT_POSITION, mCurrentPos);
                    edit.putLong(STEREO_TYPE, mStereoType);
                    if (mBucketId == null) {
                        edit.putLong(START_TIME, mStartTime);
                        edit.putLong(END_TIME, mEndTime);
                    }
                    edit.commit();
                    // Seek to the right position when we leave.
                    // We should not do this in preview mode.
                    if (!mPreview) {
                        mSeekWhenPrepared = mCurrentPos;
                    }
                    openVideo();
                    // when sdcard being mounted, the status mHasShutdown should
                    // be false.
                    mHasShutdown = false;
                }

            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)
                    || Intent.ACTION_MEDIA_BAD_REMOVAL.equals(action)
                    || Intent.ACTION_MEDIA_REMOVED.equals(action)
                    || Intent.ACTION_MEDIA_EJECT.equals(action)) {
                if (mHasShutdown) {
                    Xlog.i(TAG, "Has been shutdown, Ignore");
                    return;
                }
                if (mStoragesList != null && path != null
                        && mStoragesList.contains(path)) {
                    mStoragesList.remove(path);
                }
                String videoPath = mUri.getPath();
                if (videoPath != null && path != null
                        && videoPath.contains(path)) {
                    Xlog.w(TAG, "action: " + action
                                    + " revert to default video. sdcard path: "
                                    + path + " absolute path: " + videoPath
                                    + " mUri: " + mUri);
                    Utils.showInfo(mContext,
                            R.string.VideoScene_error_sdcard_unmounted, true);
                    // Save current sdcard video state, restore it when sdcard
                    // is ready if needed.
                    mPrevBucketId = mBucketId;
                    mPrevUri = mUri;
                    mPrevStartTime = mStartTime;
                    mPrevEndTime = mEndTime;
                    mPrevCurrentPos = getCurrentPosition();
                    mPrevStereoType = mStereoType;

                    stopAndReleaseVideoObserver();
                    release(false);
                    clear(true, true);
                    openVideo();
                    if (mVisible) {
                        start();
                    }
                }
            } else if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                String videoPath = mUri.getPath();
                if (videoPath != null && path != null && videoPath.contains(path)) {
                    if (mBucketId != null) {
                        correctBucketIdIfNeeded();
                    }
                    /// M: for s3d video
                    final int stereo = Utils.queryStereoType(mContext, mUri);
                    if (stereo != mStereoType) {
                        mStereoType = stereo;
                        setFlagsEx(false);
                        Editor edit = mSharedPref.edit();
                        edit.putLong(STEREO_TYPE, mStereoType);
                        edit.commit();
                    }
                }
            }
        }
    };
    
    MediaPlayer.OnVideoSizeChangedListener mSizeChangedListener = new MediaPlayer.OnVideoSizeChangedListener() {
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
            if (width != 0 && height != 0) {
                if (DEBUG) {
                    Xlog.i(TAG, "OnVideoSizeChangedListener, width=" + width
                            + ",height=" + height + "," + "mVideoWidth="
                            + mVideoWidth + ",mVideoHeight=" + mVideoHeight);
                }
                if (mVideoWidth != width || mVideoHeight != height) {
                    Xlog.d(TAG, "Video size changed (" + mVideoWidth + "/"
                            + mVideoHeight + ")->(" + width + "/" + height
                            + "), relayout surface");
                    mVideoWidth = width;
                    mVideoHeight = height;
                    relayout(width, height);
                }
            }
        }
    };

    MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mp) {
            mHaveGetPreparedCallBack = true;
            if (mCanGetMetaData) {
                // If can get metadata, do it.
                doPrepared(mp);
            }
            if (DEBUG) {
                Xlog.i(TAG, "onPrepared, can get metadata:" + String.valueOf(mCanGetMetaData));
            }
        }
    };

    private final MediaPlayer.OnInfoListener mInfoListener = new MediaPlayer.OnInfoListener() {
        public boolean onInfo(MediaPlayer mp, int what, int extra) {
            Xlog.v(TAG, "OnInfoListener,  what: " + what + ",extra: " + extra + ",mStereoType: " + mStereoType);
            if (FeatureOption.MTK_S3D_SUPPORT) {
                if (what == Utils.MEDIA_INFO_3D) {
                    Editor editor = mSharedPref.edit();
                    editor.putBoolean(STEREO_TYPE_CHANGE, !mStereoTypeChange);  
                    if (mStereoType != extra && mStereoType == Utils.STEREO_TYPE_UNKNOWN) {
                        mStereoType = extra;
                        setFlagsEx(true);                        
                        editor.putLong(STEREO_TYPE, mStereoType);
                    }
                    editor.commit();
                }
            }
            if (what == MEDIA_CANGETMETADATA) {
                mCanGetMetaData = true;
                if (mHaveGetPreparedCallBack) {
                    doPrepared(mp);
                }
                return true;
            } else if (what == MediaPlayer.MEDIA_INFO_VIDEO_NOT_SUPPORTED) {
                mHandler.sendEmptyMessage(MSG_INVALIDVIDEO);
                Xlog.v(TAG,
                        "OnInfoListener found MEDIA_INFO_VIDEO_NOT_SUPPORTED");
                if (mTargetState == STATE_PLAYING) {
                    start();
                }
                return true;
            }
            return false;
        }
    };
   
    private final MediaPlayer.OnCompletionListener mCompletionListener = new MediaPlayer.OnCompletionListener() {
        public void onCompletion(MediaPlayer mp) {
            mCurrentState = STATE_PLAYBACK_COMPLETED;
            mTargetState = STATE_PLAYBACK_COMPLETED;
            if (mOnCompletionListener != null) {
                mOnCompletionListener.onCompletion(mMediaPlayer);
            }
            
            if (DEBUG) {
                Xlog.d(TAG, "onCompletion() mLoopMode=" + mLoopMode + " mBucketId=" + mBucketId +
                        ", mUri=" + mUri + ", mMode=" + mMode);
            }
            if (mBucketId != null) {
                mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
                // Reload uris again
                if (mUriList != null && mUriList.isEmpty()) {
                    correctBucketIdIfNeeded();
                    mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
                    Xlog.d(TAG, "queryUrisFromBucketId: " + mUriList.size() + " videos");
                }
                if (mUriList.size() > 1) {
                    Uri lastUri = mUri;
                 // clear all state info except for mBucketId
                    clear(false, false);
                    mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                    if (mMode >= 0) {
                        Uri nextUri = mUriList.get(mMode);
                        if (nextUri.equals(lastUri)) {
                            mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                            nextUri = mUriList.get(mMode);
                        }
                        mUri = nextUri;
                        mStereoType = Utils.queryStereoType(mContext, mUri);
                    } else {
                        clear(true, true);
                        mMode = 0;
                    }
                    // Must do this for start(), it assume
                    // mUri in SharedPref can be changed only by user
                    Editor editor = mSharedPref.edit();
                    editor.putString(mPreview ? PREVIEW_WALLPAPER_URI : WALLPAPER_URI, mUri.toString());
                    editor.putLong(STEREO_TYPE, mStereoType);
                    editor.putLong(CURRENT_POSITION, (long) DEFAULT_START);
                    editor.commit();
                    // Make sure MediaPlayerService has disconnected from SurfaceTexture
                    release(false);
                    startPlayback();
                } else {
                    // loop mode
                    seekTo(mStartTime);
                    start();
                }
            } else {
                // loop mode
                seekTo(mStartTime);
                start();
            }
        }
    };

    // Note: mErrorListener is default and mOnErrorListener is one given by user
    // to handle what they want.
    private final MediaPlayer.OnErrorListener mErrorListener = new MediaPlayer.OnErrorListener() {
        public boolean onError(MediaPlayer mp, int framworkErr, int implErr) {
            Xlog.e(TAG, "Error: " + framworkErr + "," + implErr);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;

            /* If an error handler has been supplied, use it and finish. */
            if (mOnErrorListener != null) {
                if (mOnErrorListener.onError(mMediaPlayer, framworkErr,
                        implErr)) {
                    return true;
                }
            }

            int messageId = 0;
            if (framworkErr == MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
                messageId = R.string.VideoScene_error_text_invalid_progressive_playback;
            } else {
                messageId = R.string.VideoScene_error_text_unknown;
            }
            if (mContext == null) {
                Xlog.e(TAG, "mContext is null");
                return true;
            }
            if (!mStartFromBoot && !mWaitingReload) {
                Utils.showInfo(mContext, messageId, true);
            }
            if (Utils.isDefaultVideo(mUri)) {
                Utils.showInfo(mContext, R.string.VideoScene_error_text_unknown, true);
                mHandler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            release(true);
                            openVideo();
                            if (mVisible) {
                                start();
                            }
                        }
                }, RELOAD_TIME_OUT);
                return true;
        }

            boolean hasSDCard = checkMediaState();
            if (DEBUG) {
                Xlog.i(TAG, "mStartFromBoot=" + mStartFromBoot + ", has sdcard: "
                        + hasSDCard + ", mHasShutdown=" + mHasShutdown + ", mUri="
                        + mUri + ", mBucketId=" + mBucketId);
            }

            if ((mStartFromBoot && hasSDCard) || mHasShutdown) {
                release(false);
                if (!mWaitingReload) {
                    Utils.showInfo(mContext,
                            R.string.VideoScene_reload_after_mount, true);
                    Xlog.w(TAG, 
                            "Start from boot and has sdcard, wait for its preparing");
                    mWaitingReload = true;
                }
                return true;

            } else if (mBucketId != null && hasSDCard) {
                // Update mUriList.
                mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
                if (mUriList.isEmpty()) {
                    Xlog.w(TAG, "Invalid video folder, play the default video");
                    release(false);
                    clear(true, true);
                    openVideo();
                    start();
                } else {
                    if (mUriInvalid == null) {
                        mUriInvalid = new ArrayList<Uri>();
                    }
                    // This make the new invalid Uri as the last item.
                    if (mUri != null && !mUriInvalid.contains(mUri)) {
                        mUriInvalid.add(mUri);
                    }
                    Xlog.w(TAG, 
                            "Video playing is removed or invalid in selected folder, play another video");
                    release(false);
                    mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
                    if (mMode >= 0) {
                        mUri = mUriList.get(mMode);
                        mStereoType = Utils.queryStereoType(mContext, mUri);
                    } else {
                        Xlog.w(TAG,
                                "No valid video in this folder, play default video, size="
                                        + mUriList.size());
                        clear(true, true);
                        mMode = 0;
                    }
                    // Save in SharedPref so that play video correctly if user
                    // delete the playing video then reboot device by taking
                    // out battery.
                    saveSettings();
                    openVideo();
                    start();
                }
            } else {
                Xlog.w(TAG, "media file doesn't exist, play the default video");
                release(false);
                clear(true, true);
                openVideo();
                start();
            }
            return true;
        }
    };

    private final MediaPlayer.OnBufferingUpdateListener mBufferingUpdateListener = 
        new MediaPlayer.OnBufferingUpdateListener() {
        public void onBufferingUpdate(MediaPlayer mp, int percent) {
            mCurrentBufferPercentage = percent;
            if (DEBUG) {
                Xlog.i(TAG, "Buffering percent: " + percent + "mBufferWaitTimes: " + mBufferWaitTimes);
            }
            // Here wait for the correct status to update the UI.
            if (mBufferWaitTimes > 0) {
                if (percent == COMPLETE_PRECENT) {
                    mBufferWaitTimes -= 1;
                    if (DEBUG) {
                        Xlog.i(TAG, "mBufferWaitTimes: " + mBufferWaitTimes); 
                    }
                }
                return;
            }
            updateBufferState(percent);
            if (mOnBufferingUpdateListener != null) {
                mOnBufferingUpdateListener.onBufferingUpdate(mp, percent);
            }
        }
    };

    public VideoScene(Context context, SurfaceHolder holder, boolean isPreview) {
        mContext = context;
        iSavedUserID = ActivityManager.getCurrentUser();
        if (isSmartBookPluggedIn()) {
            Xlog.i(TAG, "VideoScene(),Smart book is plug in");
            mSharedPref = context.getSharedPreferences(SMARTBOOK_SHARED_PREFS_FILE, Context.MODE_PRIVATE);
        } else {
            mSharedPref = context.getSharedPreferences(SHARED_PREFS_FILE  + iSavedUserID,
                    Context.MODE_PRIVATE);
        }
        mSurfaceHolder = holder;
        mPreview = isPreview;
        mCurrentState = STATE_IDLE;
        mTargetState = STATE_IDLE;
        mWallpaperManager = (WallpaperManager) context
                .getSystemService(Context.WALLPAPER_SERVICE);
        mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        if (mOriginWallpaperWidth <= 0 || mOriginWallpaperHeight <= 0) {
            Display disp = mWindowManager.getDefaultDisplay();
            int dispW = disp.getWidth();
            int dispH = disp.getHeight();

            if (dispH > dispW) {
                mOriginWallpaperWidth = dispW * 2;
                mOriginWallpaperHeight = dispH;
            } else {
                mOriginWallpaperWidth = dispH * 2;
                mOriginWallpaperHeight = dispW;
            }
        }
        if (DEBUG) {
            Xlog.i(TAG, "===> Original wallpaper width=" + mOriginWallpaperWidth + ", height=" + mOriginWallpaperHeight
                    + ", isPreview= " + isPreview + ", iSavedUserID= " + iSavedUserID);
        }
    }

    // When MEDIA_MOUNTED, support SD card mount point changing when MOTA update from ICS to JB.
    private void correctUriIfNeeded(String path) {
        String swapPath = null;
        final String uriPath = mUri.getPath();
        final String defaultSdCardPath = Environment.getExternalStorageDirectory().getPath() + "/";
        if (uriPath.contains(ICS_EXTERNAL_SDCARD_PATH) && path != null && !path.equals(defaultSdCardPath)) {
            swapPath = uriPath.replace(ICS_EXTERNAL_SDCARD_PATH, path);
        } else if (uriPath.contains(ICS_DEFAULT_SDCARD_PATH) && path.equals(defaultSdCardPath)) {
            swapPath = uriPath.replace(ICS_DEFAULT_SDCARD_PATH, path);
        }
        if (swapPath != null && !swapPath.equals(uriPath)) {
            Xlog.w(TAG, "Correct invalid video path " + uriPath + " --> " + swapPath);
            mUri = Uri.fromFile(new File(swapPath));
            mStereoType = Utils.queryStereoType(mContext, mUri);
            Editor editor = mSharedPref.edit();
            editor.putString(WALLPAPER_URI, mUri.toString());
            editor.putLong(STEREO_TYPE, mStereoType);
            editor.commit();
        }
    }

    /**
     * Check if need the bucketId is valid, if not correct it
     * @return false if current playing is default video or external file does
     * not exist; true otherwise, we should wait for SCAN_FINISHED sometimes.
     */
    private boolean correctBucketIdIfNeeded() {
        if (mBucketId == null || Utils.isDefaultVideo(mUri) 
                || !Utils.isExternalFileExists(mUri.getPath())) {
            return false;
        }
        final String bucketId = Utils.queryBucketId(mContext, mUri.getPath());
        if (bucketId != null && !bucketId.equals(mBucketId)) {
            Xlog.w(TAG, "Correct invalid bucketId " + mBucketId + " --> " + bucketId);
            mBucketId = bucketId;
            Editor editor = mSharedPref.edit();
            editor.putString(BUCKET_ID, mBucketId);
            editor.commit();
        }
        return true;
    }
    
    public void handleInvalid() {
        mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
        if (mUriList == null || mUriList.isEmpty()) {
            Xlog.i(TAG, "invalid video folder, play the default video");
            release(false);
            clear(true, true);
            openVideo();
            if (mVisible) {
                start();
            }
        } else {
            if (mUriInvalid == null) {
                mUriInvalid = new ArrayList<Uri>();
            }
            // This make the new invalid Uri as the last item.
            if (mUri != null && !mUriInvalid.contains(mUri)) {
                mUriInvalid.add(mUri);
            }

            Xlog.i(TAG, "video playing is removed or invalid in selected folder, play another video");
            release(false);
            mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
            if (mMode >= 0) {
                mUri = mUriList.get(mMode);
                mStereoType = Utils.queryStereoType(mContext, mUri);
            } else {
                Xlog.d(TAG, "No valid video in this folder, play default video, size=" + mUriList.size());
                clear(true, true);
                mMode = 0;
            }
            // Save in SharedPref so that play video correctly if user
            // delete the playing video then reboot device by taking
            // out battery.
            saveSettings();
            openVideo();
        }
    }
    
    private void relayout(int videoWidth, int videoHeight) {
        Display disp = mWindowManager.getDefaultDisplay();
        final int dispW = disp.getWidth();
        final int dispH = disp.getHeight();
        int wpW = mOriginWallpaperWidth;
        int wpH = mOriginWallpaperHeight;
        final int curWpW = mWallpaperManager.getDesiredMinimumWidth();
        final int curWpH = mWallpaperManager.getDesiredMinimumHeight();
        if (DEBUG) {
            Xlog.d(TAG, String.format(
                    "relayout, display: (%d,%d), video: (%d,%d), "
                            + "previous wallpaper: (%d,%d)", dispW, dispH,
                    videoWidth, videoHeight, curWpW, curWpH));
        }

        // Just use the real suggested wallpaper surface size others wanted(Launcher).
        int surfaceW = videoWidth;
        int surfaceH = videoHeight;
        float scaleW = curWpW / (float) videoWidth;
        float scaleH = curWpH / (float) videoHeight;
        float scale = VideoChooser.DEFAULT_SCALE;
        boolean relayoutSurface = false;
        boolean relayoutWallpaper = false;

        // Need to resize surface if video size is not the same with wallpaper
        // or need to reset previous value to fit current video size.
        if ((curWpW != videoWidth || curWpH != videoHeight)
                || (videoWidth != mSurfaceWidth || videoHeight != mSurfaceHeight)) {
            relayoutSurface = true;
            if (scaleW > scaleH) {
                scale = scaleW;
            } else {
                scale = scaleH;
            }
        }

        // Must preserve the video width/height ratio.
        surfaceW *= scale;
        surfaceH *= scale;

        float wr = (surfaceW - curWpW) / (float) curWpW;
        float hr = (surfaceH - curWpH) / (float) curWpH;
        float percent = hr > wr ? hr : wr;
        if (percent < WALLPAPER_MAX_SCALE_PERCENT / 2) {
            if (DEBUG) {
                Xlog.d(TAG, "just scale video to fit wallpaper, percent="
                        + percent + ", scale=" + scale);
            }
        } else if (percent < WALLPAPER_MAX_SCALE_PERCENT) {
            if (DEBUG) {
                Xlog.d(TAG, "big difference percent: " + percent + ", scale="
                        + scale + ", reset wallpaper width to dispW * "
                        + WALLPAPER_SCREENS_SPAN);
            }            
            wpW = (int) (dispW * WALLPAPER_SCREENS_SPAN);
            scaleW = wpW / (float) videoWidth;
            // Need to resize surface if video size is not the same with
            // wallpaper or need to reset previous value to fit current video size
            if ((wpW != videoWidth || wpH != videoHeight)
                    || (videoWidth != mSurfaceWidth || videoHeight != mSurfaceHeight)) {
                relayoutSurface = true;
                if (scaleW > scaleH) {
                    scale = scaleW;
                } else {
                    scale = scaleH;
                }
            }
            relayoutWallpaper = true;

        } else {
            Xlog.e(TAG, "TODO: need to rotate the video, percent=" + percent
                    + ", scale=" + scale);
            scale = 1.0f;
        }
        surfaceW = videoWidth;
        surfaceH = videoHeight;
        surfaceW *= scale;
        surfaceH *= scale;
        if (relayoutSurface) {
            if (DEBUG) {
                Xlog.d(TAG, String.format("resize surface: (%d,%d)-->(%d,%d)",
                        mSurfaceWidth, mSurfaceHeight, surfaceW, surfaceH));
            }
            mSurfaceWidth = surfaceW;
            mSurfaceHeight = surfaceH;
            mSurfaceHolder.setFixedSize(surfaceW, surfaceH);
        }
    }

    private boolean checkMediaState() {
        boolean res = false;
        //String state = Environment.getExternalStorageState();
        StorageManager storageManager = (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        //String path = StorageManagerEx.getExternalStoragePath();
        //String state = storageManager.getVolumeState(path);
        //Xlog.i(TAG, "check external sdcard path: " + path);

        String state = null;
        String[] storagePathList = storageManager.getVolumePaths();
        for (String path : storagePathList) {
            Xlog.i(TAG, "check sdcard path: " + path);
            if (mUri.getPath().contains(path)) {
                state = storageManager.getVolumeState(path);
            }
        }
        if (Environment.MEDIA_REMOVED.equals(state)
                || Environment.MEDIA_BAD_REMOVAL.equals(state)
                || Environment.MEDIA_UNMOUNTABLE.equals(state)
                || Environment.MEDIA_NOFS.equals(state)) {
            // No sdcard, so set default video
            if (DEBUG) {
                Xlog.i(TAG, "check sdcard state: " + state);
            }
            res = false;
        } else if (Environment.MEDIA_UNMOUNTED.equals(state)
                || Environment.MEDIA_CHECKING.equals(state)
                || Environment.MEDIA_MOUNTED.equals(state)
                || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)
                || Environment.MEDIA_SHARED.equals(state)) {
            // Has sdcard, but has not ready and mounted now, so wait for it
            if (DEBUG) {
                Xlog.i(TAG, "check sdcard state: " + state);
            }
            res = true;
        } else {
            Xlog.w(TAG, "check sdcard state, uncaught sdcard state: " + state);
            res = false;
        }        
        return res;
    }

    public void setVisibility(boolean visible) {
        // Just do this when the visibility is really changed, because SCREEN_ON/OFF
        // will be also converted to onVisibilityChanged() by WMS for WallpaperService.
        if (DEBUG) {
            Xlog.d(TAG, "setVisibility(" + visible + ")");
        }
        if (mVisible != visible) {
            mVisible = visible;
        }
    }

    private void loadSettings() {
        Uri uri = Uri.parse(mContext.getResources().getString(
                R.string.default_video_path));
        if (mSharedPref == null) {
            mUri = uri;
            mStartTime = DEFAULT_START;
            mEndTime = DEFAULT_END;
            mCurrentPos = DEFAULT_START;
            /// M: for s3d video
            mStereoType = DEFAULT_STEREO_TYPE;
        } else {
            mBucketId = mSharedPref.getString(mPreview ? PREVIEW_BUCKET_ID : BUCKET_ID, null);
            String uriString = mSharedPref.getString(mPreview ? PREVIEW_WALLPAPER_URI : WALLPAPER_URI, uri
                    .toString());
            mUri = Uri.parse(uriString);
            mStartTime = (int) mSharedPref.getLong(START_TIME, DEFAULT_START);
            mEndTime = (int) mSharedPref.getLong(END_TIME, DEFAULT_END);
            mCurrentPos = (int) mSharedPref.getLong(CURRENT_POSITION, DEFAULT_START);

            // load previous info
            mPrevBucketId = mSharedPref.getString(BUCKET_ID_PREV, null);
            String prevUriString = mSharedPref.getString(WALLPAPER_URI_PREV, null);
            if (prevUriString != null) {
                mPrevUri = Uri.parse(prevUriString);
            } else {
                mPrevUri = null;
            }
            mPrevStartTime = (int) mSharedPref.getLong(START_TIME_PREV, DEFAULT_START);
            mPrevEndTime = (int) mSharedPref.getLong(END_TIME_PREV, DEFAULT_END);
            mPrevCurrentPos = (int) mSharedPref.getLong(CURRENT_POSITION_PREV,
                    DEFAULT_START);
            /// M: for s3d video
            mStereoType = (int) mSharedPref.getLong(STEREO_TYPE, DEFAULT_STEREO_TYPE);
            if (mPreview) {
                mStereoType = Utils.queryStereoType(mContext, mUri);
                Xlog.i(TAG, "This is preview mode, mStereoType: " + mStereoType);
            }
            mPrevStereoType = (int) mSharedPref.getLong(STEREO_TYPE_PREV, DEFAULT_STEREO_TYPE);
        }
        if (DEBUG) {
            Xlog.i(TAG, String.format("Restore shared_prefs, mStartTime=%d,"
                    + " mEndTime=%d, mCurrentPos=%d, mBucketId=%s, mUri=%s,"
                    + " mStereoType=%d", mStartTime, mEndTime, mCurrentPos, 
                    mBucketId, mUri, mStereoType));
        }

        updateVideoIndex();
    }

    void updateVideoIndex() {
        if (mBucketId != null) {
            // If come here from boot start, this will fail because sdcard is
            // not ready now. Should reload uris again somewhere!
            mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
            for (int index = 0; index < mUriList.size(); index++) {
                if (mUriList.get(index).equals(mUri)) {
                    mMode = index;
                    return;
                }
            }
        }
    }

    /**
     * Clear all current state and the shared preference to default.
     * Called when media file is removed or user set static wallpaper.
     * @hide
     */
    private void clear(boolean clearPrefs, boolean clearBucketId) {
        if (clearBucketId) {
            mBucketId = null;
            if (mUriList != null) {
                mUriList.clear();
            }
            if (mUriInvalid != null) {
                mUriInvalid.clear();
            }
        }
        mUri = Uri.parse(mContext.getResources().getString(
                R.string.default_video_path));
        mStartTime = DEFAULT_START;
        mEndTime = DEFAULT_END;
        mCurrentPos = DEFAULT_START;
        /// M: for s3d video
        mStereoType = DEFAULT_STEREO_TYPE;
        if (clearPrefs) {
            if (mSharedPref == null) {
                if (DEBUG) {
                    Xlog.e(TAG, "we lost the shared preferences");
                }
                return;
            }
            Editor edit = mSharedPref.edit();
            edit.putString(BUCKET_ID, mBucketId);
            edit.putString(WALLPAPER_URI, mUri.toString());
            edit.putLong(START_TIME, (long) mStartTime);
            edit.putLong(END_TIME, (long) mEndTime);
            edit.putLong(CURRENT_POSITION, (long) mCurrentPos);

            // Save previous info.
            edit.putString(BUCKET_ID_PREV, mPrevBucketId);
            edit.putString(WALLPAPER_URI_PREV, mPrevUri != null ? 
                    mPrevUri.toString() : null);
            edit.putLong(START_TIME_PREV, (long) mPrevStartTime);
            edit.putLong(END_TIME_PREV, (long) mPrevEndTime);
            edit.putLong(CURRENT_POSITION_PREV, mPrevCurrentPos);
            /// M: for s3d video
            edit.putLong(STEREO_TYPE, mStereoType);
            edit.putLong(STEREO_TYPE_PREV, mPrevStereoType);
            
            edit.commit();
            if (DEBUG) {
                Xlog.i(TAG,
                        "clean(), reset the default state into shared_prefs");
            }
        }
    }
    
    private void saveSettings() {
        Editor edit = mSharedPref.edit();
        mCurrentPos = getCurrentPosition();
        edit.putString(mPreview ? PREVIEW_WALLPAPER_URI : WALLPAPER_URI, mUri != null ? 
                mUri.toString() : null);
        edit.putString(WALLPAPER_URI_PREV, mPrevUri != null ? 
                mPrevUri.toString() : null);
        edit.putLong(CURRENT_POSITION, (long) mCurrentPos);
        edit.putLong(CURRENT_POSITION_PREV, (long) mPrevCurrentPos);
        /// M: for s3d video
        edit.putLong(STEREO_TYPE, mStereoType);
        edit.putLong(STEREO_TYPE_PREV, mPrevStereoType);
        
        edit.commit();
        if (DEBUG) {
            Xlog.i(TAG, String.format(
                    "save mCurrentPos=%d, mUri=%s, mPrevUri=%s, mStereoType=%d", 
                    mCurrentPos, mUri, mPrevUri, mStereoType));
        }
    }

    public void setVideoPath(String path) {
        setVideoURI(Uri.parse(path));
    }

    public void setVideoURI(Uri uri) {
        setVideoURI(uri, null);
    }

    /**
     * @hide
     */
    public void setVideoURI(Uri uri, Map<String, String> headers) {
        mUri = uri;
        mHeaders = headers;
        mSeekWhenPrepared = 0;
        openVideo();
    }

    public void startPlayback() {
        // Seek to the right position when we leave.
        // We should not do this in preview mode.
        if (!mPreview) {
            mSeekWhenPrepared = mCurrentPos;
        }
        openVideo();
        // Set mTargetState = STATE_PLAYING to play the video when it prepared.
        start();
        // If user set end-time, we should stop at that time and restart again
        // from start-time.
        // No need to monitor folder videos.
        if (mBucketId == null) {
            mHandler.sendEmptyMessage(MSG_MONITOR_POSITION);
        }
    }

    public void stopPlayback() {
        // Remember the current position when we leave.
        // We should not do this in preview mode.
        if (!mPreview) {
            saveSettings();
        }
        // Release all resources.
        release(false);
        if (mBucketId == null) {
            mHandler.removeMessages(MSG_MONITOR_POSITION);
        }
    }

    private void judgeStreamingType() {
        if (mUri == null) {         
                Xlog.w(TAG, "mUri is null, cannot judge streaming type.");
            return;
        }
        if (DEBUG) {
            Xlog.i(TAG, "judgeStreamingType, mUri=" + mUri);
        }
        String scheme = mUri.getScheme();
        mCanGetMetaData = false;
        if (SCHEMA_RTSP.equalsIgnoreCase(scheme)) {
            if (mUri.toString().toLowerCase(Locale.ENGLISH).endsWith(URI_END)) {
                mStreamingType = STREAMING_SDP;
                if (DEBUG) {
                    Xlog.i(TAG, "SDP streaming type.");
                }
            } else {
                mStreamingType = STREAMING_RTSP;
                if (DEBUG) {
                    Xlog.i(TAG, "RTSP streaming type.");
                }
            }
        } else if (SCHEMA_HTTP.equalsIgnoreCase(scheme)) {
            mStreamingType = STREAMING_HTTP;
            mCanGetMetaData = true;
            if (DEBUG) {
                Xlog.i(TAG, "HTTP streaming type.");
            }
        } else {
            // The local video can get meta data.
            // Only streaming must wait.
            mCanGetMetaData = true;
            if (DEBUG) {
                Xlog.i(TAG, "Local Video streaming type.");
            }
        }
    }

    private void openVideo() {
        judgeStreamingType();
        if (mUri == null || mSurfaceHolder == null) {
            // Not ready for playback just yet, will try again later.
            return;
        }

        /// M: for s3d video
        if (mPreview) {
            // update shared pref uri to notify preview activity
            if (mSharedPref != null) {
                Editor edit = mSharedPref.edit();
                edit.putString(PREVIEW_WALLPAPER_URI, mUri.toString());
                edit.commit();
            }
        }

        // We shouldn't clear the target state, because somebody might have
        // called start() previously.
        release(false);
        // Switch 2d/3d surface layout if needed
        if (FeatureOption.MTK_S3D_SUPPORT) {
            setFlagsEx(false);
        }
        try {
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnInfoListener(mInfoListener);
            mMediaPlayer.setOnPreparedListener(mPreparedListener);
            mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
            mDuration = -1;
            mMediaPlayer.setOnCompletionListener(mCompletionListener);
            mMediaPlayer.setOnErrorListener(mErrorListener);
            mMediaPlayer.setOnBufferingUpdateListener(mBufferingUpdateListener);
            mCurrentBufferPercentage = 0;
            mMediaPlayer.setDataSource(mContext, mUri, mHeaders);

            if (DEBUG) {
                Xlog.i(TAG, "open video path: " + mUri + "context: " + mContext);
            }
            mMediaPlayer.setDisplay(mSurfaceHolder);
            mMediaPlayer.setAudioStreamType(AudioManager.STREAM_SYSTEM);
            mMediaPlayer.setVolume(0, 0);
            mMediaPlayer.setScreenOnWhilePlaying(true);

            // Disable ClearMotion
            mMediaPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);			
            mMediaPlayer.prepareAsync();
            // We don't set the target state here either, but preserve the
            // target state that was there before.
            mCurrentState = STATE_PREPARING;
            /// M: Turn off Auto Convergence
            if (FeatureOption.MTK_S3D_SUPPORT) {
                //mMediaPlayer.setParameter(Utils.KEY_PARAMETER_3D_OFFSET,
                //        Utils.VALUE_PARAMETER_3D_AC_OFF);
            }
            // attachMediaController();
        } catch (IOException ex) {
            Xlog.w(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer,
                    MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        } catch (IllegalArgumentException ex) {
            Xlog.w(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer,
                    MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        }
    }

    private void doPrepared(MediaPlayer mp) {
        if (DEBUG) {
            Xlog.i(TAG, "do prepared.");
        }
        mCurrentState = STATE_PREPARED;

        mStartFromBoot = false;
        mHandler.removeMessages(MSG_CLEAR);

        // Get the capabilities of the player for this stream
        Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                MediaPlayer.BYPASS_METADATA_FILTER);

        if (data != null) {
            mCanPause = !data.has(Metadata.PAUSE_AVAILABLE)
                    || data.getBoolean(Metadata.PAUSE_AVAILABLE);
            mCanSeekBack = !data.has(Metadata.SEEK_BACKWARD_AVAILABLE)
                    || data.getBoolean(Metadata.SEEK_BACKWARD_AVAILABLE);
            mCanSeekForward = !data.has(Metadata.SEEK_FORWARD_AVAILABLE)
                    || data.getBoolean(Metadata.SEEK_FORWARD_AVAILABLE);
        } else {
            mCanPause = true;
            mCanSeekBack = true;
            mCanSeekForward = true;
        }

        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared(mMediaPlayer);
        }
        // mSeekWhenPrepared may be changed after seekTo() call.
        long seekToPosition = mSeekWhenPrepared;

        if (seekToPosition != 0) {
            seekTo(seekToPosition);
        }
        final int duration = mp.getDuration();
        mEndTime = (mEndTime == VideoScene.DEFAULT_END) ? duration : mEndTime;
        final int width = mp.getVideoWidth();
        final int height = mp.getVideoHeight();
        if (width != 0 && height != 0) {
            if (DEBUG) {
                Xlog.i(TAG, "doPrepared, video size: " + width + "/" + height);
            }
            if (mVideoWidth != width || mVideoHeight != height) {
                if (DEBUG) {
                    Xlog.d(TAG, "Video size changed (" + mVideoWidth + "/"
                            + mVideoHeight + ")->(" + width + "/" + height
                            + "), relayout surface");
                }             
                mVideoWidth = width;
                mVideoHeight = height;
                relayout(mVideoWidth, mVideoHeight);
            }

            start();
        } else {
            // We don't know the video size yet, but should start anyway.
            // The video size might be reported to us later.
            // Xlog.v(TAG,"BLOCK HERE>>>>> width = "+width+", height = "+height+", mTargetState = "+mTargetState);
            start();
        }
    }
    
    private void updateBufferState(int percent) {
        if (mStreamingType == STREAMING_RTSP) {
            if (percent >= COMPLETE_PRECENT) {
                mCanPause = true;
                mCanSeekBack = true;
                mCanSeekForward = true;
                // TODO we should hide buffer state on the progressive bar here if needed
            } else {
                mCanPause = false;
                mCanSeekBack = false;
                mCanSeekForward = false;
                // TODO we should show buffer state on the progressive bar here if needed
            }
        }
    }

    /**
     * Register a callback to be invoked when the media file is loaded and ready
     * to go.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnPreparedListener(MediaPlayer.OnPreparedListener l) {
        mOnPreparedListener = l;
    }

    /**
     * Register a callback to be invoked when the end of a media file has been
     * reached during playback.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnCompletionListener(OnCompletionListener l) {
        mOnCompletionListener = l;
    }

    /**
     * Register a callback to be invoked when an error occurs during playback or
     * setup. If no listener is specified, or if the listener returned false,
     * VideoScene will inform the user of any errors.
     * 
     * @param l
     *            The callback that will be run
     */
    public void setOnErrorListener(OnErrorListener l) {
        mOnErrorListener = l;
    }

    public void setOnBufferingUpdateListener(OnBufferingUpdateListener l) {
        mOnBufferingUpdateListener = l;
    }

    public void resize(SurfaceHolder holder, int w, int h) {
        if (DEBUG) {
            Xlog.d(TAG, "onSurfaceChanged(" + w + "," + h + "), sh: " + holder);
        }
        mSurfaceWidth = w;
        mSurfaceHeight = h;
    }

    public void init(SurfaceHolder holder) {
        if (DEBUG) {
            Xlog.i(TAG, "init VideoScene, sh: " + holder);
        }
        mStartFromBoot = true;
        
        loadSettings();
        mSurfaceHolder = holder;

        // Seek to the right position saved last time.
        // We should not do this in preview mode.
        if (!mPreview) {
            mSeekWhenPrepared = mStartTime;
        }
        openVideo();
        
        if (mReceiver != null && mContext != null) {
            if (DEBUG) {
                Xlog.i(TAG, "register receiver: " + mReceiver);
            }
            IntentFilter filter = new IntentFilter();
            // When sdcard is available. Can not guarantee receiving this Broadcast
            // if sdcard mounted before VideoScene is started.
            filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            // When sdcard is unavailable.
            filter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
            filter.addAction(Intent.ACTION_MEDIA_REMOVED);
            filter.addAction(Intent.ACTION_MEDIA_EJECT);
            filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            filter.addDataScheme("file");
            // When external storage scanning complete.
            filter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);        
            mContext.registerReceiver(mReceiver, filter);
        }
        
        /*
         * Because if we want to receive unmount broadcast, we should
         * addDataScheme("file") to filter,but this will lead we can not receive
         * the shut down broadcast, so we register a new receiver to deal with
         * the shut down broadcast.
         */
        if (mShutdownReceiver != null && mContext != null) {
            IntentFilter filter = new IntentFilter();
            // Save Settings when shut down.
            filter.addAction(Intent.ACTION_SHUTDOWN);
            filter.addAction(ACTION_PRE_SHUTDOWN);
            filter.addAction(ACTION_BOOT_IPO);
            filter.addAction(Intent.ACTION_REBOOT);
            if (FeatureOption.MTK_SMARTBOOK_SUPPORT) {
                filter.addAction(Intent.ACTION_SMARTBOOK_PLUG);
            }
            mContext.registerReceiver(mShutdownReceiver, filter);
        }

        if (!Utils.isDefaultVideo(mUri) && !isInPlaybackState()) {
            // Only when first time ACTION_MEDIA_MOUNTED that MSG_RELOAD_VIDEO
            // will be sent.
            if (!mHandler.hasMessages(MSG_CLEAR)) {
                mHandler.sendEmptyMessageDelayed(MSG_CLEAR, CLEAR_TIME_OUT);
            }
        }
    }

    public void destroy() {
        mStartFromBoot = false;
        // After we return from this we can't use the surface any more.
        mSurfaceHolder = null;
        release(true);
        // Clear all info saved in shared_prefs when we destroy the Video Live wallpaper
        if (!mPreview && !isSmartBookPluggedIn()) {
            if (iSavedUserID == ActivityManager.getCurrentUser()) {
                clear(true, true);
            }
        }

        stopAndReleaseVideoObserver();

        if (DEBUG) {
            Xlog.i(TAG, "destroy VideoScene");
        }

        if (mReceiver != null && mContext != null) {
            if (DEBUG) {
                Xlog.i(TAG, "unregister receiver: " + mReceiver);
            }
            mContext.unregisterReceiver(mReceiver);
        }

        if (mShutdownReceiver != null && mContext != null) {
            mContext.unregisterReceiver(mShutdownReceiver);
        }
        Xlog.w(TAG, "exit vlw, enable video mode");
        final int curWidth = mWallpaperManager.getDesiredMinimumWidth();
        final int curHeight = mWallpaperManager.getDesiredMinimumHeight();
        Xlog.i(TAG, "getDesiredMinimumWidth: " + curWidth + " ,getDesiredMinimumHeight: " + curHeight);
    }

    /*
     * Release the media player in any state.
     */
    private void release(boolean cleartargetstate) {
        if (mMediaPlayer != null) {
            MediaPlayer temp = mMediaPlayer;
            mMediaPlayer = null;
            temp.stop();
            temp.release();
            mCurrentState = STATE_IDLE;
            if (cleartargetstate) {
                mTargetState = STATE_IDLE;
            }
        }
    }

    private void checkEnvironment() {
        Uri uri = null;
        String buckekId = null;
       int stereoType = DEFAULT_STEREO_TYPE;
        try {
            uri = Uri.parse(mSharedPref.getString(mPreview ? PREVIEW_WALLPAPER_URI : WALLPAPER_URI, null));
            buckekId = mSharedPref.getString(mPreview ? PREVIEW_BUCKET_ID : BUCKET_ID, null);
            stereoType = (int) mSharedPref.getLong(STEREO_TYPE, DEFAULT_STEREO_TYPE);
            if (mPreview) {
                stereoType = Utils.queryStereoType(mContext, uri);
            }
            Xlog.i(TAG, "checkEnvironment() stereoType: " + stereoType);
        } catch (NullPointerException e) {
            // Ignore
            Xlog.e(TAG, "Read in SharedPref failed");
        }

        boolean videoChangeOrReopen = false;
        if (mMediaPlayer == null && !mWaitingReload) {
            // Seek to the right position when we leave.
            // We should not do this in preview mode.
            if (!mPreview) {
                mSeekWhenPrepared = mCurrentPos;
            }
            Xlog.i(TAG, "checkEnvironment() MediaPlayer released, open video");
            openVideo();

        } else if (buckekId != null && !buckekId.equals(mBucketId)) {
            Xlog.i(TAG, "checkEnvironment() select folder from sharedpref: " + buckekId);
            clear(false, true);
            mBucketId = buckekId;
            mUri = uri;
            updateVideoIndex();
            mStereoType = stereoType;
            videoChangeOrReopen = true;
            openVideo();

        } else if (uri != null && !mUri.equals(uri)) {
            Xlog.i(TAG, "checkEnvironment() change video from sharedpref: " + uri);
            clear(false, true);
            mUri = uri;
            updateVideoIndex();
            mStereoType = stereoType;
            videoChangeOrReopen = true;
            
            // Need not reserve the selected sdcard video info last time now, 
            // because user want to set another video
            mPrevBucketId = null;
            mPrevUri = null;
            Editor edit = mSharedPref.edit();
            edit.putString(WALLPAPER_URI_PREV, null);
            edit.commit();

            openVideo();
        }
        mBucketId = buckekId;
        int start = (int)mSharedPref.getLong(START_TIME, mStartTime);
        int end = (int)mSharedPref.getLong(END_TIME, mEndTime);
        int current = (int)mSharedPref.getLong(CURRENT_POSITION, mCurrentPos);
        // If user change start time and set, show the change immediately.
        if (buckekId == null && !videoChangeOrReopen && start != mStartTime) {
            Xlog.i(TAG, "checkEnvironment() change start from sharedpref: " + start);
            mStartTime = start;
            mCurrentPos = start;
            seekTo(start);
        }
        // Otherwise, just apply only when the end is previous to current position
        if (buckekId == null && !videoChangeOrReopen && end != mEndTime) {
            Xlog.i(TAG, "checkEnvironment() change end from sharedpref: " + end);
            mEndTime = end;
            if (mMediaPlayer != null) {
                if (mCurrentPos > end) {
                    mCurrentPos = mStartTime;
                    seekTo(mCurrentPos);
                }
            }
        }
        if (videoChangeOrReopen) {
            mCurrentPos = current;
            // if reset another video, update mSeekWhenPrepared
            if (!mPreview) {
                mSeekWhenPrepared = mCurrentPos;
            }
            Xlog.i(TAG, "reset another video mUri = " + mUri + ", mSeekWhenPrepared = " + mSeekWhenPrepared
                    + ", mCurrentPos = " + mCurrentPos);
        }

        if (FeatureOption.MTK_S3D_SUPPORT) {
            /**
             * M: Correct it ASAP if user has changed stereo type of this video in Gallery
             */
        int stereo = Utils.queryStereoType(mContext, mUri);
        if (!videoChangeOrReopen && stereo != mStereoType && stereo != Utils.STEREO_TYPE_UNKNOWN) {
            Xlog.i(TAG, "checkEnvironment() User changes s3d type in Gallery: " + stereo);
            mStereoType = stereo;
            setFlagsEx(false);
            Editor edit = mSharedPref.edit();
            edit.putLong(STEREO_TYPE, mStereoType);
            edit.commit();
            }
        }
    }

    /// M: for s3d video
    private void setFlagsEx(boolean updateDatabase) {
        if (FeatureOption.MTK_S3D_SUPPORT) {
            final int flagsEx = Utils.getSurfaceStereoMode(Utils.isStereo(mStereoType))
                    | Utils.getSurfaceLayout(mStereoType);
            Xlog.i(TAG, "setFlagsEx(): 0x00" + Integer.toHexString(flagsEx));
            if (mContext instanceof VideoLiveWallpaper) {
                ((VideoLiveWallpaper)mContext).setFlagsEx(flagsEx, Utils.FLAG_EX_S3D_MASK, mPreview);
            }
            if (updateDatabase) {
                Xlog.i(TAG, "setFlagsEx() Update MediaStore database: " 
                        + mStereoType + ", " + mUri);
                Utils.updateMediaInfoToDatabase(mContext, mUri,
                        mStereoType, System.currentTimeMillis());
            }
        }
    }
    
    private void addAndStartVideoObserver() {
        if (mUri == null || Utils.isDefaultVideo(mUri)
                || !Utils.isExternalFileExists(mUri.getPath())) {
            return;
        }
        final String directory = new File(mUri.getPath()).getParent();
        if (mFileObserver != null && directory.equals(mObserverPath)) {
            return;
        }
        mObserverPath = directory;
        stopAndReleaseVideoObserver();
        if (DEBUG) {
            Xlog.d(TAG, "======FileObserver start to monitor " + mObserverPath);
        }
        mFileObserver = new FileObserver(mObserverPath, DELETE | DELETE_SELF
                | MOVED_FROM | MOVED_TO | MOVE_SELF) {
            @Override
            public void onEvent(int event, String path) {
                if (DEBUG) {
                    Xlog.d(TAG, "FileObserver::onEvent(0x"
                            + Integer.toHexString(event) + "," + path + ")");
                }
                String videoPath = new File(mUri.getPath()).getName();
                if ((event & DELETE) == DELETE) {
                    // A file was deleted from the monitored directory.
                    if (videoPath.equals(path)) {
                        Xlog.w(TAG, "FileObserver::onEvent() media file has beed deleted");
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                if (mBucketId != null) {
                                    handleVideoDeleted();
                                    playDefaultVideoOrNextVideo();
                                    if (mUriInvalid != null) {
                                        mUriInvalid.clear();
                                    }
                                } else {
                                    release(false);
                                    clear(true, true);
                                    openVideo();
                                }
                            }
                        });
                    }
                }
                
                if ((event & MOVED_FROM) == MOVED_FROM) {
                    // A file or subdirectory was moved from the monitored
                    // directory
                    if (videoPath.equals(path)) {
                        mMovingFile = path;
                    }
                }
                
                if ((event & MOVED_TO) == MOVED_TO) {
                    // A new file or subdirectory was created under the
                    // monitored directory
                    if (mMovingFile != null && mMovingFile.equals(videoPath)) {
                        Xlog.d(TAG,
                                "FileObserver::onEvent() media file has beed renamed "
                                        + mMovingFile + " --> " + path);
                        mMovingFile = null;
                        mUri = Uri.fromFile(new File(mObserverPath + "/" + path));
                        mStereoType = Utils.queryStereoType(mContext, mUri);
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                // Continue to play this video.
                                updateVideoIndex();
                                saveSettings();
                                release(false);
                                openVideo();
                            }
                        });
                        // We will start() when vlw is visible but not here!!!
                    }
                }
                
                if ((event & DELETE_SELF) == DELETE_SELF) {
                    // The monitored directory was deleted; monitoring effectively stops.
                    // Input path is null.
                    // This msg can be ignored because DELETE will come before it.
                    Xlog.w(TAG, "FileObserver::onEvent() parent directory has beed deleted");
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mUriInvalid != null) {
                                mUriInvalid.clear();
                            }
                        }
                    });
                }
                
                if ((event & MOVE_SELF) == MOVE_SELF) {
                    // The monitored directory was moved; monitoring continues. 
                    // Input path is null
                    // The bucket_id must be changed, need to update
                    Xlog.w(TAG, "FileObserver::onEvent() parent directory has beed renamed");
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mUriInvalid != null) {
                                mUriInvalid.clear();
                            }
                            release(false);
                            clear(true, true);
                            openVideo();
                        }
                    });
                }
            }
        };
        mFileObserver.startWatching();
    }

    private void handleVideoDeleted() {
        // Update mUriList. Attention: File observer not sync with media provider,
        // so received the delete event through file observer, db may not be
        // update by media provider
        mUriList = Utils.queryUrisFromBucketId(mContext, mBucketId);
        if (mUriList.contains(mUri)) {
            mUriList.remove(mUri);
        }
        for (Uri uri : mUriList) {
            Xlog.d(TAG, "handleVideoDeleted()" + uri);
        }
        --mMode;
    }
    
    private void playDefaultVideoOrNextVideo() {
        // Update mUriList.
        // mUriList = Utils.getUrisFromBucketId(mContext, mBucketId);
        if (mUriList.isEmpty()) {
            Xlog.i(TAG, "playDefaultVideoOrNextVideo(), play the default video");
            release(false);
            clear(true, true);
            openVideo();
        } else {
            Xlog.i(TAG, "playDefaultVideoOrNextVideo(),video playing is removed " +
                    "or invalid in selected folder, play another video");
            release(false);
            mMode = Utils.getLoopIndex(mLoopMode, mMode, mUriList, mUriInvalid);
            Xlog.d(TAG, "playDefaultVideoOrNextVideo() mMode: " + mMode);
            if (mMode >= 0) {
               // mUri = mUriList.get(mMode);
               // if (!Utils.isExternalFileExists(mUri.getPath())) {
               //     clear(true, true);
               //     mMode = 0;
               // }

               // / ALPS00759051 When delete several videos, loop to get correct video @ {
               getCorrectUriListAndMode();
               // @}
            } else {
                Xlog.d(TAG, "playDefaultVideoOrNextVideo(),No valid video in this folder, " +
                        "play default video, size=" + mUriList.size());
                clear(true, true);
                mMode = 0;
            }
            // Save in SharedPref so that play video correctly if user
            // delete the playing video then reboot device by taking
            // out battery.
            saveSettings();
            openVideo();
        }
    }

    private void getCorrectUriListAndMode() {
        int mode = mMode; // this mode means which video you want play.
        int size = mUriList.size();
        ArrayList<Uri> list = new ArrayList<Uri>();
        //list = mUriList;
        list.addAll(mUriList);
        Xlog.d(TAG, "getCorrectUris() enter, mMode : " + mMode + " size: " + size);
        for (int i = 0; i < size; i++) {
            Uri currentVideoUri = mUriList.get(i);
            if (!Utils.isExternalFileExists(currentVideoUri.getPath())) {
                Xlog.d(TAG, "getCorrectUris(), this video doesn't exist,index = " + i +", "+ currentVideoUri);
                list.remove(currentVideoUri);
                if (i < mMode) {
                    mode--;
                }
            }
            // Xlog.d(TAG, "getCorrectUris(), list.size = " + list.size() +", "+ mUriList.size());
        }

        if (list.isEmpty()) {
            mMode = 0;
            clear(true, true);
        } else {
            mMode = mode;
            mUriList = list;
            if (mMode > mUriList.size() - 1) {
                mMode = mUriList.size() - 1;
            }
            mUri = mUriList.get(mMode);
        }
        Xlog.d(TAG, "getCorrectUris() out, mMode : " + mMode + " size: " + mUriList.size() + " mUri :" + mUri);
    }
    
    private void stopAndReleaseVideoObserver() {
        if (mFileObserver != null) {
            mFileObserver.stopWatching();
        }
        mFileObserver = null;
    }
    
    public void start() {
        if (mHasShutdown) {
            Xlog.w(TAG, "shuting down, do not start to play");
            return;
        }
        checkEnvironment();
        addAndStartVideoObserver();
        if (isInPlaybackState()) {
            mMediaPlayer.start();
            mCurrentState = STATE_PLAYING;
        }
        mTargetState = STATE_PLAYING;
        // If user set end-time, we should stop at that time and restart again
        // from start-time
        if (mBucketId == null) {
            mHandler.sendEmptyMessage(MSG_MONITOR_POSITION);
        }
        mHandler.removeMessages(MSG_RELEASE_TIMER);
        if (FeatureOption.MTK_S3D_SUPPORT) {
            //update new convergence value
            if (!mPreview && mContext instanceof VideoLiveWallpaper) {
                int convergence = (int)mSharedPref.getLong(VideoScene.CONVERGENCE_VALUE, 
                    VideoS3dDepthTuning.CONVERGENCE_MAX/2);
                ((VideoLiveWallpaper)mContext).setFlagsEx(convergence, Utils.FLAG_EX_S3D_CONVERGENCE, mPreview);
            }
        }
    }

    public void pause() {
        if (isInPlaybackState()) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.pause();
                mCurrentState = STATE_PAUSED;
                mCurrentPos = mMediaPlayer.getCurrentPosition();
            }
        }
        mTargetState = STATE_PAUSED;
        if (mBucketId == null) {
            mHandler.removeMessages(MSG_MONITOR_POSITION);
        }
        // Start timer
        mHandler.sendEmptyMessageDelayed(MSG_RELEASE_TIMER, RELEASE_TIME_OUT);
        // / ALPS00747931 , save settings when vlm paused
        if (!mPreview) {
            Editor edit = mSharedPref.edit();
            edit.putLong(CURRENT_POSITION, (long) mCurrentPos);
            edit.commit();
        }
    }
    
    public void suspend() {
        release(false);
    }

    public void resume() {
        openVideo();
    }
    
    // Cache duration as mDuration for faster access.
    public int getDuration() {
        if (isInPlaybackState()) {
            if (mDuration > 0) {
                return mDuration;
            }
            mDuration = mMediaPlayer.getDuration();
            return mDuration;
        }
        mDuration = -1;
        return mDuration;
    }

    public int getCurrentPosition() {
        if (isInPlaybackState()) {
            mCurrentPos = mMediaPlayer.getCurrentPosition();
        }
        return mCurrentPos;
    }

    public void seekTo(long msec) {
        if (isInPlaybackState()) {
            if (mMediaPlayer != null) {
                mMediaPlayer.seekTo((int)msec);
            }
            mSeekWhenPrepared = 0;
        } else {
            mSeekWhenPrepared = msec;
        }
    }

    public boolean isPlaying() {
        return isInPlaybackState() && mMediaPlayer.isPlaying();
    }

    public int getBufferPercentage() {
        if (mMediaPlayer != null) {
            return mCurrentBufferPercentage;
        }
        return 0;
    }

    private boolean isInPlaybackState() {
        return (mMediaPlayer != null && mCurrentState != STATE_ERROR
                && mCurrentState != STATE_IDLE && mCurrentState != STATE_PREPARING);
    }

    public boolean canPause() {
        return mCanPause;
    }

    public boolean canSeekBackward() {
        return mCanSeekBack;
    }

    public boolean canSeekForward() {
        return mCanSeekForward;
    }

    public boolean isPreview() {
        return mPreview;
    }

    public int getSurfaceWidth() {
        return mSurfaceWidth;
    }

    public int getSurfaceHeight() {
        return mSurfaceHeight;
    }

    public Bundle doCommand(String action, int x, int y, int z, Bundle extras,
            boolean resultRequested) {
        return null;
    }

    // /To support Smart Book @{
    private boolean isSmartBookPluggedIn() {
        DisplayManager displayManager = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        return displayManager.isSmartBookPluggedIn();
    }
    // /@}
}
