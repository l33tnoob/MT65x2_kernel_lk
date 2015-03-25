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

import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.Date;

/**
 * This is an item binder for received message
 */
public class ReceivedMessageItemBinder extends AbsItemBinder  {
    public static final String TAG = "ReceivedMessageItemBinder";

    private IReceivedChatMessage mMessage = null;

    public ReceivedMessageItemBinder(IReceivedChatMessage message) {
        mMessage = message;
    }

    @Override
    public void bindView(View itemView) {
        bindReceivedMessage(mMessage, itemView);
    }

    @Override
    public int getLayoutResId() {
        return R.layout.chat_item_received_message;
    }

    @Override
    public int getItemType() {
        return ChatAdapter.ITEM_TYPE_RECEIVED_MESSAGE;
    }

    private void bindReceivedMessage(IReceivedChatMessage message, View view) {
        AsyncAvatarView avatarImageView = (AsyncAvatarView) view.findViewById(R.id.peer_avatar);
        TextView remoteNameView = (TextView) view.findViewById(R.id.remote_name);
        if (avatarImageView == null || avatarImageView == null) {
            // To show debug info when avatarImageView or avatarImageView is
            // null
            Logger.w(TAG,
                    "why avatarImageView or avatarImageView is null. R.id.peer_avatar = "
                            + R.id.peer_avatar + ",R.id.remote_name = "
                            + R.id.remote_name + ",avatarImageView = "
                            + avatarImageView + ",remoteNameView = "
                            + remoteNameView);
            view.debug();
        }
        String phoneNum = null;
        String phoneNumUri = null;
        String remoteName = null;
        if (message != null) {
            if (message instanceof One2OneChatFragment.ReceivedMessage) {
                bindReceivedOne2OneMessage((One2OneChatFragment.ReceivedMessage) message, view);
                phoneNumUri = ((One2OneChatFragment.ReceivedMessage) message).getMessageSender();
            } else {
                bindReceivedGroupMessage((GroupChatFragment.ReceivedMessage) message, view);
                phoneNumUri = ((GroupChatFragment.ReceivedMessage) message).getMessageSender();
            }
            if (phoneNumUri != null) {
                phoneNum = PhoneUtils.extractNumberFromUri(phoneNumUri);
                Logger.e(TAG, "bindReceivedMessage the phoneNumUri is " + phoneNum);
                remoteName = phoneNum;
                if(avatarImageView != null){
                    avatarImageView.setAsyncContact(phoneNum);
                }
                if (PhoneUtils.isANumber(phoneNum)) {
                    remoteName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(phoneNum);
                } else {
                    Logger.e(TAG, "bindReceivedMessage() the remoteContact " + phoneNum
                            + " is not a real number");
                }
                if(remoteNameView != null){
                    remoteNameView.setText(remoteName);
                }
            } else {
                Logger.e(TAG, "bindReceivedMessage the phoneNumUri is null");
            }
            if (avatarImageView != null) {
                avatarImageView.setVisibility(View.VISIBLE);
            }
        } else {
            Logger.e(TAG, "bindReceivedMessage the message is null");
        }
    }

    private void bindReceivedOne2OneMessage(final One2OneChatFragment.ReceivedMessage message,
            View view) {
        TextView textView = (TextView) view.findViewById(R.id.chat_text_display);
        //textView.setOnClickListener(this);

        String text = message.getMessageText();
        textView.setText(EmoticonsModelImpl.getInstance().formatMessage(text));

        DateView dateView = (DateView) view.findViewById(R.id.chat_time);
        setReceivedMessageDateOnView(dateView, message.getMessageDate());
    }

    private void bindReceivedGroupMessage(final GroupChatFragment.ReceivedMessage message, View view) {
        TextView textView = (TextView) view.findViewById(R.id.chat_text_display);
       // textView.setOnClickListener(this);

        String text = message.getMessageText();
        textView.setText(EmoticonsModelImpl.getInstance().formatMessage(text));

        DateView dateView = (DateView) view.findViewById(R.id.chat_time);
        setReceivedMessageDateOnView(dateView, message.getMessageDate());
    }

    private void setReceivedMessageDateOnView(DateView v, Date date) {
        if (v != null) {
            v.setVisibility(View.VISIBLE);
            v.setTime(date);
        } else {
            Logger.w(TAG, "The view is null.");
        }
    }

    /*@Override
    public void onClick(View v) {
        MessageParsing.onChatMessageClick(v);
    }*/
}
