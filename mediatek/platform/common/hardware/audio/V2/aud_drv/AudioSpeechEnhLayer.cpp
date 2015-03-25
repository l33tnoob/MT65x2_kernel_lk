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

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include <utils/Log.h>
#include <utils/String8.h>

#include "audio_custom_exp.h"
#include <assert.h>
#include <cutils/properties.h>
#include "AudioSpeechEnhLayer.h"

//#ifdef MTK_AP_SPEECH_ENHANCEMENT
#include "AudioCustParam.h"
//#endif

#include "audio_hd_record_48k_custom.h"

#define LOG_TAG "AudioSPELayer"

namespace android
{

Word16 DefaultRecDMNR_cal_data[DMNRCalDataNum] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Word16 DefaultRecCompen_filter[CompenFilterNum] =
{
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
uWord32 DefaultRecEnhancePara[EnhanceParasNum] =
{96, 253, 16388, 32796, 49415, 0, 400, 0, 272, 4325, 99, 0, 0, 0, 0, 8192, 0, 0, 0, 10752, 32769, 0, 0, 0, 0, 0, 0, 0};

Word16 DefaultVoIPDMNR_cal_data[DMNRCalDataNum] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Word16 DefaultVoIPCompen_filter[CompenFilterNum] =
{
    26,    15,   -56,    27,   -17,    39,   -18,   -44,    40,     5, \
    38,   -63,    47,   -79,    52,   -22,    62,   -47,     4,    24, \
    -55,    46,   -28,   123,  -275,   241,  -208,   328,  -244,   -13, \
    98,   176,  -108,  -746,   476,   111,  1661, -2136,   206, -1707, \
    5461, -5885,  2762, -1354, 19656, 19656, -1354,  2762, -5885,  5461, \
    -1707,   206, -2136,  1661,   111,   476,  -746,  -108,   176,    98, \
    -13,  -244,   328,  -208,   241,  -275,   123,   -28,    46,   -55, \
    24,     4,   -47,    62,   -22,    52,   -79,    47,   -63,    38, \
    5,    40,   -44,   -18,    39,   -17,    27,   -56,    15,    26, \
    \
    26,    15,   -56,    27,   -17,    39,   -18,   -44,    40,     5, \
    38,   -63,    47,   -79,    52,   -22,    62,   -47,     4,    24, \
    -55,    46,   -28,   123,  -275,   241,  -208,   328,  -244,   -13, \
    98,   176,  -108,  -746,   476,   111,  1661, -2136,   206, -1707, \
    5461, -5885,  2762, -1354, 19656, 19656, -1354,  2762, -5885,  5461, \
    -1707,   206, -2136,  1661,   111,   476,  -746,  -108,   176,    98, \
    -13,  -244,   328,  -208,   241,  -275,   123,   -28,    46,   -55, \
    24,     4,   -47,    62,   -22,    52,   -79,    47,   -63,    38, \
    5,    40,   -44,   -18,    39,   -17,    27,   -56,    15,    26, \
    \
    -86,    73,  -153,   155,  -159,    46,    35,  -237,   309,  -476, \
    197,  -317,    -7,   -32,  -170,   -50,    44,   -50,  -172,   283, \
    -355,   226,  -380,   453, -1049,  1171, -1117,   733,   624, -1369, \
    3057, -3450,  3730, -1053,   478,  3304, -4044,  3533, -3125,  2856, \
    4304, -12328, 23197, -17817, 23197, 23197, -17817, 23197, -12328,  4304, \
    2856, -3125,  3533, -4044,  3304,   478, -1053,  3730, -3450,  3057, \
    -1369,   624,   733, -1117,  1171, -1049,   453,  -380,   226,  -355, \
    283,  -172,   -50,    44,   -50,  -170,   -32,    -7,  -317,   197, \
    -476,   309,  -237,    35,    46,  -159,   155,  -153,    73,   -86
};
uWord32 DefaultVoIPEnhancePara[EnhanceParasNum] =
{96, 253, 16388, 31, 57607, 31, 400, 64, 80, 4325, 611, 0, 16392, 0, 0, 8192, 0, 0, 0, 10752, 32769, 0, 0, 0, 0, 0, 0, 0};

#define MAX_DUMP_NUM (1024)
int SPELayer::DumpFileNum = 0;

int SPELayer::GetVoIPJitterTime(void)
{
    char value[PROPERTY_VALUE_MAX];
    int ret = 0;
    ret = property_get("SetJitterTime", value, "0");
    int JitterTime = atoi(value);
    ALOGD("GetVoIPJitterTime JitterTime=%d,ret=%d", JitterTime, ret);

    return JitterTime;
}
int SPELayer::GetVoIPLatencyTime(void)
{
    char value[PROPERTY_VALUE_MAX];
    int ret = 0;
    ret = property_get("SetLatencyTime", value, "0");
    int LatencyTime = atoi(value);
    ALOGD("GetVoIPLatencyTime LatencyTime=%d,ret=%d", LatencyTime, ret);

    return LatencyTime;
}

SPELayer::SPELayer()
{
    Mutex::Autolock lock(mLock);
    ALOGD("SPELayer::SPELayer");

    mSphCtrlBuffer = NULL;
    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferDLDelay = NULL;
    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mSPEProcessBufSize = 0;
    mMMI_ctrl_mask = MMI_CONTROL_MASK_ALL;

    //Record settings
    mRecordSampleRate = 48000;  // sample rate=48k HSR record   if sample rate=48k normal record
    mRecordFrameRate = 20;  // frame rate=20ms
    mRecordMICDigitalGain = 16; //MIC_DG for AGC
    mRecordULTotalGain = 184;   //uplink totol gain
    mRecordApp_table = 8;   //mode = "8" stereo record, "4" mono record
    mRecordFea_Cfg_table = 511; //without turing off anything

    //VoIP settings
    mPlatformOffsetTime = 0;
    mLatencyTime = 0;
    mVoIPSampleRate = 16000;    //sample rate=16k
    mLatencySampleCount = 0;    //Latency sample count, one channel
    mVoIPFrameRate = 20;    //frame rate=20ms
    mVoIPMICDigitalGain = 16;   //MIC_DG  for AGC
    mVoIPULTotalGain = 184;   //uplink totol gain
    mVoIPApp_table = 2; //mode=WB_VOIP
    mVoIPFea_Cfg_table = 511;   //without turning off anything
    mNeedDelayLatency = false;
    mLatencyDir = true;

    mNeedJitterBuffer = false;
    mDefaultJitterBufferTime = 0;
    mJitterBufferTime = GetVoIPJitterTime();
    mJitterSampleCount = mJitterBufferTime * mVoIPSampleRate / 1000; //Jitter buffer sample count, one channel

    //default record and VoIP parameters
    for (int i = 0; i < EnhanceParasNum; i++)
    {
        mRecordEnhanceParas[i] = DefaultRecEnhancePara[i];
        mVoIPEnhanceParas[i] = DefaultVoIPEnhancePara[i];
    }

    for (int i = 0; i < DMNRCalDataNum; i++)
    {
        mRecordDMNRCalData[i] = DefaultRecDMNR_cal_data[i];
        mVoIPDMNRCalData[i] = DefaultVoIPDMNR_cal_data[i];
    }

    for (int i = 0; i < CompenFilterNum; i++)
    {
        mRecordCompenFilter[i] = DefaultRecCompen_filter[i];
        mVoIPCompenFilter[i] = DefaultVoIPCompen_filter[i];
    }


    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;
    mState = SPE_STATE_IDLE;
    mError = false;
    mVoIPRunningbefore = false;

    //for debug purpose
    mfpInDL = NULL;
    mfpInUL = NULL;
    mfpOutDL = NULL;
    mfpOutUL = NULL;
    mfpProcessedDL = NULL;
    mfpProcessedUL = NULL;
    mfpEPL = NULL;
    mfpVM = NULL;
    mVMDumpEnable = false;

    hDumpThread = NULL;

    memset(&mSph_Enh_ctrl, 0, sizeof(SPH_ENH_ctrl_struct));

    int ret = 0;

    ret = pthread_mutex_init(&mBufMutex, NULL);
    if (ret != 0)
    {
        ALOGD("Failed to initialize mBufMutex!");
    }

    ret = pthread_cond_init(&mBuf_Cond, NULL);
    if (ret != 0)
    {
        ALOGD("Failed to initialize mBuf_Cond!");
    }

    ret = pthread_mutex_init(&mDumpExitMutex, NULL);
    if (ret != 0)
    {
        ALOGD("Failed to initialize mDumpExitMutex!");
    }

    ret = pthread_cond_init(&mDumpExit_Cond, NULL);
    if (ret != 0)
    {
        ALOGD("Failed to initialize mDumpExit_Cond!");
    }

    mOutputStreamRunning = false;
    mNormalModeVoIP = false;
    mULDropTime = 0;
    mDLLatencyTime = 0;
    mCompensatedBufferSize = 0;
    mFirstVoIPUplink = true;
    mFirstVoIPDownlink = true;
    mPreULBufLen = 0;
    mPreDLBufLen = 0;
    mDLNewStart = false;
    mPrepareProcessDataReady = false;
    mDLPreQnum = 5;//5 * 4;
    mDLPreQLimit = true;
    mDLlateAdjust = false;
    DLdataPrepareCount = 0;
    mNewReferenceBufferComes = false;
    memset(&mUplinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreUplinkEstTime, 0, sizeof(timespec));

    memset(&mDownlinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreDownlinkEstTime, 0, sizeof(timespec));
    memset(&mPreDownlinkQueueTime, 0, sizeof(timespec));

    mDLStreamAttribute.mBufferSize = 8192;
    mDLStreamAttribute.mChannels = 2;
    mDLStreamAttribute.mSampleRate = 44100;

    Dump_Enalbe_Check();
}

SPELayer::~SPELayer()
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    ALOGD("~SPELayer");
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 20 * 1000000;

    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;
    //mState = SPE_STATE_IDLE;

    mError = false;

    Clear();

    FlushBufferQ();

    if (hDumpThread != NULL)
    {
        hDumpThread = NULL;
        if (ETIMEDOUT == pthread_cond_timedwait_relative_np(&mDumpExit_Cond, &mDumpExitMutex, &timeout))
        {
            ALOGD("~SPELayer dumpthread close timeout?");
        }
        pthread_mutex_unlock(&mDumpExitMutex);
        DumpBufferClear();
    }

    if (mfpInDL)
    {
        fclose(mfpInDL);
        mfpInDL = NULL;
    }
    if (mfpInUL)
    {
        fclose(mfpInUL);
        mfpInUL = NULL;
    }
    if (mfpOutDL)
    {
        fclose(mfpOutDL);
        mfpOutDL = NULL;
    }
    if (mfpOutUL)
    {
        fclose(mfpOutUL);
        mfpOutUL = NULL;
    }
    if (mfpProcessedDL)
    {
        fclose(mfpProcessedDL);
        mfpProcessedDL = NULL;
    }
    if (mfpProcessedUL)
    {
        fclose(mfpProcessedUL);
        mfpProcessedUL = NULL;
    }
    if (mfpEPL)
    {
        fclose(mfpEPL);
        mfpEPL = NULL;
    }
    if (mfpVM)
    {
        fclose(mfpVM);
        mfpVM = NULL;
    }

    DumpFileNum++;
    DumpFileNum %= MAX_DUMP_NUM;
    pthread_mutex_unlock(&mBufMutex);
    ALOGD("~SPELayer --");
}

void SPELayer::FlushBufferQ(void)
{
    ALOGD("FlushBufferQ+++");
    //clear the buffer queue, need mutex protect

    ALOGD("FlushBufferQ mULOutBufferQ size=%d,mULInBufferQ.size=%d,mDLOutBufferQ.size()=%d,mDLInBufferQ.size()=%d,mDLDelayBufferQ.size()=%d", mULOutBufferQ.size(), mULInBufferQ.size(),
          mDLOutBufferQ.size(), mDLInBufferQ.size(), mDLDelayBufferQ.size());
    if (mULOutBufferQ.size() != 0)
    {
        while (mULOutBufferQ.size())
        {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
        }
        mULOutBufferQ.clear();
    }
    if (mULInBufferQ.size() != 0)
    {
        while (mULInBufferQ.size())
        {
            free(mULInBufferQ[0]->pBufBase);
            delete mULInBufferQ[0];
            mULInBufferQ.removeAt(0);
        }
        mULInBufferQ.clear();
    }

    if (mDLOutBufferQ.size() != 0)
    {
        while (mDLOutBufferQ.size())
        {
            free(mDLOutBufferQ[0]->pBufBase);
            delete mDLOutBufferQ[0];
            mDLOutBufferQ.removeAt(0);
        }
        mDLOutBufferQ.clear();
    }
    if (mDLInBufferQ.size() != 0)
    {
        while (mDLInBufferQ.size())
        {
            if (mDLInBufferQ[0]->pBufBase)
            {
                ALOGD("mDLInBufferQ::pBufBase=%p", mDLInBufferQ[0]->pBufBase);
                //                free(mDLInBufferQ[0]->pBufBase);
                //                ALOGD("mDLInBufferQ::free");
                //                delete mDLInBufferQ[0];
                //                ALOGD("mDLInBufferQ::delete");
                mDLInBufferQ.removeAt(0);
                ALOGD("mDLInBufferQ::done, free at DLDelay buffer");
            }
        }
        mDLInBufferQ.clear();
    }

    if (mDLDelayBufferQ.size() != 0)
    {
        while (mDLDelayBufferQ.size())
        {
            if (mDLDelayBufferQ[0]->pBufBase)
            {
                ALOGD("mDLDelayBufferQ::pBufBase=%p", mDLDelayBufferQ[0]->pBufBase);
                free(mDLDelayBufferQ[0]->pBufBase);
                ALOGD("mDLDelayBufferQ::free");
                delete mDLDelayBufferQ[0];
                ALOGD("mDLDelayBufferQ::delete");
                mDLDelayBufferQ.removeAt(0);
                ALOGD("mDLDelayBufferQ::done");
            }

        }
        mDLDelayBufferQ.clear();
    }

    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mCompensatedBufferSize = 0;

    ALOGD("FlushBufferQ---");
}

bool    SPELayer::MutexLock(void)
{
    mLock.lock();
    return true;
}
bool    SPELayer::MutexUnlock(void)
{
    mLock.unlock();
    return true;
}

bool SPELayer::DumpMutexLock(void)
{
    mDumpLock.lock();
    return true;
}
bool SPELayer::DumpMutexUnlock(void)
{
    mDumpLock.unlock();
    return true;
}

//set MMI table, dynamic on/off
bool SPELayer::SetDynamicFuncCtrl(const SPE_MMI_CONTROL_TABLE func, const bool enable)
{
    Mutex::Autolock lock(mLock);
    const bool current_state = ((mMMI_ctrl_mask & func) > 0);
    ALOGD("%s(), SetDynamicFuncCtrl %x(%x), enable(%d) == current_state(%d)",
          __FUNCTION__, mMMI_ctrl_mask, func, enable, current_state);
    /*    if (current_state == enable) {
            return false;
        }*/
    if (enable == false)
    {
        mMMI_ctrl_mask &= (~func);
    }
    else
    {
        mMMI_ctrl_mask |= func;
        //normal/handsfree mode DMNR are exclusive
        if (func == HANDSFREE_DMNR)
        {
            mMMI_ctrl_mask &= (~NORMAL_DMNR);
        }
        else if (func == NORMAL_DMNR)
        {
            mMMI_ctrl_mask &= (~HANDSFREE_DMNR);
        }
    }

    mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
    ALOGD("%s(), SetDynamicFuncCtrl %x", __FUNCTION__, mMMI_ctrl_mask);
    return true;
}

//parameters setting
bool    SPELayer::SetEnhPara(SPE_MODE mode, unsigned long *pEnhance_pars)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(&mRecordEnhanceParas, pEnhance_pars, EnhanceParasNum * sizeof(unsigned long));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(&mVoIPEnhanceParas, pEnhance_pars, EnhanceParasNum * sizeof(unsigned long));
            break;
        default:
            ALOGD("SPELayer::SetEnhPara, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetEnhPara, SPE_MODE=%d", mode);
    /*  for(int i=0; i<EnhanceParasNum; i++)
        {
                ALOGD("mRecordEnhanceParas[%d] %d",i,mRecordEnhanceParas[i]);
                ALOGD("mSph_Enh_ctrl.enhance_pars[%d] %d",i,mSph_Enh_ctrl.enhance_pars[i]);
        }   */
    return true;
}

bool    SPELayer::SetDMNRPara(SPE_MODE mode, short *pDMNR_cal_data)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(&mRecordDMNRCalData, pDMNR_cal_data, DMNRCalDataNum * sizeof(short));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(&mVoIPDMNRCalData, pDMNR_cal_data, DMNRCalDataNum * sizeof(short));
            break;
        default:
            ALOGD("SPELayer::SetDMNRPara, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetDMNRPara, SPE_MODE=%d", mode);

    return true;
}

bool    SPELayer::SetCompFilter(SPE_MODE mode, short *pCompen_filter)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(&mRecordCompenFilter, pCompen_filter, CompenFilterNum * sizeof(short));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(&mVoIPCompenFilter, pCompen_filter, CompenFilterNum * sizeof(short));
            break;
        default:
            ALOGD("SPELayer::SetDMNRPara, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetCompFilter, SPE_MODE=%d", mode);

    return true;
}

bool    SPELayer::SetMICDigitalGain(SPE_MODE mode, long gain)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            mRecordMICDigitalGain = gain;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            mVoIPMICDigitalGain = gain;
            break;
        default:
            ALOGD("SPELayer::SetMICDigitalGain, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetMICDigitalGain MIC_DG, SPE_MODE=%d, gain=%d", mode, gain);

    return true;
}


bool SPELayer::SetUpLinkTotalGain(SPE_MODE mode, uint8_t gain)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            mRecordULTotalGain = gain;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            mVoIPULTotalGain = gain;
            break;
        default:
            ALOGD("SPELayer::SetUpLinkTotalGain, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetUpLinkTotalGain, SPE_MODE=%d, gain=%d", mode, gain);

    return true;
}

