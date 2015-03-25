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

/* //hardware/ril/reference-ril/ril_sim.c
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
#include <cutils/properties.h>

#include <ril_callbacks.h>
#include "usim_fcp_parser.h"

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static SIM_Status getSIMStatus(RILId rid);
#ifdef MTK_GEMINI
int current_phb_storage[2] = {-1, -1};
#else
int current_phb_storage[1] = {-1};
#endif

extern int rild_sms_hexCharToDecInt(char *hex, int length);

#define SIM_CHANNEL_CTX getRILChannelCtxFromToken(t)

/* -1: invalid, 0: sim, 1: usim */
extern int isUsimDetect[];

int ril_max_phb_name_len = 40;

#ifdef MTK_GEMINI
int isModemResetStarted = 0;
extern int isSimInserted(RILId rid);
#endif
extern int s_md_off;

//MTK-START [03182012][APLS00252888] This function is called
// from many functions in different threads
static pthread_mutex_t simStatusMutex1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t simStatusMutex2 = PTHREAD_MUTEX_INITIALIZER;
//MTK-END  [03182012][APLS00252888]
RILId animation_rid = MTK_RIL_SOCKET_1;
#ifdef MTK_GEMINI
RILId animation_rid2 = MTK_RIL_SOCKET_2;
#endif
void setSimStatusChanged(RILId rid)
{
    RIL_onUnsolicitedResponse (
        RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
        NULL, 0, rid);
}

static void setPINretryCount(SimPinCount *result,  RILId rid)
{
    char oldPin1[PROPERTY_VALUE_MAX] = {0};
    char oldPin2[PROPERTY_VALUE_MAX] = {0};
    char oldPuk1[PROPERTY_VALUE_MAX] = {0};
    char oldPuk2[PROPERTY_VALUE_MAX] = {0};
    char *count = NULL;
    int simStatusChanged = 0;
    int forceToUpdate = 0;
    int simId = getMappingSIMByCurrentMode(rid);
    const char *propPin1 = PROPERTY_RIL_SIM_PIN1[simId];
    const char *propPin2 = PROPERTY_RIL_SIM_PIN2[simId];
    const char *propPuk1 = PROPERTY_RIL_SIM_PUK1[simId];
    const char *propPuk2 = PROPERTY_RIL_SIM_PUK2[simId];

    property_get(propPin1,oldPin1,NULL);

    if (!(property_get(propPin1,oldPin1,NULL)
         &&property_get(propPin2,oldPin2,NULL)
         &&property_get(propPuk1,oldPuk1,NULL)
         &&property_get(propPuk2,oldPuk2,NULL))) {
        LOGE("Load property fail!");
        forceToUpdate = 1;
    } else {
    
        LOGD("oldpin1:%s, oldpin2:%s, oldpuk1:%s, oldpuk2:%s", 
                oldPin1,oldPin2,oldPuk1,oldPuk2);
    }

    /* PIN1 */
    asprintf(&count,"%d",result->pin1); 
    if (forceToUpdate) {
        
        property_set(propPin1,count);

    } else if (strcmp((const char*)oldPin1,(const char*) count)) {
        /* PIN1 has been changed */    
        property_set(propPin1,count);
        
        if (result->pin1 == 0) {
            /* SIM goto PUK state*/
            setRadioState(RADIO_STATE_SIM_LOCKED_OR_ABSENT, rid);
            simStatusChanged++;
        } else if ((result->pin1 > 0) && (0 == strcmp("0",(const char*) oldPin1))) {
            /* SIM recovered from PUK state */
            SIM_Status sim_status = getSIMStatus(rid);
            if ( USIM_READY == sim_status || SIM_READY == sim_status ) {
                setRadioState(RADIO_STATE_SIM_READY,rid); 
            }
            simStatusChanged++;
        }
    }
    free(count);

    /* PIN2 */
    asprintf(&count,"%d",result->pin2);
    if (forceToUpdate) {
        
        property_set(propPin2,count);

    } else if (strcmp((const char*)oldPin2,(const char*) count)) {
        /* PIN2 has been changed */    
        property_set(propPin2,count);
        
        if (result->pin2 == 0) {
            /* PIN2 Blocked */
            simStatusChanged++;
        } else if ((result->pin2 > 0) && (0 == strcmp("0",(const char*) oldPin2))) {
            /* PIN2 unBlocked */
            simStatusChanged++;
        }
    }
    free(count);

    /* PUK1 */
    asprintf(&count,"%d",result->puk1);
    if (forceToUpdate) {
        
        property_set(propPuk1,count);

    } else if (strcmp((const char*)oldPuk1,(const char*) count)) {
        /* PUK1 has been changed */    
        property_set(propPuk1,count);
        
        if (result->puk1 == 0) {
            /* PUK1 Blocked */
            simStatusChanged++;
        } 
    }
    free(count);

    /* PUK2 */
    asprintf(&count,"%d",result->puk2);
    if (forceToUpdate) {
        
        property_set(propPuk2,count);

    } else if (strcmp((const char*)oldPuk2,(const char*) count)) {
        /* PUK2 has been changed */    
        property_set(propPuk2,count);
        
        if (result->puk2 == 0) {
            /* PUK2 Blocked */
            simStatusChanged++;
        } 
    }
    free(count);

    if (simStatusChanged > 0) {
        setSimStatusChanged(rid);
    }
}

