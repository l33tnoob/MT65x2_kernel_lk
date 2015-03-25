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
 *   mp3dec_exp.h
 *
 * Project:
 * --------
 *   
 *
 * Description:
 * ------------
 *   MP3 Decoder Interface
 *
 * Author:
 * -------
 *   Sammy Shih
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/

#ifndef MP3DEC_EXP_H
#define MP3DEC_EXP_H
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef NULL
#define NULL    0
#endif

/* The error message return by "MP3Dec_Decode()" */
#define NULL_HANDLE             -1
#define NULL_PCM_BUF            -2
#define PCM_BUF_NOT_ALIGNED     -3
#define NULL_BS_BUF             -4
#define NULL_BS_READ            -5
#define BS_BUF_SIZE_INCORRECT   -6
#define FORMAT_NOT_SUPPORT      -7


typedef struct {
	short sampleRateIndex;
	short CHNumber;	
	int PCMSamplesPerCH;
}Mp3dec_handle;

/*----------------------------------------------------------------------*/
// FUNCTION
//  MP3Dec_GetVersion
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
int MP3Dec_GetVersion(void);

/*----------------------------------------------------------------------*/
/* Get required buffer size for MP3 Decoder                             */
/*----------------------------------------------------------------------*/
void MP3Dec_GetMemSize(int *Min_BS_size,        /* Output, the required min. Bitsream buffer size in byte*/
							  int *PCM_size,           /* Output, the required PCM buffer size in byte          */
							  int *Working_BUF1_size,   /* Output, the required Working buffer1 size in byte    */
                              int *Working_BUF2_size);  /* Output, the required Working buffer2 size in byte    */


/*----------------------------------------------------------------------*/
/* Get the MP3 decoder handler.                                         */
/*   Return: the handle of current MP3 Decoder                          */
/*----------------------------------------------------------------------*/
Mp3dec_handle* MP3Dec_Init(void *pWorking_BUF1,          /* Input, pointer to Working buffer1  */
						   void *pWorking_BUF2);         /* Input, pointer to Working buffer2  */

/*----------------------------------------------------------------------*/
/* Decompress the bitstream to PCM data                                 */
/*   Return: The consumed data size of Bitsream buffer for this  frame  */
/*----------------------------------------------------------------------*/
int MP3Dec_Decode(Mp3dec_handle* handle,             /* the handle of current MP3 Decoder    */
                  void *pPCM_BUF,                    /* Input, pointer to PCM buffer         */
                  void *pBS_BUF,                     /* Input, pointer to Bitsream buffer    */
                  int BS_BUF_size,                   /* Input, the Bitsream buffer size      */
                  void *pBS_Read);                   /* Input, bitstream buffer read pointer */

/*----------------------------------------------------------------------*/
/* Get the MP3 frame length.                                            */
/*   Return: the frame length (in byte)                                 */
/*----------------------------------------------------------------------*/
unsigned int MP3_GetFrameLengthGeneral(unsigned int header, unsigned int br_idx);

#ifdef __cplusplus
}
#endif 

#endif /*MP3DEC_EXP_H*/
