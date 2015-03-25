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

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import static android.provider.Telephony.Intents.SECRET_CODE_ACTION;

public class BluetoothSimapReceiver extends BroadcastReceiver {

    private static final String TAG = "BluetoothSimapReceiver";

    private static final boolean V = BluetoothSimapService.VERBOSE;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (V) Log.v(TAG, "SimapReceiver onReceive: " + intent.getAction());

        String action = intent.getAction();		
		int btState;

		/*
		if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
	    	BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();

	    	if (btAdapter != null 
				&& btAdapter.getState()!= BluetoothAdapter.STATE_OFF
				&& btAdapter.getState()!= BluetoothAdapter.STATE_TURNING_OFF) {		
				Log.i(TAG, "ACTION_BOOT_COMPLETED, and btAdapter.isEnabled, startService: SIMAP");
				context.startService(new Intent(context, BluetoothSimapService.class));
	    	}
		}
		else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
			btState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
		
			switch(btState) {
			case BluetoothAdapter.STATE_TURNING_ON:	
				Log.i(TAG, "Receive BluetoothAdapter.STATE_ON, startService: SIMAP ");
				context.startService(new Intent(context, BluetoothSimapService.class));
				break;
		
			default:
				break;
			}
		}
		else 
		*/
		if (action.equals(BluetoothSimapService.ACTION_CLEAR_AUTH_NOTIFICATION)) {
			Intent intent2 = new Intent(BluetoothSimapService.ACCESS_DISALLOWED_ACTION);
			context.sendBroadcast(intent2);
		}
		else if (action.equals(BluetoothSimapService.ACTION_CLEAR_CONN_NOTIFICATION)) {
		}
		else if (action.equals(SECRET_CODE_ACTION)) {
			Intent intent2 = new Intent(BluetoothSimapService.SEND_SIMUNAVALIBLE_IND);
			context.sendBroadcast(intent2);

		}
    }
}

