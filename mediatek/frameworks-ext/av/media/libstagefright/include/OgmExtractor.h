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

#ifndef OGM_EXTRACTOR_H_
#define OGM_EXTRACTOR_H_

#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaBuffer.h>
#include <utils/KeyedVector.h>

extern "C" {
	#include <Tremolo/codec_internal.h>
	int _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb);
	int _vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb);
	int _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb);
}

namespace android {

#define OGMSEEKTHRESHOLD	3000000	// 3 seconds
#define OGMSEEKTOLERENCE	500000	// 0.5 seconds

enum OGMType {
    //Header
    OGM_HEADER = 0,        

    //Audio
    OGA_AUDIO_MP3 = 1,
    OGA_AUDIO_VORBIS = 2,
    OGA_AUDIO_AAC = 3,
    OGA_AUDIO_WMA = 4,
    OGA_AUDIO_PCM = 5,
    OGA_AUDIO_MPG = 6,  //MPEG Audio Layer 1/2
    OGA_AUDIO_AC3 = 7,
    OGA_AUDIO_DTS = 8,

    //Video
    OGV_VIDEO_AVC = 16,
    OGV_VIDEO_AVC_HEADER = 17,
    OGV_VIDEO_MP4 = 18,
    OGV_VIDEO_MPEG = 19,
    OGV_VIDEO_H263 = 20,
    OGV_VIDEO_VC1 = 21,
    OGV_VIDEO_DIVX3 = 22,

    //Other
    OGM_OTHER = 30,
};

struct Page {
	uint64_t mGranulePosition;
	uint32_t mSerialNo;
	uint32_t mPageNo;
	uint8_t mFlags;
	uint8_t mNumSegments;
	uint8_t mLace[255];

	uint32_t mSamples;	//TESTING
};

struct OgmExtractor : public MediaExtractor {
	OgmExtractor(const sp<DataSource> &source);
	virtual size_t countTracks();
	virtual sp<MediaSource> getTrack(size_t index);
	virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);
	virtual sp<MetaData> getMetaData();

	//////////////////////////////////////////////////////////////////////////////////////////////
	status_t initExtractor();
	status_t verifyHeader(MediaBuffer *buffer);
	OGMType GetFourCCTypeForHandler(const char *fourCC);
	status_t verifyVorbisHeader(const uint8_t *data, size_t size);
	status_t verifyOgmHeader(const uint8_t *data, size_t size);
	status_t findPage(off_t startOffset, off_t *pageOffset, off_t searchLength);
	ssize_t readPage(off_t pageOffset, Page *page);
	status_t dequeuePacket();
	status_t seekToOffset(off_t offset, off_t searchSize);
	status_t seekToTime(const int64_t timeUs);
	void signalDiscontinuity();
	void clearGranulePosition();
	int64_t getCurTime();
	int64_t getMaxTime(bool isAllTracksCollected);
	void parseMaxTime();
	int64_t getDuration();
	void setDequeueState(bool needDequeuePacket);
	bool getDequeueState();
	void setSeekingState(bool isVideoSeeking);
	bool getSeekingState();
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	protected:
		virtual ~OgmExtractor();

	private:
		struct OgmTrack;
		Mutex mExtractorLock;

		bool mHasVideoTrack;
		bool mHasUnsupportedVideoTrack;

		bool mSearchingTracks;
		bool mNeedDequeuePacket;
		bool mIsVideoSeeking;
		sp<DataSource> mDataSource;
		off64_t mDataSourceSize;

		int64_t mDuration;
		int64_t mAverageByteRate;
		//int64_t mSeekTimeUs;
		//off_t mSeekOffset;

		//The Data Source offset where extractor processing
		off_t mOffset;
		off_t mFirstDataPageOffset;
		//The current page handling info. (at mOffset)
		Page mCurrentPage;
		//bool mNewPacketHandling;
		ssize_t mCurrentPageSize;
		ssize_t mNextLaceIndex;

		//The Data Source Tracks, index Key is Serial Number
		KeyedVector<uint32_t, sp<OgmTrack> > mOgmTracks;

		//////////////////////////////////////////////////////////////////////////////////////////////

		OgmExtractor(const OgmExtractor &);
		OgmExtractor &operator=(const OgmExtractor &);
};


bool SniffOgm(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *);


}//end of namespace android

#endif


