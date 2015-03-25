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

package com.mediatek.rcse.service.binder;

import android.os.ParcelUuid;
import android.os.RemoteException;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment.ChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.plugin.message.PluginChatWindowFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;


import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * This class is used for message plugin to manage chat window
 */
public class ChatWindowManagerAdapter implements IChatWindowManager {

    public static final String TAG = "ChatWindowManagerAdapter";
    private IRemoteChatWindowManager mRemoteChatWindowManager = null;
    private List<BaseChatWindow> mRemoteChatWindows = new ArrayList<BaseChatWindow>();

    ChatWindowManagerAdapter(IRemoteChatWindowManager chatWindowManager) {
        Logger.d(TAG, "ChatWindowManagerAdapter() entry");
        mRemoteChatWindowManager = chatWindowManager;
    }

    IRemoteChatWindowManager getChatWindowManager() {
        Logger.d(TAG, "getChatWindowManager() entry");
        return mRemoteChatWindowManager;
    }

    @Override
    public IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList) {
        Logger.d(TAG, "addGroupChatWindow() entry! tag = " + tag + " participantList = " + participantList);
        try {
            IRemoteGroupChatWindow groupChatWindow =
                    mRemoteChatWindowManager.addGroupChatWindow(tag.toString(), participantList);
            Logger.d(TAG, "addGroupChatWindow() : groupChatWindow = "
                    + groupChatWindow);
            if (null != groupChatWindow) {
                GroupChatWindowAdapter remoteGroupChatWindow = new GroupChatWindowAdapter(groupChatWindow);
                TagTranslater.saveTag(tag);
                mRemoteChatWindows.add(remoteGroupChatWindow);
                return remoteGroupChatWindow;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public IOne2OneChatWindow addOne2OneChatWindow(Object tag, Participant participant) {
        Logger.d(TAG, "addOne2OneChatWindow() entry! tag = " + tag + " participant" + participant);
        try {
            Logger.d(TAG, "addOne2OneChatWindow(), mRemoteChatWindowManager = " + mRemoteChatWindowManager);
            IRemoteOne2OneChatWindow one2OneChatWindow =
                    mRemoteChatWindowManager.addOne2OneChatWindow(tag.toString(), participant);
            One2OneChatWindowAdapter one2OneChatWindowAdapter = new One2OneChatWindowAdapter(one2OneChatWindow);
            TagTranslater.saveTag(tag);
            mRemoteChatWindows.add(one2OneChatWindowAdapter);
            return one2OneChatWindowAdapter;
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public boolean removeChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "removeChatWindow() entry! chatWindow = " + chatWindow);
        if (mRemoteChatWindows.contains(chatWindow)) {
            Object remoteChatWindow = ((BaseChatWindow) chatWindow).getChatWindow();
            if (remoteChatWindow instanceof IRemoteGroupChatWindow) {
                try {
                    return mRemoteChatWindowManager.removeGroupChatWindow((IRemoteGroupChatWindow) remoteChatWindow);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    return mRemoteChatWindowManager.removeOne2OneChatWindow((IRemoteOne2OneChatWindow) remoteChatWindow);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    @Override
    public void switchChatWindowByTag(ParcelUuid uuidTag) {
        Logger.d(TAG, "switchChatWindowByTag() entry! uuidTag = " + uuidTag);
        try {
            mRemoteChatWindowManager.switchChatWindowByTag(uuidTag.toString());
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public abstract class BaseChatWindow {
        public abstract Object getChatWindow();
    }

    public class GroupChatWindowAdapter extends BaseChatWindow implements IGroupChatWindow {

        IRemoteGroupChatWindow mRemoteGroupChatWindow = null;

        GroupChatWindowAdapter(IRemoteGroupChatWindow groupChatWindow) {
            Logger.d(TAG, "GroupChatWindowAdapter() entry!");
            mRemoteGroupChatWindow = groupChatWindow;
        }

        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
            Logger.d(TAG, "addReceivedFileTransfer() entry! file = " + file);
            FileStructForBinder fileStructForBinder = createFileStructForBinder(file);
            IRemoteFileTransfer fileTransfer = null;
            try {
                fileTransfer = mRemoteGroupChatWindow.addReceivedFileTransfer(fileStructForBinder, isAutoAccept);
                fileTransfer.setStatus(1);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new FileTransferAdapter(fileTransfer);
        }

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.d(TAG, "addSentFileTransfer() entry! file = " + file);
            FileStructForBinder fileStructForBinder = createFileStructForBinder(file);
            IRemoteFileTransfer fileTransfer = null;
            try {
                fileTransfer = mRemoteGroupChatWindow.addSentFileTransfer(fileStructForBinder);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new FileTransferAdapter(fileTransfer);
        }

        @Override
        public void setFileTransferEnable(int reason) {
            Logger.d(TAG, "setFileTransferEnable() entry! reason = " + reason);
            try {
            	mRemoteGroupChatWindow.setFileTransferEnable(reason);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        @Override
        public Object getChatWindow() {
            Logger.d(TAG, "getChatWindow() entry!");
            return mRemoteGroupChatWindow;
        }

        @Override
        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            Logger.d(TAG, "addChatEventInformation() entry! chatEventStruct = " + chatEventStruct);
            ChatEventStructForBinder chatEventStructForBinder;
            chatEventStructForBinder = createChatEventStructForBinder(chatEventStruct);
            try {
                Logger.d(TAG,
                        "addChatEventInformation() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.addChatEventInformation(chatEventStructForBinder);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            ChatEventInformation chatEventInformation = new ChatEventInformation(chatEventStruct);
            return chatEventInformation;
        }

        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {
            Logger.d(TAG, "setIsComposing() entry! isComposing = " + isComposing + " participant = " + participant);
            try {
                Logger.d(TAG,
                        "setIsComposing() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.setIsComposing(isComposing, participant);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateChatStatus(int status) {
            Logger.d(TAG, "updateChatStatus() entry! status: " + status);
            try {
                Logger.d(TAG, "updateChatStatus() mRemoteGroupChatWindow: "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.updateChatStatus(status);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateParticipants(List<ParticipantInfo> participants) {
            Logger.d(TAG, "updateParticipants() entry! participants = " + participants);
            try {
                Logger.d(TAG,
                        "updateParticipants() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.updateParticipants(participants);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
            Logger.d(TAG, "addLoadHistoryHeader() entry! showLoader = " + showLoader);
            try {
                mRemoteGroupChatWindow.addLoadHistoryHeader(showLoader);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            Logger.d(TAG, "addReceivedMessage() entry! message = " + message + " isRead" + isRead);
            IRemoteReceivedChatMessage receivedChatMessage = null;
            try {
                Logger.d(TAG,
                        "updateParticipants() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    receivedChatMessage = mRemoteGroupChatWindow.addReceivedMessage(message, isRead);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new ReceivedChatMessageAdapter(receivedChatMessage);
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            Logger.d(TAG, "addSentMessage() entry! message = " + message + " messageTag = " + messageTag);
            IRemoteSentChatMessage sentChatMessage = null;
            try {
                Logger.d(TAG,
                        "updateParticipants() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    sentChatMessage = mRemoteGroupChatWindow.addSentMessage(message, messageTag);
                    sentChatMessage.updateStatus(Status.SENDING.toString());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new SentChatMessageAdapter(sentChatMessage);
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            Logger.d(TAG, "getSentChatMessage() entry! messageId = " + messageId);
            return null;
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry!");
            try {
                mRemoteGroupChatWindow.removeAllMessages();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateAllMsgAsRead() {
            Logger.d(TAG, "updateAllMsgAsRead() entry!");
            try {
                Logger.d(TAG,
                        "updateParticipants() : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.updateAllMsgAsRead();
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
 @Override
        public void removeChatMessage(String messageId) {
            // TODO Auto-generated method stub
            
        }
        @Override
        public void addgroupSubject(String subject) {
        	try {
                Logger.d(TAG,
                        "addgroupSubject : mRemoteGroupChatWindow = "
                                + mRemoteGroupChatWindow);
                if (mRemoteGroupChatWindow != null) {
                    mRemoteGroupChatWindow.addgroupSubject(subject);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            
        }

		@Override
		public void updateAllMsgAsReadForContact(Participant participant) {
			// TODO Auto-generated method stub
			
		}
    }

    public class One2OneChatWindowAdapter extends BaseChatWindow implements IOne2OneChatWindow {

        IRemoteOne2OneChatWindow mRemoteOne2OneChatWindow = null;

        public One2OneChatWindowAdapter(IRemoteOne2OneChatWindow one2OneChatWindow) {
            Logger.d(TAG, "One2OneChatWindowAdapter() entry! one2OneChatWindow = " + one2OneChatWindow);
            mRemoteOne2OneChatWindow = one2OneChatWindow;
        }

        @Override
        public Object getChatWindow() {
            Logger.d(TAG, "getChatWindow() entry!");
            return mRemoteOne2OneChatWindow;
        }

        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
            Logger.d(TAG, "addReceivedFileTransfer() entry! file = " + file);
            FileStructForBinder fileStructForBinder = createFileStructForBinder(file);
            IRemoteFileTransfer fileTransfer = null;
            try {
                fileTransfer = mRemoteOne2OneChatWindow.addReceivedFileTransfer(fileStructForBinder, isAutoAccept);
                if(!isAutoAccept)
                	fileTransfer.setStatus(1);
                else
                	fileTransfer.setStatus(2);
                
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new FileTransferAdapter(fileTransfer);
        }

        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.d(TAG, "addSentFileTransfer() entry! file = " + file);
            FileStructForBinder fileStructForBinder = createFileStructForBinder(file);
            IRemoteFileTransfer fileTransfer = null;
            try {
                fileTransfer = mRemoteOne2OneChatWindow.addSentFileTransfer(fileStructForBinder);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return new FileTransferAdapter(fileTransfer);
        }

        @Override
        public void setFileTransferEnable(int reason) {
            Logger.d(TAG, "setFileTransferEnable() entry! reason = " + reason);
            try {
                mRemoteOne2OneChatWindow.setFileTransferEnable(reason);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void setIsComposing(boolean isComposing) {
            Logger.d(TAG, "setIsComposing() entry! isComposing = " + isComposing);
            try {
                mRemoteOne2OneChatWindow.setIsComposing(isComposing);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {
            Logger.d(TAG, "setRemoteOfflineReminder() entry! isOffline = " + isOffline);
            try {
                mRemoteOne2OneChatWindow.setRemoteOfflineReminder(isOffline);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
            Logger.d(TAG, "addLoadHistoryHeader() entry! + showLoader =" + showLoader);
            try {
                mRemoteOne2OneChatWindow.addLoadHistoryHeader(showLoader);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            Logger.d(TAG, "addReceivedMessage() entry! message = " + message + " isRead = " + isRead);
            try {
                IRemoteReceivedChatMessage receivedChatMessage =
                        mRemoteOne2OneChatWindow.addReceivedMessage(message, isRead);
                return new ReceivedChatMessageAdapter(receivedChatMessage);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            Logger.d(TAG, "addSentMessage() entry! message = " + message + " messageTag = " + messageTag);
            try {
                IRemoteSentChatMessage sentChatMessage =
                        mRemoteOne2OneChatWindow.addSentMessage(message, messageTag);
                return new SentChatMessageAdapter(sentChatMessage);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            Logger.d(TAG, "getSentChatMessage() entry! + messageId = " + messageId);
            return null;
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry!");
            try {
                mRemoteOne2OneChatWindow.removeAllMessages();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateAllMsgAsRead() {
            Logger.d(TAG, "updateAllMsgAsRead() entry!");
            try {
                mRemoteOne2OneChatWindow.updateAllMsgAsRead();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void removeChatMessage(String messageId) {
            // TODO Auto-generated method stub
            
        }

		@Override
		public void updateAllMsgAsReadForContact(Participant participant) {
			// TODO Auto-generated method stub
			
		}
    }

    public static class ReceivedChatMessageAdapter implements IReceivedChatMessage {

        IRemoteReceivedChatMessage mReceivedChatMessage = null;

        public ReceivedChatMessageAdapter(IRemoteReceivedChatMessage receivedChatMessage) {
            Logger.d(TAG, "ReceivedChatMessageAdapter() entry! receivedChatMessage = " + receivedChatMessage);
            mReceivedChatMessage = receivedChatMessage;
        }

        @Override
        public String getId() {
            Logger.d(TAG, "getId() entry!");
            try {
                return mReceivedChatMessage.getId();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return null;
        }
    }

    public static class SentChatMessageAdapter implements ISentChatMessage {

        IRemoteSentChatMessage mSentChatMessage;

        public SentChatMessageAdapter(IRemoteSentChatMessage sentChatMessage) {
            Logger.d(TAG, "SentChatMessageAdapter() entry! sentChatMessage = " + sentChatMessage);
            mSentChatMessage = sentChatMessage;
        }

        @Override
        public void updateDate(Date date) {
            Logger.d(TAG, "updateDate() entry! date = " + date
                    + ", mSentChatMessage = " + mSentChatMessage);
            try {
                if (mSentChatMessage != null) {
                    mSentChatMessage.updateDate(date.getTime());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateStatus(Status status) {
            Logger.d(TAG, "updateStatus() entry! status = " + status
                    + ",mSentChatMessage = " + mSentChatMessage);
            try {
                if (mSentChatMessage != null) {
                    mSentChatMessage.updateStatus(status.name());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public String getId() {
            Logger.d(TAG, "getId() entry!");
            try {
                return mSentChatMessage.getId();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return null;
        }

		public void updateStatus(Status status, String contact) {
			Logger.d(TAG, "updateStatus() entry! status = " + status
                    + ",mSentChatMessage = " + mSentChatMessage + ",contact = "+contact);
            try {
                if (mSentChatMessage != null) {
                    mSentChatMessage.updateStatus(status.name());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
			
		}
    }

    public static class FileTransferAdapter implements IFileTransfer {

        IRemoteFileTransfer mFileTransfer;

        public FileTransferAdapter(IRemoteFileTransfer fileTransfer) {
            Logger.d(TAG, "FileTransferAdapter() entry! fileTransfer = " + fileTransfer);
            mFileTransfer = fileTransfer;
        }

        @Override
        public void setFilePath(String filePath) {
            Logger.d(TAG, "setFilePath() entry! filePath = " + filePath
                    + ",mFileTransfer = " + mFileTransfer);
            try {
                if (mFileTransfer != null) {
                    mFileTransfer.setFilePath(filePath);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void setProgress(long progress) {
            Logger.d(TAG, "setProgress() entry! progress = " + progress
                    + ", mFileTransfer = " + mFileTransfer);
            try {
                if (mFileTransfer != null) {
                    mFileTransfer.setProgress(progress);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void setStatus(Status status) {
            Logger.d(TAG, "setStatus() entry! status = " + status
                    + ", mFileTransfer = " + mFileTransfer);
            try {
                if (mFileTransfer != null) {
                    mFileTransfer.setStatus(status.ordinal());
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void updateTag(String transferTag, long transferSize) {
            Logger.d(TAG, "updateTag() entry! transferTag = " + transferTag
                    + ", mFileTransfer = " + mFileTransfer);
            try {
                if (mFileTransfer != null) {
                    mFileTransfer.updateTag(transferTag, transferSize);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    private ChatEventStructForBinder createChatEventStructForBinder(ChatEventStruct chatEventStruct) {
        Logger.d(TAG, "createChatEventStructForBinder(), chatEventStruct = " + chatEventStruct);
        int information = chatEventStruct.information.ordinal();
        List<String> relatedInfo = new ArrayList<String>();
        String date = null;
        Logger.d(TAG,
                "createChatEventStructForBinder(): chatEventStruct.relatedInformation = "
                        + chatEventStruct.relatedInformation
                        + ", chatEventStruct.date = " + chatEventStruct.date);
        if (chatEventStruct.relatedInformation != null) {
            relatedInfo.add(chatEventStruct.relatedInformation.toString());
        }
        if (chatEventStruct.date != null) {
            date = chatEventStruct.date.toString();
        }
        ChatEventStructForBinder chatEventStructForBinder =
                new ChatEventStructForBinder(information, relatedInfo, date);
        return chatEventStructForBinder;
    }

    private FileStructForBinder createFileStructForBinder(FileStruct fileStruct) {
        Logger.d(TAG, "createFileStructForBinder() entry! fileStruct = " + fileStruct);
        FileStructForBinder fileStructForBinder = new FileStructForBinder(fileStruct);
        return fileStructForBinder;
    }

}
