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

package com.mediatek.rcse.test.model;

import android.content.Context;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.GroupChat.NetworkConnectivityListener;
import com.mediatek.rcse.mvc.GroupChat.SessionInfo;
import com.mediatek.rcse.mvc.GroupChat.SessionStatus;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.GroupChat.GroupChatParticipants;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.network.NetworkConnectivityApi;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatError;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.EmptyStackException;
import java.util.HashMap;
import java.util.List;
import java.util.Stack;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicReference;

/**
 * This class is used to test the functions like sendmessage in groupchat.
 */
public class GroupChatTest extends AndroidTestCase {
    private static final String CONTACT_NUMBER1 = "+34200000230";
    private static final String CONTACT_NUMBER2 = "+34200000251";
    private static final String CONTACT_NUMBER3 = "+34200000252";
    private static final String CONTACT_NUMBER4 = "+34200000253";
    private static final String CONTACT_NUMBER5 = "+34200000254";
    private static final String METHOD_SEND_MESSAGE = "sendMessage";
    private static final String METHOD_ADD_PARTICIPANTS = "addParticipants";
    private static final String METHOD_UPDATE_PARTICIPANTS = "updateParticipants";
    private static final String CONTENT_SEND_MESSAGE = "test message";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String CAPABILITY_API = "mCapabilitiesApi";
    private static final String MESSAGE_API = "mMessagingApi";
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;
    private ArrayList<Participant> mParticipantList = new ArrayList<Participant>();
    private static final String TAG = "GroupChatTest";
    private static final String FIELD_NC_LISTENER = "mNetworkListener";
    private static final String FIELD_AUTO_REJOIN = "mShouldAutoRejoin";
    private GroupChat mChat = null;
    private static final String CHAT_ID = "test chat id";
    private static final String REJOIN_GROUP = "rejoinGroup";
    private static final String RESTART_GROUP = "restartGroup";
    private static final String SESSION_STACK = "mSessionStack";
    private static final String RESTORE_MESSAGES = "restoreMessages";
    private static final String CLEAR_MESSAGES = "clearRestoredMessages";
    private static final String CLEAR_HISTORY = "clearGroupHistory";
    private static final String MESSAGES_SEND_DELAYED = "mMessagesToSendDelayed";
    private static final String REGISTER = "mIsRegistered";
    private static final String METHOD_INIT = "init";
    private static final String FIELD_NETWORK_API = "mNetworkConnectivityApi";
    private static final int DEFAULT_MESSAGE_TAG = -1;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        ApiManager.initialize(null);
        ApiManager.initialize(mContext);
        ApiManager.initialize(mContext);

        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp apiManager is null", apiManager);
        CapabilityApi capabilityApi = apiManager.getCapabilityApi();
        long startTime = System.currentTimeMillis();
        while (capabilityApi == null) {
            Logger.d(TAG, "setUp() capabilityApi is null");
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            capabilityApi = apiManager.getCapabilityApi();
        }
        Field capabilityApiField = Utils.getPrivateField(ApiManager.class, CAPABILITY_API);
        MockCapabilityApi mockCapabilityApi = new MockCapabilityApi(mContext);
        capabilityApiField.set(apiManager, mockCapabilityApi);

        Field messageApiField = Utils.getPrivateField(ApiManager.class, MESSAGE_API);
        messageApiField.set(apiManager, new MockMessagingApi(mContext));

