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
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.os.IBinder;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * This class defined to test the unregister message in ModelImpl part
 */
public class UnregisterMessageTest extends InstrumentationTestCase {
	
    private static final String REGISTRATION_API = "mRegistrationApi";
    private static final String REGISTRATION_STATUS = "mRegistrationStatus";
    private static final String API_MANAGER_MESSAGING_API = "mMessagingApi";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_CONTACT_NUMBER = "3420000090";
    private static final String MOCK_MESSAGE_ID = "mock message id";

    private final Participant mParticipant = new Participant(MOCK_CONTACT_NUMBER, "TestUser");
    private final List<Participant> mParticipantList= new ArrayList<Participant>();
    private One2OneChat mChat = null;
    private Object mChatTag = null;
    private MockChatWindow mChatWindow = null;
    private ContentResolver mContentResolver = null;
    private boolean mIsRegisterred = false;
    private final static int LOOP_COUNT = 1;
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;

    private final static String TAG = "UnregisterMessageTest";

    @Override
	protected void setUp() throws Exception {
		super.setUp();
		Logger.d(TAG, "setUp() entry");
		mIsRegisterred = false;
		Field apiManagerfield = ApiManager.class.getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerfield.setAccessible(true);
        apiManagerfield.set(ApiManager.class, null);

        Method initializeMethod = ApiManager.class.getDeclaredMethod(API_MANAGER_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());

        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp apiManager is null", apiManager);
        mParticipantList.add(mParticipant);
        mChat = (One2OneChat) ModelImpl.getInstance().addChat(mParticipantList, null, null);
		assertNotNull(mChat);
		mChatTag = mChat.getChatTag();
		assertNotNull(mChatTag);
		// Remove all the messages in the database
		Context context = apiManager.getContext();
		assertNotNull("setUp context is null", context);
		mContentResolver = context.getContentResolver();
		mContentResolver.delete(UnregMessageProvider.CONTENT_URI, null, null);
		waitForStoredMessageRemoved();
		Field registrationApiField = ApiManager.class
				.getDeclaredField(REGISTRATION_API);
		registrationApiField.setAccessible(true);
		MockRegistrationApi mockRegistrationApi = new MockRegistrationApi(
				context);
		registrationApiField.set(apiManager, mockRegistrationApi);
		Field messagingApiField = ApiManager.class.getDeclaredField(API_MANAGER_MESSAGING_API);
        messagingApiField.setAccessible(true);
        messagingApiField.set(apiManager,
                new MockMessagingApi(getInstrumentation().getTargetContext()));
		Field registrationStatusField = mockRegistrationApi.getClass()
				.getSuperclass().getDeclaredField(REGISTRATION_STATUS);
		registrationStatusField.setAccessible(true);
		registrationStatusField.set(mockRegistrationApi,
				new MockRegistrationStatus());
		RegistrationApi registrationApi = apiManager.getRegistrationApi();
		assertNotNull("setUp registrationApi is null", registrationApi);
	}

    /**
     * Test the storeUnregisteredMessage() function in ModelImpl
     */
    public void testCase1_storeUnregisteredMessage() throws InterruptedException {
        mChatWindow = new MockChatWindow();
        mChat.setChatWindow(mChatWindow);
        for (int i = 0; i < LOOP_COUNT; i++) {
            mChat.sendMessage("Message" + i, i);
        }
        waitForStoredMessage();
        mChatWindow.waitForMessageFailed ();
        mIsRegisterred = true;
        mChat.onStatusChanged(mIsRegisterred);
        mChatWindow.waitForMessageResent();
        mIsRegisterred = false;
    }

    private void waitForStoredMessage() throws InterruptedException {
        int count = 0;
        Cursor cursor = null;
        long startTime = System.currentTimeMillis();
        while (count != LOOP_COUNT) {
            try {
                cursor = mContentResolver.query(
                        UnregMessageProvider.CONTENT_URI, null, null, null, null);
                Logger.d(TAG,
                        "waitForStoredMessage() - the current number is "
                                + cursor.getCount());
                count = cursor.getCount();
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
        }
    }

    /**
     * Test the removeUnregisteredMessage() function in ModelImpl
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase2_removeUnregisteredMessage() throws InterruptedException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        for (int i = 0; i < LOOP_COUNT; i++) {
            ContentValues values = new ContentValues();
            values.put(UnregMessageProvider.KEY_CHAT_TAG, mChat.toString());
            values.put(UnregMessageProvider.KEY_MESSAGE, "Message" + i);
            mContentResolver.insert(UnregMessageProvider.CONTENT_URI, values);
        }
        Method onDestroyMethod = Utils.getPrivateMethod(mChat.getClass().getSuperclass(), "onDestroy");
        onDestroyMethod.invoke(mChat);
        waitForStoredMessageRemoved();
    }

    private void waitForStoredMessageRemoved() throws InterruptedException {
        int count = 0;
        Cursor cursor = null;
        long startTime = System.currentTimeMillis();
        do {
            try {
                cursor = mContentResolver.query(
                        UnregMessageProvider.CONTENT_URI, null, null, null, null);
                Thread.sleep(SLEEP_TIME);
                count = cursor.getCount();
                Logger.d(TAG,
                        "waitForStoredMessageRemoved() - the current number is "
                                + count);
                if (System.currentTimeMillis() - startTime > TIME_OUT ) {
                    fail("waitForStoredMessageRemoved() time out, count: " + count);
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
        } while (count != 0);
    }

    private class MockRegistrationApi extends RegistrationApi {
        public MockRegistrationApi(Context context) {
            super(context);
        }
    }

    private class MockRegistrationStatus extends IRegistrationStatus.Stub {
        public void addRegistrationStatusListener(IRegistrationStatusRemoteListener listener) {

        }

        public boolean isRegistered() {
            return mIsRegisterred;
        }
    }

    /**
     * The mock messagingApi can be used to provide a mock chat session even in off-line state
     */
    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public List<IBinder> getChatSessionsWith(String contact) throws ClientApiException {
            if (MOCK_CONTACT_NUMBER.equals(contact)) {
                List<IBinder> sessionList = new ArrayList<IBinder>();
                sessionList.add(new MockChatSession().asBinder());
                return sessionList;
            } else {
                return super.getChatSessionsWith(contact);
            }

        }
    }

