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

#include SRSLIBINC(srs_cshp_api.h)
#include SRSLIBINC(srs_trubass_design_api.h)

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_CSHP"

namespace android {

struct SRS_Source_CSHP {
	SRSCshpObj		Obj;
	void*			pObjBuffer;
	void*			pObjWorkspace;
	
	int				SampleRate;
	int				ChannelCount;
	
	SRSCoef32		TBCoefs[SRS_SA_TRUBASS_COEFFICIENT_ARRAY_LEN];
	
	float			TBWant, TBActive;
	float			DFWant, DFActive;
	
	SRS_Tech_CSHP 	Active;
	bool			ForceActive;
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_CSHP();
	~SRS_Source_CSHP();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_CSHP* pCFG, bool bBypass);
	void Process_256(SRSSamp* pData);
};

SRS_Source_CSHP::SRS_Source_CSHP(){
	SRS_LOG("CSHP Created");
	
	DidCreate = false;
	DidConfig = false;
	
	pObjBuffer = malloc(SRS_CSHP_OBJ_SIZE);
		
	pObjWorkspace = malloc(SRS_CSHP_WORKSPACE_SIZE(256));
	
#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] CSHP: %d", SRS_CSHP_OBJ_SIZE+SRS_CSHP_WORKSPACE_SIZE(256)+sizeof(SRS_Source_CSHP));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_CSHP::~SRS_Source_CSHP(){
	if (pObjWorkspace != NULL) free(pObjWorkspace);
	pObjWorkspace = NULL;
	
	if (pObjBuffer != NULL) free(pObjBuffer);
	pObjBuffer = NULL;
	
	DidCreate = false;
	DidConfig = false;
	
	SRS_LOG("CSHP Destroyed");
}

void SRS_Source_CSHP::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
		
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;

	DidCreate = true;
	ForceActive = true;

	Obj = SRS_CreateCshpObj(pObjBuffer);
	
	if (SampleRate < 19000) SRS_InitCshpObj16k(Obj);
	else if (SampleRate < 23000) SRS_InitCshpObj22k(Obj);
	else if (SampleRate < 28000) SRS_InitCshpObj24k(Obj);
	else if (SampleRate < 38000) SRS_InitCshpObj32k(Obj);
	else if (SampleRate < 46000) SRS_InitCshpObj44k(Obj);
	else SRS_InitCshpObj48k(Obj);
	
	SRS_SetCshpControlDefaults(Obj);
	
	TBWant = TBActive = -1.0f;
	DFWant = DFActive = -1.0f;

	DidConfig = false;
}

