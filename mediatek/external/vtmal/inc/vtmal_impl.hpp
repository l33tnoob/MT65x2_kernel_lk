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
*created by mingliangzhong(mtk80309)@2010-08-27
*modify history:
*	1,2010-09-16 ,1st build success!
*    2,2010-12-21 modify to independent datapath version. mingliangzhong
*                         
******************************************************/
#ifndef _VTMAL_IMPL_H_
#define _VTMAL_IMPL_H_
#include "vtmal.hpp"
#include "camerasource.hpp"
#include "audiocodecs.hpp"
#include "audiosink.hpp"
#include "audiosource.hpp"
#include "videosink.hpp"
#include "videocodecs.hpp"
//for record video and audio feature
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/mediarecorder.h>



namespace videotelephone
{
class CVTMultiMediaAdaptor :public IVTMultiMediaAdaptor
{
public:

	virtual int SetEventObserver(IMediaEventObserver* pObserver);

	virtual int AddPeerSource(const sp<IMediaObject>& Video,const sp<IMediaObject>& Audio);
	
	virtual int AddLocalSink(const sp<IMediaObject>& Video,const sp<IMediaObject>& Audio) ;

	virtual int RemovePeerSources();
	
	virtual int RemoveLocalSinks() ;
		
	virtual int AddPeerVideoSource(const sp<IMediaObject>& Video);
	
	virtual int AddPeerAudioSource(const sp<IMediaObject>& Audio);
	
	virtual int AddLocalVideoSink(const sp<IMediaObject>& Video) ;
	
	virtual int AddLocalAudioSink(const sp<IMediaObject>& Audio) ;
		
	virtual int SetSurface(const sp<Surface>& peerviewsurface,const sp<Surface>& localviewsurface,int CameraPreviewWidth = 320,int CameraPreviewHeight = 240) ;

	virtual int Init(int  flag = VTMAL_CF_PREVIEW_WHEN_INIT,VTMAL_Datapath id = VTMAL_Datapath_All);	
	
	virtual int Prepare(VTMAL_Datapath id = VTMAL_Datapath_All);		
	
	virtual int Start(VTMAL_Datapath id = VTMAL_Datapath_All);	

	virtual int Stop(int flag = VTMAL_CF_NORMAL,VTMAL_Datapath id = VTMAL_Datapath_All);

	virtual int Reset(VTMAL_Datapath id = VTMAL_Datapath_All);

	//for video surface control.  if java mmi need to continue VT call call when view is paused or stoped .
	// pls call this function: HideVideoPreview(),in  java app "onpause" or "onstop" funstion
	virtual int HideVideoPreview();

	//for video surface control. if java mmi resume,pls call this function: ShowVideoPreview()
	virtual int ShowVideoPreview(const sp<Surface>& peerviewsurface,const sp<Surface>& localviewsurface);

	virtual bool IsVideoPreviewStarted();
  
	virtual int Snapshot(const String8* url,const int type = ST_PEER) ;

	virtual int Snapshot(sp<IMemory>& buffer,const int type = ST_PEER) ;

public: //Get/Set  propertys functions

     //return VTMultiMediaAdaptor_State
	virtual int getState(VTMAL_Datapath id = VTMAL_Datapath_All) const;
	
    	virtual int getCodecCapability(VTMAL_Datapath id/*[in*/,Vector<sp<MetaData> >& supportedType/*[out]*/);


	virtual int getEncVOS(sp<IMediaBuffer>& vosdata);
	virtual int CheckVideoPacket(void* vop_pointer, int size, int mode);
        virtual int CheckVideoPacket(void* vop_pointer, int size,VOS_LOCATE* vos_locate, int mode);
	virtual int CheckVideoPacket(void* vop_pointer, int size, int mode,VOS_LOCATE* vos_locate, VOP_LOCATE* vop_info);
	virtual int CheckVOS(void* vos_pointer,int vos_size,void* p_vos_info);
	
	
	
