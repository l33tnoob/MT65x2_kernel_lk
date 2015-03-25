/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

package com.mediatek.videofavorites.tests;

import com.mediatek.videofavorites.VideoFavoritesProvider;
import com.mediatek.videofavorites.VideoFavoritesProviderValues;
import com.mediatek.videofavorites.WidgetAdapter;


import com.mediatek.xlog.Xlog;

import android.content.Intent;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.test.ServiceTestCase;
import android.test.mock.MockContext;
import android.test.mock.MockContentResolver;
import android.test.RenamingDelegatingContext;
import android.test.IsolatedContext;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.RemoteViewsService.RemoteViewsFactory;

import com.android.internal.widget.IRemoteViewsFactory;


public class VideoFavoritesRemoteServiceTest extends ServiceTestCase<WidgetAdapter> {

    public static final String TAG = "RemoteServiceTest";
    private RenamingDelegatingContext mMockContext;

    public VideoFavoritesRemoteServiceTest() {
        super(WidgetAdapter.class);
    }

    private IsolatedContext getMockContext() {
        return mProviderContext;
    }

    // getSystemContext() : real context, avaliable after setUp()
    // getContext(): the context being set by set Context

    private IsolatedContext mProviderContext;
    private VideoFavoritesProvider mProvider;
    private MockContentResolver mResolver;



    @Override
    public void setUp() {
        Xlog.v(TAG, "setUp()");
        try {
            super.setUp();
        } catch (Exception e) {
            Xlog.e(TAG, "super.setUp() failed");
        }

        // mock the context and resolver.
        mResolver = new MockContentResolver();

        RenamingDelegatingContext rdc = new RenamingDelegatingContext(getSystemContext(), "test.");
        mProviderContext = new IsolatedContext(mResolver, rdc);

        mProvider = new VideoFavoritesProvider();
        mProvider.attachInfo(mProviderContext, null);

        mResolver.addProvider(VideoFavoritesProviderValues.AUTHORITY, mProvider);

        // mock context
        setContext(mProviderContext);
    }

    private MockContentResolver getMockContentResolver() {
        return mResolver;
    }

    //
    private Intent getIntent() {
        final Intent i = new Intent(getMockContext(), WidgetAdapter.class);
        return i;
    }


    private IRemoteViewsFactory getRemoteViewsFactory() {
        IBinder ib = bindService(getIntent());

        assertNotNull(ib);
        IRemoteViewsFactory rvf = IRemoteViewsFactory.Stub.asInterface(ib);
        return rvf;
    }

    private static final int DB_MODE_EMPTY = 0;
    private static final int DB_MODE_NORMAL = 1;
    private static final int DB_MODE_STRESS = 2;

    private static final int DB_ENTRY_NORMAL_COUNT = 4;
    private static final int DB_ENTRY_STRESS_COUNT = 64;

    private static final String KEY_VIDEO_URI = VideoFavoritesProviderValues.Columns.VIDEO_URI;
    private static final String KEY_CONTACT_URI = VideoFavoritesProviderValues.Columns.CONTACT_URI;
    private static final String KEY_NAME = VideoFavoritesProviderValues.Columns.NAME;

    private static final String NAME_STRING = "Foo";
    private void prepareDatabase(int mode) {
        ContentResolver resolver = getMockContentResolver();
        // clear database
        int numDelete = resolver.delete(VideoFavoritesProviderValues.Columns.CONTENT_URI,
                                        null, null);
        if (mode == DB_MODE_EMPTY) {
            // empty database, just return;
            return;
        }

        ContentValues favoriteValues = new ContentValues(3);
        favoriteValues.put(KEY_VIDEO_URI,
                           "android.resource://com.mediatek.videofavorites.tests/" + R.raw.video);
        favoriteValues.put(KEY_CONTACT_URI, "content://test");

        final int count = (mode == DB_MODE_STRESS) ? DB_ENTRY_STRESS_COUNT : DB_ENTRY_NORMAL_COUNT;

        for (int i = 0; i < count; i++) {
            favoriteValues.remove(KEY_NAME);
            favoriteValues.put(KEY_NAME, NAME_STRING + i);
            resolver.insert(VideoFavoritesProviderValues.Columns.CONTENT_URI, favoriteValues);
        }
    }


    // we test the same db /service with serveral test items  in one test case
    // 1. test empty db
    // 2. insert something into db and test again (4 rows of data)
    // 3. insert more things and test again

    public void testDataBase() {
        // initialize database first
        prepareDatabase(DB_MODE_EMPTY);
        IRemoteViewsFactory factory = getRemoteViewsFactory();
        RemoteViews rv;

        assertNotNull(factory);
        int totalCount = 0;
        try {
            totalCount = factory.getCount();
            Log.v(TAG, "testEmptyDataBase, getCount():" + factory.getCount());
        } catch (RemoteException re) {
            Log.e(TAG, "exception in testEmpty");
        }
        assertEquals(4, totalCount);    // changed to 4 slots after design change. (1 add, 3 empty)

        try {
            // get the view and check.
            rv = factory.getViewAt(0);
            Log.e(TAG, "rv: " + rv);
            assertNotNull(rv);
        } catch (RemoteException re) {
            Log.e(TAG, "exception in testNormal");
        }


        Log.e(TAG, "Test Normal database");
        prepareDatabase(DB_MODE_NORMAL);
        try {
            factory.onDataSetChanged();
            totalCount = factory.getCount();
        } catch (RemoteException re) {
            Log.e(TAG, "exception in testNormal");
        }
        assertEquals(WidgetAdapter.LARGE_MAX_NUM_VIDEOS, totalCount);
        try {
            for (int i = 0; i < totalCount; i++) {
                rv = factory.getViewAt(i);
                assertNotNull(rv);
                long j = factory.getItemId(i);
                assertTrue(i == j);
            }
        } catch (RemoteException re) {
            Log.e(TAG, "exception in getviews");
        }
        Log.e(TAG, "Test Large database");
        prepareDatabase(DB_MODE_STRESS);
        try {
            factory.onDataSetChanged();
            totalCount = factory.getCount();
        } catch (RemoteException re) {
            Log.e(TAG, "exception in testNormal");
        }
        assertEquals(WidgetAdapter.LARGE_MAX_NUM_VIDEOS, totalCount);
        try {
            for (int i = 0; i < totalCount; i++) {
                rv = factory.getViewAt(i);
                assertNotNull(rv);
                long j = factory.getItemId(i);
                assertTrue(i == j);
            }
        } catch (RemoteException re) {
            Log.e(TAG, "exception in getting view");
        }

        // other api test.
        try {
            rv = factory.getLoadingView();
            assertNull(rv);  // now we return null.
            int i = factory.getViewTypeCount();
            assertEquals(3, i);
        } catch (RemoteException re) {
            Log.e(TAG, "exception during testing misc api");
        }

    }
}
