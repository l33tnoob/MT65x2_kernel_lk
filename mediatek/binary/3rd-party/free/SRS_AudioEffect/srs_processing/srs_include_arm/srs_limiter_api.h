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
 *  Exposes all srs_limiter APIs
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_common/std_fxp/include/srs_limiter_api.h#4 $
 *  $Author: oscarh $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_LIMITER_API_H__
#define __SRS_LIMITER_API_H__

#include "srs_limiter_def.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

SRSLimiterObj	SRS_CreateLimiterObj(void *pBuf);

void	SRS_InitLimiterObj8k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj11k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj16k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj22k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj24k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj32k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj44k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj48k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj88k(SRSLimiterObj lmtObj);
void	SRS_InitLimiterObj96k(SRSLimiterObj lmtObj);

void	SRS_LimiterProcessMono(SRSLimiterObj lmtObj, srs_int32 *audioIO, int blockSize);
void	SRS_LimiterProcessStereo(SRSLimiterObj lmtObj, SRSStereoCh *audioIO, int blockSize);

SRSResult	SRS_SetLimiterMinimalGain(SRSLimiterObj lmtObj, srs_int32 minGain); //limit: I32.2
srs_int32	SRS_GetLimiterMinimalGain(SRSLimiterObj lmtObj);

void	SRS_SetLimiterEnable(SRSLimiterObj lmtObj, int enable);
int		SRS_GetLimiterEnable(SRSLimiterObj lmtObj);

void	SRS_SetLimiterControlDefaults(SRSLimiterObj lmtObj);


#ifdef __cplusplus
}
#endif /*__cplusplus*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /*__SRS_LIMITER_API_H__*/
