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


package com.mediatek.bluetooth.bip;

/*
import java.util.HashMap;
import com.mediatek.bluetooth.opp.adp.OppTask.OppTaskMetaData;
*/

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.content.Context;
import android.content.Intent;

import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;

import android.net.Uri;
import android.text.TextUtils;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class BipProvider extends ContentProvider {

    private static final String TAG = "BipProvider";

    //private DatabaseHelper database;
    private BipDatabaseHelper bipDbHelper;

    private static final String DATABASE_NAME = "bip.db";
    private static final int DATABASE_VERSION = 5;
    private static final String JOB_TABLE = "jobs";


    //table colum names 
    public static final String KEY_ID = "_id";
    public static final String KEY_OBJECT_URI = "object_uri";
    public static final String KEY_OBJECT_NAME = "object_name";
    public static final String KEY_OBJECT_MIME = "object_mime";
    public static final String KEY_OBJECT_SIZE = "object_size";
    public static final String KEY_REMOTE_ADDR = "remote_addr";
    public static final String KEY_REMOTE_NAME = "remote_name";
    public static final String KEY_CREATION_DATE = "creation_date";


    //table colum indexes
    public static final int ID_IDX = 0;
    public static final int OBJECT_URI_IDX = 1;
    public static final int OBJECT_NAME_IDX = 2;
    public static final int OBJECT_MIME_IDX = 3;
    public static final int OBJECT_SIZE_IDX = 4;
    public static final int REMOTE_ADDR_IDX = 5;
    public static final int REMOTE_NAME_IDX = 6;
    public static final int CREATION_DATE_IDX = 7;



    private static class BipDatabaseHelper extends SQLiteOpenHelper {


        private static final String DATABASE_CRATE =
                             "CREATE TABLE " + JOB_TABLE + " (" +
                             KEY_ID + " INTEGER PRIMARY KEY," +
                             KEY_OBJECT_URI + " TEXT," +
                             KEY_OBJECT_NAME + " TEXT," +
                             KEY_OBJECT_MIME + " TEXT," +
                             KEY_OBJECT_SIZE + " TEXT," +
                             KEY_REMOTE_ADDR + "  TEXT," +
                             KEY_REMOTE_NAME + " TEXT," +
                             KEY_CREATION_DATE + " INTEGER);";




        public BipDatabaseHelper(Context context, String name, SQLiteDatabase.CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL( DATABASE_CRATE );
        }
        
        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Xlog.w(TAG, "Upgrading database from version " + oldVersion + " to " + newVersion + " (will destroy all old data)!");
            db.execSQL("DROP TABLE IF EXISTS " + JOB_TABLE);
            this.onCreate(db);
        }
    }

    public static final String AUTHORITY = "com.mediatek.provider.bluetooth.bip";
    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + JOB_TABLE);
    public static final String CONTENT_TYPE = "vnd.mediatek.cursor.dir/vnd.mtkbt.bip.job";
    public static final String CONTENT_ITEM_TYPE = "vnd.mediatek.cursor.item/vnd.mtkbt.bip.job";


    private static final UriMatcher URI_MATCHER;
    private static final int MULTIPLE_TASK_URI = 1;
    private static final int SINGLE_TASK_URI = 2;
    static {
        URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
        URI_MATCHER.addURI(AUTHORITY, JOB_TABLE, MULTIPLE_TASK_URI);
        URI_MATCHER.addURI(AUTHORITY, JOB_TABLE + "/#", SINGLE_TASK_URI);
    }



    @Override
    public String getType(Uri uri) {

        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                return CONTENT_TYPE;
            case SINGLE_TASK_URI:
                return CONTENT_ITEM_TYPE;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }



    @Override
    public boolean onCreate() {

        //this.database = new DatabaseHelper(this.getContext());
        bipDbHelper = new BipDatabaseHelper(this.getContext(), DATABASE_NAME, null, DATABASE_VERSION); 
        return true;
    }



    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(JOB_TABLE);


        switch (URI_MATCHER.match(uri))
        {
            case MULTIPLE_TASK_URI:
                break;
          
            case SINGLE_TASK_URI:
                qb.appendWhere(KEY_ID + "=" + uri.getPathSegments().get(1));
                break;

            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        String orderBy;
        if (TextUtils.isEmpty(sortOrder)) {
            orderBy = KEY_ID;
        } else {
            orderBy = sortOrder;
        }

        SQLiteDatabase db = bipDbHelper.getReadableDatabase();

        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);
        //c.setNotificationUri(this.getContext().getContentResolver(), uri);

        return c;
    }


    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {

        // update database
        SQLiteDatabase db = this.bipDbHelper.getWritableDatabase();

        // change modified date
        //values.put( OppTaskMetaData.TASK_MODIFIED_DATE, System.currentTimeMillis() );

        int count;
        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                count = db.update(JOB_TABLE, values, selection, selectionArgs);
                break;
            case SINGLE_TASK_URI:
                String rowId = uri.getPathSegments().get(1);
                count = db.update(JOB_TABLE, values,
                                  KEY_ID + "=" + rowId + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                                  selectionArgs);
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        //this.getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }



    @Override
    public Uri insert(Uri uri, ContentValues values) {

        Xlog.v (TAG, "insert" );

       	if (URI_MATCHER.match(uri) != MULTIPLE_TASK_URI) {
            throw new IllegalArgumentException("Invalid URI: " + uri);
        }


        if( values.containsKey(KEY_OBJECT_URI) == false ||
            values.containsKey(KEY_REMOTE_ADDR) == false ||
            values.containsKey(KEY_REMOTE_NAME) == false ){
            //name, mime, size are updated during creating BipImage
            throw new SQLException( "insert fail: required property is null, uri: " + uri );
        }


        if( values.containsKey(KEY_CREATION_DATE) == false ){
            Xlog.v (TAG, "insert, put creation date" );
            values.put( KEY_CREATION_DATE, System.currentTimeMillis() );
        }


        // insert database
        long rowId = bipDbHelper.getWritableDatabase().insert(JOB_TABLE, null, values);
        if (rowId != -1) {
            Xlog.v (TAG, "notifiy change" );

            Uri recordUri = ContentUris.withAppendedId(CONTENT_URI, rowId);
            this.getContext().getContentResolver().notifyChange(recordUri, null);
            return recordUri;
        }
        else {
            throw new SQLException( "failed to do insert row, uri: " + uri );
        }
    }




    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {

        Xlog.v (TAG, "delete" );

        SQLiteDatabase db = bipDbHelper.getWritableDatabase();
        int count;

        switch (URI_MATCHER.match(uri)) {
            case MULTIPLE_TASK_URI:
                Xlog.v (TAG, "MULTIPLE_TASK_URI" );
                count = db.delete(JOB_TABLE, selection, selectionArgs);
                break;
            case SINGLE_TASK_URI:
                Xlog.v (TAG, "SINGLE_TASK_URI" );
                String rowId = uri.getPathSegments().get(1);
                count = db.delete(JOB_TABLE,
                                  KEY_ID + "=" + rowId + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                                  selectionArgs);
                break;
            default:
                Xlog.v (TAG, "DELETION" );
                throw new IllegalArgumentException("Unknown URI " + uri);
        } 
      	//this.getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }



}
