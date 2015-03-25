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
 *  FOCUS public APIs header file
 *
 *	Author: Oscar Huang
 *	
 *	(RCS keywords below, do not edit)
 *  $Id: //srstech/srs_common/std_fxp/include/srs_focus_api.h#3 $
 *  $Author: oscarh $
 *  $Date: 2010/09/21 $
********************************************************************************/

#ifndef __SRS_FOCUS_API_H__
#define __SRS_FOCUS_API_H__

#include "srs_typedefs.h"
#include "srs_fxp.h"
#include "srs_focus_def.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

SRSFocusObj	SRS_CreateFocusObj(void *pBuf);
void		SRS_InitFocusObj8k(SRSFocusObj focusObj);	
void		SRS_InitFocusObj11k(SRSFocusObj focusObj);
void		SRS_InitFocusObj16k(SRSFocusObj focusObj);
void		SRS_InitFocusObj22k(SRSFocusObj focusObj);
void		SRS_InitFocusObj24k(SRSFocusObj focusObj);
void		SRS_InitFocusObj32k(SRSFocusObj focusObj);
void		SRS_InitFocusObj44k(SRSFocusObj focusObj);
void		SRS_InitFocusObj48k(SRSFocusObj focusObj);
void		SRS_InitFocusObj88k(SRSFocusObj focusObj);
void		SRS_InitFocusObj96k(SRSFocusObj focusObj);

void		SRS_SetFocusControlDefaults(SRSFocusObj focusObj);

void		SRS_FocusProcess(SRSFocusObj focusObj, srs_int32 *audioIO, int blockSize);
void		SRS_Focus(SRSFocusObj focusObj, srs_int32 *audioIO, int blockSize, void *ws);

void		SRS_SetFocusEnable(SRSFocusObj focusObj, int enable);
int			SRS_GetFocusEnable(SRSFocusObj focusObj);
SRSResult 	SRS_SetFocusInputGain(SRSFocusObj focusObj, srs_int16 gain);
srs_int16 	SRS_GetFocusInputGain(SRSFocusObj focusObj);
SRSResult 	SRS_SetFocusOutputGain(SRSFocusObj focusObj, srs_int16 gain);
srs_int16 	SRS_GetFocusOutputGain(SRSFocusObj focusObj);
SRSResult 	SRS_SetFocusBypassGain(SRSFocusObj focusObj, srs_int16 gain);
srs_int16 	SRS_GetFocusBypassGain(SRSFocusObj focusObj);
SRSResult 	SRS_SetFocusFactor(SRSFocusObj focusObj, srs_int16 factor);
srs_int16 	SRS_GetFocusFactor(SRSFocusObj focusObj);

#ifdef __cplusplus
}
#endif /*__cplusplus*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //__SRS_FOCUS_API_H__
