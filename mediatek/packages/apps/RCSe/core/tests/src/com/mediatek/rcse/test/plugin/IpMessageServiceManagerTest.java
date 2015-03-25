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

import android.content.Context;
import android.test.InstrumentationTestCase;
import android.util.LruCache;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageResourceMananger;
import com.mediatek.rcse.plugin.message.IpMessageServiceMananger;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.TreeSet;

/**
 * The class is used to test IpMessageActivityManager
 */
public class IpMessageServiceManagerTest extends InstrumentationTestCase {
    private static final String TAG = "ActivitesManagerTest";
    IpMessageServiceMananger mServiceMananger = null;
    private Context mContext = null;
    private static final String NAME = "dummy";
    private static final String SETTING_ISMS = "joyn Message";
    private static final String MOCK_NUMBER = "+34200000123";
    private static final long TIME_OUT = 2000;
    private static final long SLEEP_TIME = 200;
    private static final String PROVISION_INFO_VERSION_ZERO = "0";
    private static final long PROVISION_INFO_VILIDITY_ZERO = 0;
    private static final String PROVISION_INFO_VERSION_INVILID = "invilid";
    private static final long PROVISION_INFO_VILIDITY_INVILID = Long.MIN_VALUE;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mServiceMananger = new IpMessageServiceMananger(mContext);
    }

    /**
     * Test getIpMessageServiceId
     */
    public void testCase1_getIpMessageServiceId() {
        Logger.d(TAG, "testCase1_getIpMessageServiceId() entry");
        int actualId = mServiceMananger.getIpMessageServiceId();
        assertEquals(IpMessageConsts.IpMessageServiceId.ISMS_SERVICE, actualId);
        Logger.d(TAG, "testCase1_getIpMessageServiceId() exit");
    }

    /**
     * Test isRcseEnabled
     */
    public void testCase2_IsEnabled() throws IllegalAccessException, NoSuchMethodException,
            InvocationTargetException {
        Logger.d(TAG, "testCase2_IsEnabled() entry");
        PluginApiManager.initialize(mContext);
        RcsSettings.createInstance(mContext);
        boolean state = RcsSettings.getInstance().isServiceActivated();
        boolean testState = false;
        RcsSettings settings = RcsSettings.getInstance();
        settings.setServiceActivationState(true);
        testState = mServiceMananger.isEnabled();
        assertTrue(testState);
        settings.setServiceActivationState(false);
        testState = mServiceMananger.isEnabled(0);
        assertFalse(testState);
        settings.setServiceActivationState(state);
        Logger.d(TAG, "testCase2_IsEnabled() exit");
    }

    /**
     * Test isRcseActivated
     */
    public void testCase3_IsActivated() throws IllegalAccessException, NoSuchMethodException,
            InvocationTargetException, NoSuchFieldException {
        Logger.d(TAG, "testCase3_IsActivated() entry");
        PluginApiManager.initialize(mContext);
        RcsSettings.createInstance(mContext);
        boolean state = mServiceMananger.isActivated();
        assertTrue(state);
        state = mServiceMananger.isActivated(0);
        assertTrue(state);
        Method methodCheckIsRcseActivated =
                Utils.getPrivateMethod(IpMessageServiceMananger.class, "checkIsRcseActivated");
        RcsSettings settigns = RcsSettings.getInstance();
        long validity = settigns.getProvisionValidity();
        String version = settigns.getProvisioningVersion();
        settigns.setProvisioningVersion(PROVISION_INFO_VERSION_ZERO);
        settigns.setProvisionValidity(PROVISION_INFO_VILIDITY_ZERO);
        boolean testState = (Boolean) methodCheckIsRcseActivated.invoke(mServiceMananger);
        assertFalse(testState);
        settigns.setProvisioningVersion(PROVISION_INFO_VERSION_INVILID);
        settigns.setProvisionValidity(PROVISION_INFO_VILIDITY_INVILID);
        testState = (Boolean) methodCheckIsRcseActivated.invoke(mServiceMananger);
        assertTrue(testState);
        settigns.setProvisioningVersion(version);
        settigns.setProvisionValidity(validity);
        Logger.d(TAG, "testCase3_IsActivated() exit");
    }

    /**
     * Test whether a feature is supported.Ture if support,otherwise false.
     */
    public void testCase4_IsFeatureSupported() {
        Logger.d(TAG, "testCase4_IsFeatureSupported() entry");
        boolean isFeatureSupport = mServiceMananger.isFeatureSupported(FeatureId.CHAT_SETTINGS);
        assertTrue(isFeatureSupport);
        isFeatureSupport = mServiceMananger.isFeatureSupported(FeatureId.SHARE_CHAT_HISTORY);
        assertFalse(isFeatureSupport);
        Logger.d(TAG, "testCase4_IsFeatureSupported() exit");
    }

    /**
     * Test to add a contact to stranger.
     */
    public void testCase5_onCapabilitiesChanged() {
        Logger.d(TAG, "testCase5_onCapabilitiesChanged() entry");
        ContactsListManager.initialize(mContext);
        ContactsListManager instance = ContactsListManager.getInstance();
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isRcsContact = 1;
        mServiceMananger.onCapabilitiesChanged(MOCK_NUMBER, contactInformation);
        TreeSet<String> STRANGER_LIST = instance.STRANGER_LIST;
        assertTrue(instance.isStranger(MOCK_NUMBER));
        Logger.d(TAG, "testCase5_onCapabilitiesChanged() exit");
    }

    /**
     * Test get a string "joyn message" by the id
     * IpMessageConsts.ResourceId.STR_IPMESSAGE_SETTINGS
     */
    public void testCase6_getSingleString() {
        Logger.d(TAG, "testCase6_getSingleString() entry");
        IpMessageResourceMananger resourceManager = new IpMessageResourceMananger(mContext);
        String actualString = resourceManager.getSingleString(0);
        assertNull(actualString);
        actualString =
                resourceManager.getSingleString(IpMessageConsts.ResourceId.STR_IPMESSAGE_SETTINGS);
        assertEquals(SETTING_ISMS, actualString);
        Logger.d(TAG, "testCase6_getSingleString() exit");
    }
}
