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
// AcdkImgTool.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkImgTool.cpp
//! \brief
#define LOG_TAG "AcdkImgTool"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "AcdkLog.h"
#include <mtkcam/acdk/cct_feature.h>
#include "cct_imgtool.h"
#include "cct_ErrCode.h"

#include <math.h>

#define MS 1000
#define MEDIA_PATH "//data"

//only used in this module for command callback
static AcdkImgTool* g_pAcdkImgToolObj = NULL;


/////////////////////////////////////////////////////////////////////////
//
//   AcdkImgTool()-
//!  brief contructor of AcdkImgTool
//!
/////////////////////////////////////////////////////////////////////////
AcdkImgTool::AcdkImgTool()
{
    ACDK_LOGD(" AcdkImgTool Constructor\n");
    m_bAcdkImgTool = FALSE;

    g_pAcdkImgToolObj= this;
}

/////////////////////////////////////////////////////////////////////////
//
//   AcdkImgTool()-
//!  brief distructor of AcdkImgTool
//!
/////////////////////////////////////////////////////////////////////////
AcdkImgTool::~AcdkImgTool()
{
    ACDK_LOGD(" AcdkImgTool Deconstructor \n");

    //m_pAcdkCLICmds = NULL;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrEnableAcdkImgTool () -
//!  brief enable AcdkImgTool
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrEnableAcdkImgTool()
{
    ACDK_LOGD(" Eanble AcdkImgTool !!\n");

    m_bAcdkImgTool = TRUE;
    return S_CCT_IMGTOOL_OK;
}



/////////////////////////////////////////////////////////////////////////
//
//   mrDisableAcdkImgTool () -
//!  brief disable AcdkImgTool
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrDisableAcdkImgTool()
{
    ACDK_LOGD(" Disable AcdkImgTool !!\n");

    m_bAcdkImgTool = FALSE;
    return S_CCT_IMGTOOL_OK;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrInitAcdkImgTool () -
//!  brief init the tuning tool module
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrInitAcdkImgTool()
{
    ACDK_LOGD(" Init AcdkImgTool\n");

    return S_CCT_IMGTOOL_OK;
}



/////////////////////////////////////////////////////////////////////////
//
//   mrUnPackRawImg () -
//!  brief unpack the RAW image
//!
/////////////////////////////////////////////////////////////////////////
VOID AcdkImgTool::vUnPackRawImg(MUINT8 *a_pPackedImgBuf, MUINT8 *a_pUnPackedImgBuf,  MUINT32 a_u4ImgSize, MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT8 a_bitdepth)
{
    int width = a_u4Width, bitdepth = a_bitdepth, height = a_u4Height;
    int stride;

    if ((a_pPackedImgBuf== NULL) || (a_pUnPackedImgBuf== NULL)) {
        return;
    }

    if(bitdepth == 10) {
        if((width * bitdepth) & 0x7) {
            stride = (width * bitdepth + 7) >> 3;
            stride = ((7 + stride) >> 3) << 3;  //for 8 alignment
        }
        else {
            stride = (width * bitdepth) >> 3;
        }
    }
    else if(bitdepth == 12) {
        stride = (width * bitdepth + 7) >> 3;
        stride = ((5 + stride)/6) * 6;
    }


    int w_div4 = width >> 2;
    int w_res4 = width & 3;
    unsigned char byte0, byte1, byte2, byte3, byte4;
    unsigned char *psrc = (unsigned char *)a_pPackedImgBuf;
    unsigned short *pdst = (unsigned short *)a_pUnPackedImgBuf;

    if(bitdepth == 8) {
        MUINT16 pix;

        for(int i=0;i<a_u4ImgSize; i++) {
            pix = *(psrc++);
            *(pdst++) = (pix << 2);
        }
    }
    else if(bitdepth == 10) {
        for(int h=0;h<height; h++) {

            MUINT8 *lineBuf = psrc + h * stride;

            for(int w=0;w<w_div4; w++) {
                byte0 = (MUINT8)(*(lineBuf++));
                byte1 = (MUINT8)(*(lineBuf++));
                byte2 = (MUINT8)(*(lineBuf++));
                byte3 = (MUINT8)(*(lineBuf++));
                byte4 = (MUINT8)(*(lineBuf++));

                *(pdst++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8));
                *(pdst++) = (unsigned short) (((byte1 & 0xFC) >> 2) + ((byte2 & 0xf) << 6));
                *(pdst++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 4));
                *(pdst++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2));

            }


            // process last pixel in the width
            if(w_res4 != 0) {
                byte0 = *(lineBuf++);
	    	    byte1 = *(lineBuf++);
		        byte2 = *(lineBuf++);
		        byte3 = *(lineBuf++);
			    byte4 = *(lineBuf++);

                for(int w=0;w<w_res4; w++) {
                    switch(w) {
                        case 0:
                            *(pdst++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8));
                            break;
                        case 1:
                            *(pdst++) = (unsigned short) (((byte1 & 0x3F) >> 2) + ((byte2 & 0xf) << 6));
                            break;
                        case 2:
                            *(pdst++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 6));
                            break;
                        case 3:
                            *(pdst++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2));
                            break;
                    }
                }
            }

        }
    }
    else if(bitdepth == 12) {
        //N.A.
    }

    return;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrAnalyzeRAWInfo () -
