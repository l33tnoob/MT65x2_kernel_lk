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


import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.IRemoteChatWindowMessage;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.service.binder.ChatEventStructForBinder;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;

import java.util.List;

/**
 * This interface defines one type of chat window that used for a group chat
 */
interface IRemoteGroupChatWindow {

    /**
     * Update the participant list
     * 
     * @param participantList The latest participant list
     */
    void updateParticipants(in List<ParticipantInfo> participants);

    /**
     * Set whether a specific participant is composing
     * 
     * @param isComposing Whether this participant is composing
     * @param participant The participant who is composing or not
     */
    void setIsComposing(boolean isComposing, in Participant participant);

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
    int addChatEventInformation(in ChatEventStructForBinder chatEventStruct);
    
    /**
     * Add a received message into this chat window.
     * 
     * @param message The message to add.
     * @param isRead identify if the message is read.
     * @return IReceivedChatMessage converted from InstantMessage.
     */
    IRemoteReceivedChatMessage addReceivedMessage(in InstantMessage message, boolean isRead);

    /**
     * Add a received message into this chat window.
     * 
     * @param message The message to add.
     * @param messageTag The tag of this message
     * @return ISentChatMessage converted from InstantMessage.
     */
    IRemoteSentChatMessage addSentMessage(in InstantMessage message, int messageTag);

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
    IRemoteSentChatMessage getSentChatMessage(String messageId);

    /**
     * add load history view.
     */
    void addLoadHistoryHeader(boolean showLoader);

    /**
     * update all the message as read.
     */
    void updateAllMsgAsRead();
     /**
     * add Group Subject
     */
    void addgroupSubject(String subject);
    void setFileTransferEnable(int reason);
    IRemoteFileTransfer addSentFileTransfer(in FileStructForBinder file);
    IRemoteFileTransfer addReceivedFileTransfer(in FileStructForBinder file, boolean isAutoAccept);
    
}
