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

/********************************************************************************
 *	SRS Labs CONFIDENTIAL
 *	@Copyright 2010 by SRS Labs.
 *	All rights reserved.
 *
 *  Description:
 *  Basic arithmetic functions header file
 *
 *	Author: Oscar Huang
 *	
 *	(RCS keywords below, do not edit)
 *  $Id: //srstech/srs_common/std_fxp/include/srs_math_api.h#9 $
 *  $Author: zesenz $
 *  $Date: 2011/02/16 $
********************************************************************************/

#ifndef __SRS_MATH_API_H__
#define __SRS_MATH_API_H__

#include "srs_typedefs.h"

//32x16=high 32-bit MSB:
//Note: acc and in32 cannot be the same variable
#define SRS_FXP32X16(acc, in32, in16) do{															\
									acc = (srs_int32)(srs_uint16)(in32) * (srs_int32)(in16);		\
									acc += 0x8000;													\
									acc >>= 16;														\
									acc += ((in32)>>16) * (srs_int32)(in16);						\
								  }while(0)


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

srs_int32  SRS_CountLeadingZeroes(srs_uint32 xIn);
srs_uint32 SRS_FxpSqrt(srs_uint32 xIn);
srs_uint32 SRS_FxpPow2_32(srs_uint32 x);
srs_uint32 SRS_FxpPow2_16(srs_uint16 x);
srs_int32  SRS_FxpLog2(srs_uint32 x);
srs_int32 SRS_CalRecipro(srs_int32 d, int *iwl);

void	SRS_ApplyGain(srs_int32 *audioIO, int blockSize, srs_int16 gain, int gainIwl);	//performs X*gain
void	SRS_ApplyGainWithAnticlip16(srs_int32 *audioIO, int blockSize, srs_int16 gain);	//performs X*gain, then applies anticlip, using 16-bit anticlip table
void	SRS_ApplyGainWithAnticlip32(srs_int32 *audioIO, int blockSize, srs_int16 gain);	//performs X*gain, then applies anticlip, using 32-bit anticlip table

void	SRS_MixAndScale(srs_int32 *audioIO, srs_int32 *y, int blockSize, srs_int16 gain, int gainIwl); //performs (AudioIO+Y)*gain

void	SRS_LeftShift8Bits(srs_int32 *audio, int blockSize);
void	SRS_RightShift8Bits(srs_int32 *audio, int blockSize);

void	SRS_PowXY(srs_int32 *out, int *oiwl, srs_uint32 x, int xiwl, srs_int32 y, int yiwl);

srs_int32	SRS_Div(int oiwl, srs_int32 x, int xiwl, srs_int32 y, int yiwl);


#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /*__SRS_MATH_API_H__*/

