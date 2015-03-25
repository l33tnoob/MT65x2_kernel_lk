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

#include SRSLIBINC(srs_wowhd_api.h)
#include SRSLIBINC(srs_trubass_design_api.h)

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_WOWHD"

namespace android {

struct SRS_Source_WOWHD {
	SRSWowhdObj		Obj;
	void*			pObjBuffer;
	void*			pObjWorkspace;
	
	int				SampleRate;
	int				ChannelCount;
	
	srs_int32		TBCoefs[SRS_SA_TRUBASS_COEFFICIENT_ARRAY_LEN];
	
	float			TBWant, TBActive;
	float			DFWant, DFActive;
	
	SRS_Tech_WOWHD 	Active;			// Cache of all current values for the API
    bool			ForceActive;	// Ignore differences - force update of Active
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_WOWHD();
	~SRS_Source_WOWHD();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_WOWHD* pCFG, bool bBypass);
	void Process_256(int32_t* pIn, int32_t* pOut);
};

SRS_Source_WOWHD::SRS_Source_WOWHD(){
	SRS_LOG("WOWHD Created");
	
	DidCreate = false;
	DidConfig = false;
	
	pObjBuffer = malloc(SRS_WOWHD_OBJ_SIZE);
	pObjWorkspace = malloc(SRS_WOWHD_WORKSPACE_SIZE(256));
	
#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] WOWHD: %d", SRS_WOWHD_OBJ_SIZE+SRS_WOWHD_WORKSPACE_SIZE(256)+sizeof(SRS_Source_WOWHD));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_WOWHD::~SRS_Source_WOWHD(){
	if (pObjWorkspace != NULL) free(pObjWorkspace);
	pObjWorkspace = NULL;
	
	if (pObjBuffer != NULL) free(pObjBuffer);
	pObjBuffer = NULL;
	
	DidCreate = false;
	DidConfig = false;
	
	SRS_LOG("WOWHD Destroyed");
}

void SRS_Source_WOWHD::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
	
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;

	DidCreate = true;
	ForceActive = true;
	
	Obj = SRS_CreateWowhdObj(pObjBuffer);
	
	if (SampleRate < 9500) SRS_InitWowhdObj8k(Obj);
	else if (SampleRate < 13000) SRS_InitWowhdObj11k(Obj);
	else if (SampleRate < 19000) SRS_InitWowhdObj16k(Obj);
	else if (SampleRate < 23000) SRS_InitWowhdObj22k(Obj);
	else if (SampleRate < 28000) SRS_InitWowhdObj24k(Obj);
	else if (SampleRate < 38000) SRS_InitWowhdObj32k(Obj);
	else if (SampleRate < 46000) SRS_InitWowhdObj44k(Obj);
	else if (SampleRate < 68000) SRS_InitWowhdObj48k(Obj);
	else if (SampleRate < 92000) SRS_InitWowhdObj88k(Obj);
	else SRS_InitWowhdObj96k(Obj);
	
	SRS_SetWowhdControlDefaults(Obj);
	
	TBWant = TBActive = -1.0f;
	DFWant = DFActive = -1.0f;
}

