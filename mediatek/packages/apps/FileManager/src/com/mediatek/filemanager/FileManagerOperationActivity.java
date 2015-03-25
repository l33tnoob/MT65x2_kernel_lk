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

package com.mediatek.filemanager;

import android.app.ActionBar;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Configuration;
import android.net.Uri;
import android.nfc.NfcAdapter;
import android.nfc.NfcEvent;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.TextView;
import android.content.res.Resources;

import com.mediatek.filemanager.AlertDialogFragment.AlertDialogFragmentBuilder;
import com.mediatek.filemanager.AlertDialogFragment.ChoiceDialogFragment;
import com.mediatek.filemanager.AlertDialogFragment.ChoiceDialogFragmentBuilder;
import com.mediatek.filemanager.AlertDialogFragment.EditDialogFragmentBuilder;
import com.mediatek.filemanager.AlertDialogFragment.EditTextDialogFragment;
import com.mediatek.filemanager.AlertDialogFragment.EditTextDialogFragment.EditTextDoneListener;
import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;
import com.mediatek.filemanager.service.ProgressInfo;
import com.mediatek.filemanager.utils.DrmManager;
import com.mediatek.filemanager.utils.FileUtils;
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.OptionsUtils;
import com.mediatek.filemanager.utils.PDebug;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

/**
 * This is main activity in File manager.
 */
