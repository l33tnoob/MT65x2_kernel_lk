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
#include <stdio.h>
#include "asfparser.h"

#define ASF_DEBUG_LOGI(x, ...) LOGI("[ASFParser] "x,  ##__VA_ARGS__)
#define ASF_DEBUG_LOGV(x, ...) LOGV("[ASFParser] "x,  ##__VA_ARGS__)
#define ASF_DEBUG_LOGE(x, ...) //LOGE("[ASFParser] "x,  ##__VA_ARGS__)

/******************************************************************************
**
**      ASF Parser APIs
**
******************************************************************************/  
ASFParser::ASFParser(void* source, asf_io_read_func_ptr read, asf_io_write_func_ptr write, asf_io_seek_func_ptr seek)
{
        //initialize all member variables
        file = NULL;
        iError = ASF_SUCCESS;
        
        // initialize asf stream
        asf_iostream_t stream;
        stream.read = read;
        stream.write = write;
        stream.seek = seek;
        stream.source = source;

        //Initialize asf parser library 
        file = asf_open_file(&stream);
        if (!file)
        {
                ALOGE("Error failed to Initialize ASF parser");
                iError = ASF_INSUFFICIENT_DATA; //check for current error
        }       
}

ASFParser::~ASFParser()
{
        //release all the memory
        asf_close();
}

ASFErrorType ASFParser::IsAsfFile()
{
        int status;
        status = asf_init();
        if (status < 0)
                return ASF_FILE_HDR_READ_ERR;

        return ASF_SUCCESS;
}

ASFErrorType ASFParser::ParseAsfFile()
{
        int status;
        status = asf_init();
        if (status < 0)
                return ASF_FILE_HDR_READ_ERR;

        return ASF_SUCCESS;
}


asf_file_t *
ASFParser::asf_open_file(asf_iostream_t *pStream)
{
        asf_file_t *asffile = NULL;
/*        
        asf_iostream_t stream;
        
        stream.read = ASFFileIO::asf_fileio_read_cb;
        stream.write = ASFFileIO::asf_fileio_write_cb;
        stream.seek = ASFFileIO::asf_fileio_seek_cb;
        stream.pStreamFp = fstream;
*/
        asffile = asf_open_cb(pStream);
        if (!asffile)
                return NULL;

        asffile->filename = NULL;//filename;

        return asffile;
}

asf_file_t *
ASFParser::asf_open_cb(asf_iostream_t *iostream)
{
        asf_file_t *asffile= NULL;
        int i;

        if (!iostream)
                return NULL;

        asffile = (asf_file_t *)calloc(1, sizeof(asf_file_t));
        if (!asffile)
                return NULL;

        asffile->index_parsed=false;

        asffile->filename = NULL;
        asffile->iostream.read = iostream->read;
        asffile->iostream.write = iostream->write;
        asffile->iostream.seek = iostream->seek;
        asffile->iostream.source = iostream->source;

        asffile->header = NULL;
        asffile->data = NULL;
        asffile->simple_index = NULL;
        asffile->index = NULL;
        //asffile->index = NULL;
        

        for (i=0; i < ASF_MAX_STREAMS; i++) {
                asffile->streams[i].type = ASF_STREAM_TYPE_NONE;
                asffile->streams[i].flags = ASF_STREAM_FLAG_NONE;
                asffile->streams[i].properties = NULL;
                asffile->streams[i].extended = NULL;
                asffile->streams[i].current_packet = 0;
        }
        asffile->hasDRMObj=false;

       
        return asffile;
}

