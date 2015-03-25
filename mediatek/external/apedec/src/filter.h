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

#ifndef _APE_FILTER_H
#define _APE_FILTER_H

#include "demac_config.h"
#include "decoder.h"

STATIC_DECLARE void ape_filter_init_16_11(struct filter_t* f0, struct filter_t* f1, filter_int* buf);
STATIC_DECLARE void ape_filter_apply_16_11(int fileversion,
                        struct filter_t *f0,
                        struct filter_t *f1,
                        int32_t* data0,
                        int32_t* data1,
                        int count);

STATIC_DECLARE void ape_filter_init_64_11(struct filter_t* f0, struct filter_t* f1, filter_int* buf);
STATIC_DECLARE void ape_filter_apply_64_11(int fileversion,
                        struct filter_t *f0,
                        struct filter_t *f1,
                        int32_t* data0,
                        int32_t* data1,
                        int count);

STATIC_DECLARE void ape_filter_init_32_10(struct filter_t* f0, struct filter_t* f1, filter_int* buf);
STATIC_DECLARE void ape_filter_apply_32_10(int fileversion,
                        struct filter_t *f0,
                        struct filter_t *f1,
                        int32_t* data0,
                        int32_t* data1,
                        int count);

STATIC_DECLARE void ape_filter_init_256_13(struct filter_t* f0, struct filter_t* f1, filter_int* buf);
STATIC_DECLARE void ape_filter_apply_256_13(int fileversion,
                         struct filter_t *f0,
                         struct filter_t *f1,
                         int32_t* data0,
                         int32_t* data1,
                         int count);

STATIC_DECLARE void ape_filter_init_1280_15(struct filter_t* f0, struct filter_t* f1, filter_int* buf);
STATIC_DECLARE void ape_filter_apply_1280_15(int fileversion,
                          struct filter_t *f0,
                          struct filter_t *f1,
                          int32_t* data0,
                          int32_t* data1,
                          int count);

#endif
