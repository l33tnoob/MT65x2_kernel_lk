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

package com.mediatek.imsp;

import android.content.Context;
import android.graphics.drawable.Drawable;
import com.hissage.R;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.array;
import com.mediatek.mms.ipmessage.IpMessageConsts.string;
import com.mediatek.mms.ipmessage.IpMessageConsts.drawable;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.mms.ipmessage.ResourceManager;
import java.io.InputStream;
import java.util.HashMap;


public class ResourceManagerExt extends ResourceManager {

    private static HashMap<Integer, Integer> sStringMap= new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> sStringArrayMap= new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> sDrawableMap= new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> sRawResourceMap= new HashMap<Integer, Integer>();
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
        sStringMap.put(string.ipmsg_nms_enable_success,                 R.string.STR_NMS_ENABLE_SUCCESS);
        sStringMap.put(string.ipmsg_nms_not_enough_storage,             R.string.STR_NMS_NOT_ENOUGH_STORAGE);
        sStringMap.put(string.ipmsg_nms_forward_download_title1,        R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE1);
        sStringMap.put(string.ipmsg_nms_forward_download_info1,         R.string.STR_NMS_FORWARD_DOWNLOAD_INFO1);
        sStringMap.put(string.ipmsg_nms_forward_download_result,        R.string.STR_NMS_FORWARD_DOWNLOAD_RESULT);
        sStringMap.put(string.ipmsg_nms_forward_download_info2,         R.string.STR_NMS_FORWARD_DOWNLOAD_INFO2);
        sStringMap.put(string.ipmsg_nms_forward_download_failed,         R.string.STR_NMS_FORWARD_DOWNLOAD_FAILED);

        sStringMap.put(string.ipmsg_nms_store_status_full,              R.string.STR_NMS_STORE_STATUS_FULL);

        sStringMap.put(string.ipmsg_no_ismsuser_message,                R.string.imsp_no_ismsuser_message);
        sStringMap.put(string.ipmsp_delivered_when_get_online,          R.string.imsp_delivered_when_get_online);
        sStringMap.put(string.ipmsg_title_selectphoto,                  R.string.STR_NMS_SELECT_ACTION_TITLE);
        sStringMap.put(string.ipmsg_photograph,                         R.string.STR_NMS_TAKE_PHOTO);
        sStringMap.put(string.ipmsg_selectphoto,                        R.string.STR_NMS_CHOOSE_PHOTO);

        sStringMap.put(string.ipmsg_readed_burn_pic,                    R.string.STR_NMS_READED_BURN_PIC);
        sStringMap.put(string.ipmsg_readed_burn_check,                  R.string.STR_NMS_READED_BURN_CHECK);
        sStringMap.put(string.ipmsg_readed_burn_pic_destroy,            R.string.STR_NMS_READED_BURN_PIC_DESTROY);
        sStringMap.put(string.ipmsg_in_process_of_download,             R.string.STR_NMS_DOWNLOAD_HISTORY_DLG);
        sStringMap.put(string.ipmsg_destroy_hint,                       R.string.STR_NMS_DESTROY_HINT);
        sStringMap.put(string.ipmsg_readed_burn_not_received,           R.string.STR_NMS_READED_BURN_NOT_RECEIVED);
        sStringMap.put(string.ipmsg_readed_burn_download,               R.string.STR_NMS_READED_BURN_DOWNLOAD);
        sStringMap.put(string.ipmsg_second,                             R.string.STR_NMS_SECOND);
        sStringMap.put(string.ipmsg_readed_burn,                        R.string.STR_NMS_READED_BURN);
        sStringMap.put(string.ipmsg_switch_sim,                         R.string.STR_NMS_SWITCH_SIM);

