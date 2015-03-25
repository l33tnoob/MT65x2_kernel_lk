#include <string.h>
#include "entropy.h"
#include "entropy_optz_macro.h"
#include <utils/Log.h>

#define MODEL_ELEMENTS 64

static const int32_t counts_3970[MODEL_ELEMENTS + 1]  =
{
    0x0, 0x39e8, 0x6e40, 0x99b4, 0xbaef, 0xd2ea, 0xe33b, 0xedfe,
    0xf4da, 0xf92a, 0xfbcf, 0xfd6e, 0xfe66, 0xfefc, 0xff55, 0xff8b,
    0xffaa, 0xffbd, 0xffc8, 0xffcf, 0xffd3, 0xffd5, 0xffd6, 0xffd7,
    0xffd8, 0xffd9, 0xffda, 0xffdb, 0xffdc, 0xffdd, 0xffde, 0xffdf,
    0xffe0, 0xffe1, 0xffe2, 0xffe3, 0xffe4, 0xffe5, 0xffe6, 0xffe7,
    0xffe8, 0xffe9, 0xffea, 0xffeb, 0xffec, 0xffed, 0xffee, 0xffef,
    0xfff0, 0xfff1, 0xfff2, 0xfff3, 0xfff4, 0xfff5, 0xfff6, 0xfff7,
    0xfff8, 0xfff9, 0xfffa, 0xfffb, 0xfffc, 0xfffd, 0xfffe, 0xffff,
    0x10000,
};

static const int32_t counts_diff_3970[MODEL_ELEMENTS]  =
{
    0x39e8, 0x3458, 0x2b74, 0x213b, 0x17fb, 0x1051, 0xac3, 0x6dc,
    0x450, 0x2a5, 0x19f, 0xf8, 0x96, 0x59, 0x36, 0x1f,
    0x13, 0xb, 0x7, 0x4, 0x2, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
};

static const uint32_t counts_3980[MODEL_ELEMENTS + 1]  =
{
    0x0, 0x4c7a, 0x8d40, 0xbd21, 0xdc03, 0xede3, 0xf721, 0xfbb3,
    0xfdcb, 0xfed0, 0xff47, 0xff88, 0xffa7, 0xffba, 0xffc4, 0xffca,
    0xffcd, 0xffd0, 0xffd2, 0xffd3, 0xffd4, 0xffd5, 0xffd6, 0xffd7,
    0xffd8, 0xffd9, 0xffda, 0xffdb, 0xffdc, 0xffdd, 0xffde, 0xffdf,
    0xffe0, 0xffe1, 0xffe2, 0xffe3, 0xffe4, 0xffe5, 0xffe6, 0xffe7,
    0xffe8, 0xffe9, 0xffea, 0xffeb, 0xffec, 0xffed, 0xffee, 0xffef,
    0xfff0, 0xfff1, 0xfff2, 0xfff3, 0xfff4, 0xfff5, 0xfff6, 0xfff7,
    0xfff8, 0xfff9, 0xfffa, 0xfffb, 0xfffc, 0xfffd, 0xfffe, 0xffff,
    0x10000,
};

static const uint32_t counts_diff_3980[MODEL_ELEMENTS]  =
{
    0x4c7a, 0x40c6, 0x2fe1, 0x1ee2, 0x11e0, 0x93e, 0x492, 0x218,
    0x105, 0x77, 0x41, 0x1f, 0x13, 0xa, 0x6, 0x3,
    0x3, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
};

static const uint32_t LIBAPE_K_SUM_MIN_BOUNDARY[MODEL_ELEMENTS>>1] =
{
    0x0, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x7fffffff, 0x0, 0x0, 0x0, 0x0,
};

libdemac_inline void skip_byte(struct ape_dec_internal_t* ape_dec_internal)
{
    ape_dec_internal->bytebufferoffset--;
    ape_dec_internal->bytebuffer += ape_dec_internal->bytebufferoffset & 4;
    ape_dec_internal->bytebufferoffset &= 3;
}

libdemac_inline int read_byte(struct ape_dec_internal_t* ape_dec_internal)
{
    int ch = ape_dec_internal->bytebuffer[ape_dec_internal->bytebufferoffset];

    skip_byte(ape_dec_internal);

    return ch;
}

#define CODE_BITS 32
#define TOP_VALUE ((unsigned int)1 << (CODE_BITS-1))
#define SHIFT_BITS (CODE_BITS - 9)
#define EXTRA_BITS ((CODE_BITS-2) % 8 + 1)
#define BOTTOM_VALUE (TOP_VALUE >> 8)

