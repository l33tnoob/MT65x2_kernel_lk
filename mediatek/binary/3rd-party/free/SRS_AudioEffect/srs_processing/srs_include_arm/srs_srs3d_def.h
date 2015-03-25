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
 *  SRS SRS3D types, constants
 *
 *	Author: Zesen Zhuang
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_srs3d/std_fxp/include/srs_srs3d_def.h#3 $
 *  $Author: oscarh $
 *  $Date: 2011/02/14 $
 *	
********************************************************************************/

#ifndef __SRS_SRS3D_DEF_H__
#define __SRS_SRS3D_DEF_H__

#include "srs_typedefs.h"


#define SRS_SRS3D_GAIN_IWL	1
#define SRS_SRS3D_CTRL_IWL	1

typedef struct _SRSSrs3dObj{int _;}	* SRSSrs3dObj;

#define SRS_SRS3D_OBJ_SIZE					(sizeof(_SRSSrs3dObj_t)+8)
#define SRS_SRS3D_WORKSPACE_SIZE(blksize)	(sizeof(srs_int32)*(blksize)*4+8)

/****** SRS 3D Modes ******/
typedef enum
{
	SRSSrs3dMono,
	SRSSrs3dSingleSpeaker,
	SRSSrs3dStereo,
	SRSSrs3dExtreme
//	SRSSrs3dModeSize
} SRSSrs3dMode;

////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:
typedef struct
{
	int							Enable;				//enable, 0: disabled, non-zero: enabled
	srs_int16					InputGain;			//refined?
	srs_int16					OutputGain;			//refined?
	srs_int16					BypassGain;			//refined?		

	int							HighBitRate;
	srs_int16					SpaceCtrl;
	srs_int16					CenterCtrl;
	int							Headphone;
	SRSSrs3dMode				Mode;

	srs_int16					NormGain;


	const srs_int16				*InitFilterCoefPtr;
	int							InitFilterIwl;
	const srs_int16				*InitFilterCoefPtrArry[4];
	int							InitFilterIwlArry[4];

	const srs_int16				*FinalFilterCoefPtr;
	const srs_int16				*SingleAllPassFilterCoefPtr;
	const srs_int16				*SingleFirstFilterCoefPtr;
	const srs_int16				*SingleSecondFilterCoefPtr;
	const srs_int16				*SingleThirdFilterCoefPtr;


} _SRSSrs3dSettings_t;


typedef struct
{
	/* Storage for the SRS3D filter state variables */
	srs_int32					InitFilterState[2];						/* Length:	2 */
	srs_int32					FinalFilterState[1];					/* Length:	1 */
	srs_int32					SingleAllPassFilterState[1];			/* Length:	1 */	// single speaker

	srs_int32					PhaseShiftFilterState[6];

} _SRSSrs3dState_t;


typedef struct
{
	_SRSSrs3dSettings_t		Settings;
	_SRSSrs3dState_t		State;
} _SRSSrs3dObj_t;

#endif //__SRS_SRS3D_DEF_H__
