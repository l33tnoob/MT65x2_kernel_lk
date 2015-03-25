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

/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.connectivity;

import java.lang.reflect.Method;
import java.util.ArrayList;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.widget.TextView;

public class CdsTetherSrvActivity extends Activity {
    private static final String TAG = "CDSINFO/CdsTetheringSrvActivity";
    private static final String ERROR_STRING = "Command Error";
    private static final String CRLF = "\r\n----------------------\r\n";


    private static final String ACTION_TETHER_STATE_CHANGED =
        "android.net.conn.TETHER_STATE_CHANGED";
    private static final String EXTRA_AVAILABLE_TETHER = "availableArray";
    private static final String EXTRA_ACTIVE_TETHER = "activeArray";
    private static final String EXTRA_ERRORED_TETHER = "erroredArray";
    private static final String USB_CONNECTED = "connected";
    private static final String ACTION_USB_STATE = "android.hardware.usb.action.USB_STATE";
    private static final String ACTION_MEDIA_UNSHARED = "android.intent.action.MEDIA_UNSHARED";

    private TextView mTetherInfo;
    private ConnectivityManager mConnMgr;
    private Context mContext;
    private BroadcastReceiver mTetherChangeReceiver;
    private boolean mUsbConnected;
    private boolean mMassStorageActive;
    private boolean mBluetoothEnableForTether;

    private String[] mUsbRegexs;
    private String[] mWifiRegexs;
    private String[] mBluetoothRegexs;


    /** {@hide} */
    public static final int TETHER_ERROR_NO_ERROR           = 0;
    /** {@hide} */
    public static final int TETHER_ERROR_UNKNOWN_IFACE      = 1;
    /** {@hide} */
    public static final int TETHER_ERROR_SERVICE_UNAVAIL    = 2;
    /** {@hide} */
    public static final int TETHER_ERROR_UNSUPPORTED        = 3;
    /** {@hide} */
    public static final int TETHER_ERROR_UNAVAIL_IFACE      = 4;
    /** {@hide} */
    public static final int TETHER_ERROR_MASTER_ERROR       = 5;
    /** {@hide} */
    public static final int TETHER_ERROR_TETHER_IFACE_ERROR = 6;
    /** {@hide} */
    public static final int TETHER_ERROR_UNTETHER_IFACE_ERROR = 7;
    /** {@hide} */
    public static final int TETHER_ERROR_ENABLE_NAT_ERROR     = 8;
    /** {@hide} */
    public static final int TETHER_ERROR_DISABLE_NAT_ERROR    = 9;
    /** {@hide} */
    public static final int TETHER_ERROR_IFACE_CFG_ERROR      = 10;

