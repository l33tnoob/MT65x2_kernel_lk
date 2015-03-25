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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ASFExtractor.cpp
 *
 * Project:
 * --------
 *   MT6573
 *
 * Description:
 * ------------
 *   ASF Extractor implementation
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/


#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "ASFExtractor"

#include "ASFExtractor.h"

#include <arpa/inet.h>

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/avc_utils.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#ifdef MTK_DEMUXER_QUERY_CAPABILITY_FROM_DRV_SUPPORT
#include "vdec_drv_if_public.h"
#include "val_types_public.h"
#endif

namespace android
{


//#define SEEK_BY_SEEK_MODE_SET

extern "C" bool mtk_asf_extractor_recognize(const sp<DataSource> &source)
{
    ALOGI("[ASF]mtk_asf_extractor_recognize IN\n");
    sp<ASFExtractor> extractor = new ASFExtractor(source);
    bool ret = extractor->IsValidAsfFile();
    extractor = NULL;//deconstruct
    ALOGI("[ASF]mtk_asf_extractor_recognize OUT\n");
    return ret;
}


extern "C" sp<MediaExtractor> mtk_asf_extractor_create_instance(const sp<DataSource> &source)
{
    ALOGI("[ASF]mtk_asf_extractor_create_instance\n");
    return new ASFExtractor(source);
}


#define ASF_DEBUG_LOGE(x, ...)  //ALOGE("[ASFExtractor]: "x,  ##__VA_ARGS__)
#define ASF_DEBUG_LOGV(x, ...) // ALOGE("[ASFExtractor]: "x,  ##__VA_ARGS__)

static const uint32_t kMP3HeaderMask = 0xfffe0c00;//0xfffe0cc0 add by zhihui zhang no consider channel mode

// copy from MP3Extractor
static bool get_mp3_frame_size(
    uint32_t header, size_t *frame_size,
    int *out_sampling_rate = NULL, int *out_channels = NULL,
    int *out_bitrate = NULL,int *out_sampleperframe=NULL)
{
    *frame_size = 0;
    int sampleperframe=0;
    if (out_sampling_rate)
    {
        *out_sampling_rate = 0;
    }

    if (out_channels)
    {
        *out_channels = 0;
    }

    if (out_bitrate)
    {
        *out_bitrate = 0;
    }

    if ((header & 0xffe00000) != 0xffe00000)
    {
        return false;
    }

    unsigned version = (header >> 19) & 3;

    if (version == 0x01)
    {
        return false;
    }

    unsigned layer = (header >> 17) & 3;

    if (layer == 0x00)
    {
        return false;
    }// else if(layer == 2){
    //   return false;
    //}
    else if(layer == 3)
    {
        ALOGV("Layer I");
        return false;
    }

    //add by zhihui zhang for mp2 framesize calculate
    if(layer== 2 ||(version == 3 && layer==1))
    {
        sampleperframe=1152;
    }
    else if(layer==3)
    {
        sampleperframe=384;
    }
    else if((version == 2 || version==0) && layer == 1)
    {
        sampleperframe=576;
    }
    if(out_sampleperframe!=NULL)
    {
        *out_sampleperframe=sampleperframe;
    }

    unsigned protection = (header >> 16) & 1;

    unsigned bitrate_index = (header >> 12) & 0x0f;

    if (bitrate_index == 0 || bitrate_index == 0x0f)
    {
        // Disallow "free" bitrate.
        return false;
    }

    unsigned sampling_rate_index = (header >> 10) & 3;

    if (sampling_rate_index == 3)
    {
        return false;
    }

    static const int kSamplingRateV1[] = { 44100, 48000, 32000 };
    int sampling_rate = kSamplingRateV1[sampling_rate_index];
    if (version == 2 /* V2 */)
    {
        sampling_rate /= 2;
    }
    else if (version == 0 /* V2.5 */)
    {
        sampling_rate /= 4;
    }

    unsigned padding = (header >> 9) & 1;

    if (layer == 3)
    {
        // layer I

        static const int kBitrateV1[] =
        {
            32, 64, 96, 128, 160, 192, 224, 256,
            288, 320, 352, 384, 416, 448
        };

        static const int kBitrateV2[] =
        {
            32, 48, 56, 64, 80, 96, 112, 128,
            144, 160, 176, 192, 224, 256
        };

        int bitrate =
            (version == 3 /* V1 */)
            ? kBitrateV1[bitrate_index - 1]
            : kBitrateV2[bitrate_index - 1];

        if (out_bitrate)
        {
            *out_bitrate = bitrate;
        }

        *frame_size = (12000 * bitrate / sampling_rate + padding) * 4;
    }
    else
    {
        // layer II or III

        static const int kBitrateV1L2[] =
        {
            32, 48, 56, 64, 80, 96, 112, 128,
            160, 192, 224, 256, 320, 384
        };

        static const int kBitrateV1L3[] =
        {
            32, 40, 48, 56, 64, 80, 96, 112,
            128, 160, 192, 224, 256, 320
        };

        static const int kBitrateV2[] =
        {
            8, 16, 24, 32, 40, 48, 56, 64,
            80, 96, 112, 128, 144, 160
        };

        int bitrate;
        if (version == 3 /* V1 */)
        {
            bitrate = (layer == 2 /* L2 */)
                      ? kBitrateV1L2[bitrate_index - 1]
                      : kBitrateV1L3[bitrate_index - 1];
        }
        else
        {
            // V2 (or 2.5)

            bitrate = kBitrateV2[bitrate_index - 1];
        }

        if (out_bitrate)
        {
            *out_bitrate = bitrate;
        }

        if (version == 3 /* V1 */)
        {
            //*frame_size = 144000 * bitrate / sampling_rate + padding;
            *frame_size = (sampleperframe*125) * bitrate / sampling_rate + padding;
        }
        else
        {
            // V2 or V2.5
            //*frame_size = 72000 * bitrate / sampling_rate + padding;
            *frame_size = (sampleperframe*125) * bitrate / sampling_rate + padding;
        }
    }

    if (out_sampling_rate)
    {
        *out_sampling_rate = sampling_rate;
    }

    if (out_channels)
    {
        int channel_mode = (header >> 6) & 3;

        *out_channels = (channel_mode == 3) ? 1 : 2;
    }

    return true;
}

static int mp3HeaderStartAt(const uint8_t *start, int length, int header)
{
    uint32_t code = 0;
    int i = 0;

    for(i=0; i<length; i++)
    {
        code = (code<<8) + start[i];
        if ((code & kMP3HeaderMask) == (header & kMP3HeaderMask))
        {
            // some files has no seq start code
            return i - 3;
        }
    }

    return -1;
}


struct ASFSource : public MediaSource
{
    ASFSource(const sp<ASFExtractor> &extractor, size_t index);

    virtual status_t start(MetaData *params);
    virtual status_t stop();

    virtual sp<MetaData> getFormat();
    ASFErrorType ReadFramePostProcess(MediaBuffer **out);

    virtual status_t read(
        MediaBuffer **buffer, const ReadOptions *options);

    status_t findMP3Header(int32_t * header);

private:

    sp<ASFExtractor> mExtractor;
    size_t mTrackIndex;
    Type mType;
    bool mIsVideo;
    int32_t mMP3Header;
    bool mIsMP3;
    bool mIsAVC;
    bool mIsMPEG4;
    bool mIsMJPEG;
    bool mWantsNALFragments;
    Mutex mLock;

    uint32_t mStreamId;
    bool mSeeking;
    MediaBuffer *mBuffer;

    ASFSource(const ASFSource &);
    ~ASFSource();
    ASFSource &operator=(const ASFSource &);

};

ASFSource::ASFSource(const sp<ASFExtractor> &extractor, size_t index)
    : mExtractor(extractor),
      mTrackIndex(index),
      mIsVideo(true),
      mMP3Header(0),
      mIsMP3(false),
      mIsAVC(false),
      mIsMPEG4(false),
      mIsMJPEG(false),
      mWantsNALFragments(false),
      mStreamId(0),    
      mSeeking(false), 
      mBuffer(NULL)     
{

    mStreamId = mExtractor->mTracks.itemAt(index).mTrackNum;

    ALOGI ("[ASF]ASFSource::ASFSource stream id =%d\n", mStreamId);

    TrackInfo *trackInfo = &(mExtractor->mTracks.editItemAt(mTrackIndex));

    if (trackInfo->mNextPacket)
    {
        mExtractor->mAsfParser->asf_packet_destroy(trackInfo->mNextPacket);
        trackInfo->mNextPacket = NULL;
        ALOGI ("[ASF]ASFSource::ASFSource stream id =%d, asf_packet_destroy\n", mStreamId);

    }
    ALOGI ("[ASF]ASFSource::ASFSource stream id =%d, asf_packet_create\n", mStreamId);

    trackInfo->mNextPacket =  mExtractor->mAsfParser->asf_packet_create();
    /* *///no need
    trackInfo->mCurPayloadIdx=0;

    const char *mime;
    CHECK(mExtractor->mTracks.itemAt(index).mMeta->
          findCString(kKeyMIMEType, &mime));

    if ((!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_WMV))
            || (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4))
            ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC))
            ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MJPEG)))
    {

        ALOGE(" MEDIA_MIMETYPE_VIDEO_RAW:mType = ASF_VIDEO");
        mType = ASF_VIDEO;

    }

    else if ((!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMA))
#ifdef MTK_SWIP_WMAPRO
             || (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMAPRO)) 
#endif
             || (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW))
             || (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG))
             || (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)))
    {
        ALOGE(" MEDIA_MIMETYPE_VIDEO_RAW:mType = ASF_AUDIO");
        mType = ASF_AUDIO;
    }

    //MP3
    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG))
    {
        mIsMP3 = true;

        if (findMP3Header(&mMP3Header) != OK)
        {
            ALOGW("No mp3 header found");
        }
        ALOGD("mMP3Header=0x%8.8x", mMP3Header);
    }
    //AVC
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC))
    {
        mIsAVC = true;
    }
    //MPEG4
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4))
    {
        mIsMPEG4 = true;
    }

    //MJPEG
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MJPEG))
    {
        mIsMJPEG = true;
    }
}


ASFSource::~ASFSource()
{
    ALOGI ("[ASF]~ASFSource stream id =%d", mStreamId);
    TrackInfo *trackInfo = &(mExtractor->mTracks.editItemAt(mTrackIndex));

    if (trackInfo->mNextPacket)
    {
        mExtractor->mAsfParser->asf_packet_destroy(trackInfo->mNextPacket);
        trackInfo->mNextPacket = NULL;
    }
    if (trackInfo->mCodecSpecificData!=NULL)
    {
        ALOGI("~ASFSource:free mCodecSpecificData=0x%08x\n",(uint32_t)(trackInfo->mCodecSpecificData));
        free(trackInfo->mCodecSpecificData);
        trackInfo->mCodecSpecificData = NULL;
    }

}

static int seqStartAt(const uint8_t *start, int length)
{
    uint32_t code = -1;
    int i = 0;

    for(i=0; i<length; i++)
    {
        code = (code<<8) + start[i];
        if (code == 0x000001b3 || code == 0x000001b6)
        {
            // some files has no seq start code
            return i - 3;
        }
    }

    return -1;
}

static int32_t nalStartAt(const uint8_t *start, int32_t length, int32_t *prefixLen)
{
    uint32_t code = -1;
    int32_t i = 0;

    for(i=0; i<length; i++)
    {
        code = (code<<8) + start[i];
        if ((code & 0x00ffffff) == 0x1)
        {
            int32_t fourBytes = code == 0x1;
            *prefixLen = 3 + fourBytes;
            return i - *prefixLen + 1;
        }
    }

    return -1;
}

static int32_t realAVCStart(const uint8_t *start, int32_t length)
{
    int32_t i = 0;
    for(i=0; i<length; i++)
    {
        if((start[i] == 0x00)
                && (start[i+1] == 0x00)
                && (start[i+2] == 0x01))
        {
            if((start[i+3] & 0x1f)  == 0x07 )
            {
                return i;
            }
        }
    }
    return 0;
}
//find SOI
static int32_t mjpegStartAt(const uint8_t *start, int32_t length)
{
    int32_t i = 0;
    for(i=0; i<length; i++)
    {
        if((start[i] == 0xff)
                && (start[i+1] == 0xd8))
        {
            return i;
        }
    }
    return -1;
}
//find EOI
static int32_t mjpegEndAt(const uint8_t *start, int32_t length)
{
    int32_t i = 0;
    for(i=0; i<length; i++)
    {
        if((start[i] == 0xff)
                && (start[i+1] == 0xd9))
        {
            return i+1;
        }
    }
    return -1;
}
status_t ASFSource::findMP3Header(int *pHeader)
{
    MediaBuffer *packet1 = NULL;
    uint32_t readPackets = 0;
    int header1 = 0;

    if (pHeader != NULL)
        *pHeader = 0;

    while(0 == *pHeader)
    {
        //read data from DataSource
        ASFErrorType retVal = ASF_SUCCESS;
        bool bIsKeyFrame = false;
        bool isSeeking = false;

        retVal = mExtractor->GetNextMediaFrame(&packet1, bIsKeyFrame,ASF_AUDIO,&isSeeking,mTrackIndex);//not seeking

        if ((ASF_END_OF_FILE == retVal) && (0 != header1))
        {
            ALOGE ("[ASF_ERROR]ASFSource::findMP3Header failed, fake header = 0x%x", header1);
            *pHeader = header1;
            mExtractor->ASFSeekTo(0);
            return OK;
        }
        else if (ASF_SUCCESS != retVal)
        {
            ALOGE ("[ASF_ERROR]ASFSource::findMP3Header no MP3 Header");
            mExtractor->ASFSeekTo(0);
            return ERROR_END_OF_STREAM;
        }

        int length = packet1->range_length(); // buffer data length
        const uint8_t *src =
            (const uint8_t *)packet1->data() + packet1->range_offset();

        int start = 0; //the start byte for finding MP3 header
        //find mp3 header
        while(start+ 3 < length)
        {
            header1 = U32_AT(src + start);
            size_t frame_size;

            if (get_mp3_frame_size(header1, &frame_size, NULL, NULL, NULL))
            {
                ALOGI("possible header %x size %x", header1, frame_size);

                uint8_t tmp[4];
                int j = 0;
                for(; (j + (int)frame_size + start) < length && j < 4; j++)
                {
                    tmp[j] = src[frame_size + start+ j];
                }

                //handle the end of this buffer
                if (j < 4)
                {
                    int left = 4 - j;
                    MediaBuffer *packet2 = NULL;

                    ALOGD ("[ASF_ERROR]ASFSource::findMP3Header End of this packet(= %d th, left= %d), read next.",readPackets, left);
                    if(ASF_SUCCESS == mExtractor->GetNextMediaFrame(&packet2, bIsKeyFrame,ASF_AUDIO,&isSeeking,mTrackIndex))
                    {
                        const uint8_t *src1 =
                            (const uint8_t *)packet2->data() + packet2->range_offset();

                        if(packet2->range_length() < (size_t)left)
                        {
                            ALOGD ("ASF The packet(= %d < left= %d)is too small to check MP3 Header.",packet2->range_length(), left);
                            break;
                        }

                        for( int i = 0 ; i < left && j < 4 ; i++)
                        {
                            tmp[j] = src1[i];
                            ++j;
                        }

                    }
                    else
                    {
                        ALOGD ("[ASF_ERROR]ASFSource::findMP3Header End of stream,fake header = 0x%x", header1);
                        *pHeader = header1;
                        mExtractor->ASFSeekTo(0);
                        return OK;
                    }

                }

                //check mp3 header by header2(the beginning 4 bytes of next frame)
                int header2 = U32_AT(tmp);
                ALOGI("possible header %x size %x, test %x", header1, frame_size, header2);
                if ((header2 & kMP3HeaderMask) == (header1 & kMP3HeaderMask))
                {
                    *pHeader = header1;
                    mExtractor->ASFSeekTo(0);
                    return OK;
                }
            }

            ++start;
        }

        packet1->release();
        packet1 = NULL;
        ++readPackets;
    }

    mExtractor->ASFSeekTo(0);
    return OK;
}


status_t ASFSource::start(MetaData *params)
{
    Mutex::Autolock autoLock(mLock);
    ALOGI ("[ASF]ASFSource::start stream id =%d", mStreamId);

    int32_t val;
    if (params && params->findInt32(kKeyWantsNALFragments, &val)
            && val != 0)
    {
        mWantsNALFragments = true;
        ALOGI("[ASF]ASFSource::start mWantsNALFragments = true");

    }
    else
    {
        mWantsNALFragments = false;
    }


    if(mIsVideo==true)
    {
        mExtractor->mHasVideo= true;
        mExtractor->mHasVideoTrack = true;
        ALOGI ("[ASF]ASFSource::mHasVideo=true");
    }

    return OK;
}

