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
#include "inc/Local.h"
#include "inc/ParamsManager.h"
//
#if '1'==MTKCAM_HAVE_3A_HAL
    #include <mtkcam/common.h>
    #include <mtkcam/hal/sensor_hal.h>
    #include <mtkcam/hal/aaa_hal_base.h>

    #include <camera_custom_nvram.h>
    #include <dbg_aaa_param.h>
    #include <dbg_flash_param.h>
    #include <flash_mgr.h>
#endif
//
#include <cutils/properties.h>


/******************************************************************************
*
*******************************************************************************/
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
*******************************************************************************/
namespace
{
class ParamsManagerImp : public ParamsManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        
    virtual bool                        updateBestFocusStep() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
                                        ParamsManagerImp(
                                            String8 const& rName, 
                                            int32_t const i4OpenId
                                        );

protected:  ////                        Called by updateDefaultParams1().
    virtual bool                        updateDefaultParams1_ByQuery();
    virtual bool                        updateDefaultFaceCapacity();

protected:  ////                        Called by updateDefaultParams2().
    virtual bool                        updateDefaultParams2_ByQuery();

protected:  ////                        Called by updateDefaultParams3().
    virtual bool                        updateDefaultVideoFormat();

};
};


/******************************************************************************
*
*******************************************************************************/
IParamsManager*
IParamsManager::
createInstance(
    String8 const& rName, 
    int32_t const i4OpenId
)
{
    return  new ParamsManagerImp(rName, i4OpenId);
}


/******************************************************************************
*
*******************************************************************************/
ParamsManagerImp::
ParamsManagerImp(String8 const& rName, int32_t const i4OpenId)
    : ParamsManager(rName, i4OpenId)
{
}


/******************************************************************************
*
*******************************************************************************/
bool 
ParamsManagerImp::
updateBestFocusStep() const
{
#if '1'==MTKCAM_HAVE_3A_HAL
    using namespace NS3A;
    FeatureParam_T r3ASupportedParam;
    memset(&r3ASupportedParam, 0, sizeof(r3ASupportedParam));
    Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(getOpenId()));
    if ( ! p3AHal )
    {
        MY_LOGE("Fail to create 3AHal");
        goto lbExit;
    }
    if ( ! p3AHal->getSupportedParams(r3ASupportedParam) )
    {
        MY_LOGE("getSupportedParams fail");
        goto lbExit;
    }

    MY_LOGD("bt=%d, max_step=%d, min_step=%d",r3ASupportedParam.i4AFBestPos,r3ASupportedParam.i4MaxLensPos,r3ASupportedParam.i4MinLensPos);

    mParameters.set(MtkCameraParameters::KEY_FOCUS_ENG_BEST_STEP, r3ASupportedParam.i4AFBestPos);
    mParameters.set(MtkCameraParameters::KEY_FOCUS_ENG_MAX_STEP, r3ASupportedParam.i4MaxLensPos);
    mParameters.set(MtkCameraParameters::KEY_FOCUS_ENG_MIN_STEP, r3ASupportedParam.i4MinLensPos);

lbExit:
    //
    if  ( p3AHal )
    {
        p3AHal->destroyInstance();
        p3AHal = NULL;
    }
    return true;
#else
    return false;