    /**
     * Mock chat session that always stay in connected
     */
    private class MockChatSession extends IChatSession.Stub {

        @Override
        public void acceptSession() throws RemoteException {
        }

        @Override
        public int getSessionDirection() {
        	return 0;
        }

        @Override
        public void addParticipant(String participant) throws RemoteException {
        }

        @Override
        public void addParticipants(List<String> participants) throws RemoteException {
        }

        @Override
        public void addSessionListener(IChatEventListener listener) throws RemoteException {
        }

        @Override
        public void cancelSession() throws RemoteException {
        }

        @Override
        public String getChatID() throws RemoteException {
            return null;
        }

        @Override
        public InstantMessage getFirstMessage() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getInivtedParticipants() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getParticipants() throws RemoteException {
            return null;
        }

        @Override
        public String getReferredByHeader() throws RemoteException {
            return null;
        }

        @Override
        public String getRemoteContact() throws RemoteException {
            return MOCK_CONTACT_NUMBER;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return null;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return SessionState.ESTABLISHED;
        }

        @Override
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        @Override
        public boolean isGroupChat() throws RemoteException {
            return false;
        }

        @Override
        public boolean isInComing() throws RemoteException {
            return false;
        }

        @Override
        public boolean isStoreAndForward() throws RemoteException {
            return false;
        }

        @Override
        public void rejectSession() throws RemoteException {
        }

        @Override
        public void removeSessionListener(IChatEventListener listener) throws RemoteException {
        }

        @Override
        public String sendMessage(String text) throws RemoteException {
            return MOCK_MESSAGE_ID;
        }

        @Override
        public void setIsComposingStatus(boolean status) throws RemoteException {
        }

        @Override
        public void setMessageDeliveryStatus(String msgId, String status) throws RemoteException {
        }

        @Override
        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId,
                String status) throws RemoteException {
        }

        @Override
        public IBinder asBinder() {
            return this;
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

        public void waitForMessageFailed () {
            waitForMessageAdded();
            long startTime = System.currentTimeMillis();
            boolean isAllMessagesFailed = false;
            while (!isAllMessagesFailed) {
                for (MockSentMessage message : mSentMessageList) {
                    if (!Status.FAILED.equals(message.mStatus)) {
                        isAllMessagesFailed = false;
                        break;
                    }
                    isAllMessagesFailed = true;
                }
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail("MockChatWindow failed to check message failed");
                }
            }
        }

        public void waitForMessageResent() {
            waitForMessageAdded();
            long startTime = System.currentTimeMillis();
            boolean isAllMessagesResent= false;
            while (!isAllMessagesResent) {
                for (MockSentMessage message : mSentMessageList) {
                    if (!Status.SENDING.equals(message.mStatus)) {
                        isAllMessagesResent = false;
                        break;
                    }
                    isAllMessagesResent = true;
                }
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail("MockChatWindow failed to check message resent");
                }
            }
        }

        private void waitForMessageAdded() {
            long startTime = System.currentTimeMillis();
            while (mSentMessageList.size() < LOOP_COUNT) {
                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail("MockChatWindow failed to add messages");
                }
            }
        }
        public void setFileTransferEnable(int reason) {}

        public void setIsComposing(boolean isComposing) {}
        

        public void setRemoteOfflineReminder(boolean isOffline) {}

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return null;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return null;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {}

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        private ConcurrentLinkedQueue<MockSentMessage> mSentMessageList = new ConcurrentLinkedQueue<MockSentMessage>();
        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            MockSentMessage sentMessage = new MockSentMessage();
            mSentMessageList.add(sentMessage);
            return sentMessage;
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
     * Mock sent message to test resend function
     */
    private class MockSentMessage implements ISentChatMessage {
        private Status mStatus = null;
        @Override
        public void updateDate(Date date) {}

        @Override
        public void updateStatus(Status status) {
            Logger.d(TAG, "updateStatus() status: " + status);
            mStatus = status;
        }

        @Override
        public String getId() {
            return null;
        }

		@Override
		public void updateStatus(Status status, String Contact) {
			// TODO Auto-generated method stub
			
		}
    }
}