int
ASFParser::asf_init()
{
        int tmp;

        if (!file)
                return ASF_ERROR_INTERNAL;

        tmp = asf_parse_header();
        if (tmp < 0) {
                ALOGE("error parsing header: %d", tmp);
                return tmp;
        }
        file->position += tmp;
        file->data_position = file->position;

        tmp = asf_parse_data();
        if (tmp < 0) {
                ALOGE("error parsing data object: %d", tmp);
                return tmp;
        }
        file->position += tmp;

        if (file->flags & ASF_FLAG_SEEKABLE && file->iostream.seek) {//can seek && has seek cb
                int64_t seek_position;
                ALOGE("data_position=%lld,file->data->size=%lld\n",file->data_position,file->data->size);
                file->index_position = file->data_position +
                                       file->data->size;

                //go to index pos to parse idx
                seek_position = file->iostream.seek(file->iostream.source,
                                                    file->index_position);

                /* if first seek fails, we can try to recover and just ignore seeking */
                if (seek_position >= 0) {
                    /*
                        while (seek_position == (int64_t)file->index_position &&
                               file->index_position < file->file_size && !file->index_parsed) {
                               //let this func parse all index
                                tmp = asf_parse_index();
                                if (tmp < 0) {
                                        ALOGV("Error finding index object! %d\n", tmp);
                                        break;
                                }

                                // The object read was something else than index  
                                if (!file->simple_index)
                                        file->index_position += tmp;

                                seek_position = file->iostream.seek(file->iostream.source,
                                                                          file->index_position);
                        }*/
                        //for index object, we should seek to the end of file.
                        uint64_t next_index_position,tmp_position_inc;
                        next_index_position = file->index_position ;
                        tmp_position_inc=0;
                        
                        while(next_index_position < file->file_size){
                             ALOGI("asf_parse_index next_index_position=%lld\n",next_index_position);

                            tmp = asf_parse_index(next_index_position);
							if (tmp < 0)
							{			
								
								ALOGI("asf_parse_index,tmp=%d\n", tmp);
								break;
							 }
                            tmp_position_inc = tmp_position_inc + tmp;
                            next_index_position = file->index_position + tmp_position_inc;//tmp is the parsed index size

							//if 1 indx error, should parse the next index
                            //if (tmp < 0 ||(tmp>=0 && next_index_position >=  file->file_size)) {
                            //       break;//can not resumed error
                            //}
                            //parse the next index obj
                            seek_position = file->iostream.seek(file->iostream.source,
                                                                next_index_position);
                        }
						ALOGI("asf_parse_index :next_index_position=%lld, file_size=%lld\n",next_index_position,file->file_size);


                        if (!file->simple_index && !file->index) {
                                ALOGE("Error, Couldn't find an index object\n");
                                file->index_position = 0;
                        }

                        //after parse index, will reset the read position to the data->packets_position
                        seek_position = file->iostream.seek(file->iostream.source,
                                                          file->data->packets_position);
                        if (seek_position != (int64_t)file->data->packets_position) {
                                /* Couldn't seek back to packets position, this is fatal! */
                                return ASF_ERROR_SEEK;
                        }
                }
        }

        for (tmp = 0; tmp < ASF_MAX_STREAMS; tmp++) {
                if (file->streams[tmp].type != ASF_STREAM_TYPE_NONE) {
                        ALOGV("stream %d of type %d found!\n", tmp, file->streams[tmp].type);
                }
        }

        return 0;
}

int64_t ASFParser::asf_get_data_position()
{
	int64_t seek_position;
	seek_position = file->data_position;

	ALOGI("file->data_position=%lld, file->data->size=%lld\n",file->data_position,file->data->size);

	return seek_position;
}

void
ASFParser::asf_close()
{
        if (file) {
                int i;

                asf_free_header(file->header);
                if (file->data) free(file->data);
                if (file->simple_index) {

                        if (file->simple_index->entries) free(file->simple_index->entries);
                        free(file->simple_index);
                }
                if(file->index){
                    if(file->index->specifiers_entry) free(file->index->specifiers_entry);
                    if(file->index->index_block->index_entry) free(file->index->index_block->index_entry);
                    if(file->index->index_block) free(file->index->index_block);
                    free(file->index);

                }

                if (file->iostream.source)
				{
					//fclose (file->iostream.pStreamFp);
                        file->iostream.source = NULL;
				}

                for (i=0; i < ASF_MAX_STREAMS; i++) {
                        if (file->streams[i].properties) free(file->streams[i].properties);
                        if (file->streams[i].extended) free(file->streams[i].extended);
                }
                        

                free(file);
        }
}

asf_packet_t *
ASFParser::asf_packet_create()
{
        asf_packet_t *ret = NULL;

        ret = (asf_packet_t *)calloc(1, sizeof(asf_packet_t));
        if (!ret)
                return NULL;

        asf_data_init_packet(ret);

        return ret;
}

