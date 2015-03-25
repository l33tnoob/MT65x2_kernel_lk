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

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#include "asfparser.h"

#include <media/stagefright/foundation/AString.h>

/**
 * Finds an object with the corresponding GUID type from header object. If
 * not found, just returns NULL.
 */
asfint_object_t * 
ASFParser::asf_header_get_object(asf_object_header_t *header, const guid_type_t type)
{
        asfint_object_t *current;

        current = header->first;
        while (current) {
                if (current->type == type) {
                        return current;
                }
                current = current->next;
        }

        return NULL;
}


asfint_object_t * 
ASFParser::asf_header_ext_get_object(asf_object_header_t *header, const guid_type_t type)
{
        asfint_object_t *current;

        current = header->ext->first;
        while (current) {
                if (current->type == type) {
                        return current;
                }
                current = current->next;
        }

        return NULL;
}

/**
 * Reads the stream properties object's data into the equivalent
 * data structure, and stores it in asf_stream_t structure
 * with the equivalent stream type. Needs the stream properties
 * object data as its input.
 */
int
ASFParser::asf_parse_header_stream_properties(asf_stream_t *stream,
                                   uint8_t *objdata,
                                   uint32_t objsize)
{
        asf_guid_t guid;
        guid_type_t type;
        uint32_t datalen;
        uint8_t *data;

        if (objsize < 78) {
                return ASF_ERROR_INVALID_LENGTH;
        }

        ASFByteIO::asf_byteio_getGUID(&guid, objdata);
        type = asf_guid_get_stream_type(&guid);

        datalen = ASFByteIO::asf_byteio_getDWLE(objdata + 40);
        if (datalen > objsize - 78) {
                return ASF_ERROR_INVALID_LENGTH;
        }
        data = objdata + 54;

        if (type == GUID_STREAM_TYPE_EXTENDED) {
                /* FIXME: Need to find out what actually is here...
                          but for now we can just skip the extended part */
                if (datalen < 64)
                        return ASF_ERROR_INVALID_LENGTH;

                data += 64;
                datalen -= 64;

                /* update the stream type with correct one */
                ASFByteIO::asf_byteio_getGUID(&guid, objdata);
                type = asf_guid_get_stream_type(&guid);
        }

        switch (type) {
        case GUID_STREAM_TYPE_AUDIO:
        case GUID_STREAM_TYPE_EXTENDED_AUDIO:
        {
                asf_waveformatex_t *wfx;

                stream->type = ASF_STREAM_TYPE_AUDIO;

                if (datalen < 18) { //WAVEFORMATEX  is 18
                        return ASF_ERROR_INVALID_LENGTH;
                }
                if (ASFByteIO::asf_byteio_getWLE(data + 16) > datalen - 16) {//WAVEFORMATEX start from 16
                        return ASF_ERROR_INVALID_LENGTH;
                }

                /* this should be freed in asf_close function */
                stream->properties = calloc(1, sizeof(asf_waveformatex_t));
                if (!stream->properties)
                        return ASF_ERROR_OUTOFMEM;
                stream->flags |= ASF_STREAM_FLAG_AVAILABLE;

                wfx = (asf_waveformatex_t *)stream->properties;
                wfx->wFormatTag = ASFByteIO::asf_byteio_getWLE(data);
                wfx->nChannels = ASFByteIO::asf_byteio_getWLE(data + 2);
                wfx->nSamplesPerSec = ASFByteIO::asf_byteio_getDWLE(data + 4);
                wfx->nAvgBytesPerSec = ASFByteIO::asf_byteio_getDWLE(data + 8);
                wfx->nBlockAlign = ASFByteIO::asf_byteio_getWLE(data + 12);
                wfx->wBitsPerSample = ASFByteIO::asf_byteio_getWLE(data + 14);
                wfx->cbSize = ASFByteIO::asf_byteio_getWLE(data + 16);
                wfx->data = data + 18;

                if (wfx->cbSize > datalen - 18) {
                        ALOGV("Invalid waveformatex data length, truncating!\n");
                        wfx->cbSize = datalen - 18;
                }

                break;
        }
        case GUID_STREAM_TYPE_VIDEO:
        {
                asf_bitmapinfoheader_t *bmih;
                uint32_t width, height, flags, data_size;

                stream->type = ASF_STREAM_TYPE_VIDEO;

                if (datalen < 51) {
                        return ASF_ERROR_INVALID_LENGTH;
                }

                width = ASFByteIO::asf_byteio_getDWLE(data);
                height = ASFByteIO::asf_byteio_getDWLE(data + 4);
                flags = data[8];
                data_size = ASFByteIO::asf_byteio_getWLE(data + 9);

                data += 11;
                datalen -= 11;

                if (ASFByteIO::asf_byteio_getDWLE(data) != datalen) {//format data size :BITMAPINFOHEADER  size
                        return ASF_ERROR_INVALID_LENGTH;
                }
                if (width != ASFByteIO::asf_byteio_getDWLE(data + 4) ||
                    height != ASFByteIO::asf_byteio_getDWLE(data + 8) ||
                    flags != 2) {
                        return ASF_ERROR_INVALID_VALUE;
                }

                /* this should be freed in asf_close function */
                stream->properties = malloc(sizeof(asf_bitmapinfoheader_t));
                if (!stream->properties)
                        return ASF_ERROR_OUTOFMEM;
                stream->flags |= ASF_STREAM_FLAG_AVAILABLE;

                bmih = (asf_bitmapinfoheader_t *)stream->properties;
                bmih->biSize = ASFByteIO::asf_byteio_getDWLE(data);
                bmih->biWidth = ASFByteIO::asf_byteio_getDWLE(data + 4);
                bmih->biHeight = ASFByteIO::asf_byteio_getDWLE(data + 8);
                bmih->biPlanes = ASFByteIO::asf_byteio_getDWLE(data + 12);
                bmih->biBitCount = ASFByteIO::asf_byteio_getDWLE(data + 14);
                bmih->biCompression = ASFByteIO::asf_byteio_getDWLE(data + 16);
                bmih->biSizeImage = ASFByteIO::asf_byteio_getDWLE(data + 20);
                bmih->biXPelsPerMeter = ASFByteIO::asf_byteio_getDWLE(data + 24);
                bmih->biYPelsPerMeter = ASFByteIO::asf_byteio_getDWLE(data + 28);
                bmih->biClrUsed = ASFByteIO::asf_byteio_getDWLE(data + 32);
                bmih->biClrImportant = ASFByteIO::asf_byteio_getDWLE(data + 36);
                bmih->data = data + 40;//codec specify data is ???

                if (bmih->biSize > datalen) {
                        ALOGV("Invalid bitmapinfoheader data length, truncating!\n");
                        bmih->biSize = datalen;
                }

                break;
        }
        case GUID_STREAM_TYPE_COMMAND:
                stream->type = ASF_STREAM_TYPE_COMMAND;
                break;
        default:
                stream->type = ASF_STREAM_TYPE_UNKNOWN;
                break;
        }

        return 0;
}





