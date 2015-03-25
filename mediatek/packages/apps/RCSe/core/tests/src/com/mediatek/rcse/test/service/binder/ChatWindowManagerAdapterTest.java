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

package com.mediatek.rcse.test.service.binder;

import android.content.Intent;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.test.ServiceTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.binder.ChatEventStructForBinder;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteChatWindowManager;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.WindowBinder;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.BaseChatWindow;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.FileTransferAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.GroupChatWindowAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.One2OneChatWindowAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.ReceivedChatMessageAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.SentChatMessageAdapter;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;

/**
 * This class is used to test ChatWindowManagerAdapter
 */
public class ChatWindowManagerAdapterTest extends ServiceTestCase<ApiService> {
    private static final String TAG = "ChatWindowManagerAdapterTest";
    private static final String MOCK_NUMBER = "+34200000253";
    private static final String MOCK_NUMBER2 = "+34200000254";
    private static final String MOCK_NAME = "Jack";
    private static final String MOCK_NAME2 = "James";
    private static final String MOCK_TAG = "mocktag";
    private static final String MOCK_TAG2 = "mocktag2";
    private static final String MOCK_FILEPATH = "mock_file_path";
    private static final int MOCK_SIZE = 10;
    private static final int MOCK_PROGRESS = 50;
    private WindowBinder mBinder = null;
    private MockRemoteChatWindowManager mMockRemoteChatWindowManager = null;
    private List<ChatWindowManagerAdapter> mChatWindowManagerAdapters = null;
    private ChatWindowManagerAdapter mRemoteChatWindowManager = null;

    public ChatWindowManagerAdapterTest() {
        super(ApiService.class);
    }

    @Override
    public void setUp() throws Exception {
        Logger.v(TAG, "setUp()");
        super.setUp();
        // Get WindowBinder from ApiService
        Intent intent = new Intent(getSystemContext(), ApiService.class);
        intent.setAction(IRemoteWindowBinder.class.getName());
        IBinder service = bindService(intent);
        assertTrue(service.isBinderAlive());
        mBinder = (WindowBinder) IRemoteWindowBinder.Stub.asInterface(service);
        assertNotNull(mBinder);
        mMockRemoteChatWindowManager = new MockRemoteChatWindowManager();
        Field fieldRemoteChatWindowManagers = Utils.getPrivateField(WindowBinder.class,
                "mChatWindowManagerAdapters");
        mChatWindowManagerAdapters = (List<ChatWindowManagerAdapter>) fieldRemoteChatWindowManagers
                .get(mBinder);
        // Add RemoteChatWindowManager to binder
        int startSize = mChatWindowManagerAdapters.size();
        mBinder.addChatWindowManager(mMockRemoteChatWindowManager, false);
        assertEquals(startSize + 1, mChatWindowManagerAdapters.size());
        mRemoteChatWindowManager = mChatWindowManagerAdapters.get(0);
        assertNotNull(mRemoteChatWindowManager);
    }

    public void tearDown() throws Exception {
        Logger.v(TAG, "tearDown()");
        // Remove RemoteChatWindowManager of binder
        mBinder.removeChatWindowManager(mMockRemoteChatWindowManager);
        mBinder = null;
        mMockRemoteChatWindowManager = null;
        mChatWindowManagerAdapters = null;
        mRemoteChatWindowManager = null;
        super.tearDown();
    }

    /**
     * Test the function addOne2OneChatWindow()
     */
    public void testCase1_AddOne2OneChatWindow() throws Throwable {
        Logger.v(TAG, "testCase1_AddOne2OneChatWindow()");
        mMockRemoteChatWindowManager.isAddOne2OneChatWindowCalled = false;
        IOne2OneChatWindow remoteOne2OneChatWindow = (IOne2OneChatWindow) mRemoteChatWindowManager
                .addOne2OneChatWindow(MOCK_TAG, new Participant(MOCK_NUMBER, MOCK_NAME));
        assertNotNull(remoteOne2OneChatWindow);
        assertTrue(mMockRemoteChatWindowManager.isAddOne2OneChatWindowCalled);
        mMockRemoteChatWindowManager.isAddOne2OneChatWindowCalled = false;
    }

