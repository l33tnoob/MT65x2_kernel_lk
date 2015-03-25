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
 *  Exposes all srs_cshp APIs
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_cshp/std_fxp/include/srs_cshp_api.h#7 $
 *  $Author: oscarh $
 *  $Date: 2010/11/16 $
 *	
********************************************************************************/

#ifndef __SRS_CSHP_API_H__
#define __SRS_CSHP_API_H__

#include "srs_cshp_def.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

SRSCshpObj			SRS_CreateCshpObj(void *pBuf);

void				SRS_InitCshpObj16k(SRSCshpObj cshpObj);
void				SRS_InitCshpObj22k(SRSCshpObj cshpObj);
void				SRS_InitCshpObj24k(SRSCshpObj cshpObj);
void				SRS_InitCshpObj32k(SRSCshpObj cshpObj);
void				SRS_InitCshpObj44k(SRSCshpObj cshpObj);
void				SRS_InitCshpObj48k(SRSCshpObj cshpObj);

void				SRS_SetCshpControlDefaults(SRSCshpObj cshpObj);

void				SRS_CSHeadphone(SRSCshpObj cshpObj, SRSStereoCh *audioIO, int blockSize, void *ws);

//"Set" functions:
void				SRS_SetCshpEnable(SRSCshpObj cshpObj, int enable);
SRSResult			SRS_SetCshpInputGain(SRSCshpObj cshpObj, srs_int16 gain);	//I16.1
SRSResult			SRS_SetCshpOutputGain(SRSCshpObj cshpObj, srs_int16 gain);	//I16.1
SRSResult			SRS_SetCshpBypassGain(SRSCshpObj cshpObj, srs_int16 gain);	//I16.1
SRSResult			SRS_SetCshpRenderingMode(SRSCshpObj cshpObj, SRSCshpRenderingMode mode);

//CSDecoder:
SRSResult			SRS_SetCshpCSDecoderMode(SRSCshpObj cshpObj, SRSCSDecoderMode mode);

//Focus (DialogClarity):
void				SRS_SetCshpDialogClarityEnable(SRSCshpObj cshpObj, int enable);
SRSResult			SRS_SetCshpDialogClarityControl(SRSCshpObj cshpObj, srs_int16 value); //I16.SRS_FOCUS_IWL (I16.1)

//Definition:
void				SRS_SetCshpDefinitionEnable(SRSCshpObj cshpObj, int enable);
SRSResult			SRS_SetCshpDefinitionControl(SRSCshpObj cshpObj, srs_int16 value); //I16.SRS_DEFINITION_IWL (I16.1)

//TruBass:
void				SRS_SetCshpTruBassEnable(SRSCshpObj cshpObj, int enable);
SRSResult			SRS_SetCshpTruBassControl(SRSCshpObj cshpObj, srs_int16 value);	//I16.SRS_TB_CTRL_IWL (I16.1)
SRSResult			SRS_SetCshpTruBassCompressorCtrl(SRSCshpObj cshpObj, srs_int16 ctrl); //compressor control, 0~1.0, default 0.25, I16.SRS_SATB_CTRL_IWL
SRSResult			SRS_SetCshpTruBassSpeakerSize(SRSCshpObj cshpObj, SRSSATruBassSpeakerSize sz);
SRSResult			SRS_SetCshpTruBassCustomSpeakerFilterCoefs(SRSCshpObj cshpObj, const srs_int32 *coefs); //Set custom speaker coefficients
SRSResult			SRS_SetCshpTruBassMode(SRSCshpObj cshpObj, SRSSATruBassMode mode);
void				SRS_SetCshpTruBassSplitAnalysisEnable(SRSCshpObj cshpObj, int enable);				//disabled by default

//Limiter:
void				SRS_SetCshpLimiterEnable(SRSCshpObj cshpObj, int enable);
SRSResult			SRS_SetCshpLimiterMinimalGain(SRSCshpObj cshpObj, srs_int32 value); //I32.2

//"Get" functions:
int					SRS_GetCshpEnable(SRSCshpObj cshpObj);
srs_int16			SRS_GetCshpInputGain(SRSCshpObj cshpObj);	//max gain: <1
srs_int16			SRS_GetCshpOutputGain(SRSCshpObj cshpObj);	//max gain: <1
srs_int16			SRS_GetCshpBypassGain(SRSCshpObj cshpObj);	//max gain: <1
SRSCshpRenderingMode SRS_GetCshpRenderingMode(SRSCshpObj cshpObj);

//CSDecoder:
SRSCSDecoderMode	SRS_GetCshpCSDecoderMode(SRSCshpObj cshpObj);

//Focus (DialogClarity):
int					SRS_GetCshpDialogClarityEnable(SRSCshpObj cshpObj);
srs_int16			SRS_GetCshpDialogClarityControl(SRSCshpObj cshpObj);

//Definition:
int					SRS_GetCshpDefinitionEnable(SRSCshpObj cshpObj);
srs_int16			SRS_GetCshpDefinitionControl(SRSCshpObj cshpObj);

//TruBass:
int					SRS_GetCshpTruBassEnable(SRSCshpObj cshpObj);
srs_int16			SRS_GetCshpTruBassControl(SRSCshpObj cshpObj);
srs_int16			SRS_GetCshpTruBassCompressorCtrl(SRSCshpObj cshpObj);
SRSSATruBassSpeakerSize	SRS_GetCshpTruBassSpeakerSize(SRSCshpObj cshpObj);
SRSSATruBassMode		SRS_GetCshpTruBassMode(SRSCshpObj cshpObj);
SRSSATruBassCustomSpeakerCoefs	SRS_GetCshpTruBassCustomSpeakerFilterCoefs(SRSCshpObj cshpObj);
int					SRS_GetCshpTruBassSplitAnalysisEnable(SRSCshpObj cshpObj);

//Limiter:
int					SRS_GetCshpLimiterEnable(SRSCshpObj cshpObj);
srs_int32			SRS_GetCshpLimiterMinimalGain(SRSCshpObj cshpObj);

//Version info get functions:
unsigned char SRS_GetCSHPTechVersion(SRSVersion which);
unsigned char SRS_GetCSHPLibVersion(SRSVersion which);

#ifdef __cplusplus
}
#endif /*__cplusplus*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif /*__SRS_CSHP_API_H__*/
