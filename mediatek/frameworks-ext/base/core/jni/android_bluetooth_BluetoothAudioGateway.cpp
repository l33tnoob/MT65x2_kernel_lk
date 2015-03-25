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

/*
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
/*****************************************************************************
*==========================================================
*			HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Revision: $
* $Modtime: $
* $Log: $
*
* 11 26 2012 huirong.liao
* [ALPS00373319][Huawei_BT_Xi'an_IOT][Jabra EASYGO]it will can't swith call audio path while insert charing.
* Rollback //ALPS_SW/TRUNK/ALPS.JB/alps/mediatek/frameworks-ext/base/core/jni/android_bluetooth_BluetoothAudioGateway.cpp to revision 2
 *
 * 10 04 2011 sh.lai
 * [ALPS00077467] [Need Patch] [Volunteer Patch] Support HFP AT command to access SIM PB.
 * Support AT command to access SIM PB.
 * Also fix bug that the AT command string exceed buffer size in message struct will cause whole AT command string lost.
 *
 * 03 16 2011 sh.lai
 * [ALPS00143616] [Android Build Warning Issue] frameworks/base/core/jni/android_bluetooth_BluetoothAudioGateway.cpp
 * .
 *
 * 09 22 2010 sh.lai
 * [ALPS00003522] [BLUETOOTH] Android 2.2 BLUETOOTH porting
 * Integrate bluetooth code from //ALPS_SW_PERSONAL/sh.lai/10YW1040OF_CB/ into //ALPS_SW/TRUNK/ALPS/.
 *
 * 09 10 2010 sh.lai
 * NULL
 * 1. Fix CR ALPS00125222 : [MTK BT]when dial out a invalidable number via Bluetooth headset,phone audio connot be connect automatically
 * Cause : Original HFG code that will create SCO when call state becomes alerting. In HFG SPEC, it create SCO when call state becomes DIALING.
 * Solution : Create SCO when call state becomes DIALING.
 *
 * 09 10 2010 sh.lai
 * NULL
 * 1. Fix CR ALPS00125139 : [Gemini][Call]During a Call ,unpair&disconnect the Bluetooth headset,there is a JE
 * 2. Format HFG debug log with prefix "[BT][HFG]".
 *
 * 08 20 2010 sh.lai
 * [ALPS00003522] [BLUETOOTH] Android 2.2 BLUETOOTH porting
 * Integrate BT solution into Android 2.2
 *
 * 08 17 2010 sh.lai
 * NULL
 * Integration change.
 *
 * 05 26 2010 yufeng.chu
 * [ALPS00007206][HFP, OBEX, OPP] Add $Log in source file 
 * .
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==========================================================
****************************************************************************/

#define LOG_TAG "BluetoothAudioGateway.cpp"

#include "android_bluetooth_common.h"
#include "android_runtime/AndroidRuntime.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include <sys/socket.h>
#include "cutils/sockets.h"

#define USE_ACCEPT_DIRECTLY (0)
#define USE_SELECT (0) /* 1 for select(), 0 for poll(); used only when
                          USE_ACCEPT_DIRECTLY == 0 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <ctype.h>
#include <sys/un.h>
#if defined(EXT_DYNAMIC_LOADING)
#include <dlfcn.h>
#endif

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
extern "C"
{
#include "bt_hfg_struct.h"
#include "bt_hfg_api.h"
}
#endif

#if USE_SELECT
#include <sys/select.h>
#else
#include <sys/poll.h>
#endif

