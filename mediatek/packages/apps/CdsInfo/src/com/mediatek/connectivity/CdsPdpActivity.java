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

package com.mediatek.connectivity;

import com.mediatek.connectivity.R;
import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.view.View;
import android.net.NetworkConfig;
import android.net.NetworkInfo;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.net.ConnectivityManager;
import android.os.SystemProperties;
import android.net.LinkProperties;

import android.text.InputFilter;
import android.text.Spanned;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Spinner;
import android.widget.Toast;
import android.widget.EditText;


import android.os.Message;

import com.android.internal.telephony.Phone;

import java.util.ArrayList;
import java.lang.CharSequence;
import java.net.InetAddress;

import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import android.provider.Settings;
import com.mediatek.common.featureoption.FeatureOption;

public class CdsPdpActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/PDP";
    
    private static final String PREFIX_APN = "enable";
    private static final String[] APN_LIST = new String[]{"MMS", "SUPL", "WAP", "HIPRI"};
    private static final int[] APN_TYPE_LIST = new int[]{ConnectivityManager.TYPE_MOBILE_MMS, ConnectivityManager.TYPE_MOBILE_SUPL, 
                                                ConnectivityManager.TYPE_MOBILE_WAP, ConnectivityManager.TYPE_MOBILE_HIPRI};
    private static final int MSG_KEEP_CONN = 0x3001;
    
    // how long to wait before switching back to a radio's default network
    private static final int RESTORE_DEFAULT_NETWORK_DELAY = 120 * 60 * 1000;
    // system property that can override the above value
    private static final String NETWORK_RESTORE_DELAY_PROP_NAME =
            "android.telephony.apn-restore";    
    
    private int    mSelectApnType = ConnectivityManager.TYPE_MOBILE_MMS;    
    private String mSelectApnFeature = Phone.FEATURE_ENABLE_MMS;
    private Toast mToast;
    private Spinner mApnSpinner = null;
    private ConnectivityManager mConnMgr = null;
    
    private int mSelectApnPos = 0;
   
    private Context mContext;    
    private Button mAddBtnCmd = null;
    private Button mRunBtnCmd = null;
    private Button mStopBtnCmd = null;
    private TextView mOutputScreen = null;
    private EditText mHostAddress = null;    
    private IntentFilter mConnFilter = null;
    
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_pdp);

        mContext = this.getBaseContext();
        
                
        mApnSpinner = (Spinner) findViewById(R.id.apnTypeSpinnner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, APN_LIST);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mApnSpinner.setAdapter(adapter);
        mApnSpinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View arg1,
            int position, long arg3) {
                // TODO Auto-generated method stub
                try{
                    mSelectApnFeature = PREFIX_APN + adapterView.getSelectedItem().toString();
                    mSelectApnPos     = position;
                    
                    mApnSpinner.requestFocus();
                    updateConnectButton();
                    
                }catch(Exception e){
                    mSelectApnFeature = Phone.FEATURE_ENABLE_MMS;
                    e.printStackTrace();
                }
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub

            }
        });
        
        mConnMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        mConnFilter = new IntentFilter();
        mConnFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);                

        mRunBtnCmd = (Button) findViewById(R.id.Start);
        mRunBtnCmd.setOnClickListener(this);

        mAddBtnCmd = (Button) findViewById(R.id.Add);
        mAddBtnCmd.setOnClickListener(this);
        mAddBtnCmd.requestFocus();
        
        mStopBtnCmd = (Button) findViewById(R.id.Stop);
        mStopBtnCmd.setOnClickListener(this);
        
        mHostAddress   = (EditText)  findViewById(R.id.HostAddress);
                
        mOutputScreen = (TextView) findViewById(R.id.outputText);
        
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);
        
        Xlog.i(TAG, "CdsPdpActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        mContext.registerReceiver(mNetworkConnReceiver, mConnFilter);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mContext.unregisterReceiver(mNetworkConnReceiver);
    }


    public void onClick(View v) {
        int buttonId = v.getId();
        
        switch(buttonId) {
            case R.id.Start:
                handleStartAPN();
                break;
            case R.id.Stop:
                handleStopAPN();
                break;
            case R.id.Add:
                handleAddIPAddressToRoute();
                break;
            default:
                break;
        }
    }
    
    private void getApnTypes(){
        String[] naStrings = mContext.getResources().getStringArray(com.android.internal.R.array.networkAttributes);
        String   element = "";
        int i = 0;
                        
        for (String naString : naStrings) {
            try {
                NetworkConfig n = new NetworkConfig(naString);
                if (n.type > ConnectivityManager.MAX_NETWORK_TYPE) {
                    Xlog.e(TAG, "Error in networkAttributes - ignoring attempt to define type " +
                            n.type);
                    continue;
                }else if(n.type == ConnectivityManager.TYPE_MOBILE){
                    Xlog.i(TAG, "Skip default data connection");
                    continue;
                }

                if( n.radio != ConnectivityManager.TYPE_MOBILE){
                    continue;
                }
                                                
                element = n.name + "/" + n.type;                
                Xlog.i(TAG, "Add apn:" + element);
                i++;
            } catch(Exception e) {
                // ignore it - leave the entry null
            }
        }
    }
    
    private int getDataConnectionFromSetting(){
        int slot = 0;
        
        if(FeatureOption.MTK_GEMINI_ENHANCEMENT == true){
            long currentDataConnectionMultiSimId =  Settings.System.getLong(mContext.getContentResolver(), Settings.System.GPRS_CONNECTION_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);            
            SimInfoRecord simInfo = SimInfoManager.getSimInfoById(mContext, currentDataConnectionMultiSimId);
            if(simInfo != null) {
                slot = simInfo.mSimSlotId;
            }else{
                Xlog.e(TAG, "simInfo is null");
                slot = -1;
            }
        }else{
            slot =  Settings.System.getInt(mContext.getContentResolver(), Settings.System.GPRS_CONNECTION_SETTING, Settings.System.GPRS_CONNECTION_SETTING_DEFAULT) - 1;
        }

        if(slot < 0){
           Xlog.e(TAG, "Set default SIM slot to 0 due to slot is " + slot);
           slot = 0;
        }
        Xlog.v(TAG, "Default Data Setting value=" + slot);

        return slot;
    }
    
    private void handleStopAPN(){
        int result = -1;
        
        if(FeatureOption.MTK_GEMINI_SUPPORT){
            mConnMgr.stopUsingNetworkFeatureGemini(ConnectivityManager.TYPE_MOBILE, mSelectApnFeature, getDataConnectionFromSetting());
        }else{
            mConnMgr.stopUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE, mSelectApnFeature);
        }
        
        Xlog.i(TAG, "[Feature][Stop]" + mSelectApnFeature);
    }    
    
    
    private void handleStartAPN(){
        int result = -1;
        
        //[TBD]Configure restore timer to null if this test is finished
        SystemProperties.set(NETWORK_RESTORE_DELAY_PROP_NAME, String.valueOf(RESTORE_DEFAULT_NETWORK_DELAY));
        
        if(FeatureOption.MTK_GEMINI_SUPPORT){
            mConnMgr.startUsingNetworkFeatureGemini(ConnectivityManager.TYPE_MOBILE, mSelectApnFeature, getDataConnectionFromSetting());
        }else{
            mConnMgr.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE, mSelectApnFeature);
        }
                                                
        Xlog.i(TAG, "[Feature][Start]" + mSelectApnFeature);
        
    }
    
    private void updateConnectButton(){
        NetworkInfo nwInfo = null;
        
        nwInfo = mConnMgr.getNetworkInfo(APN_TYPE_LIST[mSelectApnPos]);
        
        if(nwInfo == null){
           mRunBtnCmd.setEnabled(false);
           mStopBtnCmd.setEnabled(false);
           return;
        }
        
        if(nwInfo.getState() == NetworkInfo.State.CONNECTED){
           mRunBtnCmd.setEnabled(false);
           mStopBtnCmd.setEnabled(true);
        }else{
           mRunBtnCmd.setEnabled(true);
           mStopBtnCmd.setEnabled(false);
        }
    }
    
    private void handleAddIPAddressToRoute(){
        NetworkInfo nwInfo = null;
        String ipAddress = mHostAddress.getText().toString();
        int addr = 0;
                
        nwInfo = mConnMgr.getNetworkInfo(APN_TYPE_LIST[mSelectApnPos]);
        
        Xlog.i(TAG, "handleAddIPAddressToRoute:" + ipAddress);
        
        if(!isIpAddress(ipAddress)){
            mToast.setText("The IP address is not valid");
            mToast.show();
            return;
        }
        
        if(nwInfo != null && nwInfo.getState() == NetworkInfo.State.CONNECTED){
            
            try{
                InetAddress inetAddr = InetAddress.getByName(ipAddress);
                byte[] addressBytes = inetAddr.getAddress();
            
                addr =  ((addressBytes[3] & 0xFF) << 24)
                | ((addressBytes[2] & 0xFF) << 16)
                | ((addressBytes[1] & 0xFF) <<  8)
                | ( addressBytes[0] & 0xFF);
                
                mToast.setText("Add host:" + ipAddress + " to " + APN_LIST[mSelectApnPos]);
                mToast.show();
                
                mConnMgr.requestRouteToHost(APN_TYPE_LIST[mSelectApnPos], addr);
                
                mHostAddress.setText("");
            }catch(Exception e){
                e.printStackTrace();
                mToast.setText("Fail to add host address" + e.getMessage());
                mToast.show();
            }
        }else{
            mToast.setText("The connection(" + APN_LIST[mSelectApnPos] + ") is not connected");
            mToast.show();
        }
        
    }
        
    private BroadcastReceiver mNetworkConnReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            if(intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                Message response = null;

                NetworkInfo info = (NetworkInfo)intent.getExtra(ConnectivityManager.EXTRA_NETWORK_INFO);

                if(info == null) {
                    return;
                }

                int type = info.getType();
                NetworkInfo.State state = info.getState();
                
                Xlog.i(TAG, "Type:" + type + "/ state:" + state);
                
                if(type == APN_TYPE_LIST[mSelectApnPos]){
                    mToast.setText(info.toString());
                    mToast.show();
                }
                updateApnStatus();
            }
        }
    };
    
    private void updateApnStatus(){
        NetworkInfo    nwInfo = null;
        LinkProperties nwLink = null;
        String sb = new String();
        StringBuilder tb = new StringBuilder();
        
        nwInfo = mConnMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        sb = nwInfo.toString() + "\r\n\r\n";
        tb.append(sb.replace(',', '\n'));
        
        if(nwInfo.getState() == NetworkInfo.State.CONNECTED){            
            nwLink = mConnMgr.getLinkProperties(ConnectivityManager.TYPE_MOBILE);
            sb = nwLink.toString() + "\r\n\r\n";
            tb.append(sb.replace(',', '\n'));
        }
        
        for(int i =0; i < APN_TYPE_LIST.length; i++){
            nwInfo = mConnMgr.getNetworkInfo(APN_TYPE_LIST[i]);            
            sb = nwInfo.toString() + "\r\n\r\n";
            tb.append(sb.replace(',', '\n'));
            
            if(nwInfo.getState() == NetworkInfo.State.CONNECTED){                
                nwLink = mConnMgr.getLinkProperties(APN_TYPE_LIST[i]);
                sb = nwLink.toString() + "\r\n\r\n";
                tb.append(sb.replace(',', '\n'));
            }
        }
        
        updateConnectButton();
        mOutputScreen.setText(tb.toString());
    }
 
    private boolean isIpAddress(String address){        
        return address.matches("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");   
    }  
}