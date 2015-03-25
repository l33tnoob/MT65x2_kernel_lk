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

package com.mediatek.common.wifi;

import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;

import java.util.List;

public interface IWifiFwkExt {
    String AUTOCONNECT_SETTINGS_CHANGE = "com.mediatek.common.wifi.AUTOCONNECT_SETTINGS_CHANGE";
    String AUTOCONNECT_ENABLE_ALL_NETWORKS = "com.mediatek.common.wifi.AUTOCONNECT_ENABLE_ALL_NETWORK";
    String RESELECT_DIALOG_CLASSNAME = "com.mediatek.op01.plugin.WifiReselectApDialog";
    String ACTION_RESELECTION_AP = "android.net.wifi.WIFI_RESELECTION_AP";
    String WIFISETTINGS_CLASSNAME = "com.android.settings.Settings$WifiSettingsActivity";
    String WIFI_NOTIFICATION_ACTION = "android.net.wifi.WIFI_NOTIFICATION";
    String EXTRA_NOTIFICATION_SSID = "ssid";
    String EXTRA_NOTIFICATION_NETWORKID = "network_id";
    String EXTRA_SHOW_RESELECT_DIALOG_FLAG = "SHOW_RESELECT_DIALOG";
    long SUSPEND_NOTIFICATION_DURATION = 60 * 60 * 1000;
    int DEFAULT_FRAMEWORK_SCAN_INTERVAL_MS = 15000;
    int BEST_SIGNAL_THRESHOLD = -79;
    int WEAK_SIGNAL_THRESHOLD = -85;
    int MIN_NETWORKS_NUM = 2;
    int BSS_EXPIRE_AGE = 10;
    int BSS_EXPIRE_COUNT = 1;

    int NOTIFY_TYPE_SWITCH = 0;
    int NOTIFY_TYPE_RESELECT = 1;

    void init();
    boolean hasCustomizedAutoConnect();
    boolean shouldAutoConnect();
    boolean isWifiConnecting(int connectingNetworkId, List<Integer> disconnectNetworks);
    boolean hasConnectableAp();
    boolean handleNetworkReselection();
    void suspendNotification(int type);
    int defaultFrameworkScanIntervalMs();
    int getSecurity(WifiConfiguration config);
    int getSecurity(ScanResult result);
    String getApDefaultSsid();
    boolean needRandomSsid();
    void setCustomizedWifiSleepPolicy(Context context);
}
