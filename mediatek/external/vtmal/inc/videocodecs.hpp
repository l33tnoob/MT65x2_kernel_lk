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

#ifndef _VTMAL_VIDEO_CODECS_H_
#define _VTMAL_VIDEO_CODECS_H_
#include "mediaobject.hpp"

#include "vtmal.hpp"

//add by mtk80691 
#include "UVTSwDecApi.h"

#ifndef SW_MP4_ENC_SUPPORT
#define SW_MP4_ENC_SUPPORT
#endif

#ifndef SW_UVT_ENC_SUPPORT
#define SW_UVT_ENC_SUPPORT
#endif

#ifndef _VIDEO_ARCHI_V2_
#define _VIDEO_ARCHI_V2_
#endif

#include "UVTSwEncApi.h"


//#include "3gpfilewriter.hpp"
#include "custom_vt_video_enc_type.h"
#include "VTMediaSource.hpp"

namespace videotelephone
{

///////
 //MP4SwDecInst decInst=NULL;
extern sp<IMediaBufferAllocator> gBufferAllocator;
extern sp<IMediaBufferAllocator> gStreamBufferAllocator;

extern CUSTOM_VT_VENC_PARAM_TABLE_T* gCustomEncParam;

#define SENSOR_NUM 2
#define SENSOR_MODE_NUM 2
#define CODEC_TYPE_NUM 2


/*
#define LITERAL_TO_STRING_INTERNAL(x)    #x
#define LITERAL_TO_STRING(x) LITERAL_TO_STRING_INTERNAL(x)
#define CHECK(x)                                                        \
    LOG_ALWAYS_FATAL_IF(                                                \
            !(x),                                                       \
            __FILE__ ":" LITERAL_TO_STRING(__LINE__) " " #x)

*/

class CMp4Decoder:public CMediaObject
{
public:

	CMp4Decoder();
	
	virtual ~CMp4Decoder();

	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType);

	//virtual Vector<sp<MetaData> > GetSupportedMediaType() ;
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;

	virtual int OnStop();

	virtual int OnReset();

	//add by mtk80691 start
	virtual int Flush() ;  //add by mtk80691


	//check whether the VideoPacket is the first packet of a frame. 
	static int CheckVideoPacket(void* vop_pointer, int size, int mode) ; 
 	static int CheckVideoPacket(void* vop_pointer, int size,VOS_LOCATE* vos_locate, int mode) ;  
	static int CheckVideoPacket(void* vop_pointer, int size, int mode,VOS_LOCATE* vos_locate, VOP_LOCATE* vop_locate );
	
	static int CheckVOS(void* vos_pointer,int vos_size,void* p_vos_info);

	sp<IMemory> GetHideYouMem()const;
	sp<IMemory> GetDMLockMem()const;

	int32_t m_headret;
	
	UVTSwDecInst m_decInst;
	
	UVTSwDecInput m_decInput;
	
	UVTSwDecOutput m_decOutput;
	
	UVTSwDecInfo m_decInfo;
	
	int32_t m_frameCount;
	
	int32_t m_frameCountCircle;

	int32_t m_codecInitRet;

	int32_t  m_headerErrorCount; // The times of header parse error happens
	
	int32_t  m_VOPErrorCount; // The times of vop decode error happens
	int m_is_MPEG4;
	int m_DecFrame_Interval;

	//For Hide you function, provide one extra mem for hide you picture
	//because surfaceflinger will re-post buffer at the last address when refresh, so the image at the  last address should not be changed when enable hide you
	sp<IMemory> m_hideyou_mem;
	
	//For DM lock function, provide one extra mem for DM lock picture,
	//because surfaceflinger will re-post buffer at the last address when refresh, so the image at the  last address should not be changed when enable dm lock
	sp<IMemory> m_dmlock_mem;

	//add by mtk80691 end
	
#ifdef VTMAL_DUMP_RECEIVE_BITSTREAM
	FILE * p_mReceiveBsFd;
#endif	
protected:
	virtual bool AllocateMemoryPool(const sp<MetaData>& pMediaType);

private:
	sp<MPEG4Writer> m_mp4FileWriter;
	//sp<IMediaTrack> m_videoTrack;

public:
	//bool EnableRecMp4File(const char *filename);

	//for Record Media(Video and  Audio) feature
	bool StartRecDlVideo(sp<MPEG4Writer>& pMp4Writer); 
	bool StartRecDlVideo(sp<CVTMediaSource>& source); 
	bool StartRecDlVideo(int cmdid,sp<CVTMediaSource>& source); 
	bool StopRecDlVideo();
	bool m_EnableRecDlVideo;
	//sp<MediaSource> m_videoSource;
	sp<CVTMediaSource> m_videoSource;
	int m_RecCmdid;
	sp<CSystemTime> m_pSystemTimeClock;

	virtual int OnStart();
	
	//sp<MetaData> getCurInputMetaData();
	MetaData* getInputMetaData();

