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
 
#define LOG_TAG "VibSpk"
#include "utils/Log.h"
#include "cutils/xlog.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <system/audio.h>
#include <signal.h>
#include <time.h>

#include <binder/IServiceManager.h>
#include <hardware/audio.h>
#include <AudioSystem.h>
#include "VibSpkAudioPlayer.h"
#include <linux/rtpm_prio.h>

#ifndef FAKE_VIBSPK
//#include "AudioResourceManager.h"
//#include "AudioMTKHardware.h"
//#include "AudioType.h"
#endif

#define VIBSPK_STATE_INIT      0
#define VIBSPK_STATE_STARTING  1
#define VIBSPK_STATE_PLAYING   2
#define VIBSPK_STATE_STOPPING  3

#define VIBSPK_MINIMUM_PERIOD 60

namespace android
{
//#if defined(MTK_VIBSPK_SUPPORT)

VibSpkAudioPlayer *VibSpkAudioPlayerInstance = NULL;

//Timer *VibSpkTimeout = NULL;

VibSpkAudioPlayer::VibSpkAudioPlayer(audio_stream_type_t streamType, float volume, bool threadCanCallJava)
{
	
    SXLOGD(" constructorx\n");
    mSamplingRate      = 44100;
    mThreadCanCallJava = threadCanCallJava;
    mStreamType        = streamType;
    mVolume            = volume;
    mpAudioTrack       = 0;
    mState             = VIBSPK_STATE_INIT;
    // Generate silence by chunks of 20 ms to keep cadencing precision
    mProcessSize = (mSamplingRate * 20) / 1000;
    if (initAudioTrack()) {
        SXLOGD("INIT OK, time: %d\n", (unsigned int)(systemTime()/1000000));
    } else {
        SXLOGD("!!!INIT FAILED!!!\n");
    }
}	

VibSpkAudioPlayer::~VibSpkAudioPlayer()
{
    SXLOGD("destructor\n");
    
/*    if (mpAudioTrack) {
        SXLOGD("Delete Track: %p\n", mpAudioTrack);
        delete mpAudioTrack;
        mpAudioTrack = 0;
    }*/
}

VibSpkAudioPlayer *VibSpkAudioPlayer::getInstance()
{
    if(VibSpkAudioPlayerInstance == NULL)
    {
        SXLOGD("GetInstance, Constructor\n");
        VibSpkAudioPlayerInstance = new VibSpkAudioPlayer(AUDIO_STREAM_VIBSPK, 0, false);
        //VibSpkAudioPlayerInstance = new VibSpkAudioPlayer(AUDIO_STREAM_NOTIFICATION, 0, false);
        SXLOGD("GetInstance, Constructor done\n");
    }
    return VibSpkAudioPlayerInstance;
    
}

bool VibSpkAudioPlayer::initAudioTrack()
{
    /*if (mpAudioTrack) 
    {
        delete mpAudioTrack;
        mpAudioTrack = 0;
    }*/
    // Open audio track in mono, PCM 16bit, default sampling rate, default buffer size
    if(mpAudioTrack.get() == NULL)
    {
        mpAudioTrack = new AudioTrack();
        if (mpAudioTrack.get() == NULL)
        {
            SXLOGD("AudioTrack allocation failed");
            goto initAudioTrack_exit;
        }
//        SXLOGD("Create Track: %p\n", mpAudioTrack); 
        mpAudioTrack->set(mStreamType, 
                          0, 
                          AUDIO_FORMAT_PCM_16_BIT, 
                          AUDIO_CHANNEL_OUT_STEREO, 
                          0, 
                          AUDIO_OUTPUT_FLAG_NONE, 
                          audioCallback, 
                          this,
                          0,
                          0,
                          mThreadCanCallJava,
                          0);
        
        if (mpAudioTrack->initCheck() != NO_ERROR) 
        {
            SXLOGD("AudioTrack->initCheck failed");
            goto initAudioTrack_exit;
        }
        mpAudioTrack->setVolume(mVolume, mVolume);
    }
    return true;

initAudioTrack_exit:

    // Cleanup
    if (mpAudioTrack.get()) 
    {
//           SXLOGD("Delete Track I: %p\n", mpAudioTrack);
		mpAudioTrack.clear();
    }

    return false;
}

bool VibSpkAudioPlayer::start()
{
    status_t lStatus;
//    SXLOGD("start:%x\n", mpAudioTrack);
    mpAudioTrack->start();
    return true;
}

void VibSpkAudioPlayer::stop()
{
//    SXLOGD("stop:%x\n", mpAudioTrack);
    mpAudioTrack->stop();
}

void VibSpkAudioPlayer::audioCallback(int event, void* user, void *info) 
{
    if (event != AudioTrack::EVENT_MORE_DATA) 
        return;

    AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);
    short *lpOut = buffer->i16;
    SXLOGD("audioCallback:%x", buffer->size);
    if (buffer->size == 0) 
        return;
        
    //memset(lpOut, 0, buffer->size);
    return;
}



