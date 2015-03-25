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

/* //hardware/ril/reference-ril/ril_sms.c
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


enum{
    CMS_ERROR_NON_CMS = -1,
    CMS_SUCCESS = 0,
    CMS_CM_UNASSIGNED_NUM = 1,                  // Unassigned (unallocated) number
    CMS_CM_OPR_DTR_BARRING = 8,                 //Operator determined barring
    CMS_CM_CALL_BARRED = 10,                       // Call barred
    CMS_CM_CALL_REJECTED = 21,                    // Short message transfer rejected
    CMS_CM_DEST_OUT_OF_ORDER = 27,           //Destination out of order
    CMS_CM_INVALID_NUMBER_FORMAT = 28,   //Unidentified subscriber
    CMS_CM_FACILITY_REJECT = 29,                 // Facility rejected
    CMS_CM_RES_STATUS_ENQ = 30,                 // Unknown subscriber
    CMS_CM_NETWORK_OUT_OF_ORDER = 38,   // Network out of order
    CMS_CM_REQ_FAC_NOT_SUBS = 50,           // Requested facility not subscribed
    CMS_CM_REQ_FACILITY_UNAVAIL = 69,       // Requested facility not implemented
    CMS_CM_INVALID_TI_VALUE = 81,               // Invalid short message transfer reference value
    CMS_CM_SEMANTIC_ERR = 95,                     // Semantically incorrect message
    CMS_CM_MSG_TYPE_UNIMPL = 97,               // Message type non-existent or not implemented
    CMS_CM_IE_NON_EX = 99,                          // Information element non-existent or not implemented
    CMS_CM_SIM_IS_FULL = 322,
    CMS_UNKNOWN = 500,
    SMS_MO_SMS_NOT_ALLOW = 529,
    CMS_CM_MM_CAUSE_START = 2048,
    CMS_CM_MM_IMSI_UNKNOWN_IN_HLR = 0x02 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ILLEGAL_MS = 0x03 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ILLEGAL_ME = 0x06 + CMS_CM_MM_CAUSE_START,
    CMS_CM_RR_PLMN_SRCH_REJ_EMERGENCY = 0x74 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_AUTH_FAILURE = 0x76 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_IMSI_DETACH = 0x77 + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_EMERGENCY_NOT_ALLOWED = 0x7d + CMS_CM_MM_CAUSE_START,
    CMS_CM_MM_ACCESS_CLASS_BARRED = 0x7f + CMS_CM_MM_CAUSE_START,
    CMS_MTK_FDN_CHECK_FAILURE = 2601
}AT_CMS_ERROR;

/* TODO
* The following constant variables also defined in the ril_callbacks.c
* We should figure out a method to merge them
*/
#ifdef MTK_RIL
static const RILId s_pollSimId = MTK_RIL_SOCKET_1;
#ifdef MTK_GEMINI
static const RILId s_pollSimId2 = MTK_RIL_SOCKET_2;
#if (MTK_GEMINI_SIM_NUM >= 3)
static const RILId s_pollSimId3 = MTK_RIL_SOCKET_3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4)
static const RILId s_pollSimId4 = MTK_RIL_SOCKET_4;
#endif
#endif 
#endif /* MTK_RIL */
static const struct timeval TIMEVAL_0 = {2,0};

// default set as available
int phone_storage_status = 0;

/* 
 * This is defined on RILD and use to know MD is off or on.
 * In flight mode, it will turns off MD if feature option is ON
*/
extern int s_md_off;

/****************************************
* Temporaily, we didn't return a enum for cms ERROR
* because there are too much error casue we can't expected from modem
* 
****************************************/
static int at_get_cms_error(const ATResponse *p_response)
{
    int ret;
    int err;
    char *p_cur;

    if (p_response->success > 0) {
        return CMS_SUCCESS;
    }

    if (p_response->finalResponse == NULL)
    {
        return CMS_ERROR_NON_CMS;
    }

    if( strStartsWith(p_response->finalResponse, "ERROR" ) )
    {
        return CMS_UNKNOWN;
    }

    if( !strStartsWith(p_response->finalResponse, "+CMS ERROR:" ) )
    {
        return CMS_ERROR_NON_CMS;
    }

    p_cur = p_response->finalResponse;
    err = at_tok_start(&p_cur);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    err = at_tok_nextint(&p_cur, &ret);

    if (err < 0) {
        return CMS_ERROR_NON_CMS;
    }

    return ret;
}


