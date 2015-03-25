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
import com.mediatek.videofavorites.VideoFavoritesProviderValues.Columns;
import android.test.ProviderTestCase2;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.ContentUris;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;
import java.util.Set;
import java.util.Map;


/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.mediatek.videofavorites.VideoFavoritesProviderTests \
 * com.mediatek.videofavorites.tests/android.test.InstrumentationTestRunner
 */
public class VideoFavoritesProviderTests extends ProviderTestCase2<VideoFavoritesProvider> {

    private static final String TAG = "VideoFavoritesProviderTests";
    private Uri contentUri;
    private ContentResolver mResolver;

    /**
     * The method to print log
     * @param tag    	the tag of the class
     * @param msg		the log message to print
     */
    protected static void log(String tag, String msg) {
        if (VideoFavoritesProviderValues.DEBUG) {
            Log.d(tag, msg);
        }
    }

    //Constructor
    public VideoFavoritesProviderTests() {
        super(VideoFavoritesProvider.class, VideoFavoritesProviderValues.AUTHORITY);
        log(TAG, "constructor for VideoFavoritesProviderTests");
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        log(TAG, "setUp() for VideoFavoritesProviderTests");

        contentUri = Uri.parse("content://" + VideoFavoritesProviderValues.AUTHORITY + "/datas");
        mResolver = getMockContext().getContentResolver();

        // test empty database for setup time
        Cursor c = mResolver.query(contentUri, null, null, null, null);
        assertNotNull(c);
        assertEquals(0, c.getCount());
        c.close();
    }

    @Override
    public void tearDown() throws Exception {
        super.tearDown();
        log(TAG, "tearDown() for VideoFavoritesProviderTests");

        // before next test, clear all entries in database
        Cursor c = mResolver.query(contentUri,  null, null, null, null);
        assertNotNull(c);

        log(TAG, "tearDown(), count:" + c.getCount());

        if (c.getCount() > 0) {
            mResolver.delete(contentUri, null, null);
        }
        c.close();

        // before next test, clear all entries in database
        c = mResolver.query(contentUri,  null, null, null, null);
        assertNotNull(c);
        assertEquals(0, c.getCount());
        c.close();
    }

    public void testSetUp() {
        assertTrue(true);
    }

    public void testTearDown() {
        assertTrue(true);
    }

    public void testInsertWithNormalData() {
        log(TAG, "testInsertData() for VideoFavoritesProviderTests");

        ContentValues values = new ContentValues();
        putRawData(values, 1);

        Uri dataUri = mResolver.insert(contentUri, values);

        assertNotNull(dataUri);
        // check the query and stored data
        assertCheckStoredData(dataUri, values);
    }

    public void testInsertDataWithAbnormalData() {
        log(TAG, "testInsertDataWithoutVideoInfo() for VideoFavoritesProviderTests");

        ContentValues values = new ContentValues();
        Uri dataUri = null;

        // insert data without video uri
        putRawData(values, 1);
        values.remove(Columns.VIDEO_URI);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);
        values.clear();

