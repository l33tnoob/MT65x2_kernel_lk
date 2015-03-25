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

#ifndef ASFPARSER_H_INCLUDED
#define ASFPARSER_H_INCLUDED


//----------------------------------------------------------------------
// Include Files
//----------------------------------------------------------------------
#ifndef ASF_H
#include "asf.h"
#endif

#ifndef ASFINT_H
#include "asfint.h"
#endif

#ifndef BYTEIO_H
#include "byteio.h"
#endif

#include <cutils/log.h>
#undef LOG_TAG
#define LOG_TAG "AsfParser"
//----------------------------------------------------------------------
// Global Type Declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Global Constant Declarations
//----------------------------------------------------------------------
#define CALLBACK_BUFFERSIZE ASF_MAX_DATA_REQUESTED*2
#define MIN_HeaderObject_Size   30              // Valid Header Object Size is at least 30 bytes

//----------------------------------------------------------------------
// Global Data Declarations
//----------------------------------------------------------------------

enum METADATATAGS
{
    ASF_TITLE,
    ASF_AUTHOR,
    ASF_COPYRIGHT,
    ASF_DESCRIPTION,
    ASF_RATING
};



enum ASFType{
	ASF_WCHAR,
	ASF_BYTE,
	ASF_BOOL,
	ASF_DWORD,
	ASF_QWORD,
	ASF_WORD,
	ASF_GUID,

};

//======================================================================
//  CLASS DEFINITIONS and FUNCTION DECLARATIONS
//======================================================================


/**
 *  @brief The ASFParser Class is the class that parses the
 *  ASF file.
 */

typedef int32_t (*asf_io_read_func_ptr)(void *aSource, void *aBuffer, int32_t aSize);
typedef int32_t (*asf_io_write_func_ptr)(void *aSource, void *aBuffer, int32_t aSize);
typedef int64_t (*asf_io_seek_func_ptr)(void *aSource, int64_t aOffset);


class ASFParser
{
public:
        /**
          * @brief Constructor.
          *
          * @param Filehandle
          * @returns None
          */
        ASFParser(void* source, asf_io_read_func_ptr read, asf_io_write_func_ptr write, asf_io_seek_func_ptr seek);

        /**
          * @brief Destructor.
          *
          * @param None
          * @returns None
          */
        ~ASFParser();

        /**
          * @brief Checks the file is valid ASF clip or not
          *
          * @returns error type.
          */
        ASFErrorType IsAsfFile();

        /**
          * @brief Parses the MetaData (beginning or end) and positions
          * the file pointer at the first audio frame.
          *
          * @returns error type.
          */
        ASFErrorType    ParseAsfFile();

        /*ASF Interface functions*/
        /* initialize the library using file on a local filesystem */
        asf_file_t *asf_open_file(asf_iostream_t *pStream);

        /* initialize the library using callbacks defined on a stream structure,
           the stream structure can be freed after calling this function */
        asf_file_t *asf_open_cb(asf_iostream_t *iostream);

        /* close the library handle and free all allocated memory */
        void asf_close();

		/* set seek point to data position */
		int64_t asf_get_data_position();
		
        /* initialize the library and read all header information of the ASF file */
        int asf_init();


        /* create a packet structure for reading data packets */
        asf_packet_t *asf_packet_create();

        /* free the packet structure allocated earlier, need to be called only once */
        void asf_packet_destroy(asf_packet_t *packet);

        /* get next packet from the stream to the specified packet structure of specific stream */
        int asf_get_stream_packet(asf_packet_t *packet,uint32_t aStreamID);

        /* get next packet from the stream to the specified packet structure */
        int asf_get_packet(asf_packet_t *packet);

        /* seek to the closest (key frame) packet specified by milliseconds position */
        int64_t asf_seek_to_msec(int64_t msec);


        /* get metadata information of the ASF file handle */
        asf_metadata_t *asf_header_get_metadata();

	 asf_metadata_entry_t*  asf_findMetaValueByKey(asf_metadata_t*  meta,char* key,int in_len);
	void  asf_parse_WMPicture(uint8_t* WMPicture, uint32_t size, uint32_t* dataoff);
	
        /* free metadata structure received from the library */
        void asf_metadata_destroy(asf_metadata_t *metadata);

        /* free all header information from the ASF file structure
         * WARNING: after calling this function all asf_header_*
         *          functions will return NULL or failure!!! */
        void asf_header_destroy();


        /* calculate how many streams are available in current ASF file */
        uint8_t asf_get_stream_count();

