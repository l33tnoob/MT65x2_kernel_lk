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
#include "exif_log.h"
//


/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdValueInit()
{
    unsigned int err = 0;
    unsigned int idx;
    ifdNode_t *pnode = NULL;

    for (idx = 0; idx < (sizeof(zeroTagID) >> 1); idx++) {
        if ((zeroTagID[idx] == IFD0_TAG_GPSINFO) && (exifIsGpsOnFlag() == 0)) {
            continue;
        }
        
        pnode = ifdListNodeAlloc(IFD_TYPE_ZEROIFD);
        if (!pnode) {
            MEXIF_LOGE("ifdListNodeAlloc FAIL(ZEROIFD)");
            return LIBEXIF_IFD_ERR0004;
        }
        
        pnode->ifd.tag = zeroTagID[idx];
        if ((err = ifdZeroIFDValInit(pnode, ifdZeroListGet())) == 0) {
            ifdListNodeInsert(IFD_TYPE_ZEROIFD, pnode, 0);
        }
    }

    for (idx = 0; idx < (sizeof(exifTagID) >> 1); idx++) {
        pnode = ifdListNodeAlloc(IFD_TYPE_EXIFIFD);
        if (!pnode) {
            MEXIF_LOGE("ifdListNodeAlloc FAIL(EXIFIFD)");
            return LIBEXIF_IFD_ERR0004;
        }
        
        pnode->ifd.tag = exifTagID[idx];
        if ((err = ifdExifIFDValInit(pnode, ifdExifListGet())) == 0) {
            ifdListNodeInsert(IFD_TYPE_EXIFIFD, pnode, 0);
        }
    }

    for (idx = 0; idx < (sizeof(gpsTagID) >> 1); idx++) {
        pnode = ifdListNodeAlloc(IFD_TYPE_GPSIFD);
        if (!pnode) {
            MEXIF_LOGE("ifdListNodeAlloc FAIL(GPSIFD)");
            return LIBEXIF_IFD_ERR0004;
        }
        
        pnode->ifd.tag = gpsTagID[idx];
        if ((err = ifdGpsIFDValInit(pnode, ifdGpsListGet())) == 0) {
            ifdListNodeInsert(IFD_TYPE_GPSIFD, pnode, 0);
        }
    }

    for (idx = 0; idx < (sizeof(firstTagID) >> 1); idx++) {
        pnode = ifdListNodeAlloc(IFD_TYPE_FIRSTIFD);
        if (!pnode) {
            MEXIF_LOGE("ifdListNodeAlloc FAIL(FIRSTIFD)");
            return LIBEXIF_IFD_ERR0004;
        }
        
        pnode->ifd.tag = firstTagID[idx];
        if ((err = ifdFirstIFDValInit(pnode, ifdFirstListGet())) == 0) {
            ifdListNodeInsert(IFD_TYPE_FIRSTIFD, pnode, 0);
        }
    }

    for (idx = 0; idx < (sizeof(itopTagID) >> 1); idx++) {
        pnode = ifdListNodeAlloc(IFD_TYPE_ITOPIFD);
        if (!pnode) {
            MEXIF_LOGE("ifdListNodeAlloc FAIL(ITOPIFD)");
            return LIBEXIF_IFD_ERR0004;
        }
        
        pnode->ifd.tag = itopTagID[idx];
        if ((err = ifdItopIFDValInit(pnode, ifdItopListGet())) == 0) {
            ifdListNodeInsert(IFD_TYPE_ITOPIFD, pnode, 0);
        }
    }
    
    exifErrPrint((unsigned char *) "ifdValueInit", err);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdZeroIFDValInit(
    ifdNode_t *pnode, 
    struct zeroIFDList_t *plist
)
{   
    unsigned int err = 0;
    unsigned char *pdata;
    IFD_t *pifd;
    unsigned int idx = 0;
    
    pdata = plist->valBuf + plist->valBufPos;
    pifd = &pnode->ifd;

    while (idx < plist->nodeCnt) {
        if (plist->ifdNodePool[idx].ifd.tag == pifd->tag) {
            MEXIF_LOGE("IFD duplicated! tag(0x%x)", pifd->tag);
            err = LIBEXIF_IFD_ERR0005;
            return err;
        }
        idx ++;
    }
    
    switch (pifd->tag) {
    case IFD0_TAG_IMGDESC:
        strcpy((char *) pdata , "Unknown Image Title            ");
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->valoff = plist->valBufPos;
        pifd->count = 0x20;
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case IFD0_TAG_MAKE:
        strcpy((char *) pdata , "Unknown Manufacturer Name");
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->valoff = plist->valBufPos;
        pifd->count = 0x20;
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case IFD0_TAG_MODEL:
        strcpy((char *) pdata , "Unknown Model Name ");
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->valoff = plist->valBufPos;
        pifd->count = 0x20;
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case IFD0_TAG_ORIENT:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff , 1); /* no rotatation */
        break;
    case IFD0_TAG_XRES:
    case IFD0_TAG_YRES:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        write32( pdata , 72);
        pdata += sizeof(unsigned int);
        write32( pdata , 1);
        pdata += sizeof(unsigned int);
        plist->valBufPos += (sizeof(unsigned int) << 1);
        break;
    case IFD0_TAG_RESUNIT:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff , 2);/* inches */
        break;
    case IFD0_TAG_SOFTWARE:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 32;
        pifd->valoff = plist->valBufPos;
        strcpy((char *) pdata , "MediaTek Camera Application");
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case IFD0_TAG_DATETIME:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 20;
        pifd->valoff = plist->valBufPos;
        strcpy((char *) pdata , "2002:01:24 17:35:30"); /* get date/time from RTC */
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case IFD0_TAG_YCBCRPOS:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 2); /* cosite */
        break;
    case IFD0_MTK_IMGINDEX:     // mtk definition: the index of continuous shot image. (1~n)
    case IFD0_MTK_GROUPID:      // mtk definition: group ID for continuous shot.
    case IFD0_MTK_BESTFOCUSH:   // mtk definition: focus value (H) for best shot.
    case IFD0_MTK_BESTFOCUSL:   // mtk definition: focus value (L) for best shot.
    case IFD0_TAG_EXIFPTR:
    case IFD0_TAG_GPSINFO:
        pifd->type = IFD_DATATYPE_LONG;
        pifd->count = 1;
        break;        
    default:
        err = LIBEXIF_IFD_ERR0002;
        break;
    }

    exifErrPrint((unsigned char *) "ifdZeroIFDValInit", err);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdExifIFDValInit(
    ifdNode_t *pnode, 
    struct exifIFDList_t *plist
)
{
    unsigned int err = 0;
    unsigned char *pdata;
    IFD_t *pifd;
    unsigned int idx = 0;
    /*unsigned char timeBuf[20];*/
    
    pdata = plist->valBuf + plist->valBufPos;
    pifd = &pnode->ifd;
    
    while (idx < plist->nodeCnt) {
        if (plist->ifdNodePool[idx].ifd.tag == pifd->tag) {
            MEXIF_LOGE("IFD duplicated! tag(0x%x)", pifd->tag);
            err = LIBEXIF_IFD_ERR0005;
            return err;
        }
        idx ++;
    }

    switch (pifd->tag) {        
    case EXIF_TAG_EXPTIME:
    case EXIF_TAG_FNUM:
    case EXIF_TAG_COMPRESSBPP:
    /*case EXIF_TAG_EXPBIAS: */
    case EXIF_TAG_FOCALLEN:
    case EXIF_TAG_MAXAPTURE:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += (sizeof(unsigned int) << 1);
        pdata += (sizeof(unsigned int) << 1);
        break;
    case EXIF_TAG_EXPBIAS:
        pifd->type = IFD_DATATYPE_SRATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += (sizeof(signed int) << 1);
        pdata += (sizeof(signed int) << 1);
        break;
    case EXIF_TAG_USRCOMMENT:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 256;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += 256;
        pdata += 256;
        break;
    case EXIF_TAG_EXPPROG:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 2);/* normal mode */
        break;
    case EXIF_TAG_ISOSPEEDRATE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 0x64);
        break;
    case EXIF_TAG_EXIFVER:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 4;
        memcpy(&pifd->valoff, exifVersion, 4); /* No null for termination */
        break;
    case EXIF_TAG_DATETIMEORIG:
    case EXIF_TAG_DATETIMEDITI:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 20;
        pifd->valoff = plist->valBufPos;
        #if 0
        /*exifAscRTCGet(timeBuf);*/
        strncpy(&timeBuf[10], " ", 1);
        strcpy(pdata, timeBuf);
        #else
        strcpy((char *) pdata , "2002:01:24 17:35:30"); 
        #endif
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    case EXIF_TAG_COMPCONFIGURE:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 4;
        write32((unsigned char*)&pifd->valoff, 0x00030201);
        break;
    case EXIF_TAG_METERMODE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff , 2);/* CenterWeightedAverage */
        break;
    #if 0   
    case EXIF_TAG_AUDIOFILE:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 13;
        pifd->valoff = plist->valBufPos;
