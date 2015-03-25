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

package com.mediatek.ngin3d.animation;

import android.animation.ValueAnimator;
import android.util.Log;
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Property;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * Used to animate a property of specified target from one value to another.
 * @hide Unclear relationship between Animation and Animator
 */
public class PropertyAnimator extends ValueAnimator {

    private static final String TAG = "PropertyAnimator";
    public static final int DEFAULT_DURATION = 2000;

    protected Actor mTarget;
    protected Property mProperty;   // cached property key
    protected String mPropertyName;   // cached property name
    protected Object[] mValues;

    public PropertyAnimator() {
        // Do nothing
    }

    /**
     * Construct animator that modifies target property from specified start to end value.
     *
     * @param target Actor to modify
     * @param propertyName property name
     * @param values the first one should be start value and the second one is end value.
     */
    public PropertyAnimator(Actor target, String propertyName, Object... values) {
        if (target == null) {
            throw new IllegalArgumentException("Target cannot be null");
        }
        Property prop = target.getProperty(propertyName);
        if (prop == null) {
            throw new IllegalArgumentException("Cannot find property " + propertyName);
        }
        initialize(target, prop, values);
    }

    public PropertyAnimator(String propertyName, Object... values) {
        if (propertyName == null) {
            throw new IllegalArgumentException("Specify property name cannot be null");
        }
        mPropertyName = propertyName;
        mValues = values;
    }

    public PropertyAnimator(Actor target, Property property, Object... values) {
        initialize(target, property, values);
    }

    public PropertyAnimator(Property property, Object... values) {
        if (property == null) {
            throw new IllegalArgumentException("Specify property cannot be null");
        }
        mProperty = property;
        mValues = values;
    }

