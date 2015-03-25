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
	File:		aacenc_core.h

	Content:	aac encoder interface functions

*******************************************************************************/

#ifndef _aacenc_core_h_
#define _aacenc_core_h_


#include "typedef.h"
#include "config.h"
#include "bitenc.h"

#include "psy_configuration.h"
#include "psy_main.h"
#include "qc_main.h"
#include "psy_main.h"
/*-------------------------- defines --------------------------------------*/


/*-------------------- structure definitions ------------------------------*/
typedef  struct {
  Word32   sampleRate;            /* audio file sample rate */
  Word32   bitRate;               /* encoder bit rate in bits/sec */
  Word16   nChannelsIn;           /* number of channels on input (1,2) */
  Word16   nChannelsOut;          /* number of channels on output (1,2) */
  Word16   bandWidth;             /* targeted audio bandwidth in Hz */
  Word16   adtsUsed;			  /* whether write adts header */
} AACENC_CONFIG;


typedef struct {

  AACENC_CONFIG config;     /* Word16 size: 8 */

  ELEMENT_INFO elInfo;      /* Word16 size: 4 */

  QC_STATE qcKernel;        /* Word16 size: 6 + 5(PADDING) + 7(ELEMENT_BITS) + 54(ADJ_THR_STATE) = 72 */
  QC_OUT   qcOut;           /* Word16 size: MAX_CHANNELS*920(QC_OUT_CHANNEL) + 5(QC_OUT_ELEMENT) + 7 = 932 / 1852 */

  PSY_OUT    psyOut;        /* Word16 size: MAX_CHANNELS*186 + 2 = 188 / 374 */
  PSY_KERNEL psyKernel;     /* Word16 size:  2587 / 4491 */

  struct BITSTREAMENCODER_INIT bseInit; /* Word16 size: 6 */
  struct BIT_BUF  bitStream;            /* Word16 size: 8 */
  HANDLE_BIT_BUF  hBitStream;
  int			  initOK;

  short			*intbuf;
  short			*encbuf;
  short			*inbuf;
  int			enclen;
  int			inlen;
  int			intlen;
  int			uselength;

  void			*hCheck;
  VO_MEM_OPERATOR *voMemop;
  VO_MEM_OPERATOR voMemoprator;

}AAC_ENCODER; /* Word16 size: 3809 / 6851 */

/*-----------------------------------------------------------------------------

functionname: AacInitDefaultConfig
description:  gives reasonable default configuration
returns:      ---

------------------------------------------------------------------------------*/
void AacInitDefaultConfig(AACENC_CONFIG *config);

/*---------------------------------------------------------------------------

functionname:AacEncOpen
description: allocate and initialize a new encoder instance
returns:     AACENC_OK if success

---------------------------------------------------------------------------*/

Word16  AacEncOpen (AAC_ENCODER				*hAacEnc,       /* pointer to an encoder handle, initialized on return */
                    const  AACENC_CONFIG     config);        /* pre-initialized config struct */

Word16 AacEncEncode(AAC_ENCODER		   *hAacEnc,
                    Word16             *timeSignal,
                    const UWord8       *ancBytes,      /*!< pointer to ancillary data bytes */
                    Word16             *numAncBytes,   /*!< number of ancillary Data Bytes, send as fill element  */
                    UWord8             *outBytes,      /*!< pointer to output buffer            */
                    VO_U32             *numOutBytes    /*!< number of bytes in output buffer */
                    );

/*---------------------------------------------------------------------------

functionname:AacEncClose
description: deallocate an encoder instance

---------------------------------------------------------------------------*/

void AacEncClose (AAC_ENCODER* hAacEnc, VO_MEM_OPERATOR *pMemOP); /* an encoder handle */

#endif /* _aacenc_h_ */
