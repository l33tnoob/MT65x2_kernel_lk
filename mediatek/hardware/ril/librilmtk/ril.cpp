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

/* //device/libs/telephony/ril.cpp
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

#ifdef MTK_RIL_MD1
#define LOG_TAG "RILC"
#else
#define LOG_TAG "RILCMD2"
#endif

#include <hardware_legacy/power.h>

#include <telephony/ril.h>
#include <telephony/ril_cdma_sms.h>
#include <sys/limits.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <alloca.h>
#include <assert.h>
#include <netinet/in.h>
#include <cutils/properties.h>

#include <librilmtk.h>
#include <DfoDefines.h>
/* MTK */

namespace android {
#include <ril_gsm_util.h>

static void listenCallback (int fd, short flags, void *param);

#ifdef FATAL_ERROR_HANDLE
static void handleRILDFatalError();
#endif

static RILId s_ril_cntx[] = {
    MTK_RIL_SOCKET_1
#ifdef MTK_GEMINI
    , MTK_RIL_SOCKET_2
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    , MTK_RIL_SOCKET_3
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    , MTK_RIL_SOCKET_4
#endif
#endif
};

#define PHONE_PROCESS "radio"

#define SOCKET_NAME_RIL "rild"
#ifdef MTK_GEMINI
#define PHONE2_PROCESS "radio2"
#define SOCKET_NAME_RIL2 "rild2"
#define PROPERTY_3G_SIM "gsm.3gswitch"

#if (MTK_GEMINI_SIM_NUM >= 3)
#define PHONE3_PROCESS "radio3"
#define SOCKET_NAME_RIL3 "rild3"
#endif
#if (MTK_GEMINI_SIM_NUM >= 4)
#define PHONE4_PROCESS "radio4"
#define SOCKET_NAME_RIL4 "rild4"
#endif
#endif /* MTK_GEMINI */

#define SOCKET_NAME_RIL_DEBUG "rild-debug"
#define SOCKET_NAME_RIL_OEM "rild-oem"
#ifndef REVERSE_MTK_CHANGE
#define SOCKET_NAME_MTK_UT  "rild-mtk-ut"
#define SOCKET_NAME_MTK_UT_2    "rild-mtk-ut-2"
#endif

#define SOCKET_NAME_RIL_MD2 "rild-md2"
#define SOCKET_NAME_RIL2_MD2 "rild2-md2"
#define SOCKET_NAME_RIL_DEBUG_MD2 "rild-debug-md2"
#define SOCKET_NAME_RIL_OEM_MD2 "rild-oem-md2"
#define SOCKET_NAME_MTK_UT_MD2 "rild-mtk-ut-md2"
#define SOCKET_NAME_MTK_UT_2_MD2 "rild-mtk-ut-2-md2"
#define SOCKET_NAME_ATCI_MD2 "rild-atci-md2"
    

/* atci start */
#define SOCKET_NAME_ATCI "rild-atci"
/* atci end */

#define ANDROID_WAKE_LOCK_NAME "radio-interface"


#define PROPERTY_RIL_IMPL "gsm.version.ril-impl"


// match with constant in RIL.java
#define MAX_COMMAND_BYTES (8 * 1024)

// Basically: memset buffers that the client library
// shouldn't be using anymore in an attempt to find
// memory usage issues sooner.
#define MEMSET_FREED 1

#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1

/* Negative values for private RIL errno's */
#define RIL_ERRNO_INVALID_RESPONSE -1

// request, response, and unsolicited msg print macro
#define PRINTBUF_SIZE 8096

// Enable RILC log (don't enable when MTK_RIL is defined)
#define RILC_LOG 0

#define startRequest
#define closeRequest
#define printRequest(token, req)
#define startResponse
#define closeResponse
#define printResponse
#define clearPrintBuf
#define removeLastChar
#define appendPrintBuf(x...)




/*******************************************************************/

RIL_RadioFunctions s_callbacks = {0, NULL, NULL, NULL, NULL, NULL};
static int s_registerCalled = 0;

static pthread_t s_tid_dispatch;
static pthread_t s_tid_reader;
static int s_started = 0;

static int s_fdListen = -1;
static int s_fdCommand = -1;
#ifdef MTK_GEMINI

static int s_fdListen2  = -1;
static int s_fdCommand2 = -1;

#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static int s_fdListen3  = -1;
static int s_fdCommand3 = -1;
#endif

#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static int s_fdListen4  = -1;
static int s_fdCommand4 = -1;
#endif

#endif /* MTK_GEMINI */

static int s_fdDebug = -1;
static int s_fdOem = -1;

static int s_mdCounter = 0;

static int s_fdOem_command = -1;
static int s_EAPSIMAKA_fd = -1;
static int s_EAPSIMAKA_fd2 = -1;
static int s_THERMAL_fd = -1;

#ifndef REVERSE_MTK_CHANGE
static int s_fdUT_listen = -1;
static int s_fdUT_command = -1;
static int s_fdUT_tmp_command = -1;
#ifdef MTK_GEMINI
static int s_fdUT_listen2 = -1;
static int s_fdUT_command2 = -1;
static int s_fdUT_tmp_command2 = -1;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static int s_fdUT_listen3 = -1;
static int s_fdUT_command3 = -1;
static int s_fdUT_tmp_command3 = -1;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static int s_fdUT_listen4 = -1;
static int s_fdUT_command4 = -1;
static int s_fdUT_tmp_command4 = -1;
#endif
#endif /*MTK_GEMINI*/
#endif /*REVERSE_MTK_CHANGE*/

/* atci start */
static int s_fdATCI_listen = -1;
static int s_fdATCI_command = -1;
static int s_fdReserved_command = -1;
/* atci end */


static int s_fdWakeupRead;
static int s_fdWakeupWrite;

static struct ril_event s_commands_event;
static struct ril_event s_wakeupfd_event;
static struct ril_event s_listen_event;
static struct ril_event s_wake_timeout_event;
static struct ril_event s_debug_event;
static struct ril_event s_oem_event;
#ifdef MTK_GEMINI
static struct ril_event s_commands_event2;
static struct ril_event s_listen_event2;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static struct ril_event s_commands_event3;
static struct ril_event s_listen_event3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static struct ril_event s_commands_event4;
static struct ril_event s_listen_event4;
#endif

#endif /* MTK_GEMINI */

#ifndef REVERSE_MTK_CHANGE
static struct ril_event s_UTlisten_event;
static struct ril_event s_UTcommand_event;
#ifdef MTK_GEMINI
static struct ril_event s_UTlisten_event2;
static struct ril_event s_UTcommand_event2;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static struct ril_event s_UTlisten_event3;
static struct ril_event s_UTcommand_event3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static struct ril_event s_UTlisten_event4;
static struct ril_event s_UTcommand_event4;
#endif
#endif /*MTK_GEMINI*/
#endif /*REVERSE_MTK_CHANGE*/

/* atci start */
static struct ril_event s_ATCIlisten_event;
static struct ril_event s_ATCIcommand_event;
static char simNo[PROPERTY_VALUE_MAX];
/* atci end */

static const struct timeval TIMEVAL_WAKE_TIMEOUT = {1,0};

static pthread_mutex_t s_pendingRequestsMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_startupMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_startupCond = PTHREAD_COND_INITIALIZER;


static RequestInfo *s_pendingRequests = NULL;

#ifdef MTK_GEMINI
static pthread_mutex_t s_pendingRequestsMutex2  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex2            = PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests2          = NULL;

#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static pthread_mutex_t s_pendingRequestsMutex3	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex3			= PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests3			= NULL;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static pthread_mutex_t s_pendingRequestsMutex4	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeMutex4			= PTHREAD_MUTEX_INITIALIZER;
static RequestInfo *s_pendingRequests4			= NULL;
#endif

#endif /* MTK_GEMINI */

#ifdef MTK_RIL
pthread_mutex_t s_dispatchMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_dispatchCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t s_queryThreadMutex = PTHREAD_MUTEX_INITIALIZER;


RequestInfoProxy* s_Proxy[RIL_SUPPORT_PROXYS] = {NULL};
pthread_t s_tid_proxy[RIL_SUPPORT_PROXYS];

#endif /* MTK_RIL */


static UserCallbackInfo *s_last_wake_timeout_info = NULL;
static pthread_mutex_t s_wakeTimeoutMutex = PTHREAD_MUTEX_INITIALIZER;

static void *s_lastNITZTimeData = NULL;
static size_t s_lastNITZTimeDataSize;

SocketListenParam s_SocketLinterParam1;

#ifdef MTK_GEMINI
SocketListenParam s_SocketLinterParam2;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
SocketListenParam s_SocketLinterParam3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
SocketListenParam s_SocketLinterParam4;
#endif
#endif /* MTK_GEMINI */

// to store STK PCI string temporarily
//static char *s_tempStkPci = NULL;
//static char *s_tempStkPci_2 = NULL;
//static int s_tempStkPciFlag = 0;
//static int s_tempStkPciFlag_2 = 0;

static char *s_tempStkPci[MTK_RIL_SOCKET_NUM] = {NULL};
static int s_tempStkPciFlag[MTK_RIL_SOCKET_NUM] = {0};

static AtResponseList* cacheUrc(AtResponseList* pendedUrcList,
                                  int unsolResponse, 
                                  void *data,
                                  size_t datalen, 
                                  RILId id);

static void sendPendedUrcs(RILId rid);
static void onTempStkPci(RILId rid);

// To queue PHB ready event
static int s_phbReadyFlag[MTK_RIL_SOCKET_NUM] = {0};

/* Store ECFU URC and send to RILJ after socket is connected. */
#ifndef REVERSE_MTK_CHANGE
#ifndef MTK_RILD_UT
#define ECFU_COUNT 10
static void storeEcfuUrc(int unsolResponse, RILId id, void *data);
static void onEcfu(RILId rid);
static int s_tempEcfu_1[ECFU_COUNT][2];
static int s_tempEcfu_2[ECFU_COUNT][2];
static int s_tempEcfu_3[ECFU_COUNT][2];
static int s_tempEcfu_4[ECFU_COUNT][2];
static int s_tempEcfuCount_1 = 0;
static int s_tempEcfuCount_2 = 0;
static int s_tempEcfuCount_3 = 0;
static int s_tempEcfuCount_4 = 0;
#endif
#endif

/*******************************************************************/
static void dispatchUssd (Parcel& p, RequestInfo *pRI); /* MTK */
static void dispatchVoid (Parcel& p, RequestInfo *pRI);
static void dispatchString (Parcel& p, RequestInfo *pRI);
static void dispatchStrings (Parcel& p, RequestInfo *pRI);
static void dispatchInts (Parcel& p, RequestInfo *pRI);
static void dispatchDial (Parcel& p, RequestInfo *pRI);
static void dispatchSIM_IO (Parcel& p, RequestInfo *pRI);
static void dispatchCallForward(Parcel& p, RequestInfo *pRI);
static void dispatchRaw(Parcel& p, RequestInfo *pRI);
static void dispatchSmsWrite (Parcel &p, RequestInfo *pRI);
static void dispatchDataCall (Parcel& p, RequestInfo *pRI);

static void dispatchCdmaSms(Parcel &p, RequestInfo *pRI);
static void dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI);
static void dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI);
static void dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI);
static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI);
static void dispatchPhbEntry(Parcel &p, RequestInfo *pRI);
static void dispatchWritePhbEntryExt(Parcel &p, RequestInfo *pRI);
static void dispatchSetInitialAttachApn (Parcel& p, RequestInfo *pRI);

//[New R8 modem FD]
static void dispatchFD_Mode(Parcel &p, RequestInfo *pRI);

static int responseUssdStrings(Parcel &p, void *response, size_t responselen);
static int responseInts(Parcel &p, void *response, size_t responselen);
static int responseStrings(Parcel &p, void *response, size_t responselen);
static int responseString(Parcel &p, void *response, size_t responselen);
static int responseVoid(Parcel &p, void *response, size_t responselen);
static int responseCallList(Parcel &p, void *response, size_t responselen);
static int responseSMS(Parcel &p, void *response, size_t responselen);
static int responseSIM_IO(Parcel &p, void *response, size_t responselen);
static int responseCallForwards(Parcel &p, void *response, size_t responselen);
static int responseDataCallList(Parcel &p, void *response, size_t responselen);
static int responseSetupDataCall(Parcel &p, void *response, size_t responselen);
static int responseRaw(Parcel &p, void *response, size_t responselen);
static int responseSsn(Parcel &p, void *response, size_t responselen);
static int responseCrssN(Parcel &p, void *response, size_t responselen);
static int responseSimStatus(Parcel &p, void *response, size_t responselen);
static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen);
static int responseCdmaSms(Parcel &p, void *response, size_t responselen);
static int responseCellList(Parcel &p, void *response, size_t responselen);
static int responseCdmaInformationRecords(Parcel &p,void *response, size_t responselen);
static int responseRilSignalStrength(Parcel &p,void *response, size_t responselen);
static int responseCallRing(Parcel &p, void *response, size_t responselen);
static int responseCdmaSignalInfoRecord(Parcel &p,void *response, size_t responselen);
static int responseCdmaCallWaiting(Parcel &p,void *response, size_t responselen);

/* MR2 migration START */
static int responseCellInfoList(Parcel &p, void *response, size_t responselen);
/* MR2 migration END */

static int responsePhbEntries(Parcel &p,void *response, size_t responselen);
static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen);
static int handleSpecialRequestWithArgs(int argCount, char** args);
static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen);
static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen);
static void dispatchSmsParams(Parcel &p, RequestInfo *pRI);
static int responseSmsParams(Parcel &p, void *response, size_t responselen);
static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen);
static int responseEtwsNotification(Parcel &p, void *response, size_t responselen);
//MTK-START [mtk80776] WiFi Calling
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI);
static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI);
//MTK-END [mtk80776] WiFi Calling

#ifdef RIL_SHLIB
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
        size_t datalen, RILId id);
#endif

static UserCallbackInfo * internalRequestTimedCallback
(RIL_TimedCallback callback, void *param,
 const struct timeval *relativeTime);

#ifdef MTK_RIL
static UserCallbackInfo * internalRequestProxyTimedCallback
(RIL_TimedCallback callback, void *param,
const struct timeval *relativeTime, int proxyId);

static AtResponseList* pendedUrcList1 = NULL;
#ifdef MTK_GEMINI
static AtResponseList* pendedUrcList2 = NULL;
#endif /* MTK_GEMINI */
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static AtResponseList* pendedUrcList3 = NULL;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static AtResponseList* pendedUrcList4 = NULL;
#endif
#endif /*MTK_RIL*/

/** Index == requestNumber */
static CommandInfo s_commands[] = {
#include "ril_commands.h"
};

static UnsolResponseInfo s_unsolResponses[] = {
#include "ril_unsol_commands.h"
};

#ifdef MTK_RIL
static CommandInfo s_mtk_commands[] = {
#include "mtk_ril_commands.h"
};

static UnsolResponseInfo s_mtk_unsolResponses[] = {
#include "mtk_ril_unsol_commands.h"
};

static CommandInfo s_mtk_local_commands[] = {
#include "mtk_ril_local_commands.h" 
};

#endif /* MTK_RIL */

static void
printRILData(const void *data, size_t len)
{
    unsigned int i;
    char *pBuf = (char *) data;
    for (i=0; i <= (len - 1)/16; i++)
    {
        LOGI("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             pBuf[16*i],pBuf[16*i+1],pBuf[16*i+2],pBuf[16*i+3],pBuf[16*i+4],pBuf[16*i+5],pBuf[16*i+6],pBuf[16*i+7],
             pBuf[16*i+8],pBuf[16*i+9],pBuf[16*i+10],pBuf[16*i+11],pBuf[16*i+12],pBuf[16*i+13],pBuf[16*i+14],pBuf[16*i+15]);
    }
}

static char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}


static void
memsetString (char *s) {
    if (s != NULL) {
        memset (s, 0, strlen(s));
    }
}

void   nullParcelReleaseFunction (const uint8_t* data, size_t dataSize,
                                  const size_t* objects, size_t objectsSize,
                                  void* cookie) {
    // do nothing -- the data reference lives longer than the Parcel object
}

/**
 * To be called from dispatch thread
 * Issue a single local request, ensuring that the response
 * is not sent back up to the command process
 */
static void
issueLocalRequest(int request, void *data, int len) {
    RequestInfo *pRI;
    int ret;

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 1;
    pRI->token = 0xffffffff;        // token is not used in this context

    if (request < 1 || request > (int32_t)NUM_ELEMS(s_commands)) {
#ifdef MTK_RIL
        if (request >= (RIL_REQUEST_MTK_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)))
#endif /* MTK_RIL */
        {

            LOGE("issueLocalRequest: unsupported request code %d", request);
            // FIXME this should perhaps return a response
            return;
        }
    }

#ifdef MTK_RIL
    if (request < RIL_REQUEST_MTK_BASE) {
        pRI->pCI = &(s_commands[request]);
    } else {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_MTK_BASE]);
    }
#else
    pRI->pCI = &(s_commands[request]);
#endif /* MTK_RIL */
    ret = pthread_mutex_lock(&s_pendingRequestsMutex);
    assert (ret == 0);

    pRI->p_next = s_pendingRequests;
    s_pendingRequests = pRI;

    ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
    assert (ret == 0);

    LOGD("C[locl]> %s", requestToString(request));

    s_callbacks.onRequest(request, data, len, pRI);
}

/** 
 * To be called from dispatch thread
 * Issue a single local request for specified SIM, 
 * and sent back up to the command process
 */
extern "C"
void issueLocalRequestForResponse(int request, void *data, int len, RILId id) {
    RequestInfo *pRI;
    int ret;
	status_t status;

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 0;

    if (request < 1 || request > (int32_t)NUM_ELEMS(s_commands)) {
#ifdef MTK_RIL
        if (request < RIL_REQUEST_MTK_BASE || request >= (RIL_REQUEST_MTK_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)))
#endif /* MTK_RIL */
        {
#ifdef MTK_EAP_SIM_AKA
            if(request < RIL_LOCAL_REQUEST_MTK_BASE || request >= (RIL_LOCAL_REQUEST_MTK_BASE + (int32_t)NUM_ELEMS(s_mtk_local_commands)))
#endif
            {
                LOGE("issueLocalRequestForResponse: unsupported request code %d", request);
                // FIXME this should perhaps return a response
                return;
            }
        }
    }
   
#ifdef MTK_RIL    
    if (request >= RIL_LOCAL_REQUEST_MTK_BASE){
        pRI->pCI = &(s_mtk_local_commands[request - RIL_LOCAL_REQUEST_MTK_BASE]);
    }else if(request < RIL_REQUEST_MTK_BASE) {
        pRI->pCI = &(s_commands[request]);    
    } else {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_MTK_BASE]);    
    } 
      
#else
    pRI->pCI = &(s_commands[request]);
#endif /* MTK_RIL */

#ifdef MTK_GEMINI
    if (id == MTK_RIL_SOCKET_1){
        pRI->cid = RIL_CMD_PROXY_1;
        ret = pthread_mutex_lock(&s_pendingRequestsMutex);
        assert (ret == 0);

        pRI->p_next = s_pendingRequests;
        s_pendingRequests = pRI;

        ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
        assert (ret == 0);      
    }else if(id == MTK_RIL_SOCKET_2){
        pRI->cid = RIL_CMD2_PROXY_1;
        ret = pthread_mutex_lock(&s_pendingRequestsMutex2);
        assert (ret == 0);

        pRI->p_next = s_pendingRequests2;
        s_pendingRequests2 = pRI;

        ret = pthread_mutex_unlock(&s_pendingRequestsMutex2);
        assert (ret == 0);          
    }
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4SIM*/
     else if (id == MTK_RIL_SOCKET_4) {
	    pRI->cid = RIL_CMD4_PROXY_1;
        ret = pthread_mutex_lock(&s_pendingRequestsMutex4);
        assert (ret == 0);

        pRI->p_next = s_pendingRequests4;
        s_pendingRequests4 = pRI;

        ret = pthread_mutex_unlock(&s_pendingRequestsMutex4);
        assert (ret == 0);			
	}
#endif	
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    else if (id == MTK_RIL_SOCKET_3) {
	    pRI->cid = RIL_CMD3_PROXY_1;
        ret = pthread_mutex_lock(&s_pendingRequestsMutex3);
        assert (ret == 0);

        pRI->p_next = s_pendingRequests3;
        s_pendingRequests3 = pRI;

        ret = pthread_mutex_unlock(&s_pendingRequestsMutex3);
        assert (ret == 0);			
	}
#endif
#else 
    pRI->cid = RIL_CMD_PROXY_1;
    ret = pthread_mutex_lock(&s_pendingRequestsMutex);
    assert (ret == 0);

    pRI->p_next = s_pendingRequests;
    s_pendingRequests = pRI;

    ret = pthread_mutex_unlock(&s_pendingRequestsMutex);
    assert (ret == 0);
#endif


    LOGD("C[locl]> %s", requestToString(request));

#ifdef MTK_RIL
	{
		RequestInfoProxy * proxy = NULL;
		int proxyId;
		const char* proxyName = NULL;
		int queueSize = 0;

		proxy = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
		assert(proxy);
		proxy->p = new Parcel();

		proxy->p_next = NULL;
		proxy->pRI = pRI;
		proxy->pUCI = NULL;
		
        if(request == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION){
            const char**    strings = (const char**)data;
            writeStringToParcel(*(proxy->p), strings[0]);
            proxy->p->setDataPosition(0);
        } else if (request == RIL_LOCAL_REQUEST_USIM_AUTHENTICATION) {
            const char**    strings = (const char**)data;
            proxy->p->writeInt32(len);
            writeStringToParcel(*(proxy->p), strings[0]);
            writeStringToParcel(*(proxy->p), strings[1]);
            proxy->p->setDataPosition(0);
        } else if (request == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL) {
            int32_t token;	
            proxy->p->writeInt32(request);
             // Local request for response will be handle specially,
             // so set the token id as 0xFFFFFFFF
             proxy->p->writeInt32(0xffffffff);
             proxy->p->write(data, len);

             status = proxy->p->readInt32(&request);
             status = proxy->p->readInt32 (&token);
             assert(status == NOERROR);
        } else if (request == RIL_LOCAL_REQUEST_RILD_READ_IMSI) {
            int param = *((int*)data);
            LOGI("check 2: (%d,%d)", len, param);
            proxy->p->writeInt32(len);
            proxy->p->writeInt32(param);
            proxy->p->setDataPosition(0);
        } else {
             int32_t token;	
             proxy->p->writeInt32(request);
             // Local request for response will be handle specially,
             // so set the token id as 0xFFFFFFFF
             proxy->p->writeInt32(0xffffffff);
             proxy->p->write(data, len);

             status = proxy->p->readInt32(&request);
             status = proxy->p->readInt32 (&token);
             assert(status == NOERROR);        
        }
        proxyId = pRI->pCI->proxyId;

#ifdef MTK_GEMINI
		/* Shift proxyId if needed */
		if (id == MTK_RIL_SOCKET_2 /*&& request != RIL_REQUEST_DUAL_SIM_MODE_SWITCH*/) {
			proxyId = proxyId + RIL_CHANNEL_OFFSET;
			/* Update */
		}
#if (MTK_GEMINI_SIM_NUM >= 3)
		else if (id == MTK_RIL_SOCKET_3) {
			proxyId = proxyId + RIL_CHANNEL_SET3_OFFSET;
		}
#endif
#if (MTK_GEMINI_SIM_NUM >= 4)
		else if (id == MTK_RIL_SOCKET_4) {
			proxyId = proxyId + RIL_CHANNEL_SET4_OFFSET;
		}
#endif
#endif /* MTK_GEMINI */

		proxyName = ::proxyIdToString(proxyId);

		/* Save dispatched proxy in RequestInfo */
		proxy->pRI->cid = (RILChannelId) proxyId;

		queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

		if (0 != queueSize)
		{
			LOGD("%s is busy. %s queued. total:%d requests pending",proxyName,requestToString(request),queueSize+1);
		}

	}
#else
	s_callbacks.onRequest(request, data, len, pRI);
#endif
}


static int
processCommandBuffer(void *buffer, size_t buflen, RILId id) {
    status_t status;
    int32_t request;
    int32_t token;
    RequestInfo *pRI;
    int ret;
    Parcel p;
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo **    pendingRequestsHook = &s_pendingRequests;
    char prop_value[2]= {0};

    p.setData((uint8_t *) buffer, buflen);

    // status checked at end
    status = p.readInt32(&request);
    status = p.readInt32 (&token);

    /* LOG RIL Request for debugging */
    LOGI("RIL(%d) SOCKET REQUEST: %s length:%d",id+1, requestToString(request),buflen);
    
    if (MTK_RIL_SOCKET_1 == id) {
        //LOGI("RIL(%d) SOCKET REQUEST: %s length:%d",id, requestToString(request),buflen);
        pendingRequestsMutexHook = &s_pendingRequestsMutex;
        pendingRequestsHook = &s_pendingRequests;
    }
#ifdef MTK_GEMINI
    else {
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4SIM*/
        if (MTK_RIL_SOCKET_4 == id) {
            //LOGI("RIL(4) SOCKET REQUEST: %s length:%d",requestToString(request),buflen);
            /* Update hook for MTK_RIL_SOCKET_4*/
            pendingRequestsMutexHook = &s_pendingRequestsMutex4;
            pendingRequestsHook = &s_pendingRequests4;
        }
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
        if (MTK_RIL_SOCKET_3 == id) {
            //LOGI("RIL(3) SOCKET REQUEST: %s length:%d",requestToString(request),buflen);
            /* Update hook for MTK_RIL_SOCKET_3*/
            pendingRequestsMutexHook = &s_pendingRequestsMutex3;
            pendingRequestsHook = &s_pendingRequests3;
        }
#endif
        if (MTK_RIL_SOCKET_2 == id) {
        //LOGI("RIL(2) SOCKET REQUEST: %s length:%d",requestToString(request),buflen);
        /* Update hook for MTK_RIL_SOCKET_2 */
        pendingRequestsMutexHook = &s_pendingRequestsMutex2;
        pendingRequestsHook = &s_pendingRequests2;
    }
    }
#endif /* MTK_GEMINI */

    if (buflen <= 0) {
        LOGE("invalid request buflen");
        return 0;
    }

    printRILData((const void *)buffer, buflen);

    if (status != NO_ERROR) {
        LOGE("invalid request block");
        return 0;
    }

    if (request < 1 || request >= (int32_t)NUM_ELEMS(s_commands)) {
#ifdef MTK_RIL
        if (request > (RIL_REQUEST_MTK_BASE + (int32_t)NUM_ELEMS(s_mtk_commands)) ||
            (request >= (int32_t)NUM_ELEMS(s_commands) && 
             request < RIL_REQUEST_MTK_BASE))
#endif /* MTK_RIL */
        {

            LOGE("unsupported request code %d token %d", request, token);
            // FIXME this should perhaps return a response
            return 0;
        }
    }

    LOGD("New command received from %s", rilIdToString(id));

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->token = token;
#ifdef MTK_RIL
    if (request >= RIL_REQUEST_MTK_BASE) {
        pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_MTK_BASE]);
    }
    else
