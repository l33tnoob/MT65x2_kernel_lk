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

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.LinkedList;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.android.internal.telephony.Phone;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;

/**
 * The class is used to test RcseOnlyApnUtils
 */
public class RcseOnlyApnUtilsTest extends InstrumentationTestCase {
    private static final String TAG = "RcseOnlyApnUtilsTest";

    private Context mContext = null;
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private RcseOnlyApnUtils mUtils = null;
    private ConnectivityManager mConnManager = null;
    
    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = this.getInstrumentation().getTargetContext();
        mUtils = RcseOnlyApnUtils.getInstance();
        mConnManager = (ConnectivityManager) mContext
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        Logger.d(TAG, "setUp() entry");
    }
    
    public void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test case for setMockedRoamingState()
     */
    public void testCase1_SetMockedRoamingState() throws NoSuchFieldException,
            IllegalAccessException {
        Logger.d(TAG, "testCase1_IsMobileNetwork() entry");
        Field field = Utils.getPrivateField(RcseOnlyApnUtils.class, "mMockedRomingState");
        field.set(mUtils, Boolean.valueOf(false));
        mUtils.setMockedRoamingState(true);
        Boolean boolVal = (Boolean) field.get(mUtils);
        assertTrue(boolVal.booleanValue());
        Logger.d(TAG, "testCase1_IsMobileNetwork() exit");
    }
    
    /**
     * Test case for isMobileNetwork()
     */
    public void testCase2_IsMobileNetwork() throws NoSuchFieldException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase2_IsMobileNetwork() entry");
        Method method = Utils.getPrivateMethod(RcseOnlyApnUtils.class, "isMobileNetwork",
                NetworkInfo.class);
        NetworkInfo info = null;
        Boolean retVal = (Boolean) method.invoke(mUtils, info);
        assertFalse(retVal.booleanValue());
        info = mConnManager.getActiveNetworkInfo();
        retVal = (Boolean) method.invoke(mUtils, info);
        if (null != info && info.getType() == ConnectivityManager.TYPE_MOBILE) {
            assertTrue(retVal.booleanValue());
        } else {
            assertFalse(retVal.booleanValue());
        }
        Logger.d(TAG, "testCase2_IsMobileNetwork() exit");
    }
    
    /**
     * Test case for isMobileRoaming()
     */
    public void testCase3_IsMobileRoaming() throws NoSuchFieldException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase3_IsMobileRoaming() entry");
        Method method = Utils.getPrivateMethod(RcseOnlyApnUtils.class, "isMobileRoaming",
                NetworkInfo.class);
        NetworkInfo info = null;
        Boolean retVal = (Boolean) method.invoke(mUtils, info);
        assertFalse(retVal.booleanValue());
        info = mConnManager.getActiveNetworkInfo();
        retVal = (Boolean) method.invoke(mUtils, info);
        if (null != info && info.isRoaming()) {
            assertTrue(retVal.booleanValue());
        } else {
            assertFalse(retVal.booleanValue());
        }
        Logger.d(TAG, "testCase3_IsMobileRoaming() exit");
    }
    
    /**
     * Test case for setRcsOnlyApnStarted() & isRcsOnlyApnStarted();
     */
    public void testCase4_RcsOnlyApnStarted() throws NoSuchFieldException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase4_RcsOnlyApnStarted() entry");

        Method method = Utils.getPrivateMethod(RcseOnlyApnUtils.class, "setRcsOnlyApnStarted",
                Boolean.class);
        method.invoke(mUtils, Boolean.TRUE);
        Field field = Utils.getPrivateField(RcseOnlyApnUtils.class, "mRcsOnlyApnStarted");
        Boolean retVal = (Boolean) field.get(mUtils);
        assertTrue(retVal.booleanValue());

        field.set(mUtils, Boolean.FALSE);
        assertFalse(mUtils.isRcsOnlyApnStarted());
        Logger.d(TAG, "testCase4_RcsOnlyApnStarted() exit");
    }
    
    /**
     * Test case for startUsingRcseOnlyApn()
     */
    public void testCase5_StartUsingRcseOnlyApn() throws NoSuchFieldException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase5_StartUsingRcseOnlyApn() entry");
        Field field = Utils.getPrivateField(RcseOnlyApnUtils.class, "mRcsOnlyApnStarted");
        Method method = Utils.getPrivateMethod(RcseOnlyApnUtils.class, "startUsingRcseOnlyApn");
        field.set(mUtils, Boolean.TRUE);
        int startState = mConnManager.startUsingNetworkFeature(ConnectivityManager.TYPE_MOBILE,
                Phone.FEATURE_ENABLE_RCSE);
        Boolean retVal = (Boolean) method.invoke(mUtils);
        if (startState == 0 || startState == 1) {
            assertTrue(retVal.booleanValue());
        } else {
            assertFalse(retVal.booleanValue());
        }
        Logger.d(TAG, "testCase5_StartUsingRcseOnlyApn() exit");
    }

    /**
     * Test case for stopUsingRcseOnlyApn()
     */
    public void testCase6_StopUsingRcseOnlyApn() throws NoSuchFieldException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase6_StopUsingRcseOnlyApn() entry");
        Field field = Utils.getPrivateField(RcseOnlyApnUtils.class, "mRcsOnlyApnStarted");
        Method method = Utils.getPrivateMethod(RcseOnlyApnUtils.class, "stopUsingRcseOnlyApn");
        field.set(mUtils, Boolean.TRUE);
        Boolean retVal = (Boolean) method.invoke(mUtils);
        assertTrue(retVal.booleanValue());

        field.set(mUtils, Boolean.FALSE);
        NetworkInfo info = mConnManager.getActiveNetworkInfo();
        retVal = (Boolean) method.invoke(mUtils);
        if (null != info) {
            assertTrue(retVal.booleanValue());
        } else {
            assertFalse(retVal.booleanValue());
        }
        Logger.d(TAG, "testCase6_StopUsingRcseOnlyApn() exit");
    }
    
    /**
     * Test case for switchRcseOnlyApn()
     */
    public void testCase7_SwitchRcseOnlyApn() throws NoSuchFieldException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase7_SwitchRcseOnlyApn() entry");
        Field apnDebugMode = Utils.getPrivateField(RcseOnlyApnUtils.class, "mApnDebugMode");
        // Test case for real situation
        apnDebugMode.set(mUtils, Boolean.FALSE);
        NetworkInfo info = mConnManager.getActiveNetworkInfo();
        int retVal = -1;
        if (info == null) {
            retVal = mUtils.switchRcseOnlyApn();
            assertEquals(-1, retVal);
        } else {
            RcsSettings.getInstance().setRcseOnlyApnState(false);
            retVal = mUtils.switchRcseOnlyApn();
            assertEquals(0, retVal);
        }
        // The real situation for startUsingRcseApn() can't be implemented
        // Test case for mock situation
        apnDebugMode.set(mUtils, Boolean.TRUE);
        Field mockedRoamingState = Utils.getPrivateField(RcseOnlyApnUtils.class, "mMockedRomingState");
        mockedRoamingState.set(mUtils, Boolean.TRUE);
        RcsSettings.getInstance().setRcseOnlyApnState(true);
        retVal = mUtils.switchRcseOnlyApn();
        assertEquals(0,retVal);
        mockedRoamingState.set(mUtils, Boolean.FALSE);
        RcsSettings.getInstance().setRcseOnlyApnState(true);
        retVal = mUtils.switchRcseOnlyApn();
        assertEquals(0,retVal);
        mockedRoamingState.set(mUtils, Boolean.TRUE);
        RcsSettings.getInstance().setRcseOnlyApnState(false);
        retVal = mUtils.switchRcseOnlyApn();
        assertEquals(0,retVal);
        Logger.d(TAG, "testCase7_SwitchRcseOnlyApn() exit");
    }
    
    /**
     * Test case for getInstance() & initialize()
     */
    public void testCase8_GetinitializedInstance() throws NoSuchFieldException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase8_GetinitializedInstance() entry");
        assertNotNull(mUtils);
        Field context = Utils.getPrivateField(RcseOnlyApnUtils.class, "mContext");
        //mUtils.initialize(mContext);
        Context utilsContext = (Context) context.get(mUtils);
        assertNotNull(utilsContext);
        Logger.d(TAG, "testCase8_GetinitializedInstance() exit");
    }
}
