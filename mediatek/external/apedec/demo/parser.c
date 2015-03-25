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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#ifdef APE_MAX
#undef APE_MAX
#endif
#define APE_MAX(a,b) ((a)>(b)?(a):(b))

#define ape_parser_inline static __inline

#if 0
ape_parser_inline ape_parser_int16_t get_int16(unsigned char* buf)
{
    return(buf[0] | (buf[1] << 8));
}

ape_parser_inline ape_parser_uint16_t get_uint16(unsigned char* buf)
{
    return(buf[0] | (buf[1] << 8));
}

ape_parser_inline ape_parser_uint32_t get_uint32(unsigned char* buf)
{
    return(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}
#endif

//static int read_uint16(int fd, uint16_t* x)
static int read_uint16(FILE *fp, ape_parser_uint16_t* x)
{
    unsigned char tmp[2];
    int n;

    //n = read(fd,tmp,2);
    n = fread(tmp, 1, 2, fp);

    if (n != 2)
        return -1;

    *x = tmp[0] | (tmp[1] << 8);

    return 0;
}

//static int read_int16(int fd, int16_t* x)
static int read_int16(FILE *fp, ape_parser_int16_t* x)
{
    return read_uint16(fp, (ape_parser_uint16_t*)x);
}

//static int read_uint32(int fd, uint32_t* x)
static int read_uint32(FILE *fp, ape_parser_uint32_t* x)
{
    unsigned char tmp[4];
    int n;

    //n = read(fd,tmp,4);
    n = fread(tmp, 1, 4, fp);

    if (n != 4)
        return -1;

    *x = tmp[0] | (tmp[1] << 8) | (tmp[2] << 16) | (tmp[3] << 24);

    return 0;
}

//int ape_parseheader(int fd, struct ape_parser_ctx_t* ape_ctx)
int ape_parseheader(FILE* fp, struct ape_parser_ctx_t* ape_ctx)
{
    int i,n;

    /* Skip any leading junk such as id3v2 tags */
    memset(ape_ctx, 0, sizeof(struct ape_parser_ctx_t));
    while (1) {
        char sync[5];
        if (4 != fread(sync, 1, 4, fp))
            return -1;
        sync[4] = 0;
        if (strcmp(sync,"MAC ") == 0)
            break;
        else if (strcmp(sync + 1,"MAC") == 0) {
            fseek(fp, -1, SEEK_CUR);
            ape_ctx->junklength += 1;
            }
        else if (strcmp(sync + 2, "MA") == 0) {
            fseek(fp, -2, SEEK_CUR);
            ape_ctx->junklength += 2;
            }
        else if (sync[3]=='M') {
            fseek(fp, -3, SEEK_CUR);
            ape_ctx->junklength += 3;
            }
        else {
            ape_ctx->junklength += 4;
            }
        }


    fseek(fp, ape_ctx->junklength, SEEK_SET);

    n = fread(&ape_ctx->magic,1, 4, fp);
    if (n != 4) return -1;

    if (memcmp(ape_ctx->magic,"MAC ",4)!=0)
    {
        return -1;
    }

    if (read_int16(fp,&ape_ctx->fileversion) < 0)
        return -1;

    if (ape_ctx->fileversion >= 3980)
    {
        if (read_int16(fp,&ape_ctx->padding1) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->descriptorlength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->headerlength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->seektablelength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->wavheaderlength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->audiodatalength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->audiodatalength_high) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->wavtaillength) < 0)
            return -1;
        if (fread(&ape_ctx->md5,1, 16, fp) != 16)
            return -1;

        /* Skip any unknown bytes at the end of the descriptor.  This is for future
           compatibility */
        if (ape_ctx->descriptorlength > 52)
            fseek(fp, ape_ctx->descriptorlength - 52, SEEK_CUR);

        /* Read header data */
        if (read_uint16(fp,&ape_ctx->compressiontype) < 0)
            return -1;
        if (read_uint16(fp,&ape_ctx->formatflags) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->blocksperframe) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->finalframeblocks) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->totalframes) < 0)
            return -1;
        if (read_uint16(fp,&ape_ctx->bps) < 0)
            return -1;
        if (read_uint16(fp,&ape_ctx->channels) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->samplerate) < 0)
            return -1;
    } else {
        ape_ctx->descriptorlength = 0;
        ape_ctx->headerlength = 32;

        if (read_uint16(fp,&ape_ctx->compressiontype) < 0)
            return -1;
        if (read_uint16(fp,&ape_ctx->formatflags) < 0)
            return -1;
        if (read_uint16(fp,&ape_ctx->channels) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->samplerate) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->wavheaderlength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->wavtaillength) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->totalframes) < 0)
            return -1;
        if (read_uint32(fp,&ape_ctx->finalframeblocks) < 0)
            return -1;

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
        {
            fseek(fp, 4, SEEK_CUR);
            ape_ctx->headerlength += 4;
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
        {
            if (read_uint32(fp,&ape_ctx->seektablelength) < 0)
                return -1;
            ape_ctx->headerlength += 4;
            ape_ctx->seektablelength *= sizeof(ape_parser_int32_t);
        } else {
            ape_ctx->seektablelength = ape_ctx->totalframes * sizeof(ape_parser_int32_t);
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_8_BIT)
            ape_ctx->bps = 8;
        else if (ape_ctx->formatflags & MAC_FORMAT_FLAG_24_BIT)
            ape_ctx->bps = 24;
        else
            ape_ctx->bps = 16;

        if (ape_ctx->fileversion >= 3950)
            ape_ctx->blocksperframe = 73728 * 4;
        else if ((ape_ctx->fileversion >= 3900) || (ape_ctx->fileversion >= 3800 && ape_ctx->compressiontype >= 4000))
            ape_ctx->blocksperframe = 73728;
        else
            ape_ctx->blocksperframe = 9216;

        /* Skip any stored wav header */
        if (!(ape_ctx->formatflags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
        {
            fseek(fp, ape_ctx->wavheaderlength, SEEK_CUR);
        }
    }

    ape_ctx->totalsamples = ape_ctx->finalframeblocks;
    if (ape_ctx->totalframes > 1)
        ape_ctx->totalsamples += ape_ctx->blocksperframe * (ape_ctx->totalframes-1);

    if (ape_ctx->seektablelength > 0)
    {
        ape_ctx->seektable = malloc(ape_ctx->seektablelength);
        if (ape_ctx->seektable == NULL)
            return -1;
        for (i=0; i < ape_ctx->seektablelength / sizeof(ape_parser_uint32_t); i++)
        {
            if (read_uint32(fp,&ape_ctx->seektable[i]) < 0)
            {
                 free(ape_ctx->seektable);
                 return -1;
            }
        }
    }

    ape_ctx->firstframe = ape_ctx->junklength + ape_ctx->descriptorlength +
                           ape_ctx->headerlength + ape_ctx->seektablelength +
                           ape_ctx->wavheaderlength;

    return 0;
}

void ape_dumpinfo(struct ape_parser_ctx_t* ape_ctx)
{
  int i;

    printf("Descriptor Block:\n\n");
    printf("magic                = \"%c%c%c%c\"\n",
            ape_ctx->magic[0],ape_ctx->magic[1],
            ape_ctx->magic[2],ape_ctx->magic[3]);
    printf("fileversion          = %d\n",ape_ctx->fileversion);
    printf("descriptorlength     = %d\n",ape_ctx->descriptorlength);
    printf("headerlength         = %d\n",ape_ctx->headerlength);
    printf("seektablelength      = %d\n",ape_ctx->seektablelength);
    printf("wavheaderlength      = %d\n",ape_ctx->wavheaderlength);
    printf("audiodatalength      = %d\n",ape_ctx->audiodatalength);
    printf("audiodatalength_high = %d\n",ape_ctx->audiodatalength_high);
    printf("wavtaillength        = %d\n",ape_ctx->wavtaillength);
    printf("md5                  = ");
    for (i = 0; i < 16; i++)
        printf("%02x",ape_ctx->md5[i]);
    printf("\n");

    printf("\nHeader Block:\n\n");

    printf("compressiontype      = %d\n",ape_ctx->compressiontype);
    printf("formatflags          = %d\n",ape_ctx->formatflags);
    printf("blocksperframe       = %d\n",ape_ctx->blocksperframe);
    printf("finalframeblocks     = %d\n",ape_ctx->finalframeblocks);
    printf("totalframes          = %d\n",ape_ctx->totalframes);
    printf("bps                  = %d\n",ape_ctx->bps);
    printf("channels             = %d\n",ape_ctx->channels);
    printf("samplerate           = %d\n",ape_ctx->samplerate);

    printf("\nSeektable\n\n");
    if ((ape_ctx->seektablelength / sizeof(ape_parser_uint32_t)) != ape_ctx->totalframes)
    {
        printf("No seektable\n");
    }
    else
    {
        for ( i = 0; i < ape_ctx->seektablelength / sizeof(ape_parser_uint32_t) ; i++)
        {
            if (i < ape_ctx->totalframes-1) {
                printf("%8d   %d (%d bytes)\n",i,ape_ctx->seektable[i],ape_ctx->seektable[i+1]-ape_ctx->seektable[i]);
            } else {
                printf("%8d   %d\n",i,ape_ctx->seektable[i]);
            }
        }
    }
    printf("\nCalculated information:\n\n");
    printf("junklength           = %d\n",ape_ctx->junklength);
    printf("firstframe           = %d\n",ape_ctx->firstframe);
    printf("totalsamples         = %d\n",ape_ctx->totalsamples);
}

int ape_calc_seekpos(struct ape_parser_ctx_t* ape_ctx,
                     ape_parser_uint32_t new_blocks,
                     ape_parser_uint32_t* newframe,
                     ape_parser_uint32_t* filepos,
                     ape_parser_uint32_t* firstbyte,
                     ape_parser_uint32_t* blocks_to_skip)
{
    ape_parser_uint32_t n, delta;

    n = new_blocks / ape_ctx->blocksperframe;

    if (n >= ape_ctx->totalframes) { return -1; }

    *newframe = n;
    *filepos = ape_ctx->seektable[n];
    *blocks_to_skip = new_blocks - (n * ape_ctx->blocksperframe);

    delta = (*filepos - ape_ctx->firstframe) & 3;
    *firstbyte = 3 - delta;
    *filepos -= delta;

    return 0;
}

int ape_calc_seekpos_by_millisecond(struct ape_parser_ctx_t* ape_ctx,
                                    ape_parser_uint32_t millisecond,
                                    ape_parser_uint32_t* newframe,
                                    ape_parser_uint32_t* filepos,
                                    ape_parser_uint32_t* firstbyte,
                                    ape_parser_uint32_t* blocks_to_skip)
{
    ape_parser_uint32_t n, delta;
    ape_parser_int64_t new_blocks;

    new_blocks = (ape_parser_int64_t)millisecond * (ape_parser_int64_t)ape_ctx->samplerate;
    new_blocks /= 1000;

    n = new_blocks / ape_ctx->blocksperframe;

    if (n >= ape_ctx->totalframes) { return -1; }

    *newframe = n;
    *filepos = ape_ctx->seektable[n];
    *blocks_to_skip = new_blocks - (n * ape_ctx->blocksperframe);

    delta = (*filepos - ape_ctx->firstframe) & 3;
    *firstbyte = 3 - delta;
    *filepos -= delta;

    return 0;
}
