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

import android.util.Log;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * Quaternion class for rotations.
 */
public class Quaternion {

    private static final String TAG = "Quaternion";

    private static final float NORMALIZATION_TOLERANCE = 0.00001f;

    /**
     * Scalar component.
     */
    private float mQ0;

    /**
     * First vector component.
     */
    private float mQ1;

    /**
     * Second vector component.
     */
    private float mQ2;

    /**
     * Third vector component.
     */
    private float mQ3;

    /**
     * Constructor, sets the four components of the quaternion.
     *
     * @param q0 The scalar component
     * @param q1 The 1st vector component
     * @param q2 The 2nd vector component
     * @param q3 The 3rd vector component
     */
    public Quaternion(float q0, float q1, float q2, float q3) {
        this.set(q0, q1, q2, q3);
    }

    /**
     * Constructor, creates an identity quaternion.
     */
    public Quaternion() {
        idt();
    }

    /**
     * Constructor, sets the quaternion components from the given quaternion.
     *
     * @param quaternion The quaternion to copy from.
     */
    public Quaternion(Quaternion quaternion) {
        this.set(quaternion);
    }

    /**
     * Constructor, using an axis and the angle around that axis in degrees.
     *
     * @param axis  The axis
     * @param angle The angle in degrees.
     */
    public Quaternion(Vec3 axis, float angle) {
        this.set(axis, angle);
    }

    /**
     * Sets the components of the quaternion.
     *
     * @param x The scalar component
     * @param y The 1st vector component
     * @param z The 2nd vector component
     * @param w The 3rd vector component
     * @return This quaternion
     */
    public final Quaternion set(float x, float y, float z, float w) {
        this.mQ0 = x;
        this.mQ1 = y;
        this.mQ2 = z;
        this.mQ3 = w;
        return this;
    }

    /**
     * Sets the quaternion components from the given quaternion.
     *
     * @param quaternion The quaternion to copy from.
     * @return This quaternion
     */
    public final Quaternion set(Quaternion quaternion) {
        return this.set(quaternion.mQ0, quaternion.mQ1, quaternion.mQ2, quaternion.mQ3);
    }

    /**
     * Sets the quaternion using an axis and angle around that axis.
     *
     * @param axis  The axis
     * @param angle The angle in degrees
     * @return This quaternion
     */
    public final Quaternion set(Vec3 axis, float angle) {
        double norm = axis.getLength();
        if (norm == 0) {
            throw new Ngin3dException("MathRuntimeException");
        }
        double angrad = Math.toRadians(angle);

        double halfAngle = 0.5 * angrad;
        double coeff = Math.sin(halfAngle) / norm;

        return this.set((float) Math.cos(halfAngle), (float) (coeff * axis.x), (float) (coeff * axis.y), (float) (coeff * axis.z));
    }

    /**
     * @return a copy of this quaternion
     */
    public Quaternion cpy() {
        return new Quaternion(this);
    }

    /** Length of the quaternion (root of components squared)
     * @return the Euclidian length of this quaternion
     */
    public float len() {
        return (float) Math.sqrt(mQ0 * mQ0 + mQ1 * mQ1 + mQ2 * mQ2 + mQ3 * mQ3);
    }

    /**
     * Generate a text description of this object
     * @return Descriptive text string
     */
    @Override
    public String toString() {
        return "[" + mQ0 + "|" + mQ1 + "|" + mQ2 + "|" + mQ3 + "]";
    }

    /**
     * Sets the quaternion using euler angles.
     *
     * @param order  Order of rotation axes
     * @param alpha1 1st angle
     * @param alpha2 2nd angle
     * @param alpha3 3rd angle
     * @return This quaternion
     */
    public Quaternion setEulerAngles(EulerOrder order, float alpha1, float alpha2, float alpha3) {
        Quaternion r1 = new Quaternion(order.getA1(), alpha1);
        Quaternion r2 = new Quaternion(order.getA2(), alpha2);
        Quaternion r3 = new Quaternion(order.getA3(), alpha3);
        Quaternion composed = r1.multiply(r2.multiply(r3));

        mQ0 = composed.mQ0;
        mQ1 = composed.mQ1;
        mQ2 = composed.mQ2;
        mQ3 = composed.mQ3;

        return this;
    }

