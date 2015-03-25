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

import android.content.Intent;
import android.os.RemoteException;
import android.text.TextUtils;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.plugin.message.PluginChatWindowManager.WindowTagGetter;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;
import com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * Plugin One2One Chat Window
 */
public class PluginOne2OneChatWindow extends IRemoteOne2OneChatWindow.Stub implements WindowTagGetter {

    private static final String TAG = "PluginOne2OneChatWindow";
    public static final String ACTION_IM_STATUS = "com.mediatek.mms.ipmessage.IMStatus";
    private PluginChatWindow mPluginChatWindow;
    private Participant mParticipant;
    private final String mWindowTag;
    private boolean mIsComposing = false;
    private boolean mIsOffline = true;
    private static final int IS_ONLINE_STATUS = 0;
    private static final int IS_TYPINT_STATUS = 1;
    private IpMessageManager mMessageManager;

    /**
     * This class defines the status of a Rcse contact.
     */
    public static final class RcseContactStatus {
        /**
         * Rcse contact state : not registered
         */
        public static final int OFFLINE = 0;
        /**
         * Rcse contact state : registered
         */
        public static final int ONLINE = 1;
        /**
         * Rcse contact state : typing
         */
        public static final int TYPING = 2;

        /**
         * Rcse contact state : not typing
         */
        public static final int STOP_TYPING = 3;
    }

    public PluginOne2OneChatWindow(Participant participant, String windowTag, IpMessageManager messageManager) {
        mMessageManager = messageManager;
        mPluginChatWindow = new PluginChatWindow(mMessageManager);
        mParticipant = participant;
        mWindowTag = windowTag;
    }

    @Override
    public String getWindowTag() {
        return mWindowTag;
    }

    /*
     * Get Plugin chat window
     */
    public PluginChatWindow getPluginChatWindow() {
        return mPluginChatWindow;
    }

    /*
     * Get Plugin chat participant
     */
    public Participant getParticipant() {
        return mParticipant;
    }

