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

package com.mediatek.FMRadio;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

import com.mediatek.common.featureoption.FeatureOption;
/**
 * This class provider interface to operator FM database table StationList
 *
 */
public class FMRadioContentProvider extends ContentProvider {
    private static final String TAG = "FmRx/Provider"; // log tag

    private SQLiteDatabase mSqlDB = null; // database instance use to operate the database
    private DatabaseHelper mDbHelper = null;  // database helper use to get database instance
    private static final String DATABASE_NAME  = "FMRadio.db"; // database name
    // database version
    public static final int DATABASE_VERSION  = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 2 : 1; 
    private static final String TABLE_NAME  = "StationList"; // table name
    
    private static final int STATION_FREQ = 1; // URI match code  
    private static final int STATION_FREQ_ID = 2; // URI match code
    // use to match URI
    private static final UriMatcher URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    
    // match URI with station frequency or station frequency id
    static {
        URI_MATCHER.addURI(FMRadioStation.AUTHORITY, FMRadioStation.STATION, STATION_FREQ);
        URI_MATCHER.addURI(FMRadioStation.AUTHORITY, FMRadioStation.STATION + "/#", STATION_FREQ_ID);
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {
        
        /**
         * initial database name and database version
         * @param context application context
         */
        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }
        
        /**
         * create database table
         * @param db database
         */
        public void onCreate(SQLiteDatabase db) {
            // Create the table
            LogUtils.d(TAG, "DatabaseHelper.onCreate");
            db.execSQL(
                "Create table "
                + TABLE_NAME
                + "("
                + FMRadioStation.Station._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," 
                + FMRadioStation.Station.COLUMN_STATION_NAME + " TEXT," 
                + FMRadioStation.Station.COLUMN_STATION_FREQ + " INTEGER,"
                + FMRadioStation.Station.COLUMN_STATION_TYPE + " INTEGER"
                + ");"
            );
        }
        
        /**
         * upgrade database
         * @param db database
         * @param oldVersion old database version
         * @param newVersion new database version
         */
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            LogUtils.i(TAG, "Upgrading database from version " + oldVersion + " to "
                    + newVersion + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS" + TABLE_NAME);
            onCreate(db);
        }
    }
    
    /**
     * delete database table rows with condition
     * @param uri match URI
     * @param selection the where cause to apply, if null will delete all rows
     * @param selectionArgs the select value
     * @return rows number has be deleted
     */
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        LogUtils.d(TAG, "FMRadioContentProvider.delete");
        int rows = 0;
        mSqlDB = mDbHelper.getWritableDatabase();
        switch (URI_MATCHER.match(uri)) {
            case STATION_FREQ: 
                rows = mSqlDB.delete(TABLE_NAME, selection, selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
                
            case STATION_FREQ_ID:
                String stationID = uri.getPathSegments().get(1);
                rows = mSqlDB.delete(TABLE_NAME,
                        FMRadioStation.Station._ID
                        + "="
                        + stationID
                        + (TextUtils.isEmpty(selection) ? "" : " AND (" + selection + ")"),
                        selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
                
            default: 
                LogUtils.e(TAG, "Error: Unkown URI to delete: " + uri);
                break;
        }
        return rows;
    }
    
    /**
     * insert values to database with uri
     * @param uri table uri
     * @param values insert values
     * @return uri after inserted
     */
    public Uri insert(Uri uri, ContentValues values) {
        LogUtils.d(TAG, "FMRadioContentProvider.insert");
        Uri rowUri = null;
        mSqlDB = mDbHelper.getWritableDatabase();
        ContentValues v = new ContentValues(values);
        // Do not insert invalid values.
        if (!v.containsKey(FMRadioStation.Station.COLUMN_STATION_NAME)
                || !v.containsKey(FMRadioStation.Station.COLUMN_STATION_FREQ)
                || !v.containsKey(FMRadioStation.Station.COLUMN_STATION_TYPE)) {
            LogUtils.e(TAG, "Error: Invalid values.");
            return rowUri;
        }
        
        long rowId = mSqlDB.insert(TABLE_NAME, null, v);

        if (rowId <= 0) {
            LogUtils.e(TAG, "Error: Failed to insert row into " + uri);
        }
        
        rowUri = ContentUris.appendId(FMRadioStation.Station.CONTENT_URI.buildUpon(),rowId)
                .build();
        getContext().getContentResolver().notifyChange(rowUri, null);
        return rowUri;
    }
    
    /**
     * create database helper
     * @return whether create database helper success
     */
    public boolean onCreate() {
        mDbHelper = new DatabaseHelper(getContext());
        return (null == mDbHelper) ? false : true;
    }
    
    /**
     * query the database with current settings and add information
     * @param uri database uri
     * @param projection  the columns need to query
     * @param selection where clause
     * @param selectionArgs where value
     * @param sortOrder according which column to sort
     * @return cursor which is result
     */
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        SQLiteDatabase db = mDbHelper.getReadableDatabase();
        qb.setTables(TABLE_NAME);

        int match = URI_MATCHER.match(uri);
        
        if (STATION_FREQ_ID == match) {
            qb.appendWhere("_id = " + uri.getPathSegments().get(1));
        }
        
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, sortOrder);
        if (null != c) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }
        return c;
    }
    
    /**
     * update the database content use content values with current settings and add information
     * @param uri database uri
     * @param values values need to update
     * @param selection where clause
     * @param selectionArgs where value
     * @return row numbers have changed
     */
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        LogUtils.d(TAG, "FMRadioContentProvider.update");
        int rows = 0;
        mSqlDB = mDbHelper.getWritableDatabase();
        switch (URI_MATCHER.match(uri)) {
            case STATION_FREQ: 
                rows = mSqlDB.update(TABLE_NAME, values, selection, selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            case STATION_FREQ_ID:
                String stationID = uri.getPathSegments().get(1);
                rows = mSqlDB.update(TABLE_NAME,
                        values,
                        FMRadioStation.Station._ID
                        + "="
                        + stationID
                        + (TextUtils.isEmpty(selection) ? "" : " AND (" + selection + ")"),
                        selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            default:
                LogUtils.e(TAG, "Error: Unkown URI to update: " + uri);
                break;
        }
        return rows;
    }
    
    /**
     *  get uri type
     *  @param uri the uri
     *  @return uri type
     */
    @Override
    public String getType(Uri uri) {
        return null;
    }
}
