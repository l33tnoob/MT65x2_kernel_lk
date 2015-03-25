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

import java.util.HashMap;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.string;
import com.mediatek.mms.ipmessage.IpMessageConsts.array;
import com.mediatek.mms.ipmessage.ResourceManager;
import com.mediatek.mms.ipmessage.IpMessageConsts.drawable;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.CoreApplication;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;

/**
 * This class manages the rcse resource.
 */
public class IpMessageResourceMananger extends ResourceManager {
    protected static final String TAG = "IpMessageResourceMananger";
    private static HashMap<Integer, Integer> sStringMap= new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> sDrawableMap= new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> sStringArrayMap= new HashMap<Integer, Integer>();
    Resources resource = null;
    static {
        // string map
        sStringMap.put(string.ipmsg_ip_message,                         R.string.imsp_ip_message);
        sStringMap.put(string.ipmsg_cancel,                             R.string.STR_NMS_CANCEL);
        sStringMap.put(string.ipmsg_no_such_file,                       R.string.STR_NMS_NO_SUCH_FILE);
        sStringMap.put(string.ipmsg_over_file_limit,                    R.string.STR_NMS_OVER_FILE_LIMIT);
        sStringMap.put(string.ipmsg_no_internet,                        R.string.imsp_nointernet);
        sStringMap.put(string.ipmsg_load_all_message,                   R.string.STR_NMS_LOAD_ALL_MESSAGE);
        sStringMap.put(string.ipmsg_take_photo,                         R.string.STR_NMS_TAKE_PHOTO);
        sStringMap.put(string.ipmsg_record_video,                       R.string.STR_NMS_RECORD_VIDEO);
        sStringMap.put(string.ipmsg_draw_sketch,                        R.string.STR_NMS_DRAW_SKETCH);
        sStringMap.put(string.ipmsg_share_contact,                      R.string.STR_NMS_SHARE_CONTACT);
        sStringMap.put(string.ipmsg_choose_photo,                       R.string.STR_NMS_CHOOSE_PHOTO);
        sStringMap.put(string.ipmsg_choose_video,                       R.string.STR_NMS_CHOOSE_VIDEO);
        sStringMap.put(string.ipmsg_record_audio,                       R.string.STR_NMS_RECORD_AUDIO);
        sStringMap.put(string.ipmsg_choose_audio,                       R.string.STR_NMS_CHOOSE_AUDIO);
        sStringMap.put(string.ipmsg_share_location,                     R.string.STR_NMS_SHARE_LOCATION);
        sStringMap.put(string.ipmsg_share_calendar,                     R.string.STR_NMS_SHARE_CALENDAR);
        sStringMap.put(string.ipmsg_delete_important,                   R.string.STR_NMS_DELETE_IMPORTANT);
        sStringMap.put(string.ipmsg_chat_setting_updating,              R.string.imsp_chat_setting_updating);
        sStringMap.put(string.ipmsg_dialog_save_description,            R.string.STR_NMS_SAVE_CONTENT);
        sStringMap.put(string.ipmsg_dialog_email_description,           R.string.STR_NMS_EMAIL_CONTENT);
        sStringMap.put(string.ipmsg_dialog_clear_title,                 R.string.imsp_dialog_clear_title);
        sStringMap.put(string.ipmsg_dialog_clear_description,           R.string.imsp_dialog_clear_description);
        sStringMap.put(string.ipmsg_chat_setting_saving,                R.string.imsp_chat_setting_saving);
        sStringMap.put(string.ipmsg_chat_setting_sending,               R.string.imsp_chat_setting_sending);
        sStringMap.put(string.ipmsg_dialog_save_title,                  R.string.STR_NMS_SVAE_TITLE);
        sStringMap.put(string.ipmsg_dialog_email_title,                 R.string.STR_NMS_EMAIL_TITLE);
        sStringMap.put(string.ipmsg_save_chat_history_failed,           R.string.imsp_save_chat_history_failed);
        sStringMap.put(string.ipmsg_send_chat_history_failed,           R.string.imsp_send_chat_history_failed);
        sStringMap.put(string.ipmsg_conversation_list_all,              R.string.imsp_conversation_list_all);
        sStringMap.put(string.ipmsg_conversation_list_important,        R.string.imsp_conversation_list_important);
        sStringMap.put(string.ipmsg_conversation_list_group_chats,      R.string.imsp_conversation_list_group_chats);
        sStringMap.put(string.ipmsg_conversation_list_spam,             R.string.imsp_conversation_list_spam);
        sStringMap.put(string.ipmsg_conversation_list_joyn,             R.string.imsp_conversation_list_joyn);
        sStringMap.put(string.ipmsg_conversation_list_xms,              R.string.imsp_conversation_list_xms);
        sStringMap.put(string.ipmsg_mark_as_spam_tips,                  R.string.imsp_mark_as_spam_tips);
        sStringMap.put(string.ipmsg_typing,                             R.string.imsp_typing);
        sStringMap.put(string.ipmsg_introduction,                       R.string.STR_NMS_INTRODUCTION);
        sStringMap.put(string.ipmsg_service_title,                      R.string.STR_NMS_SERVICE_TITLE);
        sStringMap.put(string.ipmsg_type_to_compose_text,               R.string.imsp_type_to_compose_text);
        sStringMap.put(string.ipmsg_ipmsg,                              R.string.STR_NMS_MSG_DUMPER_MSG_IP);
        sStringMap.put(string.ipmsg_no_sdcard,                          R.string.imsp_no_sdcard);
        sStringMap.put(string.ipmsg_invite_friends_to_ipmsg_dialog_msg, R.string.imsp_invite_friends_to_ipmsg_dialog_msg);
        sStringMap.put(string.ipmsg_active,                             R.string.STR_NMS_ACTIVE);
        sStringMap.put(string.ipmsg_welcome_active,                     R.string.STR_NMS_PRELOAD_IP_MSG_CONTENT);  
        sStringMap.put(string.ipmsg_term_warn_welcome,                  R.string.STR_NMS_TERM_WARN_WELCOME);   
        sStringMap.put(string.ipmsg_term_warn_activate,                 R.string.STR_NMS_TERM_WARN_ACTIVATE);   
        sStringMap.put(string.ipmsg_term_key,                           R.string.STR_NMS_MENU_LICENSE_AGREEMENT);
        sStringMap.put(string.ipmsg_agree_and_continue,                 R.string.STR_NMS_AGREE_AND_CONTINUE);   
        sStringMap.put(string.ipmsg_activate_title,                     R.string.STR_NMS_EMPTY_ACTIVATE_TITLE);  
        sStringMap.put(string.ipmsg_current_sim_enabled,                R.string.imsp_current_sim_enabled);   
        sStringMap.put(string.ipmsg_sim_selected_dialog_title_for_activate, R.string.imsp_sim_selected_dialog_title_for_activate);   
        sStringMap.put(string.ipmsg_activate_message,                   R.string.imsp_activate_message);   
        sStringMap.put(string.ipmsg_enable_title,                       R.string.STR_NMS_ISMS_ENABLE_TITLE);
        sStringMap.put(string.ipmsg_enable_message,                     R.string.imsp_enable_message);  
        sStringMap.put(string.ipmsg_sim_selected_dialog_title_for_enable, R.string.imsp_sim_selected_dialog_title_for_enable);   
        sStringMap.put(string.ipmsg_switch_sim_title,                   R.string.imsp_switch_sim_title);   
        sStringMap.put(string.ipmsg_switch_sim_message,                 R.string.imsp_switch_sim_message);   
        sStringMap.put(string.ipmsg_switch_sim_button,                  R.string.STR_NMS_SWITCH);   
        sStringMap.put(string.ipmsg_switch_sim_successfully,            R.string.imsp_switch_sim_successfully);   
        sStringMap.put(string.ipmsg_dailog_multi_forward,               R.string.imsp_dailog_multi_forward);   
        sStringMap.put(string.ipmsg_dialog_send_result,                 R.string.imsp_dialog_send_result);   
        sStringMap.put(string.ipmsg_dlg_send_all,                       R.string.imsp_dlg_send_all);
        sStringMap.put(string.ipmsg_dlg_send_sucess,                    R.string.imsp_dlg_send_sucess);   
        sStringMap.put(string.ipmsg_dlg_send_failed,                    R.string.imsp_dlg_send_failed);   
        sStringMap.put(string.ipmsg_multi_forward_sim_info,             R.string.imsp_multi_forward_sim_info);   
        sStringMap.put(string.ipmsg_multi_forward_no_sim,               R.string.imsp_multi_forward_no_sim);   
        sStringMap.put(string.ipmsg_getsim_failed,                      R.string.imsp_get_sim_failed); 
        sStringMap.put(string.ipmsg_sim_status_error,                   R.string.imsp_sim_status_error);   
        sStringMap.put(string.ipmsg_forward_failed,                     R.string.STR_NMS_FORWARD_FAILED);  
        sStringMap.put(string.ipmsg_forward_norecipients,               R.string.imsp_forward_norecipients);   
        sStringMap.put(string.ipmsg_multiforward_no_message,            R.string.imsp_multiforward_no_message);   
        sStringMap.put(string.ipmsg_need_input_recipients,              R.string.imsp_need_input_recipients);   
        sStringMap.put(string.ipmsg_spam_empty,                         R.string.imsp_spam_empty);
        sStringMap.put(string.ipmsg_groupchat_empty,                    R.string.imsp_groupchat_empty);   
        sStringMap.put(string.ipmsg_important_empty,                    R.string.imsp_important_empty);   
        sStringMap.put(string.ipmsg_allchat_empty,                      R.string.imsp_allchat_empty); 
        sStringMap.put(string.ipmsg_vcard_file_name,                    R.string.STR_NMS_VCARD_NAME);
        sStringMap.put(string.ipmsg_hint,                               R.string.STR_NMS_INPUT_HINT_ISMS);
        sStringMap.put(string.ipmsg_caption_hint,                       R.string.STR_NMS_CAPTION_HINT);
        sStringMap.put(string.ipmsg_invite_chat_frequently,             R.string.imsp_invite_chat_frequently);
        sStringMap.put(string.ipmsg_enable_chat_frequently,             R.string.imsp_enable_chat_frequently);
        sStringMap.put(string.ipmsg_invite,                             R.string.imsp_invite);
        sStringMap.put(string.ipmsg_dismiss,                            R.string.STR_NMS_LATER);
        sStringMap.put(string.ipmsg_enable,                             R.string.STR_NMS_ENABLE);
        sStringMap.put(string.ipmsg_enable_notice,                      R.string.imsp_enable_notice);
        sStringMap.put(string.ipmsg_activate,                           R.string.STR_NMS_ACTIVE_BNT);
        sStringMap.put(string.ipmsg_activate_chat_frequently,           R.string.imsp_activate_chat_frequently);
        sStringMap.put(string.ipmsg_activate_note,                      R.string.imsp_activate_note);
        sStringMap.put(string.ipmsg_dismiss_content,                    R.string.imsp_dismiss_content);
        sStringMap.put(string.ipmsg_divider_online,                     R.string.imsp_divider_online);
        sStringMap.put(string.ipmsg_divider_offline,                    R.string.imsp_divider_offline);
        sStringMap.put(string.ipmsg_divider_never_online,               R.string.imsp_divider_never_online);
        sStringMap.put(string.ipmsg_not_delivered_title,                R.string.STR_NMS_NOT_DELIVERED_TITLE);
        sStringMap.put(string.ipmsg_failed_title,                       R.string.imsp_failed_title);
        sStringMap.put(string.ipmsg_try_again,                          R.string.STR_NMS_TRY_AGAIN);
        sStringMap.put(string.ipmsg_try_all_again,                      R.string.STR_NMS_TRY_ALL_AGAIN);
        sStringMap.put(string.ipmsg_resend_via_sms,                     R.string.imsp_resend_via_sms);
        sStringMap.put(string.ipmsg_resend_via_mms,                     R.string.imsp_resend_via_mms);
        sStringMap.put(string.ipmsg_cant_share,                         R.string.STR_NMS_CANT_SHARE);
        sStringMap.put(string.ipmsg_file_limit,                         R.string.STR_NMS_FILE_LIMIT);
        sStringMap.put(string.ipmsg_no_app,                             R.string.STR_NMS_NO_APP);
        sStringMap.put(string.ipmsg_typing_text,                        R.string.imsp_typing_text);
        sStringMap.put(string.ipmsg_share_title,                        R.string.STR_NMS_SHARE_TITLE);
        sStringMap.put(string.ipmsg_logo,                               R.string.STR_NMS_HISSAGE);
        sStringMap.put(string.ipmsg_cant_save,                          R.string.STR_NMS_CANT_SAVE);
        sStringMap.put(string.ipmsg_invalid_file_type,                  R.string.STR_NMS_INVALID_FILE_TYPE);
        sStringMap.put(string.ipmsg_invite_friends_to_chat,             R.string.STR_NMS_MENU_INVITE_CHAT);
        sStringMap.put(string.ipmsg_view_all_media,                     R.string.STR_NMS_VIEW_ALL_MEDIA);
        sStringMap.put(string.ipmsg_view_all_location,                  R.string.STR_NMS_VIEW_ALL_LOCATIONS);
        sStringMap.put(string.ipmsg_export_to_sdcard,                   R.string.STR_NMS_SAVE_IN_SDCARD);
        sStringMap.put(string.ipmsg_send_via_text_msg,                  R.string.STR_NMS_SEND_VIA_TEXT);
        sStringMap.put(string.ipmsg_send_via_mms,                       R.string.STR_NMS_SEND_VIA_MULTIMEDIA);
        sStringMap.put(string.ipmsg_retry,                              R.string.STR_NMS_TRY_AGAIN);
        sStringMap.put(string.ipmsg_delete,                             R.string.STR_NMS_DELETE);
        sStringMap.put(string.ipmsg_share,                              R.string.STR_NMS_SHARE);
        sStringMap.put(string.ipmsg_mark_as_important,                  R.string.STR_NMS_FLAG_IMPORTANT);
        sStringMap.put(string.ipmsg_remove_from_important,              R.string.STR_NMS_REMOVE_IMPORTANT);
        sStringMap.put(string.ipmsg_save_file,                          R.string.STR_NMS_SAVE_FILE);
        sStringMap.put(string.ipmsg_convert_to_mms,                     R.string.imsp_convert_to_mms);
        sStringMap.put(string.ipmsg_convert_to_sms,                     R.string.imsp_convert_to_sms);
        sStringMap.put(string.ipmsg_convert_to_ipmsg,                   R.string.imsp_convert_to_isms);
        sStringMap.put(string.ipmsg_convert_to_mms_for_service,         R.string.imsp_convert_to_mms_for_service);
        sStringMap.put(string.ipmsg_convert_to_sms_for_service,         R.string.imsp_convert_to_sms_for_service);
        sStringMap.put(string.ipmsg_convert_to_mms_for_recipients,      R.string.imsp_convert_to_mms_for_recipients);
        sStringMap.put(string.ipmsg_convert_to_sms_for_recipients,      R.string.imsp_convert_to_sms_for_recipients);
        sStringMap.put(string.ipmsg_sms_convert_to_ipmsg,               R.string.imsp_sms_convert_to_isms);
        sStringMap.put(string.ipmsg_mms_convert_to_ipmsg,               R.string.imsp_mms_convert_to_isms);
        sStringMap.put(string.ipmsg_keep_mms,                           R.string.imsp_keep_mms);
        sStringMap.put(string.ipmsg_keep_sms,                           R.string.imsp_keep_sms);
        sStringMap.put(string.ipmsg_switch,                             R.string.STR_NMS_SWITCH);
        sStringMap.put(string.ipmsg_replace_attach,                     R.string.STR_NMS_REPLACE_ATTACH_TITLE);
        sStringMap.put(string.ipmsg_replace_attach_msg,                 R.string.STR_NMS_REPLACE_ATTACH_MSG);
        sStringMap.put(string.ipmsg_err_file,                           R.string.STR_NMS_ERR_FILE);
        sStringMap.put(string.ipmsg_resend_discard_message,             R.string.imsp_resend_discard_message);
        sStringMap.put(string.ipmsg_invite_friends_content,             R.string.STR_NMS_INVITE_SMS_CONTENT);
        sStringMap.put(string.ipmsg_continue,                           R.string.STR_NMS_FORWARD_CONTIUE);
        sStringMap.put(string.ipmsg_no_sim_card,                        R.string.imsp_no_sim_card);
        sStringMap.put(string.ipmsg_download_history_dlg,               R.string.imsp_download_history_dlg);
        sStringMap.put(string.ipmsg_invite_friends_to_ipmsg,            R.string.imsp_invite_friends_to_ipmsg);
        sStringMap.put(string.ipmsg_multi_forward_tips_content,         R.string.imsp_multi_forward_tips_content);
        sStringMap.put(string.ipmsg_multi_forward_failed_part,          R.string.imsp_multi_forward_failed_part);
        sStringMap.put(string.ipmsg_sdcard_space_not_enough,            R.string.STR_NMS_SDSPACE_NOT_ENOUGH);
        sStringMap.put(string.ipmsg_mms_cost_remind,          			R.string.MMS_COST_REMIND);
        sStringMap.put(string.ipmsg_joyn_cost_remind,            		R.string.JOYN_COST_REMIND);
        sStringMap.put(string.ipmsg_joyn_stranger_remind,               R.string.stranger_remind);
        
        sStringMap.put(string.ipmsg_enter_joyn_chat,                    R.string.enter_joyn_chat);
        sStringMap.put(string.ipmsg_enter_xms_chat,                     R.string.enter_xms_chat);
        sStringMap.put(string.ipmsg_send_by_joyn,                       R.string.send_by_joyn);
        sStringMap.put(string.ipmsg_send_by_xms,                        R.string.send_by_xms);
        sStringMap.put(string.ipmsg_export_chat,                        R.string.export_chat);
        
     // string array
        sStringArrayMap.put(array.ipmsg_share_string_array, R.array.imsp_share_string_array);
        
        sDrawableMap.put(drawable.ipmsg_emoticon,               R.drawable.ipmsg_emoticon);
        sDrawableMap.put(drawable.emo_small_01,                 R.drawable.emo_small_01);
        sDrawableMap.put(drawable.emo_small_02,                 R.drawable.emo_small_02);
        sDrawableMap.put(drawable.emo_small_03,                 R.drawable.emo_small_03);
        sDrawableMap.put(drawable.emo_small_04,                 R.drawable.emo_small_04);
        sDrawableMap.put(drawable.emo_small_05,                 R.drawable.emo_small_05);
        sDrawableMap.put(drawable.emo_small_06,                 R.drawable.emo_small_06);
        sDrawableMap.put(drawable.emo_small_07,                 R.drawable.emo_small_07);
        sDrawableMap.put(drawable.emo_small_08,                 R.drawable.emo_small_08);
        sDrawableMap.put(drawable.emo_small_09,                 R.drawable.emo_small_09);
        sDrawableMap.put(drawable.emo_small_10,                 R.drawable.emo_small_10);
        sDrawableMap.put(drawable.emo_small_11,                 R.drawable.emo_small_11);
        sDrawableMap.put(drawable.emo_small_12,                 R.drawable.emo_small_12);
        sDrawableMap.put(drawable.emo_small_13,                 R.drawable.emo_small_13);
        sDrawableMap.put(drawable.emo_small_14,                 R.drawable.emo_small_14);
        sDrawableMap.put(drawable.emo_small_15,                 R.drawable.emo_small_15);
        sDrawableMap.put(drawable.emo_small_16,                 R.drawable.emo_small_16);
        sDrawableMap.put(drawable.emo_small_17,                 R.drawable.emo_small_17);
        sDrawableMap.put(drawable.emo_small_18,                 R.drawable.emo_small_18);
        sDrawableMap.put(drawable.emo_small_19,                 R.drawable.emo_small_19);
        sDrawableMap.put(drawable.emo_small_20,                 R.drawable.emo_small_20);
        sDrawableMap.put(drawable.emo_small_21,                 R.drawable.emo_small_21);
        sDrawableMap.put(drawable.emo_small_22,                 R.drawable.emo_small_22);
        sDrawableMap.put(drawable.emo_small_23,                 R.drawable.emo_small_23);
        sDrawableMap.put(drawable.emo_small_24,                 R.drawable.emo_small_24);
        sDrawableMap.put(drawable.emo_small_25,                 R.drawable.emo_small_25);
        sDrawableMap.put(drawable.emo_small_26,                 R.drawable.emo_small_26);
        sDrawableMap.put(drawable.emo_small_27,                 R.drawable.emo_small_27);
        sDrawableMap.put(drawable.emo_small_28,                 R.drawable.emo_small_28);
        sDrawableMap.put(drawable.emo_small_29,                 R.drawable.emo_small_29);
        sDrawableMap.put(drawable.emo_small_30,                 R.drawable.emo_small_30);
        sDrawableMap.put(drawable.emo_small_31,                 R.drawable.emo_small_31);
        sDrawableMap.put(drawable.emo_small_32,                 R.drawable.emo_small_32);
        sDrawableMap.put(drawable.emo_small_33,                 R.drawable.emo_small_33);
        sDrawableMap.put(drawable.emo_small_34,                 R.drawable.emo_small_34);
        sDrawableMap.put(drawable.emo_small_35,                 R.drawable.emo_small_35);
        sDrawableMap.put(drawable.emo_small_36,                 R.drawable.emo_small_36);
        sDrawableMap.put(drawable.emo_small_37,                 R.drawable.emo_small_37);
        sDrawableMap.put(drawable.emo_small_38,                 R.drawable.emo_small_38);
        sDrawableMap.put(drawable.emo_small_39,                 R.drawable.emo_small_39);
        sDrawableMap.put(drawable.emo_small_40,                 R.drawable.emo_small_40);
        sDrawableMap.put(drawable.emo_good,                 	R.drawable.good);
        sDrawableMap.put(drawable.emo_ok,                	    R.drawable.ok);
        sDrawableMap.put(drawable.emo_film,                     R.drawable.film);
        sDrawableMap.put(drawable.emo_fuyun,                 	R.drawable.fuyun);        
        sDrawableMap.put(drawable.emo_heart,                  	R.drawable.heart);
        sDrawableMap.put(drawable.emo_lightning,                R.drawable.lightning);
        sDrawableMap.put(drawable.emo_microphone,               R.drawable.microphone);
        sDrawableMap.put(drawable.emo_music,                    R.drawable.music);
        sDrawableMap.put(drawable.emo_no,                 		R.drawable.no);
        sDrawableMap.put(drawable.emo_victory,                  R.drawable.victory);
        sDrawableMap.put(drawable.emo_penguin,                  R.drawable.penguin);
        sDrawableMap.put(drawable.emo_pig,                 		R.drawable.pig);
        sDrawableMap.put(drawable.emo_rain,                 	R.drawable.rain);
        sDrawableMap.put(drawable.emo_rice,                 	R.drawable.rice);
        sDrawableMap.put(drawable.emo_roses,                 	R.drawable.roses);
        sDrawableMap.put(drawable.emo_seduce,                 	R.drawable.seduce);
        sDrawableMap.put(drawable.emo_shenma,                 	R.drawable.shenma);
        sDrawableMap.put(drawable.emo_star,                 	R.drawable.star);
        sDrawableMap.put(drawable.emo_sun,                 		R.drawable.sun);
        sDrawableMap.put(drawable.emo_umbrella,                 R.drawable.umbrella);
        
        
        sDrawableMap.put(drawable.ipmsg_full_integrated,        R.drawable.full_integration);
        sDrawableMap.put(drawable.ipmsg_file_transfer_pause,    R.drawable.ipmsg_file_pause);
        sDrawableMap.put(drawable.ipmsg_file_transfer_resume,   R.drawable.ipmsg_file_resume);
        sDrawableMap.put(drawable.ipmsg_file_transfer_cancel,   R.drawable.ipmsg_file_cancel);
        sDrawableMap.put(drawable.enter_joyn_chat,              R.drawable.ic_send_isms);
        
    }

