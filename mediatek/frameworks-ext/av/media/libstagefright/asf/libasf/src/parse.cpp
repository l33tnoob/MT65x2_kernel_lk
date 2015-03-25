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
#include "asfparser.h"
/**
 * Read next object from buffer pointed by data. Notice that
 * no buffer overflow checks are done! This function always
 * expects to have 24 bytes available, which is the size of
 * the object header (GUID + data size)
 */
void
ASFParser::asf_parse_read_object(asfint_object_t *obj, uint8_t *data)
{
        if (!obj || !data)
                return;

        ASFByteIO::asf_byteio_getGUID(&obj->guid, data);
        obj->type = asf_guid_get_type(&obj->guid);
        obj->size = ASFByteIO::asf_byteio_getQWLE(data + 16);//8byte
        obj->full_data = data;
        obj->datalen = 0;
        obj->data = NULL;
        obj->next = NULL;

        if (obj->type == GUID_UNKNOWN) {
                ALOGE("unknown object: %x-%x-%x-%02x%02x%02x%02x%02x%02x%02x%02x, %ld bytes\n",
                             obj->guid.v1, obj->guid.v2, obj->guid.v3, obj->guid.v4[0],
                             obj->guid.v4[1], obj->guid.v4[2], obj->guid.v4[3], obj->guid.v4[4],
                             obj->guid.v4[5], obj->guid.v4[6], obj->guid.v4[7], (long) obj->size);
                ALOGI("obj->type=%d,size=%lld,datalen=%lld\n",
                    obj->type,obj->size,obj->datalen);
        }
}

/**
 * Parse header extension object. Takes a pointer to a newly allocated
 * header extension structure, a pointer to the data buffer and the 
 * length of the data buffer as its parameters. Subobject contents are
 * not parsed, but they are added as a linked list to the header object.
 */
int
ASFParser::asf_parse_headerext(asf_object_headerext_t *header, uint8_t *buf, uint64_t buflen)
{
        int64_t datalen;
        uint8_t *data = NULL;

        if (!header || !buf)
                return ASF_ERROR_INTERNAL;

        if (header->size < 46) {
                /* invalide size for headerext */
                return ASF_ERROR_OBJECT_SIZE;
        }

        /* Read reserved and datalen fields from the buffer */
        ASFByteIO::asf_byteio_getGUID(&header->reserved1, buf + 24);
        header->reserved2 = ASFByteIO::asf_byteio_getWLE(buf + 40);
        header->datalen = ASFByteIO::asf_byteio_getDWLE(buf + 42);

        if (header->datalen != header->size - 46) {
                /* invalid header extension data length value */
                return ASF_ERROR_INVALID_LENGTH;
        }
        header->data = buf + 46;

        ALOGV("parsing header extension subobjects\n");

        datalen = header->datalen;
        data = header->data;
        while (datalen > 0) {
                asfint_object_t *current;

                if (datalen < 24) {
                        /* not enough data for reading a new object */
                        break;
                }

                /* Allocate a new subobject */
                current = (asfint_object_t *)malloc(sizeof(asfint_object_t));
                if (!current) {
                        return ASF_ERROR_OUTOFMEM;
                }

                asf_parse_read_object(current, data);
                if (current->size > (uint64_t)datalen || current->size < 24) {
                        /* invalid object size */
                        break;
                }
                current->datalen = current->size - 24;
                current->data = data + 24;

                /* add to the list of subobjects */
                if (!header->first) {
                        header->first = current;
                        header->last = current;
                } else {
                        header->last->next = current;
                        header->last = current;
                }

                data += current->size;
                datalen -= current->size;
        }

        if (datalen != 0) {
                /* not enough data for reading the whole object */
                return ASF_ERROR_INVALID_LENGTH;
        }

        ALOGV("header extension subobjects parsed successfully\n");

        return header->size;
}

/**
 * Takes an initialized asf_file_t structure file as a parameter. Allocates
 * a new asf_object_header_t in file->header and uses the file->iostream to
 * read all fields and subobjects into it. Finally calls the
 * asf_parse_header_validate function to validate the values and parse the
 * commonly used values into the asf_file_t struct itself.
 */
