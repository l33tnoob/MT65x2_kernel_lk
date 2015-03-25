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
 *  SRS CSDecoder types, constants
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/

#ifndef __SRS_CSDECODER_DEF_H__
#define __SRS_CSDECODER_DEF_H__

#include "srs_typedefs.h"
#include "srs_monotostereo_def.h"


typedef struct _SRSCSDecoderObj{int _;} *SRSCSDecoderObj;

#define SRS_CSDECODER_OBJ_SIZE					(sizeof(_SRSCSDecoderObj_t) + SRS_MONOTOSTEREO_OBJ_SIZE + 8)
#define SRS_CSDECODER_WORKSPACE_SIZE(blksize)	(sizeof(srs_int32)*(blksize)*17 + 8)

/* CS Decoder Mode definition */
typedef enum
{
	SRS_CSD_MODE_CINEMA	= 1 << 0,
	SRS_CSD_MODE_PRO	= 1 << 1,
	SRS_CSD_MODE_MUSIC	= 1 << 2,
	SRS_CSD_MODE_MONO	= 1 << 3,
	SRS_CSD_MODE_LCRS	= 1 << 4
} SRSCSDecoderMode;

/* CS Decoder Output Mode definition */
typedef enum
{
	SRS_CSD_OUTMODE_STEREO,
	SRS_CSD_OUTMODE_MULTICHS
} SRSCSDecoderOutputMode;

#define SRS_CSD_GAIN_IWL	1	//InputGain iwl


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
typedef struct
{
	/* Project Controls */
	srs_int16				InputGain;			/* CS input gain */
	int						Mode;				/* CS Decoder processing mode */
	srs_int32				Phantom;			/* True for phantom center image */
	srs_int32				CenterFB;			/* True for full bandwidth center channel */
    int						OutputMode;		/* CS Decoder output mode */
	srs_int32				RearCenter;	/* Enable rear center channel */
	srs_int32				RearCenterFB;		/* True for full bandwidth rear center channel */
	const srs_int16*		CoefHighPass100;
	const srs_int16*		CoefHighPass2k;
	const srs_int16*		CoefLowPass2k;
	const srs_int16*		CoefHighPass480;
	const srs_int16*		CoefHighPass16k;
	const srs_int16*		CoefLowPass200;
	const srs_int32*		CoefLowPass80;
	const srs_int16*		CoefLowPass7k;

}_SRSCSDecoderSettings_t;

/*
 *	SRSCSDecoderOptSteeringData defines a data type for conveying all of the steering parameters
 */
typedef struct 
{
	/* Precompute the VCA gains that the ComputeSurroundOutput() function				*/
	/* will use for computing the multi-channel output. The VCA gains are maintained	*/
	/* in the SRSCSDecoderOptSteeringData structure because many implementations		*/
	/* will decimate the steering data and it would be inefficent to compute the VCA	*/
	/* gains each time ComputeSurroundOutput() is called.								*/
	srs_int32	VCA1;
	srs_int32	VCA2;
	srs_int32	VCA3;
	srs_int32	VCA5;
	srs_int32	RVCA5;
	srs_int32	VCA8;
	srs_int32	VCA9;
	srs_int32	VCA10;
	srs_int32	VCA11;
	srs_int32	VCA12;
}SRSCSDecoderSteeringData;

/* Intermediate CSD variables */
typedef struct 
{
	srs_int32	Hb1v10db;
	srs_int32	Lb2v10db;
	srs_int32	Fb2v10dbC;
	srs_int32	FblogdifC;
	srs_int32	Tccntl;

	srs_int32	C_v;
	srs_int32	S_v;
	srs_int32	B_v;
	
	srs_int32	L_ha;
	srs_int32	R_ha;
	srs_int32	Multval;
	
	srs_int32	L_la;
	srs_int32	R_la;
	srs_int32	Lbtcout4;
	
	srs_int32	F_rv;
	srs_int32	F_lv;
	srs_int32	RL;
	
}ComputeSteeringDataVariable;

