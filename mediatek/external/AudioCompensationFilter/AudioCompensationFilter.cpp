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
 * AudioCompensationFilter.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements Audio Compensation Filter
 *
 * Author:
 * -------
 *   Tina Tsai (mtk01981)
 *
 *------------------------------------------------------------------------------
 * $Revision: #2 $
 * $Modtime:$
 * $Log:$
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

#include "AudioCompFltCustParam.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "AudioCompensationFilter"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <AudioCompensationFilter.h>
#include <assert.h>
extern "C" {
#include "../bessound/BesSound_exp.h"
#include "../bessound/BesLoudness_exp.h"
}
//#define ENABLE_LOG_AudioCompensationFilter
#ifdef ENABLE_LOG_AudioCompensationFilter
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

//#define FLT_PROFILING
//#define ACF_Test_OnlyMemcpy
//#define FLT_INPUT_BUF_SIZE  16384
//#define FLT_INPUT_BUF_SIZE  12288

#ifdef FLT_PROFILING
#include <sys/time.h>
#endif

namespace android
{
// ----------------------------------------------------------------------------
/* Compensation Filter HSF coeffs       */
/* BesLoudness also uses this coeffs    */

#if 0
//all pass filter
const unsigned int BES_COMPENSATION_HSF[9][4] =
{
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    48000 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    44100 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    32000 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    24000 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    22050 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    16000 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    12000 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*    11025 */
    {0x00000000, 0x00000000, 0x00000000, 0x00000000},  /*     8000 */

};

/* Compensation Filter BPF coeffs       */
const unsigned int BES_COMPENSATION_BPF[4][6][3] =
{
    /* filter 0 */
    {   {0x00000000, 0x00000000, 0x00000000},  /*    48000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    44100 */
        {0x00000000, 0x00000000, 0x00000000},  /*    32000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    24000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    22050 */
        {0x00000000, 0x00000000, 0x00000000}
    },  /*    16000 */

    /* filter 1 */
    {   {0x00000000, 0x00000000, 0x00000000},  /*    48000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    44100 */
        {0x00000000, 0x00000000, 0x00000000},  /*    32000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    24000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    22050 */
        {0x00000000, 0x00000000, 0x00000000}
    },  /*    16000 */

    /* filter 2 */
    {   {0x00000000, 0x00000000, 0x00000000},  /*    48000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    44100 */
        {0x00000000, 0x00000000, 0x00000000},  /*    32000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    24000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    22050 */
        {0x00000000, 0x00000000, 0x00000000}
    },  /*    16000 */

    /* filter 3 */
    {   {0x00000000, 0x00000000, 0x00000000},  /*    48000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    44100 */
        {0x00000000, 0x00000000, 0x00000000},  /*    32000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    24000 */
        {0x00000000, 0x00000000, 0x00000000},  /*    22050 */
        {0x00000000, 0x00000000, 0x00000000}
    },  /*    16000 */

};
#elif 0
//e1k
unsigned int BES_COMPENSATION_HSF[9][4] =
{
    {0x7B86516, 0xF09EC593, 0x7A8E94A, 0x7AEAC4D6},  /*    48000 */
    {0x7B22675, 0xF0AC95DF, 0x7A15B39, 0x7A76C53F},  /*    44100 */
    {0x79548FA, 0xF0EC5EE1, 0x77E8427, 0x7858C71D},  /*    32000 */
    {0x772A9EB, 0xF138BB73, 0x754E757, 0x75C3C94C},  /*    24000 */
    {0x766896F, 0xF1537168, 0x7465F65, 0x74D8CA0D},  /*    22050 */
    {0x72EF314, 0xF1CDA420, 0x7040EA8, 0x7092CD67},  /*    16000 */
    {0x6ED5A7A, 0xF25D584A, 0x6B66896, 0x6B5ED12B},  /*    12000 */
    {0x6D6AB79, 0xF28EEA5C, 0x69BB528, 0x6988D26C},  /*    11025 */
    {0x670BF8B, 0xF36CE017, 0x624AD36, 0x610AD7C5},  /*     8000 */
};


/* Compensation Filter BPF coeffs       */
unsigned int BES_COMPENSATION_BPF[4][6][3] =
{
    /* filter 0 */
    {   {0x40828447, 0x3D067BB8, 0xC2770000},  /*    48000 */
        {0x408D84D2, 0x3CC47B2D, 0xC2AD0000},  /*    44100 */
        {0x40C187B1, 0x3B94784E, 0xC3A90000},  /*    32000 */
        {0x40FF8BE7, 0x3A297418, 0xC4D60000},  /*    24000 */
        {0x41158D93, 0x39AA726C, 0xC53F0000},  /*    22050 */
        {0x4178968A, 0x37666975, 0xC7200000}
    },  /*    16000 */

    /* filter 1  */
    {   {0x424D8BF7, 0x35D27408, 0xC7DF0000},  /*    48000 */
        {0x427E8D57, 0x34FB72A8, 0xC8860000},  /*    44100 */
        {0x435B9458, 0x312A6BA7, 0xCB790000},  /*    32000 */
        {0x445A9E0B, 0x2CC161F4, 0xCEE40000},  /*    24000 */
        {0x44B2A1BF, 0x2B3E5E40, 0xD00E0000},  /*    22050 */
        {0x4635B481, 0x248D4B7E, 0xD53C0000}
    },  /*    16000 */

    /* filter 2  */
    {   {0x424D9104, 0x35D26EFB, 0xC7DF0000},  /*    48000 */
        {0x427E9346, 0x34FB6CB9, 0xC8860000},  /*    44100 */
        {0x435B9F1D, 0x312A60E2, 0xCB790000},  /*    32000 */
        {0x445AB005, 0x2CC14FFA, 0xCEE40000},  /*    24000 */
        {0x44B2B687, 0x2B3E4978, 0xD00E0000},  /*    22050 */
        {0x4635D724, 0x248D28DB, 0xD53C0000}
    },  /*    16000 */

    /* filter 3  */
    {   {0x435BA38E, 0x312A5C71, 0xCB790000},  /*    48000 */
        {0x43A0A883, 0x2FF9577C, 0xCC660000},  /*    44100 */
        {0x44D5C212, 0x2AA13DED, 0xD0880000},  /*    32000 */
        {0x4635E45E, 0x248D1BA1, 0xD53C0000},  /*    24000 */
        {0x46ACF0B1, 0x227F0F4E, 0xD6D30000},  /*    22050 */
        {0x48B62595, 0x197FDA6A, 0xDDCA0000}
    },  /*    16000 */

};


#endif

AudioCompensationFilter::AudioCompensationFilter(AudioCompFltType_t eFLTtype, size_t dStreamOutBufferSize)
{
    mpBloudHandle = NULL;
    mpWorkingBuf = NULL;
    mpACFInputBuf = NULL;
    mpTempBuf = NULL;
    mintrSize = mtmpSize = 0;
    eFilterType = eFLTtype;
    mInited = mActive = false;
    mStreamOutBufferSizeByte = dStreamOutBufferSize;
    memset((void *)(&mInitParam), 0, sizeof(BLOUD_InitParam));
    ALOGV("AudioYusuBeloudness::AudioYusuBesloudness()");
    return;
}


AudioCompensationFilter::~AudioCompensationFilter()
{
    if (mpWorkingBuf != NULL)
    {
        delete [] mpWorkingBuf;
        mpWorkingBuf = NULL;
        ALOGV("mpWorkingBuf delete");
    }

    if (mpTempBuf != NULL)
    {
        delete [] mpTempBuf;
        mpTempBuf = NULL;
        ALOGV("mpTempBuf delete");
    }

    if (mpACFInputBuf != NULL)
    {
        delete [] mpACFInputBuf;
        mpACFInputBuf = NULL;
        ALOGV("mpACFInputBuf delete");
    }


    if (mpBloudHandle != NULL)
    {
        delete mpBloudHandle;
        mpBloudHandle = NULL;
        ALOGV("mpBloudHandle Close");
    }
    return;
}
//mpBloudHandle->Open can be only called during init
//no need to called before every process called or after mInitParam modified
bool AudioCompensationFilter::Start(void)
{
    Mutex::Autolock _l(mFLTLock);
    ALOGD("AudioCompensationFilter::Start() ");
    if (false == mInited)
    {
        return false;
    }

    if (true == mActive)
    {
        return true;
    }

    memset(mpWorkingBuf, 0, mintrSize);

    if (mpTempBuf != NULL)
    {
        memset(mpTempBuf, 0, mtmpSize);
    }

    if (mpBloudHandle != NULL)
    {
        ALOGV("mpBloudHandle=0x%x, mpWorkingBuf=0x%x, sizeof(mInitParam)=%d, &mInitParam=0x%x, mpBloudHandle->bEnableOLA=%d", mpBloudHandle, mpWorkingBuf, sizeof(mInitParam), &mInitParam, mpBloudHandle->bEnableOLA);
#ifndef ACF_Test_OnlyMemcpy
        mpBloudHandle->Open(mpBloudHandle, mpWorkingBuf, sizeof(mInitParam), &mInitParam, mpBloudHandle->bEnableOLA);
#endif
        mActive = true;
    }
    return true;
}
//mpBloudHandle->Open can be only called during init
//no need to called before every process called or after mInitParam modified
bool AudioCompensationFilter::Stop(void)
{
    Mutex::Autolock _l(mFLTLock);
    ALOGD("AudioCompensationFilter::Stop()");
    if (false == mInited)
    {
        return false;
    }

    if (false == mActive)
    {
        return true;
    }

    mpACFInput_Wleft = mpACFInputBuf;
    mpACFInput_W = mpACFInputBuf;
    mACFInput_Count = 0;
    memset(mpACFInputBuf, 0, mStreamOutBufferSizeByte);//reset 12K byte buffer


    if (mpBloudHandle != NULL)
    {
#ifndef ACF_Test_OnlyMemcpy
        mpBloudHandle->Close(mpBloudHandle, mpBloudHandle->bEnableOLA);
#endif
        mActive = false;
    }
    return true;
}


bool AudioCompensationFilter::Init(void)
{
    Mutex::Autolock _l(mFLTLock);
    ALOGD("Init AudioCompensationFilter");

    if (true == mInited)
    {
        return true;
    }

    if (mpBloudHandle == NULL) //allocate for handle
    {
        mpBloudHandle = (BS_Handle *)new char[sizeof(BS_Handle)];
        BLOUD_SetHandle(mpBloudHandle);  //set this hanedle
        ALOGV("allocate mpBloudHandle = %p,", mpBloudHandle);
        mpBloudHandle->GetBufferSize(mpBloudHandle, &mintrSize, &mtmpSize); // get buffersize
        ALOGD("AudioCompensationFilter mintrSize [%d] mtmpSize [%d]\n", mintrSize, mtmpSize);
    }

    if (mintrSize)
    {
        if (mpWorkingBuf == NULL)
        {
            mpWorkingBuf = new char[mintrSize]; // allocate buffer
            ALOGV("allocate mpWorkingBuf size = %d mpWorkingBuf = %p", mintrSize, mpWorkingBuf);
        }
    }

    if (mtmpSize)
    {
        if (mpTempBuf == NULL)
        {
            mpTempBuf = new char[mtmpSize]; // allocate buffer
            ALOGV("allocate mpTempBuf size = %d mpTempBuf = %p", mtmpSize, mpTempBuf);
        }
    }

    if (mpACFInputBuf == NULL)
    {
        mpACFInputBuf = new short[mStreamOutBufferSizeByte >> 1]; // allocate 12K byte buffer
        ALOGV("allocate mpACFInputBuf size = %d mpACFInputBuf = %p", mStreamOutBufferSizeByte, mpACFInputBuf);
        mpACFInput_Wleft = mpACFInputBuf;
        mpACFInput_W = mpACFInputBuf;
        mACFInput_Count = 0;

    }

    //enable overlap
    mpBloudHandle->bEnableOLA = 1;
    ALOGV("AudioCompensationFilter::Init(), mpBloudHandle->bEnableOLA=%d", mpBloudHandle->bEnableOLA);
    if (mInitParam.pCustom_Param != NULL)
    {
        free(mInitParam.pCustom_Param);
        mInitParam.pCustom_Param = NULL;
    }

    if (mInitParam.pFilter_Coef != NULL)
    {
        free(mInitParam.pFilter_Coef);
        mInitParam.pFilter_Coef = NULL;
    }
    memset((void *)(&mInitParam), 0, sizeof(BLOUD_InitParam));
    mInitParam.pCustom_Param = (BLOUD_CustomParam *)malloc(sizeof(BLOUD_CustomParam));
    mInitParam.pFilter_Coef = (BLOUD_FilterCoef *)malloc(sizeof(BLOUD_FilterCoef));
    mInited = true;

    return true;
}

void AudioCompensationFilter::Deinit(void)
{
    Mutex::Autolock _l(mFLTLock);

    if (false == mInited)
    {
        return;
    }

    if (mpWorkingBuf != NULL)
    {
        delete [] mpWorkingBuf;
        mpWorkingBuf = NULL;
        ALOGV("mpWorkingBuf delete");
    }

    if (mpTempBuf != NULL)
    {
        delete [] mpTempBuf;
        mpTempBuf = NULL;
        ALOGV("mpTempBuf delete");
    }

    if (mpACFInputBuf != NULL)
    {
        delete [] mpACFInputBuf;
        mpACFInputBuf = NULL;
        ALOGV("mpACFInputBuf delete");
    }
    mACFInput_Count = 0;

    if (mpBloudHandle != NULL)
    {
        delete mpBloudHandle;
        mpBloudHandle = NULL;
        ALOGV("mpBloudHandle Close");
    }
    if (mInitParam.pCustom_Param != NULL)
    {
        free(mInitParam.pCustom_Param);
        mInitParam.pCustom_Param = NULL;
    }

    if (mInitParam.pFilter_Coef != NULL)
    {
        free(mInitParam.pFilter_Coef);
        mInitParam.pFilter_Coef = NULL;
    }

    memset((void *)(&mInitParam), 0, sizeof(BLOUD_InitParam));

    ALOGV("AudioYusuBeloudness::Deinit()");
}

void AudioCompensationFilter::LoadACFParameter()
{
    int i, j, k;
    Mutex::Autolock _l(mFLTLock);
    if (false == mInited)
    {
        return;
    }
    //directly use default param
    //getDefaultAudioCompFltParam(&audioParam);

    //get params from nvram, if they doesn't exist in nvram
    //nvram dirver will copy from default param to nvram
    if (eFilterType == AUDIO_COMP_FLT_AUDENH)
    {
        getDefaultAudioCompFltParam(eFilterType, &audioParam);
    }
    else
    {
        GetAudioCompFltCustParamFromNV(eFilterType, &audioParam);
    }
    ALOGD("AudioCompensationFilter::LoadACFParameter!! ");
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)  
    mInitParam.pCustom_Param->WS_Gain_Max = audioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pCustom_Param->WS_Gain_Min = audioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pCustom_Param->Filter_First = audioParam.bes_loudness_Filter_First;
    mInitParam.pCustom_Param->Att_Time = audioParam.bes_loudness_Att_Time;
    mInitParam.pCustom_Param->Rel_Time = audioParam.bes_loudness_Rel_Time;

