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
#include <cutils/properties.h>
#include <stdlib.h>
//


/*******************************************************************************
*
********************************************************************************/
ExifUtils::
ExifUtils()
    : IBaseExif()
    , mpzeroList(NULL)
    , mpexifList(NULL)
    , mpgpsList(NULL)
    , mpfirstList(NULL)
    , mpitopList(NULL)
    , exifGpsEnFlag(0)
    , mpexifHdrTmplBuf(NULL)
    , miLogLevel(1)
{
    char cLogLevel[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("debug.camera.exif.loglevel", cLogLevel, "1");
    miLogLevel = ::atoi(cLogLevel);
    MEXIF_LOGD("- this:%p, debug.camera.exif.loglevel=%s", this, cLogLevel);
}

/*******************************************************************************
*
********************************************************************************/
ExifUtils::
~ExifUtils()
{
    MEXIF_LOGD("");
}

/*******************************************************************************
*
********************************************************************************/
bool
ExifUtils::
init()
{
    MEXIF_LOGD_IF((2<=miLogLevel), "");
//    ifdListInit();

    return true;
}

/*******************************************************************************
*
********************************************************************************/
bool
ExifUtils::
uninit()
{
    MEXIF_LOGD_IF((2<=miLogLevel), "");
//    ifdListUninit();
    
    return true;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::exifTagUpdate(
    exifImageInfo_t *pexifImgInfo,
    exifAPP1Info_t *pexifAPP1Info
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned char buf[64];
    
    unsigned int w = pexifImgInfo->mainWidth;
    unsigned int h = pexifImgInfo->mainHeight;
    
    memcpy(buf, (unsigned char*)&w, 4);
    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_PEXELXDIM, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    memcpy(buf, (unsigned char*)&h, 4);
    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_PEXELYDIM, buf); 
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    // The exifApp1Sizeof excludes thumbnail size
    *(unsigned int*)buf = exifApp1Sizeof() - 0x0a;   
    err = ifdListNodeModify(IFD_TYPE_FIRSTIFD, IFD1_TAG_JPG_INTERCHGFMT, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    *(unsigned int*)buf = pexifImgInfo->thumbSize;
    err = ifdListNodeModify(IFD_TYPE_FIRSTIFD, IFD1_TAG_JPG_INTERCHGFMTLEN, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    /* make the string tag compatible with EXIF2.2 */
    memset(buf, 0x20, 32);
    memcpy(buf, (const char *) pexifAPP1Info->strImageDescription, strlen((const char *) pexifAPP1Info->strImageDescription));
    buf[31] = 0;
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_IMGDESC, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    /* make the string tag compatible with EXIF2.2 */
    memset(buf, 0x20, 32);
    memcpy(buf, (const char *) pexifAPP1Info->strMake, strlen((const char *) pexifAPP1Info->strMake));
    buf[31] = 0;
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_MAKE, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    /* make the string tag compatible with EXIF2.2 */
    memset(buf, 0x20, 32);
    memcpy(buf, (const char *) pexifAPP1Info->strModel, strlen((const char *) pexifAPP1Info->strModel));
    buf[31] = 0;
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_MODEL, buf);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_SOFTWARE, pexifAPP1Info->strSoftware);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_DATETIME, pexifAPP1Info->strDateTime);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_TAG_ORIENT, &pexifAPP1Info->orientation);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    // mtk definition: the index of continuous shot image. (1~n) 
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_MTK_IMGINDEX, &pexifAPP1Info->imgIndex);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    // mtk definition: group ID for continuous shot.
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_MTK_GROUPID, &pexifAPP1Info->groupID);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    // mtk definition: focus value(H) for best shot.
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_MTK_BESTFOCUSH, &pexifAPP1Info->bestFocusH);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    // mtk definition: focus value(L) for best shot.
    err = ifdListNodeModify(IFD_TYPE_ZEROIFD, IFD0_MTK_BESTFOCUSL, &pexifAPP1Info->bestFocusL);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_FLASH, &pexifAPP1Info->flash);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_DATETIMEORIG, pexifAPP1Info->strDateTime);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_DATETIMEDITI, pexifAPP1Info->strDateTime);
    if (err != EXIF_NO_ERROR) {
        return err;
    }
    
    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_EXPPROG, &pexifAPP1Info->exposureProgram);
    if (err != EXIF_NO_ERROR) {
        return err;
    }   

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_EXPTIME, pexifAPP1Info->exposureTime);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_EXPBIAS, pexifAPP1Info->exposureBiasValue);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_FNUM, pexifAPP1Info->fnumber);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_FOCALLEN, pexifAPP1Info->focalLength);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_ISOSPEEDRATE, &pexifAPP1Info->isoSpeedRatings);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_METERMODE, &pexifAPP1Info->meteringMode);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_DIGITALZOOMRATIO, pexifAPP1Info->digitalZoomRatio);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_SCENECAPTURETYPE, &pexifAPP1Info->sceneCaptureType);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_LIGHTSOURCE, &pexifAPP1Info->lightSource);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_FLRESHPIXVER, pexifAPP1Info->strFlashPixVer);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_EXPOSUREMODE, &pexifAPP1Info->exposureMode);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    err = ifdListNodeModify(IFD_TYPE_EXIFIFD, EXIF_TAG_WHITEBALANCEMODE, &pexifAPP1Info->whiteBalanceMode);
    if (err != EXIF_NO_ERROR) {
        return err;
    }

    if (exifIsGpsOnFlag()) {
        // Update GPS info
        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_ALTITUDE, &pexifAPP1Info->gpsAltitude[0]);
        if (err != EXIF_NO_ERROR) {
            return err;
        }
        
        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_LATITUDEREF, pexifAPP1Info->gpsLatitudeRef);
        if (err != EXIF_NO_ERROR) {
            return err;
        }
        
        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_LATITUDE, &pexifAPP1Info->gpsLatitude[0]);
        if (err != EXIF_NO_ERROR) {
            return err;
        }

        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_LONGITUDEREF, pexifAPP1Info->gpsLongitudeRef);
        if (err != EXIF_NO_ERROR) {
            return err;
        }

        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_LONGITUDE, &pexifAPP1Info->gpsLongitude[0]);
        if (err != EXIF_NO_ERROR) {
            return err;
        }

        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_TIMESTAMP, &pexifAPP1Info->gpsTimeStamp[0]);
        if (err != EXIF_NO_ERROR) {
            return err;
        }

        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_PROCESSINGMETHOD, pexifAPP1Info->gpsProcessingMethod);
        if (err != EXIF_NO_ERROR) {
            return err;
        }

        err = ifdListNodeModify(IFD_TYPE_GPSIFD, GPS_TAG_DATESTAMP, pexifAPP1Info->gpsDateStamp);
        if (err != EXIF_NO_ERROR) {
            return err;
        }
        //
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::exifApp1Make(
    exifImageInfo_t *pexifImgInfo,
    exifAPP1Info_t *pexifAPP1Info,
    unsigned int *pretSize
)
{
    unsigned int err = EXIF_NO_ERROR;
    unsigned int size = 0;
    unsigned int app1Size = 0;
    unsigned char *pdata;

    MEXIF_LOGD_IF((2<=miLogLevel), "+");

    ifdListInit();

    exifHdrTmplAddrSet((unsigned char *) pexifImgInfo->bufAddr);
    
    exifGpsEnFlag = pexifAPP1Info->gpsIsOn;

    err = ifdValueInit();
    if (err != EXIF_NO_ERROR) {
        MEXIF_LOGE("ifdValueInit FAIL(%x)", err);
        return err;
    }

    pdata = (unsigned char *) pexifImgInfo->bufAddr;
    // Start of Image
    exifSOIWrite(pdata, &size);
    pdata += size;
        
    /* For EXIF APP1 */
    if ( (err = exifAPP1Write(pdata, &size)) != 0 ) {
        MEXIF_LOGD("exifAPP1Write FAIL(%x)", err);
        return err;
    }
    // Fill the app1 size
    app1Size = exifApp1Sizeof() + pexifImgInfo->thumbSize;
    write16(pdata + 2, swap16(app1Size - 2));

    if ( (err = exifTagUpdate(pexifImgInfo, pexifAPP1Info)) != 0 ) {
        MEXIF_LOGD("exifTagUpdate FAIL(%x)", err);
        return err;
    }

    // Return the exif App1 size without thumbnail
    *pretSize = exifApp1Sizeof() + 2;

    ifdListUninit();

    MEXIF_LOGD_IF((2<=miLogLevel), "-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::exifAppnMake(
    unsigned int appn,
    unsigned char *paddr,
    unsigned char *pdata,
    unsigned int dataSize,
    unsigned int *pretSize
)
{
    unsigned int err = EXIF_NO_ERROR;

    // write Appn marker
    *paddr++ = 0xFF;
    *paddr++ = APP0_MARKER + appn;
    write16(paddr, swap16(dataSize));
    paddr += 2;

    memcpy(paddr, pdata, dataSize);
    
    *pretSize = dataSize + 0x02;

    return err;
}


