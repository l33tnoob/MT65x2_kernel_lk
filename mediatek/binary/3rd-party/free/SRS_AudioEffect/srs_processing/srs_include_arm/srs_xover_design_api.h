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
 *  SRS audio crossover filters (Linkwitz¨CRiley crossover) filter design APIs
 *
 *  Authour: Oscar Huang
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/
#ifndef __SRS_XOVER_DESIGN_API_H__
#define __SRS_XOVER_DESIGN_API_H__

#include "srs_typedefs.h"
#include "srs_xover_design_def.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

/*************************************************************************************************
 * Design floating point xover filters
 * Parameters:
 *		cutoff: [IN] The cutoff frequency
 *		order:	[IN] The order of the L-R filter
 *		sampleRate: [IN] The sampling rate
 *		lpfCoefs, hpfCoefs: [OUT] The designed LPF and HPF xover filter coefficients. The filter coefficients are
 *                    returned in the array in the following pattern respectively:

					//{
					//Band 0:
					//Coefficient B0
					//Coefficient B1
					//Coefficient B2
					//Coefficient A1
					//Coefficient A2
					               
					//Band 1:
					//Coefficient B0
					//Coefficient B1
					//Coefficient B2
					//Coefficient A1
					//Coefficient A2

					//...(more bands)
					      
					//Gain
					}
 
 * Return: SRS_NO_ERROR if design succeeds.Otherwise, an error code.

**************************************************************************************************/
SRSResult SRS_XoverFilterDesignFloat(double cutoff, int order, int sampleRate, double *lpfCoefs, double *hpfCoefs);


/*************************************************************************************************
 * Design 32-bit/16-bit fixed point xover filters
 * Parameters:
 *		cutoff: [IN] The cutoff frequency
 *		order:	[IN] The order of the L-R filter
 *		sampleRate: [IN] The sampling rate
 *		lpfCoefs, hpfCoefs: [OUT] The designed LPF and HPF xover filter coefficients. The filter coefficients are
 *                    returned in the array in the following pattern respectively:


					//{
					//Band 0:
					//iwl
					//Coefficient B0
					//Coefficient B1
					//Coefficient B2
					//Coefficient A1
					//Coefficient A2
					               
					//Band 1:
					//iwl
					//Coefficient B0
					//Coefficient B1
					//Coefficient B2
					//Coefficient A1
					//Coefficient A2

					//...(more bands)
					    
					//Gain iwl    
					//Gain
					}
		ws:	[IN] workspace scratch memory, whose size must be at least SRS_XOVER_DESIGN_WORKSPACE_SIZE(order)
 
 * Return: SRS_NO_ERROR if design succeeds.Otherwise, an error code.

**************************************************************************************************/
SRSResult SRS_XoverFilterDesignFxp16(double cutoff, int order, int sampleRate, srs_int16 *lpfCoefs, srs_int16 *hpfCoefs, void *ws);
SRSResult SRS_XoverFilterDesignFxp32(double cutoff, int order, int sampleRate, srs_int32 *lpfCoefs, srs_int32 *hpfCoefs, void *ws);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //__SRS_XOVER_DESIGN_API_H__
