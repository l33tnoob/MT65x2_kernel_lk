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

package com.mediatek.world3d;

import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import com.jayway.android.robotium.solo.Solo;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.sameInstance;

public class World3DTest extends ActivityInstrumentationTestCase2<World3D> {
    private Solo solo;

    public World3DTest() {
        super(World3D.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        solo = new Solo(getInstrumentation(), getActivity());
    }

    @Override
    protected void tearDown() throws Exception {
        //Robotium will finish all the activities that have been opened
        solo.finishOpenedActivities();
        super.tearDown();
    }

    public void testAppLaunch() throws Exception {
        assertTrue(getActivity() != null);
    }

    public void testAppLaunchPerformance() throws Exception {
        long t0 = SystemClock.elapsedRealtime();
        getInstrumentation().waitForIdleSync();
        long t1 = SystemClock.elapsedRealtime();
        assertTrue((t1 - t0) <= 1800);
    }

    public void testLeftRotation() {
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        RotateItem item0 = activity.getCurrentFocusItem();

        solo.drag(600, 400, 240, 240, 20);
        solo.sleep(800);
        RotateItem item1 = activity.getCurrentFocusItem();
        assertTrue(item0 != item1);

        solo.drag(600, 400, 240, 240, 20);
        solo.sleep(800);
        RotateItem item2 = activity.getCurrentFocusItem();
        assertTrue(item1 != item2);

        solo.drag(600, 400, 240, 240, 20);
        solo.sleep(800);
        RotateItem item3 = activity.getCurrentFocusItem();
        assertTrue(item3 == item0);
    }

    public void testRightRotation() {
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        RotateItem item0 = activity.getCurrentFocusItem();

        solo.drag(400, 600, 240, 240, 20);
        solo.sleep(800);
        RotateItem item1 = activity.getCurrentFocusItem();
        assertTrue(item0 != item1);

        solo.drag(400, 600, 240, 240, 20);
        solo.sleep(800);
        RotateItem item2 = activity.getCurrentFocusItem();
        assertTrue(item1 != item2);

        solo.drag(400, 600, 240, 240, 20);
        solo.sleep(800);
        RotateItem item3 = activity.getCurrentFocusItem();
        assertTrue(item3 == item0);
    }

    public void testInvalidTouch() {
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        solo.clickOnScreen(10, 10);
        assertFalse(activity.isOnPausedCalled());

        solo.clickOnScreen(800, 10);
        assertFalse(activity.isOnPausedCalled());

        solo.clickOnScreen(800, 460);
        assertFalse(activity.isOnPausedCalled());

        solo.clickOnScreen(10, 460);
        assertFalse(activity.isOnPausedCalled());
    }

    public void testLaunchCamera(){
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        solo.clickOnScreen(400, 240);
        solo.sleep(800);
        assertFalse(activity.isOnPausedCalled()); // Currently, Camera isn't ready yet.
    }

    public void testLaunchVideo(){
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        solo.drag(600, 400, 240, 240, 20);
        solo.sleep(800);

        solo.clickOnScreen(400, 240);
        solo.sleep(800);
        assertFalse(activity.isOnPausedCalled()); // Currently, Video isn't ready yet.
    }

    public void testLaunchGallery(){
        getInstrumentation().waitForIdleSync();
        // Wait for animation finish.
        solo.sleep(1000);

        World3D activity = getActivity();
        solo.drag(400, 600, 240, 240, 20);
        solo.sleep(800);

        solo.clickOnScreen(400, 240);
        solo.sleep(800);
        assertTrue(activity.isOnPausedCalled());
    }
}
