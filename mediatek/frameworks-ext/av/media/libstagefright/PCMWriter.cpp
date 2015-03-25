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

/*
 * Copyright (C) 2010 The Android Open Source Project
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
#define LOG_TAG "PCMWriter"

#include <media/stagefright/PCMWriter.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/foundation/ADebug.h>
///#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/mediarecorder.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/xlog.h>

#define LOGD SXLOGD
#define LOGE SXLOGE
#define LOGV SXLOGV

namespace android
{

PCMWriter::PCMWriter(const char *filename)
    : mFile(fopen(filename, "wb")),
      mInitCheck(mFile != NULL ? OK : NO_INIT),
      mStarted(false),
      mPaused(false),
      mResumed(false),
      mRecSize(0)
{
}

PCMWriter::PCMWriter(int fd)
    : mFile(fdopen(fd, "wb")),
      mInitCheck(mFile != NULL ? OK : NO_INIT),
      mStarted(false),
      mPaused(false),
      mResumed(false)
{
    LOGD("PCMWriter::PCMWriter()");
}

PCMWriter::~PCMWriter()
{
    if (mStarted)
    {
        stop();
    }

    if (mFile != NULL)
    {
        fclose(mFile);
        mFile = NULL;
    }
}

status_t PCMWriter::initCheck() const
{
    return mInitCheck;
}

status_t PCMWriter::addSource(const sp<MediaSource> &source)
{
    if (mInitCheck != OK)
    {
        return mInitCheck;
    }

    if (mSource != NULL)
    {
        // PCM files only support a single track of audio.
        return UNKNOWN_ERROR;
    }

    sp<MetaData> meta = source->getFormat();

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    if (strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW))
    {
        return ERROR_UNSUPPORTED;
    }

    int32_t channelCount;
    int32_t sampleRate;
    CHECK(meta->findInt32(kKeyChannelCount, &channelCount));
    //CHECK_EQ(channelCount, 1);
    CHECK(meta->findInt32(kKeySampleRate, &sampleRate));
    //CHECK_EQ(sampleRate, 8000);

    mSource = source;

    LOGD("PCMWriter::addSource:write wave header+");
    //add wave header...>>>
    mWavHeader.riff_id = ID_RIFF;
    mWavHeader.riff_sz = 0;
    mWavHeader.riff_fmt = ID_WAVE;
    mWavHeader.fmt_id = ID_FMT;
    mWavHeader.fmt_sz = 16;
    mWavHeader.audio_format = FORMAT_PCM;
    mWavHeader.num_channels = channelCount;
    //mWavHeader.num_channels = 1;
    //mWavHeader.sample_rate = 8000;
    mWavHeader.sample_rate = sampleRate;
    mWavHeader.byte_rate = mWavHeader.sample_rate * mWavHeader.num_channels * 2;
    mWavHeader.block_align = mWavHeader.num_channels * 2;
    mWavHeader.bits_per_sample = 16;
    mWavHeader.data_id = ID_DATA;
    mWavHeader.data_sz = 0;

    if (fwrite(&mWavHeader, sizeof(mWavHeader), 1, mFile) != 1)
    {
        LOGE("PCMWriter::addSource:write wave header error");
        return ERROR_IO;
    }

    LOGD("PCMWriter::addSource:write wave header-");
    return OK;
}

status_t PCMWriter::start(MetaData *params)
{
    if (mInitCheck != OK)
    {
        return mInitCheck;
    }

    if (mSource == NULL)
    {
        return UNKNOWN_ERROR;
    }

    if (mStarted && mPaused)
    {
        mPaused = false;
        mResumed = true;
        return OK;
    }
    else if (mStarted)
    {
        // Already started, does nothing
        return OK;
    }

    status_t err = mSource->start();

    if (err != OK)
    {
        return err;
    }

    LOGD("PCMWriter::start");
    mRecSize = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    mReachedEOS = false;
    mDone = false;

    pthread_create(&mThread, &attr, ThreadWrapper, this);
    pthread_attr_destroy(&attr);

    mStarted = true;

    return OK;
}

status_t PCMWriter::pause()
{
    if (!mStarted)
    {
        return OK;
    }

    mPaused = true;
    return OK;
}

status_t PCMWriter::stop()
{
    if (!mStarted)
    {
        return OK;
    }

    mDone = true;

    void *dummy;
    pthread_join(mThread, &dummy);

    status_t err = (status_t) dummy;
    {
        status_t status = mSource->stop();

        if (err == OK &&
                (status != OK && status != ERROR_END_OF_STREAM))
        {
            err = status;
        }
    }

    mStarted = false;
    return err;
}

bool PCMWriter::exceedsFileSizeLimit()
{
    if (mMaxFileSizeLimitBytes == 0)
    {
        return false;
    }

    return mEstimatedSizeBytes >= mMaxFileSizeLimitBytes;
}

bool PCMWriter::exceedsFileDurationLimit()
{
    if (mMaxFileDurationLimitUs == 0)
    {
        return false;
    }

    return mEstimatedDurationUs >= mMaxFileDurationLimitUs;
}

// static
void *PCMWriter::ThreadWrapper(void *me)
{
    return (void *) static_cast<PCMWriter *>(me)->threadFunc();
}

status_t PCMWriter::threadFunc()
{
    mEstimatedDurationUs = 0;
    mEstimatedSizeBytes = 0;
    bool stoppedPrematurely = true;
    int64_t previousPausedDurationUs = 0;
    int64_t maxTimestampUs = 0;
    status_t err = OK;

    prctl(PR_SET_NAME, (unsigned long)"PCMWriter", 0, 0, 0);

    while (!mDone)
    {
        MediaBuffer *buffer;
        err = mSource->read(&buffer);

        if (err != OK)
        {
            break;
        }

        if (mPaused)
        {
            buffer->release();
            buffer = NULL;
            continue;
        }

        mEstimatedSizeBytes += buffer->range_length();

        if (exceedsFileSizeLimit())
        {
            buffer->release();
            buffer = NULL;
            notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED, 0);
            break;
        }

        int64_t timestampUs;
        CHECK(buffer->meta_data()->findInt64(kKeyTime, &timestampUs));

        if (timestampUs > mEstimatedDurationUs)
        {
            mEstimatedDurationUs = timestampUs;
        }

        if (mResumed)
        {
            previousPausedDurationUs += (timestampUs - maxTimestampUs - 20000);
            mResumed = false;
        }

        timestampUs -= previousPausedDurationUs;
        LOGV("time stamp: %lld, previous paused duration: %lld",
             timestampUs, previousPausedDurationUs);

        if (timestampUs > maxTimestampUs)
        {
            maxTimestampUs = timestampUs;
        }

        if (exceedsFileDurationLimit())
        {
            buffer->release();
            buffer = NULL;
            notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_DURATION_REACHED, 0);
            break;
        }

        ssize_t n = fwrite(
                        (const uint8_t *)buffer->data() + buffer->range_offset(),
                        1,
                        buffer->range_length(),
                        mFile);

        if (n < (ssize_t)buffer->range_length())
        {
            buffer->release();
            buffer = NULL;

            break;
        }
        else
        {
            mRecSize += n;
        }

        // XXX: How to tell it is stopped prematurely?
        if (stoppedPrematurely)
        {
            stoppedPrematurely = false;
        }

        buffer->release();
        buffer = NULL;
    }

    if (stoppedPrematurely)
    {
        notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_TRACK_INFO_COMPLETION_STATUS, UNKNOWN_ERROR);
    }

    //write wav header again when end
    mWavHeader.data_sz = mRecSize;
    mWavHeader.riff_sz = mRecSize + 8 + 16 + 8;

    fseek(mFile, 0, SEEK_SET);
    fwrite(&mWavHeader, sizeof(mWavHeader), 1, mFile);

    fflush(mFile);
    fclose(mFile);
    mFile = NULL;
    mReachedEOS = true;

    if (err == ERROR_END_OF_STREAM)
    {
        return OK;
    }

    return err;
}

bool PCMWriter::reachedEOS()
{
    return mReachedEOS;
}

}  // namespace android

