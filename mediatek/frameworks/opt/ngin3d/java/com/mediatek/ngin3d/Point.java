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

/**
 * This class implements an XYZ triple, used to specify a point in a 3D space.
 */
public class Point {

    /**
     * X-axis component
     */
    public float x;

    /**
     * Y-axis component
     */
    public float y;

    /**
     * Z-axis component
     */
    public float z;

    /**
     * Indicates whether the x, y and z components should be interpreted as
     * being normalized values between 0.0 and 1.0.
     */
    public boolean isNormalized;

    /**
     * Construct a (0, 0, 0) point.
     */
    public Point() {
        // Do nothing by default
    }

    /**
     * Construct an optionally normalized zeroed point.
     *
     * @param isNormalized Whether the components should be treated as normalized
     */
    public Point(boolean isNormalized) {
        this(0, 0, 0, isNormalized);
    }

    /**
     * Construct un-normalized point with zero z-component.
     *
     * @param x X-component
     * @param y Y-component
     */
    public Point(float x, float y) {
        this(x, y, 0, false);
    }

    /**
     * Construct an optionally normalized point with zero z-component.
     *
     * @param x X-component
     * @param y Y-component
     * @param isNormalized Whether the components should be treated as normalized
     */
    public Point(float x, float y, boolean isNormalized) {
        this(x, y, 0, isNormalized);
    }

    /**
     * Construct un-normalized point.
     *
     * @param x X-component
     * @param y Y-component
     * @param z Z-component
     */
    public Point(float x, float y, float z) {
        this(x, y, z, false);
    }

    /**
     * Construct optionally normalized point.
     *
     * @param x X-component
     * @param y Y-component
     * @param z Z-component
     * @param isNormalized Whether the components should be treated as normalized
     */
    public Point(float x, float y, float z, boolean isNormalized) {
        // We cannot call set() in a constructor, because ImmutablePoint
        // forbids the use of set()
        this.x = x;
        this.y = y;
        this.z = z;
        this.isNormalized = isNormalized;
    }

    /**
     * Construct point using data from another point.
     *
     * @param other Point whose data to copy
     */
    public Point(Point other) {
        this(other.x, other.y, other.z, other.isNormalized);
    }

    /**
     * Constructs a point using the scaled data from another point.
     * The point created will be a * other.
     *
     * @param a Scale factor
     * @param other Base (unscaled) point
     */
    public Point(float a, Point other) {
        this(a * other.x, a * other.y, a * other.z, other.isNormalized);
    }

    /**
     * Set the x and y components.
     * The z-component and isNormalized flag will be unchanged.
     *
     * @param x X-component
     * @param y Y-component
     */
    public void set(float x, float y) {
        set(x, y, this.z, this.isNormalized);
    }

    /**
     * Set the x and y components and normalized flag.
     * The z-component will be unchanged.
     *
     * @param x X-component
     * @param y Y-component
     * @param isNormalized Whether the components should be treated as normalized
     */
    public void set(float x, float y, boolean isNormalized) {
        set(x, y, this.z, isNormalized);
    }

    /**
     * Set the x, y and z components.
     * The isNormalized flag will be unchanged.
     *
     * @param x X-component
     * @param y Y-component
     * @param z Z-component
     */
    public void set(float x, float y, float z) {
        set(x, y, z, this.isNormalized);
    }

    /**
     * Set the x, y and z components and normalized flag.
     *
     * @param x X-component
     * @param y Y-component
     * @param z Z-component
     * @param isNormalized Whether the components should be treated as normalized
     */
    public void set(float x, float y, float z, boolean isNormalized) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.isNormalized = isNormalized;
    }

    /**
     * Sets the value of the point to that of another point.
     *
     * @param other Point whose data to copy
     */
    public void set(Point other) {
        set(other.x, other.y, other.z, other.isNormalized);
    }

    /**
     * Sets the value of the point to a scaled version of that of another point.
     * The point created will be a * other.
     *
     * @param a Scale factor
     * @param other Base (unscaled) point
     */
    public void set(float a, Point other) {
        set(a * other.x, a * other.y, a * other.z, other.isNormalized);
    }

    /**
     * Add two Points. h = j.add(v)
     *
     * @param v Point to add
     * @return a new Point h being j+v
     */
    public Point add(Point v) {
        return new Point(x + v.x, y + v.y, z + v.z);
    }

    /**
     * Add two Points with scaling. h = j.add(factor,v)
     *
     * @param factor scale factor to apply to v before adding it
     * @param v      Point to add
     * @return a new Point h being j+(factor*v)
     */
    public Point add(float factor, Point v) {
        return new Point(x + factor * v.x, y + factor * v.y, z + factor * v.z);
    }

    /**
     * Subtract two Points. h = j.subtract(v)
     *
     * @param v Point to subtract
     * @return a new Point h being j-v
     */
    public Point subtract(Point v) {
        return new Point(x - v.x, y - v.y, z - v.z);
    }

    /**
     * Subtract two Points with scaling. h = j.subtract(factor, v)
     *
     * @param factor Scale factor to apply to <b>v</b> before subtracting it
     * @param v      Point to subtract
     * @return A new Point h being j-(factor*v)
     */
    public Point subtract(float factor, Point v) {
        return new Point(x - factor * v.x, y - factor * v.y, z - factor * v.z);
    }

    /**
     * Multiply the instance by a scalar.
     *
     * @param a scalar
     * @return a new vector
     */
    public Point multiply(float a) {
        return new Point(a * x, a * y, a * z);
    }

    /**
     * Compute the distance between two points.
     *
     * @param v1 first point
     * @param v2 second point
     * @return the distance between v1 and v2
     */
    public static float distance(Point v1, Point v2) {
        final float dx = v2.x - v1.x;
        final float dy = v2.y - v1.y;
        final float dz = v2.z - v1.z;
        return (float) Math.sqrt(dx * dx + dy * dy + dz * dz);
    }

    /** Compare two Points
     * @param o Object to compare with
     * @return True if the supplied object is, or has the same parameters as, this Point
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Point point = (Point) o;

        if (isNormalized != point.isNormalized) return false;
        if (Float.compare(point.x, x) != 0) return false;
        if (Float.compare(point.y, y) != 0) return false;
        if (Float.compare(point.z, z) != 0) return false;

        return true;
    }

    /**
     * @hide Not relevant to 3D Graphics API
     */
    @Override
    public int hashCode() {
        int result = (x == +0.0f ? 0 : Float.floatToIntBits(x));
        result = 31 * result + (y == +0.0f ? 0 : Float.floatToIntBits(y));
        result = 31 * result + (z == +0.0f ? 0 : Float.floatToIntBits(z));
        result = 31 * result + (isNormalized ? 1 : 0);
        return result;
    }

    /**
     * Generate a text description of this object
     * @return Descriptive text string
     */
    @Override
    public String toString() {
        return "Point:[" + this.x + ", " + this.y + ", " + this.z + "], isNormalized : " + isNormalized;
    }

    /**
     * Generate a text description in JSON format
     * @return Descriptive JSON formatted text string
     */
    public String toJson() {
        return "{Point:[" + this.x + ", " + this.y + ", " + this.z + "], isNormalized : " + isNormalized + "}";
    }

    /**
     * @hide Slightly obscure - internal use for AE parser?
     */
    public static Point newFromString(String positionString) {
        float[] xyz = Utils.parseStringToFloat(positionString);
        return new Point(xyz[0], xyz[1], xyz[2]);
    }
}
