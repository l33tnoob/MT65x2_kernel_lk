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
#include <time.h>
#include <utils/Log.h>
#include <utils/String8.h>

#include "srs_types.h"
#include "srs_workspace.h"
#include "srs_params.h"
#include "srs_tech.h"

#undef LOG_TAG
#define LOG_TAG "SRS_ProcWS"

namespace android {

SRS_Base_Source::SRS_Base_Source(){
	DidAPIInit = false;
	pSource = NULL;
	Route = -1;
	SampleRate = -1;
	ChannelCount = -1;
	pOwner = NULL;
	CFGSig = -1;
}

SRS_Base_Source::~SRS_Base_Source(){
	pSource = NULL;
}

SRS_Base_Workspace::SRS_Base_Workspace(){
	CFGSig = 0;
	Handle = -2;
	
#ifdef SRS_PROTECT_AUTHID
	SRS_License_SetInfo(SRS_PROTECT_AUTHID, SRS_STR(SRS_PROTECT_PATH));
#endif	// SRS_PROTECT_AUTHID
	
	pOutSpool = SRS_Spool_Create(SRS_WORKSOURCES);
	pInSpool = SRS_Spool_Create(SRS_WORKSOURCES);
	LicenseState = SRS_License_State();
	ActiveOut = 0;
	ActiveIn = 0;
	DidInit = 0;
}

SRS_Base_Workspace::~SRS_Base_Workspace(){
	SRS_Spool_Destroy(pOutSpool);
	SRS_Spool_Destroy(pInSpool);
	
	pOutSpool = NULL;
	pInSpool = NULL;
}

void SRS_Workspace::SourceOutAdd(void* pSource){
	void* pEntry = SRS_Spool_SourceFind(pOutSpool, pSource);
	if (pEntry != NULL) { SRS_LOG("SRS_Processing - SourceOutAdd - Dupe Source %p", pSource); return; }
	if (SRS_Spool_SourceAvail(pOutSpool) == false){ SRS_LOG("SRS_Processing - SourceOutAdd - No Available Slot for %p", pSource); return; }
	
	SRS_Source_Out* pSrc = new SRS_Source_Out();
	pSrc->pSource = pSource;
	pSrc->pOwner = (SRS_Workspace*)this;
	
	SRS_Spool_SourceAdd(pOutSpool, pSrc, pSource);
	ActiveOut++;
	
	CFGSig++;
	DoInit();
}

void SRS_Workspace::SourceOutDel(void* pSource){
	SRS_Source_Out* pSrc = SourceOutFind(pSource);
	if (pSrc == NULL){
		SRS_LOG("SRS_Processing - SourceOutDel - Source Not Located %p", pSource);
	}
	
	delete pSrc;
	
	if (SRS_Spool_SourceDel(pOutSpool, pSource) == false){
		SRS_LOG("SRS_Processing - SourceOutDel - Source Not Deleted %p", pSource);
		return;	
	}
	
	ActiveOut--;
	CFGSig++;
}

SRS_Source_Out* SRS_Workspace::SourceOutFind(void* pSource){
	return (SRS_Source_Out*) SRS_Spool_SourceFind(pOutSpool, pSource);
}

void SRS_Workspace::SourceInAdd(void* pSource){
	void* pEntry = SRS_Spool_SourceFind(pInSpool, pSource);
	if (pEntry != NULL) { SRS_LOG("SRS_Processing - SourceInAdd - Dupe Source %p", pSource); return; }
	if (SRS_Spool_SourceAvail(pInSpool) == false){ SRS_LOG("SRS_Processing - SourceInAdd - No Available Slot for %p", pSource); return; }
	
	SRS_Source_In* pSrc = new SRS_Source_In();
	pSrc->pSource = pSource;
	pSrc->pOwner = (SRS_Workspace*)this;
	
	SRS_Spool_SourceAdd(pInSpool, pSrc, pSource);
	ActiveIn++;
	
	CFGSig++;
	DoInit();
}

void SRS_Workspace::SourceInDel(void* pSource){
	SRS_Source_In* pSrc = SourceInFind(pSource);
	if (pSrc == NULL){
		SRS_LOG("SRS_Processing - SourceInDel - Source Not Located %p", pSource);
	}
	
	delete pSrc;
	
	if (SRS_Spool_SourceDel(pInSpool, pSource) == false){
		SRS_LOG("SRS_Processing - SourceInDel - Source Not Deleted %p", pSource);
		return;	
	}
	
	ActiveIn--;
	CFGSig++;
}

SRS_Source_In* SRS_Workspace::SourceInFind(void* pSource){
	return (SRS_Source_In*) SRS_Spool_SourceFind(pInSpool, pSource);
}

void SRS_Workspace::DoInit(){
	if (DidInit != 0) return;
	
	DidInit = 1;
	LoadBaseConfigs();
}

SRS_Workspace gAuto;

SRS_Workspace** SRS_Base_Workspace::pSW_Stack = NULL;
int SRS_Base_Workspace::pSW_StackSize = 0;

int SRS_Base_Workspace::CreateWS(){
	int handle = -1;
	int i;
	
	for (i=0; i<pSW_StackSize; i++){
		if (pSW_Stack[i] == NULL){
			handle = i;
			break;
		}
	}
	
	if (handle < 0){
		SRS_Workspace** pHold = new SRS_Workspace*[pSW_StackSize+1];
		if (pSW_StackSize > 0) memcpy(pHold, pSW_Stack, pSW_StackSize*sizeof(SRS_Workspace*));
		if (pSW_Stack != NULL){
			delete [] pSW_Stack;
			pSW_Stack = NULL;
		}
		pSW_Stack = pHold;
		handle = pSW_StackSize;
		pSW_StackSize++;
	}
	
	pSW_Stack[handle] = new SRS_Workspace;
	pSW_Stack[handle]->Handle = handle;
	
	return handle;
}

void SRS_Base_Workspace::DeleteWS(int handle){
	if (handle < 0) return;
	if (handle >= pSW_StackSize) return;
	
	SRS_Workspace* pWS = pSW_Stack[handle];
	if (pWS != NULL) delete pWS;
	pSW_Stack[handle] = NULL;
}

SRS_Workspace* SRS_Base_Workspace::GetWS(int handle, int autoId){
	if (handle == autoId)
		return &gAuto;
		
	if (handle < 0) return NULL;
	if (handle >= pSW_StackSize) return NULL;

	return pSW_Stack[handle];
}

};

