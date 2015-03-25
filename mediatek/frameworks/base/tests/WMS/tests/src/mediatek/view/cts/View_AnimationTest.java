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

package com.mediatek.cts.window;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import com.mediatek.cts.window.AnimationTestUtils;

import com.mediatek.cts.window.stub.R;


/**
 * Test {@link View}.
 */
public class View_AnimationTest extends ActivityInstrumentationTestCase2<ViewTestStubActivity> {

    private static final int TIME_OUT = 5000;
    private static final int DURATION = 2000;

    private Activity mActivity;

    private TranslateAnimation mAnimation;

    public View_AnimationTest() {
        super("com.mediatek.cts.window.stub", ViewTestStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
        mAnimation =  new TranslateAnimation(0.0f, 10.0f, 0.0f, 10.0f);
        mAnimation.setDuration(DURATION);
    }

    public void testAnimation() throws Throwable {
        final View view = mActivity.findViewById(R.id.mock_view);
        // set null animation
        view.setAnimation(null);
        assertNull(view.getAnimation());

        view.setAnimation(mAnimation);
        runTestOnUiThread(new Runnable() {
            public void run() {
                view.invalidate();
            }
        });

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), view, mAnimation, TIME_OUT);
    }

    public void testStartAnimation() throws Throwable {
        final View view = mActivity.findViewById(R.id.mock_view);
        // start null animation
        try {
            view.startAnimation(null);
            fail("did not throw NullPointerException when start null animation");
        } catch (NullPointerException e) {
            // expected
        }

        runTestOnUiThread(new Runnable() {
            public void run() {
                view.startAnimation(mAnimation);
            }
        });

        AnimationTestUtils.assertRunAnimation(getInstrumentation(), view, mAnimation, TIME_OUT);
    }

    public void testClearBeforeAnimation() throws Throwable {
        final View view = mActivity.findViewById(R.id.mock_view);
        assertFalse(mAnimation.hasStarted());

        view.setAnimation(mAnimation);

        assertSame(mAnimation, view.getAnimation());

        runTestOnUiThread(new Runnable() {
            public void run() {
                view.clearAnimation();
                view.invalidate();
            }
        });

        Thread.sleep(TIME_OUT);
        assertFalse(mAnimation.hasStarted());
        assertNull(view.getAnimation());
    }

    public void testClearDuringAnimation() throws Throwable {
        final View view = mActivity.findViewById(R.id.mock_view);
        runTestOnUiThread(new Runnable() {
            public void run() {
                view.startAnimation(mAnimation);
                assertNotNull(view.getAnimation());
            }
        });

        new PollingCheck(TIME_OUT) {
            @Override
            protected boolean check() {
                return mAnimation.hasStarted();
            }
        }.run();
        runTestOnUiThread(new Runnable() {
            public void run() {
                view.clearAnimation();
            }
        });
        Thread.sleep(TIME_OUT);
        assertTrue(mAnimation.hasStarted());
        assertTrue(mAnimation.hasEnded());
        assertNull(view.getAnimation());
    }
}
