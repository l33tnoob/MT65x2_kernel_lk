/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.test.service.binder;

import android.os.Parcel;
import android.os.ParcelUuid;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.binder.FileStructForBinder;


import java.util.Date;
import java.util.UUID;

/**
 * This class is used to test FileStructForBinder
 */
public class FileStructForBinderTest extends AndroidTestCase {
    private static final String TAG = "ChatEventStructForBinderTest";
    private FileStructForBinder mFileStructForBinder;
    private Date mDate;
    String mPath;
    String mName;
    long mSize;
    Object mTag;
    private FileStruct mFileStruct;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp entry");
        super.setUp();
        mDate = new Date();
        mPath = "/sdcard/";
        mName = "test.txt";
        mSize = 100;
        ParcelUuid uuid = new ParcelUuid(UUID.randomUUID());
        mTag = uuid;
        mFileStruct = new FileStruct(mPath, mName, mSize,
                mTag, mDate);
        mFileStructForBinder = new FileStructForBinder(mFileStruct);

    }

    /**
     * Test FileStructForBinder
     */
    public void testCase01_FileStructForBinder() throws Throwable {
        Logger.v(TAG, "testCase01_FileStructForBinder()");
        Parcel source = Parcel.obtain();
        source.writeString(mPath);
        source.writeString(mName);
        source.writeLong(mSize);
        source.writeString(mTag.toString());
        source.writeLong(mDate.getTime());
        assertNotNull(new FileStructForBinder(source));
    }

    /**
     * Test describeContents
     */
    public void testCase02_describeContents() throws Throwable {
        Logger.v(TAG, "testCase02_describeContents()");
        assertEquals(0, mFileStructForBinder.describeContents());
    }

    /**
     * Test writeToParcel
     */
    public void testCase03_writeToParcel() {
        Logger.d(TAG, "testCase03_writeToParcel");
        Parcel dest = Parcel.obtain();
        mFileStructForBinder.writeToParcel(dest, 0);
    }

    /**
     * Test createFromParcel
     */
    public void testCase04_createFromParcel() {
        Logger.d(TAG, "testCase04_createFromParcel");
        Parcel source = Parcel.obtain();
        source.writeString(mPath);
        source.writeString(mName);
        source.writeLong(mSize);
        source.writeString(mTag.toString());
        source.writeLong(mDate.getTime());
        FileStructForBinder fileStructForBinder = mFileStructForBinder.CREATOR
                .createFromParcel(source);
        assertNotNull(fileStructForBinder);
    }

    /**
     * Test newArray
     */
    public void testCase05_newArray() {
        Logger.d(TAG, "testCase05_newArray");
        FileStructForBinder[] fileStructForBinders = mFileStructForBinder.CREATOR
                .newArray(5);
        assertEquals(5, fileStructForBinders.length);
    }

}
