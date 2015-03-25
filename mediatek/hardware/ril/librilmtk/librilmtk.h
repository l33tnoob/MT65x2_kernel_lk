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

/* //device/libs/telephony/ril_event.h
**
** Copyright 2008, The Android Open Source Project
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

// Max number of fd's we watch at any one time.  Increase if necessary.

#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <cutils/record_stream.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <binder/Parcel.h>
#include <cutils/jstring.h>
#include <ril_event.h>
#include <sys/types.h>
#include <sys/un.h>


#ifdef MTK_RIL
#define RIL_SUPPORT_PROXYS  RIL_SUPPORT_CHANNELS

#define RIL_CMD_PROXY_5     RIL_CMD_4 
#define RIL_CMD_PROXY_1     RIL_CMD_3
#define RIL_CMD_PROXY_2     RIL_CMD_2
#define RIL_CMD_PROXY_3     RIL_CMD_1
#define RIL_CMD_PROXY_4     RIL_URC
#define RIL_CMD_PROXY_6     RIL_ATCI
#ifdef MTK_GEMINI
#define RIL_PROXY_OFFSET    RIL_CHANNEL_OFFSET
#define RIL_CMD2_PROXY_5    RIL_CMD2_4 
#define RIL_CMD2_PROXY_1    RIL_CMD2_3
#define RIL_CMD2_PROXY_2    RIL_CMD2_2
#define RIL_CMD2_PROXY_3    RIL_CMD2_1
#define RIL_CMD2_PROXY_4    RIL_URC2
#define RIL_CMD2_PROXY_6     RIL_ATCI2

#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
#define RIL_PROXY_SET3_OFFSET    RIL_CHANNEL_SET3_OFFSET
#define RIL_CMD3_PROXY_5    RIL_CMD3_4 
#define RIL_CMD3_PROXY_1    RIL_CMD3_3
#define RIL_CMD3_PROXY_2    RIL_CMD3_2
#define RIL_CMD3_PROXY_3    RIL_CMD3_1
#define RIL_CMD3_PROXY_4    RIL_URC3
#define RIL_CMD3_PROXY_6    RIL_ATCI3

#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
#define RIL_PROXY_SET4_OFFSET    RIL_CHANNEL_SET4_OFFSET
#define RIL_CMD4_PROXY_5    RIL_CMD4_4 
#define RIL_CMD4_PROXY_1    RIL_CMD4_3
#define RIL_CMD4_PROXY_2    RIL_CMD4_2
#define RIL_CMD4_PROXY_3    RIL_CMD4_1
#define RIL_CMD4_PROXY_4    RIL_URC4
#define RIL_CMD4_PROXY_6    RIL_ATCI4

#endif
#endif /* MTK_GEMINI */

extern "C" const char *proxyIdToString(int id);

extern "C" int getTelephonyMode();
extern "C" int isDualTalkMode();
extern "C" int isGeminiMode();
extern "C" int isSingleMode();

extern "C" int getExternalModemSlot();
extern "C" int isInternationalRoamingEnabled();
extern "C" int isEVDODTSupport();
extern "C" int getExternalModemSlotTelephonyMode();
extern "C" int isEvdoOnDualtalkMode();
#endif /* MTK_RIL */

extern "C" const char * requestToString(int request);
extern "C" const char * failCauseToString(RIL_Errno);
extern "C" const char * callStateToString(RIL_CallState);
extern "C" const char * radioStateToString(RIL_RadioState);
extern "C" const char * rilIdToString(RILId id);

namespace android {

enum WakeType {DONT_WAKE, WAKE_PARTIAL};

typedef struct {
    int requestNumber;
    void (*dispatchFunction) (Parcel &p, struct RequestInfo *pRI);
    int(*responseFunction) (Parcel &p, void *response, size_t responselen);
#ifdef MTK_RIL
    RILChannelId proxyId;
#endif /* MTK_RIL */
} CommandInfo;

typedef struct {
    int requestNumber;
    int (*responseFunction) (Parcel &p, void *response, size_t responselen);
    WakeType wakeType;
} UnsolResponseInfo;

typedef struct RequestInfo {
    int32_t token;      //this is not RIL_Token
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;         // responses to local commands do not go back to command process
#ifdef MTK_RIL
    RILChannelId cid;    // For command dispatch after onRequest()
#endif /* MTK_RIL */
} RequestInfo;

typedef struct UserCallbackInfo {
    RIL_TimedCallback p_callback;
    void *userParam;
    struct ril_event event;
    struct UserCallbackInfo *p_next;
#ifdef MTK_RIL
    RILChannelId cid;    // For command dispatch after onRequest()
#endif /* MTK_RIL */
} UserCallbackInfo;


typedef struct RequestInfoProxy {
    struct RequestInfoProxy *p_next;
    RequestInfo * pRI;
    UserCallbackInfo *pUCI;
    Parcel* p;
} RequestInfoProxy;

typedef struct SocketListenParam {
    RILId rilId;
    int* s_fdListen;
    int* s_fdCommand;
    int* s_fdUT_command;
    int* s_fdUT_tmp_command;
    char* PROCESS_NAME;
    struct ril_event* s_commands_event;
    struct ril_event* s_listen_event;
    struct ril_event* s_UTcommand_event;
    struct ril_event* s_UTlisten_event;
    void (*processCommandsCallback)(int fd, short flags, void *param);
    RecordStream *p_rs;
} SocketListenParam;

typedef struct AtResponseList {
    int id;
    char* data;
    size_t datalen;
    AtResponseList *pNext;
} AtResponseList;

#ifdef MTK_RIL
void *proxyLoop(void *param);
int enqueueProxyList(RequestInfoProxy ** pProxyList, RequestInfoProxy *pRequest);
RequestInfoProxy * dequeueProxyList(RequestInfoProxy ** pProxyList);
void RIL_startRILProxys(void);
#endif /* MTK_RIL */

}
