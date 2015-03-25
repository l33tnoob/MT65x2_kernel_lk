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

import android.content.ContentValues;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.text.format.Time;
import android.view.View;
import android.widget.Toast;

import com.mediatek.todos.provider.TodosDatabaseHelper.TodoColumn;

public class Utils {
    private static final String TAG = "Utils";
    public static final String KEY_EDIT_TODO = "key_edit_todo";
    public static final String KEY_EDIT_RESULT = "key_edit_result";
    public static final String KEY_PASSED_DATA = "data_passed";
    public static final int REQUEST_EDIT = 0;
    public static final int EDIT_RESULT_SAVED = 0;
    public static final int EDIT_RESULT_NOT_SAVED = 1;
    public static final int RESULT_EDIT = 0;

    public static final int DATE_NO_EXPIRE = -1;
    public static final int DATE_EXPIRED = 0;
    public static final int DATE_NOT_EXPIRE = 11;

    public static final int DATE_TYPE_DUE = 1;
    public static final int DATE_TYPE_COMPLETE = 2;

    // when save data in DB,default DueDate hour,minute,second
    public static final int DUE_DATE_HOUR = 23;
    public static final int DUE_DATE_MINUTE = 59;
    public static final int DUE_DATE_SECOND = 59;

    public static final int OPERATOR_NONE = 0;
    public static final int OPERATOR_QUERY = 1;
    public static final int OPERATOR_UPDATE = 2;
    public static final int OPERATOR_INSERT = 3;
    public static final int OPERATOR_DELETE = 4;

    // for theme manager.
    public static final int COLOR_INVALID = 0;
    public static final int ALPHA_TODOS = 153;//255*0.6
    public static final float TODOS_DISMISS_ALPHA = 1f;
    public static final String LIST_HEADER_BG_COLOR = "list_header_bg_color";

    private static Toast sToast = null;
    /**
     * function of how to write data to DB.
     * 
     * @param todoInfo
     * @param asyncQuery
     */
    private static void syncWithDB(QueryListener listener, TodoInfo todoInfo, AsyncQueryHandler asyncQuery,
            int operatorCode) {
        String idColumnName = TodoColumn.ID;
        ContentValues values = todoInfo.makeContentValues();
        String selection;

        switch (operatorCode) {
        case Utils.OPERATOR_UPDATE:
            selection = idColumnName + "=" + todoInfo.getId();
            asyncQuery.startUpdate(0, listener, TodoAsyncQuery.TODO_URI, values, selection, null);
            break;

        case Utils.OPERATOR_INSERT:
            asyncQuery.startInsert(0, listener, TodoAsyncQuery.TODO_URI, values);
            break;

        case Utils.OPERATOR_DELETE:
            selection = idColumnName + "=" + todoInfo.getId();
            asyncQuery.startDelete(0, listener, TodoAsyncQuery.TODO_URI, selection, null);
            break;
        case Utils.OPERATOR_NONE:
            LogUtils.d(TAG, "Unexpected token NONE");
            break;

        default:
            LogUtils.e(TAG, "Unexpected token of pending db operation");
            break;
        }
        asyncQuery = null;
    }

    /**
     * Write data to DB.
     */
    public static void writeAdapterDataToDB(TodoInfo data, AsyncQueryHandler asyncQuery,
            int operatorCode) {
        syncWithDB(null, data, asyncQuery, operatorCode);
    }

    /**
     * Write data to DB.
     */
    public static void writeAdapterDataToDB(QueryListener listener,TodoInfo data, AsyncQueryHandler asyncQuery,
            int operatorCode) {
        syncWithDB(listener, data, asyncQuery, operatorCode);
    }

    /**
     * return column value by name.
     * 
     * @param name
     * @param cursor
     * @return
     */
    public static String getColumnByName(String name, Cursor cursor) {
        int index = cursor.getColumnIndex(name);
        if (index == -1) {
            LogUtils.e(TAG, "Column Name " + name + "is not found");
            return null;
        }
        String content;
        if (name.equals(TodoColumn.ID)) {
            content = "" + cursor.getInt(index);
            return content;
        }
        content = cursor.getString(index);
        return content;
    }

    /**
     * @param context
     * @param promptStringId
     */
    public static void prompt(Context context, int promptStringId) {
        if (sToast == null) {
            sToast = Toast.makeText(context, promptStringId, Toast.LENGTH_SHORT);
        } else {
            sToast.setText(promptStringId);
        }
        sToast.show();
    }

    /**
     * 
     * @param context
     * @param dateMillis
     * @param type
     *            1wei
     * @return
     */
    public static String getDateText(Context context, long dateMillis, int type) {
        String timeFormat = null;

        if (type == DATE_TYPE_DUE) { // get dueDate
            if (dateMillis <= 0) {
                return context.getResources().getString(R.string.no_expire_date);
            }
            timeFormat = context.getResources().getString(R.string.due_day_time_format);
        } else if (type == DATE_TYPE_COMPLETE) { // get complete date
            if (dateMillis <= 0) {
                return null;
            }
            timeFormat = context.getResources().getString(R.string.done_time_format);
        } else { // no thhis type return null.
            LogUtils.w(TAG, "format date failed.No this type:" + type);
            return null;
        }

        Time time = new Time();
        time.set(dateMillis);
        return time.format(timeFormat);
    }

    /**
     * Compare two time's year/month/day, return the result
     * 
     * @param time1
     * @param time2
     * @return if time1 is earlier than time2, then return -1, if later, 1. if
     *         they're in the same day, then return 0
     */
    public static int dateCompare(Time time1, Time time2) {
        if (time1.year < time2.year) {
            return -1;
        } else if (time1.year > time2.year) {
            return 1;
            // /M: same year
        } else if (time1.yearDay < time2.yearDay) {
            return -1;
        } else if (time1.yearDay > time2.yearDay) {
            return 1;
        }
        return 0;
    }

    /**
     * update the view's background,it will be effected by the Theme Manager.
     * If it was not selected,there will be no background.
     * This function is normal used for the view that can be selected,list list item.
     * @param context
     * @param target the view that to be update
     * @param detaultDrawableId
     * @param selected whether the target view was selected
     */
     public static void updateViewBackgroud(Context context, View target, int detaultDrawableId, boolean selected) {
         // if the view was not selected,remove the background color
        if (!selected) {
            target.setAlpha(TODOS_DISMISS_ALPHA);
            target.setBackgroundColor(COLOR_INVALID);
            return;
        }
        int color = getThemeColor(context);
        if (color == COLOR_INVALID) {
            Drawable background = context.getResources().getDrawable(detaultDrawableId);
            target.setBackgroundDrawable(background);
        } else {
            target.setBackgroundColor(color);
            target.getBackground().setAlpha(ALPHA_TODOS);
        }
    }

     /**
      * update the view's background,it will be effected by the Theme Manager.
      * @param context
      * @param target
      * @param detaultColor
      */
    public static void updateViewBackgroud(Context context, View target, int detaultColor) {
        int color = getThemeColor(context);
        if (color == COLOR_INVALID) {
            target.setBackgroundColor(detaultColor);
        } else {
            target.setBackgroundColor(color);
        }
        // target.setAlpha(TODOS_ALPHA);
    }

    private static int getThemeColor(Context context) {
        // get the theme color
        int color = COLOR_INVALID;
        /*if (FeatureOption.MTK_THEMEMANAGER_APP) {
            Resources res = context.getResources();
            color = res.getThemeMainColor();
        }*/
        return color;
    }
}