int
ASFParser::asf_parse_header_extended_stream_properties(asf_stream_t *stream,
                                                                    uint8_t *objdata,
                                                                    uint32_t objsize)
{
        asf_stream_extended_t ext;
        uint32_t datalen;
        uint8_t *data;
        uint16_t flags;
        int i;

        ext.start_time = ASFByteIO::asf_byteio_getQWLE(objdata);
        ext.end_time = ASFByteIO::asf_byteio_getQWLE(objdata + 8);
        ext.data_bitrate = ASFByteIO::asf_byteio_getDWLE(objdata + 16);
        ext.buffer_size = ASFByteIO::asf_byteio_getDWLE(objdata + 20);
        ext.initial_buf_fullness = ASFByteIO::asf_byteio_getDWLE(objdata + 24);
        ext.data_bitrate2 = ASFByteIO::asf_byteio_getDWLE(objdata + 28);
        ext.buffer_size2 = ASFByteIO::asf_byteio_getDWLE(objdata + 32);
        ext.initial_buf_fullness2 = ASFByteIO::asf_byteio_getDWLE(objdata + 36);
        ext.max_obj_size = ASFByteIO::asf_byteio_getDWLE(objdata + 40);
        ext.flags = ASFByteIO::asf_byteio_getDWLE(objdata + 44);
        ext.stream_num = ASFByteIO::asf_byteio_getWLE(objdata + 48);
        ext.lang_idx = ASFByteIO::asf_byteio_getWLE(objdata + 50);
        ext.avg_time_per_frame = ASFByteIO::asf_byteio_getQWLE(objdata + 52);
        ext.stream_name_count = ASFByteIO::asf_byteio_getWLE(objdata + 60);
        ext.num_payload_ext = ASFByteIO::asf_byteio_getWLE(objdata + 62);

        datalen = objsize - 88;
        data = objdata + 64;

        /* iterate through all name strings */
        for (i=0; i<ext.stream_name_count; i++) {
                uint16_t strlen;

                if (datalen < 4) {
                        return ASF_ERROR_INVALID_VALUE;
                }

                strlen = ASFByteIO::asf_byteio_getWLE(data + 2);
                if (strlen > datalen) {
                        return ASF_ERROR_INVALID_LENGTH;
                }

                /* skip the current name string */
                data += 4 + strlen;
                datalen -= 4 + strlen;
        }

        /* iterate through all extension systems */
        for (i=0; i<ext.num_payload_ext; i++) {
                uint32_t extsyslen;

                if (datalen < 22) {
                        return ASF_ERROR_INVALID_VALUE;
                }

                extsyslen = ASFByteIO::asf_byteio_getDWLE(data + 18);
                if (extsyslen > datalen) {
                        return ASF_ERROR_INVALID_LENGTH;
                }

                /* skip the current extension system */
                data += 22 + extsyslen;
                datalen -= 22 + extsyslen;
        }

        if (datalen > 0) {
                asf_guid_t guid;

                ALOGV("hidden stream properties object found!\n");

                /* this is almost same as in stream properties handler */
                if (datalen < 78) {
                        return ASF_ERROR_OBJECT_SIZE;
                }

                /* check that we really have a stream properties object */
                ASFByteIO::asf_byteio_getGUID(&guid, data);
                if (asf_guid_get_type(&guid) != GUID_STREAM_PROPERTIES) {
                        return ASF_ERROR_INVALID_OBJECT;
                }
                if (ASFByteIO::asf_byteio_getQWLE(data + 16) != datalen) {
                        return ASF_ERROR_OBJECT_SIZE;
                }

                flags = ASFByteIO::asf_byteio_getWLE(data + 72);

                if ((flags & 0x7f) != ext.stream_num || stream->type) {
                        /* only one stream object per stream allowed and
                         * stream ids have to match with both objects*/
                        return ASF_ERROR_INVALID_OBJECT;
                } else {
                        int ret;

                        stream->flags |= ASF_STREAM_FLAG_HIDDEN;
                        ret = asf_parse_header_stream_properties(stream,
                                                                 data + 24,
                                                                 datalen);

                        if (ret < 0) {
                                return ret;
                        }
                }
        }

        stream->extended = (asf_stream_extended_t *)calloc(1, sizeof(asf_stream_extended_t));
        if (!stream->extended) {
                return ASF_ERROR_OUTOFMEM;
        }
        stream->flags |= ASF_STREAM_FLAG_EXTENDED;
        memcpy(stream->extended, &ext, sizeof(ext));

        return 0;
}

//add by qian


