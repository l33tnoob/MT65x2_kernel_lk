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
import android.app.AlertDialog;
import android.app.DialogFragment;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ParcelUuid;
import android.test.ActivityInstrumentationTestCase2;
import android.text.TextUtils;
import android.util.Pair;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatListFragment;
import com.mediatek.rcse.activities.ChatListView;
import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.ChatScreenActivity.ChatWindowManager;
import com.mediatek.rcse.activities.ChatListView.ChatsStruct;
import com.mediatek.rcse.activities.ChatListView.InvitationStruct;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.ContactsListManager.OnDisplayNameChangedListener;
import com.mediatek.rcse.activities.widgets.UnreadMessagesContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.UUID;

/**
 * This class is used to test the functions of ContactsListFragment
 */
public class ChatListFragmentTest extends ActivityInstrumentationTestCase2<ChatMainActivity> {

    private static final String TAG = "ChatListFragmentTest";
    private final static String MOCK_SESSION_ID = "mock_session_id";
    private static final String MOCK_CALLER = "+34200000250";
    private static final String MOCK_PARTICIPANT_ONE = "+34200000246";

    private static final String MOCK_PARTICIPANT_TWO = "+34200000247";

    private static final String MOCK_PARTICIPANT_THREE = "+34200000248";
    
    private static final int TIME_OUT = 3000;
    private static final int SLEEP_TIME = 200;

    private static final List<ParticipantInfo> MOCK_PARTICIPANTS = new ArrayList<ParticipantInfo>();
    static {
        MOCK_PARTICIPANTS.add(new ParticipantInfo(new Participant(MOCK_PARTICIPANT_ONE, ""),
                User.STATE_PENDING));
        MOCK_PARTICIPANTS.add(new ParticipantInfo(new Participant(MOCK_PARTICIPANT_TWO, ""),
                User.STATE_PENDING));
        MOCK_PARTICIPANTS.add(new ParticipantInfo(new Participant(MOCK_PARTICIPANT_THREE, ""),
                User.STATE_PENDING));
    }
    private static final int TIME_SPAN = 1000;
    private static final String MOCK_MESSAGE_SENT_ID = "sent";
    private static final String MOCK_MESSAGE_RECEIVE_ID = "receive";
    private static final String MOCK_SENT_MESSAGE_TEXT = "This is a sent message";
    private static final String MOCK_RECEIVED_MESSAGE_TEXT = "This is a received message";
    private static final InstantMessage MOCK_SENT_MESSAGE = new InstantMessage(
            MOCK_MESSAGE_SENT_ID,
            MOCK_PARTICIPANT_THREE, MOCK_SENT_MESSAGE_TEXT, false, new Date());
    private static final InstantMessage MOCK_RECEIVED_MESSAGE = new InstantMessage(
            MOCK_MESSAGE_RECEIVE_ID,
            MOCK_PARTICIPANT_THREE, MOCK_RECEIVED_MESSAGE_TEXT, false, new Date());
    private static final long MILLI_PER_DAY = 60 * 60 * 24 *1000;
    static {
        Date receivedDate = MOCK_RECEIVED_MESSAGE.getDate();
        receivedDate.setTime(receivedDate.getTime() + MILLI_PER_DAY);
    }

    private final static FileStruct MOCK_FILE_STRUCT = new FileStruct("", "", 0, new Object(),
            new Date());
    private final static Date MOCK_DATE =new Date();
    private final static String MOCK_INFORMATION = "This is a testCase!!!";
    private final static MockChatSession MOCK_CHAT_SESSION = new MockChatSession();
    private final static InvitationStruct MOCK_INVITATION_STRUCT = new InvitationStruct(null, null,
            MOCK_INFORMATION, null, MOCK_CHAT_SESSION);

    private ChatMainActivity mActivity = null;

    private ChatListFragment mFragment = null;

    private ChatListView mChatListView = null;

    private ChatFragment currentFragment = null;

