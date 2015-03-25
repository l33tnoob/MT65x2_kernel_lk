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

#ifndef ANDROID_SRS_TECH_TOOLS
#define ANDROID_SRS_TECH_TOOLS

#include <time.h>

struct SRS_Perf {
	SRS_Perf();
	
	static unsigned long long	GetRelativeTimeNsec(void);
	
	unsigned long long	TimingStart;	// Time at start of perf
	unsigned long long	DeltaStart;		// Time at start of delta
	
	unsigned long long	DeltaAccum;		// Accumulation of all deltas
	uint32_t			DeltaSamps;		// Accumulation of all samples
	uint32_t			DeltaCount;		// Number of times a delta is added
	
	void	StartTiming();
	void	LogTiming(const char* pTag);
	void	EndTiming();
	
	void	StartDelta();
	void	EndDelta(uint32_t sampleCount);
};

#define DIFF_FORCED(a) ((ForceActive) || (Active.a != pCFG->a))

namespace android {
	
struct SRS_Tech_State {
	bool InPath;		// Now
	bool IsActive;
	int UseIndex;
	SRSFadeScale UseScale;
	
	bool FadeOut;
	bool FadeIn;
	
	bool WantPath;		// Post Fade Out, Pre Fade In
	bool WantActive;
	int WantIndex;
	SRSFadeScale WantScale;
	
	bool WantDirty;
	
	SRS_Tech_State(){
		WantPath = InPath = false;
		WantActive = IsActive = false;
		WantIndex = UseIndex = -1;
		WantScale = UseScale = SRSFadeScale_Default;
		FadeIn = FadeOut = false;
		WantDirty = false;
	}
	
	bool SetWants(bool path, bool active, int index, SRSFadeScale scale = SRSFadeScale_Default){
		WantPath = path;
		WantActive = active;
		WantIndex = index;
		WantScale = scale;
				
		if (WantPath == false){ WantActive = false; WantIndex = -1; }
		else if (WantIndex < 0){ WantIndex = -1; WantPath = false; WantActive = false; }
		
		FadeIn = FadeOut = false;
		WantDirty = false;
		
		if (WantPath != InPath) WantDirty = true;
		if (WantActive != IsActive) WantDirty = true;
		if (WantIndex != UseIndex) WantDirty = true;
		
		if (WantDirty == false) return false;
		
		if (InPath) FadeOut = true;	// Fade out because we're active now, either turing off, or changing...
		if (WantPath) FadeIn = true;	// Fade in because we're starting, or changing.
		
		return true;	// Needs to fade
	}
	
	bool PreFade(SRSSamp* pSamples);	// If we need to fade, prep here - return true to cause Reconfig of tech...
	bool PostFade(SRSSamp* pSamples);	// If we need to fade, act here - return true to cause Reconfig of tech...
	static SRSFadeScale CalcFadeScale(float scale);
};

struct SRS_DCRState {
	int Workspace[2][4];	// Stereo
	
	SRS_DCRState(){
		memset(Workspace, 0, sizeof(Workspace));
	}
	
	void Process_256(SRSSamp* pData);
};

extern double Tool_DoComplexWithReals(double freq, double gain, unsigned sampleRate, double a1, double a2);
extern void Tool_GenHiPassCoefs(void* pCoefs, int order, int freq, int srate, int is32bit);
extern void Tool_GenLoPassCoefs(void* pCoefs, int order, int freq, int srate, int is32bit);

extern float Tool_MaxZero(float tIn, float tMax);
extern void Tool_SeekValue(float want, float& active);

};

#endif	// ANDROID_SRS_TECH_TOOLS