	virtual String8 getCameraParameters();
	
	virtual int setCameraParameters(const String8& params) ;

     virtual  int    setVoiceMute(bool fmute);
	   
     virtual  int    getVoiceMute(bool& state) const;

	virtual int    setMicMute(bool fmute);
	
	virtual int    getMicMute(bool& state) const;

	virtual int    setHideMe(const int flag, const String8* url = 0);

	virtual int    setHideYou(const int flag, const String8* url = 0);
	

	virtual int    getHideMe(int& flag) const;

	//VTService_Command--->cmdid
	virtual int sendCommand(int cmdid,void* param,const int param_lenth);

	virtual status_t     sendCameraCommand(int32_t cmd, int32_t arg1, int32_t arg2);
	virtual int GetCameraSensorCount() ;

	virtual int SetCameraSensor(int index) ;
	virtual int GetCurCameraSensor() ;
	
	
public:
	
	CVTMultiMediaAdaptor();
	
	virtual ~CVTMultiMediaAdaptor();
	
	//VTMAL_NOTIFY --->msg
	  void HandleEventNotify(int msg, int params1,int params2);

private:

	void _CreateMediaGraph(VTMAL_Datapath id = VTMAL_Datapath_All);


	template<VTMAL_Datapath id>
	int  _Prepare_impl(sp<IMediaGraph>& g,sp<IMediaObject>& source,
	sp<IMediaObject>& trans,  sp<IMediaObject>& sink)
	{
		if(getState(id)  != VTMAL_INITIALIZED)
		{
			VTMAL_LOGERR;
			return NO_INIT;
		}
		if((!sink.get())||(!trans.get()) || (!source.get()))
		{
			VTMAL_LOGERR;
			return NO_INIT;
		}

		_CreateMediaGraph(id);
		int iret = OK;
		iret = g->Connect(source,trans);
		if(iret != OK) {VTMAL_LOGERR; return iret;}
		iret = g->Connect(trans,sink);
		if(iret != OK) {VTMAL_LOGERR; return iret;}

		if(id == VTMAL_Datapath_LocalVideo)
		{
			//set Surface
			if(m_pLocalVideoSource->IsPreview() == false){
				m_pLocalVideoSource->SetSurface(m_CameraSurface);
			}
		}

		return OK;		
	}

	template<VTMAL_Datapath id>
	int  _Start_impl(sp<IMediaGraph>& g)
	{
		if(!g.get())
		{
			VTMAL_LOGERR;
			return INVALID_OPERATION;
		}	

		int state = getState(id);

		switch(state)
		{
		case VTMAL_PREPARED:
			{
				return  g->Start();
			}
			break;
		case VTMAL_INITIALIZED:
			{
				if(OK == Prepare(id))
				{
					return   g->Start();
				}
			}
			break;
		case VTMAL_STARTED:
			return OK;
			break;
		default:
			break;
		}	
	
		return INVALID_OPERATION;

	}

	template<VTMAL_Datapath id>
	int  _Stop_impl(sp<IMediaGraph>& g)
	{
		int ret = OK;
		
		if(!g.get())
		{
			VTMAL_LOGERR;
			return INVALID_OPERATION;
		}
		
		if(getState(id) == VTMAL_STARTED)
		{
			ret = g->Stop();
		}

		return ret;

	}

	//int _handleEnableRecMP4File(void* param,const int param_lenth);

	//for Record Media(Video and  Audio) feature
	void setMaxFileSize(int64_t bytes){ mMaxFileSizeLimitBytes = bytes; }
	int _handleStartRecMedia(int cmdid,void*param,const int param_lenth);
	int _handleStopRecMedia(int cmdid,void* param,const int param_lenth);
	//int _Start3gpRecording();


	int _handleEnableYUVEmulator(void* param,const int param_lenth);

	int _handleAudioRec(void* param,const int param_lenth);

	int _handleLockPeerVideo(void* param,const int param_lenth);

	int _Init_SetCameraPreview(int  flag);
	  
