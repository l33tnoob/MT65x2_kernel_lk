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
 *  SRS TruVolumeF types, constants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_truvf/std_fxp/include/srs_truvf_def.h#1 $
 *  $Author: oscarh $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_TRUVF_DEF_H__
#define __SRS_TRUVF_DEF_H__

#include "srs_typedefs.h"
#include "srs_fft_def.h"
#include "srs_gabor_def.h"


typedef struct _SRSTruVFObj{int _;} *SRSTruVFObj;


#define SRS_TRUVF_OBJ_SIZE 	(sizeof(_SRSTruVFObj_t) + SRS_RFFT_32C16_RDX2_512_SIZE + \
															SRS_GABOR_STEREO_SIZE(SRS_TRUVF_WINDOWSIZE)+ \
															sizeof(srs_int32)*(2.5*SRS_TRUVF_FFTSIZE+2*(SRS_TRUVF_MAXBLOCKLENGTH+SRS_TRUVF_DELAYLENGTH+SRS_TRUVF_OFFSET))+ \
															16)   //in bytes
															
#define SRS_TRUVF_WORKSPACE_SIZE 	(sizeof(srs_int32)*(SRS_TRUVF_BLOCKSIZE+SRS_TRUVF_DELAYLENGTH*3)+8)		//in bytes

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
//currently only support windowsize = 1024, fftsize = 512, blocksize = 256
#define SRS_TRUVF_WINDOWSIZE					1024
#define SRS_TRUVF_FFTSIZE						512
#define	SRS_TRUVF_BLOCKSIZE						256
#define SRS_TRUVF_MAXBLOCKLENGTH				SRS_TRUVF_BLOCKSIZE
#define SRS_TRUVF_DELAYLENGTH					12
#define SRS_TRUVF_OFFSET						740

#define SRS_TRUVF_GAIN_IWL						6
#define SRS_TRUVF_BYPASSGAIN_IWL				1


typedef struct
{
	int				Mode;
	srs_int16		InputGain;
	srs_int16		OutputGain;
	srs_int16		BypassGain;

	int				Enable;								//truvf enable
	int				NMEnable;							//noise manage enable
	
	//srs_int32		Boost;								//truvf volume boost
	srs_int32		ScaleLevel;							//truvf volume adjustment scalelevel

}_SRSTruVFSettings_t; //24 bytes

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
	//for truvf
	srs_int32		GH;
	srs_int32*		FFTBuffer;
	srs_int32		MaxValL;
	srs_int32*		Channel ;
	srs_int32*		FFTBufferR;
	srs_int32		MaxValR;
	srs_int32		HL;
	srs_int32		ScaleLevelHistory;					//I32.SRS_TRUVF_SCALELEVEL_IWL

}_SRSTruVFState_t;//76 bytes

typedef struct
{
	_SRSTruVFSettings_t Settings;
	_SRSTruVFState_t State;
	SRS_GaborStereoObj GaborStereoObj;
	SRS_GaborMonoObj GaborMonoObj;
	SRSFftTbl*	Ffttbl;

}_SRSTruVFObj_t;

#endif //__SRS_TRUVF_DEF_H__
