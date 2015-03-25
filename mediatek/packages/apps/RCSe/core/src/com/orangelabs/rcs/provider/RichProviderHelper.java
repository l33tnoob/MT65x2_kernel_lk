/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.provider;

import com.orangelabs.rcs.provider.ipcall.IPCallData;
import com.orangelabs.rcs.provider.ipcall.IPCallProvider;
import com.orangelabs.rcs.provider.messaging.IntegratedMessageMappingProvider;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.provider.messaging.RichMessagingProvider;
import com.orangelabs.rcs.provider.sharing.RichCallData;
import com.orangelabs.rcs.provider.sharing.RichCallProvider;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;

import android.database.sqlite.SQLiteOpenHelper;

public class RichProviderHelper extends SQLiteOpenHelper{
	private static final String DATABASE_NAME = "eventlog.db";
	private static final int DATABASE_VERSION = 10;

	@Override
	public void onCreate(SQLiteDatabase db){
		db.execSQL("create table " + RichMessagingProvider.TABLE + " ("
				// Fields for chat
				+ RichMessagingData.KEY_ID + " integer primary key, "
				+ RichMessagingData.KEY_TYPE + " integer, "
				+ RichMessagingData.KEY_CHAT_SESSION_ID + " TEXT, "
				+ RichMessagingData.KEY_TIMESTAMP + " long, "
				+ RichMessagingData.KEY_CONTACT + " TEXT, "
				+ RichMessagingData.KEY_STATUS + " integer, "
				+ RichMessagingData.KEY_DATA + " TEXT, "
				+ RichMessagingData.KEY_MESSAGE_ID + " TEXT, "
				+ RichMessagingData.KEY_IS_SPAM + " integer, "
				+ RichMessagingData.KEY_CHAT_ID + " TEXT, "
				+ RichMessagingData.KEY_CHAT_REJOIN_ID + " TEXT, "
				
				// Fields for file transfer
				+ RichMessagingData.KEY_MIME_TYPE + " TEXT, "
				+ RichMessagingData.KEY_NAME + " TEXT, "
				+ RichMessagingData.KEY_SIZE + " long, "
				+ RichMessagingData.KEY_TOTAL_SIZE + " long, "
				
				+ RichMessagingData.KEY_NUMBER_MESSAGES+ " integer,"
				
				// fields for IMDN in chat or FT group
				+ RichMessagingData.KEY_IMDN_DELIVERED_LIST + " TEXT, "
				+ RichMessagingData.KEY_IMDN_DISPLAYED_LIST + " TEXT);"
				);
		
		db.execSQL("create table " + RichCallProvider.TABLE + " ("
				+ RichCallData.KEY_ID + " integer primary key, "
				+ RichCallData.KEY_CONTACT + " TEXT, "
				+ RichCallData.KEY_DESTINATION + " integer, "
				+ RichCallData.KEY_MIME_TYPE + " TEXT, "
				+ RichCallData.KEY_NAME + " TEXT, "
				+ RichCallData.KEY_SIZE + " long, "
				+ RichCallData.KEY_DATA + " TEXT, "
				+ RichCallData.KEY_TIMESTAMP + " long,"
				+ RichCallData.KEY_NUMBER_MESSAGES + " integer,"
				+ RichCallData.KEY_STATUS + " integer,"
				+ RichCallData.KEY_SESSION_ID+ " TEXT);"
				);
		
		db.execSQL("create table " + IPCallProvider.TABLE + " ("
				+ IPCallData.KEY_ID + " integer primary key, "
				+ IPCallData.KEY_CONTACT + " TEXT, "
				+ IPCallData.KEY_EVENT_TYPE + " integer, "
				+ IPCallData.KEY_AUDIO_MIME_TYPE + " TEXT, "
				+ IPCallData.KEY_VIDEO_MIME_TYPE + " TEXT, "
				+ IPCallData.KEY_TIMESTAMP + " long,"
				+ IPCallData.KEY_NUMBER_MESSAGES + " integer,"
				+ IPCallData.KEY_STATUS + " integer,"
				+ IPCallData.KEY_SESSION_ID+ " TEXT);"
				);
		
		/** INTEGRATED MESSAGING MAPPING TABLE*/
		db.execSQL("create table " + RichMessagingData.TABLE_MESSAGE_INTEGRATED + " ("
				+ RichMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT + " TEXT , "
				+ RichMessagingData.KEY_INTEGRATED_MODE_THREAD_ID + " long primary key ); "
				);
	}
	
	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion){
		db.execSQL("DROP TABLE IF EXISTS " + RichMessagingProvider.TABLE);
		db.execSQL("DROP TABLE IF EXISTS " + RichCallProvider.TABLE);
		db.execSQL("DROP TABLE IF EXISTS " + IPCallProvider.TABLE);
		db.execSQL("DROP TABLE IF EXISTS " + IntegratedMessageMappingProvider.TABLE);
		onCreate(db);
	}
	
	
	/**
	 * To manage an unique instance.
	 */
	private static RichProviderHelper instance = null;
	
	public static synchronized void createInstance(Context ctx) {
		if (instance == null) {
			instance = new RichProviderHelper(ctx);
		}
	}
	
	public static RichProviderHelper getInstance() {
		return instance;
	}

	private RichProviderHelper(Context context) {
		 super(context, DATABASE_NAME, null, DATABASE_VERSION);
	}
}