int
ASFParser::asf_get_stream_packet(asf_packet_t *packet, uint32_t aStreamID)
{
        uint64_t new_position;
        int64_t seek_position;
        int tmp;
        
        //check whether streamID is valid
        if (!file || file->streams[aStreamID].type == ASF_STREAM_TYPE_NONE){
                ALOGE("asf_get_stream_packet:error 1,ASF_STREAM_TYPE_NONE\n");
                return ASF_ERROR_INTERNAL;
        }
        
        /* calculate stream specific packet position */
        new_position = file->data->packets_position + file->streams[aStreamID].current_packet * file->packet_size;

        //seek to stream specific packets position
        if(file->position != new_position){
                seek_position = file->iostream.seek(file->iostream.source, new_position);
                if (seek_position < 0 || seek_position != (int64_t)new_position) {
                         ALOGE("asf_get_stream_packet:error 2,seek_position=%lld,new_position=%lld\n",seek_position,new_position);
                        return ASF_ERROR_SEEK;
                }
                file->position = new_position;
                file->packet = file->streams[aStreamID].current_packet;
                
        }
        //ALOGE("asf_get_stream_packet: aStreamID=%d,file->position=%lld,file->packet =%lld\n",aStreamID,file->position,file->packet );
        //Now we are at the required position get the packet
        if ((tmp = asf_get_packet(packet)) < 0) {
                ALOGE("asf_get_stream_packet: Error 3: %d getting packet\n", tmp);  
        }
        else{
                 ASF_DEBUG_LOGE("asf_get_stream_packet: stream %d current packet index is %lld\n",aStreamID, file->streams[aStreamID].current_packet);  
                 file->streams[aStreamID].current_packet++;               
        }
        
        return tmp;
                
}


int
ASFParser::asf_get_packet(asf_packet_t *packet)
{
        int tmp;
		//ALOGE("asf_get_packet file->packet=%lld",file->packet);

        if (!file || !packet)
                return ASF_ERROR_INTERNAL;

        if (file->packet >= file->data_packets_count) {
                ALOGE("asf_get_packet:error 1,file->packet=%lld,file->data_packets_count=%lld\n",file->packet,file->data_packets_count);
                return 0;
        }

        tmp = asf_data_get_packet(packet);
        if (tmp < 0) {
               ALOGE("asf_get_packet:error 2,tmp=%d\n",tmp);
                return tmp;
        }

        file->position += tmp;
        file->packet++;

        return tmp;
}

void
ASFParser::asf_packet_destroy(asf_packet_t *packet)
{
        if (!packet) return;
        asf_data_free_packet(packet);
        free(packet);
}

