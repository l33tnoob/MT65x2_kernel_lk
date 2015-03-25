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

import android.os.Message;

/**
 * As part of the MVC pattern, this class is the Controller. This class only
 * define the interface.
 */
public final class ChatController {
    // Common class should not be instantiated
    private ChatController() {
    };

    private static final int EVENT_BASE_FT = 100;

    private static final int EVENT_BASE_WINDOW = 200;

    private static final int EVENT_BASE_ACTION_MESSAGE = 300;

    /**
     * This event indicates the user wants to open or create a chat window
     */
    public static final int EVENT_OPEN_WINDOW = EVENT_BASE_WINDOW + 1;

    /**
     * This event indicates that a specific chat window has become foreground
     */
    public static final int EVENT_SHOW_WINDOW = EVENT_BASE_WINDOW + 2;

    /**
     * This event indicates that a specific chat window has become background
     */
    public static final int EVENT_HIDE_WINDOW = EVENT_BASE_WINDOW + 3;

    /**
     * This event indicates that a specific chat window has been closed
     */
    public static final int EVENT_CLOSE_WINDOW = EVENT_BASE_WINDOW + 4;
    
    /**
     * This event indicates that a specific chat window has been closed
     */
    public static final int EVENT_CLOSE_ALL_WINDOW = EVENT_BASE_WINDOW + 5;

    /**
     * This event indicates that a specific chat window has been closed but stays in Database
     */
    public static final int EVENT_CLOSE_ALL_WINDOW_FROM_MEMORY = EVENT_BASE_WINDOW + 6;
    
    /**
     * This event indicates the user wants to send out a message from a chat
     * window
     */
    public static final int EVENT_SEND_MESSAGE = EVENT_BASE_ACTION_MESSAGE + 1;

    /**
     * This event indicates the user wants to load the chat history in a chat
     * window
     */
    public static final int EVENT_GET_CHAT_HISTORY = EVENT_BASE_ACTION_MESSAGE + 2;

    /**
     * This event indicates the user is typing in a chat window
     */
    public static final int EVENT_TEXT_CHANGED = EVENT_BASE_ACTION_MESSAGE + 3;

    /**
     * This event indicates the user wants to add some contacts into a 1-2-1
     * chat and make it become a group chat
     */
    public static final int EVENT_ONEONE_TO_GROUP = EVENT_BASE_ACTION_MESSAGE + 4;

    /**
     * This event indicates the user wants to clear the chat screen of a chat
     * window
     */
    public static final int EVENT_CLEAR_CHAT_HISTORY = EVENT_BASE_ACTION_MESSAGE + 5;

    /**
     * This event indicates the user wants to add some contacts into a group
     * chat
     */
    public static final int EVENT_GROUP_ADD_PARTICIPANT = EVENT_BASE_ACTION_MESSAGE + 6;

    /**
     * This event indicates the user wants to quit from a group chat
     */
    public static final int EVENT_QUIT_GROUP_CHAT = EVENT_BASE_ACTION_MESSAGE + 7;

    /**
     * This event will trigger a capability exchange with a specific contact
     */
    public static final int EVENT_QUERY_CAPABILITY = EVENT_BASE_ACTION_MESSAGE + 8;

    /**
     * This event will cause ModelImpl to reload specific messages
     */
    public static final int EVENT_RELOAD_MESSAGE = EVENT_BASE_ACTION_MESSAGE + 9;

    /**
     * This event indicates the user wants to re-send an outgoing file transfer
     * invitation
     */
    public static final int ADD_GROUP_SUBJECT = EVENT_BASE_ACTION_MESSAGE + 10;

    /**
     * This event indicates the user wants to re-send an outgoing file transfer
     * invitation
     */
    public static final int EVENT_DELETE_MESSAGE = EVENT_BASE_ACTION_MESSAGE + 11;
    
    /**
     * This event indicates the user wants to send out a file transfer
     * invitation to a contact
     */
    public static final int EVENT_FILE_TRANSFER_INVITATION = EVENT_BASE_FT + 1;

    /**
     * This event indicates the user has accepted an incoming file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_RECEIVER_ACCEPT = EVENT_BASE_FT + 2;

    /**
     * This event indicates the user has rejected an incoming file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_RECEIVER_REJECT = EVENT_BASE_FT + 3;

    /**
     * This event indicates the user has canceled an outgoing file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_CANCEL = EVENT_BASE_FT + 4;

    /**
     * This event indicates the user wants to re-send an outgoing file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_RESENT = EVENT_BASE_FT + 5;

    /**
     * This event indicates the user wants to re-send an outgoing file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_PAUSE = EVENT_BASE_FT + 6;


	 /**
     * This event indicates the user wants to re-send an outgoing file transfer
     * invitation
     */
    public static final int EVENT_FILE_TRANSFER_RESUME = EVENT_BASE_FT + 7;

    /**
     * This interface defines a chat controller
     */
    public interface IChatController {
        /**
         * Send message to controller,so that notify the controller to do
         * things. Notice that before call this method, you should call
         * {@link #obtainMessage(EVENT_TYPE, Object, Object)} to create a
         * message
         * 
         * @param message The message you want to send to controller.
         */
        void sendMessage(Message message);

        /**
         * Returns a new {@link android.os.Message Message} from the global
         * message pool. More efficient than creating and allocating new
         * instances. The retrieved message has its handler set to this instance
         * (Message.target == this). If you don't want that facility, just call
         * Message.obtain() instead.
         * 
         * @param event_type Message type.
         * @param tag The tag of your chat window, it's used to identify a chat
         *            window you want to manipulate.
         * @param message The message you want to send to remote participant.
         *            It's always null, but not null only when you need send
         *            message to remote participant.
         * @return A new {@link android.os.Message Message} from the global
         *         message pool.
         */

        Message obtainMessage(int eventType, Object tag, Object message);
    }
}