//!  brief Analyze the RAW info
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrAnalyzeRAWInfo(MUINT8 *a_pUnPackedImgBuf,
                                                                          UINT32 a_u4ImgWidth,
                                                                          UINT32 a_u4ImgHeight,
                                                                          eRAW_ColorOrder a_eColorOrder,
                                                                          const ROIRect &a_strROI,
                                                                          RAWAnalyzeResult &a_pStrRawInfoResult)
{
    UINT16 *pu2ImgBuf;

    INT64 i8TempCh[4] = {0, 0, 0, 0};
    INT64 i8ChSquareSum[4] = {0, 0, 0, 0};

    //UINT8 uColorOrder;
    //INT64 i8TempY;
    MRESULT mrRet = S_CCT_IMGTOOL_OK;
    ACDK_LOGD(" mrAnalyzeRAWInfo () -  \n");
    pu2ImgBuf=(UINT16*)a_pUnPackedImgBuf;

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth) {
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    //make sure  ROI in 2x boundary
    if ( (a_strROI.u4StartX | a_strROI.u4ROIWidth | a_strROI.u4StartY |a_strROI.u4ROIHeight ) & (UINT32)0x01){
       ACDK_LOGE(" The ROI arguments shoud be even number \n");
       return E_CCT_IMGTOOL_BAD_ARG;
    }

    UINT32 u4Index = 0;
    UINT32 u4Count = 0;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row+=2) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col+=2) {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth)  + (a_strROI.u4StartX + col) ;

            i8TempCh[0] += pu2ImgBuf[u4Index];
            i8TempCh[1] += pu2ImgBuf[u4Index + 1];
            i8TempCh[2] += pu2ImgBuf[u4Index + a_u4ImgWidth ];
            i8TempCh[3] += pu2ImgBuf[u4Index + a_u4ImgWidth + 1];
            u4Count++;
        }
    }

    //avoid to divide by zero
    if (u4Count == 0) {
        u4Count = 1;
    }
    //calc the average;
    i8TempCh[0] /= u4Count;
    i8TempCh[1] /= u4Count;
    i8TempCh[2] /= u4Count;
    i8TempCh[3] /= u4Count;

    u4Count = 0;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row+=2) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col+=2) {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth)  + (a_strROI.u4StartX + col) ;

            i8ChSquareSum[0] += ((pu2ImgBuf[u4Index] - i8TempCh[0]) * (pu2ImgBuf[u4Index] - i8TempCh[0]));
            i8ChSquareSum[1] += ((pu2ImgBuf[u4Index + 1] - i8TempCh[1]) * (pu2ImgBuf[u4Index + 1] - i8TempCh[1]));
            i8ChSquareSum[2] += ((pu2ImgBuf[u4Index + a_u4ImgWidth ] - i8TempCh[2]) * (pu2ImgBuf[u4Index + a_u4ImgWidth] - i8TempCh[2]));
            i8ChSquareSum[3] += ((pu2ImgBuf[u4Index + a_u4ImgWidth  + 1] -i8TempCh[3]) * (pu2ImgBuf[u4Index + a_u4ImgWidth + 1] -i8TempCh[3]));
            u4Count++;
        }
    }

    //avoid divide by zero
    if (u4Count == 0) {
        u4Count = 2;
    }

    switch (a_eColorOrder) {
        case RawPxlOrder_R:     //R, Gr, Gb,B
            a_pStrRawInfoResult.u4RAvg = (UINT16)i8TempCh[0];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)i8TempCh[1];
            a_pStrRawInfoResult.u4BAvg = (UINT16)i8TempCh[3];
            a_pStrRawInfoResult.u4GbAvg =  (UINT16)i8TempCh[2];
            a_pStrRawInfoResult.u4RStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[0] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GrStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[1] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4BStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[3] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GbStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[2] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            break;
        case RawPxlOrder_Gb:     //Gb, B, R, Gr
            a_pStrRawInfoResult.u4RAvg = (UINT16)i8TempCh[2];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)i8TempCh[3];
            a_pStrRawInfoResult.u4BAvg = (UINT16)i8TempCh[1];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)i8TempCh[0];
            a_pStrRawInfoResult.u4RStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[2] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GrStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[3] / (DOUBLE)(u4Count -1)), 0.5)* 100);
            a_pStrRawInfoResult.u4BStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[1] / (DOUBLE)(u4Count -1)), 0.5)* 100);
            a_pStrRawInfoResult.u4GbStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[0] / (DOUBLE)(u4Count -1)), 0.5)* 100);
            break;
        case RawPxlOrderr_Gr:    //Gr, R, B, Gb
            a_pStrRawInfoResult.u4RAvg = (UINT16)i8TempCh[1];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)i8TempCh[0];
            a_pStrRawInfoResult.u4BAvg = (UINT16)i8TempCh[2];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)i8TempCh[3];
            a_pStrRawInfoResult.u4RStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[1] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GrStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[0] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4BStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[2]/ (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GbStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[3] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            break;
        case RawPxlOrder_B:     //B, Gb, Gr, R
            a_pStrRawInfoResult.u4RAvg = (UINT16)i8TempCh[3];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)i8TempCh[2];
            a_pStrRawInfoResult.u4BAvg = (UINT16)i8TempCh[0];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)i8TempCh[1];
            a_pStrRawInfoResult.u4RStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[3] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GrStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[2] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4BStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[0] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GbStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[1] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            break;
        default:
            a_pStrRawInfoResult.u4RAvg = (UINT16)i8TempCh[0];
            a_pStrRawInfoResult.u4GrAvg =(UINT16)i8TempCh[1];
            a_pStrRawInfoResult.u4BAvg = (UINT16)i8TempCh[3];
            a_pStrRawInfoResult.u4GbAvg = (UINT16) i8TempCh[2];
            a_pStrRawInfoResult.u4RStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[0] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GrStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[1] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4BStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[3] /(DOUBLE)(u4Count -1)), 0.5) * 100);
            a_pStrRawInfoResult.u4GbStd = (UINT16)(pow((DOUBLE)(i8ChSquareSum[2] / (DOUBLE)(u4Count -1)), 0.5) * 100);
            break;
    }

    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrAnalyzeRAWImage () -
