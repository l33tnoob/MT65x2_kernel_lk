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
import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.FolderContent;
import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.TransferringFile;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ListActivity;
import android.bluetooth.BluetoothFtp;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.text.Editable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.SimpleCursorAdapter;
import android.widget.SimpleCursorAdapter.ViewBinder;
import android.widget.TextView;
import android.widget.Toast;

public class BluetoothFtpClient extends ListActivity
	implements XMLParsingThread.ParsingDoneListener, ViewBinder, OnItemClickListener, OnItemLongClickListener {

	private static final String TAG = "BluetoothFtpClient";

	private static final String DEFAULT_ROOT	= BluetoothFtpService.DEFAULT_ROOT,
								PATH_ROOT		= "__ftpc_path_root__",
								PATH_PARENT		= "__ftpc_path_parent__";

	/* Key strings for retrieving bundled data */
	private static final String KEY_CUR_PATH	= "current_path",
								KEY_CUR_ITEM	= "current_item",
								KEY_DIALOG_ARGS	= "dialog_arguments",
								KEY_TITLE		= "title",
								KEY_MESSAGE		= "message";

	/* For broacasted feedbacks from LocalBrowser and SeveralMarker.
	 * Because onActivityResult() won't be invoked untill the activity goes back to the foreground,
	 * the broadcasted intent is used as a work-around to start the transfer even the activity is 
	 * still in the backgroud.
	 */
	protected static final String ACTION_PULL	= "com.mediatek.bluetooth.ftp.client.ACTION_PULL",
								  ACTION_PUSH	= "com.mediatek.bluetooth.ftp.client.ACTION_PUSH",
								  ACTION_ERROR	= "com.mediatek.bluetooth.ftp.client.ACTION_ERROR";

	private static final int BT_FTP_RSP_SUCCESS	= BluetoothFtp.BT_FTP_RSP_SUCCESS;

	/* Constants for internal message ids */
	public static final int CLIENT_BASE			= 2000,
							CLIENT_DATA_READY	= CLIENT_BASE + 1,
							CLIENT_START_PULL	= CLIENT_BASE + 2, 
							CLIENT_START_PUSH	= CLIENT_BASE + 3,
							CLIENT_ERROR		= CLIENT_BASE + 4; 

	/* Operation codes for convinence */
	private static final int CLIENT_OP_BASE		= CLIENT_BASE + 10,
						     OP_REGISTER_CB		= CLIENT_OP_BASE + 1,
						     OP_UNREGISTER_CB	= CLIENT_OP_BASE + 2,
						     OP_CONNECT			= CLIENT_OP_BASE + 3,
						     OP_DISCONNECT		= CLIENT_OP_BASE + 4,
						     OP_REFRESH			= CLIENT_OP_BASE + 5,
						     OP_GOFORWARD		= CLIENT_OP_BASE + 6,
						     OP_GOBACKWARD		= CLIENT_OP_BASE + 7,
						     OP_GOTOROOT		= CLIENT_OP_BASE + 8,
						     OP_CREATE_FOLDER	= CLIENT_OP_BASE + 9,
						     OP_START_PULL		= CLIENT_OP_BASE + 10,
						     OP_START_PUSH		= CLIENT_OP_BASE + 11,
						     OP_DELETE			= CLIENT_OP_BASE + 12,
						     OP_ABORT			= CLIENT_OP_BASE + 13;

	/* Constants for option ids */
	private static final int CLIENT_MENU_BASE	= CLIENT_BASE + 50,
						     MENU_SEND_FILES	= CLIENT_MENU_BASE + 1,
						     MENU_CREATE_FOLDER	= CLIENT_MENU_BASE + 2,
						     MENU_REFRESH		= CLIENT_MENU_BASE + 3,
						     MENU_GOTO_ROOT		= CLIENT_MENU_BASE + 4,
						     MENU_MARK_SEVERAL	= CLIENT_MENU_BASE + 5,
						     MENU_EXIT			= CLIENT_MENU_BASE + 6;

	private static final int CLIENT_DIALOG_BASE = CLIENT_BASE + 60,
						     DIALOG_FILE_OP		= CLIENT_DIALOG_BASE + 1,
						     DIALOG_FOLDER_OP	= CLIENT_DIALOG_BASE + 2,
						     DIALOG_FOLDER_NEW	= CLIENT_DIALOG_BASE + 3,
						     DIALOG_CNF_DELETE	= CLIENT_DIALOG_BASE + 4,
						     DIALOG_CNF_EXIT	= CLIENT_DIALOG_BASE + 5,
						     DIALOG_ALERT		= CLIENT_DIALOG_BASE + 6,
						     DIALOG_ALERT_EX	= CLIENT_DIALOG_BASE + 7;

	/* For working around on the case attempting to show a dialog within an activity which
	 * is in the background.
	 */
	private int mPendingDialogId = CLIENT_DIALOG_BASE;

	/* Flag for transferring multiple files */
	private boolean mMultipleTransfer = false;

	/* Current browsing folder path */
	private String mCurrentPath = DEFAULT_ROOT;

	/* Current selected item name */
	private String mCurrentItem = null;

	/* Current path view */
	private TextView mCurrentPathView = null;

	/* Input edittext view */
	private EditText mNameEditText = null;

	/* The name for creating a new folder */
	private String mNameToCreate = null;

	/* Arguments for alert dialogs */
	private Bundle mDialogArgs = null;

	/* The showing progress dialog */
	private ProgressDialog mProgressDialog;

	/* Kept ContentResolver instance for convience */
	private ContentResolver mResolver;

	/* ListAdapter for binding folder content and UI entries. */
	private SimpleCursorAdapter mListAdapter;

	/* The thread that parses the pulled XML file and stores the result into ContentProvider */
	private static XMLParsingThread mThread = null;

	/* Cursor points to current folder content */
	private Cursor mFolderContentCursor = null;

	/* Cursor points to transferring queue */
	private Cursor mTransferringCursor = null;

	/* UI event handler */
	private Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {
			String title = null, message = null;
			// Log.d(TAG, "[BT][FTP] Handle event: " + msg.what + ", arg: " + msg.arg1);

			switch (msg.what) {
				case BluetoothFtp.BT_FTPCUI_CONNECTED:
					execOperation(OP_REFRESH, null);
					break;

				case BluetoothFtp.BT_FTPCUI_DISCONNECTED:
					switch (msg.arg1) {
						case BluetoothFtp.BT_FTPC_CONNECTION_FAILED:
							showTextToast(R.string.bluetooth_ftp_client_connection_failed);
							break;
						case BluetoothFtp.BT_FTPC_UNEXPECTED:
							showTextToast(R.string.bluetooth_ftp_client_disconnected);
							break;
						default:
							break;
					}

					// For ALPS00131444, Dismiss the progress dialog in advance to avoid some
					//   black blink on screen.
					dismissProgressDialog();
					finish();
					break;

				case BluetoothFtp.BT_FTPCUI_SETPATHED:
					if (msg.arg1 == BT_FTP_RSP_SUCCESS) {
						execOperation(OP_REFRESH, null);
					} else {
						String path = null;
						boolean showEx = false;

						if (mCurrentItem != null) {
							if (mCurrentItem.equals(PATH_ROOT)) {
								path = getString(R.string.bluetooth_ftp_client_root_folder);
								showEx = true;
							} else if (mCurrentItem.equals(PATH_PARENT)) {
								path = getString(R.string.bluetooth_ftp_client_parent_folder);
								showEx = true;
							} else {
								path = mCurrentItem;
							}
						}

						title = getString(R.string.bluetooth_ftp_client_set_path_failed_title);
						message = String.format(
									getString(R.string.bluetooth_ftp_client_set_path_failed_message),
									path);

						if (showEx) {
							showAlertExDialog(title, message);
						} else {
							showAlertDialog(title, message);
						}
					}
					break;

				case BluetoothFtp.BT_FTPCUI_BROWSED:
					parseFolderContent();
					break;

				case BluetoothFtp.BT_FTPCUI_PUSHING:
				case BluetoothFtp.BT_FTPCUI_PULLING:
				{
					Bundle data = msg.getData();
					long p, t;
					if (data != null) {
						p = data.getLong(TransferringFile.PROGRESS, 0);
						t = data.getLong(TransferringFile.TOTAL, 0);
					} else {
						Log.e(TAG, "[BT][FTP] " + msg.what + " with null data");
						break;
					}

					if (!mMultipleTransfer && mProgressDialog != null) {
						if (t > 0) {
							mProgressDialog.setProgressMode(ProgressDialog.MODE_SINGLE_PERCENTAGE);
							mProgressDialog.setIndeterminate(false);
							mProgressDialog.setProgress(Utils.getPercentage(p, t));
						} else {
							mProgressDialog.setIndeterminate(true);
							mProgressDialog.setProgress(p);
						}
					}
				}
				break;

				case BluetoothFtp.BT_FTPCUI_PUSHED:
				case BluetoothFtp.BT_FTPCUI_PULLED:
					updateOrDismissTransferringDialog(msg.what, msg.arg1);
					break;

				case BluetoothFtp.BT_FTPCUI_FILE_DELETED:
					if (msg.arg1 == BT_FTP_RSP_SUCCESS) {
						execOperation(OP_REFRESH, null);
					} else {
						title = getString(R.string.bluetooth_ftp_client_delete_failed_title);
						message = String.format(
									getString(R.string.bluetooth_ftp_client_delete_failed_message),
									mCurrentItem);
						showAlertDialog(title, message);
					}
					break;

				case BluetoothFtp.BT_FTPCUI_FOLDER_CREATED:
					// This case is for failed creation.
					title = getString(R.string.bluetooth_ftp_client_new_folder_failed_title);
					message = String.format(
								getString(R.string.bluetooth_ftp_client_new_folder_failed_message),
								mNameToCreate);
					showAlertDialog(title, message);
					break;

				case CLIENT_START_PULL:
					// Log.d(TAG, "[BT][FTP] CLIENT_START_PULL");
					execOperation(OP_START_PULL, null);
					break;

				case CLIENT_START_PUSH:
					// Log.d(TAG, "[BT][FTP] CLIENT_START_PUSH");
					execOperation(OP_START_PUSH, null);
					break;

				case CLIENT_DATA_READY:
					// Log.d(TAG, "[BT][FTP] Receive Parsing Done.");
					mThread = null;
					if (msg.arg1 == 1) {
						/* arg1: 1 means parsing failure */
						showTextToast(R.string.bluetooth_ftp_client_browse_failed);
					}

					updateUI();
					break;

				case CLIENT_ERROR:
					Log.w(TAG, "[BT][FTP] Some error occurred. We need to disconnect and finish.");
					execOperation(OP_DISCONNECT, null);
					break;

				default:
					break;
			}
		}
	};

	/* FTP Client binder interface to BluetoothFtpService */
	private IBluetoothFtpClient mFtpClient;

	/* FTP Client binder callback interface from BluetoothFtpService */
	private IBluetoothFtpClientCallback mFtpClientCallback = 
				new IBluetoothFtpClientCallback.Stub() {

		public void postEvent(int event, int param) {
			Message msg = Message.obtain();
			msg.what = event;
			msg.arg1 = param;
			mHandler.sendMessage(msg);
		}

		public void postEventWithData(int event, Bundle data) {
			Message msg = Message.obtain();
			msg.what = event;
			msg.setData(data);
			mHandler.sendMessage(msg);
		}
	};

	private ServiceConnection mFtpClientConn = new ServiceConnection() {

		public void onServiceConnected(ComponentName className, IBinder service) {
			int state = -1;

			mFtpClient = IBluetoothFtpClient.Stub.asInterface(service);
			if (mFtpClient != null) {
				try {
					mFtpClient.registerCallback(mFtpClientCallback);
					state = mFtpClient.getState();
					mCurrentPath = mFtpClient.getCurrentPath();
					Log.d(TAG, "[BT][FTP] Client state: " + state);

				} catch (RemoteException e) {
					Log.e(TAG, "[BT][FTP] Exception occurred when registerCallback(), " + e);
				}
			} else {
				Log.e(TAG, "[BT][FTP] onServiceConnected(), mFtpClient is null");
			}

			// When connected with FTP service, check state for different launch modes.
			switch(state) {
				case BluetoothFtp.BT_FTPC_STATE_ACTIVE:
					Log.d(TAG, "[BT][FTP] Enable and connect to FTP server.");
					execOperation(OP_CONNECT, null);
					break;

				case BluetoothFtp.BT_FTPC_STATE_AUTHORIZING:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Authorizing.");
					showProgressDialog(OP_CONNECT);
					break;

				case BluetoothFtp.BT_FTPC_STATE_CONNECTED:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Connected.");
					if (mThread != null) {
						showProgressDialog(OP_REFRESH);
						if (mThread.isDone(BluetoothFtpClient.this)) {
							mThread = null;
							updateUI();
						}
					} else {
						updateUI();
					}
					break;

				case BluetoothFtp.BT_FTPC_STATE_SENDING:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Sending.");
					updateUI();
					prepareTransferringDialog(OP_START_PUSH, true);
					break;

				case BluetoothFtp.BT_FTPC_STATE_RECEIVING:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Receiving.");
					updateUI();
					prepareTransferringDialog(OP_START_PULL, true);
					break;

				case BluetoothFtp.BT_FTPC_STATE_TOBROWSE:
					Log.d(TAG, "[BT][FTP] Connected with FTP Service, To refresh");
					showProgressDialog(OP_REFRESH);
					execOperation(OP_REFRESH, null);
					break;

				case BluetoothFtp.BT_FTPC_STATE_BROWSING:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Refreshing.");
					showProgressDialog(OP_REFRESH);
					break;

				case BluetoothFtp.BT_FTPC_STATE_BROWSED:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Start parsing.");
					showProgressDialog(OP_REFRESH);
					parseFolderContent();
					break;

				case BluetoothFtp.BT_FTPC_STATE_ABORTING:
					Log.d(TAG, "[BT][FTP] Connected with FTP service, Aborting.");
					updateUI();
					showProgressDialog(OP_ABORT);
					break;

				default:
					break;
			}
		}

		public void onServiceDisconnected(ComponentName className) {
			Log.e(TAG, "[BT][FTP] FTP Service disconnected unexpectedly. Finish this activity.");

			mFtpClient = null;
			finish();
		}
	};

	/* IntentFilter for receiving broadcasted feedback from LocalBrowser and SeveralMarker */
	private IntentFilter mFilter = new IntentFilter(); 

	/* BroadcastReceiver for receiving broadcasted feedback from LocalBrowser and SeveralMarker */
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String act = intent.getAction();
			Message msg = Message.obtain();

			if (ACTION_PULL.equals(act)) {
				msg.what = CLIENT_START_PULL;
			} else if (ACTION_PUSH.equals(act)) {
				msg.what = CLIENT_START_PUSH;
			} else if (ACTION_ERROR.equals(act)) {
				msg.what = CLIENT_ERROR;
			} else {
				Log.w(TAG, "[BT][FTP] Invalid action: " + act);
				return;
			}
			mHandler.sendMessage(msg);
		}
	};

	private OnClickListener mFileOpListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int pos) {

			if (pos == 0) {
				// Get the file.
				//	Step 1. Place a getting recored in content provider.
				//	Step 2. Request FTP service to start the getting procedure.

				Log.d(TAG, "[BT][FTP] Get the file: " + mCurrentItem);
				ContentValues values = new ContentValues();
				Uri uri = TransferringFile.CONTENT_URI;

				try {
					if (mCurrentItem != null && mCurrentItem.length() > 0) {
						// Clear the transferring queue
						mResolver.delete(uri, null, null);

						values.put(TransferringFile.NAME, mCurrentItem);
						values.put(TransferringFile.STATUS, TransferringFile.STATUS_WAITING);
						values.put(TransferringFile.DIRECTION, TransferringFile.DIRECTION_PULL);
						mResolver.insert(TransferringFile.CONTENT_URI, values);
						execOperation(OP_START_PULL, null);

					} else {
						Message msg = Message.obtain();
						msg.what = BluetoothFtp.BT_FTPCUI_PULLED;
						msg.arg1 = 1;
						mHandler.sendMessage(msg);
					}

				} catch (Exception ex) {
					Log.e(TAG, "[BT][FTP] mFileOpListener.onClick(), Exception: " + ex);
					execOperation(OP_DISCONNECT, null);
				}

			} else if (pos == 1) {
				// Delete the file
				Log.d(TAG, "[BT][FTP] Delete the file: " + mCurrentItem);
				showConfirmDeleteDialog(mCurrentItem);
			}

			// mCurrentItem = null;
			dialog.dismiss();
		}
	};

	private OnClickListener mFolderOpListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int pos) {
			if (pos == 0) {
				// Open the folder
				Log.d(TAG, "[BT][FTP] Open the folder: " + mCurrentItem);
				execOperation(OP_GOFORWARD, mCurrentItem);

			} else if (pos == 1) {
				// Delete the folder.
				//	Case 1. An empty folder. Just delete it as a file.
				//	Case 2. A non-empty folder. Need another thread to take responce of it. [No support]

				Log.d(TAG, "[BT][FTP] Delete the folder: " + mCurrentItem);
				// execOperation(OP_DELETE, mCurrentItem);
				showConfirmDeleteDialog(mCurrentItem);
			}

			// mCurrentItem = null;
			dialog.dismiss();
		}
	};

	private OnClickListener mNewFolderListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			EditText edit = null;
			Editable editable = null;

			switch (which) {
				case DialogInterface.BUTTON_POSITIVE:
					editable = mNameEditText.getText();

					if (editable != null) {
						mNameToCreate = editable.toString();

						if (mNameToCreate != null) {
							mNameToCreate = mNameToCreate.trim();
						}

						if (mNameToCreate != null && mNameToCreate.length() > 0) {
							// For issue id: ALPS00220867, a test ask not clear the editable when OK is pressed.
							// editable.clear();
							execOperation(OP_CREATE_FOLDER, mNameToCreate);
						} else {
							showTextToast(R.string.bluetooth_ftp_client_new_folder_message);
						}
					}

					break;
				case DialogInterface.BUTTON_NEGATIVE:
					break;
				default:
					break;
			}
			dialog.dismiss();
		}
	};

	private OnClickListener mConfirmDeleteListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			if (which == DialogInterface.BUTTON_POSITIVE) {
				if (mCurrentItem != null && mCurrentItem.length() > 0) {
					execOperation(OP_DELETE, mCurrentItem);
				} else {
					Message msg = Message.obtain();
					msg.what = BluetoothFtp.BT_FTPCUI_FILE_DELETED;
					msg.arg1 = 1;
					mHandler.sendMessage(msg);
				}
			}
		}
    };

	private OnClickListener mConfirmExitListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			if (which == DialogInterface.BUTTON_POSITIVE) {
				execOperation(OP_DISCONNECT, null);
			}
		}
    };

	private OnCancelListener mCancelConnectListener = new OnCancelListener() {
		public void onCancel(DialogInterface dialog) {
			execOperation(OP_DISCONNECT, null);
		}
	};

	private OnClickListener mAbortClickListener = new OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			execOperation(OP_ABORT, null);
		}
	};

	private OnCancelListener mAbortCancelListener = new OnCancelListener() {
		public void onCancel(DialogInterface dialog) {
			execOperation(OP_ABORT, null);
		}
	};

	private View.OnClickListener mRequestFocusListener = new View.OnClickListener() {
		public void onClick(View v) {
			v.requestFocus();
		}
	};

	private OnDismissListener mPendingDismissListener = new OnDismissListener() {
		public void onDismiss(DialogInterface dialog) {
			mPendingDialogId = CLIENT_DIALOG_BASE;
		}
	};

