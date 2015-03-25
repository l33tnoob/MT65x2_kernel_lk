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

package com.mediatek.rcse.test.plugin;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Telephony.Sms;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.SettingsActivity;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.message.IpMessageActivitiesManager;
import com.mediatek.rcse.plugin.message.IpMessageManager;
import com.mediatek.rcse.plugin.message.IpMessagePluginExt;
import com.mediatek.rcse.plugin.message.IpMessageServiceMananger;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.IRegistrationStatus;
import com.mediatek.rcse.service.PluginApiManager;

import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;

/**
 * The class is used to test IpMessageActivityManager
 */
public class ActivitesManagerTest extends InstrumentationTestCase {
    private static final String TAG = "ActivitesManagerTest";
    private Context mContext = null;
    private static final long TIME_OUT = 2000;
    private static final long SLEEP_TIME = 200;
    private static final String REGISTRATION_STATUS = "mRegistrationStatus";
    private static final String GROUP_CHAT_BODY = "250 invites you and 251 into a group chat";
    private static final String GROUP_CHAT_CONTACT = "7---11111111-3a00-485f-8205-1111111111111";
    private static final int GROUP_CHAT_MSGID = 10;
    private static final String[] MOCK_CONTACT = {
        "+34200000999"
    };
    private IpMessageActivitiesManager mActivityManager = null;
    private static final int MOCK_RCSE_ID = 24534214;
    private static final long MOCK_MSG_ID= 234324L;
    private static final String MOCK_FILEPATH_NEW = "/mock/filepath/new";
    private static final int REQUEST_CODE_ISMS_RECORD_AUDIO      = 206;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        if (!ApiManager.initialize(mContext)) {
            Logger.d(TAG, "initialize failed!");
            fail();
        }
        IpMessagePluginExt ipMessagePluginExt = new IpMessagePluginExt(mContext);
        waitForServiceConnected();
        RegistrationApi registrantionApi = ApiManager.getInstance().getRegistrationApi();
        Field registrationStatusField =
                Utils.getPrivateField(RegistrationApi.class, REGISTRATION_STATUS);
        registrationStatusField.set(registrantionApi, new RegistrationStatus());
        mActivityManager = new IpMessageActivitiesManager(mContext);
    }

    public void testCase1_StartRemoteActivity() throws IllegalAccessException, NoSuchFieldException,
            InterruptedException {
        Logger.d(TAG, "testCase1_StartRemoteActivity() entry");
        if (!ContactsListManager.getInstance().IS_SUPPORT) {
            Logger.d(TAG, "testCase1_StartRemoteActivity() selectActivity");
            ActivityMonitor selectActivityMonitor = getInstrumentation().addMonitor(
                    SelectContactsActivity.class.getName(), null, false);
            Intent intent = new Intent(RemoteActivities.CONTACT);
            intent.putExtra(IpMessageConsts.RemoteActivities.KEY_TYPE,
                    IpMessageConsts.SelectContactType.IP_MESSAGE_USER);
            intent.putExtra(RemoteActivities.KEY_ARRAY, MOCK_CONTACT);
            mActivityManager.startRemoteActivity(mContext, intent);
            Activity selectContactsActity = getInstrumentation().waitForMonitorWithTimeout(selectActivityMonitor,
                    TIME_OUT);
            assertNotNull(selectContactsActity);
            selectContactsActity.finish();
        }
        Logger.d(TAG, "testCase1_StartRemoteActivity() exit");
    }

    public void testCase2_StartGroupChat() throws IllegalAccessException, NoSuchFieldException, InterruptedException,
            NoSuchMethodException, InvocationTargetException {
        Logger.d(TAG, "testCase2_StartGroupChat() entry");
        Field field = Utils.getPrivateField(ApiManager.class, "mMessagingApi");
        field.set(ApiManager.getInstance(), new MockMessagingApi(mContext));

        Logger.d(TAG, "testCase2_StartGroupChat() start InvitationDialog");
        ActivityMonitor InvitationDialogMonitor = getInstrumentation().addMonitor(InvitationDialog.class.getName(),
                null, false);

        Long messageId = PluginUtils.insertDatabase(GROUP_CHAT_BODY, GROUP_CHAT_CONTACT, GROUP_CHAT_MSGID,
                PluginUtils.INBOX_MESSAGE);
        long threadId = getThreadId(messageId);
        Logger.d(TAG, "testCase2_StartGroupChat(), threadId = " + threadId + ", messageId = " + messageId);

        Intent intent = new Intent(RemoteActivities.CHAT_DETAILS_BY_THREAD_ID);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, threadId);

        mActivityManager.startRemoteActivity(mContext, intent);
        Activity invitationDialog = getInstrumentation().waitForMonitorWithTimeout(InvitationDialogMonitor, TIME_OUT);
        assertNotNull(invitationDialog);
        invitationDialog.finish();

        PluginGroupChatWindow.removeGroupChatInvitationInMms(GROUP_CHAT_CONTACT);
        Logger.d(TAG, "testCase2_StartGroupChat() exit");
    }
    
    public void testCase3_StartSettingsActivity() {
        Logger.d(TAG, "testCase3_StartSettingsActivity() entry");
        long messageIdInMms = MOCK_MSG_ID;
        Intent intentMedia = new Intent(RemoteActivities.MEDIA_DETAIL);
        intentMedia.putExtra(RemoteActivities.KEY_MESSAGE_ID, 0);
        mActivityManager.startRemoteActivity(mContext, intentMedia);
        ActivityMonitor SettingsActivityMonitor =
                getInstrumentation().addMonitor(SettingsActivity.class.getName(), null, false);
        Intent intent = new Intent(RemoteActivities.SYSTEM_SETTINGS);
        mActivityManager.startRemoteActivity(mContext, intent);
        Activity settingsActivity =
                getInstrumentation().waitForMonitorWithTimeout(SettingsActivityMonitor, TIME_OUT);
        assertNotNull(settingsActivity);
        settingsActivity.finish();
        Logger.d(TAG, "testCase3_StartSettingsActivity() exit");
    }

    private void waitForServiceConnected() throws InterruptedException {
        RegistrationApi registrantionApi = ApiManager.getInstance().getRegistrationApi();
        long startTime = System.currentTimeMillis();
        while (registrantionApi == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            registrantionApi = ApiManager.getInstance().getRegistrationApi();
        }
    }
    
    private long getThreadId(Long messageId) {
        ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();
        if (contentResolver != null && messageId != -1) {
            Uri uri = Uri.parse(Utils.SMS_CONTENT_URI + "/" + messageId);
            Cursor cursor = null;
            try {
                final String[] proj = {
                    Sms.THREAD_ID
                };
                cursor = contentResolver.query(uri, proj, null, null, null);
                if (cursor.moveToFirst()) {
                    return cursor.getLong(cursor.getColumnIndex(Sms.THREAD_ID));
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        }
        return -1;
    }

    private class RegistrationStatus extends IRegistrationStatus.Stub {

        @Override
        public void addRegistrationStatusListener(IRegistrationStatusRemoteListener listener)
                throws RemoteException {
        }

        @Override
        public boolean isRegistered() throws RemoteException {
            return true;
        }
    }

    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            return new MockChatSession();
        }
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
            return new InstantMessage("", "", "", false);
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
            return null;
        }

        @Override
        public String getSessionID() throws RemoteException {
            return null;
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
        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId, String status)
                throws RemoteException {
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