#endif  //MTKCAM_HAVE_3A_HAL
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManagerImp::
updateDefaultFaceCapacity()
{
    mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, 15);
    mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, 0);
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManagerImp::
updateDefaultParams1_ByQuery()
{
    bool ret = false;
    MY_LOGD("+");
    //
#if '1'==MTKCAM_HAVE_CAMFEATURE
    //
    using namespace NSCameraFeature;
    IFeature*const pFeature = IFeature::createInstance(getOpenId());
    if  ( ! pFeature )
    {
        MY_LOGW("IFeature::createInstance() fail");
        return  false;
    }
    //
    mpFeatureKeyedMap = pFeature->getFeatureKeyedMap();
    pFeature->destroyInstance();
    //
    if  ( ! mpFeatureKeyedMap ) {
        MY_LOGW("NULL mpFeatureKeyedMap");
        return  false;
    }
    //
    //  reset Scene mode to default.
    const_cast<FeatureKeyedMap*>(mpFeatureKeyedMap)->setCurrentSceneMode(String8(CameraParameters::SCENE_MODE_AUTO));
    //
    for (size_t fkey = 0; fkey < mpFeatureKeyedMap->size(); fkey++)
    {
        updateParams(fkey);
    }
    ret = true;
    //
#endif
    //
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManagerImp::
updateDefaultParams2_ByQuery()
{
    bool ret = false;
    MY_LOGD("+");
    //
    mParameters.set(MtkCameraParameters::KEY_SENSOR_TYPE, 0xFF); // default values of sensor type are 1s for YUV type
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    halSensorDev_s halSensorDev = (halSensorDev_s)DevMetaInfo::queryHalSensorDev(mi4OpenId);
    SensorHal* pSensorHal = SensorHal::createInstance();
    if  ( ! pSensorHal ) {
        MY_LOGE("SensorHal::createInstance()");
    }
    else {
        int iFOV_horizontal = 0, iFOV_vertical = 0;
        if  ( 0 != pSensorHal->sendCommand(halSensorDev, SENSOR_CMD_GET_SENSOR_VIEWANGLE, (int)&iFOV_horizontal, (int)&iFOV_vertical) )
        {
            MY_LOGE("SensorHal::sendCommand(%x, SENSOR_CMD_GET_SENSOR_VIEWANGLE)", halSensorDev);
        }
        else
        {
            MY_LOGD("view-angles:%d %d", iFOV_horizontal, iFOV_vertical);
            mParameters.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, iFOV_horizontal);
            mParameters.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, iFOV_vertical);
        }

        unsigned char uiSensorType;
        uiSensorType = 0xFF;
        halSensorType_e eSensorType;
        
        if (0 != pSensorHal->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_TYPE, reinterpret_cast<int>(&eSensorType), 0, 0))
        {
            MY_LOGE("SensorHal::sendCommand(%x, SENSOR_CMD_GET_SENSOR_TYPE)", SENSOR_DEV_MAIN);
        }
        else
        {
            if (SENSOR_TYPE_RAW == eSensorType)
            {
                uiSensorType &= 0xFE;
            }
            else
            {
                uiSensorType |= 0x01;
            }
        }

        if (0 != pSensorHal->sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_TYPE, reinterpret_cast<int>(&eSensorType), 0, 0))
        {
            MY_LOGE("SensorHal::sendCommand(%x, SENSOR_CMD_GET_SENSOR_TYPE)", SENSOR_DEV_SUB);
        }
        else
        {
            if (SENSOR_TYPE_RAW == eSensorType)
            {
                uiSensorType &= 0xFD;
            }
            else
            {
                uiSensorType |= 0x02;
            }
        }

        mParameters.set(MtkCameraParameters::KEY_SENSOR_TYPE, uiSensorType);
        MY_LOGD("KEY_SENSOR_TYPE = 0x%X", uiSensorType);

        pSensorHal->destroyInstance();
        pSensorHal = NULL;
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    //
    //  (1) Query from CORE
    using namespace NS3A;
    FeatureParam_T r3ASupportedParam;
    memset(&r3ASupportedParam, 0, sizeof(r3ASupportedParam));
    Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(getOpenId()));
    if ( ! p3AHal )
    {
        MY_LOGE("Fail to create 3AHal");
        return ret;
    }
    if ( ! p3AHal->getSupportedParams(r3ASupportedParam) )
    {
        MY_LOGE("getSupportedParams fail");
        goto lbExit;
    }
    //
    //  AE/AWB Lock
#undef TRUE
#undef FALSE
    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, r3ASupportedParam.bExposureLockSupported ? 
                                    CameraParameters::TRUE : CameraParameters::FALSE);
    mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, r3ASupportedParam.bAutoWhiteBalanceLockSupported ? 
                                    CameraParameters::TRUE : CameraParameters::FALSE);
    //
    //  AE/AF areas
    mParameters.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, r3ASupportedParam.u4MaxFocusAreaNum);
    mParameters.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS, r3ASupportedParam.u4MaxMeterAreaNum);

    mParameters.set(MtkCameraParameters::KEY_FOCUS_ENG_MAX_STEP, r3ASupportedParam.i4MaxLensPos);
    mParameters.set(MtkCameraParameters::KEY_FOCUS_ENG_MIN_STEP, r3ASupportedParam.i4MinLensPos);

    // Focus Full Scan Step Range
    mParameters.set(MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX, MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX_DEFAULT);
    mParameters.set(MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN, MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN_DEFAULT);
    mParameters.set(MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL, MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN_DEFAULT);
    // Shading table initial value
    mParameters.set(MtkCameraParameters::KEY_ENG_SHADING_TABLE, MtkCameraParameters::KEY_ENG_SHADING_TABLE_AUTO);
    mParameters.set(MtkCameraParameters::KEY_ENG_SAVE_SHADING_TABLE, 0);
    //
    if (FlashMgr::getInstance())
    {
        int st, ed;

        FlashMgr::getInstance()->egGetDutyRange(&st, &ed);
        MY_LOGD("duty range = %d ~ %d", st, ed);
        mParameters.set(MtkCameraParameters::KEY_ENG_FLASH_DUTY_MIN, st);
        mParameters.set(MtkCameraParameters::KEY_ENG_FLASH_DUTY_MAX, ed);
        mParameters.set(MtkCameraParameters::KEY_ENG_FLASH_DUTY_VALUE, MtkCameraParameters::KEY_ENG_FLASH_DUTY_DEFAULT_VALUE);

        FlashMgr::getInstance()->egGetStepRange(&st, &ed);
        MY_LOGD("step range = %d ~ %d", st, ed);
        mParameters.set(MtkCameraParameters::KEY_ENG_FLASH_STEP_MIN, st);
        mParameters.set(MtkCameraParameters::KEY_ENG_FLASH_STEP_MAX, ed);

        FlashMgr::getInstance()->egSetMfDutyStep(MtkCameraParameters::KEY_ENG_FLASH_DUTY_DEFAULT_VALUE, mParameters.getInt(MtkCameraParameters::KEY_ENG_FLASH_STEP_MAX)); // Default values for flash
    }
    //
    ret = true;
    //
lbExit:
    //
    if  ( p3AHal )
    {
        p3AHal->destroyInstance();
        p3AHal = NULL;
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
ParamsManagerImp::
updateDefaultVideoFormat()
{
    mParameters.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420P);
    return  true;
}

