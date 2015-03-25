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

#ifndef MINVORBIS_ENCODER_EXP_H
#define MINVORBIS_ENCODER_EXP_H

#ifdef __cplusplus
extern "C"{
#endif 

#define MINVORBIS_SAMPLES_PER_ITERATION 1024

#define MINVORBIS_48K_NORMAL_QUALITY    2
#define MINVORBIS_44K_NORMAL_QUALITY    2
#define MINVORBIS_32K_HIGH_QUALITY      3
#define MINVORBIS_32K_NORMAL_QUALITY    5
#define MINVORBIS_16K_HIGH_QUALITY      3
#define MINVORBIS_16K_NORMAL_QUALITY    5
#define MINVORBIS_8K_NORMAL_QUALITY     5

typedef void* minvorbis_enc_handle;

void
minvorbis_get_mem_size_for_encoding(int *   rt_tab,
                                    int *   shared,
                                    int *   encoder,
                                    int *   parser,
                                    int *   input_pcm,
                                    int *   output_bitstream,                                    
                                    void *  reserved);

minvorbis_enc_handle
minvorbis_init_encoder( void *          p_rt_tab_mem,
                        void *          p_shared_mem,
                        void *          p_encoder_mem,
                        void *          p_parser_mem,
                        int             channel_number,
                        int             sampling_rate,
                        int             bitrate_suggestion,
                        unsigned char * p_file_header,
                        int  *          bytes_in_header,
                        void *          reserved);

int
minvorbis_encode_one_frame( minvorbis_enc_handle    h_encoder,
                            void *                  p_pcm_in,
                            unsigned char *         p_bitstream_out,
                            int                     finalize,
                            void *                  reserved);

int /* return 32-bit version */
minvorbis_encode_get_version(void);

#ifdef __cplusplus
}
#endif 

#endif // MINVORBIS_ENCODER_EXP_H
