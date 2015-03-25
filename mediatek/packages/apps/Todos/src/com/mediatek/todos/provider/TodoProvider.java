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

package com.mediatek.todos.provider;

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.provider.BaseColumns;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;
import android.text.TextUtils;

import com.mediatek.todos.provider.TodosDatabaseHelper.Tables;
import com.mediatek.todos.provider.TodosDatabaseHelper.TodoColumn;

import java.util.HashMap;

public class TodoProvider extends ContentProvider {
    protected static final String TAG = "TodoProvider";
    protected static final String AUTHORITY = "com.mediatek.todos";
    private TodosDatabaseHelper mDbHelper;
    protected SQLiteDatabase mDb;
    private Context mContext;
    private ContentResolver mContentResolver;
    
    private static final int TODOS = 33;
    private static final int TODOS_ID = 34;
    private static final UriMatcher URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    private static final HashMap<String, String> TODO_PROJECTTION_MAP;
    private static final HashMap<String, String> COUNT_PROJECTION_MAP;
    static {
        URI_MATCHER.addURI(AUTHORITY, "todos", TODOS);
        URI_MATCHER.addURI(AUTHORITY, "todos/#", TODOS_ID);
        TODO_PROJECTTION_MAP = new HashMap<String, String>();
        TODO_PROJECTTION_MAP.put(TodoColumn.ID, TodoColumn.ID);
        TODO_PROJECTTION_MAP.put(TodoColumn.TITLE, TodoColumn.TITLE);
        TODO_PROJECTTION_MAP.put(TodoColumn.DESCRIPTION, TodoColumn.DESCRIPTION);
        TODO_PROJECTTION_MAP.put(TodoColumn.PRIORITY, TodoColumn.PRIORITY);
        TODO_PROJECTTION_MAP.put(TodoColumn.STATUS, TodoColumn.STATUS);
        TODO_PROJECTTION_MAP.put(TodoColumn.DTEND, TodoColumn.DTEND);
        TODO_PROJECTTION_MAP.put(TodoColumn.CREATE_TIME, TodoColumn.CREATE_TIME);
        TODO_PROJECTTION_MAP.put(TodoColumn.COMPLETE_TIME, TodoColumn.COMPLETE_TIME);
        
        /** Contains just BaseColumns._COUNT */
        COUNT_PROJECTION_MAP = new HashMap<String, String>();
        COUNT_PROJECTION_MAP.put(BaseColumns._COUNT, "COUNT(*)");
    }
    
    private static final String GENERIC_ID = "_id";
    protected static final String SQL_WHERE_ID = GENERIC_ID + "=?";

    @Override
    public boolean onCreate() {
        return initialize();
    }

    private boolean initialize() {
        mContext = getContext();
        mContentResolver = mContext.getContentResolver();

        mDbHelper = (TodosDatabaseHelper)getDatabaseHelper(mContext);
        mDb = mDbHelper.getWritableDatabase();

        return true;
    }
    
    protected TodosDatabaseHelper getDatabaseHelper(final Context context) {
        return TodosDatabaseHelper.getInstance(context);
    }