    @Override
    public IRemoteFileTransfer addReceivedFileTransfer(FileStructForBinder file, boolean fileAutoaccept) throws RemoteException {
        Logger.v(TAG, "addReceivedFileTransfer() file = " + file);
        PluginChatWindowFileTransfer pluginChatWindowFileTransfer = new PluginChatWindowFileTransfer(file,
                PluginUtils.INBOX_MESSAGE, mParticipant.getContact());
        String fileTransferString = PluginUtils.getStringInRcse(R.string.file_transfer_title);
        fileTransferString = fileTransferString + " " + file.fileSize + "KB";
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
                pluginChatWindowFileTransfer.initIpMessageInCache();
                idInMms = PluginUtils.insertDatabase(fileTransferString, contact, ipMsgId, PluginUtils.INBOX_MESSAGE);
                pluginChatWindowFileTransfer.storeInCache(idInMms);

                Intent intent = new Intent();
                intent.setAction(IpMessageConsts.NewMessageAction.ACTION_NEW_MESSAGE);
                intent.putExtra(IpMessageConsts.NewMessageAction.IP_MESSAGE_KEY, idInMms);
                AndroidFactory.getApplicationContext().sendBroadcast(intent);
            } else {
            	pluginChatWindowFileTransfer.initIpMessageInCache();
                pluginChatWindowFileTransfer.storeInCache(idInMms);
                Logger.d(TAG, "addReceivedFileTransfer(), already in mms database, no need to insert!");
            }

        } else {
            Logger.w(TAG, "addReceivedFileTransfer(), is not in rcse db!");
        }
        return pluginChatWindowFileTransfer;
    }

    @Override
    public IRemoteFileTransfer addSentFileTransfer(FileStructForBinder file) throws RemoteException {
        Logger.v(TAG, "addSentFileTransfer() file = " + file);
        PluginChatWindowFileTransfer pluginChatWindowFileTransfer = new PluginChatWindowFileTransfer(file,
                PluginUtils.OUTBOX_MESSAGE, mParticipant.getContact());
        Long fileTransferIdInMms;
        String fileTransferString = PluginUtils.getStringInRcse(R.string.file_transfer_title);
        if (!file.fileTransferTag.contains("-")) {
            fileTransferIdInMms = PluginUtils.storeMessageInDatabase(file.fileTransferTag, fileTransferString,
                    mParticipant.getContact(), PluginUtils.OUTBOX_MESSAGE);
            pluginChatWindowFileTransfer.initIpMessageInCache();
            pluginChatWindowFileTransfer.storeInCache(fileTransferIdInMms);
        } else {
            Logger.d(TAG, "Already in database!");
        }
        Logger.d(TAG, "addSentFileTransfer(), pluginChatWindowFileTransfer = " + pluginChatWindowFileTransfer);
        return pluginChatWindowFileTransfer;
    }

    @Override
    public void setFileTransferEnable(int reason) throws RemoteException {
        Logger.v(TAG, "setFileTransferEnable() reason = " + reason);
    }

    @Override
    public void setIsComposing(boolean isComposing) throws RemoteException {
        Logger.v(TAG, "setIsComposing() isComposing = " + isComposing);
        mIsComposing = isComposing;
        updateRcseContactStatus(IS_TYPINT_STATUS);
        Intent it = new Intent();
        it.setAction(ACTION_IM_STATUS);
        it.putExtra(IpMessageConsts.NUMBER, mParticipant.getContact());
        IpNotificationsManager.notify(it);
    }

    @Override
    public void setRemoteOfflineReminder(boolean isOffline) throws RemoteException {
        Logger.v(TAG, "setRemoteOfflineReminder() isOffline = " + isOffline);
        mIsOffline = isOffline;
        updateRcseContactStatus(IS_ONLINE_STATUS);
    }

    @Override
    public void addLoadHistoryHeader(boolean showLoader) throws RemoteException {
        mPluginChatWindow.addLoadHistoryHeader(showLoader);
    }

    @Override
    public IRemoteReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) throws RemoteException {
        return mPluginChatWindow.addReceivedMessage(message, isRead);
    }

    @Override
    public IRemoteSentChatMessage addSentMessage(InstantMessage message, int messageTag) throws RemoteException {
        Logger.d(TAG, "addSentMessage() text: " + message.getTextMessage() + " messageTag: " + messageTag);
        return mPluginChatWindow.addSentMessage(message, messageTag);
    }

    @Override
    public IRemoteSentChatMessage getSentChatMessage(String messageId) throws RemoteException {
        return mPluginChatWindow.getSentChatMessage(messageId);
    }

    @Override
    public void removeAllMessages() throws RemoteException {
        mPluginChatWindow.removeAllMessages();
    }

    @Override
    public void updateAllMsgAsRead() throws RemoteException {
        mPluginChatWindow.updateAllMsgAsRead();
    }

    private void updateRcseContactStatus(int reason) {
        Logger.d(TAG, "updateRcseContactStatus() the reason is " + reason + " mIsOffline is " + mIsOffline
                + " mIsComposing is " + mIsComposing);
        switch (reason) {
            case IS_ONLINE_STATUS:
                if (mIsOffline) {
                    IpMessageContactManager.putStatusByNumber(mParticipant.getContact(), RcseContactStatus.OFFLINE);
                } else {
                    IpMessageContactManager.putStatusByNumber(mParticipant.getContact(), RcseContactStatus.ONLINE);
                }
                break;
            case IS_TYPINT_STATUS:
                if (mIsComposing) {
                    IpMessageContactManager.putStatusByNumber(mParticipant.getContact(), RcseContactStatus.TYPING);
                } else {
                    IpMessageContactManager.putStatusByNumber(mParticipant.getContact(), RcseContactStatus.STOP_TYPING);
                }
                break;
            default:
                Logger.d(TAG, "updateRcseContactStatus() the reason is not resolved.");
        }
        
        Intent it = new Intent();
        it.setAction(ACTION_IM_STATUS);
        it.putExtra(IpMessageConsts.NUMBER, mParticipant.getContact());
        IpNotificationsManager.notify(it);
    }
}
