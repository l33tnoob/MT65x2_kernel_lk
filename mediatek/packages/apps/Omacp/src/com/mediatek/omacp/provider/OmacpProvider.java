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

package com.mediatek.omacp.provider;

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

import com.mediatek.omacp.utils.MTKlog;

public class OmacpProvider extends ContentProvider {

    private static final String TAG = "Omacp/OmacpProvider";

    static final String TABLE_OMACP = "omacp";

    private static final Uri NOTIFICATION_URI = Uri.parse("content://omacp");

    private OmacpDatabaseHelper mOmacpDatabaseHelper;

    private static final int OMACP_ALL = 0;

    private static final int OMACP_ID = 1;

    private static final UriMatcher URIMATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        URIMATCHER.addURI("omacp", null, OMACP_ALL);
        URIMATCHER.addURI("omacp", "#", OMACP_ID);
    }

    private static final String VND_ANDROID_ITEM_OMACP = "vnd.android.cursor.item/omacp";

    private static final String VND_ANDROID_DIR_OMACP = "vnd.android.cursor.dir/omacp";

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        // TODO Auto-generated method stub
        SQLiteDatabase db = mOmacpDatabaseHelper.getWritableDatabase();
        int count = 0;

        switch (URIMATCHER.match(uri)) {
            case OMACP_ALL:
                count = db.delete(TABLE_OMACP, selection, selectionArgs);
                break;
            case OMACP_ID:
                int messageId;
                try {
                    messageId = Integer.parseInt(uri.getPathSegments().get(0));
                } catch (NumberFormatException e) {
                    MTKlog.e(TAG, "OmacpProvider Delete: Bad Message ID");
                    return 0;
                }
                // Delete the message and update the thread
                count = db.delete(TABLE_OMACP, "_id=" + messageId, null);
                break;
            default:
                MTKlog.e(TAG, "OmacpProvider Unknown URI: " + uri);
                return 0;
        }

        if (count > 0) {
            ContentResolver cr = getContext().getContentResolver();
            cr.notifyChange(uri, null);
        }
        return count;
    }

    @Override
    public String getType(Uri uri) {
        if (URIMATCHER.match(uri) == OMACP_ALL) {
            return VND_ANDROID_DIR_OMACP;
        } else if (URIMATCHER.match(uri) == OMACP_ID) {
            return VND_ANDROID_ITEM_OMACP;
        }
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        // TODO Auto-generated method stub
        if (values == null) {
            return null;
        }
        if (URIMATCHER.match(uri) == OMACP_ALL) {
            SQLiteDatabase db = mOmacpDatabaseHelper.getWritableDatabase();
            long rowId = db.insert(TABLE_OMACP, null, values);
            if (rowId > 0) {
                Uri insertUri = ContentUris
                        .withAppendedId(OmacpProviderDatabase.CONTENT_URI, rowId);
                ContentResolver cr = getContext().getContentResolver();
                cr.notifyChange(uri, null);
                return insertUri;
            } else {
                MTKlog.e(TAG, "OmacpProvider Failed to insert! " + values.toString());
            }
        } else {
            MTKlog.e(TAG, "OmacpProvider insert Unknown URI: " + uri);
        }
        return null;
    }

    @Override
    public boolean onCreate() {
        // TODO Auto-generated method stub
        mOmacpDatabaseHelper = new OmacpDatabaseHelper(this.getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        // TODO Auto-generated method stub
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE_OMACP);

        switch (URIMATCHER.match(uri)) {
            case OMACP_ALL:
                break;
            case OMACP_ID:
                qb.appendWhere(OmacpProviderDatabase._ID + "=" + uri.getPathSegments().get(0));
                break;
            default:
                MTKlog.e(TAG, "OmacpProvider query Unknown URI: " + uri);
                return null;
        }

        // If no sort order is specified use the default
        String orderBy;
        if (TextUtils.isEmpty(sortOrder)) {
            orderBy = OmacpProviderDatabase.DEFAULT_SORT_ORDER;
        } else {
            orderBy = sortOrder;
        }

        // Get the database and run the query
        SQLiteDatabase db = mOmacpDatabaseHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);

        // Tell the cursor what uri to watch, so it knows when its source data
        // changes
        c.setNotificationUri(getContext().getContentResolver(), NOTIFICATION_URI);
        return c;

    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        // TODO Auto-generated method stub
        SQLiteDatabase db = mOmacpDatabaseHelper.getWritableDatabase();
        int count = 0;

        switch (URIMATCHER.match(uri)) {
            case OMACP_ALL:
                count = db.update(TABLE_OMACP, values, selection, selectionArgs);
                break;
            case OMACP_ID:
                String newIdSelection = OmacpProviderDatabase._ID + "="
                        + uri.getPathSegments().get(0)
                        + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : "");
                count = db.update(TABLE_OMACP, values, newIdSelection, selectionArgs);
                break;
            default:
                MTKlog.e(TAG, "OmacpProvider update Unknown URI: " + uri);
                return count;
        }

        if (count > 0) {
            ContentResolver cr = getContext().getContentResolver();
            cr.notifyChange(uri, null);
        }
        return count;
    }
}
