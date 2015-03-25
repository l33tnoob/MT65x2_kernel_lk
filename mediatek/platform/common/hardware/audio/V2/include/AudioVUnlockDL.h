/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
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
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudioVPWStreamIn.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Stream In Downlink data for voice password (user space)
 *
 * Author:
 * -------
 *
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime:$
 * $Log:$
 *
 * 09 24 2013 donglei.ji
 * [ALPS01026663] voice ui&unlock feature latency tuning
 * .
 *
 * 10 01 2012 vend_Wendy.lin
 *
 *
 *******************************************************************************/

#ifndef ANDROID_AUDIO_VPWStreamIn_H
#define ANDROID_AUDIO_VPWStreamIn_H

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <stdint.h>
#include <sys/types.h>
extern "C" {
#include "MtkAudioSrc.h"
}
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <media/AudioSystem.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
//#define VPW_Buffer_Count (2)
#define forUT
//VPW stream In read once size
#define VOICE_UNLOCK_RING_BUFFER_SIZE (16384*2) //140ms 48k 2ch data length
// I2S buffer size
#define VPW_OUTPUT_SAMPLERATE (16000)
namespace android
{


/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************

this enum also defined the capability of I2S , for example , now I2S is support I2S_INPUT ,
I2S_OUTPUT and I2S1_OUTPUT
*/
typedef enum
{
    VPWStreamIn_NONE                = 0x0,
    VPWStreamIn_CREATED          = 0x1,
    VPWStreamIn_READ_START   = 0x2,
    VPWStreamIn_WRITE_START   = 0x4,
} VPWStreamInSTATE;

typedef struct
{
    char *pBufBase;
    char *pRead;
    char *pWrite;
    char *pBufEnd;
    int   bufLen;
    bool buffull;
} rb_vpw;



/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/
class AudioVUnlockDL;
class AudioVUnlockRingBuf;

class AudioVUnlockRingBuf
{
    public:
        AudioVUnlockRingBuf();
        ~AudioVUnlockRingBuf();

        int32_t Write(void *buf, uint32_t datasz);
        int32_t Read(void *buf, uint32_t datasz);
        /* AdvanceReadPointer only advance read pointer, no R/W operation*/
        uint32_t AdvanceReadPointer(uint32_t datasz);
        /*ReadWithoutAdvance only read data from buffer without advance read ptr */
        uint32_t ReadWithoutAdvance(void *buf, uint32_t datasz);
        uint32_t GetBuflength(void);
        uint32_t GetBufDataSz(void);
        uint32_t GetBufSpace(void);
        /*wait if no data to read*/
        uint32_t WaitBufData(void);
        /*signal if data exist for read*/
        uint32_t SignalBufData(void);

        uint32_t ResetBuf(void);

        uint32_t WriteAdvance(void *buf, uint32_t datasz);
        uint32_t ReadAdvance(void *buf, uint32_t datasz);
    private:

        rb_vpw mbuf;
        pthread_mutex_t mBufMutex;
        pthread_cond_t mBuf_Cond;
        char mbufAddr[VOICE_UNLOCK_RING_BUFFER_SIZE];

};


class AudioVUnlockDL
{
    public:
        AudioVUnlockDL();
        ~AudioVUnlockDL();

        static AudioVUnlockDL *StreamOutGetInstance();
        static AudioVUnlockDL *getInstance();
        /*startInput create a read thread to move data, must be called first before R/W start */
        bool startInput(void);
        /* stopInput will terminate the read thread*/
        bool stopInput(void);
        /* for stream out to write data to ring buffer*/
        int32_t WriteStreamOutToRing(const void *buf, uint32_t datasz);
        /*read reference data from stream ring buffer, if no data in ring buffer, write zeroes to buf.*/
        int32_t ReadRefFromRing(void *buf, uint32_t datasz, void *DLtime);
        int32_t GetState(void);
        void ClearState(uint32_t state);
        void SetState(uint32_t state);
        bool StateInputStart(void);
        static void  freeInstance();

        int32_t GetSRCInputParameter(uint32_t inSR,                  /* Input, input sampling rate of the conversion */
                                     uint32_t inChannel             /* Input, input channel number of the conversion */);
        int32_t  SetSRC(uint32_t outSR,                 /* Input, output sampling rate of the conversion */
                        uint32_t outChannel            /* Input, output channel number of the conversion */);
        /*DoSRC : return data consumed.*/
        int32_t DoSRC(uint8_t *inbuf,   /* input buffer pointer*/
                      uint32_t *datasz , /* input: data in input buffer, output: data left in input buffer */
                      uint8_t *outbuf,    /*output buffer pointer*/
                      uint32_t *outlength);   /*input: buffer length of output buffer, output: data produced */
        /*return latency in ms*/
        int32_t GetLatency(void);
        int32_t DumpData(void *buf, uint32_t datasz);
        void SetInputStandBy(bool val);
        void GetStreamOutLatency(int32_t latency);
        bool StreamOutStandBy();
        int GetShareMemory(int *fd, int *size, uint *flags);
        int32_t SetUplinkStartTime(struct timespec uplinkStartTime);
        int32_t SetDownlinkStartTime(int remainMs);
        int32_t  GetVoiceUnlockULTime(void *ULtime);
        int32 GetFirstDLTime();
#ifdef forUT
        bool StateStartwrite(void);
        bool startWrite(void);

        bool stopWrite();


        bool mWriteThreadExit;
        bool mWriteThreadActive;
        pthread_t mWriteThread; // readthread to move data from mRingBufIn to  mRingBufOut


#endif
        AudioVUnlockRingBuf mRingBufIn;  // input ring buffer
        AudioVUnlockRingBuf mRingBufOut;    //output ring buffer
        bool mReadThreadExit;
        bool mReadThreadActive;
        bool mReadFunctionActive;
        // for dump file
        FILE *mOutFile;
        FILE *mOutFile_1;
        FILE *mOutFile_2;
        struct timespec mDLtime;
        struct timespec mNewDLtime;
        struct timespec mStandbyTime;

        bool mGetTime;
        uint32_t mInRemaining;
        uint32_t mOutRemaining;
        uint32_t mInChannel; // input channel for SRC
        uint32_t mOutChannel; // output channel for SRC
        uint32_t mOutputSampleRate; //output sampling rate for src
        uint32_t mInputSampleRate; // input sampling rate for src
    private:
        MtkAudioSrc *mpSrcHdl ;  //SRC handle
        uint32_t mSrcBufLen; // SRC working buffer length
        pthread_t mReadThread; // readthread to move data from mRingBufIn to  mRingBufOut
        int32_t mState;
        bool mInputStandby; // True : stream out is in standby mode, False: stream out is writing.
        // for mutiple access to AudioVUnlockDL, need to protect when multiple access.
        //pthread_mutex_t mVUnlockReadMutex;
        int32_t mStreamOutLatency;
        pthread_mutex_t mSRCMutex;
        uint32_t   mSampleCount_Dump;
        struct timespec mULtime;
        bool mNeedBlock;
};


}; // namespace android

#endif
