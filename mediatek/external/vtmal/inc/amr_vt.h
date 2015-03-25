/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
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
 *   amr_vt.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   AMR codec interface API
 *
 * Author:
 * -------
 *   Stan Huang
 *
 *------------------------------------------------------------------------------
 * $Revision:$ 1.0.0
 * $Modtime:$  
 * $Log:$
 * 
 * 08 19 2012 ning.feng
 * [ALPS00326839] [6575JB][BSP Package][6577JB][CTS 4.1_r1][mt6577_phone] In android.media.cts.DecoderTest 4cases - Fail
 * .
 * 
 * 08 18 2012 liwen.tan
 * NULL
 * .
 * 
 * 08 18 2012 liwen.tan
 * NULL
 * .
 * 
 * 08 18 2012 liwen.tan
 * NULL
 * .
 *
 *
 *******************************************************************************/

#ifndef AMR_CODEER_VT_DOT_H
#define AMR_CODEER_VT_DOT_H

#ifdef __cplusplus
extern "C"{
#endif 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
 
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/

typedef enum { BR475_VT = 0,
               BR515_VT,            
               BR59_VT,
               BR67_VT,
               BR74_VT,
               BR795_VT,
               BR102_VT,
               BR122_VT,            
	            BRDTX_VT	    
             }AMR_BitRate_VT;

/*****************************************************************************
 * STRUCTURE
 *  AMR_ENC_HANDLE
 *
 * DESCRIPTION
 *  AMR encoder handler
 *
 * MEMBERS
 *  amr_enc_data   Encoder internal data pointer (these data can't be release during encoding process)
 *  BitRate         Encoder bitrate
 *  dtx_enable      DTX enable or not (1:enable, 0:disable) 
 *
 *****************************************************************************
 */
typedef struct {
   void *amr_enc_data;
   int dtx_enable;    
   AMR_BitRate_VT BitRate;                	
}AMR_ENC_HANDLE_VT;

/*****************************************************************************
 * STRUCTURE
 *  AMR_DEC_HANDLE
 *
 * DESCRIPTION
 *  AMR decoder handler
 *
 * MEMBERS
 *  amr_dec_data    Decoder internal data pointer (these data can't be release during decoding process)
 *
 *****************************************************************************
 */
typedef struct {
   void *amr_dec_data;
}AMR_DEC_HANDLE_VT;
 
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
 
/*****************************************************************************
 * FUNCTION
 *  AMREnc_GetBufferSize
 *
 * DESCRIPTION
 *  This function gets the buffers size for encoder.
 *
 * PARAMETERS
 *  int_buf_size    [o] Internal buffer size
 *  tmp_buf_size    [o] Temporary buffer size
 *  bits_buf_size   [o] Bitstream buffer size
 *  pcm_buf_size    [o] Speech PCM buffer size 
 *
 * RETURNS
 *  None
 *
 *****************************************************************************
 */
void AMREnc_GetBufferSize_VT(
   unsigned int *int_buf_size,
   unsigned int *tmp_buf_size,
   unsigned int *bs_buf_size,
   unsigned int *pcm_buf_size   
);
/*****************************************************************************
 * FUNCTION
 *  AMRDec_GetBufferSize
 *
 * DESCRIPTION
 *  This function gets the buffers size for decoder.
 *
 * PARAMETERS
 *  int_buf_size    [o] Internal buffer size
 *  tmp_buf_size    [o] Temporary buffer size
 *  bits_buf_size   [o] Bitstream buffer size 
 *  pcm_buf_size    [o] Speech PCM buffer size
 *
 * RETURNS
 *  None
 *
 *****************************************************************************
 */
void AMRDec_GetBufferSize_VT(
   unsigned int *int_buf_size,
   unsigned int *tmp_buf_size,
   unsigned int *bs_buf_size,   
   unsigned int *pcm_buf_size
);


/*****************************************************************************
 * FUNCTION
 *  AMREnc_Init
 *
 * DESCRIPTION
 *  This function initialize the buffer for encoder.
 *
 * PARAMETERS
 *  int_buffer  [i] Encoder internal buffer pointer. This buffer can¡¦t be re-used by other application.
 *  dtx         [i] Supported DTX or not (1:enable, 0:disable).
 *  BitRate     [i] Bitrate of AMR codec
 * 	 
 * RETURNS
 *  the pointer of AMR encoder handler
 *
 *****************************************************************************
 */
AMR_ENC_HANDLE_VT *AMREnc_Init_VT(
   void *int_buffer,
   AMR_BitRate_VT BitRate,
   int dtx   
);

/*****************************************************************************
 * FUNCTION
 *  AMRDec_Init
 *
 * DESCRIPTION
 *  This function initialize the buffer for decoder.
 *
 * PARAMETERS
 *  int_buffer    [i] Decoder internal buffer pointer. This buffer can¡¦t be re-used by other application.
 *
 * RETURNS
 *  the pointer of AMR decoder handler
 *
 *****************************************************************************
 */
AMR_DEC_HANDLE_VT *AMRDec_Init_VT(
   void *int_buffer
);


/*****************************************************************************
 * FUNCTION
 *  AMR_Encode
 *
 * DESCRIPTION
 *  This function encodes one frame pcm data (160 samples/frame) and output AMR bitstream.
 *
 * PARAMETERS
 *  amr_enc_handle  [i] the pointer of AMR encoder handle
 *  tmp_buffer      [i] Temporary buffer pointer. Temporary buffer could be used by other application after encoding a frame.  
 *  pcm_buffer      [i] Input PCM sample buffer pointer (input 160 samples pcm data)   
 *  bs_buffer       [o] Output bitstream buffer pointer
 *  bitrate         [i] AMR Encoder bitrate 
 *
 * RETURNS
 *  Bitstream length in byte
 *
 *
 *****************************************************************************
 */
 
int AMR_Encode_VT(
   AMR_ENC_HANDLE_VT *amr_enc_handle,
   void *tmp_buffer,
   short *pcm_buffer,
   unsigned char *bs_buffer, 
	AMR_BitRate_VT bitrate						
);


/*****************************************************************************
 * FUNCTION
 *  AMR_Decode
 *
 * DESCRIPTION
 *  This function decodes AMR bitstream and output 160 samples pcm data.
 *
 * PARAMETERS
 *  amr_dec_handle  [i] The pointer of AMR decoder handle  
 *  tmp_buffer      [i] Temporary buffer pointer. Temporary buffer could be used by other application after encoding a frame.
 *  pcm_buffer      [o] Output PCM sample buffer pointer (output 160 samples pcm data)   
 *  bs_buffer       [i] Input bitstream buffer pointer 
 *
 * RETURNS
 *  None  
 *
 *****************************************************************************
 */
 
void AMR_Decode_VT(
   AMR_DEC_HANDLE_VT *amr_dec_handle,
   void *tmp_buffer,
   short *pcm_buffer,	
   unsigned char *bs_buffer,
   unsigned char frame_types   
); 
 
 
 
#ifdef __cplusplus
}
#endif 
#endif


