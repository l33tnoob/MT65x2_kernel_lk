/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef VIDEO_TELEPHONE_INTERFACE_H
#define VIDEO_TELEPHONE_INTERFACE_H

#include "VTSUtils.h"
#include <Surface.h>
#include <binder/IMemory.h>
#include <utils/String8.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>


using namespace android;
//using namespace videotelephone;

namespace VTService{
	
	class IVTSInterface: virtual public RefBase
	{
		public: 
			//virtual status_t openVTService()=0 ;
			virtual status_t openVTService(const int simId)=0 ;
			virtual status_t initVTService(const sp<VTSSurface>& localSurface, 
									const sp<VTSSurface>& peerSurface)=0;
			virtual status_t startVTService()=0;
			virtual status_t stopVTService(const int callEndType = 0)=0;
			virtual status_t closeVTService()=0;
			virtual void setEndCallFlag()=0;
							
			virtual status_t setLocalView(const int hideMeFlag, const VTSString & hideMeImageURI)=0;
			virtual status_t setPeerView(const int enableFlag, const VTSString & filePath)=0;
			virtual status_t incomingVTCall(const int flag)=0;			
			virtual status_t setInvokeLockPeerVideoBeforeOpen(const int invoked)=0;
			virtual status_t setMaxFileSize(const long long maxSize)=0;
			
			virtual status_t snapshot(const int type, const VTSString & savingImgURI)=0;
			virtual status_t snapshot(const int type, const sp<VTSBuffer>& mem)=0;
			virtual status_t setCameraParameters(const VTSCamParam & params)=0;
			virtual VTSCamParam getCameraParameters() const=0;
			virtual status_t setVTVisible(const bool isOn, const sp<VTSSurface> & localSurface,
									const sp<VTSSurface> & peerSurface)=0;
			virtual status_t setPeerVideoParam(const int paramType, const int value)=0;
			virtual void onUserInput(const String8 &buf)=0;
			virtual status_t sendCameraCommand(int cmd, int arg1, int arg2)=0;
			virtual int getCameraSensorCount()=0;
			virtual int setCameraSensor(int index)=0;	
			virtual int getCurCameraSensor()=0;
			virtual int emSetting(int msgId, int arg1, int arg2)= 0;
			virtual int sendCmd(int msgId, int arg1, int arg2, String8& arg3)= 0;
			virtual ~IVTSInterface(){};
	};

	class IVTSNotifyCallback: virtual public RefBase
	{

		public:
			virtual void notifyCallback(const int msgId, int32_t ext1, int32_t ext2)=0;
			virtual ~IVTSNotifyCallback(){};
	};

	#define DECLARE_IVTSINTERFACE	\
                        virtual status_t openVTService(const int simId);	\
			virtual status_t initVTService(const sp<VTSSurface>& localSurface, const sp<VTSSurface>& peerSurface);	\
			virtual status_t startVTService();	\
			virtual status_t stopVTService(const int callEndType);	\
			virtual status_t closeVTService();	\
			virtual void setEndCallFlag();\
			virtual status_t setLocalView(const int hideMeFlag, const VTSString & hideMeImageURI);	\
			virtual status_t setPeerView(const int enableFlag, const VTSString & filePath); \
			virtual status_t incomingVTCall(const int flag); \
			virtual status_t setInvokeLockPeerVideoBeforeOpen(const int invoked); \
			virtual status_t setMaxFileSize(const long long maxSize); \
			virtual status_t snapshot(const int type, const VTSString & savingImgURI);	\
			virtual status_t snapshot(const int type, const sp<VTSBuffer>& mem);	\
			virtual status_t setCameraParameters(const VTSCamParam & params);	\
			virtual VTSCamParam getCameraParameters() const;	\
			virtual status_t setVTVisible(const bool isOn, const sp<VTSSurface> & localSurface,	const sp<VTSSurface> & peerSurface);	\
			virtual status_t setPeerVideoParam(const int paramType, const int value);	\
			virtual void onUserInput(const String8 &buf);	\
			virtual status_t sendCameraCommand(int cmd, int arg1, int arg2);	\
			virtual int getCameraSensorCount();	\
			virtual int setCameraSensor(int index);	\
			virtual int getCurCameraSensor();	\
			virtual int emSetting(int msgId, int arg1, int arg2);	\
			virtual int sendCmd(int msgId, int arg1, int arg2, String8& arg3);	
}
#endif
