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

import android.app.ListActivity;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SimpleCursorAdapter;
import android.widget.SimpleCursorAdapter.ViewBinder;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

public class BluetoothFtpSeveralMarker extends ListActivity 
		implements ViewBinder, SetupTransferRoutine.ResultListener {
	private static final String TAG = "BluetoothFtpSeveralMarker";

	private static final String KEY_CHECKED_STATES  = "key_checked_states",
								KEY_CUR_LOCAL_PATH  = "key_current_local_path",
								KEY_DIRECTION	    = "key_direction";

	/* Constant base for BluetoothFtpSeveralMarker */
	private static final int SEVERAL_MARKER_BASE = BluetoothFtpClient.CLIENT_BASE + 400;

	protected static final int DATA_READY	= SEVERAL_MARKER_BASE + 1,
							   ERROR		= SEVERAL_MARKER_BASE + 2;

	private static final int MENU_BASE		= SEVERAL_MARKER_BASE + 10,
							 MENU_TRANSFER	= MENU_BASE + 1,
							 MENU_MARK_ALL	= MENU_BASE + 2,
							 MENU_UNMARK_ALL	= MENU_BASE + 3;

	private static final String[] sFrom = {
		FolderContent.NAME,
		FolderContent.SIZE
	};

	private static final int[] sTo = {
		R.id.entry_name,
		R.id.entry_info
	};

	static final String[] sColumns = {
		FolderContent._ID,
		FolderContent.NAME,
		FolderContent.SIZE,
		FolderContent.MODIFIED_DATE
	};

	/* Used only in pushing case */
	static final String LOCAL_PATH = "local_path";

	private int mDirection;

	/* When the routine is activated, it meas that we're preparing the transferring
	 * list and going to terminate this activity. Any life cycle callback function
	 * should lock down and do nothing till the termination of this activity.
	 */
	private static SetupTransferRoutine mThread = null;

	/* Used only in pushing case */
	private String mCurrentLocalPath = null;

	private Cursor mMarkableCursor = null;

	private MultiCheckAdapter mListAdapter;

	private ProgressDialog mProgressDialog = null;

	/* Main handler */
	private Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {

		switch (msg.what) {
			case DATA_READY:
				// Log.d(TAG, "[BT][FTP] Several marker, DATA_READY");
				mThread = null;
				sendResult(true);
				break;

			case ERROR:
				mThread = null;
				sendBroadcast(new Intent(BluetoothFtpClient.ACTION_ERROR));
				setResult(RESULT_OK);
				finish();
				break;

			default:
				break;
			}
		}
	};

