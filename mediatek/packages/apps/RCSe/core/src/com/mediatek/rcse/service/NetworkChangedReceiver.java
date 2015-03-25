/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;

import com.mediatek.rcse.activities.RoamingActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
/**
 * This class defined to receive the roaming state and network change broadcast.
 */
public class NetworkChangedReceiver extends BroadcastReceiver {

    private static final String TAG = "NetworkChangedReceiver";
    private static boolean sDeviceLastRoamingStatus = false;
    // 2G network types
    private static final String GSM = "gsm";
    private static final String GPRS = "gprs";
    // 2.5G network types
    private static final String CDMA = "cdma";
    
    // 2.75G network types
    private static final String EDGE = "edge";
    
    // 3G network types
    private static final String UMTS = "umts";
    private static final String HSPA = "hspa";
    private static final String HSUPA = "hsupa";
    private static final String HSDPA = "hsdpa";
    private static final String ONEXRTT = "1xrtt";
    private static final String EHRPD = "ehrpd";
    // 4G network types
    private static final String LTE = "lte";
    private static final String UMB = "umb";
    private static final String HSPA_PLUS = "hspa+";

    private Context mContext = null;

    @Override
    public void onReceive(final Context context, Intent intent) {
        mContext = context;
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        final NetworkInfo info = cm.getActiveNetworkInfo();
        boolean isNetworkActived = false;
        if (intent == null) {
            Logger.d(TAG, "onReceive()-intent is null");
            return;
        }
        String action = intent.getAction();
        if (action == null) {
            Logger.d(TAG, "onReceive()-action is null");
            return;
        }
        if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
            Logger.d(TAG, "onReceive()-CONNECTIVITY_ACTION");
            ApiManager.initialize(context.getApplicationContext());
            handleFtCapabilityChanged(info);
            if (info == null) {
                Logger.d(TAG, "onReceive()-info is null");
                for (OnNetworkStatusChangedListerner listerner : LISTENER_LIST) {
                    listerner.onNetworkStatusChanged(false);
                }
                return;
            }

            if (info.getType() == ConnectivityManager.TYPE_MOBILE) {
                boolean isConnected = info.isConnected();
                Logger.d(TAG, "onReceive()-it's TYPE_MOBILE network and is "
                        + (isConnected ? "connected" : "not connected"));
                if (isConnected) {
                    isNetworkActived = true;
                    RcseOnlyApnUtils.getInstance().switchRcseOnlyApn();
                    if (RcseOnlyApnUtils.getInstance().isRcsOnlyApnStarted()) {
                        ensureApnRouteToHost(info);
                            }
   //                 handleRoamingNotification(info);
                            }
            } else if (info.getType() == ConnectivityManager.TYPE_WIFI) {
                boolean isConnected = info.isConnected();
                Logger.d(TAG, "onReceive()-it's TYPE_WIFI network and is "
                        + (isConnected ? "connected" : "not connected"));
                if (isConnected) {
                    isNetworkActived = true;
        }
                if (PhoneUtils.sIsApnDebug) {
                    Logger.d(TAG, "onReceive()-it's MOCKED_TYPE_MOBILE network");
                    RcseOnlyApnUtils.getInstance().switchRcseOnlyApn();
                    if (RcseOnlyApnUtils.getInstance().isRcsOnlyApnStarted()) {
                        ensureApnRouteToHost(info);
        }
            }
            } else {
                Logger.d(TAG, "onReceive()-other network type");
                boolean isConnected = info.isConnected();
                Logger.d(TAG, "onReceive()-it's other network and is "
                        + (isConnected ? "connected" : "not connected"));
                if (isConnected) {
                    isNetworkActived = true;
                }
                Logger.d(TAG, "onReceive() getActiveNetworkInfo activeNetworkInfo: "
                        + info + " isConnected: " + isConnected);
            }
            // it's reasonable?
            for (OnNetworkStatusChangedListerner listerner : LISTENER_LIST) {
                listerner.onNetworkStatusChanged(isNetworkActived);
            }
        } else if (action.equals(RcsSettings.RCSE_ONLY_APN_ACTION)
                || action.equals(RcseOnlyApnUtils.ROAMING_MOCKED_ACTION)) {
            Logger.d(TAG, "onReceive()-RCSE_ONLY_APN_ACTION");
            RcseOnlyApnUtils.getInstance().switchRcseOnlyApn();
        } else {
            Logger.d(TAG, "onReceive()-other action");
        }
    }

    // Handle file transfer capability exchange
    private void handleFtCapabilityChanged(NetworkInfo info) {
        if (info == null) {
            Logger.d(TAG, "handleFtCapabilityChanged()-info is null");
            return;
        }
        int networkType = info.getType();
        String type = info.getSubtypeName();
        Logger.d(TAG, "handleFtCapabilityExchange()-current network sub-type name = " + type);
        if (networkType == ConnectivityManager.TYPE_MOBILE) {
            if (type.toLowerCase().equals(GSM) || type.toLowerCase().equals(GPRS)) {
                if (RcsSettings.getInstance().isFileTransferSupported()) {
                    RcsSettings.getInstance().setSupportFileTransfer(false);
                    exchangeMyFtCapability(false);
                }
            } else if (type.toLowerCase().equals(EDGE) || type.toLowerCase().startsWith(CDMA)
                    || type.toLowerCase().equals(UMTS) || type.toLowerCase().equals(ONEXRTT)
                    || type.toLowerCase().equals(EHRPD) || type.toLowerCase().equals(HSUPA)
                    || type.toLowerCase().equals(HSDPA) || type.toLowerCase().equals(HSPA)
                    || type.toLowerCase().equals(HSPA_PLUS) || type.toLowerCase().equals(UMB)
                    || type.toLowerCase().equals(LTE)) {
                if (!RcsSettings.getInstance().isFileTransferSupported()) {
                    RcsSettings.getInstance().setSupportFileTransfer(true);
                    exchangeMyFtCapability(true);
                }
            } else {
                Logger.d(TAG, "handleFtCapabilityExchange()-other network sub-type name.");
            }
        } else if (networkType == ConnectivityManager.TYPE_WIFI) {
            if (!RcsSettings.getInstance().isFileTransferSupported()) {
                RcsSettings.getInstance().setSupportFileTransfer(true);
                exchangeMyFtCapability(true);
            }
        } else {
            Logger.d(TAG, "handleFtCapabilityExchange()-other network type");
        }
    }

    private void exchangeMyFtCapability(boolean isFtCapabilityEnable) {
        ExchangeMyCapability exchangeMyCapability = ExchangeMyCapability.getInstance(mContext);
        if (exchangeMyCapability == null) {
            Logger.d(TAG, "exchangeMyCapability()-exchangeMyCapability is null");
        } else {
            Logger.d(TAG, "exchangeMyCapability()-exchangeMyCapability is not null");
            exchangeMyCapability.notifyCapabilityChanged(
                    ExchangeMyCapability.FILE_TRANSFER_CAPABILITY_CHANGE, isFtCapabilityEnable);
        }
    }
