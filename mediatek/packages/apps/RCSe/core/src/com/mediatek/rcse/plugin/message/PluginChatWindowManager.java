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
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.RemoteException;
import android.provider.Telephony.Sms;
import android.text.TextUtils;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.service.binder.IRemoteChatWindowManager;
import com.mediatek.rcse.service.binder.IRemoteGroupChatWindow;
import com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Plugin ChatWindow manager
 */
public class PluginChatWindowManager extends IRemoteChatWindowManager.Stub {

    private static final String TAG = "PluginChatWindowManager";
    private static final List<WindowTagGetter> CHAT_WINDOW_LIST = new CopyOnWriteArrayList<WindowTagGetter>();
    private static final String[] PROJECTION_ADDRESS = {
        Sms.ADDRESS
    };
    public static final String SELECTION_THREAD_ID = Sms.THREAD_ID + "=?";
    private IpMessageManager mMessageManager;

    /**
     * Implementing this interface to indicate that this class has a window tag within
     */
    public interface WindowTagGetter {
        String getWindowTag();
    }

    /*package*/ static void startGroupChatDetailActivity(final long threadId, final Context context) {
        Logger.d(TAG, "startGroupChatDetailActivity() entry, threadId: " + threadId);
        if (threadId > -1 && null != context) {
            ContentResolver contentResolver = context.getContentResolver();
            Cursor cursor = null;
            try {
                String[] args = {
                    Long.toString(threadId)
                };
                cursor = contentResolver.query(PluginUtils.SMS_CONTENT_URI, PROJECTION_ADDRESS,
                        SELECTION_THREAD_ID, args, Sms.DEFAULT_SORT_ORDER);
                if (cursor.moveToFirst()) {
                    String contactString = cursor.getString(cursor.getColumnIndex(Sms.ADDRESS));
                    WindowTagGetter targetWindow = findGroupWindowTag(contactString);
                    if (null != targetWindow) {
                        String tagString = targetWindow.getWindowTag();
                        Logger.d(TAG, "startGroupChatDetailActivity() start group chat window"
                                + " with tagString: " + tagString);
                        Intent intent = new Intent(PluginGroupChatActivity.ACTION);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, tagString);
                        context.startActivity(intent);
                    } else {
                        Logger.w(TAG, "startGroupChatDetailActivity() "
                                + "unable to find window for contactString: contactString");
                    }
                } else {
                    Logger.w(TAG, "startGroupChatDetailActivity() empty cursor");
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            Logger.w(TAG, "startGroupChatDetailActivity() invalid thread id or context: " + context);
        }
    }

    private static WindowTagGetter findGroupWindowTag(String targetContact) {
        if (TextUtils.isEmpty(targetContact)) {
            Logger.w(TAG, "findGroupWindowTag() invalid targetContact");
            return null;
        }
        Collection<WindowTagGetter> windows = new ArrayList<WindowTagGetter>(CHAT_WINDOW_LIST);
        for (WindowTagGetter chatWindow : windows) {
            if (chatWindow instanceof PluginGroupChatWindow) {
                String contactString = ((PluginGroupChatWindow)chatWindow).getContactString();
                if (targetContact.equals(contactString)) {
                    String chatWindowTag = ((PluginGroupChatWindow)chatWindow).getWindowTag();
                    Logger.d(TAG, "findGroupWindowTag(), find chatWindowTag: " + chatWindowTag
                            + " for targetContact: " + targetContact);
                    return chatWindow;
                }
            }
        }
        return null;
    }

    /**
     * Get contact string of a group chat
     * 
     * @param groupId The group chat id
     * @return The contact string of the group chat
     */
    public static String getNumberByEngineId(short groupId) {
        Logger.d(TAG, "getNumberByEngineId() entry with groupId: " + groupId);
        Collection<WindowTagGetter> windows = new ArrayList<WindowTagGetter>(CHAT_WINDOW_LIST);
        for (WindowTagGetter chatWindow : windows) {
            if (chatWindow instanceof PluginGroupChatWindow) {
                Short id = ((PluginGroupChatWindow) chatWindow).getGroupId();
                if (groupId == id) {
                    String groupName = ((PluginGroupChatWindow) chatWindow).getContactString();
                    Logger.d(TAG, "getNumberByEngineId(), find groupName: " + groupName
                            + " for groupId: " + groupId);
                    return groupName;
                }
            }
        }
        return null;
    }

    /**
     * Get chat window  of a contact
     * @param contact The contact of this chat window
     * @return The chat window or null if the chat window does not exist
     */
    public static WindowTagGetter findWindowTagIndex(String contact) {
        Logger.d(TAG, "findWindowTagIndex() entry the contact is " + contact);
        if (TextUtils.isEmpty(contact)) {
            Logger.e(TAG, "findWindowTagIndex() entry contact is null");
            return null;
        }
        Collection<WindowTagGetter> windows = new ArrayList<WindowTagGetter>(CHAT_WINDOW_LIST);
        if (contact.startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
            for (WindowTagGetter chatWindow : windows) {
                if (chatWindow instanceof PluginGroupChatWindow) {
                    String contactString = ((PluginGroupChatWindow) chatWindow).getContactString();
                    if (null != contactString && contact.equals(contactString)) {
                        String chatWindowTag = ((PluginGroupChatWindow) chatWindow).getWindowTag();
                        Logger.d(TAG, "findWindowTagIndex(), find chatWindowTag: " + chatWindowTag
                                + " for contact: " + contact);
                        return chatWindow;
                    }
                }
            }
        } else {
            for (WindowTagGetter chatWindow : windows) {
                if (chatWindow instanceof PluginOne2OneChatWindow) {
                    Participant participant = ((PluginOne2OneChatWindow) chatWindow)
                            .getParticipant();
                    if (null != participant && contact.equals(participant.getContact())) {
                        String chatWindowTag = ((PluginOne2OneChatWindow) chatWindow)
                                .getWindowTag();
                        Logger.d(TAG, "findWindowTagIndex(), find chatWindowTag: " + chatWindowTag
                                + " for contact: " + contact);
                        return chatWindow;
                    }
                }
            }
        }
        return null;
    }

    public PluginChatWindowManager(IpMessageManager messageManager) {
        mMessageManager = messageManager;
    }
 
    @Override
    public IRemoteOne2OneChatWindow addOne2OneChatWindow(String tag, Participant participant)
            throws RemoteException {
        Logger.d(TAG, "addOne2OneChatWindow(), tag = " + tag + "participant = " + participant);
        if (null != participant) {
            WindowTagGetter existingWindow = findWindowTagIndex(participant.getContact());
            if (null != existingWindow) {
                Logger.w(TAG, "addOne2OneChatWindow() One2OneChatWindow for Participant: " + participant
                        + " found, remove it first");
                CHAT_WINDOW_LIST.remove(existingWindow);
            }
            PluginOne2OneChatWindow pluginOne2OneChatWindow = new PluginOne2OneChatWindow(participant, tag, mMessageManager);
            CHAT_WINDOW_LIST.add(pluginOne2OneChatWindow);
            return pluginOne2OneChatWindow;
        } else {
            Logger.w(TAG, "addOne2OneChatWindow() participant is null");
            return null;
        }
    }

    @Override
    public IRemoteGroupChatWindow addGroupChatWindow(String tag, List<ParticipantInfo> participantList)
            throws RemoteException {
        Logger.d(TAG, "addGroupChatWindow(), tag = " + tag + "participantList = " + participantList);
        PluginGroupChatWindow groupChatWindow = new PluginGroupChatWindow(tag, mMessageManager, participantList);
        CHAT_WINDOW_LIST.add(groupChatWindow);
        return groupChatWindow;
    }

    @Override
    public boolean removeGroupChatWindow(IRemoteGroupChatWindow chatWindow) throws RemoteException {
        Logger.d(TAG, "removeGroupChatWindow(), chatWindow = " + chatWindow);
        ((PluginGroupChatWindow) chatWindow).onDestroy();
        return false;
    }

    @Override
    public boolean removeOne2OneChatWindow(IRemoteOne2OneChatWindow chatWindow) throws RemoteException {
        Logger.d(TAG, "removeOne2OneChatWindow(), chatWindow = " + chatWindow);
        return CHAT_WINDOW_LIST.remove(chatWindow);
    }

    @Override
    public void switchChatWindowByTag(String uuidTag) throws RemoteException {
        Logger.d(TAG, "switchChatWindowByTag(), uuidTag = " + uuidTag);
    }

}