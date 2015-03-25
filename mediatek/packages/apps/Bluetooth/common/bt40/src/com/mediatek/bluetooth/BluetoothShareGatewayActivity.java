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

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import android.bluetooth.BluetoothUuid;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.os.UserHandle;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.bluetooth.BluetoothUuidEx;
import com.mediatek.bluetooth.BluetoothDevicePickerEx;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

public class BluetoothShareGatewayActivity extends Activity {
    private static final String TAG = "BluetoothShareGatewayActivity";

    private static final String KEY_INTENT = "intent";

    public static final String ACTION_DEVICE_SELECTED = "com.mediatek.bluetooth.sharegateway.action.DEVICE_SELECTED";

    public static final String EXTRA_DEVICE_ADDRESS = BluetoothDevice.EXTRA_DEVICE;

    public static final String ACTION_SETTINGS = "com.mediatek.bluetooth.sharegateway.action.ACTION_SETTINGS";

    private static final String ACTION_SEND_BIP_FILES = "com.mediatek.bluetooth.sharegateway.action.ACTION_SEND_BIP_FILES";

    public static final String ACTION_SEND = "com.mediatek.bluetooth.sharegateway.action.SEND";

    private static final int BLUETOOTH_DEVICE_REQUEST = 1;

    private static BluetoothAdapter sAdapter;

    private static Intent sIntent;

    private static String sType;

    private static boolean sBip;

    private static boolean sReentry = false;

    public static final ParcelUuid[] BIP_PROFILE_UUIDS = new ParcelUuid[] {
        BluetoothUuidEx.BipResponder
    };

    @Override
    public void onBackPressed() {
        if (isResumed()) {
            super.onBackPressed();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.v(TAG, "onCreate......");

        sReentry = false;

        Intent intent = getIntent();
        String action = intent.getAction();

        Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
        if (null == uri) {
            Xlog.e(TAG, "uri is null");
        } else {
            Xlog.v(TAG, "uri = " + uri.toString());
            sBip = false;
            if ("content".equals(uri.getScheme())) {
                if (MediaStore.AUTHORITY.equals(uri.getAuthority())) {
                    sBip = true;
                }
            }
        }

        ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);

        if (null == uris) {
            Xlog.e(TAG, "uris is null");
        } else {
            sBip = false;
            for (Uri tmpUri : uris) {
				if (null == tmpUri) {
					Xlog.e(TAG, "uri is null in Uris arraylist");
					finish();
					return;
				}
                Xlog.v(TAG, "uri = " + tmpUri.toString());
                if ("content".equals(tmpUri.getScheme())) {
                    if (MediaStore.AUTHORITY.equals(tmpUri.getAuthority())) {
                        sBip = true;
                    } else {
                        sBip = false;
                        break;
                    }
                } else {
                    sBip = false;
                    break;
                }
            }
        }

        if (Intent.ACTION_SEND.equals(action) || Intent.ACTION_SEND_MULTIPLE.equals(action)
                || BluetoothIntent.ACTION_HANDOVER_SEND.equals(action)
                || BluetoothIntent.ACTION_HANDOVER_SEND_MULTIPLE.equals(action)) {

            if (BluetoothIntent.ACTION_HANDOVER_SEND.equals(action)
                    || BluetoothIntent.ACTION_HANDOVER_SEND_MULTIPLE.equals(action)) {
                    sBip = false;
            }
            sType = intent.getType();
            Xlog.v(TAG, "sType = " + sType);
            sIntent = intent;

            sAdapter = BluetoothAdapter.getDefaultAdapter();
            if (sAdapter == null) {
                Xlog.e(TAG, "bluetooth is not started! ");
                finish();
                return;
            }

            if (sAdapter.isEnabled()) {
                Xlog.v(TAG, "bluetooth is available");

                BluetoothDevice remoteDevice = intent.getParcelableExtra(EXTRA_DEVICE_ADDRESS);
                Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice);

                if (null == remoteDevice) {
                    Xlog.i(TAG, "remote device is null");
                    startDevicePicker();
                } else {
                    profileDispatcher(remoteDevice);
                    finish();
                }
            } else {
                Xlog.w(TAG, "bluetooth is not available! ");
                Xlog.v(TAG, "turning on bluetooth......");

                Intent in = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                this.startActivityForResult(in, BLUETOOTH_DEVICE_REQUEST);
            }

        } else if (ACTION_DEVICE_SELECTED.equals(action)) {

            Xlog.v(TAG, "return from device picker");
            if (null == sIntent) {
                if (null != savedInstanceState) {
                    sIntent = savedInstanceState.getParcelable(KEY_INTENT);
                    sType = sIntent.getType();
                } else {
                    finish();
                    return;
                }
            }

            BluetoothDevice remoteDevice = intent.getParcelableExtra(EXTRA_DEVICE_ADDRESS);
            Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice);