    /**
     * Test the function addGroupChatWindow()
     */
    public void testCase2_AddGroupChatWindow() throws Throwable {
        Logger.v(TAG, "testCase2_AddOne2OneChatWindow()");
        mMockRemoteChatWindowManager.isAddGroupChatWindowCalled = false;
        IGroupChatWindow remoteGroupChatWindow = (IGroupChatWindow) mRemoteChatWindowManager
                .addGroupChatWindow(MOCK_TAG2, getParticipantList());
        assertNotNull(remoteGroupChatWindow);
        assertTrue(mMockRemoteChatWindowManager.isAddGroupChatWindowCalled);
        mMockRemoteChatWindowManager.isAddGroupChatWindowCalled = false;
    }

    /**
     * Test the function switchChatWindowByTag()
     */
    public void testCase3_SwitchChatWindowByTag() throws Throwable {
        Logger.v(TAG, "testCase3_SwitchChatWindowByTag()");
        mMockRemoteChatWindowManager.isSwitchChatWindowByTagCalled = false;
        ParcelUuid parcelUuid = new ParcelUuid(new UUID(1, 1));
        mRemoteChatWindowManager.switchChatWindowByTag(parcelUuid);
        assertTrue(mMockRemoteChatWindowManager.isSwitchChatWindowByTagCalled);
        mMockRemoteChatWindowManager.isSwitchChatWindowByTagCalled = false;
    }

    /**
     * Test the function removeChatWindow()
     */
    public void testCase4_RemoveChatWindow() throws Throwable {
        Logger.v(TAG, "testCase4_RemoveChatWindow()");
        assertFalse(mRemoteChatWindowManager.removeChatWindow(null));
        // Add One2OneChatWindow
        IOne2OneChatWindow remoteOne2OneChatWindow = (IOne2OneChatWindow) mRemoteChatWindowManager
                .addOne2OneChatWindow(MOCK_TAG, new Participant(MOCK_NUMBER, MOCK_NAME));
        // Remove One2OneChatWindow
        mMockRemoteChatWindowManager.isRemoveOne2OneChatWindowCalled = false;
        boolean removeOne2OneChatWindowResult = mRemoteChatWindowManager
                .removeChatWindow(remoteOne2OneChatWindow);
        assertTrue(removeOne2OneChatWindowResult);
        assertTrue(mMockRemoteChatWindowManager.isRemoveOne2OneChatWindowCalled);
        mMockRemoteChatWindowManager.isRemoveOne2OneChatWindowCalled = false;
        // Add GroupChatWindow
        IGroupChatWindow remoteGroupChatWindow = (IGroupChatWindow) mRemoteChatWindowManager
                .addGroupChatWindow(MOCK_TAG2, getParticipantList());
        // remove GroupChatWindow
        mMockRemoteChatWindowManager.isRemoveGroupChatWindowCalled = false;
        boolean removeGroupChatWindowResult = mRemoteChatWindowManager
                .removeChatWindow(remoteGroupChatWindow);
        assertTrue(removeGroupChatWindowResult);
        assertTrue(mMockRemoteChatWindowManager.isRemoveGroupChatWindowCalled);
        mMockRemoteChatWindowManager.isRemoveGroupChatWindowCalled = false;
    }

