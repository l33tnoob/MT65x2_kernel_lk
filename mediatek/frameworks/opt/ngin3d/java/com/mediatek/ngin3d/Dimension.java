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

import com.mediatek.ngin3d.utils.JSON;

/**
 * A "Width & Height" object.
 */
public class Dimension implements JSON.ToJson {

    /**
     * Width of dimension
     */
    public float width;
    /**
     * Height of dimension
     */
    public float height;


    /**
     * @hide Why public?
     */
    public static final float NATURAL_SIZE = -1;

    /**
     * Initialize the dimension class with a default (meaningless) size
     */
    public Dimension() {
        width = NATURAL_SIZE;
        height = NATURAL_SIZE;
    }

    /**
     * Initialize the dimension class with specific width and height
     * @param width  value of width
     * @param height  value of height
     */
    public Dimension(float width, float height) {
        this.width = width;
        this.height = height;
    }

    /**
     * Set the dimension with specific width and height
     * @param width  value of width
     * @param height  value of height
     */
    public void set(float width, float height) {
        this.width = width;
        this.height = height;
    }

    /**
     * Check if the size is a valid size (a zero or positive value, or unchanged
     * from the original default).
     *
     * @param size  Size value to check
     * @return   True if the size is valid
     */
    public static boolean isValidSize(float size) {
        return (size >= 0 || size == Dimension.NATURAL_SIZE);
    }

    /**
     * Check if the input width and height is equal to this object's size.
     * @param width  Width to be compared
     * @param height Height to be compared
     * @return  True if the value of input is equal to this object's size
     */
    public final boolean equals(float width, float height) {
        return this.width == width && this.height == height;
    }

    /**
     * Check if the input object is equal to this dimension object.
     * @param o  Object to compare
     * @return  True if two objects are the same.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }

        if (o instanceof Dimension) {
            Dimension d = (Dimension) o;
            return this.width == d.width && this.height == d.height;
        }
        return false;
    }

    /**
     * Generate a new hash code.
     * @return  new hash code
     * @hide Not relevant to graphics API
     */
    @Override
    public int hashCode() {
        int result = 17;
        result = 37 * result + Float.floatToIntBits(this.width);
        result = 37 * result + Float.floatToIntBits(this.height);
        return result;
    }

    /**
     * Convert the dimension to string for output.
     * @return Descriptive text string
     */
    @Override
    public String toString() {
        return "Dimension: {width : " + width + ", height : " + height + "}";
    }

    /**
     * Convert the dimension to a JSON formatted string.
     * @return Descriptive text string in JSON format
     */
    public String toJson() {
        return "{Dimension: {width : " + width + ", height : " + height + "}}";
    }
}
