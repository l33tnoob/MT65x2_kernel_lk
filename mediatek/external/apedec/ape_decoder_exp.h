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

#ifndef APE_DECODER_EXP_H
#define APE_DECODER_EXP_H

#ifdef __cplusplus
extern "C" {
#endif

#define APE_ERR_EOS -1
#define APE_ERR_CRC -2

typedef void* ape_decoder_handle;

/*----------------------------------------------------------------------*/
// FUNCTION
//  ape_decoder_GetVersion
//
// DESCRIPTION
//  This function was used to get current version of library
//
// RETURNS
//  B31-B24:  Project Type
//  B23-B16:  Compiler and Major Version
//  B15-B08:  Minor Version
//  B07-B00:  Release Version
//
/*----------------------------------------------------------------------*/
int ape_decoder_get_version(void);

struct ape_decoder_init_param {
    short           fileversion;
    unsigned short  compressiontype;
    unsigned int    blocksperframe;
    unsigned int    finalframeblocks;
    unsigned int    totalframes;
    unsigned short  bps;
    unsigned short  channels;
};

void
ape_decoder_get_mem_size(unsigned int *bs_buffer,
                         unsigned int *working_buffer,
                         unsigned int *pcm_buffer);

ape_decoder_handle
ape_decoder_init(void*  working_buffer,
                 struct ape_decoder_init_param* ape_param);

int
ape_decoder_reset(ape_decoder_handle handle,
                  int firstbyte,
                  int newframe);

int
ape_decoder_decode(ape_decoder_handle handle,
                   unsigned char* inbuffer,
                   int* bytes_consumed,
                   unsigned char* outbuffer,
                   int* bytes_produced);

#ifdef __cplusplus
}
#endif

#endif // APE_DECODER_EXP_H