    memcpy((void *)mInitParam.pFilter_Coef->HPF_COEF, (void *)audioParam.bes_loudness_hsf_coeff, 90 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pFilter_Coef->BPF_COEF, (void *)audioParam.bes_loudness_bpf_coeff, 144 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pFilter_Coef->LPF_COEF, (void *)audioParam.bes_loudness_lpf_coeff, 18 * sizeof(unsigned int));
    //        memcpy((void*)mInitParam.pCustom_Param->DRC_Forget_Table, (void*)audioParam.bes_loudness_DRC_Forget_Table, 18*sizeof(unsigned int));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_In, (void *)audioParam.bes_loudness_Gain_Map_In, 5 * sizeof(char));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_Out, (void *)audioParam.bes_loudness_Gain_Map_Out, 5 * sizeof(char));
#else
    mInitParam.pCustom_Param->WS_Gain_Max = audioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pCustom_Param->WS_Gain_Min = audioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pCustom_Param->Filter_First = audioParam.bes_loudness_Filter_First;
    mInitParam.pCustom_Param->Att_Time = 0;
    mInitParam.pCustom_Param->Rel_Time = 0;
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_In, (void *)audioParam.bes_loudness_Gain_Map_In, 5 * sizeof(char));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_Out, (void *)audioParam.bes_loudness_Gain_Map_Out, 5 * sizeof(char));
    memset((void *)mInitParam.pFilter_Coef->LPF_COEF, 0x00, 18 * sizeof(unsigned int));
