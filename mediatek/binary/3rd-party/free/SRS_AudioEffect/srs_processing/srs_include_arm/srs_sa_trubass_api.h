/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/********************************************************************************
 *	SRS Labs CONFIDENTIAL
 *	@Copyright 2010 by SRS Labs.
 *	All rights reserved.
 *
 *  Description:
 *  Exposes all srs_trubass APIs
 *
 *  RCS keywords:
 *	$Id: //users/shawnq/srstech/srs_sa_trubass/std_fxp/include/srs_sa_trubass_api.h#3 $
 *  $Author: shawnq $
 *  $Date: 2010/12/07 $
 *	
********************************************************************************/

#ifndef __SRS_SA_TRUBASS_API_H__
#define __SRS_SA_TRUBASS_API_H__

#include "srs_sa_trubass_def.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

SRSSATruBassObj	SRS_CreateSATruBassObj(void* pBuf);

void			SRS_InitSATruBassObj8k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj11k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj16k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj22k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj24k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj32k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj44k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj48k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj88k(SRSSATruBassObj tbObj);
void			SRS_InitSATruBassObj96k(SRSSATruBassObj tbObj);

void			SRS_SATruBassProcess(SRSSATruBassObj tbObj, SRSStereoCh *audioIO, int blockSize, void* ws);
void			SRS_SATruBass(SRSSATruBassObj tbObj, SRSStereoCh *audioIO, int blockSize, void* ws);

void			SRS_SetSATruBassControlDefaults(SRSSATruBassObj tbObj);

void			SRS_SetSATruBassEnable(SRSSATruBassObj tbObj, int enable);
void			SRS_SetSATruBassLevelIndependentEnable(SRSSATruBassObj tbObj, int enable);

void			SRS_SetSATruBassOutputOption(SRSSATruBassObj tbObj, SRSSATruBassOutputOption opt);

SRSResult		SRS_SetSATruBassInputGain(SRSSATruBassObj tbObj, srs_int16 gain);	//I16.SRS_SATB_GAIN_IWL
SRSResult		SRS_SetSATruBassOutputGain(SRSSATruBassObj tbObj, srs_int16 gain);	//I16.SRS_SATB_GAIN_IWL
SRSResult		SRS_SetSATruBassBypassGain(SRSSATruBassObj tbObj, srs_int16 gain);	//I16.SRS_SATB_GAIN_IWL
SRSResult		SRS_SetSATruBassControl(SRSSATruBassObj tbObj, srs_int16 ctrl);		//I16.SRS_SATB_CTRL_IWL
SRSResult		SRS_SetSATruBassCompressorCtrl(SRSSATruBassObj tbObj, srs_int16 ctrl);	//I16.SRS_SATB_CTRL_IWL

SRSResult		SRS_SetSATruBassSpeakerSize(SRSSATruBassObj tbObj, SRSSATruBassSpeakerSize sz);
SRSResult		SRS_SetSATruBassMode(SRSSATruBassObj tbObj, SRSSATruBassMode mode);

/* Set the custom speaker filter coefficients */
/* "coefs" must point to an array of srs_int32 with length 16. Coeffcients must be arranged in the pattern as type SRSSATruBassCustomSpeakerCoefs */
/* The memory pointed to by "custom" is copied by the Set function, then no longer used */
void			SRS_SetSATruBassCustomSpeakerFilterCoefs(SRSSATruBassObj tbObj, const srs_int32 *coefs);

int				SRS_GetSATruBassEnable(SRSSATruBassObj tbObj);
int				SRS_GetSATruBassLevelIndependentEnable(SRSSATruBassObj tbObj);
srs_int16		SRS_GetSATruBassInputGain(SRSSATruBassObj tbObj);
srs_int16		SRS_GetSATruBassOutputGain(SRSSATruBassObj tbObj);
srs_int16		SRS_GetSATruBassBypassGain(SRSSATruBassObj tbObj);
srs_int16		SRS_GetSATruBassControl(SRSSATruBassObj tbObj);
srs_int16		SRS_GetSATruBassCompressorCtrl(SRSSATruBassObj tbObj);


SRSSATruBassMode	SRS_GetSATruBassMode(SRSSATruBassObj tbObj);
SRSSATruBassSpeakerSize  SRS_GetSATruBassSpeakerSize(SRSSATruBassObj tbObj);
SRSSATruBassOutputOption	SRS_GetSATruBassOutputOption(SRSSATruBassObj tbObj);
SRSSATruBassCustomSpeakerCoefs	SRS_GetSATruBassCustomSpeakerFilteCoefs(SRSSATruBassObj tbObj);

unsigned char	SRS_GetSATruBassTechVersion(SRSVersion which);
unsigned char	SRS_GetSATruBassLibVersion(SRSVersion which);

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /*__SRS_SA_TRUBASS_API_H__*/
