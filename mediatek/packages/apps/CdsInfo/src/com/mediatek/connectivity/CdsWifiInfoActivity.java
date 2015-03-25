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
 * Copyright (C) 2009 The Android Open Source Project
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

import com.mediatek.connectivity.R;
import android.net.wifi.ScanResult;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import java.util.List;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import java.net.UnknownHostException;
import android.os.SystemProperties;
import android.os.IBinder ;
import android.os.ServiceManager;
import java.util.Random;
import java.util.StringTokenizer;
import java.text.NumberFormat;
import java.text.DecimalFormat;
import android.widget.CheckBox;

/**
 * Show the current status details of Wifi related fields
 */
public class CdsWifiInfoActivity extends Activity {

    private static final String TAG = "CDSINFO/WifiInfo";
    
    private static final int MAC_ADDRESS_ID = 30;
    private static final int MAC_ADDRESS_DIGITS = 6;
    private static final int MAX_ADDRESS_VALUE = 0xff;
    private static final int INVALID_RSSI = -200;
        

    private static final String[] WIFI_SYSTEM_PROPERTY = new String[]{
        "net.hostname",
        "dhcp.wlan0.ipaddress",
        "net.dns1",
        "net.dns2",
        "dhcp.wlan0.leasetime",
        "dhcp.wlan0.gateway",
        "dhcp.wlan0.mask",
        "dhcp.wlan0.dns1",
        "dhcp.wlan0.dns2",
        "dhcp.wlan0.dns3",
        "dhcp.wlan0.dns4",
        "init.svc.dhcpcd_wlan0",
        "wlan.driver.status",
        "wifi.interface",
        "dhcp.wlan0.pid",
        "dhcp.wlan0.server",
        "dhcp.wlan0.reason",
        "dhcp.wlan0.result",
        "mediatek.wlan.ctia"};

    private Button   mUpdateButton;
    private Button   mScanButton;
    private TextView mWifiState;
    private TextView mNetworkState;
    private TextView mSupplicantState;
    private TextView mRSSI;
    private TextView mBSSID;
    private TextView mSSID;
    private TextView mHiddenSSID;
    private TextView mIPAddr;
    private TextView mMACAddr;
    private TextView mNetworkId;
    private TextView mLinkSpeed;
    private TextView mScanList;
    private TextView mSystemProperties;
    
    private TextView mMacAddrLabel;
    private EditText mMacAddrEdit;
    private Button   mMacAddBtn;
    private short[]  mRandomMacAddr;

    private Toast mToast;

    private TextView mPingIpAddr;
    private TextView mPingHostname;
    private TextView mHttpClientTest;
    private Button mPingTestButton;
    
    private static String MacAddressRandom = "";
    private boolean mTestMode;

    private String mPingIpAddrResult;
    private String mPingHostnameResult;
    private String mHttpClientTestResult;

    private String mDns1 = "";
    private String mDns2 = "";

    private int mPingHostType = 0;

    private WifiManager mWifiManager;
    private IntentFilter mWifiStateFilter;

    //poor link
    private CheckBox mCbProfiling = null;
    private TextView mPoorLinkGoodLabel;
    private EditText mPoorLinkGoodEdit;
    private TextView mPoorLinkBadLabel;
    private EditText mPoorLinkBadEdit;
    private boolean mProfilingInfo = false;
    private Button   mPoorLinkAddBtn;


    //============================
    // Activity lifecycle
    //============================