/********************************************************************************************
 * Life-cycle Callback Functions of FTP Client
 ********************************************************************************************/

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Log.d(TAG, "[BT][FTP] onCreate()");

		setTitle(R.string.bluetooth_ftp_client);
		mResolver = getContentResolver();

		setContentView(R.layout.bt_ftp_folder_listing_view);

		if (savedInstanceState != null) {
			mCurrentItem = savedInstanceState.getString(KEY_CUR_ITEM);
			mDialogArgs = savedInstanceState.getBundle(KEY_DIALOG_ARGS);
		}

		if (!bindService(new Intent(IBluetoothFtpClient.class.getName()), mFtpClientConn, 0)) {
			Log.e(TAG, "[BT][FTP] Failed to bind service");
		}

		mCurrentPathView = (TextView) findViewById(R.id.current_path);
		if (mCurrentPathView != null) {
			mCurrentPathView.setOnClickListener(mRequestFocusListener);
		} else {
			Log.e(TAG, "[BT][FTP] onCreate(): mCurrentPathView is null");
			finish();
		}

		String[] from = new String[] {FolderContent.NAME, FolderContent.TYPE, FolderContent.SIZE};

		int[] to = new int[] {R.id.entry_name, R.id.entry_type, R.id.entry_info};

		/* Cursor parameter is set as null till the data is ready. */
		mListAdapter = new SimpleCursorAdapter(this, R.layout.bt_ftp_folder_listing_entry_view, null, from, to);
		mListAdapter.setViewBinder(this);
		getListView().setOnItemClickListener(this);
		getListView().setOnItemLongClickListener(this);

		mFilter.addAction(ACTION_PULL);
		mFilter.addAction(ACTION_PUSH);
		registerReceiver(mReceiver, mFilter);
	}

	@Override
	protected void onStart() {
		super.onStart();
		// Log.d(TAG, "[BT][FTP] onStart()");
	}

	@Override
	protected void onResume() {
		super.onResume();
		// Log.d(TAG, "[BT][FTP] onResume()");

		// A workaround for not-showing dialog
		// If Dialog.show() is called when the activity is in the background, there will be some
		// displaying problem. This work-around force the dialog being redrawed when resumed.
		int pending = mPendingDialogId;
		if (pending != CLIENT_DIALOG_BASE) {
			dismissDialog(pending);
			showDialog(pending);
			return;
		} else if (mProgressDialog != null) {
			mProgressDialog.hide();
			mProgressDialog.show();
			return;
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		// Log.d(TAG, "[BT][FTP] onPause()");
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);

		if (mCurrentItem != null) {
			outState.putString(KEY_CUR_ITEM, mCurrentItem);
		}

		outState.putBundle(KEY_DIALOG_ARGS, mDialogArgs);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		Log.d(TAG, "[BT][FTP] onDestroy()");

		unregisterReceiver(mReceiver);
		dismissProgressDialog();

		finishActivity(MENU_MARK_SEVERAL);
		finishActivity(MENU_SEND_FILES);

		if (mFolderContentCursor != null) {
			mFolderContentCursor.close();
			mFolderContentCursor = null;
		}

		if (mTransferringCursor != null) {
			mTransferringCursor.close();
			mTransferringCursor = null;
		}

		if (mThread != null) {
			mThread.removeListener();
		}

		execOperation(OP_UNREGISTER_CB, null);

		unbindService(mFtpClientConn);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	}

	@Override
	public void onBackPressed() {

		if (mCurrentPath.equals(DEFAULT_ROOT)) {
			showDialog(DIALOG_CNF_EXIT);
		} else {
			mCurrentItem = PATH_PARENT;
			execOperation(OP_GOBACKWARD, null);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(0, MENU_SEND_FILES, 0, R.string.bluetooth_ftp_client_menu_send_files)
			.setIcon(R.drawable.btftp_ic_menu_send_files);
		menu.add(0, MENU_CREATE_FOLDER, 0, R.string.bluetooth_ftp_client_menu_create_folder)
			.setIcon(R.drawable.btftp_ic_menu_create_folder);
		menu.add(0, MENU_REFRESH, 0, R.string.bluetooth_ftp_client_menu_refresh)
			.setIcon(R.drawable.btftp_ic_menu_refresh);
		menu.add(0, MENU_GOTO_ROOT, 0, R.string.bluetooth_ftp_client_menu_goto_root)
			.setIcon(R.drawable.btftp_ic_menu_root);
		menu.add(0, MENU_MARK_SEVERAL, 0, R.string.bluetooth_ftp_client_menu_mark_several)
			.setIcon(R.drawable.btftp_ic_menu_mark_several);
		menu.add(0, MENU_EXIT, 0, R.string.bluetooth_ftp_client_menu_exit)
			.setIcon(R.drawable.btftp_ic_menu_exit);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// menu.setGroupEnabled(0, mProgressDialog == null);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		switch(item.getItemId()) {
			case MENU_SEND_FILES: {
				Intent intent = new Intent();
				intent.setClassName(getPackageName(), BluetoothFtpLocalBrowser.class.getName());
				startActivityForResult(intent, MENU_SEND_FILES);
				// startActivity(intent);
				break;
			}

			case MENU_CREATE_FOLDER:
				showDialog(DIALOG_FOLDER_NEW);
				break;
			case MENU_REFRESH:
				execOperation(OP_REFRESH, null);
				break;
			case MENU_GOTO_ROOT:
				if (mCurrentPath.equals(DEFAULT_ROOT)) {
					showTextToast(R.string.bluetooth_ftp_client_current_is_root);
				} else {
					mCurrentItem = PATH_ROOT;
					execOperation(OP_GOTOROOT, null);
				}
				break;

			case MENU_MARK_SEVERAL: {
				Intent intent = new Intent();
				intent.setClassName(getPackageName(), BluetoothFtpSeveralMarker.class.getName());
				intent.putExtra(TransferringFile.DIRECTION, TransferringFile.DIRECTION_PULL);
				startActivityForResult(intent, MENU_MARK_SEVERAL);
				// startActivity(intent);
				break;
			}

			case MENU_EXIT:
				execOperation(OP_DISCONNECT, null);
				break;
			default:
				break;
		}

		return true;
	}

	@Override
	protected Dialog onCreateDialog(int id) {
		AlertDialog dialog = null;
		Builder builder  = new Builder(this);

		switch (id) {
			case DIALOG_FILE_OP:
				builder.setNegativeButton(R.string.bluetooth_ftp_cancel, null)
					   .setTitle("Null")
					   .setItems(R.array.bluetooth_ftp_client_file_operations, mFileOpListener);
				dialog = builder.create();
				break;

			case DIALOG_FOLDER_OP:
				builder.setNegativeButton(R.string.bluetooth_ftp_cancel, null)
					   .setTitle("NULL")
					   .setItems(R.array.bluetooth_ftp_client_folder_operations, mFolderOpListener);
				dialog = builder.create();
				break;

			case DIALOG_FOLDER_NEW: {
				LayoutInflater inflater = getLayoutInflater();
				View inputView = inflater.inflate(R.layout.bt_ftp_new_folder_dialog, null);

				// Keep the reference to input-name EditText view
				mNameEditText = (EditText) inputView.findViewById(R.id.new_folder_name_edit);

				builder.setTitle(R.string.bluetooth_ftp_client_new_folder_title)
					   .setView(inputView)
					   .setPositiveButton(R.string.bluetooth_ftp_ok, mNewFolderListener)
					   .setNegativeButton(R.string.bluetooth_ftp_cancel, mNewFolderListener);
				dialog = builder.create();
				break;
			}

			case DIALOG_CNF_DELETE:
				builder.setIcon(android.R.drawable.ic_dialog_alert)
					   .setTitle(R.string.bluetooth_ftp_client_confirm_delete_title)
					   .setMessage("NULL")
					   .setPositiveButton(R.string.bluetooth_ftp_ok, mConfirmDeleteListener)
					   .setNegativeButton(R.string.bluetooth_ftp_cancel, mConfirmDeleteListener);
				dialog = builder.create();
				break;

			case DIALOG_CNF_EXIT:
				builder.setIcon(android.R.drawable.ic_dialog_alert)
					   .setTitle(R.string.bluetooth_ftp_client_confirm_exit_title)
					   .setMessage(R.string.bluetooth_ftp_client_confirm_exit_message)
					   .setPositiveButton(R.string.bluetooth_ftp_ok, mConfirmExitListener)
					   .setNegativeButton(R.string.bluetooth_ftp_cancel, mConfirmExitListener);
				dialog = builder.create();
				break;

			case DIALOG_ALERT:
				builder.setIcon(android.R.drawable.ic_dialog_alert)
					   .setTitle("NULL")
					   .setMessage("NULL")
					   .setPositiveButton(R.string.bluetooth_ftp_ok, null);
				dialog = builder.create();
				dialog.setOnDismissListener(mPendingDismissListener);
				break;

			case DIALOG_ALERT_EX:
				builder.setIcon(android.R.drawable.ic_dialog_alert)
					   .setTitle("NULL")
					   .setMessage("NULL")
					   .setOnCancelListener(mCancelConnectListener)
					   .setPositiveButton(R.string.bluetooth_ftp_ok, mConfirmExitListener);
				dialog = builder.create();
				dialog.setOnDismissListener(mPendingDismissListener);
				break;

			default:
				Log.w(TAG, "[BT][FTP] Invalid dialog id: " + id);
				break;
		}

		return dialog;
	} 

	@Override
	protected void onPrepareDialog(int id, Dialog dialog) {
		Bundle args = mDialogArgs;
		switch (id) {
			case DIALOG_FILE_OP:
			case DIALOG_FOLDER_OP:
				dialog.setTitle(args.getString(KEY_TITLE));
				break;

			case DIALOG_ALERT:
			case DIALOG_ALERT_EX:
				if (dialog instanceof AlertDialog) {
					AlertDialog alert = (AlertDialog) dialog;
					alert.setTitle(args.getString(KEY_TITLE));
					alert.setMessage(args.getString(KEY_MESSAGE));
				}
				break;

			// For issue id: ALPS00220867, a test ask not clear the editable when OK is pressed.
			case DIALOG_FOLDER_NEW: {
				Editable editable = mNameEditText.getText();
				if (editable != null) {
					editable.clear();
				}
				break;
			}

			case DIALOG_CNF_DELETE:
				if (dialog instanceof AlertDialog) {
					AlertDialog cnf_dialog = (AlertDialog) dialog;
					cnf_dialog.setMessage(args.getString(KEY_MESSAGE));
				}
				break;

			default:
				break;
		}
    }

