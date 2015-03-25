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

/*********************************************************************************
*created by mingliangzhong(mtk80309)@2010-09-01
*modify history:
*	1, add hide me feature.mingliangzhong(mtk80309)@2010-10-14
*    2, add VTVideoSurface to implement  hide me feature.mingliangzhong(mtk80309)@2010-10-20
*    
*
*********************************************************************************/

#ifndef _VTMAL_CAMERA_SOURCE_H_
#define _VTMAL_CAMERA_SOURCE_H_
#include "mediaobject.hpp"

#include "vtmal.hpp"


#include <camera/ICamera.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <ui/GraphicBuffer.h>
//#include <ui/Overlay.h>
#include <gui/Surface.h>
#include <binder/IPCThreadState.h>

namespace videotelephone
{

class DisableCameraCallingPidCheck
{
	public:
		DisableCameraCallingPidCheck()
		{
			if(getpid() == android::IPCThreadState::self()->getCallingPid())
			{
				fdisable_ = false;
			}
			else
			{
				fdisable_ = true;
				token_ = android::IPCThreadState::self()->clearCallingIdentity();
			}
		}

		~DisableCameraCallingPidCheck()
		{
			if(fdisable_){
		 		android::IPCThreadState::self()->restoreCallingIdentity(token_);
			}
		}
	private:
		int64_t token_;
		bool fdisable_;
};
#ifdef _TEST_ON_VTTESTS_APP
#define  VTMAL_DISABLE_CAMERA_CALLINGPID_CHECK 
#else
#define  VTMAL_DISABLE_CAMERA_CALLINGPID_CHECK DisableCameraCallingPidCheck adcpc;
#endif //_TEST_ON_VTTESTS_APP
#ifdef VT_DISABLE_RECORDING_MODE
class CCameraSource;
    #ifdef VT_CAMERASOURCE_PROCESSTHREAD
    class ProcessThread;
    #endif// VT_CAMERASOURCE_PROCESSTHREAD
#endif //VT_DISABLE_RECORDING_MODE
class VTVideoSurface: public RefBase
{
public:
#ifdef VT_DISABLE_RECORDING_MODE
	friend class CCameraSource;
	VTVideoSurface(CCameraSource* pSource,const sp<Surface> & surface);
#else
	VTVideoSurface(const sp<Surface> & surface);
#endif//VT_DISABLE_RECORDING_MODE
	virtual ~VTVideoSurface();

	virtual sp<Surface> GetRealSurface()
	{
		return m_realsurface;
	}

public: //for CCameraSource function call

	void    setHideMe(const int flag, const sp<IMediaBuffer>&  bmp);

	void    setEncHideMeImage(const sp<IMediaBuffer>&  bmp);

	void    getHideMe(int& flag) const;

	int  Show(const sp<Surface>& surface);

	int  Hide();

    void setTransform(int orientation, int facing);

    int displayBuffer(const sp<IMediaBuffer>& buffer, int w, int h);
    int disconnet();
    int connect(int w, int h, int n = 3);
    void postBuffer(const sp<IMediaBuffer>& oBuffer, int w, int h);
protected:
	
	sp<Surface> m_realsurface;
	
	bool m_fsurfacereg;

	int m_HideMeMode;
	
	mutable LockEx m_Lock;
	
	bool m_fpost_hidemepicture;
	
	bool m_fpost_1stpicture;
	
	sp<IMediaBuffer> m_hideme_bmp;
	sp<IMediaBuffer> m_hideme_encimage;
#if 0
    sp<IMediaBuffer> m_black_screen;
    unsigned int       bufW;
    unsigned int       bufH;
#endif
	bool 				m_b_clear_buffer;
	unsigned int       m_bufCnt;
#ifdef VT_DISABLE_RECORDING_MODE
	CCameraSource* m_pSource;
#endif //#ifdef VT_DISABLE_RECORDING_MODE
    int m_orientation;
    int m_facing;
private:
    int displayBuffer_l(const sp<IMediaBuffer>& buffer, int w, int h);
    int disconnet_l();
    int connect_l(int w, int h, int n = 3);
};

class CCameraSource:public CMediaObject
{
public:
	
	CCameraSource();

	virtual ~CCameraSource();

	virtual Vector<sp<MetaData> > GetSupportedMediaType() ;

	sp<Camera>& GetCamera();

    // set preview/capture parameters - key/value pairs
   virtual  int    setParameters(const String8& params);

    // get preview/capture parameters - key/value pairs
    int   getParameters( String8& para ) const;
	
	
	int    setHideMe(const int flag, const String8* url = 0);

	void    getHideMe(int& flag) const;

