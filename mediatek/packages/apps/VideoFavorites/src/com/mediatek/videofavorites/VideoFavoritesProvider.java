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

package com.mediatek.videofavorites;

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

import com.mediatek.storage.StorageManagerEx;
import com.mediatek.videofavorites.VideoFavoritesProviderValues.Columns;
import com.mediatek.xlog.Xlog;

public class VideoFavoritesProvider extends ContentProvider {

    protected static final String TAG = "VideoFavoritesProvider";

    /**
     * The method to print log
     * @param tag       the tag of the class
     * @param msg       the log message to print
     */
    protected static void log(String tag, String msg) {
        if (VideoFavoritesProviderValues.DEBUG) {
            Xlog.d(tag, msg);
        }
    }

    // define Content URI
    private static final Uri CONTENT_URI = Columns.CONTENT_URI;

    // define urimatcher
    private static final int URI_DATAS = 1;
    private static final int URI_DATAS_ID = 2;

    private static final UriMatcher SURLMATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        SURLMATCHER.addURI(VideoFavoritesProviderValues.AUTHORITY, "datas", URI_DATAS);
        SURLMATCHER.addURI(VideoFavoritesProviderValues.AUTHORITY, "datas/#", URI_DATAS_ID);
    }

    /**
     * This class helps to create, open, upgrade database file.
     */
    private static class VideoFavoritesDatabaseHelper extends SQLiteOpenHelper {

        public VideoFavoritesDatabaseHelper(Context context) {
            super(context, Columns.DATABASE_NAME, null, Columns.DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            log(TAG, "video favorite provider on create");

            db.execSQL("CREATE TABLE " + Columns.DATABASE_TABLE_NAME + " ("
                       + Columns._ID + " INTEGER PRIMARY KEY,"
                       + Columns.VIDEO_URI + " TEXT,"
                       + Columns.CONTACT_URI + " TEXT,"
                       + Columns.NAME + " TEXT,"
                       + Columns.STORAGE + " INTEGER"
                       + ");");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            log(TAG, "upgrading database from version " + oldVersion + "to " + newVersion);
            db.execSQL("DROP TABLE IF EXISTS " + Columns.DATABASE_TABLE_NAME);
            onCreate(db);
        }
    }

    private VideoFavoritesDatabaseHelper mOpenHelper;

    @Override
    public boolean onCreate() {
        mOpenHelper = new VideoFavoritesDatabaseHelper(getContext());

        return true;
    }

    @Override
    public String getType(Uri uri) {
        // TODO: auto-generated method stub
        return null;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        log(TAG, "video contact provider start querying ");

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(Columns.DATABASE_TABLE_NAME);

        switch (SURLMATCHER.match(uri)) {
        case URI_DATAS:
            break;
        case URI_DATAS_ID:
            qb.appendWhere("_id=" + uri.getPathSegments().get(1));
            break;
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor ret = qb.query(db, projection, selection, selectionArgs, null, null, sortOrder);

        if (ret == null) {
            log(TAG, "video contact query fail!");
        } else {
            ret.setNotificationUri(getContext().getContentResolver(), uri);
        }
        return ret;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        log(TAG, "video contact provider start updating ");

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String extraSelection = null;

        switch (SURLMATCHER.match(uri)) {
        case URI_DATAS:
            break;
        case URI_DATAS_ID:
            extraSelection = Columns._ID + "=" +  uri.getPathSegments().get(1);
            break;
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }

        // if values contains VIDEO_URI, update the storage path.
        if (values.containsKey(Columns.VIDEO_URI)) {
            values.put(Columns.STORAGE, getStorageType(values.getAsString(Columns.VIDEO_URI)));
        }

        String updateSelection = concatenateSelection(selection, extraSelection);
        int count = db.update(Columns.DATABASE_TABLE_NAME, values, updateSelection, selectionArgs);

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        log(TAG, "video contact provider start deleting");

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count = 0;
        String extraSelection = null;
        String deleteSelection = null;

        switch (SURLMATCHER.match(uri)) {
        case URI_DATAS:
            break;
        case URI_DATAS_ID:
            extraSelection = Columns._ID + "=" +  uri.getPathSegments().get(1);
            break;
        default:
            throw new IllegalArgumentException("Unknown URI " + uri);
        }

        deleteSelection = concatenateSelection(selection, extraSelection);
        count = db.delete(Columns.DATABASE_TABLE_NAME, deleteSelection, selectionArgs);

        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        log(TAG, "video contact provider start inserting");

        if (SURLMATCHER.match(uri) != URI_DATAS) {
            throw new IllegalArgumentException("cannot insert into URI " + uri);
        }

        ContentValues values = new ContentValues(initialValues);

        if (!values.containsKey(Columns.VIDEO_URI)) {
            values.put(Columns.VIDEO_URI, "");
            log(TAG, "insert() VIDEO_URI = null");
            values.put(Columns.STORAGE, 0);
        } else {
            values.put(Columns.STORAGE, getStorageType(values.getAsString(Columns.VIDEO_URI)));
        }

        if (!values.containsKey(Columns.CONTACT_URI)) {
            values.put(Columns.CONTACT_URI, "");
            log(TAG, "insert() CONTACT_URI = null");
        }

        if (!values.containsKey(Columns.NAME)) {
            values.put(Columns.NAME, "");
            log(TAG, "insert() NAME = null");
        }

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        long rowId = db.insert(Columns.DATABASE_TABLE_NAME, null, values);

        if (rowId < 0) {
            throw new SQLException("Fail to insert row into " + uri);
        }

        Uri newUri = ContentUris.withAppendedId(CONTENT_URI, rowId);
        getContext().getContentResolver().notifyChange(uri, null);
        return newUri;

    }

    private static String concatenateSelection(String selection, String extraSelection) {
        if (TextUtils.isEmpty(selection)) {
            return extraSelection;
        }
        if (TextUtils.isEmpty(extraSelection)) {
            return selection;
        }
        return "(" + selection + ") AND (" + extraSelection + ")";
    }

    // To support SD hot swap, we re-write content provider, ensure it
    // update correct drive while saving videoUri and update correct drive
    // while getting video uri.
    private int getStorageType(String videoUri) {
        if ("".equals(videoUri)) {
            log(TAG, "getStorageType(): Unknown");
            return VideoFavoritesProviderValues.STORAGE_UNKNOWN;
        }

        int type = VideoFavoritesProviderValues.STORAGE_UNKNOWN;
        int indexInt = videoUri.indexOf(StorageManagerEx.getInternalStoragePath());
        int indexExt = videoUri.indexOf(StorageManagerEx.getExternalStoragePath());
        if (indexInt != -1 && indexExt != -1) {
            type = (indexInt < indexExt) ? VideoFavoritesProviderValues.STORAGE_INTERNAL
                    : VideoFavoritesProviderValues.STORAGE_EXTERNAL;
        } else if (indexInt >= 0) {
            type = VideoFavoritesProviderValues.STORAGE_INTERNAL;
        } else if (indexExt >= 0) {
            type = VideoFavoritesProviderValues.STORAGE_EXTERNAL;
        }

        log(TAG, "getStoragetype():" + type);
        return type;
    }

    // To support SD hot swap, use storage type and videoUri to generate
    // correct file path uri.
    public static String getRealVideoURI(String videoUri, int storageType) {
        if (storageType == VideoFavoritesProviderValues.STORAGE_UNKNOWN || "".equals(videoUri)
                || videoUri == null) {
            return videoUri;
        }
        String storagePath = (storageType == VideoFavoritesProviderValues.STORAGE_INTERNAL) ?
            StorageManagerEx.getInternalStoragePath() :
            StorageManagerEx.getExternalStoragePath();
        int i = videoUri.indexOf(storagePath);
        if (i != -1) {
            // same path, return directly
            return videoUri;
        }
        String retStr = "file://" + storagePath + videoUri.substring(7 + storagePath.length());

        return retStr;
    }
}