    /**
     * Test the inner class One2OneChatWindowAdapter
     */
    public void testCase5_One2OneChatWindowAdapter() throws Throwable {
        Logger.v(TAG, "testCase5_One2OneChatWindowAdapter()");
        One2OneChatWindowAdapter chatWindowAdapter = getOne2OneChatWindowAdapter();
        IRemoteOne2OneChatWindow remoteChatWindow = (IRemoteOne2OneChatWindow) chatWindowAdapter
                .getChatWindow();
        MockRemoteOne2OneChatWindow mockRemoteChatWindow = (MockRemoteOne2OneChatWindow) remoteChatWindow;
        FileStruct mockFileStruct = new FileStruct("", "", MOCK_SIZE, new Object(), new Date());

        // Test One2OneChatWindowAdapter#addLoadHistoryHeader()
        mockRemoteChatWindow.isAddLoadHistoryHeaderCalled = false;
        chatWindowAdapter.addLoadHistoryHeader(false);
        assertTrue(mockRemoteChatWindow.isAddLoadHistoryHeaderCalled);
        mockRemoteChatWindow.isAddLoadHistoryHeaderCalled = false;

        // Test One2OneChatWindowAdapter#addReceivedFileTransfer()
        mockRemoteChatWindow.isAddReceivedFileTransferCalled = false;
        boolean mServiceAttached=false;
		chatWindowAdapter.addReceivedFileTransfer(mockFileStruct, mServiceAttached);
        assertTrue(mockRemoteChatWindow.isAddReceivedFileTransferCalled);
        mockRemoteChatWindow.isAddReceivedFileTransferCalled = false;

        // Test One2OneChatWindowAdapter#addReceivedMessage()
        mockRemoteChatWindow.isAddReceivedMessageCalled = false;
        chatWindowAdapter.addReceivedMessage(null, false);
        assertTrue(mockRemoteChatWindow.isAddReceivedMessageCalled);
        mockRemoteChatWindow.isAddReceivedMessageCalled = false;

        // Test One2OneChatWindowAdapter#addSentFileTransfer()
        mockRemoteChatWindow.isAddSentFileTransferCalled = false;
        chatWindowAdapter.addSentFileTransfer(mockFileStruct);
        assertTrue(mockRemoteChatWindow.isAddSentFileTransferCalled);
        mockRemoteChatWindow.isAddSentFileTransferCalled = false;

        // Test One2OneChatWindowAdapter#addSentMessage()
        mockRemoteChatWindow.isAddSentMessageCalled = false;
        chatWindowAdapter.addSentMessage(null, 0);
        assertTrue(mockRemoteChatWindow.isAddSentMessageCalled);
        mockRemoteChatWindow.isAddSentMessageCalled = false;

        // Test One2OneChatWindowAdapter#removeAllMessages()
        mockRemoteChatWindow.isRemoveAllMessagesCalled = false;
        chatWindowAdapter.removeAllMessages();
        assertTrue(mockRemoteChatWindow.isRemoveAllMessagesCalled);
        mockRemoteChatWindow.isRemoveAllMessagesCalled = false;

        // Test One2OneChatWindowAdapter#setFileTransferEnable()
        mockRemoteChatWindow.isSetFileTransferEnableCalled = false;
        chatWindowAdapter.setFileTransferEnable(0);
        assertTrue(mockRemoteChatWindow.isSetFileTransferEnableCalled);
        mockRemoteChatWindow.isSetFileTransferEnableCalled = false;

        // Test One2OneChatWindowAdapter#setIsComposing()
        mockRemoteChatWindow.isSetIsComposingCalled = false;
        chatWindowAdapter.setIsComposing(false);
        assertTrue(mockRemoteChatWindow.isSetIsComposingCalled);
        mockRemoteChatWindow.isSetIsComposingCalled = false;

        // Test One2OneChatWindowAdapter#setRemoteOfflineReminder()
        mockRemoteChatWindow.isSetRemoteOfflineReminderCalled = false;
        chatWindowAdapter.setRemoteOfflineReminder(false);
        assertTrue(mockRemoteChatWindow.isSetRemoteOfflineReminderCalled);
        mockRemoteChatWindow.isSetRemoteOfflineReminderCalled = false;

        // Test One2OneChatWindowAdapter#updateAllMsgAsRead()
        mockRemoteChatWindow.isUpdateAllMsgAsReadCalled = false;
        chatWindowAdapter.updateAllMsgAsRead();
        assertTrue(mockRemoteChatWindow.isUpdateAllMsgAsReadCalled);
        mockRemoteChatWindow.isUpdateAllMsgAsReadCalled = false;
    }

