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

#define LOG_TAG "OgmExtractor"

#include "include/avc_utils.h"
#include "mpeg2ts/ESQueue.h"

#include <utils/Log.h>
#include <media/stagefright/DataSource.h>
#include "include/OgmExtractor.h"
#include <cutils/properties.h>

#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <cutils/xlog.h>

#include <sys/time.h>

#define OGM_AU_SIZE 65535

namespace android {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct TrackAUSource{
	TrackAUSource();
	bool hasBufferAvailable(status_t *finalResult);
	void queueAccessUnit(const sp<ABuffer> &buffer);
	status_t dequeueAccessUnit(MediaBuffer **out);
	void clear();
	status_t getAccessUnitTimeStamp(int64_t *auTimeUs);
	//void setFormat(const sp<MetaData> &meta);
	//virtual status_t start(MetaData *params = NULL);
	//virtual status_t stop();
	//virtual sp<MetaData> getFormat();
	//virtual status_t read(MediaBuffer **buffer, const ReadOptions *options = NULL);
	//void signalEOS(status_t result);
	//void queueDiscontinuity(ATSParser::DiscontinuityType type, const sp<AMessage> &extra);

	//protected:
		virtual ~TrackAUSource();

	private:
		friend struct MediaSource;
		Mutex mLock;
		Condition mCondition;
		List<sp<ABuffer> > mAUs;
		status_t mEOSResult;
		//bool mIsAudio;
		//sp<MetaData> mFormat;//[qian] meta:AVC: sps/pps, MP4:ESDS