int
ASFParser::asf_parse_extended_content_description(asf_obj_extended_content_description_t *extended_content_description,
                                                  uint8_t *objdata,
                                                  uint32_t objsize)
{

        uint32_t idx=0;
        uint16_t cur_des_name_len=0;        
        uint16_t* cur_des_name_unicode;//unicode
        char* cur_des_name_char;

        uint8_t* cur_des_pos;
        uint16_t cur_des_data_len=0;
        uint16_t cur_des_data_type=0;
        void* cur_des_data=0;

        uint32_t  wm_pic_len=0;

		uint16_t cur_des_left_len=0; 
		uint8_t* cur_des_max_addr=0; 

		cur_des_left_len = objsize;
		cur_des_max_addr= objdata+objsize;
        
        
        extended_content_description->extended_content_num = ASFByteIO::asf_byteio_getWLE(objdata);
        //ALOGE("------asf_parse_extended_content_description:num=%d- cur_des_left_len=%d-cur_des_start_addr=0x%x---\n",
	//		          extended_content_description->extended_content_num,cur_des_left_len,cur_des_max_addr);

        cur_des_pos = objdata+2;
		cur_des_left_len = cur_des_max_addr - cur_des_pos;
            
        for(idx=0;idx<extended_content_description->extended_content_num;idx++){
			
			cur_des_left_len = cur_des_max_addr - cur_des_pos;
			if(cur_des_left_len < 2)
			{
				ALOGE("error 1:cur_des_left_len=%d",cur_des_left_len);
				return 0;	
			}	

			cur_des_name_len = ASFByteIO::asf_byteio_getWLE(cur_des_pos);  
			cur_des_pos=cur_des_pos+2;

			cur_des_left_len = cur_des_max_addr - cur_des_pos;
			if(cur_des_left_len < cur_des_name_len)
			{
				ALOGE("error 2:cur_des_left_len=%d cur_des_name_len=%d",cur_des_left_len,cur_des_name_len);
				return 0;	
			}	

            // ALOGE("cur_des_name_len=%d",cur_des_name_len);

			
             cur_des_name_char=asf_utf8_from_utf16le(cur_des_pos, cur_des_name_len);
            // ALOGE("cur_des_name = %s\n",cur_des_name_char);             
             cur_des_pos = cur_des_pos+cur_des_name_len;//*2;//wchar


			cur_des_left_len = cur_des_max_addr - cur_des_pos;
			if(cur_des_left_len < 4)
			{
				if(cur_des_name_char) 
				{
					free(cur_des_name_char);
				}	
				ALOGE("error 3:cur_des_left_len=%d",cur_des_left_len);
				return 0;	
			}	

             cur_des_data_type =ASFByteIO::asf_byteio_getWLE(cur_des_pos); 
             cur_des_pos=cur_des_pos+2;

             cur_des_data_len = ASFByteIO::asf_byteio_getWLE(cur_des_pos); 
             cur_des_pos=cur_des_pos+2;
 
               
             if(!strncmp(cur_des_name_char, "WM/Picture", cur_des_name_len)){     
      
                 
                uint16_t find_null_times=0;
                uint16_t finding_null_now=0;
                
                uint16_t i=0;
                uint8_t* cur_pos;

                uint16_t wchar_mime_type_len=0;
                char* wchar_mime_type=NULL; 

                uint16_t wchar_description_len=0;
                char* wchar_description=NULL;

                uint8_t picType=0;
                uint32_t picDataLen=0;
    

                /*
               // method 1: most simplest
               
                //picType 1 byte
                picType = *(cur_des_pos+i);
                ALOGE("picType=%d\n",picType);

                //picDataLen 4 byte
                 picDataLen = ASFByteIO::asf_byteio_getDWLE(cur_des_pos);  
                 ALOGE("picDataLen=%d,i=%d\n",picDataLen,i);

                 
                 cur_des_pos = cur_des_pos +(cur_des_data_len - picDataLen) ;
                 cur_des_data_len = picDataLen;  
                
                */             

                
                //picType 1 byte
                picType = *(cur_des_pos+i);
               // ALOGE("picType=%d\n",picType);
                cur_des_pos++;
                cur_des_data_len--;

                //picDataLen 4 byte
                 picDataLen = ASFByteIO::asf_byteio_getDWLE(cur_des_pos);  
                // ALOGE("picDataLen=%d\n",picDataLen);
                 cur_des_pos = cur_des_pos +4 ;
                 cur_des_data_len = cur_des_data_len-4;  

                 //wchar_mime_type
                for(i = 0; i < cur_des_data_len;i++){
                    cur_pos=(cur_des_pos+i);
                    if((*cur_pos) == 0x00 && (*(cur_pos+1)) == 0x00 && (i%2==0)){
                        i=i+2;//right shift 2 byte for "\0\0"                        
                        wchar_mime_type_len = i;
                        wchar_mime_type = asf_utf8_from_utf16le(cur_des_pos, wchar_mime_type_len);
                       // ALOGE("wchar_mime_type_len =%d,wchar_mime_type = %s\n",wchar_mime_type_len,wchar_mime_type);  
                        free(wchar_mime_type);
                        break;
                    }
                }
                cur_des_pos = cur_des_pos+wchar_mime_type_len;
                cur_des_data_len = cur_des_data_len-wchar_mime_type_len; 
                
                //pic description 2 byte
                
                cur_des_pos = cur_des_pos+2;
                cur_des_data_len = cur_des_data_len-2;           

                
                //pic data
                extended_content_description->extended_content_wm_pic_len=cur_des_data_len;
                extended_content_description->extended_content_wm_pic = cur_des_pos;
             
              /*  FILE *fp1 = fopen("/sdcard/pic.bin", "wb+");
                if(fp1)
                {
                    fwrite((void*)(cur_des_pos), 1, cur_des_data_len, fp1);
                    fclose(fp1);
                }     */
                
             }
             // ALOGE("extended_content_description cur_des_data_type =%d,data len  =%d\n",cur_des_data_type,cur_des_data_len);   
             if(cur_des_name_char) {
                    free(cur_des_name_char);
             }

             switch(cur_des_data_type){
                case 0:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//*2;
                    break;
                case 1:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//;
                    break;
                case 2:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//*4;
                    break;
                case 3:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//*4;
                    break;
                case 4:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//*8;
                    break;
                case 5:
                    cur_des_pos=cur_des_pos+cur_des_data_len;//*2;
                    break;
                default:
                    ALOGI("extended_content_description data tyep is error:cur_des_data_type=%d\n",cur_des_data_type);
                    return -1;
                     break;                  
             }                

        }
      
        return 0;      

}