//!  brief Analyze RAW image (average and median)
//
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrAnalyzeRAWImage(MUINT8 *a_pUnPackedImgBuf,
                                                                                    UINT32 a_u4ImgWidth,
                                                                                    UINT32 a_u4ImgHeight,
                                                                                    eRAW_ColorOrder a_eColorOrder,
                                                                                    const ROIRect &a_strROI,
                                       ACDK_CDVT_RAW_ANALYSIS_RESULT_T &a_rRAWAnalysisResult)
{
    UINT16 *pu2ImgBuf;

    INT64 i8ChannelSum[4] = {0, 0, 0, 0};
    DOUBLE dChannelAvg[4] = {0, 0, 0, 0};
    UINT32 u4Histogram[1024] = {};

    MRESULT mrRet = S_CCT_IMGTOOL_OK;

    ACDK_LOGD(" AcdkImgTool::mrAnalyzeRAWImage() \n");

    pu2ImgBuf=(UINT16*)a_pUnPackedImgBuf;

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth) {
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    //make sure  ROI in 2x boundary
    if ( (a_strROI.u4StartX | a_strROI.u4ROIWidth | a_strROI.u4StartY |a_strROI.u4ROIHeight ) & (UINT32)0x01) {
       ACDK_LOGE(" The ROI arguments shoud be even number \n");
       return E_CCT_IMGTOOL_BAD_ARG;
    }

    UINT32 u4Index = 0;
    UINT32 u4Count = 0;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row+=2) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col+=2) {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth)  + (a_strROI.u4StartX + col) ;

            i8ChannelSum[0] += pu2ImgBuf[u4Index];
            i8ChannelSum[1] += pu2ImgBuf[u4Index + 1];
            i8ChannelSum[2] += pu2ImgBuf[u4Index + a_u4ImgWidth];
            i8ChannelSum[3] += pu2ImgBuf[u4Index + a_u4ImgWidth + 1];
            u4Count++;

            if (pu2ImgBuf[u4Index] < 1024)
            {
                u4Histogram[pu2ImgBuf[u4Index]]++;
            }

            if (pu2ImgBuf[u4Index+1] < 1024)
            {
                u4Histogram[pu2ImgBuf[u4Index+1]]++;
            }

            if (pu2ImgBuf[u4Index+a_u4ImgWidth] < 1024)
            {
                u4Histogram[pu2ImgBuf[u4Index+a_u4ImgWidth]]++;
            }

            if (pu2ImgBuf[u4Index+a_u4ImgWidth+1] < 1024)
            {
                u4Histogram[pu2ImgBuf[u4Index+a_u4ImgWidth+1]]++;
            }
        }
    }

    // Avoid to divide by zero
    if (u4Count == 0) {
        u4Count = 1;
    }

    // Calculate channel avg
    dChannelAvg[0] = ((DOUBLE)i8ChannelSum[0]) / u4Count;
    dChannelAvg[1] = ((DOUBLE)i8ChannelSum[1]) / u4Count;
    dChannelAvg[2] = ((DOUBLE)i8ChannelSum[2]) / u4Count;
    dChannelAvg[3] = ((DOUBLE)i8ChannelSum[3]) / u4Count;

    switch (a_eColorOrder)  {
        case RawPxlOrder_R:     //R, Gr, Gb,B
            a_rRAWAnalysisResult.fRAvg = (FLOAT)dChannelAvg[0];
            a_rRAWAnalysisResult.fGrAvg = (FLOAT)dChannelAvg[1];
            a_rRAWAnalysisResult.fGbAvg = (FLOAT)dChannelAvg[2];
            a_rRAWAnalysisResult.fBAvg = (FLOAT)dChannelAvg[3];
            break;
        case RawPxlOrder_Gb:     //Gb, B, R, Gr
            a_rRAWAnalysisResult.fRAvg = (FLOAT)dChannelAvg[2];
            a_rRAWAnalysisResult.fGrAvg = (FLOAT)dChannelAvg[3];
            a_rRAWAnalysisResult.fGbAvg = (FLOAT)dChannelAvg[0];
            a_rRAWAnalysisResult.fBAvg = (FLOAT)dChannelAvg[1];
            break;
        case RawPxlOrderr_Gr:    //Gr, R, B, Gb
            a_rRAWAnalysisResult.fRAvg = (FLOAT)dChannelAvg[1];
            a_rRAWAnalysisResult.fGrAvg = (FLOAT)dChannelAvg[0];
            a_rRAWAnalysisResult.fGbAvg = (FLOAT)dChannelAvg[3];
            a_rRAWAnalysisResult.fBAvg = (FLOAT)dChannelAvg[2];
            break;
        case RawPxlOrder_B:     //B, Gb, Gr, R
            a_rRAWAnalysisResult.fRAvg = (FLOAT)dChannelAvg[3];
            a_rRAWAnalysisResult.fGrAvg = (FLOAT)dChannelAvg[2];
            a_rRAWAnalysisResult.fGbAvg = (FLOAT)dChannelAvg[1];
            a_rRAWAnalysisResult.fBAvg = (FLOAT)dChannelAvg[0];
            break;
        default:
            a_rRAWAnalysisResult.fRAvg = (FLOAT)dChannelAvg[0];
            a_rRAWAnalysisResult.fGrAvg = (FLOAT)dChannelAvg[1];
            a_rRAWAnalysisResult.fGbAvg = (FLOAT)dChannelAvg[2];
            a_rRAWAnalysisResult.fBAvg = (FLOAT)dChannelAvg[3];
            break;
    }

    // calculate median
    UINT32 u4PixelCountSum = 0;
    UINT32 u4HalfPixelCount = u4Count * 2;

    a_rRAWAnalysisResult.u4Median = 1023;

    for (INT32 i=0; i<1024; i++) {
        u4PixelCountSum += u4Histogram[i];

        if (u4PixelCountSum >= u4HalfPixelCount)  {
            a_rRAWAnalysisResult.u4Median = i;
            break;
        }
    }

    return mrRet;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrAnalyzeYInfo () -
