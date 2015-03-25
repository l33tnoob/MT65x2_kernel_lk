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
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.EulerOrder;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

/**
 * Keyframe animation that animate an actor by specified keyframe data.
 */
public class KeyframeAnimation extends BasicAnimation {
    private static final String TAG = "KeyframeAnimation";
    private final Samples mSamples;
    private KeyframeInterpolator mInterpolator;
    private Actor mTarget;
    private boolean mNormalized;

    public KeyframeAnimation(KeyframeData kfData) {
        this(null, kfData);
    }

    public KeyframeAnimation(Actor target, KeyframeData kfData) {
        int duration;
        mSamples = kfData.getSamples();
        mNormalized = kfData.isNormalized();

        setTarget(target);
        float[] time = mSamples.get(Samples.KEYFRAME_TIME);
        if (time == null) {
            duration = 0;
        } else {
            duration = (int) (time[time.length - 1] * 1000);
        }
        setDuration(duration);
        setupTimelineListener();
    }

    private void setupTimelineListener() {
        mTimeline.addListener(new Timeline.Listener() {
            public void onStarted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is started", MasterClock.getTime(), KeyframeAnimation.this));
                }
                if (mTarget != null) {
                    mTarget.onAnimationStarted(getAnimationKey(mSamples), KeyframeAnimation.this);
                }

                if ((mOptions & START_TARGET_WITH_INITIAL_VALUE) != 0) {
                    if (getDirection() == FORWARD) {
                        onAnimate(0);
                    } else {
                        onAnimate(timeline.getOriginalDuration());
                    }
                }
            }

            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                onAnimate((float) elapsedMsecs);
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                // KeyframeAnimation onMarkerReached callback function
            }

            public void onPaused(Timeline timeline) {
                if (mTarget != null) {
                    mTarget.onAnimationStopped(getAnimationKey(mSamples));
                }
            }

            public void onCompleted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is completed, target is %s", MasterClock.getTime(), KeyframeAnimation.this, mTarget));
                }
                if ((mOptions & Animation.BACK_TO_START_POINT_ON_COMPLETED) == 0) {
                    if (getDirection() == Timeline.FORWARD) {
                        onAnimate(timeline.getOriginalDuration());
                    } else {
                        onAnimate(0);
                    }
                } else {
                    if (getDirection() == Timeline.FORWARD) {
                        onAnimate(0);
                    } else {
                        onAnimate(timeline.getOriginalDuration());
                    }
                }

                if (mTarget != null) {
                    mTarget.onAnimationStopped(getAnimationKey(mSamples));
                }
            }

            public void onLooped(Timeline timeline) {
                // do nothing now
            }
        });
    }

    private void onAnimate(float timeMs) {
        if (mTarget == null) {
            return;
        }

        float currTime = timeMs / 1000;
        Object value = mInterpolator.getValue(currTime);
        if (value == null)
            return;

        switch (mSamples.getType()) {
        case Samples.ANCHOR_POINT:
            if (mTarget instanceof Plane) {
                ((Plane)mTarget).setAnchorPoint((Point)value);
            } else if (mTarget instanceof BitmapText) {
                ((BitmapText)mTarget).setAnchorPoint((Point)value);
            }
            break;

        case Samples.TRANSLATE:
            if (mNormalized) {
                ((Point)value).isNormalized = true;
            }
            mTarget.setPosition((Point)value);
            break;

        case Samples.ROTATE:
            Rotation rot = (Rotation)value;
            if ((mOptions & Animation.LOCK_Z_ROTATION) != 0) {
                float[] newValue = rot.getEulerAngles(EulerOrder.ZYX);
                float[] target = mTarget.getRotation().getEulerAngles(EulerOrder.ZYX);
                rot.set(EulerOrder.ZYX, newValue[0], newValue[1], target[2]);
            }
            mTarget.setRotation(rot);
            break;

        case Samples.X_ROTATE:
            mTarget.setRotation((Rotation)value);
            break;

        case Samples.Y_ROTATE:
            mTarget.setRotation((Rotation)value);
            break;

        case Samples.Z_ROTATE:
            if ((mOptions & Animation.LOCK_Z_ROTATION) == 0) {
                mTarget.setRotation((Rotation)value);
            }
            break;

        case Samples.SCALE:
            mTarget.setScale((Scale)value);
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

    private static String getAnimationKey(Samples samples) {
        String value = "";
        switch (samples.getType()) {
        case Samples.ANCHOR_POINT:
            value = "anchor";
            break;

        case Samples.TRANSLATE:
            value = "position";
            break;

        case Samples.ROTATE:
            break;
        case Samples.X_ROTATE:
            break;
        case Samples.Y_ROTATE:
            break;
        case Samples.Z_ROTATE:
            value = "rotation";
            break;

        case Samples.SCALE:
            value = "scale";
            break;

        case Samples.ALPHA:
            value = "alpha";
            break;

        default:
            throw new IllegalArgumentException("Unknown samples type: " + samples.getType());
        }
        return value;
    }

    @Override
    public final Animation setTarget(Actor target) {
        mTarget = target;
        // Avoid two Actors reference the same mValue when animation switch target actor and restart in one tick.
        mInterpolator =  new KeyframeInterpolator(mSamples);
        return this;
    }

    @Override
    public Actor getTarget() {
        return mTarget;
    }

    @Override
    public Animation reset() {
        super.reset();
        if (getDirection() == FORWARD) {
            onAnimate(0);
        } else {
            onAnimate(getOriginalDuration());
        }

        return this;
    }

    @Override
    public Animation complete() {
        super.complete();
        if (getDirection() == FORWARD) {
            onAnimate(getOriginalDuration());
        } else {
            onAnimate(0);
        }
        return this;
    }

    /**
     * Clone the KeyframeAnimation, value in each member of cloned animation is same of original one, except target.
     * Mew instance of KeyframeAnimation has no target in default.
     * @return the cloned KeyframeAnimation
     */
    @Override
    public KeyframeAnimation clone() {
        KeyframeAnimation animation = (KeyframeAnimation) super.clone();
        animation.setupTimelineListener();
        animation.mTarget = null;
        return animation;
    }
}
