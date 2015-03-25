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
 * The clock to drive timelines.
 *
 * @hide This level of detail should not be exposed in an abstract API
 */
public class MasterClock {

    private static MasterClock sMasterClock = new MasterClock();

    private final CopyOnWriteArrayList<Timeline> mTimelines = new CopyOnWriteArrayList<Timeline>();

    private double mTimeScale;

    public static MasterClock getDefault() {
        return sMasterClock;
    }

    public static void setDefault(MasterClock clock) {
        sMasterClock = clock;
    }

    public static void register(Timeline timeline) {
        getDefault().registerTimeline(timeline);
    }

    public static void unregister(Timeline timeline) {
        getDefault().unregisterTimeline(timeline);
    }

    public static void cleanup() {
        getDefault().removeAllTimelines();
    }

    public MasterClock() {
        mTimeScale = 1.0;
    }

    public void registerTimeline(Timeline timeline) {
        if (timeline == null) {
            throw new IllegalArgumentException("timeline cannot be null");
        }
        mTimelines.add(timeline);
    }

    public void unregisterTimeline(Timeline timeline) {
        mTimelines.remove(timeline);
    }

    public boolean isTimelineRegistered(Timeline timeline) {
        return mTimelines.contains(timeline);
    }

    public double getTimeScale() {
        return mTimeScale;
    }

    /**
     * To modify the time scale of master clock for slower or faster animation.
     *
     * @param timeScale < 1.0 for slower and > 1.0 for faster
     */
    public void setTimeScale(double timeScale) {
        if (timeScale < 0.0) {
            throw new IllegalArgumentException("timeScale cannot be nagative");
        }
        mTimeScale = timeScale;
    }

    /**
     * Returns the current value of the most precise available system timer, in milliseconds.
     *
     * @return The current value of the system timer, in milliseconds.
     */
    public static long getTime() {
        return System.nanoTime() / 1000000;
    }

    public long getTickTime() {
        long tickTime = getTime();
        if (mTimeScale != 1.0) {
            tickTime *= mTimeScale;
        }
        return tickTime;
    }

    /**
     * Pause all timelines
     */
    public void pause() {
        for (Timeline timeline : mTimelines) {
            timeline.freeze();
        }
    }

    /**
     * In order to resume rendering and have continuous animation,
     * we need resume all timelines from current tick time.
     */
    public void resume() {
        for (Timeline timeline : mTimelines) {
            timeline.unfreeze();
        }
    }

    /**
     * Called typically in rendering thread to drive timelines.
     */
    public void tick() {
        tick(getTickTime());
    }

    public void tick(long tickTime) {
        for (Timeline timeline : mTimelines) {
            timeline.doTick(tickTime);
        }
    }

    private void removeAllTimelines() {
        mTimelines.clear();
    }
}