void SRS_Source_WOWHD::Config(SRS_Source_Out* pOut, SRS_Tech_WOWHD* pCFG, bool bBypass){
	if (DidCreate == false) return;
	
	if (DIFF_FORCED(IGain)) SRS_SetWowhdInputGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->IGain, 1.0f), SRS_WOWHD_GAIN_IWL));
	if (DIFF_FORCED(OGain)) SRS_SetWowhdOutputGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->OGain, 1.0f), SRS_WOWHD_GAIN_IWL));
	if (DIFF_FORCED(BGain)) SRS_SetWowhdBypassGain(Obj, SRS_FXP16(Tool_MaxZero(pCFG->BGain, 1.0f), SRS_WOWHD_GAIN_IWL));
	
	if (DIFF_FORCED(DoSRS3D)) SRS_SetWowhdSrs3dEnable(Obj, pCFG->DoSRS3D);
	if (DIFF_FORCED(Space)) SRS_SetWowhdSrs3dSpaceCtrl(Obj, SRS_FXP16(pCFG->Space, 1));
	if (DIFF_FORCED(Center)) SRS_SetWowhdSrs3dCenterCtrl(Obj, SRS_FXP16(pCFG->Center, 1));
	
	if (DIFF_FORCED(SRSType)){
		if (pCFG->SRSType == 0) SRS_SetWowhdSrs3dHeadphone(Obj, 0);	// SRSType == 0 - Speaker...
		else SRS_SetWowhdSrs3dHeadphone(Obj, 1);	// SRSType == 1 - Headphones...
	}
	
	if (DIFF_FORCED(SRSMode)){
		if (pCFG->SRSMode == 0) SRS_SetWowhdSrs3dMode(Obj, SRSSrs3dMono);
		else if (pCFG->SRSMode == 1) SRS_SetWowhdSrs3dMode(Obj, SRSSrs3dSingleSpeaker);
		else if (pCFG->SRSMode == 2) SRS_SetWowhdSrs3dMode(Obj, SRSSrs3dStereo);
		else SRS_SetWowhdSrs3dMode(Obj, SRSSrs3dExtreme);
	}
	
	if (DIFF_FORCED(DoTB)) SRS_SetWowhdTBEnable(Obj, pCFG->DoTB);
	
	if (DIFF_FORCED(TBMode)){
		if (pCFG->TBMode == 0) SRS_SetWowhdTBMode(Obj, SRS_SATB_MODE_MONO);
		else SRS_SetWowhdTBMode(Obj, SRS_SATB_MODE_STEREO);
	}
	if (DIFF_FORCED(TBCompress)) SRS_SetWowhdTBCompressorCtrl(Obj, SRS_FXP16(Tool_MaxZero(pCFG->TBCompress, 1.0f), SRS_SATB_CTRL_IWL));
	
	if (DIFF_FORCED(TBFreq) || DIFF_FORCED(TBAnalyze)){
		char tWork[SRS_SA_TRUBASS_DESIGN_WORKSPACE_SIZE];
		
		SRS_SATruBassFilterDesignFxp32(pCFG->TBFreq, pCFG->TBAnalyze, SampleRate, TBCoefs, tWork);
		
		SRSSATruBassSpeakerSize tSetCustom;
		tSetCustom.AudioFilter = SRS_SATB_SPEAKER_LF_RESPONSE_Custom;
		tSetCustom.AnalysisFilter = SRS_SATB_SPEAKER_LF_RESPONSE_Custom;
		
		SRS_SetWowhdTBSpeakerSize(Obj, tSetCustom);
		SRS_SetWowhdTBCustomSpeakerFilterCoefs(Obj, TBCoefs);
	}
	
	if (DIFF_FORCED(DoTBSplit)) SRS_SetWowhdTBSplitAnalysisEnable(Obj, pCFG->DoTBSplit);
	
	if (DIFF_FORCED(TBMin) || DIFF_FORCED(TBWindow) || DIFF_FORCED(TBSlide)){
		TBWant = Tool_MaxZero(pCFG->TBMin+(pCFG->TBWindow*pCFG->TBSlide), 1.0f);
		//SRS_SetWowhdTBControl(Obj, SRS_FXP16(TBWant, SRS_SATB_CTRL_IWL));
		if (ForceActive) TBActive = -1.0f;
	}

	if (DIFF_FORCED(DoFocus)) SRS_SetWowhdFocusEnable(Obj, pCFG->DoFocus);
	if (DIFF_FORCED(Focus)) SRS_SetWowhdFocusFactor(Obj, SRS_FXP16(Tool_MaxZero(pCFG->Focus, 1.0f), 1));
	
	if (DIFF_FORCED(DoDef)) SRS_SetWowhdDefEnable(Obj, pCFG->DoDef);
	
	if (DIFF_FORCED(DefMin) || DIFF_FORCED(DefWindow) || DIFF_FORCED(DefSlide)){
		DFWant = Tool_MaxZero(pCFG->DefMin+(pCFG->DefWindow*pCFG->DefSlide), 1.0f);
		//SRS_SetWowhdDefFactor(Obj, SRS_FXP16(DFWant, SRS_DEFINITION_IWL));
		if (ForceActive) DFActive = -1.0f;
	}
	
	if (DIFF_FORCED(DoLimit)) SRS_SetWowhdLmtEnable(Obj, pCFG->DoLimit);
	if (DIFF_FORCED(LimitGain)) SRS_SetWowhdLmtMinimalGain(Obj, SRS_FXP32(Tool_MaxZero(pCFG->LimitGain, 1.0f), 2));
	
	if (bBypass) SRS_SetWowhdEnable(Obj, 0);
	else SRS_SetWowhdEnable(Obj, 1);
		
	DidConfig = true;
	Active = *pCFG;
	ForceActive = false;
}

