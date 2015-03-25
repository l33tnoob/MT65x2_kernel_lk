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
// AcdkCalibration.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCalibration.cpp
//! \brief
#define LOG_TAG "AcdkCalibration"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

extern "C" {
#include <linux/fb.h>
#include <linux/mtkfb.h>
}

#include "AcdkLog.h"

#include "cct_ctrl.h"
#include "cct_calibration.h"
#include "cct_ErrCode.h"
#include <mtkcam/acdk/cct_feature.h>
#include "cct_if.h"
#include <mtkcam/drv/isp_drv.h>
#include "cct_main.h"
#include <mtkcam/common.h>
using namespace NSCam;
#include <dbg_aaa_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "ae_mgr.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "lsc_mgr.h"
#include "tsf_tuning_custom.h"

#include "./ParamLSCInternal.h"
#include "./ShadingATNTable.h"
#include <mtkcam/algorithm/liblsctrans/ShadingTblTransform.h>

//#include "isp_hal.h"

using namespace NSACDK;


#define MEDIA_PATH "//data"

#define OB_SLOPEx1_UNIT         (512)
#define MS_SLEEP(x)             (usleep(x*1000))

static MUINT8 g_SAVE_IMG = 0;

static AcdkCalibration* g_pAcdkCalibrationObj = NULL;
ACDK_CDVT_RAW_ANALYSIS_RESULT_T g_RAWAnalysisResult;

/*******************************************************************************
*
********************************************************************************/
VOID vCapCb(VOID *a_pParam)
{
    if (g_pAcdkCalibrationObj != NULL)
    {
        g_pAcdkCalibrationObj->vCaptureCallBack(a_pParam);
    }

    return;
}

/*******************************************************************************
*
********************************************************************************/
VOID AcdkCalibration:: vCaptureCallBack(VOID *a_pParam)
{

    if (m_pAcdkImgToolObj == NULL)
    {
        ACDK_LOGE("Null AcdkImgTool Obj \n");
        return;
    }

    ImageBufInfo *prImgBufInfo = (ImageBufInfo*)a_pParam;

    if ((prImgBufInfo->eImgType >= PURE_RAW8_TYPE) && (prImgBufInfo->eImgType <= PROCESSED_RAW10_TYPE))    //webber check later
    {
        ACDK_LOGD("[vCaptureCallBack] Raw Type = %d\n", prImgBufInfo->eImgType);

        if (m_pucRawBuf != NULL)
        {
            free(m_pucRawBuf);
        }
        m_u2RawImgWidth = prImgBufInfo->RAWImgBufInfo.imgWidth;
        m_u2RawImgHeight = prImgBufInfo->RAWImgBufInfo.imgHeight;
        MUINT32 u2ImgSize = prImgBufInfo->RAWImgBufInfo.imgSize;
        MUINT8 *puImgBuf = prImgBufInfo->RAWImgBufInfo.bufAddr;
        m_eColorOrder = prImgBufInfo->RAWImgBufInfo.eColorOrder;
        m_pucRawBuf = (MUINT8 *) malloc (m_u2RawImgWidth * m_u2RawImgHeight * 2 * sizeof(MUINT8));


        if ((prImgBufInfo->eImgType == PURE_RAW10_TYPE) || (prImgBufInfo->eImgType == PROCESSED_RAW10_TYPE))
        {
            ACDK_LOGD("[vCaptureCallBack] Packed Mode\n");
            m_pAcdkImgToolObj->vUnPackRawImg(puImgBuf,m_pucRawBuf, u2ImgSize, m_u2RawImgWidth, m_u2RawImgHeight, 10);
        }
        else
        {
            ACDK_LOGD("[vCaptureCallBack] unPacked Mode\n");
            m_pAcdkImgToolObj->vUnPackRawImg(puImgBuf,m_pucRawBuf, u2ImgSize, m_u2RawImgWidth, m_u2RawImgHeight, 8);
        }

        //! If the mode is preview mode
        //! Simulate to downsample the RAW to preview mode
        if (m_u4CapMode == 0) //preview mode
        {
            UINT16 u2PrvRawImgWidth = (m_u2RawImgWidth / 2 ) & (~0x1);       //TODO the ratio should according real preview/capture ratio
            UINT16 u2PrvRawImgHeight = (m_u2RawImgHeight / 2 ) & (~0x1);

            ACDK_LOGD("u2PrvRawImgWidth:%d\n", u2PrvRawImgWidth);
            ACDK_LOGD("u2PrvRawImgHeight:%d\n", u2PrvRawImgHeight);
            UINT16 *pu2SrcImgBuf = (UINT16 *)m_pucRawBuf;
            UINT16 *pu2DestImgBuf = (UINT16 *)m_pucRawBuf;

            for (UINT32 y = 0;  y < u2PrvRawImgHeight/2; y++)
            {
                for (UINT32 x  = 0;  x < u2PrvRawImgWidth/2; x++)
                {
                    pu2DestImgBuf[ (2 * y) * u2PrvRawImgWidth + (2 * x)]  = pu2SrcImgBuf[ (y * 4 ) * m_u2RawImgWidth + (x * 4)];
                    pu2DestImgBuf[ (2 * y) * u2PrvRawImgWidth + (2 * x + 1)] = pu2SrcImgBuf[ (y * 4 ) * m_u2RawImgWidth + (x * 4 + 1)];
                    pu2DestImgBuf[ (2 * y + 1) * u2PrvRawImgWidth + (2 * x)]  = pu2SrcImgBuf[ (y * 4 + 1) * m_u2RawImgWidth + (x * 4)];
                    pu2DestImgBuf[ (2 * y + 1) * u2PrvRawImgWidth + (2 * x + 1)] = pu2SrcImgBuf[ (y * 4 + 1 ) * m_u2RawImgWidth + (x * 4 + 1)];
                }
            }
            m_u2RawImgWidth = u2PrvRawImgWidth;
            m_u2RawImgHeight = u2PrvRawImgHeight;
        }

        //if the raw format need ONE_BYTE_RAW
        if (m_eRawFormat == ONE_BYTE_RAW)
        {
            MUINT8 *puImgBuf = (MUINT8*)m_pucRawBuf;
            UINT16 *pu2ImgBuf = (UINT16 *)m_pucRawBuf;
            for (MINT32 i = 0 ; i < m_u2RawImgWidth * m_u2RawImgHeight; i++)
            {
                *puImgBuf++ = (*pu2ImgBuf++) >> (prImgBufInfo->RAWImgBufInfo.bitDepth - 8);
            }
        }

        {
            char filename[128];
            sprintf(filename, "/sdcard/Raw_%dx%d.raw", m_u2RawImgWidth, m_u2RawImgHeight);
            MY_LOG("[LscMgr:%s] DBG: Output %s", __FUNCTION__, filename);
            FILE* fpdebug = fopen(filename,"wb");
            if ( fpdebug == NULL )
            {
                MY_ERR("Can't open :%s\n",filename);
            } else {
                fwrite(m_pucRawBuf,m_u2RawImgWidth*m_u2RawImgHeight*2, 1,fpdebug);
                fclose(fpdebug);
            }
        }
    }

    m_bCapDone = MTRUE;

    ACDK_LOGD("[vCaptureCallBack] End\n");

}

/////////////////////////////////////////////////////////////////////////
//
//   AcdkCalibration () -
//!  @brief AcdkCalibration Constructor
//!
/////////////////////////////////////////////////////////////////////////
AcdkCalibration::AcdkCalibration()
                        :m_bAcdkCalibration(FALSE)
                        ,m_u4CapMode(0)
                        ,m_pAcdkBaseObj(0)
                        , m_u2RawImgWidth(0)
                        , m_u2RawImgHeight(0)
                        ,m_pucRawBuf(0)
                        ,m_eRawFormat(TWO_BYTE_RAW)
{

    ACDK_LOGD(" AcdkCalibration Constructor\n");
    m_pAcdkImgToolObj = new AcdkImgTool();

    g_pAcdkCalibrationObj = this;
}

