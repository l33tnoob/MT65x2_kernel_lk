/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#ifndef _MHAL_JPEG_H_
#define _MHAL_JPEG_H_

/*****************************************************************************
* Include Header File
*****************************************************************************/

#include "MediaHal.h"

#define MHAL_IMAGE_TYPE_JPEG 0x0000
#define MHAL_IMAGE_TYPE_WEBP 0x0001

/*****************************************************************************
* MHAL Jpeg Decoder API
*****************************************************************************/

int mHalJpgDecParser(unsigned char* srcAddr, unsigned int srcSize, int srcFD, unsigned int img_type);

int mHalJpgDecGetInfo(MHAL_JPEG_DEC_INFO_OUT *decOutInfo);

int mHalJpgDecStart(MHAL_JPEG_DEC_START_IN *decInParams, void* procHandler = NULL);
                     

int mi_mHalJpgDecParser(MHAL_JPEG_DEC_SRC_IN *srcInParams,unsigned char* srcAddr, unsigned int srcSize, int srcFD, unsigned int img_type);
int mi_mHalJpgDecGetInfo(MHAL_JPEG_DEC_INFO_OUT *decOutInfo);
int mi_mHalJpgDecStart(MHAL_JPEG_DEC_START_IN *decInParams, void* procHandler = NULL);                     
/*****************************************************************************
* MHAL Jpeg Encoder API
*****************************************************************************/

int mHalJpgEncStart(MHAL_JPEG_ENC_START_IN *encInParams); 

int mHalScaler_BitBlt( mHalBltParam_t* bltParam );

#endif

