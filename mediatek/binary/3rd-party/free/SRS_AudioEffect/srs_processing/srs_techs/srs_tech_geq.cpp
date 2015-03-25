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

#include SRSLIBINC(srs_10band_geq_api.h)
#include SRSLIBINC(srs_math_api.h)

bool gSRS_GEQGainMapInit = false;
int16_t gSRS_GEQGainMap[31] = { 0 };
char GEQScratch[512];

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_GEQ"

namespace android {

struct SRS_Source_GEQ {	
	char GEQBufL[SRS_10BAND_GRAPHICEQ_OBJ_SIZE]; //buffer for creating left PEQ core
    char GEQBufR[SRS_10BAND_GRAPHICEQ_OBJ_SIZE]; //buffer for creating right PEQ core

    int32_t GEQWorkspace[4096];

    SRS10BandGraphicEqObj GEQL, GEQR;
    
    int				SampleRate;
	int				ChannelCount;
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_GEQ();
	~SRS_Source_GEQ();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_GEQ* pCFG, bool bLimit);
	void Process_256(int32_t* pData);
};

SRS_Source_GEQ::SRS_Source_GEQ(){
	SRS_LOG("GEQ Created");
	DidCreate = false;
	
	if (gSRS_GEQGainMapInit == false){
		gSRS_GEQGainMap[0] = SRS_GEQ_MINUS_15DB;
		gSRS_GEQGainMap[1] = SRS_GEQ_MINUS_14DB;
		gSRS_GEQGainMap[2] = SRS_GEQ_MINUS_13DB;
		gSRS_GEQGainMap[3] = SRS_GEQ_MINUS_12DB;
		gSRS_GEQGainMap[4] = SRS_GEQ_MINUS_11DB;
		gSRS_GEQGainMap[5] = SRS_GEQ_MINUS_10DB;
		gSRS_GEQGainMap[6] = SRS_GEQ_MINUS_09DB;
		gSRS_GEQGainMap[7] = SRS_GEQ_MINUS_08DB;
		gSRS_GEQGainMap[8] = SRS_GEQ_MINUS_07DB;
		gSRS_GEQGainMap[9] = SRS_GEQ_MINUS_06DB;
		gSRS_GEQGainMap[10] = SRS_GEQ_MINUS_05DB;
		gSRS_GEQGainMap[11] = SRS_GEQ_MINUS_04DB;
		gSRS_GEQGainMap[12] = SRS_GEQ_MINUS_03DB;
		gSRS_GEQGainMap[13] = SRS_GEQ_MINUS_02DB;
		gSRS_GEQGainMap[14] = SRS_GEQ_MINUS_01DB;
		gSRS_GEQGainMap[15] = SRS_GEQ_0DB;
		gSRS_GEQGainMap[16] = SRS_GEQ_PLUS_01DB;
		gSRS_GEQGainMap[17] = SRS_GEQ_PLUS_02DB;
		gSRS_GEQGainMap[18] = SRS_GEQ_PLUS_03DB;
		gSRS_GEQGainMap[19] = SRS_GEQ_PLUS_04DB;
		gSRS_GEQGainMap[20] = SRS_GEQ_PLUS_05DB;
		gSRS_GEQGainMap[21] = SRS_GEQ_PLUS_06DB;
		gSRS_GEQGainMap[22] = SRS_GEQ_PLUS_07DB;
		gSRS_GEQGainMap[23] = SRS_GEQ_PLUS_08DB;
		gSRS_GEQGainMap[24] = SRS_GEQ_PLUS_09DB;
		gSRS_GEQGainMap[25] = SRS_GEQ_PLUS_10DB;
		gSRS_GEQGainMap[26] = SRS_GEQ_PLUS_11DB;
		gSRS_GEQGainMap[27] = SRS_GEQ_PLUS_12DB;
		gSRS_GEQGainMap[28] = SRS_GEQ_PLUS_13DB;
		gSRS_GEQGainMap[29] = SRS_GEQ_PLUS_14DB;
		gSRS_GEQGainMap[30] = SRS_GEQ_PLUS_15DB;
		gSRS_GEQGainMapInit = true;
	}
	
#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] GEQ: %d", sizeof(SRS_Source_GEQ));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_GEQ::~SRS_Source_GEQ(){
	DidCreate = false;
	
	SRS_LOG("GEQ Destroyed");
}

