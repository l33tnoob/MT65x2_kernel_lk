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

/* //hardware/ril/reference-ril/ril_stk.c
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

#include <telephony/ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <ril_callbacks.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#define STK_CHANNEL_CTX getRILChannelCtxFromToken(t)
static const struct timeval TIMEVAL_0 = {2,0};
static const struct timeval TIMEVAL_SMS = {0,0};
static sat_at_string_struct g_stk_at;

#if 0
static int is_stk_service_running = 1;
static int is_proac_cmd_queued = 0;
static char proactive_cmd[256] = {0};
#endif

void requestStkGetProfile (void *data, size_t datalen, RIL_Token t)
{

}

void requestStkSetProfile (void *data, size_t datalen, RIL_Token t)
{

}

void onStkAtSendFromUrc()
{
    switch(g_stk_at.cmd_type)
    {
        case CMD_SETUP_CALL:
            if (g_stk_at.cmd_res == 50)
            {
                at_send_command("AT+STKCALL=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }
            break;
        case CMD_DTMF:
            // at_send_command("AT+STKDTMF=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            if(inCallNumber != 0) {
                at_send_command("AT+STKDTMF=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            } else {
                at_send_command("AT+STKDTMF=32,7", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }
            break;
        case CMD_SEND_SMS:
            at_send_command("AT+STKSMS=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            break;
        case CMD_SEND_SS:
            if (g_stk_at.cmd_res == 50)
            {
                at_send_command("AT+STKSS=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }			
            else if (g_stk_at.cmd_res == 0)
            {
                at_send_command("AT+STKSS=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }
            break;
        case CMD_SEND_USSD:
            if (g_stk_at.cmd_res == 50)
            {
                at_send_command("AT+STKUSSD=50", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }			
            else if (g_stk_at.cmd_res == 0)
            {
                at_send_command("AT+STKUSSD=0", NULL, getChannelCtxbyProxy(g_stk_at.rid));
            }			
            break;
        default:
            break;			
    }
}

void StkSendRequestComplete(int err, ATResponse *p_response, RIL_Token t)
{
    if (err < 0 || p_response->success == 0) 
    {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } 
    else 
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

#if 0
void requestReportStkServiceIsRunning(void *data, size_t datalen, RIL_Token t)
{
    is_stk_service_running = 1;
	
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    LOGD("STK service is running");
    if(1 == is_proac_cmd_queued)
    {
        onStkProactiveCommand(proactive_cmd);
    }
    
}
#endif

void requestStkSendEnvelopeCommand (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;   

    asprintf(&cmd, "AT+STKENV=\"%s\"", (char *)data);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);	

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);

}

void requestStkSendTerminalResponse (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;   

    asprintf(&cmd, "AT+STKTR=\"%s\"", (char *)data);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);	

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void requestStkHandleCallSetupRequestedFromSim (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0, user_confirm = 0, addtional_info = 0;   

    if(((int *)data)[0] == 1)
    {
        user_confirm = 0;
    }
    else if(((int *)data)[0] == 32)  //ME currently unable to process
    {
        user_confirm = 32;
        addtional_info = 2;
    }
    else if(((int *)data)[0] == 33) //NW currently unable to process
    {
        user_confirm = 33;
        addtional_info = 0x9d;
    }	
    else if(((int *)data)[0] == 0)
    {
        user_confirm = 34;
    }
    else
    {
        assert(0);
    }

    if( addtional_info == 0)
    {
        asprintf(&cmd, "AT+STKCALL=%d", user_confirm);
    }
    else
    {
        asprintf(&cmd, "AT+STKCALL=%d, %d", user_confirm, addtional_info);
    }

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);	

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);

}

void requestStkSetEvdlCallByAP (void *data, size_t datalen, RIL_Token t)
{
    char* cmd;
    ATResponse *p_response = NULL;
    int err;
    int enabled = ((int *)data)[0];
    LOGD("requestStkSetEvdlCallByAP:%d.",enabled);

    asprintf(&cmd, "AT+EVDLCALL=%d", enabled);

    err = at_send_command(cmd, &p_response, STK_CHANNEL_CTX);
    free(cmd);	

    StkSendRequestComplete(err, p_response, t);

    at_response_free(p_response);
}

void onStkSessionEnd(char* urc, RILChannelCtx* p_channel)
{
    RILId rid = getRILIdByChannelCtx(p_channel);
    
    RIL_onUnsolicitedResponse (
            RIL_UNSOL_STK_SESSION_END,
            NULL, 0, 
            rid);
    return;
}
#define NUM_EVDL_CALL 6
void onStkEventDownloadCall(char* urc, RILChannelCtx* p_channel)
{
    RILId rid = getRILIdByChannelCtx(p_channel);

    int err, i = 0;
    char *p_cur = (char *)urc;
    int response[NUM_EVDL_CALL]={0};    
    //RIL_EVDLCall response = NULL;

    /*
      * +EVDLCALL: <status>,<ti>,<is_mt_call>,<is_far_end>,<cause_len>,<cause>
      */
    for(i = 0; i < NUM_EVDL_CALL-1; i++)
        response[i] = 0xff;
      
    err = at_tok_start(&p_cur);
    if (err < 0) goto error;

    for(i = 0; i < NUM_EVDL_CALL-1; i++) 
    {
        err = at_tok_nextint(&p_cur, &(response[i]));
        if(err < 0) { response[i] = 0xFF; LOGE("There is something wrong with item [%d]",i);}//goto error;
    }
    err = at_tok_nexthexint(&p_cur, &(response[i]));	
    if(err < 0) LOGE("There is something wrong with item [%d]",i);//goto error;
	
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_STK_EVDL_CALL,
            response, NUM_EVDL_CALL * sizeof(int),
            rid);
            
    return;   