/********************************************************************************************
 * Life-cycle Callback Functions of Several Marker
 ********************************************************************************************/

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setTitle(R.string.bluetooth_ftp_several_marker);
		setContentView(R.layout.bt_ftp_mark_several_view);

		if (mThread != null) {
			// Before changing orientation, we are configuring the transferring list and about to finish.
			if (mThread.isDone(this)) {
				sendResult(true);
			} else {
				showProgressDialog();
			}
			return;
		}

		if (savedInstanceState == null) {
			Intent intent = getIntent();
			mDirection = intent.getIntExtra(TransferringFile.DIRECTION, -1);
			mCurrentLocalPath = intent.getStringExtra(LOCAL_PATH);

		} else {
			Log.d(TAG, "savedInstanceState is not null in onCreate()");
			mDirection = savedInstanceState.getInt(KEY_DIRECTION, -1);
			mCurrentLocalPath = savedInstanceState.getString(KEY_CUR_LOCAL_PATH);
		}

		Uri uri = null;

		switch (mDirection) {
			case TransferringFile.DIRECTION_PULL:
				uri = FolderContent.SERVER_MARKS_URI;
				break;

			case TransferringFile.DIRECTION_PUSH:
				if (mCurrentLocalPath == null) {
					Log.w(TAG, "Bad intent: No local path for pusing.");
					sendResult(false);
					return;
				}
				uri = FolderContent.LOCAL_MARKS_URI;
				break;

			default:
				Log.w(TAG, "Bad intent");
				sendResult(false);
				return;
		}

		mMarkableCursor = getContentResolver().query(uri, sColumns, null, null, null);
		if (mMarkableCursor == null) {
			Toast toast = Toast.makeText(this,
					R.string.bluetooth_ftp_client_no_file_to_mark, Toast.LENGTH_SHORT);
			toast.show();
			sendResult(false);
			return;
		}

		int count = mMarkableCursor.getCount();

		if (count > 0) {
			mListAdapter = new MultiCheckAdapter(this,
					R.layout.bt_ftp_mark_several_entry_view, mMarkableCursor, sFrom, sTo);
			mListAdapter.setViewBinder(this);
			setListAdapter(mListAdapter);

		} else {
			Toast toast = Toast.makeText(this,
					R.string.bluetooth_ftp_client_no_file_to_mark, Toast.LENGTH_SHORT);
			toast.show();
			mMarkableCursor.close();
			sendResult(false);
		}
	}

	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);

		if (mThread == null) {
			boolean[] states = savedInstanceState.getBooleanArray(KEY_CHECKED_STATES);

			if (states != null) {
				mListAdapter.applyStates(states);
				setListAdapter(mListAdapter);
			} else {
				return;
			}
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

		if (mThread == null) {
			outState.putBooleanArray(KEY_CHECKED_STATES, mListAdapter.retrieveStates());
			outState.putString(KEY_CUR_LOCAL_PATH, mCurrentLocalPath);
			outState.putInt(KEY_DIRECTION, mDirection);
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		// Log.d(TAG, "[BT][FTP] onDestroy()");
		dismissProgressDialog();

		if (mMarkableCursor != null) {
			mMarkableCursor.close();
			mMarkableCursor = null;
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		int icon_id, string_id;

		switch(mDirection) {
			case TransferringFile.DIRECTION_PULL:
				icon_id = R.drawable.btftp_ic_menu_get_marked;
				string_id = R.string.bluetooth_ftp_client_menu_get_marked;
				break;
			case TransferringFile.DIRECTION_PUSH:
				icon_id = R.drawable.btftp_ic_menu_send_marked;
				string_id = R.string.bluetooth_ftp_client_menu_send_marked;
				break;
			default:
				return false;
		}

		menu.add(0, MENU_TRANSFER, 0, string_id)
			.setIcon(icon_id);
		menu.add(0, MENU_MARK_ALL, 0, R.string.bluetooth_ftp_client_menu_mark_all)
			.setIcon(R.drawable.btftp_ic_menu_mark_all);
		menu.add(0, MENU_UNMARK_ALL, 0, R.string.bluetooth_ftp_client_menu_unmark_all)
			.setIcon(R.drawable.btftp_ic_menu_unmark_all);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		switch(item.getItemId()) {
			case MENU_TRANSFER:
				if (mListAdapter.anyChecked()) {
					setupTransferList();
				} else {
					Toast.makeText(this, R.string.bluetooth_ftp_client_no_marked_file, 
						Toast.LENGTH_SHORT).show();
				}
				break;
			case MENU_MARK_ALL:
				mListAdapter.setAllStatesAs(true);
				mListAdapter.notifyDataSetChanged();
				break;
			case MENU_UNMARK_ALL:
				mListAdapter.setAllStatesAs(false);
				mListAdapter.notifyDataSetChanged();
				break;
			default:
				return false;
		}

		return true;
	}

	@Override
	public void onListItemClick(ListView parent, View view, int position, long id) {
		boolean rlv = view instanceof RelativeLayout;
		String debug = "onListItemClicked(), pos: " + position + ", "
					 + "A RelativeLayout? " + rlv;
		Log.d(TAG, debug);

		CheckBox cb = (CheckBox) view.findViewById(R.id.checkable_view);
		if (cb != null) {
			cb.toggle();
		} else {
			Log.d(TAG, "CheckBox not found.");
		}
	}

/********************************************************************************************
 * Utility Functions
 ********************************************************************************************/

	public void setupTransferList() {
		showProgressDialog();
		mThread = new SetupTransferRoutine(getContentResolver(), mListAdapter.retrieveStates(),
				mDirection, mCurrentLocalPath, this);
		mThread.start();
	}

	private void sendResult(boolean result) {
		if (result) {
			Intent intent = new Intent();
			setResult(RESULT_OK);
			switch (mDirection) {
				case TransferringFile.DIRECTION_PULL:
					intent.setAction(BluetoothFtpClient.ACTION_PULL);
					sendBroadcast(intent);
					break;
				case TransferringFile.DIRECTION_PUSH:
					intent.setAction(BluetoothFtpClient.ACTION_PUSH);
					sendBroadcast(intent);
					break;
				default:
					Log.w(TAG, "[BT][FTP] Bad direction: " + mDirection);
					break;
			}
		} else {
			setResult(RESULT_CANCELED);
		}
		finish();
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

/********************************************************************************************
 * Interface Functions
 ********************************************************************************************/

	public boolean setViewValue(View view, Cursor cursor, int columnIndex) {
		final int id = view.getId();
		final int dateIndex = cursor.getColumnIndex(FolderContent.MODIFIED_DATE);

		int sizeIndex = -1;
		String date = null;

		switch(id) {
			case R.id.entry_name:
				((TextView) view).setText(cursor.getString(columnIndex));
				break;

			case R.id.entry_info:
				date = cursor.getString(dateIndex);
				sizeIndex = columnIndex;
				((EntryInfoView) view).setEntryInfo(cursor.getLong(sizeIndex), date);
				break;

			default:
				// Other cases. Simply skip.
				break;
		}

		return true;
	}

	public void onThreadResult(int res) {
		mHandler.sendEmptyMessage(res);
	}
}

/********************************************************************************************
 * An list adapter that synchronizes checked states of items 
 ********************************************************************************************/

class MultiCheckAdapter extends SimpleCursorAdapter {
	private final static String TAG = "SimpleCursorAdapter";

	/* The checked states */
	private ArrayList<Boolean> mCheckedStates;

	public MultiCheckAdapter(Context context, int layout, Cursor c, String[] from, int[] to) {
		super(context, layout, c, from, to);

		initStates(c.getCount());
	}

	private OnCheckedChangeListener mCheckedListener = new OnCheckedChangeListener() {
		public void onCheckedChanged(CompoundButton btn, boolean isChecked) {
			CheckBox cb = (CheckBox) btn;
			int pos = ((Integer) cb.getTag()).intValue();
			setChecked(pos, isChecked);

		}
	};

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View v = super.getView(position, convertView, parent);

		CheckBox cb = (CheckBox) v.findViewById(R.id.checkable_view);
		if (cb != null) {
			cb.setTag(Integer.valueOf(position));
			cb.setChecked(isChecked(position));
			cb.setOnCheckedChangeListener(mCheckedListener);
		}

		return v;
	}

	private void initStates(int count) {
		mCheckedStates = new ArrayList<Boolean>(count);
		for (int i = 0; i < count; i++) {
			mCheckedStates.add(i, Boolean.valueOf(false));
		}
	}

	public synchronized boolean isChecked(int pos) {
		if (pos < 0 || pos > mCheckedStates.size() - 1) {
			return false;
		}
		return mCheckedStates.get(pos).booleanValue();
	}

	public synchronized void setChecked(int pos, boolean checked) {
		mCheckedStates.set(pos, Boolean.valueOf(checked));
	}

	public synchronized void setAllStatesAs(boolean checked) {
		int size = mCheckedStates.size();
		for (int i = 0; i < size; i++) {
			mCheckedStates.set(i, Boolean.valueOf(checked));
		}
	}

	public synchronized boolean anyChecked() {
		return mCheckedStates.contains(Boolean.valueOf(true));
	}

	public void dumpStates() {
		int size = mCheckedStates.size();
		for (int i = 0; i < size; i++) {
			Log.d(TAG, "States[" + i + "]: " + mCheckedStates.get(i));
		}
	}

	boolean[] retrieveStates() {
		int size = mCheckedStates.size();
		boolean[] states = new boolean[size];
		for (int i = 0; i < size; i++) {
			states[i] = mCheckedStates.get(i).booleanValue();
		}

		return states;
	}

	void applyStates(boolean[] states) {
		for (int i = 0; i < states.length; i++) {
			mCheckedStates.set(i, states[i]);
		}
	}
}

/********************************************************************************************
 * Thread for placing checked files into transferring queue 
 ********************************************************************************************/

class SetupTransferRoutine extends Thread {
	private static final String TAG = "SetupTransferRoutine";

	private ContentResolver mResolver;
	private Uri mUri;
	private boolean[] mStates;
	private String mPath;
	private int mDirection;
	private ResultListener mListener;

	interface ResultListener {
		public void onThreadResult(int res);
	}

	public SetupTransferRoutine(ContentResolver resolver, boolean[] states,
		int direction, String path, ResultListener l) {

		mResolver = resolver;
		mStates = states;
		mDirection = direction;
		mPath = path;
		mListener = l;

		if (mDirection == TransferringFile.DIRECTION_PULL) {
			mUri = FolderContent.SERVER_MARKS_URI;
		} else if (mDirection == TransferringFile.DIRECTION_PUSH) {
			mUri = FolderContent.LOCAL_MARKS_URI;
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

	private synchronized void postResult(int res) {
		if (mListener != null) {
			mListener.onThreadResult(res);
		}
	}

	public void run() {
		int res = BluetoothFtpSeveralMarker.DATA_READY;
		try {
			ContentValues values = new ContentValues();

			mResolver.delete(TransferringFile.CONTENT_URI, null, null);

			Cursor cursor = mResolver.query(mUri, BluetoothFtpSeveralMarker.sColumns, null, null, null);
			if (cursor == null) {
				Log.e(TAG, "[BT][FTP] No file is markable for transferring.");
				return;
			}

			int count = cursor.getCount();
			int name_idx = cursor.getColumnIndex(TransferringFile.NAME);

			cursor.moveToFirst();
			for (int i = 0; i < count; i++) {
				values.clear();
				String name = null;

				if (mStates[i]) {
					name = cursor.getString(name_idx);

					if (mDirection == TransferringFile.DIRECTION_PUSH) {
						name = mPath + "/" + name;
					}

					values.put(TransferringFile.NAME, name);
					values.put(TransferringFile.STATUS, TransferringFile.STATUS_WAITING);
					values.put(TransferringFile.DIRECTION, mDirection);

					mResolver.insert(TransferringFile.CONTENT_URI, values);
				}
			cursor.moveToNext();
			}
			cursor.close();

		} catch (Exception ex) {
			Log.e(TAG, "[BT][FTP] Exception: " + ex);
			res = BluetoothFtpSeveralMarker.ERROR;
		}

		postResult(res);
	}
}
