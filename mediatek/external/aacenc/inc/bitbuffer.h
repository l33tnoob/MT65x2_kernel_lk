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
	File:		bitbuffer.h

	Content:	Bit Buffer Management structure and functions

*******************************************************************************/

#ifndef BITBUFFER_H
#define BITBUFFER_H

#include "typedef.h"


enum direction
{
  forwardDirection,
  backwardDirection
};


/*!
   The pointer 'pReadNext' points to the next available word, where bits can be read from. The pointer
   'pWriteNext' points to the next available word, where bits can be written to. The pointer pBitBufBase
   points to the start of the bitstream buffer and the pointer pBitBufEnd points to the end of the bitstream
   buffer. The two pointers are used as lower-bound respectively upper-bound address for the modulo addressing
   mode.

   The element cntBits contains the currently available bits in the bit buffer. It will be incremented when
   bits are written to the bitstream buffer and decremented when bits are read from the bitstream buffer.
*/
struct BIT_BUF
{
  UWord8 *pBitBufBase;          /*!< pointer points to first position in bitstream buffer */
  UWord8 *pBitBufEnd;           /*!< pointer points to last position in bitstream buffer */

  UWord8 *pWriteNext;           /*!< pointer points to next available word in bitstream buffer to write */

  UWord32 cache;

  Word16  wBitPos;              /*!< 31<=wBitPos<=0*/
  Word16  cntBits;              /*!< number of available bits in the bitstream buffer
                                     write bits to bitstream buffer  => increment cntBits
                                     read bits from bitstream buffer => decrement cntBits */
  Word16  size;                 /*!< size of bitbuffer in bits */
  Word16  isValid;              /*!< indicates whether the instance has been initialized */
}; /* size Word16: 8 */

/*! Define pointer to bit buffer structure */
typedef struct BIT_BUF *HANDLE_BIT_BUF;


HANDLE_BIT_BUF CreateBitBuffer(HANDLE_BIT_BUF hBitBuf,
                               UWord8 *pBitBufBase,
                               Word16  bitBufSize);


void DeleteBitBuffer(HANDLE_BIT_BUF *hBitBuf);


Word16 GetBitsAvail(HANDLE_BIT_BUF hBitBuf);


Word16 WriteBits(HANDLE_BIT_BUF hBitBuf,
                 UWord32 writeValue,
                 Word16 noBitsToWrite);

void ResetBitBuf(HANDLE_BIT_BUF hBitBuf,
                 UWord8 *pBitBufBase,
                 Word16  bitBufSize);

#define GetNrBitsAvailable(hBitBuf) ( (hBitBuf)->cntBits)
#define GetNrBitsRead(hBitBuf)       ((hBitBuf)->size-(hBitBuf)->cntBits)

#endif /* BITBUFFER_H */
