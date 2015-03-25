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
import android.os.Bundle;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment.ChatEventInformation;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * This class is used to test ChatEventItemBinder
 */
public class ChatEventItemBinderTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "ChatEventItemBinderTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_NUMBER = "+340200000254";
    private static final String MOCK_NUMBER2 = "+340200000254";
    private static final String MOCK_USER = "Mr.Mock";
    private static final String USER_LEFT = "Mr.Left";
    private static final String USER_INVITER = "Mr.Inviter";
    // INFO_XXX is a keyword from res/values/chat_strings.xml
    private static final String INFO_JOINED = "joins in"; // From R.id.join_group_chat
    private static final String INFO_INVITE = "You invite"; // From R.id.invite_into_groupchat_title
    private static final String INFO_INVITE2 = "invite you and"; // From R.id.accept_groupchat_title
    private static final String INFO_QUIT = "You quit the group chat at"; // From R.id.quit_group_chat
    private static final String INFO_REJOIN = "You rejoined the group chat at"; // From R.id.rejoin_group_chat
    private static final String INFO_LEFT = "has left"; // From R.id.leave_group_chat
    private static final String INFO_NOBODY = "No participant joined the chat"; // From R.id.nobody_join
    private static final Participant MOCK_GROUP_PARTICIPANT = new Participant(MOCK_NUMBER,
            MOCK_NUMBER);
    private static final Participant MOCK_GROUP_PARTICIPANT2 = new Participant(MOCK_NUMBER2,
            MOCK_NUMBER2);
    private static final int MOCK_PARTICIPANT_NUM = 2;
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private static final int DATE_LABEL_POSITION = 0;
    private static final int CURRENT_POSITION = 1;
    private ChatScreenActivity mActivity = null;
    private List mChatWindowManagerList = null;
    private GroupChatFragment mFragment = null;
    private ChatImpl mChat = null;

    public ChatEventItemBinderTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() enter");
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

        mChat = waitForChat();
        mFragment = (GroupChatFragment) mActivity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        assertEquals(MOCK_PARTICIPANT_NUM, mFragment.getParticipantsNum());
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
     * Test the function ChatEventItemBinder#bindView()
     */
    public void testCase1_BindView() throws Throwable {
        Logger.v(TAG, "testCase1_BindView() enter");
        Field fieldMessageListView = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mMessageListView");
        ListView messageListView = (ListView) fieldMessageListView.get(mFragment);

        // JOIN
        String joinInfo = getChatEventInformationText(mFragment, messageListView, Information.JOIN,
                MOCK_USER);
        assertTrue(joinInfo.contains(MOCK_USER));
        assertTrue(joinInfo.contains(INFO_JOINED));
        mFragment.removeAllMessages();
        getInstrumentation().waitForIdleSync();

        // LEFT
        String leftInfo = getChatEventInformationText(mFragment, messageListView, Information.LEFT,
                USER_LEFT);
        assertTrue(leftInfo.contains(USER_LEFT));
        assertTrue(leftInfo.contains(INFO_LEFT));
        mFragment.removeAllMessages();
        getInstrumentation().waitForIdleSync();

        Logger.v(TAG, "testCase1_BindView() exit");
    }

    /**
     * Get ChatEventItem view's text string
     * 
     * @param fragment view's fragment
     * @param messageListView ChatAdapter's ListView
     * @param info Information value
     * @param relatedInfo the object put in ChatEventStruct
     * @return event text string
     */
    private String getChatEventInformationText(GroupChatFragment fragment,
            ListView messageListView, Information info, Object relatedInfo)
            throws NoSuchFieldException, IllegalAccessException, InterruptedException {
        Logger.v(TAG, "getChatEventInformationText() enter");
        ChatEventStruct chatEventStruct = new ChatEventStruct(info, relatedInfo, new Date());
        ChatEventInformation chatEventInformation = (ChatEventInformation) fragment
                .addChatEventInformation(chatEventStruct);
        getInstrumentation().waitForIdleSync();
        Field fieldMessageAdapter = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mMessageAdapter");
        ChatAdapter chatAdapter = (ChatAdapter) fieldMessageAdapter.get(fragment);
        assertNotNull(chatAdapter);
        assertEquals(2, chatAdapter.getCount());
        assertEquals(ChatAdapter.ITEM_TYPE_DATE_LABEL, chatAdapter
                .getItemViewType(DATE_LABEL_POSITION));
        assertEquals(ChatAdapter.ITEM_TYPE_CHAT_EVENT_INFORMATION, chatAdapter
                .getItemViewType(CURRENT_POSITION));
        getInstrumentation().waitForIdleSync();
        View itemView = chatAdapter.getView(CURRENT_POSITION, null, messageListView);
        TextView itemTextView = (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        String chatEventInformationText = itemTextView.getText().toString();
        Logger.d(TAG, "getChatEventInformationText() exit, and chatEventInformationText is "
                + chatEventInformationText);
        return chatEventInformationText;
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
        do {
            chatWindowManagerNum = mChatWindowManagerList.size();
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (chatWindowManagerNum != expectedNum);
    }
}
