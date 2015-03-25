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

#include <stdlib.h>

#ifndef ASFPARSER_H_INCLUDED
#include "asfparser.h"
#endif

#define GETLEN2b(bits) (((bits) == 0x03) ? 4 : bits)

#define GETVALUE2b(bits, data) \
        (((bits) != 0x03) ? ((bits) != 0x02) ? ((bits) != 0x01) ? \
         0 : *(data) : ASFByteIO::asf_byteio_getWLE(data) : ASFByteIO::asf_byteio_getDWLE(data))

int ASFParser::asf_data_read_packet_data(asf_packet_t *packet, uint8_t flags,
                          uint8_t *data, uint32_t len)
{
        uint8_t datalen;

        if (!packet || !data)
                return 0;

	// Morris Yang : 5.2.2  Page.47
        datalen = GETLEN2b((flags >> 1) & 0x03) +
                  GETLEN2b((flags >> 3) & 0x03) +
                  GETLEN2b((flags >> 5) & 0x03) + 6;

        if (datalen > len) {
                return ASF_ERROR_INVALID_LENGTH;
        }

        packet->length = GETVALUE2b((flags >> 5) & 0x03, data);
        data += GETLEN2b((flags >> 5) & 0x03);
        /* sequence value should be never used anywhere */
        GETVALUE2b((flags >> 1) & 0x03, data);
        data += GETLEN2b((flags >> 1) & 0x03);
        packet->padding_length = GETVALUE2b((flags >> 3) & 0x03, data);
        data += GETLEN2b((flags >> 3) & 0x03);
        packet->send_time = ASFByteIO::asf_byteio_getDWLE(data);
        data += 4;
        packet->duration = ASFByteIO::asf_byteio_getWLE(data);
        data += 2;

        return datalen;
}

int ASFParser::asf_data_read_payload_data(asf_payload_t *payload, 
        uint8_t flags, 
        uint8_t *data, 
        int size)
{
        uint8_t datalen;

        if (!payload || !data)
                return 0;

        datalen = GETLEN2b(flags & 0x03) +
                  GETLEN2b((flags >> 2) & 0x03) +
                  GETLEN2b((flags >> 4) & 0x03);

        if (datalen > size) {
                return ASF_ERROR_INVALID_LENGTH;
        }

        payload->media_object_number = GETVALUE2b((flags >> 4) & 0x03, data);
        data += GETLEN2b((flags >> 4) & 0x03);
        payload->media_object_offset = GETVALUE2b((flags >> 2) & 0x03, data);//in compress payload, this field is pts
        data += GETLEN2b((flags >> 2) & 0x03);
        payload->replicated_length = GETVALUE2b(flags & 0x03, data);
        data += GETLEN2b(flags & 0x03);

        return datalen;
}

