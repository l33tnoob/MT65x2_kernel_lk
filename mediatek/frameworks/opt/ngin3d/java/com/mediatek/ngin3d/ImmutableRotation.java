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
 * An immutable version of Rotation.
 * This is an object that can be created once with a set of parameters but
 * if any attempt is made to change those parameters an Exception is thrown.
 */

public class ImmutableRotation extends Rotation {
    /**
     * Build a rotation from the quaternion coordinates.
     *
     * @param q0        scalar part of the quaternion
     * @param q1        first coordinate of the vectorial part of the quaternion
     * @param q2        second coordinate of the vectorial part of the quaternion
     * @param q3        third coordinate of the vectorial part of the quaternion
     * @param normalize if true, the coordinates are considered
     *                  not to be normalized, a normalization preprocessing step is performed
     *                  before using them
     */
    public ImmutableRotation(float q0, float q1, float q2, float q3, boolean normalize) {
        super.set(q0, q1, q2, q3, normalize);
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void set(float q0, float q1, float q2, float q3, boolean normalize) {
        throw new Ngin3dException("Not allow to modify immutable Rotation with set("
                + q0 + ", " + q1 + ", " + q2 + ", " + q3 + ", " + normalize + "), "
                + "it might be a default Rotation. Create new Rotation() first then use that");
    }

    /**
     * Build a rotation from an axis and an angle.
     *
     * @param axis  axis around which to rotate
     * @param angle rotation angle in degree.
     * @throws ArithmeticException if the axis norm is zero
     */
    public ImmutableRotation(Vec3 axis, float angle) {
        super.set(axis, angle);
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void set(Vec3 axis, float angle) {
        throw new Ngin3dException("Not allow to modify immutable Rotation with set("
                + axis + ", " + angle + "), "
                + "it might be a default Rotation. Create new Rotation() first then use that");
    }

    public ImmutableRotation(float x, float y, float z, float angle) {
        super.set(new Vec3(x, y, z), angle);
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void set(float x, float y, float z, float angle) {
        throw new Ngin3dException("Not allow to modify immutable Rotation with set("
                + x + ", " + y + ", " + z + ", " + angle + "), "
                + "it might be a default Rotation. Create new Rotation() first then use that");
    }

    /**
     * Build a rotation from three Euler elementary rotations with specific order.
     *
     * @param order order of rotations to use
     * @param x     angle of the first elementary rotation
     * @param y     angle of the second elementary rotation
     * @param z     angle of the third elementary rotation
     */
    public ImmutableRotation(EulerOrder order, float x, float y, float z) {
        super.set(order, x, y, z);
    }

    /**
     * Build a rotation from three Euler elementary rotations with default XYZ order.
     *
     * @param x angle of the first elementary rotation
     * @param y angle of the second elementary rotation
     * @param z angle of the third elementary rotation
     */
    public ImmutableRotation(float x, float y, float z) {
        super.set(EulerOrder.XYZ, x, y, z);
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void set(EulerOrder order, float x, float y, float z) {
        throw new Ngin3dException("Not allow to modify immutable Rotation with set("
                + order + ", " + x + ", " + y + ", " + z + "), "
                + "it might be a default Rotation. Create new Rotation() first then use that");
    }

    /**
     * Try to modify immutable object will cause exception
     */
    @Override
    public final void set(float x, float y, float z) {
        throw new Ngin3dException("Not allow to modify immutable Rotation with set("
                + x + ", " + y + ", " + z + "), "
                + "it might be a default Rotation. Create new Rotation() first then use that");
    }
}
