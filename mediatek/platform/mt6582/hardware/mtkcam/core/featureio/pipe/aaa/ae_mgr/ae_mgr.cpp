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
#include <mtkcam/common/faces.h>
#include <ae_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <kd_camera_feature.h>
//#include "CameraProfile.h"  // For CPTLog*()/AutoCPTLog class.
#include "aaa_state_flow_custom.h"

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
    , m_BVvalueWOStrobe(0)
    , m_i4EVvalue(0)
    , m_i4WaitVDNum(0)
    , m_i4RotateDegree(0)
    , m_i4TimeOutCnt(0)
    , m_i4ShutterDelayFrames(2)
    , m_i4SensorGainDelayFrames(2)
    , m_i4SensorGainDelayFramesWOShutter(1)
    , m_i4IspGainDelayFrames(0)
    , m_i4AEidxCurrent(0)
    , m_i4AEidxNext(0)
    , m_i2AEFaceDiffIndex(0)
    , m_u4PreExposureTime(0)
    , m_u4PreSensorGain(0)
    , m_u4PreIspGain(0)
    , m_u4SmoothIspGain(0)
    , m_u4AECondition(0)
    , m_u4DynamicFrameCnt(0)
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
    , m_bRestoreAE(MFALSE)
    , m_bOtherIPRestoreAE(MFALSE)
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
    , m_bIsAutoFlare(MTRUE)
    , m_bFrameUpdate(MFALSE)
    , m_i4ObjectTrackNum(0)
    , m_pIsAEActive(MNULL)
    , m_i4AECycleNum(getAECycleNum())
{
    memset(&m_AeMgrCCTConfig, 0, sizeof(AE_CCT_CFG_T));
    memset(&m_eZoomWinInfo, 0, sizeof(EZOOM_WINDOW_T));
    memset(&m_eAEMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_eAEFDArea, 0, sizeof(AEMeterArea_T));
    memset(&m_CurrentPreviewTable, 0, sizeof(strAETable));
    memset(&m_CurrentCaptureTable, 0, sizeof(strAETable));
    memset(&mCaptureMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));

    memset(&m_backupMeterArea, 0, sizeof(CameraMeteringArea_T));
    m_backupMeterArea.u4Count = 1;
    m_isAeMeterAreaEn=1;

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
        m_i4ShutterDelayFrames = i4SutterDelay;
        m_i4SensorGainDelayFrames = i4SensorGainDelay;
        if(i4IspGainDelay >= 1) {
            m_i4IspGainDelayFrames = i4IspGainDelay - 1; // for CQ0 1 delay frame
        }

        MY_LOG("Delay info is shutter :%d sensor gain:%d isp gain:%d Sensor Info:%d %d %d\n", m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
    } else {
        MY_LOG("Delay info is incorrectly :%d %d %d\n", i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
        m_i4ShutterDelayFrames = 0;
        m_i4SensorGainDelayFrames = 0;
        m_i4IspGainDelayFrames = 1; // for CQ0 1 delay frame
    }

    // Get sensor resolution
    err = getSensorResolution();
    if (FAILED(err)) {
        MY_ERR("getSensorResolution() fail\n");
        return err;
    }

    // Get NVRAM data
    err = getNvramData(m_i4SensorDev);
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

    m_i4SensorGainDelayFramesWOShutter = g_rAEInitInput.rAEPARAM.strAEParasetting.uAESensorGainDelayCycleWOShutter;

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

    m_bRealISOSpeed = 0;

    switch(m_eCamMode) {
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
        case eAppMode_VideoMode:     //  Video Mode
        case eAppMode_VtMode:           //  VT Mode
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        }


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
    m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
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
    strAEInput rAEInput;
    strAEOutput rAEOutput;

#if 0
    if(m_bLockExposureSetting == MTRUE) {
        MY_LOG("[cameraPreviewReinit] Lock sensor setting:%d\n", m_bLockExposureSetting);
        if(m_i4AEMinFps == m_i4AEMaxFps) {
            m_AEState = AE_MANUAL_FRAMERATE_STATE;
        } else {
            m_AEState = AE_AUTO_FRAMERATE_STATE;
        }
        return S_AE_OK;
    }
#endif

    if(m_eCamMode == eAppMode_EngMode) {  //  Engineer Mode
        m_bRealISOSpeed = 1;
        m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
    } else if(m_eCamMode == eAppMode_ZsdMode) {  //  ZSD Mode
        m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
        m_bRealISOSpeed = 0;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
        m_pIAeAlgo->setAECamMode(m_eAECamMode);
    } else {
        MY_LOG("The AE algo class is NULL (45)\n");
    }

    MY_LOG("[cameraPreviewReinit] Original m_eAECamMode:%d Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_eAECamMode, m_bRealISOSpeed, g_rAEOutput.rPreviewMode.u4ExposureMode, g_rAEOutput.rPreviewMode.u4Eposuretime,
                g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate,
                g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);
    
    if(m_bEnableAE) {
        if(m_eAECamMode != LIB3A_AECAM_MODE_VIDEO) {
            rAEInput.eAeState = AE_STATE_INIT;
            rAEInput.pAESatisticBuffer = NULL;
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            } else {
                MY_LOG("The AE algo class is NULL (35)\n");
            }
            MY_LOG("[cameraPreviewReinit] init m_eAECamMode:%d Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_eAECamMode, m_bRealISOSpeed, g_rAEOutput.rPreviewMode.u4ExposureMode, rAEOutput.EvSetting.u4Eposuretime,
                rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u2FrameRate,
                rAEOutput.i2FlareGain, rAEOutput.i2FlareOffset, rAEOutput.u4ISO);
        }
  
        rAEInput.eAeState = AE_STATE_AELOCK;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("The AE algo class is NULL (40)\n");
        }

        MY_LOG("[cameraPreviewReinit] Lock Real ISO:%d Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                m_bRealISOSpeed, g_rAEOutput.rPreviewMode.u4ExposureMode, rAEOutput.EvSetting.u4Eposuretime,
                rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u2FrameRate,
                rAEOutput.i2FlareGain, rAEOutput.i2FlareOffset, rAEOutput.u4ISO);

        copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);

        MY_LOG("[cameraPreviewReinit] Exp mode: %d, Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                    g_rAEOutput.rPreviewMode.u4ExposureMode, g_rAEOutput.rPreviewMode.u4Eposuretime,
                    g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate,
                    g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);

    } else {
        MY_LOG("[cameraPreviewReinit] AE disable\n");    
    }

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

