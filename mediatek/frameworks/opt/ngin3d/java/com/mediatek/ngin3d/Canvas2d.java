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

package com.mediatek.ngin3d;

import android.text.TextPaint;
import com.mediatek.ngin3d.presentation.Graphics2d;
import com.mediatek.ngin3d.presentation.PresentationEngine;

/**
 * A canvas that can be draw by Android graphics API and then be rendered in 3D. Typically
 * it can be used for UI object that can be draw only once or for a few times.
 * <p/>
 * Applications can create their own Canvas2d object by deriving from Canvas2d class and
 * override the drawRect method.
 */
public class Canvas2d extends Plane {

    protected final TextPaint mPaint = new TextPaint();

    private static class LastProperty<T> extends Property<T> {
        public LastProperty(String name, T defaultValue) {
            super(name, defaultValue);
        }

        @Override
        public boolean dependsOn(Property other) {
            if (this == other) {
                return false;
            }
            return true;    // always depends on other properties
        }
    }

    /**
     * @hide
     */
    static final LastProperty<Box> PROP_DIRTY_RECT = new LastProperty<Box>("dirty_rect", null);

    /**
     * Create a simple 2d canvas.
     */
    public Canvas2d() {
        super(false);
    }

    /**
     * Create a 2d canvas with Y-up or Y-down quad.
     * @param isYUp   true for creating a Y-up quad, default is Y-down
     */
    public Canvas2d(boolean isYUp) {
        super(isYUp);
    }

    /**
     * Apply new rectangular data to this object
     *
     * @param property input property type
     * @param value    input property value
     * @return true if the property is applied successfully
     * @hide
     */
    @Override
    protected boolean applyValue(Property property, Object value) {
        if (property.sameInstance(PROP_DIRTY_RECT)) {
            Box rect = (Box) value;
            drawRect(rect, getPresentation());
            return true;
        }

        return super.applyValue(property, value);
    }

    /**
     * Override to provide custom drawing.
     *
     * @param rect the direct rectangle
     * @param g2d  the Graphics2d presentation object
     */
    protected void drawRect(Box rect, Graphics2d g2d) {
        // Do nothing
    }

    /**
     * Set up new rectangular data
     *
     * @param rect box variable to be set
     */
    public void setDirtyRect(Box rect) {
        setValue(PROP_DIRTY_RECT, rect);
    }

    /**
     * Get the rectangular data of this object
     *
     * @return rectangular data
     */
    public Box getDirtyRect() {
        return getValue(PROP_DIRTY_RECT);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Presentation
    /**
     * @hide
     */
    @Override
    protected Graphics2d createPresentation(PresentationEngine engine) {
        return engine.createGraphics2d(mIsYUp);
    }

    /**
     * Returns the Actor's presentation cast to the instantiated type.
     *
     * @hide Presentation API should be internal only
     *
     * @return Presentation object
     */
    @Override
    public Graphics2d getPresentation() {
        return (Graphics2d) mPresentation;
    }

}
