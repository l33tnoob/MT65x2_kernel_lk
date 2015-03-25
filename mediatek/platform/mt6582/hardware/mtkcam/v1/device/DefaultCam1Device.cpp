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
#include "DefaultCam1Device.h"
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
createCam1Device_Default(
    String8 const&          rDevName, 
    int32_t const           i4OpenId
)
{
    return new DefaultCam1Device(rDevName, i4OpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
DefaultCam1Device::
DefaultCam1Device(
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
DefaultCam1Device::
~DefaultCam1Device()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DefaultCam1Device::
onInit()
{
    MY_LOGD("+");
    initPlatformProfile(); 
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_init);
    CamProfile  profile(__FUNCTION__, "DefaultCam1Device");
    //
    bool    ret = false;
    int     err = 0;
    int const iHalSensorDevId = DevMetaInfo::queryHalSensorDev(getOpenId());
    //
    //--------------------------------------------------------------------------
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
    IResManager* pResManager = IResManager::getInstance();
    //
    if  ( pResManager != NULL )
    {
        if(!(pResManager->open("DefaultCam1Device")))
        {
            MY_LOGE("pResManager->open fail");
            goto lbExit;
        }
    }
    //
    profile.print("Resource -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    //--------------------------------------------------------------------------
    //  (1) Open Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
    MY_LOGD("SensorHal::createInstance(), iHalSensorDevId:%#x", iHalSensorDevId);
    mpSensorHal = SensorHal::createInstance();
    if  ( ! mpSensorHal ) {
        MY_LOGE("mpSensorHal == NULL");
        goto lbExit;
    }
    //
    err = mpSensorHal->sendCommand((halSensorDev_e)iHalSensorDevId, SENSOR_CMD_SET_SENSOR_DEV);
    if  ( err ) {
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        goto lbExit;
    }
    err = mpSensorHal->init();
    if  ( err ) {
        mpSensorHal->uninit();
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        goto lbExit;
    }
    profile.print("Sensor Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
    //  (2) Open 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");
    mp3AHal = NS3A::Hal3ABase::createInstance(iHalSensorDevId);
    if  ( ! mp3AHal ) {
        MY_LOGE("Hal3ABase::createInstance() fail");
        goto lbExit;
    }
    profile.print("3A Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    //  (3) Init Base.
    if  ( ! Cam1DeviceBase::onInit() )
    {
        goto lbExit;
    }
    //
    //--------------------------------------------------------------------------
    //
    ret = true;
lbExit:
    profile.print("");
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DefaultCam1Device::
onUninit()
{
    MY_LOGD("+");
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_uninit);
    CamProfile  profile(__FUNCTION__, "DefaultCam1Device");
    //
    //--------------------------------------------------------------------------
    //  (1) Uninit Base
    Cam1DeviceBase::onUninit();
    profile.print("Cam1DeviceBase::onUninit() -");
    //--------------------------------------------------------------------------
    //  (2) Close 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");
    if  ( mp3AHal )
    {
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }
    profile.print("3A Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    //  (4) Close Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
    if  ( mpSensorHal )
    {
        mpSensorHal->uninit();
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
        MY_LOGD("SensorHal::destroyInstance()");
    }
    profile.print("Sensor Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
    IResManager* pResManager = IResManager::getInstance();
    //
    if  ( pResManager != NULL )
    {
        if(!(pResManager->close("DefaultCam1Device")))
        {
            MY_LOGE("pResManager->close fail");
        }
    }
    profile.print("Resource -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    //--------------------------------------------------------------------------
    //
    profile.print("");
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 * [Template method] Called by startPreview().
 ******************************************************************************/
bool
DefaultCam1Device::
onStartPreview()
{
    bool ret = false;
    //
    //  (1) Update Hal App Mode.
    if  ( ! mpParamsMgr->updateHalAppMode() )
    {
        MY_LOGE("mpParamsMgr->updateHalAppMode() fail");
        goto lbExit;
    }

    //  (2) Initialize Camera Adapter.
    if  ( ! initCameraAdapter() )
    {
        MY_LOGE("NULL Camera Adapter");
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *  [Template method] Called by stopPreview().
 ******************************************************************************/
void
DefaultCam1Device::
onStopPreview()
{
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->cancelPicture();
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
}


/******************************************************************************
 *  Set the camera parameters. This returns BAD_VALUE if any parameter is 
 *  invalid or not supported.
 ******************************************************************************/
status_t
DefaultCam1Device::
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
#if '1'==MTKCAM_HAVE_3A_HAL
        else if ( mp3AHal )
        {
            //  Flashlight may turn on/off in case that CamAdapter doesn't exist (i.e. never call startPreview)

            using namespace NS3A;
            Param_T param;
            //
            if  ( ! mp3AHal->getParams(param) ) {
                MY_LOGW("3A Hal::getParams() fail - err(%x)", mp3AHal->getErrorCode());
            }
            //
            String8 const s8FlashMode = mpParamsMgr->getStr(CameraParameters::KEY_FLASH_MODE);
            if  ( ! s8FlashMode.isEmpty() ) {
                param.u4StrobeMode = PARAMSMANAGER_MAP_INST(eMapFlashMode)->valueFor(s8FlashMode);
            }
            //
            if  ( ! mp3AHal->setParams(param) ) {
                MY_LOGW("3A Hal::setParams() fail - err(%x)", mp3AHal->getErrorCode());
            }
        }
        else
        {
            MY_LOGW("mp3AHal==NULL");
        }
#endif
    }

lbExit:
    return  status;
}

