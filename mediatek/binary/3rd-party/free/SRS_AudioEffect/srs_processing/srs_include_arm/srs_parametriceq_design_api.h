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
 *  SRS ParametricEQ filter design APIs
 *
 *  Authour: Oscar Huang
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_designer/std_fxp/include/srs_parametriceq_design_api.h#1 $
 *  $Author: oscarh $
 *  $Date: 2010/09/26 $
 *	
********************************************************************************/
#ifndef __SRS_PARAMETRICEQ_DESIGN_API_H__
#define __SRS_PARAMETRICEQ_DESIGN_API_H__

#include "srs_parametriceq_design_def.h"
#include "srs_typedefs.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

/*************************************************************************************************
 * Design floating point PEQ filters
 * Parameters:
 *		peqSpec: [IN] PEQ specification: sampling rate, the number of bands, bands specs
 *		coefs:	[OUT] The designed filter coefficients. The length of the coefficient array is
 *                    SRS_PEQ_FLOAT_COEFFICIENT_ARRAY_LEN(NumOfBands). The filter coefficients are
 *                    returned in the array in the following pattern:

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
SRSResult SRS_ParametriceqDesignFloat(SRSParametriceqSpec *peqSpec, double *coefs);

/*************************************************************************************************
 * Design 32-bit fixed point PEQ filters
 * Parameters:
 *		peqSpec: [IN] PEQ specification: sampling rate, the number of bands, bands specs
 *		coefs:	[OUT] The designed filter coefficients. The length of the coefficient array is
 *                    SRS_PEQ_FLOAT_COEFFICIENT_ARRAY_LEN(NumOfBands). The filter coefficients are
 *                    returned in the array in the following pattern:

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
		ws:	[IN] workspace scratch memory, whose size must be at least SRS_PEQ_DESIGN_WORKSPACE_SIZE(NumOfBands)
 
 * Return: SRS_NO_ERROR if design succeeds.Otherwise, an error code.

**************************************************************************************************/
SRSResult SRS_ParametriceqDesignFxp32(SRSParametriceqSpec *peqSpec, srs_int32 *coefs, void *ws);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //__SRS_PARAMETRICEQ_DESIGN_API_H__