    public ChatListFragmentTest() {
        super(ChatMainActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Logger.d(TAG, "setUp");
        mActivity = getActivity();
        Class<? extends ChatMainActivity> clazz = mActivity.getClass();
        assertTrue(clazz != null);
        Field fragmentField = Utils.getPrivateField(ChatMainActivity.class, "mChatListFragment");
        mFragment = (ChatListFragment) fragmentField.get(mActivity);
        Field chatListViewField = Utils.getPrivateField(ChatListFragment.class, "mChatListView");
        mChatListView = (ChatListView) chatListViewField.get(mFragment);
    }

    /**
     * Test the group chat invitation number
     */
    public void testCase1_testGroupChatInvitation() throws Throwable {
        Logger.d(TAG, "testCase1_testGroupChatInvitation entry");
        assertInvitationNumber(0);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(null);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(MOCK_INVITATION_STRUCT);
            }
        });
        getInstrumentation().waitForIdleSync();
        MOCK_INVITATION_STRUCT.setDate(MOCK_DATE);
        Field dateField = Utils.getPrivateField(InvitationStruct.class, "mDate");
        Date mDate = (Date) dateField.get(MOCK_INVITATION_STRUCT);
        assertEquals(MOCK_DATE, mDate);
        assertNull(MOCK_INVITATION_STRUCT.getIntent());
        assertNull(MOCK_INVITATION_STRUCT.getDrawable());
        assertInvitationNumber(1);
        
        Field dataField = Utils.getPrivateField(ChatListView.class, "mData");
        HashMap<View, Object> data = (HashMap<View, Object>) dataField.get(mChatListView);
        assertNotNull(data);
        Set<View> viewSet = data.keySet();
        Iterator<View> viewIterator = viewSet.iterator();
        assertNotNull(viewIterator);
        View view = viewIterator.next();
        mChatListView.updateView();
        TextView textInfo = (TextView) view.findViewById(R.id.invite_infor);
        assertEquals(MOCK_INFORMATION, (String) textInfo.getText());

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mFragment.onRemovedGroupInvite(MOCK_SESSION_ID);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
        Logger.d(TAG, "testCase1_testGroupChatInvitation exit");
    }

    /**
     * Test the chat window list
     */
    public void testCase2_testChatList() throws Throwable {
        Logger.d(TAG, "testCase2_testChatList entry");
        assertChatNumber(0);
        groupChatWindowTest();
        one2oneChatWindowTest();
        Logger.d(TAG, "testCase2_testChatList exit");
    }
    
    /**
     * Test close all function
     */
    public void testCase3_testCloseAllChat() throws Throwable {
        Logger.d(TAG, "testCase1_testGroupChatInvitation entry");
        assertChatNumber(0);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                List<Participant> list1 = new ArrayList<Participant>();
                List<Participant> list2 = new ArrayList<Participant>();
                List<Participant> list3 = new ArrayList<Participant>();
                list1.add(new Participant(MOCK_PARTICIPANT_ONE, ""));
                list2.add(new Participant(MOCK_PARTICIPANT_TWO, ""));
                list3.add(new Participant(MOCK_PARTICIPANT_THREE, ""));
                ModelImpl.getInstance().addChat(list1, null, null);
                ModelImpl.getInstance().addChat(list2, null, null);
                ModelImpl.getInstance().addChat(list3, null, null);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertChatNumber(3);
        
        ControllerImpl controllerImpl = ControllerImpl.getInstance();
        Message controllerMessage = controllerImpl.obtainMessage(ChatController.EVENT_CLOSE_ALL_WINDOW, null, null);
        controllerMessage.sendToTarget();
        long startTime = System.currentTimeMillis();
        while (judgeChatNumber(0)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        Logger.d(TAG, "testCase3_testCloseAllChat exit");
    }

    private void groupChatWindowTest() throws IllegalArgumentException, NoSuchFieldException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        Object chatTag = new Object();
        //Test chat number
        IGroupChatWindow groupChatWindow = mFragment.addGroupChatWindow(chatTag,
                MOCK_PARTICIPANTS);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);

        //Test participant number
        Map groupChatMap = (Map) groupChatWindow;
        List numbers = (List) groupChatMap.get(ChatListFragment.NUMBER);
        assertEquals(MOCK_PARTICIPANTS.size(), numbers.size());
        MOCK_PARTICIPANTS.remove(0);
        groupChatWindow.updateParticipants(MOCK_PARTICIPANTS);
        getInstrumentation().waitForIdleSync();
        numbers = (List) groupChatMap.get(ChatListFragment.NUMBER);
        assertEquals(MOCK_PARTICIPANTS.size(), numbers.size());

        //Test the latest message
        ChatsStruct chatsStruct = getChatStruct(chatTag);
        assertTrue(TextUtils.isEmpty(chatsStruct.getLatestMessage()));
        groupChatWindow.addSentMessage(MOCK_SENT_MESSAGE, -1);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_SENT_MESSAGE_TEXT, chatsStruct.getLatestMessage());
        groupChatWindow.addReceivedMessage(MOCK_RECEIVED_MESSAGE, false);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_RECEIVED_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        //test remove all Messages
        groupChatWindow.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        assertEquals(null, chatsStruct.getLatestMessage());
        assertEquals(null, chatsStruct.getDate());
        assertEquals(0, chatsStruct.getUnreadMessageNumber());

        //Test remove chat window
        mFragment.removeChatWindow(groupChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    private void one2oneChatWindowTest() throws IllegalArgumentException, NoSuchFieldException,
    IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        //Test chat number
        Object chatTag = new Object();
        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Date dateOld = new Date();
        Date dateNew = new Date();
        dateNew.setTime(dateOld.getTime() + TIME_SPAN);
        InstantMessage sentInstantMessage = new InstantMessage(MOCK_MESSAGE_SENT_ID,
                MOCK_PARTICIPANT_THREE, MOCK_SENT_MESSAGE_TEXT, false, dateOld);
        InstantMessage receiveInstantMessage = new InstantMessage(MOCK_MESSAGE_RECEIVE_ID,
                MOCK_PARTICIPANT_THREE, MOCK_RECEIVED_MESSAGE_TEXT, false, dateNew);

        //Test the latest message
        ChatsStruct chatsStruct = getChatStruct(chatTag);
        assertTrue(TextUtils.isEmpty(chatsStruct.getLatestMessage()));
        ISentChatMessage sentMessage = oneOneChatWindow.addSentMessage(sentInstantMessage, -1);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_SENT_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        oneOneChatWindow.addSentMessage(receiveInstantMessage, -1);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_RECEIVED_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        Date sentMessageDate = sentInstantMessage.getDate();
        sentMessageDate.setTime(sentMessageDate.getTime() + MILLI_PER_DAY * 2);
        sentMessage.updateDate(sentMessageDate);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_SENT_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        sentMessageDate.setTime(sentMessageDate.getTime() - MILLI_PER_DAY * 2);
        sentMessage.updateDate(sentMessageDate);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_RECEIVED_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        //add a new sent message for example named MessageA that be sent to server at this time 
        sentMessageDate.setTime(System.currentTimeMillis() + MILLI_PER_DAY * 2);
        oneOneChatWindow.addSentMessage(sentInstantMessage, -1);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_SENT_MESSAGE_TEXT, chatsStruct.getLatestMessage());
        //add a new received message for example named MessageB that be sent to server earlier MILLI_PER_DAY * 2 than MessageA.
        Date receivedMessageDate = receiveInstantMessage.getDate();
        receivedMessageDate.setTime(sentMessageDate.getTime() - MILLI_PER_DAY * 2);
        oneOneChatWindow.addReceivedMessage(receiveInstantMessage, false);
        getInstrumentation().waitForIdleSync();
        assertEquals(MOCK_SENT_MESSAGE_TEXT, chatsStruct.getLatestMessage());

        //Test active file transfer icon
        IFileTransfer sentFileTransfer = oneOneChatWindow.addSentFileTransfer(MOCK_FILE_STRUCT);
        sentFileTransfer.setStatus(Status.WAITING);
        getInstrumentation().waitForIdleSync();
        assertTrue(chatsStruct.getIsFileTransfer());
        sentFileTransfer.setStatus(Status.TRANSFERING);
        getInstrumentation().waitForIdleSync();
        assertTrue(chatsStruct.getIsFileTransfer());
        sentFileTransfer.setStatus(Status.FINISHED);
        getInstrumentation().waitForIdleSync();
        assertFalse(chatsStruct.getIsFileTransfer());
        boolean mInitialTouchMode = false;
		IFileTransfer receivedFileTransfer = oneOneChatWindow.addReceivedFileTransfer(MOCK_FILE_STRUCT, mInitialTouchMode );
        receivedFileTransfer.setStatus(Status.WAITING);
        getInstrumentation().waitForIdleSync();
        assertTrue(chatsStruct.getIsFileTransfer());
        receivedFileTransfer.setStatus(Status.TRANSFERING);
        getInstrumentation().waitForIdleSync();
        assertTrue(chatsStruct.getIsFileTransfer());
        receivedFileTransfer.setStatus(Status.CANCELED);
        getInstrumentation().waitForIdleSync();
        assertFalse(chatsStruct.getIsFileTransfer());

        //test remove all Messages
        oneOneChatWindow.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        assertEquals(null, chatsStruct.getLatestMessage());
        assertEquals(null, chatsStruct.getDate());
        assertEquals(0, chatsStruct.getUnreadMessageNumber());

        //Test remove chat window
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    private ChatsStruct getChatStruct(Object chatTag) throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        Method getChatPairWithTagMethod = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTagMethod
                .invoke(mChatListView, chatTag);
        return findPair.second;
    }

    private void assertInvitationNumber(int expect) throws IllegalArgumentException,
            IllegalAccessException, NoSuchFieldException {
        Field inviteNumberField = Utils.getPrivateField(ChatListView.class, "mInviteContainer");
        LinearLayout layout = (LinearLayout) inviteNumberField.get(mChatListView);
        assertEquals(expect, layout.getChildCount());
    }

    private void assertChatNumber(int expect) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Field chatContainerField = Utils.getPrivateField(ChatListView.class, "mChatContainer");
        ViewGroup chatContainer = (ViewGroup) chatContainerField.get(mChatListView);
        assertEquals(expect, chatContainer.getChildCount() - 1);
    }
    
    private boolean judgeChatNumber(int expect) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Field chatContainerField = Utils.getPrivateField(ChatListView.class, "mChatContainer");
        ViewGroup chatContainer = (ViewGroup) chatContainerField.get(mChatListView);
        return (expect==chatContainer.getChildCount() - 1);
    }

    /**
     * Mock chat session that has a mock session-id and a mock participant list
     */
    private static class MockChatSession implements IChatSession {

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
            return MOCK_CALLER;
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
        public boolean isSessionIdle() throws RemoteException {
            return false;
        }

        @Override
        public boolean isGroupChat() throws RemoteException {
            return true;
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

		@Override
		public int getSessionDirection()
		       throws RemoteException  {
			// TODO Auto-generated method stub
            return 0;
		}

    }

    /**
     * Prepare some chats (groupchat/one2onechat),and test the context menu is
     * created correctly.
     * 
     * @throws Throwable
     */
    public void testCase4_CreateContextMenu() throws Throwable {
        // the index 2 is a gorupChat
        ParcelUuid firstChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> firstParticipants = new ArrayList<Participant>();
        firstParticipants.add(new Participant(MOCK_PARTICIPANT_ONE, ""));
        firstParticipants.add(new Participant(MOCK_PARTICIPANT_TWO, ""));
        ModelImpl.getInstance().addChat(firstParticipants, firstChatTag, null);
        getInstrumentation().waitForIdleSync();

        // the index 1 is a first One2OneChat
        ParcelUuid secondChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> secondParticipants = new ArrayList<Participant>();
        secondParticipants.add(new Participant(MOCK_PARTICIPANT_TWO, ""));
        ModelImpl.getInstance().addChat(secondParticipants, secondChatTag, null);
        getInstrumentation().waitForIdleSync();

        // the index 0 is another One2OneChat
        ParcelUuid thirdChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> thirdParticipants = new ArrayList<Participant>();
        thirdParticipants.add(new Participant(MOCK_PARTICIPANT_THREE, ""));
        ModelImpl.getInstance().addChat(thirdParticipants, thirdChatTag, null);
        getInstrumentation().waitForIdleSync();

        // get chat container
        Field chatContainerField = Utils.getPrivateField(ChatListView.class, "mChatContainer");
        ViewGroup chatContainer = (ViewGroup) chatContainerField.get(mChatListView);
        assertNotNull(chatContainer);
        ParcelUuid currentTag = null;
        String contentMenuDelete = "CONTEXT_MENU_DELETE";
        Field fieldDelete = ChatListFragment.class.getDeclaredField(contentMenuDelete);
        fieldDelete.setAccessible(true);
        final int fieldDeleteValue = fieldDelete.getInt(mFragment);

        String contentMenuBlock = "CONTEXT_MENU_BLOCK";
        Field fieldBlock = ChatListFragment.class.getDeclaredField(contentMenuBlock);
        fieldBlock.setAccessible(true);
        final int fieldBlockValue = fieldBlock.getInt(mFragment);

        // Long click the first item and popup the context menu, then click its
        // first item("Delete").
        final View firstView = (View) chatContainer.getChildAt(0);
        assertNotNull(firstView);

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                firstView.setFocusable(true);
                firstView.setLongClickable(true);
                firstView.requestFocus();
            }
        });
        getInstrumentation().invokeContextMenuAction(mActivity, fieldDeleteValue, 0);
        getInstrumentation().waitForIdleSync();

        final DialogFragment confirmOneOneCloseDialog = (DialogFragment) mActivity
                .getFragmentManager().findFragmentByTag("ConfirmOneOneCloseDialog");
        assertNotNull(confirmOneOneCloseDialog);
        assertFalse(confirmOneOneCloseDialog.isHidden());

        // click the ConfirmOneOneCloseDialog OK button.
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                ((AlertDialog) confirmOneOneCloseDialog.getDialog()).getButton(
                        AlertDialog.BUTTON_POSITIVE).performClick();
            }
        });

        getInstrumentation().waitForIdleSync();
        assertNull(ModelImpl.getInstance().getChat(thirdChatTag));
        confirmOneOneCloseDialog.dismiss();
        getInstrumentation().waitForIdleSync();

        // Long click the second item and popup the context menu, then click its
        // second item("Block").
        final View secondView = (View) chatContainer.getChildAt(0);
        assertNotNull(secondView);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                secondView.setFocusable(true);
                secondView.setLongClickable(true);
                secondView.requestFocus();
            }
        });
        getInstrumentation().invokeContextMenuAction(mActivity, fieldBlockValue, 0);
        getInstrumentation().waitForIdleSync();
        final DialogFragment confirmOneOneBlockDialog = (DialogFragment) mActivity
                .getFragmentManager().findFragmentByTag("ConfirmOneOneBlockDialog");
        assertNotNull(confirmOneOneBlockDialog);
        assertFalse(confirmOneOneBlockDialog.isHidden());

        // click the ConfirmOneOneBlockDialog OK button.
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                ((AlertDialog) confirmOneOneBlockDialog.getDialog()).getButton(
                        AlertDialog.BUTTON_POSITIVE).performClick();
            }
        });
        getInstrumentation().waitForIdleSync();
        assertTrue(ContactsManager.getInstance().getImBlockedContactsFromLocal()
                .contains(MOCK_PARTICIPANT_TWO));
        confirmOneOneBlockDialog.dismiss();
        ContactsManager.getInstance().unblockContact(MOCK_PARTICIPANT_TWO);
        getInstrumentation().waitForIdleSync();

        // Long click the third item and popup the context menu, then click its
        // first item("Delete").
        final View thirdView = (View) chatContainer.getChildAt(0);
        assertNotNull(thirdView);

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                thirdView.setFocusable(true);
                thirdView.setLongClickable(true);
                thirdView.requestFocus();
            }
        });
        getInstrumentation().invokeContextMenuAction(mActivity, fieldDeleteValue, 0);
        getInstrumentation().waitForIdleSync();
        final DialogFragment confirmGroupDeleteDialog = (DialogFragment) mActivity
                .getFragmentManager().findFragmentByTag("ConfirmGroupDeleteDialog");
        assertNotNull(confirmGroupDeleteDialog);
        assertFalse(confirmGroupDeleteDialog.isHidden());

        // click the ConfirmGroupDeleteDialog OK button.
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                ((AlertDialog) confirmGroupDeleteDialog.getDialog()).getButton(
                        AlertDialog.BUTTON_POSITIVE).performClick();
            }
        });

        getInstrumentation().waitForIdleSync();
        assertNull(ModelImpl.getInstance().getChat(firstChatTag));

        confirmGroupDeleteDialog.dismiss();
        getInstrumentation().waitForIdleSync();
    }

    /**
     * Prepare some chats (groupchat/one2onechat),and test when click one of them
     * will open correct chat window.
     * 
     * @throws Throwable
     */
    public void testCase5_testItemClick() throws Throwable {
        //the index of gorupChat is 2
        ParcelUuid firstChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> firstParticipants = new ArrayList<Participant>();
        firstParticipants.add(new Participant(MOCK_PARTICIPANT_ONE, ""));
        firstParticipants.add(new Participant(MOCK_PARTICIPANT_TWO, ""));
        ModelImpl.getInstance().addChat(firstParticipants, firstChatTag, null);
        getInstrumentation().waitForIdleSync();

        //the index of first One2OneChat is 1
        ParcelUuid secondChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> secondParticipants = new ArrayList<Participant>();
        secondParticipants.add(new Participant(MOCK_PARTICIPANT_TWO, ""));
        ModelImpl.getInstance().addChat(secondParticipants, secondChatTag, null);
        getInstrumentation().waitForIdleSync();

        //the index of second One2OneChat is 0
        ParcelUuid thirdChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> thirdParticipants = new ArrayList<Participant>();
        thirdParticipants.add(new Participant(MOCK_PARTICIPANT_THREE, ""));
        ModelImpl.getInstance().addChat(thirdParticipants, thirdChatTag, null);
        getInstrumentation().waitForIdleSync();

        //get chat container
        Field chatContainerField = Utils.getPrivateField(ChatListView.class, "mChatContainer");
        ViewGroup chatContainer = (ViewGroup) chatContainerField.get(mChatListView);
        assertNotNull(chatContainer);

        // click the second One2OneChat item
        final View firstView = (View) chatContainer.getChildAt(0);
        assertNotNull(firstView);
        ActivityMonitor am = getInstrumentation().addMonitor(ChatScreenActivity.class.getName(),
                null, false);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                mChatListView.onClick(firstView);
            }
        });
        getInstrumentation().waitForIdleSync();
        Activity chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(am, 5000l);
        try {
            assertNotNull(chatScreenActivity);
            currentFragment = waitForChatFragment(chatScreenActivity);
            assertEquals(thirdChatTag, currentFragment.getChatFragmentTag());
        } finally {
            getInstrumentation().removeMonitor(am);
            if (chatScreenActivity != null) {
                chatScreenActivity.finish();
            }
        }

        // click the groupChat item
        final View thirdView = (View) chatContainer.getChildAt(2);
        assertNotNull(thirdView);
        am = getInstrumentation().addMonitor(ChatScreenActivity.class.getName(), null, false);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                mChatListView.onClick(thirdView);
            }
        });
        getInstrumentation().waitForIdleSync();
        
        chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(am, 5000l);
        try {
            assertTrue(chatScreenActivity instanceof ChatScreenActivity);
            currentFragment = waitForChatFragment(chatScreenActivity);
            assertEquals(firstChatTag, currentFragment.getChatFragmentTag());
            
            Field chatWindowManagerField = Utils.getPrivateField(ChatScreenActivity.class,"mChatWindowManager");
            ChatWindowManager windowManager = (ChatWindowManager)chatWindowManagerField.get(chatScreenActivity);
            assertNotNull(windowManager);
            UnreadMessagesContainer.getInstance().switchTo(thirdChatTag, windowManager);
            getInstrumentation().waitForIdleSync();
            currentFragment = waitForChatFragment(chatScreenActivity);
            assertEquals(thirdChatTag, currentFragment.getChatFragmentTag());
        } finally {
            getInstrumentation().removeMonitor(am);
            if (chatScreenActivity != null) {
                chatScreenActivity.finish();
            }
        }
        ModelImpl.getInstance().removeChat(firstChatTag);
        ModelImpl.getInstance().removeChat(secondChatTag);
        ModelImpl.getInstance().removeChat(thirdChatTag);
        currentFragment = null;
    }

    /**
     * Test the getChatPairWithTag function.
     * 
     * @throws Throwable
     */
    public void testCase6_getChatPairWithTag() throws Throwable {
        Field dataField = Utils.getPrivateField(ChatListView.class, "mData");
        HashMap<View, Object> data = (HashMap<View, Object>) dataField.get(mChatListView);
        Method getChatPairWithTag = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Object chatWindowTag = null;
        HashMap<View, Object> tmpData = data;

        Pair<View, ChatsStruct> result = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);
        assertNull(result);

        data.clear();
        chatWindowTag = new Object();
        result = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(mChatListView, chatWindowTag);
        assertNull(result);

        data.put(null, new ChatsStruct(false, null));
        result = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(mChatListView, chatWindowTag);
        assertNull(result);
        data.clear();

        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatWindowTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);
        ChatsStruct tmp = findPair.second;
        data.put(findPair.first, null);
        result = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(mChatListView, chatWindowTag);
        assertNull(result);
        data.put(findPair.first, tmp);
        // Test remove chat window
        data = tmpData;
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    /**
     * Test the getParticipantNumandContacts function.
     * 
     * @throws Throwable
     */
    public void testCase7_getParticipantNumandContacts() throws Throwable {
        Method getParticipantNum = Utils.getPrivateMethod(ChatsStruct.class, "getParticipantNum");
        Method getChatPairWithTag = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Object chatWindowTag = new Object();
        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatWindowTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);
        ChatsStruct instance = findPair.second;
        Field isGroupChatField = ChatsStruct.class.getDeclaredField("mIsGroupChat");
        isGroupChatField.setAccessible(true);
        Boolean isGroupChat = (Boolean) isGroupChatField.get(instance);
        Field chatMapField = Utils.getPrivateField(ChatsStruct.class, "mChatMap");
        Object oldChatMap = chatMapField.get(instance);

        isGroupChatField.set(instance, true);
        Integer result = (Integer) getParticipantNum.invoke(instance);
        TreeSet<String> resultContacts = (TreeSet<String>) instance.getContacts();
        assertNull(resultContacts);
        assertEquals(0, result.intValue());

        Utils.getPrivateField(ChatsStruct.class, "mChatMap").set(instance, null);
        result = (Integer) getParticipantNum.invoke(instance);
        resultContacts = (TreeSet<String>) instance.getContacts();
        assertNull(resultContacts);
        assertEquals(0, result.intValue());

        // Test remove chat window
        Utils.getPrivateField(ChatsStruct.class, "mChatMap").set(instance, oldChatMap);
        isGroupChatField.set(instance, isGroupChat);
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    /**
     * Test the getOne2OneDisplayName function.
     * 
     * @throws Throwable
     */
    public void testCase8_getOne2OneDisplayName() throws Throwable {
        Method getChatPairWithTag = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Object chatWindowTag = new Object();
        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatWindowTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);
        ChatsStruct instance = findPair.second;
        Field chatMapField = Utils.getPrivateField(ChatsStruct.class, "mChatMap");
        Object oldChatMap = chatMapField.get(instance);

        HashMap<String, Object> chatMap = (HashMap<String, Object>) oldChatMap;
        chatMap.put("number", null);
        chatMapField.set(instance, chatMap);

        Method getOne2OneDisplayName = Utils.getPrivateMethod(ChatsStruct.class,
                "getOne2OneDisplayName");
        String result = (String) getOne2OneDisplayName.invoke(instance);
        assertNull(result);

        chatMapField.set(instance, null);
        result = (String) getOne2OneDisplayName.invoke(instance);
        assertNull(result);

        // Test remove chat window
        Utils.getPrivateField(ChatsStruct.class, "mChatMap").set(instance, oldChatMap);
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    /**
     * Test the getGroupDisplayName function.
     * 
     * @throws Throwable
     */
    public void testCase9_getGroupDisplayName() throws Throwable {
        Method getChatPairWithTag = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Object chatWindowTag = new Object();
        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatWindowTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);
        ChatsStruct instance = findPair.second;
        Field chatMapField = Utils.getPrivateField(ChatsStruct.class, "mChatMap");
        Object oldChatMap = chatMapField.get(instance);

        Object chatMap = oldChatMap;
        Method getGroupDisplayName = Utils.getPrivateMethod(ChatsStruct.class,
                "getGroupDisplayName");
        String result = (String) getGroupDisplayName.invoke(instance);
        assertNull(result);

        chatMapField.set(instance, null);
        result = (String) getGroupDisplayName.invoke(instance);
        assertNull(result);

        // Test remove chat window
        Utils.getPrivateField(ChatsStruct.class, "mChatMap").set(instance, oldChatMap);
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    /**
     * Test the removeInviteItem function.
     * 
     * @throws Throwable
     */
    public void testCase10_removeInviteItem() throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(null);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(MOCK_INVITATION_STRUCT);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    mChatListView.removeInviteItem(MOCK_SESSION_ID);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(MOCK_INVITATION_STRUCT);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        Field dataField = Utils.getPrivateField(ChatListView.class, "mData");
        HashMap<View, Object> data = (HashMap<View, Object>) dataField.get(mChatListView);
        Field inviteNumberField = Utils.getPrivateField(ChatListView.class, "mInviteContainer");
        LinearLayout layout = (LinearLayout) inviteNumberField.get(mChatListView);
        View view = layout.getChildAt(0);
        data.put(view, null);
        dataField.set(mChatListView, data);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    mChatListView.removeInviteItem(MOCK_SESSION_ID);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        data.put(view, MOCK_INVITATION_STRUCT);
        dataField.set(mChatListView, data);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mFragment.onRemovedGroupInvite(MOCK_SESSION_ID);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
    }

    /**
     * Test the updateChats function.
     * 
     * @throws Throwable
     */
    public void testCase11_updateChats() throws Throwable {
        Field dataField = Utils.getPrivateField(ChatListView.class, "mData");
        HashMap<View, Object> data = (HashMap<View, Object>) dataField.get(mChatListView);
        HashMap<View, Object> oldData = data;
        Method getChatPairWithTag = Utils.getPrivateMethod(ChatListView.class,
                "getChatPairWithTag", Object.class);
        Method updateChats = Utils.getPrivateMethod(ChatListView.class, "updateChats", View.class,
                ChatsStruct.class);

        Object chatWindowTag = new Object();
        IOne2OneChatWindow oneOneChatWindow = mFragment.addOne2OneChatWindow(chatWindowTag,
                new Participant(MOCK_PARTICIPANT_ONE, ""));
        getInstrumentation().waitForIdleSync();
        assertChatNumber(1);
        Pair<View, ChatsStruct> findPair = (Pair<View, ChatsStruct>) getChatPairWithTag.invoke(
                mChatListView, chatWindowTag);

        ChatsStruct tmp = findPair.second;
        Utils.getPrivateField(tmp.getClass(), "mUnreadMessageNum").set(tmp, 100);
        Utils.getPrivateField(tmp.getClass(), "mIsFileTransfer").set(tmp, true);
        data.put(findPair.first, tmp);
        updateChats.invoke(mChatListView, findPair.first, tmp);
        getInstrumentation().waitForIdleSync();
        TextView unreadMessage = (TextView) findPair.first.findViewById(R.id.unread_message_num);
        ImageView fileTransfer = (ImageView) findPair.first.findViewById(R.id.chat_file_transfer);
        assertEquals(36, unreadMessage.getWidth());
        assertEquals("99+", unreadMessage.getText());
        assertEquals(View.VISIBLE, fileTransfer.getVisibility());

        Utils.getPrivateField(tmp.getClass(), "mIsFileTransfer").set(tmp, false);
        data.put(findPair.first, tmp);
        updateChats.invoke(mChatListView, findPair.first, tmp);
        getInstrumentation().waitForIdleSync();
        fileTransfer = (ImageView) findPair.first.findViewById(R.id.chat_file_transfer);
        assertEquals(View.GONE, fileTransfer.getVisibility());

        // Test remove chat window
        data = oldData;
        mFragment.removeChatWindow(oneOneChatWindow);
        getInstrumentation().waitForIdleSync();
        assertChatNumber(0);
    }

    public void testCase12_setFunction() throws Throwable {
        InvitationStruct test = new InvitationStruct(null, new Date(), null, new Intent(),
                new MockChatSession());
        test.setChatSession(null);
        assertNull(test.getChatSession());
        test.setDate(null);
        Utils.getPrivateField(InvitationStruct.class, "mDate").set(test, null);
        assertNull(test.getDate());
        test.setDrawable(null);
        assertNull(test.getDrawable());
        test.setInformation("test set fuinction!");
        assertEquals("test set fuinction!", test.getInformation());
        test.setIntent(null);
        assertNull(test.getIntent());
        test = null;
    }

    /**
     * Test the onAttachedToWindow and onDetachedFromWindow function.
     * 
     * @throws Throwable
     */
    public void testCase13_AttachedToWindowandDetachedFromWindow() throws Throwable {
        Method onDisplayNameChanged = Utils.getPrivateMethod(ChatListView.class,
                "onDisplayNameChanged");
        onDisplayNameChanged.invoke(mChatListView);
        Field listField = Utils.getPrivateField(ContactsListManager.class, "LISTENER_LIST");
        Method onAttachedToWindow = Utils
                .getPrivateMethod(ChatListView.class, "onAttachedToWindow");
        onAttachedToWindow.invoke(mChatListView);
        List<WeakReference<OnDisplayNameChangedListener>> list = (List<WeakReference<OnDisplayNameChangedListener>>) listField
                .get(ContactsListManager.getInstance());
        assertEquals(1, list.size());
        Method onDetachedFromWindow = Utils.getPrivateMethod(ChatListView.class,
                "onDetachedFromWindow");
        onDetachedFromWindow.invoke(mChatListView);
        list = (List<WeakReference<OnDisplayNameChangedListener>>) listField
                .get(ContactsListManager.getInstance());
        assertEquals(0, list.size());
    }

    /**
     * Test the onAddedGroupInvite function.
     * 
     * @throws Throwable
     */
    public void testCase14_onAddedGroupInvite() throws Throwable {
        runTestOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                mFragment.onAddedGroupInvite(null,null);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
        
        RcsNotification instance = RcsNotification.getInstance();
        Field tempGroupInvitationInfosField = Utils.getPrivateField(RcsNotification.class,"mTempGroupInvitationInfos");
        Object old = tempGroupInvitationInfosField.get(instance);
        ConcurrentHashMap<IChatSession, Intent> tmp = (ConcurrentHashMap<IChatSession, Intent>)old;
        
        final Intent intent = new Intent();
        intent.putExtra("notify_information", "Test onAddedGroupInvite!");
        final MockChatSession chatSession = new MockChatSession();
        tmp.put(chatSession, intent);
        tempGroupInvitationInfosField.set(instance, tmp);
        Method loadGroupInvitation = Utils.getPrivateMethod(ChatListFragment.class,"loadGroupInvitation");
        loadGroupInvitation.invoke(mFragment);
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mFragment.onRemovedGroupInvite(MOCK_SESSION_ID);
            }
        });
        instance.removeGroupInvite(MOCK_SESSION_ID);
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
        
        Field activityField = Utils.getPrivateField(ChatListFragment.class ,"mActivity");
        Activity oldActivity = (Activity)activityField.get(mFragment);
        activityField.set(mFragment, null);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mFragment.onAddedGroupInvite(chatSession,intent);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);

        activityField.set(mFragment, oldActivity);
    }

    public void testCase15_getChatSession() throws Throwable {
        assertInvitationNumber(0);
        Context mContext = getInstrumentation().getTargetContext();
        MockMessagingApi messageApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), messageApi);
        Method getChatSession = Utils.getPrivateMethod(ChatListFragment.class,
                "getChatSession",String.class);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(MOCK_INVITATION_STRUCT);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        IChatSession result = (IChatSession)getChatSession.invoke(mFragment,MOCK_SESSION_ID);
        assertEquals(MOCK_CHAT_SESSION, result);
        
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.removeInviteItem(MOCK_SESSION_ID);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
        
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), null);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(MOCK_INVITATION_STRUCT);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        result = (IChatSession) getChatSession.invoke(mFragment, MOCK_SESSION_ID);
        assertNull(result);

        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.removeInviteItem(MOCK_SESSION_ID);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(0);
    }

    public void testCase16_onSelectedItem() throws Throwable {
        final InvitationStruct invitationStruct = new InvitationStruct(null, MOCK_DATE, MOCK_INFORMATION,
                new Intent(), MOCK_CHAT_SESSION);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChatListView.addInviteItem(invitationStruct);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertInvitationNumber(1);
        Field invitationContainerField = Utils.getPrivateField(ChatListView.class,
                "mInviteContainer");
        LinearLayout invitationContainer = (LinearLayout) invitationContainerField
                .get(mChatListView);
        assertNotNull(invitationContainer);
        final View view = invitationContainer.getChildAt(0);
        assertNotNull(view);
        ActivityMonitor am = getInstrumentation().addMonitor(InvitationDialog.class.getName(),
                null, false);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                mChatListView.onClick(view);
            }
        });
        getInstrumentation().waitForIdleSync();
        Activity activity = am.waitForActivityWithTimeout(TIME_OUT);
        try {
            assertTrue(activity instanceof InvitationDialog);
        } finally {
            getInstrumentation().removeMonitor(am);
            if (activity != null) {
                activity.finish();
            }
        }
    }

    private ChatFragment waitForChatFragment(Activity activity) throws Throwable {
        ChatFragment fragment = (ChatFragment) activity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        long startTime = System.currentTimeMillis();
        while (fragment == null || fragment == currentFragment) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            fragment = (ChatFragment) activity.getFragmentManager().findFragmentById(
                    R.id.chat_content);
        }
        return fragment;
    }

    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            return MOCK_CHAT_SESSION;
        }
    }
}
