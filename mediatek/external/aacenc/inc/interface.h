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
	File:		interface.h

	Content:	psychoaccoustic/quantizer structures and interface

*******************************************************************************/

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "config.h"
#include "psy_const.h"
#include "psy_data.h"
#include "typedefs.h"


enum
{
  MS_NONE = 0,
  MS_SOME = 1,
  MS_ALL  = 2
};

enum
{
  MS_ON = 1
};

struct TOOLSINFO {
  Word16 msDigest;
  Word16 msMask[MAX_GROUPED_SFB];
};


typedef struct {
  Word16  sfbCnt;
  Word16  sfbPerGroup;
  Word16  maxSfbPerGroup;
  Word16  windowSequence;
  Word16  windowShape;
  Word16  groupingMask;
  Word16  sfbOffsets[MAX_GROUPED_SFB+1];
  Word16  mdctScale;
  Word32 *sfbEnergy;
  Word32 *sfbSpreadedEnergy;
  Word32 *sfbThreshold;
  Word32 *mdctSpectrum;
  Word32  sfbEnSumLR;
  Word32  sfbEnSumMS;
  Word32 sfbDist[MAX_GROUPED_SFB];
  Word32 sfbDistNew[MAX_GROUPED_SFB];
  Word16  sfbMinSnr[MAX_GROUPED_SFB];
  Word16 minSfMaxQuant[MAX_GROUPED_SFB];
  Word16 minScfCalculated[MAX_GROUPED_SFB];
  Word16 prevScfLast[MAX_GROUPED_SFB];
  Word16 prevScfNext[MAX_GROUPED_SFB];
  Word16 deltaPeLast[MAX_GROUPED_SFB];
  TNS_INFO tnsInfo;
} PSY_OUT_CHANNEL; /* Word16 size: 14 + 60(MAX_GROUPED_SFB) + 112(TNS_INFO) = 186 */

typedef struct {
  struct TOOLSINFO toolsInfo;
  Word16 groupedSfbOffset[MAX_CHANNELS][MAX_GROUPED_SFB+1];  /* plus one for last dummy offset ! */
  Word16 groupedSfbMinSnr[MAX_CHANNELS][MAX_GROUPED_SFB];
} PSY_OUT_ELEMENT;

typedef struct {
  /* information shared by both channels  */
  PSY_OUT_ELEMENT  psyOutElement;
  /* information specific to each channel */
  PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS];
}PSY_OUT;

void BuildInterface(Word32                 *mdctSpectrum,
                    const Word16            mdctScale,
                    SFB_THRESHOLD          *sfbThreshold,
                    SFB_ENERGY             *sfbEnergy,
                    SFB_ENERGY             *sfbSpreadedEnergy,
                    const SFB_ENERGY_SUM    sfbEnergySumLR,
                    const SFB_ENERGY_SUM    sfbEnergySumMS,
                    const Word16            windowSequence,
                    const Word16            windowShape,
                    const Word16            sfbCnt,
                    const Word16           *sfbOffset,
                    const Word16            maxSfbPerGroup,
                    const Word16           *groupedSfbMinSnr,
                    const Word16            noOfGroups,
                    const Word16           *groupLen,
                    PSY_OUT_CHANNEL        *psyOutCh);

#endif /* _INTERFACE_H */
