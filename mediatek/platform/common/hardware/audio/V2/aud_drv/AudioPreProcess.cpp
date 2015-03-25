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
#include <cutils/properties.h>
#include "AudioPreProcess.h"



#define LOG_TAG "AudioPreProcess"

namespace android
{

AudioPreProcess::AudioPreProcess()
{
    Mutex::Autolock lock(mLock);
    ALOGD("AudioPreProcess constructor");


    num_preprocessors = 0;;
    need_echo_reference = false;

    proc_buf_in = NULL;
    proc_buf_out = NULL;
    proc_buf_size = 0;
    proc_buf_frames = 0;

    ref_buf = NULL;
    ref_buf_size = 0;
    ref_buf_frames = 0;

    echo_reference = NULL;

    for (int i = 0; i < MAX_PREPROCESSORS; i++)
        preprocessors[i] = {0};

    mInChn = 0;
    mInSampleRate = 16000;

    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    if (!mAudioSpeechEnhanceInfoInstance)
    {
        ALOGE("mAudioSpeechEnhanceInfoInstance get fail");
    }
    ALOGD("AudioPreProcess constructor--");
}


AudioPreProcess::~AudioPreProcess()
{

    MutexLock();

    ALOGD("AudioPreProcess destructor");
    if (proc_buf_in)
    {
        free(proc_buf_in);
        proc_buf_in = NULL;
    }
    /*
            if(proc_buf_out)
                free(proc_buf_out);
    */
    if (ref_buf)
    {
        free(ref_buf);
        ref_buf = NULL;
    }

    MutexUnlock();
    if (echo_reference != NULL)
    {
        stop_echo_reference(echo_reference);
    }

    ALOGD("~AudioPreProcess--");
}

void AudioPreProcess::stop_echo_reference(struct echo_reference_itfe *reference)
{
    Mutex::Autolock lock(mLock);
    ALOGD("stop_echo_reference");
    /* stop reading from echo reference */
    if (echo_reference != NULL && echo_reference == reference)
    {
        echo_reference->read(reference, NULL);
        clear_echo_reference(reference);
    }
}

void AudioPreProcess::clear_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("clear_echo_reference %p", reference);
    //if ((mHw->mStreamHandler->echo_reference!= NULL) && (mHw->mStreamHandler->echo_reference == reference) &&
    if ((reference == echo_reference) && (reference != NULL))
    {
        //        if (mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
        remove_echo_reference(reference);

        release_echo_reference(reference);
        echo_reference = NULL;
        //mHw->mStreamHandler->echo_reference = NULL;
    }
}

void AudioPreProcess::remove_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("remove_echo_reference %d", reference);
    mAudioSpeechEnhanceInfoInstance->remove_echo_reference(reference);
}

struct echo_reference_itfe *AudioPreProcess::start_echo_reference(audio_format_t format,
                                                                  uint32_t channel_count, uint32_t sampling_rate)
{
    Mutex::Autolock lock(mLock);
    ALOGD("start_echo_reference,channel_count=%d,sampling_rate=%d,echo_reference=%x", channel_count, sampling_rate, echo_reference);
    clear_echo_reference(echo_reference);
    /* echo reference is taken from the low latency output stream used
         * for voice use cases */
    if (mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
    {
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT 
        //use the same setting with MTK VoIP echo reference data format
        uint32_t wr_channel_count = 2;  //android limit in Echo_reference.c, must be 2!!
        uint32_t wr_sampling_rate = 16000;
#else
        uint32_t wr_channel_count = mAudioSpeechEnhanceInfoInstance->GetOutputChannelInfo();
        uint32_t wr_sampling_rate = mAudioSpeechEnhanceInfoInstance->GetOutputSampleRateInfo();
#endif
        mInChn = channel_count;
        mInSampleRate = sampling_rate;
        ALOGD("start_echo_reference,wr_channel_count=%d,wr_sampling_rate=%d", wr_channel_count, wr_sampling_rate);
        int status = create_echo_reference(AUDIO_FORMAT_PCM_16_BIT,
                                           channel_count,
                                           sampling_rate,
                                           AUDIO_FORMAT_PCM_16_BIT,
                                           wr_channel_count,
                                           wr_sampling_rate,
                                           &echo_reference);
        if (status == 0)
        {
            //mHw->mStreamHandler->echo_reference = echo_reference;
            add_echo_reference(echo_reference);
        }
        else
        {
            ALOGW("create_echo_reference fail");
        }
    }
    return echo_reference;
}

void AudioPreProcess::add_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("add_echo_reference reference=%p", reference);
    mAudioSpeechEnhanceInfoInstance->add_echo_reference(reference);
}