namespace android {

#define GET_FIELD_ID(var, clazz, fieldName, fieldDescriptor) \
        var = env->GetFieldID(clazz, fieldName, fieldDescriptor); \
        LOG_FATAL_IF(! var, "Unable to find field " fieldName);


#ifdef HAVE_BLUETOOTH
static jfieldID field_mNativeData;
    /* in */
static jfieldID field_mHandsfreeAgRfcommChannel;
static jfieldID field_mHeadsetAgRfcommChannel;
    /* out */
static jfieldID field_mTimeoutRemainingMs; /* out */

static jfieldID field_mConnectingHeadsetAddress;
static jfieldID field_mConnectingHeadsetRfcommChannel; /* -1 when not connected */
static jfieldID field_mConnectingHeadsetSocketFd;

static jfieldID field_mConnectingHandsfreeAddress;
static jfieldID field_mConnectingHandsfreeRfcommChannel; /* -1 when not connected */
static jfieldID field_mConnectingHandsfreeSocketFd;


typedef struct {
    int hcidev;
    int hf_ag_rfcomm_channel;
    int hs_ag_rfcomm_channel;
    int hf_ag_rfcomm_sock;
    int hs_ag_rfcomm_sock;
} native_data_t;

static inline native_data_t * get_native_data(JNIEnv *env, jobject object) {
    return (native_data_t *)(env->GetIntField(object,
                                                 field_mNativeData));
}

static int setup_listening_socket(int dev, int channel);
#endif

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
#define HSP_CONTEXT                     0
#define HFP_CONTEXT                     1
#define NUM_OF_CONTEXT             2

static const char CRLF[] = "\xd\xa";
static const int CRLF_LEN = 2;

#define HFG_SERV_SOCK_ADDR "bthfgserv"
#define HFG_API_SOCK_ADDR "bthfg"
//#define HFG_SERV_SOCK_ADDR "/dev/socket/bthfgserv"
//#define HFG_API_SOCK_ADDR "/dev/socket/bthfg"


static jfieldID field_mNativeData;
    /* in */
static jfieldID field_mHandsfreeAgRfcommChannel;
static jfieldID field_mHeadsetAgRfcommChannel;
    /* out */
static jfieldID field_mTimeoutRemainingMs; /* out */

static jfieldID field_mConnectingHeadsetAddress;
static jfieldID field_mConnectingHeadsetRfcommChannel; /* -1 when not connected */
static jfieldID field_mConnectingHeadsetSocketFd;

static jfieldID field_mConnectingHandsfreeAddress;
static jfieldID field_mConnectingHandsfreeRfcommChannel; /* -1 when not connected */
static jfieldID field_mConnectingHandsfreeSocketFd;

static jmethodID method_onConnectRequest;
static jmethodID method_onConnected;
static jmethodID method_onDisconnected;
static jmethodID method_onSCOConnected;
static jmethodID method_onSCODisconnected;
static jmethodID method_onPacketReceived;
static jmethodID method_onEvent;

typedef struct {
    /* temporarily keep the data member */
    int hcidev;
    int hf_ag_rfcomm_channel;
    int hs_ag_rfcomm_channel;
    int hf_ag_rfcomm_sock;
    int hs_ag_rfcomm_sock;
    /* Add hfg context, index 0 for HSP and index 1 for HFP */
    HfgChannelContext *pContext[NUM_OF_CONTEXT];
    HfgChannelContext *activeContext;   // represent the current active context
    int hfgservsock;    // for receiving indication
    int hfgsock;           // for send request and receive response
#if defined(EXT_DYNAMIC_LOADING)
    void                        *lib_handle;
    hfg_api_table          hfg_api;
#endif
} native_data_t;

static inline native_data_t * get_native_data(JNIEnv *env, jobject object) {
    return (native_data_t *)(env->GetIntField(object, field_mNativeData));
}
#endif

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
void CONVERT_BDADDR2STRING(const MTK_BD_ADDR *source, char *dest)
{
    int i;
    char tmp;
    char *ptr = (char*)source;
    sprintf(dest, "%02X:%02X:%02X:%02X:%02X:%02X",
        source->addr[5],        
        source->addr[4],        
        source->addr[3],        
        source->addr[2],        
        source->addr[1],        
        source->addr[0]);        
    ALOGI("[BT][HFG][API] CONVERT_BDADDR2STRING : 0x%02X%02X%02X%02X%02X%02X==>%s",    
        source->addr[0],    
        source->addr[1],    
        source->addr[2],    
        source->addr[3],    
        source->addr[4],    
        source->addr[5],
        dest);
}
void CONVERT_STRING2BDADDR(const char *source, MTK_BD_ADDR *dest)
{    
    //unsigned char addr[6];    
    int i;    
    char tmp;    
    char *ptr = (char*)source;    
    for(i = 6;i > 0;)    
    {        
        dest->addr[--i] = strtoul(ptr, &ptr, 16);        
        ptr++;    
    }    
    ALOGI("[BT][HFG][API] CONVERT_STRING2BDADDR : %s==>0x%02X%02X%02X%02X%02X%02X",    
        source,     
        dest->addr[0],    
        dest->addr[1],    
        dest->addr[2],    
        dest->addr[3],    
        dest->addr[4],    
        dest->addr[5]);
}
#endif

static void classInitNative(JNIEnv* env, jclass clazz) {
    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH

    /* in */
    field_mNativeData = get_field(env, clazz, "mNativeData", "I");
    field_mHandsfreeAgRfcommChannel = 
        get_field(env, clazz, "mHandsfreeAgRfcommChannel", "I");
    field_mHeadsetAgRfcommChannel = 
        get_field(env, clazz, "mHeadsetAgRfcommChannel", "I");

    /* out */
    field_mConnectingHeadsetAddress = 
        get_field(env, clazz, 
                  "mConnectingHeadsetAddress", "Ljava/lang/String;");
    field_mConnectingHeadsetRfcommChannel = 
        get_field(env, clazz, "mConnectingHeadsetRfcommChannel", "I");
    field_mConnectingHeadsetSocketFd = 
        get_field(env, clazz, "mConnectingHeadsetSocketFd", "I");

    field_mConnectingHandsfreeAddress = 
        get_field(env, clazz, 
                  "mConnectingHandsfreeAddress", "Ljava/lang/String;");
    field_mConnectingHandsfreeRfcommChannel = 
        get_field(env, clazz, "mConnectingHandsfreeRfcommChannel", "I");
    field_mConnectingHandsfreeSocketFd = 
        get_field(env, clazz, "mConnectingHandsfreeSocketFd", "I");

    field_mTimeoutRemainingMs = 
        get_field(env, clazz, "mTimeoutRemainingMs", "I");
#endif

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    /* in */
    ALOGI("[BT][HFG][API] classInitNative");
    GET_FIELD_ID(field_mNativeData, clazz, "mNativeData", "I");
    GET_FIELD_ID(field_mHandsfreeAgRfcommChannel, clazz, "mHandsfreeAgRfcommChannel", "I");
    GET_FIELD_ID(field_mHeadsetAgRfcommChannel, clazz, "mHeadsetAgRfcommChannel", "I");

    /* out */
    GET_FIELD_ID(field_mConnectingHeadsetAddress, clazz, 
                  "mConnectingHeadsetAddress", "Ljava/lang/String;");
    GET_FIELD_ID(field_mConnectingHeadsetRfcommChannel, clazz, "mConnectingHeadsetRfcommChannel", "I");
    GET_FIELD_ID(field_mConnectingHeadsetSocketFd, clazz, "mConnectingHeadsetSocketFd", "I");

    GET_FIELD_ID(field_mConnectingHandsfreeAddress, clazz, 
                  "mConnectingHandsfreeAddress", "Ljava/lang/String;");
    GET_FIELD_ID(field_mConnectingHandsfreeRfcommChannel, clazz, "mConnectingHandsfreeRfcommChannel", "I");
    GET_FIELD_ID(field_mConnectingHandsfreeSocketFd, clazz, "mConnectingHandsfreeSocketFd", "I");

    GET_FIELD_ID(field_mTimeoutRemainingMs, clazz, "mTimeoutRemainingMs", "I");

    method_onConnectRequest = env->GetMethodID(clazz, "onConnectRequest",
                                                "(Ljava/lang/String;I)V");
    method_onConnected = env->GetMethodID(clazz, "onConnected",
                                                "(Ljava/lang/String;I)V");
    method_onDisconnected = env->GetMethodID(clazz, "onDisconnected",
                                                "(Ljava/lang/String;I)V");
    method_onSCOConnected = env->GetMethodID(clazz, "onSCOConnected",
                                                "()V");
    method_onSCODisconnected = env->GetMethodID(clazz, "onSCODisconnected",
                                                "()V");
    method_onPacketReceived = env->GetMethodID(clazz, "onPacketReceived",
                                                "(Ljava/lang/String;)V");
    method_onEvent = env->GetMethodID(clazz, "onEvent",
                                                "(I)V");    
#endif
}

static void initializeNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    if (NULL == nat) {
        ALOGE("[BT][HFG]%s: out of memory!", __FUNCTION__);
        return;
    }

    nat->hcidev = BLUETOOTH_ADAPTER_HCI_NUM;

    env->SetIntField(object, field_mNativeData, (jint)nat);
    nat->hf_ag_rfcomm_channel =
        env->GetIntField(object, field_mHandsfreeAgRfcommChannel);
    nat->hs_ag_rfcomm_channel =
        env->GetIntField(object, field_mHeadsetAgRfcommChannel);
    ALOGV("[BT][HFG]HF RFCOMM channel = %d.", nat->hf_ag_rfcomm_channel);
    ALOGV("[BT][HFG]HS RFCOMM channel = %d.", nat->hs_ag_rfcomm_channel);

    /* Set the default values of these to -1. */
    env->SetIntField(object, field_mConnectingHeadsetRfcommChannel, -1);
    env->SetIntField(object, field_mConnectingHandsfreeRfcommChannel, -1);

    nat->hf_ag_rfcomm_sock = -1;
    nat->hs_ag_rfcomm_sock = -1;
#endif

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = (native_data_t *)calloc(1, sizeof(native_data_t));
    ALOGI("[BT][HFG][API] initializeNativeDataNative");
    if (NULL == nat) {
        ALOGE("[BT][HFG]%s: out of memory!", __FUNCTION__);
        return;
    }

