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

#ifndef VIDEO_TELEPHONE_SERVICE_H
#define VIDEO_TELEPHONE_SERVICE_H

#include "IVTSClient.h"
#include "IVTSService.h"
#include "VTSCore.h"

namespace VTService{


	class VTSService: public BnVTSService, public IBinder::DeathRecipient, public IVTSServiceCallBack
	{
		public:
		status_t checkPid();
		
		VTSService() :
			BnVTSService(){
				_I("VTService started: pid=%d", getpid());
				//mUser  ????
		}
	
		virtual ~VTSService(){
			if(mVTSCore!=0){
			_E("mVTSCore was still alive");
			}
		}

		virtual status_t onTransact(
				uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);

		sp<VTSCore> getVTSCore(){return mVTSCore;};
		const sp<IVTSClient>& getVTSClient() const;
		void removeClient(const sp<IVTSClient>& VTSClient);
		status_t setupVTS(sp<IVTSClient> & VTSClient);
		void releaseVTS();	//for un setup
		sp<IVTSClient> getClient();
		void notifyCallback(const int msgId, int32_t ext1, int32_t ext2) ;
		
		DECLARE_IVTSINTERFACE																
		private:
            virtual void binderDied(const wp<IBinder>& who);

            sp<IBinder> mPhoneBinder;
			mutable Mutex mLock;
			mutable Mutex mVTSClientLock;
			sp<VTSSurface> mLocalSurface;
			sp<VTSSurface> mPeerSurface;
			pid_t mClientPid;
			sp<VTSCore> mVTSCore;
			wp<IVTSClient> 	mVTSClient;
	};
}

#endif






