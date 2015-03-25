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
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   ftm_audio_Common.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Factory Mode Audio Test
 *
 * Author:
 * -------
 *
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 03 06 2013 ning.feng
 * [ALPS00488242] [Need Patch] [Volunteer Patch]GB3/JB3 porting
 * .
 *
 * 01 11 2013 changqing.yan
 * [ALPS00435826] [Factory Mode] AutoTest Fail in Loopback(Spk-Mic)
 * .
 *
 * 01 09 2013 changqing.yan
 * [ALPS00435826] [Factory Mode] AutoTest Fail in Loopback(Spk-Mic)
 * .
 *
 * 01 03 2013 changqing.yan
 * [ALPS00434013] [Need Patch] [Volunteer Patch]Receiver/Headset auto test on factory mode
 * .
 *
 * 12 26 2012 changqing.yan
 * [ALPS00428915] [Power Management] Factory mode suspend current is greater than flight mode suspend
 * .
 *
 * 11 08 2012 changqing.yan
 * [ALPS00390142] [Need Patch] [Volunteer Patch]Factory mode ringtone auto test.
 * .
 *
 * 01 30 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * .
 *
 * 01 18 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * set dual mic input.
 *
 * 01 11 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * DMNR acoustic loopback check in.
 *
 * 12 27 2011 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * change mic volume setting
 *
 * 12 26 2011 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * DMNR Acoustic loopback check in.
 *
 * 12 14 2011 donglei.ji
 * [ALPS00101149] [Need Patch] [Volunteer Patch]AudioPlayer, AMR/AWB Playback ICS migration
 * Audio factory mode migration- remove mt6516 code.
 *
 * 10 12 2011 donglei.ji
 * [ALPS00079849] [Need Patch] [Volunteer Patch][Factory Mode] TF card test disturbs Ringtone test
 * adjust file handler correctly..
 *
 *
 *******************************************************************************/

#include "cust.h"
#include <fcntl.h>

#ifndef FEATURE_DUMMY_AUDIO

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include "ftm_audio_Common.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include "SineWave_156Hz.h"

#include <hardware_legacy/AudioHardwareInterface.h>

#include <AudioFtmBase.h>

#include <audio_custom_exp.h>
#include <CFG_AUDIO_File.h>

#include <AudioVIBSPKControl.h>
#include <VibSpkAudioPlayer.h>

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/



/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

using android::status_t;

/*****************************************************************************
*                   G L O B A L      V A R I A B L E
******************************************************************************
*/

// golbal hardware pointer
static    android_audio_legacy::AudioHardwareInterface *gAudioHardware = NULL;
static    android_audio_legacy::AudioStreamOut *gAudioStreamOut = NULL;
static    android_audio_legacy::AudioStreamIn *gAudioStreamIn = NULL;
static    android::AudioFtmBase *gAudioFtmBase;


/*****************************************************************************
*                   I M P L E M E N T A T I O N
******************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif



///------------------the following is Factory mode test API------------------
//
//FUNCTION:
//      Common_Audio_init
//DESCRIPTION:
//      this function is called to init factory Audio
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int Common_Audio_init(void)
{
    // if hardware is not create , create haredware , else return.
    if (gAudioHardware == NULL)
    {
        gAudioHardware = android_audio_legacy::createAudioHardware();
    }
    // get ftm module
    if (gAudioFtmBase == NULL)
    {
        gAudioFtmBase = android::AudioFtmBase::createAudioFtmInstance();
    }
    return true;
}


///------------------the following is Factory mode test API------------------
//
//FUNCTION:
//      Common_Audio_deinit
//DESCRIPTION:
//      this function is called to deinit factory Audio
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int Common_Audio_deinit(void)
{
    if (gAudioStreamIn != NULL)
    {
        gAudioStreamIn->standby();
        //gAudioHardware->closeInputStream(gAudioStreamIn);
        //gAudioStreamIn = NULL;
    }
    /*if(gAudioHardware != NULL)
    {
        delete gAudioHardware;
        gAudioHardware = NULL;;
    }*/
    return true;
}



