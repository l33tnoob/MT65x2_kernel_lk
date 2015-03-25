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
 * Representation of rotation in 3D space.
 * <p>
 * There are several mathematical ways to represent a rotation.
 * This API presents various easy-to-use ways of specifying a rotation.
 */

public class Rotation implements JSON.ToJson {
    /**
     * @hide
     */
    public static final String TAG = "Rotation";

    /**
     * Use Quaternion to represent the rotation
     */
    public static final int MODE_QUATERNION = 0;
    /**
     * An Euler way to represent the rotation
     */
    public static final int MODE_XYZ_EULER = 1;
    /**
     * Axis and angle way to represent the rotation
     */
    public static final int MODE_AXIS_ANGLE = 2;

    /*
     * Thresholds for geometric operations
     */
    private static final float ZERO_THRESHOLD = 0.0001f;
    private static final float DOT_THRESHOLD = 0.9995f;
    /**
     * Quaternion.
     */
    private final Quaternion mQuaternion = new Quaternion();

    /**
     * The Euler Angles in degree.
     */
    private float[] mEulerAngles = new float[3];

    /**
     * The angle of the rotation in degree
     */
    private float mAngle;

    /**
     * The axis of the rotation
     */
    private Vec3 mAxis;

    /**
     * The intrinsic type of rotation.
     */
    private int mMode;

    /**
     * Default constructor of a null rotation.
     */
    public Rotation() {
        this(1, 0, 0, 0, false);
    }

    /**
     * Build a rotation from quaternion coordinates.
     *
     * @param q0        scalar part of the quaternion
     * @param q1        first coordinate of the vectorial part of the quaternion
     * @param q2        second coordinate of the vectorial part of the quaternion
     * @param q3        third coordinate of the vectorial part of the quaternion
     * @param normalize if true, the coordinates are considered
     *                  not to be normalized, a normalization preprocessing step is performed
     *                  before using them
     */
    public Rotation(float q0, float q1, float q2, float q3, boolean normalize) {
        setByQuaternion(q0, q1, q2, q3, normalize);
    }

    /**
     * Set a rotation using quaternion coordinates.
     *
     * @param q0        scalar part of the quaternion
     * @param q1        first coordinate of the vectorial part of the quaternion
     * @param q2        second coordinate of the vectorial part of the quaternion
     * @param q3        third coordinate of the vectorial part of the quaternion
     * @param normalize if true, the coordinates are considered
     *                  not to be normalized, a normalization preprocessing step is performed
     *                  before using them
     */
    public void set(float q0, float q1, float q2, float q3, boolean normalize) {
        setByQuaternion(q0, q1, q2, q3, normalize);
    }

    private void setByQuaternion(float q0, float q1, float q2, float q3, boolean normalize) {
        mQuaternion.set(q0, q1, q2, q3);
        if (normalize) {
            mQuaternion.nor();
        }
        mMode = MODE_QUATERNION;
    }

    /**
     * Build a rotation from an axis and an angle.
     *
     * @param axis  axis around which to rotate
     * @param angle rotation angle in degree.
     * @throws ArithmeticException if the axis norm is zero
     */
    public Rotation(Vec3 axis, float angle) {
        setByAxisAngle(axis, angle);
    }

    /**
     * Set a rotation using axis and angle.
     *
     * @param axis  Axis around which to rotate
     * @param angle Rotation angle in degrees.
     * @throws ArithmeticException if the axis norm is zero
     */
    public void set(Vec3 axis, float angle) {
        setByAxisAngle(axis, angle);
    }

    private void setByAxisAngle(Vec3 axis, float angle) {
        mQuaternion.set(axis, angle);
        mMode = MODE_AXIS_ANGLE;
        mAngle = angle;
        mAxis = axis;
    }

    /**
     * Create a rotation using xyz vector components and angle.
     *
     * @param x  Vector component
     * @param y  Vector component
     * @param z  Vector component
     * @param angle Rotation angle in degrees.
     */
    public Rotation(float x, float y, float z, float angle) {
        this(new Vec3(x, y, z), angle);
    }

