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

import com.mediatek.settings.ext.IRcseOnlyApnExtension;
import com.mediatek.settings.ext.IRcseOnlyApnExtension.OnRcseOnlyApnStateChangedListener;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.plugin.apn.IRcseOnlyApnStatus;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnExtension;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnExtension.ExtensionApnStatusReceiver;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

import com.mediatek.phone.ext.IPhonePlugin;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.Plugin.ObjectCreationException;
import com.mediatek.pluginmanager.PluginManager;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.LinkedList;

/**
 * The class is used to test RcseOnlyApnExtension
 */
public class RcseOnlyApnExtensionTest extends InstrumentationTestCase {
    private static final String TAG = "RcseOnlyApnExtensionTest";

    private Context mContext = null;
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private static final String SERVICE_ACTION = "com.mediatek.apn.plugin.RCSE_ONLY_APN_SERVICE";
    private boolean mApnEnabled;
    
    private PluginManager mManager = null;
    private IRcseOnlyApnExtension mIExtension = null;
    private RcseOnlyApnExtension mExtension = null;
    
    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = this.getInstrumentation().getTargetContext();
        mExtension = new RcseOnlyApnExtension(mContext);
        PluginManager<IRcseOnlyApnExtension> pm = PluginManager.<IRcseOnlyApnExtension>create(mContext, IRcseOnlyApnExtension.class.getName());
        int count = pm.getPluginCount();
        Logger.d(TAG, "count = " + count);
        if(count < 1){
            fail();
        }
        try {
            Plugin<IRcseOnlyApnExtension> plugIn = pm.getPlugin(0);
            mIExtension = plugIn.createObject();
            assertNotNull(mIExtension);
            
        } catch (ObjectCreationException e) {
            fail(e.toString());
        }
        Logger.d(TAG, "setUp() entry");
    }
    
    public void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test case for RcseOnlyApnExtension()
     */
    public void testCase1_RcseOnlyApnExtension() {
        Logger.d(TAG, "testCase1_RcseOnlyApnExtension() entry");
        assertNotNull(mIExtension);
        Logger.d(TAG, "testCase1_RcseOnlyApnExtension() exit");
    }

    /**
     * Test case for isRcseOnlyApnEnabled()
     */
    public void testCase2_IsRcseOnlyApnEnabled() throws RemoteException {
        Logger.d(TAG, "testCase2_IsRcseOnlyApnEnabled() entry");
        RcsSettings.getInstance().setRcseOnlyApnState(true);
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean retVal = RcsSettings.getInstance().isRcseOnlyApnEnabled();
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
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean status = mIExtension.isRcseOnlyApnEnabled();
                Logger.d(TAG, "status = " + status);
                try {
                    if (status) {
                        break;
                    } else {
                        Thread.sleep(THREAD_SLEEP_PERIOD);
                    }
                } catch (InterruptedException e) {
                    this.fail("Write data base fail");
                }
            }
        }
        Logger.d(TAG, "testCase2_IsRcseOnlyApnEnabled() exit");
    }
    
    /**
     * Test case for addRcseOnlyApnStateChanged() & removeRcseOnlyApnStateChanged
     */
    public void testCase3_AddAndRemoveRcseOnlyApnStateChanged() throws RemoteException, NoSuchFieldException, IllegalAccessException{
        final OnRcseOnlyApnStateChangedListener listener = new OnRcseOnlyApnStateChangedListener(){
            public void onRcseOnlyApnStateChanged(boolean isEnabled){
            }
        };
        final Field filed = Utils.getPrivateField(RcseOnlyApnExtension.class, "mListenerList");
        final ArrayList<OnRcseOnlyApnStateChangedListener> listenerList = (ArrayList<OnRcseOnlyApnStateChangedListener>)filed.get(mExtension);
        listenerList.clear();
        mExtension.addRcseOnlyApnStateChanged(null);
        assertEquals(0, listenerList.size());
        mExtension.addRcseOnlyApnStateChanged(listener);
        assertEquals(1, listenerList.size());
        mExtension.removeRcseOnlyApnStateChanged(null);
        assertEquals(1, listenerList.size());
        mExtension.removeRcseOnlyApnStateChanged(listener);
        assertEquals(0, listenerList.size());
    }
    
    /**
     * Test case for onReceive()
     */
    public void testCase4_OnReceive() throws RemoteException, NoSuchFieldException, IllegalAccessException {
        Field filedReceiver = Utils.getPrivateField(RcseOnlyApnExtension.class, "mReceiver");
        ExtensionApnStatusReceiver receiver = (ExtensionApnStatusReceiver)filedReceiver.get(mExtension);
        Intent intent = new Intent();
        intent.setAction(RcsSettings.RCSE_ONLY_APN_ACTION);
        intent.putExtra(RcsSettings.RCSE_ONLY_APN_STATUS, true);
       
        final OnRcseOnlyApnStateChangedListener listener = new OnRcseOnlyApnStateChangedListener(){
            public void onRcseOnlyApnStateChanged(boolean isEnabled){
                mApnEnabled = isEnabled;
            }
        };
        Field filed = Utils.getPrivateField(RcseOnlyApnExtension.class, "mListenerList");
        ArrayList<OnRcseOnlyApnStateChangedListener> listenerList = (ArrayList<OnRcseOnlyApnStateChangedListener>)filed.get(mExtension);
        listenerList.clear();
        mExtension.addRcseOnlyApnStateChanged(listener);
        
        receiver.onReceive(mContext,intent);
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                try {
                    if (mApnEnabled) {
                        break;
                    } else {
                        Thread.sleep(THREAD_SLEEP_PERIOD);
                    }
                } catch (InterruptedException e) {
                    this.fail("Write data base fail");
                }
            }
        }
    }
    
    /**
     * Test case for onServiceDisconnected()
     */
    public void testCase5_OnServiceDisconnected() throws RemoteException, NoSuchFieldException,
            IllegalAccessException {
        Field filedReceiver = Utils.getPrivateField(RcseOnlyApnExtension.class,
                "mServiceConnection");
        ServiceConnection conn = (ServiceConnection) filedReceiver.get(mExtension);

        Field filedService = Utils.getPrivateField(RcseOnlyApnExtension.class,
                "mRcseOnlyApnStatusService");

        conn.onServiceDisconnected(null);
        IRcseOnlyApnStatus service = (IRcseOnlyApnStatus) filedService.get(mExtension);
        assertNull(service);
    }
}
