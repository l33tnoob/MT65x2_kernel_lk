/*
 * LZ4K Offline encoder matcher by Vovo
 */

#ifndef __LZ4K_MATCH_H__
#define __LZ4K_MATCH_H__

#include <stddef.h>

#ifndef __arm__
#define PAGESIZE 4096
#endif

#define USE_UBIFS

#ifdef USE_UBIFS
#define BEST_MATCH
#define PREVIOUS_MATCH_OFF
#define NO_LITERAL_CHAR_ENTROPY
#define ALLOW_MATCH_2
#define SAVING_OFFSET
#endif // USE_UBIFS

#ifdef ALLOW_MATCH_2
#define MIN_MATCH 2
#else
#define MIN_MATCH 3
#endif

void _lz4k_do_match(const unsigned char *in, int in_len, int best_match_len[PAGESIZE], int best_match_off[PAGESIZE]);

#define DECLARE_MATCHER_DATA() \
    int best_match_len[PAGESIZE]; \
    int best_match_off[PAGESIZE]; \
    _lz4k_do_match(in, in_len, best_match_len, best_match_off);

#ifndef BEST_MATCH
#define DO_LAZY_MATCH_1() \
        { \
            int index = ip - in; \
            if ( (best_match_len[index] < MIN_MATCH) || (best_match_len[index + 1] > (best_match_len[index] + 1) ) ) { \
                ++ip; \
                if (__builtin_expect(!!(ip >= ip_end), 0)) \
                    break; \
                continue; \
            } \
            m_pos = ip - best_match_off[index]; \
        }

#else // BEST_MATCH

#define DO_LAZY_MATCH_1() \
        { \
            int index = ip - in; \
            if (best_match_off[index] == 0) { \
                ip += best_match_len[index]; \
                if (__builtin_expect(!!(ip >= ip_end), 0)) \
                    break; \
                continue; \
            } \
            m_pos = ip - best_match_off[index]; \
        }

#endif // BEST_MATCH

#define DO_LAZY_MATCH_2() \
        { \
            int index = ip - in; \
            m_pos += best_match_len[index]; \
            ip += best_match_len[index]; \
        }

#endif // __LZ4K_MATCH_H__

