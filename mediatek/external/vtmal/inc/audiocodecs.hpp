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

#ifndef _VTMAL_AUIDO_CODECS_H_
#define _VTMAL_AUIDO_CODECS_H_
#include "amr_vt.h"
#include "mediaobject.hpp"
#include "VTMediaSource.hpp"

namespace videotelephone
{
class CAudioDlMixUl;

class CAmrDecoder:public CMediaObject
{
public:
	CAmrDecoder();
	
	virtual ~CAmrDecoder();
  	
	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType);

	virtual int Receive(const sp<IMediaBuffer>& pSample) ;

	bool EnableUlMixDlRecord(const void *params);
	
	bool StartRecMixAudio(sp<CVTMediaSource>& source); 
	MetaData* getInputMetaData();
	
	bool StopRecMixAudio();
	int64_t  GetSystemTimeUs();
	
protected:
	virtual int OnStart();
	
	virtual int OnStop();

	virtual int OnReset();

private:
	sp<CAudioDlMixUl> m_pAudioDlMixUl;
	
	/*@AMR SWIP refer members */
	AMR_DEC_HANDLE_VT *m_pAmrDecHdl;

	void *m_pIntBuffer;
	void *m_pTmpBuffer;
    short *m_pPcmBuffer;

    unsigned char* m_pBsBuffer;
	
	uint32_t mIntBufferSize;
	uint32_t mTmpBufferSize;
	uint32_t mPcmBufferSize;
	uint32_t mBsBufferSize;

	/*@ For VT media record*/
	sp<CVTMediaSource> m_AudioSource;
	bool m_EnableRecMixAudio;
	int m_frameCount;
	int64_t  m_time_firstframe;
	int64_t m_last_timestamp;
	
};

class CAmrEncoder:public CMediaObject
{
public:
	CAmrEncoder();
	
	virtual ~CAmrEncoder();

	virtual bool CheckFormatIsSupported(const sp<MetaData>& pMediaType);
	
	virtual int Receive(const sp<IMediaBuffer>& pSample) ;
	
protected:
	virtual int OnStop();

	virtual int OnReset();

private:
	/*@AMR SWIP refer members */
	AMR_ENC_HANDLE_VT *m_pAmrEncHdl;

	void *m_pIntBuffer;
	void *m_pTmpBuffer;
    short *m_pPcmBuffer;

    unsigned char* m_pBsBuffer;
	
	uint32_t mIntBufferSize;
	uint32_t mTmpBufferSize;
	uint32_t mPcmBufferSize;
	uint32_t mBsBufferSize;

	AMR_BitRate_VT mEncBitRate;
	bool mEncDtx;
		
};

}

#endif //_VTMAL_AUIDO_CODECS_H_
