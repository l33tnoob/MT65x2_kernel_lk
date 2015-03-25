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

import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.bluetooth.R;
// import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.TransferringFile;
import com.mediatek.bluetooth.util.SystemUtils;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothFtp;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.IBluetoothFtpCtrl;
import android.bluetooth.IBluetoothFtpServer;
import android.bluetooth.IBluetoothFtpServerCallback;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.PatternMatcher;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Toast;

import java.io.File;

public class BluetoothFtpService extends Service {

	private static final String TAG = "BluetoothFtpService";

	private static final String SERVICE_THREAD			= "BtFtpServiceThread";

	/* Flag for debug messages */
	private static final boolean DEBUG = true;

	/* Preference name for saved server settings */
	private static final String SERVER_SETTINGS			= "ftp_server_settings";
	private static final String SERVER_KEY_ENABLE		= "server_enable";
	private static final String SERVER_KEY_PERMISSION	= "server_permission";

	/* Internal data keys */
	private static final String INTERNAL_KEY_STR_DATA	= "string_data",
								INTERNAL_KEY_PROGRESS	= "progress",
								INTERNAL_KEY_TOTAL		= "total";

	/* Start of FTP ID space */
	// private static final int FTP_ID_START = BluetoothProfile.getProfileStart(BluetoothProfile.ID_FTP);
	private static final int FTP_ID_START = 6000000;

	/* Notification Intent data key strings */
	protected static final String DEVICE_NAME			= "device_name",
								  NOTIFY_TYPE			= "notify_type";

	/* FTP Server Notification IDs */
	protected static final int FTPS_AUTHORIZE_NOTIFY	= FTP_ID_START + 1,
							   FTPS_CONNECTED_NOTIFY	= FTP_ID_START + 2,
							   FTPC_ACTIVATED_NOTIFY	= FTP_ID_START + 3;

	/* TODO: The constants used in FTP need to be rearranged. */

	/* Internal Handler Message IDs: common part */
	private static final int FTP_INTERNAL_BASE			= 5000,
							 FTP_SHOW_TOAST				= FTP_INTERNAL_BASE + 1,
							 FTP_BT_OFF					= FTP_INTERNAL_BASE + 2,
							 FTP_UPDATE_MEDIA_STORE		= FTP_INTERNAL_BASE + 3,
							 FTP_UPDATE_NOTIFICATION	= FTP_INTERNAL_BASE + 4;

	/* Internal Handler Message IDs: server part */
	private static final int FTPS_ENABLE				= FTP_INTERNAL_BASE + 10,
							 FTPS_DISABLE				= FTP_INTERNAL_BASE + 11,
							 FTPS_AUTHORIZE_RES			= FTP_INTERNAL_BASE + 12;

	/* Internal Handler Message IDs: client part */
	private static final int FTPC_CONNECT				= FTP_INTERNAL_BASE + 20,
							 FTPC_DISCONNECT			= FTP_INTERNAL_BASE + 21,
							 FTPC_ABORT					= FTP_INTERNAL_BASE + 22,
							 FTPC_REFRESH				= FTP_INTERNAL_BASE + 23,
							 FTPC_GOFORWARD				= FTP_INTERNAL_BASE + 24,
							 FTPC_GOBACKWARD			= FTP_INTERNAL_BASE + 25,
							 FTPC_GOTOROOT				= FTP_INTERNAL_BASE + 26,
							 FTPC_CREATE_FOLDER			= FTP_INTERNAL_BASE + 27,
							 FTPC_START_PULL			= FTP_INTERNAL_BASE + 28,
							 FTPC_START_PUSH			= FTP_INTERNAL_BASE + 29,
							 FTPC_DELETE				= FTP_INTERNAL_BASE + 30;

	/* Role IDs */
	private static final int FTP_SERVER					= FTP_INTERNAL_BASE + 100,
							 FTP_CLIENT					= FTP_INTERNAL_BASE + 101;

	/* State IDs for Profile Manager */
	private static final int ENABLING					= BluetoothProfileManager.STATE_ENABLING,
							 ENABLED					= BluetoothProfileManager.STATE_ENABLED,
							 DISABLING					= BluetoothProfileManager.STATE_DISABLING,
							 DISABLED					= BluetoothProfileManager.STATE_DISABLED,
							 ABNORMAL					= BluetoothProfileManager.STATE_ABNORMAL;

	private static final boolean MTK_EMMC_SUPPORT		= FeatureOption.MTK_EMMC_SUPPORT,
								 MTK_2SDCARD_SWAP		= FeatureOption.MTK_2SDCARD_SWAP;

	/* Package only, for braodcast */
	static final String SERVER_DISCONNECTED = "server_disconnected";

	/* Pulled folder-listing-object path. For client */
	static final String XML_PATH = "/data/@btmtk/ftpc_folder_obj.xml";

	/* Default root path */
	static final String DEFAULT_ROOT = "/";

	/* Default internal SD card path */
	static final String DEFAULT_SDCARD_PATH = "/mnt/sdcard/";

	/* Projection for querying transferring queue and start to pull or push */
	private final String[] mTransferringProjection = new String[] {
		TransferringFile._ID,
		TransferringFile.NAME,
		TransferringFile.STATUS,
		TransferringFile.PROGRESS,
		TransferringFile.DIRECTION
	};

	/* ContentResolver for accessing databases */
	private ContentResolver mResolver = null;

	/* Cursor points to transferring queue */
	private Cursor mTransferringCursor = null;

	/* Notification manager service */
	private NotificationManager mNM = null;

	/* Power manager service */
	private PowerManager mPM = null;

	/* WakeLock object to acquire when transferring */
	private WakeLock mWakeLock = null;

	/* SharedPreferences for storing FTP server settings: enable and permission. */
	private SharedPreferences mServerPreferences;

	/* A flag for indicating the state of SD card is unmounting. */
	private boolean mSDUnmounting = false;

	/* IntentFilter for detecting removal of SD card */
	private IntentFilter mSDRemovalFilter = new IntentFilter();

	/* BroadcastReceiver for detecting removal of SD card */
	private BroadcastReceiver mSDRemovalReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			int msg_flag = 0;
			Uri uri = intent.getData();
			String act = intent.getAction();
			String path = null;

			if (uri != null) {
				path = uri.getPath();
			} else {
				printErr("No path specified.");
				return;
			}

