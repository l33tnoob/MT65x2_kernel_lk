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

package com.mediatek.todos;

import android.content.AsyncQueryHandler;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.todos.provider.TodosDatabaseHelper.TodoColumn;

import java.util.HashMap;

/*
 * Note:When use this class,you must set CursorAdapter first.
 */
public class TodoAsyncQuery extends AsyncQueryHandler {
    private static final String TAG = "AsyncQuery";
    public static final Uri TODO_URI = Uri.parse("content://com.mediatek.todos/todos");
    public static final int TOKEN_TODO = 1;
    public static final int TOKEN_DONE = 2;
    public static final int TOKEN_INSERT_COMPLETE = 3;
    public static final int TOKEN_DELETE_COMPLETE = 4;
    public static final int TOKEN_UPDATE_COMPLETE = 5;
    public static final int TOKEN_QUERY_COMPLETE = 6;
    public static final String TODO_SELECTION = TodoColumn.STATUS + "= 1";
    public static final String DONE_SELECTION = TodoColumn.STATUS + "= 0";

    private static HashMap<Context, TodoAsyncQuery> sInstance = new HashMap<Context, TodoAsyncQuery>();

    /**
     * It suggest to use ApplicationContext to create a TodoAsyncQuery. If the context is not
     * ApplicaitonContext, it must be freed if the context is not used.
     * 
     * @param context
     * @return
     */
    public static TodoAsyncQuery getInstatnce(Context context) {
        if (!sInstance.containsKey(context)) {
            TodoAsyncQuery newQuery = new TodoAsyncQuery(context);
            sInstance.put(context, newQuery);
        }
        return sInstance.get(context);
    }

    /**
     * Remove the context if we don't need it.
     * 
     * @param context
     */
    public static void free(Context context) {
        if (sInstance.containsKey(context)) {
            sInstance.remove(context);
        }
    }

    /**
     * Free all contexts.
     */
    public static void freeAll() {
        sInstance.clear();
    }

    private TodoAsyncQuery(Context context) {
        super(context.getContentResolver());
    }

    @Override
    protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
        LogUtils.d(TAG, "onQueryComplete token=" + token);
        if (cookie instanceof QueryListener) {
            QueryListener listener = (QueryListener) cookie;
            listener.onQueryComplete(token, cursor);
        } else {
            cursor.close();
            LogUtils.e(TAG, "cookie is another object: " + cookie);
        }
    }

    /**
     * @param cookie
     *            . Need pass a int[] to point requery adapters.
     */
    @Override
    protected void onDeleteComplete(int token, Object cookie, int result) {
        LogUtils.d(TAG, "onDeleteComplete token=" + token + " result="  + result);
        if (cookie instanceof QueryListener) {
            QueryListener listener = (QueryListener) cookie;
            listener.onDeleteComplete(token, result);
        }
    }

    /*
     * @param cookie. Need pass a int[] to point requery adapters.
     */
    @Override
    protected void onUpdateComplete(int token, Object cookie, int result) {
        LogUtils.d(TAG, "onUpdateComplete token=" + token + " result=" + result);
        if (cookie instanceof QueryListener) {
            QueryListener listener = (QueryListener) cookie;
            listener.onUpdateComplete(token, result);
        }
    }

    /*
     * @param cookie. Need pass a int[] to point requery adapters.
     */
    @Override
    protected void onInsertComplete(int token, Object cookie, Uri uri) {
        LogUtils.d(TAG, "onInsertComplete token=" + token + " uri=" + uri);
        if (cookie instanceof QueryListener) {
            QueryListener listener = (QueryListener) cookie;
            listener.onInsertComplete(token, uri);
        }
    }
}