bool    SPELayer::SetSampleRate(SPE_MODE mode, long sample_rate)
{

    switch (mode)
    {
        case SPE_MODE_REC:
            if (sample_rate != 16000 && sample_rate != 48000)
            {
                ALOGD("SPELayer::SetSampleRate, Record only support 16k or 48k samplerate");
                mRecordSampleRate = 48000;
                return false;
            }
            mRecordSampleRate = sample_rate;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            if (sample_rate != 16000)
            {
                ALOGD("SPELayer::SetSampleRate, VOIP only support 16k samplerate");
            }

            mVoIPSampleRate = 16000;
            break;
        default:
            ALOGD("SPELayer::SetSampleRate, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetSampleRate, SPE_MODE=%d", mode);

    return true;
}

bool    SPELayer::SetFrameRate(SPE_MODE mode, long frame_rate)
{

    //fixme: now only support 20ms frame rate
    frame_rate = 20;

    if (frame_rate != 10 && frame_rate != 20)
    {
        ALOGD("SPELayer::SetFrameRate, only support 10ms and 20ms framerate");
        return false;
    }

    switch (mode)
    {
        case SPE_MODE_REC:
            mRecordFrameRate = frame_rate;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            mVoIPFrameRate = frame_rate;
            break;
        default:
            ALOGD("SPELayer::SetFrameRate, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetFrameRate, SPE_MODE=%d, frame_rate=%d", mode, frame_rate);

    return true;
}

bool    SPELayer::SetAPPTable(SPE_MODE mode, SPE_APP_TABLE App_table)
{
    switch (mode)
    {
        case SPE_MODE_REC:
            mRecordApp_table = App_table;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            mVoIPApp_table = App_table;
            break;
        default:
            ALOGD("SPELayer::SetAPPTable, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetAPPTable, SPE_MODE=%d, App_table=%x", mode, App_table);

    return true;
}

bool    SPELayer::SetFeaCfgTable(SPE_MODE mode, long Fea_Cfg_table)
{

    switch (mode)
    {
        case SPE_MODE_REC:
            mRecordFea_Cfg_table = Fea_Cfg_table;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            mVoIPFea_Cfg_table = Fea_Cfg_table;
            break;
        default:
            ALOGD("SPELayer::SetFeaCfgTable, not support mode");
            return false;
            break;
    }

    ALOGD("SPELayer::SetFeaCfgTable, SPE_MODE=%d,Fea_Cfg_table=%x", mode, Fea_Cfg_table);

    return true;
}

bool SPELayer::SetPlatfromTimeOffset(int ms)
{
    ALOGD("SPELayer::SetPlatfromTimeOffset, old=%d, new=%d", mPlatformOffsetTime, ms);
    mPlatformOffsetTime = ms;
    return true;
}

bool    SPELayer::SetChannel(int channel)
{
    if (channel == 1) //mono
    {
        mRecordApp_table = 4;    //mode = "8" stereo record, "4" mono record
    }
    else    //stereo
    {
        mRecordApp_table = 8;
    }

    ALOGD("SPELayer::SetChannel only for recording, mRecordApp_table=%x", mRecordApp_table);
    return true;
}

int SPELayer::GetChannel()
{
    if (mRecordApp_table == 4)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}
/*
bool    SPELayer::SetMode(SPE_MODE mode)
{
    ALOGD("SPELayer::SetMode %d",mode);

    if((mode!=SPE_MODE_REC) && (mode!=SPE_MODE_VOIP))
    {
        ALOGD("SPELayer::SetMode, SPE_MODE not correct");
        return false;
    }

    if (mMode == mode)
        return true;

    mMode = mode;
    return true;
}
*/

bool    SPELayer::SetRoute(SPE_ROUTE route)
{
    ALOGD("SPELayer::SetRoute %d", route);

    if ((route < ROUTE_NONE) && (route > ROUTE_BT))
    {
        ALOGD("SPELayer::SetRoute, route not correct");
        return false;
    }

    if (mRoute == route)
    {
        return true;
    }

    mRoute = route;
    return true;
}

void SPELayer::SetStreamAttribute(bool direct, StreamAttribute SA)//direct=0 =>downlink stream info, direct=1 =>uplink stream info
{
    ALOGD("%s(), direct=%d", __FUNCTION__ , direct);
    if (direct)
    {
        ALOGD("%s(), not support uplink stream info yet", __FUNCTION__);
    }
    else
    {
        memcpy(&mDLStreamAttribute, &SA, sizeof(StreamAttribute));
    }
    ALOGD("%s(), mBufferSize=%d, mChannels=%d, mSampleRate=%d", __FUNCTION__ , mDLStreamAttribute.mBufferSize, mDLStreamAttribute.mChannels, mDLStreamAttribute.mSampleRate);
    CalPreQNumber();
    return;
}

void SPELayer::CalPreQNumber(void)
{
    int framecount = mDLStreamAttribute.mBufferSize / mDLStreamAttribute.mChannels / 2;
    int DLPreQfact = 200 / (framecount * 1000 / mDLStreamAttribute.mSampleRate);
    mDLPreQnum = DLPreQfact;
    ALOGD("%s(), mDLPreQnum=%d, DLdataPrepareCount=%d", __FUNCTION__ , mDLPreQnum, DLdataPrepareCount);
}

void SPELayer::CalPrepareCount(void)
{
    int framecount = mDLStreamAttribute.mBufferSize / mDLStreamAttribute.mChannels / 2;
    int DLPreparefact = 100 / (framecount * 1000 / mDLStreamAttribute.mSampleRate);
    DLdataPrepareCount = DLPreparefact;
    ALOGD("%s(), mDLPreQnum=%d, DLdataPrepareCount=%d", __FUNCTION__ , mDLPreQnum, DLdataPrepareCount);
}

//Get parameters setting
int SPELayer::GetLatencyTime()
{
    //only for VoIP
    return mLatencyTime;
}

SPE_MODE    SPELayer::GetMode()
{
    return mMode;
}

SPE_ROUTE   SPELayer::GetRoute()
{
    return mRoute;
}
bool    SPELayer::IsSPERunning()
{
    if (mState == SPE_STATE_RUNNING)
    {
        return true;
    }
    else
    {
        return false;
    }
}


long    SPELayer::GetSampleRate(SPE_MODE mode)
{
    long retSampleRate = 0;
    switch (mode)
    {
        case SPE_MODE_REC:
            retSampleRate = mRecordSampleRate;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            retSampleRate = mVoIPSampleRate;
            break;
        default:
            retSampleRate = mSph_Enh_ctrl.sample_rate;
            break;
    }

    ALOGD("SPELayer::GetSampleRate, SPE_MODE=%d, retSampleRate=%d", mode, retSampleRate);
    return retSampleRate;
}

long    SPELayer::GetFrameRate(SPE_MODE mode)
{
    long retFrameRate = 0;
    switch (mode)
    {
        case SPE_MODE_REC:
            retFrameRate = mRecordFrameRate;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            retFrameRate = mVoIPFrameRate;
            break;
        default:
            retFrameRate = mSph_Enh_ctrl.frame_rate;
            break;
    }

    ALOGD("SPELayer::GetFrameRate, SPE_MODE=%d, retFrameRate=%d", mode, retFrameRate);
    return retFrameRate;
}

long    SPELayer::GetMICDigitalGain(SPE_MODE mode)
{
    long retPGAGain = 0;
    switch (mode)
    {
        case SPE_MODE_REC:
            retPGAGain = mRecordMICDigitalGain;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            retPGAGain = mVoIPMICDigitalGain;
            break;
        default:
            retPGAGain = mSph_Enh_ctrl.MIC_DG;
            break;
    }

    ALOGD("SPELayer::GetMICDigitalGain, SPE_MODE=%d, retPGAGain=%d", mode, retPGAGain);
    return retPGAGain;
}

long    SPELayer::GetAPPTable(SPE_MODE mode)
{
    long retAPPTable = 0;
    switch (mode)
    {
        case SPE_MODE_REC:
            retAPPTable = mRecordApp_table;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            retAPPTable = mVoIPApp_table;
            break;
        default:
            retAPPTable = mSph_Enh_ctrl.App_table;
            break;
    }

    ALOGD("SPELayer::GetAPPTable, SPE_MODE=%d, retAPPTable=%x", mode, retAPPTable);
    return retAPPTable;
}

long    SPELayer::GetFeaCfgTable(SPE_MODE mode)
{
    long retFeaCfgTable = 0;
    switch (mode)
    {
        case SPE_MODE_REC:
            retFeaCfgTable = mRecordFea_Cfg_table;
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            retFeaCfgTable = mVoIPFea_Cfg_table;
            break;
        default:
            retFeaCfgTable = mSph_Enh_ctrl.Fea_Cfg_table;
            break;
    }

    ALOGD("SPELayer::GetFeaCfgTable, SPE_MODE=%d, retFeaCfgTable=%x", mode, retFeaCfgTable);
    return retFeaCfgTable;
}

bool    SPELayer::GetEnhPara(SPE_MODE mode, unsigned long *pEnhance_pars)
{
    ALOGD("SPELayer::GetEnhPara, SPE_MODE=%d", mode);
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(pEnhance_pars, &mRecordEnhanceParas, EnhanceParasNum * sizeof(unsigned long));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(pEnhance_pars, &mVoIPEnhanceParas, EnhanceParasNum * sizeof(unsigned long));
            break;
        default:
            memcpy(pEnhance_pars, &mSph_Enh_ctrl.enhance_pars, EnhanceParasNum * sizeof(unsigned long));
            break;
    }

    return true;
}

bool    SPELayer::GetDMNRPara(SPE_MODE mode, short *pDMNR_cal_data)
{
    ALOGD("SPELayer::GetDMNRPara, SPE_MODE=%d", mode);
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(pDMNR_cal_data, &mRecordDMNRCalData, DMNRCalDataNum * sizeof(short));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(pDMNR_cal_data, &mVoIPDMNRCalData, DMNRCalDataNum * sizeof(short));
            break;
        default:
            memcpy(pDMNR_cal_data, &mSph_Enh_ctrl.DMNR_cal_data, DMNRCalDataNum * sizeof(short));
            break;
    }

    return true;
}

bool    SPELayer::GetCompFilter(SPE_MODE mode, short *pCompen_filter)
{
    ALOGD("SPELayer::GetCompFilter, SPE_MODE=%d", mode);
    switch (mode)
    {
        case SPE_MODE_REC:
            memcpy(pCompen_filter, &mRecordCompenFilter, CompenFilterNum * sizeof(short));
            break;
        case SPE_MODE_VOIP:
        case SPE_MODE_AECREC:
            memcpy(pCompen_filter, &mVoIPCompenFilter, CompenFilterNum * sizeof(short));
            break;
        default:
            memcpy(pCompen_filter, &mSph_Enh_ctrl.Compen_filter, CompenFilterNum * sizeof(short));
            break;
    }

    return true;
}

bool SPELayer::GetUPlinkIntrStartTime()
{
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING)
    {
        return false;
    }

    mUplinkIntrStartTime = GetSystemTime();
    ALOGD("GetUPlinkIntrStartTime sec=%d, nsec=%d", mUplinkIntrStartTime.tv_sec, mUplinkIntrStartTime.tv_nsec);
    mFirstVoIPUplink = true;
    mDLPreQLimit = false;
    return true;;
}

bool SPELayer::SetUPLinkIntrStartTime(struct timespec UPlinkStartTime)
{
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING)
    {
        return false;
    }

    mUplinkIntrStartTime = UPlinkStartTime;
    ALOGD("SetUPLinkIntrStartTime sec=%d, nsec=%d", mUplinkIntrStartTime.tv_sec, mUplinkIntrStartTime.tv_nsec);
    mFirstVoIPUplink = true;
    mDLPreQLimit = false;
    return true;;
}

bool SPELayer::GetDownlinkIntrStartTime()
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    mDownlinkIntrStartTime = GetSystemTime();
    ALOGD("GetDownlinkIntrStartTime sec=%d, nsec=%d, size=%d, mDLDelayBufferQ size()=%d", mDownlinkIntrStartTime.tv_sec, mDownlinkIntrStartTime.tv_nsec, mDLInBufferQ.size(), mDLDelayBufferQ.size());
    if (mDLInBufferQ.size() > 0)
    {
        for (size_t i = 0; i < mDLInBufferQ.size() ; i++)
        {
            if (mDLInBufferQ[i]->DLfirstBuf == true)
            {
                ALOGD("GetDownlinkIntrStartTime update estimate time");
                mDLInBufferQ[i]->time_stamp_estimate.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mDLInBufferQ[i]->time_stamp_estimate.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
                mPreDownlinkEstTime.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mPreDownlinkEstTime.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
                mPreDownlinkQueueTime.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mPreDownlinkQueueTime.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
            }
        }
        for (size_t i = 0; i < mDLDelayBufferQ.size() ; i++)
        {
            if (mDLDelayBufferQ[i]->DLfirstBuf == true)
            {
                ALOGD("GetDownlinkIntrStartTime update estimate time mDLDelayBufferQ");
                mDLDelayBufferQ[i]->time_stamp_estimate.tv_sec = mDownlinkIntrStartTime.tv_sec;
                mDLDelayBufferQ[i]->time_stamp_estimate.tv_nsec = mDownlinkIntrStartTime.tv_nsec;
            }
        }
    }
    mDLNewStart = false;
    pthread_mutex_unlock(&mBufMutex);
    return true;;
}

#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
void SPELayer::SetEchoRefStartTime(struct timespec EchoRefStartTime)
{
    Mutex::Autolock lock(mLock);
    mDownlinkIntrStartTime = EchoRefStartTime;
    ALOGD("SetEchoRefStartTime sec=%d, nsec=%d, size=%d, mDLDelayBufferQ size()=%d", mDownlinkIntrStartTime.tv_sec, mDownlinkIntrStartTime.tv_nsec, mDLInBufferQ.size(), mDLDelayBufferQ.size());

    mDLNewStart = false;
}
#endif

//speech enhancement setting and process
bool    SPELayer::Start(SPE_MODE mode)  //for VOIP, both uplink/downlink
{

    ALOGD("SPELayer::Start mode=%d", mode);
    Mutex::Autolock lock(mLock);
    if (mState == SPE_STATE_RUNNING)
    {
        ALOGD("SPELayer::Start already start!");
        return false;
    }

    if ((mMode != SPE_MODE_NONE) && (mMode != mode))
    {
        pthread_mutex_lock(&mBufMutex);
        FlushBufferQ();
        pthread_mutex_unlock(&mBufMutex);
    }

    // set mSph_Enh_ctrl parameters
    if (mode == SPE_MODE_REC)
    {
        mSph_Enh_ctrl.sample_rate = mRecordSampleRate;
        mSph_Enh_ctrl.frame_rate = mRecordFrameRate;
        mSph_Enh_ctrl.MIC_DG = mRecordMICDigitalGain;
        mSph_Enh_ctrl.Fea_Cfg_table = mRecordFea_Cfg_table;
        mSph_Enh_ctrl.App_table = mRecordApp_table;
        mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
        mSph_Enh_ctrl.MMI_MIC_GAIN = mRecordULTotalGain;
        memcpy(&mSph_Enh_ctrl.enhance_pars, &mRecordEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mRecordDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        memcpy(&mSph_Enh_ctrl.Compen_filter, &mRecordCompenFilter, CompenFilterNum * sizeof(Word16));
        ALOGD("mRecordSampleRate=%d, mRecordFrameRate=%d, mRecordApp_table=%x", mRecordSampleRate, mRecordFrameRate, mRecordApp_table);
    }
    else if ((mode == SPE_MODE_VOIP) || (mode == SPE_MODE_AECREC))
    {
        mSph_Enh_ctrl.sample_rate = mVoIPSampleRate;
        mSph_Enh_ctrl.frame_rate = mVoIPFrameRate;
        mSph_Enh_ctrl.MIC_DG = mVoIPMICDigitalGain;
        mSph_Enh_ctrl.Fea_Cfg_table = mVoIPFea_Cfg_table;
        mSph_Enh_ctrl.App_table = mVoIPApp_table;
        if (mode == SPE_MODE_AECREC)
        {
            mNormalModeVoIP = true;
        }

        mSph_Enh_ctrl.MMI_ctrl = mMMI_ctrl_mask;
        mSph_Enh_ctrl.MMI_MIC_GAIN = mVoIPULTotalGain;
        memcpy(&mSph_Enh_ctrl.enhance_pars, &mVoIPEnhanceParas, EnhanceParasNum * sizeof(uWord32));
        memcpy(&mSph_Enh_ctrl.DMNR_cal_data, &mVoIPDMNRCalData, DMNRCalDataNum * sizeof(Word16));
        memcpy(&mSph_Enh_ctrl.Compen_filter, &mVoIPCompenFilter, CompenFilterNum * sizeof(Word16));

        mLatencyTime = mPlatformOffsetTime + GetVoIPLatencyTime();
        if (mLatencyTime != 0)
        {
            mNeedDelayLatency = true;
        }

        if (mLatencyTime < 0)
        {
            mLatencyTime = abs(mLatencyTime);
            mLatencyDir = false;
        }
        ALOGD("mLatencyTime=%d,mLatencyDir=%x", mLatencyTime, mLatencyDir);
        mLatencySampleCount = mLatencyTime * mVoIPSampleRate / 1000; //Latency sample count

        mJitterSampleCount = mJitterBufferTime * mVoIPSampleRate / 1000; //Jitter buffer sample count, one channel
        if (mJitterBufferTime != 0)
        {
            mNeedJitterBuffer = true;
        }

    }
    else
    {
        ALOGD("SPELayer::Start wrong mode");
        return false;
    }

    mSph_Enh_ctrl.Device_mode = mRoute;

    if (mSphCtrlBuffer)
    {
        ALOGD("SPELayer::Start mSphCtrlBuffer already exist, delete and recreate");
        ENH_API_Free(&mSph_Enh_ctrl);
        free(mSphCtrlBuffer);
        mSphCtrlBuffer = NULL;
    }

    uint32 mem_size;
    mem_size = ENH_API_Get_Memory(&mSph_Enh_ctrl);
    mSphCtrlBuffer = (int *) malloc(mem_size);
    if (mSphCtrlBuffer == NULL)
    {
        ALOGD("SPELayer::Start create fail");
        return false;
    }
    ALOGD("SPELayer::going to configure,mSphCtrlBuffer=%p,mem_size=%d", mSphCtrlBuffer, mem_size);
    memset(mSphCtrlBuffer, 0, mem_size);

    dump();
    pthread_mutex_lock(&mBufMutex);
    ENH_API_Alloc(&mSph_Enh_ctrl, (Word32 *)mSphCtrlBuffer);

    ENH_API_Rst(&mSph_Enh_ctrl);


    mMode = mode;
    mState = SPE_STATE_START;

    //set the address
    if (mMode == SPE_MODE_REC)
    {
        if (mSph_Enh_ctrl.frame_rate == 20) //frame rate = 20ms, buffer size 320*2, input/output use the same address
        {
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];

            if (mSph_Enh_ctrl.sample_rate == 16000)
            {
                ALOGD("Start 16K record!!!");
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[320];
                mSPEProcessBufSize = 320 * 2 * sizeof(short); //for 16k samplerate with  20ms frame rate (stereo)
            }
            else    //48k sample rate
            {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[RecBufSize20ms];
                mSPEProcessBufSize = RecBufSize20ms * 2 * sizeof(short); //for 48k samplerate with  20ms frame rate (stereo)
            }

        }
        else    //frame rate = 10ms, buffer size 480*2
        {

            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];


            if (mSph_Enh_ctrl.sample_rate == 16000)
            {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[160];
                mSPEProcessBufSize = 160 * 2 * sizeof(short); //for 16k samplerate with  10ms frame rate (stereo)
            }
            else    //48K samplerate
            {
                mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[RecBufSize10ms];
                mSPEProcessBufSize = RecBufSize10ms * 2 * sizeof(short); //for 48k samplerate with  10ms frame rate (stereo)
            }

        }
    }
    else    //VoIP mode
    {
        //only support 16K samplerate
        if (mSph_Enh_ctrl.frame_rate == 20) //frame rate = 20ms, buffer size 320*4
        {
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];
            mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[320];
            mpSPEBufferDL = &mSph_Enh_ctrl.PCM_buffer[640];
            mpSPEBufferDLDelay = &mSph_Enh_ctrl.PCM_buffer[960];

            mSPEProcessBufSize = 320 * 2 * sizeof(short); //for 16k samplerate with  20ms frame rate (stereo)
        }
        else    //frame rate = 10ms, buffer size 160*4
        {
            mpSPEBufferUL1 = &mSph_Enh_ctrl.PCM_buffer[0];
            mpSPEBufferUL2 = &mSph_Enh_ctrl.PCM_buffer[160];
            mpSPEBufferDL = &mSph_Enh_ctrl.PCM_buffer[320];
            mpSPEBufferDLDelay = &mSph_Enh_ctrl.PCM_buffer[480];

            mSPEProcessBufSize = 160 * 2 * sizeof(short); //for 16k samplerate with  20ms frame rate (stereo)

        }
        mpSPEBufferNE = mpSPEBufferUL1;
        mpSPEBufferFE = mpSPEBufferDL;
    }

    mNsecPerSample = 1000000000 / mVoIPSampleRate;
    ALOGD("mNsecPerSample=%lld", mNsecPerSample);

    pthread_mutex_unlock(&mBufMutex);
    /*
        mfpInDL = NULL;
        mfpInUL = NULL;
        mfpOutDL = NULL;
        mfpOutUL = NULL;
        mfpProcessedDL = NULL;
        mfpProcessedUL = NULL;
        mfpEPL = NULL;
        mfpVM = NULL;
    */
    ALOGD("mSPEProcessBufSize=%d", mSPEProcessBufSize);

    return true;
}

void SPELayer::WriteReferenceBuffer(struct InBufferInfo *Binfo)
{
    struct timespec entertime;
    struct timespec leavetime;
    unsigned long long timediff = 0;
    mBufMutexWantLock.lock();
    entertime = GetSystemTime();
    mNewReferenceBufferComes = true;
    pthread_mutex_lock(&mBufMutex);
    mBufMutexWantLock.unlock();

    //normal VoIP is running case, and path routing case
    if (((mState == SPE_STATE_RUNNING) && ((mMode == SPE_MODE_VOIP) || (mMode == SPE_MODE_AECREC))) || mVoIPRunningbefore)
    {
        ALOGD("WriteReferenceBuffer,inBufLength=%d", Binfo->BufLen);
        AddtoInputBuffer(DOWNLINK, Binfo);
    }//prequeue happens before VoIP is running (mode is VoIP and state is START or mode is None and state is IDLE)
    else if ((mState != SPE_STATE_CLEANING) && (mMode != SPE_MODE_REC))
    {
        ALOGD("WriteDLQueue,inBufLength=%d", Binfo->BufLen);
        AddtoInputBuffer(DOWNLINK, Binfo, true);
    }
    pthread_mutex_unlock(&mBufMutex);
    mNewReferenceBufferComes = false;
    leavetime = GetSystemTime();

    timediff = TimeDifference(leavetime, entertime);
    if (timediff > 20000000)
    {
        ALOGD("WriteReferenceBuffer, process too long? %lld", timediff);
    }

}

void SPELayer::SetOutputStreamRunning(bool bRunning, bool bFromOutputStart)
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    ALOGD("SetOutputStreamRunning %d, %d, %d", bRunning, mOutputStreamRunning, bFromOutputStart);
    if ((bRunning == true) && (bFromOutputStart == true))
    {
        mDLNewStart = true;
        mPrepareProcessDataReady = false;
    }

    if (bRunning == false)
    {
        mFirstVoIPDownlink = true;
        mDLlateAdjust = false;
    }
#if 1
    if ((mOutputStreamRunning == false) && (bRunning == true))  //need to re-add the delay latency when input informed output restart running
    {
        if (mLatencyTime != 0)
        {
            ALOGD("resync the latency delay time");
            mNeedDelayLatency = true;
        }

        if (mJitterBufferTime != 0)
        {
            mNeedJitterBuffer = true;
        }
    }