extern void getPINretryCount(SimPinCount *result, RIL_Token t, RILId rid)
{

    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *line;

    result->pin1 = -1;
    result->pin2 = -1;
    result->puk1 = -1;
    result->puk2 = -1;


    if (NULL != t) {
        err = at_send_command_singleline("AT+EPINC", "+EPINC:", &p_response, SIM_CHANNEL_CTX);
    } else {
        /* called by pollSIMState */
        err = at_send_command_singleline("AT+EPINC", "+EPINC:", &p_response, getChannelCtxbyProxy(rid));
    }

    if (err < 0) {        
        LOGE("getPINretryCount Fail:%d",err);
         goto done;
    }

    if (p_response->success == 0) {
        goto done;
    }

    line = p_response->p_intermediates->line;
    
    err = at_tok_start(&line);    

    if (err < 0) {   
        LOGE("get token error");
        goto done;    
    }  

    err = at_tok_nextint(&line, &(result->pin1));

    if (err < 0) {
        LOGE("get pin1 fail");
        goto done;
    }
    
    err = at_tok_nextint(&line, &(result->pin2)); 
    
    if (err < 0) {
        LOGE("get pin2 fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->puk1));

    if (err < 0) {
        LOGE("get puk1 fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->puk2));

    if (err < 0) {
        LOGE("get puk2 fail");
        goto done;
    }

    setPINretryCount(result,rid);

    LOGD("pin1:%d, pin2:%d, puk1:%d, puk2:%d", 
            result->pin1,result->pin2,result->puk1,result->puk2);

done:
    at_response_free(p_response);
  

}


/** Returns SIM_NOT_READY or USIM_NOT_READY on error */
static SIM_Status getSIMStatus(RILId rid)
{
    //MTK-START [03182012][APLS00252888] This function is called
    // from many functions in different threads.
    if (MTK_RIL_SOCKET_1 == rid)
        pthread_mutex_lock(&simStatusMutex1);
    else
        pthread_mutex_lock(&simStatusMutex2);
    //MTK-END  [03182012][APLS00252888]

    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    int isUsim = isUsimDetect[rid];
    SimPinCount retryCounts;

    // JB MR1, it will request sim status after receiver iccStatusChangedRegistrants,
    // but MD is off in the mean time, so it will get the exception result of CPIN.
    // For this special case, handle it specially.
    // check md off and sim inserted status, then return the result directly instead of request CPIN.
    // not insert: return SIM_ABSENT, insert: return SIM_NOT_READY or USIM_NOT_READY
    if (s_md_off) {
        int slot = (1 << getMappingSIMByCurrentMode(rid));
        LOGD("getSIMStatus s_md_off: %d slot: %d", s_md_off, slot);
        if (!(slot & sim_inserted_status)) {
            ret = SIM_ABSENT;
            goto done;
        } else {
            if(isUsim == 1) {
                ret = USIM_NOT_READY;
            } else {
                ret = SIM_NOT_READY;
            }
            goto done;
        }
    }

//MTK-START [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"
    //if (getRadioState(rid) == RADIO_STATE_OFF || getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
    if (getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }
    LOGD("getSIMStatus: entering");
//MTK-END [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"

    LOGD("UICC Type: %d", isUsim);

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy(rid));

    if (err != 0) {
        if (err == AT_ERROR_INVALID_THREAD) {
            ret = SIM_BUSY;
        } else {
            if(isUsim == 1) {
                ret = USIM_NOT_READY;
            } else {
                ret = SIM_NOT_READY;
            }
        }
        goto done;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                LOGD("getSIMStatus: CME_SIM_BUSY");
                ret = SIM_BUSY;
                goto done;
                break;
            case CME_SIM_NOT_INSERTED:
            case CME_SIM_FAILURE:
                ret = SIM_ABSENT;
                goto done;
                break;
            case CME_SIM_WRONG:         
                getPINretryCount(&retryCounts, NULL, rid);
                LOGD("SIM wrong: pin1:%d, pin2:%d, puk1:%d, puk2:%d", 
                retryCounts.pin1,retryCounts.pin2,retryCounts.puk1,retryCounts.puk2);
                if (retryCounts.pin1 == 0 && retryCounts.puk1 == 0) 
                {
#ifdef MTK_BSP_PACKAGE
                    if (isUsim == 1) {
                        ret = USIM_PERM_BLOCKED; // PERM_DISABLED
                    } else { 
                        ret = SIM_PERM_BLOCKED; // PERM_DISABLED
                    }
#else
                    if (isUsim == 1) {
                        ret = USIM_PUK;
                    } else { 
                        ret = SIM_PUK;
                    }
#endif
                }else {
                    if(isUsim == 1) {
                        ret = USIM_NOT_READY;
                    } else {
                        ret = SIM_NOT_READY;
                    }
                }
                goto done;
                break;
            default:
                if(isUsim == 1) {
                    ret = USIM_NOT_READY;
                } else {
                    ret = SIM_NOT_READY;
                }
                goto done;
        }
    }
    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }

    if (0 == strcmp (cpinResult, "SIM PIN")) {
        if (isUsim == 1) {
            ret = USIM_PIN;
        } else {
            ret = SIM_PIN;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "SIM PUK")) {
        if (isUsim == 1) {
            ret = USIM_PUK;
        } else { 
            ret = SIM_PUK;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-NET PIN") ||
               0 == strcmp (cpinResult, "PH-NET PUK")) {
        if (isUsim == 1) {
            ret = USIM_NP;
        } else { 
            ret = SIM_NP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-NETSUB PIN") ||
               0 == strcmp (cpinResult, "PH-NETSUB PUK")) {
        if (isUsim == 1) {
            ret = USIM_NSP;
        } else { 
            ret = SIM_NSP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-SP PIN") ||
               0 == strcmp (cpinResult, "PH-SP PUK")) {
        if (isUsim == 1) {
            ret = USIM_SP;
        } else { 
            ret = SIM_SP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-CORP PIN") ||
               0 == strcmp (cpinResult, "PH-CORP PUK")) {
        if (isUsim == 1) {
            ret = USIM_CP;
        } else { 
            ret = SIM_CP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-FSIM PIN") ||
               0 == strcmp (cpinResult, "PH-FSIM PUK")) {
        if (isUsim == 1) {
            ret = USIM_SIMP;
        } else { 
            ret = SIM_SIMP;
        }
        goto done;
    } else if (0 != strcmp (cpinResult, "READY"))  {
        /* we're treating unsupported lock types as "sim absent" */
        ret = SIM_ABSENT;
        goto done;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;

    if (isUsim == 1) {
        ret = USIM_READY;
    } else {
        ret = SIM_READY;
    }

done:
    at_response_free(p_response);
    //MTK-START [03182012][APLS00252888] This function is called
    // from many functions in different threads.
    if (MTK_RIL_SOCKET_1 == rid)
        pthread_mutex_unlock(&simStatusMutex1);
    else
        pthread_mutex_unlock(&simStatusMutex2);
    //MTK-END  [03182012][APLS00252888]
    return ret;
}

/**
 * Get the current card status.
 *
 * This must be freed using freeCardStatus.
 * @return: On success returns RIL_E_SUCCESS
 */
static int getCardStatus(RIL_CardStatus_v6 **pp_card_status, RILId rid) {
    static RIL_AppStatus app_status_array[] = {
        // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_SIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_READY = 6
        { RIL_APPTYPE_USIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // USIM_PIN = 7        
        { RIL_APPTYPE_USIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_PUK = 8
        { RIL_APPTYPE_USIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_BUSY = 9  
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NP = 10,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // SIM_NSP = 11,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // SIM_SP = 12,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // SIM_CP = 13,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // SIM_SIMP =14  
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SIM,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PERM_BLOCKED = 15 // PERM_DISABLED
        { RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // USIM_PERM_BLOCKED = 16 // PERM_DISABLED
        { RIL_APPTYPE_USIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_ABSENT = 17
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_NOT_READY = 18
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_READY = 19
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_PIN = 20
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // RUIM_PUK = 21
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_NETWORK_PERSONALIZATION = 22
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_NP = 23,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // USIM_NSP = 24,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // USIM_SP = 25,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // USIM_CP = 26,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },          
        // USIM_SIMP =27
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SIM,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_NOT_READY =28
        { RIL_APPTYPE_USIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN }
    };
    RIL_CardState card_state;
    int num_apps;

//MTK-START [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"
LOGD("getCardStatus: entering ");
//MTK-END [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"

    SIM_Status sim_status = SIM_ABSENT;
    int count = 0;
    int isUsim = isUsimDetect[rid];
    do{
       sim_status = getSIMStatus(rid);
       if (SIM_BUSY == sim_status )
       {           
           sleepMsec(200);
           count++;     //to avoid block; if the busy time is too long; need to check modem.
           if(count == 30)
           {
                LOGE("Error in getSIM Status");
                if(isUsim == 1) {
                    sim_status = USIM_NOT_READY; //to avoid exception in RILD
                } else {
                    sim_status = SIM_NOT_READY; //to avoid exception in RILD
                }
                break;
           }
       }
       if ( USIM_READY == sim_status || SIM_READY == sim_status ) {
            LOGD("getCardStatus CPIN OK, SIM ready retry times = %d", count);
            detectTestSim(rid);
            setRadioState(RADIO_STATE_SIM_READY,rid);
       }    
    }while(SIM_BUSY == sim_status);
    if (sim_status == SIM_ABSENT) {
        card_state = RIL_CARDSTATE_ABSENT;
        num_apps = 0;
    } else {
        int simId = getMappingSIMByCurrentMode(rid);
        char eccList[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_ECC_LIST[simId],eccList,"");
        if(strlen(eccList) == 0) {
            property_set(PROPERTY_ECC_LIST[simId],"112,911");
            LOGD("Set 112/911 to property");
        }
        LOGD("ECC List = %s", eccList);

        property_set(PROPERTY_RIL_ECCLIST,"112,911");
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 1;
    }

    // Allocate and initialize base card status.
    RIL_CardStatus_v6 *p_card_status = malloc(sizeof(RIL_CardStatus_v6));
    p_card_status->card_state = card_state;
    p_card_status->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    p_card_status->gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->cdma_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->ims_subscription_app_index = RIL_CARD_MAX_APPS;
    p_card_status->num_applications = num_apps;

    // Initialize application status
    int i;
    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_card_status->applications[i] = app_status_array[SIM_ABSENT];
    }

    // Pickup the appropriate application status
    // that reflects sim_status for gsm.
    if (num_apps != 0) {
        
        // TODO, MR1 Migration
        // UIM appstatus array may out of bounds on mtk solution
        // Only support one app, gsm
        //p_card_status->num_applications = 2;
        //p_card_status->cdma_subscription_app_index = 1;
        //p_card_status->applications[1] = app_status_array[sim_status + RUIM_ABSENT];

        p_card_status->num_applications = 1;
        p_card_status->gsm_umts_subscription_app_index = 0;

        // Get the correct app status
        p_card_status->applications[0] = app_status_array[sim_status];
    }

    *pp_card_status = p_card_status;
    return RIL_E_SUCCESS;
}

/**
 * Free the card status returned by getCardStatus
 */
static void freeCardStatus(RIL_CardStatus_v6 *p_card_status) {
    free(p_card_status);
}




/**
 * SIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
extern void pollSIMState (void *param)
{
    ATResponse *p_response;
    int ret;
    RILId rid = *((RILId *) param);
    SimPinCount retryCounts;

    if (getRadioState(rid) != RADIO_STATE_SIM_NOT_READY) {
        // no longer valid to poll
        return;
    }

    switch(getSIMStatus(rid)) {
        case SIM_PIN:
        case SIM_PUK:
//        case SIM_NETWORK_PERSONALIZATION:
        case SIM_NP:
        case SIM_NSP:
        case SIM_SP:
        case SIM_CP:
        case SIM_SIMP:
        case USIM_NP:
        case USIM_NSP:
        case USIM_SP:
        case USIM_CP:
        case USIM_SIMP:
        case USIM_PIN:
        case USIM_PUK:
            getPINretryCount(&retryCounts, NULL, rid);
        case SIM_ABSENT:
#ifdef MTK_BSP_PACKAGE
        case SIM_PERM_BLOCKED: // PERM_DISABLED due to CME_SIM_WRONG
        case USIM_PERM_BLOCKED: // PERM_DISABLED due to CME_SIM_WRONG
#endif
        default:
            setRadioState(RADIO_STATE_SIM_LOCKED_OR_ABSENT, rid);
        return;

        case SIM_BUSY:
        case SIM_NOT_READY:
        case USIM_NOT_READY:
            if (rid == MTK_RIL_SOCKET_1)
            {
                RIL_requestProxyTimedCallback (pollSIMState, param, &TIMEVAL_SIMPOLL,
                RIL_CMD_3, "pollSIMState");
            }
            #ifdef MTK_GEMINI
            else
            {
                RIL_requestProxyTimedCallback (pollSIMState, param, &TIMEVAL_SIMPOLL,
                RIL_CMD2_3, "pollSIMState");
            }
            #endif
            return;

        case SIM_READY:
        case USIM_READY:    
            getPINretryCount(&retryCounts, NULL, rid);
            setRadioState(RADIO_STATE_SIM_READY, rid);
        return;
    }
}


static void requestGetSimStatus(void *data, size_t datalen, RIL_Token t)
{
    RIL_CardStatus_v6 *p_card_status;
    char *p_buffer;
    int buffer_size;
    RILId rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SimPinCount retryCounts;

    int result = getCardStatus(&p_card_status,rid);
    if (result == RIL_E_SUCCESS) {
        p_buffer = (char *)p_card_status;
        buffer_size = sizeof(*p_card_status);
        
        //ALPS01021099: We get card status from getCardStatus. In that function, we will know the 
        //SIM card is present or not no matter the modem is on or off. But we can not send AT command
        //to modem when modem is off. Thus, we need to check modem status.
        LOGD("card status = %d, modem status = %d",p_card_status->card_state , !s_md_off);
        if (RIL_CARDSTATE_PRESENT == p_card_status->card_state && !s_md_off) {
            getPINretryCount(&retryCounts, t, rid);
        }

    } else {
        p_buffer = NULL;
        buffer_size = 0;
    }
    RIL_onRequestComplete(t, result, p_buffer, buffer_size);
    freeCardStatus(p_card_status);
}

static void simSecurityOperation(void *data, size_t datalen, RIL_Token t, SIM_Operation op)
{
    ATResponse   *p_response = NULL;
    int           err = -1;
    char*         cmd = NULL;
    const char**  strings = (const char**)data;;
    RIL_Errno     ret = RIL_E_GENERIC_FAILURE;
    SimPinCount   retryCount;
    int           response;
    RILId         rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SIM_Status    sim_status;   
    int           i = 0;
    int           bIgnoreCmd = FALSE;

    switch (op) {
        case ENTER_PIN1:
        case ENTER_PUK1:    
            if ( datalen == 2*sizeof(char*) ) {
                asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);                
            } else if ( datalen == 3*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN1=\"%s\",\"%s\"", strings[0], strings[1]);                
            } else
                goto done;

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
             
            break;
        case ENTER_PIN2:   
        case ENTER_PUK2:    
            if ( datalen == 2*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN2=\"%s\"", strings[0]);                
            } else if ( datalen == 3*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN2=\"%s\",\"%s\"", strings[0], strings[1]);
            } else
                goto done;

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

            break;
        case CHANGE_PIN1:
        case CHANGE_PIN2:       
            /*  UI shall handle CHANGE PIN only operated on PIN which is enabled and nonblocking state. */
            if ( datalen == 3*sizeof(char*) ) {
                if (CHANGE_PIN1 == op) {
                    SIM_Status sim_status = getSIMStatus(rid);
                    if ((sim_status == SIM_PIN)||(sim_status == USIM_PIN)) {
                        op = ENTER_PIN1;
                        /* Solve CR - ALPS00260076 PIN can not be changed by using SS string when PIN is dismissed 
                           Modem does not support multiple AT commands, so input PIN1 first */
                        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
                        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
                        bIgnoreCmd = TRUE;
                        if (p_response->success > 0)
                        {
                           bIgnoreCmd = FALSE;
                           free(cmd);
                           asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
                           /* Wait for SIM ready before changing PIN password */
                           for (i = 0; i < 5; i++) {
                               LOGD("Wait for SIM ready");
                               sleepMsec(200);
                               sim_status = getSIMStatus(rid);
                               if ((sim_status == SIM_READY) || (sim_status == USIM_READY)) {
                                  LOGD("SIM ready");
                                  break;
                               }
                           }/* End of for */
                        }
                    } else {                          
                        asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
                    }
                } else {
                    asprintf(&cmd, "AT+CPWD=\"P2\",\"%s\",\"%s\"", strings[0], strings[1]);
                }
            } else
                goto done;
            
            if (!bIgnoreCmd) {
                // [mtk02772] p_response should be free before use it, or it will be leakage
                at_response_free(p_response);
                p_response = NULL;
               err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
            }
            break;
            
        default:
            goto done;
            break;
    }
    free(cmd);

    if (err != 0) {
        /* AT runtime error */
        at_response_free(p_response);   
        goto done;
    }

    getPINretryCount(&retryCount, t, rid);

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN_REQUIRED:
            case CME_SIM_PUK_REQUIRED:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN2_REQUIRED:
                ret = RIL_E_SIM_PIN2;
                break;
            case CME_SIM_PUK2_REQUIRED:
                ret = RIL_E_SIM_PUK2;
                break;
            default:
                ret = RIL_E_GENERIC_FAILURE;
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
    /* release resource for getPINretryCount usage */
    at_response_free(p_response);

    
    if ((ret == RIL_E_SUCCESS)&&((ENTER_PIN1 == op) || (ENTER_PUK1 == op))) {
        if (op == ENTER_PIN1) {
            LOGD("ENTER_PIN1 Success!");
        } else {
            LOGD("ENTER_PUK1 Success!");
        }

        setSimStatusChanged(rid); 
/*      while (1)
        {
            SIM_Status sim_status = getSIMStatus(rid);
            if (SIM_BUSY == sim_status ) {
                LOGE("CPIN OK, but SIM Busy retry times = %d", i);
                sleepMsec(200);
                i++;
            }else if ( USIM_READY == sim_status || SIM_READY == sim_status ) {
                LOGE("CPIN OK, SIM ready retry times = %d", i);
                  setSimStausChanged(rid);  
                
                setRadioState(RADIO_STATE_SIM_READY,rid);   
                goto done;
            }
            else
            {
                LOGE("CPIN OK, Not ready or busy, SIM ready retry times = %d", i);
                goto done;
            }
        }   */  
    }

done:    
    RIL_onRequestComplete(t, ret, &retryCount, 4* sizeof(int *));

    
}

// TODO: requestEnterNetworkDepersonalization
static void requestEnterNetworkDepersonalization(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             err = -1;
    char*           cmd = NULL;
    const char**    strings = (const char**)data;
    RILId           rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SIM_Status  sim_status = getSIMStatus(rid);
    RIL_Errno     ret = RIL_E_GENERIC_FAILURE;
        
    if (SIM_NP == sim_status ||
        SIM_NSP == sim_status ||
        SIM_SP == sim_status ||
        SIM_CP == sim_status ||
        SIM_SIMP == sim_status || 
        USIM_NP == sim_status ||
        USIM_NSP == sim_status ||
        USIM_SP == sim_status ||
        USIM_CP == sim_status ||
        USIM_SIMP == sim_status
    ) {
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]); 
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX );  
        
        free(cmd);
        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_NETWORK_PERSONALIZATION_PUK_REQUIRED:  
                case CME_INCORRECT_PASSWORD:               
                    ret = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                    break;
                case CME_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED:
                    if (SIM_NP == sim_status || USIM_NP == sim_status ) 
                    {
                        ret = RIL_E_SUCCESS;
                        break;                  
                    }
                    else if (SIM_NSP == sim_status || USIM_NSP == sim_status )
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;                  
                    }else {
                        goto error;
                    }
                case CME_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED:
                    if (SIM_NP == sim_status || SIM_NSP == sim_status ||
                        USIM_NP == sim_status || USIM_NSP == sim_status ) 
                    {
                        ret = RIL_E_SUCCESS;
                        break;                  
                    }
                    else if (SIM_SP == sim_status || USIM_SP == sim_status)
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;                  
                    }else {
                        goto error;
                    }
                case CME_CORPORATE_PERSONALIZATION_PUK_REQUIRED:                
                    if (SIM_NP == sim_status ||
                        SIM_NSP == sim_status ||
                        SIM_SP == sim_status ||
                        USIM_NP == sim_status ||
                        USIM_NSP == sim_status ||
                        USIM_SP == sim_status
                    ) 
                    {
                        ret = RIL_E_SUCCESS;
                        break;                  
                    }
                    else if (SIM_CP == sim_status || USIM_CP == sim_status)
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;                  
                    }else {
                        goto error;
                    }              
                case CME_PH_FSIM_PUK_REQUIRED:
                    if (SIM_SIMP == sim_status || USIM_SIMP == sim_status) 
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;                 
                    }
                    else 
                    {
                        ret = RIL_E_SUCCESS;
                        break;                   
                    }                  
                case CME_OPERATION_NOT_ALLOWED_ERR: 
                case CME_OPERATION_NOT_SUPPORTED:
                case CME_UNKNOWN:
                    LOGD("Unknow error");
                    goto error;
                    break;
                default:
                    LOGD("Not wrong password or not allowed.");
                    ret = RIL_E_SUCCESS;
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
            setRadioState(RADIO_STATE_SIM_READY,rid);
        }
    }
    at_response_free(p_response);
    p_response = NULL;
    
    setSimStatusChanged(rid);
//  setRadioState(RADIO_STATE_SIM_READY,rid);
    RIL_onRequestComplete(t, ret, NULL, 0);
    return;
error:  
    at_response_free(p_response);
    p_response = NULL;
    RIL_onRequestComplete(t, ret, NULL, 0);
}

static void requestGetImsi(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;

    err = at_send_command_numeric("AT+CIMI", &p_response, SIM_CHANNEL_CTX);
    
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS,
            p_response->p_intermediates->line, sizeof(char *));
    }
    at_response_free(p_response);

}

