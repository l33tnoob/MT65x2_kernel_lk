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
*   cvsd_codec_exp.h
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

#ifndef __CVSD_CODEC_EXP_H__
#define __CVSD_CODEC_EXP_H__

//CVSD CODEC Interface
void CVSD_DEC_Process(
   void   *pHandle,  //handle
   char   *pInBuf,   //input CVSD packet
   int    *pInLen,   //input length (Byte)
   short  *pOutBuf,  //output Sample
   int    *pOutLen   //output length (Word)
);

void CVSD_ENC_Process(
   void   *pHandle,  //handle
   short  *pInBuf,   //input Samples
   int    *pInLen,   //input length (word)
   char   *pOutBuf,  //CVSD packet
   int    *pOutLen   //output Length (byte)
);

int CVSD_DEC_GetBufferSize( void );
int CVSD_ENC_GetBufferSize( void );
void *CVSD_DEC_Init(signed char *pBuffer );
void *CVSD_ENC_Init(signed char *pBuffer );

//CVSD PLC interface
int g711plc_GetMemorySize_v2(void);
void g711plc_construct_v2(void *lc, unsigned int samplingrate);
void g711plc_addtohistory_v2(void *lc, short *s, unsigned int dwBtEv3HalfBad);
void g711plc_dofe_v2(void *lc,short *out, unsigned int dwBtEv3HalfBad);

#endif //__CVSD_CODEC_EXP_H__