			if (Intent.ACTION_MEDIA_EJECT.equals(act)) {

				if (MTK_2SDCARD_SWAP) {
					// For adopting MTK SWAP feature, just terminate any connection if any SD card is ejected.
					printLog("MEDIA_EJECT. Server state: " + mServerState + ", Client state: " + mClientState);
					if (mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE && mServerState != BluetoothFtp.BT_FTPS_STATE_ACTIVE) {
						serverDisconnectReqNative();
					}

					if (mClientState != BluetoothFtp.BT_FTPC_STATE_IDLE && mClientState != BluetoothFtp.BT_FTPC_STATE_ACTIVE) {
						clientDisconnectReqNative();
					}

				} else {

					if (MTK_EMMC_SUPPORT) {
						printLog("MEDIA_EJECT: " + path + " ,mServerTransferName: " + mServerTransferName + 
									" ,mClientTransferName: " + mClientTransferName);

						if (mServerTransferName != null && mServerTransferName.startsWith(path)) {
							printLog("Abort server transferring, file: " + mServerTransferName);
							serverAbortReqNative();
						}

						if (mClientState == BluetoothFtp.BT_FTPC_STATE_SENDING || mClientState == BluetoothFtp.BT_FTPC_STATE_RECEIVING) {
							if (mClientTransferName != null && mClientTransferName.startsWith(path)) {
								printLog("Abort client transferring, file:  " + mClientTransferName);
								clientAbortReqNative();
							}
						}

					} else {
						// Without EMMC support
						printLog("MEDIA_EJECT: " + path);

						// Turn off FTP Server
						if (mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE) {
							msg_flag += 1;
							mNeedUpdatePref = false;
							serverDisableReqNative();
						}

						// Disconnect FTP Client
						if ((mClientState != BluetoothFtp.BT_FTPC_STATE_ACTIVE) &&
							(mClientState != BluetoothFtp.BT_FTPC_STATE_IDLE)) {

							msg_flag += 2;
							mDisconnectByUser = true;

							if (mClientState == BluetoothFtp.BT_FTPC_STATE_AUTHORIZING) {
								clientCancelConnectNative();
								cbClientDisconnected();
							} else {
								clientDisconnectReqNative();
							}
						}

						if (msg_flag == 1) {
							showTextToast(R.string.bluetooth_ftp_server_off_sd_unmounted);
						} else if (msg_flag == 2) {
							showTextToast(R.string.bluetooth_ftp_client_off_sd_unmounted);
						} else if (msg_flag == 3) {
							showTextToast(R.string.bluetooth_ftp_both_off_sd_unmounted);
						}
					}
				}

			} else if (Intent.ACTION_MEDIA_MOUNTED.equals(act)) {
				if (MTK_EMMC_SUPPORT) {
					// If EMMC is supported, re-mount SD card is not necessary.

				} else {
					printLog("MEDIA_MOUNTED: " + path + ". Try to enable FTP server");

					// Try to enable FTP server
					BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
					if (bt == null) {
						return;
					}

					if (bt.isEnabled() && mServerPreferences.getBoolean(SERVER_KEY_ENABLE, false) &&
						(mServerState == BluetoothFtp.BT_FTPS_STATE_IDLE) &&
						SystemUtils.isExternalStorageMounted(BluetoothFtpService.this, path)) {

						mNeedUpdatePref = false;
						serverEnableReqNative(SystemUtils.getMountPointPath());
					}
				}

			} else {
				// do-nothing.
			}
		}
	};

	/* IntentFilter for receiving BT Power-off */
	private IntentFilter mBtOffFilter = new IntentFilter();

	/* BroadcastReceiver for receiving BT Power-off */
	private BroadcastReceiver mBtOffReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			int btState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
			String act = intent.getAction();

			if (act.equals(BluetoothAdapter.ACTION_STATE_CHANGED) &&
				btState == BluetoothAdapter.STATE_TURNING_OFF) {
				if (clearService()) {
					sendServiceMsg(FTP_BT_OFF, 0);
				}
			}
		}
	};

	/* Callback assoicated with the service handler */
	private Callback mServiceCallback = new Callback() {
		public boolean handleMessage(Message msg) {

			Intent intent = null;
			Notification noti = null;
			Bundle data = msg.getData();
			String strData = (data != null) ? data.getString(INTERNAL_KEY_STR_DATA) : null;

			switch (msg.what) {
				case BluetoothFtpService.FTPS_ENABLE:
					mNeedUpdatePref = true;
					serverEnableReqNative(SystemUtils.getMountPointPath());
					break;

				case BluetoothFtpService.FTPS_DISABLE:
					mNeedUpdatePref = true;
					serverDisableReqNative();
					break;

				case BluetoothFtp.BT_FTPSUI_AUTHORIZING:
					printLog("Handling: Server Authorizing");
					if (mCurrentClient != null && mCurrentClient.getTrustState()) {
						// Accept automatically
						serverAuthorizeRspNative(true);
					} else {
						// strData: Device name
						noti = genFtpNotification(FTPS_AUTHORIZE_NOTIFY, strData, true);
						mNM.notify(FTPS_AUTHORIZE_NOTIFY, noti);
					}

					handleStateChanged(FTP_SERVER, mCurrentClient,
							BluetoothFtp.BT_FTPS_STATE_ACTIVE,
							BluetoothFtp.BT_FTPS_STATE_AUTHORIZING);
					break;

				case BluetoothFtpService.FTPS_AUTHORIZE_RES:
					if (mServerState == BluetoothFtp.BT_FTPS_STATE_AUTHORIZING) {
						if (msg.arg1 == 1) {
							serverAuthorizeRspNative(true);
						} else {
							serverAuthorizeRspNative(false);
						}
					} else {
						printErr("FTPS_AUTHORIZE_RES, wrong state: " + mServerState);
					}
					break;

				case BluetoothFtp.BT_FTPSUI_CONNECTED:
					printLog("Handling: Server Connected");
					// strData: Device name
					noti = genFtpNotification(FTPS_CONNECTED_NOTIFY, strData, false);
					mNM.notify(FTPS_CONNECTED_NOTIFY, noti);
					handleStateChanged(FTP_SERVER, mCurrentClient,
							BluetoothFtp.BT_FTPS_STATE_AUTHORIZING,
							BluetoothFtp.BT_FTPS_STATE_CONNECTED);
					break;

				case BluetoothFtp.BT_FTPSUI_DISCONNECTED:
					printLog("Handling: Server Disconnected");

					// Braodcast the timeout: dismiss the dialog or clear the notification
					intent = new Intent(SERVER_DISCONNECTED);
					sendBroadcast(intent);

					handleStateChanged(FTP_SERVER, mCurrentClient,
							msg.arg1, BluetoothFtp.BT_FTPS_STATE_ACTIVE);
					mCurrentClient = null;
					mNM.cancel(FTPS_AUTHORIZE_NOTIFY);
					mNM.cancel(FTPS_CONNECTED_NOTIFY);
					break;

				case BluetoothFtp.BT_FTPCUI_CONNECTED:
					printLog("Handling: Client Connected");
					handleStateChanged(FTP_CLIENT, mCurrentServer,
							BluetoothFtp.BT_FTPC_STATE_AUTHORIZING,
							BluetoothFtp.BT_FTPC_STATE_CONNECTED);
					break;

				case BluetoothFtp.BT_FTPCUI_DISCONNECTED:
					printLog("Handling: Client Disconnected");

					handleStateChanged(FTP_CLIENT, mCurrentServer,
							msg.arg1, BluetoothFtp.BT_FTPC_STATE_ACTIVE);
					mCurrentServer = null;
					mNM.cancel(FTPC_ACTIVATED_NOTIFY);
					break;

				case BluetoothFtp.BT_FTPCUI_BROWSED:
					// printLog("Handling: Folder Content Pulled");
					break;

				case BluetoothFtpService.FTPC_START_PUSH:
				case BluetoothFtpService.FTPC_START_PULL:
					startClientTransfer(msg.what);
					break;

				case BluetoothFtpService.FTPC_ABORT:
					mAbortByUser = true;
					if (!clientAbortReqNative()) {
						Log.w(TAG, "[BT][FTP] User aborts after transfer finished.");
					}
					break;

				case BluetoothFtp.BT_FTPCUI_PUSHING:
				case BluetoothFtp.BT_FTPCUI_PULLING:
					handleClientTransferring(msg.what, data);
					break;

				case BluetoothFtp.BT_FTPCUI_PUSHED:
				case BluetoothFtp.BT_FTPCUI_PULLED:
					handleClientTransferResult(msg.what, msg.arg1);
					break;

				case BluetoothFtpService.FTP_SHOW_TOAST:
					showTextToast(msg.arg1);
					break;

				case BluetoothFtpService.FTP_BT_OFF:
					// Close opened cursors
					if (mTransferringCursor != null) {
						mTransferringCursor.close();
						mTransferringCursor = null;
					}

					// Release the WakeLock
					if (mWakeLock.isHeld()) {
						mWakeLock.setReferenceCounted(false);
						mWakeLock.release();
					}
					mWakeLock = null;
					break;

				case BluetoothFtpService.FTP_UPDATE_MEDIA_STORE:
					// strData: File path
					updateMediaStore(strData);
					break;

				case BluetoothFtpService.FTP_UPDATE_NOTIFICATION:
					updateNotification(msg.arg1);
					break;

				default:
					return false;
			}
			return true;
		}
	};

	/* The service thread associated with service handler */
	private HandlerThread mServiceThread;

	/* The service handler */
	private Handler mServiceHandler;

	/* Service flag */
	private boolean mServiceInitiated = false;
	private boolean mServiceStarted = false;

	/* Server state */
	private boolean mNeedUpdatePref = false;
	private int mServerState;
	private int mServerPermission;
	private String mServerTransferName;
	private BluetoothDevice mCurrentClient;

	/* Client state */
	private boolean mDisconnectByUser = false;
	private boolean mAbortByUser = false;
	private int mClientState;
	private String mClientCurrentPath;
	private String mClientNextPath;
	private String mClientPulledLocalPath;
	private String mClientTransferName;
	private BluetoothDevice mCurrentServer;

	/* Native data */
	private int mNativeData;

	private class SocketListenerThread extends Thread {
		private boolean init_ok = false;

		public SocketListenerThread() {
			super("BtFtpMessageListener");
			init_ok = prepareListentoSocketNative();
			printLog("After preparing, init_ok: " + init_ok);
		}

		@Override
		public void run() {
			boolean job_done = false;
			if (init_ok) {
				job_done = listentoSocketNative();
			}
			printLog("SocketListener exited. job_done: " + job_done);
		}

		public void shutdown() {
			printLog("Shutdown socketListener.");
			stopListentoSocketNative();
		}
	}

	/* A thread taht keep listening to the socket for incoming ILM */
	private SocketListenerThread mSocketListener = null;