void SRS_Source_WOWHD::Process_256(int32_t* pIn, int32_t* pOut){
	if (TBWant != TBActive){	// Slide into requested TB value?
		if (TBActive == -1.0f) TBActive = TBWant;
		else Tool_SeekValue(TBWant, TBActive);
		
		SRS_SetWowhdTBControl(Obj, SRS_FXP16(TBActive, SRS_SATB_CTRL_IWL));
	}
	
	if (DFWant != DFActive){	// Slide into requested Def value?
		if (DFActive == -1.0f) DFActive = DFWant;
		else Tool_SeekValue(DFWant, DFActive);
		
		SRS_SetWowhdDefFactor(Obj, SRS_FXP16(DFActive, SRS_DEFINITION_IWL));
	}
	
	SRSStereoCh inChans;
	SRSStereoCh outChans;
	inChans.Left = pIn;
	inChans.Right = pIn+256;
	outChans.Left = pOut;
	outChans.Right = pOut+256;
	
	SRS_Wowhd(Obj, &inChans, &outChans, 256, pObjWorkspace);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_WOWHD* SRS_Create_WOWHD(SRS_Source_Out* pOut){
	SRS_Source_WOWHD* pSrc = new SRS_Source_WOWHD();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_WOWHD(SRS_Source_WOWHD* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_WOWHD(SRS_Source_WOWHD* pSrc, SRS_Source_Out* pOut, SRS_Tech_WOWHD* pCFG, bool bBypass){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bBypass);
}

void SRS_Process_WOWHD_256(SRS_Source_WOWHD* pSrc, int32_t* pIn, int32_t* pOut){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pIn, pOut);
}

enum {
	WHD_IGain,
	WHD_OGain,
	WHD_BGain,
	WHD_DoTB,
	WHD_TBMin,
	WHD_TBWindow,
	WHD_TBSlide,
	WHD_TBSlideUD,
	WHD_TBCompress,
	WHD_TBFreq,
	WHD_TBAnalyze,
	WHD_TBDoSplit,
	WHD_TBMode,
	WHD_DoSRS3D,
	WHD_SRSType,
	WHD_SRSMode,
	WHD_Center,
	WHD_Space,
	WHD_DoFocus,
	WHD_Focus,
	WHD_DoDef,
	WHD_DefMin,
	WHD_DefWindow,
	WHD_DefSlide,
	WHD_DefSlideUD,
	WHD_DoLimit,
	WHD_LimitGain,
	WHD_Skip,
};

SRS_Param gWOWHD_Params[] = {
	{ WHD_IGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"wowhd_igain", "Input Gain", "gain", "", 0},
	{ WHD_OGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"wowhd_ogain", "Output Gain", "gain", "", 0},
	{ WHD_BGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"wowhd_bgain", "Bypass Gain", "gain", "", 0},
	{ WHD_DoTB,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"wowhd_trubass_enable", "TruBass Toggle", "toggle", "", 0},
	{ WHD_TBMin,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.0f,	0.0f,	1.0f,	"wowhd_trubass_min", "TruBass Min", "control", "", 0},
	{ WHD_TBWindow,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"wowhd_trubass_window", "TruBass Window", "control", "", 0},
	{ WHD_TBSlide,		SRS_PTYP_PREF,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"wowhd_trubass_slide", "TruBass Slider", "control", "", 0},
	{ WHD_TBSlideUD,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.6f,	0.0f,	1.0f,	"wowhd_trubass_slide_udef", "TruBass Slider - User Default", "control", "", 0},
	{ WHD_TBCompress,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.25f,	0.0f,	1.0f,	"wowhd_trubass_compressor", "TruBass Compressor", "control", "", 0},
	{ WHD_TBFreq,		SRS_PTYP_ALIAS,	SRS_PFMT_INT,		400.0f,	40.0f,	1200.0f,	"wowhd_trubass_size", "TruBass Speaker Size", "frequency", "", 0},
	{ WHD_TBFreq,		SRS_PTYP_CFG,	SRS_PFMT_INT,		400.0f,	40.0f,	1200.0f,	"wowhd_trubass_freq", "TruBass Frequency", "frequency", "", 0},	
	{ WHD_TBAnalyze,	SRS_PTYP_CFG,	SRS_PFMT_INT,		200.0f,	40.0f,	1200.0f,	"wowhd_trubass_analysis", "TruBass Analysis Size", "frequency", "", 0},
	{ WHD_TBDoSplit,	SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	0.0f,	"wowhd_trubass_sa_enable", "TruBass Split Analysis Toggle", "toggle", "", 0},
	{ WHD_TBMode,		SRS_PTYP_CFG,	SRS_PFMT_ENUM,		0.0f,	0.0f,	0.0f,	"wowhd_trubass_mode", "TruBass Mode", "enum", "Mono,Stereo", 0},
	{ WHD_DoSRS3D,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"wowhd_srs_enable", "SRS3D Toggle", "toggle", "", 0},
	{ WHD_SRSType,		SRS_PTYP_CFG,	SRS_PFMT_ENUM,		0.0f,	0.0f,	0.0f,	"wowhd_srs_speaker", "SRS3D Speaker Type", "enum", "Speaker,Headphone", 0},
	{ WHD_SRSMode,		SRS_PTYP_CFG,	SRS_PFMT_ENUM,		2.0f,	0.0f,	0.0f,	"wowhd_srs_mode", "SRS3D Mode", "enum", "Mono,Single Speaker,Stereo,Extreme", 0},
	{ WHD_Center,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.5f,	0.0f,	1.0f,	"wowhd_center", "Center", "control", "", 0},
	{ WHD_Space,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.9f,	0.0f,	1.0f,	"wowhd_space", "Space", "control", "", 0},
	{ WHD_DoFocus,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"wowhd_focus_enable", "Focus Toggle", "toggle", "", 0},
	{ WHD_Focus,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.4f,	0.0f,	1.0f,	"wowhd_focus", "Focus", "control", "", 0},
	{ WHD_DoDef,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"wowhd_definition_enable", "Definition Toggle", "toggle", "", 0},
	{ WHD_DefMin,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.0f,	0.0f,	1.0f,	"wowhd_definition_min", "Definition Min", "control", "", 0},
	{ WHD_DefWindow,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"wowhd_definition_window", "Definition Window", "control", "", 0},
	{ WHD_DefSlide,		SRS_PTYP_PREF,	SRS_PFMT_FLOAT,		0.43f,	0.0f,	1.0f,	"wowhd_definition_slide", "Definition Slide", "control", "", 0},
	{ WHD_DefSlideUD,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.43f,	0.0f,	1.0f,	"wowhd_definition_slide_udef", "Definition Slide - User Default", "control", "", 0},
	{ WHD_DoLimit,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		1.0f,	0.0f,	1.0f,	"wowhd_limiter_enable", "Limiter Toggle", "toggle", "", 0},
	{ WHD_LimitGain,	SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.3f,	0.0f,	1.0f,	"wowhd_limiter_gain", "Limiter Minimum Gain", "control", "", 0},
	{ WHD_Skip,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"wowhd_skip", "Skips WOWHD when true", "toggle", "", 0},
};

SRS_Param* SRS_GetBank_WOWHD(int& paramCount){
	paramCount = sizeof(gWOWHD_Params)/sizeof(SRS_Param);
	return gWOWHD_Params;
}

char* SRS_GetVersion_WOWHD(char* pWork, size_t maxBytes){
	int v1 = SRS_GetWowhdTechVersion(SRS_VERSION_MAJOR);
	int v2 = SRS_GetWowhdTechVersion(SRS_VERSION_MINOR);
	int v3 = SRS_GetWowhdTechVersion(SRS_VERSION_REVISION);
	int v4 = SRS_GetWowhdTechVersion(SRS_VERSION_RELEASE);
	int l1 = SRS_GetWowhdLibVersion(SRS_VERSION_MAJOR);
	int l2 = SRS_GetWowhdLibVersion(SRS_VERSION_MINOR);
	int l3 = SRS_GetWowhdLibVersion(SRS_VERSION_REVISION);
	int l4 = SRS_GetWowhdLibVersion(SRS_VERSION_RELEASE);
	snprintf(pWork, maxBytes, "%d.%d.%d.%d - lib %d.%d.%d.%d", v1, v2, v3, v4, l1, l2, l3, l4);
	return pWork;
}

void SRS_SetParam_WOWHD(SRS_Tech_WOWHD* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case WHD_IGain: pCFG->IGain = In.GetFloat(pValue); break;
		case WHD_OGain: pCFG->OGain = In.GetFloat(pValue); break;
		case WHD_BGain: pCFG->BGain = In.GetFloat(pValue); break;
		case WHD_DoTB: pCFG->DoTB = In.GetBool(pValue); break;
		case WHD_TBMin: pCFG->TBMin = In.GetFloat(pValue); break;
		case WHD_TBWindow: pCFG->TBWindow = In.GetFloat(pValue); break;
		case WHD_TBSlide: pCFG->TBSlide = In.GetFloat(pValue); break;
		case WHD_TBSlideUD: pCFG->TBSlideUDef = In.GetFloat(pValue); break;
		case WHD_TBCompress: pCFG->TBCompress = In.GetFloat(pValue); break;
		case WHD_TBFreq: pCFG->TBFreq = In.GetInt(pValue); break;
		case WHD_TBAnalyze: pCFG->TBAnalyze = In.GetInt(pValue); break;
		case WHD_TBDoSplit: pCFG->DoTBSplit = In.GetBool(pValue); break;
		case WHD_TBMode: pCFG->TBMode = In.GetInt(pValue); break;
		case WHD_DoSRS3D: pCFG->DoSRS3D = In.GetBool(pValue); break;
		case WHD_SRSType: pCFG->SRSType = In.GetInt(pValue); break;
		case WHD_SRSMode: pCFG->SRSMode = In.GetInt(pValue); break;
		case WHD_Center: pCFG->Center = In.GetFloat(pValue); break;
		case WHD_Space: pCFG->Space = In.GetFloat(pValue); break;
		case WHD_DoFocus: pCFG->DoFocus = In.GetBool(pValue); break;
		case WHD_Focus: pCFG->Focus = In.GetFloat(pValue); break;
		case WHD_DoDef: pCFG->DoDef = In.GetBool(pValue); break;
		case WHD_DefMin: pCFG->DefMin = In.GetFloat(pValue); break;
		case WHD_DefWindow: pCFG->DefWindow = In.GetFloat(pValue); break;
		case WHD_DefSlide: pCFG->DefSlide = In.GetFloat(pValue); break;
		case WHD_DefSlideUD: pCFG->DefSlideUDef = In.GetFloat(pValue); break;
		case WHD_DoLimit: pCFG->DoLimit = In.GetBool(pValue); break;
		case WHD_LimitGain: pCFG->LimitGain = In.GetFloat(pValue); break;
		case WHD_Skip: pCFG->Skip = In.GetBool(pValue); break;
	}
}