    @Override
    public String getType(Uri uri) {
        int match = URI_MATCHER.match(uri);
        switch (match) {
            case TODOS:
                return "vnd.android.cursor.dir/todo";
            case TODOS_ID:
                return "vnd.android.cursor.item/todo";
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        final int match = URI_MATCHER.match(uri);
        long id = 0;
        switch (match) {
            case TODOS:
                id = mDbHelper.todosInsert(values);
                break;
            case TODOS_ID:
                throw new UnsupportedOperationException("Cannot insert into that URL: " + uri);
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
        
        if (id < 0) {
            return null;
        }

        return ContentUris.withAppendedId(uri, id);
    }
    
    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        final int match = URI_MATCHER.match(uri);
        switch (match) {
            case TODOS:
                //TODO:check the result.
                return mDb.delete(Tables.TODOS, selection, selectionArgs);
            case TODOS_ID:
                long id = ContentUris.parseId(uri);
                return mDb.delete(Tables.TODOS, SQL_WHERE_ID, new String[] {String.valueOf(id)});
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        String groupBy = null;
        String limit = null; // Not currently implemented
        final SQLiteDatabase db = mDbHelper.getReadableDatabase();
    
        final int match = URI_MATCHER.match(uri);
        switch (match) {
            case TODOS:
                //TODO:check the result.
                qb.setTables(Tables.TODOS);
                qb.setProjectionMap(TODO_PROJECTTION_MAP);
                selection = appendAccountFromParameterToSelection(selection, uri);
                break;
            case TODOS_ID:
                qb.setTables(Tables.TODOS);
                qb.setProjectionMap(TODO_PROJECTTION_MAP);
                selectionArgs = insertSelectionArg(selectionArgs, uri.getPathSegments().get(1));
                qb.appendWhere(SQL_WHERE_ID);
                break;
            default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
        // run the query
        return query(db, qb, projection, selection, selectionArgs, sortOrder, groupBy, limit);
    }
    
    private Cursor query(final SQLiteDatabase db, SQLiteQueryBuilder qb, String[] projection,
            String selection, String[] selectionArgs, String sortOrder, String groupBy,
            String limit) {

        if (projection != null && projection.length == 1
                && BaseColumns._COUNT.equals(projection[0])) {
            qb.setProjectionMap(COUNT_PROJECTION_MAP);
        }

        final Cursor c = qb.query(db, projection, selection, selectionArgs, groupBy, null,
                sortOrder, limit);
        if (c != null) {
            // TODO: is this the right notification Uri?
            c.setNotificationUri(mContentResolver, CalendarContract.Events.CONTENT_URI);
        }
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        final int match = URI_MATCHER.match(uri);
        switch (match) {
        case TODOS:
            // TODO:check the result.
            return mDb.update(Tables.TODOS, values, selection, selectionArgs);
        case TODOS_ID:
            long id = ContentUris.parseId(uri);
            return mDb.update(Tables.TODOS, values, SQL_WHERE_ID, new String[] { String.valueOf(id) });
        default:
                throw new IllegalArgumentException("Unknown URL " + uri);
        }
    }
    
    //=====================
    //Local function
    //=====================
    
    private String appendAccountFromParameterToSelection(String selection, Uri uri) {
        final String accountName = QueryParameterUtils.getQueryParameter(uri,
                CalendarContract.EventsEntity.ACCOUNT_NAME);
        final String accountType = QueryParameterUtils.getQueryParameter(uri,
                CalendarContract.EventsEntity.ACCOUNT_TYPE);
        if (!TextUtils.isEmpty(accountName) && !TextUtils.isEmpty(accountType)) {
            final StringBuilder sb = new StringBuilder();
            sb.append(Calendars.ACCOUNT_NAME + "=")
                    .append(DatabaseUtils.sqlEscapeString(accountName))
                    .append(" AND ")
                    .append(Calendars.ACCOUNT_TYPE)
                    .append(" = ")
                    .append(DatabaseUtils.sqlEscapeString(accountType));
            return appendSelection(sb, selection);
        } else {
            return selection;
        }
    }
    
    /**
     * Inserts an argument at the beginning of the selection arg list.
     *
     * The {@link android.database.sqlite.SQLiteQueryBuilder}'s where clause is
     * prepended to the user's where clause (combined with 'AND') to generate
     * the final where close, so arguments associated with the QueryBuilder are
     * prepended before any user selection args to keep them in the right order.
     */
    private String[] insertSelectionArg(String[] selectionArgs, String arg) {
        if (selectionArgs == null) {
            return new String[] {arg};
        } else {
            int newLength = selectionArgs.length + 1;
            String[] newSelectionArgs = new String[newLength];
            newSelectionArgs[0] = arg;
            System.arraycopy(selectionArgs, 0, newSelectionArgs, 1, selectionArgs.length);
            return newSelectionArgs;
        }
    }
    
    private String appendSelection(StringBuilder sb, String selection) {
        if (!TextUtils.isEmpty(selection)) {
            sb.append(" AND (");
            sb.append(selection);
            sb.append(')');
        }
        return sb.toString();
    }

}
