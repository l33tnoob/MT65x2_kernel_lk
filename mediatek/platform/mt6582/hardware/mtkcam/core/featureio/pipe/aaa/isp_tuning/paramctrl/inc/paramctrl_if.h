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
#ifndef _PARAMCTRL_IF_H_
#define _PARAMCTRL_IF_H_

namespace NSIspTuning
{

class IParamctrl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    IParamctrl(IParamctrl const&);
    //  Copy-assignment operator is disallowed.
    IParamctrl& operator=(IParamctrl const&);

protected:
    IParamctrl(ESensorDev_T const eSensorDev) {}
    virtual ~IParamctrl() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Instance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IParamctrl* createInstance(ESensorDev_T const eSensorDev);
    virtual MVOID destroyInstance() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public: // Dynamic Tuning
    virtual MVOID       enableDynamicTuning(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicTuning() const = 0;

    virtual MVOID       enableDynamicCCM(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicCCM() const = 0;

	virtual MVOID   	enableDynamicBypass(MBOOL const fgEnable) = 0;
	virtual MBOOL   	isDynamicBypass() const = 0;

    virtual MVOID   	enableDynamicShading(MBOOL const fgEnable) = 0;
	virtual MBOOL   	isDynamicShading() const = 0;

    //virtual MVOID       updateShadingNVRAMdata(MBOOL const fgEnable) = 0;
    //virtual MBOOL       isShadingNVRAMdataChange() const  = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     // Attributes
    virtual MERROR_ENUM  setIspProfile(EIspProfile_T const eIspProfile) = 0;
    virtual MERROR_ENUM  setSceneMode(EIndex_Scene_T const eScene) = 0;
    virtual MERROR_ENUM  setEffect(EIndex_Effect_T const eEffect) = 0;
    virtual ESensorDev_T getSensorDev() const = 0;
    virtual EOperMode_T  getOperMode()   const = 0;
    virtual MERROR_ENUM  setSensorMode(ESensorMode_T const eSensorMode) = 0;
    virtual ESensorMode_T  getSensorMode()   const = 0;
    virtual MERROR_ENUM  setOperMode(EOperMode_T const eOperMode) = 0;
    virtual MERROR_ENUM setZoomRatio(MINT32 const i4ZoomRatio_x100) = 0;
    virtual MERROR_ENUM setAWBInfo(AWB_INFO_T const &rAWBInfo) = 0;
    virtual MERROR_ENUM setAEInfo(AE_INFO_T const &rAEInfo) = 0;
    virtual MERROR_ENUM setAFInfo(AF_INFO_T const &rAFInfo) = 0;
    virtual MERROR_ENUM setFlashInfo(FLASH_INFO_T const &rFlashInfo) = 0;

    //virtual MERROR_ENUM setIso(MUINT32 const u4ISOValue) = 0;
    //virtual MERROR_ENUM setCCT(MINT32 const i4CCT) = 0;
    //virtual MERROR_ENUM setCCTIndex_CCM (
    //            MINT32 const i4CCT,
    //            MINT32 const i4FluorescentIndex
    //        ) = 0;
    //virtual MERROR_ENUM setCCTIndex_Shading(MINT32 const i4CCT) = 0;
    virtual MERROR_ENUM setIndex_Shading(MINT32 const i4IDX) = 0;
    virtual MERROR_ENUM getIndex_Shading(MVOID*const pCmdArg) = 0;
	virtual MERROR_ENUM setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo) = 0;
	virtual MERROR_ENUM getPureOBCInfo(ISP_NVRAM_OBC_T *pOBCInfo) = 0;

    //
    //virtual MERROR_ENUM setSceneLightValue(MINT32 const i4SceneLV_x10) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // ISP End-User-Defined Tuning Index.
    virtual MERROR_ENUM setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    ISP Enable/Disable.
    //virtual MERROR_ENUM setEnable_Meta_Gamma(MBOOL const fgForceEnable) = 0;
    //virtual MBOOL       getEnable_Meta_Gamma()  const = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.
    virtual MERROR_ENUM getDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo
    ) const = 0;

public:     ////    Operations.
    //virtual MERROR_ENUM construct() = 0;
    virtual MERROR_ENUM init() = 0;
    virtual MERROR_ENUM uninit() = 0;
    virtual MERROR_ENUM validate(MBOOL const fgForce = MFALSE) = 0;
    virtual MERROR_ENUM validateFrameless() = 0;
    virtual MERROR_ENUM validatePerFrame(MBOOL const fgForce = MFALSE) = 0;

};


};  //  namespace NSIspTuning
#endif // _PARAMCTRL_IF_H_

