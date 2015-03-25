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
#define LOG_TAG "awb_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <mtkcam/algorithm/lib3a/awb_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <awb_tuning_custom.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <kd_camera_feature.h>
#include <isp_tuning.h>
#include <ispdrv_mgr.h>
#include <mtkcam/featureio/tdri_mgr.h>
#include <camera_custom_cam_cal.h>
#include <cam_cal_drv.h>
#include <flash_feature.h>
#include "awb_mgr.h"

using namespace NS3A;
using namespace NSIspTuning;

NVRAM_CAMERA_3A_STRUCT* g_pNVRAM_3A;
SENSOR_RESOLUTION_INFO_T g_rSensorResolution[2]; // [0]: for TG1 (main/sub), [1]: for TG2(main_2)
AWB_INIT_INPUT_T g_rAWBInitInput;
AWB_OUTPUT_T g_rAWBOutput;
AWB_STAT_CONFIG_T g_rAWBStatCfg[AWB_STROBE_MODE_NUM][AWB_SENSOR_MODE_NUM][LIB3A_AWB_MODE_NUM];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr&
AwbMgr::
getInstance()
{
    static  AwbMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr::
AwbMgr()
    : m_pIAwbAlgo(IAwbAlgo::createInstance())
    , m_eAWBMode(LIB3A_AWB_MODE_AUTO)
    , m_i4SensorMode(AWB_SENSOR_MODE_PREVIEW)
    , m_i4StrobeMode(AWB_STROBE_MODE_OFF)
    , m_bEnableAWB(isAWBEnabled())
    , m_bAWBLock(MFALSE)
    , m_bAdbAWBLock(MFALSE)
    , m_bOneShotAWB(MFALSE)
    , m_bAWBModeChanged(MFALSE)
    , m_bStrobeModeChanged(MFALSE)
    , m_pIsAWBActive(MNULL)
    , m_i4PvAWBCycleNum(getAWBCycleNum_Preview())
    , m_i4VideoAWBCycleNum(getAWBCycleNum_Video())
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    , m_bInitState(MFALSE)
    , m_i4AFLV(70)
    , m_bSkipOneFrame(MFALSE)
{


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr::
~AwbMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::cameraPreviewInit(MINT32 i4SensorDev, Param_T &rParam)
{
    MRESULT err;

    m_i4SensorDev = i4SensorDev;

    if (rParam.u4CamMode == eAppMode_ZsdMode)
        m_i4SensorMode = AWB_SENSOR_MODE_CAPTURE;
    else
        m_i4SensorMode = AWB_SENSOR_MODE_PREVIEW;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.awb_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // set strobe mode to OFF: temp added
    setStrobeMode(AWB_STROBE_MODE_OFF);

    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

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

    // Init AWB
    err = AWBInit(rParam);
    if (FAILED(err)) {
        MY_ERR("AWBInit() fail\n");
        return err;
    }

    // Init IspDrvMgr 
    err = IspDrvMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("IspDrvMgr::getInstance().init() fail\n");
        return err;
    }
    
    // Init TdriMgr
    err = TdriMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("TdriMgr::getInstance().init() fail\n");
        return err;
    }

    // AWB statistics config
    err = ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AWB_STAT_CONFIG_T::getInstance().config() fail\n");
        return err;
    }

    // update AE RAW pre-gain2
    err = ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AE_RAWPREGAIN2_T::getInstance().setRAWPregain2() fail\n");
        return err;
    }

    // update AWB gain
    err = ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_PGN_T::getInstance().setIspAWBGain() fail\n");
        return err;
    }

    // For debug
    if (m_bDebugEnable) {
        err = IspDebug::getInstance().init();
    if (FAILED(err)) {
            MY_ERR("IspDebug::getInstance().init() fail\n");
        return err;
    }
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::camcorderPreviewInit(MINT32 i4SensorDev, Param_T &rParam)
{
    MRESULT err;

    m_i4SensorDev = i4SensorDev;
    m_i4SensorMode = AWB_SENSOR_MODE_VIDEO;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.awb_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // set strobe mode to OFF: temp added
    MY_LOG("%s(): rParam.u4StrobeMode = %d\n", __FUNCTION__, rParam.u4StrobeMode);

    if (rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_TORCH) // Torch mode
        setStrobeMode(AWB_STROBE_MODE_ON);
    else
        setStrobeMode(AWB_STROBE_MODE_OFF);

    // set strobe mode to OFF: temp added
    //setStrobeMode(AWB_STROBE_MODE_OFF);

    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

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

    // AWB init
    err = AWBInit(rParam);
    if (FAILED(err)) {
        MY_ERR("AWBInit() fail\n");
        return err;
    }

    // Init IspDrvMgr 
    err = IspDrvMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("IspDrvMgr::getInstance().init() fail\n");
        return err;
    }
    
    // Init TdriMgr
    err = TdriMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("TdriMgr::getInstance().init() fail\n");
        return err;
    }

    // AWB statistics config
    err = ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[m_i4StrobeMode][m_i4SensorMode][m_eAWBMode]);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AWB_STAT_CONFIG_T::getInstance().config() fail\n");
        return err;
    }

    // update AE RAW pre-gain2
    err = ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AE_RAWPREGAIN2_T::getInstance().setRAWPregain2() fail\n");
        return err;
    }

    // update AWB gain
    err = ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_PGN_T::getInstance().setIspAWBGain() fail\n");
        return err;
    }

    // For debug
    if (m_bDebugEnable) {
        err = IspDebug::getInstance().init();
        if (FAILED(err)) {
            MY_ERR("IspDebug::getInstance().init() fail\n");
            return err;
        }
    }
    

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::cameraCaptureInit()
{
    MRESULT err;
    AWB_SENSOR_MODE_T eAWBSensorMode = AWB_SENSOR_MODE_CAPTURE;

    // Get operation mode and sensor mode for CCT and EM
    MINT32 i4OperMode = IspTuningMgr::getInstance().getOperMode();
    MINT32 i4SensorMode = IspTuningMgr::getInstance().getSensorMode();

    if ((i4OperMode == EOperMode_Meta) || (i4OperMode == EOperMode_EM)) {
        switch (i4SensorMode) {
        case ESensorMode_Preview:
            eAWBSensorMode = AWB_SENSOR_MODE_PREVIEW;
            break;
        case ESensorMode_Video:
            eAWBSensorMode = AWB_SENSOR_MODE_VIDEO;
            break;
        case ESensorMode_Capture:
        default:    
            eAWBSensorMode = AWB_SENSOR_MODE_CAPTURE;
            break;
        }
    }

    MY_LOG("(i4OperMode, i4SensorMode, eAWBSensorMode) = (%d, %d, %d)", i4OperMode, i4SensorMode, eAWBSensorMode);

    if (m_bEnableAWB)
    {
        if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_ON][eAWBSensorMode][m_eAWBMode]);

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_ON][AWB_SENSOR_MODE_CAPTURE][m_eAWBMode]);
        }
        else {
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][eAWBSensorMode][m_eAWBMode]);

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][AWB_SENSOR_MODE_CAPTURE][m_eAWBMode]);
        }
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::cameraPreviewReinit(Param_T &rParam)
{
    MRESULT err;

    if (rParam.u4CamMode == eAppMode_ZsdMode)
        m_i4SensorMode = AWB_SENSOR_MODE_CAPTURE;
    else
        m_i4SensorMode = AWB_SENSOR_MODE_PREVIEW;


    // set strobe mode to OFF: temp added
    setStrobeMode(AWB_STROBE_MODE_OFF);

    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

    // AWB statistics config
    err = ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AWB_STAT_CONFIG_T::getInstance().config() fail\n");
        return err;
    }

    // update AE RAW pre-gain2
    err = ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_AE_RAWPREGAIN2_T::getInstance().setRAWPregain2() fail\n");
        return err;
    }

    // update AWB gain
    err = ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);
    if (FAILED(err)) {
        MY_ERR("ISP_MGR_PGN_T::getInstance().setIspAWBGain() fail\n");
        return err;
    }

    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::uninit()
{
    MRESULT err;
    
    // uninit IspDrvMgr 
    err = IspDrvMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("IspDrvMgr::getInstance().uninit() fail\n");
        return err;
    }
    
    // uninit TdriMgr
    err = TdriMgr::getInstance().uninit();
    if (FAILED(err)) {
        MY_ERR("TdriMgr::getInstance().uninit() fail\n");
        return err;
    }
    
    // For debug
    if (m_bDebugEnable) {
        IspDebug::getInstance().uninit();
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setAWBMode(MINT32 i4NewAWBMode)
{
    LIB3A_AWB_MODE_T eNewAWBMode;

    MY_LOG("i4NewAWBMode: %d\n", i4NewAWBMode);

    switch (i4NewAWBMode) {
        case AWB_MODE_AUTO: // Auto
            eNewAWBMode = LIB3A_AWB_MODE_AUTO;
            break;
        case AWB_MODE_DAYLIGHT: // Daylight
            eNewAWBMode = LIB3A_AWB_MODE_DAYLIGHT;
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT: // Cloudy daylight
            eNewAWBMode = LIB3A_AWB_MODE_CLOUDY_DAYLIGHT;
            break;
        case AWB_MODE_SHADE: // Shade
            eNewAWBMode = LIB3A_AWB_MODE_SHADE;
            break;
        case AWB_MODE_TWILIGHT: // Twilight
            eNewAWBMode = LIB3A_AWB_MODE_TWILIGHT;
            break;
        case AWB_MODE_FLUORESCENT: // Fluorescent
            eNewAWBMode = LIB3A_AWB_MODE_FLUORESCENT;
            break;
        case AWB_MODE_WARM_FLUORESCENT: // Warm fluorescent
            eNewAWBMode = LIB3A_AWB_MODE_WARM_FLUORESCENT;
            break;
        case AWB_MODE_INCANDESCENT: // Incandescent
            eNewAWBMode = LIB3A_AWB_MODE_INCANDESCENT;
            break;
        case AWB_MODE_GRAYWORLD: // Grayword
            eNewAWBMode = LIB3A_AWB_MODE_GRAYWORLD;  
            break;
        default:
            MY_ERR("E_AWB_UNSUPPORT_MODE: %d\n", i4NewAWBMode);
            return (E_AWB_UNSUPPORT_MODE);
    }

    if (m_eAWBMode != eNewAWBMode)
    {
        m_eAWBMode = eNewAWBMode;
        m_bAWBModeChanged = MTRUE;
        MY_LOG("m_eAWBMode: %d\n", m_eAWBMode);
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getAWBMode() const
{
    return static_cast<MINT32>(m_eAWBMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setStrobeMode(MINT32 i4NewStrobeMode)
{
    MY_LOG("setStrobeMode: i4NewStrobeMode=%d\n", i4NewStrobeMode);
    if ((i4NewStrobeMode < AWB_STROBE_MODE_ON) || (i4NewStrobeMode > AWB_STROBE_MODE_OFF))
    {
        MY_ERR("Unsupport strobe mode: %d\n", i4NewStrobeMode);
        return E_AWB_UNSUPPORT_MODE;
    }

    if (m_i4StrobeMode != i4NewStrobeMode)
    {
        m_i4StrobeMode = i4NewStrobeMode;
        m_bStrobeModeChanged = MTRUE;
        MY_LOG("m_i4StrobeMode: %d\n", m_i4StrobeMode);
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getStrobeMode() const
{
    return m_i4StrobeMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setAWBLock(MBOOL bAWBLock)
{
    MY_LOG("[AwbMgr::setAWBLock] bAWBLock: %d\n", bAWBLock);
    
    if (m_bAWBLock != bAWBLock)
    {
        if (bAWBLock) { // AWB lock
            m_bAWBLock = MTRUE;
            m_bOneShotAWB = MTRUE;
        }
        else { // AWB unlock
            m_bAWBLock = MFALSE;
        }

        MY_LOG("[AwbMgr::setAWBLock] m_bAWBLock: %d\n", m_bAWBLock);
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::enableAWB()
{
    m_bEnableAWB = MTRUE;

    MY_LOG("enableAWB()\n");

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::disableAWB()
{
    m_bEnableAWB = MFALSE;

    MY_LOG("disableAWB()\n");

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doPvAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf)
{
    AWB_INPUT_T rAWBInput;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.awb_mgr.lock", value, "0");
    m_bAdbAWBLock = atoi(value);

    m_pIsAWBActive = getAWBActiveCycle_Preview(i4SceneLV);

    if ((m_pIsAWBActive[i4FrameCount % m_i4PvAWBCycleNum]) && m_bEnableAWB)
    {
        if ((m_bAWBModeChanged) && (!m_bInitState))
        {
            m_pIAwbAlgo->setAWBMode(m_eAWBMode);
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
            m_bAWBModeChanged = MFALSE;
            m_bOneShotAWB = MTRUE;
            m_bSkipOneFrame = MTRUE;

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
        }
        else if ((m_bStrobeModeChanged) && (!m_bInitState))
        {
             MY_LOG("m_bStrobeModeChanged = %d, m_i4StrobeMode = %d\n", m_bStrobeModeChanged, m_i4StrobeMode);
             
             if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                 m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
                 // update AWB statistics config
                 ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
             }
             else {
                 m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
                 // update AWB statistics config
                 ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
             }

             m_bStrobeModeChanged = MFALSE;
             m_bOneShotAWB = MTRUE;
             m_bSkipOneFrame = MTRUE;
        }
        else if (m_bSkipOneFrame) { // skip one frame for AWB statistics ready
             m_bSkipOneFrame = MFALSE;
        }
        else if (((!m_bAWBLock) && (!m_bAdbAWBLock)) || // m_bAWBLock = FALSE
                 ((m_bAWBLock) && (m_bOneShotAWB) && (!m_bAdbAWBLock))) // m_bAWBLock = TRUE and m_bOneShotAWB = TRUE
        {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4SensorMode = m_i4SensorMode;
            rAWBInput.i4AWBState = AWB_STATE_PREVIEW;
            rAWBInput.pAWBStatBuf = pAWBStatBuf;
            if (m_bOneShotAWB)
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

                if (!m_bInitState) {
                    m_bOneShotAWB = MFALSE;
                }
            }
            else
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_SMOOTH_TRANSITION;
            }

            m_pIAwbAlgo->handleAWB(rAWBInput, g_rAWBOutput);

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewStrobeRAWPreGain2);

                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewStrobeAWBGain);
            }
            else {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);

                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);                 
            }
            
            // set AWB info
            IspTuningMgr::getInstance().setAWBInfo(g_rAWBOutput.rAWBInfo);

            if (m_bInitState && bAEStable) {
                m_bInitState = MFALSE; 
            }
        }
    }
   
    //ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).dumpRegs();
    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doVideoAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf)
{
    AWB_INPUT_T rAWBInput;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.awb_mgr.lock", value, "0");
    m_bAdbAWBLock = atoi(value);

    m_pIsAWBActive = getAWBActiveCycle_Video(i4SceneLV);

    if ((m_pIsAWBActive[i4FrameCount % m_i4VideoAWBCycleNum]) && m_bEnableAWB)
    {
        if ((m_bAWBModeChanged) && (!m_bInitState))
        {
            m_pIAwbAlgo->setAWBMode(m_eAWBMode);
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
            m_bAWBModeChanged = MFALSE;
            m_bOneShotAWB = MTRUE;
            m_bSkipOneFrame = MTRUE;

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
        }
        else if ((m_bStrobeModeChanged) && (!m_bInitState))
        {
             MY_LOG("m_bStrobeModeChanged = %d, m_i4StrobeMode = %d\n", m_bStrobeModeChanged, m_i4StrobeMode);
             
             if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                 m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
                 // update AWB statistics config
                 ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
             }
             else {
                 m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
                 // update AWB statistics config
                 ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_i4SensorMode][m_eAWBMode]);
             }

             m_bStrobeModeChanged = MFALSE;
             m_bOneShotAWB = MTRUE;
             m_bSkipOneFrame = MTRUE;
        }
        else if (m_bSkipOneFrame) { // skip one frame for AWB statistics ready
             m_bSkipOneFrame = MFALSE;
        }
        else if (((!m_bAWBLock) && (!m_bAdbAWBLock)) || // m_bAWBLock = FALSE
                 ((m_bAWBLock) && (m_bOneShotAWB) && (!m_bAdbAWBLock))) // m_bAWBLock = TRUE and m_bOneShotAWB = TRUE
        {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4SensorMode = m_i4SensorMode;
            rAWBInput.i4AWBState = AWB_STATE_PREVIEW;
            rAWBInput.pAWBStatBuf = pAWBStatBuf;
            if (m_bOneShotAWB)
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

                if (!m_bInitState) {
                    m_bOneShotAWB = MFALSE;
                }
            }
            else
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_SMOOTH_TRANSITION;
            }

            m_pIAwbAlgo->handleAWB(rAWBInput, g_rAWBOutput);

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewStrobeRAWPreGain2);

                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewStrobeAWBGain);
            }
            else {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);

                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);                 
            }
            
            // set AWB info
            IspTuningMgr::getInstance().setAWBInfo(g_rAWBOutput.rAWBInfo);

            if (m_bInitState && bAEStable) {
                m_bInitState = MFALSE; 
            }
        }
    }
   
    //ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).dumpRegs();
    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doAFAWB(MVOID *pAWBStatBuf)
{
    AWB_INPUT_T rAWBInput;

    if (m_bEnableAWB)
    {
        if ((m_i4StrobeMode == AWB_STROBE_MODE_ON) && (m_bStrobeModeChanged))
        {
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
            m_bStrobeModeChanged = FALSE;

            // update AWB statistics config    
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);

            MY_LOG("update AWB statistics config: m_bStrobeModeChanged = %d", m_bStrobeModeChanged);
            return S_AWB_OK;
        }

        if ((!m_bAWBLock) && (!m_bAdbAWBLock)) {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = getAFLV();
            rAWBInput.i4SensorMode = m_i4SensorMode;
            rAWBInput.i4AWBState = AWB_STATE_AF;
            rAWBInput.pAWBStatBuf = pAWBStatBuf;
            rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

            m_pIAwbAlgo->handleAWB(rAWBInput, g_rAWBOutput);

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewStrobeRAWPreGain2);
                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewStrobeAWBGain);
            }
            else {          
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);
                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);
            }
        
            // set AWB info
            IspTuningMgr::getInstance().setAWBInfo(g_rAWBOutput.rAWBInfo);
        }
    }

    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doPreCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf)
{
    AWB_INPUT_T rAWBInput;

    if (m_bEnableAWB)
    {
        if ((m_i4StrobeMode == AWB_STROBE_MODE_ON) && (m_bStrobeModeChanged))
        {
            m_pIAwbAlgo->setAWBStatConfig(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);
            m_bStrobeModeChanged = FALSE;

            // update AWB statistics config    
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).config(g_rAWBStatCfg[AWB_STROBE_MODE_ON][m_i4SensorMode][m_eAWBMode]);

            MY_LOG("update AWB statistics config: m_bStrobeModeChanged = %d", m_bStrobeModeChanged);
            return S_AWB_OK;
        }

        if ((!m_bAWBLock) && (!m_bAdbAWBLock)) {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4SensorMode = m_i4SensorMode;
            rAWBInput.i4AWBState = AWB_STATE_PRECAPTURE;
            rAWBInput.pAWBStatBuf = pAWBStatBuf;
            rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

            m_pIAwbAlgo->handleAWB(rAWBInput, g_rAWBOutput);

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewStrobeRAWPreGain2);
                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewStrobeAWBGain);
            }
            else {          
                // update AE RAW pre-gain2
                ISP_MGR_AE_RAWPREGAIN2_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setRAWPregain2(g_rAWBOutput.rPreviewRAWPreGain2);
                // update AWB gain
                ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rPreviewAWBGain);
            }
        
            // set AWB info
            IspTuningMgr::getInstance().setAWBInfo(g_rAWBOutput.rAWBInfo);
        }
    }

    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf)
{
    AWB_INPUT_T rAWBInput;

    if ((m_bEnableAWB) && (!m_bAWBLock) && (!m_bAdbAWBLock))
    {
        rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
        rAWBInput.i4SceneLV = i4SceneLV;
        rAWBInput.i4SensorMode = AWB_SENSOR_MODE_CAPTURE;
        rAWBInput.i4AWBState = AWB_STATE_CAPTURE;
        rAWBInput.pAWBStatBuf = pAWBStatBuf;
        rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

        m_pIAwbAlgo->handleAWB(rAWBInput, g_rAWBOutput);

        // Debug
        //g_rAWBOutput.rCaptureAWBGain.i4R = 512;
        //g_rAWBOutput.rCaptureAWBGain.i4G = 1;
        //g_rAWBOutput.rCaptureAWBGain.i4B = 1;

        // update AWB gain            
        ISP_MGR_PGN_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).setIspAWBGain (g_rAWBOutput.rCaptureAWBGain);
        // set AWB info
        IspTuningMgr::getInstance().setAWBInfo(g_rAWBOutput.rAWBInfo);
        // valdate ISP
        IspTuningMgr::getInstance().validatePerFrame(MFALSE);
    }

    if (m_bDebugEnable) {
        MY_LOG("%s()\n", __FUNCTION__);
        IspDebug::getInstance().dumpIspDebugMessage();
    }

    return S_AWB_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getDebugInfo(AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData)
{
    m_pIAwbAlgo->getDebugInfo(rAWBDebugInfo, rAWBDebugData);

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getAWBCCT()
{
    return g_rAWBOutput.rAWBInfo.i4CCT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getASDInfo(AWB_ASD_INFO_T &a_rAWBASDInfo)
{
    m_pIAwbAlgo->getASDInfo(a_rAWBASDInfo);

    return S_AWB_OK;     
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getAWBOutput(AWB_OUTPUT_T &a_rAWBOutput)
{
    a_rAWBOutput = g_rAWBOutput;

    return S_AWB_OK;     
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getSensorResolution()
{
    MRESULT err = S_AWB_OK;

    SensorHal*const pSensorHal = SensorHal::createInstance();
    if  (!pSensorHal)
    {
        MY_ERR("Sensor error");
        err = E_AWB_SENSOR_ERROR;
        goto lbExit;
    }

    if (m_i4SensorDev == ESensorDev_Main)  // MAIN
    {
        pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorPreviewWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorPreviewHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorVideoWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorVideoHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorFullWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorFullHeight),
                                0);
    }
    else if (m_i4SensorDev == ESensorDev_Sub) // SUB
    {
        pSensorHal->sendCommand(SENSOR_DEV_SUB,
                                SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorPreviewWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorPreviewHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_SUB,
                                SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorVideoWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorVideoHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_SUB,
                                SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorFullWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[0].u2SensorFullHeight),
                                0);
    }
    else if (m_i4SensorDev == ESensorDev_MainSecond) // MAIN2
    {
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2,
                                SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorPreviewWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorPreviewHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2,
                                SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorVideoWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorVideoHeight),
                                0);
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2,
                                SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorFullWidth),
                                reinterpret_cast<MINT32>(&g_rSensorResolution[1].u2SensorFullHeight),
                                0);
    }
    else
    {
         MY_ERR("Unsupport sensor device: %d\n", m_i4SensorDev);
         err = E_AWB_PARAMETER_ERROR;
    }

