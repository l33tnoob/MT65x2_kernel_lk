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

#undef LOG_TAG
#define LOG_TAG "SRS_Routing"

namespace android {
	
SRS_Param gRoute_Params[] = {
	{ -1,		SRS_PTYP_INFO,	SRS_PFMT_STATIC,	0.0f,	0.0f,	1.0f,	"routing_info", "Routing Values", "0 = int, 1 = ext, -1 = disallow srs processing, -2 = int dsp, -3 = ext dsp", "", 0},
	{ 1000,		SRS_PTYP_CFG,	SRS_PFMT_INT,		-1.0f,	-16.0f,	16.0f,	"force_route", "Hard Lock Routing to a specific setting", "", "", 0},
	{ 0,		SRS_PTYP_CFG,	SRS_PFMT_INT,		0.0f,	0.0f,	16.0f,	"earpiece", "DEVICE_OUT_EARPIECE", "", "", 0},
	{ 1,		SRS_PTYP_CFG,	SRS_PFMT_INT,		0.0f,	0.0f,	16.0f,	"speaker", "DEVICE_OUT_SPEAKER", "", "", 0},
	{ 2,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"wired_headset", "DEVICE_OUT_WIRED_HEADSET", "", "", 0},
	{ 3,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"wired_headphone", "DEVICE_OUT_WIRED_HEADPHONE", "", "", 0},
	{ 4,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_sco", "DEVICE_OUT_BLUETOOTH_SCO", "", "", 0},
	{ 5,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_sco_headset", "DEVICE_OUT_BLUETOOTH_SCO_HEADSET", "", "", 0},
	{ 6,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_sco_carkit", "DEVICE_OUT_BLUETOOTH_SCO_CARKIT", "", "", 0},
	{ 7,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_a2dp", "DEVICE_OUT_BLUETOOTH_A2DP", "", "", 0},
	{ 8,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_a2dp_headphones", "DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES", "", "", 0},
	{ 9,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"bt_a2dp_speaker", "DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER", "", "", 0},
	{ 10,		SRS_PTYP_CFG,	SRS_PFMT_INT,		1.0f,	0.0f,	16.0f,	"aux_digital", "DEVICE_OUT_AUX_DIGITAL", "", "", 0},
};

SRS_RouteMap::SRS_RouteMap(){
	unsigned int i;

	for (i=0; i<SRS_ROUTEMAP_MAXROUTES; i++){
		if (i < 2) RouteTable[i] = 0;
		else RouteTable[i] = 1;
	}
	
	ForceRoute = -1;
}

SRS_Param* SRS_RouteMap::RouteParams(){ return gRoute_Params; }
int SRS_RouteMap::RouteParamCount(){ return sizeof(gRoute_Params)/sizeof(SRS_Param); }

void SRS_RouteMap::RouteMapSet(int index, const char* pValue){
	HELP_ParamIn In;
	
	if (index < 0) return;
	if (index >= SRS_ROUTEMAP_MAXROUTES){
		if (index == 1000) ForceRoute = In.GetInt(pValue);
		return;
	}
	
	
	RouteTable[index] = In.GetInt(pValue);
}

const char* SRS_RouteMap::RouteMapGet(int index){
	HELP_ParamOut Out;
	
	if (index < 0) return "";
	if (index >= SRS_ROUTEMAP_MAXROUTES){
		if (index == 1000) return Out.FromInt(ForceRoute);
		return "";
	}
	
	return Out.FromInt(RouteTable[index]);
}
	
int SRS_RouteMap::ResolveRoute(int routeFlags, int* pFoundFlags){
	int tRetRoute = 0;
	int tRoute = routeFlags;
	
	// Ignore Route - and scan for the active device...
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET; LOGW("BT SCO HEAD"); }
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT; LOGW("BT SCO CAR"); }
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_SCO, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_SCO; LOGW("BT SCO"); }
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES; LOGW("BT A2DP HP"); }
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER; LOGW("BT A2DP SPK"); }
	if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_BLUETOOTH_A2DP, "") != AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE){ tRoute = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP; LOGW("BT A2DP"); }
	
	// Ugly, but the only 'right' way to do it.
	if (tRoute > 0){
		if (tRoute&AUDIO_DEVICE_OUT_EARPIECE) tRetRoute = RouteTable[0];
		if (tRoute&AUDIO_DEVICE_OUT_SPEAKER) tRetRoute = RouteTable[1];
		if (tRoute&AUDIO_DEVICE_OUT_WIRED_HEADSET) tRetRoute = RouteTable[2];
		if (tRoute&AUDIO_DEVICE_OUT_WIRED_HEADPHONE) tRetRoute = RouteTable[3];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_SCO) tRetRoute = RouteTable[4];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) tRetRoute = RouteTable[5];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) tRetRoute = RouteTable[6];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_A2DP) tRetRoute = RouteTable[7];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES) tRetRoute = RouteTable[8];
		if (tRoute&AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER) tRetRoute = RouteTable[9];
		if (tRoute&AUDIO_DEVICE_OUT_AUX_DIGITAL) tRetRoute = RouteTable[10];
	} else {
		tRetRoute = -1;
	}
	
	if (ForceRoute != -1){
		LOGW("Route Forced To %d", ForceRoute);
		tRoute = ForceRoute;
	}
	
	if (pFoundFlags != NULL) *pFoundFlags = tRoute;
	
	return tRetRoute;
}

};	// namespace android