#endif /* MTK_RIL */
    {
        pRI->pCI = &(s_commands[request]);
    }
    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

#ifdef MTK_RIL
    {
        RequestInfoProxy * proxy = NULL;
        int proxyId;
        const char* proxyName = NULL;
        int queueSize = 0;

        proxy = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
        proxy->p = new Parcel();

        assert(proxy);

        proxy->p_next = NULL;

        proxy->pRI = pRI;

        proxy->pUCI = NULL;

        proxy->p->setData((uint8_t *) buffer, buflen);

        status = proxy->p->readInt32(&request);
        status = proxy->p->readInt32 (&token);
        assert(status == NOERROR);



        if (request < RIL_REQUEST_MTK_BASE) {
            if (request > (sizeof(s_commands)/sizeof(s_commands[0]))) {
                LOGE("Invalid request id, id=%d", request);
                return 0;
            }
            proxyId = s_commands[request].proxyId;
        } else {
            if ((request - RIL_REQUEST_MTK_BASE) > (sizeof(s_mtk_commands)/sizeof(s_mtk_commands[0]))) {
                LOGE("Invalid request id, id=%d", request);
                return 0;
            }
            proxyId = s_mtk_commands[request - RIL_REQUEST_MTK_BASE].proxyId;
        }

#ifdef MTK_GEMINI
        /* Shift proxyId if needed */
        if (id == MTK_RIL_SOCKET_2 /*&& request != RIL_REQUEST_DUAL_SIM_MODE_SWITCH*/) {
            proxyId = proxyId + RIL_CHANNEL_OFFSET;
            /* Update */
        }
#if (MTK_GEMINI_SIM_NUM >= 3)
        else if (id == MTK_RIL_SOCKET_3) {
            proxyId = proxyId + RIL_CHANNEL_SET3_OFFSET;
        }
#endif
#if (MTK_GEMINI_SIM_NUM >= 4)
        else if (id == MTK_RIL_SOCKET_4) {
            proxyId = proxyId + RIL_CHANNEL_SET4_OFFSET;
        }
#endif
#endif /* MTK_GEMINI */

        proxyName = ::proxyIdToString(proxyId);

        /* Save dispatched proxy in RequestInfo */
        proxy->pRI->cid = (RILChannelId) proxyId;

        queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

        if (0 != queueSize)
        {
            LOGD("%s is busy. %s queued. total:%d requests pending",proxyName,requestToString(request),queueSize+1);
        }

    }
#else
    /*    sLastDispatchedToken = token; */

    pRI->pCI->dispatchFunction(p, pRI);


#endif

    return 0;
}

static void
invalidCommandBlock (RequestInfo *pRI)
{
    LOGE("invalid command block for token %d request %s",
         pRI->token, requestToString(pRI->pCI->requestNumber));
}


/* USSD messages using the default alphabet are coded with the GSM 7-bit default alphabet         *
 * given in clause 6.2.1. The message can then consist of up to 182 user characters (3GPP 23.038) */
#define MAX_RIL_USSD_STRING_LENGTH 255
/** Callee expects const char * in UCS2 Hex decimal format */
static void
dispatchUssd (Parcel& p, RequestInfo *pRI) {
    status_t    status;
    size_t      stringlen = 0;
    char*       string8 = NULL;
    bytes_t     ucs2String = NULL;
    bytes_t     ucs2HexString = NULL;

    string8 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%s", printBuf, string8);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    ucs2String = (bytes_t) calloc(2*(MAX_RIL_USSD_STRING_LENGTH+1),sizeof(char));
    //BEGIN mtk08470 [20130109][ALPS00436983]
    // memory overwrite if strlen(string8) is larger than MAX_RIL_USSD_STRING_LENGTH
    stringlen = utf8_to_ucs2((cbytes_t)string8, MIN(strlen(string8), MAX_RIL_USSD_STRING_LENGTH), ucs2String);
    //END mtk08470 [20130109][ALPS00436983]     
    ucs2HexString = (bytes_t) calloc(2*stringlen*2+1,sizeof(char));

    gsm_hex_from_bytes((char *)ucs2HexString, ucs2String, 2*stringlen);

    s_callbacks.onRequest(pRI->pCI->requestNumber, ucs2HexString,
                          sizeof(char *), pRI);

#ifdef MEMSET_FREED
    memsetString(string8);
    memsetString((char *) ucs2String);
    memsetString((char *) ucs2HexString);
#endif
    free(ucs2String);
    free(string8);
    free(ucs2HexString);
    return;
}

/** Callee expects NULL */
static void
dispatchVoid (Parcel& p, RequestInfo *pRI) {
    clearPrintBuf;
    printRequest(pRI->token, pRI->pCI->requestNumber);
    s_callbacks.onRequest(pRI->pCI->requestNumber, NULL, 0, pRI);
}

/** Callee expects const char * */
static void
dispatchString (Parcel& p, RequestInfo *pRI) {
    status_t status;
    size_t datalen;
    size_t stringlen;
    char *string8 = NULL;

    string8 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%s", printBuf, string8);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, string8,
                          sizeof(char *), pRI);

#ifdef MEMSET_FREED
    memsetString(string8);
#endif

    free(string8);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/** Callee expects const char ** */
static void
dispatchStrings (Parcel &p, RequestInfo *pRI) {
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)alloca(sizeof(char *));
        datalen = 0;
    } else if (((int)countStrings) == -1) {
        pStrings = NULL;
        datalen = 0;
    } else {
        datalen = sizeof(char *) * countStrings;

        pStrings = (char **)alloca(datalen);

        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
            appendPrintBuf("%s%s,", printBuf, pStrings[i]);
        }
    }
    removeLastChar;
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, pStrings, datalen, pRI);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString (pStrings[i]);
#endif
            free(pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
    }

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/** Callee expects const int * */
static void
dispatchInts (Parcel &p, RequestInfo *pRI) {
    int32_t count;
    status_t status;
    size_t datalen;
    int *pInts;

    status = p.readInt32 (&count);

    if (status != NO_ERROR || count == 0) {
        goto invalid;
    }

    datalen = sizeof(int) * count;
    pInts = (int *)alloca(datalen);

    startRequest;
    for (int i = 0 ; i < count ; i++) {
        int32_t t;

        status = p.readInt32(&t);
        pInts[i] = (int)t;
        appendPrintBuf("%s%d,", printBuf, t);

        if (status != NO_ERROR) {
            goto invalid;
        }
    }
    removeLastChar;
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, const_cast<int *>(pInts),
                          datalen, pRI);

#ifdef MEMSET_FREED
    memset(pInts, 0, datalen);
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


/**
 * Callee expects const RIL_SMS_WriteArgs *
 * Payload is:
 *   int32_t status
 *   String pdu
 */
static void
dispatchSmsWrite (Parcel &p, RequestInfo *pRI) {
    RIL_SMS_WriteArgs args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    status = p.readInt32(&t);
    args.status = (int)t;

    args.pdu = strdupReadString(p);

    if (status != NO_ERROR || args.pdu == NULL) {
        goto invalid;
    }

    args.smsc = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%d,%s,smsc=%s", printBuf, args.status,
                   (char*)args.pdu,  (char*)args.smsc);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);

#ifdef MEMSET_FREED
    memsetString (args.pdu);
#endif

    free (args.pdu);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_Dial *
 * Payload is:
 *   String address
 *   int32_t clir
 */
static void
dispatchDial (Parcel &p, RequestInfo *pRI) {
    RIL_Dial dial;
    RIL_UUS_Info uusInfo;
    int32_t sizeOfDial;
    int32_t t;
    int32_t uusPresent;
    status_t status;

    memset (&dial, 0, sizeof(dial));

    dial.address = strdupReadString(p);

    status = p.readInt32(&t);
    dial.clir = (int)t;

    if (status != NO_ERROR || dial.address == NULL) {
        goto invalid;
    }

    if (s_callbacks.version < 3) { // Remove when partners upgrade to version 3
        uusPresent = 0;
        sizeOfDial = sizeof(dial) - sizeof(RIL_UUS_Info *);
    } else {
        status = p.readInt32(&uusPresent);

        if (status != NO_ERROR) {
            goto invalid;
        }

        if (uusPresent == 0) {
            dial.uusInfo = NULL;
        } else {
            int32_t len;

            memset(&uusInfo, 0, sizeof(RIL_UUS_Info));

            status = p.readInt32(&t);
            uusInfo.uusType = (RIL_UUS_Type) t;

            status = p.readInt32(&t);
            uusInfo.uusDcs = (RIL_UUS_DCS) t;

            status = p.readInt32(&len);
            if (status != NO_ERROR) {
                goto invalid;
            }

            // The java code writes -1 for null arrays
            if (((int) len) == -1) {
                uusInfo.uusData = NULL;
                len = 0;
            } else {
                uusInfo.uusData = (char*) p.readInplace(len);
            }

            uusInfo.uusLength = len;
            dial.uusInfo = &uusInfo;
        }
        sizeOfDial = sizeof(dial);
    }

    startRequest;
    appendPrintBuf("%snum=%s,clir=%d", printBuf, dial.address, dial.clir);
    if (uusPresent) {
        appendPrintBuf("%s,uusType=%d,uusDcs=%d,uusLen=%d", printBuf,
                       dial.uusInfo->uusType, dial.uusInfo->uusDcs,
                       dial.uusInfo->uusLength);
    }
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &dial, sizeOfDial, pRI);

#ifdef MEMSET_FREED
    memsetString (dial.address);
#endif

    free (dial.address);

#ifdef MEMSET_FREED
    memset(&uusInfo, 0, sizeof(RIL_UUS_Info));
    memset(&dial, 0, sizeof(dial));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_SIM_IO_v6 *
 * Payload is:
 *   int32_t cla // NFC SEEK
 *   int32_t command
 *   int32_t fileid
 *   String path
 *   int32_t p1, p2, p3
 *   String data
 *   String pin2
 */
static void
dispatchSIM_IO (Parcel &p, RequestInfo *pRI) {
    RIL_SIM_IO_v6 simIO;
    int32_t t;
    status_t status;

    memset (&simIO, 0, sizeof(simIO));

    // note we only check status at the end

    // NFC SEEK start 
    simIO.cla = 0;
    if(pRI->pCI->requestNumber != RIL_REQUEST_SIM_IO) {
        status = p.readInt32(&t);
        simIO.cla = (int)t;
    }
    // NFC SEEK end 

    status = p.readInt32(&t);
    simIO.command = (int)t;

    status = p.readInt32(&t);
    simIO.fileid = (int)t;

    simIO.path = strdupReadString(p);

    status = p.readInt32(&t);
    simIO.p1 = (int)t;

    status = p.readInt32(&t);
    simIO.p2 = (int)t;

    status = p.readInt32(&t);
    simIO.p3 = (int)t;

    simIO.data = strdupReadString(p);
    simIO.pin2 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%scmd=0x%X,efid=0x%X,path=%s,%d,%d,%d,%s,pin2=%s", printBuf,
                   simIO.command, simIO.fileid, (char*)simIO.path,
                   simIO.p1, simIO.p2, simIO.p3,
                   (char*)simIO.data,  (char*)simIO.pin2);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber, &simIO, sizeof(simIO), pRI);

#ifdef MEMSET_FREED
    memsetString (simIO.path);
    memsetString (simIO.data);
    memsetString (simIO.pin2);
#endif

    free (simIO.path);
    free (simIO.data);
    free (simIO.pin2);

#ifdef MEMSET_FREED
    memset(&simIO, 0, sizeof(simIO));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_CallForwardInfo *
 * Payload is:
 *  int32_t status/action
 *  int32_t reason
 *  int32_t serviceCode
 *  int32_t toa
 *  String number  (0 length -> null)
 *  int32_t timeSeconds
 */
static void
dispatchCallForward(Parcel &p, RequestInfo *pRI) {
    RIL_CallForwardInfo cff;
    int32_t t;
    status_t status;

    memset (&cff, 0, sizeof(cff));

    // note we only check status at the end

    status = p.readInt32(&t);
    cff.status = (int)t;

    status = p.readInt32(&t);
    cff.reason = (int)t;

    status = p.readInt32(&t);
    cff.serviceClass = (int)t;

    status = p.readInt32(&t);
    cff.toa = (int)t;

    cff.number = strdupReadString(p);

    status = p.readInt32(&t);
    cff.timeSeconds = (int)t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    // special case: number 0-length fields is null

    if (cff.number != NULL && strlen (cff.number) == 0) {
        cff.number = NULL;
    }

    startRequest;
    appendPrintBuf("%sstat=%d,reason=%d,serv=%d,toa=%d,%s,tout=%d", printBuf,
                   cff.status, cff.reason, cff.serviceClass, cff.toa,
                   (char*)cff.number, cff.timeSeconds);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &cff, sizeof(cff), pRI);

#ifdef MEMSET_FREED
    memsetString(cff.number);
#endif

    free (cff.number);

#ifdef MEMSET_FREED
    memset(&cff, 0, sizeof(cff));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


static void
dispatchRaw(Parcel &p, RequestInfo *pRI) {
    int32_t len;
    status_t status;
    const void *data;

    status = p.readInt32(&len);

    if (status != NO_ERROR) {
        goto invalid;
    }

    // The java code writes -1 for null arrays
    if (((int)len) == -1) {
        data = NULL;
        len = 0;
    }

    data = p.readInplace(len);

    startRequest;
    appendPrintBuf("%sraw_size=%d", printBuf, len);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, const_cast<void *>(data), len, pRI);

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchCdmaSms(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Message rcsm;
    int32_t  t;
    uint8_t ut;
    status_t status;
    int32_t digitCount;
    int digitLimit;

    memset(&rcsm, 0, sizeof(rcsm));

    status = p.readInt32(&t);
    rcsm.uTeleserviceID = (int) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.bIsServicePresent = (uint8_t) ut;

    status = p.readInt32(&t);
    rcsm.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsm.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sAddress.number_of_digits= (uint8_t) ut;

    digitLimit= MIN((rcsm.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.odd = (uint8_t) ut;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.number_of_digits = (uint8_t) ut;

    digitLimit= MIN((rcsm.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sSubAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.uBearerDataLen = (int) t;

    digitLimit= MIN((rcsm.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut, sizeof(ut));
        rcsm.aBearerData[digitCount] = (uint8_t) ut;
    }

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.Number_mode=%d, sAddress.number_type=%d, ",
                   printBuf, rcsm.uTeleserviceID,rcsm.bIsServicePresent,rcsm.uServicecategory,
                   rcsm.sAddress.digit_mode, rcsm.sAddress.number_mode,rcsm.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &rcsm, sizeof(rcsm),pRI);

#ifdef MEMSET_FREED
    memset(&rcsm, 0, sizeof(rcsm));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Ack rcsa;
    int32_t  t;
    status_t status;
    int32_t digitCount;

    memset(&rcsa, 0, sizeof(rcsa));

    status = p.readInt32(&t);
    rcsa.uErrorClass = (RIL_CDMA_SMS_ErrorClass) t;

    status = p.readInt32(&t);
    rcsa.uSMSCauseCode = (int) t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%suErrorClass=%d, uTLStatus=%d, ",
                   printBuf, rcsa.uErrorClass, rcsa.uSMSCauseCode);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa),pRI);

#ifdef MEMSET_FREED
    memset(&rcsa, 0, sizeof(rcsa));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    RIL_GSM_BroadcastSmsConfigInfo gsmBci[num];
    RIL_GSM_BroadcastSmsConfigInfo *gsmBciPtrs[num];

    startRequest;
    for (int i = 0 ; i < num ; i++ ) {
        gsmBciPtrs[i] = &gsmBci[i];

        status = p.readInt32(&t);
        gsmBci[i].fromServiceId = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].toServiceId = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].fromCodeScheme = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].toCodeScheme = (int) t;

        status = p.readInt32(&t);
        gsmBci[i].selected = (uint8_t) t;

        appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId =%d, \
              fromCodeScheme=%d, toCodeScheme=%d, selected =%d]", printBuf, i,
                       gsmBci[i].fromServiceId, gsmBci[i].toServiceId,
                       gsmBci[i].fromCodeScheme, gsmBci[i].toCodeScheme,
                       gsmBci[i].selected);
    }
    closeRequest;

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber,
                          gsmBciPtrs,
                          num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *),
                          pRI);

#ifdef MEMSET_FREED
    memset(gsmBci, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
    memset(gsmBciPtrs, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void
dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    RIL_CDMA_BroadcastSmsConfigInfo cdmaBci[num];
    RIL_CDMA_BroadcastSmsConfigInfo *cdmaBciPtrs[num];

    startRequest;
    for (int i = 0 ; i < num ; i++ ) {
        cdmaBciPtrs[i] = &cdmaBci[i];

        status = p.readInt32(&t);
        cdmaBci[i].service_category = (int) t;

        status = p.readInt32(&t);
        cdmaBci[i].language = (int) t;

        status = p.readInt32(&t);
        cdmaBci[i].selected = (uint8_t) t;

        appendPrintBuf("%s [%d: service_category=%d, language =%d, \
              entries.bSelected =%d]", printBuf, i, cdmaBci[i].service_category,
                       cdmaBci[i].language, cdmaBci[i].selected);
    }
    closeRequest;

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber,
                          cdmaBciPtrs,
                          num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *),
                          pRI);

#ifdef MEMSET_FREED
    memset(cdmaBci, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
    memset(cdmaBciPtrs, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_WriteArgs rcsw;
    int32_t  t;
    uint32_t ut;
    uint8_t  uct;
    status_t status;
    int32_t  digitCount;

    memset(&rcsw, 0, sizeof(rcsw));

    status = p.readInt32(&t);
    rcsw.status = t;

    status = p.readInt32(&t);
    rcsw.message.uTeleserviceID = (int) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.bIsServicePresent = (uint8_t) uct;

    status = p.readInt32(&t);
    rcsw.message.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sAddress.number_of_digits = (uint8_t) uct;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_ADDRESS_MAX; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.odd = (uint8_t) uct;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.number_of_digits = (uint8_t) uct;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_SUBADDRESS_MAX; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sSubAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.uBearerDataLen = (int) t;

    for(digitCount = 0 ; digitCount < RIL_CDMA_SMS_BEARER_DATA_MAX; digitCount ++) {
        status = p.read(&uct, sizeof(uct));
        rcsw.message.aBearerData[digitCount] = (uint8_t) uct;
    }

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%sstatus=%d, message.uTeleserviceID=%d, message.bIsServicePresent=%d, \
            message.uServicecategory=%d, message.sAddress.digit_mode=%d, \
            message.sAddress.number_mode=%d, \
            message.sAddress.number_type=%d, ",
                   printBuf, rcsw.status, rcsw.message.uTeleserviceID, rcsw.message.bIsServicePresent,
                   rcsw.message.uServicecategory, rcsw.message.sAddress.digit_mode,
                   rcsw.message.sAddress.number_mode,
                   rcsw.message.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw),pRI);

#ifdef MEMSET_FREED
    memset(&rcsw, 0, sizeof(rcsw));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;

}


// For backwards compatibility in RIL_REQUEST_SETUP_DATA_CALL.
// Version 4 of the RIL interface adds a new PDP type parameter to support
// IPv6 and dual-stack PDP contexts. When dealing with a previous version of
// RIL, remove the parameter from the request.
static void dispatchDataCall(Parcel& p, RequestInfo *pRI) {
    // In RIL v3, REQUEST_SETUP_DATA_CALL takes 6 parameters.
    const int numParamsRilV3 = 6;

    // The first bytes of the RIL parcel contain the request number and the
    // serial number - see processCommandBuffer(). Copy them over too.
    int pos = p.dataPosition();

    int numParams = p.readInt32();
    if (s_callbacks.version < 4 && numParams > numParamsRilV3) {
        Parcel p2;
        p2.appendFrom(&p, 0, pos);
        p2.writeInt32(numParamsRilV3);
        for(int i = 0; i < numParamsRilV3; i++) {
            p2.writeString16(p.readString16());
        }
        p2.setDataPosition(pos);
        dispatchStrings(p2, pRI);
    } else {
        p.setDataPosition(pos);
        dispatchStrings(p, pRI);
    }
}

static void dispatchPhbEntry(Parcel &p, RequestInfo *pRI) {

    RIL_PhbEntryStrucutre args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // storage type
    status = p.readInt32(&t);
    args.type = (int) t;

    // index of the entry
    status = p.readInt32(&t);
    args.index = (int) t;

    // phone number
    args.number = strdupReadString(p);

    // Type of the number
    status = p.readInt32(&t);
    args.ton = (int) t;

    // alpha Id
    args.alphaId = strdupReadString(p);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s%d,index=%d,num=%s,ton=%d,alphaId=%s", printBuf, args.type,
                   args.index, (char*)args.number, args.ton,  (char*)args.alphaId);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);

#ifdef MEMSET_FREED
    memsetString (args.number);
    memsetString (args.alphaId);
#endif

    free (args.number);
    free (args.alphaId);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;

}

static void dispatchWritePhbEntryExt(Parcel &p, RequestInfo *pRI) {

    RIL_PHB_ENTRY args;
    int32_t t;
    status_t status;

    memset (&args, 0, sizeof(args));

    // index of the entry
    status = p.readInt32(&t);
    args.index = (int) t;
    // phone number
    args.number = strdupReadString(p);
    // Type of the number
    status = p.readInt32(&t);
    args.type = (int) t;
    //text
    args.text = strdupReadString(p);
    //hidden
    status = p.readInt32(&t);
    args.hidden = (int) t;

    //group
    args.group = strdupReadString(p);
    //anr
    args.adnumber = strdupReadString(p);
    // Type of the adnumber
    status = p.readInt32(&t);
    args.adtype = (int) t;
    //SNE
    args.secondtext = strdupReadString(p);
    // email
    args.email = strdupReadString(p);

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s,index=%d,num=%s,type=%d,text=%s,hidden=%d,group=%s,adnumber=%s,adtype=%d,secondtext=%s,email=%s", printBuf,
                   args.index, (char*)args.number, args.type, (char*)args.text,
                   args.hidden, (char*)args.group,(char*)args.adnumber, args.adtype,(char*)args.secondtext,(char*)args.email);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);

#ifdef MEMSET_FREED
    memsetString (args.number);
    memsetString (args.text);
    memsetString (args.group);
    memsetString (args.adnumber);
    memsetString (args.secondtext);
    memsetString (args.email);
#endif
    free (args.number);
    free (args.text);
    free (args.group);
    free (args.adnumber);
    free (args.secondtext);
    free (args.email);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;

}

static void dispatchSetInitialAttachApn(Parcel &p, RequestInfo *pRI)
{
    RIL_InitialAttachApn pf;
    int32_t  t;
    status_t status;

    memset(&pf, 0, sizeof(pf));

    pf.apn = strdupReadString(p);
    pf.protocol = strdupReadString(p);

    status = p.readInt32(&t);
    pf.authtype = (int) t;

    pf.username = strdupReadString(p);
    pf.password = strdupReadString(p);

    startRequest;
    appendPrintBuf("%sapn=%s, protocol=%s, auth_type=%d, username=%s, password=%s",
            printBuf, pf.apn, pf.protocol, pf.auth_type, pf.username, pf.password);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }
    s_callbacks.onRequest(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI);

#ifdef MEMSET_FREED
    memsetString(pf.apn);
    memsetString(pf.protocol);
    memsetString(pf.username);
    memsetString(pf.password);
#endif

    free(pf.apn);
    free(pf.protocol);
    free(pf.username);
    free(pf.password);

#ifdef MEMSET_FREED
    memset(&pf, 0, sizeof(pf));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

//[New R8 modem FD]
static void dispatchFD_Mode(Parcel &p, RequestInfo *pRI) {
    RIL_FDModeStructure args;
    status_t status;	
    int t_value = 0; 
    memset(&args, 0, sizeof(args));
    status = p.readInt32(&t_value);
    args.args_num = t_value;
	
    /* AT+EFD=<mode>[,<param1>[,<param2>]] */
    /* For all modes: but mode 0 & 1 only has one argument */
    if (args.args_num >= 1) {
        status = p.readInt32(&t_value);		
        args.mode = t_value;
    }
    /* For mode 2 & 3 */	
    if (args.args_num >= 2) {
        status = p.readInt32(&t_value);
        args.parameter1 = t_value;
    }
    /* Only mode 2 */
    if (args.args_num >=3) {
        status = p.readInt32(&t_value);
        args.parameter2 = t_value;
    }
    s_callbacks.onRequest(pRI->pCI->requestNumber, &args, sizeof(args), pRI);	
    	
}


static int
blockingWrite(int fd, const void *buffer, size_t len) {
    size_t writeOffset = 0;
    const uint8_t *toWrite;

    toWrite = (const uint8_t *)buffer;

    while (writeOffset < len) {
        ssize_t written;
        do {
            written = write (fd, toWrite + writeOffset,
                             len - writeOffset);
        } while (written < 0 && (errno == EINTR || errno == EAGAIN));

        if (written >= 0) {
            writeOffset += written;
        } else {   // written < 0
            LOGE ("RIL Response: unexpected error on write errno:%d", errno);
            close(fd);
            return -1;
        }
    }

    return 0;
}

static int
sendResponseRaw (const void *data, size_t dataSize, RILId id) {
    int fd = *s_SocketLinterParam1.s_fdCommand;
    int ret;
    uint32_t header;
    pthread_mutex_t * writeMutexHook = &s_writeMutex;

#ifdef MTK_GEMINI
    if (MTK_RIL_SOCKET_2 == id) {
        writeMutexHook = &s_writeMutex2;
        fd = *s_SocketLinterParam2.s_fdCommand;
    }
#if (MTK_GEMINI_SIM_NUM >= 3)
    if (MTK_RIL_SOCKET_3 == id) {
        writeMutexHook = &s_writeMutex3;
        fd = *s_SocketLinterParam3.s_fdCommand;
    }
#endif
#if (MTK_GEMINI_SIM_NUM >= 4)
    if (MTK_RIL_SOCKET_4 == id) {
        writeMutexHook = &s_writeMutex4;
        fd = *s_SocketLinterParam4.s_fdCommand;
    }
#endif
#endif /* MTK_GEMINI */

    if (fd < 0) {
        LOGE("sendResponseRaw, fd < 0, fd: %d", fd);
        return -1;
    }

    if (dataSize > MAX_COMMAND_BYTES) {
        if (MTK_RIL_SOCKET_1 == id) {
            LOGE("RIL1: packet larger than %u (%u)",
                 MAX_COMMAND_BYTES, (unsigned int )dataSize);
        }
#ifdef MTK_GEMINI
        else {
            LOGE("RIL%d: packet larger than %u (%u)", id + 1,
                 MAX_COMMAND_BYTES, (unsigned int )dataSize);
        }
#endif /* MTK_GEMINI */
        return -1;
    }

    /* LOG RIL response for debugging */
    printRILData(data,dataSize);

    pthread_mutex_lock(writeMutexHook);

    header = htonl(dataSize);

    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
#ifdef FATAL_ERROR_HANDLE
        if (ret == -1) { // Socket fd was closed, reopen it.                
            LOGD("socket%d error", (id == MTK_RIL_SOCKET_1 ?
				1 : 2));
            handleRILDFatalError();
        }
#endif
        return ret;
    }

    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(writeMutexHook);
