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

package com.mediatek.rcse.test.service;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.UnreadMessageManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test UnreadMessageManager.java
 */
public class UnreadMessageManagerTest extends InstrumentationTestCase {
    private static final String TAG = "UnreadMessageManagerTest";
    /**
     * Broadcast Action: action used for launcher unread number feature. The
     * broadcat is sent when the unread number of application changes.
     */
    private static final String MTK_ACTION_UNREAD_CHANGED = "com.mediatek.action.UNREAD_CHANGED";
    /**
     * Extra used to indicate the unread number of which component changes.
     */
    private static final String MTK_EXTRA_UNREAD_COMPONENT = "com.mediatek.intent.extra.UNREAD_COMPONENT";
    /**
     * The number of unread messages.
     */
    private static final String MTK_EXTRA_UNREAD_NUMBER = "com.mediatek.intent.extra.UNREAD_NUMBER";
    /**
     * The package name of RCS-e
     */
    private static final String RCSE_PACKAGE_NAME = "com.orangelabs.rcs";

    private static final String HANDLE_GROUP_INVITATION = "handleGroupChatInvitation";
    private static final String TEST_ACCOUNT = "+34200000255";
    private static final String TEST_NAME = "test";
    private static final String TEST_ACCOUNT2 = "+34200000254";
    private static final String TEST_NAME2 = "test2";
    private static final String TEST_MESSAGE = "test";
    private static final String TEST_REMOTE = "+34200000246";
    private static final int SINGLE = 1;
    private static final int TIME_OUT = 5000;
    private static final int SLEEP_TIME = 20;
    private static final String API_MANAGER_MESSAGING_API = "mMessagingApi";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String FIRST_MESSAGE = "firstMessage";
    private final static String MOCK_SESSION_ID = "mock_session_id";
    private final static String MOCK_CALLER = "+34200000250";
    private final static String MOCK_PARTICIPANT_ONE = "+34200000246";
    private final static String MOCK_PARTICIPANT_TWO = "+34200000247";
    private final static String MOCK_PARTICIPANT_THREE = "+34200000248";
    private final static InstantMessage MOCK_FIRST_MESSAGE = new InstantMessage("", MOCK_CALLER,
            "Test Message", false, new Date());
    private final static List<String> MOCK_PARTICIPANTS = new ArrayList<String>();
    private final static RcsNotification NOTIFICATION_INSTANCE = RcsNotification.getInstance();

    private CopyOnWriteArrayList<IChatWindowManager> mChatWindowManagerList = null;
    private UnreadMessageReceiver mUnreadMessageReceiver = null;
    // As test case will destory UnreadMessageManager in setup
    private boolean mIsDestoryed = true;

