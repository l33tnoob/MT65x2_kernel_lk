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
 *   MtkAVIExtractor.cpp
 *
 * Project:
 * --------
 *   MT6575
 *
 * Description:
 * ------------
 *   AVI Extractor implementation
 *
 * Note:
 *   This code only works on little endian arch
 *
 * Author:
 * -------
 *   Demon Deng (mtk80976)
 *
 ****************************************************************************/
 
#define MTK_AVI_USE_XLOG
//#define LOG_NDEBUG 0
#undef LOG_TO_FILE
//#define LOG_TO_FILE
#undef MTK_AVI_TIMING_PARSE_CHUNK
//#define MTK_AVI_TIMING_PARSE_CHUNK
#define MTK_AVI_SUPPORT_B_FRAME
#define MTK_AVI_SUPPORT_FIX_SYNC_FRAME
#define LOG_TAG "MtkAVIExtractor"
#include <utils/Log.h>

#ifdef MTK_AVI_USE_XLOG
#include <cutils/xlog.h>
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#define LOGE XLOGE
#define LOGW XLOGW
#define LOGI XLOGI
#define LOGD XLOGD
#define LOGV XLOGV
#endif

#include "include/MtkAVIExtractor.h"

#include <arpa/inet.h>

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/String8.h>
#include "include/avc_utils.h"

#ifdef MTK_AVI_TIMING_PARSE_CHUNK
#include <media/stagefright/foundation/ALooper.h>
#endif

namespace android {

static const int32_t kAVIMaxRIFFSize = 0x40000000LL; // 1G bytes for one riff
static const int32_t kSizeOfChunkHeader = 8;
static const int32_t kSizeOfListHeader = 12;
static const int32_t kSizeOfSkipHeader = 8;

static const int32_t kKeyAVIIFList = 0x00000001;
static const int32_t kKeyAVIIFKeyFrame = 0x00000010;
static const int32_t kKeyAVIIFNoTime = 0x00000100;

static const int32_t kKeyAVIIndexOfIndexes = 0x00;
static const int32_t kKeyAVIIndexOfChunks = 0x01;
static const int32_t kKeyAVIIndexIsData = 0x80;
static const int32_t kKeyAVIIndex2Field = 0x01;

static const uint32_t kMP3HeaderMask = 0xfffe0c00;//0xfffe0cc0 add by zhihui zhang no consider channel mode
static const int32_t kKeyMP3NoHeader = 1;
static const int32_t kKeyMP3OneFrame = 2;

static const int8_t kKeySyncFixedMask = 0x80;
static const int32_t kKeyAVIUnknownFrame = 0;
static const int32_t kKeyAVIIFrame = 1;
static const int32_t kKeyAVIPFrame = 2;
static const int32_t kKeyAVIBFrame = 3;

static const uint32_t kAVIMaxEmptyChunks = 1000;
#if 0
static int containerAACSampleRate = 0;
static int containerAACChannelNumber = 0;
static bool isRawAACData = false;
#endif
static int32_t keepBitRate = 0;

#define FORMATFOURCC "0x%08x:%c%c%c%c"
#define PRINTFOURCC(x) x,((uint8_t*)&x)[0],((uint8_t*)&x)[1],((uint8_t*)&x)[2],((uint8_t*)&x)[3]

// big endian fourcc
#define BFOURCC(c1, c2, c3, c4) \
    (c4 << 24 | c3 << 16 | c2 << 8 | c1)
// size of chunk should be WORD align
#define EVEN(i) (uint32_t)((i) + ((i) & 1))

struct riffList {
    int32_t ID;
    int32_t size;
    int32_t type;
};

struct MtkAVISample {
    uint32_t offset;
    uint32_t size;
    uint8_t isSyncSample;
};

struct aviMainHeader {
    uint32_t microSecPerFrame;
    uint32_t maxBytesPerSec;
    uint32_t paddingGranularity;
    uint32_t flags;
    uint32_t totalFrames;
    uint32_t initialFrames;
    uint32_t streams;
    uint32_t suggestedBufferSize;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[4];
};
static const int32_t kSizeOfAVIMainHeader = 56;

struct aviStreamHeader {
    uint32_t fccType;
    uint32_t fccHandler;
    uint32_t flags;
    uint16_t priority;
    uint16_t language;
    uint32_t initialFrames;
    uint32_t scale;
    uint32_t rate;
    uint32_t start;
    uint32_t length;
    uint32_t suggestedBufferSize;
    uint32_t quality;
    uint32_t sampleSize;
    struct {
        short int left;
        short int top;
        short int right;
        short int bottom;
    } rcFrame;
};
// rcFrame is not provided by some files
static const int32_t kSizeOfAVIStreamHeader = 48;

struct bitmapInfo {
    uint32_t size; 
    uint32_t width; 
    uint32_t height; 
    uint16_t planes; 
    uint16_t bitCount;
    uint32_t compression; 
    uint32_t sizeImage; 
    uint32_t xPelsPerMeter; 
    uint32_t yPelsPerMeter; 
    uint32_t clrUsed; 
    uint32_t clrImportant; 
    // may be more datas
};
static const int32_t kSizeOfBitmapInfo = 40;

struct waveFormatEx { 
    uint16_t formatTag; 
    uint16_t nChannels; 
    uint32_t nSamplesPerSec; 
    uint32_t nAvgBytesPerSec; 
    uint16_t nBlockAlign; 
    uint16_t bitsPerSample; 
    uint16_t size; // for PCM, no this member
    uint8_t *data; 
}; 
static const int32_t kSizeOfWaveFormatEx = 16;

struct aviOldIndexEntry {
    uint32_t ID;
    uint32_t flags;
    uint32_t offset;
    uint32_t size;
};
static const int32_t kSizeOfAVIOldIndexEntry = 16;

struct MtkAVIIndexChunk {
    uint16_t longsPerEntry;
    uint8_t indexSubType;
    uint8_t indexType;
    uint32_t entriesInUse;
    uint32_t ID;
    // split offset to two int32_t to avoid 8 bytes padding
    uint32_t baseOffset;
    uint32_t baseOffsetHigh;
    uint32_t reserved3;
};
static const int32_t kSizeOfAVIIndexChunk = 24;

struct aviStdIndexEntry {
    uint32_t offset;
    uint32_t size;
};
static const int32_t kSizeOfAVIStdIndexEntry = 8;

struct aviSuperIndexEntry {
    uint64_t offset;
    uint32_t size;
    uint32_t duration;
};
static const int32_t kSizeOfAVISuperIndexEntry = 16;

#define RIFF_WAVE_FORMAT_PCM            (0x0001)
#define RIFF_WAVE_FORMAT_ALAW           (0x0006)
#define RIFF_WAVE_FORMAT_MULAW          (0x0007)
#define RIFF_WAVE_FORMAT_MPEGL12        (0x0050)
#define RIFF_WAVE_FORMAT_MPEGL3         (0x0055)
#define RIFF_WAVE_FORMAT_AMR_NB         (0x0057)
#define RIFF_WAVE_FORMAT_AMR_WB         (0x0058)
#define RIFF_WAVE_FORMAT_AAC            (0x00ff)
#define RIFF_IBM_FORMAT_MULAW           (0x0101)
#define RIFF_IBM_FORMAT_ALAW            (0x0102)
#define RIFF_WAVE_FORMAT_WMAV1          (0x0160)
#define RIFF_WAVE_FORMAT_WMAV2          (0x0161)
#define RIFF_WAVE_FORMAT_WMAV3          (0x0162)
#define RIFF_WAVE_FORMAT_WMAV3_L        (0x0163)
#define RIFF_WAVE_FORMAT_AAC_AC         (0x4143)
#define RIFF_WAVE_FORMAT_VORBIS         (0x566f)
#define RIFF_WAVE_FORMAT_VORBIS1        (0x674f)
#define RIFF_WAVE_FORMAT_VORBIS2        (0x6750)
#define RIFF_WAVE_FORMAT_VORBIS3        (0x6751)
#define RIFF_WAVE_FORMAT_VORBIS1PLUS    (0x676f)
#define RIFF_WAVE_FORMAT_VORBIS2PLUS    (0x6770)
#define RIFF_WAVE_FORMAT_VORBIS3PLUS    (0x6771)
#define RIFF_WAVE_FORMAT_AAC_pm         (0x706d)
#define RIFF_WAVE_FORMAT_GSM_AMR_CBR    (0x7A21)
#define RIFF_WAVE_FORMAT_GSM_AMR_VBR    (0x7A22)

#define RIFF_WAVE_FORMAT_AC3             (0x2000)
#define RIFF_WAVE_FORMAT_DTS             (0x2001)

#define RIFF_WAVE_FORMAT_ADPCM_MS        (0x0002)
#define RIFF_WAVE_FORMAT_ADPCM_IMA       (0x0011)
#define MAX_MP3_FRAMESIZE  1441 //bytes

static const int32_t kMaxBitrateH264 = 4000000ll;
static const int32_t kMaxBitrateMPEG4 = 4000000ll;

static const int32_t kMaxMacroPerSecond = 108000ll;//LEVEL 3.1

static const int32_t kMaxVideoWidthHeight = 1280;
static const int32_t kMaxVideoDim = 1280*720;

// copy from MP3Extractor
static bool get_mp3_frame_size(
        uint32_t header, size_t *frame_size,
        int *out_sampling_rate = NULL, int *out_channels = NULL,
        int *out_bitrate = NULL,int *out_sampleperframe=NULL) {
    *frame_size = 0;
    int sampleperframe=0;
    if (out_sampling_rate) {
        *out_sampling_rate = 0;
    }

    if (out_channels) {
        *out_channels = 0;
    }

    if (out_bitrate) {
        *out_bitrate = 0;
    }

    if ((header & 0xffe00000) != 0xffe00000) {
        return false;
    }

    unsigned version = (header >> 19) & 3;

    if (version == 0x01) {
        return false;
    }

    unsigned layer = (header >> 17) & 3;

    if (layer == 0x00) {
        return false;
    }// else if(layer == 2){
	   //   return false;
    //}
    else if(layer == 3){
	      LOGV("Layer I");
	      return false;
    }

    //add by zhihui zhang for mp2 framesize calculate 
    if(layer== 2 ||(version == 3 && layer==1)){
	    sampleperframe=1152;
    }else if(layer==3){
         sampleperframe=384;
    }
    else if((version == 2 || version==0) && layer == 1){
         sampleperframe=576;
    }
    if(out_sampleperframe!=NULL){
        *out_sampleperframe=sampleperframe;
    }

    unsigned protection = (header >> 16) & 1;

    unsigned bitrate_index = (header >> 12) & 0x0f;

    if (bitrate_index == 0 || bitrate_index == 0x0f) {
        // Disallow "free" bitrate.
        return false;
    }

    unsigned sampling_rate_index = (header >> 10) & 3;

    if (sampling_rate_index == 3) {
        return false;
    }

    static const int kSamplingRateV1[] = { 44100, 48000, 32000 };
    int sampling_rate = kSamplingRateV1[sampling_rate_index];
    if (version == 2 /* V2 */) {
        sampling_rate /= 2;
    } else if (version == 0 /* V2.5 */) {
        sampling_rate /= 4;
    }

    unsigned padding = (header >> 9) & 1;

    if (layer == 3) {
        // layer I

        static const int kBitrateV1[] = {
            32, 64, 96, 128, 160, 192, 224, 256,
            288, 320, 352, 384, 416, 448
        };

        static const int kBitrateV2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            144, 160, 176, 192, 224, 256
        };

        int bitrate =
            (version == 3 /* V1 */)
                ? kBitrateV1[bitrate_index - 1]
                : kBitrateV2[bitrate_index - 1];

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        *frame_size = (12000 * bitrate / sampling_rate + padding) * 4;
    } else {
        // layer II or III

        static const int kBitrateV1L2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            160, 192, 224, 256, 320, 384
        };

        static const int kBitrateV1L3[] = {
            32, 40, 48, 56, 64, 80, 96, 112,
            128, 160, 192, 224, 256, 320
        };

        static const int kBitrateV2[] = {
            8, 16, 24, 32, 40, 48, 56, 64,
            80, 96, 112, 128, 144, 160
        };

        int bitrate;
        if (version == 3 /* V1 */) {
            bitrate = (layer == 2 /* L2 */)
                ? kBitrateV1L2[bitrate_index - 1]
                : kBitrateV1L3[bitrate_index - 1];
        } else {
            // V2 (or 2.5)

            bitrate = kBitrateV2[bitrate_index - 1];
        }

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        if (version == 3 /* V1 */) {
            //*frame_size = 144000 * bitrate / sampling_rate + padding;
            *frame_size = (sampleperframe*125) * bitrate / sampling_rate + padding;
        } else {
            // V2 or V2.5
            //*frame_size = 72000 * bitrate / sampling_rate + padding;
		 *frame_size = (sampleperframe*125) * bitrate / sampling_rate + padding;
        }
    }

    if (out_sampling_rate) {
        *out_sampling_rate = sampling_rate;
    }

    if (out_channels) {
        int channel_mode = (header >> 6) & 3;

        *out_channels = (channel_mode == 3) ? 1 : 2;
    }

    return true;
}

static inline size_t chID2streamID(uint32_t id) {
    return ((id & 0xff) - '0') * 10 + ((id >> 8) & 0xff) - '0';
}

static inline size_t ixID2streamID(uint32_t id) {
    return chID2streamID(id >> 16);
}

