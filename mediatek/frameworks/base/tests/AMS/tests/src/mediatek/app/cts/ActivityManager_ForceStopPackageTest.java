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

package mediatek.app.cts;

import android.app.ActivityManager;
import android.app.ActivityManagerNative;
import android.app.Instrumentation;
import android.app.IActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.test.InstrumentationTestCase;
import android.util.Log;
import android.view.KeyEvent;

import java.util.List;

public class ActivityManager_ForceStopPackageTest extends InstrumentationTestCase {
    private Context mContext;
    private Instrumentation mInstrumentation;
    private static final String TAG = "AMS testForceStopPackage";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
    }

    protected void tearDown() throws Exception {
        mInstrumentation = null;
        super.tearDown();
    }

    //test case for ALPS00350599
    public void testForceStopPackage() throws Exception {
        IActivityManager am = ActivityManagerNative.getDefault();

        //step1. start ForceStopPackageTestActivity
        final Context targetContext = mInstrumentation.getTargetContext();
        final Intent intent1 = new Intent(targetContext, ForceStopPackageTestActivity.class);
        intent1.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mInstrumentation.startActivitySync(intent1);
        mInstrumentation.waitForIdleSync();

        //step2. start SubActivity of ForceStopPackageTestActivity
        final Intent intent2 = new Intent(targetContext, ForceStopPackageTestActivity.ForceStopPackageTestSubActivity.class);
        intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mInstrumentation.startActivitySync(intent2);
        mInstrumentation.waitForIdleSync();

        //step3. screen off
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_POWER);
        Thread.sleep(3000);

        //step4. force stop packages of ForceStopPackageTestActivity
        //am.forceStopPackage("com.mediatek.cts.activity.stub", UserHandle.USER_ALL); //force-stop
        Thread.sleep(3000);

        //step5. screen on
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_POWER);
        Thread.sleep(1000);

        //step6. check whether the bug happened
        List list1 = am.getTasks(1, 0, null);
        List list2 = am.getRecentTasks(1, 0, UserHandle.USER_CURRENT);

        if (list1 == null || list2 == null) {
            Log.d(TAG, "Main task or Recent task is null");
            assertTrue(false);
        } else {
            ActivityManager.RunningTaskInfo running = (ActivityManager.RunningTaskInfo)list1.get(0);
            ActivityManager.RecentTaskInfo recent = (ActivityManager.RecentTaskInfo)list2.get(0);

            if (recent.id != running.id) {
                Log.d(TAG, "Test fail, Running Task id " + running.id + " not equal to Recent Task id " + recent.id);
                assertTrue(false);
            }
        }
    }
}
