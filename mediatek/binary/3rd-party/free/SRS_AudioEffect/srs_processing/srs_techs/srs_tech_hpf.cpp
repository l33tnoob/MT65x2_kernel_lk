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

#include SRSLIBINC(srs_iir_api.h)
#include SRSLIBINC(srs_common_ver_api.h)

char HPFScratch[1024];
char HPFScratch2[1024];

//#define SRS_XOVERMODE

#undef LOG_TAG
#define LOG_TAG "SRS_Tech_HiPass"

namespace android {

struct SRS_Source_HiPass {
	SRSIirObj ObjL;
	SRSIirObj ObjR;
	
	void* pObjLBuf;
	void* pObjRBuf;
	
	srs_int32	IIR_Coefs_32bits[32];
	
#ifdef SRS_XOVERMODE
	// Temp XOver test
	SRSIirObj ObjL_L;
	SRSIirObj ObjR_L;
	
	void* pObjLBuf_L;
	void* pObjRBuf_L;
	
	srs_int32	IIR_Coefs_32bits_L[32];
#endif
		
	SRS_Tech_HiPass Active;
	bool			ForceActive;
	
	int				SampleRate;
	int				ChannelCount;
	
	bool			DidCreate;
	bool			DidConfig;
	
	SRS_Source_HiPass();
	~SRS_Source_HiPass();
	
	void Create(SRS_Source_Out* pOut);
	void Config(SRS_Source_Out* pOut, SRS_Tech_HiPass* pCFG, bool bBypass);
	void Process_256(int32_t* pData);
};

SRS_Source_HiPass::SRS_Source_HiPass(){
	SRS_LOG("HiPass Created");
	
	DidCreate = false;
	DidConfig = false;
	
	pObjLBuf = malloc(SRS_IIR_OBJ_SIZE(8));	// 8th order max...
	pObjRBuf = malloc(SRS_IIR_OBJ_SIZE(8));	// 8th order max...
	
#ifdef SRS_XOVERMODE
	pObjLBuf_L = malloc(SRS_IIR_OBJ_SIZE(8));	// 8th order max...
	pObjRBuf_L = malloc(SRS_IIR_OBJ_SIZE(8));	// 8th order max..
#endif

#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] HPF: %d", SRS_IIR_OBJ_SIZE(8)+sizeof(SRS_Source_HiPass));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_HiPass::~SRS_Source_HiPass(){
	if (pObjLBuf != NULL) free(pObjLBuf);
	pObjLBuf = NULL;
	if (pObjRBuf != NULL) free(pObjRBuf);
	pObjRBuf = NULL;
	
#ifdef SRS_XOVERMODE
	if (pObjLBuf_L != NULL) free(pObjLBuf_L);
	pObjLBuf_L = NULL;
	if (pObjRBuf != NULL) free(pObjRBuf_L);
	pObjRBuf_L = NULL;
#endif
	
	DidCreate = false;
	DidConfig = false;
	
	SRS_LOG("HiPass Destroyed");
}

void SRS_Source_HiPass::Create(SRS_Source_Out* pOut){
	if (pOut->SampleRate <= 0) return;
	if (pOut->ChannelCount != 2) return;
	
	SampleRate = pOut->SampleRate;
	ChannelCount = pOut->ChannelCount;
	
	DidCreate = true;
	
	// We don't build objects here - forced or changed filters will cause it...
	
	DidConfig = false;
}

void SRS_Source_HiPass::Config(SRS_Source_Out* pOut, SRS_Tech_HiPass* pCFG, bool bBypass){
	if (DidCreate == false) return;
	
	if (DIFF_FORCED(Is32Bit)){	// Must rebuild!
		ForceActive = true;
	}
	
	if (DIFF_FORCED(Order) || DIFF_FORCED(Frequency)){
		int tXOverDelta = 0;
		
#ifdef SRS_XOVERMODE
		tXOverDelta = pCFG->Frequency/(2*pCFG->Order-1);

		Tool_GenLoPassCoefs(IIR_Coefs_32bits_L, pCFG->Order, pCFG->Frequency-tXOverDelta, SampleRate, pCFG->Is32Bit);
		
		ObjL_L = SRS_CreateIirObj(pObjLBuf_L, pCFG->Order, IIR_Coefs_32bits_L, pCFG->Is32Bit?SRSFilter32:SRSFilter16);
		ObjR_L = SRS_CreateIirObj(pObjRBuf_L, pCFG->Order, IIR_Coefs_32bits_L, pCFG->Is32Bit?SRSFilter32:SRSFilter16);
	
		SRS_InitIirObj(ObjL_L);
		SRS_InitIirObj(ObjR_L);
#endif

		Tool_GenHiPassCoefs(IIR_Coefs_32bits, pCFG->Order, pCFG->Frequency+tXOverDelta, SampleRate, pCFG->Is32Bit);
		
		ObjL = SRS_CreateIirObj(pObjLBuf, pCFG->Order, IIR_Coefs_32bits, pCFG->Is32Bit?SRSFilter32:SRSFilter16);
		ObjR = SRS_CreateIirObj(pObjRBuf, pCFG->Order, IIR_Coefs_32bits, pCFG->Is32Bit?SRSFilter32:SRSFilter16);
		
		SRS_InitIirObj(ObjL);
		SRS_InitIirObj(ObjR);
	}
	
	if (bBypass){ SRS_SetIirEnable(ObjL, false); SRS_SetIirEnable(ObjR, false); }
	else { SRS_SetIirEnable(ObjL, true); SRS_SetIirEnable(ObjR, true); }
	
	DidConfig = true;
	Active = *pCFG;
	ForceActive = false;
}

void SRS_Source_HiPass::Process_256(int32_t* pData){
#ifdef SRS_XOVERMODE
	int32_t tSrc[512];
	memcpy(tSrc, pData, 512*sizeof(int32_t));
	
	SRS_IirProcess(ObjL_L, tSrc, 256);
	SRS_IirProcess(ObjR_L, tSrc+256, 256);
#endif
	
	SRS_IirProcess(ObjL, pData, 256);
	SRS_IirProcess(ObjR, pData+256, 256);
	
#ifdef SRS_XOVERMODE
	for (int i=0; i<512; i++){
		pData[i] += tSrc[i];
	}
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// =-=-=-=-=-=- External Interfacing =-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SRS_Source_HiPass* SRS_Create_HiPass(SRS_Source_Out* pOut){	
	SRS_Source_HiPass* pSrc = new SRS_Source_HiPass();
	pSrc->Create(pOut);
	return pSrc;
}

void SRS_Destroy_HiPass(SRS_Source_HiPass* pSrc, SRS_Source_Out* pOut){
	if (pSrc == NULL) return;
	delete pSrc;
}

void SRS_Config_HiPass(SRS_Source_HiPass* pSrc, SRS_Source_Out* pOut, SRS_Tech_HiPass* pCFG, bool bBypass){
	if (pSrc == NULL) return;
	pSrc->Config(pOut, pCFG, bBypass);
}

void SRS_Process_HiPass_256(SRS_Source_HiPass* pSrc, int32_t* pData){
	if (pSrc == NULL) return;
	if (pSrc->DidConfig == false) return;
	
	pSrc->Process_256(pData);
}

enum {
	HPF_Skip,
	
