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
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, {
 * 
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.test.model;

import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.GroupChat.GroupChatParticipants;
import com.mediatek.rcse.mvc.GroupChat.SessionStatus;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.GroupChat.SessionInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.SessionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.EmptyStackException;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test ControllerImpl.java
 */
public class ControllerImplTest extends InstrumentationTestCase {
    private static final String TAG = "ControllerImplTest";
    private static final String CONTACT_NUMBER1 = "+341";
    private static final String CONTACT_NUMBER2 = "+342";
    private static final String CONTACT_NUMBER3 = "+343";
    private static final String CONTACT_NUMBER4 = "+344";
    private static final String TEST_MESSAGE = "test message";
    private static final String MESSAGE_API = "mMessagingApi";
    private static final String FILE_PATH = "messages";
    private static final String CHAT_MAP = "mChatMap";
    private static final String FIELD_BACKGROUND = "mIsInBackground";
    private static final String FIELD_CHAT_WINDOW = "mChatWindow";
    private static final String FIELD_SESSION_STACK = "mSessionStack";
    private static final String CLASS_SESSION_INFO = "SessionInfo";
    private static final String METHOD_SESSION_STATUS = "getSessionStatus";
    private static final String METHOD_CONVERT_INFOS = "convertParticipantInfosToContacts";
    private static final String REGISTRATION_API = "mRegistrationApi";
    private static final String REGISTRATION_STATUS = "mRegistrationStatus";
    private static final String FIELD_ACTIVE_LIST = "mActiveList";
    private static final String FIELD_RESENDABLE_LIST = "mResendableList";
    private static final String FIELD_OUT_FTM = "mOutGoingFileTransferManager";
    private static final String FIELD_IN_FTM = "mReceiveFileTransferManager";
    private static final String CLASS_REV_FT = "ReceiveFileTransfer";
    private static final String CLASS_SEND_FT = "SentFileTransfer";
    private static final String FIELD_FT_TAG = "mFileTransferTag";
    private static final String METHOD_FT_INVITATION = "handleFileTransferInvitation";
    private static final int MSG_CNT = 10;
    private static final int TIME_OUT = 10000;
    private static final int MESSAGE_TAG = 111;
    private MockMessagingApi mMessagingApi;
    private MockRegistrationApi mRegistrationApi;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        ContactsListManager.initialize(getInstrumentation().getTargetContext());
        ApiManager.initialize(getInstrumentation().getTargetContext());
        ApiManager apiManager = ApiManager.getInstance();
        Field messageApiField = Utils.getPrivateField(ApiManager.class, MESSAGE_API);
        mMessagingApi = new MockMessagingApi(getInstrumentation().getTargetContext());
        messageApiField.set(apiManager, mMessagingApi);
        Field registrationApiField = Utils.getPrivateField(ApiManager.class, REGISTRATION_API);
        mRegistrationApi = new MockRegistrationApi(getInstrumentation().getTargetContext());
        registrationApiField.set(apiManager, mRegistrationApi);

        Field registrationStatusField = Utils.getPrivateField(mRegistrationApi.getClass()
                .getSuperclass(), REGISTRATION_STATUS);
        registrationStatusField.set(mRegistrationApi, new MockRegistrationStatus());
    }

    @Override
    protected void tearDown() throws Exception {
        getInstrumentation().getTargetContext().getContentResolver()
                .delete(UnregMessageProvider.CONTENT_URI, null, null);
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_CLOSE_ALL_WINDOW, null, null);
        controllerMessage.sendToTarget();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(ModelImpl.getInstance());
        waitToClean(chatMap, TIME_OUT);
        Utils.clearAllStatus();
        super.tearDown();
    }

    /**
     * Test case to open/close/closeAll Window()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase01_openCloseWindow() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        // open 1-2-1 window
        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        int afterSize = chatMap.size();
        assertEquals(afterSize, beforeSize + 1);

        // close 1-2-1 window
        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_CLOSE_WINDOW,
                null, participant1);
        controllerMessage.sendToTarget();
        waitToRemove(chatMap, tag1, TIME_OUT);

        beforeSize = chatMap.size();
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        // open group window
        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag2, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag2, TIME_OUT);

        afterSize = chatMap.size();
        assertEquals(afterSize, beforeSize + 1);

        // close group window
        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_CLOSE_WINDOW,
                tag2, null);
        controllerMessage.sendToTarget();
        waitToRemove(chatMap, tag2, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag2, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag2, TIME_OUT);

        // close all window
        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_CLOSE_ALL_WINDOW,
                null, null);
        controllerMessage.sendToTarget();
        waitToClean(chatMap, TIME_OUT);
    }

    /**
     * Test case to hide/show Window()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase02_showHideWindow() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        ChatImpl chat = (ChatImpl) chatMap.get(tag1);
        Field backgroundField = Utils.getPrivateField(ChatImpl.class, FIELD_BACKGROUND);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_HIDE_WINDOW,
                tag1, null);
        controllerMessage.sendToTarget();

        waitStatusChanged(chat, backgroundField, false, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_SHOW_WINDOW,
                tag1, null);
        controllerMessage.sendToTarget();

        waitStatusChanged(chat, backgroundField, true, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag2, participant2);
        controllerMessage.sendToTarget();
        ArrayList<Participant> participantList = new ArrayList<Participant>();
        participantList.add(participant2);
        chat = (ChatImpl) modelInstance.addChat(participantList, null, null);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_HIDE_WINDOW,
                CONTACT_NUMBER2, null);
        controllerMessage.sendToTarget();

        waitStatusChanged(chat, backgroundField, false, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_SHOW_WINDOW,
                CONTACT_NUMBER2, null);
        controllerMessage.sendToTarget();

        waitStatusChanged(chat, backgroundField, true, TIME_OUT);
    }

    /**
     * Test case to send message()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase03_sendMessage() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        MockChat chat = new MockChat(modelInstance, tag1);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        chatMap.put(tag1, chat);

        controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_SEND_MESSAGE, tag1, TEST_MESSAGE);
        controllerMessage.arg1 = MESSAGE_TAG;
        controllerMessage.sendToTarget();

        waitToSent(chat, TIME_OUT);

        controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_SEND_MESSAGE, CONTACT_NUMBER1, TEST_MESSAGE);
        controllerMessage.arg1 = MESSAGE_TAG;
        controllerMessage.sendToTarget();

        waitToSent(chat, TIME_OUT);
    }

    /**
     * Test case to query capability()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase04_queryCapability() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance, new MockOneOneChatWindow(),
                participant1, tag1);
        chatMap.put(tag1, oneoneChat);

        controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_QUERY_CAPABILITY, tag1, null);
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (oneoneChat.isCapabilityChecked()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to reload messages()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase05_reloadMessages() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance, new MockOneOneChatWindow(),
                participant1, tag1);
        chatMap.put(tag1, oneoneChat);
        ArrayList<Integer> data = new ArrayList<Integer>();
        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_RELOAD_MESSAGE,
                null, data);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
    }

    /**
     * Test case to quit group()
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase06_quitGroup() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_QUIT_GROUP_CHAT,
                tag1, list);
        controllerMessage.sendToTarget();

        Field sessionStackField = Utils.getPrivateField(GroupChat.class, FIELD_SESSION_STACK);
        Class<?>[] classes = GroupChat.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        Class sessionInfoClass = null;
        for (Class<?> classz : classes) {
            if (CLASS_SESSION_INFO.equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        ctr = ctrz;
                    }
                }
                sessionInfoClass = classz;
                break;
            }
        }
        Stack<Object> sessionStack = (Stack<Object>) sessionStackField.get(chatMap.get(tag1));
        Method method = sessionInfoClass.getDeclaredMethod(METHOD_SESSION_STATUS);
        method.setAccessible(true);
        try {
            long beginTime = System.currentTimeMillis();
            boolean success = false;
            while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
                if ((SessionStatus) method.invoke(sessionStack.peek()) == SessionStatus.TERMINATED) {
                    success = true;
                    break;
                }
            }
            assertTrue(success);
        } catch (EmptyStackException e) {
            e.printStackTrace();
        }
    }

    /**
     * Test case to add participant for grup chat.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase07_addParticipant() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        ArrayList<Participant> list2 = new ArrayList<Participant>();
        list2.add(participant3);
        list2.add(participant4);
        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_GROUP_ADD_PARTICIPANT, tag1, list2);
        controllerMessage.sendToTarget();

        GroupChat groupChat = (GroupChat) chatMap.get(tag1);
        Method method = GroupChat.class.getDeclaredMethod(METHOD_CONVERT_INFOS, List.class);
        method.setAccessible(true);
        try {
            long beginTime = System.currentTimeMillis();
            boolean success = false;
            while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
                List<String> contacts = (List<String>) method.invoke(groupChat,
                        groupChat.getParticipantInfos());
                if (contacts.contains(CONTACT_NUMBER3)) {
                    success = true;
                    break;
                }
            }
            assertTrue(success);
        } catch (EmptyStackException e) {
            e.printStackTrace();
        }
    }

    /**
     * Test case to clear history.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase08_clearHistory() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        ChatImpl chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_SEND_MESSAGE,
                CONTACT_NUMBER1, TEST_MESSAGE);
        controllerMessage.arg1 = MESSAGE_TAG;
        controllerMessage.sendToTarget();

        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_CLEAR_CHAT_HISTORY, tag1, null);
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (window.isMessagesRemoved()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag2, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag2, TIME_OUT);

        MockGroupChatWindow groupWindow = new MockGroupChatWindow();
        // chat = (ChatImpl) chatMap.get(tag2);
        // Field chatWindowField = Utils.getPrivateField(ChatImpl.class,
        // FIELD_CHAT_WINDOW);
        // chatWindowField.set(chat, groupWindow);
        chat = new MockGroupChat(modelInstance, groupWindow, list, tag2);
        chatMap.put(tag2, chat);

        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_CLEAR_CHAT_HISTORY, tag2, null);
        controllerMessage.sendToTarget();

        beginTime = System.currentTimeMillis();
        success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (groupWindow.isMessagesRemoved()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);

        window = new MockOneOneChatWindow();
        chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_CLEAR_CHAT_HISTORY, null, null);
        controllerMessage.sendToTarget();

        beginTime = System.currentTimeMillis();
        success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (window.isMessagesRemoved()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to send file transfer.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase09_sendFileTransfer() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        ChatImpl chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        Field fieldFtManager = Utils.getPrivateField(ModelImpl.class, FIELD_OUT_FTM);
        Object ftManager = fieldFtManager.get(modelInstance);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(), FIELD_ACTIVE_LIST);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);

        int sizeBefore = activeList.size();
        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_INVITATION, tag1, getFilePath());
        Bundle data = new Bundle();
        data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG, (Parcelable) tag2);
        controllerMessage.setData(data);
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (activeList.size() == sizeBefore + 1) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to reject file transfer.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase10_rejectFileTransfer() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        ChatImpl chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        Field fieldFtManager = Utils.getPrivateField(One2OneChat.class, FIELD_IN_FTM);
        Object ftManager = fieldFtManager.get(chat);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(), FIELD_ACTIVE_LIST);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);

        // Query One2OneChat inner class and create ReceiveFileTransfer object
        Class<?>[] calsses = One2OneChat.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : calsses) {
            if (CLASS_REV_FT.equals(classz.getSimpleName())) {
                ctr = classz.getConstructor(One2OneChat.class, IFileTransferSession.class);
                break;
            }
        }
        assertNotNull(ctr);
        MockFtSession ftSession = new MockFtSession();
        Object revFt = ctr.newInstance(chat, ftSession);
        Method method = Utils.getPrivateMethod(revFt.getClass(), METHOD_FT_INVITATION,
                IFileTransferSession.class);
        method.invoke(revFt, ftSession);
        activeList.add(revFt);

        int sizeBefore = activeList.size();
        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_RECEIVER_REJECT, tag1, ftSession.getSessionID());
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (ftSession.isRejected()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to accept file transfer.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase11_acceptFileTransfer() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        ChatImpl chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        Field fieldFtManager = Utils.getPrivateField(One2OneChat.class, FIELD_IN_FTM);
        Object ftManager = fieldFtManager.get(chat);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(), FIELD_ACTIVE_LIST);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);

        Class<?>[] calsses = One2OneChat.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : calsses) {
            if (CLASS_REV_FT.equals(classz.getSimpleName())) {
                ctr = classz.getConstructor(One2OneChat.class, IFileTransferSession.class);
                break;
            }
        }
        assertNotNull(ctr);
        MockFtSession ftSession = new MockFtSession();
        Object revFt = ctr.newInstance(chat, ftSession);
        Method method = Utils.getPrivateMethod(revFt.getClass(), METHOD_FT_INVITATION,
                IFileTransferSession.class);
        method.invoke(revFt, ftSession);
        activeList.add(revFt);

        int sizeBefore = activeList.size();
        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_RECEIVER_ACCEPT, tag1, ftSession.getSessionID());
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (ftSession.isAccepted()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to cancel file transfer.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase12_cancelFileTransfer() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        ChatImpl chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        Field fieldFtManager = Utils.getPrivateField(One2OneChat.class, FIELD_IN_FTM);
        Object ftManager = fieldFtManager.get(chat);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(), FIELD_ACTIVE_LIST);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);

        Class<?>[] calsses = One2OneChat.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : calsses) {
            if (CLASS_REV_FT.equals(classz.getSimpleName())) {
                ctr = classz.getConstructor(One2OneChat.class, IFileTransferSession.class);
                break;
            }
        }
        assertNotNull(ctr);
        MockFtSession ftSession = new MockFtSession();
        Object revFt = ctr.newInstance(chat, ftSession);
        Method method = Utils.getPrivateMethod(revFt.getClass(), METHOD_FT_INVITATION,
                IFileTransferSession.class);
        method.invoke(revFt, ftSession);
        activeList.add(revFt);

        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_CANCEL, tag1, ftSession.getSessionID());
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (ftSession.isCanceled()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to resend file transfer.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase13_resendFileTransfer() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        int beforeSize = chatMap.size();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        MockOneOneChat chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        Field fieldFtManager = Utils.getPrivateField(ModelImpl.class, FIELD_OUT_FTM);
        Object ftManager = fieldFtManager.get(modelInstance);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(), FIELD_ACTIVE_LIST);
        Field fieldResendableList = Utils.getPrivateField(ftManager.getClass(),
                FIELD_RESENDABLE_LIST);
        CopyOnWriteArrayList<Object> resendList = (CopyOnWriteArrayList<Object>) fieldResendableList
                .get(ftManager);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);

        MockFtSession ftSession = new MockFtSession();
        Object sentFt = chat.generateSentFileTransfer(getFilePath(), ftSession.getSessionID());
        resendList.add(sentFt);

        int sizeBefore = activeList.size();
        controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_RESENT, null, ftSession.getSessionID());
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (activeList.size() == sizeBefore + 1) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to get chat history.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase14_getChatHistory() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        MockOneOneChat chat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, chat);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_GET_CHAT_HISTORY,
                tag1, Integer.toString(MSG_CNT));
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (chat.isLoaded()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test case to text has changed.
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase15_textHasChanged() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, InstantiationException, RemoteException {
        ControllerImpl controllerInstance = ControllerImpl.getInstance();
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);

        MockOneOneChatWindow window = new MockOneOneChatWindow();
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();

        Message controllerMessage = controllerInstance.obtainMessage(
                ChatController.EVENT_OPEN_WINDOW, tag1, participant1);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag1, TIME_OUT);
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance, window, participant1, tag1);
        chatMap.put(tag1, oneoneChat);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_TEXT_CHANGED,
                tag1, false);
        controllerMessage.sendToTarget();

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (!oneoneChat.isEmpty()) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_OPEN_WINDOW,
                tag2, list);
        controllerMessage.sendToTarget();
        waitToAdd(chatMap, tag2, TIME_OUT);
        MockGroupChat groupChat = new MockGroupChat(modelInstance, new MockGroupChatWindow(), list,
                tag2);
        chatMap.put(tag2, groupChat);

        controllerMessage = controllerInstance.obtainMessage(ChatController.EVENT_TEXT_CHANGED,
                tag2, true);
        controllerMessage.sendToTarget();

        beginTime = System.currentTimeMillis();
        success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (groupChat.isEmpty()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private void waitToAdd(Map<?, ?> map, ParcelUuid tag, long timeout) {
        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < timeout) {
            if (map.containsKey(tag)) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private void waitToRemove(Map<?, ?> map, ParcelUuid tag, long timeout) {
        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < timeout) {
            if (!map.containsKey(tag)) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private void waitToClean(Map<?, ?> map, long timeout) {
        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < timeout) {
            if (map.size() == 0) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private void waitToSent(MockChat chat, long timeout) {
        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < timeout) {
            if (chat.isMessageSent()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private void waitStatusChanged(ChatImpl chat, Field backgroundField, boolean initValue,
            long timeout) throws IllegalAccessException {
        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < timeout) {
            if ((Boolean) backgroundField.get(chat) != initValue) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    private class MockMessagingApi extends MessagingApi {
        private boolean mDisplayed = false;

        public boolean isDisplayed() {
            return mDisplayed;
        }

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession initiateAdhocGroupChatSession(List<String> participants, String firstMsg,String x)
                throws ClientApiException {
            return super.initiateAdhocGroupChatSession(participants, firstMsg,null);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            return super.getChatSession(id);
        }

        @Override
        public IFileTransferSession getFileTransferSession(String id) throws ClientApiException {
            return super.getFileTransferSession(id);
        }

        @Override
        public void setMessageDeliveryStatus(String contact, String msgId, String status)
                throws ClientApiException {
            mDisplayed = true;
        }
    }

    /**
     * Mock chat.
     */
    private class MockChat extends ChatImpl {
        private boolean mMsgSent = false;

        protected MockChat(ModelImpl modelImpl, Object tag) {
            modelImpl.super(tag);
        }

        public boolean isMessageSent() {
            return mMsgSent;
        }

        @Override
        public void onCapabilityChanged(String contact, Capabilities capabilities) {

        }

        @Override
        public void onStatusChanged(boolean status) {

        }

        protected void checkCapabilities() {

        }

        protected void queryCapabilities() {

        }

        public void loadChatMessages(int count) {

        }

        protected void reloadMessage(final InstantMessage message, final int messageType,
                final int status) {

        }

        public void handleInvitation(IChatSession chatSession, ArrayList<IChatMessage> messages) {

        }

        public void sendMessage(String content, int messageTag) {
            mMsgSent = true;
        }

		@Override
		public void handleInvitation(IChatSession chatSession,
				ArrayList<IChatMessage> messages, boolean isAutoAccept) {
			// TODO Auto-generated method stub
			
		}
    }

    /**
     * Mock 1-2-1 chat.
     */
    private class MockOneOneChat extends One2OneChat {
        private boolean mIsMessageSent = false;
        private boolean mChecked = false;
        private boolean mReloaded = false;
        private boolean mLoaded = false;
        private boolean mIsEmpty = false;

        public MockOneOneChat(ModelImpl modelImpl, IOne2OneChatWindow chatWindow,
                Participant participant, Object tag) {
            super(modelImpl, chatWindow, participant, tag);
        }

        public boolean isMessageSent() {
            return mIsMessageSent;
        }

        public boolean isReloaded() {
            return mReloaded;
        }

        public boolean isLoaded() {
            return mLoaded;
        }

        public boolean isCapabilityChecked() {
            return mChecked;
        }

        public boolean isEmpty() {
            return mIsEmpty;
        }

        public void sendMessage(String content, int messageTag) {
            super.sendMessage(content, messageTag);
            mIsMessageSent = true;
        }

        protected void checkAllCapability() {
            super.checkAllCapability();
            mChecked = true;
        }

        protected void reloadMessage(final InstantMessage message, final int messageType,
                final int status) {
            super.reloadMessage(message, messageType, status);
            mReloaded = true;
        }

        public void loadChatMessages(int count) {
            super.loadChatMessages(count);
            mLoaded = true;
        }

        public void hasTextChanged(boolean isEmpty) {
            mIsEmpty = isEmpty;
        }
    }

    /**
     * Mock group chat.
     */
    private class MockGroupChat extends GroupChat {
        private boolean mIsMessageSent = false;
        private boolean mReloaded = false;
        private boolean mIsEmpty = false;

        public MockGroupChat(ModelImpl modelImpl, IGroupChatWindow chatWindow,
                List<Participant> participants, Object tag) {
            super(modelImpl, chatWindow, participants, tag);
        }

        public boolean isMessageSent() {
            return mIsMessageSent;
        }

        public boolean isReloaded() {
            return mReloaded;
        }

        public boolean isEmpty() {
            return mIsEmpty;
        }

        public void sendMessage(String content, int messageTag) {
            super.sendMessage(content, messageTag);
            mIsMessageSent = true;
        }

        protected void reloadMessage(final InstantMessage message, final int messageType,
                final int status) {
            super.reloadMessage(message, messageType, status);
            mReloaded = true;
        }

        public void hasTextChanged(boolean isEmpty) {
            mIsEmpty = isEmpty;
        }
    }

    private class MockGroupChatWindow implements IGroupChatWindow {
        private boolean mMessagesRemoved = false;

        public boolean isMessagesRemoved() {
            return mMessagesRemoved;
        }

        public void updateParticipants(List<ParticipantInfo> participants) {

        }

        public void setIsComposing(boolean isComposing, Participant participant) {

        }

        public void setIsRejoining(boolean isRejoining) {

        }

        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return null;
        }

        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            return null;
        }

        public void removeAllMessages() {
            mMessagesRemoved = true;
        }

        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        public void addLoadHistoryHeader(boolean showLoader) {

        }

        public void updateAllMsgAsRead() {

        }

		@Override
        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

		@Override
		public void removeChatMessage(String messageId) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void setFileTransferEnable(int reason) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void updateChatStatus(int status) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void addgroupSubject(String subject) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IFileTransfer addSentFileTransfer(FileStruct file) {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}

    }

    /**
     * Mock chat window for test
     */
    private class MockOneOneChatWindow implements IOne2OneChatWindow {
        private boolean mUpdated = false;
        private boolean mMessagesRemoved = false;
        private One2OneChatFragment mOne2OneChatFragment = new One2OneChatFragment();
        private One2OneChatFragment.ReceivedFileTransfer mReceivedFileTransfer = null;
        private One2OneChatFragment.SentFileTransfer mSentFileTransfer = null;

        public boolean isUpdated() {
            return mUpdated;
        }

        public boolean isMessagesRemoved() {
            return mMessagesRemoved;
        }

        public void setFileTransferEnable(int reason) {

        }

        public void setIsComposing(boolean isComposing) {

        }

        public void setRemoteOfflineReminder(boolean isOffline) {

        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            UUID uuid = UUID.randomUUID();
            ParcelUuid parcelUuid = new ParcelUuid(uuid);
            Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
            final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
            mOne2OneChatFragment = oneChatWindow.getFragment();
            mSentFileTransfer = mOne2OneChatFragment.new SentFileTransfer(file);
            return mSentFileTransfer;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            UUID uuid = UUID.randomUUID();
            ParcelUuid parcelUuid = new ParcelUuid(uuid);
            Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
            final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
            mOne2OneChatFragment = oneChatWindow.getFragment();
            mReceivedFileTransfer = mOne2OneChatFragment.new ReceivedFileTransfer(file, mMessagesRemoved);
            return mReceivedFileTransfer;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            return null;
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry");
            mMessagesRemoved = true;
        }

        @Override
        public void updateAllMsgAsRead() {
            Logger.d(TAG, "updateAllMsgAsRead() entry");
            mUpdated = true;
        }

		@Override
        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

		@Override
		public void removeChatMessage(String messageId) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}
    }

    private class MockRegistrationApi extends RegistrationApi {
        public MockRegistrationApi(Context context) {
            super(context);
        }
    }

    private class MockRegistrationStatus extends IRegistrationStatus.Stub {
        public void addRegistrationStatusListener(IRegistrationStatusRemoteListener listener) {

        }

        public boolean isRegistered() {
            return true;
        }
    }

    private String getFilePath() {
        Cursor cursor = getInstrumentation().getTargetContext().getContentResolver()
                .query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
        String filePath = null;
        try {
            if (null != cursor) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        assertNotNull(filePath);
        return filePath;
    }

    /**
     * Mock chat session that has a mock session-id , a mock file size and a
     * mock file name.
     */
    private class MockFtSession implements IFileTransferSession {
        private String mSessionId = SessionIdGenerator.getNewId();
        private boolean mIsAccepted = false;
        private boolean mIsRejected = false;
        private boolean mIsCanceled = false;

        public boolean isAccepted() {
            return mIsAccepted;
        }

        @Override
        public boolean isSessionPaused() {
        	return false;
        }
        
        @Override
        public int getSessionDirection() {
        	return 0;
        }

        public boolean isCanceled() {
            return mIsCanceled;
        }

        public boolean isRejected() {
            return mIsRejected;
        }

        @Override
        public void acceptSession() throws RemoteException {
            mIsAccepted = true;
        }

        @Override
        public void addSessionListener(IFileTransferEventListener listener) throws RemoteException {
        }

        @Override
        public void cancelSession() throws RemoteException {
            mIsCanceled = true;
        }

        @Override
        public String getFilename() throws RemoteException {
            return null;
        }

        @Override
        public long getFilesize() throws RemoteException {
            return 1L;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return CONTACT_NUMBER1;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return mSessionId;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return 0;
        }

        @Override
        public void rejectSession() throws RemoteException {
            mIsRejected = true;
        }

        @Override
        public void removeSessionListener(IFileTransferEventListener listener)
                throws RemoteException {
        }

        @Override
        public IBinder asBinder() {
            return null;
        }

		@Override
		public List<String> getContacts() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isGroupTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public boolean isHttpTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String getChatID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public String getChatSessionID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void pauseSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void resumeSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public String getFileThumbUrl() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}
    }
}
