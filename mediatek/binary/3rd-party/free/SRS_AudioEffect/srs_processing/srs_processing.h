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

#ifndef ANDROID_SRS_PROCESSING_API
#define ANDROID_SRS_PROCESSING_API

namespace android {

class SRS_Processing {
public:
	static const int AUTO;										// A special-case handle is _always_ available SRS_Processing::AUTO

	// Setup/Shutdown
	static int CreateHandle();									// Create a unique handle to an instance of SRS_Processing
	static void DestroyHandle(int handle);						// Destroy a handle
	
	// Audio to Speaker/Output
	static void ProcessOutNotify(int handle, void* pSource, bool init);		// Buffers from pSource will be processed - (or closed if init=false)
	static void ProcessOutRoute(int handle, void* pSource, int device);		// Called on any Routing parameter changes - device is from AudioSystem::DEVICE_OUT_XXX
	static void ProcessOut(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans);		// Process the buffer specified
	
	// Audio from Mic/Input
	static void ProcessInNotify(int handle, void* pSource, bool init);		// Buffers from pSource will be processed - (or closed if init=false)
	static void ProcessInRoute(int handle, void* pSource, int device);		// Called on any Routing parameter changes - device is from AudioSystem::DEVICE_IN_XXX
	static void ProcessIn(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans);		// Process the buffer specified
	
	// Parameters via String
	static void ParamsSet(int handle, const String8& keyValues);
	static String8 ParamsGet(int handle, const String8& keys);
	
	static bool ParamsSet_Notify(int handle, const String8& keyValues);
	
	// Typeless Data...
	static void RawDataSet(int* pHandle, char* pKey, void* pData, int dataLen);		// Used for side-band configuration.  NULL handle means 'global' or singleton.
	
	// Parameters via Enum	- param defined in seperate params enum header
	//ENUMS SAVED FOR LATER
	//static void EnumSet(int handle, int param, float value);
	//static float EnumGet(int handle, int param);
};

};	// namespace android

#endif // ANDROID_SRS_PROCESSING_API