    #if defined(EXT_DYNAMIC_LOADING)
    nat->lib_handle = dlopen("/system/lib/libmtkbtextadp.so", RTLD_LAZY);
    if(!nat->lib_handle)
    {
        ALOGE("[BT][HFG][ERR] Can not load /system/lib/libmtkbtextadp.so : %s", dlerror());
        return;
    }
    nat->hfg_api.hfg_register = (hfg_register_api)dlsym(nat->lib_handle, "btmtk_hfg_register");
    nat->hfg_api.hfg_deregister = (hfg_deregister_api)dlsym(nat->lib_handle, "btmtk_hfg_deregister");
    nat->hfg_api.hfg_create_service_link = (hfg_create_service_link_api)dlsym(nat->lib_handle, "btmtk_hfg_create_service_link");
    nat->hfg_api.hfg_disconnect_service_link = (hfg_disconnect_service_link_api)dlsym(nat->lib_handle, "btmtk_hfg_disconnect_service_link");
    nat->hfg_api.hfg_create_audio_link = (hfg_create_audio_link_api)dlsym(nat->lib_handle, "btmtk_hfg_create_audio_link");
    nat->hfg_api.hfg_disconnect_audio_link = (hfg_disconnect_audio_link_api)dlsym(nat->lib_handle, "btmtk_hfg_disconnect_audio_link");
    nat->hfg_api.hfg_accept_connect = (hfg_accept_connect_api)dlsym(nat->lib_handle, "btmtk_hfg_accept_connect");
    nat->hfg_api.hfg_reject_connect = (hfg_reject_connect_api)dlsym(nat->lib_handle, "btmtk_hfg_reject_connect");
    nat->hfg_api.hfg_send_data = (hfg_send_data_api)dlsym(nat->lib_handle, "btmtk_hfg_send_data");
    #endif
    
    env->SetIntField(object, field_mNativeData, (jint)nat);
    nat->pContext[HSP_CONTEXT] = (HfgChannelContext*)calloc(1, sizeof(HfgChannelContext));
    if(!nat->pContext[HSP_CONTEXT])
    {
        ALOGE("[BT][HFG]%s: out of memory!", __FUNCTION__);
        goto exit;
    }
    nat->pContext[HFP_CONTEXT] = (HfgChannelContext*)calloc(1, sizeof(HfgChannelContext));
    if(!nat->pContext[HFP_CONTEXT])
    {
        ALOGE("[BT][HFG]%s: out of memory!", __FUNCTION__);
        goto exit;
    }
    /* Clear context */
    memset((void*)nat->pContext[HSP_CONTEXT], 0, sizeof(nat->pContext[0]));
    memset((void*)nat->pContext[HFP_CONTEXT], 0, sizeof(nat->pContext[0]));
    return;
    
exit:
    // close socket
    if(nat->hfgservsock)
    {
        close(nat->hfgservsock);
    }
    if(nat->hfgsock)
    {
        close(nat->hfgsock);
    }    
    if(nat->pContext[HSP_CONTEXT])
    {
        free(nat->pContext[HSP_CONTEXT]);
	nat->pContext[HSP_CONTEXT] = NULL;
    }
    if(nat->pContext[HFP_CONTEXT])
    {
        free(nat->pContext[HFP_CONTEXT]);
	nat->pContext[HFP_CONTEXT] = NULL;
    }
#endif
}

static void cleanupNativeDataNative(JNIEnv* env, jobject object) {
    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);
    if (nat) {
        free(nat);
    }
#endif
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = get_native_data(env, object);
    ALOGI("[BT][HFG][API] cleanupNativeDataNative");
    if (nat) 
    {
        if(nat->lib_handle)
        {
            dlclose(nat->lib_handle);
        }
        /* free context */
        for (int i = 0;i < NUM_OF_CONTEXT; i++)
        {
            if(nat->pContext[i]) 
            {
                free(nat->pContext[i]);
            }
        }
        free(nat);
    }
#endif
}

#ifdef HAVE_BLUETOOTH

#if USE_ACCEPT_DIRECTLY==0
static int set_nb(int sk, bool nb) {
    int flags = fcntl(sk, F_GETFL);
    if (flags < 0) {
        ALOGE("[BT][HFG]Can't get socket flags with fcntl(): %s (%d)", 
             strerror(errno), errno);
        close(sk);
        return -1;
    }
    flags &= ~O_NONBLOCK;
    if (nb) flags |= O_NONBLOCK;
    int status = fcntl(sk, F_SETFL, flags);
    if (status < 0) {
        ALOGE("[BT][HFG]Can't set socket to nonblocking mode with fcntl(): %s (%d)",
             strerror(errno), errno);
        close(sk);
        return -1;
    }
    return 0;
}
#endif /*USE_ACCEPT_DIRECTLY==0*/

static int do_accept(JNIEnv* env, jobject object, int ag_fd,
                     jfieldID out_fd,
                     jfieldID out_address,
                     jfieldID out_channel) {

#if USE_ACCEPT_DIRECTLY==0
    if (set_nb(ag_fd, true) < 0)
        return -1;
#endif

    struct sockaddr_rc raddr;
    int alen = sizeof(raddr);
    int nsk = TEMP_FAILURE_RETRY(accept(ag_fd, (struct sockaddr *) &raddr, &alen));
    if (nsk < 0) {
        ALOGE("[BT][HFG]Error on accept from socket fd %d: %s (%d).",
             ag_fd,
             strerror(errno),
             errno);
#if USE_ACCEPT_DIRECTLY==0
        set_nb(ag_fd, false);
#endif
        return -1;
    }

    env->SetIntField(object, out_fd, nsk);
    env->SetIntField(object, out_channel, raddr.rc_channel);

    char addr[BTADDR_SIZE];
    get_bdaddr_as_string(&raddr.rc_bdaddr, addr);
    env->SetObjectField(object, out_address, env->NewStringUTF(addr));

    ALOGI("[BT][HFG]Successful accept() on AG socket %d: new socket %d, address %s, RFCOMM channel %d",
         ag_fd,
         nsk,
         addr,
         raddr.rc_channel);
#if USE_ACCEPT_DIRECTLY==0
    set_nb(ag_fd, false);
#endif
    return 0;
}

