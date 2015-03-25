/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
 *
 * Filename:
 * ---------
 * MtkAudioBitConverter.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements Audio Bit Converter
 *
 * Author:
 * -------
 *   JY Huang (mtk01352)
 *
 *------------------------------------------------------------------------------
 * $Revision: #2 $
 * $Modtime:$
 * $Log:$
 *
 * 08 07 2013 kh.hung
 * Add 32 bits version.
 *
 *******************************************************************************/

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioBitConverter"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>
extern "C" {
#include "../bessound/BesSound_exp.h"
}
#include "MtkAudioBitConverter.h"
//#define ENABLE_LOG_AudioCompensationFilter
#ifdef MtkAudioBitConverter
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

#define HAVE_SWIP

#ifdef FLT_PROFILING 
#include <sys/time.h>
#endif

namespace android {

MtkAudioBitConverter::MtkAudioBitConverter()
{
    mPcmFormat     = 0xFFFF;
    mChannelNum    = 0;
    mSampleRate    = 0;
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mState = ACE_STATE_INIT;
    ALOGD("MtkAudioBitConverter Constructor\n");
}

MtkAudioBitConverter::MtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format)
{
    mPcmFormat  = format;
    mChannelNum = channel_num;
    mSampleRate = sampling_rate;
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mState = ACE_STATE_INIT;
    ALOGD("MtkAudioBitConverter Constructor, SR %d, CH %d, format %d\n", sampling_rate, channel_num, format);
}

MtkAudioBitConverter::~MtkAudioBitConverter()
{
    ALOGD("+%s()\n",__FUNCTION__);
    if(mpTempBuf != NULL)
    {
        delete mpTempBuf;
        mpTempBuf = NULL;
    }
    if(mpInternalBuf != NULL)
    {
        delete mpInternalBuf;
        mpInternalBuf = NULL;
    }
    ALOGD("-%s()\n",__FUNCTION__);
}

