/******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2006
*
******************************************************************************/

/******************************************************************************
* Filename:
* ---------
*   msbc_codec_exp.h
*
* Project:
* --------
*   BT
*
* Description:
* ------------
*
*   This header file contains the type definitions and functions of voice encoder
*
*
* Author:
* -------
*   Wn Chen
*
*******************************************************************************/

#ifndef _MSBC_CODEC_EXP_H_
#define _MSBC_CODEC_EXP_H_

#define MSBC_PCM_LEN_IN_SAMPLE      120   
#define MSBC_BITSTREAM_LEN_IN_BYTE  57

#if 0
typedef char           Word8;
typedef short          Word16;
typedef int            Word32;
typedef unsigned int   UWord32;
#endif

#define MSBC_DECODE_ERROR            (-1)
#define MSBC_DECODE_SYNCWORD_ERROR   (-2)
#define MSBC_DECODE_CRC_ERROR_       (-3)
#define MSBC_DECODE_HEADER_ERROR     (-4)
#define MSBC_BUFFER_INCORRECT        (-5)

		
#ifndef NULL
#define NULL 0
#endif

int MSBC_DEC_Process(
   void   *pHandle,  //handle
   char   *pInBuf,   //input MSBC packet
   int    *pInLen,   //input length (Byte)
   short  *pOutBuf,  //output Sample
   int    *pOutLen   //output length (Word)
);

int MSBC_ENC_Process(
   void   *pHandle,  //handle
   short  *pInBuf,   //input Samples
   int    *pInLen,   //input length (word)
   char   *pOutBuf,  //MSBC packet
   int    *pOutLen   //output Length (byte)
);

int MSBC_DEC_GetBufferSize( void );
int MSBC_ENC_GetBufferSize( void );
void *MSBC_DEC_Init(signed char *pBuffer );
void *MSBC_ENC_Init(signed char *pBuffer );

#endif

