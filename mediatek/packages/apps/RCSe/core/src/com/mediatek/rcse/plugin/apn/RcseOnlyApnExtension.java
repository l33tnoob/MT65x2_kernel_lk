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

package com.mediatek.rcse.plugin.apn;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.settings.ext.IRcseOnlyApnExtension;
import com.mediatek.settings.ext.IRcseOnlyApnExtension.OnRcseOnlyApnStateChangedListener;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.util.ArrayList;

/**
 * This class defined to implement the function interface of IRcseOnlyApnExtension,
 * and achieve the main function here
 */
public class RcseOnlyApnExtension extends ContextWrapper implements IRcseOnlyApnExtension {

    private static final String TAG = "RcseOnlyApnExtension";
    private static final String SERVICE_ACTION = "com.mediatek.apn.plugin.RCSE_ONLY_APN_SERVICE";
    private final ArrayList<OnRcseOnlyApnStateChangedListener> mListenerList 
        = new ArrayList<OnRcseOnlyApnStateChangedListener>();
    private IRcseOnlyApnStatus mRcseOnlyApnStatusService = null;
    private ExtensionApnStatusReceiver mReceiver = null;
    
    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            Logger.d(TAG, "onServiceConnected() is called");
            mRcseOnlyApnStatusService = IRcseOnlyApnStatus.Stub.asInterface(arg1);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Logger.d(TAG, "onServiceDisconnected() is called");
            mRcseOnlyApnStatusService = null;
        }
    };
    
    public RcseOnlyApnExtension(Context context) {
        super(context);
        Logger.d(TAG, "RcseOnlyApnExtension() is called");
        Intent intent = new Intent();
        intent.setAction(SERVICE_ACTION);
        context.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);

        mReceiver = new ExtensionApnStatusReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(RcsSettings.RCSE_ONLY_APN_ACTION);
        this.registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void addRcseOnlyApnStateChanged(OnRcseOnlyApnStateChangedListener listener) {
        Logger.d(TAG, "addRcseOnlyApnStateChanged() is called");
        if (listener != null) {
            mListenerList.add(listener);
        } else {
            Logger.d(TAG, "addRcseOnlyApnStateChanged()-The listener is null");
        }
    }

    @Override
    public void removeRcseOnlyApnStateChanged(OnRcseOnlyApnStateChangedListener listener) {
        Logger.d(TAG, "removeRcseOnlyApnStateChanged() is called");
        if (listener != null) {
            mListenerList.remove(listener);
        } else {
            Logger.d(TAG, "removeRcseOnlyApnStateChanged()-The listener is null");
        }
    }

    @Override
    public boolean isRcseOnlyApnEnabled() {
        boolean status = false;
        try {
            status = mRcseOnlyApnStatusService.isRcseOnlyApnEnabled();
        } catch (RemoteException e) {
            Logger.d(TAG, "isRcseOnlyApnEnabled()-RemoteException");
            e.printStackTrace();
        } finally {
            Logger.d(TAG, "isRcseOnlyApnEnabled()-the status is:" + status);
            return status;
        }
    }
    
    /**
     * Defined this class to receive the RCS-e only APN changed broadcast
     */
    public class ExtensionApnStatusReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(RcsSettings.RCSE_ONLY_APN_ACTION)) {
                boolean status = intent.getBooleanExtra(RcsSettings.RCSE_ONLY_APN_STATUS, false);
                Logger.e(TAG, "onReceive()-current RCS-e only APN status is " + status);
                for (OnRcseOnlyApnStateChangedListener listener : mListenerList) {
                    Logger.d(TAG, "notifyListeners() notify the listener");
                    listener.onRcseOnlyApnStateChanged(status);
                }
            }
        }
    }
}