int64_t
ASFParser::asf_seek_to_msec(int64_t msec)
{
        uint64_t packet=0;
        uint64_t new_msec = msec- file->preroll;
        int k;
        bool seek_done=false;

        if (!file){
                ALOGE("asf_seek_to_msec:error 1\n");
                return ASF_ERROR_INTERNAL;
        }

        if (!(file->flags & ASF_FLAG_SEEKABLE) || !file->iostream.seek) {
                ALOGE("asf_seek_to_msec:error 2,flags=%d\n",file->flags);
                return ASF_ERROR_SEEKABLE;
        }



        for (k=0; k<ASF_MAX_STREAMS; k++) {
                if (file->streams[k].type == ASF_STREAM_TYPE_NONE)
                        continue;
                ASF_DEBUG_LOGE("asf_seek_to_msec:before seek:streams[%d].current_packet=%lld\n",k,file->streams[k].current_packet);
        }
        
        /* update current file position information */
        ASF_DEBUG_LOGE("asf_seek_to_msec::before seek:file->position=%lld,file->packet=%lld,file->data->packets_position=%lld\n",file->position,file->packet,file->data->packets_position);

        

        /* Index structure is missing, check if we can still seek */
        if (file->simple_index == NULL && file->index == NULL) {
            ALOGE("Error,this file has no index and simple index!!!\n");
                int i, audiocount;

                audiocount = 0;
                for (i=0; i<ASF_MAX_STREAMS; i++) {
                        if (file->streams[i].type == ASF_STREAM_TYPE_NONE)
                                continue;
                         /* Non-audio files are not seekable without index */
                        if (file->streams[i].type == ASF_STREAM_TYPE_AUDIO){
                                audiocount++;
                        }
                        
                }
                
                /* Audio files with more than one audio track are not seekable
                 * without index */
                if (audiocount != 1){
                         ALOGE("asf_seek_to_msec:error 4:audio count=%d\n",audiocount);
                        return ASF_ERROR_SEEKABLE;
                }
        }
        //real_duration is ms units 
        if (msec > (int64_t)(file->real_duration)) {
                 ALOGE("asf_seek_to_msec:error 5:real_duration=%lld\n",file->real_duration );
                return ASF_ERROR_SEEK;
        }

        if (file->simple_index) {
				msec= msec+file->preroll;//ms unit
                ALOGI("seek by  simple index!!!,reset seek time=%lld ms\n",msec);
                uint32_t simple_index_entry;

                /* Fetch current packet from index entry structure */
                /*entry_time_interval between each index entry in 100-nanosecond units*/
                simple_index_entry = msec * 10000 / file->simple_index->entry_time_interval;	// Morris Yang: index_entry is second unit
                //<--add by qian
                if (simple_index_entry >= file->simple_index->entry_count) {
                         ALOGE("asf_seek_to_msec:error 6 by simple index\n");
                         //should not return error, can seek by auido again
                        //return ASF_ERROR_SEEK;
                        seek_done=false;
                }
                else{
                    seek_done=true;

                    /* the correct msec time isn't known before reading the packet */
                    new_msec = msec- file->preroll;

                    //CR#ALPS00478782, avoid side effect: seek to 0 sec but progress time jump to 3 secs
                    if ( 0 == new_msec)
                    {
                        simple_index_entry = 0;
                    }
					
                    packet = file->simple_index->entries[simple_index_entry].packet_index;

                    ALOGI("asf_seek_to_msec:seek_done =%d,file->simple_index index_entry=%d,new_msec=%lld\n",(uint32_t)seek_done,simple_index_entry,new_msec );
                }
		 msec= msec-file->preroll;//reset seek time, or the next will error
       } 
       if((!seek_done) && file->index){
                msec= msec+ file->preroll;//ms unit
                ALOGI("seek by index!!!,reset seek time=%lld ms\n",msec);

                uint32_t  index_entry = msec / file->index->index_entry_time_interval;	//ms unit
                if (index_entry >= file->index->index_block->index_entry_count) {
                       ALOGE("asf_seek_to_msec:error 7 by index\n");
                       //return ASF_ERROR_SEEK;
                       //should not return error, can seek by auido again
                       seek_done=false;                         
                }
                else{
                    seek_done=true;
                    packet = (file->index->index_block->index_entry[index_entry].offset + file->index->index_block->block_positions)/file->packet_size;

                    /* the correct msec time isn't known before reading the packet */
                    new_msec = msec- file->preroll;
                    ALOGI("asf_seek_to_msec:seek_done=%d,file->index index_entry=%d,new_msec=%lld\n",(uint32_t)seek_done,index_entry,new_msec );
          
                }
		msec= msec-file->preroll;//reset seek time, or the next will error
         }
	     //3)bitrate + packet size
      /*  if((!seek_done) && (file->real_duration >0) && (file->packet_size >0)&&( file->max_bitrate>0))   {
                // convert msec into bytes per second and divide with packet_size  
                packet = msec * file->max_bitrate / 8000 / file->packet_size;			

                ///* calculate the resulting position in the audio stream  
                new_msec = packet * file->packet_size * 8000 / file->max_bitrate;

				if(packet < file->data_packets_count)//check if seek right
				{
					seek_done =true;
					ALOGE("seek done by bitrate+packet size calc");
				}
				ALOGE("seek in 3 packet=%lld  file->data_packets_count=%lld",packet,file->data_packets_count);
        }
		*///4)file size + packet size
		if((!seek_done) && (file->real_duration >0) && (file->packet_size >0)&&(file->data->size>0))  {
                /* convert msec into bytes per second and divide with packet_size */
                //
                //-->
                packet = (msec * file->data->size /(file->real_duration))/file->packet_size;	

                /* calculate the resulting position in the audio stream */
                //new_msec = packet * file->packet_size * (file->real_duration) /file->file_size;
                //-->
                new_msec = packet * file->packet_size * (file->real_duration) /file->data->size;

				if(packet > file->data_packets_count)//check if seek right
				{
					ALOGE("asf_seek_to_msec: finally, we can not find the right position packet=%lld,file->data_packets_count=%lld\n",packet,file->data_packets_count);
				}

				seek_done =true;//anyhow, should be true now
				ALOGI("seek done by time+packet size calc");
        }

        /* calculate new position to be in the beginning of the current frame */
        uint64_t new_position = file->data->packets_position + packet * file->packet_size;

        int64_t seek_position = file->iostream.seek(file->iostream.source, new_position);
        if (seek_position < 0 || seek_position != (int64_t)new_position) {
                 ALOGE("asf_seek_to_msec:error 7:seek_position=%lld,new_position=%lld\n",seek_position,new_position);
                return ASF_ERROR_SEEK;
        }
        //update current packet position for each stream 
        for (k=0; k<ASF_MAX_STREAMS; k++) {
                if (file->streams[k].type == ASF_STREAM_TYPE_NONE)
                        continue;
                file->streams[k].current_packet = packet;//all ==this packet???׼ô???
                ALOGI("asf_seek_to_msec:streams[%d].current_packet=%lld\n",k,packet);
        }
        
        /* update current file position information */
        file->position = new_position;
        file->packet = packet;
        ALOGI("asf_seek_to_msec:file->position=%lld,file->packet=%lld\n",file->position,file->packet);

        return new_msec;
}