/*
 * codec related control
 */
void Audio_Set_Speaker_Vol(int level)
{
    gAudioFtmBase->Audio_Set_Speaker_Vol(level);
}

void Audio_Set_Speaker_On(int Channel)
{
    gAudioFtmBase->Audio_Set_Speaker_On(Channel);
}

void Audio_Set_Speaker_Off(int Channel)
{
    gAudioFtmBase->Audio_Set_Speaker_Off(Channel);
}

void Audio_Set_HeadPhone_On(int Channel)
{
    gAudioFtmBase->Audio_Set_HeadPhone_On(Channel);
}

void Audio_Set_HeadPhone_Off(int Channel)
{
    gAudioFtmBase->Audio_Set_HeadPhone_Off(Channel);
}

void Audio_Set_Earpiece_On()
{
    gAudioFtmBase->Audio_Set_Earpiece_On();
}

void Audio_Set_Earpiece_Off()
{
    gAudioFtmBase->Audio_Set_Earpiece_Off();
}


///------------------the following is Factory mode test API------------------
//
//FUNCTION:
//      PhoneMic_Receiver_Loopback
//DESCRIPTION:
//      this function is called to test reciever loop back.
//
//PARAMETERS:
//      echoflag:   [IN]    (char)true mean enable, otherwise 0 is disable
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int PhoneMic_Receiver_Loopback(char echoflag)
{
    return gAudioFtmBase->PhoneMic_Receiver_Loopback(echoflag);
}

int PhoneMic_EarphoneLR_Loopback(char echoflag)
{
    return gAudioFtmBase->PhoneMic_EarphoneLR_Loopback(echoflag);
}

int PhoneMic_SpkLR_Loopback(char echoflag)
{
    return gAudioFtmBase->PhoneMic_SpkLR_Loopback(echoflag);
}

int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic)
{
    return gAudioFtmBase->HeadsetMic_EarphoneLR_Loopback(bEnable, bHeadsetMic);
}

int HeadsetMic_SpkLR_Loopback(char echoflag)
{
    return gAudioFtmBase->HeadsetMic_SpkLR_Loopback(echoflag);
}

///------------------the following is Factory mode test API------------------
//
//FUNCTION:
//      PhoneMic_Receiver_Acoustic_Loopback
//DESCRIPTION:
//      this function is called to test 2-mic acoustic loop back.
//
//PARAMETERS:
//      Acoustic_Type:  [IN]    (int)0:acoustic loopback off; 1:dualmic acoustic loopback; 2:single mic acoustic loopback
//           Acoustic_Status     [IN] (int)
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output)
{
    return gAudioFtmBase->PhoneMic_Receiver_Acoustic_Loopback(Acoustic_Type, Acoustic_Status_Flag, bHeadset_Output);
}


//-------------------------------------------------------------------
//FUNCTION:
//      RecieverTest
//DESCRIPTION:
//      this function is called to test reciever channel using inner sine wave.
//
//PARAMETERS:
//      receiver_test:  [IN]    (char)true mean enable, otherwise 0 is disable
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int RecieverTest(char receiver_test)
{
    return gAudioFtmBase->RecieverTest(receiver_test);
}


//-------------------------------------------------------------------
//FUNCTION:
//      LouderSPKTest
//DESCRIPTION:
//      this function is called to test loud speaker channel using inner sine wave.
//
//PARAMETERS:
//      left_channel/right_channel:     [IN]    (char)true mean enable, otherwise 0 isturnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//------------------------------------------------------------------
int LouderSPKTest(char left_channel, char right_channel)
{
    return gAudioFtmBase->LouderSPKTest(left_channel, right_channel);
}


//-------------------------------------------------------------------
//FUNCTION:
//      EarphoneTest
//DESCRIPTION:
//      this function is called to test earphone speaker channel using inner sinewave.
//
//PARAMETERS:
//      bEnable:    [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int EarphoneTest(char bEnable)
{
    return gAudioFtmBase->EarphoneTest(bEnable);
}