/********************************************************************************************
 * Operation Functions
 ********************************************************************************************/
	/* Centralized function for requesting FTP service */
	private void execOperation(int op, String arg) {
		// Log.d(TAG, "[BT][FTP] Enter execOperation(), opt: " + op + ", arg: " + arg);

		if (mFtpClient == null) {
			Log.e(TAG, "[BT][FTP] mFtpClient is null, opt: " + op + ", arg: " + arg);
			finish();
			return;
		}

		try {
			switch(op) {
				case OP_REGISTER_CB:
					mFtpClient.registerCallback(mFtpClientCallback);
					break;
				case OP_UNREGISTER_CB:
					mFtpClient.unregisterCallback(mFtpClientCallback);
					break;
				case OP_CONNECT:
					showProgressDialog(op);
					mFtpClient.connect();
					break;
				case OP_DISCONNECT:
					mFtpClient.disconnect();
					finish();
					break;
				case OP_REFRESH:
					showProgressDialog(op);
					mFtpClient.refresh();
					break;
				case OP_GOFORWARD:
					showProgressDialog(op);
					mFtpClient.goForward(arg);
					break;
				case OP_GOBACKWARD:
					showProgressDialog(op);
					mFtpClient.goBackward();
					break;
				case OP_GOTOROOT:
					showProgressDialog(op);
					mFtpClient.goToRoot();
					break;
				case OP_CREATE_FOLDER:
					showProgressDialog(op);
					mFtpClient.createFolder(arg);
					break;
				case OP_START_PULL:
					prepareTransferringDialog(op, false);
					mFtpClient.startPull();
					break;
				case OP_START_PUSH:
					prepareTransferringDialog(op, false);
					mFtpClient.startPush();
					break;
				case OP_DELETE:
					showProgressDialog(op);
					mFtpClient.delete(arg);
					break;
				case OP_ABORT:
					Log.d(TAG, "[BT][FTP] OP_ABORT");
					if (mFtpClient.abort()) {
						dismissProgressDialog();
						showProgressDialog(op);
					}
					break;
				default:
					Log.w(TAG, "[BT][FTP] Unknown op: " + op + ", arg: " + arg);
					break;
			}

		} catch(RemoteException re) {
			Log.e(TAG, "[BT][FTP] Exception: " + re);
			finish();
		}
	}

