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

import android.content.ContentResolver;
import android.content.Context;
import android.database.CharArrayBuffer;
import android.database.ContentObserver;
import android.database.Cursor;
import android.database.DataSetObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.SessionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
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
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

public class OneOneChatTest extends AndroidTestCase {
    private static final String TAG = "OneOneChatTest";
    private static final String METHOD_SEND_MESSAGE = "sendMessage";
    private static final String METHOD_ON_CAPABILITY_CHANGED = "onCapabilityChanged";
    private static final String METHOD_ON_REGISTRATION_CHANGED = "onStatusChanged";
    private static final String METHOD_HANDLE_INVITATION = "handleInvitation";
    private static final String METHOD_MARK_AS_DISPLAYED = "markMessageAsDisplayed";
    private static final String METHOD_RECEIVE_MESSAGE = "onReceiveMessage";
    private static final String METHOD_CLEAR_HISTORY = "clearHistoryForContact";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MESSAGE_API = "mMessagingApi";
    private static final String REGISTRATION_API = "mRegistrationApi";
    private static final String REGISTRATION_STATUS = "mRegistrationStatus";
    private static final String CONTENT_SEND_MESSAGE = "test message";
    private static final String CONTACT_NUMBER = "+34200000252";
    private static final String CONTACT_NUMBER_TEST = "+34200000251";
    private static final String DISPLAY_NAME = "Wenhuai Zhao";
    private static final String FT_CONTROLLER = "mFileTransferController";
    private static final String REMOTE_FT_CAPABILITY = "mRemoteFtCapability";
    private static final String METHOD_RESEND_MESSAGE = "resendStoredMessages";
    private static final String METHOD_MESSAGE_DEPOT = "MessageDepot";
    private static final String FIELD_MESSAGE_DEPOT = "mMessageDepot";
    private static final String METHOD_STORE_MESSAGE = "storeMessage";
    private static final int MESSAGE_TAG = 111;
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;
    private Participant mParticipant = null;
    private One2OneChat mChat = null;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp entry");
        super.setUp();
        Field apiManagerfield = ApiManager.class
                .getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerfield.setAccessible(true);
        apiManagerfield.set(ApiManager.class, null);

        Method initializeMethod = ApiManager.class.getDeclaredMethod(
                API_MANAGER_INITIALIZE, Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, mContext);

        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp apiManager is null", apiManager);
        Field messageApiField = ApiManager.class.getDeclaredField(MESSAGE_API);
        messageApiField.setAccessible(true);
        messageApiField.set(apiManager, new MockMessagingApi(mContext));

        Field registrationApiField = ApiManager.class
                .getDeclaredField(REGISTRATION_API);
        registrationApiField.setAccessible(true);
        MockRegistrationApi mockRegistrationApi = new MockRegistrationApi(
                mContext);
        registrationApiField.set(apiManager, mockRegistrationApi);

        Field registrationStatusField = mockRegistrationApi.getClass()
                .getSuperclass().getDeclaredField(REGISTRATION_STATUS);
        registrationStatusField.setAccessible(true);
        registrationStatusField.set(mockRegistrationApi,
                new MockRegistrationStatus());

        RegistrationApi registrationApi = apiManager.getRegistrationApi();
        assertNotNull("setUp registrationApi is null", registrationApi);

        MessagingApi messagingApi = apiManager.getMessagingApi();
        assertNotNull("setUp messagingApi is null", messagingApi);
        messagingApi.connectApi();

