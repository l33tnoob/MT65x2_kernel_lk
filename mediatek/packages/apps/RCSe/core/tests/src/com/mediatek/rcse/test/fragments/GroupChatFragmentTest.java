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

package com.mediatek.rcse.test.fragments;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.ParcelUuid;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestCase;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.RelativeLayout;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.fragments.GroupChatFragment.ChatEventInformation;
import com.mediatek.rcse.fragments.GroupChatFragment.SentMessage;
import com.mediatek.rcse.fragments.GroupChatFragment.ReceivedMessage;
import com.orangelabs.rcs.R;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test the functions of ComposingManager in Model part
 */
public class GroupChatFragmentTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "GroupChatFragmentTest";
    private static final int GROUP_NUM = 5;
    private GroupChatFragment mGroupChatFragment = null;
    private static final String METHOD_SEND_MESSAGE = "onSentMessage";
    private static final String DEFAULT_MESSAGE = "testMessage";
    private static final String PRE_MESSAGE_MAP = "mPreMessageMap";
    private static final String FIELD_MESSAGE_ADAPTER = "mMessageAdapter";
    private static final String METHOD_ADD_CONTACTS = "addContactsToGroupChat";
    private static final String CONTACT_NUMBER1 = "+34200000231";
    private static final String CONTACT_NUMBER2 = "+34200000251";
    private static final String CONTACT_NUMBER3 = "+34200000003";
    private static final String FIELD_CURRENT_PARTICIPANTS = "mCurrentParticipants";
    private static final String FIELD_TAG = "mTag";
    private static final String CHAT_MAP = "mChatMap";
    private static final String METHOD_ADDCONTACTS_TOCHATWINDOW = "addContactsToExistChatWindow";
    private static final String METHOD_ONACTIVITYRESULT = "onActivityResult";
    private static final String METHOD_ADD_RECDMESSAGE = "addReceivedMessage";
    private static final String METHOD_ADD_SENTMESSAGE = "addSentMessage";
    private static final String FIELD_PREMESSAGE_MAP = "mPreMessageMap";
    private static final String FIELD_MESSAGE_LIST = "mMessageList";
    private static final String FIELD_MAINTHREAD_ID = "THREAD_ID_MAIN";
    private static final String FIELD_IS_BOTTOM = "mIsBottom";
    private static final String METHOD_ON_SEND = "onSend";
    private static final String TEST_MESSAGE = "test message";
    private static final String FIELD_PREMSG_MAP = "mPreMessageMap";
    private static final String METHOD_REMOVE_CHATUI = "removeChatUi";
    private static final String METHOD_SET_COMPOSING = "setIsComposing";
    private static final String FIELD_COMPOS_LIST = "mParticipantComposList";
    private static final String METHOD_ONCLICK = "onClick";
    private static final String FIELD_PORTRAIT_STRATEGY = "mPortraitStrategy";
    private static final String FIELD_DISPLAYER = "mParticipantListDisplayer";
    private static final String FIELD_CURRENT_STRATEGY = "mCurrentStrategy";
    private static final String FIELD_EXPAND = "mIsExpand";
    private static final int TIME_OUT = 10000;

    public GroupChatFragmentTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.getActivity().getApplicationContext());
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        CopyOnWriteArrayList<ParticipantInfo> participantList = new CopyOnWriteArrayList<ParticipantInfo>();
        for (int i = 0; i < GROUP_NUM; ++i) {
            Participant participant = new Participant("+861898092818" + i, "+861898092818" + i);
            participantList.add(new ParticipantInfo(participant, User.STATE_PENDING));
        }
        final GroupChatWindow groupChatWindow = new GroupChatWindow(parcelUuid, participantList);
        mGroupChatFragment = groupChatWindow.getFragment();
        mGroupChatFragment.setTag(parcelUuid);
        mGroupChatFragment.setParticipantList(participantList);
    }

    /*
     * Test the display name
     */
    public void testCase01_getStrangerParticipantsName() {
        Logger.v(TAG, "testCase1_getParticipantsName");
        ArrayList<Participant> participantList = new ArrayList<Participant>();
        for (int i = 0; i < GROUP_NUM; ++i) {
            Participant participant = new Participant("+861898092818" + i, "+861898092818" + i);
            participantList.add(participant);
        }
        String displayName = ChatFragment.getParticipantsName(participantList
                .toArray(new Participant[1]));

        final StringBuilder stringBuilder = new StringBuilder();
        for (int i = 0; i < GROUP_NUM; ++i) {
            stringBuilder.append("+861898092818" + i);
            stringBuilder.append(",");
        }
        int length = stringBuilder.length();
        if (length > 1) {
            stringBuilder.deleteCharAt(length - 1);
        }
        assertEquals(displayName, stringBuilder.toString());
    }

    /**
     * Test the participant's number
     */
    public void testCase02_getParticipantsNum() {
        Logger.v(TAG, "testCase2_getParticipantsNum");
        if (mGroupChatFragment != null) {
            int num = mGroupChatFragment.getParticipantsNum();
            assertEquals(num, GROUP_NUM);
        } else {
            Logger.w(TAG, "testCase2_getParticipantsNum mGroupChatFragment is null.");
        }
    }

    /**
     * Test the getEmotionsVisibility() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_getEmotionsVisibility() throws Throwable, IllegalAccessException,
            NoSuchMethodException, IllegalArgumentException, InvocationTargetException {
        Logger.d(TAG, "testCase3_getEmotionsVisibility()");
        assertNotNull("mGroupChatFragment is null", mGroupChatFragment);
        final Activity activity = this.getActivity();
        final Method methodShowImm = mGroupChatFragment.getClass().getSuperclass()
                .getDeclaredMethod("showImm", Boolean.class);
        methodShowImm.setAccessible(true);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    ((ChatScreenActivity) activity).addGroupChatUi(mGroupChatFragment, null);
                    methodShowImm.invoke(mGroupChatFragment, false);
                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }
        });
        assertTrue(mGroupChatFragment.getEmotionsVisibility() == View.VISIBLE);
    }

    /**
     * Test the hideEmotions() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase04_hideEmotions() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase4_hideEmotions()");
        assertNotNull("mGroupChatFragment is null", mGroupChatFragment);
        final Activity activity = this.getActivity();
        final Method methodShowImm = mGroupChatFragment.getClass().getSuperclass()
                .getDeclaredMethod("showImm", Boolean.class);
        methodShowImm.setAccessible(true);
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                ((ChatScreenActivity) activity).addGroupChatUi(mGroupChatFragment, null);
                // UI operation must be in UI thread, and here cannot throw
                // exception , so catch it
                try {
                    methodShowImm.invoke(mGroupChatFragment, false);
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
                mGroupChatFragment.hideEmotions();
                assertTrue((mGroupChatFragment).getEmotionsVisibility() == View.GONE);
            }
        });

    }

    /**
     * Test case for on send message.
     * 
     * @throws Throwable
     * @throws InterruptedException
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase05_onSentMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase5_onSentMessage()");
        final Field preMessageField = mGroupChatFragment.getClass().getSuperclass()
                .getDeclaredField(PRE_MESSAGE_MAP);
        preMessageField.setAccessible(true);
        Map<Integer, ISentChatMessage> map = (Map<Integer, ISentChatMessage>) preMessageField
                .get(mGroupChatFragment);
        int beforeSize = map.size();
        final Field messageAdapterField = mGroupChatFragment.getClass().getSuperclass()
                .getDeclaredField(FIELD_MESSAGE_ADAPTER);
        messageAdapterField.setAccessible(true);
        messageAdapterField.set(mGroupChatFragment, null);
        final Method sendMessage = mGroupChatFragment.getClass().getDeclaredMethod(
                METHOD_SEND_MESSAGE, String.class);
        sendMessage.setAccessible(true);
        sendMessage.invoke(mGroupChatFragment, DEFAULT_MESSAGE);
        preMessageField.setAccessible(true);
        map = (Map<Integer, ISentChatMessage>) preMessageField.get(mGroupChatFragment);
        int afterSize = map.size();
        assertEquals(afterSize, beforeSize);
    }

    /**
     * Test the addContactsToGroupChat() & addContactsToExistChatWindow() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase06_addContactsToGroupChat() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final Activity activity = this.getActivity();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                ((ChatScreenActivity) activity).addGroupChatUi(mGroupChatFragment, null);
            }
        });

        // addContactsToGroupChat
        Method method = Utils.getPrivateMethod(mGroupChatFragment.getClass(), METHOD_ADD_CONTACTS,
                ArrayList.class);
        Field fieldTag = Utils.getPrivateField(mGroupChatFragment.getClass().getSuperclass(),
                FIELD_TAG);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        MockGroupChatWindow groupWindow = new MockGroupChatWindow();
        ArrayList<Participant> list = new ArrayList<Participant>();
        Participant participant1 = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        Participant participant2 = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER2);
        list.add(participant1);
        list.add(participant2);
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        ChatImpl chat = new MockGroupChat(modelInstance, groupWindow, list, tag);
        fieldTag.set(mGroupChatFragment, tag);
        chatMap.put(tag, chat);
        method.invoke(mGroupChatFragment, list);

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (((GroupChat) chatMap.get(tag)).getParticipantInfos().size() == 2) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // addContactsToExistChatWindow
        method = Utils.getPrivateMethod(GroupChatFragment.class, METHOD_ADDCONTACTS_TOCHATWINDOW,
                List.class);
        chatMap.clear();
        method.invoke(mGroupChatFragment, new ArrayList<Participant>());
        assertTrue(chatMap.size() == 0);

        chatMap.put(tag, chat);
        method.invoke(mGroupChatFragment, list);
        beginTime = System.currentTimeMillis();
        success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            GroupChat groupChat = (GroupChat) chatMap.get(tag);
            if (groupChat != null && groupChat.getParticipantInfos().size() == 2) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test the onActivityResult() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase07_onActivityResult() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final Activity activity = this.getActivity();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                ((ChatScreenActivity) activity).addGroupChatUi(mGroupChatFragment, null);
            }
        });

        Field fieldTag = Utils.getPrivateField(mGroupChatFragment.getClass().getSuperclass(),
                FIELD_TAG);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        ModelImpl modelInstance = (ModelImpl) ModelImpl.getInstance();
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modelInstance);
        chatMap.clear();
        MockGroupChatWindow groupWindow = new MockGroupChatWindow();
        ArrayList<Participant> list = new ArrayList<Participant>();
        Participant participant1 = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        Participant participant2 = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER2);
        list.add(participant1);
        list.add(participant2);
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        ChatImpl chat = new MockGroupChat(modelInstance, groupWindow, list, tag);
        fieldTag.set(mGroupChatFragment, tag);
        chatMap.put(tag, chat);

        Method method = Utils.getPrivateMethod(mGroupChatFragment.getClass(),
                METHOD_ONACTIVITYRESULT, int.class, int.class, Intent.class);
        Intent intent = new Intent();
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(participant);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        method.invoke(mGroupChatFragment, ChatFragment.RESULT_CODE_ADD_CONTACTS, 0, intent);

        long beginTime = System.currentTimeMillis();
        boolean success = false;
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            GroupChat groupChat = (GroupChat) chatMap.get(tag);
            if (groupChat != null && groupChat.getParticipantInfos().size() == 2) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test method: addSentMessage
     */
    public void testCase08_addSentMessage() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        getInstrumentation().waitForIdleSync();
        final Method method = Utils.getPrivateMethod(GroupChatFragment.class,
                METHOD_ADD_SENTMESSAGE, InstantMessage.class, int.class);
        Field fieldPreMessageMap = Utils.getPrivateField(mGroupChatFragment.getClass()
                .getSuperclass(), FIELD_PREMESSAGE_MAP);
        Map<Integer, ISentChatMessage> preMessageMap = (Map<Integer, ISentChatMessage>) fieldPreMessageMap
                .get(mGroupChatFragment);
        preMessageMap.clear();
        Field fieldMessageList = Utils.getPrivateField(mGroupChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        List<?> messageList = (List<?>) fieldMessageList.get(mGroupChatFragment);
        int sizeBefore = messageList.size();
        InstantMessage msg = new InstantMessage("", "", "", true);

        // not find in preMessageMap
        method.invoke(mGroupChatFragment, msg, -1);
        getInstrumentation().waitForIdleSync();
        int sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        // find in preMessageMap
        preMessageMap.put(-1, (ISentChatMessage) method.invoke(mGroupChatFragment, msg, -1));
        method.invoke(mGroupChatFragment, msg, -1);
        getInstrumentation().waitForIdleSync();
        assertFalse(preMessageMap.containsKey(-1));

        // null msg.
        Object result = method.invoke(mGroupChatFragment, (InstantMessage) null, -1);
        getInstrumentation().waitForIdleSync();
        assertNull(result);
    }

    /**
     * Test the addReceivedMessage() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase09_addReceivedMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = this.getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        getInstrumentation().waitForIdleSync();
        final Method method = Utils.getPrivateMethod(GroupChatFragment.class,
                METHOD_ADD_RECDMESSAGE, InstantMessage.class, boolean.class);
        final InstantMessage msg = new InstantMessage("", "", "", true);

        // null msg
        Object result = method.invoke(mGroupChatFragment, (InstantMessage) null, false);
        assertNull(result);

        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mGroupChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);

        Field fieldBottom = Utils.getPrivateField(mGroupChatFragment.getClass().getSuperclass(),
                FIELD_IS_BOTTOM);
        fieldBottom.setBoolean(mGroupChatFragment, false);
        Field fieldMessageList = Utils.getPrivateField(mGroupChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        List<?> messageList = (List<?>) fieldMessageList.get(mGroupChatFragment);
        messageList.clear();

        int sizeBefore = messageList.size();
        // main thread.
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                try {
                    method.invoke(mGroupChatFragment, msg, false);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        assertEquals(messageList.size(), sizeBefore + 1);

        // not main thread.
        messageList.clear();
        sizeBefore = messageList.size();
        Field fieldThreadId = Utils.getPrivateField(mGroupChatFragment.getClass().getSuperclass(),
                FIELD_MAINTHREAD_ID);
        fieldThreadId.setLong(mGroupChatFragment, 2);
        method.invoke(mGroupChatFragment, msg, false);
        assertEquals(messageList.size(), sizeBefore + 1);

        fieldAdapter.set(mGroupChatFragment, null);
        result = method.invoke(mGroupChatFragment, msg, false);
        assertNull(result);
    }

    /**
     * Test method: onSend().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase10_onSend() throws Throwable, InterruptedException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        Field fieldPreMsgMap = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                FIELD_PREMSG_MAP);
        Map<?, ?> preFtMap = (Map<?, ?>) fieldPreMsgMap.get(mGroupChatFragment);
        int sizeBefore = preFtMap.size();
        final Method methodSend = Utils.getPrivateMethod(GroupChatFragment.class, METHOD_ON_SEND,
                String.class);
        methodSend.invoke(mGroupChatFragment, TEST_MESSAGE);
        int sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        sizeBefore = preFtMap.size();
        methodSend.invoke(mGroupChatFragment, TEST_MESSAGE);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);
    }

    /**
     * Test method: removeChatUi().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase11_removeChatUi() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        final Method methodSend = Utils.getPrivateMethod(GroupChatFragment.class,
                METHOD_REMOVE_CHATUI);
        // This method will operate UI
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    methodSend.invoke(mGroupChatFragment);
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
                assertNull(getActivity().getActionBar().getCustomView());
            }

        });
    }

    /**
     * Test method: setIsComposing().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase12_setIsComposing() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        Field field = Utils.getPrivateField(GroupChatFragment.class, FIELD_COMPOS_LIST);
        List<?> list = (List<?>) field.get(mGroupChatFragment);
        list.clear();
        final Method method = Utils.getPrivateMethod(GroupChatFragment.class, METHOD_SET_COMPOSING,
                boolean.class, Participant.class);
        method.invoke(mGroupChatFragment, true, participant);
        assertTrue(list.size() == 1);

        method.invoke(mGroupChatFragment, false, participant);
        assertTrue(list.size() == 0);

        method.invoke(mGroupChatFragment, false, participant);
        assertTrue(list.size() == 0);

        method.invoke(mGroupChatFragment, false, null);
        assertTrue(list.size() == 0);

        method.invoke(mGroupChatFragment, true, null);
        assertTrue(list.size() == 0);
    }

    /**
     * Test method: onClick().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase13_onClick() throws Throwable, InterruptedException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addGroupChatUi(mGroupChatFragment, null);
                    ImageButton expandGroupChat = (ImageButton) activity
                            .findViewById(R.id.group_chat_expand);
                    ImageButton collapaseGroupChat = (ImageButton) activity
                            .findViewById(R.id.group_chat_collapse);
                    RelativeLayout groupChatTitleLayout = (RelativeLayout) activity
                            .findViewById(R.id.group_chat_title_layout);
                    Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
                    Field field = Utils.getPrivateField(GroupChatFragment.class, FIELD_COMPOS_LIST);
                    List<?> list = (List<?>) field.get(mGroupChatFragment);
                    list.clear();
                    final Method method = Utils.getPrivateMethod(GroupChatFragment.class,
                            METHOD_ONCLICK, View.class);
                    method.invoke(mGroupChatFragment, expandGroupChat);
                    Field fieldDisplayer = Utils.getPrivateField(GroupChatFragment.class,
                            FIELD_DISPLAYER);
                    Object displayer = fieldDisplayer.get(mGroupChatFragment);
                    Field fieldCurrentStrategy = Utils.getPrivateField(displayer.getClass(),
                            FIELD_CURRENT_STRATEGY);
                    Object strategy = fieldCurrentStrategy.get(displayer);

                    Class<?>[] classes = GroupChatFragment.class.getDeclaredClasses();
                    Class portraitClass = null;
                    for (Class<?> classz : classes) {
                        if ("PortraitStrategy".equals(classz.getSimpleName())) {
                            portraitClass = classz;
                            break;
                        }
                    }

                    Field fieldExpand = Utils.getPrivateField(portraitClass, FIELD_EXPAND);
                    assertTrue(fieldExpand.getBoolean(strategy));

                    method.invoke(mGroupChatFragment, collapaseGroupChat);
                    boolean status = fieldExpand.getBoolean(strategy);
                    assertFalse(status);

                    method.invoke(mGroupChatFragment, groupChatTitleLayout);
                    assertEquals(fieldExpand.getBoolean(strategy), !status);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test method: LandscapeStrategy().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase14_LandscapeStrategy() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addGroupChatUi(mGroupChatFragment, null);
                    Field fieldDisplayer = Utils.getPrivateField(GroupChatFragment.class,
                            FIELD_DISPLAYER);
                    Object displayer = fieldDisplayer.get(mGroupChatFragment);
                    Field fieldLandScape = Utils.getPrivateField(displayer.getClass(),
                            "mLandscapeStrategy");
                    Object landScape = fieldLandScape.get(displayer);
                    Method methodCheckArea = Utils.getPrivateMethod(landScape.getClass(),
                            "checkArea");
                    methodCheckArea.invoke(landScape);
                    Field fieldInflator = Utils.getPrivateField(landScape.getClass(), "mInflator");
                    assertNotNull(fieldInflator.get(landScape));

                    Field fieldList = Utils.getPrivateField(landScape.getClass(),
                            "mParticipantInfoList");
                    ArrayList<ParticipantInfo> list = new ArrayList<ParticipantInfo>();
                    Participant participant1 = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER1);
                    Participant participant2 = new Participant(CONTACT_NUMBER3, CONTACT_NUMBER1);
                    list.add(new ParticipantInfo(participant1, User.STATE_ALERTING));
                    list.add(new ParticipantInfo(participant2, User.STATE_ALERTING));
                    fieldList.set(landScape, list);

                    Method method = Utils.getPrivateMethod(landScape.getClass(), "getView",
                            int.class, View.class, ViewGroup.class);
                    Object result = method.invoke(landScape, 0, (View) null, (ViewGroup) null);
                    assertNotNull(result);

                    method = Utils.getPrivateMethod(landScape.getClass(), "getItemId", int.class);
                    result = method.invoke(landScape, 0);
                    assertEquals(result, 0l);

                    method = Utils.getPrivateMethod(landScape.getClass(), "getItem", int.class);
                    method.invoke(landScape, 0);

                    method = Utils.getPrivateMethod(landScape.getClass(), "getCount");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "show");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "expand");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "dismiss");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "collapse");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "updateBanner",
                            List.class);
                    method.invoke(landScape, list);
                    assertNotNull(fieldList.get(landScape));
                } catch (Throwable e) {
                    e.printStackTrace();
                }

            }
        });
    }

    /**
     * Test method: PortraitStrategy().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase15_PortraitStrategy() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addGroupChatUi(mGroupChatFragment, null);

                    ArrayList<ParticipantInfo> list = new ArrayList<ParticipantInfo>();
                    Participant participant1 = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER1);
                    Participant participant2 = new Participant(CONTACT_NUMBER3, CONTACT_NUMBER1);
                    list.add(new ParticipantInfo(participant1, User.STATE_ALERTING));
                    list.add(new ParticipantInfo(participant2, User.STATE_ALERTING));
                    Field fieldDisplayer = Utils.getPrivateField(GroupChatFragment.class,
                            FIELD_DISPLAYER);
                    Object displayer = fieldDisplayer.get(mGroupChatFragment);
                    Field fieldLandScape = Utils.getPrivateField(displayer.getClass(),
                            "mPortraitStrategy");
                    Object landScape = fieldLandScape.get(displayer);

                    Method method = Utils.getPrivateMethod(landScape.getClass(), "getView",
                            Context.class, ParticipantInfo.class, AsyncImageView.class);
                    Object result = method.invoke(landScape, getActivity(), new ParticipantInfo(
                            participant1, User.STATE_ALERTING), (AsyncImageView) null);
                    assertNotNull(result);

                    method = Utils.getPrivateMethod(landScape.getClass(), "show");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "expand");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "dismiss");
                    method.invoke(landScape);

                    method = Utils.getPrivateMethod(landScape.getClass(), "updateBanner",
                            List.class);
                    method.invoke(landScape, list);

                    method = Utils.getPrivateMethod(landScape.getClass(), "collapse");
                    method.invoke(landScape);
                    Field fieldExpand = Utils.getPrivateField(landScape.getClass(), "mIsExpand");
                    assertFalse(fieldExpand.getBoolean(landScape));
                } catch (Throwable e) {
                    e.printStackTrace();
                }

            }
        });
    }

    /**
     * Test method: onScreenSwitched().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase16_onScreenSwitched() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addGroupChatUi(mGroupChatFragment, null);
                    Field fieldDisplayer = Utils.getPrivateField(GroupChatFragment.class,
                            FIELD_DISPLAYER);
                    Object displayer = fieldDisplayer.get(mGroupChatFragment);

                    Method method = Utils.getPrivateMethod(displayer.getClass(),
                            "onScreenSwitched", int.class);
                    method.invoke(displayer, Configuration.ORIENTATION_LANDSCAPE);
                    Field fieldcurrentStrategy = Utils.getPrivateField(displayer.getClass(),
                            "mCurrentStrategy");
                    Field fieldLandscapeStrategy = Utils.getPrivateField(displayer.getClass(),
                            "mLandscapeStrategy");
                    Field fieldPortraitStrategy = Utils.getPrivateField(displayer.getClass(),
                            "mPortraitStrategy");
                    assertEquals(fieldcurrentStrategy.get(displayer),
                            fieldLandscapeStrategy.get(displayer));

                    method.invoke(displayer, Configuration.ORIENTATION_PORTRAIT);
                    assertEquals(fieldcurrentStrategy.get(displayer),
                            fieldPortraitStrategy.get(displayer));
                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test method: SentMessage().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase17_SentMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        Class<?>[] classes = GroupChatFragment.class.getDeclaredClasses();
        Constructor<?> sentMessageCtr = null;
        for (Class<?> classz : classes) {
            if ("SentMessage".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        sentMessageCtr = ctrz;
                        break;
                    }
                }
            }
        }
        Date date = new Date();
        InstantMessage msg = new InstantMessage("", CONTACT_NUMBER2, "", true, date);
        InstantMessage msg2 = new InstantMessage(null, null, null, false, null);
        SentMessage sentMessage = (SentMessage) sentMessageCtr.newInstance(mGroupChatFragment, msg);
        assertEquals(sentMessage.getId(), "");
        sentMessage.updateStatus(Status.DELIVERED);
        getInstrumentation().waitForIdleSync();
        assertEquals(sentMessage.getStatus(), Status.DELIVERED);
        assertEquals(sentMessage.getMessageText(), "");
        assertEquals(sentMessage.getMessageDate(), date);
        Method method = Utils.getPrivateMethod(sentMessage.getClass(), "updateMessage",
                InstantMessage.class);
        method.invoke(sentMessage, (InstantMessage) null);
        sentMessage.updateDate((Date) null);
        assertNull(sentMessage.getId());
        assertNull(sentMessage.getMessageText());
        assertNull(sentMessage.getMessageDate());
    }

    /**
     * Test method: ReceivedMessage().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase18_ReceivedMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        Date date = new Date();
        InstantMessage msg = new InstantMessage("", CONTACT_NUMBER2, "", true, date);
        InstantMessage msg2 = new InstantMessage(null, null, null, false, null);
        ReceivedMessage sentMessage = new ReceivedMessage(msg);
        assertEquals(sentMessage.getId(), "");
        getInstrumentation().waitForIdleSync();
        assertEquals(sentMessage.getStatus(), Status.SENDING);
        assertEquals(sentMessage.getMessageText(), "");
        assertEquals(sentMessage.getMessageDate(), date);
        assertEquals(sentMessage.getMessageSender(), CONTACT_NUMBER2);

        sentMessage = new ReceivedMessage(null);
        assertNull(sentMessage.getId());
        assertNull(sentMessage.getMessageText());
        assertNull(sentMessage.getMessageSender());
        assertNull(sentMessage.getMessageDate());
    }

    /**
     * Test method: ChatEventInformation().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase19_ChatEventInformation() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addGroupChatUi(mGroupChatFragment, null);
            }
        });
        Class<?>[] classes = GroupChatFragment.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : classes) {
            if ("ChatEventInformation".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        ctr = ctrz;
                        break;
                    }
                }
            }
        }
        ChatEventInformation info = (ChatEventInformation) ctr.newInstance((ChatEventStruct) null);
        assertNull(info.getDate());
        assertNull(info.getInformation());
        assertNull(info.getRelatedInfo());

        ChatEventStruct chatEventStruct = new ChatEventStruct(Information.LEFT, "", new Date());
        info = (ChatEventInformation) ctr.newInstance(chatEventStruct);
        assertNotNull(info.getDate());
        assertNotNull(info.getInformation());
        assertNotNull(info.getRelatedInfo());

        Method method = Utils.getPrivateMethod(mGroupChatFragment.getClass(),
                "addChatEventInformation", ChatEventStruct.class);
        assertNull(method.invoke(mGroupChatFragment, (ChatEventStruct) null));

        method = Utils.getPrivateMethod(mGroupChatFragment.getClass(), "addChatEventInfo",
                IChatEventInformation.class);
        assertNotNull(method.invoke(mGroupChatFragment, info));
    }

    /**
     * Test method: handleShowReminder().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase20_handleShowReminder() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addGroupChatUi(mGroupChatFragment, null);
                    Method method = Utils.getPrivateMethod(mGroupChatFragment.getClass(), "handleShowReminder",
                            String.class);
                    assertTrue((Boolean) method.invoke(mGroupChatFragment,
                            GroupChatFragment.SHOW_REJOING_MESSAGE_REMINDER));
                    assertTrue((Boolean) method.invoke(mGroupChatFragment,
                            GroupChatFragment.SHOW_IS_TYPING_REMINDER));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        assertNotNull(mGroupChatFragment.getParticipants());
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

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() enter");
        Utils.clearAllStatus();
        super.tearDown();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }
}
