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

//
#include <utils/String8.h>
#include "inc/Local.h"
#include "feature/inc/Feature.h"
//
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    #include <mtkcam/hal/sensor_hal.h>
    #include <camera_custom_sensor.h>
#endif
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[Feature::%s] "fmt, __FUNCTION__, ##arg)
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
bool
querySensorInfo(int32_t const i4OpenId, String8& rs8SensorName, uint32_t& ru4SensorType)
{
    if  ( DevMetaInfo::queryDeviceId(i4OpenId) == eDevId_AtvSensor )
    {
        rs8SensorName = DLSYM_MODULE_NAME_COMMON_SENSOR_ATV;
        ru4SensorType = NSSensorType::eSensorType_YUV;
        MY_LOGW("ATV sensor...return true");
        return  true;
    }

#if '1'!=MTKCAM_HAVE_SENSOR_HAL     //++++++++++++++++++++++++++++++++++++++++++
    //
    #warning "[FIXME] querySensorInfo()"
    rs8SensorName = "No_Sensor_Hal";
    ru4SensorType = NSSensorType::eSensorType_RAW;
    return  true;
    //
#else   //MTKCAM_HAVE_SENSOR_HAL    //..........................................
    //
    using namespace NSFeature;
    SensorInfoBase* pSensorInfo = NULL;
    bool ret = false;
    halSensorDev_s halSensorDev = (halSensorDev_s)DevMetaInfo::queryHalSensorDev(i4OpenId);
    SensorHal* pSensorHal = SensorHal::createInstance();
    if  ( ! pSensorHal ) {
        MY_LOGE("SensorHal::createInstance()");
        goto lbExit;
    }
    //
    if  ( 0 != pSensorHal->sendCommand(halSensorDev, SENSOR_CMD_GET_SENSOR_FEATURE_INFO, (int)&pSensorInfo) || ! pSensorInfo )
    {
        MY_LOGE("SensorHal::sendCommand(%x, SENSOR_CMD_GET_SENSOR_FEATURE_INFO), pSensorInfo(%p)", halSensorDev, pSensorInfo);
        goto lbExit;
    }
    //
    MY_LOGD("type:%d <%#x/%s/%s>", pSensorInfo->GetType(), pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
    rs8SensorName = pSensorInfo->getDrvMacroName();
    switch  (pSensorInfo->GetType())
    {
    case SensorInfoBase::EType_RAW:
        ru4SensorType = NSSensorType::eSensorType_RAW;
        break;
    case SensorInfoBase::EType_YUV:
        ru4SensorType = NSSensorType::eSensorType_YUV;
        break;
    }
    //
    ret = true;
lbExit:
    if  ( pSensorHal ) {
        pSensorHal->destroyInstance();
        pSensorHal = NULL;
    }
    return ret;
    //
#endif  //MTKCAM_HAVE_SENSOR_HAL    //------------------------------------------
}

