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

import android.test.UiThreadTest;
import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.ngin3d.animation.Timeline;
import com.mediatek.ngin3d.utils.Ngin3dException;

import static android.test.MoreAsserts.assertNotEqual;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;

public class AnimationGroupTest extends Ngin3dInstrumentationTestCase {
    private void waitForTime(PresentationStubActivity activity, int millisecond) throws InterruptedException {
        final int delta = 50;
        // Change to continuously render mode to trigger timeline
        mStageView.setRenderMode(StageView.RENDERMODE_CONTINUOUSLY);
        Timeline time = new Timeline(millisecond + delta);
        time.start();
        while (time.isStarted()) {
            try {
                Thread.sleep(30);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        mStageView.setRenderMode(StageView.RENDERMODE_WHEN_DIRTY);
    }

    @SmallTest
    @UiThreadTest
    public void testReverse() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        Animation ani2 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo2_ani);
        Animation ani3 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo3_ani);
        Animation ani4 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);

        ani1.setTarget(empty1);
        ani2.setTarget(empty2);
        ani3.setTarget(empty3);
        ani4.setTarget(empty4);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);

        rootGroup.reverse();
        assertEquals(Animation.BACKWARD, ani1.getDirection());
        assertEquals(Animation.BACKWARD, ani2.getDirection());
        assertEquals(Animation.BACKWARD, ani3.getDirection());
        assertEquals(Animation.BACKWARD, ani4.getDirection());
        assertEquals(Animation.BACKWARD, rootGroup.getDirection());
        assertEquals(670, rootGroup.getDuration());

        assertTrue("Animation should be started", ani1.isStarted());
        assertTrue("Animation should be started", ani2.isStarted());
        assertTrue("Animation should be started", ani3.isStarted());
        assertTrue("Animation should be started", ani4.isStarted());
        assertTrue("Animation should be started", rootGroup.isStarted());

        waitForTime(activity, rootGroup.getDuration());

        assertFalse(ani1.isStarted());
        assertFalse(ani2.isStarted());
        assertFalse(ani3.isStarted());
        assertFalse(ani4.isStarted());
        assertFalse(rootGroup.isStarted());

        rootGroup.reverse();
        assertEquals(Animation.FORWARD, ani1.getDirection());
        assertEquals(Animation.FORWARD, ani2.getDirection());
        assertEquals(Animation.FORWARD, ani3.getDirection());
        assertEquals(Animation.FORWARD, ani4.getDirection());
        assertEquals(Animation.FORWARD, rootGroup.getDirection());

        assertTrue("Animation should be started", ani1.isStarted());
        assertTrue("Animation should be started", ani2.isStarted());
        assertTrue("Animation should be started", ani3.isStarted());
        assertTrue("Animation should be started", ani4.isStarted());
        assertTrue("Animation should be started", rootGroup.isStarted());

        rootGroup.stop();
    }

    @SmallTest
    @UiThreadTest
    public void testPropertyAnimation() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        Rotation start = new Rotation(0, 0, 0);
        Rotation end = new Rotation(0, 0, 360);
        AnimationGroup rootGroup = new AnimationGroup();
        PropertyAnimation ani1 = new PropertyAnimation(empty1, "rotation", start, end);
        ani1.setDuration(1000);
        PropertyAnimation ani2 = new PropertyAnimation(empty2, "rotation", start, end);
        ani2.setDuration(2000);
        PropertyAnimation ani3 = new PropertyAnimation(empty3, "rotation", start, end);
        ani3.setDuration(3000);
        PropertyAnimation ani4 = new PropertyAnimation(empty4, "rotation", start, end);
        ani4.setDuration(4000);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);

        rootGroup.reverse();
        assertEquals(Animation.BACKWARD, ani1.getDirection());
        assertEquals(Animation.BACKWARD, ani2.getDirection());
        assertEquals(Animation.BACKWARD, ani3.getDirection());
        assertEquals(Animation.BACKWARD, ani4.getDirection());
        assertEquals(Animation.BACKWARD, rootGroup.getDirection());
        assertEquals(4000, rootGroup.getDuration());

        assertTrue("Animation should be started", ani1.isStarted());
        assertTrue("Animation should be started", ani2.isStarted());
        assertTrue("Animation should be started", ani3.isStarted());
        assertTrue("Animation should be started", ani4.isStarted());
        assertTrue("Animation should be started", rootGroup.isStarted());

        waitForTime(activity, rootGroup.getDuration());

        assertFalse(ani1.isStarted());
        assertFalse(ani2.isStarted());
        assertFalse(ani3.isStarted());
        assertFalse(ani4.isStarted());
        assertFalse(rootGroup.isStarted());

        rootGroup.reverse();
        assertEquals(Animation.FORWARD, ani1.getDirection());
        assertEquals(Animation.FORWARD, ani2.getDirection());
        assertEquals(Animation.FORWARD, ani3.getDirection());
        assertEquals(Animation.FORWARD, ani4.getDirection());
        assertEquals(Animation.FORWARD, rootGroup.getDirection());

        assertTrue("Animation should be started", ani1.isStarted());
        assertTrue("Animation should be started", ani2.isStarted());
        assertTrue("Animation should be started", ani3.isStarted());
        assertTrue("Animation should be started", ani4.isStarted());
        assertTrue("Animation should be started", rootGroup.isStarted());

        rootGroup.stop();
    }

    public void testProgress() {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        AnimationGroup childGroup = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        Animation ani2 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo2_ani);
        Animation ani3 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo3_ani);
        Animation ani4 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);
        Animation grandchild = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);

        ani1.setTarget(empty1);
        ani2.setTarget(empty2);
        ani3.setTarget(empty3);
        ani4.setTarget(empty4);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);
        rootGroup.add(childGroup);
        childGroup.add(grandchild);

        rootGroup.setProposedHeight(1);
        assertEquals(1, rootGroup.getProposedHeight());

        rootGroup.remove(ani2);
        assertEquals(4, rootGroup.getAnimationCount());

        assertEquals(ani1, rootGroup.getAnimation(0));

        rootGroup.setProposedWidth(1);
        assertEquals(1, rootGroup.getProposedWidth());

        ani1.setTag(100);
        childGroup.setTag(200);
        grandchild.setTag(300);
        assertEquals(ani1, rootGroup.getAnimationByTag(100));
        assertEquals(ani1, rootGroup.getAnimationByTag(100, Container.BREADTH_FIRST_SEARCH));
        assertEquals(ani1, rootGroup.getAnimationByTag(100, Container.DEPTH_FIRST_SEARCH));
        assertEquals(childGroup, rootGroup.getAnimationByTag(200));
        assertEquals(childGroup, rootGroup.getAnimationByTag(200, Container.BREADTH_FIRST_SEARCH));
        assertEquals(childGroup, rootGroup.getAnimationByTag(200, Container.DEPTH_FIRST_SEARCH));
        assertEquals(null, rootGroup.getAnimationByTag(300));
        assertEquals(grandchild, rootGroup.getAnimationByTag(300, Container.BREADTH_FIRST_SEARCH));
        assertEquals(grandchild, rootGroup.getAnimationByTag(300, Container.DEPTH_FIRST_SEARCH));

        rootGroup.setProgress(0.5f);
        assertEquals(0.5f, rootGroup.getProgress());

        rootGroup.reset();
        assertEquals(0.0f, rootGroup.getProgress());

        rootGroup.clear();
        assertEquals(0, rootGroup.getAnimationCount());
    }

    public void testVisible() {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        Animation ani2 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo2_ani);
        Animation ani3 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo3_ani);
        Animation ani4 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);

        ani1.setTarget(empty1);
        ani2.setTarget(empty2);
        ani3.setTarget(empty3);
        ani4.setTarget(empty4);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);

        rootGroup.setTargetVisible(false);
        assertFalse(ani1.getTarget().getVisible());
        assertFalse(ani2.getTarget().getVisible());
        assertFalse(ani3.getTarget().getVisible());
        assertFalse(ani4.getTarget().getVisible());

        rootGroup = rootGroup.clear();
        assertThat(rootGroup.getAnimationCount(), is(0));
    }

    public void testInvalidOperation() {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        ani1.setTarget(empty1);
        rootGroup.add(ani1);

        try {
            rootGroup.setProgress(-0.1f);
            fail("Should throw IllegalArgumentException, progress cannot be nagative");
        } catch (IllegalArgumentException e) {
            // expected
        }

        try {
            rootGroup.setDuration(1000);
            fail("Should throw Ngin3dException, Cannot specify the duration of AnimationGroup");
        } catch (Ngin3dException e) {
            // expected
        }

        try {
            rootGroup.setMode(Mode.EASE_IN_QUAD);
            fail("Should throw Ngin3dException, Cannot specify the mode of AnimationGroup");
        } catch (Ngin3dException e) {
            // expected
        }
    }

    public void testClone() throws CloneNotSupportedException, InterruptedException {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        AnimationGroup group = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        ani.setTarget(empty1);
        group.add(ani);
        group.setName("group");

        AnimationGroup clone = group.clone();
        clone.setTarget(empty2);
        assertEquals("", clone.getName());

        group.setDirection(Animation.BACKWARD);
        clone.setDirection(Animation.FORWARD);
        assertNotEqual(clone.getDirection(), group.getDirection());

        group.setTimeScale(1.5f);
        clone.setTimeScale(2f);
        assertNotEqual(clone.getTimeScale(), group.getTimeScale());

        group.setProgress(0.5f);
        clone.setProgress(0.8f);
        assertNotEqual(clone.getProgress(), group.getProgress());

        group.setLoop(true);
        clone.setLoop(false);
        assertNotEqual(clone.getLoop(), group.getLoop());

        assertNotEqual(clone.getAnimation(0), group.getAnimation(0));
        assertEquals(clone.getAnimationCount(), group.getAnimationCount());
        empty1.setPosition(new Point(0f, 0f, 0f, false));
        empty2.setPosition(new Point(0f, 0f, 0f, false));
        assertEquals(empty1.getPosition(), empty2.getPosition());
        clone.start();
        waitForTime(activity, 200);
        assertNotEqual(empty1.getPosition(), empty2.getPosition());
    }

    public void testDurationWithStop() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        Empty empty5 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        Animation ani2 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo2_ani);
        Animation ani3 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo3_ani);
        Animation ani4 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);

        ani1.setTarget(empty1);
        ani2.setTarget(empty2);
        ani3.setTarget(empty3);
        ani4.setTarget(empty4);

        PropertyAnimation ani5 = new PropertyAnimation(empty5, "position", new Point(0, 0), new Point(200, 200));
        ani5.setDuration(5000);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);
        assertEquals(rootGroup.getDuration(), ani4.getDuration());

        rootGroup.add(ani5);
        assertEquals(rootGroup.getDuration(), ani5.getDuration());

        rootGroup.remove(ani5);
        assertEquals(rootGroup.getDuration(), ani4.getDuration());

        rootGroup.add(ani5);
        rootGroup.start();
        assertEquals(rootGroup.getDuration(), ani1.getDuration());
        assertEquals(rootGroup.getDuration(), ani2.getDuration());
        assertEquals(rootGroup.getDuration(), ani3.getDuration());
        assertEquals(rootGroup.getDuration(), ani4.getDuration());
        assertEquals(rootGroup.getDuration(), ani5.getDuration());

        rootGroup.stop();
        assertEquals(ani1.getDuration(), ani1.getDuration());
        assertEquals(ani2.getDuration(), ani2.getDuration());
        assertEquals(ani3.getDuration(), ani3.getDuration());
        assertEquals(ani4.getDuration(), ani4.getDuration());
        assertEquals(ani5.getDuration(), ani5.getDuration());
    }

    public void testDurationWithComplete() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        Empty empty1 = new Empty();
        Empty empty2 = new Empty();
        Empty empty3 = new Empty();
        Empty empty4 = new Empty();
        Empty empty5 = new Empty();
        AnimationGroup rootGroup = new AnimationGroup();
        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani1 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo1_ani);
        Animation ani2 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo2_ani);
        Animation ani3 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo3_ani);
        Animation ani4 = AnimationLoader.loadAnimation(activity, R.raw.photo_next_exit_photo4_ani);

        ani1.setTarget(empty1);
        ani2.setTarget(empty2);
        ani3.setTarget(empty3);
        ani4.setTarget(empty4);

        PropertyAnimation ani5 = new PropertyAnimation(empty5, "position", new Point(0, 0), new Point(200, 200));
        ani5.setDuration(2000);

        rootGroup.add(ani1);
        rootGroup.add(ani2);
        rootGroup.add(ani3);
        rootGroup.add(ani4);
        rootGroup.add(ani5);
        assertEquals(rootGroup.getDuration(), ani5.getDuration());

        rootGroup.start();
        assertEquals(rootGroup.getDuration(), ani1.getDuration());
        assertEquals(rootGroup.getDuration(), ani2.getDuration());
        assertEquals(rootGroup.getDuration(), ani3.getDuration());
        assertEquals(rootGroup.getDuration(), ani4.getDuration());
        assertEquals(rootGroup.getDuration(), ani5.getDuration());

        assertEquals(400, ani1.getOriginalDuration());
        assertEquals(470, ani2.getOriginalDuration());
        assertEquals(570, ani3.getOriginalDuration());
        assertEquals(670, ani4.getOriginalDuration());
        assertEquals(ani5.getDuration(), ani5.getOriginalDuration());

        // Waiting for rootGroup be completed.
        Thread.sleep(10000);
        assertEquals(ani1.getDuration(), ani1.getDuration());
        assertEquals(ani2.getDuration(), ani2.getDuration());
        assertEquals(ani3.getDuration(), ani3.getDuration());
        assertEquals(ani4.getDuration(), ani4.getDuration());
        assertEquals(ani5.getDuration(), ani5.getDuration());
    }

}
