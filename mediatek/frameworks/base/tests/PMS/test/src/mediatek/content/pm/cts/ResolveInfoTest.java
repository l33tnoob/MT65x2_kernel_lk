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


import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Parcel;
import android.test.AndroidTestCase;
import android.util.Printer;

public class ResolveInfoTest extends AndroidTestCase {
    private static final String PACKAGE_NAME = "com.mediatek.cts.pms.stub";
    private static final String MAIN_ACTION_NAME = "android.intent.action.MAIN";
    private static final String ACTIVITY_NAME = "mediatek.content.pm.cts.TestPmActivity";
    private static final String SERVICE_NAME = "mediatek.content.pm.cts.activity.PMTEST_SERVICE";

    public final void testResolveInfo() {
        // Test constructor
        new ResolveInfo();

        PackageManager pm = getContext().getPackageManager();
        Intent intent = new Intent(MAIN_ACTION_NAME);
        intent.setComponent(new ComponentName(PACKAGE_NAME, ACTIVITY_NAME));
        ResolveInfo resolveInfo = pm.resolveActivity(intent, 0);
        // Test loadLabel, loadIcon, getIconResource, toString, describeContents
        String expectedLabel = "Mediatek PMS TestCase";
        assertEquals(expectedLabel, resolveInfo.loadLabel(pm).toString());
        assertNotNull(resolveInfo.loadIcon(pm));
        assertTrue(resolveInfo.getIconResource() != 0);
        assertNotNull(resolveInfo.toString());
        assertEquals(0, resolveInfo.describeContents());
    }

    public final void testDump() {
        PackageManager pm = getContext().getPackageManager();
        Intent intent = new Intent(SERVICE_NAME);
        ResolveInfo resolveInfo = pm.resolveService(intent, PackageManager.GET_RESOLVED_FILTER);

        Parcel p = Parcel.obtain();
        resolveInfo.writeToParcel(p, 0);
        p.setDataPosition(0);
        ResolveInfo infoFromParcel = ResolveInfo.CREATOR.createFromParcel(p);
        // Test writeToParcel
        assertEquals(resolveInfo.getIconResource(), infoFromParcel.getIconResource());
        assertEquals(resolveInfo.priority, infoFromParcel.priority);
        assertEquals(resolveInfo.preferredOrder, infoFromParcel.preferredOrder);
        assertEquals(resolveInfo.match, infoFromParcel.match);
        assertEquals(resolveInfo.specificIndex, infoFromParcel.specificIndex);
        assertEquals(resolveInfo.labelRes, infoFromParcel.labelRes);
        assertEquals(resolveInfo.nonLocalizedLabel, infoFromParcel.nonLocalizedLabel);
        assertEquals(resolveInfo.icon, infoFromParcel.icon);

        // Test dump
        TestPrinter printer = new TestPrinter();
        String prefix = "TestResolveInfo";
        resolveInfo.dump(printer, prefix);
        assertTrue(printer.isPrintlnCalled);
    }

    private class TestPrinter implements Printer {
        public boolean isPrintlnCalled;
        public void println(String x) {
            isPrintlnCalled = true;
        }
    }
}