int
ASFParser::asf_parse_header()
{
        asf_object_header_t *header = NULL;
        asf_iostream_t *iostream = NULL;
        uint8_t hdata[30];
        int tmp;

        file->header = NULL;
        iostream = &file->iostream;

        /* object minimum is 24 bytes and header needs to have
         * the subobject count field and two reserved fields */
        memset(hdata, 0, 30);
        tmp = ASFByteIO::asf_byteio_read(hdata, 30, iostream);
        if (tmp < 0) {
                /* not enough data to read the header object */
                ALOGE("asf_parse_header:error 1\n");
                return tmp;
        }

        file->header = (asf_object_header_t*)malloc(sizeof(asf_object_header_t));
        if (!file->header) {
                return ASF_ERROR_OUTOFMEM;
        }
        memset(file->header, 0, sizeof(asf_object_header_t));
        header = file->header;
        /* clear header extension object and subobject list */
        header->ext = NULL;
        header->first = NULL;
        header->last = NULL;

        /* read the object and check its size value */
        asf_parse_read_object((asfint_object_t *) header, hdata);
        if (header->size < 30) {
                /* invalid size for header object */
                ALOGE("asf_parse_header:error 2\n");
                return ASF_ERROR_OBJECT_SIZE;
        }
        /* to check not ASF file type */
        if (header->type == GUID_UNKNOWN) {
                /* invalid GUID for header object */
                return ASF_ERROR_INVALID_VALUE;
        }

        /* read header object specific compulsory fields */
        header->subobjects = ASFByteIO::asf_byteio_getDWLE(hdata + 24);//sub header object number
        header->reserved1 = hdata[28];
        header->reserved2 = hdata[29];


        /* the header data needs to be allocated for reading */
        header->datalen = header->size - 30;
        header->data = (uint8_t*)malloc(header->datalen * sizeof(uint8_t));
        if (!header->data) {
                return ASF_ERROR_OUTOFMEM;
        }
        memset(header->data, 0, header->datalen * sizeof(uint8_t));

        tmp = ASFByteIO::asf_byteio_read(header->data, header->datalen, iostream);
        if (tmp < 0) {
                ALOGE("asf_parse_header:error 3\n");
                return tmp;
        }

        if (header->subobjects > 0) {
                uint64_t datalen;
                uint8_t *data = NULL;
                int i;

                ALOGV("starting to read subobjects\n");

                /* use temporary variables for use during the read */
                datalen = header->datalen;
                data = header->data;
                for (i=0; i<header->subobjects; i++) {
                        void * tmp = NULL;
                        asfint_object_t *current = NULL;

                        if (datalen < 24) {//UUID+Size ==24
                                /* not enough data for reading object */
                                break;
                        }

                        
                        //current = (asfint_object_t *)oscl_malloc(sizeof(asfint_object_t));
                        tmp = (void*)malloc(sizeof(asfint_object_t));
                        memset(tmp, 0, sizeof(asfint_object_t));
                        current = (asfint_object_t *)tmp;
                        if (!current) {
                                return ASF_ERROR_OUTOFMEM;
                        }

                        asf_parse_read_object(current, data);
                        if (current->size > datalen || current->size < 24) {
                                /* invalid object size */
                                ALOGE("invalid object size\n");
                                break;
                        }

                        /* Check if the current subobject is a header extension
                         * object or just a normal subobject */
                        if (current->type == GUID_HEADER_EXTENSION && !header->ext) {
                                int ret;
                                asf_object_headerext_t *headerext = NULL;

                                /* we handle header extension separately because it has
                                 * some subobjects as well */
                                //current = (asf_object_headerext_t*)oscl_realloc(current, sizeof(asf_object_headerext_t));
                                //headerext = (asf_object_headerext_t *) current;

                                //changed by satish to fix compiler conversion problem
                                tmp = (void*)realloc(tmp, sizeof(asf_object_headerext_t));
                                current = (asfint_object_t *)tmp;
                                headerext = (asf_object_headerext_t *) tmp;


                                headerext->first = NULL;
                                headerext->last = NULL;
                                ret = asf_parse_headerext(headerext, data, datalen);

                                if (ret < 0) {
                                        /* error parsing header extension */
                                        return ret;
                                }

                                header->ext = headerext;
                        } else {
                                if (current->type == GUID_HEADER_EXTENSION) {
                                        ALOGV("WARNING! Second header extension object found, ignoring it!\n");
                                }

                                current->datalen = current->size - 24;
                                current->data = data + 24;

                                /* add to list of subobjects */
                                if (!header->first) {
                                        header->first = current;
                                        header->last = current;
                                } else {
                                        header->last->next = current;
                                        header->last = current;
                                }
                        }

                        data += current->size;
                        datalen -= current->size;
                }

                if (i != header->subobjects || datalen != 0) {
                        /* header data size doesn't match given subobject count */
                        return ASF_ERROR_INVALID_VALUE;
                }

                ALOGV("%d subobjects read successfully\n", i);
        }

        tmp = asf_parse_header_validate(file->header);
        if (tmp < 0) {
                /* header read ok but doesn't validate correctly */
                return tmp;
        }

        ALOGV("header validated correctly\n");

        return header->size;
}

