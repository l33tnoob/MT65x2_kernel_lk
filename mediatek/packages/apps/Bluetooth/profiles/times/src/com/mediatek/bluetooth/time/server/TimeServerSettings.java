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

package com.mediatek.bluetooth.time.server;

import android.app.Dialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.widget.Toast;

import com.mediatek.activity.ServiceActivityHelper;
import com.mediatek.activity.AssembledPreferenceActivity.AssemblyPreference;
import com.mediatek.activity.ServiceActivityHelper.ServiceActivity;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.IBluetoothTimes;

public class TimeServerSettings implements ServiceActivity<IBluetoothTimes>, AssemblyPreference,
		Preference.OnPreferenceChangeListener {
	private static final String TAG = "TimeServerSettings";

	private static final String KEY_SERVER_ENABLE	= "bt_times_settings_enable";

    private PreferenceActivity parentActivity;
    private ServiceActivityHelper<IBluetoothTimes> saHelper;
    private CheckBoxPreference enablePref;

	private ResultReceiver callback = new ResultReceiver(new Handler()) {
		protected void onReceiveResult(int resultCode, Bundle resultData) {
			int rsp;

			switch (resultCode) {
				case TimeServerMsg.MSG_ID_BT_TIMES_REGISTER_CNF:
					rsp = resultData.getByte("rspcode", ResponseCode.UNKNOWN);
					TimeServerLog.d(TAG, "BT_TIMES_REGISTER_CNF, rsp: " + rsp);

					if (rsp != ResponseCode.SUCCESS) {
						Toast.makeText(parentActivity, "Failed to register Time server", Toast.LENGTH_LONG).show();
					}

					saHelper.releaseServiceLock();
					saHelper.refreshUi(parentActivity);
					break;

				case TimeServerMsg.MSG_ID_BT_TIMES_DEREGISTER_CNF:
					TimeServerLog.d(TAG, "BT_TIMES_DEREGISTER_CNF");
					saHelper.releaseServiceLock();
					saHelper.refreshUi(parentActivity);
					break;

				default:
					TimeServerLog.w(TAG, "Invalid resultCode: " + resultCode);
			}
		}
	};

	public TimeServerSettings() {
		saHelper = new ServiceActivityHelper<IBluetoothTimes>(this);
	}

	// For implementing AssemblyPreference
	public int getPreferenceResourceId() {
		return R.xml.bt_times_settings;
	}

	// For implementing AssemblyPreference
	public void onCreate(PreferenceActivity activity) {
		parentActivity = activity;
		enablePref = (CheckBoxPreference) parentActivity.findPreference(KEY_SERVER_ENABLE);

		if (enablePref != null) {
			enablePref.setOnPreferenceChangeListener(this);
			saHelper.bindService(parentActivity);

		} else {
			// TODO: print some error message.
		}
	}

	// For implementing AssemblyPreference
	public Dialog onCreateDialog(int id) {
		return null;
	}

	// For implementing AssemblyPreference
	public void onDestroy() {

		try {
			saHelper.service.unregisterCallback(callback);
		} catch (Exception ex) {
			// Do nothing for now.
		}

		saHelper.unbindService(parentActivity);
		parentActivity = null;
		enablePref = null;
	}

	// For implementing AssemblyPreference
	public void onResume() {
		saHelper.refreshUi(parentActivity);
	}

	// For implementing AssemblyPreference
	public void onRestoreInstanceState(Bundle savedInstanceState) {
	}

	// For implementing AssemblyPreference
	public void onSaveInstanceState(Bundle outState) {
	}

	// For implementing ServiceActivity<IBluetoothTimes>
	public String getServiceAction() {
		return IBluetoothTimes.class.getName();
	}

	// For implementing ServiceActivity<IBluetoothTimes>
	public IBluetoothTimes asInterface(IBinder service) {
		return IBluetoothTimes.Stub.asInterface(service);
	}

	// For implementing ServiceActivity<IBluetoothTimes>
	public void onServiceConnected() {
		try {
			saHelper.service.registerCallback(callback);
		} catch (Exception ex) {
			// A RemoteException was caught, do nothing for now.
		}

		saHelper.releaseServiceLock();
		saHelper.refreshUi(parentActivity);
	}

	// For implementing ServiceActivity<IBluetoothTimes>
	public void onServiceDisconnected() {
		saHelper.acquireServiceLock();
	}

	// For implementing ServiceActivity<IBluetoothTimes>
	public void refreshActivityUi() {
		int state = -1;

		if (saHelper.service != null) {
			try {
				state = saHelper.service.getServiceState();
			} catch (Exception ex) {
				TimeServerLog.e(TAG, "Failed to get service state: " + ex);
			}
		}

		if (enablePref == null) {
			TimeServerLog.e(TAG, "Enable preference is null.");
			return;
		}

		TimeServerLog.d(TAG, "service state: " + state);

		switch (state) {
			case TimeServerConstants.TIMES_STATE_NEW:
				enablePref.setChecked(false);
				enablePref.setEnabled(true);
				break;
			case TimeServerConstants.TIMES_STATE_REGISTERING:
			case TimeServerConstants.TIMES_STATE_UNREGISTERING:
				enablePref.setEnabled(false);
				break;
			case TimeServerConstants.TIMES_STATE_CONNECTABLE:
				enablePref.setChecked(true);
				enablePref.setEnabled(true);
				break;

			default:
				enablePref.setEnabled(false);
				// Print some error message.
				TimeServerLog.e(TAG, "Invalid state: " + state);
		}
	}

	// For implementing OnPreferenceChangeListener
	public boolean onPreferenceChange(Preference pref, Object newValue) {
		String key = pref.getKey();

		if (KEY_SERVER_ENABLE.equals(key)) {
			enablePref.setEnabled(false);
			saHelper.acquireServiceLock();

			try {
				if ((Boolean) newValue) {
					saHelper.service.enableService();
				} else {
					saHelper.service.disableService();
				}
			} catch (Exception ex) {
				TimeServerLog.e(TAG, "Exception: " + ex);
			}

		} else {
			// Print warnning
		}

		return false;
	}
}
