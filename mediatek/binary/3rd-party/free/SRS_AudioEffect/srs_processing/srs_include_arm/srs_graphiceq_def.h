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
 *  SRS GraphicEQ types, constants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_graphiceq/std_fxp/include/srs_graphiceq_def.h#3 $
 *  $Author: oscarh $
 *  $Date: 2010/10/21 $
 *	
********************************************************************************/

#ifndef __SRS_GRAPHICEQ_DEF_H__
#define __SRS_GRAPHICEQ_DEF_H__

#include "srs_typedefs.h"
#include "srs_fxp.h"
#include "srs_limiter_def.h"


//Integer word length of band gains:
#define SRS_GEQ_BAND_GAIN_IWL		4	//extend to support +/-15dB range

//dB constants for GEQ band gains:
#define SRS_GEQ_MINUS_15DB			SRS_FXP16(0.177, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_14DB			SRS_FXP16(0.200, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_13DB			SRS_FXP16(0.224, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_12DB			SRS_FXP16(0.251, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_11DB			SRS_FXP16(0.282, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_10DB			SRS_FXP16(0.316, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_09DB			SRS_FXP16(0.355, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_08DB			SRS_FXP16(0.40, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_07DB			SRS_FXP16(0.45, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_06DB			SRS_FXP16(0.50, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_05DB			SRS_FXP16(0.56, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_04DB			SRS_FXP16(0.63, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_03DB			SRS_FXP16(0.71, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_02DB			SRS_FXP16(0.79, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_MINUS_01DB			SRS_FXP16(0.89, SRS_GEQ_BAND_GAIN_IWL)

#define SRS_GEQ_0DB					SRS_FXP16(1.00, SRS_GEQ_BAND_GAIN_IWL)

#define SRS_GEQ_PLUS_01DB			SRS_FXP16(1.12, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_02DB			SRS_FXP16(1.26, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_03DB			SRS_FXP16(1.41, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_04DB			SRS_FXP16(1.58, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_05DB			SRS_FXP16(1.78, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_06DB			SRS_FXP16(2.00, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_07DB			SRS_FXP16(2.24, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_08DB			SRS_FXP16(2.51, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_09DB			SRS_FXP16(2.82, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_10DB			SRS_FXP16(3.16, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_11DB			SRS_FXP16(3.55, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_12DB			SRS_FXP16(4.00, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_13DB			SRS_FXP16(4.47, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_14DB			SRS_FXP16(5.01, SRS_GEQ_BAND_GAIN_IWL)
#define SRS_GEQ_PLUS_15DB			SRS_FXP16(5.63, SRS_GEQ_BAND_GAIN_IWL)


//5-band:
#define SRS_GEQ_5BAND_NUM_OF_BANDS		5

typedef enum
{
	SRS_GEQ_5BAND_100HZ,
	SRS_GEQ_5BAND_300HZ,
	SRS_GEQ_5BAND_1000HZ,
	SRS_GEQ_5BAND_3000HZ,
	SRS_GEQ_5BAND_10000HZ
} SRSGeq5BandCenterFreq;

//Constants for maximum and minimum gain in each band:
#define SRS_5BAND_GEQ_BAND_GAIN_MAX				SRS_GEQ_PLUS_15DB
#define SRS_5BAND_GEQ_BAND_GAIN_MIN				SRS_GEQ_MINUS_15DB

typedef enum
{
	SRS_5BAND_GEQ_EXTRA_BAND_BEHAVIOR_DEFAULT,
	SRS_5BAND_GEQ_EXTRA_BAND_BEHAVIOR_UNITY,
	SRS_5BAND_GEQ_EXTRA_BAND_BEHAVIOR_SHELF
} SRS5BandGeqExtraBandBehavior;


typedef struct _SRS5BandGraphicEqObj{int _;}	*SRS5BandGraphicEqObj;

#define SRS_5BAND_GRAPHICEQ_OBJ_SIZE	(sizeof(_SRS5BandGraphicEqObj_t)+8+SRS_LIMITER_OBJ_SIZE)
#define	SRS_5BAND_GRAPHICEQ_WORKSPACE_SIZE(blockSize)	(sizeof(srs_int32)*(blockSize)*2+8)

//10-band:
#define SRS_GEQ_10BAND_NUM_OF_BANDS		10

