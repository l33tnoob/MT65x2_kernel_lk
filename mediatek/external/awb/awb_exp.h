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
 *   awb_exp.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   AMR-WB codec interface API
 *
 * Author:
 * -------
 *   Eddy Wu
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
 * 07 19 2012 ning.feng
 * [ALPS00315896] [Need Patch] [Volunteer Patch]JB migration
 * .
 *
 * 07 17 2012 ning.feng
 * NULL
 * .
 *
 * 07 17 2012 ning.feng
 * NULL
 * .
 *
 * 07 17 2012 ning.feng
 * NULL
 * .
 *
 * 12 23 2010 eddy.wu
 * [WCPSP00000503] Checkin SWIP Modification
 * .
 *
 *
 *******************************************************************************/

#ifndef AMRWB_CODEER_DOT_H
#define AMRWB_CODEER_DOT_H

#ifdef __cplusplus
extern "C"{
#endif 
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef enum 
{ 
   BR660 = 0,
   BR885,
   BR1265,
   BR1425,
   BR1585,
   BR1825,
   BR1985,
   BR2305,
   BR2385,
	AWB_BRDTX
} AWB_BITRATE;

/*****************************************************************************
 * STRUCTURE
 *  AWB_ENC_HANDLE
 *
 * DESCRIPTION
 *  AMR-WB encoder handler
 *
 * MEMBERS
 *  vadFlag  The decision of VAD (voice activity detection) after encoding
 *           ( 1 = speech, 0 = noise )
 *
 *****************************************************************************
 */
typedef struct {
   int vadFlag;
} AWB_ENC_HANDLE;

/*****************************************************************************
 * STRUCTURE
 *  AWB_DEC_HANDLE
 *
 * DESCRIPTION
 *  AMR-WB decoder handler
 *
 * MEMBERS
 *  bitRate    The bitrate of the decoded bitstream
 *
 *****************************************************************************
 */
typedef struct {
   AWB_BITRATE bitRate;
} AWB_DEC_HANDLE;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
 
/*****************************************************************************
 * FUNCTION
 *  AWBEnc_GetBufferSize
 *
 * DESCRIPTION
 *  This function gets the buffers size for encoder.
 *
 * PARAMETERS
 *  intBufSize    [o] Internal buffer size
 *  tmpBufSize    [o] Temporary buffer size
 *  bsBufSize     [o] Bitstream buffer size
 *  pcmBufSize    [o] Speech PCM buffer size 
 *
 * RETURNS
 *  None
 *
 *****************************************************************************
 */
void AWBEnc_GetBufferSize(
   unsigned int *intBufSize,
   unsigned int *tmpBufSize,
   unsigned int *bsBufSize,
   unsigned int *pcmBufSize   
);
/*****************************************************************************
 * FUNCTION
 *  AWBDec_GetBufferSize
 *
 * DESCRIPTION
 *  This function gets the buffers size for decoder.
 *
 * PARAMETERS
 *  intBufSize    [o] Internal buffer size
 *  tmpBufSize    [o] Temporary buffer size
 *  bsBufSize     [o] Bitstream buffer size 
 *  pcmBufSize    [o] Speech PCM buffer size
 *
 * RETURNS
 *  None
 *
 *****************************************************************************
 */
void AWBDec_GetBufferSize(
   unsigned int *intBufSize,
   unsigned int *tmpBufSize,
   unsigned int *bsBufSize,   
   unsigned int *pcmBufSize
);

/*****************************************************************************
 * FUNCTION
 *  AWBEnc_Init
 *
 * DESCRIPTION
 *  This function initialize the buffer for encoder.
 *
 * PARAMETERS
 *  pInternalBuf  [i] Encoder internal buffer pointer. This buffer can't be re-used by other application.
 *  bitRate       [i] Bitrate of AMR-WB codec
 *  dtxEnable     [i] Supported DTX or not (1:enable, 0:disable).
 * 	 
 * RETURNS
 *  the pointer of AMR-WB encoder handler
 *
 *****************************************************************************
 */
AWB_ENC_HANDLE *AWBEnc_Init(
   void *pInternalBuf,
   AWB_BITRATE bitRate,
   int dtxEnable
);

/*****************************************************************************
 * FUNCTION
 *  AWBDec_Init
 *
 * DESCRIPTION
 *  This function initialize the buffer for decoder.
 *
 * PARAMETERS
 *  pInternalBuf    [i] Decoder internal buffer pointer. This buffer can't be re-used by other application.
 *
 * RETURNS
 *  the pointer of AMR-WB decoder handler
 *
 *****************************************************************************
 */
AWB_DEC_HANDLE *AWBDec_Init(
   void *pInternalBuf
);

/*****************************************************************************
 * FUNCTION
 *  AWB_Encode
 *
 * DESCRIPTION
 *  This function encodes one frame pcm data (320 samples/frame) and output AMR-WB bitstream.
 *
 * PARAMETERS
 *  awbEncHandle  [i] the pointer of AMR encoder handle
 *  pTmpBuf       [i] Temporary buffer pointer. Temporary buffer could be used by other application while encoder is not working 
 *  pPcmBuf       [i] Input PCM sample buffer pointer (input 320 samples pcm data)   
 *  pBsBuf        [o] Output bitstream buffer pointer
 *  bitRate       [i] Bitrate of AMR-WB codec
 *
 * RETURNS
 *  Bitstream length in byte
 *
 *
 *****************************************************************************
 */
 
int AWB_Encode(
   AWB_ENC_HANDLE *awbEncHandle,
   void *pTmpBuf,
   short *pPcmBuf,
   unsigned char *pBsBuf,
   AWB_BITRATE bitRate
);


/*****************************************************************************
 * FUNCTION
 *  AWB_Decode
 *
 * DESCRIPTION
 *  This function decodes AMR-WB bitstream and output 320 samples pcm data.
 *
 * PARAMETERS
 *  awbDecHandle  [i] The pointer of AMR-WB decoder handle  
 *  pTmpBuf       [i] Temporary buffer pointer. Temporary buffer could be used by other application while decoder is not working
 *  pPcmBuf       [o] Output PCM sample buffer pointer (output 320 samples pcm data)   
 *  pBsBuf        [i] Input bitstream buffer pointer 
 *
 * RETURNS
 *  Actual bitstream length consumed 
 *
 *****************************************************************************
 */
 
int AWB_Decode(
   AWB_DEC_HANDLE *awbDecHandle,
   void *pTmpBuf,
   short *pPcmBuf,	
   unsigned char *pBsBuf
); 

/*****************************************************************************
 * FUNCTION
 *  AWB_GetVersion
 *
 * DESCRIPTION
 *  This function is for get AMR-WB version
 *
 * PARAMETERS
 *
 * RETURNS
 *  integer code version
 *
 *****************************************************************************
 */

int AWB_GetVersion(void);

#ifdef __cplusplus
}
#endif 
#endif
