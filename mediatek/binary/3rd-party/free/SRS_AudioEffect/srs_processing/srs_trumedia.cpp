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

#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <math.h>

#include "srs_types.h"
#include "srs_processing.h"
#include "srs_params.h"
#include "srs_workspace.h"
#include "srs_tech.h"

#include <media/AudioSystem.h>

#undef LOG_TAG
#define LOG_TAG "SRS_ProcT"

#define	FALSE					0
#define	TRUE					(!FALSE)

namespace android {

char gInfo_Scratch[1024];	// Used when creating the info strings - instead of dynamic alloc
char gParam_Scratch[512];	// Used when processing params - instead of dynamic alloc

enum {
	CFG_IPList = -100,
	CFG_Lib_Vers,
	CFG_Lib_Timestamp,
	CFG_Lib_Integration,
	CFG_Lib_Built,
	
	CFG_HPF_Vers,
	CFG_WHD_Vers,
	CFG_CSHP_Vers,
	CFG_TruEQ_Vers,
	CFG_HLimit_Vers,
	CFG_GEQ_Vers,
	
	CFG_Skip = 0,
	
	CFG_TMEnable,
	CFG_TMPreset,
	CFG_TMIGain_Int,
	CFG_TMIGain_Ext,
	CFG_VolIntEnable,
	CFG_TMSkip,
	
	CFG_GEQEnableInt,	// No need to ifdef - won't appear if not included in the params...
	CFG_GEQLimitInt,
	CFG_GEQPresetInt,
	CFG_GEQEnableExt,
	CFG_GEQLimitExt,
	CFG_GEQPresetExt,
	