/**
 * Takes an initialized asf_file_t structure file as a parameter. Allocates
 * a new asf_object_data_t in file->data and uses the file->iostream to
 * read all its compulsory fields into it. Notice that the actual data is
 * not read in any way, because we need to be able to work with non-seekable
 * streams as well.
 */
int
ASFParser::asf_parse_data()
{
        asf_object_data_t *data = NULL;
        asf_iostream_t *iostream = NULL;
        uint8_t ddata[50];
        int tmp;

        memset(ddata, 0, 50);
        file->data = NULL;
        iostream = &file->iostream;

        /* object minimum is 24 bytes and data object needs to have
         * 26 additional bytes for its internal fields */
        tmp = ASFByteIO::asf_byteio_read(ddata, 50, iostream);
        if (tmp < 0) {
                return tmp;
        }

        file->data = (asf_object_data_t*)malloc(sizeof(asf_object_data_t));
        if (!file->data) {
                return ASF_ERROR_OUTOFMEM;
        }
        data = file->data;
        memset(data, 0, sizeof(asf_object_data_t));

        /* read the object and check its size value */
        asf_parse_read_object((asfint_object_t *) data, ddata);//parser 50 byte
        if (data->size < 50) {
                /* invalid size for data object */
                return ASF_ERROR_OBJECT_SIZE;
        }

        /* read data object specific compulsory fields */
        ASFByteIO::asf_byteio_getGUID(&data->file_id, ddata + 24);
        data->total_data_packets = ASFByteIO::asf_byteio_getQWLE(ddata + 40);
        data->reserved = ASFByteIO::asf_byteio_getWLE(ddata + 48);
        data->packets_position = file->position + 50;

        /* If the file_id GUID in data object doesn't match the
         * file_id GUID in headers, the file is corrupted */
        if (!asf_guid_match(&data->file_id, &file->file_id)) {
                return ASF_ERROR_INVALID_VALUE;
        }

        /* if data->total_data_packets is non-zero (not a iostream) and
           the data packets count doesn't match, return error */
        if (data->total_data_packets &&
            data->total_data_packets != file->data_packets_count) {
                return ASF_ERROR_INVALID_VALUE;
        }

        return 50;
}




/**
 * Takes an initialized asf_file_t structure file as a parameter. Allocates
 * a new asf_object_index_t in file->index and uses the file->iostream to
 * read all its compulsory fields into it. Notice that the actual data is
 * not read in any way, because we need to be able to work with non-seekable
 * streams as well.
 */
