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
 * MtkAudioLoud.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements Mtk Audio Loudness
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
 * 08 08 2013 kh.hung
 * Clear loudness parameter.
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
#include <assert.h>
 
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioLoud"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>

#include "AudioCompFltCustParam.h"
#include "MtkAudioLoud.h"

//#define ENABLE_LOG_AUDIO_LOUD
#ifdef ENABLE_LOG_AUDIO_LOUD
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

#define BLOCK_SIZE 512
 
#ifdef FLT_PROFILING 
#include <sys/time.h>
#endif

#define HAVE_SWIP

namespace android {

#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)

MtkAudioLoud::MtkAudioLoud()
{
    Init();
}

MtkAudioLoud::MtkAudioLoud(uint32_t eFLTtype)
{
    Init();
    SetParameter(BLOUD_PAR_SET_FILTER_TYPE, (void *)eFLTtype);
}
void MtkAudioLoud::Init()
{
    mPcmFormat     = 0;
    mpTempBuf      = NULL;
    mpInternalBuf   = NULL;
    mTempBufSize   = 0;
    mInternalBufSize = 0;
    memset(&mInitParam, 0, sizeof(BLOUD_HD_InitParam));
    
    mInitParam.pMode_Param = new BLOUD_HD_ModeParam;
    memset(mInitParam.pMode_Param, 0, sizeof(BLOUD_HD_ModeParam));
    
    mInitParam.pMode_Param->pFilter_Coef_L = new BLOUD_HD_FilterCoef;
    memset(mInitParam.pMode_Param->pFilter_Coef_L, 0, sizeof(BLOUD_HD_FilterCoef));
    
    mInitParam.pMode_Param->pFilter_Coef_R = new BLOUD_HD_FilterCoef;
    memset(mInitParam.pMode_Param->pFilter_Coef_R, 0, sizeof(BLOUD_HD_FilterCoef));
    
    mInitParam.pMode_Param->pCustom_Param = new BLOUD_HD_CustomParam;
    memset(mInitParam.pMode_Param->pCustom_Param, 0, sizeof(BLOUD_HD_CustomParam));
    
    mState = ACE_STATE_INIT;

    mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter = mIsSepLR_Filter = false;

    ALOGD("MtkAudioLoud Constructor\n");
}

MtkAudioLoud::~MtkAudioLoud()
{
    ALOGD("+%s()\n",__FUNCTION__);
    if( mInitParam.pMode_Param->pFilter_Coef_L != NULL )
    {
        delete mInitParam.pMode_Param->pFilter_Coef_L;
        mInitParam.pMode_Param->pFilter_Coef_L = NULL;
    }
    if( mInitParam.pMode_Param->pFilter_Coef_R != NULL )
    {
        delete mInitParam.pMode_Param->pFilter_Coef_R;
        mInitParam.pMode_Param->pFilter_Coef_R = NULL;
    }
    if( mInitParam.pMode_Param->pCustom_Param != NULL )
    {
        delete mInitParam.pMode_Param->pCustom_Param;
        mInitParam.pMode_Param->pCustom_Param = NULL;
    }
    if( mInitParam.pMode_Param != NULL )
    {
        delete mInitParam.pMode_Param;
        mInitParam.pMode_Param = NULL;
    }
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

ACE_ERRID MtkAudioLoud::SetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    //Add constraint to limit the use after open.
    switch (paramID)
    {
        case BLOUD_PAR_SET_FILTER_TYPE:
        {
            mFilterType = (uint32_t)param;
            break;
        }
        case BLOUD_PAR_SET_WORK_MODE:
        {
            mWorkMode = (uint32_t)param;
            switch(mWorkMode)
            {
                case AUDIO_CMP_FLT_LOUDNESS_BASIC:     // basic Loudness mode
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_ENHANCED:     // enhancement(1) Loudness mode
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_ENHANCED;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_AGGRESSIVE:     // enhancement(2) Loudness mode
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_LOUD_FLT;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_AGGRESSIVE;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_LITE:     // Only DRC, no filtering
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_NONE;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_COMP:     // Audio Compensation Filter mode (No DRC)
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC:     // Audio Compensation Filter mode + DRC
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_FLT;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_BASIC;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE:     //HCF
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_COMP_HDP;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
                    break;
                case AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH:
                    mInitParam.pMode_Param->Filter_Mode   = HD_FILT_MODE_AUD_ENH;
                    mInitParam.pMode_Param->Loudness_Mode = HD_LOUD_MODE_NONE;
                    break;
                default:
                    ALOGW("%s() invalide workmode %d\n",__FUNCTION__, mWorkMode);
                    break;
            }
            break;
        }
        case BLOUD_PAR_SET_SAMPLE_RATE:
        {
            mInitParam.Sampling_Rate = (uint32_t)param;
            break;
        }
        case BLOUD_PAR_SET_PCM_FORMAT:
        {
            mPcmFormat = (uint32_t)param;
            mInitParam.PCM_Format = mPcmFormat;
            break;
        }
        case BLOUD_PAR_SET_CHANNEL_NUMBER:
        {
            uint32_t chNum = (uint32_t)param;
            if(chNum > 0 && chNum < 3)
            {  // chnum should be 1 or 2
                mInitParam.Channel = chNum;
            }
            else
            {
                return ACE_INVALIDE_PARAMETER;
            }
            break;
        }
        
        case BLOUD_PAR_SET_USE_DEFAULT_PARAM:
        {
            if (mFilterType == AUDIO_COMP_FLT_AUDENH)
                getDefaultAudioCompFltParam((AudioCompFltType_t)mFilterType, &mAudioParam);  
            else    
                GetAudioCompFltCustParamFromNV((AudioCompFltType_t)mFilterType, &mAudioParam);
            copyParam();
            break;
        }
        case BLOUD_PAR_SET_PREVIEW_PARAM:
        {
            memcpy((void *)&mAudioParam, (void *)param, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
            copyParam();
            break;
        }
        case BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB:
        {
            if (mFilterType == AUDIO_COMP_FLT_AUDIO)
            {
                GetAudioCompFltCustParamFromNV((AudioCompFltType_t)AUDIO_COMP_FLT_AUDIO_SUB, &mAudioParam);
                copyParamSub();
            }
            break;
        }
        case BLOUD_PAR_SET_PREVIEW_PARAM_SUB:
        {
            if (mFilterType == AUDIO_COMP_FLT_AUDIO)
            {
                memcpy((void *)&mAudioParam, (void *)param, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
                copyParamSub();
            }
            break;
        }
        case BLOUD_PAR_SET_SEP_LR_FILTER:
        {
            mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter = mIsSepLR_Filter = (bool)param;
            break;
        }
        case BLOUD_PAR_SET_STEREO_TO_MONO_MODE:
        {
            mInitParam.pMode_Param->S2M_Mode = (uint32_t)param;
            if(mState == ACE_STATE_OPEN)
            {
                BLOUD_HD_RuntimeParam runtime_param;
                runtime_param.Command = BLOUD_HD_CHANGE_MODE;
                runtime_param.pMode_Param = mInitParam.pMode_Param;
                mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
            }
        }
        case BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP:
        {                     
            if(mState != ACE_STATE_OPEN)
                return ACE_INVALIDE_OPERATION;
            BLOUD_HD_RuntimeParam runtime_param;            
            runtime_param.Command = BLOUD_HD_CHANGE_MODE;
            runtime_param.pMode_Param = (BLOUD_HD_ModeParam *) mInitParam.pMode_Param;
            mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);         
            break;
        }
        default:
            ALOGD("-%s() Error\n",__FUNCTION__);
            return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::GetParameter(uint32_t paramID, void *param)
{
    ALOGD("+%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    ALOGD("-%s(), paramID %d, param 0x%x\n",__FUNCTION__, paramID, param);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::Open(void)
{
    int32_t result;
    
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_INIT)
        return ACE_INVALIDE_OPERATION;
#if defined(HAVE_SWIP)
    BLOUD_HD_SetHandle(&mBloudHandle);
#endif
    mBloudHandle.GetBufferSize(&mInternalBufSize, &mTempBufSize, mPcmFormat);
    if( mInternalBufSize > 0 )
    {
        mpInternalBuf = (void *)new char[mInternalBufSize];
    }
    if( mTempBufSize > 0 )
    {
        mpTempBuf = (void *)new char[mTempBufSize];
    }
    result = mBloudHandle.Open(&mBloudHandle, (char *)mpInternalBuf, (const void *)&mInitParam);
    mState = ACE_STATE_OPEN;
    ALOGD("-%s() result %d\n",__FUNCTION__, result);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::Close(void)
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

ACE_ERRID MtkAudioLoud::ResetBuffer(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
        return ACE_INVALIDE_OPERATION;
    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_RESET;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::SetWorkMode(uint32_t chNum, uint32_t smpRate, uint32_t workMode)
{
    ACE_ERRID ret;    
    ALOGD("+%s()\n",__FUNCTION__);
    if(mState != ACE_STATE_OPEN)
    {
        ALOGD("%s(), chNum %d, sampleRate %d, workMode %d\n",__FUNCTION__, chNum, smpRate, workMode);

        if( (ret = SetParameter(BLOUD_PAR_SET_CHANNEL_NUMBER, (void *)chNum)) != ACE_SUCCESS )
            return ret;
        if( (ret = SetParameter(BLOUD_PAR_SET_SAMPLE_RATE, (void *)smpRate)) != ACE_SUCCESS )
            return ret;
        if( (ret = SetParameter(BLOUD_PAR_SET_WORK_MODE, (void *)workMode)) != ACE_SUCCESS )
            return ret;
    }
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

void MtkAudioLoud::copyParam(void)
{
    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max = mAudioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min = mAudioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pMode_Param->pCustom_Param->Filter_First = mAudioParam.bes_loudness_Filter_First;
    mInitParam.pMode_Param->pCustom_Param->Att_Time = mAudioParam.bes_loudness_Att_Time;
    mInitParam.pMode_Param->pCustom_Param->Rel_Time = mAudioParam.bes_loudness_Rel_Time;
    mInitParam.pMode_Param->pCustom_Param->Sep_LR_Filter = mIsSepLR_Filter; //0: Use same filter for both L / R
    memcpy((void*)mInitParam.pMode_Param->pCustom_Param->Gain_Map_In, (void*)mAudioParam.bes_loudness_Gain_Map_In, 5*sizeof(char));
    memcpy((void*)mInitParam.pMode_Param->pCustom_Param->Gain_Map_Out, (void*)mAudioParam.bes_loudness_Gain_Map_Out, 5*sizeof(char));
    memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF, (void*)mAudioParam.bes_loudness_hsf_coeff, 90*sizeof(unsigned int));
    memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF, (void*)mAudioParam.bes_loudness_bpf_coeff, 144*sizeof(unsigned int));
    memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_L->LPF_COEF, (void*)mAudioParam.bes_loudness_lpf_coeff, 18*sizeof(unsigned int));

    if (mIsSepLR_Filter == 0)
    {
        memset((void*)mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF, 0, 90*sizeof(unsigned int));
        memset((void*)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, 0, 144*sizeof(unsigned int));
        memset((void*)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, 0, 18*sizeof(unsigned int));
    }
#if 1
    ALOGD("copyParam mIsSepLR_Filter [%d]",mIsSepLR_Filter);
    ALOGD("LHSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][0]);
    ALOGD("LHSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][1]);
    ALOGD("LBPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][0]);
    ALOGD("LBPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][1]);
    ALOGD("RHSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][0]);
    ALOGD("RHSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][1]);
    ALOGD("RBPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][0]);
    ALOGD("RBPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][1]);
    ALOGD("WS_Gain_Max=0x%x, WS_Gain_Min=0x%x, Filter_First=0x%x", mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max, 
        mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min , mInitParam.pMode_Param->pCustom_Param->Filter_First);
    ALOGD("Att_Time=0x%x, Rel_Time=0x%x", mInitParam.pMode_Param->pCustom_Param->Att_Time, mInitParam.pMode_Param->pCustom_Param->Rel_Time);
    ALOGD("Gain_Map_In [0]=0x%x, Gain_Map_In [1]=0x%x,", mInitParam.pMode_Param->pCustom_Param->Gain_Map_In[0], mInitParam.pMode_Param->pCustom_Param->Gain_Map_In[1]);
    ALOGD("Gain_Map_Out [0]=0x%x, Gain_Map_Out [1]=0x%x,", mInitParam.pMode_Param->pCustom_Param->Gain_Map_Out[0], mInitParam.pMode_Param->pCustom_Param->Gain_Map_Out[1]);
#endif
}

void MtkAudioLoud::copyParamSub(void)
{
    if (mIsSepLR_Filter)
    {
        memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF, (void*)mAudioParam.bes_loudness_hsf_coeff, 90*sizeof(unsigned int));
        memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF, (void*)mAudioParam.bes_loudness_bpf_coeff, 144*sizeof(unsigned int));
        memcpy((void*)mInitParam.pMode_Param->pFilter_Coef_R->LPF_COEF, (void*)mAudioParam.bes_loudness_lpf_coeff, 18*sizeof(unsigned int));        
    }

    ALOGD("copyParamSub mIsSepLR_Filter [%d]",mIsSepLR_Filter);
    ALOGD("LHSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][0]);
    ALOGD("LHSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_L->HPF_COEF[0][0][1]);
    ALOGD("LBPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][0]);
    ALOGD("LBPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_L->BPF_COEF[0][0][1]);
    ALOGD("RHSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][0]);
    ALOGD("RHSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_R->HPF_COEF[0][0][1]);
    ALOGD("RBPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][0], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][0]);
    ALOGD("RBPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][1], &mInitParam.pMode_Param->pFilter_Coef_R->BPF_COEF[0][0][1]);
    ALOGD("WS_Gain_Max=0x%x, WS_Gain_Min=0x%x, Filter_First=0x%x", mInitParam.pMode_Param->pCustom_Param->WS_Gain_Max, 
        mInitParam.pMode_Param->pCustom_Param->WS_Gain_Min , mInitParam.pMode_Param->pCustom_Param->Filter_First);
    ALOGD("Att_Time=0x%x, Rel_Time=0x%x", mInitParam.pMode_Param->pCustom_Param->Att_Time, mInitParam.pMode_Param->pCustom_Param->Rel_Time);
    ALOGD("Gain_Map_In [0]=0x%x, Gain_Map_In [1]=0x%x,", mInitParam.pMode_Param->pCustom_Param->Gain_Map_In[0], mInitParam.pMode_Param->pCustom_Param->Gain_Map_In[1]);
    ALOGD("Gain_Map_Out [0]=0x%x, Gain_Map_Out [1]=0x%x,", mInitParam.pMode_Param->pCustom_Param->Gain_Map_Out[0], mInitParam.pMode_Param->pCustom_Param->Gain_Map_Out[1]);
}

        /* Return: consumed input buffer size(byte)                             */
ACE_ERRID MtkAudioLoud::Process(void *pInputBuffer,   /* Input, pointer to input buffer */
                     uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */ 
                                                        /* Output, length(byte) left in the input buffer after conversion */ 
                     void *pOutputBuffer,               /* Input, pointer to output buffer */
                     uint32_t *OutputSampleCount)       /* Input, length(byte) of output buffer */ 
                                                        /* Output, output data length(byte) */ 
{
    ALOGV("+%s(), inputCnt %d, outputCnt %d\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount);
    Mutex::Autolock _l(mLock);
    uint32_t block_size_byte, offset_bit, loop_cnt, i, totalCnt, TotalConsumedSample = 0, TotalOuputSample = 0, ConsumedSampleCount;
    int32_t result;
    if(mState != ACE_STATE_OPEN)
    {
        ALOGD("Error");
        return ACE_INVALIDE_OPERATION;
    }
    //Simplify handle (BLOCK_SIZE x N) Samples
    if( mPcmFormat == BLOUDHD_IN_Q1P15_OUT_Q1P15 ) // 16 bits
    {
        // 2-byte, L/R
        offset_bit = 2;
    }
    else //32 bits
    {
        // 4-byte, L/R
        offset_bit = 3;
    }
    block_size_byte = BLOCK_SIZE * (1<<offset_bit);
    if( ((*InputSampleCount & (block_size_byte - 1)) != 0) || ((*OutputSampleCount & (block_size_byte - 1)) != 0) || (*InputSampleCount != *OutputSampleCount))
    {
        ALOGW("-%s(), inputCnt %d, outputCnt %d\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount, block_size_byte);
        ASSERT(0);
    }
    loop_cnt = *InputSampleCount / block_size_byte;
    //ALOGV("+%s(), loop_cnt %d, block_size_byte %d, sample %d %d\n",__FUNCTION__, loop_cnt, block_size_byte, *(int *)pInputBuffer, (*(int *)pInputBuffer)>>16);
    for (i = 0; i < loop_cnt; i++)
    {
        ConsumedSampleCount = block_size_byte;
        *OutputSampleCount = block_size_byte;
        result = mBloudHandle.Process( &mBloudHandle, 
                             (char *)mpTempBuf, 
                             (int *)(pInputBuffer+TotalConsumedSample), 
                             (int *)&ConsumedSampleCount, 
                             (int *)(pOutputBuffer + TotalOuputSample), 
                             (int *)OutputSampleCount);
        ALOGV("result [%d] ConsumedSampleCount [%d] i [%d] loop_cnt [%d]",result,ConsumedSampleCount,i,loop_cnt);
        TotalConsumedSample += ConsumedSampleCount;
        TotalOuputSample += *OutputSampleCount;
    }
    //ALOGV("+%s(), result = %d, loop_cnt %d, block_size_byte %d, sample %d %d\n",__FUNCTION__, result, loop_cnt, block_size_byte, *(int *)pOutputBuffer, (*(int *)pOutputBuffer)>>16);
    *OutputSampleCount = TotalOuputSample;
    *InputSampleCount = TotalConsumedSample;
    ALOGV("-%s(), inputCnt %d, outputCnt %d\n",__FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::Change2ByPass(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
        
    BLOUD_HD_RuntimeStatus runtime_status;

    if(mBloudHandle.GetStatus(&mBloudHandle, &runtime_status)<0)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    else if (runtime_status.State == BLOUD_HD_SWITCHING_STATE)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }

    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_TO_BYPASS_STATE;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioLoud::Change2Normal(void)
{
    ALOGD("+%s()\n",__FUNCTION__);
    Mutex::Autolock _l(mLock);
    if(mState != ACE_STATE_OPEN)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    BLOUD_HD_RuntimeStatus runtime_status;

    if(mBloudHandle.GetStatus(&mBloudHandle, &runtime_status)<0)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    else if (runtime_status.State == BLOUD_HD_SWITCHING_STATE)
    {
        ALOGW("-%s() Line [%d]\n",__FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    BLOUD_HD_RuntimeParam runtime_param;
    runtime_param.Command = BLOUD_HD_TO_NORMAL_STATE;
    mBloudHandle.SetParameters(&mBloudHandle, &runtime_param);
    ALOGD("-%s()\n",__FUNCTION__);
    return ACE_SUCCESS;
}
#else
MtkAudioLoud::MtkAudioLoud(){};
MtkAudioLoud::MtkAudioLoud(uint32_t eFLTtype){};
void MtkAudioLoud::Init(){};
MtkAudioLoud::~MtkAudioLoud(){};
ACE_ERRID MtkAudioLoud::SetParameter(uint32_t paramID, void *param){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::GetParameter(uint32_t paramID, void *param){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::Open(void){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::Close(void){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::ResetBuffer(void){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::SetWorkMode(uint32_t chNum, uint32_t smpRate, uint32_t workMode){return ACE_SUCCESS;}
void MtkAudioLoud::copyParam(void){};
void MtkAudioLoud::copyParamSub(void){};
ACE_ERRID MtkAudioLoud::Process(void *pInputBuffer,   /* Input, pointer to input buffer */
                     uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */ 
                                                        /* Output, length(byte) left in the input buffer after conversion */ 
                     void *pOutputBuffer,               /* Input, pointer to output buffer */
                     uint32_t *OutputSampleCount)       /* Input, length(byte) of output buffer */ 
                                                        /* Output, output data length(byte) */ 
{return ACE_SUCCESS;}


ACE_ERRID MtkAudioLoud::Change2ByPass(void){return ACE_SUCCESS;}
ACE_ERRID MtkAudioLoud::Change2Normal(void){return ACE_SUCCESS;}

#endif

}//namespace android

