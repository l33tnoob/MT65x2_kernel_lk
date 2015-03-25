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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.stereo3dwallpaper;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.view.SurfaceHolder;

public class Stereo3DRenderer {
    private static final String TAG = "Stereo3DRenderer";
    private Stereo3DSource mSource;
    private float mCanvasWidth;
    private float mCanvasHeight;
    private float mBitmapWidth;
    private float mBitmapHeight;
    private float mXOffset3D;
    private float mXOffset2D;
    private boolean mMode3D;

    /**
     * The method sets the source for rendering
     *
     * @param source the source used for rendering
     */
    protected void setSource(Stereo3DSource source) {
        mSource = source;
        mMode3D = true;
    }

    /**
     * The method renders the image on the canvas
     *
     * @param holder the surfaceholder whose surface has rendered
     */
    protected void render(SurfaceHolder holder) {
        Bitmap bitmapL = mSource.getLeftImage();
        Bitmap bitmapR = mSource.getRightImage();

        if (bitmapL == null || bitmapR == null) {
            Stereo3DLog.log(TAG, "Bitmap is null");
        } else {

            final Canvas c = holder.lockCanvas();

            if (c != null) {
                c.drawColor(Color.BLACK);

                mCanvasWidth = c.getWidth();
                mCanvasHeight = c.getHeight();

                Stereo3DLog.log(TAG, "Canvas width: " + mCanvasWidth);
                Stereo3DLog.log(TAG, "Canvas height: " + mCanvasHeight);

                if (mMode3D) {
                    Stereo3DLog.log(TAG, "draw 3D image");

                    mBitmapWidth = bitmapL.getWidth();
                    mBitmapHeight = bitmapL.getHeight();

                    Stereo3DLog.log(TAG, "Left image width: " + mBitmapWidth);
                    Stereo3DLog.log(TAG, "Left image height: " + mBitmapHeight);

                    c.save();

                    // left view
                    c.clipRect(0, 0, mCanvasWidth / 2, mCanvasHeight);
                    c.drawBitmap(bitmapL, mXOffset3D, 0, null);
                    c.restore();

                    // right view
                    c.clipRect(mCanvasWidth / 2, 0, mCanvasWidth, mCanvasHeight);
                    c.drawBitmap(bitmapR, mCanvasWidth / 2 + mXOffset3D, 0, null);
                    c.restore();
                } else {
                    Stereo3DLog.log(TAG, "draw 2D image");

                    Bitmap bitmap2D = mSource.get2DImage();

                    Stereo3DLog.log(TAG, "2D image width: " + bitmap2D.getWidth());
                    Stereo3DLog.log(TAG, "2D image height: " + bitmap2D.getHeight());

                    c.restore();
                    c.drawBitmap(bitmap2D, mXOffset2D, 0, null);
                }

                holder.unlockCanvasAndPost(c);
            }
        }
    }

    /**
     * The method sets the 3D mode
     *
     * @param mode true if 3D, false otherwise
     */
    protected void set3DMode(boolean mode) {
        Stereo3DLog.log(TAG, "Set 3D mode: " + mode);
        mMode3D = mode;
    }

    /**
     * The method is called to inform that the wallpaper's offsets changed
     *
     * @param xOffset the offset along the X dimension, from 0 to 1.
     * @param yOffset the offset along the Y dimension, from 0 to 1.
     */
    protected void onOffsetsChanged(float xOffset, float yOffset) {
        Stereo3DLog.log(TAG, "onOffsetsChanged - xOffset: " + xOffset + "; yOffset: " + yOffset);

        float remainingW = (mCanvasWidth / 2) - mBitmapWidth;
        mXOffset3D = remainingW < 0 ? (remainingW * xOffset) : (remainingW / 2);
        mXOffset2D = mXOffset3D * 2.0f;

        Stereo3DLog.log(TAG, "onOffsetsChanged - mXOffset3D: " + mXOffset3D);
    }
}