void SRS_Source_CSHP::Config(SRS_Source_Out* pOut, SRS_Tech_CSHP* pCFG, bool bBypass){
	if (DidCreate == false) return;
		
	if (DIFF_FORCED(DoDecode)){
		if (pCFG->DoDecode) SRS_SetCshpRenderingMode(Obj, SRS_CS_DECODER);
		else SRS_SetCshpRenderingMode(Obj, SRS_PASSIVE_DECODER);
	}
	
	if (DIFF_FORCED(DecodeMode)){
		if (pCFG->DecodeMode == 0) SRS_SetCshpCSDecoderMode(Obj, SRS_CSD_MODE_CINEMA);
		else SRS_SetCshpCSDecoderMode(Obj, SRS_CSD_MODE_MUSIC);
		//SRS_CSD_MODE_PRO	= 1 << 1,
		//SRS_CSD_MODE_MONO	= 1 << 3,
		//SRS_CSD_MODE_LCRS	= 1 << 4
	}
	
	if (DIFF_FORCED(DoTB)) SRS_SetCshpTruBassEnable(Obj, pCFG->DoTB);
	if (DIFF_FORCED(TBMin) || DIFF_FORCED(TBWindow) || DIFF_FORCED(TBSlide)){
		TBWant = Tool_MaxZero(pCFG->TBMin+(pCFG->TBWindow*pCFG->TBSlide), 1.0f);
		//SRS_SetCshpTruBassControl(Obj, SRS_FXP16(TBWant, SRS_SATB_CTRL_IWL));
		if (ForceActive) TBActive = -1.0f;
	}
	if (DIFF_FORCED(TBCompress)) SRS_SetCshpTruBassCompressorCtrl(Obj, SRS_FXP16(Tool_MaxZero(pCFG->TBCompress, 1.0f), SRS_SATB_CTRL_IWL));
	
	if (DIFF_FORCED(TBMode)){
		if (pCFG->TBMode == 0) SRS_SetCshpTruBassMode(Obj, SRS_SATB_MODE_MONO);
		else SRS_SetCshpTruBassMode(Obj, SRS_SATB_MODE_STEREO);
	}
	
	if (DIFF_FORCED(TBFreq) || DIFF_FORCED(TBAnalyze)){

#ifdef _SRSCFG_ARCH_ARM
		char tWork[SRS_SA_TRUBASS_DESIGN_WORKSPACE_SIZE];
		SRS_SATruBassFilterDesignFxp32(pCFG->TBFreq, pCFG->TBAnalyze, SampleRate, TBCoefs, tWork);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
		SRS_SATruBassFilterDesignFloat(pCFG->TBFreq, pCFG->TBAnalyze, SampleRate, TBCoefs);
#endif	// _SRSCFG_ARCH_X86
		
		SRSSATruBassSpeakerSize tSetCustom;
		tSetCustom.AudioFilter = SRS_SATB_SPEAKER_LF_RESPONSE_Custom;
		tSetCustom.AnalysisFilter = SRS_SATB_SPEAKER_LF_RESPONSE_Custom;
		
		SRS_SetCshpTruBassSpeakerSize(Obj, tSetCustom);
		SRS_SetCshpTruBassCustomSpeakerFilterCoefs(Obj, TBCoefs);
	}
	
	if (DIFF_FORCED(DoTBSplit)) SRS_SetCshpTruBassSplitAnalysisEnable(Obj, pCFG->DoTBSplit);
	
	if (DIFF_FORCED(DoDef)) SRS_SetCshpDefinitionEnable(Obj, pCFG->DoDef);
	if (DIFF_FORCED(DefMin) || DIFF_FORCED(DefWindow) || DIFF_FORCED(DefSlide)){
		DFWant = Tool_MaxZero(pCFG->DefMin+(pCFG->DefWindow*pCFG->DefSlide), 1.0f);
		//SRS_SetCshpDefinitionControl(Obj, SRS_FXP16(DFWant, SRS_DEFINITION_IWL));
		if (ForceActive) DFActive = -1.0f;
	}
	
	if (DIFF_FORCED(DoDialog)) SRS_SetCshpDialogClarityEnable(Obj, pCFG->DoDialog);
	if (DIFF_FORCED(Dialog)) SRS_SetCshpDialogClarityControl(Obj, SRS_FXP16(Tool_MaxZero(pCFG->Dialog, 1.0f),SRS_FOCUS_IWL));
	
	if (DIFF_FORCED(DoLimit)) SRS_SetCshpLimiterEnable(Obj, pCFG->DoLimit);
	if (DIFF_FORCED(LimitMGain)) SRS_SetCshpLimiterMinimalGain(Obj, SRS_FXP32(pCFG->LimitMGain,2));
	
	if (DIFF_FORCED(IGain)) SRS_SetCshpInputGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->IGain, 1.0f),SRS_CSHP_GAIN_IWL));
	if (DIFF_FORCED(OGain)) SRS_SetCshpOutputGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->OGain, 1.0f),SRS_CSHP_GAIN_IWL));
	if (DIFF_FORCED(BGain)) SRS_SetCshpBypassGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->BGain, 1.0f),SRS_CSHP_GAIN_IWL));
			
	if (bBypass == false) SRS_SetCshpEnable(Obj, true);
	else SRS_SetCshpEnable(Obj, false);
	
	DidConfig = true;
	Active = *pCFG;
	ForceActive = false;
}

