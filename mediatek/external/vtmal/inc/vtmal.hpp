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
*
******************************************************/
#ifndef _VTMAL_H_
#define _VTMAL_H_
#include "minterface.hpp"

namespace videotelephone
{

enum VTMultiMediaAdaptor_State{
	VTMAL_IDLE,
	VTMAL_INITIALIZED,
	VTMAL_PREPARED,
	VTMAL_STARTED,
	VTMAL_ERROR
	};
	
enum VTMAL_Datapath{
	VTMAL_Datapath_PeerAudio,
	VTMAL_Datapath_PeerVideo,
	VTMAL_Datapath_LocalAudio,
	VTMAL_Datapath_LocalVideo,
	VTMAL_Datapath_All
	};


enum VTService_Command{
	SET_FORCE_IFRAME,
	SET_CHANGE_QUALITY,
	SET_MAX_PACKET_SIZE,
	EM_ENABLE_REC_MP4,
	EM_ENABLE_YUV_EMULATOR,
	EM_ENABLE_AUDIOREC,
	LOCK_PEER_VIDEO,
	UNLOCK_PEER_VIDEO,
	START_REC_DLVIDEO_MIXAUDIO, //haizhen
	STOP_REC_DLVIDEO_MIXAUDIO,	//haizhen
	START_REC_DLVIDEO, //haizhen
	STOP_REC_DLVIDEO, //haizhen
	};


//add by mtk80691--start
enum VideoPacket_Type
{
	VIDEO_PKT_I_Frame = 0,
	VIDEO_PKT_P_Frame,
	VIDEO_PKT_C_Packet,
	VIDEO_PKT_VOS,
	VIDEO_PKT_VOS_Bitstream,
	VIDEO_PKT_ERROR,
	VIDEO_PKT_C_VOP_I_Frame,
	VIDEO_PKT_C_VOP_P_Frame,
	VIDEO_PKT_C_VOP_ERROR_Frame, //B or S frame
	VIDEO_PKT_C_VOP_UNKNOW_Frame, //this is for Coolpad case, the vop header just at the last 4 byte of the packet, i can't know what frame it is
	VIDEO_PKT_C_VOS,
	VIDEO_PKT_C_VOS_Bitstream,
	PARAM_ERROR
	
	};


typedef struct
{
	bool find_vop;
	int vop_start_offset;
} VOP_LOCATE;


typedef struct
{
	int vos_start;
	int vos_end;
	int vos_real_size;
} VOS_LOCATE;


typedef enum
{
	VOS_RIGHT = 0,
	VOS_WRONG = -1,
	
}VOS_CHECK_RESULT;


typedef struct
{	
int width;
int height;
int Profile_and_Level;
int is_shortheader;
							 
} CHECK_VOS_INFO;

extern bool _gfStopingFlag; //used for codec stop allocate memory in while

//add by mtk80691--end


enum VTMAL_NOTIFY{
	REQ_FORCE_IFRAME,
	NOTIFY_CAMERA_ERROR,
	NOTIFY_CODEC_ERROR,
	NOTIFY_MEMPOOL_FULFILL,
	NOTIFY_SNAPSHOT_DONE,
	NOTIFY_CAMERA_MSG_BEGIN = 0xFF,
	
