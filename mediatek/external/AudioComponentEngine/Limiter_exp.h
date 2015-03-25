/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 * Limiter_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * Limiter export header file
 *
 * Author:
 * -------
 * Scholar Chang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __LIMITER_EXP_H__
#define __LIMITER_EXP_H__

// Notice: All the buffer pointers are required to be 
//         four-byte-aligned to avoid the potential on-target process error !!!

typedef enum {
    LMTR_IN_Q33P31_OUT_Q1P31 = 0,   // 64-bit Q33.31 input, 32-bit Q1.31 output
    LMTR_MAX_PCM_FORMAT
} LMTR_PCM_FORMAT;

typedef void Limiter_Handle;

/***************************************************************************************
|   STRUCTURE
|
|       Limiter_InitParam
|
|   MEMBERS
|
|   Channel                 Channel number, only support: 1 or 2
|   Sampling_Rate           Input signal sampling rate, unit: Hz
|                           Support 9 kinds of sampling rates:
|                           48000, 44100, 32000, 
|                           24000, 22050, 16000,
|                           12000, 11025,  8000
|   
|   PCM_Format              Input / output PCM format
|                           0 (LMTR_IN_Q33P31_OUT_Q1P31):
|                               64-bit Q33.31 input, 32-bit Q1.31 output
|**************************************************************************************/
typedef struct {
    unsigned int Channel;
    unsigned int Sampling_Rate;
    unsigned int PCM_Format;
} Limiter_InitParam;

/*
    Get the required buffer sizes of limiter
    Return value                    < 0  : Error 
                                    >= 0 : Normal
    p_internal_buf_size_in_byte [O] Required internal buffer size in bytes
    p_temp_buf_size_in_byte     [O] Required temp buffer size in bytes
    PCM_Format                  [I] Input / output PCM format
*/
int Limiter_GetBufferSize(  unsigned int *p_internal_buf_size_in_byte,
                            unsigned int *p_temp_buf_size_in_byte,
                            unsigned int PCM_Format);

/*
    Initialize the limiter
    Return value                < 0  : Error 
                                >= 0 : Normal
    pp_handle               [I] Pointer to the handle of the limiter
    p_internal_buf          [I] Pointer to the internal buffer
    p_init_param            [I] Pointer to the initial parameters
*/
int Limiter_Open(   Limiter_Handle **pp_handle,
                    void *p_internal_buf,
                    Limiter_InitParam *p_init_param);

/*
    Process 16-bit / 32-bit / 64-bit data from input buffer to output buffer
    Return value      < 0  : Error 
                      >= 0 : Normal
    p_handle      [I] Handle of the limiter
    p_temp_buf    [I] Pointer to the temp buffer used in processing the data
    p_in_buf      [I] Pointer to the input PCM buffer
                      For stereo input, the layout of LR is L/R/L/R...
    p_in_byte_cnt [I] Valid input buffer size in bytes
                  [O] Consumed input buffer size in bytes
    p_ou_buf      [I] Pointer to the output PCM buffer
                      For stereo output, the layout of LR is L/R/L/R...
    p_ou_byte_cnt [I] Available output buffer size in bytes
                  [O] Produced output buffer size in bytes
*/
int Limiter_Process(    Limiter_Handle *p_handle,
                        char *p_temp_buf,
                        void *p_in_buf,
                        unsigned int *p_in_byte_cnt,
                        void *p_ou_buf,
                        unsigned int *p_ou_byte_cnt);

/*
    Clear the internal status for the discontinuous input buffer 
    (such as change output device)
    Return value         < 0  : Error 
                         >= 0 : Normal
    p_handle         [I] Handle of the limiter
*/
int Limiter_Reset(Limiter_Handle *p_handle);

/*
                      +------- Main Feature Version
                      |+------ Sub Feature Version
                      ||+----- Performance Improvement Version
                      |||
    LIMITER_VERSION 0xABC
                      |||
            +---------+||
            | +--------+|
            | | +-------+
            | | |
    Version 1.0.0: (Scholar Chang)
        First release

*/
int Limiter_GetVersion(void);

#endif // __LIMITER_EXP_H__
