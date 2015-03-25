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

/* //hardware/ril/reference-ril/ril_stk.h
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
#ifndef RIL_STK_H 
#define RIL_STK_H 1
    

#define MTK_REQUEST_STK_GET_PROFILE(data,datalen,token) \
        requestStkGetProfile(data,datalen,token)
#define MTK_REQUEST_STK_SET_PROFILE(data,datalen,token) \
        requestStkSetProfile(data,datalen,token)
#define MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND(data,datalen,token) \
        requestStkSendEnvelopeCommand(data,datalen,token)
#define MTK_REQUEST_STK_SEND_TERMINAL_RESPONSE(data,datalen,token) \
        requestStkSendTerminalResponse(data,datalen,token)
#define MTK_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM(data,datalen,token) \
        requestStkHandleCallSetupRequestedFromSim(data,datalen,token)
#define MTK_REQUEST_STK_SET_EVDL_CALL_BY_AP(data,datalen,token) \
        requestStkSetEvdlCallByAP(data,datalen,token)
#define MTK_REQUEST_REPORT_STK_SERVICE_IS_RUNNING(data,datalen,token) \
        requestReportStkServiceIsRunning(data, datalen, token)
#define MTK_UNSOL_STK_SESSION_END(token,channel) \
        onStkSessionEnd(token,channel)
#define MTK_UNSOL_STK_EVDL_CALL(token,channel) \
        onStkEventDownloadCall(token,channel)        
#define MTK_UNSOL_STK_PROACTIVE_COMMAND(token,channel) \
        onStkProactiveCommand(token,channel)
#define MTK_UNSOL_STK_EVENT_NOTIFY(token,channel) \
        onStkEventNotify(token,channel)
#define MTK_UNSOL_STK_CALL_SETUP(token,channel) \
        onStkCallSetup(token,channel)
#define MTK_UNSOL_SIM_REFRESH(token,channel) \
        onSimRefresh(token,channel) 

extern void requestStkGetProfile (void *data, size_t datalen, RIL_Token t);
extern void requestStkSetProfile (void *data, size_t datalen, RIL_Token t);
extern void requestStkSendEnvelopeCommand (void *data, size_t datalen, RIL_Token t);
extern void requestStkSendTerminalResponse (void *data, size_t datalen, RIL_Token t);
extern void requestStkHandleCallSetupRequestedFromSim (void *data, size_t datalen, RIL_Token t);
extern void onStkSessionEnd(char* t, RILChannelCtx* p_channel);
extern void onStkProactiveCommand(char* t, RILChannelCtx* p_channel);
extern void onStkEventNotify(char* t, RILChannelCtx* p_channel);
extern void onSimRefresh(char* t, RILChannelCtx* p_channel);

extern int rilStkMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilStkUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

extern int inCallNumber;

typedef enum
{
   CMD_REFRESH=0x01,
   CMD_MORE_TIME=0x02,
   CMD_POLL_INTERVAL=0x03,    
   CMD_POLLING_OFF=0x04,   
   CMD_SETUP_EVENT_LIST=0x05,    
   CMD_SETUP_CALL=0x10,
   CMD_SEND_SS=0x11,
   CMD_SEND_USSD=0x12,
   CMD_SEND_SMS=0x13,
   CMD_DTMF=0x14,
   CMD_LAUNCH_BROWSER=0x15,
   CMD_PLAY_TONE=0x20,
   CMD_DSPL_TXT=0x21,
   CMD_GET_INKEY=0x22,
   CMD_GET_INPUT=0x23,
   CMD_SELECT_ITEM=0x24,
   CMD_SETUP_MENU=0x25,
   CMD_PROVIDE_LOCAL_INFO=0x26,
   CMD_TIMER_MANAGER=0x27,
   CMD_IDLE_MODEL_TXT=0x28,
   CMD_PERFORM_CARD_APDU=0x30,
   CMD_POWER_ON_CARD=0x31,
   CMD_POWER_OFF_CARD=0x32,
   CMD_GET_READER_STATUS=0x33,
   CMD_RUN_AT=0x34,
   CMD_LANGUAGE_NOTIFY=0x35,
   CMD_OPEN_CHAN=0x40,
   CMD_CLOSE_CHAN=0x41,
   CMD_RECEIVE_DATA=0x42,
   CMD_SEND_DATA=0x43,
   CMD_GET_CHAN_STATUS=0x44,
   CMD_RFU=0x60,
   CMD_END_PROACTIVE_SESSION=0x81,
   CMD_DETAIL=0xFF
}sat_proactive_cmd_enum;

typedef struct
{
    sat_proactive_cmd_enum cmd_type;
    int cmd_res;
    RILId rid;
}sat_at_string_struct;

#endif /* RIL_STK_H */

