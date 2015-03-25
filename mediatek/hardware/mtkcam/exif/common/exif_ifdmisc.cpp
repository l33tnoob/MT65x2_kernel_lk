/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/
//
#include <string.h>
//
#include "IBaseExif.h"
#include "Exif.h"
#include "exif_type.h"
#include "exif_errcode.h"
//


/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListSizeof (
)
{
    unsigned int size = 0;
    zeroIFDList_t *pzeroList;
    exifIFDList_t *pexifList;
    gpsIFDList_t *pgpsList;
    firstIFDList_t *pfirstList;
    itopIFDList_t *pitopList;
    
    pzeroList = ifdZeroListGet();
    pexifList = ifdExifListGet();
    pgpsList = ifdGpsListGet();
    pfirstList = ifdFirstListGet();
    pitopList = ifdItopListGet();
    
    size += (pzeroList->nodeCnt * sizeof(IFD_t) + pzeroList->valBufPos);
    size += (pexifList->nodeCnt * sizeof(IFD_t) + pexifList->valBufPos);
    size += (pgpsList->nodeCnt * sizeof(IFD_t) + pgpsList->valBufPos);
    size += (pfirstList->nodeCnt * sizeof(IFD_t) + pfirstList->valBufPos);  
    size += (pitopList->nodeCnt * sizeof(IFD_t) + pitopList->valBufPos);    
    
    return size;
}

/*******************************************************************************
*
********************************************************************************/
unsigned char* 
ExifUtils::ifdListValBufGet (
    unsigned int ifdType
)
{
    unsigned int err = EXIF_NO_ERROR;
    void *plist;
    unsigned char *pdata = 0;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        plist = ifdZeroListGet();
        pdata = ((zeroIFDList_t*)plist)->valBuf;
        break;
    case IFD_TYPE_EXIFIFD:
        plist = ifdExifListGet();
        pdata = ((exifIFDList_t*)plist)->valBuf;
        break;
    case IFD_TYPE_GPSIFD:
        plist = ifdGpsListGet();
        pdata = ((gpsIFDList_t*)plist)->valBuf;
        break;        
    case IFD_TYPE_FIRSTIFD:
        plist = ifdFirstListGet();
        pdata = ((firstIFDList_t*)plist)->valBuf;
        break;
    case IFD_TYPE_ITOPIFD:
        plist = ifdItopListGet();
        pdata = ((itopIFDList_t*)plist)->valBuf;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    
    return pdata;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListValBufSizeof (
    unsigned int ifdType
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned int cnt = 0;
    void *plist;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        plist = ifdZeroListGet();
        cnt = ((zeroIFDList_t*)plist)->valBufPos;
        break;
    case IFD_TYPE_EXIFIFD:
        plist = ifdExifListGet();
        cnt = ((exifIFDList_t*)plist)->valBufPos;
        break;
    case IFD_TYPE_GPSIFD:
        plist = ifdGpsListGet();
        cnt = ((gpsIFDList_t*)plist)->valBufPos;
        break;        
    case IFD_TYPE_FIRSTIFD:
        plist = ifdFirstListGet();
        cnt = ((firstIFDList_t*)plist)->valBufPos;
        break;
    case IFD_TYPE_ITOPIFD:
        plist = ifdItopListGet();
        cnt = ((itopIFDList_t*)plist)->valBufPos;
        break;  
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    return cnt;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdListNodeCntGet(
    unsigned int ifdType
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned int cnt = 0;
    void *plist = 0;
    
    switch (ifdType) {
    case IFD_TYPE_ZEROIFD:
        plist = ifdZeroListGet();
        cnt = ((zeroIFDList_t*)plist)->nodeCnt;
        break;
    case IFD_TYPE_EXIFIFD:
        plist = ifdExifListGet();
        cnt = ((exifIFDList_t*)plist)->nodeCnt;
        break;
    case IFD_TYPE_GPSIFD:
        plist = ifdGpsListGet();
        cnt = ((gpsIFDList_t*)plist)->nodeCnt;
        break;        
    case IFD_TYPE_FIRSTIFD:
        plist = ifdFirstListGet();
        cnt = ((firstIFDList_t*)plist)->nodeCnt;
        break;
    case IFD_TYPE_ITOPIFD:
        plist = ifdItopListGet();
        cnt = ((itopIFDList_t*)plist)->nodeCnt;
        break;
    default:
        err = LIBEXIF_IFD_ERR0001;
        break;
    }
    
    return cnt;
}

