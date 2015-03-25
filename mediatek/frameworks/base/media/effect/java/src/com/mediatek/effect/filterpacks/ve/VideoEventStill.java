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
import android.filterfw.core.GLFrame;
import android.filterfw.core.ShaderProgram;

/**
 * @hide
 */
public class VideoEventStill extends VideoEvent {
    private ShaderProgram mMergeProgram;

    private static String mMergeShader =
        "precision mediump float;\n" +
        "uniform sampler2D tex_sampler_0;\n" +
        "uniform sampler2D tex_sampler_1;\n" +
        "uniform float factor;\n" +
        "varying vec2 v_texcoord;\n" +
        "void main() {\n" +
        "  vec4 fg = texture2D(tex_sampler_0, v_texcoord);\n" +
        "  vec4 bg = texture2D(tex_sampler_1, v_texcoord);\n" +
        "  gl_FragColor = mix(fg, bg, factor);\n" +
        "}\n";

    public VideoEventStill(String name, long start, long end) {
        super("Still", name, start, end);
    }

    @Override
    public void open(FilterContext context, VideoEventFilter myfilter) {
        super.open(context, myfilter);
        mMergeProgram = new ShaderProgram(context, mMergeShader);
        mMergeProgram.setHostValue("factor", 1.0f);
    }

    @Override
    public boolean process(FilterContext context, VideoEventFilter myfilter, boolean isRenderOutput, GLFrame output) {
        super.process(context, myfilter, isRenderOutput, output);

        if (null != output && isRenderOutput == false) {
            if (!mGotMainFrame && !mGotBgFrame) {
                return false;
            }

            Frame flist[] = {mMainFrame, mBgFrame};
            if (mGotMainFrame && !mGotBgFrame) {
                flist[1] = mMainFrame;
            } else if (!mGotMainFrame && mGotBgFrame) {
                flist[0] = mBgFrame;
            }
            mMergeProgram.process(flist, output);
            return true;
        }
        return false;
    }
}
