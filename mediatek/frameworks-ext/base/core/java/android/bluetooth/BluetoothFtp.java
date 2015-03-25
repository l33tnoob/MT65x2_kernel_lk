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

package android.bluetooth;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.RemoteException;
import android.os.IBinder;
import android.util.Log;

import java.util.Set;
import java.util.HashSet;

/** {@hide}
 */
public class BluetoothFtp {

	private final static String TAG = "BluetoothFtp";

	/* Before profile manager is ready, use these strings for the intent. */
	public final static String ACTION_STATE_CHANGED			= "android.bluetooth.ftp.action.STATE_CHANGED",
							   ACTION_BIND_SERVER			= "android.bluetooth.ftp.action.BIND_SERVER",
							   ACTION_BIND_CLIENT			= "android.bluetooth.ftp.action.BIND_CLIENT",
							   EXTRA_STATE					= "android.bluetooth.ftp.extra.STATE";

	/* Server UI Events */
	public final static int BT_FTPSUI_EVENT_NONE			= 0;
	public final static int BT_FTPSUI_SHUTDOWNING			= 1;  /* deinit Param: none */
	public final static int BT_FTPSUI_READY					= 2;  /* server register ok Param: none */
	public final static int BT_FTPSUI_AUTHORIZING			= 3;  /* server receive a incoming */
	public final static int BT_FTPSUI_AUTHEN_WAIT			= 4;  /* Param: none. server receive a client-challenge */
	public final static int BT_FTPSUI_CONNECTING			= 5;
	public final static int BT_FTPSUI_CONNECTED				= 6;  /* Param: rspcode */
	public final static int BT_FTPSUI_SET_FOLDER_START		= 7;  /* Param: none */
	public final static int BT_FTPSUI_SET_FOLDERED			= 8;  /* Param: rspcode */
	public final static int BT_FTPSUI_BROWSE_START			= 9;  /* Param: none */
	public final static int BT_FTPSUI_BROWSING				= 10;
	public final static int BT_FTPSUI_BROWSED				= 11; /* Param: rspcode */
	public final static int BT_FTPSUI_PUSH_FILE_START		= 12; /* Param: none */
	public final static int BT_FTPSUI_PUSHING				= 13; /* Param: Percentage */
	public final static int BT_FTPSUI_PUSHED				= 14; /* Param: rspcode */
	public final static int BT_FTPSUI_PULL_FILE_START		= 15; /* Param: none */
	public final static int BT_FTPSUI_PULLING				= 16; /* Param: Percentage */
	public final static int BT_FTPSUI_PULLED				= 17; /* Param: rspcode */
	public final static int BT_FTPSUI_FILE_DELETE			= 18;
	public final static int BT_FTPSUI_FOLDER_DELETE			= 19;
	public final static int BT_FTPSUI_FILE_CREATE			= 20;
	public final static int BT_FTPSUI_FOLDER_CREAT_START	= 21;
	public final static int BT_FTPSUI_ABORTED				= 22;
	public final static int BT_FTPSUI_DISCONNECTED			= 23;
	public final static int BT_FTPSUI_ERROR					= 24;
	public final static int BT_FTPSUI_DISABLED				= 25; /* For Android */

	/* Server States */
	public final static int BT_FTPS_STATE_IDLE				= 100;
	public final static int BT_FTPS_STATE_ACTIVE			= 101;
	public final static int BT_FTPS_STATE_AUTHORIZING		= 102;
	public final static int BT_FTPS_STATE_CONNECTED			= 103;
	public final static int BT_FTPS_STATE_SENDING			= 104;
	public final static int BT_FTPS_STATE_RECEIVING			= 105;
	public final static int BT_FTPS_STATE_BROWSING			= 106;
	public final static int BT_FTPS_STATE_DISCONNECTED		= 107;

	/* Server Permission */
	public final static int BT_FTPS_READONLY				= 1000;
	public final static int BT_FTPS_FULLCTRL				= 1001;