/********************************************************************************************
 * Binder Interface Objects Definitions and Binder Callbacks
 ********************************************************************************************/

	/* The binder object for launching FTP client and requesting connection status. */
	private final IBluetoothFtpCtrl.Stub mFtpClientCtrl = new IBluetoothFtpCtrl.Stub() {

		public int getState() {
			return mClientState;
		}

		public BluetoothDevice getCurrentDevice() {
			return mCurrentServer;
		}

		public void connect(BluetoothDevice device) {
			printLog("Launching FTP Client");
			boolean launched = false;

			if (MTK_EMMC_SUPPORT) {
				// Don't check SD card because EMMC is supported.
			} else {
				String sdPath = Environment.getExternalStorageDirectory().getAbsolutePath();
				if (!SystemUtils.isExternalStorageMounted(BluetoothFtpService.this, sdPath)) {
					printWrn("No SD card for FTP client");
					sendServiceMsg(FTP_SHOW_TOAST, R.string.bluetooth_ftp_client_sd_not_ready);
					return;
				}
			}

			handleStateChanged(FTP_CLIENT, device,
					BluetoothFtp.BT_FTPC_STATE_ACTIVE,
					BluetoothFtp.BT_FTPC_STATE_AUTHORIZING);

			if (mClientState == BluetoothFtp.BT_FTPC_STATE_ACTIVE) {
				Notification noti = genFtpNotification(FTPC_ACTIVATED_NOTIFY, device.getName(), false);
				mNM.notify(FTPC_ACTIVATED_NOTIFY, noti);

				Intent intent = new Intent();

				mClientCurrentPath = DEFAULT_ROOT;
				mCurrentServer = device;

				intent.setClassName(getPackageName(), BluetoothFtpClient.class.getName())
					  .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

				mDisconnectByUser = false;
				launched = true;
				startActivity(intent);

			} else if (mClientState == BluetoothFtp.BT_FTPC_STATE_IDLE) {
				printErr("FTP client is not available.");
				sendServiceMsg(FTP_SHOW_TOAST, R.string.bluetooth_ftp_client_connection_failed);

			} else {
				// Try establishing the connection with some device, reject other attemptions.
				sendServiceMsg(FTP_SHOW_TOAST, R.string.bluetooth_ftp_service_busy);
			}

			if (!launched) {
				handleStateChanged(FTP_CLIENT, device,
						BluetoothFtp.BT_FTPC_STATE_AUTHORIZING,
						BluetoothFtp.BT_FTPC_STATE_ACTIVE);
			}
		}

		public void disconnect(BluetoothDevice device) {
			mDisconnectByUser = true;
			clientDisconnectReqNative();
		}

	};

	/* The binder object for monitoring FTP server and requesting to disconnect. */
	private final IBluetoothFtpCtrl.Stub mFtpServerCtrl = new IBluetoothFtpCtrl.Stub() {

		public int getState() {
			return mServerState;
		}

		public BluetoothDevice getCurrentDevice() {
			return mCurrentClient;
		}

		public void connect(BluetoothDevice device) {
			// do nothing.
		}

		public void disconnect(BluetoothDevice device) {
			if (device != null && device.equals(mCurrentClient)) {
				mNM.cancel(FTPS_CONNECTED_NOTIFY);
				serverDisconnectReqNative();
			} else {
				printErr("Invalid disconnect command");
			}
		}
	};


	/* The callback list to BluetoothAdvancedSettings, FTP part. */
	private final RemoteCallbackList<IBluetoothFtpServerCallback> mFtpServerCallback
			= new RemoteCallbackList<IBluetoothFtpServerCallback>();

	/* The binder object for BluetoothAdvancedSettings, FTP part. */
	private final IBluetoothFtpServer.Stub mFtpServer = new IBluetoothFtpServer.Stub() {

		public String getName() {
			return "Name";
		}

		public void registerCallback(IBluetoothFtpServerCallback cb) {
			if (cb != null) mFtpServerCallback.register(cb);
		}

		public void unregisterCallback(IBluetoothFtpServerCallback cb) {
			if (cb != null) mFtpServerCallback.unregister(cb);
		}

		public boolean enable() {
			if (mServerState == BluetoothFtp.BT_FTPS_STATE_IDLE) {
				sendServiceMsg(FTPS_ENABLE, 0);
			} else {
				printLog("Server has already been enabled.");
				postServerEvent(BluetoothFtp.BT_FTPSUI_READY);
			}
			return true;
		}

		public void disable() {
			if (mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE) {
				sendServiceMsg(FTPS_DISABLE, 0);
			} else {
				printLog("Server has already been disabled.");
				postServerEvent(BluetoothFtp.BT_FTPSUI_DISABLED);
			}
		}

		public int getStatus() {
			return mServerState;
		}

		public boolean setPermission(int permission) {
			boolean read_only = (permission == BluetoothFtp.BT_FTPS_READONLY);
			mServerPermission = read_only?
					BluetoothFtp.BT_FTPS_READONLY : BluetoothFtp.BT_FTPS_FULLCTRL;

			mServerPreferences.edit()
							  .putInt(SERVER_KEY_PERMISSION, mServerPermission)
							  .commit();

			return setPermissionNative(read_only);
		}

		public int getPermission() {
			return mServerPermission;
		}

		public boolean setRootDir(String newRoot) {
			boolean ret = false;
			// not yet implement
			// ret = setRootDirNative(newroot);
			return ret;
		}
	};

	/* The binder object for BluetoothFtpServerAuth and BluetoothFtpServerConn */
	private final IBluetoothFtpServerNotify.Stub mFtpServerNotify = new IBluetoothFtpServerNotify.Stub() {

		public void authResult(boolean res) {
			printLog("Authorize: " + res);
			sendServiceMsg(FTPS_AUTHORIZE_RES, res ? 1 : 0);
		}

		public void disconnect() {
			printLog("Disconnect from UI");
			serverDisconnectReqNative();
		}

		public void updateNotify(int notify) {
			sendServiceMsg(FTP_UPDATE_NOTIFICATION, notify);
		}
	};

	/* The callback list to BluetoothFtpClient */
	private final RemoteCallbackList<IBluetoothFtpClientCallback> mFtpClientCallback
			= new RemoteCallbackList<IBluetoothFtpClientCallback>();

	/* The binder object for BluetoothFtpClient */
	private final IBluetoothFtpClient.Stub mFtpClient = new IBluetoothFtpClient.Stub() {

		public void registerCallback(IBluetoothFtpClientCallback cb) {
			if (cb != null) mFtpClientCallback.register(cb);
		}

		public void unregisterCallback(IBluetoothFtpClientCallback cb) {
			if (cb != null) mFtpClientCallback.unregister(cb);
		}

		public int getState() {
			int ret = mClientState;

			// Reserver the intermediate state for client to trigger parsing folder content.
			switch (ret) {
				case BluetoothFtp.BT_FTPC_STATE_TOBROWSE:
				case BluetoothFtp.BT_FTPC_STATE_BROWSED:
					mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
					break;

				case BluetoothFtp.BT_FTPC_STATE_DISCONNECTED:
					mClientState = BluetoothFtp.BT_FTPC_STATE_ACTIVE;
					break;

				default:
					break;
			}

			return ret;
		}

		public String getCurrentPath() {
			return mClientCurrentPath;
		}

		public int getLastTransferResult() {
			return -1;
		}

		public void connect() {
			if (mCurrentServer != null && mClientState == BluetoothFtp.BT_FTPC_STATE_ACTIVE) {
				printLog("Connect to: " + mCurrentServer.getAddress());

				// prepareClientPulledLocalPath(SystemUtils.getReceivedFilePath(getApplicationContext()));
				mClientState = BluetoothFtp.BT_FTPC_STATE_AUTHORIZING;
				clientConnectReqNative(mCurrentServer.getAddress());

			} else {
				printErr("Invalid connect request: no server device or connected already.");
				postClientEvent(BluetoothFtp.BT_FTPCUI_DISCONNECTED, BluetoothFtp.BT_FTPC_CONNECTION_FAILED);
			}
		}

		public boolean abort() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_SENDING ||
				mClientState == BluetoothFtp.BT_FTPC_STATE_RECEIVING) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_ABORTING;
				sendServiceMsg(FTPC_ABORT, 0);
				return true;
			} else {
				Log.w(TAG, "[BT][FTP] Client aborts without trnasferring.");
				return false;
			}
		}

		public void disconnect() {
			mDisconnectByUser = true;

			if (mClientState == BluetoothFtp.BT_FTPC_STATE_AUTHORIZING) {
				mNM.cancel(FTPC_ACTIVATED_NOTIFY);
				clientCancelConnectNative();
			} else {
				clientDisconnectReqNative();
			}
		}

		public void refresh() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED ||
				mClientState == BluetoothFtp.BT_FTPC_STATE_TOBROWSE) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				clientRefreshReqNative();
			} else {
				printWrn("refresh() in wrong state: " + mClientState);
			}
		}

		public void goForward(String path) {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				mClientNextPath = mClientCurrentPath + 
						(mClientCurrentPath.equals(DEFAULT_ROOT) ? path : ("/" + path));
				printLog("goForward(): " + mClientNextPath);
				clientGoForwardReqNative(path);
			} else {
				printWrn("goForward() in wrong state: " + mClientState);
			}
		}

		public void goBackward() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				int lastSlash = mClientCurrentPath.lastIndexOf("/");
				mClientNextPath = (lastSlash > 0 ? mClientCurrentPath.substring(0, lastSlash) : DEFAULT_ROOT);
				printLog("goBackward(): " + mClientNextPath);
				clientGoBackwardReqNative();
			} else {
				printWrn("goBackward() in wrong state: " + mClientState);
			}
		}

		public void goToRoot() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				mClientNextPath = DEFAULT_ROOT;
				printLog("goToRoot(): " + mClientNextPath);
				clientGoToRootReqNative();
			} else {
				printWrn("goToRoot() in wrong state: " + mClientState);
			}
		}

		public void createFolder(String name) {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				mClientNextPath = mClientCurrentPath;
				clientCreateFolderReqNative(name);
			} else {
				printWrn("createFolder() in wrong state: " + mClientState);
			}
		}

		public void startPull() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_RECEIVING;
				sendServiceMsg(FTPC_START_PULL, null);
			} else {
				printWrn("startPull() in wrong state: " + mClientState);
			}
		}

		public void startPush() {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_SENDING;
				sendServiceMsg(FTPC_START_PUSH, null);
			} else {
				printWrn("startPush() in wrong state: " + mClientState);
			}
		}

		public void delete(String name) {
			if (mClientState == BluetoothFtp.BT_FTPC_STATE_CONNECTED) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSING;
				clientDeleteReqNative(name);
			} else {
				printWrn("delete() in wrong state: " + mClientState);
			}
		}
	};

