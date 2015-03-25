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

package com.mediatek.rcse.interfaces;

import android.content.Intent;
import android.content.Context;


import com.mediatek.rcse.api.Participant;

import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;


import java.util.ArrayList;
import java.util.List;

/**
 * As part of the MVC pattern, this class is the Model
 */
public final class ChatModel {

    // Common class should not be instantiated
    private ChatModel() {
    };

    /**
     * This interface defines a common chat message stored in a Chat.
     */
    public interface IChatMessage {
        /**
         * Get instant message.
         * 
         * @return Instant message.
         */
        InstantMessage getInstantMessage();
    }

    /**
     * This interface defines the common chat, it's consist of serials chat
     * message.
     */
    public interface IChat {
        /**
         * Add chat message to current chat
         * 
         * @param count The count of loading
         */
        void loadChatMessages(int count);

        /**
         * Remove chat message by specify index
         * 
         * @param index The index of the message you retrieved
         * @return true if success, otherwise failed
         */
        boolean removeMessage(int index);

        /**
         * Remove chat messages from current chat
         * 
         * @param start The index of the first chat message to remove
         * @param end The index of the last chat message to remove
         * @return true if success, otherwise failed
         */
        boolean removeMessages(int start, int end);

        /**
         * Get the count of chat messages in current chat
         * 
         * @return The count of the chat message
         */
        int getChatMessageCount();

        /**
         * Get a single piece of message in the current chat
         * 
         * @param index The index of the message you retrieved
         * @return IChatMessage if the chat message of the index
         *         exists,otherwise null
         */
        IChatMessage getSentChatMessage(int index);

        /**
         * Get all the chat messages in current chat
         * 
         * @return All the chat messages
         */
        List<IChatMessage> listAllChatMessages();

        /**
         * If the session is disconnected or not established, then send
         * invitation. Otherwise send message.
         * 
         * @param content The content that sends
         * @param messageTag The tag of this message
         */
        void sendMessage(String content, int messageTag);

        /**
         * Handle invitation.
         * 
         * @param chatSession The chat session to be handled.
         * @param messages The messages to be showed.
         * @param is auto accept.
         */
        void handleInvitation(IChatSession chatSession, ArrayList<IChatMessage> messages, boolean isAutoAccept);

        /**
         * Calling this method indicates the text in a specific chat window has
         * been changed
         * 
         * @param isEmpty Whether the text is empty
         */
        void hasTextChanged(boolean isEmpty);
    }

    /**
     * This interface defines a chat manager that manages the chat.
     * NOTE:Suggests use this structure 'Map<Object,IChat>' to restore
     * information in future implementation.
     */
    public interface IChatManager {
        /**
         * Add a chat
         * 
         * @param participants The participants to chat, add a one-to-one chat
         *            if the size of participants is 1, add a group chat if the
         *            size is larger than 1.
         * @param chatTag Tag of the chat.
         * @return The specify chat
         */
        IChat addChat(List<Participant> participants, Object chatTag , String Chat);

        /**
         * Get a chat by specify window tag
         * 
         * @param tag The unique window tag
         * @return The specify chat
         */
        IChat getChat(Object tag);

        /**
         * Remove a chat by specify window tag
         * 
         * @param tag The unique window tag
         * @return true if the chat has been removed success, otherwise false.
         */
        boolean removeChat(Object tag);

        /**
         * List all chats
         * 
         * @return The list of all the chats
         */
        List<IChat> listAllChat();

        /**
         * Handle coming invitation
         * 
         * @param shouldCheckSessionExist is true used to check the invitation
         *            chat exit false is used to just handle invitation
         * @return true if coming intent is a invitation, otherwise false
         */
        boolean handleInvitation(Intent intent, boolean shouldCheckSessionExist);

        /**
         * Called by RcsNotification when a file transfer invitation is coming
         * 
         * @param sessionId The id of the file transfer session
         * @return true indicates that RcsNotification doesn't need to notify
         *         the user, false indicates the RcsNotification needs to notify
         */
        boolean handleFileTransferInvitation(String sessionId, boolean isAutoAccept,boolean isGroup,String chatSessionId,String chatId);

        /**
         * Called by the MessagingApi in ApiManager when a SIP delivery
         * notification has come
         * 
         * @param contact The contact where the delivery notification from
         * @param msgId The message id of the delivery notification
         * @param status The delivery status of the message
         * @param timeStamp The time stamp when the message delivered
         */
        void handleMessageDeliveryStatus(String contact, String msgId, String status, long timeStamp);

        /**
         * Reload some messages stored in Rcse DB
         * @param messageIds The messages to be reloaded
         */
        void reloadMessages(String tag, List<Integer> messageIds);

        /**
         * Close all chat in the chat list
         */
        void closeAllChat();

        /**
         * Close all chat in the chat list , but keep them in database
         */
        void closeAllChatFromMemory();

        /**
         * Get chat list history
         */

        ArrayList<ChatListProvider> getChatListHistory(Context context);

    }
}