void SRS_Source_CSHP::Process_256(SRSSamp* pData){
	if (TBWant != TBActive){	// Slide into requested TB value?
		if (TBActive == -1.0f) TBActive = TBWant;
		else Tool_SeekValue(TBWant, TBActive);
		
		SRS_SetCshpTruBassControl(Obj, SRS_FXP16(TBActive, SRS_SATB_CTRL_IWL));
	}
	
	if (DFWant != DFActive){	// Slide into requested Def value?
		if (DFActive == -1.0f) DFActive = DFWant;
		else Tool_SeekValue(DFWant, DFActive);
		
		SRS_SetCshpDefinitionControl(Obj, SRS_FXP16(DFActive, SRS_DEFINITION_IWL));
	}
	
	SRSStereoCh inChans;
	inChans.Left = pData;
	inChans.Right = pData+256;
	
	SRS_CSHeadphone(Obj, &inChans, 256, pObjWorkspace);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_CSHP* SRS_Create_CSHP(SRS_Source_Out* pOut){	
	SRS_Source_CSHP* pSrc = new SRS_Source_CSHP();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_CSHP(SRS_Source_CSHP* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_CSHP(SRS_Source_CSHP* pSrc, SRS_Source_Out* pOut, SRS_Tech_CSHP* pCFG, bool bBypass){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bBypass);
}

void SRS_Process_CSHP_256(SRS_Source_CSHP* pSrc, SRSSamp* pData){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pData);
}

enum {
	CSHP_IGain,
	CSHP_OGain,
	CSHP_BGain,
	CSHP_DoTB,
	CSHP_TBMin,
	CSHP_TBWindow,
	CSHP_TBSlide,
	CSHP_TBSlideUD,
	CSHP_TBCompress,
	CSHP_TBFreq,
	CSHP_TBAnalyze,
	CSHP_TBDoSplit,
	CSHP_TBMode,
	CSHP_DoDef,
	CSHP_DefMin,
	CSHP_DefWindow,
	CSHP_DefSlide,
	CSHP_DefSlideUD,
	CSHP_DoDecode,
	CSHP_DecodeMode,
	CSHP_DoDialog,
	CSHP_Dialog,
	CSHP_DoLimit,
	CSHP_LimitMGain,
	CSHP_BSize,
	CSHP_Skip,
};

SRS_Param gCSHP_Params[] = {
	{ CSHP_IGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"cshp_igain", "Input Gain", "gain", "", 0},
	{ CSHP_OGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"cshp_ogain", "Output Gain", "gain", "", 0},
	{ CSHP_BGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"cshp_bgain", "Bypass Gain", "gain", "", 0},
	{ CSHP_DoTB,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"cshp_trubass_enable", "TruBass Toggle", "toggle", "", 0},
	{ CSHP_TBMin,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.0f,	0.0f,	1.0f,	"cshp_trubass_min", "TruBass Min", "control", "", 0},
	{ CSHP_TBWindow,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"cshp_trubass_window", "TruBass Window", "control", "", 0},
	{ CSHP_TBSlide,		SRS_PTYP_PREF,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"cshp_trubass_slide", "TruBass Slider", "control", "", 0},
	{ CSHP_TBSlideUD,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"cshp_trubass_slide_udef", "TruBass Slider - User Default", "control", "", 0},
	{ CSHP_TBCompress,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.25f,	0.0f,	1.0f,	"cshp_trubass_compressor", "TruBass Compressor", "control", "", 0},
	{ CSHP_TBFreq,		SRS_PTYP_ALIAS,	SRS_PFMT_INT,		400.0f,	20.0f,	1200.0f,	"cshp_trubass_size", "TruBass Speaker Size", "frequency", "", 0},
	{ CSHP_TBFreq,		SRS_PTYP_CFG,	SRS_PFMT_INT,		400.0f,	20.0f,	1200.0f,	"cshp_trubass_freq", "TruBass Frequency", "frequency", "", 0},
	{ CSHP_TBAnalyze,	SRS_PTYP_CFG,	SRS_PFMT_INT,		200.0f,	40.0f,	1200.0f,	"cshp_trubass_analysis", "TruBass Analysis Size", "frequency", "", 0},
	{ CSHP_TBDoSplit,	SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	0.0f,	"cshp_trubass_sa_enable", "TruBass Split Analysis Toggle", "toggle", "", 0},
	{ CSHP_TBMode,		SRS_PTYP_CFG,	SRS_PFMT_ENUM,		0.0f,	0.0f,	0.0f,	"cshp_trubass_mode", "TruBass Mode", "enum", "Mono,Stereo", 0},
	{ CSHP_DoDef,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.0f,	0.0f,	1.0f,	"cshp_definition_enable", "Definition Toggle", "toggle", "", 0},
	{ CSHP_DefMin,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.0f,	0.0f,	1.0f,	"cshp_definition_min", "Definition Min", "control", "", 0},
	{ CSHP_DefWindow,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"cshp_definition_window", "Definition Window", "control", "", 0},
	{ CSHP_DefSlide,	SRS_PTYP_PREF,	SRS_PFMT_FLOAT,		0.43f,	0.0f,	1.0f,	"cshp_definition_slide", "Definition Slide", "control", "", 0},
	{ CSHP_DefSlideUD,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.43f,	0.0f,	1.0f,	"cshp_definition_slide_udef", "Definition Slide - User Default", "control", "", 0},
	{ CSHP_DoDecode,	SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"cshp_csdecode_enable", "CS Decoder Toggle", "toggle", "", 0},
	{ CSHP_DecodeMode,	SRS_PTYP_CFG,	SRS_PFMT_ENUM,		0.0f,	0.0f,	0.0f,	"cshp_csdecode_mode", "CS Decoder Mode", "enum", "Cinema,Music", 0},
	{ CSHP_DoDialog,	SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"cshp_dialog_enable", "Dialog Toggle", "toggle", "", 0},
	{ CSHP_Dialog,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.3f,	0.0f,	1.0f,	"cshp_dialog", "Dialog", "control", "", 0},
	{ CSHP_DoLimit,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"cshp_limiter_enable", "Limiter Toggle", "toggle", "", 0},
	{ CSHP_LimitMGain,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.3f,	0.0f,	1.0f,	"cshp_limiter_gain", "Limiter Minimum Gain", "control", "", 0},
	{ CSHP_Skip,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"cshp_skip", "Skips CSHP when true", "toggle", "", 0},
};

SRS_Param* SRS_GetBank_CSHP(int& paramCount){
	paramCount = sizeof(gCSHP_Params)/sizeof(SRS_Param);
	return gCSHP_Params;
}

char* SRS_GetVersion_CSHP(char* pWork, size_t maxBytes){
	int v1 = SRS_GetCSHPTechVersion(SRS_VERSION_MAJOR);
	int v2 = SRS_GetCSHPTechVersion(SRS_VERSION_MINOR);
	int v3 = SRS_GetCSHPTechVersion(SRS_VERSION_REVISION);
	int v4 = SRS_GetCSHPTechVersion(SRS_VERSION_RELEASE);
	int l1 = SRS_GetCSHPLibVersion(SRS_VERSION_MAJOR);
	int l2 = SRS_GetCSHPLibVersion(SRS_VERSION_MINOR);
	int l3 = SRS_GetCSHPLibVersion(SRS_VERSION_REVISION);
	int l4 = SRS_GetCSHPLibVersion(SRS_VERSION_RELEASE);
	snprintf(pWork, maxBytes, "%d.%d.%d.%d - lib %d.%d.%d.%d", v1, v2, v3, v4, l1, l2, l3, l4);
	return pWork;
}

void SRS_SetParam_CSHP(SRS_Tech_CSHP* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case CSHP_IGain: pCFG->IGain = In.GetFloat(pValue); break;
		case CSHP_OGain: pCFG->OGain = In.GetFloat(pValue); break;
		case CSHP_BGain: pCFG->BGain = In.GetFloat(pValue); break;
		case CSHP_DoTB: pCFG->DoTB = In.GetBool(pValue); break;
		case CSHP_TBMin: pCFG->TBMin = In.GetFloat(pValue); break;
		case CSHP_TBWindow: pCFG->TBWindow = In.GetFloat(pValue); break;
		case CSHP_TBSlide: pCFG->TBSlide = In.GetFloat(pValue); break;
		case CSHP_TBSlideUD: pCFG->TBSlideUDef = In.GetFloat(pValue); break;
		case CSHP_TBCompress: pCFG->TBCompress = In.GetFloat(pValue); break;
		case CSHP_TBFreq: pCFG->TBFreq = In.GetInt(pValue); break;
		case CSHP_TBAnalyze: pCFG->TBAnalyze = In.GetInt(pValue); break;
		case CSHP_TBDoSplit: pCFG->DoTBSplit = In.GetBool(pValue); break;
		case CSHP_TBMode: pCFG->TBMode = In.GetInt(pValue); break;
		case CSHP_DoDef: pCFG->DoDef = In.GetBool(pValue); break;
		case CSHP_DefMin: pCFG->DefMin = In.GetFloat(pValue); break;
		case CSHP_DefWindow: pCFG->DefWindow = In.GetFloat(pValue); break;
		case CSHP_DefSlide: pCFG->DefSlide = In.GetFloat(pValue); break;
		case CSHP_DefSlideUD: pCFG->DefSlideUDef = In.GetFloat(pValue); break;
		case CSHP_DoDecode: pCFG->DoDecode = In.GetBool(pValue); break;
		case CSHP_DecodeMode: pCFG->DecodeMode = In.GetInt(pValue); break;
		case CSHP_DoDialog: pCFG->DoDialog = In.GetBool(pValue); break;
		case CSHP_Dialog: pCFG->Dialog = In.GetFloat(pValue); break;
		case CSHP_DoLimit: pCFG->DoLimit = In.GetBool(pValue); break;
		case CSHP_LimitMGain: pCFG->LimitMGain = In.GetFloat(pValue); break;
		case CSHP_Skip: pCFG->Skip = In.GetBool(pValue); break;
	}
}

