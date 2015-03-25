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

package com.mediatek.bluetooth.time.client;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ResultReceiver;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.IBluetoothTimec;

/* This activity is designed for PTS and debug purpose */
public class TimeClientSettings extends PreferenceActivity
		implements Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener {

	private static final String TAG = "TimeClientSettings";

	// Preference key strings
	private static final String PREF_KEY_SERVER_NAME	= "bt_timec_connect_group",
								PREF_KEY_CONNECT		= "bt_timec_connect",
								PREF_KEY_AUTO			= "bt_timec_automatic",
								PREF_KEY_SYNC			= "bt_timec_sync",
								PREF_KEY_UPDATE_SERVER	= "bt_timec_update_server",
								PREF_KEY_DST			= "bt_timec_dst";

	private static final int TIME_CLIENT_BASE	= 0;

	private static final int OP_BASE					= TIME_CLIENT_BASE,
							 OP_REGISTER_CB				= OP_BASE + 1,
							 OP_UNREGISTER_CB			= OP_BASE + 2,
							 OP_CONNECT					= OP_BASE + 3,
							 OP_DISCONNECT				= OP_BASE + 4,
							 OP_GET_TIME				= OP_BASE + 5,
							 OP_GET_AUTO_CONFIG			= OP_BASE + 6,
							 OP_SET_AUTO_CONFIG			= OP_BASE + 7,
							 OP_GET_DST					= OP_BASE + 8,
							 OP_REQUEST_SERVER_UPDATE	= OP_BASE + 9,
							 OP_CANCEL_SERVER_UPDATE	= OP_BASE + 10,
							 OP_GET_UPDATE_STATUS		= OP_BASE + 11;

	protected static final String ACTION_START	= "com.mediatek.bluetooth.time.client.settings.action.START";

	private BluetoothDevice mDevice;

	private CheckBoxPreference mConnPref;

	private CheckBoxPreference mAutoPref;

	private Preference mSyncPref;

	private Preference mUpdateServerPref;

	private ProgressDialog mProgressDialog;

	private IBluetoothTimec mTimeClient;

	private ServiceConnection mServiceConn = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			mTimeClient = IBluetoothTimec.Stub.asInterface(service);

			if (mTimeClient == null) {
				Utils.logE(TAG, "Failed to get IBinder object.");
				finish();
				return;
			}

			execOp(OP_REGISTER_CB);
			mConnPref.setEnabled(true);
		}

		public void onServiceDisconnected(ComponentName name) {
			Utils.logE(TAG, "Service disconnected!!");
			// Toast.makeText(getApplicationContext(), R.string.bt_timec_service_disconnected, Toast.LENGTH_LONG).show();
			finish();
		}
	};

	private ResultReceiver mResultReceiver = new ResultReceiver(new Handler()) {
		public void onReceiveResult(int resultCode, Bundle resultData) {
			int rsp = -1;
			if (resultData != null) {
				rsp = resultData.getByte("rspcode", Utils.RSP_UNKNOWN);
			}

			switch (resultCode) {
				case TimeClientMsg.MSG_ID_BT_TIMEC_CONNECT_CNF:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_CONNECT_CNF, rsp: " + rsp);

					dismissProgressDialog();
					mConnPref.setEnabled(true);

					if (rsp != Utils.RSP_SUCCESS) {
						// showToast(R.string.bt_timec_connection_failed);
					} else {
						mConnPref.setChecked(true);
					}
					break;

				case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_IND:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_DISCONNECT_IND, rsp: " + rsp);

					dismissProgressDialog();
					mConnPref.setEnabled(true);
					mConnPref.setChecked(false);
					break;

				case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_CNF:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_GET_CTTIME_CNF, rsp: " + rsp);
					dismissProgressDialog();
					break;

				default:
					// Do nothing.
					break;
			}
		}
	};

/**************************************************************************************************
 * Interface implementing functions
 **************************************************************************************************/
 	// For implementing Handler.Callback
	public boolean handleMessage(Message msg) {
		return false;
	}

	// For implementing Preference.OnPreferenceChangeListener
	public boolean onPreferenceChange(Preference pref, Object newValue) {
		String key = pref.getKey();

		// For debug
		Utils.logD(TAG, "Pref: " + key + ", new value: " + newValue);

		if (PREF_KEY_CONNECT.equals(key)) {
			if ((Boolean) newValue) {
				// showProgressDialog(R.string.bt_timec_connecting_title,
				//		R.string.bt_timec_connecting_msg, null);
				execOp(OP_CONNECT);

			} else {
				// showProgressDialog(R.string.bt_timec_disconnecting_title,
				//		R.string.bt_timec_disconnecting_msg, null);
				execOp(OP_DISCONNECT);
			}

			mConnPref.setEnabled(false);
			return false;

		} else if (PREF_KEY_AUTO.equals(key)) {
			return false;

		} else {
			return true;
		}
	}

	// For implementing Preference.OnPreferenceClickListener
	public boolean onPreferenceClick(Preference pref) {
		String key = pref.getKey();

		Utils.logD(TAG, "Pref: " + key);

		if (PREF_KEY_SYNC.equals(key)) {
			// showProgressDialog(R.string.bt_timec_synchronizing_title,
			//		R.string.bt_timec_synchronizing_msg, null);
			execOp(OP_GET_TIME);

		} else if (PREF_KEY_UPDATE_SERVER.equals(key)) {
			// Under construction
			// showProgressDialog(R.string.bt_timec_server_updating_title,
			//		R.string.bt_timec_server_updating_msg, null);
		}

		return true;
	}

