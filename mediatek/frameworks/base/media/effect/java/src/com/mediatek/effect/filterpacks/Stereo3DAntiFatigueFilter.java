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

import com.mediatek.effect.effects.Stereo3DAntiFatigueEffect;
import com.mediatek.effect.effects.Stereo3DAntiFatigueEffect.AntiFatigueInfo;

import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.format.ImageFormat;
import android.graphics.Bitmap;
import android.os.Build;

/**
 * @hide
 */
public class Stereo3DAntiFatigueFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    @GenerateFieldPort(name = "processDoneListener", hasDefault = true)
    private ProcessDoneListener mProcessDoneListener = null;

    @GenerateFieldPort(name = "inbitmapR", hasDefault = true)
    private Bitmap mBitmapR = null;
    @GenerateFieldPort(name = "inbitmapL", hasDefault = true)
    private Bitmap mBitmapL = null;

    @GenerateFieldPort(name = "operation", hasDefault = true)
    private int mOperation = 0;
    @GenerateFieldPort(name = "screenLayout", hasDefault = true)
    private int mScreenLayout = 0;
    @GenerateFieldPort(name = "mtk3dtag", hasDefault = true)
    private int mMTK3DTag = 0;


    public Stereo3DAntiFatigueFilter(String name) {
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
        Frame input = pullInput("image");
        pushOutput("image", input);

        AntiFatigueInfo giainfo = antiFatigue();

        if (mProcessDoneListener != null) {
            mProcessDoneListener.onProcessDone(giainfo);
        }
    }


    /**
     * Native related
     */

    public int mNativeMyStereo = 0;
    public int mNativeS3DKProcInfo = 0;
    public int mNativeS3DKWorkBufInfo = 0;
    public int mNativeS3DKInitInfo = 0;
    public int mNativeS3DKTuningParaInfo = 0;
    public int mNativeS3DKResultInfo = 0;

    static boolean isCpuAbiNone() {
        return "none".equalsIgnoreCase(Build.CPU_ABI);
    }

    static {
        if (!isCpuAbiNone()) {
            System.loadLibrary("mtkeffect");
        }
    }

    private native int native_init(int mtk3dtag);
    private native int native_close();
    private native Bitmap native_process(Bitmap bitmapR, Bitmap Right16, Bitmap Left16,
        Bitmap Right4, Bitmap Left4, int operation, int screenLayout, AntiFatigueInfo convergence);

    private static final int SUCCESS = 0;

    @GenerateFieldPort(name = "effectclass", hasDefault = true)
    private Stereo3DAntiFatigueEffect mAntiFatigueEffect = null;

    /**
     * Executes convergence algorithm for a 3D side-by-side image and produces
     * offsets information for better alignment of the left and right images.
     */
    @SuppressWarnings("finally")
    private AntiFatigueInfo antiFatigue() {
        Bitmap Right16 = null;
        Bitmap Left16 = null;
        Bitmap Right4 = null;
        Bitmap Left4 = null;
        AntiFatigueInfo info;

        if (mOperation == 0) {
            if (mBitmapR == null || mBitmapL == null) {
                mTool.log('e', "Bitmaps null");
                mTool.log('e', "mBitmapR: " + mBitmapR);
                mTool.log('e', "mBitmapL: " + mBitmapL);
                return null;
            }
        } else {
            if (mBitmapR == null) {
                mTool.log('e', "Bitmaps null");
                mTool.log('e', "mBitmapR: " + mBitmapR);
                return null;
            }
        }

        info = mAntiFatigueEffect.new AntiFatigueInfo();

        mTool.log('i', "Execute convergence: " + mBitmapR.getWidth() + " x " + mBitmapR.getHeight() + " operation:"
            + mOperation + ", screen_layout:" + mScreenLayout);

        int result[] = {-2};

        if (mOperation == 0) {
            Right16 = mTool.generateSmallImage(mBitmapR, 1f / 16f); // 1/16
            Left16 = mTool.generateSmallImage(mBitmapL, 1f / 16f); // 1/16
            Right4 = mTool.generateSmallImage(mBitmapR, 1f / 4f); // 1/4
            Left4 = mTool.generateSmallImage(mBitmapL, 1f / 4f); // 1/4
            
            mTool.log('d', "mBitmapR: " + mBitmapR.getWidth() + "x" + mBitmapR.getHeight());
            mTool.log('d', "mBitmapL: " + mBitmapL.getWidth() + "x" + mBitmapL.getHeight());
            mTool.log('d', "Right16: " + Right16.getWidth() + "x" + Right16.getHeight());
            mTool.log('d', "Left16: " + Left16.getWidth() + "x"+ Left16.getHeight());
            mTool.log('d', "Right4: " + Right4.getWidth() + "x" + Right4.getHeight());
            mTool.log('d', "Left4: " + Left4.getWidth() + "x" + Left4.getHeight());
        }

        synchronized (AntiFatigueInfo.class) {
            result[0] = native_init(mMTK3DTag);
            if (result[0] == SUCCESS) {
                info.mBitmap = native_process(mBitmapR, Right16, Left16, Right4, Left4, mOperation, mScreenLayout, info);
                native_close();
            }
        }

        mTool.log('w', "CallNativeThread Result = " + result[0]);

        if (mOperation == 0) {
            Right16.recycle();
            Left16.recycle();
            Right4.recycle();
            Left4.recycle();
        }

        if (result[0] == SUCCESS) {
            return info;
        } else {
            return null;
        }
    }
}