	int64_t GetSystemTimeUs();
	int64_t m_time_firstframe;
	sp<IMediaBuffer> m_vos_buffer;
	bool m_hasSaveVOS;
	bool m_hasSendVOS;
};

struct VT_Encode_FrameRates
{
	float HighQualityFrameRate;
	float NormalQualityFrameRate;
	float LowQualityFrameRate;
};


enum VT_Encode_Level
{
	HIGHQUALITY,
	NORMALQUALITY,
	LOWQUALITY
};

class CMp4Encoder:public CMediaObject
{
public:

	CMp4Encoder();
	
	virtual ~CMp4Encoder();

	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType);

	//virtual Vector<sp<MetaData> > GetSupportedMediaType() ;
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;

private:
	//add by mtk80691
	UVTSwEncCfg m_EncCfg;
	UVTSwEncInst m_pEncoderHandle;

	UVTSwEncRateCtrl m_RateCtrl;
	UVTSwEncCodingCtrl m_CodingCtrl;

	//encoder capability
	UVTSwEncCapability m_EncCap;

	static int32_t m_MaxVpSize;
	int32_t m_encodeInitRet;
	int32_t m_encodeConfigRet;
	int32_t m_StreamStartRet;
	int32_t m_StreamEndRet;
	int32_t m_StreamReleaseRet;
	
	int32_t is_MPEG4;
	int m_iCodecType;
	int32_t is_vos_send;
	int32_t isFirstCall;

	//for calculate which frame should drop
	int32_t m_frameCountReceive;
	int32_t m_DropBaseNum;
	
	//int32_t m_frameCountSend;
	int32_t m_p_frameCount; // the p frame number form last I frame

	int32_t m_intraVopRate;

	int32_t m_SampleRate;
	int32_t m_BitRate;
	int64_t m_timeStamp;
	int32_t m_ForceIFrame;

	int32_t m_timeIncr;
	int32_t m_h263frmRateDenom;

	int m_TimeIncrResolution[CODEC_TYPE_NUM];


	int is_Reset;

	int m_FramCount_pushData;


	//When the  frameRate of Sensor  and  three level for VT   is not the default values 
	
	//struct VT_Encode_FrameRates  m_vtEncFrameRates[SENSOR_NUM][SENSOR_MODE_NUM][CODEC_TYPE_NUM];
	
	struct VT_Encode_FrameRates  m_vtEncFRArray[CODEC_TYPE_NUM][SENSOR_NUM][SENSOR_MODE_NUM];
	
	VT_Encode_FrameRates m_CurEnc_FR; // Now three level
	
	CUSTOM_VT_VENC_PARAM_TABLE_T m_CurEnc_Param;
	
	CameraSensor_Param m_MainSensor;
	CameraSensor_Param m_SubSensor;
	
	int m_CurSensorType;  //Now Sensor parameter  o for main sensor , 1 for subsensor
	int m_CurSensorMode;  // 0 normal mode   1 night mode
	int m_CurSensorFrameRate;
	int m_CurEncLevel;
	float m_lastFrameRate;

	int m_LastSensorFrameR;
	bool is_NeedDropFrame;
#ifdef VTMAL_USE_YUV_FILE_FROM_CC
	
		FILE* m_yuv_Handle;
		int m_yuv_file_number;
		int m_theNum_yuv;
	
#endif

	//add by mtk80691 11-01-07 for record timestamp
	int64_t m_last_timestamp;

//	int m_frameCountSend;
	//int m_frameCountSendToSink;

	bool OpenEncoder();
	bool InitEncoder();
	bool ConfigureEncoder();
	bool StartEncoder();
	//bool EncoderEncode(UVTSwEncIn * pEncIn,UVTSwEncStreamSlice *pSlice,int32_t *pflag);
	bool SplitGOBs(UVTSwEncStreamSlice *pSlice,UVTSwEncStreamSlice *pbackup_Slice);
	bool SendBitstreamSample(UVTSwEncStreamSlice *pbackup_Slice);
	bool _ReleaseBitstreamBuffer(UVTSwEncStreamSlice * Slice);

	bool  _ReinitCodec();


	/*****************************************************
	* Calculate GCD(Greatest Common Divisor) of two numbers
	*****************************************************/
	int _FindGCD(int iN1,int iN2);
	
	/*****************************************************
	* Calculate LCM(Least Common Multiple) of two numbers
	*****************************************************/
	int _FindLCM(int iN1,int iN2);

	// Cacluate the Time Incrment Resolution , The LCM of all Frame Rates
	int _CalcCodecFrameRatesLCM(VT_Encode_FrameRates * pVTEncFrameRates,int iNum);

