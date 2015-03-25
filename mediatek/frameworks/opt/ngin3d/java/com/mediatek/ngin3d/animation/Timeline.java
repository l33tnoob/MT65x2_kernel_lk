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
import com.mediatek.ngin3d.Ngin3d;

import java.util.ArrayList;
import java.util.List;

/**
 * Class for managing time based events such as animations.
 *
 * @hide This level of detail should not be visible at an abstracted API
 */
public class Timeline implements Cloneable {
    protected static final String TAG = "Timeline";
    public static final int FORWARD = 0;
    public static final int BACKWARD = 1;

    private static final int STATE_PAUSED = 0;
    private static final int STATE_STARTED = 1;
    private static final int STATE_FROZEN = 2;
    private int mState;

    protected int mDuration;
    protected int mOriginalDuration;
    protected int mDirection = FORWARD;
    protected boolean mLoop;
    private boolean mAutoReverse;

    protected long mStartedTickTime;
    protected long mLastFrameTickTime;
    protected long mCurrentTickTime;
    private int mStartedTime;
    protected volatile int mElapsedTime;
    protected int mDeltaTime;

    protected float mTimeScale = 1.0f;
    protected boolean mWaitingFirstTick;

    /**
     * Construct a timeline with specified duration.
     *
     * @param duration in milliseconds
     */
    public Timeline(int duration) {
        setDuration(duration);
        mState = STATE_PAUSED;
    }

    private void setState(int state) {
        if (mState == state) {
            return;
        }

        mState = state;
        switch (mState) {
        case STATE_STARTED:
            setProgress(getProgress());
            mWaitingFirstTick = true;
            for (Listener l : mListeners) {
                l.onStarted(this);
            }

            if (mOwner == null) {
                // Register to master clock to receive tick notification
                MasterClock.register(this);
            } else {
                mOwner.register(this);
            }
            break;

        case STATE_PAUSED:
            for (Listener l : mListeners) {
                l.onPaused(this);
            }

            if (mOwner == null) {
                // Unregister from master clock
                MasterClock.unregister(this);
            } else {
                mOwner.unregister(this);
                mOwner = null;
            }

        synchronized (this) {
            mWaitingFirstTick = false;
            notifyAll();
        }

            break;

        case STATE_FROZEN:
            break;

        default:
            throw new IllegalStateException("Unknown timeline state: " + mState);
        }
    }

    public boolean isStarted() {
        return mState != STATE_PAUSED;
    }

    public void start() {
        if (!isStarted()) {
            setState(STATE_STARTED);
        }
    }

    public void pause() {
        // Restore duration to original one when timeline is paused.
        restoreDuration();
        if (Ngin3d.LOG_ANIMATION) {
            Log.d(TAG, "TAG:" + getTag() + " 2 mDuration:" + mDuration + " mOriginalDuration: " + mOriginalDuration);
        }
        if (isStarted()) {
            setState(STATE_PAUSED);
        }
    }

    public void stop() {
        pause();
        rewind();
    }

    /**
     * Rewinds to the first frame if its direction is FORWARD and the last frame if it is BACKWARD.
     */
    public void rewind() {
        if (mDirection == FORWARD) {
            advance(0);
        } else {
            advance(mDuration);
        }
    }

    public void complete() {
        if (mDirection == FORWARD) {
            advance(mDuration);
        } else {
            advance(0);
        }
    }

    /**
     * Advance timeline by the requested time in milliseconds
     *
     * @param msecs time to skip in milliseconds
     */
    public void skip(int msecs) {
        advance(mElapsedTime + msecs);
    }

    /**
     * Advance timeline to the requested point.
     *
     * @param time in milliseconds since the timeline started.
     */
    public void advance(int time) {
        if (time < 0) {
            mElapsedTime = 0;
        } else if (time > mDuration) {
            mElapsedTime = mDuration;
        } else {
            mElapsedTime = time;
        }

        if (!mWaitingFirstTick) {
            mStartedTickTime = mLastFrameTickTime;
        }
        mStartedTime = mElapsedTime;
    }

    /**
     * The position of the timeline in a [0, 1] interval with original duration.
     *
     * @return progress between [0, 1]
     */
    public float getRealProgress() {
        return calculateProgress(mOriginalDuration);
    }

