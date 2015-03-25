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

package com.mediatek.bluetooth.opp.adp;

import android.Manifest.permission;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothOpp;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.bluetooth.IBluetoothOpp;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.UserHandle;

import com.mediatek.bluetooth.opp.mmi.OppLog;
import com.mediatek.bluetooth.share.BluetoothShareTask;

public class BluetoothOppService extends Service {

    private BluetoothOppClientImpl mOppClient = null;

    private BluetoothOppServerImpl mOppServer = null;

    @Override
    public IBinder onBind(Intent intent) {

        String action = intent.getAction();
        OppLog.i("BluetoothOppService.onBind()[+]: " + action);

        if (BluetoothOpp.Client.class.getName().replaceAll("\\$", ".").equals(action)) {

            if (this.mOppClient == null) {

                this.initOppClient();
            }
            return mOppClient;
        } else if (BluetoothOpp.Server.class.getName().replaceAll("\\$", ".").equals(action)) {

            if (this.mOppServer == null) {

                this.initOppServer();
            }
            return mOppServer;
        }
        return null;
    }

    private synchronized void initOppClient() {

        if (this.mOppClient == null) {

            this.mOppClient = new BluetoothOppClientImpl(this);
        }
    }

    private synchronized void initOppServer() {

        if (this.mOppServer == null) {

            this.mOppServer = new BluetoothOppServerImpl(this);
        }
    }

    public static void sendStateChangedBroadcast(Context context, BluetoothShareTask task,
            boolean isConnect) {

        Intent intent = new Intent(BluetoothProfileManager.ACTION_STATE_CHANGED);
        if (task.isOppcTask()) {

            intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE,
                    BluetoothProfileManager.Profile.Bluetooth_OPP_Client);
        } else {
            intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE,
                    BluetoothProfileManager.Profile.Bluetooth_OPP_Server);
        }
        if (isConnect) {

            intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                    BluetoothProfileManager.STATE_CONNECTED);
            intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE,
                    BluetoothProfileManager.STATE_DISCONNECTED);
        } else {
            intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE,
                    BluetoothProfileManager.STATE_DISCONNECTED);
            intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE,
                    BluetoothProfileManager.STATE_CONNECTED);
        }

        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, BluetoothAdapter.getDefaultAdapter()
                .getRemoteDevice(task.getPeerAddr()));
        context.sendBroadcastAsUser(intent, UserHandle.ALL, permission.BLUETOOTH);
    }

    public static final int STATE_ENABLING = BluetoothProfileManager.STATE_ENABLING;

    public static final int STATE_ENABLED = BluetoothProfileManager.STATE_ENABLED;

    public static final int STATE_DISABLED = BluetoothProfileManager.STATE_DISABLED;

    public static final int STATE_ABNORMAL = BluetoothProfileManager.STATE_ABNORMAL;

    public static void sendActivationBroadcast(Context context, boolean isOppc, int state) {

        Profile profile = isOppc ? Profile.Bluetooth_OPP_Client : Profile.Bluetooth_OPP_Server;

        Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
        intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, profile);
        intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
        context.sendBroadcastAsUser(intent, UserHandle.ALL, permission.BLUETOOTH);
    }
}

class BluetoothOppClientImpl extends IBluetoothOpp.Stub {

    private Context mContext;

    private OppManager mManager;

    public BluetoothOppClientImpl(Context context) {

        this.mContext = context;
        this.mManager = OppManager.getInstance(this.mContext);
    }

    public void disconnect(BluetoothDevice device) throws RemoteException {

        this.mManager.oppAbortDeviceTasks(device.getAddress());
    }

    public BluetoothDevice getConnectedDevice() throws RemoteException {

        Uri uri = this.mManager.oppcGetCurrentTask();
        return this.mManager.oppQueryTaskDevice(uri);
    }

    public int getState() throws RemoteException {

        if (this.mManager.oppcGetCurrentTask() != null) {

            return BluetoothProfileManager.STATE_CONNECTED;
        } else {

            return BluetoothProfileManager.STATE_DISCONNECTED;
        }
    }
}

class BluetoothOppServerImpl extends IBluetoothOpp.Stub {

    private Context mContext;

    private OppManager mManager;

    public BluetoothOppServerImpl(Context context) {

        this.mContext = context;
        this.mManager = OppManager.getInstance(this.mContext);
    }

    public void disconnect(BluetoothDevice device) throws RemoteException {

        this.mManager.oppAbortDeviceTasks(device.getAddress());
    }

    public BluetoothDevice getConnectedDevice() throws RemoteException {

        Uri uri = this.mManager.oppsGetCurrentTask();
        return this.mManager.oppQueryTaskDevice(uri);
    }

    public int getState() throws RemoteException {

        if (this.mManager.oppsGetCurrentTask() != null) {

            return BluetoothProfileManager.STATE_CONNECTED;
        } else {

            return BluetoothProfileManager.STATE_DISCONNECTED;
        }
    }
}
