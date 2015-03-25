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
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.service.Utils;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.ArrayList;
import java.util.List;

/**
 * The window for 1-2-1 chat, that is used in ChatScreenActvity.
 */
public class OneOneChatWindow extends ChatScreenWindow implements ChatView.IOne2OneChatWindow {
    private One2OneChatFragment mOneOneChatFragment = null;
    public One2OneChatFragment getmOneOneChatFragment() {
        return mOneOneChatFragment;
    }

    private static final String TAG = "OneOneChatWindow";
    private Object mTag = null;
    private static Object sLock = new Object();
    private Participant mParticipant = null;

    public OneOneChatWindow(Object tag, Participant participant) {
        mTag = tag;
        mParticipant = participant;
    }

    /**
     * Get participant of this window.
     * 
     * @return The participant.
     */
    public Participant getParticipant() {
        return mParticipant;
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

    private boolean shouldLoadData() {
        synchronized (sLock) {
            Logger.d(TAG, "shouldLoadData entry");
            ChatScreenWindowContainer instance = ChatScreenWindowContainer.getInstance();
            if (mTag != null && mTag.equals(instance.getCurrentTag())) {
                Logger.d(TAG, "shouldLoadData return true");
                return true;
            } else if (mParticipant == null) {
                Logger.d(TAG, "shouldLoadData return false, with mParticipant is null");
                return false;
            } else {
                List<Participant> participants = instance.getCurrentParticipants();
                if (participants == null || participants.size() == 0) {
                    Logger.d(TAG,
                            "shouldLoadData return false, with participant is null or size is 0");
                    return false;
                } else if (mParticipant.equals(participants.get(0))) {
                    Logger.d(TAG, "shouldLoadData return true");
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * Create and get fragment.
     * 
     * @return The Fragment associated with this window.
     */
    public One2OneChatFragment getFragment() {
        Logger.d(TAG, "getFragment entry");
        if (mOneOneChatFragment == null) {
            Logger.d(TAG, "getFragment create fragment " + mTag);
            mOneOneChatFragment = new One2OneChatFragment();
            Bundle arguments = new Bundle();
            arguments.putParcelable(Utils.CHAT_TAG, (Parcelable) mTag);
            ArrayList<Participant> participantList = new ArrayList<Participant>();
            participantList.add(mParticipant);
            arguments.putParcelableArrayList(Utils.CHAT_PARTICIPANTS, participantList);
            mOneOneChatFragment.setArguments(arguments);
        }
        Logger.d(TAG, "getFragment exit ");
        return mOneOneChatFragment;
    }

    @Override
    public void pause() {
        ControllerImpl controller = ControllerImpl.getInstance();
        Logger.d(TAG, "pause entry: mTag = " + mTag + ", controller = " + controller);
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(ChatController.EVENT_HIDE_WINDOW,
                    mTag, null);
            controllerMessage.sendToTarget();
        }

        Logger.d(TAG, "pause exit");
    }

    @Override
    public void resume() {
        ControllerImpl controller = ControllerImpl.getInstance();
        Logger.d(TAG, "resume entry: mTag = " + mTag + ", controller = " + controller
                + ", mOneOneChatFragment = " + mOneOneChatFragment);
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(ChatController.EVENT_SHOW_WINDOW,
                    mTag, null);
            controllerMessage.sendToTarget();
        }

        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.resume();
        }

        Logger.d(TAG, "resume exit");
    }

    @Override
    public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
        Logger.d(TAG,
                "addReceivedMessage entry: mTag = " + mTag + ", mOneOneChatFragment = "
                        + mOneOneChatFragment + ", message = "
                        + ((message == null) ? null : message.getTextMessage()));

        IReceivedChatMessage iReceivedChatMessage = null;

        if (shouldLoadData()) {
            if (mOneOneChatFragment != null) {
                iReceivedChatMessage = mOneOneChatFragment.addReceivedMessage(message, isRead);
            }
        }

        Logger.d(TAG, "addReceivedMessage entry");
        return iReceivedChatMessage;
    }

    @Override
    public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
        Logger.d(TAG,
                "addSentMessage entry: mTag = " + mTag + ", mOneOneChatFragment = "
                        + mOneOneChatFragment + ", message = "
                        + ((message == null) ? null : message.getTextMessage()));

        ISentChatMessage iSentChatMessage = null;

        if (shouldLoadData()) {
            if (mOneOneChatFragment != null) {
                iSentChatMessage = mOneOneChatFragment.addSentMessage(message, messageTag);
            }
        }

        Logger.d(TAG, "addSentMessage exit");
        return iSentChatMessage;
    }

    @Override
    public void removeAllMessages() {
        Logger.d(TAG, "removeAllMessages entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mOneOneChatFragment);
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.removeAllMessages();
        }

        Logger.d(TAG, "removeAllMessages exit");
    }