int rild_sms_hexCharToDecInt(char *hex, int length)
{
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++)
    {
        if (hex[i]>='0' && hex[i]<='9')
        {
            digit = hex[i] - '0';
        }
        else if ( hex[i]>='A' && hex[i] <= 'F')
        {
            digit = hex[i] - 'A' + 10;
        }
        else if ( hex[i]>='a' && hex[i] <= 'f')
        {
            digit = hex[i] - 'a' + 10;
        }
        else
        {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

int rild_sms_queryCBInfo(int *mode, char **ch_str, char **dcs_str, int *alllanguageon, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    char *line, *chIDs, *dcsIDs;

    *ch_str = *dcs_str = NULL;

    err = at_send_command_singleline("AT+CSCB?", "+CSCB:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0)
        goto error;

    line = p_response->p_intermediates->line;
    
    err = at_tok_start(&line);
    if(err < 0) goto error;   

    // get mode
    err = at_tok_nextint(&line, mode);
    if(err < 0 || (*mode) < 0 || (*mode) > 1) goto error;

    // get <mids> ID string
    err = at_tok_nextstr(&line, &chIDs);
    if(err < 0) goto error;
    asprintf(ch_str, "%s", chIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str 
    * after using this function
    *******************************************/

    // get <dcss> ID string
    err = at_tok_nextstr(&line, &dcsIDs);
    if(err < 0) goto error;
    asprintf(dcs_str, "%s", dcsIDs);

    /*******************************************
    * Please remember to free the buffer ch_str and dcs_str 
    * after using this function
    *******************************************/

	// get alllanguageon
	err = at_tok_nextint(&line, alllanguageon);
	if(err < 0)
		*alllanguageon = 0;

    at_response_free(p_response);
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

int rild_sms_getIDTable(char *table, char *id_string, int maximum_id)
{
    int len, i, j;
    int value, begin;

    memset(table, 0x00, maximum_id);
    len = strlen(id_string);

    if (len == 0)    
        return 0;
    
    for(i = 0, begin = -1, value = 0; i <= len; i++, id_string++)
    {
        //LOGD("%s", id_string);
        if ( (*id_string) == ' ' )
            continue;
        else if ( (*id_string) == ',' || (*id_string == '\0'))
        {
            if (begin == -1)
            {
                /* %d, */
                begin = value;                    
            }
            else
            {                
                /* %d-%d,*/
                begin = value + 1; /* ignore MIR value*/
            }
            
            for (j = begin; j <= value; j++)
            {
                if (j < maximum_id) {
                    table[j] = 1;
                    //LOGD("Set table valud %d", j);
                }
                else {
                    ;/* ignore */
                }
            }
            
            begin = -1;
            value = 0;
        }
        else if ( (*id_string) == '-' )
        {
            begin = value;
            value = 0;
        }
        else if ( (*id_string) >= '0' && (*id_string) <= '9' )
        {
            value = value*10 + (*id_string) - '0';
        }
        else
        {
            LOGE("There is some invalid characters: %x", (*id_string) );
            return -1;
        }
    }

    return 0;
}

static int  rild_sms_packPDU(const char* smsc, const char *tpdu, char *pdu)
{
    int len=0;
    
    //Check SMSC
    if (smsc == NULL) 
    {
        // "NULL for default SMSC"
        smsc= "00";
    }
    else
    {        
        len = strlen(smsc);
        if ( (len%2) != 0 || len<2 || (len/2-1) > MAX_SMSC_LENGTH)
        {
            LOGE("The length of smsc is not valid: %d", len);
            return -1;
        }
    }

    // Check PDU
    if (tpdu == NULL)
    {      
        LOGE("PDU should not be NULL");
        return -1;
    }
    len = strlen(tpdu);
    if ( (len%2) != 0)
    {
        LOGE("The length of TPDU is not valid: %d", len);
        return -1;
    }
    len /= 2;
    if ( len > MAX_TPDU_LENGTH) 
    {
        LOGE("The length of TPDU is too long: %d", len);
        return -1;
    }

    sprintf(pdu, "%s%s", smsc, tpdu);

    return len;
}

static int rild_sms_checkReceivedPDU(int length, char* pdu)
{
    int pdu_length;
    int sca_length;

    pdu_length = strlen(pdu);
    if (pdu_length < 2)
    {
        LOGW("The PDU structure might be wrong");
        return -1;
    }

    sca_length = rild_sms_hexCharToDecInt(pdu, 2);
    if (sca_length < 0)
    {
        LOGW("The PDU structure might be wrong with invalid digits: %d", sca_length);
        return -1;
    }

    pdu_length -= (sca_length + 1)*2;
    if (pdu_length != length*2)
    {
        LOGW("The specified TPDU length is not matched to the TPDU data:  %d", length);
        return -1;
    }

    return 0;
}

static void rild_sms_sendSMS(int request, void * data, size_t datalen, RIL_Token t)
{
    int err, len=0, cause;
    const char *smsc;
    const char *pdu;
    char *cmd1, *line;
    char buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0
    RIL_SMS_Response response;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    smsc = ((const char **)data)[0];
    pdu = ((const char **)data)[1];

    /* fill error code first */
    response.errorCode = ril_errno;

    len = rild_sms_packPDU(smsc, pdu, buffer);
    if(len < 0) goto error;
    
    /********************************************
    * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
    * We should send AT+CMMS=1 to enable keeping relay protocol link
    *********************************************/
    if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE)
    {
        err = at_send_command("AT+CMMS=1", &p_response, SMS_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0) 
        {
            LOGE("Error occurs while executing AT+CMMS=1");
            goto error;
        }
        at_response_free(p_response);
    }

    asprintf(&cmd1, "AT+EMGS=%d, \"%s\"", len, buffer);    
    err = at_send_command_singleline(cmd1, "+CMGS:", &p_response, SMS_CHANNEL_CTX);
    free(cmd1);
    if (err < 0) {
        LOGE("Error occurs while executing AT+EMGS=%d, \"%s\"", len, buffer);
        goto error;
    }

    if (p_response->success == 0) {
        /****************************************
        * FIXME
        * Need to check what cause should be retry latter.
        ****************************************/
        cause = at_get_cms_error(p_response);
        if (cause == CMS_MTK_FDN_CHECK_FAILURE)
        {
            ril_errno = RIL_E_FDN_CHECK_FAILURE;
        }
        else if (!(cause == CMS_CM_UNASSIGNED_NUM ||
            cause == CMS_CM_OPR_DTR_BARRING ||
            cause == CMS_CM_CALL_BARRED ||
            cause == CMS_CM_CALL_REJECTED ||
            cause == CMS_CM_DEST_OUT_OF_ORDER ||
            cause == CMS_CM_INVALID_NUMBER_FORMAT ||
            cause == CMS_CM_FACILITY_REJECT ||
            cause == CMS_CM_RES_STATUS_ENQ ||
            cause == CMS_CM_NETWORK_OUT_OF_ORDER ||
            cause == CMS_CM_REQ_FAC_NOT_SUBS ||
            cause == CMS_CM_REQ_FACILITY_UNAVAIL ||
            cause == CMS_CM_INVALID_TI_VALUE ||
            cause == CMS_CM_SEMANTIC_ERR ||
            cause == CMS_CM_MSG_TYPE_UNIMPL ||
            cause == CMS_CM_IE_NON_EX ||
            cause == CMS_CM_MM_IMSI_UNKNOWN_IN_HLR ||
            cause == CMS_CM_MM_ILLEGAL_MS ||
            cause == CMS_CM_MM_ILLEGAL_ME ||
            cause == CMS_CM_RR_PLMN_SRCH_REJ_EMERGENCY ||
            cause == CMS_CM_MM_AUTH_FAILURE ||
            cause == CMS_CM_MM_IMSI_DETACH ||
            cause == CMS_CM_MM_EMERGENCY_NOT_ALLOWED ||
            cause == CMS_CM_MM_ACCESS_CLASS_BARRED ||
            cause == SMS_MO_SMS_NOT_ALLOW) )
        {
            ril_errno = RIL_E_SMS_SEND_FAIL_RETRY;
        }

        /********************************************
        * free because it needs to send another AT
        * commands again
        ********************************************/
        at_response_free(p_response);
        p_response = NULL;
        /********************************************
        * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
        * We should send AT+CMMS=0 to disable keeping relay protocol link
        *********************************************/
        if (request == RIL_REQUEST_SEND_SMS_EXPECT_MORE)
        {
            err = at_send_command("AT+CMMS=0", &p_response, SMS_CHANNEL_CTX);            
            if (err != 0 || p_response->success == 0) 
            {
                LOGW("Warning: error occurs while executing AT+CMMS=0");                
            }
        }

        response.errorCode = ril_errno;

        goto error;
    }
    
    // Get message reference and ackPDU
    line = p_response->p_intermediates->line;   
    err = at_tok_start(&line);

    err = at_tok_nextint(&line, &(response.messageRef));
    if (err < 0) goto error;

    if ( at_tok_hasmore(&line) )
    {
        err = at_tok_nextstr(&line, &(response.ackPDU));
        if (err < 0) goto error;
    }
    else
    {
        response.ackPDU = NULL;
    }

    /* fill success code */
    response.errorCode = RIL_E_SUCCESS;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

void requestSendSMS(void * data, size_t datalen, RIL_Token t)
{
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS, data, datalen, t);
}

void requestSendSmsExpectMore(void * data, size_t datalen, RIL_Token t)
{
    rild_sms_sendSMS(RIL_REQUEST_SEND_SMS_EXPECT_MORE, data, datalen, t);
}

void requestSMSAcknowledge(void * data, size_t datalen, RIL_Token t)
{
    int ackSuccess, cause;
    int err;
    ATResponse *p_response = NULL;
    char *cmd;

    ackSuccess = ((int *)data)[0];
    cause = ((int *)data)[1] & 0xFF;

    if (ackSuccess == 1) 
    {
        err = at_send_command("AT+CNMA=1, 2, \"0000\"", &p_response, SMS_CHANNEL_CTX);
    } 
    else if (ackSuccess == 0)  
    {
        asprintf(&cmd, "AT+CNMA=2, 3, \"00%02X00\"", cause);
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
    } 
    else 
    {
        LOGE("unsupported arg to RIL_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response->success == 0)
        goto error;
    
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    return;
}

void requestWriteSmsToSim(void * data, size_t datalen, RIL_Token t)
{
    RIL_SMS_WriteArgs *p_args;
    char *cmd, buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0;
    int length, err, response[1], ret;
    ATResponse *p_response = NULL;
    char *line;

    p_args = (RIL_SMS_WriteArgs *)data;

    //check stat
    if (p_args->status < RIL_SMS_REC_UNREAD || p_args->status >= RIL_SMS_MESSAGE_MAX)
    {
        LOGE("The status is invalid: %d", p_args->status);
        goto error;
    }

    // pack PDU with SMSC
    length = rild_sms_packPDU(p_args->smsc, p_args->pdu, buffer);
    if (length < 0) goto error;

    asprintf(&cmd, "AT+EMGW=%d,%d, \"%s\"", length, p_args->status, buffer);
    err = at_send_command_singleline(cmd, "+CMGW:", &p_response, SMS_CHANNEL_CTX);
    free(cmd);

    ret = at_get_cms_error(p_response);
    if (err < 0 || p_response->success == 0) goto error;

    // get +CMGW: <index>
    line = p_response->p_intermediates->line;
    
    err = at_tok_start( &line);
    if( err < 0) goto error;

    err = at_tok_nextint(&line, &response[0]);
    if( err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    if(CMS_CM_SIM_IS_FULL == ret) {
        RIL_onRequestComplete(t, RIL_E_SIM_MEM_FULL, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
}

void requestDeleteSmsOnSim(void *data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err, index;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    index = ((int *)data)[0];

    if(index > 0)
    {
        asprintf(&cmd, "AT+CMGD=%d", index);
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
        
        if ( !(err < 0 || p_response->success == 0) ) 
        {
            ril_errno = RIL_E_SUCCESS;
        }    
        at_response_free(p_response);
    } else if(-1 == index) {
        // delete all sms on SIM
        asprintf(&cmd, "AT+CMGD=0,4");
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
        free(cmd);
        
        if ( !(err < 0 || p_response->success == 0) ) 
        {
            ril_errno = RIL_E_SUCCESS;
        }    
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
}

void requestGSMGetBroadcastSMSConfig(void *data, size_t datalen, RIL_Token t)
{
    char *line, *ch_string, *dcs_string;
    ATResponse *p_response = NULL;
    int err, mode, count, ch_cur, dcs_cur, i, alllanguageon;
    int ch_begin, ch_end, dcs_begin, dcs_end;
    RIL_GSM_BroadcastSmsConfigInfo *info;
    RIL_GSM_BroadcastSmsConfigInfo **infos;
    RIL_SMS_Linked_list *node, head;
    char chID_table[MAX_CB_CHANNEL_ID];
    char dcsID_table[MAX_CB_DCS_ID];

     // inital value
    head.next = NULL;
    head.data = NULL;
    node = &head;
    count = 0;

    err = rild_sms_queryCBInfo(&mode, &ch_string, &dcs_string, &alllanguageon, t);
    if (err < 0) goto error;

    if (mode == 1) {        
        node->next = malloc(sizeof(RIL_SMS_Linked_list));
        node = node->next;
        
        node->data = malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
        info = node->data;
        info->fromServiceId= -1;
        info->toServiceId= -1;
        info->fromCodeScheme = -1;
        info->toCodeScheme = -1;
        info->selected= 0;

        count++;    
    } else if (mode == 0) {
  		if(alllanguageon == 1) {
			LOGD("Here we receive alllanguage == 1");
			// get the id table
	        err = rild_sms_getIDTable(chID_table, ch_string, MAX_CB_CHANNEL_ID);
	        if(err < 0) goto error;

			ch_cur = 0;
			do {
				while(ch_cur < MAX_CB_CHANNEL_ID && chID_table[ch_cur] == 0) {
					ch_cur++;
				}
				ch_begin = (ch_cur == MAX_CB_CHANNEL_ID) ? -1 : ch_cur;

				if(ch_begin != -1) {
					while(ch_cur < MAX_CB_CHANNEL_ID && chID_table[ch_cur] == 1) {
	                    ch_cur++;
					}
	                ch_end = (chID_table[ch_cur-1] != 1 ) ? -1 : ch_cur-1;
				} else {
					ch_end = -1;
				}
				LOGD("ch: %d %d %d", ch_begin, ch_end, ch_cur);

				//create a structure of RIL_GSM_BroadcastSmsConfigInfo
	            if ( ch_begin == -1 && count > 0) {
	                break;
	            } else {
	                node->next = malloc(sizeof(RIL_SMS_Linked_list));
	                node = node->next;

	                node->data = malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
	                info = node->data;
	                info->fromServiceId= ch_begin;
	                info->toServiceId= ch_end;
	                info->fromCodeScheme = -2;
	                info->toCodeScheme = -2;
	                info->selected= 1;

	                count++;
	            }
			}while(1);
		} else { 
        	// get the id table
        	err = rild_sms_getIDTable(chID_table, ch_string, MAX_CB_CHANNEL_ID);
        	if(err < 0) goto error;

        	// get the id table
        	err = rild_sms_getIDTable(dcsID_table, dcs_string, MAX_CB_DCS_ID);
        	if(err < 0) goto error;

        	// get combination of chID and dcsID
        	ch_cur = dcs_cur = 0;
        	do {
            	//find a range of <mid>
            	while(ch_cur < MAX_CB_CHANNEL_ID && chID_table[ch_cur] == 0) {
                	ch_cur++;
            	}
            	ch_begin = (ch_cur == MAX_CB_CHANNEL_ID) ? -1 : ch_cur;

            	if (ch_begin != -1) {
                	while(ch_cur < MAX_CB_CHANNEL_ID && chID_table[ch_cur] == 1) {
                    	ch_cur++;
                	}
                	ch_end = (chID_table[ch_cur-1] != 1 ) ? -1 : ch_cur-1;
            	} else {
                	ch_end = -1;
            	}
            	LOGD("ch: %d %d %d", ch_begin, ch_end, ch_cur);
            
            	//find a range of <dcs>
            	while(dcs_cur < MAX_CB_DCS_ID && dcsID_table[dcs_cur] == 0) {
                	dcs_cur++;
            	}
            	dcs_begin = (dcs_cur == MAX_CB_DCS_ID) ? -1 : dcs_cur;

            	if (dcs_begin != -1) {
                	while(dcs_cur < MAX_CB_DCS_ID && dcsID_table[dcs_cur] == 1) {
                    	dcs_cur++;
                	}
                	dcs_end = (dcsID_table[dcs_cur-1] != 1 ) ? -1 : dcs_cur-1;
            	} else {
                	dcs_end = -1;
            	}
            	LOGD("dcs: %d %d %d", dcs_begin, dcs_end, dcs_cur);

            	//create a structure of RIL_GSM_BroadcastSmsConfigInfo
            	if ( ch_begin == -1 && dcs_begin == -1 && count > 0) {
                	break;
            	} else {
	                node->next = malloc(sizeof(RIL_SMS_Linked_list));
	                node = node->next;

	                node->data = malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo));
	                info = node->data;
	                info->fromServiceId= ch_begin;
	                info->toServiceId= ch_end;
	                info->fromCodeScheme = dcs_begin;
	                info->toCodeScheme = dcs_end;
	                info->selected= 1;

	                count++;    
            	}
        	}while(1);
    	}
    } else {
        goto error;
    }

    // pack the output data
    node = head.next;
    infos = malloc(sizeof(RIL_GSM_BroadcastSmsConfigInfo*)*count);
    for (i = 0; i < count ; i++)
    {
        infos[i] = node->data;
        node = node->next;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, infos, sizeof(RIL_GSM_BroadcastSmsConfigInfo*)*count);
    at_response_free(p_response);
    
    // free the output data
    for (i = 0; i < count ; i++)
    {
        free(infos[i]);
    }
    free(infos);

    free(ch_string);
    free(dcs_string);
    return;

error:
    if (ch_string != NULL)
        free(ch_string);
    if (dcs_string != NULL)
        free(dcs_string);
    
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGSMSetBroadcastSMSConfig(void *data, size_t datalen, RIL_Token t)
{
    char *line, *ch_string, *dcs_string;
    char *ch_new_str, *dcs_new_str;
    ATResponse *p_response = NULL;
    int err, mode, count, i, j, alllanguageon;
    int ch_begin, ch_end, dcs_begin, dcs_end;
    
    RIL_GSM_BroadcastSmsConfigInfo *info;
    RIL_GSM_BroadcastSmsConfigInfo **infos;
    char chID_table[MAX_CB_CHANNEL_ID];
    char dcsID_table[MAX_CB_DCS_ID];

     // inital # of RIL_GSM_BroadcastSmsConfigInfo
    count = datalen / sizeof(RIL_GSM_BroadcastSmsConfigInfo*);
    infos = (RIL_GSM_BroadcastSmsConfigInfo **) data;

    ch_new_str = malloc(sizeof(char)*1000);
    dcs_new_str = malloc(sizeof(char)*1000);

    err = rild_sms_queryCBInfo(&mode, &ch_string, &dcs_string, &alllanguageon, t);
    if (err < 0) goto error;

    if (mode == 1)
    {
        LOGE("Should not set CB info when CB is disabled");
        goto error;
    }

	if(infos[0]->fromServiceId == -1 && infos[0]->toServiceId == -1 && 
		infos[0]->fromCodeScheme == -2 && infos[0]->toCodeScheme == -2 && infos[0]->selected == 1)
	{
		// set the new setting
		LOGD("Set Channel and Dcs settings : AT+CSCB=0, \"\", \"\", 1");
		//asprintf(&line, "AT+CSCB=0, \"%s\", \"\", 1", ch_string);
		asprintf(&line, "AT+CSCB=0, \"\", \"\", 1");
		err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
		free(line);
		if(err < 0 || p_response->success == 0) {
			LOGE("Fail to set all language on");
			goto error;
		}

	    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	    at_response_free(p_response);
	    free(ch_string);
	    free(dcs_string);
	    free(ch_new_str);
	    free(dcs_new_str); 
	    return;
	}
    //get current setting
    // get the id table
    err = rild_sms_getIDTable(chID_table, ch_string, MAX_CB_CHANNEL_ID);
    if(err < 0) goto error;

    // get the id table
    err = rild_sms_getIDTable(dcsID_table, dcs_string, MAX_CB_DCS_ID);
    if(err < 0) goto error;

    //combine the current setting and input setting
    LOGD("Get Current Setting %d", count);
    for (i = 0; i < count; i++)
    {
        ch_begin = dcs_begin = 0;
        ch_end = dcs_end = -1;

        LOGD("Get Current Setting: %d", i);
        
        if (infos[i]->selected != 1 && infos[i]->selected != 0)
        {
            continue;
        }

        // get channel ID range
        if(infos[i]->fromServiceId< MAX_CB_CHANNEL_ID 
            && infos[i]->fromServiceId >= 0)
        {
            ch_begin = infos[i]->fromServiceId;
        }
        if(infos[i]->toServiceId < MAX_CB_CHANNEL_ID 
            && infos[i]->toServiceId >= 0)
        {
            ch_end = infos[i]->toServiceId;
        }
        // get DCS ID range
        if(infos[i]->fromCodeScheme < MAX_CB_DCS_ID 
            && infos[i]->fromCodeScheme >= 0)
        {
            dcs_begin = infos[i]->fromCodeScheme;
        }
        if(infos[i]->toCodeScheme < MAX_CB_DCS_ID 
            && infos[i]->toCodeScheme >= 0)
        {
            dcs_end = infos[i]->toCodeScheme;
        }

        LOGD("Get Current Setting: %d %d %d %d", ch_begin, ch_end, dcs_begin, dcs_end);

        // Set channel ID into the table
        for (j = ch_begin; j <= ch_end; j++)
        {
            chID_table[j] = infos[i]->selected;
        }
        
        // Set dcs ID into the table
        for (j = dcs_begin; j <= dcs_end; j++)
        {
            dcsID_table[j] = infos[i]->selected;
        }
    }

	// reset the dcs setting
    asprintf(&line, "AT+CSCB=1, \"%s\", \"%s\"", ch_string, dcs_string);
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response->success == 0)
        goto error;

    at_response_free(p_response);
    p_response = NULL;

    // construct the setting string
    // set the new dcs setting
    dcs_new_str[0] = '\0';
	// count how many language need to be enabled
	count = 0;
    for(i = 0 ; i< MAX_CB_DCS_ID;i++)
    {
        if (dcsID_table[i]>0)
        {
        	if ((i>=0 && i<=15) || (i>=32 && i<=36)) {
				count++;
        	}
			
            for (j=i+1; j < MAX_CB_DCS_ID; j++)
            {    
                if (dcsID_table[j] == 0)
                {
                    break;
                }
				else
				{
					if ((j>=0 && j<=15) || (j>=32 && j<=36)) {
						count++;
					}
				}
            }

            if (j == (i+1))
            {
                sprintf(dcs_new_str, "%s%d,", dcs_new_str, i);
            }
            else
            {
                sprintf(dcs_new_str, "%s%d-%d,",dcs_new_str, i, j-1);
            }
            i = j;
        }
    }    
    if (strlen(dcs_new_str) > 0)
    {
        dcs_new_str[strlen(dcs_new_str)-1] = '\0';
    }
    else
    {
        LOGW("Didn't select any DCS");
        //goto error;
    }
    LOGD("New dcs Setting: %s %d", dcs_new_str, count);

    // count == 21 indicates that all language is on, or is off
	if (count == 21) {
		alllanguageon = 1;
	} else {
		alllanguageon = 0;
	}

    // set the new channel setting
    ch_new_str[0] = '\0';
    for(i = 0 ; i< MAX_CB_CHANNEL_ID;i++)
    {
        
        if (chID_table[i]>0)
        {
            //LOGD("ch %d, %d", i, chID_table[i]);

            #if 0
            for (j=i+1; j < MAX_CB_CHANNEL_ID; j++)
            {
                //LOGD("ch %d, %d", j, chID_table[j]);
                if (chID_table[j] == 0)
                {
                    break;
                }
            }

            if (j == (i+1))
            {
                sprintf(ch_new_str, "%s%d,", ch_new_str, i);
            }
            else
            {
                sprintf(ch_new_str, "%s%d-%d,",ch_new_str, i, j-1);
            }
            i = j;
            #else
            sprintf(ch_new_str, "%s%d,", ch_new_str, i);
            #endif
        }
    }    
    ch_new_str[strlen(ch_new_str)-1] = '\0';

    LOGD("New ch Setting: %s", ch_new_str);

    // this reset action is unnecessary, remove it
    // reset the dcs setting
    //asprintf(&line, "AT+CSCB=1, \"%s\", \"%s\"", ch_string, dcs_string);
    //err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    //free(line);
    //if (err < 0 || p_response->success == 0)
    //    goto error;

    // set the new setting
    asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", ch_new_str, dcs_new_str);
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);
    if (err < 0 || p_response->success == 0)
    {
        /********************************************
        * free because it needs to send another AT
        * commands again
        ********************************************/
        at_response_free(p_response);
        p_response = NULL;

        asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", ch_string, dcs_string);    
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);    
        if (err < 0 || p_response->success == 0)
        {
            LOGE("Restore CB Setting ERROR");
        }
        goto error;
    }

    /********************************************
    * free because it needs to send another AT
    * commands again
    ********************************************/
    at_response_free(p_response);
    p_response = NULL;

	// if all lang on is true, we should re-set all lang on by send AT+CSCB=0,"","",1
	// because new setting action will disable previous setting
	if(alllanguageon) {
		LOGD("re-set all language on, because we change cid");
		asprintf(&line, "AT+CSCB=0, \"\", \"\", 1");
		err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
		free(line);
		if(err < 0 || p_response->success == 0) {
			LOGE("Fail to re-set all language on");
			goto error;
		}
	}

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    free(ch_string);
    free(dcs_string);
    free(ch_new_str);
    free(dcs_new_str); 
    return;
    
error:
    if (ch_string != NULL)
        free(ch_string);
    if (dcs_string != NULL)
        free(dcs_string);
    free(ch_new_str);
    free(dcs_new_str); 
    
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestGSMSMSBroadcastActivation(void *data, size_t datalen, RIL_Token t)
{
    int activation;
    int err;
    char *line;
    int mode, allLanguageOn;
    char *ch_string, *dcs_string;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    activation = ((int *)data)[0];
    LOGD("activation: %d", activation);

    // asprintf(&line, "AT+CSCB=%d", activation);
    if(1 == activation) {
        asprintf(&line, "AT+CSCB=1");
    } else if(0 == activation){
        LOGD("query previous config info");
        err = rild_sms_queryCBInfo(&mode, &ch_string, &dcs_string, &allLanguageOn, t);
        if (err < 0) goto error;
        LOGD("Current setting: %s, %s", ch_string, dcs_string);
            
        asprintf(&line, "AT+CSCB=0, \"%s\", \"%s\"", ch_string, dcs_string);
        if(NULL != ch_string) {
            free(ch_string);
        }
        if(NULL != dcs_string) {
            free(dcs_string);
        }
    } else {
        LOGD("unhandled mode fot AT+CSCB");
        ril_errno = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
    free(line);    
    if (err < 0 || p_response->success <= 0) 
        ril_errno =RIL_E_GENERIC_FAILURE;
    
    RIL_onRequestComplete(t, ril_errno, NULL, 0);    
    at_response_free(p_response);
    
    return;
    
error:
    if(NULL != ch_string) {
        free(ch_string);
    }
    if(NULL != dcs_string) {
        free(dcs_string);
    }
    
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestGetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{    
    int err, tosca;
    char *line, *sca;
    ATResponse *p_response = NULL;

    err = at_send_command_singleline("AT+CSCA?", "+CSCA:", &p_response, SMS_CHANNEL_CTX);
    if (err < 0 || p_response->success <= 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    // get <SCA>
    err = at_tok_nextstr(&line, &sca);
    if (err < 0) goto error;

    // get <TOSCA>
    err = at_tok_nextint(&line, &tosca);
    if (err < 0) goto error;

    if (tosca == 0x91 && sca[0] != '+')
    {
        // add '+' in front of the sca
        sca--;
        sca[0] = '+';
    }
    
    RIL_onRequestComplete(t, RIL_E_SUCCESS, sca, sizeof(char*));    
    at_response_free(p_response);
    return;

error:          
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSetSMSCAddress(void *data, size_t datalen, RIL_Token t)
{
    int err, tosca = 0x81, len;
    char *line, *sca;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    sca = (char*) data;    

    // get <tosca>
    if (sca[0] == '+')
    {
        tosca = 0x91;
        sca++;
    }
    
    len = strlen(sca);
    if (len > (MAX_SMSC_LENGTH-1)*2)
    {
        LOGE("The input length of the SCA is too long: %d", len);
       
    }
    else
    {
        asprintf(&line, "AT+CSCA=\"%s\", %d", sca, tosca);
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);
        if ( !(err < 0 || p_response->success <= 0) )
        {
            /********************************************
            * free because it needs to send another AT
            * commands again
            ********************************************/
            at_response_free(p_response);
            p_response = NULL;
            // ril_errno = RIL_E_SUCCESS;
            asprintf(&line, "AT+CSAS");
			err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
			free(line);
			if ( !(err < 0 || p_response->success <= 0) ) {
			    ril_errno = RIL_E_SUCCESS;
			}
        }
    }

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);    
}

void onNewSms(const char *urc, const char *smspdu, RILId rid)
{
    int length;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    // skip <alpha>
    err = at_tok_nextint(&pUrc, &length);

    // get <length>
    err = at_tok_nextint(&pUrc, &length);
    
    rild_sms_checkReceivedPDU(length,(char *) smspdu);

    RIL_onUnsolicitedResponse (
        RIL_UNSOL_RESPONSE_NEW_SMS, 
        smspdu, sizeof(char*), 
        rid);
}

void onNewSmsStatusReport(const char *urc, const char *smspdu, RILId rid)
{
    int length;
    int err;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    rild_sms_checkReceivedPDU(length,(char *) smspdu);
    
    RIL_onUnsolicitedResponse (
        RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,
        smspdu, sizeof(char*),
        rid);
}

void onNewSmsOnSim(const char *urc, RILId rid)
{
    int err, index[1];
    char *mem;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);
    if (err < 0) goto error;

    err = at_tok_nextstr(&pUrc, &mem);
    if (err < 0) goto error;

    if (strncmp(mem, "SM", 2) != 0)
    {
        LOGE("The specified storage is not SIM storage");
        return;
    }

    err = at_tok_nextint(&pUrc, &index[0]);
    if (err < 0) goto error;

    RIL_onUnsolicitedResponse (
        RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM,
        index, sizeof(index),
        rid);

error:
    LOGE("There is something wrong with the input URC");
}

void onSimSmsStorageStatus(const char *line, RILId rid)
{
    int err;
    int status, ciev_id;
    int urc_id = -1;
    char *pUrc = (char *)line;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &ciev_id);
    if (err < 0) return;    

    err = at_tok_nextint(&pUrc, &status);
    if (err < 0) return;

    if (err < 0)
    {
        LOGE("There is something wrong with the URC: +CIEV:7, <status>");
    }
    else
    {
        switch(status)
        {
            case 0: // available
                break;
            case 1: // full
            case 2: // SIM exceed
                /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                break;
            case 3: // ME exceed
                LOGD("Phone storage status: %d", phone_storage_status);
                if (phone_storage_status == 1)
                {
                    /*********************************************************
                     * It is a workaround solution here.
                     * for mem1, mem2 and mem3, even if all are SIM card 
                     * storage due to set as AT+CPMS="SM", "SM", "SM", we still
                     * need to check the phone_storage_status.
                     * It is the current limitation on modem, it should 
                     * upgrate the +CIEV for better usage.
                     ********************************************************/
                    urc_id = RIL_UNSOL_ME_SMS_STORAGE_FULL;
                }
                else
                {
                    /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                    urc_id = RIL_UNSOL_SIM_SMS_STORAGE_FULL;
                }
                break;
            default:
                LOGW("The received <status> is invalid: %d", status);
                break;
        }
        if (urc_id >= 0)
        {
            RIL_onUnsolicitedResponse (urc_id, NULL, 0, rid);
        }
    }
}

void onNewBroadcastSms(const char *urc, const char *smspdu, RILId rid)
{
    int length;
    int err;
    char *line;
    char *pUrc = (char *)urc;

    err = at_tok_start(&pUrc);

    err = at_tok_nextint(&pUrc, &length);

    asprintf(&line, "00%s", smspdu);
    rild_sms_checkReceivedPDU(length, line);
    free(line);
    
    RIL_onUnsolicitedResponse (
        RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS ,
        smspdu, sizeof(char*),
        rid);

}

void onSmsReady(void *param)
{
    RILId rid = *((RILId *)param);
    
    LOGD("On Sms Ready rid: %d", rid);
    
    /* 
     * If modem turns off, RILD should not send any AT command to modem.
     * If RILD sends the at commands to Modem(off), it will block the 
     * MUXD/CCCI channels
     */
    if (s_md_off)
    {
        LOGD("Modem turns off");
        return;
    }
    LOGD("Modem turns on");

    RIL_onUnsolicitedResponse(
                RIL_UNSOL_SMS_READY_NOTIFICATION, 
                NULL, 0, rid);
    
    /*
     * Always send SMS messages directly to the TE
     *
     * mode = 1 // discard when link is reserved (link should never be
     *             reserved)
     * mt = 2   // most messages routed to TE
     * bm = 2   // new cell BM's routed to TE
     * ds = 1   // Status reports routed to TE
     * bfr = 1  // flush buffer
     */
    at_send_command_singleline("AT+CSMS=1", "+CSMS:", NULL,getChannelCtxbyProxy(rid));
    
    at_send_command("AT+CNMI=1,2,2,1,1", NULL,getChannelCtxbyProxy(rid));

    at_send_command("AT+CPMS=\"SM\", \"SM\", \"SM\"", NULL, getChannelCtxbyProxy(rid));
}

extern int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_SEND_SMS:
            requestSendSMS(data, datalen, t);
            break;
        case RIL_REQUEST_SMS_ACKNOWLEDGE:
            requestSMSAcknowledge(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_SMS_TO_SIM:
            requestWriteSmsToSim(data, datalen, t);
            break;
        case RIL_REQUEST_DELETE_SMS_ON_SIM: 
            requestDeleteSmsOnSim(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
            requestSendSmsExpectMore(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            requestGSMGetBroadcastSMSConfigEx(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
            //for channel over 1000 issue
            requestGSMSetBroadcastSMSConfigEx(data, datalen, t);
            break;
        case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
            requestGSMSMSBroadcastActivation(data, datalen, t);
            break;
        case RIL_REQUEST_GET_SMSC_ADDRESS:
            requestGetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_SET_SMSC_ADDRESS:
            requestSetSMSCAddress(data, datalen, t);
            break;
        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
            requestReportSMSMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS:
            requestGetSmsSimMemoryStatus(data, datalen, t);
            break;
        
        case RIL_REQUEST_GET_SMS_PARAMETERS:
            requestGetSmsParams(data, datalen, t);
            break;
            
        case RIL_REQUEST_SET_SMS_PARAMETERS:
            requestSetSmsParams(data, datalen, t);
            break;
            
        case RIL_REQUEST_SET_ETWS:
            requestSetEtws(data, datalen, t);
            break;
        
        case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO:
            requestSetCbChannelConfigInfo(data, datalen, t);
            break;
            
        case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO:
            requestSetCbLanguageConfigInfo(data, datalen, t);
            break;
        
        case RIL_REQUEST_GET_CB_CONFIG_INFO:
            requestGetCellBroadcastConfigInfo(data, datalen, t);
            break;
        
        case RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON:
            requestSetAllCbLanguageOn(data, datalen, t);
            break;
        
        default:
            return 0; /* no matched request */
        break;
     }

    return 1; /* request found and handled */

}

extern int rilSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RILId rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+CIEV: 7"))
    {
        onSimSmsStorageStatus(s,rid);
    } 
    else if (strStartsWith(s, "+CMT:")) 
    {
        onNewSms(s, sms_pdu,rid);
    } 
    else if (strStartsWith(s, "+CMTI:")) 
    {
        onNewSmsOnSim(s,rid);
    } 
    else if (strStartsWith(s, "+CDS:")) 
    {
        onNewSmsStatusReport(s, sms_pdu,rid);        
    } 
    else if (strStartsWith(s, "+CBM:")) 
    {
        onNewBroadcastSms(s, sms_pdu,rid);        
    } 
    else if (strStartsWith(s, "+EIND: 1"))
    {
        if (rid == MTK_RIL_SOCKET_1)
        {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId, &TIMEVAL_0,
            		getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        }
    #ifdef MTK_GEMINI
        else if (rid == MTK_RIL_SOCKET_2)
        {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId2, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        }
    #ifdef MTK_GEMINI_3SIM_SUPPORT
        else if (rid == MTK_RIL_SOCKET_3)
        {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId3, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        }
    #ifdef MTK_GEMINI_4SIM_SUPPORT
        else if (rid == MTK_RIL_SOCKET_4)
        {
            RIL_requestProxyTimedCallback(onSmsReady, &s_pollSimId4, &TIMEVAL_0,
                    getRILChannelCtx(RIL_SMS, rid)->id, "onSmsReady");
        }
    #endif /* MTK_GEMINI_4SIM_SUPPORT */
    #endif /* MTK_GEMINI_3SIM_SUPPORT */
    #endif /* MTK_GEMINI */
    }
    else if (strStartsWith(s, "+ETWS:"))
    {
        onNewEtwsNotification(s, rid);
    }
    else
    {
        return 0;
    }

    return 1;
}