    /**
     * The position of the timeline in a [0, 1] interval.
     *
     * @return progress between [0, 1]
     */
    public float getProgress() {
        return calculateProgress(mDuration);
    }

    private float calculateProgress(int duration) {
        if (mElapsedTime < 0) {
            return 0;
        } else if (mElapsedTime > duration) {
            return 1;
        }
        return (float) mElapsedTime / duration;
    }

    /**
     * Set the position of the timeline in a [0, 1] interval.
     *
     * @param progress the progress between [0, 1]
     */
    public void setProgress(float progress) {
        if (progress < 0) {
            throw new IllegalArgumentException("progress cannot be nagative");
        }
        mElapsedTime = (int)(mDuration * progress);
        mStartedTime = mElapsedTime;
        notifyNewFrame();
    }

    protected boolean isComplete() {
        return mDirection == FORWARD ? mElapsedTime >= mDuration : mElapsedTime <= 0;
    }

    /**
     * Tick the timeline with specified tick time.
     *
     * @param tickTime tick time
     */
    public void doTick(long tickTime) {
        if (!isStarted()) {
            return;
        }

        if (mWaitingFirstTick) {
            mStartedTickTime = tickTime;
            mLastFrameTickTime = tickTime;
            mWaitingFirstTick = false;
        } else {
            long delta = tickTime - mLastFrameTickTime;
            if (delta > 100) {
                if (Ngin3d.DEBUG) {
                    Log.w(TAG, "Delta time too long: " + delta);
                }
            }
            if (updateDeltaTime(delta)) {
                doFrame(tickTime);
            }
        }
    }

    protected boolean updateDeltaTime(long delta) {
        if (delta < 0) {
            return false; // skip one frame
        } else if (delta != 0) {
            mLastFrameTickTime += delta;
            mDeltaTime = (int) (delta * mTimeScale);
        }
        return true;
    }

    protected boolean doFrame(long tickTime) {
        mCurrentTickTime = tickTime;
        if (mDirection == FORWARD) {
            mElapsedTime = mStartedTime + (int) ((mCurrentTickTime - mStartedTickTime) * mTimeScale);
        } else {
            mElapsedTime = mStartedTime - (int) ((mCurrentTickTime - mStartedTickTime) * mTimeScale);
        }
        if (isComplete()) {
            return onComplete(tickTime);
        } else { // time still ticking
            notifyNewFrame();
            checkIfMarkerHit();
            return !isStarted(); // listens may pause the timeline
        }
    }

    protected boolean onComplete(long tickTime) {
        int savedDirection = mDirection;
        int overflowTime = mElapsedTime;

        if (mDirection == FORWARD) {
            mElapsedTime = mDuration;
        } else {
            mElapsedTime = 0;
        }

        mStartedTime = 0;

        int endTime = mElapsedTime;

        notifyNewFrame();
        checkIfMarkerHit();

        // Listener may play with current time.
        if (mElapsedTime != endTime) {
            return true;
        }

        if (!mLoop && isStarted()) {
            setState(STATE_PAUSED);
        }

        // notify completed
        int listenerCount = mListeners.size();
        for (int i = 0; i < listenerCount; ++i) {
            mListeners.get(i).onCompleted(this);
        }

        // Handle auto reverse
        if (mAutoReverse) {
            if (mDirection == FORWARD) {
                mDirection = BACKWARD;
                mStartedTime = mDuration;
            } else {
                mDirection = FORWARD;
                mStartedTime = 0;
            }
        }

        if (endTime != mElapsedTime
            && !((mElapsedTime == 0 && endTime == mDuration)
                || (mElapsedTime == mDuration && endTime == 0))) {
            return true;
        }

        if (mLoop) {
            // interpolate smoothly around a loop
            if (savedDirection == FORWARD) {
                // overflowTime >= mDuration
                mElapsedTime = overflowTime - mDuration;
                mStartedTickTime = tickTime - mElapsedTime;
            } else {
                // overflowTime <= 0
                mElapsedTime = mDuration + overflowTime;
                mStartedTickTime = tickTime + overflowTime;
            }

            if (savedDirection != mDirection) {
                mElapsedTime = mDuration - mElapsedTime;
            }

            checkIfMarkerHit();

            // notify looped
            listenerCount = mListeners.size();
            for (int i = 0; i < listenerCount; ++i) {
                mListeners.get(i).onLooped(this);
            }

            // Detect and do the correction of error case
            if (mElapsedTime >= mDuration && savedDirection == FORWARD) {
                Log.w(TAG, "Error case happened with Forward, correct it. "
                    + "mStartedTickTime: " + mStartedTickTime + " mElapsedTime: " + mElapsedTime + " tickTime:" + tickTime);
                mStartedTickTime = tickTime;
                mElapsedTime = 0;
            } else if (mElapsedTime < 0 && savedDirection == BACKWARD) {
                Log.w(TAG, "Error case happened with backward, correct it. "
                    + "mStartedTickTime: " + mStartedTickTime + " mElapsedTime: " + mElapsedTime + " tickTime:" + tickTime);
                mStartedTickTime = tickTime;
                mElapsedTime = mDuration;
            }

            return true;
        } else {
            // if not looping, the timeline is stopped and we should restore duration to original one.
            restoreDuration();
            rewind();
            return false;
        }

    }

