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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.CharArrayBuffer;
import android.database.ContentObserver;
import android.database.Cursor;
import android.database.DataSetObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.test.InstrumentationTestCase;
import android.test.mock.MockContentResolver;
import android.util.Log;

import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.SessionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.provider.messaging.RichMessagingProvider;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.UUID;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicReference;

/**
 * This class is used to test ModelImpl.java
 */
public class ModelImplTest extends InstrumentationTestCase {
    private static final String CONTACT_NUMBER1 = "+34200000231";
    private static final String CONTACT_NUMBER2 = "+34200000251";
    private static final String CONTACT_NUMBER3 = "+34200000252";
    private static final String CONTACT_NUMBER4 = "+34200000253";
    private static final String METHOD_CHAT_INVITATION = "handleChatInvitation";
    private static final String METHOD_FT_INVITATION = "handleFileTransferInvitation";
    private static final String CONTENT_SEND_MESSAGE = "test message";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MESSAGE_API = "mMessagingApi";
    private static final String SESSION_ID_GROUP = "session_id_group";
    private static final String SESSION_ID_ONEONE = "session_id_oneone";
    private static final String INTENT_MESSAGE = "messages";
    private static final String TAG = "ModelImplTest";
    private static final String CHAT_MAP = "mChatMap";
    private static final int CHAT_NUM1 = 1;
    private static final int CHAT_NUM2 = 2;

