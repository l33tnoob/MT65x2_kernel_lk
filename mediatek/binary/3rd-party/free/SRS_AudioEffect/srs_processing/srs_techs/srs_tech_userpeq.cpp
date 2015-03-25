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

char UPEQScratch[512];

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_UPEQ"

namespace android {

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

enum {
	UPEQ_Enabled,	// CFG
	UPEQ_PresetInt,
	
	UPEQ_IGain,
	UPEQ_OGain,
	UPEQ_BGain,
	UPEQ_Skip,
	
	UPEQ_PresetName,	// Preset
	
	UPEQ_UBand0,		// UserParams
	UPEQ_UBand1,
	UPEQ_UBand2,
	UPEQ_UBand3,
	
	UPEQ_DBand0,		// DefParams
	UPEQ_DBand1,
	UPEQ_DBand2,
	UPEQ_DBand3,
};

SRS_Param gUPEQ_Params_CFG[] = {
	{ UPEQ_Enabled,			SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"upeq_enable", "UserPEQ Toggle", "toggle", "", 0},
	{ UPEQ_PresetInt,		SRS_PTYP_PREF,	SRS_PFMT_INT,		0.0f,	0.0f,	0.0f,	"upeq_int_preset", "UserPEQ Preset - Internal", "The active UPEQ preset", "", 0},
	
	{ UPEQ_IGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	1.0f,	"upeq_igain", "UserPEQ Input Gain", "gain", "", 0},
	{ UPEQ_OGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	4.0f,	"upeq_ogain", "UserPEQ Output Gain", "gain", "", 0},
	{ UPEQ_BGain,			SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		1.0f,	0.0f,	4.0f,	"upeq_bgain", "UserPEQ Bypass Gain", "gain", "", 0},
	
	{ UPEQ_Skip,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"upeq_skip", "Skips UserPEQ when true", "toggle", "", 0},
};

SRS_Param gUPEQ_Params_Preset[] = {	
	{ UPEQ_PresetName,		SRS_PTYP_CFG,	SRS_PFMT_STRING,	0.0f,	0.0f,	0.0f,	"upeq_presetname", "User-facing name of Preset", "", "", 0},
	
	{ UPEQ_UBand0,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_uband0", "UserPEQ Band 0 User Tuning (CF Gain Q)", "", "3", 0},
	{ UPEQ_UBand1,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_uband1", "UserPEQ Band 1", "", "3", 0},
	{ UPEQ_UBand2,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_uband2", "UserPEQ Band 2", "", "3", 0},
	{ UPEQ_UBand3,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_uband3", "UserPEQ Band 3", "", "3", 0},
	
	{ UPEQ_DBand0,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_dband0", "UserPEQ Band 0 Default Tuning (CF Gain Q)", "", "3", 0},
	{ UPEQ_DBand1,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_dband1", "UserPEQ Band 1", "", "3", 0},
	{ UPEQ_DBand2,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_dband2", "UserPEQ Band 2", "", "3", 0},
	{ UPEQ_DBand3,			SRS_PTYP_PREF,	SRS_PFMT_FLOATARRAY,	0.0f,	0.0f,	0.0f,	"upeq_dband3", "UserPEQ Band 3", "", "3", 0},

};

SRS_Param* SRS_GetBank_UserPEQ_CFG(int& paramCount){
	paramCount = sizeof(gUPEQ_Params_CFG)/sizeof(SRS_Param);
	return gUPEQ_Params_CFG;
}

SRS_Param* SRS_GetBank_UserPEQ_Preset(int& paramCount){
	paramCount = sizeof(gUPEQ_Params_Preset)/sizeof(SRS_Param);
	return gUPEQ_Params_Preset;
}

void SRS_SetParam_UserPEQ_CFG(SRS_Tech_UserPEQ_CFG* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case UPEQ_Enabled: pCFG->Enabled = In.GetBool(pValue); break;
		case UPEQ_PresetInt: pCFG->Preset_Int = In.GetInt(pValue); break;
		case UPEQ_IGain: pCFG->IGain = In.GetFloat(pValue); break;
		case UPEQ_OGain: pCFG->OGain = In.GetFloat(pValue); break;
		case UPEQ_BGain: pCFG->BGain = In.GetFloat(pValue); break;
		case UPEQ_Skip: pCFG->Skip = In.GetBool(pValue); break;
	}
}
	
void SRS_SetParam_UserPEQ_Preset(SRS_Tech_UserPEQ_Preset* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case UPEQ_PresetName:{
			if (pCFG->pName != NULL){ delete [] pCFG->pName; pCFG->pName = NULL; }
			int tLen = strlen(pValue);
			if (tLen > 0){
				pCFG->pName = new char[tLen+1];
				strcpy(pCFG->pName, pValue);
			}
		} break;
		
		case UPEQ_UBand0:
		case UPEQ_UBand1:
		case UPEQ_UBand2:
		case UPEQ_UBand3:
		case UPEQ_DBand0:
		case UPEQ_DBand1:
		case UPEQ_DBand2:
		case UPEQ_DBand3:{
			float* pP = NULL;
			if ((pParam->EnumID >= UPEQ_UBand0) && (pParam->EnumID <= UPEQ_UBand3)) pP = pCFG->UserParams[pParam->EnumID-UPEQ_UBand0];
			if ((pParam->EnumID >= UPEQ_DBand0) && (pParam->EnumID <= UPEQ_DBand3)) pP = pCFG->DefParams[pParam->EnumID-UPEQ_DBand0];
			
			if (pP != NULL){			
				float tHold[3];
				float* pD = tHold;
				int i;
			
				for (i=0; i<3; i++) tHold[i] = pP[i];		// Fill with current values...
				if (sscanf(pValue, "%f,%f,%f", pD+0, pD+1, pD+2) == 3){
					for (i=0; i<3; i++) pP[i] = tHold[i];
				}
			}
		} break;
	}
}

const char* SRS_GetParam_UserPEQ_CFG(SRS_Tech_UserPEQ_CFG* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case UPEQ_Enabled: return Out.FromBool(pCFG->Enabled);
		case UPEQ_PresetInt: return Out.FromBool(pCFG->Preset_Int);
		case UPEQ_IGain: return Out.FromFloat(pCFG->IGain);
		case UPEQ_OGain: return Out.FromFloat(pCFG->OGain);
		case UPEQ_BGain: return Out.FromFloat(pCFG->BGain);
		case UPEQ_Skip: return Out.FromBool(pCFG->Skip);
	}
	
	return "";
}

const char* SRS_GetParam_UserPEQ_Preset(SRS_Tech_UserPEQ_Preset* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case UPEQ_PresetName:{
			UPEQScratch[0] = 0;
			if (pCFG->pName != NULL) strncpy(UPEQScratch, pCFG->pName, 64);
			return UPEQScratch;
		} break;
		
		case UPEQ_UBand0:
		case UPEQ_UBand1:
		case UPEQ_UBand2:
		case UPEQ_UBand3:
		case UPEQ_DBand0:
		case UPEQ_DBand1:
		case UPEQ_DBand2:
		case UPEQ_DBand3:{
			float* pP = NULL;
			if ((pParam->EnumID >= UPEQ_UBand0) && (pParam->EnumID <= UPEQ_UBand3)) pP = pCFG->UserParams[pParam->EnumID-UPEQ_UBand0];
			if ((pParam->EnumID >= UPEQ_DBand0) && (pParam->EnumID <= UPEQ_DBand3)) pP = pCFG->DefParams[pParam->EnumID-UPEQ_DBand0];
			
			if (pP != NULL){
				sprintf(UPEQScratch, "%f,%f,%f", pP[0], pP[1], pP[2]);
				return UPEQScratch;
			}
		} break;
	}

	return "";
}

extern void SRS_Default_UserPEQ_CFG(SRS_Tech_UserPEQ_CFG* pCFG){
	pCFG->IGain = 1.0f;
	pCFG->OGain = 1.0f;
	pCFG->BGain = 1.0f;
	
	pCFG->Enabled = false;
	pCFG->Preset_Int = -1;
	
	pCFG->Skip = 0;
}

extern void SRS_Default_UserPEQ_Preset(SRS_Tech_UserPEQ_Preset* pCFG){
	pCFG->pName = NULL;
	memset(pCFG->UserParams, 0, sizeof(pCFG->UserParams));
	memset(pCFG->DefParams, 0, sizeof(pCFG->DefParams));
}

void SRS_Apply_UserPEQ_CFG(SRS_Tech_TruEQ* pTruEQ, SRS_Tech_UserPEQ_CFG* pCFG){
	pTruEQ->IGain = pCFG->IGain;
	pTruEQ->OGain = pCFG->OGain;
	pTruEQ->BGain = pCFG->BGain;
	pTruEQ->Skip = pCFG->Skip;
	
	if (pCFG->Enabled == false){
		pTruEQ->LEnable = false;
		pTruEQ->REnable = false;
	}
}

void SRS_Apply_UserPEQ_Preset(SRS_Tech_TruEQ* pTruEQ, SRS_Tech_UserPEQ_Preset* pCFG){
	bool tActive = false;
	
	int i;
	for (i=0; i<4; i++){
		bool tBActive = false;
		if (pCFG->UserParams[i][1] != 0.0f) tBActive = true;
		if (tBActive == true) tActive = true;
		
		pTruEQ->LBands[i] = tBActive;
		pTruEQ->RBands[i] = tBActive;
		
		pTruEQ->Params[i][0] = pCFG->UserParams[i][0];
		pTruEQ->Params[i][1] = pCFG->UserParams[i][1];
		pTruEQ->Params[i][2] = pCFG->UserParams[i][2];
		pTruEQ->Params[i+4][0] = pCFG->UserParams[i][0];
		pTruEQ->Params[i+4][1] = pCFG->UserParams[i][1];
		pTruEQ->Params[i+4][2] = pCFG->UserParams[i][2];
	}
	
	pTruEQ->LEnable = tActive;
	pTruEQ->REnable = tActive;
}

};

