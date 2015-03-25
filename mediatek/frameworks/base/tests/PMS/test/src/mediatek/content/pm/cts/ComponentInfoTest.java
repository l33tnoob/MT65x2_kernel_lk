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

import android.content.pm.ApplicationInfo;
import android.content.pm.ComponentInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Parcel;
import android.test.AndroidTestCase;
import android.util.Printer;
import android.util.StringBuilderPrinter;
import mediatek.widget.cts.WidgetTestUtils;

import com.mediatek.cts.pms.stub.R;


/**
 * Test {@link ComponentInfo}.
 */
public class ComponentInfoTest extends AndroidTestCase {
    private final String PACKAGE_NAME = "com.mediatek.cts.pms.stub";
    private ComponentInfo mComponentInfo;

    public void testConstructor() {
        Parcel p = Parcel.obtain();
        ComponentInfo componentInfo = new ComponentInfo();
        componentInfo.applicationInfo = new ApplicationInfo();
        componentInfo.writeToParcel(p, 0);
        p.setDataPosition(0);

        new MyComponentInfo(p);

        new ComponentInfo();

        new ComponentInfo(componentInfo);

        try {
            new ComponentInfo((ComponentInfo) null);
            fail("should throw NullPointerException.");
        } catch (NullPointerException e) {
            // expected
        }

        try {
            new MyComponentInfo((Parcel) null);
            fail("should throw NullPointerException.");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testLoadIcon() {
        mComponentInfo = new ComponentInfo();
        mComponentInfo.applicationInfo = new ApplicationInfo();

        PackageManager pm = mContext.getPackageManager();
        assertNotNull(pm);

        Drawable defaultIcon = pm.getDefaultActivityIcon();
        Drawable d = null;
        Drawable d2 = null;
        d = mComponentInfo.loadIcon(pm);
        assertNotNull(d);
        assertNotSame(d, defaultIcon);
        WidgetTestUtils.assertEquals(((BitmapDrawable) d).getBitmap(),
                ((BitmapDrawable) defaultIcon).getBitmap());

        d2 = mComponentInfo.loadIcon(pm);
        assertNotNull(d2);
        assertNotSame(d, d2);
        WidgetTestUtils.assertEquals(((BitmapDrawable) d).getBitmap(),
                ((BitmapDrawable) d2).getBitmap());

        try {
            mComponentInfo.loadIcon(null);
            fail("ComponentInfo#loadIcon() throw NullPointerException");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testDumpBack() {
        MyComponentInfo ci = new MyComponentInfo();

        StringBuilder sb = new StringBuilder();
        assertEquals(0, sb.length());
        StringBuilderPrinter p = new StringBuilderPrinter(sb);
        String prefix = "";
        ci.dumpBack(p, prefix);

        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);

        ci.applicationInfo = new ApplicationInfo();
        sb = new StringBuilder();
        assertEquals(0, sb.length());
        p = new StringBuilderPrinter(sb);

        ci.dumpBack(p, prefix);
        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);

        try {
            ci.dumpBack(null, null);
            fail("ComponentInfo#dumpBack() throw NullPointerException here.");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testGetIconResource() {
        mComponentInfo = new ComponentInfo();
        mComponentInfo.applicationInfo = new ApplicationInfo();
        assertEquals(0, mComponentInfo.getIconResource());

        mComponentInfo.icon = R.drawable.red;
        assertEquals(mComponentInfo.icon, mComponentInfo.getIconResource());

        mComponentInfo.icon = 0;
        assertEquals(mComponentInfo.applicationInfo.icon, mComponentInfo.getIconResource());
    }

    public void testIsEnabled() {
        mComponentInfo = new ComponentInfo();
        mComponentInfo.applicationInfo = new ApplicationInfo();
        assertTrue(mComponentInfo.isEnabled());

        mComponentInfo.enabled = false;
        assertFalse(mComponentInfo.isEnabled());

        mComponentInfo.enabled = true;
        mComponentInfo.applicationInfo.enabled = false;
        assertFalse(mComponentInfo.isEnabled());
    }

    public void testDumpFront() {
        MyComponentInfo ci = new MyComponentInfo();

        StringBuilder sb = new StringBuilder();
        assertEquals(0, sb.length());
        StringBuilderPrinter p = new StringBuilderPrinter(sb);

        String prefix = "";
        ci.dumpFront(p, prefix);
        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);

        ci.applicationInfo = new ApplicationInfo();

        sb = new StringBuilder();
        p = new StringBuilderPrinter(sb);
        assertEquals(0, sb.length());

        ci.dumpFront(p, prefix);
        assertNotNull(sb.toString());
        assertTrue(sb.length() > 0);

        try {
            ci.dumpFront(null, null);
            fail("ComponentInfo#dumpFront() throw NullPointerException here.");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testLoadLabel() throws NameNotFoundException {
        mComponentInfo = new ComponentInfo();
        mComponentInfo.applicationInfo = new ApplicationInfo();

        final PackageManager pm = mContext.getPackageManager();

        assertNotNull(mComponentInfo);
        mComponentInfo.packageName = PACKAGE_NAME;
        mComponentInfo.nonLocalizedLabel = "nonLocalizedLabel";
        assertEquals("nonLocalizedLabel", mComponentInfo.loadLabel(pm));

        mComponentInfo.nonLocalizedLabel = null;
        mComponentInfo.labelRes = 0;
        mComponentInfo.name = "name";
        assertEquals("name", mComponentInfo.loadLabel(pm));

        mComponentInfo.applicationInfo =
                mContext.getPackageManager().getApplicationInfo(PACKAGE_NAME, 0);

        mComponentInfo.nonLocalizedLabel = null;
        mComponentInfo.labelRes = R.string.hello_android;
        assertEquals(mContext.getString(mComponentInfo.labelRes), mComponentInfo.loadLabel(pm));

        try {
            mComponentInfo.loadLabel(null);
            fail("ComponentInfo#loadLabel throw NullPointerException");
        } catch (NullPointerException e) {
            // expected
        }
    }

    public void testWriteToParcel() {
        Parcel p = Parcel.obtain();
        mComponentInfo = new ComponentInfo();
        mComponentInfo.applicationInfo = new ApplicationInfo();
        mComponentInfo.writeToParcel(p, 0);
        p.setDataPosition(0);

        MyComponentInfo ci = new MyComponentInfo(p);
        assertEquals(mComponentInfo.processName, ci.processName);
        assertEquals(mComponentInfo.enabled, ci.enabled);
        assertEquals(mComponentInfo.exported, ci.exported);

        StringBuilder sb1 = new StringBuilder();
        StringBuilderPrinter p1 = new StringBuilderPrinter(sb1);
        StringBuilder sb2 = new StringBuilder();
        StringBuilderPrinter p2 = new StringBuilderPrinter(sb2);
        mComponentInfo.applicationInfo.dump(p1, "");
        ci.applicationInfo.dump(p2, "");
        assertEquals(sb1.toString(), sb2.toString());

        try {
            mComponentInfo.writeToParcel(null, 0);
            fail("ComponentInfo#writeToParcel() throw NullPointerException");
        } catch (NullPointerException e) {
            // expected
        }
    }

    private static class MyComponentInfo extends ComponentInfo {
        public MyComponentInfo() {
            super();
        }

        public MyComponentInfo(ComponentInfo orig) {
            super(orig);
        }
        public MyComponentInfo(Parcel source) {
            super(source);
        }

        public void dumpBack(Printer pw, String prefix) {
            super.dumpBack(pw, prefix);
        }

        public void dumpFront(Printer pw, String prefix) {
            super.dumpFront(pw, prefix);
        }
    }
}
