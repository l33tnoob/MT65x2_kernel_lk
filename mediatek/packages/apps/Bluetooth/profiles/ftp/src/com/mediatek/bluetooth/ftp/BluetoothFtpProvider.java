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


package com.mediatek.bluetooth.ftp;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.FolderContent;
import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.TransferringFile;

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
import android.util.Log;

import java.util.HashMap;

/**
 * BluetoothFtpProvider Notes.
 *   There are 3 tables in BluetoothFtpProvider.
 *     1. For listing folder content of current path on FTP server.
 *     2. For listing folder content of current local path.
 *     3. For keeping a list for transferring (downloading or uploading) files.
 *   About mark-several, we don't create additional tables bur provide an URI
 *   for a query which returns the pre-defined result as a sub-table.
 */
public class BluetoothFtpProvider extends ContentProvider {
	private static final String TAG = "BluetoothFtpProvider";

	private static final String AUTHORITY = BluetoothFtpProviderHelper.AUTHORITY;

	private static final String DATABASE_NAME = "bt_ftp.db";
	private static final int DATABASE_VER = 2;

	private static final int UNKNOWN_SIZE = BluetoothFtpProviderHelper.UNKNOWN_SIZE;

	/* Table names */
	private static final String TABLE_SERVER_FOLDER_CONTENT	= "server_cur_folder",
								TABLE_LOCAL_FOLDER_CONTENT	= "local_cur_folder",
								TABLE_TRANSFERRING			= "transferring";

	/* Private constances for Uri-matching */
	private static final int FTP_BASE			= 0,
							 FTP_SERVER_FOLDER_CONTENT	= FTP_BASE + 1,
							 FTP_SERVER_MARKS			= FTP_BASE + 2,
							 FTP_LOCAL_FOLDER_CONTENT	= FTP_BASE + 3,
							 FTP_LOCAL_MARKS			= FTP_BASE + 4,
							 FTP_TRANSFERRING			= FTP_BASE + 5,
							 FTP_TRANSFERRING_ID		= FTP_BASE + 6;

	/* For matching and classifying URIs of incoming queries */
	private static UriMatcher sUriMatcher;

	/* Projection map for current folder content */
	private static HashMap<String, String> sFolderContentProjection;

	/* Projection map for markable files in current folder */
	private static HashMap<String, String> sMarksProjection;

	/* Projection map for files in transferring list */
	private static HashMap<String, String> sTransferProjection;

	private static class DatabaseHelper extends SQLiteOpenHelper {

		public DatabaseHelper(Context context) {
			super(context, DATABASE_NAME, null,  DATABASE_VER);
		}

		@Override
		public void onCreate(SQLiteDatabase db) {
			db.execSQL("CREATE TABLE " + TABLE_SERVER_FOLDER_CONTENT + " ("
						+ FolderContent._ID + " INTEGER PRIMARY KEY,"
						+ FolderContent.NAME + " TEXT,"
						+ FolderContent.TYPE + " INTEGER,"
						+ FolderContent.SIZE + " INTEGER,"
						+ FolderContent.MODIFIED_DATE + " TEXT"
						+ ");");

			db.execSQL("CREATE TABLE " + TABLE_LOCAL_FOLDER_CONTENT + " ("
						+ FolderContent._ID + " INTEGER PRIMARY KEY,"
						+ FolderContent.NAME + " TEXT,"
						+ FolderContent.TYPE + " INTEGER,"
						+ FolderContent.SIZE + " INTEGER,"
						+ FolderContent.MODIFIED_DATE + " TEXT"
						+ ");");

			db.execSQL("CREATE TABLE " + TABLE_TRANSFERRING + " ("
						+ TransferringFile._ID + " INTEGER PRIMARY KEY,"
						+ TransferringFile.NAME + " TEXT,"
						+ TransferringFile.STATUS + " INTEGER,"
						+ TransferringFile.PROGRESS + " INTEGER,"
						+ TransferringFile.TOTAL + " INTEGER,"
						+ TransferringFile.DIRECTION + " INTEGER"
						+ ");");
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			Log.w(TAG, "Upgrading database from version " + oldVersion
						+ " to " + newVersion + " (will destroy all old data)!");

			db.execSQL("DROP TABLE IF EXISTS " + TABLE_SERVER_FOLDER_CONTENT);
			db.execSQL("DROP TABLE IF EXISTS " + TABLE_LOCAL_FOLDER_CONTENT);
			db.execSQL("DROP TABLE IF EXISTS " + TABLE_TRANSFERRING);
			this.onCreate(db);
		}
	}

    private DatabaseHelper mOpenHelper;

