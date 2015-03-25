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
 *  SRS Gabor types, constants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_gabor/std_fxp/include/srs_gabor_def.h#1 $
 *  $Author: oscarh $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_GABOR_DEF_H__
#define __SRS_GABOR_DEF_H__

#include "srs_typedefs.h"
#include "srs_fft_def.h"

typedef struct _SRSGaborStereoObj{int _;}	*SRS_GaborStereoObj;
typedef struct _SRSGaborMonoObj{int _;}		*SRS_GaborMonoObj;

#define SRS_GABOR_STEREO_SIZE(windowsize) (sizeof(_SRSGaborStereoObj_t)+sizeof(srs_int32)*(windowsize)*4)   //define the memory size for GaborObj
#define SRS_GABOR_MONO_SIZE(windowsize) (sizeof(_SRSGaborMonoObj_t)+sizeof(srs_int32)*(windowsize)*2)   //define the memory size for GaborObj

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
typedef struct 
{
     int WindowSize;		//window length used for each frame
     int FFTSize;			//number of frequency bins for Gabor coefficients
     int BlockSize;			//the time sampling step of the Gabor coefficients, must be smaller than FFTSize
	 
	 srs_int32 *AnalysisWin;		//Analysis window
	 srs_int32 *SynthesisWin;	//Synthesis window

	 SRSFftTbl *Ffttbl;		//FFT structure

}_SRSGaborSettings_t;

typedef struct
{
     srs_int32 *ShiftIn;		//FIFO buffer for storing input signal, with lenght of window_size
     srs_int32 *ShiftOut;		//FIFO buffer for storing output signal, with lenght of window_size

	 srs_int32 *InStart;		//shift-in buffer start pointer
	 srs_int32 *OutStart;		//shift-out buffer start pointer


}_SRSGaborState_t;

typedef struct
{
     _SRSGaborSettings_t Settings;
     _SRSGaborState_t StateL;
	 _SRSGaborState_t StateR;
}_SRSGaborStereoObj_t;

typedef struct
{
     _SRSGaborSettings_t Settings;
     _SRSGaborState_t State;
}_SRSGaborMonoObj_t;

#endif //__SRS_GABOR_DEF_H__
