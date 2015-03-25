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

import android.os.ParcelUuid;

import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;

import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.Date;
import java.util.List;

/**
 * As part of the MVC pattern, this class is the View. Please be aware that each
 * method shown below should NOT be called from a main thread.
 */
public final class ChatView {

    // Common class should not be instantiated
    private ChatView() {
    };

    /**
     * This interface defines a common chat message shown in a chat window
     */
    public interface IChatWindowMessage {
        /**
         * Get the message id of this message
         * 
         * @return The message id
         */
        String getId();
    }

    /**
     * This interface defines one type of chat message that is received from a
     * remote contact
     */
    public interface IReceivedChatMessage extends IChatWindowMessage {
        // TODO Whether a received chat message needs additional methods is
        // still not determined.
    }

    /**
     * This interface defines one type of chat message that is sent from the
     * current user
     */
    public interface ISentChatMessage extends IChatWindowMessage {
        /**
         * This enumerate defines the status of a sent message
         */
        public static enum Status {
            /**
             * The message is being sending
             */
            SENDING,
            /**
             * The message has been delivered
             */
            DELIVERED,
            /**
             * The message has been displayed to the remote contact
             */
            DISPLAYED,
            /**
             * A failure happened when sending this message
             */
            FAILED,
            /**
             * Unknown status.
             */
            UNKNOWN
        }

        /**
         * Update the latest status for this message
         * 
         * @param status The status that would be updated
         */
        void updateStatus(Status status);

        /**
         * Update the latest status for this message in group chat
         * 
         * @param status The status that would be updated
         */
        void updateStatus(Status status, String Contact);

        /**
         * Update the date of a sent message
         * @param date The real date of this message
         */
        void updateDate(Date date);
    }

    /**
     * This interface defines the chat event information
     */
    public interface IChatEventInformation {

        /**
         * This enumerate defines the types of the chat event
         */
        public static enum Information {
            /**
             * This event indicates you invite some participant into a group
             * chat.
             */
            INVITE,
            /**
             * This event indicates user has quit from the chat
             */
            QUIT,

            /**
             * This event indicates user has rejoined into the chat
             */
            REJOIN,

            /**
             * This event indicates that one of the participants has left from
             * the chat
             */
            LEFT,

            /**
             * This event indicates that a new participant has joined into the
             * chat
             */
            JOIN,

            /**
             * This event indicates the group chat have no body join in
             */
            NOBODY_JOIN_IN
        }
    }

    /**
     * This interface stands for a single file transfer
     */
    public interface IFileTransfer {
        /**
         * The status that can be used in a file transfer.
         */
        public static enum Status {
            /**
             * The file transfer invitation has to wait until the active queue
             * is available
             */
            PENDING,
            /**
             * The file transfer invitation is waiting for the acceptation
             */
            WAITING,
            /**
             * The file transfer is on-going
             */
            TRANSFERING,
            /**
             * The file transfer is canceled by current user
             */
            CANCEL,
            /**
             * The file transfer is canceled by the remote contact
             */
            CANCELED,
            /**
             * The file transfer has failed
             */
            FAILED,
            /**
             * The file transfer has been rejected
             */
            REJECTED,
            /**
             * The file transfer has been done with success
             */
            FINISHED,
            
            /**
             * The file transfer has been timeout with no response from receiver
             */
            TIMEOUT,
        }

        /**
         * Set the current status of the file transfer
         * 
         * @param status Current status
         */
        void setStatus(Status status);

        /**
         * Set the progress of the file transfer, notice that unless the file
         * transfer is in TRANSFERING status ,calling this method will not work
         * 
         * @param progress
         */
        void setProgress(long progress);

        /**
         * When finished to received a file, set the file path to the file
         * transfer
         * 
         * @param received file path
         */
        void setFilePath(String filePath);

        /**
         * Update file transfer tag and size
         * 
         * @param received file path
         * @param received file size
         */
        void updateTag(String transferTag, long transferSize);
    }

    /**
     * This interface defines the common chat window
     */
    public interface IChatWindow {
        /**
         * Add a received message into this chat window.
         * 
         * @param message The message to add.
         * @param isRead identify if the message is read.
         * @return IReceivedChatMessage converted from InstantMessage.
         */
        IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead);

