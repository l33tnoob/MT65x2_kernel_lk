#include <string.h>
#include "filter.h"
#include "vector_math_generic.h"
#include <utils/Log.h>

#if QBITS == 11
    #if FILTER_LEN == 16
        #define ape_filter_init   ape_filter_init_16_11
        #define ape_filter_apply  ape_filter_apply_16_11
    #elif FILTER_LEN == 64
        #define ape_filter_init  ape_filter_init_64_11
        #define ape_filter_apply ape_filter_apply_64_11
    #endif
#elif QBITS == 13
    #define ape_filter_init  ape_filter_init_256_13
    #define ape_filter_apply ape_filter_apply_256_13
#elif QBITS == 10
    #define ape_filter_init  ape_filter_init_32_10
    #define ape_filter_apply ape_filter_apply_32_10
#elif QBITS == 15
    #define ape_filter_init  ape_filter_init_1280_15
    #define ape_filter_apply ape_filter_apply_1280_15
#endif

#ifdef STATIC_ENHANCE
#if QBITS == 11
    #if FILTER_LEN == 16
        #define do_ape_filter_apply_3980   do_ape_filter_apply_3980_16_11
        #define do_ape_filter_apply_3970  do_ape_filter_apply_3970_16_11
        #define do_ape_filter_init      do_ape_filter_init_16_11
    #elif FILTER_LEN == 64
        #define do_ape_filter_apply_3980   do_ape_filter_apply_3980_64_11
        #define do_ape_filter_apply_3970  do_ape_filter_apply_3970_64_11
        #define do_ape_filter_init      do_ape_filter_init_64_11
    #endif
#elif QBITS == 13
        #define do_ape_filter_apply_3980   do_ape_filter_apply_3980_256_13
        #define do_ape_filter_apply_3970  do_ape_filter_apply_3970_256_13
        #define do_ape_filter_init      do_ape_filter_init_256_13
#elif QBITS == 10
        #define do_ape_filter_apply_3980   do_ape_filter_apply_3980_32_10
        #define do_ape_filter_apply_3970  do_ape_filter_apply_3970_32_10
        #define do_ape_filter_init      do_ape_filter_init_32_10
#elif QBITS == 15
        #define do_ape_filter_apply_3980   do_ape_filter_apply_3980_1280_15
        #define do_ape_filter_apply_3970  do_ape_filter_apply_3970_1280_15
        #define do_ape_filter_init      do_ape_filter_init_1280_15
#endif
#endif

#define FRAC_TO_INT(x) ((x + (1 << (QBITS - 1))) >> QBITS)  /* round(x) */

#if defined(___CPU_ARM9E___) && defined(__CC_ARM)
    __inline int saturate_arm9e(int x)
    {
    int mask = 0x7fff, t;
    __asm {
        MOV t, x, asr #15;
        CMP t, x, asr #31;
        EORNE x, mask, x, asr #31;
        }
    return x;
    }
    #define SATURATE(x) saturate_arm9e(x)
#else
    #define SATURATE(x) (LIKELY((x) == (int16_t)(x)) ? (x) : ((x) >> 31) ^ 0x7FFF)
#endif

extern int unaligned_dot_and_add_16(short *, short *, short *);
extern int unaligned_dot_and_sub_16(short *, short *, short *);
extern int aligned_dot_and_add_16(short *, short *, short *);
extern int aligned_dot_and_sub_16(short *, short *, short *);

extern int unaligned_dot_and_add_32(short *, short *, short *);
extern int unaligned_dot_and_sub_32(short *, short *, short *);
extern int aligned_dot_and_add_32(short *, short *, short *);
extern int aligned_dot_and_sub_32(short *, short *, short *);

extern int unaligned_dot_and_add_64(short *, short *, short *);
extern int unaligned_dot_and_sub_64(short *, short *, short *);
extern int aligned_dot_and_add_64(short *, short *, short *);
extern int aligned_dot_and_sub_64(short *, short *, short *);

extern int unaligned_dot_and_add_256(short *, short *, short *);
extern int unaligned_dot_and_sub_256(short *, short *, short *);
extern int aligned_dot_and_add_256(short *, short *, short *);
extern int aligned_dot_and_sub_256(short *, short *, short *);

extern int unaligned_dot_and_add_1280(short *, short *, short *);
extern int unaligned_dot_and_sub_1280(short *, short *, short *);
extern int aligned_dot_and_add_1280(short *, short *, short *);
extern int aligned_dot_and_sub_1280(short *, short *, short *);

