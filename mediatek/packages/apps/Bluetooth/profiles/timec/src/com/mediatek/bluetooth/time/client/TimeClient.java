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

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.IBinder;
import android.os.Message;
import android.os.ResultReceiver;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.util.Log;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.IBluetoothTimec;

public class TimeClient extends PreferenceActivity implements OnClickListener,
		OnPreferenceChangeListener, OnPreferenceClickListener, Callback {

	private static final String TAG = "TimeClient";

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

	// Common UI component and key
	private static final String KEY_CURRENT_SERVER			= "bt_timec_current_server",
								KEY_SETTINGS_GROUP			= "bt_timec_sync_settings_group";

	private Preference mCurrentServerPref;

	private PreferenceGroup mSettingsPref;

	// Mode switch flag: Engineer mode is for function development and PTS.
	private static final boolean ENG_MODE = TimeClientConstants.TIMEC_ENGINEER_MODE;

	// UI components and keys for user mode
	private static final String KEY_SYNC					= "bt_timec_sync",
								KEY_AUTO					= "bt_timec_auto",
								KEY_REPEAT					= "bt_timec_repeat";

	private Preference mSyncPref;

	private CheckBoxPreference mAutoPref;

	private ListPreference mRepeatIntervalPref;

	// UI components and keys for engineer mode
	private static final String KEY_ENG_CONNECT				= "bt_timec_eng_connect",
								KEY_ENG_AUTO				= "bt_timec_eng_auto",
								KEY_ENG_SYNC				= "bt_timec_eng_sync",
								KEY_ENG_UPDATE_SERVER		= "bt_timec_eng_update_server",
								KEY_ENG_DST					= "bt_timec_eng_dst";

	private CheckBoxPreference mEngConnectPref, mEngAutoPref;

	private Preference mEngSyncPref, mEngUpdateServerPref, mEngDstPref;

	// Other stuffs
	private Handler mHandler;

	private BluetoothDevice mServerDevice;

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
		}

		public void onServiceDisconnected(ComponentName name) {
			Utils.logE(TAG, "Service disconnected!!");
			Utils.showToast(TimeClient.this, R.string.bt_timec_service_disconnected);
			finish();
		}
	};

	// An inner class for defnining the behavior of receiving result from TimeClientService
	private class TimecResultReceiver extends ResultReceiver {
		// Constructor
		TimecResultReceiver(Handler handler) {
			super(handler);
		}

		protected void onReceiveResult(int resultCode, Bundle resultData) {
			int rsp = -1;

			Utils.logD(TAG, "onReceiveResult(), result code: " + resultCode);
			if (resultData != null) {
				rsp = resultData.getByte("rspcode", Utils.RSP_UNKNOWN);
			}

			switch (resultCode) {
				case TimeClientMsg.MSG_ID_BT_TIMEC_CONNECT_CNF:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_CONNECT_CNF, rsp: " + rsp);

					if (ENG_MODE) {
						dismissProgressDialog();
						mEngConnectPref.setEnabled(true);

						if (rsp != Utils.RSP_SUCCESS) {
							Utils.showToast(TimeClient.this, R.string.bt_timec_eng_connection_failed);
						} else {
							mEngConnectPref.setChecked(true);
						}

					} else {
						// To be implemented.
					}
					break;

				case TimeClientMsg.MSG_ID_BT_TIMEC_DISCONNECT_IND:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_DISCONNECT_IND, rsp: " + rsp);

					if (ENG_MODE) {
						dismissProgressDialog();
						mEngConnectPref.setEnabled(true);
						mEngConnectPref.setChecked(false);

					} else {
						// To be implemented.
					}
					break;

				case TimeClientMsg.MSG_ID_BT_TIMEC_GET_CTTIME_CNF:
					Utils.logD(TAG, "MSG_ID_BT_TIMEC_GET_CTTIME_CNF, rsp: " + rsp);

					if (ENG_MODE) {
						dismissProgressDialog();

					} else {
						// To be implemented.
					}
					break;

				default:
					// Do nothing.
					break;
			}
		}
	}

	private TimecResultReceiver mResultReceiver;

/**************************************************************************************************
 * Temp fields for only showing user mode UI (To be deleted)
 **************************************************************************************************/
 	private boolean bSyncSuccess = false;