#endif
    mOutputStreamRunning = bRunning;
    pthread_mutex_unlock(&mBufMutex);
}

void SPELayer::EnableNormalModeVoIP(bool bSet)
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    ALOGD("EnableNormalModeVoIP %d", bSet);
    mNormalModeVoIP = bSet;
    pthread_mutex_unlock(&mBufMutex);
}

void SPELayer::SetUPLinkDropTime(uint32 droptime)
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    mULDropTime = droptime; //ms
    ALOGD("SetUPLinkDropTime %d", mULDropTime);
    pthread_mutex_unlock(&mBufMutex);
}

void SPELayer::SetDownLinkLatencyTime(uint32 latencytime)
{
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    mDLLatencyTime = latencytime; //ms
    ALOGD("SetDownLinkLatencyTime %d", mDLLatencyTime);
    pthread_mutex_unlock(&mBufMutex);
}

void SPELayer::AdjustDLDelayData(void)
{
    if (mCompensatedBufferSize > 0) //adjust compensate time
    {
        if (!mOutputStreamRunning)  //if output is not running, reset the mCompensatedBufferSize
        {
            mCompensatedBufferSize = 0;
        }

        ALOGD("AdjustDLDelayData mCompensatedBufferSize=%d", mCompensatedBufferSize);

        //todo: modify DL delay buffer and DL buffer
#if 0
        while (mCompensatedBufferSize)
        {
            if (mDLDelayBufferQ.isEmpty())
            {
                ALOGW("AdjustDLDelayData something wrong %d", mCompensatedBufferSize);
                mCompensatedBufferSize = 0;
                break;
            }

            if (mDLDelayBufferQ[0]->BufLen4Delay <= 0) //run out of DL  delay queue0 buffer
            {
                free(mDLDelayBufferQ[0]->pBufBase);
                delete mDLDelayBufferQ[0];
                mDLDelayBufferQ.removeAt(0);
            }
            ALOGD("AdjustDLDelayData BufLen4Delay=%d", mDLDelayBufferQ[0]->BufLen4Delay);
            if (mDLDelayBufferQ[0]->BufLen4Delay <= mCompensatedBufferSize)
            {
                mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                mCompensatedBufferSize -= mDLDelayBufferQ[0]->BufLen4Delay;
                mDLDelayBufferQ[0]->BufLen4Delay = 0;
                free(mDLDelayBufferQ[0]->pBufBase);
                delete mDLDelayBufferQ[0];
                mDLDelayBufferQ.removeAt(0);
            }
            else
            {
                mDLDelayBufQLenTotal -= mCompensatedBufferSize;
                mDLDelayBufferQ[0]->BufLen4Delay -= mCompensatedBufferSize;
                mDLDelayBufferQ[0]->pRead4Delay += (mCompensatedBufferSize >> 1);
                mCompensatedBufferSize = 0;
            }

        }

#endif
    }

}


void SPELayer::DropDownlinkData(uint32_t dropsample)
{
    uint32_t diffBufLength = dropsample * 2;
    while (diffBufLength > 0)
    {
        if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty())
        {
            ALOGW("DropDownlinkData no mDLInBufferQ data");
            break;
        }
        if ((diffBufLength > mDLInBufQLenTotal) || (diffBufLength > mDLDelayBufQLenTotal))
        {
            //time diff more than DL preQ data
            ALOGW("DropDownlinkData something wrong happened?");
            diffBufLength = mDLInBufQLenTotal;
            //break;
        }
        ALOGD("DropDownlinkData drop DL data diffBufLength=%d, mDLInBufferQ.size()=%d, mDLInBufferQ[0]->BufLen=%d!!!", diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
        if (diffBufLength >= mDLInBufferQ[0]->BufLen)
        {
            //drop DL data
            uint32 droplength = mDLInBufferQ[0]->BufLen;
            diffBufLength -= mDLInBufferQ[0]->BufLen;
            mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
            mDLInBufferQ.removeAt(0);

            //drop DL delay data
            while (droplength > 0)
            {
                ALOGD("DropDownlinkData drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                {
                    ALOGD("DropDownlinkData mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead);
                    mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                    mDLDelayBufQLenTotal -= droplength;
                    mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                    droplength = 0;
                    ALOGD("DropDownlinkData after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                }
                else
                {
                    droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                }
            }
        }
        else
        {
            //                    ALOGW("PrepareProcessData samtest!!! break for test");
            //                    break;
            //drop DL data
            ALOGD("DropDownlinkData mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                  mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
            uint32 droplength = diffBufLength;
            mDLInBufferQ[0]->BufLen -= diffBufLength;   //record the buffer you consumed
            mDLInBufQLenTotal -= diffBufLength;
            mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffBufLength / 2;

            //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

            uint32 adjustsample = diffBufLength / 2;
            unsigned long long updateDLnsecdiff = 0;
            updateDLnsecdiff = (adjustsample * 1000000) / 16;

            mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
            mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

            ALOGD("DropDownlinkData after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                  mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
            diffBufLength = 0;

            //drop DL delay data
            while (droplength > 0)
            {
                ALOGD("DropDownlinkData drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                {
                    ALOGD("DropDownlinkData mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead4Delay);
                    mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                    mDLDelayBufQLenTotal -= droplength;
                    mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                    droplength = 0;
                    ALOGD("DropDownlinkData after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                }
                else
                {
                    droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                }
            }

        }
    }

}

void SPELayer::AddtoInputBuffer(SPE_DATA_DIRECTION dir, struct InBufferInfo *BInputInfo, bool prequeue)
{

    //pthread_mutex_lock(&mBufMutex );
    int inBufLen = BInputInfo->BufLen;
    short *inBufAddr = BInputInfo->pBufBase;
    bool bRemainInfo = BInputInfo->bHasRemainInfo;
    bool bPreQueue = prequeue;

    Dump_PCM_In(dir, inBufAddr, inBufLen);

    //    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);

    BufferInfo *newInBuffer = new BufferInfo;
    memset(newInBuffer, 0, sizeof(BufferInfo));
    struct timespec tstamp_queue;
    newInBuffer->pBufBase = (short *) malloc(inBufLen);

    memcpy(newInBuffer->pBufBase, inBufAddr, inBufLen);

    //tstamp_queue = GetSystemTime(true, dir);  //modify to use read time
    tstamp_queue = BInputInfo->time_stamp_queued;

    newInBuffer->BufLen = inBufLen;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp_queue;
    newInBuffer->time_stamp_process = {0};
    newInBuffer->DLfirstBuf = false;
    /*
        if (prequeue)
        {
            ALOGD("AddtoInputBuffer, newInBuffer=%p, pBufBase=%p", newInBuffer, newInBuffer->pBufBase);
        }
    */
    if ((dir == UPLINK) && ((mMode == SPE_MODE_VOIP) || (mMode == SPE_MODE_AECREC)))
    {
        ALOGD("uplink estimate time bRemainInfo=%d, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d, tv_sec=%ld, pre nsec=%ld", 
            bRemainInfo, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, mPreDLBufLen,BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec);
        if (mFirstVoIPUplink)
        {
            mFirstVoIPUplink = false;
#if 1   //the first estimation time use the UL intr time, it is absolute time
            if (bRemainInfo)
            {
                mPreUplinkEstTime.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                mPreUplinkEstTime.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
            }
            else
            {
                mPreUplinkEstTime.tv_sec = mUplinkIntrStartTime.tv_sec;
                if (mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000 >= 1000000000)
                {
                    mPreUplinkEstTime.tv_sec++;
                    mPreUplinkEstTime.tv_nsec = mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000 - 1000000000;
                }
                else
                {
                    mPreUplinkEstTime.tv_nsec = mUplinkIntrStartTime.tv_nsec + mULDropTime * 1000000;
                }

            }
#else //ULTR
            //the first estimation time use the first UL buffer queue time, it is corresponding time
            mPreUplinkEstTime.tv_sec = tstamp_queue.tv_sec;
            mPreUplinkEstTime.tv_nsec = tstamp_queue.tv_nsec;

            if (tstamp_queue.tv_sec >= mUplinkIntrStartTime.tv_sec)
            {
                if (tstamp_queue.tv_nsec >= mUplinkIntrStartTime.tv_nsec)
                {
                    mULIntrDeltaTime.tv_sec = tstamp_queue.tv_sec - mUplinkIntrStartTime.tv_sec;
                    mULIntrDeltaTime.tv_nsec = tstamp_queue.tv_nsec - mUplinkIntrStartTime.tv_nsec;
                }
                else
                {
                    if (tstamp_queue.tv_sec >= mUplinkIntrStartTime.tv_sec)
                    {
                        ALOGW("first uplink estimate time error");
                    }
                    else
                    {
                        mULIntrDeltaTime.tv_sec = tstamp_queue.tv_sec - mUplinkIntrStartTime.tv_sec - 1;
                        mULIntrDeltaTime.tv_nsec = 1000000000 + tstamp_queue.tv_nsec - mUplinkIntrStartTime.tv_nsec;
                    }
                }
            }
#endif
            newInBuffer->time_stamp_estimate.tv_sec = mPreUplinkEstTime.tv_sec;
            newInBuffer->time_stamp_estimate.tv_nsec = mPreUplinkEstTime.tv_nsec;
            ALOGD("first uplink estimate time bRemainInfo=%d, sec %ld nsec %ld, inBufLength=%d, mULIntrDeltaTime sec %ld nsec %ld",
                  bRemainInfo, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, inBufLen, mULIntrDeltaTime.tv_sec, mULIntrDeltaTime.tv_nsec);
            mPreULBufLen = inBufLen;
        }
        else
        {
            if (bRemainInfo)
            {
                mPreUplinkEstTime.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                mPreUplinkEstTime.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;

                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
            }
            else
            {
                struct timespec Esttstamp;
                //unsigned long long ns = (((mPreULBufLen*1000000000)/2)/2)/mVoIPSampleRate;
                unsigned long long ns = ((mPreULBufLen * 1000000) / 64);
                Esttstamp.tv_sec = mPreUplinkEstTime.tv_sec;
                //ALOGD("uplink estimate mPreUplinkEstTime, ns=%lld, tv_sec=%ld, nsec=%ld, mPreDLBufLen=%d", ns, mPreUplinkEstTime.tv_sec, mPreUplinkEstTime.tv_nsec, mPreULBufLen);
                if (mPreUplinkEstTime.tv_nsec + ns >= 1000000000)
                {
                    Esttstamp.tv_sec++;
                    Esttstamp.tv_nsec = mPreUplinkEstTime.tv_nsec + ns - 1000000000;
                }
                else
                {
                    Esttstamp.tv_nsec = mPreUplinkEstTime.tv_nsec + ns;
                }

                newInBuffer->time_stamp_estimate.tv_sec = Esttstamp.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = Esttstamp.tv_nsec;
                ALOGD("uplink estimate time, sec %ld nsec %ld, inBufLength=%d", Esttstamp.tv_sec, Esttstamp.tv_nsec, inBufLen);
                mPreUplinkEstTime.tv_sec = Esttstamp.tv_sec;
                mPreUplinkEstTime.tv_nsec = Esttstamp.tv_nsec;
            }
            mPreULBufLen = inBufLen;
        }
    }

    if (dir == DOWNLINK)
    {
        ALOGD("AddtoInputBuffer queue downlink sec %ld nsec %ld, downlink sec %ld nsec %ld",
              BInputInfo->time_stamp_queued.tv_sec, BInputInfo->time_stamp_queued.tv_nsec, BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec);
        if (mFirstVoIPDownlink)
        {
            mFirstVoIPDownlink = false;
            if (mDLNewStart) //downlink starts first time, the first DL buffer queue will earlier than interrupt enable, it happens when output starts after input stream create
            {
                //mDLNewStart = false;
                newInBuffer->DLfirstBuf = true;

                //need to modify the estimate start time again when downlink Interrupt set.
                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                if (mDLLatencyTime * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000)
                {
                    newInBuffer->time_stamp_estimate.tv_sec++;
                    newInBuffer->time_stamp_estimate.tv_nsec = mDLLatencyTime * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                }
                mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
                mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;
                ALOGD("downlink first time mDLNewStart queue estimate time, sec %ld nsec %ld, inBufLength=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen);
            }
            else    //the first DL buffer queue after downlink already start, it happens when input stream create after output is running
            {
                if (bRemainInfo)
                {
                    newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                    newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;
                }
                else
                {
                    //use DL hardware buffer latency for estimate? or buffer length?
                    newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                    newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                    ALOGD("mDLLatencyTime=%d", mDLLatencyTime);
                    if ((mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000)
                    {
                        newInBuffer->time_stamp_estimate.tv_sec++;
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                    }
                    else
                    {
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec;
                    }
                }

                mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
                mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;

                mPreDownlinkQueueTime.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                mPreDownlinkQueueTime.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
                ALOGD("downlink first time queue estimate time, sec %ld nsec %ld, inBufLength=%d,bRemainInfo=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen, bRemainInfo);

            }
            mPreDLBufLen = inBufLen;
        }
        else    //not the first DL buffer queue, continuos queue
        {
            ALOGD("downlink estimate time bRemainInfo=%d, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d", bRemainInfo, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, mPreDLBufLen);
            if (bRemainInfo)
            {
                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_predict.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_predict.tv_nsec;

                //ALOGD("mDLLatencyTime=%d, predict sec= %ld, nsec=%ld, mPreDownlinkEstTime sec=%ld, nsec=%ld" , mDLLatencyTime,BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec);

                if ((TimeDifference(BInputInfo->time_stamp_predict, mPreDownlinkEstTime) > (mDLLatencyTime * 1000000)))
                {
                    if (!mDLlateAdjust)
                    {
                        //two downlink queue interval is larger than hardware buffer latency time, this buffer is playing directly since no previous data in the hardware buffer
                        ALOGD("downlink late time predict sec= %ld, nsec=%ld, mPreDownlinkQueueTime sec=%ld, nsec=%ld" , BInputInfo->time_stamp_predict.tv_sec, BInputInfo->time_stamp_predict.tv_nsec,
                              mPreDownlinkQueueTime.tv_sec, mPreDownlinkQueueTime.tv_nsec);
                        //mDLlateAdjust = true;
                        //need to do resync!!!
                        //mPrepareProcessDataReady = false;
                    }
                }
                /*else
                {

                    if (mDLlateAdjust && mPrepareProcessDataReady)
                    {
                        //mPrepareProcessDataReady = false;
                        mDLlateAdjust = false;
                        ALOGD("downlink late adjust, resync");
                    }
                }*/
            }
            else
            {
                struct timespec Esttstamp;
                //uint32 ns = mPreDLBufLen*1000000000/sizeof(short)/1/mVoIPSampleRate;   //downlink is mono data
                //unsigned long long ns = ((mPreDLBufLen*1000000000)/2)/mVoIPSampleRate;   //downlink is mono data
                unsigned long long diffns = 0;
                unsigned long long ns = ((mPreDLBufLen * 1000000) / 32); //downlink is mono data
                //ALOGD("downlink estimate time ns=%lld, pre tv_sec=%ld, pre nsec=%ld, mPreDLBufLen=%d", ns, mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, mPreDLBufLen);

                newInBuffer->time_stamp_estimate.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
                newInBuffer->time_stamp_estimate.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;

                //when the latest DL buffer queue and this DL buffer queue interval longer than hardware buffer latency, the new buffer will play directly
                //and if the next buffer queue longer than half hardware buffer latency, it will also play directly
                //if((TimeDifference(BInputInfo->time_stamp_queued,mPreDownlinkQueueTime)>(mDLLatencyTime*1000000))||
                //(mDLlateAdjust && (TimeDifference(BInputInfo->time_stamp_queued,mPreDownlinkQueueTime)>((mDLLatencyTime/2)*1000000))))
                if ((TimeDifference(BInputInfo->time_stamp_queued, mPreDownlinkQueueTime) > (mDLLatencyTime * 1000000)) || mDLlateAdjust)
                {
                    //if(!mDLlateAdjust)
                    {
                        //two downlink queue interval is larger than hardware buffer latency time, this buffer is playing directly since no previous data in the hardware buffer
                        ALOGD("downlink late time queue sec= %ld, nsec=%ld, mPreDownlinkQueueTime sec=%ld, nsec=%ld" , BInputInfo->time_stamp_queued.tv_sec, BInputInfo->time_stamp_queued.tv_nsec,
                              mPreDownlinkQueueTime.tv_sec, mPreDownlinkQueueTime.tv_nsec);
                        mDLlateAdjust = true;
                    }
                }
                else
                {
                    if ((mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec >= 1000000000)
                    {
                        newInBuffer->time_stamp_estimate.tv_sec++;
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec - 1000000000;
                    }
                    else
                    {
                        newInBuffer->time_stamp_estimate.tv_nsec = (mDLLatencyTime / 2) * 1000000 + newInBuffer->time_stamp_estimate.tv_nsec;
                    }

                    //mDLlateAdjust = false;
                }
            }

            mPreDownlinkQueueTime.tv_sec = BInputInfo->time_stamp_queued.tv_sec;
            mPreDownlinkQueueTime.tv_nsec = BInputInfo->time_stamp_queued.tv_nsec;
#if 1   //use queue time + HW buffer latency time           
            mPreDownlinkEstTime.tv_sec = newInBuffer->time_stamp_estimate.tv_sec;
            mPreDownlinkEstTime.tv_nsec = newInBuffer->time_stamp_estimate.tv_nsec;
            ALOGD("downlink queue estimate time, sec %ld nsec %ld, inBufLength=%d", mPreDownlinkEstTime.tv_sec, mPreDownlinkEstTime.tv_nsec, inBufLen);
#else
            //predict by previos time
            Esttstamp.tv_sec = mPreDownlinkEstTime.tv_sec;
            if (mPreDownlinkEstTime.tv_nsec + ns >= 1000000000)
            {
                Esttstamp.tv_sec++;
                Esttstamp.tv_nsec = mPreDownlinkEstTime.tv_nsec + ns - 1000000000;
            }
            else
            {
                Esttstamp.tv_nsec = mPreDownlinkEstTime.tv_nsec + ns;
            }
            newInBuffer->time_stamp_estimate.tv_sec = Esttstamp.tv_sec;
            newInBuffer->time_stamp_estimate.tv_nsec = Esttstamp.tv_nsec;
            ALOGD("downlink estimate time, sec %ld nsec %ld, inBufLength=%d", Esttstamp.tv_sec, Esttstamp.tv_nsec, inBufLen);

            mPreDownlinkEstTime.tv_sec = Esttstamp.tv_sec;
            mPreDownlinkEstTime.tv_nsec = Esttstamp.tv_nsec;
#endif

            mPreDLBufLen = inBufLen;
        }
    }


    //    ALOGD("inBufLength=%d,mULInBufQLenTotal=%d, Qsize=%d",newInBuffer->BufLen,mULInBufQLenTotal,mULInBufferQ.size());
    if (dir == UPLINK)
    {
        mULInBufferQ.add(newInBuffer);
        mULInBufQLenTotal += inBufLen;
        //          ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, size=%d",mULInBufQLenTotal,mULInBufferQ.size());
    }
    else
    {
        //queue to the downlink input buffer queue, downlink data channel is mono

        mDLInBufferQ.add(newInBuffer);
        mDLInBufQLenTotal += inBufLen;
        ALOGD("AddtoInputBuffer, mDLInBufferQ.size()=%d, mDLPreQnum=%d,mDLPreQLimit=%d,mFirstVoIPUplink=%d,mDLInBufQLenTotal=%d", mDLInBufferQ.size(), mDLPreQnum, mDLPreQLimit, mFirstVoIPUplink,
              mDLInBufQLenTotal);

        //also add to delay buffer queue

        newInBuffer->BufLen4Delay = inBufLen;
        newInBuffer->pRead4Delay = newInBuffer->pBufBase;
        newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
        mDLDelayBufferQ.add(newInBuffer);
        mDLDelayBufQLenTotal += inBufLen;
        //          ALOGD("SPELayer::Process, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());

        if (bPreQueue)
        {

            //ALOGD("AddtoInputBuffer, mDLInBufferQ.size()=%d, mDLPreQnum=%d,mDLPreQLimit=%d,mFirstVoIPUplink=%d",mDLInBufferQ.size(),mDLPreQnum,mDLPreQLimit,mFirstVoIPUplink);
#if 0
            for (int i; i < mDLInBufferQ.size(); i++)
            {
                ALOGD("mDLInBufferQ i=%d, length=%d, %p, Sec=%d, NSec=%ld", i, mDLInBufferQ[i]->BufLen, mDLInBufferQ[i]->pBufBase,
                      mDLInBufferQ[i]->time_stamp_estimate.tv_sec, mDLInBufferQ[i]->time_stamp_estimate.tv_nsec);
            }
#endif

            if ((mDLPreQLimit) || (!mDLPreQLimit && mFirstVoIPUplink)) //wait for uplink comes, only queue five buffer for reference
            {
                while (mDLInBufferQ.size() > mDLPreQnum)
                {
                    //ALOGD("free over queue, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d, BufLen=%d, %p, %p",mDLInBufferQ.size(),mDLInBufQLenTotal,mDLInBufferQ[0]->BufLen,mDLInBufferQ[0]->pBufBase,mDLInBufferQ[0]);
                    mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                    mDLInBufferQ.removeAt(0);
                }
            }
            else    //uplink interrupt starts, remove previous queue
            {
                //for(int i; i<mDLInBufferQ.size(); i++)
                while (!mDLInBufferQ.isEmpty())
                {
                    uint32 tempSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec;
                    unsigned long long tempNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                    uint32 tempsample = mDLInBufferQ[0]->BufLen / 2;
                    unsigned long long tempdeltaNSec = tempsample * 1000000 / 16;
                    unsigned long long tempEndNSec = tempNSec + tempdeltaNSec;
                    unsigned long long tempFinalNSec = 0;
                    uint32 tempFinalSec = tempSec;
                    ALOGD("check to move? %p, tempSec=%d, tempNSec=%lld, tempsample=%d", mDLInBufferQ[0]->pBufBase, tempSec, tempNSec, tempsample);

                    if (tempEndNSec > 1000000000)
                    {
                        tempFinalNSec = tempEndNSec - 1000000000;
                        tempFinalSec = tempFinalSec + 1;
                    }
                    else
                    {
                        tempFinalNSec = tempEndNSec;
                    }

                    ALOGD("tempFinalSec=%d, tempFinalNSec=%ld, tempdeltaNSec=%ld", tempFinalSec, tempFinalNSec, tempdeltaNSec);

                    if (mUplinkIntrStartTime.tv_sec > tempFinalSec)
                    {
                        mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                        mDLInBufferQ.removeAt(0);
                    }
                    else if (mUplinkIntrStartTime.tv_sec == tempFinalSec)
                    {
                        if (mUplinkIntrStartTime.tv_nsec >= tempFinalNSec)
                        {
                            mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                            mDLInBufferQ.removeAt(0);
                        }
                        else
                        {
                            //remove previous data in this buffer queue, will do it in the prepare data?
                            ALOGD("remove DL pre queue finish 1");
                            break;
                        }
                    }
                    else
                    {
                        ALOGD("remove DL pre queue finish 2");
                        break;
                    }

                }
            }
#if 0
            for (int i; i < mDLDelayBufferQ.size(); i++)
            {
                ALOGD("mDLDelayBufferQ i=%d, length=%d, %p", i, mDLDelayBufferQ[i]->BufLen, mDLDelayBufferQ[i]->pBufBase);
            }
#endif
            if (mDLPreQLimit || (!mDLPreQLimit && mFirstVoIPUplink)) //wait for uplink comes, only queue five buffer for
            {
                while (mDLDelayBufferQ.size() > mDLPreQnum)
                {
                    //ALOGD("free over queue, mDLDelayBufferQ size=%d,mDLDelayBufQLenTotal=%d, BufLen=%d, %p, %p",mDLDelayBufferQ.size(),mDLDelayBufQLenTotal,mDLDelayBufferQ[0]->BufLen4Delay,mDLDelayBufferQ[0]->pBufBase,mDLDelayBufferQ[0]);
                    mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                    free(mDLDelayBufferQ[0]->pBufBase);
                    delete mDLDelayBufferQ[0];
                    mDLDelayBufferQ.removeAt(0);
                    //ALOGD("free mDLDelayBufferQ over queue done");
                }
            }
            else    //uplink interrupt starts, remove previous queue
            {
                while (!mDLDelayBufferQ.isEmpty())
                {
                    uint32 tempSec = mDLDelayBufferQ[0]->time_stamp_estimate.tv_sec;
                    unsigned long long tempNSec = mDLDelayBufferQ[0]->time_stamp_estimate.tv_nsec;
                    uint32 tempsample = mDLDelayBufferQ[0]->BufLen / 2;
                    unsigned long long tempdeltaNSec = tempsample * 1000000 / 16;
                    unsigned long long tempEndNSec = tempNSec + tempdeltaNSec;
                    unsigned long long tempFinalNSec = 0;
                    uint32 tempFinalSec = tempSec;
                    ALOGD("mDLDelayBufferQ check to move? %p, tempSec=%d, tempNSec=%lld, tempsample=%d", mDLDelayBufferQ[0]->pBufBase, tempSec, tempNSec, tempsample);

                    if (tempEndNSec > 1000000000)
                    {
                        tempFinalNSec = tempEndNSec - 1000000000;
                        tempFinalSec = tempFinalSec + 1;
                    }
                    else
                    {
                        tempFinalNSec = tempEndNSec;
                    }

                    ALOGD("tempFinalSec=%d, tempFinalNSec=%ld, tempdeltaNSec=%ld", tempFinalSec, tempFinalNSec, tempdeltaNSec);

                    if (mUplinkIntrStartTime.tv_sec > tempFinalSec)
                    {
                        mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen;
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    }
                    else if (mUplinkIntrStartTime.tv_sec == tempFinalSec)
                    {
                        if (mUplinkIntrStartTime.tv_nsec >= tempFinalNSec)
                        {
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        }
                        else
                        {
                            //remove previous data in this buffer queue, will do it in the prepare data?
                            ALOGD("remove DL delay pre queue finish 1");
                            break;
                        }
                    }
                    else
                    {
                        ALOGD("remove DL delay pre queue finish 2");
                        break;
                    }

                }
            }

        }
        pthread_cond_signal(&mBuf_Cond);
    }

    //pthread_cond_signal(&mBuf_Cond);
    //pthread_mutex_unlock(&mBufMutex);
}

void SPELayer::CompensateBuffer(size_t BufLength, struct timespec CompenStartTime)
{
    //    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);

    ALOGD("CompensateBuffer, BufLength=%d, sec=%ld, nsec=%ld", BufLength, CompenStartTime.tv_sec, CompenStartTime.tv_nsec);
    BufferInfo *newInBuffer = new BufferInfo;
    struct timespec tstamp;
    newInBuffer->pBufBase = (short *) malloc(BufLength);

    //memset(newInBuffer->pBufBase, 0, BufLength);
    memset(newInBuffer->pBufBase, 0xCCCC, BufLength);

    tstamp = GetSystemTime();

    newInBuffer->BufLen = BufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp;
    newInBuffer->time_stamp_estimate = CompenStartTime;  //need to check the previous eatimate time
    newInBuffer->time_stamp_process = {0};

    //queue to the downlink input buffer queue, downlink data channel is mono
    mDLInBufferQ.add(newInBuffer);
    mDLInBufQLenTotal += BufLength;
    //      ALOGD("CompensateBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


    newInBuffer->BufLen4Delay = BufLength;
    newInBuffer->pRead4Delay = newInBuffer->pBufBase;
    newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
    mDLDelayBufferQ.add(newInBuffer);
    mDLDelayBufQLenTotal += BufLength;
    //    ALOGD("CompensateBuffer, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());
    //if(!mNeedDelayLatency && mOutputStreamRunning)  //compensate happen after DL first time queue, and only count when downlink is playback
    if (!mFirstVoIPDownlink && mOutputStreamRunning) //compensate happen after DL first time queue, and only count when downlink is playback
    {
        mCompensatedBufferSize += BufLength;
        ALOGD("CompensateBuffer, mCompensatedBufferSize=%d", mCompensatedBufferSize);
    }

}

bool    SPELayer::WaitforDownlinkData(void)
{
    bool bRet = true;
    struct timespec ts;
    uint32_t timeoutMs = 20;    //timeout time 20ms

    if (mNormalModeVoIP) //normal mode VoIP
    {
        if (!mOutputStreamRunning)  //no output running, might be normal record
        {
            timeoutMs = 0;
        }
        else    //output is running
        {
            if (!mPrepareProcessDataReady)
            {
                timeoutMs = 0;
            }
            else
                //if(mNeedDelayLatency) //first time output queue
                if (mFirstVoIPDownlink)
                {
                    timeoutMs = 10;
                }
        }
    }
    else        //In-communication mode VoIP
    {
        if (!mOutputStreamRunning)  //no output running, process uplink data directly
        {
            timeoutMs = 0;
        }
        else    //output is running
        {
            if (!mPrepareProcessDataReady)
            {
                timeoutMs = 0;
            }
            else
                //if(mNeedDelayLatency) //if no DL played before, it is the first time UL earlier case, wait DL for a short time
                if (mFirstVoIPDownlink)
                {
                    timeoutMs = 10;
                }
        }
    }

    ALOGD("WaitforDownlinkData pthread_cond_timedwait_relative_np start %d,mOutputStreamRunning=%d,mFirstVoIPDownlink=%d,mNormalModeVoIP=%d,mPrepareProcessDataReady=%d", timeoutMs, mOutputStreamRunning,
          mFirstVoIPDownlink,
          mNormalModeVoIP, mPrepareProcessDataReady);
    if (timeoutMs != 0)
    {
        ts.tv_sec  = timeoutMs / 1000;
        ts.tv_nsec = timeoutMs * 1000000;

        if (ETIMEDOUT == pthread_cond_timedwait_relative_np(&mBuf_Cond, &mBufMutex , &ts))
        {
            ALOGD("WaitforDownlinkData pthread_cond_timedwait_relative_np timeout");
            bRet = false;
        }
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

bool    SPELayer::InsertDownlinkData(void)
{
    bool bRet = true;
    struct timespec ts;
    uint32_t timeoutMs = 2; //timeout time 1ms


    ALOGD("InsertDownlinkData pthread_cond_timedwait_relative_np start %d,mOutputStreamRunning=%d,mFirstVoIPDownlink=%d,mNormalModeVoIP=%d,mPrepareProcessDataReady=%d", timeoutMs, mOutputStreamRunning,
          mFirstVoIPDownlink, mNormalModeVoIP, mPrepareProcessDataReady);
    if (timeoutMs != 0)
    {
        ts.tv_sec  = timeoutMs / 1000;
        ts.tv_nsec = timeoutMs * 1000000;

        if (ETIMEDOUT == pthread_cond_timedwait_relative_np(&mBuf_Cond, &mBufMutex , &ts))
        {
            ALOGD("InsertDownlinkData pthread_cond_timedwait_relative_np timeout");
            bRet = false;
        }
    }

    return bRet;

}

//return NS
unsigned long long SPELayer::TimeStampDiff(BufferInfo *BufInfo1, BufferInfo *BufInfo2)
{
    unsigned long long diffns = 0;
    struct timespec tstemp1 = BufInfo1->time_stamp_estimate;
    struct timespec tstemp2 = BufInfo2->time_stamp_estimate;

    //    ALOGD("TimeStampDiff time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec);

    if (tstemp1.tv_sec > tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec) * 1000000000) + tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec - 1) * 1000000000) + tstemp1.tv_nsec + 1000000000 - tstemp2.tv_nsec;
        }
    }
    else if (tstemp1.tv_sec == tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
    }
    else
    {
        if (tstemp2.tv_nsec >= tstemp1.tv_nsec)
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec) * 1000000000) + tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
        else
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec - 1) * 1000000000) + tstemp2.tv_nsec + 1000000000 - tstemp1.tv_nsec;
        }
    }
    ALOGD("TimeStampDiff time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, diffns=%lld" , tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec, diffns);
    return diffns;

}
unsigned long long SPELayer::TimeDifference(struct timespec time1, struct timespec time2)
{
    unsigned long long diffns = 0;
    struct timespec tstemp1 = time1;
    struct timespec tstemp2 = time2;

    //    ALOGD("TimeStampDiff time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec);

    if (tstemp1.tv_sec > tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec) * 1000000000) + tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec - 1) * 1000000000) + tstemp1.tv_nsec + 1000000000 - tstemp2.tv_nsec;
        }
    }
    else if (tstemp1.tv_sec == tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
    }
    else
    {
        if (tstemp2.tv_nsec >= tstemp1.tv_nsec)
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec) * 1000000000) + tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
        else
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec - 1) * 1000000000) + tstemp2.tv_nsec + 1000000000 - tstemp1.tv_nsec;
        }
    }
    //    ALOGD("TimeDifference time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, diffns=%lld" ,tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec,diffns);
    return diffns;
}

