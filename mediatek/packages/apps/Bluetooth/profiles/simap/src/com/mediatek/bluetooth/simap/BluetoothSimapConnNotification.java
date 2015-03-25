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


package com.mediatek.bluetooth.simap;

import com.mediatek.bluetooth.R;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Button;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.text.format.Formatter;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import android.bluetooth.BluetoothAdapter;


/**
 * This class is designed to ask user to confirm if accept incoming connection request;
 */
public class BluetoothSimapConnNotification extends AlertActivity implements
        DialogInterface.OnClickListener {
    private static final String TAG = "BluetoothSimapConnNotification";
    private static final boolean D = true;//Constants.DEBUG;
    private static final boolean V = true;//Constants.VERBOSE;

    private View mContentView;

    private TextView messageView;

    private static final int DISMISS_DISCONNECTED_DIALOG = 0;

    private static final int DISMISS_TIMEOUT_DIALOG_VALUE = 1500;


    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
 			String action = intent.getAction();
       	
            if (action.equals(BluetoothSimapService.BTSIMAP_DISCONNECTED)) {
            	onDisconnected();
			} else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
	
				switch (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
										   BluetoothAdapter.ERROR)) {
				case BluetoothAdapter.STATE_TURNING_OFF:
					if (V) Log.v(TAG, "Received BluetoothAdapter.STATE_TURNING_OFF.");
					finish();
					break;
				}
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {

		Log.i(TAG, "onCreate...");
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();

        // Set up the "dialog"
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_info;
        p.mTitle = getString(R.string.simap_disconnect_title);
        p.mView = createView();
        p.mPositiveButtonText = getString(R.string.simap_disconnect_yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.simap_disconnect_no);
        p.mNegativeButtonListener = this;

        setupAlert();

		IntentFilter filter = new IntentFilter(
                BluetoothSimapService.BTSIMAP_DISCONNECTED);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
		
        registerReceiver(mReceiver, filter);		
    }

    private View createView() {

        String mRemoteName = BluetoothSimapService.getRemoteDeviceName();

		mContentView = getLayoutInflater().inflate(R.layout.bt_simap_conn_notification, null);
		messageView = (TextView)mContentView.findViewById(R.id.message);
		String mMessage1 = getString(R.string.simap_disconnect_message, mRemoteName);
		if (messageView != null)
	   	    messageView.setText(mMessage1);

        return mContentView;
    }

    public void onClick(DialogInterface dialog, int which) {
		Log.i(TAG, "onClick:" + which);
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
				Log.i(TAG, "DialogInterface.BUTTON_POSITIVE");
				sendDisconnectRequestIntent();
				Log.i(TAG, "call finish()...");
				finish();
                break;

            case DialogInterface.BUTTON_NEGATIVE:
				Log.i(TAG, "DialogInterface.BUTTON_NEGATIVE");
                break;
        }
    }

	/*		
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (D) Log.d(TAG, "onKeyDown(): back key");

        }
        return true;
    }
*/
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

	private void sendDisconnectRequestIntent() {
		Log.d(TAG, "sendDisconnectIntent"); 	
		//send DISCONNECT_REQUEST intent
		Intent intent = new Intent(BluetoothSimapService.DISCONNECT_REQUEST);
		sendBroadcast(intent);
	}

    private void onDisconnected() {
		Log.i(TAG, "onDisconnected...");
        messageView.setText(getString(R.string.simap_disconnected_message,
                BluetoothSimapService.getRemoteDeviceName()));
        mAlert.getButton(DialogInterface.BUTTON_NEGATIVE).setVisibility(View.GONE);
        mAlert.getButton(DialogInterface.BUTTON_POSITIVE).setText("OK");

        mHandler.sendMessageDelayed(mHandler.obtainMessage(DISMISS_DISCONNECTED_DIALOG),
                DISMISS_TIMEOUT_DIALOG_VALUE);
    }


    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
				case DISMISS_DISCONNECTED_DIALOG:
					if (V) Log.v(TAG, "Received DISMISS_DISCONNECTED_DIALOG msg.");
					finish();
					break;
                default:
                    break;
            }
        }
    };

	
}

