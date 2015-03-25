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
 *  SRS Split Analysis TruBass types, constants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_sa_trubass/std_fxp/include/srs_sa_trubass_def.h#2 $
 *  $Author: oscarh $
 *  $Date: 2011/02/15 $
 *	
********************************************************************************/

#ifndef __SRS_SA_TRUBASS_DEF_H__
#define __SRS_SA_TRUBASS_DEF_H__

#include "srs_typedefs.h"


typedef struct _SRSSATruBassObj{int _;} *SRSSATruBassObj;

#define SRS_SA_TRUBASS_OBJ_SIZE		(sizeof(_SRSSATruBassObj_t)+8)
#define SRS_SA_TRUBASS_WORKSPACE_SIZE(blockSize)	(5*blockSize*sizeof(srs_int32)+8)	//in bytes

#define SRS_SATB_GAIN_IWL		1	//iwl of input gain, output gain and bypass gain.
#define SRS_SATB_CTRL_IWL		1	//iwl of Trubass Control


typedef enum
{
	SRS_SATB_SPEAKER_LF_RESPONSE_40Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_60Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_100Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_150Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_200Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_250Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_300Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_400Hz,
	SRS_SATB_SPEAKER_LF_RESPONSE_Custom,
	SRS_SATB_SPEAKER_LF_RESPONSE_SIZES
} SRSSATruBassSpeakerLFResponse;

//Properties describing speakers:
typedef struct
{
	int			AudioFilter;	//SRSSATruBassSpeakerLFResponse, Frequency response of the audio filter
	int			AnalysisFilter;	//SRSSATruBassSpeakerLFResponse, Frequency response of the analysis filter
} SRSSATruBassSpeakerSize;


typedef union
{
	struct
	{
		srs_int32	LowPassAudioFilterCoef[2];
		srs_int32	LowBandAudioFilterCoef[3];
		srs_int32	MidBandAudioFilterCoef[3];
		srs_int32	LowPassAnalysisFilterCoef[2];
		srs_int32	LowBandAnalysisFilterCoef[3];
		srs_int32	MidBandAnalysisFilterCoef[3];
	} Struct;
	srs_int32	Array[16];
} SRSSATruBassCustomSpeakerCoefs;

typedef enum
{
	SRS_SATB_MODE_MONO,
	SRS_SATB_MODE_STEREO,
	SRS_SATB_MODE_NUM
} SRSSATruBassMode;

