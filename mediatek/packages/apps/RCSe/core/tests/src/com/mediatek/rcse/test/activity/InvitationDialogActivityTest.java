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
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.Instrumentation.ActivityMonitor;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;
import com.orangelabs.rcs.R;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;


/**
 * Tests for launch invitation dialog via InvitationDialog.
 */
public class InvitationDialogActivityTest extends
        ActivityInstrumentationTestCase2<InvitationDialog> {

    private static final String TAG = "InvitationDialogActivityTest";
    public static final int LAUNCH_TIME_OUT = 1000;
    static final double SELECT_CONTACTS_PATIAL = 0.5;
    static final int MIN_NUMBER = 1;
    static final String FILE_SIZE = "123456";
    static final String FILE_NAME = "abcd.jpg";
    static final String MOKE_CONTACT = "+34200000250";
    private static final String MOCK_SESSION_ID = "mock_session_id";
    private static final long TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;
    private Context mContext = null;
    private static final String INVITATION_SESSION_ID = "sessionId";
    private MockChatSession mMockChatSession = null;
    private MockMessagingApi mMockMessagingApi = null;

    public InvitationDialogActivityTest() {
        super(InvitationDialog.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
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
     * Use to test group invitation dialog display
     */
    public void testCase1_CreateGroupInvitationDialog() throws Throwable {
        Logger.v(TAG, "testCase1_CreateGroupInvitationDialog() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        launchActivity(intent);
    }

    /**
     * Use to test file transfer invitation dialog display
     */
    public void testCase2_CreateFileTransferInvitationDialog() throws Throwable {
        Logger.v(TAG, "testCase2_CreateFileTransferInvitationDialog() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        Activity activity = launchActivity(intent);
        getInstrumentation().waitForIdleSync();
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(
                ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "FileTransferDialog", DialogInterface.BUTTON_NEGATIVE);
        Activity chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNull(chatScreenActivity);
        } finally {
            getInstrumentation().removeMonitor(chatScreenActivityMonitor);
            if (null != chatScreenActivity) {
                chatScreenActivity.finish();
            }
        }
    }

    /**
     * Use to test file transfer warming dialog display
     */
    public void testCase3_CreateFileTransferWarmingDialog() throws Throwable {
        Logger.v(TAG, "testCase3_CreateFileTransferWarmingDialog() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        launchActivity(intent);
    }

    /**
     * Use to test ipmsg group invitation dialog display
     */
    public void testCase4_CreateIpmsgGroupInvitationDialog() throws Throwable {
        Logger.v(TAG, "testCase4_CreateIpmsgGroupInvitationDialog() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        launchActivity(intent);
    }
    
    /**
     * Use to test accept a group invitation
     */
    public void testCase5_GroupInvitationDialogAccept() throws Throwable {
        Logger.v(TAG, "testCase5_GroupInvitationDialogAccept() entry");
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatScreenActivity);
        } finally {
            if (chatScreenActivity != null) {
                chatScreenActivity.finish();
            }
        }
        Logger.v(TAG, "testCase5_GroupInvitationDialogAccept() exit");
    }
    
    /**
     * Use to test decline a group invitation
     */
    public void testCase6_GroupInvitationDialogDecline() throws Throwable {
        Logger.v(TAG, "testCase6_GroupInvitationDialogDecline() entry");
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        mMockChatSession.sIsRejectSessionCalled = false;
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_NEGATIVE);
        long startTime = System.currentTimeMillis();
        while (true != mMockChatSession.sIsRejectSessionCalled) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase6_GroupInvitationDialogDecline() timeout");
            }
        }
        Logger.v(TAG, "testCase6_GroupInvitationDialogDecline() exit");
    }
    
    /**
     * Use to test accept a plugin group invitation
     */
    public void testCase7_IpMesPluginGroupInvitationDialogAccept() throws Throwable {
        Logger.v(TAG, "testCase7_IpMesPluginGroupInvitationDialogAccept() entry");
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        ActivityMonitor chatActivityMonitor = getInstrumentation().addMonitor(PluginGroupChatActivity.class.getName(), null, false);
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatActivity = getInstrumentation().waitForMonitorWithTimeout(chatActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatActivity);
        } finally {
            if (chatActivity != null) {
                chatActivity.finish();
            }
        }
        Logger.v(TAG, "testCase7_IpMesPluginGroupInvitationDialogAccept() exit");
    }
    
    /**
     * Use to test decline a plugin group invitation
     */
    public void testCase8_IpMesPluginGroupInvitationDialogDecline() throws Throwable {
        Logger.v(TAG, "testCase8_IpMesPluginGroupInvitationDialogDecline() entry");
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        mMockChatSession.sIsRejectSessionCalled = false;
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_NEGATIVE);
        long startTime = System.currentTimeMillis();
        while (true != mMockChatSession.sIsRejectSessionCalled) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase8_IpMesPluginGroupInvitationDialogDecline() timeout");
            }
        }
        Logger.v(TAG, "testCase8_IpMesPluginGroupInvitationDialogDecline() exit");
    }
    
    /**
     * Use to test accept file size warning
     */
    public void testCase9_FileSizeWarningStrategy() throws Throwable {
        Logger.d(TAG, "testCase9_FileSizeWarningStrategy() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        Activity activity = launchActivity(intent);
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(
                ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "FileSizeWarningDialog", DialogInterface.BUTTON_POSITIVE);
        assertTrue(activity.isFinishing());
        Activity chatActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatActivity);
        } finally {
            if (chatActivity != null) {
                chatActivity.finish();
            }
        }
        Logger.d(TAG, "testCase9_FileSizeWarningStrategy() exit");
    }
    
    /**
     * Use to test file transfer invitation strategy accept
     */
    public void testCase10_FileTransferInvitationStrategyAccept() throws Throwable {
        Logger.d(TAG, "testCase10_FileTransferInvitationStrategyAccept() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        Activity activity = launchActivity(intent);
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(
                ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "FileTransferDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatScreenActivity);
        } finally {
            getInstrumentation().removeMonitor(chatScreenActivityMonitor);
            if (null != chatScreenActivity) {
                chatScreenActivity.finish();
            }
        }
        Logger.d(TAG, "testCase10_FileTransferInvitationStrategyAccept() exit");
    }
    
    /**
     * Use to test file transfer invitation strategy decline
     */
    public void testCase11_FileTransferInvitationStrategyDecline() throws Throwable {
        Logger.d(TAG, "testCase11_FileTransferInvitationStrategyDecline() entry");
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        clickDialogButton(activity, "FileTransferDialog", DialogInterface.BUTTON_NEGATIVE);
        assertTrue(activity.isFinishing());
        Logger.d(TAG, "testCase11_FileTransferInvitationStrategyDecline() exit");
    }

    /**
     * Use to test file transfer invitation strategy accept when the file size bigger than warning size
     */
    public void testCase12_FileTransferInvitationStrategyWarning() throws Throwable {
        ApiManager.initialize(mContext);
        final long warningSize = ApiManager.getInstance().getWarningSizeforFileThransfer();
        Utils.getPrivateField(ApiManager.class, "sWarningFileSize").set(ApiManager.getInstance(), 123);
        SharedPreferences sPrefer = PreferenceManager.getDefaultSharedPreferences(mContext);
        Boolean isRemind = sPrefer.getBoolean(SettingsFragment.RCS_REMIND, false);
        Editor editor = sPrefer.edit();
        if (!isRemind) {
            editor.putBoolean(SettingsFragment.RCS_REMIND, true);
            editor.commit();
        }
        View view = LayoutInflater.from(mContext).inflate(R.layout.ft_invitation_content, null);
        TextView warningMessage = (TextView) view.findViewById(R.id.warning_message);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, "test.amr");
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(RcsNotification.NOTIFY_TITLE," Transfer");
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        Activity activity = launchActivity(intent);
        getInstrumentation().waitForIdleSync();
        assertEquals(View.VISIBLE, warningMessage.getVisibility());
        String title = activity.getFragmentManager().findFragmentByTag("FileTransferDialog").getArguments().getString("title");
        assertEquals("Audio Transfer", title);
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(
                ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "FileTransferDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatScreenActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatScreenActivity);
        } finally {
            getInstrumentation().removeMonitor(chatScreenActivityMonitor);
            if (null != chatScreenActivity) {
                chatScreenActivity.finish();
            }
        }
        // Restore the value of 'SettingsFragment.RCS_REMIND'
        editor.putBoolean(SettingsFragment.RCS_REMIND, isRemind);
        editor.commit();
        Utils.getPrivateField(ApiManager.class, "sWarningFileSize").set(ApiManager.getInstance(), warningSize);
    }
    
    /**
     * Use to test a group invitation time out
     */
    public void testCase13_GroupInvitationTimeOutAccept() throws Throwable {
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        String mockNullId = null;
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, mockNullId);
        Activity activity = launchActivity(intent);
        getInstrumentation().waitForIdleSync();
        clickDialogButton(activity, "TimeoutDialog", DialogInterface.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        assertEquals(true, activity.isFinishing());
    }
    
    /**
     * Use to test accept a plugin group invitation with action in IntegrationMode
     */
    public void testCase14_IpMesPluginGroupInvitationDialogAcceptIntegrationModeWithAction() throws Throwable {
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        
        Intent intentWithAction = new Intent();
        intentWithAction.setAction("com.mediatek.rcse.action.INVITE_DIALOG");
        intentWithAction.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intentWithAction.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intentWithAction.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intentWithAction.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intentWithAction);
        ActivityMonitor chatActivityMonitor = getInstrumentation().addMonitor(PluginGroupChatActivity.class.getName(), null, false);
        final boolean mode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatActivity = getInstrumentation().waitForMonitorWithTimeout(chatActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatActivity);
            assertEquals(MessagingApiIntents.CHAT_INVITATION, chatActivity.getIntent().getAction());
        } finally {
            if (chatActivity != null) {
                chatActivity.finish();
            }
        }
        getInstrumentation().removeMonitor(chatActivityMonitor);
        Logger.setIsIntegrationMode(mode);
    }
    
    /**
     * Use to test accept a plugin group invitation without action in IntegrationMode
     */
    public void testCase15_IpMesPluginGroupInvitationDialogAcceptIntegrationModeWithoutAction() throws Throwable {
        mMockChatSession = new MockChatSession();
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        
        Intent intentWithoutAction = new Intent();
        intentWithoutAction.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intentWithoutAction.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intentWithoutAction.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intentWithoutAction.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intentWithoutAction);
        ActivityMonitor chatActivityMonitor = getInstrumentation().addMonitor(PluginGroupChatActivity.class.getName(), null, false);
        final boolean mode = Logger.getIsIntegrationMode();
        Logger.setIsIntegrationMode(true);
        clickDialogButton(activity, "GroupInvitationDialog", DialogInterface.BUTTON_POSITIVE);
        Activity chatActivity = getInstrumentation().waitForMonitorWithTimeout(chatActivityMonitor, TIME_OUT);
        try {
            assertNotNull(chatActivity);
            assertNull(chatActivity.getIntent().getAction());
        } finally {
            if (chatActivity != null) {
                chatActivity.finish();
            }
        }
        getInstrumentation().removeMonitor(chatActivityMonitor);
        Logger.setIsIntegrationMode(mode);
    }
    
    /**
     * Use to test reject file size warning
     */
    public void testCase16_FileSizeWarningStrategyReject() throws Throwable {
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_FILE_TRANSFER_SIZE_WARNING);
        intent.putExtra(RcsNotification.NOTIFY_SIZE, FILE_SIZE);
        intent.putExtra(RcsNotification.NOTIFY_FILE_NAME, FILE_NAME);
        intent.putExtra(RcsNotification.CONTACT, MOKE_CONTACT);
        intent.putExtra(InvitationDialog.KEY_IS_FROM_CHAT_SCREEN, false);
        Activity activity = launchActivity(intent);
        ActivityMonitor chatScreenActivityMonitor = getInstrumentation().addMonitor(
                ChatScreenActivity.class.getName(), null, false);
        clickDialogButton(activity, "FileSizeWarningDialog", DialogInterface.BUTTON_NEGATIVE);
        assertTrue(activity.isFinishing());
        Activity chatActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatScreenActivityMonitor, TIME_OUT);
        try {
            assertNull(chatActivity);
        } finally {
            if (chatActivity != null) {
                chatActivity.finish();
            }
        }
    }
    
    /**
     * Use to test a group invitation time out With null ApiManager instance
     */
    public void testCase17_LaunchGroupInvitationWithNullApiManagerInstance() throws Throwable {
        mMockChatSession = null;
        mMockMessagingApi = new MockMessagingApi(mContext);
        ApiManager.initialize(mContext);
        Utils.getPrivateField(ApiManager.class, "mMessagingApi")
                .set(ApiManager.getInstance(), mMockMessagingApi);
        Intent intent = new Intent();
        intent.setClass(getInstrumentation().getTargetContext(), InvitationDialog.class);
        intent.putExtra(InvitationDialog.KEY_STRATEGY, InvitationDialog.STRATEGY_GROUP_INVITATION);
        intent.putExtra(RcsNotification.NOTIFY_CONTENT, TAG);
        intent.putExtra(INVITATION_SESSION_ID, MOCK_SESSION_ID);
        Activity activity = launchActivity(intent);
        getInstrumentation().waitForIdleSync();
        clickDialogButton(activity, "TimeoutDialog", DialogInterface.BUTTON_POSITIVE);
        getInstrumentation().waitForIdleSync();
        assertEquals(true, activity.isFinishing());
    }
    
    private Activity launchActivity(Intent intent) throws InterruptedException {
        setActivityIntent(intent);
        // Test launch
        Activity activity = getActivity();
        assertNotNull(activity);
        return activity;
    }

    private void clickDialogButton(Activity activity, String fragmentTag, final int which) {
        getInstrumentation().waitForIdleSync();
        final DialogInterface.OnClickListener fragment = (DialogInterface.OnClickListener) activity
        .getFragmentManager().findFragmentByTag(fragmentTag);
        assertNotNull(fragment);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                fragment.onClick(null, which);
            }
        });
        getInstrumentation().waitForIdleSync();
    }

    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            return mMockChatSession;
        }
    }
    
    /**
     * Mock chat session that has a mock session-id and a mock participant list
     */
    private static class MockChatSession implements IChatSession {
        public static boolean sIsRejectSessionCalled = false;
        
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
            List<String> inivtedParticipants = new ArrayList<String>();
            inivtedParticipants.add(MOKE_CONTACT);
            return inivtedParticipants;
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
            sIsRejectSessionCalled = true;
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
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}
    }
}
