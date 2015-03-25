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
 *  Exposes all srs_maxvf APIs
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_maxvf/std_fxp/include/srs_maxvf_api.h#16 $
 *  $Author: huixiz $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_MAXVF_API_H__
#define __SRS_MAXVF_API_H__

#include "srs_maxvf_def.h"
#include "srs_fxp.h"

/*Data type definition here:*/

//currently only support windowsize = 1024, fftsize = 512, blocksize = 256

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

//API declaration here:


SRSMaxVFObj			SRS_CreateMaxVFObj (void* pBuf, SRSIOMode mode);

SRSResult				SRS_InitMaxVFObj8k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj11k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj16k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj22k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj24k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj44k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj48k(SRSMaxVFObj maxvfObj);
SRSResult				SRS_InitMaxVFObj32k(SRSMaxVFObj maxvfObj);


SRSResult			SRS_MaxVFStereo(SRSMaxVFObj maxvfObj,SRSStereoCh *input, SRSStereoCh *output,void *ws);   //blocksize should be 256,input and output should not point to the same memory
SRSResult			SRS_MaxVFMono(SRSMaxVFObj maxvfObj,srs_int32 *input, srs_int32 *output,void *ws);   //blocksize should be 256,input and output should not point to the same memory

SRSResult			SRS_MaxVFStereoProcess(SRSMaxVFObj maxvfObj,SRSStereoCh *input, SRSStereoCh *output,void *ws);   //blocksize should be 256,input and output should not point to the same memory
SRSResult			SRS_MaxVFMonoProcess(SRSMaxVFObj maxvfObj,srs_int32 *input, srs_int32 *output,void *ws);   //blocksize should be 256,input and output should not point to the same memory


void				SRS_SetMaxVFControlDefaults(SRSMaxVFObj maxvfObj);

void				SRS_SetMaxVFEnable(SRSMaxVFObj maxvfObj,int enable);
void				SRS_SetMaxVFAntiClipEnable(SRSMaxVFObj maxvfObj,int enable);					//enable anti-clip or not: 1:enable 0:disable
void				SRS_SetMaxVFHighPassFilterEnable(SRSMaxVFObj maxvfObj,int enable);				//enable high pass filtering or not
SRSResult			SRS_SetMaxVFInputGain(SRSMaxVFObj maxvfObj,srs_int16 gain);						//gain: I16.SRS_MAXVF_GAIN_IWL (I16.6)
SRSResult			SRS_SetMaxVFOutputGain(SRSMaxVFObj maxvfObj,srs_int16 gain);					//gain: I16.SRS_MAXVF_GAIN_IWL (I16.6)
SRSResult			SRS_SetMaxVFBypassGain(SRSMaxVFObj maxvfObj,srs_int16 gain);					//gain: I16.SRS_MAXVF_GAIN_IWL (I16.1)
SRSResult			SRS_SetMaxVFBoost(SRSMaxVFObj maxvfObj,srs_int32 boost);						//MaxVF boost: I32.SRS_MAXVF_BOOST_IWL (I32.2)
void				SRS_SetMaxVFHighPassFilterCoef(SRSMaxVFObj maxvfObj,srs_int32* coef, int order);	//coef: in SRS filter format, 2<< order << 8

int					SRS_GetMaxVFEnable(SRSMaxVFObj maxvfObj);
int					SRS_GetMaxVFAntiClipEnable(SRSMaxVFObj maxvfObj);
int					SRS_GetMaxVFHighPassFilterEnable(SRSMaxVFObj maxvfObj);
srs_int16			SRS_GetMaxVFInputGain(SRSMaxVFObj maxvfObj);
srs_int16			SRS_GetMaxVFOutputGain(SRSMaxVFObj maxvfObj);
srs_int16			SRS_GetMaxVFBypassGain(SRSMaxVFObj maxvfObj);
srs_int32			SRS_GetMaxVFBoost(SRSMaxVFObj maxvfObj);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__SRS_MAXVF_API_H__*/