/////////////////////////////////////////////////////////////////////////
//
//   ~AcdkCalibration () -
//!  @brief AcdkCalibration Disonstructor
//!
/////////////////////////////////////////////////////////////////////////
AcdkCalibration::~AcdkCalibration()
{
    ACDK_LOGD(" AcdkCalibration Disconstructor\n");
    delete m_pAcdkImgToolObj;
    m_pAcdkBaseObj = NULL;
    m_pucRawBuf = NULL;
    //m_pAcdkCLICmds = NULL;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrInitAcdkImgTool () -
//!  @brief init the tuning tool module
//!  @param a_pNuCameraObj: The input Camera obj
//!  @param a_pAcdkImgToolObj: The input AcdkImgTool Obj
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::init(AcdkBase *a_pAcdkBaseObj)
{
    ACDK_LOGD(" Init AcdkCalibration Tool\n");

    m_pAcdkBaseObj = a_pAcdkBaseObj;

    if (0 == m_pAcdkBaseObj) {
        ACDK_LOGD(" [init] Null AcdkCCTCtrl Obj \n");
        m_bAcdkCalibration = FALSE;
        return E_CCT_CALIBRATION_NULL_OBJ;
    }
    m_bAcdkCalibration = TRUE;
    return S_CCT_CALIBRATION_OK;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrInitAcdkImgTool () -
//!  @brief init the tuning tool module
//!  @param a_pNuCameraObj: The input Camera obj
//!  @param a_pAcdkImgToolObj: The input AcdkImgTool Obj
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::uninit()
{
    ACDK_LOGD(" unInit AcdkCalibration Tool\n");

    m_bAcdkCalibration = FALSE;
    return S_CCT_CALIBRATION_OK;
}



/////////////////////////////////////////////////////////////////////////
//
//   takePicture () -
//!  Take Picture & Back to Preview
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::takePicture(MUINT32 a_i4PreCap, eACDK_CAP_FORMAT type)
{
    MRESULT mrRet;
    MUINT32 u4RetLen;

    m_bCapDone = MFALSE;
    mrRet = m_pAcdkBaseObj->takePicture(a_i4PreCap, (MUINT32)type, vCapCb);

    while(!m_bCapDone)
    {
        ACDK_LOGD(" Waiting Capture Done...\n");
    }

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    m_pAcdkBaseObj->startPreview(NULL);

    MS_SLEEP(1000);

    if (FAILED(mrRet) ||  m_pucRawBuf == NULL)
    {
        return E_CCT_CALIBRATION_API_FAIL;
    }

    return S_CCT_CALIBRATION_OK;

}




/////////////////////////////////////////////////////////////////////////
//
//   mrAnalyzeRAWImage () -
//!  brief analyze RAW data
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrAnalyzeRAWImage()
{

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Pointer Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null Acdk Obj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }

    MUINT32 u4RetLen;
    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    ROIRect rROI;

    /*
    *   ROI Parameters Should be Even
    */
    rROI.u4ROIWidth = (m_u2RawImgWidth / 5) &(~(UINT32)0x01);
    rROI.u4ROIHeight = (m_u2RawImgHeight / 5) &(~(UINT32)0x01) ;
    rROI.u4StartX = ((m_u2RawImgWidth - rROI.u4ROIWidth) >> 1) & (~(UINT32)0x01);
    rROI.u4StartY = ((m_u2RawImgHeight - rROI.u4ROIHeight) >> 1) & (~(UINT32)0x01);

    mrRet = m_pAcdkImgToolObj->mrAnalyzeRAWImage(m_pucRawBuf,
            	                                    m_u2RawImgWidth,
            	                                    m_u2RawImgHeight,
            	                                    m_eColorOrder,
            	                                    rROI,
            	                                     g_RAWAnalysisResult);
    if (FAILED(mrRet))
    {
        ACDK_LOGE(" m_pAcdkImgToolObj->mrAnalyzeRAWImage() error \n");
        mrRet = E_CCT_CALIBRATION_API_FAIL;
    }


    return S_CCT_CALIBRATION_OK;


}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalculateSlope () -
//!  brief calculate slope
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrCalculateSlope(DOUBLE a_dX0,
                                          DOUBLE a_dY0,
                                          DOUBLE a_dX1,
                                          DOUBLE a_dY1,
                                          DOUBLE a_dX2,
                                          DOUBLE a_dY2,
                                          DOUBLE a_dX3,
                                          DOUBLE a_dY3,
                                          DOUBLE &a_dSlope)
{
    DOUBLE dTemp0 = 4 * (a_dX0 * a_dY0 + a_dX1 * a_dY1 + a_dX2 * a_dY2 + a_dX3 * a_dY3);
    DOUBLE dTemp1 = (a_dX0 + a_dX1 + a_dX2 + a_dX3) * (a_dY0 + a_dY1 + a_dY2 + a_dY3);
    DOUBLE dTemp2 = 4 * (a_dX0 * a_dX0 + a_dX1 * a_dX1 + a_dX2 * a_dX2 + a_dX3 * a_dX3);
    DOUBLE dTemp3 = (a_dX0 + a_dX1 + a_dX2 + a_dX3) * (a_dX0 + a_dX1 + a_dX2 + a_dX3);

    a_dSlope = (dTemp0 - dTemp1) / (dTemp2 - dTemp3);

    return S_CCT_CALIBRATION_OK;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrExpLinearity () -
//!  @brief exposure time linearity test
//!  @param a_i4Gain: snsor gain (base = 1024)
//!  @param a_i4ExpMode: [0] exposure line [1] exposure time in us
//!  @param a_i4ExpStart: the starting exposure line or exposure time
//!  @param a_i4ExpEnd: the ending exposure line or exposure time
//!  @param a_i4ExpInterval: The exposure interval in exposure line or exposure time
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrExpLinearity(MINT32 a_i4Gain,
                                        MINT32 a_i4ExpMode,
                                        MINT32 a_i4ExpStart,
                                        MINT32 a_i4ExpEnd,
                                        MINT32 a_i4ExpInterval,
                                        MINT32 a_i4PreCap,
                                        ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{



    ACDK_LOGD("Enter AcdkCalibration::mrExpLinearity()\n");
	ACDK_LOGD("a_i4Gain = %d\n", a_i4Gain);
	ACDK_LOGD("a_i4ExpMode = %d\n", a_i4ExpMode);
	ACDK_LOGD("a_i4ExpStart = %d\n", a_i4ExpStart);
	ACDK_LOGD("a_i4ExpEnd = %d\n", a_i4ExpEnd);
	ACDK_LOGD("a_i4ExpInterval = %d\n", a_i4ExpInterval);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	a_prSensorTestOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);


    for (MINT32 i4Exp = a_i4ExpStart; i4Exp <= a_i4ExpEnd; i4Exp += a_i4ExpInterval)
    {

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        //Temp. Mark
        //ae_cfg.u4ExposureMode = a_i4ExpMode;
        ae_cfg.u4Eposuretime = i4Exp;
        ae_cfg.u4AfeGain = a_i4Gain;
		//ae_cfg.u4IspGain = a_i4Gain;

        ACDK_LOGD("[u4Eposuretime] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[u4Eposuretime] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[u4Eposuretime] u4IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Take Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);
        i4SequenceNo++;

    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

    //Temp. Mark
    /*if(capInfo.u4ExposureMode) {    //Restore to expo. time base
        capInfo.u4ExposureMode = 0;
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                            (UINT8 *)&capInfo,
                                                            sizeof(capInfo),
                                                            (UINT8 *)&capInfo,
                                                            sizeof(capInfo),
                                                            &u4RetLen);
    }*/

    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrExpLinearity()\n");

   return mrRet;


}

/////////////////////////////////////////////////////////////////////////
//
//   mrAEPlineTableLinearity () -
//!  @brief exposure time linearity test
//!  @param a_i4Gain: snsor gain (base = 1024)
//!  @param a_i4ExpMode: [0] exposure line [1] exposure time in us
//!  @param a_i4ExpStart: the starting exposure line or exposure time
//!  @param a_i4ExpEnd: the ending exposure line or exposure time
//!  @param a_i4ExpInterval: The exposure interval in exposure line or exposure time
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrAEPlineTableLinearity(
                                        ACDK_CDVT_AE_PLINE_TEST_INPUT_T* in, 
                                        int inSize, 
                                        ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T* out, 
                                        int outSize, 
                                        MUINT32* realOutSize)
{
    ACDK_LOGD("Enter AcdkCalibration::mrAEPlineTableLinearity()\n");

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /* Check Function Available */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    MINT32 i4ShutterDelayFrame;
    MINT32 i4SensorGainDelayFrame;
    MINT32 i4ISPGainDelayFrame;
    MINT32 i4TestSteps;
    MINT32 i4RepeatTime;
    MINT32 i4IndexScanStart;
    MINT32 i4IndexScanEnd;    
    MINT32 i4Index;    
    MINT32 i4RepeatIndex;    
    MINT32 i4ShutterTime;
    MINT32 i4SensorGain;
    MINT32 i4ISPGain;
    MINT32 i4Yvalue;
    MBOOL bFrameUpdate;
    MINT32 i4WaitYvalueIndex;

    i4ShutterDelayFrame = in->rAEPlinetableInfo.i4ShutterDelayFrame;
    i4SensorGainDelayFrame = in->rAEPlinetableInfo.i4SensorGainDelayFrame;
    i4ISPGainDelayFrame = in->rAEPlinetableInfo.i4ISPGainDelayFrame;
    i4TestSteps = in->rAEPlinetableInfo.i4TestSteps;
    i4RepeatTime = in->rAEPlinetableInfo.i4RepeatTime;
    i4IndexScanStart = in->rAEPlinetableInfo.i4IndexScanStart;
    i4IndexScanEnd = in->rAEPlinetableInfo.i4IndexScanEnd;    
    ACDK_LOGD("i4ShutterDelayFrame = %d\n", i4ShutterDelayFrame);
    ACDK_LOGD("i4SensorGainDelayFrame = %d\n", i4SensorGainDelayFrame);
    ACDK_LOGD("i4ISPGainDelayFrame = %d\n", i4ISPGainDelayFrame);
    ACDK_LOGD("i4TestSteps = %d\n", i4TestSteps);
    ACDK_LOGD("i4RepeatTime = %d\n", i4RepeatTime);
    ACDK_LOGD("i4IndexScanStart = %d\n", i4IndexScanStart);
    ACDK_LOGD("i4IndexScanEnd = %d\n", i4IndexScanEnd);

    out->i4ErrorCode = 0; // No error    
    AeMgr::getInstance().updateSensorDelayInfo(&i4ShutterDelayFrame, &i4SensorGainDelayFrame, &i4ISPGainDelayFrame);

    // Set the sensor mode.
//    in->rAEPlinetableInfo.eSensorMode
    m_pAcdkBaseObj->startPreview(NULL);
    
    ACDK_LOGD("[Disable 3A]\n");
    m_pAcdkBaseObj->sendcommand(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    m_pAcdkBaseObj->sendcommand(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

    for (i4Index = i4IndexScanStart; i4Index <= i4IndexScanEnd; i4Index += i4TestSteps) {
        i4ShutterTime = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4ShutterTime;
        i4SensorGain = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4SensorGain;
        i4ISPGain = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4ISPGain;

        AeMgr::getInstance().getBrightnessValue(&bFrameUpdate, &i4Yvalue);        
        for (i4RepeatIndex = 0; i4RepeatIndex < i4RepeatTime; i4RepeatIndex++) {
            CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
            ACDK_LOGD("[Lock Exposure Setting]\n");
            CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);
            
            ae_cfg.u4Eposuretime = i4ShutterTime;
            ae_cfg.u4AfeGain = i4SensorGain;
            ae_cfg.u4IspGain = i4ISPGain;

            ACDK_LOGD("[mrAEPlineTableLinearity] Exp = %d\n", ae_cfg.u4Eposuretime);
            ACDK_LOGD("[mrAEPlineTableLinearity] AfeGain = %d\n", ae_cfg.u4AfeGain);
            ACDK_LOGD("[mrAEPlineTableLinearity] u4IspGain = %d\n", ae_cfg.u4IspGain);

            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);
            //Wait stable
           bFrameUpdate = MFALSE;
           i4WaitYvalueIndex = 0;
           while(i4WaitYvalueIndex <= i4ISPGainDelayFrame) {
                MS_SLEEP(10);    //delay for 10ms          
                AeMgr::getInstance().getBrightnessValue(&bFrameUpdate, &i4Yvalue);
                if(bFrameUpdate == MTRUE) {
                    i4WaitYvalueIndex++;
                }         
                ACDK_LOGD("[mrAEPlineTableLinearity] Wait frame update i4Yvalue:%d\n", i4Yvalue);                
            }

            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4Index = i4Index;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4ShutterTime = i4ShutterTime;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4SensorGain = i4SensorGain;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4ISPGain = i4ISPGain;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4Yvalue = i4Yvalue;
        }
    }

    out->i4TestCount = i4Index*i4RepeatTime;
    
Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    m_pAcdkBaseObj->sendcommand(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    m_pAcdkBaseObj->sendcommand(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrAEPlineTableLinearity()\n");

   return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrGainLinearity () -
//!  @brief sensor gain linearity test
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainStart: the starting senaor gain (base = 1024)
//!  @param a_i4GainEnd: the ending senaor gain (base = 1024)
//!  @param a_i4GainInterval: The test senaor gain interval (base = 1024)
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrGainLinearity(MINT32 a_i4ExpTime,
                                         MINT32 a_i4GainStart,
                                         MINT32 a_i4GainEnd,
                                         MINT32 a_i4GainInterval,
                                         MINT32 a_i4PreCap,
                                         ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    ACDK_LOGD("Enter AcdkCalibration::mrGainLinearity()\n");
	ACDK_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
	ACDK_LOGD("a_i4GainStart = %d\n", a_i4GainStart);
	ACDK_LOGD("a_i4GainEnd = %d\n", a_i4GainEnd);
	ACDK_LOGD("a_i4GainInterval = %d\n", a_i4GainInterval);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	a_prSensorTestOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    for (MINT32 i4Gain = a_i4GainStart; i4Gain <= a_i4GainEnd; i4Gain += a_i4GainInterval)
    {

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        ae_cfg.u4Eposuretime = a_i4ExpTime;
        ae_cfg.u4AfeGain = i4Gain;
		//ae_cfg.u4IspGain = i4Gain;

        ACDK_LOGD("[mrGainLinearity] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[mrGainLinearity] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[mrGainLinearity] u4IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Take Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);
        i4SequenceNo++;

    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrGainLinearity()\n");

   return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrGainTableLinearity () -
//!  @brief sensor gain linearity test (gain table)
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainTableSize: gain table dimension
//!  @param a_pi4GainTable: pointer to gain table
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrGainTableLinearity(MINT32 a_i4ExpTime,
                                              MINT32 a_i4GainTableSize,
                                              MINT32 *a_pi4GainTable,
                                              MINT32 a_i4PreCap,
                                              ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    ACDK_LOGD("Enter AcdkCalibration::mrGainTableLinearity()\n");
	ACDK_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
	ACDK_LOGD("a_i4GainTableSize = %d\n", a_i4GainTableSize);

	for (MINT32 i = 0; i < a_i4GainTableSize; i++)
    {
        ACDK_LOGD("GAIN_TABLE [%d] = %d\n", i, a_pi4GainTable[i]);
    }

	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);
	ACDK_LOGD("=====================\n");

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	a_prSensorTestOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    for (MINT32 i4GainTableIndex = 0; i4GainTableIndex < a_i4GainTableSize; i4GainTableIndex ++)
    {

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        ae_cfg.u4Eposuretime = a_i4ExpTime;
        ae_cfg.u4AfeGain = a_pi4GainTable[i4GainTableIndex];
		//ae_cfg.u4IspGain = a_pi4GainTable[i4GainTableIndex];

        ACDK_LOGD("[mrGainTableLinearity] Gain Index = %d\n", i4GainTableIndex);
        ACDK_LOGD("[mrGainTableLinearity] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[mrGainTableLinearity] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[mrGainTableLinearity] IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Take RAW Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);
        i4SequenceNo++;

    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrGainTableLinearity()\n");

   return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrOBStability () -
//!  @brief OB stability test
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainStart: the starting senaor gain (base = 1024)
//!  @param a_i4GainEnd: the ending senaor gain (base = 1024)
//!  @param a_i4GainInterval: The test senaor gain interval (base = 1024)
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrOBStability(MINT32 a_i4ExpTime,
                                       MINT32 a_i4GainStart,
                                       MINT32 a_i4GainEnd,
                                       MINT32 a_i4GainInterval,
                                       MINT32 a_i4PreCap,
                                       ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{
    ACDK_LOGD("Enter AcdkCalibration::mrOBStability()\n");
	ACDK_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
	ACDK_LOGD("a_i4GainStart = %d\n", a_i4GainStart);
	ACDK_LOGD("a_i4GainEnd = %d\n", a_i4GainEnd);
	ACDK_LOGD("a_i4GainInterval = %d\n", a_i4GainInterval);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 i4TestCnt, i4Gain;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	a_prSensorTestOutput->i4ErrorCode = 0; // No error

	if (a_i4GainStart == a_i4GainEnd)
    {
        i4TestCnt = a_i4GainInterval;
    }
    else
    {
        i4TestCnt = ((a_i4GainEnd - a_i4GainStart) + (a_i4GainInterval - 1))/ a_i4GainInterval;

        if (((a_i4GainEnd - a_i4GainStart) % a_i4GainInterval) == 0)
        {
            i4TestCnt++;
        }
    }

    ACDK_LOGD("[mrOBStability] Test Count = %d\n", i4TestCnt);

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    for (int i = 0; i < i4TestCnt; i ++)
    {

        (a_i4GainStart == a_i4GainEnd) ? (i4Gain = a_i4GainStart)
                                       : (i4Gain = a_i4GainStart + (i * a_i4GainInterval ));

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);
        ae_cfg.u4Eposuretime = a_i4ExpTime;
        ae_cfg.u4AfeGain = i4Gain;
		//ae_cfg.u4IspGain = i4Gain;

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Take RAW Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);
        i4SequenceNo++;

    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrOBStability()\n");

    return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrGainTableOBStability () -
//!  @brief OB stability test (gain table)
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainTableSize: gain table dimension
//!  @param a_pi4GainTable: pointer to gain table
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrGainTableOBStability(MINT32 a_i4ExpTime,
                                                MINT32 a_i4GainTableSize,
                                                MINT32 *a_pi4GainTable,
                                                MINT32 a_i4PreCap,
                                                ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    ACDK_LOGD("Enter AcdkCalibration::mrGainTableOBStability()\n");
	ACDK_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
	ACDK_LOGD("a_i4GainTableSize = %d\n", a_i4GainTableSize);
    ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    for (int i = 0; i < a_i4GainTableSize; i++)
    {
        ACDK_LOGD("GAIN_TABLE[%d] = %d\n", i, a_pi4GainTable[i]);
    }

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 i4TestCnt, i4Gain;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	a_prSensorTestOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    for (MINT32 i4GainTableIndex = 0; i4GainTableIndex < a_i4GainTableSize; i4GainTableIndex++)
    {


        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);
        ae_cfg.u4Eposuretime = a_i4ExpTime;
        ae_cfg.u4AfeGain = a_pi4GainTable[i4GainTableIndex];
		//ae_cfg.u4IspGain = a_pi4GainTable[i4GainTableIndex];

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Get RAW Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);
        i4SequenceNo++;

    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrGainTableOBStability()\n");

   return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalOB () -
//!  @brief OB calibration
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4Gain: senaor gain (base = 1024)
//!  @param a_i4RepeatTimes: repeat times for OB calibration
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrCalOB(MINT32 a_i4ExpTime,
                                 MINT32 a_i4Gain,
                                 MINT32 a_i4RepeatTimes,
                                 MINT32 a_i4PreCap,
                                 ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{
    ACDK_LOGD("Enter AcdkCalibration::mrCalOB()\n");
	ACDK_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
	ACDK_LOGD("a_i4Gain = %d\n", a_i4Gain);
	ACDK_LOGD("a_i4RepeatTimes = %d\n", a_i4RepeatTimes);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0, u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	prSensorCalibrationOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    for (MINT32 i = 0; i < a_i4RepeatTimes; i++)
    {

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        ae_cfg.u4Eposuretime = a_i4ExpTime;
        ae_cfg.u4AfeGain = a_i4Gain;
		//ae_cfg.u4IspGain = a_i4Gain;

        ACDK_LOGD("[mrCalOB] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[mrCalOB] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[mrCalOB] u4IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms


        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PURE_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Take RAW Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        ACDK_LOGD("[RAW] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
    	                                                                            i4SequenceNo,
                                                                                    g_RAWAnalysisResult.fRAvg,
                                                                                    g_RAWAnalysisResult.fGrAvg,
                                                                                    g_RAWAnalysisResult.fGbAvg,
                                                                                    g_RAWAnalysisResult.fBAvg,
                                                                                    g_RAWAnalysisResult.u4Median);

        if (FAILED(mrRet))
        {
            ACDK_LOGE(" mrAnalyzeRAWImage() error \n");
			prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }
		else
		{
		    u4OBSum += g_RAWAnalysisResult.u4Median;
		}

        i4SequenceNo++;

    }

    prSensorCalibrationOutput->i4OB = (MINT32)((((DOUBLE)u4OBSum)/a_i4RepeatTimes)+0.5);

Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrGainLinearity()\n");

   return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalMinISO () -
//!  @brief minimum ISO calibration
//!  @param a_i4LV: LV of light source box (format: real LV x 10; ex. 100 = LV10)
//!  @param a_i4FNo: lens f/number (format: real f/number x 10; ex. 28 = f/2.8)
//!  @param a_i4OBLevel: sensor OB level
//!  @param a_i450Hz60Hz: [0] 50Hz [1] 60Hz
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrCalMinISO(MINT32 a_i4LV,
                                     MINT32 a_i4FNo,
                                     MINT32 a_i4OBLevel,
                                     MINT32 a_i450Hz60Hz,
                                     MINT32 a_i4PreCap,
                                     ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{
    ACDK_LOGD("Enter AcdkCalibration::mrCalMinISO()\n");
	ACDK_LOGD("a_i4LV = %d\n", a_i4LV);
	ACDK_LOGD("a_i4FNo = %d\n", a_i4FNo);
	ACDK_LOGD("a_i4OBLevel = %d\n", a_i4OBLevel);
	ACDK_LOGD("a_i450Hz60Hz = %d\n", a_i450Hz60Hz);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);

    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0, u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;


	prSensorCalibrationOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);

    /*
    *   Disable Auto Mode for Capture
    */
    ACDK_LOGD("[Disable 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("[Lock Exposure Setting]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);


    DOUBLE dFNo; // F/Number
    DOUBLE dAV;  // AV
    DOUBLE dLV;  // LV
    DOUBLE dTV;  // TV
    DOUBLE dDefaultExpTInSec; // Default exposure time in sec
    MINT32 i4DefaultExpTInUS;  // Default exposure time in us
    DOUBLE dDefaultExpTGavg;  // G average with default exposure time
    DOUBLE dTargetGavg;       // Target G average (18% reflectance)
    DOUBLE dTargetExpTInSec;  // Target exposure time in sec
    MINT32 i4TargetExpTInUS;   // Target exposure time in us
    MINT32 i4TargetExpTInUs_UA;// Flicker-free target exposure time (upper approximation)
    MINT32 i4TargetExpTInUs_LA;// Flicker-free target exposure time (lower approximation)
    DOUBLE dTargetGavg_UA;    // G average with i4TargetExpTInUs_UA
    DOUBLE dTargetGavg_LA;    // G average with i4TargetExpTInUs_LA
    DOUBLE dMinimumISO;       // Minimum ISO
    MINT32 i4Gain = 1024; // Sensor gain = 1024
    ISP_NVRAM_OBC_T obc;
    MINT32 backup_bypass;

    /*
    *   Use OBCGain to Compensate OB Level
    */
    MUINT32 u4OBISPRAWGain = (MUINT32)((((DOUBLE)1023)/(1023-a_i4OBLevel))*OB_SLOPEx1_UNIT+0.5);

    /*
    *   Enable Dynamic Bypass
    */
    if((backup_bypass = NSIspTuning::IspTuningMgr::getInstance().getDynamicBypass()) == -1)
    {
        ACDK_LOGE("[getDynamicBypass] Get Dynamic Bypass Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }
    NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass(MTRUE);

    /*
    *   Prepare OB
    */
    obc.offst0.val = a_i4OBLevel;
    obc.offst1.val = a_i4OBLevel;
    obc.offst2.val = a_i4OBLevel;
    obc.offst3.val = a_i4OBLevel;
    obc.gain0.val = u4OBISPRAWGain;
    obc.gain1.val = u4OBISPRAWGain;
    obc.gain2.val = u4OBISPRAWGain;
    obc.gain3.val = u4OBISPRAWGain;

    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).setEnable(MTRUE);
    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).put(obc);
    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalPreview);
    //Temp. Mark
    //NSIspTuning::IspTuningMgr::getInstance().setPureOBCInfo(&obc);

    prSensorCalibrationOutput->i4ErrorCode = 0; // no error

    /*  STEP 1
    *   Calculate Flicker Free Default Exposure Time
    */
    ACDK_LOGD("____STEP 1. Calculate Flicker-Free Default Exposure Time____\n");

    /*
    *   Calculate default exposure time (assume minimum ISO = 100)
    */
    dFNo = ((DOUBLE)a_i4FNo)/10;
    dAV = 2*log10(dFNo)/log10((DOUBLE)2); // AV = 2xlog2(f/no) = 2xlog10(f/no)/log10(2);
    dLV = ((DOUBLE)a_i4LV)/10;
    dTV = dLV - dAV; // TV = LV - AV
    dDefaultExpTInSec = pow(2, 0-dTV); // T = 2^-TV (uint: sec)
    i4DefaultExpTInUS = (MINT32)(dDefaultExpTInSec*1000000+0.5);

    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4DefaultExpTInUS = (i4DefaultExpTInUS/10000+1)*10000;
    }
    else // 60Hz
    {
    	i4DefaultExpTInUS = (i4DefaultExpTInUS/8333+1)*8333;
    }

    dDefaultExpTInSec = ((DOUBLE)i4DefaultExpTInUS)/1000000;

    ACDK_LOGD("[F/Number] = %4.2f\n", dFNo);
    ACDK_LOGD("[AV] = %4.2f\n", dAV);
    ACDK_LOGD("[LV] = %4.2f\n", dLV);
    ACDK_LOGD("[TV] = %4.2f\n", dTV);
    ACDK_LOGD("[Default Exposure Time in SEC] = %2.15f\n", dDefaultExpTInSec);
    ACDK_LOGD("[Default Exposure Time in US] = %d\n", i4DefaultExpTInUS);
    ACDK_LOGD("[Flicker-Free Default Exposure Time in SEC] = %2.15f\n", dDefaultExpTInSec);
    ACDK_LOGD("[Flicker-Free Default Exposure Time in US] = %d\n", i4DefaultExpTInUS);

    /*  STEP 2
    *   Calculate Target Exposure Time
    */
    ACDK_LOGD("____STEP 2. Calculate Target Exposure Time____\n");

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

    ae_cfg.u4Eposuretime = i4DefaultExpTInUS;
    ae_cfg.u4AfeGain = i4Gain;
	//ae_cfg.u4IspGain = i4Gain;

    ACDK_LOGD("[mrCalMinISO] Exp = %d\n", ae_cfg.u4Eposuretime);
    ACDK_LOGD("[mrCalMinISO] AfeGain = %d\n", ae_cfg.u4AfeGain);
    ACDK_LOGD("[mrCalMinISO] u4IspGain = %d\n", ae_cfg.u4IspGain);

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                    (UINT8 *)&ae_cfg,
                                                    sizeof(ae_cfg),
                                                    (UINT8 *)&ae_cfg,
                                                    sizeof(ae_cfg),
                                                    &u4RetLen);

    MS_SLEEP(500);    //delay for 500ms

    /*
    *   Take Picture
    */
    if((mrRet = takePicture(a_i4PreCap, PROCESSED_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
    {
        ACDK_LOGE("[takePicture] Get RAW Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    /*
    *   Raw Analysis
    */
    if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
        ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    ACDK_LOGD("[RAW-STEP2] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
                                                                             i4SequenceNo,
                                                                                g_RAWAnalysisResult.fRAvg,
                                                                                g_RAWAnalysisResult.fGrAvg,
                                                                                g_RAWAnalysisResult.fGbAvg,
                                                                                g_RAWAnalysisResult.fBAvg,
                                                                                g_RAWAnalysisResult.u4Median);


    // Calculate target exposure time
    dDefaultExpTGavg = (g_RAWAnalysisResult.fGrAvg + g_RAWAnalysisResult.fGbAvg)/2;
    dTargetGavg = 0.18 * 1023; // 18% reflectance
    dTargetExpTInSec = (dDefaultExpTInSec * dTargetGavg) / dDefaultExpTGavg;
    i4TargetExpTInUS = (MINT32)(dTargetExpTInSec*1000000+0.5);

    ACDK_LOGD("[Gavg with Default Exposure Time] = %4.2f\n", dDefaultExpTGavg);
    ACDK_LOGD("[Target Gavg] = %4.2f\n", dTargetGavg);
    ACDK_LOGD("[Target Exposure Time in SEC] = %2.15f\n", dTargetExpTInSec);
    ACDK_LOGD("[Target Exposure Time in US] = %d\n", i4TargetExpTInUS);

    if (i4TargetExpTInUS > 1000000) // > 1sec
    {
        prSensorCalibrationOutput->i4ErrorCode = 2; // Luminance level is too dark
        goto Exit;
    }

    if (a_i450Hz60Hz == 0) // 50Hz
    {
        if (i4TargetExpTInUS < 10000)
        {
            prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
            goto Exit;
        }

        i4TargetExpTInUs_LA = (i4TargetExpTInUS/10000)*10000;
        i4TargetExpTInUs_UA = i4TargetExpTInUs_LA + 10000;
    }
    else // 60Hz
    {
        if (i4TargetExpTInUS < 8333)
        {
            prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
            goto Exit;
        }

        i4TargetExpTInUs_LA = (i4TargetExpTInUS/8333)*8333;
        i4TargetExpTInUs_UA = i4TargetExpTInUs_LA + 8333;
    }

    ACDK_LOGD("[Flicker-Free Target Exposure Time Upper Approximation in US] = %d\n", i4TargetExpTInUs_UA);
    ACDK_LOGD("[Flicker-Free Target Exposure Time Lower Approximation in US] = %d\n", i4TargetExpTInUs_LA);

    /*
    *  Step 3. Get target G average (lower approximation)____
    */
    ACDK_LOGD("____STEP 3. Get Target G Average (Lower Approximation)____\n");

    /*
    *   Disable Auto Mode for Capture
    */
    ACDK_LOGD("[Disable 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("[Lock Exposure Setting]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                    (UINT8 *)&ae_cfg,
                                                    sizeof(ae_cfg),
                                                    (UINT8 *)&ae_cfg,
                                                    sizeof(ae_cfg),
                                                    &u4RetLen);

    ae_cfg.u4Eposuretime = i4TargetExpTInUs_LA;
    ae_cfg.u4AfeGain = i4Gain;
	//ae_cfg.u4IspGain = i4Gain;

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            &u4RetLen);
    MS_SLEEP(500);    //delay for 500ms

    /*
    *   Take Picture
    */
    if((mrRet = takePicture(a_i4PreCap, PROCESSED_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
    {
        ACDK_LOGE("[takePicture] Get RAW Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    ACDK_LOGD("[RAW-STEP3] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
                                                                             i4SequenceNo,
                                                                                g_RAWAnalysisResult.fRAvg,
                                                                                g_RAWAnalysisResult.fGrAvg,
                                                                                g_RAWAnalysisResult.fGbAvg,
                                                                                g_RAWAnalysisResult.fBAvg,
                                                                                g_RAWAnalysisResult.u4Median);

    /*
    *   Raw Analysis
    */
    if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
        ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    /*
    *   Get target G average (lower approximation)
    */
    dTargetGavg_LA = (g_RAWAnalysisResult.fGrAvg + g_RAWAnalysisResult.fGbAvg)/2;

    ACDK_LOGD("[Target G Average (Lower Approximation)] = %4.2f\n", dTargetGavg_LA);

    /*
    *    STEP 4. Get target G average (upper approximation)____
    */
    ACDK_LOGD("____STEP 4. Get Target G Average (Upper Approximation)____\n");


    /*
    *   Disable Auto Mode for Capture
    */
    ACDK_LOGD("[Disable 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("[Lock Exposure Setting]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            &u4RetLen);

    ae_cfg.u4Eposuretime = i4TargetExpTInUs_UA;
    ae_cfg.u4AfeGain = i4Gain;
    //ae_cfg.u4IspGain = i4Gain;

    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            (UINT8 *)&ae_cfg,
                                                            sizeof(ae_cfg),
                                                            &u4RetLen);
    MS_SLEEP(500);    //delay for 500ms

    /*
    *   Take Picture
    */
    if((mrRet = takePicture(a_i4PreCap, PROCESSED_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
    {
        ACDK_LOGE("[takePicture] Get RAW Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    /*
    *   Raw Analysis
    */
    if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
        ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    ACDK_LOGD("[RAW-STEP4] [%d] Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d\n",
                                                                             i4SequenceNo,
                                                                                g_RAWAnalysisResult.fRAvg,
                                                                                g_RAWAnalysisResult.fGrAvg,
                                                                                g_RAWAnalysisResult.fGbAvg,
                                                                                g_RAWAnalysisResult.fBAvg,
                                                                                g_RAWAnalysisResult.u4Median);

    // Get target G average (upper approximation)
    dTargetGavg_UA = (g_RAWAnalysisResult.fGrAvg + g_RAWAnalysisResult.fGbAvg)/2;

    ACDK_LOGD("[Target G Average (Upper Approximation)] = %4.2f\n", dTargetGavg_UA);

    /*
    *   Step 5. calculate minimum ISO____
    */
    ACDK_LOGD("____STEP 5. Calculate Minimum ISO____\n");

    i4TargetExpTInUS = i4TargetExpTInUs_UA - (dTargetGavg_UA-dTargetGavg)*(i4TargetExpTInUs_UA-i4TargetExpTInUs_LA)/(dTargetGavg_UA-dTargetGavg_LA);
    dTargetExpTInSec = ((DOUBLE)i4TargetExpTInUS)/1000000;
    dTV = 0 - log10(dTargetExpTInSec)/log10((DOUBLE)2);
    dMinimumISO = 100 * pow(2, dAV+dTV-dLV);
    prSensorCalibrationOutput->i4MinISO = (MINT32)(dMinimumISO+0.5);

    ACDK_LOGD("[Target Exposure Time in US] = %d\n", i4TargetExpTInUS);
    ACDK_LOGD("[Target Exposure Time in SEC] = %1.15f\n", dTargetExpTInSec);
    ACDK_LOGD("[TV] = %4.2f\n", dTV);
    ACDK_LOGD("[AV] = %4.2f\n", dAV);
    ACDK_LOGD("[LV] = %4.2f\n", dLV);
    ACDK_LOGD("[Minimum ISO] = %4.0f\n", dMinimumISO);



Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    if(backup_bypass != -1)
        NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass((MBOOL)backup_bypass);

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrCalMinISO()\n");

    return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalMinimumSaturationGain () -
//!  @brief minimum saturation gain calibration
//!  @param a_i4TargetDeclineRate: target decline rate for knee point detection
//!  @param a_i4GainBuffer: gain buffer to cover sensor module difference
//!  @param a_i4OBLevel: sensor OB level
//!  @param a_i450Hz60Hz: [0] 50Hz [1] 60Hz
//!  @param a_i4PreCap: [0] preview [1] capture
//!  @param a_i4MinSatGain: minimum saturation gain calibration result
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT AcdkCalibration::mrCalMinimumSaturationGain(MINT32 a_i4TargetDeclineRate,
                                                    MINT32 a_i4GainBuffer,
                                                    MINT32 a_i4OBLevel,
                                                    MINT32 a_i450Hz60Hz,
                                                    MINT32 a_i4PreCap,
                                                    ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{

    ACDK_LOGD("Enter AcdkCalibration::mrCalMinimumSaturationGain()\n");

	ACDK_LOGD("a_i4TargetDeclineRate = %d\n", a_i4TargetDeclineRate);
	ACDK_LOGD("a_i4GainBuffer = %d\n", a_i4GainBuffer);
	ACDK_LOGD("a_i4OBLevel = %d\n", a_i4OBLevel);
	ACDK_LOGD("a_i450Hz60Hz = %d\n", a_i450Hz60Hz);
	ACDK_LOGD("a_i4PreCap = %d\n", a_i4PreCap);


    if (m_bAcdkCalibration == FALSE)
    {
        return E_CCT_CALIBRATION_DISABLE;
    }

    /*
    *   Check Function Available
    */
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null m_pAcdkImgToolObj || m_pAcdkBaseObj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0, u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    ACDK_CDVT_RAW_ANALYSIS_RESULT_T rRAWAnalysisResult[100]; // for temp storage and debug purpose

	memset(rRAWAnalysisResult, 0, sizeof(ACDK_CDVT_RAW_ANALYSIS_RESULT_T) * 100);
	prSensorCalibrationOutput->i4ErrorCode = 0; // No error

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4PreCap, sizeof(a_i4PreCap), NULL, 0, &u4RetLen);


    /*
    *   STEP 1. check if sensor can saturate with exposure time = 0.5 sec____
    */
    ACDK_LOGD("____STEP 1. Check If Sensor Can Saturate with Exposure Time = 0.5 sec____\n");

    MINT32 i4ExpStartInUS, i4ExpEndInUS, i4ExpInterval;
    DOUBLE dG0,dG1,dG2,dG3; // for slope calculation
    DOUBLE dSlope; // for slope calculation
    DOUBLE dMovingSlope[60] = {}; // moving slope
    MINT32 i4Gain = 1024; // sensor gain = 1024
    ISP_NVRAM_OBC_T obc;
    MINT32 backup_bypass;

    MINT32 i,n;
    DOUBLE dCumulativeMovingSlope; // Cumulative moving slope
    DOUBLE dDeclineRate; // Slope decline rate
    DOUBLE dTargetDeclineRate = ((DOUBLE)a_i4TargetDeclineRate)/100;
	DOUBLE dGainBuffer = ((DOUBLE)a_i4GainBuffer)/100;
    DOUBLE dPreviousDeclineRate = 0; // previous slope decline rate
    DOUBLE dMaxLinearG = 0; // maximum linear G (decline rate = 90%)
    DOUBLE dDefaultMinSatGain; // default minimum gain
    DOUBLE dMinSatGain; // add 10% buffer to cover different sensor module

    /*
    *   Use OBCGain to Compensate OB Level
    */
    MUINT32 u4OBISPRAWGain = (MUINT32)((((DOUBLE)1023)/(1023-a_i4OBLevel))*OB_SLOPEx1_UNIT+0.5);

    /*
    *   Enable Dynamic Bypass
    */
    if((backup_bypass = NSIspTuning::IspTuningMgr::getInstance().getDynamicBypass()) == -1)
    {
        ACDK_LOGE("[getDynamicBypass] Get Dynamic Bypass Fail \n");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }
    NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass(MTRUE);

    /*
    *   Prepare OB
    */
    obc.offst0.val = a_i4OBLevel;
    obc.offst1.val = a_i4OBLevel;
    obc.offst2.val = a_i4OBLevel;
    obc.offst3.val = a_i4OBLevel;
    obc.gain0.val = u4OBISPRAWGain;
    obc.gain1.val = u4OBISPRAWGain;
    obc.gain2.val = u4OBISPRAWGain;
    obc.gain3.val = u4OBISPRAWGain;

    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).setEnable(MTRUE);
    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).put(obc);
    NSIspTuning::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalPreview);
    //Temp. Mark
    //NSIspTuning::IspTuningMgr::getInstance().setPureOBCInfo(&obc);

    // determine ExpStart, ExpEnd, and ExpInterval
    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4ExpEndInUS = 500000;
        i4ExpStartInUS = 500000 - 3*10000;
        i4ExpInterval = 10000;
    }
    else // 60Hz
    {
        i4ExpEndInUS = 500000;
        i4ExpStartInUS = 500000 - 3*8333;
        i4ExpInterval = 8333;
    }


    for (MINT32 ExpT = i4ExpStartInUS; ExpT <= i4ExpEndInUS; ExpT += i4ExpInterval)
    {
        ACDK_LOGD("[AcdkCalibration::mrCalMinimumSaturationGain] 1st ExpT = %d, Pat.No = %d\n", ExpT, i4SequenceNo);

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        ae_cfg.u4Eposuretime = ExpT;
        ae_cfg.u4AfeGain = i4Gain;
		//ae_cfg.u4IspGain = i4Gain;

        ACDK_LOGD("[u4Eposuretime] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[u4Eposuretime] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[u4Eposuretime] u4IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms

        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PROCESSED_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] Get RAW Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] Analyze Raw Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;
        i4SequenceNo++;

    }

    dG0 = (rRAWAnalysisResult[0].fGrAvg + rRAWAnalysisResult[0].fGbAvg)/2;
    dG1 = (rRAWAnalysisResult[1].fGrAvg + rRAWAnalysisResult[1].fGbAvg)/2;
    dG2 = (rRAWAnalysisResult[2].fGrAvg + rRAWAnalysisResult[2].fGbAvg)/2;
    dG3 = (rRAWAnalysisResult[3].fGrAvg + rRAWAnalysisResult[3].fGbAvg)/2;

    mrCalculateSlope(0, dG0, 1, dG1, 2, dG2, 3, dG3, dSlope);

    ACDK_LOGD("===================\n");
    ACDK_LOGD("1st [G0] = %4.2f\n", dG0);
    ACDK_LOGD("1st [G1] = %4.2f\n", dG1);
    ACDK_LOGD("1st [G2] = %4.2f\n", dG2);
    ACDK_LOGD("1st [G3] = %4.2f\n", dG3);
    ACDK_LOGD("1st [Slope] = %4.2f\n", dSlope);
    ACDK_LOGD("===================\n");

    if (dSlope > 2)
    {
        ACDK_LOGD("____STEP 1. dSlope > 2\n");
        prSensorCalibrationOutput->i4ErrorCode = 2; // Luminance level is too dark
        goto Exit;
    }

    /*
    *   Step 2. collect moving slope based on 4 consecutive Gavg____
    */
    ACDK_LOGD("____STEP 2. Collect Moving Slope Based on 4 Consecutive Gavg____\n");

   // determine ExpStart, ExpEnd, and ExpInterval
    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4ExpEndInUS = 500000;
        i4ExpStartInUS = 10000;
        i4ExpInterval = 10000;
    }
    else // 60Hz
    {
        i4ExpEndInUS = 500000;
        i4ExpStartInUS = 8333;
        i4ExpInterval = 8333;
    }


    i4SequenceNo = 0;

    for (MINT32 ExpT = i4ExpStartInUS; ExpT <= i4ExpEndInUS; ExpT += i4ExpInterval)
    {
        ACDK_LOGD("[AcdkCalibration::mrCalMinimumSaturationGain] 2nd ExpT = %d, Pat.No = %d\n", ExpT, i4SequenceNo);

        /*
        *   Disable Auto Mode for Capture
        */
        ACDK_LOGD("[Disable 3A]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

        ACDK_LOGD("[Lock Exposure Setting]\n");
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        ae_cfg.u4Eposuretime = ExpT;
        ae_cfg.u4AfeGain = i4Gain;
		//ae_cfg.u4IspGain = i4Gain;

        ACDK_LOGD("[u4Eposuretime] Exp = %d\n", ae_cfg.u4Eposuretime);
        ACDK_LOGD("[u4Eposuretime] AfeGain = %d\n", ae_cfg.u4AfeGain);
        ACDK_LOGD("[u4Eposuretime] u4IspGain = %d\n", ae_cfg.u4IspGain);

        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        (UINT8 *)&ae_cfg,
                                                        sizeof(ae_cfg),
                                                        &u4RetLen);

        MS_SLEEP(500);    //delay for 500ms

        /*
        *   Take Picture
        */
        if((mrRet = takePicture(a_i4PreCap, PROCESSED_RAW8_TYPE)) != S_CCT_CALIBRATION_OK)
        {
            ACDK_LOGE("[takePicture] 2nd Get RAW Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = mrAnalyzeRAWImage()) != S_CCT_CALIBRATION_OK) {
            ACDK_LOGE("[mrAnalyzeRAWImage] 2nd Analyze Raw Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        rRAWAnalysisResult[i4SequenceNo] = g_RAWAnalysisResult;


        /*
        *   Collect moving slope
        */
        if (i4SequenceNo >= 3)
        {
            dG0 = (rRAWAnalysisResult[i4SequenceNo-3].fGrAvg + rRAWAnalysisResult[i4SequenceNo-3].fGbAvg)/2;
            dG1 = (rRAWAnalysisResult[i4SequenceNo-2].fGrAvg + rRAWAnalysisResult[i4SequenceNo-2].fGbAvg)/2;
            dG2 = (rRAWAnalysisResult[i4SequenceNo-1].fGrAvg + rRAWAnalysisResult[i4SequenceNo-1].fGbAvg)/2;
            dG3 = (rRAWAnalysisResult[i4SequenceNo].fGrAvg + rRAWAnalysisResult[i4SequenceNo].fGbAvg)/2;

            mrCalculateSlope(0, dG0, 1, dG1, 2, dG2, 3, dG3, dSlope);

            dMovingSlope[i4SequenceNo] = dSlope;

            ACDK_LOGD("2nd [G0] = %4.2f\n", dG0);
            ACDK_LOGD("2nd [G1] = %4.2f\n", dG1);
            ACDK_LOGD("2nd [G2] = %4.2f\n", dG2);
            ACDK_LOGD("2nd [G3] = %4.2f\n", dG3);
            ACDK_LOGD("2nd [Slope] = %4.2f\n", dSlope);

            if ((dSlope > -2) && (dSlope < 2)) // Almost saturate
            {
                break;
            }
        }

        i4SequenceNo++;

    }

    if (i4SequenceNo < 20)
    {
        ACDK_LOGE("[i4SequenceNo < 20], Luminance level is too bright \n");
        prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
        goto Exit;
    }

    /*
    *   ____Step 3. Calculate minimum saturation gain____
    */
    ACDK_LOGD("____STEP 3. Calculate Minimum Saturation Gain____\n");

    n = 0;
    dCumulativeMovingSlope = dMovingSlope[3];

    for (i=3; i<= i4SequenceNo; i++)
    {
        dDeclineRate = 1 - (dMovingSlope[i]/dCumulativeMovingSlope);

        ACDK_LOGD("[%d] dTargetDeclineRate = %4.4f; dDeclineRate = %4.4f; dMovingSlope = %4.2f; dCumulativeMovingSlope = %4.2f\n",
            	      i,
            	      dTargetDeclineRate,
            	      dDeclineRate,
            	      dMovingSlope[i],
            	      dCumulativeMovingSlope);


        if (dDeclineRate < dTargetDeclineRate)
        {
            dPreviousDeclineRate = dDeclineRate; // Backup previous decline rate
            dCumulativeMovingSlope = (dMovingSlope[i] + dCumulativeMovingSlope * n) / (n+1);
            n++;
        }
        else // Estimate maximum linear G
        {
            dG0 = (rRAWAnalysisResult[i-1].fGrAvg + rRAWAnalysisResult[i-1].fGbAvg)/2;
            dG1 = (rRAWAnalysisResult[i].fGrAvg + rRAWAnalysisResult[i].fGbAvg)/2;

            dMaxLinearG = dG0 + (dG1-dG0)*(dTargetDeclineRate-dPreviousDeclineRate)/(dDeclineRate-dPreviousDeclineRate);

			ACDK_LOGD("[DeclineRate0] = %4.4f\n",dPreviousDeclineRate);
			ACDK_LOGD("[DeclineRate1] = %4.4f\n",dDeclineRate);
			ACDK_LOGD("[TargetDeclineRate] = %4.4f\n",dTargetDeclineRate);
            ACDK_LOGD("[Index0] = %d\n",i-1);
            ACDK_LOGD("[Index1] = %d\n",i);
            ACDK_LOGD("[G0] = %4.2f\n",dG0);
            ACDK_LOGD("[G1] = %4.2f\n",dG1);
            ACDK_LOGD("[MaxLinearG] = %4.2f\n",dMaxLinearG);

            break;
        }
    }

    dDefaultMinSatGain = ((DOUBLE)1023)/dMaxLinearG;
    dMinSatGain = dDefaultMinSatGain * (1+dGainBuffer);
	prSensorCalibrationOutput->i4MinSatGain = ((MINT32)(dMinSatGain * 1024 + 4) / 8) * 8; // must be multiple of 8


    ACDK_LOGD("dDefaultMinSatGain = %4.4f\n",dDefaultMinSatGain);
	ACDK_LOGD("dGainBuffer = %4.4f\n",dGainBuffer);
    ACDK_LOGD("dMinSatGain = %4.4f\n",dMinSatGain);
    ACDK_LOGD("i4MinSatGain = %d\n",prSensorCalibrationOutput->i4MinSatGain);

	ACDK_LOGD("============ Debug Info =============\n");
    ACDK_LOGD("[Exp]  [R]   [Gr]   [Gb]   [B]\n");

    for (MINT32 i=0; i<= i4SequenceNo; i++)
    {
        ACDK_LOGD("[%d] %4.2f %4.2f %4.2f %4.2f\n", i+1,
                                                    rRAWAnalysisResult[i].fRAvg,
                                                    rRAWAnalysisResult[i].fGrAvg,
                                                    rRAWAnalysisResult[i].fGbAvg,
                                                    rRAWAnalysisResult[i].fBAvg);
    }

    ACDK_LOGD("======================================\n");

Exit:
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }

    if(backup_bypass != -1)
        NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass((MBOOL)backup_bypass);

    ACDK_LOGD("[Restore 3A]\n");
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    ACDK_LOGD("Exit AcdkCalibration::mrCalMinimumSaturationGain()\n");

   return mrRet;


}


extern void lscReConstruct(int nrow, int ncol, UINT8  *input_buffer, UINT16 *output_buffer);
extern void vLSC_Calibration_INIT(unsigned short* a_u1BufferAddr);
extern void vLSC_Calibration_END(void);
extern void vLSC_PARAM_INIT(LSC_CAL_INI_PARAM_T a_rLSCCaliINIParam);
extern void vLsc_Remap_to_Bayer0(unsigned short *Src, unsigned short *Dst, unsigned int Width, unsigned int Height, int BayerOrder);
extern MRESULT mrLSC_Calibrate(LSC_CALI_INFO_T cali_info, UINT8 a_u1Mode, UINT16 a_u2SVDTermNum);
/* Lens shading calibratino global variables declaration */
extern unsigned short *g_raw;
extern LSC_CALI_INFO_T g_lsc_cali_info;
extern unsigned int *g_src_tbl;
extern unsigned int *g_dst_tbl;
extern LSC_PARAM_T g_lsc_param;
extern LSC_RESULT_T g_lsc_result;
extern int g_col_idx[GRID_MAX];
extern int g_row_idx[GRID_MAX];

/////////////////////////////////////////////////////////////////////////
//
//   mrCalLenShading () -
//!  @brief calibrate lens shading test
//!  @param a_i4GridXNUM: X Grid number
//!  @param a_i4GridYNUM: Y Grid number
//!  @param a_i4XINIBorder: X initial pixel
//!  @param a_i4XENDBorder: X end pixel = width - a_i4XENDBorder
//!  @param a_i4YINIBorder: Y initial pixel
//!  @param a_i4YENDBorder: Y end pixel = height - a_i4YENDBorder
//!  @param a_i4Iris : The Iris
//!  @param a_i4PreCap: The preview/capture mode
//!  @param a_u1FixShadingIndex: set all shading table for different color temperature to the same value
//!
/////////////////////////////////////////////////////////////////////////
MRESULT SetShadingAE(AcdkBase *m_pAcdkBaseObj) {
    MRESULT mrRet = S_CCT_CALIBRATION_OK;

    MUINT32 u4RetLen = 0;
    MINT16 u2SVDTermNum = 7;
    //MUINT32 u4AEEnable;
    INT32 i4AEMeteringMode = 0; //0 : LIB3A_AE_METERING_MODE_CENTER_WEIGHT, 2:LIB3A_AE_METERING_MODE_AVERAGE

    ACDK_AE_MODE_CFG_T rAEExpPara;
    MUINT32 u4AfeGainTemp;
    MUINT32 u4ExposureTimeTemp;

    //MUINT8 index = (UINT8)a_u4Index;

    MUINT8 uCompMode = CAMERA_TUNING_PREVIEW_SET;

    MINT32 i4AEFlickerMode = 0;

    TBL_INFO_T rTableINTO;

   typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    i_type Shading_Control;
    i_type Shading_Calibration_Control;

    //! ====================================================
    //! Get AE parameter (sensor gain & flicker setting) by user,
    //! User should set AE sensor gain & flicker by CCT tool to a specificated value for calibration.
    //! ====================================================
    memset(&rAEExpPara,0, sizeof(ACDK_AE_MODE_CFG_T));
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
                                                        (UINT8 *)&rAEExpPara,
                                                        sizeof(ACDK_AE_MODE_CFG_T),
                                                        (UINT8 *)&rAEExpPara,
                                                        sizeof(ACDK_AE_MODE_CFG_T),
                                                        &u4RetLen);

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_GET_BAND,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&i4AEFlickerMode,
                                                        sizeof(MINT32),
                                                        &u4RetLen);

    ACDK_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "(AfeGain, flicker mode, exp time) = (%d, %d, %d)\n"
                        , u4AfeGainTemp
                        , i4AEFlickerMode
                        , rAEExpPara.u4Eposuretime
                        );

    //average mode : disable histogram condition check.
    //DisableAE before set AE parameter
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_SET_METERING_MODE, (UINT8 *)&i4AEMeteringMode, sizeof(INT32), NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    sleep(1); //wait 1 sec for AE stable

    // get current AE parameter
    memset(&rAEExpPara,0, sizeof(ACDK_AE_MODE_CFG_T));

    //ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
    //CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
                                                        (UINT8 *)&rAEExpPara,
                                                        sizeof(ACDK_AE_MODE_CFG_T),
                                                        (UINT8 *)&rAEExpPara,
                                                        sizeof(ACDK_AE_MODE_CFG_T),
                                                        &u4RetLen);

    ACDK_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "AE (AfeGain, exposure) = (%d, %d)\n"
                        , rAEExpPara.u4AfeGain
                        , rAEExpPara.u4Eposuretime
                        );


    AE_NVRAM_T pAENVRAM;
    MUINT32 gLength = 0;
    NS3A::AeMgr::getInstance().CCTOPAEGetNVRAMParam((MVOID *)&pAENVRAM, &gLength);
    //u4AfeGainTemp = rAEExpPara.u4AfeGain;
    u4AfeGainTemp = pAENVRAM.rDevicesInfo.u4MinGain;
    ACDK_LOGE("[%s] [AcdkCalibration::mrCalLenShading]"
                "AE (AfeGain, exposure, minGain) = (%d, %d, %d)\n", __FUNCTION__
                , rAEExpPara.u4AfeGain
                , rAEExpPara.u4Eposuretime
                , u4AfeGainTemp
        );

     //DisableAE before set AE parameter
     CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);

     //recalculate exposure gain and set AE parameter
     //this is for using user setting sensor gain to do calibration
     u4ExposureTimeTemp = rAEExpPara.u4Eposuretime * rAEExpPara.u4AfeGain / u4AfeGainTemp;

    ACDK_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "(u4ExposureTimeTemp ++) = (%d)\n"
                        , u4ExposureTimeTemp
                        );
    if (i4AEFlickerMode != 1) //if user not select, default is 60Hz
        {
        u4ExposureTimeTemp = (u4ExposureTimeTemp/83)*83;  // 1/120 = 8333 us
    }
    else //LIB3A_AE_FLICKER_MODE_50HZ
    {
        u4ExposureTimeTemp = (u4ExposureTimeTemp/100)*100;  // 1/100 = 10000 us
    }


    ACDK_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "(u4ExposureTimeTemp --) = (%d)\n"
                        , u4ExposureTimeTemp
                        );

     if (u4ExposureTimeTemp < 2000000)
     {
         rAEExpPara.u4Eposuretime = u4ExposureTimeTemp;
         rAEExpPara.u4GainMode = 0; // AfeGain
         rAEExpPara.u4AfeGain = u4AfeGainTemp;
     }

    ACDK_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "Calibration (AfeGain, exposure) = (%d, %d)\n"
                        , rAEExpPara.u4AfeGain
                        , rAEExpPara.u4Eposuretime
                        );

    //ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
    //CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,  (UINT8 *)&rAEExpPara,  sizeof(ACDK_AE_MODE_CFG_T), NULL, 0, &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_SET_CAPTURE_PARA,  (UINT8 *)&rAEExpPara,  sizeof(ACDK_AE_MODE_CFG_T), NULL, 0, &u4RetLen);
    MS_SLEEP(500);    //delay for 500ms
    return S_CCT_CALIBRATION_OK;
}

MBOOL
AcdkCalibration::
GetShadingRaw(eACDK_CAMERA_MODE mode, UINT8 ColorTemp) {
    eACDK_CAP_FORMAT img_type = PROCESSED_RAW8_TYPE;//PURE_RAW8_TYPE; //PROCESSED_RAW8_TYPE;

    MBOOL enable = MTRUE;
    MBOOL disable = MFALSE;
    MBOOL SHAD_EN;
    MUINT32 len;

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_SET_OB_ON_OFF, (UINT8 *)&enable,
            sizeof(enable), NULL, 0, NULL);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_SET_NR_ON_OFF, (UINT8 *)&enable,
            sizeof(enable), NULL, 0, NULL);
    SHAD_EN = NSIspTuning::LscMgr::getInstance()->isEnable();

    NSIspTuning::LscMgr::getInstance()->enableLsc(MFALSE);

    if(takePicture(mode, img_type) != S_CCT_CALIBRATION_OK)
    {
        ACDK_LOGE("[%s] Get RAW Image Fail \n", __FUNCTION__);
        ACDK_LOGE(" Get Capture Image Buffer Addr Fail\n");
        return MFALSE;
    }

    NSIspTuning::LscMgr::getInstance()->enableLsc(SHAD_EN);

    return MTRUE;
}

MRESULT AcdkCalibration::mrCalLenShading(INT32 a_i4XINIBorder, INT32 a_i4XENDBorder, INT32 a_i4YINIBorder, INT32 a_i4YENDBorder,  UINT16 a_u2AttnRatio, UINT32 a_u4Index, INT32 a_i4PreCap, UINT8 a_u1FixShadingIndex)
{
    ACDK_LOGD(" Calibrate Lens Shading \n");
    ACDK_LOGD(" a_i4XINIBorder %d  a_i4XENDBorder %d  a_i4YINIBorder %d  a_i4YENDBorder %d\n"""
            "   a_u2AttnRatio %d  a_u4Index %d  a_i4PreCap %d  a_u1FixShadingIndex %d \n",
    	 a_i4XINIBorder,  a_i4XENDBorder,  a_i4YINIBorder,  a_i4YENDBorder,   a_u2AttnRatio,  a_u4Index,  a_i4PreCap,  a_u1FixShadingIndex);

    ACDK_LOGD(" Calibrate Lens Shading \n");

    // Enable Bypass Dynamic
    NSIspTuning::IspTuningMgr::getInstance().setOperMode(NSIspTuning::EOperMode_Meta);
    NSIspTuning::LscMgr::getInstance()->enableTSF(MFALSE);

    if (m_bAcdkCalibration == FALSE)
    {
        return S_CCT_CALIBRATION_OK;
    }

    //we already check the null obj in enable calibration funciton !!
    if (m_pAcdkImgToolObj == NULL || m_pAcdkBaseObj == NULL)
    {
        ACDK_LOGE(" Null Acdk Obj\n");
        return E_CCT_CALIBRATION_NULL_OBJ;
    }

    if (a_u4Index > 4)
    {
        ACDK_LOGE("Error calibration index \n");
        return E_CCT_CALIBRATION_BAD_ARG;
    }


    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MRESULT mrRetHWTbl = S_CCT_CALIBRATION_OK;
    LSC_RESULT  u4RetLSCHwTbl = S_LSC_CONVERT_OK;
    LSC_RESULT  u4RetSL2Cal   = S_LSC_CONVERT_OK;
    LSC_CAL_INI_PARAM_T rLSCCaliINIParam;
    memset (&rLSCCaliINIParam, 0, sizeof(LSC_CAL_INI_PARAM_T));
    MUINT32 u4RetLen = 0;
    MINT16 u2SVDTermNum = 7;
    //   MINT32 u4CaliMode = 0;

    INT32 i4AWBEnable;
    MUINT32 u4AEEnable;
    INT32 i4AEMeteringMode = 0; //0 : LIB3A_AE_METERING_MODE_CENTER_WEIGHT, 2:LIB3A_AE_METERING_MODE_AVERAGE

    ACDK_AE_MODE_CFG_T rAEExpPara;
    MUINT32 u4AfeGainTemp;
    MUINT32 u4ExposureTimeTemp;

    MUINT8 index = (UINT8)a_u4Index;

    MUINT8 uCompMode = CAMERA_TUNING_PREVIEW_SET;

    MINT32 i4AEFlickerMode = 0;
    unsigned short* g_raw_bayer0 = NULL;
    float* LscGaintoHwTblBuf     = NULL;
#if 0//Sammy
   TBL_INFO_T rTableINTO;
#else
    MUINT32 u4DstTblSize;
#endif
   typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    i_type Shading_Control;
    i_type Shading_Calibration_Control;

    //disable lsc compensation when calibration
    m_eRawFormat = TWO_BYTE_RAW;//TWO_BYTE_RAW;//ONE_BYTE_RAW;

   //store AE status
   mrRet = CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_ENABLE_INFO,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&u4AEEnable,
                                                        sizeof(u4AEEnable),
                                                        &u4RetLen);

    if (SetShadingAE(m_pAcdkBaseObj) != S_CCT_CALIBRATION_OK)
        goto Exit;

    //disable AWB
    //store current AWB status
    mrRet = CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&i4AWBEnable,
                                                        sizeof(INT32),
                                                        &u4RetLen);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

    //Enable Calibration
    //CCTIF_FeatureCtrl(ACDK_CCT_FEATURE_SET_OPERATION_MODE, (MUINT8 *)ISP_OPER_MODE_META, 0, NULL, 0, &u4RetLen);

    //CAMERA_TUNING_SET_ENUM
    ACDK_CCT_SHADING_COMP_STRUCT ACDK_Output;
    memset(&ACDK_Output,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));
    winmo_cct_shading_comp_struct CCT_shading;
    memset(&CCT_shading,0,sizeof(winmo_cct_shading_comp_struct));

    ACDK_Output.pShadingComp = &CCT_shading;

    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
            (UINT8 *)&a_u4Index,
            sizeof(a_u4Index),
            (UINT8 *)NULL,
            0,
            &u4RetLen);


    if ((UINT32)a_i4PreCap == PREVIEW_MODE)   //preview Test
    {
        //Store Shading Enable flag before calibration
        memset(&Shading_Control,0,sizeof(i_type));
        Shading_Control.Mode = CAMERA_TUNING_PREVIEW_SET;
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
            (UINT8 *)&Shading_Control,
            sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
            (UINT8 *)&Shading_Control,
            sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
            &u4RetLen);
        ACDK_LOGD("[AcdkCalibration]"
                            "CCAP Get Shading (mode,Enable):(%d,%d)\n"
                            ,Shading_Control.Mode
                            ,Shading_Control.Enable
                            );

        //disable AWB
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

        //disable Shading compensation before calibration
        memset(&Shading_Calibration_Control,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));
        Shading_Calibration_Control.Mode = CAMERA_TUNING_PREVIEW_SET;
        Shading_Calibration_Control.Enable = false;

        uCompMode = CAMERA_TUNING_PREVIEW_SET;
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

        m_u4CapMode = 1; //"0" will do 1/2 subsampe of image
        ACDK_LOGD("[AcdkCalibration:%s] Before takePicture", __FUNCTION__);

        if (MFALSE == GetShadingRaw(PREVIEW_MODE, a_u4Index))
            goto Exit;

        ACDK_LOGD("[AcdkCalibration:%s] After takePicture", __FUNCTION__);

        rLSCCaliINIParam.i4GridXNUM = CCT_shading.SHADINGBLK_XNUM + 1;    //a_i4GridXNUM;
        rLSCCaliINIParam.i4GridYNUM = CCT_shading.SHADINGBLK_YNUM + 1;    //a_i4GridYNUM;
        rLSCCaliINIParam.i4AvgWinSize = 33; //User configurable later.
        rLSCCaliINIParam.i4XINIBorder = a_i4XINIBorder;
        rLSCCaliINIParam.i4XENDBorder =  a_i4XENDBorder;
        rLSCCaliINIParam.i4YINIBorder =  a_i4YINIBorder;
        rLSCCaliINIParam.i4YENDBorder = a_i4YENDBorder;
        rLSCCaliINIParam.u4ImgWidth = m_u2RawImgWidth;
        rLSCCaliINIParam.u4ImgHeight = m_u2RawImgHeight;
        rLSCCaliINIParam.u2BayerStart = (UINT16)m_eColorOrder;
        rLSCCaliINIParam.poly_coef.coef_a = (float)ShadingATNTable[a_u2AttnRatio][0] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_b = (float)ShadingATNTable[a_u2AttnRatio][1] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_c = (float)ShadingATNTable[a_u2AttnRatio][2] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_d = (float)ShadingATNTable[a_u2AttnRatio][3] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_e = (float)ShadingATNTable[a_u2AttnRatio][4] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_f = 1;

        g_raw_bayer0 = (unsigned short*)malloc(m_u2RawImgWidth*m_u2RawImgHeight*2*sizeof(MUINT8));
        if(!g_raw_bayer0)
        {
             ACDK_LOGD(" Mem allocation Fail \n");
             mrRet = E_CCT_CALIBRATION_NULL_OBJ; 
             goto Exit;
        }
        vLsc_Remap_to_Bayer0((unsigned short*)m_pucRawBuf, g_raw_bayer0, m_u2RawImgWidth, m_u2RawImgHeight, rLSCCaliINIParam.u2BayerStart);


        ACDK_LOGD("GridXNUM:%d\n", rLSCCaliINIParam.i4GridXNUM);
        ACDK_LOGD("GridYNUM:%d\n", rLSCCaliINIParam.i4GridYNUM);

        ACDK_LOGD(" Attenuation Poly coef :%8f, %8f, %8f, %8f ,%8f, %8f\n",
                rLSCCaliINIParam.poly_coef.coef_a,
                rLSCCaliINIParam.poly_coef.coef_b,
                rLSCCaliINIParam.poly_coef.coef_c,
                rLSCCaliINIParam.poly_coef.coef_d,
                rLSCCaliINIParam.poly_coef.coef_e,
                rLSCCaliINIParam.poly_coef.coef_f);

        if (a_u2AttnRatio == 0)
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 0;
        }
        else
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 1;
        }

        ACDK_LOGD("[%s] vLSC_PARAM_INIT", __FUNCTION__);
        vLSC_PARAM_INIT(rLSCCaliINIParam);
        ACDK_LOGD("[%s] vLSC_Calibration_INIT", __FUNCTION__);
        vLSC_Calibration_INIT(g_raw_bayer0);
        ACDK_LOGD("[%s] mrLSC_Calibrate", __FUNCTION__);
        mrLSC_Calibrate(g_lsc_cali_info, 0 /* Preview MODE */, u2SVDTermNum);//preview

        if(g_SAVE_IMG)
        {
            char fileName[100];
            sprintf(fileName, "%s//preraw.raw", MEDIA_PATH);

            ACDK_LOGD(" Save Preview RAW Image \n");
            ACDK_LOGD(" Save File Name:%s\n", fileName);
            ACDK_LOGD(" Width:%d\n", m_u2RawImgWidth);
            ACDK_LOGD(" Height:%d\n", m_u2RawImgHeight);

            FILE *pRawFp = fopen(fileName, "wb");

            if (NULL == pRawFp )
            {
                ACDK_LOGE("Can't open file to save Image\n");
                mrRet =E_CCT_CALIBRATION_FILE_OPEN_FAIL;
                goto Exit;
            }

            INT32 i4WriteCnt = fwrite(m_pucRawBuf,1, (m_u2RawImgWidth* m_u2RawImgHeight* 1),pRawFp);
            ACDK_LOGD("Raw done!!\n");

            fflush(pRawFp);
            fclose(pRawFp);

            sync();
        }
#if 0 // sammy
        //!Apply parameter ISP
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        UINT16 uShadingTableGain[MAX_SHADING_SIZE]={0}; // buffer size > rLSCCaliINIParam.i4GridXNUM*rLSCCaliINIParam.i4GridYNUM*4
        UINT32 uShadingTableCoef[MAX_SHADING_SIZE]={0};
        FILE *fp = fopen("//data//Preview_LSC.bin", "rb"); //hwtbl.bin
        //FILE *fp = fopen("//data//hwtbl.bin", "rb"); //hwtbl.bin
        if (fp == NULL)
        {
            ACDK_LOGE("ERROR to open Calibration data \n");
            goto Exit;
        }

        ACDK_LOGD("open Calibration data %d byte\n", MAX_SVD_SHADING_SIZE);
        fread(uShadingTableSVD,  MAX_SVD_SHADING_SIZE, 1,  fp);

        lscReConstruct(rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM*4, uShadingTableSVD, uShadingTableGain);
        //calculate hw table
        memcpy(&(rTableINTO.reg_mn), uShadingTableSVD,sizeof(rTableINTO.reg_mn));
        memcpy(&(rTableINTO.reg_info0), uShadingTableSVD+sizeof(rTableINTO.reg_mn),
                sizeof(rTableINTO.reg_info0));
        memcpy(&(rTableINTO.reg_info1),
                uShadingTableSVD+sizeof(rTableINTO.reg_mn)+sizeof(rTableINTO.reg_info0),
                sizeof(rTableINTO.reg_info1));
        rTableINTO.src_tbl_addr = (UINT32 *)uShadingTableGain;
        rTableINTO.dst_tbl_addr = uShadingTableCoef;
        ACDK_LOGD("[%s] vlscHWTBLCAL", __FUNCTION__);
        mrlscHWTBLCAL(rTableINTO);
#else  
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        LscGaintoHwTblBuf = (float*)malloc(BUFFERSIZE*sizeof(float));
        if(!LscGaintoHwTblBuf)
        {
            ACDK_LOGD("[%s] Allocate LscGaintoHwTblBuf Fail\n", __FUNCTION__);
            mrRet = E_CCT_CALIBRATION_NULL_OBJ; 
            goto Exit;
        }
        u4RetSL2Cal   = LscSL2Calcu(   g_lsc_cali_info.tbl_info,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       rLSCCaliINIParam.u4ImgWidth,
                                       rLSCCaliINIParam.u4ImgHeight);         
        u4DstTblSize = (rLSCCaliINIParam.i4GridXNUM-1)*(rLSCCaliINIParam.i4GridYNUM-1)*16;      
        u4RetLSCHwTbl = LscGaintoHWTbl(g_lsc_cali_info.tbl_info.src_tbl_addr_float,
                                       g_lsc_cali_info.tbl_info.dst_tbl_addr,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       m_u2RawImgWidth,
                                       m_u2RawImgHeight,
                                       (void*)LscGaintoHwTblBuf,
                                       BUFFERSIZE);
        if(u4RetLSCHwTbl == E_LSC_CONVERT_BITS_OVERFLOW)
          mrRetHWTbl = E_CCT_CALIBRATION_COEF_OVERFLOW;  
        if(u4RetSL2Cal   == E_LSC_CONVERT_WRONG_INPUT)
          mrRet          =  E_CCT_CALIBRATION_BAD_ARG;
#endif

        ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
        memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTable.Length = MAX_SVD_SHADING_SIZE;
        shadingTable.Offset = 0;
        shadingTable.pBuffer = (UINT32 *)&uShadingTableSVD[0];
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;

        ACDK_CCT_TABLE_SET_STRUCT  shadingTablePolyCoef;
        memset (&shadingTablePolyCoef, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTablePolyCoef.Offset = 0;
#if 0 // sammy
        shadingTablePolyCoef.Length = MAX_SHADING_PvwFrm_SIZE;
        shadingTablePolyCoef.pBuffer = (UINT32 *)uShadingTableCoef;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
#else
        shadingTablePolyCoef.Length = u4DstTblSize;
        shadingTablePolyCoef.pBuffer = (UINT32 *)g_lsc_cali_info.tbl_info.dst_tbl_addr;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
#endif
        shadingTablePolyCoef.Mode = CAMERA_TUNING_PREVIEW_SET;
        if (a_u1FixShadingIndex == 0)
        {
            shadingTable.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                  (UINT8 *)&shadingTable,
                                                  sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                  NULL,
                                                  0,
                                                  &u4RetLen);
            shadingTablePolyCoef.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                  (UINT8 *)&shadingTablePolyCoef,
                                                  sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                  NULL,
                                                  0,
                                                  &u4RetLen);
        }
        else
        {
            int i;
            for (i = 0; i < 4; i++)
            {
                ACDK_LOGD("[%s] color temp %d", __FUNCTION__, i);
                shadingTable.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                        (UINT8 *)&shadingTable,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
                shadingTablePolyCoef.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                        (UINT8 *)&shadingTablePolyCoef,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
            }
        }
#if 0 // sammy
        fclose(fp);
    if(g_raw_bayer0 != NULL)
    {
        free(g_raw_bayer0);
        g_raw_bayer0 = NULL;
    }
    if(LscGaintoHwTblBuf != NULL)
    {
        free(LscGaintoHwTblBuf);
        LscGaintoHwTblBuf = NULL;
    }

	vLSC_Calibration_END();
#endif
        ACDK_LOGD("[%s] Preview Done\n", __FUNCTION__);
    }

    if ((UINT32)a_i4PreCap == CAPTURE_MODE)    //Capture Test
    {
        //Store Shading Enable flag before calibration
        memset(&Shading_Control,0,sizeof(i_type));
        Shading_Control.Mode = CAMERA_TUNING_CAPTURE_SET;
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
            (UINT8 *)&Shading_Control,
            sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
            (UINT8 *)&Shading_Control,
            sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
            &u4RetLen);
        ACDK_LOGD("[AcdkCalibration]"
                            "CCAP Get Shading (mode,Enable):(%d,%d)\n"
                            ,Shading_Control.Mode
                            ,Shading_Control.Enable
                            );
        //disable AWB
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

        //disable Shading compensation before calibration
        memset(&Shading_Calibration_Control,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));
        Shading_Calibration_Control.Mode = CAMERA_TUNING_CAPTURE_SET;
        Shading_Calibration_Control.Enable = false;

        uCompMode = CAMERA_TUNING_CAPTURE_SET;
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

        m_u4CapMode = 1; //capture
        if (MFALSE == GetShadingRaw(CAPTURE_MODE, a_u4Index))
            goto Exit;


        rLSCCaliINIParam.i4GridXNUM = CCT_shading.SHADINGBLK_XNUM + 1;
        rLSCCaliINIParam.i4GridYNUM = CCT_shading.SHADINGBLK_YNUM + 1;
        rLSCCaliINIParam.i4AvgWinSize = 33; //User configurable later.
        rLSCCaliINIParam.i4XINIBorder = a_i4XINIBorder;
        rLSCCaliINIParam.i4XENDBorder = a_i4XENDBorder;
        rLSCCaliINIParam.i4YINIBorder = a_i4YINIBorder;
        rLSCCaliINIParam.i4YENDBorder = a_i4YENDBorder;
        rLSCCaliINIParam.u4ImgWidth = m_u2RawImgWidth;
        rLSCCaliINIParam.u4ImgHeight = m_u2RawImgHeight;
        rLSCCaliINIParam.u2BayerStart = (UINT16)m_eColorOrder;
        rLSCCaliINIParam.poly_coef.coef_a = (float)ShadingATNTable[a_u2AttnRatio][0] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_b = (float)ShadingATNTable[a_u2AttnRatio][1] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_c = (float)ShadingATNTable[a_u2AttnRatio][2] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_d = (float)ShadingATNTable[a_u2AttnRatio][3] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_e = (float)ShadingATNTable[a_u2AttnRatio][4] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_f = 1;

        g_raw_bayer0 = (unsigned short*)malloc(m_u2RawImgWidth*m_u2RawImgHeight*2*sizeof(MUINT8));
        if(!g_raw_bayer0)
        {
             ACDK_LOGD(" Mem allocation Fail \n");
             mrRet = E_CCT_CALIBRATION_NULL_OBJ; 
             goto Exit;
        }
        vLsc_Remap_to_Bayer0((unsigned short*)m_pucRawBuf, g_raw_bayer0, m_u2RawImgWidth, m_u2RawImgHeight, rLSCCaliINIParam.u2BayerStart);

        ACDK_LOGD("Capture Mode \n");
        ACDK_LOGD("GridXNUM:%d\n", rLSCCaliINIParam.i4GridXNUM);
        ACDK_LOGD("GridYNUM:%d\n", rLSCCaliINIParam.i4GridYNUM);
        ACDK_LOGD("u4ImgWidth:%d\n", rLSCCaliINIParam.u4ImgWidth);
        ACDK_LOGD("u4ImgHeight:%d\n", rLSCCaliINIParam.u4ImgHeight);

        ACDK_LOGD(" Attenuation Poly coef :%8f, %8f, %8f, %8f ,%8f, %8f\n",
        rLSCCaliINIParam.poly_coef.coef_a,rLSCCaliINIParam.poly_coef.coef_b,rLSCCaliINIParam.poly_coef.coef_c,rLSCCaliINIParam.poly_coef.coef_d,rLSCCaliINIParam.poly_coef.coef_e,rLSCCaliINIParam.poly_coef.coef_f);

        if (a_u2AttnRatio == 0)
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 0;
        }
        else
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 1;
        }


        vLSC_PARAM_INIT(rLSCCaliINIParam);
        vLSC_Calibration_INIT(g_raw_bayer0);
        mrLSC_Calibrate(g_lsc_cali_info, 1 /* Capture MODE */, u2SVDTermNum);//capture

        if(g_SAVE_IMG)
        {
            char fileName[100];
            sprintf(fileName, "%s//capraw.raw", MEDIA_PATH);

            ACDK_LOGD(" Save Capture RAW Image \n");
            ACDK_LOGD(" Save File Name:%s\n", fileName);
            ACDK_LOGD(" Width:%d\n", m_u2RawImgWidth);
            ACDK_LOGD(" Height:%d\n", m_u2RawImgHeight);


            FILE *pRawFp = fopen(fileName, "wb");

            if (NULL == pRawFp )
            {
                ACDK_LOGE("Can't open file to save Image\n");
                mrRet =  E_CCT_CALIBRATION_FILE_OPEN_FAIL;
                goto Exit;
            }

            INT32 i4WriteCnt = fwrite(m_pucRawBuf,1, (m_u2RawImgWidth* m_u2RawImgHeight* 1),pRawFp);
            ACDK_LOGD("Raw done!!\n");

            fflush(pRawFp);
            fclose(pRawFp);

            sync();
        }

