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
import com.mediatek.rcse.service.binder.IRemoteChatWindow;
import com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ParticipantInfo;


/**
 * This interface defines a chat window manager that manages the chat
 * windows.
 */
interface IRemoteChatWindowManager {

    /**
     * Add a 1-2-1 chat window
     * 
     * @param participant The remote chat participant.
     * @return The tag of this chat window
     */
    IRemoteOne2OneChatWindow addOne2OneChatWindow(String tag, in Participant participant);

    /**
     * Add a group chat window
     * 
     * @param participantInfoList The list of the participants
     * @return The tag of this chat window
     */
    IRemoteGroupChatWindow addGroupChatWindow(String tag, in List<ParticipantInfo> participantList);

    /**
     * Remove a One2OneChat window
     * 
     * @param IOne2OneChatWindow The chat window to be removed
     * @return TRUE if the chat window has been removed successfully FALSE
     *         if the chat window does not exist in the ChatWindowManager
     */
    boolean removeOne2OneChatWindow(IRemoteOne2OneChatWindow chatWindow);
    
    /**
     * Remove a GroupChatWindow window
     * 
     * @param IGroupChatWindow The chat window to be removed
     * @return TRUE if the chat window has been removed successfully FALSE
     *         if the chat window does not exist in the ChatWindowManager
     */
    boolean removeGroupChatWindow(IRemoteGroupChatWindow chatWindow);

    /**
     * Show a fragment identified by tag.
     * 
     * @param uuidTag A fragment's tag.
     */
    void switchChatWindowByTag(in String uuidTag);
}