static const char *wave2MIME(uint32_t id) {
    switch (id) {
        case  RIFF_WAVE_FORMAT_AMR_NB:
        case  RIFF_WAVE_FORMAT_GSM_AMR_CBR:
        case  RIFF_WAVE_FORMAT_GSM_AMR_VBR:
            return MEDIA_MIMETYPE_AUDIO_AMR_NB;

        case  RIFF_WAVE_FORMAT_AMR_WB:
            return MEDIA_MIMETYPE_AUDIO_AMR_WB;

        case  RIFF_WAVE_FORMAT_AAC:
        case  RIFF_WAVE_FORMAT_AAC_AC:
        case  RIFF_WAVE_FORMAT_AAC_pm:       
            return MEDIA_MIMETYPE_AUDIO_AAC;

        case  RIFF_WAVE_FORMAT_VORBIS:
        case  RIFF_WAVE_FORMAT_VORBIS1:
        case  RIFF_WAVE_FORMAT_VORBIS2:        
        case  RIFF_WAVE_FORMAT_VORBIS3:
        case  RIFF_WAVE_FORMAT_VORBIS1PLUS:
        case  RIFF_WAVE_FORMAT_VORBIS2PLUS:
        case  RIFF_WAVE_FORMAT_VORBIS3PLUS:
            return MEDIA_MIMETYPE_AUDIO_VORBIS;

        case  RIFF_WAVE_FORMAT_MPEGL12:
        case  RIFF_WAVE_FORMAT_MPEGL3:
            return MEDIA_MIMETYPE_AUDIO_MPEG;

        case RIFF_WAVE_FORMAT_MULAW:
        case RIFF_IBM_FORMAT_MULAW:
            return MEDIA_MIMETYPE_AUDIO_G711_MLAW;

        case RIFF_WAVE_FORMAT_ALAW:
        case RIFF_IBM_FORMAT_ALAW:
            return MEDIA_MIMETYPE_AUDIO_G711_ALAW;

        case RIFF_WAVE_FORMAT_PCM:
            return MEDIA_MIMETYPE_AUDIO_RAW;

		case RIFF_WAVE_FORMAT_WMAV1:          
        case RIFF_WAVE_FORMAT_WMAV2:          
        case RIFF_WAVE_FORMAT_WMAV3:          
        case RIFF_WAVE_FORMAT_WMAV3_L:  
			return MEDIA_MIMETYPE_AUDIO_WMA;

		case RIFF_WAVE_FORMAT_ADPCM_MS:
			return MEDIA_MIMETYPE_AUDIO_MS_ADPCM;
        case RIFF_WAVE_FORMAT_ADPCM_IMA:
			return MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM;
			
#ifdef MTK_AUDIO_DDPLUS_SUPPORT
		case RIFF_WAVE_FORMAT_AC3:
			return MEDIA_MIMETYPE_AUDIO_AC3;
#endif			
        default:
            LOGW("unknown wave %x", id);
            return "";
    };
}
static bool IsNeedCodecData(uint32_t fourcc)
{
    switch (fourcc) {
        case BFOURCC('x', 'v', 'i', 'd'):
        case BFOURCC('X', 'V', 'I', 'D'):
        case BFOURCC('d', 'i', 'v', 'x'):
        case BFOURCC('D', 'I', 'V', 'X'):
        case BFOURCC('D', 'X', '5', '0'):
        case BFOURCC('d', 'x', '5', '0'):
        case BFOURCC('m', 'p', '4', 'v'):
        case BFOURCC('F', 'M', 'P', '4'):
        case BFOURCC('F', 'V', 'F', 'W'):  /*ffmpeg VFW*/
        case BFOURCC('f', 'v', 'f', 'w'):
			return true;

        case BFOURCC('D', 'I', 'V', '3'):  /*DivX 3.11*/
        case BFOURCC('d', 'i', 'v', '3'):
		case BFOURCC('D', 'I', 'V', '4'):
		case BFOURCC('d', 'i', 'v', '4'):
		case BFOURCC('D', 'I', 'V', '5'):
		case BFOURCC('d', 'i', 'v', '5'):
		case BFOURCC('D', 'I', 'V', '6'):
		case BFOURCC('d', 'i', 'v', '6'):
		case BFOURCC('M', 'P', '4', '3'):  /*Microsoft MPEG-4 v3*/
		case BFOURCC('m', 'p', '4', '3'):
		case BFOURCC('C', 'O', 'L', '1'):  /* cool codec*/
		case BFOURCC('c', 'o', 'l', '1'):
		case BFOURCC('A', 'P', '4', '1'):
		case BFOURCC('a', 'p', '4', '1'):  /*AngelPotion*/
		case BFOURCC('n', 'A', 'V', 'I'):  /*nAVI*/
			return false;
		case BFOURCC('F', 'L', 'V', '1'):  /*Sorenson FLV*/
		case BFOURCC('f', 'l', 'v', '1'):
            return true;

        case BFOURCC('m', 'p', 'g', '1'):
        case BFOURCC('M', 'P', 'G', '1'):
            return true;
			
		case BFOURCC('V', 'P', '6', 'F'):
		case BFOURCC('v', 'p', '6', 'f'):
			return false;
			
        case BFOURCC('m', 'p', 'g', '2'):
        case BFOURCC('M', 'P', 'G', '2'):
		case BFOURCC('m', 'p', 'e', 'g'):
		case BFOURCC('M', 'P', 'E', 'G'):
		case BFOURCC('m', 'p', 'g', 'v'):
		case BFOURCC('M', 'P', 'G', 'V'):
            return true;

        case BFOURCC('s', '2', '6', '3'):
        case BFOURCC('H', '2', '6', '3'):
        case BFOURCC('h', '2', '6', '3'):
            return false;

        case BFOURCC('a', 'v', 'c', '1'):
        case BFOURCC('A', 'V', 'C', '1'):
        case BFOURCC('H', '2', '6', '4'):
        case BFOURCC('h', '2', '6', '4'):
            return true;
			
#if 0 //megaa lin tell me not support wmv1/2
		case BFOURCC('w', 'm', 'v', '1'):
		case BFOURCC('W', 'M', 'V', '1'):
		case BFOURCC('w', 'm', 'v', '2'):
		case BFOURCC('W', 'M', 'V', '2'):
#endif			
		case BFOURCC('w', 'm', 'v', '3'):
		case BFOURCC('W', 'M', 'V', '3'):
		case BFOURCC('w', 'v', 'c', '1'):
		case BFOURCC('W', 'V', 'C', '1'):
            return true;

        case BFOURCC('m', 'j', 'p', 'g'):
        case BFOURCC('M', 'J', 'P', 'G'):
            LOGW("mjpeg fourcc " FORMATFOURCC, PRINTFOURCC(fourcc));
            return false;

        default:
            LOGW("unknown fourcc " FORMATFOURCC, PRINTFOURCC(fourcc));
            return true;
    }

}

static const char *BFourCC2MIME(uint32_t fourcc) {
    switch (fourcc) {
        case BFOURCC('m', 'p', '4', 'a'):
		case BFOURCC('M', 'P', '4', 'A'):
            return MEDIA_MIMETYPE_AUDIO_AAC;

        case BFOURCC('s', 'a', 'm', 'r'):
		case BFOURCC('S', 'A', 'M', 'R'):
            return MEDIA_MIMETYPE_AUDIO_AMR_NB;

        case BFOURCC('s', 'a', 'w', 'b'):
		case BFOURCC('S', 'A', 'W', 'B'):
            return MEDIA_MIMETYPE_AUDIO_AMR_WB;

        case BFOURCC('m', 'p', '4', 'v'):
		case BFOURCC('M', 'P', '4', 'V'):
		case BFOURCC('f', 'm', 'p', '4'):
        case BFOURCC('F', 'M', 'P', '4'):
            return MEDIA_MIMETYPE_VIDEO_MPEG4;
		case BFOURCC('D', 'I', 'V', '3'):  /*DivX 3.11*/
        case BFOURCC('d', 'i', 'v', '3'):
		case BFOURCC('D', 'I', 'V', '4'):
		case BFOURCC('d', 'i', 'v', '4'):
		case BFOURCC('D', 'I', 'V', '5'):
		case BFOURCC('d', 'i', 'v', '5'):
		case BFOURCC('D', 'I', 'V', '6'):
		case BFOURCC('d', 'i', 'v', '6'):
			return MEDIA_MIMETYPE_VIDEO_DIVX3;
	
		case BFOURCC('d', 'i', 'v', 'x'):
        case BFOURCC('D', 'I', 'V', 'X'):
        case BFOURCC('D', 'X', '5', '0'):
        case BFOURCC('d', 'x', '5', '0'):
			return MEDIA_MIMETYPE_VIDEO_DIVX;

		case BFOURCC('x', 'v', 'i', 'd'):
        case BFOURCC('X', 'V', 'I', 'D'):
			return MEDIA_MIMETYPE_VIDEO_XVID;

        case BFOURCC('s', '2', '6', '3'):
		case BFOURCC('S', '2', '6', '3'):
        case BFOURCC('H', '2', '6', '3'):
        case BFOURCC('h', '2', '6', '3'):
            return MEDIA_MIMETYPE_VIDEO_H263;

        case BFOURCC('a', 'v', 'c', '1'):
        case BFOURCC('A', 'V', 'C', '1'):
        case BFOURCC('H', '2', '6', '4'):
        case BFOURCC('h', '2', '6', '4'):
            return MEDIA_MIMETYPE_VIDEO_AVC;

        case BFOURCC('M', 'P', 'G', '2'):
        case BFOURCC('m', 'p', 'g', '2'):
        case BFOURCC('M', 'P', 'E', 'G'):
        case BFOURCC('m', 'p', 'e', 'g'):
        case BFOURCC('M', 'P', 'G', 'V'):
        case BFOURCC('m', 'p', 'g', 'v'):
			return MEDIA_MIMETYPE_VIDEO_MPEG2;
		case BFOURCC('m', 'j', 'p', 'g'):
	    case BFOURCC('M', 'J', 'P', 'G'):
			return MEDIA_MIMETYPE_VIDEO_MJPEG;
		case BFOURCC('f', 'l', 'v', '1'):
		case BFOURCC('F', 'L', 'V', '1'):
            return MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK;
        default:
            LOGW("unknown fourcc " FORMATFOURCC, PRINTFOURCC(fourcc));
            return "";
    }
}

class MtkAVISource : public MediaSource {
public:
    MtkAVISource(const sp<DataSource> &dataSource, int index);

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();

    virtual sp<MetaData> getFormat();

    virtual status_t read(
            MediaBuffer **buffer, const ReadOptions *options = NULL);

protected:
    virtual ~MtkAVISource();

private:
    status_t findMP3Header(int *pIndex, int *pOffset, int *pHeader) const;
    status_t updateSamples();
    status_t addSample(struct MtkAVISample *s);
    status_t clearSamples();
    status_t generateCodecData(bool full);
    status_t readNextChunk(uint8_t* data, int size, ssize_t& num_bytes_read, int offset = 0);

    bool isBFrame(const char* data, int size) const;
    bool isSyncFrame(const char* data, int size) const;
    int getFrameType(const char* data, int size, int *pOffset = NULL) const;
    int followingBFrames() const;
    bool fixSyncSample(int32_t index);


    friend class MtkAVIExtractor;
    Mutex mLock;

    sp<DataSource> mDataSource;
    sp<MetaData> mFormat;
    int mIndex;

    bool mIsVideo;
    bool mIsAudio;
    uint32_t mScale;
    uint32_t mRate; // mRate/mScale = fps
    uint32_t mLevel;// H264 level
    uint32_t mStartOffset;
    uint32_t mTotalSamples;
    uint32_t mSampleSize;
    uint32_t mMaxSampleSize;
    uint32_t mMaxSyncSampleSize;
    uint32_t mThumbNailIndex;
    bool includes_expensive_metadata;
    uint32_t mBitsPerSample;

    Vector<int32_t> mSampleOffsets;
    Vector<int32_t> mSampleSizes;
    Vector<int8_t> mSampleSyncs;
    Vector<int32_t> mSampleBlockSizes;

    sp<ABuffer> strdData;
    sp<ABuffer> strfData;

    uint32_t mCurrentSampleIndex;
    bool mStarted;

    int32_t mCompression;
    int32_t mMP3Header;
    int32_t mBlockAlign;
    int32_t mLastBufferOffset;
    int64_t mCurrentDTS;
    int64_t mCurrentPTSDelta;
    bool mBlockMode;
    bool mIsAVC;
	bool mIsDIVX;
	bool mIsDIVX3;
	bool mIsXVID;
    bool mWantsNALFragments;
    bool mIsADTS;
    bool mIsVorbis;
    bool mIsPCM;
    bool mIsMPEG4;
	bool mIsMJPG;
	bool mIsMPEG2;
	bool mIsS263;
	bool mNeedCodecData;
    bool mIsFirstBuffer;
    bool mINDXValid;
    bool mBrokenIndex;
    bool mIsMultiple;
#ifdef LOG_TO_FILE
    FILE *fp;
#endif
    /* variables below is for wma */
    uint16_t mformatTag;
    uint16_t mChannels;
    uint32_t mSamplesPerSec;
    uint32_t mAvgBytesPerSec;
    //mBlockAlign, mbitsPerSample;
    uint16_t mSize;

    MediaBufferGroup *mGroup;
    MediaBuffer *mBuffer;

    MtkAVISource(const MtkAVISource &);
    MtkAVISource &operator=(const MtkAVISource &);
};

MtkAVIExtractor::MtkAVIExtractor(const sp<DataSource> &source)
    : mDataSource(source),
      mHasMetadata(false),
      mHasVideo(false),
      mHasAudio(false),
      mHasIndex(false),
      mStopped(false),
      mFileSize(0),
      mInitCheck(NO_INIT),
      mFileMetaData(new MetaData) {
#ifndef ANDROID_DEFAULT_CODE // Demon Deng
	LOGI("====================================="); 
    LOGI("[AVI Playback capability info]"); 
    LOGI("====================================="); 
    LOGI("Resolution = \"[(8,8) ~ (1280,720)]\""); 
    LOGI("Support Codec = \"Video:MPEG4, H263, H264 ; Audio: PCM, AAC, AMR-NB/WB, MP3, MP2, Vorbis\""); 
    LOGI("Profile_Level = \"MPEG4: SP/ASP ; H263: Baseline ; H264: BL/MP*/HP* level 3.1\""); 
    LOGI("Max Bitrate  = 4Mbps"); 
    LOGI("=====================================\n"); 
#endif // #ifndef ANDROID_DEFAULT_CODE
    mFileMetaData->setInt32(kKeyVideoPreCheck, 1);
}

MtkAVIExtractor::~MtkAVIExtractor() {
}

sp<MetaData> MtkAVIExtractor::getMetaData() {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return new MetaData;
    }

    return mFileMetaData;
}

size_t MtkAVIExtractor::countTracks() {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return 0;
    }

    return mTracks.size();
}

sp<MetaData> MtkAVIExtractor::getTrackMetaData(
        size_t index, uint32_t flags) {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return NULL;
    }

    if (index >= mTracks.size())
        return NULL;

    sp<MtkAVISource> track = mTracks[index];
    if (track == NULL)
        return NULL;

    if ((flags & kIncludeExtensiveMetaData)
            && !track->includes_expensive_metadata) {
        track->includes_expensive_metadata = true;
        const char *mime;
        CHECK(track->getFormat()->findCString(kKeyMIMEType, &mime));
        if (!strncasecmp("video/", mime, 6)) {
            int64_t dts = (int64_t)track->mThumbNailIndex * 1000000LL 
                * track->mScale / track->mRate;
            track->getFormat()->setInt64(kKeyThumbnailTime, dts);
            LOGI("thumbnail index %d, time %lld", track->mThumbNailIndex, dts);
        }
    }

    return track->getFormat();
}

sp<MediaSource> MtkAVIExtractor::getTrack(size_t index) {
    status_t err;
    if ((err = readMetaData()) != OK) {
        return NULL;
    }

    if (index >= mTracks.size())
        return NULL;

    return mTracks[index];
}

status_t MtkAVIExtractor::addSample(size_t id, struct MtkAVISample *s) {
    if (id >= mTracks.size()) {
        LOGW("skip invalid index for chunk %d", id);
        return ERROR_UNSUPPORTED;
    }

    sp<MtkAVISource> source = mTracks[id];
    CHECK(source != NULL);

    return source->addSample(s);
}

status_t MtkAVIExtractor::parseDataChunk(int32_t pos, int ID, int size, int sync) {
	static uint64_t logtimes;
    uint32_t type = ID & 0xffff0000;
    int id = chID2streamID(ID);
    MtkAVISample s = {pos, size, 1};
    LOGV("ID " FORMATFOURCC " at pos 0x%08lx %d", PRINTFOURCC(ID), pos, gettid());

    if (size == 0 && ID == 0) {
        if (++mEmptyChunks >= kAVIMaxEmptyChunks) {
            LOGW("we parse %d empty chunks, stop", mEmptyChunks);
            return ERROR_UNSUPPORTED;
        }
    } else {
        mEmptyChunks = 0;
    }

    switch(type) {
        case BFOURCC(0,0,'d','c'):
        case BFOURCC(0,0,'w','b'):	 
            addSample(id, &s);
            break;
        case BFOURCC(0,0,'d','b'):
            //sometimes VOS is located in this area
            addSample(id, &s);
            LOGW("00db chunk " FORMATFOURCC, PRINTFOURCC(ID));
            break;
        case BFOURCC(0,0,'p','c'):
            LOGW("TODO support chunk " FORMATFOURCC, PRINTFOURCC(ID));
            break;
        default:       
            if(logtimes<5 || logtimes%100 == 0)
                LOGW("skip chunk at pos %d " FORMATFOURCC, pos, PRINTFOURCC(ID));
	     logtimes++;
	     return BAD_VALUE;
    }
    return OK;
}

