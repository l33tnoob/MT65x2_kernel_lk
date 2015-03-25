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

/* //hardware/ril/reference-ril/ril_data.h
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

#ifndef RIL_DATA_H 
#define RIL_DATA_H 1

#define SM_CAUSE_BASE 3072 // Defined in AT_DOCUMENT_full.doc
#define SM_CAUSE_END SM_CAUSE_BASE + 0x72

#define ME_PDN_URC "ME PDN ACT"

#define AUTHTYPE_NONE       2
#define AUTHTYPE_PAP        0
#define AUTHTYPE_CHAP       1
#define AUTHTYPE_PAP_CHAP   3
#define AUTHTYPE_NOT_SET    (-1)

#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define INVALID_CID -1

#define IPV4        0
#define IPV6        1
#define IPV4V6      2

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define MAX_IPV4_ADDRESS 17
#define MAX_IPV6_ADDRESS 42
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#define MAX_CCMNI_NUMBER 3

#define DATA_STATE_ACTIVE 2
#define DATA_STATE_LINKDOWN 1
#define DATA_STATE_INACTIVE 0

#define MAX_L2P_VALUE_LENGTH 20
#define L2P_VALUE_UPS "M-UPS"
#define L2P_VALUE_CCMNI "M-CCMNI"
#define L2P_VALUE_MBIM "M-MBIM"

#ifdef MTK_IPV6_SUPPORT
    #define DEFAULT_MAX_PDP_NUM 6
#else
    #define DEFAULT_MAX_PDP_NUM 3
#endif

#define DONGLE_MAX_PDP_NUM 2

/* +ESCRI: URC result code */
#define SCRI_RESULT_REQ_SENT 0
#define SCRI_CS_SESSION_ONGOING 1
#define SCRI_PS_SIGNALLING_ONGOING 2
#define SCRI_NO_PS_DATA_SESSION 3
#define SCRI_REQ_NOT_SENT 4 
#define SCRI_NOT_ALLOWED 5
#define SCRI_RAU_ENABLED 6

#define CCCI_IOC_MAGIC    'C'
#define CCCI_IOC_FORCE_FD    _IOW(CCCI_IOC_MAGIC, 16, unsigned int)

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    #define CCMNI_IFNAME "usb"
    #define LOG_TAG "RILEXTMD"   
#else
#if defined(MTK_ENABLE_MD5)
    #define CCMNI_IFNAME "ccemni"
    #define LOG_TAG "RIL"
#else
#ifdef MTK_RIL_MD1
    #define CCMNI_IFNAME "ccmni"
    #define LOG_TAG "RIL"
#else
    #define CCMNI_IFNAME "cc2mni"
    #define LOG_TAG "RILMD2"	
#endif
#endif
#endif

#define disable_test_load 

#ifdef AT_RSP_FREE
#undef AT_RSP_FREE
#endif

#define AT_RSP_FREE(rsp)    \
if (rsp) {                  \
    at_response_free(rsp);  \
    rsp = NULL;             \
}

#ifdef FREEIF
#undef FREEIF
#endif

#define FREEIF(data)    \
if (data != NULL) {     \
    free(data);         \
    data = NULL;        \
}


#define MTK_REQUEST_SETUP_DATA_CALL(data,datalen,token) \
        requestSetupDataCall(data,datalen,token)
#define MTK_REQUEST_DEACTIVATE_DATA_CALL(data,datalen,token) \
        requestDeactiveDataCall(data,datalen,token)
#define MTK_REQUEST_LAST_DATA_CALL_FAIL_CAUSE(data,datalen,token) \
        requestLastDataCallFailCause(data,datalen,token)
#define MTK_REQUEST_DATA_CALL_LIST(data,datalen,token) \
        requestDataCallList(data,datalen,token)
#define MTK_REQUEST_SET_INITIAL_ATTACH_APN(data,datalen,token) \
        requestSetInitialAttachApn(data,datalen,token)
#define MTK_REQUEST_SET_SCRI(data, datalen, token) \
        requestSetScri(data, datalen, token)
//[New R8 modem FD]
#define MTK_REQUEST_FD_MODE(data, datalen, token) \
        requestSetFDMode(data, datalen, token)
#define MTK_UNSOL_DATA_CALL_LIST_CHANGED(token) \
        onDataCallListChanged(token)
#define MTK_REQUEST_DETACH_PS(data, datalen, token) \
        requestDetachPS(data, datalen, token)
#define MTK_REQUEST_DIAL_UP_CSD(data, datalen, token) \
        requestDialUpCsd(data, datalen, token)

