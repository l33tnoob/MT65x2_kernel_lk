/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.camera;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceView;

public class VideoPreview extends SurfaceView {

    private static final int TILE_SIZE = 16;
    private float mAspectRatio = 1.0f;
    private int mHorizontalTileSize = TILE_SIZE;
    private int mVerticalTileSize = TILE_SIZE;
    private int mWeith;
    private int mHight;

    /**
     * Video preview construct
     * 
     * @param context
     *            : parent activity's context
     */
    public VideoPreview(Context context) {
        super(context);
    }

    /**
     * Video preview construct
     * 
     * @param context
     *            : parent activity's context
     * @param attrs
     *            : attribute set
     */
    public VideoPreview(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * Video preview construct
     * 
     * @param context
     *            : parent activity's context
     * @param attrs
     *            : attribute set
     * @param defStyle
     *            : defined preview style
     */
    public VideoPreview(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final int widthSpecSize = MeasureSpec.getSize(widthMeasureSpec);
        final int heightSpecSize = MeasureSpec.getSize(heightMeasureSpec);

        int width = widthSpecSize;
        int height = heightSpecSize;

        if (width > 0 && height > 0) {
            if (mWeith > width) {
                final float defaultRatio = ((float) width) / ((float) height);
                if (defaultRatio < mAspectRatio) {
                    // Need to reduce height
                    height = (int) (width / mAspectRatio);
                } else if (defaultRatio > mAspectRatio) {
                    width = (int) (height * mAspectRatio);
                }
                width =
                    roundUpToTile(width, mHorizontalTileSize, widthSpecSize);
                height =
                    roundUpToTile(height, mVerticalTileSize, heightSpecSize);
            } else {
                width = mWeith;
                height = mHight;
            }
            mWeith = width;
            mHight = height;
            setMeasuredDimension(width, height);
            return;
        }
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    /**
     * Set preview's set
     * 
     * @param horizontalSize
     *            : horizontal size
     * @param verticalSize
     *            : vertical size
     */
    public void setTileSize(int horizontalSize, int verticalSize) {
        if ((mHorizontalTileSize != horizontalSize)
            || (mVerticalTileSize != verticalSize)) {
            mHorizontalTileSize = horizontalSize;
            mVerticalTileSize = verticalSize;
            requestLayout();
            invalidate();
        }
    }

    /**
     * set aspect ration
     * 
     * @param width
     *            : aspect's width
     * @param height
     *            : aspect's height
     */
    public void setAspectRatio(int width, int height) {
        mWeith = width;
        mHight = height;
        setAspectRatio(((float) width) / ((float) height));
    }

    /**
     * set aspect ration
     * 
     * @param aspectRatio
     *            : aspect ration
     */
    public void setAspectRatio(float aspectRatio) {
        mAspectRatio = aspectRatio;
        requestLayout();
        invalidate();
    }

    /**
     * get current preview's width
     * 
     * @return width of current preivew
     */
    public int getCurrentW() {
        return mWeith;
    }

    /**
     * get current preview's height
     * 
     * @return height of current preivew
     */
    public int getCurrentH() {
        return mHight;
    }

    private int roundUpToTile(int dimension, int tileSize, int maxDimension) {
        return Math.min(((dimension + tileSize - 1) / tileSize) * tileSize,
            maxDimension);
    }

}
