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

import android.app.Activity;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Scroller;
import android.widget.TextView;

import com.mediatek.filemanager.AlertDialogFragment.EditDialogFragmentBuilder;
import com.mediatek.filemanager.AlertDialogFragment.EditTextDialogFragment;
import com.mediatek.filemanager.AlertDialogFragment.EditTextDialogFragment.EditTextDoneListener;
import com.mediatek.filemanager.AlertDialogFragment.OnDialogDismissListener;
import com.mediatek.filemanager.FileInfoManager.NavigationRecord;
import com.mediatek.filemanager.MountReceiver.MountListener;
import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.service.FileManagerService.ServiceBinder;
import com.mediatek.filemanager.service.ProgressInfo;
import com.mediatek.filemanager.utils.DrmManager;
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.LongStringUtils;
import com.mediatek.filemanager.utils.PDebug;
import com.mediatek.filemanager.utils.ToastHelper;

import java.util.ArrayList;
import java.util.List;

/**
 * This is the base activity for FileInfoManager(Activity), SelectFileActivity,
 * SelectPathActivity, SearchActivity. It defines the basic views and
 * interactions for activities.
 */
public abstract class AbsBaseActivity extends Activity implements OnItemClickListener,
        OnClickListener, MountListener, OnDialogDismissListener {
    private static final String TAG = "AbsBaseActivity";
    public static final String SAVED_PATH_KEY = "saved_path";
    private static final long NAV_BAR_AUTO_SCROLL_DELAY = 100;
    /** maximum tab text length */
    private static final int TAB_TET_MAX_LENGTH = 250;

    protected static final int DIALOG_CREATE_FOLDER = 1;

    /** ListView used for showing Files */
    protected ListView mListView = null;
    protected FileInfoAdapter mAdapter = null;
    protected TabManager mTabManager = null;
    protected SlowHorizontalScrollView mNavigationBar = null;
    protected FileInfo mSelectedFileInfo = null;
    protected MountPointManager mMountPointManager = null;
    protected MountReceiver mMountReceiver = null;

    protected FileManagerService mService = null;
    protected int mTop = -1;
    protected int mSortType = 0;
    protected String mCurrentPath = null;
    protected ToastHelper mToastHelper = null;
    protected FileInfoManager mFileInfoManager = null;
    public static final String CREATE_FOLDER_DIALOG_TAG = "CreateFolderDialog";
    protected Bundle mSavedInstanceState = null;
    protected int mSelectedTop = -1;

    public static final int MSG_DO_MOUNTED = 0;
    public static final int MSG_DO_EJECTED = 1;
    public static final int MSG_DO_UNMOUNTED = 2;
    public static final int MSG_DO_SDSWAP = 3;

    protected boolean mIsAlertDialogShowing = false;
    private static final String PREF_SHOW_HIDEN_FILE = "pref_show_hiden_file";
    protected boolean mServiceBinded = false;

    // there should always be only one dialog showing, to control it by this
    // flag, see ALPS00428101
    @Override
    public void onDialogDismiss() {
        LogUtils.d(TAG, "dialog dismissed...");
        mIsAlertDialogShowing = false;
    }

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mService.disconnected(this.getClass().getName());
            mServiceBinded = false;
            LogUtils.w(TAG, "onServiceDisconnected");
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            LogUtils.d(TAG, "onServiceConnected");
            mService = ((ServiceBinder) service).getServiceInstance();
            mServiceBinded = true;
            serviceConnected();
        }
    };

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            LogUtils.d(TAG, "handleMessage, msg = " + msg.what);
            switch (msg.what) {
            case MSG_DO_MOUNTED:
                doOnMounted((String) msg.obj);
                break;
            case MSG_DO_EJECTED:
                doOnEjected((String) msg.obj);
                break;
            case MSG_DO_UNMOUNTED:
                doOnUnMounted((String) msg.obj);
                break;
            case MSG_DO_SDSWAP:
               doOnSdSwap();
               break;
            default:
                break;
            }
        }
    };

    private void doPrepareForMount(String mountPoint) {
        LogUtils.i(TAG, "doPrepareForMount,mountPoint = " + mountPoint);
        if ((mCurrentPath + MountPointManager.SEPARATOR).startsWith(mountPoint
                + MountPointManager.SEPARATOR)
                || mMountPointManager.isRootPath(mCurrentPath)) {
            LogUtils.d(TAG, "pre-onMounted");
            if (mService != null && mService.isBusy(this.getClass().getName())) {
                mService.cancel(this.getClass().getName());
            }
        }

        mMountPointManager.init(getApplicationContext());
    }

    @Override
    public void onMounted(String mountPoint) {
        LogUtils.i(TAG, "onMounted,mountPoint = " + mountPoint);
        Message.obtain(mHandler, MSG_DO_MOUNTED, mountPoint).sendToTarget();
    }

    private void doOnMounted(String mountPoint) {
        LogUtils.i(TAG, "doOnMounted,mountPoint = " + mountPoint);
        doPrepareForMount(mountPoint);
        if (mMountPointManager.isRootPath(mCurrentPath)) {
            LogUtils.d(TAG, "doOnMounted,mCurrentPath is root path: " + mCurrentPath);
            showDirectoryContent(mCurrentPath);
        }
    }

    @Override
    public void onUnMounted(String unMountPoint) {
        LogUtils.i(TAG, "onUnMounted,unMountPoint: " + unMountPoint);
        Message.obtain(mHandler, MSG_DO_UNMOUNTED, unMountPoint).sendToTarget();
    }

    @Override
    public void onEjected(String unMountPoint) {
        LogUtils.i(TAG, "onEjected,unMountPoint: " + unMountPoint);
        Message.obtain(mHandler, MSG_DO_EJECTED, unMountPoint).sendToTarget();
    }

    @Override
    public void onSdSwap() {
        LogUtils.i(TAG, "onSdSwap...");
        Message.obtain(mHandler, MSG_DO_SDSWAP).sendToTarget();
    }

    private void doOnSdSwap() {
        mMountPointManager.init(getApplicationContext());
        backToRootPath();
    }

    private void doOnEjected(String unMountPoint) {
        if ((mCurrentPath + MountPointManager.SEPARATOR).startsWith(unMountPoint
                + MountPointManager.SEPARATOR)
                || mMountPointManager.isRootPath(mCurrentPath)
                || mMountPointManager.isPrimaryVolume(unMountPoint)) {
            LogUtils.d(TAG, "onEjected,Current Path = " + mCurrentPath);
            if (mService != null && mService.isBusy(this.getClass().getName())) {
                mService.cancel(this.getClass().getName());
            }
        }
    }

    private void doOnUnMounted(String unMountPoint) {
        if (mFileInfoManager != null) {
            int pasteCnt = mFileInfoManager.getPasteCount();
            LogUtils.i(TAG, "doOnUnmounted,unMountPoint: " + unMountPoint + ",pasteCnt = "
                    + pasteCnt);

            if (pasteCnt > 0) {
                FileInfo fileInfo = mFileInfoManager.getPasteList().get(0);
                if (fileInfo.getFileAbsolutePath().startsWith(
                        unMountPoint + MountPointManager.SEPARATOR)) {
                    LogUtils.i(TAG, "doOnUnmounted,clear paste list. ");
                    mFileInfoManager.clearPasteList();
                    invalidateOptionsMenu();
                }
            }
        }

        if ((mCurrentPath + MountPointManager.SEPARATOR).startsWith(unMountPoint
                + MountPointManager.SEPARATOR)
                || mMountPointManager.isRootPath(mCurrentPath)) {
            LogUtils.d(TAG, "onUnmounted,Current Path = " + mCurrentPath);
            if (mService != null && mService.isBusy(this.getClass().getName())) {
                mService.cancel(this.getClass().getName());
            }
            showToastForUnmount(unMountPoint);

            DialogFragment listFramgent = (DialogFragment) getFragmentManager().findFragmentByTag(
                    ListListener.LIST_DIALOG_TAG);
            if (listFramgent != null) {
                LogUtils.d(TAG, "onUnmounted,listFramgent dismiss. ");
                listFramgent.dismissAllowingStateLoss();
            }

            EditTextDialogFragment createFolderDialogFragment = (EditTextDialogFragment) getFragmentManager()
                    .findFragmentByTag(CREATE_FOLDER_DIALOG_TAG);
            if (createFolderDialogFragment != null) {
                LogUtils.d(TAG, "onUnmounted,createFolderDialogFragment dismiss. ");
                createFolderDialogFragment.dismissAllowingStateLoss();
            }

            backToRootPath();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        PDebug.Start("AbsBaseActivity - onCreate");
        super.onCreate(savedInstanceState);
        IconManager.updateCustomDrableMap(this);
        mSavedInstanceState = savedInstanceState;

        LogUtils.d(TAG, "onCreate");

        mToastHelper = new ToastHelper(this);
        // start watching external storage change.
        mMountPointManager = MountPointManager.getInstance();
        mMountPointManager.init(getApplicationContext());
        DrmManager.getInstance().init(getApplicationContext());

        PDebug.Start("AbsBaseActivity - bindService");
        bindService(new Intent(getApplicationContext(), FileManagerService.class),
                mServiceConnection, BIND_AUTO_CREATE);
        PDebug.End("AbsBaseActivity - bindService");

        setMainContentView();

        // set up a sliding navigation bar for navigation view
        mNavigationBar = (SlowHorizontalScrollView) findViewById(R.id.navigation_bar);
        if (mNavigationBar != null) {
            mNavigationBar.setVerticalScrollBarEnabled(false);
            mNavigationBar.setHorizontalScrollBarEnabled(false);
            mTabManager = new TabManager();
        }

        // set up a list view
        mListView = (ListView) findViewById(R.id.list_view);
        if (mListView != null) {
            mListView.setEmptyView(findViewById(R.id.empty_view));
            mListView.setOnItemClickListener(this);
            mListView.setFastScrollEnabled(true);
            // mListView.setVerticalScrollBarEnabled(false);
            mListView.setVerticalScrollBarEnabled(true);
        }
        PDebug.End("AbsBaseActivity - onCreate");
    }

    private void reloadContent() {
        LogUtils.d(TAG, "reloadContent");
        if (mService != null && !mService.isBusy(this.getClass().getName())) {
            if (mFileInfoManager != null && mFileInfoManager.isPathModified(mCurrentPath)) {
                showDirectoryContent(mCurrentPath);
            } else if (mFileInfoManager != null && mAdapter != null) {
                mAdapter.notifyDataSetChanged();
            }
        }
    }

    @Override
    protected void onResume() {
        PDebug.Start("AbsBaseActivity - onResume");
        super.onResume();
        LogUtils.d(TAG, "onResume");
        DrmManager.getInstance().init(getApplicationContext());
        IconManager.updateCustomDrableMap(this);
        DrmManager.getInstance().init(getApplicationContext());
        reloadContent();
        PDebug.End("AbsBaseActivity - onResume");
    }

    @Override
    protected void onPause() {
        LogUtils.d(TAG, "onPause");
        if (mServiceBinded == true) {
            unbindService(mServiceConnection);
            mServiceBinded = false;
        }
        super.onPause();
    }

    protected void showCreateFolderDialog() {
        LogUtils.d(TAG, "showCreateFolderDialog");
        if (mIsAlertDialogShowing) {
            LogUtils.d(TAG, "Another Dialog showing, return!~~");
            return;
        }
        if (isResumed()) {
            mIsAlertDialogShowing = true;
            EditDialogFragmentBuilder builder = new EditDialogFragmentBuilder();
            builder.setDefault("", 0).setDoneTitle(R.string.ok).setCancelTitle(R.string.cancel)
                    .setTitle(R.string.new_folder);
            EditTextDialogFragment createFolderDialogFragment = builder.create();
            createFolderDialogFragment.setOnEditTextDoneListener(new CreateFolderListener());
            createFolderDialogFragment.setOnDialogDismissListener(this);
            try {
                createFolderDialogFragment.show(getFragmentManager(), CREATE_FOLDER_DIALOG_TAG);
                boolean ret = getFragmentManager().executePendingTransactions();
                LogUtils.d(TAG, "executing pending transactions result: " + ret);
            } catch (IllegalStateException e) {
                LogUtils.d(TAG, "call show dialog after onSaveInstanceState " + e);
                if (createFolderDialogFragment != null) {
                    createFolderDialogFragment.dismissAllowingStateLoss();
                }
            }
        }
    }

    protected final class CreateFolderListener implements EditTextDoneListener {
        public void onClick(String text) {
            if (mService != null) {
                String dstPath = mCurrentPath + MountPointManager.SEPARATOR + text;
                mService.createFolder(AbsBaseActivity.this.getClass().getName(), dstPath,
                        new LightOperationListener(text));
            }
        }
    }

    /**
     * This method is left for its children class to set main layout
     */
    protected abstract void setMainContentView();

    @Override
    protected void onDestroy() {
        LogUtils.d(TAG, "onDestroy");
        if (mService != null) {
            if (mServiceBinded) {
                unbindService(mServiceConnection);
                mServiceBinded = false;
            }
            mMountReceiver.unregisterMountListener(this);
            unregisterReceiver(mMountReceiver);
        } else {
            LogUtils.w(TAG, "#onDestroy(),the Service hasn't connected yet.");
        }
        DrmManager.getInstance().release();
        super.onDestroy();
    }

    private void backToRootPath() {
        LogUtils.d(TAG, "backToRootPath...");
        if (mMountPointManager != null && mMountPointManager.isRootPath(mCurrentPath)) {
            showDirectoryContent(mCurrentPath);
        } else if (mTabManager != null) {
            mTabManager.updateNavigationBar(0);
        }
        clearNavigationList();
    }

    private void showToastForUnmount(String path) {
        LogUtils.d(TAG, "showToastForUnmount,path = " + path);
        if (isResumed()) {
            String unMountPointDescription = MountPointManager.getInstance().getDescriptionPath(
                    path);
            LogUtils.d(TAG, "showToastForUnmount,unMountPointDescription:"
                    + unMountPointDescription);
            mToastHelper.showToast(getString(R.string.unmounted, unMountPointDescription));
        }
    }

    /**
     * This method add a path into navigation history list
     * 
     * @param dirPath
     *            the path that should be added
     */
    protected void addToNavigationList(String path, FileInfo selectedFileInfo, int top) {
        mFileInfoManager.addToNavigationList(new NavigationRecord(path, selectedFileInfo, top));
    }

    /**
     * This method clear navigation history list
     */
    protected void clearNavigationList() {
        mFileInfoManager.clearNavigationList();
    }

    /**
     * This method used to be inherited by subclass to get a path.
     * 
     * @return path to a folder
     */
    protected abstract String initCurrentFileInfo();

    protected class TabManager {
        private final List<String> mTabNameList = new ArrayList<String>();
        protected LinearLayout mTabsHolder = null;
        private String mCurFilePath = null;
        private final Button mBlankTab;
        private LinearLayout.LayoutParams mBlanckBtnParam = null;

        public TabManager() {
            mTabsHolder = (LinearLayout) findViewById(R.id.tabs_holder);
            mBlankTab = new Button(AbsBaseActivity.this);
            mBlankTab.setBackgroundDrawable(getResources().getDrawable(R.drawable.fm_blank_tab));
            mBlanckBtnParam = new LinearLayout.LayoutParams(
                    new ViewGroup.MarginLayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
                            LinearLayout.LayoutParams.MATCH_PARENT));

            mBlankTab.setLayoutParams(mBlanckBtnParam);
            mTabsHolder.addView(mBlankTab);
        }

        public void refreshTab(String initFileInfo) {
            LogUtils.d(TAG, "refreshTab,initFileInfo = " + initFileInfo);
            int count = mTabsHolder.getChildCount();
            mTabsHolder.removeViews(0, count);
            mTabNameList.clear();
            if (getViewDirection() == ViewGroup.LAYOUT_DIRECTION_LTR) {
                mBlanckBtnParam.setMargins((int) getResources().getDimension(R.dimen.tab_margin_left), 0,
                        (int) getResources().getDimension(R.dimen.tab_margin_right), 0);
            } else if (getViewDirection() == ViewGroup.LAYOUT_DIRECTION_RTL) {
                mBlanckBtnParam.setMargins((int) getResources().getDimension(R.dimen.tab_margin_right), 0,
                        (int) getResources().getDimension(R.dimen.tab_margin_left), 0);
            }
            mBlankTab.setLayoutParams(mBlanckBtnParam);
            
            mCurFilePath = initFileInfo;
            if (mCurFilePath != null) {
                addTab(MountPointManager.HOME);
                if (!mMountPointManager.isRootPath(mCurFilePath)) {
                    String path = mMountPointManager.getDescriptionPath(mCurFilePath);
                    String[] result = path.split(MountPointManager.SEPARATOR);
                    for (String string : result) {
                        addTab(string);
                    }

                    if (getViewDirection() == ViewGroup.LAYOUT_DIRECTION_LTR) {
                        // scroll to right with slow-slide animation
                        startActionBarScroll();
                    } else if(getViewDirection() == ViewGroup.LAYOUT_DIRECTION_RTL) {
                        // scroll horizontal view to the right
                        mNavigationBar.startHorizontalScroll(-mNavigationBar.getScrollX(),
                                    -mNavigationBar.getRight());
                    }
                }
            }
            updateHomeButton();
        }

        private void startActionBarScroll() {
            // scroll to right with slow-slide animation
            // To pass the Launch performance test, avoid the scroll
            // animation when launch.
            int tabHostCount = mTabsHolder.getChildCount();
            int navigationBarCount = mNavigationBar.getChildCount();
            if ((tabHostCount > 2) && (navigationBarCount >= 1)) {
                int width = mNavigationBar.getChildAt(navigationBarCount - 1).getRight();
                mNavigationBar.startHorizontalScroll(mNavigationBar.getScrollX(), width
                        - mNavigationBar.getScrollX());
            }
        }

        protected void updateHomeButton() {
            ImageButton homeBtn = (ImageButton) mTabsHolder.getChildAt(0);
            if (homeBtn == null) {
                LogUtils.w(TAG, "HomeBtm is null,return.");
                return;
            }
            Resources resources = getResources();
            if (mTabsHolder.getChildCount() == 2) { // two tabs: home tab +
                // blank
                // tab
                homeBtn.setBackgroundDrawable(resources
                        .getDrawable(R.drawable.custom_home_ninepatch_tab));
                homeBtn.setImageDrawable(resources.getDrawable(R.drawable.ic_home_text));
                homeBtn.setPadding((int) resources.getDimension(R.dimen.home_btn_padding), 0,
                        (int) resources.getDimension(R.dimen.home_btn_padding), 0);
            } else {
                homeBtn.setBackgroundDrawable(resources
                        .getDrawable(R.drawable.custom_home_ninepatch_tab));
                homeBtn.setImageDrawable(resources.getDrawable(R.drawable.ic_home));
            }
        }

        /**
         * This method updates the navigation view to the previous view when
         * back button is pressed
         * 
         * @param newPath
         *            the previous showed directory in the navigation history
         */
        private void showPrevNavigationView(String newPath) {

            refreshTab(newPath);
            showDirectoryContent(newPath);
        }

        /**
         * This method creates tabs on the navigation bar
         * 
         * @param text
         *            the name of the tab
         */
        protected void addTab(String text) {
            LinearLayout.LayoutParams mlp = null;

            mTabsHolder.removeView(mBlankTab);
            View viewLikeBtn = null;
            if (mTabNameList.isEmpty()) {
                viewLikeBtn = new ImageButton(AbsBaseActivity.this);
                mlp = new LinearLayout.LayoutParams(new ViewGroup.MarginLayoutParams(
                        LinearLayout.LayoutParams.WRAP_CONTENT,
                        LinearLayout.LayoutParams.MATCH_PARENT));
                mlp.setMargins(0, 0, 0, 0);
                viewLikeBtn.setLayoutParams(mlp);
            } else {
                Button button = new Button(AbsBaseActivity.this);
                button.setTextColor(Color.BLACK);
                button.setBackgroundDrawable(getResources().getDrawable(R.drawable.custom_tab));
                button.setMaxWidth(TAB_TET_MAX_LENGTH);
                LongStringUtils.fadeOutLongString(((TextView)button));
                button.setText(text);
                
                mlp = new LinearLayout.LayoutParams(new ViewGroup.MarginLayoutParams(
                        LinearLayout.LayoutParams.WRAP_CONTENT,
                        LinearLayout.LayoutParams.MATCH_PARENT));

                if (getViewDirection() == ViewGroup.LAYOUT_DIRECTION_LTR) {
                    mlp.setMargins((int) getResources().getDimension(R.dimen.tab_margin_left), 0, 0, 0);
                } else if (getViewDirection() == ViewGroup.LAYOUT_DIRECTION_RTL) {
                    mlp.setMargins(0, 0, (int) getResources().getDimension(R.dimen.tab_margin_left), 0);
                }
                button.setLayoutParams(mlp);
                viewLikeBtn = button;
            }
            viewLikeBtn.setOnClickListener(AbsBaseActivity.this);
            viewLikeBtn.setId(mTabNameList.size());
            mTabsHolder.addView(viewLikeBtn);
            mTabNameList.add(text);

            // add blank tab to the tab holder
            mTabsHolder.addView(mBlankTab);
        }

        /**
         * The method updates the navigation bar
         * 
         * @param id
         *            the tab id that was clicked
         */
        protected void updateNavigationBar(int id) {
            LogUtils.d(TAG, "updateNavigationBar,id = " + id);
            // click current button do not response
            if (id < mTabNameList.size() - 1) {
                int count = mTabNameList.size() - id;
                mTabsHolder.removeViews(id + 1, count);

                for (int i = 1; i < count; i++) {
                    // update mTabNameList
                    mTabNameList.remove(mTabNameList.size() - 1);
                }
                mTabsHolder.addView(mBlankTab);

                if (id == 0) {
                    mCurFilePath = mMountPointManager.getRootPath();
                } else {
                    // get mount point path
                    String mntPointPath = mMountPointManager.getRealMountPointPath(mCurFilePath);
                    LogUtils.d(TAG, "mntPointPath: " + mntPointPath + " for mCurFilepath: " + mCurFilePath);
                    String path = mCurFilePath.substring(mntPointPath.length() + 1);
                    StringBuilder sb = new StringBuilder(mntPointPath);
                    String[] pathParts = path.split(MountPointManager.SEPARATOR);
                    // id=0,1 is for Home button and mnt point button, so from id = 2 to get other parts of path
                    for (int i = 2; i <= id; i++) {
                        sb.append(MountPointManager.SEPARATOR);
                        sb.append(pathParts[i-2]);
                    }
                    mCurFilePath = sb.toString();
                    LogUtils.d(TAG, "to enter file path: " + mCurFilePath);
                }
                int top = -1;
                int pos = -1;
                FileInfo selectedFileInfo = null;
                if (mListView.getCount() > 0) {
                    View view = mListView.getChildAt(0);
                    if (view != null) {
                        pos = mListView.getPositionForView(view);
                        selectedFileInfo = mAdapter.getItem(pos);
                        top = view.getTop();
                        LogUtils.d(TAG,"updateNavigationBar, pos: " + pos + " top: " + top);
                        addToNavigationList(mCurrentPath, selectedFileInfo, top);
                    }
                }
                showDirectoryContent(mCurFilePath);
                updateHomeButton();
            }
        }

    }

    @Override
    public void onClick(View view) {
        if (mService.isBusy(this.getClass().getName())) {
            LogUtils.d(TAG, "onClick(), service is busy.");
            return;
        }
        int id = view.getId();
        LogUtils.d(TAG, "onClick() id=" + id);
        mTabManager.updateNavigationBar(id);
    }

    private int restoreSelectedPosition() {
        if (mSelectedFileInfo == null) {
            return -1;
        } else {
            int curSelectedItemPosition = mAdapter.getPosition(mSelectedFileInfo);
            mSelectedFileInfo = null;
            return curSelectedItemPosition;
        }
    }

    /**
     * This method gets all files/folders from a directory and displays them in
     * the list view
     * 
     * @param dirPath
     *            the directory path
     */
    protected void showDirectoryContent(String path) {
        LogUtils.d(TAG, "showDirectoryContent,path = " + path);
        if (isFinishing()) {
            LogUtils.i(TAG, "showDirectoryContent,isFinishing: true, do not loading again");
            return;
        }
        mCurrentPath = path;
        if (mService != null) {
            mService.listFiles(this.getClass().getName(), mCurrentPath, new ListListener());
        }
    }

    protected void onPathChanged() {
        LogUtils.d(TAG, "onPathChanged");
        if (mTabManager != null) {
            mTabManager.refreshTab(mCurrentPath);
        }
        invalidateOptionsMenu();
    }

    @Override
    public void onBackPressed() {
        LogUtils.d(TAG, "onBackPressed");
        if (mService != null && mService.isBusy(this.getClass().getName())) {
            LogUtils.i(TAG, "onBackPressed, service is busy. ");
            return;
        }

        if (mCurrentPath != null && !mMountPointManager.isRootPath(mCurrentPath)) {
            NavigationRecord navRecord = mFileInfoManager.getPrevNavigation();
            String prevPath = null;
            if (navRecord != null) {
                prevPath = navRecord.getRecordPath();
                mSelectedFileInfo = navRecord.getSelectedFile();
                mTop = navRecord.getTop();
                if (prevPath != null) {
                    mTabManager.showPrevNavigationView(prevPath);
                    LogUtils.d(TAG, "sonBackPressed,prevPath = " + prevPath);
                    return;
                }
            }
        }
        super.onBackPressed();
    }

    protected void serviceConnected() {
        LogUtils.i(TAG, "serviceConnected");

        mFileInfoManager = mService.initFileInfoManager(this.getClass().getName());
        mService.setListType(getPrefsShowHidenFile() ? FileManagerService.FILE_FILTER_TYPE_ALL
                : FileManagerService.FILE_FILTER_TYPE_DEFAULT, this.getClass().getName());
        
        mAdapter = new FileInfoAdapter(AbsBaseActivity.this, mService, mFileInfoManager);
        if (mListView != null) {
            mListView.setAdapter(mAdapter);

            if (mSavedInstanceState == null) {
                mCurrentPath = initCurrentFileInfo();
                if (mCurrentPath != null) {
                    showDirectoryContent(mCurrentPath);
                }
            } else {
                String savePath = mSavedInstanceState.getString(SAVED_PATH_KEY);
                if (savePath != null
                        && mMountPointManager.isMounted(mMountPointManager
                                .getRealMountPointPath(savePath))) {
                    mCurrentPath = savePath;
                } else {
                    mCurrentPath = initCurrentFileInfo();
                }

                if (mCurrentPath != null) {
                    mTabManager.refreshTab(mCurrentPath);
                    reloadContent();
                }
                restoreDialog();

            }
            mAdapter.notifyDataSetChanged();
        }
        // register Receiver when service connected..
        mMountReceiver = MountReceiver.registerMountReceiver(this);
        mMountReceiver.registerMountListener(this);
    }

    protected void restoreDialog() {
        DialogFragment listFramgent = (DialogFragment) getFragmentManager().findFragmentByTag(
                ListListener.LIST_DIALOG_TAG);
        if (listFramgent != null) {
            LogUtils.i(TAG, "listFramgent != null");
            if (mService.isBusy(this.getClass().getName())) {
                LogUtils.i(TAG, "list reconnected mService");
                mService.reconnected(this.getClass().getName(), new ListListener());
            } else {
                LogUtils.i(TAG, "the list is complete dismissAllowingStateLoss");
                listFramgent.dismissAllowingStateLoss();
            }
        }
        EditTextDialogFragment createFolderDialogFragment = (EditTextDialogFragment) getFragmentManager()
                .findFragmentByTag(CREATE_FOLDER_DIALOG_TAG);
        if (createFolderDialogFragment != null) {
            createFolderDialogFragment.setOnEditTextDoneListener(new CreateFolderListener());
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        if (mCurrentPath != null) {
            outState.putString(SAVED_PATH_KEY, mCurrentPath);
        }
        super.onSaveInstanceState(outState);
    }

    protected class ListListener implements FileManagerService.OperationEventListener {

        public static final String LIST_DIALOG_TAG = "ListDialogFragment";

        protected void dismissDialogFragment() {
            LogUtils.d(TAG, "ListListener dismissDialogFragment");
            DialogFragment listDialogFragment = (DialogFragment) getFragmentManager()
                    .findFragmentByTag(LIST_DIALOG_TAG);
            if (listDialogFragment != null) {
                LogUtils.d(TAG, "ListListener listDialogFragment != null dismiss");
                listDialogFragment.dismissAllowingStateLoss();
            } else {
                LogUtils.d(TAG, "dismissDialogFragment listDialogFragment == null on dismiss....");
            }
        }

        @Override
        public void onTaskResult(int result) {
            LogUtils.i(TAG, "ListListener,TaskResult result = " + result);
            if (mAdapter.isMode(FileInfoAdapter.MODE_EDIT)) {
                mFileInfoManager.loadFileInfoList(mCurrentPath, mSortType,mSelectedFileInfo);
                mSelectedFileInfo = mAdapter.getFirstCheckedFileInfoItem();
            } else {
                mFileInfoManager.loadFileInfoList(mCurrentPath, mSortType);
            }

            mAdapter.notifyDataSetChanged();
            int selectedItemPosition = restoreSelectedPosition();
            if (selectedItemPosition == -1) {
                mListView.setSelectionAfterHeaderView();
            } else if (selectedItemPosition >= 0 && selectedItemPosition < mAdapter.getCount()) {
                if (mSelectedTop != -1) {
                    mListView.setSelectionFromTop(selectedItemPosition, mSelectedTop);
                    mSelectedTop = -1;
                } else if (mTop != -1) {
                    mListView.setSelectionFromTop(selectedItemPosition, mTop);
                    mTop = -1;
                } else {
                    mListView.setSelection(selectedItemPosition);
                }
            }
            dismissDialogFragment();
            onPathChanged();
        }

        @Override
        public void onTaskPrepare() {
            return;
        }

        @Override
        public void onTaskProgress(ProgressInfo progressInfo) {
            ProgressDialogFragment listDialogFragment = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(LIST_DIALOG_TAG);
            if (isResumed()) {
                if (listDialogFragment == null) {
                    LogUtils.d(TAG, " isResumed() onTaskProgress listDialogFragment == null on dismiss....");
                    listDialogFragment = ProgressDialogFragment.newInstance(
                            ProgressDialog.STYLE_HORIZONTAL, -1, R.string.loading,
                            AlertDialogFragment.INVIND_RES_ID);

                    listDialogFragment.setViewDirection(AbsBaseActivity.this.getViewDirection());
                    listDialogFragment.show(getFragmentManager(), LIST_DIALOG_TAG);
                    getFragmentManager().executePendingTransactions();
                }
                listDialogFragment.setProgress(progressInfo);
            }
        }
    }

    protected class LightOperationListener implements FileManagerService.OperationEventListener {

        String mDstName = null;

        LightOperationListener(String dstName) {
            mDstName = dstName;
        }

        @Override
        public void onTaskResult(int errorType) {
            LogUtils.i(TAG, "LightOperationListener,TaskResult result = " + errorType);
            switch (errorType) {
            case ERROR_CODE_SUCCESS:
            case ERROR_CODE_USER_CANCEL:
                FileInfo fileInfo = mFileInfoManager.updateOneFileInfoList(mCurrentPath, mSortType);
                mAdapter.notifyDataSetChanged();
                if (fileInfo != null) {
                    int postion = mAdapter.getPosition(fileInfo);
                    LogUtils.d(TAG, "LightOperation postion = " + postion);
                    mListView.setSelection(postion);
                    invalidateOptionsMenu();
                }

                break;
            case ERROR_CODE_FILE_EXIST:
                if (mDstName != null) {
                    mToastHelper.showToast(getResources().getString(R.string.already_exists,
                            mDstName));
                }
                break;
            case ERROR_CODE_NAME_EMPTY:
                mToastHelper.showToast(R.string.invalid_empty_name);
                break;
            case ERROR_CODE_NAME_TOO_LONG:
                mToastHelper.showToast(R.string.file_name_too_long);
                break;
            case ERROR_CODE_NOT_ENOUGH_SPACE:
                mToastHelper.showToast(R.string.insufficient_memory);
                break;
            case ERROR_CODE_UNSUCCESS:
                mToastHelper.showToast(R.string.operation_fail);
                break;
            default:
                LogUtils.e(TAG, "wrong errorType for LightOperationListener");
                break;
            }
        }

        @Override
        public void onTaskPrepare() {
            return;
        }

        @Override
        public void onTaskProgress(ProgressInfo progressInfo) {
            return;
        }
    }

    /**
     * This class add a slow-slide animation for HorizontalscrollView.
     */
    private static class SlowHorizontalScrollView extends HorizontalScrollView {
        private static final String TAG = "SlowHorizontalScrollView";
        private static final int SCROLL_DURATION = 2000;
        private final Scroller mScroller = new Scroller(getContext());

        public SlowHorizontalScrollView(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
        }

        public SlowHorizontalScrollView(Context context) {
            super(context);
        }

        public SlowHorizontalScrollView(Context context, AttributeSet attrs) {
            super(context, attrs);
        }

        public void startHorizontalScroll(int startX, int dx) {
            LogUtils.d(TAG, "start scroll");
            mScroller.startScroll(startX, 0, dx, 0, SCROLL_DURATION);
            invalidate();
        }

        @Override
        public void computeScroll() {
            if (mScroller.computeScrollOffset()) {
                scrollTo(mScroller.getCurrX(), 0);
                postInvalidate();
            }
            super.computeScroll();
        }

        @Override
        public boolean onTouchEvent(MotionEvent ev) {
            mScroller.abortAnimation();
            return super.onTouchEvent(ev);
        }
    }
    
    protected boolean changePrefsShowHidenFile() {
        boolean hide = getPrefsShowHidenFile();
        Editor editor = getPreferences(MODE_PRIVATE).edit();
        editor.putBoolean(PREF_SHOW_HIDEN_FILE, !hide);
        editor.commit();
        return hide;
    }
    
    protected boolean getPrefsShowHidenFile() {
        SharedPreferences prefs = getPreferences(MODE_PRIVATE);
        return prefs.getBoolean(PREF_SHOW_HIDEN_FILE, false);
    }

    protected int getViewDirection() {
        return mNavigationBar.getParent().getParent().getLayoutDirection();
    }
}