/********************************************************************************************
 * UI Control Functions
 ********************************************************************************************/

	/* Update UI: mCurrentPathView and folder content */
	private void updateUI() {
		// Log.d(TAG, "[BT][FTP] Enter updateUI()");

		if (mFtpClient == null) {
			Log.e(TAG, "[BT][FTP] updateUI(), mFtpClient is null");
			finish();
			return;
		}

		try {
			mCurrentPath = mFtpClient.getCurrentPath();
			mCurrentPathView.setText(mCurrentPath);
		} catch (RemoteException re) {
			Log.e(TAG, "[BT][FTP] Exception: " + re);
			finish();
		}

		// Release previous cursor instance
		if (mFolderContentCursor != null) {
			mFolderContentCursor.close();
		}

		// Grab the data from content provider and show them on screen.
		try {
			mFolderContentCursor = mResolver.query(FolderContent.SERVER_URI, null, null, null, null);
		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] updateUI(), Exception: " + ex);
			execOperation(OP_DISCONNECT, null);
		}

		mListAdapter.changeCursor(mFolderContentCursor);

		// Invoking setListAdapter() will reset the list.
		setListAdapter(mListAdapter);
		// getListView().requestFocus();
		mCurrentPathView.requestFocus();

		// Dismiss the progress dialog
		dismissProgressDialog();
	}

	/* Prepare the transferring-progress dialog */
	private synchronized void prepareTransferringDialog(int op, boolean restore) {
		ProgressDialog dialog = null;
		boolean indet = true;
		int status_idx, status, count;
		String title, msg, name;

		if (mProgressDialog != null) {
			Log.e(TAG, "[BT][FTP] prepareTransferringDialog(): mProgressDialog is not null.");
			return;
		}

		if (mTransferringCursor != null) {
			mTransferringCursor.close();
		}

		try {
			mTransferringCursor = mResolver.query(TransferringFile.CONTENT_URI, null, null, null, null);
			if (mTransferringCursor == null) {
				Log.e(TAG, "[BT][FTP] Transferring Queue is empty.");
				return;
			} else {
				count = mTransferringCursor.getCount();
				if (count == 0) {
					Log.e(TAG, "[BT][FTP] Transferring Queeu is empty.");
					return;
				}
			}
		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] prepareTransferringDialog(), Exception: " + ex);
			execOperation(OP_DISCONNECT, null);
			return;
		}

		mTransferringCursor.moveToFirst();
		status_idx = mTransferringCursor.getColumnIndex(TransferringFile.STATUS);
		status = mTransferringCursor.getInt(status_idx);

		dialog = new ProgressDialog(this);
		dialog.setTitle(getString(R.string.bluetooth_ftp_client_transferring));
		dialog.setButton(ProgressDialog.BUTTON_NEUTRAL,
				getString(R.string.bluetooth_ftp_cancel), mAbortClickListener);
		dialog.setOnCancelListener(mAbortCancelListener);

		if (count > 1) {
			mMultipleTransfer = true;
			dialog.setProgressMode(ProgressDialog.MODE_MULTIPLE);
			dialog.setMax(count);

			if (restore) {
				while(status != TransferringFile.STATUS_TRANSFERRING) {
					if (!mTransferringCursor.moveToNext()) {
						Log.w(TAG, "[BT][FTP] No file is transferring now. Don't show the dialog.");
						return;
					}
					status = mTransferringCursor.getInt(status_idx);
				}
				dialog.setProgress(mTransferringCursor.getPosition() + 1);

			} else {
				dialog.setProgress(1);
			}

		} else {
			mMultipleTransfer = false;
			dialog.setProgressMode(ProgressDialog.MODE_SINGLE_TRANSFERRED);

			if (restore) {
				long p = mTransferringCursor.getLong(mTransferringCursor.getColumnIndex(TransferringFile.PROGRESS));
				long t = mTransferringCursor.getLong(mTransferringCursor.getColumnIndex(TransferringFile.TOTAL));

				if (t > 0) {
					p = Utils.getPercentage(p, t);
					dialog.setProgressMode(ProgressDialog.MODE_SINGLE_PERCENTAGE);
					indet = false;
				} 
				dialog.setProgress(p);

			} else {
				dialog.setProgress(0);
			}
		}

		name = getCurrentTransferringName();

		if (op == OP_START_PULL) {
			msg = String.format(getString(R.string.bluetooth_ftp_client_getting), name);
		} else if (op == OP_START_PUSH) {
			msg = String.format(getString(R.string.bluetooth_ftp_client_sending), name);
		} else {
			Log.e(TAG, "[BT][FTP] Invalid Operation for Transferring.");
			return;
		}

		dialog.setIndeterminate(indet);
		dialog.setMessage(msg);

		mProgressDialog = dialog;
		mProgressDialog.show();
	}

	/* For process BT_FTPCUI_PULLED and BT_FTPCUI_PUSHED */
	private synchronized void updateOrDismissTransferringDialog(int msg_id, int rsp_code) {
		String dialog_msg = null, dialog_title = null, name = null;

		if (mProgressDialog == null) {
			return;
		}

		if (rsp_code != BT_FTP_RSP_SUCCESS) {
			dismissProgressDialog();
			dialog_title = getString(R.string.bluetooth_ftp_client_transfer_unfinished);

			// For ALPS00220811 and ALPS00220860, don't show the reason of unfinished.
			dialog_msg = dialog_title;
			showAlertDialog(dialog_title, dialog_msg);

			Log.d(TAG, "[BT][FTP] Transfer unfinished: " + getRspString(rsp_code));
			return;
		}

		if (mMultipleTransfer && mTransferringCursor.moveToNext()) {
			name = getCurrentTransferringName();

			if (msg_id == BluetoothFtp.BT_FTPCUI_PULLED) {
				dialog_msg = String.format(getString(R.string.bluetooth_ftp_client_getting), name);
			} else if (msg_id == BluetoothFtp.BT_FTPCUI_PUSHED){
				dialog_msg = String.format(getString(R.string.bluetooth_ftp_client_sending), name);
			} else {
				Log.e(TAG, "[BT][FTP] Invalid Message ID: " + msg_id);
			}

			mProgressDialog.setMessage(dialog_msg);
			mProgressDialog.setProgress(mTransferringCursor.getPosition() + 1);

		} else {
			dismissProgressDialog();
			showTextToast(R.string.bluetooth_ftp_client_transfer_finished);

			if (msg_id == BluetoothFtp.BT_FTPCUI_PULLED) {
				// request MediaScanner to add the pulled file.
			} else if (msg_id == BluetoothFtp.BT_FTPCUI_PUSHED) {
				execOperation(OP_REFRESH, null);
			}
		}
	}

	/* Show the progress dialog. There's only one dialog allowed to show at the same time. */
	private synchronized void showProgressDialog(int op) {
		if (mProgressDialog != null) {
			return;
		}

		String title = "", msg = "";
		boolean bIndet = false, bCancelable = false;
		OnCancelListener ocl = null;

		switch (op) {
			case OP_CONNECT:
				msg = getString(R.string.bluetooth_ftp_client_connecting);
				bIndet = true;
				bCancelable = true;
				ocl = mCancelConnectListener;
				break;

			case OP_REFRESH:
			case OP_GOFORWARD:
			case OP_GOBACKWARD:
			case OP_GOTOROOT:
			case OP_CREATE_FOLDER:
			case OP_DELETE:
				msg = getString(R.string.bluetooth_ftp_client_refreshing);
				bIndet = true;
				bCancelable = false;
				break;

			case OP_ABORT:
				msg = getString(R.string.bluetooth_ftp_client_cancelling);
				bIndet = true;
				bCancelable = false;
				break;

			default:
				return;
		}

		ProgressDialog dialog = new ProgressDialog(this);
		dialog.setTitle(title);
		dialog.setMessage(msg);
		dialog.setIndeterminate(bIndet);
		dialog.setCancelable(bCancelable);
		dialog.setCanceledOnTouchOutside(false);
		dialog.setOnCancelListener(ocl);

		mProgressDialog = dialog;
		mProgressDialog.show();
	}

	/* Dismiss the progress dialog */
	private synchronized void dismissProgressDialog() {
		if (mProgressDialog != null) {
			mProgressDialog.dismiss();
			mProgressDialog = null;
		}
	}

	/* Display the AlertDialog with given title and message */
	private void showAlertDialog(String title, String message) {
		mDialogArgs = new Bundle();
		mDialogArgs.putString(KEY_TITLE, title);
		mDialogArgs.putString(KEY_MESSAGE, message);

		dismissProgressDialog();
		showDialog(DIALOG_ALERT);
		mPendingDialogId = DIALOG_ALERT;
	}

	/* Display the AlertDialog and then exit FTP client. */
	private void showAlertExDialog(String title, String message) {
		mDialogArgs = new Bundle();
		mDialogArgs.putString(KEY_TITLE, title);
		mDialogArgs.putString(KEY_MESSAGE, message);

		dismissProgressDialog();
		showDialog(DIALOG_ALERT_EX);
		mPendingDialogId = DIALOG_ALERT_EX;
	}

	/* Show the operations dialog relative to a certain list item view. */
	private void showOpDialog(View view) {
		TextView nameView = (TextView) view.findViewById(R.id.entry_name);
		View typeView = view.findViewById(R.id.entry_type);

		if (nameView == null || typeView == null) {
			Log.e(TAG, "[BT][FTP] showOpDialog(), nameView or typeView is null");
			return;
		}

		String name = nameView.getText().toString();
		int type = ((Integer) typeView.getTag()).intValue();

		mCurrentItem = name;
		mDialogArgs = new Bundle();
		mDialogArgs.putString(KEY_TITLE, name);

		switch(type) {
			case FolderContent.TYPE_FOLDER:
				showDialog(DIALOG_FOLDER_OP);
				break;

			case FolderContent.TYPE_AUDIO:
			case FolderContent.TYPE_IMAGE:
			case FolderContent.TYPE_VIDEO:
			case FolderContent.TYPE_TEXT:
				showDialog(DIALOG_FILE_OP);
				break;

			default:
				Log.e(TAG, "[BT][FTP] Unknown Type");
				break;
		}
	}

	/* Show a dialog to confirm users in deleting a file or folder. */
	private void showConfirmDeleteDialog(String target) {
		String msg = String.format(
				getString(R.string.bluetooth_ftp_client_confirm_delete_message), target);

		mDialogArgs = new Bundle();
		mDialogArgs.putString(KEY_MESSAGE, msg);

		showDialog(DIALOG_CNF_DELETE);
	}

	/* Show a Toast with text message */
	private void showTextToast(int text_id) {
		Toast toast = Toast.makeText(this, text_id, Toast.LENGTH_SHORT);
		toast.show();
	}

