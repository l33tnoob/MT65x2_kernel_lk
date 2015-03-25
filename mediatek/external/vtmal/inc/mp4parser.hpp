/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _VTMAL_VIDEO_PARSER_H_

#define _VTMAL_VIDEO_PARSER_H_

/*
#ifdef __cplusplus
extern "C"
    {
#endif
*/
#include "compile_ops.hpp"
#include "dec_enc_common.h"
#include "sw_types.h"
#include <utils/Log.h>


#define VO_START_CODE		      0x8
#define VO_START_CODE_MIN	      0x100
#define VO_START_CODE_MAX	      0x11f

#define VOL_START_CODE		      0x12
#define VOL_START_CODE_MIN	      0x120
#define VOL_START_CODE_MAX	      0x12f

#define VOS_START_CODE           0x1b0
#define VOS_END_CODE             0x1b1
#define USR_START_CODE           0x1b2
#define GOP_START_CODE           0x1b3
#define VSO_START_CODE           0x1b5
#define VOP_START_CODE	         0x1b6
#define STF_START_CODE           0x1c3 // stuffing_start_code
#define SHV_START_CODE           0x020
#define SHV_END_MARKER           0x03f

#ifdef VT_ENABLE_LOG_FILTER
	#define VTMAL_PARSE_ERR  _E("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
#else
	#define VTMAL_PARSE_ERR  LOGE("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);

#endif




/* these information is obtained from bitstream parser */

typedef struct
   {	

	// Frame width
    kal_uint16 width;
    // Frame height
    kal_uint16 height;
    // This is an 8-bit integer used to signal the profile and level identification
    kal_uint8 Profile_and_Level;
    // 0: intra coded(I), 01: predictive-coded (P)
    kal_uint8 vop_coding_type;
    // Used inverse quantisation type to inverse quantisation of the DCT coefficients.
    kal_uint8 quant_type;
    // '1' indicates that the macroblock data is rearranged differently, specifically, 
    // motion vector data is separated from the texture data (i.e., DCT coefficients)
    kal_uint8 data_partitioned;
    // '1' indicates that the reversible variable length tables 
    // (Table B?23, Table B?24 and Table B?25) should be used
    kal_uint8 resversible_vlc;
    // H.263 or not
    kal_uint8 H263;
    // H.263 or not
    kal_uint8 short_video_header;
    // vop_fcode_forward, values from 1 to 7; the value of zero is forbidden. 
    // It is used in decoding of motion vectors.
    kal_uint8 fcode;
    // Signals the value of the parameter rounding_control used for pixel 
    // value interpolation in motion compensation for P-VOPs. 
    kal_uint8 rounding_code;
    // Specify the absolute value of quantiser scale to be used for inverse quantising the
    // macroblock until updated by any subsequent dquant, dbquant, or quant_scale.
    kal_uint8 vop_quant;
    // The value of this internal flag is set to 1 when the values of intra_dc_thr 
    // and the DCT quantiser indicate the usage of the intra DC VLCs as shown in Table 6?21.
    // Otherwise, the value of this flag is set to 0.
    kal_uint8 intra_dc_vlc_thr;
    // vop_time_increment_resolution, indicates the number of evenly spaced subintervals, 
    // called ticks, within one modulo time. One modulo time represents the fixed interval 
    // of one second. The value zero is forbidden.
    kal_uint16 time_increment_resolution;
    // This value represents the absolute vop_time_increment from the 
    // synchronization point marked by the modulo_time_base measured 
    // in the number of clock ticks. It can take a value in the range
    // of [0,vop_time_increment_resolution)
    kal_uint16 vop_time_increment;
    // old value of vop time increment
    kal_uint16 old_vop_time_increment;	
    // when set to '0' indicates that no subsequent data exists for the VOP
    kal_uint8 vop_coded;
    // time base
    kal_uint8 time_base;
    // temp temp base
    kal_uint8 old_time_base;
    // 1' indicates that there is no resync_marker in coded VOPs
    kal_uint8 resync_marker_disable;
    // =1, may contain interlaced video. =0, are non-interlanced video.
    kal_uint8 interlaced;
    // The value of this internal flag is set to 1 when the values of intra_dc_thr 
    // and the DCT quantiser indicate the usage of the intra DC VLCs as shown in Table 6?21. 
    // Otherwise, the value of this flag is set to 0. 
    kal_uint8 use_intra_dc_vlc;
    // =log2(hdr.height * hdr.width/(16*16)) 
    kal_uint8 mblength;
    // =Log2(vop_time_increment)
    kal_uint8 time_increment_length;
    kal_bool b_fixed_vop_rate;	
    kal_uint32 fixed_vop_time_increment;	
    kal_uint32 time_increment;	

    kal_uint32 temporal_reference;
    kal_uint32 old_temporal_reference;

								 
   } VOS_INFO;



kal_int32 mpeg4_parse_vos(VOS_INFO * hdr, kal_uint8 * data, kal_int32 bitcnt, kal_uint32 max_parse_data_size);
kal_int32 mpeg4_util_show_bits(kal_uint8 * data, kal_int32 bitcnt, kal_int32 num);
kal_int32 mpeg4_util_user_data(kal_uint8 * data, kal_int32 * bitcnt, kal_uint32 max_parse_data_size);
kal_int32 mpeg4_util_get_bits(kal_uint8 * data, kal_int32 *bitcnt, kal_int32 num);
kal_int32 mpeg4_util_show_word(kal_uint8 * a);
kal_int32 mpeg4_util_log2ceil(kal_int32 arg);



/*
#ifdef __cplusplus
    }
#endif
*/

#endif /*_VTMAL_VIDEO_PARSER_H_*/
