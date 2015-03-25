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
 * Representation of scale in 3D space.
 */
public class Scale implements JSON.ToJson {
    /** X scaling factor */
    public float x;
    /** Y scaling factor */
    public float y;
    /** Z scaling factor */
    public float z;

    /**
     * Initialize the object with empty setting.
     */
    public Scale() {
        // Do nothing by default
    }

    /**
     * Initialize the object with specific x and y amount; z = 1.
     *
     * @param x Scale in X direction
     * @param y Scale in Y direction
     */
    public Scale(float x, float y) {
        this(x, y, 1.0f);
    }

    /**
     * Initialize the object with specific x, y, and z amount.
     *
     * @param x Scale in X direction
     * @param y Scale in Y direction
     * @param z Scale in Z direction
     */
    public Scale(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    /**
     * Initialize the scale equally in all directions.
     *
     * @param xyz Scale to apply equally on all 3 axes
     */
    public Scale(float xyz) {
        this(xyz, xyz, xyz);
    }

    /**
     * Set the specific value to this scale object.
     *
     * @param x  Scale in X direction
     * @param y  Scale in Y direction
     * @param z  Scale in Z direction
     */
    public void set(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    /**
     * Compare the input object with this scale object.
     * Returns True if either these are actually the same object, or if
     * the parameters of the objects are the same (to within the resolution
     * of Float.compare).
     *
     * @param o Object to be compared
     * @return  True if two objects are the same.
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Scale scale = (Scale) o;

        if (Float.compare(scale.x, x) != 0) return false;
        if (Float.compare(scale.y, y) != 0) return false;
        if (Float.compare(scale.z, z) != 0) return false;

        return true;
    }

     /**
     * Create a new hash code.
     * @return  hash code
     * @hide Internal use
     */
    @Override
    public int hashCode() {
        int result = (x == +0.0f ? 0 : Float.floatToIntBits(x));
        result = 31 * result + (y == +0.0f ? 0 : Float.floatToIntBits(y));
        result = 31 * result + (z == +0.0f ? 0 : Float.floatToIntBits(z));
        return result;
    }

    /**
     * Generate a text description of this object.
     *
     * @return Descriptive text string
     */
    @Override
    public String toString() {
        return "Point:[" + this.x + ", " + this.y + ", " + this.z + "]";
    }

    /**
     * Generate a text description in JSON format.
     *
     * @return Descriptive JSON formatted text string
     */
    public String toJson() {
        return "{Point:[" + this.x + ", " + this.y + ", " + this.z + "]" + "}";
    }

    public static Scale newFromString(String positionString) {
        float[] xyz = Utils.parseStringToFloat(positionString);
        return new Scale(xyz[0], xyz[1], xyz[2]);
    }

}
