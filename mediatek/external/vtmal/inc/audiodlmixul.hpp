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
*
*
******************************************************/

#ifndef _VTMAL_AUIDO_DL_MIX_UL_H_
#define _VTMAL_AUIDO_DL_MIX_UL_H_

#include "basetype.h"
#include "mediaobject.hpp"
#include "amr_exp.h"
#include "awb_exp.h"
#include "bli_exp.h"

#include "VTMediaSource.hpp"



namespace videotelephone
{
 //please reference AudioPlayer in Stagefright!!,marked by mingliang zhong
class CAudioDlMixUl:public CMediaObject, public Thread
{
public:
	CAudioDlMixUl();

	virtual ~CAudioDlMixUl();

	
	enum{
		SPEECH_DL,
		SPEECH_UL
		}speech_type;

	enum{
		FM_AMR,	
		FM_AWB,
		FM_WAV
		}file_format;


	static sp<CAudioDlMixUl> getInstance();
  
	void release();
	
	virtual int addBuffer(const sp<IMediaBuffer>& pSample, int speechType) ;

	bool enableSpeechRecord(const char *filename, const char *mime);

	void setAudioSource(sp<CVTMediaSource> audioSource){m_Source = audioSource;}

	virtual int start();
		
	virtual int stop();

	static sp<CAudioDlMixUl> m_hIntance;
	
protected:
	
	virtual bool  threadLoop();

	inline void exitThread();

	mutable LockEx m_Lock;
	
private:

	i16 add16(i16 var1, i16 var2, int *pOverflow);
	int postBuffer(const sp<IMediaBuffer>& pSample);

	IMediaBufferQueue m_DlBuffersQueue;   // buffer queue for  downlink pcm data
	IMediaBufferQueue m_UlBuffersQueue;   // buffer queue for  uplink  pcm data

	//Condition mWaitWork;

	int mSampleRate;
	int mChannelNum;
    int64_t mLatencyUs;
    size_t mFrameSize;

	sp<IMediaBuffer> m_pInputBuffer;
  
	bool mStarted;
	bool mEnableRec;

	char mRecordFileName[256];
	char mRecMime[256];
	int  mRecSize;

	FILE *fileWriter;

    // amr nb encode member variable
	void *mEncIntBuffer;
	void *mEncTmpBuffer;
	unsigned char *mEncBsBuffer;
	unsigned int mEncIntBufSize;
	unsigned int mEncTmpBufSize;
	unsigned int mEncBsBufSize;
	unsigned int mEncPcmBufSize;
	AMR_ENC_HANDLE *mAmrEncHdl;
	AMR_BitRate  mBitRate;
	bool		 mDtx;


	// amr wb encode member variable
	void *mEncIntBuffer_wb;
	void *mEncTmpBuffer_wb;
	unsigned char *mEncBsBuffer_wb;
	unsigned int mEncIntBufSize_wb;
	unsigned int mEncTmpBufSize_wb;
	unsigned int mEncBsBufSize_wb;
	unsigned int mEncPcmBufSize_wb;
	AWB_ENC_HANDLE  *mAmrEncHdl_wb;
	AWB_BITRATE      mBitRate_wb;
	bool		     mDtx_wb;

	//BLI SRC SWIP for 8K pcm sample rate convert to 16k
	BLI_HANDLE *m_pSRCHdl;
	unsigned int mSRCWorkBufSize;
    unsigned int mSRCDataBufSize;
    char *mSRCWorkBuf;
    char *mSRCDataTempBuf;
    char *mSRCDataBuf;
    unsigned int mSrcWriteIdx;
    unsigned int mSrcTempWriteIdx;


	char mQBit;
	char mFrameType;

	LockEx       mLock;

	//For VT media record
	sp<CVTMediaSource> m_Source;

};

}
#endif	// _VTMAL_AUIDO_DL_MIX_UL_H_
