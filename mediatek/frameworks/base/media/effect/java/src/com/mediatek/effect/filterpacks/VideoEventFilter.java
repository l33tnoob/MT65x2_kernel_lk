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

package com.mediatek.effect.filterpacks;

import java.util.Arrays;
import java.util.HashMap;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.ProcessDoneListener;
import com.mediatek.effect.filterpacks.io.MediaSourceInFilter;
import com.mediatek.effect.filterpacks.ve.VideoEvent;
import com.mediatek.effect.filterpacks.ve.VideoEventStill;
import com.mediatek.effect.filterpacks.ve.VideoScenario;
import com.mediatek.effect.player.EffectCore;
import com.mediatek.effect.player.EffectPlayer;

import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.GLFrame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.format.ImageFormat;
import android.opengl.GLES20;
import android.os.SystemClock;

/**
 * @hide
 */
public class VideoEventFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    @GenerateFieldPort(name = "processDoneListener", hasDefault = true)
    private ProcessDoneListener mProcessDoneListener = null;

    @GenerateFieldPort(name = "mediasourcefilter", hasDefault = true)
    private MediaSourceInFilter mMediaSourceFilter = null;

    @GenerateFieldPort(name = "effectplayer", hasDefault = true)
    private EffectPlayer mEffectPlayer = null;

    private VideoScenario mScenario = new VideoScenario();

    private final String mInputPortCamera = "image";
    private final String mInputPortVideo = "video";
    private final String mOutputPort = "image";

    private GLFrame mCamera;
    private GLFrame mVideo;

    private int mFrameCount;
    private long startTime = -1;

    private boolean mGotFirstTimeStamp = false;
    private long mFirstTimeStamp = 0;
    private long mNow;
    private long mEndtime = Long.MAX_VALUE;

    private Object[] mKeys = {""};

    public VideoEventFilter(String name) {
        super(name);
        mTool.log('d', getClass().getSimpleName() + "() " + name);
        mTool.setIDandIncrease(mCount);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    @Override
    public void tearDown(FilterContext context) {
        super.tearDown(context);
    }

    @Override
    public void setupPorts() {
        FrameFormat imageFormat = ImageFormat.create(ImageFormat.COLORSPACE_RGBA, FrameFormat.TARGET_GPU);

        addMaskedInputPort(mInputPortCamera, imageFormat);
        addMaskedInputPort(mInputPortVideo, imageFormat);

        addOutputBasedOnInput(mOutputPort, mInputPortCamera);
    }

    @Override
    public FrameFormat getOutputFormat(String portName, FrameFormat inputFormat) {
        MutableFrameFormat format = inputFormat.mutableCopy();
        return format;
    }

    @Override
    public void prepare(FilterContext context) {
        mTool.log('d', "Preparing " + getClass().getSimpleName() + "!");

    }

    @Override
    public void open(FilterContext context) {
        mTool.log('d', "open() " + context);

        if (null != mScenario) {
            mKeys = mScenario.keySet().toArray();
            Arrays.sort(mKeys);

            int orientation = 0;
            if (mScenario.containsKey("orientation")) {
                try {
                    orientation = Integer.parseInt(mScenario.get("orientation") + "");
                } catch (NumberFormatException e) {
                    orientation = 0;
                }
            }

            mTool.log('d', "open() Total Event: " + mKeys.length);
            for (Object key : mKeys) {
                Object obj = mScenario.get(key);
                if (obj instanceof VideoEvent) {
                    VideoEvent event = (VideoEvent) obj;
                    event.setOrientation(orientation);
                    mTool.log('d', key + ": " + event.getStartTime() + "~" + event.getEndTime());
                    if (null != mMediaSourceFilter) {
                        event.put("mediasource", mMediaSourceFilter);
                    }
                    event.open(context, this);
                }
            }
        }
        mGotFirstTimeStamp = false;
        
        if (mScenario.containsKey("THEENDTIME")) {
            mEndtime = (Long) mScenario.get("THEENDTIME");
        }
    }

    @Override
    public void process(FilterContext context) {
        mCamera = (GLFrame)pullInput(mInputPortCamera);
        mVideo = (GLFrame)pullInput(mInputPortVideo);
        GLFrame output;

        if (mScenario.containsKey("IgnoreMainFrame")) {
            output = (GLFrame)context.getFrameManager().newFrame(mVideo.getFormat());
        } else {
            output = (GLFrame)context.getFrameManager().newFrame(mCamera.getFormat());
        }

        if (false == mGotFirstTimeStamp) {
            mFirstTimeStamp = mCamera.getTimestamp();
            mGotFirstTimeStamp = true;
        }

        mNow = mCamera.getTimestamp() - mFirstTimeStamp;
        boolean result = false;

        if (mEndtime < mNow) {
            mTool.log('w', mNow + " End Time:" + mEndtime);

            // make sure the final frame is the correct one
            for (Object key : mKeys) {
                Object obj = mScenario.get(key);
                if (obj instanceof VideoEventStill) {
                    VideoEvent event = (VideoEvent) obj;
                    if (event.getStartTime() <= mNow) {
                        mTool.log('d', "VideoEventStill.process()");
                        event.process(context, this, result, output);
                        pushOutput(mOutputPort, output);
                        output.release();
                        break;
                    }
                }
            }

            if (mEffectPlayer != null) {
                mEffectPlayer.stop();
            }
            if (mMediaSourceFilter != null) {
                mMediaSourceFilter.pauseVideo(true);
            }
            return;
        }

        if (null != mScenario) {
            VideoEvent event;
            for (Object key : mKeys) {
                Object obj = mScenario.get(key);
                if (obj instanceof VideoEvent) {
                    event = (VideoEvent) obj;
                    if (event.getStartTime() <= mNow && mNow < event.getEndTime()) {
                        boolean b = event.process(context, this, result, output);
                        result = result || b;
                    }
                }
            }
        }

        if (result) {
            pushOutput(mOutputPort, output);
        } else {
            mTool.log('w', mNow + " no VideoEvent process() !");
            pushOutput(mOutputPort, mCamera);
        }

        output.release();

        mFrameCount++;
        printFPS(context);
    }

    @Override
    public void close(FilterContext context) {
        mTool.log('d', "Filter Closing!");

        if (null != mScenario) {
            VideoEvent event;
            for (Object key : mKeys) {
                Object obj = mScenario.get(key);
                if (obj instanceof VideoEvent) {
                    event = (VideoEvent) obj;
                    event.close(context, this);
                }
            }
        }
    }

    private void printFPS(FilterContext context) {
        if (mFrameCount % 30 == 0) {
            if (startTime == -1) {
                context.getGLEnvironment().activate();
                GLES20.glFinish();
                startTime = SystemClock.elapsedRealtime();
            } else {
                context.getGLEnvironment().activate();
                GLES20.glFinish();
                long endTime = SystemClock.elapsedRealtime();
                mTool.log('d', "Avg. frame duration: " + String.format("%.2f", (endTime - startTime) / 30.)
                    + " ms. Avg. fps: " + String.format("%.2f", 1000. / ((endTime - startTime) / 30.)));
                startTime = endTime;
            }
        }
    }

    public GLFrame getInputCameraGLFrame() {
        return mCamera;
    }

    public GLFrame getInputVideoGLFrame() {
        return mVideo;
    }

    public long getFristTimeStamp() {
        if (false == mGotFirstTimeStamp) {
            return 0;
        }
        return mFirstTimeStamp;
    }

    public long getNowTimeStamp() {
        return mNow;
    }

    public VideoScenario getScenario() {
        return mScenario;
    }

    public void setScenario(VideoScenario scenario) {
        mScenario.clear();
        mScenario = scenario;
    }
}

