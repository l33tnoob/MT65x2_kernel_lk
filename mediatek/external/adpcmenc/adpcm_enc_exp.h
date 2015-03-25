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
 *   adpcm_enc_exp.h
 *
 * Project:
 * --------
 *   SWIP
 *
 * Description:
 * ------------
 *   ADPCM encoder
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
 * 09 26 2012 hidalgo.huang
 * [WCPSP00000688] [AUDIO][SWIP][ADPCM]
 * initial version.
 * 
 *
 *******************************************************************************/

#ifndef __ADPCM_ENC_LIB_
#define __ADPCM_ENC_LIB_

#ifdef __cplusplus
extern "C"{
#endif

//*******************************************************************************
//                         ADPCM_STATUS  
//*******************************************************************************
typedef enum { 
   ADPCM_ENC_FAIL = 0,
   ADPCM_ENC_SUCCESS = 1,
} ADPCM_ENC_STATUS;

//*******************************************************************************
//                         ADPCM_FORMAT  
//*******************************************************************************
typedef enum {
   FORMAT_ENC_DVI_IMAADPCM = 0,
   FORMAT_ENC_MS_ADPCM = 1,
} ADPCM_ENC_FORMAT;

//*******************************************************************************
// STRUCTURE
//  ADPCM_ENC_HDL
//
// DESCRIPTION
//  ADPCM encoder handle
//*******************************************************************************
typedef void* ADPCM_ENC_HDL;

//*******************************************************************************
// STRUCTURE
//  ADPCM_ENC_PARAM
//
// DESCRIPTION
//  ADPCM encode parameters (for APDCM encoder initialization)
//
// MEMBERS
//  enc_format          encoding format
//  channel_num         channel number
//  sample_rate         sample rate
//  block_align_size    number of bytes per block
//*******************************************************************************
typedef struct {
   ADPCM_ENC_FORMAT  enc_format;
   unsigned short    channel_num;
   unsigned int      sample_rate;
   unsigned short    block_align_size;
} ADPCM_ENC_PARAM;

//*******************************************************************************
//                         DECLARATION OF PROTOTYPES
//*******************************************************************************

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetEncVersion
//
//  DESCRIPTION
//  [ENCODER] This function gets the version of library.
//
//  RETURNS
//  B31-B24:  Project Type
//  B23-B16:  Compiler and Major Version
//  B15-B08:  Minor Version
//  B07-B00:  Release Version
//
//*******************************************************************************
int ADPCM_GetEncVersion(void);

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetEncBufferSize
//
//  DESCRIPTION
//  [ENCODER] This function gets the buffers size for encoder.
//
//  PARAMETERS
//  pcm_buf_size  [i]  pcm buffer size (byte).
//  bs_buf_size   [o]  bitstream buffer size (byte).
//  int_buf_size  [o]  internal working buffer size (byte).
//
//  RETURNS
//  ADPCM_ENC_SUCCESS, if the pcm_buf_size is legal.
//  If the pcm_buf_size is illegal, ADPCM_ENC_FAIL is returned;
//
//*******************************************************************************
ADPCM_ENC_STATUS ADPCM_GetEncBufferSize(unsigned int pcm_buf_size,
                                        unsigned int *bs_buf_size,
                                        unsigned int *int_buf_size);

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetBlockSizeByDuration
//
//  DESCRIPTION
//  [ENCODER] This function gets block size and sample counts to encode one block.
//                   The encoded block duration is approximate to the given block_duration.
//
//  PARAMETERS
//  block_duration  [i]  block duration in milliseconds
//  param           [i]  adpcm encode parameter: enc_format, channel_num & sample_rate
//  param           [0]  adpcm encode parameter: block_align_size
//
//  RETURNS
//  number of samples per block
//
//*******************************************************************************
unsigned short ADPCM_GetBlockSizeByDuration(unsigned short block_duration, ADPCM_ENC_PARAM *param);

//*******************************************************************************
//  FUNCTION
//  ADPCM_InitEncoder
//
//  DESCRIPTION
//  [ENCODER] Encoder initialization.
//
//  PARAMETERS
//  internal_buf  [i]  internal working buffer. the size should as least equal to int_buf_size
//  param         [i]  adpcm encode parameter.
//
//  RETURNS
//  encoder handle; NULL if initilation failed. 
//
//*******************************************************************************
ADPCM_ENC_HDL ADPCM_InitEncoder(void *internal_buf, ADPCM_ENC_PARAM *param);

//*******************************************************************************
//  FUNCTION
//  ADPCM_Encode
//
//  DESCRIPTION
//  [ENCODER] encoder function, encode PCM data in input buffer and put the bitstream to output buffer
//
//  PARAMETERS
//  hdl           [i]  adpcm encoder handle
//  inbuf         [i]  input buffer
//  inputBytes    [i]  input data counts (byte) in input buffer
//  outbuf        [o]  output buffer
//  outputBytes   [i]  available size (byte) in output buffer
//                [o]  output data counts (byte) in output buffer
//
//  RETURNS
//  data counts (byte) that is consumed by ADPCM_Encode()
//
//*******************************************************************************
unsigned int ADPCM_Encode(ADPCM_ENC_HDL hdl, const void *inbuf,
                          unsigned int inputBytes, void *outbuf,
                          unsigned int *outputBytes);

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetEncExtraDataSize
//
//  DESCRIPTION
//  [ENCODER] This function gets the extra data size (byte).
//
//  PARAMETERS
//  hdl  [i]  adpcm encoder handle
//
//  RETURNS
//  None
//
//*******************************************************************************
unsigned short ADPCM_GetEncExtraDataSize(ADPCM_ENC_HDL hdl);

//*******************************************************************************
//  FUNCTION
//  ADPCM_GetEncExtraData
//
//  DESCRIPTION
//  [ENCODER] This function gets the extra data.
//
//  PARAMETERS
//  hdl          [i]  adpcm encoder handle
//  outbuf       [o]  output buffer
//  outputBytes  [i]  available size (byte) in output buffer
//
//  RETURNS
//  data counts (byte) that is written by ADPCM_GetEncExtraData().
//
//*******************************************************************************
unsigned short ADPCM_GetEncExtraData(ADPCM_ENC_HDL hdl, void *outbuf, unsigned short outputBytes);

//*******************************************************************************
//  FUNCTION
//  ADPCM_FlushEncoder
//
//  DESCRIPTION
//  [ENCODER] encoder function, reset encoder to initial state
//
//  PARAMETERS
//  hdl  [i]  adpcm encoder handle
//
//  RETURNS
//  None
//
//*******************************************************************************
void ADPCM_FlushEncoder(ADPCM_ENC_HDL hdl);

#ifdef __cplusplus
}
#endif

#endif
