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
 * MtkAudioSrc.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements Mtk Audio Ssampl Rate Converter
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

#define HAVE_SWIP

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioSrc"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>
#include "MtkAudioSrc.h"
//#define ENABLE_LOG_AUDIO_SRC
#ifdef ENABLE_LOG_AUDIO_SRC
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif


#ifdef FLT_PROFILING 
#include <sys/time.h>
#endif

namespace android {

MtkAudioSrc::MtkAudioSrc()
{
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mTempBufSize   = 0;
    mInternalBufSize = 0;
    mState = ACE_STATE_INIT;
    ALOGD("MtkAudioSrc Constructor\n");
}

MtkAudioSrc::MtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format)
{
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mTempBufSize   = 0;
    mInternalBufSize = 0;
    mBlisrcParam.PCM_Format = (uint32_t)format;
    mBlisrcParam.in_sampling_rate = input_SR;
    mBlisrcParam.ou_sampling_rate = output_SR;
    mBlisrcParam.in_channel = input_channel_num;
    mBlisrcParam.ou_channel = output_channel_num;
    mState = ACE_STATE_INIT;
    ALOGD("MtkAudioSrc Constructor in SR %, CH %d; out SR %d, CH %d; format %d\n", input_SR, input_channel_num, output_SR, output_channel_num, format);
}

MtkAudioSrc::~MtkAudioSrc()
{
    ALOGD("+%s()\n",__FUNCTION__);
    if(mpTempBuf != NULL)
    {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if(mpInternalBuf != NULL)
    {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
    ALOGD("-%s()\n",__FUNCTION__);
}

ACE_ERRID MtkAudioSrc::SetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    if(mState == ACE_STATE_OPEN && (uint32_t)param != SRC_PAR_SET_INPUT_SAMPLE_RATE) //Only input sampling rate could be update during process.
        return ACE_INVALIDE_OPERATION;
    switch(paramID)
    {
        case SRC_PAR_SET_PCM_FORMAT:
        {
            if((uint32_t)param >= SRC_IN_END)
                return ACE_INVALIDE_PARAMETER;
            mBlisrcParam.PCM_Format = (uint32_t)param;
            break;
        }
        case SRC_PAR_SET_INPUT_SAMPLE_RATE:
        {
            mBlisrcParam.in_sampling_rate = (uint32_t)param;
            break;
        }
        case SRC_PAR_SET_OUTPUT_SAMPLE_RATE:
        {
            mBlisrcParam.ou_sampling_rate = (uint32_t)param;
            if(mState == ACE_STATE_OPEN)
            {
#if defined(HAVE_SWIP)
                Blisrc_SetSamplingRate( mBlisrcHandler, mBlisrcParam.ou_sampling_rate);
#endif
            }
            break;
        }
        case SRC_PAR_SET_INPUT_CHANNEL_NUMBER:
        {
            mBlisrcParam.in_channel = (uint32_t)param;
            break;
        }
        case SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER:
        {
            mBlisrcParam.ou_channel = (uint32_t)param;
            break;
        }
        default:
            return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}


ACE_ERRID MtkAudioSrc::GetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    switch (paramID)
    {
        case SRC_PAR_GET_PCM_FORMAT:
        {
            param = (void *)mBlisrcParam.PCM_Format;
            break;
        }
        case SRC_PAR_GET_INPUT_SAMPLE_RATE:
        {
            param = (void *)mBlisrcParam.in_sampling_rate;
            break;
        }
        case SRC_PAR_GET_OUTPUT_SAMPLE_RATE:
        {
            param = (void *)mBlisrcParam.ou_sampling_rate;
            break;
        }
        case SRC_PAR_GET_INPUT_CHANNEL_NUMBER:
        {
            param = (void *)mBlisrcParam.in_channel;
            break;
        }
        case SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER:
        {
            param = (void *)mBlisrcParam.ou_channel;
            break;
        }
        default:
            return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::Open(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_INIT)
        return ACE_INVALIDE_OPERATION;
    int ret;
#if defined(HAVE_SWIP)
    ret = Blisrc_GetBufferSize( &mInternalBufSize,
                            &mTempBufSize,
                            &mBlisrcParam);

    if(ret < 0){
        ALOGD("Blisrc_GetBufferSize return err %d\n", ret);
        return ACE_INVALIDE_OPERATION;
    }
#endif
    if( mInternalBufSize > 0 )
    {
        mpInternalBuf = (void *)new char[mInternalBufSize];
    }
    if( mTempBufSize > 0 )
    {
        mpTempBuf = (void *)new char[mTempBufSize];
    }
#if defined(HAVE_SWIP)
    Blisrc_Open( &mBlisrcHandler, mpInternalBuf, &mBlisrcParam);
#endif
    mState = ACE_STATE_OPEN;
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::Close(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;
    if(mpTempBuf != NULL)
    {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if(mpInternalBuf != NULL)
    {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
    mState = ACE_STATE_INIT;
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::ResetBuffer(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;
#if defined(HAVE_SWIP)
    Blisrc_Reset(mBlisrcHandler);
#endif
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::Process(void *pInputBuffer,      /* Input, pointer to input buffer */
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
    Blisrc_Process( mBlisrcHandler,
                    (char *)mpTempBuf,
                    pInputBuffer,
                    InputSampleCount,
                    pOutputBuffer,
                    OutputSampleCount);
#endif
    ALOGV("-%s(), inputCnt %d, outputCnt %x\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

}//namespace android

