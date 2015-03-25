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

package com.mediatek.rcse.test.apn;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.apn.IRcseOnlyApnStatus;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

/**
 * The class is used to test RcseOnlyApnStatusService
 */
public class RcseOnlyApnStatusServiceTest extends InstrumentationTestCase {
    private static final String TAG = "RcseOnlyApnStatusServiceTest";

    private Context mContext = null;
    private IRcseOnlyApnStatus mRcseOnlyApnStatusService = null;
    private ServiceConnection mServiceConnection = null;
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private static final String SERVICE_ACTION = "com.mediatek.apn.plugin.RCSE_ONLY_APN_SERVICE";

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mServiceConnection = new ServiceConnection() {
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
        Intent intent = new Intent();
        intent.setAction(SERVICE_ACTION);
        mContext.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
        
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                try {
                    if (mRcseOnlyApnStatusService != null) {
                        break;
                    } else {
                        Thread.sleep(THREAD_SLEEP_PERIOD);
                    }
                } catch (InterruptedException e) {
                    this.fail(e.toString());
                }
            }
        }
        Logger.d(TAG, "setUp() entry");
    }
    
    public void tearDown() throws Exception {
        super.tearDown();
        mContext.unbindService(mServiceConnection);
    }

    /**
     * Test case for onBind()
     */
    public void testCase1_OnBind() {
        Logger.d(TAG, "testCase1_OnBind() entry");
        assertNotNull(mRcseOnlyApnStatusService);
        Logger.d(TAG, "testCase1_OnBind() exit");
    }

    /**
     * Test case for isRcseOnlyApnEnabled()
     */
    public void testCase2_IsRcseOnlyApnEnabled() throws RemoteException {
        Logger.d(TAG, "testCase2_IsRcseOnlyApnEnabled() entry");
        RcsSettings.getInstance().setRcseOnlyApnState(true);
        long startTime = System.currentTimeMillis();
        boolean retVal = false;
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                retVal = RcsSettings.getInstance().isRcseOnlyApnEnabled();
                Logger.d(TAG, "retVal = " + retVal);
                try {
                    if (retVal) {
                        break;
                    } else {
                        Thread.sleep(THREAD_SLEEP_PERIOD);
                    }
                } catch (InterruptedException e) {
                    this.fail("Write data base fail");
                }
            }
        }
        boolean status = mRcseOnlyApnStatusService.isRcseOnlyApnEnabled();
        boolean compare = (retVal == status);
        assertTrue(compare);
        Logger.d(TAG, "testCase2_IsRcseOnlyApnEnabled() exit");
    }
}
