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

#ifndef VIDEO_TELEPHONE_ISERVICE_H
#define VIDEO_TELEPHONE_ISERVICE_H

#include "IVTSInterface.h"
#include "IVTSClient.h"
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace VTService{
	class IVTSService:public IInterface, public IVTSInterface
	{
		public:
		enum{
			SETUP_VTS  = IBinder::FIRST_CALL_TRANSACTION,
			//OPEN_VT_SERVICE,
			OPEN_VT_SERVICE_WITH_SIM,
			INIT_VT_SERVICE,
			START_VT_SERVICE,
			STOP_VT_SERVICE,
			CLOSE_VT_SERVICE,
			SET_END_CALL_FLAG,
			HIDE_LOCAL_VIEW,
			REPLACE_PEER_VIDEO,
			INCOMING_VIDEO_CALL,
			INVOKE_LOCKPEERVIDEO_BFOPEN,
			SET_MAX_FILE_SIZE,
			SNAPSHOT_AND_SAVING,
			SNAPSHOT,
			SET_CAMERA_PARAM,
			GET_CAMERA_PARAM,
			SET_VT_VISIBLE,
			SET_PEER_VIDEO_PARAM,
			ON_USER_INPUT,
			SEND_CAMERA_COMMAND,
			GET_CAMEREA_SENSOR_NR,
			SET_CAMERA_SENSOR,
			GET_CURRENT_CAMERA_SENSOR,
			EM_SETTING,
			SEND_CMD,
			RELEASE_VTS,
		};
			DECLARE_META_INTERFACE(VTSService);	
			virtual status_t setupVTS(sp<IVTSClient> & client) =0;	
			virtual void releaseVTS() = 0;
	};
	
	class BnVTSService: public BnInterface<IVTSService>
	{
		public:
			virtual status_t onTransact(uint32_t code, 
									const Parcel& data,
									Parcel * reply,
									uint32_t flags = 0);
	};
}
#endif