	CFG_PerfType,
};
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
const char* gInfo_LibVers = "v2.3.10.0 "
#ifdef _SRSCFG_TRUMEDIA_HD
	"TruMedia HD"
#else	// _SRSCFG_TRUMEDIA_HD
	"TruMedia"
#endif	// _SRSCFG_TRUMEDIA_HD
#else
const char* gInfo_LibVers = "v2.3.9.0 "
#ifdef _SRSCFG_WOWHDX
	"TruMedia HD"
#else	// _SRSCFG_WOWHDX
	"TruMedia"
#endif	// _SRSCFG_WOWHDX
#endif
#ifdef _SRSCFG_LIBVARIANT
	" " SRS_STR(_SRSCFG_LIBVARIANT)
#endif	// _SRSCFG_LIBVARIANT
;	// End of Library Version String

// String defined that allows applications to determine the IP
const char* gInfo_IPs = "cshp,trueq,hlimit"
#ifdef _SRSCFG_WOWHDX
	",wowhdx"
#else	// _SRSCFG_WOWHDX
	",wowhd,hpf"
#endif	// _SRSCFG_WOWHDX
#ifdef _SRSCFG_MOBILE_EQ
	",geq"
#endif	// _SRSCFG_MOBILE_EQ
;	// End of IP String

SRS_Param gCFG_Params[] = {
	{ CFG_Lib_Vers,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"lib_version", "Library Version", gInfo_LibVers, "", 0},
	{ CFG_Lib_Timestamp,	SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"lib_timestamp", "Library Timestamp", __TIMESTAMP__, "", 0},
	{ CFG_Lib_Integration,	SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"lib_integration", "Library Integration", "v2.0.0.0", "", 0},
	{ CFG_Lib_Built,		SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"lib_built", "Library Built", __DATE__ " " __TIME__, "", 0},
	{ CFG_IPList,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"ip_list", "IP Available", gInfo_IPs, "", 0},
	
#ifdef _SRSCFG_WOWHDX
	{ CFG_WHD_Vers,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"wowhdx_version", "WOWHDX Version", "", "", 0},
#else	// _SRSCFG_WOWHDX
	{ CFG_WHD_Vers,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"wowhd_version", "WOWHD Version", "", "", 0},
	{ CFG_HPF_Vers,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"hpf_version", "High Pass Filter Version", "", "", 0},
#endif	// _SRSCFG_WOWHDX
	
	{ CFG_CSHP_Vers,		SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"cshp_version", "CSHP Version", "", "", 0},
	{ CFG_TruEQ_Vers,		SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"trueq_version", "TruEQ Version", "", "", 0},
	{ CFG_HLimit_Vers,		SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"hlimit_version", "Hard Limiter Version", "", "", 0},
	
#ifdef _SRSCFG_MOBILE_EQ
	{ CFG_GEQ_Vers,			SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"geq_version", "MobileEQ Version", "", "", 0},
#endif	// _SRSCFG_MOBILE_EQ

	{ CFG_Skip,				SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"srs_skip", "Skips any SRS processing when true", "toggle", "", 0},

	{ CFG_TMEnable,			SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trumedia_enable", "TruMedia Toggle", "Turn some of TruMedia on/off", "", 0},
	{ CFG_TMPreset,			SRS_PTYP_PREF,	SRS_PFMT_ENUM,		0.0f,	0.0f,	0.0f,	"trumedia_preset", "TruMedia Preset", "The active media type", "Music,Movie,Podcast", 0},
	
	{ CFG_TMIGain_Int,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.25f,	0.0f,	1.0f,	"trumedia_igain_int", "TruMedia Input Gain - Internal", "gain", "", 0},
	{ CFG_TMIGain_Ext,		SRS_PTYP_CFG,	SRS_PFMT_FLOAT,		0.25f,	0.0f,	1.0f,	"trumedia_igain_ext", "TruMedia Input Gain - External", "gain", "", 0},
	
	{ CFG_VolIntEnable,		SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"vol_int_enable", "Volume Conditioning Toggle - Internal", "is Volume Boost enabled?", "", 0},
	{ CFG_TMSkip,			SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"trumedia_skip", "TruMedia Process Skip", "Turn all of TruMedia on/off in response to _enable", "", 0},
	
#ifdef _SRSCFG_PERFTRACK
	{ CFG_PerfType,			SRS_PTYP_PREF,	SRS_PFMT_INT,		0.0f,	-1.0f,	1000.0f,	"perf_type", "Performance Metrics Type", "-1 is disabled", "", 0},
#endif	// _SRSCFG_PERFTRACK
};

#ifdef _SRSCFG_MOBILE_EQ_BASIC
SRS_Param gCFG_EQParams[] = {
	{ 0,		SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_int_enable", "MobileEQ Toggle - Internal", "GEQ on/off", "", 0},
	{ 1,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_int_limit_enable", "MobileEQ Limiter Toggle - Internal", "GEQ limiter on/off", "", 0},
	{ 2,		SRS_PTYP_PREF,	SRS_PFMT_INT,		0.0f,	0.0f,	9.0f,	"geq_int_preset", "MobileEQ Preset - Internal", "The active GEQ preset", "", 0},
	{ 100,		SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_ext_enable", "MobileEQ Toggle - External", "GEQ on/off", "", 0},
	{ 101,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_ext_limit_enable", "MobileEQ Limiter Toggle - External", "GEQ limiter on/off", "", 0},
	{ 102,		SRS_PTYP_PREF,	SRS_PFMT_INT,		0.0f,	0.0f,	9.0f,	"geq_ext_preset", "MobileEQ Preset - External", "The active GEQ preset", "", 0},
};
#endif	// _SRSCFG_MOBILE_EQ_BASIC

#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
SRS_Param gCFG_EQParams[] = {
	{ 0,		SRS_PTYP_PREF,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_enable", "MobileEQ Toggle", "GEQ on/off", "", 0},
	{ 1,		SRS_PTYP_CFG,	SRS_PFMT_BOOL,		0.0f,	0.0f,	1.0f,	"geq_limit_enable", "MobileEQ Limiter Toggle", "GEQ limiter on/off", "", 0},
	{ 2,		SRS_PTYP_PREF,	SRS_PFMT_INT,		0.0f,	0.0f,	9.0f,	"geq_preset", "MobileEQ Preset", "The active GEQ preset", "", 0},
};
#endif	// _SRSCFG_MOBILE_EQ_EXTENDED

enum {
	BANK_CFG = 0,
	BANK_EQCFG,
	BANK_Routes,
	BANK_WOWHD,
	BANK_CSHP,
	BANK_HPF,
	BANK_TruEQ,
	BANK_HLimit,
	BANK_UPEQCFG,
	BANK_UPEQ,
	BANK_GEQ,
};

SRS_ParamBank gSRS_Banks[] = {
	{ BANK_CFG,		0, "Config", "srs_cfg", "Non-IP Configuration", gCFG_Params, sizeof(gCFG_Params)/sizeof(SRS_Param), 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_BASIC
	{ BANK_EQCFG,	0, "Config", "srs_cfg", "EQ Preset Configuration", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_BASIC
	
#ifdef _SRSCFG_USERPEQ
	{ BANK_UPEQCFG,	0, "Config", "srs_cfg", "TruEQ Preset Configuration", NULL, 0, 0, 0 },
#endif	// _SRSCFG_USERPEQ
	
	{ BANK_Routes,	0, "Routing", "srs_route_out", "Output Device Routing", SRS_RouteMap::RouteParams(), SRS_RouteMap::RouteParamCount(), 0, 0 },
	
	{ BANK_WOWHD,	0, "WOWHD", "srs_mus_int", "Music-Internal", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	0, "GEQ", "srs_mus_geq_int", "Music-Internal GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED
	
#ifdef _SRSCFG_WOWHDX
	{ BANK_TruEQ,	BID_Int_Mus_TruEQ, "TruEQ", "srs_mus_spk_int", "Music-Internal Speaker Tuning", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Mus_HLimL, "HLimit", "srs_mus_limit_int", "Music-Internal Limiter", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Mus_HLimB, "HLimit", "srs_mus_boost_int", "Music-Internal Boosted", NULL, 0, 0, 0 },
#endif	// _SRSCFG_WOWHDX
	
	{ BANK_WOWHD,	1, "WOWHD", "srs_mov_int", "Movie-Internal", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	1, "GEQ", "srs_mov_geq_int", "Movie-Internal GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED
	
#ifdef _SRSCFG_WOWHDX
	{ BANK_TruEQ,	BID_Int_Mov_TruEQ, "TruEQ", "srs_mov_spk_int", "Movie-Internal Speaker Tuning", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Mov_HLimL, "HLimit", "srs_mov_limit_int", "Movie-Internal Limiter", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Mov_HLimB, "HLimit", "srs_mov_boost_int", "Movie-Internal Boosted", NULL, 0, 0, 0 },
#endif	// _SRSCFG_WOWHDX
	
	{ BANK_WOWHD,	2, "WOWHD", "srs_pod_int", "Podcast-Internal", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	2, "GEQ", "srs_pod_geq_int", "Podcast-Internal GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED
	
#ifdef _SRSCFG_WOWHDX
	{ BANK_TruEQ,	BID_Int_Pod_TruEQ, "TruEQ", "srs_pod_spk_int", "Podcast-Internal Speaker Tuning", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Pod_HLimL, "HLimit", "srs_pod_limit_int", "Podcast-Internal Limiter", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Pod_HLimB, "HLimit", "srs_pod_boost_int", "Podcast-Internal Boosted", NULL, 0, 0, 0 },
#endif	// _SRSCFG_WOWHDX

#ifndef _SRSCFG_WOWHDX
	{ BANK_HPF,		0, "HiPass", "srs_hpf_int", "Internal Speaker HPF", NULL, 0, 0, 0 },

	{ BANK_TruEQ,	BID_Int_All_TruEQ, "TruEQ", "srs_spk_int", "Internal Speaker Tuning", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_All_HLimL, "HLimit", "srs_limit_int", "Internal Limiter", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_All_HLimB, "HLimit", "srs_boost_int", "Internal Boosted", NULL, 0, 0, 0 },
#endif	// _SRSCFG_WOWHDX
	
	{ BANK_HLimit,	BID_Int_Byp_HLimL, "HLimit", "srs_bypass_limit_int", "Internal Bypassing Limiter", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Int_Byp_HLimB, "HLimit", "srs_bypass_boost_int", "Internal Bypassing Boosted", NULL, 0, 0, 0 },
	
	{ BANK_WOWHD,	3, "WOWHD", "srs_mus_ext", "Music-External", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	3, "GEQ", "srs_mus_geq_ext", "Music-External GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED
	
	{ BANK_CSHP,	0, "CSHP", "srs_mov_ext", "Movie-External", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	4, "GEQ", "srs_mov_geq_ext", "Movie-External GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED

	{ BANK_WOWHD,	4, "WOWHD", "srs_pod_ext", "Podcast-External", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_MOBILE_EQ_EXTENDED
	{ BANK_EQCFG,	5, "GEQ", "srs_pod_geq_ext", "Podcast-External GEQ", gCFG_EQParams, sizeof(gCFG_EQParams)/sizeof(SRS_Param), 0, 0 },
#endif // _SRSCFG_MOBILE_EQ_EXTENDED
	
	{ BANK_TruEQ,	BID_Ext_All_TruEQ, "TruEQ", "srs_spk_ext", "External Speaker Tuning", NULL, 0, 0, 0 },
	{ BANK_HLimit,	BID_Ext_All_HLimL, "HLimit", "srs_limit_ext", "External Limiter", NULL, 0, 0, 0 },
	
	{ BANK_HLimit,	BID_Ext_Byp_HLimL, "HLimit", "srs_bypass_limit_ext", "External Bypassing Limiter", NULL, 0, 0, 0 },
	
#ifdef _SRSCFG_USERPEQ
	{ BANK_UPEQ,	0, "UPEQ", "srs_upeq_0_int", "UPEQ Preset 0-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	1, "UPEQ", "srs_upeq_1_int", "UPEQ Preset 1-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	2, "UPEQ", "srs_upeq_2_int", "UPEQ Preset 2-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	3, "UPEQ", "srs_upeq_3_int", "UPEQ Preset 3-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	4, "UPEQ", "srs_upeq_4_int", "UPEQ Preset 4-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	5, "UPEQ", "srs_upeq_5_int", "UPEQ Preset 5-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	6, "UPEQ", "srs_upeq_6_int", "UPEQ Preset 6-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	7, "UPEQ", "srs_upeq_7_int", "UPEQ Preset 7-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	8, "UPEQ", "srs_upeq_8_int", "UPEQ Preset 8-Internal", NULL, 0, 0, 0 },
	{ BANK_UPEQ,	9, "UPEQ", "srs_upeq_9_int", "UPEQ Preset 9-Internal", NULL, 0, 0, 0 },
#endif	// _SRSCFG_USERPEQ
	
#ifdef _SRSCFG_MOBILE_EQ
	{ BANK_GEQ,		0, "GEQ", "srs_geq_0_int", "GEQ Preset 0-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		1, "GEQ", "srs_geq_1_int", "GEQ Preset 1-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		2, "GEQ", "srs_geq_2_int", "GEQ Preset 2-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		3, "GEQ", "srs_geq_3_int", "GEQ Preset 3-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		4, "GEQ", "srs_geq_4_int", "GEQ Preset 4-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		5, "GEQ", "srs_geq_5_int", "GEQ Preset 5-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		6, "GEQ", "srs_geq_6_int", "GEQ Preset 6-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		7, "GEQ", "srs_geq_7_int", "GEQ Preset 7-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		8, "GEQ", "srs_geq_8_int", "GEQ Preset 8-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		9, "GEQ", "srs_geq_9_int", "GEQ Preset 9-Internal", NULL, 0, 0, 0 },
	{ BANK_GEQ,		10, "GEQ", "srs_geq_0_ext", "GEQ Preset 0-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		11, "GEQ", "srs_geq_1_ext", "GEQ Preset 1-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		12, "GEQ", "srs_geq_2_ext", "GEQ Preset 2-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		13, "GEQ", "srs_geq_3_ext", "GEQ Preset 3-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		14, "GEQ", "srs_geq_4_ext", "GEQ Preset 4-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		15, "GEQ", "srs_geq_5_ext", "GEQ Preset 5-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		16, "GEQ", "srs_geq_6_ext", "GEQ Preset 6-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		17, "GEQ", "srs_geq_7_ext", "GEQ Preset 7-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		18, "GEQ", "srs_geq_8_ext", "GEQ Preset 8-External", NULL, 0, 0, 0 },
	{ BANK_GEQ,		19, "GEQ", "srs_geq_9_ext", "GEQ Preset 9-External", NULL, 0, 0, 0 },
#endif	// _SRSCFG_MOBILE_EQ
};

SRS_ParamBlock gSRS_Params = { 0, gSRS_Banks, sizeof(gSRS_Banks)/sizeof(SRS_ParamBank), String8("") };

SRS_Source_In::SRS_Source_In(){
#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] SrcIn: %d", sizeof(SRS_Source_In));
#endif	//_SRSCFG_PERFTRACK

}

SRS_Source_In::~SRS_Source_In(){
}

void SRS_Source_In::SetRoute(int route){
}

void SRS_Source_In::Process(void* pSamples, int sampleBytes, int sampleRate, int countChans){
}
	
SRS_Source_Out::SRS_Source_Out(){
	Route = -2;
	LogRouteCode = -1000;
	LogRoute = -1000;
	
	ProcState = PROCST_OFF;
	ProcTrack = 0;

	Framesize = -1;
	
	UseCache = false;
	pCacheSpool = SRS_Spool_CreateCache();
	CachePos = 0;
	
#ifdef _SRSCFG_ARCH_ARM
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	ActiveGain = 256;
#else
    ActiveGain = 65536;
#endif
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	ActiveGain = 1.0f;
#endif	// _SRSCFG_ARCH_X86
	
#ifndef _SRSCFG_WOWHDX
	HPFPath = false;
	HPFActive = false;
	pHPF = NULL;
#endif	// _SRSCFG_WOWHDX
	
	pWOWHD = NULL;
	pCSHP = NULL;
	
	TruEQPath = false;
	TruEQActive = false;
	pTruEQ = NULL;
	
	HLimitPath = false;
	HLimitActive = false;
	pHLimit = NULL;
	
#ifdef _SRSCFG_USERPEQ
	pUPEQ = NULL;
#endif	// _SRSCFG_USERPEQ
	
#ifdef _SRSCFG_MOBILE_EQ
	pGEQ = NULL;
#endif	// _SRSCFG_MOBILE_EQ

#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] SrcOut: %d", sizeof(SRS_Source_Out));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Source_Out::~SRS_Source_Out(){
	
#ifndef _SRSCFG_WOWHDX
	if (pHPF != NULL) SRS_Destroy_HiPass(pHPF, this);
	pHPF = NULL;
#endif	// _SRSCFG_WOWHDX
	
	if (pWOWHD != NULL) SRS_Destroy_WOWHD(pWOWHD, this);
	pWOWHD = NULL;
	
	if (pCSHP != NULL) SRS_Destroy_CSHP(pCSHP, this);
	pCSHP = NULL;
	
	if (pTruEQ != NULL) SRS_Destroy_TruEQ(pTruEQ, this);
	pTruEQ = NULL;
	
	if (pHLimit != NULL) SRS_Destroy_HLimit(pHLimit, this);
	pHLimit = NULL;
	
#ifdef _SRSCFG_USERPEQ
	if (pUPEQ != NULL) SRS_Destroy_TruEQ(pUPEQ, this);
	pUPEQ = NULL;
#endif	// _SRSCFG_USERPEQ
	
#ifdef _SRSCFG_MOBILE_EQ
	if (pGEQ != NULL) SRS_Destroy_GEQ(pGEQ, this);
	pGEQ = NULL;
#endif	// _SRSCFG_MOBILE_EQ

	SRS_Spool_DestroyCache(pCacheSpool);
}

void SRS_Source_Out::SetRoute(int route){
	if (route == Route) return;
	Route = route;
	pOwner->CFGSig++;
}

void SRS_Source_Out::APIInit(int sampleRate, int countChans){
	DidAPIInit = false;
	
	SampleRate = sampleRate;
	if (countChans == 2)	// Won´t code mono-friendly systems just yet...
		ChannelCount = countChans;
	else
		ChannelCount = -1;
	
	if ((SampleRate < 0) || (ChannelCount < 0)) return;	// Unable to setup API...
	
	SRS_LOG("API Init");
	
	DidAPIInit = true;
}

void SRS_Source_Out::CFGSync(){
	if (CFGSig == pOwner->CFGSig) return;	
	CFGSig = pOwner->CFGSig;
	
	bool bWillProc = true;
	if ((pOwner->TMSkip) && (pOwner->TMEnable == false)) bWillProc = false;
	if (pOwner->Skip) bWillProc = false;

	if (bWillProc == false){
		pOwner->DSPOffload_Clear(this);
		return;	// DO NOT CONFIG when we may not have valid data, and won't use it anyway...
	}
	
	int i;
	int tDevice = 0;
	
	int tRouteMid = Route;
	int tPreRoute = Route;
	tDevice = pOwner->RouteMap.ResolveRoute(Route, &tRouteMid);
	Route = tRouteMid;	// The Resolve may have changed it...
	
	if (LogRoute != Route){
		LogRoute = Route;
		SRS_LOG("Route Flags %d / %d", tPreRoute, Route);
	}
	
	RouteCode = tDevice;
	
	if (LogRouteCode != RouteCode){
		LogRouteCode = RouteCode;
		SRS_LOG("Route Type %d = %d", Route, RouteCode);
	}
	
	int tTarget = 0;	// ARM
	
	if (RouteCode < 0){
		if (RouteCode == -1) {		// -1 is universal for 'no output'
			pOwner->DSPOffload_Clear(this);
			return;
		}
		tTarget = 1;	// DSP
		tDevice = (RouteCode*-1)-2;	// -2 = 0, -3 = 1, etc
	}
			
	Framesize = -1;
	
#ifdef _SRSCFG_ARCH_ARM
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	if (tDevice == 0) ActiveGain = (int)256*pOwner->TMIGains[0];
	else ActiveGain = (int)256*pOwner->TMIGains[1];
#else
	if (tDevice == 0) ActiveGain = (int)65536*pOwner->TMIGains[0];
	else ActiveGain = (int)65536*pOwner->TMIGains[1];
#endif
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
	if (tDevice == 0) ActiveGain = pOwner->TMIGains[0];
	else ActiveGain = pOwner->TMIGains[1];
#endif	// _SRSCFG_ARCH_X86
	
#ifndef _SRSCFG_WOWHDX
	// High Pass Filter Path Logic
	HPFIndex = -1;
	HPFPath = true;
	HPFActive = false;
	if (pOwner->HPFTuning.Skip) HPFPath = false;
	if (tDevice != 0) HPFPath = false;
	if (tDevice == 0) HPFActive = true;
	
	if (HPFPath){
		if (pHPF == NULL) pHPF = SRS_Create_HiPass(this);
		if (pHPF != NULL) SRS_Config_HiPass(pHPF, this, &pOwner->HPFTuning, !HPFActive);
	}
	
	if (pHPF == NULL) HPFPath = false;
	
	if (HPFActive && HPFPath) HPFIndex = 0;
	
#endif	// _SRSCFG_WOWHDX

	// WOWHD Audio Path Logic
	WOWHDIndex = -1;
	bool tWPath = false;
	bool tWActive = false;
	SRSFadeScale tWScale = SRSFadeScale_Default;
	if ((tDevice == 0) && (pOwner->TMPreset == 0)) WOWHDIndex = 0;
	if ((tDevice == 0) && (pOwner->TMPreset == 1)) WOWHDIndex = 1;
	if ((tDevice == 0) && (pOwner->TMPreset == 2)) WOWHDIndex = 2;
	if ((tDevice == 1) && (pOwner->TMPreset == 0)) WOWHDIndex = 3;
	if ((tDevice == 1) && (pOwner->TMPreset == 2)) WOWHDIndex = 4;
	
	if ((WOWHDIndex >= 0) && (WOWHDIndex <= 4)){
		tWPath = true;
		tWActive = true;
		tWScale = SRS_Tech_State::CalcFadeScale(pOwner->WHDTunings[WOWHDIndex].IGain);
		if (pOwner->WHDTunings[WOWHDIndex].Skip) tWPath = false;
		if (pOwner->TMEnable == false) tWActive = false;
	}
	
	if (tWPath){
		if (pWOWHD == NULL) pWOWHD = SRS_Create_WOWHD(this);
	}
		
	if (pWOWHD == NULL) tWPath = false;
	if (WHDState.SetWants(tWPath, tWActive, WOWHDIndex, tWScale) == false) WHDConfigState();
	
	if (tWActive == false) WOWHDIndex = -1;
	if (tWPath == false) WOWHDIndex = -1;
	
	// CSHP Audio Path Logic
	CSHPIndex = -1;
	bool tCPath = false;
	bool tCActive = false;
	SRSFadeScale tCScale = SRSFadeScale_Default;
	if ((tDevice == 1) && (pOwner->TMPreset == 1)){
		tCPath = true;
		tCActive = true;
		tCScale = SRS_Tech_State::CalcFadeScale(pOwner->CSHPTuning.IGain);
		if (pOwner->CSHPTuning.Skip) tCPath = false;
		if (pOwner->TMEnable == false) tCActive = false;
	}
	
	if (tCPath){
		if (pCSHP == NULL) pCSHP = SRS_Create_CSHP(this);
	}

	if (pCSHP == NULL) tCPath = false;
	if (CSHPState.SetWants(tCPath, tCActive, 0, tCScale) == false) CSHPConfigState();
	
	if (tCActive && tCPath) CSHPIndex = 0;
		
	// TruEQ Audio Path Logic
	TruEQIndex = -1;
	TruEQPath = true;
	TruEQActive = true;
	
	if ((tDevice == 0) && (pOwner->TMPreset == 0)) TruEQIndex = BID_Int_Mus_TruEQ;
	if ((tDevice == 0) && (pOwner->TMPreset == 1)) TruEQIndex = BID_Int_Mov_TruEQ;
	if ((tDevice == 0) && (pOwner->TMPreset == 2)) TruEQIndex = BID_Int_Pod_TruEQ;
	if (tDevice == 1) TruEQIndex = BID_Ext_All_TruEQ;
		
	if (TruEQIndex >= 0){
		if (pOwner->TEQTunings[TruEQIndex].Skip) TruEQPath = false;
	} else TruEQPath = false;
		
	if (TruEQPath){
		if (pTruEQ == NULL) pTruEQ = SRS_Create_TruEQ(this);
		if (pTruEQ != NULL) SRS_Config_TruEQ(pTruEQ, this, pOwner->TEQTunings+TruEQIndex, !TruEQActive);
	}
	
	if (pTruEQ == NULL) TruEQPath = false;
	
	if (TruEQPath == false) TruEQIndex = -1;
	if (TruEQActive == false) TruEQIndex = -1;
	
	// Hard Limiter Path Logic
	HLimitIndex = BID_Ext_All_HLimL;
	HLimitPath = true;
	HLimitActive = true;
		
	if (tDevice == 0){
		if (pOwner->TMEnable == true){
			if (pOwner->TMPreset == 0) HLimitIndex = BID_Int_Mus_HLimL;
			if (pOwner->TMPreset == 1) HLimitIndex = BID_Int_Mov_HLimL;
			if (pOwner->TMPreset == 2) HLimitIndex = BID_Int_Pod_HLimL;
		} else HLimitIndex = BID_Int_Byp_HLimL;
		if (pOwner->VolIntEnable == true) HLimitIndex += 1;
	} else {
		HLimitIndex = BID_Ext_All_HLimL;
		if (pOwner->TMEnable == false) HLimitIndex = BID_Ext_Byp_HLimL;
	}
		
	if (pOwner->HLTunings[HLimitIndex].Skip) HLimitPath = false;
	
	if (HLimitPath){
		if (pHLimit == NULL) pHLimit = SRS_Create_HLimit(this);
		if (pHLimit != NULL) SRS_Config_HLimit(pHLimit, this, pOwner->HLTunings+HLimitIndex, !HLimitActive);
	}
	
	if (pHLimit == NULL) HLimitPath = false;
	
	if (HLimitPath == false) HLimitIndex = -1;
	if (HLimitActive == false) HLimitIndex = -1;
	
#ifdef _SRSCFG_USERPEQ
	int tUPEQIdx = -1;
	bool tUPEQPath = false;
	bool tUPEQActive = pOwner->UPEQCFG.Enabled;
	SRS_Tech_UserPEQ_Preset* pUPEQPre = NULL;
	if (tDevice == 0) tUPEQIdx = pOwner->UPEQCFG.Preset_Int;
	if ((tUPEQIdx < 0) || (tUPEQIdx >= UPEQ_PRESET_COUNT)) tUPEQIdx = -1;
	else tUPEQPath = true;

	if (pOwner->UPEQCFG.Skip) tUPEQPath = false;

	if (tUPEQPath){
		if (pUPEQ == NULL) pUPEQ = SRS_Create_TruEQ(this);
	}

	if (pUPEQ == NULL) tUPEQPath = false;
	if (UPEQState.SetWants(tUPEQPath, tUPEQActive, tUPEQIdx) == false) UPEQConfigState();	
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
	// GEQ Audio Path Logic
	int tGIdx = -1;
	int tGPref = -1;
	bool tGEQPath = false;
	
	if ((tDevice == 0) && (pOwner->TMPreset == 0)) tGPref = BID_Int_Mus_GEQ;
	if ((tDevice == 0) && (pOwner->TMPreset == 1)) tGPref = BID_Int_Mov_GEQ;
	if ((tDevice == 0) && (pOwner->TMPreset == 2)) tGPref = BID_Int_Pod_GEQ;
	if ((tDevice == 1) && (pOwner->TMPreset == 0)) tGPref = BID_Ext_Mus_GEQ;
	if ((tDevice == 1) && (pOwner->TMPreset == 1)) tGPref = BID_Ext_Mov_GEQ;
	if ((tDevice == 1) && (pOwner->TMPreset == 2)) tGPref = BID_Ext_Pod_GEQ;
	
	if (tGPref >= 0){
		if (pOwner->GEQEnables[tGPref] == true) tGIdx = pOwner->GEQPresets[tGPref];
		
		if ((tGIdx < 0) || (tGIdx >= GEQ_PRESET_COUNT)) tGPref = -1;
		else tGEQPath = true;
	}
	
	if (tGEQPath){
		if (pOwner->GEQLimits[tGPref]) tGIdx |= 0x10000;	// Limiter setting...
		if (tDevice == 1) tGIdx |= 0x1000;		// Mark for 'external' EQ preset stack...
		
		if (pGEQ == NULL) pGEQ = SRS_Create_GEQ(this);
	}
	
	if (pGEQ == NULL) tGEQPath = false;
	if (GEQState.SetWants(tGEQPath, 1, tGIdx) == false) GEQConfigState();
#endif	// _SRSCFG_MOBILE_EQ

	if (tTarget > 0){
		pOwner->DSPOffload_Send(this);
	} else {
		pOwner->DSPOffload_Clear(this);
	}
}

void SRS_Source_Out::WHDConfigState(void){
	if (WHDState.InPath == false) return;
	
	SRS_Config_WOWHD(pWOWHD, this, pOwner->WHDTunings+WHDState.UseIndex, !WHDState.IsActive);
}

void SRS_Source_Out::CSHPConfigState(void){
	if (CSHPState.InPath == false) return;
	
	SRS_Config_CSHP(pCSHP, this, &pOwner->CSHPTuning, !CSHPState.IsActive);
}

#ifdef _SRSCFG_USERPEQ
void SRS_Source_Out::UPEQConfigState(void){
	if (UPEQState.InPath == false) return;
	SRS_Tech_UserPEQ_Preset* pUPEQPre = pOwner->UPEQInts+UPEQState.UseIndex;
	
	SRS_Tech_TruEQ tTune;
	SRS_Apply_UserPEQ_Preset(&tTune, pUPEQPre);
	SRS_Apply_UserPEQ_CFG(&tTune, &pOwner->UPEQCFG);
	SRS_Config_TruEQ(pUPEQ, this, &tTune, !UPEQState.IsActive);
}
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
void SRS_Source_Out::GEQConfigState(void){
	if (GEQState.InPath == false) return;
	SRS_Tech_GEQ* pG = NULL;
	
	int tIdx = GEQState.UseIndex&0xFF;
	if (GEQState.UseIndex&0x1000) pG = pOwner->GEQExts+tIdx;
	else pG = pOwner->GEQInts+tIdx;
			
	SRS_Config_GEQ(pGEQ, this, pG, (GEQState.UseIndex&0x10000)>0?true:false);
}
#endif	// _SRSCFG_MOBILE_EQ

bool SRS_Source_Out::WillProcess(){
	if (CFGSig != pOwner->CFGSig) CFGSync();
	if ((pOwner->TMSkip) && (pOwner->TMEnable == false)) return false;
	if (pOwner->Skip) return false;
	if (RouteCode < 0) return false;
	
	return true;
}
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
const int32_t gProcStep = 256/(PROC_FADELEN-PROC_DELAYIN);	// 24 bit PCM has only 8 bit left
void FadeLoop_2Chan(int32_t* pOut, int32_t* pS0, int32_t* pS1, int32_t tween, int32_t step, int steps){
#else
const int32_t gProcStep = 65536/(PROC_FADELEN-PROC_DELAYIN);
void FadeLoop_2Chan(int16_t* pOut, int16_t* pS0, int16_t* pS1, int32_t tween, int32_t step, int steps){
#endif
	int i;
	for (i=0; i<steps; i++, tween+=step, pOut+=2, pS0+=2, pS1+=2){
		int tTw = tween;
		if (tTw < 0) tTw = 0;
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
		else if (tTw > 256) tTw = 256;
		
		pOut[0] = ((pS0[0]>>8)*tTw)+((pS1[0]>>8)*(256-tTw));
		pOut[1] = ((pS0[1]>>8)*tTw)+((pS1[1]>>8)*(256-tTw));
#else
	    else if (tTw > 65536) tTw = 65536;
		
		int32_t tL = (pS0[0]*tTw)+(pS1[0]*(65536-tTw));
		int32_t tR = (pS0[1]*tTw)+(pS1[1]*(65536-tTw));
		pOut[0] = tL>>16;
		pOut[1] = tR>>16;
#endif
	}
}

void SRS_Source_Out::Process(void* pSamples, int sampleBytes, int sampleRate, int countChans){
	// Will NOT touch Audio in these cases...
	if (sampleRate < 0) return;
	if (countChans != 2) return;
	if (DidAPIInit == false) APIInit(sampleRate, countChans);
	if (DidAPIInit == false) return;	// Still not ready?
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	int tFrames = sampleBytes/(sizeof(int32_t)*countChans);	// please make sure that this one works with your system
#else
	int tFrames = sampleBytes/(sizeof(int16_t)*countChans);
#endif
	if (Framesize != tFrames){
		Framesize = tFrames;
		if ((Framesize%256 != 0) && (UseCache == false)){
			SRS_LOG("Framesize of %d forcing Cache!!", Framesize);
			InitCaching();
		}
	}
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	int32_t FadeHold[512];	// 256*stereo
#else
    int16_t FadeHold[512];	// 256*stereo
#endif
	
	bool tWantProcess = WillProcess();
	
	if (tWantProcess == false){	// Turning off...
		if (ProcState == PROCST_OFF) return;	// Already off...
		else if (ProcState == PROCST_IN){	// Was fading in, fade out...
			ProcState = PROCST_OUT;
			ProcTrack = PROC_FADELEN-ProcTrack;
		} else if (ProcState == PROCST_ON){	// start fade out..
			ProcState = PROCST_OUT;
			ProcTrack = 0;
		}
		// else already fading out - keep going...
	} else {
		if (ProcState == PROCST_OFF){
			ProcState = PROCST_IN;
			ProcTrack = 0;
			if (UseCache == true) InitCaching();	// Caching?  Reset...
		} else if (ProcState == PROCST_OUT){
			ProcState = PROCST_IN;
			ProcTrack = PROC_FADELEN-ProcTrack;
		}
	}
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT		
	int32_t* pSampsIn = (int32_t*)pSamples;
#else
    int16_t* pSampsIn = (int16_t*)pSamples;
#endif
		
	// Fade In/Out... may return early...
	while ((ProcState == PROCST_OUT) || (ProcState == PROCST_IN)){
		SRS_LOG("Fading %d - %d", ProcState, ProcTrack);
		int tProcLen = (tFrames<256)?tFrames:256;	// How many samples to process?
		tFrames -= tProcLen;	// Pull that many off...
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
		int32_t* pSampsOrig = FadeHold;
		memcpy(FadeHold, pSampsIn, tProcLen*sizeof(int32_t)*countChans);	// Back up data for fade...
#else
        int16_t* pSampsOrig = FadeHold;
		memcpy(FadeHold, pSampsIn, tProcLen*sizeof(int16_t)*countChans);	// Back up data for fade...
#endif
		CoreProcess(pSampsIn, tProcLen);	// Process...
		
		int tProcRemain = PROC_FADELEN-ProcTrack;	// How much can we process?
		int tDoProc = (tProcRemain<tProcLen)?tProcRemain:tProcLen;	// How many will we process?
		int tDoPass = (tProcLen>tDoProc)?(tProcLen-tDoProc):0;		// How many will we fill from history/processed?
		int32_t tTween = ProcTrack*gProcStep;	// Where we are in the fade...
		ProcTrack += tDoProc;
		
		if (ProcState == PROCST_OUT){	// Fading out...
			FadeLoop_2Chan(pSampsIn, pSampsOrig, pSampsIn, tTween, gProcStep, tDoProc);
			pSampsIn += 2*tDoProc;
			pSampsOrig += 2*tDoProc;
			
			if (tDoPass > 0){	// We over processed - so restore from the originals we saved...
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				memcpy(pSampsIn, pSampsOrig, tDoPass*sizeof(int32_t)*countChans);
#else
                memcpy(pSampsIn, pSampsOrig, tDoPass*sizeof(int16_t)*countChans); 
#endif
				pSampsIn += 2*tDoPass;
			}
			
			if (ProcTrack >= PROC_FADELEN){
				ProcState = PROCST_OFF;	// Done!!!
				return;
			}
		} else {						// Fading in...
			tTween -= PROC_DELAYIN*gProcStep;	// Adjust for the fade-in delay...
			FadeLoop_2Chan(pSampsIn, pSampsIn, pSampsOrig, tTween, gProcStep, tDoProc);
			pSampsIn += 2*tProcLen;		// Anything past tDoProc until tProcLen will be what we processed... no 'pass' phase...
			
			if (ProcTrack >= PROC_FADELEN){
				ProcState = PROCST_ON;	// Done!!!
			}
		}
		
		if (tFrames <= 0) break;
	}
	
	if (tFrames > 0) CoreProcess(pSampsIn, tFrames);
}
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
void SRS_Source_Out::CoreProcess(int32_t* pSamples, int frames){
#else
void SRS_Source_Out::CoreProcess(int16_t* pSamples, int frames){
#endif
	
#ifdef _SRSCFG_PERFTRACK
	int tStartFrames = frames;
	if (pOwner->PerfType == 0) pOwner->PerfTrack.StartDelta();
#endif	// _SRSCFG_PERFTRACK
	
	int i;
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
	int32_t* pSampsIn = pSamples;
	int32_t* pSampsOut = pSamples;
#else
    int16_t* pSampsIn = pSamples;
	int16_t* pSampsOut = pSamples;
#endif
	
	int tCacheBytes = SRS_Spool_GetCachePageSize(pCacheSpool);
		
	if (UseCache){		// Using the ring buffer - _never_ changes once the source is active...
		while (frames > 0) {
			int tUse = frames;
			int tPend = 256-CachePos;
			if (tUse > tPend) tUse = tPend;
			
			SRS_Spool_GetCachePtrs(pCacheSpool, (void**)&pCacheData, (void**)&pCacheTarget);
			
			SRSSamp* pCInL = pCacheData+CachePos;
			SRSSamp* pCInR = pCInL+256;
			SRSSamp* pCOutL = pCacheTarget+CachePos;
			SRSSamp* pCOutR = pCOutL+256;
	
			for (i=0; i<tUse; i++){				// Shift In and Out at the same time (ARM opt this?)

#ifdef _SRSCFG_ARCH_ARM
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pCInL++ = (*pSampsIn++>>8)*ActiveGain;
				*pCInR++ = (*pSampsIn++>>8)*ActiveGain;
				*pSampsOut++ = *pCOutL++;
				*pSampsOut++ = *pCOutR++;
#else
                *pCInL++ = *pSampsIn++*ActiveGain;
				*pCInR++ = *pSampsIn++*ActiveGain;
				*pSampsOut++ = *pCOutL++>>16;
				*pSampsOut++ = *pCOutR++>>16;
#endif
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pCInL++ = ((float)*pSampsIn++/2147483648.0f)*ActiveGain;	// 2^31
				*pCInR++ = ((float)*pSampsIn++/2147483648.0f)*ActiveGain;
				*pSampsOut++ = (int32_t)(*pCOutL++*2147483648.0f);
				*pSampsOut++ = (int32_t)(*pCOutR++*2147483648.0f);
#else
                *pCInL++ = ((float)*pSampsIn++/32768.0f)*ActiveGain;
				*pCInR++ = ((float)*pSampsIn++/32768.0f)*ActiveGain;
				*pSampsOut++ = (int16_t)(*pCOutL++*32767.0f);
				*pSampsOut++ = (int16_t)(*pCOutR++*32767.0f);
#endif
#endif	// _SRSCFG_ARCH_X86

			}
	
			frames -= tUse;
			CachePos += tUse;
	
			if (CachePos == 256){
				SubProcess();
				SRS_Spool_UpdateCachePtrs(pCacheSpool, pCacheTarget, pCacheData);	// Final data becomes 'output' next time...
				CachePos = 0;
			}
		}
	} else {
		while (frames > 0){
			int tUse = 256;		// 256 always, otherwise we'd cache...
			if (frames < tUse) break;	// Shouldn't even be possible - but safety first...
			
			SRS_Spool_GetCachePtrs(pCacheSpool, (void**)&pCacheData, (void**)&pCacheTarget);
			
			SRSSamp* pInL = pCacheData;
			SRSSamp* pInR = pInL+256;
			for (i=0; i<tUse; i++){

#ifdef _SRSCFG_ARCH_ARM
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pInL++ = (*pSampsIn++>>8)*ActiveGain;
				*pInR++ = (*pSampsIn++>>8)*ActiveGain;
#else
                *pInL++ = *pSampsIn++*ActiveGain;
				*pInR++ = *pSampsIn++*ActiveGain; 
#endif
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pInL++ = ((float)*pSampsIn++/2147483648.0f)*ActiveGain;
				*pInR++ = ((float)*pSampsIn++/2147483648.0f)*ActiveGain;
#else
                *pInL++ = ((float)*pSampsIn++/32768.0f)*ActiveGain;
				*pInR++ = ((float)*pSampsIn++/32768.0f)*ActiveGain;
#endif
#endif	// _SRSCFG_ARCH_X86
				
			}
			
			SubProcess();
			SRS_Spool_UpdateCachePtrs(pCacheSpool, pCacheTarget, pCacheData);	// Final data becomes output...
			
			SRSSamp* pOutL = pCacheData;
			SRSSamp* pOutR = pOutL+256;
			for (i=0; i<tUse; i++){

#ifdef _SRSCFG_ARCH_ARM
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pSampsOut++ = *pOutL++;
				*pSampsOut++ = *pOutR++;
#else
                *pSampsOut++ = *pOutL++>>16;
				*pSampsOut++ = *pOutR++>>16;
#endif
#endif	// _SRSCFG_ARCH_ARM

#ifdef _SRSCFG_ARCH_X86
#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
				*pSampsOut++ = (int32_t)(*pOutL++*2147483648.0f);
				*pSampsOut++ = (int32_t)(*pOutR++*2147483648.0f);
#else
                *pSampsOut++ = (int16_t)(*pOutL++*32767.0f);
				*pSampsOut++ = (int16_t)(*pOutR++*32767.0f);
#endif
#endif	// _SRSCFG_ARCH_X86

			}
			frames -= tUse;
		}
	}
	
#ifdef _SRSCFG_PERFTRACK
	if (pOwner->PerfType == 0) pOwner->PerfTrack.EndDelta(tStartFrames);
#endif	// _SRSCFG_PERFTRACK
}

void SRS_Source_Out::InitCaching(){
	UseCache = true;
	SRS_Spool_ClearCache(pCacheSpool);
	CachePos = 0;
}

void SRS_Source_Out::SwapCaching(){
	SRSSamp* pCacheSwap = pCacheData;
	pCacheData = pCacheTarget;
	pCacheTarget = pCacheSwap;
}

void SRS_Source_Out::SubProcess(){
	// WOW HD
	if (WHDState.PreFade(pCacheData) == true) WHDConfigState();
	if (WHDState.InPath){ SRS_Process_WOWHD_256(pWOWHD, pCacheData, pCacheTarget); SwapCaching(); }
	if (WHDState.PostFade(pCacheData) == true) WHDConfigState();
	
	// CSHP
	if (CSHPState.PreFade(pCacheData) == true) CSHPConfigState();
	if (CSHPState.InPath) SRS_Process_CSHP_256(pCSHP, pCacheData);
	if (CSHPState.PostFade(pCacheData) == true) CSHPConfigState(); 
#ifdef _SRSCFG_USERPEQ
	if (UPEQState.PreFade(pCacheData) == true) UPEQConfigState();
	if (UPEQState.InPath) SRS_Process_TruEQ_256(pUPEQ, pCacheData);
	if (UPEQState.PostFade(pCacheData) == true) UPEQConfigState();
#endif	// _SRSCFG_USERPEQ
		
#ifndef _SRSCFG_WOWHDX
	if (HPFPath) SRS_Process_HiPass_256(pHPF, pCacheData);
#endif	// _SRSCFG_WOWHDX

#ifdef _SRSCFG_MOBILE_EQ
	// GEQ
	if (GEQState.PreFade(pCacheData) == true) GEQConfigState();
	if (GEQState.InPath) SRS_Process_GEQ_256(pGEQ, pCacheData);
	if (GEQState.PostFade(pCacheData) == true) GEQConfigState();
#endif	// _SRSCFG_MOBILE_EQ

	if (TruEQPath) SRS_Process_TruEQ_256(pTruEQ, pCacheData);	
	if (HLimitPath){ SRS_Process_HLimit_256(pHLimit, pCacheData, pCacheTarget); SwapCaching(); }
	
	DCState.Process_256(pCacheData);	// Last-step: DC Reject filter
}

SRS_Workspace::SRS_Workspace(){
	Skip = 0;
	
	TMEnable = false;
	TMPreset = 0;
	
#ifdef _SRSCFG_WOWHDX
	TMIGains[0] = 0.25f;
	TMIGains[1] = 0.25f;
#else	// _SRSCFG_WOWHDX
	TMIGains[0] = 1.0f;
	TMIGains[1] = 1.0f;
#endif	// _SRSCFG_WOWHDX
		
	TMSkip = false;
	
	unsigned int i;
	int j;
	
#ifdef _SRSCFG_MOBILE_EQ
	memset(GEQEnables, 0, sizeof(GEQEnables));
	memset(GEQLimits, 0, sizeof(GEQLimits));
	memset(GEQPresets, 0, sizeof(GEQPresets));
#endif	// _SRSCFG_MOBILE_EQ
	
	VolIntEnable = false;
	
#ifdef _SRSCFG_PERFTRACK
	PerfType = -1;
#endif	// _SRSCFG_PERFTRACK
	
#ifndef _SRSCFG_WOWHDX
	SRS_Default_HiPass(&HPFTuning);
#endif	// _SRSCFG_WOWHDX

	unsigned int tBCount = sizeof(gSRS_Banks)/sizeof(SRS_ParamBank);	
	for (i=0; i<(sizeof(WHDTunings)/sizeof(WHDTunings[0])); i++){
		const char* pBN = NULL;
		
		for (j=0; j<(int)tBCount; j++){
			SRS_ParamBank* pB = gSRS_Banks+j;
			if (pB->EnumID != BANK_WOWHD) continue;
			if (pB->Index == (int)i){
				pBN = pB->pPrefix;
				break;
			}
		}
		
		SRS_Default_WOWHD(WHDTunings+i, pBN);
	}
	
	SRS_Default_CSHP(&CSHPTuning);
	
	for (i=0; i<BID_TruEQ_Max; i++) SRS_Default_TruEQ(TEQTunings+i);
	for (i=0; i<BID_HLim_Max; i++){
		if ((i == BID_Int_Mus_HLimB) || (i == BID_Int_Mov_HLimB) || (i == BID_Int_Pod_HLimB) || (i == BID_Int_Byp_HLimB)) SRS_Default_HLimit(HLTunings+i, true);
		else SRS_Default_HLimit(HLTunings+i, false);
	}
	
#ifdef _SRSCFG_USERPEQ
	SRS_Default_UserPEQ_CFG(&UPEQCFG);
	for (i=0; i<(sizeof(UPEQInts)/sizeof(UPEQInts[0])); i++) SRS_Default_UserPEQ_Preset(UPEQInts+i);
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
	for (i=0; i<(sizeof(GEQInts)/sizeof(GEQInts[0])); i++) SRS_Default_GEQ(GEQInts+i);
	for (i=0; i<(sizeof(GEQExts)/sizeof(GEQExts[0])); i++) SRS_Default_GEQ(GEQExts+i);
#endif	// _SRSCFG_MOBILE_EQ

#ifdef SRS_AUDIOLOG
	AL_Init();
#endif	// SRS_AUDIOLOG

#ifdef _SRSCFG_PERFTRACK
	SRS_LOG("[SRS_RAMSIZE] Tech_WS: %d", sizeof(SRS_Workspace));
#endif	//_SRSCFG_PERFTRACK
}

SRS_Workspace::~SRS_Workspace(){
#ifdef SRS_AUDIOLOG
	AL_Exit();
#endif	// SRS_AUDIOLOG
}

#ifndef _SRSCFG_DSPOFFLOAD_PATH
void SRS_Workspace::DSPOffload_Send(SRS_Source_Out* pOut){(void)pOut;}
void SRS_Workspace::DSPOffload_Clear(SRS_Source_Out* pOut){(void)pOut;}
#endif // _SRSCFG_DSPOFFLOAD_PATH

void SRS_Workspace::ApplyUserDefaults(){
	unsigned int i;
	for (i=0; i<(sizeof(WHDTunings)/sizeof(WHDTunings[0])); i++) SRS_UserDefault_WOWHD(WHDTunings+i);
	SRS_UserDefault_CSHP(&CSHPTuning);
	CFGSig++;
}

SRS_ParamBlock* SRS_Workspace::GetParamBlock(){
	if ((gSRS_Params.ConfigFlags&SRS_PBFS_FILLED) == 0){
		FillParamBlock(&gSRS_Params);
		gSRS_Params.FillPreCalcs();
	}
		
	return &gSRS_Params;
}

void SRS_Workspace::FillParamBlock(SRS_ParamBlock* pPB){
	char* pWork = gInfo_Scratch;
	
	int i;
	for (i=0; i<pPB->BankCount; i++){
		SRS_ParamBank* pB = pPB->pBanks+i;
		switch (pB->EnumID){
#ifndef _SRSCFG_WOWHDX
		case BANK_HPF: pB->pParams = SRS_GetBank_HiPass(pB->ParamCount); break;
#endif	// _SRSCFG_WOWHDX
		case BANK_WOWHD: pB->pParams = SRS_GetBank_WOWHD(pB->ParamCount); break;
		case BANK_CSHP: pB->pParams = SRS_GetBank_CSHP(pB->ParamCount); break;
		case BANK_TruEQ: pB->pParams = SRS_GetBank_TruEQ(pB->ParamCount); break;
		case BANK_HLimit: pB->pParams = SRS_GetBank_HLimit(pB->ParamCount); break;
		
#ifdef _SRSCFG_USERPEQ
		case BANK_UPEQCFG: pB->pParams = SRS_GetBank_UserPEQ_CFG(pB->ParamCount); break;
		case BANK_UPEQ: pB->pParams = SRS_GetBank_UserPEQ_Preset(pB->ParamCount); break;
#endif	// _SRSCFG_USERPEQ
		
#ifdef _SRSCFG_MOBILE_EQ
		case BANK_GEQ: pB->pParams = SRS_GetBank_GEQ(pB->ParamCount); break;
#endif	// _SRSCFG_MOBILE_EQ
		}
	}
	
	SRS_Param* pCFG = gCFG_Params;
	int nCFG = sizeof(gCFG_Params)/sizeof(SRS_Param);
	
	for (i=0; i<nCFG; i++){
		SRS_Param* pC = pCFG+i;
		
#ifndef _SRSCFG_WOWHDX
		if (pC->EnumID == CFG_HPF_Vers){
			pC->pInfo = SRS_GetVersion_HiPass(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}
#endif	// _SRSCFG_WOWHDX
						
		if (pC->EnumID == CFG_WHD_Vers){
			pC->pInfo = SRS_GetVersion_WOWHD(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}
		
		if (pC->EnumID == CFG_CSHP_Vers){
			pC->pInfo = SRS_GetVersion_CSHP(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}
		
		if (pC->EnumID == CFG_TruEQ_Vers){
			pC->pInfo = SRS_GetVersion_TruEQ(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}
		
		if (pC->EnumID == CFG_HLimit_Vers){
			pC->pInfo = SRS_GetVersion_HLimit(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}

#ifdef _SRSCFG_MOBILE_EQ
		if (pC->EnumID == CFG_GEQ_Vers){
			pC->pInfo = SRS_GetVersion_GEQ(pWork, sizeof(gInfo_Scratch));
			pWork += strlen(pC->pInfo)+1;
		}
#endif	// _SRSCFG_MOBILE_EQ
	}
}

void SRS_Workspace::SetParamValue(SRS_ParamBlock* pPB, int bank, int param, const char* pValue){
	SRS_ParamBank* pB = pPB->pBanks+bank;
	SRS_Param* pP = pB->pParams+param;
	
	HELP_ParamIn In;
	CFGSig++;
	
	if (pB->EnumID == BANK_WOWHD){
		SRS_SetParam_WOWHD(WHDTunings+pB->Index, pP, pValue);
	}
	
#ifndef _SRSCFG_WOWHDX
	else if (pB->EnumID == BANK_HPF){
		SRS_SetParam_HiPass(&HPFTuning, pP, pValue);
	}
#endif	// _SRSCFG_WOWHDX
	
	else if (pB->EnumID == BANK_CSHP){
		SRS_SetParam_CSHP(&CSHPTuning, pP, pValue);
	}
	
	else if (pB->EnumID == BANK_TruEQ){
		SRS_SetParam_TruEQ(TEQTunings+pB->Index, pP, pValue);
	}
	
	else if (pB->EnumID == BANK_HLimit){
		SRS_SetParam_HLimit(HLTunings+pB->Index, pP, pValue);
	}
	
#ifdef _SRSCFG_USERPEQ
	else if (pB->EnumID == BANK_UPEQCFG){
		SRS_SetParam_UserPEQ_CFG(&UPEQCFG, pP, pValue);
	}
	
	else if (pB->EnumID == BANK_UPEQ){
		SRS_SetParam_UserPEQ_Preset(UPEQInts+pB->Index, pP, pValue);
	}
#endif	// _SRSCFG_USERPEQ
	
#ifdef _SRSCFG_MOBILE_EQ
	else if (pB->EnumID == BANK_GEQ){
		int tIdx = pB->Index;
		SRS_Tech_GEQ* pG = NULL;
		if (tIdx >= 10) pG = GEQExts+(tIdx-10);	// Exts
		else pG = GEQInts+tIdx;
		
		SRS_SetParam_GEQ(pG, pP, pValue);
	}
#endif	// _SRSCFG_MOBILE_EQ

	else if (pB->EnumID == BANK_Routes){
		if (pP->EnumID < 0) return;
		RouteMap.RouteMapSet(pP->EnumID, pValue);
	}
	
	else if (pB->EnumID == BANK_CFG){
		if (pP->EnumID < 0) return;
		
		switch (pP->EnumID){
		case CFG_Skip: Skip = In.GetBool(pValue); break;
		case CFG_TMEnable: TMEnable = In.GetBool(pValue); break;
		case CFG_TMPreset: TMPreset = In.GetInt(pValue); break;
		
		case CFG_TMIGain_Int: TMIGains[0] = In.GetFloat(pValue); break;
		case CFG_TMIGain_Ext: TMIGains[1] = In.GetFloat(pValue); break;
		
		case CFG_TMSkip: TMSkip = In.GetBool(pValue); break;

		case CFG_VolIntEnable: VolIntEnable = In.GetBool(pValue); break;
		
#ifdef _SRSCFG_PERFTRACK
		case CFG_PerfType: {
			int tPT = In.GetInt(pValue);
			if (tPT != PerfType){
				CFGSig--;	// Undo change - can't scan CFG and perf at the same time - would skew values...
				
				if (PerfType >= 0){	// Already running?
					char tHold[32];
					sprintf(tHold, "PERF %d", PerfType);
					PerfTrack.LogTiming(tHold);
					PerfTrack.EndTiming();
				}
				
				PerfType = tPT;
				
				if (PerfType >= 0){	// Now running?
					PerfTrack.StartTiming();
				}
			}
		} break;
#endif	// _SRSCFG_PERFTRACK

		}
	}
	
#ifdef _SRSCFG_MOBILE_EQ	
	else if (pB->EnumID == BANK_EQCFG){
		int tBlock = (pP->EnumID/100)+pB->Index;
		int tBSub = pP->EnumID%100;
		
		if (tBSub == 0) GEQEnables[tBlock] = In.GetBool(pValue);
		if (tBSub == 1) GEQLimits[tBlock] = In.GetBool(pValue);
		if (tBSub == 2) GEQPresets[tBlock] = In.GetInt(pValue);
	}
#endif	// _SRSCFG_MOBILE_EQ
}

const char* SRS_Workspace::GetParamValue(SRS_ParamBlock* pPB, int bank, int param){
	SRS_ParamBank* pB = pPB->pBanks+bank;
	SRS_Param* pP = pB->pParams+param;
	
	HELP_ParamOut Out;
	
	if (pB->EnumID == BANK_WOWHD){
		return SRS_GetParam_WOWHD(WHDTunings+pB->Index, pP);
	}
	
#ifndef _SRSCFG_WOWHDX
	else if (pB->EnumID == BANK_HPF){
		return SRS_GetParam_HiPass(&HPFTuning, pP);
	}
#endif	// _SRSCFG_WOWHDX
	
	else if (pB->EnumID == BANK_CSHP){
		return SRS_GetParam_CSHP(&CSHPTuning, pP);
	}
		
	else if (pB->EnumID == BANK_TruEQ){
		return SRS_GetParam_TruEQ(TEQTunings+pB->Index, pP);
	}
	
	else if (pB->EnumID == BANK_HLimit){
		return SRS_GetParam_HLimit(HLTunings+pB->Index, pP);
	}
	
#ifdef _SRSCFG_USERPEQ
	else if (pB->EnumID == BANK_UPEQCFG){
		return SRS_GetParam_UserPEQ_CFG(&UPEQCFG, pP);
	}
	
	else if (pB->EnumID == BANK_UPEQ){
		return SRS_GetParam_UserPEQ_Preset(UPEQInts+pB->Index, pP);
	}
#endif	// _SRSCFG_USERPEQ

#ifdef _SRSCFG_MOBILE_EQ
	else if (pB->EnumID == BANK_GEQ){
		int tIdx = pB->Index;
		SRS_Tech_GEQ* pG = NULL;
		if (tIdx >= 10) pG = GEQExts+(tIdx-10);	// Exts
		else pG = GEQInts+tIdx;
		
		return SRS_GetParam_GEQ(pG, pP);
	}
#endif	// _SRSCFG_MOBILE_EQ

	else if (pB->EnumID == BANK_Routes){
		if (pP->EnumID < 0) return pP->pInfo;
		return RouteMap.RouteMapGet(pP->EnumID);
	}
	
	else if (pB->EnumID == BANK_CFG){
		if (pP->EnumID < 0) return pP->pInfo;
		
		switch (pP->EnumID){
		case CFG_Skip: return Out.FromBool(Skip);
		case CFG_TMEnable: return Out.FromBool(TMEnable);
		case CFG_TMPreset: return Out.FromInt(TMPreset);
		
		case CFG_TMIGain_Int: return Out.FromFloat(TMIGains[0]);
		case CFG_TMIGain_Ext: return Out.FromFloat(TMIGains[1]);
		
		case CFG_TMSkip: return Out.FromInt(TMSkip);
		
		case CFG_VolIntEnable: return Out.FromBool(VolIntEnable);
		
#ifdef _SRSCFG_PERFTRACK
		case CFG_PerfType: return Out.FromInt(PerfType);
#endif	// _SRSCFG_PERFTRACK
		}
	}

#ifdef _SRSCFG_MOBILE_EQ	
	else if (pB->EnumID == BANK_EQCFG){
		int tBlock = (pP->EnumID/100)+pB->Index;
		int tBSub = pP->EnumID%100;
		
		if (tBSub == 0) return Out.FromBool(GEQEnables[tBlock]);
		if (tBSub == 1) return Out.FromBool(GEQLimits[tBlock]);
		if (tBSub == 2) return Out.FromInt(GEQPresets[tBlock]);
	}
#endif	// _SRSCFG_MOBILE_EQ
	
	return "";
}

void SRS_Workspace::LoadBaseConfigs(){
	SRS_ParamBlock* pPB = GetParamBlock();
	ConfigRead(SRS_STR(SRS_BASECFG_READPATH), pPB);	// One-time load of settings (likely from a read-only source)
	LoadConfigs();	// Attempt override with user-settings...
}

void SRS_Workspace::LoadConfigs(){
#ifdef SRS_USERCFG_ALLOW
	SRS_ParamBlock* pPB = GetParamBlock();
	
	uint32_t typeMask = SRS_PTFLAG_PREF;	// Preferences only!!!
	
#ifdef SRS_USERCFG_UNLOCKED
	typeMask = 0xFFFFFFFF;
#endif
	
	ConfigRead(SRS_STR(SRS_USERCFG_PATH), pPB, typeMask);
#endif
}

bool SRS_Workspace::ReadUserConfig(const char* pPath){
#ifdef SRS_USERCFG_ALLOW
	if (pPath == NULL) pPath = SRS_STR(SRS_USERCFG_PATH);
	
	SRS_ParamBlock* pPB = GetParamBlock();
	
	uint32_t typeMask = SRS_PTFLAG_PREF;	// Preferences only!!!
	
#ifdef SRS_USERCFG_UNLOCKED
	typeMask = 0xFFFFFFFF;
#endif
	
	return ConfigRead(pPath, pPB, typeMask);
#else
	return false;
#endif
}

void SRS_Workspace::WriteUserConfig(const char* pPath){
#ifdef SRS_USERCFG_ALLOW
	if (pPath == NULL) pPath = SRS_STR(SRS_USERCFG_PATH);
	
	SRS_ParamBlock* pPB = GetParamBlock();
	
	uint32_t typeMask = SRS_PTFLAG_PREF;	// Preferences only!!!
	
#ifdef SRS_USERCFG_UNLOCKED
	typeMask = 0xFFFFFFFF;
#endif
	
	ConfigWrite(pPath, pPB, typeMask);
#endif
}

static void gSRS_SetParam(SRS_ParamBlock* pPB, SRS_ParamSource* pSrc, int bank, int param, const char* pValue){
	SRS_Workspace* pWS = (SRS_Workspace*)pSrc->pSourceToken;
	pWS->SetParamValue(pPB, bank, param, pValue);
}

static const char* gSRS_GetParam(SRS_ParamBlock* pPB, SRS_ParamSource* pSrc, int bank, int param){
	SRS_Workspace* pWS = (SRS_Workspace*)pSrc->pSourceToken;
	return pWS->GetParamValue(pPB, bank, param);
}

bool SRS_Workspace::ConfigRead(const char* pPath, SRS_ParamBlock* pBlock, uint32_t typeMask){
	SRS_ParamSource tParamIO;
	tParamIO.pSourceToken = this;
	tParamIO.SetParam = gSRS_SetParam;
	tParamIO.GetParam = gSRS_GetParam;
	
	return pBlock->ConfigRead(pPath, &tParamIO, typeMask);
}

void SRS_Workspace::ConfigWrite(const char* pPath, SRS_ParamBlock* pBlock, uint32_t typeMask){
	SRS_ParamSource tParamIO;
	tParamIO.pSourceToken = this;
	tParamIO.SetParam = gSRS_SetParam;
	tParamIO.GetParam = gSRS_GetParam;
	
	pBlock->ConfigWrite(pPath, &tParamIO, typeMask);
}

#ifdef SRS_AUDIOLOG
	
void SRS_Workspace::AL_Init(){
	AL_Active = 0;
	AL_Status = 0;
	pAL_OutPre = NULL;
	pAL_OutPost = NULL;
}

void SRS_Workspace::AL_Exit(){
	if (pAL_OutPre != NULL) fclose(pAL_OutPre);
	pAL_OutPre = NULL;
	if (pAL_OutPost != NULL) fclose(pAL_OutPost);
	pAL_OutPost = NULL;
}
	
void SRS_Workspace::AL_Start(){
	AL_Active = 1;
}

void SRS_Workspace::AL_Stop(){
	AL_Active = 0;
}

void SRS_Workspace::AL_PreProc(void* pSamples, int sampleBytes){
	if (AL_Active != AL_Status){	// Change in logging status...
		AL_Status = AL_Active;
		if (AL_Status == 0){
			if (pAL_OutPre != NULL) fclose(pAL_OutPre);
			pAL_OutPre = NULL;
			if (pAL_OutPost != NULL) fclose(pAL_OutPost);
			pAL_OutPost = NULL;
		}
		
		if (AL_Status == 1){
			pAL_OutPre = fopen(SRS_STR(SRS_AUDIOLOG_PREPATH), "wb");
			pAL_OutPost = fopen(SRS_STR(SRS_AUDIOLOG_POSTPATH), "wb");
		}
	}
	
	if ((AL_Status == 1) && (pAL_OutPre != NULL)){
		fwrite(pSamples, 1, sampleBytes, pAL_OutPre);
	}
}

void SRS_Workspace::AL_PostProc(void* pSamples, int sampleBytes){
	if ((AL_Status == 1) && (pAL_OutPost != NULL)){
		fwrite(pSamples, 1, sampleBytes, pAL_OutPost);
	}
}

#endif

};