const char* SRS_GetParam_CSHP(SRS_Tech_CSHP* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case CSHP_IGain: return Out.FromFloat(pCFG->IGain);
		case CSHP_OGain: return Out.FromFloat(pCFG->OGain);
		case CSHP_BGain: return Out.FromFloat(pCFG->BGain);
		case CSHP_DoTB: return Out.FromBool(pCFG->DoTB);
		case CSHP_TBMin: return Out.FromFloat(pCFG->TBMin);
		case CSHP_TBWindow: return Out.FromFloat(pCFG->TBWindow);
		case CSHP_TBSlide: return Out.FromFloat(pCFG->TBSlide);
		case CSHP_TBSlideUD: return Out.FromFloat(pCFG->TBSlideUDef);
		case CSHP_TBCompress: return Out.FromFloat(pCFG->TBCompress);
		case CSHP_TBFreq: return Out.FromInt(pCFG->TBFreq);
		case CSHP_TBAnalyze: return Out.FromInt(pCFG->TBAnalyze);
		case CSHP_TBDoSplit: return Out.FromBool(pCFG->DoTBSplit);
		case CSHP_TBMode: return Out.FromInt(pCFG->TBMode);
		case CSHP_DoDef: return Out.FromBool(pCFG->DoDef);
		case CSHP_DefMin: return Out.FromFloat(pCFG->DefMin);
		case CSHP_DefWindow: return Out.FromFloat(pCFG->DefWindow);
		case CSHP_DefSlide: return Out.FromFloat(pCFG->DefSlide);
		case CSHP_DefSlideUD: return Out.FromFloat(pCFG->DefSlideUDef);
		case CSHP_DoDecode: return Out.FromBool(pCFG->DoDecode);
		case CSHP_DecodeMode: return Out.FromInt(pCFG->DecodeMode);
		case CSHP_DoDialog: return Out.FromBool(pCFG->DoDialog);
		case CSHP_Dialog: return Out.FromFloat(pCFG->Dialog);
		case CSHP_DoLimit: return Out.FromBool(pCFG->DoLimit);
		case CSHP_LimitMGain: return Out.FromFloat(pCFG->LimitMGain);
		case CSHP_Skip: return Out.FromBool(pCFG->Skip);
	}

	return "";
}