void AudioPreProcess::push_echo_reference(size_t frames)
{

    Mutex::Autolock lock(mLock);
    /* read frames from echo reference buffer and update echo delay
     * in->ref_buf_frames is updated with frames available in in->ref_buf */
    int32_t delay_us = update_echo_reference(frames) / 1000;
    int i;
    audio_buffer_t buf;

    if (ref_buf_frames < frames)
    {
        frames = ref_buf_frames;
    }

    buf.frameCount = frames;
    buf.raw = ref_buf;

    for (i = 0; i < num_preprocessors; i++)
    {
        if ((*preprocessors[i].effect_itfe)->process_reverse == NULL)
        {
            continue;
        }

        (*preprocessors[i].effect_itfe)->process_reverse(preprocessors[i].effect_itfe,
                                                         &buf,
                                                         NULL);
        set_preprocessor_echo_delay(preprocessors[i].effect_itfe, delay_us);
    }

    ref_buf_frames -= buf.frameCount;
    if (ref_buf_frames)
    {
        //        ALOGV("push_echo_reference,ref_buf_frames=%d",ref_buf_frames);
        memcpy(ref_buf,
               ref_buf + buf.frameCount * mInChn,
               ref_buf_frames * mInChn * sizeof(int16_t));
    }
}

int32_t AudioPreProcess::update_echo_reference(size_t frames)
{

    struct echo_reference_buffer b;
    b.delay_ns = 0;

    //    ALOGD("update_echo_reference, frames = [%d], ref_buf_frames = [%d],  "
    //          "b.frame_count = [%d]", frames, ref_buf_frames, frames - ref_buf_frames);
    if (ref_buf_frames < frames)
    {
        if (ref_buf_size < frames)
        {
            ref_buf_size = frames;
            ref_buf = (int16_t *)realloc(ref_buf, frames * sizeof(int16_t) * mInChn);
            ALOG_ASSERT((ref_buf != NULL),
                        "update_echo_reference() failed to reallocate ref_buf");
            ALOGD("update_echo_reference(): ref_buf %p extended to %d bytes",
                  ref_buf, frames * sizeof(int16_t)*mInChn);
        }
        b.frame_count = frames - ref_buf_frames;
        //fixme? raw address?
        b.raw = (void *)(ref_buf + ref_buf_frames * mInChn);

        get_capture_delay(frames, &b);

        if (echo_reference->read(echo_reference, &b) == 0)
        {
            ref_buf_frames += b.frame_count;
            /*            ALOGD("update_echo_reference(): ref_buf_frames:[%d], "
                                "ref_buf_size:[%d], frames:[%d], b.frame_count:[%d]",
                             ref_buf_frames, ref_buf_size, frames, b.frame_count);*/
        }
    }
    else
    {
        ALOGV("update_echo_reference(): NOT enough frames to read ref buffer");
    }
    return b.delay_ns;
}

int AudioPreProcess::set_preprocessor_echo_delay(effect_handle_t handle, int32_t delay_us)
{
    uint32_t buf[sizeof(effect_param_t) / sizeof(uint32_t) + 2];
    effect_param_t *param = (effect_param_t *)buf;

    param->psize = sizeof(uint32_t);
    param->vsize = sizeof(uint32_t);
    *(uint32_t *)param->data = AEC_PARAM_ECHO_DELAY;
    *((int32_t *)param->data + 1) = delay_us;

    return set_preprocessor_param(handle, param);
}

