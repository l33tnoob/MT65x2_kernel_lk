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
 * Copyright (C) 2011 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "FLACExtractor"
#include <utils/Log.h>

#include "include/FLACExtractor.h"
// Vorbis comments
#include "include/OggExtractor.h"
// libFLAC parser
#include "FLAC/stream_decoder.h"

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

#define FLACTEXTRACT_DEFAULT
#include "include/AwesomePlayer.h"
#include "include/TableOfContentThread.h"

#define ENABLE_XLOG_MtkOmxFLACExtract
#define ENABLE_MMRIOTHREAD
#include "include/MMReadIOThread.h"

#ifdef ENABLE_XLOG_MtkOmxFLACExtract
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


namespace android
{

static const uint32_t kFlacMask = 0xfff8;//0xfffe0cc0 add by zhihui zhang no consider channel mode
static const unsigned char  FLAC__STREAM_SYNC_STRING[4] = { 'f', 'L', 'a', 'C' };
static const unsigned char ID3V2_TAG_[3] = { 'I', 'D', '3' };

static unsigned char const crc8_table[256] =
{
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

class FLACParser;
///, public TableOfContentThread
class FLACSource : public MediaSource, public TableOfContentThread, public MMReadIOThread
{

public:
    FLACSource(
        const sp<DataSource> &dataSource,
        const sp<MetaData> &trackMetadata,
        FLAC__StreamMetadata_StreamInfo MetaInfo,
        const sp<FLACParser> &Parser);

    virtual status_t getNextFramePos(off_t *curPos, off_t *pNextPos, int64_t *frameTsUs);
    bool getSeekPosforStreaming(int64_t targetTimeUS, off_t *mCurrentSeekPos, int *seekFrm);
    virtual status_t sendDurationUpdateEvent(int64_t duration);

    virtual status_t start(MetaData *params);
    virtual status_t stop();
    virtual sp<MetaData> getFormat();
    bool FindNextSycnPos(uint8_t *buf, ssize_t buf_size, off_t start_pos, uint32_t *next_pos_offset, uint64_t *frm_size);
    bool FindNextSycnFrmPos(uint8_t *buf, ssize_t buf_size, off_t start_pos, uint32_t *next_pos_offset, uint64_t *frm_size);

    virtual status_t read(
        MediaBuffer **buffer, const ReadOptions *options = NULL);

    unsigned char  FLAC__crc8(const unsigned char *data, unsigned len);
    bool FlacResync(const sp<DataSource> &source, uint32_t match_header, off_t *inout_pos, uint64_t *frm_num, int maxbuffcnt)  ;
protected:
    virtual ~FLACSource();

private:
    sp<DataSource> mDataSource;
    sp<FLACParser> mParser;
    sp<MetaData> mTrackMetadata;
    MediaBufferGroup *mGroup;
    size_t mMaxBufferSize;
    size_t mMinBufoffset;
    int64_t mCurrentTimeUs;
    off_t mCurrentFrm;
    bool mInitCheck;
    bool mStarted;
    bool bTocEnable;
    off_t mCurrentPos;

    off_t mFirstFramePos;
    AwesomePlayer *mObserver;
    FLAC__StreamMetadata_StreamInfo mMetaInfo;
    uint32_t mCurrentfrmTime;

    status_t init();


    // no copy constructor or assignment
    FLACSource(const FLACSource &);
    FLACSource &operator=(const FLACSource &);

};

// FLACParser wraps a C libFLAC parser aka stream decoder

class FLACParser : public RefBase
{

public:
    FLACParser(
        const sp<DataSource> &dataSource,
        // If metadata pointers aren't provided, we don't fill them
        const sp<MetaData> &fileMetadata = 0,
        const sp<MetaData> &trackMetadata = 0);

    status_t initCheck() const
    {

        return mInitCheck;
    }

    bool find_metadata_(off_t *inout_pos);
    bool read_metadata_(off_t *inout_pos, bool metastream_only);
    status_t ParseMetaData();

    bool parser_getseektable(uint64_t *sample_number, uint64_t *offset);

    // stream properties
    FLAC__StreamMetadata_StreamInfo mStreamInfo;

    bool mSeekTableValid;
    bool mStreamInfoValid;

protected:
    virtual ~FLACParser();

private:
    sp<DataSource> mDataSource;
    sp<MetaData> mFileMetadata;
    sp<MetaData> mTrackMetadata;
    bool mInitCheck;
    bool has_stream_info;

    // media buffers
    size_t mMaxBufferSize;
    off64_t mCurrentPos;

    // handle to underlying libFLAC parser
    FLAC__StreamDecoder *mDecoder;

    // current position within the data source

    bool mEOF;

    // cached when the STREAMINFO metadata is parsed by libFLAC

    // cached when a decoded PCM block is "written" by libFLAC parser
    bool mWriteRequested;
    bool mWriteCompleted;
    FLAC__FrameHeader mWriteHeader;
    const FLAC__int32 *const *mWriteBuffer;

    // most recent error reported by libFLAC parser
    FLAC__StreamDecoderErrorStatus mErrorStatus;

    status_t init();

    // no copy constructor or assignment
    FLACParser(const FLACParser &);
    FLACParser &operator=(const FLACParser &);

    // FLAC parser callbacks as C++ instance methods
    FLAC__StreamDecoderReadStatus readCallback(
        FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderWriteStatus writeCallback(
        const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
    void metadataCallback(const FLAC__StreamMetadata *metadata);
    void errorCallback(FLAC__StreamDecoderErrorStatus status);

    // FLAC parser callbacks as C-callable functions
    static FLAC__StreamDecoderReadStatus read_callback(
        const FLAC__StreamDecoder *decoder,
        FLAC__byte buffer[], size_t *bytes,
        void *client_data);
    static FLAC__StreamDecoderWriteStatus write_callback(
        const FLAC__StreamDecoder *decoder,
        const FLAC__Frame *frame, const FLAC__int32 *const buffer[],
        void *client_data);
    static void metadata_callback(
        const FLAC__StreamDecoder *decoder,
        const FLAC__StreamMetadata *metadata,
        void *client_data);
    static void error_callback(
        const FLAC__StreamDecoder *decoder,
        FLAC__StreamDecoderErrorStatus status,
        void *client_data);

};

FLAC__StreamDecoderReadStatus FLACParser::read_callback(
    const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
    size_t *bytes, void *client_data)
{
    return ((FLACParser *) client_data)->readCallback(buffer, bytes);
}

FLAC__StreamDecoderWriteStatus FLACParser::write_callback(
    const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
    const FLAC__int32 *const buffer[], void *client_data)
{
    return ((FLACParser *) client_data)->writeCallback(frame, buffer);
}

void FLACParser::metadata_callback(
    const FLAC__StreamDecoder *decoder,
    const FLAC__StreamMetadata *metadata, void *client_data)
{
    ((FLACParser *) client_data)->metadataCallback(metadata);
}

void FLACParser::error_callback(
    const FLAC__StreamDecoder *decoder,
    FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    ((FLACParser *) client_data)->errorCallback(status);
}

// These are the corresponding callbacks with C++ calling conventions

FLAC__StreamDecoderReadStatus FLACParser::readCallback(
    FLAC__byte buffer[], size_t *bytes)
{
    size_t requested = *bytes;
    ssize_t actual = mDataSource->readAt(mCurrentPos, buffer, requested);

    if (0 > actual)
    {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    else if (0 == actual)
    {
        *bytes = 0;
        mEOF = true;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    else
    {
        assert(actual <= requested);
        *bytes = actual;
        mCurrentPos += actual;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
}

FLAC__StreamDecoderWriteStatus FLACParser::writeCallback(
    const FLAC__Frame *frame, const FLAC__int32 *const buffer[])
{
    if (mWriteRequested)
    {
        mWriteRequested = false;
        // FLAC parser doesn't free or realloc buffer until next frame or finish
        mWriteHeader = frame->header;
        mWriteBuffer = buffer;
        mWriteCompleted = true;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    else
    {
        LOGE("FLACParser::writeCallback unexpected");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
}

void FLACParser::metadataCallback(const FLAC__StreamMetadata *metadata)
{
    switch (metadata->type)
    {
        case FLAC__METADATA_TYPE_STREAMINFO:
            if (!mStreamInfoValid)
            {
                mStreamInfo = metadata->data.stream_info;
                mStreamInfoValid = true;
                LOGV("FLACParser::metadataCallback bps %d, chn %d, maxblk %d, minblk %d", mStreamInfo.bits_per_sample,
                     mStreamInfo.channels, mStreamInfo.max_blocksize, mStreamInfo.min_blocksize);
                LOGV("FLACParser::metadataCallback maxfrm %d, minfrm %d, samprate %d, tsample %d", mStreamInfo.max_framesize,
                     mStreamInfo.min_framesize, mStreamInfo.sample_rate, mStreamInfo.total_samples);
            }
            else
            {
                LOGE("FLACParser::metadataCallback unexpected STREAMINFO");
            }

            break;

        case FLAC__METADATA_TYPE_VORBIS_COMMENT:
        {
            const FLAC__StreamMetadata_VorbisComment *vc;
            vc = &metadata->data.vorbis_comment;

            for (FLAC__uint32 i = 0; i < vc->num_comments; ++i)
            {
                FLAC__StreamMetadata_VorbisComment_Entry *vce;
                vce = &vc->comments[i];

                //if (mFileMetadata != 0)
                if (mFileMetadata != 0 && vce->entry != NULL)
                {
                    parseVorbisComment(mFileMetadata, (const char *) vce->entry,
                                       vce->length);
                }
            }
        }
        break;

        case FLAC__METADATA_TYPE_PICTURE:
            if (mFileMetadata != 0)
            {
                const FLAC__StreamMetadata_Picture *p = &metadata->data.picture;
                mFileMetadata->setData(kKeyAlbumArt,
                                       MetaData::TYPE_NONE, p->data, p->data_length);
                mFileMetadata->setCString(kKeyAlbumArtMIME, p->mime_type);
            }

            break;

        default:
            LOGW("FLACParser::metadataCallback unexpected type %u", metadata->type);
            break;
    }
}

void FLACParser::errorCallback(FLAC__StreamDecoderErrorStatus status)
{
    LOGE("FLACParser::errorCallback status=%d", status);
    mErrorStatus = status;
}

// FLACParser

FLACParser::FLACParser(
    const sp<DataSource> &dataSource,
    const sp<MetaData> &fileMetadata,
    const sp<MetaData> &trackMetadata)
    : mDataSource(dataSource),
      mFileMetadata(fileMetadata),
      mTrackMetadata(trackMetadata),
      mInitCheck(false),
      mMaxBufferSize(0),
      mDecoder(NULL),
      mCurrentPos(0LL),
      mEOF(false),
      mStreamInfoValid(false),
      mWriteRequested(false),
      mWriteCompleted(false),
      mWriteBuffer(NULL),
      mErrorStatus((FLAC__StreamDecoderErrorStatus) - 1)
{
    LOGV("FLACParser::FLACParser");
    memset(&mStreamInfo, 0, sizeof(mStreamInfo));
    memset(&mWriteHeader, 0, sizeof(mWriteHeader));
    has_stream_info = false;
    mSeekTableValid = false;
    mInitCheck = init();
}

FLACParser::~FLACParser()
{
    LOGV("FLACParser::~FLACParser");

    if (mDecoder != NULL)
    {
        FLAC__stream_decoder_delete(mDecoder);
        mDecoder = NULL;
    }
}

bool FLACParser::parser_getseektable(FLAC__uint64 *sample_number, FLAC__uint64 *offset)
{
    if (mDecoder)
    {
        return (FLAC__stream_decoder_getseektable(mDecoder, sample_number, offset, NULL));
    }
    else
    {
        return false;
    }
}


status_t FLACParser::init()
{
    // setup libFLAC parser
    mDecoder = FLAC__stream_decoder_new();

    if (mDecoder == NULL)
    {
        // The new should succeed, since probably all it does is a malloc
        // that always succeeds in Android.  But to avoid dependence on the
        // libFLAC internals, we check and log here.
        LOGE("new failed");
        return NO_INIT;
    }

    FLAC__stream_decoder_set_md5_checking(mDecoder, false);
    FLAC__stream_decoder_set_metadata_ignore_all(mDecoder);
    FLAC__stream_decoder_set_metadata_respond(
        mDecoder, FLAC__METADATA_TYPE_STREAMINFO);
    FLAC__stream_decoder_set_metadata_respond(
        mDecoder, FLAC__METADATA_TYPE_PICTURE);
    FLAC__stream_decoder_set_metadata_respond(
        mDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
    FLAC__StreamDecoderInitStatus initStatus;
    initStatus = FLAC__stream_decoder_init_stream(
                     mDecoder,
                     read_callback, NULL, NULL,
                     NULL, NULL, write_callback,
                     metadata_callback, error_callback, (void *) this);

    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        // A failure here probably indicates a programming error and so is
        // unlikely to happen. But we check and log here similarly to above.
        LOGE("init_stream failed %d", initStatus);
        return NO_INIT;
    }

    // parse all metadata
    if (!FLAC__stream_decoder_process_until_end_of_metadata(mDecoder))
    {
        LOGE("end_of_metadata failed");
        return NO_INIT;
    }

    if (mStreamInfoValid)
    {
        // check channel count
        switch (mStreamInfo.channels)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:

                break;

            default:
                LOGE("unsupported channel count %u", mStreamInfo.channels);
                return NO_INIT;
        }

        // check bit depth
        switch (mStreamInfo.bits_per_sample)
        {
            case 8:
            case 16:
            case 24:
                break;

            default:
                LOGE("unsupported bits per sample %u", mStreamInfo.bits_per_sample);
                return NO_INIT;
        }

        // check sample rate
        switch (mStreamInfo.sample_rate)
        {
            case  8000:
            case 11025:
            case 12000:
            case 16000:
            case 22050:
            case 24000:
            case 32000:
            case 44100:
            case 48000:
			case 88200:
			case 96000:	
			case 176400:
			case 192000:					
                break;

            default:
                LOGE("unsupported sample rate %u", mStreamInfo.sample_rate);
                return NO_INIT;
        }

        ///if( (mStreamInfo.sample_rate>96000) || (mStreamInfo.sample_rate<8000))
        ///    return NO_INIT;

        // populate track metadata
        if (mTrackMetadata != 0)
        {
            mTrackMetadata->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_FLAC);
            mTrackMetadata->setInt32(kKeyChannelCount, mStreamInfo.channels);
            mTrackMetadata->setInt32(kKeySampleRate, mStreamInfo.sample_rate);
            // sample rate is non-zero, so division by zero not possible

            mSeekTableValid = FLAC__stream_decoder_getseektable(mDecoder, NULL, NULL, NULL);

            if (mStreamInfo.total_samples == 0)
            {
                if (mSeekTableValid)
                {
                    unsigned last_frmsamples;
                    FLAC__uint64 sample_number = 0x7fffffff;
                    FLAC__uint64 offset;
                    FLAC__stream_decoder_getseektable(mDecoder, &sample_number, &offset, &last_frmsamples);
                    mStreamInfo.total_samples = last_frmsamples + sample_number;
                }
                else
                {
                    mStreamInfo.total_samples = 0x7fffffff;
                }
            }

            if (mStreamInfo.sample_rate > 0)
                mTrackMetadata->setInt64(kKeyDuration,
                                         (mStreamInfo.total_samples * 1000000LL) / mStreamInfo.sample_rate);
#ifdef MTK_24BIT_AUDIO_SUPPORT
			if(mStreamInfo.bits_per_sample>0)
				mTrackMetadata->setInt32(kKeyBitWidth, mStreamInfo.bits_per_sample);
#endif

            typedef struct
            {
                unsigned min_blocksize, max_blocksize;
                unsigned min_framesize, max_framesize;
                unsigned sample_rate;
                unsigned channels;
                unsigned bits_per_sample;
                uint64_t total_samples;
                unsigned char md5sum[16];
                unsigned int mMaxBufferSize;
                bool      has_stream_info;
            } FLAC__StreamMetadata_Info_;
            FLAC__StreamMetadata_Info_ FlacMetaInfo;

            memcpy(&FlacMetaInfo, &mStreamInfo, sizeof(FlacMetaInfo));
            FlacMetaInfo.mMaxBufferSize = mStreamInfo.max_framesize + 16 ;
            FlacMetaInfo.has_stream_info = true;
            ///ALOGD("kKeyFlacMetaInfo = %lld, %d, %d, %d, %d, %d, %d, %d",mStreamInfo.total_samples, mStreamInfo.min_framesize, mStreamInfo.max_framesize,
            ///    mStreamInfo.sample_rate, mStreamInfo.min_blocksize, mStreamInfo.max_blocksize, mStreamInfo.channels, mStreamInfo.bits_per_sample);
            mTrackMetadata->setData(kKeyFlacMetaInfo, 0, &FlacMetaInfo, sizeof(FlacMetaInfo));

        }
    }
    else
    {
        LOGE("missing STREAMINFO");
        return NO_INIT;
    }

    if (mFileMetadata != 0)
    {
        mFileMetadata->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_FLAC);
    }

    return OK;
}


unsigned char  FLACSource::FLAC__crc8(const unsigned char *data, unsigned len)
{
    unsigned char crc = 0;

    if (data == NULL)
    {
        return 0;
    }

    while (len--)
    {
        crc = crc8_table[crc ^ *data++];
    }

    return crc;
}

bool FLACSource::FlacResync(
    const sp<DataSource> &source, uint32_t match_header,
    off_t *inout_pos, uint64_t *frm_num, int maxbuffcnt)
{
    ///LOGD("FLACSource::FlacResync 0x%08x", *inout_pos);
    off_t pos = *inout_pos;
    bool valid = false;
    const size_t kMaxReadBytes = 1024;
    const off_t kMaxBytesChecked = 128 * 1024 * maxbuffcnt;
    uint8_t buf[kMaxReadBytes];
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;
    uint32_t next_pos_offset;
    uint64_t tmp_frm_size = 0;

    if (frm_num)
    {
        *frm_num = 0;
    }

    do
    {
        if (pos >= *inout_pos + kMaxBytesChecked)
        {
            // Don't scan forever.
            LOGD("giving up at offset %ld", pos);
            break;
        }

        if (remainingBytes < 16)
        {
            if (reachEOS)
            {
                break;
            }
            else
            {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                             * The next read position should start from the end of
                             * the last buffer, and thus should include the remaining
                             * bytes in the buffer.
                             */
                totalBytesRead = source->readAt(pos + remainingBytes,
                                                buf + remainingBytes,
                                                bytesToRead);

                if (totalBytesRead <= 0)
                {
                    break;
                }

                ///LOGD("FLACSource::read %x, %d, %d, %d ", pos, remainingBytes, totalBytesRead, bytesToRead);
                reachEOS = (totalBytesRead != bytesToRead);
                totalBytesRead += remainingBytes;
                remainingBytes = totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        ///LOGD("FLACSource::FindNextSycnFrmPos %x, %x, %d, %d, %d", pos, next_pos_offset, kMaxReadBytes-bytesToRead, remainingBytes, totalBytesRead);
        if (!FindNextSycnFrmPos((uint8_t *)(buf), remainingBytes, 0,
                                (uint32_t *)&next_pos_offset, (uint64_t *)&tmp_frm_size))
        {
            pos += next_pos_offset;
            tmp += next_pos_offset;
            remainingBytes -= next_pos_offset;
            continue;
        }

        size_t frame_size;
        int sample_rate, num_channels, bitrate;

        off_t test_pos = pos + next_pos_offset;

        valid = true;

        if (valid)
        {
            *inout_pos = test_pos;

            if (frm_num != NULL)
            {
                *frm_num = tmp_frm_size;
            }

            ///LOGD("FLACSource::FindNextSycnPos %x, %x, %d ", pos, remainingBytes, totalBytesRead);
        }
        else
        {
            LOGD("no dice, no valid sequence of frames found.");
        }

        ++pos;
        ++tmp;
        --remainingBytes;
    }
    while (!valid);

    return valid;
}


bool FLACSource:: FindNextSycnPos(uint8_t *buf, ssize_t buf_size, off_t start_pos,
                                  uint32_t *next_pos_offset, uint64_t *frm_size)
{

    off_t crc_len = 0;
    *next_pos_offset = 0;

    bool valid = false;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;
    tmp += start_pos;
    buf_size -= start_pos;

    bool ref_block = false;
    bool ref_samplerate = false;
    bool ref_blkbit = false;

    uint32_t blk_s_len = 0;
    uint32_t rate_s_len = 0;
    uint32_t blk_bit_len = 0;
    uint8_t crcbyte = 0;

    uint16_t blk_bit = 0;
    uint16_t sample_bit = 0;
    uint16_t chn_bit = 0;
    uint16_t samplesize_bit = 0;

    uint64_t v = 0;
    uint8_t x = 0;

    if (frm_size)
    {
        *frm_size = 0;
    }

    if (buf_size < 2)
    {
        return false;
    }

    *next_pos_offset += start_pos;

    do
    {
        rate_s_len = 0;
        blk_bit_len = 0;
        blk_s_len = 0;
        ref_block = true;

        uint16_t header = U16_AT(tmp);

        if ((header & 0xffff) == kFlacMask)
        {
            ///LOGD("found possible  %x, %x, %d, %d", header, *next_pos_offset, buf_size, start_pos);
            tmp += 2;
            header = U16_AT(tmp);

            blk_bit = (header & 0xf000) >> 12;
            sample_bit = (header & 0x0f00) >> 8;
            chn_bit = (header & 0x00f0) >> 4;
            samplesize_bit = header & 0x000f;

            if ((blk_bit == 0x00) || (blk_bit == 0x0f) || (sample_bit == 0x0f) || (chn_bit > 0x0a)
                    || (samplesize_bit == 0x0e) || (samplesize_bit == 0x06) || ((samplesize_bit & 0x01) == 0x01))
            {
                tmp -= 2;
                goto Next_check;
            }

            if (blk_bit == 6)
            {
                blk_bit_len = 1;
            }
            else if (blk_bit == 7)
            {
                blk_bit_len = 2;
            }

            if (sample_bit == 0x0c)
            {
                rate_s_len = 1;
            }
            else if ((sample_bit == 0x0d) || (sample_bit == 0x0e))
            {
                rate_s_len = 2;
            }

            if (*next_pos_offset > (buf_size - 10))
            {
                return false;
            }

            tmp += 2;
            ///if(ref_block)
            ///header = U16_AT(tmp);

            x = *tmp;

            if (!(x & 0x80))  /* 0xxxxxxx */
            {
                v = x;
                blk_s_len = 0;
            }
            else if (x & 0xC0 && !(x & 0x20))  /* 110xxxxx */
            {
                v = x & 0x1F;
                blk_s_len = 1;
            }
            else if (x & 0xE0 && !(x & 0x10))  /* 1110xxxx */
            {
                v = x & 0x0F;
                blk_s_len = 2;
            }
            else if (x & 0xF0 && !(x & 0x08))  /* 11110xxx */
            {
                v = x & 0x07;
                blk_s_len = 3;
            }
            else if (x & 0xF8 && !(x & 0x04))  /* 111110xx */
            {
                v = x & 0x03;
                blk_s_len = 4;
            }
            else if (x & 0xFC && !(x & 0x02)) /* 1111110x */
            {
                v = x & 0x01;
                blk_s_len = 5;
            }
            else if (x & 0xFE && !(x & 0x01))  /* 11111110 */
            {
                v = 0;
                blk_s_len = 6;
            }
            else
            {
                tmp -= 4;
                goto Next_check;
            }

            crc_len = 5 + blk_s_len + rate_s_len + blk_bit_len;
            tmp += 1;
            uint8_t *frm_tmp = tmp;

            if (frm_size)
            {
                for (uint32_t i = blk_s_len ; i > 0; i--)
                {
                    x = *frm_tmp;///(uint8_t) (U16_AT(tmp)>>8);
                    frm_tmp += 1;

                    if (!(x & 0x80) || (x & 0x40))  /* 10xxxxxx */
                    {
                        *frm_size = uint64_t(0xffffffffffffffff);
                        ///tmp -= (blk_s_len -i);
                        break;
                    }

                    v <<= 6;
                    v |= (x & 0x3F);
                }

                if (ref_block == false)
                {
                    *frm_size = 0;
                }
                else
                {
                    *frm_size = v;
                }
            }

            if (*next_pos_offset > (buf_size - crc_len - 1))
            {
                return false;
            }

            crcbyte =  *(buf + *next_pos_offset + crc_len);

            if (FLAC__crc8((buf + *next_pos_offset), crc_len) == crcbyte)
            {
                ///LOGD("found possible sync frame crc %x, %x, len %d, %x, %d, %d, %d",
                ///    FLAC__crc8((buf+ *next_pos_offset), crc_len), *next_pos_offset, crc_len, crcbyte,
                ///    blk_s_len , rate_s_len, blk_bit_len);
                return true;
            }
            else
            {
                tmp -= 5;
                ///LOGD("found possible sync frame crc err  %x, %x, len %d, %x, %d, %d, %d",
                ///    FLAC__crc8((buf+ *next_pos_offset), crc_len), *next_pos_offset, crc_len, crcbyte,
                ///    blk_s_len , rate_s_len, blk_bit_len);
            }

        }

Next_check:
        *next_pos_offset += 1;
        tmp += 1;
    }
    while (*next_pos_offset < (buf_size - 4));

    ///LOGD("found error return %x", *next_pos_offset);
    return false;
}


bool FLACSource:: FindNextSycnFrmPos(uint8_t *buf, ssize_t buf_size, off_t start_pos,
                                     uint32_t *next_pos_offset, uint64_t *frm_size)
{

    ///off_t pos = buf;
    ///off_t crc_pos = buf;
    off_t crc_len = 0;
    *next_pos_offset = 0;

    bool valid = false;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;
    tmp += start_pos;
    buf_size -= start_pos;

    bool ref_block = false;
    bool ref_samplerate = false;
    bool ref_blkbit = false;

    uint32_t blk_s_len = 0;
    uint32_t rate_s_len = 0;
    uint32_t blk_bit_len = 0;
    uint8_t crcbyte = 0;

    uint16_t blk_bit = 0;
    uint16_t sample_bit = 0;
    uint16_t chn_bit = 0;
    uint16_t samplesize_bit = 0;

    uint64_t v = 0;
    uint8_t x = 0;

    if (frm_size)
    {
        *frm_size = 0;
    }

    if (buf_size < 2)
    {
        return false;
    }

    *next_pos_offset += start_pos;

    do
    {
        rate_s_len = 0;
        blk_bit_len = 0;
        blk_s_len = 0;

        uint16_t header = U16_AT(tmp);

        if ((header & 0xffff) == kFlacMask)
        {
            tmp += 2;
            header = U16_AT(tmp);

            blk_bit = (header & 0xf000) >> 12;
            sample_bit = (header & 0x0f00) >> 8;
            chn_bit = (header & 0x00f0) >> 4;
            samplesize_bit = header & 0x000f;

            if ((blk_bit == 0x00) || (blk_bit == 0x0f) || (sample_bit == 0x0f) || (chn_bit > 0x0a)
                    || (samplesize_bit == 0x0e) || (samplesize_bit == 0x06) || ((samplesize_bit & 0x01) == 0x01))
            {
                tmp -= 2;
                goto Next_Frmcheck;
            }

            if (blk_bit == 6)
            {
                blk_bit_len = 1;
            }
            else if (blk_bit == 7)
            {
                blk_bit_len = 2;
            }

            if (sample_bit == 0x0c)
            {
                rate_s_len = 1;
            }
            else if ((sample_bit == 0x0d) || (sample_bit == 0x0e))
            {
                ///LOGD("found rate_s_len  2 %x", header);
                rate_s_len = 2;
            }

            if (*next_pos_offset > (buf_size - 10))
            {
                return false;
            }

            tmp += 2;

            x = *tmp;///(uint8_t)(header>>8);

            if (!(x & 0x80))  /* 0xxxxxxx */
            {
                v = x;
                blk_s_len = 0;
            }
            else if (x & 0xC0 && !(x & 0x20))  /* 110xxxxx */
            {
                v = x & 0x1F;
                blk_s_len = 1;
            }
            else if (x & 0xE0 && !(x & 0x10))  /* 1110xxxx */
            {
                v = x & 0x0F;
                blk_s_len = 2;
            }
            else if (x & 0xF0 && !(x & 0x08))  /* 11110xxx */
            {
                v = x & 0x07;
                blk_s_len = 3;
            }
            else if (x & 0xF8 && !(x & 0x04))  /* 111110xx */
            {
                v = x & 0x03;
                blk_s_len = 4;
            }
            else if (x & 0xFC && !(x & 0x02)) /* 1111110x */
            {
                v = x & 0x01;
                blk_s_len = 5;
            }
            else if (x & 0xFE && !(x & 0x01))  /* 11111110 */
            {
                v = 0;
                blk_s_len = 6;
            }
            else
            {
                tmp -= 4;
                goto Next_Frmcheck;
            }

            crc_len = 5 + blk_s_len + rate_s_len + blk_bit_len;
            tmp += 1;
            uint8_t *frm_tmp = tmp;

            if (frm_size)
            {
                for (uint32_t i = blk_s_len ; i > 0; i--)
                {
                    x = *frm_tmp;///(uint8_t) (U16_AT(tmp)>>8);
                    frm_tmp += 1;

                    if (!(x & 0x80) || (x & 0x40))  /* 10xxxxxx */
                    {
                        *frm_size = uint64_t(0xffffffffffffffff);
                        ///tmp -= (blk_s_len -i);
                        break;
                    }

                    v <<= 6;
                    v |= (x & 0x3F);
                }

                if (ref_block == false)
                {
                    *frm_size = 0;
                }
                else
                {
                    *frm_size = v;
                }
            }

            if (*next_pos_offset > (buf_size - crc_len - 1))
            {
                return false;
            }

            crcbyte =  *(buf + *next_pos_offset + crc_len);

            if (FLAC__crc8((buf + *next_pos_offset), crc_len) == crcbyte)
            {
                ///LOGD("found possible sync frame crc %x, %x, len %d, %x, %d, %d, %d",
                ///    FLAC__crc8((buf+ *next_pos_offset), crc_len), *next_pos_offset, crc_len, crcbyte,
                ///    blk_s_len , rate_s_len, blk_bit_len);
                return true;
            }
            else
            {
                tmp -= 5;
                ///LOGD("found sync frame crc err  %x, %x, len %d, %x, %d, %d, %d",
                ///    FLAC__crc8((buf+ *next_pos_offset), crc_len), *next_pos_offset, crc_len, crcbyte,
                ///    blk_s_len , rate_s_len, blk_bit_len);
            }

        }

Next_Frmcheck:

        *next_pos_offset += 1;
        tmp += 1;
    }
    while (*next_pos_offset < (buf_size - 4));

    return false;
}



status_t FLACSource::getNextFramePos(off_t *curPos, off_t *pNextPos, int64_t *frameTsUs)
{
    uint8_t flacheader[4];
    uint32_t cur_frame_num = 0;
    uint32_t mFixedHeader = kFlacMask;
    {
        off_t pos = *curPos;

        pos = *curPos + mMinBufoffset;

        if (!FlacResync(mDataSource, mFixedHeader, &pos, NULL, 1))
        {
            LOGD("Unable to resync. Signalling end of stream.");
            return ERROR_END_OF_STREAM;
        }

        *pNextPos = pos;
        // Try again with the new position.
    }

    if (mMetaInfo.sample_rate > 0)
    {
        *frameTsUs = (int64_t)mMetaInfo.max_blocksize * 1000000ll / mMetaInfo.sample_rate;
    }

    ///else
    ///*frameTsUs= (int64_t)(cur_frame_num-mCurrentfrmTime)*1000000ll/mMetaInfo.sample_rate;

    mCurrentfrmTime++;
    ///LOGD("getNextFramePos:: Cur 0x%08x, Next = 0x%08x, time %lld, frm %d, %d", *curPos, *pNextPos, *frameTsUs, mCurrentfrmTime, cur_frame_num);
    return OK;
}

bool FLACSource::getSeekPosforStreaming(int64_t targetTimeUS, off_t *mCurrentSeekPos, int *seekFrm)
{
    status_t ret = false;

    if ((mMetaInfo.sample_rate <= 0) || (mMetaInfo.max_blocksize <= 0) ||
            (mMetaInfo.min_framesize <= 0) || (mMetaInfo.max_framesize <= 0))
    {
        return ret;
    }

    int64_t targetFrmNum = targetTimeUS * mMetaInfo.sample_rate / (1000000ll * mMetaInfo.max_blocksize);
    off64_t filesize;
    mDataSource->getSize(&filesize);
    uint64_t  upper_frmnum = mMetaInfo.total_samples / mMetaInfo.max_blocksize;
    uint64_t  lower_frmnum = 0;
    uint64_t new_frmnum = 0;
    uint32_t next_pos_offset = 0;

    int32_t SeekOffset = (mMetaInfo.min_framesize + mMetaInfo.max_framesize) / 2;
    int32_t SeekFrmOffset = ((mMetaInfo.total_samples / mMetaInfo.sample_rate + 60) / 60) * mMetaInfo.sample_rate / mMetaInfo.max_blocksize;

    if (SeekFrmOffset < 10)
    {
        SeekFrmOffset = 10;
    }

    FLAC__uint64 lower_bound = mFirstFramePos;
    FLAC__uint64 upper_bound = filesize;
    off_t TargetFrmPos = lower_bound + (upper_bound - lower_bound) * (targetFrmNum - lower_frmnum) / (upper_frmnum - lower_frmnum);
    ssize_t n = 0;

    LOGD("getSeekPosforStreaming:: T_FrmNumber = %lld, Frmoffset = %d", targetFrmNum, SeekFrmOffset);

    char *tempseekbuffer = new char[mMetaInfo.max_framesize];

    if (tempseekbuffer == NULL)
    {
        LOGD("getSeekPosforStreaming:: new memory fail ");
        goto getSeek_Exit;
    }

    while (1)
    {
        n = mDataSource->readAt(TargetFrmPos, tempseekbuffer, mMetaInfo.max_framesize);

        if (n <= 0)
        {
            LOGD("getSeekPosforStreaming:: read file frmunm %x is 0!!", TargetFrmPos);
            goto getSeek_Exit;
        }

        if (!FindNextSycnPos((uint8_t *)tempseekbuffer, n, 0, (uint32_t *)&next_pos_offset, &new_frmnum))
        {
            LOGD("getSeekPosforStreaming:: do not find %x", TargetFrmPos);
            TargetFrmPos += SeekOffset;
            continue;
        }

        if (new_frmnum >= targetFrmNum)
        {
            upper_bound = TargetFrmPos;
            upper_frmnum = new_frmnum;
        }
        else if (new_frmnum <= targetFrmNum)
        {
            lower_bound = TargetFrmPos;
            lower_frmnum = new_frmnum;
        }

        if (((targetFrmNum - SeekFrmOffset) <= new_frmnum) &&
                (new_frmnum <= (targetFrmNum + SeekFrmOffset)))
        {
            LOGD("getSeekPosforStreaming:: to frm l_frm %lld, u_frm %lld, l_b %llx, u_b %llx", lower_frmnum,
                 upper_frmnum, lower_bound, upper_bound);
            break;
        }

        if ((upper_bound - lower_bound) <  2 * mMetaInfo.max_framesize)
        {
            LOGD("getSeekPosforStreaming:: l_b %llx, u_b %llx, to minoffset %llx, %x",
                 lower_bound,  upper_bound, 2 * mMetaInfo.max_framesize);
            break;
        }

        TargetFrmPos = lower_bound + (upper_bound - lower_bound) * (targetFrmNum - lower_frmnum) / (upper_frmnum - lower_frmnum);

    }

    ret = true;
    *mCurrentSeekPos = TargetFrmPos + next_pos_offset;
    *seekFrm = new_frmnum;

getSeek_Exit:

    if (tempseekbuffer)
    {
        delete [] tempseekbuffer;
    }

    return ret;

}

status_t FLACSource::sendDurationUpdateEvent(int64_t duration)
{
    if (mObserver && (bTocEnable))
    {
        mObserver->postDurationUpdateEvent(duration);
        LOGD("Seek Table :duration=%lld", duration);
    }

    return OK;

}

FLACSource::FLACSource(
    const sp<DataSource> &dataSource,
    const sp<MetaData> &trackMetadata, FLAC__StreamMetadata_StreamInfo MetaInfo, const sp<FLACParser> &Parser)
    : mDataSource(dataSource),
      mParser(Parser),
      mTrackMetadata(trackMetadata),
      mInitCheck(false),
      mStarted(false),
      mMetaInfo(MetaInfo)
{
    LOGV("FLACSource::FLACSource, %d, %d, %d", mParser->mSeekTableValid, mDataSource->flags(), DataSource::kIsCachingDataSource);
    mCurrentPos = 0;
    mMaxBufferSize = 0;
    mCurrentTimeUs = 0;
    mCurrentFrm = 0;
    mCurrentfrmTime = 0;

    if ((mParser->mSeekTableValid == true) ||
            (mDataSource->flags() & DataSource::kIsCachingDataSource))
    {
        bTocEnable = false;
    }
    else
    {
        bTocEnable = true;
    }

    
    void *pAwe = NULL;
    mTrackMetadata->findPointer(kKeyDataSourceObserver, &pAwe);
    mObserver = (AwesomePlayer *) pAwe;

    mInitCheck = init();
    mFirstFramePos = mCurrentPos;
#ifdef ENABLE_MMRIOTHREAD    
    startRIOThread(mDataSource, mFirstFramePos, mMaxBufferSize, 6);
#endif
    
}

FLACSource::~FLACSource()
{
    LOGV("~FLACSource::FLACSource");

    if (mStarted)
    {
        stop();
    }
}

status_t FLACSource::start(MetaData *params)
{
    if (mInitCheck != OK)
        //if(mInitCheck == NO_INIT)
    {
        return ERROR_UNSUPPORTED;
    }

    CHECK(!mStarted);
    LOGV("FLACSource::start, %d, %d, %d", mParser->mSeekTableValid, mDataSource->flags(), DataSource::kIsCachingDataSource);
    mGroup = new MediaBufferGroup;

    mGroup->add_buffer(new MediaBuffer(mMaxBufferSize));
    ///mCurrentPos = pos;
    LOGV("FLACSource::start offset: 0x%08x, bufsize %d, minbuff %d", mCurrentPos, mMaxBufferSize, mMinBufoffset);

    mFirstFramePos = mCurrentPos;

    if (bTocEnable)
    {
        startTOCThread(mFirstFramePos, 1024);
    }

    mStarted = true;
    LOGV("FLACSource::start done");

    return OK;
}

status_t FLACSource::stop()
{
    LOGV("FLACSource::stop");

    CHECK(mStarted);
    ///mParser->releaseBuffers();

    if (bTocEnable)
    {
        stopTOCThread();
    }

#ifdef ENABLE_MMRIOTHREAD
    stopRIOThread();
#endif

    delete mGroup;
    mGroup = NULL;

    mStarted = false;

    return OK;
}

sp<MetaData> FLACSource::getFormat()
{
    return mTrackMetadata;
}

status_t FLACSource::read(
    MediaBuffer **outBuffer, const ReadOptions *options)
{
    *outBuffer = NULL;

    off_t pActualPos;

    ///LOGD("FLACSource::read addr %llx", mCurrentPos);
    int64_t seekTimeUs;
    int64_t seektmpTimeUs = 0;
    ReadOptions::SeekMode mode;
    size_t frame_size = mMaxBufferSize;
    size_t n = 0;
    size_t nextfrm_offset = 0;

    size_t start_offset = 0;

    if (n > mMinBufoffset)
    {
        start_offset = mMinBufoffset;
    }
    else
    {
        start_offset = 4;
    }

    MediaBuffer *buffer;
    status_t err = mGroup->acquire_buffer(&buffer);

    if (err != OK)
    {
        LOGE("FLACSource::acquire_buffer fail");
        return err;
    }

    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
    {
        if (mParser->mSeekTableValid)
        {
            FLAC__uint64 sample = 0;
            FLAC__uint64 ptmpPos = 0;
            sample = (seekTimeUs * mMetaInfo.sample_rate) / 1000000LL;

            if (sample >= mMetaInfo.total_samples)
            {
                sample = mMetaInfo.total_samples;
            }

            if (mParser->parser_getseektable((FLAC__uint64 *)&sample, (FLAC__uint64 *)&ptmpPos))
            {
                seektmpTimeUs = sample * 1000000LL / mMetaInfo.sample_rate;
                mCurrentPos = ptmpPos + mFirstFramePos;
                LOGD("buildin seektable seektime=%lld,actual=%lld, filePos=%d, %d, %d", seekTimeUs, seektmpTimeUs, mCurrentPos,
                     mMetaInfo.max_blocksize, mMetaInfo.sample_rate);

                if (seektmpTimeUs < (seekTimeUs - (mMetaInfo.max_blocksize * 1000000LL / mMetaInfo.sample_rate)))
                {

                    while (1)
                    {
                        n = mDataSource->readAt(mCurrentPos, buffer->data(), frame_size);

                        if (FindNextSycnPos((uint8_t *)buffer->data(), frame_size, start_offset, (uint32_t *)&nextfrm_offset, NULL))
                        {
                            seektmpTimeUs += (mMetaInfo.max_blocksize * 1000000LL / mMetaInfo.sample_rate);
                            mCurrentPos += nextfrm_offset;

                            if (seektmpTimeUs >= (seekTimeUs - (mMetaInfo.max_blocksize * 1000000LL / mMetaInfo.sample_rate)))
                            {
                                mCurrentTimeUs = seektmpTimeUs;
                                break;
                            }
                        }
                        else
                        {
                            LOGD("after seeking error bad value");
                            buffer->release();
                            buffer = NULL;
                            return ERROR_UNSUPPORTED;
                        }
                    }
                }

            }
            else
            {
                LOGD("after seek error mCurrentTimeUs=%lld,ptmpPos=%ld", sample, ptmpPos);
                buffer->release();
                buffer = NULL;
                return ERROR_UNSUPPORTED;
            }
        }
        else if (mDataSource->flags() & DataSource::kIsCachingDataSource)
        {
            int Framenum = 0;
            off_t temp_mCurrentPos = 0;
            LOGD("FLACSource:: stream seek start %lld", seekTimeUs);

            if (getSeekPosforStreaming(seekTimeUs, &temp_mCurrentPos, &Framenum))
            {
                mCurrentPos = temp_mCurrentPos;
                mCurrentTimeUs = Framenum * mMetaInfo.max_blocksize * 1000000ll / mMetaInfo.sample_rate;
                mCurrentFrm = Framenum;
                LOGD("FLACSource:: stream seek done mCurrentTimeUs=%x,ptmpPos=%lld", mCurrentPos, mCurrentTimeUs);
            }
            else
            {
                LOGD("FLACSource:: stream seek fail");
            }
        }
        else
        {
            status_t stat = getFramePos(seekTimeUs, &mCurrentTimeUs, &pActualPos, false);

            if (stat == ERROR_END_OF_STREAM)
            {
                LOGE("FLACSource::seek to %lld fail, err %d !!!!!", seekTimeUs, stat);
                buffer->release();
                buffer = NULL;
                return stat;
            }
            else
            {
                mCurrentPos = pActualPos;
                LOGD("Toc seektable seektime=%lld,actual=%lld, filePos=%d", seekTimeUs, mCurrentTimeUs, mCurrentPos);
            }
        }

    }

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
    ///LOGD("FLACSource::readAt  %llx, %d, %d", mCurrentPos, buffer->size(), n);

    if (n <= 0)
    {
        buffer->release();
        buffer = NULL;
        LOGD("FLACSource::readAt fail %d %d", n, frame_size);
        return ERROR_END_OF_STREAM;
    }

    frame_size = n;
    nextfrm_offset = 0;

    if (FindNextSycnPos((uint8_t *)buffer->data(), frame_size, start_offset, (uint32_t *)&nextfrm_offset, NULL))
    {
        frame_size = nextfrm_offset;
    }
    else
    {

        off64_t filesize = 0;
        mDataSource->getSize(&filesize);
        frame_size = n;
        LOGD("FLACSource::find(%llx) syncpos %x read fail %d", filesize, mCurrentPos, frame_size);
         
        if ((off64_t)(mCurrentPos) < (filesize - mMaxBufferSize))
        {
            buffer->release();
            buffer = NULL;
            return ERROR_UNSUPPORTED;
        }
    }

    buffer->set_range(0, frame_size);

    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
    {
        buffer->meta_data()->setInt64(kKeyTime, mCurrentTimeUs);
        mCurrentFrm = (off_t)((mCurrentTimeUs * mMetaInfo.sample_rate) / (mMetaInfo.max_blocksize * 1000000ll));
        ///LOGV("FLACSource:seek time %lld %d",mCurrentTimeUs, mCurrentFrm);
    }
    else
    {
        mCurrentTimeUs = (int64_t)(mCurrentFrm * mMetaInfo.max_blocksize * 1000000ll) / (mMetaInfo.sample_rate) ;
        buffer->meta_data()->setInt64(kKeyTime, mCurrentTimeUs);
    }

    buffer->meta_data()->setInt32(kKeyIsSyncFrame, 1);

    mCurrentPos += frame_size;
#ifdef ENABLE_MMRIOTHREAD    
    UpdateReadPtr(frame_size);
#endif
    ///mCurrentTimeUs += (int64_t)(mCurrentFrm * mMetaInfo.max_blocksize * 1000000ll) / (mMetaInfo.sample_rate) ;

    *outBuffer = buffer;
    mCurrentFrm++;
    LOGV("FLACSource::kKeyTime Cur 0x%08x, frm %d--%lld, frm_size %d", mCurrentPos, mCurrentFrm, mCurrentTimeUs, frame_size);
    return OK;
}


status_t FLACSource::init()
{
    off_t pos = 0;
    typedef struct
    {
        unsigned min_blocksize, max_blocksize;
        unsigned min_framesize, max_framesize;
        unsigned sample_rate;
        unsigned channels;
        unsigned bits_per_sample;
        uint64_t total_samples;
        unsigned char md5sum[16];
        unsigned int mMaxBufferSize;
        bool      has_stream_info;
    } FLAC__StreamMetadata_Info_;
    FLAC__StreamMetadata_Info_ FlacMetaInfo;

    if (!FlacResync(mDataSource, kFlacMask, &pos, NULL, 8192))
    {
        LOGD("FLACSource::start first pos fail");
        memcpy(&FlacMetaInfo, &mMetaInfo, sizeof(mMetaInfo));
        mTrackMetadata->setData(kKeyFlacMetaInfo, 0, &FlacMetaInfo, sizeof(FlacMetaInfo));
        return NO_INIT;
    }

    mCurrentPos = pos;
    ///return OK;
    uint8_t buf[14];
    uint8_t *tmp = buf;
    ssize_t n = mDataSource->readAt(pos, buf, 14);

    if (n == 14)
    {
        if ((tmp[1] & 0x1) == 1)
        {
            LOGD("FLACSource::start varialbe block, it will error!!!!!");
        }

        unsigned blocksize,  sample_rate, bps;
        uint32_t blk_s_len = 0;
        uint32_t rate_s_len = 0;
        uint32_t blk_bit_len = 0;
        uint16_t blk_bit = 0;
        uint16_t sample_bit = 0;
        uint16_t samplesize_bit = 0;
        uint8_t x = 0;

        tmp += 2;
        uint16_t header = U16_AT(tmp);

        blk_bit = (header & 0xf000) >> 12;
        sample_bit = (header & 0x0f00) >> 8;
        samplesize_bit = header & 0x000f;

        if (blk_bit == 6)
        {
            blk_bit_len = 1;
        }
        else if (blk_bit == 7)
        {
            blk_bit_len = 2;
        }
        else
        {
            if (blk_bit > 7)
            {
                blocksize = (256 << (blk_bit - 8));
            }
            else if (blk_bit > 1)
            {
                blocksize = (576 << (blk_bit - 2));
            }
            else
            {
                blocksize = 192;
            }
        }

        if (sample_bit == 0x0c)
        {
            rate_s_len = 1;
        }
        else if ((sample_bit == 0x0d) || (sample_bit == 0x0e))
        {
            rate_s_len = 2;
        }
        else
        {
            if (sample_bit == 0)
            {
                sample_rate = mMetaInfo.sample_rate;
            }
            else if (sample_bit == 1)
            {
                sample_rate = 88200;
            }
            else if (sample_bit == 2)
            {
                sample_rate = 176400;
            }
            else if (sample_bit == 3)
            {
                sample_rate = 192000;
            }
            else if (sample_bit == 4)
            {
                sample_rate = 8000;
            }
            else if (sample_bit == 5)
            {
                sample_rate = 16000;
            }
            else if (sample_bit == 6)
            {
                sample_rate = 22050;
            }
            else if (sample_bit == 7)
            {
                sample_rate = 24000;
            }
            else if (sample_bit == 8)
            {
                sample_rate = 32000;
            }
            else if (sample_bit == 9)
            {
                sample_rate = 44100;
            }
            else if (sample_bit == 10)
            {
                sample_rate = 48000;
            }
            else
            {
                sample_rate = 96000;
            }

        }

        if (samplesize_bit == 2)
        {
            bps = 8;
        }
        else if (samplesize_bit == 4)
        {
            bps = 12;
        }
        else if (samplesize_bit == 8)
        {
            bps = 16;
        }
        else if (samplesize_bit == 10)
        {
            bps = 20;
        }
        else if (samplesize_bit == 12)
        {
            bps = 24;
        }
        else
        {
            bps = mMetaInfo.bits_per_sample;
        }

        if ((rate_s_len != 0) || (blk_bit_len != 0))
        {
            tmp += 2;
            x = *tmp;

            if (!(x & 0x80))  /* 0xxxxxxx */
            {
                blk_s_len = 0;
            }
            else if (x & 0xC0 && !(x & 0x20))  /* 110xxxxx */
            {
                blk_s_len = 1;
            }
            else if (x & 0xE0 && !(x & 0x10))  /* 1110xxxx */
            {
                blk_s_len = 2;
            }
            else if (x & 0xF0 && !(x & 0x08))  /* 11110xxx */
            {
                blk_s_len = 3;
            }
            else if (x & 0xF8 && !(x & 0x04))  /* 111110xx */
            {
                blk_s_len = 4;
            }
            else if (x & 0xFC && !(x & 0x02)) /* 1111110x */
            {
                blk_s_len = 5;
            }
            else if (x & 0xFE && !(x & 0x01))  /* 11111110 */
            {
                blk_s_len = 6;
            }

            tmp += (blk_s_len + 1);

            if (blk_bit_len == 1)
            {
                blocksize = *tmp + 1;
            }
            else if (blk_bit_len == 2)
            {
                blocksize = U16_AT(tmp) + 1;
            }

            tmp += blk_bit_len;

            if (rate_s_len == 1)
            {
                sample_rate = (*tmp * 1000);
            }
            else if (rate_s_len == 2)
            {
                sample_rate = U16_AT(tmp);
            }
        }

        mMetaInfo.bits_per_sample = bps;
        mMetaInfo.max_blocksize = blocksize;
        mMetaInfo.sample_rate = sample_rate;
        LOGD("FLACSource::start headerinfo: block %d-%x, sample %d-%x, bps %d-%x, blk_len %d, bit_len %d",
             blocksize, blk_bit, sample_rate, sample_bit, bps, samplesize_bit, blk_s_len, blk_bit_len);
    }

    /*
    if ((mMetaInfo.channels == 0) || (mMetaInfo.channels > 3))
    {
        mMetaInfo.channels = 3;
    }
    */
    if ((mMetaInfo.channels == 0))
    {
        ALOGE("init,abornal:channels:0");
        mMetaInfo.channels = 3;
    }

    if (mMetaInfo.max_framesize > 16)
    {
        mMaxBufferSize = mMetaInfo.max_framesize + 16;
    }
    else
    {
        mMaxBufferSize = mMetaInfo.max_blocksize * mMetaInfo.channels * mMetaInfo.bits_per_sample / 8;
    }

    if (mMetaInfo.min_framesize > 16)
    {
        mMinBufoffset = mMetaInfo.min_framesize;
    }
    else
    {
        mMinBufoffset = 16;
    }

    FlacMetaInfo.mMaxBufferSize = mMaxBufferSize;
    FlacMetaInfo.has_stream_info = mParser->mStreamInfoValid;
    memcpy(&FlacMetaInfo, &mMetaInfo, sizeof(mMetaInfo));
    mTrackMetadata->setData(kKeyFlacMetaInfo, 0, &FlacMetaInfo, sizeof(FlacMetaInfo));
    return OK;
}

FLACExtractor::FLACExtractor(
    const sp<DataSource> &dataSource)
    : mDataSource(dataSource),
      mInitCheck(false)
{
    LOGV("FLACExtractor::FLACExtractor %p", this);
    mInitCheck = init();
}

FLACExtractor::~FLACExtractor()
{
    LOGV("~FLACExtractor::FLACExtractor %p", this);
}

size_t FLACExtractor::countTracks()
{
    return mInitCheck == OK ? 1 : 0;
}

sp<MediaSource> FLACExtractor::getTrack(size_t index)
{
    if (mInitCheck != OK || index > 0)
    {
        return NULL;
    }

    return new FLACSource(mDataSource, mTrackMetadata, mParser->mStreamInfo, mParser);
}

sp<MetaData> FLACExtractor::getTrackMetaData(
    size_t index, uint32_t flags)
{
    if (mInitCheck != OK || index > 0)
    {
        return NULL;
    }

    return mTrackMetadata;
}

status_t FLACExtractor::init()
{
    mFileMetadata = new MetaData;
    mTrackMetadata = new MetaData;
    // FLACParser will fill in the metadata for us
    mParser = new FLACParser(mDataSource, mFileMetadata, mTrackMetadata);
    return mParser->initCheck();
}

sp<MetaData> FLACExtractor::getMetaData()
{
    return mFileMetadata;
}

bool SniffFLAC(
    const sp<DataSource> &source, String8 *mimeType, float *confidence,
    sp<AMessage> *)
{
    // first 4 is the signature word
    uint8_t header[4 + 4];

    if (source->readAt(0, header, sizeof(header)) != sizeof(header)
            || memcmp("fLaC", header, 4))   ///memcmp("fLaC\0\0\0\042", header, 4+4))
    {
        return false;
    }

    *mimeType = MEDIA_MIMETYPE_AUDIO_FLAC;
    *confidence = 0.5;

    return true;
}

}  // namespace android
