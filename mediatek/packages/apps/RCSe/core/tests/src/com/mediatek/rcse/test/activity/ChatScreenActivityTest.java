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

package com.mediatek.rcse.test.activity;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences.Editor;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.View;
import android.widget.CheckBox;
import android.widget.HorizontalScrollView;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.StatFs;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Instrumentation.ActivityMonitor;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.database.Cursor;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.ContactsListFragment;
import com.mediatek.rcse.activities.DialogActivity;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.ChatScreenActivity.ChatWindowManager;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ChatScreenWindow;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

public class ChatScreenActivityTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {
    private static final String TAG = "ChatScreenActivityTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String MODELIMPL_ISCHATEXSITED = "isChatExisted";
    private static final String FALSE = "false";
    private static final String FOCUS = "focus";
    private static final String CHATIMPL_MTAG = "mTag";
    private static final String M_CHAT_WINDOW_MANAGER = "mChatWindowManager";
    private static final String SWITCH_CHAT_WINDOW_BY_TAG = "switchChatWindowByTag";
    private final static String MOCK_NUMBER = "+3402222222";
    private final static Participant MOCK_121_PARTICIPANT = new Participant(MOCK_NUMBER,
            MOCK_NUMBER);
    private final static String MOCK_NUMBER_111 = "+3401111111";
    private final static Participant MOCK_111_PARTICIPANT = new Participant(MOCK_NUMBER_111,
            MOCK_NUMBER_111);
    private static final Date MOCK_DATE_TODAY = new Date();
    private static final Date MOCK_DATE_YESTERDAY = new Date();
    private static final Date MOCK_DATE_TOMORROW = new Date();
    private static final long MILLI_PER_DAY = 60 * 60 * 24 * 1000;
    static {
        MOCK_DATE_YESTERDAY.setTime(MOCK_DATE_YESTERDAY.getTime() - MILLI_PER_DAY);
        MOCK_DATE_TOMORROW.setTime(MOCK_DATE_TOMORROW.getTime() + MILLI_PER_DAY);
    }
    private final static String MOCK_SENT_MESSAGE_TEXT = "This is a sent message";
    private final static String MOCK_PRE_SENT_MESSAGE_TEXT_1 = "This is pre-sent message 1";
    private final static String MOCK_PRE_SENT_MESSAGE_TEXT_2 = "This is pre-sent message 2";
    private final static String MOCK_RECEIVED_MESSAGE_TEXT = "This is a received message";
    private static final InstantMessage MOCK_SENT_MESSAGE_TODAY = new InstantMessage("sent",
            MOCK_NUMBER, MOCK_SENT_MESSAGE_TEXT, true);
    private static final InstantMessage MOCK_RECEIVED_MESSAGE_YESTERDAY = new InstantMessage(
            "received", MOCK_NUMBER, MOCK_RECEIVED_MESSAGE_TEXT, true);
    private static final InstantMessage MOCK_RECEIVED_MESSAGE_TOMORROW = new InstantMessage(
            "received", MOCK_NUMBER, MOCK_RECEIVED_MESSAGE_TEXT, true);
    static {
        MOCK_SENT_MESSAGE_TODAY.setDate(MOCK_DATE_TODAY);
        MOCK_RECEIVED_MESSAGE_YESTERDAY.setDate(MOCK_DATE_YESTERDAY);
        MOCK_RECEIVED_MESSAGE_TOMORROW.setDate(MOCK_DATE_TOMORROW);
    }
    private static final int TIME_OUT = 2000;
    private static final int SLEEP_TIME = 200;
    private ChatScreenActivity mActivity = null;

    private List mChatWindowManagerList = null;
    private ParcelUuid currentTag = null;
    private ChatFragment currentFragment = null;
    private Field participantListDisplayerField = null;

    public ChatScreenActivityTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() enter");
        super.setUp();
        getInstrumentation().waitForIdleSync();
        Field apiManagerField = ApiManager.class.getDeclaredField(API_MANAGER_INSTANCE);
        apiManagerField.setAccessible(true);
        apiManagerField.set(ApiManager.class, null);