extern void requestSetupDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestSetupDataCallOverIPv6(const char* apn, const char* username, const char* password, int authType, int protocol, int interfaceId, const char* profile, RIL_Token t);
extern void requestOrSendDataCallListIpv6(RILChannelCtx* rilchnlctx, RIL_Token *t, RILId rilid);
extern void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t);
extern void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t);
extern void requestDataCallList(void * data, size_t datalen, RIL_Token t);
extern void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t);
extern void requestSetScri(void* data, size_t datalen, RIL_Token t);
extern void requestDialUpCsd(void* data, size_t datalen, RIL_Token t);
extern void onScriResult(char *s, RILId rid);
extern void onDataCallListChanged(RIL_Token t);
extern void onGPRSDeatch(char* urc, RILId rid);
extern void onPacketsFlushIndication(char* s, RILId rilid);

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

extern void requestDetachPS(void* data, size_t datalen, RIL_Token t);

typedef enum {
    SM_OPERATOR_BARRED = 0b00001000,
    SM_MBMS_CAPABILITIES_INSUFFICIENT = 0b00011000,
    SM_LLC_SNDCP_FAILURE = 0b00011001,
    SM_INSUFFICIENT_RESOURCES = 0b00011010,
    SM_MISSING_UNKNOWN_APN = 0b00011011,
    SM_UNKNOWN_PDP_ADDRESS_TYPE = 0b00011100,
    SM_USER_AUTHENTICATION_FAILED = 0b00011101,
    SM_ACTIVATION_REJECT_GGSN = 0b00011110 ,
    SM_ACTIVATION_REJECT_UNSPECIFIED = 0b00011111,
    SM_SERVICE_OPTION_NOT_SUPPORTED = 0b00100000,
    SM_SERVICE_OPTION_NOT_SUBSCRIBED = 0b00100001,
    SM_SERVICE_OPTION_OUT_OF_ORDER = 0b00100010,
    SM_NSAPI_IN_USE = 0b00100011,
    SM_REGULAR_DEACTIVATION = 0b00100100,
    SM_QOS_NOT_ACCEPTED = 0b00100101,
    SM_NETWORK_FAILURE = 0b00100110,
    SM_REACTIVATION_REQUESTED = 0b00100111,
    SM_FEATURE_NOT_SUPPORTED = 0b00101000,
    SM_SEMANTIC_ERROR_IN_TFT = 0b00101001,
    SM_SYNTACTICAL_ERROR_IN_TFT = 0b00101010,
    SM_UNKNOWN_PDP_CONTEXT = 0b00101011,
    SM_SEMANTIC_ERROR_IN_PACKET_FILTER = 0b00101100,
    SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER = 0b00101101,
    SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED = 0b00101110,
    SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT = 0b00101111,
    SM_BCM_VIOLATION = 0b00110000,
    SM_ONLY_IPV4_ALLOWED = 0b00110010,
    SM_ONLY_IPV6_ALLOWED = 0b00110011,
    SM_ONLY_SINGLE_BEARER_ALLOWED = 0b00110100,
    SM_COLLISION_WITH_NW_INITIATED_REQUEST = 0b00111000,
    SM_BEARER_HANDLING_NOT_SUPPORT = 0b00111100,
    SM_MAX_PDP_NUMBER_REACHED = 0b01000001,
    SM_APN_NOT_SUPPORT_IN_RAT_PLMN = 0b01000010,
    SM_INVALID_TRANSACTION_ID_VALUE = 0b01010001,
    SM_SEMENTICALLY_INCORRECT_MESSAGE = 0b01011111,
    SM_INVALID_MANDATORY_INFO = 0b01100000,
    SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED = 0b01100001,
    SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100010,
    SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED = 0b01100011,
    SM_CONDITIONAL_IE_ERROR = 0b01100100,
    SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100101,
    SM_PROTOCOL_ERROR = 0b01101111,
    SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT = 0b01110000
} AT_SM_Error;

typedef struct {
    int interfaceId;
    int primaryCid;
    int active; //0: inactive, 1: link down, 2: active
    char addressV4[MAX_IPV4_ADDRESS];
    char addressV6[MAX_IPV6_ADDRESS];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS];
} PdnInfo;

#define __PACKETS_FLUSH_INDICATION__ 1
/*depend on Modem compile option*/
#ifdef  __PACKETS_FLUSH_INDICATION__
#define PACKETS_FLUSH_INDICATION
#endif

#endif /* RIL_DATA_H */

