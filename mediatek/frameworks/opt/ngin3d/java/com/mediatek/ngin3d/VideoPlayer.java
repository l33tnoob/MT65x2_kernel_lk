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

package com.mediatek.ngin3d;

import android.content.Context;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

class VideoPlayer {
    private static final String TAG = "Ngin3d.VideoPlayer";

    public static final int INVALID_SEGMENT_ID = -1;
    public static final int REPLAY_SEGMENT_ID = -2;

    // all possible internal states
    private static final int STATE_ERROR = -1;
    private static final int STATE_IDLE = 0;
    private static final int STATE_PREPARING = 1;
    private static final int STATE_PREPARED = 2;
    private static final int STATE_PLAYING = 3;
    private static final int STATE_PAUSED = 4;
    private static final int STATE_PLAYBACK_COMPLETED = 5;

    private static final int INVALID_VOLUME = -1;

    private final Uri mUri;
    private MediaPlayer mPlayer;

    private int mSegmentId = INVALID_SEGMENT_ID;
    private int mDuration;
    private int mLoopStartMs;
    private int mLoopEndMs;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mCurrentState = STATE_IDLE;
    private int mIntentState = STATE_IDLE;
    private Handler mLoopHandler;
    private Runnable mLoopRunner;
    private boolean mEnableMusicPause;
    private boolean mEnableLooping;
    private float mLeftVolume = INVALID_VOLUME;
    private float mRightVolume = INVALID_VOLUME;

    // Use to disable clear motion feature
    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;

