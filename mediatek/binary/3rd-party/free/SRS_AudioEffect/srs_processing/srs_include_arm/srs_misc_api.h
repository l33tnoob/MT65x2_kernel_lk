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
 *  Header file for miscellaneous functions & macros
 *
 *	Author: Oscar Huang
 *	
 *	(RCS keywords below, do not edit)
 *  $Id: //srstech/srs_common/std_fxp/include/srs_misc_api.h#13 $
 *  $Author: zesenz $
 *  $Date: 2011/02/22 $
********************************************************************************/

#ifndef __SRS_MISC_API_H
#define __SRS_MISC_API_H

#include "srs_typedefs.h"

#define SRS_INLINE	inline					/* if we encounter some platforms which do not support inline, we cant disable them */

#define SRS_ALIGNON8(p)		((void*)(((srs_int32)(p)+7)&0xFFFFFFF8))	//adjust pointer p to be aligned on 8

#ifdef __cplusplus 
	#define SRS_EXTERNC extern "C"
#else
	#define SRS_EXTERNC
#endif /*__cplusplus*/

//Macro to define library version string and version get function:
//Usage example: SRS_DEFINE_LIB_VERSION(TruBass, 2, 1, 0, 100)
#define SRS_DEFINE_LIB_VERSION(tech, major, minor, rev, release)	SRS_DEFINE_LIB_VERSION_CORE(tech, major, minor, rev, release)
#define SRS_DEFINE_LIB_VERSION_CORE(tech, major, minor, rev, release) \
	SRS_EXTERNC const unsigned char SRS_##tech##_Lib_Ver_##major##_##minor##_##rev##_##release[] =  {major, minor, rev, release}; \
	SRS_EXTERNC unsigned char SRS_Get##tech##LibVersion(SRSVersion which) \
	{ \
		return SRS_##tech##_Lib_Ver_##major##_##minor##_##rev##_##release[which]; \
	}

//Macro to define technology version string and version get function:
//Usage example: SRS_DEFINE_TECH_VERSION(TruBass, 2, 1, 0, 100)
#define SRS_DEFINE_TECH_VERSION(tech, major, minor, rev, release)		SRS_DEFINE_TECH_VERSION_CORE(tech, major, minor, rev, release)
#define SRS_DEFINE_TECH_VERSION_CORE(tech, major, minor, rev, release) \
	SRS_EXTERNC const unsigned char SRS_##tech##_Tech_Ver_##major##_##minor##_##rev##_##release[] =  {major, minor, rev, release}; \
	SRS_EXTERNC unsigned char SRS_Get##tech##TechVersion(SRSVersion which) \
	{ \
		return SRS_##tech##_Tech_Ver_##major##_##minor##_##rev##_##release[which]; \
	}

//Macro to check version compatibility of a component:
#define SRS_VERSION_COMPATIBLE(GetVerFunc, major, minor) \
	(GetVerFunc(SRS_VERSION_MAJOR)==(major) && ((minor)?(GetVerFunc(SRS_VERSION_MINOR)>=(minor)):1))

//Macro to computes structure member address offset:
//structure: the name of the structure
//member: the name of the member whose offset is to be computed.
//e.g: to get the offset of OutputGain in structure _SRSTruBassSettings_t:
//#define OUTPUT_GAIN_OFFSET  SRS_STRUCT_MEM_OFFSET(_SRSTruBassSettings_t, OutputGain)
#define SRS_STRUCT_MEM_OFFSET(structure, member)	((int)(&((structure*)0)->member))

#define SRS_MAX(a, b)  ((a)>=(b)?(a):(b))
#define SRS_MIN(a, b)  ((a)<=(b)?(a):(b))

/* left shift with saturation, that is, Saturate(x<<iwl), x's type is srs_int32  */
#define SRS_SAT_LEFTSHIFT(x, iwl)	( (((x)<<(iwl)) >> (iwl)) != (x) ? ((x)>>31) ^ 0x7FFFFFFF : (x)<<(iwl) )

/* Note: whether the macro should be kept will be discussed later, now the macro is defined to facilitate zesen's code clean up job */
#define MUL64TO32(a, b)		((srs_int32)( ((srs_int64)(a) * (srs_int64)(b) + 0x40000000) >>31 ))

//calculate the absolute value of x
SRS_INLINE srs_int32 SRS_Abs(srs_int32 x)
{
	srs_int32 out;

	out = (x) - ((x)<0);
	out = (out) ^ ((out)>>31);

	return out;
}

//saturation result of a+b
SRS_INLINE srs_int32 SRS_SatAdd(srs_int32 a, srs_int32 b)
{
	srs_int32 out;

	out = a+b;
	if(!( (a>>31)^(b>>31) )){
		if((out>>31)^(a>>31)){
			out = (a>>31) ^ 0x7FFFFFFF;
		}
	}

	return out;
}

//saturation result of a-b
SRS_INLINE srs_int32 SRS_SatSub(srs_int32 a, srs_int32 b)
{
	srs_int32 out;

	out = a-b;
	if((a>>31)^(b>>31)){
		if((out>>31)^(a>>31)){
			out = (a>>31) ^ 0x7FFFFFFF;
		}
	}

	return out;
}

/* Calculate Saturate( ((srs_int64)a * (srs_int64)b) >> (32-iwl) )
 * if a in I32.1, b in I32.iwl, then return value in I32.1
 */
SRS_INLINE srs_int32 SRS_Mul64To32(srs_int32 a, srs_int32 b, int iwl)
{
	srs_int64 acc;
	srs_int32 hi32, acctmp;
	srs_uint32 lo32;

	acc = (srs_int64)(a) * (srs_int64)(b);
	acc += (0x01LL<<(32-iwl-1));
	hi32 = (srs_int32)(acc>>32);
	lo32 = (srs_uint32)acc;

	acctmp = (hi32<<iwl);
	lo32 = (srs_uint32)(((srs_uint64)lo32)>>(32-iwl));
	acctmp = acctmp | lo32;
	if(((hi32<<iwl)>>iwl) != hi32){							//overflow
		acctmp = 0x7FFFFFFF ^ (hi32>>31);
	}
		
	return acctmp;
}

/*
 * A inline function version of macro SRS_FXP32X16, after replacing all SRS_FXP32X16
 * with this inline function, SRS_FXP32X16 will be removed
 * Calculate a*b, return high 32 bits
 */
SRS_INLINE srs_int32 SRS_Mul32x16(srs_int32 a, srs_int16 b)
{
	srs_int32 acc;

	acc = (srs_int32)(srs_uint16)(a) * (srs_int32)(b);
	acc += 0x8000;
	acc >>= 16;
	acc += ((a)>>16) * (srs_int32)(b);

	return acc;
}

//32x32, return high 32 bits
SRS_INLINE srs_int32 SRS_Mul32x32(srs_int32 x, srs_int32 y)
{
	srs_int64 tmp;

	tmp = (srs_int64)x * (srs_int64)y;

	return (srs_int32)(tmp>>32);
}

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

void SRS_CopyInt32Vector(srs_int32 *src, srs_int32 *dest, int n);
void SRS_MemSet(void *mem, int size, char val); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif //__SRS_MISC_API_H