    /**
     * Test the inner class GroupChatWindowAdapter
     */
    public void testCase6_GroupChatWindowAdapter() throws Throwable {
        Logger.v(TAG, "testCase6_GroupChatWindowAdapter()");
        GroupChatWindowAdapter chatWindowAdapter = getGroupChatWindowAdapter();
        IRemoteGroupChatWindow remoteChatWindow = (IRemoteGroupChatWindow) chatWindowAdapter
                .getChatWindow();
        MockRemoteGroupChatWindow mockRemoteChatWindow = (MockRemoteGroupChatWindow) remoteChatWindow;

        // Test GroupChatWindowAdapter#addChatEventInformation()
        ChatEventStruct chatEventStruct = new ChatEventStruct(Information.JOIN, new Object(),
                new Date());
        mockRemoteChatWindow.isAddChatEventInformationCalled = false;
        chatWindowAdapter.addChatEventInformation(chatEventStruct);
        assertTrue(mockRemoteChatWindow.isAddChatEventInformationCalled);
        mockRemoteChatWindow.isAddChatEventInformationCalled = false;

        // Test GroupChatWindowAdapter#setIsComposing()
        mockRemoteChatWindow.isSetIsComposingCalled = false;
        chatWindowAdapter.setIsComposing(false, new Participant(MOCK_NUMBER, MOCK_NAME));
        assertTrue(mockRemoteChatWindow.isSetIsComposingCalled);
        mockRemoteChatWindow.isSetIsComposingCalled = false;

        // Test GroupChatWindowAdapter#setIsRejoining()
        mockRemoteChatWindow.isSetIsRejoiningCalled = false;

        assertTrue(mockRemoteChatWindow.isSetIsRejoiningCalled);
        mockRemoteChatWindow.isSetIsRejoiningCalled = false;

        // Test GroupChatWindowAdapter#updateParticipants()
        mockRemoteChatWindow.isUpdateParticipantsCalled = false;
        chatWindowAdapter.updateParticipants(getParticipantList());
        assertTrue(mockRemoteChatWindow.isUpdateParticipantsCalled);
        mockRemoteChatWindow.isUpdateParticipantsCalled = false;

        // Test GroupChatWindowAdapter#addLoadHistoryHeader()
        mockRemoteChatWindow.isAddLoadHistoryHeaderCalled = false;
        chatWindowAdapter.addLoadHistoryHeader(false);
        assertTrue(mockRemoteChatWindow.isAddLoadHistoryHeaderCalled);
        mockRemoteChatWindow.isAddLoadHistoryHeaderCalled = false;

        // Test GroupChatWindowAdapter#addReceivedMessage()
        mockRemoteChatWindow.isAddReceivedMessageCalled = false;
        chatWindowAdapter.addReceivedMessage(null, false);
        assertTrue(mockRemoteChatWindow.isAddReceivedMessageCalled);
        mockRemoteChatWindow.isAddReceivedMessageCalled = false;

        // Test GroupChatWindowAdapter#addSentMessage()
        mockRemoteChatWindow.isAddSentMessageCalled = false;
        chatWindowAdapter.addSentMessage(null, 0);
        assertTrue(mockRemoteChatWindow.isAddSentMessageCalled);
        mockRemoteChatWindow.isAddSentMessageCalled = false;

        // Test GroupChatWindowAdapter#removeAllMessages()
        mockRemoteChatWindow.isRemoveAllMessagesCalled = false;
        chatWindowAdapter.removeAllMessages();
        assertTrue(mockRemoteChatWindow.isRemoveAllMessagesCalled);
        mockRemoteChatWindow.isRemoveAllMessagesCalled = false;

        // Test GroupChatWindowAdapter#updateAllMsgAsRead()
        mockRemoteChatWindow.isUpdateParticipantsCalled = false;
        chatWindowAdapter.updateAllMsgAsRead();
        assertTrue(mockRemoteChatWindow.isUpdateParticipantsCalled);
        mockRemoteChatWindow.isUpdateParticipantsCalled = false;
    }