        mParticipant = new Participant(CONTACT_NUMBER, DISPLAY_NAME);
        IChatManager instance = ModelImpl.getInstance();
        assertNotNull("setUp instance is null", instance);
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        mChat = new One2OneChat((ModelImpl) instance, new MockChatWindow(),
                mParticipant, parcelUuid);
        Logger.d(TAG, "setUp exit");
    }

    @Override
    protected void tearDown() throws Exception {
        getContext().getContentResolver().delete(
                UnregMessageProvider.CONTENT_URI, null, null);
        super.tearDown();
    }

   /*
    public void testCase01_SendMessage() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException,
            InterruptedException {
        Logger.v(TAG, "testCase01_SendMessage entry");
        Method method = mChat.getClass().getDeclaredMethod(METHOD_SEND_MESSAGE,
                String.class, Integer.class);
        method.setAccessible(true);
        method.invoke(mChat, CONTENT_SEND_MESSAGE, -1);
        Field allMessageField = getPrivateField(mChat.getClass(),
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
        Logger.v(TAG, "testCase1_SendMessage exit");
    }
*/
    /**
     * test remote ft capability is true.
     */
   /* public void testCase02_FTCapabilityChangedToTrue()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase02_FTCapabilityChangedToTrue() entry");
        Capabilities ftCapbilitySupport = new Capabilities();
        ftCapbilitySupport.setFileTransferSupport(true);
        ftCapbilitySupport.setRcseContact(true);
        mChat.onCapabilityChanged(CONTACT_NUMBER, ftCapbilitySupport);
        Field ftControllerField = getPrivateField(mChat.getClass(),
                FT_CONTROLLER);
        Object ftController = ftControllerField.get(mChat);
        Field ftCapabilityField = getPrivateField(ftController.getClass(),
                REMOTE_FT_CAPABILITY);
        boolean ftCapability = ftCapabilityField.getBoolean(ftController);
        assertTrue(ftCapability);
        
        ftCapbilitySupport.setFileTransferSupport(false);
        ftCapbilitySupport.setRcseContact(false);
        ContactsListManager.getInstance().setStrangerList("+862008200", true);
        mChat.onCapabilityChanged("+862008200", ftCapbilitySupport);
    }
*/
    /**
     * test remote ft capability is false.
     */
    public void testCase03_FTCapabilityChangedToFalse()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase03_FTCapabilityChangedToFalse() entry");
        Capabilities ftCapbilitySupport = new Capabilities();
        ftCapbilitySupport.setFileTransferSupport(false);
        Method method = mChat.getClass().getDeclaredMethod(
                METHOD_ON_CAPABILITY_CHANGED, String.class, Capabilities.class);
        method.setAccessible(true);
        method.invoke(mChat, CONTACT_NUMBER, ftCapbilitySupport);
        Field ftControllerField = getPrivateField(mChat.getClass(),
                FT_CONTROLLER);
        Object ftController = ftControllerField.get(mChat);
        Field ftCapabilityField = getPrivateField(ftController.getClass(),
                REMOTE_FT_CAPABILITY);
        boolean ftCapability = ftCapabilityField.getBoolean(ftController);
        assertFalse(ftCapability);
    }

    /**
     * test local registration status is true.
     */
    public void testCase04_RegistrationChangedToTrue()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase04_RegistrationChangedToTrue() entry");
        Method method = mChat.getClass().getDeclaredMethod(
                METHOD_ON_REGISTRATION_CHANGED, boolean.class);
        method.setAccessible(true);
        method.invoke(mChat, true);
        Field ftControllerField = getPrivateField(mChat.getClass(),
                FT_CONTROLLER);
        Object ftController = ftControllerField.get(mChat);
        Field registrationField = getPrivateField(ftController.getClass(),
                REGISTRATION_STATUS);
        boolean ftCapability = registrationField.getBoolean(ftController);
        assertTrue(ftCapability);
    }

    /**
     * test local registration status is false.
     */
    public void testCase05_RegistrationChangedToFalse()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase05_RegistrationChangedToFalse() entry");
        Method method = mChat.getClass().getDeclaredMethod(
                METHOD_ON_REGISTRATION_CHANGED, boolean.class);
        method.setAccessible(true);
        method.invoke(mChat, false);
        Field ftControllerField = getPrivateField(mChat.getClass(),
                FT_CONTROLLER);
        Object ftController = ftControllerField.get(mChat);
        Field registrationField = getPrivateField(ftController.getClass(),
                REGISTRATION_STATUS);
        boolean ftCapability = registrationField.getBoolean(ftController);
        assertFalse(ftCapability);
    }

    /**
     * test handle invitation in background.
     */
    public void testCase06_HandleInvitationInBackground()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase06_HandleInvitationInBackground() entry");
        MockImSession chatSession = new MockImSession();
        ArrayList<IChatMessage> messages = new ArrayList<IChatMessage>();
        ChatMessageReceived receiveMs = new ChatMessageReceived(
                new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true, null));
        ;
        messages.add(receiveMs);
        Field isBackgroundField = getPrivateField(mChat.getClass()
                .getSuperclass(), "mIsInBackground");
        isBackgroundField.setAccessible(true);
        isBackgroundField.set(mChat, true);
        Method method = mChat.getClass().getDeclaredMethod(
                METHOD_HANDLE_INVITATION, IChatSession.class, ArrayList.class);
        method.setAccessible(true);
        method.invoke(mChat, chatSession, messages);
        Field allMessageField = getPrivateField(mChat.getClass(),
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
    }

    /**
     * test handle invitation not in background.
     */
    public void testCase07_HandleInvitationNotInBackground()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase07_HandleInvitationNotInBackground() entry");
        MockImSession chatSession = new MockImSession();
        ArrayList<IChatMessage> messages = new ArrayList<IChatMessage>();
        ChatMessageReceived receiveMs = new ChatMessageReceived(
                new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true, null));
        ;
        messages.add(receiveMs);
        Field isBackgroundField = Utils.getPrivateField(
                One2OneChat.class.getSuperclass(), "mIsInBackground");
        isBackgroundField.set(mChat, false);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_HANDLE_INVITATION, IChatSession.class, ArrayList.class);
        method.invoke(mChat, chatSession, messages);
        Field allMessageField = Utils.getPrivateField(One2OneChat.class,
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
    }

    /**
     * test handle Storeforward invitation not in background.
     */
    public void testCase08_HandleStoreforwardInvitationNotInBack()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG,
                "testCase08_HandleStoreforwardInvitationNotInBack() entry");
        MockStoreforWardImSession chatSession = new MockStoreforWardImSession();
        ArrayList<IChatMessage> messages = new ArrayList<IChatMessage>();
        ChatMessageReceived receiveMs = new ChatMessageReceived(
                new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true, null));
        ;
        messages.add(receiveMs);
        Field isBackgroundField = Utils.getPrivateField(
                One2OneChat.class.getSuperclass(), "mIsInBackground");
        isBackgroundField.set(mChat, false);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_HANDLE_INVITATION, IChatSession.class, ArrayList.class);
        method.setAccessible(true);
        method.invoke(mChat, chatSession, messages);
        Field allMessageField = Utils.getPrivateField(One2OneChat.class,
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
    }

    /**
     * test handle Storeforward invitation in background.
     */
    public void testCase09_HandleStoreforwardInvitationInBack()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase09_HandleStoreforwardInvitationInBack() entry");
        MockStoreforWardImSession chatSession = new MockStoreforWardImSession();
        ArrayList<IChatMessage> messages = new ArrayList<IChatMessage>();
        ChatMessageReceived receiveMs = new ChatMessageReceived(
                new InstantMessage(null, null, CONTENT_SEND_MESSAGE, true, null));
        ;
        messages.add(receiveMs);
        Field isBackgroundField = Utils.getPrivateField(
                One2OneChat.class.getSuperclass(), "mIsInBackground");
        isBackgroundField.set(mChat, true);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_HANDLE_INVITATION, IChatSession.class, ArrayList.class);
        method.invoke(mChat, chatSession, messages);
        Field allMessageField = Utils.getPrivateField(One2OneChat.class,
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
    }

    /**
     * test MarkMessageAsDisplayed.
     */
    public void testCase10_MarkMessageAsDisplayed()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase10_MarkMessageAsDisplayed() entry");
        Participant participant = new Participant(CONTACT_NUMBER_TEST,
                DISPLAY_NAME);
        IChatManager instance = ModelImpl.getInstance();
        assertNotNull("setUp instance is null", instance);
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        mChat = new One2OneChat((ModelImpl) instance, new MockChatWindow(),
                participant, parcelUuid);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_MARK_AS_DISPLAYED, InstantMessage.class);
        method.invoke(mChat, new InstantMessage(null, null,
                CONTENT_SEND_MESSAGE, true, null));
        assertNull(mChat);
    }

    /**
     * test onReceivedMessage displayedRequested.
     */
    public void testCase11_OnReceivedMessageDisplayedRequested()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase11_OnReceivedMessageDisplayedRequested() entry");
        InstantMessage message = new InstantMessage(null, null,
                CONTENT_SEND_MESSAGE, true, null);
        Field isBackgroundField = Utils.getPrivateField(
                One2OneChat.class.getSuperclass(), "mIsInBackground");
        isBackgroundField.set(mChat, true);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_RECEIVE_MESSAGE, InstantMessage.class);
        method.invoke(mChat, message);
        Field allMessageField = Utils.getPrivateField(One2OneChat.class,
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
        allMessage.clear();
        isBackgroundField.set(mChat, false);
        method.invoke(mChat, message);
        waitForMessageAdded(allMessage);
    }

    /**
     * test onReceivedMessage no displayedRequested.
     */
    public void testCase12_OnReceivedMessageNotDisplayedRequested()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG,
                "testCase12_OnReceivedMessageNotDisplayedRequested() entry");
        InstantMessage message = new InstantMessage(null, null,
                CONTENT_SEND_MESSAGE, false, null);
        Field isBackgroundField = Utils.getPrivateField(
                One2OneChat.class.getSuperclass(), "mIsInBackground");
        isBackgroundField.set(mChat, true);
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_RECEIVE_MESSAGE, InstantMessage.class);
        method.invoke(mChat, message);
        Field allMessageField = Utils.getPrivateField(One2OneChat.class,
                "mAllMessages");
        ArrayList<IChatMessage> allMessage = (ArrayList<IChatMessage>) allMessageField
                .get(mChat);
        waitForMessageAdded(allMessage);
        allMessage.clear();
        isBackgroundField.set(mChat, false);
        method.invoke(mChat, message);
        waitForMessageAdded(allMessage);
    }

    /**
     * test clear history for contacts.
     */
    public void testCase13_ClearHistoryForContact()
            throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase13_ClearHistoryForContact() entry");
        AndroidFactory.setApplicationContext(mContext);
        Method method = Utils.getPrivateMethod(One2OneChat.class,
                METHOD_CLEAR_HISTORY);
        method.invoke(mChat);
        assertNull(mChat);
    }

    /**
     * Test case for resendStoredMessages()
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws SecurityException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    public void testCase14_resendStoredMessages() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException,
            SecurityException, NoSuchMethodException,
            InvocationTargetException, InstantiationException {
        Logger.d(TAG, "testCase14_resendStoredMessages() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                new MockChatWindow(), participant, tag);

        Field messageDepotField = Utils.getPrivateField(One2OneChat.class,
                FIELD_MESSAGE_DEPOT);
        Object messageDepot = messageDepotField.get(oneoneChat);
        Method methodStore = Utils.getPrivateMethod(messageDepot.getClass(),
                METHOD_STORE_MESSAGE, String.class, int.class);
        methodStore.invoke(messageDepot, CONTENT_SEND_MESSAGE, MESSAGE_TAG);
        assertFalse(oneoneChat.isMessageSent());

        Method methodResend = Utils.getPrivateMethod(messageDepot.getClass(),
                METHOD_RESEND_MESSAGE);
        methodResend.invoke(messageDepot);
        assertTrue(oneoneChat.isMessageSent());
    }

    /**
     * Test judgeUnLoadedHistory
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     */
    public void testCase15_judgeUnLoadedHistory() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Logger.d(TAG, "testCase15_judgeUnLoadedHistory() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        IOne2OneChatWindow one2OneChatWindow = new MockChatWindow();
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                one2OneChatWindow, participant, tag);
        Class<?>[] clazzes = oneoneChat.getClass().getSuperclass()
                .getDeclaredClasses();
        Constructor<?> ctorQueryHandler = null;
        for (Class<?> clazz : clazzes) {
            if ("QueryHandler".equals(clazz.getSimpleName())) {
                ctorQueryHandler = clazz.getDeclaredConstructor(Context.class,
                        IChatWindow.class, ArrayList.class);
                break;
            }
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
        }
        ArrayList<IChatMessage> allMessages = new ArrayList<IChatMessage>();
        Object queryHandler = ctorQueryHandler.newInstance(ApiManager
                .getInstance().getContext(), one2OneChatWindow, allMessages);
        Method methodJudgeUnLoadedHistory = Utils.getPrivateMethod(
                queryHandler.getClass(), "judgeUnLoadedHistory", Cursor.class,
                Object.class);
        MockCursor cursor = null;
        String firstMessage = null;
        assertEquals(false, methodJudgeUnLoadedHistory.invoke(queryHandler,
                cursor, firstMessage));
        firstMessage = "12345600";
        assertEquals(false, methodJudgeUnLoadedHistory.invoke(queryHandler,
                cursor, firstMessage));
        cursor = new MockCursor();
        cursor.mIsAfterLast = true;
        assertEquals(false, methodJudgeUnLoadedHistory.invoke(queryHandler,
                cursor, firstMessage));
        cursor.mIsAfterLast = false;
        cursor.mFirstMessageId = firstMessage;
        assertEquals(false, methodJudgeUnLoadedHistory.invoke(queryHandler,
                cursor, firstMessage));
        String messageId = "89100124";
        cursor.mFirstMessageId = messageId;
        assertEquals(true, methodJudgeUnLoadedHistory.invoke(queryHandler,
                cursor, firstMessage));

        // Test onQueryComplete
        allMessages.clear();
        Method methodOnQueryComplete = Utils.getPrivateMethod(
                queryHandler.getClass(), "onQueryComplete", int.class,
                Object.class, Cursor.class);
        cursor.mFirstMessageId = firstMessage;
        int count = 0;
        String loadedId = firstMessage;
        cursor.mFirstMessageId = messageId;
        methodOnQueryComplete.invoke(queryHandler, count, loadedId, cursor);
        assertEquals(0, allMessages.size());
        cursor.mFirstMessageId = firstMessage;
        methodOnQueryComplete.invoke(queryHandler, count, loadedId, cursor);
        assertEquals(0, allMessages.size());
        count = 10;
        methodOnQueryComplete.invoke(queryHandler, count, loadedId, cursor);
        cursor.mFirstMessageId = messageId;
        methodOnQueryComplete.invoke(queryHandler, count, loadedId, cursor);

        // Test findLoadedMessage
        Method methodfindLoadedMessage = Utils.getPrivateMethod(
                queryHandler.getClass(), "findLoadedMessage", Cursor.class,
                Object.class);
        assertFalse((Boolean) methodfindLoadedMessage.invoke(queryHandler,
                cursor, firstMessage));
        assertTrue((Boolean) methodfindLoadedMessage.invoke(queryHandler,
                cursor, messageId));

        allMessages.clear();
        // Test loadMessage
        Method methodloadMessage = Utils.getPrivateMethod(
                queryHandler.getClass(), "loadMessage", Cursor.class,
                String.class, Object.class, int.class);
        cursor.mFirstMessageId = messageId;
        cursor.mType = 0;
        methodloadMessage.invoke(queryHandler, cursor, messageId, messageId,
                count);
        assertEquals(0, allMessages.size());
        cursor.mType = 1;
        methodloadMessage.invoke(queryHandler, cursor, messageId, messageId,
                count);
        assertEquals(0, allMessages.size());

        cursor.mFirstMessageId = firstMessage;
        cursor.mType = 0;
        methodloadMessage.invoke(queryHandler, cursor, messageId, messageId,
                count);
        assertEquals(1, allMessages.size());
        allMessages.clear();
        cursor.mType = 1;
        messageId = "xyzzzzzzzzzz";
        methodloadMessage.invoke(queryHandler, cursor, messageId, messageId,
                count);
        assertEquals(1, allMessages.size());
        allMessages.clear();
    }

    /**
     * Test FileTransferReceiverListene
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException 
     */
    public void testCase16_FileTransferReceiverListener()
            throws NoSuchMethodException, IllegalArgumentException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase16_FileTransferReceiverListener() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        IOne2OneChatWindow one2OneChatWindow = new MockChatWindow();
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                one2OneChatWindow, participant, tag);
        Class<?>[] clazzes = oneoneChat.getClass().getSuperclass()
                .getDeclaredClasses();
        Constructor<?> ctorReceiveFileTransfer = null;
        for (Class<?> clazz : clazzes) {
            if ("ReceiveFileTransfer".equals(clazz.getSimpleName())) {
                ctorReceiveFileTransfer = clazz.getDeclaredConstructor(
                        One2OneChat.class, IFileTransferSession.class);
                break;
            }
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
        }
        MockFtSession ftSession = new MockFtSession();
        // Create a ReceiveFileTransfer instance
        Object receiveFileTransfer = ctorReceiveFileTransfer.newInstance(
                oneoneChat, ftSession);

        Logger.d(TAG,
                "Find receiveFileTransfer's inner class: receiveFileTransfer = "
                        + receiveFileTransfer);
        clazzes = receiveFileTransfer.getClass().getDeclaredClasses();
        Logger.d(TAG, "clazzes length = " + clazzes.length);
        Constructor<?> ctorFileTransferReceiverListener = null;
        for (Class<?> clazz : clazzes) {
            if ("FileTransferReceiverListener".equals(clazz.getSimpleName())) {
                ctorFileTransferReceiverListener = clazz
                        .getDeclaredConstructor(receiveFileTransfer.getClass());
                ctorFileTransferReceiverListener.setAccessible(true);
                Logger.d(TAG, "FileTransferReceiverListener ctr :"
                        + ctorFileTransferReceiverListener);
                Logger.d(TAG, "FileTransferReceiverListener inner class name :"
                        + clazz.getSimpleName());
                break;
            }
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
        }
        Object fileTransferReceiverListener = ctorFileTransferReceiverListener
                .newInstance(receiveFileTransfer);
        Field fieldFileTransfer = Utils.getPrivateField(
                receiveFileTransfer.getClass(), "mFileTransfer");
        IFileTransfer fileTransfer = (IFileTransfer) fieldFileTransfer
                .get(receiveFileTransfer);

        // Test handleSessionStarted
        Method methodhandleSessionStarted = Utils
                .getPrivateMethod(fileTransferReceiverListener.getClass(),
                        "handleSessionStarted");
        methodhandleSessionStarted.invoke(fileTransferReceiverListener);

        // Test handleSessionAborted
        Field fieldFileTransferSession = Utils.getPrivateField(
                receiveFileTransfer.getClass(), "mFileTransferSession");
        fieldFileTransferSession.set(receiveFileTransfer, ftSession);
        Method methodhandleSessionAborted = Utils
                .getPrivateMethod(fileTransferReceiverListener.getClass(),
                        "handleSessionAborted");
        methodhandleSessionAborted.invoke(fileTransferReceiverListener);
        assertNull(fieldFileTransferSession.get(receiveFileTransfer));

        // Test handleSessionTerminatedByRemote
        fieldFileTransferSession.set(receiveFileTransfer, ftSession);
        Method methodhandleSessionTerminatedByRemote = Utils.getPrivateMethod(
                fileTransferReceiverListener.getClass(),
                "handleSessionTerminatedByRemote");
        methodhandleSessionTerminatedByRemote
                .invoke(fileTransferReceiverListener);
        assertNull(fieldFileTransferSession.get(receiveFileTransfer));

        // Test handleTransferProgress
        Method methodhandleTransferProgress = Utils.getPrivateMethod(
                fileTransferReceiverListener.getClass(),
                "handleTransferProgress", long.class, long.class);
        methodhandleTransferProgress.invoke(fileTransferReceiverListener, 10,
                200);

        // Test handleTransferError
        fieldFileTransferSession.set(receiveFileTransfer, ftSession);
        Method methodhandleTransferError = Utils.getPrivateMethod(
                fileTransferReceiverListener.getClass(), "handleTransferError",
                int.class);
        methodhandleTransferError.invoke(fileTransferReceiverListener,
                FileSharingError.SESSION_INITIATION_DECLINED);
        assertNull(fieldFileTransferSession.get(receiveFileTransfer));
        fieldFileTransferSession.set(receiveFileTransfer, ftSession);
        methodhandleTransferError.invoke(fileTransferReceiverListener,
                FileSharingError.MEDIA_SAVING_FAILED);
        assertNull(fieldFileTransferSession.get(receiveFileTransfer));

        // Test handleFileTransfered
        Method methodhandleFileTransfered = Utils.getPrivateMethod(
                fileTransferReceiverListener.getClass(),
                "handleFileTransfered", String.class);
        methodhandleFileTransfered.invoke(fileTransferReceiverListener,
                "/sdcard/test.jpg");
        fileTransfer = (IFileTransfer) fieldFileTransfer
                .get(receiveFileTransfer);
        assertNull(fileTransfer);
        
        MockFtSession ftSession2 = null;
        Method methodhandleFileTransferInvitation = Utils.getPrivateMethod(
                receiveFileTransfer.getClass(),
                "handleFileTransferInvitation", IFileTransferSession.class);
        methodhandleFileTransferInvitation.invoke(receiveFileTransfer,
                ftSession2);
        
        methodhandleFileTransferInvitation.invoke(receiveFileTransfer,
                ftSession);
        // acceptFileTransferInvitation
        Method methodacceptFileTransferInvitation = Utils.getPrivateMethod(
                receiveFileTransfer.getClass(), "acceptFileTransferInvitation");
        ftSession.mIsABigFile = true;
        //fieldFileTransfer.set(receiveFileTransfer, value)
        methodacceptFileTransferInvitation.invoke(receiveFileTransfer);
        Thread.sleep(SLEEP_TIME);
    }

    /**
     * Test SentMessageManager
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws InstantiationException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     */
    @SuppressWarnings("unchecked")
    public void testCase17_SentMessageManager() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException,
            NoSuchFieldException {
        Logger.d(TAG, "testCase17_SentMessageManager() entry");
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        IOne2OneChatWindow one2OneChatWindow = new MockChatWindow();
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                one2OneChatWindow, participant, tag);
        Class<?>[] clazzes = oneoneChat.getClass().getSuperclass()
                .getDeclaredClasses();
        Constructor<?> ctorSentMessageManager = null;
        for (Class<?> clazz : clazzes) {
            if ("SentMessageManager".equals(clazz.getSimpleName())) {
                ctorSentMessageManager = clazz
                        .getDeclaredConstructor(One2OneChat.class);
                ctorSentMessageManager.setAccessible(true);
                break;
            }
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
        }
        Object sentMessageManager = ctorSentMessageManager
                .newInstance(oneoneChat);
        Field fieldSendingMessage = Utils.getPrivateField(
                sentMessageManager.getClass(), "mSendingMessage");
        HashMap<String, Object> sendingMessage = (HashMap<String, Object>) fieldSendingMessage
                .get(sentMessageManager);

        // Test onMessageSent
        Method methodonMessageSent = Utils.getPrivateMethod(
                sentMessageManager.getClass(), "onMessageSent",
                ISentChatMessage.class);
        ISentChatMessage sentMessage = null;
        methodonMessageSent.invoke(sentMessageManager, sentMessage);
        assertTrue(sendingMessage.size() == 0);
        String messageId = "123555";
        InstantMessage message = new InstantMessage(messageId,
                CONTACT_NUMBER_TEST, "hello", true, new Date());
        sentMessage = one2OneChatWindow.addSentMessage(message, 0);
        methodonMessageSent.invoke(sentMessageManager, sentMessage);
        assertTrue(sendingMessage.size() == 1);

        // Test markSendingMessagesDisplayed
        Method methodmarkSendingMessagesDisplayed = Utils.getPrivateMethod(
                sentMessageManager.getClass(), "markSendingMessagesDisplayed");
        methodmarkSendingMessagesDisplayed.invoke(sentMessageManager);
        assertTrue(sendingMessage.size() == 0);

        // Test onChatDestroy
        methodonMessageSent.invoke(sentMessageManager, sentMessage);
        assertTrue(sendingMessage.size() == 1);
        Method methodonChatDestroy = Utils.getPrivateMethod(
                sentMessageManager.getClass(), "onChatDestroy");
        methodonChatDestroy.invoke(sentMessageManager);
        assertTrue(sendingMessage.size() == 0);

        // Create a SendMessageWatcher object
        Object sendMessageWatcher = null;
        clazzes = sentMessageManager.getClass().getDeclaredClasses();
        Constructor<?> ctorSendMessageWatcher = null;
        for (Class<?> clazz : clazzes) {
            if ("SendMessageWatcher".equals(clazz.getSimpleName())) {
                ctorSendMessageWatcher = clazz.getDeclaredConstructor(
                        sentMessageManager.getClass(), ISentChatMessage.class);
                ctorSendMessageWatcher.setAccessible(true);
                break;
            }
        }
        sendMessageWatcher = ctorSendMessageWatcher.newInstance(
                sentMessageManager, sentMessage);
        Method methodgetId = Utils.getPrivateMethod(
                sendMessageWatcher.getClass(), "getId");

        // Test onTimeout
        Method methodonTimeout = Utils.getPrivateMethod(
                sentMessageManager.getClass(), "onTimeout",
                sendMessageWatcher.getClass());
        methodonTimeout.invoke(sentMessageManager, sendMessageWatcher);
        assertTrue(!sendingMessage.containsKey(methodgetId
                .invoke(sendMessageWatcher)));

        // Test SendMessageWatcher
        Method methodtoString = Utils.getPrivateMethod(
                sendMessageWatcher.getClass(), "toString");
        assertEquals(
                "SendMessageWatcher:" + methodgetId.invoke(sendMessageWatcher),
                methodtoString.invoke(sendMessageWatcher));

        Field filedStatus = Utils.getPrivateField(
                sendMessageWatcher.getClass(), "mStatus");

        // Test onMessageDelivered
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.SENDING);
        Method methodonMessageDelivered = Utils.getPrivateMethod(
                sendMessageWatcher.getClass(), "onMessageDelivered");
        methodonMessageDelivered.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.DISPLAYED,
                filedStatus.get(sendMessageWatcher));
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.FAILED);
        methodonMessageDelivered.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.FAILED,
                filedStatus.get(sendMessageWatcher));
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.DISPLAYED);
        methodonMessageDelivered.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.DISPLAYED,
                filedStatus.get(sendMessageWatcher));

        // Test run
        Method methodrun = Utils.getPrivateMethod(
                sendMessageWatcher.getClass(), "run");
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.SENDING);
        methodrun.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.FAILED,
                filedStatus.get(sendMessageWatcher));
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.FAILED);
        methodrun.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.FAILED,
                filedStatus.get(sendMessageWatcher));
        filedStatus.set(sendMessageWatcher, ISentChatMessage.Status.DISPLAYED);
        methodrun.invoke(sendMessageWatcher);
        assertEquals(ISentChatMessage.Status.DISPLAYED,
                filedStatus.get(sendMessageWatcher));

    }

    /**
     * Test One2OneChatListener
     * @throws IllegalArgumentException
     * @throws NoSuchMethodException
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase18_One2OneChatListener()
            throws IllegalArgumentException, NoSuchMethodException,
            InstantiationException, IllegalAccessException,
            InvocationTargetException, InterruptedException {
        Logger.d(TAG, "testCase18_One2OneChatListener() entry");
        Object one2OneChatListener = getOne2OneChatListener();
        Method methodhandleAddParticipantFailed = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleAddParticipantFailed",
                String.class);
        methodhandleAddParticipantFailed.invoke(one2OneChatListener, "test");

        Method methodhandleAddParticipantSuccessful = Utils.getPrivateMethod(
                one2OneChatListener.getClass(),
                "handleAddParticipantSuccessful");
        methodhandleAddParticipantSuccessful.invoke(one2OneChatListener);

        Method methodhandleConferenceEvent = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleConferenceEvent",
                String.class, String.class, String.class);
        methodhandleConferenceEvent.invoke(one2OneChatListener, "test", "test",
                "test");

        Method methoddestroySelf = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "destroySelf");
        methoddestroySelf.invoke(one2OneChatListener);

        Method methodhandleImError = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleImError", int.class);
        methodhandleImError.invoke(one2OneChatListener, 0);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleIsComposingEvent = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleIsComposingEvent",
                String.class, boolean.class);
        methodhandleIsComposingEvent.invoke(one2OneChatListener,
                CONTACT_NUMBER, true);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleMessageDeliveryStatus = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleMessageDeliveryStatus",
                String.class, String.class, long.class);
        methodhandleMessageDeliveryStatus.invoke(one2OneChatListener,
                CONTACT_NUMBER, ImdnDocument.DELIVERY_STATUS_DELIVERED,
                System.currentTimeMillis());
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);

        Method methodhandleReceiveMessage = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleReceiveMessage",
                InstantMessage.class);
        InstantMessage instantMessage = new InstantMessage("test",
                CONTACT_NUMBER, "test", true);
        methodhandleReceiveMessage.invoke(one2OneChatListener, instantMessage);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);
        
        Method methodhandleSessionAborted = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleSessionAborted");
        methodhandleSessionAborted.invoke(one2OneChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);
        
        Method methodhandleSessionStarted = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleSessionStarted");
        methodhandleSessionStarted.invoke(one2OneChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);
        
        Method methodhandleSessionTerminatedByRemote = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "handleSessionTerminatedByRemote");
        methodhandleSessionTerminatedByRemote.invoke(one2OneChatListener);
        // Give a chance to the mWorkerHandler
        Thread.sleep(SLEEP_TIME);
    }

    /**
     * Test loadChatMessages
     * @throws NoSuchFieldException 
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     */
    @SuppressWarnings("unchecked")
    public void testCase19_loadChatMessages() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase19_loadChatMessages entry");
        RcsSettings settings = RcsSettings.getInstance();
        if (settings != null) {
            settings = null;
        }
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        IOne2OneChatWindow one2OneChatWindow = new MockChatWindow();
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                one2OneChatWindow, participant, tag);
        assertNotNull(oneoneChat);
        assertNotNull(oneoneChat.getAllMessages());
        if (ApiManager.getInstance() != null) {
            Field fieldsInstance = Utils.getPrivateField(ApiManager.class,
                    "sInstance");
            fieldsInstance.set(null, null);
        }
        mChat.loadChatMessages(10);
        ApiManager.initialize(mContext);
        Field fieldmAllMessages = Utils.getPrivateField(mChat.getClass(),
                "mAllMessages");
        ArrayList<IChatMessage> allMessages = (ArrayList<IChatMessage>) fieldmAllMessages
                .get(mChat);
        InstantMessage message = new InstantMessage("test", CONTACT_NUMBER,
                "test", true);
        allMessages.add(new ChatMessageReceived(message));
        mChat.loadChatMessages(10);
        
        if(RcsSettings.getInstance() != null){
            Field fieldinstance = Utils.getPrivateField(RcsSettings.class, "instance");
            fieldinstance.set(null, null);
        }
        One2OneChat chat = new One2OneChat(modelInstance, one2OneChatWindow, participant, tag);
        assertNotNull(chat);

    }
    
    private Object getOne2OneChatListener() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException {
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Participant participant = new Participant(CONTACT_NUMBER_TEST, "");
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        IOne2OneChatWindow one2OneChatWindow = new MockChatWindow();
        MockOneOneChat oneoneChat = new MockOneOneChat(modelInstance,
                one2OneChatWindow, participant, tag);
        Class<?>[] clazzes = oneoneChat.getClass().getSuperclass()
                .getDeclaredClasses();
        Constructor<?> ctorOne2OneChatListener = null;
        for (Class<?> clazz : clazzes) {
            if ("One2OneChatListener".equals(clazz.getSimpleName())) {
                ctorOne2OneChatListener = clazz.getDeclaredConstructor(
                        One2OneChat.class, IChatSession.class);
                ctorOne2OneChatListener.setAccessible(true);
                break;
            }
            Logger.d(TAG, "inner class name :" + clazz.getSimpleName());
        }
        MockImSession imSession = new MockImSession();
        Object one2OneChatListener = ctorOne2OneChatListener.newInstance(
                oneoneChat, null);
        Method methoddestroySelf = Utils.getPrivateMethod(
                one2OneChatListener.getClass(), "destroySelf");
        methoddestroySelf.invoke(one2OneChatListener);
        one2OneChatListener = null;
        one2OneChatListener = ctorOne2OneChatListener.newInstance(oneoneChat,
                imSession);
        return one2OneChatListener;
    }

    private void waitForMessageAdded(List<IChatMessage> messageList)
            throws InterruptedException {
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

    private class MockRegistrationApi extends RegistrationApi {
        public MockRegistrationApi(Context context) {
            super(context);
        }
    }

    private class MockRegistrationStatus extends IRegistrationStatus.Stub {
        public void addRegistrationStatusListener(
                IRegistrationStatusRemoteListener listener) {

        }

        public boolean isRegistered() {
            return true;
        }
    }

    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession initiateOne2OneChatSession(String contact,
                String firstMsg) throws ClientApiException {
            return new MockImSession();
        }

        @Override
        public List<IBinder> getChatSessionsWith(String contact)
                throws ClientApiException {
            ArrayList<IBinder> result = new ArrayList<IBinder>();
            if (contact.equals(CONTACT_NUMBER)) {
                MockImSession session = new MockImSession();
                result.add(session.asBinder());
            } else {
                MockStoreforWardImSession session = new MockStoreforWardImSession();
                result.add(session.asBinder());
            }
            return result;
        }

        @Override
        public void setMessageDeliveryStatus(String contact, String msgId,
                String status) throws ClientApiException {
            mChat = null;
        }
    }

    private class MockStoreforWardImSession extends MockImSession {
        @Override
        public boolean isStoreAndForward() {
            return true;
        }

        public int getSessionState() {
            return SessionState.ESTABLISHED;
        }
    }

    private class MockImSession extends IChatSession.Stub {
        private static final String SESSION_ID = "z9hG4bK4jfs4p108gdhn49l42s0.1";
        private static final String CHAT_ID = "f13f2bc69d681dfc65e53abef05308fb@172.21.2.31";

        @Override
        public boolean isInComing() {
            return true;
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }

        public String getSessionID() {
            return SESSION_ID;
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
        public boolean isSessionIdle() {
            return false;
        }

        public boolean isGroupChat() {
            return false;
        }

        public String getSubject() {
            return null;
        }

        @Override
        public int getMaxParticipants() {
            return 0;
        }

        @Override
        public int getMaxParticipantsToBeAdded() {
            return 0;
        }

        public boolean isStoreAndForward() {
            return false;
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
        }

        public void removeSessionListener(IChatEventListener listener) {

        }

        public void setMessageDisplayedStatusBySipMessage(String contact,
                String msgId, String status) {

        }

        public String getReferredByHeader() {
            return null;
        }

        public List<String> getInivtedParticipants() {
            return null;
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

    private Field getPrivateField(Class clazz, String filedName)
            throws NoSuchFieldException {
        Field field = clazz.getDeclaredField(filedName);
        assertTrue(field != null);
        field.setAccessible(true);
        return field;
    }

    /**
     * Mock chat window for test
     */
    private class MockChatWindow implements IOne2OneChatWindow {
        private One2OneChatFragment mOneOneChatFragment = new One2OneChatFragment();

        public void setFileTransferEnable(int reason) {
        }

        public void setIsComposing(boolean isComposing) {
        }

        public void setRemoteOfflineReminder(boolean isOffline) {
        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return null;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return mOneOneChatFragment.new ReceivedFileTransfer(file, false);
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message,
                boolean isRead) {
            Logger.d(TAG, "addReceivedMessage");
            return mOneOneChatFragment.new ReceivedMessage(message);
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message,
                int messageTag) {
            Logger.d(TAG, "addSentMessage");
            return mOneOneChatFragment.new SentMessage(message);
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        @Override
        public void removeAllMessages() {
            mChat = null;
        }

        @Override
        public void updateAllMsgAsRead() {
        }

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
     * Mock 1-2-1 chat.
     */
    private class MockOneOneChat extends One2OneChat {
        private boolean mIsMessageSent = false;

        public MockOneOneChat(ModelImpl modelImpl,
                IOne2OneChatWindow chatWindow, Participant participant,
                Object tag) {
            super(modelImpl, chatWindow, participant, tag);
        }

        public boolean isMessageSent() {
            return mIsMessageSent;
        }

        public void sendMessage(String content, int messageTag) {
            mIsMessageSent = true;
        }
    }

    /**
     * Mock Cursor
     */
    public class MockCursor implements Cursor {
        private boolean mIsAfterLast = false;
        private String mFirstMessageId = null;
        private int mType = 0;

        @Override
        public int getCount() {
            return 0;
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
            return false;
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
            if ("type".equals(columnName)) {
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

    /**
     * Mock chat session that has a mock session-id , a mock file size and a
     * mock file name.
     */
    private class MockFtSession implements IFileTransferSession {
        private String mSessionId = SessionIdGenerator.getNewId();
        private boolean mIsAccepted = false;
        private boolean mIsRejected = false;
        private boolean mIsCanceled = false;
        private boolean mIsABigFile = false;

        public boolean isAccepted() {
            return mIsAccepted;
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
        public boolean isSessionPaused() {
        	return false;
        }

        @Override
        public void acceptSession() throws RemoteException {
            mIsAccepted = true;
        }

        @Override
        public void addSessionListener(IFileTransferEventListener listener)
                throws RemoteException {
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
            if (mIsABigFile == true) {
                return 5 * 1024 * 1024 * 1024;
            }
            return 1L;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return CONTACT_NUMBER;
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
