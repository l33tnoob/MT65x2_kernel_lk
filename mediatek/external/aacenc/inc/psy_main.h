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
	File:		psy_main.h

	Content:	Psychoacoustic major function block

*******************************************************************************/

#ifndef _PSYMAIN_H
#define _PSYMAIN_H

#include "psy_configuration.h"
#include "qc_data.h"
#include "memalign.h"

/*
  psy kernel
*/
typedef struct  {
  PSY_CONFIGURATION_LONG  psyConfLong;           /* Word16 size: 515 */
  PSY_CONFIGURATION_SHORT psyConfShort;          /* Word16 size: 167 */
  PSY_DATA                psyData[MAX_CHANNELS]; /* Word16 size: MAX_CHANNELS*1669*/
  TNS_DATA                tnsData[MAX_CHANNELS]; /* Word16 size: MAX_CHANNELS*235 */
  Word32*                 pScratchTns;
  Word16				  sampleRateIdx;
}PSY_KERNEL; /* Word16 size: 2587 / 4491 */


Word16 PsyNew( PSY_KERNEL  *hPsy, Word32 nChan, VO_MEM_OPERATOR *pMemOP);
Word16 PsyDelete( PSY_KERNEL  *hPsy, VO_MEM_OPERATOR *pMemOP);

Word16 PsyOutNew( PSY_OUT *hPsyOut, VO_MEM_OPERATOR *pMemOP);
Word16 PsyOutDelete( PSY_OUT *hPsyOut, VO_MEM_OPERATOR *pMemOP);

Word16 psyMainInit( PSY_KERNEL *hPsy,
                    Word32 sampleRate,
                    Word32 bitRate,
                    Word16 channels,
                    Word16 tnsMask,
                    Word16 bandwidth);


Word16 psyMain(Word16                   nChannels,   /*!< total number of channels */
               ELEMENT_INFO             *elemInfo,
               Word16                   *timeSignal, /*!< interleaved time signal */
               PSY_DATA                 psyData[MAX_CHANNELS],
               TNS_DATA                 tnsData[MAX_CHANNELS],
               PSY_CONFIGURATION_LONG*  psyConfLong,
               PSY_CONFIGURATION_SHORT* psyConfShort,
               PSY_OUT_CHANNEL          psyOutChannel[MAX_CHANNELS],
               PSY_OUT_ELEMENT          *psyOutElement,
               Word32                   *pScratchTns,
			   Word32					sampleRate);

#endif /* _PSYMAIN_H */