        /**
         * Add a received message into this chat window.
         * 
         * @param message The message to add.
         * @param messageTag The tag of this message
         * @return ISentChatMessage converted from InstantMessage.
         */
        ISentChatMessage addSentMessage(InstantMessage message, int messageTag);

        /**
         * Remove all messages from this chat window.
         */
        void removeAllMessages();

        /**
         * Get a single piece of message interface in this chat window
         * 
         * @param index The index of the message you want to get
         * @return IChatMessage if the message of the index exists, otherwise
         *         null
         */
        IChatWindowMessage getSentChatMessage(String messageId);

        /**
         * add load history view.
         */
        void addLoadHistoryHeader(boolean showLoader);

        /**
         * update all the message as read.
         */
        void updateAllMsgAsRead();

        void updateAllMsgAsReadForContact(Participant participant);
        
        void removeChatMessage(String messageId);

    }

    /**
     * This interface defines one type of chat window that used for a 1-2-1 chat
     */
    public interface IOne2OneChatWindow extends IChatWindow {
        /**
         * Set whether the file transfer function should be enabled
         * 
         * @param isEnable Enable or disable the file transfer function
         */
        void setFileTransferEnable(int reason);

        /**
         * Set whether the contact is composing
         * 
         * @param isComposing Whether this contact is composing
         */
        void setIsComposing(boolean isComposing);

        /**
         * Set whether the remote contact is offline
         * 
         * @param isOffline Whether need to show the offline reminder
         */
        void setRemoteOfflineReminder(boolean isOffline);

        /**
         * Add a sent file transfer into VIEW module
         * 
         * @param file The file to be sent
         * @return A IFileTransfer to MODEL module
         */
        IFileTransfer addSentFileTransfer(FileStruct file);

        /**
         * Add a received file transfer into VIEW module
         * 
         * @param file The file received
         * @return A IFileTransfer to MODEL module
         */
        IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept);
        
        
    }

    /**
     * This interface defines one type of chat window that used for a group chat
     */
    public interface IGroupChatWindow extends IChatWindow {
    	
    	void setFileTransferEnable(int reason);
        /**
         * Update the participant list
         * 
         * @param participantList The latest participant list
         */
    	
        void updateParticipants(List<ParticipantInfo> participants);

        /**
         * Set whether a specific participant is composing
         * 
         * @param isComposing Whether this participant is composing
         * @param participant The participant who is composing or not
         */
        void setIsComposing(boolean isComposing, Participant participant);

        /**
         * Update view due to group chat's status.
         * 
         * @param status Group chat's status.
         */
        void updateChatStatus(int status);

        /**
         * Add chat event information like quit or rejoin.
         * 
         * @param information the type of chat event to add.
         * @return chat event information.
         */
        IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct);
        
        void addgroupSubject(String subject);
        /**
         * Add a sent file transfer into VIEW module
         * 
         * @param file The file to be sent
         * @return A IFileTransfer to MODEL module
         */
        IFileTransfer addSentFileTransfer(FileStruct file);

        /**
         * Add a received file transfer into VIEW module
         * 
         * @param file The file received
         * @return A IFileTransfer to MODEL module
         */
        IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept);
    }

    /**
     * This interface defines a chat window manager that manages the chat
     * windows.
     */
    public interface IChatWindowManager {

        /**
         * Add a 1-2-1 chat window
         * 
         * @param participant The remote chat participant.
         * @return The tag of this chat window
         */
        IOne2OneChatWindow addOne2OneChatWindow(Object tag, Participant participant);

        /**
         * Add a group chat window
         * 
         * @param participantInfoList The list of the participants
         * @return The tag of this chat window
         */
        IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList);

        /**
         * Remove a chat window
         * 
         * @param chatWindow The chat window to be removed
         * @return TRUE if the chat window has been removed successfully FALSE
         *         if the chat window does not exist in the ChatWindowManager
         */
        boolean removeChatWindow(IChatWindow chatWindow);

        /**
         * Show a fragment identified by tag.
         * 
         * @param uuidTag A fragment's tag.
         */
        void switchChatWindowByTag(ParcelUuid uuidTag);
    }
}
