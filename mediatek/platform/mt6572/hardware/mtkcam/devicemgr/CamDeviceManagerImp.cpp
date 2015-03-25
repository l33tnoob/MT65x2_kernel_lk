/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/devicemgr"
//
#include "MyUtils.h"
#include "CamDeviceManagerImp.h"
using namespace android;
using namespace NSCam;
//
#include <mtkcam/v1/camutils/CamInfo.h>
using namespace MtkCamUtils;
//
/******************************************************************************
 *
 ******************************************************************************/
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    #include <mtkcam/hal/sensor_hal.h>
#else
    #warning "[Warn] Not support Sensor Hal"
#endif
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace
{
    CamDeviceManagerImp gCamDeviceManager;
}   //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
ICamDeviceManager*
getCamDeviceManager()
{
    return  &gCamDeviceManager;
}
}


/******************************************************************************
 *
 ******************************************************************************/
CamDeviceManagerImp::
CamDeviceManagerImp()
    : CamDeviceManagerBase()
{
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerImp::
validateOpenLocked(int32_t i4OpenId) const
{
    status_t status = OK;
    //
    status = CamDeviceManagerBase::validateOpenLocked(i4OpenId);
    if  ( OK != status )
    {
        return  status;
    }
    //
    if  ( MAX_SIMUL_CAMERAS_SUPPORTED <= mOpenMap.size() )
    {
        MY_LOGE("Cannot open device %d ...", i4OpenId);
        MY_LOGE("open count(%d) >= maximum count(%d)", mOpenMap.size(), MAX_SIMUL_CAMERAS_SUPPORTED);
        status = NO_MEMORY;
    }
    //
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamDeviceManagerImp::
enumDeviceLocked()
{
    Utils::CamProfile _profile(__FUNCTION__, "CamDeviceManagerImp");
    //
    status_t status = OK;
    int32_t i4DeviceNum = 0;
    //
    mEnumMap.clear();
//------------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL

    mEnumMap.clear();
    DevMetaInfo::clear();
    //
    int32_t isFakeOrientation = 0;
    int32_t i4DevSetupOrientation = 0;
    camera_info camInfo;
    camInfo.device_version = CAMERA_DEVICE_API_VERSION_1_0;
    camInfo.static_camera_characteristics = NULL;
    //
    SensorHal* pSensorHal = SensorHal::createInstance();
    if  ( ! pSensorHal )
    {
        MY_LOGE("pSensorHal == NULL");
        return 0;
    }
    //
    int32_t const iSensorsList = pSensorHal->searchSensor();
    //
    //
    if  ( (iSensorsList & SENSOR_DEV_MAIN_3D) == SENSOR_DEV_MAIN_3D )
    {
        MY_LOGI("Stereo 3D Camera found");
#warning "[TODO] Stereo 3D Camera"
    }
    //
    if  ( (iSensorsList & SENSOR_DEV_MAIN) == SENSOR_DEV_MAIN )
    {
        int32_t const deviceId = i4DeviceNum;
        //
        halSensorDev_e const eHalSensorDev = SENSOR_DEV_MAIN;
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_FAKE_ORIENTATION, (int)&isFakeOrientation);
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE, (int)&i4DevSetupOrientation);
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_SENSOR_FACING_DIRECTION, (int)&camInfo.facing);
        camInfo.orientation = i4DevSetupOrientation;
        if  ( isFakeOrientation )
        {
            camInfo.orientation = (0==camInfo.facing) ? 90 : 270;
            MY_LOGW("Fake orientation:%d instead of %d, facing=%d HalSensorDev=%#x", camInfo.orientation, i4DevSetupOrientation, camInfo.facing, eHalSensorDev);
        }
        DevMetaInfo::add(deviceId, camInfo, i4DevSetupOrientation, eDevId_ImgSensor, eHalSensorDev);
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = camInfo.facing;
        pInfo->iWantedOrientation   = camInfo.orientation;
        pInfo->iSetupOrientation    = i4DevSetupOrientation;
        mEnumMap.add(deviceId, pInfo);
        //
        i4DeviceNum++;
    }
    //
    if  ( (iSensorsList & SENSOR_DEV_SUB) == SENSOR_DEV_SUB )
    {
        int32_t const deviceId = i4DeviceNum;
        //
        halSensorDev_e const eHalSensorDev = SENSOR_DEV_SUB;
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_FAKE_ORIENTATION, (int)&isFakeOrientation);
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE, (int)&i4DevSetupOrientation);
        pSensorHal->sendCommand(eHalSensorDev, SENSOR_CMD_GET_SENSOR_FACING_DIRECTION, (int)&camInfo.facing);
        camInfo.orientation = i4DevSetupOrientation;
        if  ( isFakeOrientation )
        {
            camInfo.orientation = (0==camInfo.facing) ? 90 : 270;
            MY_LOGW("Fake orientation:%d instead of %d, facing=%d HalSensorDev=%#x", camInfo.orientation, i4DevSetupOrientation, camInfo.facing, eHalSensorDev);
        }
        DevMetaInfo::add(deviceId, camInfo, i4DevSetupOrientation, eDevId_ImgSensor, eHalSensorDev);
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = camInfo.facing;
        pInfo->iWantedOrientation   = camInfo.orientation;
        pInfo->iSetupOrientation    = i4DevSetupOrientation;
        mEnumMap.add(deviceId, pInfo);
        //
        i4DeviceNum++;
    }
    //
