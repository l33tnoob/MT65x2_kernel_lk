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

/*  libasf - An Advanced Systems Format media file parser   
*  Copyright (C) 2006-2010 Juho Vähä-Herttua   
*   
*  This library is free software; you can redistribute it and/or   
*  modify it under the terms of the GNU Lesser General Public   
*  License as published by the Free Software Foundation; either   
*  version 2.1 of the License, or (at your option) any later version.   
*   *  This library is distributed in the hope that it will be useful,   
*  but WITHOUT ANY WARRANTY; without even the implied warranty of 
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   
*  Lesser General Public License for more details.   
*   
*  You should have received a copy of the GNU Lesser General Public   
*  License along with this library; if not, write to the Free Software   
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA   */  

#ifndef ASF_H
#define ASF_H

#include <stdio.h>
#include <string.h>
//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------
/*
#define LOG_TAG "ASFParser"
#define LOGV printf
#define LOGD printf
#define LOGE printf
*/

/* used int types for different platforms */
//typedef char                  int8_t;
/*
typedef unsigned char        uint8_t;
typedef short                   int16_t;
typedef unsigned short       uint16_t;
typedef long                     int32_t;
typedef unsigned long        uint32_t;

typedef long long               int64_t;
typedef unsigned long long      uint64_t;
*/

#define ASF_STREAM_ID_START 1 /* ASF file format stream ID start from 1 ~ 127 */


#define MAKEFOURCC(b0, b1, b2, b3) \
        ((uint32_t)(uint8_t)(b0) | ((uint32_t)(uint8_t)(b1) << 8) |   \
        ((uint32_t)(uint8_t)(b2) << 16) | ((uint32_t)(uint8_t)(b3) << 24 ))


#define  WAVE_FORMAT_WMA1               (0x0160) /*Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio*/
#define  WAVE_FORMAT_WMA2               (0x0161) /*Windows Media Audio Professional V9 */
#define  WAVE_FORMAT_WMA3               (0x0162) /*Windows Media Audio Lossless V9*/

#define  WAVE_FROMAT_MSPCM              (0x0001)
#define  WAVE_FROMAT_MSADPCM            (0x0002)
#define  WAVE_FORMAT_MP3                (0x0055)
#define  WAVE_FORMAT_MP2                (0x0050)
#define  WAVE_FORMAT_AAC                (0x00ff)
#define  WAVE_FORMAT_AAC_AC             (0x4143)
#define  WAVE_FORMAT_AAC_pm             (0x706d)

#define FOURCC_WMV3		MAKEFOURCC('W','M','V','3')	// Simple and Main Profiles of VC-1 
#define FOURCC_WMV2		MAKEFOURCC('W','M','V','2')
#define FOURCC_WMVA		MAKEFOURCC('W','M','V','A')	// WMV Advanced Profile
#define FOURCC_WVC1		MAKEFOURCC('W','V','C','1')		// Windows Media Video 9 Advanced Profile
//MPEG4
#define FOURCC_MP4S		MAKEFOURCC('M','P','4','S')
#define FOURCC_XVID		MAKEFOURCC('X','V','I','D')
#define FOURCC_DIVX		MAKEFOURCC('D','I','V','X')
#define FOURCC_DX50		MAKEFOURCC('D','X','5','0')
#define FOURCC_MP4V		MAKEFOURCC('M','P','4','V')
#define FOURCC_M4S2		MAKEFOURCC('M','4','S','2')
//H264
#define FOURCC_AVC1		MAKEFOURCC('A','V','C','1')
#define FOURCC_DAVC     MAKEFOURCC('D','A','V','C')
#define FOURCC_X264		MAKEFOURCC('X','2','6','4')
#define FOURCC_H264     MAKEFOURCC('H','2','6','4')
#define FOURCC_VSSH     MAKEFOURCC('V','S','S','H')
//MJPEG
#define FOURCC_MJPG     MAKEFOURCC('M','J','P','G')

