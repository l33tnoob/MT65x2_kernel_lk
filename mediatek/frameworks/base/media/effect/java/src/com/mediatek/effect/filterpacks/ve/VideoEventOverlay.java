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

package com.mediatek.effect.filterpacks.ve;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.VideoEventFilter;
import com.mediatek.effect.filterpacks.io.MediaSourceInFilter;

import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;
import android.graphics.Bitmap;

/**
 * @hide
 */
public class VideoEventOverlay extends VideoEvent {
    // Background subtraction shader. Uses a mipmap of the binary mask map to blend smoothly between
    //   foreground and background
    // Inputs:
    //   tex_sampler_0: Foreground (live) video frame.
    //   tex_sampler_1: Background (playback) video frame.
    private static String mOverlayShader =
        "precision mediump float;\n" +
        "uniform sampler2D tex_sampler_0;\n" +
        "uniform sampler2D tex_sampler_1;\n" +
        "uniform sampler2D tex_sampler_2;\n" +
        "uniform float fgfactor;\n" +
        "uniform float bgfactor;\n" +
        "uniform mat3 matrixroll;\n" +
        "varying vec2 v_texcoord;\n" +
        "void main() {\n" +
        "  vec4 bgcolor = texture2D(tex_sampler_1, v_texcoord);\n" +
        "  vec2 coord = (matrixroll * vec3(v_texcoord, 1.0)).xy;\n" +
        "  vec2 coordinside;\n" +
        "    coordinside.x = ((coord.x-0.5) * 1320.0 / 1280.0) + 0.5;\n" +
        "    coordinside.y = ((coord.y-0.5) * 760.0 / 720.0) + 0.5;\n" +
        "  vec4 original = texture2D(tex_sampler_0, coordinside);\n" +
        "  vec4 edge = texture2D(tex_sampler_2, coord);\n" +
        "  if ((coordinside.x > 1.0 || coordinside.y > 1.0) ||\n" +
        "      (coordinside.x < 0.0 || coordinside.y < 0.0)) {\n" +
        "    original = mix(bgcolor * bgfactor, original, edge.a);\n" +
        "  }\n" +
        "  original = mix(original, edge, smoothstep(0.0, 1.0, edge.a));\n" +
        "  gl_FragColor = mix(bgcolor * bgfactor, original, fgfactor);\n" +
        "}\n";

    private ShaderProgram mMergeProgram;
    private GLFrame mEdgeFrame;
    private GLFrame mStillBgFrame;
    private GLFrame mPreviousFrame;
    private boolean mIsSeekToBegin;
    private MediaSourceInFilter mMediaPlayer;
    private boolean mIsWaitFirstFrame;
    private int mInitOffsetTime = 0;

    public VideoEventOverlay(String name, long start, long end) {
        super("Overlay", name, start, end);
    }

    @Override
    public void open(FilterContext context, VideoEventFilter myfilter) {
        super.open(context, myfilter);
        mMergeProgram = new ShaderProgram(context, mOverlayShader);

        mInitOffsetTime = 0;

        if (this.get("background").toString().contains("video")) {
            if (this.containsKey("background_initoffsettime")) {
                mInitOffsetTime = Integer.parseInt(this.get("background_initoffsettime") + "");
            }
            mTool.log('d', " video2 Init Offset Time: " + mInitOffsetTime);

            if (this.containsKey("mediasource")) {
                mMediaPlayer = (MediaSourceInFilter) this.get("mediasource");
                mMediaPlayer.pauseVideo(true);
                mMediaPlayer.seekTo(mInitOffsetTime);
                mTool.log('d', " mMediaPlayer.pause()");
            }
        }

        mIsSeekToBegin = false;
        mIsWaitFirstFrame = false;
    }

    private long getRelatedTimeStamp(long time, long offset) {
        long result = time - offset * 1000000L;
        return result;
    }

