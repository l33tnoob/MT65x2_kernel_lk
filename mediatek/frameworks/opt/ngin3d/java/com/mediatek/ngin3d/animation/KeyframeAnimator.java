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
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

import java.util.HashMap;

/**
 * Keyframe animator that animate an actor by specified keyframe data.
 * The keyframe animator includes several interpolators. Each interpolator
 * take care of one property.
 */
public class KeyframeAnimator extends ValueAnimator {
    private HashMap<Integer, KeyframeInterpolator> mInterpolatorSet;
    private Actor mTarget;
    private KeyframeDataSet mKfDataSet;

    public KeyframeAnimator(KeyframeDataSet kfDataSet) {
        this(null, kfDataSet);
    }

    public KeyframeAnimator(Actor target, KeyframeDataSet kfDataSet) {
        mKfDataSet = kfDataSet;
        setTarget(target);
        setFloatValues(0f, getDuration());

        addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            public void onAnimationUpdate(ValueAnimator ani) {
                onAnimate((Float) ani.getAnimatedValue());
            }
        });
    }

    private void onAnimate(float timeMs) {
        if (mTarget == null) {
            return;
        }
        float currTime = timeMs / 1000;

        for (int type = 1; type < Samples.MARKER; type++) {
            KeyframeInterpolator interpolator = mInterpolatorSet.get(type);
            if (interpolator == null) {
                break;
            }

            Object value = interpolator.getValue(currTime);
            if (value == null) {
                return;
            }

            switch (type) {
            case Samples.ANCHOR_POINT:
                if (mTarget instanceof Plane) {
                    ((Plane) mTarget).setAnchorPoint((Point) value);
                } else if (mTarget instanceof BitmapText) {
                    ((BitmapText) mTarget).setAnchorPoint((Point) value);
                }
                break;
            case Samples.TRANSLATE:
                if (interpolator.isNormalized()) {
                    ((Point) value).isNormalized = true;
                }
                mTarget.setPosition((Point) value);
                break;
            case Samples.ROTATE:
            case Samples.X_ROTATE:
            case Samples.Y_ROTATE:
            case Samples.Z_ROTATE:
                mTarget.setRotation((Rotation) value);
                break;
            case Samples.SCALE:
                mTarget.setScale((Scale) value);
                break;
            case Samples.ALPHA:
                int opacity = (int) (2.55 * (Float) value);
                mTarget.setOpacity(opacity);
                break;

            default:
                // do nothing.
                break;

            }
        }
    }

    public final void setTarget(Actor target) {
        mTarget = target;
        // Avoid two Actors reference the same Interpolator when animating
        // switch target actor and restart in one tick.
        mInterpolatorSet = new HashMap<Integer, KeyframeInterpolator>();

        for (KeyframeData kfData : mKfDataSet.getList()) {
            // Create Interpolator by keyframe samples
            KeyframeInterpolator interpolator = new KeyframeInterpolator(kfData.getSamples());
            interpolator.setNormalized(kfData.isNormalized());
            mInterpolatorSet.put(interpolator.getType(), interpolator);
            if (interpolator.getDuration() > getDuration()) {
                setDuration(interpolator.getDuration());
            }
        }
    }

    public Actor getTarget() {
        return mTarget;
    }

    /**
     * Clone the KeyframeAnimator, value in each member of cloned animation is same of original one, except target.
     * Mew instance of KeyframeAnimator has no target in default.
     *
     * @return the cloned KeyframeAnimator
     */
    @Override
    public KeyframeAnimator clone() {
        KeyframeAnimator animation = (KeyframeAnimator) super.clone();
        animation.mTarget = null;
        return animation;
    }
}
