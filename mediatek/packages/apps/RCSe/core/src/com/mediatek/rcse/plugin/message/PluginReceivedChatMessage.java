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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.provider.Telephony.Sms;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

/**
 * Plugin Received ChatWindow message
 */
public class PluginReceivedChatMessage extends IRemoteReceivedChatMessage.Stub {

    private static final String TAG = "PluginReceivedChatMessage";
    private PluginChatWindowMessage mPluginChatWindowMessage;
    private static final Long STORE_MESSAGE_FAILED = -1L;

    public PluginReceivedChatMessage(InstantMessage message, boolean isRead) {
        mPluginChatWindowMessage = new PluginChatWindowMessage(message);
        Logger.d(TAG, "PluginReceivedChatMessage(), message = " + message);
        Context context = ApiManager.getInstance().getContext();
        Long messageIdInMms =
                PluginUtils.storeMessageInDatabase(message.getMessageId(),
                        message.getTextMessage(), message.getRemote(), PluginUtils.INBOX_MESSAGE);
        Logger.d(TAG, "PluginReceivedChatMessage(), messageIdInMms is " + messageIdInMms);
        // Update the read field in sms db for group chat message
        if (isRead
                && message.getRemote().startsWith(
                        PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
            Logger.d(TAG, "PluginReceivedChatMessage() message is read and is group chat");
            ContentValues values = new ContentValues();
            values.put(Sms.READ, PluginUtils.STATUS_IS_READ);
            ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                    .getContentResolver();
            contentResolver.update(PluginUtils.SMS_CONTENT_URI, values, Sms._ID + " = " + messageIdInMms, null);
        } else {
            Logger.d(TAG, "PluginReceivedChatMessage() message is not read or not group chat");
        }
        mPluginChatWindowMessage.storeInCache(messageIdInMms);
        if (!isRead && context != null && !STORE_MESSAGE_FAILED.equals(messageIdInMms)) {
            Intent intent = new Intent();
            intent.setAction(IpMessageConsts.NewMessageAction.ACTION_NEW_MESSAGE);
            intent.putExtra(IpMessageConsts.NewMessageAction.IP_MESSAGE_KEY, messageIdInMms);
            Logger.d(TAG, "PluginReceivedChatMessage(), sendbroadcast to mms with the intent is "
                    + intent + " and the extra is " + intent.getExtras());
            context.sendBroadcast(intent);
        } else {
            Logger.d(TAG, "PluginReceivedChatMessage(), context is " + null);
        }
    }

    public PluginChatWindowMessage getPluginChatWindowMessage() {
        return mPluginChatWindowMessage;
    }

    @Override
    public String getId() throws RemoteException {
        return mPluginChatWindowMessage.getId();
    }

}