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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include "MtkAtvCam1Device.h"
//
using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
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
extern "C"
NSCam::Cam1Device*
createCam1Device_MtkAtv(
    String8 const&          rDevName, 
    int32_t const           i4OpenId
)
{
    return new MtkAtvCam1Device(rDevName, i4OpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
MtkAtvCam1Device::
MtkAtvCam1Device(
    String8 const&          rDevName, 
    int32_t const           i4OpenId
)
    : Cam1DeviceBase(rDevName, i4OpenId)
    //
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    , mpSensorHal(NULL)
#endif
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    , mp3AHal(NULL)
#endif
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MtkAtvCam1Device::
~MtkAtvCam1Device()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
MtkAtvCam1Device::
onInit()
{
    MY_LOGD("+");
    //
    initPlatformProfile();
    bool    ret = false;
    int     err = 0;
    //
    IResManager* pResManager = IResManager::getInstance();
    //
    if  ( pResManager != NULL )
    {
        if(!(pResManager->open("MtkAtvCam1Device")))
        {
            MY_LOGE("pResManager->open fail");
            goto lbExit;
        }
    }
    //
    //--------------------------------------------------------------------------
    //  (1) Open Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    MY_LOGD("SensorHal::createInstance()");
    mpSensorHal = SensorHal::createInstance();
    if  ( ! mpSensorHal ) {
        MY_LOGE("SensorHal::createInstance() fail");
        goto lbExit;
    }
    //
    err = mpSensorHal->sendCommand(SENSOR_DEV_ATV, SENSOR_CMD_SET_SENSOR_DEV);
    if  ( err ) {
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        goto lbExit;
    }
    err = mpSensorHal->init();
    if  ( err ) {
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        MY_LOGE("SensorHal::init() fail");
        goto lbExit;
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
    //  (2) Open 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    mp3AHal = NS3A::Hal3ABase::createInstance(SENSOR_DEV_ATV);
    if  ( ! mp3AHal ) {
        MY_LOGE("Hal3ABase::createInstance() fail");
        goto lbExit;
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    //  (3) Init Base.
    if  ( ! Cam1DeviceBase::onInit() )
    {
        MY_LOGE("Cam1DeviceBase::onInit() fail");
        goto lbExit;
    }
    //
    //--------------------------------------------------------------------------
    //  (4) Initialize Camera Adapter.
    if  ( ! initCameraAdapter() )
    {
        MY_LOGE("NULL Camera Adapter");
        goto lbExit;
    }
    //--------------------------------------------------------------------------
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
MtkAtvCam1Device::
onUninit()
{
    MY_LOGD("+");
    //
    //--------------------------------------------------------------------------
    //  (1) Uninitialize Camera Adapter.
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->cancelPicture();
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    //
    //--------------------------------------------------------------------------
    //  (2) Uninit Base
    Cam1DeviceBase::onUninit();
    //--------------------------------------------------------------------------
    //  (3) Cloase 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    if  ( mp3AHal )
    {
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    //  (4) Close Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    if  ( mpSensorHal )
    {
        mpSensorHal->uninit();
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        MY_LOGD("SensorHal::destroyInstance()");
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------

    IResManager* pResManager = IResManager::getInstance();
    //
    if  ( pResManager != NULL )
    {
        if(!(pResManager->close("MtkAtvCam1Device")))
        {
            MY_LOGE("pResManager->close fail");
        }
    }

    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *  Set the camera parameters. This returns BAD_VALUE if any parameter is
 *  invalid or not supported.
 ******************************************************************************/
status_t
MtkAtvCam1Device::
setParameters(const char* params)
{
    status_t status = OK;
    //
    //  (1) Update params to mpParamsMgr.
    status = mpParamsMgr->setParameters(String8(params));
    if  ( OK != status ) {
        goto lbExit;
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    {
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
    }

lbExit:
    return  status;
}