//add by qian
int
ASFParser::asf_parse_index_parameters(asf_obj_index_parameters_t *index_parameters,
                                                  uint8_t *objdata,
                                                  uint32_t objsize)
{
       index_parameters->index_entry_time_interval = ASFByteIO::asf_byteio_getDWLE(objdata);
       index_parameters->index_specifiers_count = ASFByteIO::asf_byteio_getWLE(objdata+4);
       
       ALOGI("---asf_parse_index_parameters---\n");
       ALOGI("---index_entry_time_interval=%d---\n",index_parameters->index_entry_time_interval);
       ALOGI("---index_specifiers_count=%d---\n",index_parameters->index_specifiers_count);

    
       index_parameters->index_specifiers_entry = (asf_index_specifiers_t*)malloc(index_parameters->index_specifiers_count * sizeof(asf_index_specifiers_t));

       for(int i=0;i<index_parameters->index_specifiers_count;i++){
            ((asf_index_specifiers_s*)index_parameters->index_specifiers_entry)->stream_num =ASFByteIO::asf_byteio_getWLE((objdata+6)+4*i);
            ((asf_index_specifiers_s*)index_parameters->index_specifiers_entry)->index_type =ASFByteIO::asf_byteio_getWLE((objdata+8)+4*i);
            
            ALOGI("---index_specifiers_entry[%d].stream_num=%d,index_specifiers_entry[%d].type=%d---\n",
                        i,((asf_index_specifiers_s*)index_parameters->index_specifiers_entry)->stream_num,
                        i,((asf_index_specifiers_s*)index_parameters->index_specifiers_entry)->index_type);
       }
       return 0;
}

/**
 * Reads the file properties object contents to the asf_file_t structure,
 * parses the useful values from stream properties object to the equivalent
 * stream properties info structure and validates that all known header
 * subobjects have only legal values.
 */