    private void restoreDuration() {
        mDuration = mOriginalDuration;
    }

    private void notifyNewFrame() {
        // Use indexing rather than iterator to prevent frequent GC
        for (int i = 0, size = mListeners.size(); i < size; ++i) {
            mListeners.get(i).onNewFrame(this, mElapsedTime);
        }
    }

    public int getOriginalDuration() {
        return mOriginalDuration;
    }

    public int getDuration() {
        return mDuration;
    }

    public final void setDuration(int originalDuration) {
        if (originalDuration < 0) {
            throw new IllegalArgumentException("Running duration can not be negative.");
        }
        mOriginalDuration = originalDuration;
        mDuration = originalDuration;
    }

    public final void extendDuration(int extendedDuration) {
        if (extendedDuration < 0) {
            throw new IllegalArgumentException("Progress duration can not be negative.");
        }
        mDuration = extendedDuration;
    }

    public int getTime() {
        if (mElapsedTime < 0) {
            return 0;
        } else if (mElapsedTime > mDuration) {
            return mDuration;
        }
        return mElapsedTime;
    }

    public int getDirection() {
        return mDirection;
    }

    public void setDirection(int direction) {
        if (direction != mDirection) {
            mDirection = direction;
            if (!isStarted()) {
                rewind();
            }
            mStartedTime = mElapsedTime;
            mStartedTickTime = mCurrentTickTime;
        }
    }

    public int getDelta() {
        if (mDirection == FORWARD) {
            return mDeltaTime;
        } else {
            return -mDeltaTime;
        }
    }

    public void reverse() {
        if (mDirection == FORWARD) {
            setDirection(BACKWARD);
        } else {
            setDirection(FORWARD);
        }
    }

    public void setLoop(boolean loop) {
        mLoop = loop;
    }

    public boolean getLoop() {
        return mLoop;
    }

    public boolean getAutoReverse() {
        return mAutoReverse;
    }

    public void setAutoReverse(boolean autoReverse) {
        mAutoReverse = autoReverse;
    }

    public float getTimeScale() {
        return mTimeScale;
    }

    public void setTimeScale(float timeScale) {
        if (timeScale <= 0.0) {
            throw new IllegalArgumentException("time scale cannot be zero or negative.");
        }
        mStartedTime = mElapsedTime;
        mStartedTickTime = mCurrentTickTime;
        mTimeScale = timeScale;
    }

    /**
     * Freeze the timeline
     */
    public void freeze() {
        if (isStarted()) {
            setState(STATE_FROZEN);
        }
    }