extern int dot_16by16_arm9m(short *, short *, unsigned);
#if defined(__ARMv6)
extern int dot_aligned_16by16_arm9m(short *, short *, unsigned);
#endif

#ifdef ___CPU_ARM9E___
    #if FILTER_LEN == 16
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add_16(a,b,c)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub_16(a,b,c)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add_16(a,b,c)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub_16(a,b,c)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
        #if APE_24BIT_SUPPORT
        #define vector_add(a,b)  vector_add_16(a,b)
        #define vector_sub(a,b)  vector_sub_16(a,b)
        #define scalarproduct(a,b) scalarproduct_16(a,b)
        #endif
    #elif FILTER_LEN == 32
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add_32(a,b,c)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub_32(a,b,c)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add_32(a,b,c)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub_32(a,b,c)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
        #if APE_24BIT_SUPPORT
        #define vector_add(a,b)  vector_add_32(a,b)
        #define vector_sub(a,b)  vector_sub_32(a,b)
        #define scalarproduct(a,b) scalarproduct_32(a,b)
        #endif
    #elif FILTER_LEN == 64
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add_64(a,b,c)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub_64(a,b,c)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add_64(a,b,c)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub_64(a,b,c)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
        #if APE_24BIT_SUPPORT
        #define vector_add(a,b)  vector_add_64(a,b)
        #define vector_sub(a,b)  vector_sub_64(a,b)
        #define scalarproduct(a,b) scalarproduct_64(a,b)
        #endif
    #elif FILTER_LEN == 256
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add_256(a,b,c)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub_256(a,b,c)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add_256(a,b,c)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub_256(a,b,c)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
        #if APE_24BIT_SUPPORT
        #define vector_add(a,b)  vector_add_256(a,b)
        #define vector_sub(a,b)  vector_sub_256(a,b)
        #define scalarproduct(a,b) scalarproduct_256(a,b)
        #endif
    #elif FILTER_LEN == 1280
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add_1280(a,b,c)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub_1280(a,b,c)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add_1280(a,b,c)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub_1280(a,b,c)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
        #if APE_24BIT_SUPPORT
        #define vector_add(a,b)  vector_add_1280(a,b)
        #define vector_sub(a,b)  vector_sub_1280(a,b)
        #define scalarproduct(a,b) scalarproduct_1280(a,b)
        #endif
    #else
        #define DOT_AND_ADD_ALIGNED(a,b,c,d) aligned_dot_and_add(a,b,c,d)
        #define DOT_AND_SUB_ALIGNED(a,b,c,d) aligned_dot_and_sub(a,b,c,d)
        #define DOT_AND_ADD_UNALIGNED(a,b,c,d) unaligned_dot_and_add(a,b,c,d)
        #define DOT_AND_SUB_UNALIGNED(a,b,c,d) unaligned_dot_and_sub(a,b,c,d)
        #define DOT_PRODUCT(a,b,c) dot_16by16_arm9m(a,b,c)
    #endif
#else
    #define DOT_AND_ADD_ALIGNED(a,b,c,d) scalarproduct(a,b);vector_add(a,c)
    #define DOT_AND_SUB_ALIGNED(a,b,c,d) scalarproduct(a,b);vector_sub(a,c)
    #define DOT_AND_ADD_UNALIGNED(a,b,c,d) DOT_AND_ADD_ALIGNED(a,b,c,d)
    #define DOT_AND_SUB_UNALIGNED(a,b,c,d) DOT_AND_SUB_ALIGNED(a,b,c,d)
    #define DOT_PRODUCT(a,b,c) scalarproduct(a,b)
