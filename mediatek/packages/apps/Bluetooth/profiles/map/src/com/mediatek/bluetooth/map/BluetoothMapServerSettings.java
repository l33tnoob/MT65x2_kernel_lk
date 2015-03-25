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


package com.mediatek.bluetooth.map;

import com.mediatek.activity.AssembledPreferenceActivity.AssemblyPreference;
import com.mediatek.bluetooth.R;


import android.app.Dialog;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.PreferenceCategory;
import android.util.Log;
import android.widget.Toast;

import android.content.IntentFilter;
import android.content.ContentResolver;
import android.database.ContentObserver;
import android.database.Cursor;

import android.net.Uri;

import com.mediatek.common.featureoption.FeatureOption;

import android.bluetooth.BluetoothMap;
import android.bluetooth.IBluetoothMap;

import java.util.ArrayList;
import java.lang.SecurityException;

import com.mediatek.bluetooth.map.Email;
import com.mediatek.bluetooth.map.util.NetworkUtil;
import com.mediatek.bluetooth.map.IBluetoothMapSetting;
import com.mediatek.bluetooth.map.IBluetoothMapSettingCallback;
import com.mediatek.bluetooth.map.MultiSelectListPreference;

import com.mediatek.xlog.Xlog;	
public class BluetoothMapServerSettings 
	implements AssemblyPreference, Preference.OnPreferenceChangeListener{
	private static final String TAG = "BluetoothMapServerSettings";
	

	private ArrayList<String> mEmailAddresses = new ArrayList<String>();
	private ArrayList<String> mAccountIds = new ArrayList<String>();
	private long mPreferredAccountId = -1;

	private ArrayList<String> mSimCards = new ArrayList<String>();
	private ArrayList<String> mSimIds = new ArrayList<String>();


	private AlertDialog mDialog;
	private AccountObserver mAccountObserver;
	private Uri mAccountUri = Uri.parse("content://com.android.email.provider/account");;

	private static final String KEY_MAP_SERVER_CATEGORY = "map_server_category";
	private static final String KEY_MAP_SERVER_ENABLE = "map_server_enable";
	private static final String KEY_MAP_SERVER_SIM_INDEX = "map_server_sim_index";
	private static final String KEY_MAP_SERVER_ACCOUNT_INDEX = "map_server_account_index";

	private final static String KEY_SIM_ID = "Sim_id";
	private final static String KEY_SIM_CARD = "Sim_card";
	private final static String KEY_ACCOUNT = "Accounts";
	private final static String KEY_EMAIL_ADDR = "Email_addr";	

	private	PreferenceCategory mCategory;
	private CheckBoxPreference mMapServerEnabler;
	private MultiSelectListPreference mMapServerSimIndex;
	private AccountListPreference mMapServerAccountIndex;

	private static Bundle mSavedState;	

	private final IntentFilter mIntentFilter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);

	/* For handling Bluetooth state changed */
	private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			int state = BluetoothAdapter.getDefaultAdapter().getState();

			switch (state) {
				case BluetoothAdapter.STATE_ON:
					if (mMapServerEnabler != null) {
						mMapServerEnabler.setEnabled(true);
					}
					break;

				case BluetoothAdapter.STATE_TURNING_OFF:
					mMapServerEnabler.setEnabled(false);
					break;

				default:
					// do nothing.
					break;
			}
		}
	};
	
	

	class AccountObserver extends ContentObserver{
		public AccountObserver(Handler handler) {
			super(handler);
		}
		@Override
		public void onChange(boolean onSelf) {				
			super.onChange(onSelf);
			log("AccountObserver: onChange");
			updateAccount();
			mMapServerAccountIndex.onAccountChanged();								
		}
	
	}
	private DialogInterface.OnMultiChoiceClickListener mSimItemListener = 
									new DialogInterface.OnMultiChoiceClickListener () {
			public void onClick(DialogInterface dialog, int which, boolean isChecked) {
				if (mService == null){
					return;
				}
				if (isChecked) {
					try {
						mService.registerSim(which);
					} catch (RemoteException e) {
						log(e.toString());
					}
				} else {					
					try{
						mService.unregisterSim(which);
					} catch (RemoteException e) {
						log(e.toString());
					}
				}
			}
	};	

	private DialogInterface.OnClickListener mSimButtonListener = 
									new DialogInterface.OnClickListener () {
		public void onClick(DialogInterface dialog, int which) {
			dialog.dismiss();				
		}
	};
	private DialogInterface.OnClickListener mAccountButtonListener = 
									new DialogInterface.OnClickListener () {
		public void onClick(DialogInterface dialog, int which) {
			dialog.dismiss();				
		}
	};
	private IBluetoothMapSetting mService = null;
	private IBluetoothMapSettingCallback mCallback = new IBluetoothMapSettingCallback.Stub(){
		public void onStateChanged(int newState) {
			updateMapState(newState);
		}
	};
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			log("Proxy object connected");
			mService = IBluetoothMapSetting.Stub.asInterface(service);
			if (mService != null) {
				try{
					mService.registerCallback(mCallback);
					updateMapState(mService.isEnabled()? BluetoothMap.STATE_ENABLED:BluetoothMap.STATE_DISABLED);
					updateSim();
					updateAccount();
				} catch (RemoteException e) {
						log(e.toString());
				}
				registerAccountOberver();
			}
		}
		public void onServiceDisconnected(ComponentName className) {
			log("Proxy object disconnected");
			mService = null;
			 
		}
	};
	private void registerAccountOberver() {
		mAccountObserver = new AccountObserver(new Handler());
		this.parentActivity.getContentResolver().registerContentObserver(Email.CONTENT_URI, true, mAccountObserver);
			
	}

	private PreferenceActivity parentActivity;
		
	public int getPreferenceResourceId(){
	
		return com.mediatek.bluetooth.R.xml.bluetooth_map_settings;
	}

	public void onCreate( PreferenceActivity parentActivity ){
		log("onCreate");
		boolean isEnabled = false;
		this.parentActivity = parentActivity;

		if(!parentActivity.bindService(new Intent(IBluetoothMapSetting.class.getName()), mConnection, Context.BIND_AUTO_CREATE)){
			log("fail to bind service");
			return;
		}		

		//MAP part
		mMapServerEnabler = (CheckBoxPreference) this.parentActivity.findPreference(KEY_MAP_SERVER_ENABLE);
		mMapServerSimIndex = (MultiSelectListPreference)this.parentActivity.findPreference(KEY_MAP_SERVER_SIM_INDEX);
		mMapServerAccountIndex = (AccountListPreference)this.parentActivity.findPreference(KEY_MAP_SERVER_ACCOUNT_INDEX);
		if (mMapServerEnabler != null) {
			mMapServerEnabler.setOnPreferenceChangeListener(this);
		}
		if (mMapServerSimIndex != null) {
			mMapServerSimIndex.setOnPreferenceChangeListener(this);
		}
		if (mMapServerAccountIndex != null) {
			mMapServerAccountIndex.setOnPreferenceChangeListener(this);
		}

		
 		updateAccountPreference();
		updateSimPreference(null);		
		
		// Register for being notified that the state of Bluetooth has changed
        this.parentActivity.registerReceiver(mReceiver, mIntentFilter);
	
	}


	public void onResume() {
		Log.d(TAG, "onResume()");
		mMapServerAccountIndex.onResume();
	}

	public void onDestroy() {
		Log.d(TAG, "onDestroy()");

        this.parentActivity.unregisterReceiver(mReceiver);
		if (mAccountObserver != null) {
			this.parentActivity.getContentResolver().unregisterContentObserver(mAccountObserver);
		}
		
		if (mDialog != null) {
			mDialog.dismiss();
		}
		if (mService != null){
			try{
				mService.unregisterCallback(mCallback);
			} catch (RemoteException e) {
				log(e.toString());
			}
		}
		this.parentActivity.unbindService(mConnection);

	}
	
	public boolean onPreferenceChange(Preference preference, Object objValue) 
{
		boolean result = false;
		final String key = preference.getKey();
		StringBuilder summary = new StringBuilder();
		log("onPreferenceChange(), key:"+key);
			if (mService == null){
				return false;
			}
		if(key.equals(KEY_MAP_SERVER_ENABLE)){
			mMapServerEnabler.setEnabled(false);
			if (!mMapServerEnabler.isChecked()) {					
				try {
					mService.enableServer();
				} catch (RemoteException e) {
					log(e.toString());
				}		
			} else {									
				try {
					mService.disableServer();
				} catch (RemoteException e) {
					log(e.toString());
				}
			}
			result = false;
		} else if (key.equals(KEY_MAP_SERVER_ACCOUNT_INDEX)) {
			long value = Long.parseLong((String) objValue);
			Log.d(TAG, "MAP Server accpunt index Changed: " + value);
			try {
			if (mService.replaceAccount(value)){
				mPreferredAccountId = value;
				result = true;
			} else {
				result = false;
			}
			} catch (RemoteException e) {
				log(e.toString());
			}
			//get index of account id
			int index = mAccountIds.indexOf((String) objValue);			
			if (index == -1) {
				log("invalid index");				
			} else {
				summary.append(mEmailAddresses.get(index));
			}
			//set summary
			mMapServerAccountIndex.setSummary(summary.toString());		
				
		} else if (key.equals(KEY_MAP_SERVER_SIM_INDEX)) {
			boolean[] selectedSims = ((MultiSelectListPreference)preference).getSelectedItems();
			try {
			for (int index = 0; selectedSims != null && index < selectedSims.length; index ++){
				if (selectedSims[index]) {
					mService.registerSim(Integer.parseInt(mSimIds.get(index)));
				} else {
					mService.unregisterSim(Integer.parseInt(mSimIds.get(index)));
				}
			}
			result = true;
			} catch (RemoteException e) {
				log(e.toString());
				result = false;
			}
		}
		return result;
	}	



	/* Just for convience */
	private boolean isBtEnabled() {
		BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
		if (bt == null) {
			return false;
		} else {
			return bt.isEnabled();
		}
	}

	private void showWarningDialog() {
		Builder builder = new Builder(parentActivity);
		builder.setIcon(android.R.drawable.ic_dialog_alert)
				.setTitle(R.string.bluetooth_map_server_account_index_title)
				.setPositiveButton(R.string.bluetooth_map_server_OK, mAccountButtonListener)
				.setNegativeButton(R.string.bluetooth_map_server_cancel, mAccountButtonListener)
				.setMessage(R.string.bluetooth_map_server_no_account);
		mDialog = builder.create();
		mDialog.show();
	}

	private void updateAccount() {
		log("updateAccount");
		mEmailAddresses.clear();
		mAccountIds.clear();		
		long id = getEmailAccountInfo(mEmailAddresses, mAccountIds);
		
		if (mService == null){
			return;
		}
		try{
			mPreferredAccountId = mService.getEmailAccount();
		}catch(RemoteException e) {
		}		

		//if orignal account does not exist, set default account
		if (mPreferredAccountId < 0 || !mAccountIds.contains(Long.toString(mPreferredAccountId))) {
			mPreferredAccountId = id;
			try {
				mService.replaceAccount(mPreferredAccountId);
			}catch(RemoteException e) {
			}				
		} 	
			updateAccountPreference();
	} 

	private void updateAccountPreference(){
		log("updateAccountPreference()");
		int index;
		if (mEmailAddresses != null && mAccountIds != null) {
			mMapServerAccountIndex.setEntries(mEmailAddresses.toArray(new CharSequence[mEmailAddresses.size()]));
			mMapServerAccountIndex.setEntryValues(mAccountIds.toArray(new CharSequence[mAccountIds.size()]));
			mMapServerAccountIndex.setValue(Long.toString(mPreferredAccountId));
			mMapServerAccountIndex.setOnPreferenceChangeListener(this);	
		
		        index = mAccountIds.indexOf(Long.toString(mPreferredAccountId));
		        if (index >= 0 && mMapServerAccountIndex != null) {
			        mMapServerAccountIndex.setSummary(mEmailAddresses.get(index));
			} else {
				mMapServerAccountIndex.setSummary(null);
		        }
		}

	}

	private void updateMapState(int newState){
		if (newState == BluetoothMap.STATE_ENABLED) {
			mMapServerEnabler.setEnabled(true);
			mMapServerEnabler.setChecked(true);
			mMapServerEnabler.setSummary(null);
		} else if(newState == BluetoothMap.STATE_DISABLED) {
			mMapServerEnabler.setEnabled(true);
			mMapServerEnabler.setChecked(false);
	//		mMapServerEnabler.setSummary(R.string.bluetooth_map_server_enable_summary);
		}	
	}

	private void updateSim() {
		int[] sims = null;
		
		boolean[] selectedSims = new boolean[mSimIds.size()];	
		try {
			sims = mService.getSims();
		}catch(RemoteException e) {
			log(e.toString());
		}
		
		if (sims != null && sims.length != 0) {
			for (int sim : sims){
				log("sim:" + sim);
				int index = mSimIds.indexOf(Integer.toString(sim));
				if (index != -1) {					
					selectedSims[index] = true;
				} else {
					log("invalid sim card index");
				}			
			}
		}		
		updateSimPreference(selectedSims);
		
	}
	private void updateSimPreference(boolean[] selectedSims){
		mSimCards.clear();
		mSimIds.clear();
		if (mMapServerSimIndex == null) {
			log("sim preference is null");
			return;
		}		
			
		for(int index = 0; index < NetworkUtil.getTotalSlotCount(); index ++) {
			int slot = index + NetworkUtil.getDefaultSlot();
			mSimCards.add("SIM"+slot);
			mSimIds.add(Integer.toString(slot));
		} 

		mMapServerSimIndex.setEntries(mSimCards.toArray(new CharSequence[mSimCards.size()]));
		mMapServerSimIndex.setEntryValues(mSimIds.toArray(new CharSequence[mSimIds.size()]));
		if (selectedSims != null) {
			mMapServerSimIndex.setSelectedItems(selectedSims);
		} 
		
	}

		private long getEmailAccountInfo(ArrayList<String> address, ArrayList<String> accountId){
			String columnId = "_id";
			String columnAddr = "emailAddress";
			String columnDeault = "isDefault";
			long defaultAccoutId = -1;

			String[] projection;
			projection = new String[]{columnId, columnAddr, columnDeault};
		
			log("getEmailAccountInfo()");
			
			ContentResolver resolver = parentActivity.getContentResolver();
			//find the default account ID and expose as the  MAP account
			Cursor accountCursor = null;
			try	{
				accountCursor = resolver.query(mAccountUri, 
									projection,
									null,	
									null, 
									null);
			} catch (SecurityException e) {
				e.printStackTrace();
				return defaultAccoutId;
			}
			if (accountCursor == null ) {
				log("fail to query email account");
				return defaultAccoutId;
			}
			while(accountCursor.moveToNext()) {
				address.add(accountCursor.getString(1));
				accountId.add(Long.toString(accountCursor.getLong(0)));
				if (accountCursor.getInt(2) == 1) {
					defaultAccoutId = accountCursor.getLong(0);
				}
			}
			accountCursor.close();
			return defaultAccoutId;
		}
		
		public Dialog onCreateDialog( int id ){
			return null;
		}
		public void onRestoreInstanceState( Bundle savedInstanceState ){
			if (savedInstanceState != null) {	
				mAccountIds = savedInstanceState.getStringArrayList(KEY_ACCOUNT);
				mEmailAddresses = savedInstanceState.getStringArrayList(KEY_EMAIL_ADDR);	
				if (mEmailAddresses != null) {
				updateAccountPreference();
			} 
			} 
			return;
		}		
		
		public void onSaveInstanceState( Bundle outState ){	
			outState.putStringArrayList(KEY_ACCOUNT, mAccountIds);
			outState.putStringArrayList(KEY_EMAIL_ADDR, mEmailAddresses);
			return;
		}

		private void log(String info) {
			if (null != info){
			Xlog.v(TAG, info);
		}
		}
	
	
}