    /**
     * Unfreeze the timeline from specific tick time.
     *
     */
    public void unfreeze() {
        if (mState == STATE_FROZEN) {
            mStartedTime = mElapsedTime;
            mWaitingFirstTick = true;
            mState = STATE_STARTED;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Listener

    /**
     * Listener to handle timeline events.
     */
    public interface Listener {
        /**
         * Notify a timeline is started.
         *
         * @param timeline the timeline
         */
        void onStarted(Timeline timeline);

        /**
         * Notify a new time frame.
         *
         * @param timeline
         * @param elapsedMsecs
         */
        void onNewFrame(Timeline timeline, int elapsedMsecs);

        /**
         * Notify a marker is reached.
         *
         * @param timeline     the timeline
         * @param elapsedMsecs the elapsed time
         * @param marker       the marker name
         */
        void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction);

        /**
         * Notify the timeline is paused or stopped.
         *
         * @param timeline the timeline
         */
        void onPaused(Timeline timeline);

        /**
         * Notify the completion of time. It will be called multiple times if looping is enabled.
         *
         * @param timeline the timeline
         */
        void onCompleted(Timeline timeline);

        /**
         * Notify the timeline is looped. It will be called when starting to loop.
         *
         * @param timeline the timeline
         */
        void onLooped(Timeline timeline);
    }

    private ArrayList<Listener> mListeners = new ArrayList<Listener>();

    public void addListener(Listener l) {
        mListeners.add(l);
    }

    public void removeListener(Listener l) {
        mListeners.remove(l);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Markers

    public static final class Marker {
        public String name;
        public int time;

        public Marker(String name, int time) {
            this.name = name;
            this.time = time;
        }
    }

    ArrayList<Marker> mMarkers;

    private List<Marker> getMarkers() {
        if (mMarkers == null) {
            mMarkers = new ArrayList<Marker>();
        }
        return mMarkers;
    }

    private Marker getMarker(String name) {
        if (mMarkers == null) {
            return null;
        }

        List<Marker> markers = getMarkers();
        for (int i = 0, size = markers.size(); i < size; i++) {
            Marker m = markers.get(i);
            if (name.equals(m.name)) {
                return m;
            }
        }

        return null;
    }

    public void addMarkerAtTime(String name, int time) {
        if (time > mDuration || time < 0) {
            throw new IllegalArgumentException("time exceeds duration");
        }
        Marker m = new Marker(name, time);
        getMarkers().add(m);
    }

    public boolean hasMarker(String name) {
        return getMarker(name) != null;
    }

    public void removeMarker(String name) {
        if (mMarkers == null) {
            return;
        }

        List<Marker> markers = getMarkers();
        for (int i = 0, size = markers.size(); i < size; ++i) {
            if (name.equals(markers.get(i).name)) {
                markers.remove(i);
                return;
            }
        }
    }

    public void advanceToMarker(String name) {
        Marker m = getMarker(name);
        if (m != null) {
            advance(m.time);
        }
    }

    private boolean hasPassedTime(Marker m) {
        final int t = m.time;
        if (t < 0 || t > mDuration) {
            return false;
        }
        if (mDirection == FORWARD) {
            if (t == 0 && mDeltaTime > 0 && mElapsedTime - mDeltaTime <= 0) {
                return true;
            }
            return t > mElapsedTime - mDeltaTime && t <= mElapsedTime;
        } else {
            if (t == mDuration && mDeltaTime > 0 && mElapsedTime + mDeltaTime >= mDuration) {
                return true;
            }
            return t >= mElapsedTime && t < mElapsedTime + mDeltaTime;
        }
    }

    private void checkIfMarkerHit() {
        if (mMarkers == null) {
            return;
        }

        List<Marker> markers = getMarkers();
        for (int i = 0, size = markers.size(); i < size; i++) {
            Marker m = markers.get(i);
            if (hasPassedTime(m)) {
                for (int j = 0, listenersCount = mListeners.size(); j < listenersCount; ++j) {
                    mListeners.get(j).onMarkerReached(this, mElapsedTime, m.name, mDirection);
                }
            }
        }
    }

    public interface Owner {
        void register(Timeline timeline);

        void unregister(Timeline timeline);
    }

    private Owner mOwner;

    public void setOwner(Owner owner) {
        mOwner = owner;
    }

    public void waitForCompletion() throws InterruptedException {
        synchronized (this) {
            while (isStarted()) {
                wait();
            }
        }
    }

    /**
     * Clone the Timeline, the value of each field in cloned animation is same of original one, except timeline state.
     * The state of cloned timeline is paused.
     * @return the cloned Timeline
     */
    @Override
    protected Timeline clone() {
        try {
            Timeline timeline = (Timeline) super.clone();
            timeline.mListeners = new ArrayList<Listener>();
            timeline.mState = STATE_PAUSED;
            return timeline;
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }

    }

    protected int mTag;
    public void setTag(int tag) {
        mTag = tag;
    }

    public int getTag() {
        return mTag;
    }

}
