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
	File:		band_nrg.c

	Content:	Band/Line energy calculations functions

*******************************************************************************/

#include "basic_op.h"
#include "band_nrg.h"

#ifndef ARMV5E
/********************************************************************************
*
* function name: CalcBandEnergy
* description:   Calc sfb-bandwise mdct-energies for left and right channel
*
**********************************************************************************/
void CalcBandEnergy(const Word32 *mdctSpectrum,
                    const Word16 *bandOffset,
                    const Word16  numBands,
                    Word32       *bandEnergy,
                    Word32       *bandEnergySum)
{
  Word32 i, j;
  Word32 accuSum = 0;

  for (i=0; i<numBands; i++) {
    Word32 accu = 0;
    for (j=bandOffset[i]; j<bandOffset[i+1]; j++)
      accu = L_add(accu, MULHIGH(mdctSpectrum[j], mdctSpectrum[j]));

	accu = L_add(accu, accu);
    accuSum = L_add(accuSum, accu);
    bandEnergy[i] = accu;
  }
  *bandEnergySum = accuSum;
}

/********************************************************************************
*
* function name: CalcBandEnergyMS
* description:   Calc sfb-bandwise mdct-energies for left add or minus right channel
*
**********************************************************************************/
void CalcBandEnergyMS(const Word32 *mdctSpectrumLeft,
                      const Word32 *mdctSpectrumRight,
                      const Word16 *bandOffset,
                      const Word16  numBands,
                      Word32       *bandEnergyMid,
                      Word32       *bandEnergyMidSum,
                      Word32       *bandEnergySide,
                      Word32       *bandEnergySideSum)
{

  Word32 i, j;
  Word32 accuMidSum = 0;
  Word32 accuSideSum = 0;


  for(i=0; i<numBands; i++) {
    Word32 accuMid = 0;
    Word32 accuSide = 0;
    for (j=bandOffset[i]; j<bandOffset[i+1]; j++) {
      Word32 specm, specs;
      Word32 l, r;

      l = mdctSpectrumLeft[j] >> 1;
      r = mdctSpectrumRight[j] >> 1;
      specm = l + r;
      specs = l - r;
      accuMid = L_add(accuMid, MULHIGH(specm, specm));
      accuSide = L_add(accuSide, MULHIGH(specs, specs));
    }

	accuMid = L_add(accuMid, accuMid);
	accuSide = L_add(accuSide, accuSide);
	bandEnergyMid[i] = accuMid;
    accuMidSum = L_add(accuMidSum, accuMid);
    bandEnergySide[i] = accuSide;
    accuSideSum = L_add(accuSideSum, accuSide);

  }
  *bandEnergyMidSum = accuMidSum;
  *bandEnergySideSum = accuSideSum;
}

#endif
