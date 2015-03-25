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

import com.mediatek.effect.filterpacks.Stereo3DAntiFatigueFilter;

import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.FrameManager;
import android.filterfw.core.GLFrame;
import android.filterfw.format.ImageFormat;
import android.graphics.Bitmap;
import android.media.effect.EffectContext;
import android.opengl.GLES20;

public class Stereo3DAntiFatigueEffect extends CallBackEffect {
    public Stereo3DAntiFatigueEffect(EffectContext context, String name) {
        super(context, name, Stereo3DAntiFatigueFilter.class, "image", "image");
        setParameter("effectclass", this);
    }

    private Frame getDummyFrame(int width, int height) {
        FrameManager manager = getFilterContext().getFrameManager();
        FrameFormat format = ImageFormat.create(width, height,
                                                ImageFormat.COLORSPACE_RGBA,
                                                FrameFormat.TARGET_GPU);
        Frame frame = manager.newFrame(format);
        frame.setTimestamp(Frame.TIMESTAMP_UNKNOWN);
        return frame;
    }

    @Override
    public void apply(int inputTexId, int width, int height, int outputTexId) {
        beginGLEffect();

        Frame inputFrame;
        Frame outputFrame;

        if (GLES20.glIsTexture(inputTexId)) {
            inputFrame = frameFromTexture(inputTexId, width, height);
        } else {
            inputFrame = getDummyFrame(width, height);
        }

        if (GLES20.glIsTexture(outputTexId)) {
            outputFrame = frameFromTexture(outputTexId, width, height);
        } else {
            outputFrame = getDummyFrame(width, height);
        }

        Frame resultFrame = mFunction.executeWithArgList(mInputName, inputFrame);

        if (GLES20.glIsTexture(outputTexId)) {
            outputFrame.setDataFromFrame(resultFrame);
        }

        inputFrame.release();
        outputFrame.release();
        resultFrame.release();

        endGLEffect();
    }

    /**
     * AntiFatigueInfo is the class that calculates the convergence offsets
     * for a stereoscopic image. Convergence refers to the proper alignment of the left
     * and right images. If the 3D image does not produce a clear image,
     * the API can be called to give offsets information for adjusting
     * left and right image positions when rendering on the surface view
     * in order to achieve a clearer image.
     */
    public class AntiFatigueInfo {
        public static final int NUM_OF_INTERVALS = 9;

        public Bitmap mBitmap = null; // will be assigned for output

        public int mLeftOffsetX = 0;
        public int mLeftOffsetY = 0;
        public int mRightOffsetX = 0;
        public int mRightOffsetY = 0;

        // for manual adjustment offset X1
        public int mCroppingOffectX_L[] = new int[NUM_OF_INTERVALS];
        // for manual adjustment offset X2
        public int mCroppingOffectX_R[] = new int[NUM_OF_INTERVALS];

        // index for default position
        public int mCroppingIntervalDefault;

        public int mCroppingSizeWidth[] = new int[NUM_OF_INTERVALS];
        public int mCroppingSizeHeight[] = new int[NUM_OF_INTERVALS];

        // for manual adjustment offset Y1
        public int mCroppingOffectY_L;
        // for manual adjustment offset Y2
        public int mCroppingOffectY_R;

        public AntiFatigueInfo() {

        }

        private void append(StringBuilder result, String member, int[] array) {
            result.append(member + ": " + array[0]);
            for (int i = 1; i < NUM_OF_INTERVALS; i++) {
                result.append(" " + array[i]);
            }
            result.append(System.getProperty("line.separator"));
        }

        public String toString() {
            StringBuilder result = new StringBuilder();
            String newLine = System.getProperty("line.separator");

            result.append(super.toString() + newLine);
            result.append("mBitmap: " + mBitmap + newLine);
            result.append("mLeftOffsetX: " + mLeftOffsetX + newLine);
            result.append("mLeftOffsetY: " + mLeftOffsetY + newLine);
            result.append("mRightOffsetX: " + mRightOffsetX + newLine);
            result.append("mRightOffsetY: " + mRightOffsetY + newLine);

            append(result, "mCroppingOffectX_L", mCroppingOffectX_L);
            append(result, "mCroppingOffectX_R", mCroppingOffectX_R);

            result.append("mCroppingIntervalDefault: " + mCroppingIntervalDefault + newLine);

            append(result, "mCroppingSizeWidth", mCroppingSizeWidth);
            append(result, "mCroppingSizeHeight", mCroppingSizeHeight);

            result.append("mCroppingOffectY_L: " + mCroppingOffectY_L + newLine);
            result.append("mCroppingOffectY_R: " + mCroppingOffectY_R + newLine);

            return result.toString();
        }
    }
}
