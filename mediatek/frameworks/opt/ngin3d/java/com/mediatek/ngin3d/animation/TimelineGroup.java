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

import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The group of timeline.
 *
 * @hide This level of detail should not be exposed in an abstract API
 */
public class TimelineGroup extends Timeline implements Timeline.Owner {
    /**
     * Construct a timeline with specified duration.
     *
     * @param duration in milliseconds
     */
    public TimelineGroup(int duration) {
        super(duration);
    }

    @Override
    protected boolean isComplete() {
        return super.isComplete() && mTimelines.isEmpty();
    }

    private void stopAndComplete() {
        stop();
        onComplete(mCurrentTickTime);
    }

    @Override
    public void doTick(long tickTime) {
        for (Timeline timeline : mTimelines) {
            timeline.doTick(tickTime);
        }

        if (mWaitingFirstTick) {
            mStartedTickTime = tickTime;
            mLastFrameTickTime = tickTime;
            mWaitingFirstTick = false;
        } else {
            long delta = tickTime - mLastFrameTickTime;
            if (updateDeltaTime(delta)) {
                doFrame(tickTime);
            }
        }

        // Automatically stop the group when all children are stopped
        // and it's not a loop timeline.
        if (isStarted() && mTimelines.isEmpty() && !mLoop) {
            stopAndComplete();
        }
    }

    @Override
    protected boolean updateDeltaTime(long delta) {
        if (delta < 0) {
            return false; // skip one frame
        } else if (delta != 0) {
            mLastFrameTickTime += delta;
            /**
             * In the case that TimelineGroup is completed but the children in it doesn't,
             * we need to accumulate mDeltaTime to make Marker callback work correctly.
             */
            if (super.isComplete()) {
                mDeltaTime += (int) (delta * mTimeScale);
            } else {
                mDeltaTime = (int) (delta * mTimeScale);
            }
        }
        return true;
    }

    @Override
    protected boolean onComplete(long tickTime) {
        if (mDirection == FORWARD) {
            mElapsedTime = mDuration;
        } else {
            mElapsedTime = 0;
        }
        return super.onComplete(tickTime);

    }

    private final CopyOnWriteArrayList<Timeline> mTimelines = new CopyOnWriteArrayList<Timeline>();

    public void register(Timeline timeline) {
        if (timeline == null) {
            throw new IllegalArgumentException("timeline cannot be null");
        }
        mTimelines.add(timeline);
    }

    public void unregister(Timeline timeline) {
        mTimelines.remove(timeline);
    }

    public void attach(Timeline timeline) {
        timeline.setOwner(this);
    }

    public void detach(Timeline timeline) {
        timeline.setOwner(null);
    }

    public boolean isEmpty() {
        return mTimelines.isEmpty();
    }

    @Override
    public void freeze() {
        super.freeze();
        for (Timeline timeline : mTimelines) {
            timeline.freeze();
        }
    }

    @Override
    public void unfreeze() {
        super.unfreeze();
        for (Timeline timeline : mTimelines) {
            timeline.unfreeze();
        }
    }
}