        Method initializeMethod = ApiManager.class.getDeclaredMethod(API_MANAGER_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(null, getInstrumentation().getTargetContext());
        Intent intent = generateIntent(false);
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

    private Intent generateIntent(boolean isGroup) {
        Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(MOCK_121_PARTICIPANT);
        if (isGroup) {
            participants.add(MOCK_111_PARTICIPANT);
        }
        intent.putExtra(ChatMainActivity.KEY_ADD_CONTACTS, ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    /**
     * Test launch a One2OneChat and the launch a GroupChat by use mock intents
     * ,then test the fragments switch between One2OneChat and GroupChat.
     * 
     * @throws Throwable
     */
    public void testCase01_OnNewIntent() throws Throwable {
        Logger.d(TAG, "testCase01_OnNewIntent() entry!");
        currentFragment = waitForChatFragment();
        assertTrue(currentFragment instanceof One2OneChatFragment);
        Object one2OneChatTag = currentFragment.getChatFragmentTag();
        assertNotNull(one2OneChatTag);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, generateIntent(true));
            }
        });
        getInstrumentation().waitForIdleSync();
        currentFragment = waitForChatFragment();
        assertTrue(currentFragment instanceof GroupChatFragment);
        Object groupChatTag = currentFragment.getChatFragmentTag();
        assertNotNull(groupChatTag);

        Field chatWindowManagerField = Utils.getPrivateField(ChatScreenActivity.class,
                M_CHAT_WINDOW_MANAGER);
        ChatWindowManager chatWindowManager = (ChatWindowManager) chatWindowManagerField
                .get(mActivity);
        Method switchChatWindowByTag = ChatWindowManager.class.getMethod(SWITCH_CHAT_WINDOW_BY_TAG,
                ParcelUuid.class);
        assertNotNull(switchChatWindowByTag);
        Method focusWindow = ChatScreenWindowContainer.class.getMethod(FOCUS, ParcelUuid.class);
        assertNotNull(focusWindow);

        focusWindow.invoke(ChatScreenWindowContainer.getInstance(), one2OneChatTag);
        switchChatWindowByTag.invoke(chatWindowManager, one2OneChatTag);
        getInstrumentation().waitForIdleSync();
        currentFragment = waitForChatFragment();
        assertEquals(one2OneChatTag, currentFragment.getChatFragmentTag());

        focusWindow.invoke(ChatScreenWindowContainer.getInstance(), groupChatTag);
        switchChatWindowByTag.invoke(chatWindowManager, groupChatTag);
        getInstrumentation().waitForIdleSync();
        currentFragment = waitForChatFragment();
        assertEquals(groupChatTag, currentFragment.getChatFragmentTag());

        currentFragment = null;

        // int startCount = mChatWindowManagerList.size();
        // mActivity.finish();
        // waitForChatWindowManagerNum(startCount - 1);
        Logger.d(TAG, "testCase01_OnNewIntent() exit!");
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