        Participant participant = new Participant(CONTACT_NUMBER1, "");
        mParticipantList.add(participant);
        participant = new Participant(CONTACT_NUMBER2, "");
        mParticipantList.add(participant);
        participant = new Participant(CONTACT_NUMBER3, "");
        mParticipantList.add(participant);
        IChatManager instance = ModelImpl.getInstance();
        assertNotNull("setUp instance is null", instance);
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        mChat = new GroupChat((ModelImpl) instance, null, mParticipantList, parcelUuid);
        mChat.setChatWindow(new MockChatWindow());
        Logger.d(TAG, "setUp() exit");
    }

    /*
     * This test case is for the case that send message from invite.
     */
    public void testCase01_SendMessageFromInvite() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase01_SendMessageFromInvite() entry");
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Field allMessageField = Utils.getPrivateField(mChat.getClass(), "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField.get(mChat);
        waitForMessageAdded(allMessage);
        Logger.v(TAG, "testCase1_SendMessageFromInvite() exit");
    }

    /*
     * This test case is for the case that send message from session.
     */
    public void testCase02_SendMessageFromSession() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase02_SendMessageFromSession entry");
        Field currentSessionField = Utils.getPrivateField(mChat.getClass().getSuperclass(),
                "mCurrentSession");
        AtomicReference<IChatSession> currentSession = (AtomicReference<IChatSession>) currentSessionField
                .get(mChat);
        currentSession.set(new MockImSession());
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Field allMessageField = Utils.getPrivateField(mChat.getClass(), "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField.get(mChat);
        waitForMessageAdded(allMessage);
        Logger.v(TAG, "testCase2_SendMessageFromSession exit");

    }

    /**
     * Test case for network disconnect.
     */
    public void testCase03_NetworkDisconnect() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException {
        Logger.v(TAG, "testCase03_NetworkDisconnect entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.set(mChat, sessionStack);

        Field registerField = Utils.getPrivateField(mChat.getClass(), REGISTER);
        registerField.set(mChat, true);
        Field networkField = Utils.getPrivateField(mChat.getClass(), FIELD_NETWORK_API);
        networkField.set(mChat, new NetworkConnectivityApi(mContext));
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_INIT);
        method.invoke(mChat);

        registerField.set(mChat, true);

        Field ncListenerField = Utils.getPrivateField(mChat.getClass(), FIELD_NC_LISTENER);
        NetworkConnectivityListener ncListener = (NetworkConnectivityListener) ncListenerField
                .get(mChat);
        Field autoRejoinField = Utils.getPrivateField(ncListener.getClass(), FIELD_AUTO_REJOIN);
        ncListener.prepareToDisconnect();
        Boolean shouldAutoRejoin = (Boolean) autoRejoinField.get(ncListener);
        assertTrue(shouldAutoRejoin);
    }

    /*
     * This test case is for the case that add participants.
     */
    public void testCase04_addParticipants() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase04_addParticipants entry");
        Field currentSessionField = Utils.getPrivateField(mChat.getClass().getSuperclass(),
                "mCurrentSession");
        AtomicReference<IChatSession> currentSession = (AtomicReference<IChatSession>) currentSessionField
                .get(mChat);
        currentSession.set(new MockImSession());
        ArrayList<Participant> participantList = new ArrayList<Participant>();
        Participant participant = new Participant(CONTACT_NUMBER4, "");
        participantList.add(participant);
        participant = new Participant(CONTACT_NUMBER5, "");
        participantList.add(participant);
        ArrayList<ParticipantInfo> participantListBefore = new ArrayList<ParticipantInfo>(
                mChat.getParticipantInfos());
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_ADD_PARTICIPANTS,
                List.class);
        method.invoke(mChat, participantList);
        List<ParticipantInfo> participantListAfter = mChat.getParticipantInfos();
        int increaseNum = participantListAfter.size() - participantListBefore.size();
        assertTrue(increaseNum > 0);
        Logger.v(TAG, "testCase4_addParticipants exit");
    }

    /*
     * This test case is for the case that update connected participants.
     */
    public void testCase05_UpdateParticipantsForConnected() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase05_UpdateParticipantsForConnected entry");
        ArrayList<ParticipantInfo> participantListBefore = new ArrayList<ParticipantInfo>(
                mChat.getParticipantInfos());
        Method method = Utils.getPrivateMethod(GroupChatParticipants.class,
                METHOD_UPDATE_PARTICIPANTS, String.class, String.class, String.class, List.class);
        method.invoke(mChat.getGroupChatParticipants(), CONTACT_NUMBER2, CONTENT_SEND_MESSAGE,
                User.STATE_CONNECTED, new ArrayList<String>());
        List<ParticipantInfo> participantListAfter = mChat.getParticipantInfos();
        int increaseNum = participantListAfter.size() - participantListBefore.size();
        assertTrue(increaseNum == 0);
        Logger.v(TAG, "testCase5_UpdateParticipantsForConnected exit");
    }

    /*
     * This test case is for the case that update disconnected participants.
     */
    public void testCase06_UpdateParticipantsForDisconnected() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase06_UpdateParticipantsForDisconnected entry");
        ArrayList<ParticipantInfo> participantListBefore = new ArrayList<ParticipantInfo>(
                mChat.getParticipantInfos());
        Method method = Utils.getPrivateMethod(GroupChatParticipants.class,
                METHOD_UPDATE_PARTICIPANTS, String.class, String.class, String.class, List.class);
        method.invoke(mChat.getGroupChatParticipants(), CONTACT_NUMBER2, CONTENT_SEND_MESSAGE,
                User.STATE_DISCONNECTED, new ArrayList<String>());
        List<ParticipantInfo> participantListAfter = mChat.getParticipantInfos();
        int increaseNum = participantListAfter.size() - participantListBefore.size();
        assertTrue(increaseNum == 0);
        Logger.v(TAG, "testCase6_UpdateParticipantsForDisconnected exit");
    }

    /*
     * This test case is for the case that update disconnected participants.
     */
    public void testCase07_UpdateParticipantsDeclined() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase07_UpdateParticipantsDeclined entry");
        ArrayList<ParticipantInfo> participantListBefore = new ArrayList<ParticipantInfo>(
                mChat.getParticipantInfos());
        Method method = Utils.getPrivateMethod(GroupChatParticipants.class,
                METHOD_UPDATE_PARTICIPANTS, String.class, String.class, String.class, List.class);
        method.invoke(mChat.getGroupChatParticipants(), CONTACT_NUMBER2, CONTENT_SEND_MESSAGE,
                User.STATE_DECLINED, new ArrayList<String>());
        List<ParticipantInfo> participantListAfter = mChat.getParticipantInfos();
        int increaseNum = participantListAfter.size() - participantListBefore.size();
        Logger.v(TAG,
                "testCase12_UpdateParticipantsDeclined increaseNum: " + participantListAfter.size()
                        + " " + participantListBefore.size());
        assertTrue(increaseNum < 0);
    }

    /*
     * This test case is for the case that update disconnected participants.
     */
    public void testCase08_UpdateParticipantsDeparted() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.v(TAG, "testCase08_UpdateParticipantsDeparted entry");
        ArrayList<ParticipantInfo> participantListBefore = new ArrayList<ParticipantInfo>(
                mChat.getParticipantInfos());
        Method method = Utils.getPrivateMethod(GroupChatParticipants.class,
                METHOD_UPDATE_PARTICIPANTS, String.class, String.class, String.class, List.class);
        method.invoke(mChat.getGroupChatParticipants(), CONTACT_NUMBER2, CONTENT_SEND_MESSAGE,
                User.STATE_DEPARTED, new ArrayList<String>());
        List<ParticipantInfo> participantListAfter = mChat.getParticipantInfos();
        int increaseNum = participantListAfter.size() - participantListBefore.size();
        assertTrue(increaseNum < 0);
        Logger.v(TAG, "testCase13_UpdateParticipantsDeparted exit");
    }

    /*
     * This test case is for for the listener that HandleSessionStarted.
     */
    public void testCase09_HandleSessionStarted() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException {
        Logger.v(TAG, "testCase09_HandleSessionStarted() entry");
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Field currentSessionField = Utils.getPrivateField(mChat.getClass().getSuperclass(),
                "mCurrentSession");
        AtomicReference<IChatSession> currentSession = (AtomicReference<IChatSession>) currentSessionField
                .get(mChat);
        MockImSession session = (MockImSession) currentSession.get();
        session.getListener().handleSessionStarted();
        currentSession = (AtomicReference<IChatSession>) currentSessionField.get(mChat);
        assertNotNull(currentSession.get());
    }

    /*
     * This test case is for the listener that HandleImError.
     */
    public void testCase10_HandleImErrorListener() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException {
        Logger.v(TAG, "testCase10_HandleImErrorListener() entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.setAccessible(true);
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        ((MockImSession) (info.getSession())).getListener().handleImError(
                ChatError.SESSION_INITIATION_FAILED);
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        assertEquals(info.getSessionStatus(), SessionStatus.MANULLY_REJOIN);
    }

    /*
     * This test case is for the listener that HandleSessionAborted.
     */
    public void testCase11_HandleSessionAbortedListener() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException {
        Logger.v(TAG, "testCase11_HandleSessionAbortedListener() entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.setAccessible(true);
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        ((MockImSession) (info.getSession())).getListener().handleSessionAborted(0);
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        assertEquals(info.getSessionStatus(), SessionStatus.MANULLY_REJOIN);
    }

    /*
     * This test case is for the listener that HandleSessionTerminatedByRemote.
     */
    public void testCase12_HandleSessionTerminatedByRemoteListener() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase12_HandleSessionTerminatedByRemoteListener() entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.setAccessible(true);
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        ((MockImSession) (info.getSession())).getListener().handleSessionTerminatedByRemote();
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        assertEquals(info.getSessionStatus(), SessionStatus.TERMINATED);
    }

    /**
     * Test case for network connect.
     */
    public void testCase13_NetworkConnect() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, RemoteException {
        Logger.v(TAG, "testCase13_NetworkConnect() entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.set(mChat, sessionStack);

        Field registerField = Utils.getPrivateField(mChat.getClass(), REGISTER);
        registerField.set(mChat, true);
        Field networkField = Utils.getPrivateField(mChat.getClass(), FIELD_NETWORK_API);
        networkField.set(mChat, new NetworkConnectivityApi(mContext));
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_INIT);
        method.invoke(mChat);

        registerField.set(mChat, false);

        Field ncListenerField = Utils.getPrivateField(mChat.getClass(), FIELD_NC_LISTENER);
        NetworkConnectivityListener ncListener = (NetworkConnectivityListener) ncListenerField
                .get(mChat);
        Field autoRejoinField = Utils.getPrivateField(ncListener.getClass(), FIELD_AUTO_REJOIN);
        autoRejoinField.set(ncListener, true);
        ncListener.connect();
        Boolean shouldAutoRejoin = (Boolean) autoRejoinField.get(ncListener);
        assertFalse(shouldAutoRejoin);
    }

    /*
     * Test case for sendInvite().
     */
    public void testCase14_SendInvite() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase14_SendInvite() entry");
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.UNKNOWN));
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        assertEquals(info.getSessionStatus(), SessionStatus.INVITING);
    }

    /*
     * Test case for sendMsrpMessage().
     */
    public void testCase15_sendMsrpMessage() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase15_sendMsrpMessage() entry");
        Field allMessageField = Utils.getPrivateField(mChat.getClass(), "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField.get(mChat);
        int sizeBefore = allMessage.size();
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.ACTIVE));
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), METHOD_SEND_MESSAGE, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, DEFAULT_MESSAGE_TAG);
        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        waitForMessageAdded(allMessage);
        allMessage = (ArrayList<IChatMessage>) allMessageField.get(mChat);
        int sizeAfter = allMessage.size();
        assertEquals(sizeAfter, sizeBefore + 1);
    }

    /*
     * Test case for rejoinGroup().
     */
    public void testCase16_rejoinGroup() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase16_rejoinGroup() entry");
        Field messagesField = Utils.getPrivateField(mChat.getClass(), MESSAGES_SEND_DELAYED);
        HashMap<String, Integer> messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeBefore = messages.size();
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.MANULLY_REJOIN));
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), REJOIN_GROUP, String.class,
                String.class, int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, CHAT_ID, DEFAULT_MESSAGE_TAG);

        waitForMessageAdded(messages);

        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        assertEquals(info.getSessionStatus(), SessionStatus.REJOINING);
        messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeAfter = messages.size();
        assertEquals(sizeAfter, sizeBefore + 1);
    }

    /*
     * Test case for restartGroup().
     */
    public void testCase17_restartGroup() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase17_restartGroup() entry");
        Field messagesField = Utils.getPrivateField(mChat.getClass(), MESSAGES_SEND_DELAYED);
        HashMap<String, Integer> messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeBefore = messages.size();
        Field sessionStackField = Utils.getPrivateField(mChat.getClass(), SESSION_STACK);
        Stack<SessionInfo> sessionStack = new Stack<GroupChat.SessionInfo>();
        MockImSession session = new MockImSession();
        sessionStack.push(new SessionInfo(session, SessionStatus.MANULLY_RESTART));
        sessionStackField.set(mChat, sessionStack);
        Method method = Utils.getPrivateMethod(mChat.getClass(), RESTART_GROUP, String.class,
                String.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, CHAT_ID);

        waitForMessageAdded(messages);

        Stack<SessionInfo> stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        SessionInfo info = stack.peek();
        stack = (Stack<SessionInfo>) sessionStackField.get(mChat);
        info = stack.peek();
        messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeAfter = messages.size();
        assertEquals(sizeAfter, sizeBefore + 1);
    }

    /*
     * Test case for restoreMessages().
     */
    public void testCase18_restoreMessages() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase18_restoreMessages() entry");
        Field messagesField = Utils.getPrivateField(mChat.getClass(), MESSAGES_SEND_DELAYED);
        HashMap<String, Integer> messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeBefore = messages.size();
        Method method = Utils.getPrivateMethod(mChat.getClass(), RESTORE_MESSAGES, String.class,
                int.class);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, DEFAULT_MESSAGE_TAG);

        waitForMessageAdded(messages);

        messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeAfter = messages.size();
        assertEquals(sizeAfter, sizeBefore + 1);
    }

    /*
     * Test case for clearRestoredMessages().
     */
    public void testCase19_clearRestoredMessages() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase18_restoreMessages() entry");
        Field messagesField = Utils.getPrivateField(mChat.getClass(), MESSAGES_SEND_DELAYED);
        HashMap<String, Integer> messages = (HashMap<String, Integer>) messagesField.get(mChat);
        Method method = Utils.getPrivateMethod(mChat.getClass(), CLEAR_MESSAGES);
        method.invoke(mChat);

        waitForMessageRemoved(messages);

        messages = (HashMap<String, Integer>) messagesField.get(mChat);
        int sizeAfter = messages.size();
        assertEquals(sizeAfter, 0);
    }

    /*
     * Test case for clearGroupHistory().
     */
    public void testCase20_clearGroupHistory() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, RemoteException, EmptyStackException {
        Logger.v(TAG, "testCase18_restoreMessages() entry");
        Field messagesField = Utils.getPrivateField(mChat.getClass(), MESSAGES_SEND_DELAYED);
        HashMap<String, Integer> messages = (HashMap<String, Integer>) messagesField.get(mChat);
        Method method = Utils.getPrivateMethod(mChat.getClass(), CLEAR_HISTORY);
        boolean result = (Boolean) (method.invoke(mChat));
        assertTrue(result);
    }

    /**
     * Test GroupChatListener
     * @throws IllegalArgumentException
     * @throws NoSuchMethodException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase21_GroupChatListener() throws IllegalArgumentException,
            NoSuchMethodException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.v(TAG, "testCase21_GroupChatListener() entry");
        Object groupChatListener = getGroupChatListener();

        Method methodhandleAddParticipantFailed = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleAddParticipantFailed",
                String.class);
        methodhandleAddParticipantFailed.invoke(groupChatListener, "test");

        Method methodhandleAddParticipantSuccessful = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleAddParticipantSuccessful");
        methodhandleAddParticipantSuccessful.invoke(groupChatListener);

        Method methodhandleConferenceEvent = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleConferenceEvent",
                String.class, String.class, String.class);
        methodhandleConferenceEvent.invoke(groupChatListener, "test", "test",
                "test");

        Method methodhandleImError = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleImError", int.class);
        methodhandleImError.invoke(groupChatListener, 0);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleIsComposingEvent = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleIsComposingEvent",
                String.class, boolean.class);
        methodhandleIsComposingEvent.invoke(groupChatListener, CONTACT_NUMBER1,
                true);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleMessageDeliveryStatus = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleMessageDeliveryStatus",
                String.class, String.class, long.class);
        methodhandleMessageDeliveryStatus.invoke(groupChatListener,
                CONTACT_NUMBER1, ImdnDocument.DELIVERY_STATUS_DELIVERED,
                System.currentTimeMillis());
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleReceiveMessage = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleReceiveMessage",
                InstantMessage.class);
        InstantMessage instantMessage = new InstantMessage("test",
                CONTACT_NUMBER1, "test", true);
        methodhandleReceiveMessage.invoke(groupChatListener, instantMessage);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleSessionAborted = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleSessionAborted");
        methodhandleSessionAborted.invoke(groupChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleSessionStarted = Utils.getPrivateMethod(
                groupChatListener.getClass(), "handleSessionStarted");
        methodhandleSessionStarted.invoke(groupChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleSessionTerminatedByRemote = Utils
                .getPrivateMethod(groupChatListener.getClass(),
                        "handleSessionTerminatedByRemote");
        methodhandleSessionTerminatedByRemote.invoke(groupChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);
    }
    
    /**
     * Test GroupChatParticipants
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase22_GroupChatParticipants()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.v(TAG, "testCase22_GroupChatParticipants() entry");
        GroupChatParticipants groupChatParticipants = mChat.new GroupChatParticipants();
        Method methodadd = Utils.getPrivateMethod(
                groupChatParticipants.getClass(), "add", Participant.class,
                String.class);
        Participant participant = new Participant(CONTACT_NUMBER1,
                CONTACT_NUMBER1);
        String state = "connected";
        methodadd.invoke(groupChatParticipants, participant, state);
        assertNotNull(groupChatParticipants.convertParticipantsToNumbers());
        
        List<String> toBeInvited = new ArrayList<String>();
        String testNumber = "+86899200000999";
        toBeInvited.add(testNumber);
        assertTrue(groupChatParticipants.updateParticipants(testNumber, "test", state,
                toBeInvited));
        assertTrue(groupChatParticipants.updateParticipants(testNumber, null, state,
                toBeInvited));
        assertFalse(mChat.requestTransferFile(null));
        assertFalse(mChat.acceptTransferFile(false));
        mChat.onCapabilityChanged(null, null);
        
        // reloadMessage
        Method methodreloadMessage = Utils.getPrivateMethod(mChat.getClass(),
                "reloadMessage", InstantMessage.class, int.class, int.class);
        InstantMessage message = new InstantMessage("testId", CONTACT_NUMBER1,
                "test", true, new Date());
        methodreloadMessage.invoke(mChat, message,
                EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE, 0);
        methodreloadMessage.invoke(mChat, message, 100, 0);
        Thread.sleep(SLEEP_TIME);
        
       /* Field fieldmTimerOutTimers = Utils.getPrivateField(groupChatParticipants.getClass(), "mTimerOutTimers");
        groupChatParticipants.timerUnSchedule(info)*/
    }

    /**
     * Test class SessionInfo
     * @throws NoSuchFieldException
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     * @throws NoSuchMethodException 
     * @throws InvocationTargetException 
     */
    public void testCase23_SessionInfo() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.v(TAG, "testCase23_SessionInfo() entry");
        MockChatSession chatSession = null;
        GroupChat.SessionInfo sessionInfo = new GroupChat.SessionInfo(
                chatSession, SessionStatus.UNKNOWN);
        chatSession = new MockChatSession();
        sessionInfo.setSession(chatSession);
        assertEquals(chatSession, sessionInfo.getSession());

        Field fieldmSessionStack = Utils.getPrivateField(mChat.getClass(),
                "mSessionStack");
        @SuppressWarnings("unchecked")
        Stack<SessionInfo> sessionInfoStack = (Stack<SessionInfo>) fieldmSessionStack
                .get(mChat);
        sessionInfoStack.add(sessionInfo);

        Method methodquitGroup = Utils.getPrivateMethod(mChat.getClass(),
                "quitGroup");
        methodquitGroup.invoke(mChat);
        mChat.getAllMessages();
        MockChatSession chatSession2 = new MockChatSession();
        GroupChat.SessionInfo sessionInfo2 = new GroupChat.SessionInfo(
                chatSession2, SessionStatus.UNKNOWN);
        sessionInfoStack.add(sessionInfo2);
        mChat.clearGroupHistory();
    }
    
    /**
     * Test sendMessag
     * @throws NoSuchFieldException
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     */
    public void testCase24_sendMessage() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.v(TAG, "testCase24_sendMessage() entry");
        MockChatSession chatSession = new MockChatSession();
        mChat.handleInvitation(chatSession, null, false);
        mChat.handleInvitation(null, null, false);

        Field fieldmMessagingApi = Utils.getPrivateField(mChat.getClass(),
                "mMessagingApi");
        fieldmMessagingApi.set(mChat, null);
        String content = "test message";
        int messageTag = 0;
        mChat.sendMessage(content, messageTag);

        GroupChat.SessionInfo sessionInfo = new GroupChat.SessionInfo(
                chatSession, SessionStatus.MANULLY_REJOIN);

        Field fieldmSessionStack = Utils.getPrivateField(mChat.getClass(),
                "mSessionStack");
        @SuppressWarnings("unchecked")
        Stack<SessionInfo> sessionInfoStack = (Stack<SessionInfo>) fieldmSessionStack
                .get(mChat);
        sessionInfoStack.clear();
        sessionInfoStack.add(sessionInfo);
        mChat.sendMessage(content, messageTag);
        
        sessionInfo.setSessionStatus(SessionStatus.MANULLY_RESTART);
        sessionInfoStack.clear();
        sessionInfoStack.add(sessionInfo);
        mChat.sendMessage(content, messageTag);
        
        sessionInfo.setSessionStatus(SessionStatus.INVITING);
        sessionInfoStack.clear();
        sessionInfoStack.add(sessionInfo);
        mChat.sendMessage(content, messageTag);
        

    }

    private Object getGroupChatListener() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException {
        Class<?>[] clazzes = mChat.getClass().getDeclaredClasses();
        Constructor<?> ctorGroupChatListener = null;
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
            if ("GroupChatListener".equals(clazz.getSimpleName())) {
                ctorGroupChatListener = clazz.getDeclaredConstructor(
                        GroupChat.class, SessionInfo.class);
                ctorGroupChatListener.setAccessible(true);
                break;
            }
        }
        MockImSession imSession = new MockImSession();
        SessionInfo sessionInfo = new SessionInfo(imSession, SessionStatus.INVITING);
        Object groupChatListener = ctorGroupChatListener.newInstance(
                mChat, sessionInfo);
        Method methoddestroySelf = Utils.getPrivateMethod(
                groupChatListener.getClass(), "onDestroy");
        methoddestroySelf.invoke(groupChatListener);
        return groupChatListener;
    }
    
    private void waitForMessageAdded(List<IChatMessage> messageList) throws InterruptedException {
        int size = messageList.size();
        long startTime = System.currentTimeMillis();
        while (size == 0) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            size = messageList.size();
        }
    }

    private void waitForMessageAdded(HashMap<String, Integer> messages) throws InterruptedException {
        int size = messages.size();
        long startTime = System.currentTimeMillis();
        while (size == 0) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            size = messages.size();
        }
    }

    private void waitForMessageRemoved(HashMap<String, Integer> messages)
            throws InterruptedException {
        int size = messages.size();
        long startTime = System.currentTimeMillis();
        while (size != 0) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            size = messages.size();
        }
    }

    /**
     * Mock chat window for test
     */
    private class MockChatWindow implements IGroupChatWindow {

        public void updateParticipants(List<ParticipantInfo> participants) {
        }

        public void setIsComposing(boolean isComposing, Participant participant) {
        }

        public void setIsRejoining(boolean isRejoining) {
        }

        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return null;
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
        public void addLoadHistoryHeader(boolean showLoader) {
        }

        @Override
        public void removeAllMessages() {
        }

        @Override
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

    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession initiateAdhocGroupChatSession(List<String> participants, String firstMsg,String x)
                throws ClientApiException {
            Logger.d(TAG, "initiateAdhocGroupChatSession() entry");
            return new MockImSession();
        }

        @Override
        public IChatSession rejoinGroupChatSession(String chatId) throws ClientApiException {
            Logger.d(TAG, "rejoinChatGroupSession() entry");
            return new MockImSession();
        }
    }

    private class MockImSession extends IChatSession.Stub {

        IChatEventListener mListener = null;

        public IChatEventListener getListener() {
            return mListener;
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }	

        @Override
        public boolean isInComing() {
            return true;
        }

        public String getSessionID() {
            return null;
        }

        public String getChatID() {
            return CHAT_ID;
        }

        public String getRemoteContact() {
            return null;
        }

        public int getSessionState() {
            return SessionState.ESTABLISHED;
        }

        @Override
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        public boolean isGroupChat() {
            return true;
        }

        public boolean isStoreAndForward() {
            return false;
        }

        public InstantMessage getFirstMessage() {
            return new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true, null);
        }

        public void acceptSession() {

        }

        public void rejectSession() {

        }

        public void cancelSession() {

        }

        public List<String> getParticipants() {
            return null;
        }

        public void addParticipant(String participant) {

        }

        public void addParticipants(List<String> participants) {

        }

        public String sendMessage(String text) {
            return null;
        }

        public void setIsComposingStatus(boolean status) {

        }

        public void setMessageDeliveryStatus(String msgId, String status) {

        }

        public void addSessionListener(IChatEventListener listener) {
            mListener = listener;
        }

        public void removeSessionListener(IChatEventListener listener) {

        }

        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId,
                String status) {

        }

        public String getReferredByHeader() {
            return null;
        }

        public List<String> getInivtedParticipants() {
            return null;
        }

        public String getSubject() throws RemoteException {
            return null;
        }

        @Override
        public int getMaxParticipants() throws RemoteException {
            return 0;
        }

        @Override
        public int getMaxParticipantsToBeAdded() throws RemoteException {
            return 0;
        }

		@Override
		public boolean isGeolocSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String sendGeoloc(GeolocPush geoloc) throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isFileTransferSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public IFileTransferSession sendFile(String file, boolean thumbnail)
				throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void sendMessageWithMsgId(String text, String msgid)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

    }

    private class MockCapabilityApi extends CapabilityApi {
        public MockCapabilityApi(Context context) {
            super(context);
        }

        @Override
        public Capabilities getContactCapabilities(String number) {
            Logger.d(TAG, "getContactCapabilities() entry");
            Capabilities capability = new Capabilities();
            if (number.equals(CONTACT_NUMBER1) || number.equals(CONTACT_NUMBER4)) {
                capability.setImSessionSupport(false);
            } else {
                capability.setImSessionSupport(true);
            }
            return capability;
        }
    }
    
    /*
     * This class mock a ChatSession for test
     */
    private class MockChatSession implements IChatSession {

        @Override
        public boolean isInComing() {
            return true;
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }

        @Override
        public void acceptSession() throws RemoteException {

        }

        @Override
        public void addParticipant(String participant) throws RemoteException {

        }

        @Override
        public void addParticipants(List<String> participants) throws RemoteException {

        }

        @Override
        public void addSessionListener(IChatEventListener listener) throws RemoteException {

        }

        @Override
        public void cancelSession() throws RemoteException {

        }

        @Override
        public InstantMessage getFirstMessage() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getParticipants() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getInivtedParticipants() throws RemoteException {
            return null;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return null;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return null;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return 0;
        }

        @Override
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        @Override
        public boolean isGroupChat() throws RemoteException {
            return false;
        }

        @Override
        public boolean isStoreAndForward() throws RemoteException {
            return false;
        }

        @Override
        public void rejectSession() throws RemoteException {

        }

        @Override
        public void removeSessionListener(IChatEventListener listener) throws RemoteException {

        }

        @Override
        public String sendMessage(String text) throws RemoteException {
            return null;
        }

        @Override
        public void setIsComposingStatus(boolean status) throws RemoteException {
            Logger.i(TAG, "setIsComposingStatus status is " + status);

        }

        @Override
        public void setMessageDeliveryStatus(String msgId, String status) throws RemoteException {

        }

        @Override
        public IBinder asBinder() {
            return null;
        }

        @Override
        public String getChatID() throws RemoteException {
            // TODO
            return null;
        }
        
        @Override
        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId, String status) {
        }
        
        @Override
        public String getReferredByHeader(){
            return null;
        }
        
        public String getSubject() throws RemoteException {
            return null;
        }
        
        @Override
        public int getMaxParticipants() throws RemoteException {
            return 0;
        }
        
        @Override
        public int getMaxParticipantsToBeAdded() throws RemoteException {
            return 0;
        }

		@Override
		public boolean isGeolocSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String sendGeoloc(GeolocPush geoloc) throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isFileTransferSupported() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public IFileTransferSession sendFile(String file, boolean thumbnail)
				throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void sendMessageWithMsgId(String text, String msgid)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

    }
}