/********************************************************************************************
 * Utility Functions
 ********************************************************************************************/
	/* Utility function: getCurrentTransferringName */
	private String getCurrentTransferringName() {
		String ret = null;
			if (mTransferringCursor != null) {
				int name_idx = mTransferringCursor.getColumnIndex(TransferringFile.NAME);
				ret = mTransferringCursor.getString(name_idx);
			}

		return ret;
	}

	private void parseFolderContent() {
		/* Start another thread to parse xml file. */
		mThread = new XMLParsingThread(BluetoothFtpService.XML_PATH,
				FolderContent.SERVER_URI, mResolver, BluetoothFtpClient.this);
		mThread.start();
	}

	public String getRspString(int code) {
		int tmp = code - 0x80;
		tmp = (tmp > 0) ? tmp : code;

		/*
		switch (tmp) {
			case OBEX.UNAUTHORIZED:
				return getString(R.string.bluetooth_ftp_unauthorized);
			case OBEX.FORBIDDEN:
				return getString(R.string.bluetooth_ftp_forbidden);
			case OBEX.NOT_FOUND:
				return getString(R.string.bluetooth_ftp_not_found);
			case OBEX.USER_CANCEL:
				return getString(R.string.bluetooth_ftp_user_canceled);
			default:
				return new Integer(tmp).toString();
		}
		*/
		return "Unknown";
	}