#ifdef FATAL_ERROR_HANDLE
		if (ret == -1) { // Socket fd was closed, reopen it.				
			LOGD("socket%d error", (id == MTK_RIL_SOCKET_1 ?
				1 : 2));
			handleRILDFatalError();
		}
#endif
        return ret;
    }

    pthread_mutex_unlock(writeMutexHook);

    return 0;
}

static int
sendResponse (Parcel &p, RILId id) {
    printResponse;
    return sendResponseRaw(p.data(), p.dataSize(), id);
}

/* Convert "00 00" to "00 20" */
static int zero4_to_space(bytes_t  ucs2, int ucs2len) {
	int i, count = 0;

	LOGI("zero4_to_space\n");
	/* Ignore the last character */
	for (i = 0; i < (ucs2len - 2); i+=2) {
		if ((ucs2[i] == 0) && (ucs2[i+1] == 0)) {
			ucs2[i+1] = 0x20; /* Space character */
			count++;
		}
		LOGI("%d %d ", ucs2[i], ucs2[i+1]);
	}
    LOGI("\n");
	
	return count;
}

/** response is a char **, pointing to an array of char *'s */
static int
responseUssdStrings(Parcel &p, void *response, size_t responselen) {
    int         numStrings;
    const char* dcs = NULL;
    bytes_t     utf8String = NULL;
    bytes_t     hexData = NULL;
    int         len = 0, maxLen = 0;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof(char *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (response == NULL) {
        p.writeInt32 (0);
    } else {
        char **p_cur = (char **) response;

        numStrings = responselen / sizeof(char *);

        if (numStrings > 1) {

            /* the last string is for dcs reference */
            numStrings = 2;

            hexData = (bytes_t) calloc(strlen(p_cur[1]),sizeof(char));

            len = gsm_hex_to_bytes((cbytes_t) p_cur[1], strlen(p_cur[1]), hexData);

            dcs = p_cur[2];

            maxLen = (!strcmp(dcs,"UCS2")) ? len / 2 : len;
            if ((maxLen < 0) || (maxLen > MAX_RIL_USSD_STRING_LENGTH)) {
                LOGE("invalid ussd response length %d expected\n",maxLen);
                free(hexData);
                return RIL_ERRNO_INVALID_RESPONSE;
            } else {
                LOGI("Ussd string length:%d\n",maxLen);
            }
        }

        p.writeInt32 (numStrings);

        /* each string*/
        for (int i = 0 ; i < numStrings ; i++) {

            if (i == 1) {
                utf8String = (bytes_t) calloc(2*len+1,sizeof(char));

                /* The USS strings need to be transform to utf8 */
                if (!strcmp(dcs,"GSM7")) {
                    utf8_from_unpackedgsm7((cbytes_t) hexData, 0, len, utf8String);
                } else if (!strcmp(dcs,"UCS2")) {
                    /* Solve CR - [ALPS00268890][SW.Pisco][Pisco][USSD]Some character can't display when receive the ussd notification, mtk04070, 2012.05.03 */
                    zero4_to_space(hexData, len);
                    ucs2_to_utf8((cbytes_t) hexData, len/2, utf8String);
                }  else {
                    utf8_from_gsm8((cbytes_t) hexData, len, utf8String);
                }

                writeStringToParcel(p, (const char *) utf8String);

                free(hexData);
                free(utf8String);

            } else {

                writeStringToParcel (p, p_cur[i]);

            }
        }
    }
    return 0;
}

/** response is an int* pointing to an array of ints*/
static int
responseInts(Parcel &p, void *response, size_t responselen) {
    int numInts;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;

    numInts = responselen / sizeof(int *);
    p.writeInt32 (numInts);

    /* each int*/
    startResponse;
    for (int i = 0 ; i < numInts ; i++) {
        appendPrintBuf("%s%d,", printBuf, p_int[i]);
        p.writeInt32(p_int[i]);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/** response is a char **, pointing to an array of char *'s
    The parcel will begin with the version */
static int responseStringsWithVersion(int version, Parcel &p, void *response, size_t responselen) {
    p.writeInt32(version);
    return responseStrings(p, response, responselen);
}

/** response is a char **, pointing to an array of char *'s */
static int responseStrings(Parcel &p, void *response, size_t responselen) {
    int numStrings;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof(char *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (response == NULL) {
        p.writeInt32 (0);
    } else {
        char **p_cur = (char **) response;

        numStrings = responselen / sizeof(char *);
        p.writeInt32 (numStrings);

        /* each string*/
        startResponse;
        for (int i = 0 ; i < numStrings ; i++) {
            appendPrintBuf("%s%s,", printBuf, (char*)p_cur[i]);
            writeStringToParcel (p, p_cur[i]);
        }
        removeLastChar;
        closeResponse;
    }
    return 0;
}


/**
 * NULL strings are accepted
 * FIXME currently ignores responselen
 */
static int responseString(Parcel &p, void *response, size_t responselen) {
    /* one string only */
    startResponse;
    appendPrintBuf("%s%s", printBuf, (char*)response);
    closeResponse;

    writeStringToParcel(p, (const char *)response);

    return 0;
}

static int responseVoid(Parcel &p, void *response, size_t responselen) {
    startResponse;
    removeLastChar;
    return 0;
}

static int responseCallList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_Call *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_Call *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of call info's */
    num = responselen / sizeof(RIL_Call *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_Call *p_cur = ((RIL_Call **) response)[i];
        /* each call info */
        p.writeInt32(p_cur->state);
        p.writeInt32(p_cur->index);
        p.writeInt32(p_cur->toa);
        p.writeInt32(p_cur->isMpty);
        p.writeInt32(p_cur->isMT);
        p.writeInt32(p_cur->als);
        p.writeInt32(p_cur->isVoice);
        p.writeInt32(p_cur->isVoicePrivacy);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->numberPresentation);
        writeStringToParcel(p, p_cur->name);
        p.writeInt32(p_cur->namePresentation);
        // Remove when partners upgrade to version 3
        if ((s_callbacks.version < 3) || (p_cur->uusInfo == NULL || p_cur->uusInfo->uusData == NULL)) {
            p.writeInt32(0); /* UUS Information is absent */
        } else {
            RIL_UUS_Info *uusInfo = p_cur->uusInfo;
            p.writeInt32(1); /* UUS Information is present */
            p.writeInt32(uusInfo->uusType);
            p.writeInt32(uusInfo->uusDcs);
            p.writeInt32(uusInfo->uusLength);
            p.write(uusInfo->uusData, uusInfo->uusLength);
        }
        appendPrintBuf("%s[id=%d,%s,toa=%d,",
                       printBuf,
                       p_cur->index,
                       callStateToString(p_cur->state),
                       p_cur->toa);
        appendPrintBuf("%s%s,%s,als=%d,%s,%s,",
                       printBuf,
                       (p_cur->isMpty)?"conf":"norm",
                       (p_cur->isMT)?"mt":"mo",
                       p_cur->als,
                       (p_cur->isVoice)?"voc":"nonvoc",
                       (p_cur->isVoicePrivacy)?"evp":"noevp");
        appendPrintBuf("%s%s,cli=%d,name='%s',%d]",
                       printBuf,
                       p_cur->number,
                       p_cur->numberPresentation,
                       p_cur->name,
                       p_cur->namePresentation);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseSMS(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Response) ) {
        LOGE("invalid response length %d expected %d",
             (int)responselen, (int)sizeof (RIL_SMS_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SMS_Response *p_cur = (RIL_SMS_Response *) response;

    p.writeInt32(p_cur->messageRef);
    writeStringToParcel(p, p_cur->ackPDU);
    p.writeInt32(p_cur->errorCode);

    startResponse;
    appendPrintBuf("%s%d,%s,%d", printBuf, p_cur->messageRef,
                   (char*)p_cur->ackPDU, p_cur->errorCode);
    closeResponse;

    return 0;
}

static void dispatchSmsParams(Parcel &p, RequestInfo *pRI) {
    RIL_SmsParams smsParams;
    int32_t t;
    status_t status;
    
    LOGD("dispatchSmsParams Enter.");

    memset(&smsParams, 0, sizeof(smsParams));
    
    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    
    status = p.readInt32(&t);
    smsParams.format = t;
    if (status != NO_ERROR) {
        goto invalid;
    }
    
    status = p.readInt32(&t);
    smsParams.vp = t;
    if (status != NO_ERROR) {
        goto invalid;
    }
    
    status = p.readInt32(&t);
    smsParams.pid = t;
    if (status != NO_ERROR) {
        goto invalid;
    }
    
    status = p.readInt32(&t);
    smsParams.dcs = t;
    if (status != NO_ERROR) {
        goto invalid;
    }
    
    LOGD("dispatchSmsParams format: %d", smsParams.format);
    LOGD("dispatchSmsParams vp: %d", smsParams.vp);
    LOGD("dispatchSmsParams pid: %d", smsParams.pid);
    LOGD("dispatchSmsParams dcs: %d", smsParams.dcs);

    LOGD("dispatchSmsParams Send Request..");
    
    startRequest;
    appendPrintBuf("%sformat=%d,vp=%d,pid=%d,dcs=%d", printBuf,
            smsParams.format, smsParams.vp, smsParams.pid, smsParams.dcs);
    closeRequest;
    
    s_callbacks.onRequest(pRI->pCI->requestNumber, &smsParams, sizeof(smsParams), pRI);
    
#ifdef MEMSET_FREED
    memset(&smsParams, 0, sizeof(smsParams));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static int responseSmsParams(Parcel &p, void *response, size_t responselen) {
    if(response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    
    if(responselen != (int)sizeof(RIL_SmsParams)) {
        LOGE("invalid response length %d expected %d",
             (int)responselen, (int)sizeof(RIL_SmsParams));
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    
    RIL_SmsParams *p_cur = (RIL_SmsParams *)response;
    p.writeInt32(p_cur->format);
    p.writeInt32(p_cur->vp);
    p.writeInt32(p_cur->pid);
    p.writeInt32(p_cur->dcs);
    
    startResponse;
    appendPrintBuf("%s%d,%d,%d,%d", printBuf, p_cur->format, p_cur->vp,
                   p_cur->pid, p_cur->dcs);
    closeResponse;
    
    return 0;
}

static int responseDataCallListV4(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v4) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v4));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_Data_Call_Response_v4);
    p.writeInt32(num);

    RIL_Data_Call_Response_v4 *p_cur = (RIL_Data_Call_Response_v4 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        // apn is not used, so don't send.
        writeStringToParcel(p, p_cur[i].address);
        appendPrintBuf("%s[cid=%d,%s,%s,%s],", printBuf,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].address);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseDataCallList(Parcel &p, void *response, size_t responselen)
{
    // Write version
    p.writeInt32(s_callbacks.version);

    if (s_callbacks.version < 5) {
        return responseDataCallListV4(p, response, responselen);
    } else {
        if (response == NULL && responselen != 0) {
            LOGE("invalid response: NULL");
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        if (responselen % sizeof(RIL_Data_Call_Response_v6) != 0) {
            LOGE("invalid response length %d expected multiple of %d",
                    (int)responselen, (int)sizeof(RIL_Data_Call_Response_v6));
            return RIL_ERRNO_INVALID_RESPONSE;
        }

        int num = responselen / sizeof(RIL_Data_Call_Response_v6);
        p.writeInt32(num);

        RIL_Data_Call_Response_v6 *p_cur = (RIL_Data_Call_Response_v6 *) response;
        startResponse;
        int i;
        for (i = 0; i < num; i++) {
            p.writeInt32((int)p_cur[i].status);
            p.writeInt32(p_cur[i].suggestedRetryTime);
            p.writeInt32(p_cur[i].cid);
            p.writeInt32(p_cur[i].active);
            writeStringToParcel(p, p_cur[i].type);
            writeStringToParcel(p, p_cur[i].ifname);
            writeStringToParcel(p, p_cur[i].addresses);
            writeStringToParcel(p, p_cur[i].dnses);
            writeStringToParcel(p, p_cur[i].gateways);
            appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%d,%s,%s,%s],", printBuf,
                p_cur[i].status,
                p_cur[i].suggestedRetryTime,
                p_cur[i].cid,
                (p_cur[i].active==0)?"down":"up",
                (char*)p_cur[i].ifname,
                (char*)p_cur[i].addresses,
                (char*)p_cur[i].dnses,
                (char*)p_cur[i].gateways);
        }
        removeLastChar;
        closeResponse;
    }

    return 0;
}

static int responseSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    LOGD("s_callbacks.version %d", s_callbacks.version);
    if (s_callbacks.version < 5) {
        return responseStringsWithVersion(s_callbacks.version, p, response, responselen);
    } else {
        return responseDataCallList(p, response, responselen);
    }
}

static int responseRaw(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL with responselen != 0");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // The java code reads -1 size as null byte array
    if (response == NULL) {
        p.writeInt32(-1);
    } else {
        p.writeInt32(responselen);
        p.write(response, responselen);
    }

    return 0;
}


static int responseSIM_IO(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SIM_IO_Response) ) {
        LOGE("invalid response length was %d expected %d",
             (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SIM_IO_Response *p_cur = (RIL_SIM_IO_Response *) response;
    p.writeInt32(p_cur->sw1);
    p.writeInt32(p_cur->sw2);
    writeStringToParcel(p, p_cur->simResponse);

    startResponse;
    appendPrintBuf("%ssw1=0x%X,sw2=0x%X,%s", printBuf, p_cur->sw1, p_cur->sw2,
                   (char*)p_cur->simResponse);
    closeResponse;


    return 0;
}

static int responseCallForwards(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CallForwardInfo *) != 0) {
        LOGE("invalid response length %d expected multiple of %d",
             (int)responselen, (int)sizeof(RIL_CallForwardInfo *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    /* number of call info's */
    num = responselen / sizeof(RIL_CallForwardInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++) {
        RIL_CallForwardInfo *p_cur = ((RIL_CallForwardInfo **) response)[i];

        p.writeInt32(p_cur->status);
        p.writeInt32(p_cur->reason);
        p.writeInt32(p_cur->serviceClass);
        p.writeInt32(p_cur->toa);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->timeSeconds);
        appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d],", printBuf,
                       (p_cur->status==1)?"enable":"disable",
                       p_cur->reason, p_cur->serviceClass, p_cur->toa,
                       (char*)p_cur->number,
                       p_cur->timeSeconds);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseSsn(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_SuppSvcNotification)) {
        LOGE("invalid response length was %d expected %d",
             (int)responselen, (int)sizeof (RIL_SuppSvcNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SuppSvcNotification *p_cur = (RIL_SuppSvcNotification *) response;
    p.writeInt32(p_cur->notificationType);
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->index);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);

    startResponse;
    appendPrintBuf("%s%s,code=%d,id=%d,type=%d,%s", printBuf,
                   (p_cur->notificationType==0)?"mo":"mt",
                   p_cur->code, p_cur->index, p_cur->type,
                   (char*)p_cur->number);
    closeResponse;

    return 0;
}

static int responseCrssN(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CrssNotification)) {
        LOGE("invalid response length was %d expected %d",
             (int)responselen, (int)sizeof (RIL_CrssNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CrssNotification *p_cur = (RIL_CrssNotification *) response;
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);
    writeStringToParcel(p, p_cur->alphaid);
    p.writeInt32(p_cur->cli_validity);
    return 0;
}


static int responseCellList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_NeighboringCell *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_NeighboringCell *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of records */
    num = responselen / sizeof(RIL_NeighboringCell *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_NeighboringCell *p_cur = ((RIL_NeighboringCell **) response)[i];

        p.writeInt32(p_cur->rssi);
        writeStringToParcel (p, p_cur->cid);

        appendPrintBuf("%s[cid=%s,rssi=%d],", printBuf,
                       p_cur->cid, p_cur->rssi);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/**
 * Marshall the signalInfoRecord into the parcel if it exists.
 */
static void marshallSignalInfoRecord(Parcel &p,
                                     RIL_CDMA_SignalInfoRecord &p_signalInfoRecord) {
    p.writeInt32(p_signalInfoRecord.isPresent);
    p.writeInt32(p_signalInfoRecord.signalType);
    p.writeInt32(p_signalInfoRecord.alertPitch);
    p.writeInt32(p_signalInfoRecord.signal);
}

static int responseCdmaInformationRecords(Parcel &p,
        void *response, size_t responselen) {
    int num;
    char* string8 = NULL;
    int buffer_lenght;
    RIL_CDMA_InformationRecord *infoRec;
    int index = 0;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_InformationRecords)) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_CDMA_InformationRecords *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_InformationRecords *p_cur =
        (RIL_CDMA_InformationRecords *) response;
    num = MIN(p_cur->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);

    startResponse;
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        infoRec = &p_cur->infoRec[i];
        p.writeInt32(infoRec->name);
        switch (infoRec->name) {
        case RIL_CDMA_DISPLAY_INFO_REC:
        case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC:
            if (infoRec->rec.display.alpha_len >
                    CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                     (int)infoRec->rec.display.alpha_len,
                     CDMA_ALPHA_INFO_BUFFER_LENGTH);
                return RIL_ERRNO_INVALID_RESPONSE;
            }
            string8 = (char*) malloc((infoRec->rec.display.alpha_len + 1)
                                     * sizeof(char) );
            for (int i = 0 ; i < infoRec->rec.display.alpha_len ; i++) {
                string8[i] = infoRec->rec.display.alpha_buf[i];
            }

            index = (int) infoRec->rec.display.alpha_len;
            string8[index] = '\0';

            writeStringToParcel(p, (const char*)string8);
            free(string8);
            string8 = NULL;
            break;
        case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
        case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
        case RIL_CDMA_CONNECTED_NUMBER_INFO_REC:
            if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                     (int)infoRec->rec.number.len,
                     CDMA_NUMBER_INFO_BUFFER_LENGTH);
                return RIL_ERRNO_INVALID_RESPONSE;
            }
            string8 = (char*) malloc((infoRec->rec.number.len + 1)
                                     * sizeof(char) );
            for (int i = 0 ; i < infoRec->rec.number.len; i++) {
                string8[i] = infoRec->rec.number.buf[i];
            }
            index = (int) infoRec->rec.number.len;
            string8[index] = '\0';

            writeStringToParcel(p, (const char*)string8);
            free(string8);
            string8 = NULL;
            p.writeInt32(infoRec->rec.number.number_type);
            p.writeInt32(infoRec->rec.number.number_plan);
            p.writeInt32(infoRec->rec.number.pi);
            p.writeInt32(infoRec->rec.number.si);
            break;
        case RIL_CDMA_SIGNAL_INFO_REC:
            p.writeInt32(infoRec->rec.signal.isPresent);
            p.writeInt32(infoRec->rec.signal.signalType);
            p.writeInt32(infoRec->rec.signal.alertPitch);
            p.writeInt32(infoRec->rec.signal.signal);

            appendPrintBuf("%sisPresent=%X, signalType=%X, \
                                alertPitch=%X, signal=%X, ",
                           printBuf, (int)infoRec->rec.signal.isPresent,
                           (int)infoRec->rec.signal.signalType,
                           (int)infoRec->rec.signal.alertPitch,
                           (int)infoRec->rec.signal.signal);
            removeLastChar;
            break;
        case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC:
            if (infoRec->rec.redir.redirectingNumber.len >
                    CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                LOGE("invalid display info response length %d \
                          expected not more than %d\n",
                     (int)infoRec->rec.redir.redirectingNumber.len,
                     CDMA_NUMBER_INFO_BUFFER_LENGTH);
                return RIL_ERRNO_INVALID_RESPONSE;
            }
            string8 = (char*) malloc((infoRec->rec.redir.redirectingNumber
                                      .len + 1) * sizeof(char) );
            for (int i = 0;
                    i < infoRec->rec.redir.redirectingNumber.len;
                    i++) {
                string8[i] = infoRec->rec.redir.redirectingNumber.buf[i];
            }

            index = (int) infoRec->rec.redir.redirectingNumber.len;
            string8[index] = '\0';

            writeStringToParcel(p, (const char*)string8);
            free(string8);
            string8 = NULL;
            p.writeInt32(infoRec->rec.redir.redirectingNumber.number_type);
            p.writeInt32(infoRec->rec.redir.redirectingNumber.number_plan);
            p.writeInt32(infoRec->rec.redir.redirectingNumber.pi);
            p.writeInt32(infoRec->rec.redir.redirectingNumber.si);
            p.writeInt32(infoRec->rec.redir.redirectingReason);
            break;
        case RIL_CDMA_LINE_CONTROL_INFO_REC:
            p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPolarityIncluded);
            p.writeInt32(infoRec->rec.lineCtrl.lineCtrlToggle);
            p.writeInt32(infoRec->rec.lineCtrl.lineCtrlReverse);
            p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPowerDenial);

            appendPrintBuf("%slineCtrlPolarityIncluded=%d, \
                                lineCtrlToggle=%d, lineCtrlReverse=%d, \
                                lineCtrlPowerDenial=%d, ", printBuf,
                           (int)infoRec->rec.lineCtrl.lineCtrlPolarityIncluded,
                           (int)infoRec->rec.lineCtrl.lineCtrlToggle,
                           (int)infoRec->rec.lineCtrl.lineCtrlReverse,
                           (int)infoRec->rec.lineCtrl.lineCtrlPowerDenial);
            removeLastChar;
            break;
        case RIL_CDMA_T53_CLIR_INFO_REC:
            p.writeInt32((int)(infoRec->rec.clir.cause));

            appendPrintBuf("%scause%d", printBuf, infoRec->rec.clir.cause);
            removeLastChar;
            break;
        case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC:
            p.writeInt32(infoRec->rec.audioCtrl.upLink);
            p.writeInt32(infoRec->rec.audioCtrl.downLink);

            appendPrintBuf("%supLink=%d, downLink=%d, ", printBuf,
                           infoRec->rec.audioCtrl.upLink,
                           infoRec->rec.audioCtrl.downLink);
            removeLastChar;
            break;
        case RIL_CDMA_T53_RELEASE_INFO_REC:
            // TODO(Moto): See David Krause, he has the answer:)
            LOGE("RIL_CDMA_T53_RELEASE_INFO_REC: return INVALID_RESPONSE");
            return RIL_ERRNO_INVALID_RESPONSE;
        default:
            LOGE("Incorrect name value");
            return RIL_ERRNO_INVALID_RESPONSE;
        }
    }
    closeResponse;

    return 0;
}

static int responseRilSignalStrength(Parcel &p,
                                     void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen == sizeof (RIL_SignalStrength_v6)) {
        // New RIL
        RIL_SignalStrength_v6 *p_cur = ((RIL_SignalStrength_v6 *) response);

        p.writeInt32(p_cur->GW_SignalStrength.signalStrength);
        p.writeInt32(p_cur->GW_SignalStrength.bitErrorRate);
        p.writeInt32(p_cur->CDMA_SignalStrength.dbm);
        p.writeInt32(p_cur->CDMA_SignalStrength.ecio);
        p.writeInt32(p_cur->EVDO_SignalStrength.dbm);
        p.writeInt32(p_cur->EVDO_SignalStrength.ecio);
        p.writeInt32(p_cur->EVDO_SignalStrength.signalNoiseRatio);

        startResponse;
        appendPrintBuf("%s[signalStrength=%d,bitErrorRate=%d,\
                CDMA_SignalStrength.dbm=%d,CDMA_SignalStrength.ecio=%d,\
                EVDO_SignalStrength.dbm =%d,EVDO_SignalStrength.ecio=%d,\
                EVDO_SignalStrength.signalNoiseRatio=%d]",
                       printBuf,
                       p_cur->GW_SignalStrength.signalStrength,
                       p_cur->GW_SignalStrength.bitErrorRate,
                       p_cur->CDMA_SignalStrength.dbm,
                       p_cur->CDMA_SignalStrength.ecio,
                       p_cur->EVDO_SignalStrength.dbm,
                       p_cur->EVDO_SignalStrength.ecio,
                       p_cur->EVDO_SignalStrength.signalNoiseRatio);

        closeResponse;

    } else if (responselen % sizeof (int) == 0) {
        // Old RIL deprecated
        int *p_cur = (int *) response;

        startResponse;

        // With the Old RIL we see one or 2 integers.
        size_t num = responselen / sizeof (int); // Number of integers from ril
        size_t totalIntegers = 7; // Number of integers in RIL_SignalStrength
        size_t i;

        appendPrintBuf("%s[", printBuf);
        for (i = 0; i < num; i++) {
            appendPrintBuf("%s %d", printBuf, *p_cur);
            p.writeInt32(*p_cur++);
        }
        appendPrintBuf("%s]", printBuf);

        // Fill the remainder with zero's.
        for (; i < totalIntegers; i++) {
            p.writeInt32(0);
        }

        closeResponse;
    } else {
        LOGE("invalid response length");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    return 0;
}

static int responseCallRing(Parcel &p, void *response, size_t responselen) {
    if ((response == NULL) || (responselen == 0)) {
        return responseVoid(p, response, responselen);
    } else {
        return responseCdmaSignalInfoRecord(p, response, responselen);
    }
}

static int responseCdmaSignalInfoRecord(Parcel &p, void *response, size_t responselen) {
    if (response == NULL || responselen == 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_SignalInfoRecord)) {
        LOGE("invalid response length %d expected sizeof (RIL_CDMA_SignalInfoRecord) of %d\n",
             (int)responselen, (int)sizeof (RIL_CDMA_SignalInfoRecord));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_CDMA_SignalInfoRecord *p_cur = ((RIL_CDMA_SignalInfoRecord *) response);
    marshallSignalInfoRecord(p, *p_cur);

    appendPrintBuf("%s[isPresent=%d,signalType=%d,alertPitch=%d\
              signal=%d]",
                   printBuf,
                   p_cur->isPresent,
                   p_cur->signalType,
                   p_cur->alertPitch,
                   p_cur->signal);

    closeResponse;
    return 0;
}

static int responseCdmaCallWaiting(Parcel &p, void *response,
                                   size_t responselen) {
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CDMA_CallWaiting_v6)) {
        LOGE("invalid response length %d expected %d\n",
             (int)responselen, (int)sizeof(RIL_CDMA_CallWaiting_v6));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RIL_CDMA_CallWaiting_v6 *p_cur = ((RIL_CDMA_CallWaiting_v6 *) response);

    writeStringToParcel (p, p_cur->number);
    p.writeInt32(p_cur->numberPresentation);
    writeStringToParcel (p, p_cur->name);
    marshallSignalInfoRecord(p, p_cur->signalInfoRecord);

    appendPrintBuf("%snumber=%s,numberPresentation=%d, name=%s,\
            signalInfoRecord[isPresent=%d,signalType=%d,alertPitch=%d\
            signal=%d]",
                   printBuf,
                   p_cur->number,
                   p_cur->numberPresentation,
                   p_cur->name,
                   p_cur->signalInfoRecord.isPresent,
                   p_cur->signalInfoRecord.signalType,
                   p_cur->signalInfoRecord.alertPitch,
                   p_cur->signalInfoRecord.signal);

    closeResponse;

    return 0;
}

