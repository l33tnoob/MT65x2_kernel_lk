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

package com.mediatek.FMTransmitter.tests;

import java.lang.reflect.Field;

import com.mediatek.FMTransmitter.FMTransmitterContentProvider;
import com.mediatek.FMTransmitter.FMTransmitterStation;
import com.mediatek.FMTransmitter.FMTransmitterStation.Station;
import android.test.ProviderTestCase2;
import android.test.mock.MockContentProvider;
import android.test.mock.MockContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.provider.BaseColumns;

public class FMTransmitterContentProviderTestCase extends 
       ProviderTestCase2<FMTransmitterContentProvider> {
    
    private MockContentResolver mMockResolver;
    private SQLiteDatabase mDb;
    
    public static final String AUTHORITY  = "com.mediatek.FMTransmitter.FMTransmitterContentProvider";
    public static final String STATION = "TxStation";
    private static final int STATION_FREQ = 1;
    private static final int STATION_FREQ_ID = 2;
    
    static final String columns[] = new String[] {
        Station._ID,
        Station.COLUMN_STATION_NAME,
        Station.COLUMN_STATION_FREQ,
        // Use this type to identify different stations.
        Station.COLUMN_STATION_TYPE
    };
    
    public FMTransmitterContentProviderTestCase() {
        super(FMTransmitterContentProvider.class,FMTransmitterStation.AUTHORITY);
    }
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMockResolver = getMockContentResolver();
        mDb = getProvider().getOpenHelperForTest().getWritableDatabase();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testcase01_Insert() {
        ContentValues v = new ContentValues();
        v.put(Station.COLUMN_STATION_NAME, "Unoccupied channel");
        v.put(Station.COLUMN_STATION_FREQ, 8750);
        v.put(Station.COLUMN_STATION_TYPE, FMTransmitterStation.STATION_TYPE_SEARCHED);
        Uri rowUri = mMockResolver.insert(Station.CONTENT_URI, v);
        
        Cursor cursor = mMockResolver.query(Station.CONTENT_URI, columns, null, null, null);
        assertEquals(1, cursor.getCount());
        assertTrue(cursor.moveToFirst());
        
        int nameIndex = cursor.getColumnIndex(Station.COLUMN_STATION_NAME);
        int freqIndex = cursor.getColumnIndex(Station.COLUMN_STATION_FREQ);
        int typeIndex = cursor.getColumnIndex(Station.COLUMN_STATION_TYPE);
        
        assertEquals("Unoccupied channel", cursor.getString(nameIndex));
        assertEquals(8750, cursor.getInt(freqIndex));
        assertEquals(FMTransmitterStation.STATION_TYPE_SEARCHED, cursor.getInt(typeIndex));
        
        long valueId = ContentUris.parseId(rowUri);
        v.put(Station._ID, (int)valueId);
        rowUri = mMockResolver.insert(Station.CONTENT_URI, v);
        assertNull(rowUri);
    }
    
    public void testcase02_Delete() {
        final String SELECTION_COLUMNS = Station.COLUMN_STATION_FREQ + " = " + "?";
        final String[] SELECTION_ARGS = {"8750"};
        
        insertData(3);
        int rowDeletedAll = mMockResolver.delete(Station.CONTENT_URI, null, null);
        assertEquals(3, rowDeletedAll);
        Cursor cursor = mMockResolver.query(Station.CONTENT_URI, columns, null, null, null);
        assertEquals(0, cursor.getCount());
        
        insertData(3);
        int rowDeletedOne = mMockResolver.delete(Station.CONTENT_URI, SELECTION_COLUMNS, SELECTION_ARGS);
        assertEquals(1, rowDeletedOne);
        cursor = mMockResolver.query(Station.CONTENT_URI, columns, null, null, null);
        assertEquals(2, cursor.getCount());
    }
    
    public void testcase03_Update() {
        final String SELECTION_COLUMNS = Station.COLUMN_STATION_FREQ + " = " + "?";
        final String[] SELECTION_ARGS = {"8750"};
        
        insertData(3);
        ContentValues v = new ContentValues();
        v.put(Station.COLUMN_STATION_NAME, "Unoccupied channel");
        v.put(Station.COLUMN_STATION_FREQ, 8750);
        v.put(Station.COLUMN_STATION_TYPE, FMTransmitterStation.STATION_TYPE_SEARCHED);
        int updateNumber = mMockResolver.update(Station.CONTENT_URI, v, null, null);
        Cursor cursor = mMockResolver.query(Station.CONTENT_URI, columns, SELECTION_COLUMNS, SELECTION_ARGS, null);
        int queryNumber = cursor.getCount();
        assertEquals(updateNumber, queryNumber);
        
        mMockResolver.delete(Station.CONTENT_URI, null, null);
        insertData(3);
        SELECTION_ARGS[0] = "8760";
        cursor = null;
        queryNumber = 0;
        mMockResolver.update(Station.CONTENT_URI, v, SELECTION_COLUMNS, SELECTION_ARGS);
        SELECTION_ARGS[0] = "8750";
        cursor = mMockResolver.query(Station.CONTENT_URI, columns, SELECTION_COLUMNS,SELECTION_ARGS, null);
        queryNumber = cursor.getCount();
        assertEquals(2, queryNumber);
    }
    
    public void testcase04_QueryStationsUri() {
        Cursor cursor = mMockResolver.query(Station.CONTENT_URI, columns, null, null, null);
        assertEquals(0, cursor.getCount());
        insertData(3);
        cursor = mMockResolver.query(Station.CONTENT_URI, columns, null, null, null);
        assertEquals(3, cursor.getCount());
    }
    
    public void testcase05_QueryStationIdUri() {
        Uri uri = ContentUris.withAppendedId(Station.CONTENT_URI, 1);
        Cursor cursor = mMockResolver.query(uri, columns, null, null, null);
        assertEquals(0, cursor.getCount());
        insertData(3);
        cursor = mMockResolver.query(uri, columns, null, null, null);
        assertEquals(1, cursor.getCount());
    }
    
    private void insertData(int number) {
        for (int index = 0;index < number;index ++) {
            ContentValues v = new ContentValues();
            v.put(Station.COLUMN_STATION_NAME, "Unoccupied channel");
            v.put(Station.COLUMN_STATION_FREQ, 8750+index*10);
            v.put(Station.COLUMN_STATION_TYPE, FMTransmitterStation.STATION_TYPE_SEARCHED);
            Uri rowUri = mMockResolver.insert(Station.CONTENT_URI, v);
        }
    }
    
    public static final class Station implements BaseColumns {
        public static final Uri CONTENT_URI  = Uri.parse("content://"+ AUTHORITY + "/" + STATION);
        // Extra columns of the table: COLUMN_STATION_NAME COLUMN_STATION_FREQ COLUMN_STATION_TYPE
        public static final String COLUMN_STATION_NAME = "COLUMN_STATION_NAME";
        public static final String COLUMN_STATION_FREQ = "COLUMN_STATION_FREQ";
        public static final String COLUMN_STATION_TYPE = "COLUMN_STATION_TYPE";
    }
}
