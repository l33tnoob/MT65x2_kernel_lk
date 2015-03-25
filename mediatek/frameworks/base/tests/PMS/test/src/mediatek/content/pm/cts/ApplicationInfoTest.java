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

import com.mediatek.cts.pms.stub.R;


import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Parcel;
import android.test.AndroidTestCase;
import android.util.StringBuilderPrinter;

/**
 * Test {@link ApplicationInfo}.
 */
public class ApplicationInfoTest extends AndroidTestCase {
    private ApplicationInfo mApplicationInfo;
    private String mPackageName;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mPackageName = getContext().getPackageName();
    }

    public void testConstructor() {
        ApplicationInfo info = new ApplicationInfo();
        // simple test to ensure packageName is copied by copy constructor
        // TODO: consider expanding to check all member variables
        info.packageName = mPackageName;
        ApplicationInfo copy = new ApplicationInfo(info);
        assertEquals(info.packageName, copy.packageName);
    }

    public void testWriteToParcel() throws NameNotFoundException {
        mApplicationInfo = mContext.getPackageManager().getApplicationInfo(mPackageName, 0);

        Parcel p = Parcel.obtain();
        mApplicationInfo.writeToParcel(p, 0);

        p.setDataPosition(0);
        ApplicationInfo info = ApplicationInfo.CREATOR.createFromParcel(p);
        assertEquals(mApplicationInfo.taskAffinity, info.taskAffinity);
        assertEquals(mApplicationInfo.permission, info.permission);
        assertEquals(mApplicationInfo.processName, info.processName);
        assertEquals(mApplicationInfo.className, info.className);
        assertEquals(mApplicationInfo.theme, info.theme);
        assertEquals(mApplicationInfo.flags, info.flags);
        assertEquals(mApplicationInfo.sourceDir, info.sourceDir);
        assertEquals(mApplicationInfo.publicSourceDir, info.publicSourceDir);
        assertEquals(mApplicationInfo.sharedLibraryFiles, info.sharedLibraryFiles);
        assertEquals(mApplicationInfo.dataDir, info.dataDir);
        assertEquals(mApplicationInfo.uid, info.uid);
        assertEquals(mApplicationInfo.enabled, info.enabled);
        assertEquals(mApplicationInfo.manageSpaceActivityName, info.manageSpaceActivityName);
        assertEquals(mApplicationInfo.descriptionRes, info.descriptionRes);
    }

    public void testToString() {
        mApplicationInfo = new ApplicationInfo();
        assertNotNull(mApplicationInfo.toString());
    }

    public void testDescribeContents() throws NameNotFoundException {
       mApplicationInfo = mContext.getPackageManager().getApplicationInfo(mPackageName, 0);

        assertEquals(0, mApplicationInfo.describeContents());
    }

    public void testDump() {
        mApplicationInfo = new ApplicationInfo();

        StringBuilder sb = new StringBuilder();
        assertEquals(0, sb.length());
        StringBuilderPrinter p = new StringBuilderPrinter(sb);

        String prefix = "";
        mApplicationInfo.dump(p, prefix);
        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);
    }

    public void testLoadDescription() throws NameNotFoundException {
        mApplicationInfo = mContext.getPackageManager().getApplicationInfo(mPackageName, 0);

        assertNull(mApplicationInfo.loadDescription(mContext.getPackageManager()));

        mApplicationInfo.descriptionRes = R.string.hello_world;
        assertEquals(mContext.getResources().getString(R.string.hello_world),
                mApplicationInfo.loadDescription(mContext.getPackageManager()));
    }
}
