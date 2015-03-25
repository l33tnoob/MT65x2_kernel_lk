/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.schpwronoff;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

import com.mediatek.xlog.Xlog;

public class AlarmProvider extends ContentProvider {
    private static final String TAG = "AlarmProvider";
    private SQLiteOpenHelper mOpenHelper;

    private static final int SCHPWRS = 1;
    private static final int SCHPWRS_ID = 2;
    private static final UriMatcher URLMATCHER = new UriMatcher(UriMatcher.NO_MATCH);

    static {
        URLMATCHER.addURI("com.android.settings", "schpwr", SCHPWRS);
        URLMATCHER.addURI("com.android.settings", "schpwr/#", SCHPWRS_ID);
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final String DATABASE_NAME = "schpwrs.db";
        private static final int DATABASE_VERSION = 5;

        public DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE schpwrs (" + "_id INTEGER PRIMARY KEY," + "hour INTEGER, " + "minutes INTEGER, "
                    + "daysofweek INTEGER, " + "alarmtime INTEGER, " + "enabled INTEGER, " + "vibrate INTEGER, "
                    + "message TEXT, " + "alert TEXT);");

            // insert default alarms
            String insertMe = "INSERT INTO schpwrs "
                    + "(hour, minutes, daysofweek, alarmtime, enabled, vibrate, message, alert) " + "VALUES ";
            db.execSQL(insertMe + "(7, 0, 127, 0, 0, 1, '', '');");
            db.execSQL(insertMe + "(8, 30, 31, 0, 0, 1, '', '');");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            Xlog.d(TAG, "Upgrading schpwrs database from version " + oldVersion + " to " + currentVersion
                    + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS schpwrs");
            onCreate(db);
        }
    }

    /**
     * dummy constructor
     */
    public AlarmProvider() {
        super();
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri url, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        // Generate the body of the query
        int match = URLMATCHER.match(url);
        switch (match) {
        case SCHPWRS:
            qb.setTables("schpwrs");
            break;
        case SCHPWRS_ID:
            qb.setTables("schpwrs");
            qb.appendWhere("_id=");
            qb.appendWhere(url.getPathSegments().get(1));
            break;
        default:
            throw new IllegalArgumentException("Unknown URL " + url);
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor ret = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);

        if (ret == null) {
            Xlog.e(TAG, "Alarms.query: failed");
        } else {
            ret.setNotificationUri(getContext().getContentResolver(), url);
        }

        return ret;
    }

    @Override
    public String getType(Uri url) {
        int match = URLMATCHER.match(url);
        switch (match) {
        case SCHPWRS:
            return "vnd.android.cursor.dir/schpwrs";
        case SCHPWRS_ID:
            return "vnd.android.cursor.item/schpwrs";
        default:
            throw new IllegalArgumentException("Unknown URL");
        }
    }

    @Override
    public int update(Uri url, ContentValues values, String where, String[] whereArgs) {
        int count;
        long rowId = 0;
        int match = URLMATCHER.match(url);
        try {
            SQLiteDatabase db = mOpenHelper.getWritableDatabase();
            switch (match) {
            case SCHPWRS_ID: 
                String segment = url.getPathSegments().get(1);
                rowId = Long.parseLong(segment);
                count = db.update("schpwrs", values, "_id=" + rowId, null);
                break;
            default: 
                throw new UnsupportedOperationException("Cannot update URL: " + url);
            }
            
            Xlog.d(TAG, "*** notifyChange() rowId: " + rowId + " url " + url);
            getContext().getContentResolver().notifyChange(url, null);
            return count;
        } catch (SQLiteDiskIOException e) {
            Xlog.e(TAG, e.toString());
            return 0;
        }
    }

    @Override
    public Uri insert(Uri url, ContentValues initialValues) {
        Xlog.d(TAG, "---------->>> alarm provider");
        if (URLMATCHER.match(url) != SCHPWRS) {
            throw new IllegalArgumentException("Cannot insert into URL: " + url);
        }

        ContentValues values;
        if (initialValues == null) {
            values = new ContentValues();
        } else {
            values = new ContentValues(initialValues);
        }

        if (!values.containsKey(Alarm.Columns.HOUR)) {
            values.put(Alarm.Columns.HOUR, 0);
        }

        if (!values.containsKey(Alarm.Columns.MINUTES)) {
            values.put(Alarm.Columns.MINUTES, 0);
        }

        if (!values.containsKey(Alarm.Columns.DAYS_OF_WEEK)) {
            values.put(Alarm.Columns.DAYS_OF_WEEK, 0);
        }

        if (!values.containsKey(Alarm.Columns.ALARM_TIME)) {
            values.put(Alarm.Columns.ALARM_TIME, 0);
        }

        if (!values.containsKey(Alarm.Columns.ENABLED)) {
            values.put(Alarm.Columns.ENABLED, 0);
        }

        if (!values.containsKey(Alarm.Columns.VIBRATE)) {
            values.put(Alarm.Columns.VIBRATE, 1);
        }

        if (!values.containsKey(Alarm.Columns.MESSAGE)) {
            values.put(Alarm.Columns.MESSAGE, "");
        }

        if (!values.containsKey(Alarm.Columns.ALERT)) {
            values.put(Alarm.Columns.ALERT, "");
        }

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        long rowId = db.insert("schpwrs", Alarm.Columns.MESSAGE, values);
        if (rowId < 0) {
            throw new SQLException("Failed to insert row into " + url);
        }
        Xlog.d(TAG, "Added alarm rowId = " + rowId);

        Uri newUrl = ContentUris.withAppendedId(Alarm.Columns.CONTENT_URI, rowId);
        getContext().getContentResolver().notifyChange(newUrl, null);
        return newUrl;
    }

    @Override
    public int delete(Uri url, String where, String[] whereArgs) {
        Xlog.d(TAG, "---->> delete alarm provider");
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count;
        String whr;
        switch (URLMATCHER.match(url)) {
        case SCHPWRS:
            count = db.delete("schpwrs", where, whereArgs);
            break;
        case SCHPWRS_ID:
            String segment = url.getPathSegments().get(1);
            if (TextUtils.isEmpty(where)) {
                whr = "_id=" + segment;
            } else {
                whr = "_id=" + segment + " AND (" + where + ")";
            }
            count = db.delete("schpwrs", whr, whereArgs);
            break;
        default:
            throw new IllegalArgumentException("Cannot delete from URL: " + url);
        }

        getContext().getContentResolver().notifyChange(url, null);
        return count;
    }
}