static const char* getPhbStorageString(int type)
{
    char* str = NULL;
    switch(type) {
        case RIL_PHB_ADN:
            str = "SM";
            break;
        case RIL_PHB_FDN:
            str = "FD";
            break;
        case RIL_PHB_MSISDN:
            str = "ON";
            break;
        case RIL_PHB_ECC:
            str = "EN";
            break;
    }

    return str;
}

int selectPhbStorage(int type, RIL_Token t)
{
    ATResponse *p_response = NULL;
    const char *storage;
    char *cmd;
    int err, result = 1;
    const RILId rid = getRILIdByChannelCtx(SIM_CHANNEL_CTX);
    
    if (current_phb_storage[rid] != type) {
        storage = getPhbStorageString(type);
        if (storage == NULL) {
            result = 0;
        }
        
        asprintf(&cmd, "AT+CPBS=\"%s\"", storage);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        
        if (err < 0 || p_response->success == 0) {
            result = 0;
        }
        else {
            current_phb_storage[rid] = type;
        }
        free(cmd);
        at_response_free(p_response);
    }
    LOGD("Select the Storage: %d", rid);
    return result;
}

#define CMD_READ_BINARY 176
#define CMD_READ_RECORD 178
#define CMD_GET_RESPONSE 192
#define CMD_UPDATE_BINARY 214
#define CMD_UPDATE_RECORD 220
#define CMD_STATUS 242

static RIL_Errno sendCrsm(const char *cmd, ATResponse **pp_response, RIL_SIM_IO_Response *p_sr, RIL_Token t)
{
    int err;    
    char *line;
    
    err = at_send_command_singleline(cmd, "+CRSM:", pp_response, SIM_CHANNEL_CTX);

    if (err < 0 || (*pp_response)->success == 0) {
        assert(0);
        goto error;
    }

    line = (*pp_response)->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_sr->sw1));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_sr->sw2));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_sr->simResponse));
        if (err < 0) goto error;
    }

    return RIL_E_SUCCESS;

