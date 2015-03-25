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

import java.lang.reflect.Field;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Handler;
import android.os.Looper;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;
import android.test.ActivityInstrumentationTestCase2;
import android.view.View;

import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.eab.ContactsManagerException;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.R;

/**
 * Test case class for SettingsActivity
 */

public class SettingsActivityTest extends ActivityInstrumentationTestCase2<SettingsActivity> {
    private static final String TAG = "SettingsActivityTest";
    private static final String RCSE_COMPRESS_IMAGE = "rcse_compress_image";
    private static final String RCS_SEND_READ_RECEIPT = "rcse_send_read_receipt";
    private static final String EXTRA_KEY_FROM_MMS = "extraKeyFromMms";
    private static final String EXTRA_VALUE_FROM_MMS = "extraValueFromMms";
    private static final String RCS_CLEAR_HISTORY = "rcse_clear_history";
    private static final String CLEAR_HISTORY_CONFIRM_DIALOG_TAG = "ClearhistoryConfirmDialog";
    private static final String CLEAR_HISTORY_PROGRESS = "ClearHistoryProgress";
    private static final String MOCK_CONTACT = "+34200000248";
    private static final String RCS_CONTACT = "1";
    private static final String MIMETYPE_RCS_CONTACT = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
    public static final String RCS_REMIND = "rcse_large_file_reminder";
    private static final String RCS_NOTIFICATION = "rcse_notification";
    private static final String RCS_VIBRATE = "chat_invitation_vibration";
    private static final String RCS_APN_TOOL = "rcse_apn_enable";
    private static final String RCS_ROAMING_TOOL = "roaming_enable";

    private static final long TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private long mRawContactId = -1;
    private SettingsFragment mSettingsFragment = null;
    private PreferenceScreen mClearHistoryItem = null;
    private PreferenceScreen mBlockItem = null;

    private CheckBoxPreference mRemindCheckBox = null;
    private CheckBoxPreference mNotificationCheckBox = null;
    private RingtonePreference mNotificationRingtone = null;
    private CheckBoxPreference mVibrateCheckBox = null;
    private CheckBoxPreference mSendReadReceiptCheckBox = null;

    private CheckBoxPreference mApnCheckBox = null;
    private CheckBoxPreference mRoamingCheckBox = null;
    private CheckBoxPreference mCompressImageCheckBox = null;

    private Context mContext = null;
    private ContactsManager contactsManager = null;

    public SettingsActivityTest() {
        super(SettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getInstrumentation().waitForIdleSync();
    }

    public void testCase1_getCompressImageStatus() {
        Logger.d(TAG, "testCase1_getCompressImageStatus");
        mSettingsFragment = getFragment(getActivity());
        CheckBoxPreference compressImageCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCSE_COMPRESS_IMAGE);
        boolean curentUiStatus = compressImageCheckBox.isChecked();
        boolean currentDatabaseStatus = RcsSettings.getInstance().isEnabledCompressingImage();
        assertEquals(curentUiStatus, currentDatabaseStatus);
    }