//!  brief Analyze Y Info  (Include Average and standard deviation)
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrAnalyzeYInfo(MUINT8 *a_puImgBuf, UINT32 a_u4ImgWidth, UINT32 a_u4ImgHeight, const ROIRect &a_strROI, UINT32 &a_pu4Avg, UINT32 &a_pu4Std)
{
    UINT64 u8Sum = 0;
    UINT64 u8SquareSum = 0;

    MRESULT mrRet = S_CCT_IMGTOOL_OK;

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth) {
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    UINT32 u4Index = 0;
    UINT32 u4Count = 0;
    MUINT8 *puImgBug = a_puImgBuf;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row++) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col++)  {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth)  + (a_strROI.u4StartX + col) ;

            u8Sum += (UINT64)puImgBug[u4Index];
            u4Count++;
        }
    }

    //avoid to divide by zero
    if (u4Count == 0) {
        u4Count = 1;
    }

    //calc the average;
    a_pu4Avg = (UINT32)(u8Sum / u4Count);

    //calc the standard deviation
    u4Count = 0;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row++) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col++)  {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth) + (a_strROI.u4StartX + col);
            u8SquareSum+= (((UINT64)puImgBug[u4Index] - (UINT64)a_pu4Avg) * ((UINT64)puImgBug[u4Index] - (UINT64)a_pu4Avg));
            u4Count++;
        }
    }

    //avoid divide by zero
    if (u4Count == 0) {
        u4Count = 2;
    }
    a_pu4Std = (UINT16)(pow((DOUBLE)(u8SquareSum / (DOUBLE)(u4Count -1)), 0.5) * 100);

    return mrRet;
}



