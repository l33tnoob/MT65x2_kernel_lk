/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.stk;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.Item;

import java.util.List;

/**
 * Icon list view adapter to show the list of STK items.
 */
public class StkMenuAdapter extends ArrayAdapter<Item> {
    private final LayoutInflater mInflater;
    private boolean mIcosSelfExplanatory = false;
    private byte[] m_NextActionIndicator;
    private final boolean SHOW_NEXTACTION = true;

    public StkMenuAdapter(Context context, List<Item> items, byte[] nextActionIndicator,
            boolean icosSelfExplanatory) {
        super(context, 0, items);
        mInflater = LayoutInflater.from(context);
        mIcosSelfExplanatory = icosSelfExplanatory;
        m_NextActionIndicator = nextActionIndicator;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Item item = getItem(position);
        int strId = 0;
        boolean flag = false;

        if (!mIcosSelfExplanatory || (mIcosSelfExplanatory && item.icon == null)) {
            if (m_NextActionIndicator != null) {
                AppInterface.CommandType type = AppInterface.CommandType
                        .fromInt(m_NextActionIndicator[position]);
                if (type != null) {
                    flag = true;
                    switch (type) {
                        case SET_UP_MENU:
                            strId = R.string.lable_setup_menu;
                            break;
                        case DISPLAY_TEXT:
                            strId = R.string.lable_display_text;
                            break;
                        case REFRESH:
                            strId = R.string.lable_refresh;
                            break;
                        case SET_UP_IDLE_MODE_TEXT:
                            strId = R.string.lable_setup_idle_modetext;
                            break;
                        case LAUNCH_BROWSER:
                            strId = R.string.lable_launch_browser;
                            break;
                        case SELECT_ITEM:
                            strId = R.string.lable_select_item;
                            break;
                        case GET_INPUT:
                            strId = R.string.lable_get_input;
                            break;
                        case GET_INKEY:
                            strId = R.string.lable_get_inkey;
                            break;
                        case SEND_DTMF:
                            strId = R.string.lable_send_dtmf;
                            break;
                        case SET_UP_EVENT_LIST:
                            strId = R.string.lable_setup_event_list;
                            break;
                        case SEND_SMS:
                            strId = R.string.lable_send_sms;
                            break;
                        case SEND_SS:
                            strId = R.string.lable_send_ss;
                            break;
                        case SEND_USSD:
                            strId = R.string.lable_send_ussd;
                            break;
                        case PLAY_TONE:
                            strId = R.string.lable_play_tone;
                            break;
                        case SET_UP_CALL:
                            strId = R.string.lable_setup_call;
                            break;
                        case MORE_TIME:
                            strId = R.string.lable_more_time;
                            break;
                        case POLL_INTERVAL:
                            strId = R.string.lable_poll_interval;
                            break;
                        case POLLING_OFF:
                            strId = R.string.lable_polling_off;
                            break;
                        case PROVIDE_LOCAL_INFORMATION:
                            strId = R.string.lable_provide_local_information;
                            break;
                        case TIMER_MANAGEMENT:
                            strId = R.string.lable_timer_management;
                            break;
                        case PERFORM_CARD_APDU:
                            strId = R.string.lable_perform_card_apdu;
                            break;
                        case POWER_ON_CARD:
                            strId = R.string.lable_power_on_card;
                            break;
                        case POWER_OFF_CARD:
                            strId = R.string.lable_power_off_card;
                            break;
                        case GET_READER_STATUS:
                            strId = R.string.lable_get_reader_status;
                            break;
                        case RUN_AT_COMMAND:
                            strId = R.string.lable_run_at_command;
                            break;
                        case LANGUAGE_NOTIFICATION:
                            strId = R.string.lable_language_notification;
                            break;
                        case OPEN_CHANNEL:
                            strId = R.string.lable_open_channel;
                            break;
                        case CLOSE_CHANNEL:
                            strId = R.string.lable_close_channel;
                            break;
                        case RECEIVE_DATA:
                            strId = R.string.lable_receive_data;
                            break;
                        case SEND_DATA:
                            strId = R.string.lable_send_data;
                            break;
                        case GET_CHANNEL_STATUS:
                            strId = R.string.lable_get_channel_status;
                            break;
                        case SERVICE_SEARCH:
                            strId = R.string.lable_service_search;
                            break;
                        case GET_SERVICE_INFORMATION:
                            strId = R.string.lable_get_service_information;
                            break;
                        case DECLARE_SERVICE:
                            strId = R.string.lable_declare_service;
                            break;
                        case SET_FRAME:
                            strId = R.string.lable_set_frame;
                            break;
                        case GET_FRAME_STATUS:
                            strId = R.string.lable_get_frame_status;
                            break;
                        case RETRIEVE_MULTIMEDIA_MESSAGE:
                            strId = R.string.lable_retrieve_multimedia_message;
                            break;
                        case SUBMIT_MULTIMEDIA_MESSAGE:
                            strId = R.string.lable_submit_multimedia_message;
                            break;
                        case DISPLAY_MULTIMEDIA_MESSAGE:
                            strId = R.string.lable_display_multimedia_message;
                            break;
                        case ACTIVATE:
                            strId = R.string.lable_activate;
                            break;
                        default:
                            flag = false;
                            break;
                    }
                }
            }
            if (flag && SHOW_NEXTACTION) {
                String str = this.getContext().getString(strId);
                convertView = mInflater.inflate(R.layout.stk_menu_item2, parent,
                        false);
                ((TextView) convertView.findViewById(R.id.text)).setText(item.text);
                ((TextView) convertView.findViewById(R.id.summary)).setText(str);
            } else {
                convertView = mInflater.inflate(R.layout.stk_menu_item, parent,
                        false);
                ((TextView) convertView.findViewById(R.id.text)).setText(item.text);
            }
            ImageView imageView = ((ImageView) convertView.findViewById(R.id.icon));
            if (item.icon == null) {
                imageView.setVisibility(View.GONE);
            } else {
                imageView.setImageBitmap(item.icon);
                imageView.setVisibility(View.VISIBLE);
            }
        } else {
            // Only show Icon and hide text.
            convertView = mInflater.inflate(R.layout.stk_menu_item, parent,
                    false);
            ((TextView) convertView.findViewById(R.id.text)).setVisibility(View.GONE);
            ImageView imageView = ((ImageView) convertView.findViewById(R.id.icon));
            imageView.setImageBitmap(item.icon);
            imageView.setVisibility(View.VISIBLE);
        }
        return convertView;
    }
}
