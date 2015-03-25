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

package com.mediatek.ngin3d.tests;

import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.animation.Timeline;
import junit.framework.TestCase;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThan;

public class TimelineTest extends TestCase {

    private static final int DURATION = 2000;

    @SmallTest
    public void testTimeline() {
        Timeline timeline = new Timeline(DURATION);
        assertEquals(DURATION, timeline.getOriginalDuration());
        assertEquals(DURATION, timeline.getDuration());
        assertEquals(0, timeline.getTime());
        assertEquals(0.0f, timeline.getProgress());
        assertFalse(timeline.getLoop());
        assertEquals(0, timeline.getDelta());
        assertEquals(Timeline.FORWARD, timeline.getDirection());
    }

    @SmallTest
    public void testStart() {
        Timeline timeline = new Timeline(DURATION);
        MyListener listener = new MyListener();
        timeline.addListener(listener);

        timeline.start();
        assertTrue(timeline.isStarted());
        assertTrue(timeline.isStarted());
        assertEquals(1, listener.mStarted);

        timeline.pause();
        assertFalse(timeline.isStarted());
        assertEquals(1, listener.mPaused);

        timeline.removeListener(listener);
    }

    @SmallTest
    public void testStop() {
        Timeline timeline = new Timeline(DURATION);
        timeline.start();
        assertTrue(timeline.isStarted());
        timeline.stop();
        assertFalse(timeline.isStarted());
        assertEquals(0.0f, timeline.getProgress());
        timeline.setDirection(Timeline.BACKWARD);
        timeline.start();
        timeline.stop();
        assertEquals(1.0f, timeline.getProgress());
    }

    @SmallTest
    public void testSkip() {
        Timeline timeline = new Timeline(DURATION);
        timeline.advance(DURATION / 2);
        assertEquals(0.5f, timeline.getProgress());
        assertEquals(DURATION / 2, timeline.getTime());
        timeline.start();
        timeline.doTick(0);
        timeline.skip(DURATION / 10);
        assertEquals(0.6f, timeline.getProgress());
    }

    @SmallTest
    public void testRewind() {
        Timeline timeline = new Timeline(DURATION);
        timeline.advance(DURATION / 2);
        timeline.rewind();
        assertEquals(0.0f, timeline.getProgress());
        assertEquals(0, timeline.getTime());
        timeline.setDirection(Timeline.BACKWARD);
        timeline.rewind();
        assertEquals(1.0f, timeline.getProgress());
        assertEquals(DURATION, timeline.getTime());
    }