/////////////////////////////////////////////////////////////////////////
//
//   mrCalcMedian () -
//!  brief Calcuate the Meidian Value
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkImgTool::mrCalcMedian(MUINT8 *a_pUnPackedImgBuf,
                                                                UINT32 a_u4ImgWidth,
                                                                UINT32 a_u4ImgHeight,
                                                                eRAW_ColorOrder a_eColorOrder,
                                                                const ROIRect &a_strROI,
                                                                RAWAnalyzeResult &a_pStrRawInfoResult)
{
    UINT16 *pu2ImgBuf;

    UINT32 u4TempCh[4] = {0, 0, 0, 0};

    UINT32 *pu4HistogramCh1 = NULL;
    UINT32 *pu4HistogramCh2 = NULL;
    UINT32 *pu4HistogramCh3 = NULL;
    UINT32 *pu4HistogramCh4 = NULL;

    MRESULT mrRet = S_CCT_IMGTOOL_OK;

    pu2ImgBuf=(UINT16*)a_pUnPackedImgBuf;

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth){
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return E_CCT_IMGTOOL_BAD_ARG;
    }

    //make sure  ROI in 2x boundary
    if ( (a_strROI.u4StartX | a_strROI.u4ROIWidth | a_strROI.u4StartY |a_strROI.u4ROIHeight ) & (UINT32)0x01) {
       ACDK_LOGE(" The ROI arguments shoud be even number \n");
       return E_CCT_IMGTOOL_BAD_ARG;
    }

    pu4HistogramCh1 = (UINT32 *) calloc(16384, sizeof(UINT32));
    if (pu4HistogramCh1 == NULL) {
        ACDK_LOGE(" Can't allocate memory for histogram CH1\n");
        return E_CCT_IMGTOOL_MEMORY_MAX;

    }
    pu4HistogramCh2 = (UINT32 *) calloc(16384, sizeof(UINT32));
    if (pu4HistogramCh2 == NULL)  {
        free(pu4HistogramCh2);
        ACDK_LOGE(" Can't allocate memory for histogram CH2\n");
        return E_CCT_IMGTOOL_MEMORY_MAX;
    }
    pu4HistogramCh3 = (UINT32 *) calloc(16384, sizeof(UINT32));
    if (pu4HistogramCh3 == NULL) {
        free(pu4HistogramCh1);
        free(pu4HistogramCh2);
        ACDK_LOGE(" Can't allocate memory for histogram CH3\n");
        return E_CCT_IMGTOOL_MEMORY_MAX;
    }
    pu4HistogramCh4 = (UINT32 *) calloc(16384, sizeof(UINT32));
    if (pu4HistogramCh4 == NULL)  {
        free(pu4HistogramCh1);
        free(pu4HistogramCh2);
        free(pu4HistogramCh3);

        ACDK_LOGE(" Can't allocate memory for histogram CH4\n");
        return E_CCT_IMGTOOL_MEMORY_MAX;
    }

    UINT32 u4Index = 0;
    UINT32 u4Count = 0;
    for (UINT32 row = 0 ; row < a_strROI.u4ROIHeight; row+=2) {
        for (UINT32 col = 0; col < a_strROI.u4ROIWidth; col+=2) {
            u4Index = ((row + a_strROI.u4StartY) * a_u4ImgWidth)  + (a_strROI.u4StartX + col) ;

            pu4HistogramCh1[pu2ImgBuf[u4Index]]++;
            pu4HistogramCh2[pu2ImgBuf[u4Index + 1]]++;
            pu4HistogramCh3[pu2ImgBuf[u4Index + a_u4ImgWidth ]]++;
            pu4HistogramCh4[pu2ImgBuf[u4Index + a_u4ImgWidth + 1]]++;
            u4Count++;
        }
    }

    UINT32 u4Sum = 0;
    //calc CH1 median
    for (UINT32 i = 0; i < 16384; i++) {
        u4Sum += pu4HistogramCh1[i];
        if (u4Sum > (u4Count / 2)) {
            u4TempCh[0] = i;
            break;
        }
    }

    //calc CH2 median
    u4Sum = 0;
    for (UINT32 i = 0; i < 16384; i++) {
        u4Sum += pu4HistogramCh2[i];
        if (u4Sum > (u4Count / 2)) {
            u4TempCh[1] = i;
            break;
        }
    }

    //calc CH3 median
    u4Sum = 0;
    for (UINT32 i = 0; i < 16384; i++) {
        u4Sum += pu4HistogramCh3[i];
        if (u4Sum > (u4Count / 2)) {
            u4TempCh[2] = i;
            break;
        }
    }

    //calc CH4 median
    u4Sum = 0;
    for (UINT32 i = 0; i < 16384; i++) {
        u4Sum += pu4HistogramCh4[i];
        if (u4Sum > (u4Count / 2)) {
            u4TempCh[3] = i;
            break;
        }
    }

    switch (a_eColorOrder) {
        case RawPxlOrder_R:     //R, Gr, Gb,B
            a_pStrRawInfoResult.u4RAvg = (UINT16)u4TempCh[0];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)u4TempCh[1];
            a_pStrRawInfoResult.u4BAvg = (UINT16)u4TempCh[3];
            a_pStrRawInfoResult.u4GbAvg =  (UINT16)u4TempCh[2];
            break;
        case RawPxlOrder_Gb:     //Gb, B, R, Gr
            a_pStrRawInfoResult.u4RAvg = (UINT16)u4TempCh[2];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)u4TempCh[3];
            a_pStrRawInfoResult.u4BAvg = (UINT16)u4TempCh[1];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)u4TempCh[0];
            break;
        case RawPxlOrderr_Gr:    //Gr, R, B, Gb
            a_pStrRawInfoResult.u4RAvg = (UINT16)u4TempCh[1];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)u4TempCh[0];
            a_pStrRawInfoResult.u4BAvg = (UINT16)u4TempCh[2];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)u4TempCh[3];
            break;
        case RawPxlOrder_B:     //B, Gb, Gr, R
            a_pStrRawInfoResult.u4RAvg = (UINT16)u4TempCh[3];
            a_pStrRawInfoResult.u4GrAvg = (UINT16)u4TempCh[2];
            a_pStrRawInfoResult.u4BAvg = (UINT16)u4TempCh[0];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)u4TempCh[1];
            break;
        default:
            a_pStrRawInfoResult.u4RAvg = (UINT16)u4TempCh[0];
            a_pStrRawInfoResult.u4GrAvg =(UINT16)u4TempCh[1];
            a_pStrRawInfoResult.u4BAvg = (UINT16)u4TempCh[3];
            a_pStrRawInfoResult.u4GbAvg = (UINT16)u4TempCh[2];
            break;
    }

    free(pu4HistogramCh1);
    free(pu4HistogramCh2);
    free(pu4HistogramCh3);
    free(pu4HistogramCh4);

    return mrRet;
}