static int responseCellInfoList(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RLOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CellInfo) != 0) {
        RLOGE("invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CellInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_CellInfo);
    p.writeInt32(num);
    LOGD("responseCellInfoList num= %d ",num);


    RIL_CellInfo** res = NULL; 
    res = (RIL_CellInfo**) response;
    RIL_CellInfo *p_cur = (RIL_CellInfo*)res[0];
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p_cur->timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
        p_cur->timeStamp = elapsedRealtime() * 1000000; // Time milliseconds since bootup, convert to nano seconds
		
        appendPrintBuf("%s[%d: type=%d,registered=%d,timeStampType=%d,timeStamp=%lld", printBuf, i,
            p_cur->cellInfoType, p_cur->registered, p_cur->timeStampType, p_cur->timeStamp);
        p.writeInt32((int)p_cur->cellInfoType);
        p.writeInt32(p_cur->registered);
        p.writeInt32(p_cur->timeStampType);
        p.writeInt64(p_cur->timeStamp);
        switch(p_cur->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                appendPrintBuf("%s GSM id: mcc=%d,mnc=%d,lac=%d,cid=%d,", printBuf,
                    p_cur->CellInfo.gsm.cellIdentityGsm.mcc,
                    p_cur->CellInfo.gsm.cellIdentityGsm.mnc,
                    p_cur->CellInfo.gsm.cellIdentityGsm.lac,
                    p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                appendPrintBuf("%s gsmSS: ss=%d,ber=%d],", printBuf,
                    p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength,
                    p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);

                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mcc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mnc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.lac);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                appendPrintBuf("%s WCDMA id: mcc=%d,mnc=%d,lac=%d,cid=%d,psc=%d,", printBuf,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.lac,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.cid,
                    p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                appendPrintBuf("%s wcdmaSS: ss=%d,ber=%d],", printBuf,
                    p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength,
                    p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);

                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.lac);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.cid);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_CDMA: {
                appendPrintBuf("%s CDMA id: nId=%d,sId=%d,bsId=%d,long=%d,lat=%d", printBuf,
                    p_cur->CellInfo.cdma.cellIdentityCdma.networkId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.systemId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.basestationId,
                    p_cur->CellInfo.cdma.cellIdentityCdma.longitude,
                    p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.networkId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.systemId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.basestationId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.longitude);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                appendPrintBuf("%s cdmaSS: dbm=%d ecio=%d evdoSS: dbm=%d,ecio=%d,snr=%d", printBuf,
                    p_cur->CellInfo.cdma.signalStrengthCdma.dbm,
                    p_cur->CellInfo.cdma.signalStrengthCdma.ecio,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.dbm,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.ecio,
                    p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);

                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                break;
            }
            case RIL_CELL_INFO_TYPE_LTE: {
                appendPrintBuf("%s LTE id: mcc=%d,mnc=%d,ci=%d,pci=%d,tac=%d", printBuf,
                    p_cur->CellInfo.lte.cellIdentityLte.mcc,
                    p_cur->CellInfo.lte.cellIdentityLte.mnc,
                    p_cur->CellInfo.lte.cellIdentityLte.ci,
                    p_cur->CellInfo.lte.cellIdentityLte.pci,
                    p_cur->CellInfo.lte.cellIdentityLte.tac);

                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mcc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mnc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.ci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.pci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.tac);

                appendPrintBuf("%s lteSS: ss=%d,rsrp=%d,rsrq=%d,rssnr=%d,cqi=%d,ta=%d", printBuf,
                    p_cur->CellInfo.lte.signalStrengthLte.signalStrength,
                    p_cur->CellInfo.lte.signalStrengthLte.rsrp,
                    p_cur->CellInfo.lte.signalStrengthLte.rsrq,
                    p_cur->CellInfo.lte.signalStrengthLte.rssnr,
                    p_cur->CellInfo.lte.signalStrengthLte.cqi,
                    p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.signalStrength);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrp);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrq);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rssnr);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.cqi);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                break;
            }
        }
        p_cur = (RIL_CellInfo*) res[i+1];
    }
    removeLastChar;
    closeResponse;

    return 0;
}

static int responseSimStatus(Parcel &p, void *response, size_t responselen) {
    int i;

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_CardStatus_v6 *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_CardStatus_v6 *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CardStatus_v6 *p_cur = ((RIL_CardStatus_v6 *) response);

    p.writeInt32(p_cur->card_state);
    p.writeInt32(p_cur->universal_pin_state);
    p.writeInt32(p_cur->gsm_umts_subscription_app_index);
    p.writeInt32(p_cur->cdma_subscription_app_index);
    p.writeInt32(p_cur->ims_subscription_app_index);
    p.writeInt32(p_cur->num_applications);

    startResponse;
    for (i = 0; i < p_cur->num_applications; i++) {
        p.writeInt32(p_cur->applications[i].app_type);
        p.writeInt32(p_cur->applications[i].app_state);
        p.writeInt32(p_cur->applications[i].perso_substate);
        writeStringToParcel(p, (const char*)(p_cur->applications[i].aid_ptr));
        writeStringToParcel(p, (const char*)
                            (p_cur->applications[i].app_label_ptr));
        p.writeInt32(p_cur->applications[i].pin1_replaced);
        p.writeInt32(p_cur->applications[i].pin1);
        p.writeInt32(p_cur->applications[i].pin2);
        appendPrintBuf("%s[app_type=%d,app_state=%d,perso_substate=%d,\
                aid_ptr=%s,app_label_ptr=%s,pin1_replaced=%d,pin1=%d,pin2=%d],",
                       printBuf,
                       p_cur->applications[i].app_type,
                       p_cur->applications[i].app_state,
                       p_cur->applications[i].perso_substate,
                       p_cur->applications[i].aid_ptr,
                       p_cur->applications[i].app_label_ptr,
                       p_cur->applications[i].pin1_replaced,
                       p_cur->applications[i].pin1,
                       p_cur->applications[i].pin2);
    }
    closeResponse;

    return 0;
}

static int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    int num = responselen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    RIL_GSM_BroadcastSmsConfigInfo **p_cur =
        (RIL_GSM_BroadcastSmsConfigInfo **) response;
    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->fromServiceId);
        p.writeInt32(p_cur[i]->toServiceId);
        p.writeInt32(p_cur[i]->fromCodeScheme);
        p.writeInt32(p_cur[i]->toCodeScheme);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId=%d, \
                fromCodeScheme=%d, toCodeScheme=%d, selected =%d]",
                       printBuf, i, p_cur[i]->fromServiceId, p_cur[i]->toServiceId,
                       p_cur[i]->fromCodeScheme, p_cur[i]->toCodeScheme,
                       p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

static int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    RIL_CDMA_BroadcastSmsConfigInfo **p_cur =
        (RIL_CDMA_BroadcastSmsConfigInfo **) response;

    int num = responselen / sizeof (RIL_CDMA_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++ ) {
        p.writeInt32(p_cur[i]->service_category);
        p.writeInt32(p_cur[i]->language);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: srvice_category=%d, language =%d, \
              selected =%d], ",
                       printBuf, i, p_cur[i]->service_category, p_cur[i]->language,
                       p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

static int responseCdmaSms(Parcel &p, void *response, size_t responselen) {
    int num;
    int digitCount;
    int digitLimit;
    uint8_t uct;
    void* dest;

    LOGD("Inside responseCdmaSms");

    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CDMA_SMS_Message)) {
        LOGE("invalid response length was %d expected %d",
             (int)responselen, (int)sizeof(RIL_CDMA_SMS_Message));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_SMS_Message *p_cur = (RIL_CDMA_SMS_Message *) response;
    p.writeInt32(p_cur->uTeleserviceID);
    p.write(&(p_cur->bIsServicePresent),sizeof(uct));
    p.writeInt32(p_cur->uServicecategory);
    p.writeInt32(p_cur->sAddress.digit_mode);
    p.writeInt32(p_cur->sAddress.number_mode);
    p.writeInt32(p_cur->sAddress.number_type);
    p.writeInt32(p_cur->sAddress.number_plan);
    p.write(&(p_cur->sAddress.number_of_digits), sizeof(uct));
    digitLimit= MIN((p_cur->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sAddress.digits[digitCount]),sizeof(uct));
    }

    p.writeInt32(p_cur->sSubAddress.subaddressType);
    p.write(&(p_cur->sSubAddress.odd),sizeof(uct));
    p.write(&(p_cur->sSubAddress.number_of_digits),sizeof(uct));
    digitLimit= MIN((p_cur->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sSubAddress.digits[digitCount]),sizeof(uct));
    }

    digitLimit= MIN((p_cur->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    p.writeInt32(p_cur->uBearerDataLen);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->aBearerData[digitCount]), sizeof(uct));
    }

    startResponse;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.number_mode=%d, sAddress.number_type=%d, ",
                   printBuf, p_cur->uTeleserviceID,p_cur->bIsServicePresent,p_cur->uServicecategory,
                   p_cur->sAddress.digit_mode, p_cur->sAddress.number_mode,p_cur->sAddress.number_type);
    closeResponse;

    return 0;
}

static int responsePhbEntries(Parcel &p,void *response, size_t responselen) {


    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_PhbEntryStrucutre *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_PhbEntryStrucutre *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_PhbEntryStrucutre *);
    p.writeInt32(num);

    startResponse;
    RIL_PhbEntryStrucutre **p_cur =
        (RIL_PhbEntryStrucutre **) response;
    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->type);
        p.writeInt32(p_cur[i]->index);
        writeStringToParcel(p, p_cur[i]->number);
        p.writeInt32(p_cur[i]->ton);
        writeStringToParcel(p, p_cur[i]->alphaId);

        appendPrintBuf("%s [%d: type=%d, index=%d, \
                number=%s, ton=%d, alphaId =%s]",
                       printBuf, i, p_cur[i]->type, p_cur[i]->index,
                       p_cur[i]->number, p_cur[i]->ton,
                       p_cur[i]->alphaId);
    }
    closeResponse;

    return 0;

}

static int responseReadPhbEntryExt(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof (RIL_PHB_ENTRY *) != 0) {
        LOGE("invalid response length %d expected multiple of %d\n",
             (int)responselen, (int)sizeof (RIL_PHB_ENTRY *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    int num = responselen / sizeof(RIL_PHB_ENTRY *);
    p.writeInt32(num);
    startResponse;
    RIL_PHB_ENTRY **p_cur =
        (RIL_PHB_ENTRY **) response;

    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->index);
        writeStringToParcel(p, p_cur[i]->number);
        p.writeInt32(p_cur[i]->type);
        writeStringToParcel(p, p_cur[i]->text);
        p.writeInt32(p_cur[i]->hidden);
        writeStringToParcel(p, p_cur[i]->group);
        writeStringToParcel(p, p_cur[i]->adnumber);
        p.writeInt32(p_cur[i]->adtype);
        writeStringToParcel(p, p_cur[i]->secondtext);
        writeStringToParcel(p, p_cur[i]->email);

        appendPrintBuf("%s [%d: index=%d, \
                number=%s, type=%d, text =%s, hidden=%d,group=%s,adnumber=%s,adtype=%d,sectext=%s,email=%s]",
                       printBuf, i,  p_cur[i]->index,
                       p_cur[i]->number, p_cur[i]->type,
                       p_cur[i]->text, p_cur[i]->hidden,p_cur[i]->group,p_cur[i]->adnumber,
                       p_cur[i]->adtype,p_cur[i]->secondtext,p_cur[i]->email);
    }

    closeResponse;

    return 0;

}


static int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL || responselen == 0) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Memory_Status)) {
        LOGE("invalid response length %d expected sizeof (RIL_SMS_Memory_Status) of %d\n",
             (int)responselen, (int)sizeof(RIL_SMS_Memory_Status));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_SMS_Memory_Status *mem_status = (RIL_SMS_Memory_Status*)response;

    p.writeInt32(mem_status->used);
    p.writeInt32(mem_status->total);

    appendPrintBuf("%s [used = %d, total = %d]", printBuf, mem_status->used, mem_status->total);

    closeResponse;

    return 0;
}

static int responseGetPhbMemStorage(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL || responselen == 0) {
        LOGE("responseGetPhbMemStorage invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen != sizeof (RIL_PHB_MEM_STORAGE_RESPONSE)) {
        LOGE("invalid response length %d expected sizeof (RIL_PHB_MEM_STORAGE_RESPONSE) of %d\n",
             (int)responselen, (int)sizeof(RIL_PHB_MEM_STORAGE_RESPONSE));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_PHB_MEM_STORAGE_RESPONSE *mem_status = (RIL_PHB_MEM_STORAGE_RESPONSE*)response;
    writeStringToParcel (p, mem_status->storage);
    p.writeInt32(mem_status->used);
    p.writeInt32(mem_status->total);

    appendPrintBuf("%s [storage = %s, used = %d, total = %d]", printBuf, mem_status->storage, mem_status->used, mem_status->total);

    closeResponse;
    return 0;
}

static int responseCbConfigInfo(Parcel &p, void *response, size_t responselen) {
    if(NULL == response) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if(responselen != sizeof(RIL_CBConfigInfo)) {
        LOGE("invalid response length %d expected %d",
            responselen, sizeof(RIL_CBConfigInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CBConfigInfo *p_cur = (RIL_CBConfigInfo *)response;
    p.writeInt32(p_cur->mode);
    writeStringToParcel(p, p_cur->channelConfigInfo);
    writeStringToParcel(p, p_cur->languageConfigInfo);
    p.writeInt32(p_cur->isAllLanguageOn);
    
    startResponse;
    appendPrintBuf("%s%d,%s,%s,%d", printBuf, p_cur->mode, p_cur->channelConfigInfo,
                   p_cur->languageConfigInfo, p_cur->isAllLanguageOn);
    closeResponse;
    
    return 0;
}

static int responseEtwsNotification(Parcel &p, void *response, size_t responselen) {
    if(NULL == response) {
        LOGE("invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    
    if(responselen != sizeof(RIL_CBEtwsNotification)) {
        LOGE("invalid response length %d expected %d",
            responselen, sizeof(RIL_CBEtwsNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    
    RIL_CBEtwsNotification *p_cur = (RIL_CBEtwsNotification *)response;
    p.writeInt32(p_cur->warningType);
    p.writeInt32(p_cur->messageId);
    p.writeInt32(p_cur->serialNumber);
    writeStringToParcel(p, p_cur->plmnId);
    writeStringToParcel(p, p_cur->securityInfo);
    
    startResponse;
    appendPrintBuf("%s%d,%d,%d,%s,%s", printBuf, p_cur->waringType, p_cur->messageId,
                   p_cur->serialNumber, p_cur->plmnId, p_cur->securityInfo);
    closeResponse;
    
    return 0;
}

//MTK-START [mtk80776] WiFi Calling
static void dispatchUiccIo(Parcel &p, RequestInfo *pRI)
{
    RIL_UICC_IO_v6 uiccIo;
    int32_t t;
    status_t status;

    memset(&uiccIo, 0, sizeof(uiccIo));

    status = p.readInt32(&t);
    uiccIo.sessionId = (int)t;

    status = p.readInt32(&t);
    uiccIo.command = (int)t;

    status = p.readInt32(&t);
    uiccIo.fileId = (int)t;

    uiccIo.path = strdupReadString(p);

    status = p.readInt32(&t);
    uiccIo.p1 = (int)t;

    status = p.readInt32(&t);
    uiccIo.p2 = (int)t;

    status = p.readInt32(&t);
    uiccIo.p3 = (int)t;

    uiccIo.data = strdupReadString(p);
    uiccIo.pin2 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s sessionId=0x%X, cmd=0x%x, efid=0x%x path=%s,%d,%d,%d,%s,pin2=%s", printBuf,
            uiccIo.sessionId, uiccIo.command, uiccIo.fileId, (char *)uiccIo.path,
            uiccIo.p1, uiccIo.p2, uiccIo.p3, (char *)uiccIo.data, (char *)uiccIo.pin2);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->reqeustNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber, &uiccIo, sizeof(RIL_UICC_IO_v6), pRI);

#ifdef MEMSET_FREED
    memsetString(uiccIo.path);
    memsetString(uiccIo.data);
    memsetString(uiccIo.pin2);
#endif

    free(uiccIo.path);
    free(uiccIo.data);
    free(uiccIo.pin2);

#ifdef MEMSET_FREED
    memset(&uiccIo, 0, sizeof(RIL_UICC_IO_v6));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

static void dispatchUiccAuthentication(Parcel &p, RequestInfo *pRI)
{
    RIL_UICC_Authentication uiccAuth;
    int32_t t;
    int32_t len;
    status_t status;

    LOGD("dispatchUiccAuthentication Enter...");

    memset(&uiccAuth, 0, sizeof(uiccAuth));

    status = p.readInt32(&t);
    uiccAuth.session_id = (int)t;

    uiccAuth.context1 = strdupReadString(p);
    uiccAuth.context2 = strdupReadString(p);

    startRequest;
    appendPrintBuf("%ssessionId=%d, context1=%s, context2=%s", printBuf,
            uiccAuth.session_id, uiccAuth.context1, uiccAuth.context2);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    s_callbacks.onRequest(pRI->pCI->requestNumber, &uiccAuth, sizeof(uiccAuth), pRI);

#ifdef MEMSET_FREED
    memsetString(uiccAuth.context1);
    memsetString(uiccAuth.context2);
#endif

    free(uiccAuth.context1);
    free(uiccAuth.context2);

#ifdef MEMSET_FREED
    memset(&uiccAuth, 0, sizeof(uiccAuth));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}
//MTK-END [mtk80776] WiFi Calling

static void triggerEvLoop()
{
    int ret;
    if (!pthread_equal(pthread_self(), s_tid_dispatch)) {
        /* trigger event loop to wakeup. No reason to do this,
         * if we're in the event loop thread */
        do {
            ret = write (s_fdWakeupWrite, " ", 1);
        } while (ret < 0 && errno == EINTR);
    }
}

static void rilEventAddWakeup(struct ril_event *ev)
{
    ril_event_add(ev);
    triggerEvLoop();
}

#ifdef FATAL_ERROR_HANDLE
static void
handleRILDFatalError() {
    LOGD("handleRILDFatalError");
#ifdef MTK_RIL_MD2
    property_set("mux.report.case", "6");
    property_set("ctl.start", "muxreport-daemon");
#else
    property_set("mux.report.case", "2");
    property_set("ctl.start", "muxreport-daemon");
#endif
}
#endif

/**
 * A write on the wakeup fd is done just to pop us out of select()
 * We empty the buffer here and then ril_event will reset the timers on the
 * way back down
 */
static void processWakeupCallback(int fd, short flags, void *param) {
    char buff[16];
    int ret;

    LOGV("processWakeupCallback");

    /* empty our wakeup socket out */
    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

static void onCommandsSocketClosed(RILId id) {
    int ret;
    RequestInfo *p_cur;
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo **    pendingRequestsHook = &s_pendingRequests;

#ifdef MTK_GEMINI
    if (MTK_RIL_SOCKET_2 == id) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex2;
        pendingRequestsHook = &s_pendingRequests2;
    }
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
     if (MTK_RIL_SOCKET_4 == id) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex4;
        pendingRequestsHook = &s_pendingRequests4;
     }
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
     if (MTK_RIL_SOCKET_3 == id) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex3;
        pendingRequestsHook = &s_pendingRequests3;
     }
#endif
#endif /* MTK_GEMINI */

    /* mark pending requests as "cancelled" so we dont report responses */
    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    p_cur = *pendingRequestsHook;

    for (p_cur = *pendingRequestsHook
                 ; p_cur != NULL
            ; p_cur  = p_cur->p_next
        ) {
        p_cur->cancelled = 1;
    }

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

#ifdef MTK_RIL
    RequestInfo *pRI;
    int request = RIL_REQUEST_HANGUP_ALL;

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));

    pRI->local = 1;
    pRI->token = 0xffffffff;
    pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_MTK_BASE]);
    if (id == MTK_RIL_SOCKET_1) {
        pRI->cid = RIL_CMD_PROXY_2;
    }
#ifdef MTK_GEMINI
    else {
        pRI->cid = RIL_CMD2_PROXY_2;
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
        if (MTK_RIL_SOCKET_4 == id) 
            pRI->cid = RIL_CMD2_PROXY_4;
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
        if (MTK_RIL_SOCKET_3 == id) 
            pRI->cid = RIL_CMD2_PROXY_3;
#endif
    }
#endif

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    s_callbacks.onRequest(request, NULL, 0, pRI);
#endif
    
  
}

static void processCommandsCallback(int fd, short flags, void *param) {
    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;
    SocketListenParam *p_info;
    p_info = (SocketListenParam *)param;

    assert(fd == *p_info->s_fdCommand);

    p_rs = p_info->p_rs;

    for (;;) {
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            LOGI("end-of-stream ril%d", p_info->rilId+1);
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { /* && p_record != NULL */
            processCommandBuffer(p_record, recordlen, p_info->rilId);
        }
    }

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            LOGE("error on reading command socket%d errno:%d\n", p_info->rilId, errno);
        } else {
            LOGW("EOS.  Closing command socket RIL%d.", p_info->rilId+1);
        }
#ifndef REVERSE_MTK_CHANGE
        if(*p_info->s_fdCommand == *p_info->s_fdUT_command)
        {
            LOGD("RIL UT: Closing UT command socket RIL%d.", p_info->rilId+1);

            close(*p_info->s_fdUT_command);
            *p_info->s_fdUT_command = -1;

            ril_event_del(p_info->s_UTcommand_event);
            record_stream_free(p_rs);

            if (*p_info->s_fdUT_tmp_command > 0)
            {
                LOGD("RIL UT: restore command eventRIL%d.", p_info->rilId+1);

                // restore the command event
                rilEventAddWakeup(p_info->s_commands_event);

                *p_info->s_fdCommand = *p_info->s_fdUT_tmp_command;
                *p_info->s_fdUT_tmp_command = -1;
            }
            else
            {
                LOGD("RIL UT: restore listen eventRIL%d.", p_info->rilId+1);

                // restore the listen event
                rilEventAddWakeup(p_info->s_listen_event);

                *p_info->s_fdCommand = -1;
            }
            rilEventAddWakeup(p_info->s_UTlisten_event);
        }
        else
#endif
        {

            close(*p_info->s_fdCommand);
            *p_info->s_fdCommand = -1;

            ril_event_del(p_info->s_commands_event);

            record_stream_free(p_rs);

            /* start listening for new connections again */
            rilEventAddWakeup(p_info->s_listen_event);

            onCommandsSocketClosed(p_info->rilId);
        }
    }
}

static void onNewCommandConnect(RILId id)
{
    int temp_state = RADIO_TEMPSTATE_AVAILABLE;

    // implicit radio state changed
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                              NULL, 0, id);
    RIL_onUnsolicitedResponse(RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE,
                              &temp_state, sizeof(int), id);

    // Send last NITZ time data, in case it was missed
    if (s_lastNITZTimeData != NULL) {
        sendResponseRaw(s_lastNITZTimeData, s_lastNITZTimeDataSize, id);

        free(s_lastNITZTimeData);
        s_lastNITZTimeData = NULL;
    }

    RILId primary_rilid = MTK_RIL_SOCKET_1;

#ifdef MTK_GEMINI
    if (isInternationalRoamingEnabled()
        || getTelephonyMode() == 100
        || getTelephonyMode() == 101
        || isEVDODTSupport()) {
        primary_rilid = MTK_RIL_SOCKET_2;
    } else if (::isGeminiMode() == 1) {
    #if (MTK_GEMINI_SIM_NUM == 3)
        primary_rilid = MTK_RIL_SOCKET_3;
    #elif (MTK_GEMINI_SIM_NUM == 4)
        primary_rilid = MTK_RIL_SOCKET_4;
    #else		
        primary_rilid = MTK_RIL_SOCKET_2;
    #endif
    } else {
        primary_rilid = MTK_RIL_SOCKET_1;
    }
#endif
    LOGI("primary_rilid: %d\n", (primary_rilid+1));

    if (primary_rilid == id) {

#ifdef MTK_GEMINI
        int sim_status = 0;
        s_callbacks.onStateRequest(id, &sim_status);
        if (sim_status == -1) {
            LOGI("RIL callback is not initialized, not to report SIM inserted status");
        } else {
            RIL_onUnsolicitedResponse(RIL_UNSOL_SIM_INSERTED_STATUS,
                                  &sim_status, sizeof(int), id);
        }
#endif

        // Get version string
        if (s_callbacks.getVersion != NULL) {
            const char *version;
            version = s_callbacks.getVersion();
            LOGI("RIL Daemon version: %s\n", version);

            property_set(PROPERTY_RIL_IMPL, version);
        } else {
            LOGI("RIL Daemon version: unavailable\n");
            property_set(PROPERTY_RIL_IMPL, "unavailable");
        }
    }

}

static void listenCallback (int fd, short flags, void *param) {
    int ret;
    int err;
    int is_phone_socket;
    RecordStream *p_rs;
    SocketListenParam *p_info;
    p_info = (SocketListenParam *)param;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);

    struct passwd *pwd = NULL;

    assert (*p_info->s_fdCommand < 0);
    assert (fd == *p_info->s_fdListen);

    *p_info->s_fdCommand = accept(*p_info->s_fdListen, (sockaddr *) &peeraddr, &socklen);

    if (*p_info->s_fdCommand < 0 ) {
        LOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(p_info->s_listen_event);
        return;
    }

    /* check the credential of the other side and only accept socket from
     * phone process
     */
    errno = 0;
    is_phone_socket = 0;

    err = getsockopt(*p_info->s_fdCommand, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);

    if (err == 0 && szCreds > 0) {
        errno = 0;
        pwd = getpwuid(creds.uid);
        if (pwd != NULL) {
            if (strcmp(pwd->pw_name, p_info->PROCESS_NAME) == 0) {
                is_phone_socket = 1;
            } else {
                LOGE("RILD can't accept socket from process %s", pwd->pw_name);
            }
        } else {
            LOGE("Error on getpwuid() errno: %d", errno);
        }
    } else {
        LOGD("Error on getsockopt() errno: %d", errno);
    }

    if ( !is_phone_socket ) {
        LOGE("RILD must accept socket from %s", p_info->PROCESS_NAME);

        close(*p_info->s_fdCommand);
        *p_info->s_fdCommand = -1;

        onCommandsSocketClosed(p_info->rilId);

        /* start listening for new connections again */
        rilEventAddWakeup(p_info->s_listen_event);

        return;
    }

    ret = fcntl(*p_info->s_fdCommand, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }
#ifdef MTK_RIL
    LOGI("librilmtk: new connection RIL%d", p_info->rilId+1);
#else
    LOGI("libril: new connection");
#endif /* MTK_RIL */
    p_rs = record_stream_new(*p_info->s_fdCommand, MAX_COMMAND_BYTES);

	p_info->p_rs = p_rs;

    ril_event_set (p_info->s_commands_event, *p_info->s_fdCommand, 1,
                   p_info->processCommandsCallback, p_info);

    rilEventAddWakeup (p_info->s_commands_event);

    onNewCommandConnect(p_info->rilId);

    sendPendedUrcs(p_info->rilId);

}