error:
    return RIL_E_GENERIC_FAILURE;
}

static RIL_Errno usimGetResponse(RIL_SIM_IO_v6 *p_args, RIL_SIM_IO_Response *p_res,RIL_Token t)
{
    size_t len;
    RIL_SIM_IO_Response sr;
    char * line = NULL;
    char * cmd = NULL;
    ATResponse * p_response = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;

    /* For read command */
    line = (char *) malloc(2*256+1);

    memset(line,0,2*256+1);

    p_res->simResponse = line; /* assume data out */
        
    if (p_args->path == NULL) {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d",
                 p_args->command, p_args->fileid,
                 0 , 0, 0);
    } else {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,,\"%s\"",
                p_args->command, p_args->fileid,
                0 , 0, 0, p_args->path);
    }

    memset(&sr,0,sizeof(RIL_SIM_IO_Response));

    if (RIL_E_SUCCESS == sendCrsm((const char *) cmd, &p_response, &sr, t)) {
        if (sr.simResponse) {
            
            len = strlen(sr.simResponse);

            LOGW("FCP Len:%d",len);
            
            if (len > 512) {
                LOGE("Invalid len:%d",len);
                ret = RIL_E_GENERIC_FAILURE;
                p_res->simResponse = NULL;
                goto done;
            }
            
            memcpy(line,sr.simResponse,len);
            
        } else {
            /* no data e.g. 0x94 0x04 */
            p_res->simResponse = NULL;
            free(line);
            line = NULL;
            goto done;
        }
    } else {
        ret = RIL_E_GENERIC_FAILURE;                
        goto done;
    }
            
        
done:        
    p_res->sw1 = sr.sw1;
    p_res->sw2 = sr.sw2;
    if (ret != RIL_E_SUCCESS) {
        LOGE("simIo Fail!");        
        free(line);
    }
    free(cmd);
    at_response_free(p_response);
    return ret;    
}
static RIL_Errno simIo(RIL_SIM_IO_v6 *p_args, RIL_SIM_IO_Response *p_res,RIL_Token t)
{
    size_t offset = p_args->p1 * (1 << 8) + p_args->p2;
    int remain = p_args->p3;
    size_t len;
    RIL_SIM_IO_Response sr;
    char * line = NULL;
    char * cmd = NULL;
    ATResponse * p_response = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;
    LOGD("[simio]p3: %d", p_args->p3);
    
    if((p_args->path != NULL) && (0 == strcmp (p_args->path,""))) {
        p_args->path = NULL;
    }
    
    if (p_args->data == NULL) {
        /* For read command */
        line = (char *) malloc(2*remain+1);

        memset(line,0,2*remain+1);

        len = ((remain < 256)?remain:256);

        p_res->simResponse = line; /* assume data out */
        
        int round = 0;
        while (remain > 0) {  
              LOGD("[simio]Round %d: remain %d\n", round++, remain);          
            if (p_args->path == NULL) {
                asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d",
                      p_args->command, p_args->fileid,
                     (0xFF & (offset >> 8)) , (0xFF & offset),((remain < 256)?remain:0));
            } else {
                asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,,\"%s\"",
                     p_args->command, p_args->fileid,
                    (0xFF & (offset >> 8)) , (0xFF & offset),((remain < 256)?remain:0), p_args->path);
            }

            memset(&sr,0,sizeof(RIL_SIM_IO_Response));

            if (RIL_E_SUCCESS == sendCrsm((const char *) cmd, &p_response, &sr, t)) {
                if (sr.simResponse) {
                      LOGD("[simio]Copy: %d\n", len * 2);   
                    memcpy(line + 2*(p_args->p3 - remain),sr.simResponse,2*len);
                    offset += len;
                    remain -= len;
                    len = ((remain < 256)?remain:256);
                    free(cmd);
                    at_response_free(p_response);
                    p_response = NULL;
                    
                } else {
                    /* no data e.g. 0x94 0x04 */
                    LOGD("[simio]Null response\n"); 
                    p_res->simResponse = NULL;
                    free(cmd);
                    free(line);
                    goto done;
                }
            } else {
                ret = RIL_E_GENERIC_FAILURE;                
                free(cmd);
                goto done;
            }
            
        }               
        
        
    } else {
        /* For write command */
        line = (char *) malloc(512);        
        memset(line,0,512);
        len = (remain > 255)?255:remain;
        
        while (remain > 0) {
            strncpy(line,p_args->data + 2*(p_args->p3 - remain),2*len);
            
            if (p_args->path == NULL) {
                asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,\"%s\"",
                    p_args->command, p_args->fileid,
                    (0xFF & (offset >> 8)), (0xFF & offset), len, line);
            } else {
                asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,\"%s\",\"%s\"",
                    p_args->command, p_args->fileid,
                    (0xFF & (offset >> 8)), (0xFF & offset), len, line,p_args->path);
            }

            if (RIL_E_SUCCESS == sendCrsm((const char *) cmd, &p_response, &sr, t)) {
                offset += len;
                remain -= len;
                len = ((remain < 256)?remain:256);
                free(cmd);
                at_response_free(p_response);
                p_response = NULL;
            } else {
                ret = RIL_E_GENERIC_FAILURE;                
                free(cmd);
                goto done;
            }
        }
        free(line); /* free line here because no response data needed */
        p_res->simResponse = NULL;
    }

done:        
    p_res->sw1 = sr.sw1;
    p_res->sw2 = sr.sw2;
    if (ret != RIL_E_SUCCESS) {
        LOGE("simIo Fail! p3:%d, offset:%d, remain:%d",p_args->p3,offset,remain);        
        if (line) {
            free(line);
        }   
    }
    at_response_free(p_response);
    return ret;
}

void makeSimRspFromUsimFcp(unsigned char ** simResponse)
{
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_descriptor_struct fDescriptor = {0,0,0,0};
    usim_file_size_struct fSize  = {0};
    unsigned char simRspByte[GET_RESPONSE_EF_SIZE_BYTES] = {0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T, &fDescriptor)) {
        LOGE("USIM FD Parse fail:%s", *simResponse);
        goto done;
    }
    
    if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_SIZE_T,&fSize)) {
        LOGW("USIM File Size fail:%s", *simResponse);
        goto done;
    }

    if (IS_DF_ADF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_DF;
        goto done;
    } else {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_EF;
    }
    
    simRspByte[RESPONSE_DATA_FILE_SIZE_1] = (fSize.file_size & 0xFF00) >> 8;
    simRspByte[RESPONSE_DATA_FILE_SIZE_2] = fSize.file_size & 0xFF;

    if (IS_LINEAR_FIXED_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_LINEAR_FIXED;        
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    } else if (IS_TRANSPARENT_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_TRANSPARENT;
        
    } else if (IS_CYCLIC_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_CYCLIC;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    }


done:
    free(*simResponse);
    free(fcpByte);
    *simResponse = byteArrayToHexString(simRspByte, GET_RESPONSE_EF_SIZE_BYTES);
    LOGD("simRsp done:%s", *simResponse);
}

static void requestSimIo(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err;
    char *cmd = NULL;
    RIL_SIM_IO_v6 *p_args;
    char *line;
    char *cpinResult;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    SimPinCount retryCounts;
    RILId rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    
    memset(&sr, 0, sizeof(sr));

    p_args = (RIL_SIM_IO_v6 *)data;

    /* Handle if there is a PIN2 */
    if (NULL != p_args->pin2) {
        asprintf(&cmd, "AT+EPIN2=\"%s\"", (const char *) p_args->pin2);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);        

        /* AT+EPIN2 fail and check the cause */
        if (err != 0) {
            /* AT runtime error */
            assert(0);
            goto done;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SIM_PIN2_REQUIRED:                
                    ret = RIL_E_SIM_PIN2;
                    goto done;
                    break;
                case CME_SIM_PUK2_REQUIRED:
                    ret = RIL_E_SIM_PUK2;
                    goto done;
                    break;
                default:
                    ret = RIL_E_GENERIC_FAILURE;
                    goto done;
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
    }

    if ((isUsimDetect[rid] == 1) && (p_args->command == CMD_GET_RESPONSE)) {
        
        LOGD("GET RESPONSE on USIM %s:%x, p3:%d",p_args->path, p_args->fileid,p_args->p3);
        
        ret = usimGetResponse(p_args, &sr, t);

        /* Map USIM FCP to SIM RSP */
        if ((RIL_E_SUCCESS == ret) && (sr.simResponse != NULL)) {
            makeSimRspFromUsimFcp((unsigned char **) &(sr.simResponse));
        }

    } else {
       ret = simIo(p_args, &sr, t);
    }
 
done:
    if (NULL != p_args->pin2) {
        getPINretryCount(&retryCounts, t, rid);
    }
    
    if (ret == RIL_E_SUCCESS) {
        RIL_onRequestComplete(t, ret, &sr, sizeof(sr));
        if (sr.simResponse) {
            free(sr.simResponse);
        }
    } else {
        RIL_onRequestComplete(t, ret, NULL, 0);
    }
    at_response_free(p_response);
    if (cmd) {
        free(cmd);
    }
}