    /**
     * Test the inner class FileTransferAdapter
     */
    public void testCase7_FileTransferAdapter() throws Throwable {
        Logger.v(TAG, "testCase7_FileTransferAdapter()");
        One2OneChatWindowAdapter chatWindowAdapter = getOne2OneChatWindowAdapter();
        IRemoteOne2OneChatWindow remoteChatWindow = (IRemoteOne2OneChatWindow) chatWindowAdapter
                .getChatWindow();
        MockRemoteOne2OneChatWindow mockRemoteChatWindow = (MockRemoteOne2OneChatWindow) remoteChatWindow;
        FileStruct mockFileStruct = new FileStruct("", "", MOCK_SIZE, new Object(), new Date());
        FileTransferAdapter fileTransferAdapter = (FileTransferAdapter) chatWindowAdapter
                .addReceivedFileTransfer(mockFileStruct, false);

        // Test FileTransferAdapter#setFilePath()
        mockRemoteChatWindow.mockRemoteFileTransfer.isSetFilePathCalled = false;
        fileTransferAdapter.setFilePath(MOCK_FILEPATH);
        assertTrue(mockRemoteChatWindow.mockRemoteFileTransfer.isSetFilePathCalled);

        // Test FileTransferAdapter#setProgress()
        mockRemoteChatWindow.mockRemoteFileTransfer.isSetProgressCalled = false;
        fileTransferAdapter.setProgress(MOCK_PROGRESS);
        assertTrue(mockRemoteChatWindow.mockRemoteFileTransfer.isSetProgressCalled);

        // Test FileTransferAdapter#setStatus()
        mockRemoteChatWindow.mockRemoteFileTransfer.isSetStatusCalled = false;
        fileTransferAdapter.setStatus(Status.FINISHED);
        assertTrue(mockRemoteChatWindow.mockRemoteFileTransfer.isSetStatusCalled);
    }

    /**
     * Test the inner class ReceivedChatMessageAdapter
     */
    public void testCase8_ReceivedChatMessageAdapter() throws Throwable {
        Logger.v(TAG, "testCase8_ReceivedChatMessageAdapter()");
        One2OneChatWindowAdapter chatWindowAdapter = getOne2OneChatWindowAdapter();
        IRemoteOne2OneChatWindow remoteChatWindow = (IRemoteOne2OneChatWindow) chatWindowAdapter
                .getChatWindow();
        MockRemoteOne2OneChatWindow mockRemoteChatWindow = (MockRemoteOne2OneChatWindow) remoteChatWindow;
        ReceivedChatMessageAdapter receivedChatMessageAdapter = (ReceivedChatMessageAdapter) chatWindowAdapter
                .addReceivedMessage(null, false);

        // Test FileTransferAdapter#getId()
        mockRemoteChatWindow.mockRemoteReceivedChatMessage.isGetIdCalled = false;
        receivedChatMessageAdapter.getId();
        assertTrue(mockRemoteChatWindow.mockRemoteReceivedChatMessage.isGetIdCalled);
    }

    /**
     * Test the inner class SentChatMessageAdapter
     */
    public void testCase9_SentChatMessageAdapter() throws Throwable {
        Logger.v(TAG, "testCase9_SentChatMessageAdapter()");
        One2OneChatWindowAdapter chatWindowAdapter = getOne2OneChatWindowAdapter();
        IRemoteOne2OneChatWindow remoteChatWindow = (IRemoteOne2OneChatWindow) chatWindowAdapter
                .getChatWindow();
        MockRemoteOne2OneChatWindow mockRemoteChatWindow = (MockRemoteOne2OneChatWindow) remoteChatWindow;
        SentChatMessageAdapter sentChatMessageAdapter = (SentChatMessageAdapter) chatWindowAdapter
                .addSentMessage(null, 0);

        // Test SentChatMessageAdapter#getId()
        mockRemoteChatWindow.mockRemoteSentChatMessage.isGetIdCalled = false;
        sentChatMessageAdapter.getId();
        assertTrue(mockRemoteChatWindow.mockRemoteSentChatMessage.isGetIdCalled);

        // Test SentChatMessageAdapter#updateDate()
        mockRemoteChatWindow.mockRemoteSentChatMessage.isUpdateDateCalled = false;
        sentChatMessageAdapter.updateDate(new Date());
        assertTrue(mockRemoteChatWindow.mockRemoteSentChatMessage.isUpdateDateCalled);

        // Test SentChatMessageAdapter#updateStatus()
        mockRemoteChatWindow.mockRemoteSentChatMessage.isUpdateStatusCalled = false;
        sentChatMessageAdapter.updateStatus(ChatView.ISentChatMessage.Status.DISPLAYED);
        assertTrue(mockRemoteChatWindow.mockRemoteSentChatMessage.isUpdateStatusCalled);
    }

