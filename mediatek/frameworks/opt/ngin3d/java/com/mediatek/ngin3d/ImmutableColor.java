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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * An immutable version of Color.
 * This is an object that can be created once with a set of parameters but
 * if any attempt is made to change those parameters an Exception is thrown.
 */
public class ImmutableColor extends Color {
    /**
     * Initialize color to opaque black by default.
     */
    public ImmutableColor() {
        super();
    }
    /**
     * Initialize opaque color with specified R, G, B values.
     *
     * @param r R 0-255
     * @param g B 0-255
     * @param b G 0-255
     */
    public ImmutableColor(int r, int g, int b) {
        super(r, g, b);
    }

    /**
     * Initialize opaque color with RGB combination value.
     *
     * @param rgb RGB combination value
     */
    public ImmutableColor(int rgb) {
        super(rgb);
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void setRgb(int rgb) {
        throw new Ngin3dException("Not allow to modify immutable Color with setRgb(" + rgb + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }

    /**
     * Initialize color with RGB and A value.
     *
     * @param r red argument
     * @param g green argument
     * @param b blue argument
     * @param a alpha argument
     */
    public ImmutableColor(int r, int g, int b, int a) {
        super(r, g, b, a);
    }

    /**
     * Copy a color setting
     *
     * @return a new color object
     */
    @Override
    public Color copy() {
        try {
            return (Color) clone();
        } catch (CloneNotSupportedException e) {
            return new Color(red, green, blue, alpha);
        }
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final Color red(int r) {
        throw new Ngin3dException("Not allow to modify immutable Color with red(" + r + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final Color green(int g) {
        throw new Ngin3dException("Not allow to modify immutable Color with green(" + g + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final Color blue(int b) {
        throw new Ngin3dException("Not allow to modify immutable Color with blue(" + b + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final Color alpha(int a) {
        throw new Ngin3dException("Not allow to modify immutable Color with alpha(" + a + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void setHls(float hue, float luminance, float saturation) {
        throw new Ngin3dException("Not allow to modify immutable Color with setHls("
                + hue + ", " + luminance + ", " + saturation + "), "
                + "it might be a default Color. Create new Color() first then use that");
    }
}