static void simFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             err = -1;
    char*           cmd = NULL;
    const char**    strings = (const char**)data;;
    int             response = -1;
    char*           line;
    int             classRequired = 1;
    SimPinCodeE     simOperationSetLock = PINUNKNOWN;
    SimPinCount     retryCount;
    const char * p_serviceClass; 
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    RILId           rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int           resLength = 0;
    int*          p_res = NULL;
    int sendBsCode = 0;

    // [ALPS00451149][MTK02772]
    // CLCK is query before MSG_ID_SIM_MMI_READY_IND
    // FD's flag is ready after receive this msg
    // solution: request again if modem response busy, max 2.5s
    int isSimBusy = 0;
    int count = 0;

    do{
         //ALPS00839044: Modem needs more time for some special cards.
         //The detail of the time 2.5s is in the note of this CR.
         if( count == 13 ) {
             LOGE("Set Facility Lock: CME_SIM_BUSY and time out.");
             goto error;
         }

    if ((0 == strcmp (strings[0],"SC")) ||
        (0 == strcmp (strings[0],"CS")) ||
        (0 == strcmp (strings[0],"PS")) ||
        (0 == strcmp (strings[0],"PF")) ||
        (0 == strcmp (strings[0],"FD")) ||
        (0 == strcmp (strings[0],"PN")) ||
        (0 == strcmp (strings[0],"PU")) ||
        (0 == strcmp (strings[0],"PP")) ||
        (0 == strcmp (strings[0],"PC")))
    {
        classRequired = 0;
        
    }

    if ( datalen == 4*sizeof(char*) ) {
        /* Query Facility Lock */
        if (classRequired) {

            if ((0 == strcmp("AB",strings[0])) 
                || (0 == strcmp("AG",strings[0]))
                || (0 == strcmp("AC",strings[0]))) {
                
                LOGE("Call Barring Error: %s Cannot be used for Query!",strings[0]);
                goto error;
            }

            if ((NULL != strings[2]) && (0 != strcmp (strings[2],"0"))) {
                p_serviceClass = strings[2];
                sendBsCode = atoi(p_serviceClass);
            }
            
#ifdef MTK_LTE_SUPPORT
            if (sendBsCode == CLASS_MTK_VIDEO) {
                sendBsCode = CLASS_DATA_SYNC;
            }
#endif
            LOGD("sendBsCode = %d", sendBsCode);
                
            /* PASSWD is given and CLASS is necessary. Because of NW related operation */
            /* asprintf(&cmd, "AT+CLCK=\"%s\",2,\"%s\",\"%s\"", strings[0], strings[1], strings[2]);
                asprintf(&cmd, "AT+ECUSD=1,1,\"*#%s**%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                      InfoClassToMmiBSCodeString(atoi(p_serviceClass)));
            
            } else {*/
            /* BS_ALL NULL BSCodeString */
            // When query call barring setting, don't send BS code because some network cannot support BS code.
            asprintf(&cmd, "AT+ECUSD=1,1,\"*#%s#\"", callBarFacToServiceCodeStrings(strings[0]));
            //}
            
            err = at_send_command_multiline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);

        } else {
            /* No network related query. CLASS is unnecessary */
            asprintf(&cmd, "AT+CLCK=\"%s\",2", strings[0]);
            
            err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);
        }
    } else if ( datalen == 5*sizeof(char*) ) {
    
        if ( 0 == strcmp (strings[0],"SC") ) {
            simOperationSetLock = PINCODE1;
        } else if ( 0 == strcmp (strings[0],"FD") ) {
            simOperationSetLock = PINCODE2;
        }
        
        if(NULL == strings[2]) {
        
            LOGE("Set Facility Lock: Pwd cannot be null!");
            ret = RIL_E_PASSWORD_INCORRECT;
            goto error;
        }
        
        /* Set Facility Lock */
        if ( classRequired == 1) {


            if(strlen(strings[2]) != 4) {
                
                LOGE("Set Facility Lock: Incorrect passwd length:%d",strlen(strings[2]));
                ret = RIL_E_PASSWORD_INCORRECT;
                goto error;
            
            }
            
            if ((NULL != strings[3]) && (0 != strcmp (strings[3],"0"))) {
            
                p_serviceClass = strings[3];

            /* Network operation. PASSWD is necessary */
            //asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\",\"%s\"", strings[0], strings[1], strings[2], strings[3]); 
            if ( 0 == strcmp (strings[1],"0")) {
                asprintf(&cmd, "AT+ECUSD=1,1,\"#%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                          strings[2],
                                                          InfoClassToMmiBSCodeString(atoi(p_serviceClass)));
            } else {
                asprintf(&cmd, "AT+ECUSD=1,1,\"*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                          strings[2],
                                                          InfoClassToMmiBSCodeString(atoi(p_serviceClass)));
            }
        } else {
                /* For BS_ALL BS==NULL */
                if ( 0 == strcmp (strings[1],"0")) {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"#%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                          strings[2]);
                } else {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                          strings[2]);
                }
            }
        } else {
                    
            /* Local operation. PASSWD is necessary */
            asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\"", strings[0], strings[1], strings[2]);   
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    } else
        goto error;

    free(cmd);
		cmd = NULL;

        if (err < 0 || NULL == p_response) {        
            LOGE("getFacilityLock Fail");
            goto error;
        }
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                LOGD("simFacilityLock: CME_SIM_BUSY");
                sleepMsec(200);
                count++;
                isSimBusy = 1;
				at_response_free(p_response);
        		p_response = NULL;
                break;
            default:
                LOGD("simFacilityLock: default");
                isSimBusy = 0;
                break;
        }
    } while (isSimBusy == 1);

    if (PINUNKNOWN != simOperationSetLock) {
        getPINretryCount(&retryCount, t, rid);
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_PIN_REQUIRED:
            case CME_SIM_PUK_REQUIRED:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN2_REQUIRED:
                ret = RIL_E_SIM_PIN2;
                break;
            case CME_SIM_PUK2_REQUIRED:
                ret = RIL_E_SIM_PUK2;
                break;    
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                if (PINUNKNOWN == simOperationSetLock) {
                    /* Non PIN operation! Goto Error directly */
                    goto error;  
                }
                break;
            case CME_CALL_BARRED:
            case CME_OPR_DTR_BARRING:
                ret = RIL_E_CALL_BARRED;
                goto error;
                break;
            case CME_PHB_FDN_BLOCKED:
                ret = RIL_E_FDN_CHECK_FAILURE;
                goto error;
                break;
            default:
#ifdef MTK_BSP_PACKAGE
                LOGD("simFacilityLock() retryPIN2 = %d, simOperationSetLock = %d", retryCount.pin2, simOperationSetLock);           
                if (simOperationSetLock == PINCODE2 && retryCount.pin2==0){
                    ret = RIL_E_SIM_PUK2;
                }
#endif 
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }



    /* For Query command only */
    if ( p_response->p_intermediates != NULL ) {
        ATLine * p_cur;
#ifdef MTK_LTE_SUPPORT
        int serviceClass;
#endif

#ifndef MTK_LTE_SUPPORT
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            resLength++;
        }
        LOGI("%d of +CLCK: received!",resLength);     
        
        p_res = alloca(resLength * sizeof(int));
        resLength = 0; /* reset resLength for decoding */
#endif

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;

        assert(line);
    
        err = at_tok_start(&line);    

        if (err < 0) {        
                LOGE("CLCK: decode error 1!");
            goto error;    
        }  

            err = at_tok_nextint(&line, &response); /* 0 disable 1 enable */

#ifndef MTK_LTE_SUPPORT
            p_res[resLength] = 0; /* Set Init value to 0 */
#endif

            if (at_tok_hasmore(&line)) {

#ifdef MTK_LTE_SUPPORT
                err = at_tok_nextint(&line, &serviceClass); /* enabled service code */
                if (sendBsCode == serviceClass) {
                   break;
                }
#else
                err = at_tok_nextint(&line, &p_res[resLength]); /* enabled service code */
                LOGD("Status:%d, BSCode:%d\n",response,p_res[resLength]);
#endif
            }
            
#ifdef MTK_LTE_SUPPORT
            if (classRequired) {
               response = 0;
            }
#endif            
            if (err < 0) {  
                LOGE("CLCK: decode error 2!");
                goto error;    
            }  

#ifndef MTK_LTE_SUPPORT
            resLength++;
#endif
        }

    }
    
    
    if (err < 0) {
         goto error;
    } else {
        if (PINUNKNOWN != simOperationSetLock) {
            /* SIM operation we shall return pin retry counts */
            RIL_onRequestComplete(t, ret,
                &retryCount, 4* sizeof(int *));
                
        } else {
#ifdef MTK_LTE_SUPPORT
             // The specific call barring is activated(enabled), return service class value(Refer to CallBarringBasePreference.java).
            if ((response != 0) && classRequired)  {
                response = atoi(p_serviceClass);
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int *));
#else
            LOGD("sendBsCode = %d",sendBsCode);
            LOGD("resLength = %d, classRequired = %d",resLength, classRequired);
            if ((resLength >= 1) && (classRequired)) {
                /* For CLCK NW(Call Barring) results: we shall combind the results */
                
                if (response != 0) {
                    response = 0;
                    do {
                        resLength--;
                        LOGD("response = %d, MmiBSCodeToInfoClassX(p_res[resLength]) = %d",response, MmiBSCodeToInfoClassX(p_res[resLength]));
                        if ((sendBsCode != 0 && (sendBsCode & MmiBSCodeToInfoClassX(p_res[resLength])))
                            || sendBsCode == 0) {
                            response |= MmiBSCodeToInfoClassX(p_res[resLength]);
                        }
                    } while (resLength > 0);
                }
                
                RIL_onRequestComplete(t, RIL_E_SUCCESS,
                    &response, sizeof(int *));
            } else {
                /* For those non NW (Call Barring) results*/
                RIL_onRequestComplete(t, RIL_E_SUCCESS,
                &response, sizeof(int *));
            }
#endif
            LOGD("simFacilityLock response:%d",response);
        }
        
        at_response_free(p_response);
        p_response = NULL;
        return;
    }    
    
error:        
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestPhbStatus(RILId rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret = RIL_E_GENERIC_FAILURE;

    err = at_send_command("AT+EQUERY=4", &p_response, getChannelCtxbyProxy(rid));

    if (err < 0 || NULL == p_response) {
        // assume radio is off
        goto error;
    }

    if (p_response->success > 0) {
        ret = RIL_E_SUCCESS;
    }
        
    at_response_free(p_response);

    if (RIL_E_SUCCESS == ret)
        upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], "true");
    else
        upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], "false");
    return;