	/* Client UI Events */
	public final static int BT_FTPCUI_EVENT_NONE			= 30;
	public final static int BT_FTPCUI_DEACTVE				= 31; /* Param: rsp_code */
	public final static int BT_FTPCUI_ACTIVE				= 32; /* Param: none */
	public final static int BT_FTPCUI_CONNECTING			= 33; /* Param: none */
	public final static int BT_FTPCUI_AUTHEN_WAIT			= 34; /* Param: none */
	public final static int BT_FTPCUI_CONNECTED				= 35; /* Param: rsp_code */
	public final static int BT_FTPCUI_DISCONNECTING			= 36; /* Param: rsp_code */
	public final static int BT_FTPCUI_PUSHING				= 37; /* Param: TxBytes */
	public final static int BT_FTPCUI_PUSHED				= 38; /* Param: rsp_code */
	public final static int BT_FTPCUI_BROWSING				= 39; /* Param: U32 RxBytes */
	public final static int BT_FTPCUI_BROWSED				= 40; /* Param: rsp_code */
	public final static int BT_FTPCUI_PULLING				= 41; /* Param: U32 RxBytes */
	public final static int BT_FTPCUI_PULLED				= 42; /* Param: rsp_code */
	public final static int BT_FTPCUI_SETPATHING			= 43; /* Param: none */
	public final static int BT_FTPCUI_SETPATHED				= 44; /* Param: rsp_code */
	public final static int BT_FTPCUI_ABORTING				= 45; /* Param: rsp_code */
	public final static int BT_FTPCUI_FILE_DELETED			= 46; /* Param: rsp_code */
	public final static int BT_FTPCUI_FOLDER_DELETED		= 47; /* Param: rsp_code */
	public final static int BT_FTPCUI_FOLDER_CREATED		= 48; /* Param: rsp_code */
	public final static int BT_FTPCUI_DISCONNECTED			= 49; /* Param: rsp_code */
	public final static int BT_FTPCUI_ERROR					= 50; /* unexpected error. Param: exernal platform error code */
	public final static int BT_FTPCUI_MAX					= 51;

	/* Client States */
	public final static int BT_FTPC_STATE_IDLE				= 200;
	public final static int BT_FTPC_STATE_ACTIVE			= 201;
	public final static int BT_FTPC_STATE_AUTHORIZING		= 202;
	public final static int BT_FTPC_STATE_CONNECTED			= 203;
	public final static int BT_FTPC_STATE_SENDING			= 204;
	public final static int BT_FTPC_STATE_RECEIVING			= 205;
	public final static int BT_FTPC_STATE_TOBROWSE			= 206;
	public final static int BT_FTPC_STATE_BROWSING			= 207;
	public final static int BT_FTPC_STATE_BROWSED			= 208;
	public final static int BT_FTPC_STATE_ABORTING			= 209;
	public final static int BT_FTPC_STATE_DISCONNECTED		= 210;

	/* Response codes */
	public final static int BT_FTP_RSP_SUCCESS				= 0;

	/* Reasons for disconnected client */
	public final static int	BT_FTPC_UNEXPECTED				= 0,
							BT_FTPC_CONNECTION_FAILED		= 1,
							BT_FTPC_USER_CANCELED			= 2;

	private BluetoothFtp() {}

	private static abstract class Remote implements BluetoothProfileManager.BluetoothProfileBehavior {
		private String TAG = "";

		protected IBluetoothFtpCtrl mService;
		protected Context mContext;

		protected ServiceConnection mConnection = new ServiceConnection() {
			public void onServiceConnected(ComponentName className, IBinder service) {
				mService = IBluetoothFtpCtrl.Stub.asInterface(service);
			}

			public void onServiceDisconnected(ComponentName className) {
				mService = null;
			}
		};

		protected Remote(Context context, String tag) {
			mContext = context;
			TAG = tag;
		}

		protected abstract int convertState(int ftpState);

		public abstract boolean connect(BluetoothDevice device);

		public Set<BluetoothDevice> getConnectedDevices() {
			Set<BluetoothDevice> devices = null;
			if (mService != null) {
				try {
					BluetoothDevice device = mService.getCurrentDevice();
					if (device != null) {
						devices = new HashSet<BluetoothDevice>();
						devices.add(device);
					}
				} catch (RemoteException re) {
					printErr("Exception in getConnectedDevices(): " + re);
				}

			} else {
				printErr("mService is null");
			}

			return devices;
		}

