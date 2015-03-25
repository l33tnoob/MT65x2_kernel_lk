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

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManagerNative;
import android.app.AppGlobals;
import android.app.Instrumentation;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageStats;
import android.content.pm.UserInfo;

import android.graphics.Bitmap;

import android.net.Uri;

import android.os.Parcel;
import android.os.RemoteException;
import android.os.UserHandle;
import android.os.UserManager;

import android.test.AndroidTestCase;
import android.test.InstrumentationTestCase;

public class PackageManagerCRTest extends InstrumentationTestCase {
    private static final String TAG = "PackageManagerCRTest";
    private static final String PACKAGE_NAME = "com.mediatek.cts.pms";
    
    private Context mContext;
    private PackageManager mPackageManager;
    private ActivityManager mActivityManager;
    private UserManager mUserManager;
    private Intent mIntent;
    private Instrumentation mInstrumentation;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        //mPackageManager = (PackageManager) mContext.getSystemService(Context.PACKAGE_SERVICE);
        mActivityManager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        mUserManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testALPS00456704() {
        try {
            AppGlobals.getPackageManager().enforceDexOpt(PACKAGE_NAME);
        } catch (RemoteException e) {
            fail();
        } catch (Exception e) {
            //
        }
    }

    public void testALPS00421371() {
        try {
            boolean ret = AppGlobals.getPackageManager().enforceDexOpt("com.android.launcher3");
            /*
            if (!ret) {
                System.out.println("\"com.android.launcher\" is DEX_OPT_SKIPPED!");
            }
            */
            assertFalse(ret);
        } catch (RemoteException re) {
            fail("Unexpected.");
        } catch (Exception e) {
            //
        }
    }

    public void testALPS00438553() {
        Context currentUserContext = null;
        UserInfo userInfo = null;
        try {
            userInfo = ActivityManagerNative.getDefault().getCurrentUser();
            currentUserContext = mContext.createPackageContextAsUser("android", 0,
                    new UserHandle(userInfo.id));
        } catch (NameNotFoundException e) {
            fail("Couldn't create user context.");
            throw new RuntimeException(e);
        } catch (RemoteException e) {
            fail("Couldn't get user info.");
        }

        final int userId = userInfo.id;
        final String userName = userInfo.name;
        System.out.println("userId=" + userId);
        System.out.println("userName=" + userName);

        Bitmap icon = Bitmap.createBitmap(1, 1, Bitmap.Config.RGB_565);
        mUserManager.setUserIcon(userId, icon);
        assertNotNull(mUserManager.getUserIcon(userId));

        mUserManager.setUserIcon(userId, null);
        assertNull(mUserManager.getUserIcon(userId));
    }

    public void testALPS00418300() {
        try {
            ApplicationInfo info = AppGlobals.getPackageManager().getApplicationInfo("com.sohu.inputmethod.sogou", 0, 0);
            if (info != null) {
                //System.out.println(ApplicationInfo.FLAG_OPERATOR);
                //System.out.println(info.mtkFlags);
                assertEquals((int) (1<<0), ApplicationInfo.FLAG_OPERATOR);
                assertTrue((info.mtkFlags & ApplicationInfo.FLAG_OPERATOR) != 0);
            }
        } catch (NullPointerException npe) {
            //
        } catch (RemoteException re) {
            fail("Unexpected.");
        } catch (Exception e) {
            //
        }
    }
}