typedef enum
{
    ASF_ERROR_UNKNOWN = -1,
    ASF_SUCCESS = 0,
    ASF_DEFINE = 1,
    ASF_END_OF_FILE = 2,
    ASF_CRC_ERR = 3,
    ASF_FILE_READ_ERR = 4,
    ASF_FILE_HDR_READ_ERR = 5,
    ASF_FILE_HDR_DECODE_ERR = 6,
    ASF_FILE_XING_HDR_ERR = 7,
    ASF_FILE_VBRI_HDR_ERR = 8,
    ASF_ERR_NO_MEMORY = 9,
    ASF_NO_SYNC_FOUND = 10,
    ASF_FILE_OPEN_ERR = 11,
    /* PD related Error values*/
    ASF_ERROR_UNKNOWN_OBJECT = 12,
    ASF_FILE_OPEN_FAILED = 13,
    ASF_INSUFFICIENT_DATA = 14,
    ASF_METADATA_NOTPARSED = 15,
    /* Duration related Info value*/
    ASF_DURATION_PRESENT = 16
} ASFErrorType;


struct asf_guid_s
{
    uint32_t v1;
    uint32_t v2;
    uint16_t v3;
    uint8_t  v4[8];
};
typedef struct asf_guid_s asf_guid_t;

struct asf_iostream_s
{
    /* read function, returns -1 on error, 0 on EOF and read bytes
     * otherwise */
    int32_t (*read)(void* source, void *buffer, int32_t size);

    /* write function, returns -1 on error, 0 on EOF and written
     * bytes otherwise */
    int32_t (*write)(void* source, void *buffer, int32_t size);

    /* seek function, seeks to offset from beginning of the file,
     * returns -1 on error, 0 on EOF */
    int64_t (*seek)(void* source, int64_t offset);

    /* opaque data pointer passed to each of the stream handling
     * callbacks */
    void *source;
};
typedef struct asf_iostream_s asf_iostream_t;

struct asf_metadata_entry_s
{
    char *key;      /* key of extended metadata entry */
    char *value;    /* value of extended metadata entry */
    uint32_t value_type;
    uint32_t size;
};
typedef struct asf_metadata_entry_s asf_metadata_entry_t;

/* all metadata entries are presented in UTF-8 character encoding */
struct asf_metadata_s
{

    uint16_t content_count;        /* number of extended entries */
    asf_metadata_entry_t *content; /* array of extended entries */

    uint16_t extended_count;        /* number of extended entries */
    asf_metadata_entry_t *extended; /* array of extended entries */
    uint16_t metadata_count;        /* number of metadata entries */
    asf_metadata_entry_t *metadata; /* array of metadata entries */
    uint16_t metadatalib_count;        /* number of metadata entries */
    asf_metadata_entry_t *metadatalib; /* array of metadata entries */
};
typedef struct asf_metadata_s asf_metadata_t;

#define ASF_REPLICATEDDATA_SIZE_BYTES 4

struct asf_payload_s
{
    uint8_t stream_number;  /* the stream number this payload belongs to */
    uint8_t key_frame;      /* a flag indicating if this payload contains a key frame  */

    uint32_t media_object_number;   /* number of media object this payload is part of */
    uint32_t media_object_offset;   /* byte offset from beginning of media object */

    uint32_t replicated_length;     /* length of some replicated data of a media object... */
    uint8_t *replicated_data;       /* the replicated data mentioned */

    uint32_t datalen;       /* length of the actual payload data */
    uint8_t *data;          /* the actual payload data to decode */

    uint32_t pts;           /* presentation time of this payload */
};
typedef struct asf_payload_s asf_payload_t;

struct asf_packet_s
{
    uint8_t ec_length;      /* error correction data length */
    uint8_t *ec_data;       /* error correction data array */