bool SPELayer::TimeCompare(struct timespec time1, struct timespec time2)
{
    bool bRet = 0;

    if (time1.tv_sec > time2.tv_sec)
    {
        bRet = true;
    }
    else if (time1.tv_sec == time2.tv_sec)
    {
        if (time1.tv_nsec >= time2.tv_nsec)
        {
            bRet = true;
        }
        else
        {
            bRet = false;
        }
    }
    else
    {
        bRet = false;
    }
    return bRet;
}

//endtime = false => compare the start time
//endtime = true => compare the end time
bool SPELayer::TimeStampCompare(BufferInfo *BufInfo1, BufferInfo *BufInfo2, bool Endtime)
{
    bool bRet = 0;
    struct timespec tstemp1 = BufInfo1->time_stamp_estimate;
    struct timespec tstemp2 = BufInfo2->time_stamp_estimate;
    struct timespec tstemp2_end = tstemp2;
    int inSample = BufInfo2->BufLen / 2; //mono data since BufInfo2 is downlink data

    ALOGD("TimeStampCompare time1 sec= %ld, nsec=%ld, time2 sec=%ld, nsec=%ld, Endtime=%d, inSample=%d" , tstemp1.tv_sec, tstemp1.tv_nsec, tstemp2.tv_sec, tstemp2.tv_nsec, Endtime, inSample);

    if (Endtime == 0)
    {
        if (tstemp1.tv_sec > tstemp2.tv_sec)
        {
            bRet = true;
        }
        else if (tstemp1.tv_sec == tstemp2.tv_sec)
        {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
            {
                bRet = true;
            }
            else
            {
                bRet = false;
            }
        }
        else
        {
            bRet = false;
        }
    }
    else
    {
        if (tstemp1.tv_sec > tstemp2.tv_sec)
        {
            bRet = true;
        }
        else if (tstemp1.tv_sec == tstemp2.tv_sec)
        {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
            {
                bRet = true;
            }
            else
            {
                bRet = false;
                return bRet;
            }
        }
        else
        {
            bRet = false;
            return bRet;
        }

        unsigned long long ns = ((inSample * 1000000) / 16); //sample rate is 16000

        if (tstemp2.tv_nsec + ns >= 1000000000)
        {
            tstemp2.tv_sec++;
            tstemp2.tv_nsec = tstemp2.tv_nsec + ns - 1000000000;
        }
        else
        {
            tstemp2.tv_nsec = tstemp2.tv_nsec + ns;
        }

        ALOGD("TimeStampCompare tstemp2 sec=%ld, nsec=%ld, ns=%lld" , tstemp2.tv_sec, tstemp2.tv_nsec, ns);

        if (tstemp1.tv_sec > tstemp2.tv_sec)
        {
            bRet = true;
        }
        else if (tstemp1.tv_sec == tstemp2.tv_sec)
        {
            if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
            {
                bRet = true;
            }
            else
            {
                bRet = false;
            }
        }
        else
        {
            bRet = false;
        }
    }

    ALOGD("TimeStampCompare bRet=%d", bRet);
    return bRet;
}

