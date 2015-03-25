/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.oobe.utils;

import android.content.Context;
import android.content.res.Resources;
import android.util.AttributeSet;
import android.widget.RelativeLayout;

import com.mediatek.oobe.R;

    /**
     * Used as the outer frame of all setup wizard pages that need to adjust their margins based
     * on the total size of the available display. (e.g. side margins set to 10% of total width.)
     */
public class ProportionalOuterFrame extends RelativeLayout {
    public ProportionalOuterFrame(Context context) {
        super(context);
    }
    public ProportionalOuterFrame(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    public ProportionalOuterFrame(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    /**
     * Set our margins and title area height proportionally to the available display size
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int parentWidth = MeasureSpec.getSize(widthMeasureSpec);
        int parentHeight = MeasureSpec.getSize(heightMeasureSpec);
        final Resources resources = getContext().getResources();
        float sideMargin = resources.getFraction(R.dimen.setup_border_width, 1, 1);//5%
        int bottom = resources.getDimensionPixelSize(R.dimen.setup_margin_bottom);//0dip
        //left,top,right,bottom
        setPadding(
                (int) (parentWidth * sideMargin),
                (int) (parentHeight * sideMargin),
                (int) (parentWidth * sideMargin),
                bottom);

        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }
}