	virtual int    setVideoRecSize(int width, int height);

	virtual int  StartPreview(const sp<Surface>& surface,int width,int height);

	virtual int  StartPreview();

	virtual int  StopPreview();

	bool IsPreview()
	{
		return m_fPreview?true:false;
	}


protected:
	virtual int OnStart();
	
	virtual int OnPause();
	
	virtual int OnStop();

	virtual int OnReset();
	
public:
	int HideVideo();

	int ShowVideo(const sp<Surface>& surface);

	int SetSurface(const sp<Surface>& surface);

	virtual bool IsCameraConnected();


	virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);

	virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr);

#ifdef VT_DISABLE_RECORDING_MODE
	void OnPostData2Codec(const sp<IMediaBuffer>& p1);
    #ifdef VT_CAMERASOURCE_PROCESSTHREAD
	void OnProcessData(const sp<IMediaBuffer>& p1);
    #endif// VT_CAMERASOURCE_PROCESSTHREAD
#endif //VT_DISABLE_RECORDING_MODE
	virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
	virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);


	status_t     sendCameraCommand(int32_t cmd, int32_t arg1, int32_t arg2) ;
	int GetCameraSensorCount() ;

	int SetCameraSensor(int index) ;
	
	int GetCurCameraSensor() ;

	bool GetCurSensorParam(CameraSensor_Param* sCameraSensorParam); //int* curSensorFrameRate,int* curSensorNighModeFrameRate );
	bool GetCurSensorFR(CurSensorFR* sCurSensorFR);

	bool _findSceneModeFRs(const char *cSupportFrameRates,int* iSensorNormalModeFR,int* iSensorNightModeFR);
protected:
	virtual bool AllocateMemoryPool(const sp<MetaData>& pMediaType);
	
protected:
	
	int ConnectToCamera();

	void CompulsoryCameraSettings();

	int ChooseCamera(int index = 0);

	int64_t   GetSystemTimeUs();

	sp<Camera> m_camera;

	sp<CameraListener> m_listener;
	
	sp<VTVideoSurface> m_videosurface;
	
	sp<IMediaBuffer> m_videorecImage;
	
	int m_HideMeMode;
	int m_fPreview;
	int m_recwidth;
	int m_recheight;
	int64_t m_startTime;
	int m_iCurSensor;
	
	int m_curSensorNormalModeFR;
	int m_curSensorNightModeFR;
	int m_curSensorFR;
	int m_curSensorMode;

    int m_previewCbRefCount; //for make sure the call back will return back


#ifdef  VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE
	Vector< CameraInfo> m_cam_infos;
#endif //VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE	
        //add by mtk80691 11-01-07 for record timestamp
	int64_t m_timestamp;
#ifdef VT_CAMERASOURCE_PROCESSTHREAD
    sp<ProcessThread> m_ProcessThread;
#endif// VT_CAMERASOURCE_PROCESSTHREAD
};


class CFakeCameraSource:public CCameraSource
{
public:
class EmuThread :public Thread
{
public:
	EmuThread(CFakeCameraSource* p):Thread(false)
	{
		_p = p;
	}
	
	virtual ~EmuThread()
	{
	
	}


	//add vtmal test case here!!!!
	virtual bool  threadLoop()
	{
		while (!exitPending())
		{
			if(_p)
			{
				 int64_t time = _p->PushData();
				 if(time < 33333)
			 	{
			 		usleep(33333 - time);
			 	}
			}
			else
			{
				usleep(100000); //100ms
			}
		}
		return false;
	}

private:
	CFakeCameraSource* _p;
};


public:
	CFakeCameraSource(const char *filepath);

	virtual ~CFakeCameraSource();

	virtual int    setVideoRecSize(int width, int height);

	virtual int  StartPreview();

	virtual int  StopPreview();

protected:

	//virtual bool  threadLoop();
	
	virtual int OnStart();
	
	virtual int OnStop();

	virtual int OnReset();

	virtual int ConnectToCamera()
	{
		return OK;
	}

	int SetYUVFile(const char *filepath);
	
public:
	virtual bool IsCameraConnected()
	{
		 return true;
	}

private:

	int64_t PushData();
	
	 int m_fRecording;

	List< sp<IMediaBuffer> > file_queue;
	List< sp<IMediaBuffer> >::iterator iter;	

	sp<EmuThread> testthread;

	sp<IMediaBufferAllocator> displaybuffer;
};


class VTCameraListener:	virtual public CameraListener
{

public:
	VTCameraListener(CCameraSource* p):_p(p)
	{
		VTMAL_LOGDEBUG
	}

	virtual ~VTCameraListener()
	{
		_p = NULL;
		VTMAL_LOGDEBUG
	}

