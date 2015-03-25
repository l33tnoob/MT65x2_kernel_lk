#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decoder.h"
#include "crc.h"
#include "ape_decoder_exp.h"
#include <utils/Log.h>

#ifdef __ANDROID__
    #include "drvb_api.h"
#else
    #include "audip_exp.h"
#endif

#define APE_STATE_FRAME_INIT    0
#define APE_STATE_CHUNK_DECODE  1

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// Codec Version Definition

// project (bit31-28)
#define CODEC_PROJECT_SP_TABLET      (0x8)
#define CODEC_PROJECT_FP             (0x9)
#define CODEC_PROJECT_BOX_TV         (0xA)

// compiler (bit27-24)
#define CODEC_COMPILER_VC            (0x0)
#define CODEC_COMPILER_ARM_RVCT      (0x1)
#define CODEC_COMPILER_ARM_GCC       (0x2)
#define CODEC_COMPILER_APOLLO        (0x3)

// major (bit23-16)
// RESERVED
//#define CODEC_MAJOR_MULTI_CH       (0x01)
//#define CODEC_MAJOR_LTP            (0x02)
//#define CODEC_MAJOR_RAW_DEC_MODE   (0x04)
//#define CODEC_MAJOR_LATM           (0x08)

// Version 0.01
#define CODEC_APEDEC_MINOR_VER      (0x00)
#define CODEC_APEDEC_RELEAE_VER     (0x04)
#ifdef __ANDROID__
 int checkhw_result=0;
#endif
int ape_decoder_get_version(void)
{
   unsigned char Project = 0;
   unsigned char Comp = 0;
   unsigned char Major = 0;
   unsigned char Minor = 0;
   unsigned char Release = 0;

#if defined(__ANDROID_SP_TABLET__)
   Project = CODEC_PROJECT_SP_TABLET;
#elif defined(__ANDROID_BOX_TV__)
   Project = CODEC_PROJECT_BOX_TV;
#else
   Project = CODEC_PROJECT_FP;
#endif

#if (__CC_ARM)
   Comp = CODEC_COMPILER_ARM_RVCT;
#elif (__GNUC__)
   Comp = CODEC_COMPILER_ARM_GCC;
#else
   Comp = CODEC_COMPILER_VC;
#endif


   Minor   = CODEC_APEDEC_MINOR_VER;
   Release = CODEC_APEDEC_RELEAE_VER;

   return (((unsigned int)Project << 28) +
            ((unsigned int)Comp << 24)   +
            ((unsigned int)Major << 16)  +
            ((unsigned int)Minor << 8)   +
            (unsigned int)Release);
}


void
ape_decoder_get_mem_size(unsigned int *bs_buffer,
                         unsigned int *working_buffer,
                         unsigned int *pcm_buffer)
{
    *bs_buffer = INPUT_CHUNKSIZE;
    *working_buffer = sizeof(struct ape_dec_internal_t);
    *pcm_buffer = OUTPUT_CHUNKSIZE;
}

ape_decoder_handle
ape_decoder_init(void*  working_buffer,
                 struct ape_decoder_init_param* ape_param)
{
    struct ape_dec_internal_t * ape_dec_internal = working_buffer;
    memset(ape_dec_internal, 0, sizeof(struct ape_dec_internal_t));

    ape_dec_internal->fileversion = ape_param->fileversion;
    ape_dec_internal->compressiontype = ape_param->compressiontype;
    ape_dec_internal->blocksperframe = ape_param->blocksperframe;
    ape_dec_internal->finalframeblocks = ape_param->finalframeblocks;
    ape_dec_internal->totalframes = ape_param->totalframes;
    ape_dec_internal->bps = ape_param->bps;
    ape_dec_internal->channels = ape_param->channels;

    ape_dec_internal->frame_decode_state = APE_STATE_FRAME_INIT;
    ape_dec_internal->first_byte = 3;
    ape_dec_internal->curr_frame = 0;
    ape_dec_internal->curr_blocks_in_curr_frame = 0;
    ape_dec_internal->curr_frame_crc = 0;
    ape_dec_internal->err_flag = 0;
#ifdef __ANDROID__
    //CHECK_MTK_HW_1(checkhw_result)
#endif

#ifdef NO_SUPPORT_TO_4000
    if (ape_dec_internal->compressiontype > 3000)
		return 0;
#endif

#ifdef NO_SUPPORT_TO_5000
    if (ape_dec_internal->compressiontype > 4000)
		return 0;
#endif
    return ape_dec_internal;
}