    private final BroadcastReceiver mWifiStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                handleWifiStateChanged(intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                                       WifiManager.WIFI_STATE_UNKNOWN));
            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                handleNetworkStateChanged(
                    (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO));
            } else if (intent.getAction().equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)) {
                handleScanResultsAvailable();
            } else if (intent.getAction().equals(WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION)) {
                /* TODO: handle supplicant connection change later */
            } else if (intent.getAction().equals(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION)) {
                handleSupplicantStateChanged(
                    (SupplicantState) intent.getParcelableExtra(WifiManager.EXTRA_NEW_STATE),
                    intent.hasExtra(WifiManager.EXTRA_SUPPLICANT_ERROR),
                    intent.getIntExtra(WifiManager.EXTRA_SUPPLICANT_ERROR, 0));
            } else if (intent.getAction().equals(WifiManager.RSSI_CHANGED_ACTION)) {
                handleSignalChanged(intent.getIntExtra(WifiManager.EXTRA_NEW_RSSI, 0));
            } else if (intent.getAction().equals(WifiManager.NETWORK_IDS_CHANGED_ACTION)) {
                /* TODO: handle network id change info later */
            } else {
                Xlog.e(TAG, "Received an unknown Wifi Intent");
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mWifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

        mWifiStateFilter = new IntentFilter(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        mWifiStateFilter.addAction(WifiManager.SUPPLICANT_STATE_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        mWifiStateFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        

        setContentView(R.layout.wifi_status_test);

        mUpdateButton = (Button) findViewById(R.id.update);
        mUpdateButton.setOnClickListener(mUpdateButtonHandler);

        mScanButton = (Button) findViewById(R.id.scan);
        mScanButton.setOnClickListener(mScanButtonHandler);

        mWifiState = (TextView) findViewById(R.id.wifi_state);
        mNetworkState = (TextView) findViewById(R.id.network_state);
        mSupplicantState = (TextView) findViewById(R.id.supplicant_state);
        mRSSI = (TextView) findViewById(R.id.rssi);
        mBSSID = (TextView) findViewById(R.id.bssid);
        mSSID = (TextView) findViewById(R.id.ssid);
        mHiddenSSID = (TextView) findViewById(R.id.hidden_ssid);
        mIPAddr = (TextView) findViewById(R.id.ipaddr);
        mMACAddr = (TextView) findViewById(R.id.macaddr);
        mNetworkId = (TextView) findViewById(R.id.networkid);
        mLinkSpeed = (TextView) findViewById(R.id.link_speed);
        mScanList = (TextView) findViewById(R.id.scan_list);

        mSystemProperties = (TextView) findViewById(R.id.system_property);
        
        mMacAddrLabel = (TextView) findViewById(R.id.mac_label);
        mMacAddrEdit = (EditText) findViewById(R.id.macid);
        mMacAddBtn = (Button) findViewById(R.id.mac_update_btn);
        mMacAddBtn.setOnClickListener(mMacEditButtonHandler);

        mPingIpAddr = (TextView) findViewById(R.id.pingIpAddr);
        mPingHostname = (TextView) findViewById(R.id.pingHostname);
        mHttpClientTest = (TextView) findViewById(R.id.httpClientTest);

        mPingTestButton = (Button) findViewById(R.id.ping_test);
        mPingTestButton.setOnClickListener(mPingButtonHandler);

        RadioGroup mPingRadioGroup = (RadioGroup)findViewById(R.id.pingRadioGroup);

        mPingRadioGroup.setOnCheckedChangeListener( new RadioGroup.OnCheckedChangeListener() {
            public void onCheckedChanged(RadioGroup group, final int checkedId) {
                mPingHostType = checkedId;
            }
        });

        mTestMode = SystemProperties.get("ro.build.type").equals("eng");

        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        //poor link
        initPoorLink();
    
    }

    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mWifiStateReceiver, mWifiStateFilter);
        refreshWifiStatus();
        initPoorLink();
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(mWifiStateReceiver);
    }

    OnClickListener mPingButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            updatePingState();
        }
    };

    OnClickListener mScanButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            try{
                mWifiManager.startScan();
            }catch(Exception e){
                e.printStackTrace();
            }
        }
    };

    OnClickListener mUpdateButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            refreshWifiStatus();
        }
    };

    OnClickListener mMacEditButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            updateMacAddr();
        }
    };

    OnClickListener mPoorLinkButtonHandler = new OnClickListener() {
        public void onClick(View v) {
            Xlog.v(TAG, "mPoorLinkButtonHandler click ");
            updatePooorLinkInfo();
        }
    };

    private void setProfilingInfo(boolean state){
         Xlog.v(TAG, "setProfilingInfo " + state);
         mProfilingInfo = state;
         if(mProfilingInfo==false)SystemProperties.set("persist.sys.poorlinkProfile","0");
         else SystemProperties.set("persist.sys.poorlinkProfile","1");
    }

    private void updatePooorLinkInfo(){

        Xlog.i(TAG, "updatePooorLinkInfo in");
        
        boolean isPoorLinkDetectEnabled = SystemProperties.getBoolean("persist.sys.poorlinkEnable",false) ;
        if(isPoorLinkDetectEnabled==false){
            Xlog.i(TAG, "updatePoorLink isPoorLinkDetectEnabled= "+isPoorLinkDetectEnabled);
            mPoorLinkGoodLabel.setVisibility(View.GONE);
            mPoorLinkGoodEdit.setVisibility(View.GONE);
            mPoorLinkBadLabel.setVisibility(View.GONE);
            mPoorLinkBadEdit.setVisibility(View.GONE);
            mPoorLinkAddBtn.setVisibility(View.GONE); 
            mCbProfiling.setVisibility(View.GONE);
            return;
        }
        //update isProfiling          
         boolean newState = mCbProfiling.isChecked();
         Xlog.v(TAG, "mCbProfiling.isChecked : " + newState);
         mProfilingInfo = newState;

         setProfilingInfo(mProfilingInfo);
         mWifiManager.setPoorLinkProfilingOn(mProfilingInfo);

         //update poor link/good link threshold
         double plink = Double.parseDouble(mPoorLinkBadEdit.getText().toString());
         double glink = Double.parseDouble(mPoorLinkGoodEdit.getText().toString());
          
         if(glink>plink || glink>1.0 || glink<0 ||plink>1.0 || plink<0 ){
            Xlog.v(TAG,   "setPoorLinkThreshold, fail good= "+glink +" poor ="+ plink);
            mToast.setText("Invalid threshold value good= "+ glink+" poor="+plink);
            mToast.show();
            return ;
         }
         mWifiManager.setPoorLinkThreshold(true, glink);
         mWifiManager.setPoorLinkThreshold(false, plink);
         mToast.setText("Update Success. Please restart WiFi");
         mToast.show();
    }

    private void initPoorLink(){
        mPoorLinkGoodLabel = (TextView) findViewById(R.id.poorlink_good_label);
        mPoorLinkGoodEdit = (EditText) findViewById(R.id.poorlink_goodvalue);
        mPoorLinkBadLabel = (TextView) findViewById(R.id.poorlink_bad_label);
        mPoorLinkBadEdit = (EditText) findViewById(R.id.poorlink_badvalue);
        mPoorLinkAddBtn = (Button) findViewById(R.id.poorlink_update_btn);
        mPoorLinkAddBtn.setOnClickListener(mPoorLinkButtonHandler);
        mCbProfiling = (CheckBox) findViewById(R.id.poorlink_Profiling_Screen);


        boolean isPoorLinkDetectEnabled = SystemProperties.getBoolean("persist.sys.poorlinkEnable",false) ;
        if(isPoorLinkDetectEnabled==false){
            Xlog.i(TAG, "poor link function disable no show poor link option");
            mPoorLinkGoodLabel.setVisibility(View.GONE);
            mPoorLinkGoodEdit.setVisibility(View.GONE);
            mPoorLinkBadLabel.setVisibility(View.GONE);
            mPoorLinkBadEdit.setVisibility(View.GONE);
            mPoorLinkAddBtn.setVisibility(View.GONE); 
            mCbProfiling.setVisibility(View.GONE);
            return;
        }


        mPoorLinkGoodLabel.setVisibility(View.VISIBLE);
        mPoorLinkGoodEdit.setVisibility(View.VISIBLE);
        mPoorLinkBadLabel.setVisibility(View.VISIBLE);
        mPoorLinkBadEdit.setVisibility(View.VISIBLE);
        mPoorLinkAddBtn.setVisibility(View.VISIBLE);        

        double poorlinkGood = mWifiManager.getPoorLinkThreshold(true);
        double poorlinkPoor = mWifiManager.getPoorLinkThreshold(false);

        Xlog.i(TAG, "getPoorLink poorlinkGood= "+poorlinkGood +" poorlinkPoor= "+poorlinkPoor);

        boolean isprofilingEnable = false;
        if(SystemProperties.getBoolean("persist.sys.poorlinkProfile",false) == true){
            isprofilingEnable =true;
        }

        mPoorLinkGoodEdit.setText(poorlinkGood+"");
        mPoorLinkBadEdit.setText(poorlinkPoor+"");


        
        if(isprofilingEnable==true){
            mCbProfiling.setChecked(true);
            setProfilingInfo(true);
        }else{
            mCbProfiling.setChecked(false);
            setProfilingInfo(false);
        }
        mCbProfiling.setVisibility(View.VISIBLE);

        mCbProfiling.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbProfiling.isChecked();
                Xlog.v(TAG, "mCbProfiling.setOnClickListener : " + newState);
             //   mProfilingInfo = newState;
             //   setProfilingInfo(mProfilingInfo);
            }
        });
    }

    private void getMacAddr(){
        
        
        try{
            IBinder binder=ServiceManager.getService("NvRAMAgent");
            NvRAMAgent agent = NvRAMAgent.Stub.asInterface (binder);
                        
            mRandomMacAddr = new short[MAC_ADDRESS_DIGITS];

            if(!mTestMode){
                mMacAddrLabel.setVisibility(View.GONE);
                mMacAddrEdit.setVisibility(View.GONE);
                mMacAddBtn.setVisibility(View.GONE);
            }else{                
                    StringBuilder sb = new StringBuilder();
                    Random rand = new Random();
                    NumberFormat formatter = new DecimalFormat("00");
                    int end1 = rand.nextInt(100);
                    int end2 = rand.nextInt(100);
                    String num1 = formatter.format(end1);
                    String num2 = formatter.format(end2);
                    
                    sb.append("00:08:22:11:");
                    sb.append(num1).append(":").append(num2);                    

                    mMacAddrLabel.setVisibility(View.VISIBLE);
                    mMacAddrEdit.setVisibility(View.VISIBLE);
                    mMacAddBtn.setVisibility(View.VISIBLE);
                    System.out.println("string buffer:" + sb);
                    mMacAddrEdit.setText(sb);
                    MacAddressRandom = sb.toString();
                
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }

    private void updateMacAddr(){
                
        try{
            int i = 0;
            IBinder binder=ServiceManager.getService("NvRAMAgent");
            NvRAMAgent agent = NvRAMAgent.Stub.asInterface (binder);
            
            //parse mac address firstly
            StringTokenizer txtBuffer = new StringTokenizer(mMacAddrEdit.getText().toString(), ":");
            while (txtBuffer.hasMoreTokens()){
                mRandomMacAddr[i] = (short) Integer.parseInt(txtBuffer.nextToken(), 16);
                System.out.println(i + ":" + mRandomMacAddr[i]);
                i++;
            }
            if(i != 6){
               mToast.setText("The format of mac address is not correct");
               mToast.show();
               return;
            }

            byte[] buff = null;
            try{
                buff = agent.readFile(MAC_ADDRESS_ID);
            }catch(Exception e){
                e.printStackTrace();
            }
                                                
            for(i = 0; i < MAC_ADDRESS_DIGITS;i ++){
                buff[i+4] = (byte) mRandomMacAddr[i];
            }
            
            int flag = 0;
            try{
                flag = agent.writeFile(MAC_ADDRESS_ID,buff);
            }catch(Exception e){
                e.printStackTrace();
            }

            if(flag > 0){
                mToast.setText("Update successfully.\r\nPlease reboot this device");
                mToast.show();
            }else{
                mToast.setText("Update failed");
                mToast.show();
            }
            
        }catch(Exception e){
            mToast.setText(e.getMessage() + ":" + e.getCause());
            mToast.show();
            e.printStackTrace();
        }
    }

    private void refreshWifiStatus() {
        RadioButton pingRadioButton;
        final WifiInfo wifiInfo = mWifiManager.getConnectionInfo();

        Xlog.i(TAG, "refreshWifiStatus is called");
        setWifiStateText(mWifiManager.getWifiState());
        mBSSID.setText(wifiInfo.getBSSID());
        mHiddenSSID.setText(String.valueOf(wifiInfo.getHiddenSSID()));
        
        
        int ipAddr = wifiInfo.getIpAddress();
        StringBuffer ipBuf = new StringBuffer();
        ipBuf.append(ipAddr  & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff).append('.').
        append((ipAddr >>>= 8) & 0xff);
        mIPAddr.setText(ipBuf);
        
        
        
        if(wifiInfo.getLinkSpeed() > 0){
            mLinkSpeed.setText(String.valueOf(wifiInfo.getLinkSpeed())+" Mbps");
        }else{
            mLinkSpeed.setText(R.string.unknown_string);
        }
        
        mMACAddr.setText(wifiInfo.getMacAddress());
        mNetworkId.setText(String.valueOf(wifiInfo.getNetworkId()));
        
        if(wifiInfo.getRssi() != INVALID_RSSI){
            mRSSI.setText(String.valueOf(wifiInfo.getRssi()));
        }else{
            mRSSI.setText(R.string.na_string);
        }
        
        if(mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED){
            mSSID.setText(wifiInfo.getSSID());
        }else{
            mSSID.setText("");
        }

        SupplicantState supplicantState = wifiInfo.getSupplicantState();
        setSupplicantStateText(supplicantState);

        mDns1 = SystemProperties.get("net.dns1");
        mDns2 = SystemProperties.get("net.dns2");
        pingRadioButton = (RadioButton)findViewById(R.id.pingRadioBtn2);
        pingRadioButton.setText("DNS1:" + mDns1);

        pingRadioButton = (RadioButton)findViewById(R.id.pingRadioBtn3);
        pingRadioButton.setText("DNS2:" + mDns2);
        
        getMacAddr();
        
        updateSystemProperties();
    }

    private void updateSystemProperties(){
        StringBuilder sb = new StringBuilder();
        
        for(int i = 0; i < WIFI_SYSTEM_PROPERTY.length; i++){
            sb.append("[" + WIFI_SYSTEM_PROPERTY[i] + "]: [" + SystemProperties.get(WIFI_SYSTEM_PROPERTY[i],"") + "]\r\n");
        }
        
        mSystemProperties.setText(sb);
        
    }

    private void setSupplicantStateText(SupplicantState supplicantState) {
        if(SupplicantState.FOUR_WAY_HANDSHAKE.equals(supplicantState)) {
            mSupplicantState.setText("FOUR WAY HANDSHAKE");
        } else if(SupplicantState.ASSOCIATED.equals(supplicantState)) {
            mSupplicantState.setText("ASSOCIATED");
        } else if(SupplicantState.ASSOCIATING.equals(supplicantState)) {
            mSupplicantState.setText("ASSOCIATING");
        } else if(SupplicantState.COMPLETED.equals(supplicantState)) {
            mSupplicantState.setText("COMPLETED");
        } else if(SupplicantState.DISCONNECTED.equals(supplicantState)) {
            mSupplicantState.setText("DISCONNECTED");
        } else if(SupplicantState.DORMANT.equals(supplicantState)) {
            mSupplicantState.setText("DORMANT");
        } else if(SupplicantState.GROUP_HANDSHAKE.equals(supplicantState)) {
            mSupplicantState.setText("GROUP HANDSHAKE");
        } else if(SupplicantState.INACTIVE.equals(supplicantState)) {
            mSupplicantState.setText("INACTIVE");
        } else if(SupplicantState.INVALID.equals(supplicantState)) {
            mSupplicantState.setText("INVALID");
        } else if(SupplicantState.SCANNING.equals(supplicantState)) {
            mSupplicantState.setText("SCANNING");
        } else if(SupplicantState.UNINITIALIZED.equals(supplicantState)) {
            mSupplicantState.setText("UNINITIALIZED");
        } else {
            mSupplicantState.setText("BAD");
            Xlog.e(TAG, "supplicant state is bad");
        }
    }

    private void setWifiStateText(int wifiState) {
        String wifiStateString;
        switch(wifiState) {
        case WifiManager.WIFI_STATE_DISABLING:
            wifiStateString = getString(R.string.wifi_state_disabling);
            break;
        case WifiManager.WIFI_STATE_DISABLED:
            wifiStateString = getString(R.string.wifi_state_disabled);
            break;
        case WifiManager.WIFI_STATE_ENABLING:
            wifiStateString = getString(R.string.wifi_state_enabling);
            break;
        case WifiManager.WIFI_STATE_ENABLED:
            wifiStateString = getString(R.string.wifi_state_enabled);
            break;
        case WifiManager.WIFI_STATE_UNKNOWN:
            wifiStateString = getString(R.string.wifi_state_unknown);
            break;
        default:
            wifiStateString = "BAD";
            Xlog.e(TAG, "wifi state is bad");
            break;
        }

        if(wifiState == WifiManager.WIFI_STATE_DISABLED){
           mScanList.setText("");
        }

        mWifiState.setText(wifiStateString);
    }

    private void handleSignalChanged(int rssi) {
        if(rssi != INVALID_RSSI){
            mRSSI.setText(String.valueOf(rssi));
        }else{
            mRSSI.setText(R.string.na_string);
        }
    }

    private void handleWifiStateChanged(int wifiState) {
        setWifiStateText(wifiState);
    }

    private void handleScanResultsAvailable() {
        List<ScanResult> list = mWifiManager.getScanResults();

        StringBuffer scanList = new StringBuffer();
        if (list != null) {
            for (int i = list.size() - 1; i >= 0; i--) {
                final ScanResult scanResult = list.get(i);

                if (scanResult == null) {
                    continue;
                }

                if (TextUtils.isEmpty(scanResult.SSID)) {
                    continue;
                }

                scanList.append(scanResult.SSID+" ");
            }
        }
        mScanList.setText(scanList);
    }

    private void handleSupplicantStateChanged(SupplicantState state, boolean hasError, int error) {
        if (hasError) {
            mSupplicantState.setText("ERROR AUTHENTICATING");
        } else {
            setSupplicantStateText(state);
        }
    }

    private void handleNetworkStateChanged(NetworkInfo networkInfo) {
        if (mWifiManager.isWifiEnabled()) {
            String summary = Summary.get(this, mWifiManager.getConnectionInfo().getSSID(),
                                         networkInfo.getDetailedState());
            mNetworkState.setText(summary);
        }
    }

    private final void updatePingState() {
        final Handler handler = new Handler();
        // Set all to unknown since the threads will take a few secs to update.
        mPingIpAddrResult = getResources().getString(R.string.radioInfo_unknown);
        mPingHostnameResult = getResources().getString(R.string.radioInfo_unknown);
        mHttpClientTestResult = getResources().getString(R.string.radioInfo_unknown);

        mPingIpAddr.setText(mPingIpAddrResult);
        mPingHostname.setText(mPingHostnameResult);
        mHttpClientTest.setText(mHttpClientTestResult);

        final Runnable updatePingResults = new Runnable() {
            public void run() {
                if(mPingIpAddrResult.indexOf("Pass") != -1){
                   mPingIpAddr.setTextColor(android.graphics.Color.GREEN);
                }else{
                   mPingIpAddr.setTextColor(android.graphics.Color.RED);
                }
                
                if(mPingHostnameResult.indexOf("Pass") != -1){
                   mPingHostname.setTextColor(android.graphics.Color.GREEN);
                }else{
                   mPingHostname.setTextColor(android.graphics.Color.RED);
                }

                if(mHttpClientTestResult.indexOf("Pass") != -1){
                   mHttpClientTest.setTextColor(android.graphics.Color.GREEN);
                }else{
                   mHttpClientTest.setTextColor(android.graphics.Color.RED);
                }
                
                mPingIpAddr.setText(mPingIpAddrResult);
                mPingHostname.setText(mPingHostnameResult);
                mHttpClientTest.setText(mHttpClientTestResult);
            }
        };
        Thread ipAddrThread = new Thread() {
            @Override
            public void run() {
                pingIpAddr();
                handler.post(updatePingResults);
            }
        };
        ipAddrThread.start();

        Thread hostnameThread = new Thread() {
            @Override
            public void run() {
                pingHostname();
                handler.post(updatePingResults);
            }
        };
        hostnameThread.start();

        Thread httpClientThread = new Thread() {
            @Override
            public void run() {
                httpClientTest();
                handler.post(updatePingResults);
            }
        };
        httpClientThread.start();
    }

    private final String getPingHostName() {
        String hostName = "www.google.com";

        if(mPingHostType == R.id.pingRadioBtn2) {
            hostName = SystemProperties.get("net.dns1");
        } else if(mPingHostType == R.id.pingRadioBtn3) {
            hostName = SystemProperties.get("net.dns2");
        }

        return hostName;
    }

    /**
     * The ping functions have been borrowed from Radio diagnostic app to
     * enable quick access on the wifi status screen
     */
    private final void pingIpAddr() {
        try {
            // TODO: Hardcoded for now, make it UI configurable
            String ipAddress = "8.8.8.8";
            Process p = Runtime.getRuntime().exec("ping -c 1 -w 100 " + ipAddress);

            Xlog.i(TAG, "start to pingIpAddr:" + ipAddress);

            int status = p.waitFor();
            if (status == 0) {
                mPingIpAddrResult = "Pass (" + ipAddress + ")";
            } else {
                mPingIpAddrResult = "Fail IP addr(" + ipAddress + ") not reachable";
            }
            
            Xlog.i(TAG, "end to pingIpAddr:" + mPingIpAddrResult);


        } catch (IOException e) {
            mPingIpAddrResult = "Fail (IOException)";
        } catch (InterruptedException e) {
            mPingIpAddrResult = "Fail (InterruptedException)";
        }
    }

    private final void pingHostname() {
        try {
            // TODO: Hardcoded for now, make it UI configurable
            String hostName = getPingHostName();
            Process p = Runtime.getRuntime().exec("ping -c 1 -w 100 " + hostName);

            Xlog.i(TAG, "start to pingHostname: " + hostName);

            int status = p.waitFor();
            if (status == 0) {
                mPingHostnameResult = "Pass (" + hostName + ")";
            } else {
                mPingHostnameResult = "Fail Host (" + hostName + ") unreachable";
            }

            Xlog.i(TAG, "end to pingHostname:" + mPingHostnameResult);

        } catch (UnknownHostException e) {
            mPingHostnameResult = "Fail (Unknown Host)";
        } catch (IOException e) {
            mPingHostnameResult= "Fail (IOException)";
        } catch (InterruptedException e) {
            mPingHostnameResult = "Fail (InterruptedException)";
        }
    }

    private void httpClientTest() {
        String httpUrl = "http://www.google.com";
        HttpClient client = new DefaultHttpClient();
        
        try {
            // TODO: Hardcoded for now, make it UI configurable
            HttpGet request = new HttpGet(httpUrl);
            HttpResponse response = client.execute(request);
            if (response.getStatusLine().getStatusCode() == 200) {
                mHttpClientTestResult = "Pass (" + httpUrl +")";
            } else {
                mHttpClientTestResult = "Fail (Code " + String.valueOf(response + " for " + httpUrl + ")");
            }

            Xlog.i(TAG, "start to httpClientTest");

            request.abort();
        } catch (IOException e) {
            e.printStackTrace();
            mHttpClientTestResult = "Fail (IOException)";
        }
    }

}