status_t ASFSource::stop()
{
    ALOGI ("[ASF]ASFSource::stop stream id =%d", mStreamId);
    Mutex::Autolock autoLock(mLock);

    if(mIsVideo==true)
    {
        ALOGD("WMV video file, stop video track!");
        mExtractor->mHasVideo= false;
        mExtractor->mHasVideoTrack = false;
        ALOGI ("[ASF]ASFSource::mHasVideo=false");
    }

    if (mBuffer != NULL)
    {
        mBuffer->release();
        mBuffer = NULL;
    }

    return OK;
}

sp<MetaData> ASFSource::getFormat()
{
    //ALOGE ("[ASF]ASFSource::getFormat stream id =%d", mStreamId);
    sp<MetaData> meta;
    meta = mExtractor->mTracks.itemAt(mTrackIndex).mMeta;
    const char* mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    if (!strncasecmp("audio/", mime, 6))
    {
        mIsVideo = false;
    }
    else
    {
        CHECK(!strncasecmp("video/", mime, 6));
        mIsVideo =true;
    }

    return meta;
}

//if only audio file, seek will error
status_t ASFSource::read(MediaBuffer **out, const ReadOptions *options)
{

    android::Mutex::Autolock autoLock(mExtractor->mCacheLock);//avoid read by different instance read at the same time?

    ASFErrorType retVal = ASF_SUCCESS;
    int64_t seekTimeUs,keyTimeUs,seekTimeUsfake;
    ReadOptions::SeekMode mode;
    bool bIsKeyFrame = false;
    bool bAdjustSeekTime = false;
    bool bNeedResetSeekTime = false;
    bool newBuffer = false;
    status_t err;
    bool isSeek = false;

    *out = NULL;
    //1 path : seek happen, enter here
    if (options && options->getSeekTo(&seekTimeUs, &mode)) //seek and find an I
    {
        ALOGI ("ASFSource::read seek to %lld us (mType=%d, mode=%d,mHasVideo=%d)", seekTimeUs, mType, mode,mExtractor->mHasVideo);

        //reset the payload read count info after seek,as we should read a new position , a new frame
        TrackInfo *trackInfo = &(mExtractor->mTracks.editItemAt(mTrackIndex));
        trackInfo->mCurPayloadIdx=0;
        trackInfo->mNextPacket->payload_count=0;

        //just for file-with-video-track, to seek accurately
        /*
        if(seekTimeUs==0) //if seek to 0, not need to speed up
        {
            seekTimeUsfake = seekTimeUs;
        	bAdjustSeekTime = false;
        }
        else  //for speed up for finding the closet frame
        {
            seekTimeUsfake =seekTimeUs + mExtractor->mPrerollTimeUs;
        	bAdjustSeekTime = true;
        }
        */
        seekTimeUsfake = seekTimeUs;
        uint32_t repeat_cnt=0;
        int64_t diff =0;

RESET_SEEK_TIME:
        if(bNeedResetSeekTime)
        {
            if(bAdjustSeekTime)
            {
                seekTimeUsfake = seekTimeUsfake -mExtractor->mPrerollTimeUs; //subtrack preroll time
                bAdjustSeekTime = false;
                ALOGI("ASFSource::read seek : RESET_SEEK_TIME once mPrerollTimeUs!!! to %lld us\n",seekTimeUsfake);
            }
            else
            {

                if(diff>1000000ll)
                {
                    seekTimeUsfake = seekTimeUsfake -(diff/2);
                }
                else
                {
                    seekTimeUsfake = seekTimeUsfake -1000000ll;//subtrack 1S each reset
                }
                ALOGI("ASFSource::read seek : RESET_SEEK_TIME once again!!! to %lld us\n",seekTimeUsfake);
            }

            if(seekTimeUsfake<0)
            {
                seekTimeUsfake = 0;
            }
            bNeedResetSeekTime = false;//reset
        }


        ALOGI ("ASFSource::read seek fake to %lld us\n",seekTimeUsfake);

        //1.1 path : file contains video seek happen, enter here
        if (mExtractor->mHasVideo)  //has video file
        {

            if (mType == ASF_VIDEO)
            {
                //mExtractor->ASFSeekTo(seekTimeUs/1000);
                mExtractor->ASFSeekTo(seekTimeUsfake/1000);//seet to the time, and set postion in the packet for a/v stream
                mSeeking = true;
                bIsKeyFrame = false;


                while (!bIsKeyFrame)
                {

                    retVal = mExtractor->GetNextMediaFrame( out, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
                    if (ASF_END_OF_FILE == retVal)
                    {
                        ALOGI ("[ASF_ERROR]ASFSource::read EOS reached in seek (stream id = %d)", mStreamId);
                        return ERROR_END_OF_STREAM;
                    }
                    else if(ASF_FILE_READ_ERR== retVal) //not a frame data
                    {

                        (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);
                        ALOGI ("[ASF_ERROR]ASF_FILE_READ_ERR:ASF Video Seek Drop ts = %lld ms ", keyTimeUs/1000);
                        (*out)->release();
                        (*out) = NULL;
                        bIsKeyFrame=false;
                        mSeeking=true;
                    }
                    else
                    {
                        (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);
                        diff = keyTimeUs-seekTimeUs;
                        //if(keyTimeUs > seekTimeUs)
                        if( diff > 500000 && repeat_cnt < 10)
                        {
                            (*out)->release();  //seek  to 100ms, is not I ,will seek forward, bot nearest I befroe???
                            (*out) = NULL;
                            bIsKeyFrame=false;
                            mSeeking=true;
                            bNeedResetSeekTime = true;
                            repeat_cnt++;
                            ALOGI ("ASFSource::read seek bNeedResetSeekTime keyTimeUs = %lld ms > seekTimeUs =%lld ms repeat_cnt=%d", keyTimeUs/1000,seekTimeUs/1000,repeat_cnt);
                            goto RESET_SEEK_TIME;
                        }

                        else if(!bIsKeyFrame)
                        {

                            ALOGI ("ASFSource::read seek ASF Video Seek Drop non-I ts = %lld ms", keyTimeUs/1000);
                            (*out)->release();  //seek  to 100ms, is not I ,will seek forward, bot nearest I befroe???
                            (*out) = NULL;
                        }
                        repeat_cnt=0;
                    }
                }
#ifdef SEEK_BY_SEEK_MODE_SET
                if(mode == ReadOptions::SEEK_PREVIOUS_SYNC
                        || mode == ReadOptions::SEEK_NEXT_SYNC
                        || mode ==ReadOptions::SEEK_CLOSEST_SYNC )
                {

                    (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);
                    (*out)->meta_data()->setInt64(kKeyTargetTime, keyTimeUs);
                }
                else if(mode==ReadOptions::SEEK_CLOSEST)//drop to the target time

#endif //#ifdef SEEK_BY_MODE_SET                
                {
                    (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);
                    (*out)->meta_data()->setInt64(kKeyTargetTime, seekTimeUs);
                }
                ALOGI("ASFSource::read seek ASF Video Seek done: KeyTime is =%lld ms ,targetNPT is=%lld ms\n",(keyTimeUs/1000),(seekTimeUs/1000));//debug

                //if video = AVC or MJPEG, which need to be fragmented, copy (*out) to mBuffer
                if ( (mIsAVC && mWantsNALFragments) || mIsMJPEG)
                {
                    //*out = mBuffer;
                    int64_t keyTime=0;
                    int32_t keyIsSyncFrame;
                    if (mBuffer != NULL)
                    {
                        mBuffer->release();
                        mBuffer = NULL;
                    }

                    isSeek = true;

                    if(mIsAVC)
                    {
                        //post-process after find a frame , for NALFragment Seek video
                        ALOGI("ASFSource::read seek AVC Video ReadFramePostProcess()\n");//debug
                        retVal = ReadFramePostProcess(out);
                        if (retVal != ASF_SUCCESS)
                        {
                            return ERROR_END_OF_STREAM;
                        }
                    }

                    if ((*out)!= NULL)
                    {
                        newBuffer = true;

                        // clone *out buffer to mBuffer
                        mBuffer = new MediaBuffer((*out)->size());
                        (*out)->meta_data()->findInt64(kKeyTime, &keyTime);
                        (*out)->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
                        mBuffer->meta_data()->setInt64(kKeyTime, keyTime);
                        mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
                        uint8_t *data = (uint8_t *)mBuffer->data();
                        memcpy(data, (*out)->data(), (*out)->size());
                        CHECK(mBuffer != NULL);
                        mBuffer->set_range((*out)->range_offset(), (*out)->range_length());

                        (*out)->release();
                        (*out) = NULL;
                    }
                    else
                    {
                        ALOGI("[ASFERROR]ASFSource::read seek Video,*out == NULL, AVC/MJPEG get no media frame ");//debug
                    }
                }

                retVal = ASF_SUCCESS;


            }

            if (mType == ASF_AUDIO)
            {
                while (1)
                {
                    retVal = mExtractor->GetNextMediaFrame(out, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
                    if (ASF_END_OF_FILE == retVal)
                    {
                        ALOGI ("[ASF_ERROR]ASFSource::read EOS reached in seek (stream id = %d)", mStreamId);
                        return ERROR_END_OF_STREAM;
                    }
                    (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);

                    if (seekTimeUs - keyTimeUs > 300000)
                    {
                        ALOGI ("ASF Audio Seek Drop audio2 ts = %lld", keyTimeUs/1000);
                        (*out)->release();
                        (*out) = NULL;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                ALOGI("ASF Audio Seek done: KeyTime is =%lld ms\n",keyTimeUs/1000);//debug

                if(mIsMP3)
                {
                    int64_t keyTime=0;
                    int32_t keyIsSyncFrame;
                    isSeek = true;
                    if ((*out)!= NULL)
                    {
                        newBuffer = true;

                        // clone *out buffer to mBuffer
                        mBuffer = new MediaBuffer((*out)->size());
                        (*out)->meta_data()->findInt64(kKeyTime, &keyTime);
                        (*out)->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
                        mBuffer->meta_data()->setInt64(kKeyTime, keyTime);
                        mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
                        uint8_t *data = (uint8_t *)mBuffer->data();
                        memcpy(data, (*out)->data(), (*out)->size());
                        CHECK(mBuffer != NULL);
                        mBuffer->set_range((*out)->range_offset(), (*out)->range_length());

                        (*out)->release();
                        (*out) = NULL;
                    }
                    else
                    {
                        ALOGE("[ASF_ERROR]ASFSource::read seek ASF Video,*out == NULL, MP3 get no media frame ");//debug
                    }

                }
                retVal = ASF_SUCCESS;

            }

        }//has video file
        //1.2 path : file only contains audio seek happen, enter here
        else //pure audio file
        {
            if (mType == ASF_AUDIO)
            {

                ALOGI("ASF pure audio file,seekTimeUsfake =%lld ms\n",seekTimeUsfake/1000);
                mExtractor->ASFSeekTo(seekTimeUsfake/1000);

                while (1)
                {
                    //in audio path,  mSeeking and bIsKeyFrame are not used,just use payload count and index to find the audio frame
                    retVal = mExtractor->GetNextMediaFrame(out, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
                    if (ASF_END_OF_FILE == retVal)
                    {
                        ALOGI ("[ASF_ERROR]ASFSource::read EOS reached in seek (stream id = %d)", mStreamId);
                        return ERROR_END_OF_STREAM;
                    }
                    (*out)->meta_data()->findInt64(kKeyTime, &keyTimeUs);

                    diff = keyTimeUs-seekTimeUs;
                    if( diff > 500000 && repeat_cnt < 50)
                    {
                        (*out)->release();  //seek  to 100ms, is not I ,will seek forward, bot nearest I befroe???
                        (*out) = NULL;
                        bNeedResetSeekTime = true;
                        repeat_cnt++;
                        ALOGI ("ASFSource::Audio read seek bNeedResetSeekTime keyTimeUs = %lld ms > seekTimeUs =%lld ms repeat_cnt=%d", keyTimeUs/1000,seekTimeUs/1000,repeat_cnt);
                        goto RESET_SEEK_TIME;
                    }

                    else if (diff < -500000)  //Ϊʲô???
                    {
                        ALOGI ("ASF Audio Seek Drop audio2 ts = %lld", keyTimeUs/1000);
                        (*out)->release();
                        (*out) = NULL;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                if(mIsMP3)
                {
                    int64_t keyTime=0;
                    int32_t keyIsSyncFrame;
                    isSeek = true;
                    if ((*out)!= NULL)
                    {
                        newBuffer = true;

                        // clone *out buffer to mBuffer
                        mBuffer = new MediaBuffer((*out)->size());
                        (*out)->meta_data()->findInt64(kKeyTime, &keyTime);
                        (*out)->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
                        mBuffer->meta_data()->setInt64(kKeyTime, keyTime);
                        mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
                        uint8_t *data = (uint8_t *)mBuffer->data();
                        memcpy(data, (*out)->data(), (*out)->size());
                        CHECK(mBuffer != NULL);
                        mBuffer->set_range((*out)->range_offset(), (*out)->range_length());

                        (*out)->release();
                        (*out) = NULL;
                    }
                    else
                    {
                        ALOGE("[ASF_ERROR]ASFSource::read seek ASF Video,*out == NULL, MP3 get no media frame ");//debug
                    }

                }

                ALOGI("ASF Audio Seek done: KeyTime is =%lld ms\n",keyTimeUs/1000);//debug

                retVal = ASF_SUCCESS;

            }
        }//pure audio file seek end

    }//seek the 1th frame done

    //2 path : normal play, enter here
    else if ( (!mWantsNALFragments) && (!mIsMP3 )&& (!mIsMJPEG))
    {

        retVal = mExtractor->GetNextMediaFrame(out, bIsKeyFrame,mType,&mSeeking,mTrackIndex);

        if (ASF_END_OF_FILE == retVal)
        {
            ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
            return ERROR_END_OF_STREAM;
        }

    }
    else
    {
        ALOGI ("ASFSource::read() Normal play AVC or MP3 or MJPEG");
    }

    //post-process after find a frame , for non-NALFragment video
    //add for TS correction and in case of 2nd frame is B
    if ((mType == ASF_VIDEO) && (!mWantsNALFragments) && ((!mIsMJPEG)))
    {
        retVal = ReadFramePostProcess(out);
        if (retVal != ASF_SUCCESS)
        {
            return ERROR_END_OF_STREAM;
        }
    }

    //Special Case I: MP3
    //divide the ASF audio frame into MP3 frames
    if (mIsMP3)
    {

        ALOGI ("ASFSource::read()Audio Type = MP3, reassemble mBuffer to 1 MP3 frame");
#if 0
        if (mBuffer == NULL && isSeek)
        {
            ALOGI ("[ASF_ERROR]ASFSource::read AVC Seek Failed(stream id = %d)", mStreamId);
            return ERROR_END_OF_STREAM;
        }
#endif
        if (mBuffer == NULL)
        {
            ALOGI ("ASFSource::read() mBuffer == NULL, acquire buffer");
            newBuffer = true;
        }

        // Case: Normal play MP3, get 1 ASF media frame
        if (newBuffer && (!isSeek))
        {
            ALOGI ("ASFSource::read() newBuffer, get media frame");

            if (mBuffer != NULL)
            {
                mBuffer->release();
                mBuffer = NULL;
            }
            //get 1 ASF frame
            retVal = mExtractor->GetNextMediaFrame(&mBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);


            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                return ERROR_END_OF_STREAM;
            }

        }

        // for ALPS00962015(there's noice when playing MP3), 
        // to avoid a case that a MP3Header was divided in 2 packets
        // ex. 0xFF in n-th packet, and 0xFB 52 00 is in (n+1)-th packet=> you cannot find 0xFFFB5200 of MP3Header 
        if((mBuffer != NULL) && (mBuffer->range_length() < 4))
        {
            ALOGI ("ASFSource::read() mBuffer size(%d) < 4 bytes, appending buffer for detecting MP3 header",mBuffer->range_length());

            MediaBuffer *tmp = new MediaBuffer(mBuffer->range_length());
            //MediaBuffer *tmp = NULL;
            MediaBuffer *nextBuffer = NULL;
            //get 1 ASF frame
            retVal = mExtractor->GetNextMediaFrame(&nextBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                return ERROR_END_OF_STREAM;
            }

            //copy remaining data of mBuffer to tmp
            memcpy(tmp->data(), mBuffer->data()+mBuffer->range_offset(),mBuffer->range_length());

            //empty mBuffer
            if (mBuffer != NULL)
            {
                mBuffer->release();
                mBuffer = NULL;
            }
		
            //reallocate mbuffer
            {
                int64_t keyTime=0;
	            int32_t keyIsSyncFrame;
	            mBuffer = new MediaBuffer(nextBuffer->size()+tmp->size());
	            nextBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
	            nextBuffer->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
	            mBuffer->meta_data()->setInt64(kKeyTime, keyTime);
	            mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
	            uint8_t *data = (uint8_t *)mBuffer->data();
	            memcpy(data, (uint8_t*)tmp->data(), tmp->size());
	            memcpy(data+tmp->size(), (uint8_t*)nextBuffer->data()+ nextBuffer->range_offset(), nextBuffer->range_length());

	            CHECK(mBuffer != NULL);
	            mBuffer->set_range(0, nextBuffer->range_length()+tmp->size());
				
	            ALOGI ("ASFSource::read() new mBuffer size(%d), including original mBuffer size(%d)+ new packet size(%d)",
		        mBuffer->range_length(),tmp->range_length(),nextBuffer->range_length() );
            }

            //release all temporal buffer
            nextBuffer->release();
            nextBuffer = NULL;		

            tmp->release();
            tmp = NULL;	
        }

        int start=0;
        uint32_t header=0;
        ssize_t frameSize;

        // MP3 frame header start with 0xff
        if (mMP3Header >= 0)
        {
            ALOGD("[ASF_Read]mMP3Header=0x%8.8x", mMP3Header);
        }

        int length = mBuffer->range_length(); // buffer data length
        const uint8_t *src =
            (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

        // find the position of MP3 header(every mp3 frame start with MP3 header )
        start = mp3HeaderStartAt(src, length, mMP3Header);

        if (start >= 0)
            header = U32_AT(src + start);

        int bitrate;
        bool ret= false;
        ret = get_mp3_frame_size(header, (size_t*)&frameSize, NULL, NULL, &bitrate);
        ALOGI("[ASF_Read]mp3 start %d header %x frameSize %d length %d bitrate %d",start, header, frameSize, length, bitrate);

        if (start >= 0 && ret)
        {
            //framesize > buffer length, need to get next ASF frame
            if (frameSize + start > length)
            {
                ALOGI("[ASF_Read]MP3 frameSize(%d) + start(%d) > length(%d)",frameSize, start, length);
                MediaBuffer *tmp = new MediaBuffer(frameSize);
                //MediaBuffer *tmp = NULL;
                MediaBuffer *nextBuffer = NULL;
                int needSizeOrg = frameSize + start - length;
                int needSize = needSizeOrg;
                int existSizeOrg = length - start;
                int existSize = existSizeOrg;
                status_t readret =  OK;
                ssize_t bytesRea = 0;
                int remainSize = 0;

                // handle the data which exceeds mBuffer->range_length()
                while(readret == OK)
                {
                    if (nextBuffer != NULL)
                    {
                        nextBuffer->release();
                        nextBuffer = NULL;
                    }

                    //get 1 next ASF media frame
                    readret = mExtractor->GetNextMediaFrame(&nextBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
                    if (ASF_END_OF_FILE == readret)
                    {
                        ALOGI ("[ASF_ERROR]ASFSource::read EOS reached(stream id = %d) when read next MP3 frame", mStreamId);
                        tmp->release();
                        tmp = NULL;
                        return ERROR_END_OF_STREAM;
                    }

                    //copy the needSize from next ASF media frame
                    if ((size_t)needSize >= nextBuffer->range_length())
                    {
                        memcpy((uint8_t*)tmp->data()+ existSize,
                               (uint8_t*)nextBuffer->data() + nextBuffer->range_offset(), nextBuffer->range_length());
                        needSize -= nextBuffer->range_length();
                        existSize += nextBuffer->range_length();
                        ALOGI ("[ASF_Read]next asf frame size(%d) <= need Size(%d)", nextBuffer->range_length(), needSize);
                    }
                    else
                    {
                        memcpy((uint8_t*)tmp->data()+ existSize,
                               (uint8_t*)nextBuffer->data() + nextBuffer->range_offset(), needSize);
                        remainSize = nextBuffer->range_length() - needSize;
                        ALOGI ("[ASF_Read]in next ASF frame, remainSize = %d", remainSize);
                        break;
                    }

                    if( existSize >= frameSize)
                    {
                        ALOGI ("[ASF_Read]have enough frame size (existSize)=%d", existSize);
                        break;
                    }

                }

                if(readret == OK)
                {
                    int64_t keyTime=0;
                    int32_t keyIsSyncFrame;

                    memcpy(tmp->data(),
                           (uint8_t*)mBuffer->data() + mBuffer->range_offset() + start, existSizeOrg);
                    mBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
                    tmp->meta_data()->clear();
                    tmp->meta_data()->setInt64(kKeyTime, keyTime);
                    tmp->set_range(0, frameSize);
                    tmp->meta_data()->setInt32(kKeyIsSyncFrame, 1);
                    *out = tmp;

                    //copy all the data to tmp buffer, clear mBuffer
                    if(mBuffer != NULL)
                    {
                        mBuffer->release();
                        mBuffer = NULL;
                    }

                    // clone the remain "next ASF frame"(*out) to mBuffer,
                    // clone size = nextbuffer->range_length() -needSize
                    if (nextBuffer!= NULL)
                    {
                        mBuffer = new MediaBuffer(remainSize);
                        nextBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
                        nextBuffer->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
                        mBuffer->meta_data()->setInt64(kKeyTime, keyTime);
                        mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
                        uint8_t *data = (uint8_t *)mBuffer->data();
                        memcpy(data, nextBuffer->data()+ needSize, remainSize);
                        CHECK(mBuffer != NULL);
                        mBuffer->set_range(0, remainSize);

                        nextBuffer->release();
                        nextBuffer = NULL;

                        ALOGI ("[ASFSource]ASFSource::read MP3 next buffer remain Size= %d.", remainSize);
                    }
                    else
                    {
                        ALOGI ("[ASFSource]ASFSource::read MP3 next buffer no remain Size.");
                        if (nextBuffer != NULL)
                        {
                            nextBuffer->release();
                            nextBuffer = NULL;
                        }
                    }
#if 0   // for debug
                    if((*out)->data())
                    {
                        char name[255];
                        FILE*	file_;
                        sprintf(name, "/sdcard/ASFMP3AudioRead.bin");
                        file_ = fopen(name, "ab");

                        if(file_)
                        {
                            fwrite((void *)(*out)->data(), 1, (*out)->size(), file_);
                            fclose(file_);
                        }
                    }

                    ALOGI ("ASFSource::read() MP3 MediaBuffer range_offset=%d, range_length = %d",
                           (*out)->range_offset(), (*out)->range_length());
#endif

                    return OK;
                }
                else
                {
                    ALOGE("readNextASFframe return 0x%x", readret);
                    tmp->release();
                    tmp = NULL;
                    nextBuffer->release();
                    nextBuffer = NULL;

                    if(mBuffer != NULL)
                    {
                        mBuffer->release();
                        mBuffer = NULL;
                    }

                    return readret;
                }

                tmp->release();
                nextBuffer->release();

            }
            else
            {
                ALOGI("MP3 frameSize + MP3 header position < buffer length");
            }

        }
        else
        {
            ALOGW("bad MP3 frame without header, all remain bytes %d", length);

            if ( length >= 4)      //to create a header for ASF frame without MP3 header
            {
                char *p = (char*)mBuffer->data() + mBuffer->range_offset();
                char *q = (char*)&mMP3Header;

                p[0] = q[3];
                p[1] = q[2];
                p[2] = q[1];
                p[3] = q[0];

                for (int i = 4; i < 16 && i < length; i++)
                {
                    p[i] = 0;
                }

                frameSize = length; //send all mBuffer to out
                ALOGE("fake MP3 header = 0x%x", mMP3Header);

            }
            else
            {
                frameSize = length;
                ALOGW("[ASF_Error] Read MP3 frame, mBuffer length( = %d) < 4", length);
            }
            start = 0;
        }

        //clone modified data to out
        MediaBuffer *clone = new MediaBuffer(mBuffer->size());
        int64_t keyTime=0;
        int32_t keyIsSyncFrame;
        mBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
        mBuffer->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
        clone->meta_data()->setInt64(kKeyTime, keyTime);
        clone->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
        uint8_t *data = (uint8_t *)clone->data();
        memcpy(data, mBuffer->data(), mBuffer->size());
        CHECK(clone != NULL);
        clone->set_range(mBuffer->range_offset() + start, frameSize);

        CHECK(mBuffer != NULL);
        mBuffer->set_range(
            mBuffer->range_offset() + frameSize + start,
            mBuffer->range_length() - frameSize - start);


        if (mBuffer->range_length() == 0)
        {
            ALOGI ("ASFSource::read() mBuffer->range_length = 0 ");
            mBuffer->release();
            mBuffer = NULL;
        }

        *out = clone;

#if 0   // for debug
        if((*out)->data())
        {
            char name[255];
            FILE*	file_;
            sprintf(name, "/sdcard/ASFMP3AudioRead.bin");
            file_ = fopen(name, "ab");

            if(file_)
            {
                fwrite((void *)(*out)->data(), 1, (*out)->size(), file_);
                fclose(file_);
            }
        }

        ALOGI ("ASFSource::read() MP3 MediaBuffer range_offset=%d, range_length = %d",
               (*out)->range_offset(), (*out)->range_length());
#endif
        return OK;

    }
    //Special CaseII : AVC
    //divide ASF media frame into NAL fragments
    if (mType == ASF_VIDEO && mIsAVC && mWantsNALFragments)
    {
        bool bIsKeyFrame = false;

        ALOGI ("ASFSource::read()Video Type = AVC, reassemble buffer to 1 NAL unit");
        if (mBuffer == NULL && isSeek)
        {
            ALOGI ("[ASF_ERROR]ASFSource::read AVC Seek Failed(stream id = %d)", mStreamId);
            return ERROR_END_OF_STREAM;
        }

        if (mBuffer == NULL)
        {
            ALOGI ("ASFSource::read() mBuffer == NULL, acquire buffer");
            newBuffer = true;
        }

        // Case: Normal play AVC, need get 1 frame
        if (newBuffer && (!isSeek))
        {
            ALOGI ("ASFSource::read() newBuffer, get media frame");

            if (mBuffer != NULL)
            {
                mBuffer->release();
                mBuffer = NULL;
            }

            retVal = mExtractor->GetNextMediaFrame(&mBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);

            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                return ERROR_END_OF_STREAM;
            }
            else
            {
                //post-process after find a frame , for NALFragment AVC video
                retVal = ReadFramePostProcess(&mBuffer);

                if (retVal != ASF_SUCCESS)
                {
                    return ERROR_END_OF_STREAM;
                }

            }

        }

#if 1
        // Each NAL unit is split up into its constituent fragments and
        // each one of them returned in its own buffer.
        int length, start;

        if (newBuffer)
        {

            start = nalStartAt((uint8_t*)mBuffer->data(), mBuffer->size(), &length);
            ALOGI ("ASFSource::read() check newBuffer NAL Start position = %d", start);
            if (start == -1)
            {
                // not a byte-stream
                *out = mBuffer;
                mBuffer->release();
                mBuffer = NULL;
                return OK;
            }

            mBuffer->set_range(
                mBuffer->range_offset() + length,
                mBuffer->range_length() - length);
        }

        const uint8_t *src =
            (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

        start = nalStartAt(src, mBuffer->range_length(), &length);
        if (start == -1)
        {
            ALOGI ("ASFSource::read() check NAL Start position = %d", start);
            start = mBuffer->range_length();
            length = 0;
        }

        //clone modified data to out
        MediaBuffer *clone = new MediaBuffer(mBuffer->size());
        int64_t keyTime=0;
        int32_t keyIsSyncFrame;
        mBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
        mBuffer->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);
        clone->meta_data()->setInt64(kKeyTime, keyTime);
        clone->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
        uint8_t *data = (uint8_t *)clone->data();
        memcpy(data, mBuffer->data(), mBuffer->size());
        CHECK(clone != NULL);
        clone->set_range(mBuffer->range_offset(), start);

        CHECK(mBuffer != NULL);
        mBuffer->set_range(
            mBuffer->range_offset() + start + length,
            mBuffer->range_length() - start - length);

        if (mBuffer->range_length() == 0)
        {
            ALOGI ("ASFSource::read() mBuffer->range_length = 0 ");
            mBuffer->release();
            mBuffer = NULL;
        }

        *out = clone;

#if 0   // for debug
        if((*out)->data())
        {
            char name[255];
            FILE*	file_;
            sprintf(name, "/sdcard/videoRead.bin");
            file_ = fopen(name, "ab");

            if(file_)
            {
                fwrite((void *)(*out)->data(), 1, (*out)->size(), file_);
                fclose(file_);
            }
        }

        ALOGI ("ASFSource::read() AVC MediaBuffer range_offset=%d, range_length = %d",
               (*out)->range_offset(), (*out)->range_length());
#endif

        return OK;

#endif

    }
    // Reassemble JPEG frame, SOI(0xffd8) - Data - EOI(0xffd9)
    if(mIsMJPEG)
    {
        bool bIsKeyFrame = false;

        //ALOGI ("ASFSource::read()Video Type = MJPEG, reassemble buffer to 1 JPEG frame (SOI-data-EOI)");
        if (mBuffer == NULL && isSeek)
        {
            ALOGI ("[ASF_ERROR]ASFSource::read MJPEG Seek Failed(stream id = %d)", mStreamId);
            return ERROR_END_OF_STREAM;
        }

        if (mBuffer == NULL)
        {
            ALOGI ("ASFSource::read() mBuffer == NULL, acquire buffer");
            newBuffer = true;
        }

        // Case: Normal play MJPEG, need get 1 frame
        if (newBuffer && (!isSeek))
        {
            //ALOGI ("ASFSource::read() newBuffer, get media frame");

            if (mBuffer != NULL)
            {
                mBuffer->release();
                mBuffer = NULL;
            }

            retVal = mExtractor->GetNextMediaFrame(&mBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);

            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                return ERROR_END_OF_STREAM;
            }

        }

        int end = -1;
        int start = -1;
        const uint8_t *src =
            (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

        // 1st buffer must start with 0xffd8
        start = mjpegStartAt(src, mBuffer->range_length());
        ALOGI ("ASFSource::read() check newBuffer SOI position = %d", start);
        // no start code(0xffd8), release this media frame and get another one
        while (start == -1)
        {
            if (mBuffer != NULL)
            {
                mBuffer->release();
                mBuffer = NULL;
            }

            ALOGI ("ASFSource::read() find no SOI, get another media frame");
            retVal = mExtractor->GetNextMediaFrame(&mBuffer, bIsKeyFrame,mType,&mSeeking,mTrackIndex);

            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                return ERROR_END_OF_STREAM;
            }

            src = (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

            start = mjpegStartAt( src, mBuffer->range_length());
            ALOGI ("ASFSource::read() check newBuffer SOI position = %d", start);

        }

        int mjpeg_frame_size = 0;

        //find the 1st buffer's metadata for out buffer
        int64_t keyTime=0;
        int32_t keyIsSyncFrame;
        mBuffer->meta_data()->findInt64(kKeyTime, &keyTime);
        mBuffer->meta_data()->findInt32(kKeyIsSyncFrame, &keyIsSyncFrame);

        end = mjpegEndAt(src + start, mBuffer->range_length()-start);
        ALOGI ("ASFSource::read() check newBuffer EOI position = %d", start + end);
        if (end != -1)   //Here, SOI and EOI are in the same frame
        {
            mjpeg_frame_size += (end + 1);
        }
        else //find EOI(0xffd9) from subsequent ASF media frames
        {
            //pBuffer = old mBuffer(with SOI) + next ASF frame +...+ next ASF frame with EOI
            uint8_t *pBuffer = new uint8_t[2*MAX_VIDEO_INPUT_SIZE];
            MediaBuffer *tmpBuffer = NULL;
            int pBufferSize = 0;
            int tmpBufferSize = 0; //store the last tmpBufferSize

            mjpeg_frame_size += (mBuffer->range_length() - start);//EOI in next frame
            memcpy(pBuffer, src, mBuffer->range_length()); //copy old mBuffer to pBuffer
            pBufferSize += mBuffer->range_length();

            mBuffer->release();
            mBuffer = NULL;

            //if there's EOI in next frame, find it and copy the whole frame to pBuffer
            while (end == -1)
            {
                ALOGI ("ASFSource::read() get next buffer to find EOI(0xffd9)");
                retVal = mExtractor->GetNextMediaFrame(&tmpBuffer, bIsKeyFrame,ASF_VIDEO,&mSeeking,mTrackIndex);

                if (ASF_END_OF_FILE == retVal)
                {
                    ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 1(stream id = %d)", mStreamId);
                    return ERROR_END_OF_STREAM;
                }

                end = mjpegEndAt((const uint8_t *)tmpBuffer->data() + tmpBuffer->range_offset(), tmpBuffer->range_length());
                ALOGI ("ASFSource::read() check newBuffer EOI position = %d", end);

                //copy ASF media frame to pBuffer
                uint8_t *data = (uint8_t *)tmpBuffer->data() + tmpBuffer->range_offset();
                memcpy(pBuffer + pBufferSize, data, tmpBuffer->range_length());

                pBufferSize += tmpBuffer->range_length();
                mjpeg_frame_size += tmpBuffer->range_length();
                tmpBufferSize = tmpBuffer->range_length();

                tmpBuffer->release();
                tmpBuffer = NULL;
            }

            mjpeg_frame_size = mjpeg_frame_size - tmpBufferSize + end;
            //copy pBuffer(Data-SOI-Data-EOI-RemainFrame) to mBuffer
            uint8_t *data = (uint8_t *)mBuffer->data();
            memcpy(data, pBuffer, pBufferSize); // now the mBuffer=Data-SOI-Data-EOI-RemainFrame
            mBuffer->set_range(0, pBufferSize);

            delete [] pBuffer;
        }

        //clone modified data to out
        MediaBuffer *clone = new MediaBuffer(mjpeg_frame_size);
        clone->meta_data()->setInt64(kKeyTime, keyTime);
        clone->meta_data()->setInt32(kKeyIsSyncFrame, keyIsSyncFrame);
        uint8_t *data = (uint8_t *)clone->data();
        memcpy(data, mBuffer->data()+mBuffer->range_offset()+ start, mjpeg_frame_size);
        CHECK(clone != NULL);
        clone->set_range(0, mjpeg_frame_size);

        CHECK(mBuffer != NULL);
        mBuffer->set_range(
            mBuffer->range_offset() + start + mjpeg_frame_size,
            mBuffer->range_length() - start - mjpeg_frame_size);

        if (mBuffer->range_length() == 0)
        {
            //ALOGI ("ASFSource::read() mBuffer->range_length = 0 ");
            mBuffer->release();
            mBuffer = NULL;
        }

        *out = clone;

#if 0   // for debug
        if((*out)->data())
        {
            char name[255];
            FILE*	file_;
            sprintf(name, "/sdcard/MJPGvideoRead.bin");
            file_ = fopen(name, "ab");

            if(file_)
            {
                fwrite((void *)(*out)->data(), 1, (*out)->size(), file_);
                fclose(file_);
            }
        }
#endif

        ALOGI ("ASFSource::read() MJPEG MediaBuffer range_offset=%d, range_length = %d",
               (*out)->range_offset(), (*out)->range_length());


        return OK;

    }

    return OK;//if retrun OK, all paths will come here
}

//post-process after find a frame ,only for video
//add for TS correction and in case of 2nd frame is B
ASFErrorType ASFSource::ReadFramePostProcess(MediaBuffer **out)
{
    //check video frame type
    uint32_t pic_type;
    bool bIsKeyFrame = false;
    bool checkVal = true;
    ASFErrorType retVal = ASF_SUCCESS;

    while(1)
    {
        if(mIsAVC)
        {
            //check AVC video frame type
            pic_type = mExtractor->ParserAVCFrameType((uint8_t *)(*out)->data(), (*out)->range_length());
            ALOGI("ASFSource::read ASF Video ParserAVCFrameType() = %d\n", pic_type);//debug
        }
        else if (mIsMPEG4)
        {
            //check MPEG4 video frame type
            pic_type = mExtractor->ParserMPEG4FrameType((uint8_t *)(*out)->data(), (*out)->range_length());
            ALOGI("ASFSource::read ASF Video ParserMPEG4FrameType() = %d\n", pic_type);//debug
        }
        else
        {
            pic_type = mExtractor->ParserVC1FrameType((uint8_t *)((*out)->data()),mTrackIndex);
        }

        checkVal = mExtractor->CheckMediaFrame(out,pic_type);

        if(checkVal)
        {
            break;
        }
        else
        {
            retVal = mExtractor->GetNextMediaFrame(out, bIsKeyFrame,mType,&mSeeking,mTrackIndex);
            if (ASF_END_OF_FILE == retVal)
            {
                ALOGI ("[ASF_ERROR]ASFSource::read EOS reached 2,streamId=%d\n",mStreamId);
                return retVal;
            }
        }
    }

    return retVal;

}


////////////////////////////////////////////////////////////////////////////////

static void hexdump(const void *_data, size_t size)
{
    const uint8_t *data = (const uint8_t *)_data;
    size_t offset = 0;
    while (offset < size)
    {
        printf("0x%04x  ", offset);

        size_t n = size - offset;
        if (n > 16)
        {
            n = 16;
        }

        for (size_t i = 0; i < 16; ++i)
        {
            if (i == 8)
            {
                printf(" ");
            }

            if (offset + i < size)
            {
                printf("%02x ", data[offset + i]);
            }
            else
            {
                printf("   ");
            }
        }

        printf(" ");

        for (size_t i = 0; i < n; ++i)
        {
            if (isprint(data[offset + i]))
            {
                printf("%c", data[offset + i]);
            }
            else
            {
                printf(".");
            }
        }

        printf("\n");

        offset += 16;
    }
}


int32_t asf_io_read_func(void *pAsfExtractor, void *aBuffer, int32_t aSize)
{
    ASFExtractor* _pExtractor = (ASFExtractor*)pAsfExtractor;
    if (_pExtractor)
    {
        int32_t bytesRead = _pExtractor->mDataSource->readAt(_pExtractor->mAsfParserReadOffset, aBuffer, aSize);
        _pExtractor->mAsfParserReadOffset += bytesRead;
        //ALOGE("asf_io_read_func:bytesRead=%d\n",bytesRead);
        return bytesRead;
    }
    ALOGW("asf_io_read_func:retrun 0\n");
    return 0;
}

int32_t asf_io_write_func(void *pAsfExtractor, void *aBuffer, int32_t aSize)
{
    return 0;
}

int64_t asf_io_seek_func(void *pAsfExtractor, int64_t aOffset)
{
    // only supports SEEK_SET
    ASFExtractor* _pExtractor = (ASFExtractor*)pAsfExtractor;
    if (_pExtractor)
    {
        _pExtractor->mAsfParserReadOffset = aOffset;
        return aOffset;
    }

    return 0;
}


uint32_t  vc1_util_show_bits(uint8_t * data, uint32_t  bitcnt, uint32_t  num)
{
    uint32_t  tmp, out, tmp1;

    tmp = (bitcnt & 0x7) + num;

    if (tmp <= 8)
    {
        out = (data[bitcnt >> 3] >> (8 - tmp)) & ((1 << num) - 1);
    }
    else
    {
        out = data[bitcnt >> 3]&((1 << (8 - (bitcnt & 0x7))) - 1);

        tmp -= 8;
        bitcnt += (8 - (bitcnt & 0x7));

        while (tmp > 8)
        {
            out = (out << 8) + data[bitcnt >> 3];

            tmp -= 8;
            bitcnt += 8;
        }

        tmp1 = (data[bitcnt >> 3] >> (8 - tmp)) & ((1 << tmp) - 1);
        out = (out << tmp) + tmp1;
    }

    return out;
}

uint32_t  vc1_util_get_bits(uint8_t * data, uint32_t  * bitcnt, uint32_t  num)
{
    uint32_t  ret;

    ret = vc1_util_show_bits(data, *bitcnt, num);
    (*bitcnt) += num;

    return ret;
}

uint32_t  vc1_util_show_word(uint8_t * a)
{
    return ((a[0] << 24) + (a[1] << 16) + (a[2] << 8) + a[3]);
}



// output one complete frame
ASFErrorType ASFExtractor::GetNextMediaFrame(MediaBuffer** out, bool& bIsKeyFrame,Type strmType,bool *isSeeking ,uint32_t CurTrackIndex)
{
    //ALOGE ("ASFSource::GetNextMediaFrame() isSeeking=%d",*isSeeking);

    ASFErrorType retVal = ASF_SUCCESS;
    // uint32_t i=0;

    uint32_t max_buffer_size = 0;
    if (strmType == ASF_VIDEO)
    {
        max_buffer_size = MAX_VIDEO_INPUT_SIZE;//may be can be optimize by the real size
    }
    else if (strmType == ASF_AUDIO)
    {
        max_buffer_size = MAX_AUDIO_INPUT_SIZE;
    }
    else
    {
        ALOGE ("[ASF_ERROR]Undefined ASFSource type!!!");
        return ASF_ERROR_UNKNOWN;
    }


    uint32_t sample_size = max_buffer_size;
    uint32_t timestamp = 0;
    uint32_t timestamp_dummy = 0;
    uint32_t replicatedata_size = 0;
    uint32_t current_frame_size = 0;
    uint8_t *pBuffer = new uint8_t[max_buffer_size];
    bool bIsKeyFrame_dummy = false;
    int read_new_obj = 0; 


    retVal = GetNextMediaPayload(pBuffer+current_frame_size, sample_size, timestamp, replicatedata_size, bIsKeyFrame,CurTrackIndex);

    if (ASF_SUCCESS != retVal)
    {
        ALOGE ("[ASF_ERROR]GetNextMediaFrame failed A\n");
        delete [] pBuffer;
        return ASF_END_OF_FILE;
    }
    else
    {
        current_frame_size += sample_size;
    }

    // ALOGE("GetNextMediaFrame:0 isSeeking=%d,bIsKeyFrame=%d,,current_frame_size=%d,replicatedata_size=%d\n",(*isSeeking),bIsKeyFrame,current_frame_size,replicatedata_size);


// Morris Yang 20110208 skip non-key frame payload [
//in a seek ponint , the first payload belonged to a media object maybe not the start payload of this meida object
//in this scenario, we will got a incompleted frame.So will drop
//a [KF] payload must be hte start payload of this meida object
    while ((*isSeeking) && (false == bIsKeyFrame))
    {

        current_frame_size = 0;
        sample_size = max_buffer_size;
        ASF_DEBUG_LOGV ("[ASF_ERROR]GetNextMediaFrame :drop a payload: timestamp=%d\n",timestamp);

        retVal = GetNextMediaPayload(pBuffer+current_frame_size, sample_size, timestamp, replicatedata_size, bIsKeyFrame,CurTrackIndex);
        if (ASF_SUCCESS != retVal)
        {
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed B\n");
            delete [] pBuffer;
            return ASF_END_OF_FILE;
        }

        else
        {
            current_frame_size += sample_size;//meaningless,as before GetNextMediaPayload , current_frame_size is reset to 0
        }
    }
    //(*isSeeking) = false;
    ASF_DEBUG_LOGV("GetNextMediaFrame:1 isSeeking=%d,bIsKeyFrame=%d,,current_frame_size=%d,replicatedata_size=%d\n",(*isSeeking),bIsKeyFrame,current_frame_size,replicatedata_size);

// ]

RESET_PAYLOAD:

    //this while-loop will retrive all payloads belonged to one frame in cuurent stream ID

    //if a frame contains more than one payload, will enter this while-loop
    //we check this by current_frame_size(current red payload total size) < replicatedata_size(whole frame size)
    while (current_frame_size < replicatedata_size)//replicatedata_size is a frame size
    {
        sample_size = max_buffer_size;
        //timestamp_dummy is no use ,as the payload in a frame should same
        //sample_size will be update each payload read, is the patload size
        retVal = GetNextMediaPayload(pBuffer+current_frame_size, sample_size, timestamp_dummy, replicatedata_size, bIsKeyFrame_dummy,CurTrackIndex);
        if (ASF_SUCCESS != retVal)
        {
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed D\n");
            delete [] pBuffer;
            return ASF_END_OF_FILE;
        }

        //Seek case:
        //read success,BUT not in the same media object
        //skip previous payload and get a new media object which started with present payload
        else if((*isSeeking) && (timestamp_dummy != timestamp))
        {
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed C, cur payload is not belong to the same frame with previous one\n");
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed C, skip previous payload\n");

            //1st media objest after seek must be key frame, OTHERWISE, skip this payload
            if(bIsKeyFrame_dummy && (read_new_obj < 3))
            {
                //memmove(pBuffer, pBuffer+current_frame_size, sample_size);
                //memset(pBuffer+sample_size, 0, max_buffer_size-sample_size);
                uint8_t *tmp = new uint8_t[sample_size];
                memcpy(tmp, pBuffer+(uint32_t)current_frame_size, sample_size);
                delete [] pBuffer;
                pBuffer = new uint8_t[max_buffer_size];
                memcpy(pBuffer, tmp, sample_size);

                current_frame_size = sample_size;
                timestamp = timestamp_dummy;
                timestamp_dummy = 0;
                delete [] tmp;
                read_new_obj++;

                goto RESET_PAYLOAD; //to get a new frame
            }
            else
            {
                retVal = ASF_FILE_READ_ERR;//not read a right object
                break;
            }
        }
        //-->add b qian start
        // in case of the 1th I payload in the seek point is not the 1 playlod of the I frame
        //BUT,if this case happen, the Frame type will check error, we will drop the 1th-not-I frame
        else if(timestamp_dummy!=timestamp)//read success,BUT not a frame
        {
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed C, cur payload is not belong to the same frame with previous one\n");
            ALOGE ("[ASF_ERROR]GetNextMediaFrame failed C: timestamp_dummy=%d,timestamp=%d\n",timestamp_dummy,timestamp);
            retVal = ASF_FILE_READ_ERR;//not read a right frame
            break;
        }
        //<--add by qian end
        else  //read success,and belong to a frame
        {
            current_frame_size += sample_size;
        }
    }
    // ALOGE("GetNextMediaFrame:2 isSeeking=%d,bIsKeyFrame=%d,,current_frame_size=%d,replicatedata_size=%d\n",(*isSeeking),bIsKeyFrame,current_frame_size,replicatedata_size);
    MediaBuffer *buffer = new MediaBuffer(current_frame_size);//when release this buffer???
    buffer->meta_data()->setInt64(kKeyTime, timestamp*1000LL);
    buffer->meta_data()->setInt32(kKeyIsSyncFrame, bIsKeyFrame);
    uint8_t *data = (uint8_t *)buffer->data();
    memcpy(data, pBuffer, current_frame_size);
    buffer->set_range(0, current_frame_size);
    (*isSeeking) = false;
    ALOGI ("[ASF_Read]GetNextMediaFrame current_frame_size = %d\n",current_frame_size);
    *out = buffer;
#if 0
    if(pBuffer && strmType == ASF_AUDIO)
    {
        char name[260];
        FILE*	file_;
        sprintf(name, "/sdcard/audio.bin");
        file_ = fopen(name, "ab");
        uint32_t type;
        const void *data;
        size_t size;
        fwrite(pBuffer, current_frame_size, 1, file_);
        fclose(file_);
    }
#endif

#if 0
    if(pBuffer && strmType == ASF_VIDEO)
    {
        char name[260];
        FILE*	file_;
        sprintf(name, "/sdcard/video.bin");
        file_ = fopen(name, "ab");
        uint32_t type;
        const void *data;
        size_t size;
        fwrite(pBuffe,r current_frame_size, 1, file_);
        fclose(file_);
    }
#endif

    // debug//
    //if (strmType == ASF_AUDIO)//ASF_VIDEO)
    {
        ASF_DEBUG_LOGV ("GetNextMediaFrame StreamId[%d], timestamp=%d, IsKey=%d,replicatedata_size=%d,(*out)->range_length()=%d\n", mTracks.editItemAt(CurTrackIndex).mTrackNum, timestamp, bIsKeyFrame,replicatedata_size,(*out)->range_length());
        ASF_DEBUG_LOGV (" mCurPayloadIdx=%d,payload_count=%d\n",mTracks.editItemAt(CurTrackIndex).mCurPayloadIdx,mTracks.editItemAt(CurTrackIndex).mNextPacket->payload_count);
    }

    delete [] pBuffer;

    return retVal;
}
//GetNextMediaPayload(pBuffer+current_frame_size, sample_size, timestamp, replicatedata_size, bIsKeyFrame);
// ust retrive one payload in the current stream in the pNextPacket
ASFErrorType ASFExtractor::GetNextMediaPayload(uint8_t* aBuffer, uint32_t& arSize, uint32_t& arTimeStamp, uint32_t& arRepDataSize, bool& bIsKeyFrame,uint32_t CurTrackIndex)
{
    ASFErrorType retVal = ASF_SUCCESS;
    asf_packet_t * pNextPacket= NULL;
    uint32_t *CurPayloadIdx = NULL;
    bool next_payload_found = false;
    bool payload_retrieved = false;

    asf_stream_t * pStreamProp = NULL;
    asf_stream_type_t stream_type = ASF_STREAM_TYPE_UNKNOWN;
    pStreamProp = mAsfParser->asf_get_stream(mTracks.editItemAt(CurTrackIndex).mTrackNum);//mStreamId = mExtractor->mTracks.itemAt(index).mTrackNum;
    stream_type = pStreamProp->type;

    pNextPacket = mTracks.editItemAt(CurTrackIndex).mNextPacket;
    CurPayloadIdx = &(mTracks.editItemAt(CurTrackIndex).mCurPayloadIdx);

    if (mAsfParser != NULL)
    {

        //the do-while just retrive one payload in the needed stream in the pNextPacket, then out
        do
        {
            ASF_DEBUG_LOGE("[ASF]:GetNextMediaPayload:CurTrackIndex=%d, enter do-while:*CurPayloadIdx=%d,pNextPacket->payload_count=%d\n",CurTrackIndex,*CurPayloadIdx,pNextPacket->payload_count);
            if (!pNextPacket->payload_count)	 // retrieve next packet in case there is no payload left in current packet
            {

                int ret = mAsfParser->asf_get_stream_packet(pNextPacket, mTracks.editItemAt(CurTrackIndex).mTrackNum);
                ASF_DEBUG_LOGV("-----[ASF]:GetNextMediaPayload 1: CurTrackIndex=%d,find a new packet, contain payloads =%d ----\n",CurTrackIndex,pNextPacket->payload_count);

                if ( ret <= 0 )  //should >0 else is EOS, no data in file
                {
                    asf_stream_t *pStreamProp = mAsfParser->asf_get_stream(mTracks.editItemAt(CurTrackIndex).mTrackNum);

                    if(pStreamProp->flags & ASF_STREAM_FLAG_EXTENDED)//has Extended Stream Properties Object
                    {
                        //Avg. time for frame is 100-nanosec - arTimestamp unit is millisec.
                        arTimeStamp = pStreamProp->extended->avg_time_per_frame / 10000;
                    }
                    else
                    {
                        ALOGE("[ASF_ERROR]GetNextMediaPayload:no extended field. dummy value inserted\n");
                        arTimeStamp = 0;
                    }

                    ALOGE("[ASF_ERROR]GetNextMediaPayload: return ASF_END_OF_FILE A,streamID=%d,arTimeStamp=%d\n",mTracks.editItemAt(CurTrackIndex).mTrackNum,arTimeStamp);
                    return ASF_END_OF_FILE;
                }

                //updated Current Payload index
                *CurPayloadIdx = 0;// first payload in the newly found packet
            }
            //retrive all payloads in the newly found packet

            //mStreamId is  the track number(stream_number) when create the asf track source
            //payloads in a packet may be belong to different streams
            //if this payload in current packet is no belong to current stream, goto next payload
            asf_payload_t * ptrPayload = &pNextPacket->payloads[*CurPayloadIdx];
            if (NULL == ptrPayload){
                retVal = ASF_END_OF_FILE;
                break;
            }
            if (mTracks.editItemAt(CurTrackIndex).mTrackNum == (uint32_t)ptrPayload->stream_number )
            {
                if (!next_payload_found)// is retrieving current payload
                {
                    ASF_DEBUG_LOGV("[ASF]:GetNextMediaPayload 2:CurTrackIndex=%d,next_payload_found =%d \n",CurTrackIndex,next_payload_found);
                    if (ptrPayload->datalen <= arSize)//arSize is input sample size, the buffer size
                    {
                        if (stream_type == ASF_STREAM_TYPE_VIDEO)
                        {
                            ASF_DEBUG_LOGV("%s Video Payload replen %d datalen %d ",__FUNCTION__,ptrPayload->replicated_length,ptrPayload->datalen);
                            if(ptrPayload->replicated_length)
                            {
                                // the repliated data (1th 4byte is the frame size, 2th 4byte is frame TS)
                                arRepDataSize = ASFByteIO::asf_byteio_getDWLE(ptrPayload->replicated_data);
                                //ALOGV("rep size %d\n",arRepDataSize);
                            }
                            else
                            {
                                arRepDataSize  = ptrPayload->datalen;//???frame size ==this payload size ??
                            }
                        }
                        //update input parameters
                        arSize = ptrPayload->datalen;//update the input param,real payload size
                        memcpy(aBuffer, ptrPayload->data, ptrPayload->datalen);//copy this payload to the input buffer
                        arTimeStamp = ptrPayload->pts;
                        bIsKeyFrame = ptrPayload->key_frame;
                        ASF_DEBUG_LOGV("ASF]:GetNextMediaPayload 2 CurTrackIndex=%d size = %d, obj = %d, ts = %d, rep_len = %d, IsKey=%d\n", CurTrackIndex, arSize, ptrPayload->media_object_number, ptrPayload->pts, arRepDataSize, ptrPayload->key_frame);
                        //Change the Current index and decrease payload
                        (*CurPayloadIdx)++;
                        pNextPacket->payload_count--;
                        ASF_DEBUG_LOGV("ASF]:GetNextMediaPayload 2,CurTrackIndex=%d, payload_count = %d, (*CurPayloadIdx)=%d\n", CurTrackIndex, pNextPacket->payload_count,(*CurPayloadIdx));

                        next_payload_found = true;//if set true, next do-while will not enter this path,error??
                        payload_retrieved = true;
                    }
                    else
                    {
                        retVal = ASF_ERR_NO_MEMORY;
                        ALOGE("[ASF_ERROR]GetNextMediaPayload return ASF_ERR_NO_MEMORY A\n");
                        break;
                    }
                }
            }
            else
            {
                //Change the Current index and decrease payload
                (*CurPayloadIdx)++;
                pNextPacket->payload_count--;
                ASF_DEBUG_LOGV("[ASF]:GetNextMediaPayload 4:current payload is not belong to the stream CurTrackIndex=%d \n",CurTrackIndex);
            }
        }
        while(!payload_retrieved);
    }
    else
    {
        ASF_DEBUG_LOGV("%s Error OUT\n",__FUNCTION__);;
        retVal = ASF_END_OF_FILE;
        ALOGE("[ASF_ERROR]GetNextMediaPayload return ASF_END_OF_FILE B,streamID=%d\n",mTracks.editItemAt(CurTrackIndex).mTrackNum);
    }

    return retVal;
}





void ASFExtractor::ResetMediaFrameFlags()
{

    mFisrtFrameTsUs=0;
    mFisrtFrameParsed=false;
    mSecondFrameParsed=false;
    mParsedFrameNum=0;

}
//1.ASF P,B frame TS are both error (decode order time)
//2.event a frame is dropped, the frame count should be plus 1,else the TS will error

bool ASFExtractor::CorrectMediaFrameTS(MediaBuffer** pCurFrmMediaBuffer, uint32_t CurPicType)
{
    // Now TS correction is doing in WMV_OMX
    return true;

}
//if this frame is check passed, will return ok, else if drop,will return false
bool ASFExtractor::CheckMediaFrame(MediaBuffer** pCurFrmMediaBuffer, uint32_t CurPicType)
{
    //drop or not this frame,the frame count should be added 1
    // or else the TS base on counter will error
    mParsedFrameNum++;
    if(mFisrtFrameParsed && mSecondFrameParsed )
    {
        ASF_DEBUG_LOGV("ASF CheckMediaFrame 1\n" );
        CorrectMediaFrameTS(pCurFrmMediaBuffer,CurPicType);//correct frame TS
    }
    else
    {
        if(mFisrtFrameParsed == false)//start play or after seek
        {
            if(CurPicType == IVOP || CurPicType == BIVOP)
            {
                mFisrtFrameParsed=true;
                (*pCurFrmMediaBuffer)->meta_data()->findInt64(kKeyTime, &mFisrtFrameTsUs);
                ALOGI("ASF CheckMediaFrame:1th frame after restart,mFisrtFrameTsUs=%lld\n",mFisrtFrameTsUs);
                CorrectMediaFrameTS(pCurFrmMediaBuffer,CurPicType); // correct frame TS//may be this 1th I frame is not need correct
            }
            else
            {
                //drop frames until 1th I frame comes ;
                ALOGE("[ASF_ERROR]CurFrmMediaBuffer is released 1, as 1th frame is not I, is %d\n",CurPicType);
                (*pCurFrmMediaBuffer)->release();
                return false;
            }
        }
        else
        {
            if(mSecondFrameParsed == false)
            {
                if(CurPicType !=BVOP)//2nd frame should not be B
                {
                    mSecondFrameParsed =true;
                    ASF_DEBUG_LOGV("ASF CheckMediaFrame 2\n" );
                    CorrectMediaFrameTS(pCurFrmMediaBuffer,CurPicType);//correct frame TS
                }
                else
                {
                    ALOGE("[ASF_ERROR]CurFrmMediaBuffer is released 2, as 2th frame is B\n");
                    (*pCurFrmMediaBuffer)->release();
                    return false;
                }
            }
            else//mSecondFrameParsed==true, most of scenarios
            {
                ASF_DEBUG_LOGV("ASF CheckMediaFrame 3\n" );
                CorrectMediaFrameTS(pCurFrmMediaBuffer,CurPicType); //correct frame TS//will
            }
        }
    }

    return true;//means this frame is verified and will be decoded

}



ASFExtractor::ASFExtractor(const sp<DataSource> &source)
    : mFileMeta(new MetaData),
      mDbgFlags(0),
      bIgnoreAudio(0),
      bIgnoreVideo(0),
      mDurationMs(0),
      mSeekable(false),
      bThumbnailMode(false),
      mExtractedThumbnails(false),
      mParsedFrameNum(0),
      mFisrtFrameTsUs(0),
      mFisrtFrameParsed(false),
      mSecondFrameParsed(false),
      mPrerollTimeUs(0),
      mDataSource(source),
      mAsfParser(NULL),
      mAsfParserReadOffset(0),
      mIsValidAsfFile(false),
      mIsAsfParsed(false),
      mHasVideo(false),
      mHasVideoTrack(false),
      mFileSize(0)     
{

    mDataSource->getSize((off64_t*)&mFileSize);
    ALOGI("ASFExtractor: mFileSize=%lld",mFileSize);

    ALOGI ("+ASFExtractor 0x%x, tid=%d\n", (unsigned int)this, gettid());
    mAsfParser = new ASFParser((void*)this, asf_io_read_func, asf_io_write_func, asf_io_seek_func);

    if (!mAsfParser)
    {
        ALOGE ("[ASF_ERROR]Error: ASFParser creation failed\n");
    }

    int retVal = ASF_ERROR_UNKNOWN;
    retVal = mAsfParser->IsAsfFile();//parse the file here
    if (ASF_SUCCESS == retVal)
    {
        ALOGE ("This is an ASF file!!!\n");
        mIsValidAsfFile = true;
    }
    else
    {
        ALOGI ("[ASF_ERROR]Not an ASF file!!!\n");
        mIsValidAsfFile = false;
    }

    char value[PROPERTY_VALUE_MAX];
    property_get("asfff.showts", value, "0");
    bool _res = atoi(value);
    if (_res) mDbgFlags |= ASFFF_SHOW_TIMESTAMP;

    property_get("asfff.ignoreaudio", value, "0");
    _res = atoi(value);
    if (_res) mDbgFlags |= ASFFF_IGNORE_AUDIO_TRACK;

    property_get("asfff.ignorevideo", value, "0");
    _res = atoi(value);
    if (_res) mDbgFlags |= ASFFF_IGNORE_VIDEO_TRACK;


#if 0
    // retrieve meta data (optional)
    asf_metadata_t * pASFMetadata = mAsfParser->asf_header_get_metadata();
    if (!pASFMetadata)
    {
        ALOGE ("ASFExtractor pAsfParser->asf_header_get_metadata failed\n");
    }
    else
    {
        ALOGI ("--- Meta data ---\n");
        ALOGI ("Title: %s\n", pASFMetadata->title);
        ALOGI ("Author: %s\n", pASFMetadata->artist);
        ALOGI ("Description: %s\n", pASFMetadata->description);
        ALOGI ("Copyright: %s\n", pASFMetadata->copyright);
        ALOGI ("Rating: %s\n", pASFMetadata->rating);
        ALOGI ("-----------------\n");
    }
    mAsfParser->asf_metadata_destroy(pASFMetadata);
#endif

    ResetMediaFrameFlags();
    //mFileMetaData = new MetaData;
    mFileMeta->setInt32(kKeyVideoPreCheck, 1);
    ALOGI ("-ASFExtractor 0x%x, tid=%d\n", (unsigned int)this, gettid());
}

ASFExtractor::~ASFExtractor()
{
    ALOGI ("~ASFExtractor 0x%x, tid=%d", (unsigned int)this, gettid());
    if (mAsfParser)
        delete mAsfParser;
}

sp<MetaData> ASFExtractor::getMetaData()
{
    ALOGI ("[ASF]ASFExtractor::getMetaData()");
    if (false == mIsAsfParsed)
    {
        if(!ParseASF())
            return NULL;
    }
    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_ASF);

    if (countTracks() > 0)
    {
        if (mHasVideo)
        {
            const char* mime=NULL;
            for(int i=0; (size_t)i<mTracks.size(); i++)
            {
                CHECK((mTracks.itemAt(i).mMeta)->findCString(kKeyMIMEType,&mime) );
                if(!strncasecmp(MEDIA_MIMETYPE_VIDEO_WMV, mime, sizeof(MEDIA_MIMETYPE_VIDEO_WMV)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV);
                    break;
                }
                if(!strncasecmp(MEDIA_MIMETYPE_VIDEO_MPEG4, mime, sizeof(MEDIA_MIMETYPE_VIDEO_MPEG4)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);
                    break;
                }
                if(!strncasecmp(MEDIA_MIMETYPE_VIDEO_AVC, mime, sizeof(MEDIA_MIMETYPE_VIDEO_AVC)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
                    break;
                }
                if(!strncasecmp(MEDIA_MIMETYPE_VIDEO_MJPEG, mime, sizeof(MEDIA_MIMETYPE_VIDEO_MJPEG)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MJPEG);
                    break;
                }

            }

        }
        else
        {
            const char* mime=NULL;
            for(int i=0; i<(size_t)mTracks.size(); i++)
            {
                CHECK((mTracks.itemAt(i).mMeta)->findCString(kKeyMIMEType,&mime) );
                if(!strncasecmp(MEDIA_MIMETYPE_AUDIO_WMA, mime, sizeof(MEDIA_MIMETYPE_AUDIO_WMA)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMA);
                    break;
                }
            #ifdef MTK_SWIP_WMAPRO
				if(!strncasecmp(MEDIA_MIMETYPE_AUDIO_WMAPRO, mime, sizeof(MEDIA_MIMETYPE_AUDIO_WMAPRO)))
				{
					 mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMAPRO);
					 break;
				}
            #endif
                if(!strncasecmp(MEDIA_MIMETYPE_AUDIO_RAW, mime, sizeof(MEDIA_MIMETYPE_AUDIO_RAW)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RAW);
                    break;
                }
                if(!strncasecmp(MEDIA_MIMETYPE_AUDIO_MPEG, mime, sizeof(MEDIA_MIMETYPE_AUDIO_MPEG)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG);
                    break;
                }
                if (!strncasecmp(MEDIA_MIMETYPE_AUDIO_AAC, mime, sizeof(MEDIA_MIMETYPE_AUDIO_AAC)))
                {
                    mFileMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
                    break;
                }


            }



        }
    }

    if(!mHasVideo && mHasVideoTrack)
    {
        mFileMeta->setInt32(kKeyHasUnsupportVideo, true);
        ALOGD("ASF has unsupport video track");
    }

    return mFileMeta;
}

size_t ASFExtractor::countTracks()
{
    ALOGI("[ASF]ASFExtractor::countTracks:mIsAsfParsed=%d \n",mIsAsfParsed );
    if (false == mIsAsfParsed)  //normal first parsed here
    {
        if(!ParseASF())
            return 0;
    }
    ALOGI ("ASFExtractor::countTracks return %d", mTracks.size());
    return mTracks.size();;
}

sp<MetaData> ASFExtractor::getTrackMetaData(size_t index, uint32_t flags)
{
    ALOGI ("[ASF]ASFExtractor::getTrackMetaData:mIsAsfParsed=%d,index=%d\n",mIsAsfParsed,index);
    if (index >= mTracks.size())
    {
        return NULL;
    }

    if ((flags & kIncludeExtensiveMetaData) && (false == mExtractedThumbnails))
    {
        findThumbnail();
        mExtractedThumbnails = true;
    }
    return mTracks.itemAt(index).mMeta;
}


uint32_t ASFExtractor::flags() const
{
    ALOGI("[ASF]flags: mSeekable=%d\n",mSeekable);
    if(mSeekable)
    {
        return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
    }
    else
    {
        ALOGW("[ASF]flags: can not seek,just can pasue\n");
        return CAN_PAUSE;
    }
}


void ASFExtractor::findThumbnail()
{
    ALOGI ("+[ASF]ASFExtractor::findThumbnail mSeekable=%d",mSeekable);
    // TODO:
    uint32_t idx=0;
    uint32_t currIdx=0;

    MediaBuffer *FrameSize[ASF_THUMBNAIL_SCAN_SIZE];
    MediaBuffer *out=NULL;
    ASFErrorType retVal = ASF_SUCCESS;
    bThumbnailMode = true;


    if(mSeekable)
    {
        for (size_t j = 0; j < ASF_THUMBNAIL_SCAN_SIZE; j++)
        {
            FrameSize[j]=NULL;
        }

        for (size_t i = 0; i < mTracks.size(); ++i)
        {
            TrackInfo *info = &mTracks.editItemAt(i);
            const char *mime;
            CHECK(info->mMeta->findCString(kKeyMIMEType, &mime));
            currIdx=i;
            if (strncasecmp(mime, "video/", 6))
            {
                continue;
            }

            for(idx=0; idx<ASF_THUMBNAIL_SCAN_SIZE; idx++)
            {
                bool bIsKeyFrame = false;
                bool isSeeking =false;
                bool bEOS=false;

                while (!bIsKeyFrame)
                {
                    retVal = GetNextMediaFrame(&out, bIsKeyFrame,ASF_VIDEO,&isSeeking,currIdx);//not seeking
                    if (ASF_END_OF_FILE == retVal)
                    {
                        ALOGE ("[ASF_ERROR]findThumbnail EOS (stream id = %d)", info->mTrackNum);
                        bEOS=true;
                        break;//may be a file has only <10 I frames
                    }
                    else if (!bIsKeyFrame)
                    {
                        (out)->release();  //seek  to 100ms, is not I ,will seek forward, bot nearest I befroe???
                    }
                    else
                    {
                        FrameSize[idx]=out;
                        ASF_DEBUG_LOGE ("[ASF]findThumbnail find FrameSize[%d] =0x%08x (stream id = %d)", idx,out ,info->mTrackNum);
                    }
                }
                if(bEOS)
                {
                    ALOGE ("[ASF_ERROR]findThumbnail EOS (stream id = %d,idx=%d)", info->mTrackNum,idx);
                    break;
                }

            }


            uint32_t _max_frame_len = 0;
            int64_t _thumbnail_frame_ts = 0;
            uint32_t _cur_frame_len =0;
            int64_t _cur_timeUs;

            for (size_t j = 0; j < ASF_THUMBNAIL_SCAN_SIZE; j++)
            {

                if (FrameSize[j]!=NULL)
                {
                    _cur_frame_len =FrameSize[j]->range_length();
                    CHECK(FrameSize[j]->meta_data()->findInt64(kKeyTime, &_cur_timeUs));
                    //should add the preroll time
                    //as the parser give the TS is the Frame real ts,not presention ts
                    //presention ts = Frame real ts + preroll ts
                    //play when seek, it give the presention ts to seek command

                    ASF_DEBUG_LOGE("[ASF]findThumbnail: _cur_frame_len = %d,_cur_timeUs=%.2f s\n",_cur_frame_len,(_cur_timeUs)/1E6);//(_cur_timeUs+mPrerollTimeUs)/1E6);

                    if (_cur_frame_len >= _max_frame_len)
                    {
                        _max_frame_len =_cur_frame_len;
                        _thumbnail_frame_ts = _cur_timeUs;
                    }
                }

            }

            _thumbnail_frame_ts=_thumbnail_frame_ts;//+mPrerollTimeUs;

            info->mMeta->setInt64(kKeyThumbnailTime, _thumbnail_frame_ts);
            ALOGI("[ASF]findThumbnail: final time is =%.2f s,size=%d\n",_thumbnail_frame_ts/1E6,_max_frame_len);
            for (size_t j = 0; j < ASF_THUMBNAIL_SCAN_SIZE; j++)
            {
                if(FrameSize[j]!=NULL)
                {
                    ASF_DEBUG_LOGE ("ASFExtractor::findThumbnail rlease FrameSize[%d] =0x%08x (stream id = %d)", j,(uint32_t)FrameSize[j] ,info->mTrackNum);
                    FrameSize[j]->release();
                    FrameSize[j]=NULL;//avoid Wild pointer, set null after release
                }
                FrameSize[j]=NULL;
            }
        }

        ASFSeekTo(0);


        for (size_t i = 0; i < mTracks.size(); ++i)
        {
            TrackInfo *trackInfo = &mTracks.editItemAt(i);

            if (trackInfo->mNextPacket)
            {
                mAsfParser->asf_packet_destroy(trackInfo->mNextPacket);
                trackInfo->mNextPacket = NULL;
                ASF_DEBUG_LOGE ("[ASF]ASFSource::ASFSource Tracks index =%d, asf_packet_destroy\n", i);

            }
        }/**///no need to  destroy here

    }//if(mAsfParser->asf_check_simple_index_obj()) has seek index
    else
    {
        for (size_t i = 0; i < mTracks.size(); ++i)
        {
            TrackInfo *info = &mTracks.editItemAt(i);
            const char *mime;
            CHECK(info->mMeta->findCString(kKeyMIMEType, &mime));
            currIdx=i;
            if (strncasecmp(mime, "video/", 6))
            {
                continue;
            }
            info->mMeta->setInt64(kKeyThumbnailTime, 0);//just get the first frame to show
            ALOGI("kKeyThumbnailTime=0");
        }
    }

    bThumbnailMode = false;/**/
    //ALOGE ("-ASFExtractor::findThumbnail");
}

// Given a time in seconds since Jan 1 1904, produce a human-readable string.
static void convertTimeToDate(int64_t time_1904, String8 *s)
{
    time_t time_1970 = time_1904 - (((66 * 365 + 17) * 24) * 3600);

    char tmp[32];
    strftime(tmp, sizeof(tmp), "%Y%m%dT%H%M%S.000Z", gmtime(&time_1970));

    s->setTo(tmp);
}



sp<MediaSource> ASFExtractor::getTrack(size_t index)
{
    ALOGI("getTrack:indx=%d,mTracks.size()=%d",index,mTracks.size());
    if (index >= mTracks.size())
    {
        return NULL;
    }

    return new ASFSource(this, index);
}


bool ASFExtractor::ParserVC1CodecPrivateData(uint8_t*input , uint32_t inputlen, VC1SeqData* pSeqData)
{
    uint32_t bitcnt = 0;
    uint32_t reserved ;
    reserved=(uint32_t)(*input);
    // Read Sequence Header for SP/MP (STRUCT_C)
    ALOGI ("-----[ASF]ParserVC1CodecPrivateData -----\n");
    ALOGI ("-----[ASF]CodecPrivateData is 0x%08x-----\n",reserved);
    pSeqData->profile = vc1_util_get_bits(input, &bitcnt, 2);//profile
    if(pSeqData->profile == 3)//WVC1: advanced
    {
        ALOGE("[VC-1 Playback capability Error] capability not support as :VC-1 advanced profile, not support, failed\n");
        //ALOGE("[ASF_ERROR]VC-1 advanced profile, not support, failed\n");
        return false;
    }
    reserved=vc1_util_get_bits(input, &bitcnt, 2);
    reserved = vc1_util_get_bits(input, &bitcnt, 3);//frmrtq_postproc
    reserved = vc1_util_get_bits(input, &bitcnt, 5);//bitrtq_postproc
    reserved = vc1_util_get_bits(input, &bitcnt, 1);//loopfilter
    reserved = vc1_util_get_bits(input, &bitcnt, 1);
    if(reserved!=0)
    {
        ALOGE("[ASF_ERROR]VC-1 , error in BITMAPINFOHEADER, reserved bit should be 0,failed 1\n");
        //ALOGE("[VC-1 Playback capability Error] capability not support as :VC-1 advanced profile, not support, failed\n");
        return false;
    }


    pSeqData->multires = vc1_util_get_bits(input, &bitcnt, 1);//multires
    reserved = vc1_util_get_bits(input, &bitcnt, 1);
    if(reserved!=1)
    {
        ALOGE("[ASF_ERROR]VC-1  ,error in BITMAPINFOHEADER, reserved bit should be 1, failed 2\n");
        //return false;
    }

    reserved = vc1_util_get_bits(input, &bitcnt, 1);//fastuvmc
    reserved = vc1_util_get_bits(input, &bitcnt, 1);//extended_mv
    reserved = vc1_util_get_bits(input, &bitcnt, 2);//dquant
    reserved = vc1_util_get_bits(input, &bitcnt, 1);//vstransform
    reserved = vc1_util_get_bits(input, &bitcnt, 1);
    if(reserved!=0)
    {
        ALOGE("[ASF_ERROR]VC-1  ,error in BITMAPINFOHEADER, reserved bit should be 0, failed 3\n");
        return false;
    }

    reserved = vc1_util_get_bits(input, &bitcnt, 1);//overlap
    reserved = vc1_util_get_bits(input, &bitcnt, 1);//syncmarker
    pSeqData->rangered = vc1_util_get_bits(input, &bitcnt, 1);
    pSeqData->maxbframes = vc1_util_get_bits(input, &bitcnt, 3);
    reserved = vc1_util_get_bits(input, &bitcnt, 2);//quantizer
    pSeqData->finterpflag = vc1_util_get_bits(input, &bitcnt, 1);
    reserved = vc1_util_get_bits(input, &bitcnt, 1);
    if(reserved!=1)
    {
        ALOGE("[ASF_ERROR]VC-1  , error in BITMAPINFOHEADER, reserved bit should be 1, failed 4\n");
        return false;
    }
    //parser done

    ALOGI ("SeqData->profile = %d\n", pSeqData->profile);
    ALOGI ("SeqData->rangered = %d\n", pSeqData->rangered);
    ALOGI ("SeqData->maxbframes = %d\n", pSeqData->maxbframes);
    ALOGI ("SeqData->finterpflag = %d\n", pSeqData->finterpflag);
    ALOGI ("SeqData->multires = %d\n", pSeqData->multires);   // :multiresoltuin
    if(pSeqData->multires)
    {

    }
    ALOGI ("SeqData->ms_time_per_frame = %lld us\n", pSeqData->us_time_per_frame );
    ALOGI ("SeqData->framerate = %0.2f\n", (float)(pSeqData->fps100/100) );

    return true;
}
//return the frame type:I/P/B/BI
uint32_t ASFExtractor::ParserVC1FrameType(uint8_t*bitstream,uint32_t CurTrackIndex)
{
    uint32_t bitcnt = 0;
    TrackInfo *trackInfo = &(mTracks.editItemAt(CurTrackIndex));
    VC1SeqData* pCurVC1SeqData=(VC1SeqData*)(trackInfo->mCodecSpecificData);

    ASF_DEBUG_LOGV ("-----[ASF]ParserVC1FrameType -----\n");

    if(pCurVC1SeqData->finterpflag == 1)
    {
        mVC1CurPicData.interpfrm = vc1_util_get_bits(bitstream, &bitcnt, 1);
    }
    mVC1CurPicData.frmcnt = vc1_util_get_bits(bitstream, &bitcnt, 2);


    if(pCurVC1SeqData->rangered == 1)
    {
        mVC1CurPicData.rangeredfrm = vc1_util_get_bits(bitstream, &bitcnt, 1);
    }

    mVC1CurPicData.ptype = vc1_util_get_bits(bitstream, &bitcnt, 1);

    if((pCurVC1SeqData->maxbframes > 0) && (mVC1CurPicData.ptype==0))
        mVC1CurPicData.ptype = vc1_util_get_bits(bitstream, &bitcnt, 1) ? IVOP : BVOP;   // Morris: 0 => I, 1 => P, 2 => B, 3=> BI



    // BFRACTION P.60 (Table40, P.90)
    if (mVC1CurPicData.ptype == BVOP)
    {
        uint32_t iShort = 0, iLong = 0;
        iShort = vc1_util_get_bits(bitstream, &bitcnt, 3);
        if (iShort == 0x7)
        {
            iLong = vc1_util_get_bits(bitstream, &bitcnt, 4);
            if (iLong == 0xe)        // "hole" in VLC (SMPTE reserved)
            {
                ALOGE ("[ASF_ERROR]In VLC hole");
            }

            if (iLong == 0xF)
            {
                mVC1CurPicData.ptype = BIVOP;
            }
        }
    }

    ASF_DEBUG_LOGV ("mVC1CurPicData.interpfrm = %d\n", mVC1CurPicData.interpfrm);
    ASF_DEBUG_LOGV ("mVC1CurPicData.rangeredfrm = %d\n", mVC1CurPicData.rangeredfrm);
    ASF_DEBUG_LOGV ("mVC1CurPicData.ptype = %d\n", mVC1CurPicData.ptype);
    ASF_DEBUG_LOGV ("mVC1CurPicData.frmcnt = %d\n", mVC1CurPicData.frmcnt);

    switch (mVC1CurPicData.ptype)
    {
    case IVOP :
        ASF_DEBUG_LOGE("(pic_num=%lld)[IVOP] ",  mParsedFrameNum );
        break;
    case PVOP :
        ASF_DEBUG_LOGE("(pic_num=%lld)[PVOP] ",  mParsedFrameNum );
        break;
    case BVOP :
        ASF_DEBUG_LOGE("(pic_num=%lld)[BVOP] ",  mParsedFrameNum );
        break;
    case BIVOP:
        ASF_DEBUG_LOGE("(pic_num=%lld)[BIVOP]",  mParsedFrameNum );
        break;
    default   :
        ALOGE ("[ASF_ERROR]: Unrecognized picture type\n");
    }
    return mVC1CurPicData.ptype;
}

//return the frame type:I/P/B
uint32_t ASFExtractor::ParserAVCFrameType(uint8_t* data, int size)
{
    if (size < 6)
        return SKIPPED;

    int offsetStart = 0;
    int prefixLen = 0;
    uint8_t *ptr = data;
    uint8_t *end = ptr + size;
    unsigned char byte = 0;

    while (ptr < end)
    {
        offsetStart = nalStartAt(ptr, end - ptr, &prefixLen);
        ptr += offsetStart + prefixLen;
        if (offsetStart == -1 || ptr >= end)
        {
            return SKIPPED;
        }

        int i_nal_type = ptr[0] & 0x1f;
        if (i_nal_type == 5)
            return IVOP;

        if (i_nal_type >= 1 /*NAL_SLICE*/ && i_nal_type < 5 /*NAL_SLICE_IDR*/)
        {
            byte = ptr[1];
            break;
        }
    }

    // 0 => 1
    // 1~2 => 01 0~1
    // 3~6 => 001 00~11
    // 7~14 => 0001 000~111
    // assume first mb = 0 => 1
    // 1/6 => b frame: 1:1+010+xxxx, 6:1+00111+xx
    if ((byte & 0x80) != 0x80)
    {
        ALOGW("TODO support non zero first mb");
        return SKIPPED;
    }

    if ((byte & 0x40) || ((byte >> 2) == 0x26))
        return PVOP;
    if (byte == 0x88 || ((byte >> 4) == 0xb))
        return IVOP;
    if ((byte >> 4 == 0xa) || (byte >> 2 == 0x27))
        return BVOP;
    return SKIPPED;


    return SKIPPED;
}

uint32_t ASFExtractor::ParserMPEG4FrameType(uint8_t* data, int size)
{
    // here is mpeg4
    int off = seqStartAt((uint8_t*)data, size);
    size -= off;
    if (off < 0 || size < 5)
        return SKIPPED;

    if (data[3] == 0xb3)
    {
        data += 4;
        size -= 4;
        off = seqStartAt((uint8_t*)data, size);
        size -= off;
        if (off < 0 || size < 5)
            return SKIPPED;
    }

    char byte = data[4 + off] & 0xc0;
    if (byte == 0x00)
        return IVOP;
    else if (byte == 0x40)
        return PVOP;
    else if (byte == 0x80)
        return BVOP;
    else
        return SKIPPED;
}

//Stream Properties Object
bool ASFExtractor::RetrieveWmvCodecSpecificData(asf_stream_t * pStreamProp, const sp<MetaData>& meta,VC1SeqData* pCodecSpecificData, uint32_t currIdx)
{
    bool ret=true;
    char _four_cc[5];
	uint32_t retFourCC = 0;
    // if(pStreamProp->flags & ASF_STREAM_FLAG_AVAILABLE)
    //{

    asf_bitmapinfoheader_t *bmih = (asf_bitmapinfoheader_t *)pStreamProp->properties;
    ALOGE("RetrieveWmvCodecSpecificData(): streamId = %d\n",currIdx);
    MakeFourCCString(bmih->biCompression, _four_cc);
    ALOGE("Video format 1: %s\n", _four_cc);
	for (int i=0;_four_cc[i];i++){
		_four_cc[i] = toupper(_four_cc[i]);
	}
	ALOGE("Video format 2: %s\n", _four_cc);
	retFourCC =MakeStringToIntFourCC(_four_cc);
	if (retFourCC==0){
		ALOGE("Video format is null..");
	}	
	MakeFourCCString(retFourCC, _four_cc);
	ALOGE("Video format 3: %s\n", _four_cc);

	switch (retFourCC)
    {
    case FOURCC_WMV3://only support VC-1 simple and main profile
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV);
        break;  // simple and main profile VC1
    }

    case FOURCC_WMVA:
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV);
        break;
    }
    case FOURCC_WVC1:
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV);
        break;
    }
    case FOURCC_WMV2://MP4SMP42,MP43 are not supported
    {
        ALOGE("[VC-1 Playback capability Error] capability not support as :Unsupported video format: %s", _four_cc);
        //ALOGE ("[ASF_ERROR]Unsupported video format: %s", _four_cc);
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_ASF);
        ret = false;
        break;
        //return false;
    }

    //MPEG4
    case FOURCC_MP4S:
    case FOURCC_XVID:
    case FOURCC_DIVX:
    case FOURCC_DX50:
    case FOURCC_MP4V:
	case FOURCC_M4S2:
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);
        break;
    }

    // H264, from http://wiki.multimedia.cx/index.php?title=H264
    case FOURCC_AVC1:
    case FOURCC_DAVC:
    case FOURCC_X264:
    case FOURCC_H264:
    case FOURCC_VSSH:
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
        break;
    }
    //MJPEG
    case FOURCC_MJPG:
    {
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MJPEG);
        break;
    }
    default:
    {
        ALOGE("[VC-1 Playback capability Error] capability not support as :Unknown video format\n");
        meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_ASF);
        ret=false;
        break;
    }
    }

    ALOGI ("-----[ASF]RetrieveWmvCodecSpecificData -----\n");
    ALOGI ("bmih->biWidth = %d\n", bmih->biWidth);
    ALOGI ("bmih->biHeight = %d\n", bmih->biHeight);
    ALOGI ("extra data size = %d\n", bmih->biSize - ASF_BITMAPINFOHEADER_SIZE);
    /*

    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfo;
    VDEC_DRV_QUERY_VIDEO_FORMAT_T outinfo;
    memset(&qinfo, 0, sizeof(qinfo));
    memset(&outinfo, 0, sizeof(outinfo));
    qinfo.u4Width = bmih->biWidth;
    qinfo.u4Height = bmih->biHeight;
    qinfo.u4VideoFormat = VDEC_DRV_VIDEO_FORMAT_VC1;

    VDEC_DRV_MRESULT_T retval = eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_VIDEO_FORMAT, &qinfo, &outinfo);
    ALOGI("[ASF DRV capability info] ret =%d ,MaxWidth=%d, MaxHeight=%d ,profile=%d,level=%d",  retval,outinfo.u4Width , outinfo.u4Height,outinfo.u4Profile,outinfo.u4Level);

    if (VDEC_DRV_MRESULT_FAIL == retval || qinfo.u4Width > outinfo.u4Width ||qinfo.u4Height > outinfo.u4Height || qinfo.u4Width<32 || qinfo.u4Height<32) {
    	//ALOGE ("[ASF_ERROR]resolution is not supported\n");
    	ALOGE("[VC-1 Playback capability Error] capability not support \n");
    	ALOGE ("[ASF_ERROR]resolution is W=%d, H=%d\n",bmih->biWidth,bmih->biHeight);
    	ret=false;
    	return false;
    }

    //  if(bmih->biWidth > 1280 || bmih->biWidth < 32|| bmih->biHeight > 720 || bmih->biHeight < 32)
    //   {
        //ALOGE ("[ASF_ERROR]resolution is not supported\n");
    //        ALOGE("[VC-1 Playback capability Error] capability not support as :resolution is not supported\n");
    //        ALOGE ("[ASF_ERROR]resolution is W=%d, H=%d\n",bmih->biWidth,bmih->biHeight);
    //        ret=false;
    //	        return false;
    //    }

    */
    int32_t mb_y_limit=(bmih->biHeight>>4)+(((bmih->biHeight&0xf)==0)?0:1);
    int32_t mb_x_limit=(bmih->biWidth>>4)+(((bmih->biWidth&0xf)==0)?0:1);
    ALOGI ("mb_y_limit = %d\n", mb_y_limit);
    ALOGI ("mb_x_limit = %d\n", mb_x_limit);

    meta->setInt32(kKeyWidth, bmih->biWidth);
    meta->setInt32(kKeyHeight, bmih->biHeight);
    meta->setInt32(kKeyMaxInputSize, MAX_VIDEO_INPUT_SIZE);  // TODO: modify to a suitable value


    //framerate info
    //Extended Stream Properties Object (optional, 1 per media stream)
    double framerate=0.0;
    if(pStreamProp->flags & ASF_STREAM_FLAG_EXTENDED)
    {
        uint32_t max_buffer_size_video = pStreamProp->extended->max_obj_size;
        if(max_buffer_size_video <= MAX_VIDEO_INPUT_SIZE)//else,means this value is error???
        {
            meta->setInt32(kKeyMaxInputSize, max_buffer_size_video);
        }

        ALOGI("kKeyMaxInputSize=%d,MAX_VIDEO_INPUT_SIZE=%d \n",max_buffer_size_video,MAX_VIDEO_INPUT_SIZE);
        uint64_t  avg_time_per_frame = pStreamProp->extended->avg_time_per_frame;//100-nanosecond units,
        pCodecSpecificData->us_time_per_frame = avg_time_per_frame/10;//ms
        framerate = (double)(1000000.0/(double)(pCodecSpecificData->us_time_per_frame)) ;
    }
    //we will peep the 2nd frame TS, the calculate the framerate
    else
    {

    }
    pCodecSpecificData->fps100=(uint32_t)(framerate*100.0);

    ASF_DEBUG_LOGV ("fps100 = %d\n", pCodecSpecificData->fps100);
    //set codec decoder infor, add the fps in the last position


    uint32_t _config_size_bmpinfo_hdr = ASF_BITMAPINFOHEADER_SIZE;
    uint32_t _config_size_extra_data = (bmih->biSize - ASF_BITMAPINFOHEADER_SIZE);
    uint32_t _keyWMVC_size =_config_size_bmpinfo_hdr + _config_size_extra_data + sizeof(uint32_t);

    uint8_t* _config = new uint8_t[_keyWMVC_size];

    ALOGI("kKeyWMVC size is =%d\n",_keyWMVC_size);
    ALOGI("_config_size_extra_data size is =%d\n",_config_size_extra_data);

    memcpy(_config, bmih, ASF_BITMAPINFOHEADER_SIZE);
    memcpy(_config + ASF_BITMAPINFOHEADER_SIZE, bmih->data, _config_size_extra_data);
    memcpy(_config + ASF_BITMAPINFOHEADER_SIZE + _config_size_extra_data, &(pCodecSpecificData->fps100),sizeof(uint32_t));

    ASF_DEBUG_LOGV("fps100=%d\n", *(uint32_t*)(_config + ASF_BITMAPINFOHEADER_SIZE + _config_size_extra_data));


    //extra data is codec specific data, sequence header
    const char *mime_present;
    meta->findCString(kKeyMIMEType, &mime_present);
    if(!strcasecmp(mime_present,MEDIA_MIMETYPE_VIDEO_WMV))
    {
        ALOGI("mimeType = VIDEO_WMV, enter ParserVC1CodecPrivateData()\n");
        //send bitmap header + seq header + frame rate*100
        meta->setData(kKeyWMVC, 0, _config, _keyWMVC_size);
        //send (seq header + frame rate*100) only
        //meta->setData(kKeyWMVC, 0, bmih->data, (_config_size_extra_data+sizeof(unit32_t)));
        if(!ParserVC1CodecPrivateData((_config + ASF_BITMAPINFOHEADER_SIZE),_config_size_extra_data,pCodecSpecificData))
        {
            ret=false;
        }
    }
    else if(!strcasecmp(mime_present,MEDIA_MIMETYPE_VIDEO_MPEG4))
    {
        ALOGI("mimeType = VIDEO_MPEG4\n");

        if(_config_size_extra_data!=0){
			sp<ABuffer> csd = new ABuffer(_config_size_extra_data);
			memcpy(csd->data(), bmih->data, _config_size_extra_data);	
			hexdump(csd->data(), csd->size());		
			sp<ABuffer> esds = MakeESDS(csd);
			meta->setData(kKeyESDS, kTypeESDS, esds->data(), esds->size());
        }else{
            status_t ret = OK;
			ret = addMPEG4CodecSpecificData(meta);
			if( OK != ret ){
				 ALOGI("Can not find MPEG4 codec specific data,error = %d\n",ret);
			}
		}

    }
    // no codec specific data, construct a new one from bitstream
    else if(!strcasecmp(mime_present,MEDIA_MIMETYPE_VIDEO_AVC))
    {
        //MediaBuffer *out = NULL;
        ASFErrorType retVal = ASF_SUCCESS;
        bool bIsKeyFrame = false;
        bool isSeeking = false;
        int AVCPos = 0;
        int64_t readAVC = 0;

        int size = 0;

        size = MAX_VIDEO_INPUT_SIZE;
        sp<ABuffer> buffer = new ABuffer(size);

        ALOGI("mimeType = VIDEO_AVC, MakeAVCCodecSpecificData\n");

#if 1
        int prefixLen = -1;

        // get data position(Bytes)
        if (_config_size_extra_data == 0 )
        {
            readAVC = mAsfParser->asf_get_data_position();
        }
        else
        {
            readAVC = 108; //header object size(30)+stream properties object size
        }

        while(prefixLen == -1)
        {
            //read data packet from DataSource
            ssize_t n = mDataSource->readAt(readAVC, buffer->data(), size);
            ALOGI("prefixLen = -1, read %d byte\n", n);

            if ( n <= 0)
            {
                ALOGI ("[ASF_ERROR]read EOS reached, can not find AVC codec specific data ");
                delete [] _config;
                return 0;
            }

            readAVC+=n;
            uint8_t *ptr = (uint8_t*)buffer->data();
            uint8_t *end = ptr + size;
            int prefixLen,offsetStart = 0;

            // Retrieve data between 00 00 01 67 and 00 00 01 65

            AVCPos = realAVCStart(buffer->data(), size);

            while (ptr < end)
            {
                // find the start point of NAL prefix (00 00 01 or 00 00 00 01)
                offsetStart = nalStartAt(ptr, end - ptr, &prefixLen);
                ptr += offsetStart + prefixLen;
                if (offsetStart == -1 || ptr >= end)
                {
                    bIgnoreVideo = true;
                    return false;
                }

                if (*ptr == 0x65)
                {
                    //offsetStart = ptr - (uint8_t*)buffer->data() - prefixLen;
                    if (ptr > ((uint8_t*)buffer->data()+ AVCPos))
                    {
                        offsetStart = ptr - ((uint8_t*)buffer->data()+ AVCPos) - prefixLen;
                    }
                    else
                    {
                        ALOGI("[ASFDEBUG] PPS(00 00 01 65) is in front of SPS(00 00 00 67)");
                        offsetStart = ptr - (uint8_t*)buffer->data() - prefixLen;
                    }
                    break;
                }

            }


            if (offsetStart == -1)
            {
                ALOGE("found no seq start"); // can not find NAL prefix
            }
            else if (offsetStart > 0)
            {
                ALOGI("AVCPos = %d, ptr = %u, offsetStart = %d, prefixLen = %d\n", AVCPos, ptr, offsetStart, prefixLen);

                buffer->setRange(AVCPos, offsetStart);
                sp<MetaData> metaAVC = MakeAVCCodecSpecificData(buffer);

                if (metaAVC == NULL)
                {
                    ALOGE("Unable to extract AVC codec specific data");
                    return ERROR_MALFORMED;
                }

                int32_t width, height;
                CHECK(metaAVC->findInt32(kKeyWidth, &width));
                CHECK(metaAVC->findInt32(kKeyHeight, &height));

                uint32_t type;
                const void *csd;
                size_t csdSize;
                CHECK(metaAVC->findData(kKeyAVCC, &type, &csd, &csdSize));

                meta->setInt32(kKeyWidth, width);
                meta->setInt32(kKeyHeight, height);
                meta->setData(kKeyAVCC, type, csd, csdSize);
#if 0
                if(csd)
                {
                    char name[255];
                    FILE*	file_;
                    sprintf(name, "/sdcard/videoReadSeqData.bin");
                    file_ = fopen(name, "ab");

                    if(file_)
                    {
                        fwrite((void *)csd, 1, csdSize, file_);
                        fclose(file_);
                    }
                }
#endif

                break;
            }

        }

#endif
    }
    else
    {
        ALOGI("Unknown Video Type!!\n"); //should handle the other types of codecspecificdata
    }

    delete [] _config;
    return ret;

    //}

    //ALOGE ("[ASF_ERROR]RetrieveWmvCodecSpecificData no codec specific info available");
    //ret=false;
    //return ret;
}