int AudioPreProcess::set_preprocessor_param(effect_handle_t handle, effect_param_t *param)
{
    uint32_t size = sizeof(int);
    uint32_t psize = ((param->psize - 1) / sizeof(int) + 1) * sizeof(int) +
                     param->vsize;

    int status = (*handle)->command(handle,
                                    EFFECT_CMD_SET_PARAM,
                                    sizeof(effect_param_t) + psize,
                                    param,
                                    &size,
                                    &param->status);
    if (status == 0)
    {
        status = param->status;
    }

    return status;
}

void AudioPreProcess::get_capture_delay(size_t frames, struct echo_reference_buffer *buffer)
{
    //FIXME:: calculate for more precise time delay
    struct timespec tstamp;
    long buf_delay = 0;
    long rsmp_delay = 0;
    long kernel_delay = 0;
    long delay_ns = 0;


    int rc = clock_gettime(CLOCK_MONOTONIC, &tstamp);
    if (rc != 0)
    {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGD("get_capture_delay(): clock_gettime error");
        return;
    }


    buf_delay = (long)(((int64_t)(proc_buf_frames) * 1000000000) / mInSampleRate);

    delay_ns = kernel_delay + buf_delay + rsmp_delay;

    buffer->time_stamp = tstamp;
    buffer->delay_ns   = delay_ns;
    //    ALOGV("get_capture_delay time_stamp = [%ld].[%ld], delay_ns: [%d]",
    //         buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns);
}

bool AudioPreProcess::MutexLock(void)
{
    mLock.lock();
    return true;
}
bool AudioPreProcess::MutexUnlock(void)
{
    mLock.unlock();
    return true;
}


#define GET_COMMAND_STATUS(status, fct_status, cmd_status) \
    do {                                           \
        if (fct_status != 0)                       \
            status = fct_status;                   \
        else if (cmd_status != 0)                  \
            status = cmd_status;                   \
    } while(0)

int AudioPreProcess::in_configure_reverse(uint32_t channel_count, uint32_t sampling_rate)
{
    Mutex::Autolock lock(mLock);
    int32_t cmd_status;
    uint32_t size = sizeof(int);
    effect_config_t config;
    int32_t status = 0;
    int32_t fct_status = 0;
    int i;
    ALOGD("in_configure_reverse");
    //fixme:: currently AEC stereo channel has problem, force using mono while AEC enable
    if (num_preprocessors > 0)
    {
        config.inputCfg.channels = 1;//mChNum;channel_count
        config.outputCfg.channels = 1;//mHw->mStreamHandler->mOutput[0]->GetChannelInfo();
        config.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        config.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        config.inputCfg.samplingRate = sampling_rate;
        config.outputCfg.samplingRate = mAudioSpeechEnhanceInfoInstance->GetOutputSampleRateInfo();
        config.inputCfg.mask =
            (EFFECT_CONFIG_SMP_RATE | EFFECT_CONFIG_CHANNELS | EFFECT_CONFIG_FORMAT);
        config.outputCfg.mask =
            (EFFECT_CONFIG_SMP_RATE | EFFECT_CONFIG_CHANNELS | EFFECT_CONFIG_FORMAT);

        for (i = 0; i < num_preprocessors; i++)
        {
            if ((*preprocessors[i].effect_itfe)->process_reverse == NULL)
            {
                continue;
            }
            fct_status = (*(preprocessors[i].effect_itfe))->command(
                             preprocessors[i].effect_itfe,
                             EFFECT_CMD_SET_CONFIG_REVERSE,
                             sizeof(effect_config_t),
                             &config,
                             &size,
                             &cmd_status);

            GET_COMMAND_STATUS(status, fct_status, cmd_status);
        }
    }
    ALOGD("in_configure_reverse,status=%d", status);
    return status;
}

// ----------------------------------------------------------------------------
}

