/*
 * LZ4K Offline Encoder Matcher by Vovo
 */

#include "lz4k_matcher.h"

#include "divsufsort.h"

#ifndef __arm__
#include <assert.h>
#include <stdio.h>
#define PAGESIZE 4096
#endif

#define LZ4K_DEBUG_assert(x)
//#define LZ4K_DEBUG_assert assert

extern unsigned short lz4k_matchlen_encode[32];
extern unsigned short lz4k_matchoff_encode[32];
extern unsigned short lz4k_literallen_encode[18];
extern unsigned short lz4k_literalch_encode[256];

#ifdef NO_LITERAL_CHAR_ENTROPY
inline int get_literal_bits(unsigned char ch) {
    return 8;
}
#else
inline int get_literal_bits(unsigned char ch) {
    int value = lz4k_literalch_encode[ch];
    int bits = value >> 9;
    LZ4K_DEBUG_assert(bits <= 9);
    return bits;
}
#endif

inline int get_literal_len_bits(int literal_len) {
    LZ4K_DEBUG_assert(literal_len != 0);
    LZ4K_DEBUG_assert(literal_len <= 4096);
    if (literal_len == 1) {
        return 1;
    } else if (literal_len == 2) {
        return 3;
    } else if (literal_len <= 17) {
        int value = lz4k_literallen_encode[literal_len];
        return value >> 7;
    } else {
        return 17;
    }
}

inline int get_match_len_bits(int match_len) {
    LZ4K_DEBUG_assert(match_len >= 3);
    LZ4K_DEBUG_assert(match_len < 4096);
#ifdef ALLOW_MATCH_2
    if (match_len <= 20 || match_len == 24 || match_len == 28) {
        int value = lz4k_matchlen_encode[match_len];
        return value >> 8;
    } else {
        return 17;
    }
#else
    if (match_len < 32) {
        int value = lz4k_matchlen_encode[match_len];
        return value >> 9;
    } else {
        return 16;
    }
#endif
}

//unsigned short lz4k_matchoff_bits[129] = {13, 8, 13, 13, 5, 13, 8, 13, 6, 13, 8, 13, 6, 13, 8, 13, 6, 13, 8, 13, 7, 13, 8, 13, 6, 13, 8, 13, 7, 13, 8, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 8, 13, 13, 13, 7, 13, 13, 13, 8, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8};
unsigned short lz4k_matchoff_bits[133] = {13, 7, 8, 13, 5, 13, 7, 13, 5, 13, 7, 13, 6, 13, 7, 13, 5, 13, 7, 13, 6, 13, 7, 13, 6, 13, 7, 13, 7, 13, 7, 13, 7, 13, 8, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8, 13, 13, 13, 8};

inline int get_match_off_bits(int match_off, int index) {
    int bits_1 = 13;
    int bits_2 = 0;
    LZ4K_DEBUG_assert(match_off >= 1);
    LZ4K_DEBUG_assert(match_off <= 4093);

#ifndef PREVIOUS_MATCH_OFF
    if ((match_off & 3) == 0 && match_off <= 128) {
        int value = lz4k_matchoff_encode[(match_off / 4) - 1];
        return value >> 8;
    } else {
        return 13;
    }
#else
    if (match_off <= 128) {
        bits_1 = lz4k_matchoff_bits[match_off];
    }
    bits_2 = 1 + __builtin_clz(index);
    if (bits_1 < bits_2) {
        return bits_1;
    } else {
        return bits_2;
    }
#endif
}

inline int can_extend(int previous_match_len) {
#define EXTEND_THRESHOLD 32
//#define EXTEND_THRESHOLD 256
//#define EXTEND_THRESHOLD 4096
    if (previous_match_len >= EXTEND_THRESHOLD) {
        return 1;
    } else {
        return 0;
    }
}

