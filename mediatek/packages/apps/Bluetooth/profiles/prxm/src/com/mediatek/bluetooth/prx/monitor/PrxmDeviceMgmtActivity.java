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
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import com.mediatek.bluetooth.BluetoothDevicePickerEx;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.Handler.Callback;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;

import com.mediatek.activity.ServiceActivityHelper;
import com.mediatek.activity.ServiceActivityHelper.ServiceActivity;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.service.IBluetoothPrxm;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * entry point for Proximity Monitor ( Device Management )
 * 
 * TODO [L2] auto connect device while it's added (block UI for single Preference).
 */
public class PrxmDeviceMgmtActivity extends PreferenceActivity implements ServiceActivity<IBluetoothPrxm>, Callback {

	public static final String ACTION_START   = "com.mediatek.bluetooth.prx.monitor.device.mgmt.action.START";
	public static final String ACTION_CONNECT = "com.mediatek.bluetooth.prx.monitor.device.mgmt.action.CONNECT";

	private static final int BLUETOOTH_DEVICE_ENABLE_REQUEST = 1;
	private static int mRequestCode = BLUETOOTH_DEVICE_ENABLE_REQUEST;
	// ui component id
	private static final String ADD_DEVICE = "add_device";
	private static final String REGISTERED_DEVICE_LIST = "registered_device_list";

	// message id for handler
	private static final byte MESSAGE_REFRESH_UI = 0;

	// ui refresh cycle time
	private static final int REFRESH_UI_DELAY = 2500;

	// handler for ui update
	private Handler handler;

	// device list
	private PreferenceCategory registeredDeviceList;

	// activity service helper
	private ServiceActivityHelper<IBluetoothPrxm> helper;

	/**
	 * implements ServiceActivity
	 */
	public String getServiceAction(){
		return IBluetoothPrxm.class.getName();
	}
	/**
	 * implements ServiceActivity
	 */
	public IBluetoothPrxm asInterface( IBinder service ){
		return IBluetoothPrxm.Stub.asInterface(service);
	}
	/**
	 * implements ServiceActivity
	 */
	public void onServiceConnected(){

		BtLog.d( "PrxmDeviceMgmtActivity.onServiceConnected()[+]" );
		
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
	 * implements ServiceActivity
	 */
	public void refreshActivityUi(){

		try {
			// update data from service
			BluetoothPrxmDevice[] deviceList = this.helper.service.getRegisteredDevices();

			// TODO [L3] implement sorting algorithm here can prevent UI change continuously
			//this.notifyHierarchyChanged();	// This could affect ordering, so notify that also

			// update ui components
			int size = this.registeredDeviceList.getPreferenceCount();
			int min = ( size > deviceList.length ) ? deviceList.length : size;
			// update device preference
			for( int i=0; i<min; i++ ){

				((PrxmDevicePreference)this.registeredDeviceList.getPreference(i)).onDeviceAttributesChanged( deviceList[i] );
			}
			if( size > deviceList.length ){

				// remote device preference
				for( int i=deviceList.length; i<size; i++ ){

					this.registeredDeviceList.removePreference( this.registeredDeviceList.getPreference(i) );
				}
			}
			else {
				// insert device preference
				for( int i=size; i<deviceList.length; i++ ){

					this.registeredDeviceList.addPreference( new PrxmDevicePreference( this, deviceList[i] ) );
				}
			}
		}
		catch( Exception ex ){

			BtLog.e( "PrxmDeviceMgmtActivity.refreshUi() error: ", ex );
		}
	}

	/**
	 * receive BluetoothAdapter.ACTION_STATE_CHANGED
	 */
	private IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);

	private BroadcastReceiver receiver = new BroadcastReceiver(){
		@Override
		public void onReceive( Context context, Intent intent ){
			int btState = intent.getIntExtra( BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR );

			BtLog.d( "onReceive: action[" + intent.getAction() + "] btState[" + btState + "]" );

			if( btState == BluetoothAdapter.STATE_TURNING_OFF || btState == BluetoothAdapter.STATE_OFF ){
				finish();
			}
		}
	};

	@Override
	public Dialog onCreateDialog( int id ){

		BtLog.d( "PrxmDeviceMgmtActivity.onCreateDialog()[+]" );
		return this.helper.createBusyDialog( id, this );
	}