    @SmallTest
    public void testDoTick() {
        Timeline timeline = new Timeline(DURATION);
        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION / 2);
        assertEquals(DURATION / 2, timeline.getDelta());
        assertEquals(0.5f, timeline.getProgress());
    }

    @SmallTest
    public void testAutoReverse() {
        Timeline timeline = new Timeline(DURATION);
        timeline.setAutoReverse(true);
        assertTrue(timeline.getAutoReverse());
    }

    @SmallTest
    public void testLoop1() {
        Timeline timeline = new Timeline(DURATION);
        timeline.setLoop(true);
        assertTrue(timeline.getLoop());

        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION);
        assertEquals(0, timeline.getTime());
        timeline.doTick(DURATION + 1);
        assertEquals(1, timeline.getTime());

        timeline.stop();
        timeline.setDirection(Timeline.BACKWARD);
        timeline.advance(DURATION);
        timeline.start();
        timeline.doTick(0);
        assertEquals(DURATION, timeline.getTime());
        timeline.doTick(DURATION);
        assertEquals(DURATION, timeline.getTime());
        timeline.doTick(DURATION + 1);
        assertEquals(DURATION - 1, timeline.getTime());
    }

    @SmallTest
    public void testLoop2() {
        Timeline timeline = new Timeline(DURATION);
        timeline.setLoop(true);
        assertTrue(timeline.getLoop());

        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION + 1);
        assertEquals(1, timeline.getTime());
        timeline.doTick(DURATION + 2);
        assertEquals(2, timeline.getTime());

        timeline.stop();
        timeline.setDirection(Timeline.BACKWARD);
        timeline.advance(DURATION);
        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION + 1);
        assertEquals(DURATION - 1, timeline.getTime());
        timeline.doTick(DURATION + 2);
        assertEquals(DURATION - 2, timeline.getTime());
    }

    @SmallTest
    public void testTimeScale() {
        Timeline timeline = new Timeline(DURATION);
        timeline.setTimeScale(0.5f);
        assertEquals(0.5f, timeline.getTimeScale());
        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION / 2);
        assertEquals(DURATION / 4, timeline.getDelta());
        assertEquals(0.25f, timeline.getProgress());
        timeline.doTick(DURATION * 2 - 1);
        assertThat(timeline.getProgress(), is(lessThan(1.0f)));
        assertTrue(timeline.isStarted());
        timeline.doTick(DURATION * 2);
        assertEquals(0.0f, timeline.getProgress());
        assertFalse(timeline.isStarted());
        timeline.start();
        timeline.doTick(0);
        timeline.doTick(DURATION / 2);
    }

    @SmallTest
    public void testAdvance() {
        Timeline timeline = new Timeline(DURATION);
        timeline.advance(-1);
        assertEquals(timeline.getTime(), 0);
        timeline.advance(DURATION + 1);
        assertEquals(timeline.getTime(), DURATION);
    }

    @SmallTest
    public void testComplete() {
        Timeline timeline = new Timeline(DURATION);
        timeline.complete();
        assertEquals(DURATION, timeline.getTime());
        timeline.setDirection(Timeline.BACKWARD);
        timeline.complete();
        assertEquals(0, timeline.getTime());
    }

    @SmallTest
    public void testTimeScaleValues() {
        try {
            Timeline timeline = new Timeline(DURATION);
            timeline.setTimeScale(0.0f);
            fail("IllegalArgumentException expected");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    @SmallTest
    public void testAddMarkerAtTime() throws Exception {
        Timeline timeline = new Timeline(DURATION);
        MyListener listener = new MyListener();
        timeline.addListener(listener);

        assertFalse(timeline.hasMarker("1"));
        timeline.addMarkerAtTime("1", 100);
        assertTrue("Should contain added marker", timeline.hasMarker("1"));
        assertFalse("Should not contain marker that has not been added", timeline.hasMarker("0"));

        timeline.start();
        timeline.advanceToMarker("1");
        assertEquals(100, timeline.getTime());
        timeline.doTick(0);
        timeline.doTick(1);
        assertEquals(0, listener.mMarkersReached);

        timeline.removeMarker("1");
        assertFalse("Marker should be removed already", timeline.hasMarker("1"));

        timeline.addMarkerAtTime("2", 150);
        timeline.doTick(100);
        assertEquals(200, timeline.getTime());
        assertEquals(1, listener.mMarkersReached);

        timeline.addMarkerAtTime("3", 200);
        timeline.addMarkerAtTime("4", 300);
        timeline.advance(DURATION);
        timeline.doTick(2);
        assertEquals(1, listener.mMarkersReached);  // advance will skip markers
    }

    @SmallTest
    public void testRemoveMarker() throws Exception {
        Timeline timeline = new Timeline(DURATION);
        timeline.removeMarker("0");
    }

    @SmallTest
    public void testAddMarkerAtUnexpectedTime1() throws Exception {
        try {
            Timeline timeline = new Timeline(DURATION);
            timeline.addMarkerAtTime("1", DURATION + 1);
            fail("IllegalArgumentException expected");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    @SmallTest
    public void testAddMarkerAtUnexpectedTime2() throws Exception {
        try {
            Timeline timeline = new Timeline(DURATION);
            timeline.addMarkerAtTime("1", -1);
            fail("IllegalArgumentException expected");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    private static class MyListener implements Timeline.Listener {
        public int mStarted;
        public int mNewFrames;
        public int mPaused;
        public int mCompleted;
        public int mMarkersReached;

        public void onStarted(Timeline timeline) {
            mStarted++;
        }

        public void onNewFrame(Timeline timeline, int elapsedMsecs) {
            mNewFrames++;
        }

        public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
            mMarkersReached++;
        }

        public void onPaused(Timeline timeline) {
            mPaused++;
        }

        public void onCompleted(Timeline timeline) {
            mCompleted++;
        }

        public void onLooped(Timeline timeline) {
            // do nothing now
        }
    }
}
