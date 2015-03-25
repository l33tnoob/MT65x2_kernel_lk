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
/**
 * @file ae_mgr.h
 * @brief AE manager
 */

#ifndef _AE_MGR_H_
#define _AE_MGR_H_

#include <ae_feature.h>

/**  
 * @brief AE state motion define
 */
typedef enum
{
    AE_INIT_STATE = -1,    // Internal use: set in init3A()
    AE_AUTO_FRAMERATE_STATE = 0,
    AE_MANUAL_FRAMERATE_STATE,
    AE_AF_STATE,
    AE_PRE_CAPTURE_STATE,
    AE_CAPTURE_STATE,
    AE_REINIT_STATE,    // Internal use: set in init3A()
} AE_STATE_T;


namespace NS3A
{

class IAeAlgo;

/*******************************************************************************
*
*******************************************************************************/
class AeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AeMgr(AeMgr const&);
    //  Copy-assignment operator is disallowed.
    AeMgr& operator=(AeMgr const&);

public:  ////
    AeMgr();
    ~AeMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AeMgr& getInstance();
    MRESULT cameraPreviewInit(MINT32 i4SensorDev, Param_T &rParam);
    MRESULT camcorderPreviewInit(MINT32 i4SensorDev, Param_T &rParam);
    MRESULT cameraPreviewReinit();
    MRESULT uninit();

    inline MBOOL isAELockSupported()
    {
        return MTRUE;
    }