int EarphoneTestLR(char bLR)
{
    return gAudioFtmBase->EarphoneTestLR(bLR);
}

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_READ_SPK_OC_STA
//DESCRIPTION:
//      Audio_READ_SPK_OC_STA
//
//PARAMETERS:
//
//
//RETURN VALUE:
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int Audio_READ_SPK_OC_STA(void)
{
    return gAudioFtmBase->Audio_READ_SPK_OC_STA();
}


//-------------------------------------------------------------------
//FUNCTION:
//      LouderSPKOCTest
//DESCRIPTION:
//      this function is called to test OC function while loud speaker channel uses inner sine wave.
//
//PARAMETERS:
//      left_channel/right_channel:     [IN]    (char)true mean enable, otherwise 0 isturnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//------------------------------------------------------------------
int LouderSPKOCTest(char left_channel, char right_channel)
{
    return gAudioFtmBase->LouderSPKOCTest(left_channel, right_channel);
}


//-------------------------------------------------------------------
//FUNCTION:
//      Audio_Wave_playback
//DESCRIPTION:
//      Audio_Wave_playback
//
//PARAMETERS:
//
//
//RETURN VALUE:
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int PCM_decode_data(WaveHdr *wavHdr,  char *in_buf, int block_size, char *out_buf, int *out_size)
{
    int i, j;
    uint16_t *ptr_d;
    uint8_t  *ptr_s;
    int readlen = 0;
    int writelen = 0;

    uint16_t channels = wavHdr->NumChannels;
    uint16_t bits_per_sample = wavHdr->BitsPerSample;

    ptr_s = (uint8_t *)in_buf;
    ptr_d = (uint16_t *)out_buf;
    readlen = block_size;
    *out_size = 0;
    j = 0;
    switch (bits_per_sample)
    {
        case 8:
            if (channels == 2)
            {
                for (i = 0; i < readlen; i++)
                {
                    *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                    j++;
                }
            }
            else
            {
                for (i = 0; i < readlen; i++)
                {
                    *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                    *(ptr_d + j + 1) =  *(ptr_d + j);
                    j += 2;
                }
            }
            writelen = (j << 1);
            break;
        case 16:
            if (channels == 2)
            {
                for (i = 0; i < readlen; i += 2)
                {
                    *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                    j++;
                }
            }
            else
            {
                for (i = 0; i < readlen; i += 2)
                {
                    *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                    *(ptr_d + j + 1) = *(ptr_d + j);
                    j += 2;
                }
            }
            writelen = (j << 1);
            break;
        default:
            ptr_d = (uint16_t *)(out_buf);
            break;
    }
    *out_size = writelen;
    return true;
}

