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

package com.mediatek.ygps;

/*
 Copyright (C) 2009  Ludwig M Brinckmann <ludwigbrinckmann@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Paint.Style;
import android.util.AttributeSet;
import android.view.View;

/**
 * GPS satellite signal view
 * @author mtk54046
 *
 */
public class SatelliteSignalView extends View {

    private static final int TEXT_OFFSET = 8;
    private static final int BASELINE_OFFSET = 5;
    private static final double ROW_DEVIDER = 5.0;
    private static final float PERCENT = 100.0F;
    private static final int ONE_QUARTER = 25;
    private static final int TWO_QUARTER = 50;
    private static final int THREE_QUARTER = 75;
    private static final float TEXT_SIZE = 10.0f;
    private static final float THIN_LINE_STOKE_WIDTH = 0.5f;
    private static final int DIVIDER_MIN = 15;
    private static final int DIVIDER_MAX = 32;
    private static final int DIVIDER_1 = 20;
    private static final int DIVIDER_2 = 25;
    private static final int DIVIDER_3 = 30;
    private Paint mLinePaint = null;
    private Paint mThinLinePaint = null;
    private Paint mBarPaintUsed = null;
    private Paint mBarPaintUnused = null;
    private Paint mBarPaintNoFix = null;
    private Paint mBarOutlinePaint = null;
    private Paint mTextPaint = null;
    private Paint mBackground = null;

    private SatelliteDataProvider mProvider = null;
    private int mSatellites = 0;
    private int[] mPrns = new int[SatelliteDataProvider.MAX_SATELLITES_NUMBER];
    private float[] mSnrs = new float[SatelliteDataProvider.MAX_SATELLITES_NUMBER];
    private int[] mUsedInFixMask = new int[SatelliteDataProvider.SATELLITES_MASK_SIZE];

    /**
     * Constructor
     * 
     * @param context
     *            The Context the view is running in, through which it can
     *            access the current theme, resources, etc.
     */
    public SatelliteSignalView(Context context) {
        this(context, null);
    }

