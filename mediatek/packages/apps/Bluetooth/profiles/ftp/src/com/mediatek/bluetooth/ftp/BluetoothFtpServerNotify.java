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

package com.mediatek.bluetooth.ftp;

import com.mediatek.bluetooth.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

/* This activity is launched for FTP-server-authorizing and FTP-server-connected notification. */
public class BluetoothFtpServerNotify extends Activity
		implements DialogInterface.OnClickListener {

	private static final String TAG = "BluetoothFtpServerNotify";

	private boolean isDone = false;

	private AlertDialog mDialog;

	private String mDeviceName;

	private int mNotifyType;

	private IBluetoothFtpServerNotify mServerNotify = null;

	private ServiceConnection mFtpServerNotifyConn = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			mServerNotify = IBluetoothFtpServerNotify.Stub.asInterface(service);
		}

		public void onServiceDisconnected(ComponentName className) {
		}
	};

	private IntentFilter mFilter = new IntentFilter(BluetoothFtpService.SERVER_DISCONNECTED);

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			// Log.d(TAG, "onReceive()");
			forceExit();
		}
	};

	private OnCancelListener mCancelListener = new OnCancelListener() {
		public void onCancel(DialogInterface dialog) {
			finish();
		}
	};

	private AlertDialog buildDialog(int type, String deviceName) {
		Builder builder = new Builder(this);
		String msg = null;
		int icon_id = 0, title_id = 0, positive_id = 0, negative_id = 0;

		if (type == BluetoothFtpService.FTPS_AUTHORIZE_NOTIFY) {
			icon_id = android.R.drawable.ic_dialog_info;
			title_id = R.string.bluetooth_ftp_server_authorize_title;
			msg = String.format(
					getString(R.string.bluetooth_ftp_server_authorize_message),
					deviceName);
			positive_id = R.string.bluetooth_ftp_server_authorize_allow;
			negative_id = R.string.bluetooth_ftp_server_authorize_decline;

		} else if (type == BluetoothFtpService.FTPS_CONNECTED_NOTIFY) {
			icon_id = android.R.drawable.ic_dialog_info;
			title_id = R.string.bluetooth_ftp_server_disconnect_title;
			msg = String.format(
					getString(R.string.bluetooth_ftp_server_disconnect_message),
					deviceName);
			positive_id = R.string.bluetooth_ftp_yes;
			negative_id = R.string.bluetooth_ftp_no;

		} else {
			Log.e(TAG, "Invalid notification type: " + type);
			forceExit();
		}

		builder.setIcon(icon_id)
			   .setTitle(title_id)
			   .setMessage(msg)
			   .setPositiveButton(positive_id, this)
			   .setNegativeButton(negative_id, this)
			   .setOnCancelListener(mCancelListener);

		return builder.create();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Log.d(TAG, "onCreate()");
		Intent intent = getIntent();
		mNotifyType = intent.getIntExtra(BluetoothFtpService.NOTIFY_TYPE, -1);
		mDeviceName = intent.getStringExtra(BluetoothFtpService.DEVICE_NAME);

		if (mNotifyType == -1) {
			Log.e(TAG, "Notification type is not assigned.");
			forceExit();
		}

		mDialog = buildDialog(mNotifyType, mDeviceName);

		bindService(new Intent(IBluetoothFtpServerNotify.class.getName()), 
		mFtpServerNotifyConn, Context.BIND_AUTO_CREATE);
		registerReceiver(mReceiver, mFilter);
	}

	@Override
	protected void onResume() {
		super.onResume();
		Log.d(TAG, "onResume()");
		mDialog.show();
	}

	@Override
	protected void onPause() {
		super.onPause();
		Log.d(TAG, "onPause()");
		mDialog.dismiss();
	}

	@Override
	protected void onStop() {
		super.onStop();
		Log.d(TAG, "onStop()");
		if (!isDone) {
			updateNotification();
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		Log.d(TAG, "onDestroy()");
		unregisterReceiver(mReceiver);
		unbindService(mFtpServerNotifyConn);
	}

	private void sendAuthResult(boolean res) {
		// Log.d(TAG, "Authorize: " + (res ? "Allow" : "Reject"));
		try {
			mServerNotify.authResult(res);
		} catch (Exception ex) {
			Log.e(TAG, "authResult() failed.");
		}
		forceExit();
	}

	private void sendDisconnect() {
		// Log.d(TAG, "Disconnect from notification");
		try {
			mServerNotify.disconnect();
		} catch (Exception ex) {
			Log.e(TAG, "disconnect() failed.");
		}
		forceExit();
	}

	private void updateNotification() {
		try {
			mServerNotify.updateNotify(mNotifyType);
		} catch (Exception ex) {
			Log.e(TAG, "updateNotification() failed.");
			finish();
		}
	}

	private void forceExit() {
		isDone = true;
		finish();
	}

	/* Functions for DialogInterface.OnClickListener */
	public void onClick(DialogInterface dialog, int which) {
		switch (which) {
			case DialogInterface.BUTTON_POSITIVE:
				Log.d(TAG, "Positive button pressed.");
				if (mNotifyType == BluetoothFtpService.FTPS_AUTHORIZE_NOTIFY) {
					sendAuthResult(true);
				} else {
					sendDisconnect();
				}
				break;

			case DialogInterface.BUTTON_NEGATIVE:
				Log.d(TAG, "Negative button pressed.");
				if (mNotifyType == BluetoothFtpService.FTPS_AUTHORIZE_NOTIFY) {
					sendAuthResult(false);
				} else {
					finish();
				}
				break;

			default:
				break;
		}
	}
}
