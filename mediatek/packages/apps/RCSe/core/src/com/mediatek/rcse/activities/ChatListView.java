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

package com.mediatek.rcse.activities;

import android.app.Activity;

import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatListFragment.ChatMap;
import com.mediatek.rcse.activities.widgets.AsyncAvatarView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.DateView;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

/**
 * This class used to list all the chat invitations and chats information
 */
public final class ChatListView extends View implements OnClickListener, ContactsListManager.OnDisplayNameChangedListener {

    private static final String TAG = "ChatListView";
    private static final String NUMBER = "number";
    // Invitation header bar
    private View mInviteHeader = null;
    // Invitation container
    private LinearLayout mInviteContainer = null;
    // Invitation container
    private LinearLayout mChatContainer = null;
    // OnSelectedItemListener instance
    private OnSelectedItemListener mListener = null;
    // Activity instance
    private Activity mActivity = null;
    // The root view
    private View mRootView = null;
    // Used to store the invitation information
    private final HashMap<View, Object> mData = new HashMap<View, Object>();
    // Invalid position
    private static final int INVALID_POSITION = -1;
    // The blank text
    private static final String BLANK_STRING = "";

    private final Handler mMainHandler = new Handler(Looper.getMainLooper());

    private static final int MIN_NUMBER = 0;

    private static final int MAX_DISPLAYED_DIGITAL_NUMBER = 99;
    private static final String MAX_DISPLAYED_STRING_NUMBER = "99+";

    private OnCreateContextMenuListener mContextMenuListener = null;

    private static final int MAX_NUMBER_ICON_WIDTH = 36;
    private static final int MIN_NUMBER_ICON_WIDTH = 24;
    public static final String LEFT_BRACKET = "(";
    public static final String RIGHT_BRACKET = ")";

    /**
     * Set listener for context menu.
     * 
     * @param listener The listener for context menu.
     */
    public void setContextMenuListener(OnCreateContextMenuListener listener) {
        mContextMenuListener = listener;
    }

    /**
     * The listener provides a listener to return the selected item
     */
    public interface OnSelectedItemListener {
        void onSelectedItem(View view, int position);
    }

    /**
     * Constructor method
     * 
     * @param context Current context instance
     * @param rootView Current root view
     * @param listener Current OnSelecteditemListener instance
     */
    public ChatListView(Activity activity, OnSelectedItemListener listener) {
        super(activity);
        mActivity = activity;
        mRootView = LayoutInflater.from(mActivity).inflate(R.layout.chats_list_layout, null);
        mListener = listener;
        mInviteContainer = (LinearLayout) mRootView.findViewById(R.id.invite_container_layout);
        mInviteHeader = mRootView.findViewById(R.id.invite_header_layout);
        mChatContainer = (LinearLayout) mRootView.findViewById(R.id.chat_container_layout);
    }
    public LinearLayout getChatContainer()
    {
    	return mChatContainer;
    }
    
    public LinearLayout getInviteContainer()
    {
    	return mInviteContainer;
    }

    /**
     * Release the maintained resources
     */
    public void destroy() {
        Logger.d(TAG, "onDestroy() entry");
        if (mActivity != null) {
            mActivity = null;
        }
        if (mListener != null) {
            mListener = null;
        }
        if (mData != null) {
            mData.clear();
        }
        Logger.d(TAG, "onDestroy() exit");
    }

    /**
     * Called to get the root view
     */
    public View getRootView() {
        return mRootView;
    }

    @Override
    public void onClick(View v) {
        if (mData.containsKey(v)) {
            int position = ChatListView.this.getPosition(mData.get(v));
            mListener.onSelectedItem(v, position);
        }
    }

