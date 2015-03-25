/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   adpcm_dec_exp.h
 *
 * Project:
 * --------
 *   SWIP
 *
 * Description:
 * ------------
 *   ADPCM decoder
 *
 * Author:
 * -------
 *   Hidalgo Huang
 *
 *------------------------------------------------------------------------------
 * $Revision:$ 1.0.0
 * $Modtime:$  
 * $Log:$
 * 
 * 10 11 2012 hidalgo.huang
 * [WCPSP00000688] [AUDIO][SWIP][ADPCM]
 * update struct description.
 * 
 * 09 20 2012 hidalgo.huang
 * [WCPSP00000688] [AUDIO][SWIP][ADPCM]
 * refine enum type.
 * 
 * 08 06 2012 hidalgo.huang
 * [WCPSP00000688] [AUDIO][SWIP][ADPCM]
 * add function ADPCM_FlushDecoder().
 * 
 * 07 27 2012 hidalgo.huang
 * [WCPSP00000688] [AUDIO][SWIP][ADPCM]
 * initial version.
 *
 *
 *******************************************************************************/

#ifndef __ADPCM_DEC_LIB_
#define __ADPCM_DEC_LIB_

#ifdef __cplusplus
extern "C"{
#endif

//*******************************************************************************
//                         ADPCM_DEC_STATUS  
//*******************************************************************************
typedef enum { 
   ADPCM_DEC_FAIL        = 0,
   ADPCM_DEC_SUCCESS     = 1,
} ADPCM_DEC_STATUS;

//*******************************************************************************
//                         ADPCM_DEC_FORMAT  
//*******************************************************************************
typedef enum {
   FORMAT_DEC_MSADPCM  = 2,
   FORMAT_DEC_DVI_IMAADPCM = 17,
} ADPCM_DEC_FORMAT;

//*******************************************************************************
// STRUCTURE
//  ADPCM_DEC_HDL
//
// DESCRIPTION
//  ADPCM decoder handle
//*******************************************************************************
typedef void* ADPCM_DEC_HDL;

//*******************************************************************************
// STRUCTURE
//  ADPCM_DEC_PARAM
//
// DESCRIPTION
//  ADPCM parameters (for APDCM decoder initialization)
//
// MEMBERS
//  format_tag           coding format
//  channel_num          channel number
//  sample_rate          number of samples per second
//  block_align          number of bytes per block
//  bits_per_sample      number of bits per sample
//  extra_data_length    number of bytes of extra data
//  bits_per_sample      pointer to extra data
//*******************************************************************************
typedef struct {
   ADPCM_DEC_FORMAT  format_tag;
   unsigned short    channel_num;
   unsigned int      sample_rate;   
   unsigned short    block_align;
   unsigned short    bits_per_sample;
   unsigned short    extra_data_length;
   unsigned char*    extra_data;
} ADPCM_DEC_PARAM;

//*******************************************************************************
//                         DECLARATION OF PROTOTYPES
//*******************************************************************************

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetDecVersion
//
//  DESCRIPTION
//  [DECODER] This function gets the version of library.
//
//  RETURNS
//  B31-B24:  Project Type
//  B23-B16:  Compiler and Major Version
//  B15-B08:  Minor Version
//  B07-B00:  Release Version
//
//*******************************************************************************
int ADPCM_GetDecVersion(void);

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetDecBufferSize
//
//  DESCRIPTION
//  [DECODER] This function gets the buffers size for decoder.
//
//  PARAMETERS
//  bs_buf_size   [i] input bitstream buffer size (byte). 
//                    It should be a multiple number of 4. minimum size is 256
//  pcm_buf_size  [o] pcm buffer size (byte). the size depends on the bs_buf_size
//  int_buf_size  [o] internal working buffer size (byte)
//
//  RETURNS
//  ADPCM_DEC_SUCCESS, if the bs_buf_size is legal.
//  If the bs_buf_size is illegal, ADPCM_DEC_FAIL is returned;
//
//*******************************************************************************
ADPCM_DEC_STATUS ADPCM_GetDecBufferSize(unsigned int bs_buf_size,
                                    unsigned int *pcm_buf_size,
                                    unsigned int *int_buf_size);

//*******************************************************************************
//  FUNCTION
//  ADPCM_InitDecoder
//
//  DESCRIPTION
//  [DECODER] Decoder initialization.
//
//  PARAMETERS
//  internal_buf  [i] internal working buffer. the size should as least equal to int_buf_size
//  param         [i] adpcm parameter.
//
//  RETURNS
//  decoder handle; NULL if initilation failed. 
//
//*******************************************************************************
ADPCM_DEC_HDL ADPCM_InitDecoder(void *internal_buf, ADPCM_DEC_PARAM *param);

//*******************************************************************************
//  FUNCTION
//  ADPCM_Decode
//
//  DESCRIPTION
//  [DECODER] decoder function, decode bistream in input buffer and put the PCM to output buffer
//
//  PARAMETERS
//  hdl           [i]  adpcm decoder handle
//  inbuf         [i]  input buffer
//  inputBytes    [i]  input data counts (byte) in input buffer
//  outbuf        [o]  output buffer
//  outputBytes   [i]  available size (byte) in output buffer
//                [o]  output data counts (byte) in output buffer
//
//  RETURNS
//  data counts (byte) that is consumed by ADPCM_Decode()
//
//*******************************************************************************
unsigned int ADPCM_Decode(ADPCM_DEC_HDL hdl, const void *inbuf,
                          unsigned int inputBytes, void *outbuf,
                          unsigned int *outputBytes);

//*******************************************************************************
//  FUNCTION
//  ADPCM_FlushDecoder
//
//  DESCRIPTION
//  [DECODER] decoder function, reset decoder to initial state when seek happens
//
//  PARAMETERS
//  hdl           [i]  adpcm decoder handle
//
//  RETURNS
//  None
//
//*******************************************************************************
void ADPCM_FlushDecoder(ADPCM_DEC_HDL hdl);

#ifdef __cplusplus
}
#endif

#endif
