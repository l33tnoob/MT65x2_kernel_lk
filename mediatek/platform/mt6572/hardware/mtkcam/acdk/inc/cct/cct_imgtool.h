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
// AcdkImgTool.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkImgTool.h
//! \brief

#ifndef _ACDKIMGTOOL_H_
#define _ACDKIMGTOOL_H_

#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkBase.h"
#include "cct_feature.h"


using namespace NSACDK;

///////////////////////////////////////////////////////////////////////////////
//
//! \brief Structure for RAW analysis result
//
///////////////////////////////////////////////////////////////////////////////
typedef struct RAWAnalyzeResult_t
{
    //average value
    MUINT32 u4RAvg;
    MUINT32 u4GrAvg;
    MUINT32 u4GbAvg;
    MUINT32 u4BAvg;

    //standard deviation
    MUINT32 u4RStd;
    MUINT32 u4GrStd;
    MUINT32 u4GbStd;
    MUINT32 u4BStd;
}RAWAnalyzeResult;

class AcdkImgTool/*:public AcdkBase*/
{
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//1   Public
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
public:

    /////////////////////////////////////////////////////////////////////////
    //
    //   AcdkImgTool () -
    //!
    //!  brief ManuImgTool module constructor
    //
    /////////////////////////////////////////////////////////////////////////
	AcdkImgTool();

    /////////////////////////////////////////////////////////////////////////
    //
    //   ~AcdkImgTool () -
    //!
    //!  brief ManuImgTool module destructor
    //
    /////////////////////////////////////////////////////////////////////////
	virtual ~AcdkImgTool();

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrInitManuImgTool () -
    //!  brief init the manufacture image tool
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrInitAcdkImgTool();

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrEnableManuImgTool  () -
    //!  brief eanble the manufacture image tool
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrEnableAcdkImgTool();

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrDisableImgTool () -
    //!  brief disable the manufacture image tool
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrDisableAcdkImgTool();

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrUnPackRawImg () -
    //!  brief unpack RAW image
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual VOID vUnPackRawImg(MUINT8 *a_pPackedImgBuf, MUINT8 *a_pUnPackedImgBuf, MUINT32 a_u4ImgSize, MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT8 a_bitdepth);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrAnalyzeRAWInfo () -
    //!  brief Analyze RAW Info  (Include Average and standard deviation)
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrAnalyzeRAWInfo(MUINT8 *a_pUnPackedImgBuf, MUINT32 a_u4ImgWidth, MUINT32 a_u4ImgHeight, eRAW_ColorOrder a_eColorOrder,  const ROIRect &a_strROI, RAWAnalyzeResult &a_pStrRawInfoResult);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrAnalyzeRAWImage () -
    //!  brief Analyze RAW image (average and median)
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrAnalyzeRAWImage(MUINT8 *a_pUnPackedImgBuf,
                                     MUINT32 a_u4ImgWidth,
                                     MUINT32 a_u4ImgHeight,
                                     eRAW_ColorOrder a_eColorOrder,
                                     const ROIRect &a_strROI,
                                      ACDK_CDVT_RAW_ANALYSIS_RESULT_T &a_rRAWAnalysisResult);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrAnalyzeYInfo () -
    //!  brief Analyze Y Info  (Include Average and standard deviation)
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrAnalyzeYInfo(MUINT8 *puImgBuf, MUINT32 a_u4ImgWidth, MUINT32 a_u4ImgHeight, const ROIRect &a_strROI, MUINT32 &a_pu4Avg, MUINT32 &a_pu4Std);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalcMedian () -
    //!  brief Calcuate the Meidian Value
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mrCalcMedian(MUINT8 *a_pUnPackedImgBuf, MUINT32 a_u4ImgWidth, MUINT32 a_u4ImgHeight, eRAW_ColorOrder a_eColorOrder, const ROIRect &a_strROI, RAWAnalyzeResult &a_pStrRawInfoResult);


    /////////////////////////////////////////////////////////////////////////
    //
    //   vExtractPackedRawROI () -
    //!  brief Extract the packed raw image ROI
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual VOID vExtractPackedRawROI(MUINT8 *a_pSrcPackedImgBuf, MUINT8 *a_pDestPackedImgBuf, MUINT32 a_u4ImgWidth, MUINT32 a_u4ImgHeight, MUINT32 a_u4BytesPerLine,  MUINT32 a_u4DestBytesPerLine, const ROIRect &a_strROI);

    /////////////////////////////////////////////////////////////////////////
    //
    //   vExtractUnPackedRawROI () -
    //!  brief Extract the unpacked raw image ROI
    //!
    /////////////////////////////////////////////////////////////////////////
    virtual VOID vExtractUnPackedRawROI(MUINT8 *a_pSrcUnPackedImgBuf, MUINT8 *a_pDestUnPackedImgBuf, MUINT32 a_u4ImgWidth, MUINT32 a_u4ImgHeight, const ROIRect &a_strROI);


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//1   Private
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
private:
    MBOOL m_bAcdkImgTool;

};

#endif