/////////////////////////////////////////////////////////////////////////
//
//   vExtractPackedRawROI () -
//!  brief Extract the packed raw image ROI
//!
/////////////////////////////////////////////////////////////////////////
VOID AcdkImgTool::vExtractPackedRawROI(MUINT8 *a_pSrcPackedImgBuf, MUINT8 *a_pDestPackedImgBuf, UINT32 a_u4ImgWidth, UINT32 a_u4ImgHeight, UINT32 a_u4SrcBytesPerLine, UINT32 a_u4DestBytesPerLine, const ROIRect &a_strROI)
{
    if ((a_pSrcPackedImgBuf== NULL) || (a_pDestPackedImgBuf== NULL)) {
        return;
    }

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth) {
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return;
    }

    //make sure  ROI in 2x boundary
    if (a_strROI.u4StartX  & (UINT32)0x3) {
       ACDK_LOGE(" The ROI startx should be 4x number \n");
       return;
    }

    if ((a_strROI.u4ROIWidth | a_strROI.u4ROIHeight | a_strROI.u4StartY) & (UINT32)0x1) {
       ACDK_LOGE(" The ROI startY, width, height shoud be even number \n");
       return;
    }

    UINT8* pu2SrcPackedRaw = (UINT8 *) (a_pSrcPackedImgBuf);
    UINT8* pu2DestPackedRaw = (UINT8 *) (a_pDestPackedImgBuf);

    for(UINT16 row = 0; row < a_strROI.u4ROIHeight; row++ ) {
        UINT8* uSrcAddr = pu2SrcPackedRaw + ((a_strROI.u4StartX * 12)/ 8) + ((row + a_strROI.u4StartY) * a_u4SrcBytesPerLine);
        UINT8* uDestAddr = pu2DestPackedRaw + (row * a_u4DestBytesPerLine);
        memcpy((MUINT8*)uDestAddr, (MUINT8*)uSrcAddr, a_u4DestBytesPerLine);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////
//
//   vExtractUnPackedRawROI () -
//!  brief Extract the unpacked raw image ROI
//!
/////////////////////////////////////////////////////////////////////////
VOID AcdkImgTool::vExtractUnPackedRawROI(MUINT8 *a_pSrcUnPackedImgBuf, MUINT8 *a_pDestUnPackedImgBuf, UINT32 a_u4ImgWidth, UINT32 a_u4ImgHeight, const ROIRect &a_strROI)
{
    if ((a_pSrcUnPackedImgBuf== NULL) || (a_pDestUnPackedImgBuf== NULL)) {
        return;
    }

    if ((a_strROI.u4StartX + a_strROI.u4ROIWidth) > a_u4ImgWidth) {
        ACDK_LOGE(" The startX + ROI Width out of Range\n");
        return;
    }

    if ((a_strROI.u4StartY + a_strROI.u4ROIHeight) > a_u4ImgHeight) {
        ACDK_LOGE(" The startY + ROI Height out of Range\n");
        return;
    }

    if ((a_strROI.u4StartX | a_strROI.u4ROIWidth | a_strROI.u4ROIHeight | a_strROI.u4StartY) & (UINT32)0x1) {
       ACDK_LOGE(" The ROI start X, startY, width, height shoud be even number \n");
       return;
    }

    UINT8* puSrcUnPackedRaw = (UINT8 *) (a_pSrcUnPackedImgBuf);
    UINT8* puDestUnPackedRaw = (UINT8 *) (a_pDestUnPackedImgBuf);

    for(UINT16 row = 0; row < a_strROI.u4ROIHeight; row++ ) {
        UINT8* uSrcAddr = puSrcUnPackedRaw + (a_strROI.u4StartX * 2) + ((row + a_strROI.u4StartY) * a_u4ImgWidth * 2);
        UINT8* uDestAddr = puDestUnPackedRaw + (row * a_strROI.u4ROIWidth * 2);
        memcpy((MUINT8*)uDestAddr, (MUINT8*)uSrcAddr, a_strROI.u4ROIWidth * 2);
    }
    return;
}



