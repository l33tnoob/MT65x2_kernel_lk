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
*created by mingliangzhong(mtk80309)@2010-08-30
*modify history:
*	1,Add Thread to TMediaEventObserver
*		,mingliangzhong(mtk80309)@2010-09-16
*		
*   2,Add IDataListener,mingliangzhong(mtk80309)@2010-09-26
*
******************************************************/
#ifndef _MEDIA_INTERFACE_H_
#define _MEDIA_INTERFACE_H_
#include "mediatypedef.hpp"

namespace videotelephone
{
class IMediaEventObserver :virtual public RefBase
{
public:
	
	IMediaEventObserver();
	
	virtual ~IMediaEventObserver();
	
	virtual void OnEventNotify(int msg,int  params1, int params2) = 0;
};

template <typename T>
class TMediaEventObserver : public IMediaEventObserver,public Thread
{

public:

	struct event_msg
	{
		int msg;
		int params1;
		int params2;
	};	
	
	TMediaEventObserver(T* pdelegation):
		IMediaEventObserver(),
		Thread(false),
		m_pdelegation(pdelegation)
	{
		run();
	}
		
	virtual ~TMediaEventObserver()
	{    
		VTMAL_LOGDEBUG;
	}

	virtual bool   threadLoop()
	{

	    while (!exitPending())
		   {
			Mutex::Autolock _l(m_Lock);
   			 mWaitWork.wait(m_Lock);
			for(size_t i = 0; i < m_events.size(); i ++)
			{
				if(m_pdelegation)
				{
					 m_pdelegation->HandleEventNotify(m_events[i].msg,
				 		m_events[i].params1,
				 		m_events[i].params2);
				}
			}//end of for(unsigend int i = 0;
			m_events.clear();
	    	}//end of  while (!exitPending())
	    	
		VTMAL_LOGDEBUG;
		return false;
	}
	
	virtual void OnEventNotify(int msg, int params1,int params2)
	{
		AutoMutex _l(m_Lock);
		event_msg em;
		em.msg = msg;
		em.params1 = params1;
		em.params2 = params2;
		m_events.add(em);
		//if(mRunning == false)
			
		mWaitWork.signal();
	}

	/*
	* must call this function before destroy instance
	*
	*/
	void QuitThread()
	{
		exit();
	}
	
private:
	
	void exit()
	{
	    {
	        AutoMutex _l(m_Lock);
		  m_pdelegation = NULL;
	        requestExit();
	        mWaitWork.signal();
	    }
	    requestExitAndWait();
	}

	T* m_pdelegation;
	
	Condition mWaitWork;

	Vector<event_msg> m_events;
	
	mutable Mutex m_Lock;
};

#define EVENTOBSERVER_CREATE(type,obj) new TMediaEventObserver<type>(obj);
#define EVENTOBSERVER_THREAD_RUN(type,spinstance) if(spinstance.get()) {((TMediaEventObserver<type>*)(spinstance.get()))->run();}
#define EVENTOBSERVER_THREAD_QUIT(type,spinstance) if(spinstance.get()) {((TMediaEventObserver<type>*)(spinstance.get()))->QuitThread();}
#define EVENTOBSERVER_DESTROY(type,spinstance) EVENTOBSERVER_THREAD_QUIT(type,spinstance);spinstance.clear();

class IMediaEvent :virtual public RefBase
{
public:
	
	IMediaEvent();
	 
	virtual ~IMediaEvent();
	
	virtual void SetEvent(int msg, int params1,int params2) = 0;
};

class IMediaBuffer:public RefBase
{
public:
	
	IMediaBuffer();
	
	virtual ~IMediaBuffer();

	virtual void* pointer() const = 0;

	virtual size_t size() const = 0;

	virtual int SetRealSize(size_t size)  = 0;

	virtual sp<IMemory>& GetMemory() = 0;

	virtual sp<MetaData>& GetMetaData() = 0;
		 
	//for Audio Sink 
	virtual size_t range_offset() const = 0;

	//for Audio Sink 
	virtual size_t range_length() const = 0;

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length) = 0;
		 
};


//buffer heap is allocated  and managered by upstreams mediaobject
class IMediaBufferAllocator:public RefBase
{
public:

	IMediaBufferAllocator();
	
	virtual ~IMediaBufferAllocator();
	
	virtual int  GetMediaBuffer(sp<IMediaBuffer>& Sample)= 0;
	
	virtual int FreeMediaBuffer(IMediaBuffer* pSample)= 0;

	virtual sp<IMemoryHeap> GetMemoryHeap() const = 0;
	virtual sp<IMemory> HideOneMemoryForSpecUse() = 0;
};

