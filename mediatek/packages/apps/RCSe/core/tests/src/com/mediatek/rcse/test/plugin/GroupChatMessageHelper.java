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
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.Telephony.Sms;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginReceivedChatMessage;
import com.mediatek.rcse.service.binder.TagTranslater;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * As a helper to test group chat message
 */
public class GroupChatMessageHelper {
    private static final String TAG = "GroupChatMessageHelper";
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;
    private static final String MOCK_NUMBER = "+3422222244";
    private static final String MOCK_SENT_MESSAGE = "This is a sent message";
    private static final String MOCK_SENT_MESSAGE_ID = "mock sent message id";
    private static final String MOCK_RECEIVED_MESSAGE = "This is a received message";
    private static final String MOCK_RECEIVED_MESSAGE_ID = "mock received message id";
    private static final String MOCK_GROUP_CHAT_NUMBER_ONE = "+3433333333";
    private static final String MOCK_GROUP_CHAT_NUMBER_TWO = "+3433333344";
    private static final String EXTRA_SELECTION = Sms.BODY + "=?";
    private static final String SELECTION = Sms.ADDRESS + "=?";

    private ContentResolver mContentResolver = null;
    private Context mContext = null;
    private PluginGroupChatWindow mChatWindow = null;
    private List mChatWindowList = null;
    private ParcelUuid mMockTag = null;
    private PluginReceivedChatMessage mReceivedMessage = null;

    public GroupChatMessageHelper(Context context) throws NoSuchFieldException, 
    IllegalArgumentException, IllegalAccessException {
        mContext = context;
        mContentResolver = context.getContentResolver();
        Field chatWindowList = Utils.getPrivateField(PluginChatWindowManager.class, "CHAT_WINDOW_LIST");
        mChatWindowList = (List) chatWindowList.get(null);
        mMockTag = new ParcelUuid(UUID.randomUUID());
    }

