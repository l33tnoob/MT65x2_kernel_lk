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

package com.mediatek.ngin3d.animation;

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
 * Animate an Actor Property (position, rotation, etc) from one value to another
 * applies to ngin3D-animations, not artist's Glo animations.
 */
public class PropertyAnimation extends BasicAnimation {

    private static final String TAG = "PropertyAnimation";

    protected Actor mTarget;
    protected Property mProperty;   // cached property key
    protected String mPropertyName;   // cached property name
    protected Object[] mValues;
    private Interpolator mInterpolator;

    public PropertyAnimation() {
        // Do nothing
    }

    /**
     * Base class for all internal value interpolator.
     */
    private abstract class Interpolator implements Alpha.Listener {
        public void onStarted() {
            PropertyAnimation.this.onStarted();
        }

        public void onPaused() {
            PropertyAnimation.this.onPaused();
        }

        public void onCompleted(int direction) {
            PropertyAnimation.this.onCompleted(direction);
        }
    }

    /**
     * Construct animation that modifies target property from specified start to end value.
     *
     * @param target Actor to modify
     * @param propertyName property name
     * @param values the first one should be start value and the second one is end value.
     */
    public PropertyAnimation(Actor target, String propertyName, Object... values) {
        if (target == null) {
            throw new IllegalArgumentException("Target cannot be null");
        }
        Property prop = target.getProperty(propertyName);
        if (prop == null) {
            throw new IllegalArgumentException("Cannot find property " + propertyName);
        }
        initialize(target, prop, values);
    }

    public PropertyAnimation(String propertyName, Object... values) {
        if (propertyName == null) {
            throw new IllegalArgumentException("Specify property name cannot be null");
        }
        mPropertyName = propertyName;
        mValues = values;
    }

    public PropertyAnimation(Actor target, Property property, Object... values) {
        initialize(target, property, values);
    }

