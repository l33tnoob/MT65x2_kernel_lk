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
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Transaction;
import com.mediatek.ngin3d.utils.Ngin3dException;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Queue;
import java.util.Stack;

/**
 * Represents a group of animations that can be started and stopped simultaneously. Note that an animation group
 * can be added to another animation group.
 */
public class AnimationGroup extends BasicAnimation {
    private static final String TAG = "AnimationGroup";
    private Actor mTarget;
    private ArrayList<Animation> mAnimations = new ArrayList<Animation>();
    private static final String START = "{Start}";
    private static final String STOP = "{Stop}";

    private TimelineGroup mTimelineGroup;

    /**
     * Initialize this class.
     */
    public AnimationGroup() {
        super(new TimelineGroup(0), Mode.LINEAR);
        mTimelineGroup = (TimelineGroup) mTimeline;
        setupTimelineListener();
    }

    private void setupTimelineListener() {
        mTimeline.addListener(new Timeline.Listener() {
            public void onStarted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: AnimationGroup %s is started", MasterClock.getTime(), AnimationGroup.this));
                }
                applyOnStartedFlags();
                if (mTarget != null) {
                    mTarget.onAnimationStarted(AnimationGroup.this.toString(), AnimationGroup.this);
                }
            }

            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                // do nothing
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                if (marker.equals(STOP) && direction == BACKWARD) {
                    for (Animation animation : mAnimations) {
                        int duration = animation.getDuration();
                        if (duration >= elapsedMsecs) {
                            action(animation, elapsedMsecs, direction);
                        }
                    }
                }
            }

