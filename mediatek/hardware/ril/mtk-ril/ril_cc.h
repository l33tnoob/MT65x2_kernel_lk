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

/* //hardware/ril/reference-ril/ril_cc.h
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
#ifndef RIL_CC_H 
#define RIL_CC_H 1

extern void requestGetCurrentCalls(void * data, size_t datalen, RIL_Token t);
extern void requestDial(void * data, size_t datalen, RIL_Token t, int isEmergency);
extern void requestHangup(void * data, size_t datalen, RIL_Token t);
extern void requestHangupWaitingOrBackground(void * data, size_t datalen, RIL_Token t);
extern void requestHangupForegroundResumeBackground(void * data, size_t datalen, RIL_Token t);
extern void requestSwitchWaitingOrHoldingAndActive(void * data, size_t datalen, RIL_Token t);
extern void requestAnswer(void * data, size_t datalen, RIL_Token t);
extern void requestConference(void * data, size_t datalen, RIL_Token t);
extern void requestUdub(void * data, size_t datalen, RIL_Token t);
extern void requestSeparateConnection(void * data, size_t datalen, RIL_Token t);
extern void requestExplicitCallTransfer(void * data, size_t datalen, RIL_Token t);
extern void requestLastCallFailCause(void * data, size_t datalen, RIL_Token t);
extern void requestDtmf(void * data, size_t datalen, RIL_Token t);

/* MTK proprietary start */
extern void requestHangupAll(void * data, size_t datalen, RIL_Token t);
extern void requestForceReleaseCall(void * data, size_t datalen, RIL_Token t);
extern void requestSetCallIndication(void * data, size_t datalen, RIL_Token t);
extern void requestGetCcm(void * data, size_t datalen, RIL_Token t);
extern void requestGetAcm(void * data, size_t datalen, RIL_Token t);
extern void requestGetAcmMax(void * data, size_t datalen, RIL_Token t);
extern void requestGetPpuAndCurrency(void * data, size_t datalen, RIL_Token t);
extern void requestSetAcmMax(void * data, size_t datalen, RIL_Token t);
extern void requestResetAcm(void * data, size_t datalen, RIL_Token t);
extern void requestSetPpuAndCurrency(void * data, size_t datalen, RIL_Token t);

extern void requestDtmfStart(void * data, size_t datalen, RIL_Token t);
extern void requestDtmfStop(void * data, size_t datalen, RIL_Token t);
extern void requestSetTTYMode(void * data, size_t datalen, RIL_Token t);


//MTK-START [mtk04070][120104][ALPS00109412]Solve "Disable modem VT capability if AP VT compile option is closed"
//Merge from ALPS00096155
extern void requestDisableVTCapability(void * data, size_t datalen, RIL_Token t);
//MTK-END [mtk04070][120104][ALPS00109412]Solve "Disable modem VT capability if AP VT compile option is closed"
/* MTK proprietary end */
#ifdef MTK_VT3G324M_SUPPORT
extern void requestVtDial(void * data, size_t datalen, RIL_Token t);
extern void requestVoiceAccept(void * data, size_t datalen, RIL_Token t);
extern void requestReplaceVtCall(void * data, size_t datalen, RIL_Token t);
#endif

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
#endif /* RIL_CC_H */

