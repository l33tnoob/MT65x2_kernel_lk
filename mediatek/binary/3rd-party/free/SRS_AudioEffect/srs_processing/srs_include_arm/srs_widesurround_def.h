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
 *  srs_widesurround typedefs, constants
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/

#ifndef __SRS_WIDESURROUND_DEF_H__
#define __SRS_WIDESURROUND_DEF_H__

#include "srs_typedefs.h"
#include "srs_iir_def.h"
#include "srs_fft_def.h"

/*Data type definition here:*/
typedef  struct _SRSWideSurroundObj{int _;} *SRSWideSurroundObj;

#define SRS_WIDESURROUND_OBJ_SIZE			(sizeof(_SRSWideSurroundObj_t)+SRS_IIR_OBJ_SIZE(2)*2+8)
#define SRS_WIDESURROUND_STATBUF_SIZE		(sizeof(srs_int32)*(SRS_WDSRD_BLK_SZ*2+SRS_WDSRD_MAX_DELAY_LEN*2)+SRS_RFFT_32C16_RDX2_128_SIZE+8)
#define SRS_WIDESURROUND_WORKSPACE_SIZE		(sizeof(srs_int32)*SRS_WDSRD_BLK_SZ*2+8)
#define	SRS_WDSRD_BLK_SZ		64

#define SRS_WIDESURROUND_GAIN_IWL		1
#define SRS_WIDESURROUND_SPK_SEP_IWL	1


//////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
#define		SRS_WDSRD_MAX_DELAY_LEN		10
#define		SRS_WDSRD_MIN_DELAY_LEN		2

typedef struct
{
	int				Enable;				//enable, 0: disabled, non-zero: enabled
	srs_int16		InputGain;			//input gain, 0~1, default 0.5
	srs_int16		BypassGain;			//bypass gain, 0~1, default 1.0
	srs_int16		SpeakerSeparation;	//Speaker separation, 0~1, default 0.375
	srs_int16		CenterBoostGain;	//Center boost gain, 0.25~1, default 0.625
	int				DelayLen;			//Effective delay buffer len, relying on SpeakerSeparation	
	const srs_int16	*HrtfAFft;			//128-pt HRFT A in HC format
	const SRSFftTbl	*FftTbl;
} _SRSWideSurroundSettings_t;

typedef struct
{
	srs_int32		*LHistoryBuf;
	srs_int32		*RHistoryBuf;
	srs_int32		*LDelayBuf;
	srs_int32		*RDelayBuf;
} _SRSWideSurroundState_t;

typedef struct
{
	_SRSWideSurroundSettings_t		Settings;
	_SRSWideSurroundState_t			State;
	SRSIirObj						LeftHsHpf;
	SRSIirObj						RightHsHpf;
} _SRSWideSurroundObj_t;


#endif /*__SRS_WIDESURROUND_DEF_H__*/
