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

package com.mediatek.rcse.service;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.Telephony.Sms;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import android.widget.Toast;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApiIntents;
import com.orangelabs.rcs.service.api.server.gsma.GetContactCapabilitiesReceiver;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

public class RcsNotification {
    /**
     * Notification ID for chat
     */
    private static final int NOTIFICATION_ID_CHAT = 1000;
    /**
     * Notification ID for file transfer
     */
    private static final int NOTIFICATION_ID_FILE_TRANSFER = 1001;

    private static final int NOTIFICATION_ID_UNREAD_MESSAGE = 1004;

    private static final int NOTIFICATION_TITLE_LENGTH_MAX = 20;

    private static final int NOTIFICATION_CHAR_LENGTH_CONDITION = 0xFF;

    private static final int NOTIFICATION_CHAR_LENGTH_SINGLE_BYTE = 1;

    private static final int NOTIFICATION_CHAR_LENGTH_DOUBLE_BYTE = 2;

    public static final String FORCE_SCROLLTO_CHAT = "forceScroll";

    private static final String TAG = "RcsNotification";
    private static final String FIRST_MESSAGE = "firstMessage";
    private static final String GROUP_SUBJECT = "subject";
    public static final String CONTACT = "contact";
    private static final String MESSAGES = "messages";
    public static final String SESSION_ID = "sessionId";
    public static final String DISPLAY_NAME = "contactDisplayname";
	public static final String CHAT_SESSION_ID = "chatSessionId";  
	public static final String ISGROUPTRANSFER = "isGroupTransfer";
	public static final String ISHTTPTRANSFER = "isHttpTransfer";
    /**
     * Chat id.
     */
    public static final String CHAT_ID = "chatId";
    /**
     * Auto accept group invitation.
     */
    public static final String AUTO_ACCEPT = "autoAccept";

    // The RcsNotification instance
    private static final RcsNotification INSTANCE = new RcsNotification();;
    // The GroupInviteChangeListener instance
    private GroupInviteChangedListener mListener = null;

    // The blank space text
    private static final String BLANK_SPACE = " ";
    // The empty text
    private static final String EMPTY_STRING = "";
    // The seprator text
    private static final String SEPRATOR = ",";
    // Used for storing the title of a specify notification
    public static final String NOTIFY_TITLE = "notify_title";
    // Used for storing the content of a specify notification
    public static final String NOTIFY_CONTENT = "notify_content";
    // Used for storing the additional information of a specify notification
    public static final String NOTIFY_INFORMATION = "notify_information";
    public static final String NOTIFY_SIZE = "notify_size";
    public static final String NOTIFY_FILE_NAME = "notify_file_name";

    // Indicates the single group invitation
    private static final int SINGLE_GROUP_INVITATION = 1;
    // The tag of unread message notification
    private static final String UNREAD_MESSAGE = "UnreadMessage";
    private static final String FILE_TRANSFER = "FileTransfer";
    private ConcurrentHashMap<String, GroupInvitationInfo> mGroupInvitationInfos = 
            new ConcurrentHashMap<String, GroupInvitationInfo>();
    private ConcurrentHashMap<IChatSession, Intent> mTempGroupInvitationInfos = 
            new ConcurrentHashMap<IChatSession, Intent>();
    private static final int GROUP_PARTICIPANT_SIZE_TWO = 2;
    private static final int GROUP_PARTICIPANT_SIZE_THREE = 3;
    private static final String GROUP_INVITATION_TAG = "groupInvitation";
    private ConcurrentHashMap<Object, ChatReceivedInfo> mUnReadMessagesChatInfos = 
            new ConcurrentHashMap<Object, ChatReceivedInfo>();
    private List<FileInvitationInfo> mFileInvitationInfos = new CopyOnWriteArrayList<FileInvitationInfo>();
    public static final int FILE_INVITATION_INDEX_ZERO = 0;

    public boolean sIsStoreAndForwardMessageNotified = false;

    private boolean mIsInChatMainActivity = false;

    /**
     * set is in ChatMainActivity
     */
    public void setIsInChatMainActivity(boolean isInChatMainActivity) {
        mIsInChatMainActivity = isInChatMainActivity;
    }

    /**
     * Constructor
     */
    private RcsNotification() {
    }

    /**
     * Get the RcsNotification singleton instance
     * 
     * @return The RcsNotification instance
     */
    public static RcsNotification getInstance() {
        return INSTANCE;
    }

    /**
     * Register the GroupInviteChangeListener
     * 
     * @param listener The GroupInviteChangeListener to register
     */
    public synchronized void registerGroupInviteChangedListener(GroupInviteChangedListener listener) {
        Logger.d(TAG, "registerGroupInviteChangedListener() entry listener is " + listener);
        mListener = listener;
        Logger.d(TAG, "registerGroupInviteChangedListener() exit");
    }

    /**
     * Unregister the GroupInviteChangeListener
     */
    public synchronized void unregisterGroupInviteChangedListener() {
        Logger.d(TAG, "unregisterGroupInviteChangedListener() entry listener is " + mListener);
        if (mListener != null) {
            mListener = null;
        }
        Logger.d(TAG, "unregisterGroupInviteChangedListener() exit listener is " + mListener);
    }

    /**
     * The listener used to notify that the invitation has changed.
     */
    public interface GroupInviteChangedListener {
        /**
         * Add group invitation.
         * 
         * @param chatSession The chatSession used to add.
         * @param intent The invite intent used to add.
         */
        void onAddedGroupInvite(final IChatSession chatSession, final Intent intent);

        /**
         * Remove group invitation.
         * 
         * @param sessionId The sessionId used to indicate the removing item.
         */
        void onRemovedGroupInvite(final String sessionId);
    }

    /**
     * Remove the stored data of one specific session in notification manager,
     * provided.
     * 
     * @param session The session that who start the group chat.
     */
    public synchronized void removeGroupInvite(String sessionId) {
        Logger.d(TAG, "removeGroupInvite() entry, mListener: " + mListener);
        if (mListener != null) {
            mListener.onRemovedGroupInvite(sessionId);
        }
        updateTempGroupInvitationInfos(sessionId);
        mGroupInvitationInfos.remove(sessionId);
        updateGroupInvitationNotification();
        Logger.d(TAG, "removeGroupInvite() exit");
    }

    /**
     * remove a item from mTempGroupInvitationInfos by sessionId
     * 
     * @param sessionId The session that who start the group chat
     */
    private void updateTempGroupInvitationInfos(String sessionId) {
        Logger.d(TAG, "updateTempGroupInvitationInfos() entry, sessionId: " + sessionId + " size: "
                + mTempGroupInvitationInfos.size());
        try {
            for (Map.Entry<IChatSession, Intent> entry : mTempGroupInvitationInfos.entrySet()) {
                if (sessionId.equals(entry.getKey().getSessionID())) {
                    Logger.d(TAG, "updateTempGroupInvitationInfos() find session id equal");
                    mTempGroupInvitationInfos.remove(entry.getKey());
                    break;
                }
            }
        } catch (RemoteException e) {
            Logger.e(TAG, "updateTempGroupInvitationInfos() Get chat session failed");
            e.printStackTrace();
        }
        Logger.v(TAG,
                "updateTempGroupInvitationInfos() exit, size: " + mTempGroupInvitationInfos.size());
    }

