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

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.ChatScreenActivity.ChatWindowManager;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This class is used to test OneOneChatWindow
 */
public class OneOneChatWindowTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "OneOneChatWindowTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MOCK_NUMBER = "+3402222226";
    private static final Participant MOCK_121_PARTICIPANT = new Participant(MOCK_NUMBER,
            MOCK_NUMBER);
    private static final String MOCK_RECEIVED_MESSAGE_ID = "100";
    private static final String MOCK_SENT_MESSAGE_ID = "200";
    private static final String MOCK_UNREAD_MESSAGE_ID = "500";
    private static final int MOCK_SENT_MESSAGE_TAG = 123;
    private static final String MOCK_SENT_MESSAGE_ID_NOT_EXIST = "300";
    private static final String MOCK_REMOTE_USER = "RemoteUser";
    private static final String MOCK_MESSAGE_CONTENT = "content";
    private static final String MOCK_TEL_MESSAGE = "123456789\n";
    private static final String MOCK_URL_MESSAGE = "http:www.baidu.com\n";
    private static final String MOCK_EMAIL_MESSAGE = "123456789@pp.com\n";
    private static final String INFO_IS_TYPING = "is typing"; // From
                                                              // R.id.join_group_chat
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private ChatScreenActivity mActivity = null;
    private List mChatWindowManagerList = null;
    private One2OneChatFragment mFragment = null;
    private ChatImpl mChat = null;
    private OneOneChatWindow mOneOneChatWindow = null;
    private List<IChatWindowMessage> mMessageList = null;
    private ChatAdapter mChatAdapter = null;

    public OneOneChatWindowTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
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
        assertNotNull(mActivity);
        waitForChatWindowManagerNum(startCount + 1);
        /**
         * The last one is ChatWindowManager
         */
        ChatWindowManager chatWindowManager = (ChatWindowManager) mChatWindowManagerList
                .get(mChatWindowManagerList.size() - 1);

        mChat = waitForChat();
        mFragment = (One2OneChatFragment) mActivity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        assertNotNull(mFragment);
        assertEquals(1, mFragment.getParticipantsNum());
        assertEquals(MOCK_NUMBER, mFragment.getParticipant().getContact());

        Field fieldChatWindow = Utils.getPrivateField(ChatImpl.class, "mChatWindow");
        Object oneOneChatWindowDispatcher = fieldChatWindow.get(mChat);
        assertNotNull(oneOneChatWindowDispatcher);

        Field fieldChatWindowMap = Utils.getPrivateField(oneOneChatWindowDispatcher.getClass()
                .getSuperclass(), "mChatWindowMap");
        ConcurrentHashMap<IChatWindowManager, IChatWindow> chatWindowMap = (ConcurrentHashMap<IChatWindowManager, IChatWindow>) fieldChatWindowMap
                .get(oneOneChatWindowDispatcher);
        assertNotNull(chatWindowMap);

        mOneOneChatWindow = (OneOneChatWindow) chatWindowMap.get(chatWindowManager);
        assertNotNull(mOneOneChatWindow);

        Field fieldMessageList = Utils.getPrivateField(One2OneChatFragment.class, "mMessageList");
        mMessageList = (List<IChatWindowMessage>) fieldMessageList.get(mFragment);
        assertNotNull(mMessageList);
        mMessageList.clear();

        Field fieldMessageAdapter = Utils.getPrivateField(
                One2OneChatFragment.class.getSuperclass(), "mMessageAdapter");
        mChatAdapter = (ChatAdapter) fieldMessageAdapter.get(mFragment);
        assertNotNull(mChatAdapter);

        Logger.v(TAG, "setUp() exit");
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Test the function OneOneChatWindow#getFragment()
     */
    public void testCase01_GetFragment() throws Throwable {
        Logger.v(TAG, "testCase01_GetFragment() entry");
        One2OneChatFragment one2OneChatFragment = mOneOneChatWindow.getFragment();
        assertNotNull(one2OneChatFragment);
        assertEquals(mFragment, one2OneChatFragment);
        Logger.v(TAG, "testCase01_GetFragment() exit");
    }

    /**
     * Test OneOneChatWindow message functions:addReceivedMessage()
     * addSentMessage() getSentChatMessage() removeAllMessages()
     */
    public void testCase02_AddGetRemoveMessage() throws Throwable {
        Logger.v(TAG, "testCase02_AddGetRemoveMessage() entry");
        Field fieldMessageList = Utils.getPrivateField(One2OneChatFragment.class, "mMessageList");
        List<IChatWindowMessage> messageList = (List<IChatWindowMessage>) fieldMessageList
                .get(mFragment);

        // Test addReceivedMessage()
        messageList.clear();
        assertEquals(0, messageList.size());
        InstantMessage message = new InstantMessage(MOCK_RECEIVED_MESSAGE_ID, MOCK_REMOTE_USER,
                MOCK_MESSAGE_CONTENT, false, new Date());
        IReceivedChatMessage receivedChatMessage = mOneOneChatWindow.addReceivedMessage(message,
                false);
        getInstrumentation().waitForIdleSync();
        assertNotNull(receivedChatMessage);
        assertEquals(2, messageList.size());

        // Test addSentMessage()
        messageList.clear();
        assertEquals(0, messageList.size());
        InstantMessage message2 = new InstantMessage(MOCK_SENT_MESSAGE_ID, MOCK_REMOTE_USER,
                MOCK_MESSAGE_CONTENT, false, new Date());
        ISentChatMessage sentChatMessage = mOneOneChatWindow.addSentMessage(message2,
                MOCK_SENT_MESSAGE_TAG);
        getInstrumentation().waitForIdleSync();
        assertNotNull(sentChatMessage);
        assertEquals(1, messageList.size());

        // Test getSentChatMessage()
        assertNotNull(mOneOneChatWindow.getSentChatMessage(MOCK_SENT_MESSAGE_ID));
        assertNull(mOneOneChatWindow.getSentChatMessage(MOCK_SENT_MESSAGE_ID_NOT_EXIST));

        // Test removeAllMessages()
        Field fieldMessageAdapter = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mMessageAdapter");
        ChatAdapter messageAdapter = (ChatAdapter) fieldMessageAdapter.get(mFragment);
        assertTrue(messageAdapter.getCount() > 0);
        mOneOneChatWindow.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        assertEquals(0, messageAdapter.getCount());
        Logger.v(TAG, "testCase02_AddGetRemoveMessage() exit");
    }

    /**
     * Test the function OneOneChatWindow#setIsComposing()
     */
    public void testCase03_SetIsComposing() throws Throwable {
        Logger.v(TAG, "testCase03_SetIsComposing() entry");
        Field fieldTypingText = Utils.getPrivateField(One2OneChatFragment.class.getSuperclass(),
                "mTypingText");
        TextView typingText = (TextView) fieldTypingText.get(mFragment);
        Field fieldTextReminderSortedSet = Utils.getPrivateField(
                One2OneChatFragment.class.getSuperclass(), "mTextReminderSortedSet");
        TreeSet<String> mTextReminderSortedSet = (TreeSet<String>) fieldTextReminderSortedSet
                .get(mFragment);
        assertNotNull(typingText);
        assertNotNull(mTextReminderSortedSet);

        mTextReminderSortedSet.clear();
        mOneOneChatWindow.setIsComposing(true);
        getInstrumentation().waitForIdleSync();
        assertTrue(typingText.getText().toString().contains(INFO_IS_TYPING));
        assertTrue(mTextReminderSortedSet.contains(ChatFragment.SHOW_IS_TYPING_REMINDER));

        mTextReminderSortedSet.clear();
        mOneOneChatWindow.setIsComposing(false);
        getInstrumentation().waitForIdleSync();
        assertFalse(mTextReminderSortedSet.contains(ChatFragment.SHOW_IS_TYPING_REMINDER));
        Logger.v(TAG, "testCase03_SetIsComposing() exit");
    }

    /**
     * Test the function OneOneChatWindow#addSentFileTransfer()
     */
    public void testCase04_AddSentFileTransfer() throws Throwable {
        Logger.v(TAG, "testCase04_AddSetFileTransfer() entry");
        FileStruct fileStruct = FileStruct.from(getFilePath());
        assertNotNull(mOneOneChatWindow.addSentFileTransfer(fileStruct));
        Logger.v(TAG, "testCase04_AddSetFileTransfer() exit");
    }

    /**
     * Test the function OneOneChatWindow#addReceivedFileTransfer()
     */
    public void testCase05_AddReceivedFileTransfer() throws Throwable {
        Logger.v(TAG, "testCase05_AddReceivedFileTransfer() entry");
        FileStruct fileStruct = FileStruct.from(getFilePath());
        assertNotNull(mOneOneChatWindow.addReceivedFileTransfer(fileStruct, false));
        Logger.v(TAG, "testCase05_AddReceivedFileTransfer() exit");
    }

    /**
     * Test the function OneOneChatWindow#clearHistory()
     */
    public void testCase6_ClearHistory() throws Throwable {
        Logger.v(TAG, "testCase6_ClearHistory() entry");
        assertTrue(mOneOneChatWindow.clearHistory());
        Logger.v(TAG, "testCase6_ClearHistory() exit");
    }

    /**
     * Test the function OneOneChatWindow#addUnReadMessage()
     */
    public void testCase07_AddUnReadMessage() throws Throwable {
        Logger.v(TAG, "testCase07_AddUnReadMessage() entry");
        InstantMessage message = new InstantMessage(MOCK_UNREAD_MESSAGE_ID, MOCK_REMOTE_USER,
                MOCK_MESSAGE_CONTENT, false, new Date());
        mOneOneChatWindow.addUnreadMessage(message);
        getInstrumentation().waitForIdleSync();

        Field fieldMgToOtherWinReminderText = Utils.getPrivateField(
                GroupChatFragment.class.getSuperclass(), "mMgToOtherWinReminderText");
        final TextView mgToOtherWinReminderText = (TextView) fieldMgToOtherWinReminderText
                .get(mFragment);
        String text = mgToOtherWinReminderText.getText().toString();
        assertTrue(text.contains(MOCK_MESSAGE_CONTENT));
        Logger.v(TAG, "testCase07_AddUnReadMessage() exit");
    }

    /**
     * Test the function OneOneChatWindow#setFileTransferEnable()
     */
    public void testCase08_SetFileTransferEnable() throws Throwable {
        Logger.v(TAG, "testCase08_SetFileTransferEnable() entry");
        Field fieldContentView = Utils.getPrivateField(GroupChatFragment.class.getSuperclass(),
                "mContentView");
        View contentView = (View) fieldContentView.get(mFragment);
        assertNotNull(contentView);
        ImageButton btnAddView = (ImageButton) contentView.findViewById(R.id.btn_chat_add);
        assertNotNull(btnAddView);

        // FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED
        setButtonEnable(btnAddView);
        mOneOneChatWindow
                .setFileTransferEnable(One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
        getInstrumentation().waitForIdleSync();
        assertButtonIsDisable(btnAddView);

        // FILETRANSFER_DISABLE_REASON_NOT_REGISTER
        setButtonEnable(btnAddView);
        mOneOneChatWindow
                .setFileTransferEnable(One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
        getInstrumentation().waitForIdleSync();
        assertButtonIsDisable(btnAddView);

        // FILETRANSFER_DISABLE_REASON_REMOTE
        setButtonEnable(btnAddView);
        mOneOneChatWindow.setFileTransferEnable(One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);
        getInstrumentation().waitForIdleSync();
        assertButtonIsDisable(btnAddView);

        // FILETRANSFER_ENABLE_OK
        setButtonDisable(btnAddView);
        mOneOneChatWindow.setFileTransferEnable(One2OneChat.FILETRANSFER_ENABLE_OK);
        getInstrumentation().waitForIdleSync();
        assertButtonIsEnable(btnAddView);
        Logger.v(TAG, "testCase08_SetFileTransferEnable() exit");
    }

    /**
     * Test the function OneOneChatWindow#removeChatUi()
     */
    public void testCase09_RemoveChatUi() throws Throwable {
        Logger.v(TAG, "testCase09_RemoveChatUi() entry");
        assertNotNull(mActivity.getActionBar().getCustomView());
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mOneOneChatWindow.removeChatUi();
            }
        });
        getInstrumentation().waitForIdleSync();
        assertNull(mActivity.getActionBar().getCustomView());
        Logger.v(TAG, "testCase09_RemoveChatUi() exit");
    }

    /**
     * Let button disabled
     */
    private void setButtonDisable(final ImageButton button) throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                button.setClickable(false);
                button.setFocusable(false);
                button.setEnabled(false);
            }
        });
        getInstrumentation().waitForIdleSync();
    }

    /**
     * Let button enabled
     */
    private void setButtonEnable(final ImageButton button) throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                button.setClickable(true);
                button.setFocusable(true);
                button.setEnabled(true);
            }
        });
        getInstrumentation().waitForIdleSync();
    }

    private void assertButtonIsDisable(final ImageButton button) throws Throwable {
        assertFalse(button.isClickable());
        assertFalse(button.isFocusable());
        assertFalse(button.isEnabled());
    }

    private void assertButtonIsEnable(final ImageButton button) throws Throwable {
        assertTrue(button.isClickable());
        assertTrue(button.isFocusable());
        assertTrue(button.isEnabled());
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
                fail("getFilePath() Cannot find image in sdcard");
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

    /**
     * Test case for the functions onClickChatMessage of MessageParsing.
     */
    public void testCase10_onClickMessage() throws Throwable {
        Logger.v(TAG, "testCase10_onClickMessage() entry");
        // add a testcase for onClick on a tel message
        addASendMessageToListView(MOCK_TEL_MESSAGE);
        
        // Click on the text view.
        View itemView = mChatAdapter.getView(1, null, null);
        getInstrumentation().waitForIdleSync();
        assertNotNull(itemView);
        clickMessageView(itemView);

        // Assert the result.
        DialogFragment dialogFragment = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag("MultiAlertDialog");
        assertNotNull(dialogFragment);
        assertFalse(dialogFragment.isHidden());
        dialogFragment.dismiss();
        getInstrumentation().waitForIdleSync();

        Logger.v(TAG, "testCase10_onClickMessage() exit");
    }
    
    /**
     * Test case for the functions onClickChatMessage of MessageParsing.
     */
    public void testCase11_onClickMessage() throws Throwable {
        Logger.v(TAG, "testCase11_onClickMessage() entry");
        // add a testcase for onClick on a tel + http + email message
        addASendMessageToListView(MOCK_TEL_MESSAGE + MOCK_URL_MESSAGE + MOCK_EMAIL_MESSAGE);
        
        // Click on the text view.
        View itemView = mChatAdapter.getView(1, null, null);
        assertNotNull(itemView);
        clickMessageView(itemView);

        // Assert the result.
        DialogFragment dialogFragment = (DialogFragment) mActivity.getFragmentManager().findFragmentByTag(
                "MultiAlertDialog");
        assertNotNull(dialogFragment);
        assertFalse(dialogFragment.isHidden());
        dialogFragment.dismiss();
        getInstrumentation().waitForIdleSync();

        Logger.v(TAG, "testCase11_onClickMessage() exit");
    }
    
    /**
     * Test case for the functions onClick of MultiAlertDialog in class MessageParsing.
     */
    public void testCase12_onClickMultiAlertDialog() throws Throwable {
        Logger.v(TAG, "testCase12_onClickMultiAlertDialog() entry");
        // add a testcase for onClick on a tel message
        addASendMessageToListView(MOCK_TEL_MESSAGE);

        View itemView = mChatAdapter.getView(1, null, null);
        getInstrumentation().waitForIdleSync();
        assertNotNull(itemView);
        clickMessageView(itemView);

        DialogFragment dialogFragment = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag("MultiAlertDialog");
        assertNotNull(dialogFragment);
        assertFalse(dialogFragment.isHidden());

        // Click the second item on the dialog
        clickMessageView(itemView);
        ActivityMonitor activityMonitor = new ActivityMonitor(ChatScreenActivity.class.getName(), null, false);
        getInstrumentation().addMonitor(activityMonitor);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();

        Activity activity = activityMonitor.waitForActivityWithTimeout(500);
        try {
            assertNotNull(activity);
            assertTrue(activity.getClass().getName().equals(ChatScreenActivity.class.getName()));
        } finally {
            getInstrumentation().removeMonitor(activityMonitor);
            if (null != activity) {
                activity.finish();
            }
        }

        Logger.v(TAG, "testCase12_onClickMultiAlertDialog() exit");
    }
    
    /**
     * Test case for the functions onClickChatMessage of MessageParsing.
     *//*
    public void testCase13_onClickMessage() throws Throwable {
        Logger.v(TAG, "testCase13_onClickMessage() entry");
        // add a testcase for onClick on a email message
        addASendMessageToListView(MOCK_EMAIL_MESSAGE);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_VIEW);
        ActivityMonitor activityMonitor = new ActivityMonitor(intentFilter, null, false);
        getInstrumentation().addMonitor(activityMonitor);
        // Click on the text view.
        View itemView = mChatAdapter.getView(1, null, null);
        getInstrumentation().waitForIdleSync();
        assertNotNull(itemView);
        clickMessageView(itemView);
        
        // Assert the result.
        activityMonitor.waitForActivityWithTimeout(500);
        getInstrumentation().removeMonitor(activityMonitor);

        Logger.v(TAG, "testCase13_onClickMessage() exit");
    }
    
    /**
     * Test case for the functions onClick of MultiAlertDialog in class MessageParsing.
     *//*
    public void testCase14_onClickMultiAlertDialog() throws Throwable {
        Logger.v(TAG, "testCase14_onClickMultiAlertDialog() entry");
        ActivityMonitor activityMonitor = null;
        Activity activity = null;
        
        // add a testcase for onClick on a tel message
        addASendMessageToListView(MOCK_TEL_MESSAGE);

        View itemView = mChatAdapter.getView(1, null, null);
        getInstrumentation().waitForIdleSync();
        assertNotNull(itemView);
        clickMessageView(itemView);

        DialogFragment dialogFragment = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag("MultiAlertDialog");
        assertNotNull(dialogFragment);
        assertFalse(dialogFragment.isHidden());

        // Click the first item on the dialog
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_VIEW);
        activityMonitor = new ActivityMonitor(intentFilter, null, false);
        getInstrumentation().addMonitor(activityMonitor);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();

        activityMonitor.waitForActivityWithTimeout(500);
        getInstrumentation().removeMonitor(activityMonitor);

        Logger.v(TAG, "testCase14_onClickMultiAlertDialog() exit");
    }*/

    /**
     * Add a message into ListView.
     * @param message
     */
    private void addASendMessageToListView(String message){
        mOneOneChatWindow.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        InstantMessage instantMessage = new InstantMessage(MOCK_SENT_MESSAGE_ID, MOCK_REMOTE_USER, message, false, new Date());
        ISentChatMessage sentChatMessage = mOneOneChatWindow.addSentMessage(instantMessage, MOCK_SENT_MESSAGE_TAG);
        getInstrumentation().waitForIdleSync();
    }
    
    /**
     * Click the textview on the item.
     * 
     * @param itemView
     * @throws Throwable
     */
    private void clickMessageView(View itemView) throws Throwable {
        final TextView textView = (TextView) itemView.findViewById(R.id.chat_text_display);
        getInstrumentation().waitForIdleSync();
        assertNotNull(textView);

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                textView.performClick();
            }
        });

        getInstrumentation().waitForIdleSync();
    }
}