        sStringMap.put(string.ipmsg_str_nms_menu_license_agreement,       R.string.STR_NMS_MENU_LICENSE_AGREEMENT);
        sStringMap.put(string.ipmsg_str_nms_activation_button,            R.string.STR_NMS_ACTIVATE_BUTTON);
        sStringMap.put(string.ipmsg_str_nms_activation_button_ok,         R.string.STR_NMS_ACTIVATE_BUTTON_OK);
        sStringMap.put(string.ipmsg_str_nms_service_center_body,          R.string.STR_NMS_SERVICE_CENTER_BODY);
        sStringMap.put(string.ipmsg_str_nms_service_center_ok,          R.string.STR_NMS_SERVICE_CENTER_OK); 
        sStringMap.put(string.ipmsg_conversation_list_private_msg,      R.string.imsp_conversation_list_private_msg);
        sStringMap.put(string.ipmsg_conversation_list_private_contact,  R.string.imsp_conversation_list_private_contact);
        sStringMap.put(string.ipmsg_private_msg_setting_pwd,            R.string.imsp_private_msg_setting_pwd);
        sStringMap.put(string.ipmsg_private_msg_setting_pwd_tips,       R.string.imsp_private_msg_setting_pwd_tips);
        sStringMap.put(string.ipmsg_private_msg_confirm_pwd,            R.string.imsp_private_msg_confirm_pwd);
        sStringMap.put(string.ipmsg_private_msg_enter_pwd,              R.string.imsp_private_msg_enter_pwd);
        sStringMap.put(string.ipmsg_move_to,                            R.string.imsp_move_to);
        sStringMap.put(string.ipmsg_move_from,                          R.string.imsp_move_from);
        sStringMap.put(string.ipmsg_private_msg_list,                   R.string.imsp_private_msg_list);
        sStringMap.put(string.ipmsg_private_msg_pwd_cannot_null,        R.string.imsp_private_msg_pwd_cannot_null);
        sStringMap.put(string.ipmsg_private_msg_pwd_six_digit,          R.string.imsp_private_msg_pwd_six_digit);
        sStringMap.put(string.ipmsg_private_msg_pwd_error,              R.string.imsp_private_msg_pwd_error);
        sStringMap.put(string.ipmsg_sure,                               R.string.imsp_sure);
        sStringMap.put(string.ipmsg_private_from_contact,               R.string.imsp_private_from_contact);
        sStringMap.put(string.ipmsg_private_from_diy,                   R.string.imsp_private_from_diy);
        sStringMap.put(string.ipmsg_private_no_message,                 R.string.imsp_private_no_message);
        sStringMap.put(string.ipmsg_private_no_contact,                 R.string.imsp_private_no_contact);
        sStringMap.put(string.ipmsg_private_add_contact,                R.string.imsp_private_add_contact);
        sStringMap.put(string.ipmsg_private_enter_phonenumber,          R.string.imsp_private_enter_phonenumber);
        sStringMap.put(string.ipmsg_private_setting,                    R.string.imsp_private_setting);
        sStringMap.put(string.ipmsg_private_setting_modify_pwd,         R.string.imsp_private_setting_modify_pwd);
        sStringMap.put(string.ipmsg_private_setting_enterance,          R.string.imsp_private_setting_enterance);
        sStringMap.put(string.ipmsg_private_modify_enterance,           R.string.imsp_private_modify_enterance);
        sStringMap.put(string.ipmsg_private_setting_tips,               R.string.imsp_private_setting_tips);
        sStringMap.put(string.ipmsg_private_notification,               R.string.imsp_private_notification);
        sStringMap.put(string.ipmsg_private_notification_title,         R.string.imsp_private_notification_title);
        sStringMap.put(string.ipmsg_private_notification_content,       R.string.imsp_private_notification_content);
        sStringMap.put(string.ipmsg_private_notification_title_summary, R.string.imsp_private_notification_title_summary);
        sStringMap.put(string.ipmsg_private_notification_content_summary,R.string.imsp_private_notification_content_summary);
        sStringMap.put(string.ipmsg_private_notification_setting_content,R.string.imsp_private_notification_setting_content);
        sStringMap.put(string.ipmsg_private_notification_setting_title, R.string.imsp_private_notification_setting_title);
        sStringMap.put(string.ipmsg_private_new_message,                R.string.imsp_private_new_message);
        sStringMap.put(string.ipmsg_private_contact_del,                R.string.imsp_private_contact_del);
        sStringMap.put(string.ipmsg_private_msg_add_contact_tips,       R.string.imsp_private_msg_add_contact_tips);
        sStringMap.put(string.ipmsg_private_enter_six_digit,            R.string.imsp_private_enter_six_digit);
        sStringMap.put(string.ipmsg_private_contact_invalid_number,     R.string.imsp_private_contact_invalid_number);
        sStringMap.put(string.ipmsg_private_long_pressed,               R.string.STR_NMS_LONG_PRESS_TITLE);

        sStringMap.put(string.ipmsg_ip_msg_not_sendable_to_sms,               R.string.imsp_ip_msg_not_sendable_to_sms);
        sStringMap.put(string.ipmsg_ip_msg_not_sendable_to_mms,               R.string.imsp_ip_msg_not_sendable_to_mms);