	NOTIFY_RECORD_MSG_BEGIN = 0x8FF, //Recording NOtify begin
	};

enum HIDEME_FLAG{
	HIDEME_DISABLE,
	HIDEME_PICTURE,
	HIDEME_FREEZE
	};

enum HIDEYOU_FLAG{
	HIDEYOU_DISABLE = 0,
	HIDEYOU_PICTURE,
	};


enum VTMAL_Control_Flag{
	VTMAL_CF_NORMAL,
	VTMAL_CF_PREVIEW_WHEN_INIT,
	VTMAL_CF_PREVIEW_WHEN_STOP,
	VTMAL_CF_PREVIEW_WITH_PICTURE
	};

enum SNAPSHOT_TARGET{
	ST_PEER,
	ST_LOCAL
	} ;
	

enum AUDIO_REC_TYPE{
	ART_AMR,
	ART_AWB,
	ART_WAV,
	} ;


//Copy from  MediaRecord.h--start---so VTService need not inlcude mediarecord.h
//keep equal to MediaRecord.h
enum vt_media_recorder_event_type {
    VT_MEDIA_RECORDER_EVENT_ERROR                    = NOTIFY_RECORD_MSG_BEGIN+1,
    VT_MEDIA_RECORDER_EVENT_INFO                     = NOTIFY_RECORD_MSG_BEGIN+2,
    
     VT_MEDIA_RECORDER_COMPLETE											 = NOTIFY_RECORD_MSG_BEGIN+11,
};

enum vt_media_recorder_error_type {
    VT_MEDIA_RECORDER_ERROR_UNKNOWN                  = 1
};

// The codes are distributed as follow:
//   0xx: Reserved
//   8xx: General info/warning
//
enum vt_media_recorder_info_type {
    VT_MEDIA_RECORDER_INFO_UNKNOWN                   = 1,
    VT_MEDIA_RECORDER_INFO_MAX_DURATION_REACHED      = 800,
    VT_MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED      = 801,
    VT_MEDIA_RECORDER_INFO_COMPLETION_STATUS         = 802,
    VT_MEDIA_RECORDER_INFO_PROGRESS_FRAME_STATUS     = 803,
    VT_MEDIA_RECORDER_INFO_PROGRESS_TIME_STATUS      = 804,
    
    VT_MEDIA_RECORDER_INFO_NO_I_FRAME      					=0x7FFF, //special value for no i frame case
    																															//must equal to MPEG4Writer notify value
};
//Copy from  MediaRecord.h--end


struct AUDIO_REC_Params
{
	 char filename[256];
	 int filetype;
};

enum SENSOR_TYPE{
	MAIN_SENSOR = 0,
	SUB_SENSOR,
};

struct CameraSensor_Param
{
	int SensorType;   //0: main sensor    1:sub Sensor
	int NormalModeFR;  //default 30
	int NightModeFR;   //default 15
};

struct CurSensorFR
{
	int iFrameRate;
	int iMode; // normal or night mode
};
class IVTMultiMediaAdaptor :virtual public RefBase
{
public:

	static sp<IVTMultiMediaAdaptor> CreateVTMultiMediaAdaptor();
	
	static void vtmal_rec_mp4(bool enable,char * filename);

	static void vtmal_yuv_emulator(bool enable,char * filename);

	static void vtmal_enable_audio_rec(bool enable,int filetype, char * filename);
	
	//regisger  VTMAL event listener!
	virtual int SetEventObserver(IMediaEventObserver* pObserver) = 0;

	virtual int AddPeerSource(const sp<IMediaObject>& Video,const sp<IMediaObject>& Audio)= 0;
	
	virtual int AddLocalSink(const sp<IMediaObject>& Video,const sp<IMediaObject>& Audio)= 0 ;

	virtual int RemovePeerSources()= 0;
	
	virtual int RemoveLocalSinks() = 0;
		
	virtual int AddPeerVideoSource(const sp<IMediaObject>& Video)= 0;
	virtual int AddPeerAudioSource(const sp<IMediaObject>& Audio)= 0;
	virtual int AddLocalVideoSink(const sp<IMediaObject>& Video)= 0 ;
	virtual int AddLocalAudioSink(const sp<IMediaObject>& Audio)= 0 ;
		
	virtual int SetSurface(const sp<Surface>& peerviewsurface,const sp<Surface>& localviewsurface,int CameraPreviewWidth = 320,int CameraPreviewHeight = 240) = 0;

	virtual int Init(int  flag = VTMAL_CF_PREVIEW_WHEN_INIT,VTMAL_Datapath id = VTMAL_Datapath_All)= 0;	
	