#if 0
    for (i = 0; i < 2; i++)
        for (j = 0; j < 9; j++)
            for (k = 0; k < 5; k++)
            {
                mInitParam.pFilter_Coef->HPF_COEF[i][j][k] = (i == 0 && k < 4) ? audioParam.bes_loudness_hsf_coeff[j][k] : 0x00;
            }

    for (i = 0; i < 8; i++)
        for (j = 0; j < 6; j++)
            for (k = 0; k < 3; k++)
            {
                mInitParam.pFilter_Coef->BPF_COEF[i][j][k] = (i < 4) ? audioParam.bes_loudness_bpf_coeff[i][j][k] : 0x00;
            }
#else
    memset((void *)mInitParam.pFilter_Coef->HPF_COEF, 0x00, 90 * sizeof(unsigned int));
    memset((void *)mInitParam.pFilter_Coef->BPF_COEF, 0x00, 144 * sizeof(unsigned int));

    for (i = 0; i < 9; i++)
    {
        memcpy(mInitParam.pFilter_Coef->HPF_COEF[0][i], audioParam.bes_loudness_hsf_coeff[i], 4 * sizeof(unsigned int));
    }

    memcpy(mInitParam.pFilter_Coef->BPF_COEF, audioParam.bes_loudness_bpf_coeff, 4 * 6 * 3 * sizeof(unsigned int));

#endif

