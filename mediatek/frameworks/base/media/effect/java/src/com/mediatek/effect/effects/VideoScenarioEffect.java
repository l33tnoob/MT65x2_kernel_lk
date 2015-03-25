/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.effects;

import java.io.IOException;
import java.util.HashMap;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.ProcessDoneListener;
import com.mediatek.effect.filterpacks.ve.ScenarioReader;
import com.mediatek.effect.filterpacks.ve.VideoEvent;
import com.mediatek.effect.filterpacks.ve.VideoScenario;
import com.mediatek.effect.player.EffectMediaPlayer;

import android.content.Context;
import android.media.AudioManager;
import android.media.CamcorderProfile;
import android.media.MediaPlayer;

public class VideoScenarioEffect {
    protected static int[] mCount = {0};
    protected MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    public VideoScenarioEffect() {
        super();
        mTool.log('d', getClass().getSimpleName() + "()");
        mTool.setIDandIncrease(mCount);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    private EffectMediaPlayer mMediaPlayer;

    public boolean setScenario(Context context, String scenario) {
        return setScenario(context, scenario, null, null, null);
    }

    public synchronized boolean setScenario(Context context, String scenario, CamcorderProfile videoProfile, Object object1, Object object2) {
        mTool.log('d', "setScenario()");

        VideoScenario scen = ScenarioReader.getScenario(context, scenario, object1, object2);
        boolean result = false;

        while (null != scen) {

            try {
                int width = Integer.valueOf(scen.get("video_ow") + "");
                int height = Integer.valueOf(scen.get("video_oh") + "");
                mTool.log('d', "Output size:" + width + "x" + height);
                mMediaPlayer = new EffectMediaPlayer(width, height);
            } catch (NumberFormatException e) {
                mTool.log('w', "Output Size Error " + e.getMessage());
                break;
            }

            mMediaPlayer.setEffect(EffectMediaPlayer.FILTER_VIDEO_TRANSITION);
            mMediaPlayer.setResourceContext(context, "");
            mMediaPlayer.setVideoScenario(scen);
            mMediaPlayer.setRecordingPath("/sdcard/save.mp4", videoProfile);
            mMediaPlayer.setIgnoreMainFrameStreem(true);

            mTool.log('d', "videoProfile.duration:" + videoProfile.duration);
            mTool.log('d', "videoProfile.quality:" + videoProfile.quality);
            mTool.log('d', "videoProfile.fileFormat:" + videoProfile.fileFormat);
            mTool.log('d', "videoProfile.videoCodec:" + videoProfile.videoCodec);
            mTool.log('d', "videoProfile.videoBitRate:" + videoProfile.videoBitRate);
            mTool.log('d', "videoProfile.videoFrameRate:" + videoProfile.videoFrameRate);
            mTool.log('d', "videoProfile.videoFrameWidth:" + videoProfile.videoFrameWidth);
            mTool.log('d', "videoProfile.videoFrameHeight:" + videoProfile.videoFrameHeight);
            mTool.log('d', "videoProfile.audioCodec:" + videoProfile.audioCodec);
            mTool.log('d', "videoProfile.audioBitRate:" + videoProfile.audioBitRate);
            mTool.log('d', "videoProfile.audioSampleRate:" + videoProfile.audioSampleRate);
            mTool.log('d', "videoProfile.audioChannels:" + videoProfile.audioChannels);
  
            if (scen.containsKey("video1")) {
                mTool.log('d', "video1:" + scen.get("video1"));
                try {
                    mMediaPlayer.setDataSource(scen.get("video1") + "");
                    result = true;
                } catch (IOException e) {
                    mTool.log('w', "IOException " + e.getMessage());
                } catch (IllegalArgumentException e) {
                    mTool.log('w', "IllegalArgumentException " + e.getMessage());
                } catch (SecurityException e) {
                    mTool.log('w', "SecurityException " + e.getMessage());
                }
            }

            break;
        }
        return result;
    }

    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;

    private synchronized boolean playVideoSetup(EffectMediaPlayer mp) {

        if (mp == null)
            return false;

        mp.setAudioStreamType(AudioManager.STREAM_MUSIC);

        boolean result = true;

        mp.setLooping(true);
        mp.setVolume(0, 0);
        mp.setSurface(null);

        mp.setProcessDoneCallBack(new ProcessDoneListener() {
            @Override
            public void onProcessDone(Object info) {
                synchronized (VideoScenarioEffect.this) {
                    mTool.log('d', "Process done !");
                    processResult = (info != null) ? true : false;
                    VideoScenarioEffect.this.notifyAll();
                }
            }
        });

        mp.setOnVideoSizeChangedListener(new MediaPlayer.OnVideoSizeChangedListener() {
            public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
                mTool.log('d', "MediaPlayer sent dimensions: " + width + " x " + height);
            }
        });
        mp.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            public void onPrepared(MediaPlayer mp) {
                mTool.log('d', "MediaPlayer is prepared");
            }
        });
        mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            public void onCompletion(MediaPlayer arg0) {
                mTool.log('d', "onCompletion");
            }
        });

        mp.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);

        try {
            mTool.log('d', "MediaPlayer.prepared()");
            mp.prepare();
            mTool.log('d', "MediaPlayer.start() ");
            mp.start();
        } catch (IOException e) {
            mTool.log('w', "IOException " + e.getMessage());
            result = false;
        } catch (IllegalStateException e) {
            mTool.log('w', "IllegalStateException " + e.getMessage());
            result = false;
        }

        return result;
    }

    private boolean processResult;

    public boolean process() {
        boolean result = false;
        EffectMediaPlayer localPlayer = null;

        synchronized (this) {
            mTool.log('d', "process()");

            processResult = false;
            processResult = playVideoSetup(mMediaPlayer);

            try {
                if (processResult) {
                    mTool.log('d', "wait for result");
                    this.wait();
                }
            } catch (InterruptedException e) {
                mTool.log('w', "InterruptedException: " + e.getMessage());
                localPlayer = mMediaPlayer;
                mMediaPlayer = null;
                processResult = false;
            }

            result = processResult;
        }

        // process InterruptException without synchronized        
        if (localPlayer != null) {
            localPlayer.stop();
            localPlayer.setProcessDoneCallBack(null);
            localPlayer.release();
        }

        mTool.log('d', "process() = " + result);
        return result;
    }

    public synchronized void cancel() {
        mTool.log('w', "cancel() !");
        if (mMediaPlayer != null) {
            mMediaPlayer.stop();
            mMediaPlayer.setProcessDoneCallBack(null);
            mMediaPlayer.release();
            mMediaPlayer = null;
        } else {
            mTool.log('d', "No context to be released ! ignored");
            return;
        }
        processResult = false;
        mTool.log('d', "notifyAll() !");
        VideoScenarioEffect.this.notifyAll();
    }
}
