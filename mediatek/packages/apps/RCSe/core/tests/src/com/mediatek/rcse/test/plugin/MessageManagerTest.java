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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.IMessenger;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.provider.Telephony.Sms;
import android.test.ServiceTestCase;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.mms.ipmessage.message.IpTextMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager;
import com.mediatek.rcse.plugin.message.PluginController;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginIpAttachMessage;
import com.mediatek.rcse.plugin.message.PluginIpImageMessage;
import com.mediatek.rcse.plugin.message.PluginIpTextMessage;
import com.mediatek.rcse.plugin.message.PluginIpVcardMessage;
import com.mediatek.rcse.plugin.message.PluginIpVideoMessage;
import com.mediatek.rcse.plugin.message.PluginIpVoiceMessage;
import com.mediatek.rcse.plugin.message.PluginOne2OneChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.TagTranslater;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * This test is used to test PluginController
 */
public class MessageManagerTest extends ServiceTestCase<ApiService> {
    private static final String TAG = "MessageManagerTest";
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;

    private MockOne2OneChat mOne2OneChat = null;
    private MockGroupChat mGroupChat = null;
    private IRemoteOne2OneChatWindow mPluginChatWindow = null;
    private IRemoteGroupChatWindow mPluginGroupChatWindow = null;
    private IpMessageManager mMessageManager = null;
    private PluginChatWindowManager mChatWindowManager = null;

    private static final String MOCK_NUMBER = "+3422222244";
    private static final String MOCK_NUMBER_TWO = "+3422222245";
    private static final String MOCK_NAME = "mock name";
    private static final String MOCK_SENT_MESSAGE = "This is a sent message";
    private static final String MOCK_SENT_MESSAGE_ID = "mock sent message id";
    private static final String MOCK_RECEIVED_MESSAGE = "This is a received message";
    private static final String MOCK_RECEIVED_MESSAGE_ID = "mock received message id";
    private static final String MOCK_RECEIVED_MESSAGE_ID_TWO = "mock received message id 2";
    private static final String MOCK_RECEIVED_FILE_TRANSFER_ID = "123456789-987654321";
    private static final String MOCK_SENT_FILE_TRANSFER_ID = "987654321";
    private static final String MOCK_RECEIVED_FILE_TRANSFER = "This is a received file transfer";
    private static final String MOCK_SENT_FILE_TRANSFER = "This is a sent file transfer";
    private static final String MOCK_IMAGE_PATH = "/this/is/a/mock/path.png";
    private static final String SELECTION = Sms.ADDRESS + "=?";
    private static final String MOCK_WINDOW_TAG = "mock window tag";
    private static final String MOCK_GROUP_TAG = "138116f9-8376-4e99-b5fc-9be9c995ba9d";
    private static final String MOCK_GROUP_ADDRESS = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + MOCK_GROUP_TAG;

