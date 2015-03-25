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

package com.mediatek.rcse.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.rcse.service.IFlightMode;
import com.orangelabs.rcs.service.api.client.ClientApi;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * This class provides an access to the RCS-e Registration Manager.
 */
public class FlightModeApi extends ClientApi {
    public static final String TAG = "FlightModeApi";
    private IFlightMode mFlightModeApi = null;
    private static final long TIME_OUT = 200;
    private static final int WAIT_NUMBER = 1;
    private CountDownLatch mApiReadyCountDownLatch = new CountDownLatch(WAIT_NUMBER);

    /**
     * Constructor of RegistrationManagerApi.
     * 
     * @param context This Context should not be null.
     */
    public FlightModeApi(Context context) {
        super(context);
        Logger.d(TAG, "RegistrationManagerApi");
    }

    protected ServiceConnection mApiConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Logger.v(TAG, "onServiceConnected() entry");
            mFlightModeApi = IFlightMode.Stub.asInterface(service);
            mApiReadyCountDownLatch.countDown();
            notifyEventApiConnected();
            Logger.v(TAG, "onServiceConnected() exit");
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            Logger.v(TAG, "onServiceDisconnected() entry");
            notifyEventApiDisconnected();
            mFlightModeApi = null;
            Logger.v(TAG, "onServiceDisconnected() exit");
        }
    };

    /**
     * Connect API
     */
    public void connectApi() {
        Logger.d(TAG, "connectApi()");
        super.connectApi();
        ctx.bindService(new Intent(IFlightMode.class.getName()), mApiConnection, 0);
    }

    /**
     * Disconnect API
     */
    public void disconnectApi() {
        Logger.d(TAG, "disconnectApi()");
        mFlightModeApi = null;
        super.disconnectApi();
        ctx.unbindService(mApiConnection);
    }

    /**
     * @param expire
     */
    public void doRegister(int expire) {
        Logger.d(TAG, "doRegister(), expire = " + expire);
        if (mFlightModeApi != null) {
            try {
                mFlightModeApi.doRegister(expire);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Logger.d(TAG, "mRegistrationManagerApi is null");
        }
    }

    /**
     * Returns the core service API
     * 
     * @return API
     */
    public IFlightMode getCoreServiceApi() {
        Logger.d(TAG, "getCoreServiceApi() entry.");
        try {
            Logger.d(TAG, "Wait api connected.");
            boolean success = mApiReadyCountDownLatch.await(TIME_OUT, TimeUnit.MILLISECONDS);
            Logger.d(TAG, "api has connected, or timeout. mFlightModeApi = " + mFlightModeApi
                    + ", success = " + success);
            return mFlightModeApi;
        } catch (InterruptedException e) {
            Logger.w(TAG, "wait api connected failed.");
            e.printStackTrace();
        }
        return null;
    }
}
