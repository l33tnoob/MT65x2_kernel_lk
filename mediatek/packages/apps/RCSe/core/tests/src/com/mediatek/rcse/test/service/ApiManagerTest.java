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

import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.core.ims.network.NetworkConnectivityApi;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class is used to test ApiManager
 */
public class ApiManagerTest extends InstrumentationTestCase {
    private final static String TAG = "ApiManagerTest";
    private final static int SLEEP_TIME = 200;
    private Context mContext = null;
    Field mFieldsInstance = null;
    ApiManager mApiManager = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        ApiManager.initialize(mContext);
        mFieldsInstance = Utils.getPrivateField(ApiManager.class, "sInstance");
        mApiManager = (ApiManager) mFieldsInstance.get(null);
    }

    /**
     * Test SdcardReceiver
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase01_getInstance() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase01_getInstance() entry");
        mFieldsInstance.set(null, null);
        assertFalse(ApiManager.initialize(null));
        assertTrue(ApiManager.initialize(mContext));
        ApiManager.getInstance().getMaxSizeforFileThransfer();
        ApiManager.getInstance().getWarningSizeforFileThransfer();
        ApiManager.getInstance().getEventsLogApi();
        ApiManager.getInstance().getMessagingApi();
        ApiManager.getInstance().getNetworkConnectivityApi();
        ApiManager.getInstance().getRegistrationApi();
        ApiManager.getInstance().getRcseComponentController();
        ApiManager.getInstance().getCapabilityApi();
        assertNotNull(ApiManager.getInstance().getContext());
    }

    /**
     * Test ManagedRegistrationApi
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase02_ManagedRegistrationApi()
            throws NoSuchMethodException, IllegalArgumentException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        Logger.d(TAG, "testCase02_ManagedRegistrationApi() entry");
        Class<?>[] clazzes = ApiManager.getInstance().getClass()
                .getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "Inner class: " + clazz.getSimpleName());
            if ("ManagedRegistrationApi".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(ApiManager.getInstance()
                        .getClass(), Context.class);
                ctr.setAccessible(true);
            }
        }
        Object managedRegistrationApi = ctr.newInstance(
                ApiManager.getInstance(), mContext);

        clazzes = managedRegistrationApi.getClass().getDeclaredClasses();
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "Inner class: " + clazz.getSimpleName());
            if ("RegistrationStatusListener".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(managedRegistrationApi
                        .getClass());
                ctr.setAccessible(true);
            }
        }
        Object registrationStatusListener = ctr
                .newInstance(managedRegistrationApi);
        Method methodonStatusChanged = Utils.getPrivateMethod(
                registrationStatusListener.getClass(), "onStatusChanged",
                boolean.class);
        resetMessagingApi();
        resetNetworkConnectivityApi();
        methodonStatusChanged.invoke(registrationStatusListener, true);
        /*try{
            Method methodhandleConnectedd = Utils.getPrivateMethod(
                    managedRegistrationApi.getClass(), "handleConnected");
            methodhandleConnectedd.invoke(managedRegistrationApi);
        }catch(RuntimeException e){
            Logger.e(TAG, "RuntimeException:" + e.getMessage());
        }*/
        
        Method methodhandleDisconnected = Utils.getPrivateMethod(
                managedRegistrationApi.getClass(), "handleDisconnected");
        methodhandleDisconnected.invoke(managedRegistrationApi);
        Thread.sleep(SLEEP_TIME);
    }

    private void resetMessagingApi() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldmMessagingApi = Utils.getPrivateField(ApiManager
                .getInstance().getClass(), "mMessagingApi");
        fieldmMessagingApi.set(ApiManager.getInstance(), null);
    }

    private void resetNetworkConnectivityApi() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldmNetworkConnectivityApi = Utils.getPrivateField(ApiManager
                .getInstance().getClass(), "mNetworkConnectivityApi");
        fieldmNetworkConnectivityApi.set(ApiManager.getInstance(), null);
    }

    /**
     * Test RcseComponentController
     */
    public void testCase03_RcseComponentController() {
        Logger.d(TAG, "testCase03_RcseComponentController() entry");
        RcseComponentController rcseComponentController = ApiManager
                .getInstance().new RcseComponentController();
        boolean isIntegrationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        rcseComponentController.onConfigurationStatusChanged(true);
        rcseComponentController.onServiceActiveStatusChanged(true);

        // restore
        Logger.setIsIntegrationMode(isIntegrationMode);
        rcseComponentController.onConfigurationStatusChanged(false);
        rcseComponentController.onConfigurationStatusChanged(true);
        rcseComponentController.onServiceActiveStatusChanged(true);
        assertEquals(isIntegrationMode, Logger.getIsIntegrationMode());
    }

    /**
     * Test MessagingApiListener
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase04_MessagingApiListener() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase04_MessagingApiListener() entry");
        Class<?>[] clazzes = ApiManager.getInstance().getClass()
                .getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "Inner class: " + clazz.getSimpleName());
            if ("MessagingApiListener".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(ApiManager.getInstance()
                        .getClass(), MessagingApi.class);
                ctr.setAccessible(true);
            }
        }
        MessagingApi messagingApi = new MessagingApi(mContext);
        Object messagingApiListener = ctr.newInstance(ApiManager.getInstance(),
                messagingApi);
        Method methodhandleApiConnected = Utils.getPrivateMethod(
                messagingApiListener.getClass(), "handleApiConnected");
        methodhandleApiConnected.invoke(messagingApiListener);

        Method methodhandleMessageDeliveryStatus = Utils.getPrivateMethod(
                messagingApiListener.getClass(), "handleMessageDeliveryStatus",
                String.class, String.class, String.class, long.class);
        methodhandleMessageDeliveryStatus.invoke(messagingApiListener,
                "+34200000255", "123", "connected", System.currentTimeMillis());
        Thread.sleep(SLEEP_TIME);
        Method methodhandleDisconnected = Utils.getPrivateMethod(
                messagingApiListener.getClass(), "handleApiDisconnected");
        methodhandleDisconnected.invoke(messagingApiListener);
    }

    /**
     * Test ManagedCapabilityApi
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     */
    public void testCase05_ManagedCapabilityApi() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase05_ManagedCapabilityApi() entry");
        Class<?>[] clazzes = ApiManager.getInstance().getClass()
                .getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "Inner class: " + clazz.getSimpleName());
            if ("ManagedCapabilityApi".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(ApiManager.getInstance()
                        .getClass(), Context.class);
                ctr.setAccessible(true);
            }
        }
        Object managedCapabilityApi = ctr.newInstance(ApiManager.getInstance(),
                mContext);
        Method methodhandleConnectedd = Utils.getPrivateMethod(
                managedCapabilityApi.getClass(), "handleConnected");
        methodhandleConnectedd.invoke(managedCapabilityApi);
        Method methodhandleDisconnected = Utils.getPrivateMethod(
                managedCapabilityApi.getClass(), "handleDisconnected");
        methodhandleDisconnected.invoke(managedCapabilityApi);
    }

    /**
     * Test NetworkConnectivityApiListener
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     */
    public void testCase06_NetworkConnectivityApiListener()
            throws NoSuchMethodException, IllegalArgumentException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase06_NetworkConnectivityApiListener() entry");
        Class<?>[] clazzes = ApiManager.getInstance().getClass()
                .getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "Inner class: " + clazz.getSimpleName());
            if ("NetworkConnectivityApiListener".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(ApiManager.getInstance()
                        .getClass(), NetworkConnectivityApi.class);
                ctr.setAccessible(true);
            }
        }
        NetworkConnectivityApi networkConnectivityApi = new NetworkConnectivityApi(
                mContext);
        Object networkConnectivityApiListener = ctr.newInstance(
                ApiManager.getInstance(), networkConnectivityApi);
        Method methodhandleApiConnected = Utils
                .getPrivateMethod(networkConnectivityApiListener.getClass(),
                        "handleApiConnected");
        methodhandleApiConnected.invoke(networkConnectivityApiListener);
        Method methodhandleDisconnected = Utils.getPrivateMethod(
                networkConnectivityApiListener.getClass(),
                "handleApiDisconnected");
        methodhandleDisconnected.invoke(networkConnectivityApiListener);
    }

}
