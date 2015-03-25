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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
//#include "inc/BaseCamAdapter.h"
//
#include <cutils/properties.h>

#include <mtkcam/hal/sensor_hal.h>

/******************************************************************************
*   Function Prototype.
*******************************************************************************/

sp<ICamAdapter> createMtkZsdNccCamAdapter(String8 const& rName, int32_t const i4OpenId,  sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkZsdCcCamAdapter(String8 const& rName, int32_t const i4OpenId,  sp<IParamsManager> pParamsMgr);


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[MtkZsdCamAdapter::%s] "fmt,  __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)



// 5M sensor size (2528 1896)
// 8M sensor size (3264 2448)
// 12M sensor size (4000 3000)?

#define SENSOR_8M_WIDTH_MIN (3000)
#define SENSOR_8M_WIDTH_MAX (3500)

#define ZSD_MODE_NCC        (0x1)
#define ZSD_MODE_CC         (0x2)

/******************************************************************************
*
*******************************************************************************/
sp<ICamAdapter>
createMtkZsdCamAdapter(
    String8 const&      rName,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
)
{
    //
    int32_t         zsdMode = 0;
    SensorHal*      pSensorHal = NULL;
    halSensorDev_e  eSensorDev;
    halSensorType_e eSensorType;
    uint32_t        u4TgInW = 0;
    uint32_t        u4TgInH = 0;
    uint32_t        scenario = ACDK_SCENARIO_ID_CAMERA_ZSD;
    uint32_t        fps = 0;
    String8 const   s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(pParamsMgr->getHalAppMode());
    int             err = 0;

    // 1) query sensor info
    MY_LOGD("SensorHal::createInstance(), i4OpenId:%d", i4OpenId);
    pSensorHal = SensorHal::createInstance();
    if  ( ! pSensorHal ) {
        MY_LOGE("pSensorHal == NULL");
        goto lbExit;
    }
    //
    eSensorDev = (halSensorDev_e)DevMetaInfo::queryHalSensorDev(i4OpenId);
    // raw or yuv
    pSensorHal->sendCommand(eSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (int32_t)&eSensorType);
    // sensor full size
    pSensorHal->sendCommand(eSensorDev, SENSOR_CMD_GET_SENSOR_FULL_RANGE, (int32_t)&u4TgInW, (int32_t)&u4TgInH);
    // sensor full size fps
    pSensorHal->sendCommand(eSensorDev, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (int32_t)&scenario, (int32_t)&fps);
    //
    pSensorHal->destroyInstance();
    pSensorHal = NULL;


    // 2) CC or NCC
    // 5M case:
    if (u4TgInW < SENSOR_8M_WIDTH_MIN)
    {
        zsdMode = ZSD_MODE_CC;
    }
    // 13M case:
    else if (u4TgInW > SENSOR_8M_WIDTH_MAX)
    {
        zsdMode = ZSD_MODE_NCC;
    }
    // 8M case:
    else
    {
        if (fps > 240)
            zsdMode = ZSD_MODE_NCC;
        else
            zsdMode = ZSD_MODE_CC;
    }

    MY_LOGD("Sensor full size (%dx%d) fps(%d) ZSD mode (%s)",
        u4TgInW, u4TgInH, fps,(zsdMode == ZSD_MODE_NCC)?"ZSDNCC":"ZSDCC");
    //
lbExit:
    // 3) Get property for debug
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.zsdmode", value, "0");
    int pty = atoi(value);
    if ( pty > 0 ) {
        zsdMode = pty;
        MY_LOGD("Set zsd mode by property(%d)(%s)", zsdMode, (zsdMode == ZSD_MODE_NCC)?"ZSDNCC":"ZSDCC");
    }

    if ( ZSD_MODE_NCC == zsdMode ) {
        return  createMtkZsdNccCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    else {
        return  createMtkZsdCcCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }

}




