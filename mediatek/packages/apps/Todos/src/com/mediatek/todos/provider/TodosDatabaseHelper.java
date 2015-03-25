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
 * Copyright (C) 2009 The Android Open Source Project
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
 * limitations under the License
 */

package com.mediatek.todos.provider;

import android.content.ContentValues;

import android.content.Context;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;


/**
 * Database helper for calendar. Designed as a singleton to make sure that all
 * {@link android.content.ContentProvider} users get the same reference.
 */
public /* package */ class TodosDatabaseHelper extends SQLiteOpenHelper {

    private static final String TAG = "TodosDatabaseHelper";

    private static final boolean LOGD = false;

    private static final String DATABASE_NAME = "todos.db";

    private static final int DAY_IN_SECONDS = 24 * 60 * 60;

    // Note: if you update the version number, you must also update the code
    // in upgradeDatabase() to modify the database (gracefully, if possible).
    // Versions under 100 cover through Froyo, 1xx version are for Gingerbread,
    // 2xx for Honeycomb, and 3xx for ICS. For future versions bump this to the
    // next hundred at each major release.
    static final int DATABASE_VERSION = 308;
    
    private static TodosDatabaseHelper sSingleton = null;
    //private final SyncStateContentProviderHelper mSyncState;
    private DatabaseUtils.InsertHelper mTodosInserter;

    public long todosInsert(ContentValues values) {
        return mTodosInserter.insert(values);
    }
    
    public static synchronized TodosDatabaseHelper getInstance(Context context) {
        if (sSingleton == null) {
            sSingleton = new TodosDatabaseHelper(context);
        }
        return sSingleton;
    }

    /**
     * Private constructor, callers except unit tests should obtain an instance through
     * {@link #getInstance(android.content.Context)} instead.
     */
    TodosDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        if (LOGD) {
            Log.d(TAG, "Creating OpenHelper");
        }
        // mSyncState = new SyncStateContentProviderHelper();
    }

    @Override
    public void onOpen(SQLiteDatabase db) {
        //mSyncState.onDatabaseOpened(db);

        mTodosInserter = new DatabaseUtils.InsertHelper(db, Tables.TODOS);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        bootstrapDB(db);
    }

    private void bootstrapDB(SQLiteDatabase db) {
        Log.i(TAG, "Bootstrapping database");

        //mSyncState.createDatabase(db);

        createTodoTable(db);
    }
    
    public interface Tables {
        String TODOS = "Todos";
    }

    public interface TodoColumn {
        String ID = "_id";
        String TITLE = "title";
        String DESCRIPTION = "description";
        String PRIORITY = "priority";
        String STATUS = "status";
        String DTEND = "dtend";
        String CREATE_TIME = "create_time";
        String COMPLETE_TIME = "complete_time";
    }
    
    private void createTodoTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + Tables.TODOS + " (" +
                TodoColumn.ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                TodoColumn.TITLE + " TEXT," +
                TodoColumn.DESCRIPTION + " TEXT," +
                TodoColumn.PRIORITY + " INTEGER," +
                TodoColumn.STATUS + " INTEGER NOT NULL DEFAULT 0," +
                // dtend in millis since epoch
                TodoColumn.DTEND + " INTEGER NOT NULL DEFAULT 0," +
                // create time in millis
                TodoColumn.CREATE_TIME + " INTEGER," +
                // complete time in millis
                TodoColumn.COMPLETE_TIME + " INTEGER" +
                ");");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // tmp ignore.
    }

//    public SyncStateContentProviderHelper getSyncState() {
//        return mSyncState;
//    }
}