        // insert data without contact uri
        putRawData(values, 2);
        values.remove(Columns.CONTACT_URI);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);
        values.clear();

        // insert data without name
        putRawData(values, 3);
        values.remove(Columns.NAME);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);

        // insert data with only video uri
        putRawData(values, 4);
        values.remove(Columns._ID);
        values.remove(Columns.CONTACT_URI);
        values.remove(Columns.NAME);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);

        // insert data with only contact uri
        putRawData(values, 5);
        values.remove(Columns._ID);
        values.remove(Columns.VIDEO_URI);
        values.remove(Columns.NAME);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);

        // insert data without no info
        putRawData(values, 6);
        values.remove(Columns._ID);
        values.remove(Columns.VIDEO_URI);
        values.remove(Columns.CONTACT_URI);
        values.remove(Columns.NAME);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        assertCheckStoredData(dataUri, values);
    }

    public void testQueryData() {
        log(TAG, "testQueryData() for VideoFavoritesProviderTests");

        // create/insert a data
        ContentValues values = new ContentValues();
        putRawData(values, 1);
        Uri dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        // query data by append dataId into URI
        long dataId = ContentUris.parseId(dataUri);
        Cursor c = mResolver.query(ContentUris.withAppendedId(contentUri, dataId), null, null, null, null);
        assertTrue(c.moveToFirst());

        values.clear();
        getQueriedInfo(c, values);
        c.close();

        // check the query and stored data
        assertCheckStoredData(dataUri, values);
    }

    public void testQueryDataWithColumnInfo() {
        log(TAG, "testQueryDataWithVideoUri() for VideoFavoritesProviderTests");

        ContentValues values = new ContentValues();
        Uri dataUri = null;
        Cursor c = null;

        // create/insert 1st data
        putRawData(values, 1);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);

        // query data by video uri  & check data
        c = mResolver.query(contentUri, null, Columns.VIDEO_URI + "=?", new String[] {"1.avi"}, null);
        assertTrue(c.moveToFirst());
        values.clear();
        getQueriedInfo(c, values);
        assertCheckStoredData(dataUri, values);
        c.close();

        // create/insert 2nd data
        putRawData(values, 2);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);
        // query data by contact uri  & check data
        c = mResolver.query(contentUri, null, Columns.CONTACT_URI + "=?", new String[] {"102"}, null);
        assertTrue(c.moveToFirst());
        values.clear();
        getQueriedInfo(c, values);
        assertCheckStoredData(dataUri, values);
        c.close();

        // create/insert 3nd data
        putRawData(values, 3);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);

        // query data by name  & check data
        c = mResolver.query(contentUri, null, Columns.NAME + "=?", new String[] {"Fion3"}, null);
        assertTrue(c.moveToFirst());
        values.clear();
        getQueriedInfo(c, values);
        assertCheckStoredData(dataUri, values);
        c.close();
    }

    public void testUpdateColumn() {
        log(TAG, "testUpdateColumn() for VideoFavoritesProviderTests");

        ContentValues values = new ContentValues();
        Uri dataUri = null;

        // create/insert 1st data
        putRawData(values, 1);
        dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);

        // update video uri
        assertCheckUpdateColumn(dataUri, Columns.VIDEO_URI, "2.avi");

        // update contact uri
        assertCheckUpdateColumn(dataUri, Columns.CONTACT_URI, "103");

        // update name
        assertCheckUpdateColumn(dataUri, Columns.NAME, "Stanley");
    }

    public void testDeleteData() {
        log(TAG, "testDeleteData() for VideoFavoritesProviderTests");

        // create/insert a data
        ContentValues values = new ContentValues();
        putRawData(values, 1);
        Uri dataUri = mResolver.insert(contentUri, values);
        assertNotNull(dataUri);

        // delete with uri/id
        mResolver.delete(dataUri, null, null);

        // check delete successfully or not
        Cursor c = mResolver.query(dataUri,  null, null, null, null);
        assertNotNull(c);
        assertEquals(0, c.getCount());
        c.close();
    }

    protected void putRawData(ContentValues values, int index) {
        values.put(Columns._ID, index);
        values.put(Columns.VIDEO_URI, index + ".avi");
        values.put(Columns.CONTACT_URI, "10" + index);
        values.put(Columns.NAME, "Fion" + index);
    }

    protected void getQueriedInfo(Cursor c, ContentValues values) {

        long queryResultId = c.getLong(c.getColumnIndex(Columns._ID));
        String queryResultVideoUri = c.getString(c.getColumnIndex(Columns.VIDEO_URI));
        String queryResultContactUri = c.getString(c.getColumnIndex(Columns.CONTACT_URI));
        String queryResultName = c.getString(c.getColumnIndex(Columns.NAME));

        values.put(Columns._ID, queryResultId);
        values.put(Columns.VIDEO_URI, queryResultVideoUri);
        values.put(Columns.CONTACT_URI, queryResultContactUri);
        values.put(Columns.NAME, queryResultName);
    }

    protected void assertCheckStoredData(Uri uri, ContentValues values) {

        // check the query and stored data
        Cursor c = mResolver.query(uri, null, null, null, null);
        assertEquals("Record count", 1, c.getCount());
        c.moveToFirst();

        // check stored count
        try {
            Set<Map.Entry<String, Object>> entries = values.valueSet();
            for (Map.Entry<String, Object> entry: entries) {
                String column = entry.getKey();

                // check stored count
                int index = c.getColumnIndex(column);
                assertTrue("No such column: " + column, index != -1);

                Object expectedValue = values.getAsString(column);
                String value = c.getString(index);

                log(TAG, "assertCheckStoredData(), expectedValue: " + expectedValue + "value: " + value);
                assertEquals("Columns value " + column, expectedValue, value);
            }
        } finally {
            c.close();
        }
    }

    protected void assertEqualsContentValues(ContentValues values1, ContentValues values2) {

        Set<Map.Entry<String, Object>> entries = values1.valueSet();
        for (Map.Entry<String, Object> entry: entries) {
            String column = entry.getKey();

            Object expectedValue1 = values1.getAsString(column);
            Object expectedValue2 = values2.getAsString(column);

            log(TAG, "assertCheckStoredData(), expectedValue1: " + expectedValue1 + "expectedValue2: " + expectedValue2);
            assertEquals("value: " , expectedValue1, expectedValue2);
        }
    }

    protected void assertCheckUpdateColumn(Uri uri, String column, String newString) {
        log(TAG, "assertCheckUpdateColumn() for VideoFavoritesProviderTests");

        Cursor c = null;
        ContentValues storedResult = new ContentValues();
        ContentValues queryResult = new ContentValues();

        // get queried result & replace the updated column with new data
        c = mResolver.query(contentUri,  null, null, null, null);
        assertTrue(c.moveToFirst());
        getQueriedInfo(c, storedResult);
        storedResult.remove(column);
        storedResult.put(column, newString);
        c.close();

        // update video uri
        queryResult.put(column, newString);
        mResolver.update(uri, queryResult, null, null);
        queryResult.clear();

        // get new data info
        c = mResolver.query(contentUri, null, column + "=?", new String[] {newString}, null);
        assertTrue(c.moveToFirst());
        getQueriedInfo(c, queryResult);
        c.close();

        // check equal
        assertEqualsContentValues(queryResult, storedResult);
    }

}