////Stream Properties Object
//Stream Type Specific:Stream Type Audio Media  18byte config + cb size extra data
bool ASFExtractor::RetrieveWmaCodecSpecificData(asf_stream_t * pStreamProp, const sp<MetaData>& meta)
{
    if(pStreamProp->flags & ASF_STREAM_FLAG_AVAILABLE)
    {
        asf_waveformatex_t *wfx = (asf_waveformatex_t *)pStreamProp->properties;
        switch (wfx->wFormatTag)
        {
        case WAVE_FORMAT_WMA1:
            ALOGI ("WMA1\n");
            break;
        case WAVE_FORMAT_WMA2:
            ALOGI ("WMA2\n");
            break;
        case WAVE_FORMAT_WMA3:
        {
#ifdef MTK_SWIP_WMAPRO
	    ALOGE ("WMA3\n"); 
	    break;
#else
	    ALOGE("[VC-1 Playback capability Error] capability not support as :audio WMA3 is not supported.\n");
            return false;
#endif

        };
        case WAVE_FROMAT_MSPCM:
        {
            ALOGI ("MSPCM\n");

            break;
        }
        case WAVE_FORMAT_MP3:
        {
            ALOGI("MP3\n");
            break;
        }
        case WAVE_FROMAT_MSADPCM:
        {
            ALOGE("[VC-1 Playback capability Error] capability not support as :Unknown audio format MSADPCM=0x%08x\n",wfx->wFormatTag);

            return false;;
        }
        case WAVE_FORMAT_MP2:
        {
            ALOGE("MP2(MPEG-1 Audio Layer II)\n");
            break;
        }
        case WAVE_FORMAT_AAC:
        case WAVE_FORMAT_AAC_AC:
        case WAVE_FORMAT_AAC_pm:
        {
            ALOGE("AAC audio format=0x%08x\n",wfx->wFormatTag);
            meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
            int profile = 1;
            meta->setInt32(kKeyAACProfile, profile);


			meta->setInt32(kKeySampleRate, wfx->nSamplesPerSec);
			meta->setInt32(kKeyChannelCount, wfx->nChannels);

			sp<ABuffer> csd = new ABuffer(wfx->cbSize);
			memcpy(csd->data(), wfx->data, wfx->cbSize);


			hexdump(csd->data(), csd->size());
			sp<ABuffer> esds = MakeESDS(csd);
			meta->setData(kKeyESDS, kTypeESDS, esds->data(), esds->size());
            break;
        }
        /*
        case //LPCM
        {

        	break;
        }
        case //WMAPro
        {

        	break;
        }
        */
        default:
        {
            ALOGE("[VC-1 Playback capability Error] capability not support as :Unknown audio format=0x%08x\n",wfx->wFormatTag);
            return false;
        }
        }
        ALOGI ("-----[ASF]RetrieveWmaCodecSpecificData -----\n");
        ALOGI ("wfx->wFormatTag = 0x%x\n", wfx->wFormatTag);
        ALOGI ("wfx->nChannels = %d\n", wfx->nChannels);
        ALOGI ("wfx->nSamplesPerSec = %d\n", wfx->nSamplesPerSec);
        ALOGI ("wfx->nAvgBytesPerSec = %d\n", wfx->nAvgBytesPerSec);
        ALOGI ("wfx->nBlockAlign = %d\n", wfx->nBlockAlign);
        ALOGI ("wfx->wBitsPerSample = %d\n", wfx->wBitsPerSample);
        ALOGI ("wfx->cbSize = %d\n", wfx->cbSize);
        if(wfx->wFormatTag == WAVE_FORMAT_WMA1 || wfx->wFormatTag == WAVE_FORMAT_WMA2)
        {
            meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMA);
        }
#ifdef MTK_SWIP_WMAPRO
	else if(wfx->wFormatTag == WAVE_FORMAT_WMA3)
	{
	    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMAPRO);
	}