error:
    LOGE("There is something wrong with the +EVDLCALL");	
}

int checkStkCmdDisplay(char *cmd_str)
{
    int is_alpha_id_existed = 0;
    int is_icon_existed_not_self_explanatory = 0;
    int index = 0;
    int cmd_length = 0;
    char temp_str[3] = {0};
    char *end;    

    cmd_length = strlen(cmd_str);

    while (cmd_length > index)
    {
        if (cmd_str[index+1] == '5' && (cmd_str[index] == '0' || cmd_str[index] == '8') )
        {

            index += 2;
            if (cmd_str[index] != '0' ||cmd_str[index+1] != '0' )
            {
                is_alpha_id_existed = 1;
            }
            if (cmd_str[index] <= '7')
            {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            }
            else 
            {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;        
            }
        }
        else if((cmd_str[index+1] == 'E' || cmd_str[index+1] == 'e') && (cmd_str[index] == '1' || cmd_str[index] == '9'))
        {
            int icon_qualifier = 0;
			        
            index += 4;
            memset(temp_str, 0, 3);
            memcpy(temp_str, &(cmd_str[index + 1]), 1);
            icon_qualifier = strtoul(temp_str, &end, 16);

            if((icon_qualifier & 0x01) == 0x01)
            {
                if(is_alpha_id_existed == 0)
                {
                    return 1;
                }
            }
            index += 4;
        }
        else
        {
            index += 2;
            if (cmd_str[index] <= '7')
            {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            }
            else 
            {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;        
            }			
        }
    }
    return 0;
}

int checkStkCommandType(char *cmd_str)
{
    char temp_str[3] = {0};
    char *end;
    int cmd_type = 0;

    memcpy(temp_str, cmd_str, 2);

    cmd_type = strtoul(temp_str, &end, 16);
    cmd_type = 0x7F & cmd_type;

    return cmd_type;
}

