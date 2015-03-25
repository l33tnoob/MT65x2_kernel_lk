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


import android.content.ComponentName;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageItemInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.XmlResourceParser;
import android.os.Parcel;
import android.test.AndroidTestCase;
import android.util.Printer;

public class PackageItemInfoTest extends AndroidTestCase {
    private static final String PACKAGE_NAME = "com.mediatek.cts.pms.stub";
    private static final String ACTIVITY_NAME = "mediatek.content.pm.cts.TestPmActivity";
    private static final String METADATA_NAME = "mediatek.content.pm.cts.xmltest";
    private PackageManager mPackageManager;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mPackageManager = getContext().getPackageManager();
    }

    public void testLoadMethods() throws NameNotFoundException {
        // Test constructors
        ActivityInfo activityInfo = (ActivityInfo) getTestItemInfo();
        new PackageItemInfo();
        PackageItemInfo pkgItemInfo = new PackageItemInfo(activityInfo);
        checkInfoSame(activityInfo, pkgItemInfo);
        // Test loadLabel
        assertEquals(ACTIVITY_NAME, pkgItemInfo.loadLabel(mPackageManager));
        // Test loadIcon
        assertNotNull(pkgItemInfo.loadIcon(mPackageManager));

        // Test loadXmlMetaData
        XmlResourceParser parser = pkgItemInfo.loadXmlMetaData(mPackageManager, METADATA_NAME);
        assertNotNull(parser);
    }

    public void testDump() {
        MockPackageItemInfo pkgItemInfo = new MockPackageItemInfo();
        MockPrinter printer = new MockPrinter();
        // dumpBack is empty method
        pkgItemInfo.dumpBack(printer, "");

        // Test dumpFront
        String prefix = "PackageItemInfoTest";
        pkgItemInfo.dumpFront(printer, prefix);
    }

    public void testWriteToParcel() throws NameNotFoundException {
        ActivityInfo activityInfo = (ActivityInfo) getTestItemInfo();
        PackageItemInfo expectedInfo = new PackageItemInfo(activityInfo);

        Parcel p = Parcel.obtain();
        expectedInfo.writeToParcel(p, 0);
        p.setDataPosition(0);
        // PackageItemInfo(Parcel p) is protected
        MockPackageItemInfo infoFromParcel = new MockPackageItemInfo(p);
        checkInfoSame(expectedInfo, infoFromParcel);
        p.recycle();
    }

    private void checkInfoSame(PackageItemInfo expected, PackageItemInfo actual) {
        assertEquals(expected.name, actual.name);
        assertEquals(expected.packageName, actual.packageName);
        assertEquals(expected.labelRes, actual.labelRes);
        assertEquals(expected.nonLocalizedLabel, actual.nonLocalizedLabel);
        assertEquals(expected.icon, actual.icon);
        assertEquals(expected.metaData.size(), actual.metaData.size());
        assertEquals(R.xml.pm_test, actual.metaData.getInt(METADATA_NAME));
    }

    private PackageItemInfo getTestItemInfo() throws NameNotFoundException {
        ComponentName componentName = new ComponentName(PACKAGE_NAME, ACTIVITY_NAME);
        ActivityInfo activityInfo =
            mPackageManager.getActivityInfo(componentName, PackageManager.GET_META_DATA);
        return activityInfo;
    }

    private class MockPackageItemInfo extends PackageItemInfo {

        public MockPackageItemInfo() {
            super();
        }

        public MockPackageItemInfo(PackageItemInfo orig) {
            super(orig);
        }

        public MockPackageItemInfo(Parcel source) {
            super(source);
        }

        public void dumpFront(Printer pw, String prefix) {
            super.dumpFront(pw, prefix);
        }

        public void dumpBack(Printer pw, String prefix) {
            super.dumpBack(pw, prefix);
        }
    }

    private class MockPrinter implements Printer {
        public void println(String x) {
        }
    }
}