static void *Audio_Wave_Playabck_routine(void *arg)
{
    char *inBuffer = NULL;
    char *outBuffer = NULL;
    int format;
    uint32_t channels;
    uint32_t sampleRate;
    status_t status;
    uint32_t ReadBlockLen;
    uint32_t hwBufferSize;
    int out_size;

    WavePlayData *pWavePlaydata = (WavePlayData *)arg;
    if (pWavePlaydata == NULL || arg == NULL)
    {
        ALOGD("Audio_Wave_Playabck_routine Exit \n");
        pthread_exit(NULL); // thread exit_sockets
        return NULL;
    }

    printf("pWavePlaydata open file %s \n", pWavePlaydata->FileName);
    pWavePlaydata->pFile = fopen(pWavePlaydata->FileName, "rb");
    //pWavePlaydata->pFile =fopen("/sdcard/testpattern1.wav","rb");
    if (pWavePlaydata->pFile == NULL)
    {
        printf("pWavePlaydata open file fail\n");
        pWavePlaydata->ThreadExit = true;
        pthread_exit(NULL); // thread exit
        return NULL;
    }

    // read wave header
    fread((void *)&pWavePlaydata->mWaveHeader, WAVE_HEADER_SIZE, 1, pWavePlaydata->pFile);
    ALOGD("BitsPerSample = %d", pWavePlaydata->mWaveHeader.BitsPerSample);
    ALOGD("NumChannels = %d", pWavePlaydata->mWaveHeader.NumChannels);
    ALOGD("SampleRate = %d", pWavePlaydata->mWaveHeader.SampleRate);

    gAudioFtmBase->FTM_AnaLpk_on();

    //config output format channel= 2 , bits_per_sample=16
    sampleRate = pWavePlaydata->mWaveHeader.SampleRate;
    channels = android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;

    // TODO: 32 bit?
    if (pWavePlaydata->mWaveHeader.BitsPerSample == 8 || pWavePlaydata->mWaveHeader.BitsPerSample == 16)
    {
        format = android_audio_legacy::AudioSystem::PCM_16_BIT;
    }
    else
    {
        format = android_audio_legacy::AudioSystem::PCM_16_BIT;
    }

    //create output stream
    android_audio_legacy::AudioStreamOut *streamOutput = gAudioHardware->openOutputStream(0, &format, &channels, &sampleRate, &status);

    hwBufferSize = streamOutput->bufferSize(); //16k bytes

    if (pWavePlaydata->mWaveHeader.NumChannels == 1)
    {
        switch (pWavePlaydata->mWaveHeader.BitsPerSample)
        {
            case 8:
                ReadBlockLen = hwBufferSize >> 2;
                break;
            case 16:
                ReadBlockLen = hwBufferSize >> 1;
                break;
            default:
                ReadBlockLen = 0;
                break;
        }
    }
    else
    {
        switch (pWavePlaydata->mWaveHeader.BitsPerSample)
        {
            case 8:
                ReadBlockLen = hwBufferSize >> 1;
                break;
            case 16:
                ReadBlockLen = hwBufferSize;
                break;
            default:
                ReadBlockLen = 0;
                break;
        }
    }

    inBuffer = new char[ReadBlockLen];
    outBuffer = new char[hwBufferSize];

    printf("ReadBlockLen = %d, hwBufferSize = %d\n", ReadBlockLen, hwBufferSize);


    //Select audio output device
    android::AudioParameter paramRouting = android::AudioParameter();
    if (pWavePlaydata->i4Output == Output_HS)
    {
        paramRouting.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_OUT_EARPIECE);
        streamOutput->setParameters(paramRouting.toString());
    }
    else if (pWavePlaydata->i4Output == Output_HP)
    {
        paramRouting.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
        streamOutput->setParameters(paramRouting.toString());
    }
    else if (pWavePlaydata->i4Output == Output_LPK)
    {
        paramRouting.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_OUT_SPEAKER);
        streamOutput->setParameters(paramRouting.toString());

        gAudioHardware->setMasterVolume(0.5);
    }

    // continue render to hardware
    while (!feof(pWavePlaydata->pFile) && pWavePlaydata->ThreadExit == false)
    {
        int readdata = 0, writedata = 0;
        readdata = fread(inBuffer, ReadBlockLen, 1, pWavePlaydata->pFile);
        PCM_decode_data(&pWavePlaydata->mWaveHeader, inBuffer, ReadBlockLen, outBuffer, &out_size);

        writedata = streamOutput->write(outBuffer, out_size);
        ALOGV("Audio_Wave_Playabck_routine write to hardware... read = %d writedata = %d", readdata, out_size);
    }

    gAudioFtmBase->FTM_AnaLpk_off();
    streamOutput->standby();
    gAudioHardware->closeOutputStream(streamOutput);

    if (inBuffer)
    {
        delete[] inBuffer;
        inBuffer = NULL;
    }
    if (outBuffer)
    {
        delete[] outBuffer;
        outBuffer = NULL;
    }

    if (pWavePlaydata->pFile)
    {
        fclose(pWavePlaydata->pFile);
        pWavePlaydata->pFile = NULL;
    }

    // thread exit;
    pWavePlaydata->ThreadExit = true;
    pWavePlaydata->ThreadStart = false;

    printf("Audio_Wave_Playabck_routine Exit \n");
    pthread_exit(NULL); // thread exit_sockets
    return NULL;
}

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_Wave_playback
//DESCRIPTION:
//      Audio_Wave_playback
//
//PARAMETERS:
//
//
//RETURN VALUE:
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int Audio_Wave_playback(void *arg)
{
    printf("Audio_Wave_playback with arg = %x \n", (unsigned int)arg);
    WavePlayData *pWavePlaydata = (WavePlayData *)arg;
    if (pWavePlaydata == NULL || arg == NULL)
    {
        ALOGD("Audio_Wave_Playabck_routine Exit \n");
        pthread_exit(NULL); // thread exit
        return false;
    }

    if (pWavePlaydata->WavePlayThread == (pthread_t) NULL)
    {
        // create playback thread
        printf("pthread_create WavePlayThread\n");
        pthread_create(&pWavePlaydata->WavePlayThread, NULL, Audio_Wave_Playabck_routine, arg);
    }
    printf("Audio_Wave_playback return \n");
    return true;
}