//    if  ( (iSensorsList & SENSOR_DEV_ATV) == SENSOR_DEV_ATV )
    {
        int32_t const deviceId = 0xFF;
        //
        halSensorDev_e const eHalSensorDev = SENSOR_DEV_ATV;
        camInfo.facing = 0;
        camInfo.orientation = 0;
        DevMetaInfo::add(deviceId, camInfo, camInfo.orientation, eDevId_AtvSensor, eHalSensorDev);
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = camInfo.facing;
        pInfo->iWantedOrientation   = camInfo.orientation;
        pInfo->iSetupOrientation    = i4DevSetupOrientation;
        mEnumMap.add(deviceId, pInfo);
        //
//        i4DeviceNum++;
    }
    //
    //
    if  ( pSensorHal )
    {
        pSensorHal->destroyInstance();
        pSensorHal = NULL;
    }
    //
    MY_LOGI("iSensorsList=0x%08X, i4DeviceNum=%d", iSensorsList, i4DeviceNum);
    for (size_t i = 0; i < mEnumMap.size(); i++)
    {
        int32_t const deviceId = mEnumMap.keyAt(i);
        sp<EnumInfo> pInfo = mEnumMap.valueAt(i);
        uint32_t const uDeviceVersion   = pInfo->uDeviceVersion;
        camera_metadata const*pMetadata = pInfo->pMetadata;
        int32_t const iFacing           = pInfo->iFacing;
        int32_t const iWantedOrientation= pInfo->iWantedOrientation;
        int32_t const iSetupOrientation = pInfo->iSetupOrientation;
        MY_LOGI(
            "[0x%02x] orientation(wanted/setup)=(%d/%d) facing:%d metadata:%p DeviceVersion:0x%x", 
            deviceId, iWantedOrientation, iSetupOrientation, 
            iFacing, pMetadata, uDeviceVersion
        );
    }

#else   //----------------------------------------------------------------------

    #warning "[WARN] Simulation for CamDeviceManagerImp::enumDeviceLocked()"

    mEnumMap.clear();
    DevMetaInfo::clear();
    {
        int32_t const deviceId = 0;
        //
        camera_info camInfo;
        camInfo.device_version  = CAMERA_DEVICE_API_VERSION_1_0;
        camInfo.static_camera_characteristics = NULL;
        camInfo.facing      = 0;
        camInfo.orientation = 90;
        DevMetaInfo::add(deviceId, camInfo, camInfo.orientation, eDevId_ImgSensor, 0x01/*SENSOR_DEV_MAIN*/);
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = 0;
        pInfo->iWantedOrientation   = 90;
        pInfo->iSetupOrientation    = 90;
        mEnumMap.add(deviceId, pInfo);
    }
    //
    {
        int32_t const deviceId = 0xFF;
        //
        camera_info camInfo;
        camInfo.device_version  = CAMERA_DEVICE_API_VERSION_1_0;
        camInfo.static_camera_characteristics = NULL;
        camInfo.facing      = 0;
        camInfo.orientation = 0;
        DevMetaInfo::add(deviceId, camInfo, camInfo.orientation, eDevId_AtvSensor, 0x04/*SENSOR_DEV_ATV*/);
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = 0;
        pInfo->iWantedOrientation   = 0;
        pInfo->iSetupOrientation    = 0;
        mEnumMap.add(deviceId, pInfo);
    }
    //
    i4DeviceNum = 1;

#endif
//------------------------------------------------------------------------------
    //
    _profile.print("");
    return  i4DeviceNum;
}