#endif

    //ALOGD("&(audioParam.bes_loudness_hsf_coeff[0][0][0])=%p, &(audioParam.bes_loudness_bpf_coeff[0][0][0])=%p", &(audioParam.bes_loudness_hsf_coeff[0][0][0]),&(audioParam.bes_loudness_bpf_coeff[0][0][0]));
    ALOGD("mInitParam.pCustom_Param->HSF_Coeff =%p, mInitParam.pCustom_Param->BPF_Coeff =%p", &mInitParam.pFilter_Coef->HPF_COEF[0][0][0],  &mInitParam.pFilter_Coef->BPF_COEF[0][0][0]);
    ALOGD("HSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][0], &mInitParam.pFilter_Coef->HPF_COEF[0][0][0]);
    ALOGD("HSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][1], &mInitParam.pFilter_Coef->HPF_COEF[0][0][1]);
    ALOGD("BPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][0], &mInitParam.pFilter_Coef->BPF_COEF[0][0][0]);
    ALOGD("BPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][1], &mInitParam.pFilter_Coef->BPF_COEF[0][0][1]);
    //      ALOGD("DRC_Forget_Table[0][0]=0x%x, DRC_Forget_Table [0][1]=0x%x,", mInitParam.pCustom_Param->DRC_Forget_Table[0][0], mInitParam.pCustom_Param->DRC_Forget_Table[0][1]);
    ALOGD("WS_Gain_Max=0x%x, WS_Gain_Min=0x%x, Filter_First=0x%x", mInitParam.pCustom_Param->WS_Gain_Max, mInitParam.pCustom_Param->WS_Gain_Min , mInitParam.pCustom_Param->Filter_First);
    ALOGD("Att_Time=0x%x, Rel_Time=0x%x", mInitParam.pCustom_Param->Att_Time, mInitParam.pCustom_Param->Rel_Time);
    ALOGD("Gain_Map_In [0]=0x%x, Gain_Map_In [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_In[0], mInitParam.pCustom_Param->Gain_Map_In[1]);
    ALOGD("Gain_Map_Out [0]=0x%x, Gain_Map_Out [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_Out[0], mInitParam.pCustom_Param->Gain_Map_Out[1]);
#if 0
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (mInitParam.pCustom_Param->HSF_Coeff [i][j] != 0)
            {
                ALOGV("SET!!  HSF_Coeff [%d][%d]=0x%x, addr = %p", i, j, mInitParam.pCustom_Param->HSF_Coeff[i][j], &mInitParam.pCustom_Param->HSF_Coeff[i][j]);
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            for (k = 0; k < 3; k++)
            {
                if (mInitParam.pCustom_Param->BPF_Coeff[i][j][k] != 0)
                {
                    ALOGV("SET!! BPF_Coeff [%d][%d][%d]=0x%x, addr = %p", i, j, k, mInitParam.pCustom_Param->BPF_Coeff[i][j][k], &mInitParam.pCustom_Param->BPF_Coeff[i][j][k]);
                }
            }
        }
    }
#endif
#if 0

    mInitParam.HPF_Coeff = &BES_COMPENSATION_HSF[0][0];
    mInitParam.BPF_Coeff = &BES_COMPENSATION_BPF[0][0][0];
    ALOGV("&BES_COMPENSATION_HSF[0][0]=0x%x, &BES_COMPENSATION_BPF[0][0][0]=0x%x", &BES_COMPENSATION_HSF[0][0], &BES_COMPENSATION_BPF[0][0][0]);
    ALOGV("mInitParam.HPF_Coeff=0x%x, mInitParam.BPF_Coeff=0x%x", mInitParam.HPF_Coeff, mInitParam.BPF_Coeff);
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 4; j++)
        {
            //if(BES_COMPENSATION_HSF[i][j]!=0)
            ALOGV("ERROR!! BES_COMPENSATION_HSF[%d][%d]=0x%x addr = 0x%x", i , j , BES_COMPENSATION_HSF[i][j], &BES_COMPENSATION_HSF[i][j]);
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            for (k = 0; k < 3; k++)
            {
                //if(BES_COMPENSATION_BPF[i][j][k]!=0)
                ALOGV("ERROR!! BES_COMPENSATION_BPF [%d][%d][%d]=0x%x, addr = 0x%x", i, j, k, BES_COMPENSATION_BPF[i][j][k], &BES_COMPENSATION_BPF[i][j][k]);
            }
        }
    }

    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 4; j++)
        {
            //if(*(mInitParam.HPF_Coeff+(i*4+j)*sizeof(unsigned int))!=0)
            ALOGV("ERROR!! HPF_Coeff [%d][%d]=0x%x, addr = 0x%x", i, j, *(mInitParam.HPF_Coeff + (i * 4 + j)), mInitParam.HPF_Coeff + (i * 4 + j));
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            for (k = 0; k < 3; k++)
            {
                //if(*(mInitParam.BPF_Coeff+(i*18+j*3+k)*sizeof(unsigned int))!=0)
                ALOGV("ERROR!! BPF_Coeff [%d][%d][%d]=0x%x, addr = 0x%x", i, j, k, *(mInitParam.BPF_Coeff + (i * 18 + j * 3 + k)), mInitParam.BPF_Coeff + (i * 18 + j * 3 + k));
            }
        }
    }

#endif
}

