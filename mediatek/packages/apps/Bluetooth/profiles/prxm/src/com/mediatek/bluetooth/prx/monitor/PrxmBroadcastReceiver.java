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

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * Receive broadcast from Bluetooth Device Picker (ACTION_DEVICE_SELECTED)
 *
 */
public class PrxmBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive( Context context, Intent intent ){

		String action = intent.getAction();

		BtLog.d( "ProximityMonitorReceiver.onReceive()[+]:" + action );

		/**
		 * from Bluetooth Device Picker (Explicit Intent)
		 */
		if( BluetoothDevicePicker.ACTION_DEVICE_SELECTED.equals( action ) ){

			BluetoothDevice selectedDevice = intent.getParcelableExtra( BluetoothDevice.EXTRA_DEVICE );

			if( selectedDevice == null ){

				BtLog.e( "ProximityMonitorReceiver.onReceive(): null selected device");
				return;
			}

			BtLog.d( "ProximityMonitorReceiver.onReceive(): selected-device[" + selectedDevice.getName() + "]" );

			// send Intent to Activity
			PrxmDeviceMgmtActivity.startConnectActivity( context, selectedDevice );

//		            Preference mPreference=new Preference(context);
//		            PreferenceCategory pc=new PreferenceCategory(context);
//		            pc = BluetoothHidActivity.getDeviceList();
//		            mPreference.setKey(remoteDevice.getAddress());
//					mPreference.setTitle(remoteDevice.getName());
//					String	state=BluetoothHidActivity.getmServerNotify().getStateByAddr(remoteDevice.getAddress());
//					if(state==null||state.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECT)||state.equals(BluetoothHid.BT_HID_DEVICE_UNPLUG_DISCONNECT))
//						mPreference.setSummary(R.string.bluetooth_hid_summary_not_connected);
//					else if(state.equals(BluetoothHid.BT_HID_DEVICE_CONNECT))
//						mPreference.setSummary(R.string.bluetooth_hid_summary_connected);
//					else if(state.equals(BluetoothHid.BT_HID_DEVICE_DISCONNECTING)||state.equals(BluetoothHid.BT_HID_DEVICE_UNPLUG)){
//						mPreference.setSummary(R.string.bluetooth_hid_summary_disconnecting);
//						mPreference.setEnabled(false);
//					}
//					else if(state.equals(BluetoothHid.BT_HID_DEVICE_CONNECTING)||state.equals(BluetoothHid.BT_HID_DEVICE_AUTHORIZE)){
//						mPreference.setSummary(R.string.bluetooth_hid_summary_connecting);
//						mPreference.setEnabled(false);
//					}
//					if(pc.findPreference(remoteDevice.getAddress()) == null)
//						pc.addPreference(mPreference);
		}
	}
}