    @Override
    public void onCreate(Bundle icicle) {
        Method method = null;
        super.onCreate(icicle);

        setContentView(R.layout.cds_tether_info);

        mContext = this.getBaseContext();
        if(mContext == null) {
            Xlog.e(TAG, "Could not get Conext of this activity");
        }

        mConnMgr = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        if(mConnMgr == null) {
            Xlog.e(TAG, "Could not get Connectivity Manager");
            return;
        }

        try {
            method =  mConnMgr.getClass().getMethod("getTetherableUsbRegexs");
            mUsbRegexs = (String[]) method.invoke(mConnMgr);
            method =  mConnMgr.getClass().getMethod("getTetherableWifiRegexs");
            mWifiRegexs = (String[]) method.invoke(mConnMgr);
            method =  mConnMgr.getClass().getMethod("getTetherableBluetoothRegexs");
            mBluetoothRegexs = (String[]) method.invoke(mConnMgr);
        } catch(Exception e) {
            mUsbRegexs = null;
            mWifiRegexs = null;
            mBluetoothRegexs = null;
            e.printStackTrace();
        }

        mTetherInfo   = (TextView) findViewById(R.id.tether_info);
        Xlog.i(TAG, "CdsTetheringSrvActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateTetherSrvInfo();
    }

    private void updateTetherSrvInfo() {
        Method method = null;
        String infoString = "";
        String tetherString = "";
        String[] tetherInterfaces;
        int i = 0;

        try {

            method =  mConnMgr.getClass().getMethod("isTetheringSupported");
            Boolean isTetherSupported = (Boolean) method.invoke(mConnMgr);
            if(isTetherSupported) {
                infoString = "Tethering function is supported" + CRLF;
            } else {
                infoString = "Tethering function is non-supported";
                mTetherInfo.setText(infoString);
                return;
            }

            method =  mConnMgr.getClass().getMethod("getTetherableUsbRegexs");
            tetherInterfaces = (String[]) method.invoke(mConnMgr);
            for(i =0 ; i < tetherInterfaces.length; i++) {
                tetherString = tetherString + tetherInterfaces[i].replace("\\d", "") + " ";
            }
            infoString = infoString + "USB tethering interfaces: " + tetherString + CRLF;

            method =  mConnMgr.getClass().getMethod("getTetherableWifiRegexs");
            tetherInterfaces = (String[]) method.invoke(mConnMgr);
            tetherString = "";
            for(i =0 ; i < tetherInterfaces.length; i++) {
                tetherString = tetherString + tetherInterfaces[i].replace("\\d", "") + " ";
            }
            infoString = infoString + "Wifi tethering interface: " + tetherString + CRLF;

            method =  mConnMgr.getClass().getMethod("getTetherableBluetoothRegexs");
            tetherInterfaces = (String[]) method.invoke(mConnMgr);
            tetherString = "";
            for(i =0 ; i < tetherInterfaces.length; i++) {
                tetherString = tetherString + tetherInterfaces[i].replace("\\d", "") + " ";
            }
            infoString = infoString + "Bluetooth tethering interface: " + tetherString + CRLF;

            mTetherInfo.setText(infoString);
            Xlog.i(TAG, "updateTetherSrvInfo Done");


        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    private void updateState() {
        Method method = null;
        String[] available, tethered, errored;

        try {
            method = mConnMgr.getClass().getMethod("getTetherableIfaces");
            available = (String[]) method.invoke(mConnMgr);

            method = mConnMgr.getClass().getMethod("getTetheredIfaces");
            tethered = (String[]) method.invoke(mConnMgr);

            method = mConnMgr.getClass().getMethod("getTetheringErroredIfaces");
            errored = (String[]) method.invoke(mConnMgr);

            updateState(available, tethered, errored);
        } catch(Exception e) {
            e.printStackTrace();
        }

    }

    private void updateState(String[] available, String[] tethered,
                             String[] errored) {
        updateUsbState(available, tethered, errored);
        //updateBluetoothState(available, tethered, errored);
    }


    private void updateUsbState(String[] available, String[] tethered,
                                String[] errored) {
        // TODO Auto-generated method stub
        Method method;
        boolean usbAvailable = mUsbConnected && !mMassStorageActive;
        int usbError = TETHER_ERROR_NO_ERROR;


        try {
            for (String s : available) {
                for (String regex : mUsbRegexs) {
                    if (s.matches(regex)) {
                        if (usbError == TETHER_ERROR_NO_ERROR) {
                            method = mConnMgr.getClass().getMethod("getLastTetherError");
                            usbError = (Integer) method.invoke(mConnMgr);
                        }
                    }
                }
            }
        } catch(Exception e) {
            e.printStackTrace();
        }

        boolean usbTethered = false;
        for (String s : tethered) {
            for (String regex : mUsbRegexs) {
                if (s.matches(regex)) usbTethered = true;
            }
        }
        boolean usbErrored = false;
        for (String s: errored) {
            for (String regex : mUsbRegexs) {
                if (s.matches(regex)) usbErrored = true;
            }
        }

        /*
        if (usbTethered) {
            mUsbTether.setSummary(R.string.usb_tethering_active_subtext);
        } else if (usbAvailable) {
            if (usbError == TETHER_ERROR_NO_ERROR) {
                mUsbTether.setSummary(R.string.usb_tethering_available_subtext);
            } else {
                mUsbTether.setSummary(R.string.usb_tethering_errored_subtext);
            }
        } else if (usbErrored) {
            mUsbTether.setSummary(R.string.usb_tethering_errored_subtext);
        } else if (mMassStorageActive) {
            mUsbTether.setSummary(R.string.usb_tethering_storage_active_subtext);
        } else {
            mUsbTether.setSummary(R.string.usb_tethering_unavailable_subtext);
        }
        */
    }

    @Override
    public void onPause() {
        super.onPause();


    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();

        mMassStorageActive = Environment.MEDIA_SHARED.equals(Environment.getExternalStorageState());
        mTetherChangeReceiver = new TetherChangeReceiver();
        IntentFilter filter = new IntentFilter(ACTION_TETHER_STATE_CHANGED);
        Intent intent = registerReceiver(mTetherChangeReceiver, filter);

        filter = new IntentFilter();
        filter.addAction(ACTION_USB_STATE);
        registerReceiver(mTetherChangeReceiver, filter);

        filter = new IntentFilter();
        filter.addAction(Intent.ACTION_MEDIA_SHARED);
        filter.addAction(ACTION_MEDIA_UNSHARED);
        filter.addDataScheme("file");
        registerReceiver(mTetherChangeReceiver, filter);

        /*
        filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mTetherChangeReceiver, filter);
        */

        if (intent != null) mTetherChangeReceiver.onReceive(mContext, intent);

        updateState();

    }

    private class TetherChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();


            if (action.equals(ACTION_TETHER_STATE_CHANGED)) {
                // TODO - this should understand the interface types
                ArrayList<String> available = intent.getStringArrayListExtra(
                                                  EXTRA_AVAILABLE_TETHER);
                ArrayList<String> active = intent.getStringArrayListExtra(
                                               EXTRA_ACTIVE_TETHER);
                ArrayList<String> errored = intent.getStringArrayListExtra(
                                                EXTRA_ERRORED_TETHER);
                updateState(available.toArray(new String[available.size()]),
                            active.toArray(new String[active.size()]),
                            errored.toArray(new String[errored.size()]));
            } else if (action.equals(Intent.ACTION_MEDIA_SHARED)) {
                mMassStorageActive = true;
                updateState();
            } else if (action.equals(ACTION_MEDIA_UNSHARED)) {
                mMassStorageActive = false;
                updateState();
            } else if (action.equals(ACTION_USB_STATE)) {
                mUsbConnected = intent.getBooleanExtra(USB_CONNECTED, false);
                updateState();
            }
        }
    }


    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();

        unregisterReceiver(mTetherChangeReceiver);
        mTetherChangeReceiver = null;
    }

}