int
ASFParser::asf_parse_header_validate(asf_object_header_t *header)
{
        /* some flags for mandatory subobjects */
        int fileprop = 0, streamprop = 0;
        asfint_object_t *current;

        if (header->first) {
                current = header->first;
                while (current) {
                        uint64_t size = current->size;

                        switch (current->type) {
                        case GUID_FILE_PROPERTIES:
                        {
                                uint32_t max_packet_size;
                                if (size < 104)
                                        return ASF_ERROR_OBJECT_SIZE;

                                if (fileprop) {
                                        /* multiple file properties objects not allowed */
                                        return ASF_ERROR_INVALID_OBJECT;
                                }

                                fileprop = 1;
                                ASFByteIO::asf_byteio_getGUID(&file->file_id, current->data);
                                file->file_size = ASFByteIO::asf_byteio_getQWLE(current->data + 16);
                                file->creation_date = ASFByteIO::asf_byteio_getQWLE(current->data + 24);
                                file->data_packets_count = ASFByteIO::asf_byteio_getQWLE(current->data + 32);
                                file->play_duration = ASFByteIO::asf_byteio_getQWLE(current->data + 40);
                                file->send_duration = ASFByteIO::asf_byteio_getQWLE(current->data + 48);
                                file->preroll = ASFByteIO::asf_byteio_getQWLE(current->data + 56);
				   file->real_duration = 	 (file->play_duration/10000 -file->preroll)>0?  (file->play_duration/10000 -file->preroll):0;
                                file->flags = ASFByteIO::asf_byteio_getDWLE(current->data + 64);
                                file->packet_size = ASFByteIO::asf_byteio_getDWLE(current->data + 68);//min
                                file->max_bitrate = ASFByteIO::asf_byteio_getQWLE(current->data + 76);

                                max_packet_size = ASFByteIO::asf_byteio_getDWLE(current->data + 72);//max
                                if (file->packet_size != max_packet_size) {
                                        /* in ASF file minimum packet size and maximum
                                         * packet size have to be same apparently...
                                         * stupid, eh? */
                                        return ASF_ERROR_INVALID_VALUE;
                                }
                                break;
                        }
                        case GUID_STREAM_PROPERTIES:
                        {
                                uint16_t flags;
                                asf_stream_t *stream;
                                int ret;

                                if (size < 78)
                                        return ASF_ERROR_OBJECT_SIZE;

                                streamprop = 1;
                                flags = ASFByteIO::asf_byteio_getWLE(current->data + 48);
                                stream = &file->streams[flags & 0x7f];

                                if (stream->type) {
                                 //this value has been set means this stream has a STREAM_PROPERTIES already
                                        /* only one stream object per stream allowed */
                                        return ASF_ERROR_INVALID_OBJECT;
                                }

                                ret = asf_parse_header_stream_properties(stream,
                                                                         current->data,
                                                                         size);

                                if (ret < 0) {
                                        return ret;
                                }
                                break;
                        }
                        case GUID_CONTENT_DESCRIPTION:
                        {
                                uint32_t stringlen = 0;

                                if (size < 34)
                                        return ASF_ERROR_OBJECT_SIZE;

                                stringlen += ASFByteIO::asf_byteio_getWLE(current->data);
                                stringlen += ASFByteIO::asf_byteio_getWLE(current->data + 2);
                                stringlen += ASFByteIO::asf_byteio_getWLE(current->data + 4);
                                stringlen += ASFByteIO::asf_byteio_getWLE(current->data + 6);
                                stringlen += ASFByteIO::asf_byteio_getWLE(current->data + 8);

                                if (size < stringlen + 34) {
                                        /* invalid string length values */
                                        return ASF_ERROR_INVALID_LENGTH;
                                }
                                break;
                        }
                        //add by qian
                        case GUID_CONTENT_BRANDING:                            
                              break;
                        case GUID_CONTENT_ENCRYPTION:
                        {
                            file->hasDRMObj= true;
                            ALOGE("------[ASF_ERROR]this file is encrypted!!!-----");                            
                        }
                            break;
                        case GUID_MARKER:
                                break;
                        case GUID_CODEC_LIST:
                                if (size < 44)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_STREAM_BITRATE_PROPERTIES:
                                if (size < 26)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_PADDING:
                                break;
                        case GUID_EXTENDED_CONTENT_DESCRIPTION: //album-art info
                         {       if (size < 26)
                                        return ASF_ERROR_OBJECT_SIZE;
                                /*
                                int ret=0;
                                asf_obj_extended_content_description_t* pcontent_description=NULL;

                                file->header->extended_content_description = (asf_obj_extended_content_description_t*)calloc(1, sizeof(asf_obj_extended_content_description_t));
                                if (!file->header->extended_content_description ){
                                    ALOGE("[ASF_ERROR]parser GUID_EXTENDED_CONTENT_DESCRIPTION ASF_ERROR_OUTOFMEM\n");
                                    return ASF_ERROR_OUTOFMEM;
                                }
                                ALOGE("parser extended_content_description=0x%08x\n",(uint32_t)file->header->extended_content_description);        
                                pcontent_description = (asf_obj_extended_content_description_t *)file->header->extended_content_description;
                                
                                ret = asf_parse_extended_content_description(pcontent_description,
                                                                       current->data,
                                                                       size);
                                if (ret < 0) {
                                        ALOGE("[ASF_ERROR]parser GUID_EXTENDED_CONTENT_DESCRIPTION error\n");
                                        return ret;
                                }
                                
                       */
                                break;
                          }//must has the {}, else the variables in the case will cause build error       
                        case GUID_UNKNOWN:
                                /* unknown guid type */
                                break;
                        default:
                                /* identified type in wrong place */
                                return ASF_ERROR_INVALID_OBJECT;
                               
                        }

                        current = current->next;
                }
        }

        if (header->ext) {
                current = header->ext->first;
                while (current) {
                        uint64_t size = current->size;

                        switch (current->type) {
                        case GUID_METADATA:
                                if (size < 26)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_LANGUAGE_LIST:
                                if (size < 26)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_EXTENDED_STREAM_PROPERTIES:
                        {
                                uint16_t stream_num;
                                asf_stream_t *stream;
                                int ret;

                                if (size < 88)
                                        return ASF_ERROR_OBJECT_SIZE;

                                stream_num = ASFByteIO::asf_byteio_getWLE(current->data + 48);
				    if(stream_num >= ASF_MAX_STREAMS)
						return ASF_ERROR_INVALID_VALUE;
					
                                stream = &file->streams[stream_num];

                                ret = asf_parse_header_extended_stream_properties(stream,
                                                                                  current->data,
                                                                                  size);

                                if (ret < 0) {
                                        return ret;
                                }
                                break;
                        }
                        //Index Parameters Object 
                        case GUID_INDEX_PARAMETERS_OBJECT:
                        {
                        
                            if (size < 30)
                                        return ASF_ERROR_OBJECT_SIZE;

                                 
                        
                                int ret=0;
                                asf_obj_index_parameters_t* index_parameters=NULL;

                                file->header->index_parameters = (asf_obj_index_parameters_t*)calloc(1, sizeof(asf_obj_index_parameters_t));
                                if (!file->header->index_parameters ){
                                    ALOGE("[ASF_ERROR]parser GUID_INDEX_PARAMETERS_OBJECT ASF_ERROR_OUTOFMEM\n");
                                    return ASF_ERROR_OUTOFMEM;
                                }
                                ALOGI("parser asf_obj_index_parameters_t=0x%08x\n",(uint32_t)file->header->index_parameters);        
                                index_parameters = (asf_obj_index_parameters_t *)file->header->index_parameters;
                                
                                ret = asf_parse_index_parameters(index_parameters,
                                                                       current->data,
                                                                       size);
                                if (ret < 0) {
                                        ALOGE("[ASF_ERROR]parser GUID_INDEX_PARAMETERS_OBJECT error\n");
                                        return ret;
                                }
                                break;
                        }

                           

                        
                        case GUID_ADVANCED_MUTUAL_EXCLUSION:
                                if (size < 42)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_STREAM_PRIORITIZATION:
                                if (size < 26)
                                        return ASF_ERROR_OBJECT_SIZE;
                                break;
                        case GUID_UNKNOWN:
                                /* unknown guid type */
                                break;
                        default:
                                /* identified type in wrong place */
                                break;
                        }

                        current = current->next;
                }
        }

        if (!fileprop || !streamprop || !header->ext) {
                /* mandatory subobject missing */
                return ASF_ERROR_INVALID_OBJECT;
        }

        return 1;
}

/**
 * Destroy the header and all subobjects
 */
void
ASFParser::asf_free_header(asf_object_header_t *header)
{
        if (!header)
                return;

        if (header->first) {
                asfint_object_t *current = header->first;
                asfint_object_t *next = NULL;
                while (current) {
                        next = current->next;
                        free(current);
                        current = next;
                }
        }
/*      
//add by qian -->
        if(header->extended_content_description){
            ALOGE("asf_free_header:extended_content_description=0x%08x\n",(uint32_t)header->extended_content_description);
            free(header->extended_content_description);
        }
*/
        if(header->index_parameters){
			
            if(header->index_parameters->index_specifiers_entry){
                ALOGI("asf_free_header:index_specifiers_entry=0x%08x\n",(uint32_t)header->index_parameters->index_specifiers_entry);
                free(header->index_parameters->index_specifiers_entry);
            }
            ALOGI("asf_free_header:index_parameters=0x%08x\n",(uint32_t)header->index_parameters);
            free(header->index_parameters);
        }
            
//add by qian <--
        if (header->ext) {
                asfint_object_t *current = header->ext->first, *next;
                while (current) {
                        next = current->next;
                        free(current);
                        current = next;
                }
                free(header->ext);
        }
        if (header->data) free(header->data);
        free(header);
}

/**
 * Allocates a metadata struct and parses the contents from
 * the header object raw data. All strings are in UTF-8 encoded
 * format. The returned struct needs to be freed using the
 * asf_header_metadata_destroy function. Returns NULL on failure.
 */


