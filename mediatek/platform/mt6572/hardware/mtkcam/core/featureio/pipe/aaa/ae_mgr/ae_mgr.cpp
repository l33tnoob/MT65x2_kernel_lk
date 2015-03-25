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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <dbg_isp_param.h>
#include <ae_param.h>
#include <camera_custom_AEPlinetable.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <faces.h>
#include <ae_mgr.h>
#include <ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <kd_camera_feature.h>

using namespace NS3A;
using namespace NSIspTuning;

NVRAM_CAMERA_3A_STRUCT* g_p3ANVRAM;
static SENSOR_RESOLUTION_INFO_T g_rSensorResolution[2]; // [0]: for TG1 (main/sub), [1]: for TG2(main_2)
AE_INITIAL_INPUT_T g_rAEInitInput;
AE_OUTPUT_T g_rAEOutput;
static AE_STAT_PARAM_T g_rAEStatCfg;
static AE_PLINETABLE_T* g_rAEPlineTable;
static  AeMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr&
AeMgr::
getInstance()
{
//    MY_LOG("[AeMgr]0x%08x\n", &singleton);
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
AeMgr()
//    : m_pIAeAlgo(IAeAlgo::createInstance())
    : m_i4SensorDev(ESensorDev_Main)
    , m_BVvalue(0)
    , m_i4WaitVDNum(0)
    , m_i4RotateDegree(0)
    , m_i4TimeOutCnt(0)
    , m_i4ShutterDelayFrames(2)
    , m_i4SensorGainDelayFrames(2)
    , m_i4IspGainDelayFrames(0)    
    , m_i4AEidxCurrent(0)
    , m_i4AEidxNext(0)
    , m_u4PreExposureTime(0)
    , m_u4PreSensorGain(0)
    , m_u4PreIspGain(0)
    , m_u4SmoothIspGain(0)
    , m_u4AECondition(0)
    , m_bOneShotAEBeforeLock(MFALSE)
    , m_bAEModeChanged(MFALSE)
    , m_bAELock(MFALSE)
    , m_bVideoDynamic(MFALSE)
    , m_bRealISOSpeed(MFALSE)
    , m_bAElimitor(MFALSE)
    , m_bAEStable(MFALSE)
    , m_bAEReadyCapture(MFALSE)
    , m_bLockExposureSetting(MFALSE)
    , m_bStrobeOn(MFALSE)
    , m_bAEMgrDebugEnable(MFALSE)
    , m_eAEMode(LIB3A_AE_MODE_AUTO)
    , m_fEVCompStep(1)
    , m_i4EVIndex(0)
    , m_eAEMeterMode(LIB3A_AE_METERING_MODE_CENTER_WEIGHT)
    , m_eAEISOSpeed(LIB3A_AE_ISO_SPEED_AUTO)
    , m_eAEFlickerMode(LIB3A_AE_FLICKER_MODE_50HZ)
    , m_i4AEMaxFps(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_i4AEMinFps(LIB3A_AE_FRAMERATE_MODE_05FPS)
    , m_eAEAutoFlickerMode(LIB3A_AE_FLICKER_AUTO_MODE_50HZ)
    , m_eCamMode(eAppMode_PhotoMode)
    , m_eAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_eShotMode(eShotMode_NormalShot)
    , m_eAEEVcomp(LIB3A_AE_EV_COMP_00)
    , m_AEState(AE_INIT_STATE)
{
    memset(&m_AeMgrCCTConfig, 0, sizeof(AE_CCT_CFG_T));
    memset(&m_eZoomWinInfo, 0, sizeof(EZOOM_WINDOW_T));
    memset(&m_eAEMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_eAEFDArea, 0, sizeof(AEMeterArea_T));
    memset(&m_CurrentPreviewTable, 0, sizeof(strAETable));
    memset(&m_CurrentCaptureTable, 0, sizeof(strAETable));
    memset(&mCaptureMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));

    MY_LOG("[AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
~AeMgr()
{
    MY_LOG("[~AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::PreviewAEInit(MINT32 i4SensorDev, Param_T &rParam)
{
    MRESULT err;
    MINT32 i4SutterDelay, i4SensorGainDelay, i4IspGainDelay;
    
    // set sensor initial
    err = AAASensorMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().init fail\n");
        return err;
    }

    // set sensor type
    m_i4SensorDev = i4SensorDev;
    err = AAASensorMgr::getInstance().setSensorDev(m_i4SensorDev);
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().setSensorDev fail\n");
        return err;
    }

    AAASensorMgr::getInstance().getSensorSyncinfo(&i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay);

    if((i4SutterDelay <= 5) && (i4SensorGainDelay <= 5) && (i4IspGainDelay <= 5)) {
        if(i4SutterDelay < i4IspGainDelay) { 
            m_i4ShutterDelayFrames = i4IspGainDelay - i4SutterDelay;
        } else {
            m_i4ShutterDelayFrames = 2;
        }

        if(i4SensorGainDelay < i4IspGainDelay) { 
            m_i4SensorGainDelayFrames = i4IspGainDelay - i4SensorGainDelay;
        } else {
            m_i4SensorGainDelayFrames = 2;
        }

        m_i4IspGainDelayFrames = 1; // for CQ0 1 delay frame
        MY_LOG("Delay info is shutter :%d sensor gain:%d isp gain:%d Sensor Info:%d %d %d\n", m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    } else {
        MY_LOG("Delay info is incorrectly :%d %d %d\n", i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
        m_i4ShutterDelayFrames = 2;
        m_i4SensorGainDelayFrames = 2;
        m_i4IspGainDelayFrames = 1; // for CQ0 1 delay frame
    }

    // Get sensor resolution    
    err = getSensorResolution();
    if (FAILED(err)) {
        MY_ERR("getSensorResolution() fail\n");
        return err;
    }

    // Get NVRAM data
    err = getNvramData();
    if (FAILED(err)) {
        MY_ERR("getNvramData() fail\n");
        return err;
    }

    // Init AE
    err = AEInit(rParam);
    if (FAILED(err)) {
        MY_ERR("AEInit() fail\n");
        return err;
    }

    // Init IspDrvMgr 
    err = IspDrvMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("IspDrvMgr::getInstance().init() fail\n");
        return err;
    }
    
    // AE statistics and histogram config
     err = ISP_MGR_AE_STAT_HIST_CONFIG_T::getInstance((ESensorDev_T)m_i4SensorDev).config(g_rAEStatCfg);
    if (FAILED(err)) {
        MY_ERR("AE state hist config() fail\n");
        return err;
    }

    UpdateSensorISPParams(AE_INIT_STATE);
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewInit(MINT32 i4SensorDev, Param_T &rParam)
{
    MY_LOG("[AeMgr]cameraPreviewInit\n");

    PreviewAEInit(i4SensorDev, rParam);
    m_AEState = AE_AUTO_FRAMERATE_STATE;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::camcorderPreviewInit(MINT32 i4SensorDev, Param_T &rParam)
{
    MRESULT err;

    MY_LOG("[AeMgr]camcorderPreviewInit\n");

    // the same with preview initial
    PreviewAEInit(i4SensorDev, rParam);

    m_AEState = AE_MANUAL_FRAMERATE_STATE;

    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// for come back to preview/video condition use
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewReinit()
{
    MRESULT err;                    

    if(m_bLockExposureSetting == MTRUE) {
        MY_LOG("[cameraPreviewReinit] Lock sensor setting:%d\n", m_bLockExposureSetting);        
        if(m_i4AEMinFps == m_i4AEMaxFps) {
            m_AEState = AE_MANUAL_FRAMERATE_STATE;
        } else {
            m_AEState = AE_AUTO_FRAMERATE_STATE;
        }
        return S_AE_OK;        
    }
    
    MY_LOG("[cameraPreviewReinit] Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", g_rAEOutput.rPreviewMode.u4Eposuretime, 
                    g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate, 
                    g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);               

    UpdateSensorISPParams(AE_REINIT_STATE);

    if(m_i4AEMinFps == m_i4AEMaxFps) {
        m_AEState = AE_MANUAL_FRAMERATE_STATE;
    } else {
        m_AEState = AE_AUTO_FRAMERATE_STATE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::uninit()
{
    MRESULT err;

// uninit IspDrvMgr 
    err = IspDrvMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("IspDrvMgr::getInstance().uninit() fail\n");
        return err;
    }

    err = AAASensorMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().uninit fail\n");
        return err;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->destroyInstance();
        m_pIAeAlgo = NULL;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea) 
{
    MUINT32 i;
    MBOOL bAreaChage = MFALSE;
    MUINT32 u4AreaCnt;
    AEMeteringArea_T *sAEMeteringArea = (AEMeteringArea_T* )sNewAEMeteringArea;

    if (sNewAEMeteringArea->u4Count <= 0) {
//        MY_LOG("No AE Metering area cnt: %d\n", sNewAEMeteringArea->u4Count);
        return S_AE_OK;
    } else if (sNewAEMeteringArea->u4Count > MAX_METERING_AREAS) {
        MY_ERR("The AE Metering area cnt error: %d\n", sNewAEMeteringArea->u4Count);
        return E_AE_UNSUPPORT_MODE;    
    }
    
    u4AreaCnt = sAEMeteringArea->u4Count;    
   
    for(i=0; i<u4AreaCnt; i++) {            
        if (sAEMeteringArea->rAreas[i].i4Left   < -1000)  {sAEMeteringArea->rAreas[i].i4Left   = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Right  < -1000)  {sAEMeteringArea->rAreas[i].i4Right  = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Top    < -1000)  {sAEMeteringArea->rAreas[i].i4Top    = -1000;}
        if (sAEMeteringArea->rAreas[i].i4Bottom < -1000)  {sAEMeteringArea->rAreas[i].i4Bottom = -1000;}

        if (sAEMeteringArea->rAreas[i].i4Left   > 1000)  {sAEMeteringArea->rAreas[i].i4Left   = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Right  > 1000)  {sAEMeteringArea->rAreas[i].i4Right  = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Top    > 1000)  {sAEMeteringArea->rAreas[i].i4Top    = 1000;}
        if (sAEMeteringArea->rAreas[i].i4Bottom > 1000)  {sAEMeteringArea->rAreas[i].i4Bottom = 1000;}   	 

        if((sAEMeteringArea->rAreas[i].i4Left != m_eAEMeterArea.rAreas[i].i4Left) || (sAEMeteringArea->rAreas[i].i4Right != m_eAEMeterArea.rAreas[i].i4Right) ||
            (sAEMeteringArea->rAreas[i].i4Top != m_eAEMeterArea.rAreas[i].i4Top) || (sAEMeteringArea->rAreas[i].i4Bottom != m_eAEMeterArea.rAreas[i].i4Bottom)) { 
            MY_LOG("New AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n", i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
            MY_LOG("Original AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n", i, m_eAEMeterArea.rAreas[i].i4Left, m_eAEMeterArea.rAreas[i].i4Right, m_eAEMeterArea.rAreas[i].i4Top, m_eAEMeterArea.rAreas[i].i4Bottom, m_eAEMeterArea.rAreas[i].i4Weight);
            m_eAEMeterArea.rAreas[i].i4Left = sAEMeteringArea->rAreas[i].i4Left;
            m_eAEMeterArea.rAreas[i].i4Right = sAEMeteringArea->rAreas[i].i4Right;
            m_eAEMeterArea.rAreas[i].i4Top = sAEMeteringArea->rAreas[i].i4Top;
            m_eAEMeterArea.rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom;
            m_eAEMeterArea.rAreas[i].i4Weight = sAEMeteringArea->rAreas[i].i4Weight;      
            bAreaChage = MTRUE;
        }
    }    
    if(bAreaChage == MTRUE) {
        m_eAEMeterArea.u4Count = u4AreaCnt;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMeteringArea(&m_eAEMeterArea);
        } else {
            MY_LOG("The AE algo class is NULL (1)\n");
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFDInfo(MVOID* a_sFaces)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;

    if((m_eAEFDArea.i4Left != pFaces->faces->rect[0]) || (m_eAEFDArea.i4Right != pFaces->faces->rect[2]) || 
        (m_eAEFDArea.i4Top != pFaces->faces->rect[1]) || (m_eAEFDArea.i4Bottom != pFaces->faces->rect[3])) {
        m_eAEFDArea.i4Left   = pFaces->faces->rect[0];
        m_eAEFDArea.i4Right  = pFaces->faces->rect[2];
        m_eAEFDArea.i4Top    = pFaces->faces->rect[1];
        m_eAEFDArea.i4Bottom = pFaces->faces->rect[3];

        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
        } else {
            MY_LOG("The AE algo class is NULL (2)\n");
        }
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep)
{
MINT32 i4EVValue, i4EVStep;   

    if (m_i4EVIndex != i4NewEVIndex) {
        m_i4EVIndex = i4NewEVIndex;
        m_fEVCompStep = fStep;
        i4EVStep = (MINT32) (100 * m_fEVCompStep);
        i4EVValue = i4NewEVIndex * i4EVStep;
     
        if(i4EVValue < -350) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n40; } 
        else if(i4EVValue < -300) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n35; } 
        else if(i4EVValue < -250) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n30; } 
        else if(i4EVValue < -200) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n25; } 
        else if(i4EVValue < -170) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n20; } 
        else if(i4EVValue < -160) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n17; } 
        else if(i4EVValue < -140) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n15; } 
        else if(i4EVValue < -130) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n13; } 
        else if(i4EVValue < -90) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n10; } 
        else if(i4EVValue < -60) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n07; } 
        else if(i4EVValue < -40) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n05; } 
        else if(i4EVValue < -10) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n03; } 
        else if(i4EVValue == 0) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;   } 
        else if(i4EVValue < 40) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_03;   } 
        else if(i4EVValue < 60) {     m_eAEEVcomp = LIB3A_AE_EV_COMP_05;  } 
        else if(i4EVValue < 90) {     m_eAEEVcomp = LIB3A_AE_EV_COMP_07;  } 
        else if(i4EVValue < 110) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_10;   } 
        else if(i4EVValue < 140) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_13;   } 
        else if(i4EVValue < 160) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_15;   } 
        else if(i4EVValue < 180) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_17;   } 
        else if(i4EVValue < 210) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_20;   } 
        else if(i4EVValue < 260) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_25;   } 
        else if(i4EVValue < 310) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_30;   } 
        else if(i4EVValue < 360) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_35;   } 
        else { m_eAEEVcomp = LIB3A_AE_EV_COMP_40;  }
     
        MY_LOG("m_i4EVIndex: %d EVComp:%d\n", m_i4EVIndex, m_eAEEVcomp);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setEVCompensate(m_eAEEVcomp);
        } else {
            MY_LOG("The AE algo class is NULL (3)\n");
        }
    } 

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getEVCompensateIndex()
{
MINT32 iEVIndex;

    switch(m_eAEEVcomp){
        case LIB3A_AE_EV_COMP_03: { iEVIndex = 3;   break; }
        case LIB3A_AE_EV_COMP_05: { iEVIndex = 5;   break; }
        case LIB3A_AE_EV_COMP_07: { iEVIndex = 7;   break; }
        case LIB3A_AE_EV_COMP_10: { iEVIndex = 10;  break; }
        case LIB3A_AE_EV_COMP_13: { iEVIndex = 13;  break; }
        case LIB3A_AE_EV_COMP_15: { iEVIndex = 15;  break; }
        case LIB3A_AE_EV_COMP_17: { iEVIndex = 17;  break; }
        case LIB3A_AE_EV_COMP_20: { iEVIndex = 20;  break; }
        case LIB3A_AE_EV_COMP_25: { iEVIndex = 25;  break; }
        case LIB3A_AE_EV_COMP_30: { iEVIndex = 30;  break; }
        case LIB3A_AE_EV_COMP_35: { iEVIndex = 35;  break; }
        case LIB3A_AE_EV_COMP_40: { iEVIndex = 40;  break; }
        case LIB3A_AE_EV_COMP_n03: { iEVIndex = -3;   break; }
        case LIB3A_AE_EV_COMP_n05: { iEVIndex = -5;   break; }
        case LIB3A_AE_EV_COMP_n07: { iEVIndex = -7;   break; }
        case LIB3A_AE_EV_COMP_n10: { iEVIndex = -10;  break; }
        case LIB3A_AE_EV_COMP_n13: { iEVIndex = -13;  break; }
        case LIB3A_AE_EV_COMP_n15: { iEVIndex = -15;  break; }
        case LIB3A_AE_EV_COMP_n17: { iEVIndex = -17;  break; }
        case LIB3A_AE_EV_COMP_n20: { iEVIndex = -20;  break; }
        case LIB3A_AE_EV_COMP_n25: { iEVIndex = -25;  break; }
        case LIB3A_AE_EV_COMP_n30: { iEVIndex = -30;  break; }
        case LIB3A_AE_EV_COMP_n35: { iEVIndex = -35;  break; }
        case LIB3A_AE_EV_COMP_n40: { iEVIndex = -40;  break; }
        default:
        case LIB3A_AE_EV_COMP_00: 
            iEVIndex = 0;
            break;
    }
    return iEVIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMeteringMode(MUINT32 u4NewAEMeteringMode)
{
    LIB3A_AE_METERING_MODE_T eNewAEMeteringMode = static_cast<LIB3A_AE_METERING_MODE_T>(u4NewAEMeteringMode);

    if ((eNewAEMeteringMode <= LIB3A_AE_METERING_MODE_UNSUPPORTED) || (eNewAEMeteringMode >= LIB3A_AE_METERING_MODE_MAX)) {
        MY_ERR("Unsupport AE Metering Mode: %d\n", eNewAEMeteringMode);
        return E_AE_UNSUPPORT_MODE;
    } 

    if (m_eAEMeterMode != eNewAEMeteringMode) {
        m_eAEMeterMode = eNewAEMeteringMode;
        MY_LOG("m_eAEMeterMode: %d\n", m_eAEMeterMode);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMeteringMode(m_eAEMeterMode);
        } else {
            MY_LOG("The AE algo class is NULL (4)\n");
        }
    } 
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMeterMode() const
{
    return static_cast<MINT32>(m_eAEMeterMode);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEISOSpeed(MUINT32 u4NewAEISOSpeed)
{
    LIB3A_AE_ISO_SPEED_T eAEISOSpeed;

    if (u4NewAEISOSpeed > LIB3A_AE_ISO_SPEED_MAX) {
        MY_ERR("Unsupport AE ISO Speed: %d\n", u4NewAEISOSpeed);
        return E_AE_UNSUPPORT_MODE;
    } 

    switch(u4NewAEISOSpeed) {
        case 0:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_AUTO;
            break;
        case 100:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_100;
            break;
        case 150:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_150;
            break;            
        case 200:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_200;
            break;
        case 300:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_300;
            break;
        case 400:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_400;
            break;
        case 600:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_600;
            break;
        case 800:
             eAEISOSpeed = LIB3A_AE_ISO_SPEED_800;
           break;
        case 1200:
             eAEISOSpeed = LIB3A_AE_ISO_SPEED_1200;
           break;
        case 1600:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_1600;
            break;
        case 2400:
             eAEISOSpeed = LIB3A_AE_ISO_SPEED_2400;
           break;
        case 3200:
             eAEISOSpeed = LIB3A_AE_ISO_SPEED_3200;
           break;
        default:
            MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);            
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_AUTO;            
            break;
    }

    if (m_eAEISOSpeed != eAEISOSpeed) {
        MY_LOG("m_eAEISOSpeed: %d old:%d\n", eAEISOSpeed, m_eAEISOSpeed);
        m_eAEISOSpeed = eAEISOSpeed;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setIsoSpeed(m_eAEISOSpeed);
        } else {
            MY_LOG("The AE algo class is NULL (5)\n");
        }
    } 

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEISOSpeedMode() const
{
    return static_cast<MINT32>(m_eAEISOSpeed);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 i4NewMinFPS, i4NewMaxFPS;

    i4NewMinFPS = i4NewAEMinFps / 100;
    i4NewMaxFPS = i4NewAEMaxFps / 100;

    if ((i4NewMinFPS < LIB3A_AE_FRAMERATE_MODE_05FPS) || (i4NewMaxFPS > LIB3A_AE_FRAMERATE_MODE_MAX)) {
        MY_LOG("Unsupport AE frame rate range value: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        return E_AE_UNSUPPORT_MODE;
    } else if(i4NewMinFPS > i4NewMaxFPS) {
        MY_ERR("Unsupport AE frame rate: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        return E_AE_UNSUPPORT_MODE;
    }

    if ((m_i4AEMinFps != i4NewMinFPS) || (m_i4AEMaxFps != i4NewMaxFPS)) {
        m_i4AEMinFps = i4NewMinFPS;
        m_i4AEMaxFps = i4NewMaxFPS;
        MY_LOG("m_i4AEMinFps: %d m_i4AEMaxFps:%d\n", m_i4AEMinFps, m_i4AEMaxFps);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMinMaxFrameRate(m_i4AEMinFps, m_i4AEMaxFps);
            if(m_eAECamMode == LIB3A_AECAM_MODE_VIDEO) {
                if(m_i4AEMinFps == m_i4AEMaxFps) {
                    m_pIAeAlgo->setAEVideoDynamicEnable(MFALSE);
                } else {
                    m_pIAeAlgo->setAEVideoDynamicEnable(MTRUE);
                }
            }
            m_pIAeAlgo->setAECamMode(m_eAECamMode);      
        } else {
            MY_LOG("The AE algo class is NULL (6)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEFlickerMode(MUINT32 u4NewAEFLKMode)
{
    AE_FLICKER_MODE_T eNewAEFLKMode = static_cast<AE_FLICKER_MODE_T>(u4NewAEFLKMode);
    LIB3A_AE_FLICKER_MODE_T eAEFLKMode;

    if ((eNewAEFLKMode < AE_FLICKER_MODE_BEGIN) || (eNewAEFLKMode >= AE_FLICKER_MODE_TOTAL_NUM)) {
        MY_ERR("Unsupport AE flicker mode: %d\n", eNewAEFLKMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEFLKMode) {
        case AE_FLICKER_MODE_60HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_60HZ;
            break;
        case AE_FLICKER_MODE_50HZ:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;
        case AE_FLICKER_MODE_AUTO:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_AUTO;
            break;
        case AE_FLICKER_MODE_OFF:
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_OFF;
            break;
        default:
            MY_LOG("The flicker enum value is incorrectly:%d\n", eNewAEFLKMode);            
            eAEFLKMode = LIB3A_AE_FLICKER_MODE_50HZ;
            break;            
    }
        
    if (m_eAEFlickerMode != eAEFLKMode) {
        MY_LOG("AEFlickerMode: %d old:%d\n", eAEFLKMode, m_eAEFlickerMode);
        m_eAEFlickerMode = eAEFLKMode;        
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEFlickerMode(m_eAEFlickerMode);
        } else {
            MY_LOG("The AE algo class is NULL (7)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode)
{
    LIB3A_AE_FLICKER_AUTO_MODE_T eNewAEAutoFLKMode = static_cast<LIB3A_AE_FLICKER_AUTO_MODE_T>(u4NewAEAutoFLKMode);

    if ((eNewAEAutoFLKMode <= LIB3A_AE_FLICKER_AUTO_MODE_UNSUPPORTED) || (eNewAEAutoFLKMode >= LIB3A_AE_FLICKER_AUTO_MODE_MAX)) {
        MY_ERR("Unsupport AE auto flicker mode: %d\n", eNewAEAutoFLKMode);
        return E_AE_UNSUPPORT_MODE;
    }

    if (m_eAEAutoFlickerMode != eNewAEAutoFLKMode) {
        m_eAEAutoFlickerMode = eNewAEAutoFLKMode;
        MY_LOG("m_eAEAutoFlickerMode: %d\n", m_eAEAutoFlickerMode);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEFlickerAutoMode(m_eAEAutoFlickerMode);
        } else {
            MY_LOG("The AE algo class is NULL (8)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAECamMode(MUINT32 u4NewAECamMode)
{
    EAppMode eNewAECamMode = static_cast<EAppMode>(u4NewAECamMode);

    if (m_eCamMode != eNewAECamMode) {
        m_eCamMode = eNewAECamMode;
        m_bRealISOSpeed = 0;
        
        switch(m_eCamMode) {
            case eAppMode_VideoMode:     //  Video Mode
            case eAppMode_VtMode:           //  VT Mode
               m_eAECamMode = LIB3A_AECAM_MODE_VIDEO;
                break;
            case eAppMode_EngMode:        //  Engineer Mode
                m_bRealISOSpeed = 1;
                m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;                
                break;
            case eAppMode_ZsdMode:        //  ZSD Mode
                m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
                break;
            case eAppMode_S3DMode:        //  S3D Mode
                // TBD
                // m_eAECamMode = LIB3A_AECAM_MODE_S3D;
                // break;
            case eAppMode_PhotoMode:     //  Photo Mode
            case eAppMode_DefaultMode:   //  Default Mode
            case eAppMode_AtvMode:         //  ATV Mode
                m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
                break;
        }     
    
        MY_LOG("m_eCamMode:%d AECamMode:%d RealISO:%d\n", m_eCamMode, m_eAECamMode, m_bRealISOSpeed);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
            m_pIAeAlgo->setAECamMode(m_eAECamMode);
        } else {
            MY_LOG("The AE algo class is NULL (9)\n");
        }
    }

    return S_AE_OK;
}    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEShotMode(MUINT32 u4NewAEShotMode)
{
    EShotMode eNewAEShotMode = static_cast<EShotMode>(u4NewAEShotMode);

    if (m_eShotMode != eNewAEShotMode) {
        m_eShotMode = eNewAEShotMode;
        if(m_eShotMode == eShotMode_Autorama) {
            m_bAElimitor = TRUE;
        } else {
            m_bAElimitor = FALSE;     
        }
        MY_LOG("m_eAppShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);
        } else {
            MY_LOG("The AE algo class is NULL (10)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMode(MUINT32 u4NewAEMode)
{
    AE_MODE_T eNewAEMode = static_cast<AE_MODE_T>(u4NewAEMode);
    LIB3A_AE_MODE_T eAEMode;
        
    if ((eNewAEMode < AE_MODE_BEGIN) || (eNewAEMode >= AE_MODE_TOTAL_NUM)) {
        MY_ERR("Unsupport AE mode: %d\n", eNewAEMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEMode) {
        case AE_MODE_OFF:
            eAEMode = LIB3A_AE_MODE_OFF;
            break;
        case AE_MODE_AUTO:
            eAEMode = LIB3A_AE_MODE_AUTO;
            break;
        case AE_MODE_NIGHT:
            eAEMode = LIB3A_AE_MODE_NIGHT;
            break;
        case AE_MODE_ACTION:
            eAEMode = LIB3A_AE_MODE_ACTION;
            break;
        case AE_MODE_BEACH:
            eAEMode = LIB3A_AE_MODE_BEACH;
            break;
        case AE_MODE_CANDLELIGHT:
            eAEMode = LIB3A_AE_MODE_CANDLELIGHT;
            break;
        case AE_MODE_FIREWORKS:
            eAEMode = LIB3A_AE_MODE_FIREWORKS;
            break;
        case AE_MODE_LANDSCAPE:
            eAEMode = LIB3A_AE_MODE_LANDSCAPE;
            break;
        case AE_MODE_PORTRAIT:
            eAEMode = LIB3A_AE_MODE_PORTRAIT;
            break;
        case AE_MODE_NIGHT_PORTRAIT:
            eAEMode = LIB3A_AE_MODE_NIGHT_PORTRAIT;
            break;
        case AE_MODE_PARTY:
            eAEMode = LIB3A_AE_MODE_PARTY;
            break;
        case AE_MODE_SNOW:
            eAEMode = LIB3A_AE_MODE_SNOW;
            break;
        case AE_MODE_SPORTS:
            eAEMode = LIB3A_AE_MODE_SPORTS;
            break;
        case AE_MODE_STEADYPHOTO:
            eAEMode = LIB3A_AE_MODE_STEADYPHOTO;
            break;
        case AE_MODE_SUNSET:
            eAEMode = LIB3A_AE_MODE_SUNSET;
            break;
        case AE_MODE_THEATRE:
            eAEMode = LIB3A_AE_MODE_THEATRE;
            break;
        case AE_MODE_ISO_ANTI_SHAKE:
            eAEMode = LIB3A_AE_MODE_ISO_ANTI_SHAKE;
            break;
        default:
            MY_LOG("The AE mode is not correctly: %d\n", eNewAEMode);            
            eAEMode = LIB3A_AE_MODE_AUTO;
            break;
    }
    
    if (m_eAEMode != eAEMode) {
        MY_LOG("m_eAEMode: %d old:%d\n", eAEMode, m_eAEMode);
        m_eAEMode = eAEMode;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEMode(m_eAEMode);
        } else {
            MY_LOG("The AE algo class is NULL (11)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMode() const
{
    return static_cast<MINT32>(m_eAEMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELock(MBOOL bAELock)
{
    if (m_bAELock != bAELock) {
        MY_LOG("[AeMgr::setAELock] m_bAELock: %d %d\n", m_bAELock, bAELock);
        if (bAELock) { // AE lock
            m_bAELock = MTRUE;
            m_bOneShotAEBeforeLock = MTRUE;
        } else { // AE unlock
            m_bAELock = MFALSE;
        }        
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->lockAE(m_bAELock);
        } else {
            MY_LOG("The AE algo class is NULL (12)\n");
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if((m_eZoomWinInfo.u4XOffset != u4XOffset) || (m_eZoomWinInfo.u4XWidth != u4Width) || 
       (m_eZoomWinInfo.u4YOffset != u4YOffset) || (m_eZoomWinInfo.u4YHeight != u4Height)) {
        MY_LOG("[AeMgr::setZoomWinInfo] New WinX:%d %d New WinY:%d %d Old WinX:%d %d Old WinY:%d %d\n", u4XOffset, u4Width, u4YOffset, u4Height, 
           m_eZoomWinInfo.u4XOffset, m_eZoomWinInfo.u4XWidth, m_eZoomWinInfo.u4YOffset, m_eZoomWinInfo.u4YHeight);
        m_eZoomWinInfo.bZoomChange = MTRUE;
        m_eZoomWinInfo.u4XOffset = u4XOffset;
        m_eZoomWinInfo.u4XWidth = u4Width;
        m_eZoomWinInfo.u4YOffset = u4YOffset;
        m_eZoomWinInfo.u4YHeight = u4Height;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getAEMaxMeterAreaNum()
{
    return MAX_METERING_AREAS;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAE()
{
    m_bEnableAE = MTRUE;

    MY_LOG("enableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::disableAE()
{
    m_bEnableAE = MFALSE;

    MY_LOG("disableAE()\n");
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPvAE(MVOID *pAEStatBuf)
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    m_bAEReadyCapture = MFALSE;  // reset capture flag
    m_i4TimeOutCnt = MFALSE;  // reset timeout counter
    
    if(m_bEnableAE) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
        } else {
            MY_LOG("The AE algo class is NULL (13)\n");
        }
        
        if(m_i4WaitVDNum == 0x00) {
            if(m_bAELock) {
                if(m_bOneShotAEBeforeLock == MTRUE) {
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    m_bOneShotAEBeforeLock = MFALSE;
                } else {
                    rAEInput.eAeState = AE_STATE_AELOCK;
                }
            } else {    
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
            }

            if(m_eZoomWinInfo.bZoomChange == MTRUE) {
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &g_rAEStatCfg);
                } else {
                    MY_LOG("The AE algo class is NULL (14)\n");
                }

                m_eZoomWinInfo.bZoomChange = MFALSE;

                // Update AE histogram window config
                ISP_MGR_AE_STAT_HIST_CONFIG_T::getInstance((ESensorDev_T)m_i4SensorDev).config(g_rAEStatCfg);
                m_i4WaitVDNum = 0x02;
            } else {        
                rAEInput.pAESatisticBuffer = pAEStatBuf;
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                } else {
                    MY_LOG("The AE algo class is NULL (15)\n");
                }
                
                m_bAEStable = rAEOutput.bAEStable;
                copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doPvAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
                    return S_AE_OK;        
                }
                // Update the preview or video state
                if(m_i4AEMinFps == m_i4AEMaxFps) {
                    UpdateSensorISPParams(AE_MANUAL_FRAMERATE_STATE);
                } else {
                    UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);                
                }
            }
        } else {
            MY_LOG("AE Wait Vd frame:%d Enable:%d\n", m_i4WaitVDNum, m_bEnableAE);   
            // continue update the preview or video state
            UpdateSensorISPParams(m_AEState);
        }
    }else {
        MY_LOG("[doPvAE] AE don't enable Enable:%d\n", m_bEnableAE);       
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAE(MVOID *pAEStatBuf)
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    MY_LOG("[doAFAE]:%d %d\n", m_i4TimeOutCnt, m_i4WaitVDNum);        

    if(m_i4TimeOutCnt > 18) {
        MY_LOG("[doAFAE] Time out happen\n");        
        if(m_bLockExposureSetting == MTRUE) {
            MY_LOG("[doAFAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
            return S_AE_OK;        
        }
        g_rAEOutput.rAFMode = g_rAEOutput.rPreviewMode;
        UpdateSensorISPParams(AE_AF_STATE);        
        return S_AE_OK;        
    } else {
        m_i4TimeOutCnt++;    
    }
    
    if(m_bEnableAE) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
        } else {
            MY_LOG("The AE algo class is NULL (16)\n");
        }
        m_bAEStable = MFALSE;
        
        if(m_i4WaitVDNum == 0x00) {
            MY_LOG("[doAFAE] AE_STATE_ONE_SHOT\n");               
            rAEInput.eAeState = AE_STATE_ONE_SHOT;
            rAEInput.pAESatisticBuffer = pAEStatBuf;
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            } else {
                MY_LOG("The AE algo class is NULL (17)\n");
            }
            copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
            // AE is stable, change to AF state
            if(rAEOutput.bAEStable == TRUE) {
                rAEInput.eAeState = AE_STATE_AFASSIST;
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                } else {
                    MY_LOG("The AE algo class is NULL (18)\n");
                }
                copyAEInfo2mgr(&g_rAEOutput.rAFMode, &rAEOutput);
//                prepareCapParams();    
            
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doAFAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
                    return S_AE_OK;        
                }
                UpdateSensorISPParams(AE_AF_STATE);
            } else {   
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
                    return S_AE_OK;        
                }
                // Using preview state to do AE before AE stable
                UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
            }
        } else {
            MY_LOG("[doAFAE]AE Wait Vd frame:%d Enable:%d State:%d\n", m_i4WaitVDNum, m_bEnableAE, m_AEState);   
            // continue update for preview or AF state
            UpdateSensorISPParams(m_AEState);
        }
    } else {
        MY_LOG("[doAFAE] AE don't enable Enable:%d\n", m_bEnableAE);       
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPreCapAE(MBOOL bIsStrobeFired, MVOID *pAEStatBuf)
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    if (m_bEnableAE)
    {
        if(m_bAEReadyCapture == MFALSE) {
            MY_LOG("[doPreCapAE] Ready:%d isStrobe:%d\n", m_bAEReadyCapture, bIsStrobeFired);
            if(m_i4TimeOutCnt > 18) {
                MY_LOG("[doPreCapAE] Time out happen\n");        
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doPreCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
                    return S_AE_OK;        
                }
                g_rAEOutput.rAFMode = g_rAEOutput.rPreviewMode;
                UpdateSensorISPParams(AE_AF_STATE);        
                return S_AE_OK;        
            } else {
                m_i4TimeOutCnt++;    
            }
            
            if(m_bAEStable == MFALSE) {
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
                } else {
                    MY_LOG("The AE algo class is NULL (32)\n");
                }
        
                if(m_i4WaitVDNum == 0x00) {
                    MY_LOG("[doPreCapAE] AE_STATE_ONE_SHOT\n");               
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    rAEInput.pAESatisticBuffer = pAEStatBuf;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    } else {
                        MY_LOG("The AE algo class is NULL (33)\n");
                    }
                    copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
                    
                    if(m_bLockExposureSetting == MTRUE) {
                        MY_LOG("[doPreCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
                        return S_AE_OK;        
                    }
                    // AE is stable, update capture info                    
                    if(rAEOutput.bAEStable == TRUE) {            
                        UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                    } else {                       
                        // Using preview state to do AE before AE stable
                        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                    }
                } else {
                    MY_LOG("[doPreCapAE]AE Wait Vd frame:%d Enable:%d State:%d\n", m_i4WaitVDNum, m_bEnableAE, m_AEState);   
                    // continue update for preview or AF state
                    UpdateSensorISPParams(m_AEState);
               }
            } else {
                MY_LOG("[doPreCapAE] AE stable already\n");       
                UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
            }
        } else {
            MY_LOG("[doPreCapAE] Do Nothing Ready:%d isStrobe:%d\n", m_bAEReadyCapture, bIsStrobeFired);       
        }
    } else {
        MY_LOG("[doPreCapAE] AE don't enable Enable:%d\n", m_bEnableAE);       
    }
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapAE()
{
    if(m_bLockExposureSetting == MTRUE) {
        MY_LOG("[doCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);        
        return S_AE_OK;        
    }
    
    UpdateSensorISPParams(AE_CAPTURE_STATE);
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doBackAEInfo()
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    MY_LOG("doBackAEInfo\n");

    rAEInput.eAeState = AE_STATE_BACKUP_PREVIEW;
    rAEInput.pAESatisticBuffer = NULL;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    } else {
        MY_LOG("The AE algo class is NULL (34)\n");
    }
    return S_AE_OK;        
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doRestoreAEInfo()
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    MY_LOG("doRestoreAEInfo\n");
    rAEInput.eAeState = AE_STATE_RESTORE_PREVIEW;
    rAEInput.pAESatisticBuffer = NULL;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    } else {
        MY_LOG("The AE algo class is NULL (35)\n");
    }

    copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
    prepareCapParams();   
    return S_AE_OK;        
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getLVvalue()
{
    return (m_BVvalue + 50);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getDebugInfo(rAEDebugInfo);
    } else {
        MY_LOG("The AE algo class is NULL (19)\n");
    }   

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo)
{
    // Copy Sensor information to output structure
    sAEOutputInfo->u4Eposuretime = sAEInfo->EvSetting.u4Eposuretime;
    sAEOutputInfo->u4AfeGain = sAEInfo->EvSetting.u4AfeGain;
    sAEOutputInfo->u4IspGain = sAEInfo->EvSetting.u4IspGain;
    sAEOutputInfo->u2FrameRate = sAEInfo->u2FrameRate;
    sAEOutputInfo->u4RealISO = sAEInfo->u4ISO;
    sAEOutputInfo->i2FlareOffset = sAEInfo->i2FlareOffset;
    sAEOutputInfo->i2FlareGain = sAEInfo->i2FlareGain;
    
    m_BVvalue = sAEInfo->Bv;
    m_u4AECondition = sAEInfo->u4AECondition;
    m_i4AEidxCurrent = sAEInfo->i4AEidxCurrent;
    m_i4AEidxNext = sAEInfo->i4AEidxNext;

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorResolution()
{
    MRESULT err = S_AE_OK;

    if ((m_i4SensorDev == ESensorDev_Main) || (m_i4SensorDev == ESensorDev_Sub)) {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_i4SensorDev, &g_rSensorResolution[0]);
    } else if(m_i4SensorDev == ESensorDev_MainSecond) {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_i4SensorDev, &g_rSensorResolution[1]);    
    } else {
        MY_ERR("Error sensor device\n");    
    }
    
    if (FAILED(err)) {
        MY_ERR("AAASensorMgr::getInstance().getSensorWidthHeight fail\n");
        return err;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getNvramData()
{
    if (FAILED(NvramDrvMgr::getInstance().init(m_i4SensorDev))) {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AE_NVRAM_DATA;
    }

    NvramDrvMgr::getInstance().getRefBuf(g_p3ANVRAM);
    if(g_p3ANVRAM == NULL) {
         MY_ERR("Nvram 3A pointer NULL\n");
    }
    
    NvramDrvMgr::getInstance().getRefBuf(g_rAEPlineTable);
    if(g_rAEPlineTable == NULL) {
         MY_ERR("Nvram AE Pline table pointer NULL\n");
    }
   
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::AEInit(Param_T &rParam)
{
    strAEOutput rAEOutput;
    
    MY_LOG("[AeMgr] AEInit\n");

    m_bEnableAE = isAEEnabled();

    if(g_p3ANVRAM != NULL) {
        g_rAEInitInput.rAENVRAM = g_p3ANVRAM->rAENVRAM;
    } else {
         MY_ERR("Nvram 3A pointer is NULL\n");
    }

    g_rAEInitInput.rAEPARAM = getAEParam();   // TBD
    
    if(g_rAEPlineTable != NULL) {
        g_rAEInitInput.rAEPlineTable = *g_rAEPlineTable;
    } else {
         MY_ERR("Nvram AE Pline table pointer is NULL\n");
    }
    g_rAEInitInput.i4AEMaxBlockWidth = AWB_WINDOW_NUM_X;
    g_rAEInitInput.i4AEMaxBlockHeight = AWB_WINDOW_NUM_Y;

    // ezoom info default is sensor resolution
    m_eZoomWinInfo.u4XOffset = 0;
    m_eZoomWinInfo.u4XWidth = g_rSensorResolution[0].u2SensorPreviewWidth;
    m_eZoomWinInfo.u4YOffset = 0;
    m_eZoomWinInfo.u4YHeight = g_rSensorResolution[0].u2SensorPreviewHeight;

    g_rAEInitInput.rEZoomWin = m_eZoomWinInfo;
    g_rAEInitInput.eAEMeteringMode = m_eAEMeterMode;
    g_rAEInitInput.eAEMode = m_eAEMode;
    g_rAEInitInput.eAECamMode = m_eAECamMode;
    g_rAEInitInput.eAEFlickerMode = m_eAEFlickerMode;
    g_rAEInitInput.eAEAutoFlickerMode = m_eAEAutoFlickerMode;
    g_rAEInitInput.eAEEVcomp = m_eAEEVcomp;
    g_rAEInitInput.i4AEMaxFps = m_i4AEMaxFps;
    g_rAEInitInput.i4AEMinFps = m_i4AEMinFps;
  
    MY_LOG("AE max block width:%d heigh:%d\n", g_rAEInitInput.i4AEMaxBlockWidth, g_rAEInitInput.i4AEMaxBlockHeight);
    m_pIAeAlgo = IAeAlgo::createInstance();
    if (!m_pIAeAlgo) {
        MY_ERR("AeAlgo::createInstance() fail \n");
        return E_AE_ALGO_INIT_ERR;
    }

    m_pIAeAlgo->initAE(&g_rAEInitInput, &rAEOutput, &g_rAEStatCfg);
    copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.ae_mgr.enable", value, "0");
    m_bAEMgrDebugEnable = atoi(value);
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareCapParams()
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;

    MY_LOG("m_eShotMode:%d\n", m_eShotMode);
    rAEInput.eAeState = AE_STATE_CAPTURE;
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    } else {
        MY_LOG("The AE algo class is NULL (20)\n");
    }
    copyAEInfo2mgr(&g_rAEOutput.rCaptureMode[0], &rAEOutput);
    copyAEInfo2mgr(&g_rAEOutput.rCaptureMode[1], &rAEOutput);
    copyAEInfo2mgr(&g_rAEOutput.rCaptureMode[2], &rAEOutput);
    mCaptureMode = g_rAEOutput.rCaptureMode[0];
    m_bAEReadyCapture = MTRUE;  // capture ready flag

    if(m_eShotMode == eShotMode_HdrShot) {
        updateCapParamsByHDR();       
    } 
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAFPlineInfo &a_StrobeAEPlineTable)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable);
        a_PrvAEPlineTable =  m_CurrentPreviewTable;
        a_CapAEPlineTable = m_CurrentCaptureTable;        
        MY_LOG("[getCurrentPlineTable] PreId:%d CapId:%d\n", m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID);    
    } else {
        MY_LOG("The AE algo class is NULL (21)\n");
    }

    if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD) {    
        a_StrobeAEPlineTable = g_rAEInitInput.rAEPARAM.strStrobeZSDPLine;
    } else {
        a_StrobeAEPlineTable = g_rAEInitInput.rAEPARAM.strStrobePLine;    
    }
    
    MY_LOG("[getCurrentPlineTable]Strobe enable:%d AECamMode:%d\n", a_StrobeAEPlineTable.bAFPlineEnable, m_eAECamMode);    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    a_rDeviceInfo = g_p3ANVRAM->rAENVRAM.rDevicesInfo;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsDoAEInPreAF()
{
    MY_LOG("[IsDoAEInPreAF] DoAEbeforeAF:%d\n", g_rAEInitInput.rAEPARAM.strAEParasetting.bPreAFLockAE);    
    return g_rAEInitInput.rAEPARAM.strAEParasetting.bPreAFLockAE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsAEStable()
{
    if (m_bAEMgrDebugEnable) {
        MY_LOG("[IsAEStable] m_bAEStable:%d\n", m_bAEStable);    
    }
    return m_bAEStable;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getBVvalue()
{
    return (m_BVvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsStrobeBVTrigger()
{
    MBOOL bStrobeBVTrigger;
    MINT32 i4Bv = 0;
    strAETable strCurrentPreviewTable;
    strAETable strCurrentCaptureTable;
    strAFPlineInfo strobeAEPlineTable;

    memset(&strCurrentCaptureTable, 0, sizeof(strAETable));
    getCurrentPlineTable(strCurrentPreviewTable, strCurrentCaptureTable, strobeAEPlineTable);

    if(g_rAEInitInput.rAEPARAM.strAEParasetting.bEV0TriggerStrobe == TRUE) {         // The strobe trigger by the EV 0 index
        i4Bv = getBVvalue();
    } else {
        if(g_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]) {
            if(m_pIAeAlgo != NULL) {
                i4Bv = getBVvalue() - m_pIAeAlgo->getSenstivityDeltaIndex(1024 *1024/ g_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]);
            } else {
                i4Bv = getBVvalue();
                MY_LOG("The AE algo class is NULL (22)\n");
            }
        }    
    }
    
    bStrobeBVTrigger = (i4Bv < strCurrentCaptureTable.i4StrobeTrigerBV)?TRUE:FALSE;
   
    MY_LOG("[IsStrobeBVTrigger] bStrobeBVTrigger:%d BV:%d %d\n", bStrobeBVTrigger, i4Bv, strCurrentCaptureTable.i4StrobeTrigerBV);                  

    return bStrobeBVTrigger;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setStrobeMode(MBOOL bIsStrobeOn)
{
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setStrobeMode(bIsStrobeOn);
    } else {
        MY_LOG("The AE algo class is NULL (23)\n");
    }
    m_bStrobeOn = bIsStrobeOn;
    // Update flare again 
    if(m_bStrobeOn == TRUE) {   // capture on, get capture parameters for flare again.
        prepareCapParams();
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo)
{
    a_rPreviewInfo = g_rAEOutput.rPreviewMode;
//    MY_LOG("[getPreviewParams] Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", a_rPreviewInfo.u4Eposuretime, a_rPreviewInfo.u4AfeGain, 
//                   a_rPreviewInfo.u4IspGain, a_rPreviewInfo.u2FrameRate, a_rPreviewInfo.i2FlareGain, a_rPreviewInfo.i2FlareOffset, a_rPreviewInfo.u4RealISO);                  
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCaptureParams(MINT8 index, MINT32 i4EVidx, AE_MODE_CFG_T &a_rCaptureInfo)
{
    strAEOutput rAEOutput;

    if((index > 2) || (index < 0)) {
         MY_ERR("Capture index error:%d\n", index);
         index = 0;
    }

    if(i4EVidx != 0) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->switchCapureDiffEVState(&rAEOutput, i4EVidx);
        } else {
            MY_LOG("The AE algo class is NULL (24)\n");
        }

        copyAEInfo2mgr(&mCaptureMode, &rAEOutput);
        a_rCaptureInfo = mCaptureMode;            
    } else {
        a_rCaptureInfo = g_rAEOutput.rCaptureMode[index];    
    }
    
    MY_LOG("[getCaptureParams] Capture idx:%d %d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", index, i4EVidx, a_rCaptureInfo.u4Eposuretime, a_rCaptureInfo.u4AfeGain, 
                   a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u2FrameRate, a_rCaptureInfo.i2FlareGain, a_rCaptureInfo.i2FlareOffset, a_rCaptureInfo.u4RealISO);                  
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    mCaptureMode = a_rCaptureInfo;
    MY_LOG("[updateCaptureParams] Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", mCaptureMode.u4Eposuretime, 
        mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);                  
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    MUINT8 iValue;
    AEMeterArea_T sAEMeteringArea = rWinSize;
   
    if (sAEMeteringArea.i4Left   < -1000)  {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000)  {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000)  {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000)  {sAEMeteringArea.i4Bottom = -1000;}

    if (sAEMeteringArea.i4Left   > 1000)  {sAEMeteringArea.i4Left   = 1000;}
    if (sAEMeteringArea.i4Right  > 1000)  {sAEMeteringArea.i4Right  = 1000;}
    if (sAEMeteringArea.i4Top    > 1000)  {sAEMeteringArea.i4Top    = 1000;}
    if (sAEMeteringArea.i4Bottom > 1000)  {sAEMeteringArea.i4Bottom = 1000;}   	 

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEMeteringAreaValue(sAEMeteringArea, &iValue);
    } else {
        MY_LOG("The AE algo class is NULL (25)\n");
    }
    
    *uYvalue = iValue;

//    MY_LOG("[getMeteringYvalue] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCapParamsByHDR()
{
    MUINT8 i;
    MINT32 i4AEMode;
    MUINT32 rAEHistogram[128];
    HDRExpSettingInputParam_T strHDRInputSetting;
    HDRExpSettingOutputParam_T strHDROutputSetting;
    AE_EXP_GAIN_MODIFY_T	rSensorInputData, rSensorOutputData;

    memset(rAEHistogram,   0, AE_HISTOGRAM_BIN*sizeof(MUINT32));

    strHDRInputSetting.u4MaxSensorAnalogGain = g_rAEInitInput.rAENVRAM.rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MaxAEExpTimeInUS = 500000; // 0.5sec
    strHDRInputSetting.u4MinAEExpTimeInUS = 500;  // 500us
    if(g_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit < 10000) {
        strHDRInputSetting.u4ShutterLineTime = 1000*g_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit;
    } else {
        strHDRInputSetting.u4ShutterLineTime = g_rAEInitInput.rAENVRAM.rDevicesInfo.u4CapExpUnit;            
    }
    
    strHDRInputSetting.u4MaxAESensorGain = 8*g_rAEInitInput.rAENVRAM.rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MinAESensorGain = g_rAEInitInput.rAENVRAM.rDevicesInfo.u4MinGain;
    strHDRInputSetting.u4ExpTimeInUS0EV = g_rAEOutput.rCaptureMode[0].u4Eposuretime;
    strHDRInputSetting.u4SensorGain0EV = (g_rAEOutput.rCaptureMode[0].u4AfeGain)*(g_rAEOutput.rCaptureMode[0].u4IspGain) >>10;;
    strHDRInputSetting.u1FlareOffset0EV = g_rAEOutput.rCaptureMode[0].i2FlareOffset;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEHistogram(rAEHistogram);
    } else {
        MY_LOG("The AE algo class is NULL (26)\n");
    }
    
    for (i = 0; i < AE_HISTOGRAM_BIN; i++) {
        strHDRInputSetting.u4Histogram[i] = rAEHistogram[i];
    }
    
    MY_LOG("[updateCapParamsByHDR] Input MaxSensorAnalogGain:%d MaxExpTime:%d MinExpTime:%d LineTime:%d MaxSensorGain:%d ExpTime:%d SensorGain:%d FlareOffset:%d\n",
        strHDRInputSetting.u4MaxSensorAnalogGain, strHDRInputSetting.u4MaxAEExpTimeInUS, strHDRInputSetting.u4MinAEExpTimeInUS, strHDRInputSetting.u4ShutterLineTime,
        strHDRInputSetting.u4MaxAESensorGain, strHDRInputSetting.u4ExpTimeInUS0EV, strHDRInputSetting.u4SensorGain0EV, strHDRInputSetting.u1FlareOffset0EV);
    
    for (i = 0; i < AE_HISTOGRAM_BIN; i+=8) {
        MY_LOG("[updateCapParamsByHDR] Input Histogram%d~%d:%d %d %d %d %d %d %d %d\n", i, i+7, strHDRInputSetting.u4Histogram[i],
           strHDRInputSetting.u4Histogram[i+1], strHDRInputSetting.u4Histogram[i+2], strHDRInputSetting.u4Histogram[i+3], strHDRInputSetting.u4Histogram[i+4],
           strHDRInputSetting.u4Histogram[i+5], strHDRInputSetting.u4Histogram[i+6], strHDRInputSetting.u4Histogram[i+7]);
    }
    
    getHDRExpSetting(strHDRInputSetting, strHDROutputSetting);
    m_strHDROutputInfo.u4OutputFrameNum = strHDROutputSetting.u4OutputFrameNum;
    
    for(i=0; i<m_strHDROutputInfo.u4OutputFrameNum; i++) {
        rSensorInputData.u4SensorExpTime = strHDROutputSetting.u4ExpTimeInUS[i];
        rSensorInputData.u4SensorGain = strHDROutputSetting.u4SensorGain[i];
        rSensorInputData.u4IspGain = 1024;

        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->switchSensorExposureGain(rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
        } else {
            MY_LOG("The AE algo class is NULL (27)\n");
        }

        g_rAEOutput.rCaptureMode[i].u4Eposuretime = rSensorOutputData.u4SensorExpTime;
        g_rAEOutput.rCaptureMode[i].u4AfeGain = rSensorOutputData.u4SensorGain;
        g_rAEOutput.rCaptureMode[i].u4IspGain = rSensorOutputData.u4IspGain;
        g_rAEOutput.rCaptureMode[i].u4RealISO = rSensorOutputData.u4ISOSpeed;
        g_rAEOutput.rCaptureMode[i].i2FlareOffset = strHDROutputSetting.u1FlareOffset[i];
        g_rAEOutput.rCaptureMode[i].i2FlareOffset = strHDROutputSetting.u1FlareOffset[i];
        g_rAEOutput.rCaptureMode[i].i2FlareOffset = strHDROutputSetting.u1FlareOffset[i];
    }

    m_strHDROutputInfo.u4FinalGainDiff[0] = strHDROutputSetting.u4FinalGainDiff[0];
    m_strHDROutputInfo.u4FinalGainDiff[1] = strHDROutputSetting.u4FinalGainDiff[1];
    m_strHDROutputInfo.u4TargetTone = strHDROutputSetting.u4TargetTone;

    MY_LOG("[updateCapParamsByHDR] OutputFrameNum : %d FinalGainDiff[0]:%d FinalGainDiff[1]:%d TargetTone:%d\n", m_strHDROutputInfo.u4OutputFrameNum, m_strHDROutputInfo.u4FinalGainDiff[0], m_strHDROutputInfo.u4FinalGainDiff[1], m_strHDROutputInfo.u4TargetTone);
    MY_LOG("[updateCapParamsByHDR] HDR Exposuretime[0] : %d Gain[0]:%d Flare[0]:%d\n", strHDROutputSetting.u4ExpTimeInUS[0], strHDROutputSetting.u4SensorGain[0], strHDROutputSetting.u1FlareOffset[0]);
    MY_LOG("[updateCapParamsByHDR] HDR Exposuretime[1] : %d Gain[1]:%d Flare[1]:%d\n", strHDROutputSetting.u4ExpTimeInUS[1], strHDROutputSetting.u4SensorGain[1], strHDROutputSetting.u1FlareOffset[1]);
    MY_LOG("[updateCapParamsByHDR] HDR Exposuretime[2] : %d Gain[2]:%d Flare[2]:%d\n", strHDROutputSetting.u4ExpTimeInUS[2], strHDROutputSetting.u4SensorGain[2], strHDROutputSetting.u1FlareOffset[2]);

    MY_LOG("[updateCapParamsByHDR] Modify Exposuretime[0] : %d AfeGain[0]:%d IspGain[0]:%d ISO:%d\n", g_rAEOutput.rCaptureMode[0].u4Eposuretime,
                 g_rAEOutput.rCaptureMode[0].u4AfeGain, g_rAEOutput.rCaptureMode[0].u4IspGain, g_rAEOutput.rCaptureMode[0].i2FlareOffset);
    MY_LOG("[updateCapParamsByHDR] Modify Exposuretime[1] : %d AfeGain[1]:%d IspGain[1]:%d ISO:%d\n", g_rAEOutput.rCaptureMode[1].u4Eposuretime,
                g_rAEOutput.rCaptureMode[1].u4AfeGain, g_rAEOutput.rCaptureMode[1].u4IspGain, g_rAEOutput.rCaptureMode[1].i2FlareOffset);
    MY_LOG("[updateCapParamsByHDR] Modify Exposuretime[2] : %d AfeGain[2]:%d IspGain[2]:%d ISO:%d\n", g_rAEOutput.rCaptureMode[2].u4Eposuretime,
                g_rAEOutput.rCaptureMode[2].u4AfeGain, g_rAEOutput.rCaptureMode[2].u4IspGain, g_rAEOutput.rCaptureMode[2].i2FlareOffset);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getHDRCapInfo(Hal3A_HDROutputParam_T & strHDROutputInfo)
{
    strHDROutputInfo = m_strHDROutputInfo;
    MY_LOG("[getHDRCapInfo] OutputFrameNum:%d FinalGainDiff[0]:%d FinalGainDiff[1]:%d TargetTone:%d\n", strHDROutputInfo.u4OutputFrameNum, strHDROutputInfo.u4FinalGainDiff[0], strHDROutputInfo.u4FinalGainDiff[1], strHDROutputInfo.u4TargetTone);            
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getRTParams(FrameOutputParam_T &a_strFrameInfo)
{
    a_strFrameInfo.u4FRameRate_x10 = g_rAEOutput.rPreviewMode.u2FrameRate;
    a_strFrameInfo.i4BrightValue_x10 = m_BVvalue;
    a_strFrameInfo.i4LightValue_x10 = (m_BVvalue + 50);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAECondition(MUINT32 i4AECondition)
{
    if(i4AECondition & m_u4AECondition) {
        return MTRUE;        
    } else {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getLCEPlineInfo(LCEInfo_T &a_rLCEInfo)
{
MUINT32 u4LCEStartIdx = 0, u4LCEEndIdx = 0;
    

    if(m_pIAeAlgo != NULL) {        
        m_pIAeAlgo->getAELCEIndexInfo(&u4LCEStartIdx, &u4LCEEndIdx);        
    } else {
        MY_LOG("The AE algo class is NULL (36)\n");
    }

    a_rLCEInfo.i4AEidxCur = m_i4AEidxCurrent;
    a_rLCEInfo.i4AEidxNext = m_i4AEidxNext;
    a_rLCEInfo.i4NormalAEidx = (MINT32) u4LCEStartIdx;
    a_rLCEInfo.i4LowlightAEidx = (MINT32) u4LCEEndIdx;

    if (m_bAEMgrDebugEnable) {
        MY_LOG("[getLCEPlineInfo] i4AEidxCur:%d i4AEidxNext:%d i4NormalAEidx:%d i4LowlightAEidx:%d\n", a_rLCEInfo.i4AEidxCur, a_rLCEInfo.i4AEidxNext, a_rLCEInfo.i4NormalAEidx, a_rLCEInfo.i4LowlightAEidx);    
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAERotateDegree(MINT32 i4RotateDegree)
{
    if(m_i4RotateDegree == i4RotateDegree) {  // the same degree
        return S_AE_OK;        
    }
    
    MY_LOG("setAERotateDegree:%d old:%d\n", i4RotateDegree, m_i4RotateDegree);
    m_i4RotateDegree = i4RotateDegree;    

    if(m_pIAeAlgo != NULL) {        
        if((i4RotateDegree == 90) || (i4RotateDegree == 270)){
            m_pIAeAlgo->setAERotateWeighting(MTRUE);        
        } else {
            m_pIAeAlgo->setAERotateWeighting(MFALSE);          
        }
    } else {
        MY_LOG("The AE algo class is NULL (28)\n");
    }
    
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateSensorISPParams(AE_STATE_T eNewAEState)
{
    MRESULT err;
    AE_INFO_T rAEInfo2ISP;
    MUINT32 u4IndexRatio;
        
    m_AEState = eNewAEState;
    
    switch(eNewAEState) 
    {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
            // sensor initial and send shutter / gain default value
            MY_LOG("[cameraPreviewInit] Sensor Dev:%d Shutter:%d Sensor Gain:%d Isp Gain:%d Flare:%d %d Frame Rate:%d\n", m_i4SensorDev, g_rAEOutput.rPreviewMode.u4Eposuretime, 
             g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u2FrameRate);

            err = AAASensorMgr::getInstance().setPreviewParams(g_rAEOutput.rPreviewMode.u4Eposuretime, g_rAEOutput.rPreviewMode.u4AfeGain);
            if (FAILED(err)) {
                MY_ERR("AAASensorMgr::getInstance().setPreviewParams fail\n");
                return err;
            }
            m_u4PreExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
            m_u4PreSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
            m_u4PreIspGain = g_rAEOutput.rPreviewMode.u4IspGain;

            // sensor isp gain and flare value
            ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(g_rAEOutput.rPreviewMode.u4IspGain>>1);
            if (FAILED(err)) {
                MY_ERR("setIspAEGain() fail\n");
                return err;
            }
    
            err = ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(g_rAEOutput.rPreviewMode.i2FlareGain, (-1*g_rAEOutput.rPreviewMode.i2FlareOffset));
            if (FAILED(err)) {
                MY_ERR("setIspFlare() fail\n");
                return err;
            }
            
            m_bAEStable = MFALSE;
            
            if(m_pIAeAlgo != NULL) {        
                m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
            } else {
                MY_LOG("The AE algo class is NULL (32)\n");
            }

            IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);

            // valdate ISP
            IspTuningMgr::getInstance().validatePerFrame(MFALSE);        

            if(m_AEState == AE_REINIT_STATE) {
                m_i4WaitVDNum = m_i4ShutterDelayFrames;    
            }           
            break;
        case AE_AUTO_FRAMERATE_STATE:
        case AE_MANUAL_FRAMERATE_STATE:
            if(m_i4WaitVDNum == 0x00) {   // restart                
                if((m_u4PreExposureTime == g_rAEOutput.rPreviewMode.u4Eposuretime) && (m_i4SensorGainDelayFrames != m_i4ShutterDelayFrames)) {   // reduce the set shutter eyele
                    m_i4WaitVDNum = m_i4SensorGainDelayFrames;    
                    AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rPreviewMode.u4AfeGain);    
                } else {                
                    AAASensorMgr::getInstance().setSensorExpTime(g_rAEOutput.rPreviewMode.u4Eposuretime);
                    m_i4WaitVDNum = m_i4ShutterDelayFrames;    
                    if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {                
                        AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rPreviewMode.u4AfeGain);
                    }       
                }
                
                // smooth isp gain
                if((m_u4PreExposureTime != 0x00) && (m_u4PreSensorGain !=0x00)) { 
                    u4IndexRatio = (g_rAEOutput.rPreviewMode.u4Eposuretime * g_rAEOutput.rPreviewMode.u4AfeGain) / 
                                                        m_u4PreExposureTime * g_rAEOutput.rPreviewMode.u4IspGain / m_u4PreSensorGain;

                    MY_LOG("[doPvAE] SmoothGain1:%d IndexRatio:%d Current:%d %d %d Pre:%d %d\n", m_u4SmoothIspGain, u4IndexRatio, g_rAEOutput.rPreviewMode.u4Eposuretime, 
                        g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, m_u4PreExposureTime, m_u4PreSensorGain);

                    if((u4IndexRatio > 1220) || (u4IndexRatio < 850)) {   // +-0.2EV
                        m_u4SmoothIspGain = (u4IndexRatio + g_rAEOutput.rPreviewMode.u4IspGain)>>1;
                        if(u4IndexRatio <= 360) {   // -1.5EV
                            m_u4SmoothIspGain = 1024;
                        } else if((u4IndexRatio > 1024) && (m_u4SmoothIspGain < g_rAEOutput.rPreviewMode.u4IspGain)) {
                            m_u4SmoothIspGain = g_rAEOutput.rPreviewMode.u4IspGain;
                        } else if((u4IndexRatio < 1024) && (m_u4SmoothIspGain > g_rAEOutput.rPreviewMode.u4IspGain)) {
                            m_u4SmoothIspGain = g_rAEOutput.rPreviewMode.u4IspGain;            
                        }
              
                        if(m_u4SmoothIspGain < 1024) {
                            m_u4SmoothIspGain = 1024;
                        } else if(m_u4SmoothIspGain > 8000) {
                            m_u4SmoothIspGain = 8000;           
                        }     
                
                        ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(m_u4SmoothIspGain>>1);          
                        MY_LOG("[doPvAE] SmoothGain2:%d IndexRatio:%d\n", m_u4SmoothIspGain, u4IndexRatio);
                    }
                }
                m_u4PreExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
                m_u4PreSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
                m_u4PreIspGain = g_rAEOutput.rPreviewMode.u4IspGain;
                MY_LOG("[doPvAE] State:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d SmoothGain:%d\n", eNewAEState, g_rAEOutput.rPreviewMode.u4Eposuretime, 
                    g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate, 
                    g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO, m_u4SmoothIspGain);               
            } else if(m_i4WaitVDNum > 0) {
                m_i4WaitVDNum--;
            }
                
            if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rPreviewMode.u4AfeGain);                
            } 
            if(m_i4WaitVDNum == m_i4IspGainDelayFrames) {
                ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(g_rAEOutput.rPreviewMode.u4IspGain>>1);
                ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(g_rAEOutput.rPreviewMode.i2FlareGain, (-1*g_rAEOutput.rPreviewMode.i2FlareOffset));                
                m_AEState = eNewAEState;                 
                    
                if(m_pIAeAlgo != NULL) {        
                    m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
                } else {
                    MY_LOG("The AE algo class is NULL (29)\n");
                }

                IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
                if(m_AEState == AE_MANUAL_FRAMERATE_STATE) {
                    // frame rate control
                    err = AAASensorMgr::getInstance().setSensorFrameRate(g_rAEOutput.rPreviewMode.u2FrameRate);
                    if (FAILED(err)) {
                        MY_ERR("AAASensorMgr::getInstance().setSensorFrameRate fail\n");
                        return err;
                    }
                }
            } 
            break;
        case AE_AF_STATE:
            // if the AF setting is the same with preview, skip the re-setting 
            if((g_rAEOutput.rPreviewMode.u4Eposuretime != g_rAEOutput.rAFMode.u4Eposuretime) || (g_rAEOutput.rPreviewMode.u4AfeGain != g_rAEOutput.rAFMode.u4AfeGain) || 
                (g_rAEOutput.rPreviewMode.u4IspGain != g_rAEOutput.rAFMode.u4IspGain)) {
                if(m_i4WaitVDNum == 0x00) {   // restart
                    AAASensorMgr::getInstance().setSensorExpTime(g_rAEOutput.rAFMode.u4Eposuretime);
                    m_i4WaitVDNum = m_i4ShutterDelayFrames;                 
                    if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {                
                        AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rAFMode.u4AfeGain);
                    }                     
                    MY_LOG("[doAFAE] Shutter:%d Sensor Gain:%d\n", g_rAEOutput.rAFMode.u4Eposuretime, g_rAEOutput.rAFMode.u4AfeGain);               
                } else if(m_i4WaitVDNum > 0) {            
                    m_i4WaitVDNum--;
                    if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                        AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rAFMode.u4AfeGain);                
                        MY_LOG("[doAFAE] Sensor Gain:%d\n", g_rAEOutput.rAFMode.u4AfeGain);               
                    } 
                    if(m_i4WaitVDNum == m_i4IspGainDelayFrames) {
                        ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(g_rAEOutput.rAFMode.u4IspGain>>1);
                        ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(g_rAEOutput.rAFMode.i2FlareGain, (-1*g_rAEOutput.rAFMode.i2FlareOffset));                
                        if(m_pIAeAlgo != NULL) {        
                            m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
                        } else {
                            MY_LOG("The AE algo class is NULL (30)\n");
                        }
                        rAEInfo2ISP.u4Eposuretime = g_rAEOutput.rAFMode.u4Eposuretime;
                        rAEInfo2ISP.u4AfeGain = g_rAEOutput.rAFMode.u4AfeGain;
                        rAEInfo2ISP.u4IspGain = g_rAEOutput.rAFMode.u4IspGain;
                        rAEInfo2ISP.u4RealISOValue = g_rAEOutput.rAFMode.u4RealISO;
                        IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);       
                        MY_LOG("[doAFAE] ISP Gain:%d\n", g_rAEOutput.rAFMode.u4IspGain);               
                        m_bAEStable = MTRUE;   
                        prepareCapParams();
                    }
                }
            }else {
                 m_i4WaitVDNum = 0x00;
                 m_bAEStable = MTRUE;
                 prepareCapParams();
                 MY_LOG("[doAFAE] AE Stable\n");               
            }
            MY_LOG("[doAFAE] AF Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d m_i4WaitVDNum:%d\n", g_rAEOutput.rAFMode.u4Eposuretime, 
                 g_rAEOutput.rAFMode.u4AfeGain, g_rAEOutput.rAFMode.u4IspGain, g_rAEOutput.rAFMode.u2FrameRate, 
                 g_rAEOutput.rAFMode.i2FlareGain, g_rAEOutput.rAFMode.i2FlareOffset, g_rAEOutput.rAFMode.u4RealISO, m_i4WaitVDNum);               
            MY_LOG("[doAFAE] Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", g_rAEOutput.rCaptureMode[0].u4Eposuretime, 
                 g_rAEOutput.rCaptureMode[0].u4AfeGain, g_rAEOutput.rCaptureMode[0].u4IspGain, g_rAEOutput.rCaptureMode[0].u2FrameRate, 
                 g_rAEOutput.rCaptureMode[0].i2FlareGain, g_rAEOutput.rCaptureMode[0].i2FlareOffset, g_rAEOutput.rCaptureMode[0].u4RealISO);               
            break;
        case AE_PRE_CAPTURE_STATE:
            m_bAEStable = MTRUE;
            prepareCapParams();
            MY_LOG("[doPreCapAE] State:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", eNewAEState, g_rAEOutput.rPreviewMode.u4Eposuretime, 
                    g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate, 
                    g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);               
            MY_LOG("[doPreCapAE] AF Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", g_rAEOutput.rAFMode.u4Eposuretime, 
                 g_rAEOutput.rAFMode.u4AfeGain, g_rAEOutput.rAFMode.u4IspGain, g_rAEOutput.rAFMode.u2FrameRate, 
                 g_rAEOutput.rAFMode.i2FlareGain, g_rAEOutput.rAFMode.i2FlareOffset, g_rAEOutput.rAFMode.u4RealISO);              
            MY_LOG("[doPreCapAE] Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", g_rAEOutput.rCaptureMode[0].u4Eposuretime, 
                 g_rAEOutput.rCaptureMode[0].u4AfeGain, g_rAEOutput.rCaptureMode[0].u4IspGain, g_rAEOutput.rCaptureMode[0].u2FrameRate, 
                 g_rAEOutput.rCaptureMode[0].i2FlareGain, g_rAEOutput.rCaptureMode[0].i2FlareOffset, g_rAEOutput.rCaptureMode[0].u4RealISO);               
            break;
        case AE_CAPTURE_STATE:
            MY_LOG("[doCapAE] Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", mCaptureMode.u4Eposuretime, 
            mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);               

            AAASensorMgr::getInstance().setCaptureParams(mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain);
            ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(mCaptureMode.u4IspGain>>1);
            ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(mCaptureMode.i2FlareGain, (-1*mCaptureMode.i2FlareOffset));

            // Update to isp tuning
            if(m_pIAeAlgo != NULL) {        
                m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
            } else {
                MY_LOG("The AE algo class is NULL (31)\n");
            }
            rAEInfo2ISP.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rAEInfo2ISP.u4AfeGain = mCaptureMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = mCaptureMode.u4IspGain;
            rAEInfo2ISP.u4RealISOValue = mCaptureMode.u4RealISO;
            IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
            // valdate ISP
            IspTuningMgr::getInstance().validatePerFrame(MFALSE);

            break;
    }
    return S_AE_OK;
}
