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
import com.mediatek.bluetooth.util.SystemUtils;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.SimpleCursorAdapter;
import android.widget.SimpleCursorAdapter.ViewBinder;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.Calendar;

/* A simple local file browser for choosing file(s) to send */
public class BluetoothFtpLocalBrowser extends ListActivity 
		implements ViewBinder, OnItemClickListener, GetLocalContentThread.ResultListener {

	private static final String TAG = "BluetoothFtpLocalBrowser";

	/* The default root for browsing local files */
	private static final String DEFAULT_ROOT	= "/mnt";

	private static final String KEY_CUR_PATH	= "current_path";

	private static final int DATA_READY	= 0,
							 ERROR		= 1;

	private static final int LOCAL_BROWSER_BASE = BluetoothFtpClient.CLIENT_BASE + 200;

	private static final int MENU_BASE			= LOCAL_BROWSER_BASE + 10,
							 MENU_GOTO_ROOT		= MENU_BASE + 1,
							 MENU_MARK_SEVERAL	= MENU_BASE + 2,
							 MENU_EXIT			= MENU_BASE + 3;

	private static GetLocalContentThread mThread = null;

	private String mRoot, mCurrentPath;

	private TextView mCurrentPathView = null;

	private ProgressDialog mProgressDialog = null;

	private SimpleCursorAdapter mListAdapter;

	private Cursor mFolderContentCursor = null;

	private IntentFilter mFilter = new IntentFilter();

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			Uri uri = intent.getData();
			String path = (uri != null) ? uri.getPath() : null;

			Log.d(TAG, "[BT][FTP] onReceive(), unmounted path: " + path + ", current path: " + mCurrentPath);
			if (mCurrentPath != null && mCurrentPath.startsWith(path)) {
				finish();
			}
		}
	};

	/* Main handler */
	private Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {

			switch (msg.what) {
				case DATA_READY:
					Log.d(TAG, "[BT][FTP] Data is ready for retrieving.");
					mThread = null;
					updateUI();
					break;

				case ERROR:
					sendErrorAndFinish();
					break;

				default:
					break;
			}
		}
	};

	private View.OnClickListener mRequestFocusListener = new View.OnClickListener() {
		public void onClick(View v) {
			v.requestFocus();
		}
	};

