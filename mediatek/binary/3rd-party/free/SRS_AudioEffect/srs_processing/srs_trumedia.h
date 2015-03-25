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

#ifndef ANDROID_SRS_TRUMEDIA
#define ANDROID_SRS_TRUMEDIA

#include "srs_types.h"
#include "srs_techs/srs_tech_tools.h"
#include "srs_subs/srs_routing.h"

#ifdef _SRSCFG_WOWHDX
#include "srs_techs/srs_tech_wowhdx.h"
#else	// _SRSCFG_WOWHDX
#include "srs_techs/srs_tech_wowhd.h"
#include "srs_techs/srs_tech_hpf.h"
#endif	// _SRSCFG_WOWHDX

#include "srs_techs/srs_tech_cshp.h"

#include "srs_techs/srs_tech_trueq.h"
#include "srs_techs/srs_tech_hlimit.h"

#ifdef _SRSCFG_USERPEQ
	#include "srs_techs/srs_tech_userpeq.h"
	#define UPEQ_PRESET_COUNT	10
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
	#include "srs_techs/srs_tech_geq.h"
	#define GEQ_PRESET_COUNT	10
#endif	// _SRSCFG_MOBILE_EQ

namespace android {
	
#ifdef _SRSCFG_WOWHDX
	#define BID_Int_Mus_TruEQ	0
	#define BID_Int_Mov_TruEQ	1
	#define BID_Int_Pod_TruEQ	2
	#define BID_Int_All_TruEQ	0
	#define BID_Ext_All_TruEQ	3
	
	#define BID_Int_Mus_HLimL	0
	#define BID_Int_Mus_HLimB	1
	#define BID_Int_Mov_HLimL	2
	#define BID_Int_Mov_HLimB	3
	#define BID_Int_Pod_HLimL	4
	#define BID_Int_Pod_HLimB	5
	#define BID_Int_All_HLimL	0
	#define BID_Int_All_HLimB	1
	#define BID_Int_Byp_HLimL	6
	#define BID_Int_Byp_HLimB	7
	#define BID_Ext_All_HLimL	8
	#define BID_Ext_Byp_HLimL	9
#else	// _SRSCFG_WOWHDX
	#define BID_Int_Mus_TruEQ	0
	#define BID_Int_Mov_TruEQ	0
	#define BID_Int_Pod_TruEQ	0
	#define BID_Int_All_TruEQ	0
	#define BID_Ext_All_TruEQ	1
	
	#define BID_Int_Mus_HLimL	0
	#define BID_Int_Mus_HLimB	1
	#define BID_Int_Mov_HLimL	0
	#define BID_Int_Mov_HLimB	1
	#define BID_Int_Pod_HLimL	0
	#define BID_Int_Pod_HLimB	1
	#define BID_Int_All_HLimL	0
	#define BID_Int_All_HLimB	1
	#define BID_Int_Byp_HLimL	2
	#define BID_Int_Byp_HLimB	3
	#define BID_Ext_All_HLimL	4
	#define BID_Ext_Byp_HLimL	5
#endif	// _SRSCFG_WOWHDX

#define BID_TruEQ_Max (BID_Ext_All_TruEQ+1)
#define BID_HLim_Max (BID_Ext_Byp_HLimL+1)

#ifdef _SRSCFG_MOBILE_EQ_BASIC
	#define BID_Int_Mus_GEQ	0
	#define BID_Int_Mov_GEQ	0
	#define BID_Int_Pod_GEQ	0
	#define BID_Int_All_GEQ	0
	#define BID_Ext_Mus_GEQ	1
	#define BID_Ext_Mov_GEQ	1
	#define BID_Ext_Pod_GEQ	1
	#define BID_Ext_All_GEQ	1
	#define BID_GEQ_Max	2
#endif // _SRSCFG_MOBILE_EQ_BASIC

#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	#define BID_Int_Mus_GEQ	0
	#define BID_Int_Mov_GEQ	1
	#define BID_Int_Pod_GEQ	2
	#define BID_Int_All_GEQ	0
	#define BID_Ext_Mus_GEQ	3
	#define BID_Ext_Mov_GEQ	4
	#define BID_Ext_Pod_GEQ	5
	#define BID_Ext_All_GEQ	3
	#define BID_GEQ_Max	6
#endif // _SRSCFG_MOBILE_EQ_EXTENDED

enum {
	PROCST_OFF = 0,
	PROCST_IN,
	PROCST_ON,
	PROCST_OUT
};

#define PROC_FADELEN (1024)
#define PROC_DELAYIN (256)

struct SRS_Source_Out : public SRS_Base_Source {
	int RouteCode;	// The result of the route passed through the Routing Table...
	int LogRouteCode;
	int LogRoute;
	
	int ProcState;		// None, Fade In, Active, Fade Out
	int ProcTrack;		// Num of samples for Fade, etc...
	
	int Framesize;
	
	bool UseCache;
	void* pCacheSpool;
	SRSSamp* pCacheData;	// From
	SRSSamp* pCacheTarget;	// To
	int CachePos;
	
	SRSSamp ActiveGain;	// Pre-procesing scale
	
	SRS_DCRState DCState;	
	
#ifndef _SRSCFG_WOWHDX
	int HPFIndex;
	bool HPFPath;
	bool HPFActive;
	SRS_Source_HiPass* pHPF;
#endif	// _SRSCFG_WOWHDX
	
