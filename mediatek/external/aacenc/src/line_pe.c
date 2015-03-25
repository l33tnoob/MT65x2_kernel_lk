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
	File:		line_pe.c

	Content:	Perceptual entropie module functions

*******************************************************************************/

#include "basic_op.h"
#include "oper_32b.h"
#include "typedef.h"
#include "line_pe.h"


static const Word16  C1_I = 12;    /* log(8.0)/log(2) *4         */
static const Word32  C2_I = 10830; /* log(2.5)/log(2) * 1024 * 4 * 2 */
static const Word16  C3_I = 573;   /* (1-C2/C1) *1024            */


/*****************************************************************************
*
* function name: prepareSfbPe
* description:  constants that do not change during successive pe calculations
*
**********************************************************************************/
void prepareSfbPe(PE_DATA *peData,
                  PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                  Word16 logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                  Word16 sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
                  const Word16 nChannels,
                  const Word16 peOffset)
{
  Word32 sfbGrp, sfb;
  Word32 ch;

  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    PE_CHANNEL_DATA *peChanData=&peData->peChannelData[ch];
    for(sfbGrp=0;sfbGrp<psyOutChan->sfbCnt; sfbGrp+=psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
	    peChanData->sfbNLines4[sfbGrp+sfb] = sfbNRelevantLines[ch][sfbGrp+sfb];
        sfbNRelevantLines[ch][sfbGrp+sfb] = sfbNRelevantLines[ch][sfbGrp+sfb] >> 2;
	    peChanData->sfbLdEnergy[sfbGrp+sfb] = logSfbEnergy[ch][sfbGrp+sfb];
      }
    }
  }
  peData->offset = peOffset;
}


/*****************************************************************************
*
* function name: calcSfbPe
* description:  constPart is sfbPe without the threshold part n*ld(thr) or n*C3*ld(thr)
*
**********************************************************************************/
void calcSfbPe(PE_DATA *peData,
               PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
               const Word16 nChannels)
{
  Word32 ch;
  Word32 sfbGrp, sfb;
  Word32 nLines4;
  Word32 ldThr, ldRatio;
  Word32 pe, constPart, nActiveLines;

  peData->pe = peData->offset;
  peData->constPart = 0;
  peData->nActiveLines = 0;
  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    PE_CHANNEL_DATA *peChanData = &peData->peChannelData[ch];
    const Word32 *sfbEnergy = psyOutChan->sfbEnergy;
    const Word32 *sfbThreshold = psyOutChan->sfbThreshold;

    pe = 0;
    constPart = 0;
    nActiveLines = 0;

    for(sfbGrp=0; sfbGrp<psyOutChan->sfbCnt; sfbGrp+=psyOutChan->sfbPerGroup) {
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        Word32 nrg = sfbEnergy[sfbGrp+sfb];
        Word32 thres = sfbThreshold[sfbGrp+sfb];
        Word32 sfbLDEn = peChanData->sfbLdEnergy[sfbGrp+sfb];

        if (nrg > thres) {
          ldThr = iLog4(thres);

          ldRatio = sfbLDEn - ldThr;

          nLines4 = peChanData->sfbNLines4[sfbGrp+sfb];

          /* sfbPe = nl*log2(en/thr)*/
		  if (ldRatio >= C1_I) {
            peChanData->sfbPe[sfbGrp+sfb] = (nLines4*ldRatio + 8) >> 4;
            peChanData->sfbConstPart[sfbGrp+sfb] = ((nLines4*sfbLDEn)) >> 4;
          }
          else {
		  /* sfbPe = nl*(c2 + c3*log2(en/thr))*/
            peChanData->sfbPe[sfbGrp+sfb] = extract_l((L_mpy_wx(
                    (C2_I + C3_I * ldRatio * 2) << 4, nLines4) + 4) >> 3);
            peChanData->sfbConstPart[sfbGrp+sfb] = extract_l(( L_mpy_wx(
                    (C2_I + C3_I * sfbLDEn * 2) << 4, nLines4) + 4) >> 3);
            nLines4 = (nLines4 * C3_I + (1024<<1)) >> 10;
          }
          peChanData->sfbNActiveLines[sfbGrp+sfb] = nLines4 >> 2;
        }
        else {
          peChanData->sfbPe[sfbGrp+sfb] = 0;
          peChanData->sfbConstPart[sfbGrp+sfb] = 0;
          peChanData->sfbNActiveLines[sfbGrp+sfb] = 0;
        }
        pe = pe + peChanData->sfbPe[sfbGrp+sfb];
        constPart = constPart + peChanData->sfbConstPart[sfbGrp+sfb];
        nActiveLines = nActiveLines + peChanData->sfbNActiveLines[sfbGrp+sfb];
      }
    }

	peChanData->pe = saturate(pe);
    peChanData->constPart = saturate(constPart);
    peChanData->nActiveLines = saturate(nActiveLines);


	pe += peData->pe;
	peData->pe = saturate(pe);
    constPart += peData->constPart;
	peData->constPart = saturate(constPart);
    nActiveLines += peData->nActiveLines;
	peData->nActiveLines = saturate(nActiveLines);
  }
}