static void freeDebugCallbackArgs(int number, char **args) {
    for (int i = 0; i < number; i++) {
        if (args[i] != NULL) {
            free(args[i]);
        }
    }
    free(args);
}

static void debugCallback (int fd, short flags, void *param) {
    int acceptFD, option;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int data;
    unsigned int qxdm_data[6];
    const char *deactData[1] = {"1"};
    char *actData[1];
    RIL_Dial dialData;
    int hangupData[1] = {1};
    int number;
    char **args;
    RILId id = MTK_RIL_SOCKET_1;
    char *stk_str = NULL;

    acceptFD = accept (fd,  (sockaddr *) &peeraddr, &socklen);

    if (acceptFD < 0) {
        LOGE ("error accepting on debug port: %d\n", errno);
        return;
    }
   
    if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
        LOGE ("error reading on socket: number of Args: \n");
        return;
    }
    args = (char **) malloc(sizeof(char*) * number);


    LOGI ("NUMBER:%d", number);

    for (int i = 0; i < number; i++) {
        unsigned int len;
        if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
            LOGE ("error reading on socket: Len of Args: \n");
            freeDebugCallbackArgs(i, args);
            return;
        }
        // +1 for null-term

        LOGI ("arg len:%d", len);

        args[i] = (char *) malloc((sizeof(char) * len) + 1);
        if (recv(acceptFD, args[i], sizeof(char) * len, 0)
                != (int)(sizeof(char) * len)) {
            LOGE ("error reading on socket: Args[%d] \n", i);
            freeDebugCallbackArgs(i, args);
            return;
        }
        char * buf = args[i];
        buf[len] = 0;

        LOGI ("ARGS[%d]:%s",i, buf);
    }

#ifdef MTK_EAP_SIM_AKA  
    if(0< handleSpecialRequestWithArgs(number, args)){
        freeDebugCallbackArgs(number, args);
        LOGI("Debug port: SpecialRequest return");
        return;
    }
#endif

    switch (atoi(args[0])) {
    case 0:
        LOGI ("Connection on debug port: issuing reset.");
        issueLocalRequest(RIL_REQUEST_RESET_RADIO, NULL, 0);
        break;
    case 1:
        LOGI ("Connection on debug port: issuing radio power off.");
        data = 0;
        issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int));
        // Close the socket
        close(s_fdCommand);
        s_fdCommand = -1;
        break;
    case 2:
        LOGI ("Debug port: issuing unsolicited network change.");
        RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
                                  NULL, 0, id);
        break;
    case 3:
        LOGI ("Debug port: QXDM log enable.");
        qxdm_data[0] = 65536;
        qxdm_data[1] = 16;
        qxdm_data[2] = 1;
        qxdm_data[3] = 32;
        qxdm_data[4] = 0;
        qxdm_data[4] = 8;
        issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, qxdm_data,
                          6 * sizeof(int));
        break;
    case 4:
        LOGI ("Debug port: QXDM log disable.");
        qxdm_data[0] = 65536;
        qxdm_data[1] = 16;
        qxdm_data[2] = 0;
        qxdm_data[3] = 32;
        qxdm_data[4] = 0;
        qxdm_data[4] = 8;
        issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, qxdm_data,
                          6 * sizeof(int));
        break;
    case 5:
        LOGI("Debug port: Radio On");
        data = 1;
        issueLocalRequest(RIL_REQUEST_RADIO_POWER, &data, sizeof(int));
        sleep(2);
        // Set network selection automatic.
        issueLocalRequest(RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, NULL, 0);
        break;
    case 6:
        LOGI("Debug port: Setup Data Call, Apn :%s\n", args[1]);
        actData[0] = args[1];
        issueLocalRequest(RIL_REQUEST_SETUP_DATA_CALL, &actData,
                          sizeof(actData));
        break;
    case 7:
        LOGI("Debug port: Deactivate Data Call");
        issueLocalRequest(RIL_REQUEST_DEACTIVATE_DATA_CALL, &deactData,
                          sizeof(deactData));
        break;
    case 8:
        LOGI("Debug port: Dial Call");
        dialData.clir = 0;
        dialData.address = args[1];
        issueLocalRequest(RIL_REQUEST_DIAL, &dialData, sizeof(dialData));
        break;
    case 9:
        LOGI("Debug port: Answer Call");
        issueLocalRequest(RIL_REQUEST_ANSWER, NULL, 0);
        break;
    case 10:
        LOGI("Debug port: End Call");
        issueLocalRequest(RIL_REQUEST_HANGUP, &hangupData,
                          sizeof(hangupData));
        break;
    case 11:
        /**
        if(number == 2) {
            stk_str = args[1];
            LOGI("Debug port: STK URC:%s", stk_str);
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND,
                                      stk_str, strlen(stk_str), id);
        }*/
        if(3 == number) {
            id = (RILId)atoi(args[1]);
            if(id != MTK_RIL_SOCKET_1 
                #ifdef MTK_GEMINI
                && id != MTK_RIL_SOCKET_2
                #if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
                        && id != MTK_RIL_SOCKET_4
                #endif
                #if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
                        && id != MTK_RIL_SOCKET_3
                #endif
                #endif
                ) {
                id = MTK_RIL_SOCKET_1;
            }
            
            stk_str = args[2];
            LOGI("Debug port: STK URC:%s, socket id = %d", stk_str, id);
            RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND,
                    stk_str, strlen(stk_str), id);
        }
        break;
    default:
        LOGE ("Invalid request");
        break;
    }
    freeDebugCallbackArgs(number, args);
    close(acceptFD);
}

static void oemCallback (int fd, short flags, void *param) {
    int acceptFD;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int number;
    char **args;

    acceptFD = accept (fd,  (sockaddr *) &peeraddr, &socklen);

    if (acceptFD < 0) {
        LOGE ("error accepting on oem port: %d\n", errno);
        return;
    }

    s_fdOem_command = acceptFD;
    
    if (recv(acceptFD, &number, sizeof(int), 0) != sizeof(int)) {
        LOGE ("error reading on socket: number of Args: \n");
        return;
    }
    args = (char **) malloc(sizeof(char*) * number);


    LOGI ("NUMBER:%d", number);

    for (int i = 0; i < number; i++) {
        unsigned int len;
        if (recv(acceptFD, &len, sizeof(int), 0) != sizeof(int)) {
            LOGE ("error reading on socket: Len of Args: \n");
            freeDebugCallbackArgs(i, args);
            return;
        }
        // +1 for null-term

        LOGI ("arg len:%d", len);

        args[i] = (char *) malloc((sizeof(char) * len) + 1);
        if (recv(acceptFD, args[i], sizeof(char) * len, 0)
                != (int)(sizeof(char) * len)) {
            LOGE ("error reading on socket: Args[%d] \n", i);
            freeDebugCallbackArgs(i, args);
            return;
        }
        char * buf = args[i];
        buf[len] = 0;

        LOGI ("ARGS[%d]:%s",i, buf);
    }

    if(0 < handleSpecialRequestWithArgs(number, args)){
		freeDebugCallbackArgs(number, args);
    } else {
        LOGI("Oem port: SpecialRequest not support");
        freeDebugCallbackArgs(number, args);
        close(s_fdOem_command);
        s_fdOem_command = -1;
    }	 
}

static void skipWhiteSpace(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && isspace(**p_cur)) {
        (*p_cur)++;
    }
}

static void skipNextComma(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && **p_cur != ',') {
        (*p_cur)++;
    }

    if (**p_cur == ',') {
        (*p_cur)++;
    }    
}

static char * nextTok(char **p_cur)
{
    char *ret = NULL;

    skipWhiteSpace(p_cur);

    if (*p_cur == NULL) {
        ret = NULL;
    } else if (**p_cur == '"') {
        (*p_cur)++;
        ret = strsep(p_cur, "\"");
        skipNextComma(p_cur);
    } else {
        ret = strsep(p_cur, ",");
    }

    return ret;
}

int at_tok_nextstr(char **p_cur, char **p_out)
{
    if (*p_cur == NULL) {
        return -1;
    }

    *p_out = nextTok(p_cur);

    return 0;
}

/** returns 1 on "has more tokens" and 0 if no */
int at_tok_hasmore(char **p_cur)
{
    return ! (*p_cur == NULL || **p_cur == '\0');
}

/**
 * Parses the next integer in the AT response line and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 * "base" is the same as the base param in strtol
 */

static int at_tok_nextint_base(char **p_cur, int *p_out, int base, int  uns)
{
    char *ret;
    
    if (*p_cur == NULL) {
        return -1;
    }

    ret = nextTok(p_cur);

    if (ret == NULL) {
        return -1;
    } else {
        long l;
        char *end;

        if (uns)
            l = strtoul(ret, &end, base);
        else
            l = strtol(ret, &end, base);

        *p_out = (int)l;

        if (end == ret) {
            return -1;
        }
    }

    return 0;
}

/**
 * Parses the next base 10 integer in the AT response line 
 * and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 */
int at_tok_nextint(char **p_cur, int *p_out)
{
    return at_tok_nextint_base(p_cur, p_out, 10, 0);
}

static int handleSpecialRequestWithArgs(int argCount, char** args){
    char *line, *cmd;
    int err;
    int slotId;
    char *param[2];
    RILId id = MTK_RIL_SOCKET_1;    
    char sim[PROPERTY_VALUE_MAX] ={0};
    int sim_status = 0;
    int simId3G = 0;
    char org_args[PROPERTY_VALUE_MAX] = {0};
		
    if (1 == argCount)
    {
        line = args[0];
        strcpy(org_args, args[0]);
        err = at_tok_nextstr(&line,&cmd);
        if (err < 0) {
            LOGD("invalid command");
            goto error;
        }
        LOGD("handleSpecialRequestWithArgs cmd = %s", cmd);

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line,&slotId);
            if (err < 0) {
                LOGD("invalid slotId");
                goto error;
            }
            LOGD("handleSpecialRequestWithArgs slotId = %d", slotId);
        }
        
        if(at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line,&param[0]);
            if (err < 0) {
                LOGD("invalid param");
                goto error;
            }
            LOGD("handleSpecialRequestWithArgs param[0] = %s", param[0]);
        }
        
#ifdef MTK_GEMINI       
        property_get(PROPERTY_3G_SIM, sim, "1");
        simId3G = atoi(sim)-1;
        
        if((getTelephonyMode()>=5)&&(getTelephonyMode()<=8)){
            /* dual MD project ,each RIL(MD) control each SIM slot.  */			
            LOGD("EAP SIM will connect to specific ril debug socket");
        } else {	
            if (simId3G > 0){
                s_callbacks.onStateRequest(s_ril_cntx[simId3G], &sim_status);
                if(0 == slotId && sim_status >1 ){
                    id = s_ril_cntx[simId3G];
                }else if(simId3G == slotId ){
                    id = s_ril_cntx[0];					
                }else {
                    id = s_ril_cntx[slotId];
                }
                LOGD("3G switched,sim status of MTK_RIL_SOCKET_%d is %d, id=%d",(MTK_RIL_SOCKET_1+simId3G),sim_status,id);
				
            } else {
                id = s_ril_cntx[slotId];                
                LOGD("id=%d",id);					
            }
#if 0        
            property_get(PROPERTY_3G_SIM, sim, CAPABILITY_3G_SIM1);
        
            if (strcmp(sim, CAPABILITY_3G_SIM2) == 0)
                is3GSwitched = 1;           
            LOGD("is3GSwitched %d",is3GSwitched);
            
        
            //free(sim);

            s_callbacks.onStateRequest(MTK_RIL_SOCKET_2, &sim_status);
            LOGD("sim status of MTK_RIL_SOCKET_2 is %d",sim_status);

            if((1 == slotId && sim_status > 1 && !is3GSwitched)||(0 == slotId && sim_status >1 &&is3GSwitched)){
                id = MTK_RIL_SOCKET_2;
            }
#endif			
       }       
#endif /*MTK_GEMINI*/

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line, &param[1]); 
            if (err < 0) {
                goto error;
            }
            LOGD("handleSpecialRequestWithArgs param[1] = %s", param[1]);
        }
        
        if(strcmp(cmd, "EAP_SIM") == 0){
            if(s_EAPSIMAKA_fd < 0) {
                s_EAPSIMAKA_fd = s_fdOem_command;
            } else {
                s_EAPSIMAKA_fd2 = s_fdOem_command;
            }
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_SIM_AUTHENTICATION,&param,1,id);
            return 1;
        }else if(strcmp(cmd, "EAP_AKA") == 0){
            if (s_EAPSIMAKA_fd < 0) {
                s_EAPSIMAKA_fd = s_fdOem_command;
            } else {
                s_EAPSIMAKA_fd2 = s_fdOem_command;
            }
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_USIM_AUTHENTICATION,&param, 2,id);
            return 1;
        } else if (strcmp(cmd, "THERMAL") == 0) {
            s_THERMAL_fd = s_fdOem_command;
            issueLocalRequestForResponse(RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL, NULL, 0,id);      
            return 1;        
        } else if (strcmp(cmd, "AT+ERFTX=1") == 0){
            issueLocalRequest(RIL_REQUEST_OEM_HOOK_RAW, org_args, strlen(org_args));

            char* result = new char[10];
            strcpy(result, "OK");            
            int len = (int)strlen(result);
            int ret = send(s_fdOem_command, &len, sizeof(int), 0);
            LOGD("AT+ERFTX=1, fdOem:%d", s_fdOem_command);
            if (ret != sizeof(int)) {
                LOGD("Socket write Error: when sending arg length");
            } else {                
                ret = send(s_fdOem_command, result, len, 0);
                if (ret != len) {
                    LOGD("lose data when send response. ");
                }
            }
            free(result);
            close(s_fdOem_command);
            s_fdOem_command = -1;

            return 1;
        } else {
            // invalid request
            LOGD("invalid request");
            goto error;
        }
    }
    else
    {
        goto error;
    }
error:
    LOGE ("Invalid request");
    
    return 0;
}


static void userTimerCallback (int fd, short flags, void *param) {
    UserCallbackInfo *p_info;

    pthread_mutex_lock(&s_wakeTimeoutMutex);

    p_info = (UserCallbackInfo *)param;

#ifdef MTK_RIL
    if (p_info->cid > -1)
    {
        RequestInfoProxy * proxy = NULL;
        int proxyId;
        const char* proxyName = NULL;
        int queueSize = 0;

        proxy = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));

        assert(proxy);

        proxy->p_next = NULL;

        proxy->pRI = NULL;

        proxy->pUCI = p_info;

        proxyId = p_info->cid;

        proxyName = ::proxyIdToString(proxyId);

        /* Save dispatched proxy in RequestInfo */
        queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

        if (0 != queueSize)
        {
            LOGD("Request timed callback to %s is busy. total:%d requests pending",
                    proxyName,queueSize+1);
        }
    }
    else
    {
        p_info->p_callback(p_info->userParam);
    }
#else
    p_info->p_callback(p_info->userParam);
#endif


    // FIXME generalize this...there should be a cancel mechanism
    if (s_last_wake_timeout_info != NULL && s_last_wake_timeout_info == p_info) {
        s_last_wake_timeout_info = NULL;
    }

#ifdef MTK_RIL
    if (p_info->cid < 0)
    {
        free(p_info);
    }
#else
    free(p_info);
#endif

    pthread_mutex_unlock(&s_wakeTimeoutMutex);
}
#ifndef REVERSE_MTK_CHANGE
static void mtk_ut_Callback (int fd, short flags, void *param)
{
    int ret;
    int err;
    int is_phone_socket;
    RecordStream *p_rs;

    int cmd_fd;
    int *fdCommand, *fdUT_command;
    RILId rilId = MTK_RIL_SOCKET_1;
    struct ril_event *UT_listen_event;
    struct ril_event *UTcommand_event;
    struct ril_event *listen_event;
    struct ril_event *commands_event;
    void (*callback)(int, short, void *);

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);

    struct passwd *pwd = NULL;

#ifdef MTK_GEMINI
    assert (fd == s_fdUT_listen || fd == s_fdUT_listen2);
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
    assert (fd == s_fdUT_listen4);
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
    assert (fd == s_fdUT_listen3);
#endif
#else
    assert (fd == s_fdUT_listen);
#endif

#ifdef MTK_GEMINI
    if (fd == s_fdUT_listen2)
    {
        UT_listen_event = &s_UTlisten_event2;
        UTcommand_event = &s_UTcommand_event2;
        commands_event = &s_commands_event2;
        listen_event = &s_listen_event2;
        fdCommand = &s_fdCommand2;
        fdUT_command = &s_fdUT_command2;
        rilId = MTK_RIL_SOCKET_2;
        callback = processCommandsCallback;
    }
    else
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
    if (fd == s_fdUT_listen4)
    {
        UT_listen_event = &s_UTlisten_event4;
        UTcommand_event = &s_UTcommand_event4;
        commands_event = &s_commands_event4;
        listen_event = &s_listen_event4;
        fdCommand = &s_fdCommand4;
        fdUT_command = &s_fdUT_command4;
        rilId = MTK_RIL_SOCKET_4;
        callback = processCommandsCallback;
    }
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
    if (fd == s_fdUT_listen2)
    {
        UT_listen_event = &s_UTlisten_event3;
        UTcommand_event = &s_UTcommand_event3;
        commands_event = &s_commands_event3;
        listen_event = &s_listen_event3;
        fdCommand = &s_fdCommand3;
        fdUT_command = &s_fdUT_command3;
        rilId = MTK_RIL_SOCKET_3;
        callback = processCommandsCallback;
    }
#endif
#endif
    {
        UT_listen_event = &s_UTlisten_event;
        UTcommand_event = &s_UTcommand_event;
        commands_event = &s_commands_event;
        listen_event = &s_listen_event;
        fdCommand = &s_fdCommand;
        fdUT_command = &s_fdUT_command;
        rilId = MTK_RIL_SOCKET_1;
        callback = processCommandsCallback;
    }

    cmd_fd = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (cmd_fd < 0 ) {
        LOGE("Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(UT_listen_event);
        return;
    }

    errno = 0;

    ret = fcntl(cmd_fd, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("Error setting O_NONBLOCK errno:%d", errno);
    }

    LOGI("RIL%d UT: new connection", rilId+1 );

    if((*fdCommand) >= 0)
    {
        int temp_state = RADIO_TEMPSTATE_UNAVAILABLE;

        LOGD("RIL UT: Delete the commands_event");
        // the connection is already existed
        ril_event_del(commands_event);

        //s_fdUT_tmp_command = s_fdCommand;

        RIL_onUnsolicitedResponse(RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE,
                                  &temp_state, sizeof(int), rilId);
        close(*fdCommand);
    }
    else
    {
        // there is no connection
        LOGD("RIL UT: Delete the listen_event");
        // delete listen event first
        ril_event_del(listen_event);
    }

    LOGD("RIL UT: Replace the commands FD");
    *fdUT_command = cmd_fd;
    *fdCommand = cmd_fd;


    LOGD("RIL UT: Create UT events");
    p_rs = record_stream_new(cmd_fd, MAX_COMMAND_BYTES);
    ril_event_set (UTcommand_event, cmd_fd, 1,
                   callback, p_rs);
    rilEventAddWakeup (UTcommand_event);


}
#endif

/* atci start */
#define MAX_DATA_SIZE 2048

static void processAtciCommandsCallback(int fd, short flags, void *param) {
    LOGD("[ATCI]processAtciCommandsCallback");

    int ret;
    int request = RIL_REQUEST_OEM_HOOK_RAW;
    int recvLen = -1;
    char buffer[MAX_DATA_SIZE] = {0};
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex;
    RequestInfo **    pendingRequestsHook = &s_pendingRequests;
    int forceHandle = 0;

    while(recvLen == -1) {
        recvLen = recv(s_fdATCI_command, buffer, MAX_DATA_SIZE, 0);
        if (recvLen == -1) {
            LOGE("[ATCI] fail to receive data from ril-atci socket. errno = %d", errno);
            if(errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
    }

    LOGD("[ATCI] data receive from atci is %s, data length is %d", buffer, recvLen);

    if (strcmp(buffer, "DISC") == 0 || recvLen <= 0) { //recLen == 0 means the client is disconnected.
        int sendLen = 0;

        if(recvLen != -1) {
            sendLen = send(s_fdATCI_command, buffer, recvLen, 0);
            if (sendLen != recvLen) {
                LOGE("[ATCI] lose data when send disc cfm to atci. errno = %d", errno);
            }
        } else {
            LOGE("[ATCI] close ATCI due to error = %d", errno);
        }

        ril_event_del(&s_ATCIcommand_event);
        rilEventAddWakeup (&s_ATCIlisten_event);
        close(s_fdATCI_command);
        s_fdATCI_command = -1;
    } else {
        RequestInfo *pRI;
        pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));
        pRI->token = 0xffffffff;        // token is not used in this context

        if((strcmp(buffer, "AT+CFUN=0") == 0) || (strcmp(buffer, "at+cfun=0") == 0)){ 
            /* ALPS00306731 */ 
            LOGE("Special Handling for AT+CFUN=0 from ATCI");
            forceHandle = 1;
        }

        if((strcmp(buffer, "AT+CFUN=1") == 0) || (strcmp(buffer, "at+cfun=1") == 0)){ 
            /* ALPS00314520 */ 
            LOGE("Special Handling for AT+CFUN=1 from ATCI");
            forceHandle = 1;
        }

#ifdef MTK_RIL
        if (request < RIL_REQUEST_MTK_BASE) {
            pRI->pCI = &(s_commands[request]);
        } else {
            pRI->pCI = &(s_mtk_commands[request - RIL_REQUEST_MTK_BASE]);
        }
#else
        pRI->pCI = &(s_commands[request]);
#endif /* MTK_RIL */

        pRI->cid = RIL_CMD_1;
#ifdef MTK_GEMINI
        property_get("persist.service.atci.sim", simNo, NULL);
        LOGD("persist.service.atci.sim> %s", simNo);
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */ 
        if (simNo[0] == '3') {
        	LOGD("CID set to SIM4");
        	pRI->cid = RIL_CMD4_1;
        	pendingRequestsMutexHook = &s_pendingRequestsMutex4;
        	pendingRequestsHook = &s_pendingRequests4;
        }
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
        if (simNo[0] == '2') {
        	LOGD("CID set to SIM3");
        	pRI->cid = RIL_CMD3_1;
        	pendingRequestsMutexHook = &s_pendingRequestsMutex3;
        	pendingRequestsHook = &s_pendingRequests3;
        }
#endif
        if (simNo[0] == '1') {
            LOGD("CID set to SIM2");
            pRI->cid = RIL_CMD2_1;
            pendingRequestsMutexHook = &s_pendingRequestsMutex2;
            pendingRequestsHook = &s_pendingRequests2;
        }
#endif /* MTK_GEMINI */

        ret = pthread_mutex_lock(pendingRequestsMutexHook);
        assert (ret == 0);
        pRI->p_next = *pendingRequestsHook;
        *pendingRequestsHook = pRI;

        ret = pthread_mutex_unlock(pendingRequestsMutexHook);
        assert (ret == 0);


        LOGD("C[locl]> %s", requestToString(request));

#ifdef MTK_RIL
        {
            RequestInfoProxy * proxy = NULL;
            int proxyId;
            const char* proxyName = NULL;
            int queueSize = 0;            
    
            proxy = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
            proxy->p = new Parcel();
    
            assert(proxy);
    
            recvLen++;
            proxy->p_next = NULL;
            proxy->pRI = pRI;
            proxy->p->writeInt32(recvLen);
            proxy->p->write((void*) buffer, (size_t) recvLen);
            proxy->p->setDataPosition(0);
        
            if (request < RIL_REQUEST_MTK_BASE) {
                proxyId = s_commands[request].proxyId;
            } else {
                proxyId = s_mtk_commands[request - RIL_REQUEST_MTK_BASE].proxyId;
            }
        
            if(forceHandle == 1)
            {
                // ALPS00306731 send command in URC channel to prevent blocked by other command         
                LOGE("Force command from ATCI to be executed in RIL_CMD_PROXY_4");                
                proxyId = RIL_CMD_PROXY_4; 
            }           
        
    #ifdef MTK_GEMINI
            /* Shift proxyId if needed */
    #if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */ 
            if (pRI->cid == RIL_CMD4_1) {
                proxyId = proxyId + RIL_CHANNEL_SET4_OFFSET;
                /* Update */
            }
    #endif
    #if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
            if (pRI->cid == RIL_CMD3_1) {
                proxyId = proxyId + RIL_CHANNEL_SET3_OFFSET;
                /* Update */
            }
    #endif
            if (pRI->cid == RIL_CMD2_1) {
                proxyId = proxyId + RIL_CHANNEL_OFFSET;
                /* Update */
            }
    #endif /* MTK_GEMINI */
        
            proxyName = ::proxyIdToString(proxyId);
    
            /* Save dispatched proxy in RequestInfo */
            proxy->pRI->cid = (RILChannelId) proxyId;
        
            queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);
    
            if (0 != queueSize)
            {
                LOGD("%s is busy. %s queued. total:%d requests pending",proxyName,requestToString(request),queueSize+1);   
            }
        }
#else
        /*    sLastDispatchedToken = token; */
        pRI->pCI->dispatchFunction(p, pRI);
#endif 

    }

}

static void atci_Callback (int fd, short flags, void *param)
{
    LOGD("[ATCI] enter atci_Callback");
    int ret;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    assert (s_fdATCI_command< 0);
    assert (fd == s_fdATCI_listen);

    s_fdATCI_command = accept(s_fdATCI_listen, (sockaddr *) &peeraddr, &socklen);

    if (s_fdATCI_command < 0 ) {
        LOGE("[ATCI] Error on accept() errno:%d", errno);
        /* start listening for new connections again */
        rilEventAddWakeup(&s_ATCIlisten_event);
        s_fdATCI_command = -1;
        return;
    }

    LOGD("[ATCI] accept");

    ret = fcntl(s_fdATCI_command, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        LOGE ("[ATCI] Error setting O_NONBLOCK errno:%d", errno);
    }

    LOGD("[ATCI] librilmtk: new rild-atci connection");


    ril_event_set (&s_ATCIcommand_event, s_fdATCI_command, true,
                   processAtciCommandsCallback, NULL);

    rilEventAddWakeup (&s_ATCIcommand_event);
}
/* atci end */

static void *
eventLoop(void *param) {
    int ret;
    int filedes[2];

    ril_event_init();

    pthread_mutex_lock(&s_startupMutex);

    s_started = 1;
    pthread_cond_broadcast(&s_startupCond);

    pthread_mutex_unlock(&s_startupMutex);

    ret = pipe(filedes);

    if (ret < 0) {
        LOGE("Error in pipe() errno:%d", errno);
        return NULL;
    }

    s_fdWakeupRead = filedes[0];
    s_fdWakeupWrite = filedes[1];

    fcntl(s_fdWakeupRead, F_SETFL, O_NONBLOCK);

    ril_event_set (&s_wakeupfd_event, s_fdWakeupRead, true,
                   processWakeupCallback, NULL);

    rilEventAddWakeup (&s_wakeupfd_event);

    // Only returns on error
    ril_event_loop();
    LOGE ("error in event_loop_base errno:%d", errno);

    return NULL;
}

