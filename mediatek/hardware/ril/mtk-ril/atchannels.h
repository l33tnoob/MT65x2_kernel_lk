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

/* //$(MTK_PATH_SOURCE)/hardware/ril/mtk-ril/atchannels.h
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

#ifndef ATCHANNELS_H
#define ATCHANNELS_H 1

#ifdef MTK_RIL

#ifdef __cplusplus
extern "C" {
#endif

/* define AT_DEBUG to send AT traffic to /tmp/radio-at.log" */
#define AT_DEBUG  1
extern int mtk_ril_log_level;

#if AT_DEBUG
extern void  AT_DUMP(const char* prefix, const char*  buff, int  len);
#else
#define  AT_DUMP(prefix,buff,len)  do{}while(0)
#endif

#define AT_ERROR_GENERIC -1
#define AT_ERROR_COMMAND_PENDING -2
#define AT_ERROR_CHANNEL_CLOSED -3
#define AT_ERROR_TIMEOUT -4
#define AT_ERROR_INVALID_THREAD -5 /* AT commands may not be issued from
                                       reader thread (or unsolicited response
                                       callback */
#define AT_ERROR_INVALID_RESPONSE -6 /* eg an at_send_command_singleline that
                                        did not get back an intermediate
                                        response */

#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))

#define MAX_AT_RESPONSE (8 * 1024)
#define HANDSHAKE_RETRY_COUNT 8
#define HANDSHAKE_TIMEOUT_MSEC 250

extern const struct RIL_Env *s_rilenv;
#define RIL_queryMyChannelId(a) s_rilenv->QueryMyChannelId(a)

typedef enum {
    NO_RESULT,   /* no intermediate response expected */
    NUMERIC,     /* a single intermediate response starting with a 0-9 */
    SINGLELINE,  /* a single intermediate response starting with a prefix */
    MULTILINE,    /* multiple line intermediate response
                    starting with a prefix */
    /* atci start */
    RAW
    /* atci end */
} ATCommandType;

/** a singly-lined list of intermediate responses */
typedef struct ATLine  {
    struct ATLine *p_next;
    char *line;
} ATLine;

/** Free this with at_response_free() */
typedef struct {
    int success;              /* true if final response indicates
                                    success (eg "OK") */
    char *finalResponse;      /* eg OK, ERROR */
    ATLine  *p_intermediates; /* any intermediate responses */
} ATResponse;

/**
 * a user-provided unsolicited response handler function
 * this will be called from the reader thread, so do not block
 * "s" is the line, and "sms_pdu" is either NULL or the PDU response
 * for multi-line TS 27.005 SMS PDU responses (eg +CMT:)
 */
typedef void (*ATUnsolHandler)(const char *s, const char *sms_pdu, void* p_channel);

typedef struct RILChannelCtx {
    const char* myName;
    RILChannelId id;
    pthread_t tid_reader;
    pthread_t tid_myProxy;
    int fd;
    ATUnsolHandler unsolHandler;
    int readCount;
    
    pthread_mutex_t commandmutex;
    pthread_cond_t commandcond;
    
    ATCommandType type;
    const char *responsePrefix;
    const char *smsPDU;
    ATResponse *p_response;
    
    int readerClosed;
    
    char *ATBufferCur;
    char ATBuffer[MAX_AT_RESPONSE+1];
    int pendingRequest;
} RILChannelCtx;


typedef struct RequestQueue {
	RILChannelCtx* requestchannel;
	struct RequestQueue* next ;
} RequestQueue;

void initRILChannels(void);

int at_open(int fd, ATUnsolHandler h, RILChannelCtx* p_channel);
void at_close(RILChannelCtx* p_channel);

/* This callback is invoked on the command thread.
   You should reset or handshake here to avoid getting out of sync */
void at_set_on_timeout(void (*onTimeout)(RILChannelCtx* p_channel));
/* This callback is invoked on the reader thread (like ATUnsolHandler)
   when the input stream closes before you call at_close
   (not when you call at_close())
   You should still call at_close()
   It may also be invoked immediately from the current thread if the read
   channel is already closed */
void at_set_on_reader_closed(void (*onClose)(RILChannelCtx* p_channel));

int at_send_command_singleline (const char *command,
                                const char *responsePrefix,
                                ATResponse **pp_outResponse,
                                RILChannelCtx* p_channel);

int at_send_command_numeric (const char *command,
                             ATResponse **pp_outResponse,
                             RILChannelCtx* p_channel);

int at_send_command_multiline (const char *command,
                               const char *responsePrefix,
                               ATResponse **pp_outResponse,
                               RILChannelCtx* p_channel);


int at_handshake(RILChannelCtx* p_channel);

int at_send_command (const char *command, ATResponse **pp_outResponse, RILChannelCtx* p_channel);

int at_send_command_sms (const char *command, const char *pdu,
                            const char *responsePrefix,
                            ATResponse **pp_outResponse,
                            RILChannelCtx* p_channel);

void at_response_free(ATResponse *p_response);

int at_send_command_raw (const char *command,
                         ATResponse **pp_outResponse,
                         RILChannelCtx * p_channel);


typedef enum {
    RIL_DEFAULT,
    RIL_SIM,
    RIL_STK,
    RIL_CC,
    RIL_SS,
    RIL_SMS,
    RIL_DATA,
    RIL_NW,
    RIL_OEM,
    RIL_SUPPORT_SUBSYSTEMS
} RILSubSystemId;


RILChannelCtx *getDefaultChannelCtx(RILId rid);
RILChannelCtx *getRILChannelCtxFromToken(RIL_Token t);
RILChannelCtx *getChannelCtxbyId(RILChannelId id);
RILChannelCtx *getRILChannelCtx(RILSubSystemId subsystem, RILId rid);
RILId getRILIdByChannelCtx(RILChannelCtx * p_channel);
RILChannelCtx *getChannelCtxbyProxy(RILId rid);

/* SYNC TO 27.007 section 9.2.1 */
typedef enum {
    CME_ERROR_NON_CME = -1,
    /* Note: this is the only difference from spec mapping. 0 is phone failure defined in 27.007 */    
    CME_SUCCESS = 0, 
    CME_NO_CONNECTION_TO_PHONE = 1,
    CME_PHONE_ADAPTOR_LINK_RESERVED = 2,
    CME_OPERATION_NOT_ALLOWED_ERR = 3,
    CME_OPERATION_NOT_SUPPORTED = 4,
    CME_PH_SIM_PIN_REQUIRED = 5,
    CME_PH_FSIM_PIN_REQUIRED = 6,
    CME_PH_FSIM_PUK_REQUIRED = 7,
    CME_OPR_DTR_BARRING = 8,
    CME_SIM_NOT_INSERTED = 10,
    CME_CALL_BARRED = CME_SIM_NOT_INSERTED, // overwrite CME: 10
    CME_SIM_PIN_REQUIRED = 11,
    CME_SIM_PUK_REQUIRED = 12,
    CME_SIM_FAILURE = 13,
    CME_SIM_BUSY = 14,
    CME_SIM_WRONG = 15,
    CME_INCORRECT_PASSWORD = 16,
    CME_SIM_PIN2_REQUIRED = 17,
    CME_SIM_PUK2_REQUIRED = 18,
    CME_MEMORY_FULL = 20,
    CME_INVALID_INDEX = 21,
    CME_NOT_FOUND = 22,
    CME_MEMORY_FAILURE = 23,
    CME_TEXT_STRING_TOO_LONG = 24,
    CME_INVALID_CHARACTERS_IN_TEXT_STRING = 25,
    CME_DIAL_STRING_TOO_LONG = 26,
    CME_INVALID_CHARACTERS_IN_DIAL_STRING = 27,
    CME_NO_NETWORK_SERVICE = 30,
    CME_NETWORK_TIMEOUT = 31,
    CME_NETWORK_NOT_ALLOWED = 32,
    CME_NETWORK_PERSONALIZATION_PIN_REQUIRED = 40,
    CME_NETWORK_PERSONALIZATION_PUK_REQUIRED = 41,
    CME_NETWORK_SUBSET_PERSONALIZATION_PIN_REQUIRED = 42,
    CME_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED = 43,
    CME_SERVICE_PROVIDER_PERSONALIZATION_PIN_REQUIRED = 44,
    CME_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED = 45,
    CME_CORPORATE_PERSONALIZATION_PIN_REQUIRED = 46,
    CME_CORPORATE_PERSONALIZATION_PUK_REQUIRED = 47,
    CME_HIDDEN_KEY_REQUIRED = 48,
    CME_UNKNOWN = 100,
    CME_LAST_PDN_NOT_ALLOW = 151, //(0x0097)
    CME_BT_SAP_UNDEFINED = 611,
    CME_BT_SAP_NOT_ACCESSIBLE = 612,
    CME_BT_SAP_CARD_REMOVED = 613,

    CME_MM_IMSI_UNKNOWN_IN_VLR = 2052,
    CME_MM_IMEI_NOT_ACCEPTED = 2053,
    CME_MM_ILLEGAL_ME = 2054,
    CME_PHB_FDN_BLOCKED = 2600,
    CME_UNKNOWN_PDP_ADDR_OR_TYPE = 3100,
    CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED = 4105 //(0x1009)

} AT_CME_Error;

AT_CME_Error at_get_cme_error(const ATResponse *p_response);
void sleepMsec(long long msec);

inline int isATCmdRspErr(int err, const ATResponse *p_response);

#ifdef __cplusplus
}
#endif

#endif /* MTK_RIL */

#endif /*ATCHANNELS_H*/

