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
#ifdef __SCRAMBLE_SYMBOL__
#include "apedec_scrambled.h"
#endif
#include "predictor.c"
#include "filter_16_11.c"
#include "filter_32_10.c"
#include "filter_64_11.c"
#include "filter_256_13.c"
#include "filter_1280_15.c"
#include "entropy.c"
#include "crc.c"
#include "decoder.c"
#include "ape_decoder_swip.c"