    private static final int LOOP_COUNT = 5;
    private static final String NAME_A = "TesterA";
    private static final String NAME_B = "TesterB";
    private static final int TIME_OUT = 10000;
    private static final String METHOD_DISPLAY = "markMessageAsDisplayed";
    private static final String METHOD_TERMINATE_SESSION = "terminateSession";
    private static final String METHOD_REMOVE_CHAT = "removeChat";
    private static final String FIELD_OUTGOING_FTM = "mOutGoingFileTransferManager";
    private static final String FIELD_CURRENT_SESSSION = "mCurrentSession";
    private static final String FIELD_ACTIVE_LIST = "mActiveList";
    private static final String FIELD_PENDING_LIST = "mPendingList";
    private static final String FIELD_RESENDABLE_LIST = "mResendableList";
    private static final String METHOD_REMOVECHAT_BYCONTACT = "removeChatByContact";
    private MockMessagingApi mMessagingApi;
    private static final int SLEEP_TIME = 5000;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        ContactsListManager.initialize(getInstrumentation().getTargetContext());
        Field apiManagerfield = Utils.getPrivateField(ApiManager.class,
                API_MANAGER_INSTANCE);
        apiManagerfield.set(ApiManager.class, null);
        Method initializeMethod = Utils.getPrivateMethod(ApiManager.class,
                API_MANAGER_INITIALIZE, Context.class);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());
        ApiManager apiManager = ApiManager.getInstance();
        Field messageApiField = Utils.getPrivateField(ApiManager.class,
                MESSAGE_API);
        mMessagingApi = new MockMessagingApi(getInstrumentation()
                .getTargetContext());
        messageApiField.set(apiManager, mMessagingApi);
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Logger.v(TAG, "tearDown() exit");
    }

    /*
     * This test case is for the case that handle group Chat Invitation.
     */
    public void testCase1_HandleChatInvitationForGroup()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase1_HandleChatInvitationForGroup() entry");
        IChatManager modelInstance = ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        chatMap.clear();
        Intent intent = newChatInvitationIntent(SESSION_ID_GROUP);
        Method handleChatInvitation = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_CHAT_INVITATION, Intent.class, boolean.class);
        handleChatInvitation.invoke(modelInstance, intent, true);
        int size = chatMap.size();
        assertTrue(size == CHAT_NUM1);
        handleChatInvitation.invoke(modelInstance, intent, true);
        size = chatMap.size();
        assertTrue(size == CHAT_NUM2);
    }

    /*
     * This test case is for the case that handle oneone Chat Invitation.
     */
    public void testCase2_HandleChatInvitationForOneone()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase2_HandleChatInvitationForOneone() entry");
        IChatManager modelInstance = ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        chatMap.clear();
        Intent intent = newChatInvitationIntent(SESSION_ID_ONEONE);
        Method handleChatInvitation = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_CHAT_INVITATION, Intent.class, boolean.class);
        handleChatInvitation.invoke(modelInstance, intent, false);
        int size = chatMap.size();
        assertTrue(size == CHAT_NUM1);
        handleChatInvitation.invoke(modelInstance, intent, false);
        size = chatMap.size();
        assertTrue(size == CHAT_NUM1);
    }

    /*
     * This test case is for the case that handle filetransfer Invitation with
     * existed chat.
     */
    @SuppressWarnings("unchecked")
    public void testCase3_HandleFileTransferInvitation()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase3_HandleFileTransferInvitation() entry");
        IChatManager modelInstance = ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        chatMap.clear();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, "");
        One2OneChat oneoneChat = new One2OneChat((ModelImpl) modelInstance,
                new MockChatWindow(), participant, parcelUuid);
        chatMap.put(parcelUuid, oneoneChat);
        Method handleFtInvitation = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_FT_INVITATION, String.class);
        handleFtInvitation.invoke(modelInstance, SESSION_ID_ONEONE);
        int size = chatMap.size();
        assertTrue(size == CHAT_NUM1);

        handleFtInvitation.invoke(modelInstance, SESSION_ID_ONEONE + "test");
    }

    /**
     * This test case used for the case that clear all chat histoty
     */
    public void testCase4_clearAllChatHistory() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        chatMap.clear();
        Participant participant = new Participant(CONTACT_NUMBER1, "");
        One2OneChat oneoneChat = new One2OneChat(modelInstance,
                new MockChatWindow(), participant, parcelUuid);
        chatMap.put(parcelUuid, oneoneChat);
        ContentResolver contentResolver = getInstrumentation()
                .getTargetContext().getContentResolver();
        // clear the history database
        contentResolver.delete(RichMessagingData.CONTENT_URI, null, null);
        // insert the test data
        ContentValues contentValues = new ContentValues();
        for (int i = 0; i < LOOP_COUNT; i++) {
            ContentValues valuesA = new ContentValues();
            valuesA.put(RichMessagingData.KEY_NAME, NAME_A);
            valuesA.put(RichMessagingData.KEY_DATA, i);
            contentResolver.insert(RichMessagingData.CONTENT_URI, valuesA);

            ContentValues valuesB = new ContentValues();
            valuesB.put(RichMessagingData.KEY_NAME, NAME_B);
            valuesB.put(RichMessagingData.KEY_DATA, i);
            contentResolver.insert(RichMessagingData.CONTENT_URI, valuesB);
        }
        Cursor cursor = contentResolver.query(RichMessagingData.CONTENT_URI,
                null, null, null, null);
        assertTrue(cursor != null);
        assertTrue(cursor.getCount() == LOOP_COUNT * 2);
        // chear the history
        modelInstance.clearAllChatHistory();
        cursor = contentResolver.query(RichMessagingData.CONTENT_URI, null,
                null, null, null);
        assertTrue(cursor != null);
        assertTrue(cursor.getCount() == 0);
    }

    /**
     * This test case used for file transfer listener
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchMethodException
     * @throws SecurityException
     * @throws InvocationTargetException
     * @throws InstantiationException
     * @throws InterruptedException
     */
    public void testCase5_fileTransferListener() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException,
            InterruptedException {
        Logger.d(TAG, "testCase5_fileTransferListener() entry");
        MockChatWindow mockChatWindow = new MockChatWindow();
        ModelImpl.SentFileTransfer sentFileTransfer = new ModelImpl.SentFileTransfer(
                ParcelUuid.fromString(UUID.randomUUID().toString()),
                mockChatWindow, "/sdcard/test.txt", new Participant(
                        CONTACT_NUMBER1, CONTACT_NUMBER1),
                ParcelUuid.fromString(UUID.randomUUID().toString()));
        Class<?>[] classes = sentFileTransfer.getClass().getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : classes) {
            if ("FileTransferSenderListener".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    // find empty constructor
                    if (paras.length == 1) {
                        ctr = ctrz;
                    }
                }
                break;
            } else {
                Log.d(TAG, "non FileTransferSenderListener");
            }
        }
        ctr.setAccessible(true);
        // Use empty constructor
        Object object = ctr.newInstance(sentFileTransfer);
        handleSessionStarted(object);
        handleTransferProgress(object, mockChatWindow);
        handleFileTransfered(object, mockChatWindow);
        handleSessionAborted(object, sentFileTransfer);
        handleSessionTerminatedByRemote(object, sentFileTransfer);
        handleTransferError(object, mockChatWindow);
        send(sentFileTransfer, object);
    }

    @SuppressWarnings("unchecked")
    /**
     * Test cancelFileTransfer
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase6_cancelFileTransfer() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase6_cancelFileTransfer() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        assertNotNull(modelImpl);
        Field field = Utils.getPrivateField(modelImpl.getClass(),
                "mOutGoingFileTransferManager");
        assertNotNull(field);
        Object outGoingFileTransferManager = field.get(modelImpl);
        assertNotNull(outGoingFileTransferManager);
        Logger.d(TAG, "outGoingFileTransferManager class name:"
                + outGoingFileTransferManager.getClass().getName());
        Field fieldPendingList = Utils.getPrivateField(
                outGoingFileTransferManager.getClass(), "mPendingList");
        assertNotNull(fieldPendingList);
        ConcurrentLinkedQueue<SentFileTransfer> pendingList = (ConcurrentLinkedQueue<ModelImpl.SentFileTransfer>) fieldPendingList
                .get(outGoingFileTransferManager);
        int pendingSizeBefore = pendingList.size();
        MockChatWindow mockChatWindow = new MockChatWindow();
        ParcelUuid ftTag1 = ParcelUuid.fromString(UUID.randomUUID().toString());
        ModelImpl.SentFileTransfer sentFileTransfer = new ModelImpl.SentFileTransfer(
                ParcelUuid.fromString(UUID.randomUUID().toString()),
                mockChatWindow, "/sdcard/test.txt", new Participant(
                        CONTACT_NUMBER1, CONTACT_NUMBER1), ftTag1);
        pendingList.add(sentFileTransfer);
        assertTrue("pendingList.size():" + pendingList.size(),
                pendingList.size() == pendingSizeBefore + 1);

        Field fieldActiveList = Utils.getPrivateField(
                outGoingFileTransferManager.getClass(), "mActiveList");
        CopyOnWriteArrayList<SentFileTransfer> activeList = (CopyOnWriteArrayList<ModelImpl.SentFileTransfer>) fieldActiveList
                .get(outGoingFileTransferManager);
        int sizeBefore = activeList.size();
        assertNotNull(fieldActiveList);
        MockChatWindow mockChatWindow2 = new MockChatWindow();
        ParcelUuid ftTag2 = ParcelUuid.fromString(UUID.randomUUID().toString());
        ModelImpl.SentFileTransfer sentFileTransfer2 = new ModelImpl.SentFileTransfer(
                ParcelUuid.fromString(UUID.randomUUID().toString()),
                mockChatWindow2, "/sdcard/test2.txt", new Participant(
                        CONTACT_NUMBER1, CONTACT_NUMBER1), ftTag2);
        activeList.add(sentFileTransfer2);
        assertTrue("activeList.size():" + activeList.size(),
                activeList.size() == sizeBefore + 1);
        // get cancelFileTransfer
        Method methodCancelFt = outGoingFileTransferManager.getClass()
                .getMethod("cancelFileTransfer", Object.class);
        methodCancelFt.invoke(outGoingFileTransferManager, ftTag1);
        assertTrue("pendingList.size():" + pendingList.size(),
                pendingList.size() == pendingSizeBefore);
        methodCancelFt.invoke(outGoingFileTransferManager, ftTag2);
        assertTrue("activeList.size():" + activeList.size(),
                activeList.size() == sizeBefore);
    }

    @SuppressWarnings("unchecked")
    /**
     * Test removeMessages and removeMessage
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InstantiationException
     */
    public void testCase7_removeMessage() throws SecurityException,
            NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InstantiationException {
        Logger.d(TAG, "testCase7_removeMessage() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, "");
        One2OneChat oneoneChat = new One2OneChat((ModelImpl) modelImpl,
                new MockChatWindow(), participant, parcelUuid);
        List<IChatMessage> messagList = oneoneChat.listAllChatMessages();
        assertNotNull(messagList);
        Constructor<?> ctr = null;
        Class<?>[] classes = modelImpl.getClass().getDeclaredClasses();
        for (Class<?> classz : classes) {
            if ("ChatMessage".equals(classz.getSimpleName())) {
                Logger.d(TAG, "find class ChatMessage");
                ctr = classz.getConstructor(InstantMessage.class);
            }
        }
        assertNotNull(ctr);
        ctr.newInstance(new InstantMessage("123456", "tel:+34200000250",
                "test", true));
        messagList.add((IChatMessage) ctr.newInstance(new InstantMessage(
                "123456", "tel:+34200000250", "test0", true)));
        messagList.add((IChatMessage) ctr.newInstance(new InstantMessage(
                "123457", "tel:+34200000250", "test1", true)));
        messagList.add((IChatMessage) ctr.newInstance(new InstantMessage(
                "123458", "tel:+34200000250", "test2", true)));
        assertEquals(oneoneChat.getChatMessageCount(), 3);

        assertEquals(false, oneoneChat.removeMessages(-1, 5));
        assertEquals(false, oneoneChat.removeMessages(5, 5));
        assertEquals(true, oneoneChat.removeMessages(0, 3));

        assertEquals(false, oneoneChat.removeMessage(-1));
        assertEquals(true, oneoneChat.removeMessage(0));
        assertEquals(2, oneoneChat.getChatMessageCount());
        assertEquals(true, oneoneChat.removeMessage(0));
        assertEquals(1, oneoneChat.getChatMessageCount());
        assertEquals(true, oneoneChat.removeMessage(0));
        assertEquals(0, oneoneChat.getChatMessageCount());
    }

    @SuppressWarnings("unchecked")
    /**
     * Test markUnreadMessageDisplayed
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     */
    public void testCase8_markUnreadMessageDisplayed()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException,
            InvocationTargetException {
        Logger.d(TAG, "testCase8_markUnreadMessageDisplayed() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, "");
        One2OneChat oneoneChat = new One2OneChat((ModelImpl) modelImpl,
                new MockChatWindow(), participant, parcelUuid);
        Field fieldToBeDisplayed = Utils.getPrivateField(oneoneChat.getClass()
                .getSuperclass(), "mReceivedInBackgroundToBeDisplayed");
        List<InstantMessage> toBeDisplayed = (List<InstantMessage>) fieldToBeDisplayed
                .get(oneoneChat);
        toBeDisplayed.add(new InstantMessage(null, null, CONTENT_SEND_MESSAGE,
                true, null));
        Field fieldToBeRead = Utils.getPrivateField(oneoneChat.getClass()
                .getSuperclass(), "mReceivedInBackgroundToBeRead");
        List<InstantMessage> toBeRead = (List<InstantMessage>) fieldToBeRead
                .get(oneoneChat);
        toBeRead.add(new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true,
                null));
        assertEquals(1, toBeDisplayed.size());
        assertEquals(1, toBeRead.size());
        Method methodMarkUnreadMessageDisplayed = Utils.getPrivateMethod(
                oneoneChat.getClass().getSuperclass(),
                "markUnreadMessageDisplayed");
        methodMarkUnreadMessageDisplayed.invoke(oneoneChat);
        assertEquals(0, toBeDisplayed.size());
        assertEquals(0, toBeRead.size());
    }

    @SuppressWarnings("unchecked")
    /**
     * Test handleCancelFileTransfer
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InstantiationException
     * @throws InvocationTargetException
     * @throws RemoteException
     */
    public void testCase9_handleCancelFileTransfer()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, SecurityException, NoSuchMethodException,
            InstantiationException, InvocationTargetException, RemoteException {
        Logger.d(TAG, "testCase9_handleCancelFileTransfer() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        // First use a non string file transfer tag, this is the same as test
        // case6
        ParcelUuid ftTag = ParcelUuid.fromString(UUID.randomUUID().toString());
        modelImpl.handleCancelFileTransfer(null, ftTag);

        // Then use a string tag but chat tag is null
        String fileTransferTag = "testFT";
        modelImpl.handleCancelFileTransfer(null, fileTransferTag);

        // Now use a string tag but chat tag is not null
        ParcelUuid chatTag = ParcelUuid
                .fromString(UUID.randomUUID().toString());
        List<Participant> participants = new ArrayList<Participant>();
        participants.add(new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1));
        One2OneChat oneoneChat = (One2OneChat) modelImpl.addChat(participants,
                chatTag, null);
        // Add a recievied file transfer to oneoneChat
        Field fieldFtManager = Utils.getPrivateField(oneoneChat.getClass(),
                "mReceiveFileTransferManager");
        Object ftManager = fieldFtManager.get(oneoneChat);

        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(),
                "mActiveList");
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);
        int size = activeList.size();

        // Query One2OneChat inner class and create ReceiveFileTransfer object
        Class<?>[] calsses = oneoneChat.getClass().getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : calsses) {
            if ("ReceiveFileTransfer".equals(classz.getSimpleName())) {
                ctr = classz.getConstructor(One2OneChat.class,
                        IFileTransferSession.class);
                break;
            }
        }
        assertNotNull(ctr);
        MockFtSession ftSession = new MockFtSession();
        Object receivedFt = ctr.newInstance(oneoneChat, ftSession);
        activeList.add(receivedFt);
        assertEquals(size + 1, activeList.size());
        modelImpl.handleCancelFileTransfer(chatTag, ftSession.getSessionID());
        assertEquals(size, activeList.size());
    }

    @SuppressWarnings("unchecked")
    /**
     * Test handleMessageDeliveryStatus
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     */
    public void testCase10_handleMessageDeliveryStatus()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException,
            InvocationTargetException {
        Logger.d(TAG, "testCase10_handleMessageDeliveryStatus() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        ParcelUuid chatTag = ParcelUuid
                .fromString(UUID.randomUUID().toString());
        List<Participant> participants = new ArrayList<Participant>();
        participants.add(new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1));
        One2OneChat oneoneChat = (One2OneChat) modelImpl.addChat(participants,
                chatTag, null);
        // Add a recievied file transfer to oneoneChat
        Field fieldSentMessageManager = Utils.getPrivateField(
                oneoneChat.getClass(), "mSentMessageManager");
        // SentMessageManager
        Object sentMessageManager = fieldSentMessageManager.get(oneoneChat);
        assertNotNull(fieldSentMessageManager);
        Method methodonMessageSent = Utils.getPrivateMethod(
                sentMessageManager.getClass(), "onMessageSent",
                ISentChatMessage.class);
        String messageId = "123456";
        InstantMessage message = new InstantMessage(messageId, null,
                CONTENT_SEND_MESSAGE, true, null);
        int messageTag = 123456;
        SentChatMessage sentChatMessage = new SentChatMessage(message,
                messageTag);
        methodonMessageSent.invoke(sentMessageManager, sentChatMessage);
        Field fieldSendingMessage = Utils.getPrivateField(
                sentMessageManager.getClass(), "mSendingMessage");
        Map<String, Object> sendingMessageMap = (Map<String, Object>) fieldSendingMessage
                .get(sentMessageManager);
        assertTrue(sendingMessageMap.containsKey(messageId));
        modelImpl.handleMessageDeliveryStatus(CONTACT_NUMBER1, messageId,
                ImdnDocument.DELIVERY_STATUS_DELIVERED,
                System.currentTimeMillis());
        // Wait thread run and check the result
        waitonMessageDeliveredFinshed(sendingMessageMap, messageId);
    }

    private void waitonMessageDeliveredFinshed(
            Map<String, Object> sendingMessageMap, String messageId) {
        Logger.d(TAG,
                "waitonMessageDeliveredFinshed() entry. sendingMessageMap = "
                        + sendingMessageMap);
        long beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (!sendingMessageMap.containsKey(messageId)) {
                break;
            }
        }
        assertFalse(sendingMessageMap.containsKey(messageId));
        Logger.d(TAG, "waitonMessageDeliveredFinshed() exit.");
    }

    /**
     * Test case for markMessageAsDisplayed()
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase11_markMessageAsDisplayed()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase11_markMessageAsDisplayed() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        MockChat chat = new MockChat(modelInstance, tag);
        PreferenceManager
                .getDefaultSharedPreferences(
                        getInstrumentation().getTargetContext()).edit()
                .putBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true)
                .commit();
        InstantMessage msg = new InstantMessage(null, null,
                CONTENT_SEND_MESSAGE, true);
        MockOneOneSession oneonesession = new MockOneOneSession();
        oneonesession.setIsStoreAndForward(true);
        AtomicReference<IChatSession> session = new AtomicReference<IChatSession>();
        Method displayMessage = Utils.getPrivateMethod(ChatImpl.class,
                METHOD_DISPLAY, InstantMessage.class);
        Field sessionField = Utils.getPrivateField(ChatImpl.class,
                FIELD_CURRENT_SESSSION);
        session.set(null);
        sessionField.set(chat, session);
        displayMessage.invoke(chat, msg);

        session.set(oneonesession);
        sessionField.set(chat, session);
        displayMessage.invoke(chat, msg);
        assertTrue(oneonesession.isDisplayed());

        oneonesession.setIsStoreAndForward(false);
        displayMessage.invoke(chat, msg);
        assertTrue(oneonesession.isDisplayed());
    }

    /**
     * Test case for terminateSession()
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase12_terminateSession() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase12_terminateSession() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        MockChat chat = new MockChat(modelInstance, tag);
        MockOneOneSession oneonesession = new MockOneOneSession();
        AtomicReference<IChatSession> session = new AtomicReference<IChatSession>();
        Method methodTerminateSession = Utils.getPrivateMethod(ChatImpl.class,
                METHOD_TERMINATE_SESSION);
        Field sessionField = Utils.getPrivateField(ChatImpl.class,
                FIELD_CURRENT_SESSSION);
        session.set(oneonesession);
        sessionField.set(chat, session);
        methodTerminateSession.invoke(chat);
        assertTrue(oneonesession.isCanceled());
    }

    /**
     * Test case for removeChat()
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    @SuppressWarnings("unchecked")
    public void testCase13_removeChat() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase13_removeChat() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        chatMap.clear();

        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);

        Method methodRemoveChat = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_REMOVE_CHAT, Object.class);
        boolean result = (Boolean) methodRemoveChat.invoke(modelInstance,
                (Object) null);
        assertFalse(result);

        result = (Boolean) methodRemoveChat.invoke(modelInstance, tag);
        assertFalse(result);

        MockChat chat = new MockChat(modelInstance, tag);
        chatMap.put(tag, chat);

        Field fieldFtManager = Utils.getPrivateField(ModelImpl.class,
                FIELD_OUTGOING_FTM);
        Object ftManager = fieldFtManager.get(modelInstance);

        ModelImpl.SentFileTransfer sentFileTransfer = new ModelImpl.SentFileTransfer(
                tag, new MockChatWindow(), "/sdcard/test.txt", new Participant(
                        CONTACT_NUMBER1, CONTACT_NUMBER1), tag);
        Field fieldActiveList = Utils.getPrivateField(ftManager.getClass(),
                FIELD_ACTIVE_LIST);
        CopyOnWriteArrayList<Object> activeList = (CopyOnWriteArrayList<Object>) fieldActiveList
                .get(ftManager);
        activeList.add(sentFileTransfer);

        Field fieldPendingList = Utils.getPrivateField(ftManager.getClass(),
                FIELD_PENDING_LIST);
        ConcurrentLinkedQueue<Object> pendingList = (ConcurrentLinkedQueue<Object>) fieldPendingList
                .get(ftManager);
        pendingList.add(sentFileTransfer);

        Field fieldResendList = Utils.getPrivateField(ftManager.getClass(),
                FIELD_RESENDABLE_LIST);
        CopyOnWriteArrayList<Object> resendList = (CopyOnWriteArrayList<Object>) fieldResendList
                .get(ftManager);
        resendList.add(sentFileTransfer);

        result = (Boolean) methodRemoveChat.invoke(modelInstance, tag);
        assertTrue(result);

        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        One2OneChat oneoneChat1 = new One2OneChat(modelInstance,
                new MockChatWindow(), participant1, tag);
        One2OneChat oneoneChat2 = new One2OneChat(modelInstance,
                new MockChatWindow(), participant2, tag);
        Method methodRemoveChat2 = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_REMOVE_CHAT, ChatImpl.class);

        chatMap.clear();
        result = (Boolean) methodRemoveChat2.invoke(modelInstance, oneoneChat1);
        assertFalse(result);

        chatMap.put(tag, oneoneChat1);
        result = (Boolean) methodRemoveChat2.invoke(modelInstance, oneoneChat2);
        assertFalse(result);

        result = (Boolean) methodRemoveChat2.invoke(modelInstance, oneoneChat1);
        assertTrue(result);
        // sentFileTransfer.
    }

    /**
     * Test case for removeChatByContact()
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase14_removeChatByContact() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase14_removeChatByContact() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField
                .get(modelInstance);
        Participant participant = new Participant(CONTACT_NUMBER1, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        One2OneChat oneoneChat = new One2OneChat(modelInstance,
                new MockChatWindow(), participant, tag);
        chatMap.put(tag, oneoneChat);
        Method methodRemoveChat = Utils.getPrivateMethod(ModelImpl.class,
                METHOD_REMOVECHAT_BYCONTACT, Participant.class);

        methodRemoveChat.invoke(modelInstance, new Participant(CONTACT_NUMBER2,
                ""));
        assertTrue(chatMap.containsValue(oneoneChat));

        methodRemoveChat.invoke(modelInstance, participant);
        assertFalse(chatMap.containsValue(oneoneChat));
    }

    /**
     * Test switchGroupChat
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InstantiationException
     * @throws NoSuchFieldException
     */
    public void testCase15_switchGroupChat() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InstantiationException,
            NoSuchFieldException {
        Logger.d(TAG, "testCase15_switchGroupChat() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Method methodswitchGroupChat = Utils.getPrivateMethod(
                modelInstance.getClass(), "switchGroupChat", IChat.class);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        MockGroupChatWindow chatWindow = new MockGroupChatWindow();

        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid1);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);
        list.add(participant3);
        list.add(participant4);
        MockGroupChat chat = new MockGroupChat(modelInstance, chatWindow,
                participants, tag);
        methodswitchGroupChat.invoke(modelInstance, chat);

        // test getSentChatMessage
        assertNull(chat.getSentChatMessage(-1));
        List<IChatMessage> messageList = chat.listAllChatMessages();
        if (messageList != null) {
            InstantMessage instantMessage = new InstantMessage("test",
                    CONTACT_NUMBER1, "test", true);
            MockChatMessage message = new MockChatMessage(instantMessage);
            messageList.add(message);
            assertEquals(message, chat.getSentChatMessage(0));
        }

        assertNull(ModelImpl.SentFileTransfer.extractFileNameFromPath(null));
        assertNull(ModelImpl.SentFileTransfer.extractFileNameFromPath("test"));

        Class<?>[] clazzes = modelInstance.getClass().getDeclaredClasses();
        for (Class<?> clazz : clazzes) {
            Logger.d(TAG, "inner class:" + clazz.getSimpleName());
            if ("ReloadMessageInfo".equals(clazz.getSimpleName())) {
                Constructor<?> ctor = clazz.getDeclaredConstructor(
                        Object.class, int.class);
                Object object = new String();
                int id = 1;
                ctor.newInstance(object, id);
                break;
            }
        }
        String chatId = "chatId";
        Field fieldChatId = Utils.getPrivateField(chat.getClass()
                .getSuperclass(), "mChatId");
        fieldChatId.set(chat, chatId);
        Field fieldChatMap = Utils.getPrivateField(modelInstance.getClass(),
                "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap
                .get(modelInstance);
        chatMap.put(tag, chat);
        assertEquals(chat, modelInstance.getGroupChat(chatId));
    }

    /**
     * Test quitGroupChat
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws NoSuchMethodException
     */
    @SuppressWarnings("unchecked")
    public void testCase16_quitGroupChat() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            NoSuchMethodException {
        Logger.d(TAG, "testCase16_quitGroupChat() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        assertFalse(modelInstance.quitGroupChat(null));
        Field fieldChatMap = Utils.getPrivateField(modelInstance.getClass(),
                "mChatMap");
        Map<Object, IChat> chatMap = (Map<Object, IChat>) fieldChatMap
                .get(modelInstance);
        chatMap.clear();
        assertFalse(modelInstance.quitGroupChat("test"));
        modelInstance.handleFileTransferNotAvailable(null, 0);

        // Test addUnreadMessage
        ArrayList<Participant> participants = new ArrayList<Participant>();
        MockGroupChatWindow chatWindow = new MockGroupChatWindow();

        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid1);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);
        list.add(participant3);
        list.add(participant4);
        MockGroupChat groupChat = new MockGroupChat(modelInstance, chatWindow,
                participants, tag);
        InstantMessage message = new InstantMessage("test", CONTACT_NUMBER4,
                "test", true, new Date());
        groupChat.addUnreadMessage(message);
        message = new InstantMessage("test", CONTACT_NUMBER4, "test", false,
                new Date());
        groupChat.addUnreadMessage(message);
        groupChat.getUnreadMessages();
    }

    /**
     * Test handleSendFileTransferInvitation
     */
    public void testCase17_handleSendFileTransferInvitation() {
        Logger.d(TAG, "testCase17_handleSendFileTransferInvitation() entry");
        ModelImpl modelImpl = (ModelImpl) ModelImpl.getInstance();
        modelImpl.handleSendFileTransferInvitation(CONTACT_NUMBER4,
                "/sdcard/test.txt", null);
    }

    /**
     * Tetst reloadGroupMessage
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InvocationTargetException
     */
    @SuppressWarnings("unchecked")
    public void testCase18_reloadGroupMessage() throws NoSuchMethodException,
            NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase18_reloadGroupMessage() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Method methodreloadGroupMessage = Utils.getPrivateMethod(
                modelInstance.getClass(), "reloadGroupMessage", String.class,
                List.class, ContentResolver.class);

        MockContentResolver resolver = new MockContentResolver();
        // Should set a conetxt for resolver
        Field fieldmContext = Utils.getPrivateField(resolver.getClass()
                .getSuperclass(), "mContext");
        fieldmContext.set(resolver, getInstrumentation().getTargetContext());
        MockRichMessagingProvider provider = new MockRichMessagingProvider();
        resolver.addProvider(RichMessagingData.CONTENT_URI.getAuthority(),
                provider);

        UUID uuid = UUID.randomUUID();
        String tag = uuid.toString();
        ArrayList<Integer> messageIds = new ArrayList<Integer>();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);

        Field fieldmChatMap = Utils.getPrivateField(modelInstance.getClass(),
                "mChatMap");
        Map<Object, IChat> mChatMap = (Map<Object, IChat>) fieldmChatMap
                .get(modelInstance);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        Participant participant3 = new Participant(CONTACT_NUMBER3, "");
        Participant participant4 = new Participant(CONTACT_NUMBER4, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);
        list.add(participant3);
        list.add(participant4);
        MockGroupChatWindow chatWindow = new MockGroupChatWindow();
        IChat chat = new MockGroupChat(modelInstance, chatWindow, list, parcelUuid);
        mChatMap.put(parcelUuid, chat);
        methodreloadGroupMessage.invoke(modelInstance, tag, messageIds,
                resolver);

    }

    /**
     * Test fillParticipantList
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     */
    public void testCase19_fillParticipantList() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException {
        Logger.d(TAG, "testCase19_fillParticipantList() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Method methodreloadGroupMessage = Utils.getPrivateMethod(
                modelInstance.getClass(), "fillParticipantList", TreeSet.class,
                HashSet.class, ContentResolver.class);
        MockContentResolver resolver = new MockContentResolver();
        // Should set a conetxt for resolver
        Field fieldmContext = Utils.getPrivateField(resolver.getClass()
                .getSuperclass(), "mContext");
        Logger.d(TAG, "resolver's context " + fieldmContext.get(resolver));
        fieldmContext.set(resolver, getInstrumentation().getTargetContext());
        Logger.d(TAG, "resolver's context " + fieldmContext.get(resolver));
        MockRichMessagingProvider provider = new MockRichMessagingProvider();
        fieldmContext = Utils.getPrivateField(provider.getClass()
                .getSuperclass().getSuperclass(), "mContext");
        Logger.d(TAG, "provider's context " + fieldmContext.get(provider));
        fieldmContext.set(provider, getInstrumentation().getTargetContext());
        Logger.d(TAG, "provider's context " + fieldmContext.get(provider));

        Field fieldmExported = Utils.getPrivateField(provider.getClass()
                .getSuperclass().getSuperclass(), "mExported");
        Logger.d(TAG, "provider's mExported " + fieldmExported.get(provider));
        fieldmExported.set(provider, true);
        Logger.d(TAG, "provider's mExported " + fieldmExported.get(provider));

        resolver.addProvider(RichMessagingData.CONTENT_URI.getAuthority(),
                provider);
        TreeSet<Long> sessionIdList = new TreeSet<Long>();
        HashSet<Participant> participantList = new HashSet<Participant>();
        Long id = 123456L;
        sessionIdList.add(id);
        methodreloadGroupMessage.invoke(modelInstance, sessionIdList,
                participantList, resolver);
        assertTrue(participantList.size() > 0);
    }

    /**
     * Test send
     * 
     * @param sentFileTransfer
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    private void send(SentFileTransfer sentFileTransfer,
            Object fileTransferSenderListener) throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InterruptedException,
            NoSuchFieldException {
        Logger.d(TAG, "send(): sentFileTransfer = " + sentFileTransfer
                + ", fileTransferSenderListener = "
                + fileTransferSenderListener);
        Method methodsend = Utils.getPrivateMethod(sentFileTransfer.getClass(),
                "send");
        ApiManager.initialize(getInstrumentation().getTargetContext());
        waitMessagingApiConnected();
        methodsend.invoke(sentFileTransfer);

        // test onNotAvailable
        Method methodonNotAvailable = Utils.getPrivateMethod(
                sentFileTransfer.getClass(), "onNotAvailable", int.class);
        methodonNotAvailable.invoke(sentFileTransfer,
                One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);
        methodonNotAvailable.invoke(sentFileTransfer,
                One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
        methodonNotAvailable.invoke(sentFileTransfer,
                One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
        methodonNotAvailable.invoke(sentFileTransfer, 200);

        // test onDestroy
        Method methodonDestroy = Utils.getPrivateMethod(
                sentFileTransfer.getClass(), "onDestroy");
        // check whether mFileTransferListener is null
        Field fieldmFileTransferListener = Utils.getPrivateField(
                sentFileTransfer.getClass(), "mFileTransferListener");
        if (fieldmFileTransferListener.get(sentFileTransfer) == null) {
            Logger.d(TAG, "mFileTransferListener is null, then set it");
            fieldmFileTransferListener.set(sentFileTransfer,
                    fileTransferSenderListener);
        }
        methodonDestroy.invoke(sentFileTransfer);

        Method methodsetNotification = Utils.getPrivateMethod(
                sentFileTransfer.getClass(), "setNotification");
        boolean isIntegerationMode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        methodsetNotification.invoke(sentFileTransfer);
        Logger.setIsIntegrationMode(isIntegerationMode);

        Method methodbuildPercentageLabel = Utils.getPrivateMethod(
                sentFileTransfer.getClass(), "buildPercentageLabel",
                Context.class, long.class, long.class);
        methodbuildPercentageLabel.invoke(sentFileTransfer,
                getInstrumentation().getTargetContext(), 100, 50);
    }

    private void waitMessagingApiConnected() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "waitMessagingApiConnected()");
        Field fieldsInstance = Utils.getPrivateField(ApiManager.class,
                "sInstance");
        fieldsInstance.set(null, null);
        ApiManager.initialize(getInstrumentation().getTargetContext());
        long beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) > SLEEP_TIME) {
            if (ApiManager.getInstance().getMessagingApi() != null) {
                Logger.d(TAG, "waitMessagingApiConnected(): connected");
                break;
            }
        }
        if (ApiManager.getInstance().getMessagingApi() == null) {
            MessagingApi messagingApi = new MessagingApi(getInstrumentation()
                    .getTargetContext());
            Field fieldmMessagingApi = Utils.getPrivateField(ApiManager
                    .getInstance().getClass(), "mMessagingApi");
            fieldmMessagingApi.set(ApiManager.getInstance(), messagingApi);
        }
    }

    private void handleSessionStarted(Object fileTransferListener)
            throws SecurityException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        callVoidMethod(fileTransferListener, "handleSessionStarted");
    }

    private void handleSessionAborted(Object fileTransferListener, Object parent)
            throws SecurityException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException {
        Field filed = Utils.getPrivateField(parent.getClass(),
                "mFileTransferSession");
        assertNotNull(filed);
        filed.set(parent, new MockFtSession());
        assertNotNull(filed.get(parent));
        callVoidMethod(fileTransferListener, "handleSessionAborted");
        assertNull(filed.get(parent));
    }

    private void handleSessionTerminatedByRemote(Object fileTransferListener,
            Object parent) throws SecurityException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException {
        Field filed = Utils.getPrivateField(parent.getClass(),
                "mFileTransferSession");
        assertNotNull(filed);
        filed.set(parent, new MockFtSession());
        assertNotNull(filed.get(parent));
        callVoidMethod(fileTransferListener, "handleSessionTerminatedByRemote");
        assertNull(filed.get(parent));
    }

    private void handleTransferProgress(Object fileTransferListener,
            MockChatWindow mockChatWindow) throws SecurityException,
            NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Method methodHandleTransferProgress = fileTransferListener.getClass()
                .getDeclaredMethod("handleTransferProgress", long.class,
                        long.class);
        assertNotNull(methodHandleTransferProgress);
        methodHandleTransferProgress.setAccessible(true);
        methodHandleTransferProgress.invoke(fileTransferListener, 102, 1024);
        assertEquals(mockChatWindow.mSentFileTransfer.getProgress(), 102);
        methodHandleTransferProgress.invoke(fileTransferListener, 1024, 1024);
        assertEquals(mockChatWindow.mSentFileTransfer.getStatue(),
                IFileTransfer.Status.FINISHED);
    }

    private void handleTransferError(Object fileTransferListener,
            MockChatWindow mockChatWindow) throws SecurityException,
            NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Method methodHandleTransferProgress = fileTransferListener.getClass()
                .getDeclaredMethod("handleTransferError", int.class);
        assertNotNull(methodHandleTransferProgress);
        methodHandleTransferProgress.setAccessible(true);
        // Because handleTransferError do work in a background thread, wait a
        // moment for the result

        mockChatWindow.mSentFileTransfer.setStatus(Status.TRANSFERING);
        methodHandleTransferProgress.invoke(fileTransferListener,
                FileSharingError.SESSION_INITIATION_FAILED);
        waitHandleErrorDone(mockChatWindow.mSentFileTransfer,
                ChatView.IFileTransfer.Status.FAILED);
        assertEquals(mockChatWindow.mSentFileTransfer.getStatue(),
                ChatView.IFileTransfer.Status.FAILED);

        mockChatWindow.mSentFileTransfer.setStatus(Status.TRANSFERING);
        methodHandleTransferProgress.invoke(fileTransferListener,
                FileSharingError.SESSION_INITIATION_DECLINED);
        waitHandleErrorDone(mockChatWindow.mSentFileTransfer,
                ChatView.IFileTransfer.Status.REJECTED);
        assertEquals(mockChatWindow.mSentFileTransfer.getStatue(),
                ChatView.IFileTransfer.Status.REJECTED);

        mockChatWindow.mSentFileTransfer.setStatus(Status.TRANSFERING);
        methodHandleTransferProgress.invoke(fileTransferListener,
                FileSharingError.SESSION_INITIATION_CANCELLED);

    }

    private void handleFileTransfered(Object fileTransferListener,
            MockChatWindow mockChatWindow) throws SecurityException,
            NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Method methodHandleTransferProgress = fileTransferListener.getClass()
                .getDeclaredMethod("handleFileTransfered", String.class);
        assertNotNull(methodHandleTransferProgress);
        methodHandleTransferProgress.setAccessible(true);
        methodHandleTransferProgress.invoke(fileTransferListener, "/test.jpg");
        assertEquals(mockChatWindow.mSentFileTransfer.getStatue(),
                IFileTransfer.Status.FINISHED);
    }

    private void callVoidMethod(Object fileTransferListener, String methodName)
            throws SecurityException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Method methodHandleSessionAborted = fileTransferListener.getClass()
                .getDeclaredMethod(methodName);
        assertNotNull(methodHandleSessionAborted);
        methodHandleSessionAborted.setAccessible(true);
        methodHandleSessionAborted.invoke(fileTransferListener);
    }

    private void waitHandleErrorDone(
            One2OneChatFragment.SentFileTransfer fileTransfer, Status expect) {
        Logger.d(TAG, "waitHandleErrorDone() entry. expect = " + expect);
        long beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (expect == fileTransfer.getStatue()) {
                break;
            }
        }
        Logger.d(
                TAG,
                "waitHandleErrorDone() exit. actual = "
                        + fileTransfer.getStatue());
    }

    private Intent newChatInvitationIntent(String sessionId) {
        Logger.d(TAG, "newChatInvitationIntent() entry");
        Intent intent = new Intent();
        intent.putExtra("sessionId", sessionId);
        ArrayList<InstantMessage> messages = new ArrayList<InstantMessage>();
        messages.add(new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true,
                null));
        intent.putParcelableArrayListExtra(INTENT_MESSAGE, messages);
        intent.putExtra(RcsNotification.CONTACT, CONTACT_NUMBER1);
        return intent;
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
        public IChatSession initiateAdhocGroupChatSession(
                List<String> participants, String firstMsg,String x)
                throws ClientApiException {
            Logger.d(TAG, "initiateAdhocGroupChatSession() entry");
            return new MockImSession();
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            if (SESSION_ID_GROUP.equals(id)) {
                return new MockImSession();
            } else if (SESSION_ID_ONEONE.equals(id)) {
                return new MockOneOneSession();
            }
            return super.getChatSession(id);
        }

        @Override
        public IFileTransferSession getFileTransferSession(String id)
                throws ClientApiException {
            return new MockFtSession();
        }

        @Override
        public void setMessageDeliveryStatus(String contact, String msgId,
                String status) throws ClientApiException {
            mDisplayed = true;
        }
    }

    private class MockOneOneSession extends MockImSession {
        @Override
        public List<String> getInivtedParticipants() {
            ArrayList<String> numList = new ArrayList<String>();
            numList.add(CONTACT_NUMBER1);
            return numList;
        }
    }

    private class MockImSession extends IChatSession.Stub {
        private boolean mIsStoreAndForward = false;
        private boolean mDisplayed = false;
        private boolean mCanceled = false;

        public void setIsStoreAndForward(boolean storeAndForward) {
            mIsStoreAndForward = storeAndForward;
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }

        public boolean isDisplayed() {
            return mDisplayed;
        }

        public boolean isCanceled() {
            return mCanceled;
        }

        @Override
        public boolean isInComing() {
            return true;
        }

        public String getSessionID() {
            return null;
        }

        public String getChatID() {
            return null;
        }

        public String getRemoteContact() {
            return null;
        }

        public int getSessionState() {
            return 0;
        }

        @Override
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        public boolean isGroupChat() {
            return true;
        }

        public boolean isStoreAndForward() {
            return mIsStoreAndForward;
        }

        public InstantMessage getFirstMessage() {
            return new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true,
                    null);
        }

        public void acceptSession() {

        }

        public void rejectSession() {

        }

        public void cancelSession() {
            mCanceled = true;
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
            mDisplayed = true;
        }

        public void addSessionListener(IChatEventListener listener) {
        }

        public void removeSessionListener(IChatEventListener listener) {

        }

        public void setMessageDisplayedStatusBySipMessage(String contact,
                String msgId, String status) {
            mDisplayed = true;
        }

        public String getReferredByHeader() {
            return null;
        }

        public List<String> getInivtedParticipants() {
            ArrayList<String> numList = new ArrayList<String>();
            numList.add(CONTACT_NUMBER1);
            numList.add(CONTACT_NUMBER2);
            numList.add(CONTACT_NUMBER3);
            return numList;
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

    /**
     * Mock chat window for test
     */
    private class MockChatWindow implements IOne2OneChatWindow {
        private One2OneChatFragment mOne2OneChatFragment = new One2OneChatFragment();
        private One2OneChatFragment.SentFileTransfer mSentFileTransfer = null;

        public void setFileTransferEnable(int reason) {
        }

        public void setIsComposing(boolean isComposing) {
        }

        public void setRemoteOfflineReminder(boolean isOffline) {
        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            UUID uuid = UUID.randomUUID();
            ParcelUuid parcelUuid = new ParcelUuid(uuid);
            Participant participant = new Participant(CONTACT_NUMBER1,
                    CONTACT_NUMBER1);
            final OneOneChatWindow oneChatWindow = new OneOneChatWindow(
                    parcelUuid, participant);
            mOne2OneChatFragment = oneChatWindow.getFragment();
            mSentFileTransfer = mOne2OneChatFragment.new SentFileTransfer(file);
            return mSentFileTransfer;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return null;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message,
                boolean isRead) {
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message,
                int messageTag) {
            return null;
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
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
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}
    }

    /**
     * Mock chat session that has a mock session-id , a mock file size and a
     * mock file name.
     */
    private class MockFtSession implements IFileTransferSession {
        private String mSessionId = SessionIdGenerator.getNewId();

        @Override
        public void acceptSession() throws RemoteException {
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }
        
        @Override
        public boolean isSessionPaused() {
        	return false;
        }

        @Override
        public void addSessionListener(IFileTransferEventListener listener)
                throws RemoteException {
        }

        @Override
        public void cancelSession() throws RemoteException {
        }

        @Override
        public String getFilename() throws RemoteException {
            return null;
        }

        @Override
        public long getFilesize() throws RemoteException {
            return CHAT_NUM1;
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

    /**
     * Mock chat.
     */
    public class MockChat extends ChatImpl {
        protected MockChat(ModelImpl modelImpl, Object tag) {
            modelImpl.super(tag);
        }

        @Override
        public void onCapabilityChanged(String contact,
                Capabilities capabilities) {

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

        protected void reloadMessage(final InstantMessage message,
                final int messageType, final int status) {

        }

        public void handleInvitation(IChatSession chatSession,
                ArrayList<IChatMessage> messages) {

        }

        public void sendMessage(String content, int messageTag) {

        }

		@Override
		public void handleInvitation(IChatSession chatSession,
				ArrayList<IChatMessage> messages, boolean isAutoAccept) {
			// TODO Auto-generated method stub
			
		}
    }

    /**
     * Mock group chat.
     */
    private class MockGroupChat extends GroupChat {
        private boolean mIsMessageSent = false;
        private boolean mReloaded = false;
        private boolean mIsEmpty = false;
        private boolean mNeedClearReceivedInBackgroundToBeDisplayed = false;

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

        protected void reloadMessage(final InstantMessage message,
                final int messageType, final int status) {
            super.reloadMessage(message, messageType, status);
            mReloaded = true;
        }

        public void hasTextChanged(boolean isEmpty) {
            mIsEmpty = isEmpty;
        }

        @Override
        protected void addUnreadMessage(InstantMessage message) {
            mTag = UUID.randomUUID();
            super.addUnreadMessage(message);
        }

        @Override
        public List<InstantMessage> getUnreadMessages() {
            if (mNeedClearReceivedInBackgroundToBeDisplayed) {
                mReceivedInBackgroundToBeDisplayed.clear();
            }
            return super.getUnreadMessages();
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

        public IChatEventInformation addChatEventInformation(
                ChatEventStruct chatEventStruct) {
            return null;
        }

        public IReceivedChatMessage addReceivedMessage(InstantMessage message,
                boolean isRead) {
            return null;
        }

        public ISentChatMessage addSentMessage(InstantMessage message,
                int messageTag) {
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

    private class MockChatMessage implements IChatMessage {
        private InstantMessage mInstantMessage;

        public MockChatMessage(InstantMessage instantMessage) {
            mInstantMessage = instantMessage;
        }

        @Override
        public InstantMessage getInstantMessage() {
            return mInstantMessage;
        }
    }

    /**
     * Mock Provider
     */
    private class MockRichMessagingProvider extends RichMessagingProvider {
        private static final String TAG = "MockRichMessagingProvider";

        @Override
        public Cursor query(Uri uri, String[] projectionIn, String selection,
                String[] selectionArgs, String sort) {
            Logger.d(TAG, "query");
            MockCursor cursor = new MockCursor();
            return cursor;
        }
    }

    /**
     * Mock Cursor
     */
    public class MockCursor implements Cursor {
        private boolean mIsAfterLast = false;
        private String mFirstMessageId = null;
        private int mType = 0;
        private int mPos = 0;

        @Override
        public int getCount() {
            return 10;
        }

        @Override
        public Uri getNotificationUri () {
					return null;
				}

        @Override
        public int getPosition() {
            return 0;
        }

        @Override
        public boolean move(int offset) {
            return false;
        }

        @Override
        public boolean moveToPosition(int position) {
            return false;
        }

        @Override
        public boolean moveToFirst() {
            return true;
        }

        @Override
        public boolean moveToLast() {
            return false;
        }

        @Override
        public boolean moveToNext() {
            if (++mPos > 10) {
                return false;
            }
            return true;
        }

        @Override
        public boolean moveToPrevious() {
            return false;
        }

        @Override
        public boolean isFirst() {
            return false;
        }

        @Override
        public boolean isLast() {
            return false;
        }

        @Override
        public boolean isBeforeFirst() {
            return false;
        }

        @Override
        public boolean isAfterLast() {
            return mIsAfterLast;
        }

        @Override
        public int getColumnIndex(String columnName) {
            if (RichMessagingData.KEY_CONTACT.equals(columnName)) {
                return 1;
            } else if ("message_id".equals(columnName)) {
                return 2;
            } else {
                return 0;
            }
        }

        @Override
        public int getColumnIndexOrThrow(String columnName)
                throws IllegalArgumentException {
            return 0;
        }

        @Override
        public String getColumnName(int columnIndex) {
            return null;
        }

        @Override
        public String[] getColumnNames() {
            return null;
        }

        @Override
        public int getColumnCount() {
            return 10;
        }

        @Override
        public byte[] getBlob(int columnIndex) {
            return null;
        }

        @Override
        public String getString(int columnIndex) {
            if (columnIndex == 2) {
                return mFirstMessageId;
            } else if (columnIndex == 1) {
                if (mPos == 1) {
                    return "";
                } else if (mPos == 2) {
                    return "+34200000250;+34200000251;+34200000252";
                } else if (mPos == 3) {
                    return "+34200000250,+34200000251,+34200000252";
                }
                return "";
            }
            return null;
        }

        @Override
        public void copyStringToBuffer(int columnIndex, CharArrayBuffer buffer) {

        }

        @Override
        public short getShort(int columnIndex) {
            return 0;
        }

        @Override
        public int getInt(int columnIndex) {
            return mType;
        }

        @Override
        public long getLong(int columnIndex) {
            return 0;
        }

        @Override
        public float getFloat(int columnIndex) {
            return 0;
        }

        @Override
        public double getDouble(int columnIndex) {
            return 0;
        }

        @Override
        public int getType(int columnIndex) {
            return 0;
        }

        @Override
        public boolean isNull(int columnIndex) {
            return false;
        }

        @Override
        public void deactivate() {

        }

        @Override
        @Deprecated
        public boolean requery() {
            return false;
        }

        @Override
        public void close() {
        }

        @Override
        public boolean isClosed() {
            return false;
        }

        @Override
        public void registerContentObserver(ContentObserver observer) {
        }

        @Override
        public void unregisterContentObserver(ContentObserver observer) {
        }

        @Override
        public void registerDataSetObserver(DataSetObserver observer) {
        }

        @Override
        public void unregisterDataSetObserver(DataSetObserver observer) {
        }

        @Override
        public void setNotificationUri(ContentResolver cr, Uri uri) {
        }

        @Override
        public boolean getWantsAllOnMoveCalls() {
            return false;
        }

        @Override
        public Bundle getExtras() {
            return null;
        }

        @Override
        public Bundle respond(Bundle extras) {
            return null;
        }
    }

}
