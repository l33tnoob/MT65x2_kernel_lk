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

/******************************************************
*created by mingliangzhong(mtk80309)@2010-08-31
*modify history:
*
******************************************************/

#ifndef _VT_MEDIA_SOURCE_H_
#define _VT_MEDIA_SOURCE_H_



#include <media/stagefright/MPEG4Writer.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaData.h>
//#include <media/stagefright/MediaDebug.h>
//#include <media/stagefright/MediaDefs.h>
//#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
//#include <media/stagefright/Utils.h>


#include "mediatypedef.hpp"

//#include "vtmal_impl.hpp"
//#include "mediaobject.hpp"
//using namespace videotelephone;
using  android::MetaData;

namespace videotelephone
{

class CVTMediaSource :public MediaSource
{
	
	public:
		virtual ~CVTMediaSource();
	
		virtual status_t start(android::MetaData *params = NULL);
		virtual status_t stop();
	
		virtual sp<android::MetaData> getFormat();
	
		virtual status_t read(
				MediaBuffer **buffer, const ReadOptions *options = NULL); //interface with mpeg4writer
		
		status_t write(const sp<IMediaBuffer>& buffer); // interface with vtmal

		sp<android::MetaData> mMeta;
		
		Mutex mLock;
		Condition mFrameAvailableCondition;
		Condition mFrameCompleteCondition;
		bool m_isAudio;

		//for if stack not set timestamp to us
		int64_t mStartTimeUs;
		int64_t mFirstFrameTimeUs;
		int mNumFramesReceived;
		int64_t m_last_timestamp;
		
		int mNumFrameWrite;
		int mNumFrameRead;
		
#ifdef VTMS_ENABLE_CHOOSE_TM_PROVIDER
		enum TIMESTAMP_PROVIDER
		{
			USING_VTSTACK_TIMESTAMP = 0,
			USING_VTMAL_TIMESTAMP,
			
		};
		TIMESTAMP_PROVIDER m_tm_provider;
#endif	
		//int64_t mRealStartTimeUs;
		List<MediaBuffer *> m_bufferQueue;
		enum STATE{
			IDLE,
			STARTED,
			STOPPED
		};
		STATE m_state;
		
		virtual STATE getState(){return m_state;}

		//bool mStarted;
//		CVTMultiMediaAdaptor* m_pdelegation;
		CVTMediaSource(videotelephone::MetaData* outPutMeta = NULL);
	
private:		
		CVTMediaSource(const CVTMediaSource &);
		CVTMediaSource &operator=(const CVTMediaSource &);

	
};

//class CVTMultiMediaAdaptor;

};

#endif //_VT_MEDIA_SOURCE_H_