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

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.CapabilityApi.ICapabilityListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ICapabilities;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ClientApiUtils;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.lang.reflect.Field;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Test case for CapabilityApi
 */
public class CapabilityApiTest extends AndroidTestCase {
    private static final String TAG = "CapabilityApiTest";
    private static final String CONTACTS = "+34200000100";
    private static final int TIME_OUT_API = 5000;

    private CapabilityApi mCapabilityApi;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCapabilityApi = new CapabilityApi(getContext());
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown()");
        super.tearDown();
    }

    /**
     * Test CapabilityApi
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InterruptedException
     */
    public void testCase01_CapabilityApi() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase01_CapabilityApi");
        assertNotNull(mCapabilityApi);
        CapabilityApi api = null;
        RuntimeException runtimeException = null;
        try {
            api = new CapabilityApi(null);
        } catch (RuntimeException e) {
            runtimeException = e;
        } finally {
            assertNotNull(runtimeException);
        }
    }
    
    /**
     * Test registerCapabilityListener
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InterruptedException
     */
    public void testCase02_registerCapabilityListener() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase02_registerCapabilityListener");
        waitRcsCoreServiceStarted();
        assertNotNull(mCapabilityApi);
        TestCapabilityListener listener = new TestCapabilityListener();
        mCapabilityApi.registerCapabilityListener(listener);

        CopyOnWriteArrayList<ICapabilityListener> listenerList = (CopyOnWriteArrayList<ICapabilityListener>) getListes();
        assertNotNull(listenerList);
        assertTrue(listenerList.contains(listener));

        mCapabilityApi.unregisterCapabilityListener(listener);
        assertFalse(listenerList.contains(listener));

    }

    /**
     * Test onCapabilityChanged
     * 
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws RemoteException
     */
    public void testCase03_onCapabilityChanged() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException, RemoteException {
        Logger.d(TAG, "testCase03_onCapabilityChanged");
        assertNotNull(mCapabilityApi);
        TestCapabilityListener listener = new TestCapabilityListener();
        mCapabilityApi.registerCapabilityListener(listener);

        CopyOnWriteArrayList<ICapabilityListener> listenerList = (CopyOnWriteArrayList<ICapabilityListener>) getListes();
        assertNotNull(listenerList);
        assertTrue(listenerList.contains(listener));
        Capabilities capabilities = new Capabilities();
        capabilities.setVideoSharingSupport(true);
        mCapabilityApi.onCapabilityChanged(CONTACTS, capabilities);
        assertTrue(listener.mIsCapabilityChangedCalled);
        assertEquals(CONTACTS, listener.mContact);
        assertTrue(listener.mCapabilities.isVideoSharingSupported());

        mCapabilityApi.unregisterCapabilityListener(listener);
        assertFalse(listenerList.contains(listener));
    }

    /**
     * Test getMyCapabilities
     * 
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     */
    public void testCase04_getMyCapabilities() throws IllegalArgumentException,
            InterruptedException, NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase04_getMyCapabilities");
        assertNotNull(mCapabilityApi);
        if (isCapabilityApiconnected()) {
            Logger.d(TAG, "Capability is connected , then disconnect");
            mCapabilityApi.disconnect();
            waitForCapabilityApiDisconnected();
        }
        mCapabilityApi.connect();
        waitForCapabilityApiConnected();
        Capabilities myCapabilities = mCapabilityApi.getMyCapabilities();
        Capabilities myCapabilitiesFromDb = RcsSettings.getInstance().getMyCapabilities();
        Logger.d(TAG, "myCapabilities = " + myCapabilities + ", myCapabilitiesFromDb = "
                + myCapabilitiesFromDb);
        mCapabilityApi.disconnect();
    }

    /**
     * Test getContactCapabilities
     * 
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     */
    public void testCase05_getContactCapabilities() throws IllegalArgumentException,
            InterruptedException, NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase05_getContactCapabilities");
        assertNotNull(mCapabilityApi);
        if (isCapabilityApiconnected()) {
            Logger.d(TAG, "Capability is connected , then disconnect");
            mCapabilityApi.disconnect();
            waitForCapabilityApiDisconnected();
        }
        mCapabilityApi.connect();
        waitForCapabilityApiConnected();

        // Obvious it is a non RCS-e contact
        Capabilities capabilities = mCapabilityApi.getContactCapabilities("+86028");
        assertTrue(capabilities == null || capabilities.isSupportedRcseContact() == false);
        capabilities = mCapabilityApi.getContactCapabilities(CapabilityApi.ME);
        assertTrue(capabilities == null || capabilities.isSupportedRcseContact() == true);
        mCapabilityApi.disconnect();
    }
    
    /**
    * Test onServiceDisconnected()
    * 
    * @throws IllegalArgumentException
    * @throws InterruptedException
    * @throws NoSuchFieldException
    * @throws IllegalAccessException
    */
   public void testCase06_onServiceDisconnected() throws IllegalArgumentException,
           InterruptedException, NoSuchFieldException, IllegalAccessException {
       Logger.d(TAG, "testCase06_onServiceDisconnected");
       Field capabilityConnectionField = Utils.getPrivateField(mCapabilityApi.getClass(), "mCapabilityConnection");
       ServiceConnection conn = (ServiceConnection)capabilityConnectionField.get(mCapabilityApi);
       conn.onServiceDisconnected(null);
       Field fieldICapabilities = Utils.getPrivateField(mCapabilityApi.getClass(), "mICapabilities");
       ICapabilities icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);
       assertNull(icpabilities);
    }
    
    private Object getListes() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Field fieldListeners = Utils.getPrivateField(mCapabilityApi.getClass(), "mListeners");
        return fieldListeners.get(mCapabilityApi);
    }

    private class TestCapabilityListener implements ICapabilityListener {
        private boolean mIsCapabilityChangedCalled = false;
        private String mContact;
        Capabilities mCapabilities;

        @Override
        public void onCapabilityChanged(String contact, Capabilities capabilities) {
            mIsCapabilityChangedCalled = true;
            mContact = contact;
            mCapabilities = capabilities;
        }
    }

    private void waitForCapabilityApiConnected() throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "waitForCapabilityApiConnected()");
        Field fieldICapabilities = Utils.getPrivateField(mCapabilityApi.getClass(),
                "mICapabilities");
        ICapabilities icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);
        long beginTime = System.currentTimeMillis();
        while (icpabilities == null) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT_API) {
                fail();
            }
            Thread.sleep(200);
            icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);
        }
        Logger.d(TAG, "waitForCapabilityApiConnected() eixt. icpabilities = " + icpabilities);
    }

    private void waitForCapabilityApiDisconnected() throws InterruptedException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "waitForCapabilityApiDisconnected()");
        Field fieldICapabilities = Utils.getPrivateField(mCapabilityApi.getClass(),
                "mICapabilities");
        ICapabilities icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);
        long beginTime = System.currentTimeMillis();
        while (icpabilities != null) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT_API) {
                fail();
            }
            Thread.sleep(200);
            icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);
        }
        Logger.d(TAG, "waitForCapabilityApiDisconnected() eixt. icpabilities = " + icpabilities);
    }

    private boolean isCapabilityApiconnected() throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "isCapabilityApiconnected()");
        Field fieldICapabilities = Utils.getPrivateField(mCapabilityApi.getClass(),
                "mICapabilities");
        ICapabilities icpabilities = (ICapabilities) fieldICapabilities.get(mCapabilityApi);

        Logger.d(TAG, "isCapabilityApiconnected() eixt. icpabilities = " + icpabilities);
        return icpabilities != null;
    }

    private void waitRcsCoreServiceStarted() throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "waitRcsCoreServiceStarted()");
        long beginTime = System.currentTimeMillis();
        while (!ClientApiUtils.isServiceStarted(mContext)) {
            if ((System.currentTimeMillis() - beginTime) > TIME_OUT_API) {
                break;
            }
            Thread.sleep(200);
        }
        Logger.d(
                TAG,
                "waitRcsCoreServiceStarted() eixt. isServiceStarted:"
                        + ClientApiUtils.isServiceStarted(mContext));
    }
}