	HPF_Is32Bit,
	
	HPF_Order,
	HPF_Freq,
};

SRS_Param gHiPass_Params[] = {
	{ HPF_Order,			SRS_PTYP_CFG,	SRS_PFMT_INT,			6.0f,	1.0f,	8.0f,	"hipass_order", "Filter Order", "", "", 0},
	{ HPF_Freq,				SRS_PTYP_CFG,	SRS_PFMT_INT,			6.0f,	1.0f,	8.0f,	"hipass_frequency", "Filter Frequency", "", "", 0},
	{ HPF_Is32Bit,			SRS_PTYP_CFG,	SRS_PFMT_BOOL,			0.0f,	0.0f,	1.0f,	"hipass_is32bit", "Filter Precision (16bit when 0)", "", "", 0},
	{ HPF_Skip,				SRS_PTYP_CFG,	SRS_PFMT_BOOL,			0.0f,	0.0f,	1.0f,	"hipass_skip", "Skips High Pass Filter when true", "toggle", "", 0},	
};

SRS_Param* SRS_GetBank_HiPass(int& paramCount){
	paramCount = sizeof(gHiPass_Params)/sizeof(SRS_Param);
	return gHiPass_Params;
}

char* SRS_GetVersion_HiPass(char* pWork, size_t maxBytes){
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

void SRS_SetParam_HiPass(SRS_Tech_HiPass* pCFG, SRS_Param* pParam, const char* pValue){
	HELP_ParamIn In;
	
	switch (pParam->EnumID){
		case HPF_Skip: pCFG->Skip = In.GetBool(pValue); break;
		
		case HPF_Order: pCFG->Order = In.GetInt(pValue); break;
		case HPF_Freq: pCFG->Frequency = In.GetInt(pValue); break;
		case HPF_Is32Bit: pCFG->Is32Bit = In.GetBool(pValue); break;
	}
}

const char* SRS_GetParam_HiPass(SRS_Tech_HiPass* pCFG, SRS_Param* pParam){
	HELP_ParamOut Out;
	
	switch (pParam->EnumID){
		case HPF_Skip: return Out.FromBool(pCFG->Skip);
		
		case HPF_Order: return Out.FromInt(pCFG->Order);
		case HPF_Freq: return Out.FromInt(pCFG->Frequency);
		case HPF_Is32Bit: return Out.FromBool(pCFG->Is32Bit);
	}

	return "";
}

void SRS_Default_HiPass(SRS_Tech_HiPass* pCFG){
	pCFG->Skip = false;
	
	pCFG->Order = 4;
	pCFG->Frequency = 200;
	pCFG->Is32Bit = false;
}

};

