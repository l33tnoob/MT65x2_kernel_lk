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
 *  Exposes all srs_truvf APIs
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_truvf/std_fxp/include/srs_truvf_api.h#11 $
 *  $Author: oscarh $
 *  $Date: 2010/09/21 $
 *	
********************************************************************************/

#ifndef __SRS_TRUVF_API_H__
#define __SRS_TRUVF_API_H__

#include "srs_truvf_def.h"
#include "srs_fxp.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


//Creating TruVolume object, the size of the buffer pointed to by pBuf must be SRS_TRUVF_OBJ_SIZE bytes.
//mode must be either SRS_IO_STEREO for stereo audio or SRS_IO_MONO for mono audio
SRSTruVFObj			SRS_CreateTruVFObj (void* pBuf, SRSIOMode mode);

//Initializing the TruVolume object:
SRSResult			SRS_InitTruVFObj (SRSTruVFObj truvfObj);

//Call SRS_TruVFStereo to process your stereo audio (The TruVolume object must have been created with SRS_IO_STEREO mode)
//The workspace (pointed to by ws) size must be SRS_TRUVF_WORKSPACE_SIZE bytes:
SRSResult			SRS_TruVFStereo(SRSTruVFObj truvfObj,SRSStereoCh *input, SRSStereoCh *output,void *ws);//blocksize should be 256,input and output should not point to the same memory
SRSResult			SRS_TruVFStereoProcess(SRSTruVFObj truvfObj,SRSStereoCh *input, SRSStereoCh *output,void *ws);//blocksize should be 256,input and output should not point to the same memory

//Call SRS_TruVFMono to process your mono audio (The TruVolume object must have been created with SRS_IO_MONO mode)
//The workspace (pointed to by ws) size must be SRS_TRUVF_WORKSPACE_SIZE bytes:
SRSResult			SRS_TruVFMono(SRSTruVFObj truvfObj,srs_int32 *input, srs_int32 *output,void *ws);//blocksize should be 256,input and output should not point to the same memory
SRSResult			SRS_TruVFMonoProcess(SRSTruVFObj truvfObj,srs_int32 *input, srs_int32 *output,void *ws);//blocksize should be 256,input and output should not point to the same memory

void				SRS_SetTruVFControlDefaults(SRSTruVFObj truvfObj);


void				SRS_SetTruVFEnable(SRSTruVFObj truvfObj,int enable);
SRSResult			SRS_SetTruVFInputGain(SRSTruVFObj truvfObj,srs_int16 gain);			//gain: I16.SRS_MAXVF_GAIN_IWL (I16.6)
SRSResult			SRS_SetTruVFOutputGain(SRSTruVFObj truvfObj,srs_int16 gain);		//gain: I16.SRS_MAXVF_GAIN_IWL (I16.6)
SRSResult			SRS_SetTruVFBypassGain(SRSTruVFObj truvfObj,srs_int16 gain);		//gain: I16.SRS_MAXVF_GAIN_IWL (I16.1)



int					SRS_GetTruVFEnable(SRSTruVFObj truvfObj);
srs_int16			SRS_GetTruVFInputGain(SRSTruVFObj truvfObj);
srs_int16			SRS_GetTruVFOutputGain(SRSTruVFObj truvfObj);
srs_int16			SRS_GetTruVFBypassGain(SRSTruVFObj truvfObj);



#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /*__SRS_TRUVF_API_H__*/