void SPELayer::BypassDLBuffer(void)
{
    BufferInfo *newInBuffer = new BufferInfo;
    struct timespec tstamp;
    int BufLength = mSPEProcessBufSize / 2;
    newInBuffer->pBufBase = (short *) malloc(BufLength);
    //ALOGD("PrepareProcessData %p", newInBuffer->pBufBase);
    //memset(newInBuffer->pBufBase, 0, BufLength);
    memset(newInBuffer->pBufBase, 0xEEEE, BufLength);

    tstamp = GetSystemTime();
    newInBuffer->BufLen = BufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    newInBuffer->time_stamp_queued = tstamp;
    newInBuffer->time_stamp_estimate = {0};
    newInBuffer->time_stamp_process = {0};
    newInBuffer->DLfirstBuf = false;

    //queue to the begging of the downlink input buffer queue, downlink data channel is mono
    //mDLInBufferQ.add(newInBuffer);
    mDLInBufferQ.push_front(newInBuffer);
    ALOGD("BypassDLBuffer, size %d, %p", mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
    //mDLInBufferQ.insertVectorAt(newInBuffer,0);
    mDLInBufQLenTotal += BufLength;
    //      ALOGD("CompensateBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


    newInBuffer->BufLen4Delay = BufLength;
    newInBuffer->pRead4Delay = newInBuffer->pBufBase;
    newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
    //mDLDelayBufferQ.add(newInBuffer);
    mDLDelayBufferQ.push_front(newInBuffer);
    mDLDelayBufQLenTotal += BufLength;
}

bool SPELayer::PrepareProcessData()
{
    bool bRet = false;
    if (mPrepareProcessDataReady)
    {
        bRet = true;
        return bRet;
    }

    ALOGD("PrepareProcessData");
    if (mDLNewStart || (DLdataPrepareCount > 0)) //the first queue downlink buffer is not ready to play yet, only need to process uplink data, so add zero data on downlink
    {
        //compensate data to DL and DL delay as zero data for first uplink buffer process
        ALOGD("PrepareProcessData, DL data is not ready yet, size %d, %p", mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);

        BypassDLBuffer();
        if (mDLNewStart)
        {
            CalPrepareCount(); // 2 * 4;
        }
        else
        {
            DLdataPrepareCount--;
            ALOGD("prepare data DLdataPrepareCount=%d put infront of", DLdataPrepareCount);
        }
        bRet = false;
    }
    else    //when all data is ready, check the estimate time to let DL/UL could start together.
    {
        if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty())
        {
            ALOGD("no downlink data, no need to sync");
            return bRet;
        }

        if (DLdataPrepareCount > 0)
        {
            DLdataPrepareCount--;
            ALOGD("prepare data DLdataPrepareCount=%d", DLdataPrepareCount);
            return bRet;
        }
#if 1
        if (TimeCompare(mDownlinkIntrStartTime, mULInBufferQ[0]->time_stamp_estimate))  //if downlink data is later than uplink data, no need to process AEC and not need to do the sync
        {
            //compensate data to DL and DL delay as zero data for first uplink buffer process
            ALOGD("PrepareProcessData, downlink data is not ready yet, no need to sync, size %d, %p", mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
            BypassDLBuffer();
            return bRet;
        }
#endif
        for (int i; i < mDLInBufferQ.size(); i++)
        {
            ALOGD("mDLInBufferQ i=%d, length=%d, %p, Sec=%d, NSec=%ld", i, mDLInBufferQ[i]->BufLen, mDLInBufferQ[i]->pBufBase,
                  mDLInBufferQ[i]->time_stamp_estimate.tv_sec, mDLInBufferQ[i]->time_stamp_estimate.tv_nsec);
        }

        bool bULlate = false;
        //struct timespec deltatime;
        int deltaSec = 0;
        unsigned long long deltaNSec = 0;
        ALOGD("PrepareProcessData time_stamp_estimate, mULInBufferQ[0].sec=%ld, mULInBufferQ[0].nsec=%ld", mULInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);
        ALOGD("PrepareProcessData time_stamp_estimate, mDLInBufferQ[0].sec=%ld, mDLInBufferQ[0].nsec=%ld", mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
        if (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 0))  //drop downlink and downlink delay data
        {
            //remove previous queue downlink data, to match the nearlist DL buffer timestamp as uplink one
            while ((!mDLInBufferQ.isEmpty()) && (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 1)))
            {
                //drop DL data
                uint32 droplength = mDLInBufferQ[0]->BufLen;
                mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                mDLInBufferQ.removeAt(0);

                //drop DL delay data
                while (droplength > 0)
                {
                    ALOGD("PrepareProcessData 1 drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                    if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                    {
                        ALOGD("PrepareProcessData 1 mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead);
                        mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                        mDLDelayBufQLenTotal -= droplength;
                        mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                        droplength = 0;
                        ALOGD("PrepareProcessData 1 after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                    }
                    else
                    {
                        droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                        mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    }
                }

                if (mDLInBufferQ.isEmpty())
                {
                    ALOGD("PrepareProcessData, something wrong? no DL buffer data");
                    break;
                }
            }

            if (TimeStampCompare(mULInBufferQ[0], mDLInBufferQ[0], 0))
            {
                ALOGD("PrepareProcessData, calculate drop downlink data time");
                bULlate = true; //calculate drop downlink data time
                if (mULInBufferQ[0]->time_stamp_estimate.tv_nsec >= mDLInBufferQ[0]->time_stamp_estimate.tv_nsec)
                {
                    deltaSec = mULInBufferQ[0]->time_stamp_estimate.tv_sec - mDLInBufferQ[0]->time_stamp_estimate.tv_sec;
                    deltaNSec = mULInBufferQ[0]->time_stamp_estimate.tv_nsec - mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
                else
                {
                    deltaSec = mULInBufferQ[0]->time_stamp_estimate.tv_sec - mDLInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                    deltaNSec = 1000000000 + mULInBufferQ[0]->time_stamp_estimate.tv_nsec - mDLInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
            }
            else
            {
                bULlate = false;
                ALOGD("PrepareProcessData actually uplink is earlier!!! need compensate downlink as zero");
                if (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec >= mULInBufferQ[0]->time_stamp_estimate.tv_nsec)
                {
                    deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec;
                    deltaNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
                else
                {
                    deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                    deltaNSec = 1000000000 + mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
                }
            }

        }
        else    //UL time is earlier
        {
            //need compensate downlink data as zero
            bULlate = false;
            ALOGD("PrepareProcessData 2 time_stamp_estimate,mDLInBufferQ[0].nsec = %ld, mULInBufferQ[0].nsec=%ld", mDLInBufferQ[0]->time_stamp_estimate.tv_nsec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);
            ALOGD("PrepareProcessData 2 time_stamp_estimate,mDLInBufferQ[0].sec = %ld, mULInBufferQ[0].sec=%ld", mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_sec);

            if (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec >= mULInBufferQ[0]->time_stamp_estimate.tv_nsec)
            {
                deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec;
                deltaNSec = mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
            }
            else
            {
                deltaSec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec - mULInBufferQ[0]->time_stamp_estimate.tv_sec - 1;
                deltaNSec = 1000000000 + mDLInBufferQ[0]->time_stamp_estimate.tv_nsec - mULInBufferQ[0]->time_stamp_estimate.tv_nsec;
            }

        }
        ALOGD("PrepareProcessData, bULlate %d, deltaSec=%d, deltaNSec=%lld", bULlate, deltaSec, deltaNSec);

        unsigned long long diffnsec = deltaSec * 1000000000 + deltaNSec;
        //uint32 diffSample = mVoIPSampleRate*diffnsec/1000000000;
        uint32 diffSample = 16 * diffnsec / 1000000;
        uint32 diffBufLength = diffSample * sizeof(short);

        ALOGD("PrepareProcessData, diffnsec %lld, diffSample=%ld, diffBufLength=%d", diffnsec, diffSample, diffBufLength);
#if 1
        while (diffBufLength > 0)
        {
            if (bULlate == true) //drop DL data and DL delay data
            {
                if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty())
                {
                    ALOGW("PrepareProcessData no mDLInBufferQ data");
                    break;
                }
                if ((diffBufLength > mDLInBufQLenTotal) || (diffBufLength > mDLDelayBufQLenTotal))
                {
                    //time diff more than DL preQ data
                    ALOGW("PrepareProcessData something wrong happened?");
                    diffBufLength = mDLInBufQLenTotal;
                    //break;
                }
                ALOGD("PrepareProcessData drop DL data diffBufLength=%d, mDLInBufferQ.size()=%d, mDLInBufferQ[0]->BufLen=%d!!!", diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
                if (diffBufLength >= mDLInBufferQ[0]->BufLen)
                {
                    //drop DL data
                    uint32 droplength = mDLInBufferQ[0]->BufLen;
                    diffBufLength -= mDLInBufferQ[0]->BufLen;
                    mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                    mDLInBufferQ.removeAt(0);

                    //drop DL delay data
                    while (droplength > 0)
                    {
                        ALOGD("PrepareProcessData drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                        if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                        {
                            ALOGD("PrepareProcessData mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead);
                            mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                            mDLDelayBufQLenTotal -= droplength;
                            mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                            droplength = 0;
                            ALOGD("PrepareProcessData after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                        }
                        else
                        {
                            droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        }
                    }
                }
                else
                {
                    //                    ALOGW("PrepareProcessData !!! break for test");
                    //                    break;
                    //drop DL data
                    ALOGD("PrepareProcessData mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                          mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                    uint32 droplength = diffBufLength;
                    mDLInBufferQ[0]->BufLen -= diffBufLength;   //record the buffer you consumed
                    mDLInBufQLenTotal -= diffBufLength;
                    mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffBufLength / 2;

                    //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

                    uint32 adjustsample = diffBufLength / 2;
                    unsigned long long updateDLnsecdiff = 0;
                    updateDLnsecdiff = (adjustsample * 1000000) / 16;

                    mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
                    mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

                    ALOGD("PrepareProcessData after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                          mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                    diffBufLength = 0;

                    //drop DL delay data
                    while (droplength > 0)
                    {
                        ALOGD("PrepareProcessData drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                        if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                        {
                            ALOGD("PrepareProcessData mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead4Delay);
                            mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                            mDLDelayBufQLenTotal -= droplength;
                            mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                            droplength = 0;
                            ALOGD("PrepareProcessData after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                        }
                        else
                        {
                            droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                            mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                            free(mDLDelayBufferQ[0]->pBufBase);
                            delete mDLDelayBufferQ[0];
                            mDLDelayBufferQ.removeAt(0);
                        }
                    }

                }
            }
            else    //add DL zero data at the beginning
            {
                BufferInfo *newInBuffer = new BufferInfo;
                struct timespec tstamp;
                int BufLength = diffBufLength;
                newInBuffer->pBufBase = (short *) malloc(BufLength);
                ALOGD("PrepareProcessData data is ready but need adjust");
                //memset(newInBuffer->pBufBase, 0, BufLength);
                memset(newInBuffer->pBufBase, 0xEEEE, BufLength);

                tstamp = GetSystemTime();
                newInBuffer->BufLen = BufLength;
                newInBuffer->pRead = newInBuffer->pBufBase;
                newInBuffer->pWrite = newInBuffer->pBufBase;
                newInBuffer->time_stamp_queued = tstamp;
                newInBuffer->time_stamp_estimate = tstamp;
                newInBuffer->time_stamp_process = {0};

                //queue to the begging of the downlink input buffer queue, downlink data channel is mono
                //mDLInBufferQ.add(newInBuffer);
                mDLInBufferQ.push_front(newInBuffer);
                //ALOGD("PrepareProcessData, size %d, %p",mDLInBufferQ.size(), mDLInBufferQ[0]->pBufBase);
                //mDLInBufferQ.insertVectorAt(newInBuffer,0);
                mDLInBufQLenTotal += BufLength;
                //      ALOGD("CompensateBuffer, mDLInBufQLenTotal=%d, size=%d",mDLInBufQLenTotal,mDLInBufferQ.size());


                newInBuffer->BufLen4Delay = BufLength;
                newInBuffer->pRead4Delay = newInBuffer->pBufBase;
                newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
                //mDLDelayBufferQ.add(newInBuffer);
                mDLDelayBufferQ.push_front(newInBuffer);
                mDLDelayBufQLenTotal += BufLength;
                diffBufLength = 0;
            }
        }
#endif
        ALOGD("PrepareProcessData finish, mDLInBufferQ.size = %d, mDLInBufQLenTotal=%d, mULInBufferQ.size = %d, mULInBufQLenTotal=%d", mDLInBufferQ.size(), mDLInBufQLenTotal, mULInBufferQ.size(),
              mULInBufQLenTotal); //going to start process
        ALOGD("PrepareProcessData finish, mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec = %ld, nsec = %ld"
              , mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);

        ALOGD("PrepareProcessData finish, mULInBufferQ[0]->pRead=%p , mULInBufferQ[0]->BufLen=%d, sec = %ld, nsec = %ld"
              , mULInBufferQ[0]->pRead, mULInBufferQ[0]->BufLen, mULInBufferQ[0]->time_stamp_estimate.tv_sec, mULInBufferQ[0]->time_stamp_estimate.tv_nsec);

        mPrepareProcessDataReady = true;
        bRet = true;

        if (mNeedJitterBuffer && (mJitterSampleCount != 0)) //the first DL buffer, add the jitter buffer at the first downlink buffer queue and downlink delay buffer queue
        {
            mNeedJitterBuffer = false;
            BufferInfo *newJitterBuffer = new BufferInfo;
            newJitterBuffer->pBufBase = (short *) malloc(mJitterSampleCount * sizeof(short)); //one channel, 16bits
            newJitterBuffer->BufLen = mJitterSampleCount * sizeof(short);
            newJitterBuffer->pRead = newJitterBuffer->pBufBase;
            newJitterBuffer->pWrite = newJitterBuffer->pBufBase;
            newJitterBuffer->BufLen4Delay = mJitterSampleCount * sizeof(short);
            newJitterBuffer->pRead4Delay = newJitterBuffer->pBufBase;
            newJitterBuffer->pWrite4Delay = newJitterBuffer->pBufBase;
            memset(newJitterBuffer->pBufBase, 0, newJitterBuffer->BufLen);
            //newJitterBuffer->time_stamp_queued = tstamp;
            newJitterBuffer->time_stamp_process = {0};
            mDLInBufferQ.push_front(newJitterBuffer);
            mDLInBufQLenTotal += newJitterBuffer->BufLen;

            mDLDelayBufferQ.push_front(newJitterBuffer);
            mDLDelayBufQLenTotal += newJitterBuffer->BufLen;
            ALOGD("add jitter buffer,newDelayBuffer->BufLen=%d, size=%d, mJitterSampleCount=%d, pBufBase=%p", newJitterBuffer->BufLen, mDLInBufferQ.size(), mJitterSampleCount, newJitterBuffer->pBufBase);
        }

        if (mNeedDelayLatency && (mLatencySampleCount != 0)) //the first DL buffer, add the delay time buffer as first delay buffer queue
        {
            mNeedDelayLatency = false;
            ALOGD("PrepareProcessData adjust downlink data mLatencyDir=%d,mLatencySampleCount=%d", mLatencyDir, mLatencySampleCount);
            if (mLatencyDir == true)
            {
                BufferInfo *newDelayBuffer = new BufferInfo;
                newDelayBuffer->pBufBase = (short *) malloc(mLatencySampleCount * sizeof(short)); //one channel, 16bits
                newDelayBuffer->BufLen = mLatencySampleCount * sizeof(short);
                newDelayBuffer->pRead = newDelayBuffer->pBufBase;
                newDelayBuffer->pWrite = newDelayBuffer->pBufBase;
                newDelayBuffer->BufLen4Delay = mLatencySampleCount * sizeof(short);
                newDelayBuffer->pRead4Delay = newDelayBuffer->pBufBase;
                newDelayBuffer->pWrite4Delay = newDelayBuffer->pBufBase;
                memset(newDelayBuffer->pBufBase, 0, newDelayBuffer->BufLen);
                //newDelayBuffer->time_stamp_queued = tstamp;
                newDelayBuffer->time_stamp_process = {0};
                mDLDelayBufferQ.push_front(newDelayBuffer);
                mDLDelayBufQLenTotal += newDelayBuffer->BufLen;
                ALOGD("add delay latency buffer, newDelayBuffer->BufLen=%d, size=%d, mLatencySampleCount=%d, pBufBase=%p", newDelayBuffer->BufLen, mDLDelayBufferQ.size(), mLatencySampleCount,
                      newDelayBuffer->pBufBase);
            }
            else
            {
                uint32 diffLatencyBufLength = mLatencySampleCount * sizeof(short);
                while (diffLatencyBufLength > 0)
                {
                    if (mDLInBufferQ.isEmpty() || mDLDelayBufferQ.isEmpty())
                    {
                        ALOGW("adjust downlink data no mDLInBufferQ data");
                        break;
                    }
                    if ((diffLatencyBufLength > mDLInBufQLenTotal) || (diffLatencyBufLength > mDLDelayBufQLenTotal))
                    {
                        //time diff more than DL preQ data
                        ALOGW("adjust downlink data something wrong happened?");
                        diffLatencyBufLength = mDLInBufQLenTotal;
                        //break;
                    }
                    ALOGD("adjust downlink data drop DL data diffBufLength=%d, mDLInBufferQ.size()=%d, mDLInBufferQ[0]->BufLen=%d!!!", diffBufLength, mDLInBufferQ.size(), mDLInBufferQ[0]->BufLen);
                    if (diffLatencyBufLength >= mDLInBufferQ[0]->BufLen)
                    {
                        //drop DL data
                        uint32 droplength = mDLInBufferQ[0]->BufLen;
                        diffLatencyBufLength -= mDLInBufferQ[0]->BufLen;
                        mDLInBufQLenTotal -= mDLInBufferQ[0]->BufLen;
                        mDLInBufferQ.removeAt(0);

                        //drop DL delay data
                        while (droplength > 0)
                        {
                            ALOGD("adjust downlink data drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                            if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                            {
                                ALOGD("adjust downlink data mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead);
                                mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                                mDLDelayBufQLenTotal -= droplength;
                                mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                                droplength = 0;
                                ALOGD("adjust downlink data after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                            }
                            else
                            {
                                droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                                mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                                free(mDLDelayBufferQ[0]->pBufBase);
                                delete mDLDelayBufferQ[0];
                                mDLDelayBufferQ.removeAt(0);
                            }
                        }
                    }
                    else
                    {

                        //drop DL data
                        ALOGD("adjust downlink data mDLInBufferQ[0]->pRead=%p , mDLInBufferQ[0]->BufLen=%d, sec %ld, nsec %ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, mDLInBufferQ[0]->time_stamp_estimate.tv_sec,
                              mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                        uint32 droplength = diffLatencyBufLength;
                        mDLInBufferQ[0]->BufLen -= diffLatencyBufLength;   //record the buffer you consumed
                        mDLInBufQLenTotal -= diffLatencyBufLength;
                        mDLInBufferQ[0]->pRead = mDLInBufferQ[0]->pRead + diffLatencyBufLength / 2;

                        //unsigned long long updateDLnsecdiff = (diffBufLength/2)*1000000000/mVoIPSampleRate;

                        uint32 adjustsample = diffLatencyBufLength / 2;
                        unsigned long long updateDLnsecdiff = 0;
                        updateDLnsecdiff = (adjustsample * 1000000) / 16;

                        mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) / 1000000000;
                        mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + updateDLnsecdiff) % 1000000000;

                        ALOGD("adjust downlink data after mDLInBufferQ[0]->pRead=%p, mDLInBufferQ[0]->BufLen=%d, updatensecdiff=%lld, sec=%ld, nsec=%ld", mDLInBufferQ[0]->pRead, mDLInBufferQ[0]->BufLen, updateDLnsecdiff,
                              mDLInBufferQ[0]->time_stamp_estimate.tv_sec, mDLInBufferQ[0]->time_stamp_estimate.tv_nsec);
                        diffLatencyBufLength = 0;

                        //drop DL delay data
                        while (droplength > 0)
                        {
                            ALOGD("adjust downlink data drop DL Delay data droplength=%d, mDLDelayBufferQ.size()=%d, mDLDelayBufferQ[0]->BufLen4Delay=%d!!!", droplength, mDLDelayBufferQ.size(), mDLDelayBufferQ[0]->BufLen4Delay);
                            if (droplength < mDLDelayBufferQ[0]->BufLen4Delay)
                            {
                                ALOGD("adjust downlink data mDLDelayBufferQ[0]->pRead=%p", mDLDelayBufferQ[0]->pRead4Delay);
                                mDLDelayBufferQ[0]->BufLen4Delay -= droplength;
                                mDLDelayBufQLenTotal -= droplength;
                                mDLDelayBufferQ[0]->pRead4Delay = mDLDelayBufferQ[0]->pRead4Delay + droplength / 2;
                                droplength = 0;
                                ALOGD("adjust downlink data after mDLDelayBufferQ[0]->pRead=%p, mDLDelayBufferQ[0]->BufLen=%d", mDLDelayBufferQ[0]->pRead4Delay, mDLDelayBufferQ[0]->BufLen4Delay);
                            }
                            else
                            {
                                droplength -= mDLDelayBufferQ[0]->BufLen4Delay;
                                mDLDelayBufQLenTotal -= mDLDelayBufferQ[0]->BufLen4Delay;
                                free(mDLDelayBufferQ[0]->pBufBase);
                                delete mDLDelayBufferQ[0];
                                mDLDelayBufferQ.removeAt(0);
                            }
                        }

                    }
                }
            }
        }


        if (0) //(mDLlateAdjust)
        {
            mDLlateAdjust = false;
            int DLAdjustSampleCount = 0;
            DLAdjustSampleCount = (mDLLatencyTime / 2) * mVoIPSampleRate / 1000;
            ALOGD("adjust downlink data start point mDLLatencyTime=%d,DLAdjustSampleCount=%d", mDLLatencyTime, DLAdjustSampleCount);

            BufferInfo *newAdjustBuffer = new BufferInfo;
            newAdjustBuffer->pBufBase = (short *) malloc(DLAdjustSampleCount * sizeof(short)); //one channel, 16bits
            newAdjustBuffer->BufLen = DLAdjustSampleCount * sizeof(short);
            newAdjustBuffer->pRead = newAdjustBuffer->pBufBase;
            newAdjustBuffer->pWrite = newAdjustBuffer->pBufBase;
            newAdjustBuffer->BufLen4Delay = DLAdjustSampleCount * sizeof(short);
            newAdjustBuffer->pRead4Delay = newAdjustBuffer->pBufBase;
            newAdjustBuffer->pWrite4Delay = newAdjustBuffer->pBufBase;
            //memset(newAdjustBuffer->pBufBase,0,newAdjustBuffer->BufLen);
            memset(newAdjustBuffer->pBufBase, 0xEEEE, newAdjustBuffer->BufLen);
            //newDelayBuffer->time_stamp_queued = tstamp;
            newAdjustBuffer->time_stamp_process = {0};

            mDLInBufferQ.push_front(newAdjustBuffer);
            mDLInBufQLenTotal += newAdjustBuffer->BufLen;

            mDLDelayBufferQ.push_front(newAdjustBuffer);
            mDLDelayBufQLenTotal += newAdjustBuffer->BufLen;
        }

#if 0
        for (int i; i < mDLDelayBufferQ.size(); i++)
        {
            ALOGD("PrepareProcessData mDLDelayBufferQ i=%d, length=%d, %p", i, mDLDelayBufferQ[i]->BufLen, mDLDelayBufferQ[i]->pBufBase);
        }
#endif
    }
    return bRet;
}


bool SPELayer::Process_Record(short *inBuf, int  inBufLength)
{
    //pthread_mutex_lock(&mBufMutex );
    //ALOGD("Process_Record, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());
    if ((mULInBufQLenTotal < mSPEProcessBufSize) && (mULOutBufferQ.size() == 0)) //not enough UL buffer for process, and no processed uplink output buffer
    {
        ALOGD("SPELayer::Process,going memset 0 inBuf=%p,inBufLength=%d", inBuf, inBufLength);
        memset(inBuf, 0, inBufLength); //return in same input buffer address
        //pthread_mutex_unlock(&mBufMutex);
        return true;
    }

    //   ALOGD("SPELayer::Process, enough mULInBufQLenTotal buffer,size=%d",mULInBufferQ.size());
    //process input data in the buffer queue
    while (mULInBufQLenTotal >= mSPEProcessBufSize)
    {
        int tmpSPEProcessBufSize = mSPEProcessBufSize;
        int indexIn = 0;
        int tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;

        //      ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, SPERecBufSize=%d,tempULIncopysize=%d",mULInBufQLenTotal,mSPEProcessBufSize,tempULIncopysize);
        //fill in the data to the process buffer
        while (tmpSPEProcessBufSize)
        {
            if (mULInBufferQ.isEmpty())
            {
                ALOGD("uplink input buffer queue is empty, something wrong!!");
                mError = true;
                break;
            }

            //        ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,tempULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,tempULIncopysize);
            if (tempULIncopysize > 0) //get the buffer data from the first uplink input buffer queue
            {
                *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);    //left channel
                *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead + 1); //right channel
                //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
                mULInBufferQ[0]->pRead += 2;
                tempULIncopysize--;
                indexIn++;
                tmpSPEProcessBufSize -= 4;
                mULInBufQLenTotal -= 4; //int and short transform
                mULInBufferQ[0]->BufLen -= 4; //record the buffer you consumed
            }
            else    //consume all the data in first queue buffer
            {
                free(mULInBufferQ[0]->pBufBase);
                delete mULInBufferQ[0];
                mULInBufferQ.removeAt(0);
                tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;
                //                ALOGD("UL in buffer consume finish, next BufferBase=%p",mULInBufferQ[0]->pBufBase);
            }
        }

        if (mError)
        {
            ALOGD("error!!");
            break;
        }

        //process the fill in buffer
        ENH_API_Process(&mSph_Enh_ctrl);

        Dump_EPL(&mSph_Enh_ctrl.EPL_buffer, EPLBufSize * sizeof(short));
        EPLTransVMDump();

        BufferInfo *newOutBuffer = new BufferInfo;

        newOutBuffer->pBufBase = (short *) malloc(mSPEProcessBufSize);
        newOutBuffer->BufLen = mSPEProcessBufSize;

        newOutBuffer->pRead = newOutBuffer->pBufBase;
        newOutBuffer->pWrite = newOutBuffer->pBufBase;
        //        ALOGD("newOutBuffer->pBufBase=%p,newOutBuffer->pRead=%p,newOutBuffer->pWrite=%p,newOutBuffer->BufLen=%d",newOutBuffer->pBufBase,newOutBuffer->pRead,newOutBuffer->pWrite,newOutBuffer->BufLen);

        int indexOut = 0;
        int copysize = newOutBuffer->BufLen >> 2;
        while (copysize)
        {
            //            ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
            *(newOutBuffer->pWrite) = *(mpSPEBufferUL1 + indexOut); //left channel
            *(newOutBuffer->pWrite + 1) = *(mpSPEBufferUL2 + indexOut); //right channel
            newOutBuffer->pWrite += 2;
            indexOut++;
            copysize--;
        }

        Dump_PCM_Process(UPLINK, newOutBuffer->pBufBase, newOutBuffer->BufLen);

        mULOutBufferQ.add(newOutBuffer);
        mULOutBufQLenTotal += newOutBuffer->BufLen;

        //        ALOGD("mULOutBufQLenTotal=%d, indexOut=%d,newOutBuffer->pWrite=%p, mULOutBufferQsize=%d",mULOutBufQLenTotal,indexOut,newOutBuffer->pWrite,mULOutBufferQ.size());
    }


    //process the processed output buffer queue

    //    ALOGD("mULOutBufferQ=%d, mULOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal);
    if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength)
    {
        ALOGD("SPELayer not enought UL output buffer");
        memset(inBuf, 0, inBufLength); //return in same input buffer address
        //pthread_mutex_unlock(&mBufMutex);
        return true;
    }
    int tmpInBufLength = inBufLength;
    int count = 0;
    int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
    while (tmpInBufLength)
    {
        if (mULOutBufferQ.isEmpty())
        {
            ALOGD("SPELayer::uplink Output buffer queue is empty, something wrong!!");
            mError = true;
            break;
        }

        //        ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

        if (tempULCopy > 0) //get the buffer data from the first uplink input buffer queue
        {
            //            ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
            //            ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
            *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
            *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
            mULOutBufferQ[0]->pRead += 2;
            tmpInBufLength -= 4; //int and short transform
            tempULCopy--;
            count += 2;
            mULOutBufQLenTotal -= 4; //int and short transform
            mULOutBufferQ[0]->BufLen -= 4;
        }
        else    //consume all the data in first queue buffer
        {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
            tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
            //            ALOGD("SPELayer::uplink Output buffer consumed");
        }
    }
    //pthread_mutex_unlock(&mBufMutex);
    return true;
}


bool SPELayer::Process_VoIP(short *inBuf, int  inBufLength)
{
    //    ALOGD("SPELayer::process VoIP");
    //pthread_mutex_lock(&mBufMutex );
    //       ALOGD("SPELayer::Process, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());

    if (mULInBufQLenTotal < mSPEProcessBufSize) //not enough UL input buffer for process
    {
        int tmpInBufLength = inBufLength;
#if 0
        if (mULOutBufQLenTotal < inBufLength) //not enough UL output buffer, return 0 data, fixme!! done
#else
        if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength) //fixme, return data we have?
#endif
        {
            ALOGD("not enough UL output buffer, inBuf=%p,inBufLength=%d", inBuf, inBufLength);
            memset(inBuf, 0, inBufLength); //reset data
            tmpInBufLength = mULOutBufQLenTotal;
            //pthread_mutex_unlock(&mBufMutex);
            return true;
        }


        int count = 0;
        int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
        while (tmpInBufLength)
        {
            if (mULOutBufferQ.isEmpty())
            {
                ALOGD("Process_VoIP Output buffer queue is empty, return size mULOutBufQLenTotal");
                break;
            }

            //                ALOGD("mDLOutBufferQ.size = %d,tempDLCopy=%d",mDLOutBufferQ.size(),tempDLCopy);

            if (tempULCopy > 0) //get the buffer data from the first downlink input buffer queue
            {
                //                       ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
                //                       ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
                mULOutBufferQ[0]->pRead += 2;
                tmpInBufLength -= 4; //int and short transform
                tempULCopy--;
                count += 2;
                mULOutBufQLenTotal -= 4; //int and short transform
                mULOutBufferQ[0]->BufLen -= 4;
            }
            else    //consume all the data in first queue buffer
            {
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                //fixme done: need check if still have ULOutbuffer
                if (!mULOutBufferQ.isEmpty())
                {
                    ALOGD("Process_VoIP mULOutBufferQ not empty, get next one, size=%d", mULOutBufferQ.size());
                    tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
                }
                else
                {
                    ALOGD("Process_VoIP mULOutBufferQ is empty!!! size=%d", mULOutBufferQ.size());
                }
                //                  ALOGD("SPELayer::uplink Output buffer consumed a");
            }
        }
        ALOGD("Process_VoIP has UL Output buffer but not enough UL Input buffer");
        //pthread_mutex_unlock(&mBufMutex);
        return true;
    }

    //fix me!!: process when UL data is enough, DL data need to compensated as zero
    //processing if have enough input UL data (UL is stereo, DL is mono data)
    if (mULInBufQLenTotal >= mSPEProcessBufSize)      //&&(mDLInBufQLenTotal>= mSPEProcessBufSize/2))
    {
        while (mULInBufQLenTotal >= mSPEProcessBufSize) //&&(mDLInBufQLenTotal>= mSPEProcessBufSize/2))  //TODO:fixme!!! has problem
        {

            //AdjustDLDelayData();

            if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) //not enough downlink data to process, wait for a while
            {
                if (mULOutBufQLenTotal >= inBufLength)
                {
                    ALOGD("Process_VoIP have enough uplink processed data, skip this time");
                    break;
                }
                //WaitforDownlinkData();
            }

            if (PrepareProcessData())   //sync ok, could start process
            {
                //AdjustDLDelayData();
                if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) //not enough downlink data to process, wait for a while
                {
                    if (WaitforDownlinkData())  //got new DL data queue
                    {
                        if (mDLInBufQLenTotal < mSPEProcessBufSize / 2) //but still not enough data to process
                        {
                            ALOGD("got new DL buffer, but still not enough data to process");
                            continue;
                        }
                    }
                    else    //no new DL data queue
                    {
                        ALOGD("no new DL buffer queue, process directly");
                    }
                }
                else    //has enough downlink data to  process, but still check if there has downlink buffer queue wait
                {
                    if (mNewReferenceBufferComes)   //if there is new DL buffer comes, let it add the queue first (To not block the downlink process)
                    {
                        InsertDownlinkData();
                    }
                }
            }
            else    //no sync yet, no need to check or wait for downlink data
            {
                if (mNewReferenceBufferComes)   //if there is new DL buffer comes, let it add the queue first (To not block the downlink process)
                {
                    ALOGD("also check if new downlink data comes even the sync is not ready");
                    InsertDownlinkData();
                }
            }

            //fill in the data to process buffer
            int tmpSPEProcessBufSize = mSPEProcessBufSize;
            int indexIn = 0;
            int ULIncopysize = mULInBufferQ[0]->BufLen >> 2;

            struct timespec tstamp_process;
            struct timespec DLtstamp_compen;

            if (mDLInBufferQ.isEmpty()) //there is no DL data before process, compensate time use the uplink time, else use the DL previous time
            {
                DLtstamp_compen = mULInBufferQ[0]->time_stamp_estimate;
            }

            ALOGD("SPELayer::Process_VoIP, mULInBufQLenTotal=%d, mDLInBufQLenTotal=%d, SPERecBufSize=%d,ULIncopysize=%d", mULInBufQLenTotal, mDLInBufQLenTotal, mSPEProcessBufSize, ULIncopysize);

            //if(!mDLDelayBufferQ.isEmpty())
            //ALOGD("SPELayer::Process_VoIP, mDLDelayBufferQ size=%d,mDLDelayBufQLenTotal=%d, SPERecBufSize=%d, %p",mDLDelayBufferQ.size(),mDLDelayBufQLenTotal,mSPEProcessBufSize,mDLDelayBufferQ[0]->pBufBase);
            //if(!mDLInBufferQ.isEmpty())
            //ALOGD("SPELayer::Process_VoIP, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,ULIncopysize=%d, %p",mDLInBufferQ.size(),mDLInBufQLenTotal,ULIncopysize,mDLInBufferQ[0]->pBufBase);

            while (tmpSPEProcessBufSize)
            {
                if (mULInBufferQ.isEmpty()) //||mDLInBufferQ.isEmpty()||mDLDelayBufferQ.isEmpty())
                {
                    ALOGD("SPELayer::input buffer queue is empty, something wrong!!");
                    mError = true;
                    break;
                }

                tstamp_process = GetSystemTime();

                //                      ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,ULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,ULIncopysize);
                if (ULIncopysize > 0) //get the buffer data from the first uplink input buffer queue
                {
                    //fill in uplink data
                    *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);
                    *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead + 1);
                    mULInBufferQ[0]->pRead += 2;
                    mULInBufQLenTotal -= 4; //int and short transform
                    mULInBufferQ[0]->BufLen -= 4; //record the buffer you consumed

                    mULInBufferQ[0]->time_stamp_process = tstamp_process;


#if 1   //INTR
                    //update estimate time, when use the corresponding time
                    mULInBufferQ[0]->time_stamp_estimate.tv_sec = mULInBufferQ[0]->time_stamp_estimate.tv_sec + (mULInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) / 1000000000;
                    mULInBufferQ[0]->time_stamp_estimate.tv_nsec = (mULInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) % 1000000000;
#endif
                    //fill in downlink data
                    if (mDLInBufferQ.isEmpty())
                    {
                        ALOGD("no DL buffer, need compensate 1, tmpSPEProcessBufSize=%d", tmpSPEProcessBufSize);
                        CompensateBuffer(tmpSPEProcessBufSize / 2, DLtstamp_compen);
                    }
                    if (mDLInBufferQ[0]->BufLen <= 0) //run out of DL queue0 buffer
                    {
                        //not to free the buffer here due to the data still queue in the DLDelay buffer
                        //free(mDLInBufferQ[0]->pBufBase);  //just remove the queue but not delete buffer since it also queue in the delay queue
                        //delete mDLInBufferQ[0];
                        mDLInBufferQ.removeAt(0);
                        //                            ALOGD("get next DLInBufferQ, size=%d, mDLInBufQLenTotal=%d",mDLInBufferQ.size(),mDLInBufQLenTotal);
                        if (mDLInBufferQ.isEmpty())
                        {
                            ALOGD("no DL buffer, need compensate, tmpSPEProcessBufSize=%d", tmpSPEProcessBufSize);
#if 1//def DOWNLINK_MONO                            
                            CompensateBuffer(tmpSPEProcessBufSize / 2, DLtstamp_compen);
#else
                            CompensateBuffer(tmpSPEProcessBufSize);
#endif
                        }
                        //                            ALOGD("DL in buffer consume finish, next BufferBase=%p",mDLInBufferQ[0]->pBufBase);
                    }

                    mDLInBufferQ[0]->time_stamp_process = tstamp_process;
#if 1//def DOWNLINK_MONO                     
                    //*(mpSPEBufferDL + indexIn) = (*(mDLInBufferQ[0]->pRead)>>1) + (*(mDLInBufferQ[0]->pRead+1)>>1); //only mono data
                    *(mpSPEBufferDL + indexIn) = *(mDLInBufferQ[0]->pRead); //already mono data
                    mDLInBufferQ[0]->pRead++;
                    mDLInBufQLenTotal -= 2; //int and short transform
                    mDLInBufferQ[0]->BufLen -= 2; //record the buffer you consumed


                    mDLInBufferQ[0]->time_stamp_estimate.tv_sec = mDLInBufferQ[0]->time_stamp_estimate.tv_sec + (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) / 1000000000;
                    mDLInBufferQ[0]->time_stamp_estimate.tv_nsec = (mDLInBufferQ[0]->time_stamp_estimate.tv_nsec + mNsecPerSample) % 1000000000;
                    DLtstamp_compen = mDLInBufferQ[0]->time_stamp_estimate;

                    //check to remove the first DL buffer to avoid compare the wrong estimate time next time if the buffer is compensated buffer
                    if (mDLInBufferQ[0]->BufLen <= 0) //run out of DL queue0 buffer
                    {
                        //not to free the buffer here due to the data still queue in the DLDelay buffer
                        mDLInBufferQ.removeAt(0);
                    }
#else
                    *(mpSPEBufferDL + indexIn) = (*(mDLInBufferQ[0]->pRead) >> 1) + (*(mDLInBufferQ[0]->pRead + 1) >> 1); //only mono data
                    mDLInBufferQ[0]->pRead += 2;
                    mDLInBufQLenTotal -= 4; //int and short transform
                    mDLInBufferQ[0]->BufLen -= 4; //record the buffer you consumed
#endif

                    //fill in delay latency data
                    if (mDLDelayBufferQ[0]->BufLen4Delay <= 0) //run out of DL  delay queue0 buffer
                    {
                        //ALOGD("DL delay consume");
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                        if (mDLDelayBufferQ.isEmpty())
                        {
                            ALOGD("no DL delay buffer, should already compensate something wrong");
                            mError = true;
                            break;
                        }
                        //                            ALOGD("DL delay in buffer consume finish, next BufferBase=%p, size=%d",mDLDelayBufferQ[0]->pBufBase,mDLDelayBufferQ.size());
                    }
                    mDLDelayBufferQ[0]->time_stamp_process = tstamp_process;
#if 1//def DOWNLINK_MONO 
                    //*(mpSPEBufferDLDelay + indexIn) = (*(mDLDelayBufferQ[0]->pRead4Delay)>>1) + (*(mDLDelayBufferQ[0]->pRead4Delay+1)>>1); //only mono data
                    *(mpSPEBufferDLDelay + indexIn) = *(mDLDelayBufferQ[0]->pRead4Delay); //already mono data
                    mDLDelayBufferQ[0]->pRead4Delay++;
                    mDLDelayBufQLenTotal -= 2; //int and short transform
                    mDLDelayBufferQ[0]->BufLen4Delay -= 2; //record the buffer you consumed
                    //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
#else
                    *(mpSPEBufferDLDelay + indexIn) = (*(mDLDelayBufferQ[0]->pRead4Delay) >> 1) + (*(mDLDelayBufferQ[0]->pRead4Delay + 1) >> 1); //only mono data
                    mDLDelayBufferQ[0]->pRead4Delay += 2;
                    mDLDelayBufQLenTotal -= 4; //int and short transform
                    mDLDelayBufferQ[0]->BufLen4Delay -= 4; //record the buffer you consumed
#endif

                    if (mDLDelayBufferQ[0]->BufLen4Delay <= 0) //run out of DL  delay queue0 buffer
                    {
                        free(mDLDelayBufferQ[0]->pBufBase);
                        delete mDLDelayBufferQ[0];
                        mDLDelayBufferQ.removeAt(0);
                    }

                    ULIncopysize--;
                    indexIn++;
                    tmpSPEProcessBufSize -= 4;

                }
                else    //consume all the data in first queue buffer
                {
                    free(mULInBufferQ[0]->pBufBase);
                    delete mULInBufferQ[0];
                    mULInBufferQ.removeAt(0);
                    ULIncopysize = mULInBufferQ[0]->BufLen >> 2;
                    //ALOGD("UL in buffer consume finish, next BufferBase=%p, size=%d,mULInBufQLenTotal=%d",mULInBufferQ[0]->pBufBase,mULInBufferQ.size(),mULInBufQLenTotal);
                }
            }

            if (mError)
            {
                ALOGE("error happened!!");
                break;
            }

            //after fill buffer, process
            ENH_API_Process(&mSph_Enh_ctrl);

            Dump_EPL(&mSph_Enh_ctrl.EPL_buffer, EPLBufSize * sizeof(short));
            EPLTransVMDump();

            //record to the outputbuffer queue, no need processed downlink data

            //BufferInfo *newDLOutBuffer = new BufferInfo;
            BufferInfo *newULOutBuffer = new BufferInfo;

            //newDLOutBuffer->pBufBase = (short*) malloc(mSPEProcessBufSize/2);
            //newDLOutBuffer->BufLen= mSPEProcessBufSize/2;

            //newDLOutBuffer->pRead = newDLOutBuffer->pBufBase;
            //newDLOutBuffer->pWrite= newDLOutBuffer->pBufBase;

            newULOutBuffer->pBufBase = (short *) malloc(mSPEProcessBufSize);
            newULOutBuffer->BufLen = mSPEProcessBufSize;

            newULOutBuffer->pRead = newULOutBuffer->pBufBase;
            newULOutBuffer->pWrite = newULOutBuffer->pBufBase;
            //                ALOGD("newDLOutBuffer->pBufBase=%p,newDLOutBuffer->pRead=%p,newDLOutBuffer->pWrite=%p,newDLOutBuffer->BufLen=%d",newDLOutBuffer->pBufBase,newDLOutBuffer->pRead,newDLOutBuffer->pWrite,newDLOutBuffer->BufLen);
            int indexOut = 0;

            int copysizetest = newULOutBuffer->BufLen >> 2;
            while (copysizetest)
            {
                //ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
                //*(newDLOutBuffer->pWrite) = *(mpSPEBufferFE + indexOut);
                //*(newDLOutBuffer->pWrite+1) = *(mpSPEBufferFE + indexOut);

                *(newULOutBuffer->pWrite) = *(mpSPEBufferNE + indexOut);
                *(newULOutBuffer->pWrite + 1) = *(mpSPEBufferNE + indexOut);

                //                      ALOGD("indexOut=%d,mpSPEBufferFE =%d, mpSPEBufferNE=%d",indexOut,*(mpSPEBufferFE + indexOut),*(mpSPEBufferNE + indexOut));
                //newDLOutBuffer->pWrite+=2;
                newULOutBuffer->pWrite += 2;
                indexOut++;
                copysizetest--;
            }

            //mDLOutBufferQ.add(newDLOutBuffer);
            //mDLOutBufQLenTotal += newDLOutBuffer->BufLen;
            //                  ALOGD("queue to DLOut mDLOutBufQLenTotal=%d, size=%d",mDLOutBufQLenTotal,mDLOutBufferQ.size());
            //Dump_PCM_Process(DOWNLINK,newDLOutBuffer->pBufBase,newDLOutBuffer->BufLen);
            mULOutBufferQ.add(newULOutBuffer);
            mULOutBufQLenTotal += newULOutBuffer->BufLen;
            //            Dump_PCM_Process(UPLINK,newULOutBuffer->pBufBase,newULOutBuffer->BufLen);
        }
        //            ALOGD("return SPELayer::Process, mDLInBufferQ size=%d,mDLInBufQLenTotal=%d,mDLInBufferQ[0]->BufLen=%d",mDLInBufferQ.size(),mDLInBufQLenTotal,mDLInBufferQ[0]->BufLen);

    }
    else    //not enough UL  data, not process
    {
        ALOGD("not enough uplink data, not process");
    }

    //process the uplink output processed buffer queue
    memset(inBuf, 0, inBufLength); //clean the buffer will be used
#if 0
    if (mULOutBufferQ.isEmpty())  //fixme, return data we have?
#else
    if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength) //fixme, return data we have?
#endif
    {
        ALOGD("SPELayer not enought UL output buffer return size");
        //pthread_mutex_unlock(&mBufMutex);
        return true;
    }
    else
    {
        int tmpInBufLength = inBufLength;
        if (mULOutBufQLenTotal < inBufLength)
        {
            ALOGD("Process_VoIP mULOutBufQLenTotal<inBufLength");
            tmpInBufLength = mULOutBufQLenTotal;
        }
        int count = 0;
        int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
        while (tmpInBufLength)
        {
            if (mULOutBufferQ.isEmpty())
            {
                ALOGD("Process_VoIP run out of  output buffer queue");
                break;
            }

            //   ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

            if (tempULCopy > 0) //get the buffer data from the first uplink input buffer queue
            {
                //            ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
                //            ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
                mULOutBufferQ[0]->pRead += 2;
                tmpInBufLength -= 4; //int and short transform
                tempULCopy--;
                count += 2;
                mULOutBufQLenTotal -= 4; //int and short transform
                mULOutBufferQ[0]->BufLen -= 4;
            }
            else    //consume all the data in first queue buffer
            {
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                if (!mULOutBufferQ.isEmpty())
                {
                    //ALOGD("Process_VoIP mULOutBufferQ not empty, get next one 2, size=%d",mULOutBufferQ.size());
                    tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
                }
                else
                {
                    ALOGD("Process_VoIP mULOutBufferQ empty no more data 2, size=%d", mULOutBufferQ.size());
                }
                //                      ALOGD("SPELayer::uplink Output buffer consumed");
            }
        }
    }

    //pthread_mutex_unlock(&mBufMutex);
    return true;
}

//normal record + VoIP, new interface
bool SPELayer::Process(InBufferInfo *InBufinfo)
{
    if (mError == true)
    {
        ReStart();
        mError = false;
    }
    Mutex::Autolock lock(mLock);
    //ALOGD("SPELayer::Process going to take mBufMutex new interface");
    mBufMutexWantLock.lock();
    pthread_mutex_lock(&mBufMutex);
    mBufMutexWantLock.unlock();
    if (mState == SPE_STATE_IDLE)
    {
        ALOGD("SPELayer::Process wrong state, mState=%d,mMode=%d", mState, mMode);
        pthread_mutex_unlock(&mBufMutex);
        return false;
    }
    //    if(mMode == SPE_MODE_REC)
    {
        if ((mULInBufferQ.size() > 20) || (mULOutBufferQ.size() > 20))
        {
            ALOGD("no service? mULInBufferQ.size=%d, mULOutBufferQ.size=%d", mULInBufferQ.size(), mULOutBufferQ.size());
        }
    }
    mState = SPE_STATE_RUNNING;

    AddtoInputBuffer(UPLINK, InBufinfo);
    //AddUplinkBuffer(InBufinfo);

    int inBufLength = InBufinfo->BufLen;
    short *inBuf = InBufinfo->pBufBase;

    //process the input buffer queue
    if (mMode == SPE_MODE_REC)  //record
    {
        mVoIPRunningbefore = false;
        Process_Record(inBuf, inBufLength);
    }
    else    //VoIP
    {
        mVoIPRunningbefore = true;
        Process_VoIP(inBuf, inBufLength);
    }

    Dump_PCM_Out(UPLINK, inBuf, inBufLength);
    pthread_mutex_unlock(&mBufMutex);
    return true;

}

//old interface, only service record
bool    SPELayer::Process(SPE_DATA_DIRECTION dir, short *inBuf, int  inBufLength, short *outBuf, int outBufLength)
{
    if (mError == true)
    {
        ReStart();
        mError = false;
    }
    Mutex::Autolock lock(mLock);
    if ((mState == SPE_STATE_IDLE) || (dir == DOWNLINK && mMode != SPE_MODE_VOIP && mMode != SPE_MODE_AECREC))
    {
        ALOGD("SPELayer::Process wrong state,%d, mState=%d,mMode=%d", dir, mState, mMode);
        return false;
    }
    if (mMode == SPE_MODE_REC)
    {
        if ((mULInBufferQ.size() > 20) || (mULOutBufferQ.size() > 20))
        {
            ALOGD("no service? mULInBufferQ.size=%d, mULOutBufferQ.size=%d", mULInBufferQ.size(), mULOutBufferQ.size());
        }
    }
    //ALOGD("SPELayer::Process old interface");
    Dump_PCM_In(dir, inBuf, inBufLength);

    int copysize = inBufLength >> 2;
    int printindex = 0;
    //    ALOGD("SPELayer::Process, dir=%x, inBuf=%p,inBufLength=%d,mMode=%x,copysize=%d",dir,inBuf,inBufLength,mMode,copysize);

    mState = SPE_STATE_RUNNING;


    BufferInfo *newInBuffer = new BufferInfo;
    newInBuffer->pBufBase = (short *) malloc(inBufLength);

    memcpy(newInBuffer->pBufBase, inBuf, inBufLength);

    newInBuffer->BufLen = inBufLength;
    newInBuffer->pRead = newInBuffer->pBufBase;
    newInBuffer->pWrite = newInBuffer->pBufBase;
    //    ALOGD("inBufLength=%d,mULInBufQLenTotal=%d, Qsize=%d",newInBuffer->BufLen,mULInBufQLenTotal,mULInBufferQ.size());
    if (dir == UPLINK)
    {
        mULInBufferQ.add(newInBuffer);
        mULInBufQLenTotal += inBufLength;
        //          ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, size=%d",mULInBufQLenTotal,mULInBufferQ.size());
    }
    else
    {
        //queue to the downlink input buffer queue
        mDLInBufferQ.add(newInBuffer);
        mDLInBufQLenTotal += inBufLength;
        ALOGD("SPELayer::Process, mDLInBufQLenTotal=%d, size=%d", mDLInBufQLenTotal, mDLInBufferQ.size());
        //also add to delay buffer queue
        if (mNeedDelayLatency && (mLatencyDir == true)) //the first DL buffer, add the delay time buffer as first delay buffer queue
        {
            mNeedDelayLatency = false;
            BufferInfo *newDelayBuffer = new BufferInfo;
            newDelayBuffer->pBufBase = (short *) malloc(mLatencySampleCount * 2 * sizeof(short)); //one channel, 16bits
            newDelayBuffer->BufLen = mLatencySampleCount * 2 * sizeof(short);
            newDelayBuffer->pRead = newDelayBuffer->pBufBase;
            newDelayBuffer->pWrite = newDelayBuffer->pBufBase;
            newDelayBuffer->BufLen4Delay = mLatencySampleCount * 2 * sizeof(short);
            newDelayBuffer->pRead4Delay = newDelayBuffer->pBufBase;
            newDelayBuffer->pWrite4Delay = newDelayBuffer->pBufBase;
            memset(newDelayBuffer->pBufBase, 0, newDelayBuffer->BufLen);
            mDLDelayBufferQ.add(newDelayBuffer);
            mDLDelayBufQLenTotal += newDelayBuffer->BufLen;
            //              ALOGD("newDelayBuffer->BufLen=%d, size=%d",newDelayBuffer->BufLen,mDLDelayBufferQ.size());
        }

        newInBuffer->BufLen4Delay = inBufLength;
        newInBuffer->pRead4Delay = newInBuffer->pBufBase;
        newInBuffer->pWrite4Delay = newInBuffer->pBufBase;
        mDLDelayBufferQ.add(newInBuffer);
        mDLDelayBufQLenTotal += inBufLength;
        //          ALOGD("SPELayer::Process, mDLDelayBufQLenTotal=%d, size=%d",mDLDelayBufQLenTotal, mDLDelayBufferQ.size());
    }

    //process the input buffer queue
    if (mMode == SPE_MODE_REC)  //record
    {

        //       ALOGD("SPELayer::Process, SPERecBufSize=%d,inBufLength=%d,mULInBufQLenTotal=%d, Insize=%d,Outsize=%d",SPERecBufSize,inBufLength,mULInBufQLenTotal,mULInBufferQ.size(),mULOutBufferQ.size());
        if ((mULInBufQLenTotal < mSPEProcessBufSize) && (mULOutBufferQ.size() == 0)) //not enough UL buffer for process
        {
            ALOGD("SPELayer::Process,going memset 0 inBuf=%p,inBufLength=%d", inBuf, inBufLength);
            memset(inBuf, 0, inBufLength); //return in same input buffer address
            //memset(outBuf,0,inBufLength);   //return in output buffer address
            return true;
        }

        //        ALOGD("SPELayer::Process, enough mULInBufQLenTotal buffer,size=%d",mULInBufferQ.size());
        while (mULInBufQLenTotal >= mSPEProcessBufSize)
        {
            int tmpSPEProcessBufSize = mSPEProcessBufSize;
            int indexIn = 0;
            int tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;

            //            ALOGD("SPELayer::Process, mULInBufQLenTotal=%d, SPERecBufSize=%d,tempULIncopysize=%d",mULInBufQLenTotal,mSPEProcessBufSize,tempULIncopysize);
            while (tmpSPEProcessBufSize)
            {
                if (mULInBufferQ.isEmpty())
                {
                    ALOGD("uplink input buffer queue is empty, something wrong!!");
                    mError = true;
                    break;
                }

                //            ALOGD("SPELayer indexIn=%d, tmpSPERecBufSize=%d, mULInBufQLenTotal=%d,mULInBufferQ[0]->pRead=%p,mULInBufferQ[0]->pBufBase=%p,mULInBufferQ[0]->BufLen=%d,tempULIncopysize=%d",indexIn,tmpSPERecBufSize,mULInBufQLenTotal,mULInBufferQ[0]->pRead,mULInBufferQ[0]->pBufBase,mULInBufferQ[0]->BufLen,tempULIncopysize);
                if (tempULIncopysize > 0) //get the buffer data from the first uplink input buffer queue
                {
                    *(mpSPEBufferUL1 + indexIn) = *(mULInBufferQ[0]->pRead);    //left channel
                    *(mpSPEBufferUL2 + indexIn) = *(mULInBufferQ[0]->pRead + 1); //right channel
                    //ALOGD("%d,%d",*(mULInBufferQ[0]->pRead),*(mULInBufferQ[0]->pRead +1));
                    mULInBufferQ[0]->pRead += 2;
                    tempULIncopysize--;
                    indexIn++;
                    tmpSPEProcessBufSize -= 4;
                    mULInBufQLenTotal -= 4; //int and short transform
                    mULInBufferQ[0]->BufLen -= 4; //record the buffer you consumed
                }
                else    //consume all the data in first queue buffer
                {
                    free(mULInBufferQ[0]->pBufBase);
                    delete mULInBufferQ[0];
                    mULInBufferQ.removeAt(0);
                    tempULIncopysize = mULInBufferQ[0]->BufLen >> 2;
                    //                    ALOGD("UL in buffer consume finish, next BufferBase=%p",mULInBufferQ[0]->pBufBase);
                }
            }

            if (mError)
            {
                ALOGD("error!!");
                break;
            }

            //process the fill in buffer
            ENH_API_Process(&mSph_Enh_ctrl);

            Dump_EPL(&mSph_Enh_ctrl.EPL_buffer, EPLBufSize * sizeof(short));
            EPLTransVMDump();

            BufferInfo *newOutBuffer = new BufferInfo;

            newOutBuffer->pBufBase = (short *) malloc(mSPEProcessBufSize);
            newOutBuffer->BufLen = mSPEProcessBufSize;

            newOutBuffer->pRead = newOutBuffer->pBufBase;
            newOutBuffer->pWrite = newOutBuffer->pBufBase;
            //           ALOGD("newOutBuffer->pBufBase=%p,newOutBuffer->pRead=%p,newOutBuffer->pWrite=%p,newOutBuffer->BufLen=%d",newOutBuffer->pBufBase,newOutBuffer->pRead,newOutBuffer->pWrite,newOutBuffer->BufLen);


            int indexOut = 0;

            int copysizetest = newOutBuffer->BufLen >> 2;
            while (copysizetest)
            {
                //                ALOGD("newOutBuffer->pWrite=%p, indexOut=%d,copysizetest=%d",newOutBuffer->pWrite,indexOut,copysizetest);
                *(newOutBuffer->pWrite) = *(mpSPEBufferUL1 + indexOut);
                *(newOutBuffer->pWrite + 1) = *(mpSPEBufferUL2 + indexOut);
                newOutBuffer->pWrite += 2;
                indexOut++;
                copysizetest--;
            }

            Dump_PCM_Process(UPLINK, newOutBuffer->pBufBase, newOutBuffer->BufLen);

            mULOutBufferQ.add(newOutBuffer);
            mULOutBufQLenTotal += newOutBuffer->BufLen;

            //            ALOGD("mULOutBufQLenTotal=%d, indexOut=%d,newOutBuffer->pWrite=%p, mULOutBufferQsize=%d",mULOutBufQLenTotal,indexOut,newOutBuffer->pWrite,mULOutBufferQ.size());
        }

    }


    //process the output buffer queue
    if (mMode == SPE_MODE_REC)
    {
        //        ALOGD("mULOutBufferQ=%d, mULOutBufQLenTotal=%d",mULOutBufferQ.size(),mULOutBufQLenTotal);

        if (mULOutBufferQ.isEmpty() || mULOutBufQLenTotal < inBufLength)
        {
            ALOGD("SPELayer not enought UL output buffer");
            memset(inBuf, 0, inBufLength); //return in same input buffer address
            //memset(outBuf,0,inBufLength);   //return in output buffer address
            return true;
        }
        int tmpInBufLength = inBufLength;
        int count = 0;
        int tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
        while (tmpInBufLength)
        {
            if (mULOutBufferQ.isEmpty())
            {
                ALOGD("SPELayer::uplink Output buffer queue is empty, something wrong!!");
                mError = true;
                break;
            }


            //            ALOGD("mULOutBufferQ.size = %d,tempULCopy=%d",mULOutBufferQ.size(),tempULCopy);

            if (tempULCopy > 0) //get the buffer data from the first uplink input buffer queue
            {
                //                ALOGD("mULOutBufferQ[0]->pRead = %p,mULOutBufferQ[0]->pBufBase=%p,mULOutBufferQ[0]->BufLen=%d",mULOutBufferQ[0]->pRead,mULOutBufferQ[0]->pBufBase,mULOutBufferQ[0]->BufLen);
                //                ALOGD("tmpInBufLength=%d,count=%d,mULOutBufQLenTotal=%d,tempULCopy=%d",tmpInBufLength,count,mULOutBufQLenTotal,tempULCopy);
                *(inBuf + count) = *(mULOutBufferQ[0]->pRead);
                *(inBuf + count + 1) = *(mULOutBufferQ[0]->pRead + 1);
                mULOutBufferQ[0]->pRead += 2;
                tmpInBufLength -= 4; //int and short transform
                tempULCopy--;
                count += 2;
                mULOutBufQLenTotal -= 4; //int and short transform
                mULOutBufferQ[0]->BufLen -= 4;
            }
            else    //consume all the data in first queue buffer
            {
                free(mULOutBufferQ[0]->pBufBase);
                delete mULOutBufferQ[0];
                mULOutBufferQ.removeAt(0);
                tempULCopy = mULOutBufferQ[0]->BufLen >> 2;
                //                ALOGD("SPELayer::uplink Output buffer consumed");
            }

        }

    }


    Dump_PCM_Out(dir, inBuf, inBufLength);
    return true;
}

bool    SPELayer::Stop()
{
    ALOGD("SPELayer::Stop");
    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);
    if (mState == SPE_STATE_IDLE)
    {
        ALOGD("not start before");
        pthread_mutex_unlock(&mBufMutex);
        return false;
    }
    mState = SPE_STATE_CLEANING;
    Clear();
    pthread_mutex_unlock(&mBufMutex);
    return true;
}


void SPELayer::ReStart()
{
    ALOGD("SPELayer::ReStart, State=%d, mode=%d", mState, mMode);
    Stop();
    Start(mMode);
}

void SPELayer::Clear()
{
    ALOGD("SPELayer::Clear");
    //pthread_mutex_lock(&mBufMutex );
    if (mSphCtrlBuffer)
    {
        ALOGD("free mSphCtrlBuffer %p", mSphCtrlBuffer);
        ENH_API_Free(&mSph_Enh_ctrl);
        free(mSphCtrlBuffer);
        mSphCtrlBuffer = NULL;
        ALOGD("~free mSphCtrlBuffer");
    }

    mpSPEBufferUL1 = NULL;
    mpSPEBufferUL2 = NULL;
    mpSPEBufferDL = NULL;
    mpSPEBufferDLDelay = NULL;

    mNeedDelayLatency = false;
    mNeedJitterBuffer = false;
    mCompensatedBufferSize = 0;

#if 0
    //clear the buffer queue

    ALOGD("SPELayer::mULOutBufferQ size=%d,mULInBufferQ.size=%d,mDLOutBufferQ.size()=%d,mDLInBufferQ.size()=%d,mDLDelayBufferQ.size()=%d", mULOutBufferQ.size(), mULInBufferQ.size(), mDLOutBufferQ.size(),
          mDLInBufferQ.size(), mDLDelayBufferQ.size());
    if (mULOutBufferQ.size() != 0)
    {
        while (mULOutBufferQ.size())
        {
            free(mULOutBufferQ[0]->pBufBase);
            delete mULOutBufferQ[0];
            mULOutBufferQ.removeAt(0);
        }
        mULOutBufferQ.clear();
    }
    if (mULInBufferQ.size() != 0)
    {
        while (mULInBufferQ.size())
        {
            free(mULInBufferQ[0]->pBufBase);
            delete mULInBufferQ[0];
            mULInBufferQ.removeAt(0);
        }
        mULInBufferQ.clear();
    }

    if (mDLOutBufferQ.size() != 0)
    {
        while (mDLOutBufferQ.size())
        {
            free(mDLOutBufferQ[0]->pBufBase);
            delete mDLOutBufferQ[0];
            mDLOutBufferQ.removeAt(0);
        }
        mDLOutBufferQ.clear();
    }
    if (mDLInBufferQ.size() != 0)
    {
        while (mDLInBufferQ.size())
        {
            if (mDLInBufferQ[0]->pBufBase)
            {
                ALOGD("mDLInBufferQ::pBufBase=%d", mDLInBufferQ[0]->pBufBase);
                //                free(mDLInBufferQ[0]->pBufBase);
                ALOGD("mDLInBufferQ::free");
                //                delete mDLInBufferQ[0];
                ALOGD("mDLInBufferQ::delete");
                mDLInBufferQ.removeAt(0);
                ALOGD("mDLInBufferQ::done, free at DLDelay buffer");
            }
        }
        mDLInBufferQ.clear();
    }

    if (mDLDelayBufferQ.size() != 0)
    {
        while (mDLDelayBufferQ.size())
        {
            if (mDLDelayBufferQ[0]->pBufBase)
            {
                ALOGD("mDLDelayBufferQ::pBufBase=%d", mDLDelayBufferQ[0]->pBufBase);
                free(mDLDelayBufferQ[0]->pBufBase);
                ALOGD("mDLDelayBufferQ::free");
                delete mDLDelayBufferQ[0];
                ALOGD("mDLDelayBufferQ::delete");
                mDLDelayBufferQ.removeAt(0);
                ALOGD("mDLDelayBufferQ::done");
            }

        }
        mDLDelayBufferQ.clear();
    }

    mULInBufQLenTotal = 0;
    mDLInBufQLenTotal = 0;
    mULOutBufQLenTotal = 0;
    mDLOutBufQLenTotal = 0;
    mDLDelayBufQLenTotal = 0;
    mCompensatedBufferSize = 0;
#endif
    mState = SPE_STATE_IDLE;

    ALOGD("~Clear");
    //pthread_mutex_unlock(&mBufMutex);
}

bool SPELayer::Standby()
{
    ALOGD("SPELayer::Standby");
    bool bRet = true;

    Mutex::Autolock lock(mLock);
    pthread_mutex_lock(&mBufMutex);

    mState = SPE_STATE_CLEANING;
    Clear();

    mMode = SPE_MODE_NONE;
    mRoute = ROUTE_NONE;

    mError = false;

    FlushBufferQ();

    mFirstVoIPUplink = true;
    mFirstVoIPDownlink = true;
    mDLNewStart = false;
    mPrepareProcessDataReady = false;
    mDLPreQLimit = true;
    mDLlateAdjust = false;

    mVoIPRunningbefore = false;
    DLdataPrepareCount = 0;
    mOutputStreamRunning = false;

    mLatencyDir = true;
    mNeedJitterBuffer = false;
    mNormalModeVoIP = false;
    mPreULBufLen = 0;
    mPreDLBufLen = 0;

    memset(&mUplinkIntrStartTime, 0, sizeof(timespec));
    memset(&mPreUplinkEstTime, 0, sizeof(timespec));

    //    memset(&mDownlinkIntrStartTime, 0, sizeof(timespec));
    //    memset(&mPreDownlinkEstTime, 0, sizeof(timespec));
    //    memset(&mPreDownlinkQueueTime, 0, sizeof(timespec));

    pthread_mutex_unlock(&mBufMutex);
    ALOGD("Standby --");
    return bRet;
}

timespec SPELayer::GetSystemTime(bool print, int dir)
{
    struct timespec systemtime;
    int rc;
    rc = clock_gettime(CLOCK_MONOTONIC, &systemtime);
    if (rc != 0)
    {
        systemtime.tv_sec  = 0;
        systemtime.tv_nsec = 0;
        ALOGD("clock_gettime error");
    }
    if (print == true)
    {
        ALOGD("GetSystemTime %d, sec %ld nsec %ld", dir, systemtime.tv_sec, systemtime.tv_nsec);
    }

    return systemtime;
}

void SPELayer::dump()
{
    ALOGD("SPELayer::dump, State=%d, mode=%d", mState, mMode);
    //dump normal record parameters
    /*  ALOGD("Record:Samplerate = %d, FrameRate=%d,PGAGain=%d, App_table=%x, Fea_Cfg_table=%x",mRecordSampleRate,mRecordFrameRate,mRecordPGAGain,mRecordApp_table,mRecordFea_Cfg_table);
        ALOGD("Record:EnhanceParas");
        for(int i=0; i<EnhanceParasNum; i++)
            ALOGD("%d",mRecordEnhanceParas[i]);
        ALOGD("Record:DMNRCalData");
        for(int i=0; i<DMNRCalDataNum; i++)
            ALOGD("%d",mRecordDMNRCalData[i]);
        ALOGD("Record:CompenFilter");
        for(int i=0; i<CompenFilterNum; i++)
            ALOGD("%d",mRecordCompenFilter[i]);


        //dump VoIP parameters
        ALOGD("VoIP:Samplerate = %d, FrameRate=%d,PGAGain=%d, App_table=%x, Fea_Cfg_table=%x",mVoIPSampleRate,mVoIPFrameRate,mVoIPPGAGain,mVoIPApp_table,mVoIPFea_Cfg_table);
        ALOGD("VoIP:EnhanceParas");
        for(int i=0; i<EnhanceParasNum; i++)
            ALOGD("%d",mVoIPEnhanceParas[i]);
        ALOGD("VoIP:DMNRCalData");
        for(int i=0; i<DMNRCalDataNum; i++)
            ALOGD("%d",mVoIPDMNRCalData[i]);
        ALOGD("VoIP:CompenFilter");
        for(int i=0; i<CompenFilterNum; i++)
            ALOGD("%d",mVoIPCompenFilter[i]);
    */
    //dump using parameters
    ALOGD("Using:Samplerate = %d, FrameRate=%d,MIC_DG=%d, App_table=%x, Fea_Cfg_table=%x, MMI_table=%x, Device_mode=%x, MMI_MIC_GAIN=%d",
          mSph_Enh_ctrl.sample_rate, mSph_Enh_ctrl.frame_rate, mSph_Enh_ctrl.MIC_DG, mSph_Enh_ctrl.App_table, mSph_Enh_ctrl.Fea_Cfg_table, mSph_Enh_ctrl.MMI_ctrl,
          mSph_Enh_ctrl.Device_mode, mSph_Enh_ctrl.MMI_MIC_GAIN);
    ALOGD("Using:EnhanceParas");
    for (int i = 0; i < (EnhanceParasNum / 7); i++)
        ALOGD("[index %d] %d,%d,%d,%d,%d,%d,%d", i, mSph_Enh_ctrl.enhance_pars[i * 7], mSph_Enh_ctrl.enhance_pars[i * 7 + 1], mSph_Enh_ctrl.enhance_pars[i * 7 + 2]
              , mSph_Enh_ctrl.enhance_pars[i * 7 + 3], mSph_Enh_ctrl.enhance_pars[i * 7 + 4], mSph_Enh_ctrl.enhance_pars[i * 7 + 5], mSph_Enh_ctrl.enhance_pars[i * 7 + 6]);

    ALOGD("Using:DMNRCalData");
    for (int i = 0; i < (DMNRCalDataNum / 19); i++)
        ALOGD("[index %d] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
              i, mSph_Enh_ctrl.DMNR_cal_data[i * 19], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 1], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 2]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 3], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 4], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 5], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 6]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 7], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 8], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 9], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 10]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 11], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 12], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 13], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 14]
              , mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 15], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 16], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 17], mSph_Enh_ctrl.DMNR_cal_data[i * 19 + 18]);

    /*  ALOGD("Using:CompenFilter");
        for(int i=0; i<CompenFilterNum; i++)
            ALOGD("%d",mSph_Enh_ctrl.Compen_filter[i]);
        */
}

