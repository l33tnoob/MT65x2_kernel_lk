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

#ifndef OV_DECODER_EXP_H
#define OV_DECODER_EXP_H

#define OV_EXP_HOLE     (-3)
#define OV_EXP_BREAK    (-4)

typedef struct {
  int (*read_func)  (void *ptr, int bytes, void *datasource);
  int (*seek_func)  (void *datasource, int offset);
} ov_callbacks;

typedef void* ov_decoder_handle;

void
ov_decoder_get_mem_size(int * internal_mem_in_bytes,
                        int * dsp_buf_mem_in_bytes,
                        int * setup_mem_in_bytes,
                        int * parser_mem_in_bytes,
                        int * pcm_buf_in_bytes,
                        int * bitstream_buf_in_bytes);

ov_decoder_handle
ov_decoder_init(ov_callbacks callbacks,
                void *       f,
                int          file_size_in_bytes,
                int          cal_checksum,
                unsigned char * p_internal_mem,
                unsigned char * p_dsp_buf_mem,
                unsigned char * p_setup_mem,
                unsigned char * p_parser_mem,
                int * sampling_rate,
                int * channels,
                int * total_duration_in_samples,
                void *reserved);

int
ov_decoder_decode_packet(   ov_decoder_handle h_ov_decoder,
                            unsigned char *   p_pcm_buffer,
                            const unsigned char *   p_bs_buffer,
                            int               bytes_in_bs_buffer);

int
ov_decoder_seek_by_sample(ov_decoder_handle h_ov_decoder,
                          ov_callbacks      callbacks,
                          void *            f,
                          int               samples_to_seek,
                          int*              bytes_to_seek);

int
ov_decoder_get_checksum(ov_decoder_handle h_ov_decoder);

extern int ov_scan_last_page(   ov_callbacks callback,
                                void *f,
                                int file_size_in_bytes,
                                int *fpos_out,
                                int *gpos_out,
                                int *ch,
                                int *srate,
                                unsigned char *p_working_buffer);   /*  the same as parser mem,
                                                                        plz query it via get_mem_size API */

#endif // OV_DECODER_EXP_H
