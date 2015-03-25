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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkUtility.cpp

#define LOG_TAG "AcdkUtility"

#include <linux/cache.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include "mtkcam/common.h"
using namespace NSCam;
#include "mtkcam/acdk/AcdkTypes.h"

#include <mtkcam/camshot/ISImager.h>

using namespace NSCamShot; 
using namespace NSCamHW;

#include "AcdkLog.h"
#include "AcdkErrCode.h"

#include "AcdkUtility.h"
using namespace NSACDK;

/*******************************************************************************
* 
********************************************************************************/
#define PIXEL_BYTE_FP 2
#define ROUND_TO_2X(x) ((x) & (~0x1))

/*******************************************************************************
* 
********************************************************************************/
static const MUINT32 g_capRange  = 8;
static MUINT32 g_capWidthRange[g_capRange]  = {320,640,1024,1280,1600,2048,2560,3264};
static MUINT32 g_capHeightRange[g_capRange] = {240,480,768,960,1200,1536,1920,2448};

static const MUINT32 g_prvRange = 4;
static MUINT32 g_prvWidthRange[g_prvRange]  = {320,640,800,1024};
static MUINT32 g_prvHeightRange[g_prvRange] = {240,480,600,768};


/*******************************************************************************
*
********************************************************************************/
AcdkUtility *AcdkUtility::createInstance()
{
    ACDK_LOGD("createInstance");
    return new AcdkUtility;    
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::destroyInstance() 
{
    delete this; 
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::queryPrvSize(MUINT32 &oriW, MUINT32 &oriH) 
{
    ACDK_LOGD("oriW(%u),oriH(%u)",oriW,oriH);
    
    MUINT32 tempW = oriW, tempH = oriH;
    MBOOL isFound = MFALSE;

    for(MUINT32 i = 0; i < (g_prvRange - 1); ++i)
    {
        if((tempW == g_prvWidthRange[i] && tempH == g_prvHeightRange[i]) || (tempW >= g_prvWidthRange[i] && tempW < g_prvWidthRange[i + 1]))
        {
            oriW = g_prvWidthRange[i];
            oriH = g_prvHeightRange[i];
            isFound = MTRUE;
            break;
        }        
    }

    if(isFound == MFALSE)
    {
        oriW = g_prvWidthRange[g_prvRange - 1];
        oriH = g_prvHeightRange[g_prvRange - 1];
    }

    ACDK_LOGD("before ROUND_TO_2X - oriW(%u),oriH(%u)",oriW,oriH);

    oriW = ROUND_TO_2X(oriW);
    oriH = ROUND_TO_2X(oriH);

    ACDK_LOGD("X - oriW(%u),oriH(%u)",oriW,oriH);
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::queryCapSize(MUINT32 &oriW, MUINT32 &oriH) 
{
    ACDK_LOGD("oriW(%u),oriH(%u)",oriW,oriH);
    
    MUINT32 tempW = oriW, tempH = oriH;
    MBOOL isFound = MFALSE;

    for(MUINT32 i = 0; i < (g_capRange - 1); ++i)
    {
        if(tempW == g_capWidthRange[i] && tempH == g_capHeightRange[i])
        {
            oriW = g_capWidthRange[i];
            oriH = g_capHeightRange[i];
            isFound = MTRUE;
            break;
        }   
        else if(tempW >= g_capWidthRange[i] && tempW < g_capWidthRange[i + 1])
        {
            oriW = g_capWidthRange[i + 1];
            oriH = g_capHeightRange[i + 1];
            isFound = MTRUE;
            break;
        }
    }

    if(isFound == MFALSE)
    {
        oriW = g_capWidthRange[g_capRange - 1];
        oriH = g_capHeightRange[g_capRange - 1];
    }

    ACDK_LOGD("before align 16x - oriW(%u),oriH(%u)",oriW,oriH);

    oriW = oriW & ~(0xf);
    oriH = oriH & ~(0xf);
    
    ACDK_LOGD("X - oriW(%u),oriH(%u)",oriW,oriH);
}

/*******************************************************************************
* 
*******************************************************************************/
MINT32 AcdkUtility::queryRAWImgFormatInfo(MUINT32 const imgFmt, MUINT32 u4ImgWidth, MUINT32 &u4Stride, MUINT32 &pixel_byte)
{
    MUINT32 stride = u4Stride;
        
    ACDK_LOGD("imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);
    
    if(u4ImgWidth % 2 || u4Stride % 2) 
    {
        ACDK_LOGE("width and stride should be even number");
    }
    
    switch(imgFmt) 
    {
        case eImgFmt_BAYER8 :   //= 0x0001,   //Bayer format, 8-bit
            pixel_byte = 1 << PIXEL_BYTE_FP;
            break;
        case eImgFmt_BAYER10 :  //= 0x0002,   //Bayer format, 10-bit
            pixel_byte = (5 << PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25

            if(stride % 8)
            {
                stride = stride + 8 - (stride % 8);
            }
                     
            if(u4Stride < stride)
            {
                ACDK_LOGE("RAW10 STRIDE SHOULD BE MULTIPLE OF 8(%u)->(%u)",u4Stride,stride);
            }
            break;
        case eImgFmt_BAYER12 :   //= 0x0004,   //Bayer format, 12-bit
            pixel_byte = (3 << PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
           
            if(stride % 4) 
            {
                stride = stride + 4 - (stride % 4);
            }
            
            if(u4Stride < stride) 
            {
                ACDK_LOGE("RAW12 STRIDE SHOULD BE MULTIPLE OF 4(%u)->(%u)",u4Stride,stride);
            }
            break;
        default:
            ACDK_LOGE("NOT SUPPORT imgFmt(%u)",imgFmt);
            return ACDK_RETURN_INVALID_PARA;
    }
    
    u4Stride = stride;
    ACDK_LOGD("X:imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);
    return ACDK_RETURN_NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryImageSize(MUINT32 imgFormat, MUINT32 imgW, MUINT32 imgH, MUINT32 &imgSize)
{
    ACDK_LOGD("imgFormat(0x%x)",imgFormat); 

    MINT32 err = ACDK_RETURN_NO_ERROR;
   
    switch(imgFormat)
    {
        // YUV 420 format 
        case eImgFmt_YV12:
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
        case eImgFmt_I420:
            imgSize = imgW * imgH * 3 / 2; 
            break; 
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61: 
        case eImgFmt_RGB565:
            imgSize = imgW * imgH * 2; 
            break;
        case eImgFmt_RGB888:         
            imgSize = imgW * imgH * 3; 
            break; 
        case eImgFmt_ARGB888:
            imgSize = imgW * imgH * 3; 
            break; 
        case eImgFmt_JPEG:
            imgSize = imgW * imgH / 4;    //? assume the JPEG ratio is 1/4 
            break; 
        case eImgFmt_Y800:
            imgSize = imgW * imgH; 
            break;
        default:
            imgSize = 0;
            ACDK_LOGE("cannot calculate image size");
            err = ACDK_RETURN_INVALID_PARA;
            break;
    }

    ACDK_LOGD("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryImageStride(MUINT32 imgFormat, MUINT32 imgW, MUINT32 planeIndex, MUINT32 *imgStride)
{
    ACDK_LOGD("imgFormat(0x%x)",imgFormat); 
   
    switch(imgFormat)
    {
        // YUV 420 format 
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            *imgStride = (planeIndex == 2) ? (0) : (imgW); 
            break; 
        case eImgFmt_YV12:
        case eImgFmt_I420:
            *imgStride = (planeIndex == 0) ? (imgW) : (imgW / 2); 
            break; 
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_RGB565:
            *imgStride = (planeIndex == 0) ? (imgW) : 0; 
            break; 
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:        
            *imgStride = (planeIndex == 0) ? (imgW) : (imgW / 2); 
            break;         
        case eImgFmt_RGB888:         
            *imgStride = imgW; 
            break; 
        case eImgFmt_ARGB888:
            *imgStride = imgW; 
            break; 
        case eImgFmt_JPEG:
            *imgStride = imgW ; 
            break; 
        case eImgFmt_Y800:
            *imgStride = (planeIndex == 0) ? (imgW) : (0); 
            break; 
        default:
            *imgStride = imgW; 
            break; 
    } 

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::imageProcess(MUINT32 imgOutFormat, 
                                       MUINT32 srcImgW, 
                                       MUINT32 srcImgH, 
                                       MUINT32 orientaion,
                                       MUINT32 flip,
                                       IMEM_BUF_INFO srcImem, 
                                       IMEM_BUF_INFO dstImem,
                                       MUINT32 dstImgW,
                                       MUINT32 dstImgH)
{
    ACDK_LOGD("format(0x%x)",imgOutFormat);
  
    //====== Variable Setting ======
    
    // stride
    MUINT32 qvStride[3] = {0, 0, 0};    
    queryImageStride(eImgFmt_YUY2,srcImgW,0,&qvStride[0]);
    queryImageStride(eImgFmt_YUY2,srcImgW,1,&qvStride[1]);
    queryImageStride(eImgFmt_YUY2,srcImgW,2,&qvStride[2]);

    ACDK_LOGD("srcImgW(%u),srcImgH(%u)",srcImgW,srcImgH);
    ACDK_LOGD("orientaion(%u),flip(%u)",orientaion,flip);
    ACDK_LOGD("qvStride: 0(%u),1(%u),2(%u)",qvStride[0],qvStride[1],qvStride[2]);
    ACDK_LOGD("srcImem : size(%u),vir(0x%x),phy(0x%x),mimID(%d)",srcImem.size,srcImem.virtAddr,srcImem.phyAddr,srcImem.memID);
    ACDK_LOGD("dstImem : size(%u),vir(0x%x),phy(0x%x),mimID(%d)",dstImem.size,dstImem.virtAddr,dstImem.phyAddr,dstImem.memID);
   
    // src image buffer    
    ImgBufInfo rSrcImgInfo(ImgInfo(eImgFmt_YUY2, srcImgW, srcImgH), 
                            BufInfo(srcImem.size, srcImem.virtAddr, srcImem.phyAddr, srcImem.memID), qvStride);

    // dst image buffer
    BufInfo rDstImgInfo(dstImem.size, dstImem.virtAddr, dstImem.phyAddr, dstImem.memID); 

    //create SImage object
    ISImager *sImager = ISImager::createInstance(rSrcImgInfo);

    //====== SImage Process ======
    
    sImager->setTargetBufInfo(rDstImgInfo);     
    sImager->setFormat((EImageFormat)imgOutFormat);   
    sImager->setRotation(orientaion);
    sImager->setFlip(flip);

    if(dstImgW == 0 || dstImgH == 0)
    {
        sImager->setResize(srcImgW, srcImgH);
    }
    else
    {
        ACDK_LOGD("dstImgW(%u),dstImgH(%u)",dstImgW,dstImgH);
        sImager->setResize(dstImgW, dstImgH);
    }

    sImager->setROI(Rect(0, 0, srcImgW, srcImgH));
    sImager->execute();

    //====== Destory Instance ======
    
    sImager->destroyInstance();

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::rawImgUnpack(IMEM_BUF_INFO srcImem,
                                       IMEM_BUF_INFO dstImem,
                                       MUINT32 a_imgW,
                                       MUINT32 a_imgH,
                                       MUINT32 a_bitDepth,
                                       MUINT32 a_Stride)
{   

    ACDK_LOGD("srcImem : VA(0x%x),PA(0x%x),ID(%d),SZ(%u)",srcImem.virtAddr,
                                                           srcImem.phyAddr,
                                                           srcImem.memID,
                                                           srcImem.size);

    ACDK_LOGD("dstImem : VA(0x%x),PA(0x%x),ID(%d),SZ(%u)",dstImem.virtAddr,
                                                           dstImem.phyAddr,
                                                           dstImem.memID,
                                                           dstImem.size);

    ACDK_LOGD("imgW(%u),imgH(%u),bitDepth(%u),stride(%u)",a_imgW,
                                                           a_imgH,
                                                           a_bitDepth,
                                                           a_Stride);
  
    //====== Unpack ======

    MUINT8 *pSrcBuf = (MUINT8 *)srcImem.virtAddr;
    MUINT16 *pDstBuf = (MUINT16 *)dstImem.virtAddr;

    if(a_bitDepth == 8)
    {
        MUINT8 pixelValue;
        for(MUINT32 i = 0; i < (a_imgW * a_imgH); ++i)
        {
            pixelValue = *(pSrcBuf++);
            *(pDstBuf) = pixelValue;
        }
    }
    else if(a_bitDepth == 10)
    {
        MUINT8 *lineBuf;
        
        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;

            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                *(pDstBuf++) = (MUINT16)(((byte1 & 0xFC) >> 2) + ((byte2 & 0xF) << 6));
                *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 4));
                *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
            }

            //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));  

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)(((byte1 & 0x3F) >> 2) + ((byte2 & 0xF) << 6));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 6));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
                            break;
                    }
                }
            }
        }
    }
    else if(a_bitDepth == 12)
    {
        MUINT8 *lineBuf;
        
        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;
            
            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
            }

             //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
                            break;
                    }
                }
            }
        }
    }

    return ACDK_RETURN_NO_ERROR;
    ACDK_LOGD("-");
}



