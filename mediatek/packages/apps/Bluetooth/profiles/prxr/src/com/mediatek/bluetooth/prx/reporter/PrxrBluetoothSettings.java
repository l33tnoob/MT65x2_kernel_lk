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

package com.mediatek.bluetooth.prx.reporter;

import android.app.Dialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.widget.Toast;

import com.mediatek.activity.ServiceActivityHelper;
import com.mediatek.activity.AssembledPreferenceActivity.AssemblyPreference;
import com.mediatek.activity.ServiceActivityHelper.ServiceActivity;
import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.psm.ResultCode;
import com.mediatek.bluetooth.service.IBluetoothPrxr;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 */
public class PrxrBluetoothSettings implements ServiceActivity<IBluetoothPrxr>, AssemblyPreference, Preference.OnPreferenceChangeListener {

	private PreferenceActivity parentActivity;
	private ServiceActivityHelper<IBluetoothPrxr> saHelper;
	private CheckBoxPreference enablePref;	// ui components: enable/disable
	
	/**
	 * Constructor
	 */
	public PrxrBluetoothSettings(){

		this.saHelper = new ServiceActivityHelper<IBluetoothPrxr>(this);
	}
	
/******************************************************************************************
 * interface AssemblyPreference - BEG
 ******************************************************************************************/

	public int getPreferenceResourceId(){

		return R.xml.bt_prxr_settings;
	}

	public void onCreate( PreferenceActivity parentActivity ){

		BtLog.d("onCreate()[+]");
		// keep parent PreferenceActivity (Context)

		this.parentActivity = parentActivity;

		// keep ui component
		this.enablePref = (CheckBoxPreference)parentActivity.findPreference( "bt_prxr_settings_enable" );
		this.enablePref.setOnPreferenceChangeListener(this);

		// bind service
		this.saHelper.bindService( this.parentActivity );
	}

	public Dialog onCreateDialog( int id ){

		return this.saHelper.createBusyDialog( id, this.parentActivity );
	}

	public void onDestroy(){

		BtLog.d("onDestroy()[+]");

		// unregister callback before unbindService
		try {
			this.saHelper.service.unregisterCallback( this.callback );
		}
		catch( Exception ex ){

			BtLog.e( "unregisterCallback error: ", ex );
		}

		// unbind service
		try {
			this.saHelper.unbindService( this.parentActivity );
		}
		catch( Exception ex ){

			BtLog.e( "unbindService error: ", ex );
		}
		// this.enablePref = null;
		this.parentActivity = null;
	}

	public void onResume(){

		// refresh activity ui
		this.saHelper.refreshUi( this.parentActivity );
	}

	public void onRestoreInstanceState( Bundle savedInstanceState ){
	}
	public void onSaveInstanceState( Bundle outState ){
	}

/******************************************************************************************
 * interface AssemblyPreference - END
 ******************************************************************************************/

/******************************************************************************************
 * interface ServiceActivity<IPrxrService> - BEG
 ******************************************************************************************/