    /**
     * Get all the group chat invitations map.
     * 
     * @return The group chat invitations map.
     */
    public synchronized ConcurrentHashMap<IChatSession, Intent> getTempGroupInvite() {
        Logger.d(TAG, "getTempGroupInvite() entry, mTempGroupInvitationInfos size: "
                + mTempGroupInvitationInfos.size());
        return mTempGroupInvitationInfos;

    }

    /**
     * Store the group chat invitation with a specific session in notification
     * manager.
     * 
     * @param session The session that who start the group chat.
     * @param info The group invitation information.
     */
    private synchronized void addGroupInvite(IChatSession chatSession, GroupInvitationInfo info) {
        Logger.d(TAG, "addGroupInvite entry, mListener: " + mListener);
        if (mListener != null) {
            Logger.d(TAG, "addGroupInvite mListener is not null");
            mListener.onAddedGroupInvite(chatSession, info.intent);
        }
        mTempGroupInvitationInfos.put(chatSession, info.intent);
        try {
            String sessionId = chatSession.getSessionID();
            mGroupInvitationInfos.put(sessionId, info);
            Logger.d(TAG, "addGroupInvite() sessionId: " + sessionId);
        } catch (RemoteException e) {
            Logger.e(TAG, "addGroupInvite getSessionID fail");
            e.printStackTrace();
        }
    }

    /**
     * Cancel group invitation notification.
     */
    private void cancelGroupInviteNotification() {
        int size = mGroupInvitationInfos.size();
        Logger.d(TAG, "cancelGroupInviteNotification() entry, size: " + size);
        if (size == 0) {
            Context context = ApiManager.getInstance().getContext();
            Logger.e(TAG, "cancelGroupInviteNotification() context: " + context);
            if (context != null) {
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.cancel(GROUP_INVITATION_TAG, NOTIFICATION_ID_CHAT);
            }
        }
        Logger.d(TAG, "cancelGroupInviteNotification exit");
    }

    /**
     * Cancel File transfer notification when cancel the notification.
     * 
     * @param contact the notification related to.
     * @param cancleSize the file size you canceled.
     */
    public void cancelFileTransferNotificationWithContact(String contact, long cancelSize) {
        Logger.d(TAG, "cancelFileTransferNotificationWithContact entry  the contact is " + contact
                + "cancleSize is " + cancelSize);
        int size = mFileInvitationInfos.size();
        for (int i = 0; i < size; i++) {
            if (contact.equals(mFileInvitationInfos.get(i).mContact)) {
                if (cancelSize >= 0) {
                    Logger.d(TAG, "cancelFileTransferNotificationWithContact()"
                            + " with param cancelsize find the relavent contact");
                    mFileInvitationInfos.get(i).mInviteNumber--;
                    mFileInvitationInfos.get(i).mFileSize -= cancelSize;
                    if (mFileInvitationInfos.get(i).mInviteNumber == 0) {
                        mFileInvitationInfos.remove(i);
                    }
                } else {
                    Logger.e(TAG,
                            "cancelFileTransferNotificationWithContact the cancel size is illegal");
                }
                updateFileTansferNotification(null);
                break;
            }
        }
        Logger.d(TAG, "cancelFileTransferNotificationWithContact with param cancelsize exit");
    }

    /**
     * Cancel File transfer notification.
     * 
     * @param contact the notification related to.
     */
    public void cancelFileTransferNotificationWithContact(String contact) {
        Logger.d(TAG, "cancelFileTransferNotificationWithContact() entry with the contact "
                + contact);
        int size = mFileInvitationInfos.size();
        Logger.d(TAG, "cancelFileTransferNotificationWithContact() mFileInvitationInfos size is "
                + size);
        for (int i = 0; i < size; i++) {
            if (contact.equals(mFileInvitationInfos.get(i).mContact)) {
                Logger.d(TAG,
                        "cancelFileTransferNotificationWithContact() find the relavent contact");
                mFileInvitationInfos.remove(i);
                updateFileTansferNotification(null);
                break;
            }
        }
        Logger.d(TAG, "cancelFileTransferNotificationWithContact() exit");
    }

    private IChatSession getChatSession(Intent intent) {
        ApiManager instance = ApiManager.getInstance();
        String sessionId = intent.getStringExtra(SESSION_ID);
        if (instance == null) {
            Logger.i(TAG, "ApiManager instance is null");
            return null;
        }
        MessagingApi messageApi = instance.getMessagingApi();
        if (messageApi == null) {
            Logger.d(TAG, "MessageingApi instance is null");
            return null;
        }
        IChatSession chatSession = null;
        try {
            chatSession = messageApi.getChatSession(sessionId);
        } catch (ClientApiException e) {
            Logger.e(TAG, "Get chat session failed");
            e.printStackTrace();
            chatSession = null;
        } finally {
            return chatSession;
        }
    }

    private IFileTransferSession getFileTransferSession(String sessionId) {
        ApiManager instance = ApiManager.getInstance();
        MessagingApi messageApi = instance.getMessagingApi();
        if (messageApi == null) {
            Logger.d(TAG, "getFileTransferSession() MessageingApi instance is null");
            return null;
        }
        IFileTransferSession fileTransferSession = null;
        try {
            fileTransferSession = messageApi.getFileTransferSession(sessionId);
        } catch (ClientApiException e) {
            Logger.e(TAG, "getFileTransferSession() Get chat session failed");
            e.printStackTrace();
            fileTransferSession = null;
        } finally {
            return fileTransferSession;
        }
    }