lbExit:
    if (pSensorHal)
    {
        pSensorHal->destroyInstance();
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getNvramData()
{
    if (FAILED(NvramDrvMgr::getInstance().init(m_i4SensorDev))) {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AWB_SENSOR_ERROR;
    }

    NvramDrvMgr::getInstance().getRefBuf(g_pNVRAM_3A);

    NvramDrvMgr::getInstance().uninit();

#if 0
    MY_LOG("sizeof(g_pNVRAM_3A->u4Version) = %d\n", sizeof(g_pNVRAM_3A->u4Version));
    MY_LOG("sizeof(g_pNVRAM_3A->SensorId) = %d\n", sizeof(g_pNVRAM_3A->SensorId));
    MY_LOG("sizeof(g_pNVRAM_3A->rAENVRAM) = %d\n", sizeof(g_pNVRAM_3A->rAENVRAM));
    MY_LOG("sizeof(g_pNVRAM_3A->rAWBNVRAM) = %d\n", sizeof(g_pNVRAM_3A->rAWBNVRAM));
#endif

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::getEEPROMData()
{
    MUINT32 result=0; 
    CAM_CAL_DATA_STRUCT GetCamCalData; 
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance(); 
    MINT32 i4SensorDevID;

    switch (m_i4SensorDev) 
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;        
    case ESensorDev_MainSecond:  
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        return S_AWB_OK;
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }   

    #if 0
    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum=CAMERA_CAM_CAL_DATA_MODULE_VERSION;
    result= m_pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);  
    MY_LOG("(0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", result);
    if(result&CamCalReturnErr[enCamCalEnum])
    {
        MY_LOG("err (%s)", CamCalErrString[enCamCalEnum]);    
    }
    else
    {
        MY_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);           
    }
    enCamCalEnum = CAMERA_CAM_CAL_DATA_PART_NUMBER;
    result= m_pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);       
    MY_LOG("(0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", result);
    if(result&CamCalReturnErr[enCamCalEnum])
    {
        MY_LOG("err (%s)", CamCalErrString[enCamCalEnum]);    
    }
    else
    {
        MY_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);           
    }
    #endif

    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum = CAMERA_CAM_CAL_DATA_3A_GAIN;
    result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);      
    MY_LOG("(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);
    
    if (result&CamCalReturnErr[enCamCalEnum])
    {
        MY_LOG("err (%s)", CamCalErrString[enCamCalEnum]);    
    }
    else
    {
        MY_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);           
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
    } 

    MY_LOG("%s()\n", __FUNCTION__);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4R = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4R);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4G = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4G);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4B = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4B);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4R = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4R);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4G = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4G);
    MY_LOG("g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4B = %d\n", g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4B);

    return S_AWB_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::AWBInit(Param_T &rParam)
{
    g_rAWBInitInput.eAWBMode = m_eAWBMode;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

    // EEPROM
    if ((isAWBCalibrationBypassed() == MFALSE) && 
        ((g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4R == 0) ||
         (g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4G == 0) ||
         (g_pNVRAM_3A->rAWBNVRAM.rCalData.rGoldenGain.i4B == 0) ||
         (g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4R == 0)   ||
         (g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4G == 0)   ||
         (g_pNVRAM_3A->rAWBNVRAM.rCalData.rUnitGain.i4B == 0)))
    {
         getEEPROMData(); // TBC
    }


    g_rAWBInitInput.rAWBNVRAM = g_pNVRAM_3A->rAWBNVRAM;
    g_rAWBInitInput.rAWBParam = getAWBParam();
    g_rAWBInitInput.rAWBStatParam = getAWBStatParam();

    if ((m_i4SensorDev == ESensorDev_Main) ||
        (m_i4SensorDev == ESensorDev_Sub)) { // TG1
        g_rAWBInitInput.i4PvSensorWidth = g_rSensorResolution[0].u2SensorPreviewWidth;
        g_rAWBInitInput.i4PvSensorHeight = g_rSensorResolution[0].u2SensorPreviewHeight;
        g_rAWBInitInput.i4VideoSensorWidth = g_rSensorResolution[0].u2SensorVideoWidth;
        g_rAWBInitInput.i4VideoSensorHeight = g_rSensorResolution[0].u2SensorVideoHeight;
        g_rAWBInitInput.i4CapSensorWidth = g_rSensorResolution[0].u2SensorFullWidth;
        g_rAWBInitInput.i4CapSensorHeight = g_rSensorResolution[0].u2SensorFullHeight;
    }
    else { // TG2 (main2)
        g_rAWBInitInput.i4PvSensorWidth = g_rSensorResolution[1].u2SensorPreviewWidth;
        g_rAWBInitInput.i4PvSensorHeight = g_rSensorResolution[1].u2SensorPreviewHeight;
        g_rAWBInitInput.i4VideoSensorWidth = g_rSensorResolution[1].u2SensorVideoWidth;
        g_rAWBInitInput.i4VideoSensorHeight = g_rSensorResolution[1].u2SensorVideoHeight;
        g_rAWBInitInput.i4CapSensorWidth = g_rSensorResolution[1].u2SensorFullWidth;
        g_rAWBInitInput.i4CapSensorHeight = g_rSensorResolution[1].u2SensorFullHeight;
    }

    g_rAWBInitInput.i4SensorMode = m_i4SensorMode;

    //MY_LOG("rParam.u4CamMode = %d\n", rParam.u4CamMode);
    MY_LOG("g_rAWBInitInput.eAWBMode = %d\n", g_rAWBInitInput.eAWBMode);
    MY_LOG("g_rAWBInitInput.i4SensorMode = %d\n", g_rAWBInitInput.i4SensorMode);
    MY_LOG("m_i4SensorDev = %d\n", m_i4SensorDev);
    MY_LOG("g_rAWBInitInput.i4PvSensorWidth = %d", g_rAWBInitInput.i4PvSensorWidth);
    MY_LOG("g_rAWBInitInput.i4PvSensorHeight = %d", g_rAWBInitInput.i4PvSensorHeight);
    MY_LOG("g_rAWBInitInput.i4CapSensorWidth = %d", g_rAWBInitInput.i4CapSensorWidth);
    MY_LOG("g_rAWBInitInput.i4CapSensorHeight = %d", g_rAWBInitInput.i4CapSensorHeight);

    return m_pIAwbAlgo->initAWB(g_rAWBInitInput, g_rAWBOutput, g_rAWBStatCfg);

}