    public void testCase2_One2OneChatWindowDate() throws Throwable {
        Logger.d(TAG, "testCase2_One2OneChatWindowDate() entry!");
        ChatImpl chat = waitForChat();
        One2OneChatFragment fragment = (One2OneChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertEquals(1, fragment.getParticipantsNum());
        assertEquals(MOCK_NUMBER, fragment.getParticipant().getContact());
        Field fieldDateList = Utils.getPrivateField(One2OneChatFragment.class, "mDateList");
        List dateList = (List) fieldDateList.get(fragment);
        Field fieldMessageList = Utils.getPrivateField(One2OneChatFragment.class, "mMessageList");
        List messageList = (List) fieldMessageList.get(fragment);
        fragment.addReceivedMessage(MOCK_RECEIVED_MESSAGE_YESTERDAY, true);
        getInstrumentation().waitForIdleSync();
        assertEquals(1, dateList.size());
        assertEquals(2, messageList.size());
        ISentChatMessage sentMessage = fragment.addSentMessage(MOCK_SENT_MESSAGE_TODAY, -1);
        getInstrumentation().waitForIdleSync();
        assertEquals(2, dateList.size());
        assertEquals(4, messageList.size());
        fragment.addReceivedMessage(MOCK_RECEIVED_MESSAGE_TOMORROW, true);
        getInstrumentation().waitForIdleSync();
        assertEquals(3, dateList.size());
        assertEquals(6, messageList.size());
        sentMessage.updateDate(MOCK_DATE_TOMORROW);
        getInstrumentation().waitForIdleSync();
        assertEquals(2, dateList.size());
        assertEquals(5, messageList.size());
        sentMessage.updateDate(MOCK_DATE_YESTERDAY);
        getInstrumentation().waitForIdleSync();
        assertEquals(2, dateList.size());
        assertEquals(5, messageList.size());
        fragment.removeAllMessages();
        getInstrumentation().waitForIdleSync();
        assertEquals(0, dateList.size());
        assertEquals(0, messageList.size());
        ModelImpl.getInstance().removeChat(chat.getChatTag());
    }

    public void testCase3_One2OneChatWindowPresend() throws Throwable {
        Logger.d(TAG, "testCase3_One2OneChatWindowPresend() entry!");
        ChatImpl chat = waitForChat();
        One2OneChatFragment fragment = (One2OneChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertEquals(1, fragment.getParticipantsNum());
        assertEquals(MOCK_NUMBER, fragment.getParticipant().getContact());
        Field fieldDateList = Utils.getPrivateField(One2OneChatFragment.class, "mDateList");
        List dateList = (List) fieldDateList.get(fragment);
        Field fieldMessageList = Utils.getPrivateField(One2OneChatFragment.class, "mMessageList");
        List messageList = (List) fieldMessageList.get(fragment);
        Method onSendMessageMethod = Utils.getPrivateMethod(One2OneChatFragment.class,
                "onSentMessage", String.class);
        onSendMessageMethod.invoke(fragment, MOCK_PRE_SENT_MESSAGE_TEXT_1);
        getInstrumentation().waitForIdleSync();
        assertEquals(0, dateList.size());
        assertEquals(1, messageList.size());

        SentMessage sentMessage = (SentMessage) messageList.get(messageList.size() - 1);
        sentMessage.updateDate(MOCK_DATE_TODAY);
        sentMessage.updateStatus(Status.DELIVERED);
        getInstrumentation().waitForIdleSync();
        assertEquals(1, dateList.size());
        assertEquals(2, messageList.size());

        fragment.addReceivedMessage(MOCK_RECEIVED_MESSAGE_YESTERDAY, true);
        getInstrumentation().waitForIdleSync();
        assertEquals(2, dateList.size());
        assertEquals(4, messageList.size());
        sentMessage = (SentMessage) messageList.get(messageList.size() - 1);
        assertEquals(MOCK_PRE_SENT_MESSAGE_TEXT_1, sentMessage.getMessageText());

        fragment.addReceivedMessage(MOCK_RECEIVED_MESSAGE_TOMORROW, true);
        assertEquals(3, dateList.size());
        assertEquals(6, messageList.size());
        ReceivedMessage receivedMessage = (ReceivedMessage) messageList.get(messageList.size() - 1);
        assertEquals(MOCK_RECEIVED_MESSAGE_TEXT, receivedMessage.getMessageText());

        onSendMessageMethod.invoke(fragment, MOCK_PRE_SENT_MESSAGE_TEXT_2);
        assertEquals(3, dateList.size());
        assertEquals(7, messageList.size());
        sentMessage = (SentMessage) messageList.get(messageList.size() - 1);
        assertEquals(MOCK_PRE_SENT_MESSAGE_TEXT_2, sentMessage.getMessageText());

        sentMessage.updateDate(MOCK_DATE_TODAY);
        getInstrumentation().waitForIdleSync();
        assertEquals(3, dateList.size());
        assertEquals(7, messageList.size());

        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        assertEquals(0, dateList.size());
        assertEquals(0, messageList.size());
        ModelImpl.getInstance().removeChat(chat.getChatTag());
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

    /**
     * Test use a mock intent with IM_Action launch a One2OneChat,and test
     * switch between text editor interface and expression editor interface.
     * 
     * @throws Throwable
     */
    public void testCase4_handleImAction() throws Throwable {
        Logger.d(TAG, "testCase4_handleImAction() entry!");
        final Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        intent.setAction(PluginApiManager.RcseAction.IM_ACTION);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, MOCK_NUMBER);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, MOCK_NUMBER);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });
        getInstrumentation().waitForIdleSync();
        final ChatFragment fragment = (ChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertTrue(fragment instanceof One2OneChatFragment);

        final Method showImm = Utils.getPrivateMethod(ChatFragment.class, "showImm", Boolean.class);
        Field emotionLayoutField = Utils.getPrivateField(ChatFragment.class, "mEmotionLayout");
        View emotionLayout = (View) emotionLayoutField.get(fragment);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    showImm.invoke(fragment, true);
                } catch (Exception t) {
                    t.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertEquals(View.GONE, emotionLayout.getVisibility());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    showImm.invoke(fragment, false);
                } catch (Exception t) {
                    t.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertEquals(View.VISIBLE, emotionLayout.getVisibility());

        sendKeys(KeyEvent.KEYCODE_BACK);
        getInstrumentation().waitForIdleSync();
        assertEquals(true, mActivity.isFinishing());
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
                fail("testCase5_handleFtAction() Cannot find out a image file in sdcard.");
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
     * Test File transfer, If the file size is less than WARNING size, it will
     * be transfered. If the file size is greater than WARNING size but less
     * than MAX size, it will popup a waring dialog. If the file size is greater
     * than MAX size, it doesn't popup a waring dialog.
     */
    public void testCase5_handleFtAction() throws Throwable, Exception {
        Logger.d(TAG, "testCase5_handleFtAction() entry!");
        String testFilePath = getFilePath();
        assertNotNull(testFilePath);
        long testFileSize = new File(testFilePath).length();

        // Get the chat screen message count.
        One2OneChatFragment fragment = (One2OneChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertNotNull(fragment);
        assertEquals(1, fragment.getParticipantsNum());

        Field fieldMaxFileSize = ApiManager.class.getDeclaredField("sMaxFileSize");
        fieldMaxFileSize.setAccessible(true);
        Field fieldWarningFileSize = ApiManager.class.getDeclaredField("sWarningFileSize");
        fieldWarningFileSize.setAccessible(true);

        Field fieldMessageList = One2OneChatFragment.class.getDeclaredField("mMessageList");
        fieldMessageList.setAccessible(true);
        List messageList = (List) fieldMessageList.get(fragment);

        Method addReceivedFileTransferMethod = One2OneChatFragment.class.getDeclaredMethod(
                "addReceivedFileTransfer", FileStruct.class);

        final Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, MOCK_NUMBER);
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, MOCK_NUMBER);
        ArrayList<String> listTestFiles = new ArrayList<String>();
        listTestFiles.add(testFilePath);
        intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI, listTestFiles);

