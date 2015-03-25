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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.common.view.tests.animation;

import com.mediatek.common.view.tests.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;
import dalvik.annotation.ToBeFixed;

import android.app.ListActivity;
import android.content.Context;
import android.content.res.XmlResourceParser;
import android.test.ActivityInstrumentationTestCase2;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.view.animation.LayoutAnimationController;
import android.view.animation.ScaleAnimation;
import android.view.animation.Transformation;
import android.view.animation.LayoutAnimationController.AnimationParameters;
import android.widget.AbsListView;
import android.widget.ListView;

@TestTargetClass(LayoutAnimationController.class)
public class LayoutAnimationControllerTest
        extends ActivityInstrumentationTestCase2<LayoutAnimStubActivity> {

    private ListActivity mActivity;
    private Animation mDefaultAnimation;
    private ListView mListView;
    private LayoutAnimationController mController;
    /** Duration defined in layout_anim_controller_animation.xml is 1000 */
    private static final int DURATION = 1000;
    private static final float DELTA = 0.1f;
    private static final int INDEX_OF_CHILD1 = 0;
    private static final int INDEX_OF_CHILD2 = 1;
    private static final int INDEX_OF_CHILD3 = 2;
    /** Default delay of LayoutAnimationController */
    private static final float DEFAULT_DELAY = 0.5f;
    /** Default max duration of these three children */
    private static final long DEFAULT_MAX_DURATION = 2000;

    public LayoutAnimationControllerTest() {
        super("com.android.cts.stub", LayoutAnimStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
        mListView = mActivity.getListView();
        mDefaultAnimation = AnimationUtils.loadAnimation(mActivity,
                R.anim.layout_anim_controller_animation);
        mController = new LayoutAnimationController(mDefaultAnimation, DEFAULT_DELAY);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getOrder",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setOrder",
            args = {int.class}
        )
    })
    public void testAccessOrder() throws InterruptedException {

        mController.setOrder(LayoutAnimationController.ORDER_NORMAL);
        assertEquals(LayoutAnimationController.ORDER_NORMAL, mController.getOrder());

        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);

        Animation childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        Animation childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        Animation childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();

        // Test normal order
        long startTime = childAnimation1.getStartTime();
        assertEquals(0, childAnimation1.getStartOffset());
        assertEquals(500, childAnimation2.getStartOffset());
        assertEquals(1000, childAnimation3.getStartOffset());

        Transformation transformation1 = new Transformation();
        Transformation transformation2 = new Transformation();
        Transformation transformation3 = new Transformation();

        // child1 has started animation, child2 and child3 haven't started
        childAnimation1.getTransformation(startTime + 500, transformation1);
        childAnimation2.getTransformation(startTime + 500, transformation2);
        childAnimation3.getTransformation(startTime + 500, transformation3);
        assertIsRunningAnimation(transformation1.getAlpha());
        assertEquals(0.0f, transformation2.getAlpha(), DELTA);
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child2 has started animation, child1 has finished and child3 hasn't started
        childAnimation1.getTransformation(startTime + 1000, transformation1);
        childAnimation2.getTransformation(startTime + 1000, transformation2);
        childAnimation3.getTransformation(startTime + 1000, transformation3);
        assertEquals(1.0f, transformation1.getAlpha(), DELTA);
        assertIsRunningAnimation(transformation2.getAlpha());
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child3 has started animation, child1, child2 have finished
        childAnimation1.getTransformation(startTime + 1500, transformation1);
        childAnimation2.getTransformation(startTime + 1500, transformation2);
        childAnimation3.getTransformation(startTime + 1500, transformation3);
        assertEquals(1.0f, transformation1.getAlpha(), DELTA);
        assertEquals(1.0f, transformation2.getAlpha(), DELTA);
        assertIsRunningAnimation(transformation3.getAlpha());

        // Test reverse order
        mController.setOrder(LayoutAnimationController.ORDER_REVERSE);
        assertEquals(LayoutAnimationController.ORDER_REVERSE, mController.getOrder());
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);

        transformation1 = new Transformation();
        transformation2 = new Transformation();
        transformation3 = new Transformation();
        childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();
        startTime = childAnimation1.getStartTime();
        assertEquals(1000, childAnimation1.getStartOffset());
        assertEquals(500, childAnimation2.getStartOffset());
        assertEquals(0, childAnimation3.getStartOffset());

        // child3 has started animation, child1 and child2 haven't started
        childAnimation1.getTransformation(startTime + 500, transformation1);
        childAnimation2.getTransformation(startTime + 500, transformation2);
        childAnimation3.getTransformation(startTime + 500, transformation3);
        assertEquals(0.0f, transformation1.getAlpha(), DELTA);
        assertEquals(0.0f, transformation2.getAlpha(), DELTA);
        assertIsRunningAnimation(transformation3.getAlpha());

        // child2 has started animation, child3 has finished and child1 hasn't started
        childAnimation1.getTransformation(startTime + 1000, transformation1);
        childAnimation2.getTransformation(startTime + 1000, transformation2);
        childAnimation3.getTransformation(startTime + 1000, transformation3);
        assertEquals(0.0f, transformation1.getAlpha(), DELTA);
        assertIsRunningAnimation(transformation2.getAlpha());
        assertEquals(1.0f, transformation3.getAlpha(), DELTA);

        // child1 has started animation, child2 and child3 has finished
        childAnimation1.getTransformation(startTime + 1500, transformation1);
        childAnimation2.getTransformation(startTime + 1500, transformation2);
        childAnimation3.getTransformation(startTime + 1500, transformation3);
        assertIsRunningAnimation(transformation1.getAlpha());
        assertEquals(1.0f, transformation2.getAlpha(), DELTA);
        assertEquals(1.0f, transformation3.getAlpha(), DELTA);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getDelay",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setDelay",
            args = {float.class}
        )
    })
    public void testAccessDelay() throws InterruptedException {
        mController.setOrder(LayoutAnimationController.ORDER_NORMAL);
        float delay = 1.5f;
        mController.setDelay(delay);
        assertEquals(delay, mController.getDelay());
        long maxDuration = (long) (delay * INDEX_OF_CHILD3 * DURATION + DURATION);
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                maxDuration);

        Animation childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        Animation childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        Animation childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();

        long startTime = childAnimation1.getStartTime();
        long offsetTime1 = childAnimation1.getStartOffset();
        long offsetTime2 = childAnimation2.getStartOffset();
        long offsetTime3 = childAnimation3.getStartOffset();
        // child animation delay = child index * delay * animation duration
        assertEquals(0, offsetTime1);
        assertEquals(1500, offsetTime2);
        assertEquals(3000, offsetTime3);

        Transformation transformation1 = new Transformation();
        Transformation transformation2 = new Transformation();
        Transformation transformation3 = new Transformation();

        // child1 has started animation, child2 and child3 haven't started
        childAnimation1.getTransformation(startTime + 500, transformation1);
        childAnimation2.getTransformation(startTime + 500, transformation2);
        childAnimation3.getTransformation(startTime + 500, transformation3);
        assertIsRunningAnimation(transformation1.getAlpha());
        assertEquals(0.0f, transformation2.getAlpha(), DELTA);
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child1 has finished, child2 and child3 haven't started
        childAnimation1.getTransformation(startTime + 1200, transformation1);
        childAnimation2.getTransformation(startTime + 1200, transformation2);
        childAnimation3.getTransformation(startTime + 1200, transformation3);
        assertEquals(1.0f, transformation1.getAlpha(), DELTA);
        assertEquals(0.0f, transformation2.getAlpha(), DELTA);
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child1 has finished, child2 has started animation, child3 hasn't started.
        childAnimation1.getTransformation(startTime + 2000, transformation1);
        childAnimation2.getTransformation(startTime + 2000, transformation2);
        childAnimation3.getTransformation(startTime + 2000, transformation3);
        assertEquals(1.0f, transformation1.getAlpha(), DELTA);
        assertIsRunningAnimation(transformation2.getAlpha());
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child1 and child2 have finished, child3 hasn't started
        childAnimation1.getTransformation(startTime + 2700, transformation1);
        childAnimation2.getTransformation(startTime + 2700, transformation2);
        childAnimation3.getTransformation(startTime + 2700, transformation3);
        assertEquals(1.0f, transformation1.getAlpha(), DELTA);
        assertEquals(1.0f, transformation2.getAlpha(), DELTA);
        assertEquals(0.0f, transformation3.getAlpha(), DELTA);

        // child1 and child2 have finished, child3 has started animation
        childAnimation1.getTransformation(startTime + 3500, transformation1);
        childAnimation2.getTransformation(startTime + 3500, transformation2);
        childAnimation3.getTransformation(startTime + 3500, transformation3);
        assertIsRunningAnimation(transformation3.getAlpha());
    }

    private void assertIsRunningAnimation(float alpha) {
        assertTrue(alpha > 0.0f);
        assertTrue(alpha < 1.0f);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getAnimation",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setAnimation",
            args = {Animation.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setAnimation",
            args = {Context.class, int.class}
        )
    })
    public void testAccessAnimation() throws InterruptedException {
        Animation animation = AnimationUtils.loadAnimation(mActivity, R.anim.decelerate_alpha);
        animation.setFillAfter(true);
        // duration defined in decelerate_alpha.xml is 2000
        long duration = 2000;
        mController.setAnimation(animation);
        assertSame(animation, mController.getAnimation());
        long maxDuration = (long) (DEFAULT_DELAY * INDEX_OF_CHILD3 * duration + duration);
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                maxDuration);

        Animation childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        Animation childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        Animation childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();
        assertAnimation(childAnimation1, false, duration);
        assertAnimation(childAnimation2, false, duration);
        assertAnimation(childAnimation3, false, duration);

        mController.setAnimation(mActivity, R.anim.layout_anim_controller_animation);
        Animation actualAnimation = mController.getAnimation();
        assertEquals(DURATION, actualAnimation.getDuration());
        assertTrue(actualAnimation.getInterpolator() instanceof AccelerateInterpolator);
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);

        childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();
        assertAnimation(childAnimation1, true, DURATION);
        assertAnimation(childAnimation2, true, DURATION);
        assertAnimation(childAnimation3, true, DURATION);
    }

    /**
     * This method is used for asserting alpha of accelerate and decelerate animations to
     * make sure their accelerate/decelerate functionalities really work.
     * @param animation
     * @param isAccelerate
     * @param duration
     */
    private void assertAnimation(Animation animation, boolean isAccelerate, long duration) {
        Transformation transformation = new Transformation();
        long baseTime = animation.getStartTime() + animation.getStartOffset();
        animation.getTransformation(baseTime, transformation);
        long step = duration / 4;
        float alpha1 = transformation.getAlpha();
        animation.getTransformation(baseTime + step * 1, transformation);
        float alpha2 = transformation.getAlpha();
        animation.getTransformation(baseTime + step * 2, transformation);
        float alpha3 = transformation.getAlpha();
        animation.getTransformation(baseTime + step * 3, transformation);
        float alpha4 = transformation.getAlpha();
        animation.getTransformation(baseTime + step * 4, transformation);
        float alpha5 = transformation.getAlpha();

        // check decelerating delta alpha
        float delta1 = alpha2 - alpha1;
        float delta2 = alpha3 - alpha2;
        float delta3 = alpha4 - alpha3;
        float delta4 = alpha5 - alpha4;
        if (isAccelerate) {
            assertTrue(delta1 < delta2);
            assertTrue(delta2 < delta3);
            assertTrue(delta3 < delta4);
        } else {
            assertTrue(delta1 > delta2);
            assertTrue(delta2 > delta3);
            assertTrue(delta3 > delta4);
        }
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getInterpolator",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setInterpolator",
            args = {Interpolator.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setInterpolator",
            args = {Context.class, int.class}
        )
    })
    public void testAccessInterpolator() throws InterruptedException {
        DecelerateInterpolator interpolator = new DecelerateInterpolator(1.0f);
        mController.setInterpolator(interpolator);
        assertSame(interpolator, mController.getInterpolator());
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);

        Animation childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        Animation childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        Animation childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();

        long delta1 = childAnimation2.getStartOffset() - childAnimation1.getStartOffset();
        long delta2 = childAnimation3.getStartOffset() - childAnimation2.getStartOffset();
        assertTrue(delta2 < delta1);

        mController.setInterpolator(mActivity, android.R.anim.accelerate_interpolator);
        assertTrue(mController.getInterpolator() instanceof AccelerateInterpolator);
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);

        childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();

        delta1 = childAnimation2.getStartOffset() - childAnimation1.getStartOffset();
        delta2 = childAnimation3.getStartOffset() - childAnimation2.getStartOffset();
        assertTrue(delta2 > delta1);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "LayoutAnimationController",
            args = {Context.class, AttributeSet.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "LayoutAnimationController",
            args = {Animation.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "LayoutAnimationController",
            args = {Animation.class, float.class}
        )
    })
    public void testConstructor() {
        XmlResourceParser parser = mActivity.getResources().getAnimation(
                R.anim.accelerate_decelerate_alpha);
        AttributeSet attrs = Xml.asAttributeSet(parser);
        new LayoutAnimationController(mActivity, attrs);
        new LayoutAnimationController(mDefaultAnimation, DEFAULT_DELAY);
        LayoutAnimationController controller = new LayoutAnimationController(mDefaultAnimation);
        assertEquals(DEFAULT_DELAY, controller.getDelay());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "getDelayForView",
        args = {View.class}
    )
    public void testGetDelayForView() throws Throwable {
        Animation animation = AnimationUtils.loadAnimation(mActivity, R.anim.decelerate_alpha);
        animation.setFillAfter(true);
        MyLayoutAnimationController controller = new MyLayoutAnimationController(animation);

        // child1's animationParams
        final AbsListView.LayoutParams layoutParams1 = setAnimationParameters(0);
        // child2's animationParams
        final AbsListView.LayoutParams layoutParams2 = setAnimationParameters(1);
        // child3's animationParams
        final AbsListView.LayoutParams layoutParams3 = setAnimationParameters(2);

        final View child1 = mListView.getChildAt(INDEX_OF_CHILD1);
        final View child2 = mListView.getChildAt(INDEX_OF_CHILD2);
        final View child3 = mListView.getChildAt(INDEX_OF_CHILD3);
        runTestOnUiThread(new Runnable() {
            public void run() {
                child1.setLayoutParams(layoutParams1);
                child2.setLayoutParams(layoutParams2);
                child3.setLayoutParams(layoutParams3);
            }
        });

        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, controller,
                DEFAULT_MAX_DURATION);

        assertEquals(0, controller.getDelayForView(child1));
        assertEquals(1000, controller.getDelayForView(child2));
        assertEquals(2000, controller.getDelayForView(child3));
    }

    private AbsListView.LayoutParams setAnimationParameters(int index) {
        AnimationParameters animationParams = new AnimationParameters();
        animationParams.index = index;
        animationParams.count = 3;
        final AbsListView.LayoutParams layoutParams = new AbsListView.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        layoutParams.layoutAnimationParameters = animationParams;
        return layoutParams;
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "getTransformedIndex",
        args = {AnimationParameters.class}
    )
    public void testGetTransformedIndex() {
        Animation animation = AnimationUtils.loadAnimation(mActivity, R.anim.decelerate_alpha);
        animation.setFillAfter(true);
        MyLayoutAnimationController controller = new MyLayoutAnimationController(animation);
        AnimationParameters animationParams = new AnimationParameters();
        animationParams.count = 3;

        // Test getTransformedIndex in ORDER_NORMAL condition, child1_index == 0, child2_index == 1,
        // child3_index == 2
        animationParams.index = 0;
        assertEquals(0, controller.getTransformedIndex(animationParams));
        animationParams.index = 1;
        assertEquals(1, controller.getTransformedIndex(animationParams));
        animationParams.index = 2;
        assertEquals(2, controller.getTransformedIndex(animationParams));
        // Test getTransformedIndex in ORDER_REVERSE condition, child1_index == 2,
        // child2_index == 1, child3_index == 0
        controller.setOrder(LayoutAnimationController.ORDER_REVERSE);
        animationParams.index = 0;
        assertEquals(2, controller.getTransformedIndex(animationParams));
        animationParams.index = 1;
        assertEquals(1, controller.getTransformedIndex(animationParams));
        animationParams.index = 2;
        assertEquals(0, controller.getTransformedIndex(animationParams));
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "start",
        args = {}
    )
    public void testStart() {
        Animation animation = new ScaleAnimation(0.0f, 10.0f, 0.0f, 20.0f);
        animation.setStartTime(500);
        LayoutAnimationController controller = new LayoutAnimationController(animation);

        assertTrue(Animation.START_ON_FIRST_FRAME != controller.getAnimation().getStartTime());

        controller.start();
        assertEquals(Animation.START_ON_FIRST_FRAME, controller.getAnimation().getStartTime());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "isDone",
        args = {}
    )
    @ToBeFixed(bug = "1799434", explanation = "isDone() always return true")
    public void testIsDone() throws InterruptedException {
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);
        assertTrue(mController.isDone());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "getAnimationForView",
        args = {View.class}
    )
    public void testGetAnimationForView() throws InterruptedException {
        Animation animation = AnimationUtils.loadAnimation(mActivity, R.anim.decelerate_alpha);
        animation.setFillAfter(true);
        mController.setAnimation(animation);
        AnimationTestUtils.assertRunController(getInstrumentation(), mListView, mController,
                DEFAULT_MAX_DURATION);
        Animation childAnimation1 = mListView.getChildAt(INDEX_OF_CHILD1).getAnimation();
        Animation childAnimation2 = mListView.getChildAt(INDEX_OF_CHILD2).getAnimation();
        Animation childAnimation3 = mListView.getChildAt(INDEX_OF_CHILD3).getAnimation();
        // duration defined in decelerate_alpha.xml is 2000
        long duration = 2000;
        assertAnimation(childAnimation1, false, duration);
        assertAnimation(childAnimation2, false, duration);
        assertAnimation(childAnimation3, false, duration);
        assertEquals(0, childAnimation1.getStartOffset());
        assertEquals(1000, childAnimation2.getStartOffset());
        assertEquals(2000, childAnimation3.getStartOffset());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "willOverlap",
        args = {}
    )
    public void testWillOverlap() {
        LayoutAnimationController controller = new LayoutAnimationController(mDefaultAnimation);

        controller.setDelay(0.5f);
        assertTrue(controller.willOverlap());

        controller.setDelay(1.0f);
        assertFalse(controller.willOverlap());

        controller.setDelay(1.5f);
        assertFalse(controller.willOverlap());
    }

    private class MyLayoutAnimationController extends LayoutAnimationController {
        public MyLayoutAnimationController(Animation animation) {
            super(animation);
        }

        protected int getTransformedIndex(AnimationParameters params) {
            return super.getTransformedIndex(params);
        }

        protected long getDelayForView(View view) {
            return super.getDelayForView(view);
        }
    }
}