	@Override
	public boolean onCreate() {
		mOpenHelper = new DatabaseHelper(getContext());
		return true;
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
			String sortOrder) {
		SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
		String orderBy = null;

		switch (sUriMatcher.match(uri)) {
			case FTP_SERVER_FOLDER_CONTENT:
				qb.setTables(TABLE_SERVER_FOLDER_CONTENT);
				qb.setProjectionMap(sFolderContentProjection);
				orderBy = FolderContent.DEFAULT_SORT_ORDER;
				break;

			case FTP_SERVER_MARKS:
				qb.setTables(TABLE_SERVER_FOLDER_CONTENT);
				qb.setProjectionMap(sMarksProjection);
				qb.appendWhere(FolderContent.TYPE + "!=" + FolderContent.TYPE_FOLDER);
				orderBy = FolderContent.DEFAULT_SORT_ORDER;
				break;

			case FTP_LOCAL_FOLDER_CONTENT:
				qb.setTables(TABLE_LOCAL_FOLDER_CONTENT);
				qb.setProjectionMap(sFolderContentProjection);
				orderBy = FolderContent.DEFAULT_SORT_ORDER;
				break;

			case FTP_LOCAL_MARKS:
				qb.setTables(TABLE_LOCAL_FOLDER_CONTENT);
				qb.setProjectionMap(sMarksProjection);
				qb.appendWhere(FolderContent.TYPE + "!=" + FolderContent.TYPE_FOLDER);
				orderBy = FolderContent.DEFAULT_SORT_ORDER;
				break;

			case FTP_TRANSFERRING:
				qb.setTables(TABLE_TRANSFERRING);
				qb.setProjectionMap(sTransferProjection);
				orderBy = TransferringFile.DEFAULT_SORT_ORDER;
				break;

			case FTP_TRANSFERRING_ID:
				qb.setTables(TABLE_TRANSFERRING);
				qb.setProjectionMap(sTransferProjection);
				qb.appendWhere(TransferringFile._ID + "=" + uri.getPathSegments().get(1));
				orderBy = TransferringFile.DEFAULT_SORT_ORDER;
				break;

			default:
				Log.w(TAG, "Unknown URI: " + uri);
				return null;
		}

		SQLiteDatabase db = mOpenHelper.getReadableDatabase();
		Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);
		if (c != null) {
			c.setNotificationUri(getContext().getContentResolver(), uri);
		}

		return c;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		Long now = Long.valueOf(System.currentTimeMillis());
		String table = null, name = null;

		switch (sUriMatcher.match(uri)) {
			case FTP_SERVER_FOLDER_CONTENT:
				if (values.containsKey(FolderContent.NAME) == false) {
					values.put(FolderContent.NAME, "");
				}
				if (values.containsKey(FolderContent.TYPE) == false) {
					values.put(FolderContent.TYPE, FolderContent.TYPE_TEXT);
				}
				if (values.containsKey(FolderContent.SIZE) == false) {
					values.put(FolderContent.SIZE, 0);
				}
				if (values.containsKey(FolderContent.MODIFIED_DATE) == false) {
					values.put(FolderContent.MODIFIED_DATE, now);
				}

				table = TABLE_SERVER_FOLDER_CONTENT;
				name = FolderContent.NAME;
				break;

			case FTP_LOCAL_FOLDER_CONTENT:
				if (values.containsKey(FolderContent.NAME) == false) {
					values.put(FolderContent.NAME, "");
				}
				if (values.containsKey(FolderContent.TYPE) == false) {
					values.put(FolderContent.TYPE, FolderContent.TYPE_TEXT);
				}
				if (values.containsKey(FolderContent.SIZE) == false) {
					values.put(FolderContent.SIZE, 0);
				}
				if (values.containsKey(FolderContent.MODIFIED_DATE) == false) {
					values.put(FolderContent.MODIFIED_DATE, now);
				}

				table = TABLE_LOCAL_FOLDER_CONTENT;
				name = FolderContent.NAME;
				break;

			case FTP_TRANSFERRING:
				if (values.containsKey(TransferringFile.NAME) == false) {
					values.put(TransferringFile.NAME, "");
				}
				if (values.containsKey(TransferringFile.STATUS) == false) {
					values.put(TransferringFile.STATUS, TransferringFile.STATUS_FAILED);
				}
				if (values.containsKey(TransferringFile.PROGRESS) == false) {
					values.put(TransferringFile.PROGRESS, 0);
				}
				if (values.containsKey(TransferringFile.TOTAL) == false) {
					values.put(TransferringFile.TOTAL, 0);
				}
				if (values.containsKey(TransferringFile.DIRECTION) == false) {
					values.put(TransferringFile.DIRECTION, TransferringFile.DIRECTION_PULL);
				}

				table = TABLE_TRANSFERRING;
				name = TransferringFile.NAME;
				break;

			default:
				Log.w(TAG, "Unknown URI: " + uri);
				return uri;
		}