    public MessageManagerTest() {
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
        Method initializeMethod = Utils.getPrivateMethod(PluginController.class, "initialize", IMessenger.class);
        initializeMethod.invoke(null, messenger);
        Method methodInitialize =
            Utils.getPrivateMethod(ApiManager.class, "initialize", Context.class);
        methodInitialize.invoke(null, mContext);
        AndroidFactory.setApplicationContext(mContext);
        PluginApiManager.initialize(mContext);
        UUID uuid = UUID.randomUUID();
        mMessageManager = new IpMessageManager(mContext);
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Method saveTagMethod = Utils.getPrivateMethod(TagTranslater.class, "saveTag", Object.class);
        saveTagMethod.invoke(null, parcelUuid);
        Participant mockParticipant = new Participant(MOCK_NUMBER,MOCK_NAME);
        mPluginChatWindow =
                new PluginOne2OneChatWindow(mockParticipant, MOCK_WINDOW_TAG, mMessageManager);
        mPluginChatWindow.setFileTransferEnable(0);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, "mChatMap");
        mChatWindowManager = new PluginChatWindowManager(mMessageManager);
        mChatWindowManager.switchChatWindowByTag("");
        assertNull(mChatWindowManager.addOne2OneChatWindow(null, null));
        mPluginChatWindow = mChatWindowManager.addOne2OneChatWindow(parcelUuid.toString(), mockParticipant);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(ModelImpl.getInstance());
        mOne2OneChat = new MockOne2OneChat((ModelImpl) ModelImpl.getInstance(),
                new MockChatWindowAdapter(mPluginChatWindow), mockParticipant, parcelUuid);
        chatMap.put(parcelUuid, mOne2OneChat);
        parcelUuid = ParcelUuid.fromString(MOCK_GROUP_TAG);
        ArrayList<Participant> participantList = new ArrayList<Participant>();
        participantList.add(new Participant(MOCK_NUMBER,MOCK_NAME));
        participantList.add(new Participant(MOCK_NUMBER_TWO,MOCK_NAME));
        ArrayList<ParticipantInfo> infoList = new ArrayList<ParticipantInfo>();
        for (Participant participant : participantList) {
            infoList.add(new ParticipantInfo(participant, User.STATE_PENDING));
        }
        mPluginGroupChatWindow = mChatWindowManager.addGroupChatWindow(MOCK_GROUP_TAG, infoList);
        mGroupChat = new MockGroupChat((ModelImpl) ModelImpl.getInstance(), new MockGroupChatWindowAdapter(mPluginGroupChatWindow), participantList, parcelUuid);
        chatMap.put(parcelUuid, mGroupChat);
    }

    private void clearMmsDataBase() {
        ContentResolver contentResolver = mContext.getContentResolver();
        final String[] args = {MOCK_NUMBER};
        contentResolver.delete(Utils.SMS_CONTENT_URI, SELECTION, args);
        final String[] args2 = {MOCK_GROUP_ADDRESS};
        contentResolver.delete(Utils.SMS_CONTENT_URI, SELECTION , args2);
    }

    private int getTestedMessageCount() {
        ContentResolver contentResolver = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = {MOCK_NUMBER};
            cursor = contentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION, args, null);
            return cursor.getCount();
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    private long getTestedMessageId() {
        ContentResolver contentResolver = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = {MOCK_NUMBER};
            cursor = contentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION, args, null);
            assertEquals(1, cursor.getCount());
            assertTrue(cursor.moveToFirst());
            return cursor.getLong(cursor.getColumnIndex(Sms._ID));
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    private int getTestedMessageIpMsgId() {
        ContentResolver contentResolver = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = {MOCK_NUMBER};
            cursor = contentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION, args, null);
            assertEquals(1, cursor.getCount());
            assertTrue(cursor.moveToFirst());
            return cursor.getInt(cursor.getColumnIndex(Sms.IPMSG_ID));
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    private int getTestedGroupMessageCount() {
        ContentResolver contentResolver = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            final String[] args = {MOCK_GROUP_ADDRESS};
            cursor = contentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION, args, null);
            return cursor.getCount();
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
    }

    /**
     * Test whether 1-2-1 message can be added to mms db successfully
     * @throws Throwable
     */
    public void testCase1_One2OneChatMessage() throws Throwable {
        if (getTestedMessageCount() > 0) {
            clearMmsDataBase();
        }
        final IpMessage invalidMessage = new IpMessage();
        invalidMessage.setTo(MOCK_NUMBER);
        assertFalse(mMessageManager.saveIpMsg(invalidMessage, 0) > 0);
        assertNull(mMessageManager.getIpMsgInfo(0L));
        final IpTextMessage mockMessage = new IpTextMessage();
        assertFalse(mMessageManager.saveIpMsg(mockMessage, 0) > 0);
        mockMessage.setTo(MOCK_NUMBER);
        assertFalse(mMessageManager.saveIpMsg(mockMessage, 0) > 0);
        mockMessage.setBody(MOCK_SENT_MESSAGE);
        assertTrue(mMessageManager.saveIpMsg(mockMessage, 0) > 0);
        mOne2OneChat.waitForSentMessage(MOCK_SENT_MESSAGE);
        waitForMessageInMmsDB();
        long messageIdInMmsDb = getTestedMessageId();
        int ipMsgId = getTestedMessageIpMsgId();
        assertNotNull(mMessageManager.getIpMsgInfo(messageIdInMmsDb));
        mPluginChatWindow.addSentMessage(new InstantMessage(MOCK_SENT_MESSAGE_ID, MOCK_NUMBER,
                MOCK_SENT_MESSAGE, true), ipMsgId);
        assertNotNull(mMessageManager.getIpMsgInfo(messageIdInMmsDb));
        assertFalse(mMessageManager.removePresentMessage(ipMsgId));
        waitForMessageInMmsDB();
        assertEquals(1, getTestedMessageCount());
        mMessageManager.resendMessage(messageIdInMmsDb, 0);
        assertEquals(2, getTestedMessageCount());
        clearMmsDataBase();
        assertEquals(0, getTestedMessageCount());
    }

    /**
     * Test for sending 1-2-1 chat message to multiply contacts
     * @throws Throwable
     */
    public void testCase2_testMultiOne2OneChatMessage() throws Throwable {
        if (getTestedMessageCount() > 0) {
            clearMmsDataBase();
        }
        final IpTextMessage mockMessage = new IpTextMessage();
        mockMessage.setTo(MOCK_NUMBER + "," + MOCK_NUMBER);
        mockMessage.setBody(MOCK_SENT_MESSAGE);
        assertTrue(mMessageManager.saveIpMsg(mockMessage, 0) > 0);
        assertEquals(1, getTestedMessageCount());
        clearMmsDataBase();
        assertEquals(0, getTestedMessageCount());
    }
    /**
     * Test whether file transfer can be added to mms db successfully
     * @throws Throwable
     */
    public void testCase3_AttachTransfer() throws Throwable {
        if (getTestedMessageCount() > 0) {
            clearMmsDataBase();
        }
        Field instanceField = Utils.getPrivateField(ControllerImpl.class, "sControllerImpl");
        MockLocalController mockController = new MockLocalController();
        instanceField.set(null, mockController);
        final IpAttachMessage mockAttach = new IpAttachMessage();
        mockAttach.setTo(MOCK_NUMBER);
        assertFalse(mMessageManager.saveIpMsg(mockAttach, 0) > 0);
        mockAttach.setPath(MOCK_IMAGE_PATH);
        assertEquals(0, getTestedMessageCount());
        assertFalse(mMessageManager.saveIpMsg(mockAttach, 0) > 0);
        String realFilePath = findRealFile();
        mockAttach.setPath(realFilePath);
        Field fieldMaxFileSize = Utils.getPrivateField(ApiManager.class, "sMaxFileSize");
        long orgMaxFileSize = fieldMaxFileSize.getLong(null);
        fieldMaxFileSize.set(null, 1);
        assertFalse(mMessageManager.saveIpMsg(mockAttach, 0) > 0);
        fieldMaxFileSize.set(null, orgMaxFileSize);
        assertTrue(mMessageManager.saveIpMsg(mockAttach, 0) > 0);
        mockController.waitForFilePath(realFilePath);
        
        // Mock a received file transfer
        ContentValues mockRcseValues = new ContentValues();
        ContentResolver contentResolver = getContext().getContentResolver();
        mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
        mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_IMAGE_PATH);
        mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_FILE_TRANSFER_ID);
        mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_FILE_TRANSFER);
        mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
        contentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);

        FileStructForBinder fileStruct = new FileStructForBinder(new FileStruct(realFilePath, "mock name", 0, MOCK_RECEIVED_FILE_TRANSFER_ID, new Date()));

        mPluginChatWindow.addSentFileTransfer(fileStruct);
        waitForMessageInMmsDB();
        assertEquals(1, getTestedMessageCount());
        
        // Mock a received file transfer
        mockRcseValues.clear();
        mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
        mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_IMAGE_PATH);
        mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_SENT_FILE_TRANSFER_ID);
        mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_OUTGOING_FILE_TRANSFER);
        mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
        contentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
        FileStructForBinder struct = new FileStructForBinder(new FileStruct(realFilePath, "mock name", 0, MOCK_SENT_FILE_TRANSFER_ID, new Date()));

        mPluginChatWindow.addReceivedFileTransfer(struct, false);
        waitForMessageInMmsDB();
        assertEquals(2, getTestedMessageCount());

        clearMmsDataBase();
        assertEquals(0, getTestedMessageCount());
        instanceField.set(null, null);
    }

    /**
     * Test for sending file to multiply contacts
     * @throws Throwable
     */
    public void testCase4_testMultiAttachTransfer() throws Throwable {
        if (getTestedMessageCount() > 0) {
            clearMmsDataBase();
        }
        final IpAttachMessage mockAttach = new IpAttachMessage();
        mockAttach.setPath(findRealFile());
        mockAttach.setTo(MOCK_NUMBER + "," + MOCK_NUMBER);
        assertTrue(mMessageManager.saveIpMsg(mockAttach, 0) > 0);
        assertEquals(1, getTestedMessageCount());
        clearMmsDataBase();
        assertEquals(0, getTestedMessageCount());
    }

    /**
     * Test to rebuild 1-2-1 chat messages and file transfer using Mms DB information
     * @throws Throwable
     */
    public void testCase5_RebuildOne2OneChatMessageCache() throws Throwable {
        Field instanceField = Utils.getPrivateField(ControllerImpl.class, "sControllerImpl");
        MockLocalController mockController = new MockLocalController();
        instanceField.set(null, mockController);
        CacheRebuildHelper helper = new CacheRebuildHelper(getContext().getContentResolver());
        try {
            List<Integer> expectMessages = helper.prepareMockMessage();
            assertEquals(4, getTestedMessageCount());
            Method reloadMessageMethod = Utils.getPrivateMethod(PluginUtils.class, "reloadRcseMessages");
            reloadMessageMethod.invoke(null);
            mockController.waitForReloadedMessages(expectMessages);
            ModelImpl.getInstance().reloadMessages(null, expectMessages);
            helper.waitForMessageCache();
        } finally {
            helper.clearMockMessage();
            assertEquals(0, getTestedMessageCount());
        }
    }
    
    /**
     * Test to rebuild group chat messages using Mms DB information
     * @throws Throwable
     */
    public void testCase6_RebuildGroupChatMessageCache() throws Throwable {
        Method windowTagMethod =
                Utils.getPrivateMethod(PluginChatWindowManager.class, "findGroupWindowTag",
                        String.class);
        WindowTagGetter getter = (WindowTagGetter) windowTagMethod.invoke(mChatWindowManager, "");
        assertNull(getter);
        Field instanceField = Utils.getPrivateField(ControllerImpl.class, "sControllerImpl");
        MockLocalController mockController = new MockLocalController();
        instanceField.set(null, mockController);
        CacheRebuildHelper helper = new CacheRebuildHelper(getContext().getContentResolver());
        try {
            List<Integer> expectMessages = helper.prepareMockGroupMessage();
            assertEquals(2, getTestedGroupMessageCount());
            Method reloadMessageMethod = Utils.getPrivateMethod(PluginUtils.class, "reloadRcseMessages");
            reloadMessageMethod.invoke(null);
            mockController.waitForReloadedMessages(expectMessages);
            ModelImpl.getInstance().reloadMessages(MOCK_GROUP_ADDRESS, expectMessages);
            helper.waitForGroupMessageCache();
        } finally {
            helper.clearMockMessage();
            assertEquals(0, getTestedGroupMessageCount());
        }
    }

    /**
     * Test for method "getIpMessageStatusString" in IpMessageManager
     * @throws Throwable
     */
    public void testCase7_testGetIpMessageStatusString() throws Exception {
        assertNull(mMessageManager.getIpMessageStatusString(0L));
        assertEquals(0, mMessageManager.getStatus(0L));
        assertFalse(IpMessageManager.isInCache(MOCK_SENT_FILE_TRANSFER_ID));
        assertFalse(IpMessageManager.isInCache(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MO_CANCEL);

        String fileName = MOCK_IMAGE_PATH.substring(MOCK_IMAGE_PATH.lastIndexOf("/") + 1);
        Object tag = new Object();
        FileStructForBinder fileStruct = new FileStructForBinder(MOCK_IMAGE_PATH, fileName, 0, tag, new Date());
        PluginIpImageMessage imageMessage = new PluginIpImageMessage(fileStruct , MOCK_NUMBER);
        imageMessage.setStatus(IpMessageConsts.IpMessageStatus.MO_CANCEL);
        IpMessageManager.addMessage(0L, MOCK_SENT_FILE_TRANSFER_ID, imageMessage);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MO_CANCEL);
        assertEquals(IpMessageConsts.IpMessageStatus.MO_CANCEL, mMessageManager.getStatus(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));
        mMessageManager.getIpMessageStatusString(0L);
        imageMessage.setStatus(Integer.MIN_VALUE);//Mock to setup an unknown status
        assertNull(mMessageManager.getIpMessageStatusString(0L));

        PluginIpVideoMessage videoMessage = new PluginIpVideoMessage(fileStruct, MOCK_NUMBER);
        videoMessage.setStatus(IpMessageConsts.IpMessageStatus.MO_REJECTED);
        IpMessageManager.addMessage(0L, MOCK_SENT_FILE_TRANSFER_ID, videoMessage);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MO_REJECTED);
        assertEquals(IpMessageConsts.IpMessageStatus.MO_REJECTED, mMessageManager.getStatus(0L));
        mMessageManager.getIpMessageStatusString(0L);
        assertEquals(0, mMessageManager.getDownloadProcess(0L));

        PluginIpVoiceMessage voiceMessage = new PluginIpVoiceMessage(fileStruct, MOCK_NUMBER);
        voiceMessage.setStatus(IpMessageConsts.IpMessageStatus.MT_RECEIVED);
        voiceMessage.setDuration(0);
        IpMessageManager.addMessage(0L, MOCK_SENT_FILE_TRANSFER_ID, voiceMessage);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MT_RECEIVED);
        mMessageManager.getIpMessageStatusString(0L);
        assertEquals(IpMessageConsts.IpMessageStatus.MT_RECEIVED, mMessageManager.getStatus(0L));
        assertEquals(fileName, mMessageManager.getIpMessageStatusString(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));

        PluginIpVcardMessage vCardMessage = new PluginIpVcardMessage(fileStruct, MOCK_NUMBER);
        vCardMessage.setStatus(IpMessageConsts.IpMessageStatus.MT_CANCEL);
        IpMessageManager.addMessage(0L, MOCK_SENT_FILE_TRANSFER_ID, vCardMessage);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MT_CANCEL);
        mMessageManager.getIpMessageStatusString(0L);
        assertEquals(IpMessageConsts.IpMessageStatus.MT_CANCEL, mMessageManager.getStatus(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));
        vCardMessage.setRcsStatus(5);//Failed status
        mMessageManager.getIpMessageStatusString(0L);

        PluginIpAttachMessage attachMessage = new PluginIpAttachMessage(fileStruct, MOCK_NUMBER);
        attachMessage.setStatus(IpMessageConsts.IpMessageStatus.MT_REJECT);
        IpMessageManager.addMessage(0L, MOCK_SENT_FILE_TRANSFER_ID, attachMessage);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MT_REJECT);
        mMessageManager.getIpMessageStatusString(0L);
        assertEquals(IpMessageConsts.IpMessageStatus.MT_REJECT, mMessageManager.getStatus(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));
        attachMessage.setStatus(IpMessageConsts.IpMessageStatus.MT_INVITED);
        mMessageManager.setIpMessageStatus(0L, IpMessageConsts.IpMessageStatus.MT_INVITED);
        assertEquals(fileName, mMessageManager.getIpMessageStatusString(0L));
        assertEquals(IpMessageConsts.IpMessageStatus.MT_INVITED, mMessageManager.getStatus(0L));
        assertEquals(0, mMessageManager.getDownloadProcess(0L));

        IpMessageManager.updateCache(MOCK_SENT_FILE_TRANSFER_ID, fileStruct, MOCK_NUMBER);

        PluginIpTextMessage textMessage = new PluginIpTextMessage(new InstantMessage(MOCK_RECEIVED_MESSAGE_ID, MOCK_NUMBER,
                MOCK_RECEIVED_MESSAGE, true));
        IpMessageManager.addMessage(0L, MOCK_RECEIVED_MESSAGE_ID, textMessage);
        textMessage.setStatus(null);
        assertEquals(IpMessageConsts.IpMessageStatus.INBOX, mMessageManager.getStatus(0L));
        textMessage.setStatus(ISentChatMessage.Status.SENDING);
        assertEquals(IpMessageConsts.IpMessageStatus.OUTBOX, mMessageManager.getStatus(0L));
        textMessage.setStatus(ISentChatMessage.Status.DELIVERED);
        assertEquals(IpMessageConsts.IpMessageStatus.DELIVERED, mMessageManager.getStatus(0L));
        textMessage.setStatus(ISentChatMessage.Status.DISPLAYED);
        assertEquals(IpMessageConsts.IpMessageStatus.VIEWED, mMessageManager.getStatus(0L));
        textMessage.setStatus(ISentChatMessage.Status.FAILED);
        assertEquals(IpMessageConsts.IpMessageStatus.FAILED, mMessageManager.getStatus(0L));

    }

    private void clearMessageCache() throws Exception {
        Field fieldCacheRcseMessage = Utils.getPrivateField(IpMessageManager.class, "sCacheRcseMessage");
        Field fieldMessageMap = Utils.getPrivateField(IpMessageManager.class, "sMessageMap");
        ((Map)fieldCacheRcseMessage.get(null)).clear();
        ((Map)fieldMessageMap.get(null)).clear();
    }
    /**
     * As a helper to test cache rebuild
     */
    private class CacheRebuildHelper {
        private ContentResolver mContentResolver = null;

        public CacheRebuildHelper(ContentResolver contentResolver) {
            mContentResolver = contentResolver;
        }

        public List<Integer> prepareMockMessage() {
            List<Integer> rcseMessageIds = new ArrayList<Integer>();

            // Mock a sent message
            ContentValues mockRcseValues = new ContentValues();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_SENT_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_SENT_MESSAGE_ID);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            Uri insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            Integer insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_NUMBER);
            mockRcseValues.put(Sms.BODY, MOCK_SENT_MESSAGE);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "outbox"), mockRcseValues);

            // Mock a received message
            mockRcseValues.clear();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_MESSAGE_ID);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_CHAT_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_NUMBER);
            mockRcseValues.put(Sms.BODY, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "inbox"), mockRcseValues);
            
            // Mock a received file transfer
            mockRcseValues.clear();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_IMAGE_PATH);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_FILE_TRANSFER_ID);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_FILE_TRANSFER);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_NUMBER);
            mockRcseValues.put(Sms.BODY, MOCK_RECEIVED_FILE_TRANSFER);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "inbox"), mockRcseValues);

            // Mock a sent file transfer
            mockRcseValues.clear();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_IMAGE_PATH);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_SENT_FILE_TRANSFER_ID);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_OUTGOING_FILE_TRANSFER);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_NUMBER);
            mockRcseValues.put(Sms.BODY, MOCK_SENT_FILE_TRANSFER);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "outbox"), mockRcseValues);
            return rcseMessageIds;
        }
        
        public List<Integer> prepareMockGroupMessage() {
            List<Integer> rcseMessageIds = new ArrayList<Integer>();

            // Mock a received message from number 1
            ContentValues mockRcseValues = new ContentValues();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_MESSAGE_ID);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            Uri insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            Integer insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_GROUP_ADDRESS);
            mockRcseValues.put(Sms.BODY, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "inbox"), mockRcseValues);

            // Mock a received message from number 2
            mockRcseValues.clear();
            mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER_TWO);
            mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_MESSAGE_ID_TWO);
            mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE);
            mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
            insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
            insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
            rcseMessageIds.add(insertedRcseId);

            // Put it into Mms DB
            mockRcseValues.clear();
            mockRcseValues.put(Sms.ADDRESS, MOCK_GROUP_ADDRESS);
            mockRcseValues.put(Sms.BODY, MOCK_RECEIVED_MESSAGE);
            mockRcseValues.put(Sms.IPMSG_ID, insertedRcseId);
            mContentResolver.insert(Uri.withAppendedPath(Utils.SMS_CONTENT_URI, "inbox"), mockRcseValues);
            return rcseMessageIds;
        }

        public void waitForMessageCache() throws InterruptedException {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (!IpMessageManager.isInCache(MOCK_SENT_MESSAGE_ID)
                    || !IpMessageManager.isInCache(MOCK_RECEIVED_MESSAGE_ID)
                    || !IpMessageManager.isInCache(MOCK_RECEIVED_FILE_TRANSFER_ID)
                    || !IpMessageManager.isInCache(MOCK_SENT_FILE_TRANSFER_ID));
        }

        public void waitForGroupMessageCache() throws InterruptedException {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (!IpMessageManager.isInCache(MOCK_RECEIVED_MESSAGE_ID) || !IpMessageManager.isInCache(MOCK_RECEIVED_MESSAGE_ID_TWO));
        }

        public void clearMockMessage() {
            String[] arg = {MOCK_NUMBER};
            mContentResolver.delete(RichMessagingData.CONTENT_URI, RichMessagingData.KEY_CONTACT + "=?", arg);
            clearMmsDataBase();
        }
    }

    private String findRealFile() {
        Cursor cursor = null;
        String imageName = null;
        try {
            cursor =
                    mContext.getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            imageName = cursor.getString(cursor
                    .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        assertNotNull(imageName);
        return imageName;
    }

    private void waitForMessageInMmsDB() throws InterruptedException {
        long startTime = System.currentTimeMillis();
        do {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (getTestedMessageCount() <= 0);
    }

    @Override
    protected void tearDown() throws Exception {
        mPluginGroupChatWindow.getSentChatMessage("");
        mPluginGroupChatWindow.addSentMessage(null, 0);
        mPluginGroupChatWindow.addReceivedMessage(null, false);
        mPluginChatWindow.addLoadHistoryHeader(false);
        mPluginChatWindow.getSentChatMessage(null);
        mPluginChatWindow.updateAllMsgAsRead();
        mPluginChatWindow.removeAllMessages();
        mChatWindowManager.addOne2OneChatWindow(null, null);
        mChatWindowManager.removeOne2OneChatWindow(mPluginChatWindow);
        mChatWindowManager.removeGroupChatWindow(mPluginGroupChatWindow);
        ((Map)Utils.getPrivateField(IpMessageManager.class, "sCacheRcseMessage").get(null)).clear();
        ((Map)Utils.getPrivateField(IpMessageManager.class, "sMessageMap").get(null)).clear();
        Utils.clearAllStatus();
        clearMessageCache();
        super.tearDown();
    }

    /**
     * Mock a One2OneChat to receive message in Model
     */
    private class MockOne2OneChat extends One2OneChat {

        private String mSentMessage = null;

        public MockOne2OneChat(ModelImpl modelImpl, IOne2OneChatWindow chatWindow, Participant participant, Object tag) {
            super(modelImpl, chatWindow, participant, tag);
        }

        @Override
        public void sendMessage(String content, int messageTag) {
            Logger.d(MessageManagerTest.TAG, "sendMessage() content: " + content + " , messageTag: " + messageTag);
            mSentMessage = content;
        }

        public void waitForSentMessage(String content) {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
            } while (!content.equals(mSentMessage));
        }

        @Override
        protected boolean getRegistrationState() {
            return true;
        }
    }

    /**
     * Mock a GroupChat to receive message in Model
     */
    private class MockGroupChat extends GroupChat {

        private String mSentMessage = null;

        public MockGroupChat(ModelImpl modelImpl, IGroupChatWindow chatWindow, List<Participant> participants, Object tag) {
            super(modelImpl, chatWindow, participants, tag);
        }

        @Override
        public void sendMessage(String content, int messageTag) {
            Logger.d(MessageManagerTest.TAG, "sendMessage() content: " + content + " , messageTag: " + messageTag);
            mSentMessage = content;
        }

        public void waitForSentMessage(String content) {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
            } while (!content.equals(mSentMessage));
        }

    }
    
    /**
     * Mock a chat window to server as adapter between Model and Plugin
     */
    private class MockChatWindowAdapter implements IOne2OneChatWindow {

        private IRemoteOne2OneChatWindow mPluginChatWindow = null;
        public MockChatWindowAdapter(IRemoteOne2OneChatWindow pluginChatWindow) {
            mPluginChatWindow = pluginChatWindow;
        }

        

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            try {
                mPluginChatWindow.addSentFileTransfer(new FileStructForBinder(file));
            } catch (RemoteException e) {
                e.printStackTrace();
                fail();
            }
            return null;
        }

        @Override
        public void setFileTransferEnable(int reason) {}

        @Override
        public void setIsComposing(boolean isComposing) {}

        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {}

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {}

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            try {
                mPluginChatWindow.addReceivedMessage(message, isRead);
            } catch (RemoteException e) {
                e.printStackTrace();
                fail();
            }
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            try {
                mPluginChatWindow.addSentMessage(message, messageTag);
            } catch (RemoteException e) {
                e.printStackTrace();
                fail();
            }
            return null;
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        @Override
        public void removeAllMessages() {}

        @Override
        public void updateAllMsgAsRead() {}

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
     * Mock a chat window to server as adapter between Model and Plugin
     */
    private class MockGroupChatWindowAdapter implements IGroupChatWindow {

        private IRemoteGroupChatWindow mPluginChatWindow = null;
        
        private static final String TAG ="MockGroupChatWindowAdapter"; 

        public MockGroupChatWindowAdapter(IRemoteGroupChatWindow pluginChatWindow) {
            mPluginChatWindow = pluginChatWindow;
        }

        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {

        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {}
        
      
        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            Logger.d(TAG, "addReceivedMessage messageid is " + message.getMessageId());
            try {
                mPluginChatWindow.addReceivedMessage(message, isRead);
            } catch (RemoteException e) {
                e.printStackTrace();
                fail();
            }
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            try {
                mPluginChatWindow.addSentMessage(message, messageTag);
            } catch (RemoteException e) {
                e.printStackTrace();
                fail();
            }
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

        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

        @Override
        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return null;
        }

        @Override
        public void updateParticipants(List<ParticipantInfo> participants) {
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
     * Mock a controller to identify file transfer message
     */
    private class MockLocalController extends ControllerImpl {
        private Handler mHandler = new Handler();
        private String mFilePath = null;
        private List<Integer> mReloadedMessageIds = new ArrayList<Integer>();
        @Override
        public Message obtainMessage(int eventType, Object tag, Object data) {
            Logger.d(TAG, "obtainMessage() eventType: " + eventType + " , tag: " + tag + "data: " + data);
            if (data == null) {
                Logger.e(TAG, "obtainMessage() data is null: ");
                return null;
            }
            Message message = mHandler.obtainMessage(eventType);
            if (ChatController.EVENT_FILE_TRANSFER_INVITATION == eventType) {
                mFilePath = data.toString();
            } else if (ChatController.EVENT_RELOAD_MESSAGE == eventType) {
                mReloadedMessageIds.addAll((Collection<Integer>) data);
            }
            return message;
        }

        public void waitForFilePath(String expectPath) throws InterruptedException {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (!expectPath.equals(mFilePath));
        }

        public void waitForReloadedMessages(List<Integer> expectMessages) throws InterruptedException {
            long startTime = System.currentTimeMillis();
            do {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } while (!mReloadedMessageIds.containsAll(expectMessages));
        }
    }
}
