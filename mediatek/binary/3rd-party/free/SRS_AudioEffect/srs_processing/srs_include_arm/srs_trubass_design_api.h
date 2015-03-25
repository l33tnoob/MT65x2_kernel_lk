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
 *  SRS TruBass filter design APIs
 *
 *  Authour: Oscar Huang
 *
 *  RCS keywords:
 *	$Id: //srstech/srs_designer/std_fxp/include/srs_trubass_design_api.h#2 $
 *  $Author: oscarh $
 *  $Date: 2011/01/12 $
 *	
********************************************************************************/
#ifndef __SRS_TRUBASS_DESIGN_API_H__
#define __SRS_TRUBASS_DESIGN_API_H__

#include "srs_trubass_design_def.h"
#include "srs_typedefs.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

/******************************************************************************************
 * Design floating point speaker filters for Trubass
 * Parameters:
 *	lowFreq: [IN] low pass filter cut off of the speaker filter
 *  samplingRate: [IN] the sampling rate
 *  coefs: [OUT] the designed filter coefficients. Filter coefficients are arranged in
 *				  the following pattern:
	struct
	{
		double	LowPassAudioFilterCoef[2]; //b0, a1
		double	LowBandAudioFilterCoef[3]; //b0, a1, a2
		double	MidBandAudioFilterCoef[3]; //b0, a1, a2
	}
 *
*******************************************************************************************/
//void SRS_TruBassFilterDesignFloat(double lowFreq, double samplingRate, double *coefs);

/******************************************************************************************
 * Design floating point speaker filters for Split-Analysis Trubass
 * Parameters:
 *	lowFreqAudio: [IN] low pass filter cut off of the audio filter
 *  lowFreqAnalysis: [IN] low pass filter cut off of the analysis filter
 *  samplingRate: [IN] the sampling rate
 *  speakerCoefs: [OUT] the designed filter coefficients. Filter coefficients are arranged in
 *				  the following pattern:
	union
	{
		struct
		{
			double	LowPassAudioFilterCoef[2]; //b0, a1
			double	LowBandAudioFilterCoef[3]; //b0, a1, a2
			double	MidBandAudioFilterCoef[3]; //b0, a1, a2
			double	LowPassAnalysisFilterCoef[2];
			double	LowBandAnalysisFilterCoef[3];
			double	MidBandAnalysisFilterCoef[3];
		} Struct;
		double	Array[16];
	} 
*******************************************************************************************/
void SRS_SATruBassFilterDesignFloat(double lowFreqAudio, double lowFreqAnalysis, double samplingRate, double *speakerCoefs);


/******************************************************************************************
 * Design fixed-point speaker filters for TruBass. The returned coefficient structure can
 * be used with the SRS_SetTruBassCustomSpeakerFilterCoefs API of srs_trubass
 * Parameters:
  *  lowFreq: [IN]low pass filter cut off of the speaker filter
  *  samplingRate: [IN] the sampling rate
  *  ws:  [IN] workspace, whose size must be at least SRS_TRUBASS_DESIGN_WORKSPACE_SIZE bytes
* Return:
  * The designed fixed-point coefficient structure
*******************************************************************************************/
//SRSTruBassCustomSpeakerCoefs SRS_TruBassFilterDesignFxp(double lowFreq, double samplingRate, void *ws);


/******************************************************************************************
 * Design 32-bit fixed-point speaker filters for Split-Analysis TruBass
 * Parameters:
 *	lowFreqAudio: [IN] low pass filter cut off of the audio filter
 *  lowFreqAnalysis: [IN] low pass filter cut off of the analysis filter
 *  samplingRate: [IN] the sampling rate
 *  speakerCoefs: [OUT] the designed filter coefficients. Filter coefficients are arranged in
 *				  the following pattern:
	union
	{
		struct
		{
			srs_int32	LowPassAudioFilterCoef[2];
			srs_int32	LowBandAudioFilterCoef[3];
			srs_int32	MidBandAudioFilterCoef[3];
			srs_int32	LowPassAnalysisFilterCoef[2];
			srs_int32	LowBandAnalysisFilterCoef[3];
			srs_int32	MidBandAnalysisFilterCoef[3];
		} Struct;
		srs_int32	Array[16];
	} 
 *  ws:  [IN] workspace, whose size must be at least SRS_SA_TRUBASS_DESIGN_WORKSPACE_SIZE bytes
*******************************************************************************************/
void SRS_SATruBassFilterDesignFxp32(double lowFreqAudio, double lowFreqAnalysis, double samplingRate, srs_int32 *speakerCoefs, void *ws);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //__SRS_TRUBASS_DESIGN_API_H__