typedef enum
{
	SRS_SATB_OUTPUT_MIXED,		//Output the left channel and the right channel mixed with the bass component.
	SRS_SATB_OUTPUT_SEPARATED		//Output the left channel, the right channel and the bass component separately.
} SRSSATruBassOutputOption;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
typedef struct{
	int							Enable;
	int							LevelIndependentEnable;	//enable level independent processing or not (new)
	int							SplitAnalysisEnable;	//If split analysis is disabled, it reduces to the traditional TruBass (Audio filter used, Analysis filter not referenced)
	srs_int16					InputGain;
	srs_int16					OutputGain;
	srs_int16					BypassGain;
	srs_int16					TruBassCtrl;		//Bass enhancement control
	srs_int16					CompressorCtrl;		//Compressor control (new)
	srs_int32					IntegrationLimit;	//Linked with CompressorCtrl by IntegrationLimit=32*CompressorCtrl

	SRSSATruBassSpeakerSize		SpeakerSize;		//LF response of the speaker in Hz: Audio Filter & Analysis Filter
	int							Mode;				//SRSSATruBassMode, Trubass processing mode: mono or stereo
	int							OutputOption;		//SRSSATruBassOutputOption, option for final results output: mix bass with channel data or separate outputs 

	//Storage for filter coefficient pointers that depend on the sampling rate and speaker size:
	const srs_int32					*LowPassAudioFilterCoef;	//I32.1
	const srs_int32					*LowBandAudioFilterCoef;	//I32.2
	const srs_int32					*MidBandAudioFilterCoef;	//I32.2
	const srs_int32					*LowPassAnalysisFilterCoef;	//Not used in this "shared lowpass" version, I32.1
	const srs_int32					*LowBandAnalysisFilterCoef; //I32.2
	const srs_int32					*MidBandAnalysisFilterCoef;	//I32.2

	SRSSATruBassCustomSpeakerCoefs		CustomSpeakerCoefs;	//Storage for custom speaker filter coefficients.

	srs_int32					FasdAttack;			//FASD attack coefficient (sampling rate dependent) I32.1 (I16 is not sufficient...)
	srs_int32					FasdDecay;			//FASD decay coefficient  (sampling rate dependent) I32.1

	//Storage for filter coefficient pointers to be initialized according to the sampling rate:
	//Speak size 40Hz:
	const srs_int32				*LowPassFilter40Hz;
	const srs_int32				*LowBandFilter40Hz;
	const srs_int32				*MidBandFilter40Hz;

	//Speak size 60Hz:
	const srs_int32				*LowPassFilter60Hz;
	const srs_int32				*LowBandFilter60Hz;
	const srs_int32				*MidBandFilter60Hz;

	//Speak size 100Hz:
	const srs_int32				*LowPassFilter100Hz;
	const srs_int32				*LowBandFilter100Hz;
	const srs_int32				*MidBandFilter100Hz;

	//Speak size 150Hz:
	const srs_int32				*LowPassFilter150Hz;
	const srs_int32				*LowBandFilter150Hz;
	const srs_int32				*MidBandFilter150Hz;

	//Speak size 200Hz:
	const srs_int32				*LowPassFilter200Hz;
	const srs_int32				*LowBandFilter200Hz;
	const srs_int32				*MidBandFilter200Hz;

	//Speak size 250Hz:
	const srs_int32				*LowPassFilter250Hz;
	const srs_int32				*LowBandFilter250Hz;
	const srs_int32				*MidBandFilter250Hz;

	//Speak size 300Hz:
	const srs_int32				*LowPassFilter300Hz;
	const srs_int32				*LowBandFilter300Hz;
	const srs_int32				*MidBandFilter300Hz;

	//Speak size 400Hz:
	const srs_int32				*LowPassFilter400Hz;
	const srs_int32				*LowBandFilter400Hz;
	const srs_int32				*MidBandFilter400Hz;

	
} _SRSSATruBassSettings_t;

typedef struct
{
	srs_int32	LowPassFilter1; //	
	srs_int32	LowPassFilter2;
	srs_int32	LowBandFilter[2];
	srs_int32	MidBandFilter[2];
} _SRSSATBFilterState_t;

typedef struct{
	srs_int32	Peak;		//I32.1
	srs_int32	Valley;		//I32.1
	srs_int32	Multiplier;	//I32.kSrsTrubassMultiplierIwl
	srs_int32	PeakHold;	//I32.1
	srs_int32	NextPeak;	//I32.1
	int		PeakHoldCount;
	int		PeakSampleCount;	//Sampling rate dependent const
	int		LastMoveUp;
} _SRSSATBPeakTrackerState_t;	//the peak tracker state

typedef struct
{
	_SRSSATBFilterState_t	AudioFilterState;
	_SRSSATBFilterState_t	AnalysisFilterState;
	srs_int32				FasdLevel;
} _SRSSATBChannelState_t;		//channel dependent state


typedef struct{
	_SRSSATBChannelState_t			LeftChState;
	_SRSSATBChannelState_t			RightChState;
	_SRSSATBPeakTrackerState_t		PeakTrackerState;
	srs_int32						Integrator;	//I32.6
} _SRSSATruBassState_t;

typedef struct{
	_SRSSATruBassState_t			State;
	_SRSSATruBassSettings_t		Settings;
} _SRSSATruBassObj_t;

#endif //__SRS_SA_TRUBASS_DEF_H__