#if 0 // sammy
        //!Apply parameter ISP
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        UINT16 uShadingTableGain[MAX_SHADING_SIZE]={0};
        UINT32 uShadingTableCoef[MAX_SHADING_SIZE]={0};
        FILE *fp = fopen("//data//Capture_LSC.bin", "rb"); //hwtbl.bin
        //FILE *fp = fopen("//data//hwtbl.bin", "rb"); //hwtbl.bin
        if (fp == NULL)
        {
            ACDK_LOGE("ERROR to open Calibration data \n");
            goto Exit;
        }

        ACDK_LOGE("open Calibration data %d byte\n", MAX_SVD_SHADING_SIZE);
        fread(uShadingTableSVD,  MAX_SVD_SHADING_SIZE, 1, fp);

        lscReConstruct(rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM*4, uShadingTableSVD, uShadingTableGain);
        //calculate hw table
        memcpy(&(rTableINTO.reg_mn), uShadingTableSVD,sizeof(rTableINTO.reg_mn));
        memcpy(&(rTableINTO.reg_info0), uShadingTableSVD+sizeof(rTableINTO.reg_mn),
                sizeof(rTableINTO.reg_info0));
        memcpy(&(rTableINTO.reg_info1),
                uShadingTableSVD+sizeof(rTableINTO.reg_mn)+sizeof(rTableINTO.reg_info0),
                sizeof(rTableINTO.reg_info1));
        rTableINTO.src_tbl_addr = (UINT32 *)uShadingTableGain;
        rTableINTO.dst_tbl_addr = uShadingTableCoef;
        mrlscHWTBLCAL(rTableINTO);