status_t MtkAVIExtractor::parseMOVIMore(bool full) {
    for(size_t i=0; i<mTracks.size(); ++i) {
        sp<MtkAVISource> source = mTracks[i];
        source->clearSamples();
    }

    // TODO or NOT TODO?
    // if multiple avi with no or error index, should we parseMOVIMore??
    // Note: no err will be returned to make us more tolerant for bad file
    MtkAVIOffT pos = mMOVIOffset + kSizeOfListHeader;
    MtkAVIOffT oldEnd = pos + (ssize_t)EVEN(mMOVISize);
    status_t err = OK;
    mEmptyChunks = 0;

    LOGI("parse movi chunks to setup index from 0x%08x to 0x%08x full=%d", (int32_t)pos, (int32_t)oldEnd, full);

    if (oldEnd > mFileSize) {
        LOGW("chunked file, do our best to parse movi");
        oldEnd = mFileSize;
    }

#ifdef MTK_AVI_TIMING_PARSE_CHUNK
    int64_t t0 = 0, t1, tx = ALooper::GetNowUs();
#endif

    uint32_t ChunkScan_times = 0;
    while(pos < oldEnd) {
        if (mStopped) {
            LOGI("stopped by user");
            break;
        }

        if (full == false) {
            bool oneForEach = true;
            for(size_t i=0; i<mTracks.size(); ++i) {
                sp<MtkAVISource> source = mTracks[i];
                if (source->mSampleSizes.size() == 0) {
                    oneForEach = false;
                    break;
                }
            }
            if (oneForEach) {
                LOGI("found at lease one sample for each track");
                break;
            }
        }

        riffList list;
#ifdef MTK_AVI_TIMING_PARSE_CHUNK
        t1 = ALooper::GetNowUs();
#endif
        ssize_t s = mDataSource->readAt(pos, (char*)&list, sizeof(list));
#ifdef MTK_AVI_TIMING_PARSE_CHUNK
        t0 += ALooper::GetNowUs() - t1;
#endif
        if (s != kSizeOfListHeader) {
            if (s != kSizeOfChunkHeader || list.size != 0) {
                LOGE("failed to read header at 0x%08lx, stop parsing", pos);
            }
            break;
        }

        LOGV("ID " FORMATFOURCC " at pos 0x%08lx", PRINTFOURCC(list.ID), pos);
        
        err = OK;
        MtkAVIOffT end = pos + EVEN(list.size) + kSizeOfSkipHeader;
        if (end > oldEnd) {
            LOGW("skip last incomplete chunk/list at %lx-%lx", pos, end);
            break;
        }


        if (list.ID == BFOURCC('L','I','S','T')) {
            LOGV("type " FORMATFOURCC, PRINTFOURCC(list.type));
            if (list.type == BFOURCC('r', 'e', 'c', ' ')) {
                mEmptyChunks = 0;
                pos = pos + kSizeOfListHeader;
                while (pos < end) {
                    if (mStopped) {
                        LOGI("stopped by user");
                        break;
                    }

                    riffList chunk;
#ifdef MTK_AVI_TIMING_PARSE_CHUNK
                    t1 = ALooper::GetNowUs();
#endif
                    ssize_t s = mDataSource->readAt(pos, (char*)&chunk, sizeof(chunk));
#ifdef MTK_AVI_TIMING_PARSE_CHUNK
                    t0 += ALooper::GetNowUs() - t1;
#endif
                    if (s != kSizeOfListHeader && s != kSizeOfChunkHeader) {
                        LOGE("failed to read header at 0x%08lx", pos);
                        break;
                    }
                    pos += kSizeOfChunkHeader;
                    MtkAVIOffT chunkEnd = pos + EVEN(chunk.size);
                    if (chunkEnd > end) {
                        LOGW("skip last incomplete chunk in rec at %lx-%lx", pos, chunkEnd);
                        break;
                    }
                    err = parseDataChunk(pos, chunk.ID, chunk.size, 1);
                    if (err == ERROR_UNSUPPORTED) {
                        break;
                    }else if(BAD_VALUE == err){
                        ChunkScan_times ++;
                        if(500 == ChunkScan_times){
                            LOGW("unknown chunk num > 200, stop scaning!");
                            break;
                        }
	             }
                    pos = chunkEnd;
                }
            } else {
                LOGW("unknown list " FORMATFOURCC " in %s", PRINTFOURCC(list.type), __FUNCTION__);
            }
        } else {
            pos = pos + kSizeOfChunkHeader;
            err = parseDataChunk(pos, list.ID, list.size, 1);
	     if (err == ERROR_UNSUPPORTED) {
                break;
            }else if(BAD_VALUE == err){
                ChunkScan_times ++;
                if(500 == ChunkScan_times){
                    LOGW("unknown chunk num > 200, stop scaning!");
                    break;
                }
	     }		 	
        }

        pos = end;
    }
#ifdef MTK_AVI_TIMING_PARSE_CHUNK
    tx = ALooper::GetNowUs() - tx;
    LOGI("avi parse chunks cost %lld, read file using %lld us", tx, t0);
#endif
    if (full) {
        mHasIndex = true;
    }
    return OK;
}

status_t MtkAVIExtractor::parseINFO(MtkAVIOffT pos, MtkAVIOffT end) {
    struct riffList list;
    while(pos < end) {
        if (mDataSource->readAt(pos, (char*)&list, kSizeOfChunkHeader) != kSizeOfChunkHeader) {
            LOGW("parse INFO failed");
            // don't fail on INFO
            return OK;
        }

        pos += kSizeOfChunkHeader;
        uint32_t metadataKey = 0;
        uint32_t size = EVEN(list.size);
        switch(list.ID) {
            case BFOURCC('I','N','A','M'):
                metadataKey = kKeyTitle;
                break;
            case BFOURCC('I','C','M','T'):
                LOGI("kComment");
                break;
            case BFOURCC('I','C','O','P'):
                LOGI("kCopyright");
                break;
            case BFOURCC('I','P','R','D'):
                metadataKey = kKeyAlbum;
                break;
            case BFOURCC('I','A','R','T'):
                metadataKey = kKeyArtist;
                break;
            case BFOURCC('I','G','N','R'):
                metadataKey = kKeyGenre;
                break;
            case BFOURCC('I','C','R','D'):
                metadataKey = kKeyDate;
                break;
            default:
                LOGW("unknown meta key " FORMATFOURCC, PRINTFOURCC(list.ID));
                break;
        }

        if (metadataKey != 0) {
            LOGI("meta key " FORMATFOURCC, PRINTFOURCC(list.ID));

            if (size > 4096) {
                LOGW("skip abnormal meta data with %d bytes", size);
            } else {
                sp<ABuffer> value = new ABuffer(size+1);
                if (mDataSource->readAt(pos, value->data(), (size+1)) != (ssize_t)(size+1)) {
                    LOGW("read metadata error at %lx", pos);
                } else {
                    value->data()[size] = 0;
                    mFileMetaData->setCString(metadataKey, (const char*)value->data());
                    LOGI("meta value %s", value->data());
                }
            }
        }
        pos = pos + size;
    }
    return OK;
}

status_t MtkAVIExtractor::parseMOVI(MtkAVIOffT pos, MtkAVIOffT end) {
    mMOVIOffset = pos - kSizeOfListHeader;
    mMOVISize = end - pos;
    // TODO should we check movi?
    return OK;
}

status_t MtkAVIExtractor::parseAVIH(MtkAVIOffT pos, MtkAVIOffT end) {
    struct aviMainHeader header;
    ssize_t sz = kSizeOfAVIMainHeader;
    if (end - pos < sz) {
        LOGE("sizeof avih is not correct: %ld < %ld", end - pos, sz);
        return ERROR_MALFORMED;
    }

    ssize_t s = mDataSource->readAt(pos, (char*)&header, sz);
    if (s != sz) {
        LOGE("failed to read header at 0x%08lx", pos);
        return ERROR_IO;
    }

    // TODO parse more main header
    mWidth = header.width;
    mHeight = header.height;
    mNumTracks = header.streams;
    LOGI("width %d, height %d, tracks %d, padding %d", mWidth, mHeight, mNumTracks, header.paddingGranularity);
    return OK;
}

status_t MtkAVIExtractor::parseSTRL(MtkAVIOffT pos, MtkAVIOffT oldEnd, int index) {
    sp<MtkAVISource> source = new MtkAVISource(mDataSource, index);
    if (source == NULL)
        return -ENOMEM;
    mTracks.push(source);

    status_t err;
#ifdef MTK_AUDIO_CHANGE_SUPPORT
	status_t errSTRN;
#endif
    bool strhFound = false;
    bool strfFound = false;
    bool strdFound = false;

    while(pos < oldEnd) {
        riffList list;
        ssize_t s = mDataSource->readAt(pos, (char*)&list, sizeof(list));
        if (s != kSizeOfListHeader) {
            if (s != kSizeOfChunkHeader || list.size != 0) {
                LOGE("failed to read header at 0x%08lx", pos);
                return ERROR_IO;
            }
            break;
        }

        LOGV("ID " FORMATFOURCC " at pos 0x%08lx", PRINTFOURCC(list.ID), pos);
        
        err = OK;
        MtkAVIOffT end = pos + EVEN(list.size) + kSizeOfSkipHeader;

        if (list.ID == BFOURCC('L','I','S','T')) {
            LOGV("type " FORMATFOURCC, PRINTFOURCC(list.type));
            LOGW("unknown list " FORMATFOURCC " in %s", PRINTFOURCC(list.type), __FUNCTION__);
        } else {
            pos = pos + kSizeOfChunkHeader;
            switch(list.ID) {
                case BFOURCC('J','U','N','K'):
                    // skip JUNK
                    break;
                case BFOURCC('s','t','r','h'):
                    if (strhFound) {
                        LOGW("skip multiple strh");
                        break;
                    }
                    strhFound = true;
                    err = parseSTRH(pos, end, source);
                    break;
                case BFOURCC('s','t','r','f'):
                    if (strfFound) {
                        LOGW("skip multiple strf");
                        break;
                    }
                    if (!strhFound) {
                        LOGE("found strf before strh");
                        err = ERROR_MALFORMED;
                        break;
                    }
                    strfFound = true;
                    err = parseSTRF(pos, end, source);
                    break;
                case BFOURCC('s','t','r','d'):
                    if (strdFound) {
                        LOGW("skip multiple strd");
                        break;
                    }
                    if (!strhFound) {
                        LOGE("found strd before strh");
                        err = ERROR_MALFORMED;
                        break;
                    }
                    err = parseSTRD(pos, end, source);
                    break;
                case BFOURCC('v','p','r','p'):
                    if (!strfFound || !strhFound) {
                        LOGW("found vprp before strh or strf");
                        err = ERROR_MALFORMED;
                    }
                    err = parseVPRP(pos, end, source);
                    break;
                case BFOURCC('i','n','d','x'):
                    err = parseINDX(pos, end, source);
                    if (err != OK) {
                        // don't fail on bad indx
                        err = OK;
                        source->clearSamples();
                    } else {
                        source->mINDXValid = true;
                    }
                    break;
#ifdef MTK_AUDIO_CHANGE_SUPPORT
                case BFOURCC('s','t','r','n'):
                    LOGW("found strn");
                    errSTRN = parseSTRN(pos, end, source);
					if (errSTRN != OK)
						ALOGE("parse optional case STRN error, unable to support audio change");
                    break;
#endif
                default:
                    LOGW("unknown chunk " FORMATFOURCC " in %s", PRINTFOURCC(list.ID), __FUNCTION__);
            }
        }

        if (err != OK)
            return err;
        pos = end;
    }

    if (pos > oldEnd || !strhFound || !strfFound) {
        LOGE("strl of stream %d seems to be corrupted: end at 0x%08lx, "
                "expected end at 0x%08lx, strh=%d, strf=%d",
                index, pos, oldEnd, strhFound, strfFound);
        return ERROR_MALFORMED;
    }
    return OK;
}

status_t MtkAVIExtractor::parseVPRP(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source) {
    // TODO parse vprp
    LOGI("vprp found");
    return OK;
}

status_t MtkAVIExtractor::parseChunkIndex(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source, 
        struct MtkAVIIndexChunk* pHeader) {
    LOGV("parse chunk index %lx %lx %x", pos, end, pHeader);
    struct MtkAVIIndexChunk header;
    if (pHeader == NULL) {
        ssize_t sz = kSizeOfAVIIndexChunk;
        if (end - pos < sz) {
            LOGW("sizeof indx is not correct: %ld < %ld", end - pos, sz);
            return ERROR_MALFORMED;
        }

        ssize_t s = mDataSource->readAt(pos, (char*)&header, sz);
        if (s != sz) {
            LOGW("failed to read header at 0x%08lx", pos);
            return ERROR_IO;
        }
        
        pHeader = &header;
    }

    if (pHeader->indexType != kKeyAVIIndexOfChunks ||
            pHeader->indexSubType != 0) {
        LOGW("unsupport index type %d %d", pHeader->indexType, pHeader->indexSubType);
        return ERROR_MALFORMED;
    }

    pos += kSizeOfAVIIndexChunk;
    int numbers = pHeader->entriesInUse;
    if (end - pos < numbers * (int)sizeof(aviStdIndexEntry)) {
        LOGW("not enough data for index %lx - %lx < %d",
                end, pos, numbers * (int)sizeof(aviStdIndexEntry));
        return ERROR_MALFORMED;
    }

    uint32_t baseOffset = pHeader->baseOffset;
    if (pHeader->baseOffsetHigh != 0) {
        LOGW("TODO support 64 bits file %x%x", pHeader->baseOffsetHigh, pHeader->baseOffset);
        return ERROR_UNSUPPORTED;
    }

    const int numPerRead = 4096 / kSizeOfAVIStdIndexEntry;
    struct aviStdIndexEntry indexes[numPerRead];
    
    int i=0;

    while(i < numbers) {
        int read = numPerRead;
        if (numbers - i < numPerRead)
            read = numbers - i;

        if (mDataSource->readAt(pos + i * kSizeOfAVIStdIndexEntry, 
                    (char*)&indexes, read * kSizeOfAVIStdIndexEntry)
                != read * kSizeOfAVIStdIndexEntry) {
            LOGW("failed to read chunk entry at 0x%08lx", pos);
            return ERROR_IO;
        }

	    bool hastobreak = false;
        for(int j=0; j<read; ++j) {
		struct aviStdIndexEntry entry = indexes[j];
		int8_t isSyncSample = entry.size & 0x80000000 ? 0 : 1;	

		if((entry.offset + baseOffset+(entry.size & 0x7fffffff)) > mFileSize){
			LOGI("break addSample! offset %d, size %d,mFileSize %d", entry.offset + baseOffset, entry.size & 0x7fffffff,mFileSize);
			hastobreak= true;
			break;
		}
		
		MtkAVISample s = {entry.offset + baseOffset,
		entry.size & 0x7fffffff, isSyncSample};	

		source->addSample(&s);
        }
	    if (hastobreak)
		    break;
        i += numPerRead;
    }

    return OK;
}

status_t MtkAVIExtractor::parseINDX(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source) {
    struct MtkAVIIndexChunk header;
    ssize_t sz = kSizeOfAVIIndexChunk;
    if (end - pos < sz) {
        LOGW("sizeof indx is not correct: %ld < %ld", end - pos, sz);
        return ERROR_MALFORMED;
    }

    ssize_t s = mDataSource->readAt(pos, (char*)&header, sz);
    if (s != sz) {
        LOGW("failed to read header at 0x%08lx", pos);
        return ERROR_IO;
    }

    uint64_t tmpOffset = header.baseOffset;
    uint32_t baseOffset = tmpOffset & 0xffffffffLL;
    if (tmpOffset != baseOffset) {
        LOGW("TODO support 64 bits file %llx", tmpOffset);
        return ERROR_UNSUPPORTED;
    }

    if (header.indexType == kKeyAVIIndexOfIndexes) {
        int numbers = header.entriesInUse;
        if (end - pos < numbers * (int)sizeof(aviStdIndexEntry)) {
            LOGW("not enough data for index %lx - %lx < %d",
                    end, pos, numbers * sizeof(aviStdIndexEntry));
            return ERROR_MALFORMED;
        }

        int i;
        pos = pos + kSizeOfAVIIndexChunk;
        for(i = 0; i < numbers; i++) {
            struct aviSuperIndexEntry entry;
            if (mDataSource->readAt(pos, (char*)&entry, kSizeOfAVISuperIndexEntry)
                    != kSizeOfAVISuperIndexEntry) {
                LOGW("failed to read index entry at 0x%08lx", pos);
                return ERROR_IO;
            }
            
            if (entry.offset > 0xffffffffLL) {
                LOGW("TODO support 64 bits file %llx", tmpOffset);
                return ERROR_UNSUPPORTED;
            }
            
            MtkAVIOffT start = entry.offset + baseOffset + kSizeOfChunkHeader;
            status_t err = parseChunkIndex(start, start + entry.size, source, NULL);

            if (err != OK)
                return err;

            pos += kSizeOfAVISuperIndexEntry;
        }
    } else if (header.indexType == kKeyAVIIndexOfChunks) {
        return parseChunkIndex(pos, end, source, &header);
    } else {
        LOGW("TODO support index of type %d", header.indexType);
        return ERROR_UNSUPPORTED;
    }

    return OK;
}

