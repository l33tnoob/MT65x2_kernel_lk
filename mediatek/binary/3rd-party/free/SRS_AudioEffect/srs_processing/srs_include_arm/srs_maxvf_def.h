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
 *  SRS MaxVF types, constants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_maxvf/std_fxp/include/srs_maxvf_def.h#2 $
 *  $Author: huixiz $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_MAXVF_DEF_H__
#define __SRS_MAXVF_DEF_H__

#include "srs_typedefs.h"
#include "srs_fft_def.h"
#include "srs_gabor_def.h"

#define SRS_MAXVF_GAIN_IWL						6
#define SRS_MAXVF_BYPASSGAIN_IWL				1
#define SRS_MAXVF_BOOST_IWL				2


typedef struct _SRSMaxVFObj{int _;} *SRSMaxVFObj;

#define SRS_MAXVF_OBJ_SIZE (sizeof(_SRSMaxVFObj_t)+SRS_RFFT_32C16_RDX2_512_SIZE + SRS_GABOR_STEREO_SIZE(SRS_MAXVF_WINDOWSIZE)+sizeof(srs_int32)*(2.5*SRS_MAXVF_FFTSIZE+2*(SRS_MAXVF_MAXBLOCKLENGTH+SRS_MAXVF_DELAYLENGTH+SRS_MAXVF_OFFSET))+16)   //in bytes
#define SRS_MAXVF_WORKSPACE_SIZE (sizeof(srs_int32)*(SRS_MAXVF_BLOCKSIZE+SRS_MAXVF_DELAYLENGTH*3)+8)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
#define SRS_MAXVF_WINDOWSIZE					1024
#define SRS_MAXVF_FFTSIZE						512
#define	SRS_MAXVF_BLOCKSIZE						256
#define SRS_MAXVF_MAXBLOCKLENGTH				SRS_MAXVF_BLOCKSIZE
#define SRS_MAXVF_DELAYLENGTH					12
#define SRS_MAXVF_OFFSET						740


typedef struct
{
	int				Mode;
	srs_int16		InputGain;
	srs_int16		OutputGain;
	srs_int16		BypassGain;

	int				Enable;								//maxvf enable
	int				NMEnable;							//noise manage enable
	int				AcEnable;							//anti-clip enable
	int				HpEnable;							//high pass enable

	srs_int32		AcLevel;							//anti-clip level
	srs_int32		Boost;								//maxvf volume boost
	srs_int32		ScaleLevel;							//maxvf volume adjustment scalelevel
	srs_int32*		MaxVFHPFilterCoefPtr;				//high pass cutoff frequency, I32.2
	srs_int32		MaxVFHpFilternSos;					//number of SOS cascaded
	

}_SRSMaxVFSettings_t;  //48 bytes

typedef struct
{
	//for hardlimiter
	int				DelayLength;						
	int				MyFlag;
	int				Noise;
	srs_int32* 		Dhistory;
	srs_int32		History;
	srs_int32		Level;
	srs_int32		DeltaLevel;
	srs_int32		DLevel;
	srs_int32		TheBoost;
	srs_int32		Boost;
	//for noise manager
	srs_int32		NMThresh;
	//for maxvf
	srs_int32		GH;
	srs_int32*		FFTBuffer;
	srs_int32		MaxValL;
	srs_int32*		Channel ;
	srs_int32*		FFTBufferR;
	srs_int32		MaxValR;
	srs_int32		HL;
	srs_int32		ScaleLevelHistory;					//I32.SRS_MAXVF_SCALELEVEL_IWL
	//srs_int64		MaxVFHpFilter[2][8];
	srs_int32		MaxVFHpFilter[2][8];

}_SRSMaxVFState_t;

typedef struct
{
	_SRSMaxVFSettings_t Settings;
	_SRSMaxVFState_t State;
	SRS_GaborStereoObj GaborStereoObj;
	SRS_GaborMonoObj GaborMonoObj;
	SRSFftTbl*	Ffttbl;

}_SRSMaxVFObj_t;

#endif //__SRS_MAXVF_DEF_H__
