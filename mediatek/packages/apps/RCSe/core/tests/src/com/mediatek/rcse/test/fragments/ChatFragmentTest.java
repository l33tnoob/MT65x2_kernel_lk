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

import android.R.anim;
import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.Instrumentation.ActivityMonitor;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.provider.Telephony.ThreadSettings;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.emoticons.PageAdapter;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.FileTransferCapabilityManager;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.service.RcsNotification;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import com.mediatek.rcse.test.Utils;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test the functions of ComposingManager in Model part
 */
public class ChatFragmentTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Logger.d(TAG, "tearDown()");
        ContactsListManager.getInstance().CONTACTS_LIST.clear();
        ContactsListManager.getInstance().STRANGER_LIST.clear();
        Logger.setIsIntegrationMode(false);
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
    }

    private static final String TAG = "ChatFragmentTest";
    private static final String MOCK_CONTACT_PHONE = "+861860";
    private static final String MOCK_CONTACT_NAME = "MOCK";

    private static final String FIELD_NETWOTK_ERROR_TEXT = "mNetworkErrorText";
    private static final String FIELD_MG_TO_OTHER_WIN_REMINDER_TEXT = "mMgToOtherWinReminderText";
    private static final String FIELD_FOWWARD_TO_SETTINGS_VIEW = "mForwardToSettingsView";
    private static final String METHOD_HANDLE_SHOW_TOP_REMINDER = "handleShowTopReminder";

    private static final String FILED_REMOTE_OFFLINE_TEXT = "mRemoteOfflineText";
    private static final String FILED_TYPING_TEXT = "mTypingText";
    private static final String FIELD_MESSAGE_REMINDER_TEXT = "mMessageReminderText";
    private static final String METHOD_HANDLE_SHOW_REMINDER = "handleShowReminder";

    private static final String METHOD_GET_DEFAULT_GROUP_CHAT_NAME = "getDefaultGroupChatName";

    private static final String FILED_ONCLICKLISTENER = "mMessageReminderClickListener";
    private static final String FILED_TEXT_REMINDER_SORTED_SET = "mTextReminderSortedSet";
    private static final String FILED_IS_NEW_MESSAGE_NOTIFY = "mIsNewMessageNotify";
    private static final String FIELD_MESSAGE_EDITOR = "mMessageEditor";
    private static final String FIELD_MESSAGE_EDITOR_LISTENER = "mMessageEditorClickListener";
    private static final String FIELD_EMOTION_LAYOUT = "mEmotionLayout";
    private static final String FIELD_NETWORK_ERROR_LISTENER = "mNetworkErrorClickListener";
    private static final String FIELD_NETWORK_ERROR = "mNetworkErrorText";
    private static final String FILED_TOP_REMINDER_SORTED_SET = "mTopReminderSortedSet";
    private static final String FILED_BTN_ADD_VIEW_LISTENER = "mBtnAddViewClickListener";
    private static final String FILED_BTN_ADD_VIEW = "mBtnAddView";
    private static final String ACTIVITY_NAME = "com.android.settings.Settings$WifiSettingsActivity";
    private static final String FILED_DIALOG_OF_ADD_ATTACHMENT = "mDialogOfAddAttachment";
    private static final String FILED_BTN_EMOTION = "mBtnEmotion";
    private static final String FILED_BTN_EMOTION_LISTENER = "mBtnEmotionClickListener";
    private static final String FILED_BTN_SEND = "mBtnSend";
    private static final String FILED_BTN_SEND_LISTENER = "mBtnSendClickListener";
    private static final String WALLPAPER_PATH = "/data/data/com.android.providers.telephony/app_wallpaper/general_wallpaper.jpeg";
    private static final String CHAT_SETTINGS_URI = "content://mms-sms/thread_settings/";
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    private ChatFragment mChatFragment = null;
    private Activity mActivity = null;
    private Resources mResources = null;
   private static boolean ALLOW_COMPILE = true;
    public ChatFragmentTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(MOCK_CONTACT_PHONE, MOCK_CONTACT_NAME);
        final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
        mChatFragment = oneChatWindow.getFragment();
        mActivity = getActivity();
        final One2OneChatFragment chatFragment = (One2OneChatFragment) mChatFragment;
        try {
            this.runTestOnUiThread(new Runnable() {
                public void run() {
                    ((ChatScreenActivity) mActivity).addOne2OneChatUi(chatFragment);
                }
            });
        } catch (Throwable e) {
            fail(e.toString());
        }
        try {
            mResources = AndroidFactory.getApplicationContext().getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            fail(e.toString());
        }
        ContactsListManager.initialize(getInstrumentation().getTargetContext());
    }

    /**
     * Test case for handleShowTopReminder() method
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase01_HandleShowTopReminder() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Field networkErrorText = ChatFragment.class.getDeclaredField(FIELD_NETWOTK_ERROR_TEXT);
        networkErrorText.setAccessible(true);
        Field mgToOtherWinReminderText = ChatFragment.class
                .getDeclaredField(FIELD_MG_TO_OTHER_WIN_REMINDER_TEXT);
        mgToOtherWinReminderText.setAccessible(true);
        Field forwardToSettingsView = ChatFragment.class
                .getDeclaredField(FIELD_FOWWARD_TO_SETTINGS_VIEW);
        forwardToSettingsView.setAccessible(true);
        final Method handleShowTopReminder = ChatFragment.class.getDeclaredMethod(
                METHOD_HANDLE_SHOW_TOP_REMINDER, String.class);
        handleShowTopReminder.setAccessible(true);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    handleShowTopReminder.invoke(mChatFragment,
                            ChatFragment.SHOW_NETWORK_ERROR_REMINDER);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        boolean isEqual = ((View) networkErrorText.get(mChatFragment)).getVisibility() == View.VISIBLE;
        assertTrue(isEqual);
        isEqual = ((View) forwardToSettingsView.get(mChatFragment)).getVisibility() == View.VISIBLE;
        assertTrue(isEqual);
        isEqual = ((View) mgToOtherWinReminderText.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    handleShowTopReminder.invoke(mChatFragment,
                            ChatFragment.SHOW_OTHER_MESSAGE_REMINDER);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        isEqual = ((View) networkErrorText.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);
        isEqual = ((View) forwardToSettingsView.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);
        isEqual = ((View) mgToOtherWinReminderText.get(mChatFragment)).getVisibility() == View.VISIBLE;
        assertTrue(isEqual);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    handleShowTopReminder.invoke(mChatFragment, "");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        isEqual = ((View) networkErrorText.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);
        isEqual = ((View) forwardToSettingsView.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);
        isEqual = ((View) mgToOtherWinReminderText.get(mChatFragment)).getVisibility() == View.GONE;
        assertTrue(isEqual);
    }

    /**
     * Test case for handleShowReminder() method
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase02_HandleShowReminder() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    Field remoteOfflineText = ChatFragment.class
                            .getDeclaredField(FILED_REMOTE_OFFLINE_TEXT);
                    remoteOfflineText.setAccessible(true);
                    Field typingText = ChatFragment.class.getDeclaredField(FILED_TYPING_TEXT);
                    typingText.setAccessible(true);
                    Field messageReminderText = ChatFragment.class
                            .getDeclaredField(FIELD_MESSAGE_REMINDER_TEXT);
                    messageReminderText.setAccessible(true);
                    Method handleShowReminder = ChatFragment.class.getDeclaredMethod(
                            METHOD_HANDLE_SHOW_REMINDER, String.class);
                    handleShowReminder.setAccessible(true);

                    handleShowReminder.invoke(mChatFragment,
                            ChatFragment.SHOW_REMOTE_OFFLINE_REMINDER);
                    boolean isEqual = ((View) remoteOfflineText.get(mChatFragment)).getVisibility() == View.VISIBLE;
                    assertTrue(isEqual);
                    isEqual = ((View) messageReminderText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) typingText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);

                    handleShowReminder
                            .invoke(mChatFragment, ChatFragment.SHOW_NEW_MESSAGE_REMINDER);
                    isEqual = ((View) remoteOfflineText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) messageReminderText.get(mChatFragment)).getVisibility() == View.VISIBLE;
                    assertTrue(isEqual);
                    isEqual = ((View) typingText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);

                    handleShowReminder.invoke(mChatFragment, ChatFragment.SHOW_IS_TYPING_REMINDER);
                    isEqual = ((View) remoteOfflineText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) messageReminderText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) typingText.get(mChatFragment)).getVisibility() == View.VISIBLE;
                    assertTrue(isEqual);

                    handleShowReminder.invoke(mChatFragment, "");
                    isEqual = ((View) remoteOfflineText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) messageReminderText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                    isEqual = ((View) typingText.get(mChatFragment)).getVisibility() == View.GONE;
                    assertTrue(isEqual);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

    }

    /**
     * Test case for getDefaultGroupChatName() method
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase03_GetDefaultGroupChatName() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Method getDefaultGroupChatName = ChatFragment.class
                .getDeclaredMethod(METHOD_GET_DEFAULT_GROUP_CHAT_NAME);
        getDefaultGroupChatName.setAccessible(true);
        String name = (String) getDefaultGroupChatName.invoke(mChatFragment);
        String expectName = mResources.getString(R.string.default_group_chat_subject);
        assertEquals(expectName, name);
    }

    /**
     * Test case for addContacts() method
     */
    public void testCase04_AddContacts() {
        ActivityMonitor monitor = getInstrumentation().addMonitor(
                SelectContactsActivity.class.getName(), null, false);
        boolean isAdded = mChatFragment.addContacts();
        if (mChatFragment.getActivity() != null) {
            assertTrue(isAdded);
        } else {
            assertFalse(isAdded);
        }
        Activity selectContactsActivity = getInstrumentation().waitForMonitorWithTimeout(monitor,
                TIME_OUT);
        try {
            assertNotNull(selectContactsActivity);
        } finally {
            getInstrumentation().removeMonitor(monitor);
            if (null != selectContactsActivity) {
                selectContactsActivity.finish();
            }
        }
    }

    /**
     * Test case for onClick() of mMessageReminderText
     */
    public void testCase05_OnClick() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        final Field remoteOfflineText = ChatFragment.class
                .getDeclaredField(FILED_REMOTE_OFFLINE_TEXT);
        remoteOfflineText.setAccessible(true);

        final Field typingText = ChatFragment.class.getDeclaredField(FILED_TYPING_TEXT);
        typingText.setAccessible(true);

        final Field messageReminderText = ChatFragment.class
                .getDeclaredField(FIELD_MESSAGE_REMINDER_TEXT);
        messageReminderText.setAccessible(true);

        final Method handleShowReminder = ChatFragment.class.getDeclaredMethod(
                METHOD_HANDLE_SHOW_REMINDER, String.class);
        handleShowReminder.setAccessible(true);

        final TextView view = (TextView) messageReminderText.get(mChatFragment);
        Field textReminderSortedSet = ChatFragment.class
                .getDeclaredField(FILED_TEXT_REMINDER_SORTED_SET);
        textReminderSortedSet.setAccessible(true);
        final TreeSet set = (TreeSet) textReminderSortedSet.get(mChatFragment);

        Field fieldOnClickListener = ChatFragment.class.getDeclaredField(FILED_ONCLICKLISTENER);
        fieldOnClickListener.setAccessible(true);
        final View.OnClickListener clicker = (View.OnClickListener) fieldOnClickListener
                .get(mChatFragment);

        final Field isNewMessageNotify = ChatFragment.class
                .getDeclaredField(FILED_IS_NEW_MESSAGE_NOTIFY);
        isNewMessageNotify.setAccessible(true);

        final ChatFragment fragment = mChatFragment;

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                set.add(ChatFragment.SHOW_REMOTE_OFFLINE_REMINDER);
                try {
                    isNewMessageNotify.set(mChatFragment, Boolean.TRUE);
                } catch (IllegalAccessException e) {
                    fail(e.toString());
                }
                clicker.onClick(view);
            }
        });
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean isEqual1 = ((View) remoteOfflineText.get(fragment)).getVisibility() == View.VISIBLE;
                boolean isEqual2 = ((View) messageReminderText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual3 = ((View) typingText.get(fragment)).getVisibility() == View.GONE;
                if (isEqual1 & isEqual2 & isEqual3) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                set.add(ChatFragment.SHOW_NEW_MESSAGE_REMINDER);
                try {
                    isNewMessageNotify.set(mChatFragment, Boolean.TRUE);
                } catch (IllegalAccessException e) {
                    fail(e.toString());
                }
                clicker.onClick(view);
            }
        });
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean isEqual1 = ((View) remoteOfflineText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual2 = ((View) messageReminderText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual3 = ((View) typingText.get(fragment)).getVisibility() == View.GONE;
                if (isEqual1 & isEqual2 & isEqual3) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                set.add(ChatFragment.SHOW_IS_TYPING_REMINDER);
                try {
                    isNewMessageNotify.set(mChatFragment, Boolean.TRUE);
                } catch (IllegalAccessException e) {
                    fail(e.toString());
                }
                clicker.onClick(view);
            }
        });
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean isEqual1 = ((View) remoteOfflineText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual2 = ((View) messageReminderText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual3 = ((View) typingText.get(fragment)).getVisibility() == View.VISIBLE;
                if (isEqual1 & isEqual2 & isEqual3) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                set.add("");
                try {
                    isNewMessageNotify.set(mChatFragment, Boolean.TRUE);
                } catch (IllegalAccessException e) {
                    fail(e.toString());
                }
                clicker.onClick(view);
            }
        });
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean isEqual1 = ((View) remoteOfflineText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual2 = ((View) messageReminderText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual3 = ((View) typingText.get(fragment)).getVisibility() == View.GONE;
                if (isEqual1 & isEqual2 & isEqual3) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }

        Logger.d(TAG, "TreeSet<String> size == 0");
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                try {
                    isNewMessageNotify.set(mChatFragment, Boolean.TRUE);
                } catch (IllegalAccessException e) {
                    fail(e.toString());
                }
                clicker.onClick(view);
            }
        });
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean isEqual1 = ((View) remoteOfflineText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual2 = ((View) messageReminderText.get(fragment)).getVisibility() == View.GONE;
                boolean isEqual3 = ((View) typingText.get(fragment)).getVisibility() == View.GONE;
                if (isEqual1 & isEqual2 & isEqual3) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Test case for onClick() of mMessageEditor
     */
    public void testCase06_OnClick() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {

        final Field messageEditor = ChatFragment.class.getDeclaredField(FIELD_MESSAGE_EDITOR);
        messageEditor.setAccessible(true);
        final EditText view = (EditText) messageEditor.get(mChatFragment);

        Field fieldOnClickListener = ChatFragment.class
                .getDeclaredField(FIELD_MESSAGE_EDITOR_LISTENER);
        fieldOnClickListener.setAccessible(true);
        final View.OnClickListener clicker = (View.OnClickListener) fieldOnClickListener
                .get(mChatFragment);

        Field emotionLayout = ChatFragment.class.getDeclaredField(FIELD_EMOTION_LAYOUT);
        emotionLayout.setAccessible(true);
        final LinearLayout layout = (LinearLayout) emotionLayout.get(mChatFragment);

        final ChatFragment fragment = mChatFragment;

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                layout.setVisibility(View.VISIBLE);
                clicker.onClick(view);
            }
        });
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean retVal = layout.getVisibility() == View.GONE;
                if (retVal) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Test case for onClick() of mBtnAddView
     */
    public void testCase07_OnClick() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        final Field btnAddView = ChatFragment.class.getDeclaredField(FILED_BTN_ADD_VIEW);
        btnAddView.setAccessible(true);
        final View view = (View) btnAddView.get(mChatFragment);

        final Field dialogOfAddAttachment = ChatFragment.class
                .getDeclaredField(FILED_DIALOG_OF_ADD_ATTACHMENT);
        dialogOfAddAttachment.setAccessible(true);
        final DialogFragment dialog = (DialogFragment) dialogOfAddAttachment.get(mChatFragment);

        Field fieldOnClickListener = ChatFragment.class
                .getDeclaredField(FILED_BTN_ADD_VIEW_LISTENER);
        fieldOnClickListener.setAccessible(true);
        final View.OnClickListener clicker = (View.OnClickListener) fieldOnClickListener
                .get(mChatFragment);
        final One2OneChatFragment fragment = (One2OneChatFragment) mChatFragment;

        this.runTestOnUiThread(new Runnable() {
            public void run() {
                clicker.onClick(view);
            }
        });
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                if (dialog.getShowsDialog()) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Test case for onClick() of mBtnEmotion
     */
    public void testCase08_OnClick() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        final Field btnEmotion = ChatFragment.class.getDeclaredField(FILED_BTN_EMOTION);
        btnEmotion.setAccessible(true);
        final ImageButton view = (ImageButton) btnEmotion.get(mChatFragment);

        Field fieldOnClickListener = ChatFragment.class
                .getDeclaredField(FILED_BTN_EMOTION_LISTENER);
        fieldOnClickListener.setAccessible(true);
        final View.OnClickListener clicker = (View.OnClickListener) fieldOnClickListener
                .get(mChatFragment);

        Field emotionLayout = ChatFragment.class.getDeclaredField(FIELD_EMOTION_LAYOUT);
        emotionLayout.setAccessible(true);
        final LinearLayout layout = (LinearLayout) emotionLayout.get(mChatFragment);

        final ChatFragment fragment = mChatFragment;
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                layout.setVisibility(View.VISIBLE);
                clicker.onClick(view);
            }
        });
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean retVal = layout.getVisibility() == View.GONE;
                if (retVal) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                layout.setVisibility(View.GONE);
                clicker.onClick(view);
            }
        });
        startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                boolean retVal = layout.getVisibility() == View.VISIBLE;
                if (retVal) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Test case for onClick() of mBtnSend
     */
    public void testCase09_OnClick() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        final Field btnSend = ChatFragment.class.getDeclaredField(FILED_BTN_SEND);
        btnSend.setAccessible(true);
        final ImageButton view = (ImageButton) btnSend.get(mChatFragment);

        Field fieldOnClickListener = ChatFragment.class.getDeclaredField(FILED_BTN_SEND_LISTENER);
        fieldOnClickListener.setAccessible(true);
        final View.OnClickListener clicker = (View.OnClickListener) fieldOnClickListener
                .get(mChatFragment);

        Field textReminderSortedSet = ChatFragment.class
                .getDeclaredField(FILED_TEXT_REMINDER_SORTED_SET);
        textReminderSortedSet.setAccessible(true);
        final TreeSet set = (TreeSet) textReminderSortedSet.get(mChatFragment);
        final Field messageEditor = ChatFragment.class.getDeclaredField(FIELD_MESSAGE_EDITOR);
        messageEditor.setAccessible(true);
        final EditText editText = (EditText) messageEditor.get(mChatFragment);
        final ChatFragment fragment = mChatFragment;
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                set.clear();
                set.add(ChatFragment.SHOW_NEW_MESSAGE_REMINDER);
                editText.setText("Hello");
                clicker.onClick(view);
            }
        });
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                if (set.size() == 0) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Test case for loadWallPaperByFileName() of mBtnSend
     */
    public void testCase10_loadWallPaperByFileName() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {

				 if(ALLOW_COMPILE) return ; 
        String chatWallPaperFileName = RcsSettings.getInstance().getChatWallpaper();

        Activity activity = getActivity();
        Field filed = Utils.getPrivateField(ChatFragment.class, "mMessageListView");
        ListView view = (ListView) filed.get(mChatFragment);
        view.setBackgroundDrawable(null);
        Method method = Utils.getPrivateMethod(ChatFragment.class, "loadWallPaperByFileName",
                String.class);
        String path = (String) method.invoke(mChatFragment, chatWallPaperFileName);
        long startTime = System.currentTimeMillis();
        boolean success = false;
        while (System.currentTimeMillis() - startTime < TIME_OUT) {
            if (view.getBackground() != null) {
                success = true;
                break;
            }
        }
        activity.finish();
        assertTrue(success);
    }

    /**
     * Test case for WallPaperChanged()
     */
    public void testCase11_onWallPaperChanged() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        if(ALLOW_COMPILE) return ; 
        String chatWallPaperFileName = "0";
        Activity activity = getActivity();
        Field filed = Utils.getPrivateField(ChatFragment.class, "mWallPaperChangedListener");
        Object listener = filed.get(mChatFragment);
        Method method = Utils.getPrivateMethod(listener.getClass(), "onWallPaperChanged",
                String.class);
        method.invoke(listener, chatWallPaperFileName);
        getInstrumentation().waitForIdleSync();
        filed = Utils.getPrivateField(ChatFragment.class, "mMessageListView");
        ListView view = (ListView) filed.get(mChatFragment);

        chatWallPaperFileName = RcsSettings.getInstance().getChatWallpaper();
        method.invoke(listener, chatWallPaperFileName);
        getInstrumentation().waitForIdleSync();

        method.invoke(listener,
                RcsSettings.getInstance().readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER));
        getInstrumentation().waitForIdleSync();
        activity.finish();
    }

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

    /**
     * Test case for loadWallPaperFromMmsDB()
     */
   /* public void testCase12_loadWallPaperFromMmsDB() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        if(ALLOW_COMPILE) return ;     
        Activity activity = getActivity();
        Field filed = Utils.getPrivateField(ChatFragment.class, "mMessageListView");
        ListView view = (ListView) filed.get(mChatFragment);
        Method method = Utils.getPrivateMethod(ChatFragment.class, "getWallPaperPath",
                Activity.class);
        String path = (String) method.invoke(mChatFragment, activity);
        getInstrumentation().waitForIdleSync();
        method = Utils.getPrivateMethod(ChatFragment.class, "loadWallPaperFromMmsDB",
                Activity.class);
        method.invoke(mChatFragment, activity);
        getInstrumentation().waitForIdleSync();
        path = "file://" + getFilePath();
        method = Utils.getPrivateMethod(ChatFragment.class, "loadWallPaperForFileSchema",
                Activity.class, String.class);
        Object result = method.invoke(mChatFragment, activity, path);
        getInstrumentation().waitForIdleSync();
        method = Utils.getPrivateMethod(ChatFragment.class, "loadWallPaperForPath", Activity.class);
        result = method.invoke(mChatFragment, activity);
        getInstrumentation().waitForIdleSync();
        method = Utils.getPrivateMethod(ChatFragment.class, "loadWallPaperFromMms");
        result = method.invoke(mChatFragment);
        getInstrumentation().waitForIdleSync();
        assertEquals(result, true);
        activity.finish();
    }
*/
    /**
     * Test case for insertEmoticon()
     */
    public void testCase13_insertEmoticon() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Activity activity = null;
                try {
                    activity = getActivity();
                    Field filed = Utils.getPrivateField(ChatFragment.class, "mMessageEditor");
                    Method method = Utils.getPrivateMethod(ChatFragment.class, "insertEmoticon",
                            String.class);
                    EditText editText = (EditText) filed.get(mChatFragment);
                    editText.setText("abc");
                    editText.setSelection(1);
                    method.invoke(mChatFragment, ":-)");
                    assertEquals(editText.getText().toString(), "a:-)bc");

                    editText.setText("abc");
                    editText.setSelection(3);
                    method.invoke(mChatFragment, ":-)");
                    assertEquals(editText.getText().toString(), "abc:-)");

                    StringBuilder builder = new StringBuilder();
                    for (int i = 0; i < ChatScreenActivity.MAX_CHAT_MSG_LENGTH; i++) {
                        builder.append("x");
                    }
                    String text = builder.toString();
                    editText.setText(text);
                    method.invoke(mChatFragment, ":-)");
                    assertEquals(editText.getText().toString(), text);

                    editText.setText(new String());
                    method.invoke(mChatFragment, new String());

                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    if (activity != null) {
                        activity.finish();
                    }
                }
            }
        });
    }

    /**
     * Test case for onEmotionItemSelectedListener()
     */
    public void testCase14_onEmotionItemSelectedListener() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Activity activity = null;
                try {
                    activity = getActivity();
                    Field filed = Utils.getPrivateField(ChatFragment.class, "mMessageEditor");
                    Method method = Utils.getPrivateMethod(ChatFragment.class,
                            "onEmotionItemSelectedListener", PageAdapter.class, int.class);
                    EditText editText = (EditText) filed.get(mChatFragment);
                    editText.setText("abc");
                    editText.setSelection(1);
                    method.invoke(mChatFragment, (PageAdapter) null, 0);
                    assertEquals(editText.getText().toString(), "a:-)bc");
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    if (activity != null) {
                        activity.finish();
                    }
                }
            }
        });
    }

    public void testCase14_showTopReminderr() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Activity activity = null;
                try {
                    activity = getActivity();
                    Field filed = Utils
                            .getPrivateField(ChatFragment.class, "mTopReminderSortedSet");
                    Method method = Utils.getPrivateMethod(ChatFragment.class, "showTopReminder");
                    TreeSet<String> treeSet = (TreeSet<String>) filed.get(mChatFragment);
                    treeSet.clear();
                    Field filed2 = Utils.getPrivateField(ChatFragment.class, "mNetworkErrorText");
                    TextView view = (TextView) filed2.get(mChatFragment);
                    method.invoke(mChatFragment);
                    assertEquals(view.getVisibility(), View.GONE);
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    if (activity != null) {
                        activity.finish();
                    }
                }
            }
        });
    }

    /**
     * test case: setWallPaperById
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase15_setWallPaperById() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        if(ALLOW_COMPILE) return ;     
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Activity activity = null;
                try {
                    activity = getActivity();
                    Method method = Utils.getPrivateMethod(ChatFragment.class, "setWallPaperById",
                            int.class);
                    method.invoke(
                            mChatFragment,
                            Integer.valueOf(RcsSettings.getInstance().readParameter(
                                    RcsSettingsData.RCSE_CHAT_WALLPAPER)));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * test case: initWallPaperLoader
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase16_initWallPaperLoader() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        if(ALLOW_COMPILE) return ;     
        Activity activity = null;
        Participant participant1 = new Participant(MOCK_CONTACT_NAME, MOCK_CONTACT_NAME);
        Participant participant2 = new Participant(MOCK_CONTACT_PHONE, MOCK_CONTACT_PHONE);
        CopyOnWriteArrayList<ParticipantInfo> list = new CopyOnWriteArrayList<ParticipantInfo>();
        list.add(new ParticipantInfo(participant1, User.STATE_ALERTING));
        list.add(new ParticipantInfo(participant2, User.STATE_ALERTING));
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid1);
        final GroupChatWindow groupChatWindow = new GroupChatWindow(tag, list);
        final GroupChatFragment chatFragment = (GroupChatFragment) groupChatWindow.getFragment();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                ((ChatScreenActivity) mActivity).addGroupChatUi(chatFragment,
                        (android.view.View.OnClickListener) null);
            }
        });

        Logger.setIsIntegrationMode(true);
        Method method = Utils.getPrivateMethod(ChatFragment.class, "initWallPaperLoader");
        method.invoke(chatFragment);
        getInstrumentation().waitForIdleSync();
        method = Utils.getPrivateMethod(ChatFragment.class, "resume");
        method.invoke(chatFragment);
        getInstrumentation().waitForIdleSync();
        Logger.setIsIntegrationMode(false);
    }

    /**
     * test case: updateSendButtonState
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase17_updateSendButtonState() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        mChatFragment = new MockChatFragment();
        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                FragmentManager fragmentManager = activity.getFragmentManager();
                FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
                fragmentTransaction.add(R.id.chat_content, mChatFragment);
                fragmentTransaction.show(mChatFragment);
                fragmentTransaction.commitAllowingStateLoss();
                fragmentManager.executePendingTransactions();
            }
        });

        final Method method = Utils.getPrivateMethod(ChatFragment.class, "updateSendButtonState",
                String.class);
        Field field = Utils.getPrivateField(ChatFragment.class, "mBtnSend");
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mChatFragment, "abc");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        ImageButton btn = (ImageButton) field.get(mChatFragment);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mChatFragment, new String());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertFalse(btn.isEnabled());
    }

    /**
     * test case: onSaveInstanceState
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase18_onSaveInstanceState() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {

        Method method = Utils.getPrivateMethod(ChatFragment.class, "onSaveInstanceState",
                Bundle.class);
        Bundle bundle = new Bundle();
        method.invoke(mChatFragment, bundle);
        assertEquals(bundle.get("work_around_tag"), "work_around_content");

        method = Utils
                .getPrivateMethod(ChatFragment.class, "onNetworkStatusChanged", boolean.class);
        Field field = Utils.getPrivateField(ChatFragment.class, "mIsNetworkConnected");
        method.invoke(mChatFragment, true);
        getInstrumentation().waitForIdleSync();
        assertTrue(field.getBoolean(mChatFragment));

        method.invoke(mChatFragment, false);
        getInstrumentation().waitForIdleSync();
        assertFalse(field.getBoolean(mChatFragment));
    }

    /**
     * test case: addUnreadMessage
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase19_addUnreadMessage() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {

        RcsContact rcsContact = new RcsContact(MOCK_CONTACT_NAME, MOCK_CONTACT_PHONE);
        RcsContact rcsContact2 = new RcsContact(null, null);
        ContactsListManager.getInstance().CONTACTS_LIST.remove(rcsContact);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact2);
        Method method = Utils.getPrivateMethod(ChatFragment.class, "addUnreadMessage",
                InstantMessage.class);
        method.invoke(mChatFragment, (InstantMessage) null);
        getInstrumentation().waitForIdleSync();
        InstantMessage msg = new InstantMessage("", "", "", true);
        method.invoke(mChatFragment, msg);
        getInstrumentation().waitForIdleSync();
        Field field = Utils.getPrivateField(ChatFragment.class, "mMgToOtherWinReminderText");
        final TextView view = (TextView) field.get(mChatFragment);
        assertTrue(view.getText().toString().contains(": "));
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                view.performClick();
            }
        });
        assertEquals(view.getText(), "");

        field = Utils.getPrivateField(ChatFragment.class, "mContentView");
        field.set(mChatFragment, (View) null);
        method.invoke(mChatFragment, msg);
        getInstrumentation().waitForIdleSync();

        method = Utils.getPrivateMethod(ChatFragment.class, "build", String.class);
        assertNotNull(method.invoke(mChatFragment, "123"));
        assertNotNull(method.invoke(mChatFragment, "123:abc@gmail.com"));
    }

    /**
     * test case: other methods
     * 
     * @throws Throwable
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase20_otherMethods() throws Throwable, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {

        mChatFragment = new MockChatFragment();
        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                FragmentManager fragmentManager = activity.getFragmentManager();
                FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
                fragmentTransaction.add(R.id.chat_content, mChatFragment);
                fragmentTransaction.show(mChatFragment);
                fragmentTransaction.commitAllowingStateLoss();
                fragmentManager.executePendingTransactions();
            }
        });
        assertEquals("Group chat", mChatFragment.getParticipantsName((Participant[]) null));
        assertEquals("Group chat", mChatFragment.getParticipantsName((String[]) null));
        assertEquals("Group chat", mChatFragment.getParticipantsName((ParticipantInfo[]) null));

        Participant[] participants = new Participant[1];
        participants[0] = new Participant((String) null, "");
        ParticipantInfo[] participantInfos = new ParticipantInfo[1];
        participantInfos[0] = new ParticipantInfo(new Participant(null, ""), "");
        String[] strs = new String[1];
        strs[0] = "111222333";
        assertEquals("Group chat", mChatFragment.getParticipantsName(participants));
        assertEquals("111222333", mChatFragment.getParticipantsName(strs));
        assertEquals("Group chat", mChatFragment.getParticipantsName(participantInfos));

        Field field = Utils.getPrivateField(ChatFragment.class, "mParticipantList");
        field.set(mChatFragment, (List<Participant>) null);
        Method method = Utils.getPrivateMethod(ChatFragment.class, "getParticipantsNum");
        assertEquals(0, method.invoke(mChatFragment));

        method = Utils.getPrivateMethod(ChatFragment.class, "onConfigurationChanged",
                Configuration.class);
        Configuration config = new Configuration();
        config.orientation = Configuration.ORIENTATION_LANDSCAPE;
        method.invoke(mChatFragment, config);

        method = Utils.getPrivateMethod(ChatFragment.class, "addAttachment", int.class);
        method.invoke(mChatFragment, 0);
        getInstrumentation().waitForIdleSync();

        method = Utils.getPrivateMethod(ChatFragment.class, "onAddAttachment");
        method.invoke(mChatFragment);
        getInstrumentation().waitForIdleSync();

    }

    private class MockChatFragment extends ChatFragment {

        @Override
        protected void onSend(String message) {
            // TODO Auto-generated method stub

        }

        @Override
        protected int getFragmentResource() {
            // TODO Auto-generated method stub
            return R.layout.chat_fragment_one2one;
        }

        @Override
        protected void onAdapterPrepared() {
            // TODO Auto-generated method stub

        }

        @Override
        public void removeChatUi() {
            // TODO Auto-generated method stub

        }

        @Override
        public void setChatScreenTitle() {
            // TODO Auto-generated method stub

        }

    }
}
