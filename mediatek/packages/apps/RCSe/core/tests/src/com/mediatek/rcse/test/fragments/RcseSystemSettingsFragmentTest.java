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
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Looper;
import android.preference.CheckBoxPreference;
import android.test.ActivityInstrumentationTestCase2;

import com.mediatek.rcse.activities.RcseSystemSettingsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.RcseSystemSettingsFragment;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

import java.lang.reflect.Field;

/**
 * Test case class for SettingsActivity
 */
public class RcseSystemSettingsFragmentTest extends
        ActivityInstrumentationTestCase2<RcseSystemSettingsActivity> {
    private static final String TAG = "RcseSystemSettingsFragmentTest";
    private static final String RCS_ACTIVITATION = "rcs_activation";
    private static final String RCS_ROMING = "rcs_roaming";
    private static final String CONFIRM_DIALOG_TAG = "ConfirmDialog";
    private static final String MESSAGE_RCS_ID = "messageResId";
    private static final int RCSE_SYSTEM_SETTING_FRAGMENT = R.id.rcse_system_setting_fragment;
    private static final long TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private RcseSystemSettingsFragment mRcseSystemSettingsFragment = null;
    Activity mActivity = null;

    public RcseSystemSettingsFragmentTest() {
        super(RcseSystemSettingsActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Logger.v(TAG, "setUp() entry");
        super.setUp();
        // launch RcseSystemSettingsActivity
        mActivity = getActivity();
        assertNotNull(mActivity);
        mRcseSystemSettingsFragment = (RcseSystemSettingsFragment) getRcseSystemSettingsFragment(RCSE_SYSTEM_SETTING_FRAGMENT);
        Logger.v(TAG, "setUp() exit");
    }

    /**
     * Get the fragment in the activity.
     * 
     * @return The fragment in the activity.
     */
    private Fragment getRcseSystemSettingsFragment(int fragmentId) {
        Logger.d(TAG, "getRcseSystemSettingsFragment()");
        Fragment fragment = mActivity.getFragmentManager().findFragmentById(fragmentId);
        assertTrue(fragment instanceof RcseSystemSettingsFragment);
        return fragment;
    }

    /**
     * checkbox of service is unchecked at current,so we check it, and click
     * cancel at the dialog which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase1_clickNotActiveCheckboxShowDialogAndClickCancel() throws Throwable {
        final CheckBoxPreference rcsActivitationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                rcsActivitationCheckBox.setChecked(true);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });

        getInstrumentation().waitForIdleSync();
        // assertEquals(true, rcsActivitationCheckBox.isChecked());
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_enable_rcse_service, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();
        assertEquals(false, rcsActivitationCheckBox.isChecked());
    }

    /**
     * checkbox of service is unchecked at current,so we check it, and click ok
     * at the dialog which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase2_clickNotActiveCheckboxShowDialogAndClickOK() throws Throwable {
        final CheckBoxPreference rcsActivitationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                rcsActivitationCheckBox.setChecked(true);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_enable_rcse_service, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        waitForCheckBoxIsEnable(rcsActivitationCheckBox);
        assertTrue(rcsActivitationCheckBox.isEnabled());
        assertTrue(rcsActivitationCheckBox.isChecked());
    }

    /**
     * checkbox of service is checked at current,so we check it, and click
     * cancel at the dialog which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase3_clickActiveCheckboxShowDialogAndClickCancel() throws Throwable {
        final CheckBoxPreference rcsActivitationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                rcsActivitationCheckBox.setChecked(false);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_disable_rcse_service, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();
        assertEquals(true, rcsActivitationCheckBox.isChecked());
    }

    /**
     * checkbox of service is checked at current,so we check it, and click ok at
     * the dialog which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase4_clickActiveCheckboxShowDialogAndClickOK() throws Throwable {
        final CheckBoxPreference rcsActivitationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                rcsActivitationCheckBox.setChecked(false);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_disable_rcse_service, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        waitForCheckBoxIsEnable(rcsActivitationCheckBox);
        assertTrue(rcsActivitationCheckBox.isEnabled());
        assertEquals(false, rcsActivitationCheckBox.isChecked());
    }

    /**
     * checkbox of roaming is enable after service is started,checkbox is
     * unchecked at current, so we check it, and click cancel at the dialog
     * which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase5_clickUnactiveRoamingCheckboxShowDialogAndClickCancel() throws Throwable {
        // afer service is started,the checkbox is enable,so we mock it
        final CheckBoxPreference roamingCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ROMING);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                roamingCheckBox.setChecked(true);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, roamingCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_enable_rcse_service_roaming, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();
        long startTime = System.currentTimeMillis();
        assertEquals(false, roamingCheckBox.isChecked());
    }

    /**
     * checkbox of roaming is enable after service is started,checkbox is
     * unchecked at current, so we check it, and click cancel at the dialog
     * which is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase6_clickUnactiveRoamingCheckboxShowDialogAndClickOK() throws Throwable {
        // afer service is started,the checkbox is enable,so we mock it
        final CheckBoxPreference roamingCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ROMING);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                roamingCheckBox.setChecked(true);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, roamingCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_enable_rcse_service_roaming, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        long startTime = System.currentTimeMillis();
        // wait for blacklist contact loaded
        while (roamingCheckBox.isChecked() == false) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        assertEquals(true, roamingCheckBox.isChecked());
    }

    /**
     * checkbox of roaming is enable after service is started,checkbox is
     * checked at current, so we check it, and click cancel at the dialog which
     * is showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase7_clickActiveRoamingCheckboxShowDialogAndClickCancel() throws Throwable {
        // afer service is started,the checkbox is enable,so we mock it
        final CheckBoxPreference roamingCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ROMING);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                roamingCheckBox.setChecked(false);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, roamingCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_disable_rcse_service_roaming, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_NEGATIVE);
        getInstrumentation().waitForIdleSync();
        assertEquals(true, roamingCheckBox.isChecked());
    }

    /**
     * checkbox of roaming is enable after service is started,checkbox is
     * checked at current, so we check it, and click OK at the dialog which is
     * showed after check the checkbox
     * 
     * @throws Throwable
     */
    public void testCase8_clickActiveRoamingCheckboxShowDialogAndClickOK() throws Throwable {
        // afer service is started,the checkbox is enable,so we mock it
        final CheckBoxPreference roamingCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ROMING);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                roamingCheckBox.setChecked(false);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, roamingCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        // show dialog is the synchronous action,so it can be got directory
        // ,no need to wait for dialog
        DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        int messageRcsId = confirmDialog.getArguments().getInt(MESSAGE_RCS_ID);
        assertEquals(R.string.rcse_settings_disable_rcse_service_roaming, messageRcsId);
        clickDialogButton(confirmDialog, Dialog.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        long startTime = System.currentTimeMillis();
        // wait for blacklist contact loaded
        while (roamingCheckBox.isChecked() == true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        assertEquals(false, roamingCheckBox.isChecked());
    }

    public void testCase9_onPreferenceTreeClick() throws Throwable {
        // afer service is started,the checkbox is enable,so we mock it
        final CheckBoxPreference activationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        final CheckBoxPreference roamingCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
        .findPreference(RCS_ROMING);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, activationCheckBox);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, roamingCheckBox);
            }
        });
    }

    public void testCase10_onCancel() throws Throwable {
        final CheckBoxPreference rcsActivitationCheckBox = (CheckBoxPreference) mRcseSystemSettingsFragment
                .findPreference(RCS_ACTIVITATION);
        // if check box is not checked,select the check
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                rcsActivitationCheckBox.setChecked(false);
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        getInstrumentation().waitForIdleSync();
        final DialogFragment confirmDialog = (DialogFragment) mActivity.getFragmentManager()
                .findFragmentByTag(CONFIRM_DIALOG_TAG);
        assertNotNull(confirmDialog);
        Field field = Utils.getPrivateField(confirmDialog.getClass(), "mMessageResId");
        field.set(confirmDialog, R.string.rcse_settings_enable_rcse_service);
        Field fiedlEnable = Utils.getPrivateField(mRcseSystemSettingsFragment.getClass(),
                "mRcseServiceActivation");
        CheckBoxPreference preferenceEnable = (CheckBoxPreference) fiedlEnable
                .get(mRcseSystemSettingsFragment);
        boolean stateBefore1 = preferenceEnable.isChecked();
        Field fiedlRoming = Utils.getPrivateField(mRcseSystemSettingsFragment.getClass(),
                "mRcseRoming");
        CheckBoxPreference preferenceRoming = (CheckBoxPreference) fiedlRoming
                .get(mRcseSystemSettingsFragment);
        boolean stateBefore2 = preferenceRoming.isChecked();
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                confirmDialog.onCancel(confirmDialog.getDialog());
            }
        });
        assertNotSame(stateBefore1, preferenceEnable.isChecked());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        stateBefore1 = preferenceEnable.isChecked();
        field.set(confirmDialog, R.string.rcse_settings_disable_rcse_service);
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                confirmDialog.onCancel(confirmDialog.getDialog());
            }
        });
        assertNotSame(stateBefore1, preferenceEnable.isChecked());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        field.set(confirmDialog, R.string.rcse_settings_enable_rcse_service_roaming);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                confirmDialog.onCancel(confirmDialog.getDialog());
            }
        });
        assertNotSame(stateBefore2, preferenceRoming.isChecked());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRcseSystemSettingsFragment.onPreferenceTreeClick(null, rcsActivitationCheckBox);
            }
        });
        field.set(confirmDialog, R.string.rcse_settings_disable_rcse_service_roaming);
        stateBefore2 = preferenceRoming.isChecked();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                confirmDialog.onCancel(confirmDialog.getDialog());
            }
        });
        assertNotSame(stateBefore2, preferenceRoming.isChecked());
    }

    /**
     * wait for checkbox is enable
     * 
     * @param preference
     * @throws InterruptedException
     */
    public void waitForCheckBoxIsEnable(CheckBoxPreference preference) throws InterruptedException {
        Logger.d(TAG, "waitForCheckBoxIsEnable() start");
        long startTime = System.currentTimeMillis();

        while (!preference.isEnabled()) {
            Logger.d(TAG, "waitForCheckBoxIsEnable() exit success preference.isEnabled() start "
                    + preference.isEnabled());
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForCheckBoxIsEnable() timeout");
            }
            Thread.sleep(SLEEP_TIME);
            Logger.d(TAG, "waitForCheckBoxIsEnable() exit success preference.isEnabled() "
                    + preference.isEnabled());
        }

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
                dialogFragment.onClick(null, dialogButtonIndicator);
            }
        });
    }


    @Override
    protected void tearDown() throws Exception {
        RcsSettings.createInstance(getInstrumentation().getTargetContext());
        RcsSettings rcsSettings = RcsSettings.getInstance();
        rcsSettings.writeParameter(RcsSettingsData.SERVICE_ACTIVATED, "true");
        long startTime = System.currentTimeMillis();
        // wait for blacklist contact loaded
        while (rcsSettings.readParameter(RcsSettingsData.SERVICE_ACTIVATED).equals("false")) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

}
