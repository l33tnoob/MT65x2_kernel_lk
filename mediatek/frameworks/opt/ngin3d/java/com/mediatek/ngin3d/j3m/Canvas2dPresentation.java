/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/** \file
 * Canvas2d Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.util.Log;
import com.mediatek.j3m.Solid;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.presentation.Graphics2d;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * The presentation that provides 2D graphics drawing.
 * @hide
 */
// \todo Investigate why this exists in the presentation layer when it has no
// J3M-specific code
public class Canvas2dPresentation extends
    PlanePresentation implements Graphics2d {

    private static final String TAG = "Canvas2dPresentation";

    private Bitmap mBitmap;
    private Canvas mCanvas;
    private Box mBox;
    private Dimension mDimension;

    /**
     * Initializes this object with J3M presentation engine
     *
     * @param engine J3mPresentationEngine
     */
    public Canvas2dPresentation(J3mPresentationEngine engine, boolean isYUp) {
        super(engine, isYUp);
        mBox = new Box();
        mDimension = new Dimension();
    }

    /**
     * Un-initialize this object
     */
    @Override
    public void onUninitialize() {
        destroyCanvas();

        mBitmap = null;
        mCanvas = null;
        mBox = null;
        mDimension = null;

        super.onUninitialize();
    }

    /**
     * Gets the result of this canvas
     */
    public Canvas getCanvas() {
        return mCanvas;
    }

    /**
     * Begin drawing on the canvas with specified width, height
     * and background color.
     *
     * @param width           in pixels
     * @param height          in pixels
     * @param backgroundColor background color
     * @return result canvas
     */
    public Canvas beginDraw(int width, int height, int backgroundColor) {
        createCanvas(width, height, backgroundColor);
        return mCanvas;
    }

    private void createCanvas(int width, int height, int backgroundColor) {
        // Try reusing bitmap when the dimension is big enough and does not
        // change too much
        int bw = mBitmap == null ? 0 : mBitmap.getWidth();
        int bh = mBitmap == null ? 0 : mBitmap.getHeight();
        if (bw < width || bh < height
                || bw > width + 100 || bh > height + 100) {
            destroyCanvas();

            mBitmap = Bitmap.createBitmap(width, height,
                    Bitmap.Config.ARGB_8888);
            mCanvas = new Canvas(mBitmap);
            if (backgroundColor != Color.TRANSPARENT) {
                mCanvas.drawColor(backgroundColor, PorterDuff.Mode.SRC);
            }
        } else {
            mCanvas.drawColor(backgroundColor, PorterDuff.Mode.SRC);
        }
        mBox.set(0, 0, width, height);
        mDimension.set(width, height);

    }

    public void endDraw() {
        convertToTexture();
        fitTextureSize();
    }

    public void setFadeOutCoord(float x1, float x2) {
        if (x2 < x1) {
            throw new Ngin3dException("Fade end (x2) must be bigger than fade start (x1).");
        }

        float newX2;

        if (x2 > x1 * 1.05) {
            // for better performance, make x2 = 1 / ( x2 - x1 )  in advance.
            newX2 = 1 / (x2 - x1);
        } else {
            // x2 and x1 are too close, we take them as the same value.
            newX2 = 0;
        }

        ((Solid) getSceneNode()).getAppearance().setVector2f("M_FADEOUT_COORD", x1, newX2);
    }

    private void convertToTexture() {
        setImageSource(new ImageSource(ImageSource.BITMAP, mBitmap));
    }

    private void fitTextureSize() {
        setSourceRect(mBox);
        setSize(mDimension);
    }

    private void destroyCanvas() {
        if (mBitmap != null) {
            if (Ngin3d.DEBUG) {
                Log.v(TAG, "Recycle bitmap: " + mBitmap);
            }
            mBitmap.recycle();
            mBitmap = null;
        }
        mCanvas = null;
    }
}
