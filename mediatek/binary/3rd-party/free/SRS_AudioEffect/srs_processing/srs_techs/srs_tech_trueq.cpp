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

#include <media/AudioSystem.h>

#include SRSLIBINC(srs_parametriceq_api.h)
#include SRSLIBINC(srs_parametriceq_design_api.h)
#include SRSLIBINC(srs_common_ver_api.h)

#define TruEQ_BandCount	4
#define TruEQ_FilterWl	SRSFilter32
char TEQScratch[512];

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_TruEQ"

namespace android {

struct SRS_Source_TruEQ {
	void* pPBufferL;	// TruEQ support
	void* pPBufferR;
	SRSParametricEqObj PObjL;
	SRSParametricEqObj PObjR;
    
	int				SampleRate;
	int				ChannelCount;
	
	SRS_Tech_TruEQ	Active;
	bool			ForceActive;

#ifdef _SRSCFG_ARCH_ARM
	SRSCoef32		CoefL[SRS_PEQ_FXP32_COEFFICIENT_ARRAY_LEN(4)];
	SRSCoef32		CoefR[SRS_PEQ_FXP32_COEFFICIENT_ARRAY_LEN(4)];
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	SRSCoef32		CoefL[SRS_PEQ_FLOAT_COEFFICIENT_ARRAY_LEN(4)];
	SRSCoef32		CoefR[SRS_PEQ_FLOAT_COEFFICIENT_ARRAY_LEN(4)];
#endif	// _SRSCFG_ARCH_X86
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_TruEQ();
	~SRS_Source_TruEQ();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_TruEQ* pCFG, bool bBypass);
	void Process_256(SRSSamp* pData);
};

SRS_Source_TruEQ::SRS_Source_TruEQ(){
	SRS_LOG("TruEQ Created");
	
	DidCreate = false;
	DidConfig = false;
	
	int tBufRAM = 0;

#ifdef _SRSCFG_ARCH_ARM
	tBufRAM = SRS_PARAMETRICEQ_OBJ_SIZE(TruEQ_BandCount, TruEQ_FilterWl);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	tBufRAM = SRS_PARAMETRICEQ_OBJ_SIZE(TruEQ_BandCount);
#endif	// _SRSCFG_ARCH_X86

	pPBufferL = malloc(tBufRAM);
	pPBufferR = malloc(tBufRAM);

#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] TruEQ: %d", (tBufRAM*2)+sizeof(SRS_Source_TruEQ));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_TruEQ::~SRS_Source_TruEQ(){
	if (pPBufferL != NULL) free(pPBufferL);
	pPBufferL = NULL;
	if (pPBufferR != NULL) free(pPBufferR);
	pPBufferR = NULL;
	
	DidCreate = false;
	DidConfig = false;
	
	SRS_LOG("TruEQ Destroyed");
}

void SRS_Source_TruEQ::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
	
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;

	DidCreate = true;
	ForceActive = true;
		
	PObjL = SRS_CreateParametricEqObj(pPBufferL);
	PObjR = SRS_CreateParametricEqObj(pPBufferR);
	
	DidConfig = false;
}

