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
 * A rectangular box data type, in the style corner-1-xy, corner-2-xy in
 * floating point coordinates.
 */
public class Box implements JSON.ToJson {

    /** X coordinate of 1st corner */
    public float x1;
    /** Y coordinate of 1st corner */
    public float y1;
    /** X coordinate of 2nd corner */
    public float x2;
    /** Y coordinate of 2nd corner */
    public float y2;

    /**
     * Initialize a box without any argument.
     */
    public Box() {
        // Do nothing by default
    }

    /**
     * Construct by copying the contents of another box.
     *
     * @param other The Box to copy data from.
     */
    public Box(Box other) {
        x1 = other.x1;
        y1 = other.y1;
        x2 = other.x2;
        y2 = other.y2;
    }

    /**
     * Construct a box with start and end points value.
     *
     * @param x1  start value of x
     * @param y1  start value of y
     * @param x2  end value of x
     * @param y2  end value of y
     */
    public Box(float x1, float y1, float x2, float y2) {
        setBox(x1, y1, x2, y2);
    }

    /**
     * Set the box with start and end points value.
     *
     * @param x1  start value of x
     * @param y1  start value of y
     * @param x2  end value of x
     * @param y2  end value of y
     */
    public void set(float x1, float y1, float x2, float y2) {
        setBox(x1, y1, x2, y2);
    }

    private void setBox(float x1, float y1, float x2, float y2) {
        if (x2 < x1 || y2 < y1) {
            throw new IllegalArgumentException("x1 should be less than x2; y1 should be less than y2");
        }
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    /**
     * Get the size of this box.
     *
     * @return  The size of this box as a 'width, height' Dimension
     */
    public Dimension getSize() {
        return new Dimension(x2 - x1, y2 - y1);
    }

    /**
     * Get the width value of this box.
     *
     * @return  The width of this box
     */
    public float getWidth() {
        return x2 - x1;
    }
    /**
     * Get the height value of this box.
     *
     * @return  The height of this box
     */
    public float getHeight() {
        return y2 - y1;
    }

    /**
     * Convert the box property to string for output
     *
     * @return  Descriptive text string
     */
    @Override
    public String toString() {
        return "Box:[" + this.x1 + ", " + this.y1 + ", " + this.x2 + ", " + this.y2 + "]";
    }

    /**
     * Convert the box property to JSON formatted String
     *
     * @return   JSON-formatted text string
     */
    public String toJson() {
        return "{Box:[" + this.x1 + ", " + this.y1 + ", " + this.x2 + ", " + this.y2 + "]" + "}";
    }

    /**
     * Compare the object is same as this box.
     *
     * @param o  The object to be compared with
     * @return  True if the objects are the same
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Box box = (Box) o;

        if (Float.compare(box.x1, x1) != 0) return false;
        if (Float.compare(box.x2, x2) != 0) return false;
        if (Float.compare(box.y1, y1) != 0) return false;
        if (Float.compare(box.y2, y2) != 0) return false;

        return true;
    }

    /**
     * Create a new hash code.
     *
     * @return  a new hash code
     * @hide  Not a useful Graphics API function.
     */
    @Override
    public int hashCode() {
        int result = (x1 == +0.0f ? 0 : Float.floatToIntBits(x1));
        result = 31 * result + (y1 == +0.0f ? 0 : Float.floatToIntBits(y1));
        result = 31 * result + (x2 == +0.0f ? 0 : Float.floatToIntBits(x2));
        result = 31 * result + (y2 == +0.0f ? 0 : Float.floatToIntBits(y2));
        return result;
    }
}
