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
 
#ifndef ANDROID_VIBSPK_AUDIOPLAYER_H
#define ANDROID_VIBSPK_AUDIOPLAYER_H

#include <utils/threads.h>

#include <media/MediaPlayerInterface.h>
#include <media/AudioTrack.h>
#include <media/AudioRecord.h>


namespace android
{
extern bool VIBRATOR_SPKON(unsigned int timeoutms);
extern bool VIBRATOR_SPKOFF();
extern int VIBRATOR_SPKEXIST();

class VibSpkAudioPlayer
{
public:
    VibSpkAudioPlayer(audio_stream_type_t streamType, float volume, bool threadCanCallJava);
    ~VibSpkAudioPlayer();
    static VibSpkAudioPlayer *getInstance();
    bool start();
    void stop();
    unsigned short      mState;
    
private:
    unsigned short      mLoopCounter; // Current tone loopback count
    int                 mSamplingRate;  // AudioFlinger Sampling rate
    sp<AudioTrack>      mpAudioTrack;  // Pointer to audio track used for playback
    Mutex               mLock;          // Mutex to control concurent access to ToneGenerator object from audio callback and application API
    Mutex               mCbkCondLock; // Mutex associated to mWaitCbkCond
    Condition           mWaitCbkCond; // condition enabling interface to wait for audio callback completion after a change is requested
    float               mVolume;  // Volume applied to audio track
    audio_stream_type_t mStreamType; // Audio stream used for output
    unsigned int        mProcessSize;  // Size of audio blocks generated at a time by audioCallback() (in PCM frames).
    bool                initAudioTrack();
    static void         audioCallback(int event, void* user, void *info);
    bool                prepareWave();
    unsigned int        numWaves(unsigned int segmentIdx);
    void                clearWaveGens();
    bool                mThreadCanCallJava;
};
	
};   //namespace android

#endif   //ANDROID_VIBSPK_AUDIOPLAYER_H