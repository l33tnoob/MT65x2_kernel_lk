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

#ifndef VIDEO_TELEPHONE_MALSTUB_H
#define VIDEO_TELEPHONE_MALSTUB_H

#include "IVTSInterface.h"
#include "vtmal.hpp"
#include "minterface.hpp"
#include "peersources.hpp"
#include "localsinks.hpp"

using namespace videotelephone;

namespace VTService{
	typedef enum VTSMALSTATUS{
		VTS_MAL_OPENED,
		VTS_MAL_INITED,
		VTS_MAL_STARTED,
		VTS_MAL_ERROR
	}VTSMALSTATUS;

	#define VTMALLocalBufNr 12
	#define VTMALPeerBufNr	3

	class CEventObserverHandle;

	enum VIDFRAMETYPE{
			UNKNOWN_FRAME,
			I_FRAME,
			P_FRAME,
			FIRST_FRAME,
			C_FRAME,
			END_VIDEO_FRAME
	};
	
	enum VideoPacket_Type{
		VIDEO_PKT_I_Frame = 0,
		VIDEO_PKT_P_Frame,
		VIDEO_PKT_C_Packet,
		VIDEO_PKT_VOS,
		VIDEO_PKT_VOS_BS,
		VIDEO_PKT_ERROR,
		VIDEO_PKT_C_VOP_I_Frame,
		VIDEO_PKT_C_VOP_P_Frame,
		VIDEO_PKT_C_VOP_ERROR_Frame, //B or S frame
		VIDEO_PKT_C_VOP_UNKNOW_Frame, //this is for Coolpad case, the vop header just at the last 4 byte of the packet, i can't know what frame it is
		VIDEO_PKT_C_VOS,
		VIDEO_PKT_C_VOS_BS,
		PARAM_ERROR
	};
		
	class IVTSCoreCallback : virtual public RefBase{
		public:
			virtual void notifyCallbackForMAL(const int msgId, int32_t ext1, int32_t ext2){};
			virtual ~IVTSCoreCallback(){};
			IVTSCoreCallback(){};
			virtual int sendCmdForCallback(int msgId, int arg1, int arg2, String8& arg3) = 0;
	};
	
	class IVTSMALStub:virtual public RefBase{
	public:
		virtual status_t initVTSMAL() = 0;
		virtual status_t startVTSMAL() = 0;
		virtual status_t stopVTSMAL() = 0;
		virtual status_t closeVTSMAL() = 0;	
		virtual sp<IVTSCoreCallback> getVTSCore() = 0;
		virtual ~IVTSMALStub(){};
	};
	
	class CEventObserverHandle
	{
		friend class VTSMALDroidStub;
		public:
			CEventObserverHandle(){}
			
			~CEventObserverHandle(){}
			void setUser(wp<IVTSMALStub> user){
				mUser = user;	
			}
			
			void HandleEventNotify(int msg, int params1,int params2);
		private:
			wp<IVTSMALStub> mUser;
	};

	typedef struct MALLocalFrameQueue{
		sp<VTSMALMBuffer> queue[VTMALLocalBufNr];
		int readIdx;
		int writeIdx;
		Mutex lock;
		sem_t mSemRead;
		sem_t mSemWrite;
		bool mIsValid;
	}MALLocalFrameQueue;
	
	typedef struct MALPeerFrameQueue{
		sp<VTSMALMBuffer> queue[VTMALPeerBufNr];
		sp<MetaData> metadataQueue[VTMALPeerBufNr];
		int index;
		int offset;
		Mutex lock;
		sem_t mSem;
		bool mIsValid;
	}MALPeerFrameQueue;

	typedef struct VTSFrame{
		sp<VTSMALMBuffer> data;
		union{
			VTSVFrameInfo vidInfo;
			VTSAFrameInfo audInfo;
		};
	}VTSFrame;

	#define VTS_AUDIO IDataListener::VTDT_AUDIO
	#define VTS_VIDEO IDataListener::VTDT_VIDEO
	
