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

package com.mediatek.rcse.test.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestCase;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.plugin.phone.ImageSharingPlugin;
import com.mediatek.rcse.plugin.phone.SharingPlugin;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Test case for SharingPlugin.
 */
public class SharingPluginTest extends ActivityInstrumentationTestCase2<PluginProxyActivity>  {
    private final static String TAG = "SharingPluginTest";
    private final static String MOCK_NMUBER = "1234567890";
    private final static String MOCK_VF_ACCOUNT = "+34200000251";
    private final static String GET_VF_ACCOUNT = "getVodafoneAccount";
    private final static String GET_NORMAL_NUMBER = "getNormalNumber";
    private final static String RICH_CALL_API = "mRichCallApi";
    private static final int DISCONNECTED = 0;
    private static final int CONNECTING = 1;
    private static final int CONNECTED = 2;
    private Context mContext = null;
    private static final String SERVICE_STATUS = "com.orangelabs.rcs.SERVICE_STATUS";
    private static  final String SERVICE_REGISTRATION = "com.orangelabs.rcs.SERVICE_REGISTRATION";
    private static final String CORE_SERVICE_STATUS = "status";
    private static final String UNKNOW_ACTION = "unknown action";
    private boolean mIsConnectApi = false;
    private static final int SLEEP_TIME = 500;
    
    public SharingPluginTest() {
        super(PluginProxyActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        Logger.d(TAG, "setUp() entry");
    }

    /**
     * Test to get VF account
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InvocationTargetException
     */
    public void testCase1_GetVfAccount() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase1_GetVfAccount() entry");
        MockRichCallApi richCallApi = new MockRichCallApi(mContext);
        SharingPlugin sharingPlugin = new SharingPlugin(mContext);
        Field filed = Utils.getPrivateField(SharingPlugin.class, RICH_CALL_API);
        filed.set(sharingPlugin, richCallApi);
        Method method = Utils.getPrivateMethod(SharingPlugin.class, GET_VF_ACCOUNT, String.class);
        String account = (String) method.invoke(sharingPlugin, MOCK_NMUBER);
        assertEquals(MOCK_VF_ACCOUNT, account);
    }
    