void SRS_Default_CSHP(SRS_Tech_CSHP* pCFG){
	
#ifndef _SRSCFG_WOWHDX
	pCFG->IGain = 0.6f;
#else	// _SRSCFG_WOWHDX
	pCFG->IGain = 1.0f;
#endif	// _SRSCFG_WOWHDX

	pCFG->OGain = 1.0f;
	pCFG->BGain = 1.0f;
	pCFG->DoTB = true;
	pCFG->DoDef = true;
	pCFG->DoDecode = false;
	pCFG->DoDialog = true;
	pCFG->DoLimit = true;
	pCFG->TBMin = 0.0f;
	pCFG->TBWindow = 0.5f;
	pCFG->TBSlideUDef = pCFG->TBSlide = 0.6f;
	
#ifdef _SRSCFG_WOWHDX
	pCFG->TBCompress = 0.20f;
#else	// _SRSCFG_WOWHDX
	pCFG->TBCompress = 0.25f;
#endif	// _SRSCFG_WOWHDX

	pCFG->TBFreq = 80;
	pCFG->TBAnalyze = 60;
	pCFG->DoTBSplit = true;
	pCFG->TBMode = 1;
	pCFG->DefMin = 0.0f;
	pCFG->DefWindow = 0.3f;
	pCFG->DefSlideUDef = pCFG->DefSlide = 0.15f;
	pCFG->Dialog = 0.1f;
	pCFG->LimitMGain = 0.45f;
	pCFG->DecodeMode = 0;
	pCFG->Skip = false;
}

void SRS_UserDefault_CSHP(SRS_Tech_CSHP* pCFG){
	pCFG->TBSlide = pCFG->TBSlideUDef;
	pCFG->DefSlide = pCFG->DefSlideUDef;
}

};

