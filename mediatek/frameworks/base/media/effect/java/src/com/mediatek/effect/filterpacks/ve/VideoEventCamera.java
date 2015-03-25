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

import com.mediatek.effect.filterpacks.VideoEventFilter;

import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;

/**
 * @hide
 */
public class VideoEventCamera extends VideoEvent {
    // Background subtraction shader. Uses a mipmap of the binary mask map to blend smoothly between
    //   foreground and background
    // Inputs:
    //   tex_sampler_0: Foreground (live) video frame.
    //   tex_sampler_1: Background (playback) video frame.
    private static String mOverlayShader =
        "precision mediump float;\n" +
        "uniform sampler2D tex_sampler_0;\n" +
        "uniform sampler2D tex_sampler_1;\n" +
        "uniform float center_r;\n" +
        "varying vec2 v_texcoord;\n" +
        "void main() {\n" +
        "  vec2 coord = v_texcoord - vec2(0.5);\n" +
        "  vec4 original = texture2D(tex_sampler_0, v_texcoord);\n" +
        "  vec4 mask = texture2D(tex_sampler_1, v_texcoord);\n" +
        "  float rrr = length(coord);\n" +
        "  if (0.0 <= center_r) {" +
        "    mask = vec4(0.0);\n" +
        "  }\n" +
        "  if (rrr > center_r) {\n" +
        "    original = vec4(0.0);\n" +
        "  }\n" +
        "  gl_FragColor = original + mask;\n" +
        //"  if ((v_texcoord.x > 0.95 || v_texcoord.y > 0.95) || (v_texcoord.x < 0.05 || v_texcoord.y < 0.05)) {\n" +
        //"    gl_FragColor = edge;\n" +
        //"  }\n" +
        "}\n";

    private ShaderProgram mCopyProgram;
    private GLFrame mPreviousFrame;
    private boolean mIsGotPreviousFrame;

    public VideoEventCamera(String name, long start, long end) {
        super("Camera", name, start, end);
    }

    @Override
    public void open(FilterContext context, VideoEventFilter myfilter) {
        super.open(context, myfilter);
        mCopyProgram = new ShaderProgram(context, mOverlayShader);
        mCopyProgram.setHostValue("center_r", 0.0f);

        mIsGotPreviousFrame = false;
    }

    @Override
    public boolean process(FilterContext context, VideoEventFilter myfilter, boolean isRenderOutput, GLFrame output) {
        super.process(context, myfilter, isRenderOutput, output);

        GLFrame camera = myfilter.getInputCameraGLFrame();

        FrameFormat inputFormat;
        inputFormat = camera.getFormat();

        if (null == mMainFrame) {
            MutableFrameFormat outputFormat = inputFormat.mutableCopy();
            mMainFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
            mMainFrame.focus();
            mCopyProgramWithColor.process(camera, mMainFrame);
        }

        if (null == mPreviousFrame) {
            MutableFrameFormat outputFormat = inputFormat.mutableCopy();
            mPreviousFrame = (GLFrame) context.getFrameManager().newFrame(outputFormat);
            mPreviousFrame.focus();
        }

        Frame[] subtractInputs = {camera, mPreviousFrame};

        long currentTimeStamp = myfilter.getNowTimeStamp();
        long cameraPhoto;

        if (this.containsKey("camera_photo")) {
            try {
                cameraPhoto = ((Long) this.get("camera_photo")).longValue() + mStart;
            } catch (ClassCastException e) {
                e.printStackTrace();
                return false;
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
                subtractInputs[0] = mMainFrame;
            }

            if (false == mIsGotPreviousFrame ||
                true == mIsGotPreviousFrame && currentTimeStamp < mEffectStart) {
                mPreviousFrame.focus();
                if (isRenderOutput) {
                    mCopyProgramWithColor.process(output, mPreviousFrame);
                } else {
                    mCopyProgramWithColor.process(camera, mPreviousFrame);
                }
                mIsGotPreviousFrame = true;
                mTool.log('d', "Got PreviousInput:" + currentTimeStamp);
            }

            // finished, no need to do
            if (currentTimeStamp >= mEffectEnd || currentTimeStamp < mEffectStart) {
                return false;
            }

            float center_r = 0.0f;

            if (cameraPhoto >= currentTimeStamp) {
                center_r = 1.0f - (float) (currentTimeStamp - mEffectStart) / (float) (cameraPhoto - mEffectStart);
                subtractInputs[0] = mPreviousFrame;
            } else {
                center_r = (float) (currentTimeStamp - cameraPhoto) / (float) (mEffectEnd - cameraPhoto);
                subtractInputs[0] = mMainFrame;
            }

            mCopyProgram.setHostValue("center_r", center_r);

            if (null != output && isRenderOutput == false) {
                mCopyProgram.process(subtractInputs, output);
            }
            return true;
        }
        return false;
    }

    @Override
    public void close(FilterContext context, VideoEventFilter myfilter) {
        super.close(context, myfilter);

        if (null != mPreviousFrame) {
            mPreviousFrame.release();
            mPreviousFrame = null;
        }
    }
}
