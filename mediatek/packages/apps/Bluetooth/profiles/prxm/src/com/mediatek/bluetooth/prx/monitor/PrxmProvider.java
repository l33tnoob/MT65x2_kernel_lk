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

package com.mediatek.bluetooth.prx.monitor;

import java.util.HashMap;

import android.bluetooth.BluetoothAdapter;
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
import android.provider.BaseColumns;
import android.text.TextUtils;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.util.BtLog;

public class PrxmProvider extends ContentProvider {

	public static final String AUTHORITY = "com.mediatek.provider.bluetooth.prxm";
	public static final String DATABASE_NAME = "prxm.db";
	public static final int DATABASE_VERSION = 1;

	// Content Provider - BaseColumns
	public static interface BluetoothPrxmDeviceMetaData extends BaseColumns {

		public static final String TABLE_NAME = "devices";

		public static final Uri CONTENT_URI = Uri.parse( "content://" + PrxmProvider.AUTHORITY + "/" + TABLE_NAME );
		public static final String CONTENT_TYPE = "vnd.android.cursor.dir/vnd.mtkbt.prxm.device";
		public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/vnd.mtkbt.prxm.device";
		public static final String DEFAULT_SORT_ORDER = "addr DESC";

		public static final String DEVICE_ADDR = "addr";
		public static final String DEVICE_STATE = "state";
		public static final String DEVICE_LINK_LOSS_LEVEL = "link_ll";
		public static final String DEVICE_PATH_LOSS_LEVEL = "path_ll";
		public static final String DEVICE_PATH_LOSS_THRESHOLD = "path_lt";
		public static final String DEVICE_CREATION_DATE = "creation";
		public static final String DEVICE_MODIFIED_DATE = "modified";
	}