    /**
     * Set a rotation using xyz vector components and angle.
     *
     * @param x  Vector component
     * @param y  Vector component
     * @param z  Vector component
     * @param angle Rotation angle in degrees.
     */
    public void set(float x, float y, float z, float angle) {
        set(new Vec3(x, y, z), angle);
    }

    /**
     * Build a rotation from three Euler rotations with specific order.
     *
     * @param order Order of rotation axes to use
     * @param x     Angle of the first rotation
     * @param y     Angle of the second rotation
     * @param z     Angle of the third rotation
     */
    public Rotation(EulerOrder order, float x, float y, float z) {
        setByEuler(order, x, y, z);
    }

    /**
     * Build a rotation from three Euler rotations with default XYZ order.
     *
     * @param x     Angle of the first rotation
     * @param y     Angle of the second rotation
     * @param z     Angle of the third rotation
     */
    public Rotation(float x, float y, float z) {
        this(EulerOrder.XYZ , x, y, z);
    }

    /**
     * Set a rotation from three Euler rotations with specific order.
     *
     * @param order Order of rotation axes to use
     * @param x     Angle of the first rotation
     * @param y     Angle of the second rotation
     * @param z     Angle of the third rotation
     */
    public void set(EulerOrder order, float x, float y, float z) {
        setByEuler(order, x, y, z);
    }

    private void setByEuler(EulerOrder order, float x, float y, float z) {
        if (order.equals(EulerOrder.XYZ)) {
            mQuaternion.setEulerAngles(order, x, y, z);
        } else if (order.equals(EulerOrder.XZY)) {
            mQuaternion.setEulerAngles(order, x, z, y);
        } else if (order.equals(EulerOrder.ZYX)) {
            mQuaternion.setEulerAngles(order, z, y, x);
        } else if (order.equals(EulerOrder.ZXY)) {
            mQuaternion.setEulerAngles(order, z, x, y);
        } else if (order.equals(EulerOrder.YZX)) {
            mQuaternion.setEulerAngles(order, y, z, x);
        } else if (order.equals(EulerOrder.YXZ)) {
            mQuaternion.setEulerAngles(order, y, x, z);
        } else {
            mQuaternion.setEulerAngles(order, x, y, z);
        }

        mEulerAngles[0] = x;
        mEulerAngles[1] = y;
        mEulerAngles[2] = z;

        mMode = MODE_XYZ_EULER;
    }

    /**
     * Set a rotation from three Euler rotations with default XYZ order.
     *
     * @param x     Angle of the first rotation
     * @param y     Angle of the second rotation
     * @param z     Angle of the third rotation
     */
    public void set(float x, float y, float z) {
        set(EulerOrder.XYZ, x, y, z);
    }

    /**
     * Former pointAt using two Points (Deprecated).
     * Predecessor to fromTo() kept SHORT TERM for backward compatibility.
     *
     * @param s     Starting point
     * @param f     Destination point
     * @deprecated Use fromTo() - this method will be deleted with no further warning
     */
    @Deprecated
    public static final Rotation pointAt(Point s, Point f) {
        return fromTo(new Vec3(s.x, s.y, s.z), new Vec3(f.x, f.y, f.z));
    }

    /**
     * Create a rotation pointing in a given direction.
     * The returned rotation will transform the "startingDirection" vector to
     * the "finishingDirection" vector, using the shortest arc possible.
     *
     * @param startingDirection  Direction to transform from
     * @param finishingDirection Direction to transform to
     */
    public static final Rotation fromTo(
            Vec3 startingDirection, Vec3 finishingDirection) {
        Vec3 from = startingDirection.getNormalized();
        Vec3 to = finishingDirection.getNormalized();

        Vec3 half = from.add(to);
        float halfLength = half.getLength();

        // First find a quaternion which will rotate the "from" vector to the
        // "to" vector
        Rotation rotation = new Rotation();
        Quaternion quaternion = rotation.getQuaternion();
        if (halfLength < ZERO_THRESHOLD) {
            Vec3 axis = from.getOrthogonal();
            quaternion.set(axis, 180.f);
        } else {
            half = half.getNormalized();
            float dot = Vec3.dotProduct(from, half);
            Vec3 cross = Vec3.crossProduct(from, half);
            quaternion.set(dot, cross.x, cross.y, cross.z);
            quaternion.nor();
        }

        return rotation;
    }

