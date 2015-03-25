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

import android.app.Activity;
import android.content.res.XmlResourceParser;
import android.graphics.Matrix;
import android.test.ActivityInstrumentationTestCase2;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.Transformation;
import android.view.animation.TranslateAnimation;

import com.mediatek.common.view.tests.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;

@TestTargetClass(android.view.animation.TranslateAnimation.class)
public class TranslateAnimationTest
        extends ActivityInstrumentationTestCase2<AnimationTestStubActivity> {

    private Activity mActivity;

    private static final long DURATION = 1000;
    private static final float POSITION_DELTA = 0.001f;
    private static final float FROM_X_DETLTA = 0.0f;
    private static final float TO_X_DELTA = 10.0f;
    private static final float FROM_Y_DELTA = 0.0f;
    private static final float TO_Y_DELTA = 20.0f;
    private static final float RELATIVE_FROM_X_DELTA = 0.0f;
    private static final float RELATIVE_TO_X_DELTA = 0.2f;
    private static final float RELATIVE_FROM_Y_DELTA = 0.0f;
    private static final float RELATIVE_TO_Y_DELTA = 0.4f;

    public TranslateAnimationTest() {
        super("com.android.cts.stub", AnimationTestStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "TranslateAnimation",
            args = {android.content.Context.class, android.util.AttributeSet.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "TranslateAnimation",
            args = {float.class, float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "TranslateAnimation",
            args = {float.class, float.class, float.class, float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "TranslateAnimation",
            args = {int.class, float.class, int.class, float.class,
                    int.class, float.class, int.class, float.class}
        )
    })
    public void testConstructors() {

        // Test with null AttributeSet
        new TranslateAnimation(mActivity, null);

        final XmlResourceParser parser = mActivity.getResources().getAnimation(
                R.anim.anim_translate);
        final AttributeSet attr = Xml.asAttributeSet(parser);
        assertNotNull(attr);
        // Test with real AttributeSet
        new TranslateAnimation(mActivity, attr);

        // Test {@link TranslateAnimation#TranslateAnimation(float, float, float, float)}
        new TranslateAnimation(0.6f, 0.6f, 0.6f, 0.6f);
        // Test negative input values
        new TranslateAnimation(-0.6f, -0.6f, -0.6f, -0.6f);

        // Test {@link TranslateAnimation#TranslateAnimation(int, float, int, float, int, float,
        // int, float)}
        new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0.6f, Animation.RELATIVE_TO_SELF, 0.6f,
                Animation.RELATIVE_TO_SELF, 0.6f, Animation.RELATIVE_TO_SELF, 0.6f);
        // Test negative input values
        new TranslateAnimation(Animation.RELATIVE_TO_SELF, -0.6f, Animation.RELATIVE_TO_SELF, -0.6f,
                Animation.RELATIVE_TO_SELF, -0.6f, Animation.RELATIVE_TO_SELF, -0.6f);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "applyTransformation",
                args = {float.class, android.view.animation.Transformation.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            notes = "Test initialize wiht delta type Animation#ABSOLUTE",
            method = "initialize",
            args = {int.class, int.class, int.class, int.class}
        )
    })
    public void testApplyTransformation(){
        final View animWindow = mActivity.findViewById(R.id.anim_window);
        final Transformation transformation = new Transformation();
        final MyTranslateAnimation translateAnimation =
                new MyTranslateAnimation(FROM_X_DETLTA, TO_X_DELTA, FROM_Y_DELTA, TO_Y_DELTA);
        translateAnimation.setDuration(DURATION);
        translateAnimation.setInterpolator(new LinearInterpolator());
        assertFalse(translateAnimation.isInitialized());
        translateAnimation.initialize(0, 0, 0, 0);
        assertTrue(translateAnimation.isInitialized());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, translateAnimation);
        final long startTime = translateAnimation.getStartTime();

        float values[] = new float[9];
        // Test applyTransformation() in method getTransformation()
        translateAnimation.getTransformation(startTime, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(FROM_X_DETLTA, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(FROM_Y_DELTA, values[Matrix.MTRANS_Y], POSITION_DELTA);

        transformation.clear();
        translateAnimation.getTransformation(startTime + DURATION / 2, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals((TO_X_DELTA + FROM_X_DETLTA) / 2, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals((TO_Y_DELTA + FROM_Y_DELTA) / 2, values[Matrix.MTRANS_Y], POSITION_DELTA);

        transformation.clear();
        translateAnimation.getTransformation(startTime + DURATION, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(TO_X_DELTA, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(TO_Y_DELTA, values[Matrix.MTRANS_Y], POSITION_DELTA);

        // Test applyTransformation() directly
        // Test time start
        transformation.clear();
        translateAnimation.applyTransformation(0.0f, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(FROM_X_DETLTA, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(FROM_Y_DELTA, values[Matrix.MTRANS_Y], POSITION_DELTA);

        // Test time of middle 0.5
        transformation.clear();
        translateAnimation.applyTransformation(0.5f, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals((TO_X_DELTA + FROM_X_DETLTA) / 2,
                values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals((TO_Y_DELTA + FROM_Y_DELTA) / 2,
                values[Matrix.MTRANS_Y], POSITION_DELTA);

        // Test time end
        transformation.clear();
        translateAnimation.applyTransformation(1.0f, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(TO_X_DELTA, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(TO_Y_DELTA, values[Matrix.MTRANS_Y], POSITION_DELTA);
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "initialize",
        args = {int.class, int.class, int.class, int.class}
    )
    public void testInitialize() {
        final View parent = mActivity.findViewById(R.id.anim_window_parent);
        final View animWindow = mActivity.findViewById(R.id.anim_window);
        final Transformation transformation = new Transformation();
        final int selfWidth = animWindow.getWidth();
        final int selfHeight = animWindow.getHeight();
        final int parentWidth = parent.getWidth();
        final int parentHeight = parent.getHeight();
        final int actualWidth = selfWidth;
        final int actualHeight = parentHeight;
        final TranslateAnimation translateAnimation =
                new TranslateAnimation(Animation.RELATIVE_TO_SELF, RELATIVE_FROM_X_DELTA,
                        Animation.RELATIVE_TO_SELF, RELATIVE_TO_X_DELTA,
                        Animation.RELATIVE_TO_PARENT, RELATIVE_FROM_Y_DELTA,
                        Animation.RELATIVE_TO_PARENT, RELATIVE_TO_Y_DELTA);
        assertFalse(translateAnimation.isInitialized());
        translateAnimation.initialize(selfWidth, selfHeight, parentWidth, parentHeight);
        assertTrue(translateAnimation.isInitialized());
        translateAnimation.setDuration(DURATION);
        translateAnimation.setInterpolator(new LinearInterpolator());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, translateAnimation);
        final long startTime = translateAnimation.getStartTime();

        float values[] = new float[9];
        translateAnimation.getTransformation(startTime, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(RELATIVE_FROM_X_DELTA * actualWidth, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(RELATIVE_FROM_Y_DELTA * actualHeight, values[Matrix.MTRANS_Y], POSITION_DELTA);

        transformation.clear();
        translateAnimation.getTransformation(startTime + DURATION / 2, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(((RELATIVE_TO_X_DELTA + RELATIVE_FROM_X_DELTA) / 2) * actualWidth,
                values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(((RELATIVE_TO_Y_DELTA + RELATIVE_FROM_Y_DELTA) / 2) * actualHeight,
                values[Matrix.MTRANS_Y], POSITION_DELTA);

        transformation.clear();
        translateAnimation.getTransformation(startTime + DURATION, transformation);
        transformation.getMatrix().getValues(values);
        assertEquals(RELATIVE_TO_X_DELTA * actualWidth, values[Matrix.MTRANS_X], POSITION_DELTA);
        assertEquals(RELATIVE_TO_Y_DELTA * actualHeight, values[Matrix.MTRANS_Y], POSITION_DELTA);
    }

    private static class MyTranslateAnimation extends TranslateAnimation {

        public MyTranslateAnimation(float fromXDelta, float toXDelta, float fromYDelta,
                float toYDelta) {
            super(fromXDelta, toXDelta, fromYDelta, toYDelta);
        }

        @Override
        protected void applyTransformation(float interpolatedTime, Transformation t) {
            super.applyTransformation(interpolatedTime, t);
        }
    }
}