void decodeStkRefreshFileChange(char *str, int **cmd, int *cmd_length)
{
    int str_length = 0, file_num = 0, offset = 20, cmdoffset = 0;  
	/*offset 20 including cmd_detail tlv: 10, device id tlv:8, file list tag:2*/
    char temp_str[5] = {0};
    char *end;

    str_length = strlen(str);

    if(str[offset] <= '7')//file list length: if length < 7F it will use 2 bytes else it will use 4 bytes
    {
        offset += 2;
    }
    else
    {
        offset += 4;
    }
    memcpy(temp_str, str+offset, 2);  //copy number of files in file list to temp_str
    offset += 2;
	
    file_num = strtoul(temp_str, &end, 16);

#ifndef MTK_WIFI_CALLING_RIL_SUPPORT
    *cmd_length = (file_num+1) * sizeof(int);
    *cmd = (int*)malloc(*cmd_length);
	
#else
    *cmd_length = (file_num+2) * sizeof(int);
    *cmd = (int*)malloc(*cmd_length);
    cmdoffset++;

 #endif

    *(*cmd+cmdoffset) = SIM_FILE_UPDATE;
	
    cmdoffset++;
	
    while( offset < str_length)
    {
        if(((str[offset] == '6')||(str[offset] == '2')||(str[offset] == '4'))
	&&((str[offset+1] == 'F')/*||(str[offset+1] == 'f')*/))
        {
            memcpy(temp_str, str+offset, 4);  //copy EFID to temo_str
            *(*cmd+cmdoffset) = strtoul(temp_str, &end, 16);

            cmdoffset++;
        }
        offset += 4;
    }
}

#ifdef MTK_WIFI_CALLING_RIL_SUPPORT
extern int rild_sms_hexCharToDecInt(char *hex, int length);

int decodeStkRefreshAid(char *urc, char **paid)
{
    int offset = 18; //cmd_details & device identifies
    int refresh_length = strlen(urc) / 2;
    int files_length;
    int files_offset;
    int temp;

    if (offset >= refresh_length) {
        *paid = NULL;
        return 0;
    }

    temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
    offset += 2;
    if (temp == 0x12 || temp == 0x92) { //file list tag
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
        if (temp < 0x7F) {
            offset += (2 + temp * 2);
        } else {
            offset += 2;
            temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
            offset += (2 + temp * 2);
        }
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
    }

    LOGD("decodeStkRefreshAid temp = %02x, offset = %d", temp, offset);

    if (temp == 0x2F || temp == 0xAF) { // aid tag
        offset += 2;
        temp = rild_sms_hexCharToDecInt(&urc[offset], 2);
        offset += 2;
        *paid = &urc[offset];
        return temp * 2;
    }

    return -1;
}
#endif /* MTK_WIFI_CALLING_RIL_SUPPORT */

void onSimRefresh(char* urc, RILChannelCtx * p_channel)
{
    int *cmd = NULL;
    int cmd_length = 0;
    RILId rid = getRILIdByChannelCtx(p_channel);
    
#ifdef MTK_WIFI_CALLING_RIL_SUPPORT
    int sessionId = 0;
    int aid_len = 0;
    char *aid = NULL;
#endif

    switch(urc[9])  // t point to cmd_deatil tag t[9] mean refresh type
    {
        case '1':
            decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
				
            break;
        case '4':
            #ifndef MTK_WIFI_CALLING_RIL_SUPPORT
            cmd_length = 2*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = SIM_RESET;
            cmd[1] = 0;

            #else /*  MTK_WIFI_CALLING_RIL_SUPPORT */
                cmd_length = 3*sizeof(int);
                cmd = (int *)malloc(cmd_length);
                cmd[0] = -1;
                cmd[1] = SIM_RESET;
                cmd[2] = 0;
            #endif
            break;
        case '0':
            #ifndef MTK_WIFI_CALLING_RIL_SUPPORT
            cmd_length = 2*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = SIM_INIT_FULL_FILE_CHANGE;			
            cmd[1] = 0;
            #else
            cmd_length = 3*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = -1;
            cmd[1] = SIM_INIT_FULL_FILE_CHANGE;			
            cmd[2] = 0;
            #endif
            break;
        case '2':
            decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            #ifndef MTK_WIFI_CALLING_RIL_SUPPORT
            cmd[0] = SIM_INIT_FILE_CHANGE;			
            #else
            cmd[1] = SIM_INIT_FILE_CHANGE;			
            #endif
            break;
        case '3':
            #ifndef MTK_WIFI_CALLING_RIL_SUPPORT
            cmd_length = 2*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = SIM_INIT;			
            cmd[1] = 0;

            #else /*  MTK_WIFI_CALLING_RIL_SUPPORT */
                cmd_length = 3*sizeof(int);
                cmd = (int *)malloc(cmd_length);
                cmd[0] = -1;
                cmd[1] = SIM_INIT;
                cmd[2] = 0;
            #endif
            break;

    #ifdef MTK_WIFI_CALLING_RIL_SUPPORT
        case '5': // ISIM app_reset
            aid_len = decodeStkRefreshAid(urc, &aid);
            sessionId = getActiveLogicalChannelId(aid);
            LOGD("[WiFi_Calling]decodeStkRefreshAid sessionId = %d", sessionId);
            cmd_length = 3*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = sessionId;
            cmd[1] = APP_INIT;
            cmd[2] = 0;
            break;
    #endif /* MTK_WIFI_CALLING_RIL_SUPPORT */
        case '6':
        #ifndef MTK_WIFI_CALLING_RIL_SUPPORT
            cmd_length = 2*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = SESSION_RESET;			
            cmd[1] = 0;
        #else /*  MTK_WIFI_CALLING_RIL_SUPPORT */
            cmd_length = 3*sizeof(int);
            cmd = (int *)malloc(cmd_length);
            cmd[0] = -1;
            cmd[1] = SESSION_RESET;
            cmd[2] = 0;
        #endif
            break;

        default:
            break;
    }
    RIL_onUnsolicitedResponse (
        RIL_UNSOL_SIM_REFRESH,
        cmd, cmd_length,
        rid);	
    free(cmd); 	
}

