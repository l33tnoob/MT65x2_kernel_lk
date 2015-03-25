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
// AcdkCalibration.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCalibration.h
//! \brief

#ifndef _ACDKCALIBRATION_H_
#define _ACDKCALIBRATION_H_


#include "mtkcam/acdk/AcdkCommon.h"
#include "cct_imgtool.h"
#include "AcdkBase.h"
#include <mtkcam/acdk/cct_feature.h>


typedef struct SensitivityResult_t
{
    UINT16 u2LowLux;
    UINT16 u2HightLux;
    UINT16 u2LowExpTime;
    UINT16 u2HightExpTime;
}SensitivityResult;

typedef struct BadPixelParm_t
{
    UINT16 u2Start;
    UINT16 u2TransientStart;
    UINT16 u2EfficientStart;
    UINT16 u2WholeSize;
    UINT16 u2TransientSize;
    UINT16 u2EfficientSize;
}BadPixelParm;

typedef struct LSCActiveWin_t
{
    UINT16 u2Top;
    UINT16 u2Bottom;
    UINT16 u2Left;
    UINT16 u2Right;
}LSCActiveWin;


enum BAD_PIXEL_MODE
{
   DARK_PIXEL        = 0,
   WHITE_PIXEL       = 1
};

enum RAW_FORMAT
{
    ONE_BYTE_RAW = 0,
    TWO_BYTE_RAW =1
};
namespace NSACDK {

class AcdkCalibration /*: public AcdkBase*/
{
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//1   Public
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
public:

    //
    MBOOL m_bLSCEnable;
    MBOOL m_bBPCEnable;
    UINT32 m_u4BadPixelCnt;

    /////////////////////////////////////////////////////////////////////////
    //
    //   AcdkCalibration () -
    //!
    //!  brief AcdkCalibration module constructor
    //
    /////////////////////////////////////////////////////////////////////////
    AcdkCalibration();

    /////////////////////////////////////////////////////////////////////////
    //
    //   ~AcdkCalibration () -
    //!
    //!  brief AcdkCalibration module destructor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~AcdkCalibration();


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrInitAcdkCalibration () -
    //!  brief init the ACDK calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT init(AcdkBase *a_pAcdkBaseObj);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrInitAcdkCalibration () -
    //!  brief init the ACDK calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT uninit();


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrAnalyzeRAWImage () -
    //!  brief analyze RAW image
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrAnalyzeRAWImage();

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalculateSlope () -
    //!  brief calculate slope
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalculateSlope(DOUBLE a_dX0,
                             DOUBLE a_dY0,
                             DOUBLE a_dX1,
                             DOUBLE a_dY1,
                             DOUBLE a_dX2,
                             DOUBLE a_dY2,
                             DOUBLE a_dX3,
                             DOUBLE a_dY3,
                             DOUBLE &a_dSlope);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrExpLinearity () -
    //!  brief exposure time linearity test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrExpLinearity(INT32 a_i4Gain,
                           INT32 a_i4ExpMode,
                           INT32 a_i4ExpStart,
                           INT32 a_i4ExpEnd,
                           INT32 a_i4ExpInterval,
                           INT32 a_i4PreCap,
                           ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainLinearity () -
    //!  @brief sensor gain linearity test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainLinearity(INT32 a_i4ExpTime,
                                                        INT32 a_i4GainStart,
                                                        INT32 a_i4GainEnd,
                                                        INT32 a_i4GainInterval,
                                                        INT32 a_i4PreCap,
                                                        ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainTableLinearity () -
    //!  @brief sensor gain linearity test (gain table)
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainTableLinearity(INT32 a_i4ExpTime,
                                 INT32 a_i4GainTableSize,
                                 INT32 *a_pi4GainTable,
                                 INT32 a_i4PreCap,
                                 ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrOBStability () -
    //!  brief OB stability test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrOBStability(INT32 a_i4ExpTime,
                                                   INT32 a_i4GainStart,
                                                   INT32 a_i4GainEnd,
                                                   INT32 a_i4GainInterval,
                                                   INT32 a_i4PreCap,
                                                   ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainTableOBStability () -
    //!  brief OB stability test (gain table)
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainTableOBStability(INT32 a_i4ExpTime,
                                   INT32 a_i4GainTableSize,
                                   INT32 *a_pi4GainTable,
                                   INT32 a_i4PreCap,
                                   ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalOB () -
    //!  brief OB calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalOB(INT32 a_i4ExpTime,
                    INT32 a_i4Gain,
                    INT32 a_i4RepeatTimes,
                    INT32 a_i4PreCap,
                    ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalMinISO () -
    //!  brief minimum ISO calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalMinISO(INT32 a_i4LV,
                        INT32 a_i4FNo,
                        INT32 a_i4OBLevel,
                        INT32 a_i450Hz60Hz,
                        INT32 a_i4PreCap,
                        ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalMinimumSaturationGain () -
    //!  brief minimum saturation gain calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalMinimumSaturationGain(INT32 a_i4TargetDeclineRate,
                                       INT32 a_i4GainBuffer,
                                       INT32 a_i4OBLevel,
                                       INT32 a_i450Hz60Hz,
                                       INT32 a_i4PreCap,
                                       ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalLenShading () -
    //!  @brief calibrate lens shading  test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalLenShading(INT32 a_i4XINIBorder,
                            INT32 a_i4XENDBorder,
                            INT32 a_i4YINIBorder,
                            INT32 a_i4YENDBorder,
                            UINT16 a_u2AttnRatio,
                            UINT32 a_u4Index,
                            INT32 a_i4PreCap,
                            UINT8 a_u1FixShadingIndex);


    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalBadPixel () -
    //!  @brief calibrate lens shading  test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalBadPixel(INT32 a_i4AnaGain,
                                INT32 a_i4ExpTime,
                                INT32 a_i4PreCap);



    VOID vCaptureCallBack(VOID *a_pParam);

    virtual MINT32 sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen
    );

    int flashCalibration(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
    MRESULT mrAEPlineTableLinearity(ACDK_CDVT_AE_PLINE_TEST_INPUT_T* in, int inSize, ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T* out, int outSize, MUINT32* realOutSize);
    MBOOL GetShadingRaw(eACDK_CAMERA_MODE mode, UINT8 ColorTemp);


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//1   Protected
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
protected:

    /////////////////////////////////////////////////////////////////////////
    //
    //   vCalc1DPoly () -
    //!  brief Calculate the 1D polyminal coefficient
    /////////////////////////////////////////////////////////////////////////
    VOID vCalc1DPoly(INT32 a_i4DataLen, INT16 *a_pdXVal, INT16 *a_pdYVal, DOUBLE &a_pdParamA, DOUBLE &a_pdParamB);

    VOID vSaveImg(char *a_pFileName, MUINT8 *a_pucBuf, UINT16 a_u2ImgWidth, UINT16 a_u2ImgHeight);


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//1   Private
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
private:
    MBOOL m_bAcdkCalibration;
    UINT32 m_u4CapMode;
    AcdkImgTool* m_pAcdkImgToolObj;
	AcdkBase *m_pAcdkBaseObj;
	MBOOL m_bCapDone;

    UINT16 m_u2RawImgWidth;
    UINT16 m_u2RawImgHeight;
    UINT8 *m_pucRawBuf;
    RAW_FORMAT m_eRawFormat;
    eRAW_ColorOrder m_eColorOrder;

	MRESULT takePicture(MUINT32 a_i4PreCap, eACDK_CAP_FORMAT type);	//take picture & back to preview

};
}

#endif