/// record
bool recordInit(int device_in)
{
    if (gAudioStreamIn == NULL)
    {
        android::AudioParameter paramVoiceMode = android::AudioParameter();
        paramVoiceMode.addInt(android::String8("HDREC_SET_VOICE_MODE"), 0);
        gAudioHardware->setParameters(paramVoiceMode.toString());

        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        uint32_t sampleRate = 48000;
        status_t status = 0;
        android::AudioParameter param = android::AudioParameter();

        if (device_in == MATV_I2S)
        {
            sampleRate = 32000;
            device = AUDIO_DEVICE_IN_MATV ;
            gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
            param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_MATV);
        }
        else if (device_in == MATV_ANALOG)
        {
            sampleRate = 48000;
            device = AUDIO_DEVICE_IN_MATV;
            gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
            param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_MATV);
        }
        else
        {
            gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
            android::AudioParameter param = android::AudioParameter();
            param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_MIC);
        }
        gAudioStreamIn->setParameters(param.toString());
    }
    android::AudioParameter param = android::AudioParameter();
    if (device_in == BUILTIN_MIC)
    {
        param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);
    }
    else if (device_in == WIRED_HEADSET)
    {
        param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_WIRED_HEADSET);
    }
    else if (device_in == MATV_ANALOG)
    {
        param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_MATV);
    }
    else if (device_in == MATV_I2S)
    {
        param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_MATV);
    }

    gAudioStreamIn->setParameters(param.toString());

    return true;
}


int readRecordData(void *pbuffer, int bytes)
{
    int nBytes = 0;

    nBytes = gAudioStreamIn->read(pbuffer, bytes);
    return nBytes;
}



//-------------------------------------------------------------------
//FUNCTION:
//      FMLoopbackTest
//DESCRIPTION:
//      this function is called to enable or disable FM channel.
//
//PARAMETERS:
//      bEnable:    [IN]    (char)true mean enable, otherwise 0 is turnoff
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      META_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------

int FMLoopbackTest(char bEnable)
{
    return gAudioFtmBase->FMLoopbackTest(bEnable);
}


//-------------------------------------------------------------------
//FUNCTION:
//      Audio_I2S_Play
//DESCRIPTION:
//      Audio_I2S_Play
//
//PARAMETERS:
//
//
//RETURN VALUE:
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int Audio_I2S_Play(int enable_flag)
{
    return gAudioFtmBase->Audio_FM_I2S_Play(enable_flag);
}

int Audio_MATV_I2S_Play(int enable_flag)
{
    return gAudioFtmBase->Audio_MATV_I2S_Play(enable_flag);
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
    return gAudioFtmBase->Audio_FMTX_Play(Enable, Freq);
}


//-------------------------------------------------------------------
//FUNCTION:
//      ATV_AudPlay_On
//DESCRIPTION:
//      Start the mATV audio play
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int ATV_AudPlay_On(void)
{
    return gAudioFtmBase->ATV_AudPlay_On();
}

//-------------------------------------------------------------------
//FUNCTION:
//      ATV_AudPlay_Off
//DESCRIPTION:
//      Stop the mATV audio play
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int ATV_AudPlay_Off(void)
{
    return gAudioFtmBase->ATV_AudPlay_Off();
}

