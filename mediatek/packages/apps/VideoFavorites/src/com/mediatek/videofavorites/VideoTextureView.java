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


package com.mediatek.videofavorites;

import android.animation.ObjectAnimator;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnSeekCompleteListener;
import android.media.Metadata;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.widget.MediaController;
import android.widget.MediaController.MediaPlayerControl;
import android.widget.RemoteViews.RemoteView;

import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.Map;

/**
 * It is modified from android.widget.VideoView, by replacing SurfaceView with TextureView
 */
@RemoteView
public class VideoTextureView extends TextureView implements MediaPlayerControl {
    private static final String TAG = "VideoTextureView";
    // settable by the client
    /**
     * @hide
     */
    protected Uri         mUri;
    /**
     * @hide
     */
    protected Map<String, String> mHeaders;
    /**
     * @hide
     */
    protected int         mDuration;

    // all possible internal states
    /**
     * @hide
     */
    protected static final int STATE_ERROR              = -1;
    /**
     * @hide
     */
    protected static final int STATE_IDLE               = 0;
    /**
     * @hide
     */
    protected static final int STATE_PREPARING          = 1;
    /**
     * @hide
     */
    protected static final int STATE_PREPARED           = 2;
    /**
     * @hide
     */
    protected static final int STATE_PLAYING            = 3;
    /**
     * @hide
     */
    protected static final int STATE_PAUSED             = 4;
    /**
     * @hide
     */
    protected static final int STATE_PLAYBACK_COMPLETED = 5;

    /**
     * @hide
     */
    protected static final int STATE_STOPPING            = 6;    // for non-blocking stop.


    // mCurrentState is a VideoTextureView object's current state.
    // mTargetState is the state that a method caller intends to reach.
    // For instance, regardless the VideoTextureView object's current state,
    // calling pause() intends to bring the object to a target state
    // of STATE_PAUSED.
    /**
     * @hide
     */
    protected int mCurrentState = STATE_IDLE;
    /**
     * @hide
     */
    protected int mTargetState  = STATE_IDLE;

    // All the stuff we need for playing and showing a video
    /**
     * @hide
     */
    // protected SurfaceHolder mSurfaceHolder = null;

    protected Surface mSurface;
    /**
     * @hide
     */
    protected MediaPlayer mMediaPlayer;
    private int           mAudioSession;
    /**
     * @hide
     */
    protected int         mVideoWidth;
    /**
     * @hide
     */
    protected int         mVideoHeight;
    /**
     * @hide
     */
    protected int         mSurfaceWidth;
    /**
     * @hide
     */
    protected int         mSurfaceHeight;
    /**
     * @hide
     */
    protected MediaController mMediaController;
    /**
     * @hide
     */
    protected OnCompletionListener mOnCompletionListener;
    /**
     * @hide
     */
    protected MediaPlayer.OnPreparedListener mOnPreparedListener;
    /**
     * @hide
     */
    protected int         mCurrentBufferPercentage;
    /**
     * @hide
     */
    protected OnErrorListener mOnErrorListener;
    /**
     * @hide
     */
    protected int         mSeekWhenPrepared;  // recording the seek position while preparing
    /**
     * @hide
     */
    protected boolean     mCanPause;
    /**
     * @hide
     */
    protected boolean     mCanSeekBack;
    /**
     * @hide
     */
    protected boolean     mCanSeekForward;

    private boolean mIsSeeking;
    private boolean mStopWhenSeekComplete;


    private ObjectAnimator mAlphaAnimation;
    private boolean mFirstDraw = true;

    public VideoTextureView(Context context) {
        super(context);
        initVideoTextureView();
    }

