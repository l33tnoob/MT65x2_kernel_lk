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

#ifndef _DEMAC_CONFIG_H
#define _DEMAC_CONFIG_H

#ifdef UP_TO_L3
#define NO_SUPPORT_TO_4000
#define NO_SUPPORT_TO_5000
#endif

#ifdef UP_TO_L4
#define NO_SUPPORT_TO_5000
#endif

#ifdef __TARGET_FEATURE_DSPMUL
    #define ___CPU_ARM9E___
#elif defined(__GNUC__) && defined(__ANDROID__)
    #define ___CPU_ARM9E___
    #define __ARMv6
#endif

#define APE_OUTPUT_DEPTH (ape_dec_internal->bps)

#define IBSS_ATTR
#define IBSS_ATTR_DEMAC_INSANEBUF
#define ICONST_ATTR
#define ICODE_ATTR
#define ICODE_ATTR_DEMAC

#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)

#define APE_24BIT_SUPPORT 1
#define APE_24BIT_GENERIC_FILTER_USE 0
#define APE_DEBUG 0

#ifndef FILTER_HISTORY_SIZE
#define FILTER_HISTORY_SIZE 512
#endif

#ifndef PREDICTOR_HISTORY_SIZE
#define PREDICTOR_HISTORY_SIZE 512
#endif

#ifndef FILTER_BITS
#define FILTER_BITS 16
#endif

#define DIV_BY_ZERO_CHECK
#ifdef DIV_BY_ZERO_CHECK
    #define UDIV32(a, b) (b == 0 ? a : (a / b))
#else
    #define UDIV32(a, b) (a / b)
#endif

#if defined(__GNUC__) && defined(__ANDROID__)
    #include <stdint.h>
#else
    typedef unsigned int uint32_t;
    typedef unsigned short uint16_t;
    typedef unsigned char uint8_t;
    typedef signed int int32_t;
    typedef signed short int16_t;
    typedef signed char int8_t;
    typedef int intptr_t;
#endif

#if FILTER_BITS == 32
typedef int32_t filter_int;
#elif FILTER_BITS == 16
typedef int16_t filter_int;
#endif

#define libdemac_inline static __inline

#endif /* _DEMAC_CONFIG_H */