void AudioCompensationFilter::SetACFPreviewParameter(AUDIO_ACF_CUSTOM_PARAM_STRUCT *PreviewParam)
{
    int i, j, k;
    ALOGD("AudioCompensationFilter::SetPreviewParameter!! ");
    Mutex::Autolock _l(mFLTLock);
    if (false == mInited)
    {
        return;
    }

    if (eFilterType == AUDIO_COMP_FLT_AUDENH)
    {
        return;
    }

    memcpy((void *)&audioParam, (void *)PreviewParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)  
    mInitParam.pCustom_Param->WS_Gain_Max = audioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pCustom_Param->WS_Gain_Min = audioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pCustom_Param->Filter_First = audioParam.bes_loudness_Filter_First;
    mInitParam.pCustom_Param->Att_Time = audioParam.bes_loudness_Att_Time;
    mInitParam.pCustom_Param->Rel_Time = audioParam.bes_loudness_Rel_Time;
    memcpy((void *)mInitParam.pFilter_Coef->HPF_COEF, (void *)audioParam.bes_loudness_hsf_coeff, 90 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pFilter_Coef->BPF_COEF, (void *)audioParam.bes_loudness_bpf_coeff, 144 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pFilter_Coef->LPF_COEF, (void *)audioParam.bes_loudness_lpf_coeff, 18 * sizeof(unsigned int));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_In, (void *)audioParam.bes_loudness_Gain_Map_In, 5 * sizeof(char));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_Out, (void *)audioParam.bes_loudness_Gain_Map_Out, 5 * sizeof(char));
    ALOGD("========== Update ACF/HCF Data Begin==========");
    ALOGD("#ifndef AUDIO_ACF_DEFAULT_H");
    ALOGD("#define AUDIO_ACF_DEFAULT_H");
    ALOGD("#define BES_LOUDNESS_HSF_COEFF \\");
    for (i = 0; i < 2; i++)
        for (j = 0; j < 9; j++)
        {
            ALOGD("0x%x,   0x%x,   0x%x,   0x%x,   0x%x,     \\", mInitParam.pFilter_Coef->HPF_COEF[i][j][0], mInitParam.pFilter_Coef->HPF_COEF[i][j][1], mInitParam.pFilter_Coef->HPF_COEF[i][j][2], mInitParam.pFilter_Coef->HPF_COEF[i][j][3], mInitParam.pFilter_Coef->HPF_COEF[i][j][4]);
            if (j == 8)
            {
                if (i == 0)
                {
                    ALOGD("    \\");
                }
                else
                {
                    ALOGD("\n");
                }
            }
        }
    ALOGD("#define BES_LOUDNESS_BPF_COEFF \\");
    for (i = 0; i < 8; i++)
        for (j = 0; j < 6; j++)
        {
            ALOGD("0x%x,   0x%x,   0x%x,     \\", mInitParam.pFilter_Coef->BPF_COEF[i][j][0], mInitParam.pFilter_Coef->BPF_COEF[i][j][1], mInitParam.pFilter_Coef->BPF_COEF[i][j][2]);
            if (j == 5)
            {
                if (i == 7)
                {
                    ALOGD("\n");
                }
                else
                {
                    ALOGD("    \\");
                }

            }
        }
    ALOGD("#define BES_LOUDNESS_LPF_COEFF \\");
    for (i = 0; i < 6; i++)
    {
        ALOGD("0x%x,   0x%x,   0x%x,     \\", mInitParam.pFilter_Coef->LPF_COEF[i][0], mInitParam.pFilter_Coef->LPF_COEF[i][1], mInitParam.pFilter_Coef->LPF_COEF[i][2]);
    }
    ALOGD("#define BES_LOUDNESS_WS_GAIN_MAX  0x%x", mInitParam.pCustom_Param->WS_Gain_Max);
    ALOGD("#define BES_LOUDNESS_WS_GAIN_MIN  0x%x", mInitParam.pCustom_Param->WS_Gain_Max);
    ALOGD("#define BES_LOUDNESS_FILTER_FIRST  0x%x", mInitParam.pCustom_Param->Filter_First);
    ALOGD("#define BES_LOUDNESS_ATT_TIME  0x%x", mInitParam.pCustom_Param->Att_Time);
    ALOGD("#define BES_LOUDNESS_REL_TIME  0x%x", mInitParam.pCustom_Param->Rel_Time);
    ALOGD("#define BES_LOUDNESS_GAIN_MAP_IN \\");
    ALOGD("0x%x,  0x%x,  0x%x,  0x%x,  0x%x", (unsigned int)mInitParam.pCustom_Param->Gain_Map_In[0], (unsigned int)mInitParam.pCustom_Param->Gain_Map_In[1], (unsigned int)mInitParam.pCustom_Param->Gain_Map_In[2], (unsigned int)mInitParam.pCustom_Param->Gain_Map_In[3], (unsigned int)mInitParam.pCustom_Param->Gain_Map_In[4]);
    ALOGD("#define BES_LOUDNESS_GAIN_MAP_OUT \\");
    ALOGD("0x%x,  0x%x,  0x%x,  0x%x,  0x%x", (unsigned int)mInitParam.pCustom_Param->Gain_Map_Out[0], (unsigned int)mInitParam.pCustom_Param->Gain_Map_Out[1], (unsigned int)mInitParam.pCustom_Param->Gain_Map_Out[2], (unsigned int)mInitParam.pCustom_Param->Gain_Map_Out[3], (unsigned int)mInitParam.pCustom_Param->Gain_Map_Out[4]);
    ALOGD("#endif");
    ALOGD("\n\n");
    unsigned int dTempValue;
    dTempValue = 0;
    for (i = 0; i < 2; i++)
        for (j = 0; j < 9; j++)
            for (k = 0; k < 5; k++)
            {
                dTempValue = dTempValue + mInitParam.pFilter_Coef->HPF_COEF[i][j][k];
            }
    ALOGD("HPF_COEF[2][9][5] CheckSum(Summation all elements): [0x%x]", dTempValue);
    dTempValue = 0;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 6; j++)
            for (k = 0; k < 3; k++)
            {
                dTempValue = dTempValue + mInitParam.pFilter_Coef->BPF_COEF[i][j][k];
            }
    ALOGD("BPF_COEF[8][6][3] CheckSum(Summation all elements): [0x%x]", dTempValue);
    dTempValue = 0;
    for (i = 0; i < 6; i++)
        for (j = 0; j < 3; j++)
        {
            dTempValue = dTempValue + mInitParam.pFilter_Coef->LPF_COEF[i][j];
        }
    ALOGD("LPF_COEF[6][3] CheckSum(Summation all elements): [0x%x]", dTempValue);
    ALOGD("========== Update ACF/HCF Data End ==========");
#else
    mInitParam.pCustom_Param->WS_Gain_Max = audioParam.bes_loudness_WS_Gain_Max;
    mInitParam.pCustom_Param->WS_Gain_Min = audioParam.bes_loudness_WS_Gain_Min;
    mInitParam.pCustom_Param->Filter_First = audioParam.bes_loudness_Filter_First;
    mInitParam.pCustom_Param->Att_Time = 0;
    mInitParam.pCustom_Param->Rel_Time = 0;
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_In, (void *)audioParam.bes_loudness_Gain_Map_In, 5 * sizeof(char));
    memcpy((void *)mInitParam.pCustom_Param->Gain_Map_Out, (void *)audioParam.bes_loudness_Gain_Map_Out, 5 * sizeof(char));
    memset((void *)mInitParam.pFilter_Coef->LPF_COEF, 0x00, 18 * sizeof(unsigned int));