int
ape_decoder_reset(ape_decoder_handle handle,
                  int firstbyte,
                  int newframe)
{
    struct ape_dec_internal_t * ape_dec_internal = handle;
    ape_dec_internal->frame_decode_state = APE_STATE_FRAME_INIT;
    ape_dec_internal->first_byte = firstbyte;
    ape_dec_internal->curr_frame = newframe;
    ape_dec_internal->curr_blocks_in_curr_frame = 0;
    ape_dec_internal->curr_frame_crc = 0;
    ape_dec_internal->err_flag = 0;
    return 0;
}
#if APE_24BIT_SUPPORT
int APEDEPTH;
#endif
extern int g_count;
int
ape_decoder_decode(ape_decoder_handle handle,
                   unsigned char* inbuffer,
                   int* bytes_consumed,
                   unsigned char* outbuffer,
                   int* bytes_produced)
{
    struct ape_dec_internal_t* ape_dec_internal = handle;
    int32_t* decoded0 = ape_dec_internal->decoded0;
    int32_t* decoded1 = ape_dec_internal->decoded1;

#ifdef __ANDROID__
    //CHECK_MTK_HW(ape_dec_internal)
    //CHECK_MTK_HW_2(ape_dec_internal,checkhw_result)
#else
    AUDIP_COMMON()
    AUDIP_DRVSET()
#endif

    if (ape_dec_internal->frame_decode_state == APE_STATE_FRAME_INIT) {
        //calculate current frame and blocks
        if (ape_dec_internal->curr_frame == ape_dec_internal->totalframes - 1)
            ape_dec_internal->curr_blocks_in_curr_frame = ape_dec_internal->finalframeblocks;
        else if (ape_dec_internal->curr_frame < ape_dec_internal->totalframes - 1)
            ape_dec_internal->curr_blocks_in_curr_frame = ape_dec_internal->blocksperframe;
        else
            return APE_ERR_EOS;

        ape_dec_internal->currentframeblocks = ape_dec_internal->curr_blocks_in_curr_frame;

        //init_frame_coder
        init_frame_decoder(ape_dec_internal, inbuffer, &ape_dec_internal->first_byte, bytes_consumed);

        //init crc
        ape_dec_internal->curr_frame_crc = ape_initcrc();

        #if APE_24BIT_SUPPORT
        APEDEPTH = ape_dec_internal->bps;
        #endif

        ape_dec_internal->frame_decode_state = APE_STATE_CHUNK_DECODE;
        *bytes_produced = 0;
        return 0;
        }

    if (ape_dec_internal->frame_decode_state == APE_STATE_CHUNK_DECODE) {
        //calculate blocks in current pass
        int blocks_this_pass = MIN(BLOCKS_PER_LOOP, ape_dec_internal->curr_blocks_in_curr_frame);
        int ret = 0, i = 0;
        unsigned char* p = outbuffer;
        int16_t  sample16;
        int32_t  sample32;

        //decode chunk
        ret = decode_chunk(ape_dec_internal,
                                inbuffer,
                                &ape_dec_internal->first_byte,
                                bytes_consumed,
                                decoded0,
                                decoded1,
                                blocks_this_pass);
        if (ret < 0)
        {
            #if APE_DEBUG
            LOGD("[zhengwen] APE_ERROR_CRC decode_chunk");
            #endif
            return APE_ERR_CRC;
        }

        if (ape_dec_internal->bps == 8) {
            if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_PSEUDO) {
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                while (count--) {
                    int left  = *__decoded0;
                    *(__decoded1++) = *(__decoded0++) = left;
                    }
                }
            else if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_STEREO) {
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                while (count--) {
                    int left, right;
                    left = *__decoded1 - (*__decoded0 / 2);
                    right = left + *__decoded0;
                    *(__decoded0++) = left;
                    *(__decoded1++) = right;
                    }
                }
            for (i = 0 ; i < blocks_this_pass ; i++) {
                *(p++) = (decoded0[i] + 0x80) & 0xff;
                if (ape_dec_internal->channels == 2) {
                    *(p++) = (decoded1[i] + 0x80) & 0xff;
                    }
                }
            ape_dec_internal->curr_frame_crc = ape_updatecrc(outbuffer, p - outbuffer, ape_dec_internal->curr_frame_crc);
            p = outbuffer;
            for (i = 0 ; i < blocks_this_pass ; i++) {
                *p++ = 0;
                *p++ = decoded0[i] & 0xff;
                if (ape_dec_internal->channels == 2) {
                    *p++ = 0;
                    *p++ = decoded1[i] & 0xff;
                    }
                }
            }
        else if (ape_dec_internal->bps == 16) {
            if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_STEREO) {
                /* only this path (stereo without silence) is optzed */
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                unsigned int crc = ape_dec_internal->curr_frame_crc;
                short *pcm = (short*) p;
                while (count--) {
                    int left, right;
                    int16_t  sample;
                    uint32_t crc_temp;
                    left = *__decoded1++ - (*__decoded0 / 2);
                    right = left + *__decoded0++;

                    sample = left & 0xff;
                    crc_temp = crctab32[(crc & 0xff) ^ sample];
                    sample = (left >> 8) & 0xff;
                    crc = (crc >> 8) ^ crc_temp;

                    crc_temp = crctab32[(crc & 0xff) ^ sample];
                    *pcm++ = left;
                    crc = (crc >> 8) ^ crc_temp;

                    sample = right & 0xff;
                    crc_temp = crctab32[(crc & 0xff) ^ sample];
                    sample = (right >> 8) & 0xff;
                    crc = (crc >> 8) ^ crc_temp;

                    crc_temp = crctab32[(crc & 0xff) ^ sample];
                    *pcm++ = right;
                    crc = (crc >> 8) ^ crc_temp;
                    }
                ape_dec_internal->curr_frame_crc = crc;
                p = (unsigned char *) pcm;
                }
            else if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_PSEUDO) {
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                while (count--) {
                    int left;
                    left = *__decoded0;
                    *(__decoded1++) = *(__decoded0++) = left;
                    }
                for (i = 0 ; i < blocks_this_pass ; i++) {
                    sample16 = decoded0[i];
                    *(p++) = sample16 & 0xff;
                    *(p++) = (sample16 >> 8) & 0xff;
                    if (ape_dec_internal->channels == 2) {
                        sample16 = decoded1[i];
                        *(p++) = sample16 & 0xff;
                        *(p++) = (sample16 >> 8) & 0xff;
                        }
                    }
                ape_dec_internal->curr_frame_crc = ape_updatecrc(outbuffer, p - outbuffer, ape_dec_internal->curr_frame_crc);
                }
            else {
                for (i = 0 ; i < blocks_this_pass ; i++) {
                    sample16 = decoded0[i];
                    *(p++) = sample16 & 0xff;
                    *(p++) = (sample16 >> 8) & 0xff;
                    if (ape_dec_internal->channels == 2) {
                        sample16 = decoded1[i];
                        *(p++) = sample16 & 0xff;
                        *(p++) = (sample16 >> 8) & 0xff;
                        }
                    }
                ape_dec_internal->curr_frame_crc = ape_updatecrc(outbuffer, p - outbuffer, ape_dec_internal->curr_frame_crc);
                }
            }
        else if (ape_dec_internal->bps == 24) {
            if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_PSEUDO) {
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                while (count--) {
                    int left  = *__decoded0;
                    *(__decoded1++) = *(__decoded0++) = left;
                    }
                }
            else if (ape_dec_internal->mid_side_decode_type == MID_SIDE_DECODE_STEREO) {
                int32_t *__decoded0 = decoded0, *__decoded1 = decoded1;
                int count = blocks_this_pass;
                while (count--) {
                    int left, right;
                    left = *__decoded1 - (*__decoded0 / 2);
                    right = left + *__decoded0;
                    *(__decoded0++) = left;
                    *(__decoded1++) = right;
                    }
                }
            for (i = 0 ; i < blocks_this_pass ; i++) {
                sample32 = decoded0[i];
                *(p++) = sample32 & 0xff;
                *(p++) = (sample32 >> 8) & 0xff;
                *(p++) = (sample32 >> 16) & 0xff;
                if (ape_dec_internal->channels == 2) {
                    sample32 = decoded1[i];
                    *(p++) = sample32 & 0xff;
                    *(p++) = (sample32 >> 8) & 0xff;
                    *(p++) = (sample32 >> 16) & 0xff;
                    }
                }
            ape_dec_internal->curr_frame_crc = ape_updatecrc(outbuffer, p - outbuffer, ape_dec_internal->curr_frame_crc);
            }
        *bytes_produced = p - outbuffer;
        #if APE_DEBUG
        if((g_count >= 0*864 && g_count <= 1*864))
        {
        LOGD("=======================unprepare Decoded0=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded0[%d] = %8x",i,decoded0[i]);
        }
        LOGD("=======================unprepare Decoded1=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded1[%d] = %8x",i,decoded1[i]);
        }
        LOGD("==========================================================");
        }
        #endif
        //update block count
        ape_dec_internal->curr_blocks_in_curr_frame -= blocks_this_pass;

        #if APE_DEBUG
        LOGD("[zhengwen] === %d frame ==== %d block decoding done", ape_dec_internal->curr_frame, (ape_dec_internal->blocksperframe - ape_dec_internal->curr_blocks_in_curr_frame));
        #endif

        if (ape_dec_internal->curr_blocks_in_curr_frame == 0) {
            if (ape_dec_internal->fileversion <= 3950) {
                switch (ape_dec_internal->first_byte) {
                    case 0:
                        ape_dec_internal->first_byte = 2; break;
                    case 1:
                        ape_dec_internal->first_byte = 3; break;
                    case 2:
                        *bytes_consumed -= 4;
                        ape_dec_internal->first_byte = 0; break;
                    case 3:
                        *bytes_consumed -= 4;
                        ape_dec_internal->first_byte = 1; break;
                    default:
                        return APE_ERR_CRC;
                    }
                }
            ape_dec_internal->curr_frame++;
            ape_dec_internal->frame_decode_state = APE_STATE_FRAME_INIT;
            ape_dec_internal->curr_frame_crc = ape_finishcrc(ape_dec_internal->curr_frame_crc);
            if (ape_dec_internal->CRC != ape_dec_internal->curr_frame_crc)
            {
                #if APE_DEBUG
                LOGD("[zhengwen] APE_ERROR_CRC update block count");
                #endif
                return APE_ERR_CRC;
            }
        }
        return 0;
        }

    return 0;
}

