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
	File:		psy_configuration.h

	Content:	Psychoaccoustic configuration structure and functions

*******************************************************************************/

#ifndef _PSY_CONFIGURATION_H
#define _PSY_CONFIGURATION_H

#include "typedefs.h"
#include "psy_const.h"
#include "tns.h"

typedef struct{

  Word16 sfbCnt;
  Word16 sfbActive;   /* number of sf bands containing energy after lowpass */
  const Word16 *sfbOffset;

  Word32 sfbThresholdQuiet[MAX_SFB_LONG];

  Word16 maxAllowedIncreaseFactor;   /* preecho control */
  Word16 minRemainingThresholdFactor;

  Word16 lowpassLine;
  Word16 sampRateIdx;
  Word32 clipEnergy;                 /* for level dependend tmn */

  Word16 ratio;
  Word16 sfbMaskLowFactor[MAX_SFB_LONG];
  Word16 sfbMaskHighFactor[MAX_SFB_LONG];

  Word16 sfbMaskLowFactorSprEn[MAX_SFB_LONG];
  Word16 sfbMaskHighFactorSprEn[MAX_SFB_LONG];


  Word16 sfbMinSnr[MAX_SFB_LONG];       /* minimum snr (formerly known as bmax) */

  TNS_CONFIG tnsConf;

}PSY_CONFIGURATION_LONG; /*Word16 size: 8 + 52 + 102 + 51 + 51 + 51 + 51 + 47 = 515 */


typedef struct{

  Word16 sfbCnt;
  Word16 sfbActive;   /* number of sf bands containing energy after lowpass */
  const Word16 *sfbOffset;

  Word32 sfbThresholdQuiet[MAX_SFB_SHORT];

  Word16 maxAllowedIncreaseFactor;   /* preecho control */
  Word16 minRemainingThresholdFactor;

  Word16 lowpassLine;
  Word16 sampRateIdx;
  Word32 clipEnergy;                 /* for level dependend tmn */

  Word16 ratio;
  Word16 sfbMaskLowFactor[MAX_SFB_SHORT];
  Word16 sfbMaskHighFactor[MAX_SFB_SHORT];

  Word16 sfbMaskLowFactorSprEn[MAX_SFB_SHORT];
  Word16 sfbMaskHighFactorSprEn[MAX_SFB_SHORT];


  Word16 sfbMinSnr[MAX_SFB_SHORT];       /* minimum snr (formerly known as bmax) */

  TNS_CONFIG tnsConf;

}PSY_CONFIGURATION_SHORT; /*Word16 size: 8 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 47 = 167 */


/* Returns the sample rate index */
Word32 GetSRIndex(Word32 sampleRate);


Word16 InitPsyConfigurationLong(Word32 bitrate,
                                Word32 samplerate,
                                Word16 bandwidth,
                                PSY_CONFIGURATION_LONG *psyConf);

Word16 InitPsyConfigurationShort(Word32 bitrate,
                                 Word32 samplerate,
                                 Word16 bandwidth,
                                 PSY_CONFIGURATION_SHORT *psyConf);

#endif /* _PSY_CONFIGURATION_H */



