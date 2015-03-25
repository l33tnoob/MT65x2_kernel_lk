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

/********************************************************************************
 *	SRS Labs CONFIDENTIAL
 *	@Copyright 2010 by SRS Labs.
 *	All rights reserved.
 *
 *  Description:
 *  Exposes all srs_fft APIs
 *
 *	Author: Zesen Zhuang
 *
 *  RCS keywords:
 *	$Id$
 *  $Author$
 *  $Date$
 *	
********************************************************************************/

#ifndef __SRS_FFT_DEF_H__
#define __SRS_FFT_DEF_H__

#include "srs_typedefs.h"

/*Data type definition here:*/
typedef enum
{
	SRS_CFFT_32C16_RDX2,
	SRS_RFFT_32C16_RDX2,
	SRS_RFFT_16C16_RDX2
} SRSFftType;

struct tSRSFftTbl;
typedef struct tSRSFftTbl SRSFftTbl;	/* SRS Fixed Point FFT Library specific structure */

/* table sizes constants of SRSFftTbl, needed if call <SRS_Cfft/SRS_InvCfft>_32c16_rdx2  */
#define SRS_CFFT_32C16_RDX2_32_SIZE (84+8+4)
#define SRS_CFFT_32C16_RDX2_64_SIZE (148+8+4)
#define SRS_CFFT_32C16_RDX2_128_SIZE (276+8+4)
#define SRS_CFFT_32C16_RDX2_256_SIZE (532+8+4)
#define SRS_CFFT_32C16_RDX2_512_SIZE (1044+8+4)
#define SRS_CFFT_32C16_RDX2_1024_SIZE (2068+8+4)
#define SRS_CFFT_32C16_RDX2_2048_SIZE (4116+8+4)
#define SRS_CFFT_32C16_RDX2_4096_SIZE (8212+8+4)

/* table sizes constants of SRSFftTbl, needed if call SRS_Rfft_32c16_rdx2 or SRS_InvRfft_32c16_rdx2 */
#define SRS_RFFT_32C16_RDX2_32_SIZE (84+8+4)
#define SRS_RFFT_32C16_RDX2_64_SIZE (148+8+4)
#define SRS_RFFT_32C16_RDX2_128_SIZE (276+8+4)
#define SRS_RFFT_32C16_RDX2_256_SIZE (532+8+4)
#define SRS_RFFT_32C16_RDX2_512_SIZE (1044+8+4)
#define SRS_RFFT_32C16_RDX2_1024_SIZE (2068+8+4)
#define SRS_RFFT_32C16_RDX2_2048_SIZE (4116+8+4)
#define SRS_RFFT_32C16_RDX2_4096_SIZE (8212+8+4)

/* table sizes constants of SRSFftTbl, needed if call SRS_Rfft_16c16_rdx2 or SRS_InvRfft_16c16_rdx2  */
#define SRS_RFFT_16C16_RDX2_32_SIZE (84+8+4)
#define SRS_RFFT_16C16_RDX2_64_SIZE (148+8+4)
#define SRS_RFFT_16C16_RDX2_128_SIZE (276+8+4)
#define SRS_RFFT_16C16_RDX2_256_SIZE (532+8+4)
#define SRS_RFFT_16C16_RDX2_512_SIZE (1044+8+4)
#define SRS_RFFT_16C16_RDX2_1024_SIZE (2068+8+4)
#define SRS_RFFT_16C16_RDX2_2048_SIZE (4116+8+4)
#define SRS_RFFT_16C16_RDX2_4096_SIZE (8212+8+4)


#define SRS_RFFT_32C16_RDX2_160_SIZE (SRS_RFFT_32C16_RDX2_32_SIZE+160*sizeof(srs_int32)+8)
#define SRS_RFFT_32C16_RDX2_320_SIZE (SRS_RFFT_32C16_RDX2_64_SIZE+320*sizeof(srs_int32)+8)

#endif //__SRS_FFT_DEF_H__
