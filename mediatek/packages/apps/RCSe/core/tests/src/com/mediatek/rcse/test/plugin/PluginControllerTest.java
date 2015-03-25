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

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.plugin.message.PluginController;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.TagTranslater;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;

/**
 * This test is used to test PluginController
 */
public class PluginControllerTest extends ServiceTestCase<ApiService> {
    private static final String TAG = "PluginControllerTest";
    private static final long SLEEP_TIME = 20;
    private static final long TIME_OUT = 500;
    
    private static final int MOCK_EVENT_TYPE_ONE = 0x901;
    private static final String MOCK_TAG_ONE = "tag-1";
    
    private static final int MOCK_EVENT_TYPE_TWO = 0x902;
    private static final boolean MOCK_DATA_TWO = true;
    private static final String MOCK_TAG_TWO = "tag-2";
    
    private static final int MOCK_EVENT_TYPE_THREE = 0x903;
    private static final String MOCK_TAG_THREE = "tag-3";
    private static final int MOCK_DATA_THREE = 0xff;
    
    private static final int MOCK_EVENT_TYPE_FOUR = 0x904;
    private static final String MOCK_TAG_FOUR = "tag-4";
    private static final String MOCK_DATA_FOUR = "mock data-4";
    
    private static final int MOCK_EVENT_TYPE_FIVE = 0x905;
    private static final String MOCK_TAG_FIVE = "tag-5";
    private static final ArrayList<Participant> MOCK_DATA_FIVE = new ArrayList<Participant>();
    private static final Participant MOCK_PARTICIPANT_ONE = new Participant("mock participant-1","mock participant-1");
    private static final Participant MOCK_PARTICIPANT_TWO = new Participant("mock participant-2","mock participant-2");
    private static final Participant MOCK_PARTICIPANT_THREE = new Participant("mock participant-3","mock participant-3");
    static {
        MOCK_DATA_FIVE.add(MOCK_PARTICIPANT_ONE);
        MOCK_DATA_FIVE.add(MOCK_PARTICIPANT_TWO);
        MOCK_DATA_FIVE.add(MOCK_PARTICIPANT_THREE);
    }

    public PluginControllerTest() {
        super(ApiService.class);
    }

    /**
     * Test to transfer a Message from Plugin to Controller in UI process
     * @throws Throwable
     */
    public void testCase1_MessageTransferTest() throws Throwable {
        Method saveTagMethod = Utils.getPrivateMethod(TagTranslater.class, "saveTag", Object.class);
        saveTagMethod.invoke(null, MOCK_TAG_ONE);
        saveTagMethod.invoke(null, MOCK_TAG_TWO);
        saveTagMethod.invoke(null, MOCK_TAG_THREE);
        saveTagMethod.invoke(null, MOCK_TAG_FOUR);
        saveTagMethod.invoke(null, MOCK_TAG_FIVE);
        Intent intent = new Intent(IRemoteWindowBinder.class.getName());
        IBinder binder = bindService(intent);
        IRemoteWindowBinder remoteChatWindowBinder = IRemoteWindowBinder.Stub.asInterface(binder);
        assertNotNull(remoteChatWindowBinder);
        IMessenger messenger = IMessenger.Stub.asInterface(remoteChatWindowBinder.getController());
        assertNotNull(messenger);
        Method initializeMethod = Utils.getPrivateMethod(PluginController.class, "initialize", IMessenger.class);
        initializeMethod.invoke(null, messenger);
        Field instanceField = Utils.getPrivateField(ControllerImpl.class, "sControllerImpl");
        MockLocalController mockController = new MockLocalController();
        instanceField.set(null, mockController);
        PluginController.obtainMessage(MOCK_EVENT_TYPE_ONE, MOCK_TAG_ONE).sendToTarget();
        waitForControllerMessage(mockController, MOCK_EVENT_TYPE_ONE, MOCK_TAG_ONE, null);
        
        PluginController.obtainMessage(MOCK_EVENT_TYPE_TWO, MOCK_TAG_TWO, MOCK_DATA_TWO).sendToTarget();
        waitForControllerMessage(mockController, MOCK_EVENT_TYPE_TWO, MOCK_TAG_TWO, MOCK_DATA_TWO);
        
        PluginController.obtainMessage(MOCK_EVENT_TYPE_THREE, MOCK_TAG_THREE, MOCK_DATA_THREE).sendToTarget();
        waitForControllerMessage(mockController, MOCK_EVENT_TYPE_THREE, MOCK_TAG_THREE, MOCK_DATA_THREE);
        
        PluginController.obtainMessage(MOCK_EVENT_TYPE_FOUR, MOCK_TAG_FOUR, MOCK_DATA_FOUR).sendToTarget();
        waitForControllerMessage(mockController, MOCK_EVENT_TYPE_FOUR, MOCK_TAG_FOUR, MOCK_DATA_FOUR);
        
        PluginController.obtainMessage(MOCK_EVENT_TYPE_FIVE, MOCK_TAG_FIVE, MOCK_DATA_FIVE).sendToTarget();
        waitForControllerMessage(mockController, MOCK_EVENT_TYPE_FIVE, MOCK_TAG_FIVE, MOCK_DATA_FIVE);
        instanceField.set(null, null);
        Method destroyInstanceMethod =
                Utils.getPrivateMethod(PluginController.class, "destroyInstance");
        destroyInstanceMethod.invoke(null);
    }

    private void waitForControllerMessage(MockLocalController localController, 
            int expectEventType, String expectTag, Object expectData) throws InterruptedException {
        Logger.d(TAG, "waitForControllerMessage() expectEventType: " + expectEventType
                + " , expectTag: " + expectTag + " , expectData: " + expectData);
        assertNotNull(localController);
        assertNotNull(expectTag);
        long startTime = System.currentTimeMillis();
        while (expectEventType != localController.mEventType
                || !expectTag.equals(localController.mTag)
                || !isDataEquals(expectData, localController.mData)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    private boolean isDataEquals(Object expectData, Object data) {
        Logger.d(TAG, "isDataEquals() expectData: " + expectData + " ,data: " + data);
        if (null == expectData){
            return (null == data);
        } else if (expectData instanceof Collection && data instanceof Collection) {
            Object[] expectCollection = ((Collection)expectData).toArray();
            Object[] collection = ((Collection)data).toArray();
            int count = expectCollection.length;
            if (count != collection.length) {
                return false;
            }
            for (int i = 0 ; i < count ; i++) {
                if (!expectCollection[i].equals(collection[i])) {
                    return false;
                }
            }
            return true;
        } else {
            return expectData.equals(data);
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