extern "C" void
RIL_startEventLoop(void) {
    int ret;
    pthread_attr_t attr;
#ifdef MTK_RIL
    RIL_startRILProxys();
#endif /* MTK_RIL */
    /* spin up eventLoop thread and wait for it to get started */
    s_started = 0;
    pthread_mutex_lock(&s_startupMutex);

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_dispatch, &attr, eventLoop, NULL);

    while (s_started == 0) {
        pthread_cond_wait(&s_startupCond, &s_startupMutex);
    }

    pthread_mutex_unlock(&s_startupMutex);

    if (ret < 0) {
        LOGE("Failed to create dispatch thread errno:%d", errno);
        return;
    }
}

// Used for testing purpose only.
extern "C" void RIL_setcallbacks (const RIL_RadioFunctions *callbacks) {
    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));
}

extern "C" void
RIL_register (const RIL_RadioFunctions *callbacks) {
    int ret;
    int flags;
    char prop_value[2]= {0};

    if (callbacks == NULL || ((callbacks->version != RIL_VERSION)
                              && (callbacks->version < 2))) { // Remove when partners upgrade to version 3
        LOGE(
            "RIL_register: RIL_RadioFunctions * null or invalid version"
            " (expected %d)", RIL_VERSION);
        return;
    }
    if (callbacks->version < RIL_VERSION) {
        LOGE ("RIL_register: upgrade RIL to version %d current version=%d",
              RIL_VERSION, callbacks->version);
    }

    if (s_registerCalled > 0) {
        LOGE("RIL_register has been called more than once. "
             "Subsequent call ignored");
        return;
    }

    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

    /* ALPS00556811: initialize earlier before s_registerCalled is set, to prevent  null access in RIL_onUnsolicitedResponse */
    s_SocketLinterParam1 = {
                    MTK_RIL_SOCKET_1,
                    &s_fdListen, &s_fdCommand,
                    &s_fdUT_command, &s_fdUT_tmp_command,
                    PHONE_PROCESS,
                    &s_commands_event, &s_listen_event,
                    &s_UTcommand_event, &s_UTlisten_event,
                    processCommandsCallback,
                    NULL};

#ifdef MTK_GEMINI
    s_SocketLinterParam2 = {
                    MTK_RIL_SOCKET_2,
                    &s_fdListen2, &s_fdCommand2, 
                    &s_fdUT_command2, &s_fdUT_tmp_command2,
                    PHONE_PROCESS,
                    &s_commands_event2, &s_listen_event2,
                    &s_UTcommand_event2, &s_UTlisten_event2,
                    processCommandsCallback, NULL};

#if (MTK_GEMINI_SIM_NUM >= 3) 
    s_SocketLinterParam3 = {
                    MTK_RIL_SOCKET_3,
                    &s_fdListen3, &s_fdCommand3, 
                    &s_fdUT_command3, &s_fdUT_tmp_command3,
                    PHONE_PROCESS,
                    &s_commands_event3, &s_listen_event3,
                    &s_UTcommand_event3, &s_UTlisten_event3,
                    processCommandsCallback, NULL};
#endif	
#if (MTK_GEMINI_SIM_NUM >= 4) 
    s_SocketLinterParam4 = {
                    MTK_RIL_SOCKET_4,
                    &s_fdListen4, &s_fdCommand4, 
                    &s_fdUT_command4, &s_fdUT_tmp_command4,
                    PHONE_PROCESS,
                    &s_commands_event4, &s_listen_event4,
                    &s_UTcommand_event4, &s_UTlisten_event4,
                    processCommandsCallback, NULL};
#endif					
#endif
    /* ALPS00556811  END */
	 
    s_registerCalled = 1;

    LOGI("s_registerCalled flag set");

    // Little self-check

    for (int i = 0; i < (int)NUM_ELEMS(s_commands) ; i++) {
        assert(i == s_commands[i].requestNumber);
    }

    for (int i = 0; i < (int)NUM_ELEMS(s_unsolResponses) ; i++) {
        assert(i + RIL_UNSOL_RESPONSE_BASE
               == s_unsolResponses[i].requestNumber);
    }

#ifdef MTK_RIL
    for (int i = 0; i < (int)NUM_ELEMS(s_mtk_commands); i++ ) {
        assert(i + RIL_REQUEST_MTK_BASE
               == s_mtk_commands[i].requestNumber);
    }

    for (int i = 0; i < (int)NUM_ELEMS(s_mtk_unsolResponses) ; i ++) {
        assert(i + RIL_UNSOL_MTK_BASE
               == s_mtk_unsolResponses[i].requestNumber);
    }

#endif /* MTK_RIL */


    // New rild impl calls RIL_startEventLoop() first
    // old standalone impl wants it here.

    if (s_started == 0) {
        RIL_startEventLoop();
    }

    // start listen socket

    int telephonyMode = MTK_TELEPHONY_MODE;

#ifdef MTK_RIL_MD2
    char* socket1 = SOCKET_NAME_RIL_MD2;
#ifdef MTK_GEMINI
    char* socket2 = SOCKET_NAME_RIL2_MD2;
#endif
    char* socket_debug = SOCKET_NAME_RIL_DEBUG_MD2;
    char* socket_oem = SOCKET_NAME_RIL_OEM_MD2;
    char* socket_ut = SOCKET_NAME_MTK_UT_MD2;
    char* socket_ut2 = SOCKET_NAME_MTK_UT_2_MD2;
    char* socket_atci = SOCKET_NAME_ATCI_MD2;
#else
    char* socket1 = SOCKET_NAME_RIL;
#ifdef MTK_GEMINI
    char* socket2 = SOCKET_NAME_RIL2;
#endif
    char* socket_debug = SOCKET_NAME_RIL_DEBUG;
    char* socket_oem = SOCKET_NAME_RIL_OEM;
    char* socket_ut = SOCKET_NAME_MTK_UT;
    char* socket_ut2 = SOCKET_NAME_MTK_UT_2;
    char* socket_atci = SOCKET_NAME_ATCI;   
#endif

    s_fdListen = android_get_control_socket(socket1);
    if (s_fdListen < 0) {
        LOGE("Failed to get socket '%s'", socket1);
        exit(-1);
    }

    ret = listen(s_fdListen, 4);

    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s",
             s_fdListen, strerror(errno));
        exit(-1);
    }

    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_listen_event, s_fdListen, false,
                   listenCallback, &s_SocketLinterParam1);

    rilEventAddWakeup (&s_listen_event);

#ifdef MTK_GEMINI
    // start listen socket phone2
    s_fdListen2 = android_get_control_socket(socket2);
    if (s_fdListen2 < 0) {
        LOGE("Failed to get socket '%s'", socket2);
        exit(-1);
    }

    ret = listen(s_fdListen2, 4);

    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s",
             s_fdListen2, strerror(errno));
        exit(-1);
    }

    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_listen_event2, s_fdListen2, false,
                   listenCallback, &s_SocketLinterParam2);
    rilEventAddWakeup (&s_listen_event2);

#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    // start listen socket phone3
    s_fdListen3 = android_get_control_socket(SOCKET_NAME_RIL3);
    if (s_fdListen3 < 0) {
        LOGE("Failed to get socket '" SOCKET_NAME_RIL3 "'");
        exit(-1);
    }
    ret = listen(s_fdListen3, 4);
    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s",
             s_fdListen3, strerror(errno));
        exit(-1);
    }

    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_listen_event3, s_fdListen3, false,
                   listenCallback, &s_SocketLinterParam3);
    rilEventAddWakeup (&s_listen_event3);
#endif

#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    // start listen socket phone4
    s_fdListen4 = android_get_control_socket(SOCKET_NAME_RIL4);
    if (s_fdListen4 < 0) {
        LOGE("Failed to get socket '" SOCKET_NAME_RIL4 "'");
        exit(-1);
    }
    ret = listen(s_fdListen4, 4);
    if (ret < 0) {
        LOGE("Failed to listen on control socket '%d': %s",
             s_fdListen4, strerror(errno));
        exit(-1);
    }

    /* note: non-persistent so we can accept only one connection at a time */
    ril_event_set (&s_listen_event4, s_fdListen4, false,
                   listenCallback, &s_SocketLinterParam4);
    rilEventAddWakeup (&s_listen_event4);
#endif

#endif /* MTK_GEMINI */

#if 1
    // start debug interface socket

    s_fdDebug = android_get_control_socket(socket_debug);
    if (s_fdDebug < 0) {
        LOGE("Failed to get socket '%s' errno:%d", socket_debug, errno);
        exit(-1);
    }

    ret = listen(s_fdDebug, 4);

    if (ret < 0) {
        LOGE("Failed to listen on ril debug socket '%d': %s",
             s_fdDebug, strerror(errno));
        exit(-1);
    }

    ril_event_set (&s_debug_event, s_fdDebug, true,
                   debugCallback, NULL);

    rilEventAddWakeup (&s_debug_event);
#endif

// 3g_dongle doesnt have oem socket
#ifndef MTK_3GDONGLE_SUPPORT 
#ifndef MTK_EMULATOR_SUPPORT
    s_fdOem = android_get_control_socket(socket_oem);
    if (s_fdOem < 0) {
	    LOGE("Failed to get socket '%s' errno:%d", s_fdOem, errno);
        exit(-1);
    }

    ret = listen(s_fdOem, 4);

    if (ret < 0) {
	    LOGE("Failed to listen on ril Oem socket '%d': %s",
        s_fdOem, strerror(errno));
        exit(-1);
    }

    ril_event_set (&s_oem_event, s_fdOem, true, oemCallback, NULL);

    rilEventAddWakeup (&s_oem_event);
#endif
#endif

#ifndef REVERSE_MTK_CHANGE
    // start mtk_ut interface socket
    s_fdUT_listen = android_get_control_socket(socket_ut);
    if (s_fdUT_listen < 0) {
        LOGE("Failed to get socket '%s' errno:%d", socket_ut, errno);
        exit(-1);
    }

    ret = listen(s_fdUT_listen, 4);

    if (ret < 0) {
        LOGE("Failed to listen on ril mtk ut socket '%d': %s",
             s_fdUT_listen, strerror(errno));
        exit(-1);
    }

    ril_event_set (&s_UTlisten_event, s_fdUT_listen, false,
                   mtk_ut_Callback, NULL);

    rilEventAddWakeup (&s_UTlisten_event);

#ifdef MTK_GEMINI
    // start mtk_ut_2 interface socket
    s_fdUT_listen2 = android_get_control_socket(socket_ut2);
    if (s_fdUT_listen2 < 0) {
        LOGE("Failed to get socket '%s' errno:%d", socket_ut2, errno);
        exit(-1);
    }

    ret = listen(s_fdUT_listen2, 4);

    if (ret < 0) {
        LOGE("Failed to listen on ril mtk ut socket '%d': %s",
             s_fdUT_listen2, strerror(errno));
        exit(-1);
    }

    ril_event_set (&s_UTlisten_event2, s_fdUT_listen2, false,
                   mtk_ut_Callback, NULL);

    rilEventAddWakeup (&s_UTlisten_event2);

#endif
#endif

    /* atci start */
    s_fdATCI_listen= android_get_control_socket(socket_atci);
    if (s_fdATCI_listen < 0) {
        LOGE("Failed to get socket '%s'", socket_atci);
    } else {
        ret = listen(s_fdATCI_listen, 4);
        if (ret < 0) {
            LOGE("Failed to listen on control socket '%d': %s",
                 s_fdATCI_listen, strerror(errno));
        } else {
            /* note: non-persistent so we can accept only one connection at a time */
            ril_event_set (&s_ATCIlisten_event, s_fdATCI_listen, false,
                           atci_Callback, NULL);

            rilEventAddWakeup (&s_ATCIlisten_event);
        }
    }
    /* atci end */
}

static int
checkAndDequeueRequestInfo(struct RequestInfo *pRI) {
    int ret = 0;
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t * pendingRequestsMutexHook = &s_pendingRequestsMutex;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo **    pendingRequestsHook = &s_pendingRequests;

    if (pRI == NULL) {
        return 0;
    }

#ifdef MTK_GEMINI
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM */
    if (RIL_CHANNEL_SET4_OFFSET <= pRI->cid) {
		pendingRequestsMutexHook = &s_pendingRequestsMutex4;
		pendingRequestsHook = &s_pendingRequests4;
	} else if (RIL_CHANNEL_SET3_OFFSET <= pRI->cid) {
		pendingRequestsMutexHook = &s_pendingRequestsMutex3;
		pendingRequestsHook = &s_pendingRequests3;
	} else if (RIL_CHANNEL_OFFSET <= pRI->cid) {
		pendingRequestsMutexHook = &s_pendingRequestsMutex2;
		pendingRequestsHook = &s_pendingRequests2;
	}
#elif (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM */
	if (RIL_CHANNEL_SET3_OFFSET <= pRI->cid) {
	    pendingRequestsMutexHook = &s_pendingRequestsMutex3;
	    pendingRequestsHook = &s_pendingRequests3;
	} else if (RIL_CHANNEL_OFFSET <= pRI->cid) {
	    pendingRequestsMutexHook = &s_pendingRequestsMutex2;
	    pendingRequestsHook = &s_pendingRequests2;
	}
#else
    if (RIL_CHANNEL_OFFSET <= pRI->cid) {
        pendingRequestsMutexHook = &s_pendingRequestsMutex2;
        pendingRequestsHook = &s_pendingRequests2;
    }
#endif
#endif /* MTK_GEMINI */

    pthread_mutex_lock(pendingRequestsMutexHook);

    for(RequestInfo **ppCur = pendingRequestsHook
                              ; *ppCur != NULL
            ; ppCur = &((*ppCur)->p_next)
       ) {
        if (pRI == *ppCur) {
            ret = 1;

            *ppCur = (*ppCur)->p_next;
            break;
        }
    }

    pthread_mutex_unlock(pendingRequestsMutexHook);

    return ret;
}


extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI;
    int ret;
    size_t errorOffset;
    int fd = *s_SocketLinterParam1.s_fdCommand;
    RILId id = MTK_RIL_SOCKET_1;

    pRI = (RequestInfo *)t;

#ifdef MTK_GEMINI
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    if (RIL_CHANNEL_SET4_OFFSET <= pRI->cid) {
        fd = *s_SocketLinterParam4.s_fdCommand;
        id = MTK_RIL_SOCKET_4;
    } else
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    if (RIL_CHANNEL_SET3_OFFSET <= pRI->cid) {
        fd = *s_SocketLinterParam3.s_fdCommand;
        id = MTK_RIL_SOCKET_3;
    } else
#endif
    if (RIL_CHANNEL_OFFSET <= pRI->cid) {
        fd = *s_SocketLinterParam2.s_fdCommand;
        id = MTK_RIL_SOCKET_2;
    }
#endif /* MTK_GEMINI */

    if (!checkAndDequeueRequestInfo(pRI)) {
		LOGE ("RIL_onRequestComplete RIL%d: invalid RIL_Token", id+1);
        return;
    }

    if (pRI->local > 0) {
        // Locally issued command...void only!
        // response does not go back up the command socket
        LOGD("C[locl]< %s", requestToString(pRI->pCI->requestNumber));

        goto done;
    }

    appendPrintBuf("[%04d]< %s",
                   pRI->token, requestToString(pRI->pCI->requestNumber));

    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_RILD_READ_IMSI){
        RLOGD("C[locl]< %s done.", requestToString(pRI->pCI->requestNumber));

        goto done;
    }


    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL){

        LOGD("[THERMAL] local request for THERMAL returned ");
        char* strResult = NULL;
        if(RIL_E_SUCCESS == e){
            asprintf(&strResult, "%s",(char*)response);
        } else {
			asprintf(&strResult, "ERROR");

        }
		
        if(s_THERMAL_fd > 0){
            LOGD("[THERMAL] s_THERMAL_fd is valid strResult is %s", strResult);
			
            int len = (int)strlen(strResult);
            ret = send(s_THERMAL_fd, strResult, len, 0);
            if (ret != len) {
                LOGD("[THERMAL] lose data when send response. ");
            }            
            close(s_THERMAL_fd);
            s_THERMAL_fd = -1;
            free(strResult);
            goto done;
        } else {
            LOGD("[EAP] s_THERMAL_fd is < 0");
            free(strResult);
            goto done;            
        }
		
    }

#ifdef MTK_EAP_SIM_AKA
    /*handle response for local request for response*/
    if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION
        || pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_USIM_AUTHENTICATION){

        LOGD("[EAP] local request for EAP returned ");
        char* strResult = NULL;
        char* res = (char*)response;
        if(RIL_E_SUCCESS == e){             
            if(pRI->pCI->requestNumber == RIL_LOCAL_REQUEST_SIM_AUTHENTICATION
                 && strlen(res) > 24){
                strResult = new char[25];
                strResult[0] = '\0';
                if(res[0] == '0' && res[1] == '4'){                 
                    strncpy(strResult, res+2, 8);                   
                    if(res[10] == '0' && res[11] == '8'){
                        strncpy(strResult+8, res+12, 16);
                        strResult[24] = '\0';
                    }else{
                        LOGE("The length of KC is not valid.");
                    }
                }else{
                   LOGE("The length of SRES is not valid."); 
                }
            }else{
                asprintf(&strResult, "%s",res);
            }           
        }else{
            asprintf(&strResult, "ERROR:%s", failCauseToString(e));
        }

        if(s_EAPSIMAKA_fd < 0) {
           s_EAPSIMAKA_fd = s_EAPSIMAKA_fd2;
           s_EAPSIMAKA_fd2 = -1;
        }
        if(s_EAPSIMAKA_fd > 0){
            LOGD("[EAP] s_EAPSIMAKA_fd is valid strResult is %s", strResult);
            int len = (int)strlen(strResult);
            ret = send(s_EAPSIMAKA_fd, &len, sizeof(int), 0);
            if (ret != sizeof(int)) {
                LOGD("Socket write Error: when sending arg length");
            }else{                
                ret = send(s_EAPSIMAKA_fd, strResult, len, 0);
                if (ret != len) {
                    LOGD("[EAP]lose data when send response. ");
                }
            }
            close(s_EAPSIMAKA_fd);
            s_EAPSIMAKA_fd = -1;
            free(strResult);
            goto done;             
        }
    } 
#endif
    /* atci start */
    if (s_fdATCI_command > 0 && pRI->pCI->requestNumber == RIL_REQUEST_OEM_HOOK_RAW) {
        if (e == RIL_E_SUCCESS) {
            int sendLen = 0;
            int strLen = 0;
            char* data;
            data = (char*)malloc(MAX_DATA_SIZE * sizeof(char));
            memset(data, 0, MAX_DATA_SIZE);
            strLen = responselen / sizeof(char*);
            char** p_cur = (char**)response;
            char* p = data;

            for (int i = 0; i < strLen; i++) {
                memcpy(p, p_cur[i], strlen(p_cur[i]));
                p = p + strlen(p_cur[i]);
                memcpy(p, "\r\n", 2 * sizeof(char));
                p = p +  2 * sizeof(char);
            }

            LOGD("[ATCI] data sent to atci is '%s'", data);
            sendLen = send(s_fdATCI_command, data, strlen(data), 0);
            if (sendLen != (int) strlen(data)) {
                LOGD("[ATCI]lose data when send to atci. errno = %d", errno);
            }
            free(data);
        } else if(pRI->pCI->requestNumber == RIL_REQUEST_OEM_HOOK_RAW) {
            char* data = NULL;
            int sendLen = 0;
            data = (char*)malloc(MAX_DATA_SIZE * sizeof(char));
            if(data == NULL) {
                LOGE("Can't allocate data buffer");
                goto done;
            }
            memset(data, 0, MAX_DATA_SIZE);
            strcpy(data, "ERROR");
            sendLen = send(s_fdATCI_command, data, strlen(data), 0);
            if (sendLen != (int) strlen(data)) {
                LOGD("[ATCI]lose data when send to atci. errno = %d", errno);
            }
            free(data);
        }
        goto done;
    }
    /* atci end */

    if (pRI->cancelled == 0) {
        Parcel p;

        p.writeInt32 (RESPONSE_SOLICITED);
        p.writeInt32 (pRI->token);
        errorOffset = p.dataPosition();

        p.writeInt32 (e);

        if ((e == RIL_E_SUCCESS)
                || (e == RIL_E_SIM_PIN2)
                || (e == RIL_E_SIM_PUK2)
                || (e == RIL_E_PASSWORD_INCORRECT)) {
            /* process response on success */
            /* for PIN related operation we shall take retry count to framework */
            ret = pRI->pCI->responseFunction(p, response, responselen);

            /* if an error occurred, rewind and mark it */
            if (ret != 0) {
                p.setDataPosition(errorOffset);
                p.writeInt32 (ret);
            }
        } else {
            appendPrintBuf("%s returns %s", printBuf, failCauseToString(e));
        }

        if (fd < 0) {
            LOGD ("RIL onRequestComplete: Command channel closed");
        }

        LOGI("RIL SOCKET RESPONSE: %s length:%d",requestToString(pRI->pCI->requestNumber),p.dataSize());
        sendResponse(p,id);
    }

done:
    free(pRI);
}


static void
grabPartialWakeLock() {
    acquire_wake_lock(PARTIAL_WAKE_LOCK, ANDROID_WAKE_LOCK_NAME);
}

static void
releaseWakeLock() {
    release_wake_lock(ANDROID_WAKE_LOCK_NAME);
}

/**
 * Timer callback to put us back to sleep before the default timeout
 */
static void
wakeTimeoutCallback (void *param) {
    // We're using "param != NULL" as a cancellation mechanism
    if (param == NULL) {
        //LOGD("wakeTimeout: releasing wake lock");

        releaseWakeLock();
    } else {
        //LOGD("wakeTimeout: releasing wake lock CANCELLED");
    }
}

extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, void *data,
                               size_t datalen, RILId id)
{
    int unsolResponseIndex;
    int ret;
    int64_t timeReceived = 0;
    bool shouldScheduleTimeout = false;
    WakeType wakeType = WAKE_PARTIAL;

    if (s_registerCalled == 0) {
        // Ignore RIL_onUnsolicitedResponse before RIL_register
        LOGW("RIL_onUnsolicitedResponse called before RIL_register");
        return;
    }

    unsolResponseIndex = unsolResponse - RIL_UNSOL_RESPONSE_BASE;

    if ((unsolResponseIndex < 0)
            || (unsolResponseIndex >= (int32_t)NUM_ELEMS(s_unsolResponses))) {
#ifdef MTK_RIL
        if (unsolResponse > (RIL_UNSOL_MTK_BASE + (int32_t)NUM_ELEMS(s_mtk_unsolResponses)))
#endif /* MTK_RIL */
        {
            LOGE("unsupported unsolicited response code %d", unsolResponse);
            return;
        }
    }

    int skipSocket1 = 0;
    if (isInternationalRoamingEnabled() && isDualTalkMode()) {
        //LOGD("International roaming enabled and is dualtalk mode, skip check socket1");
        skipSocket1 = 1;
    }
#ifndef REVERSE_MTK_CHANGE
#ifndef MTK_RILD_UT
    if( (!skipSocket1 && (id == MTK_RIL_SOCKET_1 && *s_SocketLinterParam1.s_fdCommand == *s_SocketLinterParam1.s_fdUT_command))
#ifdef MTK_GEMINI
            || (id == MTK_RIL_SOCKET_2 && *s_SocketLinterParam2.s_fdCommand == *s_SocketLinterParam2.s_fdUT_command)
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
            || (id == MTK_RIL_SOCKET_4 && *s_SocketLinterParam4.s_fdCommand == *s_SocketLinterParam4.s_fdUT_command)
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
            || (id == MTK_RIL_SOCKET_3 && *s_SocketLinterParam3.s_fdCommand == *s_SocketLinterParam3.s_fdUT_command)
#endif
#endif
      ) {
        /* when Carkit or Wimax is connection,
         * we should not send the URCs since they didn't handle it
        */
        LOGD("Can't send URC during Carkit or Wimax connection for RIL%d, chche:%s",
             id+1, 
             requestToString(unsolResponse));

        if (MTK_RIL_SOCKET_1 == id) {
            pendedUrcList1 = cacheUrc(pendedUrcList1, unsolResponse, data, datalen , id);
        }
#ifdef MTK_GEMINI
        else if (MTK_RIL_SOCKET_2 == id) {
            pendedUrcList2 = cacheUrc(pendedUrcList2, unsolResponse, data, datalen, id);
        }
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
        else if (MTK_RIL_SOCKET_3 == id) {
            pendedUrcList3 = cacheUrc(pendedUrcList3, unsolResponse, data, datalen, id);
        }
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 3 SIM*/
        else if (MTK_RIL_SOCKET_4 == id) {
            pendedUrcList4 = cacheUrc(pendedUrcList4, unsolResponse, data, datalen, id);
        }
#endif
     #if 0
        if(RIL_UNSOL_STK_PROACTIVE_COMMAND == unsolResponse) {
            LOGD("RIL_onUnsolicitedResponse store STK proactive command temporarily");
            if (id >= MTK_RIL_SOCKET_1 && id < MTK_RIL_SOCKET_NUM) {
                s_tempStkPciFlag[id] = 1;
                // s_tempStkPci = (char *) data;
                asprintf(&s_tempStkPci[id], "%s", (char *) data);
            } 

        }

        // To queue phb ready event once the connection is not ready
        if (RIL_UNSOL_PHB_READY_NOTIFICATION == unsolResponse) {
            LOGD("RIL_onUnsolicitedResponse queue PHB ready event");
            if (id < MTK_RIL_SOCKET_NUM) {
                s_phbReadyFlag[id] = *((int *) data);
            }
        }
    #endif    
        return;
    }
#endif
#endif

    //MTK-START [mtk04070][111213][ALPS00093395] ATCI for unsolicited response
    char atci_urc_enable[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.service.atci_urc.enable", atci_urc_enable, "0");
    if ((s_fdATCI_command > 0) && 
        (RIL_UNSOL_ATCI_RESPONSE == unsolResponse) &&
        (atoi(atci_urc_enable) == 1))
    {
        char* responseData = (char*)data;
        LOGD("[ATCI] data sent to atci is '%s'", responseData);
        unsigned int sendLen = send(s_fdATCI_command, responseData, strlen(responseData), 0);
        if (sendLen != (int) strlen(responseData)) {
            LOGD("[ATCI]lose data when send to atci.");
        }
    }
    else
      {
       /* atci start */
       if (s_fdCommand == s_fdATCI_command && s_fdATCI_command != -1) {
          LOGD("[ATCI]Do not send URC");
          return;
       }
       /* atci end */
    }
    //MTK-END [mtk04070][111213][ALPS00093395] ATCI for unsolicited response

    // Grab a wake lock if needed for this reponse,
    // as we exit we'll either release it immediately
    // or set a timer to release it later.
#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_MTK_BASE) {
        unsolResponseIndex = unsolResponse - RIL_UNSOL_MTK_BASE;
        wakeType = s_mtk_unsolResponses[unsolResponseIndex].wakeType;
    }
    else
#endif /* MTK_RIL */
    {
        wakeType = s_unsolResponses[unsolResponseIndex].wakeType;

    }

    switch (wakeType) {
    case WAKE_PARTIAL:
        grabPartialWakeLock();
        shouldScheduleTimeout = true;
        break;

    case DONT_WAKE:
    default:
        // No wake lock is grabed so don't set timeout
        shouldScheduleTimeout = false;
        break;
    }

    // Mark the time this was received, doing this
    // after grabing the wakelock incase getting
    // the elapsedRealTime might cause us to goto
    // sleep.
    if (unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {
        timeReceived = elapsedRealtime();
    }

    appendPrintBuf("[UNSL]< %s", requestToString(unsolResponse));

    Parcel p;

    p.writeInt32 (RESPONSE_UNSOLICITED);
    p.writeInt32 (unsolResponse);

#ifdef MTK_RIL
    if (unsolResponse >= RIL_UNSOL_MTK_BASE) {
        ret = s_mtk_unsolResponses[unsolResponseIndex]
              .responseFunction(p, data, datalen);
    }
    else
#endif /* MTK_RIL */
    {
        ret = s_unsolResponses[unsolResponseIndex]
              .responseFunction(p, data, datalen);
    }

    if (ret != 0) {
        // Problem with the response. Don't continue;
        goto error_exit;
    }

    // some things get more payload
    switch(unsolResponse) {
    case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
#ifdef MTK_RIL
        p.writeInt32(s_callbacks.onStateRequest(id, NULL));
        appendPrintBuf("%s {%s}", printBuf,
                       radioStateToString(s_callbacks.onStateRequest(id, NULL)));
#else
        p.writeInt32(s_callbacks.onStateRequest());
        appendPrintBuf("%s {%s}", printBuf,
                       radioStateToString(s_callbacks.onStateRequest()));
#endif /* MTK_RIL */
        break;


    case RIL_UNSOL_NITZ_TIME_RECEIVED:
        // Store the time that this was received so the
        // handler of this message can account for
        // the time it takes to arrive and process. In
        // particular the system has been known to sleep
        // before this message can be processed.
        p.writeInt64(timeReceived);
        break;
    }

    LOGI("%s UNSOLICITED: %s length:%d",rilIdToString(id),requestToString(unsolResponse),p.dataSize());
    ret = sendResponse(p,id);
    if (ret != 0 && unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {

        // Unfortunately, NITZ time is not poll/update like everything
        // else in the system. So, if the upstream client isn't connected,
        // keep a copy of the last NITZ response (with receive time noted
        // above) around so we can deliver it when it is connected

        if (s_lastNITZTimeData != NULL) {
            free (s_lastNITZTimeData);
            s_lastNITZTimeData = NULL;
        }

        s_lastNITZTimeData = malloc(p.dataSize());
        s_lastNITZTimeDataSize = p.dataSize();
        memcpy(s_lastNITZTimeData, p.data(), p.dataSize());
    }

    // For now, we automatically go back to sleep after TIMEVAL_WAKE_TIMEOUT
    // FIXME The java code should handshake here to release wake lock

    if (shouldScheduleTimeout) {
        
        // Cancel the previous request
        if (s_last_wake_timeout_info != NULL) {
            s_last_wake_timeout_info->userParam = (void *)1;
        }

        s_last_wake_timeout_info
        = internalRequestTimedCallback(wakeTimeoutCallback, NULL,
                                       &TIMEVAL_WAKE_TIMEOUT);

    }

    // Normal exit
    return;

error_exit:
    // There was an error and we've got the wake lock so release it.
    if (shouldScheduleTimeout) {
        releaseWakeLock();
    }
}

/** FIXME generalize this if you track UserCAllbackInfo, clear it
    when the callback occurs
*/
static UserCallbackInfo *
internalRequestTimedCallback (RIL_TimedCallback callback, void *param,
                              const struct timeval *relativeTime)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) malloc (sizeof(UserCallbackInfo));

    p_info->p_callback = callback;
    p_info->userParam = param;
#ifdef MTK_RIL
    p_info->cid = (RILChannelId)-1;
#endif

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}


