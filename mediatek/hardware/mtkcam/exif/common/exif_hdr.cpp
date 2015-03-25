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
ExifUtils::exifSOIWrite(
    unsigned char *pdata,
    unsigned int *pretSize
)
{
    unsigned int err = EXIF_NO_ERROR;
    
    *pdata++ = 0xFF;
    *pdata++ = SOI_MARKER;  

    *pretSize = 0x02;
        
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::exifAPP1Write(
    unsigned char *pdata,
    unsigned int *pretSize
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned int tagIdx;
    unsigned int ifdValOffset = 0;
    unsigned int ifdSize, tiffHdrSize, size;
    unsigned int nextIFD0Offest;
    unsigned int nodeCnt;
    ifdNode_t *pnode;
    unsigned char *pstart;
    unsigned int value32 = 0;
    unsigned int exifNextPtr = 0;
    unsigned char  buf[12];
    unsigned short value16;
    TiffHeader_t tiff;

    *pretSize = 0;
    pstart = pdata;
    ifdSize = sizeof(IFD_t);
    tiffHdrSize = sizeof(TiffHeader_t);

    *pdata++ = 0xFF;
    *pdata++ = APP1_MARKER;
    *pretSize += 0x02;
    //write16(pdata, value16);  Fill the size later
    
    pdata += 2;/* App1 len */
    *pretSize += 2;
    
    strcpy((char *) pdata, "Exif"); /* "Exif"00 */
    pdata += 5;
    *pdata++ = 0x00; /* pad */
    *pretSize += 6;

    /* TIFF header */
    #if 0
    ((TiffHeader_t*)pdata)->byteOrder = 0x4949;
    ((TiffHeader_t*)pdata)->fixed = 0x002A;
    ((TiffHeader_t*)pdata)->ifdOffset = tiffHdrSize;
    #else
    tiff.byteOrder = 0x4949;
    tiff.fixed = 0x002A;
    tiff.ifdOffset = tiffHdrSize;
    memcpy(pdata, &tiff, tiffHdrSize);
    #endif
    
    pdata += tiffHdrSize;
    *pretSize += tiffHdrSize;
    ifdValOffset = tiffHdrSize; /* offset from the start of the TIFF header */

    /* find next IFD0 offset */
    nextIFD0Offest = tiffHdrSize;
    for (tagIdx = IFD_TYPE_ZEROIFD; tagIdx <= IFD_TYPE_GPSIFD; tagIdx++) {
        if ((tagIdx == IFD_TYPE_GPSIFD) && (!exifIsGpsOnFlag())){
            continue;
        }
        nextIFD0Offest += (2 + ifdListNodeCntGet(tagIdx) * ifdSize + ifdListValBufSizeof(tagIdx) + 4);
    }

    /* parsing all IFDs from 3 pre-defined IDF arrays */
    for (tagIdx = IFD_TYPE_ZEROIFD; tagIdx <= IFD_TYPE_ITOPIFD; tagIdx++) {
        nodeCnt = ifdListNodeCntGet(tagIdx);
        if (tagIdx == IFD_TYPE_EXIFIFD) {
            exifNextPtr = ifdValOffset;
        }

        if (tagIdx == IFD_TYPE_GPSIFD) {
            if (!exifIsGpsOnFlag()){
                continue;
            }
            exifNextPtr = ifdValOffset;
        }
        
        if (tagIdx == IFD_TYPE_ITOPIFD) {
            exifNextPtr = ifdValOffset;
        }

        write16(pdata, nodeCnt);/* numnber of IFD interoperability */
        pdata += 2;
        *pretSize += 2;
        ifdValOffset += (nodeCnt * ifdSize + 2 + 4);
        /* fill IFD nodes to template header and save each entry's offset for quick access */
        pnode = idfListHeadNodeGet(tagIdx);

        while (pnode) {
            // Special case for GPS
            memcpy(pdata, (unsigned char*)&pnode->ifd, ifdSize);
                        
            /**((IFD_t*)pdata) = (IFD_t)pnode->ifd;*/
            if (exifIFDValueSizeof(pnode->ifd.type, pnode->ifd.count) > 4) {/* record data to somewhere */  
                write32((unsigned char*)&((IFD_t*)pdata)->valoff, pnode->ifd.valoff + ifdValOffset);
                value32 = read32((unsigned char*)&((IFD_t*)pdata)->valoff);
                write32((unsigned char*)&pnode->ifd.valoff, value32 + 0x0c);
            }
            else {
                write32((unsigned char*)&pnode->ifd.valoff, *pretSize  + ifdSize - 2);              
            }
            
            pdata += ifdSize;
            *pretSize += ifdSize;
            pnode = pnode->next; /* pointer to next IFD */
        }
        
        if (tagIdx == IFD_TYPE_ZEROIFD) {
            write32(pdata, nextIFD0Offest); /* this address will be filled with next ifd0 offset */
        }
        else {
            write32(pdata, 0);
        }
        
        pdata += 4; /* offset */
        *pretSize += 4;
                
        /* copy value buffer */
        size = ifdListValBufSizeof(tagIdx);
        if (size) {
            memcpy(pdata, ifdListValBufGet(tagIdx), size);
            pdata += size; /* offset */
            ifdValOffset += size;
            *pretSize += size;      
        }
        
        if (tagIdx == IFD_TYPE_EXIFIFD) {
            write32(buf, exifNextPtr);
            ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_EXIFPTR, buf);
        }

        if (tagIdx == IFD_TYPE_GPSIFD) {
            write32(buf, exifNextPtr);
            ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_GPSINFO, buf);
        }

        if (tagIdx == IFD_TYPE_ITOPIFD) { /* update */
            write32(buf, exifNextPtr);
            ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_ITOPIFDPTR, buf);
        }
    }

    exifErrPrint((unsigned char *) "exifAPP1Write", err);
    
    return err;
}