void _lz4k_do_match(const unsigned char *in, int in_len, int best_match_len[PAGESIZE], int best_match_off[PAGESIZE])
{
    int sa_sorted[PAGESIZE];
    int sa_reverse[PAGESIZE];
    int link_p[PAGESIZE];
    int link_n[PAGESIZE];
    int lcp_height[PAGESIZE + 1];
    int literal_accumulated[PAGESIZE + 1];
    int bit_accumulated[PAGESIZE + 1];
    int literal_bit_sum[PAGESIZE + 1];
    int i = 0;
    int h = 0;
    // suffix sort
    divsufsort(in, sa_sorted, in_len);
    // initial sa_reverse and others
    for (i = 0; i < in_len; ++i) {
        sa_reverse[sa_sorted[i]] = i;
    }
    literal_bit_sum[0] = 0;
    for (i = 0; i < PAGESIZE; ++i) {
        link_p[i] = 1;
        link_n[i] = 1;
        lcp_height[i] = -2;
        best_match_len[i] = -3;
        best_match_off[i] = -4;
        literal_accumulated[i] = -5;
        bit_accumulated[i] = -6;
        literal_bit_sum[i + 1] = literal_bit_sum[i] + get_literal_bits(in[i]);
    }

    // calculate lcp_height
    lcp_height[0] = 0;
    lcp_height[in_len] = 0;
    h = 0;
    for (i = 0; i < in_len; ++i) {
        if (sa_reverse[i] != 0) {
            int max_ij = i;
            int j = sa_sorted[sa_reverse[i] - 1];
            if (j > i)
                max_ij = j;
            while (in[i+h] == in[j+h] && (max_ij+h) < in_len) {
                ++h;
            }
            //assert(sa_reverse[i] < in_len && sa_reverse[i] >= 0);
            lcp_height[sa_reverse[i]] = h;
            if (h > 0)
                --h;
        }
    }

    // index 0
    best_match_len[0] = 0; // 0 -> literal
    best_match_off[0] = 0;
    literal_accumulated[in_len] = 0;
    bit_accumulated[in_len] = 0;
    // index in_len - 1, in_len - 2
    for (i = in_len - 1; i >= in_len - 1; --i) {
        int rank = sa_reverse[i];
        int previous = rank - link_p[rank];
        int next = rank + link_n[rank];
        best_match_off[i] = 0;
        best_match_len[i] = in_len - i;
        literal_accumulated[i] = literal_accumulated[i + 1] + 1;
        bit_accumulated[i] = bit_accumulated[i + 1] + get_literal_bits(in[i]);

        if (next < in_len) {
            link_p[next] += link_p[rank];
            if (lcp_height[rank] < lcp_height[next]) {
                lcp_height[next] = lcp_height[rank];
            }
        }
        if (previous >= 0) {
            link_n[previous] += link_n[rank];
            if (lcp_height[previous] < lcp_height[rank]) {
                lcp_height[rank] = lcp_height[previous];
            }
        }
    }

    for (i = in_len - 2; i >= 0; --i) {
        int rank = sa_reverse[i];
        int previous = rank - link_p[rank];
        int next = rank + link_n[rank];

#ifndef BEST_MATCH
        // 1. longest match
        best_match_len[i] = 0;
        best_match_off[i] = 0;
        if (lcp_height[next] >= 3) {
            best_match_len[i] = lcp_height[next];
            best_match_off[i] = i - sa_sorted[next];
        }
        if ( (lcp_height[rank] >= 3) && 
                ( (lcp_height[rank] > lcp_height[next]) || 
                  ((lcp_height[rank] == lcp_height[next]) && (sa_sorted[previous] > sa_sorted[next]) ) ) ) {
            best_match_len[i] = lcp_height[rank];
            best_match_off[i] = i - sa_sorted[previous];
        }
#else
        // 2. best match
        int max_height = 0;
        int j = 1;
        if (lcp_height[rank] > lcp_height[next]) {
            max_height = lcp_height[rank];
        } else {
            max_height = lcp_height[next];
        }
        // literal case
        j = literal_accumulated[i + 1] + 1;
        best_match_off[i] = 0;
        best_match_len[i] = j;
        bit_accumulated[i] = bit_accumulated[i + j] + (literal_bit_sum[i + j] - literal_bit_sum[i]) + get_literal_len_bits(j);
        if (max_height < MIN_MATCH) {
        //if (max_height < 4) {
//#define LITERAL_JUMP_LIMIT 18
//#define LITERAL_JUMP_LIMIT 4096
#define LITERAL_JUMP_LIMIT 100
            int jump_count = 0;
            while ( (jump_count <= LITERAL_JUMP_LIMIT) && (i + j) < in_len) {
                int new_bits = 0;
                ++j;
                j = j + literal_accumulated[i + j];
                new_bits = bit_accumulated[i + j] + (literal_bit_sum[i + j] - literal_bit_sum[i]) + get_literal_len_bits(j);
                if (0) {
                    printf("j: %d, new_bits: %d, bit_accumulated[i]: %d\n", j, new_bits, bit_accumulated[i]);
                }
                if (new_bits < bit_accumulated[i]) {
                    bit_accumulated[i] = new_bits;
                    best_match_len[i] = j;
                }
                ++jump_count;
            }
        }

        // check if can extend the match
        if ((best_match_off[i + 1] != 0) && (can_extend(best_match_len[i + 1])) && ((i - best_match_off[i + 1]) > 0) && 
                (in[i] == in[i - best_match_off[i + 1]]) && 
                ( (bit_accumulated[i + 1] - get_match_len_bits(best_match_len[i + 1]) + get_match_len_bits(best_match_len[i + 1] + 1)) < bit_accumulated[i])) {
            best_match_off[i] = best_match_off[i + 1];
            best_match_len[i] = best_match_len[i + 1] + 1;
            bit_accumulated[i] = bit_accumulated[i + 1] - get_match_len_bits(best_match_len[i + 1]) + get_match_len_bits(best_match_len[i + 1] + 1);
        } else if (max_height >= MIN_MATCH) {
            int previous_i = previous;
            int next_i = next;
            int current_match_len = max_height;
            int backward_height_min = lcp_height[rank];
            int forward_height_min = lcp_height[next];
            int current_best_offset = 0;
            int current_best_offset_bits = 1000;
            // literal case bits
            if (current_match_len > (in_len - i)) {
                current_match_len = in_len - i;
            }
            current_best_offset = 0;
            while (current_match_len >= MIN_MATCH) {
                int new_bits = 0;
                new_bits = bit_accumulated[i + current_match_len];
                LZ4K_DEBUG_assert((i + current_match_len) <= 4096);
                // search for best offset
                if (current_best_offset != 4) {
                    while (backward_height_min >= current_match_len) {
                        int pos = sa_sorted[previous_i];
                        int offset = i - pos;
                        int offset_bits = get_match_off_bits(offset, i);
                        if (offset_bits < current_best_offset_bits) {
                            current_best_offset = offset;
                            current_best_offset_bits = offset_bits;
                        }
                        if (lcp_height[previous_i] < backward_height_min) {
                            backward_height_min = lcp_height[previous_i];
                        }
                        previous_i = previous_i - link_p[previous_i];
                    }
                    while (forward_height_min >= current_match_len) {
                        int pos = sa_sorted[next_i];
                        int offset = i - pos;
                        int offset_bits = get_match_off_bits(offset, i);
                        if (offset_bits < current_best_offset_bits) {
                            current_best_offset = offset;
                            current_best_offset_bits = offset_bits;
                        }
                        next_i = next_i + link_n[next_i];
                        LZ4K_DEBUG_assert(next_i <= 4096);
                        LZ4K_DEBUG_assert(next_i >= 0);
                        if (lcp_height[next_i] < forward_height_min) {
                            forward_height_min = lcp_height[next_i];
                        }
                    }
                }
#if 0
                if (current_match_len == 2 && current_best_offset_bits == 13) {
                    break;
                }
#endif
                new_bits += current_best_offset_bits + get_match_len_bits(current_match_len) + 1; // 1 bit: type
                if (new_bits < bit_accumulated[i]) {
                    if (0) {
                        printf("bit_accumulated[i]: %d, new_bits: %d, current_best_offset_bits: %d\n", bit_accumulated[i], new_bits, current_best_offset_bits);
                        printf("get_match_len_bits(current_match_len): %d\n", get_match_len_bits(current_match_len) );
                        printf("bit_accumulated[i + current_match_len]: %d\n", bit_accumulated[i + current_match_len]);
                        printf("current_match_len: %d\n", current_match_len);
                    }
                    best_match_off[i] = current_best_offset;
                    best_match_len[i] = current_match_len;
                    bit_accumulated[i] = new_bits;
                }
                --current_match_len;
            }
        }

        if (best_match_off[i] != 0) {
            literal_accumulated[i] = 0;
            if (max_height != best_match_len[i]) {
                //printf("i: %d, max_height: %d, best_match_len[i]: %d, best_match_off[i]: %d\n", i, max_height, best_match_len[i], best_match_off[i]);
            }
        } else {
            literal_accumulated[i] = literal_accumulated[i + 1] + 1;
            if (max_height > 3) {
                //printf("i: %d, max_height 2: %d\n", i, max_height);
            }
        }
#endif

        if (next < in_len) {
            //assert(sa_sorted[previous] < i); // check link list integrity
            //assert(link_n[previous] == link_p[rank]);
            link_p[next] += link_p[rank];
            if (lcp_height[rank] < lcp_height[next]) {
                lcp_height[next] = lcp_height[rank];
            }
        }
        if (previous >= 0) {
            //assert(sa_sorted[next] < i); // check link list integrity
            //assert(link_p[next] == link_n[rank]);
            link_n[previous] += link_n[rank];
            if (lcp_height[previous] < lcp_height[rank]) {
                lcp_height[rank] = lcp_height[previous];
            }
        }
    }
#if 0
    for (i = 0; i < in_len + 1; ++i) {
    //for (i = 3500; i < in_len + 1; ++i) {
        printf("best match i: %d, rank: %d, len: %d, off: %d\n", i, sa_reverse[i], best_match_len[i], best_match_off[i]);
        printf("best 2, bits: %d, lits: %d\n", bit_accumulated[i], literal_accumulated[i]);
    }
#endif
    //assert(0);
}