            public void onPaused(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: AnimationGroup %s is paused", MasterClock.getTime(), AnimationGroup.this));
                }
                if (mTarget != null) {
                    mTarget.onAnimationStopped(AnimationGroup.this.toString());
                }
            }

            public void onCompleted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: AnimationGroup %s is completed", MasterClock.getTime(), AnimationGroup.this));
                }
                if ((mOptions & Animation.BACK_TO_START_POINT_ON_COMPLETED) != 0) {
                    for (Animation animation : mAnimations) {
                        animation.setProgress(0);
                    }
                }
                applyOnCompletedFlags();
            }

            public void onLooped(Timeline timeline) {
                Runnable runnable = new Runnable() {
                    public void run() {
                        if (isStarted()) {
                            // Align all children animations before reverse animation group
                            alignAnimation();
                            for (Animation animation : mAnimations) {
                                animation.setDirection(getDirection());
                            }
                            AnimationGroup.this.startAll();
                        }
                    }
                };
                runCallback(runnable);
            }
        });

    }

    /**
     * Adds a animation object into this object.
     *
     * @param animation the animation object to be added.
     * @return the result animation group.
     */
    public AnimationGroup add(Animation animation) {
        if (animation == null) {
            throw new IllegalArgumentException("Animation cannot be null.");
        }

        mAnimations.add(animation);

        int duration = animation.getDuration();
        if (duration != 0) {
            if (duration > getDuration()) {
                mTimeline.setDuration(duration);
            }
        }

        return this;
    }

    /**
     * Removes the animation from this object.
     *
     * @param animation the animation to be removed.
     * @return the result animation group
     */
    public AnimationGroup remove(Animation animation) {
        if (animation == null) {
            throw new IllegalArgumentException("Animation cannot be null.");
        }
        if (isStarted()) {
            Log.w(TAG, String.format("Remove children animation %d when animation group %d is starting",
                animation.getTag(), getTag()));
        }
        mAnimations.remove(animation);
        // Re-calculate the duration of animation group
        if (animation.getDuration() == getDuration()) {
            calculateDuration();
        }

        return this;
    }

    private void calculateDuration() {
        mTimeline.setDuration(0);
        for (Animation ani : mAnimations) {
            int duration = ani.getDuration();
            if (duration != 0) {
                if (duration > getDuration()) {
                    mTimeline.setDuration(duration);
                }
            }
        }
    }

    /**
     * Clear all children in the Group.
     *
     * @return This object
     */
    public AnimationGroup clear() {
        mAnimations.clear();
        return this;
    }

    /**
     * Gets the number of children animations in this object.
     *
     * @return the number of animations.
     */
    public int getAnimationCount() {
        return mAnimations.size();
    }

    /**
     * Gets the number of descendant animations in this object.
     *
     * @return the number of animations.
     */
    public int getDescendantCount() {
        synchronized (this) {
            int count = mAnimations.size();
            for (Animation animation : mAnimations) {
                if (animation instanceof AnimationGroup) {
                    count += ((AnimationGroup) animation).getDescendantCount();
                }
            }
            return count;
        }
    }

    /**
     * Gets the specific animation with the index.
     *
     * @param index the index to be used for searching.
     * @return the specific animation object.
     */
    public Animation getAnimation(int index) {
        return mAnimations.get(index);
    }

    /**
     * Gets the specific animation with the tag from children of animation group.
     *
     * @param index tag of animation.
     * @return animation object, or null if the specific animation is not existed in this object.
     */
    public Animation getAnimationByTag(int index) {
        for (Animation animation : mAnimations) {
            if (animation.getTag() == index) {
                return animation;
            }
        }
        return null;
    }

    /**
     * Gets the specific animation with the tag from children and grandchildren of animation group.
     *
     * @param tag        tag of animation.
     * @param searchMode 0 is depth first search and 1 is breadth first search, otherwise search first level only.
     * @return animation object, or null if the specific animation is not existed in this object.
     */
    public Animation getAnimationByTag(int tag, int searchMode) {
        if (searchMode == Container.BREADTH_FIRST_SEARCH) {
            return findChildByBFS(tag);
        } else if (searchMode == Container.DEPTH_FIRST_SEARCH) {
            return findChildByDFS(tag);
        } else {
            return getAnimationByTag(tag);
        }
    }

    private Animation findChildByBFS(int tag) {
        Queue<AnimationGroup> queue = new ArrayDeque<AnimationGroup>();
        queue.add(this);
        while (queue.size() > 0) {
            AnimationGroup group = queue.remove();
            int size = group.getAnimationCount();
            for (int i = 0; i < size; i++) {
                Animation ani = group.getAnimation(i);
                if (ani.getTag() == tag) {
                    return group.getAnimation(i);
                }
                if (ani instanceof AnimationGroup) {
                    queue.add((AnimationGroup) ani);
                }
            }
        }
        return null;
    }

    private Animation findChildByDFS(int tag) {
        Stack<Animation> stack = new Stack<Animation>();
        stack.push(this);
        while (stack.size() > 0) {
            Animation popped = stack.pop();
            if (popped.getTag() == tag) {
                return popped;
            }
            if (popped instanceof AnimationGroup) {
                int size = ((AnimationGroup) (popped)).getAnimationCount();
                for (int i = 0; i < size; i++) {
                    stack.push(((AnimationGroup) (popped)).getAnimation(i));
                }
            }
        }
        return null;
    }

    private int mProposedWidth;
    private int mProposedHeight;

    /**
     * Sets the proposed width of the target.
     *
     * @param width the proposed width of the target
     */
    public void setProposedWidth(int width) {
        mProposedWidth = width;
    }

    /**
     * Sets the proposed height of the target.
     *
     * @param height the proposed height of the target
     */
    public void setProposedHeight(int height) {
        mProposedHeight = height;
    }

    /**
     * Gets the the proposed width of the target.
     *
     * @return Proposed width of the target
     */
    public int getProposedWidth() {
        return mProposedWidth;
    }

    /**
     * Gets the proposed height of the target.
     *
     * @return Proposed height of the target
     */
    public int getProposedHeight() {
        return mProposedHeight;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Animation

    /**
     * Start the animation.
     *
     * @return This object
     */
    @Override
    public final Animation start() {
        // Align all children animations before starting
        alignAnimation();
        startAll();
        return this;
    }

    @Override
    public Animation startDragging() {
        // Align all children animations before dragging
        alignAnimation();
        return super.startDragging();
    }

    /**
     * Start the animation group and all its children.
     *
     * @return This object
     */
    private void startAll() {
        try {
            Transaction.beginPropertiesModification();
            for (Animation animation : mAnimations) {
                animation.start();
            }
            super.start();
        } finally {
            Transaction.commit();
        }
        if (Ngin3d.LOG_ANIMATION) {
            Log.d(TAG, "Start group animation tag: " + getTag() + " Original Duration: " + getOriginalDuration() + " Duration:" + getDuration());
        }
    }

    /**
     * Invoke this method before using animation group
     *
     * @return This object
     */
    private void alignAnimation() {
        for (Animation animation : mAnimations) {
            if (animation instanceof AnimationGroup) {
                ((AnimationGroup) animation).alignAnimation();
            }
            BasicAnimation ani = (BasicAnimation) animation;
            if (ani.getDuration() != 0) {
                // Align the duration of child animation with parent's duration
                ani.mTimeline.extendDuration(mTimeline.getDuration());
                if (Ngin3d.LOG_ANIMATION) {
                    Log.d(TAG, "Start animation tag: " + ani.getTag() + " Original Duration: " + ani.getOriginalDuration() + " Duration:" + ani.getDuration());
                }
            }
            mTimelineGroup.attach(ani.mTimeline);
        }
    }

    /**
     * Stops the animation.
     *
     * @return This object
     */
    @Override
    public Animation stop() {
        // We have to stop AnimationGroup first.
        // There is a chance that AnimationGroup stops in doTick() due to all children are stopped.
        // And it will generate onComplete() callback which is unexpected.
        super.stop();
        for (Animation animation : mAnimations) {
            animation.stop();
        }

        if (Ngin3d.LOG_ANIMATION) {
            Log.d(TAG, "Stop animation tag: " + getTag());
        }
        return this;
    }

    /**
     * Reset the animation.
     *
     * @return This object
     */
    @Override
    public Animation reset() {
        for (Animation animation : mAnimations) {
            animation.reset();
        }
        super.reset();
        return this;
    }

    /**
     * Completes the animation.
     *
     * @return This object
     */
    @Override
    public Animation complete() {
        for (Animation animation : mAnimations) {
            animation.complete();
        }
        super.complete();
        return this;
    }

    /**
     * Check if this object  works.
     *
     * @return true if this object work.
     */
    @Override
    public final boolean isStarted() {
        return mTimeline.isStarted();
    }

    /**
     * Sets the target of animation of this object.
     *
     * @param target the target animation apply
     * @return This object
     */
    public Animation setTarget(Actor target) {
        mTarget = target;
        for (Animation animation : mAnimations) {
            animation.setTarget(target);
        }
        return this;
    }

    /**
     * Gets the target of the animation of this object.
     *
     * @return This object
     */
    @Override
    public Animation setTargetVisible(boolean visible) {
        super.setTargetVisible(visible);
        for (Animation animation : mAnimations) {
            animation.setTargetVisible(visible);
        }
        return this;
    }

    @Override
    public Actor getTarget() {
        return mTarget;
    }

    /**
     * Sets the time scale of animation.
     *
     * @param scale the time scale value
     */
    @Override
    public void setTimeScale(float scale) {
        for (Animation animation : mAnimations) {
            animation.setTimeScale(scale);
        }
        super.setTimeScale(scale);
    }

    /**
     * Sets the direction of animations
     *
     * @param direction the direction of animations
     */
    @Override
    public void setDirection(int direction) {
        for (Animation animation : mAnimations) {
            animation.setDirection(direction);
        }
        super.setDirection(direction);
    }

    /**
     * Reverse the animations.
     */
    @Override
    public void reverse() {
        if (getDirection() == FORWARD) {
            setDirection(BACKWARD);
        } else {
            setDirection(FORWARD);
        }
        start();
    }

    @Override
    public void setProgress(float progress) {
        for (Animation animation : mAnimations) {
            animation.setProgress(progress);
        }
        super.setProgress(progress);
    }

    public Animation enableOptions(int options) {
        if (options == START_TARGET_WITH_INITIAL_VALUE) {
            for (Animation animation : mAnimations) {
                animation.enableOptions(options);
            }
        }
        super.enableOptions(options);
        return this;
    }

    public Animation disableOptions(int options) {
        if (options == START_TARGET_WITH_INITIAL_VALUE) {
            for (Animation animation : mAnimations) {
                animation.disableOptions(options);
            }
        }
        super.disableOptions(options);
        return this;
    }

    private void action(Animation animation, int elapsedMsecs, int direction) {
        if (!animation.isStarted()) {
            animation.setDirection(direction);
            if (Ngin3d.DEBUG) {
                Log.d(TAG, String.format("[BACKWARD] elapsedMsecs: %d: start Animation %s ", elapsedMsecs, animation));
            }
            BasicAnimation basicAnimation = (BasicAnimation) animation;
            basicAnimation.mTimeline.rewind();
            mTimelineGroup.attach(basicAnimation.mTimeline);
            animation.start();
        }
    }

    @Override
    public BasicAnimation setMode(Mode mode) {
        throw new Ngin3dException("Can not set animation mode of AnimationGroup");
    }

    @Override
    public BasicAnimation setDuration(int duration) {
        throw new Ngin3dException("Can not specify the duration of AnimationGroup");
    }

    @Override
    public AnimationGroup clone() {
        AnimationGroup animation = (AnimationGroup) super.clone();
        animation.setupTimelineListener();
        animation.mTimelineGroup = (TimelineGroup) mTimelineGroup.clone();

        animation.mAnimations = new ArrayList<Animation>(mAnimations.size());
        for (Animation ani : mAnimations) {
            animation.mAnimations.add(ani.clone());
        }

        return animation;
    }

    public void dump() {
        Log.d(TAG, "Parent animation tag: " + getTag() + " Original Duration : " + getOriginalDuration() + " Duration: " + getDuration());
        for (Animation ani : mAnimations) {
            if (ani instanceof AnimationGroup) {
                ((AnimationGroup) ani).dump();
            }
            Log.d(TAG, "children animation tag: " + ani.getTag() + " Original Duration : " + ani.getOriginalDuration() + " Duration: " + ani.getDuration());
        }
    }
}