    /** Length-Squared
     * @return the length of this quaternion without square root
     */
    public float len2() {
        return mQ0 * mQ0 + mQ1 * mQ1 + mQ2 * mQ2 + mQ3 * mQ3;
    }

    /**
     * Normalizes this quaternion to unit length
     *
     * @return The quaternion
     */
    public Quaternion nor() {
        float len = len2();
        if (len != 0.f && (Math.abs(len - 1.0f) > NORMALIZATION_TOLERANCE)) {
            len = (float) Math.sqrt(len);
            mQ3 /= len;
            mQ0 /= len;
            mQ1 /= len;
            mQ2 /= len;
        }
        return this;
    }


    @Deprecated
    /** Predecessor to multiply() for SHORT TERM backward compatibility
     * @deprecated use multiply()
     */
    public Quaternion applyTo(Quaternion r) {
        return multiply(r);
    }

    /**
     * Multiplies the supplied quaternion by the member quaternion, applying the
     * result to the member quaternion.   qa.multiply(qb); means qa = qb.qa
     * Remember quaternions are not commutative.
     *
     * @return Product of multiplication
     */
    public Quaternion multiply(Quaternion r) {

        // ref http://en.wikipedia.org/wiki/Quaternion - Hamilton product
        float newQ0 = r.mQ0 * mQ0 - (r.mQ1 * mQ1 + r.mQ2 * mQ2 + r.mQ3 * mQ3);
        float newQ1 = r.mQ1 * mQ0 + r.mQ0 * mQ1 + r.mQ2 * mQ3 - r.mQ3 * mQ2;
        float newQ2 = r.mQ2 * mQ0 + r.mQ0 * mQ2 + r.mQ3 * mQ1 - r.mQ1 * mQ3;
        float newQ3 = r.mQ3 * mQ0 + r.mQ0 * mQ3 + r.mQ1 * mQ2 - r.mQ2 * mQ1;

        mQ0 = newQ0;
        mQ1 = newQ1;
        mQ2 = newQ2;
        mQ3 = newQ3;
        return this;
    }

    /**
     * Sets the quaternion to an identity Quaternion
     *
     * @return This quaternion
     */
    public final Quaternion idt() {
        this.set(1, 0, 0, 0);
        return this;
    }

    /**
     * Spherical linear interpolation between this quaternion and the other quaternion, based on the alpha value in the range
     * [0,1]. Taken from. Taken from Bones framework for JPCT, see http://www.aptalkarga.com/bones/
     *
     * @param end   The end quaternion
     * @param alpha Alpha in the range [0,1]
     * @return This quaternion
     */
    public Quaternion slerp(Quaternion end, float alpha) {
        if (this.equals(end)) {
            return this;
        }

        float result = dot(end);

        if (result < 0.0) {
            // Negate the second quaternion and the result of the dot product
            end.mul(-1);
            result = -result;
        }

        // Set the first and second scale for the interpolation
        float scale0 = 1 - alpha;
        float scale1 = alpha;

        // Check if the angle between the 2 quaternions was big enough to
        // warrant such calculations
        if ((1 - result) > 0.1) { // Get the angle between the 2 quaternions,
            // and then store the sin() of that angle
            final double theta = Math.acos(result);
            final double invSinTheta = 1f / Math.sin(theta);

            // Calculate the scale for mQ1 and mQ2, according to the angle and
            // it's sine value
            scale0 = (float) (Math.sin((1 - alpha) * theta) * invSinTheta);
            scale1 = (float) (Math.sin((alpha * theta)) * invSinTheta);
        }

        // Calculate the mQ0, mQ1, mQ2 and mQ3 values for the quaternion by using a
        // special form of linear interpolation for quaternions.
        final float x = (scale0 * this.mQ0) + (scale1 * end.mQ0);
        final float y = (scale0 * this.mQ1) + (scale1 * end.mQ1);
        final float z = (scale0 * this.mQ2) + (scale1 * end.mQ2);
        final float w = (scale0 * this.mQ3) + (scale1 * end.mQ3);
        set(x, y, z, w);

        // Return the interpolated quaternion
        return this;
    }