    MRESULT setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea);
    MRESULT setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep);
    MRESULT setAEMeteringMode(MUINT32 u4NewAEMeteringMode);
    MINT32 getAEMeterMode() const;
    MRESULT setAEISOSpeed(MUINT32 i4NewAEISOSpeed);
    MINT32 getAEISOSpeedMode() const;
    MRESULT setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    MRESULT setAEFlickerMode(MUINT32 u4NewAEFLKMode);
    MRESULT setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode);
    MRESULT setAECamMode(MUINT32 u4NewAECamMode);   
    MRESULT setAEShotMode(MUINT32 u4NewAEShotMode);
    MRESULT setAEMode(MUINT32 u4NewAEMode);
    MINT32 getAEMode() const;
    MRESULT setAELock(MBOOL bAELock);
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT enableAE();
    MRESULT disableAE();
    MRESULT doPvAE(MVOID *pAEStatBuf);
    MRESULT doAFAE(MVOID *pAEStatBuf);
    MRESULT doPreCapAE(MBOOL bIsStrobeFired, MVOID *pAEStatBuf);
    MRESULT doCapAE();
    MRESULT doBackAEInfo();
    MRESULT doRestoreAEInfo();
    MRESULT getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo);
    MINT32 getLVvalue();
    MINT32 getBVvalue();
    MUINT32 getAEMaxMeterAreaNum();
    MINT32 getEVCompensateIndex();
    MRESULT getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAFPlineInfo &a_StrobeAEPlineTable);
    MRESULT getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo);
    MBOOL IsDoAEInPreAF();
    MBOOL IsAEStable();
    MBOOL IsStrobeBVTrigger();
    MRESULT getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo);
    MRESULT getCaptureParams(MINT8 index, MINT32 i4EVidx, AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    MRESULT getHDRCapInfo(Hal3A_HDROutputParam_T & strHDROutputInfo);
    MRESULT getRTParams(FrameOutputParam_T &a_strFrameInfo);
    MRESULT setFDInfo(MVOID* a_sFaces);
    MRESULT setStrobeMode(MBOOL bIsStrobeOn);
    MRESULT setAERotateDegree(MINT32 i4RotateDegree);
    MBOOL getAECondition(MUINT32 i4AECondition);
    MRESULT getLCEPlineInfo(LCEInfo_T &a_rLCEInfo);

    // CCT feature APIs.
    MINT32 CCTOPAEEnable();
    MINT32 CCTOPAEDisable();
    MINT32 CCTOPAEGetEnableInfo(MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetAEMode(MINT32 a_AEMode);
    MINT32 CCTOPAEGetAEMode(MINT32 *a_pAEMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetMeteringMode(MINT32 a_AEMeteringMode);
    MINT32 CCTOPAEApplyExpParam(MVOID *a_pAEExpParam);
    MINT32 CCTOPAESetFlickerMode(MINT32 a_AEFlickerMode);
    MINT32 CCTOPAEGetExpParam(MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetFlickerMode(MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetMeteringMode(MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEApplyNVRAMParam(MVOID *a_pAENVRAM);
    MINT32 CCTOPAEGetNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESaveNVRAMParam();
    MINT32 CCTOPAEGetCurrentEV(MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    MINT32 CCTOPAELockExpSetting();
    MINT32 CCTOPAEUnLockExpSetting();
    MINT32 CCTOPAEGetIspOB(MUINT32 *a_pIspOB, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspOB(MUINT32 a_IspOB);
    MINT32 CCTOPAEGetIspRAWGain(MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspRAWGain(MUINT32 a_IspRAWGain);
    MINT32 CCTOPAESetSensorExpTime(MUINT32 a_ExpTime);
    MINT32 CCTOPAESetSensorExpLine(MUINT32 a_ExpLine) const;
    MINT32 CCTOPAESetSensorGain(MUINT32 a_SensorGain) const;
    MINT32 CCTOPAESetCaptureMode(MUINT32 a_CaptureMode); 
    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MRESULT getSensorResolution();
    MRESULT getNvramData();
    MRESULT AEInit(Param_T &rParam);
    MRESULT copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo);
    MRESULT prepareCapParams();
    MRESULT updateCapParamsByHDR();
    MRESULT UpdateSensorISPParams(AE_STATE_T eNewAEState);
    MRESULT PreviewAEInit(MINT32 i4SensorDev, Param_T &rParam);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IAeAlgo* m_pIAeAlgo;
    AE_CCT_CFG_T m_AeMgrCCTConfig;
    EZOOM_WINDOW_T m_eZoomWinInfo;
    AEMeteringArea_T m_eAEMeterArea;
    AEMeterArea_T m_eAEFDArea;
    MINT32 m_i4SensorDev;
    MINT32 m_BVvalue;
    MINT32 m_i4WaitVDNum;
    MINT32 m_i4RotateDegree;
    MINT32 m_i4TimeOutCnt;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4SensorGainDelayFrames;
    MINT32 m_i4IspGainDelayFrames;    
    MINT32 m_i4AEidxCurrent;  // current AE idx
    MINT32 m_i4AEidxNext;   // next AE idx
    MUINT32 m_u4PreExposureTime;
    MUINT32 m_u4PreSensorGain;
    MUINT32 m_u4PreIspGain;
    MUINT32 m_u4SmoothIspGain;    
    MUINT32 m_u4AECondition;

    MBOOL m_bOneShotAEBeforeLock;
    MBOOL m_bAEModeChanged;
    MBOOL m_bAELock;
    MBOOL m_bEnableAE;
    MBOOL m_bVideoDynamic;
    MBOOL m_bRealISOSpeed;
    MBOOL m_bAElimitor;
    MBOOL m_bAEStable;    
    MBOOL m_bAEReadyCapture;    
    MBOOL m_bLockExposureSetting;
    MBOOL m_bStrobeOn;
    MBOOL m_bAEMgrDebugEnable;
    LIB3A_AE_MODE_T m_eAEMode;     // change AE Pline
    MFLOAT  m_fEVCompStep;
    MINT32  m_i4EVIndex;
    LIB3A_AE_METERING_MODE_T    m_eAEMeterMode;
    LIB3A_AE_ISO_SPEED_T    m_eAEISOSpeed;   // change AE Pline
    LIB3A_AE_FLICKER_MODE_T    m_eAEFlickerMode;    // change AE Pline
    MINT32    m_i4AEMaxFps;    
    MINT32    m_i4AEMinFps;    
    LIB3A_AE_FLICKER_AUTO_MODE_T    m_eAEAutoFlickerMode;   // change AE Pline
    EAppMode m_eCamMode;
    LIB3A_AECAM_MODE_T m_eAECamMode;
    EShotMode m_eShotMode;   
    strAETable m_CurrentPreviewTable;
    strAETable m_CurrentCaptureTable;
    LIB3A_AE_EVCOMP_T m_eAEEVcomp;
    AE_MODE_CFG_T mCaptureMode;
    Hal3A_HDROutputParam_T m_strHDROutputInfo;
    AE_STATE_T m_AEState;
};

};  //  namespace NS3A
#endif // _AE_MGR_H_

