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

package mediatek.content.pm.cts;

import mediatek.app.cts.MockActivity;
import android.content.ComponentName;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Parcel;
import android.test.AndroidTestCase;
import android.util.StringBuilderPrinter;

/**
 * Test {@link ActivityInfo}.
 */
public class ActivityInfoTest extends AndroidTestCase {
    ActivityInfo mActivityInfo;

    public void testConstructor() {
        new ActivityInfo();

        ActivityInfo info = new ActivityInfo();
        new ActivityInfo(info);

        try {
            new ActivityInfo(null);
            fail("should throw NullPointerException.");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testWriteToParcel() throws NameNotFoundException {
        ComponentName componentName = new ComponentName(mContext, MockActivity.class);

        mActivityInfo = mContext.getPackageManager().getActivityInfo(
                componentName, PackageManager.GET_META_DATA);

        Parcel p = Parcel.obtain();
        mActivityInfo.writeToParcel(p, 0);
        p.setDataPosition(0);
        ActivityInfo info = ActivityInfo.CREATOR.createFromParcel(p);
        assertEquals(mActivityInfo.theme, info.theme);
        assertEquals(mActivityInfo.launchMode, info.launchMode);
        assertEquals(mActivityInfo.permission, info.permission);
        assertEquals(mActivityInfo.taskAffinity, info.taskAffinity);
        assertEquals(mActivityInfo.targetActivity, info.targetActivity);
        assertEquals(mActivityInfo.flags, info.flags);
        assertEquals(mActivityInfo.screenOrientation, info.screenOrientation);
        assertEquals(mActivityInfo.configChanges, info.configChanges);

        try {
            mActivityInfo.writeToParcel(null, 0);
            fail("should throw NullPointerException");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testGetThemeResource() throws NameNotFoundException {
        ComponentName componentName = new ComponentName(mContext, MockActivity.class);

        mActivityInfo = mContext.getPackageManager().getActivityInfo(
                componentName, PackageManager.GET_META_DATA);

        assertEquals(mActivityInfo.applicationInfo.theme, mActivityInfo.getThemeResource());
        mActivityInfo.theme = 1;
        assertEquals(mActivityInfo.theme, mActivityInfo.getThemeResource());
    }

    public void testToString() throws NameNotFoundException {
        mActivityInfo = new ActivityInfo();
        assertNotNull(mActivityInfo.toString());
    }

    public void testDescribeContents() throws NameNotFoundException {
        mActivityInfo = new ActivityInfo();
        assertEquals(0, mActivityInfo.describeContents());

        ComponentName componentName = new ComponentName(mContext, MockActivity.class);

        mActivityInfo = mContext.getPackageManager().getActivityInfo(
                componentName, PackageManager.GET_META_DATA);

        assertEquals(0, mActivityInfo.describeContents());
    }

    public void testDump() {
        mActivityInfo = new ActivityInfo();

        StringBuilder sb = new StringBuilder();
        assertEquals(0, sb.length());
        StringBuilderPrinter p = new StringBuilderPrinter(sb);

        String prefix = "";
        mActivityInfo.dump(p, prefix);

        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);

        try {
            mActivityInfo.dump(null, "");
            fail("should throw NullPointerException.");
        } catch (NullPointerException e) {
            // expected
        }
    }
}