#endif
        else if(wfx->wFormatTag == WAVE_FROMAT_MSPCM)
        {
            meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RAW);

            // for PCM component
#ifdef MTK_AUDIO_RAW_SUPPORT
            meta->setInt32(kKeyEndian, 2);                     //1: big endian, 2: little endian(default)
            meta->setInt32(kKeyBitWidth, wfx->wBitsPerSample); //bits per sample
            meta->setInt32(kKeyPCMType, 1);                    //1. WAV file  2.BD file 3.DVD_VOB file,  4.DVD_AOB file
            //meta->setInt32(kKeyChannelAssignment, 1);
            /* support unsigned PCM */
            if(wfx->wBitsPerSample == 8)
            {
                meta->setInt32(kKeyNumericalType, 2);
            }
#endif
        }
        else if(wfx->wFormatTag == WAVE_FORMAT_MP3)
        {
            meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG);
        }
        else if(wfx->wFormatTag == WAVE_FORMAT_MP2)
        {
            meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II);
        }
#ifdef MTK_SWIP_WMAPRO
        if(wfx->wFormatTag == WAVE_FORMAT_WMA1 || wfx->wFormatTag == WAVE_FORMAT_WMA2 || wfx->wFormatTag == WAVE_FORMAT_WMA3)
#else
        if(wfx->wFormatTag == WAVE_FORMAT_WMA1 || wfx->wFormatTag == WAVE_FORMAT_WMA2)
