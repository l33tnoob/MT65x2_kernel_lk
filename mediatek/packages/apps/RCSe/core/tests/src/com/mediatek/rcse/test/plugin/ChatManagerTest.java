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

import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.IMessenger;
import android.os.Message;
import android.test.ServiceTestCase;

import com.mediatek.mms.ipmessage.ChatManager;
import com.mediatek.mms.ipmessage.IpMessageConsts.ContactStatus;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.plugin.message.IpMessageChatManger;
import com.mediatek.rcse.plugin.message.IpMessagePluginExt;
import com.mediatek.rcse.plugin.message.PluginController;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * This test is used to test PluginController
 */
public class ChatManagerTest extends ServiceTestCase<ApiService> {
    private static final String TAG = "ChatManagerTest";
    private static final long SLEEP_TIME = 20;
    private static final long TIME_OUT = 500;
    private ChatManager mChatManager = null;
    private Field mInstanceField = null;
    private MockLocalController mMockController = null;
    
    private static final String MOCK_NUMBER = "+34000000000";

    public ChatManagerTest() {
        super(ApiService.class);
    }
    
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Intent intent = new Intent(IRemoteWindowBinder.class.getName());
        IBinder binder = bindService(intent);
        IRemoteWindowBinder remoteChatWindowBinder = IRemoteWindowBinder.Stub.asInterface(binder);
        assertNotNull(remoteChatWindowBinder);
        IMessenger messenger = IMessenger.Stub.asInterface(remoteChatWindowBinder.getController());
        assertNotNull(messenger);
        Method initializeMethod =
                Utils.getPrivateMethod(PluginController.class, "initialize", IMessenger.class);
        initializeMethod.invoke(null, messenger);
        mMockController = new MockLocalController();
        mInstanceField = Utils.getPrivateField(ControllerImpl.class, "sControllerImpl");
        mInstanceField.set(null, mMockController);
        if (!ApiManager.initialize(mContext)) {
            Logger.d(TAG, "initialize failed!");
            fail();
        }
        IpMessagePluginExt ipMessage = new IpMessagePluginExt(mContext);
        mChatManager = new IpMessageChatManger(mContext);
    }

    /**
     * Test to send chat mode in chat manager.
     * @throws Throwable
     */
    public void testCase1_SendChatModeTest() throws Throwable {
        Logger.d(TAG, "testCase1_SendChatModeTest() entry");
        mChatManager.sendChatMode(MOCK_NUMBER, ContactStatus.STOP_TYPING);
        waitForControllerMessage(mMockController, ChatController.EVENT_TEXT_CHANGED, MOCK_NUMBER,
                true);
        mChatManager.sendChatMode(MOCK_NUMBER, ContactStatus.TYPING);
        waitForControllerMessage(mMockController, ChatController.EVENT_TEXT_CHANGED, MOCK_NUMBER,
                false);
        mInstanceField.set(null, null);
        Logger.d(TAG, "testCase1_SendChatModeTest() exit");
    }

    /**
     * Test to enter chat mode in chat manager
     * 
     * @throws Throwable
     */
    public void testCase2_EnterChatMode() throws Throwable {
        Logger.d(TAG, "testCase2_EnterChatMode() entry");
        mChatManager.enterChatMode(MOCK_NUMBER);
        waitForControllerMessage(mMockController, ChatController.EVENT_SHOW_WINDOW, MOCK_NUMBER);
        mInstanceField.set(null, null);
        Logger.d(TAG, "testCase2_EnterChatMode() exit");
    }

    /**
     * Test to exit chat mode in chat manager
     * 
     * @throws Throwable
     */
    public void testCase3_ExitFromChatMode() throws Throwable {
        Logger.d(TAG, "testCase3_ExitFromChatMode() entry");
        mChatManager.exitFromChatMode(MOCK_NUMBER);
        waitForControllerMessage(mMockController, ChatController.EVENT_HIDE_WINDOW, MOCK_NUMBER);
        mInstanceField.set(null, null);
        Logger.d(TAG, "testCase3_ExitFromChatMode() exit");
    }

    private void waitForControllerMessage(MockLocalController localController, 
            int expectEventType, String expectTag, boolean expectData) throws InterruptedException {
        Logger.d(TAG, "waitForControllerMessage() expectEventType: " + expectEventType
                + " , expectTag: " + expectTag + " , expectData: " + expectData);
        assertNotNull(localController);
        assertNotNull(expectTag);
        long startTime = System.currentTimeMillis();
        while (expectEventType != localController.mEventType
                || !expectTag.equals(localController.mTag)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }
    
    private void waitForControllerMessage(MockLocalController localController, int expectEventType,
            String expectTag) throws InterruptedException {
        Logger.d(TAG, "waitForControllerMessage() expectEventType: " + expectEventType
                + " , expectTag: " + expectTag);
        assertNotNull(localController);
        assertNotNull(expectTag);
        long startTime = System.currentTimeMillis();
        while (expectEventType != localController.mEventType
                || !expectTag.equals(localController.mTag)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    private class MockLocalController extends ControllerImpl {
        private Handler mHandler = new Handler();
        private int mEventType = -1;
        private Object mTag = new Object();
        private Object mData = new Object();

        @Override
        public Message obtainMessage(int eventType, Object tag, Object data) {
            Logger.d(TAG, "obtainMessage() eventType: " + eventType + " , tag: " + tag + "data: " + data);
            Message message = mHandler.obtainMessage(eventType);
            mEventType = eventType;
            mTag = tag;
            mData = data;
            return message;
        }
    }
}