    /**
     * Handle the group chat invitation.
     * 
     * @param context The context.
     * @param invitation The group chat invitation.
     * @param autoAccept True if auto accept, otherwise false.
     */
    private void handleGroupChatInvitation(Context context, Intent invitation, boolean autoAccept) {
        Logger.v(TAG, "handleGroupChatInvitation() entry, autoAccept: " + autoAccept);
        if (autoAccept) {
        	handlePluginGroupChatInvitation(context, invitation, true);
            autoAcceptGroupChat(context, invitation);
           } else {
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG, "handleGroupChatInvitation integration mode");
                handlePluginGroupChatInvitation(context, invitation, false);
            } else {
                Logger.d(TAG, "handleGroupChatInvitation chat app mode");
            }
            nonAutoAcceptGroupChat(context, invitation);
        }
        Logger.v(TAG, "handleGroupChatInvitation() exit");
    }

    private void handlePluginGroupChatInvitation(Context context, Intent invitation, boolean autoAccept) {
        Logger.d(TAG, "handlePluginGroupChatInvitation entry");
        IChatSession chatSession = getChatSession(invitation);
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        if (chatSession == null) {
            Logger.d(TAG, "by simple The chatSession is null");
            return;
        }
        String sessionId = invitation.getStringExtra(SESSION_ID);
        String groupSubject = invitation.getStringExtra(GROUP_SUBJECT);
        String chatId = invitation.getStringExtra(CHAT_ID);
        String firstMessage = invitation.getStringExtra(FIRST_MESSAGE);
        GroupInvitationInfo info = buildNotificationInfo(context, invitation);
        if(info == null)
        {
        	Logger.d(TAG, "notification info  is null");
        	return;
        }
        // Add mms db
        String contact = PluginGroupChatWindow.generateGroupChatInvitationContact(sessionId);
        if(autoAccept)
        {
            contact = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER + parcelUuid.toString();
            info.notifyInfo = firstMessage;
        }
        invitation.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT, contact);
        int messageTag = PluginGroupChatWindow.GROUP_CHAT_INVITATION_IPMSG_ID;
       
        
        Logger.d(TAG, "notify info is" + info.notifyInfo +"contact=" + contact);
        // broadcast intent to mms plugin to insert into mms db
        Intent intent = new Intent();
        if(info!=null)
        intent.putExtra("notify", info.notifyInfo);
        intent.putExtra("contact", contact);
        intent.putExtra("messageTag", messageTag);
        intent.putExtra("subject", groupSubject);
        intent.setAction(IpMessageConsts.JoynGroupInvite.ACTION_GROUP_IP_INVITATION);
        intent.putExtra("groupinvite", 1);
        AndroidFactory.getApplicationContext().sendStickyBroadcast(intent);
        
        /*Long messageIdInMms = PluginUtils.insertDatabase(info.notifyInfo, contact, messageTag,
                PluginUtils.INBOX_MESSAGE);
        if (ThreadTranslater.tagExistInCache(contact))
		{	Logger.d(TAG,
					"plugingroupchatinvitation() Tag exists" + contact);
			Long thread = ThreadTranslater.translateTag(contact);
			insertThreadIDInDB(thread, groupSubject);				
		}*/
        if (contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
            contact = contact.substring(4);
        }
                
        
        invitation.putExtra(ChatScreenActivity.KEY_CHAT_TAG, parcelUuid);
        invitation
        .putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT,contact);
        Logger.d(TAG, "handlePluginGroupChatInvitation parcelUuid is " + parcelUuid.toString() + "contact is" + contact);
        
    }

    
        
    /**
     * Handle the group chat invitation with automatic accept.
     * 
     * @param context The context.
     * @param invitation The group chat invitation.
     */
    private void autoAcceptGroupChat(Context context, Intent invitation) {
        handleChatInvitation(context, invitation);
    }

    /**
     * Handle the group chat invitation with non-automatic accept.
     * 
     * @param context The context.
     * @param invitation The group chat invitation.
     */
    private void nonAutoAcceptGroupChat(Context context, Intent invitation) {
        Logger.v(TAG, "handleGroupChatInvitation entry");
        GroupInvitationInfo info = buildNotificationInfo(context, invitation);
        if (info == null) {
            Logger.d(TAG, "handleGroupChatInvitation info is null");
            return;
        }
        // Should increase the number of unread message
        Logger.d(TAG, "Has receive a group chat invitation");
        UnreadMessageManager.getInstance().changeUnreadMessageNum(
                UnreadMessageManager.MIN_STEP_UNREAD_MESSAGE_NUM, true);
        int size = mGroupInvitationInfos.size();
        StringBuilder notifyTitle = new StringBuilder(info.notifyTitle);
        String notifyContent = null;
        Intent intent = info.intent;
        if (size == SINGLE_GROUP_INVITATION) {
            notifyTitle.append(context.getString(R.string.group_invitation_notify_title));
            notifyTitle.append(BLANK_SPACE);
            notifyTitle.append(info.sender);
            notifyContent = info.notifyInfo;
            ParcelUuid tag = (ParcelUuid) invitation
                    .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);
            intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, tag);
            intent.setClass(context, InvitationDialog.class);
            if (Logger.getIsIntegrationMode()) {
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
                String contact = invitation
                        .getStringExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT);
                if (null != contact) {
                    intent.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT, contact);
                } else {
                    Logger.w(TAG, "nonAutoAcceptGroupChat() contact is null");
                }
            } else {
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_GROUP_INVITATION);
            }
        } else {
            notifyTitle.append(size);
            notifyTitle.append(BLANK_SPACE);
            notifyTitle.append(context.getString(R.string.group_multi_invitation_title));
            notifyContent = context.getString(R.string.group_multi_invitation_content);

            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG, "nonAutoAcceptGroupChat start mms");
                intent.setClassName(PluginGroupChatWindow.MMS_PACKAGE_STRING,
                        PluginGroupChatWindow.MMS_BOOT_ACTIVITY_STRING);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            } else {
                intent.setClass(context, ChatMainActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            }

        }
        intent.putExtra(NOTIFY_TITLE, notifyTitle.toString());
        intent.putExtra(NOTIFY_CONTENT, notifyContent);
        PendingIntent contentIntent = PendingIntent.getActivity(context, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        Notification.Builder builder = new Notification.Builder(context);
        builder.setContentTitle(notifyTitle);
        builder.setContentIntent(contentIntent);
        builder.setSmallIcon(info.icon);
        builder.setContentText(notifyContent);
        builder.setWhen(System.currentTimeMillis());
        builder.setAutoCancel(false);
        Notification notification = builder.getNotification();
        Logger.v(TAG,
                "handleGroupChatInvitation SettingsFragment.IS_NOTIFICATION_CHECKED.get() is "
                        + SettingsFragment.IS_NOTIFICATION_CHECKED.get());
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.d(TAG, "handleGroupChatInvitation notification is built, with size: " + size);
            String ringtone = RcsSettings.getInstance().getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            if (RcsSettings.getInstance().isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }
            NotificationManager groupInviteNotification = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            groupInviteNotification
                    .notify(GROUP_INVITATION_TAG, NOTIFICATION_ID_CHAT, notification);
        } else {
            Logger.v(TAG,
                    "handleGroupChatInvitation SettingsFragment.IS_NOTIFICATION_CHECKED.get() is false ");
        }
        Logger.v(TAG, "handleGroupChatInvitation exit");
    }

    private GroupInvitationInfo buildNotificationInfo(Context context, Intent invitation) {
        Logger.v(TAG, "buildNotificationInfo entry");
        if (invitation == null) {
            Logger.d(TAG, "buildNotificationInfo current invitation is null");
            return null;
        }
        IChatSession chatSession = getChatSession(invitation);
        if (chatSession == null) {
            Logger.d(TAG, "buildNotificationInfo current chat session is null");
            return null;
        }
        ArrayList<InstantMessage> messages = new ArrayList<InstantMessage>();
        messages.add((InstantMessage) invitation.getParcelableExtra(FIRST_MESSAGE));
        invitation.putParcelableArrayListExtra(MESSAGES, messages);
        StringBuffer notifyTitle = new StringBuffer();
        StringBuffer notifyInfo = new StringBuffer();
        // Notification information
        String sender = formatCallerId(invitation);
        List<String> participants = null;
        try {
            participants = chatSession.getInivtedParticipants();
        } catch (RemoteException remoteException) {
            Logger.e(TAG, "buildNotificationInfo getInivtedParticipants fail");
            remoteException.printStackTrace();
        }
        if (participants == null) {
            Logger.i(TAG, "buildNotificationInfo paticipants list is null");
            return null;
        } else {
            int count = participants.size();
            if (count >= GROUP_PARTICIPANT_SIZE_TWO) {
                String notify = null;
                String contact = participants.get(0);
                if (PhoneUtils.isANumber(contact)) {
                    contact = ContactsListManager.getInstance()
                            .getDisplayNameByPhoneNumber(contact);
                }
                if (count >= GROUP_PARTICIPANT_SIZE_THREE) {
                    notify = context.getString(R.string.notify_invitation_multi_participants,
                            sender, contact, count - 1);
                } else {
                    notify = context.getString(R.string.notify_invitation_two_participants, sender,
                            contact);
                }
                notifyInfo.append(notify);
            } else {
                Logger.i(TAG, "buildNotificationInfo paticipants list is invalid");
                return null;
            }
        }
        invitation.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        invitation.putExtra(NOTIFY_INFORMATION, notifyInfo.toString());
        invitation.putExtra(FORCE_SCROLLTO_CHAT, true);
        GroupInvitationInfo info = new GroupInvitationInfo();
        info.context = context;
        info.sender = sender;
        info.icon = R.drawable.rcs_notify_chat_message;
        info.notifyInfo = notifyInfo.toString();
        info.notifyTitle = notifyTitle.toString();
        info.intent = invitation;
        addGroupInvite(chatSession, info);
        Logger.v(TAG, "buildNotificationInfo exit");
        return info;
    }

    private void updateGroupInvitationNotification() {
        Set<String> keys = mGroupInvitationInfos.keySet();
        int size = keys.size();
        Logger.d(TAG, "updateGroupInvitationNotification entry, with size: " + size);
        if (size == SINGLE_GROUP_INVITATION) {
            String sessionId = keys.iterator().next();
            GroupInvitationInfo info = mGroupInvitationInfos.get(sessionId);
            if (info == null) {
                Logger.v(TAG, "updateGroupInvitationNotification info is null");
                return;
            }
            Context context = info.context;
            StringBuilder titleBuilder = new StringBuilder(info.notifyTitle);
            titleBuilder.append(context.getString(R.string.group_invitation_notify_title));
            titleBuilder.append(BLANK_SPACE);
            titleBuilder.append(info.sender);
            String notifyContent = info.notifyInfo;
            Intent intent = info.intent;
            if (intent != null) {
                intent.setClass(context, InvitationDialog.class);
                if (Logger.getIsIntegrationMode()) {
                    intent.putExtra(InvitationDialog.KEY_STRATEGY,
                            InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
                } else {
                    intent.putExtra(InvitationDialog.KEY_STRATEGY,
                            InvitationDialog.STRATEGY_GROUP_INVITATION);
                }
                intent.putExtra(NOTIFY_TITLE, titleBuilder.toString());
                intent.putExtra(NOTIFY_CONTENT, notifyContent);
            }
            PendingIntent contentIntent = PendingIntent.getActivity(context, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(context);
            builder.setContentTitle(titleBuilder.toString());
            builder.setContentText(notifyContent);
            builder.setContentIntent(contentIntent);
            builder.setSmallIcon(info.icon);
            builder.setWhen(System.currentTimeMillis());
            builder.setAutoCancel(false);
            Logger.v(TAG, "updateGroupInvitationNotification notification checked: "
                    + SettingsFragment.IS_NOTIFICATION_CHECKED.get() + " intent: " + intent);
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
                Notification notification = builder.getNotification();
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.notify(GROUP_INVITATION_TAG, NOTIFICATION_ID_CHAT,
                        notification);
            } 
        } else if (size > 1) {
            String sessionId = keys.iterator().next();
            GroupInvitationInfo info = mGroupInvitationInfos.get(sessionId);
            if (info == null) {
                Logger.v(TAG, "updateGroupInvitationNotification info is null");
                return;
            }
            Context context = info.context;
            StringBuilder titleBuilder = new StringBuilder();
            titleBuilder.append(size);
            titleBuilder.append(BLANK_SPACE);
            titleBuilder.append(context.getString(R.string.group_multi_invitation_title));
            String notifyTitle = titleBuilder.toString();
            String notifyContent = context.getString(R.string.group_multi_invitation_content);
            Intent intent = info.intent;
            if (intent != null) {
                intent.setClass(context, ChatMainActivity.class);
                intent.putExtra(NOTIFY_CONTENT, notifyContent);
            }
            PendingIntent contentIntent = PendingIntent.getActivity(context, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(context);
            builder.setContentTitle(notifyTitle);
            builder.setContentText(notifyContent);
            builder.setContentIntent(contentIntent);
            builder.setSmallIcon(info.icon);
            builder.setWhen(System.currentTimeMillis());
            builder.setAutoCancel(false);
            Logger.v(TAG, "updateGroupInvitationNotification notification checked: "
                    + SettingsFragment.IS_NOTIFICATION_CHECKED.get() + " intent: " + intent);
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
                Notification notification = builder.getNotification();
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.notify(GROUP_INVITATION_TAG, NOTIFICATION_ID_CHAT,
                        notification);
            }
        } else {
            cancelGroupInviteNotification();
        }
        Logger.v(TAG, "updateGroupInvitationNotification exit");
    }

    private void handleFileTransferInvitation(final Context context, Intent invitation) {
        String sessionId = invitation.getStringExtra(SESSION_ID);
		String chatSessionId = invitation.getStringExtra(CHAT_SESSION_ID);
		boolean isGroup = invitation.getBooleanExtra(ISGROUPTRANSFER,false);
        IFileTransferSession session = getFileTransferSession(sessionId);
        if (session == null) {
            Logger.w(TAG, "handleFileTransferInvitation() session is null");
            return;
        }
        long fileSize = invitation.getLongExtra(Utils.FILE_SIZE, 0);
        long availabeSize = Utils.getFreeStorageSize();
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        Logger.d(TAG, "handleFileTransferInvitation() fileSize: " + fileSize + " availabeSize: "
                + availabeSize + " maxFileSize: " + maxFileSize);
        if (fileSize > availabeSize) {
            // check if there is enough storage.
            Handler handler = new Handler(Looper.getMainLooper());
            final String toastText;
            if (availabeSize == -1) {
                toastText = context.getString(R.string.rcse_no_external_storage_for_file_transfer);
            } else {
                toastText = context.getString(R.string.rcse_no_enough_storage_for_file_transfer);
            }
            handler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(context, toastText, Toast.LENGTH_LONG).show();
                }
            });
            try {
                session.rejectSession();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            // check if it exceeds max size.
            if (fileSize >= maxFileSize && maxFileSize != 0) {
                boolean integration = Logger.getIsIntegrationMode();
                Logger.d(TAG, "handleFileTransferInvitation() integration mode: " + integration);
                if (!integration) {
                    showLargeFileNofification(context, invitation, session);
                }
            } else {
                showFileTransferNotification(context, invitation, session);
            }
        }
    }

    private void showLargeFileNofification(Context context, Intent invitation,
            IFileTransferSession session) {
        Logger.d(TAG, "showLargeFileNofification(), invitation() is " + invitation);
        String contact = formatCallerId(invitation);
        try {
            session.rejectSession();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        String notifyTitle = context.getString(R.string.file_size_notification, contact);
        Notification.Builder builder = new Notification.Builder(context);
        builder.setPriority(Notification.PRIORITY_HIGH);
        builder.setSmallIcon(R.drawable.rcs_notify_file_transfer).setTicker(notifyTitle)
                .setContentTitle(notifyTitle);
        Notification notification = builder.getNotification();
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, new Intent(), 0);
        notification.contentIntent = pendingIntent;
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(TAG,
                    "showLargeFileNofification SettingsFragment.IS_NOTIFICATION_CHECKED.get() is "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED.get());
            // Set ringtone
            String ringtone = RcsSettings.getInstance().getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            // Set vibration
            if (RcsSettings.getInstance().isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }
            Logger.i(TAG, "showLargeFileNofification(), the new file transfer invitation title is"
                    + notifyTitle);
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.notify(FILE_TRANSFER, NOTIFICATION_ID_FILE_TRANSFER, notification);
        } else {
            Logger.d(TAG, "showLargeFileNofification(), IS_NOTIFICATION_CHECKED.get() is false");
        }
    }

    private void updateFileTansferNotification(Intent invitation) {
        Logger.d(TAG, "updateFileTansferNotification() entry, invitation is " + invitation);
        int ftContactCount = mFileInvitationInfos.size();
        Intent updateIntent = null;
        Context context = ApiManager.getInstance().getContext();
        Logger.d(TAG, "updateFileTansferNotification() ftContactCount is " + ftContactCount
                + " context: " + context);
        if (ftContactCount <= 0) {
            if (context != null) {
                NotificationManager fileTransferNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                if (fileTransferNotification != null) {
                    fileTransferNotification.cancel(FILE_TRANSFER, NOTIFICATION_ID_FILE_TRANSFER);
                }
            }
        } else {
            int ftCount = 0;
            long totalSize = 0;
            if (invitation == null) {
                Logger.d(TAG, "updateFileTansferNotification(),invitation is null");
                updateIntent = new Intent();
            } else {
                updateIntent = invitation;
            }
            for (int i = 0; i < ftContactCount; i++) {
                ftCount += mFileInvitationInfos.get(i).mInviteNumber;
                totalSize += mFileInvitationInfos.get(i).mFileSize;
            }
            String notifyTitle = null;
            String notifyContent = null;
            String contactName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    mFileInvitationInfos.get(FILE_INVITATION_INDEX_ZERO).mContact);
            if (ftCount == 1) {
                Logger.d(TAG, "updateFileTansferNotification() ftCount is " + ftCount);
                String fileName = mFileInvitationInfos.get(FILE_INVITATION_INDEX_ZERO).mLastFileName;
                notifyTitle = context.getString(R.string.ft_notify_title, contactName);
                notifyContent = context.getString(R.string.ft_notify_content, fileName, Utils
                        .formatFileSizeToString(mFileInvitationInfos.get(0).mFileSize,
                                Utils.SIZE_TYPE_TOTAL_SIZE));
                String title = context.getString(R.string.file_transfer_from) + contactName;
                updateIntent.setClass(context, InvitationDialog.class);
                updateIntent.putExtra(SESSION_ID,
                        mFileInvitationInfos.get(FILE_INVITATION_INDEX_ZERO).mLastSessionId);
                updateIntent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
                updateIntent.putExtra(CONTACT,
                        mFileInvitationInfos.get(FILE_INVITATION_INDEX_ZERO).mContact);
                updateIntent.putExtra(RcsNotification.NOTIFY_CONTENT, notifyContent.toString());
                updateIntent.putExtra(RcsNotification.NOTIFY_SIZE, String.valueOf(totalSize));
                updateIntent.putExtra(RcsNotification.NOTIFY_FILE_NAME, fileName);
                updateIntent.putExtra(RcsNotification.NOTIFY_TITLE, title);
            } else if (ftCount > 1) {
                notifyContent = context.getString(R.string.ft_notify_multiple_content,
                        Utils.formatFileSizeToString(totalSize, Utils.SIZE_TYPE_TOTAL_SIZE));
                if (ftContactCount > 1) {
                    notifyTitle = context.getString(R.string.ft_notify_from_dif_contact_title,
                            ftCount);
                    updateIntent.setClass(context, ChatMainActivity.class);
                } else {
                    notifyTitle = context.getString(R.string.ft_notify_multiple_title, ftCount,
                            contactName);
                    getChatScreen(context, updateIntent, mFileInvitationInfos.get(0).mContact,
                            contactName);
                }
            }
            updateIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, updateIntent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(context);
            builder.setPriority(Notification.PRIORITY_HIGH);
            builder.setSmallIcon(R.drawable.rcs_notify_file_transfer).setContentText(notifyContent)
                    .setContentTitle(notifyTitle).setContentIntent(pendingIntent);
            if (invitation == null) {
                Logger.d(TAG,
                        "updateFileTansferNotification() no need to notify in the notification bar");
            } else {
                builder.setTicker(notifyTitle);
            }
            builder.setAutoCancel(false);
            Notification notification = builder.getNotification();
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get() && invitation != null) {
                Logger.v(TAG,
                        "updateFileTansferNotification SettingsFragment.IS_NOTIFICATION_CHECKED.get() is "
                                + SettingsFragment.IS_NOTIFICATION_CHECKED.get());
                // Set ringtone
                String ringtone = RcsSettings.getInstance().getChatInvitationRingtone();
                if (ringtone != null && ringtone.length() != 0) {
                    notification.sound = Uri.parse(ringtone);
                }
                // Set vibration
                if (RcsSettings.getInstance().isPhoneVibrateForChatInvitation()) {
                    notification.defaults |= Notification.DEFAULT_VIBRATE;
                }
            } else {
                Logger.d(TAG,
                        "updateFileTansferNotification() SettingsFragment.IS_NOTIFICATION_CHECKED.get()"
                                + SettingsFragment.IS_NOTIFICATION_CHECKED.get()
                                + " invitation is " + invitation);
            }
            Logger.i(TAG,
                    "updateFileTansferNotification() notify a new file transfer invitation title is"
                            + notifyTitle + " content is " + notifyContent);
            NotificationManager fileTransferNotification = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (!Logger.getIsIntegrationMode())
            fileTransferNotification.notify(FILE_TRANSFER, NOTIFICATION_ID_FILE_TRANSFER,
                    notification);
        }
    }

    private void showFileTransferNotification(Context context, Intent invitation,
            IFileTransferSession session) {
        Logger.d(TAG, "showFileTransferNotification() entry withe invitation is " + invitation);
		String chatSessionId = null;
		String chatId = null;
		boolean isGroup = false;
        String contactNum = invitation.getStringExtra(CONTACT);
        String sessionId = invitation.getStringExtra(SESSION_ID);
        boolean isAutoAccept = invitation.getBooleanExtra(AUTO_ACCEPT, false);
		byte[] thumbNail = invitation.getByteArrayExtra ("thumbnail");
		String isHttpTransfer = invitation.getStringExtra(ISHTTPTRANSFER);
		
			 chatSessionId = invitation.getStringExtra(CHAT_SESSION_ID);
			 isGroup = invitation.getBooleanExtra(ISGROUPTRANSFER,false);
			 chatId = invitation.getStringExtra(CHAT_ID);

			 
        Logger.d(TAG, "showFileTransferNotification(), isAutoAccept = " + isAutoAccept + "isHttpTransfer:" + isHttpTransfer);
		if(isGroup){
			Logger.d(TAG, "IsGroupTransfer: true");
		}
		else{
			Logger.d(TAG, "IsGroupTransfer: false");
		}
			
        String fileName = null;
        long fileSize = 0;
        try {
            fileName = session.getFilename();
            fileSize = session.getFilesize();
            Logger.i(TAG, "showFileTransferNotification(), the fileName is" + fileName
                    + " and the fileSize is " + fileSize);
        } catch (RemoteException exception) {
            exception.printStackTrace();
        }
        IChatManager chatManager = ModelImpl.getInstance();
        if (chatManager.handleFileTransferInvitation(sessionId, isAutoAccept,isGroup,chatSessionId,chatId)) {
            Logger.d(TAG, "showFileTransferNotification(), chatManager "
                    + "has decided to handle the file transfer invitation itself, "
                    + "so no need to notify the user");
        } else {
        	Logger.d(TAG, "showFileTransferNotification(), needs to notify the user");
            if (isAutoAccept) {
                getChatScreen(context, invitation, contactNum, contactNum);
                addAutoAcceptFileTransferNotification(context, invitation, fileName, contactNum, fileSize);
            } else {
                int size = mFileInvitationInfos.size();
                Logger.d(TAG, "showFileTransferNotification(), size of mFileInvitationInfos is" + size);
                boolean exist = false;
                for (int i = 0; i < size; i++) {
                    FileInvitationInfo fileIvitationInfo = mFileInvitationInfos.get(i);
                    if (fileIvitationInfo.mContact.equals(contactNum)) {
                        fileIvitationInfo.mFileSize += fileSize;
                        fileIvitationInfo.mInviteNumber++;
                        fileIvitationInfo.mLastFileName = fileName;
                        fileIvitationInfo.mLastSessionId = sessionId;
                        exist = true;
                        break;
                    }
                }
                if (!exist) {
                    FileInvitationInfo fileIvitationInfo = new FileInvitationInfo(contactNum,
                            fileSize, fileName, sessionId);
                    mFileInvitationInfos.add(fileIvitationInfo);
                } else {
                    Logger.d(TAG,
                            "showFileTransferNotification the contactNum has exist in the mFileInvitationInfos "
                                    + contactNum);
                }
                updateFileTansferNotification(invitation);
            }
        }
    }

    private void addAutoAcceptFileTransferNotification(Context context, Intent invitation, String fileName, String contact, long fileSize) {
        Logger.d(TAG, "addAutoAcceptFileTransferNotification() entry");
        String notifyTitle = null;
        String notifyContent = null;
        String contactName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(contact);
        notifyTitle = context.getString(R.string.ft_notify_title, contactName);
        notifyContent = context.getString(R.string.ft_notify_content, fileName, Utils
                .formatFileSizeToString(fileSize, Utils.SIZE_TYPE_TOTAL_SIZE));
        
        Notification.Builder builder = new Notification.Builder(context);
        builder.setPriority(Notification.PRIORITY_HIGH);
        builder.setSmallIcon(R.drawable.rcs_notify_file_transfer).setContentText(notifyContent).setContentTitle(
                notifyTitle);
        Notification notification = builder.getNotification();
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, invitation,
                PendingIntent.FLAG_UPDATE_CURRENT);
        notification.contentIntent = pendingIntent;
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(TAG, "addAutoAcceptFileTransferNotification SettingsFragment.IS_NOTIFICATION_CHECKED.get() is "
                    + SettingsFragment.IS_NOTIFICATION_CHECKED.get());
            // Set ringtone
            String ringtone = RcsSettings.getInstance().getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            // Set vibration
            if (RcsSettings.getInstance().isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }

            Logger.i(TAG, "addAutoAcceptFileTransferNotification(), the new file transfer invitation title is"
                    + notifyTitle);
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if(!Logger.getIsIntegrationMode())
            notificationManager.notify(FILE_TRANSFER, NOTIFICATION_ID_FILE_TRANSFER, notification);
        } else {
            Logger.d(TAG, "addAutoAcceptFileTransferNotification(), IS_NOTIFICATION_CHECKED.get() is false");
        }
    }

    private void getChatScreen(Context context, Intent invitation, String number, String name) {
        One2OneChat chat = null;

        if (null != number) {
            Participant contact = new Participant(number, name);
            List<Participant> participantList = new ArrayList<Participant>();
            participantList.add(contact);
            chat = (One2OneChat) ModelImpl.getInstance().addChat(participantList, null,null);
            if (null != chat) {
                invitation.setClass(context, ChatScreenActivity.class);
                invitation
                        .putExtra(ChatScreenActivity.KEY_CHAT_TAG, (ParcelUuid) chat.getChatTag());
            } else {
                Logger.e(TAG, "getChatScreen(), chat is null!");
            }
        } else {
            Logger.e(TAG, "getChatScreen(), fileSessionSession is null");
        }
    }

    /**
     * Start to handle invitation
     * 
     * @param context The Context instance
     * @param intent The invitation Intent instance
     */
    public static synchronized void handleInvitation(Context context, Intent intent) {
        Logger.d(TAG, "handleInvitation() entry with intend action is " + intent.getAction());
        String action = intent.getAction();
        if (action == null) {
            return;
        }
        String contact = formatCallerId(intent);
        intent.putExtra(DISPLAY_NAME, contact);

        if (MessagingApiIntents.CHAT_INVITATION.equalsIgnoreCase(action)) {
            IChatSession chatSession = RcsNotification.getInstance().getChatSession(intent);
            if (chatSession == null) {
                Logger.d(TAG, "The chat session is null");
                return;
            }
            try {
                if (chatSession.isGroupChat()) {
                    if (!(intent.getBooleanExtra("isGroupChatExist", false))) {
                        boolean autoAccept = intent.getBooleanExtra(
                                AUTO_ACCEPT, false);
                        String chatId = intent
                                .getStringExtra(RcsNotification.CHAT_ID);
                        RcsNotification.getInstance()
                                .handleGroupChatInvitation(context, intent,
                            autoAccept);
                    }
                } else {
                    handleChatInvitation(context, intent);
                }
            } catch (RemoteException e) {
                Logger.d(TAG, "IChatSession operation error");
                e.printStackTrace();
            }
        } else if (MessagingApiIntents.FILE_TRANSFER_INVITATION.equalsIgnoreCase(action)) {
            getInstance().handleFileTransferInvitation(context, intent);
        } else if (RichCallApiIntents.IMAGE_SHARING_INVITATION.equalsIgnoreCase(action)) {
            handleImageSharingInvitation(context, intent);
        } else if (RichCallApiIntents.VIDEO_SHARING_INVITATION.equalsIgnoreCase(action)) {
            handleVideoSharingInvitation(context, intent);
        } else if (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) {
            handleChatInvitation(context, intent);
        }
        Logger.v(TAG, "handleInvitation() exit");
    }

    private static void handleChatInvitation(Context context, Intent invitation) {
        Logger.v(TAG, "handleChatInvitation entry");
        ArrayList<InstantMessage> messages = new ArrayList<InstantMessage>();
        InstantMessage msg = invitation.getParcelableExtra(FIRST_MESSAGE);
        if (msg != null) {
            messages.add(msg);
        }
        invitation.putParcelableArrayListExtra(MESSAGES, messages);
        ModelImpl.getInstance().handleInvitation(invitation, false);
        Logger.v(TAG, "handleChatInvitation exit");
    }

    private static void handleImageSharingInvitation(Context context, Intent invitation) {
    }

    private static void handleVideoSharingInvitation(Context context, Intent invitation) {
    }

    public void onReceiveMessageInBackground(Context context, Object newTag,
            InstantMessage message, List<Participant> participantList, final int id) {
        onReceiveMessageInBackground(context, newTag, message, participantList, id, false);
    }

    /**
     * Showing a notifications of a new incoming message when the chat window is
     * in background
     * 
     * @param context The context
     * @param msg The new incoming message
     */
    public void onReceiveMessageInBackground(Context context, Object newTag,
            InstantMessage message, List<Participant> participantList, final int id,
            boolean isStoreAndFoward) {
        Logger.d(TAG,
                "onReceiveMessageInBackground(), the newTag is " + newTag + " isStoreAndFoward "
                        + isStoreAndFoward + "messageid is " + message.getMessageId());
        if (mIsInChatMainActivity) {
            Logger.e(TAG, "onReceiveMessageInBackground() mIsInChatMainActivity is true)");
            return;
        }
        String ticker = null;
        String contact = getRemoteContact(message);
        if (null != contact) {
            if (PhoneUtils.isANumber(contact)) {
                contact = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(contact);
            } else {
                Logger.d(TAG, "onReceiveMessageInBackground(), contact is null");
            }
        }
        if (contact != null) {
            ticker = contact + ":" + message.getTextMessage();
        }
        String notifyTitle = contact;
        if (participantList != null && participantList.size() > 1) {
            notifyTitle = getGroupNotificationTitle(participantList);
        }
        String description = message.getTextMessage();

        if (mUnReadMessagesChatInfos.containsKey(newTag)) {
            ChatReceivedInfo chatInfo = mUnReadMessagesChatInfos.get(newTag);
            if (chatInfo != null) {
                chatInfo.updateMessage(notifyTitle);
            }
        } else {
            mUnReadMessagesChatInfos.put(newTag, new ChatReceivedInfo(notifyTitle, description));
        }
        if (!isStoreAndFoward) {
            Logger.d(TAG, "onReceiveMessageInBackground(), isStoreAndFoward is " + isStoreAndFoward);
            updateReceiveMessageNotification(ticker, true);
        } else if (!RcsNotification.getInstance().sIsStoreAndForwardMessageNotified) {
            Logger.d(TAG, "onReceiveMessageInBackground(), "
                    + "sIsStoreAndForwardMessageNotified is false ");
            updateReceiveMessageNotification(ticker, true);
            RcsNotification.getInstance().sIsStoreAndForwardMessageNotified = true;
        } else {
            Logger.d(TAG, "onReceiveMessageInBackground(), "
                    + "sIsStoreAndForwardMessageNotified is true ");
            updateReceiveMessageNotification(ticker, false);
        }
    }

    private void updateReceiveMessageNotification(Context context, int requestCode, Intent intent,
            String ticker, String notifyTitle, String description, int icon,
            boolean isNewMessageNotification) {
        Logger.d(TAG,
                "updateReceiveMessageNotification() entry eight parameters isNewMessageNotification "
                        + isNewMessageNotification);
        PendingIntent contentIntent = PendingIntent.getActivity(context, requestCode, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        Notification.Builder builder = new Notification.Builder(context);
        builder.setContentTitle(notifyTitle);
        builder.setContentText(description);
        builder.setContentIntent(contentIntent);
        if (isNewMessageNotification) {
            builder.setTicker(ticker);
        }
        builder.setSmallIcon(icon);
        builder.setWhen(System.currentTimeMillis());
        builder.setAutoCancel(true);

        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(TAG,
                    "updateReceiveMessageNotification SettingsFragment.IS_NOTIFICATION_CHECKED.get() is "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED.get());
            if (isNewMessageNotification) {
                Logger.v(TAG, "updateReceiveMessageNotification isNewMessageNotification is "
                        + isNewMessageNotification);
                // Set ringtone
                String ringtone = RcsSettings.getInstance().getChatInvitationRingtone();
                if (ringtone != null && ringtone.length() != 0) {
                    Logger.v(TAG, "updateReceiveMessageNotification set rintone");
                    builder.setSound(Uri.parse(ringtone));
                } else {
                    Logger.v(TAG, "updateReceiveMessageNotification not set rintone");
                }
                // Set vibrate
                if (RcsSettings.getInstance().isPhoneVibrateForChatInvitation()) {
                    Logger.v(TAG, "updateReceiveMessageNotification set vibarate");
                    builder.setDefaults(Notification.DEFAULT_VIBRATE);
                } else {
                    Logger.v(TAG, "updateReceiveMessageNotification not set vibarate");
                }
            } else {
                Logger.v(TAG, "updateReceiveMessageNotification isNewMessageNotification is "
                        + isNewMessageNotification);
            }
            Notification notification = builder.getNotification();
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager
                    .notify(UNREAD_MESSAGE, NOTIFICATION_ID_UNREAD_MESSAGE, notification);
        } else {
            Logger.v(TAG,
                    "updateReceiveMessageNotification SettingsFragment.IS_NOTIFICATION_CHECKED.get() is false");
        }
    }

    private void updateReceiveMessageNotification(String ticker, boolean isNewMessageNotification) {
        Logger.d(TAG,
                "updateReceiveMessageNotification() two parameters entry isNewMessageNotification is "
                        + isNewMessageNotification);
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            if (mUnReadMessagesChatInfos.size() == 0) {
                Logger.d(TAG, "updateReceiveMessageNotification() the size is 0");
                NotificationManager notificationManager = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                if (notificationManager != null) {
                    notificationManager.cancel(UNREAD_MESSAGE, NOTIFICATION_ID_UNREAD_MESSAGE);
                }
            } else {
                String notifyTitle = null;
                String description = null;
                Intent intent = new Intent();
                intent.setClass(context, ChatScreenActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

                if (mUnReadMessagesChatInfos.size() > 1) {
                    Logger.d(TAG, "updateReceiveMessageNotification() the size >1");
                    intent.setClass(context, ChatMainActivity.class);
                    intent.putExtra(FORCE_SCROLLTO_CHAT, true);

                    notifyTitle = context.getString(R.string.notification_multipleChats_title,
                            Integer.toString(mUnReadMessagesChatInfos.size()));

                    int count = 0;
                    Collection<ChatReceivedInfo> chatInfos = mUnReadMessagesChatInfos.values();
                    Iterator<ChatReceivedInfo> iterator = chatInfos.iterator();
                    while (iterator.hasNext()) {
                        ChatReceivedInfo chatInfo = iterator.next();
                        count += chatInfo.getMessageNum();
                    }
                    if (count > 1) {
                        description = context.getString(R.string.notification_multiple,
                                Integer.toString(count));
                    }
                } else if (mUnReadMessagesChatInfos.size() == 1) {
                    Logger.d(TAG, "updateReceiveMessageNotification() the size = 1");
                    Object tag = mUnReadMessagesChatInfos.keys().nextElement();
                    if (tag == null) {
                        Logger.e(TAG, "updateReceiveMessageNotification the chat tag is null");
                        return;
                    }
                    if (tag instanceof UUID) {
                        ParcelUuid parcelUuid = new ParcelUuid((UUID) tag);
                        intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, parcelUuid);

                    } else {
                        intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, (ParcelUuid) tag);
                    }
                    ChatReceivedInfo chatInfo = mUnReadMessagesChatInfos.get(tag);
                    if (chatInfo == null) {
                        Logger.e(TAG, "updateReceiveMessageNotification return chatInfo is null");
                        return;
                    }
                    notifyTitle = chatInfo.getDisplayName();
                    description = chatInfo.getFirstMessage();
                    int count = chatInfo.getMessageNum();
                    if (count > 1) {
                        description = context.getString(R.string.notification_multiple,
                                Integer.toString(count));
                    }
                }

                int requestCode = 0;
                int icon = R.drawable.rcs_notify_chat_message;
                Logger.d(TAG,
                        "updateReceiveMessageNotification() mode: " + Logger.getIsIntegrationMode());
                if (!Logger.getIsIntegrationMode()) {
                    updateReceiveMessageNotification(context, requestCode, intent, ticker,
                            notifyTitle, description, icon, isNewMessageNotification);
                }
            }
        } else {
            Logger.d(TAG, "updateReceiveMessageNotification() the context is null");
        }
    }

    /**
     * cancel one message notification when user open this chat window(one2one
     * or group chat).
     * 
     * @param newTag
     */
    public void cancelReceiveMessageNotification(Object newTag) {
        if (newTag == null) {
            Logger.d(TAG, "cancelReceiveMessageNotification chat tag is null");
            return;
        }
        if (mUnReadMessagesChatInfos != null && mUnReadMessagesChatInfos.containsKey(newTag)) {
            mUnReadMessagesChatInfos.remove(newTag);
            String ticker = null;
            boolean isNewMessageNotification = false;
            updateReceiveMessageNotification(ticker, isNewMessageNotification);
        } else {
            Logger.d(TAG,
                    "cancelReceiveMessageNotification mUnReadMessagesChatInfos didn't contain this chat tag");
        }
    }

    /**
     * Cancel a previously shown notification with a special notification ID. If
     * it's transient, the view will be hidden. If it's persistent, it will be
     * removed from the status bar.
     */
    public void cancelNotification() {
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (notificationManager != null) {
                Logger.d(TAG, "cancelNotification() entry");
                notificationManager.cancel(UNREAD_MESSAGE, NOTIFICATION_ID_UNREAD_MESSAGE);
                notificationManager.cancel(FILE_TRANSFER, NOTIFICATION_ID_FILE_TRANSFER);
            } else {
                Logger.e(TAG, "cancelNotification the notificationManager is null");
            }
        } else {
            Logger.e(TAG, "cancelGroupInviteNotification the context is null");
        }
        mUnReadMessagesChatInfos.clear();
        mFileInvitationInfos.clear();
        Logger.d(TAG, "cancelNotification() exit");
    }

    private String getGroupNotificationTitle(List<Participant> participantList) {
        String notifyTitle = ChatFragment.getParticipantsName(participantList
                .toArray(new Participant[1]));
        char[] chars = notifyTitle.toCharArray();
        int length = 0;
        int index = 0;
        for (int i = 0; i < chars.length; i++) {
            char c = chars[i];
            if (c > NOTIFICATION_CHAR_LENGTH_CONDITION) {
                length += NOTIFICATION_CHAR_LENGTH_DOUBLE_BYTE;
            } else {
                length += NOTIFICATION_CHAR_LENGTH_SINGLE_BYTE;
            }
            if (length >= NOTIFICATION_TITLE_LENGTH_MAX) {
                Logger.d(TAG, "getGroupNotificationTitle(), length > maxlenght will cut the string");
                index = i;
                break;
            }
        }
        if (index > 0) {
            notifyTitle = notifyTitle.substring(0, index) + "...(" + participantList.size() + ")";
        } else {
            notifyTitle = notifyTitle + " (" + participantList.size() + ")";
        }
        return notifyTitle;
    }

    private static String formatCallerId(Intent invitation) {
        String number = invitation.getStringExtra(CONTACT);
        Logger.d(TAG, "formatCallerId, number is " + number);
        String displayName;
        if (null != number) {
            String tmpContact = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                    number);
            if (tmpContact != null) {
                displayName = tmpContact;
            } else {
                displayName = number;
            }
        } else {
            displayName = EMPTY_STRING;
            Logger.e(TAG, "formatCallerId, number is null!");
        }
        Logger.d(TAG, "formatCallerId, displayName is " + displayName);
        return displayName;
    }

    private static String getRemoteContact(InstantMessage message) {
        Logger.d(TAG, "formatRemote() entry, message.getRemote() is " + message.getRemote());
        String remote = message.getRemote();
        String displayName = PhoneUtils.extractDisplayNameFromUri(remote);
        if ((displayName != null) && (displayName.length() > 0)) {
            return displayName;
        }
        String number = PhoneUtils.extractNumberFromUri(remote);
        return number;
    }

    public static class GroupInvitationInfo {
        public Context context;
        public String sender;
        public int icon;
        public String notifyTitle;
        public String notifyInfo;
        public Intent intent;
    }

    public static class FileInvitationInfo {
        String mContact;
        String mLastSessionId;
        int mInviteNumber = 1;
        long mFileSize = 0;
        String mLastFileName;

        public FileInvitationInfo(String contact, long filesize, String filename, String sessionId) {
            mContact = contact;
            mFileSize = filesize;
            mLastFileName = filename;
            mLastSessionId = sessionId;
        }
    }

    private static class ChatReceivedInfo {
        private String mDisplayName;
        private String mFirstMessage;
        private int mMessageNum;

        public ChatReceivedInfo(String displayName, String firstMessage) {
            this.mDisplayName = displayName;
            this.mFirstMessage = firstMessage;
            this.mMessageNum = 1;
        }

        public String getDisplayName() {
            return mDisplayName;
        }

        public String getFirstMessage() {
            return mFirstMessage;
        }

        public int getMessageNum() {
            return mMessageNum;
        }

        public void updateMessage(String displayName) {
            this.mDisplayName = displayName;
            this.mMessageNum++;
        }
    }
}
