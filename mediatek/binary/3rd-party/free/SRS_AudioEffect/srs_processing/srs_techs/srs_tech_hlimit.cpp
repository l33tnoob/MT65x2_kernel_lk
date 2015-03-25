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

#include "srs_techs/srs_tech_headers.h"

#include SRSLIBINC(srs_hardlimiter_api.h)

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_HLimit"

namespace android {

struct SRS_Source_HLimit {
	void* pBuffer;
	void* pTemp;
	SRSHardLimiterObj Obj;
	
	SRS_Tech_HLimit Active;
	bool			ForceActive;
	
	int				SampleRate;
	int				ChannelCount;
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_HLimit();
	~SRS_Source_HLimit();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_HLimit* pCFG, bool bBypass);
	void Process_256(SRSSamp* pIn, SRSSamp* pOut);
};

SRS_Source_HLimit::SRS_Source_HLimit(){
	SRS_LOG("HLimit Created");
	
	DidCreate = false;
	DidConfig = false;
	
	pBuffer = malloc(SRS_HARDLIMITER_OBJ_SIZE*4);
	pTemp = malloc(SRS_HARDLIMITER_WORKSPACE_SIZE(256)*4);
	
#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] HL: %d", ((SRS_HARDLIMITER_OBJ_SIZE+SRS_HARDLIMITER_WORKSPACE_SIZE(256))*4)+sizeof(SRS_Source_HLimit));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_HLimit::~SRS_Source_HLimit(){
	if (pBuffer != NULL) free(pBuffer);
	pBuffer = NULL;
	if (pTemp != NULL) free(pTemp);
	pTemp = NULL;
	
	DidCreate = false;
	DidConfig = false;
	
	SRS_LOG("HLimit Destroyed");
}

void SRS_Source_HLimit::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
	
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;
	
	DidCreate = true;
	ForceActive = true;
		
	Obj = SRS_CreateHardLimiterObj(pBuffer);
	
	SRS_InitHardLimiterObj(Obj);
	SRS_SetHardLimiterControlDefaults(Obj);
}

void SRS_Source_HLimit::Config(SRS_Source_Out* pOut, SRS_Tech_HLimit* pCFG, bool bBypass){
	if (DidCreate == false) return;
	
	if (DIFF_FORCED(IGain)) SRS_SetHardLimiterInputGain(Obj, SRS_FXP16(pCFG->IGain,SRS_HL_INOUT_GAIN_IWL));
	if (DIFF_FORCED(OGain)) SRS_SetHardLimiterOutputGain(Obj, SRS_FXP16(pCFG->OGain,SRS_HL_INOUT_GAIN_IWL));
	if (DIFF_FORCED(BGain)) SRS_SetHardLimiterBypassGain(Obj, SRS_FXP16(pCFG->BGain,SRS_HL_BYPASS_GAIN_IWL));
	
	if (DIFF_FORCED(DelayLen)) SRS_SetHardLimiterDelaylen(Obj, pCFG->DelayLen);
	
	if (DIFF_FORCED(Boost)) SRS_SetHardLimiterBoost(Obj, SRS_FXP32(pCFG->Boost,SRS_HL_BOOST_IWL));
	if (DIFF_FORCED(Limit)) SRS_SetHardLimiterLimit(Obj, SRS_FXP16(pCFG->Limit,SRS_HL_LIMIT_IWL));
	
	if (DIFF_FORCED(DecaySmooth)) SRS_SetHardLimiterDecaySmoothEnable(Obj, pCFG->DecaySmooth);
	
	if (bBypass) SRS_SetHardLimiterEnable(Obj, false);
	else SRS_SetHardLimiterEnable(Obj, true);
	
	DidConfig = true;
	Active = *pCFG;
	ForceActive = false;
}

