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
import android.app.AlertDialog;
import android.app.DialogFragment;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.ParcelUuid;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.widget.ImageView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class ChatMainActivityTest extends
        ActivityInstrumentationTestCase2<ChatMainActivity> {
    private static final String TAG = "ChatMainActivityTest";
    private ChatMainActivity mActivity = null;
    private DialogFragment mDialogFragment;

    private static final int TIME_OUT = 5000;
    private final static String MOCK_NUMBER = "+3402222222";
    private final static Participant MOCK_121_PARTICIPANT = new Participant(
            MOCK_NUMBER, MOCK_NUMBER);
    private final static String MOCK_NUMBER_111 = "+3401111111";
    private final static Participant MOCK_111_PARTICIPANT = new Participant(
            MOCK_NUMBER_111, MOCK_NUMBER_111);

    public ChatMainActivityTest() {
        super(ChatMainActivity.class);
    }

    @Override
    protected void tearDown() throws Exception {
        Utils.clearAllStatus();
        super.tearDown();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
    }

    public void testCase1_testOptionsItem() throws Throwable {
        Logger.v(TAG, "testCase1_testOptionsItem() enter");
        ParcelUuid secondChatTag = new ParcelUuid(UUID.randomUUID());
        List<Participant> secondParticipants = new ArrayList<Participant>();
        secondParticipants.add(new Participant("+34200000247", ""));
        ModelImpl.getInstance().addChat(secondParticipants, secondChatTag, null);
        getInstrumentation().waitForIdleSync();

        List list = ModelImpl.getInstance().listAllChat();
        assertTrue("ModelImpl.getInstance() addChat is fail.", list.size() > 0);

        mActivity = getActivity();

        ActivityMonitor am = getInstrumentation().addMonitor(
                SettingsActivity.class.getName(), null, false);
        boolean result = getInstrumentation().invokeMenuActionSync(mActivity,
                R.id.menu_more_settings, 0);
        assertTrue(result);
        Activity settingsActivity = getInstrumentation()
                .waitForMonitorWithTimeout(am, TIME_OUT);
        try {
            assertNotNull(mActivity);
            assertNotNull(settingsActivity);
        } finally {
            getInstrumentation().removeMonitor(am);
            if (settingsActivity != null) {
                settingsActivity.finish();
            }
        }

        getInstrumentation().waitForIdleSync();
        result = getInstrumentation().invokeMenuActionSync(mActivity,
                R.id.menu_close_all, 0);
        assertTrue("invoke menu_close_all false ", result);

        getInstrumentation().waitForIdleSync();
        mDialogFragment = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag("ChatMainActivity");
        assertNotNull(mDialogFragment);
        assertTrue("dialogFragment getShowsDialog return false ",
                mDialogFragment.getShowsDialog());
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((AlertDialog) mDialogFragment.getDialog()).getButton(
                        AlertDialog.BUTTON_POSITIVE).performClick();
            }
        });
        getInstrumentation().waitForIdleSync();

        list = ModelImpl.getInstance().listAllChat();
        assertTrue("Don't close all action.", list.size() == 0);

        getInstrumentation().invokeMenuActionSync(mActivity,
                R.id.menu_close_all, 0);
        getInstrumentation().waitForIdleSync();
        mDialogFragment = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag("ChatMainActivity");
        assertNotNull("dialogFragment is null", mDialogFragment);
        assertTrue("dialogFragment getShowsDialog return false ",
                mDialogFragment.getShowsDialog());
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((AlertDialog) mDialogFragment.getDialog()).getButton(
                        AlertDialog.BUTTON_NEGATIVE).performClick();
            }
        });

        getInstrumentation().waitForIdleSync();
        getInstrumentation().invokeMenuActionSync(mActivity,
                R.id.menu_close_all, 0);
        getInstrumentation().waitForIdleSync();
        sendKeys(KeyEvent.KEYCODE_BACK);
        getInstrumentation().waitForIdleSync();
    }

    public void testCase2_testOnClick() throws Throwable {
        Logger.v(TAG, "testCase2_testOnClick() enter");
        mActivity = getActivity();
        sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
        getInstrumentation().waitForIdleSync();
        ActivityMonitor am = null;
        if (ContactsListManager.IS_SUPPORT) {
            IntentFilter intentFilter = new IntentFilter(
                    ChatMainActivity.ACTION_START_CONTACT);
            am = getInstrumentation().addMonitor(intentFilter, null, false);
        } else {
            am = getInstrumentation().addMonitor(
                    SelectContactsActivity.class.getName(), null, false);
        }
        final ImageView button = (ImageView) mActivity
                .findViewById(R.id.add_contacts);
        getInstrumentation().waitForIdleSync();
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                button.performClick();
            }
        });
        getInstrumentation().waitForIdleSync();
        Activity activity = getInstrumentation().waitForMonitorWithTimeout(am,
                TIME_OUT);
        try {
            assertNotNull(mActivity);
            assertNotNull(activity);
            sendKeys(KeyEvent.KEYCODE_BACK);
            getInstrumentation().waitForIdleSync();
        } finally {
            getInstrumentation().removeMonitor(am);
            if (activity != null) {
                activity.finish();
            }
        }
    }

    public void testCase3_testActivityResult() throws Throwable {
        Logger.v(TAG, "testCase3_testActivityResult() enter");
        mActivity = getActivity();
        Intent intent = new Intent(mActivity, SelectContactsActivity.class);
        ActivityMonitor am = getInstrumentation().addMonitor(
                SelectContactsActivity.class.getName(), null, false);

        mActivity.startActivityForResult(intent,
                ChatFragment.RESULT_CODE_ADD_CONTACTS);
        getInstrumentation().waitForIdleSync();

        Activity activity = getInstrumentation().waitForMonitorWithTimeout(am,
                TIME_OUT);
        ActivityMonitor chatScreenActivityAm;
        try {
            assertNotNull(mActivity);
            assertNotNull(activity);

            ArrayList<Participant> participants = new ArrayList<Participant>();
            participants.add(MOCK_121_PARTICIPANT);
            participants.add(MOCK_111_PARTICIPANT);
            Intent data = new Intent();
            data.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST,
                    participants);
            chatScreenActivityAm = getInstrumentation().addMonitor(
                    ChatScreenActivity.class.getName(), null, false);
            activity.setResult(Activity.RESULT_OK, data);
            getInstrumentation().waitForIdleSync();
        } finally {
            getInstrumentation().removeMonitor(am);
            if (activity != null) {
                activity.finish();
            }
        }
        Activity chatScreenActivity = getInstrumentation()
                .waitForMonitorWithTimeout(chatScreenActivityAm, TIME_OUT);
        try {
            assertNotNull("ChatScreenActivity activity is not create ",
                    chatScreenActivity);
            getInstrumentation().waitForIdleSync();
            sendKeys(KeyEvent.KEYCODE_BACK);
            getInstrumentation().waitForIdleSync();
        } finally {
            getInstrumentation().removeMonitor(chatScreenActivityAm);
            if (chatScreenActivity != null) {
                chatScreenActivity.finish();
            }
        }

    }

}
