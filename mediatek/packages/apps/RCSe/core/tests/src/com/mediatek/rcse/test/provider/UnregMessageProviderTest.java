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

package com.mediatek.rcse.test.provider;

import java.lang.reflect.Field;
import java.sql.SQLException;

import android.content.ContentValues;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.provider.UnregMessageProvider.UregMessageHelper;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

/**
 * The class is used to test UnregMessageProvider
 */
public class UnregMessageProviderTest extends InstrumentationTestCase {
    private static final String TAG = "UnregMessageProviderTest";
    private Context mContext = null;
    private static final int NUM_0 = 0;
    private static final int NUM_1 = 1;
    private static final int NEG_NUM_1 = -1;
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private static final Uri INVALID_URI = Uri.parse("content://com.android.contacts/contacts/as_vcard");
    private ContentResolver mResolver = null;
    private UregMessageHelper mHelper = null;
    private SQLiteDatabase mDb = null;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mHelper = new UregMessageHelper(mContext);
        assertNotNull(mHelper);
        mDb = mHelper.getWritableDatabase();
        assertNotNull(mDb);
        mResolver = mContext.getContentResolver();
        mResolver.delete(UnregMessageProvider.CONTENT_URI, null, null);
        ContentValues values = new ContentValues();
        values.put(UnregMessageProvider.KEY_CHAT_TAG, UnregMessageProvider.KEY_CHAT_TAG);
        values.put(UnregMessageProvider.KEY_MESSAGE_TAG, UnregMessageProvider.KEY_MESSAGE_TAG);
        values.put(UnregMessageProvider.KEY_MESSAGE, UnregMessageProvider.KEY_MESSAGE);
        mResolver.insert(UnregMessageProvider.CONTENT_URI, values);
        Logger.d(TAG, "setUp() entry");
    }

    /**
     * Test case for insert()
     */
    public void testCase1_Update() {
        Logger.d(TAG, "testCase1_Update() entry");
        int ret = mResolver.update(UnregMessageProvider.CONTENT_URI, null, null, null);
        assertEquals(NUM_0, ret);
        Logger.d(TAG, "testCase1_Update() exit");
    }

    /**
     * Test case for getType()
     */
    public void testCase2_GetType() {
        Logger.d(TAG, "testCase2_GetType() entry");
        String ret = mResolver.getType(UnregMessageProvider.CONTENT_URI);
        assertNull(ret);
        Logger.d(TAG, "testCase2_GetType() exit");
    }

    /**
     * Test case for delete()
     */
    public void testCase3_Delete() {
        Logger.d(TAG, "testCase3_Delete() entry");
        int ret = mResolver.delete(UnregMessageProvider.CONTENT_URI, null, null);
        assertEquals(NUM_1, ret);
        Logger.d(TAG, "testCase3_Delete() exit");
    }

    /**
     * Test case for insert()
     */
    public void testCase4_Insert() {
        Logger.d(TAG, "testCase4_Insert() entry");
        ContentValues values = new ContentValues();
        values.put(UnregMessageProvider.KEY_CHAT_TAG, "chat tag");
        values.put(UnregMessageProvider.KEY_MESSAGE_TAG, "message tag");
        values.put(UnregMessageProvider.KEY_MESSAGE, "message");
        Uri ret = mResolver.insert(UnregMessageProvider.CONTENT_URI, values);
        assertNotNull(ret);
        Logger.d(TAG, "testCase4_Insert() exit");
    }
    
    /**
     * Test case for query()
     */
    public void testCase5_Query() {
        Logger.d(TAG, "testCase5_Query() entry");
        Cursor ret = mResolver.query(UnregMessageProvider.CONTENT_URI, null, null,null,null);
        assertNotNull(ret);
        int count = ret.getCount();
        ret.close();
        assertTrue(count == NUM_1);
        Logger.d(TAG, "testCase5_Query() exit");
    }
    
    /**
     * Test case for onUpgrade()
     */
    public void testCase6_OnUpgrade() {
        Logger.d(TAG, "testCase6_OnUpgrade() entry");
        int oldVer = mDb.getVersion();
        int newVer = oldVer + 1;
        mHelper.onUpgrade(mDb, oldVer, newVer);
        assertEquals(newVer, mDb.getVersion());
        Logger.d(TAG, "testCase6_OnUpgrade() exit");
    }
    
    public void tearDown() throws Exception {
        super.tearDown();
        int oldVersion = mDb.getVersion();
        if (oldVersion != UnregMessageProvider.DM_VERSION) {
            mHelper.onUpgrade(mDb, oldVersion, UnregMessageProvider.DM_VERSION);
        }
    }
}