libdemac_inline void range_start_decoding(struct ape_dec_internal_t *ape_dec_internal)
{
    ape_dec_internal->rc.buffer = read_byte(ape_dec_internal);
    ape_dec_internal->rc.low = ape_dec_internal->rc.buffer >> (8 - EXTRA_BITS);
    ape_dec_internal->rc.range = (uint32_t) 1 << EXTRA_BITS;
}

libdemac_inline void range_dec_normalize(struct ape_dec_internal_t *ape_dec_internal)
{
    while (ape_dec_internal->rc.range <= BOTTOM_VALUE) {
        ape_dec_internal->rc.buffer = (ape_dec_internal->rc.buffer << 8) | read_byte(ape_dec_internal);
        ape_dec_internal->rc.low = (ape_dec_internal->rc.low << 8) | ((ape_dec_internal->rc.buffer >> 1) & 0xff);
        ape_dec_internal->rc.range <<= 8;
        }
}

#if 0
libdemac_inline int range_decode_culfreq(struct ape_dec_internal_t *ape_dec_internal, int tot_f)
{
    range_dec_normalize(ape_dec_internal);
    ape_dec_internal->rc.help = UDIV32(ape_dec_internal->rc.range, tot_f);
    return UDIV32(ape_dec_internal->rc.low, ape_dec_internal->rc.help);
}

libdemac_inline int range_decode_culshift(struct ape_dec_internal_t *ape_dec_internal, int shift)
{
    range_dec_normalize(ape_dec_internal);
    ape_dec_internal->rc.help = ape_dec_internal->rc.range >> shift;
    return UDIV32(ape_dec_internal->rc.low, ape_dec_internal->rc.help);
}

libdemac_inline void range_decode_update(struct ape_dec_internal_t *ape_dec_internal, int sy_f, int lt_f)
{
    ape_dec_internal->rc.low -= ape_dec_internal->rc.help * lt_f;
    ape_dec_internal->rc.range = ape_dec_internal->rc.help * sy_f;
}

libdemac_inline unsigned char decode_byte(struct ape_dec_internal_t *ape_dec_internal)
{   int tmp = range_decode_culshift(ape_dec_internal, 8);
    range_decode_update(ape_dec_internal, 1, tmp);
    return tmp;
}

libdemac_inline int short range_decode_short(struct ape_dec_internal_t *ape_dec_internal)
{   int tmp = range_decode_culshift(ape_dec_internal, 16);
    range_decode_update(ape_dec_internal, 1, tmp);
    return tmp;
}

libdemac_inline int range_decode_bits(struct ape_dec_internal_t *ape_dec_internal, int n)
{   int tmp = range_decode_culshift(ape_dec_internal, n);
    range_decode_update(ape_dec_internal, 1, tmp);
    return tmp;
}
#endif

libdemac_inline void range_done_decoding(struct ape_dec_internal_t *ape_dec_internal)
{   range_dec_normalize(ape_dec_internal);      /* normalize to use up all bytes */
}

#if 0
libdemac_inline int range_get_symbol_3970(struct ape_dec_internal_t *ape_dec_internal)
{
    int symbol, cf;

    cf = range_decode_culshift(ape_dec_internal, 16);

    /* figure out the symbol inefficiently; a binary search would be much better */
    for (symbol = 0; counts_3970[symbol+1] <= cf; symbol++);

    range_decode_update(ape_dec_internal, counts_diff_3970[symbol],counts_3970[symbol]);

    return symbol;
}
#endif

libdemac_inline void update_rice(struct rice_t* rice, int x)
{
    rice->ksum += ((x + 1) / 2) - ((rice->ksum + 16) >> 5);

    if (UNLIKELY(rice->ksum < LIBAPE_K_SUM_MIN_BOUNDARY[rice->k])) {
        rice->k--;
        }
    else if (UNLIKELY(rice->ksum >= LIBAPE_K_SUM_MIN_BOUNDARY[rice->k + 1])) {
        rice->k++;
        }
}

