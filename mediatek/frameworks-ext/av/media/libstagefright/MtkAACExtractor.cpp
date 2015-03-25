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

/*****************************************************************************
*
* Filename:
* ---------
*   MtkAACExtractor.cpp
*
* Project:
* --------
*   MT65xx
*
* Description:
* ------------
*   MTK AAC Parser
*
* Author:
* -------
*  mtk80830
*  mtk80712
****************************************************************************/

//#define LOG_NDEBUG 0


#define LOG_TAG "MtkAACExtractor"

#include <utils/Log.h>
#include <cutils/xlog.h>

#define ENABLE_AAC_EXTR_DEBUG
#ifdef ENABLE_AAC_EXTR_DEBUG
#define AAC_EXTR_VEB(fmt, arg...) SXLOGV(fmt, ##arg)
#define AAC_EXTR_DBG(fmt, arg...) SXLOGD(fmt, ##arg)
#define AAC_EXTR_INFO(fmt, arg...) SXLOGI(fmt, ##arg)
#define AAC_EXTR_WARN(fmt, arg...) SXLOGW(fmt, ##arg)
#define AAC_EXTR_ERR(fmt, arg...)  SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define AAC_EXTR_VEB ALOGV
#define AAC_EXTR_DBG ALOGD
#define AAC_EXTR_INFO ALOGI
#define AAC_EXTR_WARN ALOGW
#define AAC_EXTR_ERR ALOGE
#endif

#include "MtkAACExtractor.h"
#include "include/ID3.h"
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>
#include "include/ESDS.h"

// for AAC seek table --->
#include "TableOfContentThread.h"
#include "include/AwesomePlayer.h"
#include <cutils/properties.h>

// <---for AAC seek table
#ifdef USE_PV_AAC
#include "codecs/aacdec/pvmp4audiodecoder_api.h"
#endif

#ifdef USE_MTK_AAC
#include "heaacdec_exp.h"
#endif

#ifdef USE_FRAUNHOFER_AAC
#include "aacdecoder_lib.h"
#define ADIF_DEC_INBUF_LEN      3072
#define ADIF_DEC_OUTBUF_LEN     8192
#define ADIF_DEC_BUFFER_SIZE    3072
#define FILEREAD_MAX_LAYERS 2
#else
#define ADIF_DEC_INBUF_LEN      4096
#define ADIF_DEC_OUTBUF_LEN     8192
#define ADIF_DEC_BUFFER_SIZE    1536
#endif