	@Override
	protected void onCreate( Bundle savedInstanceState ){

		BtLog.d( "PrxmDeviceMgmtActivity.onCreate()[+]" );

		super.onCreate(savedInstanceState);

		// check bt status
		BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
		if( btAdapter == null ){

			BtLog.e("bt is not supported! ");
			Toast.makeText( this, R.string.bt_prxm_device_mgmt_bt_not_support, Toast.LENGTH_SHORT ).show();
			this.finish();
			return;
		}

		// create required object
		this.handler = new Handler( this );
		this.helper = new ServiceActivityHelper<IBluetoothPrxm>(this);

		if( !btAdapter.isEnabled() ){

			BtLog.d( "Bluetooth is not enabled, turning on..." );

			Intent in = new Intent( BluetoothAdapter.ACTION_REQUEST_ENABLE );
            in.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
			this.startActivityForResult( in, mRequestCode);
            mRequestCode += 1;

			// wait for result
			return;
		}
		else {
			// Bluetooth is ready and activity can start
			this.onCreateImpl();
		}
	}

	@Override
	protected void onActivityResult( int requestCode, int resultCode, Intent data ){
		 BtLog.d( "onActivityResult" + requestCode + " :" + resultCode);
		if( requestCode != mRequestCode - 1 )	return;

		if( resultCode == RESULT_OK ){

			// Bluetooth is ready and activity can start
			this.onCreateImpl();
		}
		else {

			BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
			if( btAdapter != null && btAdapter.isEnabled() ){
				this.onCreateImpl();
			} else {
				BtLog.e("BT is not ready!");
				Toast.makeText( this, R.string.bt_prxm_device_mgmt_bt_not_ready, Toast.LENGTH_SHORT ).show();
				this.finish();
			}
		}
	} 

	/**
	 * Activity onCreate() implementation
	 */
	private void onCreateImpl(){

		BtLog.d( "PrxmDeviceMgmtActivity.onCreateImpl()[+]" );

		// init ui components
		this.setTitle( R.string.bt_prxm_device_mgmt_title );
		this.addPreferencesFromResource( R.xml.bt_prxm_device_mgmt );
		this.registeredDeviceList = (PreferenceCategory)this.findPreference( REGISTERED_DEVICE_LIST );
		this.registerForContextMenu( this.getListView() );

		this.registerReceiver( receiver, filter );

		// bind service
		this.helper.bindService( this );
	}
	
	@Override
	protected void onDestroy(){

		// unbind service
		if( this.helper.service != null ){

			this.helper.unbindService( this );
		}

		// release resource
		this.helper = null;
		this.handler = null;
		this.registeredDeviceList = null;

		try {
			this.unregisterReceiver( receiver );
		} catch( Exception ex ){
			// just ignore it.
		}

		super.onDestroy();
	}

	@Override
	protected void onResume(){

		super.onResume();

		// update UI
		this.helper.refreshUi( this );

		// start monitoring reporters periodically
		this.handler.sendEmptyMessage( MESSAGE_REFRESH_UI );
	}

	@Override
	protected void onPause(){

		// stop monitoring reporters
		this.handler.removeMessages( MESSAGE_REFRESH_UI );

		super.onPause();
	}

	@Override
	protected void onNewIntent( Intent intent ){

		if( Options.LL_DEBUG ){

			BtLog.d( "PrxmDeviceMgmtActivity.onNewIntent()" );
		}
		super.onNewIntent(intent);

		String action = intent.getAction();
		if( ACTION_CONNECT.equals( action ) ){

			// register specific bluetooth device
			BluetoothDevice remoteDevice = intent.getParcelableExtra( BluetoothDevice.EXTRA_DEVICE );
			if( remoteDevice == null ){

				BtLog.e( "invalid intent(action=connect): device is null" );
				return;
			}
			try {
				this.helper.service.registerDevice( remoteDevice );
			}
			catch( Exception ex ){

				BtLog.e( "remote service (registerDevice) error: ", ex );
			}
		}
	}