static int checkAndCreateDirectory(const char *pC)
{
    char tmp[PATH_MAX];
    int i = 0;

    while (*pC)
    {
        tmp[i] = *pC;

        if (*pC == '/' && i)
        {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0)
            {
                if (mkdir(tmp, 0770) == -1)
                {
                    ALOGE("AudioDumpPCM: mkdir error! %s\n", (char *)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
    return 0;

}

bool SPELayer::HasBufferDump()
{
    DumpMutexLock();
    bool bret = true;
    if (mDumpDLInBufferQ.size() == 0 && mDumpDLOutBufferQ.size() == 0 && mDumpULInBufferQ.size() == 0 && mDumpULOutBufferQ.size() == 0
        && mDumpEPLBufferQ.size() == 0)
    {
        bret = false;
    }

    DumpMutexUnlock();
    return bret;
}

void SPELayer::DumpBufferClear(void)
{
    DumpMutexLock();
    ALOGD("DumpBufferClear, %d %d %d %d %d", mDumpDLInBufferQ.size(), mDumpDLOutBufferQ.size(), mDumpULInBufferQ.size(), mDumpULOutBufferQ.size(), mDumpEPLBufferQ.size());
    if (mDumpDLInBufferQ.size() != 0)
    {
        while (mDumpDLInBufferQ.size())
        {
            free(mDumpDLInBufferQ[0]->pBufBase);
            delete mDumpDLInBufferQ[0];
            mDumpDLInBufferQ.removeAt(0);
        }
        mDumpDLInBufferQ.clear();
    }

    if (mDumpDLOutBufferQ.size() != 0)
    {
        while (mDumpDLOutBufferQ.size())
        {
            free(mDumpDLOutBufferQ[0]->pBufBase);
            delete mDumpDLOutBufferQ[0];
            mDumpDLOutBufferQ.removeAt(0);
        }
        mDumpDLOutBufferQ.clear();
    }

    if (mDumpULInBufferQ.size() != 0)
    {
        while (mDumpULInBufferQ.size())
        {
            free(mDumpULInBufferQ[0]->pBufBase);
            delete mDumpULInBufferQ[0];
            mDumpULInBufferQ.removeAt(0);
        }
        mDumpULInBufferQ.clear();
    }

    if (mDumpULOutBufferQ.size() != 0)
    {
        while (mDumpULOutBufferQ.size())
        {
            free(mDumpULOutBufferQ[0]->pBufBase);
            delete mDumpULOutBufferQ[0];
            mDumpULOutBufferQ.removeAt(0);
        }
        mDumpULOutBufferQ.clear();
    }

    if (mDumpEPLBufferQ.size() != 0)
    {
        while (mDumpEPLBufferQ.size())
        {
            free(mDumpEPLBufferQ[0]->pBufBase);
            delete mDumpEPLBufferQ[0];
            mDumpEPLBufferQ.removeAt(0);
        }
        mDumpEPLBufferQ.clear();
    }
    DumpMutexUnlock();
    ALOGD("DumpBufferClear---");
}

void *DumpThread(void *arg)
{
    SPELayer *pSPEL = (SPELayer *)arg;
    ALOGD("DumpThread");
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 10 * 1000000;
    pthread_mutex_lock(&pSPEL->mDumpExitMutex);

    while (1)
    {
        if (pSPEL->hDumpThread == NULL)
        {
            ALOGD("DumpThread hDumpThread null");
            //pSPEL->DumpBufferClear();
            break;
#if 0
            if (!pSPEL->HasBufferDump())
            {
                pSPEL->DumpMutexLock();
                pSPEL->mDumpDLInBufferQ.clear();
                pSPEL->mDumpDLOutBufferQ.clear();
                pSPEL->mDumpULInBufferQ.clear();
                pSPEL->mDumpULOutBufferQ.clear();
                pSPEL->mDumpEPLBufferQ.clear();
                pSPEL->DumpMutexUnlock();
                ALOGD("DumpThread exit");
                break;
            }
            else
            {
                ALOGD("DumpThread still has buffer need consume");
            }
#endif
        }
        if (!pSPEL->HasBufferDump())
        {
            usleep(3 * 1000);
            continue;
        }
        //ALOGD( "DumpThread,mDumpULInBufferQ=%d, mDumpULOutBufferQ=%d, mDumpEPLBufferQ=%d",pSPEL->mDumpULInBufferQ.size(),pSPEL->mDumpULOutBufferQ.size(),pSPEL->mDumpEPLBufferQ.size());
        if (pSPEL->mDumpDLInBufferQ.size() > 0)
        {
            fwrite(pSPEL->mDumpDLInBufferQ[0]->pBufBase, pSPEL->mDumpDLInBufferQ[0]->BufLen, 1, pSPEL->mfpInDL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != NULL)
            {
                free(pSPEL->mDumpDLInBufferQ[0]->pBufBase);
                delete pSPEL->mDumpDLInBufferQ[0];
                pSPEL->mDumpDLInBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpDLOutBufferQ.size() > 0)
        {
            fwrite(pSPEL->mDumpDLOutBufferQ[0]->pBufBase, pSPEL->mDumpDLOutBufferQ[0]->BufLen, 1, pSPEL->mfpOutDL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != NULL)
            {
                free(pSPEL->mDumpDLOutBufferQ[0]->pBufBase);
                delete pSPEL->mDumpDLOutBufferQ[0];
                pSPEL->mDumpDLOutBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpULInBufferQ.size() > 0)
        {
            fwrite(pSPEL->mDumpULInBufferQ[0]->pBufBase, pSPEL->mDumpULInBufferQ[0]->BufLen, 1, pSPEL->mfpInUL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != NULL)
            {
                free(pSPEL->mDumpULInBufferQ[0]->pBufBase);
                delete pSPEL->mDumpULInBufferQ[0];
                pSPEL->mDumpULInBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpULOutBufferQ.size() > 0)
        {
            fwrite(pSPEL->mDumpULOutBufferQ[0]->pBufBase, pSPEL->mDumpULOutBufferQ[0]->BufLen, 1, pSPEL->mfpOutUL);
            pSPEL->DumpMutexLock();
            if (pSPEL->hDumpThread != NULL)
            {
                free(pSPEL->mDumpULOutBufferQ[0]->pBufBase);
                delete pSPEL->mDumpULOutBufferQ[0];
                pSPEL->mDumpULOutBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }

        if (pSPEL->mDumpEPLBufferQ.size() > 0)
        {
            fwrite(pSPEL->mDumpEPLBufferQ[0]->pBufBase, pSPEL->mDumpEPLBufferQ[0]->BufLen, 1, pSPEL->mfpEPL);
            pSPEL->DumpMutexLock();
            //            ALOGD("DumpThread %p, %p",pSPEL->mDumpEPLBufferQ[0],pSPEL->mDumpEPLBufferQ[0]->pBufBase);
            if (pSPEL->hDumpThread != NULL)
            {
                free(pSPEL->mDumpEPLBufferQ[0]->pBufBase);
                delete pSPEL->mDumpEPLBufferQ[0];
                pSPEL->mDumpEPLBufferQ.removeAt(0);
            }
            pSPEL->DumpMutexUnlock();
        }
    }
    pthread_mutex_unlock(&pSPEL->mDumpExitMutex);
    pthread_cond_signal(&pSPEL->mDumpExit_Cond);
    ALOGD("DumpThread exit!!");
    pthread_exit(NULL);
    return 0;
}

bool SPELayer::CreateDumpThread()
{
#if defined(PC_EMULATION)
    hDumpThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DumpThread, this, 0, 0);
    if (hDumpThread == 0) { return false; }
    return true;
#else
    //create PCM data dump thread here
    int ret;
    ret = pthread_create(&hDumpThread, NULL, DumpThread, (void *)this);
    if (ret != 0) { return false; }

    ALOGD("-CreateDumpThread \n");
    return true;
#endif

}

void SPELayer::Dump_Enalbe_Check(void)
{
    int ret;
    char Buf[10];
    sprintf(Buf, "%d.pcm", DumpFileNum);
    char value[PROPERTY_VALUE_MAX];
    char value1[PROPERTY_VALUE_MAX];

    //Dump_PCM_In check
    property_get("SPEIn.pcm.dump", value, "0");
    int bflag = atoi(value);
    if (bflag)
    {
        if (hDumpThread == NULL)
        {
            CreateDumpThread();
        }

        //uplink
        String8 DumpFileNameUpIn;
        char filenameUpIn[] = "/sdcard/mtklog/audio_dump/SPEIn_Uplink";
        DumpFileNameUpIn = String8(filenameUpIn);
        DumpFileNameUpIn.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpIn);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_In UPLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpInUL == NULL)
            {
                mfpInUL = fopen(DumpFileNameUpIn, "ab+");
                if (mfpInUL == NULL)
                {
                    ALOGD("open  SPEIn_Uplink.pcm fail");
                }
                else
                {
                    ALOGD("open SPEIn_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownIn;
        char filenameDownIn[] = "/sdcard/mtklog/audio_dump/SPEIn_Downlink";
        DumpFileNameDownIn = String8(filenameDownIn);
        DumpFileNameDownIn.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownIn);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_In DOWNLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpInDL == NULL)
            {
                mfpInDL = fopen(DumpFileNameDownIn, "ab+");
                if (mfpInDL == NULL)
                {
                    ALOGD("open  SPEIn_Downlink.pcm fail");
                }
                else
                {
                    ALOGD("open SPEIn_Downlink.pcm");
                }
            }
        }
    }

    //Dump_PCM_Process check
    property_get("SPE.pcm.dump", value, "0");
    bflag = atoi(value);
    if (bflag)
    {
        //if(hDumpThread == NULL)
        //CreateDumpThread();

        //uplink
        String8 DumpFileNameUpPro;
        char filenameUpPro[] = "/sdcard/mtklog/audio_dump/SPE_Uplink";
        DumpFileNameUpPro = String8(filenameUpPro);
        DumpFileNameUpPro.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpPro);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_Process UPLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpProcessedUL == NULL)
            {
                mfpProcessedUL = fopen(DumpFileNameUpPro, "ab+");
                if (mfpProcessedUL == NULL)
                {
                    ALOGD("open  SPE_Uplink.pcm fail");
                }
                else
                {
                    ALOGD("open SPE_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownPro;
        char filenameDownPro[] = "/sdcard/mtklog/audio_dump/SPE_Downlink";
        DumpFileNameDownPro = String8(filenameDownPro);
        DumpFileNameDownPro.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownPro);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_Process DOWNLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpProcessedDL == NULL)
            {
                mfpProcessedDL = fopen(DumpFileNameDownPro, "ab+");
                if (mfpProcessedDL == NULL)
                {
                    ALOGD("open  SPE_Downlink.pcm fail");
                }
                else
                {
                    ALOGD("open SPE_Downlink.pcm");
                }
            }
        }
    }

    //Dump_PCM_Out check
    property_get("SPEOut.pcm.dump", value, "0");
    bflag = atoi(value);
    if (bflag)
    {
        if (hDumpThread == NULL)
        {
            CreateDumpThread();
        }

        //uplink
        String8 DumpFileNameUpOut;
        char filenameUpOut[] = "/sdcard/mtklog/audio_dump/SPEOut_Uplink";
        DumpFileNameUpOut = String8(filenameUpOut);
        DumpFileNameUpOut.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameUpOut);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_Out UPLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpOutUL == NULL)
            {
                mfpOutUL = fopen(DumpFileNameUpOut, "ab+");
                if (mfpOutUL == NULL)
                {
                    ALOGD("open  SPEOut_Uplink.pcm fail");
                }
                else
                {
                    ALOGD("open SPEOut_Uplink.pcm");
                }
            }
        }

        //downlink
        String8 DumpFileNameDownOut;
        char filenameDownOut[] = "/sdcard/mtklog/audio_dump/SPEOut_Downlink";
        DumpFileNameDownOut = String8(filenameDownOut);
        DumpFileNameDownOut.append((const char *)Buf);
        ret = checkAndCreateDirectory(DumpFileNameDownOut);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_PCM_Out DOWNLINK checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpOutDL == NULL)
            {
                mfpOutDL = fopen(DumpFileNameDownOut, "ab+");
                if (mfpOutDL == NULL)
                {
                    ALOGD("open  SPEOut_Downlink.pcm fail");
                }
                else
                {
                    ALOGD("open SPEOut_Downlink.pcm");
                }
            }
        }
    }

    //Dump_EPL check
    property_get("SPE_EPL", value, "0");
    property_get("streamin.epl.dump", value1, "0");
    bflag = atoi(value);
    int bflag1 = atoi(value1);
    if (bflag || bflag1)
    {
        if (hDumpThread == NULL)
        {
            CreateDumpThread();
        }

        sprintf(Buf, "%d.EPL", DumpFileNum);
        String8 DumpFileNameEPL;
        char filenameEPL[] = "/sdcard/mtklog/audio_dump/SPE_EPL";
        DumpFileNameEPL = String8(filenameEPL);
        DumpFileNameEPL.append((const char *)Buf);

        ALOGD("Dump_EPL DumpFileNameEPL = %s", DumpFileNameEPL.string());

        ret = checkAndCreateDirectory(DumpFileNameEPL);
        if (ret < 0)
        {
            ALOGE("SPELayer::Dump_EPL checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpEPL == NULL)
            {
                mfpEPL = fopen(DumpFileNameEPL, "ab+");
                if (mfpEPL == NULL)
                {
                    ALOGD("open SPE_EPL.EPL fail");
                }
                else
                {
                    ALOGD("open SPE_EPL.EPL");
                }
            }
        }
    }
}