	sp<IMediaEventObserver> m_pMediaGraphEventObserver; 

	IMediaEventObserver* m_pVTServiceEventHandler;



	  //internal  MediaObjects!	  
	  sp<CMp4Decoder> m_pPeerVideoDecoder;
	  
	  sp<CAmrDecoder> m_pPeerAudioDecoder;
	  
	  sp<CMp4Encoder> m_pLocalVideoEncoder;
	  
	  sp<CAmrEncoder> m_pLocalAudioEncoder;
	  
	  sp<CPeerVideoSink> m_pPeerVideoSink;
	  
	  sp<CAudioSink> m_pPeerAudioSink;
	  
	  sp<CCameraSource> m_pLocalVideoSource;
	  	  
	  sp<CAudioSource> m_pLocalAudioSource;

	  //extenal MediaObjects!
	  
	  sp<IMediaObject> m_pPeerVideoSource;
	  	  
	  sp<IMediaObject> m_pPeerAudioSource;

	  sp<IMediaObject> m_pLocalVideoSink;
	  
	  sp<IMediaObject> m_pLocalAudioSink;

	  mutable LockEx m_Lock;

	  sp<Surface> m_peerVideoSurface;
	  
	  sp<Surface> m_CameraSurface;
	  
	int m_iHideMe;
	int m_iHideYou;
	String8 m_iHideMeFilePath;
	String8 m_iHideYouFilePath;
	
	int m_iMicEnable;
	int m_iVoiceEnable;
	int m_iHidePreview;
	int m_iCameraPreviewWidth;
	int m_iCameraPreviewHeight;
	  
	//add for independent datapath version.
	sp<IMediaGraph>  m_PeerVideoGraph;
	sp<IMediaGraph>  m_PeerAudioGraph;
	sp<IMediaGraph>  m_LocalAudioGraph;
	sp<IMediaGraph>  m_LocalVideoGraph;

	int m_curSensor; // the current sensor is main sensor or sub sensor  0 mainsensor   1 subsensor
	//t m_curSensorFrameRate;
	int m_curSensorMode;
	CurSensorFR m_sCurSensorFR;

	CameraSensor_Param m_mainSensorParam;
	bool m_bGetMSParam;

	CameraSensor_Param m_subSensorParam;
	bool m_bGetSSParam;
	
	//for Record Media(Video and  Audio) feature
	sp<MPEG4Writer> m_mp4Writer;
	bool m_bMediaRecording;
	String8 m_sRecFilePath;
	int64_t mMaxFileSizeLimitBytes;
	int m_RecordCmdId;
	bool m_EnRecDlVideo;
	bool m_EnRecMixAudio;
	bool m_hasStartRecDlVideo;
	bool m_hasStartRecMixAudio;
	
	class CVTMediaRecordListner : public BnMediaRecorderClient
	{
		public:
			CVTMediaRecordListner(CVTMultiMediaAdaptor* pdelegation)
			{
				m_pdelegation = pdelegation;
			}
			~CVTMediaRecordListner(){}
			
			virtual void  notify(int msg, int ext1, int ext2)
			{
				VTMAL_LOGINFO;
				/*
				switch(msg)
				{
					case MEDIA_RECORDER_EVENT_INFO:
						if(ext1 == MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED)
							m_pdelegation->HandleEventNotify(msg,ext1,ext2);
					
				}*/
				m_pdelegation->HandleEventNotify(NOTIFY_RECORD_MSG_BEGIN+msg,ext1,ext2);
			}
	
			
		private:	
			CVTMultiMediaAdaptor* m_pdelegation;
			
	};
	
	// sp<MediaSource> m_videoRecSource;
	 sp<CVTMediaSource> m_videoRecSource;
	 //sp<MediaSource> m_audioRecSource;
	 sp<CVTMediaSource> m_audioRecSource;
	 sp<IMediaRecorderClient> m_RecordListner;

};

}
#endif //_VTMAL_IMPL_H_
