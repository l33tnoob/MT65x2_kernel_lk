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

import com.mediatek.bluetooth.util.BtLog;

import android.Manifest.permission;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.content.Context;
import android.content.Intent;

/**
 * @author Jerry Hsu
 */
public class PrxrUtils {

	/**************************************************************************************************
	 * Profile Manager
	 **************************************************************************************************/

	public static void broadcastProfileManagerStateChanged( Context context, BluetoothDevice peerDevice, boolean isConnected ){

		BtLog.d( "broadcastProfileManagerStateChanged()[+]: peer[" + peerDevice + "], connected[" + isConnected + "]" );

		Intent intent = new Intent( BluetoothProfileManager.ACTION_STATE_CHANGED );
		intent.putExtra( BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_PRXR );

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
		context.sendBroadcast(intent, permission.BLUETOOTH);
	}

	public static final int STATE_ENABLING = BluetoothProfileManager.STATE_ENABLING;
	public static final int STATE_ENABLED = BluetoothProfileManager.STATE_ENABLED;
	public static final int STATE_DISABLED = BluetoothProfileManager.STATE_DISABLED;
	public static final int STATE_ABNORMAL = BluetoothProfileManager.STATE_ABNORMAL;
	public static void broadcastProfileManagerActivationState( Context context, int state ){

		BtLog.d( "broadcastProfileManagerActivationState()[+]: state[" + state + "]" );

		Intent intent = new Intent( BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE );
		intent.putExtra( BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_PRXR );
		intent.putExtra( BluetoothProfileManager.EXTRA_NEW_STATE, state );
		context.sendBroadcast( intent, permission.BLUETOOTH );
	}
}
