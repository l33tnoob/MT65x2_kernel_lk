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

package com.mediatek.rcse.test.service;

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.RcsNotification.FileInvitationInfo;
import com.mediatek.rcse.service.RcsNotification.GroupInvitationInfo;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.GeolocPush;
import com.orangelabs.rcs.service.api.client.messaging.IChatEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferEventListener;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApiIntents;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * This class is used to test RcsNotificationTest.java
 */
public class RcsNotificationTest extends InstrumentationTestCase {
    private final static String TAG = "RcsNotificationTest";
    private static final String API_MANAGER_INITIALIZE = "initialize";
    private static final String API_MANAGER_INSTANCE = "sInstance";
    private static final String API_MANAGER_MESSAGING_API = "mMessagingApi";
    private static final String RCSNOTIFICATION_GROUP_INVITATIONS = "mGroupInvitationInfos";
    private static final String RCSNOTIFICATION_FT_INVITATIONS = "mFileInvitationInfos";
    private static final String FIRST_MESSAGE = "firstMessage";
    private final static String MOCK_SESSION_ID = "mock_session_id";
    private final static String MOCK_SESSION_ID2 = "mock_session_id2";
    private final static String MOCK_CALLER = "+34200000250";
    private static final String SESSION_ID = "sessionId";
    private final static String MOCK_PARTICIPANT_ONE = "+34200000246";
    private final static String MOCK_PARTICIPANT_TWO = "+34200000247";
    private final static String MOCK_PARTICIPANT_THREE = "+34200000248";
    private final static long MOCK_FILE_SIZE = 2;
    private final static long MOCK_CANCEL_FILE_SIZE = 1;
    private final static long MOCK_MAX_FILE_SIZE_SMALLER = Long.MAX_VALUE - 3;
    private final static long MOCK_MAX_FILE_SIZE_LAGGER = Long.MAX_VALUE - 1;
    private static final long TIME_OUT = 2000;
    private boolean mSessionBeRejected = false;
    private Context mContext = null;
    private final static String MOCK_FILE_NAME = "RCSe.apk";
    private final static int FT_INVITE_SIZE_ZERO = 0;
    private final static int FT_INVITE_SIZE_ONE = 1;
    private final static int FT__SIZE_ONE = 1;
    private static final String RCSNOTIFICATION_FILE_SIZE = "mFileSize";
    private final static InstantMessage MOCK_FIRST_MESSAGE = new InstantMessage("", MOCK_CALLER,
            "Test Message", false, new Date());
    private static List<String> MOCK_PARTICIPANTS = new ArrayList<String>();
    private final static RcsNotification NOTIFICATION_INSTANCE = RcsNotification.getInstance();
    private static final String HANDLE_GROUP_INVITATION = "handleGroupChatInvitation";
    private static final String FILE_SIZE = "filesize";
    private static final long DEFAULT_FILE_SIZE = 1l;
    private static final String CHAT_MAP = "mChatMap";
    private static boolean sIsGroupChat = true;