    /**
     * Use reflect to get One2OneChatWindowAdapter instance
     */
    private One2OneChatWindowAdapter getOne2OneChatWindowAdapter() throws Throwable {
        Logger.v(TAG, "getGroupChatWindowAdapter()");
        // Add One2OneChatWindow
        mRemoteChatWindowManager.addOne2OneChatWindow(MOCK_TAG, new Participant(MOCK_NUMBER,
                MOCK_NAME));
        // Get WindowBinder#chatWindowManagerAdapters[0]
        ChatWindowManagerAdapter chatWindowManagerAdapter = mChatWindowManagerAdapters.get(0);
        // Get ChatWindowManagerAdapter#mRemoteChatWindows
        Field fieldRemoteChatWindows = Utils.getPrivateField(ChatWindowManagerAdapter.class,
                "mRemoteChatWindows");
        List<BaseChatWindow> mRemoteChatWindows = (List<BaseChatWindow>) fieldRemoteChatWindows
                .get(chatWindowManagerAdapter);
        // Get GroupChatWindowAdapter
        One2OneChatWindowAdapter one2OneChatWindowAdapter = (One2OneChatWindowAdapter) mRemoteChatWindows
                .get(0);
        assertNotNull(one2OneChatWindowAdapter);
        return one2OneChatWindowAdapter;
    }

    /**
     * Use reflect to get GroupChatWindowAdapter instance
     */
    private GroupChatWindowAdapter getGroupChatWindowAdapter() throws Throwable {
        Logger.v(TAG, "getGroupChatWindowAdapter()");
        // Add GroupChatWindow
        mRemoteChatWindowManager.addGroupChatWindow(MOCK_TAG2, getParticipantList());
        // Get WindowBinder#chatWindowManagerAdapters[0]
        ChatWindowManagerAdapter chatWindowManagerAdapter = mChatWindowManagerAdapters.get(0);
        // Get ChatWindowManagerAdapter#mRemoteChatWindows
        Field fieldRemoteChatWindows = Utils.getPrivateField(ChatWindowManagerAdapter.class,
                "mRemoteChatWindows");
        List<BaseChatWindow> mRemoteChatWindows = (List<BaseChatWindow>) fieldRemoteChatWindows
                .get(chatWindowManagerAdapter);
        // Get GroupChatWindowAdapter
        GroupChatWindowAdapter groupChatWindowAdapter = (GroupChatWindowAdapter) mRemoteChatWindows
                .get(0);
        assertNotNull(groupChatWindowAdapter);
        return groupChatWindowAdapter;
    }

    /**
     * Create a ParticipantList
     */
    private List<ParticipantInfo> getParticipantList() {
        Logger.v(TAG, "getParticipantList()");
        ParticipantInfo participantInfo1 = new ParticipantInfo(new Participant(MOCK_NUMBER,
                MOCK_NAME), User.STATE_CONNECTED);
        ParticipantInfo participantInfo2 = new ParticipantInfo(new Participant(MOCK_NUMBER2,
                MOCK_NAME2), User.STATE_CONNECTED);
        List<ParticipantInfo> participantList = new ArrayList<ParticipantInfo>();
        participantList.add(participantInfo1);
        participantList.add(participantInfo2);
        return participantList;
    }
}

/**
 * This class is a mock of remote GroupChatWindow
 */
class MockRemoteGroupChatWindow extends IRemoteGroupChatWindow.Stub {
    public boolean isAddChatEventInformationCalled = false;
    public boolean isAddLoadHistoryHeaderCalled = false;
    public boolean isAddReceivedMessageCalled = false;
    public boolean isAddSentMessageCalled = false;
    public boolean isGetSentChatMessageCalled = false;
    public boolean isRemoveAllMessagesCalled = false;
    public boolean isSetIsRejoiningCalled = false;
    public boolean isUpdateParticipantsCalled = false;
    public boolean isSetIsComposingCalled = false;

