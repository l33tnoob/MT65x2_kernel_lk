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

package com.mediatek.op.wifi;

import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiManager;

import com.mediatek.common.wifi.IWifiFwkExt;
//import com.mediatek.xlog.Xlog;

import java.util.List;

public class DefaultWifiFwkExt implements IWifiFwkExt {
    private static final String TAG = "DefaultWifiFwkExt";
    protected Context mContext;
    protected WifiManager mWifiManager;

    static final int SECURITY_NONE = 0;
    static final int SECURITY_WEP = 1;
    static final int SECURITY_PSK = 2;
    static final int SECURITY_EAP = 3;
    static final int SECURITY_WAPI_PSK = 4;
    static final int SECURITY_WAPI_CERT = 5;
    static final int SECURITY_WPA2_PSK = 6;

    public DefaultWifiFwkExt(Context context) {
        mContext = context;
    }

    public void init() {
        mWifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
    }

    public boolean hasCustomizedAutoConnect() {
        //Xlog.d(TAG, "hasCustomizedAutoConnect false");
        return false;
    }

    public boolean shouldAutoConnect() {
        //Xlog.d(TAG, "shouldAutoConnect true");
        return true;
    }

    public boolean isWifiConnecting(int connectingNetworkId, List<Integer> disconnectNetworks) {
        List<WifiConfiguration> networks = mWifiManager.getConfiguredNetworks();
        List<ScanResult> scanResults = mWifiManager.getScanResults();
        if (null != networks && null != scanResults) {
            for (WifiConfiguration network : networks) {
                for (ScanResult scanresult : scanResults) {
                    //Xlog.d(TAG, "network.SSID=" + network.SSID + ", scanresult.SSID=" + scanresult.SSID);
                    //Xlog.d(TAG, "network.security=" + getSecurity(network)
                    //      + ", scanresult.security=" + getSecurity(scanresult));
                    if ((network.SSID != null) && (scanresult.SSID != null)
                        && network.SSID.equals("\"" + scanresult.SSID + "\"")
                        && (getSecurity(network) == getSecurity(scanresult))
                        && network.networkId == connectingNetworkId) {
                           return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean hasConnectableAp() {
        return false;
    }

    public void suspendNotification(int type) {
        //Xlog.d(TAG, "suspendNotification nothing");
    }

    public int defaultFrameworkScanIntervalMs() {
        return mContext.getResources().getInteger(com.android.internal.R.integer.config_wifi_framework_scan_interval);
    }

    public int getSecurity(WifiConfiguration config) {
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_PSK)) {
            return SECURITY_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_EAP)
            || config.allowedKeyManagement.get(KeyMgmt.IEEE8021X)) {
            return SECURITY_EAP;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_PSK)) {
            return SECURITY_WAPI_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_CERT)) {
            return SECURITY_WAPI_CERT;
        }
        if (config.wepTxKeyIndex >= 0 && config.wepTxKeyIndex < config.wepKeys.length
            && config.wepKeys[config.wepTxKeyIndex] != null) {
            return SECURITY_WEP;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WPA2_PSK)) {
            return SECURITY_WPA2_PSK;
        }
        return SECURITY_NONE;
    }

    public int getSecurity(ScanResult result) {
        if (result.capabilities.contains("WAPI-PSK")) {
            return SECURITY_WAPI_PSK;
        } else if (result.capabilities.contains("WAPI-CERT")) {
            return SECURITY_WAPI_CERT;
        } else if (result.capabilities.contains("WEP")) {
            return SECURITY_WEP;
        } else if (result.capabilities.contains("PSK")) {
            return SECURITY_PSK;
        } else if (result.capabilities.contains("EAP")) {
            return SECURITY_EAP;
        } else if (result.capabilities.contains("WPA2-PSK")) {
            return SECURITY_WPA2_PSK;
        }
        return SECURITY_NONE;
    }

    public String getApDefaultSsid() {
        return mContext.getString(com.android.internal.R.string.wifi_tether_configure_ssid_default);
    }

    public boolean needRandomSsid() {
        return false;
    }
    
    public void setCustomizedWifiSleepPolicy(Context context) {    
    }

    public boolean handleNetworkReselection() {
        return false;
    }
}
