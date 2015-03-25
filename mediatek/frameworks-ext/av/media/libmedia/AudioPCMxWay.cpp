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

/* //device/extlibs/pv/android/AudioTrack.cpp
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#define LOG_TAG "AudioPCMxWay"

#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#include <sched.h>
#include <sys/resource.h>

#include <media/AudioPCMxWay.h>
#include <media/AudioSystem.h>

#include <utils/Log.h>
#include <cutils/xlog.h>
#include <binder/MemoryDealer.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <utils/Timers.h>
#include <cutils/atomic.h>


#define THREAD_PRIORITY_AUDIO_CLIENT (ANDROID_PRIORITY_AUDIO)
#define WAIT_PERIOD_MS          10

namespace android
{

// ---------------------------------------------------------------------------

AudioPCMxWay::AudioPCMxWay()
    : mStatus(NO_INIT),
      mAudioPCMxWayThread(NULL),
      mStarted(false)
{
}

AudioPCMxWay::AudioPCMxWay(
    int streamType,
    callback_t cbf,
    void *user
)
    : mStatus(NO_INIT),
      mAudioPCMxWayThread(NULL),
      mStarted(false)
{
    mStatus = set(streamType, cbf, user);
}


AudioPCMxWay::~AudioPCMxWay()
{

    if (mStatus == NO_ERROR)
    {
        // Make sure that callback function exits in the case where
        // it is looping on buffer full condition in obtainBuffer().
        // Otherwise the callback thread will never exit.
        //stop();
        SXLOGD("Enter AudioPCMxWay::~AudioPCMxWay(),mAudioPCMxWayThread = %p", mAudioPCMxWayThread.get());

        if (mAudioPCMxWayThread != 0)
        {
            mAudioPCMxWayThread->requestExitAndWait();
            //mAudioPCMxWayThread->requestExit();
            mAudioPCMxWayThread.clear();
        }

        IPCThreadState::self()->flushCommands();
    }

    SXLOGD("Exit AudioPCMxWay::~AudioPCMxWay()");
}

status_t AudioPCMxWay::set(int streamType, callback_t cbf, void *user)
{

    if (cbf != 0)
    {
        mAudioPCMxWayThread = new AudioPCMxWayThread(*this);

        if (mAudioPCMxWayThread == 0)
        {
            SXLOGE("Could not create callback thread");
            return NO_INIT;
        }
    }

    mStatus = NO_ERROR;

    mStreamType = streamType;
    mCbf = cbf;
    mUserData = user;
    //mActive = 0;

    return NO_ERROR;
}

status_t AudioPCMxWay::initCheck() const
{
    return mStatus;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void AudioPCMxWay::start()
{
    sp<AudioPCMxWayThread> t = mAudioPCMxWayThread;

    if (mStarted)
    {
        return;
    }

    mStarted = true;

    SXLOGD("start %p", this);

    if (t != 0)
    {
        if (t->exitPending())
        {
            if (t->requestExitAndWait() == WOULD_BLOCK)
            {
                SXLOGE("AudioTrack::start called from thread");
                return;
            }
        }

        t->mLock.lock();
    }


    {
        if (t != 0)
        {
            t->run("AudioPCMxWayThread", THREAD_PRIORITY_AUDIO_CLIENT);
        }
        else
        {
            setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY_AUDIO_CLIENT);
        }

        if (mStreamType == PCMXWAY_OUTPUT)
        {
            AudioSystem::xWayPlay_Start(8000);
        }
        else
        {
            AudioSystem::xWayRec_Start(8000);
        }

    }

    if (t != 0)
    {
        t->mLock.unlock();
    }
}

void AudioPCMxWay::stop()
{
    sp<AudioPCMxWayThread> t = mAudioPCMxWayThread;

    mStarted = false;

    SXLOGD("AudioPCMxWay::stop()");

    if (t != 0)
    {
        t->mLock.lock();
    }

    {

        if (t != 0)
        {
            t->requestExit();
        }
        else
        {
            setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_NORMAL);
        }

        if (mStreamType == PCMXWAY_OUTPUT)
        {
            AudioSystem::xWayPlay_Stop();
        }
        else
        {
            AudioSystem::xWayRec_Stop();
        }
    }

    if (t != 0)
    {
        t->mLock.unlock();
    }
}

// -------------------------------------------------------------------------

ssize_t AudioPCMxWay::write(const void *buffer, size_t userSize)
{

    if (ssize_t(userSize) < 0)
    {
        // sanity-check. user is most-likely passing an error code.
        SXLOGE("AudioPCMxWay::write(buffer=%p, size=%u (%d)",
               buffer, userSize, userSize);
        return BAD_VALUE;
    }

    //SXLOGV("write %p: %d bytes, mActive=%d", this, userSize, mActive);

    ssize_t written = 0;
    int8_t *src = (int8_t *)buffer;

    int tryCount = 0;

    do
    {

        size_t toWrite;

        toWrite = AudioSystem::xWayPlay_Write(src, userSize);
        src += toWrite;

        if (toWrite == 0)
        {
            usleep(5000);
        }

        tryCount++;

        if (tryCount > 5)
        {
            break;
        }

        userSize -= toWrite;
        written += toWrite;

    }
    while (userSize && mStarted);

    return written;
}


ssize_t AudioPCMxWay::read(void *buffer, size_t userSize)
{
    ssize_t read = 0;
    Buffer audioBuffer;
    int8_t *dst = static_cast<int8_t *>(buffer);

    if (ssize_t(userSize) < 0)
    {
        // sanity-check. user is most-likely passing an error code.
        SXLOGE("AudioRecord::read(buffer=%p, size=%u (%d)",
               buffer, userSize, userSize);
        return BAD_VALUE;
    }

    size_t bytesRead = 0;
    int tryCount = 0;

    do
    {

        bytesRead = AudioSystem::xWayRec_Read(dst, userSize);

        if (bytesRead == 0)
        {
            usleep(5000);
        }

        tryCount++;

        if (tryCount > 5)
        {
            break;
        }

        dst += bytesRead;
        userSize -= bytesRead;
        read += bytesRead;

    }
    while (userSize && mStarted);

    return read;
}

// -------------------------------------------------------------------------

bool AudioPCMxWay::processAudioBuffer(const sp<AudioPCMxWayThread>& thread)
{
    Buffer audioBuffer;
    uint32_t frames;
    size_t writtenSize;

    if (mStreamType == PCMXWAY_INPUT)
    {
        return false;
    }

    do
    {

        size_t reqSize = 0;

        if (!mStarted)
        {
            break;
        }

        reqSize = AudioSystem::xWayPlay_GetFreeBufferCount();

        while (reqSize < 320)
        {
            usleep(WAIT_PERIOD_MS * 1000);

            if (!mStarted)
            {
                return false;
            }

            reqSize =  AudioSystem::xWayPlay_GetFreeBufferCount();
        }

        sp<MemoryDealer>        mMemoryDealer;
        sp<IMemory>             mCblkMemory;
        mMemoryDealer = new MemoryDealer(1024 * 1024);

        audioBuffer.size = AudioSystem::xWayPlay_GetFreeBufferCount();
        mCblkMemory = mMemoryDealer->allocate(audioBuffer.size);

        if (mCblkMemory != 0)
        {
            audioBuffer.raw  = (void *)mCblkMemory->pointer();
        }

        mCbf(EVENT_MORE_DATA, mUserData, &audioBuffer);
        writtenSize = audioBuffer.size;
        reqSize = AudioSystem::xWayPlay_GetFreeBufferCount();

        // Sanity check on returned size
        if (ssize_t(writtenSize) <= 0)
        {
            // The callback is done filling buffers
            // Keep this thread going to handle timed events and
            // still try to get more data in intervals of WAIT_PERIOD_MS
            // but don't just loop and block the CPU, so wait
            usleep(WAIT_PERIOD_MS * 1000);

            if (!mStarted)
            {
                return false;
            }
        }

        if (writtenSize > reqSize)
        {
            writtenSize = reqSize;
        }

        audioBuffer.size = writtenSize;

        write(audioBuffer.raw, audioBuffer.size);

    }
    while (0);

    return true;
}


// =========================================================================

AudioPCMxWay::AudioPCMxWayThread::AudioPCMxWayThread(AudioPCMxWay &receiver)
    : mReceiver(receiver)
{
}

bool AudioPCMxWay::AudioPCMxWayThread::threadLoop()
{
    return mReceiver.processAudioBuffer(this);
}

status_t AudioPCMxWay::AudioPCMxWayThread::readyToRun()
{
    return NO_ERROR;
}

void AudioPCMxWay::AudioPCMxWayThread::onFirstRef()
{
}

// =========================================================================

}; // namespace android

