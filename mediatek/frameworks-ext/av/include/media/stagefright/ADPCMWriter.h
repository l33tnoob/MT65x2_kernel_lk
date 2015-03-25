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

#ifndef _ADPCM_WRITER_H_
#define _ADPCM_WRITER_H_

#include <media/stagefright/MediaWriter.h>
#include <utils/threads.h>

namespace android
{

struct MediaSource;
struct MetaData;

#define CHUNK_ID_RIFF 0x46464952
#define CHUNK_ID_WAVE 0x45564157
#define CHUNK_ID_FMT  0x20746d66
#define CHUNK_ID_FACT 0x74636166
#define CHUNK_ID_DATA 0x61746164

typedef unsigned long long int  ADPCM_U64;
typedef unsigned int ADPCM_U32;
typedef unsigned short int ADPCM_U16;

typedef enum { 
   ADPCM_FALSE        = 0,
   ADPCM_TRUE     = 1
} ADPCM_BOOL;

#define MS_ADPCM      2
#define DVI_IMAADCPM  17

typedef struct adpcm_header {
	int       riff_id;
	ADPCM_U32 riff_sz;
	int       riff_fmt;
	int       fmt_id;
	ADPCM_U32 fmt_sz;
	ADPCM_U16 audio_format;
	ADPCM_U16 num_channels;
	ADPCM_U32 sample_rate;
	ADPCM_U32 byte_rate;       /* sample_rate * num_channels * bps / 8 */
	ADPCM_U16 block_align;     /* num_channels * bps / 8 */
	ADPCM_U16 bits_per_sample;
	ADPCM_U16 extra_data_size;
	void*     pextraData;
	int       fact_id;
	ADPCM_U32 fact_sz;
	ADPCM_U32 samples_per_channel;
	int data_id;
	ADPCM_U32 data_sz;
}adpcm_header;

struct ADPCMWriter : public MediaWriter
{
	public:
		ADPCMWriter(const char* filePath);
		ADPCMWriter(int fd);

		ADPCM_BOOL initCheck();
			
		//override the following functions for a specific media writer
		virtual status_t addSource(const sp<MediaSource> &source);
    	virtual bool reachedEOS();
    	virtual status_t start(MetaData *params = NULL);
    	virtual status_t stop();
    	virtual status_t pause();

		

	protected:
		virtual ~ADPCMWriter();

	private:
		ADPCM_BOOL reachedFileSizeLimit();
		ADPCM_BOOL reachedFileDurationLimit();
		void writeWordToFile(short data);
		void writeDWordToFile(int data);
		void writeDataToFile(char* data, ADPCM_U32 length);
		void moveFilePointer(long offset, int fromwhere);
		status_t threadFunc();
		static void* threadWrapper(void *);
		ADPCMWriter(const ADPCMWriter &);
		ADPCMWriter &operator=(const ADPCMWriter &);
		
		FILE* mFile;
		pthread_t mThread;
		ADPCM_BOOL mInitChecked;
		ADPCM_BOOL mStarted;
		ADPCM_BOOL mStoped;
		ADPCM_BOOL mPaused;
		ADPCM_BOOL mResumed;
		ADPCM_BOOL mReachedEOS;
		ADPCM_U64 mEstimatedSizeBytes; 
		ADPCM_U64 mEstimatedDurationUs; 
		ADPCM_U32 mSamplesPerBlock; 
		ADPCM_U64 mRecordFileSize;

		sp<MediaSource> mSource;
		ADPCM_U16 mAudioFormat;
		adpcm_header mWaveHeader;	
};
} //end of namespace
#endif