enum MediaObject_Type{
	MOT_SOURCE,
	MOT_TRANSFORM,
	MOT_SINK,
	MOT_UNKNOWN};

enum MediaObject_State{
	MOS_IDLE,
	MOS_CONNECT,
	MOS_START,
	MOS_PAUSE};
	
class IMediaObject :virtual public RefBase
{
	
public:

	IMediaObject();
	
	virtual ~IMediaObject();
	
	virtual int ConnectTo(const sp<IMediaObject>&  ptargetObject,const sp<MetaData>& pMediaType)= 0;

	virtual int Disconnect() = 0;
	
	virtual int GetNextMediaObject(sp<IMediaObject>& pObject) = 0;
	
	virtual int Start()= 0;
	
	virtual int Pause()= 0;
	
	virtual int Stop()= 0;

	virtual int Reset()= 0;
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) = 0;
	    
	virtual int Send(const sp<IMediaBuffer>& pSample) = 0;	

	virtual int GetState()const = 0;
	
	virtual int GetMediaObjectType() const = 0;
	
	virtual Vector<sp<MetaData> > GetSupportedMediaType()  = 0;

	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType) = 0;
	
	virtual Vector<sp<MetaData> > GetCurrentMediaType() const = 0;
	
	virtual int SetMediaType(const sp<MetaData>& pMediaType) = 0;
	
	virtual int SetEventHandle(const sp<IMediaEvent>& pEvent) = 0;

	virtual void SetRefClock(const sp<IRefClock>& clock) = 0;

};

/******************************************
*gcc has not support "__declspec" keywords
*we define mime in android.
******************************************/

class Mime
{
public:
	Mime():mime(0){}

	Mime(int i):mime(i){}
	
	~Mime(){}

	void setMime(int i){
		mime = i;
	}
	int getMime() const
	{
		return mime;
	}
	Mime& operator = (const Mime& a)
	{
		mime = a.getMime();
		return *this;
	}

	 bool operator == (const Mime& a) const
 	{
 		return mime == a.getMime() ? true:false;
 	}
private:
	int mime;	
};
#define IIMediaInterface_mime 0
class IIMediaInterface:virtual public RefBase
{
public:
	IIMediaInterface():RefBase(){ }
	virtual ~IIMediaInterface(){}
protected:
	Mime m_mime;
};


#define IIMediaSeeking_mime 'MSK!'
class IIMediaSeeking:public IIMediaInterface
{
public:
	IIMediaSeeking():IIMediaInterface(){m_mime = IIMediaSeeking_mime;}
	virtual ~IIMediaSeeking(){}
	virtual int  GetDuration(int64_t& ret) = 0;
	virtual  int   GetPos(int64_t& ret) = 0;
	virtual int SetPos(int64_t  ret) = 0;
	virtual bool canSeek() = 0;
	virtual int GetRate(float& f) = 0;	
};

/*****************************************************************
*  mediaobject sets (MediaGroup: role as  filter in dshow  or bin in gstreamer.)
*                             -----------------------------
*                             |    Media  Object Group  |
*   ------------        -----------                         |
*   | Object  | ---> | Object |        p                |
*   ------------       -----------        r                |
*                              |                   o      --------------       -------------
*   ------------        -----------       c      |  Object    |  ---> | Object   | 
*   | Object  | ---> | Object |        e       --------------       -------------
*   ------------       -----------        s                 |
*                              |                   s                 |
*                            -------------------------------
*
*
****************************************************************/
class IMediaGroup:virtual public RefBase
{

public:

	IMediaGroup();
	
	virtual ~IMediaGroup();

	virtual int  GetMediaObjects(Vector<sp<IMediaObject> >&objs) = 0;

	virtual int  GetGroupName(String8& name) = 0;

	virtual int  Query(const int mime, sp<IIMediaInterface>& interface) = 0;
};

class IDataListener :virtual public RefBase
{
public:
	
	IDataListener();
	
	virtual ~IDataListener();
	
	enum DataType{VTDT_VIDEO,VTDT_AUDIO};
	
	virtual void postData(int type,const sp<IMediaBuffer>& data) = 0; 
};

class IMediaGraph :virtual public RefBase
{
public:
	
	IMediaGraph();
	
	virtual ~IMediaGraph();
	
	virtual int Connect(const sp<IMediaObject>& pUpObject,
		const sp<IMediaObject>& pDownObject, MetaData*pMediaType = 0)=0;

	virtual void Disconnect()=0;
	
	virtual int Start() =0;
	
	virtual int Pause()=0;
	
	virtual int Stop()=0;

	virtual int Reset()=0;

	virtual void SetEventObserver(const wp<IMediaEventObserver>& pObserver)=0;
};
}

#endif //_MEDIA_INTERFACE_H_
