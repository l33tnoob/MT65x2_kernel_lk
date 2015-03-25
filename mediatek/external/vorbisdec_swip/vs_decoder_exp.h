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

#ifndef VS_DECODER_EXP_H
#define VS_DECODER_EXP_H

typedef void* vs_decoder_handle;

void
vs_decoder_get_mem_size(int * internal_mem_in_bytes,
                        int * dsp_buf_mem_in_bytes,
                        int * setup_mem_in_bytes,
                        int * pcm_buf_and_decode_working_in_bytes);

vs_decoder_handle   /* return 0 if fail */
vs_decoder_init(unsigned char * p_internal_mem,
                unsigned char * p_dsp_buf_mem,
                unsigned char * p_setup_mem,
                void *reserved);

int /* return <0 if fail */
vs_decoder_construct_id_header(vs_decoder_handle h_vs,
                               const unsigned char * p_id_header,
                               int id_header_len_in_byte,
                               int *channel,        /* output channel */
                               int *sample_rate,    /* output sample rate */
                               void *reserved);

int /* return <0 if fail */
vs_decoder_construct_comment_header(vs_decoder_handle h_vs,
                                    const unsigned char * p_comment_header,
                                    int comment_header_len_in_byte,
                                    void *reserved);

int /* return <0 if fail */
vs_decoder_construct_setup_header(vs_decoder_handle h_vs,
                                  const unsigned char * p_setup_header,
                                  int setup_header_len_in_byte,
                                  void *reserved);

int /* return <0 if fail */
vs_decoder_decode_audio_packet(vs_decoder_handle h_vs,
                               const unsigned char * p_audio_packet,
                               int audio_packet_len_in_byte,
                               unsigned char * p_working_buffer);

int /* return <0 if fail, otherwise refers to the bytes in output pcm buffer */
vs_decoder_get_pcm_output(vs_decoder_handle h_vs,
                          unsigned char * p_pcm_out);/* output pcm buffer */

int /* return the 32-bit checksum */
vs_decoder_get_checksum(vs_decoder_handle h_vs);

int /* return 32-bit version */
vs_decoder_get_version(void);

int /* return -1 as fail */
vs_decoder_restart(vs_decoder_handle h_vs);
#endif // VS_DECODER_EXP_H