/********************************************************************************************
 * Life-cycle Callback Functions of FTP Service
 ********************************************************************************************/

	@Override
	public void onCreate() {
		printLog("Enter onCreate()");

		mResolver = getContentResolver();

		// Request system services
		mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		if (mNM == null) {
			printErr("Get Notification-Manager failed. Stop FTP service.");
			stopSelf();
		}

		mPM = (PowerManager) getSystemService(Context.POWER_SERVICE);
		if (mPM == null) {
			printErr("Get Power-Manager failed. Stop FTP service.");
			stopSelf();
		}

		// Initialize the service handler
		mServiceThread = new HandlerThread(SERVICE_THREAD);
		mServiceThread.start();
		mServiceHandler = new Handler(mServiceThread.getLooper(), mServiceCallback);

		// Basically initialize member variables
		mServerState = BluetoothFtp.BT_FTPS_STATE_IDLE;
		mClientState = BluetoothFtp.BT_FTPC_STATE_IDLE;

		// We haven't connected to any FTP server
		mCurrentServer = null;

		mBtOffFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
		registerReceiver(mBtOffReceiver, mBtOffFilter);

		mServiceInitiated = initServiceNative();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		printLog("Enter onStartCommand()");

		// Notify Profile Manager the activation is enabling.
		updateActivation(FTP_SERVER, ENABLING);
		updateActivation(FTP_CLIENT, ENABLING);

		if (mServiceInitiated) {
			printLog("BluetoothFtpService is initiated.");

			BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
			if (bt == null) {
				Log.w(TAG, "[BT][FTP] Bluetooth is not supported.");
				return START_NOT_STICKY;
			} else {
				if (bt.getState() == BluetoothAdapter.STATE_TURNING_OFF || bt.getState() == BluetoothAdapter.STATE_OFF ) {
					// Due to BT has turned off when we are trying to start FTP service, report ABNORMAL.
					Log.w(TAG, "[BT][FTP] Bluetooth is not enabled. Abandon starting FTP service this time.");
					updateActivation(FTP_SERVER, ABNORMAL);
					updateActivation(FTP_CLIENT, ABNORMAL);
					return START_NOT_STICKY;
				}
			}

			if (mSocketListener == null) {
				mSocketListener = new SocketListenerThread();
				mSocketListener.start();
				printLog("SocketListener started.");

			} else {
				Log.w(TAG, "[BT][FTP] FTP service has started.");
				// Attempting to start FTP service when it is activated.
				updateActivation(FTP_SERVER, ENABLED);
				updateActivation(FTP_CLIENT, ENABLED);
				return START_NOT_STICKY;
			}

			// Prepare the SD card removal receiver, and register
			String root = SystemUtils.getMountPointPath();
			printLog("Pre-enable FTP Server, root: " + root);

			mWakeLock = mPM.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);

			// Check if need to eanble server in advance
			mServerPreferences = getSharedPreferences(SERVER_SETTINGS, 0);
			mServerPermission = mServerPreferences.getInt(SERVER_KEY_PERMISSION,
					BluetoothFtp.BT_FTPS_READONLY);

			if (mServerPreferences.getBoolean(SERVER_KEY_ENABLE, false)) {

				if (MTK_EMMC_SUPPORT) {
					serverEnableReqNative(root);

				} else {
					if (SystemUtils.isExternalStorageMounted(this, root)) {
						serverEnableReqNative(root);
					}
				}
			}

			printLog("FTP server init-state: " + mServerState + ", permission: " + mServerPermission);
			updateActivation(FTP_SERVER, ENABLED);

			// Enable FTP client
			if (clientEnableReqNative()) {
				mClientState = BluetoothFtp.BT_FTPC_STATE_ACTIVE;
				updateActivation(FTP_CLIENT, ENABLED);
			} else {
				updateActivation(FTP_CLIENT, ABNORMAL);
			}

			mSDRemovalFilter.addAction(Intent.ACTION_MEDIA_EJECT);
			mSDRemovalFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
			mSDRemovalFilter.addDataScheme("file");
			registerReceiver(mSDRemovalReceiver, mSDRemovalFilter);

			mServiceStarted = true;

		} else {
			printLog("Failed to init BluetoothFtpService. Stop FTP service.");
			stopSelf();
		}

		return START_STICKY;
	}

	public IBinder onBind(Intent intent) {
		String action = intent.getAction();
		Log.i(TAG, "[BT][FTP] Enter onBind(): " + action);

		if (BluetoothFtp.ACTION_BIND_SERVER.equals(action)) {
			return mFtpServerCtrl;
		} else if (BluetoothFtp.ACTION_BIND_CLIENT.equals(action)) {
			return mFtpClientCtrl;
		} else if (IBluetoothFtpServer.class.getName().equals(action)) {
			if (mServiceStarted == false) {
		    	startService(new Intent(this, BluetoothFtpService.class));
			}
			return mFtpServer;
		} else if (IBluetoothFtpServerNotify.class.getName().equals(action)) {
			return mFtpServerNotify;
		} else if (IBluetoothFtpClient.class.getName().equals(action)) {
			if (mServiceStarted == false) {
		    	startService(new Intent(this, BluetoothFtpService.class));
			}
			return mFtpClient;
		}

		return null;
	}

	@Override
	public void onDestroy() {
		printLog("onDestroy()");

		boolean timeout = false;
		int cnt = 0;

		unregisterReceiver(mBtOffReceiver);

		cleanServiceNative();
		mFtpServerCallback.kill();

		mServiceHandler = null;
		if (mServiceThread != null) {
			mServiceThread.quit();
			mServiceThread = null;
		}
	}

	/** For disabling all capabilities of FTP service. This function may be called 
	 *  when BT is turning off. Under this situation, FTP server and client should 
	 *  disconnect its connection, and change back to IDLE state. SocketListener 
	 *  should also be shut down here.
	 *
	 *  If this function is called before onStartCommand(), there will be an 
	 *  IllegalArgumentException thrown because of the not-yet-registered receiver.
	 */
	private synchronized boolean clearService() {
		printLog("clearService()");

		boolean timeout = false;
		int cnt = 0;

		updateActivation(FTP_SERVER, DISABLING);
		updateActivation(FTP_CLIENT, DISABLING);

		try {
			unregisterReceiver(mSDRemovalReceiver);
		} catch (IllegalArgumentException iae) {
			Log.w(TAG, "[BT][FTP] BT power-off before FTP service is ready.");
			updateActivation(FTP_SERVER, DISABLED);
			updateActivation(FTP_CLIENT, DISABLED);
			return false;
		}

		// Disable FTP client.
		if ((mClientState != BluetoothFtp.BT_FTPC_STATE_ACTIVE) &&
			(mClientState != BluetoothFtp.BT_FTPC_STATE_IDLE)) {
			mDisconnectByUser = true;
			clientDisconnectReqNative();
		}

		// Disable FTP server
		if (mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE) {
			mNeedUpdatePref = false;
			serverDisableReqNative();
		}

		while (mClientState != BluetoothFtp.BT_FTPC_STATE_ACTIVE ||
			   mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE) {
			if (cnt >= 300) {
				timeout = true;
				break;
			}

			try {
				Thread.sleep(30);
			} catch (Exception e) {
				printErr("Waiting for client disconnect-ind was interrupted.");
			}
			cnt += 30;
		}

		// Shut SocketListener down
		if (mSocketListener != null) {
			try {
				mSocketListener.shutdown();
				mSocketListener.join(300);
				mSocketListener = null;
			} catch (InterruptedException e) {
				printErr("mSocketListener close error.");
			}
		}

		if (timeout) {
			if (mClientState != BluetoothFtp.BT_FTPC_STATE_ACTIVE) {
				Log.w(TAG, "[BT][FTP] Waiting FTPC_TPDISCONNECT_IND time-out. Force clear client context.");

				// Force clear client context and fake invoking the callback
				forceClearClientNative();
				cbClientDisconnected();
			}

			if (mServerState != BluetoothFtp.BT_FTPS_STATE_IDLE) {
				Log.w(TAG, "[BT][FTP] Waiting FTPS_DEREGISTER_SERVER_CNF time-out. Force clear server context.");

				// Force clear server context and fake invoking the callback
				forceClearServerNative();
				cbServerDisableResult(true);
			}
		}

		clientDisableReqNative();
		mClientState = BluetoothFtp.BT_FTPC_STATE_IDLE;
		updateActivation(FTP_CLIENT, DISABLED);
		updateActivation(FTP_SERVER, DISABLED);

		mServiceStarted = false;

		return true;
	}

