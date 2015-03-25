/*

libdemac - A Monkey's Audio decoder

$Id$

Copyright (C) Dave Chapman 2007

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA

*/

#ifndef _APE_DECODER_H
#define _APE_DECODER_H

#include "demac_config.h"

/* Special frame codes:

   MONO_SILENCE - All PCM samples in frame are zero (mono streams only)
   LEFT_SILENCE - All PCM samples for left channel in frame are zero (stereo streams)
   RIGHT_SILENCE - All PCM samples for left channel in frame are zero (stereo streams)
   PSEUDO_STEREO - Left and Right channels are identical

*/

#define APE_FRAMECODE_MONO_SILENCE    1
#define APE_FRAMECODE_LEFT_SILENCE    1 /* same as mono */
#define APE_FRAMECODE_RIGHT_SILENCE   2
#define APE_FRAMECODE_STEREO_SILENCE  3 /* combined */
#define APE_FRAMECODE_PSEUDO_STEREO   4

#define PREDICTOR_LEN 8
/* Total size of all predictor histories - 50 * sizeof(int32_t) */
#define PREDICTOR_SIZE 50

#define MID_SIDE_DECODE_NONE 0
#define MID_SIDE_DECODE_PSEUDO 1
#define MID_SIDE_DECODE_STEREO 2

#define MAX_CHANNELS        2
#define MAX_BYTESPERSAMPLE  3
#define BLOCKS_PER_LOOP     864 /* originally config to 4608 */

#define INPUT_CHUNKSIZE     (BLOCKS_PER_LOOP*MAX_CHANNELS*MAX_BYTESPERSAMPLE)
#define OUTPUT_CHUNKSIZE    (BLOCKS_PER_LOOP*MAX_CHANNELS*MAX_BYTESPERSAMPLE)

struct predictor_t
{
    /* Filter histories */
    int32_t* buffer;

    int32_t YlastA;
    int32_t XlastA;

    /* NOTE: The order of the next four fields is important for
       predictor-arm.S */
    int32_t YfilterB;
    int32_t XfilterA;
    int32_t XfilterB;
    int32_t YfilterA;

    /* Adaption co-efficients */
    int32_t YcoeffsA[4];
    int32_t XcoeffsA[4];
    int32_t YcoeffsB[5];
    int32_t XcoeffsB[5];
    int32_t historybuffer[PREDICTOR_HISTORY_SIZE + PREDICTOR_SIZE];
};

struct filter_t {
    filter_int* coef_original; /* FILTER_LEN entries */

    /* We store all the filter delays in a single buffer */
    filter_int* history_end;

    filter_int* delay;
    filter_int* coef_adapted;

    int avg;
};

struct rangecoder_t
{
    uint32_t low;        /* low end of interval */
    uint32_t range;      /* length of interval */
    uint32_t help;       /* bytes_to_follow resp. intermediate value */
    unsigned int buffer; /* buffer for input/output */
};

struct rice_t
{
  uint32_t k;
  uint32_t ksum;
};

struct ape_dec_internal_t
{
    int16_t       fileversion;
    uint16_t      compressiontype;
    uint32_t      blocksperframe;
    uint32_t      finalframeblocks;
    uint32_t      totalframes;
    uint16_t      bps;
    uint16_t      channels;

    /* Decoder state */
    uint32_t      CRC;
    int           entropy_flags;
    int           currentframeblocks;
    int           blocksdecoded;
    struct predictor_t predictor;

    unsigned char* bytebuffer;
    int bytebufferoffset;
    struct rangecoder_t rc;
    struct rice_t riceX;
    struct rice_t riceY;

    struct filter_t filter_16_0;
    struct filter_t filter_16_1;

    struct filter_t filter_32_0;
    struct filter_t filter_32_1;

    struct filter_t filter_64_0;
    struct filter_t filter_64_1;

    struct filter_t filter_256_0;
    struct filter_t filter_256_1;

    struct filter_t filter_1280_0;
    struct filter_t filter_1280_1;

    filter_int filterbuf64[(64*3 + FILTER_HISTORY_SIZE) * 2];
    filter_int filterbuf256[(256*3 + FILTER_HISTORY_SIZE) * 2];
    filter_int filterbuf1280[(1280*3 + FILTER_HISTORY_SIZE) * 2];

    int frame_decode_state;
    int first_byte;
    int curr_frame;
    int curr_blocks_in_curr_frame;
    unsigned int curr_frame_crc;

    int mid_side_decode_type;

    int32_t decoded0[BLOCKS_PER_LOOP];
    int32_t decoded1[BLOCKS_PER_LOOP];

    int err_flag;
};


STATIC_DECLARE void init_frame_decoder(struct ape_dec_internal_t* ape_dec_internal,
                        unsigned char* inbuffer, int* firstbyte,
                        int* bytesconsumed);

STATIC_DECLARE int decode_chunk(struct ape_dec_internal_t* ape_dec_internal,
                 unsigned char* inbuffer, int* firstbyte,
                 int* bytesconsumed,
                 int32_t* decoded0, int32_t* decoded1, 
                 int count);
#endif
