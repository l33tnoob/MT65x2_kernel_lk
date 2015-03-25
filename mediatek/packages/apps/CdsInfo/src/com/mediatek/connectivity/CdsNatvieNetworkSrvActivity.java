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

import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.widget.TextView;
import android.content.pm.ActivityInfo;
import android.os.Handler;
import android.os.Message;

public class CdsNatvieNetworkSrvActivity extends Activity {
    private static final String TAG = "CDSINFO/CdsNatvieNetworkSrvActivity";
    private static final String ERROR_STRING = "Command Error";

    private static final String NET_CFG_CMD = "netcfg";
    private static final String IF_CONFIG_CMD = "ip link";
    private static final String IP_ROUTE_CMD = "ip -4 route show"; //
    //private static final String ipTablesCmd = "iptables -L";
    private static final String NET_STAT_CMD = "netstat";

    private TextView mNetworkList;
    private TextView mNetworkConfig;
    private TextView mIpRoute;
    //private TextView mIpTables;
    private TextView mNetStat;

    private static final int MSG_UPDATE_UI = 0x3001;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_native_nw_info);
                
        mNetworkList   = (TextView) findViewById(R.id.network_list);
        mNetworkConfig = (TextView) findViewById(R.id.network_config);
        mIpRoute       = (TextView) findViewById(R.id.ip_route);
        //mIpTables      = (TextView) findViewById(R.id.ip_tables);
        mNetStat       = (TextView) findViewById(R.id.net_stat);
        Xlog.i(TAG, "CdsFrameworkSrvActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateFrameworkSrvInfo();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
    }

    private void updateFrameworkSrvInfo() {

        try {

            executeShellCmd(NET_CFG_CMD, R.id.network_list);
            mNetworkList.setText(R.string.loading_string);

            executeShellCmd(IF_CONFIG_CMD, R.id.network_config);
            mNetworkConfig.setText(R.string.loading_string);

            executeShellCmd(IP_ROUTE_CMD, R.id.ip_route);
            mIpRoute.setText(R.string.loading_string);
            
            //executeShellCmd(ipTablesCmd, R.id.ip_route);
            //mIpTables.setText(CdsShellExe.getOutput());
            
            executeShellCmd(NET_STAT_CMD, R.id.net_stat);
            mNetStat.setText(R.string.loading_string);

            Xlog.i(TAG, "updateFrameworkSrvInfo Done");
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);

    }

    private void executeShellCmd(final String cmdStr, final int id){
        
        new Thread() {
            @Override  
            public void run(){
                super.run();
                try{
                    synchronized (CdsNatvieNetworkSrvActivity.this){
                        CdsShellExe.execCommand(cmdStr);
                        Message msg = mHandler.obtainMessage();
                        msg.what = MSG_UPDATE_UI;
                        msg.arg1 = id;
                        String output = new String(CdsShellExe.getOutput());
                        msg.obj = (Object) output;
                        mHandler.sendMessage(msg);
                    }
                }catch(Exception e){
                    e.printStackTrace();    
                }
            }            
        }.start(); 
    }
    
    // Define the Handler that receives messages from the thread and update the
    // progress
    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            String output = "";
            int id = 0;
            
            switch(msg.what) {
                case MSG_UPDATE_UI:
                    output = (String) msg.obj;
                    id = msg.arg1;
                    updateInfo(id, output);
                    break;
                default:
                    break;
            }
        }
    };
    
    private void updateInfo(int id, String output){
        
        switch(id){
           case R.id.network_list:
             mNetworkList.setText(output);
           break;
             
           case R.id.network_config:
            mNetworkConfig.setText(output);
           break;
           
           case R.id.ip_route:
            mIpRoute.setText(output);
           break;
           
           case R.id.net_stat:
            mNetStat.setText(output);
           break;
            
           default:
            break;
        }
    }
}