/********************************************************************************************
 * Utility Functions
 ********************************************************************************************/

	/* Utility function: printLog */
	private void printLog(String msg) {
		if (DEBUG) Log.d(TAG, "[BT][FTP] " +msg);
	}

	private void printWrn(String msg) {
		Log.w(TAG, "[BT][FTP] " + msg);
	}

	private void printErr(String msg) {
		Log.e(TAG, "[BT][FTP] " + msg);
	}

	/* Utility function: sendServiceMsg */
	private void sendServiceMsg(int what, Bundle data) {
		Message msg = Message.obtain();
		msg.what = what;
		msg.setData(data);

		if (mServiceHandler != null) {
		mServiceHandler.sendMessage(msg);
		} else {
			Log.e(TAG, "[BT][FTP] mServiceHandler is null. msg: " + what);
		}
	}

	/* Utility function: sendServiceMsg simplified-argument version */
	private void sendServiceMsg(int what, int arg1) {
		Message msg = Message.obtain();
		msg.what = what;
		msg.arg1 = arg1;

		if (mServiceHandler != null) {
		mServiceHandler.sendMessage(msg);
		} else {
			Log.e(TAG, "[BT][FTP] mServiceHandler is null. msg: " + what);
		}
	}

	/* Utility function: genFtpNotification */
	private Notification genFtpNotification(final int type, String deviceName, boolean enableEffects) {

		Context context = getApplicationContext();
		Intent intent = new Intent();
		Notification noti = null;
		PendingIntent contentIntent = null;
		int defaults = (enableEffects) ? 
				(Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE)	: 0;

		int icon_id = -1, flags;
		String clazz = null, ticker = null, title = null, msg = null;

		if (type == FTPS_AUTHORIZE_NOTIFY) {
			icon_id = android.R.drawable.stat_sys_data_bluetooth;
			clazz = BluetoothFtpServerNotify.class.getName();
			ticker = getString(R.string.bluetooth_ftp_server_authorize_notify_ticker);
			title = getString(R.string.bluetooth_ftp_server_authorize_notify_title);
			msg = getString(R.string.bluetooth_ftp_server_authorize_notify_message);
			flags = Notification.FLAG_AUTO_CANCEL;

		} else if (type == FTPS_CONNECTED_NOTIFY) {
			icon_id = R.drawable.btftp_ic_server_connected;
			clazz = BluetoothFtpServerNotify.class.getName();
			ticker = getString(R.string.bluetooth_ftp_server_connected_notify_ticker);
			title = String.format(
						getString(R.string.bluetooth_ftp_server_connected_notify_title),
						deviceName);
			msg = getString(R.string.bluetooth_ftp_server_connected_notify_message);
			flags = Notification.FLAG_AUTO_CANCEL;

		} else if (type == FTPC_ACTIVATED_NOTIFY) {
			icon_id = R.drawable.btftp_ic_client_connected;
			clazz = BluetoothFtpClient.class.getName();
			ticker = getString(R.string.bluetooth_ftp_client_activated_notify_ticker);
			title = String.format(
						getString(R.string.bluetooth_ftp_client_activated_notify_title),
						deviceName);
			msg = getString(R.string.bluetooth_ftp_client_activated_notify_message);
			flags = Notification.FLAG_NO_CLEAR;

		} else {
			return null;
		}

		intent.setClassName(getPackageName(), clazz)
			  .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
			  .putExtra(NOTIFY_TYPE, type)
			  .putExtra(DEVICE_NAME, deviceName);

		noti = new Notification(icon_id, ticker, System.currentTimeMillis());
		noti.defaults |= defaults;
		noti.flags |= flags;
		contentIntent = PendingIntent.getActivity(BluetoothFtpService.this, 0,
				intent, PendingIntent.FLAG_UPDATE_CURRENT);
		noti.setLatestEventInfo(context, title, msg, contentIntent);

		return noti;
	}

	/* Utility function: updateNotification */
	private void updateNotification(int notify) {
		Notification noti = null;
		String deviceName = null;

		if (mCurrentClient != null) {
			deviceName = mCurrentClient.getName();
		} else {
			printErr("updateNotification(), mCurrentClient is null.");
			return;
		}

		// Make sure we are in correct states, or just ignore it.
		if (notify == FTPS_AUTHORIZE_NOTIFY) {
			if (mServerState != BluetoothFtp.BT_FTPS_STATE_AUTHORIZING)
				return;
		} else if (notify == FTPS_CONNECTED_NOTIFY) {
			if (mServerState != BluetoothFtp.BT_FTPS_STATE_CONNECTED)
				return;
		}

		noti = genFtpNotification(notify, deviceName, false);
		mNM.notify(notify, noti);
	}

	/* Utility function: Check if the file-name is used by another and return an available one */
	private String getAvailableFileName(String path, String name) {
		String ret = null;
		File file = new File(path, name);

		// No need to find another name, return null.
		if (!file.exists()) {
			return ret;
		}

		// At most 1000 duplicated files are allowed.
		// If more, the last duplicated will be over-written.
		for (int i = 0; i < 1000; i++) {
			ret = String.format("(DUP %02d)%s", i, name);
			file = new File(path, ret);
			if (!file.exists()) {
				break;
			}
		}

		return ret;
	}

	/* Get the URI of current transferring record. */
	private Uri getTransferringUri() {
		if (mTransferringCursor == null) {
			return null;
		} else {
			int id_idx = mTransferringCursor.getColumnIndex(TransferringFile._ID);
			int id = mTransferringCursor.getInt(id_idx);
			Uri uri = ContentUris.withAppendedId(TransferringFile.CONTENT_URI, id);
			return uri;
		}
	}

	/* Show the disconnected message in a Toast. */
	private void showTextToast(int text_id) {
		Toast toast = Toast.makeText(BluetoothFtpService.this, text_id, Toast.LENGTH_LONG);
		toast.show();
	}

	private int convertState(int state) {
		int ret = -1;
		switch (state) {
			case BluetoothFtp.BT_FTPS_STATE_ACTIVE:
			case BluetoothFtp.BT_FTPC_STATE_ACTIVE:
				// ret = BluetoothProfileManager.STATE_DISCONNECTED;
				ret = BluetoothProfile.STATE_DISCONNECTED;
				break;

			case BluetoothFtp.BT_FTPS_STATE_AUTHORIZING:
			case BluetoothFtp.BT_FTPC_STATE_AUTHORIZING:
				// ret = BluetoothProfileManager.STATE_CONNECTING;
				ret = BluetoothProfile.STATE_CONNECTING;
				break;

			case BluetoothFtp.BT_FTPS_STATE_CONNECTED:
			case BluetoothFtp.BT_FTPC_STATE_CONNECTED:
			case BluetoothFtp.BT_FTPC_STATE_SENDING:
			case BluetoothFtp.BT_FTPC_STATE_RECEIVING:
			case BluetoothFtp.BT_FTPC_STATE_BROWSING:
			case BluetoothFtp.BT_FTPC_STATE_BROWSED:
			case BluetoothFtp.BT_FTPC_STATE_ABORTING:
				// ret = BluetoothProfileManager.STATE_CONNECTED;
				ret = BluetoothProfile.STATE_CONNECTED;
				break;

			default:
				printErr("Invalid state: " + state);
				break;
		}
		return ret;
	}

	/* Broadcast that FTP server or client is (de)activated or abnormal. */
	private void updateActivation(int role, int state) {
		BluetoothProfileManager.Profile profile;
		Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);

		profile = (role == FTP_SERVER) ?
				BluetoothProfileManager.Profile.Bluetooth_FTP_Server :
				BluetoothProfileManager.Profile.Bluetooth_FTP_Client;

		intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, profile);
		intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);

		sendBroadcast(intent, android.Manifest.permission.BLUETOOTH);
	}

	/* Broadcast that the state of FTP server or client has changed. */
	private void handleStateChanged(int role, BluetoothDevice device, int from, int to) {
		Intent intent = new Intent(BluetoothFtp.ACTION_STATE_CHANGED);
		int oldState, newState;

		if (role == FTP_SERVER) {
			// Ignore server state for now.
			return;
		}

		oldState = convertState(from);
		newState = convertState(to);
		if (oldState == -1 || newState == -1) {
			printErr("Invalid states, from: " + from + ", to: " + to);
			return;
		}

		intent.putExtra(BluetoothDevice.EXTRA_DEVICE, device);
		intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, oldState);
		intent.putExtra(BluetoothProfile.EXTRA_STATE, newState);

		sendBroadcast(intent, android.Manifest.permission.BLUETOOTH);
	}

	/* Update value of FTP server enabled-preference */
	private void updateServerEnablePreferenceValue(boolean enable) {
		if (mNeedUpdatePref) {
			mServerPreferences.edit()
							  .putBoolean(SERVER_KEY_ENABLE, enable)
							  .commit();
		} else {
			// To be removed
			Log.v(TAG, "[BT][FTP] Ignore updating the preference this time.");
		}
	}

	private void updateMediaStore(String path) {
		try {
			Uri uri = MediaStore.Files.getContentUri("external");
			int ret = mResolver.delete(uri, MediaStore.MediaColumns.DATA + "=?", 
					new String[] {path});
			if (ret > 0) {
				printLog("Deleted " + path + " in: " + uri.toString() + ", " + ret + " row(s) deleted.");
			}
		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] updateMediaStore(), Exception: " + ex);
		}
	}

	private void prepareClientPulledLocalPath(String path) {

		mClientPulledLocalPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		if (path == null) return;

		File file = new File(path);
		if (file.exists()) {
			if (file.isDirectory()) {
				mClientPulledLocalPath = path;
				return;
			}

		} else {
			if (file.mkdirs()) {
				mClientPulledLocalPath = path;
				return;
			}
		}
	}

	private void scanPath(String path) {
		// printLog("File path to scan: " + path);

		Uri uri = Uri.fromFile(new File(path));
		Intent intent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri);
		sendBroadcast(intent);

	}

	private String getAdjustedFilePath(String filePath) {
		File file = null;

		try {
			file = new File(filePath);
			return file.getAbsolutePath();

		} catch (Exception e) {
			return null;
		}
	}