    public IpMessageResourceMananger(Context context) {
        super(context);
        Logger.d(TAG, "getSingleString() ,context is " + context);
        try {
 			resource = AndroidFactory.getApplicationContext()
 					.getPackageManager()
 					.getResourcesForApplication(CoreApplication.APP_NAME);
 		} catch (android.content.pm.PackageManager.NameNotFoundException e) {
 			e.printStackTrace();
         }
    }

    /**
     * Get drawable from remote.
     * @param id with special resource id.
     */
     public Drawable getSingleDrawable(int id) {
       	Logger.d(TAG, "getSingleDrawable() ,id is " + id);
        if (resource != null) {
        	if(sDrawableMap.get(id)==null)
        			return null;
        	Logger.d(TAG, "getSingleDrawable() ,resource not null");               
           return resource.getDrawable(sDrawableMap.get(id).intValue());              
        }
        return null;
     }

     /**
      * Get String array from remote.
      * @param id with special resource id.
      */
     @Override
      public String[] getStringArray(int id) {    	 
    	Logger.d(TAG, "getStringArray() ,id is " + id);
    	String[] stringArray = null;
 		if(resource!= null)
 			stringArray =  resource.getStringArray(sStringArrayMap.get(id).intValue());
 		return stringArray;
      }

    @Override
    public String getSingleString(int id) {
        Logger.d(TAG, "getSingleString() ,id is " + id);
		String string = null;
		if(resource!= null)
			string =  resource.getString(sStringMap.get(id).intValue());
		return string;
    }

    /**
     * Get sim name from plugin.
     * 
     * @return True Joyn.
     */
    @Override
    public String getSimStatus() {
        return resource.getString(R.string.sim_info);
    }
}
