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

package com.mediatek.bluetooth.hid;

import static android.provider.Telephony.Intents.SECRET_CODE_ACTION;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.mediatek.xlog.Xlog;

public class BluetoothHidReceiver extends BroadcastReceiver {
    private static final String TAG = "[BT][HID][BluetoothHidReceiver]";

    private static final boolean DEBUG = true;

    @Override
    public void onReceive(Context context, Intent intent) {

        if (DEBUG) {
            Xlog.v(TAG, "HID broadcast receiver receives intent");
        }
        String action = intent.getAction();

        if (action.equals(BluetoothDevicePicker.ACTION_DEVICE_SELECTED)) {

            BluetoothDevice remoteDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            if (remoteDevice == null) {
                if (DEBUG) {
                    Xlog.e(TAG, "ERROR: remoteDevice null");
                }
                return;
            }

            if (DEBUG) {
                Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice.getAddress());
            }

            restorePreferenceList(context, remoteDevice);

        } else if (action.equals(SECRET_CODE_ACTION)) {
            Intent tmpIntent = new Intent(context, BluetoothHidActivity.class);
            tmpIntent.putExtra("BLUETOOTH_HID_PTS", "TRUE");
            tmpIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(tmpIntent);
        }

    }

    private void restorePreferenceList(Context context, BluetoothDevice remoteDevice) {
        SharedPreferences settings = context.getSharedPreferences(
                BluetoothHidActivity.BT_HID_SETTING_INFO, 0);

        int preferenceCount = settings.getInt("preferenceCount", 0);
        if (DEBUG) {
            Xlog.i(TAG, "restorePreferenceList,preferenceCount=" + preferenceCount);
        }
        int preferenceIndex = 0;
        for (preferenceIndex = 0; preferenceIndex < preferenceCount; preferenceIndex++) {
            String tmpAddr = settings.getString("deviceAddr" + Integer.toString(preferenceIndex),
                    BluetoothHidActivity.BT_HID_NOT_FOUNT);
            if (tmpAddr.equals(remoteDevice.getAddress())) {
                if (DEBUG) {
                    Xlog.i(TAG, "restorePreferenceList,already exist:" + remoteDevice.getName());
                }
                settings.edit().putString("newAdd" + Integer.toString(preferenceIndex), "TRUE").commit();
                return;
            }
        }
        preferenceCount++;

        settings.edit().putString("deviceAddr" + Integer.toString(preferenceIndex),
                remoteDevice.getAddress()).putString("newAdd" + Integer.toString(preferenceIndex),
                "TRUE").commit();
        settings.edit().putInt("preferenceCount", preferenceCount).commit();

        Intent tmpInt = new Intent(BluetoothHidActivity.ACTION_DEVICE_ADDED);
        context.sendBroadcast(tmpInt);

    }

}