/********************************************************************************************
 * Callback Functions for FTP Server
 ********************************************************************************************/

	/* AIDL callback to server control appilcation */
	private int postServerEvent(int event) {
		int ret = 0;
		printLog("postServerEvent: " + event);

		if (mFtpServerCallback != null) {
			final int N = mFtpServerCallback.beginBroadcast();
			// printLog("Start broadcasting to callback. N=" + N);
			ret = N;

			for (int i = 0; i < N; i++) {
				try {
					mFtpServerCallback.getBroadcastItem(i).postEvent(event, null);
				} catch (RemoteException e) {
					// do nothing.
				}
			}
			mFtpServerCallback.finishBroadcast();

		} else {
			printLog("mFtpServerCallback is null");
		}

		return ret;
	}

	/* Callback for enable server result */
	private void cbServerEnableResult(boolean result) {
		printLog("Enter cbServerEnableResult(): " + result);
		if (result) {
			mServerState = BluetoothFtp.BT_FTPS_STATE_ACTIVE;

			updateServerEnablePreferenceValue(true);
			setPermissionNative(mServerPermission == BluetoothFtp.BT_FTPS_READONLY);
			postServerEvent(BluetoothFtp.BT_FTPSUI_READY);

		} else {
			mServerState = BluetoothFtp.BT_FTPS_STATE_IDLE;
			postServerEvent(BluetoothFtp.BT_FTPSUI_DISABLED);
		}
	}

	/* Callback for disable server result */
	private void cbServerDisableResult(boolean result) {
		printLog("Enter cbServerDisableResult()");
		mServerState = BluetoothFtp.BT_FTPS_STATE_IDLE;

		updateServerEnablePreferenceValue(false);
		postServerEvent(BluetoothFtp.BT_FTPSUI_DISABLED);

		// For cancel all notifications and update UI state.
		sendServiceMsg(BluetoothFtp.BT_FTPSUI_DISCONNECTED, BluetoothFtp.BT_FTPS_STATE_CONNECTED);
	}

	/* Callback for authorize indication */
	private void cbServerAuthorizeInd(String addr) {
		printLog("Enter cbServerAuthorizeInd()");
		BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
		String deviceAddr, deviceName;
		Bundle data = new Bundle();

		if (bt == null) {
			printErr("Bluetooth is not supported");
			return;
		}

		deviceAddr = addr.toUpperCase();
		printLog(deviceAddr);
		if (BluetoothAdapter.checkBluetoothAddress(deviceAddr)) {
			mCurrentClient = bt.getRemoteDevice(deviceAddr);
			deviceName = mCurrentClient.getName();
			data.putString(INTERNAL_KEY_STR_DATA, deviceName);
			mServerState = BluetoothFtp.BT_FTPS_STATE_AUTHORIZING;
			sendServiceMsg(BluetoothFtp.BT_FTPSUI_AUTHORIZING, data);
		} else {
			printErr("Invalid MAC address: " + deviceAddr);
			serverAuthorizeRspNative(false);
			return;	
		}
	}

	/* Callback for connect indication */
	private void cbServerConnectInd(String addr) {
		printLog("Enter cbServerConnectInd()");
		BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
		String deviceAddr, deviceName;
		Bundle data = new Bundle();

		if (bt == null) {
			printErr("Bluetooth is not supported");
			return;
		}

		deviceAddr = addr.toUpperCase();
		printLog(deviceAddr);
		if (BluetoothAdapter.checkBluetoothAddress(deviceAddr)) {
			// mCurrentClient = bt.getRemoteDevice(deviceAddr);
			deviceName = mCurrentClient.getName();
			data.putString(INTERNAL_KEY_STR_DATA, deviceName);
			mServerState = BluetoothFtp.BT_FTPS_STATE_CONNECTED;
			sendServiceMsg(BluetoothFtp.BT_FTPSUI_CONNECTED, data);
		} else {
			printErr("Invalid MAC address: " + deviceAddr);
			serverDisconnectReqNative();
			return;
		}
	}

	/* Callback for disconnect indication */
	private void cbServerDisconnectInd() {
		printLog("Enter cbServerDisconnectInd()");
		int prevState = mServerState;
		mServerState = BluetoothFtp.BT_FTPS_STATE_ACTIVE;
		sendServiceMsg(BluetoothFtp.BT_FTPSUI_DISCONNECTED, prevState);
	}

	/* Callback for starting transferring */
	private void cbServerStartTransferring(String fileName) {
		printLog("Start transferring, filename: " + fileName);
		mServerTransferName = getAdjustedFilePath(fileName);
		mWakeLock.acquire();
	}

	/* Callback for transferring complete */
	private void cbServerEndTransferring() {
		if (mWakeLock.isHeld()) {
			mWakeLock.release();
		}
		mServerTransferName = null;
	}

	/* [TO BE DEPRECATED] Callback for storing the file path to push */
	private void cbServerStartPushing(String filePath) {
		String scanPath = null;
		if (filePath != null && filePath.startsWith(DEFAULT_SDCARD_PATH)) {
			scanPath = Environment.getExternalStorageDirectory().getAbsolutePath();
			scanPath = scanPath + "/" + filePath.substring(DEFAULT_SDCARD_PATH.length());

		} else {
			scanPath = filePath;
		}

		printLog("Start pushing, path: " + filePath + ", scan path: " + scanPath);
		mServerTransferName = getAdjustedFilePath(scanPath);
		mWakeLock.acquire();
	}

	/* Callback for pushing complete */
	private void cbServerEndPushing(int res) {
		printLog("End pushing, res: " + res);
		if (res == 0) {
			scanPath(mServerTransferName);
		}
		mServerTransferName = null;

		if (mWakeLock.isHeld()) {
			mWakeLock.release();
		}
	}

	/* Callback for file or folder deleted */
	private void cbServerObjectDeleted(String filePath) {
		printLog("Deleted file path: " + filePath);
		Bundle data = new Bundle();
		data.putString(INTERNAL_KEY_STR_DATA, filePath);
		sendServiceMsg(FTP_UPDATE_MEDIA_STORE, data);
	}