extern "C" void
RIL_requestTimedCallback (RIL_TimedCallback callback, void *param,
                          const struct timeval *relativeTime) {
    internalRequestTimedCallback (callback, param, relativeTime);
}

#ifdef MTK_RIL
static UserCallbackInfo *
internalRequestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                              const struct timeval *relativeTime,
                              int proxyId)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) malloc (sizeof(UserCallbackInfo));

    p_info->p_callback = callback;
    p_info->userParam = param;
    p_info->cid = (RILChannelId)proxyId;

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}


extern "C" void
RIL_requestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                          const struct timeval *relativeTime,
                          RILChannelId proxyId) {
    internalRequestProxyTimedCallback (callback, param,
                relativeTime, proxyId);
}

extern "C" RILChannelId
RIL_queryMyChannelId(RIL_Token t)
{
    return ((RequestInfo *) t)->cid;
}

extern "C" int
RIL_queryMyProxyIdByThread()
{
    pthread_t selfThread = pthread_self();
    pthread_mutex_lock(&s_queryThreadMutex);
    int i;
    for (i=0; i< RIL_SUPPORT_PROXYS; i++)
    {
        if (0 != pthread_equal(s_tid_proxy[i], selfThread))
        {
            pthread_mutex_unlock(&s_queryThreadMutex);
            return i;
        }
    }
    pthread_mutex_unlock(&s_queryThreadMutex);
    return -1;
}
#endif

const char *
rilIdToString(RILId id)
{
    switch(id) {
    case MTK_RIL_SOCKET_1:
        return "RIL(1)";
#ifdef MTK_GEMINI
    case MTK_RIL_SOCKET_2:
        return "RIL(2)";
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    case MTK_RIL_SOCKET_4:
        return "RIL(4)";
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    case MTK_RIL_SOCKET_3:
        return "RIL(3)";
#endif
#endif
    default:
        return "not a valid RIL";
    }
}

const char *
failCauseToString(RIL_Errno e)
{
    switch(e) {
    case RIL_E_SUCCESS:
        return "E_SUCCESS";
    case RIL_E_RADIO_NOT_AVAILABLE:
        return "E_RAIDO_NOT_AVAILABLE";
    case RIL_E_GENERIC_FAILURE:
        return "E_GENERIC_FAILURE";
    case RIL_E_PASSWORD_INCORRECT:
        return "E_PASSWORD_INCORRECT";
    case RIL_E_SIM_PIN2:
        return "E_SIM_PIN2";
    case RIL_E_SIM_PUK2:
        return "E_SIM_PUK2";
    case RIL_E_REQUEST_NOT_SUPPORTED:
        return "E_REQUEST_NOT_SUPPORTED";
    case RIL_E_CANCELLED:
        return "E_CANCELLED";
    case RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL:
        return "E_OP_NOT_ALLOWED_DURING_VOICE_CALL";
    case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW:
        return "E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
    case RIL_E_SMS_SEND_FAIL_RETRY:
        return "E_SMS_SEND_FAIL_RETRY";
    case RIL_E_SIM_ABSENT:
        return "E_SIM_ABSENT";
    case RIL_E_ILLEGAL_SIM_OR_ME:
        return "E_ILLEGAL_SIM_OR_ME";
    case RIL_E_BT_SAP_UNDEFINED:
        return "RIL_E_BT_SAP_UNDEFINED";
    case RIL_E_BT_SAP_NOT_ACCESSIBLE:
        return "RIL_E_BT_SAP_NOT_ACCESSIBLE";
    case RIL_E_BT_SAP_CARD_REMOVED:
        return  "RIL_E_BT_SAP_CARD_REMOVED";
#ifdef FEATURE_MULTIMODE_ANDROID
    case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:
        return "E_SUBSCRIPTION_NOT_AVAILABLE";
    case RIL_E_MODE_NOT_SUPPORTED:
        return "E_MODE_NOT_SUPPORTED";
#endif
    default:
        return "<unknown error>";
    }
}

const char *
radioStateToString(RIL_RadioState s) {
    switch(s) {
    case RADIO_STATE_OFF:
        return "RADIO_OFF";
    case RADIO_STATE_UNAVAILABLE:
        return "RADIO_UNAVAILABLE";
    case RADIO_STATE_SIM_NOT_READY:
        return "RADIO_SIM_NOT_READY";
    case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
        return "RADIO_SIM_LOCKED_OR_ABSENT";
    case RADIO_STATE_SIM_READY:
        return "RADIO_SIM_READY";
    case RADIO_STATE_RUIM_NOT_READY:
        return"RADIO_RUIM_NOT_READY";
    case RADIO_STATE_RUIM_READY:
        return"RADIO_RUIM_READY";
    case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
        return"RADIO_RUIM_LOCKED_OR_ABSENT";
    case RADIO_STATE_NV_NOT_READY:
        return"RADIO_NV_NOT_READY";
    case RADIO_STATE_NV_READY:
        return"RADIO_NV_READY";
    default:
        return "<unknown state>";
    }
}

const char *
callStateToString(RIL_CallState s) {
    switch(s) {
    case RIL_CALL_ACTIVE :
        return "ACTIVE";
    case RIL_CALL_HOLDING:
        return "HOLDING";
    case RIL_CALL_DIALING:
        return "DIALING";
    case RIL_CALL_ALERTING:
        return "ALERTING";
    case RIL_CALL_INCOMING:
        return "INCOMING";
    case RIL_CALL_WAITING:
        return "WAITING";
    default:
        return "<unknown state>";
    }
}

static AtResponseList* cacheUrc(AtResponseList* pendedUrcList,
                                  int unsolResponse, 
                                  void *data,
                                  size_t datalen, 
                                  RILId id){
    //Only the URC list we wanted.
    if (unsolResponse != RIL_UNSOL_STK_PROACTIVE_COMMAND
        && unsolResponse != RIL_UNSOL_CALL_FORWARDING
        && unsolResponse != RIL_UNSOL_SMS_READY_NOTIFICATION
        && unsolResponse != RIL_UNSOL_PHB_READY_NOTIFICATION
        && unsolResponse != RIL_UNSOL_SIM_SMS_STORAGE_FULL
        && unsolResponse != RIL_UNSOL_ME_SMS_STORAGE_FULL) {
        return pendedUrcList;
    }
    AtResponseList* urcCur = pendedUrcList;
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;
    while (urcCur != NULL) {
        LOGD("Pended URC:%d, RILD:%d, :%s",
            pendedUrcCount, 
            id+1, 
            requestToString(urcCur->id));
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (AtResponseList*)malloc(sizeof(AtResponseList));
    if (urcPrev != NULL)
        urcPrev->pNext = urcCur;
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)malloc(datalen + 1);
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);
    LOGD("pendedUrcCount = %d", pendedUrcCount + 1);

    if (pendedUrcList == NULL)
        return urcCur;
    return pendedUrcList;
}

static void sendUrc(RILId rid, AtResponseList* urcCached) {
    AtResponseList* urc = urcCached;
    AtResponseList* urc_temp;
    while (urc != NULL) {
        LOGD("sendPendedUrcs RIL%d, %s", 
        rid + 1, 
        requestToString(urc->id)); 
        RIL_onUnsolicitedResponse ( urc->id, urc->data, urc->datalen, rid);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

static void sendPendedUrcs(RILId rid) {
    if ((MTK_RIL_SOCKET_1 == rid) && (*s_SocketLinterParam1.s_fdCommand != -1)) {
        sendUrc(rid, pendedUrcList1);
        pendedUrcList1 = NULL;
    }
#ifdef MTK_GEMINI 
    else if (isGeminiMode()){
        if ((MTK_RIL_SOCKET_2 == rid) && (*s_SocketLinterParam2.s_fdCommand != -1)) {
            sendUrc(rid, pendedUrcList2);
            pendedUrcList2 = NULL;
        }
    #if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
        else if ((MTK_RIL_SOCKET_3 == rid) && (*s_SocketLinterParam3.s_fdCommand != -1)) {
            sendUrc(rid, pendedUrcList3);
            pendedUrcList3 = NULL;
        }
    #endif
    #if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
        else if ((MTK_RIL_SOCKET_4 == rid) && (*s_SocketLinterParam4.s_fdCommand != -1)) {
            sendUrc(rid, pendedUrcList4);
            pendedUrcList4 = NULL;
        }
    #endif
    }
#endif
}

static void onTempStkPci(RILId rid) {
    if ((rid >= MTK_RIL_SOCKET_1 && rid < MTK_RIL_SOCKET_NUM) && s_tempStkPciFlag[rid] != 0) {
        LOGD("release temp stk pci string for RIL%d", rid);
        s_tempStkPciFlag[rid] = 0;
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_STK_PROACTIVE_COMMAND,
            s_tempStkPci[rid],
            strlen(s_tempStkPci[rid]),
            rid);
        free(s_tempStkPci[rid]);
        s_tempStkPci[rid] = NULL;
    }
}

#ifndef REVERSE_MTK_CHANGE
#ifndef MTK_RILD_UT
static void storeEcfuUrc(int unsolResponse, RILId id, void *data) {
   if (unsolResponse == RIL_UNSOL_CALL_FORWARDING) {
       if ((MTK_RIL_SOCKET_1 == id) && (s_fdCommand < 0)) {
           if (s_tempEcfuCount_1 < ECFU_COUNT) {
               s_tempEcfu_1[s_tempEcfuCount_1][0] = ((int*)data)[0];
               s_tempEcfu_1[s_tempEcfuCount_1][1] = ((int*)data)[1];
               LOGD("Store Ecfu URC[1][%d] = %d, %d", s_tempEcfuCount_1, 
                    s_tempEcfu_1[s_tempEcfuCount_1][0],
                    s_tempEcfu_1[s_tempEcfuCount_1][1]);
               s_tempEcfuCount_1++;
           }
       } 
#ifdef MTK_GEMINI
       else if ((MTK_RIL_SOCKET_2 == id) && (s_fdCommand2 < 0)) {
           if (s_tempEcfuCount_2 < ECFU_COUNT) {
               s_tempEcfu_2[s_tempEcfuCount_2][0] = ((int*)data)[0];
               s_tempEcfu_2[s_tempEcfuCount_2][1] = ((int*)data)[1];
               LOGD("Store Ecfu URC[2][%d] = %d, %d", s_tempEcfuCount_2, 
                    s_tempEcfu_2[s_tempEcfuCount_2][0],
                    s_tempEcfu_2[s_tempEcfuCount_2][1]);
               s_tempEcfuCount_2++;
           } 
       }
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
       else if ((MTK_RIL_SOCKET_3 == id) && (s_fdCommand3 < 0)) {
           if (s_tempEcfuCount_3 < ECFU_COUNT) {
               s_tempEcfu_3[s_tempEcfuCount_3][0] = ((int*)data)[0];
               s_tempEcfu_3[s_tempEcfuCount_3][1] = ((int*)data)[1];
               LOGD("Store Ecfu URC[3][%d] = %d, %d", s_tempEcfuCount_3, 
                    s_tempEcfu_3[s_tempEcfuCount_3][0],
                    s_tempEcfu_3[s_tempEcfuCount_3][1]);
               s_tempEcfuCount_3++;
           } 
       }
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
       else if ((MTK_RIL_SOCKET_4 == id) && (s_fdCommand4 < 0)) {
           if (s_tempEcfuCount_4 < ECFU_COUNT) {
               s_tempEcfu_4[s_tempEcfuCount_4][0] = ((int*)data)[0];
               s_tempEcfu_4[s_tempEcfuCount_4][1] = ((int*)data)[1];
               LOGD("Store Ecfu URC[4][%d] = %d, %d", s_tempEcfuCount_4, 
                    s_tempEcfu_4[s_tempEcfuCount_4][0],
                    s_tempEcfu_4[s_tempEcfuCount_4][1]);
               s_tempEcfuCount_4++;
           } 
       }
#endif
#endif
   }
}

static void onEcfu(RILId rid) {
    int i;
    if ((MTK_RIL_SOCKET_1 == rid) && (s_tempEcfuCount_1 > 0)) {
        LOGD("Send Ecfu URC[1] to RILJ, count = %d", s_tempEcfuCount_1);
        for (i = 0; i < s_tempEcfuCount_1; i++) {
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_CALL_FORWARDING,
                s_tempEcfu_1[i],
                2 * sizeof(int *),
                rid);
        }
        s_tempEcfuCount_1 = 0;	
    } 
#ifdef MTK_GEMINI
    else if ((MTK_RIL_SOCKET_2 == rid) && (s_tempEcfuCount_2 > 0)) {
        LOGD("Send Ecfu URC[2] to RILJ, count = %d", s_tempEcfuCount_2);
        for (i = 0; i < s_tempEcfuCount_2; i++) {
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_CALL_FORWARDING,
                s_tempEcfu_2[i],
                2 * sizeof(int *),
                rid);
        }
        s_tempEcfuCount_2 = 0;
    }
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    else if ((MTK_RIL_SOCKET_3 == rid) && (s_tempEcfuCount_3 > 0)) {
        LOGD("Send Ecfu URC[3] to RILJ, count = %d", s_tempEcfuCount_3);
        for (i = 0; i < s_tempEcfuCount_3; i++) {
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_CALL_FORWARDING,
                s_tempEcfu_3[i],
                2 * sizeof(int *),
                rid);
        }
        s_tempEcfuCount_3 = 0;
    }
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    else if ((MTK_RIL_SOCKET_4 == rid) && (s_tempEcfuCount_4 > 0)) {
        LOGD("Send Ecfu URC[4] to RILJ, count = %d", s_tempEcfuCount_4);
        for (i = 0; i < s_tempEcfuCount_4; i++) {
            RIL_onUnsolicitedResponse (
                RIL_UNSOL_CALL_FORWARDING,
                s_tempEcfu_4[i],
                2 * sizeof(int *),
                rid);
        }
        s_tempEcfuCount_4 = 0;
    }
#endif
#endif
}
#endif
#endif

} /* namespace android */