        // Test file transfer that the file less than WARNING size
        fieldWarningFileSize.set(ApiManager.getInstance(), testFileSize + 1);
        fieldMaxFileSize.set(ApiManager.getInstance(), testFileSize + 3);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });
        getInstrumentation().waitForIdleSync();

        FileStruct fileStruct = FileStruct.from(testFilePath);
        addReceivedFileTransferMethod.invoke(fragment, fileStruct);
        getInstrumentation().waitForIdleSync();
        assertEquals(2, messageList.size());

        DialogFragment dialogFragment = null;
        dialogFragment = (DialogFragment) mActivity.getFragmentManager().findFragmentByTag(
                ChatScreenActivity.TAG);
        getInstrumentation().waitForIdleSync();
        assertNull(dialogFragment);
        fragment.removeAllMessages();
        assertEquals(0, messageList.size());

        // Test file transfer that the file less than MAX size
        fieldWarningFileSize.set(ApiManager.getInstance(), testFileSize - 1);
        fieldMaxFileSize.set(ApiManager.getInstance(), testFileSize + 1);

        SharedPreferences sharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mActivity);
        Boolean isRemind = sharedPreferences.getBoolean(SettingsFragment.RCS_REMIND, false);
        Editor editor = sharedPreferences.edit();
        if (!isRemind) {
            editor.putBoolean(SettingsFragment.RCS_REMIND, true);
            editor.commit();
        }

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });

        getInstrumentation().waitForIdleSync();
        dialogFragment = (DialogFragment) mActivity.getFragmentManager().findFragmentByTag(
                ChatScreenActivity.TAG);
        getInstrumentation().waitForIdleSync();
        assertNotNull(dialogFragment);

        Field checkRemindField = dialogFragment.getClass().getDeclaredField("mCheckRemind");
        checkRemindField.setAccessible(true);
        final CheckBox checkRemind = (CheckBox) checkRemindField.get(dialogFragment);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                checkRemind.setChecked(true);
            }
        });
        assertEquals(true, checkRemind.isChecked());
        clickDialogButton(dialogFragment, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();

        boolean afterClickOK = sharedPreferences.getBoolean(SettingsFragment.RCS_REMIND, false);
        assertEquals(false, afterClickOK);
        dialogFragment.dismiss();

        // Restore the value of 'SettingsFragment.RCS_REMIND'
        editor.putBoolean(SettingsFragment.RCS_REMIND, isRemind);
        editor.commit();

        // Test file transfer that the file greater than MAX size
        fieldWarningFileSize.set(ApiManager.getInstance(), testFileSize - 3);
        fieldMaxFileSize.set(ApiManager.getInstance(), testFileSize - 1);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });

        dialogFragment = (DialogFragment) mActivity.getFragmentManager().findFragmentByTag(
                ChatScreenActivity.TAG);
        getInstrumentation().waitForIdleSync();
        assertNull(dialogFragment);
    }

    /**
     * Test after launch a One2OneChat by use mock intent ,colse it and test
     * whether it removed from ModelImpl.
     * 
     * @throws Throwable
     */
    public void testCase6_closeChat() throws Throwable {
        Logger.d(TAG, "testCase6_closeChat() entry!");
        assertNotNull(mActivity);
        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        Method isChatExisted = Utils.getPrivateMethod(ModelImpl.class, MODELIMPL_ISCHATEXSITED,
                Participant.class);
        assertNotNull(isChatExisted);
        String isExisted = isChatExisted.invoke(ModelImpl.getInstance(), MOCK_121_PARTICIPANT)
                .toString();
        long startTime = System.currentTimeMillis();
        while (isExisted == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            isExisted = isChatExisted.invoke(ModelImpl.getInstance(), MOCK_121_PARTICIPANT)
                    .toString();
        }
        assertEquals(FALSE, isExisted);
    }

    /**
     * Test after launch a One2OneChat by use mock intent ,block it and test
     * whether it removed from ModelImpl ,then test the contact whether be added
     * into blacklist.
     * 
     * @throws Throwable
     */
    public void testCase7_blockContact() throws Throwable {
        Logger.d(TAG, "testCase7_blockContact() entry!");
        assertNotNull(mActivity);
        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        Method isChatExisted = Utils.getPrivateMethod(ModelImpl.class, MODELIMPL_ISCHATEXSITED,
                Participant.class);
        assertNotNull(isChatExisted);
        String isExisted = isChatExisted.invoke(ModelImpl.getInstance(), MOCK_121_PARTICIPANT)
                .toString();
        long startTime = System.currentTimeMillis();
        while (isExisted == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            isExisted = isChatExisted.invoke(ModelImpl.getInstance(), MOCK_121_PARTICIPANT)
                    .toString();
        }
        assertEquals(FALSE, isExisted);

        ContactsManager instance = ContactsManager.getInstance();
        boolean isBlocked = instance.isImBlockedForContact(MOCK_NUMBER);
        long startTimeA = System.currentTimeMillis();
        while (!isBlocked) {
            if (System.currentTimeMillis() - startTimeA > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            isBlocked = instance.isImBlockedForContact(MOCK_NUMBER);
        }

        // clear my test contact
        //boolean result = instance.setImBlockedForContact(MOCK_NUMBER, false);
        assertEquals(true, false);
    }

    /**
     * Test whether launch a SelectContactsActivity after click addContacts
     * menu.
     * 
     * @throws Throwable
     */
    public void testCase8_addContacts() throws Throwable {
        Logger.d(TAG, "testCase8_addContacts() entry!");
        ActivityMonitor am = new ActivityMonitor(SelectContactsActivity.class.getName(), null,
                false);
        SelectContactsActivity selectContactsActivity = null;
        getInstrumentation().addMonitor(am);
        getInstrumentation().invokeMenuActionSync(mActivity, R.id.menu_add_contact, 0);
        getInstrumentation().waitForIdleSync();
        selectContactsActivity = (SelectContactsActivity) am.waitForActivity();
        assertTrue(selectContactsActivity instanceof SelectContactsActivity);

        getInstrumentation().removeMonitor(am);
        sendKeys(KeyEvent.KEYCODE_BACK);
        getInstrumentation().waitForIdleSync();
        assertTrue(selectContactsActivity.isFinishing());
    }

    /**
     * Launch a group chat and click the quit menu ,then test whether quit from
     * the current group chat.
     * 
     * @throws Throwable
     */
    public void testCase9_quitGroupChat() throws Throwable {
        Logger.d(TAG, "testCase9_quitGroupChat() entry!");
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, generateIntent(true));
            }
        });
        getInstrumentation().waitForIdleSync();
        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        assertTrue(mActivity.isFinishing());
    }

    /**
     * Launch a one2one or a group chat ,click the setWallpaper menu and test
     * whether launch the DialogActivity
     * 
     * @throws Throwable
     */
    public void testCase10_setWallpaper() throws Throwable {
        Logger.d(TAG, "testCase10_setWallpaper() entry!");
        ActivityMonitor am = new ActivityMonitor(DialogActivity.class.getName(), null, false);
        DialogActivity dialogActivity = null;
        getInstrumentation().addMonitor(am);

        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        dialogActivity = (DialogActivity) am.waitForActivity();
        assertTrue(dialogActivity instanceof DialogActivity);

        sendKeys(KeyEvent.KEYCODE_BACK);
        getInstrumentation().waitForIdleSync();
        assertTrue(dialogActivity.isFinishing());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, generateIntent(true));
            }
        });
        getInstrumentation().waitForIdleSync();
        sendKeys(KeyEvent.KEYCODE_MENU);
        sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
        getInstrumentation().waitForIdleSync();
        dialogActivity = (DialogActivity) am.waitForActivity();
        assertTrue(dialogActivity instanceof DialogActivity);

        sendKeys(KeyEvent.KEYCODE_BACK);
        getInstrumentation().waitForIdleSync();
        assertTrue(dialogActivity.isFinishing());
        getInstrumentation().removeMonitor(am);
    }

    /**
     * Click the button of dialog showed
     * 
     * @param confirmDialog
     * @param dialogButtonIndicator
     */
    public void clickDialogButton(DialogFragment confirmDialog, final int dialogButtonIndicator) {
        final DialogInterface.OnClickListener dialogFragment = (DialogInterface.OnClickListener) confirmDialog;
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                dialogFragment.onClick(null, dialogButtonIndicator);
            }
        });
    }

    /**
     * Mock launch a groupChat and click the expand/collapse view and title
     * layout ,then judge the result whether correct.
     * 
     * @param confirmDialog
     * @param dialogButtonIndicator
     */
    public void testCase11_ChatWindowManagerOnClick() throws Throwable {
        Logger.d(TAG, "testCase11_ChatWindowManagerOnClick() entry!");
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, generateIntent(true));
            }
        });
        getInstrumentation().waitForIdleSync();

        ChatFragment fragment = (ChatFragment) mActivity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        long startTime = System.currentTimeMillis();
        while (fragment == null || fragment instanceof One2OneChatFragment) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            fragment = (ChatFragment) mActivity.getFragmentManager().findFragmentById(
                    R.id.chat_content);
        }
        GroupChatFragment groupChatFragment = (GroupChatFragment) fragment;

        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getWindow((ParcelUuid)groupChatFragment.getChatFragmentTag());
        assertNotNull(window);
        assertEquals(true, window instanceof GroupChatWindow);
        
        participantListDisplayerField = Utils.getPrivateField(GroupChatFragment.class,
                "mParticipantListDisplayer");
        assertNotNull(participantListDisplayerField);

        Field chatWindowManagerField = Utils.getPrivateField(ChatScreenActivity.class,
                "mChatWindowManager");
        final ChatWindowManager chatWindowManager = (ChatWindowManager) chatWindowManagerField
                .get(mActivity);
        HorizontalScrollView groupChatBannerScroller = (HorizontalScrollView) mActivity
                .findViewById(R.id.group_chat_banner_scroller);
        assertNotNull(groupChatBannerScroller);

        ChatScreenWindowContainer.getInstance().setFocusWindow(window);
        final View expandView = mActivity.findViewById(R.id.group_chat_expand);
        assertNotNull(expandView);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                chatWindowManager.onClick(expandView);
            }
        });
        getInstrumentation().waitForIdleSync();
        waitForDisplay(true, groupChatFragment);
        assertEquals(View.VISIBLE, groupChatBannerScroller.getVisibility());

        ChatScreenWindowContainer.getInstance().setFocusWindow(window);
        final View collapseView = mActivity.findViewById(R.id.group_chat_collapse);
        assertNotNull(collapseView);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                chatWindowManager.onClick(collapseView);
            }
        });
        getInstrumentation().waitForIdleSync();
        waitForDisplay(false, groupChatFragment);
        assertEquals(View.GONE, groupChatBannerScroller.getVisibility());

        ChatScreenWindowContainer.getInstance().setFocusWindow(window);
        final View titleLayout = mActivity.findViewById(R.id.group_chat_title_layout);
        assertNotNull(titleLayout);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                chatWindowManager.onClick(titleLayout);
            }
        });
        getInstrumentation().waitForIdleSync();
        waitForDisplay(true, groupChatFragment);
        assertEquals(View.VISIBLE, groupChatBannerScroller.getVisibility());

        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                chatWindowManager.onClick(titleLayout);
            }
        });
        getInstrumentation().waitForIdleSync();
        waitForDisplay(false, groupChatFragment);
        assertEquals(View.GONE, groupChatBannerScroller.getVisibility());
    }

    /**
     * Test the function checkInviatation()
     */
    public void testCase12_checkInviatation() throws Throwable {
        Logger.d(TAG, "testCase12_checkInviatation() entry!");
        Method checkInvitation = Utils.getPrivateMethod(ChatScreenActivity.class,
                "checkInvitation", Intent.class);
        Intent intentA = new Intent();
        intentA.putExtra("sessionId", "123456");
        Boolean result = (Boolean) checkInvitation.invoke(mActivity, intentA);
        assertEquals(false, result.booleanValue());

        ApiManager.initialize(getInstrumentation().getTargetContext());
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), null);
        result = (Boolean) checkInvitation.invoke(mActivity, intentA);
        assertEquals(false, result.booleanValue());

        Intent intentB = new Intent();
        intentB.putExtra("sessionId", "");
        result = (Boolean) checkInvitation.invoke(mActivity, intentB);
        assertEquals(false, result.booleanValue());
    }

    /**
     * Test the function onConfigurationChanged()
     */
    public void testCase13_onConfigurationChanged() throws Throwable {
        Logger.d(TAG, "testCase13_onConfigurationChanged() entry!");
        final ChatFragment fragment = (ChatFragment) mActivity.getFragmentManager()
                .findFragmentById(R.id.chat_content);
        assertNotNull(fragment);
        final Method showImm = Utils.getPrivateMethod(ChatFragment.class, "showImm", Boolean.class);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                try {
                    showImm.invoke(fragment, true);
                } catch (Exception t) {
                    t.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();

        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                int original = mActivity.getRequestedOrientation();
                if (original == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE) {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                } else {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                }
            }
        });
        mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        getInstrumentation().waitForIdleSync();
        assertEquals(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT, mActivity.getRequestedOrientation());
    }

    /**
     * Test the function handleIntentAction() with empty number or filelist or
     * intent.
     */
    public void teseCase14_handleIntentAction() throws Throwable {
        Logger.d(TAG, "teseCase14_handleIntentAction() entry!");
        long timeOut = 1000l;
        ActivityMonitor am = getInstrumentation().addMonitor(ChatScreenActivity.class.getName(),
                null, false);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, null);
            }
        });
        getInstrumentation().waitForIdleSync();
        Activity activity = am.waitForActivityWithTimeout(timeOut);
        assertNull(activity);

        final Intent intent = new Intent(getInstrumentation().getTargetContext(),
                ChatScreenActivity.class);
        intent.setAction(PluginApiManager.RcseAction.FT_ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });
        getInstrumentation().waitForIdleSync();
        activity = am.waitForActivityWithTimeout(timeOut);
        assertNull(activity);

        String number = "1234567890";
        intent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, number);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });
        getInstrumentation().waitForIdleSync();
        activity = am.waitForActivityWithTimeout(timeOut);
        assertNull(activity);

        ArrayList<String> listTestFiles = null;
        intent.putExtra(PluginApiManager.RcseAction.MULTIPLE_FILE_URI, listTestFiles);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getInstrumentation().callActivityOnNewIntent(mActivity, intent);
            }
        });
        getInstrumentation().waitForIdleSync();
        activity = am.waitForActivityWithTimeout(timeOut);
        assertNull(activity);

        getInstrumentation().removeMonitor(am);
    }

    private ChatFragment waitForChatFragment() throws Throwable {
        ChatFragment fragment = (ChatFragment) mActivity.getFragmentManager().findFragmentById(
                R.id.chat_content);
        long startTime = System.currentTimeMillis();
        while (fragment == null || fragment == currentFragment) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            fragment = (ChatFragment) mActivity.getFragmentManager().findFragmentById(
                    R.id.chat_content);
        }
        return fragment;
    }

    private void waitForDisplay(boolean expected, GroupChatFragment groupChatFragment)
            throws Throwable {
        Object participantListDisplayer = participantListDisplayerField.get(groupChatFragment);
        Field portraitStrategyField = Utils.getPrivateField(participantListDisplayer.getClass(),
                "mPortraitStrategy");
        assertNotNull(portraitStrategyField);
        Object portraitStrategy = portraitStrategyField.get(participantListDisplayer);
        Field isExpandField = Utils.getPrivateField(portraitStrategy.getClass(), "mIsExpand");
        assertNotNull(isExpandField);
        boolean isExpand = isExpandField.getBoolean(portraitStrategy);
        long startTime = System.currentTimeMillis();
        while (isExpand != expected) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("Wait for Timeout");
            }
            Thread.sleep(SLEEP_TIME);
            isExpand = isExpandField.getBoolean(portraitStrategy);
        }
    }
}
