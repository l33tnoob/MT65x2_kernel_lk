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

import android.content.res.Resources;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Timeline;
import com.mediatek.ngin3d.presentation.IObject3d;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ObjectSource;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.utils.Ngin3dException;

import android.util.Log;

/**
 * Abstract base class for Actors representing 3D objects.
 */
public abstract class Object3D extends Actor {
    /**
     * @hide
     */
    public static final String TAG = "Object3D";

    /**
     * @hide
     */
    public static final String DEFAULT = "default";
    private ObjectSource mObjectSource;
    private Object3DAnimation mAnimation;

    /** @hide */
    public static final String PROPNAME_NODE_ROTATION = "node_rotation";

    /** @hide */
    public static final Property<Boolean> PROP_ANIMATION_PLAYING =
        new Property<Boolean>("animation_playing", false);

    /** @hide */
    public static final Property<Float> PROP_ANIMATION_PROGRESS =
        new Property<Float>("animation_progress", 0.0f);

    /**
     * @hide
     * Looping does not have a default value, because Glo animations have
     * looping enabled depending on whether they have a loop, and we have no
     * way to get this value before the actor is realized.
     */
    public static final Property<Boolean> PROP_ANIMATION_LOOP_ENABLED =
        new Property<Boolean>("animation_loop_enabled", null);

    /** @hide */
    public static final Property<Float> PROP_ANIMATION_SPEED =
        new Property<Float>("animation_speed", 1.0f);

    /**
     * @hide
     */
    @Override
    protected IObject3d createPresentation(PresentationEngine engine) {
        IObject3d iObject3d = engine.createObject3d();
        iObject3d.setObjectSource(mObjectSource);

        if (mAnimation != null) {
            int lengthMs = (int) (iObject3d.getAnimationLength() * 1000.0f);
            mAnimation.setDuration(lengthMs);
            mAnimation.setLoop(iObject3d.getAnimationLoopEnabled());
            iObject3d.disableNewAnimation();
        }

        return iObject3d;
    }

    /**
     * Returns the Actor's presentation cast to the instantiated type.
     *
     * @hide Presentation API should be internal only
     *
     * @return Presentation object
     */
    @Override
    public IObject3d getPresentation() {
        return (IObject3d) mPresentation;
    }

    /**
     * @hide Presentation API should be internal only
     */
    @Override
    public void realize(PresentationEngine presentationEngine) {
        super.realize(presentationEngine);

        // Once realized, ensure that setting the progress causes the Glo's
        // state to be updated (we just leave it disabled during realization to
        // avoid conflicts between animations affecting the same Glo
        // properties).
        getPresentation().enableApplyProgress();
    }

