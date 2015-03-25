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

//#define LOG_NDEBUG 0
#define LOG_TAG "OggWriter"

#include <media/stagefright/OggWriter.h>
#include <media/stagefright/MediaBuffer.h>
///#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/mediarecorder.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/xlog.h>

#include <media/stagefright/Utils.h>
#include <linux/rtpm_prio.h>

#define LOGD SXLOGD
#define LOGE SXLOGE
#define LOGV SXLOGV


namespace android
{

OggWriter::OggWriter(const char *filename)
    : mFile(fopen(filename, "wb")),
      mInitCheck(mFile != NULL ? OK : NO_INIT),
      mStarted(false),
      mPaused(false),
      mResumed(false),
      mPausedflag(false),
      iPausedTime(0),
      mSampleRate(0)
{
}

OggWriter::OggWriter(int fd)
    : mFile(fdopen(fd, "wb")),
      mInitCheck(mFile != NULL ? OK : NO_INIT),
      mStarted(false),
      mPaused(false),
      mResumed(false),
      mPausedflag(false),
      iPausedTime(0),
      mSampleRate(0)
{
    LOGD("OggWriter::OggWriter");
}

OggWriter::~OggWriter()
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

status_t OggWriter::initCheck() const
{
    return mInitCheck;
}

status_t OggWriter::addSource(const sp<MediaSource> &source)
{
    LOGV("OggWriter::addSource");

    if (mInitCheck != OK)
    {
        return mInitCheck;
    }

    if (mSource != NULL)
    {
        // Ogg files only support a single track of audio.
        return UNKNOWN_ERROR;
    }

    sp<MetaData> meta = source->getFormat();

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    if (strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS))
    {
        return ERROR_UNSUPPORTED;
    }

    meta->findInt32(kKeySampleRate, &mSampleRate);
    mSource = source;
    return OK;
}

status_t OggWriter::start(MetaData *params)
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

    LOGD("OggWriter::start");
    status_t err = mSource->start();

    if (err != OK)
    {
        return err;
    }

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

status_t OggWriter::pause()
{
    if (!mStarted)
    {
        return OK;
    }

    iPausedTime++;
    LOGD("OggWriter::pause:%d time,", iPausedTime);
    mPaused = true;
    mPausedflag = true;
    //Add by Mtk80721 debug pause error
    //mSource->pause();
    //
    return OK;
}

status_t OggWriter::stop()
{
    if (!mStarted)
    {
        return OK;
    }

    mDone = true;
    //status_t status = mSource->stop();

    LOGD("stop+");
    void *dummy;
    pthread_join(mThread, &dummy);
    LOGD("write thread exit");
    status_t err = (status_t) dummy;
    {
        status_t status = mSource->stop();
        LOGD("msrc stop");

        if (err == OK && (status != OK && status != ERROR_END_OF_STREAM))
        {
            err = status;
        }
    }

    mStarted = false;
    LOGD("stop-");
    return err;
}

bool OggWriter::exceedsFileSizeLimit()
{
    if (mMaxFileSizeLimitBytes == 0)
    {
        return false;
    }

    return mEstimatedSizeBytes >= mMaxFileSizeLimitBytes;
}

bool OggWriter::exceedsFileDurationLimit()
{
    if (mMaxFileDurationLimitUs == 0)
    {
        return false;
    }

    return mEstimatedDurationUs >= mMaxFileDurationLimitUs;
}

// static
void *OggWriter::ThreadWrapper(void *me)
{
    return (void *) static_cast<OggWriter *>(me)->threadFunc();
}

