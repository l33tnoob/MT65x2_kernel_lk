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

import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;

import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.plugin.message.IpMessageActivitiesManager;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.plugin.message.PluginReceivedChatMessage;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;

import java.util.ArrayList;
import java.util.List;

/**
 * This class is used to test PluginGroupChatActivity
 */
public class PluginGroupChatActivityTest extends
        ActivityInstrumentationTestCase2<PluginGroupChatActivity> {

    private static final String TAG = "PluginGroupChatActivityTest";
    private PluginGroupChatActivity mActivity = null;
    private static final String MOCK_CONTACT_ONE = "+34200000111";
    private static final String MOCK_CONTACT_TWO = "+34200000222";
    private static final Participant MOCK_GROUP_PARTICIPANT_ONE =
            new Participant(MOCK_CONTACT_ONE, MOCK_CONTACT_ONE);
    private static final Participant MOCK_GROUP_PARTICIPANT_TWO =
            new Participant(MOCK_CONTACT_ONE, MOCK_CONTACT_TWO);
    private static final int TIME_OUT = 4000;
    private static final int SLEEP_TIME = 200;
    private static final String MOCK_RECEIVED_MESSAGE_ID = "mock received message id";

    public PluginGroupChatActivityTest() {
        super(PluginGroupChatActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp() entry");
        AndroidFactory.setApplicationContext(getInstrumentation().getTargetContext());
        Logger.d(TAG, "setUp() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.d(TAG, "tearDown() exit");
    }

    private Intent generateIntent() {
        Logger.d(TAG, "generateIntent() entry");
        Intent intent =
                new Intent(getInstrumentation().getTargetContext(), ChatScreenActivity.class);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(MOCK_GROUP_PARTICIPANT_ONE);
        participants.add(MOCK_GROUP_PARTICIPANT_TWO);
        intent.setAction(PluginGroupChatActivity.ACTION);
        intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Logger.d(TAG, "generateIntent() exit with the intent is " + intent);
        return intent;
    }

    /**
     * Test to get the group chat fragment
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InterruptedException
     */
    public void testCase1_GetGroupChatFragment() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase1_GetGroupChatFragment() entry");
        Intent intent = generateIntent();
        setActivityIntent(intent);
        mActivity = getActivity();
        assertNotNull(mActivity);
        ChatImpl groupChat = waitForGroupChat();
        Logger.d(TAG, "testCase1_GetGroupChatFragment() groupChat is " + groupChat);
        waitForFragment(groupChat.getChatTag());
        mActivity.updateParticipants(null);
        mActivity.onPhotoChanged();
        mActivity.finish();
        Logger.d(TAG, "testCase1_GetGroupChatFragment() exit");
    }

    private ChatImpl waitForGroupChat() throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "waitForGroupChat() entry");
        long startTime = System.currentTimeMillis();
        ChatImpl groupChat = null;
        do {
            List<IChat> chatList = ModelImpl.getInstance().listAllChat();
            int size = chatList.size();
            if (size == 1) {
                groupChat = (ChatImpl) chatList.get(0);
            }
            checkTimeOut(startTime);
        } while (null == groupChat);
        Logger.d(TAG, "waitForGroupChat() exit");
        return groupChat;
    }

    private void waitForFragment(Object tag) throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        GroupChatFragment fragment = (GroupChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        Logger.d(TAG, "waitForFragement() entry");
        long startTime = System.currentTimeMillis();
        while (fragment == null) {
            fragment = (GroupChatFragment) mActivity.getFragmentManager().findFragmentById(
                    R.id.chat_content);
            checkTimeOut(startTime);
        }
        Logger.d(TAG, "waitForFragement() exit");
    }

    private void checkTimeOut(long startTime) throws InterruptedException {
        if (System.currentTimeMillis() - startTime > TIME_OUT) {
            fail();
        }
        Thread.sleep(SLEEP_TIME);
    }

    /**
     * Test to enter PluginGroupChatActivity from ConversationList
     * @throws Throwable
     */
    public void testCase2_EnterFromConversationList() throws Throwable {
        Logger.d(TAG, "testCase2_EnterFromConversationList() entry");
        Context targetContext = getInstrumentation().getTargetContext();
        IpMessageActivitiesManager activitiesManager = new IpMessageActivitiesManager(targetContext);
        GroupChatMessageHelper helper = new GroupChatMessageHelper(targetContext);
        ActivityMonitor monitor = new ActivityMonitor(PluginGroupChatActivity.class.getName(), null, false);
        getInstrumentation().addMonitor(monitor);
        try {
            helper.prepareMockMessage();
            helper.setupViewImpl();
            helper.setupGroupChatWindow();
            assertTrue(helper.waitForExpectedMessage());
            PluginReceivedChatMessage receivedMessage = helper.getReceivedChatMessage();
            assertNotNull(receivedMessage);
            assertNotNull(receivedMessage.getPluginChatWindowMessage());
            assertEquals(receivedMessage.getId(), MOCK_RECEIVED_MESSAGE_ID);
            long threadId = helper.getThreadId();
            assertTrue(threadId > -1L);
            Intent intent = new Intent(RemoteActivities.CHAT_DETAILS_BY_THREAD_ID);
            intent.putExtra(RemoteActivities.KEY_THREAD_ID, threadId);
            activitiesManager.startRemoteActivity(targetContext, intent);
            mActivity = (PluginGroupChatActivity) monitor.waitForActivityWithTimeout(TIME_OUT);
            assertNotNull(mActivity);
            waitForFragment(helper.getMockTag());
        } finally {
            helper.clearMockMessage();
            if (null != mActivity) {
                mActivity.finish();
            }
        }
    }
}