#endif
        {
            uint32_t _config_size = ASF_WAVEFORMATEX_SIZE + wfx->cbSize;  //wfx->cbSize must be 10
            uint8_t* _config = new uint8_t[_config_size];
            ALOGI("kKeyWMAC size is =%d\n",_config_size);
            memcpy(_config, wfx, ASF_WAVEFORMATEX_SIZE);
            memcpy(_config + ASF_WAVEFORMATEX_SIZE, wfx->data, wfx->cbSize);
			if(wfx->wFormatTag == WAVE_FORMAT_WMA3)
				meta->setData(kKeyWMAPROC, 0, _config, _config_size);
			else
				meta->setData(kKeyWMAC, 0, _config, _config_size);

            delete [] _config;
        }

        meta->setInt32(kKeySampleRate, wfx->nSamplesPerSec);
        meta->setInt32(kKeyChannelCount, wfx->nChannels);
        meta->setInt32(kKeyMaxInputSize, MAX_AUDIO_INPUT_SIZE);//4K

        return true;

    }

    ALOGE ("[ASF_ERROR]RetrieveWmaCodecSpecificData no codec specific info available");
    return false;
}


bool ASFExtractor::ParseASF()
{
    ALOGI ("+[ASF]ASFExtractor::ParseASF");


    ALOGI("===============================================================================\n");
    ALOGI("[VC-1 Playback capability info]\n");
    ALOGI("=====================================\n");
    ALOGI("Resolution = \"[(8,8) ~ (1280720)]\" \n");
    ALOGI("Profile_Level = \"VC1 simple, main,adnvanced profile\" \n");
    ALOGI("Support Codec = \"Video:WMV3,WMVA,WVC1 ; Audio: WMA1 0x160,WMA2 0x161\" \n");
    //ALOGI("Max frameRate =  120fps \n");
    ALOGI("Performance limitation = 4Mbps  (720*480@30fps)\n");
    ALOGI("===============================================================================\n");



    if (mAsfParser)
    {
        uint8_t hasDRMObj=mAsfParser->asf_parse_check_hasDRM();
        if(hasDRMObj)
        {
            ALOGE("!!![ASF_ERROR]has DRM obj, encrypted file, not support");
            return false;
        }
        uint64_t mDurationMs = mAsfParser->asf_get_duration();
        ALOGI ("Duration: %lld ms\n", mDurationMs);
        if(mDurationMs<=0)
        {
            ALOGE("!!![ASF_ERROR]Duration =0, error file, not support");
            return false;
        }

        int numTracks = mAsfParser->asf_get_stream_count();
        ALOGI ("Num of Streams: %d\n", numTracks);

        uint32_t max_bitrate = mAsfParser->asf_get_max_bitrate();
        ALOGI ("Max bitrate: %d\n", max_bitrate);

        mSeekable = mAsfParser->asf_is_seekable();
        ALOGI ("mSeekable: %d\n", (mSeekable > 0));

        int numpackets = mAsfParser->asf_get_data_packets();
        ALOGI ("numpackets: %d \n", numpackets);
        if(numpackets<=0)
        {
            ALOGE("!!![ASF_ERROR]has no packets data, error file, not support");
            return false;
        }

        mPrerollTimeUs =(mAsfParser->asf_get_preroll_ms())*1000;

        ALOGI ("mPrerollTimeUs: %lld ms\n", mPrerollTimeUs/1000);
        /*
                asf_obj_extended_content_description_t* extended_content_description;
                extended_content_description = mAsfParser->asf_get_extended_content_description();
                if(extended_content_description==NULL){
                     ALOGE("ParseASF: no extended_content_description data\n ");
                }
                else{

                    uint32_t size=extended_content_description->extended_content_wm_pic_len;
                    void* data = extended_content_description->extended_content_wm_pic;
        	     mFileMeta->setData(kKeyAlbumArt, MetaData::TYPE_NONE,  data,  size);
                    ASF_DEBUG_LOGE("ParseASF:mAlbumArt.size=%d,mAlbumArt.data=0x%08x", size, data);
                }
         */

#if 1
        // retrieve meta data (optional)
        asf_metadata_t * pASFMetadata = mAsfParser->asf_header_get_metadata();
        if (!pASFMetadata)
        {
            ALOGE ("ASFExtractor pAsfParser->asf_header_get_metadata failed\n");
        }
        else
        {
            //kKeyArtist ' "artist"
            //  kKeyAlbum             = 'albu',  //
            // kKeyTitle             = 'titl',
            //kKeyAlbumArt          = 'albA',

            char keyArtist[7]="Author";
            uint32_t out_len;
            asf_metadata_entry_t* entry  =  NULL;

            entry =mAsfParser->asf_findMetaValueByKey(pASFMetadata,keyArtist,6);
            if(entry && entry->value){
				mFileMeta->setCString(kKeyArtist, entry->value);
			}else{
			    ASF_DEBUG_LOGE("no kKeyArtist / Author in file meta");
			}
            entry=NULL;
            char keyAlbum[14]="WM/AlbumTitle";
            entry =  mAsfParser->asf_findMetaValueByKey(pASFMetadata,keyAlbum,13);
            if(entry && entry->value) {
				mFileMeta->setCString(kKeyAlbum,  entry->value);
            }else{
                ASF_DEBUG_LOGE("no kKeyAlbum / WM/AlbumTitle  in file meta");
            }
            entry=NULL;
            char keyTitle[6]="Title";
            entry =  mAsfParser->asf_findMetaValueByKey(pASFMetadata,keyTitle,5);
            if(entry && entry->value){
			    mFileMeta->setCString(kKeyTitle,  entry->value);
			}else{
			    ASF_DEBUG_LOGE("no kKeyTitle / Title in file meta");
		    }
            entry=NULL;
            char keyAlbumArt[11]="WM/Picture";
            entry =  mAsfParser->asf_findMetaValueByKey(pASFMetadata,keyAlbumArt,10);
            if(entry && entry->value){
                uint32_t dataoff=0;
                mAsfParser->asf_parse_WMPicture((uint8_t*)(entry->value), entry->size, &dataoff);
                mFileMeta->setData(kKeyAlbumArt, MetaData::TYPE_NONE,  entry->value+dataoff, entry->size-dataoff);
            }else{
                ASF_DEBUG_LOGE("no kKeyAlbumArt  / WM/Picture in file meta ");
            }
#if 0
            if(entry)
            {
                char name[260];
                FILE*	file_;
                sprintf(name, "/sdcard/pic.bmp");
                file_ = fopen(name, "wb");
                uint32_t type;
                const void *data;
                size_t size;

                if (mFileMeta->findData(kKeyAlbumArt, &type, &data, &size))
                {
                    fwrite(data, 1, size,   file_);
                    ALOGE("WM/Picture size  %d    )",size);
                }

                fclose(file_);


            }
#endif

            mAsfParser->asf_metadata_destroy(pASFMetadata);
        }

#endif



        //uint32_t max_buffer_size_video;
        //uint32_t max_buffer_size_audio;

        uint32_t stream_id_video = 0;
        uint32_t stream_id_audio = 0;

        // get stream and format type
        for (int i = ASF_STREAM_ID_START ; i <= numTracks ; i++)
        {
            asf_stream_t * pStreamProp = NULL;
            asf_stream_type_t stream_type = ASF_STREAM_TYPE_UNKNOWN;
            pStreamProp = mAsfParser->asf_get_stream(i);
            stream_type = pStreamProp->type;

            sp<MetaData> meta = new MetaData;

            void* pCodecSpecificData =NULL;//video will set ,audio not use
            uint32_t CurCodecSpecificSize=0;

            meta->setInt64(kKeyDuration, mDurationMs*1000LL);
            meta->setInt32(kKeyBitRate, max_bitrate);

            if (stream_type == ASF_STREAM_TYPE_AUDIO)
            {

                if (mDbgFlags & ASFFF_IGNORE_AUDIO_TRACK)
                {
                    ALOGE("[ASF_ERROR]ParseASF:ASFFF_IGNORE_AUDIO_TRACK\n");
                    continue;
                }

                ALOGI ("[ASF]Stream %d is AUDIO: ", i);
                stream_id_audio = i;

                //codec specific data poniter is not used,set null

                //set WMA codec specific data to meta
                if (false == RetrieveWmaCodecSpecificData(pStreamProp, meta))
                {
                    meta = NULL;
                    ALOGE("[ASF_ERROR]RetrieveWmaCodecSpecificData rerurn false, meta == NULL\n");
                    continue;
                }

#if 0
                // get max buffer size
                if(pStreamProp->flags & ASF_STREAM_FLAG_EXTENDED)
                {
                    max_buffer_size_audio = pStreamProp->extended->max_obj_size;
                }
                else
                {
                    max_buffer_size_audio = mAsfParser->asf_get_packet_size();
                }
                ALOGE ("Audio Max buffer size = %d\n", max_buffer_size_audio);
#endif
            }

            else if (stream_type == ASF_STREAM_TYPE_VIDEO)
            {

                mHasVideoTrack = true;

                if (mDbgFlags & ASFFF_IGNORE_VIDEO_TRACK)
                {
                    ALOGE("[ASF_ERROR]ParseASF:ASFFF_IGNORE_VIDEO_TRACK\n");
                    mHasVideo = false;
                    continue;
                }

                ALOGI ("Stream %d is VIDEO: ", i);
                stream_id_video = i;

                pCodecSpecificData =(VC1SeqData*)calloc(1, sizeof(VC1SeqData));
                CurCodecSpecificSize= sizeof(VC1SeqData);
                if(pCodecSpecificData==NULL)
                {
                    ALOGE("[ASF_ERROR]:calloc VC1SeqData failed, stream_id_video=%d\n",i);
                }
                ASF_DEBUG_LOGV("ASF ParseASF:calloc pCodecSpecificData=0x%08x\n",(uint32_t)pCodecSpecificData);



                if(!(pStreamProp->flags & ASF_STREAM_FLAG_AVAILABLE))
                {
                    ALOGE ("[ASF_ERROR]RetrieveWmvCodecSpecificData no codec specific info available");
                    meta = NULL;
                    //mFileMeta->setInt32(kKeyHasUnsupportVideo,true);
                    continue;
                }
                else if (false == RetrieveWmvCodecSpecificData(pStreamProp, meta,(VC1SeqData*)pCodecSpecificData, i))
                {
                    ALOGI ("no codec specific data, or unsupported video format ");
					if( bIgnoreVideo== true){
						 meta = NULL;
						 continue;
					}		
                }
                else
                {
                    ALOGI ("change!");
                }

                mHasVideo = true;//if code come to here , video

#if 0
                // get max buffer size
                if(pStreamProp->flags & ASF_STREAM_FLAG_EXTENDED)
                {
                    max_buffer_size_video = pStreamProp->extended->max_obj_size;
                }
                else
                {
                    max_buffer_size_video = mAsfParser->asf_get_packet_size();
                }
                ALOGE ("Video Max buffer size = %d\n", max_buffer_size_video);

#endif



            }
            else
            {
                ALOGE ("[ASF]Stream %d is not audio or video, skip it,stream type is =%d: ", i,stream_type);
                meta = NULL;
                continue;
            }
            //acesss meta from here start NE
            ALOGI("mTrackNum=%d",i);

            if(meta == NULL)
            {
                ALOGE ("[ASF]Stream %d has no metadata, please check,stream type is =%d: ", i,stream_type);

            }
            mTracks.push();
            TrackInfo *trackInfo = &mTracks.editItemAt(mTracks.size() - 1);
            trackInfo->mTrackNum = i;	// stream id
            trackInfo->mMeta = meta;//track meta is important
            trackInfo->mCodecSpecificData=pCodecSpecificData;
            trackInfo->mCodecSpecificSize=CurCodecSpecificSize;
            trackInfo->mNextPacket =  mAsfParser->asf_packet_create();
            trackInfo->mCurPayloadIdx=0;
        }

    }

    mIsAsfParsed = true;
    return true;
}


