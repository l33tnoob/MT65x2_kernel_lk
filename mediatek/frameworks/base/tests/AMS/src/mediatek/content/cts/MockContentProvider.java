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

package android.content.cts;

import java.util.HashMap;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

public class MockContentProvider extends ContentProvider {

    private SQLiteOpenHelper mOpenHelper;

    private static final String AUTHORITY = "ctstest";
    private static final String DBNAME = "ctstest.db";
    private static final int DBVERSION = 2;

    private static final UriMatcher URL_MATCHER;
    private static final int TESTTABLE1 = 1;
    private static final int TESTTABLE1_ID = 2;
    private static final int TESTTABLE2 = 3;
    private static final int TESTTABLE2_ID = 4;

    private static HashMap<String, String> CTSDBTABLE1_LIST_PROJECTION_MAP;
    private static HashMap<String, String> CTSDBTABLE2_LIST_PROJECTION_MAP;

    private static class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {
            super(context, DBNAME, null, DBVERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE TestTable1 ("
                    + "_id INTEGER PRIMARY KEY, " + "key TEXT, " + "value INTEGER"
                    + ");");

            db.execSQL("CREATE TABLE TestTable2 ("
                    + "_id INTEGER PRIMARY KEY, " + "key TEXT, " + "value INTEGER"
                    + ");");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            db.execSQL("DROP TABLE IF EXISTS TestTable1");
            db.execSQL("DROP TABLE IF EXISTS TestTable2");
            onCreate(db);
        }
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String segment;
        int count;

        switch (URL_MATCHER.match(uri)) {
        case TESTTABLE1:
            if (null == selection) {
                // get the count when remove all rows
                selection = "1";
            }
            count = db.delete("TestTable1", selection, selectionArgs);
            break;
        case TESTTABLE1_ID:
            segment = uri.getPathSegments().get(1);
            count = db.delete("TestTable1", "_id=" + segment +
                    (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                    selectionArgs);
            break;
        case TESTTABLE2:
            count = db.delete("TestTable2", selection, selectionArgs);
            break;
        case TESTTABLE2_ID:
            segment = uri.getPathSegments().get(1);
            count = db.delete("TestTable2", "_id=" + segment +
                    (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                    selectionArgs);
            break;
        default:
            throw new IllegalArgumentException("Unknown URL " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public String getType(Uri uri) {
        switch (URL_MATCHER.match(uri)) {
        case TESTTABLE1:
            return "vnd.android.cursor.dir/com.android.content.testtable1";
        case TESTTABLE1_ID:
            return "vnd.android.cursor.item/com.android.content.testtable1";
        case TESTTABLE2:
            return "vnd.android.cursor.dir/com.android.content.testtable2";
        case TESTTABLE2_ID:
            return "vnd.android.cursor.item/com.android.content.testtable2";

        default:
            throw new IllegalArgumentException("Unknown URL " + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        long rowID;
        ContentValues values;
        String table;
        Uri testUri;

        if (initialValues != null)
            values = new ContentValues(initialValues);
        else
            values = new ContentValues();

        if (values.containsKey("value") == false)
            values.put("value", -1);

        switch (URL_MATCHER.match(uri)) {
        case TESTTABLE1:
            table = "TestTable1";
            testUri = Uri.parse("content://" + AUTHORITY + "/testtable1");
            break;
        case TESTTABLE2:
            table = "TestTable2";
            testUri = Uri.parse("content://" + AUTHORITY + "/testtable2");
            break;
        default:
            throw new IllegalArgumentException("Unknown URL " + uri);
        }

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        rowID = db.insert(table, "key", values);

        if (rowID > 0) {
            Uri url = ContentUris.withAppendedId(testUri, rowID);
            getContext().getContentResolver().notifyChange(url, null);
            return url;
        }

        throw new SQLException("Failed to insert row into " + uri);
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        switch (URL_MATCHER.match(uri)) {
        case TESTTABLE1:
            qb.setTables("TestTable1");
            qb.setProjectionMap(CTSDBTABLE1_LIST_PROJECTION_MAP);
            break;

        case TESTTABLE1_ID:
            qb.setTables("TestTable1");
            qb.appendWhere("_id=" + uri.getPathSegments().get(1));
            break;

        case TESTTABLE2:
            qb.setTables("TestTable2");
            qb.setProjectionMap(CTSDBTABLE2_LIST_PROJECTION_MAP);
            break;

        case TESTTABLE2_ID:
            qb.setTables("TestTable2");
            qb.appendWhere("_id=" + uri.getPathSegments().get(1));
            break;

        default:
            throw new IllegalArgumentException("Unknown URL " + uri);
        }

        /* If no sort order is specified use the default */
        String orderBy;
        if (TextUtils.isEmpty(sortOrder))
            orderBy = "_id";
        else
            orderBy = sortOrder;

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);

        c.setNotificationUri(getContext().getContentResolver(), uri);
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String segment;
        int count;

        switch (URL_MATCHER.match(uri)) {
        case TESTTABLE1:
            count = db.update("TestTable1", values, selection, selectionArgs);
            break;

        case TESTTABLE1_ID:
            segment = uri.getPathSegments().get(1);
            count = db.update("TestTable1", values, "_id=" + segment +
                    (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                    selectionArgs);
            break;

        case TESTTABLE2:
            count = db.update("TestTable2", values, selection, selectionArgs);
            break;

        case TESTTABLE2_ID:
            segment = uri.getPathSegments().get(1);
            count = db.update("TestTable2", values, "_id=" + segment +
                    (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                    selectionArgs);
            break;

        default:
            throw new IllegalArgumentException("Unknown URL " + uri);
        }

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    static {
        URL_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
        URL_MATCHER.addURI(AUTHORITY, "testtable1", TESTTABLE1);
        URL_MATCHER.addURI(AUTHORITY, "testtable1/#", TESTTABLE1_ID);
        URL_MATCHER.addURI(AUTHORITY, "testtable2", TESTTABLE2);
        URL_MATCHER.addURI(AUTHORITY, "testtable2/#", TESTTABLE2_ID);

        CTSDBTABLE1_LIST_PROJECTION_MAP = new HashMap<String, String>();
        CTSDBTABLE1_LIST_PROJECTION_MAP.put("_id", "_id");
        CTSDBTABLE1_LIST_PROJECTION_MAP.put("key", "key");
        CTSDBTABLE1_LIST_PROJECTION_MAP.put("value", "value");

        CTSDBTABLE2_LIST_PROJECTION_MAP = new HashMap<String, String>();
        CTSDBTABLE2_LIST_PROJECTION_MAP.put("_id", "_id");
        CTSDBTABLE2_LIST_PROJECTION_MAP.put("key", "key");
        CTSDBTABLE2_LIST_PROJECTION_MAP.put("value", "value");
    }
}