#else   
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        LscGaintoHwTblBuf = (float*)malloc(BUFFERSIZE*sizeof(float));
        if(!LscGaintoHwTblBuf)
        {
            ACDK_LOGD("[%s] Allocate LscGaintoHwTblBuf Fail\n", __FUNCTION__);
            mrRet = E_CCT_CALIBRATION_NULL_OBJ; 
            goto Exit;
        }
        u4RetSL2Cal   = LscSL2Calcu(   g_lsc_cali_info.tbl_info,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       rLSCCaliINIParam.u4ImgWidth,
                                       rLSCCaliINIParam.u4ImgHeight);        
        u4DstTblSize = (rLSCCaliINIParam.i4GridXNUM-1)*(rLSCCaliINIParam.i4GridYNUM-1)*16;
        u4RetLSCHwTbl = LscGaintoHWTbl(g_lsc_cali_info.tbl_info.src_tbl_addr_float,
                                       g_lsc_cali_info.tbl_info.dst_tbl_addr,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       m_u2RawImgWidth,
                                       m_u2RawImgHeight,
                                       (void*)LscGaintoHwTblBuf,
                                       BUFFERSIZE);
        if(u4RetLSCHwTbl == E_LSC_CONVERT_BITS_OVERFLOW)
          mrRetHWTbl = E_CCT_CALIBRATION_COEF_OVERFLOW;  
        if(u4RetSL2Cal   == E_LSC_CONVERT_WRONG_INPUT)
          mrRet          =  E_CCT_CALIBRATION_BAD_ARG;       