    /**
     * @hide
     */
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property instanceof KeyPathProperty) {
            KeyPathProperty kp = (KeyPathProperty) property;
            String propertyName = kp.getKey(0);

            if (propertyName.equals(PROPNAME_NODE_ROTATION)) {
                String sceneNodeName = kp.getKey(1);
                if (value != null) {
                    Rotation rotation = (Rotation) value;
                    getPresentation().setRotation(sceneNodeName, rotation);
                }
                return true;
            }
        } else {
            if (property.sameInstance(PROP_ANIMATION_PLAYING)) {
                if (value != null) {
                    getPresentation().setAnimationPlaying((Boolean) value);
                }
                return true;
            } else if (property.sameInstance(PROP_ANIMATION_PROGRESS)) {
                if (value != null) {
                    getPresentation().setAnimationProgress((Float) value);
                }
                return true;
            } else if (property.sameInstance(PROP_ANIMATION_LOOP_ENABLED)) {
                if (value != null) {
                    getPresentation().setAnimationLoopEnabled((Boolean) value);
                }
                return true;
            } else if (property.sameInstance(PROP_ANIMATION_SPEED)) {
                if (value != null) {
                    getPresentation().setAnimationSpeed((Float) value);
                }
                return true;
            }
        }

        return false;
    }

    /**
     * @hide
     */
    @Override
    protected void refreshState() {
        super.refreshState();

        // Set properties without marking them as dirty.
        setValueIfNotDirty(PROP_ANIMATION_PLAYING,
                getPresentation().isAnimationPlaying(), false);
        setValueIfNotDirty(PROP_ANIMATION_PROGRESS,
                getPresentation().getAnimationProgress(), false);
    }

    /**
     * Specify the Object3D object by the object3d file name.
     *
     * @param filename Object3d file name
     */
    public void setObjectFromFile(String filename) {
        if (filename == null) {
            throw new NullPointerException("filename cannot be null");
        }
        mObjectSource = new ObjectSource(ObjectSource.FILE, filename);
    }

    /**
     * Specify the Object3D object by android resource and resource id.
     *
     * @param resources Android resource
     * @param resId     Android resource id
     */
    public void setObjectFromResource(Resources resources, int resId) {
        if (resources == null) {
            throw new NullPointerException("resources cannot be null");
        }
        mObjectSource = new ObjectSource(ObjectSource.RES_ID, new ImageDisplay.Resource(resources, resId));
    }

    /**
     * Specify the Object3D object by the asset name.
     *
     * @param assetName Asset file name
     */
    public void setObjectFromAsset(String assetName) {
        if (assetName == null) {
            throw new NullPointerException("assetname cannot be null");
        }
        mObjectSource = new ObjectSource(ObjectSource.ASSET, assetName);
    }

    public void setRotation(String sceneNodeName, Rotation rotation) {
        setKeyPathValue(PROPNAME_NODE_ROTATION + "." + sceneNodeName, rotation);
    }

    /**
     * Starts or resumes the object's animation playback.
     * <p>
     * If an animation reaches the end of playback, it will be paused (i.e. it
     * will behave as if the pause() function had been called), but the
     * animation will start again from the beginning when play() is next
     * called.
     * <p>
     * <b>Warning</b>
     * Do not play two animations which animate the same properties at
     * the same time, as they will conflict with one another.
     */
    public void play() {
        setValue(PROP_ANIMATION_PLAYING, true);
    }

    /**
     * Pauses the object's animation playback.
     * <p>
     * Pausing an animation will cause it to freeze in place.  While an
     * animation is paused, another animation which animates the same
     * properties as the paused animation may be used.
     */
    public void pause() {
        setValue(PROP_ANIMATION_PLAYING, false);
    }

    /**
     * Halts the object's animation playback and rewinds it to the beginning.
     * Stopping an animation will cause playback to cease, and the object to
     * revert to its state at the beginning of the animation.
     */
    public void stop() {
        pause();
        rewind();
    }

    /**
     * Toggles the whether the object's animation is playing or paused.
     */
    public void togglePlaying() {
        if (getValue(PROP_ANIMATION_PLAYING)) {
            pause();
        } else {
            play();
        }
    }

    /**
     * Returns whether the object's animation is playing or paused.
     * @return True if animation is playing
     */
    public boolean isAnimationPlaying() {
        return getValue(PROP_ANIMATION_PLAYING);
    }

    /**
     * Moves the object's animation to a given playback progress.
     * <p>
     * Progress is given as a value in the range 0 to 1, where 0 is the start
     * of playback range and 1 is the end.
     *
     * @param progress Playback progress (0-1)
     */
    public void setAnimationProgress(float progress) {
        setValue(PROP_ANIMATION_PROGRESS, progress);
    }

    /**
     * Returns the playback progress of the object's animation.
     *
     * @return Playback position (0-1)
     */
    public float getAnimationProgress() {
        return getValue(PROP_ANIMATION_PROGRESS);
    }

    /**
     * Sets the playback of the object's animation to the beginning.
     * <p>
     * Note that that rewinding is not always the same as setting the progress
     * to the start of the animation.  If the animation has speed greater than
     * zero, the beginning is at the start of the animation; if it is less than
     * zero, the beginning is the end of the animation; if the speed is zero,
     * this function does nothing.  This allows rewind() to work intuitively
     * when using animations with negative speeds.
     */
    public void rewind() {
        if (getValue(PROP_ANIMATION_SPEED) >= 0.0f) {
            setAnimationProgress(0.0f);
        } else {
            setAnimationProgress(1.0f);
        }
    }

    /**
     * Sets whether the object's animation should loop over its loop range.
     * @param flag True if animation should loop
     */
    public void setAnimationLoopEnabled(boolean flag) {
        setValue(PROP_ANIMATION_LOOP_ENABLED, flag);
    }

    /**
     * Sets the playback speed multiplier of the object's animation.
     * <p>
     * For example, a speed of 1.0 is normal playback speed, 2.0 is double
     * speed, 0.5 is half speed.  Negative speeds can be used to play the
     * animation in reverse.
     *
     * @param speed Playback speed multiplier
     */
    public void setAnimationSpeed(float speed) {
        setValue(PROP_ANIMATION_SPEED, speed);
    }

    /*
     * The following is the old-style Glo animation system, and should be
     * removed once the new system has been universally adopted.
     */

    /**
     * @deprecated Please use the animation controls directly in Object3D.
     */
    public BasicAnimation getAnimation() {
        if (mAnimation == null) {
            int lengthMs = 0;
            boolean loopEnabled = false;

            if (isRealized()) {
                lengthMs = (int) (getPresentation().getAnimationLength() * 1000.0f);
                loopEnabled = getPresentation().getAnimationLoopEnabled();
                getPresentation().disableNewAnimation();
            }

            mAnimation = new Object3DAnimation(this, lengthMs);
            mAnimation.setLoop(loopEnabled);
        }

        return mAnimation;
    }

    /**
     * @deprecated Please use the animation controls directly in Object3D.
     */
    public BasicAnimation getAnimation(String name) {
        if (name.compareTo(DEFAULT) == 0) {
            return getAnimation();
        }
        return null;
    }


    private void update(float time) {
        if (isRealized()) {
            getPresentation().update(time);
        }
    }

    /**
     * Object3DAnimation is an inner class of Object3D that can operation on Object3D only
     * This class can be created by Object3D only
     */
    private class Object3DAnimation extends BasicAnimation {
        private final Object3D mTarget;
        private long mLoopTime;

        Object3DAnimation(Object3D target, int duration) {
            mTarget = target;
            mTimeline.setDuration(duration);

            mTimeline.addListener(new Timeline.Listener() {
                public void onStarted(Timeline timeline) {
                    mTarget.onAnimationStarted(Object3DAnimation.this.toString(), Object3DAnimation.this);
                }

                public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                    mTarget.update((mLoopTime + elapsedMsecs) / 1000f);
                }

                public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                    // do nothing now
                }

                public void onPaused(Timeline timeline) {
                    mTarget.onAnimationStopped(Object3DAnimation.this.toString());
                }

                public void onCompleted(Timeline timeline) {
                    if (!mAnimation.getLoop()) {
                        float duration = mTimeline.getOriginalDuration();
                        mTarget.setAnimationProgress(1.0f);
                        Log.i(TAG, "Completed: " + duration);
                    }
                }

                public void onLooped(Timeline timeline) {
                    mLoopTime += mTimeline.getDuration();
                }
            });
        }

        @Override
        public Actor getTarget() {
            return mTarget;
        }

        @Override
        public Animation start() {
            super.start();
            mTarget.rewind();
            mTarget.play();
            mTarget.requestRender();
            return this;
        }

        @Override
        public Animation stop() {
            super.stop();
            mTarget.stop();
            mLoopTime = 0;
            return this;
        }

        @Override
        public BasicAnimation setLoop(boolean loop) {
            if (isRealized()) {
                // For performance purpose, avoid loop animation if the duration is 0.
                if (mAnimation.getDuration() == 0) {
                    return this;
                }
            }
            super.setLoop(loop);
            mTarget.setAnimationLoopEnabled(loop);
            return this;
        }

        @Override
        public final Animation setTarget(Actor target) {
            throw new Ngin3dException("Object3DAnimation can not change target.");
        }
    }
}
