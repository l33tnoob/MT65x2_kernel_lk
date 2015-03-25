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

#ifndef _FTM_AUDIO_COMMON_H_
#define _FTM_AUDIO_COMMON_H_

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif



#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3


// for acoustic loopback
#define ACOUSTIC_STATUS   -1
#define DUAL_MIC_WITHOUT_DMNR_ACS_OFF 0
#define DUAL_MIC_WITHOUT_DMNR_ACS_ON  1
#define DUAL_MIC_WITH_DMNR_ACS_OFF   2
#define DUAL_MIC_WITH_DMNR_ACS_ON    3



#define Output_HS  0
#define Output_HP  1
#define Output_LPK 2

enum audio_device_in
{
    BUILTIN_MIC,
    WIRED_HEADSET,
    MATV_ANALOG,
    MATV_I2S
};

enum audio_devices
{
    // output devices
    OUT_EARPIECE = 0,
    OUT_SPEAKER = 1,
    OUT_WIRED_HEADSET = 2,
    DEVICE_OUT_WIRED_HEADPHONE = 3,
    DEVICE_OUT_BLUETOOTH_SCO = 4
};

typedef struct
{
    unsigned int ChunkID;
    unsigned int ChunkSize;
    unsigned int Format;
    unsigned int Subchunk1ID;
    unsigned int Subchunk1IDSize;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    unsigned int SubChunk2ID;
    unsigned int SubChunk2Size;
} WaveHdr;

#define WAVE_HEADER_SIZE (sizeof(WaveHdr))

struct WavePlayData
{
    char *FileName;
    FILE  *pFile;
    bool ThreadStart;
    bool ThreadExit;
    WaveHdr mWaveHeader;
    pthread_t WavePlayThread;
    int  i4Output;
};



/// init / deinit
int Common_Audio_init(void);
int Common_Audio_deinit(void);


/// Loopback
int PhoneMic_Receiver_Loopback(char echoflag);
int PhoneMic_EarphoneLR_Loopback(char echoflag);
int PhoneMic_SpkLR_Loopback(char echoflag);
int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic);
int HeadsetMic_SpkLR_Loopback(char echoflag);

int PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output);


/// Output device test
int RecieverTest(char receiver_test);
int LouderSPKTest(char left_channel, char right_channel);
int EarphoneTest(char bEnable);
int EarphoneTestLR(char bLR);


/// Speaker over current test
int Audio_READ_SPK_OC_STA(void);
int LouderSPKOCTest(char left_channel, char right_channel);


/// Wave playback
int Audio_Wave_playback(void *arg);


/// Read record data while recording
bool recordInit(int device_in);
int readRecordData(void *pbuffer, int bytes);


/// FM / mATV
int FMLoopbackTest(char bEnable);

int Audio_I2S_Play(int enable_flag);
int Audio_MATV_I2S_Play(int enable_flag);
int Audio_FMTX_Play(bool Enable, unsigned int Freq);

int ATV_AudPlay_On(void);
int ATV_AudPlay_Off(void);
int ATV_AudAnalogPath(char bEnable);
unsigned int ATV_AudioWrite(void *buffer, unsigned int bytes);


/// HDMI
int Audio_HDMI_Play(bool Enable, unsigned int Freq);


/// Vibration speaker
void Audio_VSCurrent_Enable(bool enable);
void Audio_VSCurrent_WriteRoutine();
int Audio_VSCurrent_GetFrequency();




#ifdef __cplusplus
};
#endif

#endif // end of _FTM_AUDIO_COMMON_H_
