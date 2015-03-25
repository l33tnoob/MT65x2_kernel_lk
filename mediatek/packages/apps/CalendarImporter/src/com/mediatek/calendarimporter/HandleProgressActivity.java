/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.calendarimporter;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.CalendarContract.Calendars;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.calendarimporter.BindServiceHelper.ServiceConnectedOperation;
import com.mediatek.calendarimporter.service.ImportProcessor;
import com.mediatek.calendarimporter.service.ProcessorMsgType;
import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.Utils;
import com.mediatek.vcalendar.VCalendarException;

import java.util.ArrayList;

public class HandleProgressActivity extends Activity implements OnCancelListener, OnClickListener,
        ServiceConnectedOperation {
    private static final String TAG = "HandleProgressActivity";

    private boolean mFirstEnter;
    private VCalService mService;
    private Handler mHandler;
    private static final String DATA_URI = "DataUri";
    private static final String ACCOUNT_NAME = "TargetAccountName";
    private ImportProcessor mProcessor;
    private String mAccountName;
    // private boolean mIsProcessShowed = false;
    private BindServiceHelper mServiceHelper;

    // A flag that indicate the dialog that created to show no calendar.
    private static final int ID_DIALOG_NO_CALENDAR_ALERT = 1;
    private static final int ID_DIALOG_PROGRESS_BAR = 2;
    // private static final int HUNDRED = 100;

    private Uri mDataUri;
    private ListView mAccountList = null;
    private ProgressDialog mProgressDialog;
    private AlertDialog mAlertDialog;

    protected static final String BUNDLE_KEY_START_MILLIS = "key_start_millis";

    private final DialogInterface.OnClickListener mDialogListener = new OnClickListener() {
        public void onClick(DialogInterface dialog, int button) {
            switch (button) {
            case AlertDialog.BUTTON_POSITIVE:
                // retry
                addParseRequest();
                break;
            case AlertDialog.BUTTON_NEGATIVE:
                if (mService != null) {
                    mService.tryCancelProcessor(mProcessor);
                }
                // finish();
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mDataUri = getIntent().getData();
        mFirstEnter = true;

        if (Utils.hasExchangeOrGoogleAccount(this)) {
            mServiceHelper = new BindServiceHelper(this);
            mServiceHelper.onBindService();
            showAccountListView();
        } else {
            LogUtils.e(TAG, "onCreate, should not be created when no account exists.");
            finish();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (!hasFocus) {
            mFirstEnter = false;
            return;
        }
        if (Utils.hasExchangeOrGoogleAccount(this)) {
            showAccountListView();
        } else {
            if (!mFirstEnter) {
                finish();
            }
        }
        super.onWindowFocusChanged(hasFocus);
    }

    /**
     * show account list view for user to select.
     */
    private void showAccountListView() {
        setContentView(R.layout.account);

        int color = Utils.getThemeMainColor(this, Utils.DEFAULT_COLOR);
        if (color != Utils.DEFAULT_COLOR) {
            TextView view = (TextView) findViewById(R.id.account_title);
            view.setTextColor(color);
            findViewById(R.id.account_devide_line).setBackgroundColor(color);
        }

        mAccountList = (ListView) findViewById(R.id.account_list);
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.list_adapter, getAccount());
        mAccountList.setAdapter(adapter);
        setTitleColor(Color.GRAY);
        mAccountList.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> arg0, View view, int arg2, long arg3) {
                mAccountName = (String) ((TextView) view).getText();
                String select = Calendars.ACCOUNT_NAME + "=\"" + mAccountName + "\"";
                LogUtils.d(TAG, "showAccountListView() Select = " + select);
                addParseRequest();
                // mIsProcessShowed = false;
                mAccountList.setEnabled(false);
            }
        });
    }

    /**
     * when no calendar exists. show 'no calendar' alert dialog
     */
    protected void onSaveInstanceState(Bundle outState) {
        outState.putParcelable(DATA_URI, mDataUri);
        outState.putString(ACCOUNT_NAME, mAccountName);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mDataUri = savedInstanceState.getParcelable(DATA_URI);
        mAccountName = savedInstanceState.getString(ACCOUNT_NAME);
    }

    private String[] getAccount() {
        final Account[] account = AccountManager.get(getApplicationContext()).getAccounts();
        ArrayList<String> accountList = new ArrayList<String>();

        for (int i = 0; i < account.length; i++) {
            if (Utils.isExchangeOrGoogleAccount(account[i])) {
                accountList.add(account[i].name);
            }
        }
        return accountList.toArray(new String[accountList.size()]);
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        finish();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        finish();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        LogUtils.d(TAG, "onCreateDialog,id=" + id);
        Dialog dialog = null; 

        if (ID_DIALOG_NO_CALENDAR_ALERT == id) {
            AlertDialog.Builder builder = new AlertDialog.Builder(HandleProgressActivity.this);
            builder.setTitle(R.string.no_syncable_calendars).setIcon(R.drawable.ic_dialog_alert_holo_light).setMessage(
                    R.string.no_calendars_found).setPositiveButton(R.string.retry, mDialogListener).setNegativeButton(
                    R.string.give_up, mDialogListener);
            mAlertDialog = builder.create();
            dialog = mAlertDialog;
        } else if (ID_DIALOG_PROGRESS_BAR == id) {
            mProgressDialog = new ProgressDialog(HandleProgressActivity.this);
            mProgressDialog.setTitle(R.string.import_title);
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setCancelable(false);
            mProgressDialog.setIndeterminate(false);
            mProgressDialog.setButton(getText(R.string.give_up), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (mService != null && mProcessor != null) {
                        mService.tryCancelProcessor(mProcessor);
                        mProgressDialog.dismiss();
                    }
                }
            });
            dialog = mProgressDialog;
        }
        return dialog;
    }

    private void addParseRequest() {
        if (mService != null && mHandler != null && mAccountName != null && mDataUri != null) {
            LogUtils.d(TAG, "addParseRequest. AccountName = " + mAccountName);
            mProcessor = new ImportProcessor(this, mAccountName, mHandler, mDataUri);
            mService.tryExecuteProcessor(mProcessor);
        }
    }

    @Override
    protected void onDestroy() {
        LogUtils.d(TAG, "onDestroy() + mService:" + mService);
        mServiceHelper.unBindService();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        LogUtils.d(TAG, "onBackPressed() + mService:" + mService);
        if (mService != null) {
            mService.tryCancelProcessor(mProcessor);
        }
        super.onBackPressed();
    }

    @Override
    public void serviceConnected(VCalService service) {
        mService = service;
        mHandler = new Handler() {

            public void handleMessage(Message msg) {
                switch (msg.what) {
                // the given account is not one of calendars table
                case ProcessorMsgType.PROCESSOR_EXCEPTION:
                    LogUtils.i(TAG, "serviceConnected. ProcessorMsgType:PROCESSOR_EXCEPTION. type = " + msg.arg2);
                    if (VCalendarException.NO_ACCOUNT_EXCEPTION == msg.arg2) {
                        showDialog(ID_DIALOG_NO_CALENDAR_ALERT);
                    } else {
                        Toast.makeText(HandleProgressActivity.this, R.string.import_vcs_failed, Toast.LENGTH_SHORT).show();
                        HandleProgressActivity.this.finish();
                    }
                    break;
                // start progress, show process bar.
                case ProcessorMsgType.PROCESSOR_STATUS_UPDATE:
                    /*
                     * for only handle one event, do not show PROGRESS_BAR if
                     * (!mIsProcessShowed) { showDialog(ID_DIALOG_PROGRESS_BAR);
                     * mIsProcessShowed = true; } if (mProgressDialog != null &&
                     * mProgressDialog.isShowing()) {
                     * mProgressDialog.setProgress((HUNDRED * msg.arg1) /
                     * msg.arg2);
                     * mProgressDialog.setProgressNumberFormat(msg.arg1 + "/" +
                     * msg.arg2); }
                     */
                    break;
                case ProcessorMsgType.PROCESSOR_FINISH: // progress finished.
                    if (mProgressDialog != null) {
                        mProgressDialog.dismiss();
                    }
                    LogUtils.i(TAG, "serviceConnected,ProcessorMsgType:PROCESSOR_FINISH. Start result Activity.");
                    Intent intent = new Intent();
                    intent.setClass(HandleProgressActivity.this, ShowHandleResultActivity.class);
                    intent.putExtra("SucceedCnt", msg.arg1);
                    intent.putExtra("totalCnt", msg.arg2);
                    intent.putExtra("accountName", mAccountName);

                    Bundle eventInfo = (Bundle) msg.obj;
                    long startMills = eventInfo.getLong(BUNDLE_KEY_START_MILLIS, -1);
                    LogUtils.d(TAG, "serviceConnected,ProcessorMsgType:PROCESSOR_FINISH. DtStart = "
                            + startMills);
                    intent.putExtra("eventStartTime", startMills);
                    startActivity(intent);
                    HandleProgressActivity.this.finish();
                    break;
                default:
                    break;
                }
            }
        };
    }

    @Override
    public void serviceUnConnected() {
        mService = null;
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        if (mService == null) {
            mServiceHelper = new BindServiceHelper(this);
            mServiceHelper.onBindService();
        }
        mDataUri = intent.getData();
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
        }
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            if (mService != null) {
                mService.tryCancelProcessor(mProcessor);
            }
        }
        if (Utils.hasExchangeOrGoogleAccount(this)) {
            showAccountListView();
        } else {
            LogUtils.e(TAG, "onNewIntent, should not continue when no account exists.");
            finish();
        }
    }
}