void SRS_Source_TruEQ::Config(SRS_Source_Out* pOut, SRS_Tech_TruEQ* pCFG, bool bBypass){
	if (DidCreate == false) return;
		
	int i;
	
	// TruEQ
	bool bCalcL = false;
	for (i=0; i<4; i++){
		if (DIFF_FORCED(Params[i][0]) || DIFF_FORCED(Params[i][1]) || DIFF_FORCED(Params[i][2]) || DIFF_FORCED(LBands[i])){
			bCalcL = true;
		}
	}
	
	bool bCalcR = false;
	for (i=0; i<4; i++){
		if (DIFF_FORCED(Params[i+4][0]) || DIFF_FORCED(Params[i+4][1]) || DIFF_FORCED(Params[i+4][2]) || DIFF_FORCED(RBands[i])){
			bCalcR = true;
		}
	}
	
	bool bMirrorCoefs = false;
	if (bCalcL || bCalcR){
		bool bSame = true;
		
		for (i=0; i<4; i++){
			if (pCFG->Params[i][0] != pCFG->Params[i+4][0]) bSame = false;
			if (pCFG->Params[i][1] != pCFG->Params[i+4][1]) bSame = false;
			if (pCFG->Params[i][2] != pCFG->Params[i+4][2]) bSame = false;
			if (pCFG->LBands[i] != pCFG->RBands[i]) bSame = false;
		}
		
		if (bSame == true){
			bMirrorCoefs = true;
			bCalcL = true;
			bCalcR = false;
		}
	}
	
	if (bCalcL){
		SRSParametriceqBandSpec tBands[4];
		SRSParametriceqSpec tSpec;
		tSpec.SamplingRate = SampleRate;
		tSpec.BandSpecs = tBands;
		tSpec.NumOfBands = 4;
		
		for (i=0; i<4; i++){
			tBands[i].CenterFreq = pCFG->Params[i][0];
			tBands[i].Gain = pCFG->Params[i][1];
			tBands[i].QFactor = pCFG->Params[i][2];
			
			if (tBands[i].CenterFreq < 5.0f) tBands[i].CenterFreq = 5.0f;
			if (tBands[i].CenterFreq > (SampleRate*0.5f)) tBands[i].CenterFreq = SampleRate*0.5f;
			
			if (tBands[i].QFactor < 0.1f) tBands[i].QFactor = 0.1f;
			if (tBands[i].QFactor > 10.0f) tBands[i].QFactor = 10.0f;
			
			if (pCFG->LBands[i] == false) tBands[i].Gain = 0.0f;	// Disabled means disabled...
		}
		
#ifdef _SRSCFG_ARCH_ARM
		char tWork[SRS_PEQ_DESIGN_WORKSPACE_SIZE(4)];
		SRS_ParametriceqDesignFxp32(&tSpec, CoefL, tWork); //Design TruEQ filters	
		SRS_InitParametricEqObj(PObjL, TruEQ_BandCount, (void*)CoefL, TruEQ_FilterWl);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
		SRS_ParametriceqDesignFloat(&tSpec, CoefL); //Design TruEQ filters	
		SRS_InitParametricEqObj(PObjL, TruEQ_BandCount, CoefL);
#endif	// _SRSCFG_ARCH_X86
		
		ForceActive = true;
	}
	
	if (DIFF_FORCED(LBands[0])) SRS_SetParametricEqBandEnable(PObjL, 0, pCFG->LBands[0]);
	if (DIFF_FORCED(LBands[1])) SRS_SetParametricEqBandEnable(PObjL, 1, pCFG->LBands[1]);
	if (DIFF_FORCED(LBands[2])) SRS_SetParametricEqBandEnable(PObjL, 2, pCFG->LBands[2]);
	if (DIFF_FORCED(LBands[3])) SRS_SetParametricEqBandEnable(PObjL, 3, pCFG->LBands[3]);	
	if (DIFF_FORCED(LEnable)) SRS_SetParametricEqMasterEnable(PObjL, pCFG->LEnable);
	
	if (bMirrorCoefs){

#ifdef _SRSCFG_ARCH_ARM
		SRS_InitParametricEqObj(PObjR, TruEQ_BandCount, (void*)CoefL, TruEQ_FilterWl);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
		SRS_InitParametricEqObj(PObjR, TruEQ_BandCount, CoefL);
#endif	// _SRSCFG_ARCH_X86
	
	}
	
	if (bCalcR){
		SRSParametriceqBandSpec tBands[4];
		SRSParametriceqSpec tSpec;
		tSpec.SamplingRate = SampleRate;
		tSpec.BandSpecs = tBands;
		tSpec.NumOfBands = 4;
		
		for (i=0; i<4; i++){
			tBands[i].CenterFreq = pCFG->Params[i+4][0];
			tBands[i].Gain = pCFG->Params[i+4][1];
			tBands[i].QFactor = pCFG->Params[i+4][2];
			
			if (tBands[i].CenterFreq < 5.0f) tBands[i].CenterFreq = 5.0f;
			if (tBands[i].CenterFreq > (SampleRate*0.5f)) tBands[i].CenterFreq = SampleRate*0.5f;
			
			if (tBands[i].QFactor < 0.1f) tBands[i].QFactor = 0.1f;
			if (tBands[i].QFactor > 10.0f) tBands[i].QFactor = 10.0f;
			
			if (pCFG->RBands[i] == false) tBands[i].Gain = 0.0f;	// Disabled means disabled...
		}

#ifdef _SRSCFG_ARCH_ARM
		char tWork[SRS_PEQ_DESIGN_WORKSPACE_SIZE(4)];
		SRS_ParametriceqDesignFxp32(&tSpec, CoefR, tWork); //Design TruEQ filters	
		SRS_InitParametricEqObj(PObjR, TruEQ_BandCount, (void*)CoefR, TruEQ_FilterWl);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
		SRS_ParametriceqDesignFloat(&tSpec, CoefR); //Design TruEQ filters	
		SRS_InitParametricEqObj(PObjR, TruEQ_BandCount, CoefR);
#endif	// _SRSCFG_ARCH_X86

		ForceActive = true;
	}
	
	if (DIFF_FORCED(RBands[0])) SRS_SetParametricEqBandEnable(PObjR, 0, pCFG->RBands[0]);
	if (DIFF_FORCED(RBands[1])) SRS_SetParametricEqBandEnable(PObjR, 1, pCFG->RBands[1]);
	if (DIFF_FORCED(RBands[2])) SRS_SetParametricEqBandEnable(PObjR, 2, pCFG->RBands[2]);
	if (DIFF_FORCED(RBands[3])) SRS_SetParametricEqBandEnable(PObjR, 3, pCFG->RBands[3]);
	if (DIFF_FORCED(REnable)) SRS_SetParametricEqMasterEnable(PObjR, pCFG->REnable);
	
	if (DIFF_FORCED(IGain)){
		SRS_SetParametricEqInputGain(PObjL, SRS_FXP16(pCFG->IGain,SRS_PEQ_GAIN_IWL));
		SRS_SetParametricEqInputGain(PObjR, SRS_FXP16(pCFG->IGain,SRS_PEQ_GAIN_IWL));
	}
	if (DIFF_FORCED(OGain)){
		SRS_SetParametricEqOutputGain(PObjL, SRS_FXP16(pCFG->OGain,SRS_PEQ_GAIN_IWL));
		SRS_SetParametricEqOutputGain(PObjR, SRS_FXP16(pCFG->OGain,SRS_PEQ_GAIN_IWL));
	}
	if (DIFF_FORCED(BGain)){
		SRS_SetParametricEqBypassGain(PObjL, SRS_FXP16(pCFG->BGain,SRS_PEQ_GAIN_IWL));
		SRS_SetParametricEqBypassGain(PObjR, SRS_FXP16(pCFG->BGain,SRS_PEQ_GAIN_IWL));
	}	
			
	DidConfig = true;
	Active = *pCFG;
	
	if (bBypass){
		SRS_SetParametricEqMasterEnable(PObjL, false);
		Active.LEnable = false;
		SRS_SetParametricEqMasterEnable(PObjR, false);
		Active.REnable = false;
	}
	
	ForceActive = false;
}

