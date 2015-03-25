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

package com.mediatek.bluetooth.ftp;

import java.security.acl.Group;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.AlertDialog.Builder;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.PreferenceCategory;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.activity.AssembledPreferenceActivity.AssemblyPreference;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.SystemUtils;
import com.mediatek.common.featureoption.FeatureOption;

import android.bluetooth.BluetoothFtp;
import android.bluetooth.IBluetoothFtpServer;
import android.bluetooth.IBluetoothFtpServerCallback;

	
public class BluetoothFtpServerAdvSettings implements AssemblyPreference, Preference.OnPreferenceChangeListener {

	private static final String TAG = "FtpsAdvancedSettings";

	private static final String KEY_FTP_SERVER_ENABLE = "ftp_server_enable",
								KEY_FTP_SERVER_PERMISSION = "ftp_server_permission",
								KEY_SHOW_DIALOG = "show_alert_dialog",
								KEY_PROFILE = "profile_key_for_dialog";

	private static final int PROFILE_BASE = 0,
							 PROFILE_FTP = PROFILE_BASE + 1;

	private static final boolean MTK_EMMC_SUPPORT = FeatureOption.MTK_EMMC_SUPPORT;

	private AlertDialog mDialog;

	private int mProfileKey = PROFILE_BASE;

	private CheckBoxPreference mFtpServerEnable;
	private ListPreference mFtpServerPermission;

	/* FTP Server binder interface to BluetoothFtpService */
	private IBluetoothFtpServer mFtpServer = null;

	/* FTP Server binder callback interface to BluetoothFtpService */
	private IBluetoothFtpServerCallback mFtpServerCallback = 
			new IBluetoothFtpServerCallback.Stub() {

		public void postEvent(int event, Bundle data) {
			// Log.d(TAG, "[BT][FTP] received event: " + event);
			Message msg = Message.obtain();
			msg.what = event;
			if (data != null) {
				msg.setData(new Bundle(data));
			}
			mHandler.sendMessage(msg);
		}
	};

	private ServiceConnection mFtpServerConn = new ServiceConnection() {

		public void onServiceConnected(ComponentName className, IBinder service) {
			mFtpServer = IBluetoothFtpServer.Stub.asInterface(service);

			if (mFtpServer != null) {
				try {
					mFtpServer.registerCallback(mFtpServerCallback);

				// Check FTP server status
				if (mFtpServer.getStatus() != BluetoothFtp.BT_FTPS_STATE_IDLE) {
					mFtpServerEnable.setChecked(true);
				}

				int perm = mFtpServer.getPermission();
				if (perm == BluetoothFtp.BT_FTPS_FULLCTRL) {
					mFtpServerPermission.setValueIndex(1);
					mFtpServerPermission.setSummary(R.string.bluetooth_ftp_server_permission_summary_fullctrl);
				} else {
					// Read-only or other cases (ex. default)
					mFtpServerPermission.setValueIndex(0);
					mFtpServerPermission.setSummary(R.string.bluetooth_ftp_server_permission_summary_readonly);
				}
				Log.d(TAG, "[BT][FTP] Init-value of FTP server permission: " + perm);

				} catch (RemoteException e) {
					// do nothing for now.
				}
			}
		}

		public void onServiceDisconnected(ComponentName className) {
			Log.w(TAG, "[BT][FTP] Unexpectedly disconnected with BluetoothFtpService");
			mFtpServerEnable.setEnabled(false);
			Toast.makeText( BluetoothFtpServerAdvSettings.this.parentActivity,
					"FTP Service disconnected unexpectedly.",
					Toast.LENGTH_SHORT).show();
		}
	};