//-------------------------------------------------------------------
//FUNCTION:
//      ATV_AudAnalogPath
//DESCRIPTION:
//      Audio Analog Line in play
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
int ATV_AudAnalogPath(char bEnable)
{
    ALOGD("ATV_AudAnalogPath bEnable=%d", bEnable);
    return FMLoopbackTest(bEnable);
}

//-------------------------------------------------------------------
//FUNCTION:
//      ATV_AudioWrite
//DESCRIPTION:
//      Write PCM data to audio HW
//
//PARAMETERS:
//
//
//RETURN VALUE:
//      TRUE is success, otherwise is fail
//
//DEPENDENCY:
//      Common_Audio_init must be called before
//
//GLOBALS AFFECTED
//      None
//-------------------------------------------------------------------
unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes)
{
    return gAudioFtmBase->ATV_AudioWrite(buffer, bytes);
}



/// HDMI
int Audio_HDMI_Play(bool Enable, unsigned int Freq)
{
    return gAudioFtmBase->Audio_HDMI_Play(Enable, Freq);
}



/// VIBSPK
#define CENTER_FREQ_START                 141
#define CENTER_FREQ_MIN                   141
#define CENTER_FREQ_MAX                   330
#define CENTER_FREQ_END                   350
#define DELTA_FREQ_COARSE                   6
#define DELTA_FREQ_FINE                     3
#define DELTA_FREQ_FINE_RANGE               9
#define TONE_FRAME_COUNT                    6
#define AUDIO_SET_CUREENT_SENSOR_ENABLE 0x210
#define AUDIO_SET_CURRENT_SENSOR_RESET  0x211
#ifndef VIBSPK_DEFAULT_FREQ
#define VIBSPK_DEFAULT_FREQ (156) //141~330 Hz
#endif

const int32_t AUD_VIBRFACTORY_FILTER_COEF_Table[VIBSPK_FILTER_NUM][2][6][3] =
{
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_141,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_144,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_147,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_150,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_153,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_156,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_159,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_162,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_165,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_168,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_171,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_174,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_177,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_180,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_183,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_186,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_189,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_192,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_195,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_198,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_201,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_204,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_207,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_210,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_213,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_216,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_219,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_222,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_225,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_228,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_231,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_234,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_237,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_240,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_243,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_246,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_249,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_252,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_255,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_258,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_261,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_264,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_267,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_270,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_273,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_276,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_279,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_282,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_285,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_288,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_291,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_294,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_297,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_300,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_303,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_306,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_309,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_312,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_315,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_318,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_321,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_324,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_327,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_330,
};

static    android_audio_legacy::AudioStreamOut *vibspkStreamOutput = NULL;
static    uint32_t vibspkBufferSize = 0;
static    char *vibspkBuffer = NULL;
static    uint32_t vibspkwrite_counter = 0;

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_VSCurrent_Enable
//DESCRIPTION:
//      this function is called to enable/disable vibspk process
//------------------------------------------------------------------
void Audio_VSCurrent_Enable(bool enable)
{
    uint32_t channels, sampleRate;
    int32_t format;
    status_t status;

    if (enable)
    {
        /********************************************
        *   Open output stream and set device       *
        ********************************************/
        gAudioFtmBase->FTM_AnaLpk_on();
        sampleRate = 44100;
        channels = android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;
        format = android_audio_legacy::AudioSystem::PCM_16_BIT;
        vibspkStreamOutput = gAudioHardware->openOutputStream(0, &format, &channels, &sampleRate, &status);
        
        android::AudioParameter paramRouting = android::AudioParameter();
        paramRouting.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_OUT_SPEAKER);
        vibspkStreamOutput->setParameters(paramRouting.toString());

        vibspkBufferSize = vibspkStreamOutput->bufferSize();
        vibspkBuffer = new char[vibspkBufferSize];
        memset(vibspkBuffer, 0, vibspkBufferSize);
        vibspkwrite_counter = 0;
        gAudioHardware->SetAudioCommand(AUDIO_SET_CUREENT_SENSOR_ENABLE, 1);
    }
    else
    {
        gAudioFtmBase->SetVibSpkEnable(false, 0);
        vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
        vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
        vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
        gAudioFtmBase->FTM_AnaLpk_off();
        vibspkStreamOutput->standby();
        gAudioHardware->closeOutputStream(vibspkStreamOutput);
        gAudioHardware->SetAudioCommand(AUDIO_SET_CUREENT_SENSOR_ENABLE, 0);

        if (vibspkBuffer)
        {
            delete[] vibspkBuffer;
            vibspkBuffer = NULL;
        }
    }
}

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_VSCurrent_WriteRoutine
//DESCRIPTION:
//      this function is called to write silence data to audio HW
//------------------------------------------------------------------
void Audio_VSCurrent_WriteRoutine()
{
    vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
    vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
    vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
    vibspkwrite_counter++;
}

