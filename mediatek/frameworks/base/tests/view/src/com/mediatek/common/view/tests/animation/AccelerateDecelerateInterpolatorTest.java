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

import android.app.Activity;
import android.content.res.XmlResourceParser;
import android.test.ActivityInstrumentationTestCase2;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Interpolator;
import android.view.animation.Transformation;

@TestTargetClass(AccelerateDecelerateInterpolator.class)
public class AccelerateDecelerateInterpolatorTest
        extends ActivityInstrumentationTestCase2<AnimationTestStubActivity> {

    private Activity mActivity;
    private static final float ALPHA_DELTA = 0.001f;
    /** It is defined in R.anim.accelarate_decelerate_alpha */
    private static final long ALPHA_DURATION = 2000;

    public AccelerateDecelerateInterpolatorTest() {
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
            method = "AccelerateDecelerateInterpolator",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "AccelerateDecelerateInterpolator",
            args = {android.content.Context.class, android.util.AttributeSet.class}
        )
    })
    public void testConstructor() {
        new AccelerateDecelerateInterpolator();

        XmlResourceParser parser = mActivity.getResources().getAnimation(
                R.anim.accelerate_decelerate_alpha);
        AttributeSet attrs = Xml.asAttributeSet(parser);
        new AccelerateDecelerateInterpolator(mActivity, attrs);
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        notes = "Test case will accelerate then decelerate AlphaAnimation. It will change"
              + " alpha from 0.0 to 1.0, the rate of changing alpha starts out slowly, then"
              + " accelerates to the middle, and then decelerates to the end",
        method = "getInterpolation",
        args = {float.class}
    )
    public void testAccelerateDecelerateInterpolator() {
        final View animWindow = mActivity.findViewById(R.id.anim_window);

        // XML file of R.anim.accelerate_decelerate_alpha
        // <alpha xmlns:android="http://schemas.android.com/apk/res/android"
        //      android:interpolator="@android:anim/accelerate_decelerate_interpolator"
        //      android:fromAlpha="0.0"
        //      android:toAlpha="1.0"
        //      android:duration="2000" />
        final Animation anim = AnimationUtils.loadAnimation(mActivity,
                R.anim.accelerate_decelerate_alpha);
        assertEquals(ALPHA_DURATION, anim.getDuration());
        assertTrue(anim instanceof AlphaAnimation);
        assertFalse(anim.hasStarted());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, anim);

        Transformation transformation = new Transformation();
        long startTime = anim.getStartTime();
        anim.getTransformation(startTime, transformation);
        float alpha1 = transformation.getAlpha();
        assertEquals(0.0f, alpha1, ALPHA_DELTA);

        anim.getTransformation(startTime + 500, transformation);
        float alpha2 = transformation.getAlpha();

        anim.getTransformation(startTime + 1000, transformation);
        float alpha3 = transformation.getAlpha();

        anim.getTransformation(startTime + 1500, transformation);
        float alpha4 = transformation.getAlpha();

        anim.getTransformation(startTime + ALPHA_DURATION, transformation);
        float alpha5 = transformation.getAlpha();
        assertEquals(1.0f, alpha5, ALPHA_DELTA);

        // check accelerating and decelerating delta alpha
        // starts and ends slowly, accelerates through middle
        float delta1 = alpha2 - alpha1;
        float delta2 = alpha3 - alpha2;
        float delta3 = alpha4 - alpha3;
        float delta4 = alpha5 - alpha4;
        assertTrue(delta1 < delta2);
        assertTrue(delta3 > delta4);
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        notes = "Test getInterpolation(float), call it directly.",
        method = "getInterpolation",
        args = {float.class}
    )
    public void testGetInterpolation() {
        Interpolator interpolator = new AccelerateDecelerateInterpolator();

        float alpha1 = interpolator.getInterpolation(0f);
        float alpha2 = interpolator.getInterpolation(0.25f);
        float alpha3 = interpolator.getInterpolation(0.5f);
        float alpha4 = interpolator.getInterpolation(0.75f);
        float alpha5 = interpolator.getInterpolation(1f);
        // check accelerating and decelerating delta alpha
        // starts and ends slowly, accelerates through middle
        float delta1 = alpha2 - alpha1;
        float delta2 = alpha3 - alpha2;
        float delta3 = alpha4 - alpha3;
        float delta4 = alpha5 - alpha4;
        assertTrue(delta1 < delta2);
        assertTrue(delta3 > delta4);
    }
}
