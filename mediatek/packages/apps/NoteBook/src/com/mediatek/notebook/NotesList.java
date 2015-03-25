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
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.notebook;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.notebook.NoteAdapter.NoteItem;
import com.mediatek.notebook.NotePad.Notes;

import java.util.Locale;

public class NotesList extends ListActivity implements OnItemLongClickListener {

    private static final String TAG = "NotesList";
    public TextView countView;
    public NoteAdapter noteadapter;
    private ProgressDialog mPdialogDeleting;
    private ModeCallback mActionModeListener = new ModeCallback();
    public ActionMode mActionMode;
    private Button mSelectedItemsView;
    private NoteWaitCursorView mWaitCursorView;
    private TextView mLoadingNotes;
    private View mLoadingContainer;
    private ProgressBar mProgress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setDefaultKeyMode(DEFAULT_KEYS_SHORTCUT);
        setContentView(R.layout.noteslist_item_main);
        ActionBar actionBar = getActionBar();
        actionBar.setHomeButtonEnabled(false);
        ViewGroup view = (ViewGroup) LayoutInflater.from(this).inflate(
                R.layout.notelist_action_bar, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(view, new ActionBar.LayoutParams(
                LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                        | Gravity.RIGHT));
        countView = (TextView) view.findViewById(R.id.note_count);
        getListView().setOnItemLongClickListener(this);
        mLoadingNotes = (TextView)findViewById(R.id.loading_notes);
        mLoadingContainer = findViewById(R.id.loading_container);
        mProgress = (ProgressBar)findViewById(R.id.progress_loading);
        mProgress.setVisibility(View.GONE);
        mWaitCursorView = new NoteWaitCursorView(this, mLoadingContainer,
                mProgress, mLoadingNotes);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "mActionMode = " + mActionMode + "\nsFlagNotesChanged"
                + Notes.sFlagNotesChanged);
        if (mActionMode == null && Notes.sFlagNotesChanged) {
            queryUpdateData();
            Notes.sFlagNotesChanged = false;
        }
        if (Notes.sSaveNoteFlag) {
            Toast.makeText(this, R.string.note_saved, Toast.LENGTH_LONG).show();
            Notes.sSaveNoteFlag = false;
        } else if (Notes.sSaveNoNote) {
            Toast.makeText(this, R.string.save_none, Toast.LENGTH_LONG).show();
            Notes.sSaveNoNote = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        noteadapter = null;
        Notes.sFlagNotesChanged = true;
    }