libdemac_inline int entropy_decode3970(struct ape_dec_internal_t *ape_dec_internal, struct rice_t* rice)
{
    int x, tmpk;
    int overflow;
    unsigned int rc_range, rc_low, rc_buffer, rc_help;

    rc_buffer = ape_dec_internal->rc.buffer;
    rc_low = ape_dec_internal->rc.low;
    rc_range = ape_dec_internal->rc.range;
    rc_help = ape_dec_internal->rc.help;

    RANGE_CODER_NORMALIZE_3970
    RANGE_CODER_SEARCH_SYMBOL_AND_UPDATE_3970(overflow)

    if (UNLIKELY(overflow == (MODEL_ELEMENTS - 1))) {
        RANGE_CODER_NORMALIZE_3970
        rc_help = rc_range >> 5;
        tmpk = UDIV32(rc_low, rc_help);
        RANGE_CODER_UPDATE(tmpk, 1)
        overflow = 0;
        }
    else { tmpk = (rice->k < 1) ? 0 : rice->k - 1; }

    if (tmpk <= 16) {
        RANGE_CODER_NORMALIZE_3970
        rc_help = rc_range >> tmpk;
        x = UDIV32(rc_low, rc_help);
        RANGE_CODER_UPDATE(x, 1)
        }
    else {
        int xx;
        RANGE_CODER_NORMALIZE_3970
        rc_help = rc_range >> 16;
        x = UDIV32(rc_low, rc_help);
        RANGE_CODER_UPDATE(x, 1)
        RANGE_CODER_NORMALIZE_3970
        rc_help = rc_range >> (tmpk - 16);
        xx = UDIV32(rc_low, rc_help);
        RANGE_CODER_UPDATE(xx, 1)

        x |= xx << 16;
        }
    x += (overflow << tmpk);

    update_rice(rice, x);

    ape_dec_internal->rc.buffer = rc_buffer;
    ape_dec_internal->rc.low = rc_low;
    ape_dec_internal->rc.range = rc_range;
    ape_dec_internal->rc.help = rc_help;

    if (x & 1)
        return (x >> 1) + 1;
    else
        return -(x >> 1);
}

STATIC_DECLARE void init_entropy_decoder(struct ape_dec_internal_t* ape_dec_internal,
                          unsigned char* inbuffer,
                          int* firstbyte,
                          int* bytesconsumed)
{
    ape_dec_internal->bytebuffer = inbuffer;
    ape_dec_internal->bytebufferoffset = *firstbyte;

    /* Read the CRC */
    ape_dec_internal->CRC = read_byte(ape_dec_internal);
    ape_dec_internal->CRC = (ape_dec_internal->CRC << 8) | read_byte(ape_dec_internal);
    ape_dec_internal->CRC = (ape_dec_internal->CRC << 8) | read_byte(ape_dec_internal);
    ape_dec_internal->CRC = (ape_dec_internal->CRC << 8) | read_byte(ape_dec_internal);

    /* Read the frame flags if they exist */
    ape_dec_internal->entropy_flags = 0;
    if ((ape_dec_internal->fileversion > 3820) && (ape_dec_internal->CRC & 0x80000000)) {
        ape_dec_internal->CRC &= ~0x80000000;

        ape_dec_internal->entropy_flags = read_byte(ape_dec_internal);
        ape_dec_internal->entropy_flags = (ape_dec_internal->entropy_flags << 8) | read_byte(ape_dec_internal);
        ape_dec_internal->entropy_flags = (ape_dec_internal->entropy_flags << 8) | read_byte(ape_dec_internal);
        ape_dec_internal->entropy_flags = (ape_dec_internal->entropy_flags << 8) | read_byte(ape_dec_internal);
        }
    /* Keep a count of the blocks decoded in this frame */
    ape_dec_internal->blocksdecoded = 0;

    /* Initialise the rice structs */
    ape_dec_internal->riceX.k = 10;
    ape_dec_internal->riceX.ksum = (1 << ape_dec_internal->riceX.k) * 16;
    ape_dec_internal->riceY.k = 10;
    ape_dec_internal->riceY.ksum = (1 << ape_dec_internal->riceY.k) * 16;

    /* The first 8 bits of input are ignored. */
    skip_byte(ape_dec_internal);

    range_start_decoding(ape_dec_internal);

    /* Return the new state of the buffer */
    *bytesconsumed = (intptr_t)ape_dec_internal->bytebuffer - (intptr_t)inbuffer;
    *firstbyte = ape_dec_internal->bytebufferoffset;
}