int ASFParser::asf_data_read_payloads(asf_packet_t *packet,
                       uint64_t preroll,
                       uint8_t multiple,
                       uint8_t type,
                       uint8_t flags,
                       uint8_t *data,
                       uint32_t datalen)
{
        asf_payload_t pl;
        int i, tmp, skip;

        if (!packet || !data)
                return 0;

        skip = 0, i = 0;
        while (i < packet->payload_count) {
                uint8_t pts_delta = 0;
                int compressed = 0;

		  // Morris Yang 5.2.3.3 Page 53
                pl.stream_number = data[skip] & 0x7f;
                pl.key_frame = !!(data[skip] & 0x80);
                skip++;

                tmp = asf_data_read_payload_data(&pl, flags, data + skip, datalen - skip);
                if (tmp < 0) {
                        return tmp;
                }
                skip += tmp;

                if (pl.replicated_length > 1) {//min is 8bytes
                        if (pl.replicated_length < 8 || pl.replicated_length + skip > datalen) {
                                /* not enough data */
                                return ASF_ERROR_INVALID_LENGTH;
                        }
                        pl.replicated_data = data + skip;
                        skip += pl.replicated_length;
                        //pres time bytes(4bytes)Presentation Time
                        pl.pts = ASFByteIO::asf_byteio_getDWLE(pl.replicated_data + 4);
                        //ALOGE("asf_data_read_payloads 1: pl.pts=%d\n",pl.pts);
                } else if (pl.replicated_length == 1) {//conpress payload data
                        if (skip >= (int)datalen) {
                                /* not enough data */
                                return ASF_ERROR_INVALID_LENGTH;
                        }

                        /* in compressed payload object offset is actually pts */
                        pl.pts = pl.media_object_offset;
                        ALOGI("asf_data_read_payloads 2: pl.pts=%d\n",pl.pts);
                        pl.media_object_offset = 0;

                        pl.replicated_length = 0;
                        pl.replicated_data = NULL;

                        pts_delta = data[skip];
                        skip++;
                        compressed = 1;
                } else {//==0
                        pl.pts = packet->send_time;
                        //ALOGE("asf_data_read_payloads 3: pl.pts=%d\n",pl.pts);
                        pl.replicated_data = NULL;
                }

                /* substract preroll value from pts since it's counted in */
                pl.pts -= preroll;//??????????????
                /* FIXME: check that pts is positive */

                if (multiple) {
                        tmp = GETLEN2b(type);

                        if (tmp != 2) {
                                /* in multiple payloads datalen should be a word */
                                return ASF_ERROR_INVALID_VALUE;
                        }
                        if (skip + tmp > (int)datalen) {
                                /* not enough data */
                                return ASF_ERROR_INVALID_LENGTH;
                        }

                        pl.datalen = GETVALUE2b(type, data + skip);
                        skip += tmp;
                } else {
                        pl.datalen = datalen - skip;
                }

                if (compressed) {
                        int payloads, start = skip, used = 0;

                        /* count how many compressed payloads this payload includes */
                        for (payloads=0; used < (int)pl.datalen; payloads++) {
                                used += 1 + data[start + used];//1byte data len + real data len
                        }

                        if (used != (int)pl.datalen) {
                                /* invalid compressed data size */
                                return ASF_ERROR_INVALID_LENGTH;
                        }

                        /* add additional payloads excluding the already allocated one */
                        packet->payload_count += payloads - 1;
                        if (packet->payload_count > packet->payloads_size) {
                                void *tempptr;

                                tempptr = realloc(packet->payloads,
                                                  packet->payload_count * sizeof(asf_payload_t));
                                if (!tempptr) {
                                        return ASF_ERROR_OUTOFMEM;
                                }
                                packet->payloads = (asf_payload_t *)tempptr;
                                packet->payloads_size = packet->payload_count;
                        }

                        while (skip < start + used) {
                                pl.datalen = data[skip];
                                skip++;

                                pl.data = data + skip;
                                skip += pl.datalen;

                                pl.pts += pts_delta;
                                memcpy(&packet->payloads[i], &pl, sizeof(asf_payload_t));
                                i++;
								/*
                                ALOGV("\npayload(%d/%d) stream: %d, key frame: %d, object: %d, offset: %d, pts: %d, datalen: %d\n",
                                             i, packet->payload_count, pl.stream_number, (int) pl.key_frame, pl.media_object_number,
                                             pl.media_object_offset, pl.pts, pl.datalen);
								*/
                        }
                } else {
                        pl.data = data + skip;
                        memcpy(&packet->payloads[i], &pl, sizeof(asf_payload_t));

                        /* update the skipped data amount and payload index */
                        skip += pl.datalen;
                        i++;

						/*
                        ALOGV("payload(%d/%d) stream: %d, key frame: %d, object: %d, offset: %d, pts: %d, datalen: %d\n",
                                     i, packet->payload_count, pl.stream_number, (int) pl.key_frame, pl.media_object_number,
                                     pl.media_object_offset, pl.pts, pl.datalen);
						*/
                }
        }

        return skip;
}

void ASFParser::asf_data_init_packet(asf_packet_t *packet)
{
        if (!packet)
                return;

        packet->ec_length = 0;
        packet->ec_data = NULL;

        packet->length = 0;
        packet->padding_length = 0;
        packet->send_time = 0;
        packet->duration = 0;

        packet->payload_count = 0;
        packet->payloads = NULL;
        packet->payloads_size = 0;

        packet->payload_data_len = 0;
        packet->payload_data = NULL;

        packet->data = NULL;
        packet->data_size = 0;
}