	@Override
	public boolean onPreferenceTreeClick( PreferenceScreen preferenceScreen, Preference preference ){

		// if user click 'add device'
		if( ADD_DEVICE.equals( preference.getKey() ) ){

			// start device picker to select device
			Intent intent = new Intent( BluetoothDevicePicker.ACTION_LAUNCH );
			intent.setFlags( Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS );
			intent.putExtra( BluetoothDevicePicker.EXTRA_NEED_AUTH, false );
			// Add device filter for proximity reporter
			// 1. add UUID for PRX in android.bluetooth.BluetoothUuid: Proximity
			// 2. add Class for PRX in android.bluetooth.BluetoothClass: ?
			// 3. add ParcelUuid for PRX in com.android.settings.bluetooth.LocalBluetoothProfileManager: PRX_PROFILE_UUIDS
			// 4. add filtering function in com.android.settings.bluetooth.BluetoothSettings (addDevicePreference)
			// 5. add filter type in android.bluetooth.BluetoothDevicePicker: FILTER_TYPE_PRX
			// intent.putExtra( BluetoothDevicePicker.EXTRA_FILTER_TYPE, BluetoothDevicePickerEx.FILTER_TYPE_PRX );
			intent.putExtra( BluetoothDevicePicker.EXTRA_LAUNCH_PACKAGE, Options.APPLICATION_PACKAGE_NAME );
			intent.putExtra( BluetoothDevicePicker.EXTRA_LAUNCH_CLASS, PrxmBroadcastReceiver.class.getName() );
			this.startActivity(intent);
			return true;
		}
		// if user click 'device'
		if( preference instanceof PrxmDevicePreference ){

			// selected proximity-reporter
			PrxmDevicePreference prxmPreference = (PrxmDevicePreference)preference;
			if( Options.LL_DEBUG ){

				BtLog.d( "selected device:" + prxmPreference.getDeviceInfo().getName() );
			}

			// start DeviceSettingActivity to handle device config
			Intent intent = new Intent( PrxmDeviceSettingActivity.ACTION_START );
			intent.putExtra( BluetoothDevice.EXTRA_DEVICE, prxmPreference.getDeviceInfo().getDevice() );
			this.startActivity( intent );
			return true;
		}
		return super.onPreferenceTreeClick( preferenceScreen, preference );
	}

	@Override
	public void onCreateContextMenu( ContextMenu menu, View v, ContextMenuInfo menuInfo ){

		// get device from menu
		BluetoothDevice device = this.getDeviceFromMenuInfo( menuInfo );
		if( device == null )	return;

		// menu header title
		menu.setHeaderTitle( device.getName() );

		// unregister
		menu.add( 0, Menu.FIRST+1, 0, R.string.bt_prxm_device_mgmt_remove_device );
	}
	@Override
	public boolean onContextItemSelected( MenuItem item ){

		// get device from menu
		BluetoothDevice device = this.getDeviceFromMenuInfo( item.getMenuInfo() );
		if( device == null )	return false;

		try {
			// unregister device
			this.helper.service.unregisterDevice( device );
			return true;
		}
		catch( Exception ex ){

			BtLog.e( "PrxmDeviceMgmtActivity.onContextItemSelected() error:", ex );
			return false;
		}
	}

	private BluetoothDevice getDeviceFromMenuInfo( ContextMenuInfo menuInfo ){

		// check and cast to PrxmDevicePreference
		if( menuInfo == null || !(menuInfo instanceof AdapterContextMenuInfo) )	return null;
		Preference pref = (Preference)getPreferenceScreen().getRootAdapter().getItem( ((AdapterContextMenuInfo)menuInfo).position );
		if( pref == null || !(pref instanceof PrxmDevicePreference) )	return null;
		return ((PrxmDevicePreference)pref).getDeviceInfo().getDevice();
	}
	
	/**
	 * handle callback from ui handler
	 */
	public boolean handleMessage( Message msg ){

		switch( msg.what ){

			case MESSAGE_REFRESH_UI:
				if( this.helper != null ){
					this.helper.refreshUi( this );
					this.handler.sendEmptyMessageDelayed( MESSAGE_REFRESH_UI, REFRESH_UI_DELAY );
				}
				return true;
			default:
				return false;	// not handled by callback => will be handed by Handler
		}
	}

	protected static void startConnectActivity( Context context, BluetoothDevice device ){

		// try to send intent to onNewIntent (this Activity must be running)
		Intent intent = new Intent( PrxmDeviceMgmtActivity.ACTION_CONNECT );
		intent.setClass( context, PrxmDeviceMgmtActivity.class );
		intent.setFlags( Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS );
		intent.putExtra( BluetoothDevice.EXTRA_DEVICE, device );
		context.startActivity( intent );
	}
}
