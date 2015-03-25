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

import android.content.Context;
import android.content.ServiceConnection;
import android.os.RemoteException;
import android.test.AndroidTestCase;
import android.view.View;

import com.mediatek.rcse.api.FlightModeApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.service.ICapabilities;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;

import java.lang.reflect.Field;

/**
 * Test case for RegistrationApi
 *
 */
public class RegistrationApiTest extends AndroidTestCase {
    private static final String TAG = "RegistrationApi";
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private RegistrationApi mApi = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApi = new RegistrationApi(this.getContext());
        RcsSettings.createInstance(this.getContext());
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown()");
        super.tearDown();
        Utils.clearAllStatus();
    }

    /**
     * Test RegistrationApi()
     */
    public void testCase01_RegistrationApi() {
        Logger.d(TAG, "testCase01_RegistrationApi() entry");
        Context context = null;
        RegistrationApi api = null;
        RuntimeException runtimeException = null;
        try {
            api = new RegistrationApi(context);
        } catch (RuntimeException e) {
            runtimeException = e;
        } finally {
            assertNotNull(runtimeException);
        }
        assertNotNull(mApi);
        Logger.d(TAG, "testCase01_RegistrationApi() exit");
    }
    
    /**
     * Test connect()
     */
    public void testCase02_connect() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase02_connect() entry");

        Field registrationStatus = RegistrationApi.class.getDeclaredField("mRegistrationStatus");
        registrationStatus.setAccessible(true);
        registrationStatus.set(mApi, null);
        mApi.connect();
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                IRegistrationStatus service = (IRegistrationStatus) registrationStatus.get(mApi);
                if(service != null){
                    break;
                }else{
                    Thread.sleep(THREAD_SLEEP_PERIOD); 
                }
            }
        }
        mApi.disconnect();
        Logger.d(TAG, "testCase02_connect() exit");
    }
    
    /**
     * Test onServiceDisconnected()
     */
    public void testCase03_onServiceDisconnected() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase03_onServiceDisconnected() entry");

        Field registrationConnField = RegistrationApi.class
                .getDeclaredField("mRegistrationStatusConnection");
        registrationConnField.setAccessible(true);
        ServiceConnection conn = (ServiceConnection) registrationConnField.get(mApi);
        conn.onServiceDisconnected(null);
        Field registrationStatus = RegistrationApi.class.getDeclaredField("mRegistrationStatus");
        registrationStatus.setAccessible(true);
        IRegistrationStatus service = (IRegistrationStatus) registrationStatus.get(mApi);
        assertNull(service);
        Logger.d(TAG, "testCase03_onServiceDisconnected() exit");
    }

    /**
     * Test start() and stop()
     */
    public void testCase04_startAndStop() {
        Logger.d(TAG, "testCase04_startAndStop() entry");
        assertNotNull(mApi);
        assertFalse(mApi.start(null));
        assertTrue(mApi.start(this.getContext()));
        assertFalse(mApi.stop(null));
        assertTrue(mApi.stop(this.getContext()));
        Logger.d(TAG, "testCase04_startAndStop() exit");
    }
}