error:
    LOGE("[RIL%d]AT+EQUERY return ERROR", (rid==MTK_RIL_SOCKET_1)?1:2 );
    at_response_free(p_response);
}

static void onPhbStateChanged(RILId rid, int isPhbReady)
{
    LOGD("onPhbStateChanged rid=%d, isPhbReady=%d", rid, isPhbReady);

#ifdef MTK_GEMINI
    LOGD("onPhbStateChanged isSimInserted=%d, isModemResetStarted=%d", isSimInserted(rid), isModemResetStarted);

    if (isSimInserted(rid) == 0) {
        return;
    }

    if ((isPhbReady == TRUE) && (isModemResetStarted)) {
        return;
    }
#endif

    if (isPhbReady == TRUE) {
        upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], "true");
    } else {
        upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], "false");
    }

    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, &isPhbReady, sizeof(int), rid);
}

extern void querySimStateForBootAnimation(void *param);

extern int rilSimMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_GET_SIM_STATUS: 
            requestGetSimStatus(data,datalen,t);
        break;
        case RIL_REQUEST_GET_IMSI:
            requestGetImsi(data,datalen,t);
        break;
        case RIL_REQUEST_SIM_IO:
            requestSimIo(data,datalen,t);
        break;
        case RIL_REQUEST_ENTER_SIM_PIN:            
            simSecurityOperation(data,datalen,t,ENTER_PIN1);
        break;
        case RIL_REQUEST_ENTER_SIM_PUK:
            simSecurityOperation(data,datalen,t,ENTER_PUK1);
        break;
        case RIL_REQUEST_ENTER_SIM_PIN2:
            simSecurityOperation(data,datalen,t,ENTER_PIN2);
        break;
        case RIL_REQUEST_ENTER_SIM_PUK2:
            simSecurityOperation(data,datalen,t,ENTER_PUK2);
        break;
        case RIL_REQUEST_CHANGE_SIM_PIN:
            simSecurityOperation(data,datalen,t,CHANGE_PIN1);
        break;
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            simSecurityOperation(data,datalen,t,CHANGE_PIN2);
        break;
        case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION:
            requestEnterNetworkDepersonalization(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
        case RIL_REQUEST_SET_FACILITY_LOCK:
            simFacilityLock(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO:
            requestQueryPhbInfo(data, datalen, t);
        break;
        case RIL_REQUEST_WRITE_PHB_ENTRY:
            requestWritePhbEntry(data, datalen, t);
        break;
        case RIL_REQUEST_READ_PHB_ENTRY:
            requestReadPhbEntry(data, datalen, t);
        break;
        case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK:
            queryNetworkLock(data, datalen, t);
        break;
        case RIL_REQUEST_SET_SIM_NETWORK_LOCK:
            simNetworkLock(data,datalen,t);
        break;
        case RIL_REQUEST_BTSIM_CONNECT:
            requestConnectSIM(data,datalen,t);
        break;
        case RIL_REQUEST_BTSIM_DISCONNECT_OR_POWEROFF:
            requestDisconnectOrPowerOffSIM(data,datalen,t);
        break;  
        case RIL_REQUEST_BTSIM_POWERON_OR_RESETSIM:
            requestPowerOnOrResetSIM(data,datalen,t);
        break;
        case RIL_REQUEST_BTSIM_TRANSFERAPDU:
            requestTransferApdu(data, datalen, t);  
        break;
        case RIL_REQUEST_QUERY_ICCID:
            requestIccId(data, datalen, t);
        break;  
        case RIL_REQUEST_SIM_AUTHENTICATION:
        case RIL_LOCAL_REQUEST_SIM_AUTHENTICATION:
           requestSimAuthentication(data, datalen, t);
        break;  
        case RIL_REQUEST_USIM_AUTHENTICATION:
        case RIL_LOCAL_REQUEST_USIM_AUTHENTICATION:
           requestUSimAuthentication(data, datalen, t);
        break;
        case RIL_LOCAL_REQUEST_RILD_READ_IMSI:
//            querySimStateForBootAnimation(data, datalen, t);
        {
            int rid = ((int*)data)[0];
            LOGD("check 3: %d",rid);
            querySimStateForBootAnimation((void *)&rid);
        }
        break;
        case RIL_REQUEST_GET_POL_CAPABILITY:
            requestGetPOLCapability(data, datalen, t);
        break;
        case RIL_REQUEST_GET_POL_LIST:
            requestGetPOLList(data, datalen, t);
        break;
        case RIL_REQUEST_SET_POL_ENTRY:
            requestSetPOLEntry(data, datalen, t);
        break;      
        case RIL_REQUEST_QUERY_UPB_CAPABILITY:
            requestQueryUPBCapability(data, datalen, t);
        break;
        case RIL_REQUEST_EDIT_UPB_ENTRY:
            requestEditUPBEntry(data, datalen, t);
        break;
        case RIL_REQUEST_DELETE_UPB_ENTRY:
            requestDeleteUPBEntry(data, datalen, t);
        break;
        case RIL_REQUEST_READ_UPB_GAS_LIST:
            requestReadGasList(data, datalen, t);
        break;
        case RIL_REQUEST_READ_UPB_GRP:
            requestReadUpbGrpEntry(data, datalen, t);
        break;
        case RIL_REQUEST_WRITE_UPB_GRP:
            requestWriteUpbGrpEntry(data, datalen, t);
        break;
        case RIL_REQUEST_DETECT_SIM_MISSING:
            requestDetectSimMissing(data,datalen,t);
            break;
        case RIL_REQUEST_GET_PHB_STRING_LENGTH:
            requestGetPhoneBookStringsLength(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PHB_MEM_STORAGE:
            requestGetPhoneBookMemStorage(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PHB_MEM_STORAGE:
            requestSetPhoneBookMemStorage(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PHB_ENTRY_EXT:
            requestReadPhoneBookEntryExt(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PHB_ENTRY_EXT:
            requestWritePhoneBookEntryExt(data, datalen, t);
            break;
        // NFC SEEK start
        case RIL_REQUEST_SIM_TRANSMIT_BASIC:
            requestSIM_TransmitBasic(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
            requestSIM_OpenChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
            requestSIM_CloseChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_CHANNEL:
            requestSIM_TransmitChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_GET_ATR:
            requestSIM_GetATR(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL_WITH_SW:
            requestSIM_OpenChannelWithSw(data, datalen, t);
            break;
        // NFC SEEK end
        case RIL_REQUEST_SIM_INTERFACE_SWITCH:
            requestSimInterfaceSwitch(data,datalen,t);
            break;
        case RIL_REQUEST_SET_TELEPHONY_MODE:
            requestSetTelephonyMode(data,datalen,t);
            break;
        // WiFi Calling start
    #ifdef MTK_WIFI_CALLING_RIL_SUPPORT
        case RIL_REQUEST_UICC_SELECT_APPLICATION:
            requestUiccSelectApp(data, datalen, t);
            break;
        case RIL_REQUEST_UICC_DEACTIVATE_APPLICATION:
            requestUiccDeactivateApp(data, datalen, t);
            break;
        case RIL_REQUEST_UICC_APPLICATION_IO:
            reqeustUiccIO(data, datalen, t);
            break;
        case RIL_REQUEST_UICC_AKA_AUTHENTICATE:
            requestUiccAuthentication(data, datalen, t, AUTHENTICATE_AKA);
            break;
        case RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP:
            requestUiccAuthentication(data, datalen, t, AUTHENTICATE_GBA_BOOTSTRAP);
            break;
        case RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF:
            requestUiccAuthentication(data, datalen, t, AUTHENTICATE_GBA_NAF);
            break;
    #endif /* MTK_WIFI_CALLING_RIL_SUPPORT */
        // WiFi Calling end

        default:
            return 0; /* no match */
        break;
    }
    return 1; /* request find */
}

extern int rilSimUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RILId rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+EIND: 2")) {
        onPhbStateChanged(rid, TRUE);
    } else if (strStartsWith(s, "+EIND: 32")) {
        onPhbStateChanged(rid, FALSE);
    } else if (strStartsWith(s, "+EUSIM:")) {
        LOGD("EUSIM URC:%s",s);
        onUsimDetected(s,rid);
    } else if (strStartsWith(s, "+ETESTSIM:")) {
        LOGD("ETESTSIM URC:%s",s);
        onTestSimDetected(s, rid);
    } else if (strStartsWith(s, "+PACSP")) { // ALPS00302698
        LOGD("PACSP URC:%s",s);
        onEfCspPlmnModeBitDetected(s, rid);
    } else {
        return 0;
    }

    return 1;
}

void resetPhbReady(RILId rid){
    int isPhbReady = FALSE;
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], "false");
    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, &isPhbReady, sizeof(int), rid);
}

void rilSwitchSimProperty() {
    LOGI("To switch SIM property");
    
    int simId3G = RIL_get3GSIM()-1;

    // 3G switch, 1 -> N, N->1
    if(RIL_is3GSwitched()) {
        switchProperty(PROPERTY_RIL_SIM_PIN1[0], PROPERTY_RIL_SIM_PIN1[simId3G], 128);
        switchProperty(PROPERTY_RIL_SIM_PIN2[0], PROPERTY_RIL_SIM_PIN2[simId3G], 128);
        switchProperty(PROPERTY_RIL_SIM_PUK1[0], PROPERTY_RIL_SIM_PUK1[simId3G], 128);
        switchProperty(PROPERTY_RIL_SIM_PUK2[0], PROPERTY_RIL_SIM_PUK2[simId3G], 128);
        switchProperty(PROPERTY_RIL_TEST_SIM[0], PROPERTY_RIL_TEST_SIM[simId3G], 128);
    }
}

void switchProperty(char* property1, char* property2, int maxLength) {
    char* p1 = malloc(sizeof(char) * maxLength);
    char* p2 = malloc(sizeof(char) * maxLength);
    memset(p1, 0, sizeof(char) * maxLength);
    memset(p2, 0, sizeof(char) * maxLength);
    property_get(property1, p1, NULL);
    property_get(property2, p2, NULL);
    property_set(property1, p2);
    property_set(property2, p1);
    free(p1);
    free(p2);
}

void resetSIMProperties(RILId rid) {
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_SIM_PIN1[0], PROPERTY_RIL_SIM_PIN1[1], PROPERTY_RIL_SIM_PIN1[2], PROPERTY_RIL_SIM_PIN1[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_SIM_PUK1[0], PROPERTY_RIL_SIM_PUK1[1], PROPERTY_RIL_SIM_PUK1[2], PROPERTY_RIL_SIM_PUK1[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_SIM_PIN2[0], PROPERTY_RIL_SIM_PIN2[1], PROPERTY_RIL_SIM_PIN2[2], PROPERTY_RIL_SIM_PIN2[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_SIM_PUK2[0], PROPERTY_RIL_SIM_PUK2[1], PROPERTY_RIL_SIM_PUK2[2], PROPERTY_RIL_SIM_PUK2[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_SIM_OPERATOR_DEFAULT_NAME_LIST[0], PROPERTY_RIL_SIM_OPERATOR_DEFAULT_NAME_LIST[1],
            PROPERTY_RIL_SIM_OPERATOR_DEFAULT_NAME_LIST[2], PROPERTY_RIL_SIM_OPERATOR_DEFAULT_NAME_LIST[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_UICC_TYPE[0], PROPERTY_RIL_UICC_TYPE[1], PROPERTY_RIL_UICC_TYPE[2], PROPERTY_RIL_UICC_TYPE[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_RIL_PHB_READY[0], PROPERTY_RIL_PHB_READY[1], PROPERTY_RIL_PHB_READY[2], PROPERTY_RIL_PHB_READY[3], NULL); 
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_ICCID_SIM[0], PROPERTY_ICCID_SIM[1], PROPERTY_ICCID_SIM[2], PROPERTY_ICCID_SIM[3], NULL);
    upadteSystemPropertyByCurrentModeGemini(rid, PROPERTY_ECC_LIST[0], PROPERTY_ECC_LIST[1],PROPERTY_ECC_LIST[2],PROPERTY_ECC_LIST[3], NULL);
}

// Regional Phone: boot animation START
#ifdef  MTK_RIL
static const RILId s_pollSimId = MTK_RIL_SOCKET_1;
#ifdef  MTK_GEMINI
static const RILId s_pollSimId2 = MTK_RIL_SOCKET_2;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
static const RILId s_pollSimId3 = MTK_RIL_SOCKET_3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
static const RILId s_pollSimId4 = MTK_RIL_SOCKET_4;
#endif
#endif  /* MTK_GEMINI */
#endif  /* MTK_RIL */

#define PROPERTY_RIL_SIM_MCC_MNC  "gsm.sim.ril.mcc.mnc"
#define PROPERTY_RIL_SIM_MCC_MNC_2  "gsm.sim.ril.mcc.mnc.2"
#define PROPERTY_RIL_SIM_MCC_MNC_3  "gsm.sim.ril.mcc.mnc.3"
#define PROPERTY_RIL_SIM_MCC_MNC_4  "gsm.sim.ril.mcc.mnc.4"

typedef struct MCCTABLE {
    int mcc;
    int mncLength;
} MccTable;

static MccTable s_mtk_mcc_table[] = {
#include "mtk_mcc_table.h"
};

static unsigned short hexCharToInt(char c)
{
    if(c >= '0' && c <= '9')
        return (unsigned short)(c - '0');
    else if( c >= 'A' && c <= 'F')
        return (unsigned short)(c- 'A') + (unsigned short)10;
    else if( c >= 'a' && c <= 'f')
        return (unsigned short)(c- 'a') + (unsigned short)10;
    return 0;
}

static void readMccMncForBootAnimationError(RILId rid, char *errStrCode) {
    char errStr[20] = {0};
    const char *propSim = PROPERTY_RIL_SIM_MCC_MNC;
    strcpy(errStr, errStrCode);
    #ifdef MTK_GEMINI
        #ifndef MTK_DT_SUPPORT

            int isSimSwitched = RIL_is3GSwitched();
            int simId3G = RIL_get3GSIM()-1;

            int simPropIdx = rid;
            if (isSimSwitched) {
                if (rid == MTK_RIL_SOCKET_1)
                    simPropIdx = simId3G;
                else if (rid == simId3G)
                    simPropIdx = MTK_RIL_SOCKET_1;
            }
            switch (simPropIdx) {
            case 0:
                propSim = PROPERTY_RIL_SIM_MCC_MNC;
                break;
            case 1:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_2;
                break;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
            case 2:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_3;
                break;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
            case 3:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_4;
                break;
#endif
            }
        #endif
    #endif
    LOGD("readMccMncForBootAnimationError%d: %s, %s", rid, propSim, errStr);
    property_set(propSim, errStr);
}

static void readMccMncForBootAnimation(RILId rid) {
    ATResponse *p_response = NULL;
    int isUsim = isUsimDetect[rid];
    int cmeError;
    int i;
    int mcc;
    int err;
    int sw1 = 0, sw2 = 0;
    int len = 0;
    char *cmd = NULL;
    char *line = NULL, *rsp = NULL;
    //char efadInfo[2*256+1] = {0};
    char imsi[16] = {0};
    char mccmnc[7] = {0};
    int mncLength = -1;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_size_struct fSize  = {0};
    const char *propSim = PROPERTY_RIL_SIM_MCC_MNC;

    LOGD("readMccMncForBootAnimation%d: entering", rid);
    // read IMSI
    err = at_send_command_numeric("AT+CIMI", &p_response, getRILChannelCtx(RIL_SIM, rid));
    if (err < 0 || p_response->success == 0) {
        cmeError = at_get_cme_error(p_response);
        LOGD("readMccMncForBootAnimation%d: fail %d %d %d", rid, err, p_response->success, cmeError);
        at_response_free(p_response);
        readMccMncForBootAnimationError(rid, "error");
        return;
    } else {
        strcpy(imsi, p_response->p_intermediates->line);
        at_response_free(p_response);
        p_response = NULL;
        if(imsi[0] == '\0' || (strlen(imsi) < 6 || strlen(imsi) > 15) ) {
            LOGD("readMccMncForBootAnimation%d: imsi fail %s", rid, imsi);
            readMccMncForBootAnimationError(rid, "sim_error");
            return;
        }
        LOGD("readMccMncForBootAnimation%d: imsi success %s", rid, imsi);
    }

    // get IccType
    if(isUsim == -1) {
        LOGD("readMccMncForBootAnimation%d: fail to get icc type", rid);
        goto ERROR_READ_EFAD_INFO;
    } else {
        LOGD("readMccMncForBootAnimation%d: icc type = %d", rid, isUsim);
    }

    // get EF_AD(0x6fad) information
    if(isUsim == 1) {
        err = at_send_command_singleline("AT+CRSM=192,28589,0,0,0,,\"7FFF\"", "+CRSM:", &p_response, getRILChannelCtx(RIL_SIM, rid));
    } else {
        err = at_send_command_singleline("AT+CRSM=192,28589,0,0,0,,\"7F20\"", "+CRSM:", &p_response, getRILChannelCtx(RIL_SIM, rid));
    }
    if (err < 0 || p_response->success == 0)
        goto ERROR_READ_EFAD_INFO;
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    err = at_tok_nextint(&line, &sw1);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    err = at_tok_nextint(&line, &sw2);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &rsp);
        if (err < 0) goto ERROR_READ_EFAD_INFO;
    }

    if(isUsim == 1 && rsp != NULL) { // USIM response
        if((len = strlen(rsp)) > 512)
            goto ERROR_READ_EFAD_INFO;

        // get USIM response's file size
        fcpLen = hexStringToByteArray(rsp, &fcpByte);
        if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_SIZE_T,&fSize)) {
            LOGD("readMccMncForBootAnimation%d: USIM EF_AD File Size fail", rid);
            goto ERROR_READ_EFAD_INFO;
        }
    } else if(isUsim == 0 && rsp != NULL && strlen(rsp) >= 8) { // get SIM response's file size
        fSize.file_size = (unsigned short)( (hexCharToInt(rsp[4]) << 12) | (hexCharToInt(rsp[5]) << 8) | (hexCharToInt(rsp[6]) << 4) | (hexCharToInt(rsp[7])) );
    } else {
        goto ERROR_READ_EFAD_INFO;
    }

    LOGD("readMccMncForBootAnimation%d: iccType = %d, EF_AD File Size = %u", rid, isUsim, fSize.file_size);
    at_response_free(p_response);
    p_response = NULL;

    // read EF_AD(0x6fad) content
    if(isUsim == 1) {
        asprintf(&cmd, "AT+CRSM=176,28589,0,0,%u,,\"7FFF\"", fSize.file_size);
    } else {
        asprintf(&cmd, "AT+CRSM=176,28589,0,0,%u,,\"7F20\"", fSize.file_size);
    }
    err = at_send_command_singleline(cmd, "+CRSM:", &p_response, getRILChannelCtx(RIL_SIM, rid));
    if (err < 0 || p_response->success == 0)
        goto ERROR_READ_EFAD_INFO;
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    err = at_tok_nextint(&line, &sw1);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    err = at_tok_nextint(&line, &sw2);
    if (err < 0) goto ERROR_READ_EFAD_INFO;
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &rsp);
        if (err < 0) goto ERROR_READ_EFAD_INFO;
    }

    if(sw1 == 0x90 && sw2 == 0x00 && rsp != NULL && strlen(rsp) >= 8) {
        mncLength = (hexCharToInt(rsp[6]) << 4) | (hexCharToInt(rsp[7]) );
        LOGD("readMccMncForBootAnimation%d: mccLength = %d", rid, mncLength);
        at_response_free(p_response);
        if(mncLength != 2 && mncLength != 3)
            goto ERROR_READ_EFAD_INFO;

        goto READ_EF_AD_INFO;
    } else {
        LOGD("readMccMncForBootAnimation%d: fail to read EF_AD", rid);
        at_response_free(p_response);
        goto ERROR_READ_EFAD_INFO;
    }