	void setNULL()
	{
		AutolockEx _l(m_lock);
		_p = NULL;
	}

	void OnStopRecording()
	{
		AutolockEx _l(m_lock);
		m_heap.clear();
	}
	
	virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2)
	{
		AutolockEx _l(m_lock);
		if(_p)
			_p->notify(msgType,ext1,ext2);
	}

	virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr)
	{
		AutolockEx _l(m_lock);
		if(_p)
			_p->postData(msgType,dataPtr);
	}

	virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr)
	{
		AutolockEx _l(m_lock);
		VTMAL_LOGDEBUG;
		if((msgType == CAMERA_MSG_VIDEO_FRAME))
		{
			if(!m_heap.get())
			{			
				m_heap = dataPtr->getMemory();	
				int heapid = m_heap->getHeapID();
				void* pbase = m_heap->getBase();	
				LOGD("[VTMAL]@ Recording Memory id = %d, base = 0x%x,\n",heapid,(unsigned int)pbase);		
			}
		if(_p)
			_p->postDataTimestamp(timestamp,msgType,dataPtr);
		}

	}

	virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
	{
		AutolockEx _l(m_lock);
		if(_p)
			_p->postData(msgType,dataPtr,metadata);
    }

	private:
		CCameraSource* _p;
		mutable LockEx m_lock;
		sp<IMemoryHeap> m_heap;

};

#ifdef VT_DISABLE_RECORDING_MODE
class CNonMemcpyBuffer:public IMediaBuffer
{
public:
	CNonMemcpyBuffer(void* buffer, size_t size)
		:m_buffer(buffer),m_realsize(size),m_offset(0),m_length(size),m_metadata(new MetaData){

	}

private:
	void*  m_buffer;
	size_t m_realsize;
	size_t m_offset;
	size_t m_length;
	sp<MetaData> m_metadata;	
	sp<IMemory>  m_fakeMem;
public:
	virtual void* pointer() const{
		return m_buffer;
	}

	int SetRealSize(size_t size){
		VTMAL_LOGDEBUG
		return INVALID_OPERATION;
	}
	
	virtual size_t size() const{
		return m_realsize;
	}

	virtual sp<IMemory>& GetMemory(){
		return m_fakeMem;
	}

	virtual sp<MetaData>& GetMetaData(){
		return m_metadata;
	}


	//for Audio Sink 
	virtual size_t range_offset() const{
		return  m_offset;
	}
	//for Audio Sink 
	virtual size_t range_length() const{
		return  m_length;
	}

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length){		
	}		
};
#endif //VT_DISABLE_RECORDING_MODE

class CCameraMediaBuffer:public IMediaBuffer
{
	friend class CCameraSource;

	CCameraMediaBuffer(const sp<Camera>& camera,const sp<IMemory>& buffer)
		:m_camera(camera),m_buffer(buffer),m_metadata(new MetaData)
	{
			m_realsize = m_buffer->size();
			m_offset = 0;
			m_length = m_realsize;
	}
public:	
	virtual ~CCameraMediaBuffer()
	{
		if(m_camera.get())
		{
			VTMAL_DISABLE_CAMERA_CALLINGPID_CHECK;
			m_camera->releaseRecordingFrame(m_buffer);
		}
	}
	
	virtual void* pointer() const 
	{
		return m_buffer->pointer();
	}

	int SetRealSize(size_t size)
	{
		VTMAL_LOGDEBUG
		return INVALID_OPERATION;
	}
	
	virtual size_t size() const 
	{
		return m_buffer->size();
	}

	virtual sp<IMemory>& GetMemory() 
	{
		return m_buffer;
	}

	virtual sp<MetaData>& GetMetaData() 
	{
		return m_metadata;
	}


	//for Audio Sink 
	virtual size_t range_offset() const
	{
		return  m_offset;
	}
	//for Audio Sink 
	virtual size_t range_length() const
	{
		return  m_length;
	}

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length)
	{
		if(offset >= m_realsize)
		{
			VTMAL_LOGDEBUG;
			m_offset = 0;
			m_length = 0;
			return;
		}

		if((length + offset) > m_realsize) 
		{
			VTMAL_LOGDEBUG;
			m_offset = offset;
			m_length = m_realsize - offset;
			return;
		}
	
		m_offset = offset;
		m_length = length;
	}

private:
	sp<Camera>  m_camera;
	
	sp<IMemory>  m_buffer;

	sp<MetaData> m_metadata;	
	
	size_t m_realsize;
	 
	 size_t m_offset;
	 size_t m_length;
};



}
#endif //_VTMAL_CAMERA_SOURCE_H_
