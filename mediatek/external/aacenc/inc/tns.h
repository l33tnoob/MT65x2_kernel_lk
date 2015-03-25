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
	File:		tns.h

	Content:	TNS structures

*******************************************************************************/

#ifndef _TNS_H
#define _TNS_H

#include "typedef.h"
#include "psy_const.h"



#define TNS_MAX_ORDER 12
#define TNS_MAX_ORDER_SHORT 5

#define FILTER_DIRECTION    0

typedef struct{ /*stuff that is tabulated dependent on bitrate etc. */
  Word16     threshOn;                /* min. prediction gain for using tns TABUL * 100*/
  Word32     lpcStartFreq;            /* lowest freq for lpc TABUL*/
  Word32     lpcStopFreq;             /* TABUL */
  Word32     tnsTimeResolution;
}TNS_CONFIG_TABULATED;


typedef struct {   /*assigned at InitTime*/
  Word16 tnsActive;
  Word16 tnsMaxSfb;

  Word16 maxOrder;                /* max. order of tns filter */
  Word16 tnsStartFreq;            /* lowest freq. for tns filtering */
  Word16 coefRes;

  TNS_CONFIG_TABULATED confTab;

  Word32 acfWindow[TNS_MAX_ORDER+1];

  Word16 tnsStartBand;
  Word16 tnsStartLine;

  Word16 tnsStopBand;
  Word16 tnsStopLine;

  Word16 lpcStartBand;
  Word16 lpcStartLine;

  Word16 lpcStopBand;
  Word16 lpcStopLine;

  Word16 tnsRatioPatchLowestCb;
  Word16 tnsModifyBeginCb;

  Word16 threshold; /* min. prediction gain for using tns TABUL * 100 */

}TNS_CONFIG;


typedef struct {
  Word16 tnsActive;
  Word32 parcor[TNS_MAX_ORDER];
  Word16 predictionGain;
} TNS_SUBBLOCK_INFO; /* Word16 size: 26 */

typedef struct{
  TNS_SUBBLOCK_INFO subBlockInfo[TRANS_FAC];
} TNS_DATA_SHORT;

typedef struct{
  TNS_SUBBLOCK_INFO subBlockInfo;
} TNS_DATA_LONG;

typedef struct{
  TNS_DATA_LONG tnsLong;
  TNS_DATA_SHORT tnsShort;
}TNS_DATA_RAW;

typedef struct{
  Word16 numOfSubblocks;
  TNS_DATA_RAW dataRaw;
}TNS_DATA; /* Word16 size: 1 + 8*26 + 26 = 235 */

typedef struct{
  Word16 tnsActive[TRANS_FAC];
  Word16 coefRes[TRANS_FAC];
  Word16 length[TRANS_FAC];
  Word16 order[TRANS_FAC];
  Word16 coef[TRANS_FAC*TNS_MAX_ORDER_SHORT];
}TNS_INFO; /* Word16 size: 72 */

#endif /* _TNS_H */
