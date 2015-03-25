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
 *  SRS WOWHD types, constants
 *
 *	Author: Zesen Zhuang
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_wowhd/std_fxp/include/srs_wowhd_def.h#3 $
 *  $Author: zesenz $
 *  $Date: 2010/11/22 $
 *	
********************************************************************************/

#ifndef __SRS_WOWHD_DEF_H__
#define __SRS_WOWHD_DEF_H__

#include "srs_typedefs.h"
#include "srs_srs3d_def.h"
#include "srs_definition_def.h"
#include "srs_focus_def.h"
#include "srs_limiter_def.h"
#include "srs_sa_trubass_def.h"


#define SRS_WOWHD_GAIN_IWL	1

/*Data type definition here:*/
typedef  struct _SRSWowhdObj{int _;} *SRSWowhdObj;

#define SRS_WOWHD_OBJ_SIZE					(sizeof(_SRSWowhdObj_t)+SRS_SRS3D_OBJ_SIZE+SRS_SA_TRUBASS_OBJ_SIZE+2*SRS_DEFINITION_OBJ_SIZE+2*SRS_FOCUS_OBJ_SIZE+SRS_LIMITER_OBJ_SIZE)
#define SRS_WOWHD_WORKSPACE_SIZE(blksize)	(sizeof(srs_int32)*6*(blksize)+40)

///////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:


typedef struct
{
	int							Enable;				//enable, 0: disabled, non-zero: enabled
	srs_int16					InputGain;			//input gain
	srs_int16					OutputGain;			//output gain
	srs_int16					BypassGain;			//bypass gain	

} _SRSWowhdSettings_t;



typedef struct
{
	_SRSWowhdSettings_t		Settings;
	SRSSrs3dObj				Srs3dObj;
	SRSDefinitionObj		LDefObj;
	SRSDefinitionObj		RDefObj;
	SRSFocusObj				LFocusObj;
	SRSFocusObj				RFocusObj;
	SRSLimiterObj			LmtObj;
	SRSSATruBassObj			TrubObj;

} _SRSWowhdObj_t;


#endif //__SRS_WOWHD_DEF_H__