            if (null == remoteDevice) {
                Xlog.e(TAG, "remote device is null");
            } else {
                profileDispatcher(remoteDevice);
                /*
                 * sIntent.putExtra(EXTRA_DEVICE_ADDRESS, remoteDevice); ParcelUuid[] uuids = remoteDevice.getUuids(); if (
                 * sBip == true && sType.startsWith("image") && BluetoothUuid.containsAnyUuid(uuids, BIP_PROFILE_UUIDS)) {
                 * Log.v(TAG, "BIP is supported"); sIntent.setClassName("com.mediatek.bluetooth",
                 * "com.mediatek.bluetooth.bip.BipInitEntryActivity"); } else { Log.v(TAG, "OPP is supported"); if(
                 * FeatureOption.MTK_BT_PROFILE_OPP ){ sIntent.setClassName("com.mediatek.bluetooth",
                 * "com.mediatek.bluetooth.opp.mmi.OppClientActivity"); } else { Toast.makeText( this,
                 * R.string.bt_base_profile_feature_disabled, Toast.LENGTH_SHORT ).show(); finish(); return; } }
                 * startActivity(mIntent);
                 */
            }

            finish();

        } else {
            Xlog.e(TAG, "unsupported action: " + action);
            finish();
        }
    }

    @Override
    public void onStart() {
        Xlog.v(TAG, "onStart......");
        super.onStart();

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putParcelable(KEY_INTENT, sIntent);
    }

    @Override
    public void onResume() {
        Xlog.v(TAG, "onResume......");
        super.onResume();
        if (sReentry) {
            Xlog.v(TAG, "onResume forget......");
            finish();
        }
    }

    @Override
    public void onPause() {
        Xlog.v(TAG, "onPause......");
        super.onPause();
        // finish();
        // sReentry = true;
    }

    @Override
    public void onStop() {
        Xlog.v(TAG, "onStop......");
        super.onStop();
        // finish();
        // sReentry = true;
    }

    @Override
    public void onDestroy() {
        Xlog.v(TAG, "onDestroy......");

        // unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == BLUETOOTH_DEVICE_REQUEST) {
            sReentry = false;
            if (Activity.RESULT_OK == resultCode) {
                // Bluetooth device is ready
                startDevicePicker();
                // finish();
            } else {
                this.finish();
            }
        }// BLUETOOTH_DEVICE_REQUEST end
    }

    private void startDevicePicker() {
        Intent in = new Intent(ACTION_SETTINGS);
        Bundle intentBundle = new Bundle();
        intentBundle.putBoolean("BipFlag", sBip);
        intentBundle.putString("Type", sType);
        intentBundle.putParcelable("Intent", sIntent);
        in.putExtras(intentBundle);
        sendBroadcastAsUser(in, UserHandle.ALL);
        Xlog.v(TAG, "Start Device Picker!");
        sReentry = true;

        Intent inToBDP = new Intent(BluetoothDevicePicker.ACTION_LAUNCH);
        inToBDP.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        inToBDP.putExtra(BluetoothDevicePicker.EXTRA_NEED_AUTH, false);
        inToBDP.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_PACKAGE, Options.APPLICATION_PACKAGE_NAME);
        inToBDP.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_CLASS, BluetoothShareGatewayReceiver.class.getName());
        //if (FeatureOption.MTK_BT_PROFILE_OPP) {
        //    inToBDP.putExtra(BluetoothDevicePicker.EXTRA_FILTER_TYPE, BluetoothDevicePicker.FILTER_TYPE_TRANSFER);
        //}
        if (FeatureOption.MTK_BT_PROFILE_BIP) {
            inToBDP.putExtra(BluetoothDevicePicker.EXTRA_FILTER_TYPE, BluetoothDevicePickerEx.FILTER_TYPE_BIP);
        }
        startActivity(inToBDP);
    }

    private void profileDispatcher(BluetoothDevice remoteDevice) {
        sIntent.putExtra(EXTRA_DEVICE_ADDRESS, remoteDevice);
        ParcelUuid[] uuids = remoteDevice.getUuids();

        if (FeatureOption.MTK_BT_PROFILE_BIP && sBip && sType.startsWith("image")
                && BluetoothUuid.containsAnyUuid(uuids, BIP_PROFILE_UUIDS)) {
            Xlog.v(TAG, "BIP is supported");
            Intent in = new Intent(ACTION_SEND_BIP_FILES);
            Bundle intentBundle = new Bundle();
            intentBundle.putParcelable("Intent", sIntent);
            in.putExtras(intentBundle);
            sendBroadcastAsUser(in, UserHandle.ALL);
        } else {
            //Xlog.v(TAG, "OPP is supported");
            //if (FeatureOption.MTK_BT_PROFILE_OPP) {
            //    sIntent.setClassName("com.mediatek.bluetooth", "com.mediatek.bluetooth.opp.mmi.OppClientActivity");
            //} else {
            //    Toast.makeText(this, R.string.bt_base_profile_feature_disabled, Toast.LENGTH_SHORT).show();
            //    return;
            //}
            //startActivity(sIntent);

			//Easy Migration Test, Advance proflie entry will disaptch opp task to default solution
			Xlog.v(TAG, "Disaptch to default OPP");
			sIntent.setClassName("com.android.bluetooth", "com.android.bluetooth.opp.BluetoothOppLauncherActivity");
            startActivity(sIntent);
        }
    }

}
