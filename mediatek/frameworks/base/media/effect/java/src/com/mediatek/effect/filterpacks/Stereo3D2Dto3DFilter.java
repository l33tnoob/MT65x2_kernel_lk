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

import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GLFrame;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.format.ImageFormat;
import android.graphics.Bitmap;
import android.os.Build;

/**
 * @hide
 */
public class Stereo3D2Dto3DFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    @GenerateFieldPort(name = "processDoneListener", hasDefault = true)
    private ProcessDoneListener mProcessDoneListener = null;

    public Stereo3D2Dto3DFilter(String name) {
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
    public void setupPorts() {
        addMaskedInputPort("image", ImageFormat.create(ImageFormat.COLORSPACE_RGBA, FrameFormat.TARGET_GPU));
        addOutputBasedOnInput("image", "image");
    }

    @Override
    public FrameFormat getOutputFormat(String portName, FrameFormat inputFormat) {
        return inputFormat;
    }

    @Override
    public void process(FilterContext context) {
        GLFrame input = (GLFrame) pullInput("image");
        FrameFormat inputFormat = input.getFormat();
        MutableFrameFormat outputFormat = inputFormat.mutableCopy();
        outputFormat.setDimensions(inputFormat.getWidth() * 2, inputFormat.getHeight());

        Bitmap bmpOrg = input.getBitmap();
        Bitmap bmpSmall = mTool.generateSmallImage(bmpOrg, 0.1f);
        bmpOrg.recycle();

        mTool.log('d', input + " [GPU in]");

        GLFrame output = (GLFrame) context.getFrameManager().newFrame(outputFormat);
        mTool.log('d', output + " [GPU out]");

        if (0 >= output.getTextureId() || 0 >= output.getFboId()) {
            output.focus();
        }

        int result;
        synchronized (Stereo3D2Dto3DFilter.class) {
            native_init(inputFormat.getWidth(), inputFormat.getHeight());
            result = native_process(input.getTextureId(), bmpSmall, output.getTextureId(), output.getFboId());
            native_close();
        }
        bmpSmall.recycle();

        mTool.log('d', output + " [GPU out]");

        // Push output and yield ownership.
        pushOutput("image", output);

        if (mProcessDoneListener != null) {
            Bitmap outputBitmap = null;
            if (0 == result) {
                outputBitmap = output.getBitmap();
            }
            mProcessDoneListener.onProcessDone(outputBitmap);
        }

        output.release();
    }

    /**
     * Native related
     */
    
    public int mNativeMyTo3d = 0;
    public int mNativeInitInfo = 0;
    public int mNativeProcInfo = 0;

    static boolean isCpuAbiNone() {
        return "none".equalsIgnoreCase(Build.CPU_ABI);
    }

    static {
        if (!isCpuAbiNone()) {
            System.loadLibrary("mtkeffect");
        }
    }

    private native int native_init(int inputWidth, int inputHeight);
    private native int native_process(int inTextureId, Bitmap smallImage, int outTextureId, int outFboId);
    private native int native_close();
}