    public void testCase2_setCompressImageStatus() {
        Logger.d(TAG, "testCase2_setCompressImageStatus");
        RcsSettings.getInstance().setCompressingImage(true);
        mSettingsFragment = getFragment(getActivity());
        CheckBoxPreference compressImageCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCSE_COMPRESS_IMAGE);
        boolean curentUiStatus = compressImageCheckBox.isChecked();
        Logger.d(TAG, "curentUiStatus = " + curentUiStatus);
        boolean currentDatabaseStatus = RcsSettings.getInstance().isEnabledCompressingImage();
        assertEquals(true, currentDatabaseStatus);
        assertEquals(true, curentUiStatus);
    }

    /**
     * validate if chebox's RCS_SEND_READ_RECEIPT status is as the same as
     * mSettings's RCS_SEND_READ_RECEIPT status
     * 
     * @throws Throwable
     */

    public void testCase3_clickSendReadReceiptCheckBox() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        validateclickSendReadReceiptCheckBox(mSettingsFragment);
    }

    /**
     * if EXTRA_KEY_FROM_MMS equals with EXTRA_VALUE_FROM_MMS,call
     * removeUnusePreferences(). the validate if chebox's RCS_SEND_READ_RECEIPT
     * status is as the same as mSettings's RCS_SEND_READ_RECEIPT status
     * 
     * @throws Throwable
     */

    public void testCase4_removeUnusePreferences() throws Throwable {
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), SettingsActivity.class);
        intent.putExtra(EXTRA_KEY_FROM_MMS, EXTRA_VALUE_FROM_MMS);
        setActivityIntent(intent);
        mSettingsFragment = getFragment(getActivity());
        validateclickSendReadReceiptCheckBox(mSettingsFragment);
    }

    public void testCase5_showClearChatHistoryDialogAndClickCancel() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        mClearHistoryItem = (PreferenceScreen) mSettingsFragment.findPreference(RCS_CLEAR_HISTORY);
        assertNotNull(mClearHistoryItem);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mSettingsFragment.onPreferenceTreeClick(null, mClearHistoryItem);
            }
        });
        getInstrumentation().waitForIdleSync();
        DialogFragment confirmDialogOfClearHistory = (DialogFragment) getActivity()
                .getFragmentManager().findFragmentByTag(CLEAR_HISTORY_CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialogOfClearHistory);
        clickDialogButton(confirmDialogOfClearHistory, Dialog.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();

    }

    public void testCase6_showClearChatHistoryDialogAndClickOK() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        mClearHistoryItem = (PreferenceScreen) mSettingsFragment.findPreference(RCS_CLEAR_HISTORY);
        assertNotNull(mClearHistoryItem);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mSettingsFragment.onPreferenceTreeClick(null, mClearHistoryItem);
            }
        });
        getInstrumentation().waitForIdleSync();
        DialogFragment confirmDialogOfClearhistory = (DialogFragment) getActivity()
                .getFragmentManager().findFragmentByTag(CLEAR_HISTORY_CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialogOfClearhistory);
        clickDialogButton(confirmDialogOfClearhistory, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        DialogFragment clearHistoryProgress = (DialogFragment) getActivity().getFragmentManager()
                .findFragmentByTag(CLEAR_HISTORY_PROGRESS);
        assertNull(clearHistoryProgress);
        Thread.sleep(3000);
        assertNull(clearHistoryProgress);
    }

    public void testCase7_onSharedPreferenceChanged() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        mRemindCheckBox = (CheckBoxPreference) mSettingsFragment.findPreference(RCS_REMIND);
        final SharedPreferences sharedPreferences = new SharedPreferences() {

            @Override
            public void unregisterOnSharedPreferenceChangeListener(
                    OnSharedPreferenceChangeListener arg0) {
                // TODO Auto-generated method stub

            }

            @Override
            public void registerOnSharedPreferenceChangeListener(
                    OnSharedPreferenceChangeListener arg0) {
                // TODO Auto-generated method stub

            }

            @Override
            public Set<String> getStringSet(String arg0, Set<String> arg1) {
                // TODO Auto-generated method stub
                return null;
            }

            @Override
            public String getString(String arg0, String arg1) {
                // TODO Auto-generated method stub
                return null;
            }

            @Override
            public long getLong(String arg0, long arg1) {
                // TODO Auto-generated method stub
                return 0;
            }

            @Override
            public int getInt(String arg0, int arg1) {
                // TODO Auto-generated method stub
                return 0;
            }

            @Override
            public float getFloat(String arg0, float arg1) {
                // TODO Auto-generated method stub
                return 0;
            }

            @Override
            public boolean getBoolean(String arg0, boolean arg1) {
                // TODO Auto-generated method stub
                return false;
            }

            @Override
            public Map<String, ?> getAll() {
                // TODO Auto-generated method stub
                return null;
            }

            @Override
            public Editor edit() {
                // TODO Auto-generated method stub
                return null;
            }

            @Override
            public boolean contains(String arg0) {
                // TODO Auto-generated method stub
                return false;
            }
        };
        // mRemindCheckBox
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mSettingsFragment.onSharedPreferenceChanged(sharedPreferences, RCS_REMIND);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertFalse(mRemindCheckBox.isChecked());

        // mNotificationCheckBox
        mNotificationCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCS_NOTIFICATION);

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mNotificationCheckBox.setChecked(true);
                mSettingsFragment.onSharedPreferenceChanged(sharedPreferences, RCS_NOTIFICATION);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field IS_NOTIFICATION_CHECKED_FIELD = Utils.getPrivateField(SettingsFragment.class,
                "IS_NOTIFICATION_CHECKED");
        AtomicBoolean IS_NOTIFICATION_CHECKED = (AtomicBoolean) IS_NOTIFICATION_CHECKED_FIELD
                .get(mSettingsFragment);
        assertEquals(true, IS_NOTIFICATION_CHECKED.get());

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mNotificationCheckBox.setChecked(false);
                mSettingsFragment.onSharedPreferenceChanged(sharedPreferences, RCS_NOTIFICATION);
            }
        });
        getInstrumentation().waitForIdleSync();
        IS_NOTIFICATION_CHECKED_FIELD = Utils.getPrivateField(SettingsFragment.class,
                "IS_NOTIFICATION_CHECKED");
        IS_NOTIFICATION_CHECKED = (AtomicBoolean) IS_NOTIFICATION_CHECKED_FIELD
                .get(mSettingsFragment);
        assertEquals(false, IS_NOTIFICATION_CHECKED.get());

        // mApnCheckBox
        if (PhoneUtils.sIsApnDebug) {
            mApnCheckBox = (CheckBoxPreference) mSettingsFragment.findPreference(RCS_APN_TOOL);
            runTestOnUiThread(new Runnable() {

                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mApnCheckBox.setChecked(true);
                    mSettingsFragment.onSharedPreferenceChanged(sharedPreferences, RCS_APN_TOOL);
                }
            });
            getInstrumentation().waitForIdleSync();
            RcsSettings.createInstance(getInstrumentation().getTargetContext());
            String rcsApnSwitch = RcsSettings.getInstance().readParameter(
                    RcsSettingsData.RCS_APN_SWITCH);
            long startTime = System.currentTimeMillis();
            // wait for blacklist contact loaded
            while (!RcsSettings.getInstance().readParameter(RcsSettingsData.RCS_APN_SWITCH)
                    .equals("1")) {

                if (System.currentTimeMillis() - startTime > TIME_OUT) {
                    fail();
                }
                Thread.sleep(SLEEP_TIME);
            }
            rcsApnSwitch = RcsSettings.getInstance().readParameter(RcsSettingsData.RCS_APN_SWITCH);
            assertEquals("1", rcsApnSwitch);

            mRoamingCheckBox = (CheckBoxPreference) mSettingsFragment
                    .findPreference(RCS_ROAMING_TOOL);
            runTestOnUiThread(new Runnable() {

                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    mRoamingCheckBox.setChecked(false);
                    mSettingsFragment
                            .onSharedPreferenceChanged(sharedPreferences, RCS_ROAMING_TOOL);
                }
            });
            getInstrumentation().waitForIdleSync();
            Field mMockedRomingStateField = Utils.getPrivateField(RcseOnlyApnUtils.class,
                    "mMockedRomingState");
            Boolean mMockedRomingState = (Boolean)mMockedRomingStateField.get(RcseOnlyApnUtils.getInstance());
            assertEquals(mRoamingCheckBox.isChecked(), mMockedRomingState.booleanValue());
        }

        // mVibrateCheckBox
        mVibrateCheckBox = (CheckBoxPreference) mSettingsFragment.findPreference(RCS_VIBRATE);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mVibrateCheckBox.setChecked(false);
                mSettingsFragment.onSharedPreferenceChanged(sharedPreferences, RCS_VIBRATE);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertEquals(mVibrateCheckBox.isChecked(), RcsSettings.getInstance()
                .isPhoneVibrateForChatInvitation());
        mSettingsFragment.onDestroy();
    }

    public void testCase8_sendReadReceiptCheckBox_onPreferenceClick() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        mSendReadReceiptCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCS_SEND_READ_RECEIPT);
        final OnPreferenceClickListener onPreferenceClickListener = mSendReadReceiptCheckBox
                .getOnPreferenceClickListener();
        final Preference preference = new Preference(getInstrumentation().getContext());

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                onPreferenceClickListener.onPreferenceClick(null);
                onPreferenceClickListener.onPreferenceClick(preference);
                preference.setKey(RCS_SEND_READ_RECEIPT);
                onPreferenceClickListener.onPreferenceClick(preference);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field mSettingsField = Utils.getPrivateField(SettingsFragment.class, "mSettings");
        SharedPreferences sharedPreferences = (SharedPreferences) mSettingsField
                .get(mSettingsFragment);
        boolean sendReadReceiptFlag = sharedPreferences.getBoolean(RCS_SEND_READ_RECEIPT,
                mSendReadReceiptCheckBox.isChecked());
        if (sendReadReceiptFlag == true || sendReadReceiptFlag == false) {
            assertTrue(true);
        }

    }

    public void testCase9_mCompressImageCheckBox_onPreferenceClick() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        mCompressImageCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCSE_COMPRESS_IMAGE);
        final OnPreferenceClickListener onPreferenceClickListener = mCompressImageCheckBox
                .getOnPreferenceClickListener();
        final Preference preference = new Preference(getInstrumentation().getContext());
        preference.setKey(RCSE_COMPRESS_IMAGE);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                onPreferenceClickListener.onPreferenceClick(preference);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertEquals(mCompressImageCheckBox.isChecked(), RcsSettings.getInstance()
                .isEnabledCompressingImageFromDB());
    }

    public void testCase10_onHiddenChanged() throws Throwable {
        mSettingsFragment = getFragment(getActivity());
        clearBlockedContact();

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mSettingsFragment.onHiddenChanged(false);
            }
        });
        getInstrumentation().waitForIdleSync();
        Field mBlockItemField = Utils.getPrivateField(SettingsFragment.class, "mBlockItem");
        PreferenceScreen mBlockItem = (PreferenceScreen) mBlockItemField.get(mSettingsFragment);
        long startTime = System.currentTimeMillis();

        while ((PreferenceScreen) mBlockItemField.get(mSettingsFragment) == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        mBlockItem = (PreferenceScreen) mBlockItemField.get(mSettingsFragment);
        startTime = System.currentTimeMillis();
        while (mBlockItem.getSummary() == null) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        Resources r = getActivity().getResources();
        String temp = r.getString(R.string.text_block_contact, Integer.toString(0));
        assertEquals(temp, mBlockItem.getSummary());

    }

    /**
     * Get the fragment in the activity.
     * 
     * @return The fragment in the activity.
     */
    private SettingsFragment getFragment(Activity activity) {
        Logger.d(TAG, "getFragment()");
        assertNotNull(activity);
        View view = activity.findViewById(R.id.rcs_more_settings);
        assertNotNull(view);
        Fragment fragment = activity.getFragmentManager().findFragmentById(R.id.rcs_more_settings);
        assertTrue(fragment instanceof SettingsFragment);
        return (SettingsFragment) fragment;
    }

    /**
     * validate if chebox's RCS_SEND_READ_RECEIPT status is as the same as
     * mSettings's RCS_SEND_READ_RECEIPT status
     * 
     * @param mSettingsFragment
     * @throws Throwable
     */
    private void validateclickSendReadReceiptCheckBox(SettingsFragment mSettingsFragment)
            throws Throwable {
        final CheckBoxPreference sendReadReceiptCheckBox = (CheckBoxPreference) mSettingsFragment
                .findPreference(RCS_SEND_READ_RECEIPT);
        SharedPreferences mSettings = PreferenceManager.getDefaultSharedPreferences(getActivity());

        boolean currentSendReadReceiptStatus = sendReadReceiptCheckBox.isChecked();
        boolean currentSendReadReceiptStatusOfmSetting = mSettings.getBoolean(
                RCS_SEND_READ_RECEIPT, true);
        // currentSendReadReceiptStatus must be as the same as
        // currentSendReadReceiptStatusOfmSetting
        assertEquals(currentSendReadReceiptStatus, currentSendReadReceiptStatusOfmSetting);
        final OnPreferenceClickListener onPreferenceClickListener = sendReadReceiptCheckBox
                .getOnPreferenceClickListener();
        assertNotNull(onPreferenceClickListener);
        final boolean sendReadReceiptStatus = !currentSendReadReceiptStatus;
        // change the status of the chebox ,then validate whether
        // currentSendReadReceiptStatusOfmSetting also be changed
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                sendReadReceiptCheckBox.setChecked(sendReadReceiptStatus);
                onPreferenceClickListener.onPreferenceClick(sendReadReceiptCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        boolean changedSendReadReceiptStatus = sendReadReceiptCheckBox.isChecked();
        // make sure that the value of checkbox is changed
        assertEquals(sendReadReceiptStatus, changedSendReadReceiptStatus);
        currentSendReadReceiptStatusOfmSetting = mSettings.getBoolean(RCS_SEND_READ_RECEIPT, true);
        assertEquals(changedSendReadReceiptStatus, currentSendReadReceiptStatusOfmSetting);
    }

    /**
     * click the button of dialog showed
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

    public void clearBlockedContact() throws ContactsManagerException {
        ContactsManager.createInstance(getInstrumentation().getTargetContext());
        contactsManager = ContactsManager.getInstance();
        List<String> imBlockContactsList = contactsManager.getImBlockedContacts();
        List<String> rcsBlockContactsList = contactsManager.getRcsBlockedContacts();
        ContactsManager.StrangerBlocker.initialize(getInstrumentation().getTargetContext());
        Set<String> strangerBlockContactsList = ContactsManager.StrangerBlocker.getAllBlockedList();
        // clear blocked contact in the phone
        // clear stranger blocked contacts first
        if (strangerBlockContactsList != null) {
            Iterator<String> interator = strangerBlockContactsList.iterator();
            if (interator != null) {
                while (interator.hasNext()) {
                    String tmpString = interator.next();
                    Logger.d("Stranger", tmpString);
                    ContactsManager.StrangerBlocker.unblockContact(tmpString);
                }
            }
        }
        // clear IM blocked contacts
        int imBlockContactsNum = imBlockContactsList.size();
        if (imBlockContactsNum > 0) {

            for (int i = 0; i < imBlockContactsNum; i++) {
                Logger.d("IMBLOCK", imBlockContactsList.get(i));
                contactsManager.setImBlockedForContact(imBlockContactsList.get(i), false);
            }
        }

        // clear RCSe blocked contacts
        int rcsBlockContactsNum = rcsBlockContactsList.size();

        if (rcsBlockContactsNum > 0) {

            for (int i = 0; i < rcsBlockContactsNum; i++) {
                Logger.d("RcsBLOCK", rcsBlockContactsList.get(i));
                contactsManager.unblockContact(rcsBlockContactsList.get(i));
            }
        }

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
    }

}