ACE_ERRID MtkAudioBitConverter::SetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    if(mState == ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;

    switch(paramID) {
        case BCV_PAR_SET_PCM_FORMAT:
        {
            uint32_t pcm_format = (uint32_t)param;
            if( ((pcm_format >= BCV_UP_BIT) && (pcm_format <= BCV_UP_BIT_END)) ||
                ((pcm_format >= BCV_DOWN_BIT) && (pcm_format <= BCV_DOWN_BIT_END)) )
            {
                mPcmFormat = pcm_format;
            }
            else
            {
                return ACE_INVALIDE_PARAMETER;
            }
            break;
        }
        case BCV_PAR_SET_SAMPLE_RATE:
        {
            //To do : Add Check 
            mSampleRate = (uint32_t)param;
            break;
        }
        case BCV_PAR_SET_CHANNEL_NUMBER:
        {
            if( (uint32_t)param == 2 || (uint32_t)param == 1)
            {
                mChannelNum = (uint32_t)param;
            }
            else 
            {
                return ACE_INVALIDE_PARAMETER;
            }
            break;
        }
        default:
            return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::GetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    switch(paramID) {
        case BCV_PAR_GET_PCM_FORMAT:
        {
            param = (void *)mPcmFormat;
            break;
        }
        case BCV_PAR_SET_SAMPLE_RATE:
        {
            param = (void *)mSampleRate;
            break;
        }
        case BCV_PAR_SET_CHANNEL_NUMBER:
        {
            param = (void *)mChannelNum;
            break;
        }
        default:
            return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::Open(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_INIT)
        return ACE_INVALIDE_OPERATION;
    if( mPcmFormat != 0xFFFF && mChannelNum != 0 && mSampleRate != 0)
    {
        if( mPcmFormat >= BCV_DOWN_BIT && mPcmFormat < BCV_DOWN_BIT_END) // Use Limiter
        {
#if defined(HAVE_SWIP)
            uint32_t transformPcmFormat; // transform to Limiter's PcmFormat
            transformPcmFormat = mPcmFormat - BCV_DOWN_BIT;
            Limiter_GetBufferSize( &mInternalBufSize, &mTempBufSize, mPcmFormat);
            if( mInternalBufSize > 0 )
            {
                mpInternalBuf = (void *)new char[mInternalBufSize];
            }
            if( mTempBufSize > 0 )
            {
                mpTempBuf = (void *)new char[mTempBufSize];
            }
            mLimiterInitPar.Channel = mChannelNum;
            mLimiterInitPar.Sampling_Rate = mSampleRate;
            mLimiterInitPar.PCM_Format = transformPcmFormat;
            Limiter_Open( &mLimiterHandler, mpInternalBuf, &mLimiterInitPar );
#endif
        }
        else // Use Shifter
        {
            // Do nothing
        }
    }
    mState = ACE_STATE_OPEN;
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::Close(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;
    if(mpTempBuf != NULL)
    {
        delete mpTempBuf;
        mpTempBuf = NULL;
    }
    if(mpInternalBuf != NULL)
    {
        delete mpInternalBuf;
        mpInternalBuf = NULL;
    }
    mState = ACE_STATE_INIT;
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::ResetBuffer(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if( mPcmFormat >= BCV_DOWN_BIT && mState == ACE_STATE_OPEN) // Use Limiter
    {
#if defined(HAVE_SWIP)
        Limiter_Reset( mLimiterHandler );
#endif
    }
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}
        /* Return: consumed input buffer size(byte)                             */
ACE_ERRID MtkAudioBitConverter::Process(void *pInputBuffer,   /* Input, pointer to input buffer */
                     uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */ 
                                                        /* Output, length(byte) left in the input buffer after conversion */ 
                     void *pOutputBuffer,               /* Input, pointer to output buffer */
                     uint32_t *OutputSampleCount)       /* Input, length(byte) of output buffer */ 
                                                        /* Output, output data length(byte) */ 
{
    ALOGV("+%s(), inputCnt %d, outputCnt %x\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;
#if defined(HAVE_SWIP)
    if( mPcmFormat >= BCV_DOWN_BIT && mPcmFormat < BCV_DOWN_BIT_END) // Use Limiter
    {
        Limiter_Process( mLimiterHandler,
                        (char *)mpTempBuf,
                        pInputBuffer,
                        InputSampleCount,
                        pOutputBuffer,
                        OutputSampleCount );
    }
    else if(mPcmFormat < BCV_UP_BIT_END)// Use shifter
    {
        Shifter_Process(pInputBuffer, InputSampleCount, pOutputBuffer, OutputSampleCount, mPcmFormat);
    }
#endif
#if 0
    if(mPcmFormat >= BCV_SIMPLE_SHIFT_BIT && mPcmFormat < BCV_SIMPLE_SHIFT_BIT_END)// Simply shift bit
    {
        uint32_t in_cnt, out_cnt, cnt;
        if(mPcmFormat == BCV_IN_Q1P31_OUT_Q1P15)
        {
            int16_t *outBuf;
            int32_t *inBuf;
            in_cnt = (*InputSampleCount) >> 2;
            out_cnt = (*OutputSampleCount) >> 1;
            outBuf = (int16_t *)pOutputBuffer;
            inBuf = (int32_t *)pInputBuffer;
            if(in_cnt >= out_cnt)
                cnt = out_cnt;
            else
                cnt = in_cnt;
            for(in_cnt = 0; in_cnt < cnt ; in_cnt ++)
            {
                *outBuf = (int16_t)(*inBuf >> 16);
                outBuf++;
                inBuf++;
            }
            *InputSampleCount = *InputSampleCount - (cnt << 2);
            *OutputSampleCount = (cnt << 1);
        }
        else if(mPcmFormat == BCV_IN_Q1P31_OUT_Q9P23)
        {
            int32_t *inBuf, *outBuf;
            in_cnt = (*InputSampleCount) >> 2;
            out_cnt = (*OutputSampleCount) >> 2;
            outBuf = (int32_t *)pOutputBuffer;
            inBuf = (int32_t *)pInputBuffer;
            if(in_cnt >= out_cnt)
                cnt = out_cnt;
            else
                cnt = in_cnt;
            for(in_cnt = 0; in_cnt < cnt ; in_cnt ++)
            {
                *outBuf = (*inBuf >> 8);
                outBuf++;
                inBuf++;
            }
            *InputSampleCount = *InputSampleCount - (cnt << 2);
            *OutputSampleCount = (cnt << 2);
        }
    }
#endif
    
    ALOGV("-%s(), inputCnt %d, outputCnt %x\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

}//namespace android

