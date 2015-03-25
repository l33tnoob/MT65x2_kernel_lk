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
unsigned short 
ExifUtils::swap16 (
    unsigned short x
)
{   
    x = (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8));
    return x;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::swap32 (
    unsigned int x
)
{   
    x = (((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8) | ((x & 0x000000ff) << 24));
    return x;
}

/*******************************************************************************
*
********************************************************************************/
unsigned short 
ExifUtils::swap16ByOrder (
    unsigned short order,
    unsigned short x
)
{   
    if (order == 0x4D4D)
        x = (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8));
    
    return x;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::swap32ByOrder (
    unsigned short order,
    unsigned int x
)
{   
    if (order == 0x4D4D)
        x = (((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8) | ((x & 0x000000ff) << 24));

    return x;
}

/*******************************************************************************
*
********************************************************************************/
unsigned short
ExifUtils::read16 ( 
    void *psrc
)
{
    unsigned char *pdata;
    unsigned short ret;

    pdata = (unsigned char *)psrc;
    ret = (*(pdata + 1) << 8) + (*pdata);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::read32 (
    void *psrc
)
{
    unsigned char *pdata;
    unsigned int ret;

    pdata = (unsigned char *)psrc;
    ret = (*(pdata + 3) << 24) + 
    (*(pdata + 2) << 16) + 
    (*(pdata + 1) << 8 ) + 
    (*(pdata    )      );

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
void
ExifUtils::write16 (
    void *pdst,
    unsigned short src
)
{
    unsigned char byte0, byte1;
    unsigned char *pdata;

    pdata = (unsigned char *)pdst;
    byte0 = (unsigned char )((src     ) & 0x00ff);
    byte1 = (unsigned char )((src >> 8) & 0x00ff);
    *pdata = byte0;
    pdata++;
    *pdata = byte1;
}

/*******************************************************************************
*
********************************************************************************/
void
ExifUtils::write32 (
    void *pdst,
    unsigned int src
)
{
    unsigned char byte0, byte1, byte2, byte3;
    unsigned char *pdata;

    pdata = (unsigned char *)pdst;
    byte0 = (unsigned char )((src      ) & 0x000000ff);
    byte1 = (unsigned char )((src >> 8 ) & 0x000000ff);
    byte2 = (unsigned char )((src >> 16) & 0x000000ff);
    byte3 = (unsigned char )((src >> 24) & 0x000000ff);
    *pdata = byte0;
    pdata++;
    *pdata = byte1;
    pdata++;
    *pdata = byte2;
    pdata++;
    *pdata = byte3;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
ExifUtils::exifMemcmp (
    unsigned char *pdst,
    unsigned char *psrc,
    unsigned int size
)
{
    while ( size > 0 ) {
        if ( *pdst != *psrc ) {
            break;
        }
        pdst++;
        psrc++;
        size--;
    }

    return size;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::exifApp1Sizeof (
)
{
    unsigned int size;

    size = 0x0a + sizeof(TiffHeader_t) + ifdListSizeof()+ IFD_TYPE_ITOPIFD * 6 ; /* was 18 */

    // This size excludes thumbnail size
    
    return size;
}


/*******************************************************************************
*
********************************************************************************/
unsigned int 
ExifUtils::exifIFDValueSizeof (
    unsigned short type, 
    unsigned int count
)
{
    unsigned int size = 0;
    
    switch (type) {
    case IFD_DATATYPE_BYTE:
    case IFD_DATATYPE_UNDEFINED:
    case IFD_DATATYPE_ASCII:
        size = count;
        break;
    case IFD_DATATYPE_SHORT:
        size = count << 1;  
        break;
    case IFD_DATATYPE_SLONG:
    case IFD_DATATYPE_LONG:
        size = count << 2;  
        break;
    case IFD_DATATYPE_RATIONAL:
    case IFD_DATATYPE_SRATIONAL:
        size = count << 3;  
        break;
    default:
        MEXIF_LOGE("Unsupport tag, type(%d), err = %x\n", type, LIBEXIF_IFD_ERR0002);
        break;
    }

    return size;
}

/*******************************************************************************
*
********************************************************************************/
void 
ExifUtils::exifErrPrint (
    unsigned char *pname, 
    unsigned int err
)
{
    switch(err) {
    case EXIF_NO_ERROR:
        break;
    case LIBEXIF_FILE_ERR0001:
        MEXIF_LOGE("Error in %s() call, Unsupport file format, err  = %x\n", pname, err);
        break;
    
    case LIBEXIF_APP1_ERR0001:
        MEXIF_LOGE("Error in %s() call, THumbnail not found, err = %x\n", pname, err);
        break;  
    case LIBEXIF_APP1_ERR0002:
        MEXIF_LOGE("Error in %s() call, TIFF header error, err  =%x\n", pname, err);
        break;      
    case LIBEXIF_DQT_ERR0001:
        MEXIF_LOGE("Error in %s() call, Too many DQT found, err  =%x\n", pname, err);
        break;
#ifdef EXIF_WARNING_DEBUG
    case LIBEXIF_SOI_ERR0001:
        MEXIF_LOGE("Error in %s() call, SOI not found, err =%x\n", pname, err);
        break;  
    case LIBEXIF_EOI_ERR0001:
        MEXIF_LOGE("Error in %s() call, EOI not found, err = %x\n", pname, err);
        break;
#endif
    case LIBEXIF_DQT_ERR0002:
        MEXIF_LOGE("Error in %s() call, DQT not found!, err = %x\n", pname, err);
        break;  
    case LIBEXIF_DQT_ERR0003:
    case LIBEXIF_DHT_ERR0002:
    case LIBEXIF_DHT_ERR0004:
    case LIBEXIF_DHT_ERR0003:
    case LIBEXIF_DHT_ERR0005:   
    case LIBEXIF_DHT_ERR0006:   
        MEXIF_LOGE("Error in %s() call, Unsupport DHT found, err = %x\n", pname, err);
        break;
    case LIBEXIF_SOF_ERR0001:
        MEXIF_LOGE("Error in %s() call, SOF not found, err = %x\n", pname, err);
        break;  
    case LIBEXIF_SOF_ERR0002:
        MEXIF_LOGE("Error in %s() call, Support SOF length, err = %x\n", pname, err);
        break;      
    case LIBEXIF_SOF_ERR0003:
        MEXIF_LOGE("Error in %s() call, Unsupport data format, err = %x\n", pname, err);
        break;      
    case LIBEXIF_SOS_ERR0001:
        MEXIF_LOGE("Error in %s() call, SOS not found, err = %x\n", pname, err);;
        break;  
    case LIBEXIF_SOS_ERR0002:
        MEXIF_LOGE("Error in %s() call, Support SOS length, err = %x\n", pname, err);
        break;  
    case LIBEXIF_MISC_ERR0001:
        MEXIF_LOGE("Error in %s() call, Unknow Maker!, err = %x\n", pname, err);
        break;  
    case LIBEXIF_MISC_ERR0002:
        MEXIF_LOGE("Error in %s() call, file size overflow!, err = %x\n", pname, err);
        break;
    case LIBEXIF_IFD_ERR0001:
        MEXIF_LOGE(" Error in %s() call, not support IFD list!, err = %x\n", pname, err);
        break;
    case LIBEXIF_IFD_ERR0002:
        MEXIF_LOGE("Error in %s() call, Unsupport tag!, err = %x\n", pname, err);
        break;
    case LIBEXIF_IFD_ERR0005:   
        break;
    default:
        MEXIF_LOGE("Error in %s() call, Unknow err code!, err = %x\n", pname, err);

    }
}