    public void queryUpdateData() {
        Log.i(TAG, "queryUpdateData");
        mWaitCursorView.startWaitCursor();
        invalidateOptionsMenu();
        noteadapter = new NoteAdapter(this, null, 0);
        QueryHandler qh = new QueryHandler(this.getContentResolver(), this);
        qh.startQuery(0, mWaitCursorView, Notes.CONTENT_URI, Notes.PROJECTION,
                null, null, Notes.sSortOrder);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.list_options_menu, menu);
        Intent intent = new Intent(null, Notes.CONTENT_URI);
        intent.addCategory(Intent.CATEGORY_ALTERNATIVE);
        menu.addIntentOptions(Menu.CATEGORY_ALTERNATIVE, 0, 0,
                new ComponentName(this, NotesList.class), null, intent, 0, null);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        if (Notes.sNoteCount == 0 || mWaitCursorView.getQueryStatus()) {
            menu.findItem(R.id.menu_muti_delete).setEnabled(false);
            menu.findItem(R.id.menu_sort_by_modify).setEnabled(false);
            menu.findItem(R.id.menu_sort_by_tab).setEnabled(false);
        } else {
            menu.findItem(R.id.menu_muti_delete).setEnabled(true);
            menu.findItem(R.id.menu_sort_by_modify).setEnabled(true);
            menu.findItem(R.id.menu_sort_by_tab).setEnabled(true);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Intent it;
        switch (item.getItemId()) {
        case R.id.menu_add:
            it = new Intent(this, NoteEdit.class);
            this.startActivity(it);
            return true;
        case R.id.menu_muti_delete:
            deleteAllNotes();
            return true;
        case R.id.menu_sort_by_tab:
            Notes.sSortOrder = "notegroup DESC, modified DESC";
            queryUpdateData();
            return true;
        case R.id.menu_sort_by_modify:
            Notes.sSortOrder = "modified DESC";
            queryUpdateData();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        NoteItem noteitem = (NoteItem) l.getAdapter().getItem(position);
        Uri uri = ContentUris.withAppendedId(Notes.CONTENT_URI, noteitem.id);
        if (mActionMode != null) {
            noteadapter.checkboxClickAction(position);
            mSelectedItemsView.setText(getString(R.string.selected_item_count,
                    Notes.sDeleteNum));
            if (Notes.sDeleteNum == 0) {
                mActionMode.finish();
            }
        } else {
            Intent it = new Intent(this, NoteReading.class);
            it.setData(uri);
            this.startActivity(it);
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView parent, View view, int position,
            long id) {
        Notes.sDeleteNum = 1;
        mActionMode = startActionMode(mActionModeListener);
        noteadapter.checkboxClickAction(position);
        return true;
    }

    private class ModeCallback implements ActionMode.Callback {
        private View mMultiSelectActionBarView;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.list_delete_menu, menu);
            if (mMultiSelectActionBarView == null) {
                mMultiSelectActionBarView = LayoutInflater.from(NotesList.this)
                        .inflate(R.layout.noteslist_item_delete_action_bar,
                                null);
            }
            mode.setCustomView(mMultiSelectActionBarView);
            ((TextView) mMultiSelectActionBarView
                    .findViewById(R.id.select_items)).setText(getString(
                    R.string.selected_item_count, Notes.sDeleteNum));
            mMultiSelectActionBarView.setVisibility(View.VISIBLE);
            mSelectedItemsView = (Button) mMultiSelectActionBarView
                    .findViewById(R.id.select_items);
            mSelectedItemsView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    PopupMenu popup = new PopupMenu(NotesList.this, v);
                    popup.getMenuInflater().inflate(
                            R.menu.multi_delete_action_bar, popup.getMenu());
                    popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        @Override
                        public boolean onMenuItemClick(MenuItem item) {
                            int id = item.getItemId();
                            if (id == R.id.menu_all_select) {
                                noteadapter.selectAllOrNoCheckbox(true);
                                mSelectedItemsView.setText(getString(
                                        R.string.selected_item_count,
                                        Notes.sDeleteNum));
                            } else if (id == R.id.menu_no_select) {
                                mActionMode.finish();
                            } else {
                                return true;
                            }
                            return false;
                        }
                    });
                    Menu popupMenu = popup.getMenu();
                    MenuItem selectAllItem = popupMenu
                            .findItem(R.id.menu_all_select);
                    MenuItem unSelectAllItem = popupMenu
                            .findItem(R.id.menu_no_select);
                    if (Notes.sDeleteNum >= noteadapter.getCount()) {
                        if (selectAllItem != null) {
                            selectAllItem.setVisible(false);
                        }
                        if (unSelectAllItem != null) {
                            unSelectAllItem.setVisible(true);
                        }
                    } else {
                        if (selectAllItem != null) {
                            selectAllItem.setVisible(true);
                        }
                        if (unSelectAllItem != null) {
                            unSelectAllItem.setVisible(false);
                        }
                    }
                    popup.show();
                }
            });    
            getListView().setLongClickable(false);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            Log.i(TAG, "onDestroyActionMode");
            mMultiSelectActionBarView.setVisibility(View.GONE);
            invalidateOptionsMenu();
            getListView().setLongClickable(true);
            mActionMode = null;
            noteadapter.selectAllOrNoCheckbox(false);
        }

        @Override
        public boolean onActionItemClicked(final ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.menu_delete:
                mutiDeleteSelectNotes();
                break;
            default:
                break;
            }
            return true;
        }
    }

    private void mutiDeleteSelectNotes() {
        if (Notes.sDeleteNum > 0) {
            AlertDialog.Builder bld = new AlertDialog.Builder(this);
            bld.setPositiveButton(getString(R.string.delete_confirm_ok),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            beginDeleteNotes(Notes.NOTESLIST_DELETE_TOKEN);
                        }
                    });
            bld.setNegativeButton(getString(R.string.delete_confirm_cancel),
                    null);
            bld.setCancelable(true);
            bld.setMessage(getString(R.string.delete_confirm));
            if (Notes.sDeleteNum > 1
                    && Locale.getDefault().getLanguage().equals("en")) {
                bld.setMessage(getString(R.string.delete_confirm));
            } else {
                bld.setMessage(getString(R.string.delete_confirm_one));
            }
            bld.setIconAttribute(android.R.attr.alertDialogIcon);
            bld.setTitle(getString(R.string.delete_confirm_title));
            AlertDialog dlg = bld.create();
            dlg.show();
        } else {
            Toast.makeText(this, R.string.no_selected, Toast.LENGTH_LONG)
                    .show();
        }        
    }
    
    public void beginDeleteNotes(int token) {
        Log.i(TAG, "beginDeleteNotes");
        mPdialogDeleting = ProgressDialog.show(this, "",
                getString(R.string.delete_progress), true);
        QueryHandler qh = new QueryHandler(this.getContentResolver(), this);
        Log.i(TAG, "noteadapter.getFilter() =="+noteadapter.getFilter());
        qh.startDelete(token, mPdialogDeleting, Notes.CONTENT_URI,
                noteadapter.getFilter(), null);
    }
    
    private void deleteAllNotes() {
        AlertDialog.Builder bld = new AlertDialog.Builder(this);
        bld.setPositiveButton(getString(R.string.delete_confirm_ok),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        for (int i = 0; i < noteadapter.list.size(); i++) {
                            noteadapter.list.get(i).isselect = true;
                        }
                        beginDeleteNotes(Notes.NOTESLIST_DELETE_ALL_TOKEN);
                    }
                });
        bld.setNegativeButton(getString(R.string.delete_confirm_cancel), null);
        bld.setCancelable(true);
        bld.setMessage(getString(R.string.delete_confirm));
        bld.setMessage(getString(R.string.delete_all_notes_confirm));
        bld.setIconAttribute(android.R.attr.alertDialogIcon);
        bld.setTitle(getString(R.string.delete_confirm_title));
        bld.create().show();
    }
}
