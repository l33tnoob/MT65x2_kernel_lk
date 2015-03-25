/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "AudioResamplerMTK"

#include "AudioResampler.h"
#include "AudioResamplermtk.h"
#include <cutils/xlog.h>

namespace android {
// ----------------------------------------------------------------------------
static const int32_t kDefaultInSampleRate = 8000;
static const int32_t kDefaultOutChannelCount = 2;


AudioResamplerMtk::AudioResamplerMtk(int bitDepth,
        int inChannelCount, int32_t sampleRate)
    : AudioResampler(bitDepth, inChannelCount, sampleRate,MTK_QUALITY),
    mOutputBuf(NULL),
    mOutputBufSize(0),
    mBliSrc(NULL)
{

}

AudioResamplerMtk::~AudioResamplerMtk()
{
    delete mBliSrc;
    delete[] mOutputBuf;
}

void AudioResamplerMtk::reset()
{
    SXLOGD("reset");
    if(mBliSrc != NULL)
    {
        mBliSrc->reset();
    }
}

void AudioResamplerMtk::init(int32_t SrcSampleRate)
{
    SXLOGD("init outsampleRate %u, intsampleRate %u",mSampleRate, SrcSampleRate);
    mBliSrc = new Blisrc(kDefaultInSampleRate,mChannelCount, mSampleRate, kDefaultOutChannelCount);
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

void AudioResamplerMtk::setSampleRate(int32_t inSampleRate)
{
    //SXLOGD("setSampleRate %u",inSampleRate); log too much when playback
    if(mBliSrc != NULL)
    {
        mInSampleRate = inSampleRate;
        mBliSrc->setInSampleRate(inSampleRate);
    }
}

status_t AudioResamplerMtk::configBuf(size_t size)
{
    int ret = OK;
    if(mOutputBufSize != size)
    {
        mOutputBufSize = size;
        if(mOutputBuf != NULL )
        {
            delete[] mOutputBuf;
            mOutputBuf = NULL;
        }
        mOutputBuf = new int16_t[mOutputBufSize>>1];
        if(mOutputBuf != NULL)
        {
            memset(mOutputBuf,0,mOutputBufSize);
        }
        else
        {
            ret = NO_MEMORY;
        }
    }
    return ret;
}

void AudioResamplerMtk::resample(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider) {

    if(mOutputBuf == NULL) // size of this buffer can only be determined at this time.
    {
        status_t ret = configBuf( outFrameCount << 2);
        if(ret != OK)
        {
            SXLOGE("resample fail to configBuf, There is no memory");
            return ;
        }
    }
    
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * 2;
    size_t inFrameCount = ((outFrameCount*mInSampleRate)+4)/mSampleRate;

    SXLOGV("starting resample outFrameCount %d, outputSampleCount %d, inFrameCount %d",
          outFrameCount,outputSampleCount,inFrameCount);

    // still have data to ouput
    while(outFrameCount){
        if(mBuffer.frameCount == 0) 
        {
            mBuffer.frameCount = inFrameCount;
            provider->getNextBuffer(&mBuffer);  // get bufferNumber
            if (mBuffer.raw == NULL) {
                SXLOGD("AudioResamplerMtk::resample() can't get buffer");
                reset();
                goto Mtkresample_exit;
            }
        }
        int16_t *in = mBuffer.i16;
        size_t inBufSize = mBuffer.frameCount * mChannelCount * (mBitDepth>>3);
        size_t outBuffSize = outFrameCount <<2;
        int consume =0;
        if(mBliSrc)
        {
            SXLOGV("+resample inBufSize %d, outBufSize %d",inBufSize,outBuffSize);
            consume = mBliSrc->resample(in,&inBufSize, mOutputBuf + outputIndex,&outBuffSize);
            SXLOGV("-resample consume %d, inBufSize %d, outBufSize %d",consume,inBufSize,outBuffSize);
        }
        else
        {
            SXLOGE("Can not Resampler, pad 0");
            consume = inBufSize;
            memset((void*)(mOutputBuf + outputIndex),0,inBufSize);
        }
        outputIndex += outBuffSize >>1;
        outFrameCount -= outBuffSize >>2;
        mBuffer.frameCount = consume>>mChannelCount;
        provider->releaseBuffer(&mBuffer);// release buffer
    }

Mtkresample_exit:
    int32_t vl = mVolume[0];
    int32_t vr = mVolume[1];
    size_t inputIndex = 0;
    while(inputIndex < outputIndex){
        out[inputIndex] += mOutputBuf[inputIndex] * vl;
        inputIndex++;
        out[inputIndex] += mOutputBuf[inputIndex] * vr;
        inputIndex++;
    }
}


AudioResamplerMtk::Blisrc::Blisrc(int inSampleRate, int inChannelCount, int outSampleRate,int outChannelCount)
    :mInSampleRate(inSampleRate),
    mInChannelCount(inChannelCount),
    mOutSampleRate(outSampleRate),
    mOutChannelCount(outChannelCount),
    mWorkBufSize(0),
	mWorkBuf(NULL),
	mHandle(NULL),
	mInitCheck(NO_INIT)
{
    init();
}

AudioResamplerMtk::Blisrc::~Blisrc()
{
    if(mInitCheck == OK )
    {
        BLI_Close(mHandle,NULL);
        delete[] mWorkBuf;
    }
        
}
bool AudioResamplerMtk::Blisrc::initCheck()
{
    return mInitCheck==OK ? true : false;
}

void AudioResamplerMtk::Blisrc::init()
{
    BLI_GetMemSize(mInSampleRate, mInChannelCount, mOutSampleRate, mOutChannelCount, &mWorkBufSize);
    if(mWorkBufSize > 0 )
    {
        mWorkBuf = new int8_t[mWorkBufSize];
        if(mWorkBuf != NULL)
        {
            memset((void*)mWorkBuf,0,mWorkBufSize);
            mHandle = BLI_Open(mInSampleRate, mInChannelCount, mOutSampleRate, mOutChannelCount,(char *)mWorkBuf,NULL);
            if(mHandle != 0)
            {
                mInitCheck = OK;
                return;
            }
            delete[] mWorkBuf;
        }
    }
}

int AudioResamplerMtk::Blisrc::setInSampleRate(int32_t inSampleRate)
{
    mInSampleRate = inSampleRate;
    return BLI_SetSamplingRate(mHandle,mInSampleRate);
}

int AudioResamplerMtk::Blisrc::resample(void * inBuf, size_t * pInSize, void *outBuf, size_t * pOutSize)
{
    return BLI_Convert(mHandle,(short*)inBuf, pInSize, (short*)outBuf, pOutSize);
}

int AudioResamplerMtk::Blisrc::reset()
{
    return BLI_Reset(mHandle);
}


// ----------------------------------------------------------------------------
}
; // namespace android