    /** Comparison.
     *
     * @param o Object to compare with
     * @return True if the supplied object is, or has the same parameters as, this object
     */
    public boolean equals(final Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof Quaternion)) {
            return false;
        }
        final Quaternion comp = (Quaternion) o;
        return this.mQ0 == comp.mQ0 && this.mQ1 == comp.mQ1 && this.mQ2 == comp.mQ2 && this.mQ3 == comp.mQ3;
    }


    /**
     * @hide Not relevant to 3D Graphics API
     */
     @Override
    public int hashCode() {
        int result = (mQ0 == +0.0f ? 0 : Float.floatToIntBits(mQ0));
        result = 31 * result + (mQ1 == +0.0f ? 0 : Float.floatToIntBits(mQ1));
        result = 31 * result + (mQ2 == +0.0f ? 0 : Float.floatToIntBits(mQ2));
        result = 31 * result + (mQ3 == +0.0f ? 0 : Float.floatToIntBits(mQ3));
        return result;
    }

    /**
     * Dot product between this and the other quaternion.
     *
     * @param other the other quaternion.
     * @return this quaternion.
     */
    public float dot(Quaternion other) {
        return mQ0 * other.mQ0 + mQ1 * other.mQ1 + mQ2 * other.mQ2 + mQ3 * other.mQ3;
    }

    /**
     * Multiplication by a scalar.
     *
     * @param scalar The scalar.
     * @return This quaternion
     */
    public Quaternion mul(float scalar) {
        this.mQ0 *= scalar;
        this.mQ1 *= scalar;
        this.mQ2 *= scalar;
        this.mQ3 *= scalar;
        return this;
    }

    /**
     * Get the normalized axis of the rotation.
     *
     * @return Normalized axis of the rotation
     */
    public Vec3 getAxis() {
        double squaredSine = mQ1 * mQ1 + mQ2 * mQ2 + mQ3 * mQ3;
        if (squaredSine == 0) {
            return new Vec3(1, 0, 0);
        }
        float inverse = 1 / (float) Math.sqrt(squaredSine);
        return new Vec3(mQ1 * inverse, mQ2 * inverse, mQ3 * inverse);
    }

    /**
     * Get the angle of the rotation.
     *
     * @return Angle of the rotation (between 0 and 2pi)
     */
    public float getAxisAngle() {
        return 2 * (float) Math.toDegrees(Math.acos(mQ0));
    }

    /**
     * Get the Euler angles corresponding to the Quaternion.
     *
     * <p><b>Warning.</b> The benefit of using quaternions instead of Euler angles
     * is to avoid the effect known as Gimbal Lock.  Do not convert to using
     * Euler angles without great care.
     *
     * @param order Rotation order to use
     * @return Array of three angles, in the order specified
     */
    public float[] getEulerAngles(EulerOrder order) {
        /*
         * The equations show that each rotation can be defined by two
         * different values of the Cardan or Euler angles set. For example
         * if Cardan angles are used, the rotation defined by the angles
         * a1, a2 and a3 is the same as the rotation defined by the angles pi + a1,
         * PI - a2 and PI + a3. This method implements
         * the following arbitrary choices:
         *
         * * for Cardan angles, the chosen set is the one for which the
         * second angle is between -PI/2 and PI/2 (i.e its cosine is
         * positive),
         * * for Euler angles, the chosen set is the one for which the
         * second angle is between 0 and PI (i.e its sine is positive).
         *
         * Cardan and Euler angle have a very disappointing drawback: all
         * of them have singularities. This means that if the instance is
         * too close to the singularities corresponding to the given
         * rotation order, it will be impossible to retrieve the angles. For
         * Cardan angles, this is often called gimbal lock. There is
         * <em>nothing</em> to do to prevent this, it is an intrinsic problem
         * with Cardan and Euler representation (but not a problem with the
         * rotation itself, which is perfectly well defined). For Cardan
         * angles, singularities occur when the second angle is close to
         * -PI/2 or +PI/2, for Euler angle singularities occur when the
         * second angle is close to 0 or PI, this implies that the identity
         * rotation is always singular for Euler angles!
         */
        if (order == EulerOrder.XYZ) {

            // r (Vec3.plusK) coordinates are :
            //  sin (theta), -cos (theta) sin (phi), cos (theta) cos (phi)
            // (-r) (Vec3.plusI) coordinates are :
            // cos (psi) cos (theta), -sin (psi) cos (theta), sin (theta)
            // and we can choose to have theta in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.Z_AXIS);
            Vec3 v2 = applyTo(Vec3.X_AXIS);
            if ((v2.z < -0.9999999999) || (v2.z > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(-(v1.y), v1.z)),
                (float) -Math.toDegrees(Math.asin(v2.z)),
                (float) -Math.toDegrees(Math.atan2(-(v2.y), v2.x))
            };

        } else if (order == EulerOrder.XZY) {

            // r (Vec3.plusJ) coordinates are :
            // -sin (psi), cos (psi) cos (phi), cos (psi) sin (phi)
            // (-r) (Vec3.plusI) coordinates are :
            // cos (theta) cos (psi), -sin (psi), sin (theta) cos (psi)
            // and we can choose to have psi in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.Y_AXIS);
            Vec3 v2 = applyTo(Vec3.X_AXIS);
            if ((v2.y < -0.9999999999) || (v2.y > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(v1.z, v1.y)),
                (float) -Math.toDegrees(-Math.asin(v2.y)),
                (float) -Math.toDegrees(Math.atan2(v2.z, v2.x))
            };

        } else if (order == EulerOrder.YXZ) {

            // r (Vec3.plusK) coordinates are :
            //  cos (phi) sin (theta), -sin (phi), cos (phi) cos (theta)
            // (-r) (Vec3.plusJ) coordinates are :
            // sin (psi) cos (phi), cos (psi) cos (phi), -sin (phi)
            // and we can choose to have phi in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.Z_AXIS);
            Vec3 v2 = applyTo(Vec3.Y_AXIS);
            if ((v2.z < -0.9999999999) || (v2.z > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(v1.x, v1.z)),
                (float) -Math.toDegrees(-Math.asin(v2.z)),
                (float) -Math.toDegrees(Math.atan2(v2.x, v2.y))
            };

        } else if (order == EulerOrder.YZX) {

            // r (Vec3.plusI) coordinates are :
            // cos (psi) cos (theta), sin (psi), -cos (psi) sin (theta)
            // (-r) (Vec3.plusJ) coordinates are :
            // sin (psi), cos (phi) cos (psi), -sin (phi) cos (psi)
            // and we can choose to have psi in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.X_AXIS);
            Vec3 v2 = applyTo(Vec3.Y_AXIS);
            if ((v2.x < -0.9999999999) || (v2.x > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(-(v1.z), v1.x)),
                (float) -Math.toDegrees(Math.asin(v2.x)),
                (float) -Math.toDegrees(Math.atan2(-(v2.z), v2.y))
            };

        } else if (order == EulerOrder.ZXY) {

            // r (Vec3.plusJ) coordinates are :
            // -cos (phi) sin (psi), cos (phi) cos (psi), sin (phi)
            // (-r) (Vec3.plusK) coordinates are :
            // -sin (theta) cos (phi), sin (phi), cos (theta) cos (phi)
            // and we can choose to have phi in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.Y_AXIS);
            Vec3 v2 = applyTo(Vec3.Z_AXIS);
            if ((v2.y < -0.9999999999) || (v2.y > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(-(v1.x), v1.y)),
                (float) -Math.toDegrees(Math.asin(v2.y)),
                (float) -Math.toDegrees(Math.atan2(-(v2.x), v2.z))
            };

        } else { // last possibility is ZYX

            // r (Vec3.plusI) coordinates are :
            //  cos (theta) cos (psi), cos (theta) sin (psi), -sin (theta)
            // (-r) (Vec3.plusK) coordinates are :
            // -sin (theta), sin (phi) cos (theta), cos (phi) cos (theta)
            // and we can choose to have theta in the interval [-PI/2 ; +PI/2]
            Vec3 v1 = applyInverseTo(Vec3.X_AXIS);
            Vec3 v2 = applyTo(Vec3.Z_AXIS);
            if ((v2.x < -0.9999999999) || (v2.x > 0.9999999999)) {
                Log.w(TAG, "Touch Cardan Euler Singularity");
            }
            return new float[] {
                (float) -Math.toDegrees(Math.atan2(v1.y, v1.x)),
                (float) -Math.toDegrees(-Math.asin(v2.x)),
                (float) -Math.toDegrees(Math.atan2(v2.y, v2.z))
            };
        }
    }

    /**
     * Get the XYZ set of Euler angles.
     *
     * @see getEulerAngles(EulerOrder order)
     *
     * @return Array of three angles, in the order specified
     */
    public float[] getEulerAngles() {
        return getEulerAngles(EulerOrder.XYZ);
    }

    /**
     * Apply the rotation to a vector.
     *
     * @param u vector to apply the rotation to
     * @return a copy of the given vector rotated by this quaternion
     */
    public Vec3 applyTo(Vec3 u) {
        float x = u.x;
        float y = u.y;
        float z = u.z;

        float s = mQ1 * x + mQ2 * y + mQ3 * z;
        float m0 = -mQ0;

        return new Vec3(2 * (m0 * (x * m0 - (mQ2 * z - mQ3 * y)) + s * mQ1) - x,
            2 * (m0 * (y * m0 - (mQ3 * x - mQ1 * z)) + s * mQ2) - y,
            2 * (m0 * (z * m0 - (mQ1 * y - mQ2 * x)) + s * mQ3) - z);
    }

    /**
     * Apply the inverse of the rotation to a vector.
     *
     * @param u vector to apply the inverse of the rotation to
     * @return a copy of the given vector rotated by this quaternion's inverse
     */
    public Vec3 applyInverseTo(Vec3 u) {
        float x = u.x;
        float y = u.y;
        float z = u.z;

        float s = mQ1 * x + mQ2 * y + mQ3 * z;

        return new Vec3(2 * (mQ0 * (x * mQ0 - (mQ2 * z - mQ3 * y)) + s * mQ1) - x,
            2 * (mQ0 * (y * mQ0 - (mQ3 * x - mQ1 * z)) + s * mQ2) - y,
            2 * (mQ0 * (z * mQ0 - (mQ1 * y - mQ2 * x)) + s * mQ3) - z);
    }

    /**
     * Get the scalar component.
     *
     * @return Scalar component
     */
    public float getQ0() {
        return mQ0;
    }

    /**
     * Get the first vector component.
     *
     * @return first vector component
     */
    public float getQ1() {
        return mQ1;
    }

    /**
     * Get the second vector component.
     *
     * @return second vector component
     */
    public float getQ2() {
        return mQ2;
    }

    /**
     * Get the third vector component.
     *
     * @return third vector component
     */
    public float getQ3() {
        return mQ3;
    }

}