status_t MtkAVIExtractor::parseSTRD(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source) {
    // TODO parse strd
    return OK;
}
#ifdef MTK_AUDIO_CHANGE_SUPPORT
status_t MtkAVIExtractor::parseSTRN(MtkAVIOffT off, MtkAVIOffT end, sp<MtkAVISource> source) {    
    if (end - off == 0) {
        //chongliang cao 2013-07-23 for ALPS00876375 there is an invalid strn chunk,need to return and do't care it begin
        LOGE("sizeof strn is not correct: end - pos == 0,don't care strn");
        return OK;
        //chongliang cao 2013-07-23 for ALPS00876375 there is an invalid strn chunk,need to return and do't care it end
	}
    
    sp<MetaData> meta = source->getFormat();
	
	char lang_code[6] = {0};
	char* ptr = new char[end - off + 1];
	
	{
		memset( (void *)ptr,0x0,end - off + 1);
		
	    ssize_t s = mDataSource->readAt(off, ptr, end - off);
		
		LOGE("[CFA AVI] ptr date = %s",ptr);
	    if (s != end - off) {
	        LOGE("parseSTRN failed to read header at 0x%08lx", off);
	        return ERROR_IO;
	    }
		if (!strncmp(ptr,"Subtitle",strlen("Subtitle"))){
			source->mIsVideo = false;
			
			if( strlen(ptr) >= (strlen("Subtitle - ") + 2)){
	            memcpy( lang_code,ptr + strlen("Subtitle - "),0x5);
	            lang_code[5] = '\0';
			}
	        LOGW("GET subtitle chunk %d",strlen("Subtitle - "));
		}
		else if(!strncmp(ptr,"Video",strlen("Video") )){
			source->mIsVideo = true;
			if( strlen(ptr) >= (strlen("Video - ") + 2)) {
	            memcpy( lang_code,ptr + strlen("Video - "),0x5);
	            lang_code[5] = '\0';
			}
	        LOGW("GET vids chunk %d",strlen("Video - "));
		}
		else if(!strncmp(ptr,"Audio",strlen("Audio") )) {
			source->mIsAudio = true;
			
			if( strlen(ptr) >= (strlen("Audio - ") + 2)){
	            memcpy( lang_code,ptr + strlen("Audio - "),0x5);
	            lang_code[5] = '\0';
			}
	        LOGW("GET Aud chunk %d",strlen("Audio - "));
		}
		
		LOGW("GET kKeyMediaLanguage: %s",lang_code);
		meta->setCString(
				kKeyMediaLanguage, lang_code);
		
	}

	delete [] ptr;
	
    return OK;
}
#endif
status_t MtkAVIExtractor::parseSTRF(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source) {
    struct bitmapInfo vHeader;
    struct waveFormatEx aHeader;
    ssize_t sz;
    char *ptr = NULL;
    sp<MetaData> meta = source->getFormat();
	
    if (source->mIsAudio) {
        sz = kSizeOfWaveFormatEx;
        ptr = (char*)&aHeader;
    } else if (source->mIsVideo) {
        sz = kSizeOfBitmapInfo;
        ptr = (char*)&vHeader;
    } else {    
  	LOGW("Unknown track");	
    	meta->setCString(kKeyMIMEType, "Unknown");		
        // skip other types of stream
        return OK;
    }

    if (end - pos < sz) {
        LOGE("sizeof strf is not correct: %ld < %ld", end - pos, sz);
        return ERROR_MALFORMED;
    }

    ssize_t s = mDataSource->readAt(pos, (char*)ptr, sz);
    if (s != sz) {
        LOGE("failed to read header at 0x%08lx", pos);
        return ERROR_IO;
    }

    if (end - pos > sz) {
        sp<ABuffer> buffer = NULL;
        ssize_t leftBytes = end - pos - sz;
        ssize_t size = end - pos - sz;
        MtkAVIOffT offset = sz;
        bool valid = false;

        if (source->mIsAudio && leftBytes > 2) {
            if (mDataSource->readAt(pos + offset, (char*)&size, 2) != 2) {
                LOGE("failed to read header at 0x%08lx", pos + offset);
                return ERROR_IO;
            }
            if (size <= leftBytes) {
                valid = true;
                offset += 2;
            }
			aHeader.size = size;
			source->mSize = size; //wma need this, some adpcm don't have this member
			LOGI("found cbsize is %d", size);
        } else if (source->mIsVideo && leftBytes > 4) {
            valid = true;
        }

        if (valid) {
            sp<ABuffer> buffer = new ABuffer(size);
            if (buffer == NULL) {
                return -ENOMEM;
            }
            if (mDataSource->readAt(pos + offset, buffer->data(), size) != size) {
                LOGE("failed to read header at 0x%08lx", pos + offset);
                return ERROR_IO;
            }

            source->strfData = buffer;
            LOGI("get extra codec data %ld bytes", size);
        } else {
            LOGW("skip invalid codec data %ld bytes", leftBytes);
        }
    }

    if (source->mIsVideo) {
        // TODO parse more strf
        const char *mime = BFourCC2MIME(vHeader.compression);
        meta->setCString(kKeyMIMEType, mime);
        source->mCompression = vHeader.compression;
		source->mNeedCodecData = IsNeedCodecData(vHeader.compression);
        source->mIsAVC = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC);
		source->mIsMJPG = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MJPEG);
		source->mIsDIVX = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX);
		source->mIsDIVX3 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3);
		source->mIsXVID = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_XVID);
		source->mIsS263 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK);
        meta->setInt32(kKeyWidth, vHeader.width);
        meta->setInt32(kKeyHeight, vHeader.height);
        if (!strcmp("", mime)) {
            mFileMetaData->setInt32(kKeyHasUnsupportVideo, true);
            source->mIsVideo = false;
        }
    } else if (source->mIsAudio) {
        LOGI("wave %x", aHeader.formatTag);
        const char* mime = wave2MIME(aHeader.formatTag);
        if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
            source->mIsVorbis = true;
        }
#ifdef MTK_AUDIO_RAW_SUPPORT
        /* for unsigned PCM */
		if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_RAW)) {
			if(aHeader.bitsPerSample == 8){
				meta->setInt32(kKeyNumericalType, 2);
			}
        }
#endif
        meta->setCString(kKeyMIMEType, mime);
        meta->setInt32(kKeyChannelCount, aHeader.nChannels);
        meta->setInt32(kKeySampleRate, aHeader.nSamplesPerSec);
		meta->setInt32(kKeyBitRate, aHeader.nAvgBytesPerSec);
		meta->setInt32(kKeyBitsPerSample, aHeader.bitsPerSample); 
#ifdef MTK_AUDIO_RAW_SUPPORT
		//add pcm component support
		if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_RAW)) {
            meta->setInt32(kKeyBitWidth, aHeader.bitsPerSample);
			meta->setInt32(kKeyEndian, 2);
			meta->setInt32(kKeyPCMType,1);
			//meta->setInt32(kKeyChannelAssignment, 1);	
        }
#endif
		if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM) 
					|| !strcmp(mime, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM)) {
		    if (source->mSize != 0) {
		        meta->setData(kKeyExtraDataPointer, 0, source->strfData->data(), source->strfData->size());
				LOGI("strfData size is %d", source->strfData->size());
		    } else {
			    meta->setData(kKeyExtraDataPointer, 0, NULL, 0);
				LOGI("ADPCM wave extra data is null");
			}
		}
		keepBitRate = aHeader.nAvgBytesPerSec;
		#if 0
		if ((!strcmp(mime, MEDIA_MIMETYPE_AUDIO_AAC))
			||(!strcmp(mime, MEDIA_MIMETYPE_AUDIO_AAC_ADTS))){
		    containerAACSampleRate = aHeader.nSamplesPerSec;
		    containerAACChannelNumber = aHeader.nChannels;
		}
		#endif
        source->mBitsPerSample = aHeader.bitsPerSample;
        source->mBlockAlign = aHeader.nBlockAlign;
        LOGI("channels %d bits %d samplerate %d bitrate %d nBlockAlign %d", 
                aHeader.nChannels, aHeader.bitsPerSample, aHeader.nSamplesPerSec,
                aHeader.nAvgBytesPerSec, aHeader.nBlockAlign);
        if (source->mSampleSize == 0 && source->mBlockAlign == 1 && !strcmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
            LOGW("fix mp3 samplesize to 1");
            source->mSampleSize = 1;
        }
		if ((source->mSampleSize != source->mBlockAlign) 
			&& (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_RAW))) {
            LOGW("PCM sample size (%d) != block align (%d), fix it", source->mSampleSize, source->mBlockAlign);
            source->mSampleSize = source->mBlockAlign;
        }
        // format, nAvgBytesPerSec, nBlockAlign
        // kKeyAacObjType, kKeySampleRate, kKeyChannelCount,
        // kKeyDuration,
        meta->setInt32(kKeyBlockAlign, aHeader.nBlockAlign); //wma and ADPCM both need this
		//support wma
		//if (!strcmp(mime, MEDIA_MIMETYPE_AUDIO_WMA)) {
            source->mformatTag = aHeader.formatTag;
            source->mChannels = aHeader.nChannels;
            source->mSamplesPerSec = aHeader.nSamplesPerSec;
            source->mAvgBytesPerSec = aHeader.nAvgBytesPerSec;
      	    //source->mSize = aHeader.size;
            LOGI("aHeader.formatTag = 0x%x aHeader.nChannels = %d aHeader.nSamplesPerSec = %d aHeader.nAvgBytesPerSec = %d aHeader.size = %d",
                aHeader.formatTag, aHeader.nChannels, aHeader.nSamplesPerSec,
                aHeader.nAvgBytesPerSec, aHeader.size);

        //}
        if (!strcmp("", mime))
            source->mIsAudio = false;
    }
    return OK;
}

status_t MtkAVIExtractor::parseSTRH(MtkAVIOffT pos, MtkAVIOffT end, sp<MtkAVISource> source) {
    struct aviStreamHeader header;
    ssize_t sz = kSizeOfAVIStreamHeader;
    if (end - pos < sz) {
        LOGE("sizeof strh is not correct: %ld < %ld", end - pos, sz);
        return ERROR_MALFORMED;
    }

    ssize_t s = mDataSource->readAt(pos, (char*)&header, sz);
    if (s != sz) {
        LOGE("failed to read header at 0x%08lx", pos);
        return ERROR_IO;
    }

    if (header.scale == 0 || header.rate == 0) {
        LOGE("wrong rate/scale = %d/%d", header.rate, header.scale);
        return ERROR_MALFORMED;
    }

    source->mIsVideo = header.fccType == BFOURCC('v','i','d','s');
    source->mIsAudio = header.fccType == BFOURCC('m','i','d','i') 
        || header.fccType == BFOURCC('a','u','d','s');
	if (header.fccType == BFOURCC('v','i','d','s')) {
		sp<MetaData> meta = source->getFormat();
		if (header.rate != 0 && header.scale != 0) {
			meta->setInt32(kKeyFrameRate, (float)header.rate/header.scale);
		}
		else
		{
			LOGW("no valid frame rate info in container.\n");
		}
	}

    source->mScale = header.scale;
    source->mRate = header.rate;
    source->mStartOffset = header.start;// unit is refer to main header
    source->mTotalSamples = header.length;
    if (source->mIsVideo && header.sampleSize != 0) {
        LOGW("video has samplesize %d, we don't support", header.sampleSize);
        header.sampleSize = 0;
    }
    source->mMaxSampleSize = source->mSampleSize = header.sampleSize;
    LOGI("header scale %d rate %d length %d samplesize %d init %d start %d", 
            header.scale, header.rate, header.length, header.sampleSize, header.initialFrames, header.start);

    return OK;
}

status_t MtkAVIExtractor::parseHDRL(MtkAVIOffT pos, MtkAVIOffT oldEnd) {
    MtkAVIOffT oldPos = pos;
    status_t err;
    bool avihFound = false;
    int i = 0;

    while(pos < oldEnd) {
        riffList list;
        uint8_t *p = (uint8_t*)&list.ID;
        ssize_t s = mDataSource->readAt(pos, (char*)&list, sizeof(list));
        if (s != kSizeOfListHeader) {
            if (s != kSizeOfChunkHeader || list.size != 0) {
                LOGE("failed to read header at 0x%08lx", pos);
                return ERROR_IO;
            }
            break;
        }

        LOGV("ID " FORMATFOURCC " at pos 0x%08lx", PRINTFOURCC(list.ID), pos);
        
        err = OK;
        MtkAVIOffT end = pos + EVEN(list.size) + kSizeOfSkipHeader;
        if (list.ID == BFOURCC('L','I','S','T')) {
            uint8_t *p2 = (uint8_t*)&list.type;
            LOGV("type " FORMATFOURCC, PRINTFOURCC(list.type));
            switch (list.type) {
                case BFOURCC('s','t','r','l'):
                    if (avihFound == false) {
                        LOGE("avih MUST before strl");
                        err = ERROR_MALFORMED;
                    } else {
                        err = parseSTRL(pos + kSizeOfListHeader, end, i++);
                    }
                    break;
                default:
                    LOGW("unknown list " FORMATFOURCC " in %s", PRINTFOURCC(list.type), __FUNCTION__);
            }
        } else {
            pos = pos + kSizeOfChunkHeader;
            switch(list.ID) {
                case BFOURCC('J','U','N','K'):
                    // skip JUNK
                    break;
                case BFOURCC('a','v','i','h'):
                    avihFound = true;
                    err = parseAVIH(pos, end);
                    break;
                default:
                    LOGW("unknown chunk " FORMATFOURCC " in %s", PRINTFOURCC(list.ID), __FUNCTION__);
            }
        }

        if (err != OK)
            return err;
        pos = end;
    }

    if (pos > oldEnd || !avihFound || i < mNumTracks) {
        LOGE("hdrl seems to be corrupted: end at 0x%08lx, expected end at 0x%08lx," 
                "avih=%d, actual streams %d, expected streams %d",
                pos, oldEnd, avihFound, i, mNumTracks);
        return ERROR_MALFORMED;
    }
    return OK;
}

status_t MtkAVIExtractor::parseIDX1(MtkAVIOffT pos, MtkAVIOffT end) {
    if (end > mFileSize) {
        LOGW("chunked file with wrong idx1, ignore it");
        return OK;
    }
    if (mHasIndex) {
        LOGW("ignore multiple index");
        return OK;
    }
    const int numPerRead = 4096 / kSizeOfAVIOldIndexEntry;
    struct aviOldIndexEntry indexes[numPerRead];
    int numbers = (end - pos) / kSizeOfAVIOldIndexEntry;

    int i=0;
    bool firstIndex = true;

    while(i < numbers) {
        int read = numPerRead;
        if (numbers - i < numPerRead)
            read = numbers - i;

        if (mDataSource->readAt(pos + i * kSizeOfAVIOldIndexEntry, 
                    (char*)&indexes, read * kSizeOfAVIOldIndexEntry)
                != read * kSizeOfAVIOldIndexEntry) {
            LOGE("failed to read old index entry at 0x%08lx", pos + i * kSizeOfAVIOldIndexEntry);
            return ERROR_IO;
        }

        for(int j=0; j<read; ++j) {
            struct aviOldIndexEntry entry = indexes[j];
            size_t id = chID2streamID(entry.ID);
            
            if (firstIndex) {
                if (entry.offset < mMOVIOffset) {
                    // relative offset from 'movi' 
                    mIndexOffset = mMOVIOffset + kSizeOfSkipHeader;
                } else {
                    // absolute offset from file start
                    mIndexOffset = 0;
                }
                firstIndex = false;
            }

            int8_t isSyncSample = 0;
            if (entry.flags & kKeyAVIIFList) {
                LOGW("TODO parse rec index");
            } else if (entry.flags & kKeyAVIIFKeyFrame) {
                isSyncSample = 1;
            }

            MtkAVISample s = {entry.offset + mIndexOffset + kSizeOfChunkHeader, 
                entry.size, isSyncSample};

            addSample(id, &s);
        }
        i += numPerRead;
    }
    mHasIndex = true;
    return OK;
}

status_t MtkAVIExtractor::checkCapability() {
    for(size_t i=0; i<mTracks.size(); ++i) {
        sp<MtkAVISource> source = mTracks[i];
        CHECK(source != NULL);
        source->getFormat()->setInt32(kKeyMaxInputSize, source->mMaxSampleSize);
        status_t err = source->generateCodecData(mHasIndex);
        if (source->mIsVideo) {
		    const char *mime;
		    source->getFormat()->findCString(kKeyMIMEType, &mime);
        }
        if (err == ERROR_UNSUPPORTED) {
            const char *mime;
            CHECK(source->getFormat()->findCString(kKeyMIMEType, &mime));
            LOGW("skip unsupport %s", mime);
            source->getFormat()->setCString(kKeyMIMEType, "");
            if (source->mIsVideo) {
                mFileMetaData->setInt32(kKeyHasUnsupportVideo, true);
            }
            source->mIsAudio = source->mIsVideo = false;
        } else if (err != OK) {
            LOGE("checkCapability got err %d", err);
            return err;
        } else {
            LOGI("finished sample number %d", source->mSampleSizes.size());
        }
    }

    for(size_t i=0; i<mTracks.size(); ++i) {
        sp<MtkAVISource> source = mTracks[i];
        CHECK(source != NULL);
        mHasVideo |= source->mIsVideo;
        mHasAudio |= source->mIsAudio;
        source->updateSamples();
    }

    if (!mHasAudio && !mHasVideo) {
        return ERROR_UNSUPPORTED;
    }
    return OK;
}