    @Override
    public int addChatEventInformation(ChatEventStructForBinder chatEventStruct)
            throws RemoteException {
        isAddChatEventInformationCalled = true;
        return 0;
    }

    @Override
    public void addLoadHistoryHeader(boolean showLoader) throws RemoteException {
        isAddLoadHistoryHeaderCalled = true;
    }

    @Override
    public IRemoteReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead)
            throws RemoteException {
        isAddReceivedMessageCalled = true;
        return null;
    }

    @Override
    public IRemoteSentChatMessage addSentMessage(InstantMessage message, int messageTag)
            throws RemoteException {
        isAddSentMessageCalled = true;
        return null;
    }

    @Override
    public IRemoteSentChatMessage getSentChatMessage(String messageId) throws RemoteException {
        isGetSentChatMessageCalled = true;
        return null;
    }

    @Override
    public void removeAllMessages() throws RemoteException {
        isRemoveAllMessagesCalled = true;
    }

    @Override
    public void setIsComposing(boolean isComposing, Participant participant) throws RemoteException {
        isSetIsComposingCalled = true;
    }

   
    @Override
    public void updateAllMsgAsRead() throws RemoteException {
        isUpdateParticipantsCalled = true;
    }

    @Override
    public void updateParticipants(List<ParticipantInfo> participants) throws RemoteException {
        isUpdateParticipantsCalled = true;
    }

    @Override
    public IBinder asBinder() {
        return null;
    }

	@Override
	public void updateChatStatus(int status) throws RemoteException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void addgroupSubject(String subject) throws RemoteException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setFileTransferEnable(int reason) throws RemoteException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public IRemoteFileTransfer addSentFileTransfer(FileStructForBinder file)
			throws RemoteException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public IRemoteFileTransfer addReceivedFileTransfer(
			FileStructForBinder file, boolean isAutoAccept)
			throws RemoteException {
		// TODO Auto-generated method stub
		return null;
	}
}

/**
 * This class is a mock of remote One2OneChatWindow
 */
class MockRemoteOne2OneChatWindow extends IRemoteOne2OneChatWindow.Stub {
    public boolean isAddLoadHistoryHeaderCalled = false;
    public boolean isAddReceivedFileTransferCalled = false;
    public boolean isAddReceivedMessageCalled = false;
    public boolean isAddSentFileTransferCalled = false;
    public boolean isAddSentMessageCalled = false;
    public boolean isGetSentChatMessageCalled = false;
    public boolean isRemoveAllMessagesCalled = false;
    public boolean isSetFileTransferEnableCalled = false;
    public boolean isSetIsComposingCalled = false;
    public boolean isSetRemoteOfflineReminderCalled = false;
    public boolean isUpdateAllMsgAsReadCalled = false;
    public MockRemoteFileTransfer mockRemoteFileTransfer = new MockRemoteFileTransfer();
    public MockRemoteReceivedChatMessage mockRemoteReceivedChatMessage = new MockRemoteReceivedChatMessage();
    public MockRemoteSentChatMessage mockRemoteSentChatMessage = new MockRemoteSentChatMessage();

    @Override
    public void addLoadHistoryHeader(boolean showLoader) throws RemoteException {
        isAddLoadHistoryHeaderCalled = true;
    }

  
    @Override
    public IRemoteReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead)
            throws RemoteException {
        isAddReceivedMessageCalled = true;
        return mockRemoteReceivedChatMessage;
    }

    @Override
    public IRemoteFileTransfer addSentFileTransfer(FileStructForBinder file) throws RemoteException {
        isAddSentFileTransferCalled = true;
        return mockRemoteFileTransfer;
    }

    @Override
    public IRemoteSentChatMessage addSentMessage(InstantMessage message, int messageTag)
            throws RemoteException {
        isAddSentMessageCalled = true;
        return mockRemoteSentChatMessage;
    }

    @Override
    public IRemoteSentChatMessage getSentChatMessage(String messageId) throws RemoteException {
        isGetSentChatMessageCalled = true;
        return mockRemoteSentChatMessage;
    }

    @Override
    public void removeAllMessages() throws RemoteException {
        isRemoveAllMessagesCalled = true;
    }

    @Override
    public void setFileTransferEnable(int reason) throws RemoteException {
        isSetFileTransferEnableCalled = true;
    }

    @Override
    public void setIsComposing(boolean isComposing) throws RemoteException {
        isSetIsComposingCalled = true;
    }

    @Override
    public void setRemoteOfflineReminder(boolean isOffline) throws RemoteException {
        isSetRemoteOfflineReminderCalled = true;
    }

    @Override
    public void updateAllMsgAsRead() throws RemoteException {
        isUpdateAllMsgAsReadCalled = true;
    }

    @Override
    public IBinder asBinder() {
        return null;
    }


	@Override
	public IRemoteFileTransfer addReceivedFileTransfer(
			FileStructForBinder file, boolean isAutoAccept)
			throws RemoteException {
		// TODO Auto-generated method stub
		return null;
	}
}