status_t OggWriter::threadFunc()
{
    mEstimatedDurationUs = 0;
    mEstimatedSizeBytes = 0;
    bool stoppedPrematurely = true;
    int64_t previousPausedDurationUs = 0;
    int64_t maxTimestampUs = 0;
    status_t err = OK;
    int64_t ltotalSize = 0;
    int64_t timestampUs = 0;
    //
    int64_t tsUsPauseBeg = 0;
    int64_t tsUsPauseEnd = 0;
    //paused sample count
    int64_t smpPaused = 0;
    //
    prctl(PR_SET_NAME, (unsigned long)"OggWriter", 0, 0, 0);

    //struct sched_param param;
    //param.sched_priority = RTPM_PRIO_OMX_AUDIO;
    //sched_setscheduler(0, SCHED_RR, &param);
    //androidSetThreadPriority(0, ANDROID_PRIORITY_AUDIO);
    //while (!mDone) {
    while (1 == 1)
    {
        MediaBuffer *buffer = NULL;
        MediaBuffer *buffer1 = NULL;

        if (mDone)
        {
            buffer = new MediaBuffer(0);
            buffer1 = buffer;
        }

        LOGV("OggWriter::threadFunc:mSource->read+:buffer=%p,buffer1=%p", buffer, buffer1);
        err = mSource->read(&buffer);
        LOGV("OggWriter::threadFunc:mSource->read-,err=%d,buffer=%p", err, buffer);

        if (err != OK)
        {
            break;
        }

        LOGV("OggWriter::threadFunc:buffer->range_length()=%d", buffer->range_length());

        //buffer->range_length() == 0, ogg encoder SWIP caching data
        if (mPaused || buffer->range_length() == 0)
        {
            //mtk80721 deal pause time error+
            if (mPaused && mPausedflag)
            {
                buffer->meta_data()->findInt64(kKeyTime, &tsUsPauseBeg);
                LOGD("OggWriter::threadFunc,pausetime=%d,tsUsPauseBeg=%lld", iPausedTime, tsUsPauseBeg);
                mPausedflag =  false;
            }

            //-
            if (buffer->range_length() > 0)
            {
                //not vorbis header data should be released
                if (memcmp(buffer->data() + 29, "vorbis", 6) != 0)
                {
                    buffer->release();
                    buffer = NULL;
                    continue;
                }
                else
                {
                    LOGD("ogg header:buffer=%p,size=%d", buffer, buffer->range_length());
                }
            }
            else
            {
                buffer->release();
                buffer = NULL;
                continue;
            }
        }

        mEstimatedSizeBytes += buffer->range_length();

        if (exceedsFileSizeLimit())
        {
            buffer->release();
            buffer = NULL;
            notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED, 0);
            break;
        }

        CHECK(buffer->meta_data()->findInt64(kKeyTime, &timestampUs));

        if (timestampUs > 0)
        {
            if (timestampUs > mEstimatedDurationUs)
            {
                mEstimatedDurationUs = timestampUs;
            }

            if (mResumed)
            {
                //mtk80721 deal pause time error+
                buffer->meta_data()->findInt64(kKeyTime, &tsUsPauseEnd);
                LOGD("previousPausedDurationUs =%lld,pausetime=%d,tsUsPauseBeg=%lld,tsUsPauseEnd=%lld",
                     previousPausedDurationUs, iPausedTime, tsUsPauseBeg, tsUsPauseEnd);

                previousPausedDurationUs = previousPausedDurationUs + (tsUsPauseEnd - tsUsPauseBeg);
                smpPaused = previousPausedDurationUs * mSampleRate / 1000000ll;

                LOGD("previousPausedDurationUs =%lld,samplecount=%lld", previousPausedDurationUs, smpPaused);
                //previousPausedDurationUs += (timestampUs - maxTimestampUs - 20000);
                //-
                mResumed = false;
            }

            timestampUs -= previousPausedDurationUs;
            LOGV("time stamp: %lld, previous paused duration: %lld", timestampUs, previousPausedDurationUs);

            if (timestampUs > maxTimestampUs)
            {
                maxTimestampUs = timestampUs;
            }
        }

        if (exceedsFileDurationLimit())
        {
            buffer->release();
            buffer = NULL;
            notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_DURATION_REACHED, 0);
            break;
        }

        LOGV("OggWriter::threadFunc:fwrite");
        //write timestamp
        uint8_t *ptimestamp = (uint8_t *)buffer->data() + buffer->range_offset() + 6;
        uint64_t ts = U64LE_AT(ptimestamp);

        if (smpPaused > 0)
        {
            ts -= smpPaused;
            memcpy(ptimestamp, &ts, sizeof(int64_t));
        }

        ssize_t n = fwrite(
                        (const uint8_t *)buffer->data() + buffer->range_offset(),
                        1,
                        buffer->range_length(),
                        mFile);

        ltotalSize += n;

        if (n < (ssize_t)buffer->range_length())
        {
            buffer->release();
            buffer = NULL;
            break;
        }

        // XXX: How to tell it is stopped prematurely?
        if (stoppedPrematurely)
        {
            stoppedPrematurely = false;
        }

        LOGV("OggWriter::threadFunc:buffer->release:buffer=%p", buffer);
        buffer->release();
        buffer = NULL;
    }

    if (stoppedPrematurely)
    {
        notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_TRACK_INFO_COMPLETION_STATUS, UNKNOWN_ERROR);
    }

    /*
    //
        int bitrate = ltotalSize  / (timestampUs/1E6) * 8;
        LOGV("ltotalSize=%lld, timestampUs=%lld, bitrate=%d",ltotalSize, timestampUs, bitrate);
     //seek to the bitrate field
        fseek(mFile, 44, SEEK_SET);
     // max bitrate
        fwrite(&bitrate, 1, sizeof(int), mFile);
     // nominal bitrate
        fwrite(&bitrate, 1, sizeof(int), mFile);
     // min bitrate
        fwrite(&bitrate, 1, sizeof(int), mFile);
    */
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

bool OggWriter::reachedEOS()
{
    return mReachedEOS;
}

}  // namespace android

