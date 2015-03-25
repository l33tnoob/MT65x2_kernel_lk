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

import android.content.ContentValues;
import android.database.Cursor;
import android.text.TextUtils;
import android.text.format.Time;

import com.mediatek.todos.provider.TodosDatabaseHelper.TodoColumn;

import java.io.Serializable;

/**
 * Like a record in DB.
 */
public class TodoInfo implements Serializable {
    private static final String TAG = "TodoInfo";

    public static final String STATUS_TODO = "1";
    public static final String STATUS_DONE = "2";

    private String mId = null;
    private String mTitle = "";
    private String mDescription = "";
    /** @see STATUS_TODO, STATUS_DONE */
    private String mStatus = STATUS_TODO;
    private String mDueDate = "0";
    private String mCreateTime = null;
    private String mCompleteTime = "0";

    public TodoInfo() {
    }

    public void copy(TodoInfo target) {
        if (target != null) {
            mId = target.mId;
            mTitle = target.mTitle;
            mDescription = target.mDescription;
            mStatus = target.mStatus;
            mDueDate = target.mDueDate;
            mCreateTime = target.mCreateTime;
            mCompleteTime = target.mCompleteTime;
        }
    }

    public void clear() {
        mId = null;
        mTitle = "";
        mDescription = "";
        mStatus = STATUS_TODO;
        mDueDate = "0";
        mCreateTime = null;
        mCompleteTime = "0";
    }

    /**
     * extract cursor content, to fill a new TodoInfo object, then return it.
     * 
     * @param cursor
     *            an valid cursor. if not valid, empty TodoInfo will be created
     * @return TodoInfo
     */
    public static TodoInfo makeTodoInfoFromCursor(Cursor cursor) {
        String id;
        String title;
        String description;
        String status;
        String dueDate;
        String createTime;
        String completeTime;
        TodoInfo todoInfo = new TodoInfo();

        id = Utils.getColumnByName(TodoColumn.ID, cursor);
        title = Utils.getColumnByName(TodoColumn.TITLE, cursor);
        description = Utils.getColumnByName(TodoColumn.DESCRIPTION, cursor);
        status = Utils.getColumnByName(TodoColumn.STATUS, cursor);
        dueDate = Utils.getColumnByName(TodoColumn.DTEND, cursor);
        createTime = Utils.getColumnByName(TodoColumn.CREATE_TIME, cursor);
        completeTime = Utils.getColumnByName(TodoColumn.COMPLETE_TIME, cursor);
        
        todoInfo.mId = id;
        todoInfo.mTitle = title == null ? "" : title;
        todoInfo.mDescription = description == null ? "" : description;
        todoInfo.mStatus = status;
        todoInfo.mDueDate = dueDate;
        todoInfo.mCreateTime = createTime;
        todoInfo.mCompleteTime = completeTime;
        return todoInfo;
    }
    
    public boolean equals(TodoInfo target) {
        // M: not a new Todo, compare everything to decide whether they're equal
        if (!TextUtils.equals(mId, target.mId)) {
            return false;
        }
        if (!TextUtils.equals(mTitle, target.mTitle)) {
            return false;
        }
        if (!TextUtils.equals(mDescription, target.mDescription)) {
            return false;
        }
        if (!TextUtils.equals(mDueDate, target.mDueDate)) {
            return false;
        }
        if (!TextUtils.equals(mCompleteTime, target.mCompleteTime)) {
            return false;
        }
        return true;
    }
    
    

    @Override
    public int hashCode() {
        return super.hashCode();
    }

    public ContentValues makeContentValues() {
        ContentValues values = new ContentValues();
        values.put(TodoColumn.ID, mId);
        values.put(TodoColumn.TITLE, mTitle);
        values.put(TodoColumn.COMPLETE_TIME, mCompleteTime);
        values.put(TodoColumn.CREATE_TIME, mCreateTime);
        values.put(TodoColumn.DESCRIPTION, mDescription);
        values.put(TodoColumn.DTEND, mDueDate);
        values.put(TodoColumn.STATUS, mStatus);
        return values;
    }

    String getId() {
        return mId;
    }

    String getTitle() {
        return mTitle;
    }

    void setTitle(String title) {
        this.mTitle = title;
    }

    String getDescription() {
        return mDescription;
    }

    void setDescription(String description) {
        this.mDescription = description;
    }

    String getStatus() {
        return mStatus;
    }

    void setStatus(String status) {
        this.mStatus = status;
    }

    long getDueDate() {
        return Long.parseLong(mDueDate);
    }

    void setDueDate(String dueDay) {
        this.mDueDate = dueDay;
    }

    String getCreateTime() {
        return mCreateTime;
    }

    void setCreateTime(String createTime) {
        this.mCreateTime = createTime;
    }

    long getCompleteTime() {
        return Long.parseLong(mCompleteTime);
    }

    void setCompleteTime(String completeTime) {
        this.mCompleteTime = completeTime;
    }

    /**
     * To a Todo Item, if the current time is bigger than it's due date, it's expire.
     * 
     * @return
     */
    public boolean isExpire() {
        boolean expire = false;
        if (!STATUS_DONE.equals(mStatus)) {
            if (getDueDate() <= 0) {
                expire = false;
            } else if (System.currentTimeMillis() > getDueDate()) {
                expire = true;
            }
        }
        return expire;
    }

    public boolean isComplete() {
        boolean complete = false;
        if (!STATUS_TODO.equals(mStatus)) {
            if (getCompleteTime() <= 0) {
                complete = false;
            } else {
                complete = true;
            }
        }
        return complete;
    }

    /**
     * set due day millis
     * 
     * @param dueDayMillis
     */
    void setDueDay(long dueDayMillis) {
        mDueDate = String.valueOf(dueDayMillis);
    }

    /**
     * set due day millis
     * 
     * @param dueDayMillis
     */
    void setCompleteDay(long completeDayMillis) {
        mCompleteTime = String.valueOf(completeDayMillis);
    }

    /**
     * update TodoInfo status&time.todo->done done->todo
     * 
     * @param todoInfo
     */
    public void updateStatus(String status) {
        if (STATUS_DONE.equals(status)) {
            Time completeDay = new Time();
            completeDay.setToNow();
            setStatus(STATUS_DONE);
            setCompleteDay(completeDay.normalize(true));
        } else {
            setStatus(STATUS_TODO);
            setCompleteDay(0);
        }
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("------ TodoInfo--------\n");
        sb.append("id = ").append(getId()).append(" status=").append(getStatus()).append("\n");
        sb.append("title = ").append(getTitle());
        sb.append("description = ").append(getDescription());
        return sb.toString();
    }

}
