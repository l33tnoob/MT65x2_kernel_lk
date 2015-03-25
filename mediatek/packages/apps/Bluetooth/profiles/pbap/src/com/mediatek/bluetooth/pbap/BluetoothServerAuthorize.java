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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
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
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;

import com.mediatek.bluetooth.R;

/* This activity is launched for PBAP-server-authorize notification. */
public class BluetoothServerAuthorize extends Activity implements DialogInterface.OnClickListener,
        DialogInterface.OnDismissListener {

    private static final String TAG = "BluetoothServerAuthorize";

    /* Message definition */
    private static final int AUTHORIZE_TIMEOUT_IND = 101;

    /* Result codes */
    public static final int RESULT_USER_ACCEPT = 1;

    public static final int RESULT_USER_REJECT = 2;

    public static final int RESULT_CANCEL = 3;

    public static final int RESULT_TIMEOUT = 4;

    public static final int RESULT_OTHERS = 5;

    /* Package only */
    /* input values */
    public static final String DEVICE_NAME = "com.mediatek.bluetooth.extra.device_name";

    /* action name that owner used to cancel the dialog */
    public static final String ACTION_CANCEL = "com.mediatek.bluetooth.extra.action_cancel";

    /* action that used to send back result to owner */
    public static final String ACTION_RETURN = "com.mediatek.bluetooth.extra.action_return";

    /* Timeout value, 0 means no timeout */
    public static final String TIMEOUT_VALUE = "com.mediatek.bluetooth.extra.timeout_value";

    /* return values */
    public static final String AUTHORIZE_RESULT = "com.mediatek.bluetooth.extra.authorize_result";

    /* Is always allowed */
    public static final String AUTHORIZE_ALWAYS_ALLOWED = "com.mediatek.bluetooth.extra.authorize_always_allowed";

    private View mView;

    private TextView mMessageView;

    private String mSessionKey = "";

    private int mCurrentDialog;

    private CheckBox mAlwaysAllowed;

    private String mDeviceName = null;

    private AlertDialog mInfoDialog = null;

    private IntentFilter mCancelFilter = null;

    private String mReturnAction = null;

    private boolean mResultSent = false;

    private boolean mAlwaysAllowedValue = false;

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case AUTHORIZE_TIMEOUT_IND:
                    sendAuthResult(RESULT_TIMEOUT);
                    break;
                default:
                    Log.e(TAG, "Unsupported indication");
                    break;
            }
        }
    };

    private void sendAuthResult(int res) {
        if (!mResultSent) {
            // TODO: Send result here
            Log.d(TAG, "sendAuthResult(" + res + ")");
            Intent intent = new Intent(mReturnAction);
            intent.putExtra(BluetoothServerAuthorize.AUTHORIZE_RESULT, res);
            intent.putExtra(BluetoothServerAuthorize.AUTHORIZE_ALWAYS_ALLOWED, mAlwaysAllowedValue);
            sendBroadcastAsUser(intent, UserHandle.ALL);
            mResultSent = true;
        }
    }

    private BroadcastReceiver mCancelReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive(" + intent.getAction() + ")");
            sendAuthResult(RESULT_CANCEL);
            mInfoDialog.dismiss();
        }
    };

    private View createView(String msg) {
        mView = getLayoutInflater().inflate(R.layout.access, null);
        mMessageView = (TextView) mView.findViewById(R.id.message);
        if (mMessageView != null) {
            mMessageView.setText(msg);
        }
        mAlwaysAllowed = (CheckBox) mView.findViewById(R.id.alwaysallowed);
        if (mAlwaysAllowed != null) {
            mAlwaysAllowed.setChecked(mAlwaysAllowedValue);
            mAlwaysAllowed.setOnCheckedChangeListener(new OnCheckedChangeListener() {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    Log.d(TAG, "onCheckedChanged : " + String.valueOf(isChecked) + ")");
                    if (isChecked) {
                        mAlwaysAllowedValue = true;
                    } else {
                        mAlwaysAllowedValue = false;
                    }
                }
            });
        }
        return mView;
    }

    private AlertDialog buildDialog(String deviceName) {
        /*
         * final AlertController.AlertParams p = mAlertParams; Log.d(TAG, "buildDialog("+deviceName+")"); p.mIconId =
         * android.R.drawable.ic_dialog_info; p.mTitle = getString(R.string.bluetooth_pbap_server_authorize_title); p.mView =
         * createView (getString(R.string.bluetooth_pbap_server_authorize_message, deviceName)); p.mPositiveButtonText =
         * getString(R.string.bluetooth_pbap_server_authorize_allow); p.mPositiveButtonListener = this; p.mNegativeButtonText
         * = getString(R.string.bluetooth_pbap_server_authorize_decline); p.mNegativeButtonListener = this; setupAlert();
         */
        Builder builder = new Builder(this);
        CharSequence[] items = {
            getString(R.string.bluetooth_pbap_server_authorize_alwaysallowed)
        };
        boolean[] checked = {
            false
        };

        String msg = getString(R.string.bluetooth_pbap_server_authorize_message, deviceName);

        Log.d(TAG, "buildDialog : items=" + items[0]);

        builder.setIcon(android.R.drawable.ic_dialog_info).setTitle(R.string.bluetooth_pbap_server_authorize_title).setView(
                createView(msg))
        // .setMessage(msg)
                /*
                 * .setMultiChoiceItems (items, checked, new DialogInterface.OnMultiChoiceClickListener(){ public void
                 * onClick (DialogInterface dialog, int which, boolean isChecked){ if(which == 0) { mAlwaysAllowedValue =
                 * isChecked; }else{ Log.w(TAG, "index of always allowed is not correct : "+which); } } } )
                 */
                .setPositiveButton(R.string.bluetooth_pbap_server_authorize_allow, this).setNegativeButton(
                        R.string.bluetooth_pbap_server_authorize_decline, this);

        return builder.create();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.d(TAG, "onCreate()");
        Intent intent = getIntent();
        String deviceName = intent.getStringExtra(DEVICE_NAME);
        String cancel = intent.getStringExtra(ACTION_CANCEL);
        mReturnAction = intent.getStringExtra(ACTION_RETURN);
        int timeout = intent.getIntExtra(TIMEOUT_VALUE, -1);
        Log.d(TAG, "Device name: " + deviceName);
        Log.d(TAG, "cancel filter : " + cancel);

        /* Verify intent */
        /*
         * android.os.Bundle b = intent.getExtras(); java.util.Set<String> s = b.keySet(); Log.e(TAG,
         * "Start intent extra listing"); for( String ss : s ){ android.util.Log.e("INTENT", "Intent extra key: [" + ss + "]"
         * ); } Log.e(TAG, "End intent extra listing");
         */
        /* Verify intent */

        if (cancel != null) {
            mCancelFilter = new IntentFilter(cancel);
        } else {
            mCancelFilter = null;
        }
        mInfoDialog = buildDialog(deviceName);
        mInfoDialog.setOnDismissListener(this);
        mInfoDialog.show();
        // buildDialog(deviceName);
        /* Start timer here */
        if (timeout > 0) {
            mHandler.sendMessageDelayed(mHandler.obtainMessage(AUTHORIZE_TIMEOUT_IND), timeout);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume()");
        if (mCancelFilter != null) {
            registerReceiver(mCancelReceiver, mCancelFilter);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause()");
        try {
            unregisterReceiver(mCancelReceiver);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "", e);
        }
        sendAuthResult(RESULT_OTHERS);
        mInfoDialog.dismiss();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy()");
    }

    /* Functions for DialogInterface.OnClickListener */
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                Log.d(TAG, "Positive button pressed.");
                sendAuthResult(RESULT_USER_ACCEPT);
                break;

            case DialogInterface.BUTTON_NEGATIVE:
                Log.d(TAG, "Negative button pressed.");
                sendAuthResult(RESULT_USER_REJECT);
                break;

            default:
                break;
        }
        dialog.dismiss();
    }

    public void onDismiss(DialogInterface dialog) {
        finish();
    }
}
