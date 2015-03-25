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
 *  Exposes ANSI 10-band graphic EQ APIs
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_graphiceq/std_fxp/include/srs_10band_geq_api.h#6 $
 *  $Author: oscarh $
 *  $Date: 2010/11/16 $
 *	
********************************************************************************/

#ifndef __SRS_10BAND_GRAPHICEQ_API_H__
#define __SRS_10BAND_GRAPHICEQ_API_H__

#include "srs_graphiceq_def.h"
#include "srs_graphiceq_ver_api.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

//API declaration here:
SRS10BandGraphicEqObj		SRS_Create10BandGraphicEqObj(void *pBuf);

void	SRS_Init10BandGraphicEqObj8k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj11k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj16k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj22k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj24k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj32k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj44k(SRS10BandGraphicEqObj geqObj);
void	SRS_Init10BandGraphicEqObj48k(SRS10BandGraphicEqObj geqObj);

void	SRS_Set10BandGraphicEqControlDefaults(SRS10BandGraphicEqObj geqObj);


void	SRS_Set10BandGraphicEqEnable(SRS10BandGraphicEqObj geqObj, int enable);
int		SRS_Get10BandGraphicEqEnable(SRS10BandGraphicEqObj geqObj);

SRSResult	SRS_Set10BandGraphicEqBandGain(SRS10BandGraphicEqObj geqObj, int bandIndex, srs_int16 gain);
srs_int16	SRS_Get10BandGraphicEqBandGain(SRS10BandGraphicEqObj geqObj, int bandIndex);

void	SRS_Set10BandGraphicEqLimiterEnable(SRS10BandGraphicEqObj geqObj, int enable);
int		SRS_Get10BandGraphicEqLimiterEnable(SRS10BandGraphicEqObj geqObj);

void	SRS_10BandGraphicEqProcess(SRS10BandGraphicEqObj geqObj, srs_int32 *audioIO, int blockSize, void *ws);


#ifdef __cplusplus
}
#endif /*__cplusplus*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif /*__SRS_10BAND_GRAPHICEQ_API_H__*/
