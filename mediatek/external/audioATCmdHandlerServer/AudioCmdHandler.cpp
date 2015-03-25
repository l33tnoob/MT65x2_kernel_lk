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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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
 * AudioCmdHandler.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the  handling about audio command comming from AT Command Service.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 * 06 08 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 06 04 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 05 24 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 05 15 2013 donglei.ji
 * [ALPS00683353] [Need Patch] [Volunteer Patch] DMNR3.0 and VOIP tuning check in
 * .
 *
 * 01 17 2013 donglei.ji
 * [ALPS00425279] [Need Patch] [Volunteer Patch] voice ui and password unlock feature check in
 * .
 *
 * 01 09 2013 donglei.ji
 * [ALPS00438595] [Need Patch] [Volunteer Patch] Audio Tool -  HD Calibration
 * HD Record Calibration.
 *
 * 08 15 2012 donglei.ji
 * [ALPS00337843] [Need Patch] [Volunteer Patch] MM Command Handler JB migration
 * MM Command Handler JB migration.
 *
 * 06 28 2012 donglei.ji
 * [ALPS00308450] [Need Patch] [Volunteer Patch] Audio Taste feature
 * Audio Taste feature.
 *
 * 06 27 2012 donglei.ji
 * [ALPS00307929] [Need Patch] [Volunteer Patch] Audio HQA add I2S output 32 bit test case
 * for MT6577 HQA.
 *
 * 04 27 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add two case:
 * 1. get phone support info
 * 2. read dual mic parameters if phone support dual mic
 *
 * 04 20 2012 donglei.ji
 * [ALPS00272538] [Need Patch] [Volunteer Patch] AT Command for ACF/HCF calibration
 * ACF/HCF calibration feature.
 *
 * 04 12 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * Add wide band speech tuning.
 *
 * 03 16 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add speech parameters encode and decode for transfer.
 *
 * 03 09 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * add a set parameters setting.
 *
 * 03 06 2012 donglei.ji
 * [ALPS00247341] [Need Patch] [Volunteer Patch] AT Command for Speech Tuning Tool feature
 * AT Command for Speech Tuning tool feature check in.
 *
 * 12 27 2011 donglei.ji
 * [ALPS00107090] [Need Patch] [Volunteer Patch][ICS Migration] MM Command Handler Service Migration
 * MM Command Handler Service check in.
 *
 * 11 21 2011 donglei.ji
 * [ALPS00094843] [Need Patch] [Volunteer Patch] XLOG enhance
 * log enhance -- SXLOG.
 *
 * 07 15 2011 donglei.ji
 * [ALPS00053673] [Need Patch] [Volunteer Patch][Audio HQA]Add Audio HQA test cases
 * check in HQA code.
 *
 * 06 15 2011 donglei.ji
 * [ALPS00053673] [Need Patch] [Volunteer Patch][Audio HQA]Add Audio HQA test cases
 * MM Cmd Handler code check in for MT6575.
 *
 * 05 26 2011 changqing.yan
 * [ALPS00050318] [Need Patch] [Volunteer Patch]Remove fmaudioplayer and matvaudiopath path from mediaplayer.
 * .
 *
 * 05 26 2011 changqing.yan
 * [ALPS00050318] [Need Patch] [Volunteer Patch]Remove fmaudioplayer and matvaudiopath path from mediaplayer.
 * .
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include <cutils/xlog.h>
#include <utils/String8.h>
#include <sys/types.h> //for opendir, readdir closdir
#include <dirent.h>    //for opendir, readdir closdir
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <media/AudioSystem.h>
#include <audiocustparam/AudioCustParam.h>
#include <AudioCompensationFilter/AudioCompFltCustParam.h>

#include "AudioCmdHandler.h"

#include <binder/IServiceManager.h>
#include <media/IAudioPolicyService.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCmdHandler"

#define AUDIO_TEST_ROOT "/sdcard/"
#define RECORD_DIR_UPLINK "/sdcard/Up_link_ADC/"
#define RECORD_DIR_FM "/sdcard/FM_Play_Record/"
#define RECORD_DIR_I2S "/sdcard/I2SRecord/"

#define MAX_FILE_NAME 512
#define TEMP_ARRAY_SIZE 64
#define AUD_TASTE_TUNING_CMD 0x70
#define DEV_NUM 3

#if 0
typedef AUDIO_VOLUME_CUSTOM_STRUCT AUD_VOL_CUSTOM_STRUCT;
#else
typedef AUDIO_VER1_CUSTOM_VOLUME_STRUCT AUD_VOL_CUSTOM_STRUCT;
#endif

#define HD_REC_48K_SUPPORT 0x51

using namespace android;

const char *findFileName(const char *pFilePath, const char *pFileName, char *pFileWholeName)
{
    char *pSearchFileNameSrc = NULL;
    DIR *pSearchPath;
    struct dirent *pFileHandle;

    if ((pSearchPath = opendir(pFilePath)) == NULL)
    {
        SXLOGE("open file path: %s error", pFilePath);
        return NULL;
    }

    while ((pFileHandle = readdir(pSearchPath)) != NULL)
    {
        pSearchFileNameSrc = pFileHandle->d_name;
        if (strncmp(pSearchFileNameSrc, pFileName, 2/*strlen(pFileName)*/) == 0)
        {
            SXLOGD("Find the file: %s",  pFileHandle->d_name);
            if (pFileWholeName == NULL)
            {
                SXLOGE("the pointer pFileWholeName is NULL");
                return NULL;
            }

            strcpy(pFileWholeName, pFileHandle->d_name);
            closedir(pSearchPath);
            return pFileWholeName;
        }
    }

    SXLOGE("there are not the file: %s", pFileName);
    return NULL;
}

// AT Command for Speech calibration
static void dataEncode(char *pPara, int length)
{
    char *pParaTemp = pPara + length;
    memcpy((void *)pParaTemp, (void *)pPara, length);

    for (int i = 0; i < length; i++)
    {
        *(pPara + 2 * i) = ((*(pParaTemp + i) >> 4) & 0x0F) | 0x30;
        *(pPara + 2 * i + 1) = (*(pParaTemp + i) & 0x0F) | 0x30;
    }
}

static void dataDecode(char *pPara, int length)
{
    char *pParaTemp = pPara + length;

    for (int i = 0; i < length; i++)
    {
        *(pPara + i) = ((*(pPara + 2 * i) << 4) & 0xF0) | (*(pPara + 2 * i + 1) & 0x0F);
    }

    memset(pParaTemp, 0, length);
}

/*=============================================================================
 *                             Public Function
 *===========================================================================*/

AudioCmdHandler::AudioCmdHandler() :
    m_RecordMaxDur(-1),
    m_RecordChns(1),
    m_RecordSampleRate(8000),
    m_RecordBitsPerSample(16),
    m_fd(-1),
    m_bRecording(false),
    m_RecordAudioSource(Analog_MIC1_Single),
    m_bHDRecTunning(false)
{
    SXLOGD("Constructor--AudioCmdHandler::AudioCmdHandler()");
    m_pMediaRecorderListenner = new PCMRecorderListener(this);
}

AudioCmdHandler::~AudioCmdHandler()
{
    SXLOGD("Deconstructor--AudioCmdHandler::AudioCmdHandler()");

    if (m_MediaRecorderClient.get() != NULL)
    {
        m_MediaRecorderClient->release();
    }

}

ACHStatus AudioCmdHandler::setRecorderParam(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::setRecorderParam() in");

    if (audioCmdParams.param1 < 0)
    {
        SXLOGE("Fail to setRecorderParam: the duration <0");
        return ACHParamError;
    }

    m_RecordMaxDur = audioCmdParams.param1;

    if (audioCmdParams.param2 != 1 && audioCmdParams.param2 != 2)
    {
        SXLOGE("Fail to setRecorderParam: the channels is not equal to 1 or 2");
        return ACHParamError;
    }

    m_RecordChns = audioCmdParams.param2;

    if (audioCmdParams.param3 < 8000 || audioCmdParams.param3 > 48000)
    {
        SXLOGE("Fail to setRecorderParam: the sample rate is invalid");
        return ACHParamError;
    }

    m_RecordSampleRate = audioCmdParams.param3;

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::startRecorderFrMIC(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::startRecorderFrMIC() in");

    int ret = ACHSucceeded;
    char recordFilepath[MAX_FILE_NAME];
    char recordFileName[TEMP_ARRAY_SIZE];
    char pRecordSrc[TEMP_ARRAY_SIZE];
    AudioSourceType audioSourceType = Analog_MIC1_Single;

    if (audioCmdParams.param1 < 0 || audioCmdParams.param1 > 4)
    {
        return ACHParamError;
    }

    sprintf(pRecordSrc, "HQA_RDMIC_P1=%d", audioCmdParams.param1);
    AudioSystem::setParameters(0, String8(pRecordSrc));
    /*
        switch (audioCmdParams.param1) {
          case 0:
            audioSourceType = Analog_MIC1_Single;
            break;
          case 1:
            audioSourceType = Analog_MIC2_Single;
            break;
          case 3:
            audioSourceType = Analog_MIC_Dual;
            break;
          default:
    //      SXLOGE("AudioCmdHandler::startRecorderFrMIC-the audio source type is not supported");
    //      return ACHParamError;
            break;
        }
    */
    if (audioCmdParams.param2 != 0)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param3 < 1 || audioCmdParams.param3 > 999)
    {
        return ACHParamError;
    }

    strcpy(recordFilepath, RECORD_DIR_UPLINK);
    if (access(recordFilepath, F_OK) < 0)
    {
        SXLOGE("startRecorderFrMIC() the path %s is not exit", recordFilepath);
        ret = mkdir(recordFilepath, S_IRWXU | S_IRWXG | S_IRWXO);
        if (-1 == ret)
        {
            SXLOGE("startRecorderFrMIC() create path %s failed", recordFilepath);
            return ACHFailed;
        }
    }

    sprintf(recordFileName, "%03d.wav", audioCmdParams.param3);
    strcat(recordFilepath, recordFileName);

    ret = startRecorder(recordFilepath, audioSourceType);
    if (ret != ACHSucceeded)
    {
        return ACHFailed;
    }

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::setMICGain(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::setMICGain() in");

    if (audioCmdParams.param1 < 0 || audioCmdParams.param1 > 63)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param2 < 0 || audioCmdParams.param2 > 62)
    {
        return ACHParamError;
    }

    char pPGAGain[TEMP_ARRAY_SIZE];
    sprintf(pPGAGain, "HQA_PGAGAIN_P1=%d", audioCmdParams.param1);
    AudioSystem::setParameters(0, String8(pPGAGain));

    char minPGAGain[TEMP_ARRAY_SIZE];
    sprintf(minPGAGain, "HQA_PGAGAIN_P2=%d", audioCmdParams.param2);
    AudioSystem::setParameters(0, String8(minPGAGain));

    return ACHSucceeded;

}