#if USE_SELECT
static inline int on_accept_set_fields(JNIEnv* env, jobject object,
                                       fd_set *rset, int ag_fd,
                                       jfieldID out_fd,
                                       jfieldID out_address,
                                       jfieldID out_channel) {

    env->SetIntField(object, out_channel, -1);

    if (ag_fd >= 0 && FD_ISSET(ag_fd, &rset)) {
        return do_accept(env, object, ag_fd,
                         out_fd, out_address, out_channel);
    }
    else {
        ALOGI("[BT][HFG]fd = %d, FD_ISSET() = %d",
             ag_fd,
             FD_ISSET(ag_fd, &rset));
        if (ag_fd >= 0 && !FD_ISSET(ag_fd, &rset)) {
            ALOGE("[BT][HFG]WTF???");
            return -1;
        }
    }

    return 0;
}
#endif
#endif /* HAVE_BLUETOOTH */

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
static int handleIndication(JNIEnv* env, jobject object, native_data_t *nat, ilm_struct *ilm)
{
    int res = -1;
    ALOGI("[BT][HFG][FLOW] handleIndication : ind=%lu", ilm->msg_id);
    switch(ilm->msg_id)
    {
    case MSG_ID_BT_HFG_RFCOMM_CONNECTED_IND:
    {
        char address[17];
        bt_hfg_rfcomm_connected_ind_struct *ind_p = (bt_hfg_rfcomm_connected_ind_struct*)ilm->ilm_data;
        // set active context
        nat->activeContext = (HfgChannelContext*)ind_p->user_context;
        CONVERT_BDADDR2STRING(&ind_p->bt_addr, address);
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_RFCOMM_CONNECTED_IND : address=%s, activeContext=%p", address, nat->activeContext);
        env->CallVoidMethod(object,
                    method_onConnected,
                    env->NewStringUTF(address),
                    /* 1 : HSP, 2 : HFP */
                    (nat->activeContext==nat->pContext[0]) ? 1 : 2);

        break;
    }
    case MSG_ID_BT_HFG_CONNECT_REQ_IND:
    {
        char address[17];
        bt_hfg_connect_req_ind_struct *ind_p = (bt_hfg_connect_req_ind_struct*)ilm->ilm_data;
        // set active context
        nat->activeContext = (HfgChannelContext*)ind_p->user_context;
        CONVERT_BDADDR2STRING(&ind_p->bt_addr, address);
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_CONNECT_REQ_IND : address=%s, activeContext=%p", address, nat->activeContext);
        env->CallVoidMethod(object,
                    method_onConnectRequest,
                    env->NewStringUTF(address),
                    (nat->activeContext==nat->pContext[0]) ? 1 : 2);
        break;
    }
    case MSG_ID_BT_HFG_DISCONNECTED_IND:
    {
        char address[17];
        bt_hfg_disconnected_ind_struct *ind_p = (bt_hfg_disconnected_ind_struct*)ilm->ilm_data;
        // clear active context
        CONVERT_BDADDR2STRING(&ind_p->bt_addr, address);
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_DISCONNECTED_IND : address=%s,", address);
        env->CallVoidMethod(object,
                    method_onDisconnected,
                    env->NewStringUTF(address),
                    (nat->activeContext==nat->pContext[0]) ? 1 : 2);
        nat->activeContext = NULL;
        break;
    }
    case MSG_ID_BT_HFG_SCO_CONNECTED_IND:
    {
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_SCO_CONNECTED_IND");
        env->CallVoidMethod(object,
                    method_onSCOConnected);
        break;
    }
    case MSG_ID_BT_HFG_SCO_DISCONNECTED_IND:
    {
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_SCO_DISCONNECTED_IND");
        env->CallVoidMethod(object,
                    method_onSCODisconnected);
        break;
    }
    case MSG_ID_BT_HFG_AT_COMMAND_DATA_IND:
    {
        bt_hfg_at_command_data_ind_struct *ind_p = (bt_hfg_at_command_data_ind_struct*)ilm->ilm_data;
        ALOGI("[BT][HFG][MSG] MSG_ID_BT_HFG_AT_COMMAND_DATA_IND : at=%s, size=%d", ind_p->atcmd, ind_p->datasize);
        env->CallVoidMethod(object,
            method_onPacketReceived,
            env->NewStringUTF(ind_p->atcmd));
        break;
    }
    default:
        ALOGI("[BT][HFG][MSG] Unknown message");
        return -1;
        break;
    }
    return 1;
}
#endif

static jboolean waitForHandsfreeIndicationNative(JNIEnv* env, jobject object)
{
    int res = 0;
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    fd_set readfs;
    //struct timeval Timeout;
    int sockfd;
    ilm_struct ilm;
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[BT][HFG][API] waitForHandsfreeIndicationNative");
    if(!nat)
    {
        ALOGE("[BT][HFG][ERR] nat is NULL");
        return JNI_FALSE;
    }

    sockfd = nat->hfgservsock;
    //Timeout.tv_usec = (%1000)*1000;
    //Timeout.tv_sec  = RESPONSE_TIMEOUT/1000;
    FD_ZERO(&readfs);
    if(sockfd)
    {
        FD_SET(sockfd, &readfs);
    }
    else
    {
        ALOGE("[BT][HFG][ERR] nat->hfgservsock == 0. exit");
    }
    ALOGI("[BT][HFG]Start select : sockfd=%d", sockfd);
    res = select(sockfd+1, &readfs, NULL, NULL, NULL);
    ALOGI("[BT][HFG]Return from select : soresckfd=%d", res);
    if(res > 0)
    {
        res = recvfrom(sockfd, (void*)&ilm, sizeof(ilm_struct), 0, NULL, NULL);
        ALOGI("[BT][HFG]Recv HFP indication : %lu", ilm.msg_id);
        if(res < 0)
        {
            ALOGE("[BT][HFG][ERR] recvfrom failed : %s, %d", strerror(errno), errno);
        }
        else if(res == sizeof(int))
        {
            int evt = *(int*)&ilm;
            ALOGI("[BT][HFG]recv event : %d", evt);
            env->CallVoidMethod(object,
                method_onEvent,
                evt);
        }
        else
        {
            handleIndication(env, object, nat, &ilm);
        }
    }
    else if(res == 0)
    {
        ALOGE("[BT][HFG][ERR] timeout waiting indication");
    }
    else
    {
        if ( errno != EINTR ) {
            ALOGE("[BT][HFG][ERR] select failed : %s, %d", strerror(errno), errno);
        } else {
            return JNI_TRUE;
        }
    }
#endif
    return (res>0) ? JNI_TRUE : JNI_FALSE;
}

