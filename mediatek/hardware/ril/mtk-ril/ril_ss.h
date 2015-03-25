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

/* //hardware/ril/reference-ril/ril_ss.h
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

#ifndef RIL_SS_H 
#define RIL_SS_H 1
        
/* USSD messages using the default alphabet are coded with the GSM 7-bit default alphabet         *
 * given in clause 6.2.1. The message can then consist of up to 182 user characters (3GPP 23.038) */
#define MAX_RIL_USSD_STRING_LENGTH 255
    
//BEGIN mtk08470 [20130109][ALPS00436983]
// number string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
#define MAX_RIL_USSD_NUMBER_LENGTH 160
//END mtk08470 [20130109][ALPS00436983]  

/*
 * <classx> is a sum of integers each representing a class of information (default 7):
 * 1    voice (telephony)
 * 2    data (refers to all bearer services; with <mode>=2 this may refer only to some bearer service if TA does not support values 16, 32, 64 and 128)
 * 4    fax (facsimile services)
 * 8    short message service
 * 16   data circuit sync
 * 32   data circuit async
 * 64   dedicated packet access
 * 128  dedicated PAD access
 */
typedef enum {
    CLASS_NONE                      = 0,
    CLASS_VOICE                     = 1,
    CLASS_DATA                      = 2,
    CLASS_FAX                       = 4,
    CLASS_DEFAULT                   = 7,
    CLASS_SMS                       = 8,
    CLASS_DATA_SYNC                 = 16,
    CLASS_DATA_ASYNC                = 32,
    CLASS_DEDICATED_PACKET_ACCESS   = 64,
    CLASS_DEDICATED_PAD_ACCESS      = 128,
    CLASS_MTK_LINE2                 = 256,
    CLASS_MTK_VIDEO                 = 512
} AtInfoClassE;

typedef enum {
    BS_ALL_E                        = 0,
    BS_TELE_ALL_E                   = 10,
    BS_TELEPHONY_E                  = 11,
    BS_TELE_DATA_ALL_E              = 12,
    BS_TELE_FAX_E                   = 13,
    BS_TELE_SMS_E                   = 16,
    BS_TELE_VGCS_E                  = 17, /* Not supported by framework */
    BS_TELE_VBS_E                   = 18, /* Not supported by framework */
    BS_TELE_ALL_EXCEPT_SMS_E        = 19,
    BS_DATA_ALL_E                   = 20,
    BS_DATA_ASYNC_ALL_E             = 21,
    BS_DATA_SYNC_ALL_E              = 22,
    BS_DATA_CIRCUIT_SYNC_E          = 24,
    BS_DATA_CIRCUIT_ASYNC_E         = 25,
    BS_DATA_SYNC_TELE_E             = 26, /* Supported by framework */
    BS_GPRS_ALL_E                   = 99
} BsCodeE;

/***
 * "AO"	BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"	BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"	BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"	BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"	BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"	All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"	All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"	All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 */

typedef enum {
    CB_BAOC,
    CB_BOIC,
    CB_BOIC_EXHC,
    CB_BAIC,
    CB_BIC_ROAM,
    CB_ABS,
    CB_AOBS,
    CB_AIBS,
    CB_SUPPORT_NUM
} CallBarServicesE;

extern const char *InfoClassToMmiBSCodeString (AtInfoClassE infoClass);
extern int MmiBSCodeToInfoClassX (int serviceCode);
extern const char *callBarFacToServiceCodeStrings(const char * fac);
extern int rilSsMain(int request, void *data, size_t datalen, RIL_Token t);    
extern int rilSsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

/* MTK proprietary start */
extern void requestColpOperation(void * data, size_t datalen, RIL_Token t);
extern void requestGetColp(void * data, size_t datalen, RIL_Token t);
extern void requestSetColp(void * data, size_t datalen, RIL_Token t);
extern void onCfuNotify(char *s, RILId rid);
/* MTK proprietary end */

#endif /* RIL_SS_H */