#endif  

        ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
        memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTable.Length = MAX_SVD_SHADING_SIZE;
        shadingTable.Offset = 0;
        shadingTable.pBuffer = (UINT32 *)&uShadingTableSVD[0];
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;

        ACDK_CCT_TABLE_SET_STRUCT  shadingTablePolyCoef;
        memset (&shadingTablePolyCoef, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTablePolyCoef.Offset = 0;
#if 0 // sammy
        shadingTablePolyCoef.Length = MAX_SHADING_CapTil_SIZE;
        shadingTablePolyCoef.pBuffer = (UINT32 *)uShadingTableCoef;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
#else
        shadingTablePolyCoef.Length = u4DstTblSize;
        shadingTablePolyCoef.pBuffer = (UINT32 *)g_lsc_cali_info.tbl_info.dst_tbl_addr;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
#endif
        shadingTablePolyCoef.Mode = CAMERA_TUNING_CAPTURE_SET;
        if (a_u1FixShadingIndex == 0)
        {
            shadingTable.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                  (UINT8 *)&shadingTable,
                                                  sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                  NULL,
                                                  0,
                                                  &u4RetLen);
            shadingTablePolyCoef.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                  (UINT8 *)&shadingTablePolyCoef,
                                                  sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                  NULL,
                                                  0,
                                                  &u4RetLen);
        }
        else
        {
            int i;
            for (i = 0; i < 4; i++)
            {
                ACDK_LOGD("[%s] color temp %d", __FUNCTION__, i);
                shadingTable.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                        (UINT8 *)&shadingTable,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
                shadingTablePolyCoef.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                        (UINT8 *)&shadingTablePolyCoef,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
            }
        }
