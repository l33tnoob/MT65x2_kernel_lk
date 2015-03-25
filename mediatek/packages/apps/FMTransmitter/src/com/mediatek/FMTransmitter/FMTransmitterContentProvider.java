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

package com.mediatek.FMTransmitter;

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

public class FMTransmitterContentProvider extends ContentProvider {
    private SQLiteDatabase  mSqlDb;
    private DatabaseHelper  mDbHelper;
    private static final String  DATABASE_NAME  = "FMTransmitter.db";
    public static final int DATABASE_VERSION  = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 2 : 1;
    private static final String TABLE_NAME  = "TxStationList";
    private static final String TAG = "FMTxAPK";
    private static final int STATION_FREQ = 1;
    private static final int STATION_FREQ_ID = 2;

    private static UriMatcher sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        sUriMatcher.addURI(FMTransmitterStation.AUTHORITY, FMTransmitterStation.STATION, STATION_FREQ);
        sUriMatcher.addURI(FMTransmitterStation.AUTHORITY, FMTransmitterStation.STATION + "/#", STATION_FREQ_ID);
    }

    public static class DatabaseHelper extends SQLiteOpenHelper {
        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }
        
        public void onCreate(SQLiteDatabase db) {
            // Create the table
            FMTxLogUtils.d(TAG, ">>> DatabaseHelper.onCreate");
            db.execSQL(
                "Create table "
                + TABLE_NAME
                + "("
                + FMTransmitterStation.Station._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," 
                + FMTransmitterStation.Station.COLUMN_STATION_NAME + " TEXT," 
                + FMTransmitterStation.Station.COLUMN_STATION_FREQ + " INTEGER,"
                + FMTransmitterStation.Station.COLUMN_STATION_TYPE + " INTEGER"
                + ");"
            );
            FMTxLogUtils.d(TAG, "<<< DatabaseHelper.onCreate");
        }

        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            FMTxLogUtils.d(TAG, "Upgrading database from version " + oldVersion + " to "
                    + newVersion + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS" + TABLE_NAME);
            onCreate(db);
        }
    }

    public int delete(Uri uri, String selection, String[] selectionArgs) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterContentProvider.delete");
        int iRet = 0;
        mSqlDb = mDbHelper.getWritableDatabase();
        switch (sUriMatcher.match(uri)) {
            case STATION_FREQ: 
                iRet = mSqlDb.delete(TABLE_NAME, selection, selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            case STATION_FREQ_ID: 
                String stationID = uri.getPathSegments().get(1);
                iRet = mSqlDb.delete(TABLE_NAME,
                        FMTransmitterStation.Station._ID
                        + "="
                        + stationID
                        + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ")" : ""),
                        selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            default: 
                FMTxLogUtils.e(TAG, "Error: Unkown URI to delete: " + uri);
                //throw new IllegalArgumentException("Unknown URI " + uri);
                break;
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterContentProvider.delete");
        return iRet;
    }

    public String getType(Uri uri) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterContentProvider.getType");
        FMTxLogUtils.d(TAG, "<<< FMTransmitterContentProvider.getType");
        return null;
    }

    public Uri insert(Uri uri, ContentValues values) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterContentProvider.insert");
        Uri rowUri = null;
        mSqlDb = mDbHelper.getWritableDatabase();
        ContentValues v = new ContentValues(values);
        // Do not insert invalid values.
        if (!(v.containsKey(FMTransmitterStation.Station.COLUMN_STATION_NAME)) 
            || !(v.containsKey(FMTransmitterStation.Station.COLUMN_STATION_FREQ)) 
            || !(v.containsKey(FMTransmitterStation.Station.COLUMN_STATION_TYPE))) {
            FMTxLogUtils.e(TAG, "Error: Invalid values.");
        } else {
            long rowId = mSqlDb.insert(TABLE_NAME, null, v);
            if (rowId > 0) {
                rowUri = ContentUris.appendId(FMTransmitterStation.Station.CONTENT_URI.buildUpon(), rowId).build();
                getContext().getContentResolver().notifyChange(rowUri, null);
            } else {
                FMTxLogUtils.e(TAG, "Error: Failed to insert row into " + uri);
            }
            FMTxLogUtils.d(TAG, "<<< FMTransmitterContentProvider.insert");
        }
        return rowUri;
        //throw new SQLException("Failed to insert row into "+ uri);
    }

    public boolean onCreate() {
        mDbHelper = new DatabaseHelper(getContext());
        return (mDbHelper == null) ? false : true;
    }

    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        //FMTxLogUtils.i(TAG, ">>> FMTransmitterContentProvider.query");
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        SQLiteDatabase db = mDbHelper.getReadableDatabase();
        qb.setTables(TABLE_NAME);
        //FMTxLogUtils.i(TAG, "query uri: " + uri.toString());

        int match = sUriMatcher.match(uri);
        switch (match) {
        case STATION_FREQ_ID:
            qb.appendWhere("_id = " + uri.getPathSegments().get(1));
            break;
        case STATION_FREQ:
            break;
        default:
            break;
        }

        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, sortOrder);
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }

        //FMTxLogUtils.i(TAG, "<<< FMTransmitterContentProvider.query");
        return c;
    }

    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterContentProvider.update");
        int iRet = 0;
        mSqlDb = mDbHelper.getWritableDatabase();
        switch (sUriMatcher.match(uri)) {
            case STATION_FREQ:
                iRet = mSqlDb.update(TABLE_NAME, values, selection, selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            case STATION_FREQ_ID:
                String stationID = uri.getPathSegments().get(1);
                iRet = mSqlDb.update(TABLE_NAME,
                        values,
                        FMTransmitterStation.Station._ID
                        + "="
                        + stationID
                        + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ")" : ""),
                        selectionArgs);
                getContext().getContentResolver().notifyChange(uri, null);
                break;
            default: 
                FMTxLogUtils.e(TAG, "Error: Unkown URI to update: " + uri);
                //throw new IllegalArgumentException("Unknown URI " + uri);
                break;
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterContentProvider.update");
        return iRet;
    }
    public DatabaseHelper getOpenHelperForTest() {
        return mDbHelper;
    }
}