        /* get info of a stream, the resulting pointer and its contents should NOT be freed */
        asf_stream_t *asf_get_stream(uint8_t track);


        /* return non-zero if the file is broadcasted, 0 otherwise */
        int asf_is_broadcast();

        /* return non-zero if the file is seekable, 0 otherwise */
        int asf_is_seekable();

        /* get size of the ASF file in bytes */
        uint64_t asf_get_file_size();

        /* get creation date in 100-nanosecond units since Jan 1, 1601 GMT
           this value should be ignored for broadcasts */
        uint64_t asf_get_creation_date();

        /* get number of data packets available in this file
           this value should be ignored for broadcasts */
        uint64_t asf_get_data_packets();

        /* get play duration of the file in millisecond units,
           this value should be ignored for broadcasts */
        uint64_t asf_get_duration();


        /* maximum bitrate as bits per second in the entire file */
        uint32_t asf_get_max_bitrate();

        uint32_t asf_get_packet_size();

        /* get audio/video track ID if available in current ASF file 
        return 0 if audio/video track missing  */
        int asf_get_track_num(asf_stream_type_t type);

        //add by qian-->
        
        /* get play prerol time of of the file in millisecond units*/
        uint64_t asf_get_preroll_ms();
        
        uint8_t asf_check_simple_index_obj();

       // asf_obj_extended_content_description_t* asf_get_extended_content_description();

        uint8_t asf_parse_check_hasDRM();
        //<--

private:
        /*ASF Payload parse handling functions*/

        /*Initialize and allocate a payload structure*/
        void asf_data_init_packet(asf_packet_t *packet);
        /*get actual contents of payload*/
        int asf_data_get_packet(asf_packet_t *packet);
        /*release the memory allocated by asf_data_init_packet*/
        void asf_data_free_packet(asf_packet_t *packet);

        int asf_data_read_packet_data(asf_packet_t *packet, uint8_t flags,
                                                uint8_t *data, uint32_t len);
        int asf_data_read_payload_data(asf_payload_t *payload, uint8_t flags, 
                                                uint8_t *data, int size);
        int asf_data_read_payloads(asf_packet_t *packet,uint64_t preroll,uint8_t multiple,
                       uint8_t type,uint8_t flags,uint8_t *data,uint32_t datalen);

        /*GUID Handling functions*/
        int asf_guid_match(const asf_guid_t *guid1, const asf_guid_t *guid2);
        guid_type_t asf_guid_get_object_type(const asf_guid_t *guid);
        guid_type_t asf_guid_get_stream_type(const asf_guid_t *guid);
        guid_type_t asf_guid_get_type(const asf_guid_t *guid);

        /*ASF header and Metadata handling functions*/
        int asf_parse_header_validate(asf_object_header_t *header);
        void asf_free_header(asf_object_header_t *header);
        asf_metadata_t *asf_header_metadata(asf_object_header_t *header);
        void asf_header_free_metadata(asf_metadata_t *metadata);
        asfint_object_t * asf_header_get_object(asf_object_header_t *header, const guid_type_t type);
        asfint_object_t * asf_header_ext_get_object(asf_object_header_t *header, const guid_type_t type);
        int asf_parse_header_stream_properties(asf_stream_t *stream,
                                                        uint8_t *objdata,uint32_t objsize);
        int asf_parse_header_extended_stream_properties(asf_stream_t *stream,
                                                                    uint8_t *objdata,uint32_t objsize);
        int asf_parse_index_parameters(asf_obj_index_parameters_t *index_parameters,
                                                  uint8_t *objdata,
                                                  uint32_t objsize);
        /*ASF top level mandatory objects parsing function*/
        int asf_parse_header();
        int asf_parse_data();
        int asf_parse_index(int64_t next_index_position);
        int asf_parse_index_simple_index();
        int asf_parse_index_index();
        
        void asf_parse_read_object(asfint_object_t *obj, uint8_t *data);
        int asf_parse_headerext(asf_object_headerext_t *header, uint8_t *buf, uint64_t buflen);

        char *asf_utf8_from_utf16le(uint8_t *buf, uint16_t buflen);
        //add by qian
        int asf_parse_extended_content_description(asf_obj_extended_content_description_t *extended_content_description,
                                                   uint8_t *objdata,
                                                uint32_t objsize);

protected:
        //PVFile * fp;

        //ASF context structure 
        asf_file_t *    file;

        ASFErrorType iError;
};

#endif // #ifdef ASFPARSER_H_INCLUDED
