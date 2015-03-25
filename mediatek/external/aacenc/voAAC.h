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

/*
 ** Copyright 2003-2010, VisualOn, Inc.
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */
/*******************************************************************************
	File:		voAAC.h

	Content:	AAC codec APIs & data types

*******************************************************************************/

#ifndef __voAAC_H__
#define __voAAC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "voAudio.h"

/*!
 * the frame type that the decoder supports
 */
typedef enum {
	VOAAC_RAWDATA			= 0,	/*!<contains only raw aac data in a frame*/
	VOAAC_ADTS				= 1,	/*!<contains ADTS header + raw AAC data in a frame*/
	VOAAC_FT_MAX			= VO_MAX_ENUM_VALUE
} VOAACFRAMETYPE;

/*!
 * the structure for AAC encoder input parameter
 */
typedef  struct {
  int	  sampleRate;          /*! audio file sample rate */
  int	  bitRate;             /*! encoder bit rate in bits/sec */
  short   nChannels;		   /*! number of channels on input (1,2) */
  short   adtsUsed;			   /*! whether write adts header */
} AACENC_PARAM;

/* AAC Param ID */
#define VO_PID_AAC_Mdoule				0x42211000
#define VO_PID_AAC_ENCPARAM				VO_PID_AAC_Mdoule | 0x0040  /*!< get/set AAC encoder parameter, the parameter is a pointer to AACENC_PARAM */

/* AAC decoder error ID */
#define VO_ERR_AAC_Mdoule				0x82210000
#define VO_ERR_AAC_UNSFILEFORMAT		(VO_ERR_AAC_Mdoule | 0xF001)
#define VO_ERR_AAC_UNSPROFILE			(VO_ERR_AAC_Mdoule | 0xF002)

/**
 * Get audio encoder API interface
 * \param pEncHandle [out] Return the AAC Encoder handle.
 * \retval VO_ERR_OK Succeeded.
 */
VO_S32 VO_API voGetAACEncAPI (VO_AUDIO_CODECAPI * pEncHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __voAAC_H__



