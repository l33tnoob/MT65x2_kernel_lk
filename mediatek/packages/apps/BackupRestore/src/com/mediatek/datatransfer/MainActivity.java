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

package com.mediatek.datatransfer;

import java.io.File;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.DialogID;

public class MainActivity extends Activity{
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MainActivity";
    private final String STATE_TAB = "tab";
    public static final String ACTION_WHERE="action_where";
    public static final String ACTION_BACKUP="action_backup";
    public static final String ACTION_RESTORE="ction_restore";    
    private Fragment mBackupFragment;
    private Fragment mRestoreFragment;
    MainOnSDCardStatusChangedListener mMainOnSDCardStatusChangedListener = new MainOnSDCardStatusChangedListener();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyLogger.logD(CLASS_TAG, "onCreate");
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        initFragments(savedInstanceState);
        setupActionBar(savedInstanceState);
        setProgressBarIndeterminateVisibility(false);
        registerSDCardListener();
        NotifyManager.getInstance(this).clearNotification();
    }

    @Override
    protected void onResume() {
        super.onResume();
        MyLogger.logD(CLASS_TAG, "onResume");
        Intent actionIntent = getIntent();
        if (getIntent() != null) {
            ActionBar actionBar = getActionBar();
            String actionString = actionIntent.getAction();
            String actionWhere = actionIntent.getStringExtra(ACTION_WHERE);
            MyLogger.logD(CLASS_TAG, "actionWhere is " + actionWhere);
            if (actionString != null
                    && actionString.equals(NotifyManager.FP_NEW_DETECTION_INTENT_LIST)) {
                actionBar.selectTab(actionBar.getTabAt(actionBar.getTabCount() - 1));
            } else if (actionWhere != null && actionWhere.equals(ACTION_RESTORE)) {
                actionBar.selectTab(actionBar.getTabAt(actionBar.getTabCount() - 1));
            } else if (actionWhere != null && actionWhere.equals(ACTION_BACKUP)) {
                actionBar.selectTab(actionBar.getTabAt(0));
            }
            actionIntent.putExtra(ACTION_WHERE, "");

        }else{
            MyLogger.logE(CLASS_TAG, "Intent is null");
        }
        sdCardCheck();
    }

    private void sdCardCheck() {
        if(SDCardUtils.getExternalStoragePath(this)==null){
            showDialog(DialogID.DLG_NO_SDCARD);
        }else{
            removeDialog(DialogID.DLG_NO_SDCARD);
        }
    }

    protected void onDestroy() {
        super.onDestroy();
        MyLogger.logD(CLASS_TAG, "onDestroy");
        unRegisterSDCardListener();
    }
    
    protected void  onSaveInstanceState  (Bundle outState){
        super.onSaveInstanceState(outState);
        ActionBar bar = getActionBar();
        int position = bar.getSelectedNavigationIndex();
        outState.putInt(STATE_TAB, position);
    }
    
    private void initFragments(Bundle state){
        FragmentManager fm = getFragmentManager();
        if(state != null){
            mBackupFragment = fm.findFragmentByTag(Constants.BACKUP);
            mRestoreFragment = fm.findFragmentByTag(Constants.RESTORE);
        }
        if(mBackupFragment == null){
            mBackupFragment = new BackupTabFragment();
        }
        if(mRestoreFragment == null){
            mRestoreFragment = new RestoreTabFragment();
        }
    }

    private void registerSDCardListener() {
        SDCardReceiver.getInstance().registerOnSDCardChangedListener(mMainOnSDCardStatusChangedListener);
    }
    private void unRegisterSDCardListener() {
        SDCardReceiver.getInstance().unRegisterOnSDCardChangedListener(mMainOnSDCardStatusChangedListener);
    }

    class MainOnSDCardStatusChangedListener implements SDCardReceiver.OnSDCardStatusChangedListener {

        @Override
        public void onSDCardStatusChanged(boolean mount) {
            // TODO Auto-generated method stub
            MyLogger.logD(CLASS_TAG, "onSDCardStatusChanged - " + mount);
            sdCardCheck();
        }
    }
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        getActionBar().getTabAt(0).setText(R.string.backup);
        getActionBar().getTabAt(1).setText(R.string.restore);
        MyLogger.logD(CLASS_TAG, "onConfigurationChanged");
    }

    private void setupActionBar(Bundle state) {

        ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
        actionBar.setDisplayOptions(0, ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE);

        Tab tab = actionBar.newTab();
        tab.setText(R.string.backup);
        tab.setTabListener(new TabListener(mBackupFragment, Constants.BACKUP));
        actionBar.addTab(tab);

        tab = actionBar.newTab();
        tab.setText(R.string.restore);
        tab.setTabListener(new TabListener(mRestoreFragment, Constants.RESTORE));
        actionBar.addTab(tab);

        if(state != null){
            int position = state.getInt(STATE_TAB, 0);
            if(position != 0){
                actionBar.setSelectedNavigationItem(position);
            }
        }
    }

    protected Dialog onCreateDialog(int id, Bundle args) {

        MyLogger.logI(CLASS_TAG, "oncreateDialog, id = " + id);
        switch (id) {
            case DialogID.DLG_DELETE_AND_WAIT: {
                ProgressDialog dialog = new ProgressDialog(this);
                dialog.setCancelable(false);
                dialog.setMessage(getString(R.string.delete_please_wait));
                dialog.setIndeterminate(true);
                return dialog;
            }

            case DialogID.DLG_LOADING: {
                ProgressDialog dialog = new ProgressDialog(this);
                dialog.setCancelable(false);
                dialog.setMessage(getString(R.string.loading_please_wait));
                dialog.setIndeterminate(true);
                return dialog;
            }
            case DialogID.DLG_NO_SDCARD: {
                return new AlertDialog.Builder(this)
                        .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.notice)
                        .setMessage(SDCardUtils.getSDStatueMessage(this))
                        .setCancelable(false)
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                // TODO Auto-generated method stub
                                MainActivity.this.finish();
                            }
                        }).create();
            }
        }
        return null;
    }
    
    /**
     * A TabListener receives event callbacks from the action bar as tabs
     * are deselected, selected, and reselected. A FragmentTransaction
     * is provided to each of these callbacks; if any operations are added
     * to it, it will be committed at the end of the full tab switch operation.
     * This lets tab switches be atomic without the app needing to track
     * the interactions between different tabs.
     *
     * NOTE: This is a very simple implementation that does not retain
     * fragment state of the non-visible tabs across activity instances.
     * Look at the FragmentTabs example for how to do a more complete
     * implementation.
     */
    private class TabListener implements ActionBar.TabListener {
        private Fragment mFragment;
        private String mTag;
        

        public TabListener(Fragment fragment, String tag) {
            mFragment = fragment;
            mTag = tag;
        }

        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            sdCardCheck();
            FragmentManager fm = getFragmentManager();
            Fragment f = fm.findFragmentByTag(mTag);
            if(f == null){
                ft.add(android.R.id.content, mFragment, mTag);
            }
        }

        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
            ft.remove(mFragment);
        }

        public void onTabReselected(Tab tab, FragmentTransaction ft) {
            //do nothing
        }
    }
}
