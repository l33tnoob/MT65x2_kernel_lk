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
 */

package com.mediatek.videofavorites;

import android.content.Context;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.util.AttributeSet;
import android.widget.RemoteViews.RemoteView;

import com.mediatek.xlog.Xlog;

import java.io.IOException;


@RemoteView
public class RemoteVideoView extends VideoTextureView {

    private static final String TAG = "RemoteVideoView";

    public RemoteVideoView(Context context) {
        super(context);
    }

    public RemoteVideoView(Context context, AttributeSet attrs) {
        super(context, attrs, 0);
    }

    public RemoteVideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }




    @android.view.RemotableViewMethod
    @Override
    public void setVisibility(int visibility) {
        Xlog.i(TAG, "setVisibility(): v:" + visibility + ", " + this);
        super.setVisibility(visibility);
    }

    @android.view.RemotableViewMethod
    @Override
    public void setVideoPath(String path) {
        super.setVideoPath(path);
    }


    @android.view.RemotableViewMethod
    public void setVideoURI(String uri) {
        super.setVideoURI(Uri.parse(uri));
    }

    @android.view.RemotableViewMethod
    public void setVideoUriWithoutOpenVideo(String uri) {
        mUri = Uri.parse(uri);
        mSeekWhenPrepared = 0;
        requestLayout();
        invalidate();
    }

    boolean mMuteAudio;
    @android.view.RemotableViewMethod
    public void setAudioMute(boolean isMute) {
        mMuteAudio = isMute;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);

        final int maxWidth = getResources().getInteger(R.integer.thumb_width);
        final int maxHeight = getResources().getInteger(R.integer.thumb_height);
        // fit in rect but remain correct aspect ratio.
        if (mVideoWidth == 0 || mVideoHeight == 0) {
            width = maxWidth;
            height = maxHeight;
        } else {
            final float rectRatio = maxWidth / (float) maxHeight;
            final float videoRatio = mVideoWidth / (float) mVideoHeight;
            if (videoRatio < rectRatio) {
                width = maxWidth;
                height = width * mVideoHeight / mVideoWidth;
                if (height % 2 != 0) {
                    height ++;
                }
            } else {
                height = maxHeight;
                width = height * mVideoWidth / mVideoHeight;
            }
        }
        setMeasuredDimension(width, height);
    }

    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;

    @Override
    public void openVideo() {
        if (mMuteAudio) {
            Xlog.d(TAG, "openVideo(): " + this);
            if (mUri == null || mSurface == null) {
                // not ready for playback just yet, will try again later
                Xlog.d(TAG, "Not ready, return, mUri:" + mUri + ", mSurface:" + mSurface);
                return;
            }

            // we shouldn't clear the target state, because somebody might have
            // called start() previously
            release(false);
            try {
                mMediaPlayer = new MediaPlayer();
                mMediaPlayer.setOnPreparedListener(mPreparedListener);
                mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
                mDuration = -1;
                mMediaPlayer.setOnCompletionListener(mCompletionListener);
                mMediaPlayer.setOnErrorListener(mErrorListener);
                mMediaPlayer.setOnBufferingUpdateListener(mBufferingUpdateListener);
                mCurrentBufferPercentage = 0;
                mMediaPlayer.setDataSource(mContext, mUri, mHeaders);
                mMediaPlayer.setSurface(mSurface);
                // since it is remoteView, mute, and we don't want the adjust music volume UI appear
                // when user press volume key while this video is playing
                // change it from STREAM_MUSIC to STREAM_SYSTEM
                mMediaPlayer.setAudioStreamType(AudioManager.STREAM_SYSTEM);
                mMediaPlayer.setScreenOnWhilePlaying(true);
                mMediaPlayer.setVolume(0, 0);
                mMediaPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);
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
        } else {
            super.openVideo();
        }
    }

    @android.view.RemotableViewMethod
    public void setPlay(boolean play) {
        if (play) {
            start();
        } else {
            stopPlayback();
        }
    }

    public boolean isUriAvailable() {
        return (mUri != null) && (!("".equals(mUri.toString())));
    }

    public String getUriPath() {
        return mUri.getPath();
    }

    public String getUriScheme() {
        return mUri.getScheme();
    }

    // In the case that when a view is put in collection view. When refreshing,
    // the view will be temprorary detatched and put into a pool for reuse later.
    // But if handset runs out of memory in this case, the framework will traverse view tree to
    // delete views' GLcontext. However, during that moment the texture view is in pool,
    // the surface will not be deleted. so after it is adding to view tree again later
    // there will be strange behavior such as black frame or JE when destorying.
    // Therefore we fix it by force releasing surface in onStartTemporaryDetach().
    @Override
    public void onStartTemporaryDetach() {
        Xlog.d(TAG, "onStartTemporaryDetach():" + this);
        if (isInPlaybackState() && mMediaPlayer != null) {
            mCompletionListener.onCompletion(mMediaPlayer);
            stopPlayback();
        }
        super.onStartTemporaryDetach();
        destroySurfaceSafely();
    }

    @Override
    public void onFinishTemporaryDetach() {
        Xlog.d(TAG, "onFinishTemporaryDetach()" + this);
        super.onFinishTemporaryDetach();
    }
}