    /**
     * Update the specify invitation item
     * 
     * @param view The item to update
     * @param inviteStructAfterChange The data to set to the item
     */
    private void updateInvitations(final View view, final InvitationStruct inviteStructAfterChange) {
        if (mMainHandler != null) {
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    // Date info
                    DateView date = (DateView) view.findViewById(R.id.invite_date);
                    if (inviteStructAfterChange != null) {
                        date.setTime(inviteStructAfterChange.getDate(), false);
                        // Information
                        TextView textInfo = (TextView) view.findViewById(R.id.invite_infor);
                        String information = inviteStructAfterChange.getInformation();
                        if (information != null && textInfo != null) {
                            textInfo.setText(information);
                        } else {
                            Logger.d(TAG, "The last message is null");
                        }
                    } else {
                        Logger.e(TAG, "updateInvitations the inviteStructAfterChange is null");
                    }
                }
            });
        }
    }

    /**
     * Update the specify chats item
     * 
     * @param view The item to update
     * @param chatsStructAfterChange The data to set to the item
     */
    private void updateChats(final View view, final ChatsStruct chatsStructAfterChange) {
        if (mMainHandler != null) {
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    final TextView unreadMessage =
                            (TextView) view.findViewById(R.id.unread_message_num);
                    final ImageView fileTransfer =
                            (ImageView) view.findViewById(R.id.chat_file_transfer);
                    if (chatsStructAfterChange.getIsFileTransfer()) {
                        fileTransfer.setVisibility(View.VISIBLE);
                    } else {
                        fileTransfer.setVisibility(View.GONE);
                    }
                    final int number = chatsStructAfterChange.getUnreadMessageNumber();
                    if (number <= MIN_NUMBER) {
                        unreadMessage.setVisibility(View.GONE);
                    } else {
                        // Unread message info
                        unreadMessage.setVisibility(View.VISIBLE);
                        if (number > MAX_DISPLAYED_DIGITAL_NUMBER) {
                            unreadMessage.setWidth(MAX_NUMBER_ICON_WIDTH);
                            unreadMessage.setText(MAX_DISPLAYED_STRING_NUMBER);
                        } else {
                            unreadMessage.setWidth(MIN_NUMBER_ICON_WIDTH);
                            unreadMessage.setText(String.valueOf(number));
                        }
                    }
                    View displayNameView = view.findViewById(R.id.chat_remote_name);
                    if (null != displayNameView) {
                        String displayName = chatsStructAfterChange.getDisplayName();
                        Logger.d(TAG, "updateChats()->run() displayName is " + displayName);
                        ((TextView) displayNameView).setText(displayName);
                    } else {
                        Logger.e(TAG, "updateChats()->run() textViewInfo is null");
                    }

                    if (chatsStructAfterChange.isGroupChat()) {
                        Logger.d(TAG, "updateChats()->run() this is a group chat");
                        TextView participantNum =
                                (TextView) view.findViewById(R.id.chat_remote_num);
                        StringBuilder chatNum = new StringBuilder();
                        chatNum.append(LEFT_BRACKET).append(
                                (chatsStructAfterChange.getParticipantNum()) + ChatFragment.ONE)
                                .append(RIGHT_BRACKET);
                        if(chatsStructAfterChange.getmSubject().equals(""))
                        participantNum.setText(chatNum.toString());
                        else
                        {     
                            ((TextView) displayNameView).setText(chatsStructAfterChange.getmSubject());
                            participantNum.setText("");
                        }
                        AsyncAvatarView avatarImageView =
                            (AsyncAvatarView) view.findViewById(R.id.chat_contact_quick);
                        avatarImageView.setAsyncContact(chatsStructAfterChange.getContacts());
                    } else {
                        Logger.d(TAG, "updateChats()->run() this is a 1-2-1 chat");
                    }

                    // Date info
                    DateView date = (DateView) view.findViewById(R.id.chat_date);
                    date.setTime(chatsStructAfterChange.getDate(), false);
                    // Last message
                    TextView lastMessage = (TextView) view.findViewById(R.id.chat_last_message);
                    String message = chatsStructAfterChange.getLatestMessage();
                    if (message == null) {
                        message = BLANK_STRING;
                    }
                    lastMessage.setText(message);
                }
            });
        }
    }

    /**
     * Update the list view
     */
    public void updateView() {
        if (mData.isEmpty()) {
            Logger.d(TAG, "The mData is empty");
            return;
        }
        Set<View> viewSet = mData.keySet(); // keySet() returns [] if map is empty
        Iterator<View> viewIterator = viewSet.iterator();
        if (viewIterator == null) {
            Logger.d(TAG, "The viewIterator is null");
            return;
        }
        while (viewIterator.hasNext()) {
            View view = viewIterator.next();
            if (view == null) {
                Logger.d(TAG, "The view is null");
            } else {
                Object obj = mData.get(view);
                if (obj == null) {
                    Logger.d(TAG, "The value is null");
                } else {
                    if (obj instanceof ChatsStruct) {
                        ChatsStruct chatStruct = (ChatsStruct) obj;
                        updateChats(view, chatStruct);
                    } else if (obj instanceof InvitationStruct) {
                        InvitationStruct inviteStruct = (InvitationStruct) obj;
                        updateInvitations(view, inviteStruct);
                    } else {
                        Logger.d(TAG, "Unknown view type");
                    }
                }
            }
        }
    }

    /**
     * Update chat item with the specific tag.
     * 
     * @param tag The tag of the chat item
     */
    public void updateChat(Object tag) {
        if (null == tag || mData.isEmpty()) {
            Logger.d(TAG, "updateChat() invalid mData: " + mData + " or tag: " + tag);
            return;
        }
        Set<View> viewSet = mData.keySet();
        Iterator<View> viewIterator = viewSet.iterator();
        if (viewIterator == null) {
            Logger.d(TAG, "updateChat() The viewIterator is null");
            return;
        }
        while (viewIterator.hasNext()) {
            View view = viewIterator.next();
            if (view == null) {
                Logger.d(TAG, "updateChat() The view is null");
            } else {
                Object obj = mData.get(view);
                if (obj == null) {
                    Logger.d(TAG, "updateChat() The value is null");
                } else {
                    if (obj instanceof ChatsStruct) {
                        ChatsStruct chatStruct = (ChatsStruct) obj;
                        if (tag.equals(chatStruct.getWindowTag())) {
                            updateChats(view, chatStruct);
                        } else {
                            Logger.d(TAG, "updateChat() not the chatStruct with the tag" + tag);
                        }
                    } else {
                        Logger.d(TAG, "updateChat() the chat struct is " + obj);
                    }
                }
            }
        }
    }

    /**
     * Add invite item
     * 
     * @param type The item type
     * @param obj The item instance
     */
    public void addInviteItem(Object obj) {
        Logger.d(TAG, "addInviteItem() entry, obj: " + obj);
        if (obj != null) {
            mInviteHeader.setVisibility(View.VISIBLE);
            InvitationStruct invite = (InvitationStruct) obj;
            View view = LayoutInflater.from(mActivity).inflate(R.layout.invite_item, null);
            AsyncAvatarView avatarImageView =
                    (AsyncAvatarView) view.findViewById(R.id.invite_contact_quick);
            try {
                String phoneNumUri = invite.getChatSession().getRemoteContact();
                String phoneNum = PhoneUtils.extractNumberFromUri(phoneNumUri);
                if (phoneNum != null) {
                    Logger.w(TAG, "addInviteItem  the phoneNum is " + phoneNum);
                    avatarImageView.setAsyncContact(phoneNum);
                } else {
                    Logger.e(TAG, "addInviteItem  the phoneNum is null");
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            TextView textViewInfo = (TextView) view.findViewById(R.id.invite_infor);
            textViewInfo.setText(invite.getInformation());
            DateView textViewDate = (DateView) view.findViewById(R.id.invite_date);
            textViewDate.setTime(invite.getDate(), false);
            mInviteContainer.addView(view, 0);
            view.setOnClickListener(this);
            mData.put(view, invite);
        } else {
            Logger.e(TAG, "addInviteItem(), invite is null");
        }
    }

    /**
     * Add chat item
     * 
     * @param type The item type
     * @param obj The item instance
     */
    public void addChatItem(Object obj) {
        Logger.d(TAG, "addChatItem() entry the obj is " + obj);
        TextView noChatView = (TextView) mChatContainer.findViewById(R.id.no_chat);
        if (noChatView != null) {
            noChatView.setVisibility(View.GONE);
        } else {
            Logger.e(TAG, "addChatItem(), noChatView is null!");
        }
        ChatsStruct chat = (ChatsStruct) obj;
        View view = LayoutInflater.from(mActivity).inflate(R.layout.chat_item, null);
        view.setOnClickListener(this);
        if (mChatContainer.getChildCount() == 1) {
            view.findViewById(R.id.under_line).setVisibility(View.GONE);
        } else {
            view.findViewById(R.id.under_line).setVisibility(View.VISIBLE);
        }

        mChatContainer.addView(view, 0);
        view.setOnCreateContextMenuListener(mContextMenuListener);
        AsyncAvatarView avatarImageView =
                (AsyncAvatarView) view.findViewById(R.id.chat_contact_quick);

        TextView textViewInfo = (TextView) view.findViewById(R.id.chat_remote_name);
        TextView participantNum = (TextView) view.findViewById(R.id.chat_remote_num);
        if (chat != null) {
            if (chat.isGroupChat()) {
                HashMap chatMap = chat.getChatMap();
                if (chatMap != null) {
                    TreeSet<String> nums = new TreeSet<String>();
                    List<String> nunList = (List<String>) chatMap.get(ChatListFragment.NUMBER);
                    for (String number : nunList) {
                        nums.add(number);
                    }
                    avatarImageView.setAsyncContact(nums);
                }
                participantNum.setVisibility(View.VISIBLE);
                StringBuilder chatNum = new StringBuilder();
                chatNum.append(LEFT_BRACKET).append(chat.getParticipantNum()).append(RIGHT_BRACKET);
                participantNum.setText(chatNum.toString());
            } else {
                Logger.d(TAG, "addChatItem() chat.isone2oneChat");
                HashMap chatMap = chat.getChatMap();
                if (chatMap != null) {
                    String phoneNum = (String) chatMap.get(NUMBER);
                    if (phoneNum != null) {
                        Logger.w(TAG, "addChatItem  the phonenum is " + phoneNum);
                        avatarImageView.setAsyncContact(phoneNum);
                    } else {
                        Logger.w(TAG, "addChatItem()  the phonenum is " + null);
                    }
                } else {
                    Logger.w(TAG, "addChatItem()  the chatMap is " + null);
                }
            }
            textViewInfo.setText(chat.getDisplayName());
            mData.put(view, chat);
            Logger.d(TAG, "addChatItem() the mData is " + mData);
            TextView unreadMessage = (TextView) view.findViewById(R.id.unread_message_num);
            unreadMessage.setVisibility(View.GONE);
        } else {
            Logger.e(TAG, "addChatItem(), chat is null");
        }
    }

    /**
     * Remove invitation item.
     * 
     * @param sessionId The session id that identify the invitation item.
     */
    public void removeInviteItem(String sessionId) {
        Logger.v(TAG, "removeInviteItem entry, with sessionId: " + sessionId);
        if (sessionId == null) {
            Logger.w(TAG, "sessionId is null, do not remove invite item");
            return;
        }
        Set<View> viewSet = mData.keySet();
        Object[] viewList = viewSet.toArray();
        for (int i = 0; i < viewList.length; i++) {
            View view = (View) viewList[i];
            if (view != null) {
                Object obj = mData.get(view);
                if (obj instanceof InvitationStruct) {
                    InvitationStruct invitation = (InvitationStruct) obj;
                    String itemSessionId = null;
                    try {
                        itemSessionId = invitation.getChatSession().getSessionID();
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                    Logger.v(TAG, "itemSessionId = " + itemSessionId);
                    if (sessionId.equals(itemSessionId)) {
                        mData.remove(view);
                        mInviteContainer.removeView(view);
                        if (mInviteContainer.getChildCount() == 0) {
                            mInviteHeader.setVisibility(View.GONE);
                        } else {
                            Logger.d(TAG,
                                    "removeInviteItem(), mInviteContainer have invite child view!");
                        }
                        return;
                    }
                } else {
                    Logger.d(TAG, "Current obj data is not a InvitationStruct instance");
                }
            } else {
                Logger.d(TAG, "Current view retrieved is null");
            }
        }
    }

    /**
     * Remove chat item by IChatWindow
     * 
     * @param chatWindow Indicate which item to remove
     */
    public void removeChatItem(IChatWindow chatWindow) {
        Set<View> viewSet = mData.keySet();
        Object[] viewList = viewSet.toArray();
        for (int i = 0; i < viewList.length; i++) {
            View view = (View) viewList[i];
            if (view != null) {
                Object obj = mData.get(view);
                if (obj instanceof ChatsStruct) {
                    ChatsStruct chat = (ChatsStruct) obj;
                    if (chat.getChatMap() == chatWindow) {
                        mData.remove(view);
                        if (mData.size() == 0) {
                            TextView noChatView =
                                    (TextView) mChatContainer.findViewById(R.id.no_chat);
                            if (noChatView != null) {
                                noChatView.setVisibility(View.VISIBLE);
                            } else {
                                Logger.d(TAG,
                                        "removeChatItem(), mChatContainer have chat child view!");
                            }
                        }
                        mChatContainer.removeView(view);
                        return;
                    }
                } else {
                    Logger.d(TAG, "Current obj data is not a ChatsStruct instance");
                }
            } else {
                Logger.d(TAG, "Current view retrieved is null");
            }
        }
    }

    /**
     * Retrieve item
     * 
     * @param type The item type
     * @param obj The item instance
     */
    public int getPosition(Object object) {
        Collection<Object> objCollection = mData.values();
        Object[] objList = (Object[]) objCollection.toArray();
        if (objList == null) {
            return INVALID_POSITION;
        }
        for (int i = 0; i < objList.length; i++) {
            if (objList[i] == object) {
                return i;
            }
        }
        return INVALID_POSITION;
    }

    /**
     * Retrieve the data
     * 
     * @param view The key of map
     * @return Data instance
     */
    public Object getData(View view) {
        if (!mData.containsKey(view)) {
            return null;
        }
        return mData.get(view);
    }

    /**
     * The invitation structure
     */
    public static class InvitationStruct {

        // The time stamp
        private Date mDate = null;
        // The information to show
        private String mInformation = null;
        // The invitation intent
        private Intent mInviteIntent = null;
        // The contact icon
        private Drawable mDrawable = null;
        // The IChatSession instance
        private IChatSession mChatSession = null;

        /**
         * Structure method
         * 
         * @param drawable The contact icon
         * @param date The time stamp
         * @param information The information to show
         * @param intent The invitation intent
         * @param IChatSession The IChatSession instance
         */
        public InvitationStruct(Drawable drawable, Date date, String information, Intent intent,
                IChatSession chatSession) {
            this.mDrawable = drawable;
            if (date != null) {
                this.mDate = (Date) date.clone();
            } else {
                Logger.d(TAG, "The date is null");
            }
            this.mInformation = information;
            this.mInviteIntent = intent;
            this.mChatSession = chatSession;
        }

        /**
         * Get the the contact icon
         * 
         * @return The contact icon
         */
        public Drawable getDrawable() {
            return mDrawable;
        }

        /**
         * Set the contact icon
         * 
         * @param drawable The contact icon
         */
        public void setDrawable(Drawable drawable) {
            this.mDrawable = drawable;
        }

        /**
         * The the time stamp
         * 
         * @return The time stamp
         */
        public Date getDate() {
            if (mDate != null) {
                return (Date) mDate.clone();
            } else {
                Logger.d(TAG, "The mDate is null");
                return null;
            }
        }

        /**
         * Set the time stamp
         * 
         * @param date The time stamp to show
         */
        public void setDate(Date date) {
            if (date != null) {
                this.mDate = (Date) date.clone();
            } else {
                Logger.d(TAG, "The date is null");
            }
        }

        /**
         * Get the information to show
         * 
         * @return The information to show
         */
        public String getInformation() {
            return mInformation;
        }

        /**
         * Set the information to show
         * 
         * @param information The information to show
         */
        public void setInformation(String information) {
            this.mInformation = information;
        }

        /**
         * Get the IChatSession
         * 
         * @return IChatSession
         */
        public IChatSession getChatSession() {
            return mChatSession;
        }

        /**
         * Set the IChatSession
         * 
         * @param chatSession The IChatSession instance to set
         */
        public void setChatSession(IChatSession chatSession) {
            this.mChatSession = chatSession;
        }

        /**
         * Set the invite Intent instance
         * 
         * @param intent The invite intent to set
         */
        public void setIntent(Intent intent) {
            this.mInviteIntent = intent;
        }

        /**
         * Get the invite intent
         * 
         * @return Return the invite intent
         */
        public Intent getIntent() {
            return mInviteIntent;
        }
    }

    public static class MessageItemStructForContact
    {
        private String mlatestMessageId ="";
        public String getMlatestMessageId() {
            return mlatestMessageId;
        }
        public void setMlatestMessageId(String mlatestMessageId) {
            this.mlatestMessageId = mlatestMessageId;
        }
        private String mlatestMessage ="";
        public String getMlatestMessage() {
            return mlatestMessage;
        }
        public void setMlatestMessage(String mlatestMessage) {
            this.mlatestMessage = mlatestMessage;
        }
        private Date mDate = null;
        public Date getmDate() {
            return mDate;
        }
        public void setmDate(Date mDate) {
            this.mDate = mDate;
        }
        
    }

    /**
     * The chats structure
     */
    public static class ChatsStruct {
        // Flag to indicate the type, true for group chat, false for one to one
        // chat
        private boolean mIsGroupChat;
        // The ChatMap instance
        private ChatMap mChatMap = null;
        // Unread message number
        private int mUnreadMessageNum;
        // The time stamp
        private Date mDate = null;
        // The information to show
        private String mLastMessage = null;
        
        public ArrayList<MessageItemStructForContact> chatItems = new ArrayList<MessageItemStructForContact>();
        
       
        private String mlatestMessageId ="";
        // Is file transferring
public String getMlatestMessageId() {
            return mlatestMessageId;
        }

        public void setMlatestMessageId(String mlatestMessageId) {
            this.mlatestMessageId = mlatestMessageId;
        }    
        private boolean mIsFileTransfer = false;

        private String mSubject = "";

        public String getmSubject() {
            return mSubject;
        }

        public void setmSubject(String mSubject) {
            this.mSubject = mSubject;
        }

        /**
         * Structure method
         * 
         * @param drawable The contact icon
         * @param displayName The diaplay name
         */
        public ChatsStruct(boolean isGroupChat, ChatMap chatMap) {
            mIsGroupChat = isGroupChat;
            mChatMap = chatMap;
            mUnreadMessageNum = 0;
        }

        /**
         * Set the file transfer state
         * 
         * @param isFileTransfer True for file transfer, false for none file
         *            transfer
         */
        public void setFileTransfer(boolean isFileTransfer) {
            mIsFileTransfer = isFileTransfer;
        }

        /**
         * Get the state if the file transfer is true
         * 
         * @return The file transfer state
         */
        public boolean getIsFileTransfer() {
            return mIsFileTransfer;
        }

        /**
         * Get the the display name
         * 
         * @return The display name
         */
        public String getDisplayName() {
            Logger.d(TAG, "getDisplayName() entry, mIsGroupChat is " + mIsGroupChat);
            if (mIsGroupChat) {
                return getGroupDisplayName();
            } else {
                return getOne2OneDisplayName();
            }
        }

        private String getOne2OneDisplayName() {
            Logger.d(TAG, "getOne2OneDisplayName() entry");
            if (null != mChatMap) {
                Object obj = mChatMap.get(ChatListFragment.NUMBER);
                Logger.d(TAG, "getOne2OneDisplayName() the obj is " + obj);
                if (obj instanceof String) {
                    String displayName =
 ContactsListManager.getInstance().getDisplayNameByPhoneNumber((String) obj);
                    Logger.d(TAG, "getOne2OneDisplayName() the display name is " + displayName);
                    return displayName;
                } else {
                    Logger.e(TAG, "getOne2OneDisplayName() obj is not a String");
                    return null;
                }
            } else {
                Logger.e(TAG, "getOne2OneDisplayName() mChatMap is null");
                return null;
            }
        }

        private String getGroupDisplayName() {
            Logger.d(TAG, "getGroupDisplayName() entry");
            if (null != mChatMap) {
                Object obj = mChatMap.get(ChatListFragment.NUMBER);
                Logger.d(TAG, "getGroupDisplayName() the obj is " + obj);
                if (obj instanceof List) {
                    List<String> numbers = (List<String>) obj;
                    String displayName =
                            ChatFragment.getParticipantsName(numbers.toArray(new String[1]));
                    Logger.d(TAG, "getGroupDisplayName() the display name is " + displayName);
                    return displayName;
                } else {
                    Logger.e(TAG, "getGroupDisplayName() obj is not a List");
                    return null;
                }
            } else {
                Logger.e(TAG, "getGroupDisplayName() mChatMap is null");
                return null;
            }
        }

        /**
         * Get the the participant number
         * 
         * @return The participant number
         */
        public int getParticipantNum() {
            if (mIsGroupChat) {
                Logger.d(TAG, "getParticipantNum() this is a group chat");
                if (null != mChatMap) {
                    Object obj = mChatMap.get(ChatListFragment.NUMBER);
                    Logger.d(TAG, "getParticipantNum() the obj is " + obj);
                    if (obj instanceof List) {
                        List<String> numbers = (List<String>) obj;
                        int size = numbers.size();
                        Logger.d(TAG, "getParticipantNum() the size is " + size);
                        return size;
                    } else {
                        Logger.e(TAG, "getParticipantNum() obj is not a List");
                        return 0;
                    }
                } else {
                    Logger.e(TAG, "getParticipantNum() this is a group chat, but mChatMap is null");
                    return 0;
                }
            } else {
                Logger.d(TAG, "getParticipantNum() this is a 1-2-1 chat");
                return 1;
            }
        }
        
        /**
         * Get the the participant number list
         * 
         * @return The participant number list
         */
        public TreeSet<String> getContacts() {
            if (mIsGroupChat) {
                Logger.d(TAG, "getContacts() this is a group chat");
                if (null != mChatMap) {
                    Object obj = mChatMap.get(ChatListFragment.NUMBER);
                    Logger.d(TAG, "getContacts() the obj is " + obj);
                    if (obj instanceof List) {
                        TreeSet<String> result = new TreeSet<String>();
                        List<String> numbers = (List<String>) obj;
                        for (String number : numbers) {
                            result.add(number);
                        }
                        Logger.d(TAG, "getContacts() the result is " + result);
                        return result;
                    } else {
                        Logger.e(TAG, "getContacts() obj is not a List");
                        return null;
                    }
                } else {
                    Logger.e(TAG, "getContacts() this is a group chat, but mChatMap is null");
                    return null;
                }
            } else {
                Logger.d(TAG, "getContacts() this is a 1-2-1 chat");
                return null;
            }
        }

        /**
         * Get the chat type
         * 
         * @return true if this is group chat, false if this is a 1-2-1 chat
         */
        public boolean isGroupChat() {
            return mIsGroupChat;
        }

        /**
         * Get the ChatMap
         * 
         * @return ChatMap
         */
        public ChatMap getChatMap() {
            return mChatMap;
        }

        /**
         * Get the chat window tag
         * 
         * @return The chat window tag
         */
        public Object getWindowTag() {
            if (null != mChatMap) {
                Object tag = mChatMap.getTag();
                return tag;
            } else {
                Logger.w(TAG, "getWindowTag() mChatMap is null");
                return null;
            }
        }

        /**
         * Add the unread message number
         * 
         * @param number The number of unread message to add
         */
        public void addUnreadMessageNumber() {
            this.mUnreadMessageNum++;
        }

        /**
         * Clear the unread message number
         */
        public void clearUnreadMessageNumber() {
            this.mUnreadMessageNum = 0;
        }

        /**
         * Get the unread message number
         * 
         * @return The unread message number
         */
        public int getUnreadMessageNumber() {
            return mUnreadMessageNum;
        }

        /**
         * The the time stamp
         * 
         * @return The time stamp
         */
        public Date getDate() {
            if (mDate != null) {
                return (Date) mDate.clone();
            } else {
                Logger.d(TAG, "The mDate is null");
                return null;
            }
        }

        /**
         * Set the time stamp
         * 
         * @param date The time stamp to show
         */
        public void setDate(Date date) {
            if (date != null) {
                this.mDate = (Date) date.clone();
            } else {
                this.mDate = null;
            }
        }

        /**
         * Get the last message to display
         * 
         * @return The last message to display
         */
        public String getLatestMessage() {
            return mLastMessage;
        }

        /**
         * Set the last message to display
         * 
         * @param message The last message to display
         */
        public void setLatestMessage(String message) {
            this.mLastMessage = message;
        }
    }

    /**
     * update chat item associated with the specific WindowTag in chat list as
     * read status.
     * 
     * @param chatWindowTag the window tag need to be update
     */
    public void updateAllMsgAsRead(Object chatWindowTag) {
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG, "updateAllMsgAsRead() not find the chatWindowTag, findPair is null");
            return;
        }
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        Logger.d(TAG, "updateAllMsgAsRead() findPair is " + findPair);
        chatStruct.clearUnreadMessageNumber();
        updateChats(view, chatStruct);
    }

    private Pair<View, ChatsStruct> getChatPairWithTag(Object chatWindowTag) {
        Logger.d(TAG, "getChatPairWithTag() entry the chatwindowTag is " + chatWindowTag);
        if (null == chatWindowTag) {
            Logger.d(TAG, "getChatPairWithTag() The chatWindowTag is null");
            return null;
        }
        if (mData.isEmpty()) {
            Logger.d(TAG, "getChatPairWithTag() The mData is empty");
            return null;
        }
        Set<View> viewSet = mData.keySet();
        Iterator<View> viewIterator = viewSet.iterator();
        if (viewIterator == null) {
            Logger.d(TAG, "getChatPairWithTag() The viewIterator is null");
            return null;
        }
        while (viewIterator.hasNext()) {
            View view = viewIterator.next();
            if (view == null) {
                Logger.d(TAG, "getChatPairWithTag() The view is null");
            } else {
                Object obj = mData.get(view);
                if (obj == null) {
                    Logger.d(TAG, "getChatPairWithTag() The value is null");
                } else {
                    if (obj instanceof ChatsStruct) {
                        ChatsStruct chatStruct = (ChatsStruct) obj;
                        Object windowTag = chatStruct.getWindowTag();
                        if (null != windowTag && chatWindowTag == windowTag) {
                            Logger.d(TAG, "getChatPairWithTag() find the specific window Tag");
                            Pair<View, ChatsStruct> tempPair =
                                    new Pair<View, ChatsStruct>(view, chatStruct);
                            return tempPair;
                        }
                    }
                }
            }
        }
        return null;
    }

    /**
     * add a message in a particular Window .
     * 
     * @param chatWindowTag the specific window Tag whose message need to be
     *            added.
     * @param message the message need to be added
     * @param isRead the message is read or not
     */
    public void addMessage(Object chatWindowTag, InstantMessage message, boolean isRead) {

        Logger.d(TAG, "addMessage() entry chatWindowTag is " + chatWindowTag + " message is "
                + message);
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG, "addMessage() not find the chatWindowTag, findPair is null");
            return;
        }
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        Logger.d(TAG, "addMessage() findPair is " + findPair);
        if (null != message) {
            
            chatStruct.setMlatestMessageId(message.getMessageId());        
                       
            chatStruct.setDate(message.getDate());
            chatStruct.setLatestMessage(message.getTextMessage());
            
            MessageItemStructForContact newMessage = new MessageItemStructForContact();
            newMessage.setmDate(message.getDate());
            newMessage.setMlatestMessage(message.getTextMessage());
            newMessage.setMlatestMessageId(chatStruct.getMlatestMessageId());
            
            if (!isRead) {
                chatStruct.addUnreadMessageNumber();
            } else {
                Logger.d(TAG, "addMessage() the message read already");
            }
            for(int i=0 ; i< chatStruct.chatItems.size(); i++)
            {
                Logger.d(TAG, "addMessage() chatStruct is " + chatStruct +"msgID : " + message.getMessageId());
                
                if(!chatStruct.chatItems.isEmpty() && message.getMessageId()!=null && (message.getMessageId().equals(chatStruct.chatItems.get(i).getMlatestMessageId())))
                        chatStruct.chatItems.remove(i);   
            }
            chatStruct.chatItems.add(newMessage);
        } else {
            chatStruct.setDate(null);
            chatStruct.setLatestMessage(null);
        }
        updateChats(view, chatStruct);
    }
    /**
     * Remove the Message of a particular ID.
     * 
     * @param chatWindowTag the specific window Tag whose history need to be
     *            clear.
     */
    public void removeMessageForId(Object chatWindowTag, String MessageId) {
        Logger.d(TAG, "removeMessageForWinTag() entry chatWindowTag is " + chatWindowTag);
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG, "removeMessageForWinTag() not find the chatWindowTag findPair is null");
            return;
        }        
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        String latestMessageId = chatStruct.getMlatestMessageId();
        if(latestMessageId.equals(MessageId))
        {          
            chatStruct.chatItems.remove(chatStruct.chatItems.size()-1);
            Logger.d(TAG, "removeMessageForWinTag() findPair is " + findPair);
            if(chatStruct.chatItems.size() == 0)
            {
                chatStruct.setDate(null);
                chatStruct.setLatestMessage(null);
                chatStruct.clearUnreadMessageNumber();            
                chatStruct.setMlatestMessageId("");
            }
            else
            {
                chatStruct.setDate(chatStruct.chatItems.get(chatStruct.chatItems.size()-1).getmDate());
                chatStruct.setLatestMessage(chatStruct.chatItems.get(chatStruct.chatItems.size()-1).getMlatestMessage());
                chatStruct.clearUnreadMessageNumber();            
                chatStruct.setMlatestMessageId(chatStruct.chatItems.get(chatStruct.chatItems.size()-1).getMlatestMessageId());  
            }
           
            updateChats(view, chatStruct);
        }
        
        for(int i =0 ; i < chatStruct.chatItems.size() ; i++)
        {
            if(MessageId.equals(chatStruct.chatItems.get(i).getMlatestMessageId()))
                chatStruct.chatItems.remove(i);
        }
    }

    /**
     * add a message in a particular Window .
     * 
     * @param chatWindowTag the specific window Tag whose message need to be
     *            added.
     * @param message the message need to be added
     * @param isRead the message is read or not
     */
    public void addGroupSubject(Object chatWindowTag, String subject) {
        
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG, "addMessage() not find the chatWindowTag, findPair is null");
            return;
        }
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        Logger.d(TAG, "addGroupSubject() findPair is " + findPair);
        chatStruct.setmSubject(subject);        
        Logger.d(TAG, "addGroupSubject Subject is " + subject);
        updateChats(view, chatStruct);
    }

    /**
     * Remove the chat history of a particular Window Tag.
     * 
     * @param chatWindowTag the specific window Tag whose history need to be
     *            clear.
     */
    public void removeMessageForWinTag(Object chatWindowTag) {
        Logger.d(TAG, "removeMessageForWinTag() entry chatWindowTag is " + chatWindowTag);
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG, "removeMessageForWinTag() not find the chatWindowTag findPair is null");
            return;
        }
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        chatStruct.chatItems.clear();
        Logger.d(TAG, "removeMessageForWinTag() findPair is " + findPair);
        chatStruct.setDate(null);
        chatStruct.setLatestMessage(null);
        chatStruct.clearUnreadMessageNumber();
        updateChats(view, chatStruct);
    }

    public void onFileTransferStateChanged(Object chatWindowTag, boolean isActive) {
        Logger.d(TAG, "onFileTransferStateChanged() entry the window tag is " + chatWindowTag
                + " isActive " + isActive);
        Pair<View, ChatsStruct> findPair = getChatPairWithTag(chatWindowTag);
        if (null == findPair) {
            Logger.e(TAG,
                    "onFileTransferStateChanged() not find the chatWindowTag findPair is null");
            return;
        }
        View view = findPair.first;
        ChatsStruct chatStruct = findPair.second;
        Logger.d(TAG, "onFileTransferStateChanged() findPair is " + findPair);
        chatStruct.setFileTransfer(isActive);
        updateChats(view, chatStruct);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        ContactsListManager.getInstance().addListener(this);
    }

    @Override
    protected void onDetachedFromWindow() {
        ContactsListManager.getInstance().removeListener(this);
        super.onDetachedFromWindow();
    }

    @Override
    public void onDisplayNameChanged() {
        Logger.d(TAG, "onDisplayNameChanged()");
        updateView();
    }
}
