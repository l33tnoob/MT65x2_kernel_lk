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

package com.mediatek.bluetooth.avrcp;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;

public class BluetoothAvrcpReceiver extends BroadcastReceiver {
    public static final String TAG = "AVRCP";

    public static BluetoothAvrcpService sAvrcpServer = null;

    // Add JNI part
    private int mNativeData;

    @Override
    public void onReceive(Context context, Intent intent) {
        String textMessage;
        String action = intent.getAction();
        String data;
        textMessage = "[BT][AVRCP] onReceive ".concat(action);

        Log.v(TAG, textMessage);
/*
        if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
            int state = 0;
            state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
            if (BluetoothAdapter.STATE_ON == state || BluetoothAdapter.STATE_OFF == state) {

                Intent in = new Intent();
                in.putExtras(intent);
                in.setClass(context, BluetoothAvrcpService.class);
                in.putExtra("action", action);

                context.startService(in);
            }
        }

        if (action.equals("android.provider.Telephony.SECRET_CODE")) {
            data = intent.getDataString();
            Log.v(TAG, "[BT][AVRCP] Get the securty code (" + action.toString() + ")");
            if (null != data) {
                Log.v(TAG, "[BT][AVRCP] Get the securty code data: (" + data + ")");
            }

            textMessage = "AVRCP PTS enable mode (Source:Telephone)";
            if (data != null && data.indexOf("2872710") != -1) {
                initalConnect("00:00:00:00:00:00");
                textMessage = "AVRCP PTS connect mode (Source:Telephone)";
            } else {
                BluetoothAvrcpService.sPTSDebugMode = 3;
            }
            Toast.makeText(context, textMessage, Toast.LENGTH_SHORT).show();
        }
        if (action.equals("android.mediatek.bluetooth.avrcp.pts")) {
            Log.v(TAG, "Get the avrcp.pts code");
            BluetoothAvrcpService.sPTSDebugMode = 3;
            textMessage = "AVRCP PTS enable mode (Source:pts action)";
            Toast.makeText(context, textMessage, Toast.LENGTH_SHORT).show();
        }
        if (action.equals("android.mediatek.bluetooth.avrcp.connect")) {
            Log.v(TAG, "Get the avrcp.connect code");
            textMessage = "AVRCP PTS connect (Source: action)";
            Toast.makeText(context, textMessage, Toast.LENGTH_SHORT).show();
            initalConnect("00:00:00:00:00:00"); // use the latest a2dp addr 
        }
        if (action.equals("android.mediatek.bluetooth.avrcp.disconnect")) {
            Log.v(TAG, "Get the avrcp.disconnect code");
            if (null != sAvrcpServer) {
                textMessage = "AVRCP PTS disconnect (Source: action)";
                Toast.makeText(context, textMessage, Toast.LENGTH_SHORT).show();
                sAvrcpServer.disconnectNative();
            }
        }
*/        
    }

    public void destroyMyself(BluetoothAvrcpService server) {
        if (server == sAvrcpServer) {
            Log.v(TAG, "destroyMyself");
        }
    }

    public void initalConnect(String sAddr) {
        if (null != sAvrcpServer) {
            Log.v(TAG, "AVRCP initConnect connectReqNative used!");
            sAvrcpServer.connectReqNative(sAddr);
        } else {
            Log.v(TAG, "AVRCP initConnect fail !!! no mAvrcpServer");
        }
    }

}
