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

#ifndef _APE_PARSER_H
#define _APE_PARSER_H

#include <stdio.h>

/* The earliest and latest file formats supported by this library */
#define APE_MIN_VERSION 3950 /* originally set to 3970, but 3950 should work */
#define APE_MAX_VERSION 3990

#define MAC_FORMAT_FLAG_8_BIT                 1    // is 8-bit [OBSOLETE]
#define MAC_FORMAT_FLAG_CRC                   2    // uses the new CRC32 error detection [OBSOLETE]
#define MAC_FORMAT_FLAG_HAS_PEAK_LEVEL        4    // uint32 nPeakLevel after the header [OBSOLETE]
#define MAC_FORMAT_FLAG_24_BIT                8    // is 24-bit [OBSOLETE]
#define MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS    16    // has the number of seek elements after the peak level
#define MAC_FORMAT_FLAG_CREATE_WAV_HEADER    32    // create the wave header on decompression (not stored)

#define ape_parser_uint32_t unsigned int
#define ape_parser_uint16_t unsigned short
#define ape_parser_uint8_t  unsigned char
#define ape_parser_int32_t signed int
#define ape_parser_int16_t signed short
#define ape_parser_int8_t  signed char

#ifdef __CC_ARM
    #define ape_parser_int64_t long long
#elif defined(__GNUC__)
    #define ape_parser_int64_t long long
#else
    #define ape_parser_int64_t __int64
#endif

struct ape_parser_ctx_t
{
    /* Derived fields */
    ape_parser_uint32_t      junklength;
    ape_parser_uint32_t      firstframe;
    ape_parser_uint32_t      totalsamples;

    /* Info from Descriptor Block */
    char          magic[4];
    ape_parser_int16_t       fileversion;
    ape_parser_int16_t       padding1;
    ape_parser_uint32_t      descriptorlength;
    ape_parser_uint32_t      headerlength;
    ape_parser_uint32_t      seektablelength;
    ape_parser_uint32_t      wavheaderlength;
    ape_parser_uint32_t      audiodatalength;
    ape_parser_uint32_t      audiodatalength_high;
    ape_parser_uint32_t      wavtaillength;
    ape_parser_uint8_t       md5[16];

    /* Info from Header Block */
    ape_parser_uint16_t      compressiontype;
    ape_parser_uint16_t      formatflags;
    ape_parser_uint32_t      blocksperframe;
    ape_parser_uint32_t      finalframeblocks;
    ape_parser_uint32_t      totalframes;
    ape_parser_uint16_t      bps;
    ape_parser_uint16_t      channels;
    ape_parser_uint32_t      samplerate;

    /* Seektable */
    ape_parser_uint32_t*     seektable;        /* Seektable buffer */
    ape_parser_uint32_t      maxseekpoints;    /* Max seekpoints we can store (size of seektable buffer) */
    ape_parser_uint32_t      numseekpoints;    /* Number of seekpoints */
    int           seektablefilepos; /* Location in .ape file of seektable */
};

int
ape_parseheader(FILE* fp,
                struct ape_parser_ctx_t* ape_ctx);

void
ape_dumpinfo(struct ape_parser_ctx_t* ape_ctx);

int
ape_calc_seekpos_by_millisecond(struct ape_parser_ctx_t* ape_ctx,
                                ape_parser_uint32_t millisecond,
                                ape_parser_uint32_t* newframe,
                                ape_parser_uint32_t* filepos,
                                ape_parser_uint32_t* firstbyte,
                                ape_parser_uint32_t* blocks_to_skip);

int
ape_calc_seekpos(struct ape_parser_ctx_t* ape_ctx,
             ape_parser_uint32_t new_blocks,
             ape_parser_uint32_t* newframe,
             ape_parser_uint32_t* filepos,
             ape_parser_uint32_t* firstbyte,
             ape_parser_uint32_t* blocks_to_skip);

#endif