/**************************************************************************************************
 * Interface implementing functions
 **************************************************************************************************/
 	// For implementing Handler.Callback
	public boolean handleMessage(Message msg) {
		Bundle data = msg.getData();
		BluetoothDevice device;

		switch (msg.what) {
			case TimeClientConstants.MSG_SERVER_SELECTED:
				if (data != null) {
					device = data.getParcelable(TimeClientReceiver.DEVICE);
					if (device != null) {
						mServerDevice = device;
						mCurrentServerPref.setTitle(device.getName());
						mCurrentServerPref.setSummary("");
						mSettingsPref.setEnabled(true);
						return true;
					} else {
						Utils.logW(TAG, "Device is null");
					}
				} else {
					Utils.logW(TAG, "No data.");
				}
				break;

			case TimeClientConstants.MSG_SYNC_COMPLETED:
				dismissProgressDialog();
				Utils.showToast(this, R.string.bt_timec_sync_completed);
				return true;

			case TimeClientConstants.MSG_SYNC_FAILED:
				dismissProgressDialog();
				Utils.showToast(this, R.string.bt_timec_sync_failed);
				return true;
		}

		return false;
	}

	// For implementing OnClickListener
	public void onClick(DialogInterface dialog, int which) {
		switch (which) {
			case DialogInterface.BUTTON_POSITIVE:
				showProgressDialog(R.string.bt_timec_synchronizing_title,
						R.string.bt_timec_synchronizing_msg, null);
				bSyncSuccess = !bSyncSuccess;
				mHandler.sendEmptyMessageDelayed(bSyncSuccess ? TimeClientConstants.MSG_SYNC_COMPLETED :
						TimeClientConstants.MSG_SYNC_FAILED, 2000);
				break;

			case DialogInterface.BUTTON_NEGATIVE:
				break;
			default:
				break;
		}
	}

	// For implementing OnPreferenceChangeListener
	public boolean onPreferenceChange(Preference pref, Object newValue) {
		String key = pref.getKey();

		if (KEY_AUTO.equals(key)) {
			return true;

		} else if (KEY_REPEAT.equals(key)) {

		} else if (KEY_ENG_CONNECT.equals(key)) {
			if ((Boolean) newValue) {
				showProgressDialog(R.string.bt_timec_eng_connecting_title,
						R.string.bt_timec_eng_connecting_msg, null);
				execOp(OP_CONNECT);

			} else {
				showProgressDialog(R.string.bt_timec_eng_disconnecting_title,
						R.string.bt_timec_eng_disconnecting_msg, null);
				execOp(OP_DISCONNECT);
			}
			mEngConnectPref.setEnabled(false);

		} else if (KEY_ENG_AUTO.equals(key)) {
			// Implementing...
		} else {
			Utils.logW(TAG, "Invalid pref key: " + key);
		}

		return false;
	}

	// For implementing OnPreferenceClickListener
	public boolean onPreferenceClick(Preference pref) {

		String key = pref.getKey();
		if (KEY_CURRENT_SERVER.equals(key)) {
			Intent intent = new Intent(BluetoothDevicePicker.ACTION_LAUNCH);
            intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            intent.putExtra(BluetoothDevicePicker.EXTRA_NEED_AUTH, false);
            intent.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_PACKAGE, Utils.APP_PACKAGE);
            intent.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_CLASS, TimeClientReceiver.class.getName());
            startActivity(intent);
			return true;

		} else if (KEY_SYNC.equals(key)) {
            Builder builder = new Builder(this)
					.setTitle(R.string.bt_timec_sync_confirm_title)
					.setMessage(R.string.bt_timec_sync_confirm_msg)
					.setPositiveButton(android.R.string.ok, this)
					.setNegativeButton(android.R.string.cancel, this);
			AlertDialog dialog = builder.create();
			dialog.show();
			return true;

		} else if (KEY_ENG_SYNC.equals(key)) {
			showProgressDialog(R.string.bt_timec_synchronizing_title,
					R.string.bt_timec_synchronizing_msg, null);
			execOp(OP_GET_TIME);
			return true;

		} else if (KEY_ENG_UPDATE_SERVER.equals(key)) {
			// Implementing...
		} else if (KEY_ENG_DST.equals(key)) {
			// Implementing...
		} else {
			Utils.logW(TAG, "Invalid pref key: " + key);
		}

		return false;
	}

/**************************************************************************************************
 * Activity life-cycle functions
 **************************************************************************************************/
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setTitle(R.string.bt_timec_main_title);
		initPrefs();

		mHandler = new Handler(this);
		mResultReceiver = new TimecResultReceiver(mHandler);
		TimeClientReceiver.setHandler(mHandler);

		bindService(new Intent(IBluetoothTimec.class.getName()), mServiceConn, BIND_AUTO_CREATE);
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		TimeClientReceiver.clearHandler();
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
					mTimeClient.connect(mServerDevice);
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
 * UI relative functions
 **************************************************************************************************/
	private Preference initPref(String key) {
		Preference pref = findPreference(key);
		if (pref != null) {
			if (pref instanceof CheckBoxPreference) {
				pref.setOnPreferenceChangeListener(this);
			} else if (pref instanceof ListPreference) {
				ListPreference list_pref = (ListPreference) pref;
				list_pref.setOnPreferenceChangeListener(this);
				list_pref.setSummary(list_pref.getEntry());
			} else {
				pref.setOnPreferenceClickListener(this);
			}
		} else {
			Utils.logE(TAG, "Invalid preference key " + key);
			finish();
		}
		return pref;
	}

	private void initPrefs() {
		addPreferencesFromResource(R.xml.bt_timec_main);
		mCurrentServerPref = initPref(KEY_CURRENT_SERVER);

		if (ENG_MODE) {
			addPreferencesFromResource(R.xml.bt_timec_settings_eng);
			mEngConnectPref = (CheckBoxPreference) initPref(KEY_ENG_CONNECT);
			mEngAutoPref = (CheckBoxPreference) initPref(KEY_ENG_AUTO);
			mEngSyncPref = initPref(KEY_ENG_SYNC);
			mEngUpdateServerPref = initPref(KEY_ENG_UPDATE_SERVER);
			mEngDstPref = initPref(KEY_ENG_DST);

		} else {
			addPreferencesFromResource(R.xml.bt_timec_settings_user);
			mSyncPref = initPref(KEY_SYNC);
			mAutoPref = (CheckBoxPreference) initPref(KEY_AUTO);
			mRepeatIntervalPref = (ListPreference) initPref(KEY_REPEAT);
		}

		mSettingsPref = (PreferenceGroup) findPreference(KEY_SETTINGS_GROUP);
		mSettingsPref.setEnabled(false);
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

		dismissProgressDialog();
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