public:
	bool  ChangeVideoQuality(float frameRate);
	bool ChangeVideoQuality(int iCodecLevel);
	bool ForceEncIFrame();
	static bool SetMaxVpSize(int size);
	bool GetVOS(sp<IMediaBuffer>& pVOS);
	
	
	//inline bool ShouldDropFrame(int framNum);
	inline bool ShouldDropFrame(int iSourceFrameR,int framNum,float iTargetFrameR );
	

	int PushData();
	

	//for Customer table
	void SetCameraSensorPara(CameraSensor_Param & sCameraSensorParm);
	bool SetCurSensorFrameRate(int SensorType,int iFrameRate,int bMode);
	//ol SetCurSensorFrameRate(int iFrameRate);
	//bool ChangeToNightMode(int iNightModeFrameRate);  //need reset codec
	bool CheckFrameRatePairs(int iSensorFrameRate,bool isNightMode, VT_Encode_FrameRates* sEncoderFrameRates );
	
	//Check whether the customer setting of frame rates is conform to Spec os operator
	bool CheckCustomSettingFrameRates(int iSceneMode,VT_Encode_FrameRates* pEncoderFrameRates);
	void GetAndCheckCustomFR();

	
	bool GetFrameRatesPairs(int SensorType,int iSensorMode,int iCodecType, VT_Encode_FrameRates* pEncoderFrameRates );

	bool GetCurEncFR(int iCodecType,VT_Encode_FrameRates* pVTEncFRs, float* fFrameRates);
	bool GetCusSetCodecParam(CUSTOM_VT_VENC_PARAM_TABLE_T* tCusVTCodecParam);
	//bool CheckCusSetOnCodecParam(CUSTOM_VT_VENC_PARAM_TABLE_T* tCusVTCodecParam);
	bool _CompareCodecParams(const CUSTOM_VT_VENC_PARAM_TABLE_T& tCusVTCodecParam1, const CUSTOM_VT_VENC_PARAM_TABLE_T& tCusVTCodecParam2);
protected:
	virtual bool AllocateMemoryPool(const sp<MetaData>& pMediaType);
	virtual int OnStop();
	virtual int OnStart();

	virtual int OnReset();

	//add by mtk80691 start
	virtual int Flush() ;  //add by mtk80691

	int64_t GetSystemTimeUs();

	
private:

class SendThread :public Thread
{
public:
	SendThread(CMp4Encoder* p):Thread(false)
	{
		_p = p;
		LOGI("[VTMAL]CMp4Encoder @ SendThread _p = 0x%x",_p);
		m_iSignalCount = 0;
		m_sentSignal = false;
	}
	
	virtual ~SendThread()
	{
		Exit();
	}

	void SendSignal()
	{
		Mutex::Autolock _l(m_LockCondition);
		// can't add this lock--may be cause dead lock of m_lock m_LockCondition
		m_sentSignal = true;
		mWaitWork.signal();
	}

	void Exit()
	{
		{
			Mutex::Autolock _l(m_LockCondition);
			LOGI("[VTMAL][SendThread]@Thread Exit--start");
			requestExit();
			LOGI("[VTMAL][SendThread]@Thread Exit--end");
			m_sentSignal = true;
			mWaitWork.signal();
		}
		requestExitAndWait();
		LOGI("[VTMAL][SendThread]@Thread Exit--real exit");
	}


	//add vtmal test case here!!!!
	virtual bool  threadLoop()
	{
		VTMAL_LOGINFO;
		while (!exitPending())
		{
			m_iSignalCount ++;
			if(m_iSignalCount == 1)
			{
				VTMAL_LOGINFO;
			}
			{ //avoid m_Lock and m_LockCondition deadlock
				//so free m_LockCondition first before try to lock m_Lock in PushData()
				Mutex::Autolock _l(m_LockCondition);
				if(!m_sentSignal)
					mWaitWork.wait(m_LockCondition);
				m_sentSignal = false;
				if(m_iSignalCount == 1)
				{
					VTMAL_LOGINFO;
				}
				if(exitPending())
				{
					VTMAL_LOGINFO;
					return false;
				}
			}			
			if(_p)
			{
				_p->PushData();
			}
			else
			{
				VTMAL_LOGERR;
				LOGE("[VTMAL]CMp4Encoder @ SendThread _p is NULL!!! = 0x%x",_p);
				//CHECK(_p);
			}

		}
		VTMAL_LOGINFO;
		return false;
	}

private:
	CMp4Encoder* _p;
	Condition mWaitWork;	
	mutable Mutex m_LockCondition;
	int m_iSignalCount;
	bool m_sentSignal;
	
};
	
	sp<IMediaBufferAllocator> m_CameraBufferAllocator;
	IMediaBufferQueue m_CameraBuffersQueue;
#ifndef VT_DISABLE_RECORDING_MODE
	IMediaBufferQueue m_CameraBuffersQueue4AsyncRelease;
#endif //VT_DISABLE_RECORDING_MODE
	
	sp<SendThread> m_SendThread;
};
}
#endif //_VTMAL_VIDEO_CODECS_H_