namespace android
{
// adts fixheader must match except for
// layer,protection, private bit, mode, mode extension,
// copyright/original  bit ,home bit,copyright identification bit,and copyright identification start bit.
// Yes ... there are things that must indeed match...
static const uint32_t kMask = 0xfff8fdc0;//for adts

#define MEDIA_BUFFER_LEN        8192

// AAC seek table --->
#define ADTS_MANDATORY_SEEK         // for ADTS mandatory seek  
//#define ADIF_MANDATORY_SEEK           // for ADIF mandatory seek
#define ENABLE_USE_TABLE_OF_CONTENT


// <--- for AAC seek table


/*
 * Sampling Frequency look up table
 * The look up index is found in the
 * header of an ADTS packet
 */
static const uint32_t AACSampleFreqTable[16] =
{
    96000, /* 96000 Hz */
    88200, /* 88200 Hz */
    64000, /* 64000 Hz */
    48000, /* 48000 Hz */
    44100, /* 44100 Hz */
    32000, /* 32000 Hz */
    24000, /* 24000 Hz */
    22050, /* 22050 Hz */
    16000, /* 16000 Hz */
    12000, /* 12000 Hz */
    11025, /* 11025 Hz */
    8000, /*  8000 Hz */
    -1, /* future use */
    -1, /* future use */
    -1, /* future use */
    -1  /* escape value */
};

static bool isAACADTSHeader(uint8_t header[4])
{
    if (header[0] != 0xFF || (header[1] & 0xF0) != 0xF0) // test syncword
    {
        return false;
    }

    if ((header[1] & 0x06) != 0) // layer must be 0
    {
        return false;
    }

    if (((header[2] >> 2) & 0x0F) >= 12) // samplerate index must <12
    {
        return false;
    }

    if ((header[3] & 0x02) != 0) // frame size can not lager than 4096
    {
        return false;
    }

    return true;
}

static bool findADTSSyncword(
    const sp<DataSource> &source,
    off_t *inout_pos, uint32_t *out_header)
{
    if (*inout_pos == 0)
    {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.
        for (;;)
        {
            uint8_t id3header[10];

            if (source->readAt(*inout_pos, id3header, sizeof(id3header))
                    < (ssize_t)sizeof(id3header))
            {
                // If we can't even read these 10 bytes, we might as well bail
                // out, even if there _were_ 10 bytes of valid mp3 audio data...
                return false;
            }

            if (memcmp("ID3", id3header, 3))
            {
                break;
            }

            // Skip the ID3v2 header.

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;

            *inout_pos += len;

            //AAC_EXTR_DBG("skipped ID3 tag, new starting offset is %ld (0x%08lx)",
            //     *inout_pos, *inout_pos);
        }
    }

    off_t pos = *inout_pos;
    bool valid = false;

    const size_t kMaxReadBytes = 1024;
    const off_t kMaxBytesChecked = 128 * 1024;
    uint8_t buf[kMaxReadBytes];
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;

    do
    {
        if (pos >= *inout_pos + kMaxBytesChecked)
        {
            // Don't scan forever.
            //AAC_EXTR_DBG("giving up at offset %ld", pos);
            break;
        }

        if (remainingBytes < 7)
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

                reachEOS = (totalBytesRead != bytesToRead);
                totalBytesRead += remainingBytes;
                remainingBytes = totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        if (!isAACADTSHeader(tmp))
        {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        size_t frame_size;
        frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

        if (frame_size <= 7)
        {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        //AAC_EXTR_DBG("found possible 1st frame at %ld, frame size = %d ", pos, frame_size);

        // We found what looks like a valid frame,
        // now find its successors.

        off_t test_pos = pos + frame_size;

        valid = true;

        for (int j = 0; j < 3; ++j)
        {
            uint8_t tmp[7];

            if (source->readAt(test_pos, tmp, 7) < 7)
            {
                valid = false;
                break;
            }

            uint32_t test_header = U32_AT(tmp);

            if (header != 0 && (test_header & kMask) != (header & kMask))  //  compare frame fixed header
            {
                valid = false;
                break;
            }

            size_t test_frame_size;
            test_frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

            if (test_frame_size <= 7)    //  compare frame fixed header
            {
                valid = false;
                break;
            }

            //AAC_EXTR_DBG("found subsequent frame #%d at %ld", j + 2, test_pos);

            test_pos += test_frame_size;
        }

        if (valid)
        {
            *inout_pos = pos;

            if (out_header != NULL)
            {
                *out_header = header;
            }
        }
        else
        {
            //AAC_EXTR_DBG("no dice, no valid sequence of frames found.");
        }

        ++pos;
        ++tmp;
        --remainingBytes;
    }
    while (!valid);

    return valid;
}

static bool findADIFHeader(
    const sp<DataSource> &source,
    off_t *inout_pos, uint32_t *out_header, uint32_t *pSampleFreqIndex,
    uint32_t *pBitRate, uint32_t *pProfile, uint32_t *pChannelNum)
{
    if (*inout_pos == 0)
    {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.

        for (;;)
        {
            uint8_t id3header[10];

            if (source->readAt(*inout_pos, id3header, sizeof(id3header))
                    < (ssize_t)sizeof(id3header))
            {
                // If we can't even read these 10 bytes, we might as well bail
                // out, even if there _were_ 10 bytes of valid mp3 audio data...
                return false;
            }

            if (memcmp("ID3", id3header, 3))
            {
                break;
            }

            // Skip the ID3v2 header.

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;

            *inout_pos += len;

            //AAC_EXTR_DBG("skipped ID3 tag, new starting offset is %ld (0x%08lx)",
            //     *inout_pos, *inout_pos);
        }
    }

    off_t pos = *inout_pos;

    uint8_t tmpBuf[4];

    if (source->readAt(pos, tmpBuf, 4) != 4)
    {
        return false;
    }

    if (tmpBuf[0] == 0x41 && // 'A'
            tmpBuf[1] == 0x44 &&  // 'D'
            tmpBuf[2] == 0x49 &&  // 'I'
            tmpBuf[3] == 0x46)    // 'F'
    {
        *out_header = U32_AT(tmpBuf);

        uint8_t pBuffer[4096];
        memset(pBuffer, 0, 4096);

        if (source->readAt(pos, pBuffer, 4096) <= 0)
        {
            return false;
        }

        uint32_t id;
        uint32_t bitstreamType;
        uint32_t numProgConfigElem;
        uint32_t ADIFHeaderLen;
        uint32_t numFrontChanElem;
        uint32_t numSideChanElem;
        uint32_t numBackChanElem;
        uint32_t numLfeChanElem;
        uint32_t numAssocDataElem;
        uint32_t numValidCCElem;
        uint32_t commentFieldBytes;
        uint32_t offset;
        uint32_t bitIndex = 0;
        uint8_t tmp;

        uint32_t iBitrate = 0;
        uint32_t iAudioObjectType = 0;
        uint32_t iSampleFreqIndex = 0;
        uint32_t iChannelConfig = 0;
        uint32_t sampleFreqIndex = 0;
        uint32_t bitRate = 0;
        int32_t i = 0;

        ADIFHeaderLen = 32; // adif_id 4 bytes

        // check copyright_id_present (1 bit)
        id = pBuffer[4] & 0x80;

        if (id != 0)    //copyright ID is presented
        {
            ADIFHeaderLen += 75;    // copyright_id_present 1 bit
            // copyright_id 72 bits,
            // original_copy 1 bit,
            // home 1 bit,

            // check bitstream type
            bitstreamType = pBuffer[13] & 0x10;

            // get number of program config. element
            numProgConfigElem = (pBuffer[16] & 0x1E) >> 1;

            // get bitrate (max for variable rate bitstream)
            /*
            iBitrate = bitRate = ((pBuffer[13] & 0xF0) << 15) |
                                 (pBuffer[14] << 11) |
                                 (pBuffer[15] << 3)  |
                                 ((pBuffer[16] & 0xE0) >> 5);
                                 */
            //Modified by HP Cheng, fix the bug of getting bit rate
            iBitrate = bitRate = ((pBuffer[13] & 0x0F) << 19) |
                                 (pBuffer[14] << 11) |
                                 (pBuffer[15] << 3)  |
                                 ((pBuffer[16] & 0xE0) >> 5);

            if (iBitrate == 0)   //bitrate is not known
            {
                ////PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
                ////return AACBitstreamObject::MISC_ERROR;
                AAC_EXTR_ERR("findADIFHeader -- iBitrate is 0");
                return false;
            }

            ADIFHeaderLen += 28;    // bitstream_type 1 bit,
            // bitrate 23 bits
            // num_program_config_elements 4 bits

            for (i = 0; i < (int32_t)numProgConfigElem + 1; i++)
            {
                if (bitstreamType == 0) //bistream type is constant rate bitstream
                {
                    ADIFHeaderLen += 20;    //adif_buffer_fullness 20 bits

                    // get audio object type
                    iAudioObjectType = (uint8_t)(((pBuffer[19] & 0x1) << 1) | ((pBuffer[20] & 0x80) >> 7));

                    // get sampling rate index
                    iSampleFreqIndex = sampleFreqIndex = (uint8_t)((pBuffer[20] & 0x78) >> 3);

                    // get number of front channel elements
                    numFrontChanElem = (uint32_t)(((pBuffer[20] & 0x7) << 1) | ((pBuffer[21] & 0x80) >> 7));

                    // get number of side channel elements
                    numSideChanElem = (uint32_t)((pBuffer[21] & 0x78) >> 3);

                    // get number of back channel elements
                    numBackChanElem = (uint32_t)(((pBuffer[21] & 0x7) << 1) | ((pBuffer[22] & 0x80) >> 7));

                    // get number of LFE channel elements
                    numLfeChanElem = (uint32_t)((pBuffer[22] & 0x60) >> 5);

                    // get number of assoc data elements
                    numAssocDataElem = (uint32_t)((pBuffer[22] & 0x1C) >> 2);

                    // get number of valid CC elements
                    numValidCCElem = (uint32_t)(((pBuffer[22] & 0x3) << 2) | ((pBuffer[23] & 0xC0) >> 6));

                    ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                    //object_type 2 bits,
                    //sampling_frequency_index 4 bits,
                    //num_front_channel_elements 4 bits,
                    //num_side_channel_elements 4 bits,
                    //num_back_channel_elements 4 bits,
                    //num_lfe_channel_elements 2 bits,
                    //num_assoc_data_elements 3 bits,
                    //num_valid_cc_elements 4 bits

                    // check mono_mixdown_present
                    if ((pBuffer[23] & 0x20) != 0)  //mono mixdown is presented
                    {
                        ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                        //mono_mixdown_element_number 4 bits

                        //check stereo_mixdown_present
                        if ((pBuffer[23] & 0x1) != 0)   //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[24] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[24] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[23] & 0x1) != 0)
                    }
                    else    //mono mixdown is not presented
                    {
                        ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                        //check stereo_mixdown_present
                        if ((pBuffer[23] & 0x10) != 0)  //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[24] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[23] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[23] & 0x10) != 0)

                    }//if ((pBuffer[23] & 0x20) != 0)

                }
                else    //bistream type is variable rate bitstream
                {
                    // get audio object type
                    iAudioObjectType = (uint8_t)((pBuffer[17] & 0x18) >> 3);

                    // get sampling rate index
                    iSampleFreqIndex = sampleFreqIndex = (uint8_t)(((pBuffer[17] & 0x7) << 1) | ((pBuffer[18] & 0x80) >> 7));

                    // get number of front channel elements
                    numFrontChanElem = (uint32_t)((pBuffer[18] & 0x78) >> 3);

                    // get number of side channel elements
                    numSideChanElem = (uint32_t)(((pBuffer[18] & 0x7) << 1) | ((pBuffer[19] & 0x80) >> 7));

                    // get number of back channel elements
                    numBackChanElem = (uint32_t)((pBuffer[19] & 0x78) >> 3);

                    // get number of LFE channel elements
                    numLfeChanElem = (uint32_t)((pBuffer[19] & 0x6) >> 1);

                    // get number of assoc data elements
                    numAssocDataElem = (uint32_t)(((pBuffer[19] & 0x1) << 2) | ((pBuffer[20] & 0xC0) >> 6));

                    // get number of valid CC elements
                    numValidCCElem = (uint32_t)((pBuffer[20] & 0x3C) >> 2);

                    ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                    //object_type 2 bits,
                    //sampling_frequency_index 4 bits,
                    //num_front_channel_elements 4 bits,
                    //num_side_channel_elements 4 bits,
                    //num_back_channel_elements 4 bits,
                    //num_lfe_channel_elements 2 bits,
                    //num_assoc_data_elements 3 bits,
                    //num_valid_cc_elements 4 bits

                    // check mono_mixdown_present
                    if ((pBuffer[20] & 0x2) != 0)   //mono mixdown is presented
                    {
                        ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                        //mono_mixdown_element_number 4 bits

                        //check stereo_mixdown_present
                        if ((pBuffer[21] & 0x10) != 0)  //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[22] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[21] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[21] & 0x10) != 0)
                    }
                    else    //mono mixdown is not presented
                    {
                        ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                        //check stereo_mixdown_present
                        if ((pBuffer[20] & 0x1) != 0)   //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[21] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[21] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[20] & 0x1) != 0)

                    }//if ((pBuffer[20] & 0x2) != 0)

                }// if (bitstreamType == 0)

                for (i = 0; i < (int32_t)numFrontChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //front channel element takes 1 bit
                    iChannelConfig += tmp;

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //front_element_is_cpe[i] 1 bit,
                    //front_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numSideChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //side channel element takes 1 bit
                    iChannelConfig += (tmp + 1);

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                    //side_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numBackChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //back channel element takes 1 bit
                    iChannelConfig += (tmp + 1);

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //back_element_is_cpe[i] 1 bit,
                    //back_element_tag_select[i] 4 bits
                }

                if (numLfeChanElem != 0)
                {
                    iChannelConfig++;   //1 front low frequency effects speaker
                }

                for (i = 0; i < (int32_t)numLfeChanElem; i++)
                {
                    ADIFHeaderLen += 4; //lfe_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numAssocDataElem; i++)
                {
                    ADIFHeaderLen += 4; //assoc_data_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numValidCCElem; i++)
                {
                    ADIFHeaderLen += 5; //cc_element_is_ind_sw[i] 1 bit,
                    //valid_cc_element_tag_select[i] 4 bits
                }

                // byte_allignment
                ADIFHeaderLen += 7;
                ADIFHeaderLen &= 0xF8;

                // comment_field_bytes (8 bits)
                offset = ADIFHeaderLen >> 3;
                bitIndex = ADIFHeaderLen & 0x7;
                commentFieldBytes = (pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex));

                ADIFHeaderLen += 8; //comment_field_bytes 8 bits

                for (i = 0; i < (int32_t)commentFieldBytes; i++)
                {
                    ADIFHeaderLen += 8; //comment_field_data 8 bits
                }

            }// for (i = 0; i < (int32)numProgConfigElem + 1; i++)

        }
        else    //copyright ID is not presented
        {
            ADIFHeaderLen += 3;     // copyright_id_present 1 bit
            // original_copy 1 bit,
            // home 1 bit,

            // check bitstream type
            bitstreamType = pBuffer[4] & 0x10;

            // get number of program config. element
            numProgConfigElem = (pBuffer[7] & 0x1E) >> 1;

            // get bitrate (max for variable rate bitstream)
            /*
            iBitrate  = bitRate = ((pBuffer[4] & 0xF0) << 15) |
                                  (pBuffer[5] << 11) |
                                  (pBuffer[6] << 3)  |
                                  ((pBuffer[7] & 0xE0) >> 5);
                                  */
            //Modified by HP Cheng, fix the bug of getting bit rate
            iBitrate  = bitRate = ((pBuffer[4] & 0x0F) << 19) |
                                  (pBuffer[5] << 11) |
                                  (pBuffer[6] << 3)  |
                                  ((pBuffer[7] & 0xE0) >> 5);


            if (iBitrate == 0)  //bitrate is not known
            {
                AAC_EXTR_ERR("findADIFHeader--the adif bitrate is 0");
                return false;
                //PVMF_AACPARSER_LOGERROR((0, "AACBitstreamObject::getFileInfo- Misc Error"));
                //return AACBitstreamObject::MISC_ERROR;
            }

            ADIFHeaderLen += 28;    // bitstream_type 1 bit,
            // bitrate 23 bits
            // num_program_config_elements 4 bits

            for (i = 0; i < (int32_t)numProgConfigElem + 1; i++)
            {
                if (bitstreamType == 0) //bistream type is constant rate bitstream
                {
                    ADIFHeaderLen += 20;    //adif_buffer_fullness 20 bits

                    // get audio object type
                    iAudioObjectType = (uint8_t)(((pBuffer[10] & 0x1) << 1) | ((pBuffer[11] & 0x80) >> 7));

                    // get sampling rate index
                    iSampleFreqIndex = sampleFreqIndex = (uint8_t)((pBuffer[11] & 0x78) >> 3);

                    // get number of front channel elements
                    numFrontChanElem = (uint32_t)(((pBuffer[11] & 0x7) << 1) | ((pBuffer[12] & 0x80) >> 7));

                    // get number of side channel elements
                    numSideChanElem = (uint32_t)((pBuffer[12] & 0x78) >> 3);

                    // get number of back channel elements
                    numBackChanElem = (uint32_t)(((pBuffer[12] & 0x7) << 1) | ((pBuffer[13] & 0x80) >> 7));

                    // get number of LFE channel elements
                    numLfeChanElem = (uint32_t)((pBuffer[13] & 0x60) >> 5);

                    // get number of assoc data elements
                    numAssocDataElem = (uint32_t)((pBuffer[13] & 0x1C) >> 2);

                    // get number of valid CC elements
                    numValidCCElem = (uint32_t)(((pBuffer[13] & 0x3) << 2) | ((pBuffer[14] & 0xC0) >> 6));

                    ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                    //object_type 2 bits,
                    //sampling_frequency_index 4 bits,
                    //num_front_channel_elements 4 bits,
                    //num_side_channel_elements 4 bits,
                    //num_back_channel_elements 4 bits,
                    //num_lfe_channel_elements 2 bits,
                    //num_assoc_data_elements 3 bits,
                    //num_valid_cc_elements 4 bits

                    // check mono_mixdown_present
                    if ((pBuffer[14] & 0x20) != 0)  //mono mixdown is presented
                    {
                        ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                        //mono_mixdown_element_number 4 bits

                        //check stereo_mixdown_present
                        if ((pBuffer[14] & 0x1) != 0)   //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[15] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[15] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[14] & 0x1) != 0)
                    }
                    else    //mono mixdown is not presented
                    {
                        ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                        //check stereo_mixdown_present
                        if ((pBuffer[14] & 0x10) != 0)  //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[15] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[14] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[14] & 0x10) != 0)

                    }//if ((pBuffer[14] & 0x20) != 0)

                }
                else    //bistream type is variable rate bitstream
                {
                    // get audio object type
                    iAudioObjectType = (uint8_t)((pBuffer[8] & 0x18) >> 3);

                    // get sampling rate index
                    iSampleFreqIndex = sampleFreqIndex = (uint8_t)(((pBuffer[8] & 0x7) << 1) | ((pBuffer[9] & 0x80) >> 7));

                    // get number of front channel elements
                    numFrontChanElem = (uint32_t)((pBuffer[9] & 0x78) >> 3);

                    // get number of side channel elements
                    numSideChanElem = (uint32_t)(((pBuffer[9] & 0x7) << 1) | ((pBuffer[10] & 0x80) >> 7));

                    // get number of back channel elements
                    numBackChanElem = (uint32_t)((pBuffer[10] & 0x78) >> 3);

                    // get number of LFE channel elements
                    numLfeChanElem = (uint32_t)((pBuffer[10] & 0x6) >> 1);

                    // get number of assoc data elements
                    numAssocDataElem = (uint32_t)(((pBuffer[10] & 0x1) << 2) | ((pBuffer[11] & 0xC0) >> 6));

                    // get number of valid CC elements
                    numValidCCElem = (uint32_t)((pBuffer[11] & 0x3C) >> 2);

                    ADIFHeaderLen += 31;    //element_instance_tag 4 bits,
                    //object_type 2 bits,
                    //sampling_frequency_index 4 bits,
                    //num_front_channel_elements 4 bits,
                    //num_side_channel_elements 4 bits,
                    //num_back_channel_elements 4 bits,
                    //num_lfe_channel_elements 2 bits,
                    //num_assoc_data_elements 3 bits,
                    //num_valid_cc_elements 4 bits

                    // check mono_mixdown_present
                    if ((pBuffer[11] & 0x2) != 0)   //mono mixdown is presented
                    {
                        ADIFHeaderLen += 5; //mono_mixdown_present 1 bit
                        //mono_mixdown_element_number 4 bits

                        //check stereo_mixdown_present
                        if ((pBuffer[12] & 0x10) != 0)  //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[13] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[12] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[12] & 0x10) != 0)
                    }
                    else    //mono mixdown is not presented
                    {
                        ADIFHeaderLen += 1; //mono_mixdown_present 1 bit

                        //check stereo_mixdown_present
                        if ((pBuffer[11] & 0x1) != 0)   //stereo mixdown is presented
                        {
                            ADIFHeaderLen += 5; //stereo_mixdown_present 1 bit
                            //stereo_mixdown_element_number 4 bits

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[12] & 0x8) != 0)   //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }
                        else    //stereo mixdown is not presented
                        {
                            ADIFHeaderLen += 1; //stereo_mixdown_present 1 bit

                            //check matrix_mixdown_idx_present
                            if ((pBuffer[12] & 0x80) != 0)  //matrix mixdown is presented
                            {
                                ADIFHeaderLen += 4; //matrix_mixdown_idx_present 1 bit
                                //matrix_mixdown_idx 2 bits
                                //pseudo_surround_enable 1 bit
                            }
                            else    //matrix mixdown is not presented
                            {
                                ADIFHeaderLen += 1; //matrix_mixdown_idx_present 1 bit
                            }
                        }//if ((pBuffer[11] & 0x1) != 0)

                    }//if ((pBuffer[11] & 0x2) != 0)

                }// if (bitstreamType == 0)

                for (i = 0; i < (int32_t)numFrontChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //front channel element takes 1 bit
                    iChannelConfig += tmp;

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //front_element_is_cpe[i] 1 bit,
                    //front_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numSideChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //side channel element takes 1 bit
                    iChannelConfig += (tmp + 1);

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                    //side_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numBackChanElem; i++)
                {
                    //calculate channel configuration
                    offset = ADIFHeaderLen >> 3;
                    bitIndex = ADIFHeaderLen & 0x7;
                    tmp = (uint8_t)((pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex)));
                    tmp >>= (8 - 1); //back channel element takes 1 bit
                    iChannelConfig += (tmp + 1);

                    //update ADIF variable header length
                    ADIFHeaderLen += 5; //side_element_is_cpe[i] 1 bit,
                    //side_element_tag_select[i] 4 bits
                }

                if (numLfeChanElem != 0)
                {
                    iChannelConfig++;   //1 front low frequency effects speaker
                }

                for (i = 0; i < (int32_t)numLfeChanElem; i++)
                {
                    ADIFHeaderLen += 4; //lfe_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numAssocDataElem; i++)
                {
                    ADIFHeaderLen += 4; //assoc_data_element_tag_select[i] 4 bits
                }

                for (i = 0; i < (int32_t)numValidCCElem; i++)
                {
                    ADIFHeaderLen += 5; //cc_element_is_ind_sw[i] 1 bit,
                    //valid_cc_element_tag_select[i] 4 bits
                }

                // byte_allignment
                ADIFHeaderLen += 7;
                ADIFHeaderLen &= 0xF8;

                // comment_field_bytes (8 bits)
                offset = ADIFHeaderLen >> 3;
                bitIndex = ADIFHeaderLen & 0x7;
                commentFieldBytes = (pBuffer[offset] << bitIndex) | (pBuffer[offset + 1] >> (8 - bitIndex));

                ADIFHeaderLen += 8; //comment_field_bytes 8 bits

                for (i = 0; i < (int32_t)commentFieldBytes; i++)
                {
                    ADIFHeaderLen += 8; //comment_field_data 8 bits
                }

            }// for (i = 0; i < (int32)numProgConfigElem + 1; i++)

        } // if(id!=0)

        // ADIF header length in bits
        //iADIFHeaderLen = HeaderLen = ADIFHeaderLen;
        *inout_pos += ADIFHeaderLen >> 3;
        *pChannelNum = iChannelConfig + 1;
        *pSampleFreqIndex = sampleFreqIndex;
        *pBitRate = iBitrate;
        *pProfile = iAudioObjectType;

        return true;
    }
    else
    {
        return false;
    }
}

static bool getAACInfo(
    const sp<DataSource> &source, off_t *inout_pos, uint32_t *out_header, uint32_t *pSampleFreqIndex,
    uint32_t *pBitRate, uint32_t *pProfile, uint32_t *pChannelNum, int32_t *pIsADIF)
{
    //uint32_t nSampleFreqIndex;
    uint32_t header;
    off_t pos = 0;

    if (findADTSSyncword(source, inout_pos, out_header))
    {
        header = *out_header;
        pos = *inout_pos;

        *pIsADIF = false;
        *pSampleFreqIndex = (header >> 10) & 0x0F;
        *pProfile = (header >> 14) & 0x03;
        *pChannelNum = (header >> 6) & 0x07;

        AAC_EXTR_DBG("getAACInfo -- ADTS");

        return true;
    }
    else if (findADIFHeader(source, inout_pos, out_header, pSampleFreqIndex, pBitRate, pProfile, pChannelNum))
    {
        *pIsADIF = true;

        AAC_EXTR_DBG("getAACInfo -- ADIF");
        AAC_EXTR_DBG("getAACInfo -- pos=%d", *inout_pos);

        return true;
    }

    return false;
}

#ifdef USE_FRAUNHOFER_AAC

// <---   add for fraunhofer AAC seek table
class AACFrameLenDecoder
{
public:
    AACFrameLenDecoder(
        const sp<MetaData> &meta, const sp<DataSource> &source,
        off_t first_frame_pos);

    status_t getNextFramePos(off_t *pCurPos, off_t *pNextPos);
    bool initAACDec();
    void deinitAACDec();

    //protected:
    virtual ~AACFrameLenDecoder();

private:

    sp<MetaData> mMeta;
    sp<DataSource> mDataSource;
    bool mInited;
    off_t mFirstFramePos;
    Mutex mLock;

    HANDLE_AACDECODER mAACDecoder;
    CStreamInfo *mStreamInfo;
    uint8_t     *mTmpBuf;
    uint8_t     *mOutBuf;
    uint32_t     mTmpBufLen;
    uint32_t     mInterDecBufSize;
    uint32_t     mCurPosition;
    bool         mReadFlag;
    uint32_t     mLastNumTotalBytes;

    off_t        mFramePosition;
    off_t        mCurFliePos;

};

AACFrameLenDecoder::AACFrameLenDecoder(
    const sp<MetaData> &meta, const sp<DataSource> &source,
    off_t first_frame_pos)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mInited(false)
{

    AAC_EXTR_VEB("FRAUNHOFER_AAC :AACFrameLenDecoder!");
    mCurFliePos = first_frame_pos;
    mFramePosition = first_frame_pos;
    mTmpBuf = NULL;
    mOutBuf = NULL;
    mTmpBufLen = 0;
    mInterDecBufSize = 0;
    mCurPosition = 0;
    mReadFlag = true;
    mLastNumTotalBytes = 0;
}
AACFrameLenDecoder::~AACFrameLenDecoder()
{
    AAC_EXTR_VEB("FRAUNHOFER_AAC :~AACFrameLenDecoder!");
    deinitAACDec();
}

bool AACFrameLenDecoder::initAACDec()
{
    AAC_EXTR_VEB("FRAUNHOFER_AAC--initAACDec");

    if (mInited)
    {
        return true;
    }

    mAACDecoder = aacDecoder_Open(TT_MP4_ADIF, /* num layers */ 1);

    if (AAC_DEC_OK != aacDecoder_GetFreeBytes(mAACDecoder, (UINT *)&mInterDecBufSize))
    {
        AAC_EXTR_ERR("initDecoder get inbuf size failure");
        return false;
    }

    AAC_EXTR_VEB("mInterDecBufSize %d", mInterDecBufSize);

    if (mAACDecoder != NULL)
    {
        mStreamInfo = aacDecoder_GetStreamInfo(mAACDecoder);

        if (mStreamInfo == NULL)
        {
            AAC_EXTR_ERR("FRAUNHOFER_AAC--initAACDec:aacDecoder_GetStreamInfo error!");
            return false;
        }
    }
    else
    {
        AAC_EXTR_ERR("FRAUNHOFER_AAC--initAACDec:aacDecoder_Open fail!");
        return false;
    }

    mTmpBuf = (uint8_t *) malloc(ADIF_DEC_INBUF_LEN);

    if (!mTmpBuf)
    {
        AAC_EXTR_ERR("initDecoder:adif temp buffer alloc fail!");
        return false;
    }
    else
    {
        memset(mTmpBuf, 0x00, ADIF_DEC_INBUF_LEN);
    }

    mOutBuf = (uint8_t *) malloc(ADIF_DEC_OUTBUF_LEN);

    if (!mOutBuf)
    {
        AAC_EXTR_ERR("initDecoder:adif temp buffer alloc fail!");
        return false;
    }
    else
    {
        memset(mOutBuf, 0x00, ADIF_DEC_OUTBUF_LEN);
    }

    mCurFliePos = mFirstFramePos;//file position
    mFramePosition = mCurFliePos; //frame position

    uint32_t type;
    const void *data;
    size_t size;

    if (mMeta->findData(kKeyESDS, &type, &data, &size))
    {
        const void *codec_specific_data;
        size_t codec_specific_data_size;
        ESDS esds((const char *)data, size);

        if (OK != esds.InitCheck())
        {
            return false;
        }

        esds.getCodecSpecificInfo(
            &codec_specific_data, &codec_specific_data_size);
        AAC_EXTR_DBG("FRAUNHOFER_AAC--initAACDec codec_specific_data_size =%d", codec_specific_data_size);

        AAC_DECODER_ERROR decoderErr = aacDecoder_ConfigRaw(mAACDecoder,
                                       (UCHAR **)&codec_specific_data,
                                       (UINT *)&size);

        if (decoderErr != AAC_DEC_OK)
        {
            AAC_EXTR_ERR("FRAUNHOFER_AAC--aacDecoder_ConfigRaw return err %d", decoderErr);
            return false;
        }
    }

    mInited = true;
    return true;
}

void AACFrameLenDecoder::deinitAACDec()
{
    AAC_EXTR_VEB("FRAUNHOFER_AAC--deinitAACDec");

    if (NULL != mTmpBuf)
    {
        free(mTmpBuf);
        mTmpBuf = NULL;
    }

    if (NULL != mOutBuf)
    {
        free(mOutBuf);
        mOutBuf = NULL;
    }

    aacDecoder_Close(mAACDecoder);
    mTmpBufLen = 0;
    mCurPosition = 0;
    mInited = false;
    mReadFlag = true;
}
status_t AACFrameLenDecoder::getNextFramePos(off_t *pCurPos, off_t *pNextPos)
{
    Mutex::Autolock autoLock(mLock);
    int consumBsLen = 0;
    ssize_t size = 0;
    UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};
    UCHAR *pFrameDecodeBuffer[FILEREAD_MAX_LAYERS];

    AAC_EXTR_VEB("|---------------getNextFramePos:*pCurPos %d mFramePosition %d mTmpBufLen %d-------------------------|", *pCurPos, mFramePosition, mTmpBufLen);

    if (*pCurPos != mFramePosition)
    {
        aacDecoder_SetParam(mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
        mTmpBufLen = 0;
        mCurPosition = 0;
        mReadFlag = true;
        size = mDataSource->readAt(*pCurPos, mTmpBuf, ADIF_DEC_INBUF_LEN);

        if (size <= 0)
        {
            return ERROR_END_OF_STREAM;
        }

        mCurFliePos = *pCurPos + size;
        mFramePosition = *pCurPos;
        mTmpBufLen += size;
        mLastNumTotalBytes = 0;
        AAC_EXTR_VEB("Seek *pCurPos %ld mCurFliePos =%ld mFramePosition = %ld mTmpBufLen %d", *pCurPos, mCurFliePos, mFramePosition, mTmpBufLen);
    }

    if (mReadFlag)
    {
        if (mTmpBufLen < ADIF_DEC_INBUF_LEN / 2) //Fill
        {
            if (mCurPosition > 0)
            {
                //move
                AAC_EXTR_VEB("move mCurPosition %d mTempBufLen %d", mCurPosition, mTmpBufLen);
                memcpy(mTmpBuf, mTmpBuf + mCurPosition, mTmpBufLen);
                mCurPosition = 0;
            }

            AAC_EXTR_VEB("mCurFliePos %d,mTmpBuf %p mCurPosition %d read %d", mCurFliePos, mTmpBuf, mCurPosition, ADIF_DEC_INBUF_LEN - mTmpBufLen);
            size = mDataSource->readAt(mCurFliePos, mTmpBuf + mTmpBufLen, ADIF_DEC_INBUF_LEN - mTmpBufLen);

            if (size <= 0)
            {
                mReadFlag = false;
                AAC_EXTR_VEB("Read all data from file,don't read file");
            }
            else
            {
                mCurFliePos += size;
                mTmpBufLen += size;
            }
        }

        AAC_EXTR_VEB("mCurFliePos %d mReadFlag :mTmpBufLen %d , mCurPosition %d size %d", mCurFliePos, mTmpBufLen, mCurPosition, size);
    }

    AAC_DECODER_ERROR decoderErr;
    // Fill and decode
    INT_PCM *outBuffer = reinterpret_cast<INT_PCM *>(mOutBuf);
    bytesValid[0] = mTmpBufLen;

    if (mTmpBufLen > 0)
    {
        pFrameDecodeBuffer[0] = mTmpBuf + mCurPosition;
        decoderErr = AAC_DEC_NOT_ENOUGH_BITS;
        AAC_EXTR_VEB("mTmpBufLen >0  inbuf %p mTmpBuf %p mCurPosition %d", pFrameDecodeBuffer[0], mTmpBuf, mCurPosition);

        while (bytesValid[0] > 0 && decoderErr == AAC_DEC_NOT_ENOUGH_BITS)
        {
            AAC_EXTR_VEB("--Fill bytesValid[0] %d mTmpBufLen %d mCurPosition %d--", bytesValid[0], mTmpBufLen, mCurPosition);
            aacDecoder_Fill(mAACDecoder,
                            pFrameDecodeBuffer,
                            (UINT *)&mTmpBufLen,
                            (UINT *)bytesValid);
            AAC_EXTR_VEB("--After Fill bytesValid[0] %d mTmpBufLen %d--", bytesValid[0], mTmpBufLen);
            decoderErr = aacDecoder_DecodeFrame(mAACDecoder,
                                                outBuffer,
                                                ADIF_DEC_OUTBUF_LEN,
                                                AACDEC_BYPASS);

            if (decoderErr == AAC_DEC_NOT_ENOUGH_BITS)
            {
                AAC_EXTR_WARN("Not enough bits, bytesValid %d", bytesValid[0]);
            }
        }

        int inBufferUsedLength = mTmpBufLen - bytesValid[0];

        if (AAC_DEC_OK == decoderErr && inBufferUsedLength >= 0)
        {
            *pCurPos = mFramePosition;
            consumBsLen = mStreamInfo->numTotalBytes - mLastNumTotalBytes;

            if (consumBsLen <= 0)
            {
                AAC_EXTR_WARN("ERROR_END_OF_STREAM consumBsLen %d", consumBsLen);
                return ERROR_END_OF_STREAM;
            }
        }
        else
        {
            AAC_EXTR_WARN("Error 0x%x  consumed %d", decoderErr, mStreamInfo->numTotalBytes - mLastNumTotalBytes);
            *pCurPos = mFramePosition;
            uint32_t DecFreeBytes = 0;
            decoderErr = aacDecoder_GetFreeBytes(mAACDecoder, (UINT *)&DecFreeBytes);

            if (AAC_DEC_OK != decoderErr)
            {
                AAC_EXTR_WARN("aacDecoder_GetFreeBytes err 0x%x", decoderErr);
                return ERROR_END_OF_STREAM;
            }

            consumBsLen = (mInterDecBufSize - DecFreeBytes) + (mStreamInfo->numTotalBytes - mLastNumTotalBytes);
            //discard buffer
            aacDecoder_SetParam(mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
            AAC_EXTR_WARN("Remain internal buffer len %d ", mInterDecBufSize - DecFreeBytes);
        }

        *pNextPos = mFramePosition + consumBsLen;
        mFramePosition += consumBsLen;

        mTmpBufLen -= inBufferUsedLength;

        if (mTmpBufLen == 0)
        {
            mCurPosition = 0;
        }
        else
        {
            mCurPosition += inBufferUsedLength;
        }

        mLastNumTotalBytes = mStreamInfo->numTotalBytes;
        AAC_EXTR_VEB("mCurPosition %d inBufferUsedLength %d mTmpBufLen %d consumBsLen %d mStreamInfo->numTotalBytes %d", mCurPosition, inBufferUsedLength, mTmpBufLen, consumBsLen, mStreamInfo->numTotalBytes);
    }
    else
    {
        AAC_EXTR_VEB("have no tmp buf");
        decoderErr = aacDecoder_DecodeFrame(mAACDecoder,
                                            outBuffer,
                                            ADIF_DEC_OUTBUF_LEN,
                                            AACDEC_BYPASS);

        if (AAC_DEC_OK != decoderErr)
        {
            AAC_EXTR_WARN("Seek Table ERROR_END_OF_STREAM decoderErr 0x%x", decoderErr);
            aacDecoder_SetParam(mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
            return ERROR_END_OF_STREAM;
        }
        else
        {
            *pCurPos = mFramePosition;
            consumBsLen = mStreamInfo->numTotalBytes - mLastNumTotalBytes;

            if (consumBsLen <= 0)
            {
                AAC_EXTR_WARN("3.ERROR_END_OF_STREAM");
                return ERROR_END_OF_STREAM;
            }

            *pNextPos = mFramePosition + consumBsLen;
            mFramePosition += consumBsLen;
            mLastNumTotalBytes = mStreamInfo->numTotalBytes;
            AAC_EXTR_VEB("consumBsLen %d,", consumBsLen);
        }
    }

    AAC_EXTR_VEB("getNextFramePos end *pCurPos %d,*pNextPos %d consumBsLen %d", *pCurPos, *pNextPos, consumBsLen);
    return OK;
}

//<---   add  fraunhoferAAC seek table
#else

#ifdef USE_PV_AAC
//   add for AAC seek table --->
class AACFrameLenDecoder
{
public:
    AACFrameLenDecoder(
        const sp<MetaData> &meta, const sp<DataSource> &source,
        off_t first_frame_pos);

    status_t getNextFramePos(off_t *pCurPos, off_t *pNextPos);
    bool initAACDec();
    void deinitAACDec();

    //protected:
    virtual ~AACFrameLenDecoder();

private:
    sp<MetaData> mMeta;
    sp<DataSource> mDataSource;
    off_t mFirstFramePos;
    bool mInited;
    Mutex mLock;

    uint8_t     *mInbufPtr;
    uint8_t     *mInbufStartPtr;
    int16_t     *mOutbufPtr;
    uint32_t    mInbufLen;
    uint32_t    mInBufSize;
    off_t       mCurFliePos;
    tPVMP4AudioDecoderExternal *mConfig;
    void        *mDecoderBuf;
};

AACFrameLenDecoder::AACFrameLenDecoder(
    const sp<MetaData> &meta, const sp<DataSource> &source,
    off_t first_frame_pos)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mInited(false)
{
    mInbufPtr = NULL;
    mInbufStartPtr = NULL;
    mOutbufPtr = NULL;
    mInBufSize = 0;
    mCurFliePos = first_frame_pos;
    mConfig = NULL;
    mDecoderBuf = NULL;
}

AACFrameLenDecoder::~AACFrameLenDecoder()
{
    deinitAACDec();
}

bool AACFrameLenDecoder::initAACDec()
{
    if (mInited)
    {
        return true;
    }

    mInbufStartPtr = (uint8_t *)malloc(ADIF_DEC_INBUF_LEN);

    if (NULL == mInbufStartPtr)
    {
        return false;
    }

    mInbufPtr = mInbufStartPtr;

    mOutbufPtr = (int16_t *)malloc(ADIF_DEC_OUTBUF_LEN * 2);

    if (NULL == mOutbufPtr)
    {
        return false;
    }

    mInBufSize = 0;
    mCurFliePos = mFirstFramePos;

    mConfig = new tPVMP4AudioDecoderExternal;

    if (NULL == mConfig)
    {
        return false;
    }

    memset(mConfig, 0, sizeof(tPVMP4AudioDecoderExternal));
    mConfig->outputFormat = OUTPUTFORMAT_16PCM_INTERLEAVED;
    mConfig->aacPlusEnabled = 1;  // Fix me for remove HE AAC fun

    // The software decoder doesn't properly support mono output on
    // AACplus files. Always output stereo.
    mConfig->desiredChannels = 2;

    UInt32 memRequirements = PVMP4AudioDecoderGetMemRequirements();
    mDecoderBuf = malloc(memRequirements);

    if (NULL == mDecoderBuf)
    {
        return false;
    }

    status_t err = PVMP4AudioDecoderInitLibrary(mConfig, mDecoderBuf);

    if (err != MP4AUDEC_SUCCESS)
    {
        AAC_EXTR_ERR("MtkAACSource::initAACDec--Failed to initialize MP4 audio decoder");
        return false;
    }

    uint32_t type;
    const void *data;
    size_t size;

    if (mMeta->findData(kKeyESDS, &type, &data, &size))
    {
        const void *codec_specific_data;
        size_t codec_specific_data_size;
        ESDS esds((const char *)data, size);

        if (OK != esds.InitCheck())
        {
            return false;
        }

        esds.getCodecSpecificInfo(
            &codec_specific_data, &codec_specific_data_size);
        AAC_EXTR_DBG("initAACDec codec_specific_data_size =%d", codec_specific_data_size);
        mConfig->pInputBuffer = (UChar *)codec_specific_data;
        mConfig->inputBufferCurrentLength = size;
#if 0
        mConfig->pInputBuffer = (UChar *)data;
        mConfig->inputBufferCurrentLength = size;
#endif
        mConfig->inputBufferMaxLength = 0;

        mConfig->pOutputBuffer = mOutbufPtr;
        mConfig->pOutputBuffer_plus = &mConfig->pOutputBuffer[2048];

        if (PVMP4AudioDecoderConfig(mConfig, mDecoderBuf)
                != MP4AUDEC_SUCCESS)
        {
            return false;
        }
    }

    mInited = true;
    return true;
}

void AACFrameLenDecoder::deinitAACDec()
{
    if (NULL != mInbufStartPtr)
    {
        free(mInbufStartPtr);
        mInbufStartPtr = NULL;
        mInbufPtr = NULL;
    }

    if (NULL != mOutbufPtr)
    {
        free(mOutbufPtr);
        mOutbufPtr = NULL;
    }

    if (NULL != mConfig)
    {
        delete mConfig;
        mConfig = NULL;
    }

    if (NULL != mDecoderBuf)
    {
        free(mDecoderBuf);
        mDecoderBuf = NULL;
    }

    mInited = false;
}

status_t AACFrameLenDecoder::getNextFramePos(off_t *pCurPos, off_t *pNextPos)
{
    {
        Mutex::Autolock autoLock(mLock);

        if (*pCurPos != mCurFliePos)
        {
            mInBufSize = mDataSource->readAt(*pCurPos, mInbufStartPtr, ADIF_DEC_BUFFER_SIZE);

            if (mInBufSize <= 0)
            {
                return ERROR_END_OF_STREAM;
            }

            mInbufPtr = mInbufStartPtr;
            mCurFliePos = *pCurPos;
        }
    }

    while (1)
    {
        Mutex::Autolock autoLock(mLock);

        if (0 == mInBufSize) // refill in bufffer
        {
            mInBufSize = mDataSource->readAt(mCurFliePos, mInbufStartPtr, ADIF_DEC_BUFFER_SIZE);

            if (mInBufSize <= 0)
            {
                return ERROR_END_OF_STREAM;
            }

            mInbufPtr = mInbufStartPtr;
        }

        mConfig->pInputBuffer = mInbufPtr;

        mConfig->inputBufferCurrentLength = mInBufSize;
        mConfig->inputBufferMaxLength = 0;
        mConfig->inputBufferUsedLength = 0;
        mConfig->remainderBits = 0;

        mConfig->pOutputBuffer = mOutbufPtr;
        mConfig->pOutputBuffer_plus = &mConfig->pOutputBuffer[2048];
        mConfig->repositionFlag = false;
        Int decoderErr = 1;
        decoderErr = PVMP4AudioDecodeFrameTwo(mConfig, mDecoderBuf);

        if (MP4AUDEC_SUCCESS == decoderErr && mConfig->inputBufferUsedLength > 0 && (uint32_t)(mConfig->inputBufferUsedLength) <= mInBufSize)
        {
            //LOGV("mConfig->inputBufferUsedLength %d",mConfig->inputBufferUsedLength);
            mInBufSize -= mConfig->inputBufferUsedLength;
            mInbufPtr += mConfig->inputBufferUsedLength;
            *pCurPos = mCurFliePos;
            *pNextPos = mCurFliePos + mConfig->inputBufferUsedLength;
            mCurFliePos += mConfig->inputBufferUsedLength;
            break;
        }
        else if (MP4AUDEC_INCOMPLETE_FRAME == decoderErr)
        {
            uint32_t size = 0;

            //AAC_EXTR_WARN("1.mInBufSize %d",mInBufSize);
            if (mInBufSize < ADIF_DEC_BUFFER_SIZE)
            {
                memcpy(mInbufStartPtr, mInbufPtr, mInBufSize);
                size = mDataSource->readAt(mCurFliePos + mInBufSize, mInbufStartPtr + mInBufSize, ADIF_DEC_BUFFER_SIZE - mInBufSize); // refill to ADIF_DEC_BUFFER_SIZE
            }
            else
            {
                size = mDataSource->readAt(mCurFliePos + mInBufSize, mInbufStartPtr, ADIF_DEC_BUFFER_SIZE);
                mCurFliePos += mInBufSize;
                mInBufSize = 0;
            }

            if (size <= 0)
            {
                return ERROR_END_OF_STREAM;
            }

            mInbufPtr = mInbufStartPtr;
            mInBufSize += size;
            //AAC_EXTR_WARN("2.mInBufSize %d",mInBufSize);
        }
        else
        {
            *pCurPos = mCurFliePos;
            *pNextPos = mCurFliePos + ADIF_DEC_BUFFER_SIZE;
            mInBufSize = 0;
            break;
        }

    }

    AAC_EXTR_VEB("*pCurPos %d *pNextPos %d", *pCurPos , *pNextPos);
    return OK;

}

#else // <---   add for AAC seek table 
#ifdef USE_MTK_AAC
//-----> add for mtk aac decoder
typedef struct
{

    HEAACDEC_HANDLE *pHEAACDecHdl;
    int InterBufSize;
    int TmpBufSize;
    int PcmBufSize;
    int BsBufSize;
    void *pInterBuf;
    void *pTmpBuf;
    void *pPcmBuf;
    void *pBsBuf;

} MtkAACDecEngine;

class AACFrameLenDecoder
{
public:
    AACFrameLenDecoder(
        const sp<MetaData> &meta, const sp<DataSource> &source,
        off_t first_frame_pos);

    virtual ~AACFrameLenDecoder();
public:
    status_t getNextFramePos(off_t *pCurPos, off_t *pNextPos);
    bool  initAACDec();
    void  deinitAACDec();
    int   DecodeFrame(off_t *pCurPos, off_t *pNextPos);
    bool  ReadFromFile(off_t *CurPos);
    void *GetCurrentPointer();
    uint32_t GetRemainCount();

protected:

private:
    sp<MetaData> mMeta;
    sp<DataSource> mDataSource;
    off_t mFirstFramePos;
    off_t mCurFliePos;
    bool mInited;
    Mutex mLock;
    MtkAACDecEngine *mAACDec;
    void *m_pBsRead;
    void *m_pBsWrite;
    void *m_pEndBsBuf;

    uint32_t mValidCount;
    uint32_t mReadOutCount;
    off_t mFramePosition;
    off64_t mFileSize;

};

AACFrameLenDecoder::AACFrameLenDecoder(
    const sp<MetaData> &meta, const sp<DataSource> &source,
    off_t first_frame_pos)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mInited(false)
{
    AAC_EXTR_DBG("AACFrameLenDecoder Construct! %ld", mCurFliePos);
    mCurFliePos = first_frame_pos;
    mFramePosition = first_frame_pos;
    mValidCount = 0;
    mReadOutCount = 0;
    mFileSize = 0;
    m_pBsRead = NULL;
    m_pBsWrite = NULL;
    m_pEndBsBuf = NULL;
}
AACFrameLenDecoder::~AACFrameLenDecoder()
{
    AAC_EXTR_VEB("~AACFrameLenDecoder Destruct");
    deinitAACDec();
}

bool AACFrameLenDecoder::initAACDec()
{
    if (mInited)
    {
        return true;
    }

    mAACDec = (MtkAACDecEngine *)malloc(sizeof(MtkAACDecEngine));

    if (NULL == mAACDec)
    {
        return false;
    }

    memset(mAACDec, 0, sizeof(MtkAACDecEngine));
    HEAACDec_GetMemSize(&mAACDec->InterBufSize, &mAACDec->TmpBufSize, &mAACDec->PcmBufSize, &mAACDec->BsBufSize);
    AAC_EXTR_VEB("Get SWIP Decoder Required Mem :");
    AAC_EXTR_VEB("    InterBufSize = 0x%lx", mAACDec->InterBufSize);
    AAC_EXTR_VEB("    TmpBufSize   = 0x%lx", mAACDec->TmpBufSize);
    AAC_EXTR_VEB("    PcmBufSize   = 0x%lx", mAACDec->PcmBufSize);
    AAC_EXTR_VEB("    BsBufSize    = 0x%lx", mAACDec->BsBufSize);

    mAACDec->pInterBuf = malloc(mAACDec->InterBufSize);

    if (NULL == mAACDec->pInterBuf)
    {
        AAC_EXTR_ERR("MTK_OMX_ALLOC InterBuf failure!");
        return OMX_FALSE;
    }
    else
    {
        memset(mAACDec->pInterBuf, 0, mAACDec->InterBufSize);
        AAC_EXTR_VEB("AAC InterBuf = 0x%lx-0x%lx", (unsigned int)(mAACDec->pInterBuf), (unsigned int)(mAACDec->pInterBuf) + mAACDec->InterBufSize);
    }

    mAACDec->pPcmBuf = malloc(mAACDec->PcmBufSize);

    if (NULL == mAACDec->pPcmBuf)
    {
        AAC_EXTR_ERR("MTK_OMX_ALLOC PcmBuf failure!");
        return OMX_FALSE;
    }
    else
    {
        memset(mAACDec->pPcmBuf, 0, mAACDec->PcmBufSize);
        AAC_EXTR_VEB("AAC PcmBuf = 0x%lx-0x%lx", (unsigned int)(mAACDec->pPcmBuf), (unsigned int)(mAACDec->pPcmBuf) + mAACDec->PcmBufSize);
    }

    mAACDec->pTmpBuf = malloc(mAACDec->TmpBufSize);

    if (NULL == mAACDec->pTmpBuf)
    {
        AAC_EXTR_ERR("MTK_OMX_ALLOC TmpBuf failure!");
        return OMX_FALSE;
    }
    else
    {
        memset(mAACDec->pTmpBuf, 0, mAACDec->TmpBufSize);
        AAC_EXTR_VEB("AAC pTmpBuf = 0x%lx-0x%lx", (unsigned int)(mAACDec->pTmpBuf), (unsigned int)(mAACDec->pTmpBuf) + mAACDec->TmpBufSize);
    }

    if (mAACDec->pBsBuf == NULL)
    {
        mAACDec->pBsBuf = malloc(mAACDec->BsBufSize);

        if (NULL == mAACDec->pBsBuf)
        {
            AAC_EXTR_ERR("MTK_OMX_ALLOC BsBuf failure!");
            return OMX_FALSE;
        }

        memset(mAACDec->pBsBuf, 0, mAACDec->BsBufSize);
        m_pBsRead = mAACDec->pBsBuf;
        m_pBsWrite = mAACDec->pBsBuf;
        m_pEndBsBuf = (OMX_U8 *)(mAACDec->pBsBuf) + mAACDec->BsBufSize;
    }

    AAC_EXTR_VEB("m_pBsRead %p,m_pBsWrite %p,m_pBsBuf %p,m_pEndBsBuf %p,BsBufSize %d", m_pBsRead, m_pBsWrite, mAACDec->pBsBuf, m_pEndBsBuf, mAACDec->BsBufSize);

    if (NULL == mAACDec->pHEAACDecHdl)
    {
        mAACDec->pHEAACDecHdl = HEAACDec_Init(mAACDec->pInterBuf, SBR_AUTO, SBR_MODE_AUTO, 0);
    }

    mCurFliePos = mFirstFramePos;//file position
    mFramePosition = mCurFliePos; //frame position
    mDataSource->getSize(&mFileSize);//get file size

    AAC_EXTR_VEB("mCurFliePos %ld,mFramePosition %ld mFileSize =%lld", mCurFliePos, mFramePosition, mFileSize);
    uint32_t type;
    const void *data;
    size_t size;

    if (mMeta->findData(kKeyESDS, &type, &data, &size))
    {
        const void *codec_specific_data;
        size_t codec_specific_data_size;
        ESDS esds((const char *)data, size);

        if (OK != esds.InitCheck())
        {
            return false;
        }

        esds.getCodecSpecificInfo(&codec_specific_data, &codec_specific_data_size);
        unsigned char  *pInputBuffer = (unsigned char *)codec_specific_data;
        unsigned int    audio_config[2];
        unsigned int    u4AudioObjectType;
        unsigned int    u4SamplingFreqIndex;
        unsigned char   u1ChannelNum;
        int Status;
        audio_config[0] = *((unsigned char *)codec_specific_data);
        audio_config[1] = *((unsigned char *)codec_specific_data + 1);

        // oooo offf fccc c000
        // o - audioObjectType
        // f - samplingFreqIndex
        // c - channelConfig
        u4AudioObjectType   = (unsigned int)((audio_config[0] >> 3) & 7);//3bit
        u4SamplingFreqIndex = (unsigned int)(((audio_config[0] & 7) << 1) + (audio_config[1] >> 7));//4bit
        u1ChannelNum        = (unsigned char)((audio_config[1] >> 3) & 0x0f);//4bit

        AAC_EXTR_VEB("audio_config[0]=%x,audio_config[1]=%x", audio_config[0], audio_config[1]);
        Status = HEAACDec_SetAudioSpecificConfig(mAACDec->pHEAACDecHdl, u4AudioObjectType, u4SamplingFreqIndex, u1ChannelNum);

        if (Status != 0)
        {
            AAC_EXTR_ERR(" Set Audio Config Error!");
            return false;
        }
        else
        {
            AAC_EXTR_VEB(" Set Audio Config : objtype = %d fs = %d chnum = %d", u4AudioObjectType, u4SamplingFreqIndex, u1ChannelNum);
        }

    }

    mInited = true;
    return true;

}


int AACFrameLenDecoder::DecodeFrame(off_t *pCurPos, off_t *pNextPos)
{
    int consumBsLen = 0;
    void *ptr = GetCurrentPointer();
    int Status = HEAACDec_DecodeFrameBypass(mAACDec->pHEAACDecHdl,
                                            mAACDec->pPcmBuf,
                                            mAACDec->pTmpBuf,
                                            ptr,
                                            GetRemainCount(),
                                            ptr,
                                            &consumBsLen);

    if ((consumBsLen <= 0) || (consumBsLen > GetRemainCount()))
    {
        consumBsLen = GetRemainCount();
    }

    mReadOutCount += consumBsLen;
    *pCurPos = mFramePosition;
    *pNextPos = mFramePosition + consumBsLen;
    mFramePosition += consumBsLen;
    //AAC_EXTR_VEB("DecodeFrame Status %d mReadOutCount=%d,*pCurPos %ld,*pNextPos %ld mFramePosition= %ld",Status,mReadOutCount,*pCurPos,*pNextPos,mFramePosition);
    return Status;
}


status_t AACFrameLenDecoder::getNextFramePos(off_t *pCurPos, off_t *pNextPos)
{

    Mutex::Autolock autoLock(mLock);
    int consumBsLen = 0;
    int Status;
    void *ptr;
    int remain = 0;

    if (*pCurPos != mFramePosition)
    {
        uint32_t size = mDataSource->readAt(*pCurPos, mAACDec->pBsBuf, mAACDec->BsBufSize);
        mValidCount = size;
        mReadOutCount = 0;
        mCurFliePos = *pCurPos + mValidCount;
        mFramePosition = *pCurPos;
        AAC_EXTR_VEB("Seek mCurFliePos =%ld mFramePosition = %ld mValidCount =%d size =%d mReadOutCount =%d", mCurFliePos, mFramePosition, mValidCount, size, mReadOutCount);
    }

    if (ReadFromFile(&mCurFliePos))
    {
        DecodeFrame(pCurPos, pNextPos);
    }
    else
    {
        remain = GetRemainCount();

        if (remain > 0)
        {
            DecodeFrame(pCurPos, pNextPos);
        }
        else
        {
            AAC_EXTR_VEB("getNextFramePos ERROR_END_OF_STREAMs");
            return ERROR_END_OF_STREAM;
        }
    }

    return OK;
}

bool AACFrameLenDecoder::ReadFromFile(off_t *CurPos)
{
    off_t size = 0;

    if (mReadOutCount)
    {
        memmove(mAACDec->pBsBuf, mAACDec->pBsBuf + mReadOutCount, mValidCount - mReadOutCount);
        mValidCount = mValidCount - mReadOutCount;
        mReadOutCount = 0;
        //AAC_EXTR_VEB("ReadFromFile mValidCount %d,mReadOutCount %d",mValidCount,mReadOutCount);
    }

    off_t remain = mAACDec->BsBufSize - mValidCount;

    if (*CurPos < mFileSize)
    {
        if (mValidCount < mAACDec->BsBufSize)
        {
            size = mDataSource->readAt(*CurPos, mAACDec->pBsBuf + mValidCount, remain);
        }
    }

    if (size <= 0)
    {
        //AAC_EXTR_VEB("ReadFromFile mValidCount %d CurPos %ld remain %d size %d",mValidCount,*CurPos,remain,size);
        return false;
    }
    else
    {
        mValidCount += size;
        *CurPos += size;
    }

    //AAC_EXTR_VEB("ReadFromFile mValidCount %d CurPos %ld remain %d size %d",mValidCount,*CurPos,remain,size);
    return true;
}

void *AACFrameLenDecoder::GetCurrentPointer()
{
    return (unsigned char *)(mAACDec->pBsBuf) + mReadOutCount;
}

uint32_t AACFrameLenDecoder::GetRemainCount()
{
    return mValidCount - mReadOutCount;
}

void AACFrameLenDecoder::deinitAACDec()
{
    if (mAACDec)
    {
        if (mAACDec->pInterBuf)
        {
            free(mAACDec->pInterBuf);
            mAACDec->pInterBuf = NULL;
        }

        if (mAACDec->pPcmBuf)
        {
            free(mAACDec->pPcmBuf);
            mAACDec->pPcmBuf = NULL;
        }

        if (mAACDec->pTmpBuf)
        {
            free(mAACDec->pTmpBuf);
            mAACDec->pTmpBuf = NULL;
        }

        if (mAACDec->pBsBuf)
        {
            free(mAACDec->pBsBuf);
            mAACDec->pBsBuf = NULL;
        }

        free(mAACDec);
        mAACDec = NULL;
    }

    mInited = false;
}

#endif //<----- add for mtk aac decoder
#endif //<-----add for pv aac decoder
#endif//<------add for fraunhofer aac decoder

class MtkAACSource : public MediaSource
#ifdef ENABLE_USE_TABLE_OF_CONTENT
    , public TableOfContentThread
#endif
{
    //  for AAC seek table
public:
    MtkAACSource(
        const sp<MetaData> &meta, const sp<DataSource> &source,
        off_t first_frame_pos, uint32_t fixed_header,
        bool isADTS);

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();

    virtual sp<MetaData> getFormat();

    virtual status_t read(
        MediaBuffer **buffer, const ReadOptions *options = NULL);

    //  for AAC seek table--->
    virtual status_t getNextFramePos(off_t *pCurPos, off_t *pNextPos, int64_t *frameTsUs);
    virtual status_t sendDurationUpdateEvent(int64_t duration);
    // <--- for AAC seek table

protected:
    virtual ~MtkAACSource();

private:
    sp<MetaData> mMeta;
    sp<DataSource> mDataSource;
    off_t mFirstFramePos;
    uint32_t mFixedHeader;
    off_t mCurrentPos;
    int64_t mCurrentTimeUs;
    bool mStarted;

    MediaBufferGroup *mGroup;
    bool mIsADTS;
    int64_t mFrameDurationUs;
    //  for AAC seek table --->
    AACFrameLenDecoder *mAACFrameLenDecoderPtr;
    AwesomePlayer *mObserver;
    // <---  for AAC seek table

    MtkAACSource(const MtkAACSource &);
    MtkAACSource &operator=(const MtkAACSource &);
};

MtkAACExtractor::MtkAACExtractor(
    const sp<DataSource> &source, const sp<AMessage> &meta)
    : mInitCheck(NO_INIT),
      mDataSource(source),
      mFirstFramePos(-1),
      mFixedHeader(0)
{
    off_t pos = 0;
    uint32_t header;
    uint32_t nSampleFreqIndex;
    uint32_t nBitRate = 0;
    uint32_t nProfile;
    uint32_t nChannelNum;
    uint32_t nSampleRate;
    int32_t  bIsADIF;
    uint64_t nDurationUs = 0;

    bool success;
    int64_t meta_offset;
    uint32_t meta_header;

    if (meta != NULL
            && meta->findInt64("offset", &meta_offset)
            && meta->findInt32("header", (int32_t *)&meta_header)
            && meta->findInt32("samplefreqidx", (int32_t *)&nSampleFreqIndex)
          	&& meta->findInt32("bitrate", (int32_t *)&nBitRate)
            && meta->findInt32("profile", (int32_t *)&nProfile)
            && meta->findInt32("channelnum", (int32_t *)&nChannelNum)
            && meta->findInt32("isadif", (int32_t *)&bIsADIF))
    {
        // The sniffer has already done all the hard work for us, simply
        // accept its judgement.
        pos = (off_t)meta_offset;
        header = meta_header;

        success = true;
    }
    else
    {
        success = getAACInfo(source, &pos, &header, &nSampleFreqIndex, &nBitRate, &nProfile, &nChannelNum, &bIsADIF);
    }

    if (!success)
    {
        // mInitCheck will remain NO_INIT
        return;
    }

	if(!bIsADIF && calculateAdtsAverageBitRate(source,pos,header,nSampleFreqIndex,&nBitRate))
	{
		AAC_EXTR_WARN("no find bitrate!");
		// mInitCheck will remain NO_INIT
		return;
	}
    mFirstFramePos = pos;
    mFixedHeader = header;
    nSampleRate = AACSampleFreqTable[nSampleFreqIndex];
    mMeta = MakeAACCodecSpecificData(nProfile, nSampleFreqIndex, nChannelNum);

    AAC_EXTR_DBG("AACExtractor::AACExtractor--nBitRate=%d", nBitRate);
    AAC_EXTR_DBG("AACExtractor::AACExtractor--samplerate = %d", nSampleRate);
    AAC_EXTR_DBG("AACExtractor::AACExtractor--nProfile = %d", nProfile);
    AAC_EXTR_DBG("AACExtractor::AACExtractor--num channels = %d", nChannelNum);
	AAC_EXTR_DBG("AACExtractor::AACExtractor--nSampleFreqIndex = %d", nSampleFreqIndex);

#if 0
    uint8_t config_info[2];

    config_info[0] = config_info[1] = 0;
    config_info[0] |= (nProfile + 1) << 3; // put it into the highest 5 bits
    config_info[0] |= ((nSampleFreqIndex & 0x0F) >> 1);    // put 3 bits
    config_info[1] |= ((nSampleFreqIndex & 0x01) << 7); // put 1 bit
    config_info[1] |= nChannelNum << 3;

    AAC_EXTR_VEB("AACExtractor::AACExtractor--nBitRate=%d", nBitRate);
    AAC_EXTR_VEB("AACExtractor::AACExtractor--samplerate = %d", nSampleRate);
    AAC_EXTR_VEB("AACExtractor::AACExtractor--nProfile = %d", nProfile);
    AAC_EXTR_VEB("AACExtractor::AACExtractor--num channels = %d", nChannelNum);
    AAC_EXTR_VEB("AACExtractor::AACExtractor--config_info[0]=%x", config_info[0]);
    AAC_EXTR_VEB("AACExtractor::AACExtractor--config_info[1]=%x", config_info[1]);

    mMeta = new MetaData;


    mMeta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
    mMeta->setInt32(kKeySampleRate, nSampleRate);
#ifndef ANDROID_DEFAULT_CODE
    mMeta->setData(kKeyCodecConfigInfo, 0, config_info, 2);
#endif  //#ifndef ANDROID_DEFAULT_CODE  
    mMeta->setInt32(kKeyChannelCount, nChannelNum);
#endif

    mMeta->setInt32(kKeyBitRate, nBitRate); // need to add configinfo fix me
    mMeta->setInt32(kKeyIsAACADIF, bIsADIF);


    off64_t fileSize;

    if (mDataSource->getSize(&fileSize) == OK)
    {
        nDurationUs = 8000000LL * ((float)(fileSize - mFirstFramePos) / nBitRate);
        mMeta->setInt64(kKeyDuration, nDurationUs);
        AAC_EXTR_DBG("AACExtractor::AACExtractor--fileSize=%d", fileSize);
        AAC_EXTR_DBG("AACExtractor::AACExtractor--nDurationUs=%d", (int32_t)nDurationUs);
    }

    if (0 == bIsADIF)
    {
        mIsADTS = true;
    }
    else
    {
        mIsADTS = false;
    }

    mInitCheck = OK;

    // get iTunes-style gapless info if present
    ID3 id3(mDataSource);

    if (id3.isValid())
    {
        ID3::Iterator *com = new ID3::Iterator(id3, "COM");

        if (com->done())
        {
            delete com;
            com = new ID3::Iterator(id3, "COMM");
        }

        while (!com->done())
        {
            String8 commentdesc;
            String8 commentvalue;
            com->getString(&commentdesc, &commentvalue);
            const char *desc = commentdesc.string();
            const char *value = commentvalue.string();

            // first 3 characters are the language, which we don't care about
            if (strlen(desc) > 3 && strcmp(desc + 3, "iTunSMPB") == 0)
            {

                int32_t delay, padding;

                if (sscanf(value, " %*x %x %x %*x", &delay, &padding) == 2)
                {
                    mMeta->setInt32(kKeyEncoderDelay, delay);
                    mMeta->setInt32(kKeyEncoderPadding, padding);
                }

                break;
            }

            com->next();
        }

        delete com;
        com = NULL;
    }
}

status_t MtkAACExtractor::calculateAdtsAverageBitRate(const sp<DataSource> &source,off_t filePos,uint32_t header,
	                                                  uint32_t sampleFreqIndex,uint32_t *pBitRate)
{
	//estimate duration
	AAC_EXTR_DBG("calculateAverageBitRate filePos %d header 0x%08x sampleFreqIndex %d",filePos,header,sampleFreqIndex);

	uint8_t tmp[7];
	uint32_t frameNum = 0;
	uint32_t readedDataSize = 0;
	uint32_t frameSize;

	uint32_t frameNumPre = 0;
	uint32_t readedDataSizePre = 0;

	while (frameNum < 100)
	{
		if (source->readAt(filePos, tmp, 7) != 7)
		{
			break;
		}
		uint32_t headerTmp = U32_AT(tmp);
		if ((headerTmp & kMask) == (header & kMask))
		{
			frameSize = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);
			if (frameSize > 7)
			{
				readedDataSize += frameSize;
				frameNum++;
				filePos += frameSize;	// fix me for the end frame is a bad frame
				if (30 == frameNum)
				{
					frameNumPre = frameNum;
					readedDataSizePre = readedDataSize;
				}
			}
			else
			{
				filePos ++;
			}
		}
		else
		{
			filePos++;
		}
	}

	if (frameNum == 100 && readedDataSize > readedDataSizePre)
	{
		frameNum -= frameNumPre;
		readedDataSize -= readedDataSizePre;
	}

	if (frameNum > 0 && readedDataSize > 0)
	{
		*pBitRate = (float)readedDataSize * 8 / frameNum  / 1024 * AACSampleFreqTable[sampleFreqIndex];
	}
	else
	{
		return BAD_VALUE;
	}

	return OK;
}
size_t MtkAACExtractor::countTracks()
{
    return mInitCheck != OK ? 0 : 1;
}

sp<MediaSource> MtkAACExtractor::getTrack(size_t index)
{
    if (mInitCheck != OK || index != 0)
    {
        return NULL;
    }

    return new MtkAACSource(
               mMeta, mDataSource, mFirstFramePos, mFixedHeader,
               mIsADTS);
}

sp<MetaData> MtkAACExtractor::getTrackMetaData(size_t index, uint32_t flags)
{
    if (mInitCheck != OK || index != 0)
    {
        return NULL;
    }

    return mMeta;
}

uint32_t MtkAACExtractor::flags() const
{
    if (mIsADTS)
    {
        return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
    }
    else
    {

#ifdef ENABLE_USE_TABLE_OF_CONTENT
#ifdef ADIF_MANDATORY_SEEK
        return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
#endif
#endif
        return CAN_PAUSE;
    }
}

sp<MetaData> MtkAACExtractor::MakeAACCodecSpecificData(
    unsigned profile, unsigned sampling_freq_index,
    unsigned channel_configuration)
{
    sp<MetaData> meta = new MetaData;
    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);

    meta->setInt32(kKeySampleRate, AACSampleFreqTable[sampling_freq_index]);
    meta->setInt32(kKeyChannelCount, channel_configuration);

    static const uint8_t kStaticESDS[] =
    {
        0x03, 22,
        0x00, 0x00,     // ES_ID
        0x00,           // streamDependenceFlag, URL_Flag, OCRstreamFlag

        0x04, 17,
        0x40,                       // Audio ISO/IEC 14496-3
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        0x05, 2,
        // AudioSpecificInfo follows

        // oooo offf fccc c000
        // o - audioObjectType
        // f - samplingFreqIndex
        // c - channelConfig
    };
    sp<ABuffer> csd = new ABuffer(sizeof(kStaticESDS) + 2);
    memcpy(csd->data(), kStaticESDS, sizeof(kStaticESDS));

    csd->data()[sizeof(kStaticESDS)] =
        ((profile + 1) << 3) | (sampling_freq_index >> 1);

    csd->data()[sizeof(kStaticESDS) + 1] =
        ((sampling_freq_index << 7) & 0x80) | (channel_configuration << 3);

    AAC_EXTR_DBG("AACExtractor::MakeAACCodecSpecificData--csd->data()[%d]=%x", sizeof(kStaticESDS), csd->data()[sizeof(kStaticESDS)]);
    AAC_EXTR_DBG("AACExtractor::MakeAACCodecSpecificData--csd->data()[%d]=%x", sizeof(kStaticESDS) + 1, csd->data()[sizeof(kStaticESDS) + 1]);

    meta->setData(kKeyESDS, 0, csd->data(), csd->size());

    return meta;
}



////////////////////////////////////////////////////////////////////////////////

MtkAACSource::MtkAACSource(
    const sp<MetaData> &meta, const sp<DataSource> &source,
    off_t first_frame_pos, uint32_t fixed_header,
    bool isADTS)
    : mMeta(meta),
      mDataSource(source),
      mFirstFramePos(first_frame_pos),
      mFixedHeader(fixed_header),
      mCurrentPos(0),
      mCurrentTimeUs(0),
      mStarted(false),
      mGroup(NULL),
      mIsADTS(isADTS),
      mAACFrameLenDecoderPtr(NULL),  //  for aac seek table
      mObserver(NULL),
      mFrameDurationUs(0)
{

    void *pAwe = NULL;
    meta->findPointer(kKeyDataSourceObserver, &pAwe);
    mObserver = (AwesomePlayer *) pAwe;


}

MtkAACSource::~MtkAACSource()
{
    if (mStarted)
    {
        stop();
    }
}

status_t MtkAACSource::start(MetaData *)
{
    CHECK(!mStarted);


#ifdef ENABLE_USE_TABLE_OF_CONTENT

    if (!mIsADTS)
    {
        mAACFrameLenDecoderPtr = new AACFrameLenDecoder(mMeta, mDataSource, mFirstFramePos);

        if (NULL == mAACFrameLenDecoderPtr)
        {
            return ERROR_UNSUPPORTED;
        }

        if (!mAACFrameLenDecoderPtr->initAACDec())
        {
            //free(mAACFrameLenDecoderPtr);
            delete mAACFrameLenDecoderPtr;
            mAACFrameLenDecoderPtr = NULL;
            return ERROR_UNSUPPORTED;
        }

        AAC_EXTR_DBG("Mtk AACSource::start--ADIF initAACDec OK");
    }

    int32_t nSampleRate;

    if (mMeta->findInt32(kKeySampleRate, &nSampleRate))
    {
        mFrameDurationUs = 1024.0 / nSampleRate * 1000000.0;
        AAC_EXTR_DBG("Per Frame Duration =%d,First frame pos =%ld", mFrameDurationUs, mFirstFramePos);
        startTOCThread(mFirstFramePos);
    }

#endif


    mGroup = new MediaBufferGroup;

    const size_t kMaxFrameSize = MEDIA_BUFFER_LEN;
    mGroup->add_buffer(new MediaBuffer(kMaxFrameSize));

    mCurrentPos = mFirstFramePos;
    mCurrentTimeUs = 0;

    mStarted = true;

    return OK;
}

status_t MtkAACSource::stop()
{
    CHECK(mStarted);

    //  for AAC seek table --->
#ifdef ENABLE_USE_TABLE_OF_CONTENT
    //for seek table
    stopTOCThread();

    if (NULL != mAACFrameLenDecoderPtr)
    {
        free(mAACFrameLenDecoderPtr);
        mAACFrameLenDecoderPtr = NULL;
    }

#endif
    // <---  for AAC seek table

    delete mGroup;
    mGroup = NULL;

    mStarted = false;

    return OK;
}

sp<MetaData> MtkAACSource::getFormat()
{
    return mMeta;
}

//  for AAC seek table --->
status_t MtkAACSource::getNextFramePos(off_t *pCurPos, off_t *pNextPos, int64_t *frameTsUs)
{
    off_t curPos = *pCurPos;

    if (mIsADTS)
    {
        uint8_t tmp[7];
        size_t frame_size;

        for (;;)
        {
            ssize_t n = mDataSource->readAt(curPos, tmp, 7);

            if (n < 7)
            {
                return ERROR_END_OF_STREAM;
            }

            uint32_t header = U32_AT(tmp);

            if ((mFixedHeader & kMask) == (header & kMask))
            {
                frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

                if (frame_size > 7)
                {
                    *pCurPos = curPos;
                    *pNextPos = curPos + frame_size;
                    *frameTsUs = mFrameDurationUs;
                    return OK;
                }
            }

            curPos++;
        }

        return ERROR_END_OF_STREAM;
    }
    else
    {
        *frameTsUs = mFrameDurationUs;
        return mAACFrameLenDecoderPtr->getNextFramePos(pCurPos, pNextPos);
    }
}

status_t MtkAACSource::sendDurationUpdateEvent(int64_t duration)
{
    if (mObserver)
    {
        AAC_EXTR_DBG("AAC Seek Table duration %lld", duration);
        mObserver->postDurationUpdateEvent(duration);
    }

    return OK;
}



status_t MtkAACSource::read(
    MediaBuffer **out, const ReadOptions *options)
{
    *out = NULL;

    if (mIsADTS)
    {
        int64_t seekTimeUs;
        ReadOptions::SeekMode mode;

        if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
        {

#ifdef ENABLE_USE_TABLE_OF_CONTENT

#ifdef ADTS_MANDATORY_SEEK
            status_t status = getFramePos(seekTimeUs, &mCurrentTimeUs, &mCurrentPos, true);
#else
            status_t status = getFramePos(seekTimeUs, &mCurrentTimeUs, &mCurrentPos, false);
#endif

            if (ERROR_END_OF_STREAM == status)
            {
                return ERROR_END_OF_STREAM;
            }
            else if (OK != status)
            {
                int32_t bitrate;

                if (!mMeta->findInt32(kKeyBitRate, &bitrate))
                {
                    // bitrate is in bits/sec.
                    AAC_EXTR_INFO("no bitrate");
                    return ERROR_UNSUPPORTED;
                }

                mCurrentTimeUs = seekTimeUs;
                mCurrentPos = mFirstFramePos + seekTimeUs * bitrate / 8000000; // estimate duration
            }

#else
            int32_t bitrate;

            if (!mMeta->findInt32(kKeyBitRate, &bitrate))
            {
                // bitrate is in bits/sec.
                AAC_EXTR_INFO("no bitrate");
                return ERROR_UNSUPPORTED;
            }

            mCurrentTimeUs = seekTimeUs;
            mCurrentPos = mFirstFramePos + seekTimeUs * bitrate / 8000000; // estimate duration
#endif //  for aac seek table

            AAC_EXTR_DBG("MtkAACSource::read--seek seekTimeUs = %dus", (int32_t)(seekTimeUs));
            AAC_EXTR_DBG("MtkAACSource::read--seek mCurrentTimeUs = %dus", (int32_t)(mCurrentTimeUs));
            AAC_EXTR_DBG("MtkAACSource::read--seek mCurrentPos = %d", mCurrentPos);
        }

        MediaBuffer *buffer;
        status_t err = mGroup->acquire_buffer(&buffer);

        if (err != OK)
        {
            return err;
        }

        size_t frame_size;

        for (;;)
        {
            ssize_t n = mDataSource->readAt(mCurrentPos, buffer->data(), 7);

            if (n < 7)
            {
                buffer->release();
                buffer = NULL;
                return ERROR_END_OF_STREAM;
            }

            uint32_t header = U32_AT((const uint8_t *)buffer->data());
            const uint8_t *tmp = (const uint8_t *)buffer->data();

            if ((mFixedHeader & kMask) == (header & kMask))
            {
                frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

                if (frame_size > 9)
                {
                    if (tmp[1] & 0x01) //no crc
                    {
                        frame_size -= 7;
                        mCurrentPos += 7;
                    }
                    else
                    {
                        frame_size -= (7 + 2);
                        mCurrentPos += (7 + 2);
                    }

                    break;
                }
            }

            mCurrentPos++;
        }

        CHECK(frame_size <= buffer->size());

        ssize_t n = mDataSource->readAt(mCurrentPos, buffer->data(), frame_size);

        if (n < (ssize_t)frame_size)
        {
            buffer->release();
            buffer = NULL;

            return ERROR_END_OF_STREAM;
        }

        buffer->set_range(0, frame_size);
        buffer->meta_data()->setInt64(kKeyTime, mCurrentTimeUs);

        mCurrentPos += frame_size;
        int32_t sample_rate;

        if (!mMeta->findInt32(kKeySampleRate, &sample_rate))
        {
            // bitrate is in bits/sec.
            AAC_EXTR_WARN("no sample_rate");
            return ERROR_UNSUPPORTED;
        }

        mCurrentTimeUs += 1024 * 1000000 / sample_rate;

        *out = buffer;
    }
    else
    {
        int64_t seekTimeUs = -1;
        ReadOptions::SeekMode mode;

        if (options != NULL && options->getSeekTo(&seekTimeUs, &mode))
        {

#ifdef ENABLE_USE_TABLE_OF_CONTENT

#ifdef ADIF_MANDATORY_SEEK
            status_t status = getFramePos(seekTimeUs, &mCurrentTimeUs, &mCurrentPos, true);

            if (OK != status)
            {
                return status;
            }

            AAC_EXTR_DBG("MtkAACSource::read--seek seekTimeUs = %dus", (int32_t)(seekTimeUs));
            AAC_EXTR_DBG("MtkAACSource::read--seek mCurrentTimeUs = %dus", (int32_t)(mCurrentTimeUs));
            AAC_EXTR_DBG("MtkAACSource::read--seek mCurrentPos = %d", mCurrentPos);
#else

            off_t currentPos;
            int64_t currentTimeUs;

            status_t status = getFramePos(seekTimeUs, &currentTimeUs, &currentPos, false);

            if (ERROR_END_OF_STREAM == status)
            {
                return ERROR_END_OF_STREAM;
            }
            else if (OK != status)
            {
                return ERROR_END_OF_STREAM;
            }
            else
            {
                mCurrentPos = currentPos;
                mCurrentTimeUs = currentTimeUs;
            }

#endif
#else
            //seekTimeUs = mCurrentTimeUs;
            seekTimeUs = 0;
            mCurrentPos = 0;
#endif

        }
        else
        {
            seekTimeUs = -1;
        }

        MediaBuffer *buffer;
        status_t err = mGroup->acquire_buffer(&buffer);

        if (err != OK)
        {
            return err;
        }

        uint32_t nReadSize = ADIF_DEC_BUFFER_SIZE;//1536;
        CHECK(nReadSize <= buffer->size());

        ssize_t n = mDataSource->readAt(mCurrentPos, ((uint8_t *)buffer->data()), nReadSize);

        if (n <= 0)
        {
            buffer->release();
            buffer = NULL;

            return ERROR_END_OF_STREAM;
        }

        nReadSize = n;  // for the end of the file

        buffer->set_range(0, nReadSize);

        if (seekTimeUs >= 0)
        {
            buffer->meta_data()->setInt64(kKeyTime, mCurrentTimeUs);
        }

        mCurrentPos += nReadSize;

        *out = buffer;
    }

    return OK;
}

sp<MetaData> MtkAACExtractor::getMetaData()
{
    sp<MetaData> meta = new MetaData;

    if (mInitCheck != OK)
    {
        return meta;
    }

    meta->setCString(kKeyMIMEType, "audio/aac");

    ID3 id3(mDataSource);

    if (!id3.isValid())
    {
        return meta;
    }

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

bool SniffMtkAAC(
    const sp<DataSource> &source, String8 *mimeType,
    float *confidence, sp<AMessage> *meta)
{
    off_t pos = 0;
    uint32_t header;
    uint32_t nSampleFreqIndex;
    uint32_t nBitRate=0;
    uint32_t nProfile;
    uint32_t nChannelNum;
    int32_t bIsADIF;

    if (!getAACInfo(source, &pos, &header, &nSampleFreqIndex, &nBitRate, &nProfile, &nChannelNum, &bIsADIF))
    {
        return false;
    }

    *meta = new AMessage;
    (*meta)->setInt64("offset", pos);
    (*meta)->setInt32("header", header);
    (*meta)->setInt32("samplefreqidx", nSampleFreqIndex);
	(*meta)->setInt32("bitrate", nBitRate);
    (*meta)->setInt32("profile", nProfile);
    (*meta)->setInt32("channelnum", nChannelNum);
    (*meta)->setInt32("isadif", bIsADIF);

    *mimeType = MEDIA_MIMETYPE_AUDIO_AAC; // not define new type
    *confidence = 0.3f;

    return true;
}

bool FastSniffAAC(
    const sp<DataSource> &source, String8 *mimeType,
    float *confidence, sp<AMessage> *meta)
{
    off_t inout_pos = 0;
    int32_t bIsADIF = false;
    bool valid = false;
    off_t test_header;
    off_t pos = 0;

    if (inout_pos == 0)
    {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.
        for (;;)
        {
            uint8_t id3header[10];

            if (source->readAt(inout_pos, id3header, sizeof(id3header))
                    < (ssize_t)sizeof(id3header))
            {
                return false;
            }

            if (memcmp("ID3", id3header, 3))
            {
                break;
            }

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;
            inout_pos += len;

        }
    }

    pos = inout_pos;

    uint8_t tmpBuf[4];

    if (source->readAt(pos, tmpBuf, 4) != 4)
    {
        return false;
    }

    if (tmpBuf[0] == 0x41 &&      // 'A'
            tmpBuf[1] == 0x44 &&  // 'D'
            tmpBuf[2] == 0x49 &&  // 'I'
            tmpBuf[3] == 0x46)    // 'F'
    {
        bIsADIF = true;
        valid = true;

    }
    else if (isAACADTSHeader(tmpBuf)) ///adts
    {
        uint32_t header = U32_AT(tmpBuf);
        uint8_t tmp[7];

        if (source->readAt(pos, tmp, 7) != 7)
        {
            return false;
        }

        size_t frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

        if (frame_size <= 7)
        {
            return false;
        }

        off_t test_pos = pos + frame_size;
        valid = true;

        for (int j = 0; j < 3; ++j)
        {

            if (source->readAt(test_pos, tmp, 7) < 7)
            {
                valid = false;
                break;
            }

            test_header = U32_AT(tmp);

            if (header != 0 && (test_header & kMask) != (header & kMask))   //  compare frame fixed header
            {
                valid = false;
                break;
            }

            size_t test_frame_size = ((tmp[3] & 0x03) << 11) | (tmp[4] << 3) | ((tmp[5] & 0xe0) >> 5);

            if (test_frame_size <= 7)    //  compare frame fixed header
            {
                valid = false;
                break;
            }

            test_pos += test_frame_size;
        }

    }

    if (valid == false)
    {
        return false;
    }

    *meta = new AMessage;
    (*meta)->setInt64("offset", pos);
    (*meta)->setInt32("header", test_header);
    (*meta)->setInt32("isadif", bIsADIF);

    *mimeType = MEDIA_MIMETYPE_AUDIO_AAC; // not define new type
    *confidence = 0.3f;

    return true;
}


}  // namespace android