STATIC_DECLARE void entropy_decode(struct ape_dec_internal_t* ape_dec_internal,
                                     unsigned char* inbuffer,
                                     int* firstbyte,
                                     int* bytesconsumed,
                                     int32_t* decoded0,
                                     int32_t* decoded1,
                                     int blockstodecode)
{
    ape_dec_internal->bytebuffer = inbuffer;
    ape_dec_internal->bytebufferoffset = *firstbyte;
    ape_dec_internal->blocksdecoded += blockstodecode;

    if ((ape_dec_internal->entropy_flags & APE_FRAMECODE_LEFT_SILENCE) &&
        ((ape_dec_internal->entropy_flags & APE_FRAMECODE_RIGHT_SILENCE) || (decoded1 == NULL))) { /* pure silence */
        memset(decoded0, 0, blockstodecode * sizeof(int32_t));
        if (decoded1 != NULL) { memset(decoded1, 0, blockstodecode * sizeof(int32_t)); }
        }
    else {
        if (ape_dec_internal->fileversion > 3980) {
            unsigned int rc_range, rc_low, rc_buffer, rc_help;
            unsigned int rice_y_k, rice_y_ksum, rice_x_k, rice_x_ksum;
            rc_buffer = ape_dec_internal->rc.buffer;
            rc_low = ape_dec_internal->rc.low;
            rc_range = ape_dec_internal->rc.range;
            rc_help = ape_dec_internal->rc.help;
            rice_y_k = ape_dec_internal->riceY.k;
            rice_y_ksum = ape_dec_internal->riceY.ksum;
            rice_x_k = ape_dec_internal->riceX.k;
            rice_x_ksum = ape_dec_internal->riceX.ksum;

            while (LIKELY(blockstodecode--)) {
                int pivot, overflow, temp;
                RANGE_CODER_NORMALIZE
                RANGE_CODER_SEARCH_SYMBOL_AND_UPDATE(overflow)
                if (UNLIKELY(overflow == (MODEL_ELEMENTS-1))) {
                    RANGE_CODER_NORMALIZE
                    rc_help = rc_range >> 16;
                    temp = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(temp, 1)
                    overflow = temp << 16;
                    RANGE_CODER_NORMALIZE
                    rc_help = rc_range >> 16;
                    temp = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(temp, 1)
                    overflow |= temp;
                    }

                pivot = rice_y_ksum >> 5;
                if (UNLIKELY(pivot == 0)) { pivot=1; }

                //if (pivot < 0x100000)
                if(pivot < 0x10000)//zhengwen modified
                    { /* 16-bit */
                    int base, x;
                    RANGE_CODER_NORMALIZE
                    DIV_LOOKUP(rc_range, pivot, rc_help);
                    base = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(base, 1)
                    x = base + (overflow * pivot);
                    rice_y_ksum += ((x + 1) / 2) - ((rice_y_ksum + 16) >> 5);
                    if (x & 1) { *(decoded0++) = (x >> 1) + 1; }
                    else { *(decoded0++) = -(x >> 1); }
                    if (UNLIKELY(rice_y_k == 0)) { rice_y_k = 1; }
                    else {
                        uint32_t lim = 1 << (rice_y_k + 4);
                        if (UNLIKELY(rice_y_ksum < lim)) { rice_y_k--; }
                        else if (UNLIKELY(rice_y_ksum >= 2 * lim)) { rice_y_k++; }
                        }
                    }
                else { /* 24-bit, currently not support  */
      #if APE_24BIT_SUPPORT
                    int base, x;
                    int nbits, lo_bits, base_hi, base_lo;

                    nbits = 17;
                    while((pivot >> nbits) > 0){nbits++;}

                    lo_bits = (nbits - 16);

                    RANGE_CODER_NORMALIZE
                    DIV_LOOKUP(rc_range, ((pivot >> lo_bits) + 1), rc_help);
                    base_hi = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(base_hi, 1)

                    RANGE_CODER_NORMALIZE
                    rc_help = rc_range >> lo_bits;
                    base_lo = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(base_lo, 1);

                    base = (base_hi << lo_bits) + base_lo;

                    x = base + (overflow * pivot);
                    rice_y_ksum += ((x + 1) / 2) - ((rice_y_ksum + 16) >> 5);
                    if (x & 1) { *(decoded0++) = (x >> 1) + 1; }
                    else { *(decoded0++) = -(x >> 1); }
                    if (UNLIKELY(rice_y_k == 0)) { rice_y_k = 1; }
                    else {
                        uint32_t lim = 1 << (rice_y_k + 4);
                        if (UNLIKELY(rice_y_ksum < lim)) { rice_y_k--; }
                        else if (UNLIKELY(rice_y_ksum >= 2 * lim)) { rice_y_k++; }
                        }
     #endif
                    }

                if (decoded1 != NULL) {
                    RANGE_CODER_NORMALIZE
                    RANGE_CODER_SEARCH_SYMBOL_AND_UPDATE(overflow)
                    if (UNLIKELY(overflow == (MODEL_ELEMENTS-1))) {
                        RANGE_CODER_NORMALIZE
                        rc_help = rc_range >> 16;
                        temp = UDIV32(rc_low, rc_help);
                        RANGE_CODER_UPDATE(temp, 1)
                        overflow = temp << 16;
                        RANGE_CODER_NORMALIZE
                        rc_help = rc_range >> 16;
                        temp = UDIV32(rc_low, rc_help);
                        RANGE_CODER_UPDATE(temp, 1)
                        overflow |= temp;
                        }

                    pivot = rice_x_ksum >> 5;
                    if (UNLIKELY(pivot == 0)) { pivot=1; }

                //if (pivot < 0x100000)
                if(pivot < 0x10000)//zhengwen modified
                        { /* 16-bit */
                        int base, x;
                        RANGE_CODER_NORMALIZE
                        DIV_LOOKUP(rc_range, pivot, rc_help);
                        base = UDIV32(rc_low, rc_help);
                        RANGE_CODER_UPDATE(base, 1)
                        x = base + (overflow * pivot);
                        rice_x_ksum += ((x + 1) / 2) - ((rice_x_ksum + 16) >> 5);
                        if (x & 1) { *(decoded1++) = (x >> 1) + 1; }
                        else { *(decoded1++) = -(x >> 1); }
                        if (UNLIKELY(rice_x_k == 0)) { rice_x_k = 1; }
                        else {
                            uint32_t lim = 1 << (rice_x_k + 4);
                            if (UNLIKELY(rice_x_ksum < lim)) { rice_x_k--; }
                            else if (UNLIKELY(rice_x_ksum >= 2 * lim)) { rice_x_k++; }
                            }
                        }
                    else { /* 24-bit, currently not support */
      #if APE_24BIT_SUPPORT
                    int base, x;
                    int nbits, lo_bits, base_hi, base_lo;

                    nbits = 17;
                    while((pivot >> nbits) > 0){nbits++;}

                    lo_bits = (nbits - 16);

                    RANGE_CODER_NORMALIZE
                    DIV_LOOKUP(rc_range, ((pivot >> lo_bits) + 1), rc_help);
                    base_hi = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(base_hi, 1)

                    RANGE_CODER_NORMALIZE
                    rc_help = rc_range >> lo_bits;
                    base_lo = UDIV32(rc_low, rc_help);
                    RANGE_CODER_UPDATE(base_lo, 1);

                    base = (base_hi << lo_bits) + base_lo;

                    x = base + (overflow * pivot);
                    rice_x_ksum += ((x + 1) / 2) - ((rice_x_ksum + 16) >> 5);
                    if (x & 1) { *(decoded1++) = (x >> 1) + 1; }
                    else { *(decoded1++) = -(x >> 1); }
                    if (UNLIKELY(rice_x_k == 0)) { rice_x_k = 1; }
                    else {
                        uint32_t lim = 1 << (rice_x_k + 4);
                        if (UNLIKELY(rice_x_ksum < lim)) { rice_x_k--; }
                        else if (UNLIKELY(rice_x_ksum >= 2 * lim)) { rice_x_k++; }
                        }
     #endif
                        }
                    }
                }

            ape_dec_internal->rc.buffer = rc_buffer;
            ape_dec_internal->rc.low = rc_low;
            ape_dec_internal->rc.range = rc_range;
            ape_dec_internal->rc.help = rc_help;
            ape_dec_internal->riceY.k = rice_y_k;
            ape_dec_internal->riceY.ksum = rice_y_ksum;
            ape_dec_internal->riceX.k = rice_x_k;
            ape_dec_internal->riceX.ksum = rice_x_ksum;
            }
        else {
            while (LIKELY(blockstodecode--)) {
                *(decoded0++) = entropy_decode3970(ape_dec_internal, &ape_dec_internal->riceY);
                if (decoded1 != NULL) {
                    *(decoded1++) = entropy_decode3970(ape_dec_internal, &ape_dec_internal->riceX);
                    }
                }
            }
        }

    if (ape_dec_internal->blocksdecoded == ape_dec_internal->currentframeblocks) {
        range_done_decoding(ape_dec_internal);
        }

    /* Return the new state of the buffer */
    *bytesconsumed = ape_dec_internal->bytebuffer - inbuffer;
    *firstbyte = ape_dec_internal->bytebufferoffset;
}