	public String getServiceAction(){

		return IBluetoothPrxr.class.getName();
	}
	public IBluetoothPrxr asInterface( IBinder service ){

		return IBluetoothPrxr.Stub.asInterface(service);
	}
	public void onServiceConnected(){

		BtLog.d( "onServiceConnected()[+]" );

		// register connection
		try {
			this.saHelper.service.registerCallback( callback );
		}
		catch( Exception ex ){

			BtLog.e( "PrxrBluetoothSettings.onServiceConnected(): register callback failed.", ex );
		}

		// release service lock and refresh ui
		this.saHelper.releaseServiceLock();
		this.saHelper.refreshUi( this.parentActivity );
	}
	public void onServiceDisconnected(){

		BtLog.d( "onServiceDisconnected()[+]" );

		try {
			this.saHelper.service.unregisterCallback( this.callback );
		}
		catch( Exception ex ){

			BtLog.e( "unregisterCallback error: ", ex );
		}

		// try to finish parent activity when service is unavailable
		this.parentActivity.finish();
	}
	/**
	 * update ui components according current stack state
	 */
	public void refreshActivityUi(){

		int currentState = -1;

		// update state from service
		if( this.saHelper.service != null ){

			try {
				currentState = this.saHelper.service.getServiceState();
			}
			catch( Exception ex ){

				BtLog.e( "getServiceSatate error.", ex );
			}
		}

		BtLog.d( "PrxrBluetoothSettings.refreshActivityUi() - currentState:" + currentState );
		// update ui according to current state
		switch( currentState ){
			case PrxrConstants.PRXR_STATE_NEW:
				// disabled
				this.enablePref.setEnabled(true);
				this.enablePref.setChecked(false);
				//this.enablePref.setSummary( R.string.bt_prxr_settings_enable_summary );
				break;
			case PrxrConstants.PRXR_STATE_REGISTERING:
			case PrxrConstants.PRXR_STATE_UNREGISTERING:
				// enabling / disabling
				this.enablePref.setEnabled(false);
				break;
			case PrxrConstants.PRXR_STATE_CONNECTABLE:
				// enabled
				this.enablePref.setEnabled(true);
				this.enablePref.setChecked(true);
				//this.enablePref.setSummary( "" );
				break;
			default:
				this.enablePref.setEnabled( false );
				BtLog.e( "invalid state: [" + currentState + "]" );
		}
	}

/******************************************************************************************
 * interface ServiceActivity<IPrxrService> - END
 ******************************************************************************************/

	/**
	 * handle callback (result) from service
	 */
	private ResultReceiver callback = new ResultReceiver( new Handler() ){

		@Override
		protected void onReceiveResult( int resultCode, Bundle resultData ){

			int rspcode;
			switch( resultCode ){
				// service is ready: connect / getCapability / getTxPower
				case PrxrMsg.MSG_ID_BT_PRXR_REGISTER_CNF:
					rspcode = resultData.getByte( "rspcode", ResponseCode.UNKNOWN );
					if( rspcode != ResponseCode.SUCCESS ){

						BtLog.i( "PrxrBluetoothSettings.onReceiveResult(): RXR_REGISTER_CNF fail with rspcode[" + rspcode + "]" );
						Toast.makeText( parentActivity, R.string.bt_prxr_settings_enable_fail, Toast.LENGTH_LONG ).show();
					}
					saHelper.releaseServiceLock();
					saHelper.refreshUi( parentActivity );
					break;
				case PrxrMsg.MSG_ID_BT_PRXR_DEREGISTER_CNF:
					saHelper.releaseServiceLock();
					saHelper.refreshUi( parentActivity );
					break;
				default:
					// e.g.	PrxrMsg.MSG_ID_BT_PRXR_CONNECT_IND 
					BtLog.w( "undefined resultCode:" + resultCode );
			}
		}
	};

	/**
	 * implements interface: Preference.OnPreferenceChangeListener
	 * handle preference value change event (user action)
	 */
	public boolean onPreferenceChange( Preference preference, Object newValue ){

		if( preference == this.enablePref ){

			try {
				BtLog.d( "Checkbox[enable]: new value[" + newValue + "]" );
				this.enablePref.setEnabled( false );

				// lock service and call enable or disable api
				this.saHelper.acquireServiceLock();
				int result = ResultCode.create( ResultCode.STATUS_FAILED, 0 );
				if( this.saHelper.service != null ){

					result = ( (Boolean)newValue )
						? this.saHelper.service.enableService()
						: this.saHelper.service.disableService();
				}
				else {
					BtLog.e( "PrxrBluetoothSettings access service(null) failed." );
				}

				// handle error
				if( ResultCode.status( result ) == ResultCode.STATUS_FAILED ){

					BtLog.e( "PRXR enable/disable error: [" + newValue + "]" );
					this.saHelper.releaseServiceLock();
					this.enablePref.setEnabled( true );
					Toast.makeText( this.parentActivity, R.string.bt_prxr_settings_enable_fail, Toast.LENGTH_SHORT );
				}			
			}
			catch( Exception ex ){

				BtLog.e( "onPreferenceChange(): enable/disable error.", ex );
			}
		}

		// don't update UI until result is back
		return false;
	}
}