/*      strcpy(pdata, "DSC_0047.WAV");*/
        plist->valBufPos += pifd->count + 1;
        pdata += pifd->count;
        break;
    #endif  
    case EXIF_TAG_ITOPIFDPTR:
        pifd->type = IFD_DATATYPE_LONG;
        pifd->count = 1;
        write32((unsigned char*)&pifd->valoff, 0x00000000);
        break;
    case EXIF_TAG_LIGHTSOURCE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 2);
        break;
    case EXIF_TAG_FLASH:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 0); /* strobe return light detected */
        break;
    case EXIF_TAG_FLRESHPIXVER:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 4;
        memcpy((unsigned char*)&pifd->valoff, "0100", 4); /* No null for termination */
        break;
    case EXIF_TAG_COLORSPACE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 1); /*srgb */
        break;
    case EXIF_TAG_PEXELXDIM:
    case EXIF_TAG_PEXELYDIM:
        pifd->type = IFD_DATATYPE_LONG;
        pifd->count = 1;
        write32((unsigned char*)&pifd->valoff, 1024); /*srgb */
        break;
    /*case IDF_EXIF_INTEROPIFDPTR:
        break;*/
    case EXIF_TAG_FILESOURCE:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 3); /* DSC */
        break;
    case EXIF_TAG_SENCETYPE:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 1); /* directly photographed */
        break;
    case EXIF_TAG_DIGITALZOOMRATIO:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += (sizeof(unsigned int) << 1);
        pdata += (sizeof(unsigned int) << 1);
        break;
    case EXIF_TAG_SCENECAPTURETYPE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 0); /*scenecapturetype*/
        break;
    case EXIF_TAG_EXPOSUREMODE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 0); /*exposureMode*/
        break;
    case EXIF_TAG_WHITEBALANCEMODE:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 0); /*whiteBalanceMode*/
        break;
    default:
        err = LIBEXIF_IFD_ERR0002;
        break;
    }

    exifErrPrint((unsigned char *) "ifdExifIFDValInit", err);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdGpsIFDValInit(
    ifdNode_t *pnode, 
    struct gpsIFDList_t *plist
)
{
    unsigned int err = 0;
    unsigned char *pdata;
    IFD_t *pifd;
    unsigned int idx = 0;
    /*unsigned char timeBuf[20];*/
    
    pdata = plist->valBuf + plist->valBufPos;
    pifd = &pnode->ifd;
    
    while (idx < plist->nodeCnt) {
        if (plist->ifdNodePool[idx].ifd.tag == pifd->tag) {
            MEXIF_LOGE("IFD duplicated! tag(0x%x)", pifd->tag);
            err = LIBEXIF_IFD_ERR0005;
            return err;
        }
        idx ++;
    }

    switch (pifd->tag) {
    case GPS_TAG_VERSIONID:
        pifd->type = IFD_DATATYPE_BYTE;
        pifd->count = 4;
        memcpy(&pifd->valoff, gpsVersion, 4); /* No null for termination */
        break;
    case GPS_TAG_ALTITUDEREF:
        pifd->type = IFD_DATATYPE_BYTE;
        pifd->count = 1;
        pifd->valoff = 0;
        break;
    case GPS_TAG_LATITUDEREF:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 2;
        memcpy(&pifd->valoff, "N", 2); // Give default value
        break;
    case GPS_TAG_LONGITUDEREF:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 2;
        memcpy(&pifd->valoff, "E", 2); // Give default value
        break;        
    case GPS_TAG_LATITUDE:
    case GPS_TAG_LONGITUDE:
    case GPS_TAG_TIMESTAMP:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 3;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += (sizeof(unsigned int) << 1) * 3;
        pdata += (sizeof(unsigned int) << 1) * 3;
        break;
    case GPS_TAG_ALTITUDE:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += (sizeof(unsigned int) << 1) * 1;
        pdata += (sizeof(unsigned int) << 1) * 1;
        break;
    case GPS_TAG_PROCESSINGMETHOD:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 64;
        pifd->valoff = plist->valBufPos;
        plist->valBufPos += 64;
        pdata += 64;    
        break;
    case GPS_TAG_DATESTAMP:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->valoff = plist->valBufPos;
        pifd->count = 11;
        plist->valBufPos += pifd->count;
        pdata += pifd->count;
        break;
    default:
        err = LIBEXIF_IFD_ERR0002;
        break;
    }

    exifErrPrint((unsigned char *) "ifdGpsIFDValInit", err);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdFirstIFDValInit(
    ifdNode_t *pnode, 
    struct firstIFDList_t *plist
)
{   
    unsigned int err = 0;
    unsigned char* pdata;
    IFD_t* pifd;
    unsigned int idx = 0;

    pdata = plist->valBuf + plist->valBufPos;
    pifd = &pnode->ifd;
    
    while (idx < plist->nodeCnt) {
        if (plist->ifdNodePool[idx].ifd.tag == pifd->tag) {
            MEXIF_LOGE("IFD duplicated! tag(0x%x)", pifd->tag);
            err = LIBEXIF_IFD_ERR0005;
            return err;
        }
        idx ++;
    }
    
    switch (pifd->tag) {
    case IFD1_TAG_COMPRESS:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 6);/* JPEG thumbnail compress */
        break;
    case IFD1_TAG_ORIENT:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 1); /* no rotatation */
        break;
    case IFD1_TAG_XRES:
    case IFD1_TAG_YRES:
        pifd->type = IFD_DATATYPE_RATIONAL;
        pifd->count = 1;
        pifd->valoff = plist->valBufPos;
        write32(pdata , 0x48);
        pdata += 4;
        write32(pdata , 0x01);
        pdata += 4;
        plist->valBufPos += (sizeof(unsigned int) << 1);
        break;
    case IFD1_TAG_RESUINT:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 2); /* inches */
        break;
    case IFD1_TAG_JPG_INTERCHGFMT: /*thumbnail offset from TIFF header */
        pifd->type = IFD_DATATYPE_LONG;
        pifd->count = 1;
        break;
    case IFD1_TAG_JPG_INTERCHGFMTLEN: /*thumbnail length (from SOI to EOI) */
        pifd->type = IFD_DATATYPE_LONG;
        pifd->count = 1;
        break;
    case IFD1_TAG_YCBCRPOS:
        pifd->type = IFD_DATATYPE_SHORT;
        pifd->count = 1;
        write16((unsigned char*)&pifd->valoff, 2);/* cosite */
        break;
    default:
        err = LIBEXIF_IFD_ERR0002;
        break;
    }
    
    exifErrPrint((unsigned char *) "ifdFirstIFDValInit", err);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::ifdItopIFDValInit(
    ifdNode_t *pnode, 
    struct itopIFDList_t *plist
)
{   
    unsigned int err = 0;
    unsigned char* pdata;
    IFD_t* pifd;
    unsigned int idx = 0;

    pdata = plist->valBuf + plist->valBufPos;
    pifd = &pnode->ifd;
    
    while (idx < plist->nodeCnt) {
        if (plist->ifdNodePool[idx].ifd.tag == pifd->tag) {
            MEXIF_LOGE("IFD duplicated! tag(0x%x)", pifd->tag);
            err = LIBEXIF_IFD_ERR0005;
            return err;
        }
        idx ++;
    }

    switch (pifd->tag) {
    case ITOP_TAG_ITOPINDEX:
        pifd->type = IFD_DATATYPE_ASCII;
        pifd->count = 4;
        strcpy((char *)&pifd->valoff, "R98\0");/* JPEG thumbnail compress */
        break;
    case ITOP_TAG_ITOPVERSION:
        pifd->type = IFD_DATATYPE_UNDEFINED;
        pifd->count = 4;
        memcpy((unsigned char*)&pifd->valoff, "0100", 4); /* No null for termination */
        break;
    default:
        err = LIBEXIF_IFD_ERR0002;
        break;
    }
    
    exifErrPrint((unsigned char *) "ifditopIFDValInit", err);
    
    return err;
}