asf_metadata_t *
ASFParser::asf_header_get_metadata()
{
        if (!file || !file->header)
                return NULL;

        return asf_header_metadata(file->header);
}


asf_metadata_entry_t* 
ASFParser::asf_findMetaValueByKey(asf_metadata_t*  meta, char* key,int  in_len)
{
	asf_metadata_t* ret=	meta;
	if(ret==NULL)
        {
            ALOGE("[ASF_ERROR]: no meta!");
            return NULL;
        }   
        ALOGI("content_count=%d, extended_count=%d, metadata_count=%d,ret->metadatalib_count=%d",
                    ret->content_count,ret->extended_count,ret->metadata_count,ret->metadatalib_count);
        
	 if(ret->content_count>0)
        {
            for(int i=0;i<ret->content_count;i++)
           {
                if(ret->content[i].key &&  !strncmp(key,ret->content[i].key, in_len))
                  { 
                  	  if(ret->content[i].value)
			          {
                         return  &(ret->content[i]);
                  	   }
                  }
            }
        }
        
        if(ret->extended_count>0)
        {
            for(int i=0;i<ret->extended_count;i++)
           {
                if(ret->extended[i].key && !strncmp(key,ret->extended[i].key, in_len))
                  { 
                      	if(ret->extended[i].value)
			          {
                         return  &(ret->extended[i]) ;
                      	}
                  }
            }
        }

        if(ret->metadata_count>0)
        {
            for(int i=0;i<ret->metadata_count;i++)
           {
                if(ret->metadata[i].key && !strncmp(key,ret->metadata[i].key, in_len))
                 { 
                        if(ret->metadata[i].value)
			          {
                        return  &(ret->metadata[i])  ;
                       	}
                 }
            }
        }

         if(ret->metadatalib_count>0)
        {
            for(int i=0;i<ret->metadatalib_count;i++)
           {
                 if(ret->metadatalib[i].key &&  !strncmp(key,ret->metadatalib[i].key, in_len))
                 { 
                       if(ret->metadatalib[i].value)
			          {
                        return  &(ret->metadatalib[i]);
                       	}
                 }
            }
        }
        
         return NULL;
        
}

/*
typedef struct _WMPicture {
        LPWSTR pwszMIMEType;  //end with '/0/0'
        BYTE   bPictureType;
        LPWSTR pwszDescription;
        DWORD  dwDataLen;
        BYTE   *pbData;
} WM_PICTURE;
-->this is error

right is :

typedef struct _WMPicture {
        BYTE   bPictureType;
        DWORD  dwDataLen;
        LPWSTR pwszMIMEType;
        LPWSTR pwszDescription;
        BYTE   *pbData;
} WM_PICTURE;

*/

