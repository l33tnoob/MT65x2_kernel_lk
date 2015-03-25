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

package com.mediatek.rcse.plugin.message;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.Telephony.Sms;
import android.text.TextUtils;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.activities.widgets.PhotoLoaderManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;
import com.mediatek.rcse.service.binder.ChatEventStructForBinder;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.List;
import java.util.TreeSet;
import java.util.UUID;

/**
 * Plugin Group Chat Window
 */
public class PluginGroupChatWindow extends IRemoteGroupChatWindow.Stub implements WindowTagGetter,
        PhotoLoaderManager.OnPhotoChangedListener {

    private static final String TAG = "PluginGroupChatWindow";
    public static final String GROUP_CONTACT_STRING_BEGINNER = "7---";
    // This is used for group chat inviation in sms db address field
    public static final String GROUP_CHAT_INVITATION_TAG_BEGINNER = "11111111";
    public static final int GROUP_CHAT_INVITATION_IPMSG_ID = 10;
    public static final int GROUP_CHAT_CREATE_IPMSG_ID = 11;
    public static final String MESSAGES = "messages";
    public static final String GROUP_CHAT_CONTACT = "plugin group chat contact";
    public static final int GROUP_CHAT_INVITATION_SEEN = 1;
    public static final String MMS_PACKAGE_STRING = "com.android.mms";
    public static final String MMS_BOOT_ACTIVITY_STRING = "com.android.mms.ui.BootActivity";
    private static final String GROUP_CONTACT_TAG_DELIMITER = "-";
    private final String mWindowTag;
    private final PluginChatWindow mChatWindow;
    private final String mContactString;
    private final short mGroupId;
    public boolean mNewGroupCreate = false;
    public String mSubject = "";
    private Bitmap mAvatarBitMap = null;
    private TreeSet<String> mKey = new TreeSet<String>();
    private AsyncListener mCurrentListener = null;
    private static final String[] PROJECTION_ADDRESS = {
        Sms.ADDRESS
    };
    private static final String SELECTION_THREAD_ID = Sms.THREAD_ID + "=?";
    private static final String SELECTION_SMS_ADDRESS = Sms.ADDRESS + "=?";
    private IpMessageManager mMessageManager;

    /**
     * get the combined Bitmap.
     */
    public Bitmap getAvatarBitmap() {
        return mAvatarBitMap;
    }
    
    /**
     * get the group chat displays names.
     */
    public String getDisplayName() {
        Logger.d(TAG, "getDisplayName() entry mKey is " + mKey);
        String name = ChatFragment.getParticipantsName(mKey.toArray(new String[1]));
        Logger.d(TAG, "getDisplayName() name is " + name);
        return name;
    }

    private class AsyncListener implements OnLoadImageFinishListener {
        private Object mKey = null;

        AsyncListener(Object key) {
            mKey = key;
        }

        @Override
        public void onLoadImageFished(Bitmap image) {
            Logger.d(TAG, "onLoadImageFished() entry");
            mAvatarBitMap = image;
            if (this == mCurrentListener) {
                mCurrentListener = null;
            }
        }

        public void destroy() {
            ImageLoader.interrupt(mKey);
        }
    }
    
    public PluginGroupChatWindow(String windowTag, IpMessageManager messageManager, List<ParticipantInfo> participantList) {
        mGroupId = (short) windowTag.hashCode();
        mWindowTag = windowTag;
        mMessageManager = messageManager;
        mChatWindow = new PluginChatWindow(mMessageManager);
        mContactString = GROUP_CONTACT_STRING_BEGINNER + windowTag;
        Logger.d(TAG, "PluginGroupChatWindow() constructor, windowTag: " + windowTag
                + " participantList: " + participantList);
        TreeSet<String> nums = new TreeSet<String>();
        for (ParticipantInfo parInfo : participantList) {
            nums.add(parInfo.getContact());
        }
        mKey = nums;
        PhotoLoaderManager.addListener(this);
        updateAvatar();
    }
    
    private Bitmap requestImage() {
        Logger.d(TAG, "requestImage() entry mKey is " + mKey);
        if (mCurrentListener == null) {
            mCurrentListener = new AsyncListener(mKey);
        }
        return ImageLoader.requestImage(mKey, mCurrentListener);
    }

    @Override
    public void addgroupSubject(String subject)
    {
    	Logger.v(TAG, "addgroupSubject(), subject = " + subject +"contactString =" + mContactString);
    	mChatWindow.addgroupSubject(subject);
    	mNewGroupCreate = true;
    	mSubject = subject;    	
    }

    @Override
    public int addChatEventInformation(ChatEventStructForBinder chatEventStruct)
            throws RemoteException {
        Logger.v(TAG, "addChatEventInformation(), chatEventStruct = " + chatEventStruct);
        return 0;
    }

    @Override
    public void setIsComposing(boolean isComposing, Participant participant) throws RemoteException {
        Logger.v(TAG, "setIsComposing(), isComposing = " + isComposing + " participant = "
                + participant);
    }
    
    @Override
    public void updateChatStatus(int status) throws RemoteException {
        Logger.v(TAG, "setIsRejoining(), status = " + status);
    }

    @Override
    public void updateParticipants(List<ParticipantInfo> participants) throws RemoteException {
        Logger.v(TAG, "updateParticipants(), participants = " + participants);
        TreeSet<String> nums = new TreeSet<String>();
        for (ParticipantInfo parInfo : participants) {
            nums.add(parInfo.getContact());
        }
        mKey = nums;
        updateAvatar();
        updateGroupInfo();
    }

    @Override
    public void addLoadHistoryHeader(boolean showLoader) throws RemoteException {
    }

    @Override
    public IRemoteReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead)
            throws RemoteException {
        if (null != message) {
            Logger.w(TAG, "addReceivedMessage() message id is " + message.getMessageId()
                    + ", isRead:" + isRead);
            if(mNewGroupCreate)
            	mNewGroupCreate = false;
            InstantMessage newMessage = new InstantMessage(message.getMessageId(), mContactString,
                    message.getTextMessage(), message.isImdnDisplayedRequested());
            return mChatWindow.addReceivedMessage(newMessage, isRead);
        } else {
            Logger.w(TAG, "addReceivedMessage() message is null, mContactString: " + mContactString);
            return null;
        }
    }

    @Override
    public IRemoteSentChatMessage addSentMessage(InstantMessage message, int messageTag)
            throws RemoteException {
        if (null != message) {
            InstantMessage newMessage = new InstantMessage(message.getMessageId(), mContactString,
                    message.getTextMessage(), message.isImdnDisplayedRequested());
            if(mNewGroupCreate)
            {
				mChatWindow.setmNewGroupCreate(true);
                mNewGroupCreate = false;
            }
            return mChatWindow.addSentMessage(newMessage, messageTag);
        } else {
            Logger.w(TAG, "addSentMessage() message is null, mContactString: " + mContactString);
            return null;
        }
    }

    @Override
    public IRemoteSentChatMessage getSentChatMessage(String messageId) throws RemoteException {
        return null;
    }

    @Override
    public void removeAllMessages() throws RemoteException {
        Logger.d(TAG, "removeAllMessages() entry");
        final String[] args = {
            mContactString
        };
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                .getContentResolver();
        contentResolver.delete(PluginUtils.SMS_CONTENT_URI, SELECTION_SMS_ADDRESS, args);
        Logger.d(TAG, "removeAllMessages() exit address is " + mContactString);
    }

    @Override
    public void updateAllMsgAsRead() throws RemoteException {
        Logger.d(TAG, "updateAllMsgAsRead() entry mContactString is " + mContactString);
        ContentValues values = new ContentValues();
        values.put(Sms.READ, PluginUtils.STATUS_IS_READ);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                .getContentResolver();
        contentResolver.update(PluginUtils.SMS_CONTENT_URI, values, Sms.ADDRESS + " = \"" + mContactString
                + "\"" + " AND " + Sms.READ + " = " + PluginUtils.STATUS_IS_NOT_READ, null);
    }

    @Override
    public String getWindowTag() {
        return mWindowTag;
    }
    
    /**
     * get the contactstring of the group chat window.
     * @return contactstring of the group chat window.
     */
    public String getContactString() {
        Logger.d(TAG, "getContactString() entry mContactString is " + mContactString);
        return mContactString;
    }

    /**
     * Get the group id of the group chat window.
     * 
     * @return The group id of the group chat window.
     */
    public short getGroupId() {
        Logger.d(TAG, "getGroupId() entry getGroupId is " + mGroupId);
        return mGroupId;
    }

    @Override
    public void onPhotoChanged() {
        Logger.d(TAG, "onPhotoChanged() entry");
        updateAvatar();
        updateGroupInfo();
    }
    
    public void onDestroy() {
        Logger.d(TAG, "onDestroy() entry");
        PhotoLoaderManager.removeListener(this);
    }

    /**
     * Generate group chat invitaion contact which will insert into sms db
     * 
     * @param sessionId The session id
     * @return Group chat invitation contact
     */
    public static String generateGroupChatInvitationContact(String sessionId) {
        Logger.d(TAG, "generateGroupChatInvitationContact entry");
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        String tag = parcelUuid.toString();
        String items[] = tag.split(GROUP_CONTACT_TAG_DELIMITER);
        // Modify the first item to mark group chat invitation
        int firstIndex = 0;
        int lastIndex = items.length - 1;
        items[firstIndex] = GROUP_CHAT_INVITATION_TAG_BEGINNER;
        items[lastIndex] = sessionId;
        // Rebuild the new tag
        StringBuilder sb = new StringBuilder();
        for (String item : items) {
            sb.append(item);
            sb.append(GROUP_CONTACT_TAG_DELIMITER);
        }
        // Remove the last "-"
        sb.deleteCharAt(sb.length() - 1);
        String groupChatInvitationTag = sb.toString();
        String contact = GROUP_CONTACT_STRING_BEGINNER + groupChatInvitationTag;
        Logger.d(TAG, "generateGroupChatInvitationContact exit contact is " + contact);
        return contact;
    }

    /**
     * Check a group chat is an invitation or not
     * 
     * @param Contact Group chat contact string
     * @return true or false
     */
    public static boolean isGroupChatInvitation(String contact) {
        Logger.d(TAG, "isGroupChatInvitation() entry contact is " + contact);
        boolean ret = false;
        if (null != contact) {
            if (isContactContainsInvitationBeginner(contact)) {
                ret = true;
            } else {
                Logger.d(TAG, "isGroupChatInvitation() contact is not group invitation");
            }
        } else {
            Logger.w(TAG, "isGroupChatInvitation() contact is null");
        }
        Logger.d(TAG, "isGroupChatInvitation() exit ret is " + ret);
        return ret;
    }

    /**
     * Get group chat contact from mms db address field
     * 
     * @param context The context
     * @param threadId Thread id in mms db
     * @return Group chat contact string
     */
    public static String getContactByThreadId(Context context, long threadId) {
        Logger.d(TAG, "getContactByThreadId() entry");
        String contact = null;
        if (threadId > -1 && null != context) {
            ContentResolver contentResolver = context.getContentResolver();
            Cursor cursor = null;
            try {
                String[] args = {
                    Long.toString(threadId)
                };
                cursor = contentResolver.query(PluginUtils.SMS_CONTENT_URI, PROJECTION_ADDRESS,
                        SELECTION_THREAD_ID, args, Sms.DEFAULT_SORT_ORDER);
                if (cursor.moveToFirst()) {
                    contact = cursor.getString(cursor.getColumnIndex(Sms.ADDRESS));
                } else {
                    Logger.w(TAG, "getContactByThreadId() empty cursor");
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            Logger.d(TAG, "getContactByThreadId() invalid thread id or context: " + context);
        }
        Logger.d(TAG, "getContactByThreadId() exit contact is " + contact);
        return contact;
    }

    private static boolean isContactContainsInvitationBeginner(String contact) {
        Logger.d(TAG, "isContactContainsInvitation() entry contact is " + contact);
        boolean ret = false;
        if (contact.contains(GROUP_CONTACT_STRING_BEGINNER + GROUP_CHAT_INVITATION_TAG_BEGINNER)) {
            ret = true;
        } else {
            Logger.d(TAG, "isContactContainsInvitation() entry contact is " + contact);
        }
        return ret;
    }

    /**
     * Get session id from group chat contact
     * @param contact Group chat contact
     * @return Session id
     */
    public static String getSessionIdByContact(String contact) {
        Logger.d(TAG, "getSessionIdByContact() entry contact is " + contact);
        String sessionId = null;
        if (null != contact) {
            String items[] = contact.split(GROUP_CONTACT_TAG_DELIMITER);
            int lastIndex = items.length - 1;
            sessionId = items[lastIndex];
        } else {
            Logger.w(TAG, "getSessionIdByContact() contact is null");
        }
        Logger.d(TAG, "getSessionIdByContact() exit sessionId is " + sessionId);
        return sessionId;
    }

    /**
     * Get IChatSession instance by session id
     * 
     * @param sessionId Session id
     * @return IChatSession instance
     */
    public static IChatSession getChatSession(String sessionId) {
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.i(TAG, "getChatSession ApiManager instance is null");
            return null;
        }
        MessagingApi messageApi = instance.getMessagingApi();
        if (messageApi == null) {
            Logger.d(TAG, "getChatSession MessageingApi instance is null");
            return null;
        }
        IChatSession chatSession = null;
        try {
            chatSession = messageApi.getChatSession(sessionId);
            return chatSession;
        } catch (ClientApiException e) {
            Logger.e(TAG, "getChatSession Get chat session failed");
            e.printStackTrace();
            chatSession = null;
            return chatSession;
        }
    }
    
    
    /**
     * Remove all group chat invitations in mms db
     */
    public static void removeAllGroupChatInvitationsInMms() {
        Logger.v(TAG, "removeAllGroupChatInvitationsInMms entry");
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
            .getContentResolver();
        contentResolver.delete(PluginUtils.SMS_CONTENT_URI, Sms.ADDRESS + " LIKE '"
                + GROUP_CONTACT_STRING_BEGINNER + GROUP_CHAT_INVITATION_TAG_BEGINNER + "%'", null);
    }
    
    /**
     * Remove one all group chat invitations in mms db by contact
     * @param contact
     */
    public static void removeGroupChatInvitationInMms(String contact) {
        Logger.d(TAG, "removeGroupChatInvitationInMms contact is" + contact);
        Intent intent = new Intent();
        intent.putExtra("removeFromMms", true);
        intent.putExtra("contact", contact);        
        intent.setAction(IpMessageConsts.JoynGroupInvite.ACTION_GROUP_IP_INVITATION);      
        AndroidFactory.getApplicationContext().sendStickyBroadcast(intent);
    }

    /**
     * Get group chat invitation info from mms db
     * 
     * @param threadId The invitation thread id in mms
     * @return Group chat invitatoin info
     */
    public static String getGroupChatInvitationInfoInMms(long threadId) {
        Logger.d(TAG, "getGroupChatInvitationInfoInMms() entry threadId is " + threadId);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                .getContentResolver();
        String[] selectionArg = {
            String.valueOf(threadId)
        };
        String info = null;
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(PluginUtils.SMS_CONTENT_URI, null, Sms.THREAD_ID + "=?",
                    selectionArg, null);
            if (cursor.moveToFirst()) {
                info = cursor.getString(cursor.getColumnIndex(Sms.BODY));
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        Logger.d(TAG, "getGroupChatInvitationInfoInMms() exit info is " + info);
        return info;
    }
    
    private void updateAvatar() {
        Bitmap bitMap = requestImage();
        if (bitMap != null) {
            mAvatarBitMap = bitMap;
        } else {
            Logger.d(TAG, "updateAvatar() bitMap is null");
        }
    }

    private void updateGroupInfo() {
        Logger.d(TAG, "updateGroupInfo()");
        Intent intent = new Intent();
        intent.setAction(IpMessageConsts.UpdateGroup.UPDATE_GROUP_ACTION);
        intent.putExtra(IpMessageConsts.UpdateGroup.GROUP_ID, (int) mGroupId);
        Logger.d(TAG, "updateGroupInfo(), notify notification manager with intent" + intent
                + " and the extra is " + mGroupId);
        IpNotificationsManager.notify(intent);
    }
    @Override
    public void setFileTransferEnable(int reason) throws RemoteException {
        Logger.v(TAG, "setFileTransferEnable() reason = " + reason);
    }
    
    @Override
    public IRemoteFileTransfer addReceivedFileTransfer(FileStructForBinder file, boolean fileAutoaccept) throws RemoteException {
        Logger.v(TAG, "addReceivedFileTransfer() file = " + file);
        /*PluginChatWindowFileTransfer pluginChatWindowFileTransfer = new PluginChatWindowFileTransfer(file,
                PluginUtils.INBOX_MESSAGE, mParticipant.getContact());
        String fileTransferString = PluginUtils.getStringInRcse(R.string.file_transfer_title);
        int ipMsgId = PluginUtils.findIdInRcseDb(file.fileTransferTag);
        String remote = mParticipant.getContact();
        if (-1 < ipMsgId) {
            Logger.d(TAG, "addReceivedFileTransfer(), ipMsgId = " + ipMsgId);
            long idInMms = PluginUtils.getIdInMmsDb(ipMsgId);
            if (idInMms == -1) {
                Logger.d(TAG, "addReceivedFileTransfer(), it is a new message, idInMms = " + idInMms);
                if (TextUtils.isEmpty(remote)) {
                    Logger.w(TAG, "storeMessageInDatabase() invalid remote: " + remote);
                }
                final String contact = PhoneUtils.extractNumberFromUri(remote);
                idInMms = PluginUtils.insertDatabase(fileTransferString, contact, ipMsgId, PluginUtils.INBOX_MESSAGE);
                pluginChatWindowFileTransfer.storeInCache(idInMms);

                Intent intent = new Intent();
                intent.setAction(IpMessageConsts.NewMessageAction.ACTION_NEW_MESSAGE);
                intent.putExtra(IpMessageConsts.NewMessageAction.IP_MESSAGE_KEY, idInMms);
                AndroidFactory.getApplicationContext().sendBroadcast(intent);
            } else {
                pluginChatWindowFileTransfer.storeInCache(idInMms);
                Logger.d(TAG, "addReceivedFileTransfer(), already in mms database, no need to insert!");
            }

        } else {
            Logger.w(TAG, "addReceivedFileTransfer(), is not in rcse db!");
        }*/
        return null;
    }

    @Override
    public IRemoteFileTransfer addSentFileTransfer(FileStructForBinder file) throws RemoteException {
        Logger.v(TAG, "addSentFileTransfer() file = " + file);
        /*PluginChatWindowFileTransfer pluginChatWindowFileTransfer = new PluginChatWindowFileTransfer(file,
                PluginUtils.OUTBOX_MESSAGE, mParticipant.getContact());
        Long fileTransferIdInMms;
        String fileTransferString = PluginUtils.getStringInRcse(R.string.file_transfer_title);
        if (!file.fileTransferTag.contains("-")) {
            fileTransferIdInMms = PluginUtils.storeMessageInDatabase(file.fileTransferTag, fileTransferString,
                    mParticipant.getContact(), PluginUtils.OUTBOX_MESSAGE);
            pluginChatWindowFileTransfer.storeInCache(fileTransferIdInMms);
        } else {
            Logger.d(TAG, "Already in database!");
        }
        Logger.d(TAG, "addSentFileTransfer(), pluginChatWindowFileTransfer = " + pluginChatWindowFileTransfer);*/
        return null;
    }
    
}
