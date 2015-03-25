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

import android.content.Context;
import android.preference.Preference;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * Preference for Proximity Reporter
 */
public class PrxmDevicePreference extends Preference {

	/**
	 * need to sync with drawable count: bt_prxm_device_path_loss_1.png ~ bt_prxm_device_path_loss_4.png
	 * bt_prxm_device_path_loss_0.png is for disconnect state
	 */
	private static final int PATH_LOSS_LEVEL = 4;

	private BluetoothPrxmDevice deviceInfo;	// info of corresponding device
	private TextView deviceName;		// ui component: device name
	private TextView deviceSetting;		// ui component: device setting description
	private ImageView devicePathLoss;	// ui component: device path loss icon

	/**
	 * Constructor
	 * 
	 * @param context
	 * @param deviceInfo
	 */
	public PrxmDevicePreference( Context context, BluetoothPrxmDevice deviceInfo ){

		super( context );

		// keep device info and setup layout
		this.deviceInfo = deviceInfo;
		this.setLayoutResource( R.layout.bt_prxm_device_preference );
	}

	@Override
	protected void onBindView( View view ){
		String displayName;

		super.onBindView( view );

		// keep component reference (instance will be changed when onBindView called)
		this.deviceName = (TextView)view.findViewById( R.id.bt_prxm_device_name );
		this.deviceSetting = (TextView)view.findViewById( R.id.bt_prxm_device_setting );
		this.devicePathLoss = (ImageView)view.findViewById( R.id.bt_prxm_device_path_loss );

		// device name
		displayName = this.deviceInfo.getName();
		if( displayName == null ){
			displayName = this.deviceInfo.getAddress();
			BtLog.d( "PrxmDevicePreference.onBindView(): name is null, use address instead (" + displayName + ")" );
		}
		this.deviceName.setText( displayName );

		// device setting
		String linkLossLevel = PrxmUtils.getAlertLevelString( this.getContext(), this.deviceInfo.getLinkLossLevel() );
		String pathLossLevel = PrxmUtils.getAlertLevelString( this.getContext(), this.deviceInfo.getPathLossLevel() );
		this.deviceSetting.setText( this.getContext().getString( R.string.bt_prxm_device_pref_device_setting, linkLossLevel, pathLossLevel ) );

		// device path loss icon: 0 for disconnect / 1~4 for signal strength
		byte pathloss = PrxmUtils.getPathLoss( deviceInfo.getCurrentRssi(), deviceInfo.getRemoteTxPower() );
		int level = ( this.deviceInfo.getCurrentState() != PrxmConstants.PRXM_STATE_CONNECTED ) ? 0 :
			(PATH_LOSS_LEVEL - (pathloss*PATH_LOSS_LEVEL/PrxmConstants.PRXM_PATH_LOSS_GRANULARITY));
		level = (level>PATH_LOSS_LEVEL) ? level = PATH_LOSS_LEVEL : level;
		this.devicePathLoss.getDrawable().setLevel( level );
	}

	/**
	 * handle DeviceInfo changed => update data model and UI
	 * 
	 * @param deviceInfo
	 */
	public void onDeviceAttributesChanged( BluetoothPrxmDevice deviceInfo ){

		// update model
		this.deviceInfo = deviceInfo;

		// notify change
		this.notifyChanged();
	}

	// default: sort by title
//	@Override
//	public int compareTo( Preference another ){
//
//		// return 1: this object will be after another
//		if( !(another instanceof PrxmDevicePreference) )	return 1;
//
//		// sort according to deviceInfo
//		return this.deviceInfo.compareTo( ((PrxmDevicePreference)another).getDeviceInfo() );
//	}

	public BluetoothPrxmDevice getDeviceInfo(){

		return deviceInfo;
	}
}