    private void initialize(Actor target, Property property, Object... values) {
        if (target == null) {
            throw new IllegalArgumentException("Target cannot be null");
        }
        if (property == null) {
            throw new IllegalArgumentException("Specify property cannot be null");
        }
        if (values.length < 2) {
            throw new IllegalArgumentException("Should specify at least two values");
        }

        mProperty = property;
        mTarget = target;
        mValues = values;

        setFloatValues(0f, 1f);
        setDuration(DEFAULT_DURATION);

        addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            public void onAnimationUpdate(ValueAnimator ani) {
                float fraction = ani.getAnimatedFraction();
                if (mValues[0] instanceof Float) {
                    float mStart = (Float) mValues[0];
                    float mEnd = (Float) mValues[1];
                    float value = mStart + fraction * (mEnd - mStart);
                    mTarget.setValue(mProperty, value);
                } else if (mValues[0] instanceof Integer) {
                    int mStart = (Integer) mValues[0];
                    int mEnd = (Integer) mValues[1];
                    int value = mStart + (int) (fraction * (mEnd - mStart));
                    mTarget.setValue(mProperty, value);
                } else if (mValues[0] instanceof Point) {
                    if (((Point) mValues[0]).isNormalized != ((Point)mValues[1]).isNormalized) {
                        throw new IllegalArgumentException("Cannot animate between normalized and unnormalized position");
                    }
                    Point mStart = (Point) mValues[0];
                    Point mEnd = (Point) mValues[1];
                    Point mValue = new Point(mEnd);
                    mValue.set(mStart.x + fraction * (mEnd.x - mStart.x), mStart.y
                            + fraction * (mEnd.y - mStart.y), mStart.z
                            + fraction * (mEnd.z - mStart.z));
                    mTarget.setValue(mProperty, mValue);
                } else if (mValues[0] instanceof Scale) {
                    Scale mStart = (Scale) mValues[0];
                    Scale mEnd = (Scale) mValues[1];
                    Scale mValue = new Scale();
                    mValue.set(mStart.x + fraction * (mEnd.x - mStart.x), mStart.y
                            + fraction * (mEnd.y - mStart.y), mStart.z
                            + fraction * (mEnd.z - mStart.z));
                    mTarget.setValue(mProperty, mValue);
                } else if (mValues[0] instanceof Color) {
                    Color mStart = (Color) mValues[0];
                    Color mEnd = (Color) mValues[1];
                    Color mValue = new Color();
                    mValue.red = mStart.red + (int)(fraction * (mEnd.red - mStart.red));
                    mValue.green = mStart.green + (int)(fraction * (mEnd.green - mStart.green));
                    mValue.blue = mStart.blue + (int)(fraction * (mEnd.blue - mStart.blue));
                    mValue.alpha = mStart.alpha + (int)(fraction * (mEnd.alpha - mStart.alpha));
                    mTarget.setValue(mProperty, mValue);
                } else if (mValues[0] instanceof Rotation) {
                    Rotation mStart = (Rotation) mValues[0];
                    Rotation mEnd = (Rotation) mValues[1];
                    Rotation mValue = new Rotation();
                    if (mEnd.getMode() == Rotation.MODE_XYZ_EULER) {
                        float[] euler1 = mStart.getEulerAngles();
                        float[] euler2 = mEnd.getEulerAngles();
                        float x = euler1[0] + fraction * (euler2[0] - euler1[0]);
                        float y = euler1[1] + fraction * (euler2[1] - euler1[1]);
                        float z = euler1[2] + fraction * (euler2[2] - euler1[2]);
                        mValue.set(x, y, z);

                    } else if (mEnd.getMode() == Rotation.MODE_AXIS_ANGLE) {
                        float angle1 = mStart.getAxisAngle();
                        Point v1 = mStart.getAxis();
                        float angle2 = mEnd.getAxisAngle();
                        Point v2 = mEnd.getAxis();

                        mValue.set(
                                v1.x + fraction * (v2.x - v1.x),
                                v1.y + fraction * (v2.y - v1.y),
                                v1.z + fraction * (v2.z - v1.z),
                                angle1 + fraction * (angle2 - angle1));
                    }
                    mTarget.setValue(mProperty, mValue);
                    if (mStart.getMode() != mEnd.getMode()) {
                        Log.w(TAG, "Warning: mixed angle interpolation");
                    }
                } else if (mValues[0] instanceof Stage.Camera) {
                    Stage.Camera mStart = (Stage.Camera) mValues[0];
                    Stage.Camera mEnd = (Stage.Camera) mValues[1];
                    Stage.Camera mValue = new Stage.Camera(mStart.position, mStart.lookAt);
                    mValue.position.set(
                            mStart.position.x + fraction * (mEnd.position.x - mStart.position.x),
                            mStart.position.y + fraction * (mEnd.position.y - mStart.position.y),
                            mStart.position.z + fraction * (mEnd.position.z - mStart.position.z));
                    mValue.lookAt.set(
                            mStart.lookAt.x + fraction * (mEnd.lookAt.x - mStart.lookAt.x),
                            mStart.lookAt.y + fraction * (mEnd.lookAt.y - mStart.lookAt.y),
                            mStart.lookAt.z + fraction * (mEnd.lookAt.z - mStart.lookAt.z));
                    mTarget.setValue(mProperty, mValue);
                } else {
                    throw new Ngin3dException("Property is not animatable");
                }
            }
        });
    }

    public String getPropertyName() {
        return mProperty.getName();
    }

    public Object getStartValue() {
        return mValues[0];
    }

    public Object getEndValue() {
        return mValues[1];
    }

    public void setTarget(Actor target) {
        if (target == null) {
            throw new IllegalArgumentException("Target cannot be null");
        }

        if (mProperty == null && mPropertyName == null) {
            // It's impossible to go this line
            throw new IllegalArgumentException("Property and property name can not both null");
        } else {
            if (mPropertyName == null) {
                if (target.getProperty(mProperty.getName()) == null) {
                    throw new IllegalArgumentException("The target has no property " + mProperty);
                }
                initialize(target, mProperty, mValues);
            } else {
                Property prop = target.getProperty(mPropertyName);
                if (prop == null) {
                    throw new IllegalArgumentException("Cannot find property " + mPropertyName);
                }
                initialize(target, prop, mValues);
            }
        }
    }

    public Actor getTarget() {
        return mTarget;
    }

    /**
     * Clone the PropertyAnimation, value in each member of cloned animation is same of original one, except target.
     * Mew instance of PropertyAnimation has no target in default.
     * @return the cloned PropertyAnimation
     */
    @Override
    public PropertyAnimator clone() {
        PropertyAnimator animation = (PropertyAnimator) super.clone();
        animation.mTarget = null;
        return animation;
    }
}