	private final IntentFilter mFilter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);

	private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

			switch (state) {
				case BluetoothAdapter.STATE_ON:
					if (mFtpServer != null) {
						mFtpServerEnable.setEnabled(true);
					}
					break;

				case BluetoothAdapter.STATE_TURNING_OFF:
					mFtpServerEnable.setEnabled(false);
					break;

				default:
					// Do nothing.
					break;
			}
		}
	};

	private Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {
			// Log.d(TAG, "[BT][FTP] handling event: " + msg.what);
			switch(msg.what) {
				case BluetoothFtp.BT_FTPSUI_READY:
					// Change state of check-box to checked
					// Log.d(TAG, "[BT][FTP] Server is turned on.");
					mFtpServerEnable.setChecked(true);
					mFtpServerEnable.setEnabled(true);
					break;
				case BluetoothFtp.BT_FTPSUI_DISABLED:
					// Change state of check-box to unchecked
					// Log.d(TAG, "[BT][FTP] Server is turned off.");
					mFtpServerEnable.setChecked(false);
					mFtpServerEnable.setEnabled(true);
					break;
				default:
					break;
			}
		}
	};

	private DialogInterface.OnClickListener mSDDialogListener = new DialogInterface.OnClickListener() {

		public void onClick(DialogInterface dialog, int which) {
			dialog.dismiss();
		}
	};

	private void showWarnningDialog(int profile) {
		Builder builder = new Builder(this.parentActivity);

		switch (profile) {
			case PROFILE_FTP:
				builder.setIcon(android.R.drawable.ic_dialog_alert)
					   .setTitle(R.string.bluetooth_ftp_server_sd_dialog_title)
					   .setMessage(R.string.bluetooth_ftp_server_sd_dialog_msg)
					   .setPositiveButton(R.string.bluetooth_ftp_server_sd_dialog_ok, mSDDialogListener);
				break;

			default:
				break;
		}

		mProfileKey = profile;
		mDialog = builder.create();
		mDialog.show();
	}

	private PreferenceActivity parentActivity;
	
	public int getPreferenceResourceId(){

		return com.mediatek.bluetooth.R.xml.bt_ftps_adv_settings;
	}

	public void onCreate( PreferenceActivity parentActivity ){

		this.parentActivity = parentActivity;
		mFtpServerEnable = (CheckBoxPreference)this.parentActivity.findPreference(KEY_FTP_SERVER_ENABLE);
		mFtpServerPermission = (ListPreference)this.parentActivity.findPreference(KEY_FTP_SERVER_PERMISSION);
		if (mFtpServerEnable == null || mFtpServerPermission == null) {
			Log.e(TAG, "[BT][FTP] Can't find FTP preferences.");
			this.parentActivity.finish();
			return;
		} else {
			mFtpServerEnable.setOnPreferenceChangeListener(this);
			mFtpServerPermission.setOnPreferenceChangeListener(this);
		}

		// Bind to BluetoothFtpService
		// Log.i(TAG, "[BT][FTP] bindService...");
		boolean bindFtp = this.parentActivity.bindService(
				new Intent(IBluetoothFtpServer.class.getName()), mFtpServerConn, 0);
		if (!bindFtp) {
			Log.w(TAG, "[BT][FTP] Failed to bind service.");
		} else {
			this.parentActivity.registerReceiver(mReceiver, mFilter);
		}
	}

	public Dialog onCreateDialog(int id){

		return null;
	}

	public void onResume(){

	}

	public void onRestoreInstanceState( Bundle savedInstanceState ){

		mProfileKey = savedInstanceState.getInt(KEY_PROFILE, PROFILE_BASE);
		if (savedInstanceState.getBoolean(KEY_SHOW_DIALOG, false) &&
			mProfileKey != PROFILE_BASE) {
			showWarnningDialog(mProfileKey);
		}
	}

	public void onSaveInstanceState( Bundle outState ){

		if (mDialog != null) {
			outState.putBoolean(KEY_SHOW_DIALOG, mDialog.isShowing());
			outState.putInt(KEY_PROFILE, mProfileKey);
		}
	}

	public void onDestroy(){

		try {
			this.parentActivity.unregisterReceiver(mReceiver);
		} catch (Exception e) {
			// This try-catch is for the case that unregisterReceiver() is called when 
			// the receiver is not yet registered. 
			Log.w(TAG, "[BT][FTP] Receiver not registered.");
		}

		if (mDialog != null) {
			mDialog.dismiss();
		}

		// Unregister the callbacks
		try {
			if (mFtpServer != null) {
				mFtpServer.unregisterCallback(mFtpServerCallback);
			} else {
				Log.e(TAG, "[BT][FTP] Unregister FTP server callback failed: null mFtpServer.");
			}
		} catch (RemoteException e) {
			Log.e(TAG, "[BT][FTP] Unregister FTP server callback failed: RemoteException.");
		}

		try {
			this.parentActivity.unbindService(mFtpServerConn);
		} catch (Exception ex) {
			Log.w(TAG, "[BT][FTP] Exception triggered when unbinding service: " + ex);
		}
	}

	/* This function is called when the new value hasn't been applied. */
	public boolean onPreferenceChange(Preference preference, Object objValue) {
		final String key = preference.getKey();
		// Log.d(TAG, "[FTP] onPreferenceChange(), key: " + key);

		if (KEY_FTP_SERVER_ENABLE.equals(key)) {
			// Check FTP server interface
			if (mFtpServer == null) {
				Log.e(TAG, "[BT][FTP] mFtpServer is empty.");
				return false;
			}

			if (MTK_EMMC_SUPPORT) {
				// If EMMC is supported, don't check SD card.
			} else {
				String sdPath = Environment.getExternalStorageDirectory().getAbsolutePath();
				if (!SystemUtils.isExternalStorageMounted(this.parentActivity, sdPath)) {
					Log.w(TAG, "[BT][FTP] No SD card for FTP server");
					showWarnningDialog(PROFILE_FTP);
					return false;
				}
			}

			try {
				if (!mFtpServerEnable.isChecked()) {
					// Enable FTP server via binder function call
					mFtpServer.enable();
					// Disable the checkbox preference
					// Log.d(TAG, "[FTP] Disable the UI, waiting for enable result.");
					mFtpServerEnable.setEnabled(false);
				} else {
					// Disable FTP erver via binder function call
					mFtpServer.disable();
					// Log.d(TAG, "[FTP] Disable the UI, waiting for disable result.");
					mFtpServerEnable.setEnabled(false);
				}
			} catch (RemoteException e) {
				Log.e(TAG, "[BT][FTP] Enable/disable FTP server failed.");
			}
			// Do not update status of the checkbox.
			return false;

		} else if (KEY_FTP_SERVER_PERMISSION.equals(key)) {
			try {
				int value = Integer.parseInt((String) objValue);
				// Log.d(TAG, "FTP Server Permission Changed: " + value);
				if (mFtpServer != null) {
					mFtpServer.setPermission(value);
				}

				mFtpServerPermission.setSummary(
					value == BluetoothFtp.BT_FTPS_READONLY ?
					R.string.bluetooth_ftp_server_permission_summary_readonly :
					R.string.bluetooth_ftp_server_permission_summary_fullctrl
				);

				Log.d(TAG, "[BT][FTP] Permission: " + mFtpServerPermission.getSummary());

			} catch (NumberFormatException ex) { 
				Log.e(TAG, "[BT][FTP] Could not parse ftp server permission value.");
			} catch (RemoteException ex) {
				Log.e(TAG, "[BT][FTP] RemoteException occurred when setPermission().");
			}
		}
		// return ture to update state, false for not update.
		return true;
	}
}
