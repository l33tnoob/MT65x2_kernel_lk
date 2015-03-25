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

package com.mediatek.rcse.test.service;

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.test.InstrumentationTestCase;
import android.view.KeyEvent;

import com.mediatek.rcse.activities.RoamingActivity;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class is used to test NetworkChangedReceiver.java
 */
public class NetworkChangedReceiverTest extends InstrumentationTestCase {
    private static final String TAG = "NetworkChangedReceiverTest";

    private NetworkChangedReceiver mNetworkChangedReceiver = new NetworkChangedReceiver();
    private boolean mFileTransferCapability = false;
    private Context mContext = null;
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String MESSAGE_API = "mMessagingApi";
    private static final String EXCHANGE_FT_CAPABILITY = "exchangeMyFtCapability";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        RcsSettings.createInstance(mContext);
        mFileTransferCapability = RcsSettings.getInstance().isFileTransferSupported();
        Method methodInitialize = Utils.getPrivateMethod(ApiManager.class, API_MANAGER_INITIALIZE,
                Context.class);
        methodInitialize.invoke(null, mContext);
        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp() apiManager is null", apiManager);
        Field messageApiField = Utils.getPrivateField(ApiManager.class, MESSAGE_API);
        messageApiField.set(apiManager, new MessagingApi(mContext));
        MessagingApi messagingApi = apiManager.getMessagingApi();
        assertNotNull("setUp() messagingApi is null", messagingApi);
        messagingApi.connectApi();
        mNetworkChangedReceiver.onReceive(mContext, new Intent(
                ConnectivityManager.CONNECTIVITY_ACTION));
        mNetworkChangedReceiver.onReceive(mContext, new Intent(
                RcseOnlyApnUtils.ROAMING_MOCKED_ACTION));
        mNetworkChangedReceiver.onReceive(mContext, null);
        mNetworkChangedReceiver.onReceive(mContext, new Intent());
        mNetworkChangedReceiver.onReceive(mContext, new Intent("action"));
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Logger.d(TAG, "tearDown() entry");
        RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER,
                Boolean.toString(mFileTransferCapability));
    }

    /**
     * Test whether the file transfer capability is changed successfully.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase1_ExchangeMyFtCapability() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase1_ExchangeMyFtCapability() entry");
        Method methodExgMyFtCapability = Utils.getPrivateMethod(NetworkChangedReceiver.class,
                EXCHANGE_FT_CAPABILITY, boolean.class);
        methodExgMyFtCapability.invoke(mNetworkChangedReceiver, false);
        boolean result = RcsSettings.getInstance().isFileTransferSupported();
        assertFalse(result);
        methodExgMyFtCapability.invoke(mNetworkChangedReceiver, true);
        result = RcsSettings.getInstance().isFileTransferSupported();
        assertTrue(result);
    }

    /**
     * Test case: handleFtCapabilityChanged.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase02_handleFtCapabilityChanged() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        RcsSettings.createInstance(mContext);
        RcsSettings.getInstance().setSupportFileTransfer(true);
        Method method = Utils.getPrivateMethod(NetworkChangedReceiver.class,
                "handleFtCapabilityChanged", NetworkInfo.class);
        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "gprs", "gprs"));
        long begin = System.currentTimeMillis();
        boolean success = false;
        while (System.currentTimeMillis() - begin < 5000) {
            if (!RcsSettings.getInstance().isFileTransferSupported()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "lte", "lte"));
        begin = System.currentTimeMillis();
        success = false;
        while (System.currentTimeMillis() - begin < 5000) {
            if (RcsSettings.getInstance().isFileTransferSupported()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "", ""));

        RcsSettings.getInstance().setSupportFileTransfer(false);
        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_WIFI,
                ConnectivityManager.TYPE_MOBILE, "lte", "lte"));
        begin = System.currentTimeMillis();
        success = false;
        while (System.currentTimeMillis() - begin < 5000) {
            if (RcsSettings.getInstance().isFileTransferSupported()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_BLUETOOTH,
                ConnectivityManager.TYPE_MOBILE, "", ""));
    }

    /**
     * Test case: handleRoamingNotification.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_handleRoamingNotification() throws Exception {
        RcsSettings.createInstance(mContext);
        RcsSettings.getInstance().setSupportFileTransfer(true);
        Method method = Utils.getPrivateMethod(NetworkChangedReceiver.class,
                "handleRoamingNotification", NetworkInfo.class);
        method.invoke(mNetworkChangedReceiver, (NetworkInfo) null);

        method.invoke(mNetworkChangedReceiver, new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "", ""));

        Field field = Utils.getPrivateField(NetworkChangedReceiver.class,
                "sDeviceLastRoamingStatus");
        field.set(null, false);
        NetworkInfo info = new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "", "");
        Method methodRoaming = Utils.getPrivateMethod(NetworkInfo.class, "setRoaming",
                boolean.class);
        methodRoaming.invoke(info, true);
        RcsSettings.getInstance().setRoamingAuthorizationState(true);
        ActivityMonitor am = new ActivityMonitor(SelectContactsActivity.class.getName(), null,
                false);
        getInstrumentation().addMonitor(am);
        getInstrumentation().waitForIdleSync();
        method.invoke(mNetworkChangedReceiver, info);
        Activity activity = getInstrumentation().waitForMonitorWithTimeout(am, 5000);
        getInstrumentation().removeMonitor(am);
        sendKeys(KeyEvent.KEYCODE_BACK);
        assertTrue(field.getBoolean(null));

        RcsSettings.getInstance().setRoamingAuthorizationState(false);
        getInstrumentation().addMonitor(am);
        getInstrumentation().waitForIdleSync();
        method.invoke(mNetworkChangedReceiver, info);
        activity = getInstrumentation().waitForMonitorWithTimeout(am, 5000);
        getInstrumentation().removeMonitor(am);
        sendKeys(KeyEvent.KEYCODE_BACK);
        assertTrue(field.getBoolean(null));

        field.set(null, true);
        methodRoaming.invoke(info, false);
        getInstrumentation().addMonitor(am);
        getInstrumentation().waitForIdleSync();
        method.invoke(mNetworkChangedReceiver, info);
        getInstrumentation().waitForIdleSync();
        activity = getInstrumentation().waitForMonitorWithTimeout(am, 5000);
        getInstrumentation().removeMonitor(am);
        sendKeys(KeyEvent.KEYCODE_BACK);
        assertFalse(field.getBoolean(null));
    }

    /**
     * Test case: ensureApnRouteToHost.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase04_ensureApnRouteToHost() throws Exception {
        RcsSettings.createInstance(mContext);
        RcsSettings.getInstance().setSupportFileTransfer(true);
        Method method = Utils.getPrivateMethod(NetworkChangedReceiver.class,
                "ensureApnRouteToHost", NetworkInfo.class);
        method.invoke(mNetworkChangedReceiver, (NetworkInfo) null);

        Field field = Utils.getPrivateField(NetworkInfo.class, "mState");
        NetworkInfo info = new NetworkInfo(ConnectivityManager.TYPE_MOBILE,
                ConnectivityManager.TYPE_MOBILE, "", "");
        field.set(info, NetworkInfo.State.CONNECTING);
        method.invoke(mNetworkChangedReceiver, info);
        field.set(info, NetworkInfo.State.CONNECTED);
        method.invoke(mNetworkChangedReceiver, info);
    }
}