/********************************************************************************************
 * Operation Functions for FTP Client
 ********************************************************************************************/

	/* Initiates pushing a file to FTP server device*/
	private void startClientTransfer(int msg_id) {
		boolean error = false;
		String name = null;
		Uri updateUri = null;
		ContentValues values = new ContentValues();

		mAbortByUser = false;
		mWakeLock.acquire();

		// Prepare the transferring.
		try {
			mTransferringCursor = mResolver.query(TransferringFile.CONTENT_URI, null, null, null, null);
			if (mTransferringCursor == null) {
				printErr("startClientTransfer(), mTransferringCursor is null");
				return;
			}

			if (mTransferringCursor.moveToFirst()) {
				values.put(TransferringFile.STATUS, TransferringFile.STATUS_TRANSFERRING);
				updateUri = getTransferringUri();
				mResolver.update(updateUri, values, null, null);

				// Get the first file name in transferring queue
				int nameIdx = mTransferringCursor.getColumnIndex(TransferringFile.NAME);
				name = mTransferringCursor.getString(nameIdx);

			} else {
				printErr("No record is in transferring queue.");
				mTransferringCursor.close();
				mTransferringCursor = null;
				return;
			}

		} catch (Exception ex) {
			printErr("startClientTransfer(), msg_id: " + msg_id + " Exception: " + ex);
			clientDisconnectReqNative();
			return;
		}

		if (msg_id == BluetoothFtpService.FTPC_START_PUSH) {
			mClientTransferName = getAdjustedFilePath(name);
			if (!clientPushReqNative(name)) {
				sendServiceMsg(BluetoothFtp.BT_FTPCUI_PUSHED, OBEX.NOT_FOUND);
			}

		} else if (msg_id == BluetoothFtpService.FTPC_START_PULL) {
			prepareClientPulledLocalPath(SystemUtils.getReceivedFilePath(getApplicationContext()));
			String path = mClientPulledLocalPath; 
			String rename = getAvailableFileName(path, name);
			clientPullReqNative(path, name, rename);
			mClientTransferName = getAdjustedFilePath(path + "/" + ((rename == null) ? name : rename));

		} else {
			printErr("Wrong Message ID to start transferring: " + msg_id);
			mTransferringCursor.close();
			mTransferringCursor = null;
		}

	}

	private int nextClientTransfer(int res) {

		if (mTransferringCursor != null && mTransferringCursor.moveToNext()) {

			ContentValues values = new ContentValues();
			Uri updateUri = getTransferringUri();

			if (mAbortByUser) {
				printLog("User aborted between files.");
				values.put(TransferringFile.STATUS, TransferringFile.STATUS_FAILED);
				mResolver.update(updateUri, values, null, null);

				mAbortByUser = false;
				endClientTransfer();
				return OBEX.USER_CANCEL;

			} else {
				values.put(TransferringFile.STATUS, TransferringFile.STATUS_TRANSFERRING);
				mResolver.update(updateUri, values, null, null);
			}

			int nameIdx = mTransferringCursor.getColumnIndex(TransferringFile.NAME);
			int directIdx = mTransferringCursor.getColumnIndex(TransferringFile.DIRECTION);

			String name = mTransferringCursor.getString(nameIdx);
			int direct = mTransferringCursor.getInt(directIdx);
			printLog("Next file to trnasfer: " + name + ", direction: " + direct);

			if (direct == TransferringFile.DIRECTION_PUSH) {
				mClientTransferName = getAdjustedFilePath(name);
				if (!clientPushReqNative(name)) {
					sendServiceMsg(BluetoothFtp.BT_FTPCUI_PUSHED, OBEX.NOT_FOUND);
				}

			} else if (direct == TransferringFile.DIRECTION_PULL) {
				String path = mClientPulledLocalPath;
				String rename = getAvailableFileName(path, name);
				clientPullReqNative(path, name, rename);
				mClientTransferName = getAdjustedFilePath(path + "/" + ((rename == null) ? name : rename));
			}

		} else {
			endClientTransfer();
			printLog("No file to transfer.");
		}

		return res;
	}

	private void handleClientTransferring(int msg, Bundle data) {
		Uri updateUri = getTransferringUri();
		ContentValues values = new ContentValues();
		long progress, total;

		if (updateUri == null || data == null) {
			printErr("handleClientTransferring(), updateUri: " + updateUri + ", data: " + data);
			return;
		}

		progress = data.getLong(TransferringFile.PROGRESS, 0);
		total = data.getLong(TransferringFile.TOTAL, 0);

		if (msg == BluetoothFtp.BT_FTPCUI_PUSHING) {
			printLog("Pushing, progress: " + progress + ", total: " + total);
		} else if (msg == BluetoothFtp.BT_FTPCUI_PULLING) {
			printLog("Pulling, progress: " + progress + ", total: " + total);
		}

		try {
			values.put(TransferringFile.PROGRESS, progress);
			values.put(TransferringFile.TOTAL, total);
			mResolver.update(updateUri, values, null, null);

			postClientEvent(msg, data);
		
		} catch (Exception ex) {
			printErr("handleClientTransferring(), Exception: " + ex);
			clientDisconnectReqNative();
		}
	}

	private void handleClientTransferResult(int msg, int res) {
		Uri updateUri = getTransferringUri();
		ContentValues values = new ContentValues();

		if (updateUri == null) {
			printErr("handleClientTransferResult(), updateUri is null");
			return;
		}

		if (msg == BluetoothFtp.BT_FTPCUI_PUSHED) {
			printLog("Pushed, result: " + res);
		} else if (msg == BluetoothFtp.BT_FTPCUI_PULLED) {
			printLog("Pulled, result: " + res);
		}

		try {
			if (res == 0) {
				if (msg == BluetoothFtp.BT_FTPCUI_PULLED) {
					scanPath(mClientTransferName);
				}
				values.put(TransferringFile.STATUS, TransferringFile.STATUS_SUCCESSFUL);
				res = nextClientTransfer(res);

			} else {
				endClientTransfer();
				values.put(TransferringFile.STATUS, TransferringFile.STATUS_FAILED);
			}

			mResolver.update(updateUri, values, null, null);
			postClientEvent(msg, res);

		} catch (Exception ex) {
			printErr("handleClientTransferResult(), Exception: " + ex);
			clientDisconnectReqNative();
			return;
		}
	}

	/*
	 *   This function is called through mHandler (more precisely, callback from
	 * MessageQueue of Looper in main thread). The point is, we are not up-to-date
	 * with the real client state. So we have to check for unexpected states and
	 * change the state carefully.
	 */
	private void endClientTransfer() {
		if (mWakeLock.isHeld()) {
			mWakeLock.release();
		}

		switch (mClientState) {
			case BluetoothFtp.BT_FTPC_STATE_SENDING:
			case BluetoothFtp.BT_FTPC_STATE_RECEIVING:
			case BluetoothFtp.BT_FTPC_STATE_ABORTING:
				mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
				break;

			default:
				// Do not change the state, cause we are delaied here.
				break;
		}

		if (mTransferringCursor != null) {
			mTransferringCursor.close();
			mTransferringCursor = null;
		}
	}