#if 0 // sammy
        fclose(fp);
    if(g_raw_bayer0 != NULL)
    {
        free(g_raw_bayer0);
        g_raw_bayer0 = NULL;
    }
    if(LscGaintoHwTblBuf != NULL)
    {
        free(LscGaintoHwTblBuf);
        LscGaintoHwTblBuf = NULL;
    }    
	vLSC_Calibration_END();
#endif
        ACDK_LOGD("[%s] Capture Done\n", __FUNCTION__);
    }

    if ((UINT32)a_i4PreCap == VIDEO_MODE)    //Video Test
    {
        //Store Shading Enable flag before calibration
        memset(&Shading_Control,0,sizeof(i_type));
        Shading_Control.Mode = CAMERA_TUNING_VIDEO_SET;

        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
                (UINT8 *)&Shading_Control,
                sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                (UINT8 *)&Shading_Control,
                sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                &u4RetLen);
        ACDK_LOGD("[AcdkCalibration]"
                "CCAP Get Shading (mode,Enable):(%d,%d)\n"
                ,Shading_Control.Mode
                ,Shading_Control.Enable
        );

        //disable AWB
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);

        //disable Shading compensation before calibration
        memset(&Shading_Calibration_Control,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));
        Shading_Calibration_Control.Mode = CAMERA_TUNING_VIDEO_SET;
        Shading_Calibration_Control.Enable = false;

        uCompMode = CAMERA_TUNING_VIDEO_SET;
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                (UINT8*) &uCompMode,
                sizeof(UINT8),
                (UINT8*)&ACDK_Output,
                sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                &u4RetLen);


        m_u4CapMode = 1; //"0" will do 1/2 subsampe of image
        if (MFALSE == GetShadingRaw(VIDEO_MODE, a_u4Index))
            goto Exit;

        rLSCCaliINIParam.i4GridXNUM = CCT_shading.SHADINGBLK_XNUM + 1;    //a_i4GridXNUM;
        rLSCCaliINIParam.i4GridYNUM = CCT_shading.SHADINGBLK_YNUM + 1;    //a_i4GridYNUM;
        rLSCCaliINIParam.i4AvgWinSize = 33; //User configurable later.
        rLSCCaliINIParam.i4XINIBorder = a_i4XINIBorder;
        rLSCCaliINIParam.i4XENDBorder =  a_i4XENDBorder;
        rLSCCaliINIParam.i4YINIBorder =  a_i4YINIBorder;
        rLSCCaliINIParam.i4YENDBorder = a_i4YENDBorder;
        rLSCCaliINIParam.u4ImgWidth = m_u2RawImgWidth;
        rLSCCaliINIParam.u4ImgHeight = m_u2RawImgHeight;
        rLSCCaliINIParam.u2BayerStart = (UINT16)m_eColorOrder;
        rLSCCaliINIParam.poly_coef.coef_a = (float)ShadingATNTable[a_u2AttnRatio][0] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_b = (float)ShadingATNTable[a_u2AttnRatio][1] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_c = (float)ShadingATNTable[a_u2AttnRatio][2] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_d = (float)ShadingATNTable[a_u2AttnRatio][3] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_e = (float)ShadingATNTable[a_u2AttnRatio][4] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_f = 1;
        
        g_raw_bayer0 = (unsigned short*)malloc(m_u2RawImgWidth*m_u2RawImgHeight*2*sizeof(MUINT8));
        if(!g_raw_bayer0)
        {
             ACDK_LOGD(" Mem allocation Fail \n");
             mrRet = E_CCT_CALIBRATION_NULL_OBJ;  
             goto Exit;
        }
        vLsc_Remap_to_Bayer0((unsigned short*)m_pucRawBuf, g_raw_bayer0, m_u2RawImgWidth, m_u2RawImgHeight, rLSCCaliINIParam.u2BayerStart);


        ACDK_LOGD("GridXNUM:%d\n", rLSCCaliINIParam.i4GridXNUM);
        ACDK_LOGD("GridYNUM:%d\n", rLSCCaliINIParam.i4GridYNUM);

        ACDK_LOGD(" Attenuation Poly coef :%8f, %8f, %8f, %8f ,%8f, %8f\n",
                rLSCCaliINIParam.poly_coef.coef_a,rLSCCaliINIParam.poly_coef.coef_b,rLSCCaliINIParam.poly_coef.coef_c,rLSCCaliINIParam.poly_coef.coef_d,rLSCCaliINIParam.poly_coef.coef_e,rLSCCaliINIParam.poly_coef.coef_f);

        if (a_u2AttnRatio == 0)
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 0;
        }
        else
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 1;
        }

        vLSC_PARAM_INIT(rLSCCaliINIParam);
        vLSC_Calibration_INIT(g_raw_bayer0);

        mrLSC_Calibrate(g_lsc_cali_info, 2 /* VIDEO MODE */, u2SVDTermNum);//video
        ACDK_LOGD(" mrLSC_Calibrate Done!\n");

        if(g_SAVE_IMG)
        {
            char fileName[100];
            sprintf(fileName, "%s//vdoraw.raw", MEDIA_PATH);

            ACDK_LOGD(" Save Video RAW Image \n");
            ACDK_LOGD(" Save File Name:%s\n", fileName);
            ACDK_LOGD(" Width:%d\n", m_u2RawImgWidth);
            ACDK_LOGD(" Height:%d\n", m_u2RawImgHeight);

            FILE *pRawFp = fopen(fileName, "wb");

            if (NULL == pRawFp )
            {
                ACDK_LOGE("Can't open file to save Image\n");
                mrRet =E_CCT_CALIBRATION_FILE_OPEN_FAIL;
                goto Exit;
            }

            INT32 i4WriteCnt = fwrite(m_pucRawBuf,1, (m_u2RawImgWidth* m_u2RawImgHeight* 1),pRawFp);
            ACDK_LOGD("Raw done!!\n");

            fflush(pRawFp);
            fclose(pRawFp);

            sync();
        }