/*
    // Handle roaming notification
    private void handleRoamingNotification(final NetworkInfo info) {
        if (info == null) {
            Logger.d(TAG, "handleRoamingNotification()-info is null");
            return;
        }
        if (info.getType() != ConnectivityManager.TYPE_MOBILE) {
            Logger.d(TAG, "handleRoamingNotification()-netwotk type is not TYPE_MOBILE");
            return;
        }
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... arg0) {
                if (info.isRoaming()) {
                    // Display roaming dialog
                    if (!sDeviceLastRoamingStatus) {
                        if (RcsSettings.getInstance().isRoamingAuthorized()) {
                            showRoamingNofitication(mContext, RoamingActivity.ROAMING_WITH_ENABLE);
                        } else {
                            showRoamingNofitication(mContext, RoamingActivity.ROAMING_WITH_DISABLE);
                        }
                        sDeviceLastRoamingStatus = true;
                    }
                } else {
                    if (sDeviceLastRoamingStatus) {
                        showRoamingNofitication(mContext, RoamingActivity.UNROAMING);
                        sDeviceLastRoamingStatus = false;
                    }
                }
                return null;
            }
        }.execute();
    }
*/
  /*  private void showRoamingNofitication(Context context, int type) {
        Intent intent = new Intent();
        intent.setAction(RoamingActivity.ROAMING_MESSAGE_DIALOG_ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(RoamingActivity.EXTRA_ACTION_TYPE, type);
        context.startActivity(intent);
    }*/

    // Ensure request route to host
    private void ensureApnRouteToHost(NetworkInfo info) {
        Logger.d(TAG, "ensureApnRouteToHost() called");
        if (info == null) {
            Logger.d(TAG, "ensureApnRouteToHost()-info is null");
        } else {
            if (info.getState() == NetworkInfo.State.CONNECTED) {
                ConnectivityManager connManager = (ConnectivityManager) mContext
                        .getSystemService(Context.CONNECTIVITY_SERVICE);
                RcseOnlyApnUtils.getInstance().routeToHost(connManager);
            } else {
                Logger.d(TAG, "ConnectivityBroadcastReceiver.onReceive() "
                        + "network is no connected");
            }
        }
    }
    
    /**
     * This interface defines a listener to handle the network connection status changes.
     */
    public interface OnNetworkStatusChangedListerner {
        /**
         * Called when network connection status changes.
         */
        void onNetworkStatusChanged(boolean isConnected);
    }

    private static final List<OnNetworkStatusChangedListerner> LISTENER_LIST = 
            new CopyOnWriteArrayList<OnNetworkStatusChangedListerner>();

    /**
     * Add an OnNetworkStatusChangedListerner listener.
     * 
     * @param listener The listener be added.
     */
    public static void addListener(OnNetworkStatusChangedListerner listener) {
        Logger.d(TAG, "addListener() entry");
        LISTENER_LIST.add(listener);
        Logger.d(TAG, "addListener() exit,the list size is " + LISTENER_LIST.size());
    }

    /**
     * Remove an OnNetworkStatusChangedListerner listener.
     * 
     * @param listener The listener be removed.
     */
    public static void removeListener(OnNetworkStatusChangedListerner listener) {
        Logger.d(TAG, "removeListener() entry");
        LISTENER_LIST.remove(listener);
        Logger.d(TAG, "removeListener() exit,the list size is " + LISTENER_LIST.size());
    }
}
