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

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;

/**
 * Plugin ChatWindow file transfer
 */
public class PluginChatWindowFileTransfer extends IRemoteFileTransfer.Stub {

    private static final String TAG = "PluginChatWindowFileTransfer";

    public static final int STARTING = 0;
    public static final int DOWNLOADING = 1;
    public static final int DONE = 2;

    private FileStructForBinder mFileTransfer;
    private int mMessageBox;
    private String mRemote;
    private IpMessage mIpMessage = null;

    public PluginChatWindowFileTransfer(FileStructForBinder fileTransfer, int messageBox,
            String remote) {
        mFileTransfer = fileTransfer;
        mMessageBox = messageBox;
        mRemote = remote;
    }
    
    public void initIpMessageInCache()
    {
    	  Logger.d(TAG, "initIpMessageInCache() entry!  ");
    	mIpMessage = PluginUtils.analysisFileType(mRemote, mFileTransfer);
    }
    
    public void storeInCache(Long messageIdInMms) {
        Logger.d(TAG, "storeInCache() entry! filePath = " + mFileTransfer.filePath);
        //IpMessage ipMessage = PluginUtils.analysisFileType(mRemote, mFileTransfer);
        IpMessageManager.addMessage(messageIdInMms, mFileTransfer.fileTransferTag, mIpMessage);
        Logger.d(TAG, "storeInCache() exit!");
    }

    @Override
    public void setFilePath(String filePath) throws RemoteException {
        Logger.d(TAG, "setFilePath() entry! filePath = " + filePath);
        Long messageIdInMms = IpMessageManager.getMessageId(mFileTransfer.fileTransferTag);
        if (messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null && ipMessage instanceof IpAttachMessage) {
                ((IpAttachMessage) ipMessage).setPath(filePath);
            } else {
                Logger.w(TAG, "setFilePath(), ipMessage is null!");
            }
        } else {
            Logger.w(TAG, "setFilePath(), not in cache!");
        }
    }

    @Override
    public void setProgress(long progress) throws RemoteException {
        Logger.d(TAG, "setProgress() entry! progress = " + progress);
        Long messageIdInMms = IpMessageManager.getMessageId(mFileTransfer.fileTransferTag);
        if (messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null) {
                progress = (progress * 100) / mFileTransfer.fileSize;
                int messageType = ipMessage.getType();
                switch (messageType) {
                    case IpMessageConsts.IpMessageType.PICTURE:
                        ((PluginIpImageMessage) ipMessage).setProgress(progress);
                        break;
                    case IpMessageConsts.IpMessageType.VIDEO:
                        ((PluginIpVideoMessage) ipMessage).setProgress(progress);
                        break;
                    case IpMessageConsts.IpMessageType.VOICE:
                        ((PluginIpVoiceMessage) ipMessage).setProgress(progress);
                        break;
                    case IpMessageConsts.IpMessageType.VCARD:
                        ((PluginIpVcardMessage) ipMessage).setProgress(progress);
                        break;
                    default:
                        ((PluginIpAttachMessage) ipMessage).setProgress(progress);
                }
                Intent it = new Intent();
                it.setAction(IpMessageConsts.DownloadAttachStatus.ACTION_DOWNLOAD_ATTACH_STATUS);
                it.putExtra(IpMessageConsts.STATUS, DOWNLOADING);
                it.putExtra(IpMessageConsts.DownloadAttachStatus.DOWNLOAD_PERCENTAGE, progress);
                it.putExtra(IpMessageConsts.DownloadAttachStatus.DOWNLOAD_MSG_ID, messageIdInMms);
                IpNotificationsManager.notify(it);
            } else {
                Logger.w(TAG, "setProgress(), ipMessage is null!");
            }
        } else {
            Logger.w(TAG, "setProgress(), not in cache!");
        }
    }

    @Override
    public void setStatus(int status) throws RemoteException {
        Logger.d(TAG, "setStatus() entry! status = " + status);
        int statusInMms = convertStatus(status);
        Long messageIdInMms = IpMessageManager.getMessageId(mFileTransfer.fileTransferTag);
        if (messageIdInMms != null && messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null) {
                int messageType = ipMessage.getType();
                switch (messageType) {
                    case IpMessageConsts.IpMessageType.PICTURE:
                        if (ipMessage instanceof PluginIpImageMessage) {
                            ((PluginIpImageMessage) ipMessage).setStatus(statusInMms);
                            ((PluginIpImageMessage) ipMessage).setRcsStatus(status);
                        }
                        break;
                    case IpMessageConsts.IpMessageType.VIDEO:
                        ((PluginIpVideoMessage) ipMessage).setStatus(statusInMms);
                        ((PluginIpVideoMessage) ipMessage).setRcsStatus(status);
                        break;
                    case IpMessageConsts.IpMessageType.VOICE:
                        ((PluginIpVoiceMessage) ipMessage).setStatus(statusInMms);
                        ((PluginIpVoiceMessage) ipMessage).setRcsStatus(status);
                        break;
                    case IpMessageConsts.IpMessageType.VCARD:
                        ((PluginIpVcardMessage) ipMessage).setStatus(statusInMms);
                        ((PluginIpVcardMessage) ipMessage).setRcsStatus(status);
                        break;
                    default:
                        ((PluginIpAttachMessage) ipMessage).setStatus(statusInMms);
                        ((PluginIpAttachMessage) ipMessage).setRcsStatus(status);
                }

                Intent it = new Intent();
                it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_STATUS);
                it.putExtra(IpMessageConsts.STATUS, statusInMms);
                it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID, messageIdInMms);
                IpNotificationsManager.notify(it);
            }
        }
    }
    
    private int convertStatus(int status) {
        Logger.d(TAG, "convertStatus() entry! status = " + status);
        Status enumStatus = Status.values()[status];
        int statusInMms = 0;
        if (mMessageBox == PluginUtils.OUTBOX_MESSAGE) {
            switch (enumStatus) {
                case WAITING:
                case PENDING:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_INVITE;
                    break;
                case TRANSFERING:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_SENDING;
                    break;
                case CANCEL:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_CANCEL;
                    break;
                case CANCELED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                    break;
                case REJECTED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_REJECTED;
                    break;
                case FINISHED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_SENT;
                    break;
                case FAILED:
                    statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
                    break;
                default:
                    break;
            }
        } else {
            switch (enumStatus) {
                case WAITING:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_INVITED;
                    break;
                case TRANSFERING:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_RECEIVING;
                    break;
                case CANCEL:
                    statusInMms = IpMessageConsts.IpMessageStatus.MO_CANCEL;
                    break;
                case CANCELED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                    break;
                case REJECTED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_REJECT;
                    break;
                case FINISHED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_RECEIVED;
                    break;
                case FAILED:
                    statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                    break;
                default:
                    break;
            }
        }
        
        return statusInMms;
    }

    @Override
    public void updateTag(String transferTag, long transferSize) throws RemoteException {
        String oldTag = mFileTransfer.fileTransferTag;
        mFileTransfer.fileTransferTag = transferTag;
        mFileTransfer.fileSize = transferSize;
        PluginUtils.updateMessageIdInMmsDb(oldTag, transferTag);
        IpMessageManager.updateCache(oldTag, mFileTransfer, mRemote);
    }
}