const char* SRS_GetParam_WOWHD(SRS_Tech_WOWHD* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case WHD_IGain: return Out.FromFloat(pCFG->IGain);
		case WHD_OGain: return Out.FromFloat(pCFG->OGain);
		case WHD_BGain: return Out.FromFloat(pCFG->BGain);
		case WHD_DoTB: return Out.FromBool(pCFG->DoTB);
		case WHD_TBMin: return Out.FromFloat(pCFG->TBMin);
		case WHD_TBWindow: return Out.FromFloat(pCFG->TBWindow);
		case WHD_TBSlide: return Out.FromFloat(pCFG->TBSlide);
		case WHD_TBSlideUD: return Out.FromFloat(pCFG->TBSlideUDef);
		case WHD_TBCompress: return Out.FromFloat(pCFG->TBCompress);
		case WHD_TBFreq: return Out.FromInt(pCFG->TBFreq);
		case WHD_TBAnalyze: return Out.FromInt(pCFG->TBAnalyze);
		case WHD_TBDoSplit: return Out.FromBool(pCFG->DoTBSplit);
		case WHD_TBMode: return Out.FromInt(pCFG->TBMode);
		case WHD_DoSRS3D: return Out.FromBool(pCFG->DoSRS3D);
		case WHD_SRSType: return Out.FromInt(pCFG->SRSType);
		case WHD_SRSMode: return Out.FromInt(pCFG->SRSMode);
		case WHD_Center: return Out.FromFloat(pCFG->Center);
		case WHD_Space: return Out.FromFloat(pCFG->Space);
		case WHD_DoFocus: return Out.FromBool(pCFG->DoFocus);
		case WHD_Focus: return Out.FromFloat(pCFG->Focus);
		case WHD_DoDef: return Out.FromBool(pCFG->DoDef);
		case WHD_DefMin: return Out.FromFloat(pCFG->DefMin);
		case WHD_DefWindow: return Out.FromFloat(pCFG->DefWindow);
		case WHD_DefSlide: return Out.FromFloat(pCFG->DefSlide);
		case WHD_DefSlideUD: return Out.FromFloat(pCFG->DefSlideUDef);
		case WHD_DoLimit: return Out.FromBool(pCFG->DoLimit);
		case WHD_LimitGain: return Out.FromFloat(pCFG->LimitGain);
		case WHD_Skip: return Out.FromBool(pCFG->Skip);
	}
	return "";
}

