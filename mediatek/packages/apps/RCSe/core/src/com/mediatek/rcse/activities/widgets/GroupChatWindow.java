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

package com.mediatek.rcse.activities.widgets;

import android.os.Bundle;
import android.os.Message;
import android.os.Parcelable;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The window for group chat, that is used in ChatScreenActvity.
 */
public class GroupChatWindow extends ChatScreenWindow implements ChatView.IGroupChatWindow {
    private static final String TAG = "GroupChatWindow";
    private GroupChatFragment mGroupChatFragment = null;
    public GroupChatFragment getmGroupChatFragment() {
        return mGroupChatFragment;
    }

    private Object mTag = null;
    private static Object sLock = new Object();
    private List<Participant> mParticipants = null;
    private CopyOnWriteArrayList<ParticipantInfo> mParticipantInfos = null;

    public GroupChatWindow(Object tag, CopyOnWriteArrayList<ParticipantInfo> participants) {
        mTag = tag;
        mParticipantInfos = participants;
        List<Participant> participantList = new ArrayList<Participant>();
        for (ParticipantInfo participantInfo : participants) {
            participantList.add(participantInfo.getParticipant());
        }
        mParticipants = participantList;
    }

    @Override
    public void setFileTransferEnable(int reason) {
        Logger.d(TAG, "setFileTransferEnable entry, the tag is: " + mTag);
        if (!shouldLoadData()) {
            Logger.d(TAG, "setFileTransferEnable this window is not focused, the tag is: " + mTag);
            return;
        }
        if (mGroupChatFragment != null) {
        	mGroupChatFragment.setFileTransferEnable(reason);
        } else {
            Logger.d(TAG, "setFileTransferEnable mOneOneChatFragment is null");
        }
    }
    private boolean shouldLoadData() {
        synchronized (sLock) {
            Logger.d(TAG, "shouldLoadData entry");
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            if (mTag != null && mTag.equals(instance.getCurrentTag())) {
                Logger.d(TAG, "shouldLoadData true");
                return true;
            } else if (mParticipants == null) {
                return false;
            } else {
                List<Participant> participants = instance.getCurrentParticipants();
                if (participants == null) {
                    return false;
                } else if (mParticipants.equals(participants)) {
                    Logger.d(TAG, "shouldLoadData true");
                    return true;
                }
            }
            return false;
        }
    }
    