#if 0
static jboolean waitForHandsfreeConnectNative(JNIEnv* env, jobject object,
                                              jint timeout_ms) {
//    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH

    env->SetIntField(object, field_mTimeoutRemainingMs, timeout_ms);

    int n = 0;
    native_data_t *nat = get_native_data(env, object);
#if USE_ACCEPT_DIRECTLY
    if (nat->hf_ag_rfcomm_channel > 0) {
        ALOGI("[BT][HFG]Setting HF AG server socket to RFCOMM port %d!", 
             nat->hf_ag_rfcomm_channel);
        struct timeval tv;
        int len = sizeof(tv);
        if (getsockopt(nat->hf_ag_rfcomm_channel, 
                       SOL_SOCKET, SO_RCVTIMEO, &tv, &len) < 0) {
            ALOGE("[BT][HFG]getsockopt(%d, SOL_SOCKET, SO_RCVTIMEO): %s (%d)",
                 nat->hf_ag_rfcomm_channel,
                 strerror(errno),
                 errno);
            return JNI_FALSE;
        }
        ALOGI("[BT][HFG]Current HF AG server socket RCVTIMEO is (%d(s), %d(us))!", 
             (int)tv.tv_sec, (int)tv.tv_usec);
        if (timeout_ms >= 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = 1000 * (timeout_ms % 1000);
            if (setsockopt(nat->hf_ag_rfcomm_channel, 
                           SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                ALOGE("[BT][HFG]setsockopt(%d, SOL_SOCKET, SO_RCVTIMEO): %s (%d)",
                     nat->hf_ag_rfcomm_channel,
                     strerror(errno),
                     errno);
                return JNI_FALSE;
            }
            ALOGI("[BT][HFG]Changed HF AG server socket RCVTIMEO to (%d(s), %d(us))!", 
                 (int)tv.tv_sec, (int)tv.tv_usec);
        }

        if (!do_accept(env, object, nat->hf_ag_rfcomm_sock, 
                       field_mConnectingHandsfreeSocketFd,
                       field_mConnectingHandsfreeAddress,
                       field_mConnectingHandsfreeRfcommChannel))
        {
            env->SetIntField(object, field_mTimeoutRemainingMs, 0);
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }
#else
#if USE_SELECT
    fd_set rset;
    FD_ZERO(&rset);
    int cnt = 0;
    if (nat->hf_ag_rfcomm_channel > 0) {
        ALOGI("[BT][HFG]Setting HF AG server socket to RFCOMM port %d!", 
             nat->hf_ag_rfcomm_channel);
        cnt++;
        FD_SET(nat->hf_ag_rfcomm_sock, &rset);
    }
    if (nat->hs_ag_rfcomm_channel > 0) {
        ALOGI("[BT][HFG]Setting HS AG server socket to RFCOMM port %d!", 
             nat->hs_ag_rfcomm_channel);
        cnt++;
        FD_SET(nat->hs_ag_rfcomm_sock, &rset);
    }
    if (cnt == 0) {
        ALOGE("[BT][HFG]Neither HF nor HS listening sockets are open!");
        return JNI_FALSE;
    }

    struct timeval to;
    if (timeout_ms >= 0) {
        to.tv_sec = timeout_ms / 1000;
        to.tv_usec = 1000 * (timeout_ms % 1000);
    }
    n = TEMP_FAILURE_RETRY(select( 
                   MAX(nat->hf_ag_rfcomm_sock, nat->hs_ag_rfcomm_sock) + 1,
                   &rset,
                   NULL,
                   NULL,
                   (timeout_ms < 0 ? NULL : &to));
    if (timeout_ms > 0) {
        jint remaining = to.tv_sec*1000 + to.tv_usec/1000;
        ALOGI("[BT][HFG]Remaining time %ldms", (long)remaining);
        env->SetIntField(object, field_mTimeoutRemainingMs,
                         remaining);
    }

    ALOGI("[BT][HFG]listening select() returned %d", n);

    if (n <= 0) {
        if (n < 0)  {
            ALOGE("[BT][HFG]listening select() on RFCOMM sockets: %s (%d)",
                 strerror(errno),
                 errno);
        }
        return JNI_FALSE;
    }

    n = on_accept_set_fields(env, object, 
                             &rset, nat->hf_ag_rfcomm_sock,
                             field_mConnectingHandsfreeSocketFd,
                             field_mConnectingHandsfreeAddress,
                             field_mConnectingHandsfreeRfcommChannel);

    n += on_accept_set_fields(env, object,
                              &rset, nat->hs_ag_rfcomm_sock,
                              field_mConnectingHeadsetSocketFd,
                              field_mConnectingHeadsetAddress,
                              field_mConnectingHeadsetRfcommChannel);

    return !n ? JNI_TRUE : JNI_FALSE;
#else
    struct pollfd fds[2];
    int cnt = 0;
    if (nat->hf_ag_rfcomm_channel > 0) {
//        ALOGI("[BT][HFG]Setting HF AG server socket %d to RFCOMM port %d!", 
//             nat->hf_ag_rfcomm_sock,
//             nat->hf_ag_rfcomm_channel);
        fds[cnt].fd = nat->hf_ag_rfcomm_sock;
        fds[cnt].events = POLLIN | POLLPRI | POLLOUT | POLLERR;
        cnt++;
    }
    if (nat->hs_ag_rfcomm_channel > 0) {
//        ALOGI("[BT][HFG]Setting HS AG server socket %d to RFCOMM port %d!", 
//             nat->hs_ag_rfcomm_sock,
//             nat->hs_ag_rfcomm_channel);
        fds[cnt].fd = nat->hs_ag_rfcomm_sock;
        fds[cnt].events = POLLIN | POLLPRI | POLLOUT | POLLERR;
        cnt++;
    }
    if (cnt == 0) {
        ALOGE("[BT][HFG]Neither HF nor HS listening sockets are open!");
        return JNI_FALSE;
    }
    n = TEMP_FAILURE_RETRY(poll(fds, cnt, timeout_ms));
    if (n <= 0) {
        if (n < 0)  {
            ALOGE("[BT][HFG]listening poll() on RFCOMM sockets: %s (%d)",
                 strerror(errno),
                 errno);
        }
        else {
            env->SetIntField(object, field_mTimeoutRemainingMs, 0);
//            ALOGI("[BT][HFG]listening poll() on RFCOMM socket timed out");
        }
        return JNI_FALSE;
    }

    //ALOGI("[BT][HFG]listening poll() on RFCOMM socket returned %d", n);
    int err = 0;
    for (cnt = 0; cnt < (int)(sizeof(fds)/sizeof(fds[0])); cnt++) {
        //ALOGI("[BT][HFG]Poll on fd %d revent = %d.", fds[cnt].fd, fds[cnt].revents);
        if (fds[cnt].fd == nat->hf_ag_rfcomm_sock) {
            if (fds[cnt].revents & (POLLIN | POLLPRI | POLLOUT)) {
                ALOGI("[BT][HFG]Accepting HF connection.\n");
                err += do_accept(env, object, fds[cnt].fd, 
                               field_mConnectingHandsfreeSocketFd,
                               field_mConnectingHandsfreeAddress,
                               field_mConnectingHandsfreeRfcommChannel);
                n--;
            }
        }
        else if (fds[cnt].fd == nat->hs_ag_rfcomm_sock) {
            if (fds[cnt].revents & (POLLIN | POLLPRI | POLLOUT)) {
                ALOGI("[BT][HFG]Accepting HS connection.\n");
                err += do_accept(env, object, fds[cnt].fd, 
                               field_mConnectingHeadsetSocketFd,
                               field_mConnectingHeadsetAddress,
                               field_mConnectingHeadsetRfcommChannel);
                n--;
            }
        }
    } /* for */

    if (n != 0) {
        ALOGI("[BT][HFG]Bogus poll(): %d fake pollfd entrie(s)!", n);
        return JNI_FALSE;
    }

    return !err ? JNI_TRUE : JNI_FALSE;
#endif /* USE_SELECT */
#endif /* USE_ACCEPT_DIRECTLY */
#else
    return JNI_FALSE;
#endif /* HAVE_BLUETOOTH */
}
#endif

static jboolean setUpListeningSocketsNative(JNIEnv* env, jobject object) {
    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);

    nat->hf_ag_rfcomm_sock =
        setup_listening_socket(nat->hcidev, nat->hf_ag_rfcomm_channel);
    if (nat->hf_ag_rfcomm_sock < 0)
        return JNI_FALSE;

    nat->hs_ag_rfcomm_sock =
        setup_listening_socket(nat->hcidev, nat->hs_ag_rfcomm_channel);
    if (nat->hs_ag_rfcomm_sock < 0) {
        close(nat->hf_ag_rfcomm_sock);
        nat->hf_ag_rfcomm_sock = -1;
        return JNI_FALSE;
    }

    return JNI_TRUE;
#endif /* HAVE_BLUETOOTH */

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat;
    sockaddr_un hfgname;
    socklen_t   hfgnamelen;
    struct sockaddr_un btname;
    socklen_t   btnamelen;    

    ALOGI("[BT][HFG][API] setUpListeningSocketsNative");
    nat = get_native_data(env, object);
    if(!nat)
    {
        ALOGE("[BT][HFG][ERR] nat is null");
        return JNI_FALSE;
    }
    // Setup bt server address
    btname.sun_family = AF_UNIX;
    strcpy (btname.sun_path, /*BT_SERV_SOCK_ADDR*/BT_SOCK_NAME_INT_ADP);
    btnamelen = (offsetof (struct sockaddr_un, sun_path) + strlen (btname.sun_path) + 1);    
    // Setup hfg service socket
#if 0    
    hfgname.sun_family = AF_LOCAL;
    //strcpy (hfgname.sun_path, HFG_SERV_SOCK_ADDR);
    hfgnamelen = sizeof(short);
    nat->hfgservsock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    ALOGI("[BT][HFG]nat->hfgservsock==%d", nat->hfgservsock);
    if (nat->hfgservsock < 0)
    {
        ALOGE("[BT][HFG][ERR] create hfg server socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    if (bind (nat->hfgservsock, (struct sockaddr *) &hfgname, hfgnamelen) < 0)
    {
        ALOGE("[BT][HFG][ERR] bind hfg service socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
    else
    {
        hfgnamelen = sizeof(hfgname.sun_path);
        hfgname.sun_path[0] = '\0';
        if (getsockname(nat->hfgservsock, (sockaddr*)&hfgname, &hfgnamelen) < 0)
        {
            ALOGI("[BT][HFG]getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[BT][HFG]Auto bind HFG server : len=%d, addr=%s", hfgnamelen, &hfgname.sun_path[1]);
        }
    }
#else
    nat->hfgservsock = socket_local_server(BT_SOCK_NAME_EXT_ADP_HFP, 
                                                                      ANDROID_SOCKET_NAMESPACE_ABSTRACT, 
                                                                      SOCK_DGRAM);
    if(nat->hfgservsock < 0)
    {
        ALOGE("[BT][HFG][ERR] create hfg server socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    else
   	{
        hfgnamelen = sizeof(hfgname.sun_path);
        hfgname.sun_path[0] = '\0';
	ALOGI("[BT][HFG]create hfg server socket success");
        if (getsockname(nat->hfgservsock, (sockaddr*)&hfgname, &hfgnamelen) < 0)
        {
            ALOGI("[BT][HFG]getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[BT][HFG]Auto bind HFG server : len=%d, addr=%s", hfgnamelen, &hfgname.sun_path[1]);
        }
   	}
#endif
    // Setup hfg api socket
    //strcpy (hfgname.sun_path, HFG_API_SOCK_ADDR);
    //hfgnamelen = (offsetof (struct sockaddr_un, sun_path) + strlen (hfgname.sun_path) + 1);
    hfgnamelen = sizeof(short);
    //unlink(hfgname.sun_path);
    nat->hfgsock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    ALOGI("[BT][HFG]nat->hfgsock==%d", nat->hfgsock);
    if (nat->hfgsock < 0)
    {
        ALOGE("[BT][HFG][ERR] create hfg api socket failed : %s, errno=%d", strerror(errno), errno);
        return JNI_FALSE;
    }
    if (bind (nat->hfgsock, (struct sockaddr *) &hfgname, hfgnamelen) < 0)
    {
        ALOGE("[BT][HFG][ERR] bind hfg api socket failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
    else
    {
        hfgnamelen = sizeof(hfgname.sun_path);
        hfgname.sun_path[0] = '\0';
        if (getsockname(nat->hfgsock, (sockaddr*)&hfgname, &hfgnamelen) < 0)
        {
            ALOGE("[BT][HFG][ERR] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
        else
        {
            ALOGI("[BT][HFG]Auto bind HFG server : len=%d, addr=%s", hfgnamelen, &hfgname.sun_path[1]);
        }
    }
    if ( connect(nat->hfgsock, (const struct sockaddr*)&btname, btnamelen) < 0)
    {
        ALOGE("[BT][HFG][ERR] connect to /data/btserv failed : %s, errno=%d", strerror(errno), errno);
        goto exit;
    }
    // Init the active context to NULL
    nat->activeContext = NULL;
    // Register context
    #if defined(EXT_DYNAMIC_LOADING)
    if( BT_STATUS_SUCCESS ==  nat->hfg_api.hfg_register(nat->pContext[HSP_CONTEXT], NULL, nat->hfgsock, /*nat->hfgservsock,*/ KAL_TRUE) )
    #else
    if( BT_STATUS_SUCCESS ==  btmtk_hfg_register(nat->pContext[HSP_CONTEXT], NULL, nat->hfgsock, /*nat->hfgservsock,*/ KAL_TRUE) )
    #endif
    {
        ALOGI("[BT][HFG]pContext[HSP_CONTEXT] = %p, hfgContext=%p", nat->pContext[HSP_CONTEXT], nat->pContext[HSP_CONTEXT]->hfgContext);
        #if defined(EXT_DYNAMIC_LOADING)
        if( BT_STATUS_SUCCESS !=  nat->hfg_api.hfg_register(nat->pContext[HFP_CONTEXT], NULL, nat->hfgsock, /*nat->hfgservsock,*/ KAL_FALSE) )
        #else        
        if( BT_STATUS_SUCCESS !=  btmtk_hfg_register(nat->pContext[HFP_CONTEXT], NULL, nat->hfgsock, /*nat->hfgservsock,*/ KAL_FALSE) )
        #endif            
        {
            ALOGE("[BT][HFG][ERR] btmtk_hfg_register for HFP returns failed");
            #if defined(EXT_DYNAMIC_LOADING)
            nat->hfg_api.hfg_deregister(nat->pContext[HSP_CONTEXT]);
            #else
            btmtk_hfg_deregister(nat->pContext[HSP_CONTEXT]);
            #endif
            goto exit;
        }
        ALOGI("[BT][HFG]pContext[HFP_CONTEXT] = %p, hfgContext=%p", nat->pContext[HFP_CONTEXT], nat->pContext[HFP_CONTEXT]->hfgContext);
    }
    else
    {
        ALOGE("[BT][HFG][ERR] btmtk_hfg_register for HSP returns failed");
        goto exit;
    }
    return JNI_TRUE;
exit:
    if (nat->hfgservsock)
    {
        close(nat->hfgservsock);
        nat->hfgservsock = 0;
    }
    if (nat->hfgsock)
    {
        close(nat->hfgsock);
        nat->hfgsock = 0;
    }
    return JNI_FALSE;
#endif
    return JNI_FALSE;
}

#ifdef HAVE_BLUETOOTH
static int setup_listening_socket(int dev, int channel) {
    struct sockaddr_rc laddr;
    int sk, lm;

    sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sk < 0) {
        ALOGE("[BT][HFG]Can't create RFCOMM socket");
        return -1;
    }

    if (debug_no_encrypt()) {
        lm = RFCOMM_LM_AUTH;
    } else {
        lm = RFCOMM_LM_AUTH | RFCOMM_LM_ENCRYPT;
    }

	if (lm && setsockopt(sk, SOL_RFCOMM, RFCOMM_LM, &lm, sizeof(lm)) < 0) {
		ALOGE("[BT][HFG]Can't set RFCOMM link mode");
		close(sk);
		return -1;
	}

    laddr.rc_family = AF_BLUETOOTH;
    bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
    laddr.rc_channel = channel;

	if (bind(sk, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		ALOGE("[BT][HFG]Can't bind RFCOMM socket");
		close(sk);
		return -1;
	}

    listen(sk, 10);
    return sk;
}
#endif /* HAVE_BLUETOOTH */

/*
    private native void tearDownListeningSocketsNative();
*/
static void tearDownListeningSocketsNative(JNIEnv *env, jobject object) {
    ALOGV(__FUNCTION__);
#ifdef HAVE_BLUETOOTH
    native_data_t *nat = get_native_data(env, object);

    if (nat->hf_ag_rfcomm_sock > 0) {
        if (close(nat->hf_ag_rfcomm_sock) < 0) {
            ALOGE("[BT][HFG]Could not close HF server socket: %s (%d)\n",
                 strerror(errno), errno);
        }
        nat->hf_ag_rfcomm_sock = -1;
    }
    if (nat->hs_ag_rfcomm_sock > 0) {
        if (close(nat->hs_ag_rfcomm_sock) < 0) {
            ALOGE("[BT][HFG]Could not close HS server socket: %s (%d)\n",
                 strerror(errno), errno);
        }
        nat->hs_ag_rfcomm_sock = -1;
    }
#endif /* HAVE_BLUETOOTH */

#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = get_native_data(env, object);
    ALOGI("[BT][HFG][API] tearDownListeningSocketsNative");
    // Deregister HFG
    for(int i = 0;i < NUM_OF_CONTEXT;i++)
    {
        if(nat->pContext[i])
        {
            BtStatus status;
            #if defined(EXT_DYNAMIC_LOADING)
            status = nat->hfg_api.hfg_deregister(nat->pContext[i]);
            #else
            status = btmtk_hfg_deregister(nat->pContext[i]);
            #endif
            ALOGI("[BT][HFG]deregister %p context : return=%d", nat->pContext[i], status);
            //nat->pContext[i]= NULL;
        }
    }
    // Close sockets
    if (nat->hfgservsock)
    {
        ALOGI("[BT][HFG]Close hfg serv socket");
        close(nat->hfgservsock);
        nat->hfgservsock = 0;
    }
    if (nat->hfgsock)
    {
        ALOGI("[BT][HFG]Close hfg api socket");
        close(nat->hfgsock);
        nat->hfgsock = 0;
    }
#endif
}

/* Replacement of HeadsetBase Interface */
static jint waitForAsyncConnectNative(JNIEnv *env, jobject obj, jstring address, jint timeout_ms, jint type)
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = get_native_data(env, obj);
    ALOGI("[BT][HFG][API] waitForAsyncConnectNative : type=%d", type);
    if(nat)
    {
        BtStatus status;
        MTK_BD_ADDR bdaddr;
        const char *c_address = env->GetStringUTFChars(address, NULL);
        if(nat->activeContext != NULL)
        {
            ALOGE("[BT][HFG][ERR] activeContext is not NULL : %p", nat->activeContext);
            return -1;
        }
        CONVERT_STRING2BDADDR(c_address, &bdaddr);
        //btmtk_util_convert_string2bdaddr(c_address, &bdaddr);
        env->ReleaseStringUTFChars(address, c_address);
        #if defined(EXT_DYNAMIC_LOADING)
        status = nat->hfg_api.hfg_create_service_link(nat->pContext[type-1], &bdaddr);
        #else
        status = btmtk_hfg_create_service_link(nat->pContext[type-1], &bdaddr);
        #endif
        if(status == BT_STATUS_PENDING)
        {
            nat->activeContext = nat->pContext[type-1];
            return 1;
        }
        else
        {
            ALOGE("[BT][HFG][ERR] btmtk_hfg_create_service_link returns %d", status);
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
#endif
    return -1;
}
static void disconnectNative(JNIEnv *env, jobject obj)
{
    ALOGI("[BT][HFG][API] disconnectNative");
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = get_native_data(env, obj);
    if(nat)
    {
        BtStatus status;
        if(nat->activeContext == NULL)
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
            return;
        }        
        #if defined(EXT_DYNAMIC_LOADING)
        status = nat->hfg_api.hfg_disconnect_service_link(nat->activeContext);
        #else
        status = btmtk_hfg_disconnect_service_link(nat->activeContext);
        #endif
        if(status != BT_STATUS_PENDING)
        {
            ALOGE("[BT][HFG][ERR] btmtk_hfg_create_service_link returns %d", status);
            // if fail that means probably no disconnected indication, 
            // so we set active context to NULL here
            nat->activeContext = NULL;
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
#endif
}

static int findNextAtCmd(const char *src, int len){
    int l = 0;
    if(len >= 2 && *src == 0xD && *(src+1) == 0xA){
        src += 2;
        len -= 2;
    }
    while( (++l) < len){
        if( *src == 0xD && *(src+1) == 0xA ){
            return l+1;
        }
        src++;
    }
    return len;
}

static jboolean sendURCNative(JNIEnv *env, jobject obj, jstring urc)
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    BtStatus status = BT_STATUS_FAILED;
    int len;
    char *buffer;
    
    ALOGI("[BT][HFG][API] sendURCNative");
    native_data_t *nat = get_native_data(env, obj);
    if(nat)
    {
        const char *c_urc;
        if(nat->activeContext != NULL)
        {
            c_urc = env->GetStringUTFChars(urc, NULL);
            ALOGI("[BT][HFG]urc = %s", c_urc);
            len = strlen(c_urc) + CRLF_LEN * 2 + 1;
            buffer = (char*)calloc(len, sizeof(char));
            if(buffer)
            {
                int at_len = 0;
                int total = 0;
                char *ptr1, *ptr2;
                 
                sprintf(buffer, "%s%s%s", CRLF, c_urc, CRLF);
                ptr1 = ptr2 = buffer;
                while(len > 0){
                    len -= at_len;
                    at_len = findNextAtCmd(ptr2, len);
                    if( (total+at_len) > (MAX_AT_STRING_LEN-1) || len <= 0){
                        #if defined(EXT_DYNAMIC_LOADING)
                        status = nat->hfg_api.hfg_send_data(nat->activeContext, ptr1, total);
                        #else
                        status = btmtk_hfg_send_data(nat->activeContext, ptr1, total);
                        #endif
                        ALOGI("1. at_len=%d, total=%d, len=%d", at_len, total, len);
                        total = at_len;
                        ptr1 = ptr2;
                        ALOGI("2. at_len=%d, total=%d, len=%d", at_len, total, len);
                    }else{
                        total += at_len;
                        ALOGI("3. at_len=%d, total=%d, len=%d", at_len, total, len);
                    }
                    ptr2 += at_len;
                }
                free(buffer);
                if(status != BT_STATUS_PENDING && status != BT_STATUS_SUCCESS)
                {
                    ALOGE("[BT][HFG][ERR] sendURCNative returns %d", status);
                }
            }
            else
            {
                ALOGE("[BT][HFG][ERR] Out of memory");
            }
            env->ReleaseStringUTFChars(urc, c_urc);
        }
        else
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
    return (status==BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
#endif
    return JNI_FALSE;
}
/* Replacement of ScoSocket Interface */
static jboolean connectNative(JNIEnv *env, jobject object) 
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    BtStatus status = BT_STATUS_FAILED;
    ALOGI("[BT][HFG][API] connectNative");
    native_data_t *nat = get_native_data(env, object);
    if(nat)
    {
        if(nat->activeContext != NULL)
        {
            #if defined(EXT_DYNAMIC_LOADING)
            status = nat->hfg_api.hfg_create_audio_link(nat->activeContext);
            #else
            status = btmtk_hfg_create_audio_link(nat->activeContext);
            #endif
            if(status != BT_STATUS_PENDING)
            {
                ALOGE("[BT][HFG][ERR] btmtk_hfg_create_audio_link returns %d", status);
            }
        }
        else
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
    return (status==BT_STATUS_PENDING || status==BT_STATUS_SUCCESS ||status==BT_STATUS_IN_PROGRESS) ? JNI_TRUE : JNI_FALSE;
#endif
    return JNI_FALSE;
}
/*
static jboolean acceptNative(JNIEnv *env, jobject object)
{
    return JNI_FALSE;
}
*/
static void closeNative(JNIEnv *env, jobject object) 
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    BtStatus status = BT_STATUS_FAILED;
    ALOGI("[BT][HFG][API] closeNative");
    native_data_t *nat = get_native_data(env, object);
    if(nat)
    {
        if(nat->activeContext != NULL)
        {
            #if defined(EXT_DYNAMIC_LOADING)
            status = nat->hfg_api.hfg_disconnect_audio_link(nat->activeContext);
            #else
            status = btmtk_hfg_disconnect_audio_link(nat->activeContext);
            #endif
            if(status != BT_STATUS_PENDING)
            {
                ALOGE("[BT][HFG][ERR] btmtk_hfg_disconnect_audio_link returns %d", status);
            }
        }
        else
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
    //return (status==BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;
#endif
}
/* return 1 for success and -1 for failed */
static jint acceptConnectionNative(JNIEnv *env, jobject object)
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    BtStatus status = BT_STATUS_FAILED;
    ALOGI("[BT][HFG][API] acceptConnectionNative");
    native_data_t *nat = get_native_data(env, object);
    if(nat)
    {
        if(nat->activeContext != NULL)
        {
            #if defined(EXT_DYNAMIC_LOADING)
            status = nat->hfg_api.hfg_accept_connect(nat->activeContext);
            #else
            status = btmtk_hfg_accept_connect(nat->activeContext);
            #endif
            if(status != BT_STATUS_PENDING)
            {
                ALOGE("[BT][HFG][ERR] btmtk_hfg_accept_connect returns %d", status);
            }
        }
        else
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR] nat is NULL");
    }
    return (status==BT_STATUS_PENDING) ? 1 : -1;
#endif
    return -1;
}
static void rejectConnectionNative(JNIEnv *env, jobject object)
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    BtStatus status = BT_STATUS_FAILED;
    ALOGI("[BT][HFG][API] rejectConnectionNative");
    native_data_t *nat = get_native_data(env, object);
    if(nat)
    {
        if(nat->activeContext != NULL)
        {
            #if defined(EXT_DYNAMIC_LOADING)
            status = nat->hfg_api.hfg_reject_connect(nat->activeContext);
            #else
            status = btmtk_hfg_reject_connect(nat->activeContext);
            #endif
            if(status != BT_STATUS_PENDING)
            {
                ALOGE("[BT][HFG][ERR] btmtk_hfg_reject_connect returns %d", status);
            }
        }
        else
        {
            ALOGE("[BT][HFG][ERR] activeContext is NULL");
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR]nat is NULL");
    }
#endif
}

static void setEventNative(JNIEnv *env, jobject object, jint evt)
{
#if defined(__BTMTK__) && defined(__BT_HFG_PROFILE__)
    native_data_t *nat = get_native_data(env, object);

    ALOGI("[BT][HFG][API] setEventNative(%d)", evt);
    if(nat)
    {
        sockaddr_un name;
        socklen_t   namelen;
        //int t;
        int ret;
        
        namelen = sizeof(sockaddr_un);
        ret = getsockname(nat->hfgservsock, (struct sockaddr*)&name, &namelen);
        ALOGI("[BT][HFG]nat->hfgservsock : name=%s, size=%d", &name.sun_path[1], namelen);
        if(ret == 0 && namelen > 0)
        {
            ret = sendto(nat->hfgsock, &evt, sizeof(int), 0, (struct sockaddr*)&name, namelen);
            if(ret < 0)
            {
                ALOGE("[BT][HFG][ERR] sendto servsock itself failed : %s, errno=%d", strerror(errno), errno);
            }
        }
        else
        {
            ALOGE("[BT][HFG][ERR] getsockname failed : %s, errno=%d", strerror(errno), errno);
        }
    }
    else
    {
        ALOGE("[BT][HFG][ERR] nat is null");
    }
#endif
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */

    {"classInitNative", "()V", (void*)classInitNative},
    {"initializeNativeDataNative", "()V", (void *)initializeNativeDataNative},
    {"cleanupNativeDataNative", "()V", (void *)cleanupNativeDataNative},

    {"setUpListeningSocketsNative", "()Z", (void *)setUpListeningSocketsNative},
    {"tearDownListeningSocketsNative", "()V", (void *)tearDownListeningSocketsNative},
    //{"waitForHandsfreeConnectNative", "(I)Z", (void *)waitForHandsfreeConnectNative},
    {"waitForHandsfreeIndicationNative", "()Z", (void *)waitForHandsfreeIndicationNative},
    
    /* Replacement of HeadsetBase Interface */
    {"waitForAsyncConnectNative", "(Ljava/lang/String;II)I", (void *)waitForAsyncConnectNative},
    {"disconnectNative", "()V", (void *)disconnectNative},
    {"sendURCNative", "(Ljava/lang/String;)Z", (void *)sendURCNative},
    /* Replacement of ScoSocket Interface */
    {"connectNative", "()Z", (void *)connectNative},
    //{"acceptNative", "()Z", (void *)acceptNative},
    {"closeNative", "()V", (void *)closeNative},
    /* Additional native interface */
    {"acceptConnectionNative", "()I", (void*)acceptConnectionNative},
    {"rejectConnectionNative", "()V", (void *)rejectConnectionNative},
    {"setEventNative", "(I)V", (void *)setEventNative},
};

int register_android_bluetooth_BluetoothAudioGateway(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env,
            "android/bluetooth/BluetoothAudioGateway", sMethods,
            NELEM(sMethods));
}

} /* namespace android */
