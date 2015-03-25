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
*	1, implement most features but snapshot
*                 ,mingliangzhong(mtk80309)@2010-09-29
*
******************************************************/

#ifndef _VTMAL_VIDEO_SINK_H_
#define _VTMAL_VIDEO_SINK_H_

#include "mediaobject.hpp"
#include <gui/Surface.h>
namespace videotelephone
{
class CPeerVideoSink:public CMediaObject,public Thread
{
public:
	
	CPeerVideoSink();
	
	virtual ~CPeerVideoSink();

	void SetSurface(const sp<Surface> surface);

	void HideVideo();

	void ShowVideo(const sp<Surface>& surface);
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;

	int Snapshot(const String8* url);
	
	int Snapshot(sp<IMemory>& buffer);
	
	void LockVideo(const char* pfile = NULL);

	void UnlockVideo();
	void setHideYouMem(sp<IMemory> hide_you_mem);
	void setDMLockMem(sp<IMemory> dm_lock_mem);
	virtual int    setHideYou(const int flag, const String8* url = 0);
	
protected:

	inline void exitThread();

	inline void doSnapshot(const sp<IMediaBuffer>& buffer);
	
	virtual int OnStart();

	virtual int OnPause();

	virtual int OnStop();

	virtual int OnReset();

	virtual bool  threadLoop();

	inline void postbuffer();

	virtual int Flush() ;
	
	void DecodeImage(const char* pfile,sp<IMediaBuffer>& p,int w,int h);
	
private:
	
    int displayBuffer_l(const sp<IMediaBuffer>& buffer);
	sp<Surface> m_Surface;
	
	bool m_fRegisterbuffers;

	IMediaBufferQueue m_BuffersQueue;

	Condition mWaitWork;
	
	mutable Mutex m_LockCondition;

	Condition mWaitSnapshot;
	mutable Mutex m_LockSnapshot;
	bool m_SnapshotResult;

	struct snapshot_patams
	{
		Condition WaitWork;
		sp<IMemory> memory;
		String8 url;
		bool isstringflag;
		bool fEnableSnapShot;
		bool ret;
	};

	snapshot_patams m_snapshot;
	
	bool m_fLockVideo;
	sp<IMediaBuffer> m_LockVideoBuffer;
	
	sp<IMemory> m_hideyou_mem;
	sp<IMemory> m_dmlock_mem;
	
	int m_iHideYou;
	sp<IMediaBuffer> m_HideYouImage;
	String8 m_iHideYouFilePath;
	int m_recwidth;
	int m_recheight;
	bool m_fpost_hideYouPicture;

	bool m_exitThread;
	
	
};

}
#endif //_VTMAL_VIDEO_SINK_H_
