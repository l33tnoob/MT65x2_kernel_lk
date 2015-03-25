/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 * Dc_Remove_fit.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * DC_Remove_fit Common header file.
 *
 * Author:
 * -------
 * Chipeng Chang
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
#ifndef __DC_REMOVE_FLT_H__
#define __DC_REMOVE_FLT_H__

#ifndef NULL
#define NULL    0
#endif

#ifndef ASSERT
#define ASSERT(x)
#endif

typedef void DCRemove_Handle;

//============================================================//
// Get buffer size for DC Removal Engine
// internal_buf_size_in_bytes   output  internal buffer size in bytes
//============================================================//
void DCR_GetBufferSize (
    unsigned int *internal_buf_size_in_bytes);

//============================================================//
// Open DC Removal Engine
// p_internal_buf   input   pointer to the internal buffer
// channel          input   channel number (1 or 2)
// sampling_rate    input   sampling rate (unit: Hz)
// working_mode     input   working mode
// return                   handle of the SWIP engine
//============================================================//
DCRemove_Handle *DCR_Open (
    signed char *p_internal_buf,
    unsigned int channel,
    unsigned int sampling_rate,
    unsigned int working_mode);

//============================================================//
// Process DC Removal Filter for Q1.15 Format PCM Data
// p_handle         input   handle of the SWIP engine
// p_in_buf         input   pointer to the input buffer
// p_in_byte_cnt    input   available input byte count
//                  output  remained input byte count
// p_ou_buf         input   pointer to the output buffer
// p_ou_byte_cnt    input   available output byte count
//                  output  produced output byte count
// return                   produced output byte count
//============================================================//
unsigned int DCR_Process (
    DCRemove_Handle *p_handle,
    short *p_in_buf,
    unsigned int *p_in_byte_cnt,
    short *p_ou_buf,
    unsigned int *p_ou_byte_cnt);

//============================================================//
// Process DC Removal Filter for Q1.31 Format PCM Data
// p_handle         input   handle of the SWIP engine
// p_in_buf         input   pointer to the input buffer
// p_in_byte_cnt    input   available input byte count
//                  output  remained input byte count
// p_ou_buf         input   pointer to the output buffer
// p_ou_byte_cnt    input   available output byte count
//                  output  produced output byte count
// return                   produced output byte count
//============================================================//
unsigned int DCR_Process_24(
    DCRemove_Handle *p_handle,
    long *p_in_buf,
    unsigned int *p_in_byte_cnt,
    long *p_ou_buf,
    unsigned int *p_ou_byte_cnt);

//============================================================//
// Close DC Removal Engine
// p_handle         input   handle of the SWIP engine
//============================================================//
void DCR_Close(DCRemove_Handle *p_handle);

//============================================================//
// Re-configurate DC Removal Engine
// p_handle         input   handle of the SWIP engine
// channel          input   channel number (1 or 2)
// sampling_rate    input   sampling rate (unit: Hz)
// working_mode     input   working mode
// return                   handle of the SWIP engine
//============================================================//
DCRemove_Handle *DCR_ReConfig(
    DCRemove_Handle *p_handle,
    unsigned int channel,
    unsigned int sampling_rate,
    unsigned int working_mode);

#endif
