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

package com.mediatek.bluetooth.prx.monitor;

import android.app.Dialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.widget.Toast;

import com.mediatek.activity.ServiceActivityHelper;
import com.mediatek.activity.ServiceActivityHelper.ServiceActivity;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.service.IBluetoothPrxm;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * 1. response code 
 * 2. busy state: setpath will make the stack busy and normal config can't be perform
 *
 */
public class PrxmDeviceSettingActivity extends PreferenceActivity implements ServiceActivity<IBluetoothPrxm>, 
					Preference.OnPreferenceChangeListener, 
					Preference.OnPreferenceClickListener {

	public static final String ACTION_START = "com.mediatek.bluetooth.prx.monitor.device.setting.action.START";

	// device for setting
	private BluetoothDevice device;
	private BluetoothPrxmDevice deviceInfo;

	// ui components
	private CheckBoxPreference connectPref;				// connect checkbox
	private Preference findDevicePref;			// find device
	private ListPreference linkLossLevelPref;			// link loss level
	private ListPreference pathLossLevelPref;			// path loss level
	private PrxmPathLossThresholdPreference pathLossThresholdPref;	// path loss threshold

	private static boolean findDeviceCanceled = true;
	// activity service helper
	private ServiceActivityHelper<IBluetoothPrxm> helper;


	/**
	 * implements ServiceActivity
	 */
	public IBluetoothPrxm asInterface( IBinder service ){

		return IBluetoothPrxm.Stub.asInterface(service);
	}
	/**
	 * implements ServiceActivity
	 */
	public String getServiceAction(){

		return IBluetoothPrxm.class.getName();
	}
	/**
	 * implements ServiceActivity
	 */
	public void onServiceConnected(){

		try {
			// register device
			this.helper.service.registerDevice( this.device );

			// start to receive callback for device
			this.helper.service.registerDeviceCallback( this.device.getAddress(), this.callback );
		}
		catch( Exception ex ){

			BtLog.e( "PrxmDeviceSettingActivity call service(registerDeviceCallback) error: ", ex );
		}

		// release service lock and refresh ui: onDestroy() can be called before onServiceConnected() callback
		if( this.helper != null ){

			this.helper.releaseServiceLock();
			this.helper.refreshUi( this );
		}
	}
	/**
	 * implements ServiceActivity
	 */
	public void onServiceDisconnected(){
	}
	/**
	 * implements ServiceActivity - update ui components according current stack state
	 */
	public void refreshActivityUi(){

		// update DeviceInfo
		this.deviceInfo = this.getDeviceInfo();

		if( this.deviceInfo == null ){

			BtLog.e( "PrxmDeviceSettingActivity.refreshActivityUi() error: null device-info" );
			return;
		}

		if( Options.LL_DEBUG ){

			BtLog.d( "refreshActivityUi(): new state[" + this.deviceInfo.getCurrentState() + "]" );
		}

		switch( this.deviceInfo.getCurrentState() ){
			case PrxmConstants.PRXM_STATE_NEW:
				// connect
				this.connectPref.setEnabled(true);
				this.connectPref.setChecked(false);
				this.connectPref.setSummary( R.string.bt_prxm_devs_connect_summary );
				break;
			case PrxmConstants.PRXM_STATE_CONNECTED:
				// connect
				this.connectPref.setEnabled(true);
				this.connectPref.setChecked(true);
				this.connectPref.setSummary( "" );

				// link loss
				this.linkLossLevelPref.setValue( Byte.toString( this.deviceInfo.getLinkLossLevel() ) );

				// path loss
				if( this.deviceInfo.getCapability() != PrxmConstants.PRXM_CAP_NONE ){

					this.pathLossLevelPref.setValue( Byte.toString( this.deviceInfo.getPathLossLevel() ) );
					this.pathLossLevelPref.setEnabled(true);
					this.pathLossThresholdPref.setEnabled( ( this.deviceInfo.getPathLossLevel() != PrxmConstants.PRXM_ALERT_LEVEL_NO ) );
					this.findDevicePref.setEnabled(true);
				}
				else {
					// no path loss support
					this.pathLossLevelPref.setEnabled(false);
					this.pathLossThresholdPref.setEnabled(false);
					this.findDevicePref.setEnabled(false);
				}
				break;
			case PrxmConstants.PRXM_STATE_CONNECTING:
			case PrxmConstants.PRXM_STATE_DISCONNECTING:
				this.connectPref.setEnabled(false);
				break;
		}
	}


	/**
	 * handle callback (result) from service
	 */
	private ResultReceiver callback = new ResultReceiver( new Handler() ){

		@Override
		protected void onReceiveResult( int resultCode, Bundle resultData ){

			switch( resultCode ){
				case PrxmMsg.MSG_ID_BT_PRXM_CONNECT_CNF:
				case PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND:
				case PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_CNF:
					// response code
					byte rspcode = resultData.getByte( "rspcode", ResponseCode.UNKNOWN );
					// display error message
					if( rspcode != ResponseCode.SUCCESS ){

						String msg = null;
						if( resultCode == PrxmMsg.MSG_ID_BT_PRXM_CONNECT_CNF ){
							msg = getString( R.string.bt_prxm_devs_connect_failed );	
						}
						else if( resultCode == PrxmMsg.MSG_ID_BT_PRXM_DISCONNECT_IND ){
							msg = getString( R.string.bt_prxm_devs_disconnect_failed );
						}
						else if( resultCode == PrxmMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_CNF ){
							msg = getString( R.string.bt_prxm_devs_link_loss_config_failed );
						}
						Toast.makeText( PrxmDeviceSettingActivity.this, msg, Toast.LENGTH_SHORT ).show();
					}
					// refresh state & ui
					if( helper != null ){

						helper.releaseServiceLock();
						helper.refreshUi( PrxmDeviceSettingActivity.this );
					}
					break;
				case PrxmMsg.MSG_ID_BT_PRXM_SET_PATHLOSS_CNF:
					// triggered by service (not MMI) when path loss occurs
					// TODO [L3] used to alert at Monitor side if necessary
					break;
				case PrxmMsg.MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF:
				case PrxmMsg.MSG_ID_BT_PRXM_GET_RSSI_CNF:
					pathLossThresholdPref.updateCurPathLoss( PrxmUtils.getPathLoss( deviceInfo.getCurrentRssi(), deviceInfo.getRemoteTxPower() ) );
					break;
				case PrxmMsg.MSG_ID_BT_PRXM_GET_CAPABILITY_CNF:
					// do nothing
					break;
				default:
					BtLog.e( "PrxmDeviceSettingActivity: unexpected resultCode:" + resultCode );
			}
		}
	};

	/**
	 * receive BluetoothAdapter.ACTION_STATE_CHANGED
	 */
	private IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);

	private BroadcastReceiver receiver = new BroadcastReceiver(){
		@Override
		public void onReceive( Context context, Intent intent ){
			int btState = intent.getIntExtra( BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR );

			BtLog.d( "onReceive: action[" + intent.getAction() + "] btState[" + btState + "]" );

			if( btState == BluetoothAdapter.STATE_TURNING_OFF ){
				finish();
			}
		}
	};

	private class FindDeviceThread extends Thread{
		private String m_bdaddr;
		private byte m_level; 
		private ServiceActivityHelper<IBluetoothPrxm> m_helper;

		public FindDeviceThread(String bdaddr, byte level, ServiceActivityHelper<IBluetoothPrxm> helper){
			m_bdaddr = bdaddr;
			m_level = level;
			m_helper = helper;
		}
			
		@Override
        	public void run() {
            		//for(int i = 0; i < 100; i++){
				try {
					this.m_helper.service.setPathLoss( m_bdaddr, m_level );
				}
				catch( Exception ex ){

					BtLog.e( "onPreferenceClick(): path_loss_level error.", ex );
				}
			//}
       		}
	};

	public boolean onPreferenceClick( Preference preference)
	{
		if( Options.LL_DEBUG ){

			BtLog.d( "onPreferenceClick:" );
		}
		// Find device preference
		if(preference == this.findDevicePref ){
			try {
				byte newLevel;
				if(findDeviceCanceled == true)
				{
					newLevel = PrxmConstants.PRXM_ALERT_LEVEL_HIGH;
					findDeviceCanceled = false;
					preference.setSummary(R.string.bt_prxm_find_device_cancel_summary );
				}
				else
				{
					newLevel = PrxmConstants.PRXM_ALERT_LEVEL_NO;
					findDeviceCanceled = true;
					preference.setSummary(R.string.bt_prxm_find_device_summary );
				}
				this.deviceInfo.setPathLossLevel(newLevel);
				//this.helper.service.setPathLoss( this.device.getAddress(), newLevel );
				this.helper.refreshUi( this );
				FindDeviceThread fdt = new FindDeviceThread(this.device.getAddress(), newLevel, this.helper);
				fdt.start();
				return true;	// sync call
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceClick(): path_loss_level error.", ex );
			}
		}
		return false;
	}

	/**
	 * implements Preference.OnPreferenceChangeListener
	 * 
	 * handle preference value change event (user action)
	 */
	public boolean onPreferenceChange( Preference preference, Object newValue ){

		if( Options.LL_DEBUG ){

			BtLog.d( "onPreferenceChange: new value[" + newValue + "]" );
		}

		// connect preference
		if( preference == this.connectPref ){

			try {
				// disable until cnf is back
				this.connectPref.setEnabled( false );
				this.helper.acquireServiceLock();
				this.helper.refreshUi( this );

				// call service
				if( (Boolean)newValue ){

					this.helper.service.connect( this.device.getAddress() );
				}
				else {
					this.helper.service.disconnect( this.device.getAddress() );
				}
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): connect/disconnect error.", ex );
			}
		}
		// Find device preference
		else if(preference == this.findDevicePref ){
			try {
				byte newLevel = PrxmConstants.PRXM_ALERT_LEVEL_HIGH;
				this.deviceInfo.setPathLossLevel(newLevel);
				this.helper.service.configPathLossLevel( this.device.getAddress(), newLevel );
				this.helper.refreshUi( this );
				return true;	// sync call
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): path_loss_level error.", ex );
			}
		}
		// link loss level preference
		else if( preference == this.linkLossLevelPref ){

			try {
				byte newLevel = Byte.parseByte( (String)newValue );
				if( newLevel != this.deviceInfo.getLinkLossLevel() ){

					this.helper.acquireServiceLock();
					this.helper.refreshUi( this );
					this.helper.service.setLinkLoss( this.device.getAddress(), newLevel );
				}
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): linkloss error.", ex );
			}
		}
		// path loss level preference
		else if( preference == this.pathLossLevelPref ){

			try {
				byte newLevel = Byte.parseByte( (String)newValue );
				if( newLevel != this.deviceInfo.getPathLossLevel() ){

					this.helper.service.configPathLossLevel( this.device.getAddress(), newLevel );
					this.helper.refreshUi( this );
					return true;	// sync call
				}
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): path_loss_level error.", ex );
			}
		}
		// path loss threshold preference
		else if( preference == this.pathLossThresholdPref ){

			try {
				byte newThreshold = ((Integer)newValue).byteValue();
				if( newThreshold != this.deviceInfo.getPathLossThreshold() ){

					this.helper.service.configPathLossThreshold( this.device.getAddress(), newThreshold );
					this.helper.refreshUi( this );
					return true;	// sync call
				}
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): path_loss_threshold error.", ex );
			}
		}

		// don't update UI until result is back
		return false;
	}

	/**
	 * get DeviceInfo from service
	 * 
	 * @return
	 */
	protected BluetoothPrxmDevice getDeviceInfo(){

		if( this.helper == null || this.helper.service == null || this.device == null ){

			return null;
		}

		try {
			return this.helper.service.getDeviceInfo( this.device.getAddress() );
		} 
		catch( Exception ex ){

			BtLog.e( "getDeviceInfo() error: ", ex );
			return null;
		}
	}

	@Override
	protected void onCreate( Bundle savedInstanceState ){

		BtLog.d( "onCreateBeforeBindService()[+]" );

		super.onCreate(savedInstanceState);

		this.registerReceiver( this.receiver, this.filter );

		// get device addr from Intent
		Intent intent = this.getIntent();
		this.device = intent.getParcelableExtra( BluetoothDevice.EXTRA_DEVICE );
		if( Options.LL_DEBUG ){

			BtLog.d( "selected device:[" + ((this.device==null)? "" : this.device.getName()) + "]" );
		}

		// create ui components
		this.addPreferencesFromResource( R.xml.bt_prxm_device_settings );

		// connect
		this.connectPref = (CheckBoxPreference)this.findPreference( "bt_prxm_connect" );
		this.connectPref.setOnPreferenceChangeListener(this);
		//find device
		this.findDevicePref = (Preference)this.findPreference( "bt_prxm_find_device" );		
		this.findDevicePref.setOnPreferenceClickListener(this);
		// link loss
		this.linkLossLevelPref = (ListPreference)this.findPreference( "bt_prxm_link_loss_level" );
		this.linkLossLevelPref.setOnPreferenceChangeListener(this);
		// path loss
		//this.pathLossGroup = (PreferenceGroup)this.findPreference( "bt_prxm_path_loss_group" );
		this.pathLossLevelPref = (ListPreference)this.findPreference( "bt_prxm_path_loss_level" );
		this.pathLossLevelPref.setOnPreferenceChangeListener(this);
		this.pathLossThresholdPref = (PrxmPathLossThresholdPreference)this.findPreference( "bt_prxm_path_loss_threshold" );
		this.pathLossThresholdPref.setParent( this );
		this.pathLossThresholdPref.setOnPreferenceChangeListener(this);
		//this.pathLossGroup.removePreference( this.pathLossThresholdPref );

		// register context menu
		this.registerForContextMenu( this.getListView() );

		// bind service
		this.helper = new ServiceActivityHelper<IBluetoothPrxm>( this );
		this.helper.bindService( this );
	}

	@Override
	public Dialog onCreateDialog( int id ){

		return this.helper.createBusyDialog( id, this );
	}

	@Override
	protected void onResume(){

		super.onResume();

		// update UI
		this.helper.refreshUi( this );
	}
	
	@Override
	protected void onDestroy(){

		BtLog.i( "onDestroy()[+]" );

		try {
			this.unregisterReceiver( this.receiver );
			this.helper.service.unregisterDeviceCallback( this.device.getAddress() );
		}
		catch( Exception ex ){
			
			BtLog.e( "unregister device callback failed: ", ex );
		}
		this.helper.unbindService( this );
		this.helper = null;
		this.device = null;
		super.onDestroy();
	}
}
