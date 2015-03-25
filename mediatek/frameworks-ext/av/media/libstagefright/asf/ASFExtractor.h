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
 *   ASFExtractor.h
 *
 * Project:
 * --------
 *   MT6573
 *
 * Description:
 * ------------
 *   ASF Extractor interface
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/
#ifndef ASF_EXTRACTOR_H_

#define ASF_EXTRACTOR_H_

#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaBuffer.h>
#include <utils/Vector.h>
#include <utils/threads.h>

#include "asfparser.h"


namespace android {


#define ASF_EVERYTHING_FINE 0
#define ASF_END_OF_TRACK 1
#define ASF_INSUFFICIENT_DATA 2
#define ASF_INSUFFICIENT_BUFFER_SIZE 3
#define ASF_READ_FAILED 4


#define MAX_VIDEO_WIDTH 1920
#define MAX_VIDEO_HEIGHT 1080
#define MAX_VIDEO_INPUT_SIZE (MAX_VIDEO_WIDTH*MAX_VIDEO_HEIGHT*3 >> 1)
#define MAX_AUDIO_INPUT_SIZE (1024*20)

#define ASFFF_SHOW_TIMESTAMP        (1 << 0)
#define ASFFF_IGNORE_AUDIO_TRACK    (1 << 1)
#define ASFFF_IGNORE_VIDEO_TRACK    (1 << 2)

#define ASF_THUMBNAIL_SCAN_SIZE 10

struct AMessage;
class DataSource;
class String8;


#define GECKO_VERSION                 ((1L<<24)|(0L<<16)|(0L<<8)|(3L))


enum Type {
        ASF_VIDEO,
        ASF_AUDIO,
        OTHER
};

enum {
	IVOP,
	PVOP, 
	BVOP,
	BIVOP,
	SKIPPED
};


struct  VC1SeqData{
    uint32_t profile;
    uint32_t level;
    uint32_t rangered;
    uint32_t maxbframes;
    uint32_t finterpflag;
    uint32_t multires;
    uint32_t fps100;    
    uint64_t us_time_per_frame;
    
};



struct TrackInfo {
        uint32_t mTrackNum;
        sp<MetaData> mMeta;    
        void * mCodecSpecificData;
        uint32_t mCodecSpecificSize;
        asf_packet_t * mNextPacket;
        uint32_t mCurPayloadIdx;        
    };


struct  VC1PicData{
    uint32_t interpfrm;
    uint32_t rangeredfrm;
    uint32_t frmcnt;
    uint32_t ptype;
};



class ASFExtractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source".
    ASFExtractor(const sp<DataSource> &source);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();
    virtual uint32_t flags() const;

protected:
    virtual ~ASFExtractor();

    bool ParseASF();
    bool ASFSeekTo(uint32_t targetNPT);

    bool RetrieveWmvCodecSpecificData(asf_stream_t * pStreamProp, const sp<MetaData>& meta,VC1SeqData* pCodecSpecificData, uint32_t currIdx);
    bool RetrieveWmaCodecSpecificData(asf_stream_t * pStreamProp, const sp<MetaData>& meta);


    void findThumbnail();

public:
    friend class ASFSource;

    friend int32_t asf_io_read_func(void *aSource, void *aBuffer, int32_t aSize);
    friend int32_t asf_io_write_func(void *aSource, void *aBuffer, int32_t aSize);
    friend int64_t asf_io_seek_func(void *aSource, int64_t aOffset);

    bool IsValidAsfFile() { return mIsValidAsfFile; }
    bool ParserASF();
    ASFErrorType GetNextMediaPayload(uint8_t* aBuffer, uint32_t& arSize, uint32_t& arTimeStamp, uint32_t& arRepDataSize, bool& bIsKeyFrame,uint32_t CurTrackIndex);
    ASFErrorType GetNextMediaFrame(MediaBuffer** out, bool& bIsKeyFrame,Type strmType,bool *isSeeking,uint32_t CurTrackIndex);
    bool CheckMediaFrame(MediaBuffer** pCurFrmMediaBuffer, uint32_t CurPicType);
    bool CorrectMediaFrameTS(MediaBuffer** pCurFrmMediaBuffer, uint32_t CurPicType);
    void ResetMediaFrameFlags();

    bool ParserVC1CodecPrivateData(uint8_t*input , uint32_t inputlen, VC1SeqData* pSeqData);
    uint32_t ParserVC1FrameType(uint8_t* bitstream, uint32_t CurTrackIndex);
	uint32_t ParserAVCFrameType(uint8_t* data, int size);
	uint32_t ParserMPEG4FrameType(uint8_t* data, int size);
	sp<ABuffer> MakeESDS(const sp<ABuffer> &config);
	status_t addMPEG4CodecSpecificData(const sp<MetaData>& mMeta);
	void EncodeSize14(uint8_t **_ptr, size_t size);
	void MakeFourCCString(uint32_t x, char *s);
	uint32_t MakeStringToIntFourCC(char *s);

private:       

   
    VC1PicData mVC1CurPicData;
    sp<MetaData> mFileMeta;
        
    uint32_t mDbgFlags;	
    bool bIgnoreAudio;
    bool bIgnoreVideo;
    uint64_t mDurationMs;
    bool mSeekable;

    bool bThumbnailMode;
    bool mExtractedThumbnails;

    int64_t mParsedFrameNum ;
    int64_t mFisrtFrameTsUs;
    bool mFisrtFrameParsed;
    bool mSecondFrameParsed;

    int64_t mPrerollTimeUs;
    sp<MetaData> mFileMetaData;

    ASFExtractor(const ASFExtractor &);
    ASFExtractor &operator=(const ASFExtractor &);

protected:
	sp<DataSource> mDataSource;
	Vector<TrackInfo> mTracks;
	android::Mutex mCacheLock;

	ASFParser* mAsfParser;
	uint32_t mAsfParserReadOffset;
	bool mIsValidAsfFile;
	bool mIsAsfParsed;
	bool mHasVideo;
    bool mHasVideoTrack;
    uint64_t mFileSize;
};

int switchAACSampleRateToIndex(int sample_rate);

}  // namespace android

#endif  // RM_EXTRACTOR_H_
