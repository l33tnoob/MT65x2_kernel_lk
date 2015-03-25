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

package com.mediatek.calendarimporter;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.net.Uri;
import android.test.ProviderTestCase2;

import com.mediatek.calendarimporter.utils.LogUtils;

import java.io.FileNotFoundException;

public class ICalendarProviderTest extends ProviderTestCase2<ICalendarProvider> {
    private static final String TAG = "ICalendarProviderTest";

    private Uri mCalendarUri;
    private ContentResolver mResolver;

    public ICalendarProviderTest() {
        super(ICalendarProvider.class, "com.mediatek.calendarimporter");
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mResolver = getContext().getContentResolver();
    }

    public void test01_Query() {
        mCalendarUri = TestUtils.addOneEventsToDB(mResolver);
        Uri uri = Uri.parse("content://com.mediatek.calendarimporter/" + ContentUris.parseId(mCalendarUri));
        LogUtils.d(TAG, "test01_Query: uri=" + uri.toString());
        Cursor cursor = mResolver.query(uri, null, null, null, null);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        assertTrue(cursor.getCount() == 1);
        TestUtils.removeTheAddedEvent(mResolver, uri);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
    }

    public void test02_OtherOperation() {
        ICalendarProvider provider = getProvider();
        provider.insert(mCalendarUri, null);
        provider.update(mCalendarUri, null, null, null);
        provider.delete(mCalendarUri, null, null);
    }

    public void testO3_OpenAssetFile() {
        mCalendarUri = TestUtils.addOneEventsToDB(mResolver);
        LogUtils.i(TAG, "testO3_OpenAssetFile, mUri = " + mCalendarUri.toString());
        Uri uri = Uri.parse("content://com.mediatek.calendarimporter/" + ContentUris.parseId(mCalendarUri));
        AssetFileDescriptor descriptor = null;
        try {
            ICalendarProvider provider = getProvider();
            // descriptor = provider.openAssetFile(uri, null);
            descriptor = mResolver.openAssetFileDescriptor(uri, null);
        } catch (FileNotFoundException e) {
            LogUtils.e(TAG, "testO3_OpenAssetFile, FileNotFoundException.");
            e.printStackTrace();
        }
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);

        assertNotNull(descriptor);
        assertTrue(descriptor.getLength() > 0);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        TestUtils.removeTheAddedEvent(mResolver, uri);
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
    }

}
