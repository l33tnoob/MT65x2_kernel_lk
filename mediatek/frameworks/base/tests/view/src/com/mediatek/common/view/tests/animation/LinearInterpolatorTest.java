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
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.Interpolator;
import android.view.animation.LinearInterpolator;
import android.view.animation.Transformation;

import com.mediatek.common.view.tests.R;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;

/**
 * Test {@link LinearInterpolator}.
 */
@TestTargetClass(LinearInterpolator.class)
public class LinearInterpolatorTest extends ActivityInstrumentationTestCase2<AnimationTestStubActivity> {

    private Activity mActivity;
    private static final float ALPHA_DELTA = 0.001f;

    /** It is defined in R.anim.alpha */
    private static final long LINEAR_ALPHA_DURATION = 500;
    private static final long LINEAR_ALPHA_TIME_STEP = LINEAR_ALPHA_DURATION / 5;

    public LinearInterpolatorTest() {
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
            method = "LinearInterpolator",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "LinearInterpolator",
            args = {android.content.Context.class, android.util.AttributeSet.class}
        )
    })
    public void testConstructor() {
        new LinearInterpolator();
        new LinearInterpolator(mActivity, null);
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "getInterpolation",
        args = {float.class}
    )
    public void testGetInterpolation() {
        LinearInterpolator interpolator = new LinearInterpolator();
        final float delta1 = interpolator.getInterpolation(0.1f)
                - interpolator.getInterpolation(0.0f);
        final float delta2 = interpolator.getInterpolation(0.2f)
                - interpolator.getInterpolation(0.1f);
        final float delta3 = interpolator.getInterpolation(0.3f)
                - interpolator.getInterpolation(0.2f);
        assertEquals(delta1, delta2, ALPHA_DELTA);
        assertEquals(delta2, delta3, ALPHA_DELTA);
    }

    public void testLinearInterpolator() {
        final View animWindow = mActivity.findViewById(R.id.anim_window);
        final Animation anim = AnimationUtils.loadAnimation(mActivity, R.anim.alpha);
        assertEquals(LINEAR_ALPHA_DURATION, anim.getDuration());
        assertTrue(anim instanceof AlphaAnimation);

        Interpolator interpolator = new LinearInterpolator();
        anim.setInterpolator(interpolator);
        assertFalse(anim.hasStarted());

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), animWindow, anim);

        Transformation transformation = new Transformation();
        final long startTime = anim.getStartTime();
        anim.getTransformation(startTime, transformation);
        final float alpha1 = transformation.getAlpha();
        assertEquals(0.0f, alpha1, ALPHA_DELTA);

        anim.getTransformation(startTime + LINEAR_ALPHA_TIME_STEP, transformation);
        final float alpha2 = transformation.getAlpha();

        anim.getTransformation(startTime + LINEAR_ALPHA_TIME_STEP * 2, transformation);
        final float alpha3 = transformation.getAlpha();

        anim.getTransformation(startTime + LINEAR_ALPHA_TIME_STEP * 3, transformation);
        final float alpha4 = transformation.getAlpha();

        anim.getTransformation(startTime + LINEAR_ALPHA_TIME_STEP * 4, transformation);
        final float alpha5 = transformation.getAlpha();

        anim.getTransformation(startTime + LINEAR_ALPHA_DURATION, transformation);
        final float alpha6 = transformation.getAlpha();
        assertEquals(1.0f, alpha6, ALPHA_DELTA);

        final float delta1 = alpha2 - alpha1;
        final float delta2 = alpha3 - alpha2;
        final float delta3 = alpha4 - alpha3;
        final float delta4 = alpha5 - alpha4;
        final float delta5 = alpha6 - alpha5;
        assertEquals(delta1, delta2, ALPHA_DELTA);
        assertEquals(delta2, delta3, ALPHA_DELTA);
        assertEquals(delta3, delta4, ALPHA_DELTA);
        assertEquals(delta4, delta5, ALPHA_DELTA);
    }
}