void SRS_Source_GEQ::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
	
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;

	DidCreate = true;

	GEQL = SRS_Create10BandGraphicEqObj(GEQBufL);
    GEQR = SRS_Create10BandGraphicEqObj(GEQBufR);
    
    if (SampleRate < 9500){ SRS_Init10BandGraphicEqObj8k(GEQL); SRS_Init10BandGraphicEqObj8k(GEQR); }
    else if (SampleRate < 13500){ SRS_Init10BandGraphicEqObj11k(GEQL); SRS_Init10BandGraphicEqObj11k(GEQR); }
	else if (SampleRate < 19000){ SRS_Init10BandGraphicEqObj16k(GEQL); SRS_Init10BandGraphicEqObj16k(GEQR); }
	else if (SampleRate < 23000){ SRS_Init10BandGraphicEqObj22k(GEQL); SRS_Init10BandGraphicEqObj22k(GEQR); }
	else if (SampleRate < 28000){ SRS_Init10BandGraphicEqObj24k(GEQL); SRS_Init10BandGraphicEqObj24k(GEQR); }
	else if (SampleRate < 38000){ SRS_Init10BandGraphicEqObj32k(GEQL); SRS_Init10BandGraphicEqObj32k(GEQR); }
	else if (SampleRate < 46000){ SRS_Init10BandGraphicEqObj44k(GEQL); SRS_Init10BandGraphicEqObj44k(GEQR); }
	else { SRS_Init10BandGraphicEqObj48k(GEQL); SRS_Init10BandGraphicEqObj48k(GEQR); }
	
	DidConfig = false;
}

void SRS_Source_GEQ::Config(SRS_Source_Out* pOut, SRS_Tech_GEQ* pCFG, bool bLimit){
	if (DidCreate == false) return;
		
	SRS_Set10BandGraphicEqEnable(GEQL, 1);
	SRS_Set10BandGraphicEqEnable(GEQR, 1);
	
	SRS_Set10BandGraphicEqLimiterEnable(GEQL, bLimit);
	SRS_Set10BandGraphicEqLimiterEnable(GEQR, bLimit);
	
	int i;
	for (i=0; i<10; i++){
		SRS_Set10BandGraphicEqBandGain(GEQL, i, gSRS_GEQGainMap[pCFG->Users[i]+15]);
		SRS_Set10BandGraphicEqBandGain(GEQR, i, gSRS_GEQGainMap[pCFG->Users[i]+15]);
	}
	
	DidConfig = true;
}

