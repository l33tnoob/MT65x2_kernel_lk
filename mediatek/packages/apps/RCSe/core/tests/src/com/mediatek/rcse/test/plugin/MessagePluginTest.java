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
import android.content.Intent;

import java.lang.reflect.Field;

import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.rcse.activities.widgets.PhotoLoaderManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.message.IpMessageActivitiesManager;
import com.mediatek.rcse.plugin.message.IpMessageChatManger;
import com.mediatek.rcse.plugin.message.IpMessageContactManager;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.IpMessagePluginExt;
import com.mediatek.rcse.plugin.message.IpMessageResourceMananger;
import com.mediatek.rcse.plugin.message.IpMessageServiceMananger;
import com.mediatek.rcse.plugin.message.IpNotificationsManager;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiIntents;
import com.orangelabs.rcs.service.LauncherUtils;

/**
 * This class is used to test MessagePlugin.
 */
public class MessagePluginTest extends InstrumentationTestCase {
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;
    private static final String KEY_STATUS = "status";

    public void testCase1_CreatePlugin() throws Throwable {
        PluginManager<IIpMessagePlugin> pluginManager = 
            PluginManager.create(getInstrumentation().getTargetContext(), IIpMessagePlugin.class.getName());
        int pluginCount = pluginManager.getPluginCount();
        IpMessagePluginExt ipMessagePluginExt = null;
        for (int i = 0; i < pluginCount ; i++) {
            IIpMessagePlugin plugin = pluginManager.getPlugin(i).createObject();
            if (IpMessagePluginExt.class.getName().equals(plugin.getClass().getName())) {
                ipMessagePluginExt = (IpMessagePluginExt) plugin;
            }
        }
        Context context = getInstrumentation().getTargetContext();
        assertNotNull(ipMessagePluginExt);
        ipMessagePluginExt.isActualPlugin();
        ipMessagePluginExt.onRcsCoreServiceStatusChanged(0);
        ipMessagePluginExt.onApiConnectedStatusChanged(false);
        assertFalse(Logger.getIsIntegrationMode());
        assertFalse(ipMessagePluginExt.getActivitiesManager(context) instanceof IpMessageActivitiesManager);
        assertFalse(ipMessagePluginExt.getChatManager(context) instanceof IpMessageChatManger);
        assertFalse(ipMessagePluginExt.getMessageManager(context) instanceof IpMessageManager);
        assertFalse(ipMessagePluginExt.getResourceManager(context) instanceof IpMessageResourceMananger);
        assertFalse(ipMessagePluginExt.getContactManager(context) instanceof IpMessageContactManager);
        assertFalse(ipMessagePluginExt.getNotificationsManager(context) instanceof IpNotificationsManager);
        assertFalse(ipMessagePluginExt.getServiceManager(context) instanceof IpMessageServiceMananger);

        Field fieldIsIntegrationMode = Utils.getPrivateField(Logger.class, "sIsIntegrationMode");
        fieldIsIntegrationMode.set(null, true);
        Field fieldIsSimAvailable = Utils.getPrivateField(IpMessagePluginExt.class, "mIsSimCardAvailable");
        fieldIsSimAvailable.set(ipMessagePluginExt, true);

        assertTrue(ipMessagePluginExt.getActivitiesManager(context) instanceof IpMessageActivitiesManager);
        assertTrue(ipMessagePluginExt.getChatManager(context) instanceof IpMessageChatManger);
        assertTrue(ipMessagePluginExt.getMessageManager(context) instanceof IpMessageManager);
        assertTrue(ipMessagePluginExt.getResourceManager(context) instanceof IpMessageResourceMananger);
        assertTrue(ipMessagePluginExt.getContactManager(context) instanceof IpMessageContactManager);
        assertTrue(ipMessagePluginExt.getNotificationsManager(context) instanceof IpNotificationsManager);
        assertTrue(ipMessagePluginExt.getServiceManager(context) instanceof IpMessageServiceMananger);

        fieldIsIntegrationMode.set(null, false);
    }
    
    /**
     * test IpMessageServiceMananger such as startIpservice, ServiceisReady.
     */
    public void testCase2_ServiceMananger() throws Throwable {
        Log.d("testCase2_ServiceMananger", "testCase2_ServiceMananger entry!");
        Context context = getInstrumentation().getTargetContext();
        PhotoLoaderManager.initialize(context);
        if (!ApiManager.initialize(context)) {
            Log.d("testCase2_ServiceMananger", "initialize failed!");
        }
        RcsSettings.createInstance(context);
        IpMessagePluginExt ipMessagePluginExt = new IpMessagePluginExt(context);
        waitForServiceConnected();
        IpMessageServiceMananger ipMessageServiceManager;
        ipMessageServiceManager = new IpMessageServiceMananger(context);
        LauncherUtils.stopRcsService(context);
        Intent intent = new Intent(ClientApiIntents.SERVICE_REGISTRATION);
        intent.putExtra(KEY_STATUS, true);
        context.sendBroadcast(intent);
        waitForRegistrationChange(true, ipMessageServiceManager);
    }

    private void waitForRegistrationChange(boolean setStatus, IpMessageServiceMananger manager)
            throws InterruptedException {
        boolean status = manager.serviceIsReady();
        long startTime = System.currentTimeMillis();
        while (status != setStatus) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            status = manager.serviceIsReady();
        }
    }
    
    private void waitForServiceConnected() throws InterruptedException {
        RegistrationApi registrantionApi = ApiManager.getInstance().getRegistrationApi();
        long startTime = System.currentTimeMillis();
        while (registrantionApi == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            registrantionApi = ApiManager.getInstance().getRegistrationApi();
        }
    }
}