    static {
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_ONE);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_TWO);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_THREE);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mFileInvitationInfos");
        List<FileInvitationInfo> infos = (List<FileInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
        Field fieldGroupInfos = Utils.getPrivateField(RcsNotification.class,
                "mGroupInvitationInfos");
        ConcurrentHashMap<String, GroupInvitationInfo> groupInfos = (ConcurrentHashMap<String, GroupInvitationInfo>) fieldGroupInfos
                .get(NOTIFICATION_INSTANCE);
        groupInfos.clear();
        Field field = Utils.getPrivateField(SettingsFragment.class, "IS_NOTIFICATION_CHECKED");
        field.set(null, new AtomicBoolean(true));
        Logger.d(TAG, "setUp() entry");
        mContext = getInstrumentation().getTargetContext();
        RcsSettings.createInstance(mContext);
        Constructor<ApiManager> constructor = Utils.getPrivateConstructor(ApiManager.class,
                Context.class);
        ApiManager apiManager = constructor.newInstance(mContext);
        Field fieldInstance = Utils.getPrivateField(ApiManager.class, "sInstance");
        fieldInstance.set(null, apiManager);
        assertNotNull("setUp apiManager is null", ApiManager.getInstance());
        ContactsListManager.initialize(mContext);
        Field messagingApiField = ApiManager.class.getDeclaredField(API_MANAGER_MESSAGING_API);
        messagingApiField.setAccessible(true);
        messagingApiField.set(apiManager, new MockMessagingApi(getInstrumentation()
                .getTargetContext()));
    }

    /**
     * Test the behavior of RcsNotification after accepting or rejecting a group
     * chat invitation
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase1_HandleGroupChatInvitation() throws InterruptedException,
            NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            NoSuchMethodException, InvocationTargetException {
        checkGroupInvitationSize(0);
        Method method = Utils.getPrivateMethod(RcsNotification.class, HANDLE_GROUP_INVITATION,
                Context.class, Intent.class, boolean.class);
        method.invoke(NOTIFICATION_INSTANCE, mContext, getMockIntent(), false);
        checkGroupInvitationSize(1);
        NOTIFICATION_INSTANCE.removeGroupInvite(MOCK_SESSION_ID);
        checkGroupInvitationSize(0);
    }

    /**
     * Test the behavior of RcsNotification while an file transfer invitation
     * has been sent.In this test case, we test two circumstance that the file
     * size is bigger than the max file size and the file size is smaller than
     * the max file size .
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase2_HandleFtInvitation() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase2_HandleFtInvitation() entry");
        ApiManager apiManager = ApiManager.getInstance();
        Field messagingApiField = Utils
                .getPrivateField(ApiManager.class, API_MANAGER_MESSAGING_API);
        messagingApiField.set(apiManager, new MockMessagingApi(mContext));
        Method methodhandleFtInvitation = Utils.getPrivateMethod(RcsNotification.class,
                "handleFileTransferInvitation", Context.class, Intent.class);
        Field fileTransferInviteField = Utils.getPrivateField(RcsNotification.class,
                RCSNOTIFICATION_FT_INVITATIONS);
        List fileTsfInvitations = (List) fileTransferInviteField.get(NOTIFICATION_INSTANCE);
        Field apiMgMaxFsField = Utils.getPrivateField(ApiManager.class, "sMaxFileSize");
        long maxFileSize = (Long) apiMgMaxFsField.get(apiManager);

        /*
         * The file size is normal circumstance.
         */
        apiMgMaxFsField.set(apiManager, MOCK_MAX_FILE_SIZE_LAGGER);
        maxFileSize = (Long) apiMgMaxFsField.get(apiManager);
        assertEquals(maxFileSize, MOCK_MAX_FILE_SIZE_LAGGER);
        methodhandleFtInvitation.invoke(NOTIFICATION_INSTANCE, mContext, getMockIntent());
        assertEquals(FT_INVITE_SIZE_ONE, fileTsfInvitations.size());

        /*
         * The file size is lager than the max file size circumstance.
         */
        messagingApiField.set(apiManager, new MockMessagingApi(mContext));
        apiMgMaxFsField.set(apiManager, DEFAULT_FILE_SIZE);
        maxFileSize = (Long) apiMgMaxFsField.get(apiManager);
        assertEquals(maxFileSize, DEFAULT_FILE_SIZE);
        methodhandleFtInvitation.invoke(NOTIFICATION_INSTANCE, mContext, getMockIntent());
        assertTrue(mSessionBeRejected);
        assertEquals(FT_INVITE_SIZE_ONE, fileTsfInvitations.size());

        /*
         * not enough storage.
         */
        messagingApiField.set(apiManager, new MockMessagingApi(mContext));
        Intent intent = getMockIntent();
        intent.putExtra(FILE_SIZE, com.mediatek.rcse.service.Utils.getFreeStorageSize() + 1);
        methodhandleFtInvitation.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertTrue(mSessionBeRejected);
        assertEquals(FT_INVITE_SIZE_ONE, fileTsfInvitations.size());

    }

    /**
     * Test to cancel a file transfer notification.
     */
    public void testCase3_CancelFtNotification() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase3_CancelFtNotification() entry");
        Field fileTransferInviteField = Utils.getPrivateField(RcsNotification.class,
                RCSNOTIFICATION_FT_INVITATIONS);
        List fileTsfInvitations = (List) fileTransferInviteField.get(NOTIFICATION_INSTANCE);
        FileInvitationInfo fileIvitationInfo = new FileInvitationInfo(RcsNotification.CONTACT,
                MOCK_FILE_SIZE, MOCK_FILE_NAME, MOCK_SESSION_ID);
        fileTsfInvitations.add(fileIvitationInfo);
        Field fileSizeField = Utils.getPrivateField(FileInvitationInfo.class,
                RCSNOTIFICATION_FILE_SIZE);
        assertEquals(MOCK_FILE_SIZE, fileSizeField.getLong(fileIvitationInfo));
        NOTIFICATION_INSTANCE.cancelFileTransferNotificationWithContact(RcsNotification.CONTACT,
                MOCK_CANCEL_FILE_SIZE);
        assertEquals(FT__SIZE_ONE, fileSizeField.getLong(fileIvitationInfo));
    }

    /**
     * Test to accept InvitationDialogForIpMesGroup .
     */
    public void testCase4_InvitationDialogForIpMesGroup() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Logger.d(TAG, "testCase4_InvitationDialogForIpMesGroup() entry");
        ActivityMonitor InvitationDialogMonitor = getInstrumentation().addMonitor(
                InvitationDialog.class.getName(), null, false);
        ActivityMonitor chatSreenMonitor = getInstrumentation().addMonitor(
                PluginGroupChatActivity.class.getName(), null, false);
        Intent intent = new Intent(mContext, InvitationDialog.class);
        intent.putExtra(SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(InvitationDialog.KEY_STRATEGY,
                InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getInstrumentation().startActivitySync(intent);
        Activity invitationDialog = getInstrumentation().waitForMonitorWithTimeout(
                InvitationDialogMonitor, TIME_OUT);
        assertNotNull(invitationDialog);
        ApiManager apiManager = ApiManager.getInstance();
        Field messagingApiField = ApiManager.class.getDeclaredField(API_MANAGER_MESSAGING_API);
        messagingApiField.setAccessible(true);
        messagingApiField.set(apiManager, new MockMessagingApi(mContext));
        Field currentStrategyField = Utils.getPrivateField(InvitationDialog.class,
                "mCurrentStrategy");
        Object currentStrategy = currentStrategyField.get(invitationDialog);
        assertNotNull(currentStrategy);
        Method onUserAcceptMethod = Utils.getPrivateMethod(currentStrategy.getClass(),
                "onUserAccept");
        onUserAcceptMethod.invoke(currentStrategy);
        Activity chatSreenActivity = getInstrumentation().waitForMonitorWithTimeout(
                chatSreenMonitor, TIME_OUT);
        assertNotNull(chatSreenActivity);
        invitationDialog.finish();
        chatSreenActivity.finish();
    }

    /**
     * Test case for getRemoteContact() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase5_GetRemoteContact() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "getRemoteContact",
                InstantMessage.class);
        method.setAccessible(true);

        // Test phone number
        InstantMessage message = new InstantMessage("msgid_112", MOCK_PARTICIPANT_ONE,
                "Mocked message", false);
        String contact = (String) method.invoke(NOTIFICATION_INSTANCE, message);
        assertEquals(MOCK_PARTICIPANT_ONE, contact);
        // Test display name
        message = new InstantMessage("msgid_112", "tel:" + MOCK_PARTICIPANT_ONE, "Mocked message",
                false);
        contact = (String) method.invoke(NOTIFICATION_INSTANCE, message);
        assertEquals(MOCK_PARTICIPANT_ONE, contact);
    }

    /**
     * Test case for getGroupChatNotificationTitle() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase6_GetGroupChatNotificationTitle() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "getGroupNotificationTitle",
                List.class);
        method.setAccessible(true);
        // Get group chat title with empty participant list
        List<Participant> participantList1 = new ArrayList<Participant>() {
        };
        String title = (String) method.invoke(NOTIFICATION_INSTANCE, participantList1);
        Resources resources = null;
        resources = AndroidFactory.getApplicationContext().getPackageManager()
                .getResourcesForApplication(CoreApplication.APP_NAME);
        assertNotNull(resources);
        String defaultGroupChatName = resources.getString(R.string.default_group_chat_subject);
        assertTrue(title.contains(defaultGroupChatName));
        // Get group chat title with specify participant list
        List<Participant> participantList2 = new ArrayList<Participant>();
        participantList2.add(new Participant(MOCK_PARTICIPANT_ONE, MOCK_PARTICIPANT_ONE));
        participantList2.add(new Participant(MOCK_PARTICIPANT_TWO, MOCK_PARTICIPANT_TWO));
        participantList2.add(new Participant(MOCK_PARTICIPANT_THREE, MOCK_PARTICIPANT_THREE));
        title = null;
        title = (String) method.invoke(NOTIFICATION_INSTANCE, participantList2);
        int size = participantList2.size();
        String[] participants = new String[size];
        for (int i = 0; i < size; i++) {
            participants[i] = participantList2.get(i).getContact();
        }
        String groupChatName = ChatFragment.getParticipantsName(participants);
        assertTrue(!title.contains(defaultGroupChatName) && title.contains("(" + size + ")"));
    }

    /**
     * Test case for getChatScreen() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase7_GetChatScreen() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "getChatScreen",
                Context.class, Intent.class, String.class, String.class);
        method.setAccessible(true);

        Intent intent1 = new Intent();
        String name = MOCK_PARTICIPANT_ONE;
        String number = MOCK_PARTICIPANT_ONE;
        List<IChat> chatList = (List<IChat>) ModelImpl.getInstance().listAllChat();
        chatList.clear();
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent1, name, number);
        assertTrue(intent1.hasExtra(ChatScreenActivity.KEY_CHAT_TAG));

        Intent intent2 = new Intent();
        name = MOCK_PARTICIPANT_TWO;
        number = null;
        chatList.clear();
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent2, name, number);
        assertTrue(intent2.hasExtra(ChatScreenActivity.KEY_CHAT_TAG));
    }

    /**
     * Test case for updateGroupInvitationNotification() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase08_updateGroupInvitationNotification() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class,
                "updateGroupInvitationNotification");
        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mGroupInvitationInfos");
        ConcurrentHashMap<String, GroupInvitationInfo> infos = (ConcurrentHashMap<String, GroupInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
        // cancelGroupInviteNotification
        method.invoke(NOTIFICATION_INSTANCE);

        // info is null
        String sessionId1 = "id1";
        method.invoke(NOTIFICATION_INSTANCE);

        // SINGLE_GROUP_INVITATION
        GroupInvitationInfo info1 = new GroupInvitationInfo();
        Intent invitation = new Intent();
        info1.context = mContext;
        info1.sender = "abc";
        info1.icon = android.R.drawable.alert_dark_frame;
        info1.notifyInfo = "group invitation1";
        info1.notifyTitle = "title1";
        info1.intent = invitation;
        infos.put(sessionId1, info1);
        method.invoke(NOTIFICATION_INSTANCE);
        assertNotNull(invitation.getStringExtra(RcsNotification.NOTIFY_CONTENT));

        // integreation mode
        Logger.setIsIntegrationMode(true);
        method.invoke(NOTIFICATION_INSTANCE);
        assertNotNull(invitation.getStringExtra(RcsNotification.NOTIFY_CONTENT));

        // size >1
        Logger.setIsIntegrationMode(false);
        GroupInvitationInfo info2 = new GroupInvitationInfo();
        info2.context = mContext;
        info2.sender = "cde";
        info2.icon = R.drawable.rcs_notify_chat_message;
        info2.notifyInfo = "group invitation2";
        info2.notifyTitle = "title2";
        info2.intent = invitation;
        String sessionId2 = "id2";
        infos.put(sessionId1, info1);
        infos.put(sessionId2, info2);
        method.invoke(NOTIFICATION_INSTANCE);
        assertNotNull(invitation.getStringExtra(RcsNotification.NOTIFY_CONTENT));
        infos.clear();
    }

    /**
     * Test case for updateReceiveMessageNotification() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase09_updateReceiveMessageNotification() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class,
                "updateReceiveMessageNotification", String.class, boolean.class);
        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mUnReadMessagesChatInfos");
        ConcurrentHashMap<?, ?> infos = (ConcurrentHashMap<?, ?>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
        method.invoke(NOTIFICATION_INSTANCE, "", true);

        method = Utils.getPrivateMethod(RcsNotification.class, "updateReceiveMessageNotification",
                Context.class, int.class, Intent.class, String.class, String.class, String.class,
                int.class, boolean.class);
        RcsSettings.getInstance().setPhoneVibrateForChatInvitation(true);
        RcsSettings.getInstance().setChatInvitationRingtone(null);
        method.invoke(NOTIFICATION_INSTANCE, mContext, 1, new Intent(), "", "", "",
                android.R.drawable.alert_light_frame, true);
        method.invoke(NOTIFICATION_INSTANCE, mContext, 1, new Intent(), "", "", "",
                android.R.drawable.alert_light_frame, false);
        infos.clear();
    }

    /**
     * Test case for nonAutoAcceptGroupChat() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase10_nonAutoAcceptGroupChat() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "nonAutoAcceptGroupChat",
                Context.class, Intent.class);
        MOCK_PARTICIPANTS = null;
        method.invoke(NOTIFICATION_INSTANCE, mContext, new Intent());

        MOCK_PARTICIPANTS = new ArrayList<String>();
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_ONE);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_TWO);
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_THREE);
        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mGroupInvitationInfos");
        ConcurrentHashMap<String, GroupInvitationInfo> infos = (ConcurrentHashMap<String, GroupInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
        Logger.setIsIntegrationMode(true);
        String sessionId1 = MOCK_SESSION_ID;
        String sessionId2 = MOCK_SESSION_ID2;
        GroupInvitationInfo info1 = new GroupInvitationInfo();
        Intent invitation = new Intent();
        info1.context = mContext;
        info1.sender = "abc";
        info1.icon = android.R.drawable.alert_dark_frame;
        info1.notifyInfo = "group invitation1";
        info1.notifyTitle = "title1";
        info1.intent = invitation;
        infos.put(sessionId1, info1);
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);

        intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT, "");
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);

        Logger.setIsIntegrationMode(false);
        intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);

        Logger.setIsIntegrationMode(true);
        GroupInvitationInfo info2 = new GroupInvitationInfo();
        info2.context = mContext;
        info2.sender = "cde";
        info2.icon = R.drawable.rcs_notify_chat_message;
        info2.notifyInfo = "group invitation2";
        info2.notifyTitle = "title2";
        info2.intent = invitation;
        infos.put(sessionId1, info1);
        infos.put(sessionId2, info2);
        intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID2);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);

        Logger.setIsIntegrationMode(false);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        infos.clear();
    }

    /**
     * Test case for updateFileTansferNotification() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase11_updateFileTansferNotification() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class,
                "updateFileTansferNotification", Intent.class);

        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mFileInvitationInfos");
        List<FileInvitationInfo> infos = (List<FileInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, intent);

        FileInvitationInfo info1 = new FileInvitationInfo("", 1l, "", MOCK_SESSION_ID);
        FileInvitationInfo info2 = new FileInvitationInfo("", 1l, "", MOCK_SESSION_ID);
        infos.add(info1);
        infos.add(info2);
        intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, intent);

        infos.clear();

    }

    /**
     * Test case for showFileTransferNotification() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase12_showFileTransferNotification() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class,
                "showFileTransferNotification", Context.class, Intent.class,
                IFileTransferSession.class);

        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mFileInvitationInfos");
        List<FileInvitationInfo> infos = (List<FileInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);

        FileInvitationInfo info1 = new FileInvitationInfo(MOCK_CALLER, 1l, "", MOCK_SESSION_ID);
        FileInvitationInfo info2 = new FileInvitationInfo(MOCK_CALLER, 1l, "", MOCK_SESSION_ID);
        infos.add(info1);
        infos.add(info2);
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent, new MockFtSession());

        infos.clear();

    }

    /**
     * Test case for buildNotificationInfo() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase13_buildNotificationInfo() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "buildNotificationInfo",
                Context.class, Intent.class);
        assertNull(method.invoke(NOTIFICATION_INSTANCE, mContext, null));
        MOCK_PARTICIPANTS = null;

        Intent intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        assertNull(method.invoke(NOTIFICATION_INSTANCE, mContext, intent));

        MOCK_PARTICIPANTS = new ArrayList<String>();
        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_ONE);
        assertNull(method.invoke(NOTIFICATION_INSTANCE, mContext, intent));

        MOCK_PARTICIPANTS.add(MOCK_PARTICIPANT_TWO);
        assertNotNull(method.invoke(NOTIFICATION_INSTANCE, mContext, intent));

        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mGroupInvitationInfos");
        ConcurrentHashMap<String, GroupInvitationInfo> infos = (ConcurrentHashMap<String, GroupInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
    }

    /**
     * Test case for autoAcceptGroupChat() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase14_autoAcceptGroupChat() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "autoAcceptGroupChat",
                Context.class, Intent.class);
        Intent intent = new Intent();
        InstantMessage msg = new InstantMessage("", "", "", true);
        intent.putExtra("firstMessage", msg);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getParcelableArrayListExtra("messages"));
    }

    /**
     * Test case for handleInvitation() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase15_handleInvitation() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class, "handleInvitation",
                Context.class, Intent.class);
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, "");
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

        // non auto accept group chat, integration mode
        Logger.setIsIntegrationMode(true);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get((ModelImpl) ModelImpl
                .getInstance());
        chatMap.clear();
        sIsGroupChat = true;
        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(RcsNotification.CHAT_ID, "123");
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT));

        // non auto accept group chat, not integration mode
        Logger.setIsIntegrationMode(false);
        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(RcsNotification.CHAT_ID, "123");
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNull(intent.getStringExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT));

        // auto accept group chat
        Participant participant1 = new Participant(MOCK_PARTICIPANT_ONE, "");
        Participant participant2 = new Participant(MOCK_PARTICIPANT_TWO, "");
        ArrayList<Participant> list = new ArrayList<Participant>();
        list.add(participant1);
        list.add(participant2);
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid1);
        MockGroupChat groupChat = new MockGroupChat((ModelImpl) ModelImpl.getInstance(),
                new MockGroupChatWindow(), list, tag);
        chatMap.put(tag, groupChat);
        sIsGroupChat = true;
        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(RcsNotification.CHAT_ID, "123");
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNull(intent.getStringExtra("messages"));

        // not group chat
        sIsGroupChat = false;
        intent = new Intent(MessagingApiIntents.CHAT_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        InstantMessage msg = new InstantMessage("", "", "", true);
        intent.putExtra("firstMessage", msg);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getParcelableArrayListExtra("messages"));

        // file transfer
        intent = new Intent(MessagingApiIntents.FILE_TRANSFER_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

        // image share
        intent = new Intent(RichCallApiIntents.IMAGE_SHARING_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

        // video share
        intent = new Intent(RichCallApiIntents.VIDEO_SHARING_INVITATION);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

        // session replace
        intent = new Intent(MessagingApiIntents.CHAT_SESSION_REPLACED);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        method.invoke(NOTIFICATION_INSTANCE, mContext, intent);
        assertNotNull(intent.getStringExtra(RcsNotification.DISPLAY_NAME));

    }

    /**
     * Test case for cancelFileTransferNotificationWithContact() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase16_cancelFileTransferNotificationWithContact()
            throws NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException, InterruptedException,
            android.content.pm.PackageManager.NameNotFoundException {
        Method method = Utils.getPrivateMethod(RcsNotification.class,
                "cancelFileTransferNotificationWithContact", String.class);

        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mFileInvitationInfos");
        List<FileInvitationInfo> infos = (List<FileInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        FileInvitationInfo info1 = new FileInvitationInfo("", 1l, "", MOCK_SESSION_ID);
        infos.add(info1);
        method.invoke(NOTIFICATION_INSTANCE, "");
        assertEquals(0, infos.size());
    }

    @Override
    protected void tearDown() throws NoSuchFieldException, Exception {
        super.tearDown();
        Field fieldInfos = Utils.getPrivateField(RcsNotification.class, "mFileInvitationInfos");
        List<FileInvitationInfo> infos = (List<FileInvitationInfo>) fieldInfos
                .get(NOTIFICATION_INSTANCE);
        infos.clear();
        Field fieldGroupInfos = Utils.getPrivateField(RcsNotification.class,
                "mGroupInvitationInfos");
        ConcurrentHashMap<String, GroupInvitationInfo> groupInfos = (ConcurrentHashMap<String, GroupInvitationInfo>) fieldGroupInfos
                .get(NOTIFICATION_INSTANCE);
        groupInfos.clear();
        Utils.clearAllStatus();
    }

    private Intent getMockIntent() {
        Intent intent = new Intent();
        intent.putExtra(RcsNotification.SESSION_ID, MOCK_SESSION_ID);
        intent.putExtra(FIRST_MESSAGE, MOCK_FIRST_MESSAGE);
        intent.putExtra(FILE_SIZE, DEFAULT_FILE_SIZE);
        intent.putExtra(RcsNotification.CONTACT, MOCK_CALLER);
        return intent;
    }

    private void checkGroupInvitationSize(int expectSize) throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field groupInviteField = RcsNotification.class
                .getDeclaredField(RCSNOTIFICATION_GROUP_INVITATIONS);
        groupInviteField.setAccessible(true);
        Map groupInvitations = (Map) groupInviteField.get(NOTIFICATION_INSTANCE);
        assertEquals(expectSize, groupInvitations.size());
    }

    /**
     * The mock messagingApi can be used to provide a mock chat session even in
     * off-line state
     */
    private class MockMessagingApi extends MessagingApi {

        public MockMessagingApi(Context ctx) {
            super(ctx);
        }

        @Override
        public IChatSession getChatSession(String id) throws ClientApiException {
            Logger.d(TAG, "getChatSession the id is " + id);
            if (MOCK_SESSION_ID.equals(id) || MOCK_SESSION_ID2.equals(id)) {
                return new MockChatSession();
            } else {
                return super.getChatSession(id);
            }
        }

        @Override
        public IFileTransferSession getFileTransferSession(String id) throws ClientApiException {
            if (MOCK_SESSION_ID.equals(id) || MOCK_SESSION_ID2.equals(id)) {
                return new MockFtSession();
            } else {
                return super.getFileTransferSession(id);
            }

        }
    }

    /**
     * Mock chat session that has a mock session-id and a mock participant list
     */
    private class MockChatSession implements IChatSession {

        @Override
        public void acceptSession() throws RemoteException {
        }

        @Override
        public void addParticipant(String participant) throws RemoteException {
        }

        @Override
        public int getSessionDirection()
        {
        	return 0;
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
            return "123";
        }

        @Override
        public InstantMessage getFirstMessage() throws RemoteException {
            return null;
        }

        @Override
        public List<String> getInivtedParticipants() throws RemoteException {
            return MOCK_PARTICIPANTS;
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
            return sIsGroupChat;
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
        public void setMessageDisplayedStatusBySipMessage(String contact, String msgId,
                String status) throws RemoteException {
        }

        @Override
        public IBinder asBinder() {
            return null;
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
    }

    /**
     * Mock chat session that has a mock session-id , a mock file size and a
     * mock file name.
     */
    private class MockFtSession implements IFileTransferSession {

        @Override
        public void acceptSession() throws RemoteException {
        }

        @Override
        public void addSessionListener(IFileTransferEventListener listener) throws RemoteException {
        }

        @Override
        public void cancelSession() throws RemoteException {
        }

        @Override
        public boolean isSessionPaused()
        {
        	return false;
        }
    
        @Override
        public String getFilename() throws RemoteException {
            return MOCK_FILE_NAME;
        }

        @Override
        public long getFilesize() throws RemoteException {
            return MOCK_FILE_SIZE;
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
        public void rejectSession() throws RemoteException {
            Logger.d(TAG, "rejectSession entry");
            mSessionBeRejected = true;
            Logger.d(TAG, "rejectSession exit");
        }

        @Override
        public void removeSessionListener(IFileTransferEventListener listener)
                throws RemoteException {
        }

        @Override
        public IBinder asBinder() {
            return null;
        }

		@Override
		public List<String> getContacts() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public boolean isGroupTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public boolean isHttpTransfer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}

		@Override
		public String getChatID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public String getChatSessionID() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void pauseSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void resumeSession() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public String getFileThumbUrl() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}

    }

    /**
     * Mock group chat.
     */
    private class MockGroupChat extends GroupChat {
        private boolean mIsMessageSent = false;
        private boolean mReloaded = false;
        private boolean mIsEmpty = false;

        public MockGroupChat(ModelImpl modelImpl, IGroupChatWindow chatWindow,
                List<Participant> participants, Object tag) {
            super(modelImpl, chatWindow, participants, tag);
        }

        public boolean isMessageSent() {
            return mIsMessageSent;
        }

        public boolean isReloaded() {
            return mReloaded;
        }

        public boolean isEmpty() {
            return mIsEmpty;
        }

        public void sendMessage(String content, int messageTag) {
            super.sendMessage(content, messageTag);
            mIsMessageSent = true;
        }

        protected void reloadMessage(final InstantMessage message, final int messageType,
                final int status) {
            super.reloadMessage(message, messageType, status);
            mReloaded = true;
        }

        public void hasTextChanged(boolean isEmpty) {
            mIsEmpty = isEmpty;
        }
    }

    private class MockGroupChatWindow implements IGroupChatWindow {
        private boolean mMessagesRemoved = false;

        public boolean isMessagesRemoved() {
            return mMessagesRemoved;
        }

        public void updateParticipants(List<ParticipantInfo> participants) {

        }

        public void setIsComposing(boolean isComposing, Participant participant) {

        }

        public void setIsRejoining(boolean isRejoining) {

        }

        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return null;
        }

        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            return null;
        }

        public void removeAllMessages() {
            mMessagesRemoved = true;
        }

        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        public void addLoadHistoryHeader(boolean showLoader) {

        }

        public void updateAllMsgAsRead() {

        }

        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

		@Override
		public void removeChatMessage(String messageId) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void setFileTransferEnable(int reason) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void updateChatStatus(int status) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void addgroupSubject(String subject) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IFileTransfer addSentFileTransfer(FileStruct file) {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}

    }
}