    @Override
    public IFileTransfer addSentFileTransfer(FileStruct file) {
        Logger.d(TAG, "addSentFileTransfer entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mGroupChatFragment);
        IFileTransfer iFileTransfer = null;
        if (shouldLoadData()) {
            if (mGroupChatFragment != null) {
                iFileTransfer = mGroupChatFragment.addSentFileTransfer(file);
            }
        }

        Logger.d(TAG, "addSentFileTransfer exit");
        return iFileTransfer;
    }

    
    @Override
    public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
        Logger.d(TAG, "addReceivedFileTransfer entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mGroupChatFragment);
        IFileTransfer iFileTransfer = null;

        if (shouldLoadData()) {
            if (mGroupChatFragment != null) {
                iFileTransfer = mGroupChatFragment.addReceivedFileTransfer(file, isAutoAccept);
            }
        }

        Logger.d(TAG, "addReceivedFileTransfer exit");
        return iFileTransfer;
    }

    /**
     * Create and get fragment.
     * 
     * @return The Fragment associated with this window.
     */
    public GroupChatFragment getFragment() {
        Logger.d(TAG, "getFragment entry");
        if (mGroupChatFragment == null) {
            Logger.d(TAG, "getFragment create fragment " + mTag);
            mGroupChatFragment = new GroupChatFragment();
            Bundle arguments = new Bundle();
            arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) mTag);
            Logger.d(TAG, "getFragment() mParticipantInfos: " + mParticipantInfos);
            if (mParticipantInfos != null) {
                ArrayList<ParticipantInfo> infos = new ArrayList<ParticipantInfo>();
                for (ParticipantInfo info : mParticipantInfos) {
                    infos.add(info);
                }
                arguments.putParcelableArrayList(Utils.CHAT_PARTICIPANTS, infos);
            }
            mGroupChatFragment.setArguments(arguments);
        }
        Logger.d(TAG, "getFragment exit");
        return mGroupChatFragment;
    }

    /**
     * Get tag of this window.
     * 
     * @return The tag.
     */
    @Override
    public Object getTag() {
        return mTag;
    }

    /**
     * when switching from one fragment to another, this method should be called
     * to pause the previous associated model chat.
     */
    public void pause() {
        Logger.d(TAG, "pause " + mTag);
        collapseGroupChat();
        ControllerImpl controller = ControllerImpl.getInstance();
        if (null != controller) {
            Message controllerMessage =
                    controller.obtainMessage(ChatController.EVENT_HIDE_WINDOW, mTag, null);
            controllerMessage.sendToTarget();
        } else {
            Logger.e(TAG, "pause controller is null");
        }
    }

    /**
     * when switching from one fragment to another, this method should be called
     * to resume the new associated model chat.
     */
    public void resume() {
        Logger.d(TAG, "resume " + mTag);
        ControllerImpl controller = ControllerImpl.getInstance();
        if (null != controller) {
            Message controllerMessage =
                    controller.obtainMessage(ChatController.EVENT_SHOW_WINDOW, mTag, null);
            controllerMessage.sendToTarget();
        } else {
            Logger.e(TAG, "resume controller is null");
        }
        if (mGroupChatFragment != null) {
            mGroupChatFragment.resume();
        } else {
            Logger.d(TAG, "resume mGroupChatFragment is null");
        }
    }

    @Override
    public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
        Logger.d(TAG, "addReceivedMessage entry, the tag is: " + mTag + " message is: "
                + ((message == null) ? null : message.getTextMessage()));
        if (!shouldLoadData()) {
            Logger.d(TAG, "addReceivedMessage this window is not focused, the tag is: " + mTag);
            return null;
        }
        if (mGroupChatFragment != null) {
            mGroupChatFragment.addReceivedMessage(message, isRead);
        } else {
            Logger.d(TAG, "addReceivedMessage mGroupChatFragment is null");
        }
        return null;
    }

    @Override
    public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
        Logger.d(TAG, "addSentMessage entry, the tag is: " + mTag + " message is: "
                + ((message == null) ? null : message.getTextMessage()));
        SentMessage msg = null;
        if (!shouldLoadData()) {
            Logger.d(TAG, "addSentMessage this window is not focused, the tag is: " + mTag);
            return null;
        }
        if (mGroupChatFragment != null) {
            msg = (SentMessage)mGroupChatFragment.addSentMessage(message, messageTag);
        } else {
            Logger.d(TAG, "addSentMessage mGroupChatFragment is null");
        }
        return msg;
    }

    @Override
    public void removeAllMessages() {
        Logger.d(TAG, "removeAllMessages entry, the tag is: " + mTag + " ,mGroupChatFragment is :"
                + mGroupChatFragment);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.removeAllMessages();
        } else {
            Logger.d(TAG, "removeAllMessages mGroupChatFragment is null");
        }
    }

    @Override
    public IChatWindowMessage getSentChatMessage(String messageId) {
        Logger.d(TAG, "getSentChatMessage entry, the tag is: " + mTag);
        if (mGroupChatFragment != null) {
            return mGroupChatFragment.getSentChatMessage(messageId);
        } else {
            Logger.d(TAG, "getSentChatMessage mGroupChatFragment is null");
            return null;
        }
    }

    @Override
    public void addLoadHistoryHeader(boolean showLoader) {
    }

    @Override
    public void updateParticipants(List<ParticipantInfo> participantList) {
        Logger.d(TAG, "updateParticipants entry, the tag is: " + mTag);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.updateParticipants(participantList);
        } else {
            Logger.d(TAG, "updateParticipants mGroupChatFragment is null");
        }
    }

    @Override
    public void setIsComposing(boolean isComposing, Participant participant) {
        Logger.d(TAG, "setIsComposing entry, the tag is: " + mTag);
        if (!shouldLoadData()) {
            Logger.d(TAG, "setIsComposing this window is not focused, the tag is: " + mTag);
            return;
        }
        if (mGroupChatFragment != null) {
            mGroupChatFragment.setIsComposing(isComposing, participant);
        } else {
            Logger.d(TAG, "setIsComposing mGroupChatFragment is null");
        }
    }

    /**
     * Expand group chat's avatars.
     */
    public void expandGroupChat() {
        Logger.d(TAG, "expandGroupChat entry, the tag is: " + mTag + " ,mGroupChatFragment is :"
                + mGroupChatFragment);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.expandGroupChat();
        }
    }

    /**
     * Collapse group chat's avatars.
     */
    public void collapseGroupChat() {
        Logger.d(TAG, "collapseGroupChat entry, the tag is: " + mTag + " ,mGroupChatFragment is :"
                + mGroupChatFragment);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.collapseGroupChat();
        }
    }

    @Override
    public void addContacts() {
        Logger.d(TAG, "addContacts mGroupChatFragment is: " + mGroupChatFragment);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.addContacts();
        }
    }

    @Override
    public void removeChatUi() {
        Logger.d(TAG, "removeChatUi mGroupChatFragment is : " + mGroupChatFragment);
        if (mGroupChatFragment != null) {
            mGroupChatFragment.removeChatUi();
        }
    }

    @Override
    public boolean clearHistory() {
        Logger.d(TAG, "clearHistory entry");
        if (mGroupChatFragment != null) {
            return mGroupChatFragment.clearHistory();
        } else {
            Logger.d(TAG, "clearHistory mGroupChatFragment is null");
            return false;
        }
    }

    @Override
    public void addUnreadMessage(InstantMessage message) {
        Logger.d(TAG, "addUnreadMessage entry, notice null check is in outer, " + "the tag is: "
                + mTag + " message is: " + message.getTextMessage());
        if (Logger.getIsIntegrationMode()) {
            Logger.d(TAG,
                    "addUnreadMessage() it is integration mode, not need to add unread message");
        } else {
            if (mGroupChatFragment != null) {
                mGroupChatFragment.addUnreadMessage(message);
            } else {
                Logger.d(TAG, "removeChatUi mGroupChatFragment is null");
            }
        }
        Logger.d(TAG, "addUnreadMessage exit");
    }

    @Override
    public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
        Logger.d(TAG, "addChatEventInformation entry, the tag is: " + mTag
                + " chatEventStruct is: " + chatEventStruct);
        if (!shouldLoadData()) {
            Logger
                    .d(TAG, "addChatEventInformation this window is not focused, the tag is: "
                            + mTag);
            return null;
        }
        if (mGroupChatFragment != null) {
            return mGroupChatFragment.addChatEventInformation(chatEventStruct);
        } else {
            Logger.d(TAG, "addChatEventInformation mGroupChatFragment is null");
            return null;
        }

    }

    
    @Override
    public void updateChatStatus(int status) {
        Logger.d(TAG, "updateChatStatus() entry, the tag is: " + mTag + " status: " + status);
        if (!shouldLoadData()) {
            Logger.d(TAG, "updateChatStatus() this window is not focused, the tag is: " + mTag);
            return;
        }
        if (mGroupChatFragment != null) {
            mGroupChatFragment.updateChatStatus(status);
        }
    }

    @Override
    public void updateAllMsgAsRead() {
        Logger.d(TAG, "updateAllMsgAsRead() entry");
        if (mGroupChatFragment != null) {
            mGroupChatFragment.updateAllMsgAsRead();
        } else {
            Logger.d(TAG, "updateAllMsgAsRead() mGroupChatFragment is null");
        }
    }
 @Override
    public void removeChatMessage(String messageId) {
        // TODO Auto-generated method stub
        
    }
    @Override
    public void addgroupSubject(String subject) {
        if (mGroupChatFragment != null) {
            mGroupChatFragment.addgroupSubject(subject);
        } else {
            Logger.d(TAG, "addgroupSubject() mGroupChatFragment is null");
        }
        
    }

	@Override
	public void updateAllMsgAsReadForContact(Participant participant) {
		// TODO Auto-generated method stub
		
	}
}