bool ASFExtractor::ASFSeekTo(uint32_t targetNPT)
{
    int64_t _new_ts = mAsfParser->asf_seek_to_msec(targetNPT);	// ms
    ALOGI ("[ASF]ASFSeekTo [%d ms] return %lld", targetNPT, _new_ts);
    //if(_new_ts<0)
    //add by qian
    ResetMediaFrameFlags();
    return true;
}
////////////////////////////////////////////////////////////////////////////////
int switchAACSampleRateToIndex(int sample_rate)
{

    int index = 0;

    switch(sample_rate)
    {
    case 96000:
        index = 0;
        return index;

    case 88200:
        index = 1;
        return index;

    case 64000:
        index = 2;
        return index;

    case 48000:
        index = 3;
        return index;

    case 44100:
        index = 4;
        return index;

    case 32000:
        index = 5;
        return index;

    case 24000:
        index = 6;
        return index;
		
	case 22050:
		index = 7;
		return index;

    case 16000:
        index = 8;
        return index;

    case 12000:
        index = 9;
        return index;

    case 11025:
        index = 10;
        return index;

    case 8000:
        index = 11;
        return index;
		
	case 7350:
		index = 12;
		return index;

    default:
        index = -1;
        ALOGE("switchAACSampleRateToIndex: error sample rate: %d , just use index 0 to try", sample_rate);
        return index;

    }
}
void ASFExtractor::EncodeSize14(uint8_t **_ptr, size_t size) {
    CHECK_LE(size, 0x3fff);

    uint8_t *ptr = *_ptr;

    *ptr++ = 0x80 | (size >> 7);
    *ptr++ = size & 0x7f;

    *_ptr = ptr;
}