//-------------------------------------------------------------------
//FUNCTION:
//      Audio_VSCurrent_GetFrequency
//DESCRIPTION:
//      this function is called to get the center frequency of vibspk
//------------------------------------------------------------------
int Audio_VSCurrent_GetFrequency()
{
    uint32_t califreq, vib_idx;
    int32_t center_freq, counter, adc_value;
    uint32_t adc_min_freq, adc_min_vol;
    uint32_t freq_fine_max;
    bool adc_status, freq_coarse_done;
    AUDIO_ACF_CUSTOM_PARAM_STRUCT cali_param;

    /********************************************
    *        Init Region                        *
    ********************************************/
    counter          = 0;
    adc_min_freq     = CENTER_FREQ_START;
    adc_min_vol      = 0x7FFFFFFF;
    freq_coarse_done = false;

#if 0  //if don't want to do calibration each time
    califreq = gAudioHardware->GetVibSpkCalibrationStatus();
    if (califreq != 0)
    {
        ALOGD("VSCurSensor get frequency:%x", califreq);
        gAudioHardware->SetVibSpkEnable(true, califreq);
        return califreq;
    }
#endif
    /********************************************
    *   Sine Gen and Current Sensor             *
    ********************************************/

    center_freq = CENTER_FREQ_START;
    while (1)
    {
        gAudioFtmBase->SetVibSpkEnable(true, center_freq);
        if (counter == 0)
        {
            gAudioFtmBase->SetVibSpkRampControl(2);
        }
        vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
        counter++;
        if (counter == TONE_FRAME_COUNT)
        {
            gAudioHardware->SetAudioCommand(AUDIO_SET_CURRENT_SENSOR_RESET, 1);
            usleep(1000);
            //gAudioHardware->SetAudioCommand(AUDIO_SET_CURRENT_SENSOR_RESET,0);
            usleep(5000);
            adc_status = gAudioFtmBase->ReadAuxadcData(16, &adc_value);
            if (adc_value < adc_min_vol)
            {
                adc_min_vol  = adc_value;
                adc_min_freq = center_freq;
            }
            ALOGD("VSCurrentSensor=0x%x %x at freq %d", adc_value, adc_status, center_freq);
            gAudioFtmBase->SetVibSpkRampControl(1);
            vibspkStreamOutput->write(vibspkBuffer, vibspkBufferSize);
            counter = 0;
            if (freq_coarse_done)
            {
                center_freq += DELTA_FREQ_FINE;
            }
            else
            {
                center_freq += DELTA_FREQ_COARSE;
            }

            if (center_freq > CENTER_FREQ_END && freq_coarse_done == false)
            {
                freq_coarse_done = true;
                if (adc_min_freq - DELTA_FREQ_FINE_RANGE < CENTER_FREQ_START)
                {
                    adc_min_freq = CENTER_FREQ_START + DELTA_FREQ_FINE_RANGE;
                }
                if (adc_min_freq + DELTA_FREQ_FINE_RANGE > CENTER_FREQ_END)
                {
                    adc_min_freq = CENTER_FREQ_END - DELTA_FREQ_FINE_RANGE;
                }
                adc_min_vol = 0x7FFFFFFF;
                center_freq = adc_min_freq - DELTA_FREQ_FINE_RANGE;
                freq_fine_max = adc_min_freq + DELTA_FREQ_FINE_RANGE;
            }

            if (freq_coarse_done == true && center_freq > freq_fine_max)
            {
                break;
            }
        }
    }

    gAudioFtmBase->SetVibSpkRampControl(2);
    if (adc_min_freq < CENTER_FREQ_MIN)
    {
        adc_min_freq = VIBSPK_DEFAULT_FREQ;
    }
    if (adc_min_freq > CENTER_FREQ_MAX)
    {
        adc_min_freq = VIBSPK_DEFAULT_FREQ;
    }
    /********************************************
    *   Write Calibration Data to NVRAM         *
    ********************************************/
    memset(&cali_param, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    vib_idx = (adc_min_freq - CENTER_FREQ_MIN) / DELTA_FREQ;

    memcpy(&cali_param.bes_loudness_bpf_coeff, &AUD_VIBRFACTORY_FILTER_COEF_Table[vib_idx], sizeof(uint32_t)*VIBSPK_AUD_PARAM_SIZE);
    cali_param.bes_loudness_WS_Gain_Min = adc_min_freq;
    cali_param.bes_loudness_WS_Gain_Max = VIBSPK_CALIBRATION_DONE;
    gAudioFtmBase->SetVibSpkCalibrationParam(&cali_param);
    gAudioFtmBase->SetVibSpkEnable(true, adc_min_freq);

    ALOGD("VIBSPK_CalibrationDone:%x", adc_min_freq);
    return adc_min_freq;
}

int Audio_HDMI_SineTonePlayback(bool bEnable, int dSamplingRate)
{
    return gAudioFtmBase->HDMI_SineGenPlayback(bEnable, dSamplingRate);
}




#ifdef __cplusplus
};
#endif