    /**
     * Mock some group chat messages in both RCSe db
     * @return The id of each mock messages
     */
    public List<Integer> prepareMockMessage() {
        List<Integer> rcseMessageIds = new ArrayList<Integer>();

        // Mock a sent group chat message
        ContentValues mockRcseValues = new ContentValues();
        mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
        mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_SENT_MESSAGE);
        mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_SENT_MESSAGE_ID);
        mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE);
        mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
        Uri insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
        Integer insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
        rcseMessageIds.add(insertedRcseId);

        // Mock a received group chat message
        mockRcseValues.clear();
        mockRcseValues.put(RichMessagingData.KEY_CONTACT, MOCK_NUMBER);
        mockRcseValues.put(RichMessagingData.KEY_DATA, MOCK_RECEIVED_MESSAGE);
        mockRcseValues.put(RichMessagingData.KEY_MESSAGE_ID, MOCK_RECEIVED_MESSAGE_ID);
        mockRcseValues.put(RichMessagingData.KEY_TYPE, EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE);
        mockRcseValues.put(RichMessagingData.KEY_TIMESTAMP, new Date().getTime());
        insertedRcseUri = mContentResolver.insert(RichMessagingData.CONTENT_URI, mockRcseValues);
        insertedRcseId = Integer.parseInt(insertedRcseUri.getLastPathSegment());
        rcseMessageIds.add(insertedRcseId);
        return rcseMessageIds;
    }

    /**
     * Add a group chat view into ViewImpl to mock dispatch behavior
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void setupViewImpl() throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        List<ParticipantInfo> participants = new CopyOnWriteArrayList<ParticipantInfo>();
        participants.add(new ParticipantInfo(new Participant(MOCK_GROUP_CHAT_NUMBER_ONE, MOCK_GROUP_CHAT_NUMBER_ONE)
        , User.STATE_PENDING));
        participants.add(new ParticipantInfo(new Participant(MOCK_GROUP_CHAT_NUMBER_TWO, MOCK_GROUP_CHAT_NUMBER_TWO)
        , User.STATE_PENDING));
        ViewImpl.getInstance().addGroupChatWindow(mMockTag, participants);
    }

    /**
     * Get the tag of the mock group chat
     * @return The tag of the mock group chat
     */
    public Object getMockTag() {
        return mMockTag;
    }

    /**
     * Setup a plugin group chat window
     * @throws RemoteException
     */
    public void setupGroupChatWindow() throws RemoteException {
        List<ParticipantInfo> participants = new ArrayList<ParticipantInfo>();
        participants.add(new ParticipantInfo(new Participant(MOCK_GROUP_CHAT_NUMBER_ONE, MOCK_GROUP_CHAT_NUMBER_ONE)
        , User.STATE_PENDING));
        participants.add(new ParticipantInfo(new Participant(MOCK_GROUP_CHAT_NUMBER_TWO, MOCK_GROUP_CHAT_NUMBER_TWO)
        , User.STATE_PENDING));
        TagTranslater.saveTag(mMockTag);
        mChatWindow = new PluginGroupChatWindow(mMockTag.toString(), new IpMessageManager(mContext), participants);
        mChatWindow.updateParticipants(participants);
        mChatWindowList.add(mChatWindow);
        mChatWindow.addSentMessage(new InstantMessage(MOCK_SENT_MESSAGE_ID, MOCK_GROUP_CHAT_NUMBER_ONE,
                MOCK_SENT_MESSAGE, false), -1);
        mReceivedMessage =
                (PluginReceivedChatMessage) mChatWindow.addReceivedMessage(new InstantMessage(
                        MOCK_RECEIVED_MESSAGE_ID, MOCK_GROUP_CHAT_NUMBER_ONE,
                        MOCK_RECEIVED_MESSAGE, false), false);
    }
    
    /**
     * Get received chat Message
     * @return
     */
    public PluginReceivedChatMessage getReceivedChatMessage() {
        return mReceivedMessage;
    }

    /**
     * Wait for the messages in Mms db
     * @return true if success to query the messages without time out, otherwise false
     * @throws InterruptedException
     */
    public boolean waitForExpectedMessage() throws InterruptedException {
        long startTime = System.currentTimeMillis();
        do {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                return false;
            }
            Thread.sleep(SLEEP_TIME);
        } while (!isMessageExistInMmsDB(MOCK_SENT_MESSAGE) || !isMessageExistInMmsDB(MOCK_RECEIVED_MESSAGE));
        return true;
    }

    /**
     * Check whether a message is in Mms db
     * @param messageText The target message to be checked
     * @return true if success to query this target message, otherwise false
     */
    private boolean isMessageExistInMmsDB(String messageText) {
        Cursor cursor = null;
        try {
            final String[] args = {PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + mMockTag,
                    messageText};
            cursor = mContentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION + " AND " + EXTRA_SELECTION, args, null);
            if (cursor.moveToFirst()) {
                Logger.d(TAG, "isMessageExistInMmsDB() messageText: " + messageText + " found!");
                return true;
            } else {
                return false;
            }
        } finally {
            cursor.close();
        }
    }

    /**
     * Get the thread id of group chat
     * @return The thread id
     */
    public long getThreadId() {
        Cursor cursor = null;
        try {
            final String[] args = {PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + mMockTag};
            cursor = mContentResolver.query(Utils.SMS_CONTENT_URI, null, SELECTION, args, null);
            if (cursor.moveToFirst()) {
                Logger.d(TAG, "getThreadId() success!");
                return cursor.getLong(cursor.getColumnIndex(Sms.THREAD_ID));
            } else {
                Logger.d(TAG, "getThreadId() fail!");
                return -1L;
            }
        } finally {
            cursor.close();
        }
    }

    /**
     * Clear all the mock messages, including both RCSe db and Mms db
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws RemoteException
     */
    public void clearMockMessage() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, RemoteException {
        String[] arg = {
            MOCK_NUMBER
        };
        mContentResolver.delete(RichMessagingData.CONTENT_URI,
                RichMessagingData.KEY_CONTACT + "=?", arg);
        final String[] args = {
            PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + mMockTag
        };
        Field filedContactString =
                Utils.getPrivateField(PluginGroupChatWindow.class, "mContactString");
        filedContactString.set(mChatWindow, args[0]);
        mChatWindow.removeAllMessages();
        mChatWindowList.remove(mChatWindow);
    }
}