    public PropertyAnimation(Property property, Object... values) {
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

        if (mValues[0] instanceof Float) {
            mInterpolator = new Interpolator() {
                float mStart = (Float) mValues[0];
                float mEnd = (Float) mValues[1];

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        float value = mStart + progress * (mEnd - mStart);
                        mTarget.setValue(mProperty, value);
                    }
                }
            };
        } else if (mValues[0] instanceof Integer) {
            mInterpolator = new Interpolator() {
                int mStart = (Integer) mValues[0];
                int mEnd = (Integer) mValues[1];

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        int value = mStart + (int) (progress * (mEnd - mStart));
                        mTarget.setValue(mProperty, value);
                    }
                }
            };
        } else if (mValues[0] instanceof Point) {
            if (((Point) mValues[0]).isNormalized != ((Point)mValues[1]).isNormalized) {
                throw new IllegalArgumentException("Cannot animate between normalized and unnormalized position");
            }

            mInterpolator = new Interpolator() {
                Point mStart = (Point) mValues[0];
                Point mEnd = (Point) mValues[1];
                Point mValue = new Point(mEnd);

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        mValue.set(mStart.x + progress * (mEnd.x - mStart.x), mStart.y + progress * (mEnd.y - mStart.y), mStart.z + progress * (mEnd.z - mStart.z));
                        mTarget.setValue(mProperty, mValue);
                    }
                }
            };
        } else if (mValues[0] instanceof Scale) {
            mInterpolator = new Interpolator() {
                Scale mStart = (Scale) mValues[0];
                Scale mEnd = (Scale) mValues[1];
                Scale mValue = new Scale();

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        mValue.set(mStart.x + progress * (mEnd.x - mStart.x), mStart.y + progress * (mEnd.y - mStart.y), mStart.z + progress * (mEnd.z - mStart.z));
                        mTarget.setValue(mProperty, mValue);
                    }
                }
            };
        } else if (mValues[0] instanceof Color) {
            mInterpolator = new Interpolator() {
                Color mStart = (Color) mValues[0];
                Color mEnd = (Color) mValues[1];
                Color mValue = new Color();

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        mValue.red = mStart.red + (int)(progress * (mEnd.red - mStart.red));
                        mValue.green = mStart.green + (int)(progress * (mEnd.green - mStart.green));
                        mValue.blue = mStart.blue + (int)(progress * (mEnd.blue - mStart.blue));
                        mValue.alpha = mStart.alpha + (int)(progress * (mEnd.alpha - mStart.alpha));
                        mTarget.setValue(mProperty, mValue);
                    }
                }
            };
        } else if (mValues[0] instanceof Rotation) {
            mInterpolator = new Interpolator() {
                Rotation mStart = (Rotation) mValues[0];
                Rotation mEnd = (Rotation) mValues[1];
                Rotation mValue = new Rotation();

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        if (mEnd.getMode() == Rotation.MODE_XYZ_EULER) {
                            float[] euler1 = mStart.getEulerAngles();
                            float[] euler2 = mEnd.getEulerAngles();
                            float x = euler1[0] + progress * (euler2[0] - euler1[0]);
                            float y = euler1[1] + progress * (euler2[1] - euler1[1]);
                            float z = euler1[2] + progress * (euler2[2] - euler1[2]);
                            mValue.set(x, y, z);

                        } else if (mEnd.getMode() == Rotation.MODE_AXIS_ANGLE) {
                            float angle1 = mStart.getAxisAngle();
                            Point v1 = mStart.getAxis();
                            float angle2 = mEnd.getAxisAngle();
                            Point v2 = mEnd.getAxis();

                            mValue.set(
                                v1.x + progress * (v2.x - v1.x),
                                v1.y + progress * (v2.y - v1.y),
                                v1.z + progress * (v2.z - v1.z),
                                angle1 + progress * (angle2 - angle1));
                        }
                        mTarget.setValue(mProperty, mValue);
                        if (mStart.getMode() != mEnd.getMode()) {
                            Log.w(TAG, "Warning: mixed angle interpolation");
                        }
                    }
                }
            };
        } else if (mValues[0] instanceof Stage.Camera) {
            mInterpolator = new Interpolator() {
                Stage.Camera mStart = (Stage.Camera) mValues[0];
                Stage.Camera mEnd = (Stage.Camera) mValues[1];
                Stage.Camera mValue = new Stage.Camera(mStart.position, mStart.lookAt);

                public void onAlphaUpdate(float progress) {
                    if (progress <= 1) {
                        mValue.position.set(
                                mStart.position.x + progress * (mEnd.position.x - mStart.position.x),
                                mStart.position.y + progress * (mEnd.position.y - mStart.position.y),
                                mStart.position.z + progress * (mEnd.position.z - mStart.position.z));
                        mValue.lookAt.set(
                                mStart.lookAt.x + progress * (mEnd.lookAt.x - mStart.lookAt.x),
                                mStart.lookAt.y + progress * (mEnd.lookAt.y - mStart.lookAt.y),
                                mStart.lookAt.z + progress * (mEnd.lookAt.z - mStart.lookAt.z));
                        mTarget.setValue(mProperty, mValue);
                    }
                }
            };
        } else {
            throw new Ngin3dException("Property is not animatable");
        }
        mAlpha.addListener(mInterpolator);
    }

    protected void onStarted() {
        mTarget.onAnimationStarted(mProperty.getName(), this);
        if ((mOptions & START_TARGET_WITH_INITIAL_VALUE) != 0) {
            if (getDirection() == FORWARD) {
                mTarget.setValue(mProperty, mValues[0]);
            } else {
                mTarget.setValue(mProperty, mValues[1]);
            }
        }
    }

    protected void onPaused() {
        mTarget.onAnimationStopped(mProperty.getName());
    }

    protected void onCompleted(int direction) {
        if ((mOptions & Animation.BACK_TO_START_POINT_ON_COMPLETED) == 0) {
            if (direction == Timeline.FORWARD) {
                mTarget.setValue(mProperty, mValues[1]);
            } else {
                mTarget.setValue(mProperty, mValues[0]);
            }
        } else {
            if (direction == Timeline.FORWARD) {
                mTarget.setValue(mProperty, mValues[0]);
            } else {
                mTarget.setValue(mProperty, mValues[1]);
            }
        }
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

    @Override
    public Animation setTarget(Actor target) {
        if (target == null) {
            throw new IllegalArgumentException("Target cannot be null");
        }

        if (mProperty == null && mPropertyName == null) {
            // It's impossible to go this line
            throw new IllegalArgumentException("Property and property name can not both null");
        } else {

            // Need to remove old mInterpolator before changing target
            mAlpha.removeListener(mInterpolator);
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

        return this;
    }

    @Override
    public Actor getTarget() {
        return mTarget;
    }

    /**
     * Clone the PropertyAnimation, value in each member of cloned animation is same of original one, except target.
     * Mew instance of PropertyAnimation has no target in default.
     * @return the cloned PropertyAnimation
     */
    @Override
    public PropertyAnimation clone() {
        PropertyAnimation animation = (PropertyAnimation) super.clone();
        animation.mTarget = null;
        return animation;
    }

}
