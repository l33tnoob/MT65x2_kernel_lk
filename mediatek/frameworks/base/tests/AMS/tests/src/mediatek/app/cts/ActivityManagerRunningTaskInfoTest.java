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

import android.app.ActivityManager;
import android.graphics.Bitmap;
import android.os.Parcel;
import android.test.AndroidTestCase;

public class ActivityManagerRunningTaskInfoTest extends AndroidTestCase {
    protected ActivityManager.RunningTaskInfo mRunningTaskInfo;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mRunningTaskInfo = new ActivityManager.RunningTaskInfo();

        mRunningTaskInfo.id = 1;
        mRunningTaskInfo.baseActivity = null;
        mRunningTaskInfo.topActivity = null;
        mRunningTaskInfo.thumbnail = null;
        mRunningTaskInfo.numActivities = 1;
        mRunningTaskInfo.numRunning = 2;
        mRunningTaskInfo.description = null;
    }

    public void testConstructor() {
        new ActivityManager.RunningTaskInfo();
    }

    public void testDescribeContents() {
        assertEquals(0, mRunningTaskInfo.describeContents());
    }

    public void testWriteToParcel() throws Exception {

        Parcel parcel = Parcel.obtain();
        mRunningTaskInfo.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        ActivityManager.RunningTaskInfo values = ActivityManager.RunningTaskInfo.CREATOR
                .createFromParcel(parcel);
        assertEquals(1, values.id);
        assertNull(values.baseActivity);
        assertNull(values.topActivity);
        assertNull(values.thumbnail);
        assertEquals(1, values.numActivities);
        assertEquals(2, values.numRunning);
        // test thumbnail is not null
        mRunningTaskInfo.thumbnail = Bitmap.createBitmap(480, 320,
                Bitmap.Config.RGB_565);
        parcel = Parcel.obtain();
        mRunningTaskInfo.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        values = ActivityManager.RunningTaskInfo.CREATOR
                .createFromParcel(parcel);
        assertNotNull(values.thumbnail);
        assertEquals(320, values.thumbnail.getHeight());
        assertEquals(480, values.thumbnail.getWidth());
        assertEquals(Bitmap.Config.RGB_565, values.thumbnail.getConfig());

    }

    public void testReadFromParcel() throws Exception {

        Parcel parcel = Parcel.obtain();
        mRunningTaskInfo.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        ActivityManager.RunningTaskInfo values = new ActivityManager.RunningTaskInfo();
        values.readFromParcel(parcel);
        assertEquals(1, values.id);
        assertNull(values.baseActivity);
        assertNull(values.topActivity);
        assertNull(values.thumbnail);
        assertEquals(1, values.numActivities);
        assertEquals(2, values.numRunning);
        // test thumbnail is not null
        mRunningTaskInfo.thumbnail = Bitmap.createBitmap(480, 320,
                Bitmap.Config.RGB_565);
        parcel = Parcel.obtain();
        mRunningTaskInfo.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        values.readFromParcel(parcel);
        assertNotNull(values.thumbnail);
        assertEquals(320, values.thumbnail.getHeight());
        assertEquals(480, values.thumbnail.getWidth());
        assertEquals(Bitmap.Config.RGB_565, values.thumbnail.getConfig());
    }

}
