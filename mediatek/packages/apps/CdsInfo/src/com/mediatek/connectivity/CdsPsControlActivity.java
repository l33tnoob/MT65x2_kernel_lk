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




import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.net.INetworkStatsService;
import android.net.NetworkStats;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.view.View;
import android.view.View.OnClickListener;
import android.util.Log;

import com.mediatek.connectivity.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public class CdsPsControlActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsPsControlActivity";

    private static final String INTERNET = "android.permission.INTERNET";
    private static final String DATAFILE ="datafile";
    
    private static final String FW_ENABLED = "fw_enable";
    private static final String FW_GCFSTK = "fw_gcf_stk";

    
    private static final int MDLOGGER_PORT = 30017;
    private static final int ADB_PORT = 5037;
    private static final int HTTP_PORT = 80;

    private static INetworkStatsService sStatsService;

    private SharedPreferences mDataStore;


    private ArrayList<MyAppInfo> mAppList;
    private Button mEnableFwBtn = null;
    private Button mDisableFwBtn = null;
    private CheckBox mGcfChk = null;
    private Context mContext;
    private INetworkManagementService mNetd;
    private ListView mAppListViw;
    private SimpleAdapter mAdapter;
    private TextView mFwStatus;    
    private boolean mIsEnabled;
    private boolean mIsGcfStk;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        mContext = this.getBaseContext();

        setContentView(R.layout.cds_ps_data);

        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNetd = INetworkManagementService.Stub.asInterface(b);

        mDataStore = this.getSharedPreferences(DATAFILE, 0);


        mEnableFwBtn = (Button)this.findViewById(R.id.btn_enable_fw);
        if(mEnableFwBtn != null) {
            mEnableFwBtn.setOnClickListener(new OnClickListener() {
                public void onClick(View arg0) {
                    setFirewallEnabled(true);
                }
            });
        }

        mDisableFwBtn = (Button)this.findViewById(R.id.btn_disable_fw);
        if(mDisableFwBtn != null) {
            mDisableFwBtn.setOnClickListener(new OnClickListener() {
                public void onClick(View arg0) {
                    setFirewallEnabled(false);
                }
            });
        }

        mFwStatus = (TextView) findViewById(R.id.ps_fw_rule_value);        

        mGcfChk = (CheckBox) findViewById(R.id.chk_stk_fw);
        mGcfChk.setOnCheckedChangeListener(cbListener);

        if(mNetd == null) {
            Xlog.e(TAG, "INetworkManagementService is null");
            return;
        }

        try {
            mIsEnabled = mDataStore.getBoolean(FW_ENABLED, false);
            mIsGcfStk = mDataStore.getBoolean(FW_GCFSTK, false);
            setFirewallEnabled(mIsEnabled, true);            
        } catch(Exception e) {
            e.printStackTrace();
        }

        Xlog.i(TAG, "CdsPsControlActivity is started");
    }

    private void setGcfFwRule(){
        
        try{
            mNetd.setFirewallEgressDestRule("1.1.1.0/24", HTTP_PORT, mIsGcfStk);
        }catch(Exception e){
            e.printStackTrace();
        }
        

       try{
            SharedPreferences.Editor editor = mDataStore.edit();            
            editor.putBoolean(FW_GCFSTK, mIsGcfStk);
            editor.commit();
       }catch(Exception e){
           e.printStackTrace();
       }       
    }

    private void setFirewallEnabled(boolean enabled) {
        setFirewallEnabled(enabled, false);
    }

    private void setFirewallEnabled(boolean enabled, boolean force) {
        if(mNetd == null) {
            Xlog.e(TAG, "INetworkManagementService is null");
            return;
        }

        Xlog.i(TAG, "set firewall:" + enabled);

        if(mIsEnabled == enabled && !force){
           Xlog.i(TAG, "No change");
           return;
        }

        mIsEnabled = enabled;

        try {
            //If enable is true, enable before iptable rule configuration
            if(enabled){
                mNetd.setFirewallEnabled(true);
                mNetd.setFirewallEgressProtoRule("icmp", true);
                mNetd.setFirewallInterfaceRule("lo", true);
            }

            try{
                mNetd.setFirewallEgressDestRule("1.1.1.0/24", HTTP_PORT, mIsGcfStk);
            }catch(Exception ee){
                ee.printStackTrace();
            }

            mNetd.setFirewallEgressDestRule("", MDLOGGER_PORT, enabled);
            mNetd.setFirewallEgressDestRule("", ADB_PORT, enabled);

            //int uid = android.os.Process.getUidForName("root");
            //mNetd.setFirewallUidRule(uid, enabled);

            //if enable is false, disable after iptable rule configuration
            if(!enabled){
                mNetd.setFirewallEgressProtoRule("icmp", true);
                mNetd.setFirewallInterfaceRule("lo", false);
                mNetd.setFirewallEnabled(false);
            }
        } catch(Exception e) {
            e.printStackTrace();
        } finally{
            updateFwButton(enabled);
        }
        
        try{
            SharedPreferences.Editor editor = mDataStore.edit();
            editor.putBoolean(FW_ENABLED, mIsEnabled);
            editor.putBoolean(FW_GCFSTK, mIsGcfStk);
            editor.commit();
        }catch(Exception e){
            e.printStackTrace();
        }
        
    }

    private void updateAppList() {

    }

    private void updateFwButton(boolean enabled) {
        if(enabled) {
            mFwStatus.setText("enabled");
        } else {
            mFwStatus.setText("disabled");
        }

        Xlog.i(TAG, "mIsGcfStk:" + mIsGcfStk);
        mGcfChk.setChecked(mIsGcfStk);
    }

    @Override
    protected void onResume() {
        try {
            updateFwButton(mIsEnabled);
        } catch(Exception e) {
            e.printStackTrace();
        }

        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private CheckBox.OnCheckedChangeListener cbListener = 
        new CheckBox.OnCheckedChangeListener(){
            
            public void onCheckedChanged(CompoundButton buttonView,boolean isChecked)
            {
                mIsGcfStk = isChecked;
                setGcfFwRule();
            }
    };
        


    public void onClick(View v) {
        int buttonId = v.getId();
    }

    void getApps(Context context) {

        try {
            PackageManager packageManager = context.getPackageManager();
            List<ApplicationInfo> packages = packageManager.getInstalledApplications(0);
            mAppList = new ArrayList<MyAppInfo>(packages.size());
            Iterator<ApplicationInfo> apps = packages.iterator();
            MyAppInfo myApp;

            while (apps.hasNext()) {
                ApplicationInfo app = (ApplicationInfo) apps.next();
                String packageName = app.packageName;// package name
                if(packageManager.checkPermission(INTERNET, packageName) != 0) {
                    int uid = app.uid;
                    Xlog.d(TAG, "packageName:" + packageName + ":" + uid);

                    NetworkStats dataStats = getNetworkStatsForUid(uid);

                    myApp = new MyAppInfo(packageName, app);
                    mAppList.add(myApp);
                }
            }

            //Add pre-defined UID process
            ApplicationInfo appInfo = new ApplicationInfo();
            appInfo.uid = android.os.Process.getUidForName("root");
            appInfo.packageName = "root Applications";
            myApp = new MyAppInfo(appInfo.packageName, appInfo);
            mAppList.add(myApp);

            appInfo.uid = android.os.Process.getUidForName("media");
            appInfo.packageName = "Media server";
            myApp = new MyAppInfo(appInfo.packageName, appInfo);
            mAppList.add(myApp);

            appInfo.uid = android.os.Process.getUidForName("vpn");
            appInfo.packageName = "VPN networking";
            myApp = new MyAppInfo(appInfo.packageName, appInfo);
            mAppList.add(myApp);

            appInfo.uid = android.os.Process.getUidForName("shell");
            appInfo.packageName = "Linux shell";
            myApp = new MyAppInfo(appInfo.packageName, appInfo);
            mAppList.add(myApp);

            appInfo.uid = android.os.Process.getUidForName("gps");
            appInfo.packageName = "GPS";
            myApp = new MyAppInfo(appInfo.packageName, appInfo);
            mAppList.add(myApp);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    private List<? extends Map<String, ?>> getData() {
        int i = 0;
        List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        Map<String, Object> map;
        MyAppInfo myApp;

        for(i = 0; i < mAppList.size(); i++) {
            myApp = (MyAppInfo) mAppList.get(i);
            map = new HashMap<String, Object>();
            map.put("appText", myApp.getPackageName());
            list.add(map);
        }

        return list;
    }

    private synchronized static INetworkStatsService getStatsService() {
        if (sStatsService == null) {
            sStatsService = INetworkStatsService.Stub.asInterface(
                                ServiceManager.getService(Context.NETWORK_STATS_SERVICE));
        }
        return sStatsService;
    }

    private static NetworkStats getNetworkStatsForUid(int uid) {
        try {
            return getStatsService().getDataLayerSnapshotForUid(uid);
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }
    }


    private class MyAppInfo {
        private String mPackageName;
        private ApplicationInfo mAppInfo;

        public MyAppInfo() {

        }

        public MyAppInfo(String name, ApplicationInfo info) {
            mPackageName = name;
            mAppInfo = info;
        }

        public String getPackageName() {
            return mPackageName;
        }

    }
}