typedef enum
{
	SRS_GEQ_10BAND_31HZ,
	SRS_GEQ_10BAND_62HZ,
	SRS_GEQ_10BAND_125HZ,
	SRS_GEQ_10BAND_250HZ,
	SRS_GEQ_10BAND_500HZ,
	SRS_GEQ_10BAND_1000HZ,
	SRS_GEQ_10BAND_2000HZ,
	SRS_GEQ_10BAND_4000HZ,
	SRS_GEQ_10BAND_8000HZ,
	SRS_GEQ_10BAND_16000HZ,
} SRSGeq10BandCenterFreq;

//Constants for maximum and minimum gain in each band:
#define SRS_10BAND_GEQ_BAND_GAIN_MAX				SRS_GEQ_PLUS_15DB
#define SRS_10BAND_GEQ_BAND_GAIN_MIN				SRS_GEQ_MINUS_15DB

typedef struct _SRS10BandGraphicEqObj{int _;}	*SRS10BandGraphicEqObj;

#define SRS_10BAND_GRAPHICEQ_OBJ_SIZE	(sizeof(_SRS10BandGraphicEqObj_t)+8+SRS_LIMITER_OBJ_SIZE)
#define	SRS_10BAND_GRAPHICEQ_WORKSPACE_SIZE(blockSize)	(sizeof(srs_int32)*(blockSize)+8)

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
typedef struct
{
	int					Enable;
	int					NumOfInternalBands;
	int					ExtraBandBehavior;	//0: default. 1: Unity behavior, 2: Shelf behavior
	int					MakeResponseFlat;	
	const srs_int16		*WarpCoef;		//rho, the number of rho's is the number of interal bands
	const srs_int16		*GainModifier;  //the number of gain modifiers is the number of internal bands, I16.ModifierIwl
	int					BitGrowth;		//input signal: I32.N, output signal: I32.(N+bitGrowth) if no last stage scale.
	srs_int16			BandGain[SRS_GEQ_5BAND_NUM_OF_BANDS];		//the number of band gains is the number of user bands, I16.SRS_GEQ_BAND_GAIN_IWL
	srs_int16			ModifiedBandGain[SRS_GEQ_5BAND_NUM_OF_BANDS+2]; //size is the number of internal bands, I16.(SRS_GEQ_BAND_GAIN_IWL+ModifierIwl-1)
	int					GainUpdated;
	int					BlockSize;
	srs_int32			InvBlockSize;	//1/BlockSize in I32.1
} _SRS5BandGraphicEqSettings_t;

typedef struct
{
	srs_int32			Z[6*(SRS_GEQ_5BAND_NUM_OF_BANDS+2)];			//filter delay buffers
} _SRS5BandGraphicEqState_t;

typedef struct
{
	SRSLimiterObj						Limiter;
	_SRS5BandGraphicEqSettings_t		Settings;
	_SRS5BandGraphicEqState_t			State;
} _SRS5BandGraphicEqObj_t;
	

typedef struct
{
	int					Enable;
	int					NumOfInternalBands;
	const srs_int16		*WarpCoef;		//rho, the number of rho's is the number of interal bands
	const srs_int16		*GainModifier;  //the number of gain modifiers is the number of internal bands, I16.ModifierIwl;
	int					BitGrowth;		//input signal: I32.N, output signal: I32.(N+bitGrowth) if no last stage scale.
	srs_int16			BandGain[SRS_GEQ_10BAND_NUM_OF_BANDS];		//the number of band gains is the number of user bands, I16.SRS_GEQ_BAND_GAIN_IWL
	srs_int16			ModifiedBandGain[SRS_GEQ_10BAND_NUM_OF_BANDS]; //size is the number of internal bands, I16.(SRS_GEQ_BAND_GAIN_IWL+ModifierIwl-1)
} _SRS10BandGraphicEqSettings_t;

typedef struct
{
	srs_int32			Z[6*SRS_GEQ_10BAND_NUM_OF_BANDS];			//filter delay buffers
} _SRS10BandGraphicEqState_t;

typedef struct
{
	SRSLimiterObj						Limiter;
	_SRS10BandGraphicEqSettings_t		Settings;
	_SRS10BandGraphicEqState_t			State;
} _SRS10BandGraphicEqObj_t;
	
#endif /*__SRS_GRAPHICEQ_DEF_H__*/
