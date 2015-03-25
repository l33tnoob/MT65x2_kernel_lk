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

/********************************************************************************
 *	SRS Labs CONFIDENTIAL
 *	@Copyright 2012 by SRS Labs.
 *	All rights reserved.
 *
 *  Delta-minimizing patch for Jellybean's AudioFlinger.cpp
 ********************************************************************************/

#ifndef ANDROID_SRS_AUDIOFLINGER_PATCH
#define ANDROID_SRS_AUDIOFLINGER_PATCH

// DEFINE if detailed SRS-related logs are required...
//#define SRS_VERBOSE

#include "srs_processing.h"

#define SRS_PROCESSING_ACTIVE

namespace android {
	
// MACROS to help create very minimal deltas at the audioflinger level

#ifdef SRS_VERBOSE
	#define SRS_LOG(...) ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
	#define SRS_LOG(...) ((void)0)
#endif

#define POSTPRO_PATCH_JB_PARAMS_SET(a) \
	if (SRS_DoAny()){ \
		SRS_Processing::ParamsSet(SRS_Processing::AUTO, a); \
	}

#define POSTPRO_PATCH_JB_PARAMS_GET(a, b) \
	if (SRS_DoAny()){ \
		String8 srs_params = SRS_Processing::ParamsGet(SRS_Processing::AUTO, a); \
    	if (srs_params != "") b += srs_params+";"; \
	}
	
#define POSTPRO_PATCH_JB_OUTPROC_PLAY_INIT(a, b) \
	if (SRS_DoOutput()){ \
		SRS_LOG("SRS_Processing - OutNotify_Init: %s\n", b.string()); \
		SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, true); \
	}

#ifdef MTK_HIGH_RESOLUTION_AUDIO_SUPPORT
#define POSTPRO_PATCH_JB_OUTPROC_PLAY_SAMPLES(a, fmt, buf, bsize, rate, count) \
	if ((fmt == AUDIO_FORMAT_PCM_16_BIT || fmt == AUDIO_FORMAT_PCM_32_BIT) && SRS_DoOutput()){ \
		SRS_Processing::ProcessOut(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
	}
#else
#define POSTPRO_PATCH_JB_OUTPROC_PLAY_SAMPLES(a, fmt, buf, bsize, rate, count) \
	if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoOutput()){ \
		SRS_Processing::ProcessOut(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
	}
#endif

#define POSTPRO_PATCH_JB_OUTPROC_PLAY_EXIT(a, b) \
	if (SRS_DoOutput()){ \
		SRS_LOG("SRS_Processing - OutNotify_Exit: %s\n", b.string()); \
		SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, false); \
	}
	
#define POSTPRO_PATCH_JB_OUTPROC_PLAY_ROUTE(a, para, val) \
	if (SRS_DoOutput()){ \
		if (para.getInt(String8(AudioParameter::keyRouting), val) == NO_ERROR){ \
        	SRS_Processing::ProcessOutRoute(SRS_Processing::AUTO, a, val); \
        } \
	}
	
#define POSTPRO_PATCH_JB_OUTPROC_DUPE_SAMPLES(a, fmt, buf, bsize, rate, count) \
	if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoOutput()){ \
		SRS_Processing::ProcessOut(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
	}
	
#define POSTPRO_PATCH_JB_INPROC_INIT(a, b, fmt) \
	if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
		SRS_LOG("SRS_Processing - RecordThread - InNotify_Init: %p TID %d\n", a, b); \
		SRS_Processing::ProcessInNotify(SRS_Processing::AUTO, a, true); \
	}
	
#define POSTPRO_PATCH_JB_INPROC_SAMPLES(a, fmt, buf, bsize, rate, count) \
	if ((bsize > 0) && (fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
		SRS_Processing::ProcessIn(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
	}
	
#define POSTPRO_PATCH_JB_INPROC_EXIT(a, b, fmt) \
	if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
		SRS_LOG("SRS_Processing - RecordThread - InNotify_Exit: %p TID %d\n", a, b); \
		SRS_Processing::ProcessInNotify(SRS_Processing::AUTO, a, false); \
	}
	
#define POSTPRO_PATCH_JB_INPROC_ROUTE(a, para, val) \
	if (SRS_DoInput()){ \
		if (para.getInt(String8(AudioParameter::keyRouting), val) == NO_ERROR){ \
        	SRS_Processing::ProcessInRoute(SRS_Processing::AUTO, a, val); \
        } \
	}
	    
	      	
// FUNCTIONS to help direct execution based on build.prop settings

#ifdef POSTPRO_PROPGATE
bool SRS_DoOutput();
bool SRS_DoInput();
bool SRS_DoAny();
#else
inline bool SRS_DoOutput(){ return true; }
inline bool SRS_DoInput(){ return true; }
inline bool SRS_DoAny(){ return true; }
#endif

};	// namespace android

#endif // ANDROID_SRS_AUDIOFLINGER_PATCH