    /**
     * Create a rotation pointing in a given direction.
     * <p>
     * The returned rotation will transform the "startingDirection" vector to
     * the "finishingDirection" vector. It also transform the "up" vector to
     * lie in the plane containing the passed "up" vector and the "to" vector
     * (i.e. it will try to keep the objects it rotates "upright").
     * <p>
     * e.g. To create a Rotation which will face a camera towards another object
     * work out the heading from the camera to the object
     * (from=cameraPos-objectPos), and then point the -ve z-axis of the camera
     * in that direction (to=(0,0,-1)), keeping the y-axis "up" (up=(0,1,0)).
     *
     * @param startingDirection  Direction to transform from
     * @param finishingDirection Direction to transform to
     * @param startingUp         Vector to keep up
     * @param finishingUp        Vector to keep up
     */
    public static final Rotation pointAt(
            Vec3 startingDirection, Vec3 finishingDirection,
            Vec3 startingUp, Vec3 finishingUp) {
        Vec3 to = finishingDirection.getNormalized();
        Vec3 upStart = startingUp.getNormalized();
        Vec3 up = finishingUp.getNormalized();

        // Perform basic point-at operation.
        Rotation rotation = fromTo(startingDirection, finishingDirection);
        Quaternion quaternion = rotation.getQuaternion();

        /* After pointing, the rotation will have an arbitrary "roll" around
         * the "to" z-axis.  We must find the new position of the "up" vector
         * after rotation.
         */
        Vec3 rolledUp = quaternion.applyTo(upStart);

        /* We now project this vector and the original "up" vector onto a plane
         * perpendicular to the "to" vector so that we can find the extra
         * rotation needed to rotate the rolled "up" vector onto the given
         * "up" vector. Note that these vectors won't have unit length.
         */
        Vec3 rolledUpProjected = rolledUp.subtract(
            Vec3.dotProduct(to, rolledUp), to);
        Vec3 upProjected = up.subtract(Vec3.dotProduct(to, up), to);

        /* Calculate the rotation bring rolledUpProjected onto upProjected.
         * Note that this rotation will be around the "to" vector (because both
         * vectors are parallel to the "to" vector after projection).
         */
        Rotation rollRotation = pointAt(rolledUpProjected, upProjected);

        // Combine the two rotations.
        quaternion.multiply(rollRotation.getQuaternion());

        return rotation;
    }

    /**
     * Get the normalized axis of the rotation.
     *
     * @return Normalized axis of the rotation
     * @see #Rotation(Vec3, float)
     */
    public Vec3 getAxis() {
        if (mMode == MODE_AXIS_ANGLE) {
            return mAxis;
        } else {
            return mQuaternion.getAxis();
        }
    }

    /**
     * Get the angle of the rotation.
     *
     * @return angle of the rotation (between 0 and &pi;)
     * @see #Rotation(Vec3, float)
     */
    public float getAxisAngle() {
        if (mMode == MODE_AXIS_ANGLE) {
            return mAngle;
        } else {
            return mQuaternion.getAxisAngle();
        }
    }

    /**
     * Get the Euler angles of the rotation for any axis order.
     *
     * @param order Order of axes, example YXY
     * @return Array of angles for the specifed order
     */
    public float[] getEulerAngles(EulerOrder order) {
        if (mMode == MODE_XYZ_EULER) {
            return mEulerAngles;
        } else {
            return mQuaternion.getEulerAngles(order);
        }

    }

    /**
     * Get the Euler angles of the rotation for XYZ.
     *
     * @return Array of angles for the default XYZ order
     */
    public float[] getEulerAngles() {
        return getEulerAngles(EulerOrder.XYZ);
    }

    /**
     * Get the type of this rotation object
     *
     * @return Rotation internal type (Euler, Quaternion, etc)
     * @hide This should really be fully encapsulated
     */
    public int getMode() {
        return mMode;
    }

    /**
     * Get the underlying Quaternion for this rotation
     *
     * @return A quaternion
     */
    public Quaternion getQuaternion() {
        return mQuaternion;
    }

