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

package com.mediatek.rcse.test.api;

import android.content.ServiceConnection;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.FlightModeApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.IFlightMode;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;

import java.lang.reflect.Field;

/**
 * Test case for FlightModeAPi
 *
 */
public class FlightModeApiTest extends AndroidTestCase {
    private static final String TAG = "FlightModeApiTest";
    private static final int TIME_OUT_API = 3000;
    private FlightModeApi mFlightModeApi;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        RcsSettings.createInstance(mContext);
        mFlightModeApi = new FlightModeApi(mContext);
        LauncherUtils.launchRcsCoreService(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown()");
        super.tearDown();
        LauncherUtils.stopRcsService(mContext);
        Utils.clearAllStatus();
    }

    /**
     * Test connectApi()
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws RemoteException
     * @throws InterruptedException
     */
    public void testCase01_connectApi() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, RemoteException, InterruptedException {
        Logger.d(TAG, "testCase01_connectApi()");
        assertNotNull(mFlightModeApi);
        IFlightMode flightMode = null;
        mFlightModeApi.doRegister(0);
        assertNull(flightMode);
        
        waitForFlightModeApiConnected();
        mFlightModeApi.connectApi();
        flightMode = mFlightModeApi.getCoreServiceApi();
        assertNotNull(flightMode);
        mFlightModeApi.doRegister(0);
        mFlightModeApi.disconnectApi();
        waitForFlightModeApiDisconnected();
        flightMode = mFlightModeApi.getCoreServiceApi();
        assertNull(flightMode);

    }
    
    /**
     * Test onServiceDisconnected()
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws RemoteException
     * @throws InterruptedException
     */
    public void testCase02_onServiceDisconnected() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, RemoteException, InterruptedException {
        Logger.d(TAG, "testCase02_onServiceDisconnected()");
        
        waitForFlightModeApiConnected();
        mFlightModeApi.connectApi();
        IFlightMode flightMode = mFlightModeApi.getCoreServiceApi();
        assertNotNull(flightMode);
        mFlightModeApi.doRegister(0);
        
        Field field = FlightModeApi.class.getDeclaredField("mApiConnection");
        field.setAccessible(true);
        ServiceConnection apiConnection = (ServiceConnection)field.get(mFlightModeApi);
        apiConnection.onServiceDisconnected(null);
       
        waitForFlightModeApiDisconnected();
        flightMode = mFlightModeApi.getCoreServiceApi();
        assertNull(flightMode);
    }

    private void waitForFlightModeApiConnected() throws InterruptedException {
        Logger.d(TAG, "waitForFlightModeApiConnected()");
        long beginTime = System.currentTimeMillis();
        while (mFlightModeApi.getCoreServiceApi() == null) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT_API) {
                break;
            }
            Thread.sleep(200);
        }
    }

    private void waitForFlightModeApiDisconnected() throws InterruptedException {
        Logger.d(TAG, "waitForFlightModeApiDisconnected()");
        long beginTime = System.currentTimeMillis();
        while (mFlightModeApi.getCoreServiceApi() != null) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT_API) {
                break;
            }
            Thread.sleep(200);
        }
    }
}
