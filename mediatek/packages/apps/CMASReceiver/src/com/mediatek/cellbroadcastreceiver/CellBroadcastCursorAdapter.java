/*
 * Copyright (C) 2011 The Android Open Source Project
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
 */

package com.mediatek.cellbroadcastreceiver;

import java.util.HashSet;

import android.content.Context;
import android.database.Cursor;
import android.telephony.CellBroadcastMessage;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.provider.Telephony;

/**
 * The back-end data adapter for {@link CellBroadcastListActivity}.
 */
public class CellBroadcastCursorAdapter extends CursorAdapter {
    private OnContentChangedListener mOnContentChangedListener;
    private HashSet<Long> mSelectedBroadcastIds;

    public CellBroadcastCursorAdapter(Context context, Cursor cursor) {
        // don't set FLAG_AUTO_REQUERY or FLAG_REGISTER_CONTENT_OBSERVER
        super(context, cursor, 0);
        mSelectedBroadcastIds = new HashSet<Long>();
    }

    /**
     * Makes a new view to hold the data pointed to by cursor.
     * @param context Interface to application's global information
     * @param cursor The cursor from which to get the data. The cursor is already
     * moved to the correct position.
     * @param parent The parent to which the new view is attached to
     * @return the newly created view.
     */
    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        //CellBroadcastMessage message = CellBroadcastMessage.createFromCursor(cursor);
        LayoutInflater factory = LayoutInflater.from(context);
        CellBroadcastListItem listItem = (CellBroadcastListItem) factory.inflate(
                    R.layout.cell_broadcast_list_item, parent, false);
        //listItem.bind(message,bSelected);
        return listItem;
    }

    /**
     * Bind an existing view to the data pointed to by cursor
     * @param view Existing view, returned earlier by newView
     * @param context Interface to application's global information
     * @param cursor The cursor from which to get the data. The cursor is already
     * moved to the correct position.
     */
    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        CellBroadcastMessage message = CellBroadcastMessage.createFromCursor(cursor);
        boolean bSelected = false;
        boolean bRead = false;
        if (mSelectedBroadcastIds != null && cursor.getCount() > 0) {
            bSelected = mSelectedBroadcastIds.contains(cursor.getLong(0));
            int index = cursor.getColumnIndex(Telephony.CellBroadcasts.MESSAGE_READ);
            bRead = cursor.getInt(index) == 1 ? true : false;
        }
        CellBroadcastListItem listItem = (CellBroadcastListItem) view;
        listItem.bind(message, bSelected, bRead);
    }

    public void removeSelectedId(long id) {
        if (mSelectedBroadcastIds != null) {
            mSelectedBroadcastIds.remove(id);
        }
    }

    public void addSelectedId(long id) {
        if (mSelectedBroadcastIds != null) {
            mSelectedBroadcastIds.add(id);
        }
    }

    public boolean IsItemSelected(long id) {
        boolean bSelected = false;
        if (mSelectedBroadcastIds != null) {
            bSelected = mSelectedBroadcastIds.contains(id);
        }
        return bSelected;
    }

    public void clearSeleted(){
        if (mSelectedBroadcastIds != null) {
            mSelectedBroadcastIds.clear();
        }
    }

    public int getSelectedCount() {
        int count = 0;
        if (mSelectedBroadcastIds != null) {
            count = mSelectedBroadcastIds.size();
        }
        return count;
    }

    public  HashSet<Long> getSelectedSet() {
         return mSelectedBroadcastIds;
    }

    public interface OnContentChangedListener {
        void onContentChanged(CellBroadcastCursorAdapter adapter);
    }

    public void setOnContentChangedListener(OnContentChangedListener l) {
        mOnContentChangedListener = l;
    }

    @Override
    protected void onContentChanged() {
        if (mOnContentChangedListener != null) {
            mOnContentChangedListener.onContentChanged(this);
        }
    }
}
