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

import android.content.Context;
import android.view.View;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.GroupChatFragment.ChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.service.ApiManager;

import com.orangelabs.rcs.R;

/**
 * This is an item binder for chat event
 */
public class ChatEventItemBinder extends AbsItemBinder {
    public static final String TAG = "ChatEventItemBinder";
    // The seprator text
    private static final String SEPRATOR = ", ";

    private IChatEventInformation mChatEventInformation = null;

    public ChatEventItemBinder(IChatEventInformation information) {
        mChatEventInformation = information;
    }

    @Override
    public void bindView(View itemView) {
        TextView chatEventInformation =
                (TextView) itemView.findViewById(R.id.file_transfer_terminated);
        Information information = ((ChatEventInformation) mChatEventInformation).getInformation();
        Logger.d(TAG, "bindView the information is " + information);
        switch (information) {
            case LEFT:
                Object leftContact =
                        ((ChatEventInformation) mChatEventInformation).getRelatedInfo();
                bindChatEventLeft(chatEventInformation, leftContact);
                break;
            case JOIN:
                Object contact = ((ChatEventInformation) mChatEventInformation).getRelatedInfo();
                bindChatEventJoin(chatEventInformation, contact);
                break;
            default:
                Logger.e(TAG, "bindView the information is not defined " + information);
                break;
        }
    }

    @Override
    public int getLayoutResId() {
        return R.layout.chat_item_file_transfer_terminated;
    }

    @Override
    public int getItemType() {
        return ChatAdapter.ITEM_TYPE_CHAT_EVENT_INFORMATION;
    }

    private void bindChatEventLeft(TextView textView, Object contact) {
        Logger.d(TAG, "bindChatEventLeft entry " + contact);
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            String eventInformation = null;
            if (contact instanceof String) {
                String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                        (String) contact);
                eventInformation = context.getString(R.string.leave_group_chat, displayName);
                Logger.d(TAG, "bindChatEventQui the eventInformation is " + eventInformation);
            } else {
                eventInformation =
                        context.getString(R.string.leave_group_chat, context
                                .getString(R.string.group_chat_stranger));
                Logger.e(TAG, "bindChatEventLeft the contactName is not a String");
            }
            textView.setText(eventInformation);
        } else {
            Logger.e(TAG, "bindChatEventQui the context is null");
        }
    }

    private void bindChatEventJoin(TextView textView, Object contact) {
        Logger.d(TAG, "bindChatEventJoin entry " + contact);
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            String eventInformation = null;
            if (contact instanceof String) {
                String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                        (String) contact);
                eventInformation = context.getString(R.string.join_group_chat, displayName);
                Logger.d(TAG, "bindChatEventQui the eventInformation is " + eventInformation);
            } else {
                eventInformation =
                        context.getString(R.string.join_group_chat, context
                                .getString(R.string.group_chat_stranger));
                Logger.e(TAG, "bindChatEventLeft the contactName is not a String");
            }
            textView.setText(eventInformation);
        } else {
            Logger.e(TAG, "bindChatEventQui the context is null");
        }
    }
}