	class VTSMALDroidStub: public IVTSMALStub {
			public:
				VTSMALDroidStub();
				VTSMALDroidStub(const sp<IVTSCoreCallback>& VTSCore);
				virtual ~VTSMALDroidStub(){};
				status_t initVTSMAL(const sp<VTSSurface> & localSurface, 
												const sp<VTSSurface> & peerSurface,
												const int flag,
												const VTSString & hideMeImgUrl);
				virtual status_t initVTSMAL();
				virtual status_t startVTSMAL();
				virtual void cancelStartVTSMAL();
				virtual status_t stopVTSMAL();
				virtual status_t closeVTSMAL();	
				status_t startVTSMALDataPath(int path, ChannelConfig channelConfig);
				int getVTSMALDataPathStatus(int path);
				void preStopAllDataPath();
				void deQueueLocalFrame(int flag);
				void getNextLocalAudioFrame(VTSFrame * frame);
				void getNextLocalVideoFrame(VTSFrame * frame, bool isVOS);
				int transferContinueType(int type);

				bool PushNextVFrameData(VTSQueueNode *node, void * buffer, int size, int type, int start);
				bool PushNextVFrameData(VTSQueueNode *node, void * buffer, int size, int type);
				bool PushNextAFrameData(VTSQueueNode *node, void * buffer, int size, int type);
				
				sp<VTSMAObject> getMALObject();
				sp<IVTSCoreCallback> getVTSCore();
				int getStatus();
				int getVOS(void * buffer, int *size);	
				void invalidLocalQueue(int type);
				void invalidPeerQueue(int type);
				void enQueueLocalFrame(int type, const sp<VTSMALMBuffer>& data);
				
				void restartPeerQueue();
				int setCameraClosed(int flag);
				
				////////for test////////////////////////////////////////////////////////////////////
				void sinkSendNextAudioFrame();
				void sinkSendNextVideoFrame();
			private:
				sp<VTSMAObject> mVTMediaAdaptor;
				wp<IVTSCoreCallback> mVTSCore;
				int mMalStatus;
				int mMalDataPathStatus[VTS_PATH_NR];
				
				int cameraClosed;
				int bLockPeerVideoInvokedStub;
				
				int mPathStartedNr;
				sem_t mSemVTSMALStarted;
				VTSString mHideMeImgUrl;
				int mHideMeFlag;
				int mPeerVideoType;
				int mLocalVideoType;
				sp<VTSSurface> mPeerSurface;
				sp<VTSSurface> mLocalSurface;
				sp<CPeerVideoSource> mPeerVideo;
				sp<CPeerAudioSource> mPeerAudio;
				sp<CLocalVideoSink> mLocalVideo;
				sp<CLocalAudioSink> mLocalAudio;
				sp<IDataListener> mDataListener;
				CEventObserverHandle mEventObsHandl;
				sp<IMediaEventObserver> mEventObs;
				MALLocalFrameQueue mLocalVQueue;	
				MALLocalFrameQueue mLocalAQueue;
				MALPeerFrameQueue mPeerVQueue;
				MALPeerFrameQueue mPeerAQueue;
				Mutex mLock;
				void initLocalQueue(MALLocalFrameQueue* queue);
				void initPeerQueue(MALPeerFrameQueue* queue);
				void destoryPeerQueue(MALPeerFrameQueue* queue);
				void destoryLocalQueue(MALLocalFrameQueue* queue);
				void invalidPeerQueue(MALPeerFrameQueue* queue);
				void invalidLocalQueue(MALLocalFrameQueue* queue);
				void dequeueAllLocalData(MALLocalFrameQueue* queue);
				void dequeueAllPeerData(MALPeerFrameQueue* queue);
	};

	class CDataListener:public IDataListener
	{

	public:
		CDataListener():IDataListener()
		{
		}
		CDataListener(const sp<VTSMALDroidStub>&user):IDataListener()
		{
			mUser = user;
		}
		virtual ~CDataListener()
		{
		}

		virtual void postData(int type,const sp<VTSMALMBuffer>& data);
	private:
		wp<VTSMALDroidStub> mUser;
	};

}
#endif