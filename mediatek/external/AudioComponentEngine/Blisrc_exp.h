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
 *   Blisrc_exp.h
 *
 * Project:
 * --------
 *   SWIP
 *
 * Description:
 * ------------
 *   BLISRC Interface Definition
 *
 * Author:
 * -------
 *   Scholar Chang
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/

#ifndef __BLISRC_EXP_H__
#define __BLISRC_EXP_H__

typedef void Blisrc_Handle;

typedef enum {
    BLISRC_IN_Q1P15_OUT_Q1P15 = 0,   // 16-bit Q1.15 input, 16-bit Q1.15 output
    BLISRC_IN_Q1P15_OUT_Q1P31 = 1,   // 16-bit Q1.15 input, 32-bit Q1.31 output
    BLISRC_IN_Q9P23_OUT_Q1P31 = 2,   // 32-bit Q9.23 input, 32-bit Q1.31 output
    BLISRC_IN_Q1P31_OUT_Q1P31 = 3,   // 32-bit Q1.31 input, 32-bit Q1.31 output
} BLISRC_PCM_FORMAT;

/***************************************************************************************
|   STRUCTURE
|
|       Blisrc_Param
|
|   MEMBERS
|
|   in_sampling_rate            [I] Input sampling rate, unit: Hz
|   in_channel                  [I] Input channel number
|                                   1: Mono
|                                   2: Stereo
|   ou_sampling_rate            [I] Output sampling rate, unit: Hz
|   ou_channel                  [I] Output channel number
|                                   1: Mono
|                                   2: Stereo
|   PCM_Format                  [I] Input / output PCM format
|                                   0 (BLISRC_IN_Q1P15_OUT_Q1P15):
|                                       16-bit Q1.15 input, 16-bit Q1.15 output
|                                   1 (BLISRC_IN_Q1P15_OUT_Q1P31):
|                                       16-bit Q1.15 input, 32-bit Q1.31 output
|                                   2 (BLISRC_IN_Q9P23_OUT_Q1P31):
|                                       32-bit Q9.23 input, 32-bit Q1.31 output
|                                   3 (BLISRC_IN_Q1P31_OUT_Q1P31):
|                                       32-bit Q1.31 input, 32-bit Q1.31 output
|**************************************************************************************/
typedef struct {
    unsigned int in_sampling_rate;
    unsigned int in_channel;
    unsigned int ou_sampling_rate;
    unsigned int ou_channel;
    unsigned int PCM_Format;
} Blisrc_Param;

/*
    Get the required buffer sizes of BLISRC
    Return value                    < 0  : Error 
                                    >= 0 : Normal
    p_internal_buf_size_in_byte [O] Required internal buffer size in bytes
    p_temp_buf_size_in_byte     [O] Required temp buffer size in bytes
    p_param                     [I] Pointer to the parameter structure
*/
int Blisrc_GetBufferSize(   unsigned int *p_internal_buf_size_in_byte,
                            unsigned int *p_temp_buf_size_in_byte,
                            Blisrc_Param *p_param);

/*----------------------------------------------------------------------*/
/* Get the BLI Software SRC handler.                                    */
/* Return: the handle of current BLI Software SRC                       */
/*----------------------------------------------------------------------*/
/*
    Initialize the BLISRC
    Return value       < 0  : Error 
                       >= 0 : Normal
    pp_handle      [I] Pointer to the handle of the BLISRC SWIP
    p_internal_buf [I] Pointer to the internal buffer
    p_param        [I] Pointer to the parameter structure
*/
int Blisrc_Open(    Blisrc_Handle **pp_handle,
                    void *p_internal_buf,
                    Blisrc_Param *p_param);

/*
    Process 16-bit / 32-bit data from input buffer to output buffer
    Return value      < 0  : Error 
                      >= 0 : Normal
    p_handle      [I] Handle of the BLISRC SWIP
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
int Blisrc_Process( Blisrc_Handle *p_handle,
                    char *p_temp_buf,
                    void *p_in_buf,
                    unsigned int *p_in_byte_cnt,
                    void *p_ou_buf,
                    unsigned int *p_ou_byte_cnt);

/*
    Change the input sampling rate between two Blisrc_Process function calls
    Return value         < 0  : Error 
                         >= 0 : Normal
    p_handle         [I] Handle of the BLISRC SWIP
    in_sampling_rate [I] Input sampling rate, unit: Hz
*/
int Blisrc_SetSamplingRate( Blisrc_Handle *p_handle,
                            unsigned int in_sampling_rate);

/*
    Clear the internal status for the discontinuous input buffer 
    (such as change output device)
    Return value         < 0  : Error 
                         >= 0 : Normal
    p_handle         [I] Handle of the BLISRC SWIP
*/
int Blisrc_Reset(Blisrc_Handle *p_handle);

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
    Version 1.0.0: (JY Huang)
        First release
    Version 2.0.0: (HP Cheng)
        Optimization & performance improvement
    Version 2.1.0: (Scholar Chang)
        Add the dynamic sampling rate change mechanism
    Version 2.2.0: (Scholar Chang)
        Modify the interface for 32-bit audio
*/
int Blisrc_GetVersion(void);

#endif  // __BLISRC_EXP_H__

