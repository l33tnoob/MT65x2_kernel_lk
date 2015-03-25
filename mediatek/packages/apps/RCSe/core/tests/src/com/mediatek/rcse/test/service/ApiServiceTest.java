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
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.ICapabilityRemoteListener;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.StartService;
import com.orangelabs.rcs.service.api.client.ClientApiIntents;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApi;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApiIntents;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class is used to test ApiService
 */
public class ApiServiceTest extends InstrumentationTestCase {
    private final static String TAG = "ApiService";
    private final static int SLEEP_TIME = 200;
    private Context mContext = null;
    private ApiService mApiService = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        mApiService = new ApiService();
        ApiManager.initialize(mContext);
        mContext.startService(new Intent(
                "com.mediatek.rcse.service.IRegistrationStatus"));
        Field feFieldmBaseContext = Utils.getPrivateField(mApiService
                .getClass().getSuperclass().getSuperclass(), "mBase");
        feFieldmBaseContext.set(mApiService, mContext);
        mApiService.onCreate();
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
    public void testCase01_SdcardReceiver() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase01_SdcardReceiver() entry");
        Class<?>[] clazzes = mApiService.getClass().getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "iner class:" + clazz.getSimpleName());
            if ("SdcardReceiver".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor();
                ctr.setAccessible(true);
                break;
            }
        }
        Object sdcardReceiver = ctr.newInstance();
        Method methodOnreceive = Utils.getPrivateMethod(
                sdcardReceiver.getClass(), "onReceive", Context.class,
                Intent.class);
        Intent intent = new Intent(Intent.ACTION_MEDIA_MOUNTED);
        methodOnreceive.invoke(sdcardReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent(Intent.ACTION_MEDIA_UNMOUNTED);
        methodOnreceive.invoke(sdcardReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
    }

    /**
     * Test ApiReceiver
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase02_ApiReceiver() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase02_ApiReceiver() entry");
        Intent serviceIntent = new Intent();
        assertNull(mApiService.onBind(serviceIntent));
        Class<?>[] clazzes = mApiService.getClass().getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "iner class:" + clazz.getSimpleName());
            if ("ApiReceiver".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(mApiService.getClass());
                ctr.setAccessible(true);
                break;
            }
        }
        Object apiReceiver = ctr.newInstance(mApiService);
        Method methodOnreceive = Utils.getPrivateMethod(apiReceiver.getClass(),
                "onReceive", Context.class, Intent.class);
        Intent intent = new Intent(CapabilityApiIntents.CONTACT_CAPABILITIES);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent(ClientApiIntents.SERVICE_REGISTRATION);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent(MessagingApiIntents.CHAT_SESSION_REPLACED);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);

        intent = new Intent("ACTION_DEVICE_STORAGE_LOW");
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent("ACTION_DEVICE_STORAGE_OK");
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent(StartService.CONFIGURATION_STATUS);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
        intent = new Intent("com.test.test");
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);

        // Add extra
        intent = new Intent(CapabilityApiIntents.CONTACT_CAPABILITIES);
        String contact = "+34200000255";
        intent.putExtra("contact", contact);
        Capabilities capabilities = new Capabilities();
        intent.putExtra("capabilities", capabilities);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);

        // Add extra
        intent = new Intent(ClientApiIntents.SERVICE_REGISTRATION);
        intent.putExtra("status", true);
        methodOnreceive.invoke(apiReceiver, mContext, intent);
        Thread.sleep(SLEEP_TIME);
    }

    /**
     * Test RegistrationStatusStub
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     */
    public void testCase03_RegistrationStatusStub()
            throws NoSuchMethodException, IllegalArgumentException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException {
        Logger.d(TAG, "testCase03_RegistrationStatusStub() entry");
        Intent serviceIntent = new Intent();
        assertNull(mApiService.onBind(serviceIntent));
        serviceIntent = new Intent(IRegistrationStatus.class.getName());
        mApiService.onBind(serviceIntent);

        Field fieldmRegistrationStatusStub = Utils.getPrivateField(
                mApiService.getClass(), "mRegistrationStatusStub");
        Object registrationStatusStub = fieldmRegistrationStatusStub
                .get(mApiService);

        Field fieldmListeners = Utils.getPrivateField(
                registrationStatusStub.getClass(), "mListeners");
        Object listeners = fieldmListeners.get(registrationStatusStub);
        if (listeners == null) {
            Logger.d(TAG, "mListener is null, set it ");
            fieldmListeners
                    .set(registrationStatusStub,
                            new RemoteCallbackList<IRegistrationStatusRemoteListener>());
        }
        Method methodaddRegistrationStatusListener = Utils.getPrivateMethod(
                registrationStatusStub.getClass(),
                "addRegistrationStatusListener",
                IRegistrationStatusRemoteListener.class);
        RegistrationStatusRemoteListener listener = new RegistrationStatusRemoteListener();
        methodaddRegistrationStatusListener.invoke(registrationStatusStub,
                listener);

        Method methodnotifyRegistrationStatus = Utils.getPrivateMethod(
                registrationStatusStub.getClass(), "notifyRegistrationStatus",
                boolean.class);
        methodnotifyRegistrationStatus.invoke(registrationStatusStub, true);
        assertEquals(true, listener.mIsRegistered);
    }

    private class RegistrationStatusRemoteListener extends
            IRegistrationStatusRemoteListener.Stub {
        private boolean mIsRegistered = false;

        @Override
        public void onStatusChanged(boolean status) throws RemoteException {
            mIsRegistered = status;
        }
    }

    /**
     * Test CapabilitiesStu
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     */
    public void testCase04_CapabilitiesStub() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Logger.d(TAG, "testCase04_CapabilitiesStub() entry");

        Field fieldmCapabilitiesStub = Utils.getPrivateField(
                mApiService.getClass(), "mCapabilitiesStub");
        Object capabilitiesStub = fieldmCapabilitiesStub.get(mApiService);
        Field fieldmApi = Utils.getPrivateField(capabilitiesStub.getClass(),
                "mApi");
        CapabilityApi capabilityApi = new CapabilityApi(mContext);
        if (fieldmApi.get(capabilitiesStub) == null) {
            Logger.d(TAG, "mApi is null, set it");
            fieldmApi.set(capabilitiesStub, capabilityApi);
        }

        Class<?>[] clazzes = capabilitiesStub.getClass().getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "inner class : " + clazz.getSimpleName());
            if ("CapabilityApiListener".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor(capabilitiesStub.getClass(),
                        CapabilityApi.class);
                break;
            }
        }
        Object capabilityApiListener = ctr.newInstance(capabilitiesStub,
                capabilityApi);
        Method methodhandleApiConnected = Utils.getPrivateMethod(
                capabilityApiListener.getClass(), "handleApiConnected");
        methodhandleApiConnected.invoke(capabilityApiListener);

        Method methodaddCapabilityListener = Utils.getPrivateMethod(
                capabilitiesStub.getClass(), "addCapabilityListener",
                ICapabilityRemoteListener.class);
        CapabilityListener listener = new CapabilityListener();
        methodaddCapabilityListener.invoke(capabilitiesStub, listener);

        Method methodnotifyCapabilities = Utils.getPrivateMethod(
                capabilitiesStub.getClass(), "notifyCapabilities",
                String.class, Capabilities.class);
        String contact = "+34200000255";
        Capabilities capabilities = new Capabilities();
        methodnotifyCapabilities
                .invoke(capabilitiesStub, contact, capabilities);
        assertEquals(true, listener.mIsCapabilityChanged);

        Method methodgetContactCapabilities = Utils.getPrivateMethod(
                capabilitiesStub.getClass(), "getContactCapabilities",
                String.class);
        methodgetContactCapabilities.invoke(capabilitiesStub, contact);

        Method methodonStatusChanged = Utils.getPrivateMethod(
                capabilitiesStub.getClass(), "onStatusChanged", boolean.class);
        methodonStatusChanged.invoke(capabilitiesStub, true);
    }

    private class CapabilityListener extends ICapabilityRemoteListener.Stub {
        private boolean mIsCapabilityChanged = false;

        @Override
        public void onCapabilityChanged(String contact,
                Capabilities capabilities) throws RemoteException {
            mIsCapabilityChanged = true;
        }
    }

}