	virtual int Prepare(VTMAL_Datapath id = VTMAL_Datapath_All)= 0;		
	
	virtual int Start(VTMAL_Datapath id = VTMAL_Datapath_All)= 0;	

	virtual int Stop(int flag = VTMAL_CF_NORMAL,VTMAL_Datapath id = VTMAL_Datapath_All)= 0;

	virtual int Reset(VTMAL_Datapath id = VTMAL_Datapath_All)= 0;

	//for video surface control.  if java mmi need to continue VT call  when view is paused or stoped .
	// pls call this function: HideVideoPreview(),in  java app "onpause" or "onstop" funstion
	virtual int HideVideoPreview()= 0;

	//for video surface control. if java mmi resume,pls call this function: ShowVideoPreview()
	virtual int ShowVideoPreview(const sp<Surface>& peerviewsurface,const sp<Surface>& localviewsurface)= 0;

	virtual bool IsVideoPreviewStarted()= 0;
  
	virtual int Snapshot(const String8* url,const int type = ST_PEER) = 0;

	virtual int Snapshot(sp<IMemory>& buffer,const int type = ST_PEER) = 0;

public: //Get/Set  propertys functions

     //return VTMultiMediaAdaptor_State
	virtual int getState(VTMAL_Datapath id = VTMAL_Datapath_All) const = 0;
	
    	virtual int getCodecCapability(VTMAL_Datapath id/*[in*/,Vector<sp<MetaData> >& supportedType/*[out]*/)= 0;
	

	
	virtual int getEncVOS(sp<IMediaBuffer>& vosdata) = 0;
	
	virtual int CheckVideoPacket(void* vop_pointer, int size, int mode) = 0;
	virtual int CheckVideoPacket(void* vop_pointer, int size,VOS_LOCATE* vos_locate, int mode) = 0;	
	virtual int CheckVideoPacket(void* vop_pointer, int size, int mode,VOS_LOCATE* vos_locate, VOP_LOCATE* vop_info) = 0;
    virtual int CheckVOS(void* vos_pointer,int vos_size,void* p_vos_info) = 0;
	
	
	
	virtual String8 getCameraParameters() = 0;
	
	virtual int setCameraParameters(const String8& params) = 0;

     virtual  int    setVoiceMute(bool fmute)= 0;
	   
     virtual  int    getVoiceMute(bool& state) const= 0;

	virtual int    setMicMute(bool fmute)= 0;
	
	virtual int    getMicMute(bool& state) const= 0;

	virtual int    setHideMe(const int flag, const String8* url = 0)= 0;

	//add by mtk80691 for Hide Peer Picture when peer phone close Camera
	virtual int    setHideYou(const int flag, const String8* url = 0)= 0;
	
	virtual int    getHideMe(int& flag) const = 0;

	//VTService_Command--->cmdid
	virtual int sendCommand(int cmdid,void* param,const int param_lenth)= 0;

	//caemra apis extension
	//same usage with Camera
	 virtual status_t     sendCameraCommand(int32_t cmd, int32_t arg1, int32_t arg2) = 0;

	//return Sensor count
	virtual int GetCameraSensorCount() = 0;
	
	//For Record Video and Audio, AP should set the free space of sdcard to VTMAL first
	virtual void setMaxFileSize(int64_t bytes) = 0;
	
	enum CAMERA_SENSOR
	{
		  CS_MAIN = 0,
		  CS_SUB = 1,		  
	};	

	//set current sensor , 0 is main, 1 is sub sensor
	virtual int SetCameraSensor(int index) = 0;

	//get current sensor , 0 is main, 1 is sub sensor
	virtual int GetCurCameraSensor() = 0;
	
	
public:
	
	IVTMultiMediaAdaptor();
	
	virtual ~IVTMultiMediaAdaptor();
};

}
#endif //_VTMAL_H_