    private final MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mediaPlayer) {
            setCurrentState(STATE_PREPARED);
            notifyPreparedListener(mediaPlayer);
            try {
                mVideoWidth = mediaPlayer.getVideoWidth();
                mVideoHeight = mediaPlayer.getVideoHeight();
                mDuration = mediaPlayer.getDuration();
            } catch (IllegalStateException e) {
                Log.e(TAG, "MediaPlayer object has been released. Exception : " + e);
                return;
            }

            // TODO : seek
            if (mVideoWidth == 0 || mVideoHeight == 0) {
                 // TODO : report size issue.
                if (mIntentState == STATE_PLAYING) {
                    start(mSegmentId);
                }
            } else {
                if (mIntentState == STATE_PLAYING) {
                    start(mSegmentId);
                }
            }
        }
    };

    private final MediaPlayer.OnCompletionListener mCompletionListener = new MediaPlayer.OnCompletionListener() {
        public void onCompletion(MediaPlayer mediaPlayer) {
            setAllState(STATE_PLAYBACK_COMPLETED);
            notifyCompletionListener(mediaPlayer);
        }
    };

    private final MediaPlayer.OnVideoSizeChangedListener mSizeChangedListener = new MediaPlayer.OnVideoSizeChangedListener() {
        public void onVideoSizeChanged(MediaPlayer mediaPlayer, int width, int height) {
            try {
                mVideoWidth = mediaPlayer.getVideoWidth();
                mVideoHeight = mediaPlayer.getVideoHeight();
            } catch (IllegalStateException e) {
                Log.e(TAG, "MediaPlayer object has been released. Exception : " + e);
                return;
            }
        }
    };

    private final MediaPlayer.OnErrorListener mErrorListener = new MediaPlayer.OnErrorListener() {
        public boolean onError(MediaPlayer mediaPlayer, int frameworkErr, int implErr) {
            setAllState(STATE_ERROR);
            if (notifyErrorListener(mediaPlayer, frameworkErr, implErr)) {
                return true;
            }
            return true;
        }
    };

    private Surface mSurface;
    private int mFrameCount;
    private int mTotalFrameCount;
    private long mFrameCountingStart;
    private double mFPS;

    private void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture) {
        mSurface = new Surface(surfaceTexture);
    }

    private void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture) {
        final boolean isValidState = (mIntentState == STATE_PLAYING);
        if (mPlayer != null && isValidState) {
            // TODO : seek issue
            start(mSegmentId);
        }
    }

    private boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        release(true);
        return true;
    }

    private static final boolean ENABLE_FPS_DUMP = true;
    private void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
        if (ENABLE_FPS_DUMP) {
            long now = System.nanoTime();
            if (mFrameCountingStart == 0) {
                mFrameCountingStart = now;
            } else if ((now - mFrameCountingStart) > 1000000000) {
                mFPS = (double) mFrameCount * 1000000000 / (now - mFrameCountingStart);
                Log.v(TAG, "fps: " + mFPS);
                mFrameCountingStart = now;
                mFrameCount = 0;
            }
            ++mFrameCount;
            ++mTotalFrameCount;
        }
    }

    // Send notification to client
    private MediaPlayer.OnCompletionListener mOnCompletionListener;
    private MediaPlayer.OnPreparedListener mOnPreparedListener;
    private MediaPlayer.OnErrorListener mOnErrorListener;

    Context mCtx;
    public VideoPlayer(Context context, Uri uri) {
        mCtx = context;
        mUri = uri;
        initializeView();
    }

    public boolean setLooping(final boolean enableLooping) {
        mEnableLooping = enableLooping;
        return mEnableLooping;
    }

    public void setVolume(float leftVolume, float rightVolume) {
        if (leftVolume >= 0 && rightVolume >= 0) {
            mLeftVolume = leftVolume;
            mRightVolume = rightVolume;
        } else {
            throw new IllegalArgumentException("The volume can't be negtive");
        }
    }

    public boolean setBackgroundMusicPauseEnabled(final boolean pause) {
        mEnableMusicPause = pause;
        return mEnableMusicPause;
    }

    public void prepareVideo() {
        openVideo();
    }

    private void initializeView() {
        mVideoWidth = 0;
        mVideoHeight = 0;
        setAllState(STATE_IDLE);
    }

    private void openVideo() {
        if (mUri == null || mSurface == null) {
            return;
        }

        if (mEnableMusicPause) {
            sendMusicPauseRequest();
        }

        release(false);
        try {
            mPlayer = new MediaPlayer();
            mPlayer.setOnPreparedListener(mPreparedListener);
            mPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
            mPlayer.setOnCompletionListener(mCompletionListener);
            mPlayer.setOnErrorListener(mErrorListener);
            mPlayer.setDataSource(mCtx,  mUri);
            mPlayer.setSurface(mSurface);
            mSurface.release();
            mSurface = null;
            mPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);
            mPlayer.prepareAsync();
            setCurrentState(STATE_PREPARING);
        } catch (IOException ex) {
            Log.e(TAG, "IOException : " + ex);
            setAllState(STATE_ERROR);
            mErrorListener.onError(mPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "IllegalArgumentException : " + ex);
            setAllState(STATE_ERROR);
            mErrorListener.onError(mPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
        }
    }

    private void prepareLoopTimer(final int startMs, final int endMs) {
        if (mEnableLooping) {
            if (mLoopHandler == null) {
                mLoopHandler = new Handler();
            }

            mLoopStartMs = startMs;
            mLoopEndMs = endMs;
            if (mLoopRunner == null) {
                mLoopRunner = new Runnable() {
                    public void run() {
                        seekToAndStart(mLoopStartMs, mLoopEndMs);
                    }
                };
            }
            mLoopHandler.postDelayed(mLoopRunner, mLoopEndMs - mLoopStartMs);
        }
    }

    private void removeLoopTimer() {
        if (mLoopHandler != null && mLoopRunner != null) {
            mLoopHandler.removeCallbacks(mLoopRunner);
            mLoopRunner = null;
        }
    }

    public void start() {
        if (isPlayable()) {
            if (mLeftVolume != INVALID_VOLUME) {
                mPlayer.setVolume(mLeftVolume, mRightVolume);
            }
            mPlayer.setLooping(mEnableLooping);
            mPlayer.start();
            setCurrentState(STATE_PLAYING);
        }
        setIntentState(STATE_PLAYING);
    }

    private boolean isValidPeriod(final int startMs, final int endMs) {
        return (startMs <= endMs) && (startMs >= 0 && startMs <= mDuration) && (endMs >= 0 && endMs <= mDuration);
    }

    public void start(int segmentId) {
        if (segmentId != REPLAY_SEGMENT_ID) {
            mSegmentId = segmentId;
        }
        start();
        setIntentState(STATE_PLAYING);
    }

    private void
    seekToAndStart(final int startMs, final int endMs) {
        if (isPlayable()) {
            pause();
            seekTo(startMs);
            start();
            prepareLoopTimer(startMs, endMs);
        }
    }

    public void pause() {
        if (isPlayable() && isPlaying()) {
            removeLoopTimer();
            mPlayer.pause();
            setCurrentState(STATE_PAUSED);
        }
        setIntentState(STATE_PAUSED);
    }

    public void stopPlayback() {
        if (mPlayer != null) {
            removeLoopTimer();
            mPlayer.stop();
            mPlayer.release();
            mPlayer = null;
            setAllState(STATE_IDLE);
        }
    }

    public void seekTo(int mSec) {
        if (isPlayable()) {
            mPlayer.seekTo(mSec);
        }
    }

    public void release(boolean clearIntent) {
        if (mPlayer != null) {
            removeLoopTimer();
            mPlayer.reset();
            mPlayer.release();
            mPlayer = null;
            setCurrentState(STATE_IDLE);
            if (clearIntent) {
                setIntentState(STATE_IDLE);
            }
        }
    }

    public boolean isPlaying() {
        return (isPlayable() && mPlayer.isPlaying());
    }

    private boolean isPlayable() {
        return (mPlayer != null
                && mCurrentState != STATE_ERROR
                && mCurrentState != STATE_IDLE
                && mCurrentState != STATE_PREPARING);
    }

    public void setOnPreparedListener(MediaPlayer.OnPreparedListener listener) {
        mOnPreparedListener = listener;
    }

    public void setOnCompletionListener(MediaPlayer.OnCompletionListener listener) {
        mOnCompletionListener = listener;
    }

    public void setmOnErrorListener(MediaPlayer.OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    private boolean notifyCompletionListener(MediaPlayer mediaplayer) {
        if (mOnCompletionListener != null) {
            mOnCompletionListener.onCompletion(mediaplayer);
            return true;
        }
        return false;
    }

    private boolean notifyPreparedListener(MediaPlayer mediaplayer) {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared(mediaplayer);
            return true;
        }
        return false;
    }

    private boolean notifyErrorListener(MediaPlayer mediaplayer, int frameworkErr, int implErr) {
        if (mOnErrorListener != null) {
            mOnErrorListener.onError(mediaplayer, frameworkErr, implErr);
            return true;
        }
        return false;
    }

    private void sendMusicPauseRequest() {
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");
        mCtx.sendBroadcast(i);
    }

    private void setCurrentState(int state) {
        mCurrentState = state;
    }

    private void setIntentState(int state) {
        mIntentState = state;
    }

    private void setAllState(int state) {
        setCurrentState(state);
        setIntentState(state);
    }

    private SurfaceTexture mSurfaceTexture;
    private final AtomicBoolean mIsNewFrameArrival = new AtomicBoolean(false);

    public void initialize(int textureName) {
        if (mSurfaceTexture != null) {
            mSurfaceTexture.release();
            mSurfaceTexture = null;
        }

        mSurfaceTexture = new SurfaceTexture(textureName);
        mSurfaceTexture.setOnFrameAvailableListener(
            new SurfaceTexture.OnFrameAvailableListener() {
                public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                    mIsNewFrameArrival.compareAndSet(false, true);
                }
            }
        );

        onSurfaceTextureAvailable(mSurfaceTexture);
        prepareVideo();
        start();
    }

    public boolean isInitialized() {
        return (mSurfaceTexture != null);
    }

    private boolean mIsFirstFrameAndPause = true;

    private boolean isFirstFrameAndPause() {
        boolean isPause = mIsFirstFrameAndPause;
        mIsFirstFrameAndPause = false;
        return isPause;
    }

    // Need to call from GL Thread.
    // To update texture image if there is a new available frame from video source.
    public boolean applyUpdate() {
        if (mSurfaceTexture == null) {
            return false;
        }
        if (!mIsNewFrameArrival.get()) {
            return false;
        }

        mIsNewFrameArrival.set(false);
        mSurfaceTexture.updateTexImage();
        onSurfaceTextureUpdated(mSurfaceTexture);
        if (isFirstFrameAndPause()) {
            pause();
            return true;
        }
        return false;
    }

    /**
     * Retrieve the 4x4 texture coordinate transform matrix associated with the texture image set by
     * the most recent call to updateTexImage.
     *
     * @param matrix the array into which the 4x4 matrix will be stored.  The array must have exactly
     *     16 elements.
     * @return true for success to get the matrix and false for fail to get it.
     */
    public boolean getTransformMatrix(float[] matrix) {
        if (mSurfaceTexture == null) {
            return false;
        } else {
            mSurfaceTexture.getTransformMatrix(matrix);
            return true;
        }
    }

    public void destroy() {
        stopPlayback();
        mIsNewFrameArrival.set(false);
        if (mSurfaceTexture != null) {
            mSurfaceTexture.release();
            mSurfaceTexture = null;
        }
    }
}