/*
enum ASFType{
	ASF_WCHAR,
	ASF_BYTE,
	ASF_BOOL,
	ASF_DWORD,
	ASF_QWORD,
	ASF_WORD,
	ASF_GUID,
}
*/
asf_metadata_t *
ASFParser::asf_header_metadata(asf_object_header_t *header)
{
        asfint_object_t *current = NULL;
        asf_metadata_t *ret = NULL;

        /* allocate the metadata struct */
        ret = (asf_metadata_t*)calloc(1, sizeof(asf_metadata_t));
        if (!ret) {
                 ALOGE("asf_header_metadata:error 1");
                return NULL;
        }

        current = asf_header_get_object(header, GUID_CONTENT_DESCRIPTION);
        if (current) {
                char *str = NULL;
                uint16_t strlen;
                int i, read = 0;
		  
                /* The validity of the object is already checked so we can assume
                 * there's always enough data to read and there are no overflows */
                ret->content_count = 5;
                ret->content = (asf_metadata_entry_t*)calloc(ret->content_count, sizeof(asf_metadata_entry_t));
                if(!ret->content)
                {
                        free(ret);
                         ALOGE("asf_header_metadata:error 2");
                        return NULL;
                }
                
                for (i=0; i<5; i++) {
                        strlen = ASFByteIO::asf_byteio_getWLE(current->data + i*2);
                        if (!strlen || strlen>current->datalen)
                        {
                                ALOGE("error in parse %d content strlen=%d",i,strlen);
                                continue;
                        }
                        str = asf_utf8_from_utf16le(current->data + 10 + read, strlen);
                        read += strlen;
                        ret->content[i].value_type = 0;

                        switch (i) {
                        case 0:
                                ret->content[i].key = (char*)calloc(6, sizeof(char));
                                sprintf(ret->content[i].key, "%s","Title");                                
                                ret->content[i].value = str;
                                ret->content[i].size = 6;                               
                               
                                break;
                        case 1:
                                ret->content[i].key = (char*)calloc(7, sizeof(char));
                                sprintf(ret->content[i].key, "%s","Author");                                
                                ret->content[i].value = str;
                                ret->content[i].size = 7;
                                break;
                        case 2:
                                 ret->content[i].key = (char*)calloc(10, sizeof(char));
                                sprintf(ret->content[i].key, "%s","Copyright");                                
                                ret->content[i].value = str;
                                ret->content[i].size = 10;
                                break;
                        case 3:
                                ret->content[i].key = (char*)calloc(12, sizeof(char));
                                sprintf(ret->content[i].key, "%s","Description");                                
                                ret->content[i].value = str;
                                ret->content[i].size = 12;//Memory corrupt 
                                break;
                        case 4:
                                 ret->content[i].key = (char*)calloc(7, sizeof(char));
                                sprintf(ret->content[i].key, "%s","Rating");                                
                                ret->content[i].value = str;
                                ret->content[i].size = 7;
                                break;
                        default:
                                free(str);
                                break;
                        }
                        ALOGI("ret->content[%d].key=%s, value=%s",i,ret->content[i].key,ret->content[i].value);

                }
        }

        current = asf_header_get_object(header, GUID_EXTENDED_CONTENT_DESCRIPTION);
        if (current) {
                int i, j, position;

                ret->extended_count = ASFByteIO::asf_byteio_getWLE(current->data);
                ret->extended = (asf_metadata_entry_t*)calloc(ret->extended_count, sizeof(asf_metadata_entry_t));
                if (!ret->extended) {
                        /* Clean up the already allocated parts and return */
                        for (i=0; i<ret->content_count; i++) 
			 {
		                if (ret->content[i].key) free(ret->content[i].key);
		                if (ret->content[i].value) free(ret->content[i].value);
		         }
		          if (ret->content) free(ret->content);
                        
                        free(ret);

                        return NULL;
                }

                position = 2;
                for (i=0; i<ret->extended_count; i++) {
                    uint16_t length, type;
                    
                    length = ASFByteIO::asf_byteio_getWLE(current->data + position);
                    position += 2;
                    if(((uint16_t)position + length)> current->datalen){                    
                         ALOGE("error in extended object key length = %d, tatal object length = %lld",length,current->datalen);
                         continue;
                    }
                    ret->extended[i].key = asf_utf8_from_utf16le(current->data + position, length);
                    position += length;
                    

                    type = ASFByteIO::asf_byteio_getWLE(current->data + position);
                    position += 2;
                    ret->extended[i].value_type = type;
                    
                    length = ASFByteIO::asf_byteio_getWLE(current->data + position);
                    position += 2;			
                    ret->extended[i].size = length;
                    ALOGI("ret->extended[%d].key=%s,data_length=%d",i,ret->extended[i].key,ret->extended[i].size);
                        
                    if (!(ret->extended[i].size) || ((uint16_t)position + length)>current->datalen)
                    {
                         ALOGE("error in extended object value length = %d, tatal object length = %lld",length,(current->datalen));
                         ret->extended[i].value=NULL;
                         //  position += length;
                         continue;
                    }
                        switch (type) {
                        case 0:
                                /* type of the value is a string */
                                ret->extended[i].value = asf_utf8_from_utf16le(current->data + position, length);
                                break;
                        case 1:
                                 //type of the value is a data block 
                                /*ret->extended[i].value = (char*)calloc((length*2 + 1), sizeof(char));
                                
                                for (j=0; j<length; j++) {
                                        //static const char hex[16] = "0123456789ABCDEF";
                                        static const char hex[] = "0123456789ABCDEF";
                                        ret->extended[i].value[j*2+0] = hex[current->data[position+j]>>4];
                                        ret->extended[i].value[j*2+1] = hex[current->data[position+j]&0x0f];
                                }
                                ret->extended[i].value[j*2] = '\0';
                                */
                                ret->extended[i].value = (char*)calloc((length + 1), sizeof(char));
                                for (j=0; j<length; j++) {
                                        ret->extended[i].value[j] =  current->data[position+j] ;
                                }
                                ret->extended[i].value[j] = '\0';
                                
                                break;
                        case 2:
                                /* type of the value is a boolean */
                                ret->extended[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->extended[i].value, "%s",
                                        *current->data ? "true" : "false");
                                break;
                        case 3:
                                /* type of the value is a signed 32-bit integer */
                                ret->extended[i].value = (char*)calloc(11, sizeof(char));
                                sprintf(ret->extended[i].value, "%u",
                                        ASFByteIO::asf_byteio_getDWLE(current->data + position));
                                break;
                        case 4:
                                /* FIXME: This doesn't print whole 64-bit integer */
                                ret->extended[i].value = (char*)calloc(21, sizeof(char));
                                sprintf(ret->extended[i].value, "%u",
                                        (uint32_t) ASFByteIO::asf_byteio_getQWLE(current->data + position));
                                break;
                        case 5:
                                /* type of the value is a signed 16-bit integer */
                                ret->extended[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->extended[i].value, "%u",
                                        ASFByteIO::asf_byteio_getWLE(current->data + position));
                                break;
                        default:
                                /* Unknown value type... */
                                ret->extended[i].value = NULL;
                                break;
                        }
                        position += length;
                }
				
        }

	//meta data

	 current = asf_header_ext_get_object(header, GUID_METADATA);
        if (current) {
                int i,position;
				uint32_t j;

                ret->metadata_count = ASFByteIO::asf_byteio_getWLE(current->data);
                ret->metadata = (asf_metadata_entry_t*)calloc(ret->metadata_count, sizeof(asf_metadata_entry_t));
                if (!ret->metadata) {
                        /* Clean up the already allocated parts and return */
                         for (i=0; i<ret->content_count; i++) 
			 {
		                if (ret->content[i].key) free(ret->content[i].key);
		                if (ret->content[i].value) free(ret->content[i].value);
		         }
		          if (ret->content) free(ret->content);
                        
			  for (i=0; i<ret->extended_count; i++) 
			 {
		                if (ret->extended[i].key) free(ret->extended[i].key);
		                if (ret->extended[i].value) free(ret->extended[i].value);
		         }
		          if (ret->extended) free(ret->extended);
			   
                        free(ret);

                        return NULL;
                }

                position = 2;
                for (i=0; i<ret->metadata_count; i++) {
                        uint32_t  reserve, name_len, data_length, type;

		          reserve = ASFByteIO::asf_byteio_getDWLE(current->data + position);
                        position += 4; //reserve+stream number
			   

                        name_len = ASFByteIO::asf_byteio_getWLE(current->data + position);
                        position += 2;

                        ret->metadata[i].key = asf_utf8_from_utf16le(current->data + position+6, name_len);                        
			

                        type = ASFByteIO::asf_byteio_getWLE(current->data + position);
                        position += 2;

                         ret->metadata[i].value_type = type;

                       data_length = ASFByteIO::asf_byteio_getDWLE(current->data + position);
                        position += 4 ;
						
			   position += name_len ;
                        ret->metadata[i].size = data_length;
			   ALOGI("ret->metadata[%d].key=%s,data_length=%d",i,ret->metadata[i].key,data_length);
                        if (!(ret->metadata[i].size) || ret->metadata[i].size>current->datalen)
                        {
                                ALOGE("error in parse %d metadata datalen=%d",i,data_length);
                                  ret->metadata[i].value=NULL;
                                   position += data_length;
                                continue;
                        }
                        
                        switch (type) {
                        case 0:
                                /* type of the value is a string */
                                ret->metadata[i].value = asf_utf8_from_utf16le(current->data + position, data_length);
                                break;
                        case 1:
                               //  type of the value is a data block
                               // ret->metadata[i].value = (char*)calloc((data_length*2 + 1), sizeof(char));//Memory corrupt 
                               /* 
                               for (j=0; j<data_length; j++) {
                                        //static const char hex[16] = "0123456789ABCDEF";
                                        static const char hex[] = "0123456789ABCDEF";
                                        ret->metadata[i].value[j*2+0] = hex[current->data[position+j]>>4];
                                        ret->metadata[i].value[j*2+1] = hex[current->data[position+j]&0x0f];
                                }
                                ret->metadata[i].value[j*2] = '\0' ;
                                */

                                ret->metadata[i].value = (char*)calloc((data_length + 1), sizeof(char));
                                for (j=0; j<data_length; j++) {
                                        ret->metadata[i].value[j] =  current->data[position+j] ;
                                }
                                ret->metadata[i].value[j] = '\0';////Memory corrupt 

                                
                                break;
                        case 2:
                                /* type of the value is a boolean */
                                ret->metadata[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->metadata[i].value, "%s",
                                        *current->data ? "true" : "false");
                                break;
                        case 3:
                                /* type of the value is a signed 32-bit integer */
                                ret->metadata[i].value = (char*)calloc(11, sizeof(char));
                                sprintf(ret->metadata[i].value, "%u",
                                        ASFByteIO::asf_byteio_getDWLE(current->data + position));
                                break;
                        case 4:
                                /* FIXME: This doesn't print whole 64-bit integer */
                                ret->metadata[i].value = (char*)calloc(21, sizeof(char));
                                sprintf(ret->metadata[i].value, "%u",
                                        (uint32_t) ASFByteIO::asf_byteio_getQWLE(current->data + position));
                                break;
                        case 5:
                                /* type of the value is a signed 16-bit integer */
                                ret->metadata[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->metadata[i].value, "%u",
                                        ASFByteIO::asf_byteio_getWLE(current->data + position));
                                break;
                        default:
                                /* Unknown value type... */
                                ret->metadata[i].value = NULL;
                                break;
                        }
                        position += data_length;
                }
				
        }


        	//metadate library data

	 current = asf_header_ext_get_object(header, GUID_METADATA_LIBRARY);
        if (current) {
                int i, j, position;

                ret->metadatalib_count = ASFByteIO::asf_byteio_getWLE(current->data);
                ret->metadatalib = (asf_metadata_entry_t*)calloc(ret->metadatalib_count, sizeof(asf_metadata_entry_t));
                if (!ret->metadatalib) 
                {
                        /* Clean up the already allocated parts and return */
                         for (i=0; i<ret->content_count; i++) 
			 {
		                if (ret->content[i].key) free(ret->content[i].key);
		                if (ret->content[i].value) free(ret->content[i].value);
		         }
		          if (ret->content) free(ret->content);

                        
			   for (i=0; i<ret->extended_count; i++) 
			 {
		                if (ret->extended[i].key) free(ret->extended[i].key);
		                if (ret->extended[i].value) free(ret->extended[i].value);
		         }
		          if (ret->extended) free(ret->extended);

			    for (i=0; i<ret->metadata_count; i++) 
			 {
		                if (ret->metadata[i].key) free(ret->metadata[i].key);
		                if (ret->metadata[i].value) free(ret->metadata[i].value);
		         }
		          if (ret->metadata) free(ret->metadata);
			   
                        free(ret);

                        return NULL;
                }

                position = 2;
                for (i=0; i<ret->metadatalib_count; i++) {
                        uint32_t  reserve, name_len, data_length, type;

		          reserve = ASFByteIO::asf_byteio_getDWLE(current->data + position);
                        position += 4; //language+stream number
			   

                        name_len = ASFByteIO::asf_byteio_getWLE(current->data + position);
                        position += 2;

                        ret->metadatalib[i].key = asf_utf8_from_utf16le(current->data + position+6, name_len);
                         
			  

                        type = ASFByteIO::asf_byteio_getWLE(current->data + position);
                        position += 2;

                        ret->metadatalib[i].value_type = type;

                       data_length = ASFByteIO::asf_byteio_getDWLE(current->data + position);
                        position += 4 ;
                        ret->metadatalib[i].size = data_length;
						
			   position += name_len ;
			   ALOGI("ret->metadatalib[%d].key=%s,data_length=%d",i,ret->metadatalib[i].key,data_length);
                         if (!(ret->metadatalib[i].size) || ret->metadatalib[i].size>current->datalen)
                        {
                                ALOGE("error in parse %d metadatalib datalen=%d",i,data_length);
                                  ret->metadatalib[i].value=NULL;
                                  position += data_length;
                                 continue;
                        }
                        switch (type) {
                        case 0:
                                /* type of the value is a string */
                                ret->metadatalib[i].value = asf_utf8_from_utf16le(current->data + position, data_length);
                                break;
                        case 1:
                                // type of the value is a data block 
                              //  ret->metadatalib[i].value = (char*)calloc((data_length*2 + 1), sizeof(char)); //Memory corrupt 
                               /* 
                               for (j=0; j<data_length; j++) {
                                        //static const char hex[16] = "0123456789ABCDEF";
                                        static const char hex[] = "0123456789ABCDEF";
                                        ret->metadatalib[i].value[j*2+0] = hex[current->data[position+j]>>4];
                                        ret->metadatalib[i].value[j*2+1] = hex[current->data[position+j]&0x0f];
                                }
                                ret->metadatalib[i].value[j*2] = '\0';
                                */
					            ALOGE("data_length=%d,sizeof(char)=%d,sizeof(uint8)=%d",data_length,sizeof(char),sizeof(uint8_t));
                                ret->metadatalib[i].value = (char*)calloc((data_length + 1), sizeof(char));
                                for (j=0; j<data_length; j++) {
                                        ret->metadatalib[i].value[j] =  current->data[position+j] ;
                                }
					            ALOGE("j=%d",j);
                                ret->metadatalib[i].value[j] = '\0';//Memory corrupt 

                                
                                break;
                        case 2:
                                /* type of the value is a boolean */
                                ret->metadatalib[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->metadatalib[i].value, "%s",
                                        *current->data ? "true" : "false");
                                break;
                        case 3:
                                /* type of the value is a signed 32-bit integer */
                                ret->metadatalib[i].value = (char*)calloc(11, sizeof(char));
                                sprintf(ret->metadatalib[i].value, "%u",
                                        ASFByteIO::asf_byteio_getDWLE(current->data + position));
                                break;
                        case 4:
                                /* FIXME: This doesn't print whole 64-bit integer */
                                ret->metadatalib[i].value = (char*)calloc(21, sizeof(char));
                                sprintf(ret->metadatalib[i].value, "%u",
                                        (uint32_t) ASFByteIO::asf_byteio_getQWLE(current->data + position));
                                break;
                        case 5:
                                /* type of the value is a signed 16-bit integer */
                                ret->metadatalib[i].value = (char*)calloc(6, sizeof(char));
                                sprintf(ret->metadatalib[i].value, "%u",
                                        ASFByteIO::asf_byteio_getWLE(current->data + position));
                                break;
                        default:
                                /* Unknown value type... */
                                ret->metadatalib[i].value = NULL;
                                break;
                        }
                        position += data_length;
                }
				
        }

	//GUID_CONTENT_BRANDING
	
        return ret;
}

