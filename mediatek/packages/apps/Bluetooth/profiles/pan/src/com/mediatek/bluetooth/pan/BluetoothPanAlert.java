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

package com.mediatek.bluetooth.pan;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothPan;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.bluetooth.R;
import com.mediatek.xlog.Xlog;

public class BluetoothPanAlert extends AlertActivity implements DialogInterface.OnClickListener {
    private static final int BLUETOOTHNOTIFYALERT = 1;

    private static final String TAG = "[BT][PAN][BluetoothPanAlert]";

    private TextView mContentView;

    String mDeviceName = new String();

    String mDeviceAddr = new String();

    String mAction = new String();

    private static boolean sOnlyOnce = true;

    private static final boolean DEBUG = true;

    private IBluetoothPanAction mServerNotify = null;

    private ServiceConnection mPanAction = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            mServerNotify = IBluetoothPanAction.Stub.asInterface(service);
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mServerNotify = null;
        }
    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String recAction = intent.getAction();
            if (recAction.equals(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothPan.EXTRA_STATE,
                        BluetoothPan.STATE_DISCONNECTED);
                if (state == BluetoothPan.STATE_DISCONNECTED) {
                    finish();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (DEBUG) {
            Xlog.i(TAG, "onCreate");
        }
        if (sOnlyOnce) {
            Bundle data = getIntent().getExtras();
            mDeviceAddr = (data != null) ? data.getString(BluetoothPan.DEVICE_ADDR) : null;
            mAction = (data != null) ? data.getString(BluetoothPan.ACTION) : null;
            if (DEBUG) {
                Xlog.i(TAG, "bluetoothPanAlert " + mDeviceAddr);
            }

            if (mDeviceAddr != null) {
                mDeviceName = getDeviceName(mDeviceAddr);
            }

            // Set up the "dialog"
            final AlertController.AlertParams p = mAlertParams;
            p.mIconId = android.R.drawable.ic_dialog_info;
            if (mAction != null) {
                if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE)) {
                    p.mTitle = getString(R.string.bluetooth_pan_GN_authorize_confirm_title);
                } else if (mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE)) {
                    p.mTitle = getString(R.string.bluetooth_pan_NAP_authorize_confirm_title);
                } else if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_CONNECTED)) {
                    p.mTitle = getString(R.string.bluetooth_pan_GN_connected_confirm_title);
                } else if (mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_CONNECTED)) {
                    p.mTitle = getString(R.string.bluetooth_pan_NAP_connected_confirm_title);
                }
                p.mView = createView();
            }

            p.mPositiveButtonText = getString(R.string.bluetooth_pan_yes);
            p.mPositiveButtonListener = this;
            p.mNegativeButtonText = getString(R.string.bluetooth_pan_no);
            p.mNegativeButtonListener = this;
            sOnlyOnce = false;
            setupAlert();

            registerReceiver(mReceiver, new IntentFilter(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED));
        } else {
            dismiss();
            cancel();
        }
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        if (DEBUG) {
            Xlog.i(TAG, "onStart");
        }
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        if (mBluetoothAdapter.getState() != BluetoothAdapter.STATE_TURNING_OFF) {
            this.bindService(new Intent(BluetoothPanAlert.this, BluetoothPanService.class), mPanAction,
                    Context.BIND_AUTO_CREATE);
        } else {
            finish();
        }
    }

    private View createView() {
        View view = getLayoutInflater().inflate(R.layout.pan_confirm_dialog, null);
        String text = new String();
        mContentView = (TextView) view.findViewById(R.id.content);
        if (mContentView != null) {
            if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE)) {
                text = this.getString(R.string.bluetooth_pan_GN_authorize_confirm, mDeviceName);
            } else if (mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE)) {
                text = this.getString(R.string.bluetooth_pan_NAP_authorize_confirm, mDeviceName);
            } else if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_CONNECTED)) {
                text = this.getString(R.string.bluetooth_pan_GN_connected_confirm, mDeviceName);
            } else if (mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_CONNECTED)) {
                text = this.getString(R.string.bluetooth_pan_NAP_connected_confirm, mDeviceName);
            }
            mContentView.setText(text);
        }

        return view;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        try {
            switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE)
                            || mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE)) {
                        mServerNotify.authorizeRspAction(mDeviceAddr, true);
                    } else if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_CONNECTED)
                            || mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_CONNECTED)) {
                        mServerNotify.disconnectPanDeviceAction(mDeviceAddr);
                    }
                    break;

                case DialogInterface.BUTTON_NEGATIVE:
                    // Update database
                    if (DEBUG) {
                        Xlog.i(TAG, "onClick:BUTTON_NEGATIVE");
                    }
                    if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE)
                            || mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE)) {
                        mServerNotify.authorizeRspAction(mDeviceAddr, false);
                    }
                    break;
                default:
                    break;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        sOnlyOnce = true;
        finish();
    }

    private String getDeviceName(String btAddr) {
        BluetoothDevice mBD = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(btAddr);

        if (mBD != null) {
            return mBD.getName();
        } else {
            return null;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (DEBUG) {
            Xlog.i(TAG, "onDestroy");
        }
        unregisterReceiver(mReceiver);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (DEBUG) {
            Xlog.i(TAG, "onPause");
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (DEBUG) {
            Xlog.i(TAG, "onStop:unbind pan service");
        }
        sOnlyOnce = true;
        this.unbindService(mPanAction);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_HOME) {
            if (mAction.equals(BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE)
                    || mAction.equals(BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE)) {
                try {
                    mServerNotify.authorizeRspAction(mDeviceAddr, false);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
            sOnlyOnce = true;
            finish();
        }
        return super.onKeyDown(keyCode, event);
    }

}
