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

/* //hardware/ril/reference-ril/ril_callbacks.h
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
#ifndef RIL_CALLBACKS_H 
#define RIL_CALLBACKS_H 1

#include <ril_sim.h>
#include <ril_stk.h>
#include <ril_cc.h>
#include <ril_ss.h>
#include <ril_sms.h>
#include <ril_data.h>
#include <ril_nw.h>
#include <ril_oem.h>

#ifdef RIL_SHLIB
extern const struct RIL_Env *s_rilenv;

#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c,d) s_rilenv->OnUnsolicitedResponse(a,b,c,d)
#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)
#ifdef MTK_RIL
#define RIL_requestProxyTimedCallback(a,b,c,d,e) \
		LOGD("%s request timed callback %s to %s", __FUNCTION__, e, proxyIdToString((int)d)); \
		s_rilenv->RequestProxyTimedCallback(a,b,c,(int)d)
#define RIL_queryMyChannelId(a) s_rilenv->QueryMyChannelId(a)
#define RIL_queryMyProxyIdByThread  s_rilenv->QueryMyProxyIdByThread()
#endif
#endif /* RIL_SHLIB */

/* RILcheckPendingRequest() used to query if any RIL request is pending currently */
extern int RILcheckPendingRequest(int request);
extern void setRadioState(RIL_RadioState newState, RILId rid);
extern RIL_RadioState getRadioState(RILId rid);

#define PROPERTY_3G_SIM "gsm.3gswitch"

static char PROPERTY_ICCID_SIM[4][25] = {
    "ril.iccid.sim1",
    "ril.iccid.sim2",
    "ril.iccid.sim3",
    "ril.iccid.sim4",
};

#define PROPERTY_FLIGHT_MODE_POWER_OFF_MD "ril.flightmode.poweroffMD"

#define TELEPHONY_MODE_0_NONE 0
#define TELEPHONY_MODE_1_WG_GEMINI 1
#define TELEPHONY_MODE_2_TG_GEMINI 2
#define TELEPHONY_MODE_3_FDD_SINGLE 3
#define TELEPHONY_MODE_4_TDD_SINGLE 4
#define TELEPHONY_MODE_5_WGNTG_DUALTALK 5
#define TELEPHONY_MODE_6_TGNG_DUALTALK 6
#define TELEPHONY_MODE_7_WGNG_DUALTALK 7
#define TELEPHONY_MODE_8_GNG_DUALTALK 8

#define TELEPHONY_MODE_100_TDNC_DUALTALK 100
#define TELEPHONY_MODE_101_FDNC_DUALTALK 101
#define TELEPHONY_MODE_102_WNC_DUALTALK 102
#define TELEPHONY_MODE_103_TNC_DUALTALK 103

#define FIRST_MODEM_NOT_SPECIFIED 0
#define FIRST_MODEM_MD1 1
#define FIRST_MODEM_MD2 2

#define NETWORK_MODE_GSM_ONLY 1
#define NETWORK_MODE_WCDMA_PREF 3
#define CAPABILITY_3G_SIM1 1
#define CAPABILITY_3G_SIM2 2
#define CAPABILITY_3G_SIM3 3
#define CAPABILITY_3G_SIM4 4
#define CAPABILITY_NO_3G -1

/* SIM mode. It's bitmap value. bit1 for SIM1, bit 2 for SIM2 ,bit 3 for SIM3 ... */
#define RADIO_MODE_POWER_OFF -1
#define RADIO_MODE_FLIGHT_MODE 0
#define RADIO_MODE_SIM1_ONLY 1
#define RADIO_MODE_SIM2_ONLY 2    
#define RADIO_MODE_SIM3_ONLY 4    	
#define RADIO_MODE_SIM4_ONLY 8    	
#define RADIO_MODE_DUAL_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY
#define RADIO_MODE_TRIPLE_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY | RADIO_MODE_SIM3_ONLY	
#define RADIO_MODE_QUAD_SIM RADIO_MODE_SIM1_ONLY | RADIO_MODE_SIM2_ONLY | RADIO_MODE_SIM3_ONLY | RADIO_MODE_SIM4_ONLY		

extern int RIL_get3GSIM();
extern int RIL_is3GSwitched();
extern int getTelephonyMode();
extern int getFirstModem();

extern int isEmulatorRunning();
extern int isCCCIPowerOffModem();
extern int isDualTalkMode();
extern void mtk_initializeCallback(void *param);
extern int getExternalModemSlot();
extern int isInternationalRoamingEnabled();
extern int isEVDODTSupport();
extern int getExternalModemSlotTelephonyMode();
extern int isEvdoOnDualtalkMode();
extern int isSupportCommonSlot();
extern int isWorldPhoneSupport();

#endif /* RIL_CALLBACKS_H */