    static {
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_ONE);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_TWO);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_THREE);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp()");
        super.setUp();
        ApiManager.initialize(getInstrumentation().getTargetContext());
        mIsDestoryed = true;
        // Clear first
        destory();
        // Register rich call invitation listener
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(MTK_ACTION_UNREAD_CHANGED);
        mUnreadMessageReceiver = new UnreadMessageReceiver();
        getInstrumentation().getTargetContext().registerReceiver(mUnreadMessageReceiver,
                intentFilter);
        UnreadMessageManager.getInstance();
        getInstrumentation().waitForIdleSync();
    }

    /**
     * Check whether UnreadMessageManager is singleton
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     */
    public void testCase1_UnreadMessageManagerIsSingleton() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase1_UnreadMessageManagerIsSingleton() entry");
        UnreadMessageManager.getInstance();
        int num = waitForUnreadMessageManager();
        Logger.d(TAG, "num = " + num);
        assertEquals(SINGLE, num);
    }

    public void testCase2_One2OneChatMesasgeUnread() throws InterruptedException {
        Logger.d(TAG, "testCase2_One2OneChatMesasgeUnread() entry");
        testReceiveMessageUnread(false);
    }

    public void testCase3_GroupChatMesasgeUnread() throws InterruptedException {
        Logger.d(TAG, "testCase3_GroupChatMesasgeUnread() entry");
        testReceiveMessageUnread(true);
    }

    public void testCase4_One2OneChatMesasgeRead() throws InterruptedException {
        Logger.d(TAG, "testCase4_One2OneChatMesasgeRead() entry");
        testReceiveMessageRead(false);
    }

    public void testCase5_GroupChatMesasgeRead() throws InterruptedException {
        Logger.d(TAG, "testCase5_GroupChatMesasgeRead() entry");
        testReceiveMessageRead(true);
    }

    public void testCase6_FileTransferInvitation() throws NoSuchMethodException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InterruptedException {
        Logger.d(TAG, "testCase6_FileTransferInvitation() entry");
        testReceiveFileTransferInvitation();
    }

    public void testCase7_GroupChatInvitation() throws InterruptedException, NoSuchMethodException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase7_GroupChatInvitation() entry");
        Field apiManagerField = ApiManager.class.getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerField.setAccessible(true);
        apiManagerField.set(ApiManager.class, null);
        Method initializeMethod = ApiManager.class.getDeclaredMethod(API_MANAGER_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());
        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull("setUp apiManager is null", apiManager);
        Field messagingApiField = ApiManager.class.getDeclaredField(API_MANAGER_MESSAGING_API);
        messagingApiField.setAccessible(true);
        messagingApiField.set(apiManager, new MockMessagingApi(getInstrumentation()
                .getTargetContext()));

        Method method = Utils.getPrivateMethod(RcsNotification.class, HANDLE_GROUP_INVITATION,
                Context.class, Intent.class, boolean.class);
        method.invoke(NOTIFICATION_INSTANCE, getInstrumentation().getTargetContext(),
                getMockIntent(), false);
        boolean broadcastStatus = waitBroadcast();
        Logger.d(TAG, "broadcastStatus = " + broadcastStatus);
        assertTrue(broadcastStatus);
        assertEquals(UnreadMessageManager.getInstance().getUnreadMessageNum(),
                mUnreadMessageReceiver.getUnreadMessageNum());
        NOTIFICATION_INSTANCE.removeGroupInvite(MOCK_SESSION_ID);
    }

    public void testCase8_resetUnreadMessageNum() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase8_resetUnreadMessageNum() entry");
        UnreadMessageManager.getInstance().resetUnreadMessageNum();
        int num = waitForUnreadMessageManager();
        Logger.d(TAG, "num = " + num);
        assertEquals(0, UnreadMessageManager.getInstance().getUnreadMessageNum());
    }

    public void testCase09_UnreadMessageChatWindow() throws Exception {
        UnreadMessageManager.getInstance().resetUnreadMessageNum();
        Object window = UnreadMessageManager.getInstance().addOne2OneChatWindow("", null);
        Method method = Utils.getPrivateMethod(window.getClass(), "updateAllMsgAsRead");
        method.invoke(window);
        assertEquals(0, UnreadMessageManager.getInstance().getUnreadMessageNum());

        // removeAllMessages
        method = Utils.getPrivateMethod(window.getClass(), "removeAllMessages");
        method.invoke(window);

        method = Utils.getPrivateMethod(window.getClass(), "addChatEventInformation",
                ChatEventStruct.class);
        assertNull(method.invoke(window, (ChatEventStruct) null));

        method = Utils.getPrivateMethod(window.getClass(), "addSentFileTransfer", FileStruct.class);
        assertNull(method.invoke(window, (FileStruct) null));

        method = Utils.getPrivateMethod(window.getClass(), "addSentMessage", InstantMessage.class,
                int.class);
        assertNull(method.invoke(window, (InstantMessage) null, 0));

        method = Utils.getPrivateMethod(window.getClass(), "addLoadHistoryHeader", boolean.class);
        method.invoke(window, false);

        method = Utils.getPrivateMethod(window.getClass(), "setIsComposing", boolean.class,
                Participant.class);
        method.invoke(window, true, (Participant) null);

        method = Utils.getPrivateMethod(window.getClass(), "addLoadHistoryHeader", boolean.class);
        method.invoke(window, false);

        method = Utils.getPrivateMethod(window.getClass(), "getSentChatMessage", String.class);
        assertNull(method.invoke(window, ""));
    }

    @SuppressWarnings("unchecked")
    private int waitForUnreadMessageManager() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Field chatWindowManagerListField = Utils.getPrivateField(ViewImpl.class,
                "mChatWindowManagerList");
        mChatWindowManagerList = (CopyOnWriteArrayList<IChatWindowManager>) chatWindowManagerListField
                .get(ViewImpl.getInstance());
        int size = mChatWindowManagerList.size();
        waitBroadcast();
        Logger.d(TAG, "size = " + size);
        int unreadMessageManagerNum = 0;
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            if (chatWindowManager instanceof UnreadMessageManager) {
                ++unreadMessageManagerNum;
            }
        }
        return unreadMessageManagerNum;
    }

    private IChatWindow getChatWindow(boolean isGroup) {
        ParcelUuid tag = new ParcelUuid(UUID.randomUUID());
        IChatWindow chatWindow = null;
        if (!isGroup) {
            Logger.d(TAG, "Add one2one chat window");
            chatWindow = ViewImpl.getInstance().addOne2OneChatWindow(tag,
                    new Participant(TEST_ACCOUNT, TEST_NAME));
        } else {
            Logger.d(TAG, "Add group chat window");
            CopyOnWriteArrayList<ParticipantInfo> participantInfoList = new CopyOnWriteArrayList<ParticipantInfo>();
            ParticipantInfo participantInfo1 = new ParticipantInfo(new Participant(TEST_ACCOUNT,
                    TEST_NAME), User.STATE_CONNECTED);
            participantInfoList.add(participantInfo1);
            ParticipantInfo participantInfo2 = new ParticipantInfo(new Participant(TEST_ACCOUNT2,
                    TEST_NAME2), User.STATE_CONNECTED);
            participantInfoList.add(participantInfo2);
            chatWindow = ViewImpl.getInstance().addGroupChatWindow(tag, participantInfoList);
        }
        return chatWindow;
    }

    private void testReceiveMessageUnread(boolean isGroup) throws InterruptedException {
        Logger.d(TAG, "testReceiveMessageUnread(), isGroup = " + isGroup);
        InstantMessage message = new InstantMessage(System.currentTimeMillis() + "", TEST_REMOTE,
                TEST_MESSAGE, true, new Date());
        IChatWindow chatWindow = getChatWindow(isGroup);
        mIsDestoryed = false;
        Logger.d(TAG, "testReceiveMessageUnread(), Mock receive message unread");
        chatWindow.addReceivedMessage(message, false);
        boolean broadcastStatus = waitBroadcast();
        Logger.d(TAG, "broadcastStatus = " + broadcastStatus);
        assertEquals(true, broadcastStatus);
        assertEquals(UnreadMessageManager.getInstance().getUnreadMessageNum(),
                mUnreadMessageReceiver.getUnreadMessageNum());
        ViewImpl.getInstance().removeChatWindow(chatWindow);
    }

    private void testReceiveMessageRead(boolean isGroup) throws InterruptedException {
        Logger.d(TAG, "testReceiveMessageRead(), isGroup = " + isGroup);
        InstantMessage message = new InstantMessage(System.currentTimeMillis() + "", TEST_REMOTE,
                TEST_MESSAGE, true, new Date());
        IChatWindow chatWindow = getChatWindow(isGroup);
        mIsDestoryed = false;
        Logger.d(TAG, "testReceiveMessageUnread(), Mock receive message read");
        chatWindow.addReceivedMessage(message, true);
        boolean broadcastStatus = waitBroadcast();
        Logger.d(TAG, "broadcastStatus = " + broadcastStatus);
        assertEquals(0, mUnreadMessageReceiver.getUnreadMessageNum());
        ViewImpl.getInstance().removeChatWindow(chatWindow);
    }

    private boolean testReceiveFileTransferInvitation() throws InterruptedException {
        Logger.d(TAG, "testReceiveFileTransferInvitation()");
        IOne2OneChatWindow chatWindow = (IOne2OneChatWindow) getChatWindow(false);
        FileStruct file = new FileStruct("testpath", "test.jpg", 10240, UUID.randomUUID(),
                new Date());
        mIsDestoryed = false;
        Logger.d(TAG, "testReceiveMessageUnread(), Mock receive file transfer invitation unread");
        chatWindow.addReceivedFileTransfer(file, mIsDestoryed);
        boolean broadcastStatus = waitBroadcast();
        Logger.d(TAG, "broadcastStatus = " + broadcastStatus);
        assertEquals(true, broadcastStatus);
        assertEquals(UnreadMessageManager.getInstance().getUnreadMessageNum(),
                mUnreadMessageReceiver.getUnreadMessageNum());
        ViewImpl.getInstance().removeChatWindow(chatWindow);
        return true;
    }

    /**
     * Wait the unread message broadcast
     * 
     * @return
     * @throws InterruptedException
     */
    private boolean waitBroadcast() throws InterruptedException {
        long begTime = System.currentTimeMillis();
        while (mUnreadMessageReceiver.getBroadcastStatus() == false || mIsDestoryed == true) {
            if (System.currentTimeMillis() - begTime > TIME_OUT) {
                break;
            }
            Thread.sleep(SLEEP_TIME);
        }
        return mUnreadMessageReceiver.getBroadcastStatus();
    }

    private Intent getMockIntent() {
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(FIRST_MESSAGE, MOCK_FIRST_MESSAGE);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        return intent;
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        getInstrumentation().getTargetContext().unregisterReceiver(mUnreadMessageReceiver);
        mUnreadMessageReceiver = null;
    }

    private void destory() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.d(TAG, "destory UnreadMessageManager.");
        Field instance = Utils.getPrivateField(UnreadMessageManager.class, "sInstance");
        instance.set(UnreadMessageManager.class, null);
        Utils.clearAllStatus();
    }

    /**
     * A mocked unread message receiver
     */
    private class UnreadMessageReceiver extends BroadcastReceiver {
        private static final String TAG = "UnreadMessageReceiver";
        private boolean mHasReceivedBroadcast = false;
        private int mUnReadMessageNum = 0;

        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive(), context = " + context + ", intent = " + intent);
            if (intent != null) {
                String action = intent.getAction();
                Logger.v(TAG, "action = " + action);
                if (MTK_ACTION_UNREAD_CHANGED.equals(action)) {
                    int unReadMessageNum = intent.getIntExtra(MTK_EXTRA_UNREAD_NUMBER, 0);
                    ComponentName componentName = intent
                            .getParcelableExtra(MTK_EXTRA_UNREAD_COMPONENT);
                    String packageName = componentName.getPackageName();
                    if (RCSE_PACKAGE_NAME.equals(packageName)) {
                        Logger.d(TAG, "Receive RCS-e unread message broadcast. unReadMessageNum = "
                                + unReadMessageNum);
                        synchronized (TAG) {
                            mHasReceivedBroadcast = true;
                            mUnReadMessageNum = unReadMessageNum;
                            // This is a workaround solution to avoid
                            // desotry()'s side effect
                            if (mUnReadMessageNum == 0) {
                                Logger.v(TAG, "reset broadcast status");
                                mHasReceivedBroadcast = false;
                            }
                        }
                    }
                }
            } else {
                Logger.w(TAG, "intent is null");
            }
        }

        /**
         * Get the broadcast status
         * 
         * @return True is the receiver has received the broadcast sent by RCS-e
         *         package, otherwise return false
         */
        public boolean getBroadcastStatus() {
            synchronized (TAG) {
                return mHasReceivedBroadcast;
            }
        }

        /**
         * Get the number of unread message
         * 
         * @return The number of unread message
         */
        public int getUnreadMessageNum() {
            synchronized (TAG) {
                return mUnReadMessageNum;
            }
        }

    }

    /**
     * The mock messagingApi can be used to provide a mock chat session even in
     * off-line state
     */
    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            if (MOCK_SESSION_ID.equals(id)) {
                return new MockChatSession();
            } else {
                return super.getChatSession(id);
            }
        }

        @Override
        public IFileTransferSession getFileTransferSession(String id) throws ClientApiException {
            if (MOCK_SESSION_ID.equals(id)) {
                return null;
            } else {
                return super.getFileTransferSession(id);
            }
        }
    }

    /**
     * Mock chat session that has a mock session-id and a mock participant list
     */
    private class MockChatSession implements IChatSession {

        @Override
        public void acceptSession() throws RemoteException {
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
            return MOCK_PARTICIPANTS;
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
            return null;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return MOCK_SESSION_ID;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return 0;
        }


        @Override
        public boolean isGroupChat() throws RemoteException {
            return true;
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
            return null;
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

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public boolean isSessionIdle() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}
    }
}