void SPELayer::Dump_PCM_In(SPE_DATA_DIRECTION dir, void *buffer, int bytes)
{
    if (hDumpThread == NULL)
    {
        return;
    }
    if (dir == UPLINK)
    {
        if (mfpInUL != NULL)
        {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpULInBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }
    else
    {
        if (mfpInDL != NULL)
        {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpDLInBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }

}

void SPELayer::Dump_PCM_Process(SPE_DATA_DIRECTION dir, void *buffer, int bytes)
{
    if (dir == UPLINK)
    {
        if (mfpProcessedUL != NULL)
        {
            fwrite(buffer, bytes, 1, mfpProcessedUL);
        }
    }
    else
    {
        if (mfpProcessedDL != NULL)
        {
            fwrite(buffer, bytes, 1, mfpProcessedDL);
        }
    }
}

void SPELayer::Dump_PCM_Out(SPE_DATA_DIRECTION dir, void *buffer, int bytes)
{
    if (hDumpThread == NULL)
    {
        return;
    }
    if (dir == UPLINK)
    {
        if (mfpOutUL != NULL)
        {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpULOutBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }
    else
    {
        if (mfpOutDL != NULL)
        {
            BufferInfo *newInBuffer = new BufferInfo;
            newInBuffer->pBufBase = (short *) malloc(bytes);
            memcpy(newInBuffer->pBufBase, buffer, bytes);

            newInBuffer->BufLen = bytes;
            newInBuffer->pRead = newInBuffer->pBufBase;
            newInBuffer->pWrite = newInBuffer->pBufBase;
            DumpMutexLock();
            mDumpDLOutBufferQ.add(newInBuffer);
            DumpMutexUnlock();
        }
    }
}

void SPELayer::Dump_EPL(void *buffer, int bytes)
{
    if (hDumpThread == NULL)
    {
        return;
    }
    if (mfpEPL != NULL)
    {
        BufferInfo *newInBuffer = new BufferInfo;
        newInBuffer->pBufBase = (short *) malloc(bytes);
        memcpy(newInBuffer->pBufBase, buffer, bytes);

        newInBuffer->BufLen = bytes;
        newInBuffer->pRead = newInBuffer->pBufBase;
        newInBuffer->pWrite = newInBuffer->pBufBase;
        DumpMutexLock();
        //        ALOGD("Dump_EPL %p, %p",newInBuffer,newInBuffer->pBufBase);
        mDumpEPLBufferQ.add(newInBuffer);
        DumpMutexUnlock();
    }
}

void SPELayer::EPLTransVMDump()
{

    char value[PROPERTY_VALUE_MAX];
    property_get("APVM.dump", value, "0");
    int bflag = atoi(value);
    if (bflag || mVMDumpEnable)
    {
        int ret;
        char filename[] = "/sdcard/mtklog/audio_dump/SPE.VM";
        if (bflag)
        {
            memset(mVMDumpFileName, 0, 128);
            strcpy(mVMDumpFileName, filename);
        }
        if (mVMDumpFileName == NULL)
        {
            ALOGE("no mVMDumpFileName name?");
        }

        ret = checkAndCreateDirectory(mVMDumpFileName);
        if (ret < 0)
        {
            ALOGE("EPLTransVMDump checkAndCreateDirectory() fail!!!");
        }
        else
        {
            if (mfpVM == NULL && mVMDumpFileName != NULL)
            {
                mfpVM = fopen(mVMDumpFileName, "ab+");
            }
        }

        if (mfpVM != NULL)
        {
            if (mSph_Enh_ctrl.sample_rate == 48000)
            {
                /*        memcpy(mVM, mSph_Enh_ctrl.EPL_buffer, RecBufSize20ms*2*sizeof(short));
                        mVM[MaxVMSize-2] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                        mVM[MaxVMSize-1] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                */
                ALOGD("EPLTransVMDump 48k write to sdcard");
                for (int i = 0; i < MaxVMSize; i++)
                {
                    if (i == (MaxVMSize - 2))
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    }
                    else if (i == (MaxVMSize - 1))
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    }
                    else
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[i];
                    }
                }
                //                ALOGE("EPLTransVMDump write to sdcard");
                fwrite(mVM, MaxVMSize * sizeof(short), 1, mfpVM);
            }
            else    //suppose only 16K
            {
                /*        memcpy(mVM, &mSph_Enh_ctrl.EPL_buffer[160*4], 320*2*sizeof(short));
                        mVM[640] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                        mVM[641] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                */
                ALOGD("EPLTransVMDump 16k write to sdcard");
                for (int i = 0; i < 642; i++) //320*2+2
                {
                    if (i == 640)
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC1];
                    }
                    else if (i == 641)
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[VMAGC2];
                    }
                    else
                    {
                        mVM[i] = mSph_Enh_ctrl.EPL_buffer[640 + i];
                    }
                }
                fwrite(mVM, 642 * sizeof(short), 1, mfpVM);
            }
        }
        else
        {
            ALOGD("open APVM.dump fail");
        }
    }
}

void SPELayer::SetVMDumpEnable(bool bEnable)
{
    ALOGD("SetVMDumpEnable bEnable=%x", bEnable);
    mVMDumpEnable = bEnable;
}

void SPELayer::SetVMDumpFileName(const char *VMFileName)
{
    ALOGD("+++SetVMDumpFileName VMFileName=%s", VMFileName);
    memset(mVMDumpFileName, 0, 128);
    strcpy(mVMDumpFileName, VMFileName);
    ALOGD("---SetVMDumpFileName VMFileName=%s, mVMDumpFileName=%s", VMFileName, mVMDumpFileName);
}

// ----------------------------------------------------------------------------
}

