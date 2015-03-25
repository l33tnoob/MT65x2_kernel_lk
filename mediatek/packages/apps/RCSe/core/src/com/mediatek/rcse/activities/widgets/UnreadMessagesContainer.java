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

import android.os.ParcelUuid;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.GroupChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ViewImpl;

import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.util.List;

/**
 * Container for unread messages.
 */
public class UnreadMessagesContainer {
    private static final String TAG = "UnreadMessagesContainer";
    private UnreadMessage mHead = null;
    private UnreadMessage mTail = null;
    private static UnreadMessagesContainer sUnreadMessagesContainer = null;

    static final class UnreadMessage {
        public UnreadMessage prev;
        public UnreadMessage next;
        public ParcelUuid tag;
    }

    private UnreadMessagesContainer() {

    }

    /**
     * Get instance of UnreadMessagesContainer.
     * 
     * @return The instance of UnreadMessagesContainer;
     */
    public static synchronized UnreadMessagesContainer getInstance() {
        Logger.v(TAG, "getInstance entry");
        if (sUnreadMessagesContainer == null) {
            Logger.v(TAG, "getInstance create");
            sUnreadMessagesContainer = new UnreadMessagesContainer();
        } else {
            Logger.d(TAG, "getInstance exit with not null");
        }
        return sUnreadMessagesContainer;
    }

    /**
     * Add unread message chat to the queue.
     * 
     * @param tag The tag of the unread message' chat.
     */
    public synchronized void add(ParcelUuid tag) {
        Logger.d(TAG, "add entry, the tag is: " + tag);
        if (tag == null) {
            Logger.d(TAG, "add tag is null");
            return;
        }
        UnreadMessage unreadMsg = new UnreadMessage();
        unreadMsg.tag = tag;
        remove(tag);
        if (mHead == null) {
            mTail = unreadMsg;
            mHead = mTail;
        } else if (null != mTail) {
            unreadMsg.prev = mTail;
            mTail.next = unreadMsg;
            mTail = unreadMsg;
        } else {
            Logger.w(TAG, "add() invalid mHead: " + mHead + " , mTail: " + mTail);
        }
        Logger.d(TAG, "add exit");
    }

    /**
     * Remove unread message chat to the queue.
     * 
     * @param tag The tag of the unread message' chat.
     */
    public synchronized void remove(ParcelUuid tag) {
        Logger.d(TAG, "remove entry, the tag is: " + tag);
        if (tag == null) {
            Logger.d(TAG, "remove tag is null");
            return;
        }
        if (mHead == null) {
            Logger.d(TAG, "remove mHead is null");
            return;
        }
        UnreadMessage unreadMsg = mTail;
        while (unreadMsg != null) {
            if (tag.equals(unreadMsg.tag)) {
                UnreadMessage prev = unreadMsg.prev;
                UnreadMessage next = unreadMsg.next;
                if (prev == null) {
                    if (next == null) {
                        mTail = null;
                        mHead = mTail;
                    } else {
                        mHead = next;
                        mHead.prev = null;
                    }
                } else {
                    if (next == null) {
                        mTail = prev;
                        mTail.next = null;
                    } else {
                        prev.next = next;
                        next.prev = prev;
                    }
                }
                break;
            } else {
                unreadMsg = unreadMsg.prev;
            }
        }
        Logger.d(TAG, "remove exit");
    }

    /**
     * Get the latest unread message.
     * 
     * @return The latest unread message.
     */
    public synchronized ParcelUuid getLatestUnreadMessageChat() {
        Logger.d(TAG, "getLatestUnreadMessageChat entry");
        if (mTail == null) {
            Logger.d(TAG, "getLatestUnreadMessageChat mTail is null");
            return null;
        }
        Logger.d(TAG, "getLatestUnreadMessageChat exit");
        return mTail.tag;
    }

