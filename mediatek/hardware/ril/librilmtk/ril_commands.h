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

/* //device/libs/telephony/ril_commands.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef MTK_RIL
    {0, NULL, NULL},                   //none
    {RIL_REQUEST_GET_SIM_STATUS, dispatchVoid, responseSimStatus},
    {RIL_REQUEST_ENTER_SIM_PIN, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PUK, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PIN2, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PUK2, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_SIM_PIN, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_SIM_PIN2, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, dispatchStrings, responseInts},
    {RIL_REQUEST_GET_CURRENT_CALLS, dispatchVoid, responseCallList},
    {RIL_REQUEST_DIAL, dispatchDial, responseVoid},
    {RIL_REQUEST_GET_IMSI, dispatchVoid, responseString},
    {RIL_REQUEST_HANGUP, dispatchInts, responseVoid},
    {RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid},
    {RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid},
    {RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid},
    {RIL_REQUEST_CONFERENCE, dispatchVoid, responseVoid},
    {RIL_REQUEST_UDUB, dispatchVoid, responseVoid},
    {RIL_REQUEST_LAST_CALL_FAIL_CAUSE, dispatchVoid, responseInts},
    {RIL_REQUEST_SIGNAL_STRENGTH, dispatchVoid, responseRilSignalStrength},
    {RIL_REQUEST_VOICE_REGISTRATION_STATE, dispatchVoid, responseStrings},
    {RIL_REQUEST_DATA_REGISTRATION_STATE, dispatchVoid, responseStrings},
    {RIL_REQUEST_OPERATOR, dispatchVoid, responseStrings},
    {RIL_REQUEST_RADIO_POWER, dispatchInts, responseVoid},
    {RIL_REQUEST_DTMF, dispatchString, responseVoid},
    {RIL_REQUEST_SEND_SMS, dispatchStrings, responseSMS},
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, dispatchStrings, responseSMS},
    {RIL_REQUEST_SETUP_DATA_CALL, dispatchStrings, responseSetupDataCall},
    {RIL_REQUEST_SIM_IO, dispatchSIM_IO, responseSIM_IO},
    {RIL_REQUEST_SEND_USSD, dispatchUssd, responseVoid},
    {RIL_REQUEST_CANCEL_USSD, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_CLIR, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_CLIR, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, dispatchCallForward, responseCallForwards},
    {RIL_REQUEST_SET_CALL_FORWARD, dispatchCallForward, responseVoid},
    {RIL_REQUEST_QUERY_CALL_WAITING, dispatchInts, responseInts},
    {RIL_REQUEST_SET_CALL_WAITING, dispatchInts, responseVoid},
    {RIL_REQUEST_SMS_ACKNOWLEDGE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_IMEI, dispatchVoid, responseString},
    {RIL_REQUEST_GET_IMEISV, dispatchVoid, responseString},
    {RIL_REQUEST_ANSWER,dispatchVoid, responseVoid},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid},
    {RIL_REQUEST_QUERY_FACILITY_LOCK, dispatchStrings, responseInts},
    {RIL_REQUEST_SET_FACILITY_LOCK, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, dispatchStrings, responseVoid},
    {RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, dispatchString, responseVoid},
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseStrings},
    {RIL_REQUEST_DTMF_START, dispatchString, responseVoid},
    {RIL_REQUEST_DTMF_STOP, dispatchVoid, responseVoid},
    {RIL_REQUEST_BASEBAND_VERSION, dispatchVoid, responseString},
    {RIL_REQUEST_SEPARATE_CONNECTION, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_MUTE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_MUTE, dispatchVoid, responseInts},
    {RIL_REQUEST_QUERY_CLIP, dispatchVoid, responseInts},
    {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts},
    {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList},
    {RIL_REQUEST_RESET_RADIO, dispatchVoid, responseVoid},
    {RIL_REQUEST_OEM_HOOK_RAW, dispatchRaw, responseRaw},
    {RIL_REQUEST_OEM_HOOK_STRINGS, dispatchStrings, responseStrings},
    {RIL_REQUEST_SCREEN_STATE, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, dispatchInts, responseVoid},
    {RIL_REQUEST_WRITE_SMS_TO_SIM, dispatchSmsWrite, responseInts},
    {RIL_REQUEST_DELETE_SMS_ON_SIM, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_BAND_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_STK_GET_PROFILE, dispatchVoid, responseString},
    {RIL_REQUEST_STK_SET_PROFILE, dispatchString, responseVoid},
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, dispatchString, responseString},
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, dispatchString, responseVoid},
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, dispatchInts, responseVoid},
    {RIL_REQUEST_EXPLICIT_CALL_TRANSFER, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, dispatchVoid, responseInts},
    {RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, dispatchVoid, responseCellList},
    {RIL_REQUEST_SET_LOCATION_UPDATES, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_TTY_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_TTY_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_CDMA_FLASH, dispatchString, responseVoid},
    {RIL_REQUEST_CDMA_BURST_DTMF, dispatchStrings, responseVoid},
    {RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, dispatchString, responseVoid},
    {RIL_REQUEST_CDMA_SEND_SMS, dispatchCdmaSms, responseSMS},
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, dispatchCdmaSmsAck, responseVoid},
    {RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseGsmBrSmsCnf},
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, dispatchGsmBrSmsCnf, responseVoid},
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseCdmaBrSmsCnf},
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, dispatchCdmaBrSmsCnf, responseVoid},
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SUBSCRIPTION, dispatchVoid, responseStrings},
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, dispatchRilCdmaSmsWriteArgs, responseInts},
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, dispatchInts, responseVoid},
    {RIL_REQUEST_DEVICE_IDENTITY, dispatchVoid, responseStrings},
    {RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_SMSC_ADDRESS, dispatchVoid, responseString},
    {RIL_REQUEST_SET_SMSC_ADDRESS, dispatchString, responseVoid},
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, dispatchInts, responseVoid},
    {RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, dispatchVoid, responseVoid},
    {RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, dispatchVoid, responseInts},
    {RIL_REQUEST_ISIM_AUTHENTICATION, dispatchString, responseString},
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, dispatchStrings, responseVoid},
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, dispatchString, responseSIM_IO},
    {RIL_REQUEST_VOICE_RADIO_TECH, dispatchVoid, responseInts},    
    {RIL_REQUEST_GET_CELL_INFO_LIST, dispatchVoid, responseCellInfoList},
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, dispatchSetInitialAttachApn, responseVoid},
#else
    {0, NULL, NULL, RIL_SUPPORT_PROXYS},                   //none
    {RIL_REQUEST_GET_SIM_STATUS,dispatchVoid, responseSimStatus, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ENTER_SIM_PIN, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ENTER_SIM_PUK, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ENTER_SIM_PIN2, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ENTER_SIM_PUK2, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CHANGE_SIM_PIN, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CHANGE_SIM_PIN2, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, dispatchStrings, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_GET_CURRENT_CALLS, dispatchVoid, responseCallList, RIL_CMD_PROXY_2},
    {RIL_REQUEST_DIAL, dispatchDial, responseVoid, RIL_CMD_PROXY_2},   
    {RIL_REQUEST_GET_IMSI, dispatchVoid, responseString, RIL_CMD_PROXY_1},
    {RIL_REQUEST_HANGUP, dispatchInts, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_CONFERENCE, dispatchVoid, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_UDUB, dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_LAST_CALL_FAIL_CAUSE, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
    {RIL_REQUEST_SIGNAL_STRENGTH, dispatchVoid, responseRilSignalStrength, RIL_CMD_PROXY_3},
    {RIL_REQUEST_VOICE_REGISTRATION_STATE, dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_DATA_REGISTRATION_STATE, dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_OPERATOR, dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_RADIO_POWER, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_DTMF, dispatchString, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_SEND_SMS, dispatchStrings, responseSMS, RIL_CMD_PROXY_1},
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, dispatchStrings, responseSMS, RIL_CMD_PROXY_1},
    {RIL_REQUEST_SETUP_DATA_CALL, dispatchStrings, responseSetupDataCall, RIL_CMD_PROXY_5},
    {RIL_REQUEST_SIM_IO, dispatchSIM_IO, responseSIM_IO, RIL_CMD_PROXY_1},
    {RIL_REQUEST_SEND_USSD, dispatchUssd, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CANCEL_USSD, dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_GET_CLIR, dispatchVoid, responseInts, RIL_CMD_PROXY_1},  /* Solve [ALPS00284553] Change to 1, mtk04070, 20120516 */
    {RIL_REQUEST_SET_CLIR, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, dispatchCallForward, responseCallForwards, RIL_CMD_PROXY_1},
    {RIL_REQUEST_SET_CALL_FORWARD, dispatchCallForward, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_QUERY_CALL_WAITING, dispatchInts, responseInts, RIL_CMD_PROXY_1},   /* Solve [ALPS00284553] Change to 1, mtk04070, 20120516 */
    {RIL_REQUEST_SET_CALL_WAITING, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_SMS_ACKNOWLEDGE, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_GET_IMEI, dispatchVoid, responseString, RIL_CMD_PROXY_3},
    {RIL_REQUEST_GET_IMEISV, dispatchVoid, responseString, RIL_CMD_PROXY_3},
    {RIL_REQUEST_ANSWER,dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid, RIL_CMD_PROXY_5},
    {RIL_REQUEST_QUERY_FACILITY_LOCK, dispatchStrings, responseInts, RIL_CMD_PROXY_1},    
    {RIL_REQUEST_SET_FACILITY_LOCK, dispatchStrings, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, dispatchStrings, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, dispatchVoid, responseInts ,RIL_CMD_PROXY_3},
    {RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, dispatchVoid, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, dispatchString, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_DTMF_START, dispatchString, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_DTMF_STOP, dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_BASEBAND_VERSION, dispatchVoid, responseString, RIL_CMD_PROXY_3},
    {RIL_REQUEST_SEPARATE_CONNECTION, dispatchInts, responseVoid, RIL_CMD_PROXY_4},
    {RIL_REQUEST_SET_MUTE, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_GET_MUTE, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
    {RIL_REQUEST_QUERY_CLIP, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
    {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts, RIL_CMD_PROXY_5},
    {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList, RIL_CMD_PROXY_5},
    {RIL_REQUEST_RESET_RADIO, dispatchVoid, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_OEM_HOOK_RAW, dispatchRaw, responseRaw, RIL_CMD_PROXY_6},
    {RIL_REQUEST_OEM_HOOK_STRINGS, dispatchStrings, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_SCREEN_STATE, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_WRITE_SMS_TO_SIM, dispatchSmsWrite, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_DELETE_SMS_ON_SIM, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_SET_BAND_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, dispatchVoid, responseInts, RIL_CMD_PROXY_3},
    {RIL_REQUEST_STK_GET_PROFILE, dispatchVoid, responseString, RIL_CMD_PROXY_1},
    {RIL_REQUEST_STK_SET_PROFILE, dispatchString, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, dispatchString, responseString, RIL_CMD_PROXY_1},
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, dispatchString, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_EXPLICIT_CALL_TRANSFER, dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, dispatchVoid, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, dispatchVoid, responseCellList, RIL_CMD_PROXY_2},//ALPS01356354  : getNeighboringCellInfo might be pending and cause APP ANR
    {RIL_REQUEST_SET_LOCATION_UPDATES, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, dispatchVoid, responseInts, RIL_CMD_PROXY_3},
    {RIL_REQUEST_SET_TTY_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_3},
    {RIL_REQUEST_QUERY_TTY_MODE, dispatchVoid, responseInts, RIL_CMD_PROXY_3},
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, dispatchInts, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, dispatchVoid, responseInts, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CDMA_FLASH, dispatchString, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CDMA_BURST_DTMF, dispatchStrings, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, dispatchString, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_CDMA_SEND_SMS, dispatchCdmaSms, responseSMS, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, dispatchCdmaSmsAck, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseGsmBrSmsCnf, RIL_CMD_PROXY_2},
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, dispatchGsmBrSmsCnf, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseCdmaBrSmsCnf, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, dispatchCdmaBrSmsCnf, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_SUBSCRIPTION, dispatchVoid, responseStrings, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, dispatchRilCdmaSmsWriteArgs, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_DEVICE_IDENTITY, dispatchVoid, responseStrings, RIL_CMD_PROXY_3},
    {RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, dispatchVoid, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_GET_SMSC_ADDRESS, dispatchVoid, responseString, RIL_CMD_PROXY_2},
    {RIL_REQUEST_SET_SMSC_ADDRESS, dispatchString, responseVoid, RIL_CMD_PROXY_2},
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, dispatchInts, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, dispatchVoid, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, dispatchVoid, responseInts, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ISIM_AUTHENTICATION, dispatchString, responseString, RIL_CMD_PROXY_1},
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, dispatchStrings, responseVoid, RIL_CMD_PROXY_1},
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, dispatchString, responseSIM_IO, RIL_CMD_PROXY_2},
    {RIL_REQUEST_VOICE_RADIO_TECH, dispatchVoid, responseInts, RIL_CMD_PROXY_3},    
    {RIL_REQUEST_GET_CELL_INFO_LIST, dispatchVoid, responseCellInfoList,RIL_CMD_PROXY_2}, // ALPS01286560: getallcellinfo might be pending by PLMN list such long time request and case SWT .  
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, dispatchInts, responseVoid,RIL_CMD_PROXY_3},
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, dispatchSetInitialAttachApn, responseVoid, RIL_CMD_PROXY_5},
#endif /* MTK_RIL */