        // string array
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
            sStringArrayMap.put(array.ipmsg_share_string_array, R.array.imsp_share_string_array);
        }else{
            sStringArrayMap.put(array.ipmsg_share_string_array, R.array.imsp_share_string_array_no_new_features);
        }
        // drawable map
        sDrawableMap.put(drawable.ipmsg_emoticon_default_h,     R.drawable.default_h);
        sDrawableMap.put(drawable.ipmsg_emoticon_default_n,   R.drawable.default_n);
        sDrawableMap.put(drawable.ipmsg_emoticon_xm_h,   R.drawable.xm_h);
        sDrawableMap.put(drawable.ipmsg_emoticon_xm_n,    R.drawable.xm_n);
        sDrawableMap.put(drawable.ipmsg_emoticon_ad_h,      R.drawable.ad_h);
        sDrawableMap.put(drawable.ipmsg_emoticon_ad_n,     R.drawable.ad_n);
        sDrawableMap.put(drawable.ipmsg_emoticon_rabbit_h,      R.drawable.rabbit_h);
        sDrawableMap.put(drawable.ipmsg_emoticon_rabbit_n,    R.drawable.rabbit_n);
        sDrawableMap.put(drawable.ipmsg_emoticon_dragon_h,  R.drawable.dragon_h);
        sDrawableMap.put(drawable.ipmsg_emoticon_dragon_n,  R.drawable.dragon_n);

        sDrawableMap.put(drawable.emo_small_01,   R.drawable.emo_small_01);
        sDrawableMap.put(drawable.emo_small_02,     R.drawable.emo_small_02);
        sDrawableMap.put(drawable.emo_small_03,   R.drawable.emo_small_03);
        sDrawableMap.put(drawable.emo_small_04,   R.drawable.emo_small_04);
        sDrawableMap.put(drawable.emo_small_05,    R.drawable.emo_small_05);
        sDrawableMap.put(drawable.emo_small_06,      R.drawable.emo_small_06);
        sDrawableMap.put(drawable.emo_small_07,     R.drawable.emo_small_07);
        sDrawableMap.put(drawable.emo_small_08,      R.drawable.emo_small_08);
        sDrawableMap.put(drawable.emo_small_09,    R.drawable.emo_small_09);
        sDrawableMap.put(drawable.emo_small_10,  R.drawable.emo_small_10);
        sDrawableMap.put(drawable.emo_small_11,   R.drawable.emo_small_11);
        sDrawableMap.put(drawable.emo_small_12,      R.drawable.emo_small_12);
        sDrawableMap.put(drawable.emo_small_13,  R.drawable.emo_small_13);
        sDrawableMap.put(drawable.emo_small_14,   R.drawable.emo_small_14);
        sDrawableMap.put(drawable.emo_small_15,    R.drawable.emo_small_15);
        sDrawableMap.put(drawable.emo_small_16,     R.drawable.emo_small_16);
        sDrawableMap.put(drawable.emo_small_17,  R.drawable.emo_small_17);
        sDrawableMap.put(drawable.emo_small_18,     R.drawable.emo_small_18);
        sDrawableMap.put(drawable.emo_small_19,     R.drawable.emo_small_19);
        sDrawableMap.put(drawable.emo_small_20, R.drawable.emo_small_20);
        sDrawableMap.put(drawable.emo_small_21,     R.drawable.emo_small_21);
        sDrawableMap.put(drawable.emo_small_22,    R.drawable.emo_small_22);
        sDrawableMap.put(drawable.emo_small_23,   R.drawable.emo_small_23);
        sDrawableMap.put(drawable.emo_small_24, R.drawable.emo_small_24);
        sDrawableMap.put(drawable.emo_small_25, R.drawable.emo_small_25);
        sDrawableMap.put(drawable.emo_small_26,     R.drawable.emo_small_26);
        sDrawableMap.put(drawable.emo_small_27,    R.drawable.emo_small_27);
        sDrawableMap.put(drawable.emo_small_28,   R.drawable.emo_small_28);
        sDrawableMap.put(drawable.emo_small_29, R.drawable.emo_small_29);
        sDrawableMap.put(drawable.emo_small_30, R.drawable.emo_small_30);
        sDrawableMap.put(drawable.emo_small_31,     R.drawable.emo_small_31);
        sDrawableMap.put(drawable.emo_small_32,    R.drawable.emo_small_32);
        sDrawableMap.put(drawable.emo_small_33,   R.drawable.emo_small_33);
        sDrawableMap.put(drawable.emo_small_34, R.drawable.emo_small_34);
        sDrawableMap.put(drawable.emo_small_35, R.drawable.emo_small_35);
        sDrawableMap.put(drawable.emo_small_36,     R.drawable.emo_small_36);
        sDrawableMap.put(drawable.emo_small_37,    R.drawable.emo_small_37);
        sDrawableMap.put(drawable.emo_small_38,   R.drawable.emo_small_38);
        sDrawableMap.put(drawable.emo_small_39, R.drawable.emo_small_39);
        sDrawableMap.put(drawable.emo_small_40, R.drawable.emo_small_40);

        sDrawableMap.put(drawable.emo_good, R.drawable.good);
        sDrawableMap.put(drawable.emo_no,     R.drawable.no);
        sDrawableMap.put(drawable.emo_ok,    R.drawable.ok);
        sDrawableMap.put(drawable.emo_victory,   R.drawable.victory);
        sDrawableMap.put(drawable.emo_seduce, R.drawable.seduce);
        sDrawableMap.put(drawable.emo_down, R.drawable.down);
        sDrawableMap.put(drawable.emo_rain,     R.drawable.rain);
        sDrawableMap.put(drawable.emo_lightning,    R.drawable.lightning);
        sDrawableMap.put(drawable.emo_sun,   R.drawable.sun);
        sDrawableMap.put(drawable.emo_microphone, R.drawable.microphone);
        sDrawableMap.put(drawable.emo_clock, R.drawable.clock);
        sDrawableMap.put(drawable.emo_email,     R.drawable.email);
        sDrawableMap.put(drawable.emo_candle,    R.drawable.candle);
        sDrawableMap.put(drawable.emo_birthday_cake,   R.drawable.birthday_cake);
        sDrawableMap.put(drawable.emo_small_gift, R.drawable.gift);
        sDrawableMap.put(drawable.emo_star, R.drawable.star);
        sDrawableMap.put(drawable.emo_heart,     R.drawable.heart);
        sDrawableMap.put(drawable.emo_brokenheart,    R.drawable.brokenheart);
        sDrawableMap.put(drawable.emo_bulb,   R.drawable.bulb);
        sDrawableMap.put(drawable.emo_music, R.drawable.music);
        sDrawableMap.put(drawable.emo_shenma, R.drawable.shenma);
        sDrawableMap.put(drawable.emo_fuyun,     R.drawable.fuyun);
        sDrawableMap.put(drawable.emo_rice,    R.drawable.rice);
        sDrawableMap.put(drawable.emo_roses,   R.drawable.roses);
        sDrawableMap.put(drawable.emo_film, R.drawable.film);
        sDrawableMap.put(drawable.emo_aeroplane, R.drawable.aeroplane);
        sDrawableMap.put(drawable.emo_umbrella,   R.drawable.umbrella);
        sDrawableMap.put(drawable.emo_caonima, R.drawable.caonima);
        sDrawableMap.put(drawable.emo_penguin, R.drawable.penguin);
        sDrawableMap.put(drawable.emo_pig,     R.drawable.pig);

        sDrawableMap.put(drawable.emo_praise,   R.drawable.emo_praise);       // largeIconArr
        sDrawableMap.put(drawable.emo_gift,     R.drawable.emo_gift);
        sDrawableMap.put(drawable.emo_kongfu,   R.drawable.emo_kongfu);
        sDrawableMap.put(drawable.emo_shower,   R.drawable.emo_shower);
        sDrawableMap.put(drawable.emo_scare,    R.drawable.emo_scare);
        sDrawableMap.put(drawable.emo_ill,      R.drawable.emo_ill);
        sDrawableMap.put(drawable.emo_rich,     R.drawable.emo_rich);
        sDrawableMap.put(drawable.emo_fly,      R.drawable.emo_fly);
        sDrawableMap.put(drawable.emo_angry,    R.drawable.emo_angry);
        sDrawableMap.put(drawable.emo_approve,  R.drawable.emo_approve);
        sDrawableMap.put(drawable.emo_boring,   R.drawable.emo_boring);
        sDrawableMap.put(drawable.emo_cry,      R.drawable.emo_cry);
        sDrawableMap.put(drawable.emo_driving,  R.drawable.emo_driving);
        sDrawableMap.put(drawable.emo_eating,   R.drawable.emo_eating);
        sDrawableMap.put(drawable.emo_happy,    R.drawable.emo_happy);
        sDrawableMap.put(drawable.emo_hold,     R.drawable.emo_hold);
        sDrawableMap.put(drawable.emo_holiday,  R.drawable.emo_holiday);
        sDrawableMap.put(drawable.emo_love,     R.drawable.emo_love);
        sDrawableMap.put(drawable.emo_pray,     R.drawable.emo_pray);
        sDrawableMap.put(drawable.emo_pressure, R.drawable.emo_pressure);
        sDrawableMap.put(drawable.emo_sing,     R.drawable.emo_sing);
        sDrawableMap.put(drawable.emo_sleep,    R.drawable.emo_sleep);
        sDrawableMap.put(drawable.emo_sports,   R.drawable.emo_sports);
        sDrawableMap.put(drawable.emo_swimming, R.drawable.emo_swimming);

        sDrawableMap.put(drawable.emo_dynamic_01_png, R.drawable.emo_dynamic_01_png);   // dynamicPngIconArr
        sDrawableMap.put(drawable.emo_dynamic_02_png, R.drawable.emo_dynamic_02_png);
        sDrawableMap.put(drawable.emo_dynamic_03_png, R.drawable.emo_dynamic_03_png);
        sDrawableMap.put(drawable.emo_dynamic_04_png, R.drawable.emo_dynamic_04_png);
        sDrawableMap.put(drawable.emo_dynamic_05_png, R.drawable.emo_dynamic_05_png);
        sDrawableMap.put(drawable.emo_dynamic_06_png, R.drawable.emo_dynamic_06_png);
        sDrawableMap.put(drawable.emo_dynamic_07_png, R.drawable.emo_dynamic_07_png);
        sDrawableMap.put(drawable.emo_dynamic_08_png, R.drawable.emo_dynamic_08_png);
        sDrawableMap.put(drawable.emo_dynamic_09_png, R.drawable.emo_dynamic_09_png);
        sDrawableMap.put(drawable.emo_dynamic_10_png, R.drawable.emo_dynamic_10_png);
        sDrawableMap.put(drawable.emo_dynamic_11_png, R.drawable.emo_dynamic_11_png);
        sDrawableMap.put(drawable.emo_dynamic_12_png, R.drawable.emo_dynamic_12_png);
        sDrawableMap.put(drawable.emo_dynamic_13_png, R.drawable.emo_dynamic_13_png);
        sDrawableMap.put(drawable.emo_dynamic_14_png, R.drawable.emo_dynamic_14_png);
        sDrawableMap.put(drawable.emo_dynamic_15_png, R.drawable.emo_dynamic_15_png);
        sDrawableMap.put(drawable.emo_dynamic_16_png, R.drawable.emo_dynamic_16_png);
        sDrawableMap.put(drawable.emo_dynamic_17_png, R.drawable.emo_dynamic_17_png);
        sDrawableMap.put(drawable.emo_dynamic_18_png, R.drawable.emo_dynamic_18_png);
        sDrawableMap.put(drawable.emo_dynamic_19_png, R.drawable.emo_dynamic_19_png);
        sDrawableMap.put(drawable.emo_dynamic_20_png, R.drawable.emo_dynamic_20_png);
        sDrawableMap.put(drawable.emo_dynamic_21_png, R.drawable.emo_dynamic_21_png);
        sDrawableMap.put(drawable.emo_dynamic_22_png, R.drawable.emo_dynamic_22_png);
        sDrawableMap.put(drawable.emo_dynamic_23_png, R.drawable.emo_dynamic_23_png);
        sDrawableMap.put(drawable.emo_dynamic_24_png, R.drawable.emo_dynamic_24_png);

        sDrawableMap.put(drawable.ad01_png, R.drawable.ad01_png);     // adPngIconArr
        sDrawableMap.put(drawable.ad02_png, R.drawable.ad02_png);
        sDrawableMap.put(drawable.ad03_png, R.drawable.ad03_png);
        sDrawableMap.put(drawable.ad04_png, R.drawable.ad04_png);
        sDrawableMap.put(drawable.ad05_png, R.drawable.ad05_png);
        sDrawableMap.put(drawable.ad06_png, R.drawable.ad06_png);
        sDrawableMap.put(drawable.ad07_png, R.drawable.ad07_png);
        sDrawableMap.put(drawable.ad08_png, R.drawable.ad08_png);
        sDrawableMap.put(drawable.ad09_png, R.drawable.ad09_png);
        sDrawableMap.put(drawable.ad10_png, R.drawable.ad10_png);
        sDrawableMap.put(drawable.ad11_png, R.drawable.ad11_png);
        sDrawableMap.put(drawable.ad12_png, R.drawable.ad12_png);
        sDrawableMap.put(drawable.ad13_png, R.drawable.ad13_png);
        sDrawableMap.put(drawable.ad14_png, R.drawable.ad14_png);
        sDrawableMap.put(drawable.ad15_png, R.drawable.ad15_png);
        sDrawableMap.put(drawable.ad16_png, R.drawable.ad16_png);
        sDrawableMap.put(drawable.ad17_png, R.drawable.ad17_png);
        sDrawableMap.put(drawable.ad18_png, R.drawable.ad18_png);
        sDrawableMap.put(drawable.ad19_png, R.drawable.ad19_png);
        sDrawableMap.put(drawable.ad20_png, R.drawable.ad20_png);
        sDrawableMap.put(drawable.ad21_png, R.drawable.ad21_png);
        sDrawableMap.put(drawable.ad22_png, R.drawable.ad22_png);
        sDrawableMap.put(drawable.ad23_png, R.drawable.ad23_png);
        sDrawableMap.put(drawable.ad24_png, R.drawable.ad24_png);

        sDrawableMap.put(drawable.xm01_png, R.drawable.xm01_png);     // xmPngIconArr
        sDrawableMap.put(drawable.xm02_png, R.drawable.xm02_png);
        sDrawableMap.put(drawable.xm03_png, R.drawable.xm03_png);
        sDrawableMap.put(drawable.xm04_png, R.drawable.xm04_png);
        sDrawableMap.put(drawable.xm05_png, R.drawable.xm05_png);
        sDrawableMap.put(drawable.xm06_png, R.drawable.xm06_png);
        sDrawableMap.put(drawable.xm07_png, R.drawable.xm07_png);
        sDrawableMap.put(drawable.xm08_png, R.drawable.xm08_png);
        sDrawableMap.put(drawable.xm09_png, R.drawable.xm09_png);
        sDrawableMap.put(drawable.xm10_png, R.drawable.xm10_png);
        sDrawableMap.put(drawable.xm11_png, R.drawable.xm11_png);
        sDrawableMap.put(drawable.xm12_png, R.drawable.xm12_png);
        sDrawableMap.put(drawable.xm13_png, R.drawable.xm13_png);
        sDrawableMap.put(drawable.xm14_png, R.drawable.xm14_png);
        sDrawableMap.put(drawable.xm15_png, R.drawable.xm15_png);
        sDrawableMap.put(drawable.xm16_png, R.drawable.xm16_png);
        sDrawableMap.put(drawable.xm17_png, R.drawable.xm17_png);
        sDrawableMap.put(drawable.xm18_png, R.drawable.xm18_png);
        sDrawableMap.put(drawable.xm19_png, R.drawable.xm19_png);
        sDrawableMap.put(drawable.xm20_png, R.drawable.xm20_png);
        sDrawableMap.put(drawable.xm21_png, R.drawable.xm21_png);
        sDrawableMap.put(drawable.xm22_png, R.drawable.xm22_png);
        sDrawableMap.put(drawable.xm23_png, R.drawable.xm23_png);
        sDrawableMap.put(drawable.xm24_png, R.drawable.xm24_png);

        sDrawableMap.put(drawable.ipmsg_service, R.drawable.isms_service);
        sDrawableMap.put(drawable.ipmsg_sim_indicator, R.drawable.imsp_ipmsg_sim_indicator);
        sDrawableMap.put(drawable.ipmsg_delete_caption, R.drawable.ic_input_field_image_delete);
        sDrawableMap.put(drawable.ipmsg_emotion_unread_prompt, R.drawable.share_new_indicator);
        sDrawableMap.put(drawable.ipmsg_chat_button_grey, R.drawable.isms_chat_button_grey_nor);
        sDrawableMap.put(drawable.ipmsg_chat_button_green, R.drawable.isms_chat_button_green_nor);
        sDrawableMap.put(drawable.ic_chronograph, R.drawable.ic_chronograph);
        sDrawableMap.put(drawable.ipmsg_share_burn, R.drawable.isms_share_burn);
        sDrawableMap.put(drawable.ipmsg_share_burn_active, R.drawable.isms_share_burn_active);
        sDrawableMap.put(drawable.ipmsg_no_private_contacts, R.drawable.ic_no_private_contacts);
        sDrawableMap.put(drawable.ipmsg_no_private_message, R.drawable.ic_no_private_message);
        sDrawableMap.put(drawable.ipmsg_bt_private_add_contact, R.drawable.ic_bt_private_add_contact);
        sDrawableMap.put(drawable.ipmsg_bt_private_contact, R.drawable.ic_bt_private_contact);
        sDrawableMap.put(drawable.ipmsg_bt_private_setting, R.drawable.ic_bt_private_setting);
        sDrawableMap.put(drawable.ipmsg_private_message_warning, R.drawable.ic_private_message_warning);
        sDrawableMap.put(drawable.ipmsg_button_blue_nor, R.drawable.isms_chat_button_blue_nor);
        sDrawableMap.put(drawable.ipmsg_button_blue_press, R.drawable.isms_chat_button_blue_press);
        sDrawableMap.put(drawable.ipmsg_emoticon,       R.drawable.default_h);
        // raw resource
        sRawResourceMap.put(drawable.emo_dynamic_01, R.drawable.emo_dynamic_01);   // dynamicIconArr
        sRawResourceMap.put(drawable.emo_dynamic_02, R.drawable.emo_dynamic_02);
        sRawResourceMap.put(drawable.emo_dynamic_03, R.drawable.emo_dynamic_03);
        sRawResourceMap.put(drawable.emo_dynamic_04, R.drawable.emo_dynamic_04);
        sRawResourceMap.put(drawable.emo_dynamic_05, R.drawable.emo_dynamic_05);
        sRawResourceMap.put(drawable.emo_dynamic_06, R.drawable.emo_dynamic_06);
        sRawResourceMap.put(drawable.emo_dynamic_07, R.drawable.emo_dynamic_07);
        sRawResourceMap.put(drawable.emo_dynamic_08, R.drawable.emo_dynamic_08);
        sRawResourceMap.put(drawable.emo_dynamic_09, R.drawable.emo_dynamic_09);
        sRawResourceMap.put(drawable.emo_dynamic_10, R.drawable.emo_dynamic_10);
        sRawResourceMap.put(drawable.emo_dynamic_11, R.drawable.emo_dynamic_11);
        sRawResourceMap.put(drawable.emo_dynamic_12, R.drawable.emo_dynamic_12);
        sRawResourceMap.put(drawable.emo_dynamic_13, R.drawable.emo_dynamic_13);
        sRawResourceMap.put(drawable.emo_dynamic_14, R.drawable.emo_dynamic_14);
        sRawResourceMap.put(drawable.emo_dynamic_15, R.drawable.emo_dynamic_15);
        sRawResourceMap.put(drawable.emo_dynamic_16, R.drawable.emo_dynamic_16);
        sRawResourceMap.put(drawable.emo_dynamic_17, R.drawable.emo_dynamic_17);
        sRawResourceMap.put(drawable.emo_dynamic_18, R.drawable.emo_dynamic_18);
        sRawResourceMap.put(drawable.emo_dynamic_19, R.drawable.emo_dynamic_19);
        sRawResourceMap.put(drawable.emo_dynamic_20, R.drawable.emo_dynamic_20);
        sRawResourceMap.put(drawable.emo_dynamic_21, R.drawable.emo_dynamic_21);
        sRawResourceMap.put(drawable.emo_dynamic_22, R.drawable.emo_dynamic_22);
        sRawResourceMap.put(drawable.emo_dynamic_23, R.drawable.emo_dynamic_23);
        sRawResourceMap.put(drawable.emo_dynamic_24, R.drawable.emo_dynamic_24);

        sRawResourceMap.put(drawable.ad01, R.drawable.ad01);         // adIconArr
        sRawResourceMap.put(drawable.ad02, R.drawable.ad02);
        sRawResourceMap.put(drawable.ad03, R.drawable.ad03);
        sRawResourceMap.put(drawable.ad04, R.drawable.ad04);
        sRawResourceMap.put(drawable.ad05, R.drawable.ad05);
        sRawResourceMap.put(drawable.ad06, R.drawable.ad06);
        sRawResourceMap.put(drawable.ad07, R.drawable.ad07);
        sRawResourceMap.put(drawable.ad08, R.drawable.ad08);
        sRawResourceMap.put(drawable.ad09, R.drawable.ad09);
        sRawResourceMap.put(drawable.ad10, R.drawable.ad10);
        sRawResourceMap.put(drawable.ad11, R.drawable.ad11);
        sRawResourceMap.put(drawable.ad12, R.drawable.ad12);
        sRawResourceMap.put(drawable.ad13, R.drawable.ad13);
        sRawResourceMap.put(drawable.ad14, R.drawable.ad14);
        sRawResourceMap.put(drawable.ad15, R.drawable.ad15);
        sRawResourceMap.put(drawable.ad16, R.drawable.ad16);
        sRawResourceMap.put(drawable.ad17, R.drawable.ad17);
        sRawResourceMap.put(drawable.ad18, R.drawable.ad18);
        sRawResourceMap.put(drawable.ad19, R.drawable.ad19);
        sRawResourceMap.put(drawable.ad20, R.drawable.ad20);
        sRawResourceMap.put(drawable.ad21, R.drawable.ad21);
        sRawResourceMap.put(drawable.ad22, R.drawable.ad22);
        sRawResourceMap.put(drawable.ad23, R.drawable.ad23);
        sRawResourceMap.put(drawable.ad24, R.drawable.ad24);

        sRawResourceMap.put(drawable.xm01, R.drawable.xm01);         // xmIconArr
        sRawResourceMap.put(drawable.xm02, R.drawable.xm02);
        sRawResourceMap.put(drawable.xm03, R.drawable.xm03);
        sRawResourceMap.put(drawable.xm04, R.drawable.xm04);
        sRawResourceMap.put(drawable.xm05, R.drawable.xm05);
        sRawResourceMap.put(drawable.xm06, R.drawable.xm06);
        sRawResourceMap.put(drawable.xm07, R.drawable.xm07);
        sRawResourceMap.put(drawable.xm08, R.drawable.xm08);
        sRawResourceMap.put(drawable.xm09, R.drawable.xm09);
        sRawResourceMap.put(drawable.xm10, R.drawable.xm10);
        sRawResourceMap.put(drawable.xm11, R.drawable.xm11);
        sRawResourceMap.put(drawable.xm12, R.drawable.xm12);
        sRawResourceMap.put(drawable.xm13, R.drawable.xm13);
        sRawResourceMap.put(drawable.xm14, R.drawable.xm14);
        sRawResourceMap.put(drawable.xm15, R.drawable.xm15);
        sRawResourceMap.put(drawable.xm16, R.drawable.xm16);
        sRawResourceMap.put(drawable.xm17, R.drawable.xm17);
        sRawResourceMap.put(drawable.xm18, R.drawable.xm18);
        sRawResourceMap.put(drawable.xm19, R.drawable.xm19);
        sRawResourceMap.put(drawable.xm20, R.drawable.xm20);
        sRawResourceMap.put(drawable.xm21, R.drawable.xm21);
        sRawResourceMap.put(drawable.xm22, R.drawable.xm22);
        sRawResourceMap.put(drawable.xm23, R.drawable.xm23);
        sRawResourceMap.put(drawable.xm24, R.drawable.xm24);
    }

    public ResourceManagerExt(Context context) {
        super(context);
    }

    /**
    * Get string by resource id.
    * @param id is special resource id.
    */
    public String getSingleString(int id) {
        if (sStringMap.get(id) == null) {
            return null;
        }
        return mContext.getString(sStringMap.get(id).intValue());
    }

    /**
    * Get String from remote.
    * @param id with special resource id.
    * @param formatArgs the format arguments will be used for substitution.
    */
    public String getSingleString(int id, Object... formatArgs) {
        if (sStringMap.get(id) == null) {
            return null;
        }
        return mContext.getString(sStringMap.get(id).intValue(), formatArgs);
    }

    /**
    * Get String array from remote.
    * @param id with special resource id.
    */
    public String[] getStringArray(int id) {
        if (sStringArrayMap.get(id) == null) {
            return null;
        }
        return mContext.getResources().getStringArray(sStringArrayMap.get(id).intValue());
    }

    /**
    * Get drawable from remote.
    * @param id with special resource id.
    */
    public Drawable getSingleDrawable(int id) {
        if (sDrawableMap.get(id) == null) {
            return null;
        }
        return mContext.getResources().getDrawable(sDrawableMap.get(id).intValue());
    }

    /**
    * Get InputStream from remote.
    * @param id with special resource id.
    */
    public InputStream getRawResource(int id) {
        if (sRawResourceMap.get(id) == null) {
            return null;
        }
        return mContext.getResources().openRawResource(sRawResourceMap.get(id).intValue());
    }
}
