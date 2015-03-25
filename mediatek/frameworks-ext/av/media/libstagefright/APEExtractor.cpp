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
 * Copyright (C) 2009 The Android Open Source Project
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
#ifndef ANDROID_DEFAULT_CODE

#define LOG_NDEBUG 0
#define LOG_TAG "APEExtractor"
#include <utils/Log.h>


#include "include/AwesomePlayer.h"
#include "include/ID3.h"

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

#include "include/APEExtractor.h"
#include "include/APETag.h"

#include "include/TableOfContentThread.h"

#define ENABLE_XLOG_MtkAPEExtract
#ifdef ENABLE_XLOG_MtkAPEExtract
#include <cutils/xlog.h>
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#define LOGE SXLOGE
#define LOGW SXLOGW
#define LOGI SXLOGI
#define LOGD SXLOGD
#define LOGV SXLOGV
#endif

//#define ENABLE_MMRIOTHREAD
//#include "include/MMReadIOThread.h"

namespace android
{

///#ifndef ANDROID_DEFAULT_CODE
// Everything must match except for
// protection, bitrate, padding, private bits, mode extension,
// copyright bit, original bit and emphasis.
// Yes ... there are things that must indeed match...
static const uint32_t kMask = 0xfffe0c00;//0xfffe0cc0 add by zhihui zhang no consider channel mode

static bool getAPEInfo(
    const sp<DataSource> &source, off_t *inout_pos, ape_parser_ctx_t *ape_ctx, bool parseall)
{
    unsigned int i, n;
    unsigned int file_offset = 0;
    bool ret = false;
    off_t ori_pos = *inout_pos;
    ///LOGD("getAPEInfo %d, %d", *inout_pos, parseall);
    memset(ape_ctx, 0, sizeof(ape_parser_ctx_t));
    char *pFile = new char[20480 + 1024];

    if (pFile == NULL)
    {
        LOGE("getAPEInfo memory error");
        goto GetApeInfo_Exit;
    }

    if (source->readAt(*inout_pos, pFile, 20480 + 1024) <= 0)
    {
        goto GetApeInfo_Exit;
    }

    while (1)
    {
        char *sync;
        ///if (4 != fread(sync, 1, 4, fp))
        ///if((source->readAt(*inout_pos, sync, 4)!= 4)
        sync = pFile + (*inout_pos - ori_pos) ;

        if (*inout_pos > 20480)
        {
            ///LOGD("getAPEInfo not ape %x", *inout_pos);
            goto GetApeInfo_Exit;
        }

        if (memcmp(sync, "MAC ", 4) == 0)
        {
            LOGV("getAPEInfo parse ok, %x!!!!", *inout_pos);
            ///return false;
            break;
        }
        else if (memcmp(sync + 1, "MAC", 3) == 0)
        {
            *inout_pos += 1;
        }
        else if (memcmp(sync + 2, "MA", 2) == 0)
        {
            *inout_pos += 2;
        }
        else if (memcmp(sync + 3, "M", 1) == 0)
        {
            *inout_pos += 3;
        }
        else if ((memcmp("ID3", sync, 3) == 0))
        {
            size_t len =
                ((sync[6] & 0x7f) << 21)
                | ((sync[7] & 0x7f) << 14)
                | ((sync[8] & 0x7f) << 7)
                | (sync[9] & 0x7f);

            len += 10;

            LOGV("getAPEInfo id3 tag %x, len %x", *inout_pos, len);
            *inout_pos += len;

            ori_pos = *inout_pos;

            if (source->readAt(*inout_pos, pFile, 20480 + 1024) <= 0)
            {
                goto GetApeInfo_Exit;
            }
        }
        else
        {
            *inout_pos += 4;
        }

    }

    file_offset = *inout_pos;
    memcpy(ape_ctx->magic, "MAC ", 4);
    ape_ctx->junklength = *inout_pos;
    *inout_pos += 4;

    unsigned char sync4[4];
    unsigned char sync2[2];

    if (source->readAt(*inout_pos, &ape_ctx->fileversion, sizeof(ape_ctx->fileversion)) < 0)
    {        
        goto GetApeInfo_Exit;
    }

    if((ape_ctx->fileversion > 4200) 
        || (ape_ctx->fileversion < 3940) )
    {
        LOGV("getAPEInfo version is not match %d", ape_ctx->fileversion);
        goto GetApeInfo_Exit;
    }
        
    if (parseall == false)
    {
        ret = true;
        goto GetApeInfo_Exit;
    }

    *inout_pos += 2;

    if (ape_ctx->fileversion >= 3980)
    {
        if (source->readAt(*inout_pos, &ape_ctx->padding1, sizeof(ape_ctx->padding1)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->descriptorlength, sizeof(ape_ctx->descriptorlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->headerlength, sizeof(ape_ctx->headerlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->seektablelength, sizeof(ape_ctx->seektablelength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->wavheaderlength, sizeof(ape_ctx->wavheaderlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->audiodatalength, sizeof(ape_ctx->audiodatalength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->audiodatalength_high, sizeof(ape_ctx->audiodatalength_high)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->wavtaillength, sizeof(ape_ctx->wavtaillength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->md5, 16) != 16)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 16;

        /* Skip any unknown bytes at the end of the descriptor.  This is for future  compatibility */
        if (ape_ctx->descriptorlength > 52)
        {
            *inout_pos += (ape_ctx->descriptorlength - 52);
        }

        /* Read header data */
        if (source->readAt(*inout_pos, &ape_ctx->compressiontype, sizeof(ape_ctx->compressiontype)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->compressiontype > APE_MAX_COMPRESS)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->formatflags, sizeof(ape_ctx->formatflags)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->blocksperframe, sizeof(ape_ctx->blocksperframe)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->finalframeblocks, sizeof(ape_ctx->finalframeblocks)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->totalframes, sizeof(ape_ctx->totalframes)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->bps, sizeof(ape_ctx->bps)) < 0)
        {
            goto GetApeInfo_Exit;
        }
#if 0
        if (ape_ctx->bps > 16)
        {
            goto GetApeInfo_Exit;
        }
#endif
        if (parseall == false)
        {
            ret = true;
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->channels, sizeof(ape_ctx->channels)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->samplerate, sizeof(ape_ctx->samplerate)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->blocksperframe <= 0
                || ape_ctx->totalframes <= 0
                || ape_ctx->bps <= 0
                || ape_ctx->seektablelength <= 0
                || ape_ctx->samplerate <= 0
                || ape_ctx->samplerate > 192000)
        {
            LOGD("getAPEInfo header error: blocksperframe %x,totalframes %x, bps %x,seektablelength %x, samplerate %x ",
                 ape_ctx->blocksperframe,
                 ape_ctx->totalframes,
                 ape_ctx->bps,
                 ape_ctx->seektablelength,
                 ape_ctx->samplerate);
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;
    }
    else
    {
        ape_ctx->descriptorlength = 0;
        ape_ctx->headerlength = 32;

        if (source->readAt(*inout_pos, &ape_ctx->compressiontype, sizeof(ape_ctx->compressiontype)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->compressiontype > APE_MAX_COMPRESS)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->formatflags, sizeof(ape_ctx->formatflags)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->channels, sizeof(ape_ctx->channels)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (source->readAt(*inout_pos, &ape_ctx->samplerate, sizeof(ape_ctx->samplerate)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->wavheaderlength, sizeof(ape_ctx->wavheaderlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->wavtaillength, sizeof(ape_ctx->wavtaillength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->totalframes, sizeof(ape_ctx->totalframes)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (source->readAt(*inout_pos, &ape_ctx->finalframeblocks, sizeof(ape_ctx->finalframeblocks)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
        {
            ///fseek(fp, 4, SEEK_CUR);
            *inout_pos += 4;
            ape_ctx->headerlength += 4;
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
        {
            if (source->readAt(*inout_pos, &ape_ctx->seektablelength, sizeof(ape_ctx->seektablelength)) < 0)
            {
                goto GetApeInfo_Exit;
            }

            *inout_pos += 4;
            ape_ctx->headerlength += 4;
            ape_ctx->seektablelength *= sizeof(ape_parser_int32_t);
        }
        else
        {
            ape_ctx->seektablelength = ape_ctx->totalframes * sizeof(ape_parser_int32_t);
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_8_BIT)
        {
            ape_ctx->bps = 8;
        }
        else if (ape_ctx->formatflags & MAC_FORMAT_FLAG_24_BIT)
        {
            ape_ctx->bps = 24;
            goto GetApeInfo_Exit;
        }
        else
        {
            ape_ctx->bps = 16;
        }

        if (parseall == false)
        {
            ret = true;
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->fileversion >= APE_MIN_VERSION)
        {
            ape_ctx->blocksperframe = 73728 * 4;
        }
        else if ((ape_ctx->fileversion >= 3900) || (ape_ctx->fileversion >= 3800 && ape_ctx->compressiontype >= APE_MAX_COMPRESS))
        {
            ape_ctx->blocksperframe = 73728;
        }
        else
        {
            ape_ctx->blocksperframe = 9216;
        }

        /* Skip any stored wav header */
        if (!(ape_ctx->formatflags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
        {
            *inout_pos += ape_ctx->wavheaderlength;
        }

        if (ape_ctx->blocksperframe <= 0
                || ape_ctx->totalframes <= 0
                || ape_ctx->bps <= 0
                || ape_ctx->seektablelength <= 0
                || ape_ctx->samplerate <= 0
                || ape_ctx->samplerate > 192000)
        {
            LOGD("getAPEInfo header error: blocksperframe %x,totalframes %x, bps %x,seektablelength %x, samplerate %x ",
                 ape_ctx->blocksperframe,
                 ape_ctx->totalframes,
                 ape_ctx->bps,
                 ape_ctx->seektablelength,
                 ape_ctx->samplerate);
            goto GetApeInfo_Exit;
        }
    }
    ape_ctx->totalsamples = ape_ctx->finalframeblocks;

    if (ape_ctx->totalframes > 1)
    {
        ape_ctx->totalsamples += ape_ctx->blocksperframe * (ape_ctx->totalframes - 1);
    }

    if (ape_ctx->seektablelength > 0)
    {
        ape_parser_uint32_t seekaddr = 0;
        ape_ctx->seektable = (uint32_t *)malloc(ape_ctx->seektablelength);

        if (ape_ctx->seektable == NULL)
        {
            goto GetApeInfo_Exit;
        }

        for (i = 0; i < ape_ctx->seektablelength / sizeof(ape_parser_uint32_t); i++)
        {
            if (source->readAt(*inout_pos, &seekaddr, 4) < 0)
            {
                free(ape_ctx->seektable);
                ape_ctx->seektable = NULL;
                goto GetApeInfo_Exit;
            }

            ape_ctx->seektable[i] = (seekaddr + file_offset);
            *inout_pos += 4;
        }
    }

    ape_ctx->firstframe = ape_ctx->junklength + ape_ctx->descriptorlength +
                          ape_ctx->headerlength + ape_ctx->seektablelength +
                          ape_ctx->wavheaderlength;
    ape_ctx->seektablefilepos = ape_ctx->junklength + ape_ctx->descriptorlength +
                                ape_ctx->headerlength;


    *inout_pos = ape_ctx->firstframe;

    LOGV("getAPEInfo header info: offset %d, ape_ctx->junklength %x,ape_ctx->firstframe %x, ape_ctx->totalsamples %x,ape_ctx->fileversion %x, ape_ctx->padding1 %x ",
         file_offset,
         ape_ctx->junklength,
         ape_ctx->firstframe,
         ape_ctx->totalsamples,
         ape_ctx->fileversion,
         ape_ctx->padding1);

    LOGV("ape_ctx->descriptorlength %x,ape_ctx->headerlength %x,ape_ctx->seektablelength %x,ape_ctx->wavheaderlength %x,ape_ctx->audiodatalength %x ",
         ape_ctx->descriptorlength,
         ape_ctx->headerlength,
         ape_ctx->seektablelength,
         ape_ctx->wavheaderlength,
         ape_ctx->audiodatalength);


    LOGV("ape_ctx->audiodatalength_high %x,ape_ctx->wavtaillength %x,ape_ctx->compressiontype %x, ape_ctx->formatflags %x,ape_ctx->blocksperframe %x",
         ape_ctx->audiodatalength_high,
         ape_ctx->wavtaillength,
         ape_ctx->compressiontype,
         ape_ctx->formatflags,
         ape_ctx->blocksperframe);

    LOGV("ape_ctx->finalframeblocks %x,ape_ctx->totalframes %x,ape_ctx->bps %x,ape_ctx->channels %x,ape_ctx->samplerate %x",
         ape_ctx->finalframeblocks,
         ape_ctx->totalframes,
         ape_ctx->bps,
         ape_ctx->channels,
         ape_ctx->samplerate);

    ret = true;

GetApeInfo_Exit:

    if (pFile)
    {
        delete[] pFile;
    }

    pFile = NULL;
    return ret;

}


bool SniffAPE(
    const sp<DataSource> &source, String8 *mimeType,
    float *confidence, sp<AMessage> *meta)
{
    ///LOGD("SniffAPE++");
    off_t pos = 0;
    uint32_t header = 0;
    ape_parser_ctx_t ape_ctx;

    if (!getAPEInfo(source, &pos, &ape_ctx, false))
    {
        return false;
    }

    *meta = new AMessage;
    (*meta)->setInt64("offset", pos);
    (*meta)->setInt32("fileversion", ape_ctx.fileversion);

    *mimeType = MEDIA_MIMETYPE_AUDIO_APE;
    *confidence = 0.3f;
    LOGV("SniffAPE OK");
    return true;
}


class APESource : public MediaSource
{
public:
    APESource(
        const sp<MetaData> &meta, const sp<DataSource> &source,
        off_t first_frame_pos, uint32_t totalsample, uint32_t finalsample,
        int32_t TotalFrame, uint32_t  *table_of_contents, int sample_per_frame, int32_t frm_size, int32_t pst_bound
    );
    int ape_calc_seekpos_by_microsecond(struct ape_parser_ctx_t *ape_ctx,
                                        int64_t millisecond,
                                        uint32_t *newframe,
                                        uint32_t *filepos,
                                        uint32_t *firstbyte,
                                        uint32_t *blocks_to_skip);
    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();

    virtual sp<MetaData> getFormat();
    virtual status_t read(
        MediaBuffer **buffer, const ReadOptions *options = NULL);

protected:
    virtual ~APESource();

private:
    sp<MetaData> mMeta;
    sp<DataSource> mDataSource;

    off_t mFirstFramePos;
    int32_t mTotalFrame;
    int32_t mSt_bound;
    uint32_t mTotalsample;
    uint32_t mFinalsample;
    int mChannels;
    int mSampleRate;
    int mSamplesPerFrame;
    uint32_t *mTableOfContents;

    off_t mCurrentPos;
    off_t mFileoffset;
    int64_t mCurrentTimeUs;
    int64_t mCurrentFrame;

    bool mStarted;
    MediaBufferGroup *mGroup;
    size_t kMaxFrameSize;
    AwesomePlayer *mObserver;
    APESource(const APESource &);
    APESource &operator=(const APESource &);
};

APESource::APESource(
    const sp<MetaData> &meta, const sp<DataSource> &source,
    off_t first_frame_pos, uint32_t totalsample, uint32_t finalsample,
    int32_t TotalFrame, uint32_t  *table_of_contents, int sample_per_frame, int32_t frm_size, int32_t pst_bound
)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mTotalFrame(TotalFrame),
      mTotalsample(totalsample),
      mFinalsample(finalsample),
      mTableOfContents(table_of_contents),
      mCurrentTimeUs(0),
      mStarted(false),
      mSamplesPerFrame(sample_per_frame),
      mSt_bound(pst_bound)
{

    LOGV("APESource %d, %d, %d, %d, %d", mTotalsample, mFirstFramePos, mFinalsample, sample_per_frame, frm_size);
    mObserver = NULL;
    mCurrentFrame = 0;
    mCurrentPos = 0;
    kMaxFrameSize = frm_size;
    mFileoffset = 0;
#ifdef ENABLE_MMRIOTHREAD    
    startRIOThread(mDataSource, mFirstFramePos, kMaxFrameSize, 6);
#endif
}


APESource::~APESource()
{
    if (mStarted)
    {
        stop();
    }
}

status_t APESource::start(MetaData *)
{
    CHECK(!mStarted);
    LOGD("APESource::start In");

    unsigned int working_BUF_size, in_size = 5184, out_size;

    mGroup = new MediaBufferGroup;

    mGroup->add_buffer(new MediaBuffer(kMaxFrameSize));

    mCurrentPos = mFirstFramePos;
    mCurrentTimeUs = 0;

    mStarted = true;
    off64_t fileoffset = 0;
    mDataSource->getSize(&fileoffset);

//    if (fileoffset < mTableOfContents[mTotalFrame - 1])
    if (fileoffset < mTableOfContents[mSt_bound- 1])
    {
        int i = 0;

//        for (i = 0; i < mTotalFrame; i++)
        for (i = 0; i < mSt_bound; i++)
        {
            if (fileoffset < mTableOfContents[i])
            {
                i--;
                break;
            }
            else if (fileoffset == mTableOfContents[i])
            {
                break;
            }
        }

        mFileoffset = mTableOfContents[i];
        LOGD("mFileoffset redefine old %d, new %d", fileoffset, mFileoffset);
    }

    return OK;
}

status_t APESource::stop()
{
    LOGV("APESource::stop() In");
    CHECK(mStarted);
    delete mGroup;
    mGroup = NULL;

    if (mTableOfContents)
    {
        free(mTableOfContents);
    }

    mTableOfContents = NULL;
#ifdef ENABLE_MMRIOTHREAD
    stopRIOThread();
#endif
    mStarted = false;
    LOGV("APESource::stop() out");
    return OK;
}

sp<MetaData> APESource::getFormat()
{
    return mMeta;
}


int APESource:: ape_calc_seekpos_by_microsecond(struct ape_parser_ctx_t *ape_ctx,
        int64_t microsecond,
        uint32_t *newframe,
        uint32_t *filepos,
        uint32_t *firstbyte,
        uint32_t *blocks_to_skip)
{
    uint32_t n = 0xffffffff, delta;
    int64_t new_blocks;

    new_blocks = (int64_t)microsecond * (int64_t)ape_ctx->samplerate;
    new_blocks /= 1000000;

    if (ape_ctx->blocksperframe >= 0)
    {
        n = new_blocks / ape_ctx->blocksperframe;
    }

    if (n >= ape_ctx->totalframes)
    {
        return -1;
    }

    *newframe = n;
    *filepos = ape_ctx->seektable[n];
    *blocks_to_skip = new_blocks - (n * ape_ctx->blocksperframe);

    delta = (*filepos - ape_ctx->firstframe) & 3;
    LOGV("ape_calc_seekpos_by_microsecond %d, %x, %x, %d", n, ape_ctx->seektable[n], ape_ctx->firstframe, delta);
    *firstbyte = 3 - delta;
    *filepos -= delta;

    return 0;
}

status_t APESource::read(
    MediaBuffer **out, const ReadOptions *options)
{
    *out = NULL;
    uint32_t newframe = 0 , firstbyte = 0;

    ///LOGV("APESource::read");
    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    int32_t bitrate = 0;

    if (!mMeta->findInt32(kKeyBitRate, &bitrate)
            || !mMeta->findInt32(kKeySampleRate, &mSampleRate))
    {
        LOGI("no bitrate");
        return ERROR_UNSUPPORTED;
    }

    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
    {

        {

            int64_t duration = 0;
            int64_t seektabletime = 0;

            if ((mTotalsample > 0) && (mTableOfContents[0] > 0) && (mSamplesPerFrame > 0)
                    && mMeta->findInt64(kKeyDuration, &duration))
            {
                ape_parser_ctx_t ape_ctx;
                uint32_t filepos, blocks_to_skip;
                ape_ctx.samplerate = mSampleRate;
                ape_ctx.blocksperframe = mSamplesPerFrame;
                ape_ctx.totalframes = mTotalFrame;
                ape_ctx.seektable = mTableOfContents;
                ape_ctx.firstframe = mTableOfContents[0];

                if (ape_calc_seekpos_by_microsecond(&ape_ctx,
                                                    seekTimeUs,
                                                    &newframe,
                                                    &filepos,
                                                    &firstbyte,
                                                    &blocks_to_skip) < 0)
                {
                    LOGD("getseekto error exit");
                    return ERROR_UNSUPPORTED;
                }

                mCurrentPos = filepos;
                mCurrentTimeUs = (int64_t)newframe * mSamplesPerFrame * 1000000ll / mSampleRate;

                LOGD("getseekto seekTimeUs=%lld, Actual time%lld, filepos%x,frame %d, seekbyte %d", seekTimeUs, mCurrentTimeUs, mCurrentPos, newframe, firstbyte);

            }
            else
            {
                LOGD("getseekto parameter error exit");
                return ERROR_UNSUPPORTED;
            }


        }

    }


    if ((mFileoffset != 0)
            && (mCurrentPos >= mFileoffset))
    {
        LOGD("APESource::readAt to end filesize %x curr: %x", mFileoffset, mCurrentPos);
        return ERROR_END_OF_STREAM;
    }

    MediaBuffer *buffer;
    status_t err = mGroup->acquire_buffer(&buffer);

    if (err != OK)
    {
        LOGD("APESource::acquire_buffer fail");
        return err;
    }

    size_t frame_size;
    frame_size = kMaxFrameSize;
    ssize_t n = 0;	

#ifdef ENABLE_MMRIOTHREAD
    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
    {
        ResetReadioPtr(mCurrentPos);        
    }
    n = ReadBitsteam(buffer->data(), frame_size);
#else    
    ///frame_size = mMaxBufferSize;
    n = mDataSource->readAt(mCurrentPos, buffer->data(), frame_size);    
#endif

    ///LOGE("APESource::readAt  %x, %x, %d, %d, %d, %d, %d", mCurrentPos, buffer->data(), buffer->size(), mTotalsample, bitrate, mSampleRate, frame_size);
    //ssize_t n = mDataSource->readAt(mCurrentPos, buffer->data(), frame_size);

    if ((mFileoffset != 0)
            && ((mCurrentPos + n) >= mFileoffset))
    {
        frame_size = mFileoffset - mCurrentPos;
        memset(buffer->data() + frame_size, 0, n - frame_size);
    }
    else if ((n < (ssize_t)frame_size)
             && (n > 0))
    {
        frame_size = n;
        off64_t fileoffset = 0;
        mDataSource->getSize(&fileoffset);
        LOGD("APESource::readAt not enough read %d frmsize %x, filepos %x, filesize %x", n, frame_size, mCurrentPos + frame_size, fileoffset);

        //if ((mCurrentPos + frame_size) >= fileoffset
        //        && (mCurrentPos + frame_size) < mTableOfContents[mTotalFrame - 1])
        if ((mCurrentPos + frame_size) >= fileoffset && (mCurrentPos + frame_size) < mTableOfContents[mSt_bound- 1])
        {
            memset(buffer->data(), 0, buffer->size());
            /// for this file is not complete error, frame buffer should not transfer to avoid decoding noise data.
            LOGD("APESource::file is not enough to end --> memset");
        }
    }
    else if (n <= 0)
    {
        buffer->release();
        buffer = NULL;
        LOGD("APESource::readAt EOS filepos %x frmsize %d", mCurrentPos, frame_size);
        return ERROR_END_OF_STREAM;
    }

    buffer->set_range(0, frame_size);

    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
    {
        buffer->meta_data()->setInt64(kKeyTime, mCurrentTimeUs);
        buffer->meta_data()->setInt32(kKeyNemFrame, newframe);
        buffer->meta_data()->setInt32(kKeySeekByte, firstbyte);
    }

    buffer->meta_data()->setInt32(kKeyIsSyncFrame, 1);

    mCurrentPos += frame_size;
    mCurrentTimeUs += (int64_t)(frame_size * 8000000ll) / bitrate ;
    
#ifdef ENABLE_MMRIOTHREAD    
    UpdateReadPtr(frame_size);
#endif

    *out = buffer;

    ///LOGE("APESource::kKeyTime done %x %lld", mCurrentPos, mCurrentTimeUs);
    return OK;
}


APEExtractor::APEExtractor(
    const sp<DataSource> &source, const sp<AMessage> &meta)
    : mInitCheck(NO_INIT),
      mDataSource(source),
      mFirstFramePos(-1),
      mFinalsample(0),
      mSamplesPerFrame(0)
{
    off_t pos = 0;
    ape_parser_ctx_t ape_ctx;
    ape_ctx.seektable = NULL;
    bool success = false;

    ///LOGD("APEExtractor");
    int64_t meta_offset;
    uint32_t meta_header;
    success = getAPEInfo(mDataSource, &pos, &ape_ctx, true);

    if (!success)
    {
        return;
    }

    if ((ape_ctx.samplerate <= 0)
            || (ape_ctx.bps <= 0)
            || (ape_ctx.channels <= 0))
    {
        mInitCheck = NO_INIT;
        LOGD("APEExtractor parameter wrong return samplerate %d bps %d channels %d ", ape_ctx.samplerate, ape_ctx.bps, ape_ctx.channels);
        return;
    }


    mFirstFramePos = pos;
    mTotalsample = ape_ctx.totalsamples;
    mFinalsample = ape_ctx.finalframeblocks;
    mSamplesPerFrame = ape_ctx.blocksperframe;

    int sample_rate;
    int num_channels;

    unsigned int working_BUF_size, in_size = 5184, out_size;
    int64_t duration;

    if (ape_ctx.samplerate > 0)
    {
        duration = (int64_t)1000000ll * ape_ctx.totalsamples / ape_ctx.samplerate;
    }
    else
    {
        duration = 0;
    }

    mMeta = new MetaData;

    mMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_APE);
    mMeta->setInt32(kKeySampleRate, ape_ctx.samplerate);
    mMeta->setInt32(kKeyBitRate,  16 * 2 * ape_ctx.samplerate);
    mMeta->setInt32(kKeyChannelCount, ape_ctx.channels);

    mMeta->setInt32(kkeyApebit,  ape_ctx.bps * ape_ctx.channels * ape_ctx.samplerate);
    mMeta->setInt32(kkeyApechl,  ape_ctx.channels);
    LOGV("kKeyDuration set %lld, chn %d", duration, ape_ctx.channels);

    off64_t fileoffset = 0;
    mDataSource->getSize(&fileoffset);
    mSt_bound = ape_ctx.seektablelength>>2;
    LOGD("totalframes=%d,seektablelength=%d,mSt_bound=%d",ape_ctx.totalframes,ape_ctx.seektablelength,mSt_bound);
//    if (fileoffset < ape_ctx.seektable[ape_ctx.totalframes - 1])
    if (fileoffset < ape_ctx.seektable[mSt_bound - 1])
    {
        int i = 0;

        //for (i = 0; i < ape_ctx.totalframes; i++)
        for (i = 0; i < mSt_bound; i++)
        {
            if (fileoffset < ape_ctx.seektable[i])
            {
                i--;
                break;
            }
            else if (fileoffset == ape_ctx.seektable[i])
            {
                break;
            }
        }

        duration = (int64_t)1000000ll * i * ape_ctx.blocksperframe / ape_ctx.samplerate;
        ape_ctx.totalframes = i;
        ape_ctx.finalframeblocks = ape_ctx.blocksperframe;
        LOGD("kKeyDuration redefine duration %lld, totalfrm %d, finalblk %d", duration, ape_ctx.totalframes, ape_ctx.finalframeblocks);
    }

    mMeta->setInt64(kKeyDuration, duration);

    mMeta->setInt32(kKeyFileType, ape_ctx.fileversion);
    mMeta->setInt32(kkeyComptype, ape_ctx.compressiontype);
    mMeta->setInt32(kKeySamplesperframe, ape_ctx.blocksperframe);
    mMeta->setInt32(kKeyTotalFrame, ape_ctx.totalframes);
    mMeta->setInt32(kKeyFinalSample, ape_ctx.finalframeblocks);

    buf_size = in_size * 7; ////ape_ctx.blocksperframe*ape_ctx.channels*ape_ctx.bps/8;

    if (buf_size > 12288)
    {
        buf_size = 12288;    ///12k
    }

    mMeta->setInt32(kKeyBufferSize, buf_size);

    mTotalFrame = ape_ctx.totalframes;
//    mTableOfContents = (uint32_t *)malloc(mTotalFrame * sizeof(uint32_t));
    mTableOfContents = (uint32_t *)malloc(mSt_bound * sizeof(uint32_t));    
    mInitCheck = OK;

    if (mTableOfContents == NULL)
    {
        mInitCheck = NO_INIT;
        LOGE("APEExtractor has no builtin seektable return ");
    }

    if (mTableOfContents)
    {
        //memcpy(mTableOfContents, ape_ctx.seektable, (mTotalFrame * sizeof(int32_t)));
        memcpy(mTableOfContents, ape_ctx.seektable, mSt_bound* sizeof(int32_t));
    }

    if (ape_ctx.seektable)
    {
        free(ape_ctx.seektable);
    }

    ape_ctx.seektable = NULL;

    LOGV("APEExtractor done");
}

size_t APEExtractor::countTracks()
{
    return mInitCheck != OK ? 0 : 1;
}

sp<MediaSource> APEExtractor::getTrack(size_t index)
{
    if (mInitCheck != OK || index != 0)
    {
        return NULL;
    }

    LOGV("getTrack, %d, %d, %d, %d, %d", mFirstFramePos, mTotalsample, mFinalsample,
         mTotalFrame, mSamplesPerFrame);

    return new APESource(
               mMeta, mDataSource, mFirstFramePos, mTotalsample, mFinalsample,
               mTotalFrame, mTableOfContents, mSamplesPerFrame, buf_size,mSt_bound);
}

sp<MetaData> APEExtractor::getTrackMetaData(size_t index, uint32_t flags)
{
    if (mInitCheck != OK || index != 0)
    {
        return NULL;
    }

    return mMeta;
}

sp<MetaData> APEExtractor::getMetaData()
{
    LOGV("APEExtractor::getMetaData()");
    sp<MetaData> meta = new MetaData;

    if (mInitCheck != OK)
    {
        return meta;
    }

    meta->setCString(kKeyMIMEType, "audio/ape");

    ID3 id3(mDataSource);

    if (id3.isValid())
    {
        LOGE("APEExtractor::getMetaData() ID3 id3");
        struct Map
        {
            int key;
            const char *tag1;
            const char *tag2;
        };
        static const Map kMap[] =
        {
            { kKeyAlbum, "TALB", "TAL" },
            { kKeyArtist, "TPE1", "TP1" },
            { kKeyAlbumArtist, "TPE2", "TP2" },
            { kKeyComposer, "TCOM", "TCM" },
            { kKeyGenre, "TCON", "TCO" },
            { kKeyTitle, "TIT2", "TT2" },
            { kKeyYear, "TYE", "TYER" },
            { kKeyAuthor, "TXT", "TEXT" },
            { kKeyCDTrackNumber, "TRK", "TRCK" },
            { kKeyDiscNumber, "TPA", "TPOS" },
            { kKeyCompilation, "TCP", "TCMP" },
        };
        static const size_t kNumMapEntries = sizeof(kMap) / sizeof(kMap[0]);

        for (size_t i = 0; i < kNumMapEntries; ++i)
        {
            ///LOGE("getMetaData() id3 kMap %d, %d", kNumMapEntries, i);
            ID3::Iterator *it = new ID3::Iterator(id3, kMap[i].tag1);

            if (it->done())
            {
                delete it;
                it = new ID3::Iterator(id3, kMap[i].tag2);
            }

            if (it->done())
            {
                delete it;
                continue;
            }

            String8 s;
            it->getString(&s);
            delete it;

            meta->setCString(kMap[i].key, s);
        }

        size_t dataSize;
        String8 mime;
        const void *data = id3.getAlbumArt(&dataSize, &mime);

        if (data)
        {
            meta->setData(kKeyAlbumArt, MetaData::TYPE_NONE, data, dataSize);
            meta->setCString(kKeyAlbumArtMIME, mime.string());
        }

        return meta;

    }

    APETAG apetag(mDataSource);

    if (apetag.isValid())
    {

        struct ApeMap
        {
            int key;
            const char *tag;
            uint16_t    key_len;
            uint32_t    key_attr;
        };
        static const ApeMap kMap[] =
        {
            { kKeyAlbum,        "Album",    5,  META_TAG_ATTR_ALBUM },
            { kKeyArtist,       "Artist",   6,  META_TAG_ATTR_ARTIST },
            { kKeyComposer,     "Composer", 7,  META_TAG_ATTR_AUTHOR },
            { kKeyGenre,        "Genre",    5,  META_TAG_ATTR_GENRE },
            { kKeyTitle,        "Title",    5,  META_TAG_ATTR_TITLE },
            { kKeyYear,         "Year",     4,  META_TAG_ATTR_YEAR },
            { kKeyCDTrackNumber, "Track",    5,  META_TAG_ATTR_TRACKNUM },
        };

        static const size_t kNumMapEntries = sizeof(kMap) / sizeof(kMap[0]);

        for (size_t i = 0; i < kNumMapEntries; ++i)
        {
            APETAG::Iterator *it = new APETAG::Iterator(apetag, kMap[i].tag, kMap[i].key_len);

            if (it->done())
            {
                delete it;
                continue;
            }

            String8 s;
            it->getString(&s);
            delete it;

            meta->setCString(kMap[i].key, s);
        }

        return meta;

    }

    return meta;


}

}  // namespace android
///#else

///#endif
#endif
