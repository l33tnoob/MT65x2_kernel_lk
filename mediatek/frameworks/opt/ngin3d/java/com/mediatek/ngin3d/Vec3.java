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
 * Representation of a vector in a three-dimensional space.
 * <p>
 * Mostly this is a synonym of Point but at times it makes sense to treat
 * an XYZ triplet as a position in space and sometimes as a vector.  Some
 * methods (like "length") only make sense in the context of a vector.
 */
public class Vec3 extends Point {
    /** X-axis vector. */
    public static final Vec3 X_AXIS = new Vec3(1, 0, 0);
    /** Y-axis vector. */
    public static final Vec3 Y_AXIS = new Vec3(0, 1, 0);
    /** Z-axis vector. */
    public static final Vec3 Z_AXIS = new Vec3(0, 0, 1);


    /**
     * Simple constructor.
     * Build a vector from its coordinates
     *
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
    public Vec3(float x, float y, float z) {
        super(x, y, z);
    }

    /**
     * Default constructor.
     */
    public Vec3() {
        // Do nothing by default
    }

    /**
     * Simple constructor using an existing Point.
     *
     * @param other XYZ triplet as a Point
     */
    public Vec3(Point other) {
        super(other);
    }

    /**
     * Add a vector to the instance.
     *
     * @param v Vector to add
     * @return A new vector
     */
    public Vec3 add(Vec3 v) {
        return new Vec3(x + v.x, y + v.y, z + v.z);
    }

    /**
     * Add a scaled vector to the instance.
     *
     * @param factor Scale factor to apply to v before adding it
     * @param v      Vector to add
     * @return A new vector
     */
    public Vec3 add(float factor, Vec3 v) {
        return new Vec3(x + factor * v.x, y + factor * v.y, z + factor * v.z);
    }

    /**
     * Subtract a vector from the instance.
     *
     * @param v Vector to subtract
     * @return A new vector
     */
    public Vec3 subtract(Vec3 v) {
        return new Vec3(x - v.x, y - v.y, z - v.z);
    }

    /**
     * Subtract a scaled vector from the instance.
     *
     * @param factor Scale factor to apply to v before subtracting it
     * @param v      Vector to subtract
     * @return A new vector
     */
    public Vec3 subtract(float factor, Vec3 v) {
        return new Vec3(x - factor * v.x, y - factor * v.y, z - factor * v.z);
    }

    /**
     * Multiply the instance by a scalar
     *
     * @param a Scalar multiplicand
     * @return A new vector
     */
    public Vec3 multiply(float a) {
        return new Vec3(a * x, a * y, a * z);
    }

    /**
     * Get the length of the vector.
     *
     * @return Length of the vector
     */
    public float getLength() {
        return (float) Math.sqrt(x * x + y * y + z * z);
    }

    /**
     * Normalize.
     *
     * @return A new vector with length=1
     */
    public Vec3 getNormalized() {
        float len = getLength();
        if (len > 0) {
            return new Vec3(x / len, y / len, z / len);
        } else {
            return new Vec3(0, 0, 0);
        }
    }


    /**
     * Compute the dot-product of two vectors.
     *
     * @param v1 1st vector
     * @param v2 2nd vector
     * @return Dot product (v1 DOT v2)
     */
    public static float dotProduct(Vec3 v1, Vec3 v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    /**
     * Compute the cross-product of two vectors.
     *
     * @param v1 1st vector
     * @param v2 2nd vector
     * @return The cross product v1 ^ v2 as a new Vector
     */
    public static Vec3 crossProduct(Vec3 v1, Vec3 v2) {
        return new Vec3(v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x);
    }

    /**
     * Compute the angular separation between two vectors.
     *
     * <p>This method computes the angular separation between two
     * vectors using the dot product for well separated vectors and the
     * cross product for almost aligned vectors. This allows to have a
     * good accuracy in all cases, even for vectors very close to each
     * other.</p>
     *
     * @param v1 1st vector
     * @param v2 2nd vector
     * @return Angular separation between v1 and v2
     */
    public static float angle(Vec3 v1, Vec3 v2) {

        float normProduct = v1.getLength() * v2.getLength();
        if (normProduct == 0) {
            throw new Ngin3dException("MathArithmeticException");
        }

        float dot = dotProduct(v1, v2);
        float threshold = normProduct * 0.9999f;
        if ((dot < -threshold) || (dot > threshold)) {
            // the vectors are almost aligned, compute using the sine
            Vec3 v3 = crossProduct(v1, v2);
            if (dot >= 0) {
                return (float) Math.asin(v3.getLength() / normProduct);
            }
            return (float) Math.PI - (float) Math.asin(v3.getLength() / normProduct);
        }

        // the vectors are sufficiently separated to use the cosine
        return (float) Math.acos(dot / normProduct);

    }

    /**
     * Get a new vector orthogonal to the instance.
     *
     * <p>There are an infinite number of normalized vectors orthogonal
     * to the instance. This method picks up one of them almost
     * arbitrarily. It is useful when one needs to compute a reference
     * frame with one of the axes in a predefined direction. The
     * following example shows how to build a frame having the k axis
     * aligned with the known vector u :
     * <pre><code>
     *   Vec3 k = u.getNormalized();
     *   Vec3 i = k.getOrthogonal();
     *   Vec3 j = Vec3.crossProduct(k, i);
     * </code></pre></p>
     *
     * @return a new normalized Vec3 orthogonal to the instance
     * @throws ArithmeticException if the norm of the instance is null
     */
    public Vec3 getOrthogonal() {

        float threshold = 0.6f * getLength();
        if (threshold == 0) {
            throw new Ngin3dException("MathArithmeticException");
        }

        if ((x >= -threshold) && (x <= threshold)) {
            float inverse = 1 / (float) Math.sqrt(y * y + z * z);
            return new Vec3(0, inverse * z, -inverse * y);
        } else if ((y >= -threshold) && (y <= threshold)) {
            float inverse = 1 / (float) Math.sqrt(x * x + z * z);
            return new Vec3(-inverse * z, 0, inverse * x);
        }
        float inverse = 1 / (float) Math.sqrt(x * x + y * y);
        return new Vec3(inverse * y, -inverse * x, 0);

    }

}