/********************************************************************************************
 * Interface Functions
 ********************************************************************************************/
	/* For implementing XMLParsingTherad.OnParsingDoneListener */
	public void onParsingDone(int result) {
		Message msg = Message.obtain();

		if (result > -1) {
			msg.what = CLIENT_DATA_READY;
			msg.arg1 = result;

		} else {
			msg.what = CLIENT_ERROR;
		}

		mHandler.sendMessage(msg);
	}

	/* For implementing SimpleCursorAdapter.ViewBinder */
	public boolean setViewValue(View view, Cursor cursor, int columnIndex) {
		final int id = view.getId();
		final int typeIndex = cursor.getColumnIndex(FolderContent.TYPE);
		final int type = cursor.getInt(typeIndex);
		final int dateIndex = cursor.getColumnIndex(FolderContent.MODIFIED_DATE);

		int sizeIndex = -1;
		String date = null;

		switch(id) {
			case R.id.entry_name:
				((TextView) view).setText(cursor.getString(columnIndex));
				break;

			case R.id.entry_type:
				// Reserver entry type in an integer field.
				view.setTag(Integer.valueOf(type));
				switch(type) {
					case FolderContent.TYPE_FOLDER:
						((ImageView) view).setImageResource(R.drawable.btftp_ic_folder);
						break;
					case FolderContent.TYPE_AUDIO:
						((ImageView) view).setImageResource(R.drawable.btftp_ic_audio);
						break;
					case FolderContent.TYPE_IMAGE:
						((ImageView) view).setImageResource(R.drawable.btftp_ic_image);
						break;
					case FolderContent.TYPE_VIDEO:
						((ImageView) view).setImageResource(R.drawable.btftp_ic_video);
						break;
					default:
						((ImageView) view).setImageResource(R.drawable.btftp_ic_text);
						break;
				}
				break;

			case R.id.entry_info:
				date = cursor.getString(dateIndex);
				sizeIndex = columnIndex;

				if (type == FolderContent.TYPE_FOLDER) {
					((EntryInfoView) view).setEntryInfo(-1, date);
				} else {
					((EntryInfoView) view).setEntryInfo(cursor.getLong(sizeIndex), date);
				}
				break;

			default:
				// Other cases, for LinearLayout instances. Simply skip.
				break;
		}

		return true;
	}

	/* For implementing AdapterView.OnItemClickListener */
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		showOpDialog(view);
	}

	/* For implementing AdapterView.OnItemLongClickListener */
	public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
		showOpDialog(view);
		// consume the callback.
		return true;
	}
}

