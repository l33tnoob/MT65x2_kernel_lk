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


import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Button;
import android.net.NetworkUtils;
import android.widget.Toast;
import com.mediatek.xlog.Xlog;

public class CdsMtuSettingActivity extends Activity implements View.OnClickListener{
    private static final String TAG = "CDSINFO/CdsMtuSettingActivity";
    private Spinner mSpinnerIntefaceName;
    private EditText mMtuSizeValue;
    private TextView mInterfaceName;
    private Button   mConfigureBtn;
    private CheckBox mMobileChk;
    private CheckBox mInterfaceChk;
    private ArrayAdapter<String> mAdapterInterfaceName;
    private INetworkManagementService mNetd;



    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        
        
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cds_mtu_setting);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNetd = INetworkManagementService.Stub.asInterface(b);

        mInterfaceName = (TextView) findViewById(R.id.interfaceName);
        mMtuSizeValue = (EditText) findViewById(R.id.mtuValue);
        mSpinnerIntefaceName = (Spinner) findViewById(R.id.spinnerInterfaceName);
        mConfigureBtn = (Button) findViewById(R.id.mtuConfigure);        
        mConfigureBtn.setOnClickListener(this);
        
        mMobileChk = (CheckBox) findViewById(R.id.mobileOnlyChk);
        mMobileChk.setOnClickListener(this);
        
        mInterfaceChk = (CheckBox) findViewById(R.id.interfaceUpChk);
        mInterfaceChk.setOnClickListener(this);
        
        //create an arrayAdapter an assign it to the spinner
        mAdapterInterfaceName = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mAdapterInterfaceName.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinnerIntefaceName.setAdapter(mAdapterInterfaceName);
        mSpinnerIntefaceName.setOnItemSelectedListener(new Spinner.OnItemSelectedListener(){
            public void onItemSelected(AdapterView<?> adapterView, View view, int position, long id){
                if(adapterView.getSelectedItem() != null){
                    String item = adapterView.getSelectedItem().toString();
                    if(item != null){
                        mInterfaceName.setText(item);
                    }
                }
            }
            public void onNothingSelected(AdapterView<?> adapterView) {
                if(adapterView.getSelectedItem() != null){
                    String item = adapterView.getSelectedItem().toString();
                    if(item != null){
                        mInterfaceName.setText(item);
                    }
                }
            }
        });        
    }

    @Override
    protected void onResume() {
        
        getInterfaceList();
        // TODO Auto-generated method stub
        super.onResume();
    }
    
    public String getInterfaceList() {
        String name;            
        Boolean mobileOnly = false;
        mAdapterInterfaceName.clear();
                
        mobileOnly = mMobileChk.isChecked();
                
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                if(intf == null) continue;
                if(!intf.isLoopback() && !intf.isVirtual()){
                    name = intf.getName();
                    if(name == null) continue;
                    if(mobileOnly && (name.indexOf("ccmni") >= 0)){
                            addInterfaceUp(intf);
                    }else if(!mobileOnly){
                        addInterfaceUp(intf);
                    }
                }
            }
        } catch (SocketException ex) {
            ex.printStackTrace();
        }
        
        return null;
    } 
    
    private void addInterfaceUp(NetworkInterface intf){
        int mtu = 0;        
        String item;        
        Boolean interfaceUpOnly = false;
        interfaceUpOnly = mInterfaceChk.isChecked();
        
        String name = intf.getName();
        try {
            mtu  = intf.getMTU();
            item = name + "/" + mtu;
             if(interfaceUpOnly && intf.isUp()){
                mAdapterInterfaceName.add(item);
            }else if(!interfaceUpOnly){
                mAdapterInterfaceName.add(item);
            }             
        } catch (SocketException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }                                 
       
    }
    
    public void onClick(View v) {        
            int buttonId = v.getId();
            
            switch(buttonId){
                case R.id.mtuConfigure:
                    //setMtuByInterface()
                    String name = mInterfaceName.getText().toString();
                    String mtuValueText = mMtuSizeValue.getText().toString();
                    boolean ret = false;
                    int mtuValue = 0;
                    Toast toast;
                    String tempString;
                    
                    try{
                        mtuValue = Integer.parseInt(mtuValueText);
                    }catch(Exception e){
                        Xlog.e(TAG, "Invalid value:" + mtuValueText);
                    }                
                    
                    if(name.length() == 0 || (mtuValue > 1500 || mtuValue < 1000)){
                        tempString = "Invalid inteface name or MTU size:" + name + "/" + mtuValue + "\r\n MTU size is from 1000 ~ 1500";
                        toast = Toast.makeText(this, tempString, Toast.LENGTH_SHORT);
                        toast.show();
                        return;
                    }
                    
                    name = name.substring(0, name.indexOf("/"));
                    Xlog.i(TAG, "Configure MTU size:" + name + "/" + mtuValue);
                    
                    try{
                        mNetd.setMtu(name, mtuValue);
                        ret = true;
                    }catch(Exception e){
                        e.printStackTrace();
                    }

                    if(ret){                
                        tempString = "Successfully for MTU size configuation";
                    }else{
                        tempString = "Fail to set MTU size";
                    }
                    toast = Toast.makeText(this, tempString, Toast.LENGTH_LONG);
                    toast.show();
                    
                    break; 
                case R.id.interfaceUpChk:
                case R.id.mobileOnlyChk:
                    getInterfaceList();
                    mSpinnerIntefaceName.refreshDrawableState();
                    break;
                default:
                    Xlog.e(TAG, "Unknown button ID");
                    break;
        }    
    }
    
    private void executeShellCmd(final String cmdStr){
        
        new Thread() {
            @Override  
            public void run(){
                super.run();
                try{
                    synchronized (CdsMtuSettingActivity.this){
                        CdsShellExe.execCommand(cmdStr);
                    }
                }catch(Exception e){
                    e.printStackTrace();    
                }
            }            
        }.start(); 
    }    
}