void SRS_Source_GEQ::Process_256(int32_t* pData){
	SRS_10BandGraphicEqProcess(GEQL, pData, 256, GEQWorkspace);
	SRS_10BandGraphicEqProcess(GEQR, pData+256, 256, GEQWorkspace);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_GEQ* SRS_Create_GEQ(SRS_Source_Out* pOut){	
	SRS_Source_GEQ* pSrc = new SRS_Source_GEQ();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_GEQ(SRS_Source_GEQ* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_GEQ(SRS_Source_GEQ* pSrc, SRS_Source_Out* pOut, SRS_Tech_GEQ* pCFG, bool bLimit){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bLimit);
}

void SRS_Process_GEQ_256(SRS_Source_GEQ* pSrc, int32_t* pData){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pData);
}

enum {
	GEQ_PresetName,
	GEQ_DefaultGains,
	GEQ_UserGains,
};

SRS_Param gGEQ_Params[] = {
	{ GEQ_PresetName,		SRS_PTYP_CFG,	SRS_PFMT_STRING,	0.0f,	0.0f,	0.0f,	"geq_presetname", "User-facing name of Preset", "", "", 0},
	{ GEQ_DefaultGains,		SRS_PTYP_CFG,	SRS_PFMT_INTARRAY,	0.0f,	-15.0f,	15.0f,	"geq_defgains", "Default gains of Preset", "", "10", 0},
	{ GEQ_UserGains,		SRS_PTYP_CFG,	SRS_PFMT_INTARRAY,	0.0f,	-15.0f,	15.0f,	"geq_usergains", "User gains of Preset", "", "10", 0},
};

SRS_Param* SRS_GetBank_GEQ(int& paramCount){
	paramCount = sizeof(gGEQ_Params)/sizeof(SRS_Param);
	return gGEQ_Params;
}

char* SRS_GetVersion_GEQ(char* pWork, size_t maxBytes){
	int v1 = SRS_GetGraphicEQTechVersion(SRS_VERSION_MAJOR);
	int v2 = SRS_GetGraphicEQTechVersion(SRS_VERSION_MINOR);
	int v3 = SRS_GetGraphicEQTechVersion(SRS_VERSION_REVISION);
	int v4 = SRS_GetGraphicEQTechVersion(SRS_VERSION_RELEASE);
	int l1 = SRS_GetGraphicEQLibVersion(SRS_VERSION_MAJOR);
	int l2 = SRS_GetGraphicEQLibVersion(SRS_VERSION_MINOR);
	int l3 = SRS_GetGraphicEQLibVersion(SRS_VERSION_REVISION);
	int l4 = SRS_GetGraphicEQLibVersion(SRS_VERSION_RELEASE);
	snprintf(pWork, maxBytes, "%d.%d.%d.%d - lib %d.%d.%d.%d", v1, v2, v3, v4, l1, l2, l3, l4);
	return pWork;
}

void SRS_SetParam_GEQ(SRS_Tech_GEQ* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case GEQ_PresetName:{
			if (pCFG->pName != NULL){ delete [] pCFG->pName; pCFG->pName = NULL; }
			int tLen = strlen(pValue);
			if (tLen > 0){
				if (tLen > 63) tLen = 63;
				pCFG->pName = new char[tLen+1];
				
				if (pCFG->pName != NULL)
					strlcpy(pCFG->pName, pValue, tLen+1);
			}
		} break;
		case GEQ_UserGains:
		case GEQ_DefaultGains:{
			int tHold[10];
			int* pD = tHold;
			int i;
			
			int16_t* pS = pCFG->Defs;
			if (pParam->EnumID == GEQ_UserGains) pS = pCFG->Users;
			
			for (i=0; i<10; i++) tHold[i] = pS[i];		// Fill with current values...
			if (sscanf(pValue, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pD+0, pD+1, pD+2, pD+3, pD+4, pD+5, pD+6, pD+7, pD+8, pD+9) == 10){
				for (i=0; i<10; i++){
					if (tHold[i] < -15) pS[i] = -15;
					else if (tHold[i] > 15) pS[i] = 15;
					else pS[i] = tHold[i];
				}
			}
		} break;
	}
}

const char* SRS_GetParam_GEQ(SRS_Tech_GEQ* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case GEQ_PresetName:{
			GEQScratch[0] = 0;
			if (pCFG->pName != NULL) strlcpy(GEQScratch, pCFG->pName, sizeof(GEQScratch));
			return GEQScratch;
		} break;
		case GEQ_DefaultGains:{
			int16_t* pD = pCFG->Defs;
			snprintf(GEQScratch, sizeof(GEQScratch), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pD[0], pD[1], pD[2], pD[3], pD[4], pD[5], pD[6], pD[7], pD[8], pD[9]);
			return GEQScratch; 
		} break;
		case GEQ_UserGains:{
			int16_t* pD = pCFG->Users;
			snprintf(GEQScratch, sizeof(GEQScratch), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", pD[0], pD[1], pD[2], pD[3], pD[4], pD[5], pD[6], pD[7], pD[8], pD[9]);
			return GEQScratch; 
		} break;
	}

	return "";
}

extern void SRS_Default_GEQ(SRS_Tech_GEQ* pCFG){
	pCFG->pName = NULL;
	memset(pCFG->Defs, 0, sizeof(pCFG->Defs));
	memset(pCFG->Users, 0, sizeof(pCFG->Users));
}

};

