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

import java.io.IOException;
import java.util.ArrayList;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;

import com.mediatek.datatransfer.BackupEngine.BackupResultType;
import com.mediatek.datatransfer.BackupService.BackupBinder;
import com.mediatek.datatransfer.BackupService.OnBackupStatusListener;
import com.mediatek.datatransfer.CheckedListActivity.OnCheckedCountChangedListener;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.utils.Constants.DialogID;
import com.mediatek.datatransfer.utils.Constants.MessageID;
import com.mediatek.datatransfer.utils.Constants.State;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Utils;

public abstract class AbstractBackupActivity extends CheckedListActivity implements
        OnCheckedCountChangedListener {

    private String CLASS_TAG = MyLogger.LOG_TAG + "/AbstractBackupActivity";
    //test
    protected BaseAdapter mAdapter;
    private Button mButtonBackup;
    //private Button mButtonSelect;
    private CheckBox mCheckBoxSelect;
    private View mDivider ;
    protected ProgressDialog mProgressDialog;
    protected ProgressDialog mCancelDlg;
    protected Handler mHandler;
    protected BackupBinder mBackupService;
    private OnBackupStatusListener mBackupListener;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.backup);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        init();
        Log.i(CLASS_TAG, "onCreate");
        if (savedInstanceState != null) {
            updateButtonState();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(CLASS_TAG, "onResume");
        mAdapter = initBackupAdapter();
        setListAdapter(mAdapter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(CLASS_TAG, "onDestroy");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        if (mBackupService != null && mBackupService.getState() == State.INIT) {
            stopService();
        }
        if (mBackupService != null) {
            mBackupService.setOnBackupChangedListner(null);
        }
        unBindService();
        mHandler = null;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU && event.isLongPress()) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            // app icon in action bar clicked; go home
            Intent intent = new Intent(this, MainActivity.class);
            intent.putExtra(MainActivity.ACTION_WHERE, MainActivity.ACTION_BACKUP);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private void init() {
        this.bindService();
        registerOnCheckedCountChangedListener(this);
        initButton();
        initHandler();
        initLoadingView();
        createProgressDlg();
        // mAdapter = initBackupAdapter();
        // setListAdapter(mAdapter);
    }

    LinearLayout loadingContent = null;

    private void initLoadingView() {
        // TODO Auto-generated method stub
        loadingContent = (LinearLayout) findViewById(R.id.loading_container);
    }

    protected void showLoadingContent(boolean show) {
        findViewById(R.id.loading_container).setVisibility(show ? View.VISIBLE : View.GONE);
        findViewById(R.id.backup_content).setVisibility(!show ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onCheckedCountChanged() {
        mAdapter.notifyDataSetChanged();
        updateButtonState();
    }

    public void setOnBackupStatusListener(OnBackupStatusListener listener) {
        mBackupListener = listener;
        if (mBackupListener != null&&mBackupService != null) {
            mBackupService.setOnBackupChangedListner(mBackupListener);
        }
    }

    private ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getString(R.string.backuping));
            mProgressDialog.setCancelable(true);
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setCancelMessage(mHandler.obtainMessage(MessageID.PRESS_BACK));
        }
        return mProgressDialog;
    }

    private ProgressDialog createCancelDlg() {
        if (mCancelDlg == null) {
            mCancelDlg = new ProgressDialog(this);
            mCancelDlg.setMessage(getString(R.string.cancelling));
            mCancelDlg.setCancelable(false);
        }
        return mCancelDlg;
    }

    protected void showProgress() {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.show();
    }

    protected boolean errChecked() {
        boolean ret = false;
        String path = SDCardUtils.getStoragePath(this);
        if (path == null) {
            // no sdcard
            Log.d(CLASS_TAG, "SDCard is removed");
            ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_SDCARD_REMOVED);
                    }
                });
            }
        } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
            // no space
            Log.d(CLASS_TAG, "SDCard is full");
            ret = true;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_SDCARD_FULL);
                    }
                });
            }
        } else {
            Log.e(CLASS_TAG, "unkown error");
        }
        return ret;
    }

    private void initButton() {
        mDivider = findViewById(R.id.backup_divider);
        mDivider.setBackground(getListView().getDivider());
        mButtonBackup = (Button) findViewById(R.id.backup_bt_backcup);
        mButtonBackup.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(final View view) {
                if (mBackupService == null || mBackupService.getState() != State.INIT) {
                    Log.e(CLASS_TAG, "Can not to start. BackupService not ready or BackupService is ruuning");
                    return;
                }

                if (isAllChecked(false)) {
                    Log.e(CLASS_TAG, "to Backup List is null or empty");
                    return;
                }
                String path = SDCardUtils.getStoragePath(AbstractBackupActivity.this);
                if (path != null) {
                    if (Utils.getWorkingInfo() < 0) {
                        startBackup();
                    } else {
                        showDialog(DialogID.DLG_RUNNING);
                    }
                } else {
                    // scard not available
                    showDialog(DialogID.DLG_NO_SDCARD);
                }
            }
        });

        mCheckBoxSelect = (CheckBox) findViewById(R.id.backup_checkbox_select);
        mCheckBoxSelect.setChecked(true);
        mCheckBoxSelect.setVisibility(View.INVISIBLE);
        mCheckBoxSelect.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(final View view) {
                if (isAllChecked(true)) {
                    setAllChecked(false);
                } else {
                    setAllChecked(true);
                }
            }
        });
    }

    protected void setButtonsEnable(boolean enable) {
        MyLogger.logD(CLASS_TAG, "setButtonsEnable - " + enable);
        if (mButtonBackup != null) {
            mButtonBackup.setEnabled(enable);
        }
        if (mCheckBoxSelect != null) {
            mCheckBoxSelect.setEnabled(enable);
            // mCheckBoxSelect.setVisibility(enable?View.VISIBLE:View.INVISIBLE);
            // findViewById(R.id.divider).setVisibility(enable?View.VISIBLE:View.INVISIBLE);
        }
    }

    protected void updateButtonState() {
        mCheckBoxSelect.setVisibility(View.VISIBLE);
        mCheckBoxSelect.setText(getApplication().getResources().getString(R.string.selectall));
        mDivider.setVisibility(View.VISIBLE);
        if (isAllChecked(false)) {
            mButtonBackup.setEnabled(false);
            mCheckBoxSelect.setChecked(false);
        } else {
            mButtonBackup.setEnabled(true);
            mCheckBoxSelect.setChecked(isAllChecked(true));
        }
    }

    protected final void initHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(final Message msg) {
                switch (msg.what) {
                case MessageID.PRESS_BACK:
                    if (mBackupService != null && mBackupService.getState() != State.INIT
                            && mBackupService.getState() != State.FINISH) {
                        mBackupService.pauseBackup();
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_CANCEL_CONFIRM);
                    }
                    break;
                default:
                    break;
                }
            }
        };
    }

    @Override
    protected Dialog onCreateDialog(final int id, final Bundle args) {
        Dialog dialog = null;
        switch (id) {
        case DialogID.DLG_CANCEL_CONFIRM:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setTitle(R.string.warning).setMessage(R.string.cancel_backup_confirm)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface arg0, final int arg1) {
                            if (mBackupService != null && mBackupService.getState() != State.INIT
                                    && mBackupService.getState() != State.FINISH) {
                                if (mCancelDlg == null) {
                                    mCancelDlg = createCancelDlg();
                                }
                                mCancelDlg.show();
                                mBackupService.cancelBackup();
                            }
                        }
                    }).setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                        public void onClick(final DialogInterface arg0, final int arg1) {

                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.continueBackup();
                            }
                            if (mProgressDialog != null) {
                                mProgressDialog.show();
                            }
                        }
                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_SDCARD_REMOVED:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setTitle(R.string.warning).setMessage(R.string.sdcard_removed)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.cancelBackup();
                            }
                        }

                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_SDCARD_FULL:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setTitle(R.string.warning).setMessage(R.string.sdcard_is_full)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            if (mBackupService != null && mBackupService.getState() == State.PAUSE) {
                                mBackupService.cancelBackup();
                            }
                        }
                    }).setCancelable(false).create();
            break;
        case DialogID.DLG_RUNNING:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.warning)
                    .setMessage(R.string.state_running)
                    .setPositiveButton(android.R.string.ok, null).create();
            break;
            
        case DialogID.DLG_NO_SDCARD:
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.notice)
                    .setMessage(SDCardUtils.getSDStatueMessage(this))
                    .setPositiveButton(android.R.string.ok, null).create();
            break;

        case DialogID.DLG_CREATE_FOLDER_FAILED:
            String name = args.getString("name");
            String msg = String.format(getString(R.string.create_folder_fail), name);
            dialog = new AlertDialog.Builder(AbstractBackupActivity.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(R.string.notice)
                    .setMessage(msg).setPositiveButton(android.R.string.ok, null).create();
            break;

        default:
            break;
        }
        return dialog;
    }

    /**
     * when backup button click, if can start backup, will call startBackup
     */
    public abstract void startBackup();

    /**
     * init Backup Adapter, when activity will can this function. after call
     * this, activity can't change the adapter.
     * 
     * @return
     */
    public abstract BaseAdapter initBackupAdapter();

    /**
     * after service connected, will can the function, the son activity can do
     * anything that need service connected
     */
    protected abstract void afterServiceConnected();

    @Override
    public void onConfigurationChanged(final Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e(CLASS_TAG, "onConfigurationChanged");
    }

    /**
     * after service connected and data initialed, to check restore state to
     * restore UI. only to check once after onCreate, always used for activity
     * has been killed in background.
     */
    protected void checkBackupState() {
        if (mBackupService != null) {
            int state = mBackupService.getState();
            switch (state) {
            case State.ERR_HAPPEN:
                errChecked();
                break;
            default:
                break;
            }
        }
    }

    private void bindService() {
        this.getApplicationContext().bindService(new Intent(this, BackupService.class),
                mServiceCon, Service.BIND_AUTO_CREATE);
    }

    private void unBindService() {
        if (mBackupService != null) {
            mBackupService.setOnBackupChangedListner(null);
        }
        this.getApplicationContext().unbindService(mServiceCon);
    }

    protected void startService() {
        this.startService(new Intent(this, BackupService.class));
    }

    protected void stopService() {
        if (mBackupService != null) {
            mBackupService.reset();
        }
        this.stopService(new Intent(this, BackupService.class));
    }

    private ServiceConnection mServiceCon = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service) {
            mBackupService = (BackupBinder) service;
            if (mBackupService != null) {
                if (mBackupListener != null) {
                    mBackupService.setOnBackupChangedListner(mBackupListener);
                }
            }
            // checkBackupState();
            afterServiceConnected();
            Log.i(CLASS_TAG, "onServiceConnected");
        }

        @Override
        public void onServiceDisconnected(final ComponentName name) {
            mBackupService = null;
            Log.i(CLASS_TAG, "onServiceDisconnected");
        }
    };

    public class NomalBackupStatusListener implements OnBackupStatusListener {
        @Override
        public void onBackupEnd(final BackupResultType resultCode,
                final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {
            // do nothing
        }

        @Override
        public void onBackupErr(final IOException e) {
            if (errChecked()) {
                if (mBackupService != null && mBackupService.getState() != State.INIT
                        && mBackupService.getState() != State.FINISH) {
                    mBackupService.pauseBackup();
                }
            }
        }

        @Override
        public void onComposerChanged(final Composer composer) {

        }

        @Override
        public void onProgressChanged(final Composer composer, final int progress) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            }
        }
    }

}