#else  // dummy audio function   -->   #ifndef FEATURE_DUMMY_AUDIO

#ifdef __cplusplus
extern "C" {
#endif


    /// init / deinit
    int Common_Audio_init(void) { return true; }
    int Common_Audio_deinit(void) { return true; }


    /// Loopback
    int PhoneMic_Receiver_Loopback(char echoflag) { return true; }
    int PhoneMic_EarphoneLR_Loopback(char echoflag) { return true; }
    int PhoneMic_SpkLR_Loopback(char echoflag) { return true; }
    int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic) { return true; }
    int HeadsetMic_SpkLR_Loopback(char echoflag) { return true; }

    int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output) { return true; }


    /// Output device test
    int RecieverTest(char receiver_test) { return true; }
    int LouderSPKTest(char left_channel, char right_channel) { return true; }
    int EarphoneTest(char bEnable) { return true; }


    /// Speaker over current test
    int Audio_READ_SPK_OC_STA(void) { return true; }
    int LouderSPKOCTest(char left_channel, char right_channel) { return true; }


    /// Wave playback
    int Audio_Wave_playback(void *arg) { return true; }


    /// Read record data while recording
    bool recordInit(int device_in) { return true; }
    int readRecordData(void *pbuffer, int bytes) { return 0; }


    /// FM / mATV
    int FMLoopbackTest(char bEnable) { return true; }

    int Audio_I2S_Play(int enable_flag) { return true; }
    int Audio_MATV_I2S_Play(int enable_flag) { return true; }
    int Audio_FMTX_Play(bool Enable, unsigned int Freq) { return true; }

    int ATV_AudPlay_On(void) { return true; }
    int ATV_AudPlay_Off(void) { return true; }
    int ATV_AudAnalogPath(char bEnable) { return true; }
    unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes) { return true; }


    /// HDMI
    int Audio_HDMI_Play(bool Enable, unsigned int Freq) { return true; }


    /// Vibration speaker
    void Audio_VSCurrent_Enable(bool enable) { }
    void Audio_VSCurrent_WriteRoutine() { }
    int Audio_VSCurrent_GetFrequency() { return true; }
    int Audio_HDMI_SineTonePlayback(bool bEnable, int dSamplingRate) { return true; }

#ifdef __cplusplus
};
#endif

#endif   // #ifndef FEATURE_DUMMY_AUDIO