void SRS_Source_HLimit::Process_256(SRSSamp* pIn, SRSSamp* pOut){
	SRSStereoCh inChans;
	SRSStereoCh outChans;
	inChans.Left = pIn;
	inChans.Right = pIn+256;
	outChans.Left = pOut;
	outChans.Right = pOut+256;
	
	SRS_HardLimiter(Obj, &inChans, &outChans, 256, pTemp);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_HLimit* SRS_Create_HLimit(SRS_Source_Out* pOut){	
	SRS_Source_HLimit* pSrc = new SRS_Source_HLimit();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_HLimit(SRS_Source_HLimit* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_HLimit(SRS_Source_HLimit* pSrc, SRS_Source_Out* pOut, SRS_Tech_HLimit* pCFG, bool bBypass){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bBypass);
}

void SRS_Process_HLimit_256(SRS_Source_HLimit* pSrc, SRSSamp* pIn, SRSSamp* pOut){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pIn, pOut);
}

enum {
	HL_IGain,
	HL_OGain,
	HL_BGain,
	HL_Skip,
	HL_DelayLen,
	HL_DecaySmooth,
	HL_Boost,
	HL_Limit,
};

SRS_Param gHLimit_Params[] = {
	{ HL_IGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"hlimit_igain", "Input Gain", "gain", "", 0},
	{ HL_OGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"hlimit_ogain", "Output Gain", "gain", "", 0},
	{ HL_BGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"hlimit_bgain", "Bypass Gain", "gain", "", 0},
	{ HL_DelayLen,		SRS_PTYP_CFG,	SRS_PFMT_INT,		16.0f,	8.0f,	32.0f,	"hlimit_delaylen", "Length of the Delay Line", "length", "", 0},
	{ HL_DecaySmooth,	SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"hlimit_decaysmooth", "Toggles use of smoothing on Decay", "toggle", "", 0},
	{ HL_Boost,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.001f,	256.0f,	"hlimit_boost", "Boost", "gain", "", 0},
	{ HL_Limit,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"hlimit_limit", "Limit", "control", "", 0},
	{ HL_Skip,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"hlimit_skip", "Skips Hard Limiter when true", "toggle", "", 0},
};

SRS_Param* SRS_GetBank_HLimit(int& paramCount){
	paramCount = sizeof(gHLimit_Params)/sizeof(SRS_Param);
	return gHLimit_Params;
}

char* SRS_GetVersion_HLimit(char* pWork, size_t maxBytes){
	int v1 = SRS_GetHardLimiterTechVersion(SRS_VERSION_MAJOR);
	int v2 = SRS_GetHardLimiterTechVersion(SRS_VERSION_MINOR);
	int v3 = SRS_GetHardLimiterTechVersion(SRS_VERSION_REVISION);
	int v4 = SRS_GetHardLimiterTechVersion(SRS_VERSION_RELEASE);
	int l1 = SRS_GetHardLimiterLibVersion(SRS_VERSION_MAJOR);
	int l2 = SRS_GetHardLimiterLibVersion(SRS_VERSION_MINOR);
	int l3 = SRS_GetHardLimiterLibVersion(SRS_VERSION_REVISION);
	int l4 = SRS_GetHardLimiterLibVersion(SRS_VERSION_RELEASE);
	snprintf(pWork, maxBytes, "%d.%d.%d.%d - lib %d.%d.%d.%d", v1, v2, v3, v4, l1, l2, l3, l4);
	return pWork;
}

void SRS_SetParam_HLimit(SRS_Tech_HLimit* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case HL_IGain: pCFG->IGain = In.GetFloat(pValue); break;
		case HL_OGain: pCFG->OGain = In.GetFloat(pValue); break;
		case HL_BGain: pCFG->BGain = In.GetFloat(pValue); break;
		case HL_Skip: pCFG->Skip = In.GetBool(pValue); break;
		case HL_DelayLen: pCFG->DelayLen = In.GetInt(pValue); break;
		case HL_DecaySmooth: pCFG->DecaySmooth = In.GetBool(pValue); break;
		case HL_Boost: pCFG->Boost = In.GetFloat(pValue); break;
		case HL_Limit: pCFG->Limit = In.GetFloat(pValue); break;
	}
}

const char* SRS_GetParam_HLimit(SRS_Tech_HLimit* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case HL_IGain: return Out.FromFloat(pCFG->IGain);
		case HL_OGain: return Out.FromFloat(pCFG->OGain);
		case HL_BGain: return Out.FromFloat(pCFG->BGain);
		case HL_Skip: return Out.FromBool(pCFG->Skip);
		case HL_DelayLen: return Out.FromInt(pCFG->DelayLen);
		case HL_DecaySmooth: return Out.FromBool(pCFG->DecaySmooth);
		case HL_Boost: return Out.FromFloat(pCFG->Boost);
		case HL_Limit: return Out.FromFloat(pCFG->Limit);
	}

	return "";
}

void SRS_Default_HLimit(SRS_Tech_HLimit* pCFG, bool Boosted){
	pCFG->IGain = 1.0f;
	pCFG->OGain = 1.0f;
	pCFG->BGain = 1.0f;
	pCFG->Skip = false;
	pCFG->DelayLen = 22;
	
#ifdef _SRSCFG_WOWHDX
	pCFG->Boost = 5.5f;
	if (Boosted) pCFG->Boost = 10.0f;
#else	// _SRSCFG_WOWHDX
	pCFG->Boost = 1.0f;
	if (Boosted) pCFG->Boost = 2.5f;
#endif	// _SRSCFG_WOWHDX
	
	pCFG->Limit = 1.0f;
	pCFG->DecaySmooth = true;
}

};