ERROR_READ_EFAD_INFO:
    // use database to get mncLength
    mncLength = -1;
    mcc = (hexCharToInt(imsi[0]) * 100) + (hexCharToInt(imsi[1]) * 10) + hexCharToInt(imsi[2]);
    len = sizeof(s_mtk_mcc_table) / sizeof(s_mtk_mcc_table[0]);
    for(i = 0; i < len; i++) {
        if(s_mtk_mcc_table[i].mcc == mcc) {
            mncLength = s_mtk_mcc_table[i].mncLength;
            LOGD("readMccMncForBootAnimation%d: MccTable: mcc = %d, mncLength = %d", rid, mcc, mncLength);
            break;
        }
    }
    if(mncLength == -1) {
        LOGD("readMccMncForBootAnimation%d: MccTable not found %d", rid, mcc);
        readMccMncForBootAnimationError(rid, "sim_error");
        return;
    }

READ_EF_AD_INFO:
    strncpy(mccmnc, imsi, 3 + mncLength);

    #ifdef MTK_GEMINI
        #ifndef MTK_DT_SUPPORT
            int isSimSwitched = RIL_is3GSwitched();
            int simId3G = RIL_get3GSIM()-1;

            int simPropIdx = rid;
            if (isSimSwitched) {
                if (rid == MTK_RIL_SOCKET_1)
                    simPropIdx = simId3G;
                else if (rid == simId3G)
                    simPropIdx = MTK_RIL_SOCKET_1;
            }
            switch (simPropIdx) {
            case 0:
                propSim = PROPERTY_RIL_SIM_MCC_MNC;
                break;
            case 1:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_2;
                break;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
            case 2:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_3;
                break;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
            case 3:
                propSim = PROPERTY_RIL_SIM_MCC_MNC_4;
                break;
#endif
            }
        #endif
    #endif
    property_set(propSim, mccmnc);
    LOGD("readMccMncForBootAnimation%d: mcc+mnc = %s in %s", rid, mccmnc, propSim);
}

