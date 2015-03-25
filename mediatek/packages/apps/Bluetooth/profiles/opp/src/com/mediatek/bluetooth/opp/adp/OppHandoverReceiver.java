/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.bluetooth.opp.adp;

import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.mediatek.bluetooth.BluetoothShareGatewayActivity;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;
import com.mediatek.xlog.Xlog;

public class OppHandoverReceiver extends BroadcastReceiver {
    public static final String TAG = "OppHandoverReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {

        String action = intent.getAction();

        if (action.equals(OppConstants.ACTION_WHITELIST_DEVICE)) {

            BluetoothDevice device = (BluetoothDevice) intent
                    .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            if (device != null) {
                Xlog.d(TAG, "add new device into white list:" + device.getAddress());

                OppManager.getInstance(context).addToWhitelist(device.getAddress());
            }
        } else if (action.equals(OppConstants.ACTION_STOP_HANDOVER)) {
            int id = intent.getIntExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_ID, -1);
            if (id == -1) {
                Uri contentUri = Uri.parse(BluetoothShareTaskMetaData.CONTENT_URI + "/" + id);
                // send abort request to OppManager
                OppManager.getInstance(context).oppAbortTask(contentUri);
            }
        } else if (OppConstants.ACTION_HANDOVER_SEND.equals(action)
                || OppConstants.ACTION_HANDOVER_SEND_MULTIPLE.equals(action)) {
            intent.setClassName("com.mediatek.bluetooth", BluetoothShareGatewayActivity.class
                    .getName());
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            context.startActivity(intent);
        } else {
            Xlog.d(TAG, "Unknown action: " + action);
        }
    }
}
