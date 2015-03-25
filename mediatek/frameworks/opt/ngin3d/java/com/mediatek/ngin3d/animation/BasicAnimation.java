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
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.UiHandler;

/**
 * Basic animation that has its own timeline and alpha.
 */
public abstract class BasicAnimation extends Animation {
    private static final String TAG = "BasicAnimation";

    public static final int DEFAULT_DURATION = 2000;

    protected Timeline mTimeline;
    protected Alpha mAlpha;

    public BasicAnimation() {
        this(DEFAULT_DURATION, Mode.LINEAR);
    }

    public BasicAnimation(int duration, Mode mode) {
        this(new Timeline(duration), mode);
    }

    public BasicAnimation(Timeline timeline, Mode mode) {
        setTimeline(timeline);
        setAlphaMode(mode);
    }

    private void setTimeline(Timeline timeline) {
        mTimeline = timeline;
        mTimeline.addListener(new Timeline.Listener() {
            public void onStarted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is started", MasterClock.getTime(), BasicAnimation.this));
                }

                applyOnStartedFlags();
                synchronized (mListeners) {
                    int size = mListeners.size();
                    for (int i = 0; i < size; i++) {
                        final Listener l = mListeners.get(i);
                        Runnable runnable = new Runnable() {
                            public void run() {
                                l.onStarted(BasicAnimation.this);
                            }
                        };
                        runCallback(runnable);
                    }
                }
            }

            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                // Call back each of the registered listeners as a new thread

                final int t = elapsedMsecs;
                synchronized (mListeners) {
                    int size = mListeners.size();
                    for (int i = 0; i < size; i++) {
                        final Listener l = mListeners.get(i);
                        Runnable runnable = new Runnable() {
                            public void run() {
                                l.onNewFrame(BasicAnimation.this, t);
                            }
                        };
                        runCallback(runnable);
                    }
                }
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Marker [%s] of Animation %s is reached", MasterClock.getTime(), marker, BasicAnimation.this));
                }
                final String m = marker;
                final int d = direction;
                synchronized (mListeners) {
                    int size = mListeners.size();
                    for (int i = 0; i < size; i++) {
                        final Listener l = mListeners.get(i);
                        Runnable runnable = new Runnable() {
                            public void run() {
                                l.onMarkerReached(BasicAnimation.this, d, m);
                            }
                        };
                        runCallback(runnable);
                    }
                }
            }

            public void onPaused(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is paused", MasterClock.getTime(), BasicAnimation.this));
                }
                synchronized (mListeners) {
                    int size = mListeners.size();
                    for (int i = 0; i < size; i++) {
                        final Listener l = mListeners.get(i);
                        Runnable runnable = new Runnable() {
                            public void run() {
                                l.onPaused(BasicAnimation.this);
                            }
                        };
                        runCallback(runnable);
                    }
                }
            }

            public void onCompleted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is completed", MasterClock.getTime(), BasicAnimation.this));
                }

                applyOnCompletedFlags();
                synchronized (mListeners) {
                    int size = mListeners.size();
                    for (int i = 0; i < size; i++) {
                        final Listener l = mListeners.get(i);
                        Runnable runnable = new Runnable() {
                            public void run() {
                                l.onCompleted(BasicAnimation.this);
                            }
                        };
                        runCallback(runnable);
                    }
                }
            }

            public void onLooped(Timeline timeline) {
                // do nothing now
            }
        });

    }

    private void setAlphaMode(Mode mode) {
        mAlpha = new Alpha(mTimeline, mode);
    }

    protected final void runCallback(Runnable runnable) {
        UiHandler uiHandler = Stage.getUiHandler();
        if (uiHandler == null) {
            runnable.run();
        } else {
            uiHandler.post(runnable);
        }
    }

    public void addMarkerAtTime(String name, int time) {
        mTimeline.addMarkerAtTime(name, time);
    }

    public Mode getMode() {
        return mAlpha.getMode();
    }

    public BasicAnimation setMode(Mode mode) {
        mAlpha.setMode(mode);
        return this;
    }

    public BasicAnimation setAutoReverse(boolean autoReverse) {
        mTimeline.setAutoReverse(autoReverse);
        return this;
    }

    public boolean getAutoReverse() {
        return mTimeline.getAutoReverse();
    }

    /**
     * Set duration of animation.
     *
     * @param duration in milliseconds
     * @return the animation itself
     */
    public BasicAnimation setDuration(int duration) {
        mTimeline.setDuration(duration);
        return this;
    }

    public int getDuration() {
        return mTimeline.getDuration();
    }

    public int getOriginalDuration() {
        return mTimeline.getOriginalDuration();
    }

    public void setProgress(float progress) {
        mTimeline.setProgress(progress);
    }

    public float getProgress() {
        return mTimeline.getProgress();
    }

    public BasicAnimation setLoop(boolean loop) {
        mTimeline.setLoop(loop);
        return this;
    }

    public boolean getLoop() {
        return mTimeline.getLoop();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Animation

    @Override
    public Animation start() {
        if (Ngin3d.LOG_ANIMATION) {
            Log.d(TAG, "Animation start, tag: " + getTag());
        }
        Actor target = getTarget();
        if (target != null || (mOptions & CAN_START_WITHOUT_TARGET) != 0) {
            mTimeline.start();
        }

        return this;
    }

    @Override
    public Animation startDragging() {
        disableOptions(Animation.START_TARGET_WITH_INITIAL_VALUE);
        setDirection(Animation.FORWARD);
        setProgress(0);
        setTargetVisible(true);
        return this;
    }

    @Override
    public Animation pause() {
        mTimeline.pause();
        return this;
    }

    @Override
    public Animation stop() {
        if (Ngin3d.LOG_ANIMATION) {
            Log.d(TAG, "Animation stop, tag: " + getTag());
        }
        mTimeline.stop();
        return this;
    }

    @Override
    public Animation stopDragging() {
        start();
        return this;
    }

    @Override
    public Animation reset() {
        mTimeline.rewind();
        return this;
    }

    @Override
    public Animation complete() {
        mTimeline.complete();
        return this;
    }

    @Override
    public boolean isStarted() {
        return mTimeline.isStarted();
    }

    @Override
    public void setTimeScale(float scale) {
        mTimeline.setTimeScale(scale);
    }

    @Override
    public float getTimeScale() {
        return mTimeline.getTimeScale();
    }

    @Override
    public void setDirection(int direction) {
        mTimeline.setDirection(direction);
    }

    @Override
    public int getDirection() {
        return mTimeline.getDirection();
    }

    @Override
    public void reverse() {
        mTimeline.reverse();
    }

    @Override
    public Animation setTargetVisible(boolean visible) {
        Actor target = getTarget();
        if (target != null) {
            target.setVisible(visible);
        }
        return this;
    }

    /**
     * Wait for an animation to complete. A stopped animation is treated as a completed one.
     *
     * @throws InterruptedException if the thread is interrupted
     */
    public void waitForCompletion() throws InterruptedException {
        mTimeline.waitForCompletion();
    }

    /**
     * Clone the BasicAnimation, value in each member of cloned BasicAnimation is same of original one, except Timeline and Alpha.
     * Mew instance of Timeline and Alpha will be created for cloned BasicAnimation.
     * @return the cloned animation
     */
    @Override
    public BasicAnimation clone() {
        BasicAnimation animation = (BasicAnimation) super.clone();
        animation.setTimeline(mTimeline.clone());
        animation.setAlphaMode(mAlpha.getMode());
        return animation;
    }

    public void setTag(int tag) {
        super.setTag(tag);
        mTimeline.setTag(tag);
    }
}