    uint32_t length;                /* length of this packet, usually constant per stream */
    uint32_t padding_length;        /* length of the padding after the data in this packet */
    uint32_t send_time;             /* send time of this packet in milliseconds */
    uint16_t duration;              /* duration of this packet in milliseconds */

    uint16_t payload_count;         /* number of payloads contained in this packet */
    asf_payload_t *payloads;        /* an array of payloads in this packet */
    uint16_t payloads_size;         /* for internal library use, not to be modified by applications! */

    uint32_t payload_data_len;      /* length of the raw payload data of this packet */
    uint8_t *payload_data;          /* the raw payload data of this packet, usually not useful */

    uint8_t *data;          /* for internal library use, not to be modified by applications! */
    uint32_t data_size;     /* for internal library use, not to be modified by applications! */
};
typedef struct asf_packet_s asf_packet_t;

#define ASF_WAVEFORMATEX_SIZE 18
/* waveformatex fields specified in Microsoft documentation:
   http://msdn2.microsoft.com/en-us/library/ms713497.aspx */
struct asf_waveformatex_s
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
    uint8_t *data;
};
typedef struct asf_waveformatex_s asf_waveformatex_t;

#define ASF_BITMAPINFOHEADER_SIZE 40

/* bitmapinfoheader fields specified in Microsoft documentation:
   http://msdn2.microsoft.com/en-us/library/ms532290.aspx */
struct asf_bitmapinfoheader_s
{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
    uint8_t *data;
};
typedef struct asf_bitmapinfoheader_s asf_bitmapinfoheader_t;

enum asf_stream_type_e
{
    ASF_STREAM_TYPE_NONE     = 0x00,
    ASF_STREAM_TYPE_AUDIO    = 0x01,
    ASF_STREAM_TYPE_VIDEO    = 0x02,
    ASF_STREAM_TYPE_COMMAND  = 0x03,
    ASF_STREAM_TYPE_UNKNOWN  = 0xff
};
typedef enum asf_stream_type_e asf_stream_type_t;

#define ASF_STREAM_FLAG_NONE       0x0000
#define ASF_STREAM_FLAG_AVAILABLE  0x0001
#define ASF_STREAM_FLAG_HIDDEN     0x0002
#define ASF_STREAM_FLAG_EXTENDED   0x0004

struct asf_stream_extended_s
{
    uint64_t start_time;
    uint64_t end_time;
    uint32_t data_bitrate;
    uint32_t buffer_size;
    uint32_t initial_buf_fullness;
    uint32_t data_bitrate2;
    uint32_t buffer_size2;
    uint32_t initial_buf_fullness2;
    uint32_t max_obj_size;
    uint32_t flags;
    uint16_t stream_num;
    uint16_t lang_idx;
    uint64_t avg_time_per_frame;
    uint16_t stream_name_count;
    uint16_t num_payload_ext;
};
typedef struct asf_stream_extended_s asf_stream_extended_t;

struct asf_stream_s
{
    asf_stream_type_t type; /* type of this current stream */
    uint16_t flags;         /* possible flags related to this stream */

    /* pointer to type specific data (ie. waveformatex or bitmapinfoheader)
     * only available if ASF_STREAM_FLAG_AVAILABLE flag is set, otherwise NULL */
    void *properties;

    /* pointer to extended properties of the stream if they specified
     * only available if ASF_STREAM_FLAG_EXTENDED flag is set, otherwise NULL */
    asf_stream_extended_t *extended;

    /*current read position of packet*/
    uint64_t current_packet;
};
typedef struct asf_stream_s asf_stream_t;

typedef struct asf_file_s asf_file_t;

