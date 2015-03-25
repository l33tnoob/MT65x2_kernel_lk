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
 *  The header file defines SRS fixed-point related data types and macros
 *
 *	Author: Oscar Huang
 *	
 *	(RCS keywords below, do not edit)
 *  $Id: //srstech/srs_common/std_fxp/include/srs_fxp.h#2 $
 *  $Author: oscarh $
 *  $Date: 2010/06/02 $
********************************************************************************/

#ifndef __SRS_FXP_H__
#define __SRS_FXP_H__

#include "srs_typedefs.h"

typedef enum
{
	SRS_RND_INF,				// Rounding to infinity, (a.k.a round to nearest)
	SRS_TRN_ZERO				// Truncation to zero (a.k.a magnitude truncation, floating to integer conversion in C uses this mode)
} SRSQuantizationMode;

#define _SIGNED_HALF(val)	((val)>=0? 0.5:-0.5)

#define _MININTVAL(wl)		((srs_int64)~0<<((wl)-1))		//min value of the integer with word length wl
#define _MAXINTVAL(wl)		(~_MININTVAL(wl))				//max value of the integer with word length wl

#define _CLIP(val, wl)		((val)> _MAXINTVAL(wl)? _MAXINTVAL(wl):((val)<_MININTVAL(wl)? _MININTVAL(wl):val))

#define _FLOAT_VAL_OF_FXP_REP(val, wl, iwl)		((val)*((srs_int64)1<<((wl)-(iwl))))

#define SRS_FXP16(val, iwl)		((srs_int16)_CLIP(_FLOAT_VAL_OF_FXP_REP(val, 16, iwl) + _SIGNED_HALF(val), 16))		//convert to 16-bit fxp with SRS_RND_INF mode
#define SRS_FXP32(val, iwl)		((srs_int32)_CLIP(_FLOAT_VAL_OF_FXP_REP(val, 32, iwl) + _SIGNED_HALF(val), 32))		//convert to 32-bit fxp with SRS_RND_INF mode


#endif //__SRS_FXP_H__