    @Override
    public IChatWindowMessage getSentChatMessage(String messageId) {
        Logger.d(TAG, "getSentChatMessage entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mOneOneChatFragment);
        IChatWindowMessage iChatWindowMessage = null;

        if (mOneOneChatFragment != null) {
            iChatWindowMessage = mOneOneChatFragment.getSentChatMessage(messageId);
        }

        Logger.d(TAG, "getSentChatMessage entry");
        return iChatWindowMessage;
    }

    @Override
    public void addLoadHistoryHeader(boolean showLoader) {
        Logger.d(TAG, "addLoadHistoryHeader entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mOneOneChatFragment);

        if (shouldLoadData()) {
            if (mOneOneChatFragment != null) {
                mOneOneChatFragment.addLoadHistoryHeader(showLoader);
            }
        }

        Logger.d(TAG, "addLoadHistoryHeader exiy");
    }

    @Override
    public void setFileTransferEnable(int reason) {
        Logger.d(TAG, "setFileTransferEnable entry, the tag is: " + mTag);
        if (!shouldLoadData()) {
            Logger.d(TAG, "setFileTransferEnable this window is not focused, the tag is: " + mTag);
            return;
        }
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.setFileTransferEnable(reason);
        } else {
            Logger.d(TAG, "setFileTransferEnable mOneOneChatFragment is null");
        }
    }

    @Override
    public void setIsComposing(boolean isComposing) {
        Logger.d(TAG, "setIsComposing entry, the tag is: " + mTag);
        if (!shouldLoadData()) {
            Logger.d(TAG, "setIsComposing this window is not focused, the tag is: " + mTag);
            return;
        }
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.setIsComposing(isComposing);
        } else {
            Logger.d(TAG, "setIsComposing mOneOneChatFragment is null");
        }
    }

    @Override
    public void setRemoteOfflineReminder(boolean isOffline) {
        Logger.d(TAG, "setRemoteOfflineReminder() entry, isOffline status is : " + isOffline);
        if (!shouldLoadData()) {
            Logger.d(TAG, "setRemoteOfflineReminder(), this window is not focused, the tag is: "
                    + mTag);
            return;
        }
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.setRemoteOfflineReminder(isOffline);
        } else {
            Logger.d(TAG, "setRemoteOfflineReminder() mOneOneChatFragment is null");
        }
    }

    @Override
    public IFileTransfer addSentFileTransfer(FileStruct file) {
        Logger.d(TAG, "addSentFileTransfer entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mOneOneChatFragment);
        IFileTransfer iFileTransfer = null;
        if (shouldLoadData()) {
            if (mOneOneChatFragment != null) {
                iFileTransfer = mOneOneChatFragment.addSentFileTransfer(file);
            }
        }

        Logger.d(TAG, "addSentFileTransfer exit");
        return iFileTransfer;
    }

    @Override
    public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept) {
        Logger.d(TAG, "addReceivedFileTransfer entry: mTag = " + mTag + ", mOneOneChatFragment = "
                + mOneOneChatFragment);
        IFileTransfer iFileTransfer = null;

        if (shouldLoadData()) {
            if (mOneOneChatFragment != null) {
                iFileTransfer = mOneOneChatFragment.addReceivedFileTransfer(file, isAutoAccept);
            }
        }

        Logger.d(TAG, "addReceivedFileTransfer exit");
        return iFileTransfer;
    }

    @Override
    public void addUnreadMessage(InstantMessage message) {
        Logger.d(TAG, "addUnreadMessage entry, notice null check is in outer, " + ", mTag =  "
                + mTag + ", mOneOneChatFragment = " + mOneOneChatFragment + ", message = "
                + message.getTextMessage());

        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.addUnreadMessage(message);
        }

        Logger.d(TAG, "addUnreadMessage exit");
    }

    @Override
    public void addContacts() {
        Logger.d(TAG, "addContacts entry, mOneOneChatFragment = " + mOneOneChatFragment);
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.addContacts();
        }
        Logger.d(TAG, "addContacts exit");
    }

    @Override
    public void removeChatUi() {
        Logger.d(TAG, "removeChatUi entry: mOneOneChatFragment = " + mOneOneChatFragment);

        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.removeChatUi();
        }

        Logger.d(TAG, "removeChatUi exit");
    }

    @Override
    public boolean clearHistory() {
        Logger.d(TAG, "clearHistory entry: mOneOneChatFragment = " + mOneOneChatFragment);
        boolean clearResult = false;

        if (mOneOneChatFragment != null) {
            clearResult = mOneOneChatFragment.clearHistory();
        }

        Logger.d(TAG, "clearHistory exit");
        return clearResult;
    }

    @Override
    public void updateAllMsgAsRead() {
        Logger.d(TAG, "updateAllMsgAsRead() entry");
        if (mOneOneChatFragment != null) {
            mOneOneChatFragment.updateAllMsgAsRead();
        } else {
            Logger.d(TAG, "updateAllMsgAsRead() mOneOneChatFragment is null");
            return;
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