	//bool wasFormatChange(int32_t discontinuityType) const;
	//DISALLOW_EVIL_CONSTRUCTORS(TrackAUSource);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TrackAUSource::TrackAUSource():
mEOSResult(OK)
{
}

TrackAUSource::~TrackAUSource()
{
}

void TrackAUSource::clear() {
	Mutex::Autolock autoLock(mLock);
	if (!mAUs.empty())
	{
		mAUs.clear();
	}
	mEOSResult = OK;
}

bool TrackAUSource::hasBufferAvailable(status_t *finalResult)
{
	Mutex::Autolock autoLock(mLock);
	if (!mAUs.empty()) {
		return true;
	}

	*finalResult = mEOSResult;
	return false;
}

void TrackAUSource::queueAccessUnit(const sp<ABuffer> &buffer)
{
	int32_t damaged;
	if(buffer->meta()->findInt32("damaged", &damaged) && damaged)
	{
		XLOGD("Damaged AU, returned\n");
		return;
	}

	int64_t timeUs;
	CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
	//XLOGD("queueAccessUnit timeUs = %lld us(%.2f secs)", timeUs, timeUs/1E6);

	Mutex::Autolock autoLock(mLock);
	mAUs.push_back(buffer);
	mCondition.signal();
}

status_t TrackAUSource::dequeueAccessUnit(MediaBuffer **out)
{
	*out = NULL;

	Mutex::Autolock autoLock(mLock);
	while(mEOSResult == OK && mAUs.empty())
	{
		mCondition.wait(mLock);
	}

	if(!mAUs.empty())
	{
		const sp<ABuffer> buffer = *mAUs.begin();
		mAUs.erase(mAUs.begin());

		//int32_t discontinuity;
		//if(buffer->meta()->findInt32("discontinuity", &discontinuity))
		//{
		//
		//}
		int64_t timeUs;
		CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
		MediaBuffer *mediaBuffer = new MediaBuffer(buffer);
		mediaBuffer->meta_data()->setInt64(kKeyTime, timeUs);
		*out = mediaBuffer;
		//XLOGD("dequeueAccessUnit timeUs = %lld us(%.2f secs)", timeUs, timeUs/1E6);
		return OK;
	}
	return mEOSResult;
}

status_t TrackAUSource::getAccessUnitTimeStamp(int64_t *auTimeUs)
{
	Mutex::Autolock autoLock(mLock);
	*auTimeUs = 0;
	if(!mAUs.empty())
	{
		int64_t timeUs;
		const sp<ABuffer> buffer = *mAUs.begin();
		CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
		*auTimeUs = timeUs;
	}
	return OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct OgmExtractor::OgmTrack : public MediaSource{
	OgmTrack(const sp<OgmExtractor> &extractor, uint32_t serialNo, OGMType type);
	virtual status_t start(MetaData *params);
	virtual status_t stop();
	virtual sp<MetaData> getFormat();
	virtual status_t read(MediaBuffer **buffer, const ReadOptions *options);

	//////////////////////////////////////////////////////////////////////////////////////////////
	status_t appendData(MediaBuffer *buffer, bool ifNewPacket);
	void signalDiscontinuity();
	void clearGranulePosition();
	void setGranulePosition(uint64_t currentGranulePosition);
	void setSampleRate(int sampleRate);
	int64_t getTime();
	int64_t getMaxTime();
	bool isVideo();
	bool isAudio();

	//////////////////////////////////////////////////////////////////////////////////////////////

	protected:
		virtual ~OgmTrack();

	private:
		friend struct OgmExtractor;
		friend struct TrackAUSource;
		sp<OgmExtractor> mExtractor;
		ElementaryStreamQueue *mQueue;
		TrackAUSource* mAUSource;
		uint32_t mSerialNo;
		OGMType mType;
		bool mStarted;

		uint64_t mCurGranulePosition;
		uint64_t mPreGranulePosition;
		uint8_t mCurrentPageSamples;
		uint8_t mCurrentPageSamplesOut;
		bool mPreGPValid;
		int mSampleRate; //sample rate * 1000
		bool mWaitingVideoKeyFramePacket;
		int64_t mKeyPositionTime;
		int64_t mMaxKeyInterval;
		
		android::Mutex mOgmTrackOptLock;
		sp<ABuffer> maccessUnit;
		int32_t mVideoWidth;
		int32_t mVideoHeight;
		bool mFirstPkt;
		int32_t mBitsPerSample;
		int16_t mChannels;

		//ElementaryStreamQueue *mQueue;
		//OgmTrack(const OgmTrack &);
		//OgmTrack &Operator=(const OgmTrack &);

	DISALLOW_EVIL_CONSTRUCTORS(OgmTrack);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OgmExtractor::OgmTrack::OgmTrack(const sp<OgmExtractor> &extractor, uint32_t serialNo, OGMType type):
mExtractor(extractor),
mSerialNo(serialNo),
mType(type),
mStarted(false),
mQueue(NULL),
mAUSource(NULL),
mSampleRate(0),
mWaitingVideoKeyFramePacket(0),
mKeyPositionTime(0),
mMaxKeyInterval(0),
maccessUnit(NULL),
mVideoWidth(0),
mVideoHeight(0),
mFirstPkt(true),
mBitsPerSample(0),
mChannels(0)
{
	ElementaryStreamQueue::Mode mode;
	switch(mType)
	{
		case OGV_VIDEO_AVC:
			mode = ElementaryStreamQueue::H264;
			mQueue = new ElementaryStreamQueue(mode);
			break;
		case OGV_VIDEO_DIVX3:
			maccessUnit = new ABuffer(OGM_AU_SIZE);
			maccessUnit->setRange(0, 0);
		case OGV_VIDEO_MP4:
			mode = ElementaryStreamQueue::MPEG4_VIDEO;
			mQueue = new ElementaryStreamQueue(mode);
			break;
		case OGV_VIDEO_MPEG:
			mode = ElementaryStreamQueue::MPEG_VIDEO;
			mQueue = new ElementaryStreamQueue(mode);
			#if !defined(ANDROID_DEFAULT_CODE) && defined(MTK_OGM_PLAYBACK_SUPPORT)  
			mQueue->setSearchSCOptimize(true);
			#endif
			break;
		case OGA_AUDIO_AAC:
			mode = ElementaryStreamQueue::AAC;
			mQueue = new ElementaryStreamQueue(mode);
			break;
		case OGA_AUDIO_VORBIS:
			mode = ElementaryStreamQueue::VORBIS_AUDIO;
			mQueue = new ElementaryStreamQueue(mode);
			break;
		case OGA_AUDIO_MP3:
		case OGA_AUDIO_MPG:
			mode = ElementaryStreamQueue::MPEG_AUDIO;
			mQueue = new ElementaryStreamQueue(mode);
			break;
#ifdef MTK_AUDIO_RAW_SUPPORT
		case OGA_AUDIO_PCM:
			maccessUnit = new ABuffer(OGM_AU_SIZE);
			maccessUnit->setRange(0, 0);
			//dummy esqueue
			mode = ElementaryStreamQueue::PCM_AUDIO;
			mQueue = new ElementaryStreamQueue(mode);
			break;
#endif
		default:
			break;
	}

	if(mQueue != NULL)
	{
		mAUSource = new TrackAUSource;
	}

	clearGranulePosition();
}

OgmExtractor::OgmTrack::~OgmTrack()
{
	XLOGD("Track Type[%d] destroy\n", mType);
	if(mStarted)
	{
		stop();
	}

	if(mQueue)
	{
		delete mQueue;
		mQueue = NULL;
	}
	if(mAUSource)
	{
		mAUSource->clear();
		delete mAUSource;
		mAUSource = NULL;
	}
}

status_t OgmExtractor::OgmTrack::start(MetaData *params)
{
	if(mStarted)
	{
		return INVALID_OPERATION;
	}
	XLOGD("Track Type[%d] Start\n", mType);
	mStarted = true;
	return OK;
}

status_t OgmExtractor::OgmTrack::stop()
{
	XLOGD("Track Type[%d] Stop\n", mType);
	mStarted = false;
	return OK;
}

sp<MetaData> OgmExtractor::OgmTrack::getFormat()
{
	XLOGD("Track Type[%d] getFormat\n", mType);

	sp<MetaData> meta;

	if(mType == OGV_VIDEO_DIVX3)
	{
		meta = new MetaData;
		const unsigned char codecPrivate[1] = {0}; //unused

		meta->setCString(
				kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_DIVX3);
		meta->setData(kKeyMPEG4VOS, 0, &codecPrivate, 0);				
		meta->setInt32(kKeyWidth, mVideoWidth);
		meta->setInt32(kKeyHeight, mVideoHeight);
		meta->setInt64(kKeyDuration, mExtractor->getDuration());   //Need to Enable Seek Feature
		meta->setInt64(kKeyThumbnailTime, 0);
		return meta;
	}
#ifdef MTK_AUDIO_RAW_SUPPORT
	else if(mType == OGA_AUDIO_PCM)
	{
		meta = new MetaData;

		meta->setCString(
				kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RAW);
		
		meta->setInt32(kKeySampleRate, mSampleRate/1000);
		meta->setInt32(kKeyChannelCount, mChannels);
		meta->setInt32(kKeyEndian, 2);	//1: big endian, 2: little endia
		meta->setInt32(kKeyBitWidth, mBitsPerSample);
		if(mBitsPerSample == 8)
	{
			meta->setInt32(kKeyNumericalType, 2); // 1:signed(default), 2: unsigned
		}
		meta->setInt32(kKeyPCMType, 1);	//1: WAV file, 2: BD file, 3: DVD_VOB file, 4: DVD_AOB file
		ALOGD("PCM SampleRate %d, ChannelCount %d, Little endian, BitWidth %d, PCMType:WAV\n", 
			mSampleRate/1000, mChannels, mBitsPerSample);

		meta->setInt64(kKeyDuration, mExtractor->getDuration());   //Need to Enable Seek Feature
		meta->setInt64(kKeyThumbnailTime, 0);
		return meta;
	}
#endif
	else if(mQueue)
	{
		meta = mQueue->getFormat();
		if(meta != NULL)
		{
#ifndef ANDROID_DEFAULT_CODE		
#ifdef MTK_OGM_PLAYBACK_SUPPORT
			int32_t layer = 0;
			if(meta->findInt32(kKeyMPEGAudLayer, &layer))
			{
			if((mType == OGA_AUDIO_MPG) && (layer == 1))
			{
				XLOGD("MPEG Audio layer 1 not support\n");
				return NULL;
			}
			}
#endif
#endif
			meta->setInt64(kKeyDuration, mExtractor->getDuration());   //Need to Enable Seek Feature
			meta->setInt64(kKeyThumbnailTime, 0);
		}
		else
		{
			XLOGD("********** Track Type[%d] meta is NULL **************8\n", mType);
		}
		return meta;
		//return mQueue->getFormat();
	}
	else
	{
		XLOGD("********** Track Type[%d] mQueue is NULL????? **************8\n", mType);
		return NULL;
	}
}

status_t OgmExtractor::OgmTrack::read(MediaBuffer **buffer, const ReadOptions *options)
{
	int64_t seekTimeUs = 0;
	ReadOptions::SeekMode mode;
	bool ifSeekOption = false, ifAudioSeekOption = false;
	status_t result;
	status_t finalResult;
	int64_t currentTimeUs = 0;

	XLOGD("Track Type[%d] read\n", mType);

	if (options && options->getSeekTo(&seekTimeUs, &mode))
	{
		XLOGD("[%d]Seek time is %lld us(%.2f secs)\n", mType, seekTimeUs, seekTimeUs/1E6);

		if(mExtractor->mHasVideoTrack && isAudio())
		{
			XLOGD("Audio Track, Seek Option skipped\n");
			ifAudioSeekOption = true;
		}
		else
		{
			if(mSampleRate)
			{
				uint64_t targetGranulePosition = seekTimeUs * mSampleRate / 1000000000;
				XLOGD("Seek time transfer to GranulePosition is %llx", targetGranulePosition);
			}
			if(isVideo())
			{
				mExtractor->setSeekingState(true);
			}
			mExtractor->seekToTime(seekTimeUs);
			ifSeekOption = true;
			//int64_t currentTimeUs = 0;
			int64_t targetTimeUs = seekTimeUs;
			//Get the first I Frame from Queue
			if(isVideo())
			{
				mWaitingVideoKeyFramePacket = true;
			}
			while(!mAUSource->hasBufferAvailable(&finalResult) && mWaitingVideoKeyFramePacket)
			{
				if(targetTimeUs < OGMSEEKTHRESHOLD)
				{
					XLOGD("Seek to head\n");
					break;
				}

				if(isVideo())
				{
					mExtractor->setDequeueState(false);	//find key from video packet header 
				}
				result = mExtractor->dequeuePacket();
				if(result != OK)
				{
					mAUSource->clear();
					return ERROR_END_OF_STREAM;
				}
				//currentTimeUs = mExtractor->getMaxTime();
				//currentTimeUs = mExtractor->getCurTime();
				currentTimeUs = getTime();
				if(currentTimeUs > (targetTimeUs + OGMSEEKTOLERENCE)/*seekTimeUs*/)
				{
					targetTimeUs -= OGMSEEKTHRESHOLD;
					mWaitingVideoKeyFramePacket = false;
					
					mExtractor->seekToTime(targetTimeUs);
					if(isVideo())
					{
						mWaitingVideoKeyFramePacket = true;
					}
				}
			}
			mExtractor->setDequeueState(true);

			//Get AccessUnit Time
			if(!isVideo())
			{
			mAUSource->getAccessUnitTimeStamp(&currentTimeUs);
			}
			XLOGD("#[%d]Seek to time %lld us(%.2f secs),target(%.2f secs)I frame interval(%.2f secs)\n", 
				mType, currentTimeUs, currentTimeUs/1E6, seekTimeUs/1E6, (seekTimeUs-currentTimeUs)/1E6);
			mExtractor->setSeekingState(false);
		}
	}

	
	while(ifAudioSeekOption && mAUSource->hasBufferAvailable(&finalResult))
	{
		mAUSource->getAccessUnitTimeStamp(&currentTimeUs);
		if(currentTimeUs < seekTimeUs)
			result = mAUSource->dequeueAccessUnit(buffer);
		else
			break;
	}

	while(!mAUSource->hasBufferAvailable(&finalResult))
	{
		result = mExtractor->dequeuePacket();
		if(result != OK)
		{
			mAUSource->clear();
			return ERROR_END_OF_STREAM;
		}
		while(ifAudioSeekOption && mAUSource->hasBufferAvailable(&finalResult))
		{
			mAUSource->getAccessUnitTimeStamp(&currentTimeUs);
			if(currentTimeUs < seekTimeUs)
				result = mAUSource->dequeueAccessUnit(buffer);
			else
				break;
		}
	}

	result = mAUSource->dequeueAccessUnit(buffer);
	MediaBuffer *mediaBuffer;
	mediaBuffer = *buffer;
	if(mediaBuffer != NULL && ifSeekOption)
	{
		mediaBuffer->meta_data()->setInt64(kKeyTargetTime, seekTimeUs);
	}

	XLOGD("Track Type[%d] read out %lld/%lld, sz %d\n", mType, currentTimeUs, seekTimeUs, mediaBuffer->size());

	/*
	if(mType == OGA_AUDIO_MP3)
	{
		FILE *fp = fopen("/sdcard/mp3.bin", "ab");
		if(fp)
		{
			fwrite((void *)mediaBuffer->data(), 1, mediaBuffer->size(), fp);
			fclose(fp);
		}
	}
	if(mType == OGA_AUDIO_VORBIS)
	{
		FILE *fp = fopen("/sdcard/VORBIS.bin", "ab");
		if(fp)
		{
			fwrite((void *)mediaBuffer->data(), 1, mediaBuffer->size(), fp);
			fclose(fp);
		}
	}
	if(mType == OGV_VIDEO_MP4)
	{
		FILE *fp = fopen("/sdcard/MP4.bin", "ab");
		if(fp)
		{
			fwrite((void *)mediaBuffer->data(), 1, mediaBuffer->size(), fp);
			fclose(fp);
		}
	}
	if(mType == OGV_VIDEO_AVC)
	{
		FILE *fp = fopen("/sdcard/AVC.bin", "ab");
		if(fp)
		{
			fwrite((void *)mediaBuffer->data(), 1, mediaBuffer->size(), fp);
			fclose(fp);
		}
	}
	*/
	return result;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
status_t OgmExtractor::OgmTrack::appendData(MediaBuffer *buffer, bool ifNewPacket)
{
	android::Mutex::Autolock autoLock(mOgmTrackOptLock); 

	if(isAudio() && mExtractor->getSeekingState())
	{
		XLOGD("********** Track Type[%d] skip appending data because seeking*************\n", mType);
		return OK;
	}

	if(buffer == NULL)
	{
		XLOGD("appendData Buffer null\n");
		return ERROR_MALFORMED;
	}
	const uint8_t *data = (const uint8_t *)buffer->data() + buffer->range_offset();
	size_t size = buffer->range_length();
	//XLOGD("Check mQueue is not NULL\n");
	if(mQueue == NULL)
	{
		XLOGD("********** Track Type[%d] mQueue is Null, cannot append data *************\n", mType);
		return ERROR_MALFORMED;
	}
	//XLOGD("append Data\n");
	int64_t timeUs = 0;
	if(mSampleRate)
	{
		if(mPreGPValid)
		{
			timeUs = (mPreGranulePosition + mCurrentPageSamplesOut)  * 1000000000 / mSampleRate;
		}
		else
		{
			XLOGD("********** mPreGranulePosition is not vaild, Skip appending this item*************\n");	//PANDA
			return OK;
		}
	}
	if(ifNewPacket && (mCurrentPageSamplesOut < mCurrentPageSamples))
	{
		mCurrentPageSamplesOut ++;
	}

	if(mType == OGA_AUDIO_MP3 && ifNewPacket)
	{
		data += 3;
		size -= 3;
	}

	if(mType == OGV_VIDEO_DIVX3 || mType == OGA_AUDIO_PCM)
	{
		if((maccessUnit!=NULL) && (mAUSource != NULL) /*&& (mExtractor->mDuration > 0)*/)
		{
			static unsigned int u4Offset = 0;

			if(mFirstPkt)
			{
				u4Offset = 0;
				if(mType == OGV_VIDEO_DIVX3)
				{
				maccessUnit->meta()->setInt64("timeUs", 0);
				mAUSource->queueAccessUnit(maccessUnit);	//null packet to substitude VOS/VOL info
				XLOGD("Send NULL AU for VOS/VOL Info\n");
				}
				mFirstPkt = false;
			}

			if(ifNewPacket)
			{
				if(mType == OGV_VIDEO_DIVX3)
				{
				data += 1;
				size -= 1;
			}
				else if(mType == OGA_AUDIO_PCM)
				{
					data += 3;
					size -= 3;
				}
			}
			if(maccessUnit->size()>0 && ifNewPacket)
			{
				mAUSource->queueAccessUnit(maccessUnit);
				XLOGV("queueAccessUnit Sz %5d/0x%x, %x %x %x %x\n", maccessUnit->size(), u4Offset,
					*((unsigned char*)maccessUnit->data()), *((unsigned char*)maccessUnit->data() + 1),
					*((unsigned char*)maccessUnit->data() + 2), *((unsigned char*)maccessUnit->data() + 3));

				u4Offset += maccessUnit->size();
				maccessUnit = new ABuffer(OGM_AU_SIZE);
				maccessUnit->setRange(0, 0);
			}
			if(maccessUnit->size()==0)
			{
				maccessUnit->meta()->setInt64("timeUs", timeUs);
			}
			if((maccessUnit->size() + size) > maccessUnit->capacity())
			{
				int64_t buffer_timeUs = 0;
				sp<ABuffer> buffer = new ABuffer((maccessUnit->size() + size + OGM_AU_SIZE) & ~OGM_AU_SIZE);
				maccessUnit->meta()->findInt64("timeUs", &buffer_timeUs);
				
				memcpy(buffer->data(), maccessUnit->data(), maccessUnit->size());
				buffer->setRange(0, maccessUnit->size());			
				buffer->meta()->setInt64("timeUs", timeUs);
				maccessUnit = buffer;
				XLOGD("resize maccessUnit to %d\n", maccessUnit->capacity());
			}
			memcpy(maccessUnit->data() + maccessUnit->size(), data, size);
			maccessUnit->setRange(0, maccessUnit->size() + size);
		}
	}
	else
	{
	if(mQueue->appendData(data, size, timeUs) != OK)
	{
		XLOGD("********** Track Type[%d] mQueue append data fail *************\n", mType);
		return ERROR_MALFORMED;
	}

	//XLOGD("Check Track Type[%d] if mQueue can release a new access unit?\n", mType);
	sp<ABuffer> accessUnit;
	while((accessUnit = mQueue->dequeueAccessUnit()) != NULL)
	{
		//XLOGD(" Track Type[%d] mQueue dequeue AccessUnit\n", mType);
		if(mAUSource != NULL)
		{
			mAUSource->queueAccessUnit(accessUnit);
		}
	}
	}
	return OK;
}

void OgmExtractor::OgmTrack::signalDiscontinuity()
{
	if(mQueue == NULL)
	{
		return;
	}
	android::Mutex::Autolock autoLock(mOgmTrackOptLock); 
    
	mQueue->clear(false);	//Clean ESQueue without delete mFormat
	mQueue->setSeeking();
	mAUSource->clear();	//Clean Access units

	if(maccessUnit!=NULL)
	{
		maccessUnit->setRange(0, 0);
	}
}

void OgmExtractor::OgmTrack::clearGranulePosition()
{
	mCurGranulePosition = 0;
	if(mExtractor->mOffset <= mExtractor->mFirstDataPageOffset)
		mPreGranulePosition = 0;
	else
		mPreGranulePosition = 1;	//let PreGranulePosition bigger than CurGranulePosition, to check the PreGranulePosition can be used or not
	mCurrentPageSamples = 0;
	mCurrentPageSamplesOut = 0;
	mPreGPValid = false;
}

void OgmExtractor::OgmTrack::setGranulePosition(uint64_t currentGranulePosition)
{
	if(currentGranulePosition < mCurGranulePosition)
	{
		XLOGD(" *****Track Type[%d] GranulePosition Error*****", mType);
		clearGranulePosition();
	}
	if((mPreGranulePosition <= mCurGranulePosition) || (mCurGranulePosition == currentGranulePosition))
	{
		//XLOGD(" *****Track Type[%d] GranulePosition Valid is true*****", mType);
		mPreGPValid = true;
	}
	mPreGranulePosition = mCurGranulePosition;
	mCurGranulePosition = currentGranulePosition;
	mCurrentPageSamples = (uint8_t)(mCurGranulePosition - mPreGranulePosition);
	mCurrentPageSamplesOut = 0;

	if(mPreGPValid == false)
	{
		XLOGD(" *****Track Type[%d] update GP %lld, %lld*****", mType, mPreGranulePosition, mCurGranulePosition);
	}
}

int64_t OgmExtractor::OgmTrack::getTime()
{
	int64_t timeUs = 0;
	if(mSampleRate && mPreGPValid)
	{
		timeUs = (mPreGranulePosition + mCurrentPageSamplesOut)  * 1000000000 / mSampleRate;
		//XLOGD("getTime, mPreGranulePosition %lld\n", mPreGranulePosition);
		//XLOGD("getTime, timeUs %lld\n", timeUs);
	}
	return timeUs;
}

int64_t OgmExtractor::OgmTrack::getMaxTime()
{
	int64_t timeUs = 0;
	if(mSampleRate)
	{
		timeUs = mCurGranulePosition  * 1000000000 / mSampleRate;
		//XLOGD("getTime, mPreGranulePosition %lld\n", mPreGranulePosition);
		//XLOGD("getTime, timeUs %lld\n", timeUs);
	}
	return timeUs;
}

void OgmExtractor::OgmTrack::setSampleRate(int sampleRate)
{
	mSampleRate = sampleRate;
}

bool OgmExtractor::OgmTrack::isVideo()
{
	switch (mType) 
	{
		case OGV_VIDEO_AVC:
		case OGV_VIDEO_MP4:
		case OGV_VIDEO_MPEG:
		case OGV_VIDEO_VC1:
		case OGV_VIDEO_DIVX3:
			return true; 

		default:
			return false;
	}
}
bool OgmExtractor::OgmTrack::isAudio()
{
	switch (mType) 
	{
		case OGA_AUDIO_MP3:
		case OGA_AUDIO_VORBIS:
		case OGA_AUDIO_AAC:
		case OGA_AUDIO_WMA:
		case OGA_AUDIO_MPG:
#ifdef MTK_AUDIO_RAW_SUPPORT
		case OGA_AUDIO_PCM:
#endif
			return true;

		default:
			return false;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OgmExtractor::OgmExtractor(const sp<DataSource> &source):
mSearchingTracks(false),
mDataSource(source),
mDataSourceSize(0),
mDuration(0),
mOffset(0),
mFirstDataPageOffset(0),
mCurrentPageSize(0),
mNextLaceIndex(0),
mNeedDequeuePacket(true),
mIsVideoSeeking(false),
mHasVideoTrack(false),
mHasUnsupportedVideoTrack(false),
mAverageByteRate(0)
{
	mDataSource->getSize(&mDataSourceSize);
	initExtractor();	//To get track info
	if(mOgmTracks.size())
	{
		parseMaxTime();	//To get whole file play time
		signalDiscontinuity();	//Clear Track ES-Queue data and AUs
	}
	//seekToOffset(0, 0);
	seekToOffset(mFirstDataPageOffset, 0);
	XLOGD("Source size= 0x%llx, 1st data offset=0x%x\n", mDataSourceSize, mFirstDataPageOffset);
	XLOGD("OgmExtractor initial done, tracks num = %d, has Video = %d\n", mOgmTracks.size(), mHasVideoTrack);
}

OgmExtractor::~OgmExtractor()
{
	mOgmTracks.clear();
}

size_t OgmExtractor::countTracks()
{
	XLOGD("Extractor return countTrackers = %d\n", mOgmTracks.size());
	return mOgmTracks.size();
}

sp<MediaSource> OgmExtractor::getTrack(size_t index)
{
	XLOGD("Extractor return getTracker index = %d\n", index);
	if(index < mOgmTracks.size())
	{
		return mOgmTracks.valueAt(index);
	}
	return NULL;
}

sp<MetaData> OgmExtractor::getTrackMetaData(size_t index, uint32_t flags)
{
	XLOGD("Extractor return getTrackerMetaData index = %d\n", index);
	if(index < mOgmTracks.size())
	{
		return mOgmTracks.valueAt(index)->getFormat();
	}
	return NULL;
}

sp<MetaData> OgmExtractor::getMetaData()
{
	sp<MetaData> meta = new MetaData;
	meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_OGM);	 
	meta->setInt64(kKeyDuration, getDuration());   //Need to Enable Seek Feature
	meta->setInt64(kKeyThumbnailTime, 0);

	if(mHasUnsupportedVideoTrack)
	{
		meta->setInt32(kKeyHasUnsupportVideo, true);
		ALOGD("OGM has unsupport video track");
	}
	return meta;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
status_t OgmExtractor::initExtractor()
{
	int numPageParsed = 0;
	size_t index = 0;

	//Seek to file start position to get and parse headers of video, audio, etc...
	seekToOffset(0, 0);
	mSearchingTracks = true;
	while((dequeuePacket() == OK) && (numPageParsed++ < 20));
	mSearchingTracks = false;

	//Collect Tracks' Data for creating ES Format
	seekToOffset(mFirstDataPageOffset, 0);
	while((dequeuePacket() == OK) && (numPageParsed++ < 70));  // ALPS00439151


	//Check if get Audio / Video Stream Format Data or not
	while(index < mOgmTracks.size())
	{
		if(mOgmTracks.valueAt(index)->getFormat() != NULL)
		{
			if(mOgmTracks.valueAt(index)->isVideo())
			{
				mHasVideoTrack = true;
			}
			index ++;
		}
		else
		{
			if(mOgmTracks.valueAt(index)->isVideo())
			{
				mHasUnsupportedVideoTrack = true;
			}
			XLOGD("Track index %d has no format, remove it\n", index);
			mOgmTracks.removeItemsAt(index);
		}
	}
	return OK;
}

status_t OgmExtractor::verifyHeader(MediaBuffer *buffer)
{
	const uint8_t *data = (const uint8_t *)buffer->data()+buffer->range_offset();
	size_t size = buffer->range_length();
	status_t result;

	//XLOGD("Verify Header %02X %02X %02X %02X %02X %02X\n", data[0], data[1], data[2], data[3], data[4], data[5]);

	if(!memcmp(&data[1], "vorbis", 6))
	{
		XLOGD("Verify Vorbis Header, 0x%x\n", mOffset);
		result = verifyVorbisHeader(data, size);
	}
	else if(!memcmp(&data[1], "theora", 6))
	{
		//result = verifyTheoraHeader();
		XLOGD("************* theora header??? **************\n");
	}
	else if((!memcmp(&data[0], "fishead", 7)) || (!memcmp(&data[0], "fisbone", 7)))
	{
		//result = verifySkeletonHeader();
		XLOGD("************* Skeleton header??? **************\n");
	}
	else if(data[0] == 1)	
	{
		XLOGD("Verify Ogm Header, 0x%x\n", mOffset);
		result = verifyOgmHeader(data, size);
	}
	else if(data[0] == 3)
	{
		XLOGD("************* vorbis audio header in OGM header??? **************\n");
	}
	else
	{
		XLOGD("************* unknow header %02x %02x %02x %02x %02x %02x %02x %02x **************\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
		return ERROR_UNSUPPORTED;
	}

	return OK;
}


#define FOURCC_CTI(c1, c2, c3, c4)  ((c1 << 24) | (c2 << 16) | (c3<< 8) | c4)

OGMType OgmExtractor::GetFourCCTypeForHandler(const char *fourCC)
{
	uint32_t intFourCC;
	intFourCC = (fourCC[0] << 24) | (fourCC[1] << 16) | (fourCC[2] << 8) | fourCC[3];

	switch(intFourCC)
	{
		case FOURCC_CTI('a', 'v', 'c', '1'):
		case FOURCC_CTI('d', 'a', 'v', 'c'):
		case FOURCC_CTI('x', '2', '6', '4'):
		case FOURCC_CTI('v', 's', 's', 'h'):
		case FOURCC_CTI('h', '2', '6', '4'):
			XLOGD("Video - AVC Codec \n");
			return OGV_VIDEO_AVC;
		case FOURCC_CTI('3', 'I', 'V', '2'):
		case FOURCC_CTI('3', 'i', 'v', '2'):
		case FOURCC_CTI('B', 'L', 'Z', '0'):
		case FOURCC_CTI('D', 'I', 'G', 'I'):
		case FOURCC_CTI('D', 'I', 'V', '1'):
		case FOURCC_CTI('d', 'i', 'v', '1'):
		case FOURCC_CTI('D', 'I', 'V', 'X'):
		case FOURCC_CTI('d', 'i', 'v', 'x'):
		case FOURCC_CTI('D', 'X', '5', '0'):
		case FOURCC_CTI('d', 'x', '5', '0'):
		case FOURCC_CTI('D', 'X', 'G', 'M'):
		case FOURCC_CTI('E', 'M', '4', 'A'):
		case FOURCC_CTI('E', 'P', 'H', 'V'):
		case FOURCC_CTI('F', 'M', 'P', '4'):
		case FOURCC_CTI('f', 'm', 'p', '4'):
		case FOURCC_CTI('F', 'V', 'F', 'W'):
		case FOURCC_CTI('H', 'D', 'X', '4'):
		case FOURCC_CTI('h', 'd', 'x', '4'):
		case FOURCC_CTI('M', '4', 'C', 'C'):
		case FOURCC_CTI('M', '4', 'S', '2'):
		case FOURCC_CTI('m', '4', 's', '2'):
		case FOURCC_CTI('M', 'P', '4', 'S'):
		case FOURCC_CTI('m', 'p', '4', 's'):
		case FOURCC_CTI('M', 'P', '4', 'V'):
		case FOURCC_CTI('m', 'p', '4', 'v'):
		case FOURCC_CTI('M', 'V', 'X', 'M'):
		case FOURCC_CTI('R', 'M', 'P', '4'):
		case FOURCC_CTI('S', 'E', 'D', 'G'):
		case FOURCC_CTI('S', 'M', 'P', '4'):
		case FOURCC_CTI('U', 'M', 'P', '4'):
		case FOURCC_CTI('W', 'V', '1', 'F'):
		case FOURCC_CTI('X', 'V', 'I', 'D'):
		case FOURCC_CTI('X', 'v', 'i', 'D'):
		case FOURCC_CTI('x', 'v', 'i', 'd'):
		case FOURCC_CTI('X', 'V', 'I', 'X'):
			XLOGD("Video - MP4 Codec \n");
			return OGV_VIDEO_MP4;
		case FOURCC_CTI('D', 'I', 'V', '3'):
		case FOURCC_CTI('d', 'i', 'v', '3'):
			XLOGD("Video - DIVX3 Codec \n");			
			return OGV_VIDEO_DIVX3;
		case FOURCC_CTI('m', 'p', 'e', 'g'):
		case FOURCC_CTI('m', 'p', 'g', '1'):
		case FOURCC_CTI('m', 'p', 'g', '2'):
		case FOURCC_CTI('M', 'P', 'E', 'G'):
		case FOURCC_CTI('E', 'M', '2', 'V'):
		//case FOURCC_CTI('P', 'I', 'M', '1'):
		//case FOURCC_CTI('V', 'C', 'R', '2'):
		//case FOURCC_CTI('D', 'V', 'R', ' '):
			XLOGD("Video - MPEG Codec \n");
			return OGV_VIDEO_MPEG;

		/* need to add ESQueue support
		case FOURCC_CTI('W', 'M', 'V', '3'):
		case FOURCC_CTI('W', 'V', 'C', '1'):
			XLOGD("Video - VC1 Codec \n");
			return OGV_VIDEO_VC1;
		*/
		default:
			return OGM_OTHER;
	}
}

status_t OgmExtractor::verifyVorbisHeader(const uint8_t *data, size_t size)
{
	uint8_t type = data[0];
	OGMType currentSreamType = OGM_OTHER;
	ssize_t index = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);
	int finalRate;

	if(index >= 0)
	{
		XLOGD("************ Serial Number %d already in Tracks[%d] ************\n", mCurrentPage.mSerialNo, index);
		return OK;
	}

	//Start to verify Vorbis header info
	if(!memcmp(&data[1], "vorbis", 6))
	{
		switch(type)
		{
			case 1:
				uint8_t vorbisAudioChannels;
				uint32_t vorbisSampleRate;
				uint32_t vorbisBitrateMax;
				uint32_t vorbisBitrateMin;
				uint32_t vorbisBitrateNominal;
				data += 7;	// Skip Type and vorbis syncword
				data += 4;	// Skip Vorbis Version
				vorbisAudioChannels = data[0];
				data += 1;
				vorbisSampleRate = U32LE_AT(data);
				data += 4;
				vorbisBitrateMax = U32LE_AT(data);
				data += 4;
				vorbisBitrateNominal = U32LE_AT(data);
				data += 4;
				vorbisBitrateMin = U32LE_AT(data);
				data += 4;
				data += 2;	// useless size field  
				//finalRate = (vorbisBitrateNominal == 0)?((vorbisBitrateMax + vorbisBitrateMin) / 2) : vorbisBitrateNominal;
				finalRate = vorbisSampleRate;
				XLOGD("************ Vorbis Identification header ************\n");
				XLOGD("Audio Channels = %d\n", vorbisAudioChannels);
				XLOGD("SampleRate = %d\n", vorbisSampleRate);
				XLOGD("Bitrate = %d\n", vorbisBitrateNominal);
				currentSreamType = OGA_AUDIO_VORBIS;
				break;

			case 3:
				XLOGD("************ Vorbis comment header ************\n");
				break;
			case 5:
				XLOGD("************ Vorbis setup header ************\n");
				break;
			default:
				XLOGD("************ Vorbis unknow header ************\n");
				break;
		}

		if(currentSreamType != OGM_OTHER)
		{
			//sp<OgmTrack> OgmTrackSource = new OgmTrack(this, mCurrentPage.mSerialNo, currentSreamType);
			//if(OgmTrackSource != NULL)
			//{
			//	XLOGD("Add stream type %d into Tracks size = %d\n", currentSreamType, mOgmTracks.size());
			//	index = mOgmTracks.add(mCurrentPage.mSerialNo, OgmTrackSource);
			//	XLOGD("Add stream type %d into Tracks[%d]\n", currentSreamType, index);
			//	mOgmTracks.valueAt(index)->setSampleRate(finalRate);	//PANDA
			//}
			//else
			//{
			//	XLOGD("Add stream type %d, OgmTrackSource created fail\n", currentSreamType);
			//}
			index = mOgmTracks.add(mCurrentPage.mSerialNo, new OgmTrack(this, mCurrentPage.mSerialNo, currentSreamType));
			mOgmTracks.valueAt(index)->setSampleRate(finalRate * 1000);
			XLOGD("Add stream type %d, into Tracks[%d], SampleRate = %d\n", currentSreamType, index, finalRate);
		}
	}
	return OK;
}

status_t OgmExtractor::verifyOgmHeader(const uint8_t *data, size_t size)
{
	bool isAudio = false; 
	bool isVideo = false;
	bool isText = false;
	uint64_t time_unit;
	uint64_t spu;
	int width, height;
	uint64_t time_base_den, time_base_num; 
	int BitsPerSample, bit_rate, blockalign;    
	char   macFourCC[4];
	int finalRate;

	OGMType currentSreamType;
	ssize_t index;
	index = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);

	if(index >= 0)
	{
		XLOGD("************ Serial Number %d already in Tracks[%d] ************\n", mCurrentPage.mSerialNo, index);
		return OK;
	}

	if(*data == 1)
	{
		data++; 
		if (!memcmp(&data[0], "video", 5))
		{
			data += 8; 
			isVideo = true; 

			//Keep Sub-Fourcc (sub_type)
			memcpy(macFourCC, data, 4);
			data += 4;

			currentSreamType = GetFourCCTypeForHandler(macFourCC);

			if(currentSreamType == OGM_OTHER)
			{
				XLOGD("Video - Other Codec: %02x %02x %02x %02x \n", macFourCC[0], macFourCC[1], macFourCC[2], macFourCC[3]);
				mHasUnsupportedVideoTrack = true;
				isVideo = false; 
			}

			/*
			if (( !memcmp(macFourCC, "avc1", 4) ) || ( !memcmp(macFourCC, "h264", 4) ))
			{
				XLOGD("Video - AVC Codec \n");
				currentSreamType = OGV_VIDEO_AVC;
			}
			else if( !memcmp(macFourCC, "DIVX", 4) )
			{
				
			}
			else	//Other Codec
			{
				XLOGD("Video - Other Codec: %02x %02x %02x %02x \n", macFourCC[0], macFourCC[1], macFourCC[2], macFourCC[3]);
				isVideo = false; 
				currentSreamType = OTHER;
			}
			*/
		}
		else if (!memcmp(&data[0], "text", 4))
		{
			isText=true;
			data += 12;
			currentSreamType = OGM_OTHER;
		}
		else if (!memcmp(&data[0], "audio", 5))
		{
			data += 8;
			isAudio = true;

			//Keep Sub-Fourcc
			memcpy(macFourCC, data, 4);
			data += 4;

			if (!memcmp(macFourCC, "00ff", 4) )
			{
				XLOGD("Audio - AAC Codec \n");
				currentSreamType = OGA_AUDIO_AAC;
			}
			else if (!memcmp(macFourCC, "0055", 4))
			{
				XLOGD("Audio - MP3 Codec \n");
				currentSreamType = OGA_AUDIO_MP3;
			}
			else if (!memcmp(macFourCC, "566F", 4))
			{
				XLOGD("Audio - Vorbis Codec, Not Support \n");
				isAudio = false;
				currentSreamType = OGA_AUDIO_VORBIS;
			}
			else if (!memcmp(macFourCC, "0050", 4))
			{
				XLOGD("Audio - Mpeg Layer-1/2\n");
				currentSreamType = OGA_AUDIO_MPG;
			}
#ifdef MTK_AUDIO_RAW_SUPPORT
			else if (!memcmp(macFourCC, "0001", 4))
			{
				XLOGD("Audio - WAV format PCM Codec\n");
				currentSreamType = OGA_AUDIO_PCM;
			}
#endif
			/* need to add ESQueue support
			else if (!memcmp(macFourCC, "2000", 4))
			{
				XLOGD("Audio - AC3\n");
				currentSreamType = OGA_AUDIO_AC3;
			}
			else if (!memcmp(macFourCC, "2001", 4))
			{
				XLOGD("Audio - DTS\n");
				currentSreamType = OGA_AUDIO_DTS;
			}*/
			else
			{
				XLOGD("Audio - Other Codec: %02x %02x %02x %02x \n", macFourCC[0], macFourCC[1], macFourCC[2], macFourCC[3]);
				isAudio = false;
				currentSreamType = OGM_OTHER;
			}
		}
		else 
		{
			return ERROR_MALFORMED;
		}

		data += 4;	// useless size field  
		time_unit = U64LE_AT(data);
		data += 8;
		spu = U64LE_AT(data);	//samples per unit
		data += 8;
		data += 4;	// default_len
		data += 4;	// buffersize
		BitsPerSample = U32LE_AT(data);
		data += 4;

		if(currentSreamType != OGM_OTHER)
		{
			//sp<OgmTrack> OgmTrackSource = new OgmTrack(this, mCurrentPage.mSerialNo, currentSreamType);
			//if(OgmTrackSource != NULL)
			//{
			//	XLOGD("Add stream type %d into Tracks size = %d\n", currentSreamType, mOgmTracks.size());
			//	index = mOgmTracks.add(mCurrentPage.mSerialNo, OgmTrackSource);
			//	XLOGD("Add stream type %d into Tracks[%d]\n", currentSreamType, index);
			//}
			//else
			//{
			//	XLOGD("Add stream type %d, OgmTrackSource created fail\n", currentSreamType);
			//}
			index = mOgmTracks.add(mCurrentPage.mSerialNo, new OgmTrack(this, mCurrentPage.mSerialNo, currentSreamType));
			XLOGD("Add stream type %d into Tracks[%d]\n", currentSreamType, index);
		}

		if(isVideo && index >=0)
		{   
			width = U32LE_AT(data);
			data += 4;
			height = U32LE_AT(data);
			data += 4;
			time_base_den = spu * 10000000;
			time_base_num = time_unit;
			finalRate = (int)(time_base_den * 1000 / time_base_num);	//frame rate
			//mOgmTracks.valueAt(index)->mMetaData->setInt32(kKeyWidth, width);
			//mOgmTracks.valueAt(index)->mMetaData->setInt32(kKeyHeight, height);
			mOgmTracks.valueAt(index)->mVideoWidth = width;
			mOgmTracks.valueAt(index)->mVideoHeight = height;
			mOgmTracks.valueAt(index)->setSampleRate(finalRate);
			XLOGD("Set FrameRate %d, W/H=%d/%d for Tracks[%d]\n", finalRate, width, height, index);
		} 
		else if(isAudio && index >=0)
		{  
			mOgmTracks.valueAt(index)->mChannels = U16LE_AT(data);
			data += 2;
			blockalign = U16LE_AT(data);
			data += 2;	// block_align 
			bit_rate = U32LE_AT(data) * 8;
			data += 4;
			finalRate = spu * 10000000 / time_unit;	//Sample Rate
			mOgmTracks.valueAt(index)->setSampleRate(finalRate * 1000);
			mOgmTracks.valueAt(index)->mBitsPerSample = BitsPerSample;
			XLOGD("Set SampleRate %d for Tracks[%d]\n", finalRate, index);
		}
	}

	return OK;
}

//////////////////////////////////////////////////////////////////////////////////////
//	findPage(off_t startOffset, off_t *pageOffset, off_t searchLength)
//	startOffset: the start offset for searching OGM Page start signature "oggs"
//	pageOffset: he offset of a Page start
//////////////////////////////////////////////////////////////////////////////////////
int64_t OgmgetTickCountMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec*1000LL + tv.tv_usec/1000);
}


status_t OgmExtractor::findPage(off_t startOffset, off_t *pageOffset, off_t searchLength)
{
	char signature[4];
	char data[4196];
	int data_ptr = 0, data_sz = 0;
	int64_t timeS, timeE;
	
	*pageOffset = startOffset;

	timeS = OgmgetTickCountMs();
	memset(data, 0, sizeof(data));
	while(true)
	{
		//if searchLength <= 0, keep searching page start
		if((searchLength > 0) && (*pageOffset - startOffset >= searchLength))
		{
			timeE = OgmgetTickCountMs();
			XLOGD("Skip %ld bytes of junk from %ld still cannot reach page start, %lld ms\n", searchLength, startOffset, timeE - timeS);
			//*pageOffset = 0;
			return ERROR_UNSUPPORTED;
		}
		
		//read in buffer first instead of read 4 bytes at one time to improve performance
		if((*pageOffset - startOffset) && (data_ptr == 0))
		{
			data_sz = data_ptr = mDataSource->readAt(*pageOffset, data, sizeof(data));
		}

		if(data_ptr > 0)
		{
			if(!memcmp(data + (data_sz - data_ptr), "OggS", 4))
			{
				if(*pageOffset - startOffset)
				{					
					timeE = OgmgetTickCountMs();
					XLOGD("** Skip %ld bytes of junk from %ld to reach page start,%lld ms (%d)***\n", 
						*pageOffset - startOffset, startOffset, timeE - timeS, (data_sz - data_ptr));
				}
				return OK;
			}
			else
			{
				data_ptr--;
				++*pageOffset;
			}
		}
		else if(mDataSource->readAt(*pageOffset, signature, 4) < 4)
		{
			timeE = OgmgetTickCountMs();
			XLOGD("Reach file end, scan %d bytes, %lld ms\n", *pageOffset - startOffset, timeE - timeS);

			//*pageOffset = 0;
			return ERROR_END_OF_STREAM;
		}
		else if(!memcmp(signature, "OggS", 4))	//Find page start
		{
			if(*pageOffset - startOffset)
			{
				timeE = OgmgetTickCountMs();
				XLOGD("**** Skip %ld bytes of junk from %ld to reach page start,%lld ms ***\n", *pageOffset - startOffset, startOffset, timeE - timeS);
			}
			return OK;
		}
		else
		{
			++*pageOffset;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//	readPage(off_t pageOffset, Page *page)
//	pageOffset: the start offset of a Page
//////////////////////////////////////////////////////////////////////////////////////
ssize_t OgmExtractor::readPage(off_t pageOffset, Page *page)
{
	uint8_t header[27];
	if(mDataSource->readAt(pageOffset, header, sizeof(header)) < sizeof(header))
	{
		XLOGD("failed to read %d bytesat offset 0x%08lx\n", sizeof(header), pageOffset);
		return ERROR_IO;
	}

	if(memcmp(header, "OggS", 4))
	{
		XLOGD("Page Starts without signatures\n");
		return ERROR_MALFORMED;
	}

	if(header[4] != 0)
	{
		XLOGD("Version Error, verison should be 0 but %X\n", header[4]);
		return ERROR_UNSUPPORTED;
	}

	if(header[5] & ~7)
	{
		XLOGD("Flag only bits 0-2 are defined in version 0\n");
		return ERROR_MALFORMED;
	}
	page->mFlags = header[5];
	page->mGranulePosition = U64LE_AT(&header[6]);
	page->mSerialNo = U32LE_AT(&header[14]);
	page->mPageNo = U32LE_AT(&header[18]);
	page->mNumSegments = header[26];

	if(mDataSource->readAt(pageOffset + sizeof(header), page->mLace, page->mNumSegments) < page->mNumSegments)
	{
		XLOGD("failed to read segments lace length data\n");
		return ERROR_IO;
	}

	size_t totalSize, index;
	totalSize = sizeof(header) + page->mNumSegments;
	page->mSamples = 0;

	//XLOGD("Page info - %d-%2X, %X, %d\n", page->mSerialNo, page->mPageNo, page->mFlags, page->mGranulePosition);
	//XLOGD("Page info - %d Segments\n", page->mNumSegments);
	
	for(index = 0; index < page->mNumSegments; index++)
	{
		totalSize += page->mLace[index];
		if(page->mLace[index] < 255)
		{
			//XLOGD("Page info - [%d] Segment leng not 255\n", index);
			page->mSamples += 1;
		}
	}
	//XLOGD("Page info - Samples %d\n", page->mSamples);
	return totalSize;
}


//////////////////////////////////////////////////////////////////////////////////////
//	dequeuePacket()
//	read out current mOffset Page payload data out to collect a packet
//	The following variables should be prepared when enter this function
//	mCurrentPageSize
//	mCurrentPage
//	mNextLaceIndex
//	mOffset
//////////////////////////////////////////////////////////////////////////////////////
status_t OgmExtractor::dequeuePacket()
{
	MediaBuffer *buffer = NULL;
	size_t segIndex, index;
	size_t packetSize = 0;
	bool newPacketStart = false, fgIsKeyFrame = false;
	//bool newPage = false;
	off_t dataOffset;

	Mutex::Autolock autoLock(mExtractorLock);

	//If current Page payload is already handled, find next page and read it
	if(mNextLaceIndex == mCurrentPage.mNumSegments)
	{
		off_t pageOffset;
		mOffset += mCurrentPageSize;
		if(findPage(mOffset, &pageOffset, 0) != OK)
		{
			XLOGD("Cannot find a page start, from 0x%08X\n", mOffset);
			mOffset = pageOffset;
			return ERROR_END_OF_STREAM;
		}
		mCurrentPageSize = readPage(pageOffset, &mCurrentPage);
		if(mCurrentPageSize <=0 )
		{
			XLOGD("Cannot read a page header, at 0x%08X\n", pageOffset);
			return ERROR_END_OF_STREAM;
		}
		mOffset = pageOffset;
		mNextLaceIndex = 0;
	}

	//For checking current Page is in Tracks or not
	ssize_t tarckIndex = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);

	//If this page is new one, need to update GranulePosition later.
	if((mNextLaceIndex == 0) && (tarckIndex >= 0) && !mSearchingTracks)
	{
		//newPage = true;
		mOgmTracks.valueAt(tarckIndex)->setGranulePosition(mCurrentPage.mGranulePosition);
	}

	//Skip handling Data
	bool fgIsWaitingKeyFrm = (tarckIndex >= 0)?mOgmTracks.valueAt(tarckIndex)->mWaitingVideoKeyFramePacket:false;
	if(!getDequeueState() && (!fgIsWaitingKeyFrm))
	{
		mNextLaceIndex = mCurrentPage.mNumSegments;
		return OK;
	}
	//There is data need to collect....
	else if(mNextLaceIndex < mCurrentPage.mNumSegments)
	{
		//Caculate the packet payload data size
		for(segIndex = mNextLaceIndex; segIndex < mCurrentPage.mNumSegments; segIndex++)
		{
			packetSize += mCurrentPage.mLace[segIndex];
			if(mCurrentPage.mLace[segIndex] < 255)
			{
				segIndex ++;
				break;
			}
		}
		if((mNextLaceIndex != 0) || ((mCurrentPage.mFlags & 0x1) == 0x00))
		{
			newPacketStart = true;
		}

		//Caculate the packet payload data start offset
		dataOffset = mOffset + 27 + mCurrentPage.mNumSegments;
		for (index = 0; index < mNextLaceIndex; index++) {
			dataOffset += mCurrentPage.mLace[index];           
		}
		if(dataOffset > mDataSourceSize)
		{
			return ERROR_END_OF_STREAM;
		}
		
		if(fgIsWaitingKeyFrm)
		{
			packetSize = 4;// only needs to check key frame at byte0 bit 3, read 4 bytes for log
		}
		buffer = new MediaBuffer(packetSize);
		buffer->set_range(0, 0);
		if(mDataSource->readAt(dataOffset, (uint8_t *)buffer->data(), packetSize) != packetSize)
		{
			XLOGD("Cannot read page payload, dataOffset 0x%x/0x%x, size %d/%d\n", mOffset, dataOffset, packetSize, buffer->size());
			buffer->release();
			return ERROR_IO;
		}

		if(newPacketStart && (tarckIndex >= 0))
		{
			fgIsKeyFrame = ((*(uint8_t *)buffer->data()) & 0x8);
			
			if(mOgmTracks.valueAt(tarckIndex)->isVideo())
			{
				XLOGV("bufferOffset at 0x%x, %x %x %x %x, %d\n", 
					dataOffset, *(uint8_t *)buffer->data(), *((uint8_t *)buffer->data()+1), 
					*((uint8_t *)buffer->data()+2), *((uint8_t *)buffer->data()+3),
					fgIsWaitingKeyFrm);
				if(fgIsKeyFrame)
				{
					int64_t i8KeyInterval = mOgmTracks.valueAt(tarckIndex)->getTime() - mOgmTracks.valueAt(tarckIndex)->mKeyPositionTime;
					XLOGV("Key Frame, %.2f secs, diff %.2f secs\n", mOgmTracks.valueAt(tarckIndex)->getTime()/1E6, i8KeyInterval);
					mOgmTracks.valueAt(tarckIndex)->mKeyPositionTime = mOgmTracks.valueAt(tarckIndex)->getTime();
					if(i8KeyInterval > mOgmTracks.valueAt(tarckIndex)->mMaxKeyInterval)
					{
						mOgmTracks.valueAt(tarckIndex)->mMaxKeyInterval = i8KeyInterval;
						XLOGD("Max Key Interval, %.2f secs\n", i8KeyInterval/1E6);
					}
				}
			}
			if(fgIsWaitingKeyFrm)
			{
				fgIsWaitingKeyFrm = fgIsKeyFrame?false:true;
				
				if(!fgIsWaitingKeyFrm)
				{
					XLOGD("Find key frame\n");
					mOgmTracks.valueAt(tarckIndex)->mWaitingVideoKeyFramePacket = false;
					mNextLaceIndex = 0; //re-do current page after getDequeueState()=on
					return OK;
				}
			}
		}
		buffer->set_range(0, packetSize);
		mNextLaceIndex = segIndex;	
	}
	else
	{
		return OK;	//The new Page have no packet data.....
	}
	//Skip handling Data after collect mIsVideoKeyFramePacket, keep mNextLaceIndex rolling
	if(!getDequeueState())	//video track
	{
		return OK;
	}

	//Check if need to add track info
	//ssize_t tarckIndex = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);
	if(mSearchingTracks)
	{
		//Verify Tracks' header, and try to find first data page
		if(verifyHeader(buffer) == OK)
		{
			//mFirstDataPageOffset = 0;

			//For Vorbis
			//Vorbis ES-Queue need header info to create format
			tarckIndex = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);
			if(tarckIndex >= 0 && (mOgmTracks.valueAt(tarckIndex)->mType == OGA_AUDIO_VORBIS))
			{
				mOgmTracks.valueAt(tarckIndex)->setGranulePosition(mCurrentPage.mGranulePosition);
				mOgmTracks.valueAt(tarckIndex)->appendData(buffer, newPacketStart);
			}
		}
		else if(mFirstDataPageOffset == 0)
		{
			mFirstDataPageOffset = mOffset;
			XLOGD("Get First Data Page, Offset %d\n", mFirstDataPageOffset);
		}
		//tarckIndex = mOgmTracks.indexOfKey(mCurrentPage.mSerialNo);
	}
	//Append packet data into Track ES-Queue
	else if(tarckIndex >= 0)
	{
		//if(newPage)
		//{
		//	mOgmTracks.valueAt(tarckIndex)->setGranulePosition(mCurrentPage.mGranulePosition);
		//}
		//if(getDequeueState())
		//{
			mOgmTracks.valueAt(tarckIndex)->appendData(buffer, newPacketStart);
		//}
	}
	buffer->release();
	return OK;
}

//////////////////////////////////////////////////////////////////////////////////////
//	seekToOffset(off_t offset)
//	will find a page start, start from offset and set Extractor::mOffset to it
//////////////////////////////////////////////////////////////////////////////////////
status_t OgmExtractor::seekToOffset(off_t offset, off_t searchSize)
{
	off_t pageOffset;
	bool ifKeepFinding = false;

	Mutex::Autolock autoLock(mExtractorLock);

	status_t result;

	do{
		result = findPage(offset, &pageOffset, searchSize);
		if(result != OK)
		{
			if(result == ERROR_END_OF_STREAM)
			{
				XLOGD("Seek to file end\n");
				pageOffset = mDataSourceSize;	//move the offset to file end
				break;
			}
			XLOGD("Cannot find a page start, from 0x%08X in size %d\n", offset, searchSize);
			return result;
		}
		offset = pageOffset;
		mCurrentPageSize = readPage(pageOffset, &mCurrentPage);
		if(mCurrentPageSize <=0 )
		{
			XLOGD("Cannot read a page header, at 0x%08X\n", pageOffset);
			return ERROR_END_OF_STREAM;
		}
		////////////////////////////////////////////////////////////////
		//	Coding in Feature.....
		////////////////////////////////////////////////////////////////
		//if(ifSeek)
		//{
		//	if(mCurrentPage.mFlags != 0x01)
		//	{
		//		ifKeepFinding = false;
		//	}
		//	else
		//	{
		//		ifKeepFinding = true;
		//		offset += mCurrentPageSize;
		//	}
		//}
		////////////////////////////////////////////////////////////////
	}while(ifKeepFinding);
	mOffset = pageOffset;
	mNextLaceIndex = 0;
	clearGranulePosition();
	return result;
}

status_t OgmExtractor::seekToTime(const int64_t timeUs)
{
	off_t searchSize = mDataSourceSize / 1024;	//1204 Search Point ?
	off64_t searchOffset = 0, minOffset = 0, maxOffset = mDataSourceSize;
	bool ifSeeking = true;
	int64_t seekTimeUs;

	if((timeUs < OGMSEEKTHRESHOLD) || (mDuration == 0))
	{
		searchOffset = 0;
		signalDiscontinuity();
		//seekToOffset(0, 0);
		seekToOffset(mFirstDataPageOffset, 0);
		return OK;
	}
	else if(mDuration && (mDuration - timeUs < 200000))
	{
		searchOffset = mDataSourceSize;
		signalDiscontinuity();
		seekToOffset(mDataSourceSize, 0);
		return OK;
	}
	else
	{
		int64_t seekTargetUs = timeUs - OGMSEEKTHRESHOLD;
		int64_t lastSeekTimeUs = 0, i8Diff = 0;
		off64_t u8MaxOffset = mDataSourceSize, u8MinOffset = 0;
		bool fgFinalSequentialSearch = false;
		int32_t u4SeekTryCnt = 0;
		signalDiscontinuity();
		setDequeueState(false);

		searchOffset = seekTargetUs * mAverageByteRate/1000000;//mFirstDataPageOffset;
		do{
			lastSeekTimeUs = getCurTime();
		seekToOffset(searchOffset, 0);
		seekTimeUs = 0;
		
		while(dequeuePacket() == OK)
		{
			seekTimeUs = getCurTime();
				if((!fgFinalSequentialSearch && (seekTimeUs != lastSeekTimeUs) && (seekTimeUs > 0)) ||
			   		(fgFinalSequentialSearch && (seekTimeUs >= seekTargetUs)))
				{
					break;
				}
			}
			
			XLOGD("seekTimeUs/Target/Last=%lld/%lld/%lld,Offset Cur/Min/Max=%lld/%lld/%lld, Cnt/Final %d/%d\n", 
				seekTimeUs, seekTargetUs, lastSeekTimeUs,
				searchOffset, u8MinOffset, u8MaxOffset,
				u4SeekTryCnt++, fgFinalSequentialSearch);
			
			i8Diff = seekTimeUs - seekTargetUs;
			if(fgFinalSequentialSearch || (i8Diff > -OGMSEEKTOLERENCE && i8Diff < OGMSEEKTOLERENCE)) // diff less than 500ms
			{
				break;
			}
			//update range
			if((seekTimeUs < seekTargetUs) && (searchOffset > u8MinOffset))
			{
				u8MinOffset = searchOffset;
			}
			else if((seekTimeUs > seekTargetUs) && (searchOffset < u8MaxOffset))
			{
				u8MaxOffset = searchOffset;
			}
			// set next searchOffset
			// sequentialSearch if the range is less than 1MB
			if(u8MaxOffset - u8MinOffset < 100000) // 1MB
			{
				searchOffset = u8MinOffset;
				fgFinalSequentialSearch = true;
			}
			else if(u8MaxOffset == mDataSourceSize || u8MinOffset == 0)
			{
				searchOffset += (seekTargetUs - seekTimeUs) * mAverageByteRate /(1000000>>1);
				if((searchOffset >=mDataSourceSize) || (searchOffset <=0))
				{
					if(searchOffset >=mDataSourceSize)
					{
						u8MaxOffset = mDataSourceSize - 1;
					}
					if(searchOffset <=0)
					{
						u8MinOffset = 1;
					}
					searchOffset = mDataSourceSize >>1;
				}
		}
			else
			{
				searchOffset = (u8MinOffset + u8MaxOffset)>>1;
			}
		}while((seekTimeUs != seekTargetUs) || (u8MinOffset != u8MaxOffset));

		//seekToOffset(searchOffset, 0);
		setDequeueState(true);
		return OK;
	}
	/*
	{
		uint8_t seekCnt = 0;
		int64_t seekTargetUs = timeUs - OGMSEEKTHRESHOLD;
		int64_t lastSeekTimeUs = 0;

		signalDiscontinuity();
		setDequeueState(false);

		XLOGD("[ Seeking ] Target Time %lld us\n", timeUs);
		while(ifSeeking)
		{
			//Caculate new offset to hit
			searchOffset = (minOffset + maxOffset)/2;
			seekToOffset(searchOffset, 0);
			seekTimeUs = 0;

			//XLOGD("Seeking Time at 0x%X\n", searchOffset);
			//Get Time from guested offset
			while(dequeuePacket() == OK)
			{
				seekTimeUs = getCurTime();
				if(seekTimeUs)
				{
					XLOGD("[ Seeking ] Find Time %lld\n", seekTimeUs);
					if(lastSeekTimeUs == seekTimeUs)
					{
						ifSeeking = false;
						XLOGD("[ Seeking ] Seeking get same Time, times: %d\n", seekCnt);
					}
					lastSeekTimeUs = seekTimeUs;
					break;
				}
			}

			//See if hit
			if(seekTimeUs)
			{
				if(seekTimeUs > seekTargetUs)
				{
					if(seekTimeUs - seekTargetUs < 50000)	//Hit tartget time + 0.05sec
					{
						ifSeeking = false;
						XLOGD("[ Seeking ] Done at %lld, times: %d\n", seekTimeUs, seekCnt);
					}
					else
					{
						maxOffset = searchOffset + searchSize;
						
					}
				}
				else if(seekTimeUs <= seekTargetUs)
				{
					if(seekTargetUs - seekTimeUs < 100000)	//Hit target time - 0.1 sec
					{
						ifSeeking = false;
						XLOGD("[ Seeking ] Done at %lld, times: %d\n", seekTimeUs, seekCnt);
					}
					else
					{
						minOffset = searchOffset + searchSize;
					}
				}
			}
			else
			{
				XLOGD("[ Seeking ] Cannot find a offset to time %lld, jump to end\n", timeUs);
				//seekToOffset(mDataSourceSize, 0);
				searchOffset = mDataSourceSize;
				break;
			}

			if(maxOffset <= minOffset)
			{
				XLOGD("[ Seeking ] Seeking Range over\n");
				ifSeeking = false;
			}

			if(seekCnt++ > 13)
			{
				XLOGD("[ Seeking ] Seeking Time over\n");
				ifSeeking = false;
			}
		}
		seekToOffset(searchOffset, 0);
		setDequeueState(true);
		return OK;
	}
	*/
}


//////////////////////////////////////////////////////////////////////////////////////
//	signalDiscontinuity()
//	Clean all Tracks' ESQueue data and Access Units
//////////////////////////////////////////////////////////////////////////////////////
void OgmExtractor::signalDiscontinuity()
{
	size_t index;
	for(index = 0; index < mOgmTracks.size(); index ++)
	{
		mOgmTracks.valueAt(index)->signalDiscontinuity();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//	clearGranulePosition()
//	Clean all Tracks' GranulePosition information
//////////////////////////////////////////////////////////////////////////////////////
void OgmExtractor::clearGranulePosition()
{
	size_t index;
	for(index = 0; index < mOgmTracks.size(); index ++)
	{
		mOgmTracks.valueAt(index)->clearGranulePosition();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//	getCurTime()
//	take tracks Current Sample Time and return Max one
//////////////////////////////////////////////////////////////////////////////////////
int64_t OgmExtractor::getCurTime()
{
	int64_t maxTime = 0;
	size_t index;
	for(index = 0; index < mOgmTracks.size(); index ++)
	{
		int64_t time = mOgmTracks.valueAt(index)->getTime();

		if(time == 0)
		{
			return 0;
		}
		if( maxTime < time )
		{
			maxTime = time;
		}
	}
	return maxTime;
}

//////////////////////////////////////////////////////////////////////////////////////
//	getMaxTime()
//	take tracks Time and return Max one
//////////////////////////////////////////////////////////////////////////////////////
int64_t OgmExtractor::getMaxTime(bool isAllTracksCollected)
{
	int64_t maxTime = 0;
	size_t index;
	for(index = 0; index < mOgmTracks.size(); index ++)
	{
		int64_t time = mOgmTracks.valueAt(index)->getMaxTime();

		if((time == 0) && isAllTracksCollected)
		{
			ALOGD("Track index %d MaxTime=0\n", index);
			return 0;
		}
		if( maxTime < time )
		{
			maxTime = time;
		}
	}
	return maxTime;
}

//////////////////////////////////////////////////////////////////////////////////////
//	parseMaxTime()
//	to get the duration time info
//////////////////////////////////////////////////////////////////////////////////////
void OgmExtractor::parseMaxTime()
{
	off_t searchSize = 4096;
	off_t searchOffset = 0;
	off_t searchOffsetMax;
	uint8_t u1MaxTryCnt = 10, u1MaxPosTryCnt = 4;
	bool fgFindBackward = true;
	
	setDequeueState(false);
	searchOffset = mDataSourceSize;
	searchOffsetMax = mDataSourceSize;
	while(mDuration == 0 && (searchOffset != 0) && (u1MaxPosTryCnt > 0))
	{
		if(searchOffset > searchSize)
		{
			if(fgFindBackward)
			{
				searchOffset = searchOffset -searchSize;
			}else
			{
				searchOffset = searchOffset +searchSize;
			}
		}
		else
		{
			searchOffset = 0;
		}
		XLOGD("parseMaxTime from 0x%8X, u1MaxTryCnt %d/%d\n", searchOffset, u1MaxTryCnt, u1MaxPosTryCnt);
		if(seekToOffset(searchOffset, searchSize) == OK)
		{
		//Parsing Page info to get Max Time
		while((dequeuePacket() == OK) && (mOffset < searchOffsetMax));
		mDuration = getMaxTime((u1MaxTryCnt==0 || u1MaxPosTryCnt==1)?false:true);
		if(mDuration)
		{
			XLOGD("Get Duration = %lld us(%.2f secs)", mDuration, mDuration/1E6);
			mAverageByteRate = mDataSourceSize * 1000000 / mDuration;
			break;
			}
		}
		u1MaxTryCnt--;

		searchOffsetMax = searchOffset;
		if(u1MaxTryCnt == 0)
		{
			searchOffset -= (searchOffset > 0x100000)?0x100000:searchOffset; // 1 MB
			u1MaxTryCnt = 1;
			u1MaxPosTryCnt--;
			fgFindBackward = false;  // try to find in the range of searchOffset to searchOffsetMax
			XLOGD("Reset searchOffset as 0x%8X\n", searchOffset);
		}
	}
	setDequeueState(true);
}

//////////////////////////////////////////////////////////////////////////////////////
//	getDuration()
//	return mDuration, it should be the Max TimeUs in whole file
//////////////////////////////////////////////////////////////////////////////////////
int64_t OgmExtractor::getDuration()
{
	return mDuration;
}

//////////////////////////////////////////////////////////////////////////////////////
//	setDequeueState()
//	mNeedDequeuePacket:
//		true: the Packet Payload will be appended into ES Queue of related Track
//		false: the Packet Payload will not be appended into ES Queue of related Track, but still parsing data
//////////////////////////////////////////////////////////////////////////////////////
void OgmExtractor::setDequeueState(bool needDequeuePacket)
{
	mNeedDequeuePacket = needDequeuePacket;
}

bool OgmExtractor::getDequeueState()
{
	return mNeedDequeuePacket;
}

//////////////////////////////////////////////////////////////////////////////////////
//	setSeekingState()
//	mIsVideoSeeking:
//		true: the Audio Packet Payload will NOT be appended into ES Queue of related Track when seeking
//////////////////////////////////////////////////////////////////////////////////////
void OgmExtractor::setSeekingState(bool isVideoSeeking)
{
	mIsVideoSeeking = isVideoSeeking;
}

bool OgmExtractor::getSeekingState()
{
	return mIsVideoSeeking;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t searchOgmSignature(uint8_t *data, uint32_t datalength) 
{
	uint32_t dataOffset = 0;

	for (;;) 
	{        
		if (!memcmp(&data[dataOffset], "OggS", 4)) 
		{
			return dataOffset;
		}
		dataOffset++;       
		if (dataOffset >= (datalength - 4))
		{
			XLOGD("search Ogm signature fail\n");
			return 0xFFFFFFFF;
		}
	}
}

bool  parseOgmHeader(uint8_t *data, uint32_t datalength)
{
	bool bchecktesult = false;
	uint32_t dataOffset = 0;
	uint32_t searchoffset = 0;
	uint32_t uOgmHdrSkipLength = 26;
	uint8_t* dataptr;

	for (;;)
	{
		//Check Video Ogm Packet
		if (memcmp(data, "OggS", 4)) 
		{
			searchoffset = searchOgmSignature(data+dataOffset, datalength-dataOffset);
			if (searchoffset == 0xFFFFFFFF)
			{
				bchecktesult = false;
				return bchecktesult;
			}
		}

		//Check Video Ogm Packet
		dataOffset = dataOffset + searchoffset+uOgmHdrSkipLength;
		dataptr = data + dataOffset;

		if (dataOffset >= (datalength-4))
		{
			bchecktesult = false;
			return bchecktesult;
		}

		if ((dataptr[0] == 0x1) && (dataptr[2] == 0x1) && (!memcmp(&dataptr[3], "video", 5)))
		{
			XLOGD("Find Video pattern\n");
			bchecktesult = true;
			return bchecktesult;
		}

		dataOffset = dataOffset + 2 + dataptr[1];
	}

	return bchecktesult;  
}

bool SniffOgm(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *)
{
	uint8_t readbuff[1024];
	memset(readbuff,0,1024);
	int length = source->readAt(0, readbuff, 1024);

	XLOGD("SniffOgm ...\n");
	if (length < 4) 
	{
		return false;
	}

	if (parseOgmHeader(readbuff, 1024) == false)
	{
		return false;
	}

	XLOGD("SniffOgm OK\n");
	mimeType->setTo(MEDIA_MIMETYPE_CONTAINER_OGM);
	*confidence = 0.5f; 
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