sp<ABuffer> ASFExtractor::MakeESDS(const sp<ABuffer> &csd) {
	sp<ABuffer> esds = new ABuffer(csd->size() + 25);

	uint8_t *ptr = esds->data();
	*ptr++ = 0x03;
	EncodeSize14(&ptr, 22 + csd->size());

	*ptr++ = 0x00;  // ES_ID
	*ptr++ = 0x00;

	*ptr++ = 0x00;  // streamDependenceFlag, URL_Flag, OCRstreamFlag

	*ptr++ = 0x04;
	EncodeSize14(&ptr, 16 + csd->size());

	*ptr++ = 0x40;  // Audio ISO/IEC 14496-3

	for (size_t i = 0; i < 12; ++i) {
	    *ptr++ = 0x00;
	}

	*ptr++ = 0x05;
	EncodeSize14(&ptr, csd->size());

	memcpy(ptr, csd->data(), csd->size());

	return esds;
}

status_t ASFExtractor::addMPEG4CodecSpecificData(const sp<MetaData>& mMeta) {
	
    off64_t offset;
    size_t size;
	size_t vopStart = 0;
    bool foundVopStart = false;
	size_t vopEnd = 0;
    bool foundVopEnd = false;

	offset = mAsfParser->asf_get_data_position()+50;
    size = MAX_VIDEO_INPUT_SIZE ;
    sp<ABuffer> buffer = new ABuffer(size);
    ssize_t n = mDataSource->readAt(offset, buffer->data(), buffer->size());

    if (n < (ssize_t)size) {
        return n < 0 ? (status_t)n : ERROR_MALFORMED;
    }

    // Extract everything up to the first VOP start code from the first
    // frame's encoded data and use it to construct an ESDS with the
    // codec specific data.

    while (vopStart + 3 < buffer->size()) {
        if (!memcmp("\x00\x00\x01\xb0", &buffer->data()[vopStart], 4)) {
            foundVopStart = true;
            break;
        }
        ++vopStart;
    }

    if (!foundVopStart) {
		ALOGI("Vop start not found!");
        return ERROR_MALFORMED;			
    }
	
    while (vopEnd + 3 < buffer->size()) {
        if (!memcmp("\x00\x00\x01\xb6", &buffer->data()[vopEnd], 4)) {
            foundVopEnd = true;
            break;
        }
        ++vopEnd;
    }

    if (!foundVopEnd) {
		ALOGI("Vop end not found!");
        return ERROR_MALFORMED;			
    }
	ALOGI("vopStart = lu%,vopEnd = lu%", vopStart,vopEnd);
	for (uint32_t i=vopStart; i<vopEnd; i++){
		ALOGI("VOS[%d] = 0x%x", i, *((uint8_t *)buffer->data() + i));
	}
	
    buffer->setRange(vopStart, vopEnd);
    sp<ABuffer> csd = MakeESDS(buffer);
	mMeta->setData(kKeyESDS, kTypeESDS, csd->data(), csd->size());
    return OK;
}
//little endian
void ASFExtractor::MakeFourCCString(uint32_t x, char *s) {
	s[0] = x;
    s[1] = (x >> 8) & 0xff;
    s[2] = (x >> 16) & 0xff;
    s[3] = (x >> 24) & 0xff;
    s[4] = '\0';
}
uint32_t ASFExtractor::MakeStringToIntFourCC(char *strFourCC)
{
	uint32_t intfourCC=0;
	if ( strFourCC==NULL ) {
		return 0;
	}
	
	for( int i=3; i>=0;i-- ){
		 if(i!=3){
		 	intfourCC=(intfourCC<<8)|strFourCC[i];
		}else{
			intfourCC = strFourCC[i];
		}
	}
	return intfourCC;
}

}  // namespace android



