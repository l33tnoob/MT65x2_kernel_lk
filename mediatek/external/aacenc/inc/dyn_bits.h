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
	File:		dyn_bits.h

	Content:	Noiseless coder module structure and functions

*******************************************************************************/

#ifndef __DYN_BITS_H
#define __DYN_BITS_H

#include "psy_const.h"
#include "tns.h"
#include "bit_cnt.h"



#define MAX_SECTIONS          MAX_GROUPED_SFB
#define SECT_ESC_VAL_LONG    31
#define SECT_ESC_VAL_SHORT    7
#define CODE_BOOK_BITS        4
#define SECT_BITS_LONG        5
#define SECT_BITS_SHORT       3

typedef struct
{
  Word16 codeBook;
  Word16 sfbStart;
  Word16 sfbCnt;
  Word16 sectionBits;
}
SECTION_INFO;




typedef struct
{
  Word16 blockType;
  Word16 noOfGroups;
  Word16 sfbCnt;
  Word16 maxSfbPerGroup;
  Word16 sfbPerGroup;
  Word16 noOfSections;
  SECTION_INFO sectionInfo[MAX_SECTIONS];
  Word16 sideInfoBits;             /* sectioning bits       */
  Word16 huffmanBits;              /* huffman    coded bits */
  Word16 scalefacBits;             /* scalefac   coded bits */
  Word16 firstScf;                 /* first scf to be coded */
  Word16 bitLookUp[MAX_SFB_LONG*(CODE_BOOK_ESC_NDX+1)];
  Word16 mergeGainLookUp[MAX_SFB_LONG];
}
SECTION_DATA; /*  Word16 size: 10 + 60(MAX_SECTIONS)*4(SECTION_INFO) + 51(MAX_SFB_LONG)*12(CODE_BOOK_ESC_NDX+1) + 51(MAX_SFB_LONG) = 913 */


Word16 BCInit(void);

Word16 dynBitCount(const Word16 *quantSpectrum,
                   const UWord16 *maxValueInSfb,
                   const Word16 *scalefac,
                   const Word16 blockType,
                   const Word16 sfbCnt,
                   const Word16 maxSfbPerGroup,
                   const Word16 sfbPerGroup,
                   const Word16 *sfbOffset,
                   SECTION_DATA *sectionData);

#endif
