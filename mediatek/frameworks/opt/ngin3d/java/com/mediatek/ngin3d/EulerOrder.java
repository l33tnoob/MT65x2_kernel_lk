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
 * 'Axis order' for rotation operations.
 *
 * Technically 'Euler' angles are a 3D rotation using three angles and two axes:
 * rotation about axis A, then axis B, then axis A again. However, this class
 * is used to specify all 3-axis-sequence rotations.
 *
 */
public final class EulerOrder {

    /** Rotate around X, then Y then Z
     */
    public static final EulerOrder XYZ =
        new EulerOrder("XYZ", Vec3.X_AXIS, Vec3.Y_AXIS, Vec3.Z_AXIS);

    /** Rotate around X, then Z then Y
     */
    public static final EulerOrder XZY =
        new EulerOrder("XZY", Vec3.X_AXIS, Vec3.Z_AXIS, Vec3.Y_AXIS);

    /** Rotate around Y, then X then Z
     */
    public static final EulerOrder YXZ =
        new EulerOrder("YXZ", Vec3.Y_AXIS, Vec3.X_AXIS, Vec3.Z_AXIS);

    /** Rotate around Y, then Z then X
     */
    public static final EulerOrder YZX =
        new EulerOrder("YZX", Vec3.Y_AXIS, Vec3.Z_AXIS, Vec3.X_AXIS);

    /** Rotate around Z, then X then Y
     */
    public static final EulerOrder ZXY =
        new EulerOrder("ZXY", Vec3.Z_AXIS, Vec3.X_AXIS, Vec3.Y_AXIS);

    /** Rotate around Z, then Y then X
     */
    public static final EulerOrder ZYX =
        new EulerOrder("ZYX", Vec3.Z_AXIS, Vec3.Y_AXIS, Vec3.X_AXIS);

    /** Rotate around X, then Y then X again
     */
    public static final EulerOrder XYX =
        new EulerOrder("XYX", Vec3.X_AXIS, Vec3.Y_AXIS, Vec3.X_AXIS);

    /** Rotate around X, then Z then X again
     */
    public static final EulerOrder XZX =
        new EulerOrder("XZX", Vec3.X_AXIS, Vec3.Z_AXIS, Vec3.X_AXIS);

    /** Rotate around Y, then X then Y again
     */
    public static final EulerOrder YXY =
        new EulerOrder("YXY", Vec3.Y_AXIS, Vec3.X_AXIS, Vec3.Y_AXIS);

    /** Rotate around Y, then Z then Y again
     */
    public static final EulerOrder YZY =
        new EulerOrder("YZY", Vec3.Y_AXIS, Vec3.Z_AXIS, Vec3.Y_AXIS);

    /** Rotate around Z, then X then Z again
     */
    public static final EulerOrder ZXZ =
        new EulerOrder("ZXZ", Vec3.Z_AXIS, Vec3.X_AXIS, Vec3.Z_AXIS);

    /** Rotate around Z, then Y then Z again
     */
    public static final EulerOrder ZYZ =
        new EulerOrder("ZYZ", Vec3.Z_AXIS, Vec3.Y_AXIS, Vec3.Z_AXIS);

    /** Name of the rotations order. */
    private final String mName;

    /** Axis of the first rotation. */
    private final Vec3 mA1;

    /** Axis of the second rotation. */
    private final Vec3 mA2;

    /** Axis of the third rotation. */
    private final Vec3 mA3;

    /** Private constructor.
     * This is a utility class that cannot be instantiated by the user,
     * so its only constructor is private.
     * @param name name of the rotation order
     * @param a1 axis of the first rotation
     * @param a2 axis of the second rotation
     * @param a3 axis of the third rotation
     */
    private EulerOrder(final String name,
                       final Vec3 a1, final Vec3 a2, final Vec3 a3) {
        this.mName = name;
        this.mA1 = a1;
        this.mA2 = a2;
        this.mA3 = a3;
    }

    /** Get a string representation of the instance, example "XYX".
     * @return Text string of the form 'XYZ'
     */
    @Override
    public String toString() {
        return mName;
    }

    /** Get the axis of the first rotation.
     * @return Axis of the first rotation
     */
    public Vec3 getA1() {
        return mA1;
    }

    /** Get the axis of the second rotation.
     * @return Axis of the second rotation
     */
    public Vec3 getA2() {
        return mA2;
    }

    /** Get the axis of the third rotation.
     * @return Axis of the third rotation
     */
    public Vec3 getA3() {
        return mA3;
    }

    /**
     * Compare the supplied object with this instance.
     *
     * @param o Oobject to be compared
     * @return True if two objects are the same.
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        EulerOrder order = (EulerOrder) o;
        if (!mName.equals(order.mName)) return false;
        if (!mA1.equals(order.mA1)) return false;
        if (!mA2.equals(order.mA2)) return false;
        if (!mA3.equals(order.mA3)) return false;

        return true;
    }

    /**
     * @hide Not relevant to 3D graphics API
     */
    @Override
    public int hashCode() {
        int result = (mName.hashCode());
        result = 31 * result + mA1.hashCode();
        result = 31 * result + mA2.hashCode();
        result = 31 * result + mA3.hashCode();
        return result;
    }
}
