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


import android.content.pm.PackageManager;
import android.content.pm.PermissionGroupInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Parcel;
import android.test.AndroidTestCase;

public class PermissionGroupInfoTest extends AndroidTestCase {
    private static final String PERMISSIONGROUP_NAME = "android.permission-group.COST_MONEY";
    private static final String DEFAULT_DISCRIPTION = "Do things that can cost you money.";

    public void testPermissionGroupInfo() throws NameNotFoundException {
        PackageManager pm = getContext().getPackageManager();
        Parcel p = Parcel.obtain();
        // Test constructors
        new PermissionGroupInfo();
        PermissionGroupInfo permissionGroupInfo = pm
                .getPermissionGroupInfo(PERMISSIONGROUP_NAME, 0);
        PermissionGroupInfo infoFromExisted = new PermissionGroupInfo(permissionGroupInfo);
        checkInfoSame(permissionGroupInfo, infoFromExisted);

        // Test toString, describeContents, loadDescription
        assertNotNull(permissionGroupInfo.toString());
        assertEquals(0, permissionGroupInfo.describeContents());
        assertEquals(DEFAULT_DISCRIPTION, permissionGroupInfo.loadDescription(pm));

        // Test writeToParcel
        permissionGroupInfo.writeToParcel(p, 0);
        p.setDataPosition(0);
        PermissionGroupInfo infoFromParcel = PermissionGroupInfo.CREATOR.createFromParcel(p);
        checkInfoSame(permissionGroupInfo, infoFromParcel);
        p.recycle();
    }

    private void checkInfoSame(PermissionGroupInfo expected, PermissionGroupInfo actual) {
        assertEquals(expected.descriptionRes, actual.descriptionRes);
        assertEquals(expected.nonLocalizedDescription, actual.nonLocalizedDescription);
    }
}
