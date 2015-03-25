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
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.todos.TodosListAdapter.ViewHolder;
import com.mediatek.todos.provider.TodosDatabaseHelper.TodoColumn;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class TodosListAdapter extends BaseAdapter implements QueryListener,
        View.OnClickListener, TimeChangeReceiver.TimeChangeListener {
    private static final String TAG = "TodosListAdapter";
    private static final String EMPTY_TEXT = "";

    public static final int TYPE_TODOS_HEADER = 0;
    public static final int TYPE_TODOS_ITEM = TYPE_TODOS_HEADER + 1;
    public static final int TYPE_TODOS_FOOTER = TYPE_TODOS_ITEM + 1;
    public static final int TYPE_DONES_HEADER = TYPE_TODOS_FOOTER + 1;
    public static final int TYPE_DONES_ITEM = TYPE_DONES_HEADER + 1;
    public static final int TYPE_DONES_FOOTER = TYPE_DONES_ITEM + 1;

    public static final int EDIT_NULL = 0;
    public static final int EDIT_TODOS = 1;
    public static final int EDIT_DONES = EDIT_TODOS << 1;
    
    private static final float ALPHA_DONE = 0.6f;
    private static final float ALPHA_TODO = 1.0f;
    private static final float ALPHA_DISABLE = 0.4f;
    private static final float ALPHA_ENABLE = 1.0f;
    private static final int ALPHA_TRANSPARENT = 0;
    private static final int ALPHA_OPAQUE = 255;

    private static int BATCH_OPER_MAX_NUM = 100;

    private Comparator<TodoInfo> mTodoComparator = new Comparator<TodoInfo>() {
        public int compare(TodoInfo info1, TodoInfo info2) {
            final long dueDate1 = info1.getDueDate();
            final long dueDate2 = info2.getDueDate();
            int result = 0;
            if (dueDate1 == dueDate2) {
                result = 0;
            } else if (dueDate2 == 0 || (dueDate1 != 0 && dueDate2 > dueDate1)) {
                result = -1;
            } else {
                result = 1;
            }
            return result;
        }
    };

    private Comparator<TodoInfo> mDoneComparator = new Comparator<TodoInfo>() {
        public int compare(TodoInfo info1, TodoInfo info2) {
            final long compTime1 = info1.getCompleteTime();
            final long compTime2 = info2.getCompleteTime();
            int result = 0;
            if (compTime1 < compTime2) {
                result = 1;
            } else if (compTime1 > compTime2) {
                result = -1;
            }
            return result;
        }
    };

    // the data display in list.
    private ArrayList<TodoInfo> mTodosDataSource = new ArrayList<TodoInfo>();
    // the data display in list.
    private ArrayList<TodoInfo> mDonesDataSource = new ArrayList<TodoInfo>();

    // list check box status.
    private SparseBooleanArray mTodosCheckStates = new SparseBooleanArray();
    // list check box status.
    private SparseBooleanArray mDonesCheckStates = new SparseBooleanArray();

    private Context mContext = null;
    private LayoutInflater mInflater = null;
    private TodoAsyncQuery mAsyncQuery = null;

    private View mTodosHeaderView = null;
    private View mTodosFooterView = null;
    private View mDonesHeaderView = null;
    private View mDonesFooterView = null;

    private int mEditType = EDIT_NULL;
    private boolean mTodosExpand = true;
    private boolean mDonesExpand = true;

    // assume the Adapter should loading items form DB first.
    private boolean mIsLoadingData = false;
    // True means that mTodosDataSource | mDonesDataSource needs to update data from DB.
    private boolean mIsDataDirty = false;

    public TodosListAdapter(Context context) {
        mContext = context;
        mAsyncQuery = TodoAsyncQuery.getInstatnce(context.getApplicationContext());
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        // start query form DB.
        startQuery(null);
    }

    public boolean setEditingType(int flag) {
        if ((flag == EDIT_TODOS && mTodosDataSource.isEmpty())
                || (flag == EDIT_DONES && mDonesDataSource.isEmpty())) {
            return false;
        }
        if (mEditType != flag) {
            mEditType = flag;
            mTodosCheckStates.clear();
            mDonesCheckStates.clear();
            notifyDataSetChanged();
        }
        return true;
    }

    public int getEditType() {
        return mEditType;
    }

    public boolean isEditing() {
        return mEditType == EDIT_TODOS || mEditType == EDIT_DONES;
    }

    public void setTodosExpand(boolean expand) {
        if (mTodosExpand != expand) {
            mTodosExpand = expand;
            notifyDataSetChanged();
        }
    }

    public void setDonesExpand(boolean expand) {
        if (mDonesExpand != expand) {
            mDonesExpand = expand;
            notifyDataSetChanged();
        }
    }

    public boolean isTodosExpand() {
        return mTodosExpand;
    }

    public boolean isDonesExPand() {
        return mDonesExpand;
    }

    public ArrayList<TodoInfo> getTodosDataSource() {
        return mTodosDataSource;
    }

    public ArrayList<TodoInfo> getDonesDataSource() {
        return mDonesDataSource;
    }

    public int getCount() {
        int count = 2; // Todos' Header, Dones' Header
        if (mTodosDataSource.isEmpty()) {
            count++;
        } else {
            count += (mTodosExpand ? mTodosDataSource.size() : 0);
        }

        if (mDonesDataSource.isEmpty()) {
            count++;
        } else {
            count += (mDonesExpand ? mDonesDataSource.size() : 0);
        }
        return count;
    }

    public TodoInfo getItem(int position) {
        if (mTodosExpand) {
            int index = position - 1;
            if (index < 0) {
                return null;
            } else if (index < mTodosDataSource.size()) {
                return mTodosDataSource.get(index);
            }
        }
        if (mDonesExpand) {
            int index = 0;
            if (mTodosDataSource.isEmpty()) {
                index = position - 3;
            } else {
                if (mTodosExpand) {
                    index = position - 2 - mTodosDataSource.size();
                } else {
                    index = position - 2;
                }
            }
            if (index >= 0 && index < mDonesDataSource.size()) {
                return mDonesDataSource.get(index);
            }
        }
        return null;
    }

    public long getItemId(int position) {
        return position;
    }

    /**
     * Add a TodoInfo for View. The info should be already stored in DB. 
     * @param info
     */
    public void addItem(TodoInfo info) {
        if (info == null || info.getId() == null || mIsLoadingData) {
            LogUtils.e(TAG, "addItem() failed. mFinishLoadingItems=" + mIsLoadingData + " Or Ill-legeal TodoInfo.");
            return;
        }
        int index = -1;
        if (TodoInfo.STATUS_DONE.equals(info.getStatus())) {
            if (info.getCompleteTime() != 0) {
                index = reallyAddItem(TodoInfo.STATUS_DONE, info);
            }
            if (index == -1) {
                mDonesDataSource.add(info);
                index = mDonesDataSource.size() - 1;
            }
            updateCheckBoxValue(mEditType, index);
        } else {
            if (info.getDueDate() != 0) {
                index = reallyAddItem(TodoInfo.STATUS_TODO, info);
            }
            if (index == -1) {
                mTodosDataSource.add(info);
                index = mTodosDataSource.size() - 1;
            }
            updateCheckBoxValue(mEditType, index);
        }
        notifyDataSetChanged();
        updateHeaderNumberText();
    }
    
    /**
     * update checkbox's value.
     * @param type
     * @param index
     */
    private void updateCheckBoxValue(int type, int index) {
        SparseBooleanArray temp = null;
        int itemNum = 0;
        if (type == EDIT_DONES) {
            temp = mDonesCheckStates;
            itemNum = mDonesDataSource.size();
        } else if (type == EDIT_TODOS) {
            temp = mTodosCheckStates;
            itemNum = mTodosDataSource.size();
        }
        if (temp == null) {
            return;
        }

        for (int i = itemNum - 1; i >= index; i--) {
            if (temp.get(i)) {
                temp.put(i + 1, true);
                temp.delete(i);
            }
        }
    }
    
    /**
     * add TodoInfo item to the todo/done list
     * @param status
     * @param info
     * @return the index at which to insert the object.
     */
    private int reallyAddItem(String status,TodoInfo info) {
        TodoInfo temp = null;
        int index = -1;
        
        if (TodoInfo.STATUS_DONE.equals(status)) {
            int doneSize = mDonesDataSource.size();
            for (int i = 0; i < doneSize; i++) {
                temp = mDonesDataSource.get(i);
                if (info.getCompleteTime() > temp.getCompleteTime()) {
                    index = i;
                    mDonesDataSource.add(index, info);
                    break;
                }
            }
        } else {
            int todoSize = mTodosDataSource.size();
            for (int i = 0; i < todoSize; i++) {
                temp = mTodosDataSource.get(i);
                if (info.getDueDate() < temp.getDueDate() || temp.getDueDate() == 0) {
                    index = i;
                    mTodosDataSource.add(index, info);
                    break;
                }
            }
        }
        return index;
    }

    /**
     * The item already deleted from DB, remove the item form ListView.
     * 
     * @param info
     */
    public void removeItem(TodoInfo info) {
        final int countTodos = mTodosDataSource.size();
        for (int i = 0; i < countTodos; i++) {
            TodoInfo item = mTodosDataSource.get(i);
            if (item.getId().equals(info.getId())) {
                LogUtils.d(TAG, "removeItem remove a To-do item Pos=" + i);
                mTodosDataSource.remove(i);
                notifyDataSetChanged();
                updateHeaderNumberText();
                return;
            }
        }
        final int countDones = mDonesDataSource.size();
        for (int i = 0; i < countDones; i++) {
            TodoInfo item = mDonesDataSource.get(i);
            if (item.getId().equals(info.getId())) {
                LogUtils.d(TAG, "removeItem remove a Done item Pos=" + i);
                mDonesDataSource.remove(i);
                notifyDataSetChanged();
                updateHeaderNumberText();
                return;
            }
        }
        LogUtils.w(TAG, "removeItem failed. not find TodoInfo :" + info);
    }

    public void updateItemData(TodoInfo info) {
        if (mIsLoadingData) {
            LogUtils.w(TAG, "updateItemData failed for Adapter is loading data from DB.");
            return;
        }
        final int countTodos = mTodosDataSource.size();
        for (int i = 0; i < countTodos; i++) {
            TodoInfo item = mTodosDataSource.get(i);
            if (item.getId().equals(info.getId())) {
                if (item.getStatus().equals(info.getStatus())) {
                    if (item.getDueDate() != info.getDueDate() && countTodos > 1) {
                        mTodosDataSource.set(i, info);
                        Collections.sort(mTodosDataSource, mTodoComparator);
                    } else {
                        mTodosDataSource.set(i, info);
                    }
                } else { // info's status changed to Done.
                    mTodosDataSource.remove(i);
                    mDonesDataSource.add(0, info);
                    if (mDonesDataSource.size() > 1) {
                        Collections.sort(mDonesDataSource, mDoneComparator);
                    }
                }
                notifyDataSetChanged();
                updateHeaderNumberText();
                return;
            }
        }
        final int countDones = mDonesDataSource.size();
        for (int i = 0; i < countDones; i++) {
            TodoInfo item = mDonesDataSource.get(i);
            if (item.getId().equals(info.getId())) {
                if (item.getStatus().equals(info.getStatus())) {
                    if (item.getDueDate() != info.getDueDate() && countDones > 1) {
                        mDonesDataSource.set(i, info);
                        Collections.sort(mDonesDataSource, mDoneComparator);
                    } else {
                        mDonesDataSource.set(i, info);
                    }
                } else { // info's status changed to ToDo.
                    mDonesDataSource.remove(i);
                    mTodosDataSource.add(0, info);
                    if (mTodosDataSource.size() > 1) {
                        Collections.sort(mTodosDataSource, mTodoComparator);
                    }
                }
                notifyDataSetChanged();
                updateHeaderNumberText();
                return;
            }
        }
        LogUtils.w(TAG, "updateItemData failed. not find TodoInfo :" + info);
    }

    public int getSeletedTodosNumber() {
        int count = 0;
        if (mEditType == EDIT_TODOS) {
            final int total = mTodosDataSource.size();
            for (int i = 0; i < total; i++) {
                if (mTodosCheckStates.get(i)) {
                    count++;
                }
            }
        }
        return count;
    }

    public int getSeletedDonesNumber() {
        int count = 0;
        if (mEditType == EDIT_DONES) {
            final int total = mDonesDataSource.size();
            for (int i = 0; i < total; i++) {
                if (mDonesCheckStates.get(i)) {
                    count++;
                }
            }
        }
        return count;
    }

    public void allSelectAction(boolean select) {
        if (!select) {
            mTodosCheckStates.clear();
            mDonesCheckStates.clear();
        } else {
            if (mEditType == EDIT_TODOS) {
                final int countTodos = mTodosDataSource.size();
                for (int i = 0; i < countTodos; i++) {
                    mTodosCheckStates.put(i, select);
                }
            }
            if (mEditType == EDIT_DONES) {
                final int countDones = mDonesDataSource.size();
                for (int i = 0; i < countDones; i++) {
                    mDonesCheckStates.put(i, select);
                }
            }
        }
        notifyDataSetChanged();
    }

    public void selectItem(int position, boolean checked) {
        if (mTodosExpand) {
            int index = position - 1;
            LogUtils.d(TAG, "selectItem To-do : index = " + index);
            if (index >= 0 && index < mTodosDataSource.size()) { // Todo item
                mTodosCheckStates.put(index, checked);
                notifyDataSetChanged();
                return;
            }
        }
        if (mDonesExpand) { // Not Todo, should be a done item
            int index = 0;
            if (mTodosDataSource.isEmpty()) {
                index = position - 3;
            } else {
                if (mTodosExpand) {
                    index = position - 2 - mTodosDataSource.size();
                } else {
                    index = position - 2;
                }
            }
            LogUtils.d(TAG, "selectItem done : index = " + index);
            if (index >= 0 && index < mDonesDataSource.size()) {
                mDonesCheckStates.put(index, checked);
                notifyDataSetChanged();
            }
        }
    }

    public void deleteSelectedItems() {
        String selection = "";
        String ids = "";
        ArrayList<String> idsArray = new ArrayList<String>();
        boolean isSelectedAll = isSelectedAll();

        if (mEditType == EDIT_TODOS) {
            final int countTodos = mTodosDataSource.size();
            for (int i = countTodos - 1; i >= 0; i--) {
                if (mTodosCheckStates.get(i)) {
                    TodoInfo info = mTodosDataSource.get(i);
                    if (!isSelectedAll) {
                        ids += info.getId() + ",";
                        if (i % BATCH_OPER_MAX_NUM == 0) {
                            idsArray.add(ids);
                            ids = "";
                        }
                    }
                    mTodosDataSource.remove(i);
                }
            }
            mTodosCheckStates.clear();
            if(isSelectedAll) {
                selection = TodoColumn.STATUS + "=" + TodoInfo.STATUS_TODO;
            }
        } else if (mEditType == EDIT_DONES) {
            final int countDones = mDonesDataSource.size();
            for (int i = countDones - 1; i >= 0; i--) {
                if (mDonesCheckStates.get(i)) {
                    TodoInfo info = mDonesDataSource.get(i);
                    if (!isSelectedAll) {
                        ids += info.getId() + ",";
                        if (i % BATCH_OPER_MAX_NUM == 0) {
                            idsArray.add(ids);
                            ids = "";
                        }
                    }
                    mDonesDataSource.remove(i);
                }
            }
            mDonesCheckStates.clear();
            if(isSelectedAll) {
                selection = TodoColumn.STATUS + "=" + TodoInfo.STATUS_DONE;
            }
        }
        //delete todos from db.
        if(isSelectedAll) {
            mAsyncQuery.startDelete(0, this, TodoAsyncQuery.TODO_URI, selection, null);
        } else {
            if(ids.length() > 0) {
                idsArray.add(ids);
            }
            for (String idsItem: idsArray) {
                idsItem = idsItem.substring(0, idsItem.length() - 1);
                selection = TodoColumn.ID + " in (" + idsItem + ")";
                mAsyncQuery.startDelete(0, this, TodoAsyncQuery.TODO_URI, selection, null);
            }
        }
        notifyDataSetChanged();
        updateHeaderNumberText();
    }

    public void updateSelectedStatus(String curStatus, String targetStatus) {
        String selection = "";
        String ids = "";
        ArrayList<String> idsArray = new ArrayList<String>();
        ContentValues values = new ContentValues();
        boolean isSelectedAll = isSelectedAll();

        if (TodoInfo.STATUS_TODO.equals(curStatus)) { // change Todos to done
            final int countTodos = mTodosDataSource.size();
            for (int i = countTodos - 1; i >= 0; i--) {
                if (mTodosCheckStates.get(i)) {
                    TodoInfo info = mTodosDataSource.get(i);
                    info.updateStatus(TodoInfo.STATUS_DONE);
                    if (!isSelectedAll) {
                        ids += info.getId() + ",";
                        if (i % BATCH_OPER_MAX_NUM == 0) {
                            idsArray.add(ids);
                            ids = "";
                        }
                    }
                    mTodosDataSource.remove(i);
                    mDonesDataSource.add(0, info);
                }
            }
            values.put(TodoColumn.STATUS, TodoInfo.STATUS_DONE);
            values.put(TodoColumn.COMPLETE_TIME, System.currentTimeMillis());
            if(isSelectedAll) {
                selection = TodoColumn.STATUS + "=" + TodoInfo.STATUS_TODO;
            }
        } else if (TodoInfo.STATUS_DONE.equals(curStatus)) {
            final int countDones = mDonesDataSource.size();
            for (int i = countDones - 1; i >= 0; i--) {
                if (mDonesCheckStates.get(i)) {
                    TodoInfo info = mDonesDataSource.get(i);
                    info.updateStatus(TodoInfo.STATUS_TODO);
                    if (!isSelectedAll) {
                        ids += info.getId() + ",";
                        if (i % BATCH_OPER_MAX_NUM == 0) {
                            idsArray.add(ids);
                            ids = "";
                        }
                    }
                    mDonesDataSource.remove(i);
                    mTodosDataSource.add(0, info);
                }
            }
            values.put(TodoColumn.STATUS, TodoInfo.STATUS_TODO);
            values.put(TodoColumn.COMPLETE_TIME, 0);
            if(isSelectedAll) {
                selection = TodoColumn.STATUS + "=" + TodoInfo.STATUS_DONE;
            }
        }
        //update to db.
        if(isSelectedAll) {
            mAsyncQuery.startUpdate(0, this, TodoAsyncQuery.TODO_URI, values, selection, null);
        } else {
            if(ids.length() > 0) {
                idsArray.add(ids);
            }
            for (String idsItem: idsArray) {
                idsItem = idsItem.substring(0, idsItem.length() - 1);
                selection = TodoColumn.ID + " in (" + idsItem + ")";
                mAsyncQuery.startUpdate(0, this, TodoAsyncQuery.TODO_URI, values, selection, null);
            }
        }
        if (mTodosDataSource.size() > 1) {
            Collections.sort(mTodosDataSource, mTodoComparator);
        }
        if (mDonesDataSource.size() > 1) {
            Collections.sort(mDonesDataSource, mDoneComparator);
        }
        notifyDataSetChanged();
        mTodosCheckStates.clear();
        mDonesCheckStates.clear();
        updateHeaderNumberText();
    }

    public void startQuery(String selection) {
        LogUtils.d(TAG, "startQuery query from DB. selection : " + selection);
        mIsLoadingData = true;
        mAsyncQuery.startQuery(0, this, TodoAsyncQuery.TODO_URI, null, selection, null, null);
    }

    public void onQueryComplete(int token, Cursor cursor) {
        LogUtils.d(TAG, "onQueryComplete.");
        if (cursor != null && cursor.moveToFirst()) {
            do {
                TodoInfo todoInfo = TodoInfo.makeTodoInfoFromCursor(cursor);
                if (TodoInfo.STATUS_DONE.equals(todoInfo.getStatus())) {
                    mDonesDataSource.add(todoInfo);
                } else {
                    mTodosDataSource.add(todoInfo);
                }
            } while (cursor.moveToNext());
        }
        /**M: close cursor*/
        if(cursor != null){
            cursor.close();
        }

        if (mTodosDataSource.size() > 1) {
            Collections.sort(mTodosDataSource, mTodoComparator);
        }
        if (mDonesDataSource.size() > 1) {
            Collections.sort(mDonesDataSource, mDoneComparator);
        }
        updateHeaderNumberText();
        notifyDataSetChanged();
        mIsLoadingData = false;
    }

    public void startDelete(TodoInfo info) {
        LogUtils.d(TAG, "startDelete(). info = " + info);
        Utils.writeAdapterDataToDB(this, info, mAsyncQuery, Utils.OPERATOR_DELETE);
    }

    public void onDeleteComplete(int token, int result) {
        if (result <= 0) {
            LogUtils.e(TAG, "onDeleteComplete() failed : " + result);
            Utils.prompt(mContext, R.string.operator_failed);
            mIsDataDirty = true;
        }
    }

    public void startUpdate(TodoInfo info) {
        LogUtils.d(TAG, "startUpdate() : ");
        Utils.writeAdapterDataToDB(this, info, mAsyncQuery, Utils.OPERATOR_UPDATE);
    }

    public void onUpdateComplete(int token, int result) {
        if (result <= 0) {
            LogUtils.e(TAG, "onUpdateComplete() result : " + result);
            Utils.prompt(mContext, R.string.operator_failed);
            mIsDataDirty = true;
        }
    }

    public void startInsert(TodoInfo info) {
        LogUtils.d(TAG, "startInsert().");
        Utils.writeAdapterDataToDB(this, info, mAsyncQuery, Utils.OPERATOR_INSERT);
    }

    public void onInsertComplete(int token, Uri uri) {
        LogUtils.d(TAG, "onInsertComplete() uri : " + uri);
    }

    @Override
    public int getItemViewType(int position) {
        int viewType = -1;
        final int mTodosDataSize = mTodosDataSource.size();
        final int mTodosShowSize = mTodosExpand ? mTodosDataSource.size() : 0;
        if (position == 0) {
            viewType = TYPE_TODOS_HEADER;
        } else if (position == 1 && mTodosDataSize == 0) {
            viewType = TYPE_TODOS_FOOTER;
        } else if (position <= mTodosShowSize) {
            viewType = TYPE_TODOS_ITEM;
        } else if ((mTodosDataSize == 0 && position == 2)
                || (mTodosDataSize != 0 && position == mTodosShowSize + 1)) {
            viewType = TYPE_DONES_HEADER;
        } else if (mDonesDataSource.isEmpty()) {
            viewType = TYPE_DONES_FOOTER;
        } else {
            viewType = TYPE_DONES_ITEM;
        }
        return viewType;
    }

    public int getItemPosition(TodoInfo info) {
        int position = 0;
        if (TodoInfo.STATUS_TODO.equals(info.getStatus())) {
            if (mTodosExpand && !mTodosDataSource.isEmpty()) {
                int index = mTodosDataSource.indexOf(info);
                if (index >= 0) {
                    position = index + 1;
                }
            }
        } else if (TodoInfo.STATUS_DONE.equals(info.getStatus())) {
            if (mDonesExpand && !mDonesDataSource.isEmpty()) {
                int index = mDonesDataSource.indexOf(info);
                if (mTodosExpand && !mTodosDataSource.isEmpty()) {
                    position = 2 + mTodosDataSource.size();
                } else {
                    position = 3;
                }
                if (index >= 0) {
                    position += index;
                }
            }
        }
        LogUtils.d(TAG, "getItemPosition() position=" + position);
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        int viewType = getItemViewType(position);
        switch (viewType) {
        case TYPE_TODOS_HEADER:
            updateTodosHeaderView();
            return mTodosHeaderView;
        case TYPE_TODOS_FOOTER:
            updateTodosFooterView();
            return mTodosFooterView;
        case TYPE_DONES_HEADER:
            updateDonesHeaderView();
            return mDonesHeaderView;
        case TYPE_DONES_FOOTER:
            updateDonesFooterView();
            return mDonesFooterView;
        case TYPE_TODOS_ITEM:
        case TYPE_DONES_ITEM:
        default:
            return getItemView(position, convertView, parent);
        }
    }

    private void updateTodosHeaderView() {
        StringBuffer todosTitle = new StringBuffer();
        todosTitle.append(mContext.getResources().getString(R.string.todos_title));
        todosTitle.append(" (").append(mTodosDataSource.size()).append(")");
        HeaderHolder todosHolder = null;
        if (mTodosHeaderView == null) {
            todosHolder = new HeaderHolder();
            mTodosHeaderView = mInflater.inflate(R.layout.list_header, null);
            todosHolder.mHeaderTitle = (TextView) mTodosHeaderView.findViewById(R.id.list_title);
            todosHolder.mBtnExpand = (ImageView) mTodosHeaderView.findViewById(R.id.btn_expand);
            mTodosHeaderView.setTag(todosHolder);
        } else {
            todosHolder = (HeaderHolder) mTodosHeaderView.getTag();
        }
        todosHolder.mHeaderTitle.setText(todosTitle.toString());
        if (mTodosExpand) {
            todosHolder.mBtnExpand.setImageDrawable(mContext.getResources().getDrawable(R.drawable.ic_expand_open));
        } else {
            todosHolder.mBtnExpand.setImageDrawable(mContext.getResources().getDrawable(R.drawable.ic_expand_close));
        }

        // no items in todos list, set the button transparent and un-clickable
        if (mTodosDataSource.isEmpty()) {
            todosHolder.mBtnExpand.setAlpha(0f);
            todosHolder.mBtnExpand.setClickable(true);
        } else {
            todosHolder.mBtnExpand.setAlpha(1f);
            todosHolder.mBtnExpand.setClickable(false);
        }

        int detaultColor = mContext.getResources().getColor(R.color.ListHeaderBgColor);
        Utils.updateViewBackgroud(mContext, mTodosHeaderView, detaultColor);
    }

    private void updateTodosFooterView() {
        String todosInfo = mContext.getResources().getString(R.string.todos_empty_info);
        FooterHolder todoFooterHolder = null;
        if (mTodosFooterView == null) {
            todoFooterHolder = new FooterHolder();
            mTodosFooterView = mInflater.inflate(R.layout.list_footer, null);
            todoFooterHolder.mFooterHelper = (TextView) mTodosFooterView
                    .findViewById(R.id.footer_info);
            mTodosFooterView.setTag(todoFooterHolder);
        } else {
            todoFooterHolder = (FooterHolder) mTodosFooterView.getTag();
        }
        todoFooterHolder.mFooterHelper.setText(todosInfo);
        updateFooterViewState(mTodosFooterView);
    }

    private void updateDonesHeaderView() {
        StringBuffer donesTitle = new StringBuffer();
        donesTitle.append(mContext.getResources().getString(R.string.dones_title));
        donesTitle.append(" (").append(mDonesDataSource.size()).append(")");
        HeaderHolder donesHolder = null;
        if (mDonesHeaderView == null) {
            donesHolder = new HeaderHolder();
            mDonesHeaderView = mInflater.inflate(R.layout.list_header, null);
            donesHolder.mHeaderTitle = (TextView) mDonesHeaderView.findViewById(R.id.list_title);
            donesHolder.mBtnExpand = (ImageView) mDonesHeaderView.findViewById(R.id.btn_expand);
            mDonesHeaderView.setTag(donesHolder);
        } else {
            donesHolder = (HeaderHolder) mDonesHeaderView.getTag();
        }
        donesHolder.mHeaderTitle.setText(donesTitle.toString());
        if (mDonesExpand) {
            donesHolder.mBtnExpand.setImageDrawable(mContext.getResources().getDrawable(R.drawable.ic_expand_open));
        } else {
            donesHolder.mBtnExpand.setImageDrawable(mContext.getResources().getDrawable(R.drawable.ic_expand_close));
        }

        // no items in dones list, set the button transparent and un-clickable
        if (mDonesDataSource.isEmpty()) {
            donesHolder.mBtnExpand.setAlpha(0f);
            donesHolder.mBtnExpand.setClickable(true);
        } else {
            donesHolder.mBtnExpand.setAlpha(1f);
            donesHolder.mBtnExpand.setClickable(false);
        }

        int detaultColor = mContext.getResources().getColor(R.color.ListHeaderBgColor);
        Utils.updateViewBackgroud(mContext, mDonesHeaderView, detaultColor);
    }

    private void updateDonesFooterView() {
        String donesInfo = mContext.getResources().getString(R.string.dones_empty_info);
        FooterHolder donesFooterHolder = null;
        if (mDonesFooterView == null) {
            donesFooterHolder = new FooterHolder();
            mDonesFooterView = mInflater.inflate(R.layout.list_footer, null);
            donesFooterHolder.mFooterHelper = (TextView) mDonesFooterView
                    .findViewById(R.id.footer_info);
            mDonesFooterView.setTag(donesFooterHolder);
        } else {
            donesFooterHolder = (FooterHolder) mDonesFooterView.getTag();
        }
        donesFooterHolder.mFooterHelper.setText(donesInfo);
        updateFooterViewState(mDonesFooterView);
    }

    private View getItemView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null || !(convertView.getTag() instanceof ViewHolder)) {
            convertView = mInflater.inflate(R.layout.list_item, parent, false);
            holder = new ViewHolder();
            holder.mTodoInfoText = (TextView) convertView.findViewById(R.id.item_text);
            holder.mTodoInfoState = (ImageView) convertView.findViewById(R.id.item_state);
            holder.mTodoInfoDueDate = (TextView) convertView.findViewById(R.id.item_due_date);
            holder.mTodoInfoCheckBox = (CheckBox) convertView.findViewById(R.id.item_checkbox);
            holder.mChangeInfoState = (ImageView) convertView.findViewById(R.id.change_info_state);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        TodoInfo info = null;
        int statusDrawableId = R.drawable.todos_ic_expired;
        String dateText = null;
        boolean checkBoxEnable = false;
        boolean checked = false;
        int changeStatusDrawableId = R.drawable.todos_ic_todo;
        if (mTodosExpand) {
            int index = getIndexByPosition(TodoInfo.STATUS_TODO, position);
            if (index >= 0 && index < mTodosDataSource.size()) { // Todo item
                info = mTodosDataSource.get(index);
                dateText = Utils.getDateText(mContext, info.getDueDate(), Utils.DATE_TYPE_DUE);
                checked = mTodosCheckStates.get(index);
                if (info.isExpire()) {
                    statusDrawableId = R.drawable.todos_ic_expired;
                } else {
                    statusDrawableId = R.drawable.todos_ic_unexpire;
                }
                checkBoxEnable = (mEditType == EDIT_TODOS) ? true : false;
                //set todo font 100% opaque.
                holder.mTodoInfoText.setAlpha(ALPHA_TODO);
                holder.mTodoInfoDueDate.setAlpha(ALPHA_TODO);
                holder.mChangeInfoState.setImageResource(changeStatusDrawableId);
            }
        }
        if (info == null && mDonesExpand) { // Not Todo, should be Done.
            int index = getIndexByPosition(TodoInfo.STATUS_DONE, position);
            if (index >= 0 && index < mDonesDataSource.size()) {
                info = mDonesDataSource.get(index);
                dateText = Utils.getDateText(mContext, info.getCompleteTime(),
                        Utils.DATE_TYPE_COMPLETE);
                checked = mDonesCheckStates.get(index);
                checkBoxEnable = (mEditType == EDIT_DONES) ? true : false;
                statusDrawableId = R.drawable.todos_ic_done;
                changeStatusDrawableId = R.drawable.todos_ic_todo_done;
                //set done font 60% opaque.
                holder.mTodoInfoText.setAlpha(ALPHA_DONE);
                holder.mTodoInfoDueDate.setAlpha(ALPHA_DONE);
                holder.mChangeInfoState.setImageResource(changeStatusDrawableId);
            }
        }

        if (info == null) {
            LogUtils.e(TAG, "-------info is null ---------" + position);
            LogUtils.e(TAG, "position = " + position + " To-do size : " + mTodosDataSource.size()
                    + " dones size : " + mDonesDataSource.size());
        }
        holder.mTodoInfoText.setText(getInfoText(info));
        holder.mTodoInfoState.setImageResource(statusDrawableId);
        holder.mTodoInfoDueDate.setText(dateText);
        // update the convertView Background,it will be effected be theme manager
        Utils.updateViewBackgroud(mContext, convertView, R.drawable.list_selected_holo_light, checked);
        // do not show checkbox, but should update its status, because other
        // place still use its status. TODO: change it in future.
        holder.mTodoInfoCheckBox.setVisibility(View.INVISIBLE);
        updateCheckBoxIcon(holder);
        holder.mTodoInfoCheckBox.setChecked(checked);

        if (isEditing()) {
            if (!checkBoxEnable && getSelectedNum() != 0) {
                // In edit mode, 1.if the list(it belongs to) is not in multi-select status
                // And 2. another list is in multi-select status , disable it.
                convertView.setAlpha(ALPHA_DISABLE);
                convertView.setEnabled(false);
            } else {
                convertView.setAlpha(ALPHA_ENABLE);
                convertView.setEnabled(true);
            }
            holder.mChangeInfoState.setEnabled(false);
            holder.mChangeInfoState.getBackground().setAlpha(ALPHA_TRANSPARENT);
        } else {
            convertView.setEnabled(true);
            holder.mChangeInfoState.getBackground().setAlpha(ALPHA_OPAQUE);
            holder.mChangeInfoState.setEnabled(true);
            holder.mChangeInfoState.setImageResource(changeStatusDrawableId);
            holder.mChangeInfoState.setOnClickListener(this);
            holder.mChangeInfoState.setTag(info);
        }
        return convertView;
    }
    
    private int getIndexByPosition(String status, int position) {
        int index = 0;
        if (TodoInfo.STATUS_TODO.equals(status)) {
            index = position - 1;
        } else {
            if (mTodosDataSource.isEmpty()) {
                index = position - 3;
            } else {
                if (mTodosExpand) {
                    index = position - 2 - mTodosDataSource.size();
                } else {
                    index = position - 2;
                }
            }
        }
        return index;
    }

    private String getInfoText(TodoInfo info) {
        if (info == null) {
            return "";
        }
        if (TextUtils.isEmpty(info.getTitle())) {
            return info.getDescription();
        }
        return info.getTitle();
    }

    static class HeaderHolder {
        TextView mHeaderTitle = null;
        ImageView mBtnExpand = null;
    }

    static class FooterHolder {
        TextView mFooterHelper = null;
    }

    static class ViewHolder {
        TextView mTodoInfoText = null;
        ImageView mTodoInfoState = null;
        TextView mTodoInfoDueDate = null;
        CheckBox mTodoInfoCheckBox = null;
        ImageView mChangeInfoState = null;
    }

    public void onClick(View v) {
        Object tag = v.getTag();
        LogUtils.d(TAG, "TodosListAdapter Click view tag : " + tag);
        if (tag != null && tag instanceof TodoInfo) {
            TodoInfo info = (TodoInfo) tag;
            if (TodoInfo.STATUS_DONE.equals(info.getStatus())) {
                info.updateStatus(TodoInfo.STATUS_TODO);
                // write the change to DB.
                startUpdate(info);
                mDonesDataSource.remove(info);
                addItem(info);
            } else if (TodoInfo.STATUS_TODO.equals(info.getStatus())) {
                info.updateStatus(TodoInfo.STATUS_DONE);
                // write the change to DB.
                startUpdate(info);
                mTodosDataSource.remove(info);
                addItem(info);
            }

            if (mContext instanceof TodosActivity) {
                ((TodosActivity) mContext).updateBottomBarWidgetState();
            }
        }
    }

    /**
     * Clear all data in Adapter, then query data from DB.
     */
    public void refreshData() {
        mTodosDataSource.clear();
        mDonesDataSource.clear();
        mIsDataDirty = false;
        startQuery(null);
    }

    public void onDateChange() {
        notifyDataSetChanged();
    }

    public void onTimePick() {
        notifyDataSetChanged();
    }

    private void updateHeaderNumberText() {
        TodosActivity activity = (TodosActivity) mContext;
        if (mTodosDataSource.isEmpty() && mDonesDataSource.isEmpty()) {
            activity.updateHeaderNumberText(EMPTY_TEXT);
        } else {
            // If todos' count is more than 999, we display "999+"
            int count = mTodosDataSource.size();
            activity.updateHeaderNumberText((count > 999)
                    ? mContext.getString(R.string.more_than_999) : Integer.toString(count));
        }
    }

    private void updateCheckBoxIcon(ViewHolder holder) {
        if (mEditType == EDIT_TODOS) {
            holder.mTodoInfoCheckBox.setButtonDrawable(R.drawable.todos_ic_todo);
        } else if (mEditType == EDIT_DONES) {
            holder.mTodoInfoCheckBox.setButtonDrawable(R.drawable.todos_ic_todo_done);
        }
    }

    public boolean isSelectedAll() {
        if (mEditType == EDIT_TODOS) {
            return mTodosDataSource.size() == getSeletedTodosNumber();
        } else if (mEditType == EDIT_DONES) {
            return mDonesDataSource.size() == getSeletedDonesNumber();
        }
        return false;
    }

    public int getSelectedNum() {
        return getSeletedDonesNumber() + getSeletedTodosNumber();
    }

    public void updateFooterViewState(View footerView) {
        if (isEditing()) {
            footerView.setEnabled(false);
            footerView.setAlpha(ALPHA_DISABLE);
        } else {
            footerView.setEnabled(true);
            footerView.setAlpha(ALPHA_ENABLE);
        }
    }
}