void
ASFParser::asf_parse_WMPicture(uint8_t* WMPicture,uint32_t size, uint32_t* dataoff)
{

/*


        uint16_t find_null_times=0;
        uint16_t finding_null_now=0;

        uint16_t i=0;
        uint8_t* cur_pos;
        uint8_t* satrt_pos;
        uint8_t* search_pos;
        

        uint16_t wchar_mime_type_len=0;
        char* wchar_mime_type=NULL; 

        uint16_t wchar_description_len=0;
        char* wchar_description=NULL;

        uint8_t picType=0;
        uint32_t picDataLen=0;
        uint32_t cur_des_data_len=0;

        
        
        cur_pos = WMPicture;
        satrt_pos  = WMPicture;
        search_pos = WMPicture;
        cur_des_data_len = size;
        *dataoff=0;

        ALOGE("size=%d,datalen=%d,dataoff=%d",size,cur_des_data_len,*dataoff);
        
         //wchar_mime_type
        for(i = 0; i < cur_des_data_len;i++)
        {
            cur_pos=(search_pos+i);
            ALOGE("*cur_pos=0x%x,*cur_pos+1=0x%x",*cur_pos,*(cur_pos+1));
            if((*cur_pos) == 0x00 && (*(cur_pos+1)) == 0x00 && (i%2==0))
            {
                i=i+2;//right shift 2 byte for "\0\0"                        
                wchar_mime_type_len = i;
                wchar_mime_type = asf_utf8_from_utf16le(cur_pos, wchar_mime_type_len);
                 ALOGE("wchar_mime_type_len =%d,wchar_mime_type =%s  ",wchar_mime_type_len,wchar_mime_type);  
                free(wchar_mime_type);
                break;
            }
        }
        *dataoff =*dataoff+ i;
        cur_pos = satrt_pos +*dataoff;
         cur_des_data_len = size-*dataoff; 

        ALOGE("size=%d,datalen=%d,dataoff=%d",size,cur_des_data_len,*dataoff);
        
        picType = *(cur_pos);
       *dataoff =*dataoff+1 ;
        cur_pos = satrt_pos +*dataoff;       
        cur_des_data_len = size-*dataoff;
        ALOGE("size=%d,datalen=%d,dataoff=%d",size,cur_des_data_len,*dataoff);

        search_pos =cur_pos;
        //wchar_des_type
        for(i = 0; i < cur_des_data_len;i++){
            cur_pos=(search_pos+i);
            if((*cur_pos) == 0x00 && (*(cur_pos+1)) == 0x00 && (i%2==0)){
                i=i+2;//right shift 2 byte for "\0\0"                        
                wchar_description_len = i;
                wchar_description = asf_utf8_from_utf16le(cur_pos, wchar_description_len);
                 ALOGE("wchar_description_len =%d,wchar_description = %s",wchar_description_len,wchar_description);  
                free(wchar_description);
                break;
            }
        }   

        
        *dataoff =*dataoff+ i;
         cur_pos = satrt_pos +*dataoff;       
        cur_des_data_len = size-*dataoff; 

         ALOGE("size=%d,datalen=%d,dataoff=%d",size,cur_des_data_len,*dataoff);

        //picDataLen 4 byte
         picDataLen = ASFByteIO::asf_byteio_getDWLE(cur_pos);  
        // ALOGE("picDataLen=%d\n",picDataLen);
       *dataoff =*dataoff+ 4;
         cur_pos = satrt_pos +*dataoff;       
        cur_des_data_len = size-*dataoff;  

         ALOGE("size=%d,datalen=%d,dataoff=%d,picDataLen=%d",size,cur_des_data_len,*dataoff,picDataLen);
*/
            uint8_t picType=0;
            uint32_t picDataLen=0;
            uint32_t cur_des_data_len=0;
            uint8_t* cur_pos = WMPicture;
            *dataoff = 0;
            
            picType = *(cur_pos);
            cur_pos = cur_pos+1;

           picDataLen = ASFByteIO::asf_byteio_getDWLE(cur_pos);  
           if(picDataLen<size)
           {
               *dataoff = size - picDataLen;
               ALOGI("picType=%d,picDataLen=%d,off=%d\n",picType,picDataLen,*dataoff);
           }
           else
           {
                *dataoff = 0;
                ALOGE("Error: asf_parse_WMPicture:picDataLen=%d",picDataLen);
           }

        
}