#endif
#if APE_24BIT_SUPPORT
extern int APEDEPTH;
#endif
static void do_ape_filter_apply_3980(struct filter_t* f,
                                 int32_t* data,
                                 int count)
{
    int res;
    filter_int* adaptcoeffs = f->coef_adapted;
    filter_int* coeffs = f->coef_original;
    filter_int* delay = f->delay;
    int avg = f->avg;


{
        #define MOVE_COEFFS_TO_HEAD\
        memmove(coeffs + FILTER_LEN, delay - (FILTER_LEN*2),(FILTER_LEN*2) * sizeof(filter_int));\
        adaptcoeffs = coeffs + FILTER_LEN*2;\
        delay = coeffs + FILTER_LEN*3;
}
    while (count--) {
        filter_int* delay_addr = delay - FILTER_LEN;

        #if APE_24BIT_SUPPORT
        #if APE_24BIT_GENERIC_FILTER_USE
        if (APEDEPTH == 24)
        {
            #if APE_DEBUG
            if(((864-count) < 20) && ((864-count) > 0)&&(g_count >= 0*864) && (g_count <= 1*864))
            {    int ii;
                for(ii = 0; ii<64;ii++)
                {
                    LOGD("[zhengwen before] [%d] coeffs[%d]=%x, delay[%d]=%x, adaptcoeffs[%d]=%x", FILTER_LEN, ii, coeffs[ii],ii, (delay - FILTER_LEN)[ii],ii, (adaptcoeffs - FILTER_LEN)[ii]);

                }
            }
            #endif
            res = scalarproduct(coeffs, delay_addr);
            if (LIKELY(*data != 0)) {
                if (*data < 0)
                    vector_add(coeffs, adaptcoeffs - FILTER_LEN);
                else
                    vector_sub(coeffs, adaptcoeffs - FILTER_LEN);
            }
            #if APE_DEBUG
            if(((864-count) < 20) && ((864-count) > 0)&&(g_count >= 0*864) && (g_count <= 1*864))
            {    int ii;
                for(ii = 0; ii<64;ii++)
                {
                    LOGD("[zhengwen after] [%d] coeffs[%d]=%x, adaptcoeffs[%d]=%x", FILTER_LEN, ii, coeffs[ii],ii,(adaptcoeffs - FILTER_LEN)[ii]);
                }
            }
            #endif
        }
        else
        #endif
        #endif
        {
        if ((int)delay_addr & 0x3) {//unaligned
            if (*data < 0) {
                res = DOT_AND_ADD_UNALIGNED(coeffs,delay_addr,adaptcoeffs - FILTER_LEN,FILTER_LEN);
                }
            else if (*data > 0) {
                res = DOT_AND_SUB_UNALIGNED(coeffs,delay_addr,adaptcoeffs - FILTER_LEN,FILTER_LEN);
                }
            else {
                res = DOT_PRODUCT(coeffs,delay_addr,FILTER_LEN);
                }
            }
        else {
            if (*data < 0) {
                res = DOT_AND_ADD_ALIGNED(coeffs,delay_addr,adaptcoeffs - FILTER_LEN,FILTER_LEN);
                }
            else if (*data > 0) {
                res = DOT_AND_SUB_ALIGNED(coeffs,delay_addr,adaptcoeffs - FILTER_LEN,FILTER_LEN);
                }
            else {
#if defined(__ARMv6)
                res = dot_aligned_16by16_arm9m(coeffs,delay_addr,FILTER_LEN);
#else
                res = DOT_PRODUCT(coeffs,delay_addr,FILTER_LEN);
#endif
                }
            }
        }
        {
        int t, tt;
        int ttt, tttt;//zhengwen
        t = adaptcoeffs[-1];
        tt = adaptcoeffs[-2];
        adaptcoeffs[-1] = t>>1;
        adaptcoeffs[-2] = tt>>1;
        t = adaptcoeffs[-8];
        tt = *data;
        adaptcoeffs[-8] = t>>1;
        ttt = res;
        res = FRAC_TO_INT(res) + tt;
        tttt = res;
        *data++ = res;
        *delay++ = SATURATE(res);
        #if APE_DEBUG
        if(((864-count) < 20) && ((864-count) > 0)&&(g_count >= 0*864) && (g_count <= 1*864))
        {
            LOGD("[zhengwen]input = %x, DotProduct[%d] = %x, output[%d]=%x", tt, (864-count), ttt,(864-count), tttt);
        }
        #endif
        }
        if (res > 0) {
            if (3 * res <= 4 * avg) {
                *adaptcoeffs++ = -8;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            else if (res <= 3 * avg) {
                *adaptcoeffs++ = -16;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            else {
                *adaptcoeffs++ = -32;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            }
        else if (res < 0) {
            res = -res;
            if (3 * res <= 4 * avg) {
                *adaptcoeffs++ = 8;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            else if (res <= 3 * avg) {
                *adaptcoeffs++ = 16;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            else {
                *adaptcoeffs++ = 32;
                avg += (res - avg) / 16;
                if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
                }
            }
        else {
            *adaptcoeffs++ = 0;
            avg += (-avg) / 16;
            if (delay == f->history_end) { MOVE_COEFFS_TO_HEAD }
            }
        }

    f->coef_adapted = adaptcoeffs;
    f->coef_original = coeffs;//zhengwen add
    f->delay = delay;
    f->avg = avg;
}

static void do_ape_filter_apply_3970(struct filter_t* f,
                                 int32_t* data,
                                 int count)
{
    int res;

#ifdef PREPARE_SCALARPRODUCT
    PREPARE_SCALARPRODUCT
#endif

    while(LIKELY(count--)) {
        filter_int* delay_addr = f->delay - FILTER_LEN;

        #if APE_24BIT_SUPPORT
        #if APE_24BIT_GENERIC_FILTER_USE
        if (APEDEPTH == 24)
        {
            res = scalarproduct(f->coef_original,delay_addr);
            if(*data != 0)
            {
                if(*data < 0)
                    vector_add(f->coef_original,f->coef_adapted - FILTER_LEN);
                else
                    vector_sub(f->coef_original,f->coef_adapted - FILTER_LEN);
            }
        }
        else
        #endif
        #endif
        if ((int)delay_addr & 0x3) {//unaligned
            if (*data < 0) {
                res = DOT_AND_ADD_UNALIGNED(f->coef_original,delay_addr,f->coef_adapted - FILTER_LEN,FILTER_LEN);
                }
            else if (*data > 0) {
                res = DOT_AND_SUB_UNALIGNED(f->coef_original,delay_addr,f->coef_adapted - FILTER_LEN,FILTER_LEN);
                }
            else {
                res = DOT_PRODUCT(f->coef_original,delay_addr,FILTER_LEN);
                }
            }
        else {
            if (*data < 0) {
                res = DOT_AND_ADD_ALIGNED(f->coef_original,delay_addr,f->coef_adapted - FILTER_LEN,FILTER_LEN);
                }
            else if (*data > 0) {
                res = DOT_AND_SUB_ALIGNED(f->coef_original,delay_addr,f->coef_adapted - FILTER_LEN,FILTER_LEN);
                }
            else {
                res = DOT_PRODUCT(f->coef_original,delay_addr,FILTER_LEN);
                }
            }

        /* Convert res from (32-QBITS).QBITS fixed-point format to an
           integer (rounding to nearest) and add the input value to
           it */
        res = FRAC_TO_INT(res);

        res += *data;

        *data++ = res;

        /* Update the output history */
        *f->delay++ = SATURATE(res);

        /* Version ??? to < 3.98 files (untested) */
        f->coef_adapted[0] = (res == 0) ? 0 : ((res >> 28) & 8) - 4;
        f->coef_adapted[-4] >>= 1;
        f->coef_adapted[-8] >>= 1;

        f->coef_adapted++;

        /* Have we filled the history buffer? */
        if (UNLIKELY(f->delay == f->history_end)) {
            memmove(f->coef_original + FILTER_LEN, f->delay - (FILTER_LEN*2),
                    (FILTER_LEN*2) * sizeof(filter_int));
            f->coef_adapted = f->coef_original + FILTER_LEN*2;
            f->delay = f->coef_original + FILTER_LEN*3;
            }
        }
}

static void do_ape_filter_init(struct filter_t* f,
                           filter_int* buf)
{
    f->coef_original = buf;
    f->history_end = buf + FILTER_LEN*3 + FILTER_HISTORY_SIZE;

    /* Init pointers */
    f->coef_adapted = f->coef_original + FILTER_LEN*2;
    f->delay = f->coef_original + FILTER_LEN*3;

    /* Zero coefficients and history buffer */
    memset(f->coef_original, 0, FILTER_LEN*3 * sizeof(filter_int));

    /* Zero the running average */
    f->avg = 0;
}

void ape_filter_init(struct filter_t* f0,
                 struct filter_t* f1,
                 filter_int* buf)
{
    do_ape_filter_init(f0, buf);
    do_ape_filter_init(f1, buf + FILTER_LEN*3 + FILTER_HISTORY_SIZE);
}

void ape_filter_apply(int fileversion,
                  struct filter_t *f0,
                  struct filter_t *f1,
                  int32_t* data0,
                  int32_t* data1,
                  int count)
{
    if (fileversion >= 3980) {
        do_ape_filter_apply_3980(f0, data0, count);
        if (data1 != NULL)
            do_ape_filter_apply_3980(f1, data1, count);
        }
    else {
        do_ape_filter_apply_3970(f0, data0, count);
        if (data1 != NULL)
            do_ape_filter_apply_3970(f1, data1, count);
        }
}
