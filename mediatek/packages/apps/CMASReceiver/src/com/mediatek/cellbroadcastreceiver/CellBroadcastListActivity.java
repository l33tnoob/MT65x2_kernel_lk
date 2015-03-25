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

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.app.NotificationManager;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Configuration;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.HashSet;

/**
 * This activity provides a list view of received cell broadcasts.
 */
public class CellBroadcastListActivity extends ListActivity {
    private static final String TAG = "CellBroadcastListActivity";

    private static final int ALERTS_LIST_QUERY_TOKEN   = 1701;
    private static final int UNREAD_ALERTS_QUERY_TOKEN = 1702;

     // IDs of the main menu items.
    private static final int MENU_DELETE_ALL           = 1;
    private static final int MENU_PREFERENCES          = 2;

    private TextView mUnreadConvCount;
    private CellBroadcastListQueryHandler mQueryHandler;
    private ListView mListView;
    private CellBroadcastCursorAdapter mListAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.cell_broadcast_list_screen);
        initListAdapter();
        setupActionBar();

        mQueryHandler = new CellBroadcastListQueryHandler(getContentResolver());

        // Dismiss the notification that brought us here (if any).
        ((NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE))
                .cancel(CellBroadcastAlertService.NOTIFICATION_ID);

        clearNotification(getIntent().getBooleanExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, false));
        dismissAllDialogShow();
    }

    private void initListAdapter() {
        mListView = getListView();
        mListAdapter = new CellBroadcastCursorAdapter(this, null);
        mListAdapter.setOnContentChangedListener(mContentChangedListener);
        setListAdapter(mListAdapter);

        mListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);
        mListView.setMultiChoiceModeListener(new ModeCallback());
    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();

        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(
                R.layout.cell_broadcast_list_actionbar, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL | Gravity.RIGHT));

        mUnreadConvCount = (TextView) v.findViewById(R.id.unread_conv_count);
    }

    private void registerCellBroadcastObserver() {
        getContentResolver().registerContentObserver(CellBroadcastContentProvider.CONTENT_URI,
                true, cellBroadcastChangeObserver);
    }

    private final ContentObserver cellBroadcastChangeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfUpdate) {
            Log.d(TAG,"cellBroadcastChangeObserver,startAsyncQuery...");
            startAsyncQuery();
        }
    };

    private final CellBroadcastCursorAdapter.OnContentChangedListener mContentChangedListener =
        new CellBroadcastCursorAdapter.OnContentChangedListener() {
        @Override
        public void onContentChanged(CellBroadcastCursorAdapter adapter) {
            startAsyncQuery();
        }
    };

    @Override
    public void onResume() {
        super.onResume();
        registerCellBroadcastObserver();
    }

    @Override
    protected void onStart() {
        super.onStart();
        startAsyncQuery();
    }

    @Override
    protected void onStop() {
        super.onStop();
        getContentResolver().unregisterContentObserver(cellBroadcastChangeObserver);
        if (mListAdapter != null) {
            Cursor cursor = mListAdapter.getCursor();
            if (cursor != null && !cursor.isClosed()) {
                Log.d(TAG, "onStop,close cursor");
                cursor.close();
                cursor = null;
            }
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        CellBroadcastListItem cbli = (CellBroadcastListItem) v;
        showDialogAndMarkRead(cbli.getMessage(), id);
        super.onListItemClick(l, v, position, id);
    }

    private void showDialogAndMarkRead(CellBroadcastMessage cbm, long msgRowId) {
        // show emergency alerts with the warning icon, but don't play alert tone
        CMASAlertFullWindow showWindow = CMASAlertFullWindow.getInstance(getApplicationContext());
        showWindow.showView(cbm, msgRowId, false);
    }

    private void startAsyncQuery() {
        try {
            mQueryHandler.cancelOperation(ALERTS_LIST_QUERY_TOKEN);
            mQueryHandler.cancelOperation(UNREAD_ALERTS_QUERY_TOKEN);

            mQueryHandler.startQuery(ALERTS_LIST_QUERY_TOKEN, null,
                    CellBroadcastContentProvider.CONTENT_URI, null,
                    null, null, null);
            mQueryHandler.startQuery(UNREAD_ALERTS_QUERY_TOKEN, null,
                    CellBroadcastContentProvider.CONTENT_URI, null,
                    Telephony.CellBroadcasts.MESSAGE_READ + " =0", null, null);
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_DELETE_ALL, 0, R.string.menu_delete_all).setIcon(
                android.R.drawable.ic_menu_delete);
        menu.add(0, MENU_PREFERENCES, 0, R.string.menu_preferences).setIcon(
                android.R.drawable.ic_menu_preferences);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case MENU_DELETE_ALL:
                confirmDeleteThread(true,null);
                break;

            case MENU_PREFERENCES:
                Intent intent = new Intent(this, CellBroadcastMainSettings.class);
                startActivity(intent);
                break;

            default:
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        menu.findItem(MENU_DELETE_ALL).setVisible(!mListAdapter.isEmpty());
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.d(TAG, "onNewIntent ++");
        if (intent.getBooleanExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, false)) {
            //clear notification
            clearNotification(getIntent().getBooleanExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, false));

            dismissAllDialogShow();
        }
    }

    private void clearNotification(boolean bClear) {
        Log.i(TAG, "enter onNewIntent");
        if (bClear) {
            Log.d(TAG, "Dimissing notification");
            NotificationManager notificationManager = (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.cancel(CMASPresentationService.NOTIFICATION_ID);
        }
    }

    private final class CellBroadcastListQueryHandler extends AsyncQueryHandler {
        public CellBroadcastListQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            // TODO Auto-generated method stub
            super.onQueryComplete(token, cookie, cursor);
            switch (token) {
                case ALERTS_LIST_QUERY_TOKEN:
                    if (cursor != null) {
                        mListAdapter.changeCursor(cursor);
                        if (mListAdapter.getCount() == 0) {
                            ((TextView) (getListView().getEmptyView()))
                                    .setText(R.string.no_cell_broadcasts);
                        }
                        invalidateOptionsMenu();
                    }
                    break;
                case UNREAD_ALERTS_QUERY_TOKEN:
                    int count = 0;
                    if (cursor != null) {
                        count = cursor.getCount();
                        cursor.close();
                    }
                    mUnreadConvCount.setText(count > 0 ? Integer.toString(count) : null);
                    break;
               default:
                    Log.e(TAG, "onQueryComplete called with unknown token " + token);
            }
        }
    }

    private class ModeCallback implements ListView.MultiChoiceModeListener {
        private View mMultiSelectActionBarView;
        private TextView mTitle;
        private HashSet<Long> mSelectedBroadcastIds;
        private MenuItem deleteItem;
        private MenuItem selectAllItem;
        private MenuItem unSelectAllItem;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = getMenuInflater();
            mSelectedBroadcastIds = new HashSet<Long>();
            inflater.inflate(R.menu.cell_broadcast_multi_select_menu, menu);
            deleteItem = menu.findItem(R.id.delete);
            unSelectAllItem = menu.findItem(R.id.cancel_select);
            selectAllItem = menu.findItem(R.id.select_all);

            if (mMultiSelectActionBarView == null) {
                mMultiSelectActionBarView = LayoutInflater.from(CellBroadcastListActivity.this)
                        .inflate(R.layout.cell_broadcast_list_multi_select_actionbar, null);

                mTitle = (TextView) mMultiSelectActionBarView.findViewById(R.id.title);
            }
            mode.setCustomView(mMultiSelectActionBarView);
            ((TextView) mMultiSelectActionBarView.findViewById(R.id.title))
                    .setText(R.string.select_cellbroadcasts);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            if (mMultiSelectActionBarView == null) {
                ViewGroup v = (ViewGroup) LayoutInflater.from(CellBroadcastListActivity.this)
                        .inflate(R.layout.cell_broadcast_list_multi_select_actionbar, null);
                mode.setCustomView(v);
                mTitle = (TextView) mMultiSelectActionBarView.findViewById(R.id.title);
            }
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
                case R.id.select_all:
                    setAllItemChecked(true);
                    break;

                case R.id.cancel_select:
                    setAllItemChecked(false);
                    break;

                case R.id.delete:
                    if (mListView != null) {
                        CellBroadcastCursorAdapter adapter = (CellBroadcastCursorAdapter) mListView
                                .getAdapter();
                        if (adapter != null) {
                            confirmDeleteThread(false, adapter.getSelectedSet());
                        }
                    }
                    break;

                default:
                    break;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            CellBroadcastCursorAdapter adapter = (CellBroadcastCursorAdapter) mListView
                    .getAdapter();
            if (adapter != null) {
                adapter.clearSeleted();
            }
        }

        @Override
        public void onItemCheckedStateChanged(ActionMode mode, int position, long id,
                boolean checked) {
            mTitle.setText(R.string.select_cellbroadcasts);
            if (mListView != null) {
                CellBroadcastCursorAdapter adapter = (CellBroadcastCursorAdapter) mListView
                        .getAdapter();
                Log.d(TAG, "selected id = " + id);
                if (adapter != null) {
                    if (adapter.IsItemSelected(id)) {
                        adapter.removeSelectedId(id);
                    } else {
                        adapter.addSelectedId(id);
                    }
                    adapter.notifyDataSetChanged();
                }
                if (adapter.getSelectedCount() > 0) {
                    deleteItem.setEnabled(true);
                    unSelectAllItem.setEnabled(true);
                } else {
                    deleteItem.setEnabled(false);
                    unSelectAllItem.setEnabled(false);
                }
                mTitle.setText(CellBroadcastListActivity.this.getResources().getQuantityString(
                        R.plurals.message_view_selected_message_count, adapter.getSelectedCount(),
                        adapter.getSelectedCount()));
            }
        }

        private void setAllItemChecked(boolean checked) {
            if (mListView != null) {
                CellBroadcastCursorAdapter adapter = (CellBroadcastCursorAdapter) mListView
                        .getAdapter();
                for (int position = 0; position < mListView.getCount(); position++) {
                    Cursor cursor = (Cursor) mListView.getItemAtPosition(position);
                    if (cursor != null && cursor.getCount() > 0) {
                        long id = cursor.getLong(0);
                        if (adapter != null) {
                            if (checked) {
                                adapter.addSelectedId(id);
                            } else {
                                adapter.removeSelectedId(id);
                            }
                        }
                    }
                }

                if (checked) {
                    deleteItem.setEnabled(true);
                    unSelectAllItem.setEnabled(true);
                    selectAllItem.setEnabled(false);
                    mTitle.setText(CellBroadcastListActivity.this.getResources().getQuantityString(
                            R.plurals.message_view_selected_message_count,
                            adapter.getSelectedCount(), adapter.getSelectedCount()));
                } else {
                    deleteItem.setEnabled(false);
                    selectAllItem.setEnabled(true);
                    unSelectAllItem.setEnabled(false);
                    mTitle.setText(R.string.select_cellbroadcasts);
                }

                if (adapter != null) {
                    adapter.notifyDataSetChanged();
                }
            }
        }
    }

    private void dismissAllDialogShow() {
        Log.i(TAG, "dismissAllDialogShow");
        CMASAlertFullWindow cmasAlertFullWindow = CMASAlertFullWindow.getInstance(getApplicationContext());
        cmasAlertFullWindow.dismissAll();
    }

    /**
     * Start the process of putting up a dialog to confirm deleting a broadcast.
     * @param rowId the row ID of the broadcast to delete, or -1 to delete all broadcasts
     */
    public void confirmDeleteThread(boolean deleteAll,HashSet<Long> mSelectedBroadcastIds) {
        DeleteThreadListener listener = new DeleteThreadListener(deleteAll, mSelectedBroadcastIds);
        String message = new String();
        if (deleteAll) {
            message = getResources().getString(R.string.confirm_delete_all_broadcasts);
        } else {
            if (mSelectedBroadcastIds != null) {
                message = getResources().getQuantityString(R.plurals.confirm_delete_broadcast,
                        mSelectedBroadcastIds.size(), mSelectedBroadcastIds.size());
            }
        }
        confirmDeleteThreadDialog(listener, deleteAll, this, message);
    }

    /**
     * Build and show the proper delete broadcast dialog. The UI is slightly different
     * depending on whether there are locked messages in the thread(s) and whether we're
     * deleting a single broadcast or all broadcasts.
     * @param listener gets called when the delete button is pressed
     * @param deleteAll whether to show a single thread or all threads UI
     * @param context used to load the various UI elements
     */
    public void confirmDeleteThreadDialog(DeleteThreadListener listener,
            boolean deleteAll, Context context, String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setIconAttribute(android.R.attr.alertDialogIcon)
                .setCancelable(true)
                .setPositiveButton(R.string.button_delete, listener)
                .setNegativeButton(R.string.button_cancel, null)
                .setMessage(message)
                .show();
    }

    public class DeleteThreadListener implements OnClickListener {
        private boolean mDeleteAll;
        private HashSet<Long> mSelectedBroadcastIds;

        public DeleteThreadListener(boolean deleteAll, HashSet<Long> selectIds) {
            mDeleteAll = deleteAll;
            mSelectedBroadcastIds = selectIds;
        }

        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            // delete from database on a background thread
            new CellBroadcastContentProvider.AsyncCellBroadcastTask(
                    getContentResolver()).execute(
                    new CellBroadcastContentProvider.CellBroadcastOperation() {
                        @Override
                        public boolean execute(CellBroadcastContentProvider provider) {
                            if (!mDeleteAll) {
                                if (mSelectedBroadcastIds != null) {
                                    for (long rowId : mSelectedBroadcastIds) {
                                        provider.deleteBroadcast(rowId);
                                    }
                                }
                                return true;
                            } else {
                                return provider.deleteAllBroadcasts();
                            }
                        }
                    });
            dialog.dismiss();
        }
    }
}
