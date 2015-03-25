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
 *  SRS HardLimiter types, constants
 *
 *	Author: Zesen Zhuang
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_hardlimiter/std_fxp/include/srs_hardlimiter_def.h#2 $
 *  $Author: jianc $
 *  $Date: 2010/11/25 $
 *	
********************************************************************************/

#ifndef __SRS_HARDLIMITER_DEF_H__
#define __SRS_HARDLIMITER_DEF_H__

#include "srs_typedefs.h"


typedef struct _SRSHardLimiterObj{int _;} *	SRSHardLimiterObj;

#define SRS_HARDLIMITER_OBJ_SIZE					(sizeof(_SRSHardLimiterObj_t)+2*SRS_HL_MAX_DELAY_LEN*sizeof(srs_int32)+24)
#define SRS_HARDLIMITER_WORKSPACE_SIZE(blksize)		(sizeof(srs_int32)*(blksize)*3+sizeof(srs_int32)*SRS_HL_MAX_DELAY_LEN+8)

#define		SRS_HL_BOOST_IWL		9
#define		SRS_HL_INOUT_GAIN_IWL	3
#define		SRS_HL_BYPASS_GAIN_IWL	1
#define		SRS_HL_LIMIT_IWL		1

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
#define	SRS_HL_MAX_DELAY_LEN				240

typedef struct
{
	int							Enable;				//Master enable, 0: disabled, non-zero: enabled
	srs_int16					InputGain;			//refined?
	srs_int16					OutputGain;			//refined?
	srs_int16					BypassGain;			//refined?		

	int							Delaylen;
	srs_int32					Limiterboost;		/* HL gain boost */
	srs_int16					Hardlimit;			/* HL limit */

	srs_int32					LimiterCoeff;		
	srs_int32					HLThresh;
	int							DecaySmoothEn;		


} _SRSHardLimiterSettings_t;

typedef struct
{
	srs_int32	*Dhistory;
	srs_int32	MaxVal;
	srs_int32	PrevLastGain;

	srs_int32	DLevel;
	int			DecayFlag;

} _SRSHardLimiterState_t;


typedef struct
{
	_SRSHardLimiterSettings_t	Settings;
	_SRSHardLimiterState_t		LState;
	_SRSHardLimiterState_t		RState;
} _SRSHardLimiterObj_t;



#endif //__SRS_HARDLIMITER_DEF_H__
