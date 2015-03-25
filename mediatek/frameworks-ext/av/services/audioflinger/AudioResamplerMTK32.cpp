/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2013. All rights reserved.
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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioResamplerMTK32"

#include "AudioResampler.h"
extern "C" {
#include "AudioResamplerMTK32.h"
}
#include <cutils/xlog.h>

#ifdef DEBUG_MIXER_PCM
#include "AudioUtilmtk.h"
#endif

namespace android {
// ----------------------------------------------------------------------------
static const int32_t kDefaultInSampleRate = 8000;
static const int32_t kDefaultOutChannelCount = 2;

#ifdef DEBUG_MIXER_PCM
static   const char * gaf_mixer_before_src_pcm = "/sdcard/mtklog/audio_dump/mixer_src_before";
static   const char * gaf_mixer_after_src_pcm  = "/sdcard/mtklog/audio_dump/mixer_src_after";
static   const char * gaf_mixer_src_propty     = "af.mixer.src.pcm";

#define MixerDumpPcm(name, propty, tid, value, buffer, size) \
{\
  char fileName[256]; \
  sprintf(fileName,"%s_%d_%p.pcm", name, tid, value); \
  AudioDump::dump(fileName, buffer, size, propty); \
}
#else
#define MixerDumpPcm(name, propty, tid, value, buffer, size)
#endif


AudioResamplerMtk32::AudioResamplerMtk32(int bitDepth, int inChannelCount, int32_t sampleRate)
    : AudioResampler(bitDepth, inChannelCount, sampleRate,MTK_QUALITY_32BIT),
    mBliSrc(NULL)
{
    if (bitDepth == 16) {
        mInFormat = BLISRC_IN_Q1P15_OUT_Q1P31;
    } else if (bitDepth == 24) {
        mInFormat = BLISRC_IN_Q9P23_OUT_Q1P31;
    } else if (bitDepth == 32) {
        mInFormat = BLISRC_IN_Q1P31_OUT_Q1P31;
    } else {
        mInFormat = BLISRC_IN_Q1P31_OUT_Q1P31;
        ALOGE("Unsupported sample format, %d bits", bitDepth);
    }
}

AudioResamplerMtk32::~AudioResamplerMtk32()
{
    if(mBliSrc != NULL)
    {
        delete mBliSrc;
        mBliSrc = NULL;
    }
}

void AudioResamplerMtk32::reset()
{
    SXLOGD("reset");
    if(mBliSrc != NULL)
    {
        mBliSrc->reset();
    }
}

void AudioResamplerMtk32::init(int32_t SrcSampleRate)
{
    SXLOGD("init outsampleRate %u, insampleRate %u", mSampleRate, SrcSampleRate);

    if(SrcSampleRate) {
        mBliSrc = new Blisrc(SrcSampleRate, mChannelCount, mSampleRate, kDefaultOutChannelCount, mInFormat);
    }
    else {
        mBliSrc = new Blisrc(kDefaultInSampleRate, mChannelCount, mSampleRate, kDefaultOutChannelCount, mInFormat);
    }
    if(mBliSrc != NULL)
    {
        SXLOG_ASSERT(!mBliSrc->initCheck(),"Fail to open Blisrc");
        if(!mBliSrc->initCheck()) // fail to init
        {
            delete mBliSrc;
            mBliSrc = NULL;
        }
    }
}

void AudioResamplerMtk32::setSampleRate(int32_t inSampleRate)
{
    //SXLOGD("setSampleRate %u",inSampleRate); log too much when playback
    if(mBliSrc != NULL)
    {
        mInSampleRate = inSampleRate;
        mBliSrc->setInSampleRate(inSampleRate);
    }
}

void AudioResamplerMtk32::resample(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider) {
    
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * 2;

    // still have data to ouput
    while(outFrameCount){
        
        size_t inFrameCount = ((outFrameCount*mInSampleRate)/mSampleRate);
		if(((outFrameCount * mInSampleRate) % mSampleRate) != 0)
		{
			inFrameCount += 1;
		}
        
        SXLOGV("starting resample outFrameCount %d, outputSampleCount %d, inFrameCount %d",
              outFrameCount,outputSampleCount,inFrameCount);
        if(mBuffer.frameCount == 0) 
        {
            mBuffer.frameCount = inFrameCount;
            provider->getNextBuffer(&mBuffer);  // get bufferNumber
            if (mBuffer.raw == NULL) {
                reset();
                break;
            }
        }
        int16_t *in = mBuffer.i16;
        size_t frame_size ;
        if(mInFormat == BLISRC_IN_Q1P31_OUT_Q1P31 ||mInFormat == BLISRC_IN_Q9P23_OUT_Q1P31 )
        {
            frame_size = sizeof(int);
        }
        else
        {
            frame_size = sizeof(short);
        }
        size_t inBufSize = mBuffer.frameCount * mChannelCount * frame_size;
        size_t outBuffSize = outFrameCount <<3;
        int consume =0;
        if(mBliSrc)
        {
            SXLOGV("+resample inBufSize %d, outBufSize %d",inBufSize,outBuffSize);
            consume = mBliSrc->resample(in,&inBufSize, out + outputIndex,&outBuffSize);
            SXLOGV("-resample consume %d, inBufSize %d, outBufSize %d",consume,inBufSize,outBuffSize);
        }
        else
        {
            SXLOGE("Can not Resampler, pad 0");
            consume = inBufSize;
            memset((void*)(out + outputIndex),0,inBufSize);
        }

//ALOGD("KH_SRC %x, In %d, Out %d", mBliSrc, in[0], out[outputIndex]);

        outputIndex += outBuffSize >> 2;
        outFrameCount -= outBuffSize >> 3;
        
        if(mInFormat == BLISRC_IN_Q1P31_OUT_Q1P31 ||mInFormat == BLISRC_IN_Q9P23_OUT_Q1P31 )            
        mBuffer.frameCount = consume>>(mChannelCount+1);
        else
        mBuffer.frameCount = consume>>mChannelCount;
        provider->releaseBuffer(&mBuffer);// release buffer
    }
}


AudioResamplerMtk32::Blisrc::Blisrc(int32_t inSampleRate, int32_t inChannelCount, int32_t outSampleRate, int32_t outChannelCount, int32_t format)
    :mInternalBufSize(0),
  mTempBufSize(0),
  mInternalBuf(NULL),
	mTempBuf(NULL),
	mHandle(NULL),
	mInitCheck(NO_INIT)
{
    mBliParam.in_sampling_rate = inSampleRate;
    mBliParam.in_channel = inChannelCount;
    mBliParam.ou_sampling_rate = outSampleRate;
    mBliParam.ou_channel = outChannelCount;
    mBliParam.PCM_Format = format;
    init();
}

AudioResamplerMtk32::Blisrc::~Blisrc()
{
    if(mInitCheck == OK )
    {
        if(NULL != mInternalBuf) {
            delete[] mInternalBuf;
            mInternalBuf = NULL;
        }
        if(NULL != mTempBuf) {
            delete[] mTempBuf;
            mTempBuf = NULL;
        }
    }
        
}
bool AudioResamplerMtk32::Blisrc::initCheck()
{
    return mInitCheck==OK ? true : false;
}

void AudioResamplerMtk32::Blisrc::init()
{
    int32_t result;
    
    result = Blisrc_GetBufferSize(&mInternalBufSize, &mTempBufSize, &mBliParam);
    if (result < 0) {
        ALOGE("Blisrc_GetBufferSize error %d", result);
    }
    
    mInternalBuf = new int8_t[mInternalBufSize];
    memset((void*)mInternalBuf, 0, mInternalBufSize);
    mTempBuf = new int8_t[mTempBufSize];
    
    result = Blisrc_Open(&mHandle,(void *)mInternalBuf, &mBliParam);
    if(result >= 0) {
        mInitCheck = OK;
    } else {
        ALOGE("Blisrc_Open error %d, IntBuf %d %x, TempBuf %d %x", result, mInternalBufSize, mInternalBuf, mTempBufSize, mTempBuf);

        if(NULL != mInternalBuf) {
            delete[] mInternalBuf;
            mInternalBuf = NULL;
        }
        if(NULL != mTempBuf) {
            delete[] mTempBuf;
            mTempBuf = NULL;
        }
    }
}

int AudioResamplerMtk32::Blisrc::setInSampleRate(int32_t inSampleRate)
{
    mBliParam.in_sampling_rate = inSampleRate;
    return Blisrc_SetSamplingRate(mHandle, mBliParam.in_sampling_rate);
}

int AudioResamplerMtk32::Blisrc::resample(void * inBuf, size_t *pInSize, void *outBuf, size_t * pOutSize)
{
    int value;
    value = Blisrc_Process(mHandle, (char *)mTempBuf, (void *)inBuf, pInSize, (void *)outBuf, pOutSize);
    MixerDumpPcm(gaf_mixer_before_src_pcm, gaf_mixer_src_propty, gettid(), (int)this, inBuf, value);
    MixerDumpPcm(gaf_mixer_after_src_pcm, gaf_mixer_src_propty, gettid(), (int)this, outBuf, *pOutSize);
    return value;
}

int AudioResamplerMtk32::Blisrc::reset()
{
    return Blisrc_Reset(mHandle);
}


// ----------------------------------------------------------------------------
}
; // namespace android