void AeMgr::setAeMeterAreaEn(int en)
{
	XLOGD("setAeMeterAreaEn en=%d, m_isAeMeterAreaEn=%d",en, m_isAeMeterAreaEn);
	m_isAeMeterAreaEn=en;
	setAEMeteringArea(&m_backupMeterArea);

}
MRESULT AeMgr::setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea)
{

		XLOGD("setAEMeteringArea m_isAeMeterAreaEn=%d",m_isAeMeterAreaEn);


		CameraMeteringArea_T meterArea;
		memcpy(&m_backupMeterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));
		memcpy(&meterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));




		if(m_isAeMeterAreaEn==0)
		{
			memset(&meterArea, 0, sizeof(CameraMeteringArea_T));
			meterArea.u4Count = 1;
		}


    MUINT32 i;
    MBOOL bAreaChage = MFALSE;
    MUINT32 u4AreaCnt;
    AEMeteringArea_T *sAEMeteringArea = (AEMeteringArea_T* )&meterArea;

    if (sAEMeteringArea->u4Count <= 0) {
//        MY_LOG("No AE Metering area cnt: %d\n", sAEMeteringArea->u4Count);
        return S_AE_OK;
    } else if (sAEMeteringArea->u4Count > MAX_METERING_AREAS) {
        MY_ERR("The AE Metering area cnt error: %d\n", sAEMeteringArea->u4Count);
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

    if((m_eAEFDArea.i4Left != pFaces->faces->rect[0]) || (m_eAEFDArea.i4Right != pFaces->faces->rect[2]) || (m_eAEFDArea.i4Top != pFaces->faces->rect[1]) ||
                  (m_eAEFDArea.i4Bottom != pFaces->faces->rect[3]) || (m_eAEFDArea.i4Weight != pFaces->number_of_faces)) {
        m_eAEFDArea.i4Left = pFaces->faces->rect[0];
        m_eAEFDArea.i4Right = pFaces->faces->rect[2];
        m_eAEFDArea.i4Top = pFaces->faces->rect[1];
        m_eAEFDArea.i4Bottom = pFaces->faces->rect[3];
        m_eAEFDArea.i4Weight = pFaces->number_of_faces;

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
MRESULT AeMgr::setOTInfo(MVOID* a_sOT)
{
    MtkCameraFaceMetadata *pOTWindow = (MtkCameraFaceMetadata *)a_sOT;
    MBOOL bEnableObjectTracking;

    if( m_i4ObjectTrackNum != pOTWindow->number_of_faces) {
        MY_LOG("[setOTInfo] Object tracking:%d %d\n", pOTWindow->number_of_faces, m_i4ObjectTrackNum);
        m_i4ObjectTrackNum = pOTWindow->number_of_faces;
        if(pOTWindow->number_of_faces > 0) { // Object tracking enable
            bEnableObjectTracking = MTRUE;
        } else {
            bEnableObjectTracking = MFALSE;
        }

        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAEObjectTracking(bEnableObjectTracking);
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
        else if(i4EVValue < -120) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n13; }
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
    AE_METERING_T eNewAEMeteringMode = static_cast<AE_METERING_T>(u4NewAEMeteringMode);
    LIB3A_AE_METERING_MODE_T eAEMeteringMode;

    if ((eNewAEMeteringMode < AE_METERING_BEGIN) || (eNewAEMeteringMode >= NUM_OF_AE_METER)) {
        MY_ERR("Unsupport AE Metering Mode: %d\n", eNewAEMeteringMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEMeteringMode) {
        case AE_METERING_MODE_SOPT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_SOPT;
            break;
        case AE_METERING_MODE_AVERAGE:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_AVERAGE;
            break;
        case AE_METERING_MODE_CENTER_WEIGHT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
        default:
            MY_LOG("The AE metering mode enum value is incorrectly:%d\n", eNewAEMeteringMode);
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
    }

    if (m_eAEMeterMode != eAEMeteringMode) {
        m_eAEMeterMode = eAEMeteringMode;
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
        case 50:
            eAEISOSpeed = LIB3A_AE_ISO_SPEED_50;
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
        return S_AE_OK;
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
#if 0
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
#else
    EAppMode eNewAECamMode = static_cast<EAppMode>(u4NewAECamMode);

    if (m_eCamMode != eNewAECamMode) {
        m_eCamMode = eNewAECamMode;
        MY_LOG("m_eCamMode:%d AECamMode:%d \n", m_eCamMode, m_eAECamMode);
    }
#endif
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
        m_bAElimitor = MFALSE;
        MY_LOG("m_eAppShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELimiterMode(MBOOL bAELimter)
{
    m_bAElimitor = bAELimter;
    MY_LOG("ShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);
    } else {
        MY_LOG("The AE algo class is NULL (10)\n");
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
//        case AE_MODE_BACKLIGHT:
//            eAEMode = LIB3A_AE_MODE_BACKLIGHT;
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
MRESULT AeMgr::doPvAE(MINT32 i4FrameCount, MVOID *pAEStatBuf, MBOOL bVideoMode)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;
MUINT32 u4DFpsThres = 0;


    m_bAEReadyCapture = MFALSE;  // reset capture flag
    m_i4TimeOutCnt = MFALSE;  // reset timeout counter
    m_bFrameUpdate = MTRUE;

    m_pIsAEActive = getAEActiveCycle(bVideoMode);

    if(m_pIAeAlgo != NULL) {
        AaaTimer localTimer("setAESatisticBufferAddr", m_i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
        m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
        localTimer.printTime();
        if(m_bIsAutoFlare == MTRUE) {
            if (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_FLARE) {
                if(m_bEnableAE && (m_bAEStable == MTRUE)) {                
                   AaaTimer localTimer("DoPreFlare", m_i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
                   m_pIAeAlgo->DoPreFlare(pAEStatBuf);
                   UpdateFlare2ISP();
                   localTimer.printTime();
                }
            }
        }
    } else {
        MY_LOG("The AE algo class is NULL (13)\n");
    }

    if(m_bEnableAE) {
//        if(m_i4WaitVDNum == 0x00) {
        if (((m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO) || 
              (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE)) && (m_i4WaitVDNum > m_i4IspGainDelayFrames)){    
            if(m_bAELock) {
                if(m_bOneShotAEBeforeLock == MTRUE) {
                    //rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    rAEInput.eAeState = AE_STATE_AELOCK;
                    m_bOneShotAEBeforeLock = MFALSE;
                } else {
                    rAEInput.eAeState = AE_STATE_AELOCK;
                }
            } else {
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
            }

			int bRestore=0;
            if(m_eZoomWinInfo.bZoomChange == MTRUE) {
                if(m_pIAeAlgo != NULL) {
                    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &g_rAEStatCfg);
                } else {
                    MY_LOG("The AE algo class is NULL (14)\n");
                }

                m_eZoomWinInfo.bZoomChange = MFALSE;

                // Update AE histogram window config
                ISP_MGR_AE_STAT_HIST_CONFIG_T::getInstance((ESensorDev_T)m_i4SensorDev).config(g_rAEStatCfg);
//                m_i4WaitVDNum = 0x02;
            } else {
                rAEInput.pAESatisticBuffer = pAEStatBuf;
                if(m_pIAeAlgo != NULL) {
                    if(m_bRestoreAE == MFALSE) {
                    AaaTimer localTimer("handleAE", m_i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
                    m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    localTimer.printTime();
                        copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
                        if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD) {   // copy the information to
                            copyAEInfo2mgr(&g_rAEOutput.rCaptureMode[0], &rAEOutput);
                        }
                        m_i4WaitVDNum = 0; // reset the delay frame
                    } else {
                        bRestore=1;
                        m_bRestoreAE = MFALSE;
                        MY_LOG("Restore AE, skip AE one time\n");
                    }
                } else {
                    MY_LOG("The AE algo class is NULL (15)\n");
                }

                m_bAEStable = rAEOutput.bAEStable;
                if(m_bLockExposureSetting == MTRUE) {
                    MY_LOG("[doPvAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
                    return S_AE_OK;
                }

                // Update the preview or video state
                if(m_eAECamMode == LIB3A_AECAM_MODE_VIDEO) {
                    if (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE) {    
                        UpdateSensorISPParams(AE_MANUAL_FRAMERATE_STATE);
                    } else {
                        m_AEState = AE_MANUAL_FRAMERATE_STATE;
                    }
                } else {
                    if (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE) {                    
                        if(bRestore == 0) {
                            UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                        } else {
                            UpdateSensorISPParams(AE_AF_RESTORE_STATE);
                        }
                    } else {
                        m_AEState = AE_AUTO_FRAMERATE_STATE;                    
                    }
                }
            }
        } else {
            if(m_pIAeAlgo != NULL) {
                MY_LOG("[doPvAE]AE Wait Vd frame:%d Enable:%d avgY:%d m_AEState:%d\n", m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue(), m_AEState);
            }
            if((m_AEState == AE_AUTO_FRAMERATE_STATE) || (m_AEState == AE_MANUAL_FRAMERATE_STATE)) {
                // continue update the preview or video state
                UpdateSensorISPParams(m_AEState);
            } else {
                m_i4WaitVDNum = m_i4IspGainDelayFrames+1;
            }
        }
    }else {
//        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
        MY_LOG("[doPvAE] AE don't enable Enable:%d Y:%d\n", m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue());
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAE(MINT32 i4FrameCount, MVOID *pAEStatBuf)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;

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

    m_pIsAEActive = getAEActiveCycle((m_i4AEMinFps >= AE_HIGH_FPS_THRES)? 1: 0);

    if(m_bEnableAE) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
        } else {
            MY_LOG("The AE algo class is NULL (16)\n");
        }
        m_bAEStable = MFALSE;

//        if(m_i4WaitVDNum == 0x00) {
        if (((m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO) || 
              (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE)) && (m_i4WaitVDNum > m_i4IspGainDelayFrames)){    
            MY_LOG("[doAFAE] AE_STATE_ONE_SHOT\n");
            rAEInput.eAeState = AE_STATE_ONE_SHOT;
            rAEInput.pAESatisticBuffer = pAEStatBuf;
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
                if(rAEOutput.bAEStable == MTRUE) {
                    rAEInput.eAeState = AE_STATE_AFASSIST;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    } else {
                        MY_LOG("The AE algo class is NULL (18)\n");
                    }
                    copyAEInfo2mgr(&g_rAEOutput.rAFMode, &rAEOutput);
                    m_AEState = AE_AF_STATE;
                }
                m_i4WaitVDNum = 0x00;
            } else {
                MY_LOG("The AE algo class is NULL (17)\n");
            }
            
            if(m_bLockExposureSetting == MTRUE) {
                MY_LOG("[doAFAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
                return S_AE_OK;
            }
            
            // AE is stable, change to AF state            
            if (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE) {                    
                if(m_AEState == AE_AF_STATE) {
                    UpdateSensorISPParams(AE_AF_STATE);
                } else {
                    // Using preview state to do AE before AE stable
                    UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                }
            } else {
                m_AEState = AE_AUTO_FRAMERATE_STATE;
            }
        } else {
            if(m_pIAeAlgo != NULL) {
                MY_LOG("[doAFAE]AE Wait Vd frame:%d Enable:%d avgY:%d State:%d\n", m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue(), m_AEState);
            }
            // continue update for preview or AF state
            UpdateSensorISPParams(m_AEState);
        }
    } else {
        m_bAEStable = MTRUE;
        MY_LOG("[doAFAE] AE don't enable Enable:%d\n", m_bEnableAE);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPreCapAE(MBOOL bIsStrobeFired, MINT32 i4FrameCount, MVOID *pAEStatBuf)
{
strAEInput rAEInput;
strAEOutput rAEOutput;
AE_INFO_T rAEInfo2ISP;

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

//                if(m_i4WaitVDNum == 0x00) {
            if (((m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO) || 
                  (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE)) && (m_i4WaitVDNum > m_i4IspGainDelayFrames)){    
                    MY_LOG("[doPreCapAE] AE_STATE_ONE_SHOT\n");
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    rAEInput.pAESatisticBuffer = pAEStatBuf;
                    if(m_pIAeAlgo != NULL) {
                        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
                    } else {
                        MY_LOG("The AE algo class is NULL (33)\n");
                    }
                    copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
                    m_i4WaitVDNum = 0x00;

                    if(m_bLockExposureSetting == MTRUE) {
                        MY_LOG("[doPreCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
                        return S_AE_OK;
                    }

                    // AE is stable, update capture info
                    if(rAEOutput.bAEStable == MTRUE) {
                        UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                        doBackAEInfo();   // do back up AE for Precapture AF state.
                    } else {
                        // Using preview state to do AE before AE stable
                        if (m_pIsAEActive[i4FrameCount % m_i4AECycleNum] & AE_CYCLE_ALGO_CONFIGURE) {   
                            UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                        }
                    }
                } else {
                    MY_LOG("[doPreCapAE]AE Wait Vd frame:%d Enable:%d State:%d\n", m_i4WaitVDNum, m_bEnableAE, m_AEState);
                    // continue update for preview or AF state
                    UpdateSensorISPParams(m_AEState);
               }
            } else {
                MY_LOG("[doPreCapAE] AE stable already\n");
                UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                doBackAEInfo();   // do back up AE for Precapture AF state.
            }
        } else {
            MY_LOG("[doPreCapAE] Do Nothing Ready:%d isStrobe:%d\n", m_bAEReadyCapture, bIsStrobeFired);
        }
    } else {
        m_bAEStable = MTRUE;
        MY_LOG("[doPreCapAE] AE don't enable Enable:%d\n", m_bEnableAE);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapAE()
{
#if 0
    if(m_bLockExposureSetting == MTRUE) {
        MY_LOG("[doCapAE] Lock sensor setting:%d\n", m_bLockExposureSetting);
        return S_AE_OK;
    }
#endif

    UpdateSensorISPParams(AE_CAPTURE_STATE);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int g_defaultFrameRate;
static int g_rExp;
static int g_rAfe;
static int g_rIsp;

static int g_rExpOn2;
static int g_rAfeOn2;
static int g_rIspOn2;

static int g_rIspAFLampOff;
static int g_rFrm;
static AE_MODE_CFG_T g_lastAFInfo;

int mapAFLampOffIsp2(int rollFrmTime, int exp, int afe, int isp, int expOn, int afeOn, int ispOn, int* expOn2=0, int* afeOn2=0, int* ispOn2=0)
{
	XLOGD("AFLampExp, expOn: %d %d %d %d %d %d",exp, afe, isp, expOn, afeOn, ispOn);
	double rDif;
	rDif = (double)exp*afe*isp/expOn/afeOn/ispOn;

	double ispIncRat;
	if(rDif<1.2)
		ispIncRat=1.1;
	else if(rDif<1.5)
		ispIncRat=1.15;
	else if(rDif<2)
		ispIncRat=1.18;
	else
		ispIncRat=1.22;


	int rollFrameTime;

	int exp2;
	int afe2;
	int isp2;

	if(expOn2!=0)
		*expOn2 = expOn;
	if(afeOn2!=0)
		*afeOn2 = afeOn;

	double r;
	r = (rDif*exp)/ (rDif*(exp-rollFrmTime/2)+rollFrmTime/2);

	r = r*1.03;

	XLOGD("AFLampExpp ispIncRat=%lf r=%lf", ispIncRat, r);

	r=1;

	return ispOn*r;
}

int mapAFLampOffIsp(int exp, int afe, int isp, int expOn, int afeOn, int ispOn)
{
	XLOGD("AFLampExp, expOn: %d %d %d %d %d %d",exp, afe, isp, expOn, afeOn, ispOn);
	double rDif;
	rDif = (double)exp*afe*isp/expOn/afeOn/ispOn;

	double ispIncRat;
	if(rDif<1.2)
		ispIncRat=1.1;
	else if(rDif<1.5)
		ispIncRat=1.15;
	else if(rDif<2)
		ispIncRat=1.18;
	else
		ispIncRat=1.22;

	return ispOn*ispIncRat;
}

MRESULT AeMgr::doBackAEInfo()
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    if(m_bEnableAE) {
        MY_LOG("doBackAEInfo\n");

        rAEInput.eAeState = AE_STATE_BACKUP_PREVIEW;
        rAEInput.pAESatisticBuffer = NULL;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("The AE algo class is NULL (34)\n");
        }
    } else {
        MY_LOG("[doBackAEInfo] AE don't enable Enable:%d\n", m_bEnableAE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AeMgr::setExp(int exp)
{
	AAASensorMgr::getInstance().setSensorExpTime(exp);
}
void AeMgr::setAfe(int afe)
{
	AAASensorMgr::getInstance().setSensorGain(afe);
}

void AeMgr::setIsp(int isp)
{
	AE_INFO_T rAEInfo2ISP;
	rAEInfo2ISP.u4Eposuretime = 60000;
    rAEInfo2ISP.u4AfeGain = 4000;
    rAEInfo2ISP.u4IspGain = isp;
    rAEInfo2ISP.u4RealISOValue = isp*4/1024*100;
    IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);

	ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(isp>>1);
    // valdate ISP
    IspTuningMgr::getInstance().validatePerFrame(MFALSE);


	//ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(isp>>1);
	// valdate ISP
	//IspTuningMgr::getInstance().validatePerFrame(MFALSE);
}

void AeMgr::setRestore(int frm)
{
	MY_LOG("setRestore line=%d frm=%d\n", __LINE__, frm);
	m_bOtherIPRestoreAE = MTRUE;
	int expSetFrm;
	int afeSetFrm;
	int ispSetFrm;

#if 0
	int maxFrmDelay=0;
	maxFrmDelay = m_i4ShutterDelayFrames;
	if(maxFrmDelay<m_i4SensorGainDelayFrames)
		maxFrmDelay=m_i4SensorGainDelayFrames;
	if(maxFrmDelay<m_i4IspGainDelayFrames)
		maxFrmDelay=m_i4IspGainDelayFrames;
	expSetFrm = maxFrmDelay-m_i4ShutterDelayFrames;
	afeSetFrm = maxFrmDelay-m_i4SensorGainDelayFrames;
	ispSetFrm = maxFrmDelay-m_i4IspGainDelayFrames;
#else
	int minFrmDelay=0;
	minFrmDelay = m_i4ShutterDelayFrames;
	if(minFrmDelay>m_i4SensorGainDelayFrames)
		minFrmDelay=m_i4SensorGainDelayFrames;
	if(minFrmDelay>m_i4IspGainDelayFrames)
		minFrmDelay=m_i4IspGainDelayFrames;
	expSetFrm = m_i4ShutterDelayFrames-minFrmDelay;
	afeSetFrm = m_i4SensorGainDelayFrames-minFrmDelay;
	ispSetFrm = m_i4IspGainDelayFrames-minFrmDelay;
#endif

	if(frm==expSetFrm) //on exp
	{
	}
	else if(frm==1+expSetFrm) //off exp
	{
		setExp(g_rExp);

		int frmRate;

		if(g_rExp<33000)
			frmRate = 1000000*10/(40000+33000);
		else
			frmRate = 1000000*10/(40000+g_rExp);

		frmRate = 1000000*10/(40000+g_rExp);
		g_defaultFrameRate = AAASensorMgr::getInstance().getPreviewDefaultFrameRate(m_eAECamMode == LIB3A_AECAM_MODE_ZSD);
//#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
		if (isSupportSetMaxFrameRate()) AAASensorMgr::getInstance().setPreviewMaxFrameRate(frmRate, m_eAECamMode == LIB3A_AECAM_MODE_ZSD);
//#else
//#endif
		MY_LOG("setRestore line=%d default frame=%d, frmRate=%d exp=%d\n", __LINE__, g_defaultFrameRate, frmRate, g_rExp);


	}
	else if(frm==2+expSetFrm) //next of off exp
	{
//#ifdef MTK_AF_SYNC_RESTORE_SUPPORT
		if (isSupportSetMaxFrameRate()) AAASensorMgr::getInstance().setPreviewMaxFrameRate(g_defaultFrameRate, m_eAECamMode == LIB3A_AECAM_MODE_ZSD);
//#else
//#endif
		setExp(g_rExp);
		MY_LOG("setRestore line=%d default frame=%d\n", __LINE__, g_defaultFrameRate);
	}




	if(frm==afeSetFrm) //on afe
	{
	}
	else if(frm==1+afeSetFrm) //off afe
		setAfe(g_rAfe);

	if(frm==ispSetFrm) //on isp
		setIsp(g_rIspAFLampOff);
	else if(frm==1+ispSetFrm) //off isp
		setIsp(g_rIsp);

	MY_LOG("setRestore set frame %d %d %d\n", expSetFrm, afeSetFrm, ispSetFrm );
}


MRESULT AeMgr::doRestoreAEInfo()
{
strAEInput rAEInput;
strAEOutput rAEOutput;

    if(m_bEnableAE) {
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
    MY_LOG("[getPreviewParams3] Exp. mode: %d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                   g_rAEOutput.rPreviewMode.u4ExposureMode, g_rAEOutput.rPreviewMode.u4AfeGain,
                   g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate, g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);

    g_rExp = g_rAEOutput.rPreviewMode.u4Eposuretime;
    g_rAfe = g_rAEOutput.rPreviewMode.u4AfeGain;
    g_rIsp = g_rAEOutput.rPreviewMode.u4IspGain;


     m_u4PreExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
     m_u4PreSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;

    //g_rAEOutput.rAFMode
		int rollFrmTime;
		if(m_eAECamMode == LIB3A_AECAM_MODE_ZSD)
			rollFrmTime = 66000;
		else
			rollFrmTime = 33000;

    g_rIspAFLampOff = mapAFLampOffIsp2(rollFrmTime, g_rExp, g_rAfe, g_rIsp,
                                     g_rAEOutput.rAFMode.u4Eposuretime, g_rAEOutput.rAFMode.u4AfeGain, g_rAEOutput.rAFMode.u4IspGain);

    m_i4WaitVDNum = 0;   // reset the Vd count
    m_bRestoreAE = MTRUE; // restore AE
    }else {
        MY_LOG("[doRestoreAEInfo] AE don't enable Enable:%d\n", m_bEnableAE);
    }
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    AE_MODE_CFG_T a_rCaptureInfo;

    if(m_bIsAutoFlare == FALSE) {
        return S_AE_OK;
    }

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->setAESatisticBufferAddr(pAEStatBuf);
        m_pIAeAlgo->CalculateCaptureFlare(pAEStatBuf,bIsStrobe);

        rAEInput.eAeState = AE_STATE_POST_CAPTURE;
        rAEInput.pAESatisticBuffer = NULL;
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);

        if(m_bStrobeOn == TRUE) {
            if(g_rAEInitInput.rAENVRAM.rCCTConfig.bEnableStrobeThres == MFALSE) {
                mCaptureMode.i2FlareOffset = g_rAEInitInput.rAENVRAM.rCCTConfig.u4StrobeFlareOffset;
                mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN  - g_rAEInitInput.rAENVRAM.rCCTConfig.u4StrobeFlareOffset));
            } else {
        mCaptureMode.i2FlareOffset=rAEOutput.i2FlareOffset;
        mCaptureMode.i2FlareGain=rAEOutput.i2FlareGain;
            }
        } else {
            mCaptureMode.i2FlareOffset=rAEOutput.i2FlareOffset;
            mCaptureMode.i2FlareGain=rAEOutput.i2FlareGain;
            MY_LOG("i2FlareOffset:%d i2FlareGain:%d\n", mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain);                
        }

        UpdateSensorISPParams(AE_POST_CAPTURE_STATE);
    } 	else {
        MY_LOG("The AE algo class is NULL (38)\n");
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getLVvalue(MBOOL isStrobeOn)
{
    if(isStrobeOn == MTRUE) {
        return (m_BVvalue + 50);
    } else {
        return (m_BVvalueWOStrobe + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getCaptureLVvalue()
{
    MINT32 i4LVValue_10x;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->CalculateCaptureLV(&i4LVValue_10x);
    } else {
        MY_LOG("The AE algo class is NULL (39)\n");
    }

    return (i4LVValue_10x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo)
{
    if(m_pIAeAlgo != NULL) {
        MY_LOG("[getDebugInfo] \n");       
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
    if(sAEOutputInfo->u4IspGain < 1024) {
        MY_LOG("[copyAEInfo2mgr] ISP gain too small:%d\n", sAEOutputInfo->u4IspGain);    
        sAEOutputInfo->u4IspGain = 1024;
    }
    sAEOutputInfo->u2FrameRate = sAEInfo->u2FrameRate;
    sAEOutputInfo->u4RealISO = sAEInfo->u4ISO;

    if(m_bIsAutoFlare == TRUE) {
        sAEOutputInfo->i2FlareOffset = sAEInfo->i2FlareOffset;
        sAEOutputInfo->i2FlareGain = sAEInfo->i2FlareGain;
    } else {     // flare disable, don't update.
        MY_LOG("[copyAEInfo2mgr] i2FlareOffset:%d i2FlareGain:%d\n", sAEOutputInfo->i2FlareOffset, sAEOutputInfo->i2FlareGain);
    }

    m_BVvalue = sAEInfo->Bv;
    
    if(m_bStrobeOn == MFALSE) {
        m_BVvalueWOStrobe = sAEInfo->Bv;
    }
    
    m_i4EVvalue = sAEInfo->i4EV;
    m_u4AECondition = sAEInfo->u4AECondition;
    m_i4AEidxCurrent = sAEInfo->i4AEidxCurrent;
    m_i4AEidxNext = sAEInfo->i4AEidxNext;
    m_i2AEFaceDiffIndex = sAEInfo->i2FaceDiffIndex;

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
MRESULT AeMgr::getNvramData(MINT32 i4SensorDev)
{
    MY_LOG("[getNvramData] Sensor id:%d\n", i4SensorDev);

    if (FAILED(NvramDrvMgr::getInstance().init(i4SensorDev))) {
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
    m_eZoomWinInfo.u4YOffset = 0;
    m_eZoomWinInfo.u4XWidth = AWB_WINDOW_NUM_X;
    m_eZoomWinInfo.u4YHeight = AWB_WINDOW_NUM_Y;

    switch(m_i4SensorDev)
    {
        case ESensorDev_Sub:
            g_rAEInitInput.eSensorDev = AE_SENSOR_SUB;
            break;
        case ESensorDev_MainSecond:
            g_rAEInitInput.eSensorDev = AE_SENSOR_MAIN2;
            break;
        default:
            MY_LOG("[setSensorDev] Wrong sensor type:%d \n", m_i4SensorDev);
        case ESensorDev_Main:
            g_rAEInitInput.eSensorDev = AE_SENSOR_MAIN;
            break;
    }

    g_rAEInitInput.rEZoomWin = m_eZoomWinInfo;
    g_rAEInitInput.eAEMeteringMode = m_eAEMeterMode;
    g_rAEInitInput.eAEMode = m_eAEMode;
    g_rAEInitInput.eAECamMode = m_eAECamMode;
    g_rAEInitInput.eAEFlickerMode = m_eAEFlickerMode;
    g_rAEInitInput.eAEAutoFlickerMode = m_eAEAutoFlickerMode;
    g_rAEInitInput.eAEEVcomp = m_eAEEVcomp;
    g_rAEInitInput.eAEISOSpeed = m_eAEISOSpeed;
    g_rAEInitInput.i4AEMaxFps = m_i4AEMaxFps;
    g_rAEInitInput.i4AEMinFps = m_i4AEMinFps;
    
    MY_LOG("Sensor id:%d AE max block width:%d heigh:%d AE mter:%d CamMode:%d AEMode:%d Flicker :%d %d EVcomp:%d ISO:%d %d MinFps:%d MaxFps:%d Limiter:%d\n", 
             g_rAEInitInput.eSensorDev, g_rAEInitInput.i4AEMaxBlockWidth, g_rAEInitInput.i4AEMaxBlockHeight, m_eAEMeterMode, m_eAECamMode, m_eAEMode, 
             m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eAEEVcomp, m_eAEISOSpeed, m_bRealISOSpeed, m_i4AEMinFps, m_i4AEMaxFps, m_bAElimitor);
    
    m_pIAeAlgo = IAeAlgo::createInstance();
    if (!m_pIAeAlgo) {
        MY_ERR("AeAlgo::createInstance() fail \n");
        return E_AE_ALGO_INIT_ERR;
    }

    m_pIAeAlgo->lockAE(m_bAELock);
    m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
    m_pIAeAlgo->setAEVideoDynamicEnable(MFALSE);
    m_bVideoDynamic = MFALSE;
    m_pIAeAlgo->initAE(&g_rAEInitInput, &rAEOutput, &g_rAEStatCfg);
    copyAEInfo2mgr(&g_rAEOutput.rPreviewMode, &rAEOutput);
    m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);  // update limiter
    m_pIAeAlgo->setAEObjectTracking(MFALSE);;
    m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
    
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
    if(g_p3ANVRAM != NULL) {
        a_rDeviceInfo = g_p3ANVRAM->rAENVRAM.rDevicesInfo;
    } else {
        MY_LOG("[getSensorDeviceInfo] NVRAM Data is NULL\n");
    }
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

    if(g_rAEInitInput.rAEPARAM.strAEParasetting.bEV0TriggerStrobe == MTRUE) {         // The strobe trigger by the EV 0 index
        i4Bv = m_BVvalueWOStrobe;
    } else {
        if(g_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]) {
            if(m_pIAeAlgo != NULL) {
                i4Bv = m_BVvalueWOStrobe - m_pIAeAlgo->getSenstivityDeltaIndex(1024 *1024/ g_rAEInitInput.rAEPARAM.pEVValueArray[m_eAEEVcomp]);
            } else {
                i4Bv = m_BVvalueWOStrobe;
                MY_LOG("The AE algo class is NULL (22)\n");
            }
        }
    }

    bStrobeBVTrigger = (i4Bv < strCurrentCaptureTable.i4StrobeTrigerBV)?MTRUE:MFALSE;

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
    if(m_bStrobeOn == MTRUE) {   // capture on, get capture parameters for flare again.
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
MRESULT AeMgr::updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext)
{
    mPreviewMode = a_rPreviewInfo;
    m_i4WaitVDNum = 0; // reset the delay frame

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->updateAEIndex(i4AEidxNext);
    } else {
        MY_LOG("The AE algo class is NULL (42)\n");
    }

    MY_LOG("[updatePreviewParams] m_i4SensorDev:%d i4AEidxNext:%d Exp. mode = %d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        m_i4SensorDev, i4AEidxNext, mPreviewMode.u4ExposureMode, mPreviewMode.u4Eposuretime,
        mPreviewMode.u4AfeGain, mPreviewMode.u4IspGain, mPreviewMode.u2FrameRate, mPreviewMode.i2FlareGain, mPreviewMode.i2FlareOffset, mPreviewMode.u4RealISO);

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

    MY_LOG("[getCaptureParams] Capture idx:%d %d Exp. mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", index, i4EVidx, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime, a_rCaptureInfo.u4AfeGain,
                   a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u2FrameRate, a_rCaptureInfo.i2FlareGain, a_rCaptureInfo.i2FlareOffset, a_rCaptureInfo.u4RealISO);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain=1024;
    MUINT32 u4PreviewBaseISO=100;

    if(g_p3ANVRAM != NULL) {
        u4PreviewBaseISO=g_p3ANVRAM->rAENVRAM.rDevicesInfo.u4MiniISOGain;
    } else {
        MY_LOG("[updateCaptureParams] NVRAM data is NULL\n");
    }
    mCaptureMode = a_rCaptureInfo;
    u4FinalGain = (mCaptureMode.u4AfeGain*mCaptureMode.u4IspGain)>>10;
    mCaptureMode.u4RealISO = u4PreviewBaseISO*u4FinalGain/u4PreviewBaseGain;
//    g_rAEOutput.rCaptureMode[0] = mCaptureMode;
    MY_LOG("[updateCaptureParams] Exp. mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
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
MRESULT AeMgr::getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size)
{
    if(m_pIAeAlgo != NULL) 
    {
        m_pIAeAlgo->getAEBlockYvalues(pYvalues, size);
    } 
    else 
    {
        MY_LOG("The AE algo class is NULL (25)\n");
    }
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
    strAEInput rAEInput;
    strAEOutput rAEOutput;

    a_strFrameInfo.u4FRameRate_x10 = g_rAEOutput.rPreviewMode.u2FrameRate;
    a_strFrameInfo.i4BrightValue_x10 = m_BVvalue;
    a_strFrameInfo.i4ExposureValue_x10 = m_i4EVvalue;
    a_strFrameInfo.i4LightValue_x10 = (m_BVvalue + 50);
    a_strFrameInfo.u4AEIndex = m_i4AEidxNext;
    a_strFrameInfo.u4ShutterSpeed_us = g_rAEOutput.rPreviewMode.u4Eposuretime;
    a_strFrameInfo.u4SensorGain_x1024 = g_rAEOutput.rPreviewMode.u4AfeGain;
    a_strFrameInfo.u4ISPGain_x1024 = g_rAEOutput.rPreviewMode.u4IspGain;

    if(m_eCamMode == eAppMode_EngMode){
        rAEInput.eAeState = AE_STATE_CAPTURE;
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        } else {
            MY_LOG("The AE algo class is NULL (40)\n");
        }
        copyAEInfo2mgr(&g_rAEOutput.rCaptureMode[0], &rAEOutput);
        a_strFrameInfo.u4CapShutterSpeed_us = g_rAEOutput.rCaptureMode[0].u4Eposuretime;
        a_strFrameInfo.u4CapSensorGain_x1024 = g_rAEOutput.rCaptureMode[0].u4AfeGain;
        a_strFrameInfo.u4CapISPGain_x1024 = g_rAEOutput.rCaptureMode[0].u4IspGain;
    } else {
        a_strFrameInfo.u4CapShutterSpeed_us = g_rAEOutput.rPreviewMode.u4Eposuretime;
        a_strFrameInfo.u4CapSensorGain_x1024 = g_rAEOutput.rPreviewMode.u4AfeGain;
        a_strFrameInfo.u4CapISPGain_x1024 = g_rAEOutput.rPreviewMode.u4IspGain;
    }
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

    static UINT32 u4PrevExposureTime = 0;
    static UINT32 u4PrevSensorGain = 0;
    static UINT32 u4PrevIspGain = 0;
    static LIB3A_AECAM_MODE_T ePrevAECamMode = m_eAECamMode;

    switch(eNewAEState)
    {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
            // sensor initial and send shutter / gain default value
            MY_LOG("[cameraPreviewInit] Sensor Dev:%d Exp Mode: %d Shutter:%d Sensor Gain:%d Isp Gain:%d Flare:%d %d Frame Rate:%d\n",
             m_i4SensorDev, g_rAEOutput.rPreviewMode.u4ExposureMode, g_rAEOutput.rPreviewMode.u4Eposuretime,
             g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u2FrameRate);

            (g_rAEOutput.rPreviewMode.u4ExposureMode == eAE_EXPO_TIME) ? (err = AAASensorMgr::getInstance().setPreviewParams(g_rAEOutput.rPreviewMode.u4Eposuretime, g_rAEOutput.rPreviewMode.u4AfeGain))
                                                           : (err = AAASensorMgr::getInstance().setPreviewLineBaseParams(g_rAEOutput.rPreviewMode.u4Eposuretime, g_rAEOutput.rPreviewMode.u4AfeGain));

            if (FAILED(err)) {
                MY_ERR("AAASensorMgr::getInstance().setPreviewParams fail\n");
                return err;
            }
            u4PrevExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
            u4PrevSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
            u4PrevIspGain = g_rAEOutput.rPreviewMode.u4IspGain;

            // sensor isp gain and flare value
            ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(g_rAEOutput.rPreviewMode.u4IspGain>>1);
            u4PrevIspGain = g_rAEOutput.rPreviewMode.u4IspGain; // v1.2
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
                MY_LOG("The AE algo class is NULL (39)\n");
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
        {
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
            } else {
                MY_LOG("The AE algo class is NULL (29)\n");
            }

            MY_LOG("[%s:s] VDNum %d, Prev %d/%d/%d, Cur %d/%d/%d, Output %d/%d/%d", __FUNCTION__, m_i4WaitVDNum,
                    m_u4PreExposureTime,
                    m_u4PreSensorGain,
                    m_u4PreIspGain,
                    rAEInfo2ISP.u4Eposuretime,
                    rAEInfo2ISP.u4AfeGain,
                    rAEInfo2ISP.u4IspGain,
                    g_rAEOutput.rPreviewMode.u4Eposuretime,
                    g_rAEOutput.rPreviewMode.u4AfeGain,
                    g_rAEOutput.rPreviewMode.u4IspGain);


            if (u4PrevExposureTime == 0 ||
                    u4PrevSensorGain == 0 ||
                    u4PrevIspGain == 0) {
                u4PrevExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
                u4PrevSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
                u4PrevIspGain = g_rAEOutput.rPreviewMode.u4IspGain;
            }


#if 1 // edwin version
            if(m_i4WaitVDNum <= m_i4IspGainDelayFrames) {   // restart
                m_u4PreExposureTime = u4PrevExposureTime;
                m_u4PreSensorGain = u4PrevSensorGain;
                m_u4PreIspGain = u4PrevIspGain;

                if(m_i4WaitVDNum == m_i4ShutterDelayFrames) {
                    AaaTimer localTimer("setSensorExpTime", m_i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
                    AAASensorMgr::getInstance().setSensorExpTime(g_rAEOutput.rPreviewMode.u4Eposuretime);
                    localTimer.printTime();
                    u4PrevExposureTime = g_rAEOutput.rPreviewMode.u4Eposuretime;
                }
                if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                        AaaTimer localTimer("setSensorGain", m_i4SensorDev, (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
                        AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rPreviewMode.u4AfeGain);
                        localTimer.printTime();
                        u4PrevSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
                }

#if 1 //enable smooth
                if(m_i4WaitVDNum < m_i4IspGainDelayFrames && ePrevAECamMode == m_eAECamMode && m_bOtherIPRestoreAE == MFALSE && (m_bEnableAE == MTRUE)) {
                    MUINT32 w1;
                    UINT32 CurTotalGain;

#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#endif

                    w1 = 12;
                    CurTotalGain = (g_rAEOutput.rPreviewMode.u4Eposuretime * g_rAEOutput.rPreviewMode.u4AfeGain) /
                        m_u4PreExposureTime *
                        g_rAEOutput.rPreviewMode.u4IspGain /
                        m_u4PreSensorGain;

                    m_u4SmoothIspGain = ((32-w1)*u4PrevIspGain + w1*CurTotalGain)>>5;
                    m_u4SmoothIspGain = max(min(m_u4SmoothIspGain, 10*1024), 1024);
                if(m_u4SmoothIspGain != g_rAEOutput.rPreviewMode.u4IspGain) {
                    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(m_u4SmoothIspGain>>1);
                    rAEInfo2ISP.u4IspGain = m_u4SmoothIspGain;
                    u4PrevIspGain = m_u4SmoothIspGain;
//                MY_LOG("[%s] total/prev/smooth/output %d/%d/%d/%d", __FUNCTION__,
//                        CurTotalGain,
//                        m_u4PreIspGain,
//                        m_u4SmoothIspGain,
//                        g_rAEOutput.rPreviewMode.u4IspGain);
                    IspTuningMgr::getInstance().validatePerFrame(MTRUE);
                  }
                } else {    // reset condition
                    m_u4PreExposureTime =g_rAEOutput.rPreviewMode.u4Eposuretime;
                    m_u4PreSensorGain = g_rAEOutput.rPreviewMode.u4AfeGain;
                    u4PrevIspGain = m_u4PreIspGain = g_rAEOutput.rPreviewMode.u4IspGain;
 
                    if (m_i4WaitVDNum > m_i4IspGainDelayFrames && m_bAEStable == TRUE) { // a complete cycle
                        ePrevAECamMode = m_eAECamMode;
                        m_bOtherIPRestoreAE = MFALSE;
                    }
                }
#endif
                MY_LOG("[%s:e] VDNum %d, Delay %d/%d/%d, Prev %d/%d/%d, Cur %d/%d/%d, Output %d/%d/%d", __FUNCTION__, m_i4WaitVDNum,
                    m_i4ShutterDelayFrames,
                    m_i4SensorGainDelayFrames,
                    m_i4IspGainDelayFrames,
                    m_u4PreExposureTime,
                    m_u4PreSensorGain,
                    m_u4PreIspGain,
                    rAEInfo2ISP.u4Eposuretime,
                    rAEInfo2ISP.u4AfeGain,
                    rAEInfo2ISP.u4IspGain,
                    g_rAEOutput.rPreviewMode.u4Eposuretime,
                    g_rAEOutput.rPreviewMode.u4AfeGain,
                    g_rAEOutput.rPreviewMode.u4IspGain);

                ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(g_rAEOutput.rPreviewMode.i2FlareGain, (-1*g_rAEOutput.rPreviewMode.i2FlareOffset));

                IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);

                if(m_i4WaitVDNum == m_i4IspGainDelayFrames) {
                    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(g_rAEOutput.rPreviewMode.u4IspGain>>1);
                    m_AEState = eNewAEState;
                    if(m_AEState == AE_MANUAL_FRAMERATE_STATE) {
                        // frame rate control
                        err = AAASensorMgr::getInstance().setSensorFrameRate(g_rAEOutput.rPreviewMode.u2FrameRate);
                        if (FAILED(err)) {
                            MY_ERR("AAASensorMgr::getInstance().setSensorFrameRate fail\n");
                            return err;
                        }
                    }
                    IspTuningMgr::getInstance().validatePerFrame(MTRUE);
                } else {
                    IspTuningMgr::getInstance().validatePerFrame(MFALSE);
                }
                m_i4WaitVDNum++;  
            }else {
                MY_LOG("[doPvAE] m_i4WaitVDNum:%d \n", m_i4WaitVDNum);              
            }
        }
#endif
            break;
        case AE_AF_STATE:
            // if the AF setting is the same with preview, skip the re-setting
            if((g_rAEOutput.rPreviewMode.u4Eposuretime != g_rAEOutput.rAFMode.u4Eposuretime) || (g_rAEOutput.rPreviewMode.u4AfeGain != g_rAEOutput.rAFMode.u4AfeGain) ||
                (g_rAEOutput.rPreviewMode.u4IspGain != g_rAEOutput.rAFMode.u4IspGain)) {
                if(m_i4WaitVDNum == m_i4ShutterDelayFrames) {
                    AAASensorMgr::getInstance().setSensorExpTime(g_rAEOutput.rAFMode.u4Eposuretime);
                }
                if(m_i4WaitVDNum == m_i4SensorGainDelayFrames) {
                    AAASensorMgr::getInstance().setSensorGain(g_rAEOutput.rAFMode.u4AfeGain);
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
                    u4PrevExposureTime = 0;
                    u4PrevSensorGain = 0;
                    u4PrevIspGain = 0;
                    MY_LOG("[doAFAE] ISP Gain:%d\n", g_rAEOutput.rAFMode.u4IspGain);
                    m_bAEStable = MTRUE;
                    prepareCapParams();
                }                    
                m_i4WaitVDNum ++;
                MY_LOG("[doAFAE] Shutter:%d Sensor Gain:%d\n", g_rAEOutput.rAFMode.u4Eposuretime, g_rAEOutput.rAFMode.u4AfeGain);
            }else {
                 m_bAEStable = MTRUE;
                 m_i4WaitVDNum = m_i4IspGainDelayFrames+1;
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
            MY_LOG("[doPreCapAE] State:%d Exp mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                    eNewAEState, g_rAEOutput.rPreviewMode.u4ExposureMode, g_rAEOutput.rPreviewMode.u4Eposuretime,
                    g_rAEOutput.rPreviewMode.u4AfeGain, g_rAEOutput.rPreviewMode.u4IspGain, g_rAEOutput.rPreviewMode.u2FrameRate,
                    g_rAEOutput.rPreviewMode.i2FlareGain, g_rAEOutput.rPreviewMode.i2FlareOffset, g_rAEOutput.rPreviewMode.u4RealISO);
            MY_LOG("[doPreCapAE] AF Exp mode:%d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                 g_rAEOutput.rAFMode.u4ExposureMode, g_rAEOutput.rAFMode.u4Eposuretime,
                 g_rAEOutput.rAFMode.u4AfeGain, g_rAEOutput.rAFMode.u4IspGain, g_rAEOutput.rAFMode.u2FrameRate,
                 g_rAEOutput.rAFMode.i2FlareGain, g_rAEOutput.rAFMode.i2FlareOffset, g_rAEOutput.rAFMode.u4RealISO);
            MY_LOG("[doPreCapAE] Capture Exp mode: %d Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
                 g_rAEOutput.rCaptureMode[0].u4ExposureMode, g_rAEOutput.rCaptureMode[0].u4Eposuretime,
                 g_rAEOutput.rCaptureMode[0].u4AfeGain, g_rAEOutput.rCaptureMode[0].u4IspGain, g_rAEOutput.rCaptureMode[0].u2FrameRate,
                 g_rAEOutput.rCaptureMode[0].i2FlareGain, g_rAEOutput.rCaptureMode[0].i2FlareOffset, g_rAEOutput.rCaptureMode[0].u4RealISO);
            break;
        case AE_CAPTURE_STATE:
            MY_LOG("[doCapAE] Exp. Mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
            mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);

            (mCaptureMode.u4ExposureMode == eAE_EXPO_TIME) ? (AAASensorMgr::getInstance().setCaptureParams(mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain))
                                               : (AAASensorMgr::getInstance().setCaptureLineBaseParams(mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain));

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
            IspTuningMgr::getInstance().validatePerFrame(MTRUE);

            break;
        case AE_POST_CAPTURE_STATE:
            MY_LOG("[PostCapAE] Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n", mCaptureMode.u4Eposuretime,
                mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);

            //AAASensorMgr::getInstance().setCaptureParams(mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain);
            //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspAEGain(mCaptureMode.u4IspGain>>1);
            ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(mCaptureMode.i2FlareGain, (-1*mCaptureMode.i2FlareOffset));

            // Update to isp tuning
            if(m_pIAeAlgo != NULL) {
                m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
            } else {
                MY_LOG("The AE algo class is NULL (40)\n");
            }

            rAEInfo2ISP.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rAEInfo2ISP.i2FlareOffset  =mCaptureMode.i2FlareOffset;
            rAEInfo2ISP.u4AfeGain = mCaptureMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = mCaptureMode.u4IspGain;
            rAEInfo2ISP.u4RealISOValue = mCaptureMode.u4RealISO;
            IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
            // valdate ISP
            IspTuningMgr::getInstance().validatePerFrame(MTRUE);
            break;
    }
    return S_AE_OK;
}

MRESULT AeMgr::UpdateFlare2ISP()
{
    MINT16   i2FlareGain;   // 512 is 1x
    AE_INFO_T rAEInfo2ISP;

    if(m_pIAeAlgo != NULL) {
        m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
    } else {
        MY_LOG("The AE algo class is NULL (49)\n");
    }
    i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - rAEInfo2ISP.i2FlareOffset);
    ISP_MGR_PGN_T::getInstance((ESensorDev_T)m_i4SensorDev).setIspFlare(i2FlareGain, (-1*rAEInfo2ISP.i2FlareOffset));
    IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);
    IspTuningMgr::getInstance().validatePerFrame(MFALSE);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 AeMgr::getAEFaceDiffIndex()
{
 return m_i2AEFaceDiffIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    m_i4ShutterDelayFrames = *i4IspGainDelay - *i4SutterDelay;
    m_i4SensorGainDelayFrames = *i4IspGainDelay - *i4SensorGainDelay;
    if(*i4IspGainDelay > 0) {
        m_i4IspGainDelayFrames = *i4IspGainDelay - 1; // for CQ0 1 delay frame    
    } else {
        m_i4IspGainDelayFrames = 0; // for CQ0 1 delay frame        
    }

    MY_LOG("[updateSensorDelayInfo] m_i4ShutterDelayFrames:%d m_i4SensorGainDelayFrames:%d Isp gain:%d %d %d %d\n",
        m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
MUINT8 uYvalue[5][5];

    if(m_bFrameUpdate == MTRUE) {
        if(m_pIAeAlgo != NULL) {
            m_pIAeAlgo->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO*AE_BLOCK_NO);
            * i4Yvalue = uYvalue[2][2];
        } else {
            MY_LOG("The AE algo class is NULL (40)\n");
        }

        *bFrameUpdate = m_bFrameUpdate;
        m_bFrameUpdate = MFALSE;
    } else {
        * i4Yvalue = 0;
        *bFrameUpdate = MFALSE;
    }

    MY_LOG("[getBrightnessValue] Yvalue:%d FrameUpdate:%d\n",* i4Yvalue, *bFrameUpdate);
    return S_AE_OK;
}

