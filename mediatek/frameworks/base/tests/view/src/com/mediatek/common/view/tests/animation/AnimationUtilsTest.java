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

import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.GridLayoutAnimationController;
import android.view.animation.Interpolator;
import android.view.animation.LayoutAnimationController;

@TestTargetClass(AnimationUtils.class)
public class AnimationUtilsTest extends
        ActivityInstrumentationTestCase2<AnimationTestStubActivity> {

    private AnimationTestStubActivity mActivity;

    public AnimationUtilsTest() {
        super("com.android.cts.stub", AnimationTestStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = (AnimationTestStubActivity) getActivity();
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "loadAnimation",
            args = {Context.class, int.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "loadInterpolator",
            args = {Context.class, int.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "loadLayoutAnimation",
            args = {Context.class, int.class}
        )
    })
    public void testLoad() {
        // XML file of com.android.cts.stub.R.anim.anim_alpha
        // <alpha xmlns:android="http://schemas.android.com/apk/res/android"
        //      android:interpolator="@android:anim/accelerate_interpolator"
        //      android:fromAlpha="0.0"
        //      android:toAlpha="1.0"
        //      android:duration="500" />
        int duration = 500;
        Animation animation = AnimationUtils.loadAnimation(mActivity, R.anim.anim_alpha);
        assertEquals(duration, animation.getDuration());
        assertTrue(animation instanceof AlphaAnimation);

        // Load Interpolator from android.R.anim.accelerate_interpolator
        Interpolator interpolator = AnimationUtils.loadInterpolator(mActivity,
                android.R.anim.accelerate_interpolator);
        assertTrue(interpolator instanceof AccelerateInterpolator);

        // Load LayoutAnimationController from com.android.cts.stub.R.anim.anim_gridlayout
        // <gridLayoutAnimation xmlns:android="http://schemas.android.com/apk/res/android"
        //      android:delay="10%"
        //      android:rowDelay="50%"
        //      android:directionPriority="column"
        //      android:animation="@anim/anim_alpha" />
        LayoutAnimationController controller = AnimationUtils.loadLayoutAnimation(mActivity,
                R.anim.anim_gridlayout);
        assertTrue(controller instanceof GridLayoutAnimationController);
        assertEquals(duration, controller.getAnimation().getDuration());
        assertEquals(0.1f, controller.getDelay(), 0.001f);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.NOT_FEASIBLE,
            method = "makeInAnimation",
            args = {Context.class, boolean.class}
        ),
        @TestTargetNew(
            level = TestLevel.NOT_FEASIBLE,
            method = "makeOutAnimation",
            args = {Context.class, boolean.class}
        ),
        @TestTargetNew(
            level = TestLevel.NOT_FEASIBLE,
            method = "makeInChildBottomAnimation",
            args = {Context.class}
        )
    })
    public void testMakeAnimation() {
        Animation inAnimation = AnimationUtils.makeInAnimation(mActivity, true);
        assertNotNull(inAnimation);
        Animation outAnimation = AnimationUtils.makeOutAnimation(mActivity, true);
        assertNotNull(outAnimation);
        Animation bottomAnimation = AnimationUtils.makeInChildBottomAnimation(mActivity);
        assertNotNull(bottomAnimation);
        // TODO: How to assert these Animations.
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "currentAnimationTimeMillis",
        args = {}
    )
    public void testCurrentAnimationTimeMillis() {
        long time1 = AnimationUtils.currentAnimationTimeMillis();
        assertTrue(time1 > 0);

        long time2 = 0L;
        for (int i = 0; i < 1000 && time1 >= time2; i++) {
            time2 = AnimationUtils.currentAnimationTimeMillis();
            assertTrue(time2 > 0);
        }
        assertTrue(time2 > time1);
    }
}
