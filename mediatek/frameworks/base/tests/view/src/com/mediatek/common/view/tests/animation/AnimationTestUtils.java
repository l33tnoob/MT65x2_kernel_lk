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
 * Copyright (C) 2009 The Android Open Source Project
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

import android.app.Instrumentation;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LayoutAnimationController;

/**
 * The utility methods for animation test.
 */
public final class AnimationTestUtils {
    /** timeout delta when wait in case the system is sluggish */
    private static final long TIMEOUT_DELTA = 1000;

    /**
     * no public constructor since this is a utility class
     */
    private AnimationTestUtils() {

    }

    /**
     * Assert run an animation successfully. Timeout is duration of animation.
     *
     * @param instrumentation to run animation.
     * @param view view window to run animation.
     * @param animation will be run.
     */
    public static void assertRunAnimation(final Instrumentation instrumentation,
            final View view, final Animation animation) {
        assertRunAnimation(instrumentation, view, animation, animation.getDuration());
    }

    /**
     * Assert run an animation successfully.
     *
     * @param instrumentation to run animation.
     * @param view window to run animation.
     * @param animation will be run.
     * @param duration in milliseconds.
     */
    public static void assertRunAnimation(final Instrumentation instrumentation,
            final View view, final Animation animation, final long duration) {

        instrumentation.runOnMainSync(new Runnable() {
            public void run() {
                view.startAnimation(animation);
            }
        });

        // check whether it has started
        new PollingCheck() {
            @Override
            protected boolean check() {
                return animation.hasStarted();
            }
        }.run();

        // check whether it has ended after duration
        new PollingCheck(duration + TIMEOUT_DELTA) {
            @Override
            protected boolean check() {
                return animation.hasEnded();
            }
        }.run();

        instrumentation.waitForIdleSync();
    }

    /**
     * Assert run an AbsListView with LayoutAnimationController successfully.
     * @param instrumentation
     * @param view
     * @param controller
     * @param duration
     * @throws InterruptedException
     */
    public static void assertRunController(final Instrumentation instrumentation,
            final ViewGroup view, final LayoutAnimationController controller,
            final long duration) throws InterruptedException {

        instrumentation.runOnMainSync(new Runnable() {
           public void run() {
                view.setLayoutAnimation(controller);
                view.requestLayout();
           }
        });

        // LayoutAnimationController.isDone() always returns true, it's no use for stopping
        // the running, so just using sleeping fixed time instead. we reported issue 1799434 for it.
        Thread.sleep(duration + TIMEOUT_DELTA);
    }
}
