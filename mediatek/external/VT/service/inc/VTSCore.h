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

#ifndef VIDEO_TELEPHONE_CORE_H
#define VIDEO_TELEPHONE_CORE_H

#include "IVTSService.h"
#include "VTSMALStub.h"
#include "med_vt_struct.h"

//#define VTS_STK_LOOPBACK

namespace VTService{
	#define VTS_FR_NORMAL	10
	#define VTS_FR_SHARP	15
	#define VTS_FR_LOW		7.5
	
	typedef struct VTSFdSetContext{
			fd_set mSvcSet;
			int mMaxSvcSd;
	}VTSFdSetContext, *PtrVTSFdSetContext;

	
	typedef struct VTSDataPathConfig{
		ChannelConfig channelConfig;
	}VTSDataPathConfig,* PtrVTSDataPathConfig;

	#define min(a,b) ((a)<(b)?(a):(b))
	#define max(a,b) ((a)>(b)?(a):(b))
	
	typedef enum VTSCORESTATUS{
		VTS_IDLE,
		VTS_OPENED,
		VTS_INITED,
		VTS_STARTED,
		VTS_STOPPING,
		VTS_ERROR
	}VTSCORESTATUS;

	typedef enum VTSCORECHECKFLAG{
		VTS_CORE_INIT,
		VTS_CORE_OPEN_START,
		VTS_CORE_OPEN_END,
		VTS_CORE_INIT_START,
		VTS_CORE_INIT_ERROR,
		VTS_CORE_INIT_END,
		VTS_CORE_START_START,
		VTS_CORE_START_START_MAL_ST,
		VTS_CORE_START_START_MAL_SE,
		VTS_CORE_START_START_MAL_SP,
		VTS_CORE_START_END,
		VTS_CORE_STOP_START,
		VTS_CORE_STOP_STOP_STACK,
		VTS_CORE_STOP_STOP_MAL_ST,
		VTS_CORE_STOP_STOP_MAL_SE,
		VTS_CORE_STOP_END,
		VTS_CORE_CLOSE_START,
		VTS_CORE_CLOSE_CLOSE_MAL_ST,
		VTS_CORE_CLOSE_CLOSE_MAL_SE,
		VTS_CORE_CLOSE_CLOSE_MAL_SP,
		VTS_CORE_CLOSE_DEINIT_ST,
		VTS_CORE_CLOSE_DEINIT_SP,
		VTS_CORE_CLOSE_END,
		VTS_CORE_END
	}VTSCORECHECKFLAG;

	typedef enum VTSINTERSTATUSFLAG{
		VTS_CORE_STATUS=0,
		VTS_STACK_STATUS,
		VTS_TCV_STATUS
	}VTSINTERSTATUSFLAG;

	typedef struct VTSMsgHandlerContext{
		int msgId;
		void(*msgHandler)(void*);
	}VTSMsgHandlerContext;

	typedef struct VTSHandlerContext{
		int threadIdx;
		VTSMsgHandlerContext * handlerContext;
		int itemNr;
	}VTSHandlerContext;

	typedef enum VTSChangePeerType{
		VTS_QUALITY = 0,
		VTS_RESOLUTION,
	}VTSChangePeerType;

	typedef enum
	{
	    VTS_VIDEO_H263 = 0,
		VTS_VIDEO_MPEG4 = 1,
	    VTS_VIDEO_TYPE_NR
	}VTSVideoType;

	class IVTSServiceCallBack:virtual public RefBase{
		public:
			virtual sp<IVTSClient> getClient() = 0;
			virtual void notifyCallback(const int msgId, int32_t ext1, int32_t ext2) = 0;
	};
	
	class VTSCore: public IVTSInterface, public IVTSCoreCallback
	{
		public:														
			VTSCore(const sp<IVTSServiceCallBack> & user);
			VTSCore();
			virtual ~VTSCore(){};

			DECLARE_IVTSINTERFACE
			
			virtual int sendCmdForCallback(int msgId, int arg1, int arg2, String8& arg3);
			
			virtual void notifyCallback(const int msgId, int32_t ext1, int32_t ext2);
			virtual void notifyCallbackForMAL(const int msgId, int32_t ext1, int32_t ext2);
			status_t channelConfigCallback(int dataPathIdx, ChannelConfig *channelConfig);
			void setToFDSet(int idx);
			status_t getStatus();
			
			int setIncomingVideoChannelConnected(int flag);
			int getIncomingVideoChannelConnected();
			
			friend void * mtk_vt_dlvp_task(void * arg);
			friend void * mtk_vt_dlap_task(void * arg);
			friend void * mtk_vt_ulvp_task(void * arg);
			friend void * mtk_vt_ulap_task(void * arg);
			friend void * mtk_vt_svc_task(void * arg);
			
			friend void StkVideoGetDecConfigHandl(void* para);
			friend void StkCallActivateCnfHandl(void* para);		
			friend void StkCallDeactivateCnfHandl(void* para);				
			friend void StkCallDiscIndHandl(void * para);			
			friend void StkMediaChannelConfigHandl(void * para);	
			friend void StkAudioSetMaxSkewHandl(void* skew);		
			friend void StkVideoSetLocalQualityHandl(void * para);		
			friend void StkVideoSetH263ResolutionHandl(void* para);			
			friend void StkVideoEncFastUpdateHandl(void* para);
			friend void StkVideoChannelActiveHandl(void* para);
			friend void StkVideoChannelInactiveHandl(void* para);
			
			friend void vt_em_rec_mp4(int arg1, int arg2);
			friend void vt_em_YUV_emulator(int arg1, int arg2);
			friend void vt_em_enable_audio_rec(int arg1, int arg2);
			//for test
			friend void * test_ul_a(void*);
			friend void * test_ul_v(void*);
			sp<VTSMALStub> mVTSMALStub;
		private:
			VTSFdSetContext mFdSetContext;
			
			int mHideMeFlag;
			VTSString mHideMeImageURI;
			
			//int bEnableReplacePeerVideo;
			//VTSString sReplacePeerVideoFilePath;
			
			int incomingVideoChannelConnected;
			
			int mPeerRes ;
			int mPeerQual ;
				
			mutable Mutex mLock;
			sp<VTSSurface> mLocalSurface;
			sp<VTSSurface> mPeerSurface;
			int mCallEndType;
			sp<VTSMAObject> mVTSMAObject;
			VTSDataPathConfig mDataPathsConfig[VTS_DATA_PATH_NR];
			VTSChannelInfo mChannelInfo;
			int mPeerVideoType;
			int mPeerIsShortHeader;
			int mLocalVideoType;
			int mCheckFlag;
			int mStatus;
			int mCoreStatus;
			int mStackStatus;
			int mTcvStatus;
			sem_t mChannelConfigSem;
			sem_t mCallActivateSem;

      int mIsSvcThreadExit;
      
            /* bw ctrl related */
            static int vt_con_sce_file_fd;
            static const char *vt_con_sce_file;
            static const char *vt_mem_bw_ctrl_en;
            static const char *vt_mem_bw_ctrl_dis;

			int mNeedStartDataPath;
			sp<IVTSServiceCallBack> mUser;
			status_t _init();
			status_t _deinit();
			int videoChannelActive(int type);
			int getStkFrameType(void * buf, int size, int type);
            static void bwCtrlSwitch(bool isEnable);
	};
}

#endif
