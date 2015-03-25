#include <string.h>
#include "decoder.h"
#include "predictor.h"
#include "filter.h"
#include "entropy.h"
#include <utils/Log.h>

#if defined(__ARMv6)
extern void predictor_decode_stereo_asm(struct predictor_t* , int32_t*, int32_t* , int );
extern void predictor_decode_mono_asm(struct predictor_t*, int32_t* , int) ;
#define PREDICTOR_DECODE_STEREO(a,b,c,d)  predictor_decode_stereo_asm(a,b,c,d);
#define PREDICTOR_DECODE_MONO(a,b,d)  predictor_decode_mono_asm(a,b,d);
#else
#define PREDICTOR_DECODE_STEREO(a,b,c,d)  predictor_decode_stereo(a,b,c,d);
#define PREDICTOR_DECODE_MONO(a,b,d)  predictor_decode_mono(a,b,d);
#endif

STATIC_DECLARE void init_frame_decoder(struct ape_dec_internal_t* ape_dec_internal,
                        unsigned char* inbuffer,
                        int* firstbyte,
                        int* bytesconsumed)
{
    init_entropy_decoder(ape_dec_internal, inbuffer, firstbyte, bytesconsumed);

    init_predictor_decoder(&ape_dec_internal->predictor);

    switch (ape_dec_internal->compressiontype) {
        case 2000:
            ape_filter_init_16_11(&ape_dec_internal->filter_16_0, &ape_dec_internal->filter_16_1, ape_dec_internal->filterbuf64);
            break;

        case 3000:
            ape_filter_init_64_11(&ape_dec_internal->filter_64_0, &ape_dec_internal->filter_64_1, ape_dec_internal->filterbuf64);
            break;

#ifndef NO_SUPPORT_TO_4000
        case 4000:
            ape_filter_init_256_13(&ape_dec_internal->filter_256_0, &ape_dec_internal->filter_256_1, ape_dec_internal->filterbuf256);
            ape_filter_init_32_10(&ape_dec_internal->filter_32_0, &ape_dec_internal->filter_32_1, ape_dec_internal->filterbuf64);
            break;
#endif

#ifndef NO_SUPPORT_TO_5000
        case 5000:
            ape_filter_init_1280_15(&ape_dec_internal->filter_1280_0, &ape_dec_internal->filter_1280_1, ape_dec_internal->filterbuf1280);
            ape_filter_init_256_13(&ape_dec_internal->filter_256_0, &ape_dec_internal->filter_256_1, ape_dec_internal->filterbuf256);
            ape_filter_init_16_11(&ape_dec_internal->filter_16_0, &ape_dec_internal->filter_16_1, ape_dec_internal->filterbuf64);
            break;
#endif
        }
}

int g_count = 0;//zhengwen

STATIC_DECLARE int ICODE_ATTR_DEMAC decode_chunk(struct ape_dec_internal_t* ape_dec_internal,
                                  unsigned char* inbuffer,
                                  int* firstbyte,
                                  int* bytesconsumed,
                                  int32_t* decoded0,
                                  int32_t* decoded1,
                                  int count)
{
    int i;//zhengwen

    if (count <= 0)
        return -1;
    if ((ape_dec_internal->channels==1) || ((ape_dec_internal->entropy_flags
        & (APE_FRAMECODE_PSEUDO_STEREO|APE_FRAMECODE_STEREO_SILENCE))
        == APE_FRAMECODE_PSEUDO_STEREO)) {

        entropy_decode(ape_dec_internal, inbuffer, firstbyte, bytesconsumed, decoded0, NULL, count);

        if (ape_dec_internal->err_flag) {
            return ape_dec_internal->err_flag;
            }

        ape_dec_internal->mid_side_decode_type = MID_SIDE_DECODE_NONE;
        if (ape_dec_internal->entropy_flags & APE_FRAMECODE_MONO_SILENCE) {
            return 0;
            }

        switch (ape_dec_internal->compressiontype) {
            case 2000:
                ape_filter_apply_16_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_16_0,&ape_dec_internal->filter_16_1,
                                   decoded0,NULL,count);
                break;

            case 3000:
                ape_filter_apply_64_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_64_0,&ape_dec_internal->filter_64_1,
                                   decoded0,NULL,count);
                break;

#ifndef NO_SUPPORT_TO_4000
            case 4000:
                ape_filter_apply_32_10(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_32_0,&ape_dec_internal->filter_32_1,
                                   decoded0,NULL,count);
                ape_filter_apply_256_13(ape_dec_internal->fileversion,
                                    &ape_dec_internal->filter_256_0,&ape_dec_internal->filter_256_1,
                                    decoded0,NULL,count);
                break;
#endif

#ifndef NO_SUPPORT_TO_5000
            case 5000:
                ape_filter_apply_16_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_16_0,&ape_dec_internal->filter_16_1,
                                   decoded0,NULL,count);
                ape_filter_apply_256_13(ape_dec_internal->fileversion,
                                    &ape_dec_internal->filter_256_0,&ape_dec_internal->filter_256_1,
                                    decoded0,NULL,count);
                ape_filter_apply_1280_15(ape_dec_internal->fileversion,
                                     &ape_dec_internal->filter_1280_0,&ape_dec_internal->filter_1280_1,
                                     decoded0,NULL,count);