/**************************************************************************************************
 * Activity life-cycle functions
 **************************************************************************************************/
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// setTitle(R.string.bt_timec_settings_group_title);
		// addPreferencesFromResource(R.xml.bt_timec_settings_eng);

		if (!applyServerName()) {
			finish();
			return;
		}

		try {
			mConnPref = (CheckBoxPreference) findAndSetPrefChangeListener(PREF_KEY_CONNECT);
			mAutoPref = (CheckBoxPreference) findAndSetPrefChangeListener(PREF_KEY_AUTO);
			mSyncPref = findAndSetPrefClickListener(PREF_KEY_SYNC);
			mUpdateServerPref = findAndSetPrefClickListener(PREF_KEY_UPDATE_SERVER);

		} catch (Exception ex) {
			Utils.logE(TAG, "Preference not found: " + ex.getMessage());
			finish();
		}

		bindService(new Intent(IBluetoothTimec.class.getName()), mServiceConn, BIND_AUTO_CREATE);
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		execOp(OP_UNREGISTER_CB);
		unbindService(mServiceConn);
	}

/**************************************************************************************************
 * Operation functions
 **************************************************************************************************/
	private void execOp(int op) {
		if (mTimeClient == null) {
			Utils.logE(TAG, "execOp(): mTimeClient is null.");
			finish();
			return;
		}

		try {
			switch (op) {
				case OP_REGISTER_CB:
					mTimeClient.registerCallback(mResultReceiver);
					break;
				case OP_UNREGISTER_CB:
					mTimeClient.unregisterCallback(mResultReceiver);
					break;
				case OP_CONNECT:
					mTimeClient.connect(mDevice);
					break;
				case OP_DISCONNECT:
					mTimeClient.disconnect();
					break;
				case OP_GET_TIME:
					mTimeClient.getServerTime();
					break;
				default:
					Utils.logW(TAG, "Invalid operation: " + op);
					break;
			}
		} catch (Exception ex) {
			Utils.logE(TAG, "Exception occurred in execOp, op: " + op + " ex: " + ex);
		}
	}

/**************************************************************************************************
 * Utility functions
 **************************************************************************************************/
	private boolean applyServerName() {
		Intent intent = getIntent();
		BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
		Preference pref = findPreference(PREF_KEY_SERVER_NAME);

		if (device == null) {
			Utils.logE(TAG, "Null Bluetooth device.");
			return false;
		}

		if (pref == null) {
			Utils.logE(TAG, "Preference not found: " + PREF_KEY_SERVER_NAME);
			return false;
		}

		pref.setTitle(device.getName());
		mDevice = device;
		return true;
	}

	private Preference findAndSetPrefChangeListener(String key) throws Exception {
		Preference pref = findPreference(key);
		if (pref != null) {
			pref.setOnPreferenceChangeListener(this);
		} else {
			throw new Exception(key);
		}
		return pref;
	}

	private Preference findAndSetPrefClickListener(String key) throws Exception{
		Preference pref = findPreference(key);
		if (pref != null) {
			pref.setOnPreferenceClickListener(this);
		} else {
			throw new Exception(key);
		}
		return pref;
	}

	private void showToast(int text_id) {
		try {
			Toast toast = Toast.makeText(this, text_id, Toast.LENGTH_LONG);
			toast.show();
		} catch (Exception ex) {
			Utils.logE(TAG, "Exception occurred in showToast(): " + ex);
		}
	}

	private void showProgressDialog(int title_id, int msg_id, OnCancelListener cancel_listener) {
		String title = getString(title_id);
		String msg = getString(msg_id);
		ProgressDialog dialog = new ProgressDialog(this);

		if (cancel_listener != null) {
			dialog.setCancelable(true);
			dialog.setOnCancelListener(cancel_listener);
		} else {
			// dialog.setCancelable(false);
		}

		dialog.setIndeterminate(true);
		dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		dialog.setTitle(title);
		dialog.setMessage(msg);
		dialog.show();

		mProgressDialog = dialog;
	}

	private void dismissProgressDialog() {
		if (mProgressDialog != null) {
			mProgressDialog.dismiss();
			mProgressDialog = null;
		}
	}

}
