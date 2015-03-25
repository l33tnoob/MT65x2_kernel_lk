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
import android.database.Cursor;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test SentFileTransferItemBinder
 */
public class SentFileTransferItemBinderTest extends
        ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "SentFileTransferItemBinderTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_NUMBER = "+3402222226";
    private static final Participant MOCK_121_PARTICIPANT = new Participant(MOCK_NUMBER,
            MOCK_NUMBER);
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private static final int DATE_LABEL_POSITION = 0;
    private static final int CURRENT_POSITION = 1;
    private ChatScreenActivity mActivity = null;
    private List mChatWindowManagerList = null;

    public SentFileTransferItemBinderTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() enter");
        super.setUp();
        getInstrumentation().waitForIdleSync();
        Field apiManagerField = Utils.getPrivateField(ApiManager.class, API_MANAGER_INSTANCE);
        apiManagerField.set(ApiManager.class, null);

        Method initializeMethod = Utils.getPrivateMethod(ApiManager.class, API_MANAGER_INITIALIZE,
                Context.class);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());
        
        // Make sure that the fragment can be added.
        ChatScreenWindowContainer.getInstance().clearCurrentStatus();
        
        Intent intent = generateIntent();
        setActivityIntent(intent);
        Field fieldChatWindowManagerList = Utils.getPrivateField(ViewImpl.class,
                "mChatWindowManagerList");
        mChatWindowManagerList = (List) fieldChatWindowManagerList.get(ViewImpl.getInstance());
        int startCount = mChatWindowManagerList.size();
        mActivity = getActivity();
        waitForChatWindowManagerNum(startCount + 1);
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

    private Intent generateIntent() {
        Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(MOCK_121_PARTICIPANT);
        intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    /**
     * Test the function SentFileTransferItemBinderTest#bindView()
     */
    public void testCase01_BindView() throws Throwable {
        Logger.v(TAG, "testCase01_BindView()");
        ChatImpl chat = waitForChat();
        One2OneChatFragment fragment = (One2OneChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertNotNull(fragment);
        assertEquals(1, fragment.getParticipantsNum());
        assertEquals(MOCK_NUMBER, fragment.getParticipant().getContact());
        String filePath = getFilePath();
        FileStruct fileStruct = FileStruct.from(filePath);
        // PENDING
        SentFileTransfer fileTransfer = (SentFileTransfer) fragment.addSentFileTransfer(fileStruct);
        getInstrumentation().waitForIdleSync();
        Field fieldMessageAdapter = Utils.getPrivateField(
                One2OneChatFragment.class.getSuperclass(), "mMessageAdapter");
        ChatAdapter chatAdapter = (ChatAdapter) fieldMessageAdapter.get(fragment);
        assertNotNull(chatAdapter);
        assertEquals(2, chatAdapter.getCount());
        assertEquals(ChatAdapter.ITEM_TYPE_DATE_LABEL,
                chatAdapter.getItemViewType(DATE_LABEL_POSITION));
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_PENDING,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // FAILED
        fileTransfer.setStatus(Status.FAILED);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_FAILED,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // REJECTED
        fileTransfer.setStatus(Status.REJECTED);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_REJECT,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // CANCEL
        fileTransfer.setStatus(Status.CANCEL);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_CANCEL,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // CANCELED
        fileTransfer.setStatus(Status.CANCELED);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_CANCELED,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // WAITING
        fileTransfer.setStatus(Status.WAITING);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // TRANSFERING
        fileTransfer.setStatus(Status.TRANSFERING);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // FINISHED
        fileTransfer.setStatus(Status.FINISHED);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_FINISHED,
                chatAdapter.getItemViewType(CURRENT_POSITION));
        // Clear
        fragment.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        ModelImpl.getInstance().removeChat(chat.getChatTag());
        getInstrumentation().waitForIdleSync();
        Logger.v(TAG, "testCase01_BindView() out");
    }

    /**
     * Test the method 'onClick' in class 'SentFileTransferItemBinder'.
     */
    public void testCase02_onClick() throws Throwable {
        Logger.v(TAG, "testCase02_onClick() entry");
        ChatImpl chat = waitForChat();
        One2OneChatFragment fragment = (One2OneChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertNotNull(fragment);
        assertEquals(1, fragment.getParticipantsNum());
        assertEquals(MOCK_NUMBER, fragment.getParticipant().getContact());
        String filePath = getFilePath();
        FileStruct fileStruct = FileStruct.from(filePath);
        // Add a SentFileTransferItemBinder
        SentFileTransfer fileTransfer = (SentFileTransfer) fragment.addSentFileTransfer(fileStruct);
        getInstrumentation().waitForIdleSync();
        Field fieldMessageAdapter = Utils.getPrivateField(
                One2OneChatFragment.class.getSuperclass(), "mMessageAdapter");
        ChatAdapter chatAdapter = (ChatAdapter) fieldMessageAdapter.get(fragment);
        assertNotNull(chatAdapter);
        assertEquals(2, chatAdapter.getCount());
        assertEquals(ChatAdapter.ITEM_TYPE_DATE_LABEL,
                chatAdapter.getItemViewType(DATE_LABEL_POSITION));
        assertEquals(ChatAdapter.ITEM_TYPE_SENT_FILE_TRANSFER_PENDING,
                chatAdapter.getItemViewType(CURRENT_POSITION));

        View itemView = null;

        // Bind WAITING status view
        fileTransfer.setStatus(Status.WAITING);
        getInstrumentation().waitForIdleSync();
        // (1)Click on cancel button.
        itemView = chatAdapter.getView(CURRENT_POSITION, null, null);
        ImageView cancelView = (ImageView) itemView.findViewById(R.id.file_transfer_btn_cancel);
        clickOnView(cancelView);
        assertEquals(Status.CANCEL, fileTransfer.getStatue());

        // Bind WAITING status view
        fileTransfer.setStatus(Status.REJECTED);
        // (2)Click on reject button.
        itemView = chatAdapter.getView(CURRENT_POSITION, null, null);
        Button resendView = (Button) itemView.findViewById(R.id.file_transfer_resent);
        clickOnView(resendView);
        assertEquals(Status.REJECTED, fileTransfer.getStatue());

        Logger.v(TAG, "testCase02_onClick() exit");
    }

    /**
     * Mock clicking on the view.
     * 
     * @param view
     * @throws Throwable
     */
    private void clickOnView(final View view) throws Throwable {
        assertNotNull(view);
        getInstrumentation().waitForIdleSync();

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                view.performClick();
            }
        });

        getInstrumentation().waitForIdleSync();
    }

    /**
     * Get a image file path from database
     */
    private String getFilePath() {
        Logger.v(TAG, "getFilePath()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            } else {
                fail("testCase1_BindView() Cannot find image in sdcard");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
        return filePath;
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