#if 0 // sammy
        //!Apply parameter ISP
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        UINT16 uShadingTableGain[MAX_SHADING_SIZE]={0}; // buffer size > rLSCCaliINIParam.i4GridXNUM*rLSCCaliINIParam.i4GridYNUM*4
        UINT32 uShadingTableCoef[MAX_SHADING_SIZE]={0};
        FILE *fp = fopen("//data//Video_LSC.bin", "rb"); //hwtbl.bin
        //FILE *fp = fopen("//data//hwtbl.bin", "rb"); //hwtbl.bin
        if (fp == NULL)
        {
            ACDK_LOGE("ERROR to open Calibration data \n");
            goto Exit;
        }

        ACDK_LOGE("open Calibration data %d byte\n", MAX_SVD_SHADING_SIZE);
        fread(uShadingTableSVD,  MAX_SVD_SHADING_SIZE, 1,  fp);

        lscReConstruct(rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM*4, uShadingTableSVD, uShadingTableGain);
        //calculate hw table
        memcpy(&(rTableINTO.reg_mn), uShadingTableSVD,sizeof(rTableINTO.reg_mn));
        memcpy(&(rTableINTO.reg_info0), uShadingTableSVD+sizeof(rTableINTO.reg_mn),
                sizeof(rTableINTO.reg_info0));
        memcpy(&(rTableINTO.reg_info1),
                uShadingTableSVD+sizeof(rTableINTO.reg_mn)+sizeof(rTableINTO.reg_info0),
                sizeof(rTableINTO.reg_info1));
        rTableINTO.src_tbl_addr = (UINT32 *)uShadingTableGain;
        rTableINTO.dst_tbl_addr = uShadingTableCoef;
        mrlscHWTBLCAL(rTableINTO);
#else    
        UINT8 uShadingTableSVD[MAX_SVD_SHADING_SIZE]={0};
        LscGaintoHwTblBuf = (float*)malloc(BUFFERSIZE*sizeof(float));
        if(!LscGaintoHwTblBuf)
        {
            ACDK_LOGD("[%s] Allocate LscGaintoHwTblBuf Fail\n", __FUNCTION__);
            mrRet = E_CCT_CALIBRATION_NULL_OBJ; 
            goto Exit;
        }
        u4RetSL2Cal   = LscSL2Calcu(   g_lsc_cali_info.tbl_info,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       rLSCCaliINIParam.u4ImgWidth,
                                       rLSCCaliINIParam.u4ImgHeight);
        u4DstTblSize = (rLSCCaliINIParam.i4GridXNUM-1)*(rLSCCaliINIParam.i4GridYNUM-1)*16;    
        u4RetLSCHwTbl = LscGaintoHWTbl(g_lsc_cali_info.tbl_info.src_tbl_addr_float,
                                       g_lsc_cali_info.tbl_info.dst_tbl_addr,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       m_u2RawImgWidth,
                                       m_u2RawImgHeight,
                                       (void*)LscGaintoHwTblBuf,
                                       BUFFERSIZE);
        if(u4RetLSCHwTbl == E_LSC_CONVERT_BITS_OVERFLOW)
          mrRetHWTbl = E_CCT_CALIBRATION_COEF_OVERFLOW;  
        if(u4RetSL2Cal   == E_LSC_CONVERT_WRONG_INPUT)
          mrRet          =  E_CCT_CALIBRATION_BAD_ARG;       
#endif  

        ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
        memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTable.Length = MAX_SVD_SHADING_SIZE;
        shadingTable.Offset = 0;
        shadingTable.pBuffer = (UINT32 *)&uShadingTableSVD[0];
        shadingTable.Mode = CAMERA_TUNING_VIDEO_SET;

        ACDK_CCT_TABLE_SET_STRUCT  shadingTablePolyCoef;
        memset (&shadingTablePolyCoef, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTablePolyCoef.Offset = 0;
#if 0 // sammy
        shadingTablePolyCoef.Length = MAX_SHADING_VdoFrm_SIZE;
        shadingTablePolyCoef.pBuffer = (UINT32 *)uShadingTableCoef;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
#else
        shadingTablePolyCoef.Length = u4DstTblSize;
        shadingTablePolyCoef.pBuffer = (UINT32 *)g_lsc_cali_info.tbl_info.dst_tbl_addr;
#endif
        shadingTablePolyCoef.Mode = CAMERA_TUNING_VIDEO_SET;
        if (a_u1FixShadingIndex == 0)
        {
            shadingTable.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                    (UINT8 *)&shadingTable,
                    sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                    NULL,
                    0,
                    &u4RetLen);
            shadingTablePolyCoef.ColorTemp = a_u4Index;
            CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                    (UINT8 *)&shadingTablePolyCoef,
                    sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                    NULL,
                    0,
                    &u4RetLen);
        }
        else
        {
            int i;
            for (i = 0; i < 4; i++)
            {
                ACDK_LOGD("[%s] color temp %d", __FUNCTION__, i);
                shadingTable.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                        (UINT8 *)&shadingTable,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
                shadingTablePolyCoef.ColorTemp = i;
                CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                        (UINT8 *)&shadingTablePolyCoef,
                        sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                        NULL,
                        0,
                        &u4RetLen);
            }
        }
#if 0 // sammy
        fclose(fp);
    if(g_raw_bayer0 != NULL)
    {
        free(g_raw_bayer0);
        g_raw_bayer0 = NULL;
    }
    if(LscGaintoHwTblBuf != NULL)
    {
        free(LscGaintoHwTblBuf);
        LscGaintoHwTblBuf = NULL;
    }
	vLSC_Calibration_END();
#endif
        ACDK_LOGD("[%s] Video Done\n", __FUNCTION__);
    }
Exit:
    ACDK_LOGD("[%s] Exit:\n", __FUNCTION__);
    //Enable AE
    ACDK_LOGD("[%s] Exit:ACDK_CCT_OP_AE_\n", __FUNCTION__);
    if (u4AEEnable == 1)
    {
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
    }
    else
    {
        CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    }
    ACDK_LOGD("[%s] Exit:ACDK_CCT_V2_OP_AWB_\n", __FUNCTION__);
    //disable AWB
    if (i4AWBEnable == 1)
    {
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    }
    else
    {
        CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
    }


    //restore Shading compensation on/off setting
    memset(&Shading_Calibration_Control,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));
    Shading_Calibration_Control.Mode = Shading_Control.Mode;
    Shading_Calibration_Control.Enable = Shading_Control.Enable;
    ACDK_LOGD("[%s] Exit:ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF!!\n", __FUNCTION__);

    //disable Calibration
    //u4CaliMode = 0;
    //CCTIF_FeatureCtrl(ACDK_CCT_OP_SET_CALI_MODE, (UINT8*)&u4CaliMode, sizeof(UINT32), NULL, 0,  &u4RetLen);

    //restore color temperature index change by AWB 2nd flow, this index is not used when capture raw, but will use in later actions.
    ACDK_LOGD("[%s] Exit:ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX!!\n", __FUNCTION__);
    CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
                                                        (UINT8*)&index,
                                                        sizeof(UINT8),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


#define ACDK_LOGD(fmt,arg...)      XLOGD("[%s] " fmt, __func__, ##arg)
    //m_eRawFormat = TWO_BYTE_RAW;

    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf =NULL;
    }

    {
#if ENABLE_TSF
        MBOOL bTSF = isEnableTSF(LscMgr::getInstance()->getActiveSensorDev());
        if (bTSF == MTRUE) {
            ACDK_LOGE(" [%s] set LSCMGR_TSF_STATE_IDLE!!\n", __FUNCTION__);
            LscMgr::getInstance()->changeTSFState(NSIspTuning::LscMgr::LSCMGR_TSF_STATE_IDLE);
            ACDK_LOGE(" [%s] set LSCMGR_TSF_STATE_INIT!!\n", __FUNCTION__);
            LscMgr::getInstance()->changeTSFState(NSIspTuning::LscMgr::LSCMGR_TSF_STATE_INIT);
        } else
            ACDK_LOGE(" [%s] bTSF = MFALSE!!\n", __FUNCTION__);
#endif
    }

    // Disble Bypass Dynamic
    //NSIspTuning::IspTuningMgr::getInstance().setOperMode(NSIspTuning::EOperMode_Meta);
    NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MTRUE);

    mrRet = (0 == m_pAcdkBaseObj->startPreview(NULL) ? S_CCT_CALIBRATION_OK : E_CCT_CALIBRATION_API_FAIL);

    ACDK_LOGD("Exit:Lens Shading Calibration Finish !! mrRetHWTbl=%d, mrRet=%d\n", mrRetHWTbl, mrRet);

    if(g_raw_bayer0 != NULL)
    {
        free(g_raw_bayer0);
        g_raw_bayer0 = NULL;
    }
    if(LscGaintoHwTblBuf != NULL)
    {
        free(LscGaintoHwTblBuf);
        LscGaintoHwTblBuf = NULL;
    }
	vLSC_Calibration_END();

    return ((E_CCT_CALIBRATION_COEF_OVERFLOW  == mrRetHWTbl) ?
        E_CCT_CALIBRATION_COEF_OVERFLOW  : mrRet);
}

/////////////////////////////////////////////////////////////////////////
//
//   vCalc1DPoly () -
//!  @brief Calculate the 1D polyminal coefficient
//!  @param a_i4DataLen: The input data length
//!  @param a_pdXVal: The input X data value
//!  @param a_pdYVal: The input Y data value
//!  @param a_pdParamA: The output coefficient A
//!  @param a_pdParamB: The output coefficient B
/////////////////////////////////////////////////////////////////////////
VOID AcdkCalibration::vCalc1DPoly(INT32 a_i4DataLen, INT16 *a_pdXVal, INT16 *a_pdYVal, DOUBLE &a_pdParamA, DOUBLE &a_pdParamB)
{
    DOUBLE dSumX = 0.0;
    DOUBLE dSumSqureX = 0.0;
    DOUBLE dSumY = 0.0;
    DOUBLE dTemp = 0.0;
    DOUBLE dSumXxY = 0.0;

	for (INT32 i = 0; i < a_i4DataLen; i++)
	{
		dSumX += (DOUBLE)a_pdXVal[i];
		dSumSqureX += ((DOUBLE)a_pdXVal[i] * (DOUBLE)a_pdXVal[i]);
		dSumY += (DOUBLE)a_pdYVal[i];
		dSumXxY += ((DOUBLE)a_pdXVal[i] * (DOUBLE)a_pdYVal[i]);
	}

	dTemp= (DOUBLE)a_i4DataLen * dSumSqureX - dSumX * dSumX;


	DOUBLE dTempParmA = 0;
	DOUBLE dTempParmB = 0;

	dTempParmA = (DOUBLE)a_i4DataLen * dSumXxY - dSumX * dSumY;
	dTempParmB =  dSumY * dSumSqureX - dSumX * dSumXxY;


	a_pdParamA = dTempParmA / dTemp;
	a_pdParamB = dTempParmB / dTemp;

}