		public boolean disconnect(BluetoothDevice device) {
			if (mService != null) {
				try {
					mService.disconnect(device);
				} catch (RemoteException e) {
					printErr("Exception: " + e);
					return false;
				}

			} else {
				printErr("mService is null");
				return false;
			}
			return true;
		}

		public int getState(BluetoothDevice device) {
			int ret = BluetoothProfileManager.STATE_DISCONNECTED;

			if (mService != null) {
				try {
					BluetoothDevice current = mService.getCurrentDevice();
					ret = (current != null) && current.equals(device) ?
							convertState(mService.getState()) :
							BluetoothProfileManager.STATE_DISCONNECTED;
				} catch (RemoteException e) {
					printErr("Exception when get state from FTP service: " + e);
				}
			}
			return ret;
		}

		public void close() {
			disconnectService();
		}

		protected void disconnectService() {
			try {
				if (mConnection != null) {
					mContext.unbindService(mConnection);
					mConnection = null;
				}
			} catch (Exception e) {
				printErr("Exception occurred in unbindService(): " + e);

			}
		}

		protected void printErr(String msg) {
			Log.e(TAG, "[BT][FTP] " + msg);
		}
	}

	/* Remote control of FTP server. */
	public static class Server extends Remote {

		public Server(Context context) {
			super(context, "BluetoothFtp.Server");
			if (!context.bindService(
					new Intent(ACTION_BIND_SERVER), mConnection, Context.BIND_AUTO_CREATE)) {
				printErr("Could not bind to Bluetooth FTP Service");
			}
		}

		public boolean connect(BluetoothDevice device) {
			return false;
		}

		protected int convertState(int serverState) {
			switch (serverState) {
				case BT_FTPS_STATE_CONNECTED:
					return BluetoothProfileManager.STATE_CONNECTED;

				case BT_FTPS_STATE_AUTHORIZING:
					return BluetoothProfileManager.STATE_CONNECTING;

				default:
					return BluetoothProfileManager.STATE_DISCONNECTED;
			}
		}

		protected void finalize() {
			disconnectService();
			try {
				super.finalize();
			} catch (Throwable t) {
				printErr("Throwable caught in finalize(): " + t);
			}
		}
	}

	/* Remote control of FTP client. */
	public static class Client extends Remote {

		public Client(Context context) {
			super(context, "BluetoothFtp.Client");
			if (!context.bindService(
					new Intent(ACTION_BIND_CLIENT), mConnection, Context.BIND_AUTO_CREATE)) {
				printErr("Could not bind to Bluetooth FTP Service");
			}
		}

		public boolean connect(BluetoothDevice device) {
			if (mService != null) {
				try {
					mService.connect(device);
				} catch (RemoteException e) {
					printErr("Exception: " + e);
					return false;
				}

			} else {
				printErr("mService is null");
				return false;
			}
			return true;
		}

		protected int convertState(int ftpState) {
			switch (ftpState) {
				case BT_FTPC_STATE_IDLE:
				case BT_FTPC_STATE_ACTIVE:
				case BT_FTPC_STATE_DISCONNECTED:
					return BluetoothProfileManager.STATE_DISCONNECTED;

				case BT_FTPC_STATE_CONNECTED:
				case BT_FTPC_STATE_SENDING:
				case BT_FTPC_STATE_RECEIVING:
				case BT_FTPC_STATE_BROWSING:
				case BT_FTPC_STATE_BROWSED:
				case BT_FTPC_STATE_ABORTING:
					return BluetoothProfileManager.STATE_CONNECTED;

				case BT_FTPC_STATE_AUTHORIZING:
					return BluetoothProfileManager.STATE_CONNECTING;

				default:
					return BluetoothProfileManager.STATE_UNKNOWN;
			}
		}

		protected void finalize() {
			disconnectService();
			try {
				super.finalize();
			} catch (Throwable t) {
				printErr("Throwable caught in finalize(): " + t);
			}
		}
	}
}
