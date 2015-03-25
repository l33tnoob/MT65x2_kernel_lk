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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.pbap;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.UserHandle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.bluetooth.R;


public class BluetoothAuthenticating extends AlertActivity implements DialogInterface.OnClickListener {

    private static final String TAG = "BluetoothAuthenticating";

    private AlertController.AlertParams mPara;

    private View mView = null;

    private EditText mPasscodeEdit;

    private IntentFilter mCancelFilter = null;

    private String mReturnAction = null;

    private boolean mResultSent = false;

    /* Message definition */
    private static final int AUTHETICATE_TIMEOUT_IND = 101;

    /* Result codes */
    public static final int RESULT_USER_ACCEPT = 1;

    public static final int RESULT_USER_REJECT = 2;

    public static final int RESULT_CANCEL = 3;

    public static final int RESULT_TIMEOUT = 4;

    public static final int RESULT_OTHERS = 5;

    /* input values */
    public static final String DEVICE_NAME = "com.mediatek.bluetooth.extra.device_name";

    /* action name that owner used to cancel the dialog */
    public static final String ACTION_CANCEL = "com.mediatek.bluetooth.extra.action_cancel";

    /* action that used to send back result to owner */
    public static final String ACTION_RETURN = "com.mediatek.bluetooth.extra.action_return";

    /* Timeout value, 0 means no timeout */
    public static final String TIMEOUT_VALUE = "com.mediatek.bluetooth.extra.timeout_value";

    /* return values */
    public static final String AUTHENTICATE_RESULT = "com.mediatek.bluetooth.extra.authenticate_result";

    public static final String AUTHETICATE_CODE = "com.mediatek.bluetooth.extra.autheticate_code";

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case AUTHETICATE_TIMEOUT_IND:
                    sendResult(RESULT_TIMEOUT);
                    break;
                default:
                    Log.e(TAG, "Unsupported indication");
                    break;
            }
        }
    };

    private BroadcastReceiver mCancelReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive()");
            sendResult(RESULT_CANCEL);
            BluetoothAuthenticating.this.finish();
        }
    };

    private void sendResult(int res) {
        Log.d(TAG, "Authenticate : " + res);
        if (!mResultSent) {
            // TODO: Send result here
            Intent intent = new Intent(mReturnAction);
            intent.putExtra(AUTHENTICATE_RESULT, res);
            intent.putExtra(AUTHETICATE_CODE, mPasscodeEdit.getText().toString());
            sendBroadcastAsUser(intent, UserHandle.ALL);
            mResultSent = true;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.v(TAG, "OnCreate");

        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        // String action = intent.getAction();
        // String deviceName = intent.getStringExtra(DEVICE_NAME);
        String cancel = intent.getStringExtra(ACTION_CANCEL);
        mReturnAction = intent.getStringExtra(ACTION_RETURN);
        int timeout = intent.getIntExtra(TIMEOUT_VALUE, -1);
        if (cancel != null) {
            mCancelFilter = new IntentFilter(intent.getStringExtra(ACTION_CANCEL));
        } else {
            mCancelFilter = null;
        }

        setUpDialog();
        /* Start timer here */
        if (timeout > 0) {
            mHandler.sendMessageDelayed(mHandler.obtainMessage(AUTHETICATE_TIMEOUT_IND), timeout);
        }
    }

    @Override
    protected void onStart() {
        Log.v(TAG, "onStart");
        super.onStart();
        if (mCancelFilter != null) {
            registerReceiver(mCancelReceiver, mCancelFilter);
        }
    }

    @Override
    protected void onDestroy() {
        Log.v(TAG, "onDestroy()");
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        Log.v(TAG, "onStop()");
        if (mCancelFilter != null) {
            unregisterReceiver(mCancelReceiver);
        }
        /* Send result in case result is not sent yet */
        sendResult(RESULT_OTHERS);
        super.onStop();
    }

    private void setUpDialog() {
        Log.v(TAG, "setUpDialog");

        // final AlertController.AlertParams p = mAlertParams;
        mPara = mAlertParams;
        mPara.mIconId = android.R.drawable.ic_dialog_info;
        mPara.mTitle = getString(R.string.bluetooth_pbap_server);

        mPara.mPositiveButtonText = getString(R.string.bluetooth_pbap_server_auth_ok);
        mPara.mPositiveButtonListener = this;
        mPara.mNegativeButtonText = getString(R.string.bluetooth_pbap_server_auth_cancel);
        mPara.mNegativeButtonListener = this;

        mPara.mView = createView();
        setupAlert();
    }

    private View createView() {
        Log.v(TAG, "createView");

        mView = getLayoutInflater().inflate(R.layout.bt_authenticating_dialog, null);

        mPasscodeEdit = (EditText) mView.findViewById(R.id.pass_code_edit);

        return mView;
    }

    public void onClick(DialogInterface dialog, int which) {
        Log.v(TAG, "onClick");

        if (which == DialogInterface.BUTTON_POSITIVE) {
            Log.v(TAG, "positive button");
            if (mPasscodeEdit.getText().toString().length() > 0) {
                sendResult(RESULT_USER_ACCEPT);
            } else {
                Log.e(TAG, "passcode is null");
                sendResult(RESULT_USER_REJECT);
            }
        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Log.v(TAG, "negative button");
            sendResult(RESULT_USER_REJECT);
        }
        finish();
    }
}
