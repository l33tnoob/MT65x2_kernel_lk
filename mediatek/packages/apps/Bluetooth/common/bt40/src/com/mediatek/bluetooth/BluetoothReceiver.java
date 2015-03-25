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

package com.mediatek.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfileManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.mediatek.bluetooth.util.BtLog;

import java.util.HashSet;

public class BluetoothReceiver extends BroadcastReceiver {

    // used to keep the un-mounting sdcard
    public static HashSet<String> sUnmountingStorageSet = new HashSet<String>(2);

    // constants for show-toast feature
    public static final String ACTION_SHOW_TOAST = "com.mediatek.bluetooth.receiver.action.SHOW_TOAST";

    public static final String EXTRA_TEXT = "com.mediatek.bluetooth.receiver.extra.TEXT";

	private static final String ADVANCED_SERVICE_CLASS = "com.mediatek.bluetooth.AdvancedService";

    public static boolean isPathMounted(String path){
         boolean ret;
         synchronized(sUnmountingStorageSet){
             ret = !BluetoothReceiver.sUnmountingStorageSet.contains(path);
         }
         return ret;
    }


    @Override
    public void onReceive(Context context, Intent intent) {

        int btState;
        String action = intent.getAction();
        BtLog.i("BluetoothReceiver receive action:" + action);
        // Bluetooth On or Boot Completed => start profile services
        if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {	
            btState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
			BtLog.i("BluetoothReceiver get btState:" + btState);

			if (btState == BluetoothAdapter.STATE_ON) {

                //this.startProfileServices(context);
                this.startService(context, ADVANCED_SERVICE_CLASS);
            } else if (btState == BluetoothAdapter.STATE_TURNING_OFF) {

                //this.stopProfileServices(context);
            }
        } else if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            BtLog.i("BluetoothReceiver get ACTION_BOOT_COMPLETED BluetoothAdvancedService:");
            //this.startProfileServices(context);
            this.startService(context, ADVANCED_SERVICE_CLASS);
        } else if (action.equals(BluetoothProfileManager.ACTION_DISABLE_PROFILES)) {
            // ProfileManager notify to stop profile services
            BtLog.i("BluetoothProfileManaher disable profile");
            // this.stopProfileServices( context );
        } else if (ACTION_SHOW_TOAST.equals(action)) { // show toast

            String text = intent.getStringExtra(EXTRA_TEXT);
            if (text != null) {
                Toast.makeText(context, text, Toast.LENGTH_SHORT).show();
            }
        } else if (Intent.ACTION_MEDIA_EJECT.equals(action)) {

            Uri path = intent.getData();
            if (path != null) {
                if (Options.LL_DEBUG) {
                    BtLog.d("BluetoothReceiver: add un-mounting path[" + path.getPath() + "] for " + action);
                }
                synchronized(sUnmountingStorageSet) {
                    sUnmountingStorageSet.add(path.getPath());
                }
            }
        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action) || Intent.ACTION_MEDIA_MOUNTED.equals(action)) {

            Uri path = intent.getData();
            if (path != null) {
                if (Options.LL_DEBUG) {
                    BtLog.d("BluetoothReceiver: del un-mounting path[" + path.getPath() + "] for " + action);
                }
                synchronized(sUnmountingStorageSet) {
                    sUnmountingStorageSet.remove(path.getPath());
                }
            }
        }
    }

    private void startService(Context context, String serviceClass) {

	    try {

			BtLog.e("start service [" + serviceClass);
	    	context.startService(new Intent(context, Class.forName(serviceClass)));
	    } catch (ClassNotFoundException ex) {
	    	BtLog.e("start service for class[" + serviceClass + "] fail:", ex);
	    }
    }

}