extern void SRS_Default_WOWHD(SRS_Tech_WOWHD* pCFG, const char* pBankName){
	pCFG->IGain = 0.6f;
	pCFG->OGain = 1.0f;
	pCFG->BGain = 1.0f;
	pCFG->DoTB = true;
	pCFG->TBMin = 0.0f;
	pCFG->TBWindow = 1.0f;
	pCFG->TBSlideUDef = pCFG->TBSlide = 0.5f;
	pCFG->TBCompress = 0.2f;
	pCFG->TBFreq = 400;
	pCFG->TBAnalyze = 200;
	pCFG->DoTBSplit = true;
	pCFG->TBMode = 1;
	pCFG->DoSRS3D = true;
	pCFG->SRSType = 0;
	pCFG->SRSMode = 2;
	pCFG->Center = 0.5f;
	pCFG->Space = 0.7f;
	pCFG->DoFocus = true;
	pCFG->Focus = 0.4f;
	pCFG->DoDef = true;
	pCFG->DefMin = 0.0f;
	pCFG->DefWindow = 0.5f;
	pCFG->DefSlideUDef = pCFG->DefSlide = 0.3f;
	pCFG->DoLimit = true;
	pCFG->LimitGain = 0.25f;
	pCFG->Skip = false;
	
	if (pBankName != NULL){
		if (strcmp(pBankName, "srs_mus_int") == 0){
			pCFG->DefWindow = 0.6f;
		}
		
		if (strcmp(pBankName, "srs_mov_int") == 0){
			pCFG->TBSlideUDef = pCFG->TBSlide = 0.6f;
			pCFG->Space = 0.8f;
		}
		
		if (strcmp(pBankName, "srs_pod_int") == 0){
			pCFG->TBSlideUDef = pCFG->TBSlide = 0.4f;
			pCFG->Space = 0.6f;
		}
		
		if (strcmp(pBankName, "srs_mus_ext") == 0){
			pCFG->TBSlideUDef = pCFG->TBSlide = 0.4f;
			pCFG->TBFreq = 80;
			pCFG->TBAnalyze = 60;
			pCFG->Space = 0.8f;
			pCFG->DefWindow = 0.6f;
		}
		
		if (strcmp(pBankName, "srs_pod_ext") == 0){
			pCFG->TBSlideUDef = pCFG->TBSlide = 0.4f;
			pCFG->TBFreq = 110;
			pCFG->TBAnalyze = 80;
			pCFG->Space = 0.8f;
			pCFG->DefWindow = 0.5f;
		}
	}
}

void SRS_UserDefault_WOWHD(SRS_Tech_WOWHD* pCFG){
	pCFG->TBSlide = pCFG->TBSlideUDef;
	pCFG->DefSlide = pCFG->DefSlideUDef;
}

};

