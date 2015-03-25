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

package com.mtk.telephony;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.RemoteViews;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ITelephony;
import android.os.SystemProperties;
import android.os.ServiceManager;

import com.mediatek.telephony.TelephonyManagerEx;


public class BSPTelephonyDevToolService extends Service {
    private static final String TAG = "BSP_Telephony_Dev_Service";
    private static final int[] NOTIFICATION_ID_SIM = {0x500, 0x520, 0x540, 0x560};
    private static boolean mIsRunning;

    private TelephonyManager mTelephonyManager;
    private TelephonyManagerEx mTelephonyManagerEx;
    private ITelephony mTelephony;
    private NotificationManager mNotificationManager;
    private Notification[] mSimNotification = new Notification[PhoneConstants.GEMINI_SIM_NUM];
    private SignalStrength[] mSimSignalStrength = new SignalStrength[PhoneConstants.GEMINI_SIM_NUM];
    private int[] mSimDataNetworkType = new int[PhoneConstants.GEMINI_SIM_NUM];
    private int[] mSimDataDirection = new int[PhoneConstants.GEMINI_SIM_NUM];
    private GeminiPhoneStateListener[] mPhoneStateListener = new GeminiPhoneStateListener[PhoneConstants.GEMINI_SIM_NUM];

    private class GeminiPhoneStateListener extends PhoneStateListener {
        int mSimId;

        public GeminiPhoneStateListener(int simId) {
            mSimId = simId;
        }

        @Override
        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            mSimSignalStrength[mSimId] = signalStrength;
            updateNotifications(mSimId);
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            updateNotifications(mSimId);
        }

        @Override
        public void onDataConnectionStateChanged(int state, int networkType) {
            mSimDataNetworkType[mSimId] = networkType;
            updateNotifications(mSimId);
        }

        @Override
        public void onDataActivity(int direction) {
            mSimDataDirection[mSimId] = direction;
            updateNotifications(mSimId);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "BSP package telephony dev service started");
        mIsRunning = true;
        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManagerEx = TelephonyManagerEx.getDefault();
        mTelephony = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
            new Intent(this, BSPTelephonyDevToolActivity.class), 0);

        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mSimNotification[i] = new Notification();
            mSimDataNetworkType[i] = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            mSimDataDirection[i] = TelephonyManager.DATA_ACTIVITY_NONE;
            mPhoneStateListener[i] = new GeminiPhoneStateListener(i);
            
            mTelephonyManagerEx.listen(mPhoneStateListener[i],
                    PhoneStateListener.LISTEN_SIGNAL_STRENGTHS |
                    PhoneStateListener.LISTEN_SERVICE_STATE |
                    PhoneStateListener.LISTEN_DATA_CONNECTION_STATE |
                    PhoneStateListener.LISTEN_DATA_ACTIVITY, i);
            mSimNotification[i].flags = Notification.FLAG_NO_CLEAR;
            mSimNotification[i].contentIntent = contentIntent;
            mSimNotification[i].icon = R.drawable.ic_launcher;

            updateNotifications(i);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        for (int i=0; i<PhoneConstants.GEMINI_SIM_NUM; i++) {
            mTelephonyManagerEx.listen(mPhoneStateListener[i], PhoneStateListener.LISTEN_NONE, i);
            mNotificationManager.cancel(NOTIFICATION_ID_SIM[i]);
        }
        mIsRunning = false;

        Log.i(TAG, "BSP package telephony dev service stopped");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    static public boolean isRunning() {
        return mIsRunning;
    }

    private void updateNotifications(int simId) {
        mSimNotification[simId].contentView = getNotificationRemoteViews(simId);
        mNotificationManager.notify(NOTIFICATION_ID_SIM[simId], mSimNotification[simId]);
    }

    private RemoteViews getNotificationRemoteViews(int simId) {
        RemoteViews remoteViews = new RemoteViews(getPackageName(), R.layout.notification);
        String simIdText = getString(R.string.sim) + (simId+1);
        remoteViews.setTextViewText(R.id.notification_sim_id, simIdText);
        remoteViews.setTextViewText(R.id.notification_network_type,
                Utility.getNetworkTypeString(mTelephonyManagerEx.getNetworkType(simId)));

        int signalStrength = 0;

        remoteViews.setTextViewText(R.id.notification_data_activity,
                    Utility.getDataDirectionString(mSimDataDirection[simId]));

        if (mTelephonyManagerEx.getDataState(simId) == TelephonyManager.DATA_CONNECTED) {
            remoteViews.setTextViewText(R.id.notification_data_connection_type,
                    Utility.getNetworkTypeString(mSimDataNetworkType[simId]));
        } else {
            remoteViews.setTextViewText(R.id.notification_data_connection_type, "");
        }

        if (mSimSignalStrength[simId] != null)
            signalStrength = mSimSignalStrength[simId].getGsmSignalStrength();

        if (signalStrength == 99)
            signalStrength = 0;
        remoteViews.setProgressBar(R.id.notification_progress_signal, 31, signalStrength, false);

        return remoteViews;
    }
}