static const struct timeval TIMEVAL_MccMnc = { 0, 100000 };


extern void requestSimStateForBootAnimation(void *param);

void querySimStateForBootAnimation(void *param) {
    RILId rid = *((RILId *) param);
    ATResponse *p_response = NULL;
    int err;
    int cmeError;
    char *cpinLine;
    char *cpinResult;

    static int rePollingTimes1 = 0;
    static int rePollingTimes2 = 0;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    static int rePollingTimes3 = 0;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    static int rePollingTimes4 = 0;
#endif

    //while(1) {
        int rePollingTimes = 0;
        if (rid == MTK_RIL_SOCKET_1) {
            rePollingTimes = ++rePollingTimes1;
#ifdef MTK_GEMINI
        } else if (rid == MTK_RIL_SOCKET_2) {
            rePollingTimes = ++rePollingTimes2;
#endif
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
        } else if (rid == MTK_RIL_SOCKET_3) {
            rePollingTimes = ++rePollingTimes3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
        } else if (rid == MTK_RIL_SOCKET_4) {
            rePollingTimes = ++rePollingTimes4;
#endif
        }

        LOGD("querySimStateForBootAnimation%d: entering %d", rid, rePollingTimes);

        err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getRILChannelCtx(RIL_SIM, rid));
        if (err != 0) {
            LOGD("querySimStateForBootAnimation%d: err: %d", rid, err);
            readMccMncForBootAnimationError(rid, "error");
            return;
        }
        if (p_response->success > 0) {
            LOGD("querySimStateForBootAnimation%d: response success", rid);
            cpinLine = p_response->p_intermediates->line;
            err = at_tok_start (&cpinLine);
            if(err < 0) {
                LOGD("querySimStateForBootAnimation%d: CME_SUCCESS: err1: %d", rid, err);
                readMccMncForBootAnimationError(rid, "modem_error");
                goto StopQuerySimStateForBootAnimation;
            }
            err = at_tok_nextstr(&cpinLine, &cpinResult);
            if(err < 0) {
                LOGD("querySimStateForBootAnimation%d: CME_SUCCESS: err2: %d", rid, err);
                readMccMncForBootAnimationError(rid, "modem_error");
                goto StopQuerySimStateForBootAnimation;
            }

            if(!strcmp (cpinResult, "READY")) {
                LOGD("querySimStateForBootAnimation%d: cpin-pass result: %s", rid, cpinResult);
                readMccMncForBootAnimation(rid);
                goto StopQuerySimStateForBootAnimation;
            } else {
                LOGD("querySimStateForBootAnimation%d: cpin-fail result: %s", rid, cpinResult);
                readMccMncForBootAnimationError(rid, "sim_lock");
                goto StopQuerySimStateForBootAnimation;
            }
        } else {
            cmeError = at_get_cme_error(p_response);
            LOGD("querySimStateForBootAnimation%d: cme_ERROR: %d", rid, cmeError);
            switch (cmeError) {
                case CME_SIM_BUSY:
                    goto ReQuerySimStateForBootAnimation;
                    break;
                case CME_SIM_NOT_INSERTED:
                    LOGD("querySimStateForBootAnimation%d: SIM absent", rid);
                    readMccMncForBootAnimationError(rid, "sim_absent");
                    goto StopQuerySimStateForBootAnimation;
                    break;
                default:
                    LOGD("querySimStateForBootAnimation%d: default: %d", rid, cmeError);
                    readMccMncForBootAnimationError(rid, "modem_error");
                    goto StopQuerySimStateForBootAnimation;
            }
        }
    //}

StopQuerySimStateForBootAnimation:
    at_response_free(p_response);
    LOGD("querySimStateForBootAnimation%d: Stop", rid);
    return;

ReQuerySimStateForBootAnimation:
    at_response_free(p_response);
    LOGD("querySimStateForBootAnimation%d: repoll", rid);
//    RIL_requestProxyTimedCallback (querySimStateForBootAnimation, param, &TIMEVAL_MccMnc, getChannelCtxbyProxy(rid)->id, "querySimStateForBootAnimation");
//    RIL_requestProxyTimedCallback (requestSimStateForBootAnimation, (void *)&rid, &TIMEVAL_MccMnc, getRILChannelCtx(RIL_SIM, rid)->id, "requestSimStateForBootAnimation");
    if (rid == MTK_RIL_SOCKET_1) {
        RIL_requestProxyTimedCallback (requestSimStateForBootAnimation, (void *)&animation_rid, &TIMEVAL_MccMnc, getRILChannelCtx(RIL_SIM, rid)->id, "requestSimStateForBootAnimation");
#ifdef MTK_GEMINI
    } else if (rid == MTK_RIL_SOCKET_2) {
        RIL_requestProxyTimedCallback (requestSimStateForBootAnimation, (void *)&animation_rid2, &TIMEVAL_MccMnc, getRILChannelCtx(RIL_SIM, rid)->id, "requestSimStateForBootAnimation");
#endif
    }
}

extern void issueLocalRequestForResponse(int request, void *data, int len, RILId id);

void requestSimStateForBootAnimation(void *param) {
    RILId rid = *((RILId *) param);
    LOGD("re-request RIL_LOCAL_REQUEST_RILD_READ_IMSI (%d)", rid);
    issueLocalRequestForResponse(RIL_LOCAL_REQUEST_RILD_READ_IMSI, (void *)&rid, 1, rid);
}

void requestMccMncForBootAnimation(RILId rid) {
    const RILId *p_rilId = &s_pollSimId;
#ifdef MTK_GEMINI
    if (MTK_RIL_SOCKET_2 == rid)
        p_rilId = &s_pollSimId2;
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    else if (MTK_RIL_SOCKET_3 == rid)
        p_rilId = &s_pollSimId3;
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    else if (MTK_RIL_SOCKET_4 == rid)
        p_rilId = &s_pollSimId4;
#endif
#endif
    querySimStateForBootAnimation(p_rilId);
}
// Regional Phone: boot animation END

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_UPDATE_SIM_SLOT_CFG _IOW(CCCI_IOC_MAGIC, 34, unsigned int)

void requestSetTelephonyMode(void *data, size_t datalen, RIL_Token t)
{
	LOGD("[SwitchPhone] requestSetTelephonyMode");

	int err;
	char *line;
	RIL_Errno ret = RIL_E_GENERIC_FAILURE;

	int* params = (int *) data;

	int ret_ioctl_val = -1;
	int ccci_sys_fd = -1;

	ccci_sys_fd = open("/dev/ccci_ioctl0", O_RDWR | O_NONBLOCK);
	if (ccci_sys_fd < 0) {
		LOGD("[requestSetTelephonyMode] id=%d with index=%d", ccci_sys_fd, index);
		goto error;
	}

	LOGD("[SwitchPhone] requestSetTelephonyMode %d, %d, %d", params[0], params[1], params[2]);
	ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_UPDATE_SIM_SLOT_CFG, params);
	LOGD("[SwitchPhone] ret_val=%d with index=%d", ret_ioctl_val, index);

	if (ret_ioctl_val  == -1) {
		LOGD("[SwitchPhone] strerror(errno)=%s", strerror(errno));
		goto error;
	}

	ret = RIL_E_SUCCESS;
error:
	close(ccci_sys_fd);
	RIL_onRequestComplete(t, ret, NULL, 0);
}
