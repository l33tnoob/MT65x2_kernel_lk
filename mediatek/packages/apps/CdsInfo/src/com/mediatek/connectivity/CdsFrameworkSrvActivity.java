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

import android.app.Activity;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.NetworkInfo;
import android.net.NetworkPolicy;
import android.net.NetworkPolicyManager;
import android.net.NetworkQuotaInfo;
import android.os.Bundle;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.widget.TextView;

public class CdsFrameworkSrvActivity extends Activity {
    private static final String TAG = "CDSINFO/CdsFrameworkSrvActivity";

    private static final String NULL_INFO = "N/A";

    private TextView mActiveNetworkInfo;
    private TextView mActiveNetworkLinkProperties;
    private TextView mActiveNetworkQuotaInfo;
    private TextView mNetworkInfo;
    private TextView mNetworkPolicy;

    private Context mContext;
    private ConnectivityManager mConnMgr;
    private NetworkPolicyManager mNetPolicyMgr;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_framework_info);

        mContext = this.getBaseContext();
        if(mContext == null) {
            Xlog.e(TAG, "Could not get Conext of this activity");
        }

        mConnMgr = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        if(mConnMgr == null) {
            Xlog.e(TAG, "Could not get Connectivity Manager");
            return;
        }

        mNetPolicyMgr = NetworkPolicyManager.from(mContext);
        if(mNetPolicyMgr == null) {
            Xlog.e(TAG, "Could not get Network policy manager");
            return;
        }

        mActiveNetworkInfo            = (TextView) findViewById(R.id.active_network_info);
        mActiveNetworkLinkProperties  = (TextView) findViewById(R.id.active_link_properties);
        mActiveNetworkQuotaInfo       = (TextView) findViewById(R.id.active_network_quota_info);
        mNetworkPolicy                = (TextView) findViewById(R.id.network_policy);
        mNetworkInfo                  = (TextView) findViewById(R.id.network_infos);

        Xlog.i(TAG, "CdsFrameworkSrvActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateFrameworkSrvInfo();
    }

    private void updateFrameworkSrvInfo() {
        int i = 0;
        NetworkInfo networkInfo;
        NetworkPolicy[] networkPolicy;
        LinkProperties linkProperties;
        String infoString = "";
        //Method method = null;

        try {


            networkInfo = mConnMgr.getActiveNetworkInfo();                       
            if(networkInfo != null) {
                mActiveNetworkInfo.setText(networkInfo.toString().replace(',','\n'));
            } else {
                mActiveNetworkInfo.setText(NULL_INFO);
            }

            //method =  mConnMgr.getClass().getMethod("getActiveLinkProperties");
            //linkProperties = (LinkProperties) method.invoke(mConnMgr);
            linkProperties = mConnMgr.getActiveLinkProperties();            
            if(linkProperties != null){
                Xlog.i(TAG, linkProperties.toString());
            }

            if(networkInfo != null) {
                mActiveNetworkLinkProperties.setText(linkProperties.toString().replace("]", "]\n"));
            } else {
                mActiveNetworkLinkProperties.setText(NULL_INFO);
            }


            //method = mConnMgr.getClass().getMethod("getActiveNetworkQuotaInfo");
            //NetworkQuotaInfo networkQuotaInfo = (NetworkQuotaInfo) method.invoke(mConnMgr);
            NetworkQuotaInfo networkQuotaInfo = mConnMgr.getActiveNetworkQuotaInfo();

            if(networkQuotaInfo != null) {
                Xlog.i(TAG, networkQuotaInfo.toString());
                infoString = "Actual:" + networkQuotaInfo.getEstimatedBytes() + " Warning:" + networkQuotaInfo.getSoftLimitBytes() + " Limit:" + networkQuotaInfo.getHardLimitBytes();
                mActiveNetworkQuotaInfo.setText(infoString);
            } else {
                mActiveNetworkQuotaInfo.setText(NULL_INFO);
            }


            infoString = "";
            NetworkInfo[] networkInfoArr = mConnMgr.getAllNetworkInfo();
            for(i = 0; i < networkInfoArr.length; i++) {
                networkInfo = networkInfoArr[i];
                infoString = infoString + "[" + i + "]" + networkInfo.toString().replace(',','\n') + "\r\n----------------------\r\n";
                infoString = infoString.replace(',','\n');
                if(networkInfo.isConnected()) {
                    //method =  mConnMgr.getClass().getMethod("getLinkProperties", int.class);
                    //linkProperties = (LinkProperties) method.invoke(mConnMgr, networkInfo.getType());
                    linkProperties = mConnMgr.getLinkProperties(networkInfo.getType());
                    infoString = infoString + "[" + i + "]" + linkProperties.toString() + "\r\n----------------------\r\n";
                }
            }
            mNetworkInfo.setText(infoString);

            networkPolicy = mNetPolicyMgr.getNetworkPolicies();
            infoString = "";
            for(i = 0; i < networkPolicy.length; i++) {
                infoString = infoString + "[" + i + "]" + networkPolicy[i].toString().replace(',','\n') + "\r\n----------------------\r\n";
            }            
            mNetworkPolicy.setText(infoString);

            Xlog.i(TAG, "updateFrameworkSrvInfo Done");
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onPause() {
        super.onPause();


    }

}
