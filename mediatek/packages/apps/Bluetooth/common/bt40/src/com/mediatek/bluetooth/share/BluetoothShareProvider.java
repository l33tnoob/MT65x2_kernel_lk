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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.share;

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

import com.mediatek.xlog.Xlog;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;

import java.io.File;
import java.util.HashMap;

public class BluetoothShareProvider extends ContentProvider {

    private static final String TAG = "BluetoothShareProvider";

    public static final String AUTHORITY = "com.mediatek.provider.bluetooth.share";

    public static final String DATABASE_NAME = "share.db";

    public static final int DATABASE_VERSION = 1;

    // Column projection map
    private static HashMap<String, String> sProjectionMap = new HashMap<String, String>();
    static {
        sProjectionMap.put(BluetoothShareTaskMetaData._ID, BluetoothShareTaskMetaData._ID);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_TYPE, BluetoothShareTaskMetaData.TASK_TYPE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTaskMetaData.TASK_STATE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_RESULT, BluetoothShareTaskMetaData.TASK_RESULT);

        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_OBJECT_NAME, BluetoothShareTaskMetaData.TASK_OBJECT_NAME);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_OBJECT_URI, BluetoothShareTaskMetaData.TASK_OBJECT_URI);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_OBJECT_FILE, BluetoothShareTaskMetaData.TASK_OBJECT_FILE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_MIMETYPE, BluetoothShareTaskMetaData.TASK_MIMETYPE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_PEER_NAME, BluetoothShareTaskMetaData.TASK_PEER_NAME);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_PEER_ADDR, BluetoothShareTaskMetaData.TASK_PEER_ADDR);

        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES, BluetoothShareTaskMetaData.TASK_TOTAL_BYTES);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_DONE_BYTES, BluetoothShareTaskMetaData.TASK_DONE_BYTES);

        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_CREATION_DATE, BluetoothShareTaskMetaData.TASK_CREATION_DATE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE, BluetoothShareTaskMetaData.TASK_MODIFIED_DATE);
        sProjectionMap.put(BluetoothShareTaskMetaData.TASK_IS_HANDOVER, BluetoothShareTaskMetaData.TASK_IS_HANDOVER);
    }

    // Uri Matcher
    private static final UriMatcher URI_MATCHER;

    private static final int MULTIPLE_TASK_URI = 1;

    private static final int SINGLE_TASK_URI = 2;
    static {
        URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
        URI_MATCHER.addURI(AUTHORITY, BluetoothShareTaskMetaData.TABLE_NAME, MULTIPLE_TASK_URI);
        URI_MATCHER.addURI(AUTHORITY, BluetoothShareTaskMetaData.TABLE_NAME + "/#", SINGLE_TASK_URI);
    }

    private DatabaseHelper mDbHelper;

    private SQLiteDatabase mDb;

    @Override
    public boolean onCreate() {

        this.mDbHelper = new DatabaseHelper(this.getContext());
        this.mDb = this.mDbHelper.getWritableDatabase();
        return true;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {

        File dbfile = new File(this.mDb.getPath());
        if (!dbfile.exists()) {
            // OppLog.w( "delete: ShareDB is not exist, need create db!");
            this.mDb.close();
        }

        this.mDb = this.mDbHelper.getWritableDatabase();

        int count;
        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                count = this.mDb.delete(BluetoothShareTaskMetaData.TABLE_NAME, selection, selectionArgs);
                break;
            case SINGLE_TASK_URI:
                String rowId = uri.getPathSegments().get(1);
                count = this.mDb.delete(BluetoothShareTaskMetaData.TABLE_NAME, BluetoothShareTaskMetaData._ID + "=" + rowId
                        + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""), selectionArgs);
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
        this.getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public String getType(Uri uri) {

        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                return BluetoothShareTaskMetaData.CONTENT_TYPE;
            case SINGLE_TASK_URI:
                return BluetoothShareTaskMetaData.CONTENT_ITEM_TYPE;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {

        File dbfile = new File(this.mDb.getPath());
        if (!dbfile.exists()) {

            // OppLog.w( "insert: ShareDB is not exist, need create db!");
            this.mDb.close();
        }

        if (URI_MATCHER.match(uri) != MULTIPLE_TASK_URI) {

            throw new IllegalArgumentException("Invalid URI: " + uri);
        }

        // required fields x 6
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_TYPE)
                || !values.containsKey(BluetoothShareTaskMetaData.TASK_STATE)
                || !values.containsKey(BluetoothShareTaskMetaData.TASK_OBJECT_URI)
                || !values.containsKey(BluetoothShareTaskMetaData.TASK_MIMETYPE)
                || !values.containsKey(BluetoothShareTaskMetaData.TASK_PEER_NAME)
                || !values.containsKey(BluetoothShareTaskMetaData.TASK_PEER_ADDR)) {

            throw new SQLException("insert fail: required property is null, uri: " + uri);
        }

        // optional fields x 5
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_OBJECT_NAME)) {

            values.put(BluetoothShareTaskMetaData.TASK_OBJECT_NAME, "");
        }
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_OBJECT_FILE)) {

            values.put(BluetoothShareTaskMetaData.TASK_OBJECT_FILE, "");
        }
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_RESULT)) {

            values.put(BluetoothShareTaskMetaData.TASK_RESULT, "");
        }
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES)) {

            values.put(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES, 0);
        }
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_DONE_BYTES)) {

            values.put(BluetoothShareTaskMetaData.TASK_DONE_BYTES, 0);
        }
        if( !values.containsKey(BluetoothShareTaskMetaData.TASK_IS_HANDOVER)){

            values.put( BluetoothShareTaskMetaData.TASK_IS_HANDOVER, 0 );
        } 

        // auto fields
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_CREATION_DATE)) {

            values.put(BluetoothShareTaskMetaData.TASK_CREATION_DATE, System.currentTimeMillis());
        }
        if (!values.containsKey(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE)) {

            values.put(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE, System.currentTimeMillis());
        }

        // insert database
        this.mDb = this.mDbHelper.getWritableDatabase();
        long rowId = this.mDb.insert(BluetoothShareTaskMetaData.TABLE_NAME, null, values);
        if (rowId != -1) {
            Uri recordUri = ContentUris.withAppendedId(BluetoothShareTaskMetaData.CONTENT_URI, rowId);
            this.getContext().getContentResolver().notifyChange(recordUri, null);
            return recordUri;
        } else {
            //throw new SQLException("failed to do insert row, uri: " + uri);
            return null;
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                qb.setTables(BluetoothShareTaskMetaData.TABLE_NAME);
                qb.setProjectionMap(sProjectionMap);
                break;
            case SINGLE_TASK_URI:
                qb.setTables(BluetoothShareTaskMetaData.TABLE_NAME);
                qb.setProjectionMap(sProjectionMap);
                qb.appendWhere(BluetoothShareTaskMetaData._ID + "=" + uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        String orderBy;
        if (TextUtils.isEmpty(sortOrder)) {

            orderBy = BluetoothShareTaskMetaData.DEFAULT_SORT_ORDER;
        } else {
            orderBy = sortOrder;
        }

        File dbfile = new File(this.mDb.getPath());
        if (!dbfile.exists()) {

            // OppLog.w( "insert: ShareDB is not exist, need create db!");
            this.mDb.close();
        }

        this.mDb = this.mDbHelper.getReadableDatabase();

        Cursor c = qb.query(this.mDb, projection, selection, selectionArgs, null, null, orderBy);
        if (c != null) {

            c.setNotificationUri(this.getContext().getContentResolver(), uri);
        }
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {

        // change modified date
        values.put(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE, System.currentTimeMillis());

        File dbfile = new File(this.mDb.getPath());
        if (!dbfile.exists()) {

            // OppLog.w( "update: ShareDB is not exist, need create db!");
            this.mDb.close();
        }

        // update database
        this.mDb = this.mDbHelper.getWritableDatabase();

        int count;
        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                count = this.mDb.update(BluetoothShareTaskMetaData.TABLE_NAME, values, selection, selectionArgs);
                break;
            case SINGLE_TASK_URI:
                String rowId = uri.getPathSegments().get(1);
                count = this.mDb.update(BluetoothShareTaskMetaData.TABLE_NAME, values, BluetoothShareTaskMetaData._ID + "="
                        + rowId + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""), selectionArgs);
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
        this.getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {

            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {

            db.execSQL("CREATE TABLE " + BluetoothShareTaskMetaData.TABLE_NAME + " (" + BluetoothShareTaskMetaData._ID
                    + " INTEGER PRIMARY KEY," + BluetoothShareTaskMetaData.TASK_TYPE + " INTEGER,"
                    + BluetoothShareTaskMetaData.TASK_STATE + " INTEGER," + BluetoothShareTaskMetaData.TASK_RESULT
                    + " TEXT," + BluetoothShareTaskMetaData.TASK_OBJECT_NAME + " TEXT,"
                    + BluetoothShareTaskMetaData.TASK_OBJECT_URI + " TEXT," + BluetoothShareTaskMetaData.TASK_OBJECT_FILE
                    + " TEXT," + BluetoothShareTaskMetaData.TASK_MIMETYPE + " TEXT,"
                    + BluetoothShareTaskMetaData.TASK_PEER_NAME + " TEXT," + BluetoothShareTaskMetaData.TASK_PEER_ADDR
                    + " TEXT," + BluetoothShareTaskMetaData.TASK_TOTAL_BYTES + " INTEGER,"
                    + BluetoothShareTaskMetaData.TASK_DONE_BYTES + " INTEGER,"
                    + BluetoothShareTaskMetaData.TASK_CREATION_DATE + " INTEGER,"
                    + BluetoothShareTaskMetaData.TASK_IS_HANDOVER + " INTEGER,"
                    + BluetoothShareTaskMetaData.TASK_MODIFIED_DATE + " INTEGER);");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

            Xlog.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion + " (will destroy all old data)!");
            db.execSQL("DROP TABLE IF EXISTS " + BluetoothShareTaskMetaData.TABLE_NAME);
            this.onCreate(db);
        }
    }
}
