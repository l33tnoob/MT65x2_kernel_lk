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

package com.mediatek.rcse.test.activity.widgets;

import android.content.Context;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.ImageButton;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.ChatScreenActivity.ChatWindowManager;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test GroupChatWindow
 */
public class GroupChatWindowTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "GroupChatWindowTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_NUMBER = "+340200000254";
    private static final String MOCK_NUMBER2 = "+340200000255";
    private static final String MOCK_NUMBER3 = "+340200000256";
    private static final String MOCK_NUMBER4 = "+340200000257";
    private static final Participant MOCK_GROUP_PARTICIPANT = new Participant(MOCK_NUMBER,
            MOCK_NUMBER);
    private static final Participant MOCK_GROUP_PARTICIPANT2 = new Participant(MOCK_NUMBER2,
            MOCK_NUMBER2);
    private static final int MOCK_PARTICIPANT_NUM = 2;
    private static final String MOCK_RECEIVED_MESSAGE_ID = "100";
    private static final String MOCK_SENT_MESSAGE_ID = "200";
    private static final String MOCK_UNREAD_MESSAGE_ID = "500";
    private static final int MOCK_SENT_MESSAGE_TAG = 123;
    private static final String MOCK_SENT_MESSAGE_ID_NOT_EXIST = "300";
    private static final String MOCK_REMOTE_USER = "RemoteUser";
    private static final String MOCK_MESSAGE_CONTENT = "content";
    private static final String SEND_BUTTON = "mBtnSend";
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private ChatScreenActivity mActivity = null;
    private List mChatWindowManagerList = null;
    private GroupChatFragment mFragment = null;
    private ChatImpl mChat = null;
    private GroupChatWindow mGroupChatWindow = null;

    public GroupChatWindowTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
        super.setUp();
        getInstrumentation().waitForIdleSync();
        Field apiManagerField = ApiManager.class.getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerField.setAccessible(true);
        apiManagerField.set(ApiManager.class, null);

        Method initializeMethod = ApiManager.class.getDeclaredMethod(API_MANAGER_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());
        Intent intent = generateIntent();
        setActivityIntent(intent);
        Field fieldChatWindowManagerList = Utils.getPrivateField(ViewImpl.class,
                "mChatWindowManagerList");
        mChatWindowManagerList = (List) fieldChatWindowManagerList.get(ViewImpl.getInstance());
        int startCount = mChatWindowManagerList.size();
        mActivity = getActivity();
        waitForChatWindowManagerNum(startCount + 1);
        /**
         * The last one is ChatWindowManager
         */
        ChatWindowManager chatWindowManager = (ChatWindowManager) mChatWindowManagerList
                .get(mChatWindowManagerList.size() - 1);

        mChat = waitForChat();
        getInstrumentation().waitForIdleSync();
        mFragment = (GroupChatFragment) mActivity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        assertNotNull(mFragment);
        assertEquals(MOCK_PARTICIPANT_NUM, mFragment.getParticipantsNum());

        Field fieldChatWindow = Utils.getPrivateField(ChatImpl.class, "mChatWindow");
        Object groupChatWindowDispatcher = fieldChatWindow.get(mChat);
        assertNotNull(groupChatWindowDispatcher);

        Field fieldChatWindowMap = Utils.getPrivateField(groupChatWindowDispatcher.getClass()
                .getSuperclass(), "mChatWindowMap");
        ConcurrentHashMap<IChatWindowManager, IChatWindow> chatWindowMap = (ConcurrentHashMap<IChatWindowManager, IChatWindow>) fieldChatWindowMap
                .get(groupChatWindowDispatcher);
        assertNotNull(chatWindowMap);

        mGroupChatWindow = (GroupChatWindow) chatWindowMap.get(chatWindowManager);
        assertNotNull(mGroupChatWindow);
        Logger.v(TAG, "setUp() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() enter");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Test the function GroupChatWindow#getFragment()
     */
    public void testCase1_GetFragment() throws Throwable {
        Logger.v(TAG, "testCase1_GetFragment() entry");
        GroupChatFragment groupChatFragment = mGroupChatWindow.getFragment();
        assertNotNull(groupChatFragment);
        assertEquals(mFragment, groupChatFragment);
        Logger.v(TAG, "testCase1_GetFragment() exit");
    }

    /**
     * Test GroupChatWindow message functions:addReceivedMessage()
     * addSentMessage() getSentChatMessage() removeAllMessages()
     */
    public void testCase2_AddGetRemoveMessage() throws Throwable {
        Logger.v(TAG, "testCase2_AddGetRemoveMessage() entry");
        Field mGroupChatFragmentField = Utils.getPrivateField(GroupChatWindow.class,
                "mGroupChatFragment");
        Field sLockField = Utils.getPrivateField(GroupChatWindow.class, "sLock");
        Field mParticipantsField = Utils.getPrivateField(GroupChatWindow.class, "mParticipants");
        Field fieldMessageList = Utils.getPrivateField(GroupChatFragment.class, "mMessageList");
        List<IChatWindowMessage> messageList = (List<IChatWindowMessage>) fieldMessageList
                .get(mFragment);

        // Test addReceivedMessage()
        messageList.clear();
        assertEquals(0, messageList.size());
        InstantMessage receivedMessage = new InstantMessage(MOCK_RECEIVED_MESSAGE_ID,
                MOCK_REMOTE_USER, MOCK_MESSAGE_CONTENT, false, new Date());
        mGroupChatWindow.addReceivedMessage(receivedMessage, false);
        getInstrumentation().waitForIdleSync();
        assertEquals(1, messageList.size());

        // Test addSentMessage()
        messageList.clear();
        assertEquals(0, messageList.size());
        InstantMessage sentMessage = new InstantMessage(MOCK_SENT_MESSAGE_ID, MOCK_REMOTE_USER,
                MOCK_MESSAGE_CONTENT, false, new Date());
        mGroupChatWindow.addSentMessage(sentMessage, MOCK_SENT_MESSAGE_TAG);
        getInstrumentation().waitForIdleSync();
        assertEquals(1, messageList.size());

        // Test getSentChatMessage()
        assertNotNull(mGroupChatWindow.getSentChatMessage(MOCK_SENT_MESSAGE_ID));
        assertNull(mGroupChatWindow.getSentChatMessage(MOCK_SENT_MESSAGE_ID_NOT_EXIST));

        // test other cases
        Object oldGroupChatFragment = mGroupChatFragmentField.get(mGroupChatWindow);
        mGroupChatFragmentField.set(mGroupChatWindow, null);
        assertNull(mGroupChatWindow.getSentChatMessage(MOCK_SENT_MESSAGE_ID));
        IReceivedChatMessage resultReceived = mGroupChatWindow.addReceivedMessage(receivedMessage,
                false);
        getInstrumentation().waitForIdleSync();
        assertNull(resultReceived);
        ISentChatMessage resultSent = mGroupChatWindow.addSentMessage(sentMessage,
                MOCK_SENT_MESSAGE_TAG);
        getInstrumentation().waitForIdleSync();
        assertNull(resultSent);

        Object oldLock = sLockField.get(mGroupChatWindow);
        Object oldParticipants = mParticipantsField.get(mGroupChatWindow);
        sLockField.set(mGroupChatWindow, true);
        mParticipantsField.set(mGroupChatWindow, null);
        resultReceived = mGroupChatWindow.addReceivedMessage(receivedMessage, false);
        getInstrumentation().waitForIdleSync();
        assertNull(resultReceived);
        resultSent = mGroupChatWindow.addSentMessage(sentMessage, MOCK_SENT_MESSAGE_TAG);
        getInstrumentation().waitForIdleSync();
        assertNull(resultSent);

        mGroupChatFragmentField.set(mGroupChatWindow, oldGroupChatFragment);
        sLockField.set(mGroupChatWindow, oldLock);
        mParticipantsField.set(mGroupChatWindow, oldParticipants);

        // Test removeAllMessages()
        Field fieldMessageAdapter = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mMessageAdapter");
        ChatAdapter messageAdapter = (ChatAdapter) fieldMessageAdapter.get(mFragment);
        assertTrue(messageAdapter.getCount() > 0);
        mGroupChatWindow.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        assertEquals(0, messageAdapter.getCount());
        Logger.v(TAG, "testCase2_AddGetRemoveMessage() exit");
    }

    /**
     * Test the function GroupChatWindow#updateParticipants()
     */
    public void testCase3_UpdateParticipants() throws Throwable {
        Logger.v(TAG, "testCase3_UpdateParticipants() entry");
        ArrayList<ParticipantInfo> newParticipantList = new ArrayList<ParticipantInfo>();
        newParticipantList.add(new ParticipantInfo(new Participant(MOCK_NUMBER3, MOCK_NUMBER3),
                User.STATE_CONNECTED));
        newParticipantList.add(new ParticipantInfo(new Participant(MOCK_NUMBER4, MOCK_NUMBER4),
                User.STATE_CONNECTED));
        Field fieldGroupChatParticipantList = Utils.getPrivateField(GroupChatFragment.class,
                "mGroupChatParticipantList");
        List<ParticipantInfo> groupChatParticipantList = (List<ParticipantInfo>) fieldGroupChatParticipantList
                .get(mFragment);
        assertFalse(groupChatParticipantList.equals(newParticipantList));
        mGroupChatWindow.updateParticipants(newParticipantList);
        getInstrumentation().waitForIdleSync();
        assertTrue(groupChatParticipantList.equals(newParticipantList));
        Logger.v(TAG, "testCase3_UpdateParticipants() exit");
    }

    /**
     * Test the function GroupChatWindow#setIsComposing()
     */
    public void testCase4_SetIsComposing() throws Throwable {
        Logger.v(TAG, "testCase4_SetIsComposing() entry");
        Field fieldParticipantComposList = Utils.getPrivateField(GroupChatFragment.class,
                "mParticipantComposList");
        CopyOnWriteArrayList<Participant> participantComposList = (CopyOnWriteArrayList<Participant>) fieldParticipantComposList
                .get(mFragment);
        assertFalse(participantComposList.contains(MOCK_GROUP_PARTICIPANT));
        assertFalse(participantComposList.contains(MOCK_GROUP_PARTICIPANT2));
        mGroupChatWindow.setIsComposing(true, MOCK_GROUP_PARTICIPANT);
        getInstrumentation().waitForIdleSync();
        assertTrue(participantComposList.contains(MOCK_GROUP_PARTICIPANT));
        assertFalse(participantComposList.contains(MOCK_GROUP_PARTICIPANT2));
        mGroupChatWindow.setIsComposing(false, MOCK_GROUP_PARTICIPANT);
        getInstrumentation().waitForIdleSync();
        assertFalse(participantComposList.contains(MOCK_GROUP_PARTICIPANT));
        assertFalse(participantComposList.contains(MOCK_GROUP_PARTICIPANT2));
        Logger.v(TAG, "testCase4_SetIsComposing() exit");
    }

    /**
     * Test the function GroupChatWindow#addUnReadMessage()
     */
    public void testCase5_AddUnReadMessage() throws Throwable {
        Logger.v(TAG, "testCase5_AddUnReadMessage() entry");
        InstantMessage message = new InstantMessage(MOCK_UNREAD_MESSAGE_ID, MOCK_REMOTE_USER,
                MOCK_MESSAGE_CONTENT, false, new Date());
        mGroupChatWindow.addUnreadMessage(message);
        getInstrumentation().waitForIdleSync();

        Field fieldMgToOtherWinReminderText = Utils.getPrivateField(GroupChatFragment.class
                .getSuperclass(), "mMgToOtherWinReminderText");
        final TextView mgToOtherWinReminderText = (TextView) fieldMgToOtherWinReminderText
                .get(mFragment);
        String text = mgToOtherWinReminderText.getText().toString();
        assertTrue(text.contains(MOCK_MESSAGE_CONTENT));
        Logger.v(TAG, "testCase5_AddUnReadMessage() exit");
    }

    /**
     * Test the function GroupChatWindow#addChatEventInformation()
     */
    public void testCase6_AddChatEventInformation() throws Throwable {
        Logger.v(TAG, "testCase6_AddChatEventInformation() entry");
        Field fieldMessageAdapter = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mMessageAdapter");
        ChatAdapter chatAdapter = (ChatAdapter) fieldMessageAdapter.get(mFragment);
        assertNotNull(chatAdapter);
        int startCount = chatAdapter.getCount();
        ChatEventStruct chatEventStruct = new ChatEventStruct(Information.JOIN, new Object(),
                new Date());
        mGroupChatWindow.addChatEventInformation(chatEventStruct);
        getInstrumentation().waitForIdleSync();
        int currentCount = chatAdapter.getCount();
        assertTrue(currentCount > startCount);

        Field mGroupChatFragmentField = Utils.getPrivateField(GroupChatWindow.class,
                "mGroupChatFragment");
        Object oldGroupChatFragment = mGroupChatFragmentField.get(mGroupChatWindow);
        mGroupChatFragmentField.set(mGroupChatWindow, null);
        IChatEventInformation result = mGroupChatWindow.addChatEventInformation(chatEventStruct);
        getInstrumentation().waitForIdleSync();
        assertNull(result);

        Field sLockField = Utils.getPrivateField(GroupChatWindow.class, "sLock");
        Object oldLock = sLockField.get(mGroupChatWindow);
        Field mParticipantsField = Utils.getPrivateField(GroupChatWindow.class, "mParticipants");
        Object oldParticipants = mParticipantsField.get(mGroupChatWindow);
        sLockField.set(mGroupChatWindow, true);
        mParticipantsField.set(mGroupChatWindow, null);
        result = mGroupChatWindow.addChatEventInformation(chatEventStruct);
        getInstrumentation().waitForIdleSync();
        assertNull(result);

        mGroupChatFragmentField.set(mGroupChatWindow, oldGroupChatFragment);
        sLockField.set(mGroupChatWindow, oldLock);
        mParticipantsField.set(mGroupChatWindow, oldParticipants);

        Logger.v(TAG, "testCase6_AddChatEventInformation() exit");
    }

    /**
     * Test the function GroupChatWindow#setIsRejoining()
     */
    public void testCase7_SetIsRejoining() throws Throwable {
        Logger.v(TAG, "testCase7_SetIsRejoining() entry");
        Field fieldSendButton = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                SEND_BUTTON);

        getInstrumentation().waitForIdleSync();
        ImageButton sendButton = (ImageButton) fieldSendButton.get(mFragment);
        assertFalse(sendButton.isClickable());

        getInstrumentation().waitForIdleSync();
        sendButton = (ImageButton) fieldSendButton.get(mFragment);
        assertTrue(sendButton.isClickable());
        Logger.v(TAG, "testCase7_SetIsRejoining() exit");
    }

    public void testCase8_AddContactsWhenParticipantAlreadyMax() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.v(TAG, "testCase8_AddContactsWhenParticipantAlreadyMax() entry");
        addParticipantsToMax();
        mGroupChatWindow.addContacts();
        assertEquals(true, mFragment.isMaxGroupChatParticipantsWork());
    }

    public void testCase9_clearHistory() throws Throwable {
        Field mGroupChatFragmentField = Utils.getPrivateField(GroupChatWindow.class,
                "mGroupChatFragment");
        Object oldGroupChatFragment = mGroupChatFragmentField.get(mGroupChatWindow);
        assertNotNull(oldGroupChatFragment);
        boolean result = mGroupChatWindow.clearHistory();
        assertTrue(result);

        mGroupChatFragmentField.set(mGroupChatWindow, null);
        result = mGroupChatWindow.clearHistory();
        assertFalse(result);
        mGroupChatFragmentField.set(mGroupChatWindow, oldGroupChatFragment);
    }

    private void addParticipantsToMax() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.v(TAG, "addParticipantsToMax() entry");
        int maxParticipants = RcsSettings.getInstance().getMaxChatParticipants();
        ArrayList<ParticipantInfo> newParticipantList = new ArrayList<ParticipantInfo>();
        int number = 246;
        for (int i = 0; i < maxParticipants; ++i) {
            number += i;
            String numberString = "+34200000" + number;
            newParticipantList.add(new ParticipantInfo(new Participant(numberString, numberString),
                    User.STATE_CONNECTED));
        }
        Field fieldGroupChatParticipantList = Utils.getPrivateField(GroupChatFragment.class,
                "mGroupChatParticipantList");
        List<ParticipantInfo> groupChatParticipantList = (List<ParticipantInfo>) fieldGroupChatParticipantList
                .get(mFragment);
        groupChatParticipantList.clear();
        assertEquals(0, groupChatParticipantList.size());
        mGroupChatWindow.updateParticipants(newParticipantList);
        getInstrumentation().waitForIdleSync();
        assertEquals(maxParticipants, groupChatParticipantList.size());
    }

    private ChatImpl waitForChat() throws InterruptedException {
        long startTime = System.currentTimeMillis();
        ChatImpl chat = null;
        do {
            List<IChat> chatList = ModelImpl.getInstance().listAllChat();
            if (chatList.size() == 1) {
                chat = (ChatImpl) chatList.get(0);
            }
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (null == chat);
        return chat;
    }

    private Intent generateIntent() {
        Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(MOCK_GROUP_PARTICIPANT);
        participants.add(MOCK_GROUP_PARTICIPANT2);
        intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    private void waitForChatWindowManagerNum(int expectedNum) throws InterruptedException {
        long startTime = System.currentTimeMillis();
        int chatWindowManagerNum = mChatWindowManagerList.size();
        Logger.d(TAG, "expectedNum is " + expectedNum + ", chatWindowManagerNum is "
                + chatWindowManagerNum);
        do {
            chatWindowManagerNum = mChatWindowManagerList.size();
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (chatWindowManagerNum != expectedNum);
    }
}