void onStkProactiveCommand(char* urc, RILChannelCtx* p_channel)
{
    int err = 0, temp_int = 0, type_pos = 0;
    ATResponse *p_response = NULL;	
    char *temp_str;
    char *cmd;
    RILId rid = getRILIdByChannelCtx(p_channel);

#if 0
    if(0 == is_stk_service_running)
    {
        is_proac_cmd_queued = 1;
        memcpy(proactive_cmd, urc, strlen(urc));
        LOGD("STK service is not running yet");

        return;
    }
#endif

    err = at_tok_start(&urc);

    err = at_tok_nextint(&urc, &temp_int);

    err = at_tok_nextstr(&urc, &temp_str);

    if(temp_str[2] <= '7' ) /*add comment*/
    {
        type_pos = 10;
    }
    else
    {
        type_pos = 12;
    }    
    switch(checkStkCommandType(&(temp_str[type_pos])))
    {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos -6]),p_channel);	
            // return;
        break;
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
            		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            RIL_onUnsolicitedResponse (
                    RIL_UNSOL_STK_EVENT_NOTIFY,
                    temp_str, strlen(temp_str),
                    rid);	
            return;
        break;
        default:
        break;
    }
    RIL_onUnsolicitedResponse (
            RIL_UNSOL_STK_PROACTIVE_COMMAND,
            temp_str, strlen(temp_str),
            rid);

    return;
}

unsigned int findStkCallDuration(char* str)
{
    int length = 0, offset = 0, temp = 0;
    unsigned int duration = 0;
    char temp_str[3] = {0};
    char *end;

    length = strlen(str);

    while(length > 0)
    {

        if(str[offset] == '8' && str[offset+1] == '4')
        {
            memcpy(temp_str, &(str[offset+6]), 2);
            if(str[offset+5] == '0')
            {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 60000;
            }
            else if(str[offset+5] == '1')
            {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 1000;            
            }
            else if(str[offset+5] == '2')
            {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 100;            
            }			
            break;
        }
        else
        {
            length -= 2;
            offset += 2;
            memcpy(temp_str, &(str[offset]), 2);
            temp = strtoul(temp_str, &end, 16);
            length -= (2*temp +2);
            offset += (2*temp +2);
			
        }
    }
    return duration;
}

