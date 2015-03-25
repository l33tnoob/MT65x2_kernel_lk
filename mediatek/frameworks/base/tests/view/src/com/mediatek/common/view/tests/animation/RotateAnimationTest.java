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
import android.view.animation.RotateAnimation;
import android.view.animation.Transformation;

import com.mediatek.common.view.tests.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;

@TestTargetClass(android.view.animation.RotateAnimation.class)
public class RotateAnimationTest
        extends ActivityInstrumentationTestCase2<AnimationTestStubActivity> {

    private Activity mActivity;

    private static final long DURATION = 1000;
    private static final float ROTATE_DELTA = 0.001f;
    private static final float FROM_DEGREE = 0.0f;
    private static final float TO_DEGREE = 90.0f;

    public RotateAnimationTest() {
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
            method = "RotateAnimation",
            args = {android.content.Context.class, android.util.AttributeSet.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "RotateAnimation",
            args = {float.class, float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "RotateAnimation",
            args = {float.class, float.class, float.class, float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "RotateAnimation",
            args = {float.class, float.class, int.class, float.class, int.class, float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "RotateAnimation",
            args = {float.class, float.class}
        )
    })
    public void testConstructors() {

        // Test with null AttributeSet
        new RotateAnimation(mActivity, null);

        final XmlResourceParser parser = mActivity.getResources().getAnimation(
                R.anim.anim_rotate);
        final AttributeSet attr = Xml.asAttributeSet(parser);
        assertNotNull(attr);
        // Test with real AttributeSet
        new RotateAnimation(mActivity, attr);

        // Test {@link RotateAnimation#RotateAnimation(float, float)}
        new RotateAnimation(0.6f, 0.6f);
        // Test negative input values
        new RotateAnimation(-0.6f, -0.6f);

        // Test {@link RotateAnimation#RotateAnimation(float, float, float, float)}
        new RotateAnimation(0.6f, 0.6f, 0.6f, 0.6f);
        // Test negative input values
        new RotateAnimation(-0.6f, -0.6f, -0.6f, -0.6f);

        // Test {@link RotateAnimation#RotateAnimation(float, float, int, float, int, float)}
        new RotateAnimation(0.6f, 0.6f, Animation.ABSOLUTE, 0.6f, Animation.ABSOLUTE, 0.6f);
        // Test negative input values
        new RotateAnimation(-0.6f, -0.6f, Animation.ABSOLUTE, -0.6f, Animation.ABSOLUTE, -0.6f);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "applyTransformation",
            args = {float.class, android.view.animation.Transformation.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "initialize",
            args = {int.class, int.class, int.class, int.class}
        )
    })
    public void testRotateAgainstOrigin(){
        final View animWindowParent = mActivity.findViewById(R.id.anim_window_parent);
        final View animWindow = mActivity.findViewById(R.id.anim_window);
        Transformation transformation = new Transformation();
        // Test when mPivot x and y equal to 0.
        MyRotateAnimation rotateAnimation = new MyRotateAnimation(FROM_DEGREE, TO_DEGREE);
        rotateAnimation.setDuration(DURATION);
        rotateAnimation.setInterpolator(new LinearInterpolator());

        assertFalse(rotateAnimation.isInitialized());
        rotateAnimation.initialize(animWindow.getWidth(), animWindow.getHeight(),
                animWindowParent.getWidth(), animWindowParent.getHeight());
        assertTrue(rotateAnimation.isInitialized());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, rotateAnimation);
        final long startTime = rotateAnimation.getStartTime();

        Matrix expectedMatrix = new Matrix();
        expectedMatrix.setRotate(FROM_DEGREE);
        rotateAnimation.getTransformation(startTime, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        transformation.clear();
        rotateAnimation.applyTransformation(0.0f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());

        expectedMatrix.reset();
        expectedMatrix.setRotate((FROM_DEGREE + TO_DEGREE) / 2);
        rotateAnimation.getTransformation(startTime + DURATION / 2, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        transformation.clear();
        rotateAnimation.applyTransformation(0.5f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());

        expectedMatrix.reset();
        expectedMatrix.setRotate(TO_DEGREE);
        rotateAnimation.getTransformation(startTime + DURATION, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        rotateAnimation.applyTransformation(1.0f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
    }

    private void assertMatrixEquals(Matrix expectedMatrix, Matrix actualMatrix) {
        final float[] expectedMatrixValues = new float[9];
        final float[] actualMatrixValues = new float[9];
        expectedMatrix.getValues(expectedMatrixValues);
        actualMatrix.getValues(actualMatrixValues);
        for (int i = 0; i < 9; i++) {
            assertEquals(expectedMatrixValues[i], actualMatrixValues[i], ROTATE_DELTA);
        }
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "applyTransformation",
            args = {float.class, android.view.animation.Transformation.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "initialize",
            args = {int.class, int.class, int.class, int.class}
        )
    })
    public void testRotateAgainstPoint(){
        final View animWindowParent = mActivity.findViewById(R.id.anim_window_parent);
        final View animWindow = mActivity.findViewById(R.id.anim_window);
        Transformation transformation = new Transformation();
        final float pivotX = 0.2f;
        final float pivotY = 0.2f;
        final float actualPivotX = pivotX * animWindowParent.getWidth();
        final float actualPivotY = pivotY * animWindow.getHeight();
        // Test when mPivot x and y are not origin
        MyRotateAnimation rotateAnimation = new MyRotateAnimation(FROM_DEGREE, TO_DEGREE,
                    Animation.RELATIVE_TO_PARENT, pivotX, Animation.RELATIVE_TO_SELF, pivotY);
        rotateAnimation.setDuration(DURATION);
        rotateAnimation.setInterpolator(new LinearInterpolator());

        assertFalse(rotateAnimation.isInitialized());
        rotateAnimation.initialize(animWindow.getWidth(), animWindow.getHeight(),
                animWindowParent.getWidth(), animWindowParent.getHeight());
        assertTrue(rotateAnimation.isInitialized());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, rotateAnimation);
        final long startTime = rotateAnimation.getStartTime();

        Matrix expectedMatrix = new Matrix();
        expectedMatrix.setRotate(FROM_DEGREE, actualPivotX, actualPivotY);
        rotateAnimation.getTransformation(startTime, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        transformation.clear();
        rotateAnimation.applyTransformation(0.0f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());

        expectedMatrix.reset();
        expectedMatrix.setRotate((FROM_DEGREE + TO_DEGREE) / 2, actualPivotX, actualPivotY);
        rotateAnimation.getTransformation(startTime + DURATION / 2, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        transformation.clear();
        rotateAnimation.applyTransformation(0.5f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());

        expectedMatrix.reset();
        expectedMatrix.setRotate(TO_DEGREE, actualPivotX, actualPivotY);
        rotateAnimation.getTransformation(startTime + DURATION, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
        transformation.clear();
        rotateAnimation.applyTransformation(1.0f, transformation);
        assertMatrixEquals(expectedMatrix, transformation.getMatrix());
    }

    private static class MyRotateAnimation extends RotateAnimation {

        public MyRotateAnimation(float fromDegrees, float toDegrees) {
            super(fromDegrees, toDegrees);
        }

        public MyRotateAnimation(float fromDegrees, float toDegrees, float pivotX, float pivotY) {
            super(fromDegrees, toDegrees, pivotX, pivotY);
        }

        public MyRotateAnimation(float fromDegrees, float toDegrees, int pivotXType,
                float pivotX, int pivotYType, float pivotY) {
            super(fromDegrees, toDegrees, pivotXType, pivotX, pivotYType, pivotY);
        }

        @Override
        protected void applyTransformation(float interpolatedTime, Transformation t) {
            super.applyTransformation(interpolatedTime, t);
        }
    }
}