int
ASFParser::asf_parse_index_simple_index()
{
        asf_object_simple_index_t *simple_index = NULL;
        asf_iostream_t *iostream = NULL;
        uint8_t idata[56];
        uint64_t entry_data_size;
        uint8_t *entry_data = NULL;
        int tmp;
        uint32_t i;
        ALOGI("-----asf_parse_index_simple_index----");
        file->simple_index = NULL;
        iostream = &file->iostream;
        memset(idata, 0, 56);

		// Morris Yang 6.1 Page 57
        /* read the raw data of an index header */
        tmp = ASFByteIO::asf_byteio_read(idata, 56, iostream);
        if (tmp < 0) {
                ALOGE("asf_parse_index_simple_index:error 1\n");
                return tmp;
        }

        /* allocate the index object */
        simple_index = (asf_object_simple_index_t*)malloc(sizeof(asf_object_simple_index_t));
        if (!simple_index) {
                return ASF_ERROR_OUTOFMEM;
        }
        memset(simple_index, 0, sizeof(asf_object_simple_index_t));

        asf_parse_read_object((asfint_object_t *) simple_index, idata);

        if (simple_index->type != GUID_SIMPLE_INDEX) {
                tmp = simple_index->size;
                free(simple_index);
                ALOGE(" index->type != GUID_SIMPLE_INDEX The guid type was wrong, just return the bytes to skip \n");
                /* The guid type was wrong, just return the bytes to skip */
                return (tmp == 0) ? ASF_ERROR_EOF : ASF_ERROR_IO;
        }

        if (simple_index->size < 56) {
                /* invalid size for index object */
                free(simple_index);
                ALOGE("[ASF_ERROR]invalid size 1 for index object\n");
                return ASF_ERROR_OBJECT_SIZE;
        }

        ASFByteIO::asf_byteio_getGUID(&simple_index->file_id, idata + 24);
        simple_index->entry_time_interval = ASFByteIO::asf_byteio_getQWLE(idata + 40);//in 100-nanosecond units. 
        simple_index->max_packet_count = ASFByteIO::asf_byteio_getDWLE(idata + 48);
        simple_index->entry_count = ASFByteIO::asf_byteio_getDWLE(idata + 52);

        if (simple_index->entry_count * 6 + 56 > simple_index->size) {//entry_count:packet num 32 +packet count16 =6byte
                free(simple_index);
                ALOGE("[ASF_ERROR]invalid size 2 for index object\n");
                return ASF_ERROR_INVALID_LENGTH;
        }

        entry_data_size = simple_index->entry_count * 6;
        entry_data = (uint8_t*)malloc(entry_data_size * sizeof(uint8_t));
        if (!entry_data) {
                free(simple_index);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index object\n");
                return ASF_ERROR_OUTOFMEM;
        }
        memset(entry_data, 0, entry_data_size * sizeof(uint8_t));

        tmp = ASFByteIO::asf_byteio_read(entry_data, entry_data_size, iostream);
        if (tmp < 0) {
                free(simple_index);
                free(entry_data);
                ALOGE("asf_parse_index_simple_index:error 2\n");
                return tmp;
        }

	//ALOGD ("@@ index->entry_count = %d, index->entry_time_interval = %d\n", index->entry_count, index->entry_time_interval);
        simple_index->entries = (asf_simple_index_entry_t*)malloc(simple_index->entry_count * sizeof(asf_simple_index_entry_t));
        if (!simple_index->entries) {
                free(simple_index);
                free(entry_data);
                ALOGE("asf_parse_index_simple_index:error 3\n");
                return ASF_ERROR_OUTOFMEM;
        }
        memset(simple_index->entries, 0, simple_index->entry_count * sizeof(asf_simple_index_entry_t));

        for (i=0; i<simple_index->entry_count; i++) {
                simple_index->entries[i].packet_index = ASFByteIO::asf_byteio_getDWLE(entry_data + i*6);
                simple_index->entries[i].packet_count = ASFByteIO::asf_byteio_getWLE(entry_data + i*6 + 4);
				//ALOGD ("@@ entries[%d] packet_index = %d, packet_count = %d\n", i, index->entries[i].packet_index, index->entries[i].packet_count);
        }

        free(entry_data);
        file->simple_index = simple_index;        

        return simple_index->size;
}
int
ASFParser::asf_parse_index_index()
{
        asf_object_index_t *index = NULL;
        asf_iostream_t *iostream = NULL;
        uint8_t idata[34];
        uint64_t specifiers_data_size;
        uint8_t *specifiers_data = NULL;
         
        int tmp =-1;
        uint32_t i;

        ALOGI("-----asf_parse_index_index----");
        
        file->index = NULL;
        iostream = &file->iostream;
        memset(idata, 0, 34);

        tmp = ASFByteIO::asf_byteio_read(idata, 34, iostream);
        if (tmp < 0) {
                return tmp;
        }

        // allocate the index object 
        index = (asf_object_index_t*)calloc(1,sizeof(asf_object_index_t));
        if (!index) {
                ALOGE("asf_parse_index_index:ASF_ERROR_OUTOFMEM 0\n");
                return ASF_ERROR_OUTOFMEM;
        }
        asf_parse_read_object((asfint_object_t *) index, idata);

        if (index->type != GUID_INDEX) {
                tmp = index->size;
                free(index);
                ALOGE(" index->type != GUID_INDEX The guid type was wrong, just return the bytes to skip \n");
                // The guid type was wrong, just return the bytes to skip  
                return (tmp == 0) ? ASF_ERROR_EOF : tmp;
        }

        if (index->size < 34) {
                // invalid size for index object 
                free(index);
                ALOGE("[ASF_ERROR]asf_parse_index_index: ASF_ERROR_OBJECT_SIZE 1\n");
                return ASF_ERROR_OBJECT_SIZE;
        }

        index->index_entry_time_interval = ASFByteIO::asf_byteio_getQWLE(idata + 24);//in 100-nanosecond units. 
        index->index_specifies_count= ASFByteIO::asf_byteio_getWLE(idata + 28);
        index->index_block_count= ASFByteIO::asf_byteio_getDWLE(idata + 30);

        if(index->index_block_count>1){
            tmp=index->size; // to make sure we can parser next index 
            free(index); 
            ALOGE("index_block_count=%d >1, not support now\n",index->index_block_count);
            return (tmp == 0) ? ASF_ERROR_EOF : tmp;
        }
        else if(index->index_specifies_count>1){
            tmp=index->size; // to make sure we can parser next index 
            //free(index); 
            //ALOGE("index_specifies_count=%d >1, not support now\n",index->index_specifies_count);
            //return (tmp == 0) ? ASF_ERROR_EOF : tmp;
        }

        
        //right? check the min size
        if ((34 + index->index_block_count * 16 + index->index_specifies_count * 4) > index->size) { 
                free(index);
                ALOGE("[ASF_ERROR]parse index object:invalid size 2 \n");
                return ASF_ERROR_INVALID_LENGTH;
        }
        
        //Index Specifiers
       specifiers_data_size = index->index_specifies_count * sizeof(asf_index_specifiers_t);
       specifiers_data = (uint8_t*)calloc(1,specifiers_data_size*sizeof(uint8_t));
        if (!specifiers_data) {
                free(index);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index object 1 \n");
                return ASF_ERROR_OUTOFMEM;
        }
        
        //read specifiers_data
       tmp = ASFByteIO::asf_byteio_read(specifiers_data, specifiers_data_size, iostream);
       if (tmp < 0) {
                free(index);
                free(specifiers_data);
                return tmp;
       }
       ALOGI("---index_specifies_count =%d---\n", index->index_specifies_count);
        
       index->specifiers_entry= (asf_index_specifiers_t*)calloc(1,specifiers_data_size);
       if (!index->specifiers_entry) {
                free(index);
                free(specifiers_data);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index->specifiers_entry \n");
                return ASF_ERROR_OUTOFMEM;
        }

       for(i=0;i<index->index_specifies_count;i++){
            ((asf_index_specifiers_s*)index->specifiers_entry)->stream_num =ASFByteIO::asf_byteio_getWLE((specifiers_data)+4*i);
            ((asf_index_specifiers_s*)index->specifiers_entry)->index_type =ASFByteIO::asf_byteio_getWLE((specifiers_data+2)+4*i);
            
            ALOGI("---index_specifiers_entry[%d].stream_num=%d,index_specifiers_entry[%d].type=%d---\n",
                        i,((asf_index_specifiers_s*)index->specifiers_entry)->stream_num,
                        i,((asf_index_specifiers_s*)index->specifiers_entry)->index_type);
       }

       free(specifiers_data);
       
       //Index Blocks, just 1 count
       uint32_t block_count;
       uint32_t index_entry_count;
       uint64_t index_block_position;
       uint8_t *block_data = NULL;       
       uint64_t entry_data_size;
       uint8_t *entry_data = NULL;
       
       block_count = index->index_block_count;
       if (NULL == file->header->index_parameters) { 
	      ALOGI("[ASF_ERROR] file->header->index_parameters null point");
		  return  ASF_ERROR_OBJECT_SIZE;
	   }

	    block_data = (uint8_t*)calloc(1,(4+file->header->index_parameters->index_specifiers_count*8) * sizeof(uint8_t));
		//parser 1ht blocks
		tmp = ASFByteIO::asf_byteio_read(block_data,4+file->header->index_parameters->index_specifiers_count*8, iostream);

       if (tmp < 0) {
                free(index);
                return tmp;
        }

       index_entry_count =ASFByteIO::asf_byteio_getDWLE(block_data);   
       ALOGI("---Index Blocks,:index_entry_count=%d----\n",index_entry_count);

       index_block_position =ASFByteIO::asf_byteio_getDWLE(block_data + 4);   
       ALOGI("---Index Blocks,:index_block_position=%lld----\n",index_block_position);
       
	   entry_data_size = index_entry_count * (file->header->index_parameters->index_specifiers_count *4);

       entry_data = (uint8_t*)calloc(1,entry_data_size * sizeof(uint8_t));

        if (!entry_data) {
                free(index->specifiers_entry);
                free(index);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index object2 \n");
                return ASF_ERROR_OUTOFMEM;
        }

        tmp = ASFByteIO::asf_byteio_read(entry_data, entry_data_size, iostream);
        if (tmp < 0) {
                free(index->specifiers_entry);
                free(index);
                return tmp;
        }

        index->index_block = (asf_index_blocks_s *)calloc(1,sizeof(asf_index_blocks_s));//entry_count 4+ Block Positions 8 + entry pointer 4

        if (!index->index_block) {
                free(index->specifiers_entry);                
                free(index);
                free(entry_data);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index object 3 \n");
                return ASF_ERROR_OUTOFMEM;
        }

        index->index_block->index_entry_count = index_entry_count;        
        index->index_block->block_positions =index_block_position; 

        index->index_block->index_entry = (asf_index_entry_s *)calloc(1,index_entry_count*sizeof(asf_index_entry_s));
        if (!index->index_block->index_entry ) {
                free(index->specifiers_entry);
                free(index->index_block);
                free(entry_data);
                free(index);
                ALOGE("[ASF_ERROR]ASF_ERROR_OUTOFMEM for index object 3 \n");
                return ASF_ERROR_OUTOFMEM;
        }
        
        for(i=0;i<index_entry_count;i++)
        {
			if (ASF_STREAM_TYPE_VIDEO == file->streams[1].type)
			{
				index->index_block->index_entry[i].offset = ASFByteIO::asf_byteio_getDWLE(entry_data+i*(file->header->index_parameters->index_specifiers_count*4));
			}
			else
			{
				index->index_block->index_entry[i].offset = ASFByteIO::asf_byteio_getDWLE(entry_data+i*(file->header->index_parameters->index_specifiers_count*4)+4);
			}
			if(index->index_block->index_entry[i].offset >= file->file_size)
			{
				index->index_block->index_entry[i].offset = 0;
			}
			ALOGI("%d entry offset =%d\n",i,index->index_block->index_entry[i].offset);
        }
       

        free(entry_data);
        free(block_data);
        file->index = index;
        return index->size;
}

