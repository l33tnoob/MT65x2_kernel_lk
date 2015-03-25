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
#define LOG_TAG "aaa_sensor_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/hal/sensor_hal.h> 
#include <cutils/properties.h>
#include <string.h>
//#include <cutils/pmem.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <isp_drv.h>
#include <isp_tuning.h>
#include "buf_mgr.h"
#include <linux/cache.h>

#include <aaa_sensor_mgr.h>

using namespace NS3A;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr&
AAASensorMgr::
getInstance()
{
    static  AAASensorMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
AAASensorMgr()
    : m_pSensorHalObj(MNULL)
    , m_eSensorDevId(SENSOR_DEV_MAIN)
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_bFlickerState(MFALSE)
    , m_u4ExpTime(0)
    , m_u4SensorGain(0)
    , m_u4SensorFrameRate(0)
    , m_i4ShutterDelayFrame(0)
    , m_i4SensorGainDelayFrame(0)
    , m_i4IspGainDelayFrame(2)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAASensorMgr::
~AAASensorMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
init()
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0) {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return ret;
    }

    // Sensor hal init
    m_pSensorHalObj = SensorHal::createInstance();    // create sensor hal object
    if(m_pSensorHalObj == NULL) {
        MY_ERR("[AAA Sensor Mgr] Can not create SensorHal obj\n");       
    }    
    
    android_atomic_inc(&m_Users);

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("debug.aaa_sensor_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
uninit()
{
    MRESULT ret = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0) {
        return ret;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) { // There is no more User after decrease one User
        if(m_pSensorHalObj) {
            m_pSensorHalObj->destroyInstance();
            m_pSensorHalObj = NULL;
        }
    }	else {	// There are still some users.
        MY_LOG("Still %d users \n", m_Users);
    }

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorDev(MINT32 i4SensorDev)
{
    switch(i4SensorDev)
    {
        case ESensorDev_Main:
            m_eSensorDevId = SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub:
            m_eSensorDevId = SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            m_eSensorDevId = SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D:
            m_eSensorDevId = SENSOR_DEV_MAIN_3D;
            break;            
        default:
            MY_LOG("[setSensorDev] Wrong sensor type:%d \n", i4SensorDev);
            break;
    }
    MY_LOG("[setSensorDev] Sensor Dev type:%d \n", i4SensorDev);

    // Get sensor delay frame for sync
    m_i4ShutterDelayFrame = getSensorDelayFrame(m_eSensorDevId, SENSOR_AE_SHUTTER_DELAY);
    m_i4SensorGainDelayFrame = getSensorDelayFrame(m_eSensorDevId, SENSOR_AE_GAIN_DELAY);
    m_i4IspGainDelayFrame = getSensorDelayFrame(m_eSensorDevId, SENSOR_AE_ISP_DELAY);
    MY_LOG("[setSensorDev] Sensor delay frame Shutter:%d Gain:%d Isp:%d\n", m_i4ShutterDelayFrame, m_i4SensorGainDelayFrame, m_i4IspGainDelayFrame);

    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT 
AAASensorMgr::
getSensorSyncinfo(MINT32 *i4SutterDelay, MINT32 *i4SensorGainDelay, MINT32 *i4IspGainDelay)
{
    *i4SutterDelay = m_i4ShutterDelayFrame;
    *i4SensorGainDelay = m_i4SensorGainDelayFrame;
    *i4IspGainDelay = m_i4IspGainDelayFrame;
    
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT 
AAASensorMgr::
getSensorWidthHeight(MINT32 i4SensorDev, SENSOR_RESOLUTION_INFO_T* a_rSensorResolution)
{
    MRESULT err = S_AAA_SENSOR_MGR_OK;
    halSensorDev_e eSensorDevId;

    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    if(i4SensorDev == ESensorDev_Main) {
       eSensorDevId = SENSOR_DEV_MAIN;
    } else if(i4SensorDev == ESensorDev_Sub) {
       eSensorDevId = SENSOR_DEV_SUB;
    } else if(i4SensorDev == ESensorDev_MainSecond) {
       eSensorDevId = SENSOR_DEV_MAIN_2;
    } else {
        MY_ERR("Sensor type error");
        return E_AAA_SENSOR_NULL;    
    }
    
    m_pSensorHalObj->sendCommand(eSensorDevId, SENSOR_CMD_GET_SENSOR_PRV_RANGE, (MINT32)&(a_rSensorResolution->u2SensorPreviewWidth), 
                                                        (MINT32)&(a_rSensorResolution->u2SensorPreviewHeight), 0);   
    m_pSensorHalObj->sendCommand(eSensorDevId, SENSOR_CMD_GET_SENSOR_VIDEO_RANGE, (MINT32)&(a_rSensorResolution->u2SensorVideoWidth), 
                                                        (MINT32)&(a_rSensorResolution->u2SensorVideoHeight), 0);   
    m_pSensorHalObj->sendCommand(eSensorDevId, SENSOR_CMD_GET_SENSOR_FULL_RANGE, (MINT32)&(a_rSensorResolution->u2SensorFullWidth), 
                                                        (MINT32)&(a_rSensorResolution->u2SensorFullHeight), 0);   
    MY_LOG("[getSensorWidthHeight] Sensor id:%d Prv:%d %d Video:%d %d Cap:%d %d\n", eSensorDevId, a_rSensorResolution->u2SensorPreviewWidth, 
                  a_rSensorResolution->u2SensorPreviewHeight, a_rSensorResolution->u2SensorVideoWidth, a_rSensorResolution->u2SensorVideoHeight, 
                  a_rSensorResolution->u2SensorFullWidth, a_rSensorResolution->u2SensorFullHeight);
    return S_AAA_SENSOR_MGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpTime(MUINT32 a_u4ExpTime)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    
    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorExpTime] a_u4ExpTime = %d \n", a_u4ExpTime);
    
    if (a_u4ExpTime == 0) {
        MY_ERR("setSensorExpTime() error: exposure time = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_SENSOR_EXP_TIME, (MINT32)&a_u4ExpTime, 0, 0);
    if(err) {
        MY_ERR("Err CMD_SENSOR_SET_EXP_TIME, Sensor dev:%d\n", m_eSensorDevId);
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAASensorMgr::
getSensorExpTime()
{
    return m_u4ExpTime;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorGain(MUINT32 a_u4SensorGain) 
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;

    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorGain] a_u4SensorGain = %d \n", a_u4SensorGain);

    if (a_u4SensorGain < 1024) {
        MY_ERR("setSensorGain() error: sensor gain:%d \n", a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_SENSOR_GAIN, (MINT32)&a_u4SensorGain, 0, 0);

    if(err) {
        MY_ERR("Err CMD_SENSOR_SET_GAIN, Sensor dev:%d\n", m_eSensorDevId);
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAASensorMgr::
getSensorGain()
{
    return m_u4SensorGain;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorFrameRate(MUINT32 a_u4SensorFrameRate)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4SensorFrameRate;

    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    if(a_u4SensorFrameRate == m_u4SensorFrameRate){
        MY_LOG_IF(m_bDebugEnable,"[setSensorFrameRate] The same frame rate m_u4SensorFrameRate = %d \n", m_u4SensorFrameRate);
        return err;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorFrameRate] a_u4SensorFrameRate = %d \n", a_u4SensorFrameRate);

    // Set sensor gain
    u4SensorFrameRate = a_u4SensorFrameRate / 10;    // 10 base frame rate from AE
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MINT32)&u4SensorFrameRate, 0, 0);   
    if(err) {
        MY_ERR("Err CMD_SENSOR_SET_FRAME_RATE, Sensor dev:%d Frame rate:%d\n", m_eSensorDevId, u4SensorFrameRate);
    }
    
    m_u4SensorFrameRate = a_u4SensorFrameRate;
    return err;    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAASensorMgr::
getSensorFrameRate()
{
    return m_u4SensorFrameRate;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorExpLine(MUINT32 a_u4ExpLine)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    
    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorExpTime] a_u4ExpLine = %d \n", a_u4ExpLine);
    
    if (a_u4ExpLine == 0) {
        MY_ERR("setSensorExpTime() error: exposure line = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_SENSOR_EXP_LINE, (MINT32)&a_u4ExpLine, 0, 0);
    if(err) {
        MY_ERR("Err SENSOR_CMD_SET_SENSOR_EXP_LINE, Sensor dev:%d\n", m_eSensorDevId);
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setFlickerFrameRateActive(MBOOL a_bFlickerFPSAvtive)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4FlickerInfo;

    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setFlickerFrameRateActive] a_bFlickerFPSAvtive = %d \n", a_bFlickerFPSAvtive);

    if ((MINT32) a_bFlickerFPSAvtive == m_bFlickerState) {
        MY_LOG_IF(m_bDebugEnable,"[setFlickerFrameRateActive] The same flicker status = %d \n", a_bFlickerFPSAvtive);
        return err;
    }

    // Set sensor gain
    u4FlickerInfo = (MUINT32)a_bFlickerFPSAvtive;
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_FLICKER_FRAME_RATE, (MINT32)&u4FlickerInfo, 0, 0);
    if(err) {
        MY_ERR("Err CMD_SENSOR_SET_FRAME_RATE, Sensor dev:%d Flicker status:%d\n", m_eSensorDevId, a_bFlickerFPSAvtive);
    }
    
    m_bFlickerState = a_bFlickerFPSAvtive;
  
    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAASensorMgr::
getFlickerFrameRateActive()
{
    return m_bFlickerState;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setSensorParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain, MUINT32 a_u4RawGain)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;
    MUINT32 u4ExpData[5];

    if  (!m_pSensorHalObj) {
        MY_ERR("No Sensor object error");
        return E_AAA_SENSOR_NULL;
    }

    MY_LOG_IF(m_bDebugEnable,"[setSensorParams] Sensor Shutter:%d gain:%d Isp gain:%d delay frame:%d %d %d\n", a_u4ExpTime, a_u4SensorGain, a_u4RawGain, 0, 0, 2);

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0) || (a_u4RawGain == 0)) {
        MY_ERR("[setSensorParams] error: a_u4ExpTime = %d; a_u4SensorGain = %d; a_u4RawGain = %d\n", a_u4ExpTime, a_u4SensorGain, a_u4RawGain);
        return MHAL_INVALID_PARA;
    }
    
#if 1    
    u4ExpData[0] = (a_u4RawGain) | (a_u4RawGain << 16);
    u4ExpData[1] = (a_u4RawGain) | (a_u4RawGain << 16);
    u4ExpData[2] = a_u4ExpTime;
    u4ExpData[3] = a_u4SensorGain;

    u4ExpData[4] = m_i4ShutterDelayFrame |(m_i4SensorGainDelayFrame << 8) | ((m_i4IspGainDelayFrame) << 16);
    err = m_pSensorHalObj->sendCommand(m_eSensorDevId, SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC, (MINT32) &u4ExpData[0], 0, 0);
    
    if(err) {
        MY_ERR("Err SENSOR_FEATURE_SET_SENSOR_SYNC\n");
    }
#else  // send to sensor without sync
    setSensorExpTime(a_u4ExpTime);
    setSensorGain(a_u4SensorGain);
#endif
    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setPreviewParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setPreviewParams] Id:%d Shutter:%d Sensor Gain:%d \n", m_eSensorDevId, a_u4ExpTime, a_u4SensorGain);

    m_u4SensorFrameRate = 0;

    setSensorExpTime(a_u4ExpTime);
    setSensorGain(a_u4SensorGain);

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
AAASensorMgr::
setCaptureParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 err = S_AAA_SENSOR_MGR_OK;

    MY_LOG("[setCaptureParams] Id:%d Shutter:%d Sensor Gain:%d \n", m_eSensorDevId, a_u4ExpTime, a_u4SensorGain);
    
    setSensorExpTime(a_u4ExpTime);
    setSensorGain(a_u4SensorGain);

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
AAASensorMgr::
getSensorDelayFrame(halSensorDev_e eSensorDEv, MINT32 mode)
{ 
    MINT32 i4DelayFrame;
    
    if(m_pSensorHalObj != NULL) {
        m_pSensorHalObj->sendCommand(eSensorDEv, SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT, (MINT32)&i4DelayFrame, (MINT32)&mode);
    }
    return i4DelayFrame;
}

