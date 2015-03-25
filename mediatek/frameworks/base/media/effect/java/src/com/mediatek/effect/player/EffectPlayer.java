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

package com.mediatek.effect.player;

import java.util.HashMap;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.ProcessDoneListener;
import com.mediatek.effect.filterpacks.ve.VideoEvent;
import com.mediatek.effect.filterpacks.ve.VideoScenario;

import android.content.Context;
import android.filterfw.core.FilterSurfaceView;
import android.media.CamcorderProfile;
import android.media.MediaPlayer;
import android.view.TextureView;

/**
 * @hide
 */
public class EffectPlayer extends MediaPlayer {
    protected static int[] mCount = {0};
    protected MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    protected int mWidth = 0;
    protected int mHeight = 0;

    protected EffectCore mEffectGraphCore = null;
    protected boolean isStartThreadRunning = false;

    // normal maximum
    public static final int MAX_NORMAL_FILTER_EFFECT = EffectCore.MAX_NORMAL_FILTER_EFFECT;

    // should be equal to the latest effect item
    public static final int MAX_FILTER_EFFECT        = EffectCore.MAX_FILTER_EFFECT;

    // special filter !!
    public static final int FILTER_RANDOMIZE_EFFECT  = EffectCore.FILTER_RANDOMIZE_EFFECT;
    public static final int FILTER_VIDEO_TRANSITION  = EffectCore.FILTER_VIDEO_TRANSITION;
    public static final int FILTER_PROCESS_MAX       = EffectCore.FILTER_PROCESS_MAX;

    public EffectPlayer(int width, int height) {
        super();
        mTool.log('d', getClass().getSimpleName() + "() " + width + "x" + height);
        mTool.setIDandIncrease(mCount);

        mWidth = width;
        mHeight = height;

        mEffectGraphCore = new EffectCore(width, height, this);
    }

    public int getId() {
        return mTool.id;
    }

    public void submit(Runnable task) {
        if (null != mEffectGraphCore) {
            mEffectGraphCore.submit(task);
        }
    }

    @Override
    public void finalize() {
        mTool.log('d', "~" + getClass().getSimpleName() + "() " + mWidth + "x" + mHeight);
        release();
        super.finalize();
    }

    public static boolean isSupport(int effect) {
        return EffectCore.isSupport(effect);
    }

    public static boolean isGMSGoofySupport() {
        return isSupport(EffectCore.FILTER_GF_SMALL_EYES);
    }

    public void setOutputSurfaceView(FilterSurfaceView sv) {
        if (mEffectGraphCore != null)
            mEffectGraphCore.setOutputSurfaceView(sv);
    }

    public void setOutputTextureView(TextureView tv) {
        if (mEffectGraphCore != null)
            mEffectGraphCore.setOutputTextureView(tv);
    }

    public void setOutputTextureViewWithoutEffect(TextureView tv) {
        if (mEffectGraphCore != null)
            mEffectGraphCore.setOutputTextureViewWithoutEffect(tv);
    }

    public void setHandler(EffectUiHandler tv) {
        if (mEffectGraphCore != null)
            mEffectGraphCore.setHandler(tv);
    }

    public void setProcessDoneCallBack(ProcessDoneListener callback) {
        if (mEffectGraphCore != null)
            mEffectGraphCore.setProcessDoneCallBack(callback);
    }


    public void setRecordingPath(String path, CamcorderProfile profile) {
        if (null != mEffectGraphCore) {
            mEffectGraphCore.setRecordingPath(path, profile);
        }
    }

    public String getGraphEffectName() {
        if (null != mEffectGraphCore)
            return mEffectGraphCore.getGraphEffectName();
        return null;
    }

    public static String getGraphEffectName(int i) {
        return EffectCore.getGraphEffectName(i);
    }

    public boolean setEffect(int effect) {
        mTool.log('d', "setEffect (" + effect + ")");
        return mEffectGraphCore.setEffect(effect);
    }

    public void setIgnoreMainFrameStreem(boolean ignore) {
        if (null != mEffectGraphCore) {
            mEffectGraphCore.setIgnoreMainFrameStreem(ignore);
        }
    }

    protected Context mCntx;
    protected String mEffectVideoUri;

    public void setResourceContext(Context cntx, String video) {
        mCntx = cntx;
        mEffectVideoUri = video;
    }

    public void setVideoScenario(VideoScenario scenario) {
        mTool.log('d', "setScenario() " + scenario);

        if (mEffectGraphCore != null) {
            mEffectGraphCore.setScenario(scenario);
        }
    }

    public void graphClose() {
        if (mEffectGraphCore != null) {
            mEffectGraphCore.graphClose();
        }
    }

    @Override
    public synchronized void stop() {
        if (this.isPlaying())
            super.stop();
    }

    @Override
    public synchronized void release() {
        mTool.log('d', "release()");

        while (isStartThreadRunning == true) {
            mTool.log('d', "waiting for StartThread released !! " + "isStartThreadRunning: " + isStartThreadRunning);
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mEffectGraphCore != null) {
            mEffectGraphCore.graphClose();
            mEffectGraphCore.graphCleanResource();
            mEffectGraphCore = null;
        }
        super.release();
    }

}