public class FileManagerOperationActivity extends AbsBaseActivity implements
        AdapterView.OnItemLongClickListener, NfcAdapter.CreateBeamUrisCallback {
    private static final String TAG = "FileManagerOperationActivity";

    public static final String RENAME_EXTENSION_DIALOG_TAG = "rename_extension_dialog_fragment_tag";
    public static final String RENAME_DIALOG_TAG = "rename_dialog_fragment_tag";
    public static final String DELETE_DIALOG_TAG = "delete_dialog_fragment_tag";
    public static final String FORBIDDEN_DIALOG_TAG = "forbidden_dialog_fragment_tag";
    public static final String INTENT_EXTRA_SELECT_PATH = "select_path";
    private static final String NEW_FILE_PATH_KEY = "new_file_path_key";
    private static final String SAVED_SELECTED_PATH_KEY = "saved_selected_path";
    private static final String CURRENT_VIEW_MODE_KEY = "view_mode_key";
    private static final String CURRENT_POSTION_KEY = "current_postion_key";
    private static final String CURRENT_TOP_KEY = "current_top_key";
    private static final String PREF_SORT_BY = "pref_sort_by";
    private static final String DETAIL_INFO_KEY = "detail_info_key";
    private static final String SAVED_SELECTED_TOP_KEY = "saved_selected_top_key";
    private static final String TXT_MIME_TYPE = "text/plain";
    
    //the max count of files can be share,if too lager,the Binder will failed.
    private static final int MAX_SHARE_FILES_COUNT = 2000;

    // private static final int BACKGROUND_COLOR = 0xff848284;

    private View mNavigationView = null;
    private boolean mIsConfigChanged = false;
    private int mOrientationConfig;
    private ActionMode mActionMode;
    public final ActionModeCallBack mActionModeCallBack = new ActionModeCallBack();
    private NfcAdapter mNfcAdapter;
    private FileInfo mTxtFile = null;

    @Override
    public void onEjected(String unMountPoint) {
        super.onEjected(unMountPoint);
    }

    public ActionMode getActionMode() {
        return mActionMode;
    }

    @Override
    public void onUnMounted(String unMountPoint) {
        LogUtils.d(TAG, "onUnMounted,unMountPoint :" + unMountPoint);
        if (mCurrentPath.startsWith(unMountPoint) || mMountPointManager.isRootPath(mCurrentPath)) {

            if (mAdapter != null && mAdapter.getMode() == FileInfoAdapter.MODE_EDIT
                    && mActionMode != null) {
                mActionMode.finish();
            }

            ProgressDialogFragment pf = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(HeavyOperationListener.HEAVY_DIALOG_TAG);
            if (pf != null) {
                pf.dismissAllowingStateLoss();
            }

            // Restore the detail_dialog
            AlertDialogFragment af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(
                    DetailInfoListener.DETAIL_DIALOG_TAG);

            if (af != null) {
                af.dismissAllowingStateLoss();
            }

            // restore delete dialog
            af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(DELETE_DIALOG_TAG);
            if (af != null) {
                af.dismissAllowingStateLoss();
            }

            af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(
                    RENAME_EXTENSION_DIALOG_TAG);
            if (af != null) {
                af.dismissAllowingStateLoss();
            }

            af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(FORBIDDEN_DIALOG_TAG);
            if (af != null) {
                af.dismissAllowingStateLoss();
            }

            ChoiceDialogFragment sortDialogFragment = (ChoiceDialogFragment) getFragmentManager()
                    .findFragmentByTag(ChoiceDialogFragment.CHOICE_DIALOG_TAG);
            if (sortDialogFragment != null) {
                sortDialogFragment.dismissAllowingStateLoss();
            }

            EditTextDialogFragment renameDialogFragment = (EditTextDialogFragment) getFragmentManager()
                    .findFragmentByTag(RENAME_DIALOG_TAG);
            if (renameDialogFragment != null) {
                renameDialogFragment.dismissAllowingStateLoss();
            }
        }

        super.onUnMounted(unMountPoint);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PDebug.Start("FileManagerOperationActivity -- onCreate");

        LogUtils.d(TAG, "onCreate()");
        // get sort by
        mSortType = getPrefsSortBy();
        mOrientationConfig = this.getResources().getConfiguration().orientation;

        mNfcAdapter = NfcAdapter.getDefaultAdapter(this);
        if (mNfcAdapter == null) {
            LogUtils.w(TAG, "mNfcAdapter == null");
        } else if (OptionsUtils.isMtkBeamSurpported()) {
            mNfcAdapter.setMtkBeamPushUrisCallback(this, this);
        }
        PDebug.End("FileManagerOperationActivity -- onCreate");
    }

    @Override		
    protected void onResume() {
        if (mTxtFile != null) {
           if (mTxtFile.getFileLastModifiedTime() != mTxtFile.getNewModifiedTime()) {
               mTxtFile.updateFileInfo();
           }
           mTxtFile = null;
        }
        super.onResume();
    }

    @Override
    public Uri[] createBeamUris(NfcEvent event) {
        LogUtils.d(TAG, "Call createBeamUris() in FileManagerOperationActivity.");

        if (!OptionsUtils.isMtkBeamSurpported()) {
            LogUtils.d(TAG, "MtkBeam is not surpport!");
            return null;
        }

        if (!mAdapter.isMode(FileInfoAdapter.MODE_EDIT)) {
            LogUtils.d(TAG, "current mode is not Edit Mode.");
            return null;
        }
        if (mAdapter.getCheckedItemsCount() == 0) {
            LogUtils.d(TAG, "Edit Mode; select count == 0.");
            return null;
        }

        List<FileInfo> fileInfos = null;
        List<Uri> sendFiles = new ArrayList<Uri>();
        fileInfos = mAdapter.getCheckedFileInfoItemsList();
        for (FileInfo fileInfo : fileInfos) { // check if any folder is selected
            if (fileInfo.isDirectory()) {
                showForbiddenDialog(R.string.folder_beam_forbidden_title,
                        R.string.folder_beam_forbidden_message);
                return null;
            }
        }
        for (FileInfo fileInfo : fileInfos) { // check if any Drm file is
            // selected
            if (fileInfo.isDrmFile()
                    && DrmManager.getInstance().isRightsStatus(fileInfo.getFileAbsolutePath())
                    || !fileInfo.getFile().canRead()) {
                showForbiddenDialog(R.string.drm_beam_forbidden_title,
                        R.string.drm_beam_forbidden_message);
                return null;
            }
            sendFiles.add(fileInfo.getUri());
        }
        LogUtils.d(TAG, "The number of sending files is: " + sendFiles.size());
        Uri[] uris = new Uri[sendFiles.size()];
        sendFiles.toArray(uris);
        return uris;
    }

    protected void showForbiddenDialog(int title, int message) {
        LogUtils.d(TAG, "show ForbiddenDialog...");
        if (mIsAlertDialogShowing) {
            LogUtils.d(TAG, "Another Dialog is exist, return!~~");
            return;
        }
        mIsAlertDialogShowing = true;

        AlertDialogFragment forbiddenDialogFragment = (AlertDialogFragment) getFragmentManager()
                .findFragmentByTag(FORBIDDEN_DIALOG_TAG);
        if (forbiddenDialogFragment != null) {
            forbiddenDialogFragment.dismissAllowingStateLoss();
        }
        AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
        forbiddenDialogFragment = builder.setTitle(title).setIcon(
                R.drawable.ic_dialog_alert_holo_light).setMessage(message).setCancelable(false)
                .setCancelTitle(R.string.ok).create();
        forbiddenDialogFragment.setOnDialogDismissListener(this);
        forbiddenDialogFragment.show(getFragmentManager(), FORBIDDEN_DIALOG_TAG);
        boolean ret = getFragmentManager().executePendingTransactions();
        LogUtils.d(TAG, "executing pending transactions result: " + ret);
    }

    @Override
    protected void serviceConnected() {
        LogUtils.d(TAG, "serviceConnected...");
        super.serviceConnected();
        if (mSavedInstanceState != null) {
            int mode = mSavedInstanceState.getInt(CURRENT_VIEW_MODE_KEY,
                    FileInfoAdapter.MODE_NORMAL);
            int position = mSavedInstanceState.getInt(CURRENT_POSTION_KEY, 0);
            int top = mSavedInstanceState.getInt(CURRENT_TOP_KEY, -1);
            LogUtils.d(TAG, "serviceConnected mode=" + mode);
            restoreViewMode(mode, position, top);
        }
        mListView.setOnItemLongClickListener(this);
    }

    private void restoreViewMode(int mode, int position, int top) {
        if (mode == FileInfoAdapter.MODE_EDIT) {
            mListView.setFastScrollEnabled(false);
            mAdapter.changeMode(mode);
            mActionMode = startActionMode(mActionModeCallBack);
            mActionModeCallBack.updateActionMode();
            String saveSelectedPath = mSavedInstanceState.getString(SAVED_SELECTED_PATH_KEY);
            if (saveSelectedPath != null && !saveSelectedPath.equals("")) {
                mSelectedFileInfo = new FileInfo(saveSelectedPath);
                mSelectedTop = mSavedInstanceState.getInt(SAVED_SELECTED_TOP_KEY);
            }
        } else {
            mNavigationView.setVisibility(View.VISIBLE);
            mListView.setFastScrollEnabled(true);
            mAdapter.changeMode(FileInfoAdapter.MODE_NORMAL);
            invalidateOptionsMenu();
        }
        mListView.setSelectionFromTop(position, top);
    }

    protected void restoreDialog() {
        // Restore the heavy_dialog : pasting deleting
        ProgressDialogFragment pf = (ProgressDialogFragment) getFragmentManager()
                .findFragmentByTag(HeavyOperationListener.HEAVY_DIALOG_TAG);
        if (pf != null) {
            if (!mService.isBusy(this.getClass().getName())) {
                pf.dismissAllowingStateLoss();
            } else {
                HeavyOperationListener listener = new HeavyOperationListener(
                        AlertDialogFragment.INVIND_RES_ID);
                mService.reconnected(this.getClass().getName(), listener);
                pf.setCancelListener(listener);
            }
        }

        String saveSelectedPath = mSavedInstanceState.getString(SAVED_SELECTED_PATH_KEY);
        FileInfo saveSelectedFile = null;
        if (saveSelectedPath != null) {
            saveSelectedFile = new FileInfo(saveSelectedPath);
        }

        // Restore the detail_dialog
        AlertDialogFragment af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(
                DetailInfoListener.DETAIL_DIALOG_TAG);

        if (af != null && saveSelectedFile != null && mService != null) {
            DetailInfoListener listener = new DetailInfoListener(saveSelectedFile);
            af.setDismissListener(listener);
            String savedDetailInfo = af.getArguments().getString(DETAIL_INFO_KEY);
            if (mService.isBusy(this.getClass().getName()) && mService.isDetailTask(this.getClass().getName())) {
                mService.reconnected(this.getClass().getName(), listener);
            } else if(savedDetailInfo != null && !savedDetailInfo.equals("")){
                TextView mDetailsText = (TextView) af.getDialog().findViewById(R.id.details_text);
                if (mDetailsText != null) {
                    mDetailsText.setText(savedDetailInfo);
                }
            } else if(!mService.isBusy(this.getClass().getName())) {
                af.dismissAllowingStateLoss();
                mService.getDetailInfo(this.getClass().getName(), saveSelectedFile, listener);
            } else {
                af.dismissAllowingStateLoss();
            }
        } else if(af != null && saveSelectedFile == null){
            af.dismissAllowingStateLoss();
            mIsAlertDialogShowing = false;
        }

        // restore delete dialog
        af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(DELETE_DIALOG_TAG);
        if (af != null) {
            af.setOnDoneListener(new DeleteListener());
        }
        // rename Ext Dialog.
        af = (AlertDialogFragment) getFragmentManager().findFragmentByTag(
                RENAME_EXTENSION_DIALOG_TAG);
        if (af != null) {
            String newFilePath = af.getArguments().getString(NEW_FILE_PATH_KEY);
            if (newFilePath != null && saveSelectedFile != null) {
                af.setOnDoneListener(new RenameExtensionListener(saveSelectedFile, newFilePath));
            }
        }

        ChoiceDialogFragment sortDialogFragment = (ChoiceDialogFragment) getFragmentManager()
                .findFragmentByTag(ChoiceDialogFragment.CHOICE_DIALOG_TAG);
        if (sortDialogFragment != null) {
            sortDialogFragment.setItemClickListener(new SortClickListner());
        }

        EditTextDialogFragment renameDialogFragment = (EditTextDialogFragment) getFragmentManager()
                .findFragmentByTag(RENAME_DIALOG_TAG);
        if (renameDialogFragment != null && saveSelectedFile != null) {
            renameDialogFragment
                    .setOnEditTextDoneListener(new RenameDoneListener(saveSelectedFile));
        }
        super.restoreDialog();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if (mAdapter != null && mAdapter.getCheckedItemsCount() == 1) {
            FileInfo selectFileInfo = mAdapter.getCheckedFileInfoItemsList().get(0);
            if (selectFileInfo != null) {
                outState.putString(SAVED_SELECTED_PATH_KEY, selectFileInfo.getFileAbsolutePath());
                int pos = mAdapter.getPosition(selectFileInfo);
                LogUtils.d(TAG, "onSaveInstanceSteate selected pos: " + pos);
                View view = mListView.getChildAt(pos);
                int top = -1;
                if (view != null) {
                    top = view.getTop();
                }
                outState.putInt(SAVED_SELECTED_TOP_KEY,top);
            }
        }
        int currentMode = (mAdapter != null) ? mAdapter.getMode() : FileInfoAdapter.MODE_NORMAL;
        outState.putInt(CURRENT_VIEW_MODE_KEY, currentMode);
        if (mListView.getChildCount() > 0) {
            View view = mListView.getChildAt(0);
            int position = (mListView.getPositionForView(view));
            int top = view.getTop();
            outState.putInt(CURRENT_POSTION_KEY, position);
            outState.putInt(CURRENT_TOP_KEY, top);
        }
    }

    @Override
    protected void setMainContentView() {
        setContentView(R.layout.main);

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View customActionBarView = inflater.inflate(R.layout.actionbar, null);

            actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                    ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_SHOW_HOME
                            | ActionBar.DISPLAY_SHOW_TITLE);

            mNavigationView = customActionBarView.findViewById(R.id.bar_background);

            actionBar.setCustomView(customActionBarView);
            actionBar.setBackgroundDrawable(getResources().getDrawable(R.drawable.title_bar_bg));
            actionBar.setSplitBackgroundDrawable(getResources().getDrawable(
                    R.drawable.actionbar_background));
        }
    }

    /**
     * This method switches edit view to navigation view
     */
    private void switchToNavigationView() {
        LogUtils.d(TAG, "Switch to navigation view");
        mNavigationView.setVisibility(View.VISIBLE);
        mListView.setFastScrollEnabled(true);

        mAdapter.changeMode(FileInfoAdapter.MODE_NORMAL);
        invalidateOptionsMenu();
    }

    private void switchToEditView(int position, int top) {
        LogUtils.d(TAG, "switchToEditView position and top" + position + "/" + top);
        mAdapter.setChecked(position, true);
        mListView.setSelectionFromTop(position, top);
        switchToEditView();
    }

    private void switchToEditView() {
        LogUtils.d(TAG, "Switch to edit view");
        mListView.setFastScrollEnabled(false);
        mAdapter.changeMode(FileInfoAdapter.MODE_EDIT);
        mActionMode = startActionMode(mActionModeCallBack);
        mActionModeCallBack.updateActionMode();
    }

    /**
     * The method shares the files/folders MMS: support only single files BT:
     * support single and multiple files
     */
    private void share() {
        Intent intent;
        boolean forbidden = false;
        List<FileInfo> files = null;
        ArrayList<Parcelable> sendList = new ArrayList<Parcelable>();

        if (mAdapter.isMode(FileInfoAdapter.MODE_EDIT)) {
            files = mAdapter.getCheckedFileInfoItemsList();
        } else {
            LogUtils.w(TAG, "Maybe dispatch events twice, view mode error.");
            return;
        }

        if (files.size() > 1) {
            // send multiple files
            LogUtils.d(TAG, "Share multiple files");
            for (FileInfo info : files) {
                if (info.isDrmFile()
                        && DrmManager.getInstance().isRightsStatus(info.getFileAbsolutePath())) {
                    forbidden = true;
                    break;
                }

                sendList.add(info.getUri());
            }

            if (!forbidden) {
                intent = new Intent();
                intent.setAction(Intent.ACTION_SEND_MULTIPLE);
                intent.setType(FileUtils.getMultipleMimeType(mService, mCurrentPath, files));
                intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, sendList);

                try {
                    startActivity(Intent.createChooser(intent, getString(R.string.send_file)));
                } catch (android.content.ActivityNotFoundException e) {
                    LogUtils.e(TAG, "Cannot find any activity", e);
                    // TODO add a toast to notify user; get a function from here
                    // and if(!forbidden)
                    // below
                }
            }
        } else {
            // send single file
            LogUtils.d(TAG, "Share a single file");
            FileInfo fileInfo = files.get(0);
            String mimeType = fileInfo.getFileMimeType(mService);

            if (fileInfo.isDrmFile()
                    && DrmManager.getInstance().isRightsStatus(fileInfo.getFileAbsolutePath())) {
                forbidden = true;
            }

            if (mimeType == null || mimeType.startsWith("unknown")) {
                mimeType = FileInfo.MIMETYPE_UNRECOGNIZED;
            }

            if (!forbidden) {
                intent = new Intent();
                intent.setAction(Intent.ACTION_SEND);
                intent.setType(mimeType);
                Uri uri = Uri.fromFile(fileInfo.getFile());
                intent.putExtra(Intent.EXTRA_STREAM, uri);
                LogUtils.d(TAG, "Share Uri file: " + uri);
                LogUtils.d(TAG, "Share file mimetype: " + mimeType);

                try {
                    startActivity(Intent.createChooser(intent, getString(R.string.send_file)));
                } catch (android.content.ActivityNotFoundException e) {
                    LogUtils.e(TAG, "Cannot find any activity", e);
                    // TODO add a toast to notify user
                }
            }
        }

        if (forbidden) {
            showForbiddenDialog(com.mediatek.internal.R.string.drm_forwardforbidden_title,
                    com.mediatek.internal.R.string.drm_forwardforbidden_message);
        } else {
            if (mActionMode != null) {
                mActionMode.finish();
            }
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        LogUtils.d(TAG, "onItemClick, position = " + position);
        if (mService != null && mService.isBusy(this.getClass().getName())) {
            LogUtils.d(TAG, "onItemClick, service is busy,return. ");
            return;
        }
        if (mAdapter.isMode(FileInfoAdapter.MODE_NORMAL)) {
            LogUtils.d(TAG, "onItemClick,Selected position: " + position);

            if (position >= mAdapter.getCount() || position < 0) {
                LogUtils.e(TAG, "onItemClick,events error,mFileInfoList.size(): "
                        + mAdapter.getCount());
                return;
            }
            FileInfo selecteItemFileInfo = (FileInfo) mAdapter.getItem(position);

            if (selecteItemFileInfo.isDirectory()) {
                int top = view.getTop();
                LogUtils.v(TAG, "onItemClick,fromTop = " + top);
                addToNavigationList(mCurrentPath, selecteItemFileInfo, top);
                showDirectoryContent(selecteItemFileInfo.getFileAbsolutePath());
            } else {
                // open file here
                boolean canOpen = true;
                String mimeType = selecteItemFileInfo.getFileMimeType(mService);

                if (selecteItemFileInfo.isDrmFile()) {
                    mimeType = DrmManager.getInstance().getOriginalMimeType(
                            selecteItemFileInfo.getFileAbsolutePath());

                    if (TextUtils.isEmpty(mimeType)) {
                        canOpen = false;
                        mToastHelper.showToast(R.string.msg_unable_open_file);
                    }
                }

                if (canOpen) {
                    Intent intent = new Intent(Intent.ACTION_VIEW);
                    Uri uri = selecteItemFileInfo.getUri();
                    LogUtils.d(TAG, "onItemClick,Open uri file: " + uri);
                    intent.setDataAndType(uri, mimeType);
                    if (mimeType != null && mimeType.equals(TXT_MIME_TYPE)) {
                        mTxtFile = selecteItemFileInfo;
                    }

                    try {
                        startActivity(intent);
                    } catch (android.content.ActivityNotFoundException e) {
                        mTxtFile = null;
                        mToastHelper.showToast(R.string.msg_unable_open_file);
                        LogUtils.w(TAG, "onItemClick,Cannot open file: "
                                + selecteItemFileInfo.getFileAbsolutePath());
                    }
                }
            }
        } else {
            LogUtils.d(TAG, "onItemClick,edit view .");
            boolean state = mAdapter.getItem(position).isChecked();
            mAdapter.setChecked(position, !state);
            mActionModeCallBack.updateActionMode();
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onClick(View view) {
        if (mService.isBusy(this.getClass().getName())) {
            LogUtils.d(TAG, "onClick, service is busy,return.");
            return;
        }
        int id = view.getId();
        LogUtils.d(TAG, "onClick,id: " + id);

        boolean isMounted = mMountPointManager.isRootPathMount(mCurrentPath);
        if (mAdapter.isMode(FileInfoAdapter.MODE_EDIT) && isMounted) {
            mActionModeCallBack.updateActionMode();
            LogUtils.d(TAG, "onClick,retuen.");
            return;
        }
        super.onClick(view);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        LogUtils.d(TAG, "onCreateOptionsMenu...");
        super.onCreateOptionsMenu(menu);

        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (mService == null) {
            LogUtils.i(TAG, "onCreateOptionsMenu, invalid service,return true.");
            return true;
        }
        inflater.inflate(R.menu.navigation_view_menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        LogUtils.d(TAG, "onPrepareOptionsMenu...");
        if (mCurrentPath != null && mMountPointManager.isRootPath(mCurrentPath)) {
            menu.findItem(R.id.create_folder).setEnabled(false);
            menu.findItem(R.id.paste).setVisible(false);
            menu.findItem(R.id.paste).setEnabled(false);
            menu.findItem(R.id.search).setEnabled(true);
            // more items
            menu.findItem(R.id.change_mode).setEnabled(false);
            if (getPrefsShowHidenFile()) {
                menu.findItem(R.id.hide).setTitle(R.string.hide_file);
            } else {
                menu.findItem(R.id.hide).setTitle(R.string.show_file);
            }
            menu.findItem(R.id.sort).setEnabled(true);

            return true;
        }
        if (mFileInfoManager.getPasteCount() > 0) {
            menu.findItem(R.id.paste).setVisible(true);
            menu.findItem(R.id.paste).setEnabled(true);
        } else {
            menu.findItem(R.id.paste).setVisible(false);
            menu.findItem(R.id.paste).setEnabled(false);
        }
        if (mCurrentPath != null && !(new File(mCurrentPath)).canWrite()) {
            menu.findItem(R.id.create_folder).setEnabled(false);
            menu.findItem(R.id.paste).setVisible(false);
        } else {
            menu.findItem(R.id.create_folder).setEnabled(true);
        }

        if (mAdapter.getCount() == 0) {
            menu.findItem(R.id.search).setEnabled(false);
        } else {
            menu.findItem(R.id.search).setEnabled(true);
        }
        // more items
        if (getPrefsShowHidenFile()) {
            menu.findItem(R.id.hide).setTitle(R.string.hide_file);
        } else {
            menu.findItem(R.id.hide).setTitle(R.string.show_file);
        }
        if ((mAdapter.getCount() == 0)
                || (mCurrentPath != null && mMountPointManager.isRootPath(mCurrentPath))) {
            menu.findItem(R.id.change_mode).setEnabled(false);
        } else {
            menu.findItem(R.id.change_mode).setEnabled(true);
        }

        if (mActionMode != null && mActionModeCallBack != null) {
            mActionModeCallBack.updateActionMode();
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        LogUtils.d(TAG, "onOptionsItemSelected: " + item.getItemId());

        if (mService != null && mService.isBusy(this.getClass().getName())) {
            LogUtils.i(TAG, "onOptionsItemSelected,service is busy. ");
            return true;
        }

        switch (item.getItemId()) {
        case R.id.create_folder:
            showCreateFolderDialog();
            break;
        case R.id.search:
            Intent intent = new Intent();
            intent.setClass(this, FileManagerSearchActivity.class);
            intent.putExtra(FileManagerSearchActivity.CURRENT_PATH, mCurrentPath);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            break;
        case R.id.paste:
            if (mService != null) {
                mService.pasteFiles(this.getClass().getName(), mFileInfoManager.getPasteList(),
                        mCurrentPath, mFileInfoManager.getPasteType(), new HeavyOperationListener(
                                R.string.pasting));
            }
            break;
        case R.id.sort:
            showSortDialog();
            break;
        case R.id.hide:
            if (mService != null) {
                mService.setListType(
                        changePrefsShowHidenFile() ? FileManagerService.FILE_FILTER_TYPE_DEFAULT
                                : FileManagerService.FILE_FILTER_TYPE_ALL, this.getClass()
                                .getName());
                mService.listFiles(this.getClass().getName(), mCurrentPath, new ListListener());
            }
            break;
        case R.id.change_mode:
            switchToEditView();
            break;
        default:
            return super.onOptionsItemSelected(item);
        }
        return true;
    }

    /**
     * This method switches edit view to navigation view
     * 
     * @param refresh
     *            whether to refresh the screen after the switch is done
     */
    private void sortFileInfoList() {
        LogUtils.d(TAG, "Start sortFileInfoList()");

        int selection = mListView.getFirstVisiblePosition(); // save current
        // visible position

        // refresh only when paste or delete operation is performed
        mFileInfoManager.sort(mSortType);
        mAdapter.notifyDataSetChanged();
        mListView.setSelection(selection);
        // restore the selection in the navigation view

        LogUtils.d(TAG, "End sortFileInfoList()");
    }

    /**
     * This method sets the sorting type in the preference
     * 
     * @param sort
     *            the sorting type
     */
    private void setPrefsSortBy(int sort) {
        mSortType = sort;
        Editor editor = getPreferences(MODE_PRIVATE).edit();
        editor.putInt(PREF_SORT_BY, sort);
        editor.commit();
    }

    /**
     * This method gets the sorting type from the preference
     * 
     * @return the sorting type
     */
    private int getPrefsSortBy() {
        SharedPreferences prefs = getPreferences(MODE_PRIVATE);
        return prefs.getInt(PREF_SORT_BY, 0);
    }

    protected void showDeleteDialog() {
        LogUtils.d(TAG, "show DeleteDialog...");
        if (mIsAlertDialogShowing) {
            LogUtils.d(TAG, "Another Dialog is exist, return!~~");
            return;
        }
        mIsAlertDialogShowing = true;
        int alertMsgId = R.string.alert_delete_multiple;
        if (mAdapter.getCheckedItemsCount() == 1) {
            alertMsgId = R.string.alert_delete_single;
        } else {
            alertMsgId = R.string.alert_delete_multiple;
        }

        AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
        AlertDialogFragment deleteDialogFragment = builder.setMessage(alertMsgId).setDoneTitle(
                R.string.ok).setCancelTitle(R.string.cancel).setIcon(
                R.drawable.ic_dialog_alert_holo_light).setTitle(R.string.delete).create();
        deleteDialogFragment.setOnDoneListener(new DeleteListener());
        deleteDialogFragment.setOnDialogDismissListener(this);
        deleteDialogFragment.show(getFragmentManager(), DELETE_DIALOG_TAG);
        boolean ret = getFragmentManager().executePendingTransactions();
        LogUtils.d(TAG, "executing pending transactions result: " + ret);
    }

    private class DeleteListener implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int id) {
            LogUtils.d(TAG, "onClick() method for alertDeleteDialog, OK button");
            if (mService != null) {
                mService.deleteFiles(FileManagerOperationActivity.this.getClass().getName(),
                        mAdapter.getCheckedFileInfoItemsList(), new HeavyOperationListener(
                                R.string.deleting));
            }
            if (mActionMode != null) {
                mActionMode.finish();
            }
        }
    }

    /**
     * The method creates an alert delete dialog
     * 
     * @param args
     *            argument, the boolean value who will indicates whether the
     *            selected files just only one. The prompt message will be
     *            different.
     * @return a dialog
     */
    protected void showRenameExtensionDialog(FileInfo srcfileInfo, final String newFilePath) {
        LogUtils.d(TAG, "show RenameExtensionDialog...");

        AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
        AlertDialogFragment renameExtensionDialogFragment = builder.setTitle(
                R.string.confirm_rename).setIcon(R.drawable.ic_dialog_alert_holo_light).setMessage(
                R.string.msg_rename_ext).setCancelTitle(R.string.cancel).setDoneTitle(R.string.ok)
                .create();
        renameExtensionDialogFragment.getArguments().putString(NEW_FILE_PATH_KEY, newFilePath);
        renameExtensionDialogFragment.setOnDoneListener(new RenameExtensionListener(srcfileInfo,
                newFilePath));
        renameExtensionDialogFragment.show(getFragmentManager(), RENAME_EXTENSION_DIALOG_TAG);
        boolean ret = getFragmentManager().executePendingTransactions();
        LogUtils.d(TAG, "executing pending transactions result: " + ret);
    }

    private class RenameExtensionListener implements OnClickListener {
        private final String mNewFilePath;
        private final FileInfo mSrcFile;

        public RenameExtensionListener(FileInfo fileInfo, String newFilePath) {
            mNewFilePath = newFilePath;
            mSrcFile = fileInfo;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mService != null) {
                if (mActionMode != null) {
                    mActionMode.finish();
                }
                mService.rename(FileManagerOperationActivity.this.getClass().getName(), mSrcFile,
                        new FileInfo(mNewFilePath), new LightOperationListener(FileUtils
                                .getFileName(mNewFilePath)));
            }
        }

    }

    /**
     * The method creates an alert sort dialog
     * 
     * @return a dialog
     */
    protected void showSortDialog() {
        LogUtils.d(TAG, "show SortDialog...");
        if (mIsAlertDialogShowing) {
            LogUtils.d(TAG, "Another Dialog is exist, return!~~");
            return;
        }
        mIsAlertDialogShowing = true;
        ChoiceDialogFragmentBuilder builder = new ChoiceDialogFragmentBuilder();
        builder.setDefault(R.array.sort_by, mSortType).setTitle(R.string.sort_by).setCancelTitle(
                R.string.cancel);
        ChoiceDialogFragment sortDialogFragment = builder.create();
        sortDialogFragment.setItemClickListener(new SortClickListner());
        sortDialogFragment.setOnDialogDismissListener(this);
        sortDialogFragment.show(getFragmentManager(), ChoiceDialogFragment.CHOICE_DIALOG_TAG);
        boolean ret = getFragmentManager().executePendingTransactions();
        LogUtils.d(TAG, "executing pending transactions result: " + ret);
    }

    private class SortClickListner implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int id) {
            if (id != mSortType) {
                setPrefsSortBy(id);
                dialog.dismiss();
                sortFileInfoList();
            }
        }

    }

    protected void showRenameDialog() {
        LogUtils.d(TAG, "show RenameDialog...");
        if (mIsAlertDialogShowing) {
            LogUtils.d(TAG, "Another Dialog showing, return!~~");
            return;
        }
        mIsAlertDialogShowing = true;
        FileInfo fileInfo = mAdapter.getFirstCheckedFileInfoItem();
        int selection = 0;
        if (fileInfo != null) {
            String name = fileInfo.getFileName();
            String fileExtension = FileUtils.getFileExtension(name);
            selection = name.length();
            if (!fileInfo.isDirectory() && fileExtension != null) {
                selection = selection - fileExtension.length() - 1;
            }
            EditDialogFragmentBuilder builder = new EditDialogFragmentBuilder();
            builder.setDefault(name, selection).setDoneTitle(R.string.done).setCancelTitle(
                    R.string.cancel).setTitle(R.string.rename);
            EditTextDialogFragment renameDialogFragment = builder.create();
            renameDialogFragment.setOnEditTextDoneListener(new RenameDoneListener(fileInfo));
            renameDialogFragment.setOnDialogDismissListener(this);
            renameDialogFragment.show(getFragmentManager(), RENAME_DIALOG_TAG);
            boolean ret = getFragmentManager().executePendingTransactions();
            LogUtils.d(TAG, "executing pending transactions result: " + ret);
        }
    }

    protected class RenameDoneListener implements EditTextDoneListener {
        FileInfo mSrcfileInfo;

        public RenameDoneListener(FileInfo srcFile) {
            mSrcfileInfo = srcFile;
        }

        @Override
        public void onClick(String text) {
            String newFilePath = mCurrentPath + MountPointManager.SEPARATOR + text;
            if (null == mSrcfileInfo) {
                LogUtils.w(TAG, "mSrcfileInfo is null.");
                return;
            }
            if (FileUtils.isExtensionChange(newFilePath, mSrcfileInfo.getFileAbsolutePath())) {
                showRenameExtensionDialog(mSrcfileInfo, newFilePath);
            } else {
                if (mService != null) {
                    if (mActionMode != null) {
                        mActionMode.finish();
                    }
                    mService.rename(FileManagerOperationActivity.this.getClass().getName(),
                            mSrcfileInfo, new FileInfo(newFilePath), new LightOperationListener(
                                    FileUtils.getFileName(newFilePath)));
                }
            }

        }
    }

    @Override
    public void onBackPressed() {
        if (mAdapter != null && mAdapter.isMode(FileInfoAdapter.MODE_EDIT)) {
            if (mActionMode != null) {
                mActionMode.finish();
            }
            return;
        }
        super.onBackPressed();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        String path = intent.getStringExtra(INTENT_EXTRA_SELECT_PATH);
        if (path != null && mService != null && !mService.isBusy(this.getClass().getName())) {
            File file = new File(path);
            if (!file.exists()) {
                mToastHelper.showToast(getString(R.string.path_not_exists, path));
                path = mMountPointManager.getRootPath();
            }
            addToNavigationList(mCurrentPath, null, -1);
            showDirectoryContent(path);
        }
    }

    @Override
    protected String initCurrentFileInfo() {
        String path = getIntent().getStringExtra(INTENT_EXTRA_SELECT_PATH);
        if (path != null) {
            File file = new File(path);
            if (file.exists()) {
                return path;
            }
            mToastHelper.showToast(getString(R.string.path_not_exists, path));
        }
        return mMountPointManager.getRootPath();
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> adapter, View v, int position, long id) {
        if (mAdapter.isMode(FileInfoAdapter.MODE_NORMAL)) {
            if (!mMountPointManager.isRootPath(mCurrentPath)
                    && !mService.isBusy(this.getClass().getName())) {
                int top = v.getTop();
                switchToEditView(position, top);
                return true;
            }
        }
        return false;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation != mOrientationConfig) {
            mIsConfigChanged = true;
            mOrientationConfig = newConfig.orientation;
        }
    }

    @Override
    protected void onPathChanged() {
        super.onPathChanged();
        if (mActionMode != null && mActionModeCallBack != null) {
            mActionModeCallBack.updateActionMode();
        }
    }

    protected class DetailInfoListener implements FileManagerService.OperationEventListener,
            OnDismissListener {
        public static final String DETAIL_DIALOG_TAG = "detaildialogtag";
        private TextView mDetailsText;
        private final String mName;
        private String mSize;
        private final String mModifiedTime;
        private final String mPermission;
        private final StringBuilder mStringBuilder = new StringBuilder();

        public DetailInfoListener(FileInfo fileInfo) {
            mStringBuilder.setLength(0);
            mName = mStringBuilder.append(getString(R.string.name)).append(": ").append(
                    fileInfo.getFileName()).append("\n").toString();
            mStringBuilder.setLength(0);
            mSize = mStringBuilder.append(getString(R.string.size)).append(": ").append(
                    FileUtils.sizeToString(0)).append(" \n").toString();

            long time = fileInfo.getFileLastModifiedTime();

            mStringBuilder.setLength(0);
            mModifiedTime = mStringBuilder.append(getString(R.string.modified_time)).append(": ")
                    .append(DateFormat.getDateInstance().format(new Date(time))).append("\n")
                    .toString();
            mStringBuilder.setLength(0);
            mPermission = getPermission(fileInfo.getFile());
        }

        private void appendPermission(boolean hasPermission, int title) {
            mStringBuilder.append(getString(title) + ": ");
            if (hasPermission) {
                mStringBuilder.append(getString(R.string.yes));
            } else {
                mStringBuilder.append(getString(R.string.no));
            }
        }

        private String getPermission(File file) {
            appendPermission(file.canRead(), R.string.readable);
            mStringBuilder.append("\n");
            appendPermission(file.canWrite(), R.string.writable);
            mStringBuilder.append("\n");
            appendPermission(file.canExecute(), R.string.executable);

            return mStringBuilder.toString();
        }

        @Override
        public void onTaskPrepare() {
            AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
            AlertDialogFragment detailFragment = builder.setCancelTitle(R.string.ok).setLayout(
                    R.layout.dialog_details).setTitle(R.string.details).create();

            detailFragment.setDismissListener(this);
            detailFragment.show(getFragmentManager(), DETAIL_DIALOG_TAG);
            boolean ret = getFragmentManager().executePendingTransactions();
            LogUtils.d(TAG,"executing pending transactions result: " + ret);
            if (detailFragment.getDialog() != null) {
                mDetailsText = (TextView) detailFragment.getDialog()
                        .findViewById(R.id.details_text);
                mStringBuilder.setLength(0);
                if (mDetailsText != null) {
                    mDetailsText.setText(mStringBuilder.append(mName).append(mSize).append(
                            mModifiedTime).append(mPermission).toString());
                    mDetailsText.setMovementMethod(ScrollingMovementMethod.getInstance());
                }
            }
        }

        @Override
        public void onTaskProgress(ProgressInfo progressInfo) {
            mSize = getString(R.string.size) + ": "
                    + FileUtils.sizeToString(progressInfo.getTotal()) + " \n";
            if (mDetailsText != null) {
                mStringBuilder.setLength(0);
                mStringBuilder.append(mName).append(mSize).append(mModifiedTime)
                        .append(mPermission);
                mDetailsText.setText(mStringBuilder.toString());
            }
        }

        @Override
        public void onTaskResult(int result) {
            LogUtils.d(TAG, "DetailInfoListener onTaskResult.");
            getFragmentManager().findFragmentByTag(DETAIL_DIALOG_TAG).getArguments().putString(
                    DETAIL_INFO_KEY, mStringBuilder.toString());
            return;
        }

        @Override
        public void onDismiss(DialogInterface dialog) {
            if (mService != null) {
                LogUtils.d(this.getClass().getName(), "onDismiss");
                mService.cancel(FileManagerOperationActivity.this.getClass().getName());
            }
        }
    }

    protected class HeavyOperationListener implements FileManagerService.OperationEventListener,
            View.OnClickListener {
        int mTitle = R.string.deleting;

        private boolean mPermissionToast = false;
        private boolean mOperationToast = false;
        public static final String HEAVY_DIALOG_TAG = "HeavyDialogFragment";

        // test performance
        // private long beforeTime = 0;

        public HeavyOperationListener(int titleID) {
            mTitle = titleID;
        }

        @Override
        public void onTaskPrepare() {
            // beforeTime = System.currentTimeMillis();
            ProgressDialogFragment heavyDialogFragment = ProgressDialogFragment.newInstance(
                    ProgressDialog.STYLE_HORIZONTAL, mTitle, R.string.wait, R.string.cancel);
            heavyDialogFragment.setCancelListener(this);
            heavyDialogFragment.setViewDirection(getViewDirection());
            heavyDialogFragment.show(getFragmentManager(), HEAVY_DIALOG_TAG);
            boolean ret = getFragmentManager().executePendingTransactions();
            LogUtils.d(TAG, "executing pending transactions result: " + ret);
        }

        @Override
        public void onTaskProgress(ProgressInfo progressInfo) {
            if (progressInfo.isFailInfo()) {
                switch (progressInfo.getErrorCode()) {
                case OperationEventListener.ERROR_CODE_COPY_NO_PERMISSION:
                    if (!mPermissionToast) {
                        mToastHelper.showToast(R.string.copy_deny);
                        mPermissionToast = true;
                    }
                    break;
                case OperationEventListener.ERROR_CODE_DELETE_NO_PERMISSION:
                    if (!mPermissionToast) {
                        mToastHelper.showToast(R.string.delete_deny);
                        mPermissionToast = true;
                    }
                    break;
                case OperationEventListener.ERROR_CODE_DELETE_UNSUCCESS:
                    if (!mOperationToast) {
                        mToastHelper.showToast(R.string.some_delete_fail);
                        mOperationToast = true;
                    }
                    break;
                case OperationEventListener.ERROR_CODE_PASTE_UNSUCCESS:
                    if (!mOperationToast) {
                        mToastHelper.showToast(R.string.some_paste_fail);
                        mOperationToast = true;
                    }
                    break;
                default:
                    if (!mPermissionToast) {
                        mToastHelper.showToast(R.string.operation_fail);
                        mPermissionToast = true;
                    }
                    break;
                }

            } else {
                ProgressDialogFragment heavyDialogFragment = (ProgressDialogFragment) getFragmentManager()
                        .findFragmentByTag(HEAVY_DIALOG_TAG);
                if (heavyDialogFragment != null) {
                    heavyDialogFragment.setProgress(progressInfo);
                }
            }
        }

        @Override
        public void onTaskResult(int errorType) {
            LogUtils.d(TAG, "HeavyOperationListener,onTaskResult result = " + errorType);
            switch (errorType) {
            case ERROR_CODE_PASTE_TO_SUB:
                mToastHelper.showToast(R.string.paste_sub_folder);
                break;
            case ERROR_CODE_CUT_SAME_PATH:
                mToastHelper.showToast(R.string.paste_same_folder);
                break;
            case ERROR_CODE_NOT_ENOUGH_SPACE:
                mToastHelper.showToast(R.string.insufficient_memory);
                break;
            case ERROR_CODE_DELETE_FAILS:
                mToastHelper.showToast(R.string.delete_fail);
                break;
            case ERROR_CODE_COPY_NO_PERMISSION:
                mToastHelper.showToast(R.string.copy_deny);
                break;
            case ERROR_CODE_COPY_GREATER_4G_TO_FAT32:
            	mToastHelper.showToast(R.string.operation_fail);
            	break;
            default:
                mFileInfoManager.updateFileInfoList(mCurrentPath, mSortType);
                mAdapter.notifyDataSetChanged();
                break;
            }
            ProgressDialogFragment heavyDialogFragment = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(HEAVY_DIALOG_TAG);
            if (heavyDialogFragment != null) {
                heavyDialogFragment.dismissAllowingStateLoss();
            }
            if (mFileInfoManager.getPasteType() == FileInfoManager.PASTE_MODE_CUT) {
                mFileInfoManager.clearPasteList();
                mAdapter.notifyDataSetChanged();
            }
            // final long endTime = System.currentTimeMillis();
            // LogUtils.i(TAG,
            // "HeavyOperationListener, onTaskResult,time cost is:" +
            // (endTime-beforeTime)/1000);

            invalidateOptionsMenu();
        }

        @Override
        public void onClick(View v) {
            if (mService != null) {
                LogUtils.i(this.getClass().getName(), "onClick cancel");
                mService.cancel(FileManagerOperationActivity.this.getClass().getName());
            }
        }
    }

    protected class ActionModeCallBack implements ActionMode.Callback, OnMenuItemClickListener {

        private PopupMenu mSelectPopupMenu = null;
        private boolean mSelectedAll = true;
        private Button mTextSelect = null;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View customView = layoutInflater.inflate(R.layout.actionbar_edit, null);
            mode.setCustomView(customView);
            mTextSelect = (Button) customView.findViewById(R.id.text_select);
            mTextSelect.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mSelectPopupMenu == null) {
                        mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                    } else {
                        updateSelectPopupMenu();
                        mSelectPopupMenu.show();
                    }
                }
            });
            MenuInflater menuInflater = mode.getMenuInflater();
            menuInflater.inflate(R.menu.edit_view_menu, menu);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            int selectedCount = mAdapter.getCheckedItemsCount();

            // enable(disable) copy, cut, and delete icon
            if (selectedCount == 0) {
                menu.findItem(R.id.copy).setEnabled(false);
                menu.findItem(R.id.delete).setEnabled(false);
                menu.findItem(R.id.cut).setEnabled(false);
            } else {
                menu.findItem(R.id.copy).setEnabled(true);
                menu.findItem(R.id.delete).setEnabled(true);
                menu.findItem(R.id.cut).setEnabled(true);
            }

            if (selectedCount == 0 || selectedCount > MAX_SHARE_FILES_COUNT) {
                menu.findItem(R.id.share).setEnabled(false);
            } else if (selectedCount == 1) {
                FileInfo fileInfo = mAdapter.getCheckedFileInfoItemsList().get(0);
                if (fileInfo.isDrmFile()
                        && DrmManager.getInstance().isRightsStatus(fileInfo.getFileAbsolutePath())
                        || fileInfo.isDirectory()) {
                    menu.findItem(R.id.share).setEnabled(false);
                } else {
                    menu.findItem(R.id.share).setEnabled(true);
                }
            } else {
                menu.findItem(R.id.share).setEnabled(true);
                List<FileInfo> files = mAdapter.getCheckedFileInfoItemsList();
                for (FileInfo info : files) {
                    File file = info.getFile();
                    if (file.isDirectory()) {
                        // break for loop; disable share icon
                        menu.findItem(R.id.share).setEnabled(false);
                        break;
                    }
                }
            }

            // more items
            // remove (disable) protection info icon
            menu.removeItem(R.id.protection_info);

            if (selectedCount == 0) {
                menu.findItem(R.id.rename).setEnabled(false);
                menu.findItem(R.id.details).setEnabled(false);
            } else if (selectedCount == 1) {
                // enable details icon
                menu.findItem(R.id.details).setEnabled(true);
                // enable rename icon
                if (mAdapter.getCheckedFileInfoItemsList().get(0).getFile().canWrite()) {
                    menu.findItem(R.id.rename).setEnabled(true);
                }
                // enable protection info icon
                FileInfo fileInfo = mAdapter.getCheckedFileInfoItemsList().get(0);
                if (fileInfo.isDrmFile()) {
                    String path = fileInfo.getFileAbsolutePath();
                    if (DrmManager.getInstance().checkDrmObjectType(path)) {
                        String mimeType = DrmManager.getInstance().getOriginalMimeType(path);
                        if (mimeType != null && mimeType.trim().length() != 0) {
                            menu.add(0, R.id.protection_info, 0,
                                    com.mediatek.internal.R.string.drm_protectioninfo_title);
                        }
                    }
                }
            } else {
                // disable details icon
                menu.findItem(R.id.details).setEnabled(false);
                // disable rename icon
                menu.findItem(R.id.rename).setEnabled(false);
            }
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.copy:
                mFileInfoManager.savePasteList(FileInfoManager.PASTE_MODE_COPY, mAdapter
                        .getCheckedFileInfoItemsList());
                mode.finish();
                break;
            case R.id.cut:
                mFileInfoManager.savePasteList(FileInfoManager.PASTE_MODE_CUT, mAdapter
                        .getCheckedFileInfoItemsList());
                mode.finish();
                break;
            case R.id.delete:
                showDeleteDialog();
                break;
            case R.id.share:
                share();
                break;
            case R.id.rename:
                showRenameDialog();
                break;
            case R.id.details:
                mService.getDetailInfo(FileManagerOperationActivity.this.getClass().getName(),
                        mAdapter.getCheckedFileInfoItemsList().get(0), new DetailInfoListener(
                                mAdapter.getCheckedFileInfoItemsList().get(0)));
                break;
            case R.id.protection_info:
                // calling framework to show a protection info dialog
                String path = mCurrentPath + MountPointManager.SEPARATOR
                        + mAdapter.getCheckedFileInfoItemsList().get(0).getFileName();
                DrmManager.getInstance().showProtectionInfoDialog(
                        FileManagerOperationActivity.this, path);
                if (mActionMode != null) {
                    mActionMode.finish();
                }
                break;
            case R.id.select:
                if (mSelectedAll) {
                    mAdapter.setAllItemChecked(true);
                } else {
                    mAdapter.setAllItemChecked(false);
                }
                updateActionMode();
                invalidateOptionsMenu();
                break;
            default:
                return false;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            switchToNavigationView();
            if (mActionMode != null) {
                mActionMode = null;
            }
            if (mSelectPopupMenu != null) {
                mSelectPopupMenu.dismiss();
                mSelectPopupMenu = null;
            }
        }

        private PopupMenu createSelectPopupMenu(View anchorView) {
            final PopupMenu popupMenu = new PopupMenu(FileManagerOperationActivity.this, anchorView);
            popupMenu.inflate(R.menu.select_popup_menu);
            popupMenu.setOnMenuItemClickListener(this);
            return popupMenu;
        }

        private void updateSelectPopupMenu() {
            if (mSelectPopupMenu == null) {
                mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                return;
            }
            final Menu menu = mSelectPopupMenu.getMenu();
            int selectedCount = mAdapter.getCheckedItemsCount();
            if (mAdapter.getCount() == 0) {
                menu.findItem(R.id.select).setEnabled(false);
            } else {
                menu.findItem(R.id.select).setEnabled(true);
                if (mAdapter.getCount() != selectedCount) {
                    menu.findItem(R.id.select).setTitle(R.string.select_all);
                    mSelectedAll = true;
                } else {
                    menu.findItem(R.id.select).setTitle(R.string.deselect_all);
                    mSelectedAll = false;
                }
            }
        }

        @Override
        public boolean onMenuItemClick(MenuItem menuItem) {
            switch (menuItem.getItemId()) {
            case R.id.select:
                if (mSelectedAll) {
                    mAdapter.setAllItemChecked(true);
                } else {
                    mAdapter.setAllItemChecked(false);
                }
                updateActionMode();
                invalidateOptionsMenu();
                break;
            default:
                return false;
            }
            return true;
        }

        public void updateActionMode() {
            int selectedCount = mAdapter.getCheckedItemsCount();
            String selected = "";
            if(Locale.getDefault().getLanguage().equals("fr") && selectedCount > 1) {
                try {
                    selected = getResources().getString(R.string.mutil_selected);
                } catch (Resources.NotFoundException e) {
                    selected = getResources().getString(R.string.selected);
                }
            } else {
                selected = getResources().getString(R.string.selected);
            }
            selected = "" + selectedCount + " " + selected;
            mTextSelect.setText(selected);

            mActionModeCallBack.updateSelectPopupMenu();
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
        }

    }

}
