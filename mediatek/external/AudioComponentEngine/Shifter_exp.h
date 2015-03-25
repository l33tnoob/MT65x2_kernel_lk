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
 * Shifter_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * Shifter export header file
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

#ifndef __SHIFTER_EXP_H__
#define __SHIFTER_EXP_H__

// Notice: All the buffer pointers are required to be 
//         four-byte-aligned to avoid the potential on-target process error !!!

typedef enum {
    SHFTR_IN_Q1P15_OUT_Q1P31 = 0,   // 16-bit Q1.15 input, 32-bit Q1.31 output
    SHFTR_IN_Q1P31_OUT_Q1P15 = 1,   // 32-bit Q1.31 input, 16-bit Q1.15 output
    SHFTR_IN_Q9P23_OUT_Q1P31 = 2,   // 32-bit Q9.23 input, 32-bit Q1.31 output
    SHFTR_IN_Q1P31_OUT_Q9P23 = 3,   // 32-bit Q1.31 input, 32-bit Q9.23 output
} SHFTR_PCM_FORMAT;

/*
    Process 16-bit / 32-bit data from input buffer to output buffer
    Return value      < 0  : Error 
                             -1 => PCM_Format incorrect enumerator
                             -2 => p_in_buf incorrect byte alignment
                             -3 => p_ou_buf incorrect byte alignment
                      >= 0 : Normal
    p_in_buf      [I] Pointer to the input PCM buffer
                      For stereo input, the layout of LR is L/R/L/R...
    p_in_byte_cnt [I] Valid input buffer size in bytes
                  [O] Consumed input buffer size in bytes
    p_ou_buf      [I] Pointer to the output PCM buffer
                      For stereo output, the layout of LR is L/R/L/R...
    p_ou_byte_cnt [I] Available output buffer size in bytes
                  [O] Produced output buffer size in bytes
    PCM_Format    [I] Input / output PCM format
                      0 (SHFTR_IN_Q1P15_OUT_Q1P31):
                          16-bit Q1.15 input, 32-bit Q1.31 output
                      1 (SHFTR_IN_Q1P31_OUT_Q1P15):
                          32-bit Q1.31 input, 16-bit Q1.15 output
                      2 (SHFTR_IN_Q9P23_OUT_Q1P31):
                          32-bit Q9.23 input, 32-bit Q1.31 output
                      3 (SHFTR_IN_Q1P31_OUT_Q9P23):
                          32-bit Q1.31 input, 32-bit Q9.23 output

    Note:
    Support N-in-N-out
*/
int Shifter_Process(    void *p_in_buf,
                        unsigned int *p_in_byte_cnt,
                        void *p_ou_buf,
                        unsigned int *p_ou_byte_cnt,
                        unsigned int PCM_Format);

#endif // __SHIFTER_EXP_H__
