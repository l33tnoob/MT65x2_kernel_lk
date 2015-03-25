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

package com.mediatek.rcse.test.mvc.view;

import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.util.Date;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This class is used to test SentChatMessage
 */
public class SentChatMessageTest extends InstrumentationTestCase {
    private static final String TAG = "SentChatMessageTest";
    private static final String MESSAGE_ID = "123456";
    private static final int MSG_ID = 123456;
    private static final String CONTACT = "+34200000246";
    private static final String MESSAGE = "test";
    private SentChatMessage mSentChatMessage = null;
    private MockOneOneChatWindow mOneOneWindow = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        InstantMessage msg = new InstantMessage(MESSAGE_ID, CONTACT, MESSAGE,
                true);
        mSentChatMessage = new SentChatMessage(msg, MSG_ID);
        mOneOneWindow = new MockOneOneChatWindow();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test case to getMessageTag().
     */
    public void testCase01_getMessageTag() {
        Logger.d(TAG, "testCase01_getMessageTag");
        assertEquals(MSG_ID, mSentChatMessage.getMessageTag());
    }

    /**
     * Test updateStatus
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase02_updateStatus() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase02_updateStatus");
        Field fieldmStatus = Utils.getPrivateField(mSentChatMessage.getClass(),
                "mStatus");
        ISentChatMessage.Status status = Status.DISPLAYED;
        fieldmStatus.set(mSentChatMessage, Status.DISPLAYED);
        mSentChatMessage.updateStatus(status);
        fieldmStatus.set(mSentChatMessage, Status.SENDING);
        ConcurrentHashMap<IChatWindow, IChatWindowMessage> chatWindowMap = getChatWindowMapFiled();
        MockSentChatMessage sentChatMessage = new MockSentChatMessage();
        MockReceivedChatMessage receivedChatMessage = new MockReceivedChatMessage();
        chatWindowMap.put(mOneOneWindow, sentChatMessage);
        chatWindowMap.put(new MockOneOneChatWindow(), receivedChatMessage);
        mSentChatMessage.updateStatus(status);
        assertTrue(sentChatMessage.mStatusUpdated);
        chatWindowMap.clear();
    }

    /**
     * Test onAddChatWindow
     * 
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     */
    public void testCase03_onAddChatWindow() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase03_onAddChatWindow");
        mSentChatMessage.onAddChatWindow(null);
        mOneOneWindow.mShouldAddSentMessage = false;
        mSentChatMessage.onAddChatWindow(mOneOneWindow);
        mOneOneWindow.mShouldAddSentMessage = true;
        mSentChatMessage.onAddChatWindow(mOneOneWindow);
        ConcurrentHashMap<IChatWindow, IChatWindowMessage> chatWindowMap = getChatWindowMapFiled();
        assertTrue(chatWindowMap.containsKey(mOneOneWindow));
        chatWindowMap.clear();
    }

    /**
     * Test onRemoveChatWindow
     * 
     * @throws IllegalAccessException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     */
    public void testCase04_onRemoveChatWindow()
            throws IllegalArgumentException, NoSuchFieldException,
            IllegalAccessException {
        Logger.d(TAG, "testCase04_onRemoveChatWindow");
        ConcurrentHashMap<IChatWindow, IChatWindowMessage> chatWindowMap = getChatWindowMapFiled();
        MockSentChatMessage sentChatMessage = new MockSentChatMessage();
        chatWindowMap.put(mOneOneWindow, sentChatMessage);
        mSentChatMessage.onRemoveChatWindow(mOneOneWindow);
        assertFalse(chatWindowMap.containsKey(mOneOneWindow));
    }

    /**
     * Test updateDate
     * 
     * @throws IllegalAccessException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     */
    public void testCase05_updateDate() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase05_updateDate");
        mSentChatMessage.updateDate(null);
        ConcurrentHashMap<IChatWindow, IChatWindowMessage> chatWindowMap = getChatWindowMapFiled();
        MockSentChatMessage sentChatMessage = new MockSentChatMessage();
        MockReceivedChatMessage receivedChatMessage = new MockReceivedChatMessage();
        chatWindowMap.put(mOneOneWindow, sentChatMessage);
        chatWindowMap.put(new MockOneOneChatWindow(), receivedChatMessage);
        mSentChatMessage.updateDate(new Date());
        chatWindowMap.clear();
    }

    private class MockSentChatMessage implements ISentChatMessage {
        private boolean mStatusUpdated = false;
        private boolean mDateUpdated = false;

        @Override
        public String getId() {
            return "MockSentChatMessage";
        }

        @Override
        public void updateStatus(Status status) {
            mStatusUpdated = true;
        }

        @Override
        public void updateDate(Date date) {
            Logger.d(TAG, "updateDate()");
            mDateUpdated = true;
        }

		@Override
		public void updateStatus(Status status, String Contact) {
			// TODO Auto-generated method stub
			
		}
    }

    private class MockReceivedChatMessage implements IReceivedChatMessage {

        @Override
        public String getId() {
            return "MockReceivedChatMessage";
        }

    }

    /**
     * Mock chat window for test
     */
    private class MockOneOneChatWindow implements IOne2OneChatWindow {
        private boolean mUpdated = false;
        private boolean mMessagesRemoved = false;
        private boolean mIsComposing = false;
        private boolean mShowLoader = false;
        private boolean mShouldAddSentMessage = true;

        public boolean isUpdated() {
            return mUpdated;
        }

        public boolean showLoader() {
            return mShowLoader;
        }

        public boolean isMessagesRemoved() {
            return mMessagesRemoved;
        }

        public boolean isComposing() {
            return mIsComposing;
        }

        public void setFileTransferEnable(int reason) {

        }

        public void setIsComposing(boolean isComposing) {
            Logger.d(TAG, "setIsComposing() isComposing: " + isComposing);
            mIsComposing = isComposing;
        }

        public void setRemoteOfflineReminder(boolean isOffline) {

        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return null;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return null;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
            mShowLoader = showLoader;
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message,
                boolean isRead) {
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message,
                int messageTag) {
            Logger.d(TAG, "addSentMessage(): mShouldAddSentMessage = " + mShouldAddSentMessage);
            SentMessage sentMessage = null;
            if (mShouldAddSentMessage) {
                One2OneChatFragment fragment = new One2OneChatFragment();
                sentMessage = fragment.new SentMessage(message);
                return sentMessage;
            } else {
                return sentMessage;
            }
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

    @SuppressWarnings("unchecked")
    private ConcurrentHashMap<IChatWindow, IChatWindowMessage> getChatWindowMapFiled()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Field fieldmChatWindowMap = Utils.getPrivateField(
                mSentChatMessage.getClass().getSuperclass(), "mChatWindowMap");
        return (ConcurrentHashMap<IChatWindow, IChatWindowMessage>) fieldmChatWindowMap
                .get(mSentChatMessage);
    }

}
