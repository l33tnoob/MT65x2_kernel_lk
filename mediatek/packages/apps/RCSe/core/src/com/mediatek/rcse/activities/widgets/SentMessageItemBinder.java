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

import android.text.InputFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Button;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Message;

import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.mvc.ControllerImpl;



import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * This is an item binder for sent message
 */
public class SentMessageItemBinder extends AbsItemBinder  {
    public static final String TAG = "SentMessageItemBinder";

    private ISentChatMessage mMessage = null;

    public SentMessageItemBinder(ISentChatMessage message) {
        mMessage = message;
    }

    @Override
    public void bindView(View itemView) {
        bindSentMessage(mMessage, itemView);
    }

    @Override
    public int getLayoutResId() {
        return R.layout.chat_item_sent_message;
    }

    @Override
    public int getItemType() {
        return ChatAdapter.ITEM_TYPE_SENT_MESSAGE;
    }

    private void bindSentMessage(ISentChatMessage message, View view) {
        Logger.d(TAG, "bindSentMessage() entry");
        ImageView statusIcon = (ImageView) view.findViewById(R.id.chat_status_display);
        if (message instanceof One2OneChatFragment.SentMessage) {
            if (statusIcon != null) {
                statusIcon.setVisibility(View.VISIBLE);
            } else {
                Logger.d(TAG, "bindSentMessage() it is integration mode");
            }
            bindSentOne2OneMessage((One2OneChatFragment.SentMessage) message, view);
        } else {
            if (statusIcon != null) {
                statusIcon.setVisibility(View.GONE);
            } else {
                Logger.d(TAG, "bindSentMessage() it is integration mode");
            }
            bindSentGroupMessage((GroupChatFragment.SentMessage) message, view);
        }
    }

    private void bindSentOne2OneMessage(final One2OneChatFragment.SentMessage message, View view) {
        ImageView imageView = (ImageView) view.findViewById(R.id.chat_status_display);
        Button resentView = (Button)view.findViewById(R.id.chat_message_resent);
        TextView textView = (TextView) view.findViewById(R.id.chat_text_display);
        /*textView.setOnClickListener(this);
        if (resentView != null) {
            resentView.setOnClickListener(this);
        } else {
            Logger.d(TAG, "resendView is null!");
        }*/


        String text = message.getMessageText();
        textView.setText(EmoticonsModelImpl.getInstance().formatMessage(text));

        DateView dateView = (DateView) view.findViewById(R.id.chat_time);
        View isDisplayedIcon = view.findViewById(R.id.is_displayed);
        switch (message.getStatus()) {
            case SENDING:
                isDisplayedIcon.setVisibility(View.GONE);
                imageView.setVisibility(View.GONE);
                dateView.setVisibility(View.GONE);
                resentView.setVisibility(View.GONE);
                break;
            case DELIVERED:
                isDisplayedIcon.setVisibility(View.GONE);
                imageView.setVisibility(View.GONE);
                resentView.setVisibility(View.GONE);
                setSentMessageDateOnView(dateView, message.getMessageDate());
                break;
            case DISPLAYED:
                isDisplayedIcon.setVisibility(View.VISIBLE);
                imageView.setVisibility(View.GONE);
                resentView.setVisibility(View.GONE);
                setSentMessageDateOnView(dateView, message.getMessageDate());
                break;
            case FAILED:
                isDisplayedIcon.setVisibility(View.GONE);
                imageView.setVisibility(View.VISIBLE);
                imageView.setImageResource(R.drawable.icon_message_status_error);
                dateView.setVisibility(View.GONE);
                resentView.setVisibility(View.GONE);
                break;
            default:
                imageView.setImageResource(R.drawable.rcs_core_notif_off_icon);
                break;
        }
    }

    private void bindSentGroupMessage(final GroupChatFragment.SentMessage message, View view) {
        Logger.d(TAG, "bindSentGroupMessage() entry");
        ImageView imageView = (ImageView) view.findViewById(R.id.chat_status_display);
        TextView textView = (TextView) view.findViewById(R.id.chat_text_display);
        String text = message.getMessageText();
        textView.setText(EmoticonsModelImpl.getInstance().formatMessage(text));
        //textView.setOnClickListener(this);

        /*Button resentView = (Button)view.findViewById(R.id.chat_message_resent);
        if (resentView != null) {
            resentView.setOnClickListener(this);
        } else {
            Logger.d(TAG, "resendView is null!");
        }*/
        
        /*Button statusView = (Button)view.findViewById(R.id.message_status);
        if (statusView != null) {
        	statusView.setVisibility(View.VISIBLE);
        	statusView.setOnClickListener(this);
        } else {
            Logger.d(TAG, "statusView is null!");
        }*/
        
        DateView dateView = (DateView) view.findViewById(R.id.chat_time);
        switch (message.getStatus()) {
            case SENDING:
                Logger.d(TAG, "bindSentGroupMessage() status is SENDING");
                setSentMessageDateOnView(dateView, message.getMessageDate());
                dateView.setVisibility(View.VISIBLE);
                break;
            case DELIVERED:
                setSentMessageDateOnView(dateView, message.getMessageDate());
                break;
            case DISPLAYED:
                if (imageView != null) {
                    imageView.setVisibility(View.GONE);
                } else {
                    Logger.d(TAG, "bindSentGroupMessage() it is integration mode ");
                }
                setSentMessageDateOnView(dateView, message.getMessageDate());
                break;
            case FAILED:
                setSentMessageDateOnView(dateView, message.getMessageDate());
                if (imageView != null) {
                    imageView.setImageResource(R.drawable.icon_message_status_error);
                } else {
                    Logger.d(TAG, "bindSentGroupMessage() it is integration mode ");
                }
                break;
            default:
                if (imageView != null) {
                    imageView.setImageResource(R.drawable.rcs_core_notif_off_icon);
                } else {
                    Logger.d(TAG, "bindSentGroupMessage() it is integration mode ");
                }
                break;
        }
    }

    private void setSentMessageDateOnView(DateView v, Date date) {
        if (v != null) {
            v.setVisibility(View.VISIBLE);
            v.setTime(date);
        } else {
            Logger.w(TAG, "The view is null.");
        }
    }

    private void handleMessageResent() {        
        int messageTag = ((SentMessage)mMessage).getMessageTag();
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
        ChatController.EVENT_SEND_MESSAGE, ((SentMessage)mMessage).getChatTag(), ((SentMessage)mMessage).getMessageText());
        controllerMessage.arg1 = messageTag;
        controllerMessage.sendToTarget();        
    }
    
    /*@Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
        case R.id.chat_text_display:
            Logger.v(TAG, "Text Message parsing.");
        MessageParsing.onChatMessageClick(v);
            break;
        case R.id.chat_message_resent:
            Logger.v(TAG, "Sender resend the message.");
            handleMessageResent();
            break;
        case R.id.message_status:
        	Logger.v(TAG, "show status dialog group");
            showStatusDialog(v.getContext());
            break;
        default:
            break;
    }
}*/
    
}