static Mutex VibSpkMutex;
static Condition VibSpkCondition;
static int VibSpkTimer = 0;
static int VibSpkCount = 0;
static bool VibSpkExit = false;
static bool VibSpkStop = false;
static int VibspkState = VIBSPK_STATE_INIT;
// golbal hardware pointer
//static    android::AudioMTKHardware *gAudioHardware = NULL;

int AudioTrackThread(void *p)
{
   VibSpkCondition.signal();
   VibSpkMutex.lock();
   VibSpkAudioPlayer::getInstance()->mState = VIBSPK_STATE_PLAYING;
   VibspkState = VIBSPK_STATE_PLAYING;
   VibSpkMutex.unlock();
   AudioSystem::setParameters(0, (String8)"SET_VIBSPK_RAMPDOWN=0");	
   AudioSystem::setParameters(0, (String8)"SET_VIBSPK_ENABLE=1");
   VibSpkAudioPlayer::getInstance()->start();
   
   while(1)
   {
   //gAudioHardware->SetVibSpkRampControl(1);
      SXLOGD("VIBRATOR_AucioTrackThread:%x %x %X\n", VibSpkTimer, VibSpkExit, VibspkState);
     /* if(VibSpkTimer == 40)
         AudioSystem::setParameters(0, (String8)"SET_VIBSPK_RAMPDOWN=1");*/
         
      if(VibSpkTimer == 0 || VibSpkExit == true)
      {
         AudioSystem::setParameters(0, (String8)"SET_VIBSPK_ENABLE=0");
         VibSpkAudioPlayer::getInstance()->stop();
         VibSpkMutex.lock();
         VibSpkAudioPlayer::getInstance()->mState = VIBSPK_STATE_INIT;
         VibspkState = VIBSPK_STATE_INIT;
         VibSpkTimer = 0;
         VibSpkMutex.unlock();
         break;
      }
      usleep(1000);
      VibSpkTimer--;
      VibSpkCount++;
      if( VibSpkStop == true && (VibSpkCount >= VIBSPK_MINIMUM_PERIOD) )
          VibSpkExit = true;
   }
   return 1;
}


bool VIBRATOR_SPKON(unsigned int timeoutms)
{
    long timeout_temp;
    int stat;
    timeout_temp = timeoutms;
    //Mutex::Autolock l(VibSpkMutex);
    VibSpkMutex.lock();
    SXLOGD("VIBRATOR_SPKON:%x %x %x\n", VibSpkAudioPlayerInstance, timeoutms, VibspkState);
    if(VibSpkAudioPlayer::getInstance()->mState == VIBSPK_STATE_INIT)
    {
        SXLOGD("VIBRATOR_SPKON_Process\n");
        VibSpkTimer = timeoutms;
        if(VibSpkTimer < VIBSPK_MINIMUM_PERIOD)
            VibSpkTimer = VIBSPK_MINIMUM_PERIOD;
        VibSpkCount = 0;
        VibSpkExit = false;
        VibSpkStop = false;
        VibSpkAudioPlayer::getInstance()->mState = VIBSPK_STATE_STARTING;
        VibspkState = VIBSPK_STATE_STARTING;
        createThreadEtc(AudioTrackThread, NULL, "VibSpk audio player", ANDROID_PRIORITY_AUDIO);
        VibSpkCondition.waitRelative(VibSpkMutex, seconds(3));	
        VibSpkMutex.unlock();
        SXLOGD("VIBRATOR_SPKON_ProcessDone:%x\n", VibspkState);
        return true;
    }
    VibSpkMutex.unlock();
    return false;
}

bool VIBRATOR_SPKOFF()
{
	  VibSpkMutex.lock();
	  SXLOGD("VIBRATOR_SPKOFF:%x %x\n", VibSpkAudioPlayerInstance, VibspkState);
    if(VibSpkAudioPlayer::getInstance()->mState == VIBSPK_STATE_PLAYING)
    {
        if(VibSpkCount >= VIBSPK_MINIMUM_PERIOD)
        {
            SXLOGD("VIBRATOR_SPKOFF_Process\n");
            VibSpkAudioPlayer::getInstance()->mState = VIBSPK_STATE_STOPPING;
            VibspkState = VIBSPK_STATE_STOPPING;
            VibSpkExit = true;
            VibSpkMutex.unlock();
            return true;
        }
        else
        {
            VibSpkStop = true;
            SXLOGD("VIBRATOR_SPKOFF_Counter:%x\n", VibSpkCount);
        }
    }
    VibSpkMutex.unlock();
    return false;
}

int VIBRATOR_SPKEXIST()
{
    //Mutex::Autolock l(VibSpkMutex);
    VibSpkMutex.lock();
    if(VibSpkAudioPlayer::getInstance()->mState == VIBSPK_STATE_INIT)
    {
    	  VibSpkMutex.unlock();
        return 0;
    }
    else
    {
    	  VibSpkMutex.unlock();
        return 1;
    }
}
//#endif //MTK_VIBSPK_SUPPORT

} // end namespace android