int ASFParser::asf_data_get_packet(asf_packet_t *packet)
{
        asf_iostream_t *iostream = NULL;
        uint32_t read = 0;
        int packet_flags, packet_property, payload_length_type;
        void *tmpptr;
        int tmp;

        if (!packet || !file)
                return 0;

        iostream = &file->iostream;
        if (file->packet_size == 0) {
                ALOGE("asf_data_get_packet:error 1\n");
                return ASF_ERROR_INVALID_LENGTH;
        }

        /* If the internal data is not allocated, allocate it */
        if (packet->data_size < file->packet_size) {
                tmpptr = realloc(packet->data, file->packet_size);
                if (!tmpptr) {
                        ALOGE("asf_data_get_packet:error 2\n");
                        return ASF_ERROR_OUTOFMEM;
                }
                packet->data = (uint8_t*)tmpptr;
                packet->data_size = file->packet_size;
        }

        tmp = ASFByteIO::asf_byteio_read(packet->data,
                              file->packet_size,
                              iostream);
        if (tmp < 0) {
                /* Error reading packet data */
                ALOGE("asf_data_get_packet:error 3\n");
                return tmp;
        }

        tmp = packet->data[read++];
        // Morris Yang : 5.2.1  Page 45
        if (tmp & 0x80) {
                uint8_t opaque_data, ec_length_type;

                packet->ec_length = tmp & 0x0f;
                opaque_data = (tmp >> 4) & 0x01;
                ec_length_type = (tmp >> 5) & 0x03;

                if (ec_length_type != 0x00 ||
                    opaque_data != 0 ||
                    packet->ec_length != 0x02) {
                        /* incorrect error correction flags */
                        ALOGE("asf_data_get_packet:error 4\n");
                        //return ASF_ERROR_INVALID_VALUE;
                }

                if (read+packet->ec_length > file->packet_size) {
                    ALOGE("asf_data_get_packet:error 5\n");
                        return ASF_ERROR_INVALID_LENGTH;
                }
                packet->ec_data = &packet->data[read];
                read += packet->ec_length;
        } else {
                packet->ec_length = 0;
        }
        
        if (read+2 > file->packet_size) {
            ALOGE("asf_data_get_packet:error 6\n");
                return ASF_ERROR_INVALID_LENGTH;
        }
        //Payload parsing information 
        packet_flags = packet->data[read++]; //length type flag
        packet_property = packet->data[read++];

        tmp = asf_data_read_packet_data(packet,
                                        packet_flags,
                                        &packet->data[read],//packet length
                                        file->packet_size - read);
        if (tmp < 0) {
            ALOGE("asf_data_get_packet:error 7\n");
                return tmp;
        }
        read += tmp;

        /* this is really idiotic(Very stupid ), packet length can (and often will) be
         * undefined and we just have to use the header packet size as the size
         * value */
        if (!((packet_flags >> 5) & 0x03)) {
                packet->length = file->packet_size;
        }
        
        /* this is also really idiotic, if packet length is smaller than packet
         * size, we need to manually add the additional bytes into padding length
         */
        if (packet->length < file->packet_size) {
                packet->padding_length += file->packet_size - packet->length;
                packet->length = file->packet_size;
        }

        if (packet->length != file->packet_size) {
                /* packet with invalid length value */
                ALOGE("asf_data_get_packet:error 8\n");
                return ASF_ERROR_INVALID_LENGTH;
        }

        /* check if we have multiple payloads */
        if (packet_flags & 0x01) {
                if (read+1 > packet->length) {
                        ALOGE("asf_data_get_packet:error 9\n");
                        return ASF_ERROR_INVALID_LENGTH;
                }
                tmp = packet->data[read++];

                packet->payload_count = tmp & 0x3f;	// Morris Yang 5.2.3.3 Page 53 (Number of Payloads bits 0-5)
                payload_length_type = (tmp >> 6) & 0x03;
        } else {
                packet->payload_count = 1;
                payload_length_type = 0x02; /* not used */
        }

        packet->payload_data_len = packet->length - read;

        if (packet->payload_count > packet->payloads_size) {
                tmpptr = realloc(packet->payloads,
                                 packet->payload_count * sizeof(asf_payload_t));
                if (!tmpptr) {
                        ALOGE("asf_data_get_packet:error 10\n");
                        return ASF_ERROR_OUTOFMEM;
                }
                packet->payloads = (asf_payload_t *)tmpptr;
                packet->payloads_size = packet->payload_count;
        }

        packet->payload_data = &packet->data[read];
        read += packet->payload_data_len;

        /* The return value will be consumed bytes, not including the padding */
        tmp = asf_data_read_payloads(packet, file->preroll, packet_flags & 0x01,
                                     payload_length_type, packet_property,
                                     packet->payload_data,
                                     packet->payload_data_len - packet->padding_length);
        if (tmp < 0) {
                ALOGE("asf_data_get_packet:error 11\n");
                //return tmp;
        }

		/*
        ALOGE("\npacket read %d bytes, eclen: %d, length: %d, padding: %d, time %d, duration: %d, payloads: %d\n",
                     read, packet->ec_length, packet->length, packet->padding_length, packet->send_time,
                     packet->duration, packet->payload_count);
		*/
        return read;
}

void ASFParser::asf_data_free_packet(asf_packet_t *packet)
{
        if (!packet)
                return;

        if (packet->payloads) free(packet->payloads);
        if (packet->data) free(packet->data);

        packet->ec_data = NULL;
        packet->payloads = NULL;
        packet->payload_data = NULL;
        packet->data = NULL;
}