void onStkEventNotify(char* urc, RILChannelCtx* p_channel)
{
    int err = 0;
    int temp_int = 0;
    int cmd_type = 0;
    int type_pos = 0;
    int cmd_not_understood = 0; /* mtk02374 20100502*/
    unsigned int duration = 0;
    char *temp_str;
    char *cmd;
    RILId rid = getRILIdByChannelCtx(p_channel);
	
    ATResponse *p_response = NULL;
	
    err = at_tok_start(&urc);
    err = at_tok_nextint(&urc, &temp_int);
    err = at_tok_nextstr(&urc, &temp_str);
    if(temp_str[2] <= '7' ) /*add comment*/
    {
        type_pos = 10;
    }
    else if(temp_str[2] >'7' )
    {
        type_pos = 12;
    }
    cmd_not_understood = checkStkCmdDisplay(&(temp_str[type_pos-6])); /*temp_str[type_pos -6] points to cmd_detail tag*/
    switch(checkStkCommandType(&(temp_str[type_pos])))
    {
        case CMD_REFRESH:
            onSimRefresh(&(temp_str[type_pos -6]),p_channel);
            break;
        case CMD_SETUP_CALL:
            cmd_type = CMD_SETUP_CALL;
            if(cmd_not_understood == 0)
            {
                duration = findStkCallDuration(&temp_str[type_pos -6]);	/*temp_str[type_pos -6] points to cmd_detail tag*/		
            }
            else
            {
                g_stk_at.cmd_type = CMD_SETUP_CALL;
                g_stk_at.cmd_res = 50;
                g_stk_at.rid = rid;
                RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
                		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");
		
            }
            break;
        case CMD_SEND_SMS:
            g_stk_at.cmd_type = CMD_SEND_SMS;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_SMS,
            		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        case CMD_SEND_SS:
            g_stk_at.cmd_type = CMD_SEND_SS;
            if(cmd_not_understood == 0)
            {
                g_stk_at.cmd_res = 0;
            }
            else
            {
                g_stk_at.cmd_res = 50;
            }

            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
            		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        case CMD_SEND_USSD:
            g_stk_at.cmd_type = CMD_SEND_USSD;
            if(cmd_not_understood == 0)
            {
                g_stk_at.cmd_res = 0;
            }
            else
            {
                g_stk_at.cmd_res = 50;
            }			
            g_stk_at.rid = rid;
            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
            		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");
  
            break;			
        case CMD_DTMF:
            g_stk_at.cmd_type = CMD_DTMF;
            g_stk_at.cmd_res = 0;
            g_stk_at.rid = rid;

            RIL_requestProxyTimedCallback(onStkAtSendFromUrc, NULL, &TIMEVAL_0,
            		getChannelCtxbyProxy(rid)->id, "onStkAtSendFromUrc");

            break;
        default:
            break;
    }

    RIL_onUnsolicitedResponse (
            RIL_UNSOL_STK_EVENT_NOTIFY,
            temp_str, strlen(temp_str),
            rid);
    if(CMD_SETUP_CALL == cmd_type)
    {
        RIL_onUnsolicitedResponse (
                RIL_UNSOL_STK_CALL_SETUP,
                &duration, sizeof(duration),
                rid);        
    }
    return;
}

extern int rilStkMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            MTK_REQUEST_STK_SEND_ENVELOPE_COMMAND(data, datalen, t);
            break;
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            MTK_REQUEST_STK_SEND_TERMINAL_RESPONSE(data, datalen, t);
            break;
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
            MTK_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM(data, datalen, t);
            break;    
        case RIL_REQUEST_STK_EVDL_CALL_BY_AP:
            MTK_REQUEST_STK_SET_EVDL_CALL_BY_AP(data, datalen, t);
            break;
#if 0            
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            MTK_REQUEST_REPORT_STK_SERVICE_IS_RUNNING(data, datalen, t);
            break;
#endif            
        default:
            return 0; /* no matched requests */    
        break;
    }

    return 1; /* request found and handled */
}

extern int rilStkUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    if(strStartsWith(s, "+STKPCI: 0"))
    {
        MTK_UNSOL_STK_PROACTIVE_COMMAND((char *)s, p_channel);
    } 
    else if(strStartsWith(s, "+STKPCI: 1"))
    {
        MTK_UNSOL_STK_EVENT_NOTIFY((char *)s, p_channel);
    } 
    else if(strStartsWith(s, "+STKPCI: 2"))
    {
        MTK_UNSOL_STK_SESSION_END((char *)s, p_channel);
    }
    else if(strStartsWith(s, "+EVDLCALL:"))
    {
        MTK_UNSOL_STK_EVDL_CALL((char *)s, p_channel);
    }
    else
        return 0;
    return 1;
}


