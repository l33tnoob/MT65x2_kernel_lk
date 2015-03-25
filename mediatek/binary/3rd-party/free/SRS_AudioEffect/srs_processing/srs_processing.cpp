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
#include "cutils/properties.h"

#include <media/AudioSystem.h>
#include <media/AudioParameter.h>

#include "srs_types.h"
#include "srs_processing.h"
#include "srs_params.h"
#include "srs_workspace.h"
#include "srs_tech.h"

#undef LOG_TAG
#define LOG_TAG "SRS_Proc"

namespace android {
	
// New build.prop testing routines

#ifdef POSTPRO_PROPGATE

static int gSRS_AllowOutput = -1;
static int gSRS_AllowInput = -1;

void SRS_PullAllows(){
	char buf[PROPERTY_VALUE_MAX];
	const char* pInKey = "audio.srs.input";
	const char* pOutKey = "audio.srs.output";
		
	int len = property_get(pInKey, buf, "");
	if (len <= 0) gSRS_AllowInput = 1;	// No prop?  Allow.
	else if (buf[0] == '1') gSRS_AllowInput = 1;	// Prop?  1?  Allow.
	else gSRS_AllowInput = 0;	// Prop?  Anything but 1?  Disallow.
	
	len = property_get(pOutKey, buf, "");
	if (len <= 0) gSRS_AllowOutput = 1;	// No prop?  Allow.
	else if (buf[0] == '1') gSRS_AllowOutput = 1;	// Prop?  1?  Allow.
	else gSRS_AllowOutput = 0;	// Prop?  Anything but 1?  Disallow.
}

bool SRS_DoOutput(){
	if (gSRS_AllowOutput < 0) SRS_PullAllows();
	return (gSRS_AllowOutput > 0)?true:false;
}

bool SRS_DoInput(){
	if (gSRS_AllowInput < 0) SRS_PullAllows();
	return (gSRS_AllowInput > 0)?true:false;
}

bool SRS_DoAny(){
	if ((SRS_DoOutput() == true) || (SRS_DoInput() == true)) return true;
	return false;
}

#endif

const int SRS_Processing::AUTO = -1;

int SRS_Processing::CreateHandle(){
	return SRS_Base_Workspace::CreateWS();
}

void SRS_Processing::DestroyHandle(int handle){
	SRS_Base_Workspace::DeleteWS(handle);
}

void SRS_Processing::ProcessOutNotify(int handle, void* pSource, bool init){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;
	
	if (init) pWS->SourceOutAdd(pSource);
	else pWS->SourceOutDel(pSource);
	
	SRS_ParamBlock* pPB = pWS->GetParamBlock();
	pPB->GetParamCache();
}

void SRS_Processing::ProcessOutRoute(int handle, void* pSource, int device){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;
	
	SRS_Source_Out* pSrc = pWS->SourceOutFind(pSource);
	if (pSrc != NULL) pSrc->SetRoute(device);
}

#ifdef SRS_AUDIOLOG
#ifdef SRS_AUDIOLOG_MARKERS
unsigned long long gTrigTime = 0;
#endif	// SRS_AUDIOLOG_MARKERS
#endif	// SRS_AUDIOLOG

void SRS_Processing::ProcessOut(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;

	SRS_Source_Out* pSrc = pWS->SourceOutFind(pSource);
	if (pSrc == NULL) return;
	
#ifdef SRS_AUDIOLOG
#ifdef SRS_AUDIOLOG_MARKERS
	unsigned long long nowTime = SRS_Perf::GetRelativeTimeNsec();
	unsigned long long difTime = nowTime-gTrigTime;
	gTrigTime = nowTime;
	
	if (difTime > 2000000000){	// 2 seconds of sleep!
		short sessMarker[] = { 0, 4096, 0, 2048, 0, 1024, 0, 0, 0, -1024, 0, -2048, 0, -4096 };
		pWS->AL_PreProc(sessMarker, sizeof(sessMarker));
		pWS->AL_PostProc(sessMarker, sizeof(sessMarker));
	}
#endif	// SRS_AUDIOLOG_MARKERS

	pWS->AL_PreProc(pSamples, sampleBytes);
#endif	// SRS_AUDIOLOG
	
	pSrc->Process(pSamples, sampleBytes, sampleRate, countChans);
	
#ifdef SRS_AUDIOLOG
	pWS->AL_PostProc(pSamples, sampleBytes);
#endif	// SRS_AUDIOLOG

#ifdef SRS_FORCE_SILENCE
	memset(pSamples, 0, sampleBytes);
#endif	// SRS_FORCE_SILENCE

}

void SRS_Processing::ProcessInNotify(int handle, void* pSource, bool init){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;
	
	if (init) pWS->SourceInAdd(pSource);
	else pWS->SourceInDel(pSource);
}

void SRS_Processing::ProcessInRoute(int handle, void* pSource, int device){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;
	
	SRS_Source_In* pSrc = pWS->SourceInFind(pSource);
	if (pSrc != NULL) pSrc->SetRoute(device);
}

void SRS_Processing::ProcessIn(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return;
	
	SRS_Source_In* pSrc = pWS->SourceInFind(pSource);
	if (pSrc == NULL) return;
	
	pSrc->Process(pSamples, sampleBytes, sampleRate, countChans);
}

#ifdef _SRSCFG_DSPOFFLOAD_PATH
	#define DSPOFFLOAD_RAWDATASET_HEAD
	#include SRS_STR(_SRSCFG_DSPOFFLOAD_PATH)
	#undef DSPOFFLOAD_RAWDATASET_HEAD
#endif // _SRSCFG_DSPOFFLOAD_PATH

void SRS_Processing::RawDataSet(int* pHandle, char* pKey, void* pData, int dataLen){
	SRS_Workspace* pWS = NULL;
	
	if (pHandle != NULL)
		pWS = SRS_Base_Workspace::GetWS(*pHandle, SRS_Processing::AUTO);
		
#ifdef _SRSCFG_DSPOFFLOAD_PATH
	#define DSPOFFLOAD_RAWDATASET_FUNC
	#include SRS_STR(_SRSCFG_DSPOFFLOAD_PATH)
	#undef DSPOFFLOAD_RAWDATASET_FUNC
#endif // _SRSCFG_DSPOFFLOAD_PATH
}

void SRS_Processing::ParamsSet(int handle, const String8& keyValues){
	ParamsSet_Notify(handle, keyValues);
}

bool SRS_Processing::ParamsSet_Notify(int handle, const String8& keyValues){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return false;
	
	SRS_ParamBlock* pPB = pWS->GetParamBlock();
	if (pPB == NULL) return false;
	
	AudioParameter param = AudioParameter(keyValues);
	String8 value;
	String8 key;
	
	int tHoldSIG = pWS->CFGSig;
    
	bool bPostSave = true;
	bool bDidSet = false;
	
	key = "bluetooth_enabled";
	if (param.get(key, value) == NO_ERROR){
		pWS->CFGSig++;
		return true;	// Do not process other keys!
	}
    
	key = "srs_processing_defersave";
	if (param.get(key, value) == NO_ERROR) bPostSave = false;	// Don't save as a result of change (useful for sliders, etc)
	
	key = "srs_processing_forcesave";
	if (param.get(key, value) == NO_ERROR){ bDidSet = true; bPostSave = true; }		// Save even without changes occuring (often used after a chain of defers)
	
	key = "srs_processing_loadcfg";						// Load from path in value, or stored path if empty
	if (param.get(key, value) == NO_ERROR){
		const char* pPath = value.string();
		pWS->ReadUserConfig(pPath);
	}
	
	key = "srs_processing_savecfg";						// save to path in value, or stored path if empty
	if (param.get(key, value) == NO_ERROR){
		const char* pPath = value.string();
		pWS->WriteUserConfig(pPath);
	}
	
	key = "srs_processing_userdefaults";
	if (param.get(key, value) == NO_ERROR){
		pWS->ApplyUserDefaults();
		bDidSet = true;
	}
	
#ifdef SRS_AUDIOLOG
	key = "srs_audiolog_start";
	if (param.get(key, value) == NO_ERROR) pWS->AL_Start();
	
	key = "srs_audiolog_stop";
	if (param.get(key, value) == NO_ERROR) pWS->AL_Stop();
#endif	// SRS_AUDIOLOG
	
    int bankId, paramId;
    
    size_t i;
    size_t tLen = param.size();
    for (i=0; i<tLen; i++){
    	param.getAt(i, key, value);
    	
    	SRS_Param* pP = pPB->FindParam(key.string(), bankId, paramId);
    	
    	if ((pP != NULL) && (pP->Type >= SRS_PTYP_CFG)){
    		
#ifdef SRS_PARAMWRITE_CFG_BLOCK
			if (pP->Type == SRS_PTYP_CFG) continue;
#endif	// SRS_PARAMWRITE_CFG_BLOCK

#ifdef SRS_PARAMWRITE_PREF_BLOCK
			if (pP->Type == SRS_PTYP_PREF) continue;
#endif	// SRS_PARAMWRITE_PREF_BLOCK
    		
    		pWS->SetParamValue(pPB, bankId, paramId, value.string());
    		bDidSet = true;		// Flag to indicate something may have changed...
    	}
    }
    
    if (bPostSave && bDidSet) pWS->WriteUserConfig(NULL);
    
    if (pWS->CFGSig != tHoldSIG) return true;
    return false;
}

const char* gBlockTypes = ""
#ifdef SRS_PARAMWRITE_CFG_BLOCK
	"write_cfg,"
#endif
#ifdef SRS_PARAMWRITE_PREF_BLOCK
	"write_pref,"
#endif
#ifdef SRS_PARAMREAD_INFO_BLOCK
	"read_info,"
#endif
#ifdef SRS_PARAMREAD_DEBUG_BLOCK
	"read_debug,"
#endif
#ifdef SRS_PARAMREAD_CFG_BLOCK
	"read_cfg,"
#endif
#ifdef SRS_PARAMREAD_PREF_BLOCK
	"read_pref,"
#endif
#ifdef SRS_USERCFG_ALLOW
	"allow_user,"
#endif
#ifdef SRS_USERCFG_UNLOCKED
	"unlock_user,"
#endif
#ifdef SRS_AUDIOLOG
	"audio_log,"
#endif	// SRS_AUDIOLOG
;	// End of Blocking String

char gBuildTags[] = { "INV - " SRS_STR(SRS_TAGS) };

String8 SRS_Processing::ParamsGet(int handle, const String8& keys){
	SRS_Workspace* pWS = SRS_Base_Workspace::GetWS(handle, SRS_Processing::AUTO);
	if (pWS == NULL) return String8("");
	
	SRS_ParamBlock* pPB = pWS->GetParamBlock();
	
	AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key;
    
    key = "srs_processing_params";
    if (param.get(key, value) == NO_ERROR){		// If we get this, we _only_ do this.
    	return String8(pPB->GetParamCache());
    }
    
    AudioParameter outParams = AudioParameter();
    int bankId, paramId;
    
    size_t i;
    
    key = "srs_processing_basecfg";
    if (param.get(key, value) == NO_ERROR){
		value = SRS_STR(SRS_BASECFG_READPATH);
		outParams.add(key, value);
    }
    
    key = "srs_processing_usercfg";
    if (param.get(key, value) == NO_ERROR){
		value = SRS_STR(SRS_USERCFG_PATH);
		outParams.add(key, value);
    }
    
    key = "srs_processing_tags";
    if (param.get(key, value) == NO_ERROR){
    	size_t tLen = strlen(gBuildTags);
		for (i=0; i<tLen; i++)
			if (gBuildTags[i] == '^') gBuildTags[i] = ',';	// Weird sep due to cmd-line define syntax...
			
		if (pWS->LicenseState == 1){ gBuildTags[0] = 'D'; gBuildTags[1] = 'M'; gBuildTags[2] = 'O'; }
		else if (pWS->LicenseState > 1){ gBuildTags[0] = 'R'; gBuildTags[1] = 'E'; gBuildTags[2] = 'T'; }
				
		value = (const char *)gBuildTags;
    	outParams.add(key, value);
    }
    
    key = "srs_processing_blocked";
    if (param.get(key, value) == NO_ERROR){
		value = gBlockTypes;
		outParams.add(key, value);
    }
    
    size_t tLen = param.size();
    for (i=0; i<tLen; i++){
    	param.getAt(i, key, value);
    	
    	SRS_Param* pP = pPB->FindParam(key.string(), bankId, paramId);
    	if (pP != NULL){
    		
#ifdef SRS_PARAMREAD_INFO_BLOCK
			if (pP->Type == SRS_PTYP_INFO) continue;
#endif

#ifdef SRS_PARAMREAD_DEBUG_BLOCK
			if (pP->Type == SRS_PTYP_DEBUG) continue;
#endif

#ifdef SRS_PARAMREAD_CFG_BLOCK
			if (pP->Type == SRS_PTYP_CFG) continue;
#endif

#ifdef SRS_PARAMREAD_PREF_BLOCK
			if (pP->Type == SRS_PTYP_PREF) continue;
#endif
    		
    		const char* pV = pWS->GetParamValue(pPB, bankId, paramId);
    		if (pV != NULL){
    			value = pV;
    			outParams.add(key, value);
    		}
    	}
    }
    
    return outParams.toString();
}

};	// namespace android