class OBEX {
public static final int STATUS_FAILED			= 1,
						BAD_REQUEST				= 0x40,  /* Bad Request */
						UNAUTHORIZED			= 0x41,  /* Unauthorized */
						PAYMENT_REQUIRED		= 0x42,  /* Payment Required */
						FORBIDDEN				= 0x43,  /* Forbidden - operation is understood but refused */
						NOT_FOUND				= 0x44,  /* Not Found */
						METHOD_NOT_ALLOWED		= 0x45,  /* Method Not Allowed */
						NOT_ACCEPTABLE			= 0x46,  /* Not Acceptable */
						PROXY_AUTHEN_REQ	    = 0x47,  /* Proxy Authentication Required */
						REQUEST_TIME_OUT	    = 0x48,  /* Request Timed Out */
						CONFLICT				= 0x49,  /* Conflict */
						GONE					= 0x4a,  /* Gone */
						LENGTH_REQUIRED			= 0x4b,  /* Length Required */
						PRECONDITION_FAILED		= 0x4c,  /* Precondition Failed */
						REQ_ENTITY_TOO_LARGE	= 0x4d,  /* Requested entity is too large */
						REQ_URL_TOO_LARGE		= 0x4e,  /* Requested URL is too large */
						UNSUPPORT_MEDIA_TYPE    = 0x4f,  /* Unsupported Media Type */
						INTERNAL_SERVER_ERR		= 0x50,  /* Internal Server Error */
						NOT_IMPLEMENTED			= 0x51,  /* Not Implemented */
						BAD_GATEWAY				= 0x52,  /* Bad Gateway */
						SERVICE_UNAVAILABLE		= 0x53,  /* Service Unavailable */
						GATEWAY_TIMEOUT			= 0x54,  /* Gateway Timeout */
						HTTP_VER_NO_SUPPORT		= 0x55,  /* HTTP version not supported */

						USER_CANCEL				= 0x70, /* User cancel the pushing/pulling/connect */
						USER_UNKNOW				= 0x71; /* User cancel the pushing/pulling/connect */
}
