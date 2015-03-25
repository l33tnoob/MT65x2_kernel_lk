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

package mediatek.app.cts;


import android.app.Activity;
import android.app.Instrumentation;
import android.app.TabActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.test.InstrumentationTestCase;
import android.view.KeyEvent;
import android.widget.TabHost;

public class TabActivityTest extends InstrumentationTestCase {
    private Instrumentation mInstrumentation;
    private MockTabActivity mActivity;
    private Activity mChildActivity;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = super.getInstrumentation();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mActivity != null) {
            if (!mActivity.isFinishing()) {
                mActivity.finish();
            } else if (mChildActivity != null) {
                if (!mChildActivity.isFinishing()) {
                    mChildActivity.finish();
                }
            }
        }
        super.tearDown();
    }

    /*
    public void testTabActivity() throws Throwable {
        // Test constructor
        new TabActivity();

        final String packageName = "com.mediatek.cts.activity.stub";
        final Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setClassName(packageName, MockTabActivity.class.getName());
        mActivity = (MockTabActivity) mInstrumentation.startActivitySync(intent);
        // Test onPostCreate, onContentChanged. These two methods are invoked in starting
        // activity. Default values of isOnContentChangedCalled, isOnPostCreateCalled are false.
        assertTrue(mActivity.isOnContentChangedCalled);
        assertTrue(mActivity.isOnPostCreateCalled);

        // Can't get default value.
        final int defaultIndex = 1;
        mActivity.setDefaultTab(defaultIndex);
        final String defaultTab = "DefaultTab";
        mActivity.setDefaultTab(defaultTab);
        // Test getTabHost, getTabWidget
        final TabHost tabHost = mActivity.getTabHost();
        assertNotNull(tabHost);
        assertNotNull(tabHost.getTabWidget());

        // Test onSaveInstanceState
        assertFalse(mActivity.isOnSaveInstanceStateCalled);
        final Intent embedded = new Intent(mInstrumentation.getTargetContext(),
                ChildTabActivity.class);
        mActivity.startActivity(embedded);
        mInstrumentation.waitForIdleSync();
        assertTrue(mActivity.isOnSaveInstanceStateCalled);

        // Test onRestoreInstanceState
        sendKeys(KeyEvent.KEYCODE_BACK);
        mInstrumentation.waitForIdleSync();
        assertFalse(MockTabActivity.isOnRestoreInstanceStateCalled);
        OrientationTestUtils.toggleOrientationSync(mActivity, mInstrumentation);
        assertTrue(MockTabActivity.isOnRestoreInstanceStateCalled);
    }
    */
    
    public void testChildTitleCallback() throws Exception {
        final Context context = mInstrumentation.getTargetContext();
        final Intent intent = new Intent(context, MockTabActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        final MockTabActivity father = new MockTabActivity();
        final ComponentName componentName = new ComponentName(context, MockTabActivity.class);
        final ActivityInfo info = context.getPackageManager().getActivityInfo(componentName, 0);
        mChildActivity = mInstrumentation.newActivity(MockTabActivity.class, mInstrumentation
                .getTargetContext(), null, null, intent, info, MockTabActivity.class.getName(),
                father, null, null);

        assertNotNull(mChildActivity);
        final String newTitle = "New Title";
        mChildActivity.setTitle(newTitle);
        assertTrue(father.isOnChildTitleChangedCalled);
    }
}
