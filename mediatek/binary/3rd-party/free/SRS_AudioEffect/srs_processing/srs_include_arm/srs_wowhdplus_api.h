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
 *  Exposes all srs_wowhdplus APIs
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/

#ifndef __SRS_WOWHDPLUS_API_H__
#define __SRS_WOWHDPLUS_API_H__

#include "srs_wowhdplus_def.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

//API declaration here:
SRSWowhdPlusObj		SRS_CreateWowhdPlusObj(void *pBuf);

SRSResult			SRS_InitWowhdPlusObj44k(SRSWowhdPlusObj wpObj, const SRSWowhdPlusFilterConfig *pFilterConfig, void *statBuf); //Using default filters if pFilterConfig is NULL
SRSResult			SRS_InitWowhdPlusObj48k(SRSWowhdPlusObj wpObj, const SRSWowhdPlusFilterConfig *pFilterConfig, void *statBuf);

SRSResult			SRS_WowhdPlusProcess(SRSWowhdPlusObj wpObj, SRSStereoCh *audioIO, void *ws);

void				SRS_SetWowhdPlusControlDefaults(SRSWowhdPlusObj wpObj);

//Overall APIs:
SRSResult			SRS_SetWowhdPlusInputGain(SRSWowhdPlusObj wpObj, srs_int16 gain);
SRSResult			SRS_SetWowhdPlusOutputGain(SRSWowhdPlusObj wpObj, srs_int16 gain);
void				SRS_SetWowhdPlusBassEnhancementEnable(SRSWowhdPlusObj wpObj, int enable);
SRSResult			SRS_WowhdPlusEnableAllComponents(SRSWowhdPlusObj wpObj);		//Enable all components in the signal path
SRSResult			SRS_WowhdPlusDisableAllComponents(SRSWowhdPlusObj wpObj);	//Disable all components, switching them to bypass mode

//TruBass APIs:
SRSResult			SRS_SetWowhdPlusSATbCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl);		//TruBass control
SRSResult			SRS_SetWowhdPlusSATbMode(SRSWowhdPlusObj wpObj, SRSSATruBassMode mode); //mono mode or stereo mode
SRSResult			SRS_SetWowhdPlusSATbSpeakerSize(SRSWowhdPlusObj wpObj, SRSSATruBassSpeakerSize size); 
SRSResult			SRS_SetWowhdPlusSATbCustomSpeakerCoefs(SRSWowhdPlusObj wpObj, const srs_int32 *coefs); //Set custom speaker filter coefficients
SRSResult			SRS_SetWowhdPlusSATbCompressorCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl); //Compressor control: 0.0~1.0: I16.SRS_SATB_CTRL_IWL

//Post TruBass HPF:
void				SRS_SetWowhdPlusPostTBHpfEnable(SRSWowhdPlusObj wpObj, int enable); //Turn on/off the post TruBass HPF

//SRS3D APIs:
void				SRS_SetWowhdPlusSrs3DEnable(SRSWowhdPlusObj wpObj, int enable);
SRSResult			SRS_SetWowhdPlusSrs3DMode(SRSWowhdPlusObj wpObj, SRSSrs3dMode mode);
SRSResult			SRS_SetWowhdPlusSrs3DSpaceCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl);
SRSResult			SRS_SetWowhdPlusSrs3DCenterCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl);
void				SRS_SetWowhdPlusSrs3DHeadphone(SRSWowhdPlusObj wpObj, int isHeadphone);
void				SRS_SetWowhdPlusSrs3DHighBitRate(SRSWowhdPlusObj wpObj, int isHighBitRate);

//Focus APIs:
void				SRS_SetWowhdPlusFocusEnable(SRSWowhdPlusObj wpObj, int enable);
SRSResult			SRS_SetWowhdPlusFocusCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl);

//Definition APIs:
void				SRS_SetWowhdPlusDefinitionEnable(SRSWowhdPlusObj wpObj, int enable);
SRSResult			SRS_SetWowhdPlusDefinitionCtrl(SRSWowhdPlusObj wpObj, srs_int16 ctrl);

//WideSurround APIs:
void				SRS_SetWowhdPlusWdSrdEnable(SRSWowhdPlusObj wpObj, int enable);
SRSResult			SRS_SetWowhdPlusWdSrdInputGain(SRSWowhdPlusObj wpObj, srs_int16 gain);
SRSResult			SRS_SetWowhdPlusWdSrdCenterBoostGain(SRSWowhdPlusObj wpObj, srs_int16 gain);
SRSResult			SRS_SetWowhdPlusWdSrdSpeakerSeparation(SRSWowhdPlusObj wpObj, srs_int16 separation);
void				SRS_SetWowhdPlusWdSrdHandsetHPFEnable(SRSWowhdPlusObj wpObj, int enable);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Control Get functions:
//Overall APIs:
srs_int16				SRS_GetWowhdPlusInputGain(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusOutputGain(SRSWowhdPlusObj wpObj);
int						SRS_GetWowhdPlusBassEnhancementEnable(SRSWowhdPlusObj wpObj);

//TruBass APIs:
srs_int16				SRS_GetWowhdPlusSATbCtrl(SRSWowhdPlusObj wpObj);		//TruBass control
SRSSATruBassMode		SRS_GetWowhdPlusSATbMode(SRSWowhdPlusObj wpObj); //mono mode or stereo mode
SRSSATruBassSpeakerSize					SRS_GetWowhdPlusSATbSpeakerSize(SRSWowhdPlusObj wpObj); 
SRSSATruBassCustomSpeakerCoefs			SRS_GetWowhdPlusSATbCustomSpeakerCoefs(SRSWowhdPlusObj wpObj); //Set custom speaker filter coefficients
srs_int16				SRS_GetWowhdPlusSATbCompressorCtrl(SRSWowhdPlusObj wpObj);

//Post TruBass HPF:
int						SRS_GetWowhdPlusPostTBHpfEnable(SRSWowhdPlusObj wpObj);

//SRS3D APIs:
int						SRS_GetWowhdPlusSrs3DEnable(SRSWowhdPlusObj wpObj);
SRSSrs3dMode			SRS_GetWowhdPlusSrs3DMode(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusSrs3DSpaceCtrl(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusSrs3DCenterCtrl(SRSWowhdPlusObj wpObj);
int						SRS_GetWowhdPlusSrs3DHeadphone(SRSWowhdPlusObj wpObj);
int						SRS_GetWowhdPlusSrs3DHighBitRate(SRSWowhdPlusObj wpObj);

//Focus APIs:
int						SRS_GetWowhdPlusFocusEnable(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusFocusCtrl(SRSWowhdPlusObj wpObj);

//Definition APIs:
int						SRS_GetWowhdPlusDefinitionEnable(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusDefinitionCtrl(SRSWowhdPlusObj wpObj);

//WideSurround APIs:
int						SRS_GetWowhdPlusWdSrdEnable(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusWdSrdInputGain(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusWdSrdCenterBoostGain(SRSWowhdPlusObj wpObj);
srs_int16				SRS_GetWowhdPlusWdSrdSpeakerSeparation(SRSWowhdPlusObj wpObj);
int						SRS_GetWowhdPlusWdSrdHandsetHPFEnable(SRSWowhdPlusObj wpObj);


//version info query functions
unsigned char	SRS_GetWowhdPlusTechVersion(SRSVersion which);
unsigned char	SRS_GetWowhdPlusLibVersion(SRSVersion which);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__SRS_WOWHDPLUS_API_H__*/