const char *
requestToString(int request) {
    /*
     cat libs/telephony/ril_commands.h \
     | egrep "^ *{RIL_" \
     | sed -re 's/\{RIL_([^,]+),[^,]+,([^}]+).+/case RIL_\1: return "\1";/'


     cat libs/telephony/ril_unsol_commands.h \
     | egrep "^ *{RIL_" \
     | sed -re 's/\{RIL_([^,]+),([^}]+).+/case RIL_\1: return "\1";/'

    */
    switch(request) {
    case RIL_REQUEST_GET_SIM_STATUS:
        return "GET_SIM_STATUS";
    case RIL_REQUEST_ENTER_SIM_PIN:
        return "ENTER_SIM_PIN";
    case RIL_REQUEST_ENTER_SIM_PUK:
        return "ENTER_SIM_PUK";
    case RIL_REQUEST_ENTER_SIM_PIN2:
        return "ENTER_SIM_PIN2";
    case RIL_REQUEST_ENTER_SIM_PUK2:
        return "ENTER_SIM_PUK2";
    case RIL_REQUEST_CHANGE_SIM_PIN:
        return "CHANGE_SIM_PIN";
    case RIL_REQUEST_CHANGE_SIM_PIN2:
        return "CHANGE_SIM_PIN2";
    case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION:
        return "ENTER_NETWORK_DEPERSONALIZATION";
    case RIL_REQUEST_GET_CURRENT_CALLS:
        return "GET_CURRENT_CALLS";
    case RIL_REQUEST_DIAL:
        return "DIAL";
    case RIL_REQUEST_EMERGENCY_DIAL :
        return "EMERGENCY_DIAL";
#ifdef MTK_VT3G324M_SUPPORT
    case RIL_REQUEST_VT_DIAL:
        return "VT_DIAL";
    case RIL_REQUEST_VOICE_ACCEPT:
        return "VOICE_ACCEPT";
#endif
    case RIL_REQUEST_GET_IMSI:
        return "GET_IMSI";
    case RIL_REQUEST_HANGUP:
        return "HANGUP";
    case RIL_REQUEST_HANGUP_ALL:
        return "HANGUP_ALL";
    case RIL_REQUEST_HANGUP_ALL_EX:
        return "HANGUP_ALL_EX";
    case RIL_REQUEST_GET_CCM:
        return "GET_CCM";
    case RIL_REQUEST_GET_ACM:
        return "GET_ACM";
    case RIL_REQUEST_GET_ACMMAX:
        return "GET_ACMMAX";
    case RIL_REQUEST_GET_PPU_AND_CURRENCY:
        return "GET_PPU_AND_CURRENCY";
    case RIL_REQUEST_SET_ACMMAX:
        return "SET_ACMMAX";
    case RIL_REQUEST_RESET_ACM:
        return "RESET_ACM";
    case RIL_REQUEST_SET_PPU_AND_CURRENCY:
        return "SET_PPU_AND_CURRENCY";
    case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        return "HANGUP_WAITING_OR_BACKGROUND";
    case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
    case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
        return "SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
    case RIL_REQUEST_CONFERENCE:
        return "CONFERENCE";
    case RIL_REQUEST_UDUB:
        return "UDUB";
    case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
        return "LAST_CALL_FAIL_CAUSE";
    case RIL_REQUEST_SIGNAL_STRENGTH:
        return "SIGNAL_STRENGTH";
    case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        return "VOICE_REGISTRATION_STATE";
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
        return "DATA_REGISTRATION_STATE";
    case RIL_REQUEST_OPERATOR:
        return "OPERATOR";
    case RIL_REQUEST_RADIO_POWER:
        return "RADIO_POWER";
    case RIL_REQUEST_DTMF:
        return "DTMF";
    case RIL_REQUEST_SEND_SMS:
        return "SEND_SMS";
    case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
        return "SEND_SMS_EXPECT_MORE";
    case RIL_REQUEST_SETUP_DATA_CALL:
        return "SETUP_DATA_CALL";
    case RIL_REQUEST_SIM_IO:
        return "SIM_IO";
    case RIL_REQUEST_SEND_USSD:
        return "SEND_USSD";
    case RIL_REQUEST_CANCEL_USSD:
        return "CANCEL_USSD";
    case RIL_REQUEST_GET_CLIR:
        return "GET_CLIR";
    case RIL_REQUEST_SET_CLIR:
        return "SET_CLIR";
    case RIL_REQUEST_GET_COLP:
        return "GET_COLP";
    case RIL_REQUEST_SET_COLP:
        return "SET_COLP";
    case RIL_REQUEST_GET_COLR:
        return "GET_COLR";
    case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
        return "QUERY_CALL_FORWARD_STATUS";
    case RIL_REQUEST_SET_CALL_FORWARD:
        return "SET_CALL_FORWARD";
    case RIL_REQUEST_QUERY_CALL_WAITING:
        return "QUERY_CALL_WAITING";
    case RIL_REQUEST_SET_CALL_WAITING:
        return "SET_CALL_WAITING";
    case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION:
        return "SET_SUPP_SVC_NOTIFICATION";
    case RIL_REQUEST_SMS_ACKNOWLEDGE:
        return "SMS_ACKNOWLEDGE";
    case RIL_REQUEST_GET_IMEI:
        return "GET_IMEI";
    case RIL_REQUEST_GET_IMEISV:
        return "GET_IMEISV";
    case RIL_REQUEST_ANSWER:
        return "ANSWER";
    case RIL_REQUEST_DEACTIVATE_DATA_CALL:
        return "DEACTIVATE_DATA_CALL";
    case RIL_REQUEST_QUERY_FACILITY_LOCK:
        return "QUERY_FACILITY_LOCK";
    case RIL_REQUEST_SET_FACILITY_LOCK:
        return "SET_FACILITY_LOCK";
    case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
        return "CHANGE_BARRING_PASSWORD";
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
        return "QUERY_NETWORK_SELECTION_MODE";
    case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
        return "SET_NETWORK_SELECTION_AUTOMATIC";
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
        return "SET_NETWORK_SELECTION_MANUAL";
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT:
        return "SET_NETWORK_SELECTION_MANUAL_WIT_ACT";
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS :
        return "QUERY_AVAILABLE_NETWORKS ";
    case RIL_REQUEST_DTMF_START:
        return "DTMF_START";
    case RIL_REQUEST_DTMF_STOP:
        return "DTMF_STOP";
    case RIL_REQUEST_BASEBAND_VERSION:
        return "BASEBAND_VERSION";
    case RIL_REQUEST_SEPARATE_CONNECTION:
        return "SEPARATE_CONNECTION";
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        return "SET_PREFERRED_NETWORK_TYPE";
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
        return "GET_PREFERRED_NETWORK_TYPE";
    case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
        return "GET_NEIGHBORING_CELL_IDS";
    case RIL_REQUEST_SET_MUTE:
        return "SET_MUTE";
    case RIL_REQUEST_GET_MUTE:
        return "GET_MUTE";
    case RIL_REQUEST_QUERY_CLIP:
        return "QUERY_CLIP";
    case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
        return "LAST_DATA_CALL_FAIL_CAUSE";
    case RIL_REQUEST_DATA_CALL_LIST:
        return "DATA_CALL_LIST";
    case RIL_REQUEST_RESET_RADIO:
        return "RESET_RADIO";
    case RIL_REQUEST_OEM_HOOK_RAW:
        return "OEM_HOOK_RAW";
    case RIL_REQUEST_OEM_HOOK_STRINGS:
        return "OEM_HOOK_STRINGS";
    case RIL_REQUEST_SET_BAND_MODE:
        return "SET_BAND_MODE";
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        return "QUERY_AVAILABLE_BAND_MODE";
    case RIL_REQUEST_STK_GET_PROFILE:
        return "STK_GET_PROFILE";
    case RIL_REQUEST_STK_SET_PROFILE:
        return "STK_SET_PROFILE";
    case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
        return "STK_SEND_ENVELOPE_COMMAND";
    case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
        return "STK_SEND_TERMINAL_RESPONSE";
    case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM";
    case RIL_REQUEST_SCREEN_STATE:
        return "SCREEN_STATE";
    case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
        return "EXPLICIT_CALL_TRANSFER";
    case RIL_REQUEST_SET_LOCATION_UPDATES:
        return "SET_LOCATION_UPDATES";
    case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:
        return"CDMA_SET_SUBSCRIPTION_SOURCE";
    case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
        return"CDMA_SET_ROAMING_PREFERENCE";
    case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
        return"CDMA_QUERY_ROAMING_PREFERENCE";
    case RIL_REQUEST_SET_TTY_MODE:
        return"SET_TTY_MODE";
    case RIL_REQUEST_QUERY_TTY_MODE:
        return"QUERY_TTY_MODE";
    case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:
        return"CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE";
    case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
        return"CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE";
    case RIL_REQUEST_CDMA_FLASH:
        return"CDMA_FLASH";
    case RIL_REQUEST_CDMA_BURST_DTMF:
        return"CDMA_BURST_DTMF";
    case RIL_REQUEST_CDMA_SEND_SMS:
        return"CDMA_SEND_SMS";
    case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
        return"CDMA_SMS_ACKNOWLEDGE";
    case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
        return"GSM_GET_BROADCAST_SMS_CONFIG";
    case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        return"GSM_SET_BROADCAST_SMS_CONFIG";
    case RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
        return "CDMA_GET_BROADCAST_SMS_CONFIG";
    case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
        return "CDMA_SET_BROADCAST_SMS_CONFIG";
    case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
        return "CDMA_SMS_BROADCAST_ACTIVATION";
    case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY:
        return"CDMA_VALIDATE_AND_WRITE_AKEY";
    case RIL_REQUEST_CDMA_SUBSCRIPTION:
        return"CDMA_SUBSCRIPTION";
    case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
        return "CDMA_WRITE_SMS_TO_RUIM";
    case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM:
        return "CDMA_DELETE_SMS_ON_RUIM";
    case RIL_REQUEST_DEVICE_IDENTITY:
        return "DEVICE_IDENTITY";
    case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
        return "EXIT_EMERGENCY_CALLBACK_MODE";
    case RIL_REQUEST_GET_SMSC_ADDRESS:
        return "GET_SMSC_ADDRESS";
    case RIL_REQUEST_SET_SMSC_ADDRESS:
        return "SET_SMSC_ADDRESS";
    case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
        return "REPORT_SMS_MEMORY_STATUS";
    case RIL_REQUEST_DUAL_SIM_MODE_SWITCH:
        return "DUAL_SIM_MODE_SWITCH";
    case RIL_REQUEST_RADIO_POWERON:
        return "RADIO_POWERON";
    case RIL_REQUEST_RADIO_POWEROFF:
        return "RADIO_POWEROFF";
    case RIL_REQUEST_QUERY_PHB_STORAGE_INFO:
        return "QUERY_PHB_STORAGE_INFO";
    case RIL_REQUEST_WRITE_PHB_ENTRY:
        return "WRITE_PHB_ENTRY";
    case RIL_REQUEST_READ_PHB_ENTRY:
        return "READ_PHB_ENTRY";
    case RIL_REQUEST_SET_GPRS_CONNECT_TYPE:
        return "RIL_REQUEST_SET_GPRS_CONNECT_TYPE";
    case RIL_REQUEST_SET_GPRS_TRANSFER_TYPE:
        return "RIL_REQUEST_SET_GPRS_TRANSFER_TYPE";
    case RIL_REQUEST_MOBILEREVISION_AND_IMEI:
        return "RIL_REQUEST_MOBILEREVISION_AND_IMEI";//Add by mtk80372 for Barcode Number
    case RIL_REQUEST_SET_SCRI:
        return "RIL_REQUEST_SET_SCRI";
    case RIL_REQUEST_BTSIM_CONNECT:
        return "RIL_REQUEST_BTSIM_CONNECT";
    case RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF:
        return "RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF";
    case RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM:
        return "RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM";
    case RIL_REQUEST_BTSIM_TRANSFERAPDU:
        return "RIL_REQUEST_SEND_BTSIM_TRANSFERAPDU";
    case RIL_REQUEST_QUERY_ICCID:
        return "RIL_REQUEST_QUERY_ICCID";
    case RIL_REQUEST_SIM_AUTHENTICATION:
        return "RIL_REQUEST_SIM_AUTHENTICATION";
    case RIL_REQUEST_USIM_AUTHENTICATION:
        return "RIL_REQUEST_USIM_AUTHENTICATION";
    case RIL_REQUEST_FORCE_RELEASE_CALL:
        return "RIL_REQUEST_FORCE_RELEASE_CALL";
    case RIL_REQUEST_SET_CALL_INDICATION:
        return "RIL_REQUEST_SET_CALL_INDICATION";
    case RIL_REQUEST_REPLACE_VT_CALL:
        return "RIL_REQUEST_REPLACE_VT_CALL";
    case RIL_REQUEST_GET_3G_CAPABILITY: return "RIL_REQUEST_GET_3G_CAPABILITY";
    case RIL_REQUEST_SET_3G_CAPABILITY: return "RIL_REQUEST_SET_3G_CAPABILITY";
    case RIL_REQUEST_GET_POL_CAPABILITY: 
        return "RIL_REQUEST_GET_POL_CAPABILITY";
    case RIL_REQUEST_GET_POL_LIST: 
        return "RIL_REQUEST_GET_POL_LIST";
    case RIL_REQUEST_SET_POL_ENTRY: 
        return "RIL_REQUEST_SET_POL_ENTRY";     
    case RIL_REQUEST_QUERY_UPB_CAPABILITY:
        return "RIL_REQUEST_QUERY_UPB_CAPABILITY";
    case RIL_REQUEST_EDIT_UPB_ENTRY:
        return "RIL_REQUEST_EDIT_UPB_ENTRY";
    case RIL_REQUEST_DELETE_UPB_ENTRY:
        return "RIL_REQUEST_DELETE_UPB_ENTRY";
    case RIL_REQUEST_READ_UPB_GAS_LIST:
        return "RIL_REQUEST_READ_UPB_GAS_LIST";
    case RIL_REQUEST_READ_UPB_GRP:
        return "RIL_REQUEST_READ_UPB_GRP";
    case RIL_REQUEST_WRITE_UPB_GRP:
        return "RIL_REQUEST_WRITE_UPB_GRP";
    case RIL_REQUEST_DISABLE_VT_CAPABILITY:
        return "RIL_REQUEST_DISABLE_VT_CAPABILITY";
    case RIL_REQUEST_SET_SIM_RECOVERY_ON:
            return "RIL_REQUEST_SET_SIM_RECOVERY_ON";
    case RIL_REQUEST_GET_SIM_RECOVERY_ON:
            return "RIL_REQUEST_GET_SIM_RECOVERY_ON";
    case RIL_REQUEST_SET_TRM:  
        return "RIL_REQUEST_SET_TRM";       
    case RIL_REQUEST_VOICE_RADIO_TECH:
            return "RIL_REQUEST_VOICE_RADIO_TECH";              
    case RIL_REQUEST_GET_CELL_INFO_LIST: 
        return"GET_CELL_INFO_LIST"; // MR2
    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
        return "RIL_REQUEST_SET_INITIAL_ATTACH_APN";
    case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE: 
        return"SET_UNSOL_CELL_INFO_LIST_RATE"; // MR2
    case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
        return "UNSOL_RESPONSE_RADIO_STATE_CHANGED";
    case RIL_REQUEST_GET_SMS_SIM_MEM_STATUS:
        return "RIL_REQUEST_GET_SMS_SIM_MEM_STATUS";
    case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
        return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
    case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
        return "UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED";
    case RIL_UNSOL_RESPONSE_NEW_SMS:
        return "UNSOL_RESPONSE_NEW_SMS";
    case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT:
        return "UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT";
    case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM:
        return "UNSOL_RESPONSE_NEW_SMS_ON_SIM";
    case RIL_UNSOL_ON_USSD:
        return "UNSOL_ON_USSD";
    case RIL_UNSOL_ON_USSD_REQUEST:
        return "UNSOL_ON_USSD_REQUEST(obsolete)";
    case RIL_UNSOL_NITZ_TIME_RECEIVED:
        return "UNSOL_NITZ_TIME_RECEIVED";
    case RIL_UNSOL_SIGNAL_STRENGTH:
        return "UNSOL_SIGNAL_STRENGTH";
    case RIL_UNSOL_STK_SESSION_END:
        return "UNSOL_STK_SESSION_END";
    case RIL_UNSOL_STK_PROACTIVE_COMMAND:
        return "UNSOL_STK_PROACTIVE_COMMAND";
    case RIL_UNSOL_STK_EVENT_NOTIFY:
        return "UNSOL_STK_EVENT_NOTIFY";
    case RIL_UNSOL_STK_CALL_SETUP:
        return "UNSOL_STK_CALL_SETUP";
    case RIL_UNSOL_SIM_SMS_STORAGE_FULL:
        return "UNSOL_SIM_SMS_STORAGE_FUL";
    case RIL_UNSOL_SIM_REFRESH:
        return "UNSOL_SIM_REFRESH";
    case RIL_UNSOL_DATA_CALL_LIST_CHANGED:
        return "UNSOL_DATA_CALL_LIST_CHANGED";
    case RIL_UNSOL_CALL_RING:
        return "UNSOL_CALL_RING";
    case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED:
        return "UNSOL_RESPONSE_SIM_STATUS_CHANGED";
    case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS:
        return "UNSOL_NEW_CDMA_SMS";
    case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS:
        return "UNSOL_NEW_BROADCAST_SMS";
    case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL:
        return "UNSOL_CDMA_RUIM_SMS_STORAGE_FULL";
    case RIL_UNSOL_RESTRICTED_STATE_CHANGED:
        return "UNSOL_RESTRICTED_STATE_CHANGED";
    case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
        return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
    case RIL_UNSOL_CDMA_CALL_WAITING:
        return "UNSOL_CDMA_CALL_WAITING";
    case RIL_UNSOL_CDMA_OTA_PROVISION_STATUS:
        return "UNSOL_CDMA_OTA_PROVISION_STATUS";
    case RIL_UNSOL_CDMA_INFO_REC:
        return "UNSOL_CDMA_INFO_REC";
    case RIL_UNSOL_OEM_HOOK_RAW:
        return "UNSOL_OEM_HOOK_RAW";
    case RIL_UNSOL_RINGBACK_TONE:
        return "UNSOL_RINGBACK_TONE";
    case RIL_UNSOL_RESEND_INCALL_MUTE:
        return "UNSOL_RESEND_INCALL_MUTE";
    case RIL_UNSOL_CELL_INFO_LIST: 
        return "UNSOL_CELL_INFO_LIST"; // MR2
        /* MTK Proprietary URC */
    case RIL_UNSOL_NEIGHBORING_CELL_INFO:
        return "UNSOL_NEIGHBORING_CELL_INFO";
    case RIL_UNSOL_NETWORK_INFO:
        return "UNSOL_NETWORK_INFO";
    case RIL_UNSOL_CALL_FORWARDING:
        return "UNSOL_CALL_FORWARDING";
    case RIL_UNSOL_CRSS_NOTIFICATION:
        return "UNSOL_CRSS_NOTIFICATION";
    case RIL_UNSOL_CALL_PROGRESS_INFO:
        return "UNSOL_CALL_PROGRESS_INFO";
    case RIL_UNSOL_PHB_READY_NOTIFICATION:
        return "RIL_UNSOL_PHB_READY_NOTIFICATION";
    case RIL_UNSOL_SIM_INSERTED_STATUS:
        return "UNSOL_SIM_INSERTED_STATUS";
    case RIL_UNSOL_SIM_MISSING:
        return "UNSOL_SIM_MISSING";
    case RIL_UNSOL_SIM_RECOVERY:
        return "RIL_UNSOL_SIM_RECOVERY";
    case RIL_UNSOL_VIRTUAL_SIM_ON:
        return "RIL_UNSOL_VIRTUAL_SIM_ON";
    case RIL_UNSOL_VIRTUAL_SIM_OFF:
        return "RIL_UNSOL_VIRTUAL_SIM_OFF";
    case RIL_UNSOL_SPEECH_INFO:
        return "UNSOL_SPEECH_INFO";
    case RIL_UNSOL_RADIO_TEMPORARILY_UNAVAILABLE:
        return "UNSOL_RADIO_TEMPORARILY_UNAVAILABLE";
    case RIL_UNSOL_ME_SMS_STORAGE_FULL:
        return "RIL_UNSOL_ME_SMS_STORAGE_FULL";
    case RIL_UNSOL_SMS_READY_NOTIFICATION:
        return "RIL_UNSOL_SMS_READY_NOTIFICATION";
#ifdef MTK_VT3G324M_SUPPORT
    case RIL_UNSOL_VT_STATUS_INFO:
        return "UNSOL_VT_STATUS_INFO";
    case RIL_UNSOL_VT_RING_INFO:
        return "UNSOL_VT_RING_INFO";
#endif
    case RIL_UNSOL_INCOMING_CALL_INDICATION:
        return "UNSOL_INCOMING_CALL_INDICATION";
    case RIL_UNSOL_SCRI_RESULT:
        return "UNSOL_SCRI_RESULT";        
    case RIL_UNSOL_GPRS_DETACH:
        return "RIL_UNSOL_GPRS_DETACH";
    case RIL_REQUEST_DELETE_SMS_ON_SIM:
        return "RIL_REQUEST_DELETE_SMS_ON_SIM";
    case RIL_LOCAL_REQUEST_SIM_AUTHENTICATION:
        return "RIL_LOCAL_REQUEST_SIM_AUTHENTICATION";
    case RIL_LOCAL_REQUEST_USIM_AUTHENTICATION:
        return "RIL_LOCAL_REQUEST_USIM_AUTHENTICATION";     
    case RIL_UNSOL_IMEI_LOCK: 
        return "RIL_UNSOL_IMEI_LOCK";
    case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED:
        return "RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED";
    case RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED:
        return "RIL_UNSOL_RESPONSE_MMRR_STATUS_CHANGED"; 
    case RIL_REQUEST_DETECT_SIM_MISSING:
        return "RIL_REQUEST_DETECT_SIM_MISSING";
    //MTK-START [mtk80950][120410][ALPS00266631xxxxxxxx]check whether download calibration data or not
    case RIL_REQUEST_GET_CALIBRATION_DATA:
        return "RIL_REQUEST_GET_CALIBRATION_DATA";
    //MTK-END [mtk80950][120410][ALPS00266631]check whether download calibration data or not
    case RIL_REQUEST_GET_PHB_STRING_LENGTH:
        return "RIL_REQUEST_GET_PHB_STRING_LENGTH";
    case RIL_REQUEST_GET_PHB_MEM_STORAGE:
        return "RIL_REQUEST_GET_PHB_MEM_STORAGE";
    case RIL_REQUEST_SET_PHB_MEM_STORAGE:
        return "RIL_REQUEST_SET_PHB_MEM_STORAGE";
    case RIL_REQUEST_READ_PHB_ENTRY_EXT:
        return "RIL_REQUEST_READ_PHB_ENTRY_EXT";
    case RIL_REQUEST_WRITE_PHB_ENTRY_EXT:
        return "RIL_REQUEST_WRITE_PHB_ENTRY_EXT";
    case RIL_UNSOL_RESPONSE_ACMT:
        return "RIL_UNSOL_RESPONSE_ACMT";
    case RIL_UNSOL_EF_CSP_PLMN_MODE_BIT:
        return "RIL_UNSOL_EF_CSP_PLMN_MODE_BIT"; 
    case RIL_REQUEST_GET_SMS_PARAMETERS:
        return "RIL_REQUEST_GET_SMS_PARAMETERS";
    case RIL_REQUEST_SET_SMS_PARAMETERS:
        return "RIL_REQUEST_SET_SMS_PARAMETERS";
    case RIL_REQUEST_WRITE_SMS_TO_SIM:
        return "RIL_REQUEST_WRITE_SMS_TO_SIM";
    // NFC SEEK start
    case RIL_REQUEST_SIM_TRANSMIT_BASIC: 
        return "SIM_TRANSMIT_BASIC";
    case RIL_REQUEST_SIM_OPEN_CHANNEL: 
        return "SIM_OPEN_CHANNEL";
    case RIL_REQUEST_SIM_CLOSE_CHANNEL: 
        return "SIM_CLOSE_CHANNEL";
    case RIL_REQUEST_SIM_TRANSMIT_CHANNEL: 
        return "SIM_TRANSMIT_CHANNEL";
    case RIL_REQUEST_SIM_GET_ATR:
        return "SIM_GET_ATR";
    case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW: 
        return "SIM_OPEN_CHANNEL_WITH_SW";
    // NFC SEEK end
    case RIL_UNSOL_SIM_PLUG_OUT:
        return "RIL_UNSOL_SIM_PLUG_OUT";
    case RIL_UNSOL_SIM_PLUG_IN:
        return "RIL_UNSOL_SIM_PLUG_IN";
    case RIL_REQUEST_SET_ETWS:
        return "SET_ETWS";
    case RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO:
        return "RIL_REQUEST_SET_CB_CHANNEL_CONFIG_INFO";
    case RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO:
        return "RIL_REQUEST_SET_CB_LANGUAGE_CONFIG_INFO";
    case RIL_REQUEST_GET_CB_CONFIG_INFO:
        return "RIL_REQUEST_GET_CB_CONFIG_INFO";
    case RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON:
        return "RIL_REQUEST_SET_ALL_CB_LANGUAGE_ON";
    //[New R8 modem FD]
    case RIL_REQUEST_SET_FD_MODE:
        return "SET_FD_MODE";
    case RIL_REQUEST_STK_EVDL_CALL_BY_AP:
        return "RIL_REQUEST_STK_EVDL_CALL_BY_AP";
    case RIL_REQUEST_SET_REG_SUSPEND_ENABLED:
        return "RIL_REQUEST_SET_REG_SUSPEND_ENABLED";
    case RIL_REQUEST_RESUME_REGISTRATION:
        return "RIL_REQUEST_RESUME_REGISTRATION";
    case RIL_UNSOL_RESPONSE_PLMN_CHANGED:
        return "RIL_UNSOL_RESPONSE_PLMN_CHANGED";
    case RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED:
        return "RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED";
    case RIL_REQUEST_STORE_MODEM_TYPE:
        return "RIL_REQUEST_STORE_MODEM_TYPE";
    case RIL_REQUEST_QUERY_MODEM_TYPE:
        return "RIL_REQUEST_QUERY_MODEM_TYPE";
    //Femtocell (CSG) START
    case RIL_REQUEST_GET_FEMTOCELL_LIST:  
        return "RIL_REQUEST_GET_FEMTOCELL_LIST";       
    case RIL_REQUEST_ABORT_FEMTOCELL_LIST:  
        return "RIL_REQUEST_ABORT_FEMTOCELL_LIST";       
    case RIL_REQUEST_SELECT_FEMTOCELL:  
        return "RIL_REQUEST_SELECT_FEMTOCELL";	
	case RIL_UNSOL_FEMTOCELL_INFO:
		return "RIL_UNSOL_FEMTOCELL_INFO";
    //Femtocell (CSG) END
    case RIL_REQUEST_SIM_INTERFACE_SWITCH:
        return "RIL_REQUEST_SIM_INTERFACE_SWITCH";
    // WiFi Calling start
#ifdef MTK_WIFI_CALLING_RIL_SUPPORT
    case RIL_REQUEST_UICC_SELECT_APPLICATION:
        return "RIL_REQUEST_UICC_SELECT_APPLICATION";
    case RIL_REQUEST_UICC_DEACTIVATE_APPLICATION:
        return "RIL_REQUEST_UICC_DEACTIVATE_APPLICATION";
    case RIL_REQUEST_UICC_APPLICATION_IO:
        return "RIL_REQUEST_UICC_APPLICATION_IO";
    case RIL_REQUEST_UICC_AKA_AUTHENTICATE:
        return "RIL_REQUEST_UICC_AKA_AUTHENTICATE";
    case RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP:
        return "RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP";
    case RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF:
        return "RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF";
#endif
    // WiFi Calling end
    case RIL_UNSOL_STK_EVDL_CALL: 
        return "RIL_UNSOL_STK_EVDL_CALL";
    case RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL: 
        return "RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL";
    case RIL_UNSOL_CIPHER_INDICATION:
        return "RIL_UNSOL_CIPHER_INDICATION";
    case RIL_LOCAL_REQUEST_RILD_READ_IMSI:
        return "RIL_LOCAL_REQUEST_RILD_READ_IMSI";
    case RIL_UNSOL_CNAP:
        return "RIL_UNSOL_CNAP";
    case RIL_REQUEST_SEND_OPLMN:
        return "RIL_REQUEST_SEND_OPLMN";
    case RIL_REQUEST_GET_OPLMN_VERSION:
        return "RIL_REQUEST_GET_OPLMN_VERSION";
    case RIL_REQUEST_SET_TELEPHONY_MODE:
        return "RIL_REQUEST_SET_TELEPHONY_MODE";
    default:
        return "<unknown request>";
    }
}

const char *
rilIdToString(RILId id)
{
    switch(id) {
    case MTK_RIL_SOCKET_1:
        return "RIL(1)";
#ifdef MTK_GEMINI
    case MTK_RIL_SOCKET_2:
        return "RIL(2)";
#if (MTK_GEMINI_SIM_NUM >= 4) 
    case MTK_RIL_SOCKET_4:
        return "RIL(4)";
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) 
    case MTK_RIL_SOCKET_3:
        return "RIL(3)";
#endif		
#endif
    default:
        return "not a valid RIL";
    }
}

const char *
failCauseToString(RIL_Errno e)
{
    switch(e) {
    case RIL_E_SUCCESS:
        return "E_SUCCESS";
    case RIL_E_RADIO_NOT_AVAILABLE:
        return "E_RAIDO_NOT_AVAILABLE";
    case RIL_E_GENERIC_FAILURE:
        return "E_GENERIC_FAILURE";
    case RIL_E_PASSWORD_INCORRECT:
        return "E_PASSWORD_INCORRECT";
    case RIL_E_SIM_PIN2:
        return "E_SIM_PIN2";
    case RIL_E_SIM_PUK2:
        return "E_SIM_PUK2";
    case RIL_E_REQUEST_NOT_SUPPORTED:
        return "E_REQUEST_NOT_SUPPORTED";
    case RIL_E_CANCELLED:
        return "E_CANCELLED";
    case RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL:
        return "E_OP_NOT_ALLOWED_DURING_VOICE_CALL";
    case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW:
        return "E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
    case RIL_E_SMS_SEND_FAIL_RETRY:
        return "E_SMS_SEND_FAIL_RETRY";
    case RIL_E_SIM_ABSENT:
        return "E_SIM_ABSENT";
    case RIL_E_ILLEGAL_SIM_OR_ME:
        return "E_ILLEGAL_SIM_OR_ME";
    case RIL_E_BT_SAP_UNDEFINED:
        return "RIL_E_BT_SAP_UNDEFINED";
    case RIL_E_BT_SAP_NOT_ACCESSIBLE:
        return "RIL_E_BT_SAP_NOT_ACCESSIBLE";
    case RIL_E_BT_SAP_CARD_REMOVED:
        return  "RIL_E_BT_SAP_CARD_REMOVED";
#ifdef FEATURE_MULTIMODE_ANDROID
    case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:
        return "E_SUBSCRIPTION_NOT_AVAILABLE";
    case RIL_E_MODE_NOT_SUPPORTED:
        return "E_MODE_NOT_SUPPORTED";
#endif
    default:
        return "<unknown error>";
    }
}

const char *
radioStateToString(RIL_RadioState s) {
    switch(s) {
    case RADIO_STATE_OFF:
        return "RADIO_OFF";
    case RADIO_STATE_UNAVAILABLE:
        return "RADIO_UNAVAILABLE";
    case RADIO_STATE_SIM_NOT_READY:
        return "RADIO_SIM_NOT_READY";
    case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
        return "RADIO_SIM_LOCKED_OR_ABSENT";
    case RADIO_STATE_SIM_READY:
        return "RADIO_SIM_READY";
    case RADIO_STATE_RUIM_NOT_READY:
        return"RADIO_RUIM_NOT_READY";
    case RADIO_STATE_RUIM_READY:
        return"RADIO_RUIM_READY";
    case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
        return"RADIO_RUIM_LOCKED_OR_ABSENT";
    case RADIO_STATE_NV_NOT_READY:
        return"RADIO_NV_NOT_READY";
    case RADIO_STATE_NV_READY:
        return"RADIO_NV_READY";
    default:
        return "<unknown state>";
    }
}

const char *
callStateToString(RIL_CallState s) {
    switch(s) {
    case RIL_CALL_ACTIVE :
        return "ACTIVE";
    case RIL_CALL_HOLDING:
        return "HOLDING";
    case RIL_CALL_DIALING:
        return "DIALING";
    case RIL_CALL_ALERTING:
        return "ALERTING";
    case RIL_CALL_INCOMING:
        return "INCOMING";
    case RIL_CALL_WAITING:
        return "WAITING";
    default:
        return "<unknown state>";
    }
}

/*********************************
    [Telephony Mode Definition]
        0: default, not MT6589
        1: W+G Gemini
        2: T+G Gemini
        3: FDD Single SIM
        4: TDD Single SIM
        5: W+T DualTalk
        6: T+G DualTalk
        7: W+G DualTalk
        8: G+G DualTalk
*********************************/
int getTelephonyMode() {
     int telephonyMode = MTK_TELEPHONY_MODE;
    if (telephonyMode < 0) {
        telephonyMode = 0;
    }
    return telephonyMode;
}

int isDualTalkMode() {
    /// M: when EVDODT support, check if external phone is CDMA.
    if (isEVDODTSupport()) {
        return (getExternalModemSlotTelephonyMode() == 0);
    }

    int telephonyMode = getTelephonyMode();
    if (telephonyMode == 0) {
        char property_value1[PROPERTY_VALUE_MAX] = { 0 };
        char property_value2[PROPERTY_VALUE_MAX] = { 0 };
        property_get("rild3.libpath", property_value1, "");
        property_get("rild3.libargs", property_value2, "");
        return (strlen(property_value1) > 0) && (strlen(property_value2) > 0);
    } else if (telephonyMode >= 5) {
        return 1;
    } else {
        return 0;
    }
}

int isGeminiMode()
{
    int telephonyMode = getTelephonyMode();
    char property_value[5] = { 0 };
    int current_share_modem = 0;
    
    if (telephonyMode == 0)
    {
        current_share_modem = MTK_SHARE_MODEM_CURRENT;
        switch (current_share_modem) {
            case 1:
                return 0;
            case 2:
                return 1;
        }
    } else if (telephonyMode < 3) {
        LOGD("isGeminiMode 1");
        return 1;
    } else if (telephonyMode >= 100) {
        LOGD("isGeminiMode 1 for EVDO case");
        return 1;
    }
    return 0;
}

int getExternalModemSlot() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.external.md", property_value, "0");
    return atoi(property_value)-1;
}

int isInternationalRoamingEnabled() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.evdo.irsupport", property_value, "0");
    return atoi(property_value);
}

int isEvdoOnDualtalkMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("mediatek.evdo.mode.dualtalk", property_value, "1");
    int mode = atoi(property_value);
    LOGI("evdoOnDualtalkMode mode: %d", mode);
    return mode;
}

int isSingleMode()
{
    int telephonyMode = getTelephonyMode();
    char property_value[5] = { 0 };
    int current_share_modem = 0;
    
    if (telephonyMode == 0) {
        current_share_modem = MTK_SHARE_MODEM_CURRENT;
        switch (current_share_modem) {
            case 1:
                return 1;
            case 2:
                return 0;
        }
    } else if (telephonyMode == 3 || telephonyMode == 4) {
        return 1;
    } 
    return 0;
}

int getExternalModemSlotTelephonyMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    if (getExternalModemSlot() == 0) {
        property_get("mtk_telephony_mode_slot1", property_value, "1");
    } else {
        property_get("mtk_telephony_mode_slot2", property_value, "1");
    }
    return atoi(property_value);
}

int isEVDODTSupport()
{
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ril.evdo.dtsupport", property_value, "0");
    return atoi(property_value);
}