#if 0
    for (i = 0; i < 2; i++)
        for (j = 0; j < 9; j++)
            for (k = 0; k < 5; k++)
            {
                mInitParam.pFilter_Coef->HPF_COEF[i][j][k] = (i == 0 && k < 4) ? audioParam.bes_loudness_hsf_coeff[j][k] : 0x00;
            }

    for (i = 0; i < 8; i++)
        for (j = 0; j < 6; j++)
            for (k = 0; k < 3; k++)
            {
                mInitParam.pFilter_Coef->BPF_COEF[i][j][k] = (i < 4) ? audioParam.bes_loudness_bpf_coeff[i][j][k] : 0x00;
            }
#else
    memset((void *)mInitParam.pFilter_Coef->HPF_COEF, 0x00, 90 * sizeof(unsigned int));
    memset((void *)mInitParam.pFilter_Coef->BPF_COEF, 0x00, 144 * sizeof(unsigned int));

    for (i = 0; i < 9; i++)
    {
        memcpy(mInitParam.pFilter_Coef->HPF_COEF[0][i], audioParam.bes_loudness_hsf_coeff[i], 4 * sizeof(unsigned int));
    }

    memcpy(mInitParam.pFilter_Coef->BPF_COEF, audioParam.bes_loudness_bpf_coeff, 4 * 6 * 3 * sizeof(unsigned int));

#endif
#endif
    //    ALOGD("HSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][0], &mInitParam.pFilter_Coef->HPF_COEF[0][0][0]);
    //    ALOGD("HSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][1], &mInitParam.pFilter_Coef->HPF_COEF[0][0][1]);
    //    ALOGD("BPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][0], &mInitParam.pFilter_Coef->BPF_COEF[0][0][0]);
    //    ALOGD("BPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][1], &mInitParam.pFilter_Coef->BPF_COEF[0][0][1]);
    //    ALOGD("DRC_Forget_Table[0][0]=0x%x, DRC_Forget_Table [0][1]=0x%x,", mInitParam.pCustom_Param->DRC_Forget_Table[0][0], mInitParam.pCustom_Param->DRC_Forget_Table[0][1]);
    //    ALOGD("WS_Gain_Max=0x%x, WS_Gain_Min=0x%x, Filter_First=0x%x", mInitParam.pCustom_Param->WS_Gain_Max, mInitParam.pCustom_Param->WS_Gain_Min , mInitParam.pCustom_Param->Filter_First);
    //    ALOGD("Att_Time=0x%x, Rel_Time=0x%x", mInitParam.pCustom_Param->Att_Time, mInitParam.pCustom_Param->Rel_Time);
    //    ALOGD("Gain_Map_In [0]=0x%x, Gain_Map_In [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_In[0], mInitParam.pCustom_Param->Gain_Map_In[1]);
    //    ALOGD("Gain_Map_Out [0]=0x%x, Gain_Map_Out [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_Out[0], mInitParam.pCustom_Param->Gain_Map_Out[1]);
#if 0
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (mInitParam.pCustom_Param->HSF_Coeff [i][j] != 0)
            {
                ALOGV("Preview !! HSF_Coeff [%d][%d]=0x%x, addr = %p", i, j, mInitParam.pCustom_Param->HSF_Coeff[i][j], &mInitParam.pCustom_Param->HSF_Coeff[i][j]);
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 6; j++)
        {
            for (k = 0; k < 3; k++)
            {
                if (mInitParam.pCustom_Param->BPF_Coeff[i][j][k] != 0)
                {
                    ALOGV("Preview !! BPF_Coeff [%d][%d][%d]=0x%x, addr = %p", i, j, k, mInitParam.pCustom_Param->BPF_Coeff[i][j][k], &mInitParam.pCustom_Param->BPF_Coeff[i][j][k]);
                }
            }
        }
    }

#endif
}

/*
Besloudness Mode Description
 0 : BesLoudness Basic Mode
 1 : BesLoudness Enhanced Mode
 2 : BesLoudness Aggressive Mode
 3 : BesLoudness Lite Mode (loudness enhance without filtering)
 4 : Compensation Mode
 5 : Compensation Mode + BesLoudness Lite Mode
*/
void AudioCompensationFilter::SetWorkMode(unsigned int chNum, unsigned int smpRate, AudioComFltMode_t workMode)
{
    if (chNum > 0 && chNum < 3)  // chnum should be 1 or 2
    {
        mInitParam.Channel_Num = chNum;
    }
    else
    {
        return ;
    }
    Mutex::Autolock _l(mFLTLock);
    if (false == mInited)
    {
        return;
    }
    mInitParam.Sample_Rate = smpRate;
    mWorkMode = workMode;
    switch (mWorkMode)
    {
        case AUDIO_CMP_FLT_LOUDNESS_BASIC:     // basic Loudness mode
            mInitParam.Filter_Mode   = FILT_MODE_LOUD_FLT;
            mInitParam.Loudness_Mode = LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_ENHANCED:     // enhancement(1) Loudness mode
            mInitParam.Filter_Mode   = FILT_MODE_LOUD_FLT;
            mInitParam.Loudness_Mode = LOUD_MODE_ENHANCED;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_AGGRESSIVE:     // enhancement(2) Loudness mode
            mInitParam.Filter_Mode   = FILT_MODE_LOUD_FLT;
            mInitParam.Loudness_Mode = LOUD_MODE_AGGRESSIVE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_LITE:     // Only DRC, no filtering
            mInitParam.Filter_Mode   = FILT_MODE_NONE;
            mInitParam.Loudness_Mode = LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP:     // Audio Compensation Filter mode (No DRC)
            mInitParam.Filter_Mode   = FILT_MODE_COMP_FLT;
            mInitParam.Loudness_Mode = LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC:     // Audio Compensation Filter mode + DRC
            mInitParam.Filter_Mode   = FILT_MODE_COMP_FLT;
            mInitParam.Loudness_Mode = LOUD_MODE_BASIC;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_HEADPHONE:     //HCF
            mInitParam.Filter_Mode   = FILT_MODE_COMP_HDP;
            mInitParam.Loudness_Mode = LOUD_MODE_NONE;
            break;
        case AUDIO_CMP_FLT_LOUDNESS_COMP_AUDENH:
            mInitParam.Filter_Mode   = FILT_MODE_AUD_ENH;
            mInitParam.Loudness_Mode = LOUD_MODE_NONE;
            break;
        default:
            ASSERT(0);
            break;
    }
    ALOGD("AudioCompensationFilter mWorkMode=%d, Channel_Num=%d, Sample_Rate=%d", mWorkMode, mInitParam.Channel_Num, mInitParam.Sample_Rate);
}
//reset working buffer and left input buffer
void AudioCompensationFilter::ResetBuffer(void)
{
    Mutex::Autolock _l(mFLTLock);
    if (false == mInited)
    {
        return;
    }

    if (true == mActive)
    {
        return;
    }

    ALOGV("AudioCompensationFilter::ResetBuffer() mintrSize=%d, mACFInput_Count=%d", mintrSize, mACFInput_Count);
    memset(mpWorkingBuf, 0, mintrSize);
    if (mpTempBuf != NULL)
    {
        memset(mpTempBuf, 0, mtmpSize);
    }
    //clear left input data to 0
    if (mpACFInputBuf != NULL)
    {
        memset((void *)(mpACFInputBuf), 0 , mStreamOutBufferSizeByte);
    }
}

