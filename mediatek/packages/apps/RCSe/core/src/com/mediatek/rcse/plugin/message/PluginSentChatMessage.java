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

import android.R;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.os.RemoteException;

import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

/**
 * Plugin Sent ChatWindow message
 */
public class PluginSentChatMessage extends IRemoteSentChatMessage.Stub {

    private static final String TAG = "PluginSentChatMessage";
    public static final String ACTION_MESSAGE_STATUS = "com.mediatek.mms.ipmessage.messageStatus";
    public static final String STATUS = "status";
    public static final String IP_MESSAGE_ID = "com.mediatek.mms.ipmessage.IpMessageRecdId";

    private long mDate;
    private Status mStatus;
   
	private boolean newgroupInvite = false;
    
    public boolean isNewgroupInvite() {
		return newgroupInvite;
	}

	public void setNewgroupInvite(boolean newgroupInvite) {
		this.newgroupInvite = newgroupInvite;
	}

    private final PluginChatWindowMessage mPluginChatWindowMessage;
    private IpMessageManager mMessageManager;

    public PluginSentChatMessage(IpMessageManager messageManager, InstantMessage message, int messageTag) {
        mPluginChatWindowMessage = new PluginChatWindowMessage(message);
        mMessageManager = messageManager;
        Logger.d(TAG, "PluginSentChatMessage(), messageManager: " + messageManager + "message = " + message
                + " ,messageTag: " + messageTag);
        Long messageIdInMms = null;
        if (-1 != messageTag && mMessageManager.removePresentMessage(messageTag)) {
            messageIdInMms = PluginUtils.getIdInMmsDb(messageTag);
            Logger.d(TAG, "PluginSentChatMessage() this is a present message, we'll update it." +
                    " messageIdInMms: " + messageIdInMms);
            PluginUtils.updatePreSentMessageInMmsDb(message.getMessageId(), messageTag);
        } else {
            messageIdInMms =
                PluginUtils.storeMessageInDatabase(message.getMessageId(),
                        message.getTextMessage(), message.getRemote(), PluginUtils.OUTBOX_MESSAGE,0);
           
        }
        mPluginChatWindowMessage.storeInCache(messageIdInMms);
    }
    
    private void insertThreadIDInDB(long threadId, String groupSubject){
      	 ContentValues values = new ContentValues();
           values.put(RichMessagingData.KEY_INTEGRATED_MODE_THREAD_ID, threadId);
     	     values.put(RichMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT, groupSubject);
     	     
     	     
           try {
   			Uri uri = AndroidFactory.getApplicationContext().getContentResolver().insert(RichMessagingData.CONTENT_URI_INTEGRATED, values);
   		} catch (Exception e) {
   			// TODO Auto-generated catch block
   			e.printStackTrace();
   		}
      }
    
    public PluginSentChatMessage(IpMessageManager messageManager, InstantMessage message, int messageTag, String groupSubject) {
        mPluginChatWindowMessage = new PluginChatWindowMessage(message);
        mMessageManager = messageManager;
        Logger.d(TAG, "PluginSentChatMessage(), messageManager: " + messageManager + "message = " + message
                + " ,messageTag: " + messageTag);
        Long messageIdInMms = null;
        if (-1 != messageTag && mMessageManager.removePresentMessage(messageTag)) {
            messageIdInMms = PluginUtils.getIdInMmsDb(messageTag);
            Logger.d(TAG, "PluginSentChatMessage() this is a present message, we'll update it." +
                    " messageIdInMms: " + messageIdInMms);
            PluginUtils.updatePreSentMessageInMmsDb(message.getMessageId(), messageTag);
        } else {
            messageIdInMms =
                PluginUtils.storeMessageInDatabase(message.getMessageId(),
                        message.getTextMessage(), message.getRemote(), PluginUtils.OUTBOX_MESSAGE,0);
            if (ThreadTranslater.tagExistInCache(message.getRemote()))
			{	Logger.d(TAG,
						"addSentMessage() Tag exists" + message.getRemote());
				Long thread = ThreadTranslater.translateTag(message
						.getRemote());
				insertThreadIDInDB(thread, groupSubject);				
			}
        }
        mPluginChatWindowMessage.storeInCache(messageIdInMms);
    }

    public PluginChatWindowMessage getPluginChatWindowMessage() {
        return mPluginChatWindowMessage;
    }

    @Override
    public void updateDate(long date) throws RemoteException {
        Logger.v(TAG, "updateDate(), date = " + date);
        mDate = date;
    }

    @Override
    public void updateStatus(String status) throws RemoteException {
        Logger.v(TAG, "updateStatus(), status = " + status);
        String messageId = mPluginChatWindowMessage.getInstantMessage().getMessageId();
        Long messageIdInMms = IpMessageManager.getMessageId(messageId);
        mStatus = Status.valueOf(status);
        IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
        if (ipMessage instanceof PluginIpTextMessage) {
            Logger.w(TAG, "updateStatus() setStatus to " + mStatus);
            ((PluginIpTextMessage) ipMessage).setStatus(mStatus);
        } else {
            Logger.w(TAG, "updateStatus() ipMessage is " + ipMessage);
        }
        Intent it = new Intent();
        it.setAction(ACTION_MESSAGE_STATUS);
        it.putExtra(STATUS, mStatus.ordinal());
        it.putExtra(IP_MESSAGE_ID, messageIdInMms);

        IpNotificationsManager.notify(it);
		if ((PluginUtils.getMessagingMode() == 1)&& (mStatus == Status.FAILED)) {
			Logger.v(TAG, "updateStatus(), status failed = " + status);
			Intent intent = new Intent(InvitationDialog.ACTION);
			intent.putExtra(RcsNotification.CONTACT, ipMessage.getFrom());
			intent.putExtra(InvitationDialog.KEY_STRATEGY,
					InvitationDialog.STRATEGY_IPMES_SEND_BY_SMS);			
            PluginUtils.saveThreadandTag(1, ipMessage.getFrom());
			intent.putExtra("send_by_sms_text", ((PluginIpTextMessage) ipMessage).getBody());
			intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
					| Intent.FLAG_ACTIVITY_NEW_TASK);
			AndroidFactory.getApplicationContext().startActivity(intent);
		}
    }

    public long getDate() {
        Logger.v(TAG, "getDate(), mDate = " + mDate);
        return mDate;
    }

    public Status getStatus() {
        Logger.v(TAG, "getStatus(), mStatus = " + mStatus);
        return mStatus;
    }

    @Override
    public String getId() throws RemoteException {
        // TODO Auto-generated method stub
        return null;
    }
}