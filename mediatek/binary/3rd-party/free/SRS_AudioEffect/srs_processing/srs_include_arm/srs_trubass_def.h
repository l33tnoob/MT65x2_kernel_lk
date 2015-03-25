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
 *  SRS TruBass types, constants
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/

#ifndef __SRS_TRUBASS_DEF_H__
#define __SRS_TRUBASS_DEF_H__

#include "srs_typedefs.h"

typedef struct _SRSTruBassObj{int _;} *SRSTruBassObj;

#define SRS_TRUBASS_OBJ_SIZE		(sizeof(_SRSTruBassObj_t)+8)
#define SRS_TRUBASS_WORKSPACE_SIZE(blockSize)	(5*blockSize*sizeof(srs_int32)+8)	//in bytes

#define SRS_TB_GAIN_IWL		1	//iwl of input gain, output gain and bypass gain.
#define SRS_TB_CTRL_IWL		1	//iwl of Trubass Control

typedef enum
{
	SRS_TB_SPEAKER_LF_RESPONSE_40Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_60Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_100Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_150Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_200Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_250Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_300Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_400Hz,
	SRS_TB_SPEAKER_LF_RESPONSE_Custom,
	SRS_TB_SPEAKER_LF_RESPONSE_SIZES
} SRSTruBassSpeakerSize;

typedef struct
{
	srs_int16	LowPassFilterCoef[2]; //B0, A1
	srs_int32	LowBandFilterCoef[3]; //B0, A1, A2
	srs_int32	MidBandFilterCoef[3]; //B0, A1, A2
} SRSTruBassCustomSpeakerCoefs;

typedef enum
{
	SRS_TB_MODE_MONO,
	SRS_TB_MODE_STEREO,
	SRS_TB_MODE_MODES
} SRSTruBassMode;

typedef enum
{
	SRS_TB_OUTPUT_MIXED,		//Output the left channel and the right channel mixed with the bass component.
	SRS_TB_OUTPUT_SEPARATED		//Output the left channel, the right channel and the bass component separately.
} SRSTruBassOutputOption;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:

typedef struct{
	int							Enable;
	srs_int16					InputGain;
	srs_int16					OutputGain;
	srs_int16					BypassGain;
	srs_int16					TruBassCtrl;		//Bass enhancement control
	srs_int32					IntegrationLimit;	//=2 + truBassControl * 6, value range (2, 8), I32.4

	int							SpeakerSize;		//SRSTruBassSpeakerSize, LF response of the speaker in Hz
	int							Mode;				//SRSTruBassMode, Trubass processing mode: mono or stereo
	int							OutputOption;		//SRSTruBassOutputOption, option for final results output: mix bass with channel data or separate outputs 

	//Storage for filter coefficient pointers that depend on the sampling rate and speaker size:
	const srs_int16				*LowPassFilterCoef;	//I16.1
	const srs_int32				*LowBandFilterCoef;	//I32.2
	const srs_int32				*MidBandFilterCoef;	//I32.2

	srs_int32					FasdAttack;			//FASD attack coefficient (sampling rate dependent) I32.1 (I16 is not sufficient...)
	srs_int32					FasdDecay;			//FASD decay coefficient  (sampling rate dependent) I32.1

	//Storage for filter coefficient pointers to be initialized according to the sampling rate:
	//Speak size 40Hz:
	const srs_int16				*LowPassFilter40Hz;
	const srs_int32				*LowBandFilter40Hz;
	const srs_int32				*MidBandFilter40Hz;

	//Speak size 60Hz:
	const srs_int16				*LowPassFilter60Hz;
	const srs_int32				*LowBandFilter60Hz;
	const srs_int32				*MidBandFilter60Hz;

	//Speak size 100Hz:
	const srs_int16				*LowPassFilter100Hz;
	const srs_int32				*LowBandFilter100Hz;
	const srs_int32				*MidBandFilter100Hz;

	//Speak size 150Hz:
	const srs_int16				*LowPassFilter150Hz;
	const srs_int32				*LowBandFilter150Hz;
	const srs_int32				*MidBandFilter150Hz;

	//Speak size 200Hz:
	const srs_int16				*LowPassFilter200Hz;
	const srs_int32				*LowBandFilter200Hz;
	const srs_int32				*MidBandFilter200Hz;

	//Speak size 250Hz:
	const srs_int16				*LowPassFilter250Hz;
	const srs_int32				*LowBandFilter250Hz;
	const srs_int32				*MidBandFilter250Hz;

	//Speak size 300Hz:
	const srs_int16				*LowPassFilter300Hz;
	const srs_int32				*LowBandFilter300Hz;
	const srs_int32				*MidBandFilter300Hz;

	//Speak size 400Hz:
	const srs_int16				*LowPassFilter400Hz;
	const srs_int32				*LowBandFilter400Hz;
	const srs_int32				*MidBandFilter400Hz;

	//Custom TruBass filter coefficients:
	SRSTruBassCustomSpeakerCoefs	CustomSpeakerCoefs;
	
} _SRSTruBassSettings_t;

typedef struct{
	srs_int32	LowPassFilter1; //	
	srs_int32	LowPassFilter2;
	srs_int32	LowBandFilter[2];
	srs_int32	MidBandFilter[2];
	srs_int32	FasdLevel;
} _SRSTBChannelState_t;		//channel dependent state

typedef struct{
	srs_int32	Peak;		//I32.1
	srs_int32	Valley;		//I32.1
	srs_int32	Multiplier;	//I32.kSrsTrubassMultiplierIwl
	srs_int32	PeakHold;	//I32.1
	srs_int32	NextPeak;	//I32.1
	int		PeakHoldCount;
	int		PeakSampleCount;	//Sampling rate dependent const
	int		LastMoveUp;
} _SRSTBPeakTrackerState_t;	//the peak tracker state

typedef struct{
	_SRSTBChannelState_t			LeftChState;
	_SRSTBChannelState_t			RightChState;
	_SRSTBPeakTrackerState_t		PeakTrackerState;
	srs_int32					Integrator;	//I32.4
} _SRSTruBassState_t;

typedef struct{
	_SRSTruBassState_t			State;
	_SRSTruBassSettings_t		Settings;
} _SRSTruBassObj_t;

#endif //__SRS_TRUBASS_DEF_H__