status_t MtkAVIExtractor::finishParsing()
{
	LOGI("finishParsing--> index %d meta %d init %d", mHasIndex, mHasMetadata, mInitCheck);  
	status_t err;
	if(mHasIndex){		
		return OK;
	}

    if ((err = readMetaData()) != OK) {
        return err;
    }

	
       err=parseMOVIMore(true);    

	if(err!=OK)
	   return err;  
	   
       err=checkCapability();
	if(err!=OK)
	   	return err;
	LOGI("finishParsing--<");
	return OK;
}

status_t MtkAVIExtractor::parseFirstRIFF() {
    struct riffList list;
    uint8_t *p = (uint8_t*)&list.ID;
    uint8_t *p2 = (uint8_t*)&list.type;

    if (mDataSource->readAt(0, (char*)&list, sizeof(list)) != sizeof(list)) {
        LOGE("failed to read header at 0x%08x", 0);
        return ERROR_IO;
    } 
    //CHECK_EQ(list.ID, BFOURCC('R', 'I', 'F', 'F'));
    //CHECK_EQ(list.type, BFOURCC('A', 'V', 'I', ' '));
    if(list.ID != BFOURCC('R', 'I', 'F', 'F'))
		return ERROR_UNSUPPORTED;
    
    ssize_t size = list.size;
    if (size > kAVIMaxRIFFSize) {
        LOGI("size of RIFF is out of spec %ld > %d", size, kAVIMaxRIFFSize);
        //return ERROR_MALFORMED;
    }

    MtkAVIOffT fSize;
    mDataSource->getSize((MtkAVIOffT*)&fSize);
    mFileSize = fSize;

    if (fSize <= 0) {
        LOGE("abnormal file size %d. out of 32 bit? not supported yet", fSize);
        return ERROR_UNSUPPORTED;
    }

    if (size + kSizeOfSkipHeader > fSize) {
        // TODO do our best to support chunked file
        LOGW("file seems to be chunked (%ld + %d) > %ld", size, kSizeOfSkipHeader, fSize);
    }
    
    MtkAVIOffT pos, oldEnd;
    pos = kSizeOfListHeader;
    oldEnd = kSizeOfSkipHeader + size;
    oldEnd = oldEnd > fSize ? fSize : size;

    status_t err = OK;
    bool idx1Found = false;
    bool moviFound = false;

	int32_t times=0;
    while(pos < oldEnd) {
        if (mStopped) {
            LOGI("stopped by user");
            break;
        }

        if (pos > fSize) {
            LOGW("chunked file? %lx > %lx", pos, fSize);
            break;
        }

        ssize_t s = mDataSource->readAt(pos, (char*)&list, sizeof(list));
        if (s != kSizeOfListHeader) {
            if (s != kSizeOfChunkHeader || list.size != 0) {
                LOGE("failed to read header at 0x%08lx", pos);
                return ERROR_IO;
            }
            break;
        }

        LOGV("ID " FORMATFOURCC " at pos 0x%08lx", PRINTFOURCC(list.ID), pos);
        
        err = OK;
        MtkAVIOffT end = pos + EVEN(list.size) + kSizeOfSkipHeader;

        if (list.ID == BFOURCC('L','I','S','T')) {
            LOGV("type " FORMATFOURCC, PRINTFOURCC(list.type));
            pos = pos + kSizeOfListHeader;
            switch (list.type) {
                case BFOURCC('h','d','r','l'):
                    err = parseHDRL(pos, end);
                    break;
                case BFOURCC('m','o','v','i'):
                    err = parseMOVI(pos, end);
                    moviFound = true;
                    break;
                case BFOURCC('I','N','F','O'):
                    err = parseINFO(pos, end);
                    break;
                default:
                    LOGW("unknown list " FORMATFOURCC " in %s", PRINTFOURCC(list.type), __FUNCTION__);
            }
        } else {
            bool hasINDX = true;
            pos = pos + kSizeOfChunkHeader;
            switch (list.ID) {
                case BFOURCC('J','U','N','K'):
                    // skip JUNK
                    break;
                case BFOURCC('i','d','x','1'):
                    if (idx1Found) {
                        LOGW("skip multiple idx1");
                        break;
                    }
                    idx1Found = true;

                    for(size_t i=0; i<mTracks.size(); ++i) {
                        sp<MtkAVISource> source = mTracks[i];
                        CHECK(source != NULL);
                        if (source->mIsAudio || source->mIsVideo) {
                            if (!source->mINDXValid) {
                                hasINDX = false;
                                break;
                            }
                        }
                    }

                    if (!hasINDX) {
                        for(size_t i=0; i<mTracks.size(); ++i) {
                            sp<MtkAVISource> source = mTracks[i];
                            source->clearSamples();
                        }
                        err = parseIDX1(pos, end);
                    } else {
                        LOGI("we already have index, ignore idx1");
                    }
                    break;
                default:
 		      if(times<5 || times%5000 == 0){
                    		LOGW("unknown chunk " FORMATFOURCC " in %s", PRINTFOURCC(list.ID), __FUNCTION__);
 		      }
		     times++;
                   break;
            }
        }

        if (err != OK)
            return err;
        pos = end;
    }

#if 0
    // TODO parse more RIFFs
    while ((size_t)pos < fSize) {
        err = parseMoreRIFF(&pos);
        if (err != OK) {
            return err;
        }
    }
#endif

    if (!moviFound) {
        LOGE("found no movi");
        return ERROR_MALFORMED;
    }

    if (!mHasIndex) {
        bool hasINDX = true;
        for(size_t i=0; i<mTracks.size(); ++i) {
            sp<MtkAVISource> source = mTracks[i];
            CHECK(source != NULL);
            if (source->mIsAudio || source->mIsVideo) {
                if (!source->mINDXValid) {
                    hasINDX = false;
                    break;
                }
            }
        }

        mHasIndex = hasINDX;
        if (!mHasIndex) {
            for(size_t i=0; i<mTracks.size(); ++i) {
                sp<MtkAVISource> source = mTracks[i];
                CHECK(source != NULL);
                if (source->mIsVideo)
                    source->mBrokenIndex = true;
            }
            err = parseMOVIMore(false);
            if (err != OK)
                return err;
        }
    }

    err = checkCapability();
    if (err != OK)
        return err;

    if (!mHasIndex) {
        for(size_t i=0; i<mTracks.size(); ++i) {
            sp<MtkAVISource> source = mTracks[i];
            if (source == NULL)
                continue;
            int64_t time = source->mRate == 0? 0: source->mTotalSamples 
                * 1000000LL * source->mScale / source->mRate;
            source->getFormat()->setInt64(kKeyDuration, time);
            LOGI("update duration for broken file: %lldus", time);
        }
    }

    mHasMetadata = true;
    return OK;
}

status_t MtkAVIExtractor::readMetaData() {
    if (mHasMetadata) {
        return OK;
    }

    if (mInitCheck != NO_INIT) {
        return mInitCheck;
    }

    MtkAVIOffT offset = 0;
    int seq = 0;
    status_t err = parseFirstRIFF();
    mInitCheck = err;

    if (err != OK) {
        LOGE("error happens when parse file %d", err);
    } else if (mHasMetadata) {
        mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_AVI);
        return OK;
    }

    return err;
}
////////////////////////////////////////////////////////////////////////////////

MtkAVISource::MtkAVISource(const sp<DataSource> &dataSource, int index)
    : mDataSource(dataSource),
      mFormat(new MetaData),
      mIndex(index),
      mIsVideo(false),
      mIsAudio(false),
      mScale(0),
      mRate(0),
      mStartOffset(0),
      mTotalSamples(0),
      mSampleSize(0),
      mMaxSampleSize(0),
      mMaxSyncSampleSize(0),
      mThumbNailIndex(0),
      includes_expensive_metadata(false),
      mCurrentSampleIndex(0),
      mStarted(false),
      mCompression(0),
      mMP3Header(0),
      mLastBufferOffset(0),
      mCurrentDTS(0),
      mCurrentPTSDelta(0),
      mBlockMode(false),
      mIsAVC(false),
      mIsDIVX(false),
      mIsDIVX3(false),
      mIsXVID(false),
      mWantsNALFragments(false),
      mIsADTS(false),
      mIsVorbis(false),
      mIsPCM(false),
      mIsMPEG4(false),
      mIsS263(false),
      mIsMJPG(false),
      mIsFirstBuffer(true),
      mINDXValid(false),
      mBrokenIndex(false),
      mIsMultiple(false),
      mGroup(NULL),
      mBuffer(NULL) {
#ifdef LOG_TO_FILE
      char name[128];
      sprintf(name, "/sdcard/%x.raw", (int)mFormat.get());
      fp = fopen(name, "wb");
#endif
      //for wma
      mformatTag = 0;
      mChannels = 0;
      mSamplesPerSec = 0;
      mAvgBytesPerSec = 0;
      //mBlockAlign, mbitsPerSample;
      mSize = 0;
}

MtkAVISource::~MtkAVISource() {
    if (mStarted) {
        stop();
    }
#ifdef LOG_TO_FILE
    fclose(fp);
#endif
}

static void PCM32toPCM16(uint8_t* src, int length) {
    for(int i=0; i<length/4; i++) {
        src[2 * i + 0] = src[4 * i + 2];
        src[2 * i + 1] = src[4 * i + 3];
    }
}

static void PCM24toPCM16(uint8_t* src, int length) {
    for(int i=0; i<length/3; i++) {
        src[2 * i + 0] = src[3 * i + 1];
        src[2 * i + 1] = src[3 * i + 2];
    }
}

static void PCM8toPCM16(const uint8_t* src, int16_t* dst, int length) {
    for(int i=0; i<length; i++)
        dst[i] = ((int16_t)src[i] - 128) * 256;
}

status_t MtkAVISource::start(MetaData *params) {
    Mutex::Autolock autoLock(mLock);

    CHECK(!mStarted);

    int32_t val;
    if (params && params->findInt32(kKeyWantsNALFragments, &val)
        && val != 0) {
        mWantsNALFragments = true;
    } else {
        mWantsNALFragments = false;
    }

    mGroup = new MediaBufferGroup;

    int32_t max_size = (1920*1080*3 >> 1);
    //CHECK(mFormat->findInt32(kKeyMaxInputSize, &max_size));

    if (mIsPCM && mBitsPerSample == 8) {
        mGroup->add_buffer(new MediaBuffer(2 * max_size));
        mGroup->add_buffer(new MediaBuffer(2 * max_size));
    } else if (mMP3Header < 0) {
        mGroup->add_buffer(new MediaBuffer(MAX_MP3_FRAMESIZE>(2 * max_size)?MAX_MP3_FRAMESIZE:(2 * max_size)));
        mGroup->add_buffer(new MediaBuffer(MAX_MP3_FRAMESIZE>(2 * max_size)?MAX_MP3_FRAMESIZE:(2 * max_size)));
    } else {
        mGroup->add_buffer(new MediaBuffer(max_size));
    }

#if 0
    fp = fopen("/sdcard/a.mp3", "wb");
#endif
    mStarted = true;

    return OK;
}

status_t MtkAVISource::stop() {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);

    if (mBuffer != NULL) {
        mBuffer->release();
        mBuffer = NULL;
    }

    delete mGroup;
    mGroup = NULL;

    mStarted = false;
    mCurrentSampleIndex = 0;

#if 0
    fclose(fp);
#endif
    return OK;
}

sp<MetaData> MtkAVISource::getFormat() {
    Mutex::Autolock autoLock(mLock);

    return mFormat;
}

static int mp3HeaderStartAt(const uint8_t *start, int length, int header) {
    uint32_t code = 0;
    int i = 0;

    for(i=0; i<length; i++){
        code = (code<<8) + start[i];
        if ((code & kMP3HeaderMask) == (header & kMP3HeaderMask)) {
            // some files has no seq start code
            return i - 3;
        }
    }

    return -1;
}

static int seqStartAt(const uint8_t *start, int length) {
    uint32_t code = -1;
    int i = 0;

    for(i=0; i<length; i++){
        code = (code<<8) + start[i];
        if (code == 0x000001b3 || code == 0x000001b6) {
            // some files has no seq start code
            return i - 3;
        }
    }

    return -1;
}

static int nalStartAt(const uint8_t *start, int length, int *prefixLen) {
    uint32_t code = -1;
    int i = 0;

    for(i=0; i<length; i++){
        code = (code<<8) + start[i];
        if ((code & 0x00ffffff) == 0x1) {
            int fourBytes = code == 0x1;
            *prefixLen = 3 + fourBytes;
            return i - *prefixLen + 1;
        }
    }

    return -1;
}