void AudioCompensationFilter::Process(const short *pInputBuffer, int *InputSampleCount, short *pOutputBuffer, int *OutputSampleCount)
{
    int i, j, k, InputCountLeft, a, b;
    int  TotalConsumedSample = 0, TotalOuputSample = 0, ConsumedSampleCount = 0;
    Mutex::Autolock _l(mFLTLock);
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("AudioYusuBesloudness::Process mpBloudHandle = %p, pInputBuffer = %p InputSampleCount = %d, pOutputBuffer = %p, OutputSampleCount = %d"
          , mpBloudHandle, pInputBuffer, *InputSampleCount, pOutputBuffer, *OutputSampleCount);
    ALOGV("AudioCompensationFilter Channel_Num=%d, Sample_Rate=%d", mInitParam.Channel_Num, mInitParam.Sample_Rate);
    ALOGV("HSF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][0], &mInitParam.pFilter_Coef->HPF_COEF[0][0][0]);
    ALOGV("HSF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->HPF_COEF[0][0][1], &mInitParam.pFilter_Coef->HPF_COEF[0][0][1]);
    ALOGV("BPF_Coeff [0][0][0]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][0], &mInitParam.pFilter_Coef->BPF_COEF[0][0][0]);
    ALOGV("BPF_Coeff [0][0][1]=0x%x, addr = %p,", mInitParam.pFilter_Coef->BPF_COEF[0][0][1], &mInitParam.pFilter_Coef->BPF_COEF[0][0][1]);
    //     ALOGV("DRC_Forget_Table[0][0]=0x%x, DRC_Forget_Table [0][1]=0x%x,", mInitParam.pCustom_Param->DRC_Forget_Table[0][0], mInitParam.pCustom_Param->DRC_Forget_Table[0][1]);
    ALOGV("WS_Gain_Max=0x%x, WS_Gain_Min=0x%x, Filter_First=0x%x", mInitParam.pCustom_Param->WS_Gain_Max, mInitParam.pCustom_Param->WS_Gain_Min , mInitParam.pCustom_Param->Filter_First);
    ALOGV("Att_Time=0x%x, Rel_Time=0x%x", mInitParam.pCustom_Param->Att_Time, mInitParam.pCustom_Param->Rel_Time);
    ALOGV("Gain_Map_In [0]=0x%x, Gain_Map_In [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_In[0], mInitParam.pCustom_Param->Gain_Map_In[1]);
    ALOGV("Gain_Map_Out [0]=0x%x, Gain_Map_Out [1]=0x%x,", mInitParam.pCustom_Param->Gain_Map_Out[0], mInitParam.pCustom_Param->Gain_Map_Out[1]);
#endif

#if 1
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("b copy whole data mACFInput_Count = %d, *InputSampleCount  = %d ", mACFInput_Count, *InputSampleCount);
#endif

    if (((false == mInited || false == mActive) && (eFilterType == AUDIO_COMP_FLT_AUDIO || eFilterType == AUDIO_COMP_FLT_HEADPHONE))
        || ((eFilterType == AUDIO_COMP_FLT_AUDENH) && (false == mInited)))
    {
        *InputSampleCount = 0;
        *OutputSampleCount = 0;
        return;
    }
    else if ((eFilterType == AUDIO_COMP_FLT_AUDENH) && (0 == mpBloudHandle->bEnabled))
    {
        *InputSampleCount = 0;
        *OutputSampleCount = 0;
        mpACFInput_Wleft = mpACFInputBuf;
        mpACFInput_W = mpACFInputBuf;
        mACFInput_Count = 0;
        return;
    }

    memcpy((void *)(mpACFInput_W), pInputBuffer , (*InputSampleCount) << 1);
    mACFInput_Count +=  *InputSampleCount;
    mpACFInput_W += *InputSampleCount;//update write ptr
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("copy whole data mACFInput_Count = %d, *InputSampleCount  = %d ", mACFInput_Count, *InputSampleCount);
#endif

    a = mACFInput_Count >> 10;
    b = a << 10;
    InputCountLeft = mACFInput_Count - (b); // 512 sample x 2ch
    mACFInput_Count -= InputCountLeft;
    mpACFInput_Wleft = mpACFInputBuf + mACFInput_Count;
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("Before Process InputCountLeft = %d, mACFInput_Count = %d InputSampleCount = %d, mpACFInputBuf = %p, mpACFInput_W = %p, mpACFInput_Wleft= %p, a=%d, b=%d"
          , InputCountLeft, mACFInput_Count, *InputSampleCount, mpACFInputBuf , mpACFInput_W , mpACFInput_Wleft, a, b);
#endif
#ifdef FLT_PROFILING
    struct timeval time0, time1, time2, time3, time4, time5, time6;
#endif

    while (mACFInput_Count >= 1024)
    {
#ifdef FLT_PROFILING
        ALOGD("-1.[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time6.tv_sec, time6.tv_usec);
        gettimeofday(&time0, NULL);
#endif
        int sum_input = 0;
        ConsumedSampleCount = mACFInput_Count;
        //ALOGV("Process mAudioCompFlt InputSampleCount=%d, ConsumedBytes=%d, OutputBytes=%d", mACFInput_Count, TotalConsumedSample, TotalOuputSample);
#ifdef ACF_Test_OnlyMemcpy
        //ALOGD("ACF_Test_OnlyMemcpy!!");
        //test only memcpy
        memcpy(pOutputBuffer + TotalOuputSample, mpACFInputBuf + TotalConsumedSample, 2048);
        ConsumedSampleCount = 1024;
        *OutputSampleCount = 1024;
#else

#ifdef FLT_PROFILING
        gettimeofday(&time1, NULL);
        gettimeofday(&time2, NULL);
#endif
        mpBloudHandle->Process(mpBloudHandle, mpTempBuf, mpACFInputBuf + TotalConsumedSample, &ConsumedSampleCount, pOutputBuffer + TotalOuputSample, OutputSampleCount);
#ifdef FLT_PROFILING
        gettimeofday(&time3, NULL);
#endif
#endif

        if (mWorkMode == AUDIO_CMP_FLT_LOUDNESS_LITE || mWorkMode == AUDIO_CMP_FLT_LOUDNESS_COMP_BASIC)
        {
            for (int i = 0; i < ConsumedSampleCount; i++)
            {
               if(*(mpACFInputBuf + TotalConsumedSample + i))
                {
                    sum_input = 1;
                    break;
                }                
            }
            if (sum_input == 0)
            {
#ifdef ENABLE_LOG_AudioCompensationFilter
                ALOGV("skip DRC result of input 0: OutputSampleCount:%d, mWorkMode=%d", *OutputSampleCount, mWorkMode);
#endif
                memset(pOutputBuffer + TotalOuputSample, 0, (*OutputSampleCount) << 1);
            }
        }
        //ALOGV("OutputSampleCount:%d", *OutputSampleCount);
        if (ConsumedSampleCount == 0)
        {
            ALOGE("afterProcess mAudioCompFlt ConsumedSampleCount=%d, OutputSampleCount=%d", ConsumedSampleCount, *OutputSampleCount);
        }

        TotalConsumedSample += ConsumedSampleCount;
        TotalOuputSample += *OutputSampleCount;
        mACFInput_Count -= ConsumedSampleCount;
#ifdef FLT_PROFILING
        gettimeofday(&time4, NULL);
        ALOGD("0.[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time0.tv_sec, time0.tv_usec);
        ALOGD("1.[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time1.tv_sec, time1.tv_usec);
        ALOGD("2.B4 P[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time2.tv_sec, time2.tv_usec);
        ALOGD("3.AF P[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time3.tv_sec, time3.tv_usec);
        ALOGD("4.[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time4.tv_sec, time4.tv_usec);
        gettimeofday(&time5, NULL);
        ALOGD("5.[%s][%d][%d]:[%d]", __FUNCTION__, __LINE__, time5.tv_sec, time5.tv_usec);
        gettimeofday(&time6, NULL);
#endif
    }
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("after Process TotalConsumedSample = %d, pOutputBuffer = %p, TotalOuputSample = %d"
          ,  TotalConsumedSample, pOutputBuffer, TotalOuputSample);
#endif

    if (TotalConsumedSample != TotalOuputSample)
    {
        ALOGE("ACF TotalConsumedSample(%d) != TotalOuputSample(%d)!!!", TotalConsumedSample, TotalOuputSample);
    }
    *InputSampleCount = TotalConsumedSample;
    *OutputSampleCount = TotalOuputSample;
    mpACFInput_W = mpACFInputBuf;
    mACFInput_Count = InputCountLeft;
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("after Process InputCountLeft = %d, mACFInput_Count = %d InputSampleCount = %d, mpACFInputBuf = %p, mpACFInput_W = %p, mpACFInput_Wleft= %p"
          , InputCountLeft, mACFInput_Count, *InputSampleCount, mpACFInputBuf , mpACFInput_W , mpACFInput_Wleft);
#endif
    if (InputCountLeft != 0)
    {
        //copy left data
        memcpy((void *)(mpACFInput_W), mpACFInput_Wleft , InputCountLeft << 1);
        mpACFInput_W = mpACFInputBuf + InputCountLeft; //update write ptr
    }
    mpACFInput_Wleft = mpACFInputBuf;
#ifdef ENABLE_LOG_AudioCompensationFilter
    ALOGV("after copy left data InputCountLeft = %d, mACFInput_Count = %d InputSampleCount = %d, mpACFInputBuf = %p, mpACFInput_W = %p, mpACFInput_Wleft= %p"
          , InputCountLeft, mACFInput_Count, *InputSampleCount, mpACFInputBuf , mpACFInput_W , mpACFInput_Wleft);
#endif

#else
    //only for audio master mode, not support audio slave mode
    mpBloudHandle->Process(mpBloudHandle, NULL, pInputBuffer, InputSampleCount, pOutputBuffer, OutputSampleCount);
#endif
}

bool AudioCompensationFilter::Resume(void)
{
    Mutex::Autolock _l(mFLTLock);
    ALOGD("AudioCompensationFilter::Resume() ");
    if (false == mInited)
    {
        return false;
    }

    if (false == mActive)//Because FixedHCF will resume when pause/START state!
    {
        return false;
    }

    if (mpBloudHandle != NULL)
    {
        ALOGV("mpBloudHandle=0x%x, mpWorkingBuf=0x%x, sizeof(mInitParam)=%d, &mInitParam=0x%x, mpBloudHandle->bEnableOLA=%d", mpBloudHandle, mpWorkingBuf, sizeof(mInitParam), &mInitParam, mpBloudHandle->bEnableOLA);
#ifndef ACF_Test_OnlyMemcpy
        mpBloudHandle->Open(mpBloudHandle, mpWorkingBuf, sizeof(mInitParam), &mInitParam, mpBloudHandle->bEnableOLA);
#endif
        //mActive = true; Resume must be in the start state.
    }
    return true;
}

bool AudioCompensationFilter::Pause(void)
{
    Mutex::Autolock _l(mFLTLock);
    ALOGD("AudioCompensationFilter::Pause()");
    if (false == mInited)
    {
        return false;
    }

    if (false == mActive)
    {
        return true;
    }

    if (mpBloudHandle != NULL)
    {
#ifndef ACF_Test_OnlyMemcpy
        mpBloudHandle->Close(mpBloudHandle, mpBloudHandle->bEnableOLA);
#endif
        //mActive = false;  Remove, Because FixedHCF will stop when pause state!
    }

    return true;
}

// ---------------------------------------------------------------------------
}

