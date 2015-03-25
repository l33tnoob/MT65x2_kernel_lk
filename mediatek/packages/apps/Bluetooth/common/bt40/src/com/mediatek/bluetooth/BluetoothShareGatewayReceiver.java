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

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.bluetooth.BluetoothUuid;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.os.UserHandle;
import android.widget.Toast;

import com.mediatek.bluetooth.share.BluetoothShareMgmtActivity;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

public class BluetoothShareGatewayReceiver extends BroadcastReceiver {

    private static final String TAG = "BluetoothShareGatewayReceiver";

    // define by Android default OPP & Settings apps
    private static final String ACTION_OPEN_RECEIVED_FILES = "android.btopp.intent.action.OPEN_RECEIVED_FILES";

    private static final String ACTION_SEND_BIP_FILES = "com.mediatek.bluetooth.sharegateway.action.ACTION_SEND_BIP_FILES";

    private static String sType;

    private static boolean sBip;

    private static Intent sIntent;

    @Override
    public void onReceive(Context context, Intent intent) {

        Xlog.v(TAG, "BSG broadcast receiver receives intent");

        String action = intent.getAction();

        if (action.equals(BluetoothDevicePicker.ACTION_DEVICE_SELECTED)) {

            BluetoothDevice remoteDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice);

            /*
             * Log.v(TAG, "SEND broadcast to gateway"); Intent inBSG = new
             * Intent(BluetoothShareGatewayActivity.ACTION_DEVICE_SELECTED);
             * inBSG
             * .putExtra(BluetoothShareGatewayActivity.EXTRA_DEVICE_ADDRESS,
             * remoteDevice); inBSG.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
             * Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
             * //context.sendBroadcast(inBSG); context.startActivity(inBSG);
             */
            if (null != remoteDevice) {
                profileDispatcher(context, remoteDevice);
            }
        } else if (action.equals(ACTION_OPEN_RECEIVED_FILES)) {

            Intent openReceivedIntent = new Intent(context, BluetoothShareMgmtActivity.class);
            openReceivedIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(openReceivedIntent);
        } else if (action.equals(BluetoothShareGatewayActivity.ACTION_SETTINGS)) {
            Bundle bundle = intent.getExtras();
            if (null != bundle) {
                sType = bundle.getString("Type");
                sBip = bundle.getBoolean("BipFlag", false);
                sIntent = bundle.getParcelable("Intent");
                if (sIntent == null) {
                    Xlog.v(TAG, "sIntent == null");
                }
            }

            Xlog.v(TAG, "sType = " + sType + "; sBip = " + sBip);
        }
    }

    private void profileDispatcher(Context context, BluetoothDevice remoteDevice) {
        sIntent.putExtra(BluetoothShareGatewayActivity.EXTRA_DEVICE_ADDRESS, remoteDevice);
        ParcelUuid[] uuids = remoteDevice.getUuids();

        if (FeatureOption.MTK_BT_PROFILE_BIP
                && sBip
                && sType.startsWith("image")
                && BluetoothUuid.containsAnyUuid(uuids,
                        BluetoothShareGatewayActivity.BIP_PROFILE_UUIDS)) {
            Xlog.v(TAG, "BIP is supported");
            // sIntent.setClassName("com.mediatek.bluetooth",
            // "com.mediatek.bluetooth.bip.BipInitEntryActivity");
            // BipInitEntry sBipInitEntry = new BipInitEntry(context, sIntent);
            Intent in = new Intent(ACTION_SEND_BIP_FILES);
            Bundle intentBundle = new Bundle();
            intentBundle.putParcelable("Intent", sIntent);
            in.putExtras(intentBundle);
            context.sendBroadcastAsUser(in, UserHandle.ALL);
        } else {
            Xlog.v(TAG, "OPP is supported");
            if (FeatureOption.MTK_BT_PROFILE_OPP) {
                sIntent.setClassName("com.mediatek.bluetooth",
                        "com.mediatek.bluetooth.opp.mmi.OppClientActivity");
            } else {
                Toast.makeText(context, R.string.bt_base_profile_feature_disabled,
                        Toast.LENGTH_SHORT).show();
                return;
            }
            sIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(sIntent);
        }
    }
}