enum asf_error_e
{
    ASF_PARSE_SUCCESS        = 0,
    ASF_ERROR_INTERNAL       = -1,  /* incorrect input to API calls */
    ASF_ERROR_OUTOFMEM       = -2,  /* some malloc inside program failed */
    ASF_ERROR_EOF            = -3,  /* unexpected end of file */
    ASF_ERROR_IO             = -4,  /* error reading or writing to file */
    ASF_ERROR_INVALID_LENGTH = -5,  /* length value conflict in input data */
    ASF_ERROR_INVALID_VALUE  = -6,  /* other value conflict in input data */
    ASF_ERROR_INVALID_OBJECT = -7,  /* ASF object missing or in wrong place */
    ASF_ERROR_OBJECT_SIZE    = -8,  /* invalid ASF object size (too small) */
    ASF_ERROR_SEEKABLE       = -9,  /* file not seekable */
    ASF_ERROR_SEEK           = -10  /* file is seekable but seeking failed */
};

struct asf_object_s
{
    asf_guid_t   guid;
    uint64_t     size;
    uint8_t      *data;
};
typedef struct asf_object_s asf_object_t;

/*GUID types*/
enum guid_type_e
{
    GUID_UNKNOWN,

    GUID_HEADER,
    GUID_DATA,
    GUID_SIMPLE_INDEX,
    GUID_INDEX,

    GUID_FILE_PROPERTIES,
    GUID_STREAM_PROPERTIES,
    GUID_CONTENT_DESCRIPTION,
    GUID_HEADER_EXTENSION,
    GUID_MARKER,
    GUID_CODEC_LIST,
    //add by qian
    GUID_CONTENT_BRANDING,
    GUID_CONTENT_ENCRYPTION,
    GUID_STREAM_BITRATE_PROPERTIES,
    GUID_PADDING,
    GUID_EXTENDED_CONTENT_DESCRIPTION,

    GUID_METADATA,
    GUID_LANGUAGE_LIST,
    GUID_EXTENDED_STREAM_PROPERTIES,
    GUID_ADVANCED_MUTUAL_EXCLUSION,
    GUID_STREAM_PRIORITIZATION,
    GUID_INDEX_PARAMETERS_OBJECT,
    GUID_METADATA_LIBRARY,//add by qian

    GUID_STREAM_TYPE_AUDIO,
    GUID_STREAM_TYPE_VIDEO,
    GUID_STREAM_TYPE_COMMAND,
    GUID_STREAM_TYPE_EXTENDED,
    GUID_STREAM_TYPE_EXTENDED_AUDIO
};
typedef enum guid_type_e guid_type_t;


//add by qian
struct asf_content_branding_s
{

    uint32_t banner_img_type;
    uint32_t banner_img_data_size;
    void* banner_img_data;

    uint32_t banner_img_url_len;
    char*    banner_img_url;

    uint32_t copyright_url_len;
    char*    copyright_url;
};
typedef struct asf_content_branding_s asf_content_branding_t;

struct asf_obj_extended_content_description_s
{

    //other info discard, not parse
    uint32_t extended_content_num;
    uint32_t extended_content_wm_pic_len;
    void* extended_content_wm_pic;
};
typedef struct asf_obj_extended_content_description_s asf_obj_extended_content_description_t;


struct asf_obj_content_description_s
{
    uint32_t Title_len;
    char*  Title;
    uint32_t Author_len;
    char*  Author;
    uint32_t Copyright_len;
    char*  Copyright;
    uint32_t Description_len;
    char*  Description;
    uint32_t Rating_len;
    char*  Rating;
};
typedef struct asf_obj_content_description_s   asf_obj_content_description_t;


//add by qian
struct asf_index_specifiers_s
{
    uint16_t   stream_num;
    uint16_t   index_type;
};
typedef struct asf_index_specifiers_s asf_index_specifiers_t;

struct asf_obj_index_parameters_s
{

    //other info discard, not parse
    uint32_t index_entry_time_interval;
    uint16_t index_specifiers_count;
    asf_index_specifiers_t *index_specifiers_entry;
};
typedef struct asf_obj_index_parameters_s asf_obj_index_parameters_t;




#endif