static int nalStartAt2(const uint8_t *start, int length, int *prefixlen)
{
    int i = 0;
	for(i=0; i<length; i++)
	{
		if((start[i] == 0x00) 
			&& (start[i+1] == 0x00)
			&& (start[i+2] == 0x01))
		{
		        *prefixlen = 4;
				return i;
		}
	}
	return 0;
}
static int realAVCStart(const uint8_t *start, int length)
{
    int i = 0;
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
static int findSOI(const uint8_t *start, int offset, int length)
{
    int i = 0;
	int j = 0;

    #if 0
	LOGI("dump data to check SOI location");
	for(j = 0; j < 256; ++j) 
	{
        LOGI("%02d: %02x", j, start[j]);
    }
	#endif
	
	for(i = offset; i<length; ++i)
	{
		if((start[i] == 0xff) 
			&& (start[i+1] == 0xd8))
		{
		        LOGI("found right SOI at %d", i);
				return i;
		}
	}
	LOGE("can't find SOI");
	return -1;
}

static int findEOI(const uint8_t *start, int offset, int length)
{
    int i = 0;
	int j = 0;

	#if 0
	LOGI("dump data to check EOI location");
	for(int j = 0; j < 256; ++j) 
	{
        LOGI("%02d: %02x", j, start[j]);
    }
	#endif
	
	for(i = offset; i<length; ++i)
	{
		if((start[i] == 0xff) 
			&& (start[i+1] == 0xd9))
		{
		        LOGI("we found right EOI at %d", i);
				return i;
		}
	}
	LOGE("can't find EOI");
	return -1;
}
status_t MtkAVISource::readNextChunk(uint8_t* data, int size, ssize_t& num_bytes_read, int offset) {
    if (mCurrentSampleIndex >= mSampleSizes.size())
        return ERROR_END_OF_STREAM;

    MtkAVIOffT nextOffset = mSampleOffsets[mCurrentSampleIndex];
    ssize_t nextSize = mSampleSizes[mCurrentSampleIndex];

    if (mSampleSize != 0 && mCurrentSampleIndex > 0) {
        nextSize -= mSampleSizes[mCurrentSampleIndex - 1];
    }

     if (nextSize < size)
	 	size = nextSize;
        //  return ERROR_END_OF_STREAM;

      num_bytes_read = 
        mDataSource->readAt(nextOffset + offset, data, size);
    if (num_bytes_read < (ssize_t)size) {
        LOGW("failed to read next MP3 frame");
        return ERROR_IO;
    }

    return OK;
}

status_t MtkAVISource::read(
        MediaBuffer **out, const ReadOptions *options) {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);

    *out = NULL;

    int32_t totalSamples = mSampleSizes.size();
    int64_t targetSampleTimeUs = -1;
    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;

    if (totalSamples == 0)
        return ERROR_END_OF_STREAM;

    bool afterseek=false;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {
        LOGI("seekTimeUs=%lld, seekMode=%d, broken=%d", seekTimeUs, mode, mBrokenIndex);
        mIsMultiple = false;
        mLastBufferOffset = 0;
        mCurrentPTSDelta = 0;
        seekTimeUs = seekTimeUs > 0 ? seekTimeUs : 0;
        int32_t index = 0;

        if (mBuffer != NULL) {
            mBuffer->release();
            mBuffer = NULL;
        }

        if (mMP3Header < 0) {
            mCurrentDTS = seekTimeUs;
        }

        if (mBlockMode) {
            int seekBlocks = (int)((float)seekTimeUs * mRate / mScale / 1000000LL + 0.5);
            int low = 0;
            int high = totalSamples;
            index = high;

            while(low < high) {
                int mid = (low + high) / 2;
                uint32_t totalSize = mSampleBlockSizes[mid];
                if (totalSize > (uint32_t)seekBlocks) {
                    index = high = mid;
                } else {
                    low = mid + 1;
                }
            };

            if (index >= totalSamples)
            {
            	LOGI("return ERROR_END_OF_STREAM  1");
                return ERROR_END_OF_STREAM;
          	}

            int blockOffset;
            if (index == 0) {
                blockOffset = seekBlocks;
            } else {
                blockOffset = seekBlocks - mSampleBlockSizes[index - 1];
            }
            mLastBufferOffset = blockOffset * mBlockAlign;
            LOGI("seek blocks %d block offset %d", seekBlocks, mLastBufferOffset);
        } else if (mSampleSize == 0) {
            index = (int)((float)seekTimeUs * mRate / mScale / 1000000LL + 0.5);
            if (index >= totalSamples){
                LOGI("seekTimeUs %lld,  mRate %d,  mScale %d, index %d, totalSamples %d",seekTimeUs,mRate,mScale,index,totalSamples);
                LOGI("return ERROR_END_OF_STREAM  2");
                return ERROR_END_OF_STREAM;
	     }

            if (mIsVideo) {
                int32_t oldIndex = index;
                if (mode == ReadOptions::SEEK_NEXT_SYNC) {
                    while(index < totalSamples) {
                        int8_t isSyncSample = mSampleSyncs[index];
                        if (isSyncSample) {
                            if (!mBrokenIndex || (isSyncSample & kKeySyncFixedMask))
                                break;
                            if (fixSyncSample(index))
                                break;
                        }
                        index++;
                    }
                    if (index >= totalSamples)	{
                        LOGI("return ERROR_END_OF_STREAM  3");
                        return ERROR_END_OF_STREAM;
			       }
                } else {
                    while(index >= 0) {
                        int8_t isSyncSample = mSampleSyncs[index];
                        if (isSyncSample) {
                            if (!mBrokenIndex || (isSyncSample & kKeySyncFixedMask))
                                break;
                            if (fixSyncSample(index))
                                break;
                        }
                        index--;
                    }
                    if (index < 0) {
                        LOGW("found no prev sync frame");
                        index = 0;
                        //return ERROR_MALFORMED;
                    }

                    if (mode == ReadOptions::SEEK_CLOSEST) {
                        // we need to provide kKeyTargetTime
                        targetSampleTimeUs = (int64_t)oldIndex * 1000000LL * mScale / mRate;
                    }
                }
                LOGI("seekTo sample %d of %d", index, totalSamples);
            }
        } else {
            int low = 0;
            int high = totalSamples;
            index = high;

            while(low < high) {
                int mid = (low + high) / 2;
                uint32_t totalSize = mSampleSizes[mid];
                int64_t dts = (int64_t)(totalSize) * 1000000LL * mScale / mRate / mSampleSize;
                if (dts > seekTimeUs) {
                    index = high = mid;
                } else {
                    low = mid + 1;
                }
            };

            if (index >= totalSamples){
                LOGI("return ERROR_END_OF_STREAM  4");
                return ERROR_END_OF_STREAM;
	     }
        }
  	    afterseek=true;
        mCurrentSampleIndex = index;
        LOGI("seek to index of %d", index);
    }

 	
    MtkAVIOffT offset = 0;
    size_t size = 0;
    bool isSyncSample = false;
    bool newBuffer = false;
    status_t err;
    uint32_t totalSize = 0;

    if (mBuffer == NULL || (mMP3Header < 0 && mBuffer->range_length() < 4)) {
        if ((int32_t)mCurrentSampleIndex >= totalSamples){
             LOGI("return ERROR_END_OF_STREAM  5");
             return ERROR_END_OF_STREAM;
	     }

        if (mSampleSize != 0) {
            offset = mSampleOffsets[mCurrentSampleIndex];
            totalSize = mSampleSizes[mCurrentSampleIndex];
            if (mCurrentSampleIndex == 0) {
                size = totalSize;
            } else {
                CHECK(mCurrentSampleIndex >= 1);
                size = totalSize - mSampleSizes[mCurrentSampleIndex - 1];
            }
            totalSize -= size;
            mCurrentSampleIndex++;
        } else {
            do {
                if (mCurrentSampleIndex >= mSampleOffsets.size()){		
                    LOGI("return ERROR_END_OF_STREAM  6");
                    return ERROR_END_OF_STREAM;
		       }

                offset = mSampleOffsets[mCurrentSampleIndex];
                size = mSampleSizes[mCurrentSampleIndex];
                if (mIsVideo)
                    isSyncSample = mSampleSyncs[mCurrentSampleIndex];
                mCurrentSampleIndex++;
            } while (size == 0);
            // some file has chunk with size of 0
        }

        newBuffer = true;

        if (mMP3Header < 0 && mBuffer && mBuffer->range_length() < 4) {
            memcpy((uint8_t*)mBuffer->data(), (uint8_t*)mBuffer->data() + mBuffer->range_offset(),
                    mBuffer->range_length());
            mBuffer->set_range(mBuffer->range_length(), 0);
            mLastBufferOffset = 0;
            // keep our mBuffer
        } else {
            err = mGroup->acquire_buffer(&mBuffer);
            if (err != OK) {
                CHECK(mBuffer == NULL);
                return err;
            }
        }
    } else if (mIsMultiple) {
        CHECK(mBuffer != NULL);
        if ((int32_t)mCurrentSampleIndex >= totalSamples) {
            mBuffer->release();
            mBuffer = NULL;
            LOGI("return ERROR_END_OF_STREAM  7");
            return ERROR_END_OF_STREAM;
        }
        mCurrentSampleIndex++;
        mIsMultiple = false;
    }

    if (mIsADTS) {
        if (size < 7) {
            return ERROR_MALFORMED;
        } else {
            offset += 7;
            size -= 7;
        } 
    }

    bool NeedRefinePos = false;
    if (newBuffer) {
        CHECK(mBuffer != NULL);
        ssize_t num_bytes_read =
            mDataSource->readAt(offset, (uint8_t *)mBuffer->data() + mBuffer->range_offset(), size);

        if (num_bytes_read < (ssize_t)size) {
            mBuffer->release();
            mBuffer = NULL;
            LOGE("failed to read data at 0x%08lx", offset);
            return ERROR_IO;
        }
        mBuffer->set_range(mLastBufferOffset, size + mBuffer->range_offset() - mLastBufferOffset);
        mLastBufferOffset = 0;

        int64_t dts;
        if (mIsAudio && mSampleSize != 0) {
			#if 0
			if(true == isRawAACData){
                dts = (int64_t)(mCurrentSampleIndex - 1) * 1000000LL * mScale / mRate;
			}
			else{
				dts = (int64_t)(totalSize) * 1000000LL * mScale / mRate / mSampleSize;
			}
			#endif
			dts = (int64_t)(totalSize) * 1000000LL * mScale / mRate / mSampleSize;
        } else {
            dts = (int64_t)(mCurrentSampleIndex - 1) * 1000000LL * mScale / mRate;
        }

        if (mIsVideo) {
            if (isBFrame((const char*)mBuffer->data(), mBuffer->range_length())) {
                LOGV("b frame at %x", mSampleOffsets[mCurrentSampleIndex-1]);
                dts -= 1000000LL * mScale / mRate;
            } else {
                int n = followingBFrames();
                if (n < 0)
                    return ERROR_IO;
                dts += 1000000LL * mScale / mRate * n;
            }
        }

        if (mMP3Header >= 0)
            mCurrentDTS = dts;

        LOGV("v=%d, dts=%lld off=%lx s=%x totalSize=%x tid=%d", mIsVideo, dts, offset, size, totalSize, gettid());

#ifndef MTK_AUDIO_RAW_SUPPORT
        if (mIsPCM) {
            if (mBitsPerSample == 8) {
                MediaBuffer *tmp;
                CHECK(mGroup->acquire_buffer(&tmp) == OK);
                tmp->set_range(0, 2 * size);
                PCM8toPCM16((uint8_t*)mBuffer->data(), (int16_t*)tmp->data(), size);
                mBuffer->release();
                mBuffer = tmp;
            } else if (mBitsPerSample == 24) {
                PCM24toPCM16((uint8_t*)mBuffer->data(), size);
                mBuffer->set_range(0, size * 16 / 24);
            } else if (mBitsPerSample == 32) {
                PCM32toPCM16((uint8_t*)mBuffer->data(), size);
                mBuffer->set_range(0, size / 2);
            }
        }
#endif

        mBuffer->meta_data()->clear();
        mBuffer->meta_data()->setInt64(kKeyTime, dts);

	//cr:after seek AV not sync. a chunk has so many mp3 frame, has to fine the right frame.
	if(mMP3Header < 0 && afterseek && (dts<seekTimeUs && seekTimeUs-dts>800000))
	{
		NeedRefinePos = true;	
		mCurrentDTS = dts;
		LOGI("seekTimeUs-dts>800000!!!! dts=%lld,NeedRefinePos=%d",dts,NeedRefinePos);
	}


        if (targetSampleTimeUs >= 0) {
            mBuffer->meta_data()->setInt64(
                    kKeyTargetTime, targetSampleTimeUs);
        }

        if (isSyncSample || mIsAudio) {
            mBuffer->meta_data()->setInt32(kKeyIsSyncFrame, 1);
        }
    }

    if (mIsMJPG) {
        // Each JPEG unit is split up into its constituent JPEG bitstream and
        // each one of them returned in its own buffer.
        int length, start, end;
		#if 0
        if (newBuffer) {
			start = findSOI((uint8_t*)mBuffer->data(), 0, mBuffer->size());
			end = findEOI((uint8_t*)mBuffer->data(), 0,  mBuffer->size());
			length = end - start + 2;
            if ((start == -1) || (end == -1)) {
                // can not find a JPEG bitstream
                *out = mBuffer;
                mBuffer = NULL;
                return OK;
            }

            mBuffer->set_range(
                    mBuffer->range_offset() + length,
                    mBuffer->range_length() - length);
        }
       #endif
        
        const uint8_t *src =
            (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

		start = findSOI(src, 0, mBuffer->range_length());
		end = findEOI(src, 0, mBuffer->range_length());
	    length = end - start + 2;
		LOGI("start at %d, end at %d, length is %d", start, end, length);
        if ((start == -1) || (end == -1)) {
            start = mBuffer->range_length();
            length = 0;
			LOGI("need skip extra data?");
        }

        MediaBuffer *clone = mBuffer->clone();
        CHECK(clone != NULL);
        clone->set_range(mBuffer->range_offset(), length);

        CHECK(mBuffer != NULL);
        mBuffer->set_range(
                mBuffer->range_offset() + start + length,
                mBuffer->range_length() - start - length);

        if (mBuffer->range_length() == 0) {
            mBuffer->release();
            mBuffer = NULL;
        }

        *out = clone;
        return OK;
    }
	if (mIsAVC) {
		 	if(mWantsNALFragments){
	        // Each NAL unit is split up into its constituent fragments and
	        // each one of them returned in its own buffer.
	        int length, start;
	        if (newBuffer) {
	            start = nalStartAt((uint8_t*)mBuffer->data(), mBuffer->size(), &length);
	            if (start == -1) {
	                // not a byte-stream
	                *out = mBuffer;
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
	        if (start == -1) {
	            start = mBuffer->range_length();
	            length = 0;
	        }

	        MediaBuffer *clone = mBuffer->clone();
	        CHECK(clone != NULL);
	        clone->set_range(mBuffer->range_offset(), start);

	        CHECK(mBuffer != NULL);
	        mBuffer->set_range(
	                mBuffer->range_offset() + start + length,
	                mBuffer->range_length() - start - length);

	        if (mBuffer->range_length() == 0) {
	            mBuffer->release();
	            mBuffer = NULL;
	        }

	        *out = clone;
	        return OK;
	    }else{
	    // Whole NAL units are returned but each fragment is prefixed by
        // the start code (0x00 00 00 01).
	        *out = mBuffer;
	        mBuffer = NULL;
	        return OK;
	    }
	}else if(mIsS263 || mIsXVID || mIsDIVX3 ||mIsDIVX){
		*out = mBuffer;
	     mBuffer = NULL;
	     return OK;
	}else if (mMP3Header < 0) {
	    int start=0;
        uint32_t header=0;
        ssize_t frameSize;

    do{
        // MP3 frame header start with 0xff, MUST < 0
        int length = mBuffer->range_length();
        const uint8_t *src =
            (const uint8_t *)mBuffer->data() + mBuffer->range_offset();

        start = mp3HeaderStartAt(src, length, mMP3Header);

        if (start >= 0)
           header = U32_AT(src + start);

        int bitrate;
        bool ret= false;
        ret = get_mp3_frame_size(header, (size_t*)&frameSize, NULL, NULL, &bitrate);
        LOGV("mp3 start %d header %x frameSize %d length %d bitrate %d",start, header, frameSize, length, bitrate);

        if (start >= 0 && ret) {           
            if (frameSize + start > length) {
                // try to get data from next chunk, ugly code ..
                MediaBuffer *tmp;
                CHECK(mGroup->acquire_buffer(&tmp) == OK);
                int needSizeOrg = frameSize + start - length;
                int needSize = needSizeOrg;
                int existSizeOrg = length - start;
                int existSize = existSizeOrg;
                status_t readret=OK;
                ssize_t bytesRead=0;
		
               while(readret == OK)
               {	       	 	
                   readret = readNextChunk((uint8_t*)tmp->data() + existSize,needSize,bytesRead);
                  // LOGE("need more data: needSize %d existSize %d",needSize, existSize);	
                   if(readret == OK){
                       mCurrentSampleIndex++;
                       existSize+=(int)bytesRead;
                       needSize-=(int)bytesRead;				
                   }else{
                       break;
                   }
                   if(existSize>=frameSize){
                       mCurrentSampleIndex--;
                       break;	
                   }
               }

        	if(readret == OK){
               	memcpy(tmp->data(), 
               	            (uint8_t*)mBuffer->data() + mBuffer->range_offset() + start, existSizeOrg);
               	mBuffer->release();
               	mBuffer = NULL;
               
               	tmp->set_range(0, frameSize);
               	tmp->meta_data()->clear();
               	mCurrentDTS += frameSize * 8000ll / bitrate;
               	tmp->meta_data()->setInt64(kKeyTime, mCurrentDTS);
               	tmp->meta_data()->setInt32(kKeyIsSyncFrame, 1);
               	*out = tmp;
               	mLastBufferOffset = (int)bytesRead;
#ifdef LOG_TO_FILE
        	fwrite(tmp->data() + tmp->range_offset(), 1, tmp->range_length(), fp);
#endif
        	       return OK;
        	}else{
                    LOGE("readNextChunk return 0x%x", readret);
                    mBuffer->release();
                    mBuffer = NULL;
                    tmp->release();			
                    return readret;                     
        	}

                tmp->release();


	
                LOGW("send incomplete MP3 frame %d < %ld", length - start, frameSize);
                frameSize = length - start;
            }
		
		mCurrentDTS += frameSize * 8000ll / bitrate;
		if(NeedRefinePos)
		{				
		       if(mBuffer->range_length() - frameSize - start < frameSize){
                          NeedRefinePos= false;
                          LOGI("refine pos: the last frame! NeedRefinePos=%d",NeedRefinePos);
                          break;		
		       }			   
                     
			 mBuffer->set_range(
		                mBuffer->range_offset() + frameSize + start,
		                mBuffer->range_length() - frameSize - start);
		        LOGI("refine pos: NeedRefinePos=%d,mCurrentDTS=%lld, begin=%d,end=%d",
				NeedRefinePos,mCurrentDTS,mBuffer->range_offset(), mBuffer->range_length());
	
			if(seekTimeUs > mCurrentDTS && seekTimeUs -mCurrentDTS < frameSize * 8000ll / bitrate)
			{	
                          NeedRefinePos= false;
                          LOGI("refine pos: NeedRefinePos=%d",NeedRefinePos);
                          break;	
			}						
		}			
        } else {
            LOGW("bad MP3 frame without header, all remain bytes %d", length);

            if (/*mBlockMode &&*/ length >= 4) {    //to create a header for the broken chunk
                char *p = (char*)mBuffer->data() + mBuffer->range_offset();
                char *q = (char*)&mMP3Header;

                p[0] = q[3];
                p[1] = q[2];
                p[2] = q[1];
                p[3] = q[0];

                for (int i = 4; i < 16 && i < length; i++) {
                    p[i] = 0;
                }

                mCurrentDTS += 1000000LL * mScale / mRate;
                if(mBlockMode){
                     frameSize = mBlockAlign;
                }
                else{
                     frameSize = length;
                }
                //mCurrentDTS += length * 1000000LL / mRate;
            } else {
                frameSize = length;
            }
            start = 0;
        }
      }while(NeedRefinePos);
        
        MediaBuffer *clone = mBuffer->clone();
        CHECK(clone != NULL);
        clone->set_range(mBuffer->range_offset() + start, frameSize);

        CHECK(mBuffer != NULL);
        mBuffer->set_range(
                mBuffer->range_offset() + frameSize + start,
                mBuffer->range_length() - frameSize - start);

        if (mBuffer->range_length() == 0) {
            mBuffer->release();
            mBuffer = NULL;
        }

#ifdef LOG_TO_FILE
        fwrite(clone->data() + clone->range_offset(), 1, clone->range_length(), fp);
#endif
        clone->meta_data()->setInt64(kKeyTime, mCurrentDTS);
        *out = clone;
        return OK;
    } else {
        if (mIsMPEG4 || mIsDIVX || mIsXVID) { 
            if (mIsFirstBuffer) {
                // the first buffer sent to decoder should start with 00 00 01 b6/b3
                // or will be bypassed by our decoder
                mIsFirstBuffer = false;
                int offsetStart = seqStartAt((const uint8_t*)mBuffer->data(), mBuffer->range_length());
                if (offsetStart > 0) {
                    mBuffer->set_range(
                            mBuffer->range_offset() + offsetStart,
                            mBuffer->range_length() - offsetStart);
                }
            } else {
                int offsetStart = -1;
                int type = getFrameType((const char*)mBuffer->data() + mBuffer->range_offset(),
                        mBuffer->range_length(), &offsetStart);
                if (type == kKeyAVIUnknownFrame || offsetStart < 0) {
                    LOGW("unknown type sample at %x", mSampleOffsets[mCurrentSampleIndex - 1]);
                } else {
                    mBuffer->set_range(
                            mBuffer->range_offset() + offsetStart, 
                            mBuffer->range_length() - offsetStart);

                    char *ptr = (char*)mBuffer->data() + mBuffer->range_offset();
                    // check if multiple frames in one chunk
                    offsetStart = seqStartAt((const uint8_t*)ptr + 4, mBuffer->range_length() - 4);
                    if (offsetStart >= 0)
                        offsetStart += 4;

                    if (ptr[3] == 0xb3 && offsetStart >= 0) {
                        LOGI("found VOP_STARTCODE and GOP_STARTCODE at %x", mSampleOffsets[mCurrentSampleIndex - 1]);
                        int off = seqStartAt((const uint8_t*)ptr + offsetStart + 4, mBuffer->range_length() - offsetStart - 4);
                        if (off >= 0) {
                            offsetStart = offsetStart + 4 + off;
                        } else {
                            offsetStart = -1;
                        }
                    }

                    if (offsetStart >= 0) {
                        LOGI("packed sample at %x, offset %x", mSampleOffsets[mCurrentSampleIndex-1], offsetStart);
                        mIsMultiple = true;

                        MediaBuffer *clone = mBuffer->clone();
                        clone->set_range(mBuffer->range_offset(), offsetStart);
                        int64_t dts = (int64_t)(mCurrentSampleIndex) * 1000000LL * mScale / mRate;
                        clone->meta_data()->setInt64(kKeyTime, dts);
#ifdef LOG_TO_FILE
                        fwrite(clone->data() + clone->range_offset(), 1, clone->range_length(), fp);
#endif

                        mBuffer->set_range(
                                mBuffer->range_offset() + offsetStart,
                                mBuffer->range_length() - offsetStart);

                        *out = clone;
                        return OK;
                    }
                }
            }
        }
#ifdef LOG_TO_FILE
        fwrite(mBuffer->data() + mBuffer->range_offset(), 1, mBuffer->range_length(), fp);
#endif
        *out = mBuffer;
        mBuffer = NULL;
        return OK;
    }

    return OK;
}

bool MtkAVISource::isSyncFrame(const char* data, int size) const {
    LOGV("isSyncFrame type %d", getFrameType(data, size));
    return getFrameType(data, size) == kKeyAVIIFrame;
}

bool MtkAVISource::isBFrame(const char* data, int size) const {
#ifndef MTK_AVI_SUPPORT_B_FRAME
    return false;
#endif
    LOGV("isBFrame type %d", getFrameType(data, size));
    return getFrameType(data, size) == kKeyAVIBFrame;
}

int MtkAVISource::getFrameType(const char* data, int size, int *pOffset) const {
    if (mIsAVC) {
        if (size < 6)
            return kKeyAVIUnknownFrame;

        int offsetStart = 0;
        int prefixLen = 0;
        uint8_t *ptr = (uint8_t*)data;
        uint8_t *end = ptr + size;
        unsigned char byte = 0;

        while (ptr < end) {
            offsetStart = nalStartAt(ptr, end - ptr, &prefixLen);
            ptr += offsetStart + prefixLen;
            if (offsetStart == -1 || ptr >= end) {
                return kKeyAVIUnknownFrame;
            }

            int i_nal_type = ptr[0] & 0x1f;
            if (i_nal_type == 5)
                return kKeyAVIIFrame;

            if (i_nal_type >= 1 /*NAL_SLICE*/ && i_nal_type < 5 /*NAL_SLICE_IDR*/) {
                byte = ptr[1];
                break;
            }
        }

        if (pOffset != NULL) {
            *pOffset = ptr - (uint8_t*)data - prefixLen;
        }
        // 0 => 1
        // 1~2 => 01 0~1
        // 3~6 => 001 00~11
        // 7~14 => 0001 000~111
        // assume first mb = 0 => 1
        // 1/6 => b frame: 1:1+010+xxxx, 6:1+00111+xx
        if ((byte & 0x80) != 0x80) {
            LOGW("TODO support non zero first mb");
            return kKeyAVIUnknownFrame;
        }

        if ((byte & 0x40) || ((byte >> 2) == 0x26))
            return kKeyAVIPFrame;
        if (byte == 0x88 || ((byte >> 4) == 0xb))
            return kKeyAVIIFrame;
        if ((byte >> 4 == 0xa) || (byte >> 2 == 0x27))
            return kKeyAVIBFrame;
        return kKeyAVIUnknownFrame;
    }

    if (size < 5)
        return kKeyAVIUnknownFrame;

    if (mIsMPEG4 || mIsDIVX || mIsDIVX3 || mIsXVID) { 
        // here is mpeg4
        int off = seqStartAt((uint8_t*)data, size);
        size -= off;
        if (off < 0 || size < 5)
            return kKeyAVIUnknownFrame;

        if (pOffset != NULL)
            *pOffset = off;

        if (data[3] == 0xb3) {
            data += 4;
            size -= 4;
            off = seqStartAt((uint8_t*)data, size);
            size -= off;
            if (off < 0 || size < 5)
                return kKeyAVIUnknownFrame;
        }

        char byte = data[4 + off] & 0xc0;
        if (byte == 0x00)
            return kKeyAVIIFrame;
        else if (byte == 0x40)
            return kKeyAVIPFrame;
        else if (byte == 0x80)
            return kKeyAVIBFrame;
    } else {
        // find the seq start ??
        // here is H263
        if (pOffset != NULL)
            *pOffset = 0;

        char byte = data[4] & 0x02;
        if (byte == 0x00) 
            return kKeyAVIIFrame;
        else
            return kKeyAVIPFrame;
        // TODO parse B frame
    }
    return kKeyAVIUnknownFrame;
}

int MtkAVISource::followingBFrames() const {
    int cnt = 0;
    int i = mCurrentSampleIndex;
    while (i < (int)mSampleOffsets.size()) {
        char head[256];
        int offset = mSampleOffsets[i];
        int size = mSampleSizes[i];
        i++;
        if (size > 256)
            size = 256;

        if (mDataSource->readAt(offset, head, size) != size) {
            return -1;
        }

        if (isBFrame(head, size))
            cnt++;
        else
            break;
    }
    return cnt;
}

// copy from AACExtractor.cpp
static bool isAACADTSHeader(uint8_t header[4]) {
	if(header[0] != 0xFF || (header[1]&0xF0) != 0xF0 ) // test syncword
		return false;
	if((header[1]&0x06) != 0) // layer must be 0
		return false;
	if(((header[2]>>2)&0x0F) >= 12) // samplerate index must <12
		return false;
	if((header[3]&0x02) != 0) // frame size can not lager than 4096
		return false;
	
	return true;
}

status_t MtkAVISource::findMP3Header(int *pIndex, int *pOffset, int *pHeader) const {
    LOGI("max sample size %d", mMaxSampleSize);
    sp<ABuffer> buffer = new ABuffer(mMaxSampleSize);
    int start = *pIndex;
    *pOffset = 0;
    *pHeader = 0;

    while(start < (int)mSampleSizes.size()) {
        int size, offset;
        size = mSampleSizes[start];
        if (start > 0 && mSampleSize != 0) {
            size -= mSampleSizes[start - 1];
        }

        offset = mSampleOffsets[start];
        if (mDataSource->readAt(offset, (uint8_t*)buffer->data(), size) != size) {
            LOGE("failed to read data at 0x%08x", offset);
            return ERROR_IO;
        }

        int i = 0;
        while(i + 3 < size) {
            int header1 = U32_AT(buffer->data() + i);
            size_t frame_size;

            if (get_mp3_frame_size(header1, &frame_size, NULL, NULL, NULL)) {
                uint8_t tmp[4];
                int j = 0;
                for(; j + (int)frame_size + i < size && j < 4; j++) {
                    tmp[j] = buffer->data()[frame_size + i + j];
                }

                if (j < 4) {
                    int left = 4 - j;
                    if (start == (int)mSampleOffsets.size() - 1) {
                        *pIndex = start;
                        *pOffset = i;
                        *pHeader = header1;
                        return OK;
                    } else if (mSampleSizes[start + 1] < left) {
                        tmp[0] = 0;
                    } else {
                        int off = mSampleOffsets[start + 1];
                        if (mDataSource->readAt(off, (uint8_t*)&tmp + j, left) != left) {
                            LOGE("failed to read data at 0x%08x", off);
                            return ERROR_IO;
                        }
                    }
                }

                int header2 = U32_AT(tmp);
                LOGI("possible header %x at %x size %x, test %x", header1, offset + i, frame_size, header2);
                if ((header2 & kMP3HeaderMask) == (header1 & kMP3HeaderMask)) {
                    *pIndex = start;
                    *pOffset = i;
                    *pHeader = header1;
                    return OK;
                }
            }
            ++i;
        }
        start++;
    }

    return OK;
}

static uint32_t FindAVCSPS(uint8_t* buf, uint32_t buflen)
{
	int i=0;
	for(;i<(int)buflen-7;i++)
	{
		if(buf[i] == 0 && buf[i+1] ==0 && buf[i+2] ==0 && buf[i+3] ==1)
		{
			LOGE("i=%d, buf[i+4]: 0x%x", i, buf[i+4]);
			if((buf[i+4] & 0x1f) == 0x07)			
				break;
		}
				
	}
	
	if(i==(int)buflen-7)
	{
		LOGE("can not find SPS!!");
		return 0xff;
	}
	
	LOGE("i=%d, level: %d", i, buf[i+7]);
	return buf[i+7];

}
static void EncodeSize14(uint8_t **_ptr, size_t size) {
    //CHECK_LE(size, 0x3fff);

    uint8_t *ptr = *_ptr;

    *ptr++ = 0x80 | (size >> 7);
    *ptr++ = size & 0x7f;

    *_ptr = ptr;
}

static sp<ABuffer> MakeMPEGVideoESDS(const sp<ABuffer> &csd) {
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
status_t MtkAVISource::generateCodecData(bool full) {
    if (!mIsAudio && !mIsVideo){
        LOGW("NOT V/A track");
        return ERROR_UNSUPPORTED;
    }

    if (mSampleSizes.size() == 0) {
        LOGE("skip empty track");
        return ERROR_UNSUPPORTED;
    }

    if (mIsVideo && (mRate == 0 || mScale == 0)) {
        LOGE("skip rate %d scale %d video track", mRate, mScale);
        return ERROR_UNSUPPORTED;
    }

    const char *mime;
    CHECK(mFormat->findCString(kKeyMIMEType, &mime));

	int AVCPos = 0; 
    LOGI("generateCodecData for %s", mime);
    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK)
            || !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_XVID)) {
        // after setData, mime will be changed???
        bool isMPEG4 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4);
		bool isDIVX = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX);
		bool isXVID = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_XVID);
		bool isDIVX3 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3);
		bool isS263 = !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK);
        bool found = false;
        if ((strfData != NULL) && ((!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4))
			                        ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX))
			                        ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_XVID)))) {
            mFormat->setData(kKeyMPEG4VOS, 0, strfData->data(), strfData->size());
            found = true;
        } else if ((strdData != NULL) && ((!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4))
                                             ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX))
                                             ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_XVID)))) {
            strfData = strdData;
            mFormat->setData(kKeyMPEG4VOS, 0, strdData->data(), strdData->size());
            found = true;
        }else if((strfData != NULL) &&((!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3))
                                         ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK)))){
            mFormat->setData(kKeyMPEG4VOS, 0, strfData->data(), 0);
			found = true;
        }else if((strdData != NULL) && ((!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_DIVX3))
                                        ||(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_SORENSON_SPARK)))){
            strfData = strdData;
            mFormat->setData(kKeyMPEG4VOS, 0, strfData->data(), 0);
			found = true;
        }
       
        if (found && mIsAVC) {
            int prefixLen = 0;
            if (nalStartAt(strfData->data(), strfData->size(), &prefixLen) < 0) {
				if (nalStartAt2(strfData->data(), strfData->size(), &prefixLen) <= 0){
	                int s = strfData->size() > 32 ?  32 : strfData->size();
	                LOGW("discard bad strf size of %d: dumping first %d", strfData->size(), s);
	                for(int i = 0; i < s; ++i) {
	                    LOGW("%02d: %02x", i, strfData->data()[i]);
	                }
	                strdData = NULL;
	                strfData = NULL;
	                found = false;
				}
            }
        }

        if (mSampleSizes.size() > 0) {
            // skip data before first I frame
            int size, offset;
            size = mSampleSizes[0];
            offset = mSampleOffsets[0];
            size = size > 4096 ? 4096 : size;

            sp<ABuffer> buffer = new ABuffer(size);
            if (mDataSource->readAt(offset, (uint8_t*)buffer->data(), size) != size) {
                LOGE("failed to read data at 0x%08x", offset);
                return ERROR_IO;
            }

            int offsetStart = 0;

            if (isMPEG4 || isDIVX || isXVID || isDIVX3 || isS263) { 
                offsetStart = seqStartAt(buffer->data(), size);
                mIsMPEG4 = true;
				if(isDIVX3 && (-1 == offsetStart)){
					LOGI("divx3 is a special case");
					offsetStart = 0;
				}
				if(isS263 && (-1 == offsetStart)){
					LOGI("sorenson spark is a special case");
					offsetStart = 0;
				}
            } else {
                int prefixLen = 0;
                uint8_t *ptr = (uint8_t*)buffer->data();
                uint8_t *end = ptr + size;

				if (mIsAVC){
				    AVCPos = realAVCStart(buffer->data(), size);
				}
                while (ptr < end) {
                    offsetStart = nalStartAt(ptr, end - ptr, &prefixLen);
					if(offsetStart == -1){
						offsetStart = nalStartAt2(ptr, end - ptr, &prefixLen);
					}
                    ptr += offsetStart + prefixLen;
                    if (offsetStart == -1 || ptr >= end) {
                        offsetStart = -1;
                        break;
                    }

                    if (*ptr == 0x65) {
                        offsetStart = ptr - (uint8_t*)buffer->data() - prefixLen;
                        break;
                    }
                }
            }

            if (offsetStart == -1) {
                LOGE("found no seq start");
            } else if (offsetStart >= 0) {
                mSampleOffsets.replaceAt(offset + offsetStart, 0);
                mSampleSizes.replaceAt(mSampleSizes[0] - offsetStart, 0);

                if (!found || mCompression == BFOURCC('X', 'V', 'I', 'D')
                            || mCompression == BFOURCC('x', 'v', 'i', 'd')) {
                    if (mIsAVC){
						int cap= buffer->capacity();
						if(AVCPos + offsetStart < cap){
                            buffer->setRange(AVCPos, offsetStart);
						}else{
						    buffer->setRange(0, offsetStart);
						}
                    }else{
                        buffer->setRange(0, offsetStart);
                    }
                    strfData = buffer;
					if (mIsAVC){
					// mtk80902: FIXME! 
					//    MakeAVCCodecSpecificData2(strfData, mFormat);
					    mFormat = MakeAVCCodecSpecificData(strfData);
					}else{				
					    if(!(mIsDIVX3||mIsS263)){  
                            mFormat->setData(kKeyMPEG4VOS, 0, strfData->data(), strfData->size());
					    }else{
					        mFormat->setData(kKeyMPEG4VOS, 0, strfData->data(), 0);
					    }
					}
                    found = true;
                }
            }
            
            if (!found) {
				if(mNeedCodecData){
                    LOGE("can not parse valid codec info, skip");
                    return ERROR_UNSUPPORTED;
				}else{
				    LOGI("No codec specific data");
				}
            } else {
				int s = 0;
				if (mIsAVC){
                    s = (strfData->size() - AVCPos) > 256 ?  256 : (strfData->size() - AVCPos);
					LOGW("strf size of %d: dumping first %d", (strfData->size() - AVCPos), s);
				}else{
					s = strfData->size() > 256 ?  256 : strfData->size();
					LOGW("strf size of %d: dumping first %d", strfData->size(), s);
				}
                for(int i = 0; i < s; ++i) {
                    LOGW("%02d: %02x", i, strfData->data()[i]);					
                }

		 if(mIsAVC)
		 {
			 mLevel = FindAVCSPS(strfData->data(), strfData->size());
		 }else
		 {
		 	 int volHeader = findVOLHeader(strfData->data(), strfData->size());
	                if (volHeader >= 0) {
	                    LOGI("found vol header at %d", volHeader);
                        return OK;
	                    struct MPEG4Info s;
	                    s.progressive = 0;
	                    if (decodeVOLHeader(strfData->data() + volHeader + 4,
	                                strfData->size() - volHeader - 4, &s) != 0) {
	                        LOGE("MPEG4 bad vol header");
	                        return ERROR_UNSUPPORTED;
	                    } else if (!s.progressive) {
	                        LOGE("[MPEG4 capability error]Unsupported interlaced video");
	                        return ERROR_UNSUPPORTED;
	                    }
	                }		 	
		 }               
            }
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_H263)) {
        return OK;
#if 0
        int offset = mSampleOffsets[0];
        int size = mSampleSizes[0];
        sp<ABuffer> buffer = new ABuffer(size);
        if (mDataSource->readAt(offset, (uint8_t*)buffer->data(), size) != size) {
            LOGE("failed to read data at 0x%08x", offset);
            return ERROR_IO;
        }
        struct MPEG4Info s;
        s.cpcf = 0;
        if (decodeShortHeader(buffer->data(), buffer->size(), &s) != 0) {
            LOGE("H263 bad header");
            return ERROR_UNSUPPORTED;
        } else if (s.cpcf) {
            LOGE("[H263 capability error]unsupport CPCF");
            return ERROR_UNSUPPORTED;
        }
#endif
    }
	else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG2)){ 
		
		mIsMPEG2 = true;
		
		int offset = mSampleOffsets[0];
		int size = mSampleSizes[0];

		sp<ABuffer> buffer = new ABuffer(size);

        if(mDataSource->readAt(offset, (uint8_t*)buffer->data(), size) != size){
			LOGE("failed to read data at 0x%08x", offset);
			return ERROR_IO;
        }

        sp<ABuffer> esds = MakeMPEGVideoESDS(buffer);

		mFormat->setData(kKeyESDS, kTypeESDS, esds->data(), esds->size());
        //return OK;    
    }
	else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RAW)) {
        if (mBitsPerSample != 8 && mBitsPerSample != 16 
                && mBitsPerSample != 24 && mBitsPerSample != 32) {
            LOGE("unsupport bits per sample %d", mBitsPerSample);
            return ERROR_UNSUPPORTED;
        }
        mIsPCM = true;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_G711_ALAW)
                || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_G711_MLAW)) {
        if (mBitsPerSample != 8) {
            LOGE("unsupport bits per sample %d", mBitsPerSample);
            return ERROR_UNSUPPORTED;
        }
	} else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MS_ADPCM)
                || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM)) {
        if (mBitsPerSample != 4) {
            LOGE("unsupport bits per sample %d", mBitsPerSample);
            return ERROR_UNSUPPORTED;
        }
	}else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMA)) {
        waveFormatEx wfx;

		wfx.formatTag = mformatTag;
        wfx.nChannels = mChannels;
        wfx.nSamplesPerSec = mSamplesPerSec;
        wfx.nAvgBytesPerSec = mAvgBytesPerSec;
        wfx.nBlockAlign = mBlockAlign;
        wfx.bitsPerSample = mBitsPerSample;
        wfx.size = mSize;
		LOGI("wma wave cbSize is %d", mSize);

        uint32_t _config_size = kSizeOfWaveFormatEx + 2 + wfx.size;  //wfx->cbSize must be 10
		uint8_t* _config = new uint8_t[_config_size];
        LOGI("kKeyWMAC size is =%d\n",_config_size);
		memcpy(_config, &wfx, (kSizeOfWaveFormatEx + 2));
		memcpy(_config + kSizeOfWaveFormatEx + 2, strfData->data(), wfx.size);
		mFormat->setData(kKeyWMAC, 0, _config, _config_size);
		delete [] _config;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
        if (mSampleSizes.size() == 0) {
            return OK;
        }

        int value;
        if (mFormat->findInt32(kKeyChannelCount, &value)
                && value != 1 && value != 2) {
            LOGE("[AAC capability error]Unsupported AAC audio, channels %d", value);
            return ERROR_UNSUPPORTED;
        }

        int size, offset;
        size = mSampleSizes[0];
        offset = mSampleOffsets[0];

        uint8_t header[4];
        if (mDataSource->readAt(offset, (uint8_t*)&header, 4) != 4) {
            LOGE("failed to read data at 0x%08x", offset);
            return ERROR_IO;
        }
		#if 0
		    bool isADIF = false;
			uint8_t codecData[2] = {0, 0};
			int channels, realSampleRate, sampleFreqIndex;
			uint32_t profile;
			mFormat->setInt32(kKeyBitRate, keepBitRate);
			
        if (!isAACADTSHeader(header)) {
            LOGI("raw AAC data");

		    isRawAACData = true;

			channels = (int)containerAACChannelNumber;
			LOGI(" channel number is %d", channels);

			realSampleRate = containerAACSampleRate > 48000? containerAACSampleRate: containerAACSampleRate/2;
			sampleFreqIndex = switchAACSampleRateToIndex(realSampleRate);
			if(-1 == sampleFreqIndex){
				if(realSampleRate<24001){
					sampleFreqIndex = switchAACSampleRateToIndex(realSampleRate*2);
				}
				if(-1 == sampleFreqIndex){
					LOGE("raw aac data and sample rate in file format not support");
				    return ERROR_UNSUPPORTED;	
				}
			}
			LOGI("sample rate is %d , sample rate index is %d", realSampleRate, sampleFreqIndex);
			
			profile = 1; 
			mFormat->setInt32(kKeyAACProfile, profile);

			codecData[0] |= (profile + 1) << 3;
	        codecData[0] |= ((sampleFreqIndex & 0x0F) >> 1);
	        codecData[1] |= ((sampleFreqIndex & 0x01) << 7);
	        codecData[1] |= channels << 3;
			
			LOGI("REAL aac codec data %x %x", codecData[0], codecData[1]);
			mIsADTS = false;
            //return ERROR_UNSUPPORTED;
			
        }else{
	        channels = ((header[2] & 0x3) << 2) | (header[3] >> 6);
	        sampleFreqIndex = (header[2] >> 2) & 0xF;
	        profile = header[2] >> 6;

			mFormat->setInt32(kKeyAACProfile, profile);
	        codecData[0] |= (profile + 1) << 3;
	        codecData[0] |= ((sampleFreqIndex & 0x0F) >> 1);
	        codecData[1] |= ((sampleFreqIndex & 0x01) << 7);
	        codecData[1] |= channels << 3;

            LOGI("aac codec data %x %x", codecData[0], codecData[1]);
			mIsADTS = true;
        }
		#endif		
		if (!isAACADTSHeader(header)) {
			LOGE("raw AAC data, cann't generate codec data");
			return ERROR_UNSUPPORTED;
		}
		
		bool isADIF = false;
		int channels, sampleFreqIndex, profile;
		channels = ((header[2] & 0x3) << 2) | (header[3] >> 6);
		sampleFreqIndex = (header[2] >> 2) & 0xF;
		profile = header[2] >> 6;
		
		uint8_t codecData[2] = {0, 0};
		codecData[0] |= (profile + 1) << 3;
		codecData[0] |= ((sampleFreqIndex & 0x0F) >> 1);
		codecData[1] |= ((sampleFreqIndex & 0x01) << 7);
		codecData[1] |= channels << 3;
		
		LOGI("aac codec data %x %x", codecData[0], codecData[1]);
		mFormat->setData(kKeyCodecConfigInfo, 0, codecData, 2);
		mFormat->setInt32(kKeyIsAACADIF, isADIF);
		mIsADTS = true;
		
        mFormat->setData(kKeyCodecConfigInfo, 0, codecData, 2);
        mFormat->setInt32(kKeyIsAACADIF, isADIF);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
        mMP3Header = kKeyMP3NoHeader;
        if (!full) {
            LOGI("just return OK when we are in fast parsing");
            return OK;
        }

        if (mSampleSizes.size() == 0) {
            return OK;
        }

        int start = 0, i;
        status_t err = findMP3Header(&start, &i, &mMP3Header);
        if (err != OK || mMP3Header == 0) {
            LOGE("found no mp3 header, skip this track");
            return ERROR_UNSUPPORTED;
        }

        LOGI("MP3 Header = %x at frame %d, offset %x", mMP3Header, start, i);
        if (mSampleSize > 1 && mScale == 1) {
            LOGW("rewrite abnormal mp3 samplesize %d to 1", mSampleSize);
            mSampleSize = 1;
        } else if (mSampleSize == 0) {
            LOGW("framed mp3? with samplesize 0");
            int block = mBlockAlign;
            int framesize, sampleperframe;
            get_mp3_frame_size(mMP3Header, (size_t*)&framesize, NULL, NULL, NULL, &sampleperframe);

            if (start > 0 && block > 0 && mSampleSizes[0] > 0 
                    && block == sampleperframe && mSampleSizes[0] % block == 0) {
                int accu = 0;
                for(int i = 0; i < (int)mSampleSizes.size(); ++i) {
                    accu += (mSampleSizes[i] + block - 1) / block;
                    mSampleBlockSizes.push(accu);
                }
                mBlockMode = true;
                LOGW("enable block mode align %d total %d", block, accu);
            }
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
        // should we return ERROR_MALFORMED if codec data is invalid?
        if (strfData == NULL) {
            LOGE("no codec data for vorbis");
            return ERROR_UNSUPPORTED;
        }

        uint8_t *data = strfData->data();
        ssize_t size = strfData->size();
        if (size < 3) {
            LOGE("error codec data size for vorbis: %ld", size);
            return ERROR_UNSUPPORTED;
        }

        ssize_t len1 = data[1];
        ssize_t len2 = data[2];

        if (size <= 3 + len1 + len2) {
            LOGE("error codec data size for vorbis: %ld %ld %ld", size, len1, len2);
            return ERROR_UNSUPPORTED;
        }

        if (data[0] != 0x02 || data[3] != 0x01 || data[len1 + 3] != 0x03
                || data[len1 + len2 + 3] != 0x05) {
            LOGE("error codec data for vorbis %x %x %x %x",
                    data[0], data[3], data[len1 + 3], data[len1 + len2 + 3]);
            return ERROR_UNSUPPORTED;
        }
        mFormat->setData(kKeyVorbisInfo, 0, data + 3, len1);
        mFormat->setData(kKeyVorbisBooks, 0, data + len1 + len2 + 3,
                size - len1 - len2 - 3);
        mIsVorbis = true;
    }

    return OK;
}

