/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*  libasf - An Advanced Systems Format media file parser   
*  Copyright (C) 2006-2010 Juho Vähä-Herttua   
*   
*  This library is free software; you can redistribute it and/or   
*  modify it under the terms of the GNU Lesser General Public   
*  License as published by the Free Software Foundation; either   
*  version 2.1 of the License, or (at your option) any later version.   
*   *  This library is distributed in the hope that it will be useful,   
*  but WITHOUT ANY WARRANTY; without even the implied warranty of 
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   
*  Lesser General Public License for more details.   
*   
*  You should have received a copy of the GNU Lesser General Public   
*  License along with this library; if not, write to the Free Software   
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA   */  

//#include <stdlib.h>

#ifndef ASFPARSER_H_INCLUDED
#include "asfparser.h"
#endif

#include <stdlib.h>
/**
 * Decode UTF-16LE text from buffer of buflen size and
 * allocate a new buffer containing the same string
 * encoded as UTF-8. Supports characters outside of BMP
 * encoded as an UTF-16 surrogate pair. Returns NULL in 
 * case of allocation failure or invalid surrogate pair.
 * Buflen is in bytes.
 */
char * ASFParser::asf_utf8_from_utf16le(uint8_t *buf, uint16_t buflen)
{
        uint32_t length, pos;
        char *ret;
        int i;

        length = 0;
        for (i=0; i<buflen/2; i++) {
                uint16_t wchar1, wchar2;

                wchar1 = ASFByteIO::asf_byteio_getWLE(&buf[i*2]);
                if (wchar1 >= 0xD800 && wchar1 < 0xDB00) {
                        i++;

                        if (i*2 >= buflen) {
                                /* unexpected end of buffer */
                                return NULL;
                        }
                        wchar2 = ASFByteIO::asf_byteio_getWLE(&buf[i*2]);
                        if (wchar2 < 0xDB00 || wchar2 > 0xDFFF) {
                                /* invalid surrogate pair */
                                return NULL;
                        }
                        length += 4;
                } else if (wchar1 > 0x07FF) {
                        length += 3;
                } else if (wchar1 > 0x7F) {
                        length += 2;
                } else {
                        length++;
                }
        }

        ret = (char*)malloc(length + 1);
        if (!ret) {
                return NULL;
        }

        pos = 0;
        for (i=0; i<buflen/2; i++) {
                uint16_t wchar1, wchar2;
                uint32_t codepoint;

                wchar1 = ASFByteIO::asf_byteio_getWLE(&buf[i*2]);
                if (wchar1 >= 0xD800 && wchar1 < 0xDB00) {
                        i++;
                        wchar2 = ASFByteIO::asf_byteio_getWLE(&buf[i*2]);
                        codepoint = 0x10000;
                        codepoint += ((wchar1 & 0x03FF) << 10);
                        codepoint |=  (wchar2 & 0x03FF);
                } else {
                        codepoint = wchar1;
                }

                if (codepoint > 0xFFFF) {
                        ret[pos++] = 0xF0 | ((codepoint >> 18) & 0x07);
                        ret[pos++] = 0x80 | ((codepoint >> 12) & 0x3F);
                        ret[pos++] = 0x80 | ((codepoint >> 6)  & 0x3F);
                        ret[pos++] = 0x80 |  (codepoint & 0x3F);
                } else if (codepoint > 0x07FF) {
                        ret[pos++] = 0xE0 |  (codepoint >> 12);
                        ret[pos++] = 0x80 | ((codepoint >> 6) & 0x3F);
                        ret[pos++] = 0x80 |  (codepoint & 0x3F);
                } else if (codepoint > 0x7F) {
                        ret[pos++] = 0xC0 |  (codepoint >> 6);
                        ret[pos++] = 0x80 |  (codepoint & 0x3F);
                } else {
                        ret[pos++] = codepoint;
                }
        }

        ret[length] = '\0';
        return ret;
}