void
ASFParser::asf_header_destroy()
{
        if (!file)
                return;

        asf_free_header(file->header);
        file->header = NULL;
}

void
ASFParser::asf_metadata_destroy(asf_metadata_t *metadata)
{
        if (metadata)
                asf_header_free_metadata(metadata);
}

uint8_t
ASFParser::asf_get_stream_count()
{
        uint8_t ret = 0;
        int i;

        if (!file)
                return 0;

        for (i = 0; i < ASF_MAX_STREAMS; i++) {
                if (file->streams[i].type != ASF_STREAM_TYPE_NONE)
                        ret = i;//???why not ret++???
        }

        return ret;
}

int
ASFParser::asf_is_broadcast() {
        if (!file)
                return 0;

        return (file->flags & ASF_FLAG_BROADCAST);
}

/*

int
ASFParser::asf_is_seekable() {
    if (!file)
            return 0;

        //return (file->flags & ASF_FLAG_SEEKABLE);
}
*/



int
ASFParser::asf_is_seekable() {
    if (!file)
            return 0;

        //return (file->flags & ASF_FLAG_SEEKABLE);
    if(!(file->flags & ASF_FLAG_SEEKABLE)) {
         ALOGE("asf_is_seekable:error 1:!(file->flags & ASF_FLAG_SEEKABLE)\n");
        return 0;
    }

    else{//flags=ASF_FLAG_SEEKABLE
        if (file->simple_index == NULL) {
        int i, audiocount;

            audiocount = 0;
            for (i=0; i<ASF_MAX_STREAMS; i++) {
                    if (file->streams[i].type == ASF_STREAM_TYPE_NONE)
                            continue;

                    // Non-audio files are not seekable without index 
                     if (file->streams[i].type == ASF_STREAM_TYPE_AUDIO){
                                audiocount++;
                     }
            }

            // Audio files with more than one audio track are not seekable
            // without index 
            if (audiocount != 1){
                   ALOGE("asf_is_seekable:warning!!! more than one audio track are not seekable without index\n");
            //       return 0;
            }
        }
    }   
    return 1;
}

asf_stream_t *
ASFParser::asf_get_stream(uint8_t track)
{
        if (!file || track >= ASF_MAX_STREAMS)
                return NULL;

        return &file->streams[track];
}

uint64_t
ASFParser::asf_get_file_size()
{
        if (!file)
                return 0;

        return file->file_size;
}

uint64_t
ASFParser::asf_get_creation_date()
{
        if (!file)
                return 0;

        return file->creation_date;
}

uint64_t
ASFParser::asf_get_data_packets()
{
        if (!file)
                return 0;

        return file->data_packets_count;
}

uint64_t
ASFParser::asf_get_duration()
{
        if (!file)
                return 0;
        
        return (file->real_duration);// ms unit
}


uint32_t
ASFParser::asf_get_max_bitrate()
{
        if (!file)
                return 0;

        return file->max_bitrate;
}

uint32_t
ASFParser::asf_get_packet_size()
{
        if (!file)
                return 0;

        return file->packet_size;
}

int
ASFParser::asf_get_track_num(asf_stream_type_t type)
{
        int ret = 0;
        int i;

        if (!file)
                return 0;

        for (i = 0; i < ASF_MAX_STREAMS; i++) {
                if (file->streams[i].type == type)
                {
                        ret = i;
                        break;
                }
        }

        return ret;
}
//add by qian

uint64_t
ASFParser::asf_get_preroll_ms()
{
        if (!file)
                return 0;
        
        return file->preroll; //ms
}


uint8_t
ASFParser::asf_check_simple_index_obj()
{
        if (!file)
                return 0;
        
        else if(file->simple_index) //ms
                return 1;
        else
                return 0;
         
}
/*
asf_obj_extended_content_description_t*  
ASFParser::asf_get_extended_content_description(){

    if (!file)
        return NULL;
    else if(!file->header)
        return NULL;
    else if(!file->header->extended_content_description)
         return NULL;
    else
         return file->header->extended_content_description;       

}
*/
uint8_t 
ASFParser::asf_parse_check_hasDRM()
{
    if (file->hasDRMObj)
                return 1;
        else
                return 0;
}