/********************************************************************************************
 * Life-cycle Callback Functions of FTP Client
 ********************************************************************************************/

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setTitle(R.string.bluetooth_ftp_local_browser);
		setContentView(R.layout.bt_ftp_folder_listing_view);

		mCurrentPathView = (TextView) findViewById(R.id.current_path);
		if (mCurrentPathView == null) {
			Log.e(TAG, "onCreate(): mCurrentPathView is null.");
			finish();
			return;
		}

		mRoot = SystemUtils.getMountPointPath();
		if (mRoot == null) {
			mRoot = DEFAULT_ROOT;
		}

		mCurrentPath = mRoot;

		mCurrentPathView.setOnClickListener(mRequestFocusListener);

		mFilter.addAction(Intent.ACTION_MEDIA_EJECT);
		mFilter.addDataScheme("file");
		registerReceiver(mReceiver, mFilter);

		String[] from = new String[] {FolderContent.NAME, FolderContent.TYPE, FolderContent.SIZE};

		int[] to = new int[] {R.id.entry_name, R.id.entry_type, R.id.entry_info};

		/* Cursor parameter is set as null till the data is ready. */
		mListAdapter = new SimpleCursorAdapter(this, R.layout.bt_ftp_folder_listing_entry_view, null, from, to);
		mListAdapter.setViewBinder(this);
		getListView().setOnItemClickListener(this);

		if (savedInstanceState != null) {
			mCurrentPath = savedInstanceState.getString(KEY_CUR_PATH);
			if (mThread != null) {
				showProgressDialog();
				if (mThread.isDone(this)) {
					mThread = null;
					updateUI();
				}
			} else {
				updateUI();
			}
		} else {
			updateData(mCurrentPath);
		}
	}

	@Override
	protected void onPause() {
		super.onPause();

		if (isFinishing()) {
			dismissProgressDialog();
		}
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);

		outState.putString(KEY_CUR_PATH, mCurrentPath);
    }

    @Override
    protected void onDestroy() {
		super.onDestroy();

		dismissProgressDialog();

		finishActivity(MENU_MARK_SEVERAL);

		if (mFolderContentCursor != null) {
			mFolderContentCursor.close();
			mFolderContentCursor = null;
		}

		if (mThread != null) {
			mThread.removeListener();
		}

		try {
			unregisterReceiver(mReceiver);
		} catch (IllegalArgumentException iae) {
			// Do nothing
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		Log.d(TAG, "[BT][FTP] Local browser onActivityResult(): " + resultCode);
		if (resultCode == RESULT_OK) {
			switch (requestCode) {
				case MENU_MARK_SEVERAL:
					finish();
					break;

				default:
					break;
			}
		} else {
			// Log.d(TAG, "RESULT_CANCELED");
		}
	}

	@Override
	public void onBackPressed() {
		if (mCurrentPath.equals(mRoot)) {
			super.onBackPressed();

		} else {
			int lastSlash = mCurrentPath.lastIndexOf("/");
			mCurrentPath = mCurrentPath.substring(0, lastSlash);
			updateData(mCurrentPath);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(0, MENU_GOTO_ROOT, 0, R.string.bluetooth_ftp_client_menu_goto_root)
			.setIcon(R.drawable.btftp_ic_menu_root);
		menu.add(0, MENU_MARK_SEVERAL, 0, R.string.bluetooth_ftp_client_menu_mark_several)
			.setIcon(R.drawable.btftp_ic_menu_mark_several);
		menu.add(0, MENU_EXIT, 0, R.string.bluetooth_ftp_client_menu_exit)
			.setIcon(R.drawable.btftp_ic_menu_exit);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		switch(item.getItemId()) {
			case MENU_GOTO_ROOT:
				if (mCurrentPath.equals(mRoot)) {
					Toast.makeText(this, R.string.bluetooth_ftp_client_current_is_root,
							Toast.LENGTH_SHORT).show();
				} else {
					mCurrentPath = mRoot;
					updateData(mCurrentPath);
				}
				break;

			case MENU_MARK_SEVERAL:
				Intent intent = new Intent();
				intent.setClassName(getPackageName(), BluetoothFtpSeveralMarker.class.getName());
				intent.putExtra(TransferringFile.DIRECTION, TransferringFile.DIRECTION_PUSH);
				intent.putExtra(BluetoothFtpSeveralMarker.LOCAL_PATH, mCurrentPath);
				startActivityForResult(intent, MENU_MARK_SEVERAL);
				break;

			case MENU_EXIT:
				finish();
				break;

			default:
				break;
		}

		return true;
	}

/********************************************************************************************
 * Operation Functions
 ********************************************************************************************/

	private synchronized void updateData(String path) {
		showProgressDialog();
		getListView().setEnabled(false);

		mThread = new GetLocalContentThread(path, getContentResolver(), this);
		mThread.start();
	}

	private void updateUI() {
		mCurrentPathView.setText(mCurrentPath);

		// Release previous cursor instance
		if (mFolderContentCursor != null) {
			mFolderContentCursor.close();
		}

		try {
			// Grab the data from content provider and show them on screen.
			mFolderContentCursor = getContentResolver().query(FolderContent.LOCAL_URI, null, null, null, null);
			mListAdapter.changeCursor(mFolderContentCursor);

			// Invoking setListAdapter() will reset the list.
			setListAdapter(mListAdapter);
			mCurrentPathView.requestFocus();

			dismissProgressDialog();
			getListView().setEnabled(true);

		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] updateUI(), Exception: " + ex);
			sendErrorAndFinish();
		}
	}

/********************************************************************************************
 * Interface Functions
 ********************************************************************************************/

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
		ContentResolver resolver = getContentResolver();
		ContentValues values = null;

		TextView nameView = (TextView) view.findViewById(R.id.entry_name);
		TextView infoView = (TextView) view.findViewById(R.id.entry_info);
		View typeView = view.findViewById(R.id.entry_type);

		if (nameView == null || infoView == null || typeView == null) {
			Log.e(TAG, "[BT][FTP] Can't find entry_name, entry_info, or entry_type in the list item.");
			return;
		}

		String name = nameView.getText().toString();
		String info = infoView.getText().toString();
		int type = ((Integer) typeView.getTag()).intValue();
		long size = -1;

		switch(type) {
			case FolderContent.TYPE_FOLDER:
				mCurrentPath = mCurrentPath + "/" + name;
				updateData(mCurrentPath);
				break;

			case FolderContent.TYPE_AUDIO:
			case FolderContent.TYPE_IMAGE:
			case FolderContent.TYPE_VIDEO:
			case FolderContent.TYPE_TEXT:
				try {
					values = new ContentValues();
					resolver.delete(TransferringFile.CONTENT_URI, null, null);

					values.put(TransferringFile.NAME, mCurrentPath + "/" + name);
					values.put(TransferringFile.STATUS, TransferringFile.STATUS_WAITING);
					values.put(TransferringFile.DIRECTION, TransferringFile.DIRECTION_PUSH);
					resolver.insert(TransferringFile.CONTENT_URI, values);

					sendBroadcast(new Intent(BluetoothFtpClient.ACTION_PUSH));
					finish();

				} catch (Exception ex) {
					Log.e(TAG, "[BT][FTP] onItemClick(), Execption: " + ex);
					sendErrorAndFinish();
				}

				break;

			default:
				Log.e(TAG, "Unknown Type");
				break;
		}
	}

	/* For implementing GetLocalContentThread.ResultListener */
	public void onThreadResult(int result) {
		if (result == GetLocalContentThread.RESULT_SUCCEED) {
			mHandler.sendEmptyMessage(DATA_READY);
		} else {
			mHandler.sendEmptyMessage(ERROR);
		}
	}

