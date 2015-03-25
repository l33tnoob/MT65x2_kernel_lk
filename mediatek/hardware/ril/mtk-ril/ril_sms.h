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

/* //hardware/ril/reference-ril/ril_sms.h
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
#ifndef RIL_SMS_H 
#define RIL_SMS_H 1

#define MAX_SMSC_LENGTH 11
#define MAX_TPDU_LENGTH 164

/******************************
* Although CB CHANNEL ID can be 65535 defined in the spec.
* But currently, all application only support the ID 0-999
* so we define the MAX_CB_CHANNEL_ID as 10000
******************************/
#define MAX_CB_CHANNEL_ID   1000
#define MAX_CB_DCS_ID           256

typedef enum
{
    RIL_SMS_REC_UNREAD,
    RIL_SMS_REC_RDAD,
    RIL_SMS_STO_UNSENT,
    RIL_SMS_STO_SENT,
    RIL_SMS_MESSAGE_MAX
}RIL_SMS_MESSAGE_STAT;

typedef struct 
{
    void *data;
    void *next;
}RIL_SMS_Linked_list;

#define SMS_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RIL_SMS_MEM_TYPE_TOTAL          3
#define RIL_SMS_GOTO_CHECK(condition, label)    \
			if (condition)                      \
				goto label

#define RIL_SMS_GOTO_DONE_CHECK(error)  RIL_SMS_GOTO_CHECK((error < 0), done)
#define RIL_SMS_GOTO_ERR_CHECK(error)   RIL_SMS_GOTO_CHECK((error < 0), err)

extern void requestSendSMS(void * data, size_t datalen, RIL_Token t);

extern void requestSendSmsExpectMore(void * data, size_t datalen, RIL_Token t);

extern void requestSMSAcknowledge(void * data, size_t datalen, RIL_Token t);

extern void requestWriteSmsToSim(void * data, size_t datalen, RIL_Token t);

extern void requestDeleteSmsOnSim(void *data, size_t datalen, RIL_Token t);

extern void requestGSMGetBroadcastSMSConfig(void *data, size_t datalen, RIL_Token t);

extern void requestGSMSetBroadcastSMSConfig(void *data, size_t datalen, RIL_Token t);

extern void requestGSMSMSBroadcastActivation(void *data, size_t datalen, RIL_Token t);

extern void requestGetSMSCAddress(void *data, size_t datalen, RIL_Token t);

extern void requestSetSMSCAddress(void *data, size_t datalen, RIL_Token t);

extern void onNewSms(const char *urc, const char *smspdu, RILId rid);

extern void onNewSmsStatusReport(const char *urc, const char *smspdu, RILId rid);

extern void onNewSmsOnSim(const char *urc, RILId rid);

extern void onSimSmsStorageStatus(const char *line, RILId rid);

extern void onNewBroadcastSms(const char *urc, const char *smspdu, RILId rid);

extern int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

/* MTK proprietary start */
extern void requestGetSmsSimMemoryStatus(void *data, size_t datalen, RIL_Token t);
extern void requestReportSMSMemoryStatus(void *data, size_t datalen, RIL_Token t);
extern void requestGetSmsParams(void *data, size_t datalen, RIL_Token t);
extern void requestSetSmsParams(void *data, size_t datalen, RIL_Token t);
extern void requestSetEtws(void *data, size_t datalen, RIL_Token t);
extern void requestSetCbChannelConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestSetCbLanguageConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestGetCellBroadcastConfigInfo(void *data, size_t datalen, RIL_Token t);
extern void requestSetAllCbLanguageOn(void *data, size_t datalen, RIL_Token t);

extern void requestGSMGetBroadcastSMSConfigEx(void *data, size_t datalen, RIL_Token t);
extern void requestGSMSetBroadcastSMSConfigEx(void *data, size_t datalen, RIL_Token t);


extern void onNewEtwsNotification(const char *line, RILId rid);
/* MTK proprietary end */

#endif /* RIL_SMS_H */