    @Override
    public boolean process(FilterContext context, VideoEventFilter myfilter, boolean isRenderOutput, GLFrame output) {
        super.process(context, myfilter, isRenderOutput, output);

        GLFrame camera = myfilter.getInputCameraGLFrame();
        GLFrame effectVideo = myfilter.getInputVideoGLFrame();

        FrameFormat inputFormat;
        inputFormat = effectVideo.getFormat();//camera.getFormat();

        if (null == mMainFrame) {
            MutableFrameFormat outputFormat = inputFormat.mutableCopy();
            mMainFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
            mMainFrame.focus();
            mCopyProgramWithColor.process(effectVideo, mMainFrame);
        }

        if (null == mEdgeFrame) {
            if (this.containsKey("edge")) {
                Bitmap bitmap = (Bitmap) this.get("edge");
                mEdgeFrame = MyUtility.createBitmapFrame(context, bitmap);
            } else {
                MutableFrameFormat outputFormat = inputFormat.mutableCopy();
                mEdgeFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
                mEdgeFrame.focus();
                mColor[3] = 0.0f;
                mColor[0] = 1.0f;
                mCopyProgramWithColor.setHostValue("ccc", mColor);
                mCopyProgramWithColor.process(effectVideo, mEdgeFrame);
            }
        }

        if (null == mStillBgFrame) {
            if (this.containsKey("background_still") && this.get("background") instanceof Bitmap) {
                mStillBgFrame = MyUtility.createBitmapFrame(context, (Bitmap)this.get("background"));
                mTool.log('d', "mStillBgFrame(b): bitmap");
            } else {
                MutableFrameFormat outputFormat = inputFormat.mutableCopy();
                mStillBgFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
                mStillBgFrame.focus();
                mColor[3] = 1.0f;
                mColor[0] = 1.0f;
                mCopyProgramWithColor.setHostValue("ccc", mColor);
                mCopyProgramWithColor.process(effectVideo, mStillBgFrame);
                mTool.log('d', "mStillBgFrame(b):" + getRelatedTimeStamp(effectVideo.getTimestamp(), mInitOffsetTime));
            }
        }

        if (null == mPreviousFrame) {
            MutableFrameFormat outputFormat = inputFormat.mutableCopy();
            mPreviousFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
            mPreviousFrame.focus();
            mColor[3] = 1.0f;
            mColor[0] = 1.0f;
            mCopyProgramWithColor.setHostValue("ccc", mColor);
            mCopyProgramWithColor.process(effectVideo, mPreviousFrame);
            mTool.log('d', "mPreviousFrame:" + getRelatedTimeStamp(effectVideo.getTimestamp(), mInitOffsetTime));
        }

        if (isRenderOutput) {
            mPreviousFrame.focus();
            mColor[3] = 0.0f;
            mCopyProgramWithColor.setHostValue("ccc", mColor);
            mCopyProgramWithColor.process(output, mPreviousFrame);
            mTool.log('d', "mPreviousFrame:" + myfilter.getNowTimeStamp());
        }

        Frame[] subtractInputs = {mMainFrame, effectVideo, mEdgeFrame};

        long currentTimeStamp = myfilter.getNowTimeStamp();
        long cameraPhoto = mStart;

        if (this.containsKey("move_photo")) {
            try {
                cameraPhoto = ((Long) this.get("move_photo")).longValue() + mStart;
            } catch (ClassCastException e) {
                e.printStackTrace();
                return false;
            }
        }

        // get fix frame
        if (currentTimeStamp >= cameraPhoto) {
            if (false == mGotMainFrame) {
                mMainFrame.focus();
                mColor[3] = 1.0f;
                mCopyProgramWithColor.setHostValue("ccc", mColor);
                mCopyProgramWithColor.process(camera, mMainFrame);
                mGotMainFrame = true;
                mTool.log('d', "Got CameraInput:" + currentTimeStamp);
            }
        }

        if (mEffectStart > currentTimeStamp) {
            mIsSeekToBegin = false;
            if (null != mMediaPlayer) {
                mTool.log('d', currentTimeStamp + " mMediaPlayer.seekTo(0) pause()");
                mMediaPlayer.seekTo(mInitOffsetTime);
                mMediaPlayer.pauseVideo(true);
            }
        }

        if (mEffectStart <= currentTimeStamp && false == mIsSeekToBegin) {
            if (this.containsKey("background_still") && this.get("background") instanceof Bitmap) {
                // ignore the background video
            } else {
                if (null != mMediaPlayer) {
                    mTool.log('d', currentTimeStamp + " mMediaPlayer.seekTo(0) play()");
                    mMediaPlayer.seekTo(mInitOffsetTime);
                    if (!this.containsKey("background_still")) {
                        mMediaPlayer.pauseVideo(false);
                    }
                }
                mIsSeekToBegin = true;
                mIsWaitFirstFrame = true;
            }
        }

        // finished, no need to do
        if (currentTimeStamp >= mEffectEnd || currentTimeStamp < mEffectStart) {
            mTool.log('d', "now:" + currentTimeStamp + " do nothing ! ["
                + mEffectStart + "~" + mEffectEnd + "]");
            return false;
        }

        if (this.containsKey("background_still") && this.get("background") instanceof Bitmap) {
            // ignore the background video and do not get the frame
        } else {
            if (mIsWaitFirstFrame == true && currentTimeStamp > mEffectStart + 100000000L) {
                mIsWaitFirstFrame = false;
            } else if (mIsWaitFirstFrame == true && getRelatedTimeStamp(effectVideo.getTimestamp(), mInitOffsetTime) > 500000000L) {
                if (null != output && isRenderOutput == false) {
                    mPreviousFrame.focus();
                    mColor[3] = 1.0f;
                    mCopyProgramWithColor.setHostValue("ccc", mColor);
                    mCopyProgramWithColor.process(mPreviousFrame, output);
                    mTool.log('d', "show mPreviousFrame @ " + currentTimeStamp);
                    return true;
                }
                return false;
            } else {
                mIsWaitFirstFrame = false;
            }
        }

        if (this.containsKey("background_still")) {
            subtractInputs[1] = mStillBgFrame;
        }

        // The following matrix is the transpose of the actual matrix
        long tickStep = (currentTimeStamp - mEffectStart) % (mEffectEnd - mEffectStart + 1);
        tickStep = tickStep / 1000000L;

        float fgFactor = 1.0f;
        if (this.containsKey("bitmap_fadeout")) {
            fgFactor = fgFactor - (float) ((tickStep) * (1.0 / (float) ((mEffectEnd - mEffectStart) / 1000000L)));
        }
        float bgFactor = 1.0f;
        if (this.containsKey("background_fadein")) {
            bgFactor = (float) ((tickStep) * (1.0 / (float) ((mEffectEnd - mEffectStart) / 1000000L)));
        }

        if (!this.containsKey("bitmap_move")) {
            tickStep = (mEffectEnd - mEffectStart) / 1000000L;
        }

        float tick = (float) ((tickStep) * (1.0 / (float) ((mEffectEnd - mEffectStart) / 1000000L)));

        float width = inputFormat.getWidth();
        float height = inputFormat.getHeight();
        float inputAspectRatio = width / height;

        float xStep;
        float yStep;

        float xLocation0 = 150.0f / 1280.0f;
        float yLocation0 = 575.0f / 720.0f;

        float xLocation90 = 230.0f / 1280.0f;
        float yLocation90 = 675.0f / 720.0f;

        if (this.containsKey("x")) {
            xLocation0 = Float.parseFloat(this.get("x") + "");
            xLocation90 = Float.parseFloat(this.get("x") + "");
        }

        if (this.containsKey("y")) {
            yLocation0 = Float.parseFloat(this.get("y") + "");
            yLocation90 = Float.parseFloat(this.get("y") + "");
        }

        switch (mOrientation) {
            case 90:
                xStep = xLocation90 - 0.5f;
                yStep = 0.5f - yLocation90;
                break;

            case 180:
                xStep = xLocation0 - 0.5f;
                yStep = yLocation0 - 0.5f;
                break;

            case 270:
                xStep = 0.5f - xLocation90;
                yStep = yLocation90 - 0.5f;
                break;

            default:
                xStep = 0.5f - xLocation0;
                yStep = 0.5f - yLocation0;
                break;
        }


        xStep = xStep * tick;
        yStep = yStep * tick;

        float percent = 0.345f;

        if (this.containsKey("scale")) {
            percent = Float.parseFloat(this.get("scale") + "");
        }

        float sizeRatio = 1.f + (tick * (1 - percent) / percent);

        //mTool.log('d', "tick: " + tick + ", tStep:" + tickStep + ", sizeR:" + sizeRatio);

        float angel = (float) ((float)tickStep * (1.0 / (float) ((mEffectEnd - mEffectStart)/1000000L)));
        float finalangel = 6.0f;
        angel = (float) (((angel / 180.f) * Math.PI) * finalangel);

        float cosval = (float) (Math.cos(angel) * sizeRatio);
        float sinval = (float) (Math.sin(angel) * sizeRatio);

        /*
         * K = AspectRatio = W/H
         * A(to texture)    B(roll)            C(to axle-xy (0, 0))
         *   1,   0,  0      cos,  sin,  0        1,      0,  0
         *   0,   K,  0     -sin,  cos,  0        0,    1/K,  0
         * 0.5, 0.5,  1         0,   0,  1     -0.5, -0.5/K,  1
         *
         * R*v = ABC*v, optimized to one matrix
         * v*R = v*CBA, optimized to one matrix
         */

        /*float[][] CC = {
            {1.0f,   0.0f,  0.0f},
            {0.0f,   1.0f / inputAspectRatio,  0.0f},
            {-0.5f, -0.5f / inputAspectRatio,  1.0f}};

        float[][] BB = {
            {(float) cosval,  (float) sinval,  .0f},
            {(float) -sinval, (float) cosval,  .0f},
              {0.0f,  0.0f,  1.0f}};

        float[][] AA = {
            {1.0f,  0.0f, 0.0f},
            {0.0f,  1.0f * inputAspectRatio, 0.0f},
            {(xStep) * (sizeRatio) + 0.5f, (yStep) * sizeRatio + 0.5f, 1.0f}};

        float[][] RR = MyUtility.MatrixMultiply(CC, BB);
        RR = MyUtility.MatrixMultiply(RR, AA);
        float[] rollTransform = MyUtility.MatrixToOneWay(RR);*/
        
        /*
         * C:=array([[1,0,0],[0,1/r,0],[-0.5,-0.5/r,1]]):
         * B:=array([[c,s,0],[-s,c,0],[0,0,1]]):
         * A:=array([[1,0,0],[0,r,0],[a+0.5,b+0.5,1]]):
         * multiply(C,B,A);
         * ==>
         * c,               sr,             0
         * -s/r,            c,              0
         * 0.5*(1-c+s/r)+a, 0.5*(1-c-sr)+b, 1
         * 
         */
        
        float[] rollTransform = {
            cosval, (sinval * inputAspectRatio), 0.0f,
            (-sinval / inputAspectRatio), cosval, 0.0f,
            ((1.0f - cosval + sinval / inputAspectRatio) * 0.5f + (xStep) * (sizeRatio)),
            ((1.0f - cosval - sinval * inputAspectRatio) * 0.5f + (yStep) * (sizeRatio)), 1.0f
        };

        mMergeProgram.setHostValue("fgfactor", fgFactor);
        mMergeProgram.setHostValue("bgfactor", bgFactor);
        mMergeProgram.setHostValue("matrixroll", rollTransform);

        if (null != output && isRenderOutput == false) {
            mMergeProgram.process(subtractInputs, output);
        }

        return true;
    }

    @Override
    public void close(FilterContext context, VideoEventFilter myfilter) {
        super.close(context, myfilter);

        if (null != mEdgeFrame) {
            mEdgeFrame.release();
            mEdgeFrame = null;
        }

        if (null != mStillBgFrame) {
            mStillBgFrame.release();
            mStillBgFrame = null;
        }

        if (null != mPreviousFrame) {
            mPreviousFrame.release();
            mPreviousFrame = null;
        }
    }
}
