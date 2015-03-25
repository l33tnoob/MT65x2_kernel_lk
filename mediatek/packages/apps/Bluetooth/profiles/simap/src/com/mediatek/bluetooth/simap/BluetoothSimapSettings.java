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


package com.mediatek.bluetooth.simap;

import java.security.acl.Group;

import android.app.AlertDialog;
import android.app.Dialog;
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

import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.activity.AssembledPreferenceActivity.AssemblyPreference;
import com.mediatek.bluetooth.R;

import android.bluetooth.BluetoothSimap;
import android.bluetooth.IBluetoothSimap;
import android.bluetooth.IBluetoothSimapCallback;

//add by mtk81225
import android.telephony.TelephonyManager;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;


	
public class BluetoothSimapSettings 
		implements AssemblyPreference, Preference.OnPreferenceChangeListener {
	private static final String TAG = "BluetoothSimapSettings";

	private static final String 
								KEY_SHOW_DIALOG				= "show_alert_dialog",
								KEY_PROFILE					= "profile_key_for_dialog";

	private static final int PROFILE_BASE   = 0;
	
	private AlertDialog mDialog;
	private int mProfileKey = PROFILE_BASE;

    private static final String KEY_SIMAP_SERVER_CATEGORY = "simap_server_category";
    private static final String KEY_SIMAP_SERVER_ENABLE = "simap_server_enable";
    private static final String KEY_SIMAP_SERVER_SIM_INDEX = "simap_server_sim_index";

    private CheckBoxPreference mSimapServerEnable;
	private PreferenceCategory mSimapCategory;
    private ListPreference mSimapServerSimIndex;

    private IBluetoothSimap mSimapService = null;

	//add by mtk81225
	private final static int MAX_SIM_NUM = PhoneConstants.GEMINI_SIM_NUM;
	private final static int MSG_SIM_STATE_CHECK = 100;
	private int[] mEntryValueIndex = new int[MAX_SIM_NUM];
	private final static int mSimIndexSummary[] = {R.string.bluetooth_simap_server_sim_index_summary_sim1,
		                                           R.string.bluetooth_simap_server_sim_index_summary_sim2,
		                                           R.string.bluetooth_simap_server_sim_index_summary_sim3,
		                                           R.string.bluetooth_simap_server_sim_index_summary_sim4};


	private IBluetoothSimapCallback mSimapUICallback =
			new IBluetoothSimapCallback.Stub() {
		
		public void postEvent(int event, Bundle data) {
			Log.d(TAG, "SimapUICallback received event: " + event);
			Message msg = Message.obtain();
			msg.what = event;
			if (data != null) {
				msg.setData(new Bundle(data));
			}
			mSimapHandler.sendMessage(msg);
		}
	};

	private ServiceConnection mSimapServerConn = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			Log.i(TAG, "SIMAP: onServiceConnected[+]");
			mSimapService = IBluetoothSimap.Stub.asInterface(service);
			Log.i(TAG, "mSimapService = " + mSimapService);

			if (mSimapService == null){
				Log.e(TAG, " mSimapService == null !");
				return;
			}

			try {
				if (!mSimapService.isServiceStarted())
				{
					Log.i(TAG, "BluetoothSimapService is NOT started yet, start it now ");
					mSimapService.startSimapService();	
				}
			}catch (RemoteException e) {
				Log.e(TAG, "startSimapService error");
			}

			// Check SIMAP server status 
			try {

				mSimapService.registerCallback(mSimapUICallback);
			int simapSate = mSimapService.getState();
		    if (simapSate != BluetoothSimap.STATE_IDLE && simapSate != BluetoothSimap.STATE_ENABLING) {
					Log.i(TAG, "SIMAP server: setChecked(true)...");
					mSimapServerEnable.setChecked(true);
					mSimapServerEnable.setSummary(null);
				}
			else {				
				Log.i(TAG, "SIMAP server: setChecked(false)...");
				mSimapServerEnable.setChecked(false);
			}			
			}catch (RemoteException e) {
				Log.e(TAG, "getState error");
			}

			if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
				mSimapHandler.sendEmptyMessage(MSG_SIM_STATE_CHECK);
			}
			Log.i(TAG, "SIMAP: onServiceConnected[-]");			
		}

		public void onServiceDisconnected(ComponentName className) {
			Log.e(TAG, "Unexpectedly disconnected with BluetoothSimapService");
			mSimapServerEnable.setEnabled(false);
			Toast.makeText(BluetoothSimapSettings.this.parentActivity,
					"SIMAP Service disconnected unexpectedly.",
					Toast.LENGTH_SHORT).show();
		}
	};
	
	//private final IntentFilter mIntentFilter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);

	/* For handling Bluetooth state changed */
	//private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if(action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)){
				int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

				switch (state) {
					case BluetoothAdapter.STATE_ON:
						if (mSimapService != null) {
							mSimapServerEnable.setEnabled(true);
						}
						break;

					case BluetoothAdapter.STATE_TURNING_OFF:
						mSimapServerEnable.setEnabled(false);
						break;

					default:
						// do nothing.
						break;
				}
			}
			else if(action.equals(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED)){
				Log.d(TAG,"receiver: TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED");
                int slotId = intent.getIntExtra(TelephonyIntents.INTENT_KEY_ICC_SLOT, -1);
                int simStatus = intent.getIntExtra(TelephonyIntents.INTENT_KEY_ICC_STATE, -1);
                Log.d(TAG, "slotid is " +slotId + "status is "+ simStatus);
				if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
					mSimapHandler.sendEmptyMessage(MSG_SIM_STATE_CHECK);
				}
			}
		}
	};


    private Handler mSimapHandler = new Handler() {
	public void handleMessage(Message msg) {
	    Log.d(TAG, "mSimapHandler handling event: " + msg.what);
		
	    switch(msg.what) {
		case BluetoothSimap.BT_SIMAPUI_READY:
		    /* Change state of check-box to checked */
		    Log.d(TAG, "SIMAP Server is turned on ! ");
		    mSimapServerEnable.setChecked(true);
		    mSimapServerEnable.setEnabled(true);
		    //mSimapServerEnable.setSummary(null);
		    break;
		case BluetoothSimap.BT_SIMAPUI_DISABLED:
		    /* Change state of check-box to unchecked */
		    Log.d(TAG, "SIMAP Server is turned off.");
		    mSimapServerEnable.setChecked(false);
		    mSimapServerEnable.setEnabled(true);
		    //mSimapServerEnable.setSummary(null);
		    break;
		case MSG_SIM_STATE_CHECK:
			Log.d(TAG, "SIMAP sim state check!");
			handleCheckSimState();
			break;
		default:
		    break;
	    }
	}
    };


	private PreferenceActivity parentActivity;
	
	public int getPreferenceResourceId(){

		return com.mediatek.bluetooth.R.xml.bluetooth_simap_settings;
	}

	//add by mtk81225
	public boolean isSimExist(int slotId){
		//TelephonyManager tm = TelephonyManager.getDefault();
		//int simState = tm.getSimStateGemini(slotId);
		TelephonyManagerEx mTelephonyManager = TelephonyManagerEx.getDefault();
		int simState = mTelephonyManager.getSimState(slotId);
		
		Log.d(TAG, "sim current state = " + simState);
		if(simState == TelephonyManager.SIM_STATE_ABSENT)
			return false;
		else
			return true;
	}

	private int getAvailableSimNum()
	{
		int i= 0;
		int simNum = 0;
		
		for(i = 0; i < MAX_SIM_NUM; i++){
			if(isSimExist(i) == true)
				simNum ++;
		}
		return simNum;
	}

	private void setEntryValueIndex(int slotId, int valueId)
	{
		if(mEntryValueIndex != null)
			mEntryValueIndex[slotId] = valueId;
	}
	
	private int getEntryValueIndex(int slotId)
	{
		if(mEntryValueIndex != null)
			return mEntryValueIndex[slotId];
		else
			return 0;
	}
	
	private int getAvailSimId()
	{
		int i= 0;
		
		for(i = 0; i < MAX_SIM_NUM; i++){
			if(isSimExist(i) == true)
				return (i+1);
		}
		return 1;
	}

	public void handleCheckSimState(){
		int simNum = 0;
		int i= 0;
		int simIndex = 1;
		int[] slot = {0, 0, 0, 0};
		
		for(i = 0; i < MAX_SIM_NUM; i++){
			if(isSimExist(i) == true){
				simNum ++;
				slot[i] = 1;
			}
			else{
				slot[i] = 0;
			}
		}
		
		Log.d(TAG, "simNum = " + simNum);
		if(simNum > 1){
			//<!-- for dual-SIM			
			try {
				if(mSimapService != null){
					simIndex = mSimapService.getSIMIndex();
					if(slot[simIndex-1] == 0)
						simIndex = getAvailSimId();
				}
				else{
					Log.e(TAG, "handleCheckSimState mSimapService = null!");
				}
				Log.i(TAG, "simIndex = " + simIndex);
			}catch (RemoteException e) {
				Log.e(TAG, "getSIMIndex error");
			}

			//valueIndex start from 0
			mSimapServerSimIndex.setValueIndex(getEntryValueIndex(simIndex-1));
			mSimapServerSimIndex.setSummary(mSimIndexSummary[simIndex-1]);			
			if(!mSimapServerSimIndex.isEnabled())
				mSimapServerSimIndex.setEnabled(true);
			//-->	
		}	
		else if(simNum == 1){
			simIndex = getAvailSimId();
			try {
				if(mSimapService != null){
					mSimapService.selectSIM(simIndex);
				}
				else{
					Log.e(TAG, "handleCheckSimState mSimapService = null!");
				}
				Log.i(TAG, "simIndex = " + simIndex);
			}catch (RemoteException e) {
				Log.e(TAG, "getSIMIndex error");
			}
			mSimapServerSimIndex.setValueIndex(getEntryValueIndex(simIndex-1));
			mSimapServerSimIndex.setSummary(mSimIndexSummary[simIndex-1]);	
			mSimapServerSimIndex.setEnabled(false);
		}
		else if(simNum == 0){
			if (mSimapCategory != null && mSimapServerSimIndex != null) {
				boolean ret = mSimapCategory.removePreference(mSimapServerSimIndex);
				Log.i(TAG, "SIMAP: removePreference mSimapServerSimIndex...return: " + ret);		
			}
		}
	}
	
	public void onCreate(PreferenceActivity parentActivity) {
		Log.i(TAG, "onCreate..."); 
		int i = 0;
		int slotId = 0;
		this.parentActivity = parentActivity;

		mSimapServerEnable = (CheckBoxPreference)this.parentActivity.findPreference(KEY_SIMAP_SERVER_ENABLE);
		mSimapServerSimIndex = (ListPreference)this.parentActivity.findPreference(KEY_SIMAP_SERVER_SIM_INDEX);
		mSimapCategory = (PreferenceCategory)this.parentActivity.findPreference(KEY_SIMAP_SERVER_CATEGORY);
		int availableSimNum = getAvailableSimNum();
		CharSequence[] mTmpEntries = this.parentActivity.getResources().getTextArray(R.array.bluetooth_simap_server_sim_index_entries);
		CharSequence[] mTmpEntriesValue = this.parentActivity.getResources().getTextArray(R.array.bluetooth_simap_server_sim_index_values);
		CharSequence[] mEntries = new CharSequence[availableSimNum];
		CharSequence[] mEntriesValue = new CharSequence[availableSimNum];
		for(i = 0; i < availableSimNum;)
		{
			if(slotId >= MAX_SIM_NUM)
				break;
			
			if(isSimExist(slotId) == true)
			{
				mEntries[i] = mTmpEntries[slotId];
				mEntriesValue[i] = mTmpEntriesValue[slotId];
				setEntryValueIndex(slotId, i);
				i++;
			}
			else
			{
				setEntryValueIndex(slotId, 0);
			}
			slotId ++;
		}
		mSimapServerSimIndex.setEntries(mEntries);
		mSimapServerSimIndex.setEntryValues(mEntriesValue);
		
		if (mSimapServerEnable == null || mSimapServerSimIndex == null)
		{
		  Log.e(TAG, "[BT][SIMAP] Can't find SIMAP preferences.");
			this.parentActivity.finish();
			return;
    }
		else
		{
		    mSimapServerEnable.setOnPreferenceChangeListener(this);
		    mSimapServerSimIndex.setOnPreferenceChangeListener(this);
		}


		/* Bind to BluetoothSimapService */
		Log.i(TAG, "SIMAP bindService: " + IBluetoothSimap.class.getName());
		boolean bindSimap = this.parentActivity.bindService(
			new Intent(IBluetoothSimap.class.getName()), mSimapServerConn, Context.BIND_AUTO_CREATE);

		if (!bindSimap) {
			Log.d(TAG, "SIMAP Service binding failed.");
		}

		if (FeatureOption.MTK_GEMINI_SUPPORT == false) {
			Log.i(TAG, "SIMAP: MTK_GEMINI_SUPPORT == false");

			Log.i(TAG, "SIMAP: findPreference KEY_SIMAP_SERVER_CATEGORY...return: " + mSimapCategory);		

			if (mSimapCategory != null && mSimapServerSimIndex != null) {
				boolean ret = mSimapCategory.removePreference(mSimapServerSimIndex);
				Log.i(TAG, "SIMAP: removePreference mSimapServerSimIndex...return: " + ret);		
		    }
		}

		// Register for being notified that the state of Bluetooth has changed
		IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
		filter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        this.parentActivity.registerReceiver(mReceiver, filter);
	}

	public Dialog onCreateDialog( int id ){

		return null;
	}

	public void onRestoreInstanceState(Bundle savedInstanceState) {

		mProfileKey = savedInstanceState.getInt(KEY_PROFILE, PROFILE_BASE);

		if (savedInstanceState.getBoolean(KEY_SHOW_DIALOG, false) &&
			mProfileKey != PROFILE_BASE) {
			//showWarnningDialog(mProfileKey);
		}
	}

	public void onSaveInstanceState(Bundle outState) {

		if (mDialog != null) {
			outState.putBoolean(KEY_SHOW_DIALOG, mDialog.isShowing());
			outState.putInt(KEY_PROFILE, mProfileKey);
		}
	}


	public void onResume() {
		Log.d(TAG, "onResume()");

	}

	
	public void onDestroy() {

		Log.d(TAG, "onDestroy()");

		if (mDialog != null) {
			mDialog.dismiss();
		}
        this.parentActivity.unregisterReceiver(mReceiver);

			try {
			if (mSimapService != null) {
			    mSimapService.unregisterCallback(mSimapUICallback);
			} else {
				Log.e(TAG, "unregisterCallback(mSimapUICallback) failed: null mSimapService.");
			}
			}catch (RemoteException e) {
				Log.e(TAG, "unregisterCallback(mSimapUICallback) error");
			}

			try {
				this.parentActivity.unbindService(mSimapServerConn);
			} catch (Exception ex) {
				Log.w(TAG, "[BT][SIMAP] Exception triggered when unbinding service: " + ex);
			}
	}

	
	public boolean onPreferenceTreeClick(PreferenceScreen prefScreen, Preference pref) {
		final String key = pref.getKey();
		// Log.d(TAG, "onPreferenceTreeClick(), key: " + key);
    if (pref == mSimapServerEnable) {
			Log.d(TAG, "SIMAP Server Checkbox is Clicked.");
		}
		return false;
	} 

	/* This function is called when the new value hasn't been applied. */
	public boolean onPreferenceChange(Preference preference, Object objValue) {
		String sdState = Environment.getExternalStorageState();
		final String key = preference.getKey();
		Log.d(TAG, "onPreferenceChange(), key: " + key);

		//SIMAP server part
		if (KEY_SIMAP_SERVER_ENABLE.equals(key)) {

			/* Check SIMAP server interface */
			if (mSimapService == null) {
				Log.e(TAG, "mSimapService is empty.");
				return false;
			}

			try {
				if (!mSimapServerEnable.isChecked()) {
					/* Enable SIMAP server via binder function call */
					mSimapService.enableService();
					/* Disable the checkbox preference UI */
					Log.d(TAG, "Enable the UI, waiting for SIMAP enable result.");
					mSimapServerEnable.setEnabled(false);
				} else {
					/* Disable simap erver via binder function call */
					mSimapService.disableService();
					Log.d(TAG, "Disable the UI, waiting for SIMAP disable result.");
					mSimapServerEnable.setEnabled(false);
				}
			} catch (RemoteException e) {
				Log.e(TAG, "Enable/disable SIMAP server failed.");
			}
			// Do not update status of the checkbox.
			return false;

		} else 	if (FeatureOption.MTK_GEMINI_SUPPORT == true) {

			if (KEY_SIMAP_SERVER_SIM_INDEX.equals(key)) {
				try {
					int value = Integer.parseInt((String) objValue);
					Log.d(TAG, "SIMAP Server SIM index Changed: " + value);
					mSimapService.selectSIM(value);

					mSimapServerSimIndex.setSummary(mSimIndexSummary[value-1]);			
					
				} catch (RemoteException ex) {
					Log.e(TAG, "Could not parse SIMAP Server SIM index value.");
				}
			}
		}


		// return ture to update state, false for not update.
		return true;
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
}
