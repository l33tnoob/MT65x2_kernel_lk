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
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.view.ReceivedChatMessage;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This class is used to test ReceivedChatMessage
 */
public class ReceivedChatMessageTest extends InstrumentationTestCase {
    private static final String TAG = "SentChatMessageTest";
    private static final String MESSAGE_ID = "123456";
    private static final String CONTACT = "+34200000246";
    private static final String MESSAGE = "test";
    private ReceivedChatMessage mReceivedChatMessage = null;
    private MockOneOneChatWindow mOneOneWindow = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        InstantMessage msg = new InstantMessage(MESSAGE_ID, CONTACT, MESSAGE,
                true);
        mReceivedChatMessage = new ReceivedChatMessage(msg, false);
        mOneOneWindow = new MockOneOneChatWindow();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test updateStatus
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase01_updateStatus() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase01_updateStatus");
        Field fieldmIsRead = Utils.getPrivateField(
                mReceivedChatMessage.getClass(), "mIsRead");
        assertFalse(fieldmIsRead.getBoolean(mReceivedChatMessage));
        mReceivedChatMessage.updateStatus(true);
        assertTrue(fieldmIsRead.getBoolean(mReceivedChatMessage));
    }

    /**
     * Test onAddChatWindow
     * 
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     */
    public void testCase02_onAddChatWindow() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase02_onAddChatWindow");
        ConcurrentHashMap<IChatWindow, IChatWindowMessage> chatWindowMap = getChatWindowMapFiled();
        chatWindowMap.clear();
        mReceivedChatMessage.onAddChatWindow(null);
        assertTrue(chatWindowMap.size() == 0);
        mOneOneWindow.mShouldAddSentMessage = false;
        mReceivedChatMessage.onAddChatWindow(mOneOneWindow);
        assertTrue(chatWindowMap.size() == 0);
        mOneOneWindow.mShouldAddSentMessage = true;
        mReceivedChatMessage.onAddChatWindow(mOneOneWindow);
        assertTrue(chatWindowMap.containsKey(mOneOneWindow));
        chatWindowMap.clear();

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
            Logger.d(TAG, "addReceivedMessage(): mShouldAddSentMessage = " + mShouldAddSentMessage);
            ReceivedMessage receivedMessage = null;
            if (mShouldAddSentMessage) {
                One2OneChatFragment fragment = new One2OneChatFragment();
                receivedMessage = fragment.new ReceivedMessage(message);
                return receivedMessage;
            } else {
                return receivedMessage;
            }
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message,
                int messageTag) {
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
        Field fieldmChatWindowMap = Utils.getPrivateField(mReceivedChatMessage
                .getClass().getSuperclass(), "mChatWindowMap");
        return (ConcurrentHashMap<IChatWindow, IChatWindowMessage>) fieldmChatWindowMap
                .get(mReceivedChatMessage);
    }

}
