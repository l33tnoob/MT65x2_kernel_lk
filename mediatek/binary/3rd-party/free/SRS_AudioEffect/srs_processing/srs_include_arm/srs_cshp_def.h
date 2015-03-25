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
 *  SRS CSHP types, contants
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_cshp/std_fxp/include/srs_cshp_def.h#2 $
 *  $Author: oscarh $
 *  $Date: 2011/02/15 $
 *	
********************************************************************************/

#ifndef __SRS_CSHP_DEF_H__
#define __SRS_CSHP_DEF_H__

#include "srs_typedefs.h"
#include "srs_csdecoder_def.h"
#include "srs_definition_def.h"
#include "srs_focus_def.h"
#include "srs_limiter_def.h"
#include "srs_hp360_def.h"
#include "srs_sa_trubass_def.h"


typedef struct _SRSCshpObj{int _;} *SRSCshpObj;

#define SRS_CSHP_OBJ_SIZE	(sizeof(_SRSCshpObj_t) + \
								SRS_CSDECODER_OBJ_SIZE + \
								SRS_PASSIVE_DECODER_OBJ_SIZE + \
								SRS_FOCUS_OBJ_SIZE + \
								SRS_HP360_OBJ_SIZE + \
								SRS_DEFINITION_OBJ_SIZE + \
								SRS_DEFINITION_OBJ_SIZE + \
								SRS_SA_TRUBASS_OBJ_SIZE + \
								SRS_LIMITER_OBJ_SIZE + \
								8)

#define SRS_CSHP_WORKSPACE_SIZE(blockSize)		(SRS_CSHP_MAX(\
													SRS_CSHP_MAX(SRS_CSDECODER_WORKSPACE_SIZE(blockSize), SRS_SA_TRUBASS_WORKSPACE_SIZE(blockSize)), \
													SRS_HP360_WORKSPACE_SIZE(blockSize) \
												 ) + 6*sizeof(srs_int32)*(blockSize) + 8)

#define SRS_CSHP_GAIN_IWL	1		//iwl of InputGain, OutputGain and BypassGain

typedef enum
{
	SRS_PASSIVE_DECODER,
	SRS_CS_DECODER
} SRSCshpRenderingMode;

#define SRS_CSHP_MAX(a, b)	((a)>=(b)? (a):(b))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SRS Internal Use:

#define SRS_PASSIVE_DECODER_OBJ_SIZE	(sizeof(_SRSPassiveDecoderObj_t)+8)
typedef struct _SRSPassiveDecoderObj{int _;} *SRSPassiveDecoderObj;

typedef struct{
	int						Enable;
	srs_int16				InputGain; //<1
	srs_int16				OutputGain; //<1
	srs_int16				BypassGain; //<1
	int						RendMode; //SRSCshpRenderingMode
} _SRSCshpSettings_t;

typedef struct{
	_SRSCshpSettings_t		Settings;

	SRSCSDecoderObj			CSDecoder;
	SRSPassiveDecoderObj	PassiveDecoder;
	SRSFocusObj				Focus;			//Dialog clarity
	SRSHp360Obj				Hp360;
	SRSDefinitionObj		DefinitionLeft;
	SRSDefinitionObj		DefinitionRight;
	SRSSATruBassObj			TruBass;
	SRSLimiterObj			Limiter;
} _SRSCshpObj_t;

typedef struct{
	const srs_int32	*SwFilterCoef;		//subwoofer filter coefficients
	srs_int32		SwFilterState[2];	//state buffer for the subwoofer filter
} _SRSPassiveDecoderObj_t;


#endif //__SRS_CSHP_DEF_H__