VOID AcdkCalibration::vSaveImg(char *a_pFileName, MUINT8 *a_pucBuf, UINT16 a_u2ImgWidth, UINT16 a_u2ImgHeight)
{
    ACDK_LOGD(" Save File Name:%s\n", a_pFileName);

#if 0
    BMQFile *pBMQFile = new BMQFile(a_pFileName);

    MRESULT mrRet = pBMQFile->buildFile(a_pucBuf, a_u2ImgWidth, a_u2ImgHeight);
    if(FAILED(mrRet))
    {
        ACDK_LOGE("\n Save Capture RAW Image Failed\n");
    }
    else
    {
        ACDK_LOGD(" Width:%d\n", m_u2RawImgWidth);
        ACDK_LOGD(" Height:%d\n", m_u2RawImgHeight);
     }
    delete pBMQFile;
#else
    FILE *pFp = fopen(a_pFileName, "wb");

    if (NULL == pFp ) {
        ACDK_LOGD("Can't open file to save Image\n");
    }

    INT32 i4WriteCnt = fwrite(a_pucBuf, 1, a_u2ImgWidth * a_u2ImgHeight  * 2 , pFp);

    ACDK_LOGD("Save image file name:%s\n", a_pFileName);

    fclose(pFp);

    ACDK_LOGD("mrSaveRAWImg() - X \n");
#endif
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkCalibration::sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen
)
{
    MINT32 fgRet = S_CCT_CALIBRATION_OK;
    typedef ACDK_CCT_LSC_CAL_SET_STRUCT i_type;
    i_type* const pShadingCalSet = reinterpret_cast<i_type*> (puParaIn);

    ACDK_CDVT_SENSOR_TEST_INPUT_T *prSensorTestInput = (ACDK_CDVT_SENSOR_TEST_INPUT_T *)puParaIn;
    ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput = (ACDK_CDVT_SENSOR_TEST_OUTPUT_T *)puParaOut;

    ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T *prSensorCalibrationInput = (ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T *)puParaIn;
    ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput = (ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *)puParaOut;

    ACDK_CDVT_AE_PLINE_TEST_INPUT_T *prSensorAEPlineTableInput = (ACDK_CDVT_AE_PLINE_TEST_INPUT_T *)puParaIn;
    ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T *prSensorAEPlineTableOutput = (ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T *)puParaOut;
    
    switch (a_u4Ioctl)
    {
        case ACDK_CCT_OP_AE_PLINE_TABLE_TEST:
            ACDK_LOGD("ACDK_CCT_OP_AE_PLINE_TABLE_TEST\n");
            mrAEPlineTableLinearity(prSensorAEPlineTableInput, u4ParaInLen, prSensorAEPlineTableOutput, u4ParaOutLen, pu4RealParaOutLen);
            break;

        case ACDK_CCT_OP_FLASH_CALIBRATION:
            ACDK_LOGD("ACDK_CCT_OP_FLASH_CALIBRATION line=%d \n", __LINE__);
            flashCalibration((void*)puParaIn, u4ParaInLen, (void*)puParaOut, u4ParaOutLen, pu4RealParaOutLen);
            break;

        case ACDK_CCT_V2_OP_SHADING_CAL:
            //typedef ACDK_CCT_LSC_CAL_SET_STRUCT i_type;
            if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
                return  E_CCT_CALIBRATION_BAD_ARG;

            ACDK_LOGD("[+AcdkCalibration]"
                    "ACDK_CCT_V2_OP_SHADING_CAL (mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
                    ,pShadingCalSet->mode
                    ,pShadingCalSet->colorTemp
            );

            if (pShadingCalSet->mode == CAMERA_TUNING_PREVIEW_SET)
            {
                ACDK_LOGD("Cal Shading for preview xstart %d ystart %d xend %d yend %d ratio %d color index %d\n", pShadingCalSet->boundaryStartX,
                        pShadingCalSet->boundaryStartY, pShadingCalSet->boundaryEndX, pShadingCalSet->boundaryEndY, pShadingCalSet->attnRatio, pShadingCalSet->colorTemp);
                fgRet =  mrCalLenShading(pShadingCalSet->boundaryStartX
                        ,pShadingCalSet->boundaryStartY
                        ,pShadingCalSet->boundaryEndX
                        ,pShadingCalSet->boundaryEndY
                        ,pShadingCalSet->attnRatio
                        ,pShadingCalSet->colorTemp
                        ,PREVIEW_MODE
                        ,pShadingCalSet->u1FixShadingIndex
                );

            }
            else if (pShadingCalSet->mode == CAMERA_TUNING_CAPTURE_SET)
            {
                ACDK_LOGD("Cal Shading for capture xstart %d ystart %d xend %d yend %d ratio %d color index %d\n", pShadingCalSet->boundaryStartX,
                        pShadingCalSet->boundaryStartY, pShadingCalSet->boundaryEndX, pShadingCalSet->boundaryEndY, pShadingCalSet->attnRatio, pShadingCalSet->colorTemp);
                fgRet =  mrCalLenShading(pShadingCalSet->boundaryStartX
                        ,pShadingCalSet->boundaryStartY
                        ,pShadingCalSet->boundaryEndX
                        ,pShadingCalSet->boundaryEndY
                        ,pShadingCalSet->attnRatio
                        ,pShadingCalSet->colorTemp
                        ,CAPTURE_MODE
                        ,pShadingCalSet->u1FixShadingIndex
                );

            }
            else if (pShadingCalSet->mode == CAMERA_TUNING_VIDEO_SET)
            {
                ACDK_LOGD("Cal Shading for video xstart %d ystart %d xend %d yend %d ratio %d color index %d\n", pShadingCalSet->boundaryStartX,
                        pShadingCalSet->boundaryStartY, pShadingCalSet->boundaryEndX, pShadingCalSet->boundaryEndY, pShadingCalSet->attnRatio, pShadingCalSet->colorTemp);
                fgRet =  mrCalLenShading(pShadingCalSet->boundaryStartX
                        ,pShadingCalSet->boundaryStartY
                        ,pShadingCalSet->boundaryEndX
                        ,pShadingCalSet->boundaryEndY
                        ,pShadingCalSet->attnRatio
                        ,pShadingCalSet->colorTemp
                        ,VIDEO_MODE
                        ,pShadingCalSet->u1FixShadingIndex
                );

            }
            else
            {
                ACDK_LOGD("Erro Cal mode:%d \n", pShadingCalSet->mode);
                fgRet =  E_CCT_CALIBRATION_BAD_ARG;
            }
            break;

        case ACDK_CCT_OP_CDVT_SENSOR_TEST:
            ACDK_LOGD("=========================================\n");
            ACDK_LOGD("eTestItem = %d\n",prSensorTestInput->eTestItem);
            ACDK_LOGD("eSensorMode = %d\n",prSensorTestInput->eSensorMode);
            ACDK_LOGD("rExpLinearity.i4Gain = %d\n",prSensorTestInput->rExpLinearity.i4Gain);
            ACDK_LOGD("rExpLinearity.eExpMode = %d\n",prSensorTestInput->rExpLinearity.eExpMode);
            ACDK_LOGD("rExpLinearity.i4ExpStart = %d\n",prSensorTestInput->rExpLinearity.i4ExpStart);
            ACDK_LOGD("rExpLinearity.i4ExpEnd = %d\n",prSensorTestInput->rExpLinearity.i4ExpEnd);
            ACDK_LOGD("rExpLinearity.i4ExpInterval = %d\n",prSensorTestInput->rExpLinearity.i4ExpInterval);
            ACDK_LOGD("rGainLinearityOBStability.eGainControlMode = %d\n",prSensorTestInput->rGainLinearityOBStability.eGainControlMode);
            ACDK_LOGD("rGainLinearityOBStability.i4ExpTime = %d\n",prSensorTestInput->rGainLinearityOBStability.i4ExpTime);
            ACDK_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainStart = %d\n",prSensorTestInput->rGainLinearityOBStability.i4GainStart);
            ACDK_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainEnd = %d\n",prSensorTestInput->rGainLinearityOBStability.i4GainEnd);
            ACDK_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainInterval = %d\n",prSensorTestInput->rGainLinearityOBStability.i4GainInterval);
            ACDK_LOGD("=========================================\n");


            switch (prSensorTestInput->eTestItem)
            {
                case ACDK_CDVT_TEST_EXPOSURE_LINEARITY:
                      mrExpLinearity(prSensorTestInput->rExpLinearity.i4Gain,
                                               (INT32)prSensorTestInput->rExpLinearity.eExpMode,
                                                prSensorTestInput->rExpLinearity.i4ExpStart,
                                                prSensorTestInput->rExpLinearity.i4ExpEnd,
                                                prSensorTestInput->rExpLinearity.i4ExpInterval,
                                                (INT32)prSensorTestInput->eSensorMode,
                                                prSensorTestOutput);
                    break;
                case ACDK_CDVT_TEST_GAIN_LINEARITY:
                    if (prSensorTestInput->rGainLinearityOBStability.eGainControlMode == ACDK_CDVT_GAIN_CONFIG)
                    {
                         mrGainLinearity(prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                  prSensorTestInput->rGainLinearityOBStability.i4GainStart,
                                                  prSensorTestInput->rGainLinearityOBStability.i4GainEnd,
                                                  prSensorTestInput->rGainLinearityOBStability.i4GainInterval,
                                                  (INT32)prSensorTestInput->eSensorMode,
                                                  prSensorTestOutput);
                    }
                    else // ACDK_CDVT_GAIN_TABLE
                    {
                          mrGainTableLinearity(prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                            prSensorTestInput->rGainLinearityOBStability.i4GainTableSize,
                                                            prSensorTestInput->rGainLinearityOBStability.i4GainTable,
                                                            (INT32)prSensorTestInput->eSensorMode,
                                                            prSensorTestOutput);
                    }
                    break;
                case ACDK_CDVT_TEST_OB_STABILITY:
                    if (prSensorTestInput->rGainLinearityOBStability.eGainControlMode == ACDK_CDVT_GAIN_CONFIG)
                    {
                         mrOBStability(prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainStart,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainEnd,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainInterval,
                                                (INT32)prSensorTestInput->eSensorMode,
                                                prSensorTestOutput);
                    }
                    else // ACDK_CDVT_GAIN_TABLE
                    {
                        mrGainTableOBStability(prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                              prSensorTestInput->rGainLinearityOBStability.i4GainTableSize,
                                                              prSensorTestInput->rGainLinearityOBStability.i4GainTable,
                                                              (INT32)prSensorTestInput->eSensorMode,
                                                              prSensorTestOutput);
                    }
                    break;
            }

            ACDK_LOGD("=========================================\n");
            ACDK_LOGD("i4ErrorCode = %d\n",prSensorTestOutput->i4ErrorCode);
            ACDK_LOGD("i4TestCount = %d\n",prSensorTestOutput->i4TestCount);
            for (INT32 i=0; i<prSensorTestOutput->i4TestCount; i++)
            {
                ACDK_LOGD("[%d] %4.2f %4.2f %4.2f %4.2f %d \n",
                                        i,
                                        prSensorTestOutput->rRAWAnalysisResult[i].fRAvg,
                                        prSensorTestOutput->rRAWAnalysisResult[i].fGrAvg,
                                        prSensorTestOutput->rRAWAnalysisResult[i].fGbAvg,
                                        prSensorTestOutput->rRAWAnalysisResult[i].fBAvg,
                                        prSensorTestOutput->rRAWAnalysisResult[i].u4Median);
            }
            ACDK_LOGD("=========================================\n");
            //return S_ACDK_CALIBRATION_OK;
            break;
        case ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION:
            ACDK_LOGD("=========================================\n");
            ACDK_LOGD("eCalibrationItem = %d\n",prSensorCalibrationInput->eCalibrationItem);
            ACDK_LOGD("eSensorMode = %d\n",prSensorCalibrationInput->eSensorMode);
            ACDK_LOGD("rOB.i4ExpTime = %d\n",prSensorCalibrationInput->rOB.i4ExpTime);
            ACDK_LOGD("rOB.i4Gain = %d\n",prSensorCalibrationInput->rOB.i4Gain);
            ACDK_LOGD("rOB.i4RepeatTimes = %d\n",prSensorCalibrationInput->rOB.i4RepeatTimes);
            ACDK_LOGD("rMinISO.i4LV = %d\n",prSensorCalibrationInput->rMinISO.i4LV);
            ACDK_LOGD("rMinISO.i4FNumber = %d\n",prSensorCalibrationInput->rMinISO.i4FNumber);
            ACDK_LOGD("rMinISO.eFlicker = %d\n",prSensorCalibrationInput->rMinISO.eFlicker);
            ACDK_LOGD("rMinISO.i4OB = %d\n",prSensorCalibrationInput->rMinISO.i4OB);
            ACDK_LOGD("rMinSatGain.i4TargetDeclineRate = %d\n",prSensorCalibrationInput->rMinSatGain.i4TargetDeclineRate);
            ACDK_LOGD("rMinSatGain.i4GainBuffer = %d\n",prSensorCalibrationInput->rMinSatGain.i4GainBuffer);
            ACDK_LOGD("rMinSatGain.eFlicker = %d\n",prSensorCalibrationInput->rMinSatGain.eFlicker);
            ACDK_LOGD("rMinSatGain.i4OB = %d\n",prSensorCalibrationInput->rMinSatGain.i4OB);
            ACDK_LOGD("=========================================\n");

            switch (prSensorCalibrationInput->eCalibrationItem)
            {
                case ACDK_CDVT_CALIBRATION_OB:
                    mrCalOB(prSensorCalibrationInput->rOB.i4ExpTime,
                                   prSensorCalibrationInput->rOB.i4Gain,
                                   prSensorCalibrationInput->rOB.i4RepeatTimes,
                                   (INT32)prSensorCalibrationInput->eSensorMode,
                                   prSensorCalibrationOutput);

                    break;
                case ACDK_CDVT_CALIBRATION_MIN_ISO:
                    mrCalMinISO(prSensorCalibrationInput->rMinISO.i4LV,
                                         prSensorCalibrationInput->rMinISO.i4FNumber,
                                         prSensorCalibrationInput->rMinISO.i4OB,
                                         (INT32)prSensorCalibrationInput->rMinISO.eFlicker,
                                         (INT32)prSensorCalibrationInput->eSensorMode,
                                         prSensorCalibrationOutput);
                    break;
                case ACDK_CDVT_CALIBRATION_MIN_SAT_GAIN:
                    mrCalMinimumSaturationGain(prSensorCalibrationInput->rMinSatGain.i4TargetDeclineRate,
                                                                    prSensorCalibrationInput->rMinSatGain.i4GainBuffer,
                                                                    prSensorCalibrationInput->rMinSatGain.i4OB,
                                                                    (INT32)prSensorCalibrationInput->rMinISO.eFlicker,
                                                                   (INT32)prSensorCalibrationInput->eSensorMode,
                                                                    prSensorCalibrationOutput);
                    break;
            }

            ACDK_LOGD("=========================================\n");
            ACDK_LOGD("i4ErrorCode = %d\n",prSensorCalibrationOutput->i4ErrorCode);
            ACDK_LOGD("i4MinISO = %d\n",prSensorCalibrationOutput->i4MinISO);
            ACDK_LOGD("i4MinSatGain = %d\n",prSensorCalibrationOutput->i4MinSatGain);
            ACDK_LOGD("i4OB = %d\n",prSensorCalibrationOutput->i4OB);
            ACDK_LOGD("=========================================\n");
            //return S_ACDK_CALIBRATION_OK;
            break;
        default:
            ACDK_LOGE("[sendcommand] Error cmds = 0x%x\n", a_u4Ioctl);
            fgRet = E_CCT_CALIBRATION_BAD_ARG;
            break;
    }

    return fgRet;
}