void SRS_Source_TruEQ::Process_256(SRSSamp* pData){

#ifdef _SRSCFG_ARCH_ARM
	SRS_ParametricEq32(PObjL, pData, 256);
	SRS_ParametricEq32(PObjR, pData+256, 256);
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	SRS_ParametricEq(PObjL, pData, 256);
	SRS_ParametricEq(PObjR, pData+256, 256);
#endif	// _SRSCFG_ARCH_X86

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_TruEQ* SRS_Create_TruEQ(SRS_Source_Out* pOut){	
	SRS_Source_TruEQ* pSrc = new SRS_Source_TruEQ();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_TruEQ(SRS_Source_TruEQ* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_TruEQ(SRS_Source_TruEQ* pSrc, SRS_Source_Out* pOut, SRS_Tech_TruEQ* pCFG, bool bBypass){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bBypass);
}

void SRS_Process_TruEQ_256(SRS_Source_TruEQ* pSrc, SRSSamp* pData){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pData);
}

enum {
	TEQ_IGain,
	TEQ_OGain,
	TEQ_BGain,
	TEQ_L_Enable,
	TEQ_R_Enable,
	TEQ_LB0,
	TEQ_LB1,
	TEQ_LB2,
	TEQ_LB3,
	TEQ_RB0,
	TEQ_RB1,
	TEQ_RB2,
	TEQ_RB3,
	TEQ_Def_LBand0,
	TEQ_Def_LBand1,
	TEQ_Def_LBand2,
	TEQ_Def_LBand3,
	TEQ_Def_RBand0,
	TEQ_Def_RBand1,
	TEQ_Def_RBand2,
	TEQ_Def_RBand3,
	TEQ_Skip,
};

SRS_Param gTruEQ_Params[] = {
	{ TEQ_IGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"trueq_igain", "TruEQ Input Gain", "gain", "", 0},
	{ TEQ_OGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	4.0f,	"trueq_ogain", "TruEQ Output Gain", "gain", "", 0},
	{ TEQ_BGain,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	4.0f,	"trueq_bgain", "TruEQ Bypass Gain", "gain", "", 0},
	
	{ TEQ_L_Enable,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_left_enable", "TruEQ Left Toggle", "toggle", "", 0},
	{ TEQ_LB0,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_lband0_enable", "TruEQ Left Band 0 Toggle", "toggle", "", 0},
	{ TEQ_LB1,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_lband1_enable", "TruEQ Left Band 1 Toggle", "toggle", "", 0},
	{ TEQ_LB2,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_lband2_enable", "TruEQ Left Band 2 Toggle", "toggle", "", 0},
	{ TEQ_LB3,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_lband3_enable", "TruEQ Left Band 3 Toggle", "toggle", "", 0},

	{ TEQ_R_Enable,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_right_enable", "TruEQ Right Toggle", "toggle", "", 0},
	{ TEQ_RB0,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_rband0_enable", "TruEQ Right Band 0 Toggle", "toggle", "", 0},
	{ TEQ_RB1,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_rband1_enable", "TruEQ Right Band 1 Toggle", "toggle", "", 0},
	{ TEQ_RB2,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_rband2_enable", "TruEQ Right Band 2 Toggle", "toggle", "", 0},
	{ TEQ_RB3,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_rband3_enable", "TruEQ Right Band 3 Toggle", "toggle", "", 0},
	
	{ TEQ_Def_LBand0,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_lband0", "TruEQ Left Band 0 Tuning (CF Gain Q)", "", "3", 0},
	{ TEQ_Def_LBand1,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_lband1", "TruEQ Left Band 1 Tuning", "", "3", 0},
	{ TEQ_Def_LBand2,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_lband2", "TruEQ Left Band 2 Tuning", "", "3", 0},
	{ TEQ_Def_LBand3,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_lband3", "TruEQ Left Band 3 Tuning", "", "3", 0},
	{ TEQ_Def_RBand0,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_rband0", "TruEQ Right Band 0 Tuning", "", "3", 0},
	{ TEQ_Def_RBand1,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_rband1", "TruEQ Right Band 1 Tuning", "", "3", 0},
	{ TEQ_Def_RBand2,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_rband2", "TruEQ Right Band 2 Tuning", "", "3", 0},
	{ TEQ_Def_RBand3,	SRS_PTYP_CFG,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"trueq_rband3", "TruEQ Right Band 3 Tuning", "", "3", 0},
	
	{ TEQ_Skip,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trueq_skip", "Skips TruEQ when true", "toggle", "", 0},	
};

SRS_Param* SRS_GetBank_TruEQ(int& paramCount){
	paramCount = sizeof(gTruEQ_Params)/sizeof(SRS_Param);
	return gTruEQ_Params;
}

char* SRS_GetVersion_TruEQ(char* pWork, size_t maxBytes){
	int v1 = SRS_GetCommonTechVersion(SRS_VERSION_MAJOR);
	int v2 = SRS_GetCommonTechVersion(SRS_VERSION_MINOR);
	int v3 = SRS_GetCommonTechVersion(SRS_VERSION_REVISION);
	int v4 = SRS_GetCommonTechVersion(SRS_VERSION_RELEASE);
	int l1 = SRS_GetCommonLibVersion(SRS_VERSION_MAJOR);
	int l2 = SRS_GetCommonLibVersion(SRS_VERSION_MINOR);
	int l3 = SRS_GetCommonLibVersion(SRS_VERSION_REVISION);
	int l4 = SRS_GetCommonLibVersion(SRS_VERSION_RELEASE);
	snprintf(pWork, maxBytes, "%d.%d.%d.%d - lib %d.%d.%d.%d", v1, v2, v3, v4, l1, l2, l3, l4);
	return pWork;
}

void SRS_SetParam_TruEQ(SRS_Tech_TruEQ* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case TEQ_IGain: pCFG->IGain = In.GetFloat(pValue); break;
		case TEQ_OGain: pCFG->OGain = In.GetFloat(pValue); break;
		case TEQ_BGain: pCFG->BGain = In.GetFloat(pValue); break;
		case TEQ_Skip: pCFG->Skip = In.GetBool(pValue); break;
		
		case TEQ_L_Enable: pCFG->LEnable = In.GetBool(pValue); break;
		case TEQ_R_Enable: pCFG->REnable = In.GetBool(pValue); break;
		
		case TEQ_LB0:
		case TEQ_LB1:
		case TEQ_LB2:
		case TEQ_LB3: { int tIdx = pParam->EnumID-TEQ_LB0; pCFG->LBands[tIdx] = In.GetBool(pValue); } break;
		
		case TEQ_RB0:
		case TEQ_RB1:
		case TEQ_RB2:
		case TEQ_RB3: { int tIdx = pParam->EnumID-TEQ_RB0; pCFG->RBands[tIdx] = In.GetBool(pValue); } break;
				
		case TEQ_Def_LBand0:
		case TEQ_Def_LBand1:
		case TEQ_Def_LBand2:
		case TEQ_Def_LBand3:
		case TEQ_Def_RBand0:
		case TEQ_Def_RBand1:
		case TEQ_Def_RBand2:
		case TEQ_Def_RBand3:
		{
			int tIdx = pParam->EnumID-TEQ_Def_LBand0;
			float tHold[3];
			float* pD = tHold;
			int i;
			
			float* pS = pCFG->Params[tIdx];
			for (i=0; i<3; i++) tHold[i] = pS[i];		// Fill with current values...
			if (sscanf(pValue, "%f,%f,%f", pD+0, pD+1, pD+2) == 3){
				for (i=0; i<3; i++) pS[i] = tHold[i];
			}
		} break;		
	}
}

const char* SRS_GetParam_TruEQ(SRS_Tech_TruEQ* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case TEQ_IGain: return Out.FromFloat(pCFG->IGain);
		case TEQ_OGain: return Out.FromFloat(pCFG->OGain);
		case TEQ_BGain: return Out.FromFloat(pCFG->BGain);
		case TEQ_Skip: return Out.FromBool(pCFG->Skip);
		
		case TEQ_L_Enable: return Out.FromBool(pCFG->LEnable);
		case TEQ_R_Enable: return Out.FromBool(pCFG->REnable);
		
		case TEQ_LB0:
		case TEQ_LB1:
		case TEQ_LB2:
		case TEQ_LB3: { int tIdx = pParam->EnumID-TEQ_LB0; return Out.FromBool(pCFG->LBands[tIdx]); } break;
		
		case TEQ_RB0:
		case TEQ_RB1:
		case TEQ_RB2:
		case TEQ_RB3: { int tIdx = pParam->EnumID-TEQ_RB0; return Out.FromBool(pCFG->RBands[tIdx]); } break;
				
		case TEQ_Def_LBand0:
		case TEQ_Def_LBand1:
		case TEQ_Def_LBand2:
		case TEQ_Def_LBand3:
		case TEQ_Def_RBand0:
		case TEQ_Def_RBand1:
		case TEQ_Def_RBand2:
		case TEQ_Def_RBand3:
		{
			int tIdx = pParam->EnumID-TEQ_Def_LBand0;
			float* pS = pCFG->Params[tIdx];
			
			snprintf(TEQScratch, sizeof(TEQScratch), "%f,%f,%f", pS[0], pS[1], pS[2]);
			return TEQScratch;
		} break;
	}

	return "";
}

void SRS_Default_TruEQ(SRS_Tech_TruEQ* pCFG){
	pCFG->IGain = 1.0f;
	pCFG->OGain = 1.0f;
	pCFG->BGain = 1.0f;
	pCFG->Skip = false;
	
	pCFG->LEnable = false;
	pCFG->REnable = false;
	memset(pCFG->LBands, 0, sizeof(pCFG->LBands));
	memset(pCFG->RBands, 0, sizeof(pCFG->RBands));
	
	memset(pCFG->Params, 0, sizeof(pCFG->Params));
	
	for (int i=0; i<8; i++) pCFG->Params[i][2] = 2.0f;
}

};