#endif
            }
         PREDICTOR_DECODE_MONO(&ape_dec_internal->predictor,decoded0,count);
        //predictor_decode_mono(&ape_dec_internal->predictor,decoded0,count);

        if (ape_dec_internal->channels==2) {
            ape_dec_internal->mid_side_decode_type = MID_SIDE_DECODE_PSEUDO;
            }
        }
    else { /* Stereo */
        entropy_decode(ape_dec_internal, inbuffer, firstbyte, bytesconsumed, decoded0, decoded1, count);
        #if APE_DEBUG
        g_count+=count;//zhengwen
        LOGD("[zhengwen] stereo entropy_decode finish!, g_count = %d, err_flag = %d", g_count,ape_dec_internal->err_flag);
        if((g_count >= 0*864 && g_count <= 1*864))
        {
        LOGD("=======================entropy_decode Decoded0=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded0[%d] = %8x",i,decoded0[i]);
        }
        LOGD("=======================entropy_decode Decoded1=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded1[%d] = %8x",i,decoded1[i]);
        }
        LOGD("==========================================================");
        }
        #endif
        if (ape_dec_internal->err_flag) {
            return ape_dec_internal->err_flag;
            }

        ape_dec_internal->mid_side_decode_type = MID_SIDE_DECODE_NONE;
        if ((ape_dec_internal->entropy_flags & APE_FRAMECODE_STEREO_SILENCE)
            == APE_FRAMECODE_STEREO_SILENCE) {
            return 0;
            }
        /* Apply filters - compression type 1000 doesn't have any */
        switch (ape_dec_internal->compressiontype) {
            case 2000:
                ape_filter_apply_16_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_16_0,&ape_dec_internal->filter_16_1,
                                   decoded0,decoded1,count);
                break;

            case 3000:
                ape_filter_apply_64_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_64_0,&ape_dec_internal->filter_64_1,
                                   decoded0,decoded1,count);
                break;

#ifndef NO_SUPPORT_TO_4000
            case 4000:
                ape_filter_apply_32_10(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_32_0,&ape_dec_internal->filter_32_1,
                                   decoded0,decoded1,count);
                ape_filter_apply_256_13(ape_dec_internal->fileversion,
                                    &ape_dec_internal->filter_256_0,&ape_dec_internal->filter_256_1,
                                    decoded0,decoded1,count);
                break;
#endif

#ifndef NO_SUPPORT_TO_5000
            case 5000:
                ape_filter_apply_16_11(ape_dec_internal->fileversion,
                                   &ape_dec_internal->filter_16_0,&ape_dec_internal->filter_16_1,
                                   decoded0,decoded1,count);
                ape_filter_apply_256_13(ape_dec_internal->fileversion,
                                    &ape_dec_internal->filter_256_0,&ape_dec_internal->filter_256_1,
                                    decoded0,decoded1,count);
                ape_filter_apply_1280_15(ape_dec_internal->fileversion,
                                     &ape_dec_internal->filter_1280_0,&ape_dec_internal->filter_1280_1,
                                     decoded0,decoded1,count);
#endif
            }
        #if APE_DEBUG
        if((g_count >= 0*864 && g_count <= 1*864))
        {
        LOGD("=======================filter Decoded0=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded0[%d] = %8x",i,decoded0[i]);
        }
        LOGD("=======================filter Decoded1=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded1[%d] = %8x",i,decoded1[i]);
        }
        LOGD("==========================================================");
        }
        #endif
         PREDICTOR_DECODE_STEREO(&ape_dec_internal->predictor,decoded0,decoded1,count);
        #if APE_DEBUG
         if((g_count >= 0*864 && g_count <= 1*864))
        {
        LOGD("=======================predictor Decoded0=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded0[%d] = %8x",i,decoded0[i]);
        }
        LOGD("=======================predictor Decoded1=================");
        for(i = 0; i < 20;i++)
        {
            LOGD("decoded1[%d] = %8x",i,decoded1[i]);
        }
        LOGD("==========================================================");
        }
        #endif
        //predictor_decode_stereo(&ape_dec_internal->predictor,decoded0,decoded1,count);
        ape_dec_internal->mid_side_decode_type = MID_SIDE_DECODE_STEREO;
        }
    return 0;
}
