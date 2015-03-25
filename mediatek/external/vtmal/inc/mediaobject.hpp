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

#ifndef _MEDIA_OBJECT_H_
#define _MEDIA_OBJECT_H_
#include "minterface.hpp"
#include "buffermanagement.hpp"

namespace videotelephone
{
class CMediaObject :virtual public IMediaObject
{
	
public:
	explicit CMediaObject(int type); //type ==> MOT_SOURCE/MOT_TRANSFORM/MOT_SINK
	
	virtual ~CMediaObject();
	
	virtual int ConnectTo(const sp<IMediaObject>& pNextObject,const sp<MetaData>& pMediaType);

	virtual int Disconnect();
		
	virtual int GetNextMediaObject(sp<IMediaObject>& pObject) ;
	
	virtual int Start(); 
	
	virtual int Pause();
	
	virtual int Stop();

	virtual int Reset();
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;
	    
	virtual int Send(const sp<IMediaBuffer>& pSample);	

	virtual int GetState()const;
	
	virtual int GetMediaObjectType() const;
	
	virtual Vector<sp<MetaData> > GetCurrentMediaType() const;
	
	virtual int SetMediaType(const sp<MetaData>& pMediaType);
	
	virtual  int SetEventHandle(const sp<IMediaEvent>& pEvent);

	void SetRefClock(const sp<IRefClock>& clock);

	virtual Vector<sp<MetaData> > GetSupportedMediaType();

	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType);

	static bool _CheckBasicVideo(const sp<MetaData>& MetaRef,const sp<MetaData>&  MediaConn);

	static bool _CheckBasicAudio(const sp<MetaData>& MetaRef,const sp<MetaData>&  MediaConn);

protected:	

	virtual int OnStart();
	
	virtual int OnPause();
	
	virtual int OnStop();

	virtual int OnReset();
	
	virtual int Flush() ;

	

	virtual bool AllocateMemoryPool(const sp<MetaData>& pMediaType);

	mutable LockEx m_Lock;

	int m_State;
	
	int m_MediaObjectType;

	sp<IMediaObject> m_NextMediaObject;	

	sp<MetaData> m_CurInputMetaData;
	
	sp<MetaData> m_CurOutputMetaData;
	
	Vector<sp<MetaData> > m_SupportedMetaDatas;
	
	sp<IMediaEvent> m_EventNotify;
	
	sp<IMediaBufferAllocator> m_BufferAllocator;

	sp<IRefClock> m_clock;
	
};



/*

template<class T> 
class AThread:public Thread
{
public:
	
	AThread(T* pThis, void (T::*method)()):
		Thread(false),
		m_pThis(pThis),
		mMethod(method)
	{
	
	}
		
	virtual ~AThread()
	{
		exit();
	}
	
protected:
	
	virtual bool  threadLoop()
	{
	    while (!exitPending())
		{
			AutolockEx _l(mLock);
			mWaitWork.wait(mLock);
			if(m_pThis)
			 (m_pThis->*mMethod)();

		}//end of  while (!exitPending())

		VTMAL_LOGDEBUG;
		return false;
	}
	
public:
	void trigger()
	{
		run();
		mWaitWork.signal();
	}

	void exit()
	{
	    {
			AutolockEx _l(mLock);
			requestExit();
			mWaitWork.signal();
		 }
		requestExitAndWait();
	}

protected:

	T* m_pThis;
	 void (T::*mMethod)();

	Condition mWaitWork;
		
};
*/


}




#endif //_MEDIA_OBJECT_H_
