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

#ifndef _VTMAL_AUIDO_SINK_H_
#define _VTMAL_AUIDO_SINK_H_

#include "mediaobject.hpp"

#define USE_PCMXWAY_DIRECT_MODE

#ifdef USE_PCMXWAY_DIRECT_MODE
#include <media/AudioPCMxWay.h>
#else
#include <media/AudioTrack.h>
#endif

namespace videotelephone
{
 //please reference AudioPlayer in Stagefright!!,marked by mingliang zhong
class CAudioSink:public CMediaObject
{
public:
	CAudioSink();

	
	virtual ~CAudioSink();
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;

	void SetMute(bool fenable);

	bool GetMute() const;	
	
protected:
	virtual int OnStart();

	virtual int OnPause();
		
	virtual int OnStop();

	virtual int OnReset();
	
private:

	IMediaBufferQueue m_BuffersQueue;

	int mSampleRate;
	int mChannelNum;
    int64_t mLatencyUs;
    size_t mFrameSize;

	sp<IMediaBuffer> m_pInputBuffer;
#ifdef USE_PCMXWAY_DIRECT_MODE
    AudioPCMxWay* m_pAudioPCMxWay;
#else    
	AudioTrack* m_pAudioTrack;
#endif    
	bool mStarted;

	static void AudioCallback(int event, void *user, void *info);
    void AudioCallback(int event, void *info);

	size_t fillBuffer(void *data, size_t size);
};

}
#endif	// _VTMAL_AUIDO_SINK_H_