    /**
     * Get the latest unread message.
     * 
     * @return The latest unread message.
     */
    public synchronized InstantMessage getLatestUnreadMessage() {
        Logger.d(TAG, "getLatestUnreadMessageChat entry");
        if (mTail == null) {
            Logger.d(TAG, "getLatestUnreadMessageChat mTail is null");
            return null;
        }
        IChatManager instance = ModelImpl.getInstance();
        if (instance == null) {
            Logger.d(TAG, "getLatestUnreadMessageChat instance is null");
            return null;
        } else {
            List<InstantMessage> messages = null;
            IChat chat = instance.getChat(mTail.tag);
            if (chat instanceof One2OneChat) {
                messages = ((One2OneChat) chat).getUnreadMessages();
            } else if (chat instanceof GroupChat) {
                messages = ((GroupChat) chat).getUnreadMessages();
            }
            if (messages == null) {
                Logger.d(TAG, "getLatestUnreadMessage messages is null");
                return null;
            } else {
                Object[] messageArray = messages.toArray();
                int length = messageArray.length;
                if (length == 0) {
                    Logger.d(TAG, "getLatestUnreadMessage messages size is 0");
                    return null;
                } else {
                    InstantMessage message = (InstantMessage) messageArray[length - 1];
                    if (null != message) {
                        Logger.d(TAG, "getLatestUnreadMessage exit, the message text is "
                                + message.getTextMessage() + " the remote contact is "
                                + message.getRemote());
                        return message;
                    } else {
                        Logger.e(TAG, "getLatestUnreadMessage exit, the message is null");
                        return null;
                    }
                }
            }
        }
    }

    /**
     * Load latest unread message in current focused window.
     */
    public synchronized void loadLatestUnreadMessage() {
        Logger.d(TAG, "loadLatestUnreadMessage entry");
        InstantMessage unreadMessage = getLatestUnreadMessage();
        ChatScreenWindow window = ChatScreenWindowContainer.getInstance().getFocusWindow();
        if (mTail != null) {
            ParcelUuid currentTag = ChatScreenWindowContainer.getInstance().getCurrentTag();
            if (mTail.tag.equals(currentTag)) {
                Logger.d(TAG, "loadLatestUnreadMessage should not load unread message");
                return;
            } else {
                Logger.d(TAG, "loadLatestUnreadMessage " + mTail.tag);
                if (unreadMessage != null && window != null) {
                    window.addUnreadMessage(unreadMessage);
                }
            }
        } else {
            Logger.d(TAG, "loadLatestUnreadMessage mTail is null");
        }
    }

    /**
     * Switch the chat window to another window with tag.
     * 
     * @param tag The tag of chat window to be switched to.
     * @param windowManager The window manager in which the window will be
     *            switched.
     */
    public synchronized void switchTo(ParcelUuid tag, ChatScreenActivity.ChatWindowManager windowManager) {
        Logger.d(TAG, "switchTo() entry, target tag: " + tag);

        ViewImpl instance = ViewImpl.getInstance();
        if (instance == null | null == tag) {
            Logger.e(TAG, "switchTo() ViewImpl instance is " + instance + ", tag is" + tag);
            return;
        }
        windowManager.disconnect();
        ChatScreenWindowContainer container = ChatScreenWindowContainer.getInstance();
        ChatScreenWindow window = container.getFocusWindow();
        container.clearCurrentStatus();

        if (window != null) {
            Object currentWindowTag = window.getTag();
            Logger.d(TAG, "switchTo() current window is " + currentWindowTag);
            if (!tag.equals(currentWindowTag)) {
                // Make current window background
                window.pause();
                container.focus(tag);
                container.focus((List<Participant>) null);
                windowManager.connect(true);
                window = container.getFocusWindow();
                if (window != null) {
                    // Make target window foreground
                    window.resume();
                } else {
                    Logger.w(TAG, "switchTo() next window is null");
                }
            } else {
                Logger.w(TAG, "switchTo() duplicate tag, skip it");
            }
        } else {
            Logger.w(TAG, "switchTo() current window is null");
        }

        Logger.d(TAG, "switchTo() exit");
    }
}
