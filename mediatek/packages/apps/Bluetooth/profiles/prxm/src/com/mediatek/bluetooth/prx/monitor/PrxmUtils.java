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

import android.Manifest.permission;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.content.Context;
import android.content.Intent;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 */
public class PrxmUtils {

	/**
	 * PRXM
	 * @param context
	 * @param alertLevel
	 * @return
	 */
	public static String getAlertLevelString( Context context, byte alertLevel ){

		switch( alertLevel ){
			case PrxmConstants.PRXM_ALERT_LEVEL_NO:
				return context.getString( R.string.bt_prxm_alert_level_no );
			case PrxmConstants.PRXM_ALERT_LEVEL_MILD:
				return context.getString( R.string.bt_prxm_alert_level_mild );
			case PrxmConstants.PRXM_ALERT_LEVEL_HIGH:
				return context.getString( R.string.bt_prxm_alert_level_high );
			default:
				BtLog.e( "undefined alert level: " + alertLevel );
				return "Unknown";
		}
	}

	/**
	 * get the path-loss value (like distance)
	 * 
	 * @param rssi
	 * @param txPower
	 * @return value range: 0 ~ (PrxConstants.PRXM_PATH_LOSS_GRANULARITY-1)
	 */
	public static byte getPathLoss( byte rssi, byte txPower ){

		BtLog.d( "getPathLoss: rssi[" + rssi + "], txpower[" + txPower + " ]" );

		// TODO [L2] implement proper transform function for rssi
		int curDiff = txPower - rssi;
		curDiff = (curDiff>0) ? curDiff : 0;
		int maxDiff = txPower - PrxmConstants.PRXM_PATH_LOSS_MIN_RSSI;
		maxDiff = (maxDiff>0) ? maxDiff : 1;
		int pathLoss = curDiff * PrxmConstants.PRXM_PATH_LOSS_GRANULARITY / maxDiff;
		return (byte)( ( pathLoss >= PrxmConstants.PRXM_PATH_LOSS_GRANULARITY ) ? PrxmConstants.PRXM_PATH_LOSS_GRANULARITY-1 : pathLoss );
	}

	/**************************************************************************************************
	 * Profile Manager
	 **************************************************************************************************/

	public static void broadcastProfileManagerStateChanged( Context context, BluetoothDevice peerDevice, boolean isConnected ){

		Intent intent = new Intent( BluetoothProfileManager.ACTION_STATE_CHANGED );
		intent.putExtra( BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_PRXM );

		// state
		if( isConnected ){

			intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_CONNECTED);
			intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_DISCONNECTED);
		}
		else {
			intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_DISCONNECTED);
			intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_CONNECTED);
		}

		// device
		intent.putExtra( BluetoothDevice.EXTRA_DEVICE, peerDevice );
		context.sendBroadcast( intent, permission.BLUETOOTH );
	}

	public static final int STATE_ENABLING = BluetoothProfileManager.STATE_ENABLING;
	public static final int STATE_ENABLED = BluetoothProfileManager.STATE_ENABLED;
	public static final int STATE_DISABLED = BluetoothProfileManager.STATE_DISABLED;
	public static final int STATE_ABNORMAL = BluetoothProfileManager.STATE_ABNORMAL;
	public static void broadcastProfileManagerActivationState( Context context, int state ){

		Intent intent = new Intent( BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE );
		intent.putExtra( BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_PRXM );
		intent.putExtra( BluetoothProfileManager.EXTRA_NEW_STATE, state );
		context.sendBroadcast( intent, permission.BLUETOOTH );
	}
}