//main entry of parsing index object
//int64_t next_index_position is the cuurent file postion to find index
int
ASFParser::asf_parse_index(int64_t next_index_position){ 
        asf_object_index_t *index = NULL;
        asf_iostream_t *iostream = NULL;
        int64_t seek_position=0;
        uint8_t idata[24];

        int tmp =-1;
        
        
        ALOGI("-----asf_parse_index----");
        iostream = &file->iostream;

        //from the file->index_postion read 24 byte : GUID+size
        //obj min is 24 byte size
        tmp = ASFByteIO::asf_byteio_read(idata, 24, iostream);
        if (tmp < 0) {
				ALOGE("asf_parse_index:error1 tmp=%d",tmp);
                return tmp;//ASF_ERROR_EOF or ASF_ERROR_IO;
        }
        ALOGI("asf_parse_index:tmp1=%d",tmp);
        // allocate the index object 
        index = (asf_object_index_t*)calloc(1,sizeof(asf_object_index_t));
        if (!index) {
                ALOGE("asf_parse_index:ASF_ERROR_OUTOFMEM 0\n");
                return ASF_ERROR_OUTOFMEM;
        }
        asf_parse_read_object((asfint_object_t *) index, idata);

        if (index->type != GUID_INDEX && index->type != GUID_SIMPLE_INDEX ) {
                tmp = index->size;
                free(index);
                // The guid type was wrong, just return the bytes to skip  
                return (tmp == 0) ? ASF_ERROR_EOF : tmp;
                //return tmp;
        }
        else{ //GUID_INDEX || GUID_SIMPLE_INDEX
            //back to the start postion to parse this index
            seek_position = file->iostream.seek(file->iostream.source,
                                                next_index_position);
            if(index->type == GUID_SIMPLE_INDEX ){
                   tmp = asf_parse_index_simple_index();                
            }
            if(index->type == GUID_INDEX ){
                   tmp = asf_parse_index_index();                
            }

        }
        free(index);// re-parser the index
        return tmp;
    
}