/**
 * This class is a mock of a remote ChatWindowManager
 */
class MockRemoteChatWindowManager extends IRemoteChatWindowManager.Stub {
    public boolean isAddGroupChatWindowCalled = false;
    public boolean isAddOne2OneChatWindowCalled = false;
    public boolean isRemoveGroupChatWindowCalled = false;
    public boolean isRemoveOne2OneChatWindowCalled = false;
    public boolean isSwitchChatWindowByTagCalled = false;
    public boolean isUpdateMessageTag = false;

    @Override
    public IRemoteGroupChatWindow addGroupChatWindow(String tag,
            List<ParticipantInfo> participantList) throws RemoteException {
        isAddGroupChatWindowCalled = true;
        return new MockRemoteGroupChatWindow();
    }

    @Override
    public IRemoteOne2OneChatWindow addOne2OneChatWindow(String tag, Participant participant)
            throws RemoteException {
        isAddOne2OneChatWindowCalled = true;
        return new MockRemoteOne2OneChatWindow();
    }

    @Override
    public boolean removeGroupChatWindow(IRemoteGroupChatWindow chatWindow) throws RemoteException {
        isRemoveGroupChatWindowCalled = true;
        return true;
    }

    @Override
    public boolean removeOne2OneChatWindow(IRemoteOne2OneChatWindow chatWindow)
            throws RemoteException {
        isRemoveOne2OneChatWindowCalled = true;
        return true;
    }

    @Override
    public void switchChatWindowByTag(String uuidTag) throws RemoteException {
        isSwitchChatWindowByTagCalled = true;
    }

    @Override
    public IBinder asBinder() {
        return null;
    }
}

/**
 * This class is a mock of remote ReceivedChatMessage
 */
class MockRemoteReceivedChatMessage extends IRemoteReceivedChatMessage.Stub {
    public boolean isGetIdCalled = false;

    @Override
    public String getId() throws RemoteException {
        isGetIdCalled = true;
        return null;
    }
}

/**
 * This class is a mock of remote SentChatMessage
 */
class MockRemoteSentChatMessage extends IRemoteSentChatMessage.Stub {
    public boolean isGetIdCalled = false;
    public boolean isUpdateDateCalled = false;
    public boolean isUpdateStatusCalled = false;

    @Override
    public String getId() throws RemoteException {
        isGetIdCalled = true;
        return null;
    }

    @Override
    public void updateDate(long date) throws RemoteException {
        isUpdateDateCalled = true;
    }

    @Override
    public void updateStatus(String status) throws RemoteException {
        isUpdateStatusCalled = true;
    }
}

/**
 * This class is a mock of remote FileTransfer
 */
class MockRemoteFileTransfer extends IRemoteFileTransfer.Stub {
    public boolean isSetFilePathCalled = false;
    public boolean isSetProgressCalled = false;
    public boolean isSetStatusCalled = false;

    @Override
    public void setFilePath(String filePath) throws RemoteException {
        isSetFilePathCalled = true;
    }

    @Override
    public void setProgress(long progress) throws RemoteException {
        isSetProgressCalled = true;
    }

    @Override
    public void setStatus(int status) throws RemoteException {
        isSetStatusCalled = true;
    }

    @Override
    public void updateTag(String transferTag, long transferSize) throws RemoteException {
        // TODO Auto-generated method stub

    }
}
