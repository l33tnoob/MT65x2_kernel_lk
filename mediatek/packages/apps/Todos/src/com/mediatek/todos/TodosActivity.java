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

import java.util.Locale;

import android.app.ActionBar;
import android.app.ActionBar.LayoutParams;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.PopupMenu.OnMenuItemClickListener;

public class TodosActivity extends Activity {
    private static final String TAG = "TodosActivity";

    private static final int REQUEST_ADD_NEW = 1;
    private static final int REQUEST_SHOW_DETAILS = REQUEST_ADD_NEW + 1;

    private static final int DIALOG_DELETE_ITEMS = 1;
    private boolean mShowingDialog = false;

    private TimeChangeReceiver mTimeChangeReceiver = null;

    /** display number of All Todos */
    private TextView mNumberTextView;
    private MenuItem mBtnChangeState;
    private MenuItem mBtnDelete;

    /** Read all Todo infos from QB */
    private TodosListAdapter mTodosListAdapter = null;
    /** Show all Todo infos in ListView */
    private ListView mTodosListView = null;

    /** Item click & long click listener */
    private AdapterViewListener mAdapterViewListener = null;
    
    /**
     * Action Mode
     */
    private ActionMode mActionMode = null;
    private TodosActionModeCallBack mTodosActionCallBack = new TodosActionModeCallBack();

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.items);
        initViews();
        configureActionBar();
        mTimeChangeReceiver = TimeChangeReceiver.registTimeChangeReceiver(this);
        mTimeChangeReceiver.addDateChangeListener(mTodosListAdapter);
        LogUtils.d(TAG, "TodosActivity.onCreate() finished.");
        
    }

    private void initViews() {
        mTodosListAdapter = new TodosListAdapter(this);

        mAdapterViewListener = new AdapterViewListener();
        mTodosListView = (ListView) findViewById(R.id.list_todos);
        mTodosListView.setAdapter(mTodosListAdapter);
        mTodosListView.setOnItemClickListener(mAdapterViewListener);
        mTodosListView.setOnItemLongClickListener(mAdapterViewListener);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        LogUtils.d(TAG, "onActivityResult request=" + requestCode + " result=" + resultCode);
        TodoInfo info = null;
        switch (resultCode) {
        case Utils.OPERATOR_INSERT:
            info = (TodoInfo) data.getSerializableExtra(Utils.KEY_PASSED_DATA);
            mTodosListAdapter.addItem(info);
            final int addPos = mTodosListAdapter.getItemPosition(info);
            mTodosListView.setSelection(addPos);
            break;
        case Utils.OPERATOR_UPDATE:
            info = (TodoInfo) data.getSerializableExtra(Utils.KEY_PASSED_DATA);
            if (null == info) {
                LogUtils.e(TAG, "result: OPERATOR_UPDATE, but data didn't contain the info");
                return;
            }
            mTodosListAdapter.updateItemData(info);
            final int updatePos = mTodosListAdapter.getItemPosition(info);
            mTodosListView.setSelection(updatePos);
            break;
        case Utils.OPERATOR_DELETE:
            info = (TodoInfo) data.getSerializableExtra(Utils.KEY_PASSED_DATA);
            if (null == info) {
                LogUtils.e(TAG, "result: OPERATOR_DELETE, but data didn't contain the info");
                return;
            }
            mTodosListAdapter.removeItem(info);
            break;
        default:
            break;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mShowingDialog) {
            showDialog(DIALOG_DELETE_ITEMS);
        }
    }

    @Override
    protected void onPause() {
        if (mShowingDialog) {
            removeDialog(DIALOG_DELETE_ITEMS);
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        mTimeChangeReceiver.clearChangeListener();
        unregisterReceiver(mTimeChangeReceiver);
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (mTodosListAdapter.isEditing()) {
            updateToEditNull();
            return;
        }
        super.onBackPressed();
    }

    private void updateToEditNull() {
        if (mActionMode != null) {
            mActionMode.finish();
        }
        invalidateOptionsMenu();
        mTodosListAdapter.setEditingType(TodosListAdapter.EDIT_NULL);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog dialog = null;
        switch (id) {
        case DIALOG_DELETE_ITEMS:
            dialog = new AlertDialog.Builder(this).setTitle(R.string.delete)
                    .setMessage(R.string.delete_selected_items)
                    .setIconAttribute(android.R.attr.alertDialogIcon).create();
            dialog.setButton(DialogInterface.BUTTON_NEGATIVE, getText(R.string.cancel),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mShowingDialog = false;
                        }
                    });
            dialog.setButton(DialogInterface.BUTTON_POSITIVE, getText(R.string.delete),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mTodosListAdapter.deleteSelectedItems();
                            updateToEditNull();
                            updateBottomBarWidgetState();
                            mShowingDialog = false;
                        }
                    });
            dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
                public void onCancel(DialogInterface dialog) {
                    mShowingDialog = false;
                }
            });
            return dialog;
        default:
            break;
        }
        return super.onCreateDialog(id);
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        super.onPrepareDialog(id, dialog);
        switch (id) {
        case DIALOG_DELETE_ITEMS:
            String msg = "";
            if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_TODOS) {
                if (mTodosListAdapter.getSeletedTodosNumber() > 1) {
                    msg = getString(R.string.delete_selected_items);
                } else {
                    msg = getString(R.string.delete_item);
                }
                ((AlertDialog) dialog).setMessage(msg);
            } else if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_DONES) {
                if (mTodosListAdapter.getSeletedDonesNumber() > 1) {
                    msg = getString(R.string.delete_selected_items);
                } else {
                    msg = getString(R.string.delete_item);
                }
                ((AlertDialog) dialog).setMessage(msg);
            }
            break;
        default:
            break;
        }
    }

    void updateBottomBarWidgetState() {
        LogUtils.d(TAG, "updateBottomBarWidgetState(), editing=" + mTodosListAdapter.isEditing());
        if (mTodosListAdapter.isEditing()) {
            int selectedNumber = 0;
            int dataSourceNumber = 0;
            if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_TODOS) {
                selectedNumber = mTodosListAdapter.getSeletedTodosNumber();
                dataSourceNumber = mTodosListAdapter.getTodosDataSource().size();
            } else if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_DONES) {
                selectedNumber = mTodosListAdapter.getSeletedDonesNumber();
                dataSourceNumber = mTodosListAdapter.getDonesDataSource().size();
            }

            LogUtils.d(TAG, "selectedNumber=" + selectedNumber + ", dataSourceNumber="
                    + dataSourceNumber);
            if (dataSourceNumber == 0) {
                updateToEditNull();
            } else {
                ///M: add to avoid this case: no item has been selected but the button
                // can be pressed {@
                if (selectedNumber > 0) {
                    mBtnDelete.setEnabled(true);
                    mBtnChangeState.setEnabled(true);
                } else {
                    mBtnDelete.setEnabled(false);
                    mBtnChangeState.setEnabled(false);
                }
                /// @}
                mTodosActionCallBack.updateActionMode();
            }
        }
    }

    private void onChangeItemStateClick() {
        String currentStatus = null;
        String targetStatus = null;
        if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_TODOS) {
            currentStatus = TodoInfo.STATUS_TODO;
            targetStatus = TodoInfo.STATUS_DONE;
        } else if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_DONES) {
            currentStatus = TodoInfo.STATUS_DONE;
            targetStatus = TodoInfo.STATUS_TODO;
        }
        mTodosListAdapter.updateSelectedStatus(currentStatus, targetStatus);
        updateToEditNull();
    }

    class AdapterViewListener implements AdapterView.OnItemClickListener,
            AdapterView.OnItemLongClickListener {
        public void onItemClick(AdapterView<?> adapterView, View view, int position, long rowId) {
            int viewType = mTodosListAdapter.getItemViewType(position);
            LogUtils.d(TAG, "onItemClick viewType =" + viewType + " position=" + position);

            boolean selectDoneItem = false;
            switch (viewType) {
            case TodosListAdapter.TYPE_TODOS_HEADER:
                mTodosListAdapter.setTodosExpand(!mTodosListAdapter.isTodosExpand());
                break;
            case TodosListAdapter.TYPE_TODOS_FOOTER:
                if (!mTodosListAdapter.isEditing()) {
                    Intent intentAdd = new Intent(TodosActivity.this, EditTodoActivity.class);
                    startActivityForResult(intentAdd, REQUEST_ADD_NEW);
                }
                break;
            case TodosListAdapter.TYPE_DONES_HEADER:
                mTodosListAdapter.setDonesExpand(!mTodosListAdapter.isDonesExPand());
                break;
            case TodosListAdapter.TYPE_DONES_ITEM:
                selectDoneItem = true;
            case TodosListAdapter.TYPE_TODOS_ITEM:
                if (mTodosListAdapter.isEditing()) {
                    if (mTodosListAdapter.getSelectedNum() == 0) {
                        // its status is re-select, switch the type.
                        int editType = selectDoneItem ? TodosListAdapter.EDIT_DONES
                                : TodosListAdapter.EDIT_TODOS;
                        mTodosListAdapter.setEditingType(editType);
                        //update change state icon
                        updateChangeStateIcon(editType);
                    }
                    final boolean todosEditAble = viewType == TodosListAdapter.TYPE_TODOS_ITEM
                            && mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_TODOS;
                    final boolean donesEditAble = viewType == TodosListAdapter.TYPE_DONES_ITEM
                            && mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_DONES;
                    if (todosEditAble || donesEditAble) {
                        Object tag = view.getTag();
                        if (tag != null && tag instanceof TodosListAdapter.ViewHolder) {
                            TodosListAdapter.ViewHolder holder = (TodosListAdapter.ViewHolder) tag;
                            final boolean checked = holder.mTodoInfoCheckBox.isChecked();
                            holder.mTodoInfoCheckBox.setChecked(!checked);
                            mTodosListAdapter.selectItem(position, !checked);
                        }
                    }
                    updateBottomBarWidgetState();
                    updateActionModeTitle();
                } else {
                    Intent intentDetails = new Intent(TodosActivity.this, EditTodoActivity.class);
                    TodoInfo info = (TodoInfo) adapterView.getAdapter().getItem(position);
                    intentDetails.putExtra(Utils.KEY_PASSED_DATA, info);
                    startActivityForResult(intentDetails, REQUEST_SHOW_DETAILS);
                }
                break;
            default:
                break;
            }
        }

        public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position,
                long rowId) {
            final int viewType = mTodosListAdapter.getItemViewType(position);
            int editType = TodosListAdapter.EDIT_NULL;
            switch (viewType) {
            case TodosListAdapter.TYPE_TODOS_ITEM:
                editType = TodosListAdapter.EDIT_TODOS;
                break;
            case TodosListAdapter.TYPE_DONES_ITEM:
                editType = TodosListAdapter.EDIT_DONES;
                break;
            default:
                LogUtils.d(TAG, "cann't switch viewType:" + viewType);
            }
            LogUtils.d(TAG, "onItemLongClick viewType =" + viewType + " position=" + position
                    + " editType=" + editType);
            if (mTodosListAdapter.isEditing() && editType != mTodosListAdapter.getEditType()
                    && getSelectedCount() != 0) {
                // In multi-select mode, not allow do the long click when item
                // is not in edit mode.
                return true;
            }
            if (editType != TodosListAdapter.EDIT_NULL
                    && editType != mTodosListAdapter.getEditType()) {
                if (mTodosListAdapter.setEditingType(editType)) {
                    // when first long click,start action mode
                    startTodoActionMode();
                }
                updateChangeStateIcon(editType);
            }
            if (editType != TodosListAdapter.EDIT_NULL) {
                Object tag = view.getTag();
                if (tag != null && tag instanceof TodosListAdapter.ViewHolder) {
                    TodosListAdapter.ViewHolder holder = (TodosListAdapter.ViewHolder) tag;
                    final boolean checked = holder.mTodoInfoCheckBox.isChecked();
                    holder.mTodoInfoCheckBox.setChecked(!checked);
                    mTodosListAdapter.selectItem(position, !checked);
                }
            }
            updateActionModeTitle();
            updateBottomBarWidgetState();
            return true;
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case R.id.btn_new_todo:
            Intent intent = new Intent(TodosActivity.this, EditTodoActivity.class);
            startActivityForResult(intent, REQUEST_ADD_NEW);
            break;
        default:
            break;
        }
        return true;
    }
    
    public void updateHeaderNumberText(String text) {
        mNumberTextView.setText(text);
    }

    /**
     * configure the action bar
     */
    private void configureActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayShowCustomEnabled(true);
        View actionBarNumberView = getLayoutInflater().inflate(R.layout.title_bar_number, null);
        actionBar.setCustomView(actionBarNumberView, new LayoutParams(Gravity.END));
        mNumberTextView = (TextView)actionBarNumberView.findViewById(R.id.number);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.list_default, menu);
        return true;
    }
    
    class TodosActionModeCallBack implements ActionMode.Callback {
        private Button mSelectionButton;
        private PopupMenu mSelectionMenu;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            mActionMode = mode;
            getMenuInflater().inflate(R.menu.list_edit, menu);
            mBtnChangeState = menu.findItem(R.id.btn_change_state);
            mBtnDelete = menu.findItem(R.id.btn_delete);

            startSelectionMode(mActionMode);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            updateActionModeTitle();
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.action_select_all:
                if (mTodosListAdapter.isSelectedAll()) {
                    mTodosListAdapter.allSelectAction(false);
                } else {
                    mTodosListAdapter.allSelectAction(true);
                }

                updateBottomBarWidgetState();
                break;
            case R.id.btn_change_state:
                onChangeItemStateClick();
                updateBottomBarWidgetState();
                break;
            case R.id.btn_delete:
                showDialog(DIALOG_DELETE_ITEMS);
                mShowingDialog = true;
                break;
            default:
            }
            updateActionModeTitle();
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            //close mode action & exit editing mode
            mActionMode = null;
            onBackPressed();
        }

        private void startSelectionMode(final ActionMode actionMode) {
            View customView = getLayoutInflater().inflate(R.layout.action_mode, null);
            actionMode.setCustomView(customView);
            mSelectionButton = (Button) customView.findViewById(R.id.selection_menu);
            mSelectionMenu = new PopupMenu(TodosActivity.this, mSelectionButton);
            mSelectionMenu.getMenuInflater().inflate(R.menu.selection, mSelectionMenu.getMenu());

            mSelectionMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    return onActionItemClicked(actionMode, item);
                }
            });
            mSelectionButton.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    updateSelectPopupMenu();
                    mSelectionMenu.show();
                }
            });
        }

        private void updateSelectPopupMenu() {
            MenuItem item = mSelectionMenu.getMenu().findItem(R.id.action_select_all);
            if (item != null) {
                if (mTodosListAdapter.isSelectedAll()) {
                    item.setChecked(true);
                    item.setTitle(R.string.unselect_all);
                } else {
                    item.setChecked(false);
                    item.setTitle(R.string.select_all);
                }
            }
        }

        public void updateActionMode() {
            int num = getSelectedCount();
            String str = getResources().getString(R.string.selected_count);
            str = String.format(Locale.US, str, num);
            mSelectionButton.setText(str);
            updateSelectPopupMenu();
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
        }
    }

    private int getSelectedCount() {
        int num = -1;
        if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_TODOS) {
            num = mTodosListAdapter.getSeletedTodosNumber();
        } else if (mTodosListAdapter.getEditType() == TodosListAdapter.EDIT_DONES) {
            num = mTodosListAdapter.getSeletedDonesNumber();
        } else {
            LogUtils.w(TAG, "mTodosListAdapter.getEditType():+mTodosListAdapter.getEditType()" + ",may be has error.");
        }
        return num;
    }

    private void updateActionModeTitle() {
        if (mActionMode != null) {
            int num = getSelectedCount();
            String str = getResources().getString(R.string.selected_count);
            str = String.format(str, num);
            mActionMode.setTitle(str);
        }
    }

    private void startTodoActionMode() {
        if (mActionMode == null) {
            mActionMode = startActionMode(mTodosActionCallBack);
        }
    }

    private void updateChangeStateIcon(int editType) {
        if (editType == TodosListAdapter.EDIT_TODOS) {
            // mark done if in to-do state
            mBtnChangeState.setIcon(R.drawable.todo_mark_done);
            mBtnChangeState.setTitle(R.string.mark_todo);// "Mark as Done"
        } else if (editType == TodosListAdapter.EDIT_DONES) {
            // mark to do if in done state
            mBtnChangeState.setIcon(R.drawable.todo_mark_todo);
            mBtnChangeState.setTitle(R.string.mark_done);// "Mark as ToDo"
        }
    }
}