	int WOWHDIndex;
	SRS_Tech_State WHDState;
	SRS_Source_WOWHD* pWOWHD;
	void WHDConfigState(void);
	
	int CSHPIndex;
	SRS_Tech_State CSHPState;
	SRS_Source_CSHP* pCSHP;
	void CSHPConfigState(void);
	
#ifdef _SRSCFG_USERPEQ
	SRS_Tech_State UPEQState;
	SRS_Source_TruEQ* pUPEQ;
	void UPEQConfigState(void);
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
	SRS_Tech_State GEQState;
	SRS_Source_GEQ* pGEQ;
	void GEQConfigState(void);
#endif	// _SRSCFG_MOBILE_EQ

	int TruEQIndex;
	bool TruEQPath;
	bool TruEQActive;
	SRS_Source_TruEQ* pTruEQ;
	
	int HLimitIndex;
	bool HLimitPath;
	bool HLimitActive;
	SRS_Source_HLimit* pHLimit;
	
	SRS_Source_Out();
	~SRS_Source_Out();
	
	void SetRoute(int route);
	void Process(void* pSamples, int sampleBytes, int sampleRate, int countChans);
	
	void InitCaching();
	void SwapCaching();
	
	bool WillProcess();
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	void CoreProcess(int32_t* pSamples, int frames);
#else
    void CoreProcess(int16_t* pSamples, int frames);
#endif
	void SubProcess();
	
	void APIInit(int sampleRate, int countChans);
	void CFGSync();
};

struct SRS_Source_In : public SRS_Base_Source {
	SRS_Source_In();
	~SRS_Source_In();
	
	void SetRoute(int route);
	void Process(void* pSamples, int sampleBytes, int sampleRate, int countChans);
};

struct SRS_Workspace : public SRS_Base_Workspace {
	bool Skip;
	
	bool TMEnable;
	int TMPreset;
	bool VolIntEnable;	// Vol Int or Ext can be either MaxVF or TruVol
	
	float TMIGains[2];	// Int/Ext (add more for extra devices)
	
	int TMSkip;
	
	SRS_RouteMap RouteMap;
		
#ifdef _SRSCFG_MOBILE_EQ
	bool GEQEnables[BID_GEQ_Max];
	bool GEQLimits[BID_GEQ_Max];
	int GEQPresets[BID_GEQ_Max];
#endif

#ifdef _SRSCFG_PERFTRACK
	SRS_Perf PerfTrack;
	int PerfType;
#endif
	
#ifndef _SRSCFG_WOWHDX
	SRS_Tech_HiPass HPFTuning;
#endif	// _SRSCFG_WOWHDX

	SRS_Tech_WOWHD WHDTunings[5];
	SRS_Tech_CSHP CSHPTuning;
	SRS_Tech_TruEQ TEQTunings[BID_TruEQ_Max];
	SRS_Tech_HLimit HLTunings[BID_HLim_Max];
	
#ifdef _SRSCFG_USERPEQ
	SRS_Tech_UserPEQ_CFG UPEQCFG;
	SRS_Tech_UserPEQ_Preset UPEQInts[UPEQ_PRESET_COUNT];
#endif
	
#ifdef _SRSCFG_MOBILE_EQ
	SRS_Tech_GEQ GEQInts[GEQ_PRESET_COUNT];
	SRS_Tech_GEQ GEQExts[GEQ_PRESET_COUNT];
#endif
	
	SRS_Workspace();
	~SRS_Workspace();
	
	void SourceOutAdd(void* pSource);
	void SourceOutDel(void* pSource);
	SRS_Source_Out* SourceOutFind(void* pSource);
	
	void SourceInAdd(void* pSource);
	void SourceInDel(void* pSource);
	SRS_Source_In* SourceInFind(void* pSource);
	
	SRS_ParamBlock* GetParamBlock();
	void FillParamBlock(SRS_ParamBlock* pPB);
	void SetParamValue(SRS_ParamBlock* pPB, int bank, int param, const char* pValue);
	const char* GetParamValue(SRS_ParamBlock* pPB, int bank, int param);
	
	void DoInit();
	void LoadBaseConfigs();
	void LoadConfigs();
	
	void ApplyUserDefaults();
	
	bool ReadUserConfig(const char* pPath);
	void WriteUserConfig(const char* pPath);
	
	bool ConfigRead(const char* pPath, SRS_ParamBlock* pBlock, uint32_t typeMask=0xFFFFFFFF);
	void ConfigWrite(const char* pPath, SRS_ParamBlock* pBlock, uint32_t typeMask=0xFFFFFFFF);
	
#ifdef _SRSCFG_DSPOFFLOAD_PATH
	#define DSPOFFLOAD_WORKSPACE
	#include SRS_STR(_SRSCFG_DSPOFFLOAD_PATH)
	#undef DSPOFFLOAD_WORKSPACE
#endif // _SRSCFG_DSPOFFLOAD_PATH

	void DSPOffload_Send(SRS_Source_Out* pOut);
	void DSPOffload_Clear(SRS_Source_Out* pOut);
	
#ifdef SRS_AUDIOLOG
	int AL_Active;
	int AL_Status;
	FILE* pAL_OutPre;
	FILE* pAL_OutPost;
	
	void AL_Init();
	void AL_Exit();
	
	void AL_Start();
	void AL_Stop();
	void AL_PreProc(void* pSamples, int sampleBytes);
	void AL_PostProc(void* pSamples, int sampleBytes);
#endif
};

};

#endif	// ANDROID_SRS_TRUMEDIA