    /**
     * This is to test isImageShareSupported() function
     */
    public void testCase2_IsImageShareSupported() throws Throwable {
    	Logger.d(TAG, "testCase2_IsImageShareSupported() entry");
    	// init PluginApiManager
        PluginApiManager.initialize(mContext);
        PluginApiManager apiManager = PluginApiManager.getInstance();
        Field fieldRegistrationApi = Utils.getPrivateField(PluginApiManager.class, "mRegistrationApi");
        Field fieldCapabilitiesApi = Utils.getPrivateField(PluginApiManager.class, "mCapabilitiesApi");
        fieldRegistrationApi.set(apiManager, new MockRegistrationApi(mContext));
        fieldCapabilitiesApi.set(apiManager, new MockCapabilityApi(mContext));
        
        assertNotNull(apiManager.getRegistrationStatus());
        assertNotNull(fieldRegistrationApi.get(apiManager));
        assertNotNull(apiManager.getRegistrationApi());
    	
    	MockRichCallApi richCallApi = new MockRichCallApi(mContext);
        SharingPlugin sharingPlugin = new SharingPlugin(mContext);
        Field filed = Utils.getPrivateField(SharingPlugin.class, RICH_CALL_API);
        filed.set(sharingPlugin, richCallApi);
        Field mRichCallStatusField = Utils.getPrivateField(SharingPlugin.class, "mRichCallStatus");
        
        mRichCallStatusField.set(sharingPlugin, CONNECTING);
        Method isImageShareSupported = Utils.getPrivateMethod(SharingPlugin.class, "isImageShareSupported", String.class);
        Object result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));
        
        apiManager.setRegistrationStatus(false);
        mRichCallStatusField.set(sharingPlugin, CONNECTED);
        result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));

        apiManager.setRegistrationStatus(true);
        result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));
    }
    
    /**
     * This is to test isVideoShareSupported() function
     */
    public void testCase3_IsVideoShareSupported() throws Throwable {
    	Logger.d(TAG, "testCase3_IsVideoShareSupported() entry");
    	// init PluginApiManager
        PluginApiManager.initialize(mContext);
        PluginApiManager apiManager = PluginApiManager.getInstance();
        Field fieldRegistrationApi = Utils.getPrivateField(PluginApiManager.class, "mRegistrationApi");
        Field fieldCapabilitiesApi = Utils.getPrivateField(PluginApiManager.class, "mCapabilitiesApi");
        fieldRegistrationApi.set(apiManager, new MockRegistrationApi(mContext));
        fieldCapabilitiesApi.set(apiManager, new MockCapabilityApi(mContext));
        
        assertNotNull(apiManager.getRegistrationStatus());
        assertNotNull(fieldRegistrationApi.get(apiManager));
        assertNotNull(apiManager.getRegistrationApi());
    	
    	MockRichCallApi richCallApi = new MockRichCallApi(mContext);
        SharingPlugin sharingPlugin = new SharingPlugin(mContext);
        Field filed = Utils.getPrivateField(SharingPlugin.class, RICH_CALL_API);
        filed.set(sharingPlugin, richCallApi);
        Field mRichCallStatusField = Utils.getPrivateField(SharingPlugin.class, "mRichCallStatus");
        
        mRichCallStatusField.set(sharingPlugin, CONNECTING);
        Method isImageShareSupported = Utils.getPrivateMethod(SharingPlugin.class, "isVideoShareSupported", String.class);
        Object result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));
        
        apiManager.setRegistrationStatus(false);
        mRichCallStatusField.set(sharingPlugin, CONNECTED);
        result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));

        apiManager.setRegistrationStatus(true);
        result = isImageShareSupported.invoke(sharingPlugin, MOCK_NMUBER);
        assertFalse(Boolean.valueOf(result.toString()));
    }

    /**
     * This is to test RcseCoreServiceStatusReceiver class
     */
    public void testCase4_RcseCoreServiceStatusReceiver() throws Throwable {
    	Logger.d(TAG, "testCase4_RcseCoreServiceStatusReceiver() entry");
    	MockRichCallApi richCallApi = new MockRichCallApi(mContext);
        SharingPlugin sharingPlugin = new SharingPlugin(mContext);
        Field filed = Utils.getPrivateField(SharingPlugin.class, RICH_CALL_API);
        filed.set(sharingPlugin, richCallApi);
        
        Field mRcseCoreServiceStatusReceiverField = Utils.getPrivateField(SharingPlugin.class, "mRcseCoreServiceStatusReceiver");
        BroadcastReceiver mRcseCoreServiceStatusReceiver = (BroadcastReceiver) mRcseCoreServiceStatusReceiverField.get(sharingPlugin);
        assertNotNull(mRcseCoreServiceStatusReceiver);
        // other condition
        mRcseCoreServiceStatusReceiver.onReceive(mContext, null);
        mRcseCoreServiceStatusReceiver.onReceive(mContext, new Intent(UNKNOW_ACTION));
        // normal condition
        Intent serviceIt = new Intent();
        serviceIt.setAction(SERVICE_STATUS);
        mRcseCoreServiceStatusReceiver.onReceive(mContext, serviceIt);
        Field mRichCallStatusField = Utils.getPrivateField(SharingPlugin.class, "mRichCallStatus");
        mRichCallStatusField.set(sharingPlugin, CONNECTING);
        Intent regIt = new Intent();
        regIt.setAction(SERVICE_REGISTRATION);
        regIt.putExtra(CORE_SERVICE_STATUS, true);
        mIsConnectApi = false;
        mRcseCoreServiceStatusReceiver.onReceive(mContext, regIt);
        assertTrue(mIsConnectApi);
        mRichCallStatusField.set(sharingPlugin, CONNECTED);
        mIsConnectApi = false;
        mRcseCoreServiceStatusReceiver.onReceive(mContext, regIt);
        assertFalse(mIsConnectApi);
    }
    
    /**
     * This is to test videoShareNotSupported() function
     */
    public void testCase5_videoShareNotSupported() throws Throwable {
    	Logger.d(TAG, "testCase5_videoShareNotSupported() entry");
    	MockRichCallApi richCallApi = new MockRichCallApi(mContext);
        SharingPlugin sharingPlugin = new SharingPlugin(mContext);
        Field filed = Utils.getPrivateField(SharingPlugin.class, RICH_CALL_API);
        filed.set(sharingPlugin, richCallApi);
        Method videoShareNotSupported = Utils.getPrivateMethod(SharingPlugin.class, "videoShareNotSupported");
        videoShareNotSupported.invoke(sharingPlugin);
        Thread.sleep(SLEEP_TIME);
    }
    
    /**
     * Mock RichCallApi with a mocked VF account and a mocked number.
     */
    private class MockRichCallApi extends RichCallApi {

        public MockRichCallApi(Context ctx) {
            super(ctx);
        }

        @Override
        public String getVfAccountViaNumber(String number) throws ClientApiException {
            if (number.equals(MOCK_NMUBER)) {
                return MOCK_VF_ACCOUNT;
            }
            return null;
        }

        @Override
        public String getNumberViaVfAccount(String account) throws ClientApiException {
            if (account.equals(MOCK_VF_ACCOUNT)) {
                return MOCK_NMUBER;
            }
            return null;
        }
        
        @Override
        public void connectApi() {
        	mIsConnectApi = true;
        	super.connectApi();
        }
    }
    
 // Mock capabilityApi to pass capability check
    private class MockCapabilityApi extends CapabilityApi {

        public MockCapabilityApi(Context context) {
            super(context);
        }

        @Override
        public void connect() {
            // Mock api do not need to do real connect
        }

        @Override
        public void disconnect() {
            // Mock api do not need to do real connect
        }

        @Override
        public Capabilities getContactCapabilities(String contact) {
            Capabilities capabilities = new Capabilities();
            capabilities.setFileTransferSupport(true);
            capabilities.setRcseContact(true);
            capabilities.setImSessionSupport(true);
            return capabilities;
        }

        @Override
        public Capabilities getMyCapabilities() {
            Capabilities capabilities = new Capabilities();
            capabilities.setCsVideoSupport(true);
            capabilities.setFileTransferSupport(true);
            capabilities.setRcseContact(true);
            capabilities.setImSessionSupport(true);
            return capabilities;
        }
        
    }
    
    private class MockRegistrationApi extends RegistrationApi {

        public MockRegistrationApi(Context context) {
            super(context);
        }

        @Override
        public void connect() {
            // Mock api do not need to do real connect
        }

        @Override
        public void disconnect() {
            // Mock api do not need to do real connect
        }

        @Override
        public boolean isRegistered() {
        	Logger.d(TAG, "isRegistered()");
            return true;
        }
    }
}