/********************************************************************************************
 * Callback Functions for FTP Client
 ********************************************************************************************/

	/* AIDL callback to client application */
	private int postClientEvent(int event, int param) {
		int ret = 0;
		// printLog("postClientEvent(" + event + ", " + param + ")");

		if (mFtpClientCallback != null) {
			final int N = mFtpClientCallback.beginBroadcast();
			// printLog("Start broadcasting to callback. N=" + N);
			ret = N;

			for (int i = 0; i < N; i++) {
				try {
					mFtpClientCallback.getBroadcastItem(i).postEvent(event, param);
				} catch (RemoteException e) {
					// do nothing.
				}
			}
			mFtpClientCallback.finishBroadcast();

		} else {
			printLog("mFtpClientCallback is null");
		}

		return ret;
	}

	/* AIDL callback to client application with Bundle data  */
	private int postClientEvent(int event, Bundle data) {
		int ret = 0;

		if (mFtpClientCallback != null) {
			final int N = mFtpClientCallback.beginBroadcast();
			// printLog("Start broadcasting to callback. N=" + N);
			ret = N;

			for (int i = 0; i < N; i++) {
				try {
					mFtpClientCallback.getBroadcastItem(i).postEventWithData(event, data);
				} catch (RemoteException e) {
					// do nothing.
				}
			}
			mFtpClientCallback.finishBroadcast();

		} else {
			printLog("mFtpClientCallback is null");
		}

		return ret;
	}

	/* This function is called when connected with server */
	private void cbClientConnected() {
		printLog("Enter cbClientConnected()");
		mClientState = BluetoothFtp.BT_FTPC_STATE_TOBROWSE;

		postClientEvent(BluetoothFtp.BT_FTPCUI_CONNECTED, 0);
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_CONNECTED, null);
	}

	/* Callback for passive disconnection handling */
	private void cbClientDisconnected() {
		printLog("Enter cbClientDisconnected()");
		int rsp = 0, prevState = mClientState;
		if (mDisconnectByUser) {
			rsp = BluetoothFtp.BT_FTPC_USER_CANCELED;
		} else {
			rsp = (mClientState == BluetoothFtp.BT_FTPC_STATE_AUTHORIZING) ?
				  BluetoothFtp.BT_FTPC_CONNECTION_FAILED :
				  BluetoothFtp.BT_FTPC_UNEXPECTED;
		}

		mClientState = BluetoothFtp.BT_FTPC_STATE_ACTIVE;
		postClientEvent(BluetoothFtp.BT_FTPCUI_DISCONNECTED, rsp);
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_DISCONNECTED, prevState);
	}

	private void cbClientBrowsed(int result) {
		printLog("Enter cbClientBrowsed(): " + result);
		mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
		int postNum = postClientEvent(BluetoothFtp.BT_FTPCUI_BROWSED, result);

		if (postNum <= 0 && (result == BluetoothFtp.BT_FTP_RSP_SUCCESS)) {
			mClientState = BluetoothFtp.BT_FTPC_STATE_BROWSED;
		}
	}

	private void cbClientSetpathed(int result) {
		printLog("Enter cbClientSetpathed(): " + result);
		if (result == BluetoothFtp.BT_FTP_RSP_SUCCESS) {
			mClientCurrentPath = mClientNextPath;
			mClientState = BluetoothFtp.BT_FTPC_STATE_TOBROWSE;
		} else {
			mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
		}

		postClientEvent(BluetoothFtp.BT_FTPCUI_SETPATHED, result);
	}

	private void cbClientPulling(long progress, long total) {
		Bundle data = new Bundle();
		data.putLong(TransferringFile.PROGRESS, progress);
		data.putLong(TransferringFile.TOTAL, total);
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_PULLING, data);
	}

	private void cbClientPulled(int result) {
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_PULLED, result);
	}

	private void cbClientPushing(long progress, long total) {
		Bundle data = new Bundle();
		data.putLong(TransferringFile.PROGRESS, progress);
		data.putLong(TransferringFile.TOTAL, total);
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_PUSHING, data);
	}

	private void cbClientPushed(int result) {
		sendServiceMsg(BluetoothFtp.BT_FTPCUI_PUSHED, result);
	}

	private void cbClientDeleted(int result) {
		if (result != BluetoothFtp.BT_FTP_RSP_SUCCESS) {
			mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
		} else {
			mClientState = BluetoothFtp.BT_FTPC_STATE_TOBROWSE;
		}
		postClientEvent(BluetoothFtp.BT_FTPCUI_FILE_DELETED, result);
	}

	private void cbClientCreateFolderFailed() {
		// This is for failed creation.
		mClientState = BluetoothFtp.BT_FTPC_STATE_CONNECTED;
		postClientEvent(BluetoothFtp.BT_FTPCUI_FOLDER_CREATED, -1);
	}

/********************************************************************************************
 * Native Functions
 ********************************************************************************************/

	static {
		System.loadLibrary("extftp_jni");
	}

	/* Native functions */
	private native void cleanServiceNative();
	private native void forceClearServerNative();
	private native void forceClearClientNative();
	private native void stopListentoSocketNative();
	private native boolean initServiceNative();
	private native boolean prepareListentoSocketNative();
	private native boolean listentoSocketNative();

	/* Native functions for FTP Server */
	private native void serverEnableReqNative(String root);
	private native void serverDisableReqNative();
	private native int serverGetStatusNative();
	private native boolean setPermissionNative(boolean perm);
	private native boolean getPremissionNative();
	private native boolean setRootDirNative(String rootDir);

	private native void serverAuthorizeRspNative(boolean rsp);
	private native void serverDisconnectReqNative();
	private native boolean serverAbortReqNative();

	/* Native functions for FTP Client */
	private native boolean clientEnableReqNative();
	private native void clientDisableReqNative();
	private native void clientConnectReqNative(String addr);
	private native void clientDisconnectReqNative();
	private native void clientRefreshReqNative();
	private native void clientGoForwardReqNative(String path);
	private native void clientGoBackwardReqNative();
	private native void clientGoToRootReqNative();
	private native void clientPullReqNative(String path, String name, String rename);
	private native boolean clientPushReqNative(String name);
	private native void clientDeleteReqNative(String name);
	private native void clientCreateFolderReqNative(String name);
	private native boolean clientAbortReqNative();
	private native void clientCancelConnectNative();
}