/* CSD state variables */
typedef struct
{
	srs_int32						Offset;
	srs_int32						LBLogDifSave0;
	srs_int32						LBLogDifSave1;
	srs_int32						LBLogDifSave2;
	srs_int32						LBLogDifSave3;
	srs_int32						LBLogDifSave4;
	srs_int32						LBLogDifSave5;
	srs_int32						LBLogDifSave6;
	srs_int32						LBLogDifSave7;
	srs_int32						FBLogDifCSave0;
	srs_int32						FBLogDifCSave1;
	srs_int32						FBLogDifCSave2;
	srs_int32						FBLogDifCSave3;
	srs_int32						FBLogDifCSave4;
	srs_int32						FBLogDifCSave5;
	srs_int32						FBLogDifCSave6;
	srs_int32						FBLogDifCSave7;
	srs_int32						LRHBLevel0;
	srs_int32						LRHBLevel0Snapshot;
	srs_int32						LRHBLevel1;
	srs_int32						LRHBLevel1Snapshot;
	srs_int32						LRHBLevel2;
	srs_int32						LRHBLevel3;
	srs_int32						LRLBLevel0;
	srs_int32						LRLBLevel0Snapshot;
	srs_int32						LRLBLevel1;
	srs_int32						LRLBLevel1Snapshot;
	srs_int32						LRLBLevel2;
	srs_int32						LRLBLevel3;
	srs_int32						FBLevelC0;
	srs_int32						FBLevelC0Snapshot;
	srs_int32						FBLevelC1;
	srs_int32						FBLevelC1Snapshot;
	srs_int32						FBLevelC2;
	srs_int32						FBLevelC3;
}ComputeSteeringDataState;

/* The function pointer type for the CSD functions */
typedef int (*CsdFunc)(void	*csdecoderObj);

#define	IIR_16K_STATE_SIZE		2
#define	IIR_2K_STATE_SIZE		2
#define	IIR_480_STATE_SIZE		2
#define	IIR_100_STATE_SIZE		2
#define	IIR_7K_STATE_SIZE		4
#define	IIR_80_STATE_SIZE		4
#define	IIR_200_STATE_SIZE		4

/* State structure */
typedef struct
{

	/* Circle Surround steering state information */
	ComputeSteeringDataState	CsdState;
	ComputeSteeringDataVariable	CsdVar;
	SRSCSDecoderSteeringData	SDVar;
	SRSCSDecoderSteeringData	SDOut;
	CsdFunc						NextCsdFunc;
	CsdFunc						NextRearCenterCsdFunc;
	int							DecimationFactor;
	int							SteeringSamplePeriods;
	srs_int32					SteeringSamplePeriodConstant;
	srs_int32					SteeringIncrement;
	int							FrameCounter;
	int							RearCenterFrameCounter;

	/* Storage for the filter state variables (delay elements) */
	srs_int32		CenterHP100		[IIR_100_STATE_SIZE];
	srs_int32		RearHP2k		[IIR_2K_STATE_SIZE];
	srs_int32		RearLP2k		[IIR_2K_STATE_SIZE];
	srs_int32		LeftHP480		[IIR_480_STATE_SIZE];
	srs_int32		RightHP480		[IIR_480_STATE_SIZE];
	srs_int32		LeftHP16k		[IIR_16K_STATE_SIZE];
	srs_int32		RightHP16k		[IIR_16K_STATE_SIZE];
	srs_int32		RightLP200S1	[IIR_200_STATE_SIZE];		/* Front right VCA filter (stage 1) */
	srs_int32		RightLP200S2	[IIR_200_STATE_SIZE];		/* Front right VCA filter (stage 2) */
	srs_int32		LeftLP200S1		[IIR_200_STATE_SIZE];		/* Front left VCA filter  (stage 1) */
	srs_int32		LeftLP200S2		[IIR_200_STATE_SIZE];		/* Front left VCA filter  (stage 2) */
    srs_int32		LeftSLP7k		[IIR_7K_STATE_SIZE];		/* add a 6.9k LP filter for SL */
    srs_int32		RightSLP7k		[IIR_7K_STATE_SIZE];		/* add a 6.9k LP filter for SR */
	srs_int32		SubLP80S1		[IIR_80_STATE_SIZE];		/* Subwoofer lowpass filter (stage 1) */
	srs_int32		SubLP80S2		[IIR_80_STATE_SIZE];		/* Subwoofer lowpass filter (stage 2) */
	srs_int32		lbAdaptive		[1];
	srs_int32		hbAdaptive		[1];
	srs_int32		fbAdaptiveC		[1];
	srs_int32		Vca8Steer		[1];	/* Front to Rear steering filter	*/

	/* Variable filter coefficients */
	srs_int32		lbAdaptiveCoef[2];
	srs_int32		hbAdaptiveCoef[2];
	srs_int32		fbAdaptiveCCoef[2];
	srs_int32		Vca8SteerAttackCoef[2];
	srs_int32		Vca8SteerDecayCoef[2];


}_SRSCSDecoderState_t;

typedef struct
{
	_SRSCSDecoderSettings_t		Settings;
	_SRSCSDecoderState_t		State;
	SRSMonoToStereoObj			MonoToStereo;
	//_SRSTruBassObj_t			TruBassObj;
	//_SRSFocusObj_t			FocusObj;
} _SRSCSDecoderObj_t;

#endif //__SRS_CSDECODER_DEF_H__