    /**
     * Constructor
     * 
     * @param context
     *            The Context the view is running in, through which it can
     *            access the current theme, resources, etc.
     * @param attrs
     *            The attributes of the XML tag that is inflating the view.
     */
    public SatelliteSignalView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    /**
     * Constructor
     * 
     * @param context
     *            The Context the view is running in, through which it can
     *            access the current theme, resources, etc.
     * @param attrs
     *            The attributes of the XML tag that is inflating the view.
     * @param defStyle
     *            The default style to apply to this view. If 0, no style will
     *            be applied (beyond what is included in the theme). This may
     *            either be an attribute resource, whose value will be retrieved
     *            from the current theme, or an explicit style resource.
     */
    public SatelliteSignalView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        Resources res = getResources();
        if (null != res) {
            mLinePaint = new Paint();
            mLinePaint.setColor(res.getColor(R.color.sigview_line_color));
            mLinePaint.setAntiAlias(true);
            mLinePaint.setStyle(Style.STROKE);
            mLinePaint.setStrokeWidth(1.0f);

            mThinLinePaint = new Paint(mLinePaint);
            mThinLinePaint.setStrokeWidth(THIN_LINE_STOKE_WIDTH);

            mBarPaintUsed = new Paint();
            mBarPaintUsed.setColor(res.getColor(R.color.bar_used));
            mBarPaintUsed.setAntiAlias(true);
            mBarPaintUsed.setStyle(Style.FILL);
            mBarPaintUsed.setStrokeWidth(1.0f);

            mBarPaintUnused = new Paint(mBarPaintUsed);
            mBarPaintUnused.setColor(res.getColor(R.color.bar_unused));

            mBarPaintNoFix = new Paint(mBarPaintUsed);
            mBarPaintNoFix.setStyle(Style.STROKE);

            mBarOutlinePaint = new Paint();
            mBarOutlinePaint.setColor(res.getColor(R.color.bar_outline));
            mBarOutlinePaint.setAntiAlias(true);
            mBarOutlinePaint.setStyle(Style.STROKE);
            mBarOutlinePaint.setStrokeWidth(1.0f);

            mTextPaint = new Paint();
            mTextPaint.setColor(res.getColor(R.color.sigview_text_color));
            mTextPaint.setTextSize(TEXT_SIZE);
            mTextPaint.setTextAlign(Align.CENTER);
            mBackground = new Paint();
            mBackground.setColor(res.getColor(R.color.sigview_background));
        }
    }

    /**
     * Set data provider
     * 
     * @param provider
     *            Class that implement interface #SatelliteDataProvider
     * @see #SatelliteDataProvider
     */
    void setDataProvider(SatelliteDataProvider provider) {
        mProvider = provider;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        final int width = getWidth();
        final int height = getHeight();
        final float rowHeight = (float) Math.floor(height / ROW_DEVIDER);
        final float baseline = height - rowHeight + BASELINE_OFFSET;
        final float maxHeight = rowHeight * 4;
        final float scale = maxHeight / PERCENT;

        if (null != mProvider) {
            mSatellites = mProvider.getSatelliteStatus(mPrns, mSnrs, null,
                    null, 0, 0, mUsedInFixMask);
            for (int i = 0; i < mSatellites; i++) {
                if (mSnrs[i] < 0) {
                    mSnrs[i] = 0;
                }
            }
        }
        int devide = DIVIDER_MIN;
        if (mSatellites > DIVIDER_MAX) {
            devide = mSatellites;
        } else if (mSatellites > DIVIDER_3) {
            devide = DIVIDER_MAX;
        } else if (mSatellites > DIVIDER_2) {
            devide = DIVIDER_3;
        } else if (mSatellites > DIVIDER_1) {
            devide = DIVIDER_2;
        } else if (mSatellites > DIVIDER_MIN) {
            devide = DIVIDER_1;
        }
        final float slotWidth = (float) Math.floor(width / devide);
        final float barWidth = slotWidth / PERCENT * THREE_QUARTER;
        final float fill = slotWidth - barWidth;
        float margin = (width - slotWidth * devide) / 2;

        canvas.drawPaint(mBackground);
        canvas.drawLine(0, baseline, width, baseline, mLinePaint);
        float y = baseline - (PERCENT * scale);
        canvas.drawLine(0, y, getWidth(), y, mThinLinePaint);
        y = baseline - (TWO_QUARTER * scale);
        canvas.drawLine(0, y, getWidth(), y, mThinLinePaint);
        y = baseline - (ONE_QUARTER * scale);
        canvas.drawLine(0, y, getWidth(), y, mThinLinePaint);
        y = baseline - (THREE_QUARTER * scale);
        canvas.drawLine(0, y, getWidth(), y, mThinLinePaint);
        int drawn = 0;
        for (int i = 0; i < mSatellites; i++) {
            if (0 >= mPrns[i]) {
                continue;
            }
            final float left = margin + (drawn * slotWidth) + fill / 2;
            final float top = baseline - (mSnrs[i] * scale);
            final float right = left + barWidth;
            final float center = left + barWidth / 2;
            // if (0 == mUsedInFixMask[0]) {
            if (!isUsedInFix(0)) {
                canvas.drawRect(left, top, right, baseline, mBarPaintNoFix);
                // } else if (0 != (mUsedInFixMask[0] & (1 << (mPrns[i] - 1))))
                // {
            } else if (isUsedInFix(mPrns[i])) {
                canvas.drawRect(left, top, right, baseline, mBarPaintUsed);
            } else {
                canvas.drawRect(left, top, right, baseline, mBarPaintUnused);
            }
            canvas.drawRect(left, top, right, baseline, mBarOutlinePaint);
            String tmp = String.format("%3.1f", mSnrs[i]);
            canvas.drawText(tmp, center, top - fill, mTextPaint);
            canvas.drawText(Integer.toString(mPrns[i]), center, baseline
                    + TEXT_OFFSET + fill, mTextPaint);
            drawn += 1;
        }
    }

    /**
     * Check whether the satellite is used in fix
     * 
     * @param prn
     *            PRN of the satellite
     * @return True if the satellite is used in fix, or false
     */
    private boolean isUsedInFix(int prn) {
        int innerPrn = prn;
        boolean result = false;
        if (0 >= innerPrn) {
            for (int mask : mUsedInFixMask) {
                if (0 != mask) {
                    result = true;
                    break;
                }
            }
        } else {
            innerPrn = innerPrn - 1;
            int index = innerPrn / SatelliteDataProvider.SATELLITES_MASK_BIT_WIDTH;
            int bit = innerPrn % SatelliteDataProvider.SATELLITES_MASK_BIT_WIDTH;
            result = (0 != (mUsedInFixMask[index] & (1 << bit)));
        }
        return result;
    }
}