    /**
     * Compare the input object is the same as this rotation object.
     *
     * @param o Object to be compared
     * @return True if two objects are the same or their properties are the same.
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Rotation rotation = (Rotation) o;

        if (mMode != rotation.getMode()) return false;

        if (mMode == MODE_XYZ_EULER) {
            float[] euler = rotation.getEulerAngles();
            if (Float.compare(mEulerAngles[0], euler[0]) != 0) return false;
            if (Float.compare(mEulerAngles[1], euler[1]) != 0) return false;
            if (Float.compare(mEulerAngles[2], euler[2]) != 0) return false;
        } else if (mMode == MODE_AXIS_ANGLE) {
            float angle = rotation.getAxisAngle();
            Vec3 axis = rotation.getAxis();
            if (Float.compare(mAngle, angle) != 0) return false;

            return mAxis.equals(axis);
        } else {
            return mQuaternion.equals(rotation.getQuaternion());
        }

        return true;
    }

    /**
     * Create a new hash code.
     *
     * @return hash code
     * @hide Not a useful 3D graphics API method
     */
    @Override
    public int hashCode() {
        int result;
        if (mMode == MODE_XYZ_EULER) {
            result = (mEulerAngles[0] == +0.0f ? 0 : Float.floatToIntBits(mEulerAngles[0]));
            result = 31 * result + (mEulerAngles[1] == +0.0f ? 0 : Float.floatToIntBits(mEulerAngles[1]));
            result = 31 * result + (mEulerAngles[2] == +0.0f ? 0 : Float.floatToIntBits(mEulerAngles[2]));
            return result;
        } else if (mMode == MODE_AXIS_ANGLE) {
            result = (mAxis.x == +0.0f ? 0 : Float.floatToIntBits(mAxis.x));
            result = 31 * result + (mAxis.y == +0.0f ? 0 : Float.floatToIntBits(mAxis.y));
            result = 31 * result + (mAxis.z == +0.0f ? 0 : Float.floatToIntBits(mAxis.z));
            result = 31 * result + (mAngle == +0.0f ? 0 : Float.floatToIntBits(mAngle));
            return result;
        } else {
            return 31 * mQuaternion.hashCode();
        }
    }

    /**
     * Generate a text description of this object.
     *
     * @return Descriptive text string
     */
    @Override
    public String toString() {
        if (mMode == MODE_AXIS_ANGLE) {
            return "Rotation:[" + mAxis.x + ", " + mAxis.y + ", " + mAxis.z + "], Mode: \"Axis Angle\", Angle: " + mAngle;
        } else if (mMode == MODE_XYZ_EULER) {
            return "Rotation:[" + mEulerAngles[0] + ", " + mEulerAngles[1] + ", " + mEulerAngles[2] + "], Mode: \"Euler\" ";
        } else {
            return "Rotation:[" + mQuaternion.getQ0() + ", " + mQuaternion.getQ1() + ", "
                + mQuaternion.getQ2() + ", " + mQuaternion.getQ3() + "], Mode: \"Quaternion\" ";
        }
    }

    /**
     * Generate a text description in JSON format.
     *
     * @return Descriptive JSON formatted text string
     */
    public String toJson() {
        if (mMode == MODE_AXIS_ANGLE) {
            return "{Rotation:[" + mAxis.x + ", " + mAxis.y + ", " + mAxis.z + "], Mode: \"Axis Angle\", Angle: " + mAngle + "}";
        } else if (mMode == MODE_XYZ_EULER) {
            return "{Rotation:[" + mEulerAngles[0] + ", " + mEulerAngles[1] + ", " + mEulerAngles[2] + "], Mode: \"Euler\" " + "}";
        } else {
            return "{Rotation:[" + mQuaternion.getQ0() + ", " + mQuaternion.getQ1() + ", "
                + mQuaternion.getQ2() + ", " + mQuaternion.getQ3() + "], Mode: \"Quaternion\" " + "}";
        }
    }

    /**
     * @hide Slightly obscure - internal use for AE parser?
     */
    public static Rotation newFromString(String positionString) {
        float[] xyz = Utils.parseStringToFloat(positionString);
        return new Rotation(xyz[0], xyz[1], xyz[2]);
    }
}