    public VideoTextureView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
        initVideoTextureView();
    }

    public VideoTextureView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initVideoTextureView();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // Xlog.i("@@@@", "onMeasure");
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
        if (mVideoWidth > 0 && mVideoHeight > 0) {
            if (mVideoWidth * height  > width * mVideoHeight) {
                // Xlog.i("@@@", "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            } else if (mVideoWidth * height  < width * mVideoHeight) {
                // Xlog.i("@@@", "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            }
        }
        // Xlog.i("@@@@@@@@@@", "setting size: " + width + 'x' + height);
        setMeasuredDimension(width, height);
    }

    public int resolveAdjustedSize(int desiredSize, int measureSpec) {
        int result = desiredSize;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize =  MeasureSpec.getSize(measureSpec);

        switch (specMode) {
        case MeasureSpec.UNSPECIFIED:
            /* Parent says we can be as big as we want. Just don't be larger
             * than max size imposed on ourselves.
             */
            result = desiredSize;
            break;

        case MeasureSpec.AT_MOST:
            /* Parent says we can be as big as we want, up to specSize.
             * Don't be larger than specSize, and don't be larger than
             * the max size imposed on ourselves.
             */
            result = Math.min(desiredSize, specSize);
            break;

        case MeasureSpec.EXACTLY:
            // No choice. Do what we are told.
            result = specSize;
            break;

        default:
            break;

        }
        return result;
    }

    /**
     * @hide
     */
    private void initVideoTextureView() {
        mVideoWidth = 0;
        mVideoHeight = 0;
        setSurfaceTextureListener(mSTListener);
        mCurrentState = STATE_IDLE;
        mTargetState  = STATE_IDLE;
        mAlphaAnimation = ObjectAnimator.ofFloat(this, View.ALPHA, 0.0f, 1.0f).setDuration(200);
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
        requestLayout();
        invalidate();
    }

    public void stopPlayback() {
        Xlog.d(TAG, "stopPlayback(): " + getId() + " skipped:" + (mMediaPlayer == null));
        if (mMediaPlayer != null) {
            mMediaPlayer.stop();
            mMediaPlayer.release();
            mMediaPlayer = null;
            mCurrentState = STATE_IDLE;
            mTargetState  = STATE_IDLE;
        }
    }

    public void stopPlaybackAsync() {
        Xlog.d(TAG, "stopPlayback() Async: " + getId() + " skipped:" + (mMediaPlayer == null));
        if (mMediaPlayer != null) {
            if (mIsSeeking) {
                mStopWhenSeekComplete = true;
                return;
            }
            mCurrentState = STATE_IDLE;
            mTargetState = STATE_IDLE;

            final MediaPlayer finalPlayer = mMediaPlayer;
            mMediaPlayer = null;
            new Thread(new Runnable() {
                public void run() {
                    Xlog.d(TAG, "stopPlayback thread(): executed");
                    finalPlayer.stop();
                    finalPlayer.release();
                }
            }).start();
        }
    }

    /**
     * @hide
     */
    public void openVideo() {
        if (mUri == null || mSurface == null) {
            // not ready for playback just yet, will try again later
            return;
        }
        Xlog.d(TAG, "openVideo() viewId:" + getId());
        // Tell the music playback service to pause
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");
        mContext.sendBroadcast(i);

        // we shouldn't clear the target state, because somebody might have
        // called start() previously
        release(false);
        try {
            mMediaPlayer = new MediaPlayer();
            if (mAudioSession != 0) {
                mMediaPlayer.setAudioSessionId(mAudioSession);
            } else {
                mAudioSession = mMediaPlayer.getAudioSessionId();
            }
            mMediaPlayer.setOnPreparedListener(mPreparedListener);
            mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
            mDuration = -1;
            mMediaPlayer.setOnCompletionListener(mCompletionListener);
            mMediaPlayer.setOnErrorListener(mErrorListener);
            mMediaPlayer.setOnBufferingUpdateListener(mBufferingUpdateListener);
            mMediaPlayer.setOnSeekCompleteListener(mSeekCompleteListener);
            mCurrentBufferPercentage = 0;
            mMediaPlayer.setDataSource(mContext, mUri, mHeaders);
            mMediaPlayer.setSurface(mSurface);
            mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mMediaPlayer.setScreenOnWhilePlaying(true);
            mMediaPlayer.prepareAsync();
            // we don't set the target state here either, but preserve the
            // target state that was there before.
            mCurrentState = STATE_PREPARING;
            attachMediaController();
        } catch (IOException ex) {
            Xlog.w(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        } catch (IllegalArgumentException ex) {
            Xlog.w(TAG, "Unable to open content: " + mUri, ex);
            mCurrentState = STATE_ERROR;
            mTargetState = STATE_ERROR;
            mErrorListener.onError(mMediaPlayer, MediaPlayer.MEDIA_ERROR_UNKNOWN, 0);
            return;
        }
    }

    public void setMediaController(MediaController controller) {
        if (mMediaController != null) {
            mMediaController.hide();
        }
        mMediaController = controller;
        attachMediaController();
    }
    /**
     * @hide
     */
    protected void attachMediaController() {
        if (mMediaPlayer != null && mMediaController != null) {
            mMediaController.setMediaPlayer(this);
            View anchorView = this.getParent() instanceof View ?
                              (View)this.getParent() : this;
            mMediaController.setAnchorView(anchorView);
            mMediaController.setEnabled(isInPlaybackState());
        }
    }
    /**
     * @hide
     */
    protected MediaPlayer.OnVideoSizeChangedListener mSizeChangedListener =
        new MediaPlayer.OnVideoSizeChangedListener() {
            public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
                if (!isInPlaybackState()) {
                    return;
                }

                mVideoWidth = mp.getVideoWidth();
                mVideoHeight = mp.getVideoHeight();
                if (mVideoWidth != 0 && mVideoHeight != 0) {
                    //mSurface.openTransaction();
                    //mSurface.setSize(mVideoWidth, mVideoHeight);
                    //mSurface.closeTransaction();
                }
            }
        };


    /**
     * @hide
     */
    protected MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mp) {
            if (mCurrentState != STATE_PREPARING) {
                Xlog.d(TAG, "onPrepared(): ignored, mCurrentState: " + mCurrentState);
                return;
            }
            mCurrentState = STATE_PREPARED;

            if (mVideoWidth == 0 || mVideoHeight == 0) {
                // m: avoid get width/height repeatedly in video favorite case

                // Get the capabilities of the player for this stream
                Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                                               MediaPlayer.BYPASS_METADATA_FILTER);

                if (data == null) {
                    mCanPause = true;
                    mCanSeekBack = true;
                    mCanSeekForward = true;
                } else {
                    mCanPause = !data.has(Metadata.PAUSE_AVAILABLE)
                                || data.getBoolean(Metadata.PAUSE_AVAILABLE);
                    mCanSeekBack = !data.has(Metadata.SEEK_BACKWARD_AVAILABLE)
                                   || data.getBoolean(Metadata.SEEK_BACKWARD_AVAILABLE);
                    mCanSeekForward = !data.has(Metadata.SEEK_FORWARD_AVAILABLE)
                                      || data.getBoolean(Metadata.SEEK_FORWARD_AVAILABLE);
                }
            }

            if (mOnPreparedListener != null) {
                mOnPreparedListener.onPrepared(mMediaPlayer);
            }
            if (mMediaController != null) {
                mMediaController.setEnabled(true);
            }
            mVideoWidth = mp.getVideoWidth();
            mVideoHeight = mp.getVideoHeight();

            int seekToPosition = mSeekWhenPrepared;  // mSeekWhenPrepared may be changed after seekTo() call
            if (seekToPosition != 0) {
                seekTo(seekToPosition);
            }
            if (mVideoWidth == 0 || mVideoHeight == 0) {
                // We don't know the video size yet, but should start anyway.
                // The video size might be reported to us later.
                if (mTargetState == STATE_PLAYING) {
                    start();
                    // We should also display the progressive bar
                    if (mMediaController != null) {
                        mMediaController.show();
                    }
                } else if (!isPlaying() && (seekToPosition != 0 || getCurrentPosition() > 0)) {
                    // For Video which only has audio output,show progressive bar when returned.
                    if (mMediaController != null) {
                        mMediaController.show(0);
                    }
                }

            } else {
                // Xlog.i("@@@@", "video size: " + mVideoWidth +"/"+ mVideoHeight);
                //mSurface.openTransaction();
                //mSurface.setSize(mVideoWidth, mVideoHeight);
                //mSurface.closeTransaction();
                requestLayout();

                /*
                 * Technically speaking, the following condition should always be true,
                 * because we set video's size to surface. But sometimes, surfaceChanged
                 * will be called again, resulted in that the following condition becomes
                 * false. As a result, no start() gets called when MediaPlayer ready to
                 * play.
                 */
                // We didn't actually change the size (it was already at the size
                // we need), so we won't get a "surface changed" callback, so
                // start the video here instead of in the callback.
                if (mTargetState == STATE_PLAYING) {
                    start();
                    if (mMediaController != null) {
                        mMediaController.show();
                    }
                } else if (!isPlaying() &&
                        (seekToPosition != 0 || getCurrentPosition() > 0)) {
                    if (mMediaController != null) {
                        // Show the media controls when we're paused into a video and make 'em stick.
                        mMediaController.show(0);
                    }
                }
            }
        }
    };
    /**
     * @hide
     */
    protected MediaPlayer.OnCompletionListener mCompletionListener =
        new MediaPlayer.OnCompletionListener() {
            public void onCompletion(MediaPlayer mp) {
                mCurrentState = STATE_PLAYBACK_COMPLETED;
                mTargetState = STATE_PLAYBACK_COMPLETED;
                if (mMediaController != null) {
                    mMediaController.hide();
                }
                if (mOnCompletionListener != null) {
                    mOnCompletionListener.onCompletion(mMediaPlayer);
                }
            }
        };
    /**
     * @hide
     */
    protected MediaPlayer.OnErrorListener mErrorListener =
        new MediaPlayer.OnErrorListener() {
            public boolean onError(MediaPlayer mp, int frameworkErr, int implErr) {
                Xlog.d(TAG, "Error: " + frameworkErr + "," + implErr);
                mCurrentState = STATE_ERROR;
                mTargetState = STATE_ERROR;
                if (mMediaController != null) {
                    mMediaController.hide();
                }

                /* If an error handler has been supplied, use it and finish. */
                if (mOnErrorListener != null) {
                    if (mOnErrorListener.onError(mMediaPlayer, frameworkErr, implErr)) {
                        return true;
                    }
                }

                /* Otherwise, pop up an error dialog so the user knows that
                 * something bad has happened. Only try and pop up the dialog
                 * if we're attached to a window. When we're going away and no
                 * longer have a window, don't bother showing the user an error.
                 */
                if (getWindowToken() != null) {
                    int messageId;

                    if (frameworkErr == MediaPlayer.MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
                        messageId = com.android.internal.R.string.VideoView_error_text_invalid_progressive_playback;
                    } else {
                        messageId = com.android.internal.R.string.VideoView_error_text_unknown;
                    }

                    new AlertDialog.Builder(mContext)
                        .setTitle(com.android.internal.R.string.VideoView_error_title)
                        .setMessage(messageId)
                        .setPositiveButton(com.android.internal.R.string.VideoView_error_button,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        /* If we get here, there is no onError listener, so
                                         * at least inform them that the video is over.
                                         */
                                        if (mOnCompletionListener != null) {
                                            mOnCompletionListener.onCompletion(mMediaPlayer);
                                        }
                                    }
                                })
                        .setCancelable(false)
                        .show();
                }
                return true;
            }
        };
    /**
     * @hide
     */
    protected MediaPlayer.OnBufferingUpdateListener mBufferingUpdateListener =
        new MediaPlayer.OnBufferingUpdateListener() {
            public void onBufferingUpdate(MediaPlayer mp, int percent) {
                mCurrentBufferPercentage = percent;
            }
        };

    /**
     * Register a callback to be invoked when the media file
     * is loaded and ready to go.
     *
     * @param l The callback that will be run
     */
    public void setOnPreparedListener(MediaPlayer.OnPreparedListener l) {
        mOnPreparedListener = l;
    }

    /**
     * Register a callback to be invoked when the end of a media file
     * has been reached during playback.
     *
     * @param l The callback that will be run
     */
    public void setOnCompletionListener(OnCompletionListener l) {
        mOnCompletionListener = l;
    }

    /**
     * Register a callback to be invoked when an error occurs
     * during playback or setup.  If no listener is specified,
     * or if the listener returned false, VideoView will inform
     * the user of any errors.
     *
     * @param l The callback that will be run
     */
    public void setOnErrorListener(OnErrorListener l) {
        mOnErrorListener = l;
    }


    /**
     * @hide
     */
    protected TextureView.SurfaceTextureListener mSTListener =
        new TextureView.SurfaceTextureListener() {
            public void onSurfaceTextureAvailable(SurfaceTexture surface,
            int width, int height) {
                Xlog.i(TAG, "onSurfaceTextureAvailable");
                mSurface = new Surface(surface);
                mSurfaceWidth = width;
                mSurfaceHeight = height;
                openVideo();
            }

            public void onSurfaceTextureSizeChanged(SurfaceTexture surface,
            int w, int h) {
                mSurfaceWidth = w;
                mSurfaceHeight = h;
                boolean isValidState = (mTargetState == STATE_PLAYING);
                boolean hasValidSize = (mVideoWidth == w && mVideoHeight == h);
                if (mMediaPlayer != null && isValidState && hasValidSize) {
                    if (mSeekWhenPrepared != 0) {
                        seekTo(mSeekWhenPrepared);
                    }
                    start();
                }
            }

            public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                Xlog.i(TAG, "onSurfaceTextureDestroyed");
                mSurface.release();
                mSurface = null;
                if (mMediaController != null) {
                    mMediaController.hide();
                }
                release(true);
                return true;
            }

            public void onSurfaceTextureUpdated(SurfaceTexture surface) {
                // ignore for now since don't need it.
                if (mFirstDraw) {
                    Xlog.i(TAG, "onSurfaceTextureUpdated(), drawing first time");
                    mAlphaAnimation.start();
                    mFirstDraw = false;
                }
            }
        };
    /**
     * release the media player in any state
     * @hide
     */
    protected void release(boolean cleartargetstate) {
        if (mMediaPlayer != null) {
            mMediaPlayer.reset();
            mMediaPlayer.release();
            mMediaPlayer = null;
            mCurrentState = STATE_IDLE;
            if (cleartargetstate) {
                mTargetState  = STATE_IDLE;
            }
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        boolean isKeyCodeSupported = keyCode != KeyEvent.KEYCODE_BACK &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_UP &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_DOWN &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_MUTE &&
                                     keyCode != KeyEvent.KEYCODE_MENU &&
                                     keyCode != KeyEvent.KEYCODE_CALL &&
                                     keyCode != KeyEvent.KEYCODE_ENDCALL &&
                                     keyCode != KeyEvent.KEYCODE_CAMERA;
        if (isInPlaybackState() && isKeyCodeSupported && mMediaController != null) {
            if (keyCode == KeyEvent.KEYCODE_HEADSETHOOK ||
                    keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
                if (mMediaPlayer.isPlaying()) {
                    pause();
                    mMediaController.show();
                } else {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY) {
                if (!mMediaPlayer.isPlaying()) {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                       || keyCode == KeyEvent.KEYCODE_MEDIA_PAUSE) {
                if (mMediaPlayer.isPlaying()) {
                    pause();
                    mMediaController.show();
                }
                return true;
            } else {
                toggleMediaControlsVisiblity();
            }
        }

        return super.onKeyDown(keyCode, event);
    }
    /**
     * @hide
     */
    protected void toggleMediaControlsVisiblity() {
        if (mMediaController.isShowing()) {
            mMediaController.hide();
        } else {
            mMediaController.show();
        }
    }

    public void start() {
        if (isInPlaybackState()) {
            mMediaPlayer.start();
            mCurrentState = STATE_PLAYING;
        }
        mTargetState = STATE_PLAYING;
    }

    public void pause() {
        if (isInPlaybackState()) {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.pause();
                mCurrentState = STATE_PAUSED;
            }
        }
        mTargetState = STATE_PAUSED;
    }

    public void suspend() {
        release(false);
    }

    public void resume() {
        openVideo();
    }

    // cache duration as mDuration for faster access
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
            return mMediaPlayer.getCurrentPosition();
        }
        return 0;
    }

    public void seekTo(int msec) {
        if (isInPlaybackState()) {
            mIsSeeking = true;
            mMediaPlayer.seekTo(msec);
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


    public boolean isInPlaybackState() {
        return (mMediaPlayer != null &&
                mCurrentState != STATE_ERROR &&
                mCurrentState != STATE_IDLE &&
                mCurrentState != STATE_PREPARING);
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

    public boolean isSameMediaPlayer(MediaPlayer mp) {
        return (mp != null && mp.equals(mMediaPlayer));
    }


    /**
     *  @hide
     */
    private OnSeekCompleteListener mOnSeekCompleteListener;

    /**
     *  @hide
     */
    private final MediaPlayer.OnSeekCompleteListener mSeekCompleteListener =
        new OnSeekCompleteListener() {
            public void onSeekComplete(MediaPlayer mp) {
                mIsSeeking = false;
                if (mStopWhenSeekComplete) {
                    mStopWhenSeekComplete = false;
                    stopPlaybackAsync();
                    return;
                }

                if (mOnSeekCompleteListener != null) {
                    Xlog.v(TAG, "calling onSeekComplete()");
                    mOnSeekCompleteListener.onSeekComplete(mp);
                }
            }
        };

    public void setOnSeekCompleteListener(OnSeekCompleteListener listener) {
        mOnSeekCompleteListener = listener;
    }

    public boolean isInErrorState() {
        return (mCurrentState == STATE_ERROR);
    }

    @Override
    public int getAudioSessionId() {
        if (mAudioSession == 0) {
            MediaPlayer foo = new MediaPlayer();
            mAudioSession = foo.getAudioSessionId();
            foo.release();
        }
        return mAudioSession;
    }

    // add animation to workaroud black visual defects
    public void cancelAnimation() {
        mAlphaAnimation.cancel();
    }

    public void prepareAnimation() {
        setAlpha(0.0f);
        mFirstDraw = true;
    }
}

