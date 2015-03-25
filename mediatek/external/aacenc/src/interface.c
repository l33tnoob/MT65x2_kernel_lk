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
	File:		interface.c

	Content:	Interface psychoaccoustic/quantizer functions

*******************************************************************************/

#include "basic_op.h"
#include "oper_32b.h"
#include "psy_const.h"
#include "interface.h"

/*****************************************************************************
*
* function name: BuildInterface
* description:  update output parameter
*
**********************************************************************************/
void BuildInterface(Word32                  *groupedMdctSpectrum,
                    const Word16             mdctScale,
                    SFB_THRESHOLD           *groupedSfbThreshold,
                    SFB_ENERGY              *groupedSfbEnergy,
                    SFB_ENERGY              *groupedSfbSpreadedEnergy,
                    const SFB_ENERGY_SUM     sfbEnergySumLR,
                    const SFB_ENERGY_SUM     sfbEnergySumMS,
                    const Word16             windowSequence,
                    const Word16             windowShape,
                    const Word16             groupedSfbCnt,
                    const Word16            *groupedSfbOffset,
                    const Word16             maxSfbPerGroup,
                    const Word16            *groupedSfbMinSnr,
                    const Word16             noOfGroups,
                    const Word16            *groupLen,
                    PSY_OUT_CHANNEL         *psyOutCh)
{
  Word32 j;
  Word32 grp;
  Word32 mask;
  Word16 *tmpV;

  /*
  copy values to psyOut
  */
  psyOutCh->maxSfbPerGroup    = maxSfbPerGroup;
  psyOutCh->sfbCnt            = groupedSfbCnt;
  if(noOfGroups)
	psyOutCh->sfbPerGroup     = groupedSfbCnt/ noOfGroups;
  else
	psyOutCh->sfbPerGroup     = 0x7fff;
  psyOutCh->windowSequence    = windowSequence;
  psyOutCh->windowShape       = windowShape;
  psyOutCh->mdctScale         = mdctScale;
  psyOutCh->mdctSpectrum      = groupedMdctSpectrum;
  psyOutCh->sfbEnergy         = groupedSfbEnergy->sfbLong;
  psyOutCh->sfbThreshold      = groupedSfbThreshold->sfbLong;
  psyOutCh->sfbSpreadedEnergy = groupedSfbSpreadedEnergy->sfbLong;

  tmpV = psyOutCh->sfbOffsets;
  for(j=0; j<groupedSfbCnt + 1; j++) {
      *tmpV++ = groupedSfbOffset[j];
  }

  tmpV = psyOutCh->sfbMinSnr;
  for(j=0;j<groupedSfbCnt; j++) {
	  *tmpV++ =   groupedSfbMinSnr[j];
  }

  /* generate grouping mask */
  mask = 0;
  for (grp = 0; grp < noOfGroups; grp++) {
    mask = mask << 1;
    for (j=1; j<groupLen[grp]; j++) {
      mask = mask << 1;
      mask |= 1;
    }
  }
  psyOutCh->groupingMask = mask;

  if (windowSequence != SHORT_WINDOW) {
    psyOutCh->sfbEnSumLR =  sfbEnergySumLR.sfbLong;
    psyOutCh->sfbEnSumMS =  sfbEnergySumMS.sfbLong;
  }
  else {
    Word32 i;
    Word32 accuSumMS=0;
    Word32 accuSumLR=0;
    const Word32 *pSumMS = sfbEnergySumMS.sfbShort;
    const Word32 *pSumLR = sfbEnergySumLR.sfbShort;

    for (i=TRANS_FAC; i; i--) {
      accuSumLR = L_add(accuSumLR, *pSumLR); pSumLR++;
      accuSumMS = L_add(accuSumMS, *pSumMS); pSumMS++;
    }
    psyOutCh->sfbEnSumMS = accuSumMS;
    psyOutCh->sfbEnSumLR = accuSumLR;
  }
}