/********************************************************************************************
 * Utility Functions
 ********************************************************************************************/
	/* Conver Date String from Milli Seconds to YYYY/MM/DD Format */
	public static String getDateString(long millis) {
		String year, month, day;
		Calendar calendar = Calendar.getInstance();

		calendar.setTimeInMillis(millis);

		year	= "" + calendar.get(Calendar.YEAR);
		month	= "" + (calendar.get(Calendar.MONTH) + 1);
		day		= "" + calendar.get(Calendar.DAY_OF_MONTH);

		if (month.length() < 2) {
			month = "0" + month;
		}

		if (day.length() < 2) {
			day = "0" + day;
		}

		return year + "/" + month + "/" + day;
	}

	private synchronized void showProgressDialog() {
		if (mProgressDialog != null) {
			return;
		}

		ProgressDialog dialog = new ProgressDialog(this);
		dialog.setTitle("");
		dialog.setMessage(getString(R.string.bluetooth_ftp_client_processing));
		dialog.setIndeterminate(false);
		dialog.setCancelable(false);

		mProgressDialog = dialog;
		mProgressDialog.show();
	}

	private synchronized void dismissProgressDialog() {
		if (mProgressDialog != null) {
			mProgressDialog.dismiss();
			mProgressDialog = null;
		}
	}

	private void sendErrorAndFinish() {
		sendBroadcast(new Intent(BluetoothFtpClient.ACTION_ERROR));
		finish();
	}
}

/********************************************************************************************
 * Thread for placing local folder content into ContentProvider
 ********************************************************************************************/

class GetLocalContentThread extends Thread {

	private static final String TAG = "GetLocalContentThread";
	private static final String DEFAULT_ROOT	= "/mnt",
								SDCARD			= "sdcard";

	protected static final int RESULT_SUCCEED	= 0,
							   RESULT_FAIL		= 1;

	private String mRoot;
	private File mPath = null;
	private ContentResolver mResolver = null;
	private ResultListener mListener = null;

	interface ResultListener {
		public void onThreadResult(int result);
	}

	public GetLocalContentThread(String path, ContentResolver r, ResultListener l) {
		mPath = new File(path);
		mResolver = r;
		mListener = l;

		mRoot = SystemUtils.getMountPointPath();
		if (mRoot == null) {
			mRoot = DEFAULT_ROOT;
		}
	}

	public synchronized boolean isDone(ResultListener l) {
		if (getState() == Thread.State.TERMINATED) {
			return true;
		} else {
			mListener = l;
			return false;
		}
	}

	public synchronized void removeListener() {
		mListener = null;
	}

	private synchronized void postResult(int result) {
		if (mListener != null) {
			mListener.onThreadResult(result);
		}
	}

	public void run() {
		ContentValues values = new ContentValues();
		File[] files = null;
		String strPath;
		int result = RESULT_SUCCEED;

		try {
			if (mPath.isDirectory()) {
				mResolver.delete(FolderContent.LOCAL_URI, null, null);

				files = mPath.listFiles();
				strPath = mPath.getAbsolutePath();
				if (files == null) {
					postResult(result);
					return;
				}

				for (File f : files) {
					String name, date;
					int type = -1;
					long size = 0;
					values.clear();

					if (f.isHidden()) {
					   continue;
					}

					if (f.isFile()) {
						if (!mRoot.equals(strPath)) {
							name = f.getName();
							type = BluetoothFtpProviderHelper.getTypeCode(name);
							date = BluetoothFtpLocalBrowser.getDateString(f.lastModified());
							size = f.length();

							values.put(FolderContent.NAME, name);
							values.put(FolderContent.TYPE, type);
							values.put(FolderContent.MODIFIED_DATE, date);
							values.put(FolderContent.SIZE, size);

						} else {
							continue;
						}

					} else if (f.isDirectory()) {
						name = f.getName();
						type = FolderContent.TYPE_FOLDER;
						date = BluetoothFtpLocalBrowser.getDateString(f.lastModified());

						if (!mRoot.equals(strPath)) {
							values.put(FolderContent.NAME, name);
							values.put(FolderContent.TYPE, type);
							values.put(FolderContent.MODIFIED_DATE, date);

						} else {
							if (name != null && name.startsWith(SDCARD)) {
								values.put(FolderContent.NAME, name);
								values.put(FolderContent.TYPE, type);
								values.put(FolderContent.MODIFIED_DATE, date);

							} else {
								continue;
							}
						}

					} else {
						Log.d(TAG, "Unknown Type");
						continue;
					}

					mResolver.insert(FolderContent.LOCAL_URI, values);
				}

			} else {
				Log.e(TAG, "[BT][FTP] The path is not a directory: " + mPath.getAbsolutePath());
				result = RESULT_FAIL;
			}

		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] Failed to get local folder content. Exception: " + ex);
			result = RESULT_FAIL;
		}

		postResult(result);
	}
}
