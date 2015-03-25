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
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.ImplicitAnimation;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.animation.MasterClock;
import com.mediatek.ngin3d.animation.PropertyAnimation;

public class AnimationTest extends Ngin3dInstrumentationTestCase {
    @SmallTest
    @UiThreadTest
    public void testBasics() {
        PresentationStubActivity activity = getActivity();

        AnimationLoader.setCacheDir(activity.getCacheDir());
        Animation ani = AnimationLoader.loadAnimation(activity, R.raw.photo_next_enter_photo1_ani);
        assertEquals(getActivity().getResources().getResourceName(R.raw.photo_next_enter_photo1_ani), ani.getName());

        int tag = 111;
        ani.setTag(tag);
        assertEquals(tag, ani.getTag());

        int direction = Animation.BACKWARD;
        ani.setDirection(direction);
        assertEquals(direction, ani.getDirection());

        direction = Animation.FORWARD;
        ani.setDirection(direction);
        assertEquals(direction, ani.getDirection());

        int options = Animation.ACTIVATE_TARGET_ON_COMPLETED;
        ani.enableOptions(options);
        assertEquals("The option should be turned on", options, (ani.getOptions() & options));

        ani.disableOptions(options);
        assertEquals("The option should be turned off", 0, (ani.getOptions() & options));

        Animation.Listener listener = new Animation.Listener() {
        };
        ani.addListener(listener);
        ani.removeListener(listener);

        ani.start();
        assertTrue(ani.isStarted());
        ani.stop();
        assertFalse(ani.isStarted());

        ani.enableOptions(options);
        ani.start();
        assertTrue(ani.isStarted());
        ani.stop();
        assertFalse(ani.isStarted());

        ani.start();
        ani.complete();

        Empty empty = new Empty();
        ani.setTarget(empty);
        assertEquals(empty, ani.getTarget());

        assertEquals(1.0f, ani.getTimeScale());
        ani.setTimeScale(0.5f);
        assertEquals(0.5f, ani.getTimeScale());
    }

    class TestedPropertyAnimation extends PropertyAnimation {

        public static final int IDLE = 0;
        public static final int STARTED = 1;
        public static final int PAUSED = 2;
        public static final int COMPLETED = 3;

        public int mState;

        public TestedPropertyAnimation(Actor target, String propertyName, Object... values) {
            super(target, propertyName, values);

            mState = IDLE;
            addListener(new Animation.Listener() {
                @Override
                public void onStarted(Animation animation) {
                    super.onStarted(animation);
                    mState = STARTED;
                }

                @Override
                public void onPaused(Animation animation) {
                    super.onPaused(animation);
                    mState = PAUSED;
                }

                @Override
                public void onCompleted(Animation animation) {
                    super.onCompleted(animation);
                    mState = COMPLETED;
                }
            });
        }
    }

    @SmallTest
    @UiThreadTest
    public void unfinishedTestGroup() throws InterruptedException {
        MasterClock defClock = MasterClock.getDefault();
        MasterClock clock = new MasterClock();
        MasterClock.setDefault(clock);

        try {
            AnimationGroup rootGroup;
            AnimationGroup subgroup1, subgroup2;
            TestedPropertyAnimation posAnimation, rotAnimation, scaleAnimation, colorAnimation;

            // Setup the group
            Empty target = new Empty();
            rootGroup = new AnimationGroup();
            {
                subgroup1 = new AnimationGroup();
                {
                    posAnimation = new TestedPropertyAnimation(target, "position", new Point(0, 0, 0), new Point(1, 1, 1));
                    posAnimation.setDuration(100);
                    posAnimation.addListener(new Animation.Listener() {
                        @Override
                        public void onCompleted(Animation animation) {
                            super.onCompleted(animation);
                        }
                    });
                    subgroup1.add(posAnimation);

                    rotAnimation = new TestedPropertyAnimation(target, "rotation", new Rotation(0, 0, 0), new Rotation(1, 1, 1));
                    rotAnimation.setDuration(200);
                    subgroup1.add(rotAnimation);
                }
                rootGroup.add(subgroup1);

                subgroup2 = new AnimationGroup();
                {
                    scaleAnimation = new TestedPropertyAnimation(target, "scale", new Scale(1, 1, 1), new Scale(2, 2, 2));
                    scaleAnimation.setDuration(300);
                    subgroup2.add(scaleAnimation);

                    colorAnimation = new TestedPropertyAnimation(target, "color", new Color(0, 0, 0), new Color(255, 255, 255));
                    colorAnimation.setDuration(400);
                    subgroup2.add(colorAnimation);
                }
                rootGroup.add(subgroup2);
            }

            // Start the animation group
            rootGroup.start();
            assertTrue(posAnimation.isStarted());
            assertTrue(rotAnimation.isStarted());
            assertTrue(scaleAnimation.isStarted());
            assertTrue(colorAnimation.isStarted());
            assertTrue(subgroup1.isStarted());
            assertTrue(subgroup2.isStarted());
            assertTrue("Group should be started", rootGroup.isStarted());

            clock.tick(0);
            clock.tick(50);
            clock.tick(100);
            clock.tick(150);
            assertEquals(TestedPropertyAnimation.COMPLETED, posAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, rotAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, scaleAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, colorAnimation.mState);

            clock.tick(250);
            assertEquals(TestedPropertyAnimation.COMPLETED, posAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, rotAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, scaleAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, colorAnimation.mState);

            clock.tick(350);
            assertEquals(TestedPropertyAnimation.COMPLETED, posAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, rotAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, scaleAnimation.mState);
            assertEquals(TestedPropertyAnimation.STARTED, colorAnimation.mState);

            clock.tick(450);
            assertEquals(TestedPropertyAnimation.COMPLETED, posAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, rotAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, scaleAnimation.mState);
            assertEquals(TestedPropertyAnimation.COMPLETED, colorAnimation.mState);

            assertFalse(posAnimation.isStarted());
            assertFalse(rotAnimation.isStarted());
            assertFalse(scaleAnimation.isStarted());
            assertFalse(colorAnimation.isStarted());
            assertFalse(subgroup1.isStarted());
            assertFalse(subgroup2.isStarted());
            assertFalse(rootGroup.isStarted());
        } finally {
            MasterClock.setDefault(defClock);
        }
    }

    public void testImplicitAnimation() throws InterruptedException {
        final Empty empty = new Empty();
        final boolean[] finished = {false};
        getActivity().getStage().add(empty);
        final Point newPos = new Point(100, 100, 100);

        ImplicitAnimation animation = Actor.animate(
            100,
            new Runnable() {
                public void run() {
                    empty.setPosition(newPos);
                }
            },
            new Runnable() {
                public void run() {
                    finished[0] = true;
                }
            });
        animation.waitForCompletion();
        
        assertEquals(empty.getPosition(), newPos);
        assertTrue(finished[0]);
    }
}