/**
 * Free the metadata struct and all fields it includes
 */
void
ASFParser::asf_header_free_metadata(asf_metadata_t *metadata)
{
        int i;

        if (!metadata)
                return;

         for (i=0; i<metadata->content_count; i++) 
	 {
                if (metadata->content[i].key) 
					free(metadata->content[i].key);
						 
                if (metadata->content[i].value)  
					free(metadata->content[i].value);
         }
          if (metadata->content) free(metadata->content);
                  
        for (i=0; i<metadata->extended_count; i++) 
	{
                if (metadata->extended[i].key) free(metadata->extended[i].key);
                if (metadata->extended[i].value) free(metadata->extended[i].value);
        }
        if (metadata->extended) free(metadata->extended);
	

	for (i=0; i<metadata->metadata_count; i++) 
	{
	    if (metadata->metadata[i].key) free(metadata->metadata[i].key);
	    if (metadata->metadata[i].value) free(metadata->metadata[i].value);
	}
	if (metadata->metadata) free(metadata->metadata);

	for (i=0; i<metadata->metadatalib_count; i++) 
	{
	    if (metadata->metadatalib[i].key) free(metadata->metadatalib[i].key);
	    if (metadata->metadatalib[i].value) free(metadata->metadatalib[i].value);
	}
	if (metadata->metadatalib) free(metadata->metadatalib);
	
        free(metadata);
}