ACHStatus AudioCmdHandler::startRecorderFrFM(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::startRecorderFrFM() in");

    int ret = ACHSucceeded;
    char recordFilepath[MAX_FILE_NAME];
    char recordFileName[TEMP_ARRAY_SIZE];

    if (audioCmdParams.param2 != 0)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param3 < 1 || audioCmdParams.param3 > 999)
    {
        return ACHParamError;
    }

    strcpy(recordFilepath, RECORD_DIR_FM);
    if (access(recordFilepath, F_OK) < 0)
    {
        SXLOGE("startRecorderFrI2S() the path %s is not exit", recordFilepath);
        ret = mkdir(recordFilepath, S_IRWXU | S_IRWXG | S_IRWXO);
        if (-1 == ret)
        {
            SXLOGE("startRecorderFrI2S() create path %s failed", recordFilepath);
            return ACHFailed;
        }
    }

    sprintf(recordFileName, "%03d.wav", audioCmdParams.param3);
    strcat(recordFilepath, recordFileName);

    ret = startRecorder(recordFilepath, Audio_FM_IN);
    if (ret != ACHSucceeded)
    {
        AudioSystem::setParameters(0, String8("HQA_FMREC_LINE_IN=0"));
        return ACHFailed;
    }

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::playFM()
{
    SXLOGD("AudioCmdHandler::playFM() in");

    AudioSystem::setParameters(0, String8("HQA_FMPLY_LINE_IN=1"));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::stopPlayingFM()
{
    SXLOGD("AudioCmdHandler::stopPlayingFM() in");

    AudioSystem::setParameters(0, String8("HQA_FMPLY_LINE_IN=0"));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::setVDPG(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::setVDPG() in");

    if (audioCmdParams.param1 < 0 || audioCmdParams.param1 > 44)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param2 < 0 || audioCmdParams.param2 > 44)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param3 < 0 || audioCmdParams.param3 > 44)
    {
        return ACHParamError;
    }

    char pVDPG1[TEMP_ARRAY_SIZE];
    sprintf(pVDPG1, "HQA_VDPG_P1=%d", audioCmdParams.param1);
    AudioSystem::setParameters(0, String8(pVDPG1));

    char pVDPG2[TEMP_ARRAY_SIZE];
    sprintf(pVDPG2, "HQA_VDPG_P2=%d", audioCmdParams.param2);
    AudioSystem::setParameters(0, String8(pVDPG2));

    char pVDPG3[TEMP_ARRAY_SIZE];
    sprintf(pVDPG3, "HQA_VDPG_P3=%d", audioCmdParams.param3);
    AudioSystem::setParameters(0, String8(pVDPG3));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::setAUDLINEPG(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::setAUDLINEPG() in");

    if (audioCmdParams.param1 < 0 || audioCmdParams.param1 > 8)
    {
        return ACHParamError;
    }

    char pAUDLINEPG[TEMP_ARRAY_SIZE];
    sprintf(pAUDLINEPG, "HQA_AUDLINEPG_P1=%d", audioCmdParams.param1);
    AudioSystem::setParameters(0, String8(pAUDLINEPG));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::setFMorMICVUPG(AudioCmdParam &audioCmdParams, bool bFMGain)
{
    SXLOGD("AudioCmdHandler::setFMVUPG() in");

    if (audioCmdParams.param1 < 0 || audioCmdParams.param1 > 63)
    {
        return ACHParamError;
    }

    if (audioCmdParams.param2 < 0 || audioCmdParams.param2 > 63)
    {
        return ACHParamError;
    }

    char pFMorMICVUPG1[TEMP_ARRAY_SIZE];
    char pFMorMICVUPG2[TEMP_ARRAY_SIZE];

    if (bFMGain)
    {
        sprintf(pFMorMICVUPG1, "HQA_FMVUPG_P1=%d", audioCmdParams.param1);
        sprintf(pFMorMICVUPG2, "HQA_FMVUPG_P2=%d", audioCmdParams.param2);
    }
    else
    {
        sprintf(pFMorMICVUPG1, "HQA_MICVUPG_P1=%d", audioCmdParams.param1);
        sprintf(pFMorMICVUPG2, "HQA_MICVUPG_P2=%d", audioCmdParams.param2);
    }

    AudioSystem::setParameters(0, String8(pFMorMICVUPG1));
    AudioSystem::setParameters(0, String8(pFMorMICVUPG2));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::startAudioPlayer(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::startAudioPlayer() in");

    char filePath[MAX_FILE_NAME];
    char fileName[TEMP_ARRAY_SIZE];
    char fileDir[TEMP_ARRAY_SIZE];
    char fileDirSub[TEMP_ARRAY_SIZE];
    char fileWholeName[TEMP_ARRAY_SIZE];

    strcpy(filePath, AUDIO_TEST_ROOT);
    sprintf(fileName, "%02d", audioCmdParams.param3);
    sprintf(fileDir, "%02d/%02d/", audioCmdParams.param1, audioCmdParams.param2);
    strcat(filePath, fileDir);

    findFileName(filePath, fileName, fileWholeName);
    strcat(filePath, fileWholeName);
    SXLOGD("AudioCmdHandler::startAudioPlayer:Audio file is %s", filePath);

    if (m_MediaPlayerClient.get() == NULL)
    {
        m_MediaPlayerClient = new MediaPlayer();
        m_MediaPlayerClient->setListener(this);
    }
    else if (m_MediaPlayerClient->isPlaying())
    {
        m_MediaPlayerClient->stop();
        m_MediaPlayerClient->reset();
    }

    if (m_MediaPlayerClient->setDataSource(filePath, NULL/* headers*/) != NO_ERROR)
    {
        m_MediaPlayerClient->reset();
        SXLOGE("Fail to load the audio file");
        return ACHLoadFileFailed;
    }

    m_MediaPlayerClient->setAudioStreamType(AUDIO_STREAM_MUSIC);
    if (m_MediaPlayerClient->prepare() != NO_ERROR)
    {
        m_MediaPlayerClient->reset();
        SXLOGE("Fail to play the audio file, prepare failed");
        return ACHFailed;
    }

    if (m_MediaPlayerClient->start() != NO_ERROR)
    {
        m_MediaPlayerClient->reset();
        SXLOGE("Fail to play the audio file, start failed");
        return ACHFailed;
    }

    if (2 == audioCmdParams.param1)
    {
        AudioSystem::setParameters(0, String8("HQA_MEDPLY_P1=2"));
    }
    else
    {
        AudioSystem::setParameters(0, String8("HQA_MEDPLY_P1=1"));
    }
    SXLOGD("AudioCmdHandler::startAudioPlayer() out");
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::stopAudioPlayer()
{
    SXLOGD("AudioCmdHandler::stopAudioPlayer() in");

    if (m_MediaPlayerClient.get() != NULL && m_MediaPlayerClient->isPlaying())
    {
        if (m_MediaPlayerClient->stop() != NO_ERROR)
        {
            SXLOGE("Fail to stop playing the audio file");
            return ACHFailed;
        }
        m_MediaPlayerClient->reset();
        return ACHSucceeded;
    }

    return ACHFailed;
}

ACHStatus AudioCmdHandler::startRecorderFrI2S(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::startRecorderFrI2S() in");

    int ret = ACHSucceeded;
    char recordFilepath[MAX_FILE_NAME];
    char recordFileName[TEMP_ARRAY_SIZE];

    if (audioCmdParams.param3 < 1 || audioCmdParams.param3 > 99)
    {
        return ACHParamError;
    }

    strcpy(recordFilepath, RECORD_DIR_I2S);
    if (access(recordFilepath, F_OK) < 0)
    {
        SXLOGE("startRecorderFrI2S() the path %s is not exit", recordFilepath);
        ret = mkdir(recordFilepath, S_IRWXU | S_IRWXG | S_IRWXO);
        if (-1 == ret)
        {
            SXLOGE("startRecorderFrI2S() create path %s failed", recordFilepath);
            return ACHFailed;
        }
    }

    sprintf(recordFileName, "%02d.wav", audioCmdParams.param3);
    strcat(recordFilepath, recordFileName);

    m_RecordSampleRate = audioCmdParams.param4;

    ret = startRecorder(recordFilepath, Audio_I2S_IN);
    if (ret != ACHSucceeded)
    {
        stopFMAudioPlayer();
        AudioSystem::setParameters(0, String8("HQA_I2SREC=0"));
        return ACHFailed;
    }

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::writeRegister(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::writeRegister() in");

    char pRegWrite1[TEMP_ARRAY_SIZE];
    sprintf(pRegWrite1, "HQA_REGWRITE_P1=%x", audioCmdParams.param1);
    AudioSystem::setParameters(0, String8(pRegWrite1));

    char pRegWrite2[TEMP_ARRAY_SIZE];
    sprintf(pRegWrite2, "HQA_REGWRITE_P2=%x", audioCmdParams.param2);
    AudioSystem::setParameters(0, String8(pRegWrite2));

    char pRegWrite3[TEMP_ARRAY_SIZE];
    sprintf(pRegWrite3, "HQA_REGWRITE_P3=%x", audioCmdParams.param3);
    AudioSystem::setParameters(0, String8(pRegWrite3));

    return ACHSucceeded;
}

String8 AudioCmdHandler::readRegister(AudioCmdParam &audioCmdParams)
{
    SXLOGD("AudioCmdHandler::readRegister() in");

    char pRegRead[TEMP_ARRAY_SIZE];
    String8 returnValue = String8("");

    sprintf(pRegRead, "HQA_REGREAD_P1=%x,%x", audioCmdParams.param1, audioCmdParams.param2);
    returnValue = AudioSystem::getParameters(0, String8(pRegRead));
    return returnValue;
}

void AudioCmdHandler::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    SXLOGD("AudioCmdHandler received message: msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
    switch (msg)
    {
        case MEDIA_PLAYBACK_COMPLETE:
            if (m_MediaPlayerClient.get() != NULL)
            {
                m_MediaPlayerClient->stop();
                m_MediaPlayerClient->reset();
            }

            SXLOGD("AudioCmdHandler::notify -- audio playback complete");
            break;
        case MEDIA_ERROR:
            if (m_MediaPlayerClient.get() != NULL)
            {
                m_MediaPlayerClient->reset();
            }

            SXLOGE("AudioCmdHandler::notify -- audio playback error, exit");
            break;
        default:
            break;
    }
}

// add for mt6575 HQA
void AudioCmdHandler::setParameters(const String8 &keyValuePaires)
{
    AudioSystem::setParameters(0, keyValuePaires);
}

// add for mt6577 HQA
ACHStatus AudioCmdHandler::SetAudioData(int cmdType, void *ptr, size_t len)
{
    SXLOGD("AudioCmdHandler::SetAudioData() cmdType=%d, len=%d", cmdType, len);
    int ret = NO_ERROR;
    ret = AudioSystem::SetAudioData(cmdType, len, ptr);

    return ret == NO_ERROR ? ACHSucceeded : ACHFailed;
}

// add for speech parameters calibration-2/6/2012
ACHStatus AudioCmdHandler::DLCustSPHParamToNV(void *pParam, int block)
{
    int write_size = 0;
    int size = 0;

    SXLOGD("AudioCmdHandler::DLCustSPHParamToNV() in");
    AUD_SPH_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_PARAM_MED_STRUCT *pSPHMedPara = (AUDIO_PARAM_MED_STRUCT *)malloc(sizeof(AUDIO_PARAM_MED_STRUCT));
    if (pSPHMedPara == NULL)
    {
        return ACHFailed;
    }
    else if (pParam == NULL)
    {
        free(pSPHMedPara);
        pSPHMedPara = NULL;
        return ACHFailed;
    }

    size = GetMedParamFromNV(pSPHMedPara);
    SXLOGD("DLCustSPHParamToNV- GetMedParamFromNV read size=%d", size);
    if (size != sizeof(AUDIO_PARAM_MED_STRUCT))
    {
        SXLOGD("DLCustSPHParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_PARAM_MED_STRUCT), size);
        free(pSPHMedPara);
        pSPHMedPara = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        pCustomPara = (AUD_SPH_PARAM_STRUCT *)(pParam + sizeof(int));
        AUDIO_CUSTOM_PARAM_STRUCT *pSPHPara = (AUDIO_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        if (pSPHPara == NULL)
        {
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        SXLOGD("DLCustSPHParamToNV customer size=%d,sph_param size=%d", size , sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        size = GetNBSpeechParamFromNVRam(pSPHPara);
        if (size != sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
        {
            SXLOGD("DLCustSPHParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_PARAM_STRUCT), size);
            free(pSPHPara);
            pSPHPara = NULL;
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        size = sizeof(pCustomPara->sph_com_param) + sizeof(pCustomPara->sph_mode_param) + sizeof(pCustomPara->sph_in_fir) + sizeof(pCustomPara->sph_out_fir);
        dataDecode((char *)pParam, size + sizeof(int));
        if (*((int *)pParam) != size)
        {
            SXLOGE("DLCustSPHParamToNV miss data !!");
            free(pSPHPara);
            pSPHPara = NULL;
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("Received FIR Coefs sph_out_fir[0][%d]=%d", i, pCustomPara->sph_out_fir[0][i]);
        }

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("Speech Out FIR Coefs ori sph_out_fir[0][%d]=%d", i, pSPHPara->sph_out_fir[0][i]);
        }

        SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        SXLOGD("Speech Param uSupportVM_ori = %d", pSPHPara->uSupportVM);
        SXLOGD("Speech Param uAutoVM_ori = %d", pSPHPara->uAutoVM);
        SXLOGD("Speech Param uMicbiasVolt_ori = %d", pSPHPara->uMicbiasVolt);

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ;  i++)
        {
            SXLOGV("Received speech mode parameters ori sph_mode_param[0][%d]=%d", i, pCustomPara->sph_mode_param[0][i]);
        }

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ;  i++)
        {
            SXLOGV("Speech mode parameters ori speech_mode_para[0][%d]=%d", i, pSPHPara->speech_mode_para[0][i]);
        }

        memcpy((void *)pSPHPara->speech_common_para, (void *)pCustomPara->sph_com_param, sizeof(pCustomPara->sph_com_param));
        memcpy((void *)pSPHPara->speech_mode_para, (void *)pCustomPara->sph_mode_param, sizeof(pCustomPara->sph_mode_param));
        memcpy((void *)pSPHPara->sph_in_fir, (void *)pCustomPara->sph_in_fir, sizeof(pCustomPara->sph_in_fir));
        memcpy((void *)pSPHPara->sph_out_fir, (void *)pCustomPara->sph_out_fir, sizeof(pCustomPara->sph_out_fir));

        memcpy((void *)pSPHMedPara->speech_mode_para, (void *)pCustomPara->sph_mode_param, sizeof(pCustomPara->sph_mode_param));
        memcpy((void *)pSPHMedPara->speech_input_FIR_coeffs, (void *)pCustomPara->sph_in_fir, sizeof(pCustomPara->sph_in_fir));

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("Speech Out FIR Coefs new sph_out_fir[0][%d]=%d", i, pSPHPara->sph_out_fir[0][i]);
        }

        SXLOGV("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        SXLOGV("Speech Param uSupportVM_new = %d", pSPHPara->uSupportVM);
        SXLOGV("Speech Param uAutoVM_new = %d", pSPHPara->uAutoVM);
        SXLOGV("Speech Param uMicbiasVolt_new = %d", pSPHPara->uMicbiasVolt);

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ;  i++)
        {
            SXLOGV("Speech mode parameters new speech_mode_para[0][%d]=%d", i, pSPHPara->speech_mode_para[0][i]);
        }

        write_size = SetNBSpeechParamToNVRam(pSPHPara);
        if (write_size != sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
        {
            SXLOGD("DLCustSPHParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_CUSTOM_PARAM_STRUCT), write_size);
            free(pSPHPara);
            pSPHPara = NULL;
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        free(pSPHPara);
        pSPHPara = NULL;
    }
    else if (block == 1)
    {
        size = 2 * sizeof(pCustomPara->sph_output_FIR_coeffs[0]);
        dataDecode((char *)pParam, size + sizeof(int));

        if (*((int *)pParam) != size)
        {
            SXLOGE("DLCustSPHParamToNV miss data !! block = %d, received size = %d", block, *((int *)pParam));
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        for (int i = 0; i < SPH_MODE_NUM ;  i++)
        {
            SXLOGV("selected Speech Out FIR index ori select_FIR_output_index[%d]=%d", i, pSPHMedPara->select_FIR_output_index[i]);
        }

        for (int i = 0; i < SPH_MODE_NUM; i++)
        {
            for (int j = 0; j < SPH_ENHANCE_PARAM_NUM; j++)
            {
                SXLOGV("MED Speech Out FIR index ori speech_output_FIR_coeffs[%d][0][%d]=%d", i, j, pSPHMedPara->speech_output_FIR_coeffs[i][0][j]);
            }
        }

        memcpy((void *)pSPHMedPara->speech_output_FIR_coeffs, pParam + sizeof(int), size);
    }
    else
    {
        size = 2 * sizeof(pCustomPara->sph_output_FIR_coeffs[0]) + sizeof(pCustomPara->selected_FIR_output_index);
        dataDecode((char *)pParam, size + sizeof(int));

        if (*((int *)pParam) != size)
        {
            SXLOGE("DLCustSPHParamToNV miss data !! block = %d, received size = %d", block, *((int *)pParam));
            free(pSPHMedPara);
            pSPHMedPara = NULL;
            return ACHFailed;
        }

        memcpy((void *)pSPHMedPara->speech_output_FIR_coeffs[2], pParam + sizeof(int), 2 * sizeof(pCustomPara->sph_output_FIR_coeffs[0]));
        size = 2 * sizeof(pCustomPara->sph_output_FIR_coeffs[0]) + sizeof(int);
        memcpy((void *)pSPHMedPara->select_FIR_output_index, pParam + size, sizeof(pCustomPara->selected_FIR_output_index));

        for (int i = 0; i < SPH_MODE_NUM ;  i++)
        {
            SXLOGV("selected Speech Out FIR index new select_FIR_output_index[%d]=%d", i, pSPHMedPara->select_FIR_output_index[i]);
        }

        for (int i = 0; i < SPH_MODE_NUM; i++)
        {
            for (int j = 0; j < SPH_ENHANCE_PARAM_NUM; j++)
            {
                SXLOGV("MED Speech Out FIR index new speech_output_FIR_coeffs[%d][0][%d]=%d", i, j, pSPHMedPara->speech_output_FIR_coeffs[i][0][j]);
            }
        }

        AudioSystem::setParameters(0, String8("UpdateSpeechParameter=0"));
    }

    write_size = SetMedParamToNV(pSPHMedPara);
    if (write_size != sizeof(AUDIO_PARAM_MED_STRUCT))
    {
        SXLOGD("DLCustSPHParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_PARAM_MED_STRUCT), write_size);
        free(pSPHMedPara);
        pSPHMedPara = NULL;
        return ACHFailed;
    }

    free(pSPHMedPara);
    pSPHMedPara = NULL;

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::ULCustSPHParamFromNV(void *pParam, int *len, int block)
{
    int size = 0;
    int dataLen = 0;

    SXLOGD("AudioCmdHandler::ULCustSPHParamFromNV() in");
    AUDIO_CUSTOM_PARAM_STRUCT *pSPHPara = (AUDIO_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    AUDIO_PARAM_MED_STRUCT *pSPHMedPara = (AUDIO_PARAM_MED_STRUCT *)malloc(sizeof(AUDIO_PARAM_MED_STRUCT));
    if (pSPHMedPara == NULL || pSPHPara == NULL || pParam == NULL)
    {
        return ACHFailed;
    }

    size = GetNBSpeechParamFromNVRam(pSPHPara);
    if (size != sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("ULCustSPHParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_PARAM_STRUCT), size);
        free(pSPHPara);
        pSPHPara = NULL;
        free(pSPHMedPara);
        pSPHPara = NULL;
        return ACHFailed;
    }

    size = GetMedParamFromNV(pSPHMedPara);
    if (size != sizeof(AUDIO_PARAM_MED_STRUCT))
    {
        SXLOGD("ULCustSPHParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_PARAM_MED_STRUCT), size);
        free(pSPHPara);
        pSPHPara = NULL;
        free(pSPHMedPara);
        pSPHMedPara = NULL;
        return ACHFailed;
    }

    for (int i = 0; i < FIR_NUM_NB ;  i++)
    {
        SXLOGV("Speech Out FIR Coefs ori sph_out_fir[0][%d]=%d", i, pSPHPara->sph_out_fir[0][i]);
    }

    SXLOGV("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGV("Speech Param uSupportVM_ori = %d", pSPHPara->uSupportVM);
    SXLOGV("Speech Param uAutoVM_ori = %d", pSPHPara->uAutoVM);
    SXLOGV("Speech Param uMicbiasVolt_ori = %d", pSPHPara->uMicbiasVolt);

    for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ;  i++)
    {
        SXLOGV("Speech mode parameters ori speech_mode_para[0][%d]=%d", i, pSPHPara->speech_mode_para[0][i]);
    }

    for (int i = 0; i < 8 ;  i++)
    {
        SXLOGV("selected Speech Out FIR index ori select_FIR_output_index[%d]=%d", i, pSPHMedPara->select_FIR_output_index[i]);
    }

    for (int i = 0; i < 5 ;  i++)
    {
        SXLOGV("MED Speech Out FIR index ori speech_output_FIR_coeffs[1][0][%d]=%d", i, pSPHMedPara->speech_output_FIR_coeffs[1][0][i]);
    }

    if (block == 0)
    {
        dataLen = sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
        memcpy(pParam + sizeof(int), (void *)pSPHPara, dataLen);
    }
    else if (block == 1)
    {
        dataLen = sizeof(pSPHMedPara->speech_input_FIR_coeffs);
        memcpy(pParam + sizeof(int), (void *)pSPHMedPara->speech_input_FIR_coeffs, dataLen);
    }
    else if (block == 2 || block == 3)
    {
        dataLen = 3 * sizeof(pSPHMedPara->speech_output_FIR_coeffs[0]);
        memcpy(pParam + sizeof(int), (void *)pSPHMedPara->speech_output_FIR_coeffs[3 * block - 6], dataLen);
    }
    else
    {
        dataLen = 2 * sizeof(pSPHMedPara->speech_output_FIR_coeffs[0]) + sizeof(pSPHMedPara->speech_mode_para);
        dataLen += sizeof(pSPHMedPara->select_FIR_intput_index) + sizeof(pSPHMedPara->select_FIR_output_index);
        memcpy(pParam + sizeof(int), (void *)pSPHMedPara->speech_output_FIR_coeffs[6], dataLen);
    }

    SXLOGD("ULCustSPHParamFromNV the data sent to PC is %d", dataLen);

    *((int *)pParam) = dataLen;
    *len = 2 * (dataLen + sizeof(int));

    dataEncode((char *)pParam, dataLen + sizeof(int));

    free(pSPHPara);
    pSPHPara = NULL;
    free(pSPHMedPara);
    pSPHMedPara = NULL;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::DLCustSPHVolumeParamToNV(void *pParam)
{
    int write_size = 0;
    int size = 0;
    int dataLen = 0;

    SXLOGD("AudioCmdHandler::DLCustSPHVolumeParamToNV() in");

    AUD_VOL_CUSTOM_STRUCT *pSPHVolPara = (AUD_VOL_CUSTOM_STRUCT *)malloc(sizeof(AUD_VOL_CUSTOM_STRUCT));
    if (pSPHVolPara == NULL || pParam == NULL)
    {
        return ACHFailed;
    }

    dataDecode((char *)pParam, sizeof(AUD_VOL_CUSTOM_STRUCT) + sizeof(int));
    dataLen = *((int *)pParam);
    if (dataLen != sizeof(AUD_VOL_CUSTOM_STRUCT))
    {
        SXLOGE("DLCustSPHVolumeParamToNV miss data !!");
        free(pSPHVolPara);
        pSPHVolPara = NULL;
        return ACHFailed;
    }

    AUD_VOL_CUSTOM_STRUCT *pCustomPara = (AUD_VOL_CUSTOM_STRUCT *)(pParam + sizeof(int));

#if 0
    size = GetAudioCustomParamFromNV(pSPHVolPara);
#else
    size = GetVolumeVer1ParamFromNV(pSPHVolPara);
#endif
    if (size != sizeof(AUD_VOL_CUSTOM_STRUCT))
    {
        SXLOGD("DLCustSPHVolumeParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUD_VOL_CUSTOM_STRUCT), size);
        free(pSPHVolPara);
        pSPHVolPara = NULL;
        return ACHFailed;
    }
#if 0
    SXLOGV("~~~~~~~~~~~~~~~~DL mic volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("ori data - mic volume audiovolume_mic[%d][%d] = %d", i, j, pSPHVolPara->audiovolume_mic[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~DL sid volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("ori data - sid volume audiovolume_sid[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sid[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~DL sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("ori data - sph volume audiovolume_sph[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }
#else
    SXLOGV("~~~~~~~~~~~~~~~~DL ring volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("ori data - ring volume audiovolume_ring[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_ring[i][j]);
        }
    }

    SXLOGV("~~~~~~~~~~~~~~~~DL sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("ori data - mic volume audiovolume_sph[%d][%d] = %d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }

    SXLOGV("~~~~~~~~~~~~~~~~audio volume level~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < VER1_NUM_OF_VOL_TYPE; i++)
    {
        SXLOGV("ori data - audio volume level audiovolume_level[%d] = %d", i, pSPHVolPara->audiovolume_level[i]);
    }
#endif

    memcpy((void *)pSPHVolPara, (void *)pCustomPara, sizeof(AUD_VOL_CUSTOM_STRUCT));

#if 0
    SXLOGV("~~~~~~~~~~~~~~~~mic volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("new data - mic volume audiovolume_mic[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_mic[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~sid volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("new data - sid volume audiovolume_sid[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sid[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("new data - sph volume audiovolume_sph[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }

    write_size = SetAudioCustomParamToNV(pSPHVolPara);
#else
    SXLOGV("~~~~~~~~~~~~~~~~DL ring volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("new data - ring volume audiovolume_ring[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_ring[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~DL sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("new data - sph volume audiovolume_sph[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~audio volume level~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        SXLOGV("new data - audio volume level audiovolume_level[%d]=%d", i, pSPHVolPara->audiovolume_level[i]);
    }

    write_size = SetVolumeVer1ParamToNV(pSPHVolPara);
#endif

    if (write_size != sizeof(AUD_VOL_CUSTOM_STRUCT))
    {
        SXLOGD("DLCustSPHVolumeParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUD_VOL_CUSTOM_STRUCT), write_size);
        free(pSPHVolPara);
        pSPHVolPara = NULL;
        return ACHFailed;
    }

    free(pSPHVolPara);
    pSPHVolPara = NULL;

    int volumeIndex[DEV_NUM][AUDIO_STREAM_CNT];
    for (int i = 0; i < AUDIO_STREAM_CNT; i++)
    {
        // get volue for different devices, 0:receiver; 1:speaker; 2:headset;
        AudioSystem::getStreamVolumeIndex((audio_stream_type_t)i, &volumeIndex[0][i], AUDIO_DEVICE_OUT_EARPIECE);
        AudioSystem::getStreamVolumeIndex((audio_stream_type_t)i, &volumeIndex[1][i], AUDIO_DEVICE_OUT_SPEAKER);
        AudioSystem::getStreamVolumeIndex((audio_stream_type_t)i, &volumeIndex[2][i], AUDIO_DEVICE_OUT_WIRED_HEADSET);
    }
#ifdef MTK_AUDIO
    const sp<IAudioPolicyService> &aps = AudioSystem::get_audio_policy_service();
    if (aps == 0) { return ACHFailed; }
    aps->SetPolicyManagerParameters(LOAD_VOLUME_POLICY, 0, 0, 0);
#endif
    AudioSystem::setParameters(0, String8("UpdateSphVolumeParameter=0"));
    for (int i = 0; i < AUDIO_STREAM_CNT; i++)
    {
        AudioSystem::setStreamVolumeIndex((audio_stream_type_t)i, volumeIndex[0][i], AUDIO_DEVICE_OUT_EARPIECE);
        AudioSystem::setStreamVolumeIndex((audio_stream_type_t)i, volumeIndex[1][i], AUDIO_DEVICE_OUT_SPEAKER);
        AudioSystem::setStreamVolumeIndex((audio_stream_type_t)i, volumeIndex[2][i], AUDIO_DEVICE_OUT_WIRED_HEADSET);
    }

    return ACHSucceeded;

}

ACHStatus AudioCmdHandler::ULCustSPHVolumeParamFromNV(void *pParam, int *len)
{
    int size = 0;

    SXLOGD("AudioCmdHandler::ULCustSPHVolumeParamFromNV() in");

    AUD_VOL_CUSTOM_STRUCT *pSPHVolPara = (AUD_VOL_CUSTOM_STRUCT *)malloc(sizeof(AUD_VOL_CUSTOM_STRUCT));
    if (pSPHVolPara == NULL || pParam == NULL)
    {
        return ACHFailed;
    }

    *len = 2 * (sizeof(AUD_VOL_CUSTOM_STRUCT) + sizeof(int));

#if 0
    size = GetAudioCustomParamFromNV(pSPHVolPara);
#else
    size = GetVolumeVer1ParamFromNV(pSPHVolPara);
#endif

    if (size != sizeof(AUD_VOL_CUSTOM_STRUCT))
    {
        SXLOGD("ULCustSPHVolumeParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUD_VOL_CUSTOM_STRUCT), size);
        free(pSPHVolPara);
        pSPHVolPara = NULL;
        return ACHFailed;
    }

#if 0
    SXLOGV("~~~~~~~~~~~~~~~~UL mic volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP ; j++)
        {
            SXLOGV("ori data - mic volume audiovolume_mic[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_mic[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~UL sid volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("ori data - sid volume audiovolume_sid[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sid[i][j]);
        }
    }
    SXLOGV("~~~~~~~~~~~~~~~~UL sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < MAX_VOL_CATE; i++)
    {
        for (int j = 0; j < CUSTOM_VOL_STEP; j++)
        {
            SXLOGV("ori data - sph volume audiovolume_sph[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }
#else
    SXLOGV("~~~~~~~~~~~~~~~~UL ring volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("ori data - ring volume audiovolume_ring[%d][%d]=%d", i, j, pSPHVolPara->audiovolume_ring[i][j]);
        }
    }

    SXLOGV("~~~~~~~~~~~~~~~~UL sph volume~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < NUM_OF_VOL_MODE; i++)
    {
        for (int j = 0; j < AUDIO_MAX_VOLUME_STEP; j++)
        {
            SXLOGV("ori data - mic volume audiovolume_sph[%d][%d] = %d", i, j, pSPHVolPara->audiovolume_sph[i][j]);
        }
    }

    SXLOGV("~~~~~~~~~~~~~~~~audio volume level~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    for (int i = 0; i < VER1_NUM_OF_VOL_TYPE; i++)
    {
        SXLOGV("ori data - audio volume level audiovolume_level[%d] = %d", i, pSPHVolPara->audiovolume_level[i]);
    }
#endif

    *((int *)pParam) = sizeof(AUD_VOL_CUSTOM_STRUCT);
    AUD_VOL_CUSTOM_STRUCT *pCustomPara = (AUD_VOL_CUSTOM_STRUCT *)(pParam + sizeof(int));

    memcpy((void *)pCustomPara, (void *)pSPHVolPara, sizeof(AUD_VOL_CUSTOM_STRUCT));
    dataEncode((char *)pParam, sizeof(AUD_VOL_CUSTOM_STRUCT) + sizeof(int));

    free(pSPHVolPara);
    pSPHVolPara = NULL;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::DLCustSPHWBParamToNV(void *pParam, int block)
{
    int write_size = 0;
    int size = 0;
    int dataLen = 0;

    SXLOGD("AudioCmdHandler::DLCustSPHWBParamToNV() in");
#ifdef MTK_WB_SPEECH_SUPPORT
    AUD_SPH_WB_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_CUSTOM_WB_PARAM_STRUCT *pSPHWBPara = (AUDIO_CUSTOM_WB_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
    if (pSPHWBPara == NULL)
    {
        return ACHFailed;
    }

    SXLOGD("DLCustSPHWBParamToNV ,wb sph param size=%d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
    size = GetWBSpeechParamFromNVRam(pSPHWBPara);
    if (size != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
    {
        SXLOGD("DLCustSPHWBParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), size);
        free(pSPHWBPara);
        pSPHWBPara = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        size = sizeof(pCustomPara->sph_mode_wb_param) + sizeof(pCustomPara->sph_wb_in_fir) + sizeof(int);
        dataDecode((char *)pParam, size);

        dataLen = *((int *)pParam);
        pCustomPara = (AUD_SPH_WB_PARAM_STRUCT *)(pParam + sizeof(int));

        if (dataLen != (size - sizeof(int)))
        {
            SXLOGE("DLCustSPHWBParamToNV data miss !!");
            free(pSPHWBPara);
            pSPHWBPara = NULL;
            return ACHFailed;
        }

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ; i++)
        {
            SXLOGV("Received speech mode parameters sph_mode_wb_param[0][i]=%d", i, pCustomPara->sph_mode_wb_param[0][i]);
        }

        for (int i = 0; i < FIR_NUM_NB; i++)
        {
            SXLOGV("Received WB FIR Coefs sph_wb_in_fir[0][%d]=%d", i, pCustomPara->sph_wb_in_fir[0][i]);
        }

        memcpy((void *)pSPHWBPara->speech_mode_wb_para, (void *)pCustomPara->sph_mode_wb_param, sizeof(pCustomPara->sph_mode_wb_param));
        memcpy((void *)pSPHWBPara->sph_wb_in_fir, (void *)pCustomPara->sph_wb_in_fir, sizeof(pCustomPara->sph_wb_in_fir));

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ;  i++)
        {
            SXLOGV("WB speech mode parameters new=%d", pSPHWBPara->speech_mode_wb_para[0][i]);
        }

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("WB speech in FIR Coefs new=%d", pSPHWBPara->sph_wb_in_fir[0][i]);
        }

        write_size = SetWBSpeechParamToNVRam(pSPHWBPara);
        if (write_size != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
        {
            SXLOGD("DLCustSPHWBParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), write_size);
            free(pSPHWBPara);
            pSPHWBPara = NULL;
            return ACHFailed;
        }
    }
    else
    {
        dataDecode((char *)pParam, sizeof(pCustomPara->sph_wb_out_fir) + sizeof(int));

        dataLen = *((int *)pParam);
        if (dataLen != sizeof(pCustomPara->sph_wb_out_fir))
        {
            SXLOGE("DLCustSPHWBParamToNV data miss !!");
            free(pSPHWBPara);
            pSPHWBPara = NULL;
            return ACHFailed;
        }

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("WB speech out FIR Coefs ori=%d", pSPHWBPara->sph_wb_out_fir[0][i]);
        }

        memcpy((void *)pSPHWBPara->sph_wb_out_fir, pParam + sizeof(int), sizeof(pSPHWBPara->sph_wb_out_fir));

        for (int i = 0; i < FIR_NUM_NB ;  i++)
        {
            SXLOGV("WB speech out FIR Coefs new=%d", pSPHWBPara->sph_wb_out_fir[0][i]);
        }

        write_size = SetWBSpeechParamToNVRam(pSPHWBPara);
        if (write_size != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
        {
            SXLOGD("DLCustSPHWBParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), write_size);
            free(pSPHWBPara);
            pSPHWBPara = NULL;
            return ACHFailed;
        }

        AudioSystem::setParameters(0, String8("UpdateSpeechParameter=1"));
    }

    free(pSPHWBPara);
    pSPHWBPara = NULL;
#else
    return ACHFailed;
#endif
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::ULCustSPHWBParamFromNV(void *pParam, int *len, int block)
{
    int size = 0;

    SXLOGD("AudioCmdHandler::ULCustSPHWBParamFromNV() in");
#ifdef MTK_WB_SPEECH_SUPPORT
    AUD_SPH_WB_PARAM_STRUCT *pCustomPara = (AUD_SPH_WB_PARAM_STRUCT *)(pParam + sizeof(int));
    AUDIO_CUSTOM_WB_PARAM_STRUCT *pSPHWBPara = (AUDIO_CUSTOM_WB_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));

    if (pSPHWBPara == NULL)
    {
        return ACHFailed;
    }

    size = GetWBSpeechParamFromNVRam(pSPHWBPara);
    if (size != sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT))
    {
        SXLOGD("ULCustSPHWBParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT), size);
        free(pSPHWBPara);
        pSPHWBPara = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        *len = 2 * (sizeof(pCustomPara->sph_mode_wb_param) + sizeof(pCustomPara->sph_wb_in_fir) + sizeof(int));
        *((int *)pParam) = sizeof(pCustomPara->sph_mode_wb_param) + sizeof(pCustomPara->sph_wb_in_fir);

        for (int i = 0; i < FIR_NUM_NB ; i++)
        {
            SXLOGV("Speech In FIR Coefs ori=%d", pSPHWBPara->sph_wb_in_fir[0][i]);
        }

        for (int i = 0; i < SPH_ENHANCE_PARAM_NUM ; i++)
        {
            SXLOGV("Speech mode parameters ori=%d", pSPHWBPara->speech_mode_wb_para[0][i]);
        }

        memcpy((void *)pCustomPara->sph_mode_wb_param, (void *)pSPHWBPara->speech_mode_wb_para, sizeof(pCustomPara->sph_mode_wb_param));
        memcpy((void *)pCustomPara->sph_wb_in_fir, (void *)pSPHWBPara->sph_wb_in_fir, sizeof(pCustomPara->sph_wb_in_fir));

        dataEncode((char *)pParam, (*len) / 2);
    }
    else if (block == 1)
    {
        *len = 2 * (sizeof(pCustomPara->sph_wb_out_fir) + sizeof(int));
        *((int *)pParam) = sizeof(pCustomPara->sph_wb_out_fir);

        for (int i = 0; i < FIR_NUM_NB ; i++)
        {
            SXLOGV("Speech Out FIR Coefs ori=%d", pSPHWBPara->sph_wb_out_fir[0][i]);
        }

        memcpy((void *)pCustomPara, (void *)pSPHWBPara->sph_wb_out_fir, sizeof(pCustomPara->sph_wb_out_fir));

        dataEncode((char *)pParam, (*len) / 2);
    }

    free(pSPHWBPara);
    pSPHWBPara = NULL;
#else
    return ACHFailed;
#endif
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::DLCustACFParamToNV(void *pParam)
{
    int write_size = 0;
    int size = 0;
    int dataLen = 0;

    SXLOGD("AudioCmdHandler::DLCustACFParamToNV() in");

    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pACFPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    if (pACFPara == NULL)
    {
        return ACHFailed;
    }

    SXLOGD("DLCustACFParamToNV ,acf param size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    size = GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_AUDIO, pACFPara);
    if (size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("DLCustACFParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), size);
        free(pACFPara);
        pACFPara = NULL;
        return ACHFailed;
    }

    size = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int);
    dataDecode((char *)pParam, size);

    dataLen = *((int *)pParam);
    pCustomPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)(pParam + sizeof(int));

    if (dataLen != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGE("DLCustACFParamToNV data miss !!");
        free(pACFPara);
        pACFPara = NULL;
        return ACHFailed;
    }

#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received  - loudness hsf coeffs bes_loudness_hsf_coeff[0][0][%d] = %d", i, pCustomPara->bes_loudness_hsf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pCustomPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness lpf coeffs bes_loudness_lpf_coeff[0][%d]=%d", i, pCustomPara->bes_loudness_lpf_coeff[0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Received - ACF Param bes_loudness_WS_Gain_Max = %d", pCustomPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Received - ACF Param bes_loudness_WS_Gain_Min = %d", pCustomPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Received - ACF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    SXLOGD("Received - ACF Param bes_loudness_Att_Time = %d", pCustomPara->bes_loudness_Att_Time);
    SXLOGD("Received - ACF Param bes_loudness_Rel_Time = %d", pCustomPara->bes_loudness_Rel_Time);

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_Out[i]);
    }
#else
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 4 ; j++)
        {
            SXLOGV("Received  - loudness hsf coeffs bes_loudness_hsf_coeff[%d][%d] = %d", i, j, pCustomPara->bes_loudness_hsf_coeff[i][j]);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pCustomPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Received - ACF Param bes_loudness_DRC_Forget_Table[0][0] = %d", pCustomPara->bes_loudness_DRC_Forget_Table[0][0]);
    SXLOGD("Received - ACF Param bes_loudness_WS_Gain_Max = %d", pCustomPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Received - ACF Param bes_loudness_WS_Gain_Min = %d", pCustomPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Received - ACF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    SXLOGD("Received - ACF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_Out[i]);
    }
#endif

    memcpy((void *)pACFPara, (void *)pCustomPara, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    write_size = SetAudioCompFltCustParamToNV(AUDIO_COMP_FLT_AUDIO, pACFPara);
    if (write_size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("DLCustACFParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), write_size);
        free(pACFPara);
        pACFPara = NULL;
        return ACHFailed;
    }

    AudioSystem::setParameters(0, String8("UpdateACFHCFParameters=0"));

    free(pACFPara);
    pACFPara = NULL;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::ULCustACFParamFromNV(void *pParam, int *len)
{
    int size = 0;

    SXLOGD("AudioCmdHandler::ULCustACFParamFromNV() in");

    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pACFPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    if (pACFPara == NULL)
    {
        return ACHFailed;
    }

    SXLOGD("ULCustACFParamFromNV ,acf param size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    size = GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_AUDIO, pACFPara);
    if (size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("ULCustACFParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), size);
        free(pACFPara);
        pACFPara = NULL;
        return ACHFailed;
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori  - loudness hsf coeffs bes_loudness_hsf_coeff[0][0][%d] = %d", i, pACFPara->bes_loudness_hsf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pACFPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness lpf coeffs bes_loudness_lpf_coeff[0][%d]=%d", i, pACFPara->bes_loudness_lpf_coeff[0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Ori - ACF Param bes_loudness_WS_Gain_Max = %d", pACFPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Ori - ACF Param bes_loudness_WS_Gain_Min = %d", pACFPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Ori - ACF Param bes_loudness_Filter_First = %d", pACFPara->bes_loudness_Filter_First);
    SXLOGD("Ori - ACF Param bes_loudness_Att_Time = %d", pACFPara->bes_loudness_Att_Time);
    SXLOGD("Ori - ACF Param bes_loudness_Rel_Time = %d", pACFPara->bes_loudness_Rel_Time);

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pACFPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pACFPara->bes_loudness_Gain_Map_Out[i]);
    }
#else
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 4 ; j++)
        {
            SXLOGV("Ori  - loudness hsf coeffs bes_loudness_hsf_coeff[%d][%d] = %d", i, j, pACFPara->bes_loudness_hsf_coeff[i][j]);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pACFPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Ori - ACF Param bes_loudness_DRC_Forget_Table[0][0] = %d", pACFPara->bes_loudness_DRC_Forget_Table[0][0]);
    SXLOGD("Ori - ACF Param bes_loudness_WS_Gain_Max = %d", pACFPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Ori - ACF Param bes_loudness_WS_Gain_Min = %d", pACFPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Ori - ACF Param bes_loudness_Filter_First = %d", pACFPara->bes_loudness_Filter_First);
    SXLOGD("Ori - ACF Param bes_loudness_Filter_First = %d", pACFPara->bes_loudness_Filter_First);
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pACFPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pACFPara->bes_loudness_Gain_Map_Out[i]);
    }
#endif

    *((int *)pParam) = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    pCustomPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)(pParam + sizeof(int));

    memcpy((void *)pCustomPara, (void *)pACFPara, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    dataEncode((char *)pParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int));
    *len = 2 * (sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int));

    free(pACFPara);
    pACFPara = NULL;
    return ACHSucceeded;
}
ACHStatus AudioCmdHandler::findACFFO(unsigned short cmdType, void *pParam, int *len)
{
	ACHStatus ret = ACHFailed;
	status_t ret1 = BAD_VALUE;
	int initVal = -1;
	int* pfo_value = &initVal;

	if(cmdType == 0 || cmdType == 1)
	{
		ret1 = AudioSystem::SetAudioCommand(AUDIO_ACF_FO, cmdType);
		if(ret1 == NO_ERROR)
			ret = ACHSucceeded;
	}
	else if(cmdType == 2)
	{
		if(NULL == len)
			return ret;
		
		ret1 = AudioSystem::GetAudioCommand(AUDIO_ACF_FO, pfo_value);
		if(ret1 != NO_ERROR)
			return ACHFailed;
		else
			SXLOGD("findACFFO, Test");

		SXLOGD("findACFFO, fo_value is %d", *pfo_value);
		memcpy(pParam+sizeof(int), (void *)pfo_value, 4);

		*((int *)pParam) = 4;
		*len = 2*(4 + sizeof(int));
		
		dataEncode((char *)pParam, 4+sizeof(int));
	}

	return ret;
}

ACHStatus AudioCmdHandler::DLCustHCFParamToNV(void *pParam)
{
    int write_size = 0;
    int size = 0;
    int dataLen = 0;

    SXLOGD("AudioCmdHandler::DLCustHCFParamToNV() in");

    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pHCFPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    if (pHCFPara == NULL)
    {
        return ACHFailed;
    }

    SXLOGD("DLCustHCFParamToNV ,acf param size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    size = GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_HEADPHONE, pHCFPara);
    if (size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("DLCustHCFParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), size);
        free(pHCFPara);
        pHCFPara = NULL;
        return ACHFailed;
    }

    size = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int);
    dataDecode((char *)pParam, size);

    dataLen = *((int *)pParam);
    pCustomPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)(pParam + sizeof(int));

    if (dataLen != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGE("DLCustHCFParamToNV data miss !!");
        free(pHCFPara);
        pHCFPara = NULL;
        return ACHFailed;
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)      
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received  - loudness hsf coeffs bes_loudness_hsf_coeff[0][0][%d] = %d", i, pCustomPara->bes_loudness_hsf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pCustomPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness lpf coeffs bes_loudness_lpf_coeff[0][%d]=%d", i, pCustomPara->bes_loudness_lpf_coeff[0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Received - HCF Param bes_loudness_WS_Gain_Max = %d", pCustomPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Received - HCF Param bes_loudness_WS_Gain_Min = %d", pCustomPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Received - HCF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    SXLOGD("Received - HCF Param bes_loudness_Att_Time = %d", pCustomPara->bes_loudness_Att_Time);
    SXLOGD("Received - HCF Param bes_loudness_Rel_Time = %d", pCustomPara->bes_loudness_Rel_Time);

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_Out[i]);
    }
#else
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 4 ; j++)
        {
            SXLOGV("Received  - loudness hsf coeffs bes_loudness_hsf_coeff[%d][%d] = %d", i, j, pCustomPara->bes_loudness_hsf_coeff[i][j]);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Received - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pCustomPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    SXLOGV("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGV("Received - HCF Param bes_loudness_DRC_Forget_Table[0][0] = %d", pCustomPara->bes_loudness_DRC_Forget_Table[0][0]);
    SXLOGV("Received - HCF Param bes_loudness_WS_Gain_Max = %d", pCustomPara->bes_loudness_WS_Gain_Max);
    SXLOGV("Received - HCF Param bes_loudness_WS_Gain_Min = %d", pCustomPara->bes_loudness_WS_Gain_Min);
    SXLOGV("Received - HCF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    SXLOGV("Received - HCF Param bes_loudness_Filter_First = %d", pCustomPara->bes_loudness_Filter_First);
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Received - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pCustomPara->bes_loudness_Gain_Map_Out[i]);
    }
#endif
    memcpy((void *)pHCFPara, (void *)pCustomPara, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    write_size = SetAudioCompFltCustParamToNV(AUDIO_COMP_FLT_HEADPHONE, pHCFPara);
    if (write_size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("DLCustHCFParamToNV Down load to NVRAM fail, structure size=%d,writed_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), write_size);
        free(pHCFPara);
        pHCFPara = NULL;
        return ACHFailed;
    }

    AudioSystem::setParameters(0, String8("UpdateACFHCFParameters=1"));

    free(pHCFPara);
    pHCFPara = NULL;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::ULCustHCFParamFromNV(void *pParam, int *len)
{
    int size = 0;

    SXLOGD("AudioCmdHandler::ULCustHCFParamFromNV() in");

    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pCustomPara = NULL;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT *pHCFPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)malloc(sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    if (pHCFPara == NULL)
    {
        return ACHFailed;
    }

    SXLOGD("ULCustHCFParamFromNV ,acf param size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    size = GetAudioCompFltCustParamFromNV(AUDIO_COMP_FLT_HEADPHONE, pHCFPara);
    if (size != sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT))
    {
        SXLOGD("ULCustHCFParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT), size);
        free(pHCFPara);
        pHCFPara = NULL;
        return ACHFailed;
    }
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)  

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori  - loudness hsf coeffs bes_loudness_hsf_coeff[0][0][%d] = %d", i, pHCFPara->bes_loudness_hsf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pHCFPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness lpf coeffs bes_loudness_lpf_coeff[0][%d]=%d", i, pHCFPara->bes_loudness_lpf_coeff[0][i]);
    }

    SXLOGD("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGD("Ori - HCF Param bes_loudness_WS_Gain_Max = %d", pHCFPara->bes_loudness_WS_Gain_Max);
    SXLOGD("Ori - HCF Param bes_loudness_WS_Gain_Min = %d", pHCFPara->bes_loudness_WS_Gain_Min);
    SXLOGD("Ori - HCF Param bes_loudness_Filter_First = %d", pHCFPara->bes_loudness_Filter_First);
    SXLOGD("Ori - HCF Param bes_loudness_Att_Time = %d", pHCFPara->bes_loudness_Att_Time);
    SXLOGD("Ori - HCF Param bes_loudness_Rel_Time = %d", pHCFPara->bes_loudness_Rel_Time);

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pHCFPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pHCFPara->bes_loudness_Gain_Map_Out[i]);
    }
#else
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 4 ; j++)
        {
            SXLOGV("Ori  - loudness hsf coeffs bes_loudness_hsf_coeff[%d][%d] = %d", i, j, pHCFPara->bes_loudness_hsf_coeff[i][j]);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        SXLOGV("Ori - loudness bpf coeffs bes_loudness_bpf_coeff[0][0][%d]=%d", i, pHCFPara->bes_loudness_bpf_coeff[0][0][i]);
    }

    SXLOGV("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    SXLOGV("Ori - HCF Param bes_loudness_DRC_Forget_Table[0][0] = %d", pHCFPara->bes_loudness_DRC_Forget_Table[0][0]);
    SXLOGV("Ori - HCF Param bes_loudness_WS_Gain_Max = %d", pHCFPara->bes_loudness_WS_Gain_Max);
    SXLOGV("Ori - HCF Param bes_loudness_WS_Gain_Min = %d", pHCFPara->bes_loudness_WS_Gain_Min);
    SXLOGV("Ori - HCF Param bes_loudness_Filter_First = %d", pHCFPara->bes_loudness_Filter_First);
    SXLOGV("Ori - HCF Param bes_loudness_Filter_First = %d", pHCFPara->bes_loudness_Filter_First);
    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map in bes_loudness_Gain_Map_In[%d]=%d", i, pHCFPara->bes_loudness_Gain_Map_In[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        SXLOGV("Ori - gain map out bes_loudness_Gain_Map_Out[%d]=%d", i, pHCFPara->bes_loudness_Gain_Map_Out[i]);
    }
#endif
    *((int *)pParam) = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
    pCustomPara = (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)(pParam + sizeof(int));

    memcpy((void *)pCustomPara, (void *)pHCFPara, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));

    dataEncode((char *)pParam, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int));
    *len = 2 * (sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT) + sizeof(int));

    free(pHCFPara);
    pHCFPara = NULL;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::AudioTasteRunning(unsigned short cmdType, void *pParam)
{
    SXLOGD("AudioCmdHandler::AudioTasteRunning() in");

    int size = 0;
    int dataLen = 0;
    status_t ret = NO_ERROR;
    AUD_TASTE_PARAM_STRUCT sAudioTAsteParam;;

    if (cmdType)
    {
        size = sizeof(AUD_TASTE_PARAM_STRUCT) + sizeof(int);
        dataDecode((char *)pParam, size);

        dataLen = *((int *)pParam);
        size = 5 * sizeof(unsigned short) + strlen((char *)(pParam + 7 * sizeof(unsigned short))) + 1;
        SXLOGD("AudioTasteRunning() received data size is dataLen=%d, ori size is %d", dataLen, size);
        if (dataLen != size || (size > (5 * sizeof(unsigned short) + MAX_FILE_NAME_LEN)))
        {
            SXLOGE("AudioTasteRunning data miss !! dataLen=%d, size=%d", dataLen, size);
            return ACHFailed;
        }
        sAudioTAsteParam.cmdType = cmdType;
        memcpy((void *)&sAudioTAsteParam.selected_fir_index, pParam + sizeof(int), size);

        SXLOGD("AudioTasteRunning - cmd type=%d, mode=%d, wb_mode=%d", cmdType, sAudioTAsteParam.phone_mode, sAudioTAsteParam.wb_mode);
        SXLOGD("AudioTasteRunning - index=%d, dl DG gain=%d, dl PGA=%d", sAudioTAsteParam.selected_fir_index, sAudioTAsteParam.dlDGGain, sAudioTAsteParam.dlPGA);
        SXLOGD("AudioTasteRunning - file name: %s", sAudioTAsteParam.audio_file);
        ret = AudioSystem::SetAudioData(AUD_TASTE_TUNING_CMD, sizeof(AUD_TASTE_PARAM_STRUCT), (void *)&sAudioTAsteParam);
        if (ret != NO_ERROR)
        {
            return ACHFailed;
        }
    }
    else
    {
        sAudioTAsteParam.cmdType = 0;
        ret = AudioSystem::SetAudioData(AUD_TASTE_TUNING_CMD, sizeof(AUD_TASTE_PARAM_STRUCT), (void *)&sAudioTAsteParam);
        if (ret != NO_ERROR)
        {
            return ACHFailed;
        }
    }

    return ACHSucceeded;
}


ACHStatus AudioCmdHandler::DLCustDualMicParamToNV(void *pParam, int block)
{
    // block = 0: for ABF_para[NUM_ABF_PARAM + NUM_ABFWB_PARAM]
    // block = 1: for remains
    SXLOGD("AudioCmdHandler::DLCustDualMicParamToNV() in");
    int size = 0;
    int dataLen = 0;
#ifdef MTK_DUAL_MIC_SUPPORT
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pDualMicPara = (AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    if (pDualMicPara == NULL || pParam == NULL)
    {
        if (pDualMicPara) { free(pDualMicPara); }
        return ACHFailed;
    }

    SXLOGD("DLCustDualMicParamToNV ,dual mic param size=%d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    size = GetDualMicSpeechParamFromNVRam(pDualMicPara);
    if (size != sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT))
    {
        SXLOGD("DLCustDualMicParamToNV Upload from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT), size);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        size = sizeof(pDualMicPara->ABF_para) + sizeof(int);
        dataDecode((char *)pParam, size);

        dataLen = *((int *)pParam);
        size -= sizeof(int);
        if (dataLen != size)
        {
            SXLOGE("DLCustDualMicParamToNV ABF_para data miss !! dataLen=%d, size=%d", dataLen, size);
            if (pDualMicPara) { free(pDualMicPara); }
            return ACHFailed;
        }

        memcpy((void *)pDualMicPara->ABF_para, pParam + sizeof(int), size);

        for (int i = 0; i < NUM_ABF_PARAM + NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF[%d]=%d", i, pDualMicPara->ABF_para[i]);
        }
    }
    else if (block == 1)
    {
#ifdef MTK_HANDSFREE_DMNR_SUPPORT
        size = sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT) - sizeof(pDualMicPara->ABF_para) + sizeof(int);
        dataDecode((char *)pParam, size);

        dataLen = *((int *)pParam);
        size -= sizeof(int);
        if (dataLen != size)
        {
            SXLOGE("DLCustDualMicParamToNV ABF_para data miss !! dataLen=%d, size=%d", dataLen, size);
            if (pDualMicPara) { free(pDualMicPara); }
            return ACHFailed;
        }

        memcpy((void *)pDualMicPara->ABF_para_LoudSPK, pParam + sizeof(int), size);

        for (int i = 0; i < NUM_ABF_PARAM + NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_LoudSPK[%d]=%d", i, pDualMicPara->ABF_para_LoudSPK[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VR[%d]=%d", i, pDualMicPara->ABF_para_VR[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VOIP[%d]=%d", i, pDualMicPara->ABF_para_VOIP[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VOIP_LoudSPK[%d]=%d", i, pDualMicPara->ABF_para_VOIP_LoudSPK[i]);
        }
#else
        SXLOGW("do not support DMNR3.0, block:%d", block);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
#endif
    }
    else
    {
        SXLOGW("command parameters error block:%d", block);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
    }

    size = SetDualMicSpeechParamToNVRam(pDualMicPara);
    if (size != sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT))
    {
        SXLOGD("DLCustDualMicParamToNV down load to NVRAM fail, structure size=%d,write_size=%d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT), size);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
    }
    AudioSystem::setParameters(0, String8("UpdateDualMicParameters=1"));

    if (pDualMicPara) { free(pDualMicPara); }
    return ACHSucceeded;
#else
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::ULCustDualMicParamFromNV(void *pParam, int *len, int block)
{
    SXLOGD("AudioCmdHandler::ULCustDualMicParamFromNV() in");
    int size = 0;

    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pDualMicPara = (AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)malloc(sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    if (pDualMicPara == NULL || pParam == NULL)
    {
        if (pDualMicPara != NULL)
        {
            free(pDualMicPara);
        }
        return ACHFailed;
    }

    SXLOGD("dual mic param size=%d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
    size = GetDualMicSpeechParamFromNVRam(pDualMicPara);
    if (size != sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT))
    {
        SXLOGD("Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT), size);
        free(pDualMicPara);
        pDualMicPara = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        for (int i = 0; i < NUM_ABF_PARAM + NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF[%d]=%d", i, pDualMicPara->ABF_para[i]);
        }

        *((int *)pParam) = sizeof(pDualMicPara->ABF_para);
        memcpy(pParam + sizeof(int), (void *)pDualMicPara->ABF_para, sizeof(pDualMicPara->ABF_para));

        dataEncode((char *)pParam, sizeof(pDualMicPara->ABF_para) + sizeof(int));
        *len = 2 * (sizeof(pDualMicPara->ABF_para) + sizeof(int));
    }
    else if (block == 1)
    {
#ifdef MTK_HANDSFREE_DMNR_SUPPORT
        for (int i = 0; i < NUM_ABF_PARAM + NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_LoudSPK[%d]=%d", i, pDualMicPara->ABF_para_LoudSPK[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VR[%d]=%d", i, pDualMicPara->ABF_para_VR[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VOIP[%d]=%d", i, pDualMicPara->ABF_para_VOIP[i]);
        }

        for (int i = 0; i < NUM_ABFWB_PARAM; i++)
        {
            SXLOGV("Dual Mic parameters ABF_para_VOIP_LoudSPK[%d]=%d", i, pDualMicPara->ABF_para_VOIP_LoudSPK[i]);
        }

        size = sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT) - sizeof(pDualMicPara->ABF_para);
        *((int *)pParam) = size;
        memcpy(pParam + sizeof(int), (void *)pDualMicPara->ABF_para_LoudSPK, size);

        dataEncode((char *)pParam, size + sizeof(int));
        *len = 2 * (size + sizeof(int));
#else
        SXLOGW("do not support DMNR3.0, block:%d", block);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
#endif
    }
    else
    {
        SXLOGW("command parameters error block:%d", block);
        if (pDualMicPara) { free(pDualMicPara); }
        pDualMicPara = NULL;
        return ACHFailed;
    }

    if (pDualMicPara) { free(pDualMicPara); }
    pDualMicPara = NULL;
    return ACHSucceeded;
}

//for DMNR tuning
ACHStatus AudioCmdHandler::AudioDMNRTuning(unsigned short cmdType, bool bWB, void *pParam)
{
    SXLOGD("AudioCmdHandler::AudioDMNRTuning() in");
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    int size = 0;
    int dataLen = 0;
    char keyInputFileName[MAX_FILE_NAME_LEN];
    char keyOutputFileName[MAX_FILE_NAME_LEN];
    char keyCmdType[MAX_FILE_NAME_LEN];
    DMNRTuningFileName *pFileName = NULL;

    switch (cmdType)
    {
        case DUAL_MIC_REC_PLAY_STOP:
            sprintf(keyCmdType, "DUAL_MIC_REC_PLAY=%d", cmdType);
            AudioSystem::setParameters(0, String8(keyCmdType));
            break;
        case DUAL_MIC_REC:
        case DUAL_MIC_REC_HF:
        {
            size = MAX_FILE_NAME_LEN + sizeof(int);
            dataDecode((char *)pParam, size);

            dataLen = *((int *)pParam);
            size = strlen((char *)(pParam + sizeof(int))) + 1;
            if (dataLen != size)
            {
                SXLOGE("AudioDMNRTuning data miss !! dataLen=%d, size=%d", dataLen, size);
                return ACHFailed;
            }

            cmdType = bWB ? (cmdType | 0x10) : (cmdType);
            sprintf(keyCmdType, "DUAL_MIC_REC_PLAY=%d", cmdType);
            sprintf(keyOutputFileName, "DUAL_MIC_OUT_FILE_NAME=%s", (char *)(pParam + sizeof(int)));

            AudioSystem::setParameters(0, String8(keyOutputFileName));
            AudioSystem::setParameters(0, String8(keyCmdType));
            break;
        }
        case DUAL_MIC_REC_PLAY:
        case DUAL_MIC_REC_PLAY_HS:
        case DUAL_MIC_REC_PLAY_HF:
        case DUAL_MIC_REC_PLAY_HS_HF:
        {
            size = 2 * MAX_FILE_NAME_LEN + sizeof(int);
            dataDecode((char *)pParam, size);

            dataLen = *((int *)pParam);
            pFileName = (DMNRTuningFileName *)(pParam + sizeof(int));
            size = strlen(pFileName->input_file) + strlen(pFileName->output_file) + 2;
            if (dataLen != size)
            {
                SXLOGE("AudioDMNRTuning data miss !! dataLen=%d, size=%d", dataLen, size);
                return ACHFailed;
            }

            cmdType = bWB ? (cmdType | 0x10) : (cmdType);
            sprintf(keyCmdType, "DUAL_MIC_REC_PLAY=%d", cmdType);
            sprintf(keyInputFileName, "DUAL_MIC_IN_FILE_NAME=%s", pFileName->input_file);
            sprintf(keyOutputFileName, "DUAL_MIC_OUT_FILE_NAME=%s", pFileName->output_file);

            AudioSystem::setParameters(0, String8(keyInputFileName));
            AudioSystem::setParameters(0, String8(keyOutputFileName));
            AudioSystem::setParameters(0, String8(keyCmdType));
            break;
        }
        default:
            SXLOGD("AudioCmdHandler::AudioDMNRTuning() cmdType error, cmdType=%d", cmdType);
            return ACHFailed;
            break;
    }

    return ACHSucceeded;
#else
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::getDMNRGain(unsigned short cmdType, void *pParam, int *len)
{
    SXLOGD("AudioCmdHandler::getDMNRGain() in");
    unsigned short gain = 0;
    char gainStr[MAX_FILE_NAME_LEN];
    char *pParamValue = NULL;
    char *pParamName = NULL;
    String8 mValue;

    if (cmdType < 0 || cmdType >= DUAL_MIC_GAIN_CNT)
    {
        return ACHFailed;
    }

    sprintf(gainStr, "DUAL_MIC_GET_GAIN=%d", cmdType);
    mValue = AudioSystem::getParameters(0, String8(gainStr));

    SXLOGD("getDMNRGain the parameters is %s", mValue.string());
    strcpy(gainStr, mValue.string());
    pParamValue = gainStr;
    pParamName = strsep(&pParamValue, "=");
    if (NULL == pParamName || NULL == pParamValue)
    {
        return ACHFailed;
    }

    gain = atoi(pParamValue);
    SXLOGD("getDMNRGain ori str:%s, new: %s, gain=%d", gainStr, pParamName, gain);

    *((int *)pParam) = sizeof(unsigned short);
    *((unsigned short *)(pParam + sizeof(int))) = gain;

    dataEncode((char *)pParam, sizeof(unsigned short) + sizeof(int));
    *len = 2 * (sizeof(unsigned short) + sizeof(int));

    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::setDMNRGain(unsigned short cmdType, unsigned short gain)
{
    SXLOGD("AudioCmdHandler::setDMNRGain() in");
    SXLOGD("setDMNRGain() cmd type=%d, gain=%d", cmdType, gain);
    char gainStr[MAX_FILE_NAME_LEN];

    if (cmdType < 0 || cmdType >= DUAL_MIC_GAIN_CNT)
    {
        return ACHFailed;
    }

    switch (cmdType)
    {
        case DUAL_MIC_UL_GAIN:
            sprintf(gainStr, "DUAL_MIC_SET_UL_GAIN=%d", gain);
            break;
        case DUAL_MIC_DL_GAIN:
            sprintf(gainStr, "DUAL_MIC_SET_DL_GAIN=%d", gain);
            break;
        case DUAL_MIC_HSDL_GAIN:
            sprintf(gainStr, "DUAL_MIC_SET_HSDL_GAIN=%d", gain);
            break;
        case DUAL_MIC_UL_GAIN_HF:
            sprintf(gainStr, "DUAL_MIC_SET_UL_GAIN_HF=%d", gain);
            break;
        default:
            break;
    }

    AudioSystem::setParameters(0, String8(gainStr));
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::DLCustHDRecParamToNV(void *pParam, int block)
{
    SXLOGD("DLCustHDRecParamToNV() in");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    int dataLen = 0;
    AUDIO_HD_RECORD_PARAM_STRUCT *pHDRecParam = (AUDIO_HD_RECORD_PARAM_STRUCT *)malloc(sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));

    size = GetHdRecordParamFromNV(pHDRecParam);
    if (size != sizeof(AUDIO_HD_RECORD_PARAM_STRUCT))
    {
        SXLOGD("DLCustHDRecParamToNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT), size);
        if (pHDRecParam) { free(pHDRecParam); }
        pHDRecParam = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        size = 2 * sizeof(pHDRecParam->hd_rec_mode_num) + sizeof(pHDRecParam->hd_rec_speech_mode_para);
        dataDecode((char *)pParam, size + sizeof(int));
        dataLen = *((int *)pParam);
        if (dataLen != size)
        {
            SXLOGE("DLCustHDRecParamToNV part1 data miss !! dataLen=%d, size=%d", dataLen, size);
            if (pHDRecParam) { free(pHDRecParam); }
            pHDRecParam = NULL;
            return ACHFailed;
        }

        memcpy((void *)(&pHDRecParam->hd_rec_mode_num), pParam + sizeof(int), size);

        SXLOGD("HD Record parameters part 1 ~~~~~~~~~~~~~~~~");
    }
    else if (block == 1)
    {
        SXLOGD("HD Record parameters part 2 ~~~~~~~~~~~~~~~~");
#if 0
        size = sizeof(AUDIO_HD_RECORD_PARAM_STRUCT) - 2 * sizeof(pHDRecParam->hd_rec_mode_num) - sizeof(pHDRecParam->hd_rec_speech_mode_para);
#else
        size = sizeof(pHDRecParam->hd_rec_fir) / 2;
#endif

        dataDecode((char *)pParam, size + sizeof(int));
        dataLen = *((int *)pParam);
        if (dataLen != size)
        {
            SXLOGE("DLCustHDRecParamToNV part2 data miss !! dataLen=%d, size=%d", dataLen, size);
            if (pHDRecParam) { free(pHDRecParam); }
            pHDRecParam = NULL;
            return ACHFailed;
        }

        memcpy((void *)pHDRecParam->hd_rec_fir, pParam + sizeof(int), size);
    }
    else if (block == 2)
    {
#if 0
        size = 0;
#else
        SXLOGD("HD Record parameters part 3 ~~~~~~~~~~~~~~~~");
        size = sizeof(AUDIO_HD_RECORD_PARAM_STRUCT) - 2 * sizeof(pHDRecParam->hd_rec_mode_num) - sizeof(pHDRecParam->hd_rec_speech_mode_para) - sizeof(pHDRecParam->hd_rec_fir) / 2;

        dataDecode((char *)pParam, size + sizeof(int));
        dataLen = *((int *)pParam);
        if (dataLen != size)
        {
            SXLOGE("DLCustHDRecParamToNV part2 data miss !! dataLen=%d, size=%d", dataLen, size);
            if (pHDRecParam) { free(pHDRecParam); }
            pHDRecParam = NULL;
            return ACHFailed;
        }

        memcpy((void *)pHDRecParam->hd_rec_fir[SPC_MAX_NUM_RECORD_INPUT_FIR / 2], pParam + sizeof(int), size);
#endif
    }

    size = SetHdRecordParamToNV(pHDRecParam);
    if (size != sizeof(AUDIO_HD_RECORD_PARAM_STRUCT))
    {
        SXLOGD("DLCustHDRecParamToNV down load to NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT), size);
        if (pHDRecParam) { free(pHDRecParam); }
        pHDRecParam = NULL;
        return ACHFailed;
    }

    SXLOGD("HD Record Parameters ~~~~~~~~~~~~~~~~");
    SXLOGD("mode num=%d, fir num=%d", pHDRecParam->hd_rec_mode_num, pHDRecParam->hd_rec_fir_num);
    SXLOGD("speenc mode hd_rec_speech_mode_para[0][0]=%d, hd_rec_speech_mode_para[end][end]=%d", pHDRecParam->hd_rec_speech_mode_para[0][0], pHDRecParam->hd_rec_speech_mode_para[SPC_MAX_NUM_RECORD_SPH_MODE - 1][SPEECH_PARA_NUM - 1]);
    SXLOGD("speenc fir hd_rec_fir[0][0]=%d, hd_rec_fir[end][end]=%d", pHDRecParam->hd_rec_fir[0][0], pHDRecParam->hd_rec_fir[SPC_MAX_NUM_RECORD_INPUT_FIR - 1][WB_FIR_NUM - 1]);

    for (int i = 0; i < SPC_MAX_NUM_RECORD_SPH_MODE; i++)
    {
        SXLOGD("hd_rec_map_to_fir_for_ch1[%d]=%d", i, pHDRecParam->hd_rec_map_to_fir_for_ch1[i]);
        SXLOGD("hd_rec_map_to_fir_for_ch2[%d]=%d", i, pHDRecParam->hd_rec_map_to_fir_for_ch2[i]);
        SXLOGD("hd_rec_map_to_dev_mode[%d]=%d", i, pHDRecParam->hd_rec_map_to_dev_mode[i]);
        SXLOGD("hd_rec_map_to_input_src[%d]=%d", i, pHDRecParam->hd_rec_map_to_input_src[i]);
        SXLOGD("hd_rec_map_to_stereo_flag[%d]=%d", i, pHDRecParam->hd_rec_map_to_stereo_flag[i]);
    }

    if (pHDRecParam) { free(pHDRecParam); }
    pHDRecParam = NULL;
    return ACHSucceeded;
#else
    SXLOGW("DLCustHDRecParamToNV-do not support HD record!!");
    return ACHFailed;
#endif
}


ACHStatus AudioCmdHandler::ULCustHDRecParamFromNV(void *pParam, int *len, int block)
{
    SXLOGD("ULCustHDRecParamFromNV() in");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    AUDIO_HD_RECORD_PARAM_STRUCT *pHDRecCustParam = (AUDIO_HD_RECORD_PARAM_STRUCT *)(pParam + sizeof(int));
    AUDIO_HD_RECORD_PARAM_STRUCT *pHDRecParam = (AUDIO_HD_RECORD_PARAM_STRUCT *)malloc(sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));

    SXLOGD("ULCustHDRecParamFromNV ,HD record parameters size=%d", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
    size = GetHdRecordParamFromNV(pHDRecParam);
    if (size != sizeof(AUDIO_HD_RECORD_PARAM_STRUCT))
    {
        SXLOGD("ULCustHDRecParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_HD_RECORD_PARAM_STRUCT), size);
        if (pHDRecParam) { free(pHDRecParam); }
        pHDRecParam = NULL;
        return ACHFailed;
    }

    if (block == 0)
    {
        pHDRecCustParam->hd_rec_mode_num = pHDRecParam->hd_rec_mode_num;
        pHDRecCustParam->hd_rec_fir_num  = pHDRecParam->hd_rec_fir_num;
        memcpy((void *)pHDRecCustParam->hd_rec_speech_mode_para, (void *)pHDRecParam->hd_rec_speech_mode_para, sizeof(pHDRecParam->hd_rec_speech_mode_para));

        size = 2 * sizeof(pHDRecParam->hd_rec_mode_num) + sizeof(pHDRecParam->hd_rec_speech_mode_para);

        SXLOGD("HD Record parameters part 1 ~~~~~~~~~~~~~~~~");
        SXLOGD("audio record mode num=%d, fir num=%d", pHDRecCustParam->hd_rec_mode_num, pHDRecCustParam->hd_rec_fir_num);
        for (int i = 0; i < SPC_MAX_NUM_RECORD_SPH_MODE; i++)
        {
            for (int j = 0; j < SPEECH_PARA_NUM; j++)
            {
                SXLOGD("HD record speech mode parameters[%d][%d]=%d", i, j, pHDRecCustParam->hd_rec_speech_mode_para[i][j]);
            }
        }
    }
    else if (block == 1)
    {
        SXLOGD("HD Record parameters part 2 ~~~~~~~~~~~~~~~~");
#if 0
        size = sizeof(AUDIO_HD_RECORD_PARAM_STRUCT) - 2 * sizeof(pHDRecParam->hd_rec_mode_num) - sizeof(pHDRecParam->hd_rec_speech_mode_para);
#else
        size = sizeof(pHDRecParam->hd_rec_fir) / 2;
#endif
        memcpy(pParam + sizeof(int), (void *)pHDRecParam->hd_rec_fir, size);
    }
    else if (block == 2)
    {
#if 0
        size = 0;
#else
        SXLOGD("HD Record parameters part 3 ~~~~~~~~~~~~~~~~");
        size = sizeof(AUDIO_HD_RECORD_PARAM_STRUCT) - 2 * sizeof(pHDRecParam->hd_rec_mode_num) - sizeof(pHDRecParam->hd_rec_speech_mode_para) - sizeof(pHDRecParam->hd_rec_fir) / 2;
#endif
        memcpy(pParam + sizeof(int), (void *)pHDRecParam->hd_rec_fir[SPC_MAX_NUM_RECORD_INPUT_FIR / 2], size);
    }
    else
    {
        size = 0;
    }

    *((int *)pParam) = size;

    dataEncode((char *)pParam, size + sizeof(int));
    *len = 2 * (size + sizeof(int));

    if (pHDRecParam) { free(pHDRecParam); }
    pHDRecParam = NULL;
    return ACHSucceeded;
#else
    SXLOGW("ULCustHDRecParamFromNV-do not support HD record!!");
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::DLCustHDRecSceTableToNV(void *pParam)
{
    SXLOGD("DLCustHDRecSceTableToNV() in");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    int dataLen = 0;

    SXLOGD("DLCustHDRecSceTableToNV ,HD record scene table size=%d", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    size = sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT) + sizeof(int);
    dataDecode((char *)pParam, size);

    dataLen = *((int *)pParam);
    if (dataLen != sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT))
    {
        SXLOGE("DLCustHDRecSceTableToNV data miss !! dataLen=%d, size=%d", dataLen, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        return ACHFailed;
    }

    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pHDRecPara = (AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *)(pParam + sizeof(int));
    size = SetHdRecordSceneTableToNV(pHDRecPara);
    if (size != sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT))
    {
        SXLOGD("DLCustHDRecSceTableToNV down load to NVRAM fail, structure size=%d,write_size=%d", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT), size);
        return ACHFailed;
    }

    SXLOGD("HD Record Scene Table ~~~~~~~~~~~~~~~~");
    SXLOGD("audio record scene num=%d, video record scene num=%d", pHDRecPara->num_voice_rec_scenes, pHDRecPara->num_video_rec_scenes);
    for (int i = 0; i < MAX_HD_REC_SCENES; i++)
    {
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++)
        {
            SXLOGV("scene_table[%d][%d]=%d", i, j, pHDRecPara->scene_table[i][j]);
        }
    }

    return ACHSucceeded;
#else
    SXLOGW("DLCustHDRecSceTableToNV-do not support HD record!!");
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::ULCustHDRecSceTableFromNV(void *pParam, int *len)
{
    SXLOGD("ULCustHDRecSceTableFromNV() in");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pHDRecPara = (AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *)(pParam + sizeof(int));

    SXLOGD("ULCustHDRecSceTableFromNV ,HD record scene table size=%d", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
    size = GetHdRecordSceneTableFromNV(pHDRecPara);
    if (size != sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT))
    {
        SXLOGD("ULCustHDRecSceTableFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT), size);
        return ACHFailed;
    }

    SXLOGD("HD Record Scene Table ~~~~~~~~~~~~~~~~");
    SXLOGD("audio record scene num=%d, video record scene num=%d", pHDRecPara->num_voice_rec_scenes, pHDRecPara->num_video_rec_scenes);
    for (int i = 0; i < MAX_HD_REC_SCENES; i++)
    {
        for (int j = 0; j < NUM_HD_REC_DEVICE_SOURCE; j++)
        {
            SXLOGV("scene_table[%d][%d]=%d", i, j, pHDRecPara->scene_table[i][j]);
        }
    }

    *((int *)pParam) = sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT);

    dataEncode((char *)pParam, sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT) + sizeof(int));
    *len = 2 * (sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT) + sizeof(int));

    return ACHSucceeded;
#else
    SXLOGW("ULCustHDRecSceTableFromNV-do not support HD record!!");
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::DLCustHDRecHSParamToNV(void *pParam)
{
    SXLOGD("DLCustHDRecHSParamToNV() in +");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    int dataLen = 0;

    SXLOGD("DLCustHDRecHSParamToNV ,48k HD record parameters size=%d", sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
    size = sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT) + sizeof(int);
    dataDecode((char *)pParam, size);

    dataLen = *((int *)pParam);
    if (dataLen != sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT))
    {
        SXLOGE("DLCustHDRecHSParamToNV data miss !! dataLen=%d, size=%d", dataLen, sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
        return ACHFailed;
    }

    AUDIO_HD_RECORD_48K_PARAM_STRUCT *pHDRecPara = (AUDIO_HD_RECORD_48K_PARAM_STRUCT *)(pParam + sizeof(int));
    size = SetHdRecord48kParamToNV(pHDRecPara);
    if (size != sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT))
    {
        SXLOGD("DLCustHDRecHSParamToNV down load to NVRAM fail, structure size=%d,write_size=%d", sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT), size);
        return ACHFailed;
    }

    for (int i = 0; i < SPC_MAX_NUM_48K_RECORD_INPUT_FIR; i++)
    {
        for (int j = 0; j < WB_FIR_NUM; j++)
        {
            SXLOGV("48k sample rate parameters hd_rec_fir[%d][%d]=%d", i, j, pHDRecPara->hd_rec_fir[i][j]);
        }
    }

    return ACHSucceeded;
#else
    SXLOGW("DLCustHDRecHSParamToNV-do not support HD record!!");
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::ULCustHDRecHSParamFromNV(void *pParam, int *len)
{
    SXLOGD("ULCustHDRecHSParamFromNV() in +");
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    AUDIO_HD_RECORD_48K_PARAM_STRUCT *pHDRecPara = (AUDIO_HD_RECORD_48K_PARAM_STRUCT *)(pParam + sizeof(int));

    SXLOGD("ULCustHDRecHSParamFromNV ,HD record scene table size=%d", sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT));
    size = GetHdRecord48kParamFromNV(pHDRecPara);
    if (size != sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT))
    {
        SXLOGD("ULCustHDRecHSParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT), size);
        return ACHFailed;
    }

    for (int i = 0; i < SPC_MAX_NUM_48K_RECORD_INPUT_FIR; i++)
    {
        for (int j = 0; j < WB_FIR_NUM; j++)
        {
            SXLOGV("48k sample rate parameters hd_rec_fir[%d][%d]=%d", i, j, pHDRecPara->hd_rec_fir[i][j]);
        }
    }

    *((int *)pParam) = sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT);

    dataEncode((char *)pParam, sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT) + sizeof(int));
    *len = 2 * (sizeof(AUDIO_HD_RECORD_48K_PARAM_STRUCT) + sizeof(int));

    return ACHSucceeded;
#else
    SXLOGW("ULCustHDRecHSParamFromNV-do not support HD record!!");
    return ACHFailed;
#endif
}

ACHStatus AudioCmdHandler::HDRecording(int enable, void *pParam)
{
    SXLOGD("HDRecording() in +");
    int dataLen = 0;
    ACHStatus ret = ACHSucceeded;
    int size = MAX_FILE_NAME_LEN + sizeof(int);
    char outputFileName[MAX_FILE_NAME_LEN];
    String8 mValue;

    if (enable)
    {
        dataDecode((char *)pParam, size);
        dataLen = *((int *)pParam);
        size = strlen((char *)(pParam + sizeof(int))) + 1;
        if (dataLen != size)
        {
            SXLOGE("HDRecording data miss !! dataLen=%d, size=%d", dataLen, size);
            return ACHFailed;
        }

        sprintf(outputFileName, "HDRecVMFileName=%s", (char *)(pParam + sizeof(int)));
        AudioSystem::setParameters(0, String8(outputFileName));
        SXLOGD("HDRecording output file name:%s", outputFileName);

        enableHALHDRecTunning(true);
        ret = startRecorder("/sdcard/HDRecordTemp.wav", AUDIO_SOURCE_MIC);
        if (ret != ACHSucceeded)
        {
            enableHALHDRecTunning(false);
        }
    }
    else
    {
        enableHALHDRecTunning(false);
        ret = stopRecorder();
    }

    return ret;
}

ACHStatus AudioCmdHandler::getPhoneSupportInfo(unsigned int *supportInfo)
{
    SXLOGD("AudioCmdHandler::getPhoneSupportInfo() in");
    *supportInfo = (unsigned int)QueryFeatureSupportInfo();

    SXLOGD("AudioCmdHandler::getPhoneSupportInfo() supportInfo=0x%x", *supportInfo);
    return ACHSucceeded;
}

void AudioCmdHandler::enableHALHDRecTunning(bool enbale)
{
    SXLOGD("enableHALHDRecTunning in");
    if (enbale)
    {
        AudioSystem::setParameters(0, String8("HDRecTunningEnable=1"));
        m_bHDRecTunning = true;
    }
    else if (m_bHDRecTunning)
    {
        AudioSystem::setParameters(0, String8("HDRecTunningEnable=0"));
        m_bHDRecTunning = false;
    }
}

ACHStatus AudioCmdHandler::DLCustVOIPParamToNV(void *pParam)
{
    SXLOGD("DLCustVOIPParamToNV in");

    if (pParam == NULL)
    {
        return ACHFailed;
    }

    int size = 0;
    int dataLen = 0;
    uint32_t supportInfo = QueryFeatureSupportInfo();

    if (supportInfo & 0x40)
    {
        SXLOGD("DLCustVOIPParamToNV ,VOIP parameters size=%d", sizeof(AUDIO_VOIP_PARAM_STRUCT));
        size = sizeof(AUDIO_VOIP_PARAM_STRUCT) + sizeof(int);
        dataDecode((char *)pParam, size);

        dataLen = *((int *)pParam);
        if (dataLen != sizeof(AUDIO_VOIP_PARAM_STRUCT))
        {
            SXLOGE("DLCustVOIPParamToNV data miss !! dataLen=%d, size=%d", dataLen, sizeof(AUDIO_VOIP_PARAM_STRUCT));
            return ACHFailed;
        }

        AUDIO_VOIP_PARAM_STRUCT *pVOIPPara = (AUDIO_VOIP_PARAM_STRUCT *)(pParam + sizeof(int));
        size = SetAudioVoIPParamToNV(pVOIPPara);
        if (size != sizeof(AUDIO_VOIP_PARAM_STRUCT))
        {
            SXLOGD("DLCustVOIPParamToNV down load to NVRAM fail, structure size=%d,write_size=%d", sizeof(AUDIO_VOIP_PARAM_STRUCT), size);
            return ACHFailed;
        }

        for (int i = 0; i < SPEECH_COMMON_NUM; i++)
        {
            SXLOGV("VOIP parameters speech_common_para[%d]=%d", i, pVOIPPara->speech_common_para[i]);
        }

        for (int i = 0; i < VOIP_INDEX_NUM; i++)
        {
            for (int j = 0; j < SPEECH_PARA_NUM; j++)
            {
                SXLOGV("VOIP parameters speech_mode_para[%d][%d]=%d", i, j, pVOIPPara->speech_mode_para[i][j]);
            }
        }

        for (int i = 0; i < WB_FIR_NUM; i++)
        {
            SXLOGV("VOIP parameters in_fir[0][%d]=%d", i, pVOIPPara->in_fir[0][i]);
            SXLOGV("VOIP parameters out_fir[0][%d]=%d", i, pVOIPPara->out_fir[0][i]);
        }
    }
    else 
    {
        SXLOGW("DLCustVOIPParamToNV - do not support VOIP enhance!!");
        return ACHFailed;
    }
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::ULCustVOIPParamFromNV(void *pParam, int *len)
{
    SXLOGD("ULCustVOIPParamFromNV in");

    if (pParam == NULL)
    {
        return ACHFailed;
    }

    uint32_t supportInfo = QueryFeatureSupportInfo();

    if (supportInfo & 0x40)
    {
        int size = 0;
        AUDIO_VOIP_PARAM_STRUCT *pVOIPPara = (AUDIO_VOIP_PARAM_STRUCT *)(pParam + sizeof(int));

        SXLOGD("ULCustVOIPParamFromNV ,VOIP parameters size=%d", sizeof(AUDIO_VOIP_PARAM_STRUCT));
        size = GetAudioVoIPParamFromNV(pVOIPPara);
        if (size != sizeof(AUDIO_VOIP_PARAM_STRUCT))
        {
            SXLOGD("ULCustVOIPParamFromNV Up load from NVRAM fail, structure size=%d,read_size=%d", sizeof(AUDIO_VOIP_PARAM_STRUCT), size);
            return ACHFailed;
        }

        for (int i = 0; i < SPEECH_COMMON_NUM; i++)
        {
            SXLOGV("VOIP parameters speech_common_para[%d]=%d", i, pVOIPPara->speech_common_para[i]);
        }

        for (int i = 0; i < VOIP_INDEX_NUM; i++)
        {
            for (int j = 0; j < SPEECH_PARA_NUM; j++)
            {
                SXLOGV("VOIP parameters speech_mode_para[%d][%d]=%d", i, j, pVOIPPara->speech_mode_para[i][j]);
            }
        }

        for (int i = 0; i < WB_FIR_NUM; i++)
        {
            SXLOGV("VOIP parameters in_fir[0][%d]=%d", i, pVOIPPara->in_fir[0][i]);
            SXLOGV("VOIP parameters out_fir[0][%d]=%d", i, pVOIPPara->out_fir[0][i]);
        }

        *((int *)pParam) = sizeof(AUDIO_VOIP_PARAM_STRUCT);

        dataEncode((char *)pParam, sizeof(AUDIO_VOIP_PARAM_STRUCT) + sizeof(int));
        *len = 2 * (sizeof(AUDIO_VOIP_PARAM_STRUCT) + sizeof(int));
    }
    else
    {
        SXLOGW("ULCustVOIPParamFromNV - do not support VOIP enhance!!");
        return ACHFailed;
    }
    
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::startRecorder(const char *filePath, int recordAudioSource)
{
    SXLOGD("AudioCmdHandler::startRecorder() in");
    int ret = OK;

    if (m_MediaRecorderClient.get() == NULL)
    {
        m_MediaRecorderClient = new MediaRecorder();
        if (m_pMediaRecorderListenner != NULL)
        {
            m_MediaRecorderClient->setListener(m_pMediaRecorderListenner);
        }
    }
    else if (isRecording())
    {
        stopRecorder();
    }

    ret = m_MediaRecorderClient->setAudioSource(recordAudioSource);
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        SXLOGE("AudioCmdHandler::startRecorder-Fail to setAudioSource");
        return ACHFailed;
    }

    ret = m_MediaRecorderClient->setOutputFormat(OUTPUT_FORMAT_WAV);
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        SXLOGE("AudioCmdHandler::startRecorder-Fail to setOutputFormat");
        return ACHFailed;
    }

    ret = m_MediaRecorderClient->setAudioEncoder(AUDIO_ENCODER_PCM);
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        SXLOGE("AudioCmdHandler::startRecorder-Fail to setAudioEncoder");
        return ACHFailed;
    }

    //  ret = m_MediaRecorderClient->setOutputFile(filePath);   //for opencore
    SXLOGD("AudioCmdHandler::filePath=%s", filePath);
    m_fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (m_fd == -1)
    {
        m_MediaRecorderClient->reset();
        SXLOGE("AudioCmdHandler::Create file failed  errno = %d, m_fd =%d", errno, m_fd);
        return ACHFailed;
    }

    ret = m_MediaRecorderClient->setOutputFile(m_fd, 0, 0);
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        close(m_fd);
        SXLOGE("AudioCmdHandler::startRecorder-Fail to setOutputFile");
        return ACHFailed;
    }

    if (recordAudioSource == Audio_I2S_IN)
    {
        AudioSystem::setParameters(0, String8("HQA_I2SREC=1"));
        ret = m_MediaRecorderClient->setParameters(String8("audio-param-number-of-channels=2"));
        if (ret != OK)
        {
            m_MediaRecorderClient->reset();
            close(m_fd);
            SXLOGE("AudioCmdHandler::startRecorder-Fail to set audio-param-number-of-channels");
            return ACHFailed;
        }
    }
    else
    {
        char param1[TEMP_ARRAY_SIZE];
        sprintf(param1, "max-duration=%d", m_RecordMaxDur);
        ret = m_MediaRecorderClient->setParameters(String8(param1));
        if (ret != OK)
        {
            m_MediaRecorderClient->reset();
            close(m_fd);
            SXLOGE("AudioCmdHandler::startRecorder-Fail to set max-duration");
            return ACHFailed;
        }

        char param2[TEMP_ARRAY_SIZE];
        sprintf(param2, "audio-param-number-of-channels=%d", m_RecordChns);
        ret = m_MediaRecorderClient->setParameters(String8(param2));
        if (ret != OK)
        {
            m_MediaRecorderClient->reset();
            close(m_fd);
            SXLOGE("AudioCmdHandler::startRecorder-Fail to set audio-param-number-of-channels");
            return ACHFailed;
        }

        if (recordAudioSource == Audio_FM_IN)
        {
            AudioSystem::setParameters(0, String8("HQA_FMREC_LINE_IN=1"));
        }
    }

    char param4[TEMP_ARRAY_SIZE];
    sprintf(param4, "audio-param-sampling-rate=%d", m_RecordSampleRate);
    ret = m_MediaRecorderClient->setParameters(String8(param4));
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        close(m_fd);
        SXLOGE("AudioCmdHandler::startRecorder-Fail to set audio-param-sampling-rate");
        return ACHFailed;
    }

    ret = m_MediaRecorderClient->prepare();
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        close(m_fd);
        SXLOGE("AudioCmdHandler::startRecorder-Fail to prepare");
        return ACHFailed;
    }

    ret = m_MediaRecorderClient->start();
    if (ret != OK)
    {
        m_MediaRecorderClient->reset();
        close(m_fd);
        SXLOGE("AudioCmdHandler::startRecorder-Fail to start");
        return ACHFailed;
    }

    if (recordAudioSource == Audio_I2S_IN)
    {
        ret = startFMAudioPlayer();
        if (ret != ACHSucceeded)
        {
            m_MediaRecorderClient->reset();
            close(m_fd);
            SXLOGE("AudioCmdHandler::startRecorder-Fail to startFMAudioPlayer");
            return ACHFailed;
        }
    }

    m_bRecording = true;
    m_RecordAudioSource = (AudioSourceType)recordAudioSource;
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::stopRecorder()
{
    SXLOGD("AudioCmdHandler::stopRecorder() in");

    int ret = OK;
    if (m_MediaRecorderClient.get() == NULL)
    {
        SXLOGE("AudioCmdHandler::stopRecorder-have not start recording");
        return ACHFailed;
    }

    if (!isRecording())
    {
        return ACHFailed;
    }

    if (m_RecordAudioSource == Audio_I2S_IN)
    {
        stopFMAudioPlayer();
    }

    //  ret = m_MediaRecorderClient->stop();
    //  if (ret!=OK && ret!=INVALID_OPERATION)
    //      SXLOGE("AudioCmdHandler::stopRecorder-fail to stop recorder");

    m_MediaRecorderClient->reset();
    if (m_RecordAudioSource == Audio_I2S_IN)
    {
        AudioSystem::setParameters(0, String8("HQA_I2SREC=0"));
    }
    else if (m_RecordAudioSource == Audio_FM_IN)
    {
        AudioSystem::setParameters(0, String8("HQA_FMREC_LINE_IN=0"));
    }

    m_bRecording = false;
    close(m_fd);
    return ACHSucceeded;
}

ACHStatus AudioCmdHandler::startFMAudioPlayer()
{
    SXLOGD("AudioCmdHandler::startFMAudioPlayer() in");

    if (m_FMMediaPlayerClient.get() == NULL)
    {
        m_FMMediaPlayerClient = new MediaPlayer();
    }
    else if (m_FMMediaPlayerClient->isPlaying())
    {
        m_FMMediaPlayerClient->stop();
        m_FMMediaPlayerClient->reset();
    }

    if (m_FMMediaPlayerClient->setDataSource("MEDIATEK://MEDIAPLAYER_PLAYERTYPE_FM", 0) != NO_ERROR)
    {
        m_FMMediaPlayerClient->reset();
        SXLOGE("AudioCmdHandler::startFMAudioPlayer-fail to create FM Audio player");
        return ACHFailed;
    }

    //    m_MediaPlayerClient->setAudioStreamType(AUDIO_STREAM_MUSIC);
    if (m_FMMediaPlayerClient->prepare() != NO_ERROR)
    {
        m_FMMediaPlayerClient->reset();
        SXLOGE("AudioCmdHandler::startFMAudioPlayer-fail to play I2S data, FMAudioPlayer prepare failed");
        return ACHFailed;
    }

    if (m_FMMediaPlayerClient->start() != NO_ERROR)
    {
        m_FMMediaPlayerClient->reset();
        SXLOGE("AudioCmdHandler::startFMAudioPlayer-fail to play I2S data, FMAudioPlayer start failed");
        return ACHFailed;
    }

    return ACHSucceeded;
}


ACHStatus AudioCmdHandler::stopFMAudioPlayer()
{
    SXLOGD("AudioCmdHandler::stopAudioPlayer() in");

    if (m_FMMediaPlayerClient.get() != NULL && m_FMMediaPlayerClient->isPlaying())
    {
        if (m_FMMediaPlayerClient->stop() != NO_ERROR)
        {
            SXLOGE("AudioCmdHandler::stopFMAudioPlayer-fail to stop playing I2S data");
            return ACHFailed;
        }
        m_FMMediaPlayerClient->reset();
        return ACHSucceeded;
    }

    SXLOGE("AudioCmdHandler::stopFMAudioPlayer-fail to stop playing I2S data");
    return ACHFailed;
}


//*****************Media Record Listener*********************//
PCMRecorderListener::PCMRecorderListener(AudioCmdHandler *pListener) :
    m_pListener(pListener)
{
}

void PCMRecorderListener::notify(int msg, int ext1, int ext2)
{
    SXLOGD("AudioCmdHandler::PCMRecorderListener::notify-received message: msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
    switch (msg)
    {
        case MEDIA_RECORDER_EVENT_INFO:
            switch (ext1)
            {
                case MEDIA_RECORDER_INFO_MAX_DURATION_REACHED:
                    if (m_pListener != NULL)
                    {
                        m_pListener->enableHALHDRecTunning(false);
                        m_pListener->stopRecorder();
                    }
                    break;
                default:
                    break;
            }
            break;
        case MEDIA_RECORDER_EVENT_ERROR:
            if (m_pListener != NULL)
            {
                m_pListener->enableHALHDRecTunning(false);
                m_pListener->stopRecorder();
            }
            break;
        default:
            break;
    }
}