status_t MtkAVISource::updateSamples() {
    int64_t dts = 0;
    if ((mIsAudio || mIsVideo) && mSampleSizes.size() != 0) {
        if (mBlockMode) {
            uint32_t totalSize = mSampleBlockSizes[mSampleBlockSizes.size() - 1];
            dts = (int64_t)(totalSize) * 1000000LL * mScale / mRate;
        } else if (mSampleSize == 0) {
            dts = mSampleSizes.size() * 1000000LL * mScale / mRate;
        } else {
            uint32_t totalSize = mSampleSizes[mSampleSizes.size() - 1];
            dts = (int64_t)(totalSize) * 1000000LL * mScale / mRate / mSampleSize;
        }
    }
    mFormat->setInt64(kKeyDuration, dts);
    return OK;
}

status_t MtkAVISource::clearSamples() {
    mSampleSyncs.clear();
    mSampleSizes.clear();
    mSampleOffsets.clear();
    mMaxSyncSampleSize = mMaxSampleSize = mThumbNailIndex = 0;
    return OK;
}

status_t MtkAVISource::addSample(struct MtkAVISample *s) {
    if (s->size == 0 && !mIsVorbis && (!mIsVideo || mSampleSizes.size() == 0))
        return OK;

    mSampleOffsets.push(s->offset);
    if (mMaxSampleSize < s->size)
        mMaxSampleSize = s->size;

    if (mIsVideo) {
        uint8_t isSyncSample = s->isSyncSample;
        CHECK(isSyncSample == 0 || isSyncSample == 1);
        mSampleSyncs.push(isSyncSample);
        if (isSyncSample && mMaxSyncSampleSize < s->size) {
            mMaxSyncSampleSize = s->size;
            mThumbNailIndex = mSampleSyncs.size() - 1;
        }
    }

    size_t totalSamples = mSampleSizes.size();
    if (mSampleSize == 0 || totalSamples == 0) {
        mSampleSizes.push(s->size);
    } else {
        mSampleSizes.push(s->size + mSampleSizes[totalSamples - 1]);
    }

    return OK;
}