		SQLiteDatabase db = mOpenHelper.getWritableDatabase();
		if (db.insert(table, name, values) <= 0) {
			Log.e(TAG, "Insert failed: (" + table + ", " + values + ")");
		}
		return uri;
	}

	@Override
	public int delete(Uri uri, String where, String[] whereArgs) {
		SQLiteDatabase db = mOpenHelper.getWritableDatabase();
		int count = -1;

		switch (sUriMatcher.match(uri)) {
			case FTP_SERVER_FOLDER_CONTENT:
				count = db.delete(TABLE_SERVER_FOLDER_CONTENT, where, whereArgs);
				break;

			case FTP_LOCAL_FOLDER_CONTENT:
				count = db.delete(TABLE_LOCAL_FOLDER_CONTENT, where, whereArgs);
				break;

			case FTP_TRANSFERRING:
				count = db.delete(TABLE_TRANSFERRING, where, whereArgs);
				break;

			case FTP_TRANSFERRING_ID:
				String id = uri.getPathSegments().get(1);
				count = db.delete(TABLE_TRANSFERRING, TransferringFile._ID + "=" + id
					+ (TextUtils.isEmpty(where) ? "" : " AND + (" + where + ")"), whereArgs);
				break;

			default:
				Log.w(TAG, "Unknown URI: " + uri);
				return 0;
		}
		return count;
	}

	@Override
	public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
		SQLiteDatabase db = mOpenHelper.getWritableDatabase();
		int count = -1;

		switch (sUriMatcher.match(uri)) {
			case FTP_SERVER_FOLDER_CONTENT:
				count = db.update(TABLE_SERVER_FOLDER_CONTENT, values, where, whereArgs);
				break;

			case FTP_LOCAL_FOLDER_CONTENT:
				count = db.update(TABLE_LOCAL_FOLDER_CONTENT, values, where, whereArgs);
				break;

			case FTP_TRANSFERRING:
				count = db.update(TABLE_TRANSFERRING, values, where, whereArgs);
				break;

			case FTP_TRANSFERRING_ID:
				String id = uri.getPathSegments().get(1);
				count = db.update(TABLE_TRANSFERRING, values, TransferringFile._ID + "=" + id
					+ (TextUtils.isEmpty(where) ? "" : " AND + (" + where + ")"), whereArgs);
				break;

			default:
				Log.w(TAG, "Unknown URI: " + uri);
				return 0;
		}
		return count;
	}

	@Override
	public String getType(Uri uri) {
		switch (sUriMatcher.match(uri)) {
			case FTP_SERVER_FOLDER_CONTENT:
			case FTP_SERVER_MARKS:
			case FTP_LOCAL_FOLDER_CONTENT:
			case FTP_LOCAL_MARKS:
				return FolderContent.CONTENT_TYPE;
			case FTP_TRANSFERRING:
				return TransferringFile.CONTENT_TYPE;
			case FTP_TRANSFERRING_ID:
				return TransferringFile.CONTENT_ITEM_TYPE;
			default:
				Log.w(TAG, "Unknown URI: " + uri);
				break;
		}
		return null;
	}

	static {
		sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		sUriMatcher.addURI(AUTHORITY, "server_cur_folder", FTP_SERVER_FOLDER_CONTENT);
		sUriMatcher.addURI(AUTHORITY, "server_cur_folder/marks", FTP_SERVER_MARKS);
		sUriMatcher.addURI(AUTHORITY, "local_cur_folder", FTP_LOCAL_FOLDER_CONTENT);
		sUriMatcher.addURI(AUTHORITY, "local_cur_folder/marks", FTP_LOCAL_MARKS);
		sUriMatcher.addURI(AUTHORITY, "transferring", FTP_TRANSFERRING);
		sUriMatcher.addURI(AUTHORITY, "transferring/#", FTP_TRANSFERRING_ID);

		sFolderContentProjection = new HashMap<String, String>();
		sFolderContentProjection.put(FolderContent._ID, FolderContent._ID);
		sFolderContentProjection.put(FolderContent.NAME, FolderContent.NAME);
		sFolderContentProjection.put(FolderContent.TYPE, FolderContent.TYPE);
		sFolderContentProjection.put(FolderContent.SIZE, FolderContent.SIZE);
		sFolderContentProjection.put(FolderContent.MODIFIED_DATE, FolderContent.MODIFIED_DATE);

		sMarksProjection = new HashMap<String, String>();
		sMarksProjection.put(FolderContent._ID, FolderContent._ID);
		sMarksProjection.put(FolderContent.NAME, FolderContent.NAME);
		sMarksProjection.put(FolderContent.SIZE, FolderContent.SIZE);
		sMarksProjection.put(FolderContent.MODIFIED_DATE, FolderContent.MODIFIED_DATE);

		sTransferProjection = new HashMap<String, String>();
		sTransferProjection.put(TransferringFile._ID, TransferringFile._ID);
		sTransferProjection.put(TransferringFile.NAME, TransferringFile.NAME);
		sTransferProjection.put(TransferringFile.STATUS, TransferringFile.STATUS);
		sTransferProjection.put(TransferringFile.PROGRESS, TransferringFile.PROGRESS);
		sTransferProjection.put(TransferringFile.TOTAL, TransferringFile.TOTAL);
		sTransferProjection.put(TransferringFile.DIRECTION, TransferringFile.DIRECTION);
	}
}
