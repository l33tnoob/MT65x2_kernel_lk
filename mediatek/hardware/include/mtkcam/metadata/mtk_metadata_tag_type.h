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


#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_TYPE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_METADATA_MTK_METADATA_TAG_TYPE_H_

#include <string.h>
#include <stdint.h>
#include <cutils/compiler.h>

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * Type definitions for camera_metadata_entry
 * =============================================================================
 */
enum {
    TYPE_UNKNOWN = -1,
    // Unsigned 8-bit integer (uint8_t)
    TYPE_MUINT8, 
    //TYPE_MUINT8,
    // Signed 32-bit integer (int32_t)
    TYPE_MINT32,
    //TYPE_MINT32,
    // 32-bit float (float)
    TYPE_MFLOAT,
    //TYPE_MFLOAT,
    // Signed 64-bit integer (int64_t)
    TYPE_MINT64,
    //TYPE_MINT64,
    // 64-bit float (double)
    TYPE_MDOUBLE,
    //TYPE_MDOUBLE,
    // A 64-bit fraction (camera_metadata_rational_t)
    TYPE_MRational,
    //TYPE_MRational,
    // [MTK added]
    TYPE_MPoint,
    
    TYPE_MSize,
    
    TYPE_MRect,
    
    TYPE_IMetadata,    
    
    // Number of type fields    
    NUM_MTYPES
};


/**
 * Retrieve the type of a tag. Returns -1 if no such tag is defined. Returns -1
 * for tags in the vendor section, unless set_vendor_tag_query_ops() has been
 * used.
 */
int get_mtk_metadata_tag_type(unsigned int tag);

/**
 * Retrieve the name of a tag. Returns NULL if no such tag is defined. 
 */
char const* get_mtk_metadata_tag_name(unsigned int tag);

#ifdef __cplusplus
}
#endif

#endif 