bool MtkAVISource::fixSyncSample(int32_t index) {
#ifndef MTK_AVI_SUPPORT_FIX_SYNC_FRAME
    return true;
#endif
    MtkAVIOffT offset = mSampleOffsets[index];
    ssize_t size = mSampleSizes[index];

    if (size > 0) {
        char head[256];
        if (size > 256)
            size = 256;

        if (mDataSource->readAt(offset, head, size) == size) {
            if (isSyncFrame(head, size)) {
                LOGI("fixSyncSample at %d", index);
                mSampleSyncs.replaceAt(1 | kKeySyncFixedMask, index);
                return true;
            }
        }
    }
    mSampleSyncs.replaceAt(0, index);
    return false;
}

bool MtkSniffAVI(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *) {
    char header[12];
    if (source->readAt(0, header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    if (memcmp(header, "RIFF", 4)) {
        return false;
    }

    if (!memcmp(header + kSizeOfSkipHeader, "AVI", 3)) {
        char x = header[kSizeOfSkipHeader + 3];
        if (x != ' ' && x != 0x19) {
            LOGE("riff avi %x", x);
            return false;
        }
        *mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
        *confidence = 0.5;

        return true;
    }

    return false;
}

uint32_t MtkAVIExtractor::flags() const {
    return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK | MAY_PARSE_TOO_LONG;
}

status_t MtkAVIExtractor::stopParsing() {
    mStopped = true;
    return OK;
}

int switchAACSampleRateToIndex(int sample_rate){
	
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

		case 16000:
			index = 7;
		return index;

		case 12000:
			index = 8;
		return index;

		case 11025:
			index = 9;
		return index;

		case 8000:
			index = 10;
		return index;

        default:
			index = -1;
			LOGE("switchAACSampleRateToIndex: error sample rate: %d", sample_rate);
		return index;
		
	}
		
}
}  // namespace android
