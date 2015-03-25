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
	File:		psy_data.h

	Content:	Psychoacoustic data and structures

*******************************************************************************/

#ifndef _PSY_DATA_H
#define _PSY_DATA_H

#include "block_switch.h"
#include "tns.h"

/*
  the structs can be implemented as unions
*/

typedef struct{
  Word32 sfbLong[MAX_GROUPED_SFB];
  Word32 sfbShort[TRANS_FAC][MAX_SFB_SHORT];
}SFB_THRESHOLD; /* Word16 size: 260 */

typedef struct{
  Word32 sfbLong[MAX_GROUPED_SFB];
  Word32 sfbShort[TRANS_FAC][MAX_SFB_SHORT];
}SFB_ENERGY; /* Word16 size: 260 */

typedef struct{
  Word32 sfbLong;
  Word32 sfbShort[TRANS_FAC];
}SFB_ENERGY_SUM; /* Word16 size: 18 */


typedef struct{
  BLOCK_SWITCHING_CONTROL   blockSwitchingControl;          /* block switching */
  Word16                    *mdctDelayBuffer;               /* mdct delay buffer [BLOCK_SWITCHING_OFFSET]*/
  Word32                    sfbThresholdnm1[MAX_SFB];       /* PreEchoControl */
  Word16                    mdctScalenm1;                   /* scale of last block's mdct (PreEchoControl) */

  SFB_THRESHOLD             sfbThreshold;                   /* adapt           */
  SFB_ENERGY                sfbEnergy;                      /* sfb Energy      */
  SFB_ENERGY                sfbEnergyMS;
  SFB_ENERGY_SUM            sfbEnergySum;
  SFB_ENERGY_SUM            sfbEnergySumMS;
  SFB_ENERGY                sfbSpreadedEnergy;

  Word32                    *mdctSpectrum;                  /* mdct spectrum [FRAME_LEN_LONG] */
  Word16                    mdctScale;                      /* scale of mdct   */
}PSY_DATA; /* Word16 size: 4 + 87 + 102 + 360 + 360 + 360 + 18 + 18 + 360 = 1669 */

#endif /* _PSY_DATA_H */