	/**
	 * BluetoothPrxmDevice - fetch data and create BluetoothPrxmDevice from Cursor object
	 * 
	 * @param cursor
	 */
	public static BluetoothPrxmDevice fetchDevice( Cursor cursor ){

		String addr = cursor.getString( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData.DEVICE_ADDR) );
		BluetoothPrxmDevice device = new BluetoothPrxmDevice( BluetoothAdapter.getDefaultAdapter().getRemoteDevice(addr) );
		device.setId( cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData._ID) ) );
		device.setCurrentState( (byte)cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData.DEVICE_STATE) ) );
		device.setLinkLossLevel( (byte)cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL) ) );
		device.setPathLossLevel( (byte)cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL) ) );
		device.setPathLossThreshold( (byte)cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD) ) );
		return device;
	}

	/**
	 * BluetoothPrxmDevice - create ContentValues for BluetoothPrxmDevice object
	 * 
	 * @return
	 */
	public static ContentValues getContentValues( BluetoothPrxmDevice device ){

		ContentValues values = new ContentValues();
		if( device.getId() != -1 ){

			// old record
			values.put( BluetoothPrxmDeviceMetaData._ID, device.getId() );
		}
		values.put( BluetoothPrxmDeviceMetaData.DEVICE_ADDR, device.getAddress() );
		values.put( BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL, device.getLinkLossLevel() );
		values.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL, device.getPathLossLevel() );
		values.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD, device.getPathLossThreshold() );
		return values;
	}

	/**
	 * BluetoothPrxmDevice - create BluetoothPrxmDevice array from Cursor object
	 * 
	 * @param cursor
	 * @return
	 */
	public static BluetoothPrxmDevice[] fetchDevices( Cursor cursor ){

		try {
			if( cursor == null || !cursor.moveToFirst() ){

				BtLog.i( "fetchDevices() - can't find any device from cursor." );
				return new BluetoothPrxmDevice[0];
			}

			if( Options.LL_DEBUG ){

				BtLog.d( "found [" + cursor.getCount() + "] device(s)." );
			}
			BluetoothPrxmDevice[] res = new BluetoothPrxmDevice[cursor.getCount()];
			for( int i=0; !cursor.isAfterLast(); cursor.moveToNext(), i++ ){

				res[i] = PrxmProvider.fetchDevice(cursor);
			}
			return res;
		}
		finally {
			if( cursor != null )	cursor.close();
		}
	}
	

	// Column projection map
	private static HashMap<String, String> PROJECTION_MAP = new HashMap<String, String>();
	static {
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData._ID, BluetoothPrxmDeviceMetaData._ID);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_ADDR, BluetoothPrxmDeviceMetaData.DEVICE_ADDR);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_STATE, BluetoothPrxmDeviceMetaData.DEVICE_STATE);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL, BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL, BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD, BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_CREATION_DATE, BluetoothPrxmDeviceMetaData.DEVICE_CREATION_DATE);
		PROJECTION_MAP.put(BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE, BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE);
	}

	// Uri Matcher
	private static final UriMatcher URI_MATCHER;
	private static final int MULTIPLE_DEVICE_URI = 1;
	private static final int SINGLE_DEVICE_URI = 2;
	static {
		URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
		URI_MATCHER.addURI(AUTHORITY, BluetoothPrxmDeviceMetaData.TABLE_NAME, MULTIPLE_DEVICE_URI);
		URI_MATCHER.addURI(AUTHORITY, BluetoothPrxmDeviceMetaData.TABLE_NAME + "/#", SINGLE_DEVICE_URI);
	}

	private DatabaseHelper database;

	@Override
	public boolean onCreate() {

		this.database = new DatabaseHelper(this.getContext());
		return true;
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {

		SQLiteDatabase db = this.database.getWritableDatabase();
		int count;
		switch (URI_MATCHER.match(uri)) {
			case MULTIPLE_DEVICE_URI:
				count = db.delete(BluetoothPrxmDeviceMetaData.TABLE_NAME, selection, selectionArgs);
				break;
			case SINGLE_DEVICE_URI:
				String rowId = uri.getPathSegments().get(1);
				count = db.delete(BluetoothPrxmDeviceMetaData.TABLE_NAME,
						BluetoothPrxmDeviceMetaData._ID + "=" + rowId + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
						selectionArgs);
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
			case MULTIPLE_DEVICE_URI:
				return BluetoothPrxmDeviceMetaData.CONTENT_TYPE;
			case SINGLE_DEVICE_URI:
				return BluetoothPrxmDeviceMetaData.CONTENT_ITEM_TYPE;
			default:
				throw new IllegalArgumentException("Unknown URI " + uri);
		}
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {

		if (URI_MATCHER.match(uri) != MULTIPLE_DEVICE_URI) {

			throw new IllegalArgumentException("Invalid URI: " + uri);
		}

		// required field(s): addr
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_ADDR) == false ){

			throw new SQLException( "insert fail: required property is null, uri: " + uri );
		}

		// optional fields: state, link loss level, path loss level, path loss threshold
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_STATE) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_STATE, PrxmConstants.PRXM_STATE_NEW );
		}
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL, PrxmConstants.PRXM_DEFAULT_LINK_LOSS_LEVEL );
		}
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL, PrxmConstants.PRXM_DEFAULT_PATH_LOSS_LEVEL );
		}
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD, PrxmConstants.PRXM_DEFAULT_PATH_LOSS_THRESHOLD );
		}

		// auto fields
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_CREATION_DATE) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_CREATION_DATE, System.currentTimeMillis() );
		}
		if( values.containsKey(BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE) == false ){

			values.put( BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE, System.currentTimeMillis() );
		}

		// insert database
		long rowId = this.database.getWritableDatabase().insert(BluetoothPrxmDeviceMetaData.TABLE_NAME, null, values);
		if (rowId != -1) {
			Uri recordUri = ContentUris.withAppendedId(BluetoothPrxmDeviceMetaData.CONTENT_URI, rowId);
			this.getContext().getContentResolver().notifyChange(recordUri, null);
			return recordUri;
		}
		else {
			throw new SQLException( "failed to do insert row, uri: " + uri );
		}
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {

		SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
		switch (URI_MATCHER.match(uri))
		{
			case MULTIPLE_DEVICE_URI:
				qb.setTables(BluetoothPrxmDeviceMetaData.TABLE_NAME);
				qb.setProjectionMap(PROJECTION_MAP);
				break;
			case SINGLE_DEVICE_URI:
				qb.setTables(BluetoothPrxmDeviceMetaData.TABLE_NAME);
				qb.setProjectionMap(PROJECTION_MAP);
				qb.appendWhere(BluetoothPrxmDeviceMetaData._ID + "=" + uri.getPathSegments().get(1));
				break;
			default:
				throw new IllegalArgumentException("Unknown URI " + uri);
		}

		String orderBy;
		if (TextUtils.isEmpty(sortOrder)) {

			orderBy = BluetoothPrxmDeviceMetaData.DEFAULT_SORT_ORDER;
		} else {
			orderBy = sortOrder;
		}

		SQLiteDatabase db = this.database.getReadableDatabase();
		Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);
		if( c != null ){

			c.setNotificationUri(this.getContext().getContentResolver(), uri);
		}
		return c;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {

		// change modified date
		values.put( BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE, System.currentTimeMillis() );

		// update database
		SQLiteDatabase db = this.database.getWritableDatabase();
		int count;
		switch (URI_MATCHER.match(uri)) {
			case MULTIPLE_DEVICE_URI:
				count = db.update(BluetoothPrxmDeviceMetaData.TABLE_NAME, values, selection, selectionArgs);
				break;
			case SINGLE_DEVICE_URI:
				String rowId = uri.getPathSegments().get(1);
				count = db.update(BluetoothPrxmDeviceMetaData.TABLE_NAME, values ,
						BluetoothPrxmDeviceMetaData._ID + "=" + rowId + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
						selectionArgs);
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

			db.execSQL( "CREATE TABLE " + BluetoothPrxmDeviceMetaData.TABLE_NAME + " (" +
					BluetoothPrxmDeviceMetaData._ID + " INTEGER PRIMARY KEY," +
					BluetoothPrxmDeviceMetaData.DEVICE_ADDR + " VARCHAR(18)," +
					BluetoothPrxmDeviceMetaData.DEVICE_STATE + " TINYINT," +
					BluetoothPrxmDeviceMetaData.DEVICE_LINK_LOSS_LEVEL + " TINYINT," +
					BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_LEVEL + " TINYINT," +
					BluetoothPrxmDeviceMetaData.DEVICE_PATH_LOSS_THRESHOLD + " TINYINT," +
					BluetoothPrxmDeviceMetaData.DEVICE_CREATION_DATE + " INTEGER," +
					BluetoothPrxmDeviceMetaData.DEVICE_MODIFIED_DATE + " INTEGER);" );
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

			BtLog.w( "Upgrading database from version " + oldVersion + " to " + newVersion + " (will destroy all old data)!");
			db.execSQL("DROP TABLE IF EXISTS " + BluetoothPrxmDeviceMetaData.TABLE_NAME);
			this.onCreate(db);
		}
	}
}
