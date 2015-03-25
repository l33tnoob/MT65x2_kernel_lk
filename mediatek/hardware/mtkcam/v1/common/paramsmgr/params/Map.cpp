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

#ifndef _MTK_CAMERA_PARAMSMGR_MAP_H_
#define _MTK_CAMERA_PARAMSMGR_MAP_H_
//
#include "inc/Local.h"
#include <mtkcam/common.h>
using namespace NSCam;
#include <camera_feature.h>
using namespace NSFeature;


/*******************************************************************************
*
*******************************************************************************/


namespace android {
/*******************************************************************************
*
*******************************************************************************/
namespace
{
class ParamMap : public IParamsManager::IMap
{
public:
    virtual VAL_T   valueFor(STR_T const& str) const    { return m_vStr2Val.valueFor(str); }
    virtual STR_T   stringFor(VAL_T const& val) const   { return m_vVal2Str.valueFor(val); }

                    ParamMap(const char strDefault[], VAL_T const& valDefault)
                        : m_vStr2Val(valDefault)
                        , m_vVal2Str(String8(strDefault))
                    {
                        m_vStr2Val.clear();
                        m_vVal2Str.clear();
                    }

protected:
    DefaultKeyedVector<STR_T, VAL_T>    m_vStr2Val;
    DefaultKeyedVector<VAL_T, STR_T>    m_vVal2Str;
};
}


/*******************************************************************************
*   
*******************************************************************************/
#define _DEFAULT_STRING_VALUE_(_default_...) (_default_)
#define _PARAM_MAP_BEG_(_map_enum_) \
    struct TYPE##_map_enum_ : public ParamMap \
    { \
        static TYPE##_map_enum_ const  singleton; \
    protected: \
        TYPE##_map_enum_() \
            : ParamMap

#define _PARAM_MAP_END_(_map_enum_) \
    }; \
    TYPE##_map_enum_ const TYPE##_map_enum_::singleton; \
    template <> IParamsManager::IMap const* IParamsManager::getMapInst(IParamsManager::int2type<IParamsManager::_map_enum_>) { return &TYPE##_map_enum_::singleton; }


#define _ADD_STRING_VALUE_MAP_(_str_, _val_) \
    do {\
        m_vStr2Val.add(String8(_str_), _val_); \
        m_vVal2Str.add(_val_, String8(_str_)); \
    } while (0)

#define _ADD_STRING_VALUE_MAP2_(_str_, _val_to_str_, _val_from_str_) \
    do {\
        m_vStr2Val.add(String8(_str_), _val_from_str_); \
        m_vVal2Str.add(_val_to_str_, String8(_str_)); \
    } while (0)


/*******************************************************************************
*   Internal App Mode.
*******************************************************************************/
_PARAM_MAP_BEG_(eMapAppMode)
    _DEFAULT_STRING_VALUE_(MtkCameraParameters::APP_MODE_NAME_DEFAULT,      eAppMode_DefaultMode) // Default
    {
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_DEFAULT,      eAppMode_DefaultMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_ENG,      eAppMode_EngMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_ATV,      eAppMode_AtvMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_S3D,      eAppMode_S3DMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_VT,       eAppMode_VtMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO,    eAppMode_PhotoMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO,    eAppMode_VideoMode);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::APP_MODE_NAME_MTK_ZSD,      eAppMode_ZsdMode);
    }
_PARAM_MAP_END_(eMapAppMode)


/*******************************************************************************
*   Level
*******************************************************************************/
_PARAM_MAP_BEG_(eMapLevel)
    _DEFAULT_STRING_VALUE_(MtkCameraParameters::MIDDLE,     1) // Default
    {
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::LOW,        0);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::MIDDLE,     1);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::HIGH,       2);
    }
_PARAM_MAP_END_(eMapLevel)


/*******************************************************************************
*   Scene
*******************************************************************************/
_PARAM_MAP_BEG_(eMapScene)
    _DEFAULT_STRING_VALUE_(   CameraParameters::SCENE_MODE_AUTO,            SCENE_MODE_OFF) // Default
    {
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_AUTO,            SCENE_MODE_OFF);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::SCENE_MODE_NORMAL,          SCENE_MODE_NORMAL);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_PORTRAIT,        SCENE_MODE_PORTRAIT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_LANDSCAPE,       SCENE_MODE_LANDSCAPE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_NIGHT,           SCENE_MODE_NIGHTSCENE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_NIGHT_PORTRAIT,  SCENE_MODE_NIGHTPORTRAIT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_THEATRE,         SCENE_MODE_THEATRE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_BEACH,           SCENE_MODE_BEACH);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SNOW,            SCENE_MODE_SNOW);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SUNSET,          SCENE_MODE_SUNSET);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_STEADYPHOTO,     SCENE_MODE_STEADYPHOTO);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_FIREWORKS,       SCENE_MODE_FIREWORKS);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SPORTS,          SCENE_MODE_SPORTS);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_PARTY,           SCENE_MODE_PARTY);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_CANDLELIGHT,     SCENE_MODE_CANDLELIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_HDR,             SCENE_MODE_HDR);
    }
_PARAM_MAP_END_(eMapScene)


/*******************************************************************************
*   Effect
*******************************************************************************/
_PARAM_MAP_BEG_(eMapEffect)
    _DEFAULT_STRING_VALUE_(   CameraParameters::EFFECT_NONE,            MEFFECT_OFF) // Default
    {
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_NONE,            MEFFECT_OFF);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_MONO,            MEFFECT_MONO);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_SEPIA,           MEFFECT_SEPIA);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_NEGATIVE,        MEFFECT_NEGATIVE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_SOLARIZE,        MEFFECT_SOLARIZE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_AQUA,            MEFFECT_AQUA);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_BLACKBOARD,      MEFFECT_BLACKBOARD);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_POSTERIZE,       MEFFECT_POSTERIZE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::EFFECT_WHITEBOARD,      MEFFECT_WHITEBOARD);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::EFFECT_SEPIA_BLUE,      MEFFECT_SEPIABLUE);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::EFFECT_SEPIA_GREEN,     MEFFECT_SEPIAGREEN);
    }
_PARAM_MAP_END_(eMapEffect)


/*******************************************************************************
*   WhiteBalance
*******************************************************************************/
_PARAM_MAP_BEG_(eMapWhiteBalance)
    _DEFAULT_STRING_VALUE_(   CameraParameters::WHITE_BALANCE_AUTO,             AWB_MODE_AUTO) // Default
    {
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_AUTO,             AWB_MODE_AUTO);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_INCANDESCENT,     AWB_MODE_INCANDESCENT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_FLUORESCENT,      AWB_MODE_FLUORESCENT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT, AWB_MODE_WARM_FLUORESCENT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_DAYLIGHT,         AWB_MODE_DAYLIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,  AWB_MODE_CLOUDY_DAYLIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_TWILIGHT,         AWB_MODE_TWILIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::WHITE_BALANCE_SHADE,            AWB_MODE_SHADE);
    _ADD_STRING_VALUE_MAP_(MtkCameraParameters::WHITE_BALANCE_TUNGSTEN,         AWB_MODE_TUNGSTEN);
    }
_PARAM_MAP_END_(eMapWhiteBalance)


/*******************************************************************************
*   focus mode
*******************************************************************************/
_PARAM_MAP_BEG_(eMapFocusMode)
    _DEFAULT_STRING_VALUE_(CameraParameters::FOCUS_MODE_INFINITY,           AF_MODE_INFINITY) // Default
    {
    _ADD_STRING_VALUE_MAP_(CameraParameters::FOCUS_MODE_AUTO,               AF_MODE_AFS);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FOCUS_MODE_INFINITY,           AF_MODE_INFINITY);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FOCUS_MODE_MACRO,              AF_MODE_MACRO);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO,   AF_MODE_AFC_VIDEO);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE, AF_MODE_AFC);
    _ADD_STRING_VALUE_MAP_("manual",                                        AF_MODE_MF);
    _ADD_STRING_VALUE_MAP_("fullscan",                                      AF_MODE_FULLSCAN);
    }
_PARAM_MAP_END_(eMapFocusMode)


/*******************************************************************************
*   focus lamp
*******************************************************************************/
_PARAM_MAP_BEG_(eMapFocusLamp)
    _DEFAULT_STRING_VALUE_("off",   AF_LAMP_OFF) // Default
    {
    _ADD_STRING_VALUE_MAP_("off",   AF_LAMP_OFF);
    _ADD_STRING_VALUE_MAP_("on",    AF_LAMP_ON);
    _ADD_STRING_VALUE_MAP_("auto",  AF_LAMP_AUTO);
    _ADD_STRING_VALUE_MAP_("flash", AF_LAMP_FLASH);
    }
_PARAM_MAP_END_(eMapFocusLamp)


/*******************************************************************************
*   exposure mode
*******************************************************************************/
_PARAM_MAP_BEG_(eMapExpMode)
    _DEFAULT_STRING_VALUE_(   CameraParameters::SCENE_MODE_AUTO,            AE_MODE_AUTO) // Default
    {
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_AUTO,            AE_MODE_AUTO);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_NIGHT,           AE_MODE_NIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_ACTION,          AE_MODE_ACTION);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_BEACH,           AE_MODE_BEACH);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_CANDLELIGHT,     AE_MODE_CANDLELIGHT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_FIREWORKS,       AE_MODE_FIREWORKS);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_LANDSCAPE,       AE_MODE_LANDSCAPE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_PORTRAIT,        AE_MODE_PORTRAIT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_NIGHT_PORTRAIT,  AE_MODE_NIGHT_PORTRAIT);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_PARTY,           AE_MODE_PARTY);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SNOW,            AE_MODE_SNOW);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SPORTS,          AE_MODE_SPORTS);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_STEADYPHOTO,     AE_MODE_STEADYPHOTO);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_SUNSET,          AE_MODE_SUNSET);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_THEATRE,         AE_MODE_THEATRE);
    _ADD_STRING_VALUE_MAP_(   CameraParameters::SCENE_MODE_HDR,             AE_MODE_HDR);
    }
_PARAM_MAP_END_(eMapExpMode)


/*******************************************************************************
*   ISO
*******************************************************************************/
_PARAM_MAP_BEG_(eMapIso)
    _DEFAULT_STRING_VALUE_(MtkCameraParameters::ISO_SPEED_AUTO,     0) // Default
    {
    }
    //
    virtual VAL_T   valueFor(STR_T const& str) const
    {
        VAL_T value = 0;
        if  ( str == MtkCameraParameters::ISO_SPEED_AUTO ) {
            value = 0;
        }
        else {
            value = ::atoi(str.string());
        }
        //
        CAM_LOGD_IF(0, "iso value=%d(%s)", value, str.string());
        return  value;
    }
_PARAM_MAP_END_(eMapIso)


/*******************************************************************************
*   antibanding
*******************************************************************************/
_PARAM_MAP_BEG_(eMapAntiBanding)
    _DEFAULT_STRING_VALUE_(CameraParameters::ANTIBANDING_OFF,       AE_FLICKER_MODE_OFF) // Default
    {
    _ADD_STRING_VALUE_MAP_(CameraParameters::ANTIBANDING_OFF,       AE_FLICKER_MODE_OFF);
    _ADD_STRING_VALUE_MAP_(CameraParameters::ANTIBANDING_AUTO,      AE_FLICKER_MODE_AUTO);
    _ADD_STRING_VALUE_MAP_(CameraParameters::ANTIBANDING_50HZ,      AE_FLICKER_MODE_50HZ);
    _ADD_STRING_VALUE_MAP_(CameraParameters::ANTIBANDING_60HZ,      AE_FLICKER_MODE_60HZ);
    }
_PARAM_MAP_END_(eMapAntiBanding)


/*******************************************************************************
*   flash
*******************************************************************************/
_PARAM_MAP_BEG_(eMapFlashMode)
    _DEFAULT_STRING_VALUE_(CameraParameters::FLASH_MODE_OFF,        FLASHLIGHT_FORCE_OFF) // Default
    {
    _ADD_STRING_VALUE_MAP_(CameraParameters::FLASH_MODE_OFF,        FLASHLIGHT_FORCE_OFF);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FLASH_MODE_AUTO,       FLASHLIGHT_AUTO);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FLASH_MODE_ON,         FLASHLIGHT_FORCE_ON);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FLASH_MODE_RED_EYE,    FLASHLIGHT_REDEYE);
    _ADD_STRING_VALUE_MAP_(CameraParameters::FLASH_MODE_TORCH,      FLASHLIGHT_TORCH);
    }
_PARAM_MAP_END_(eMapFlashMode)


/*******************************************************************************
*   Shot Mode
*******************************************************************************/
_PARAM_MAP_BEG_(eMapShotMode)
    _DEFAULT_STRING_VALUE_( MtkCameraParameters::CAPTURE_MODE_NORMAL,               0) // Default
    {
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_NORMAL,               CAPTURE_MODE_NORMAL,            eShotMode_NormalShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT,      CAPTURE_MODE_CONTINUOUS_SHOT,   eShotMode_ContinuousShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_BEST_SHOT,            CAPTURE_MODE_BEST_SHOT,         eShotMode_BestShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_EV_BRACKET_SHOT,      CAPTURE_MODE_EV_BRACKET,        eShotMode_EvShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT,           CAPTURE_MODE_SMILE_SHOT,        eShotMode_SmileShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_HDR_SHOT,             CAPTURE_MODE_HDR,               eShotMode_HdrShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_ASD_SHOT,             CAPTURE_MODE_ASD,               eShotMode_AsdShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_ZSD_SHOT,             CAPTURE_MODE_ZSD,               eShotMode_ZsdShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY,          CAPTURE_MODE_FACE_BEAUTY,       eShotMode_FaceBeautyShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_MAV_SHOT,             CAPTURE_MODE_MAV,               eShotMode_Mav);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT,   CAPTURE_MODE_AUTORAMA,          eShotMode_Autorama);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_MULTI_MOTION,         CAPTURE_MODE_MULTI_MOTION,      eShotMode_MultiMotionShot);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_SINGLE_3D,            CAPTURE_MODE_SINGLE_3D,         eShotMode_Single3D);
    _ADD_STRING_VALUE_MAP2_(MtkCameraParameters::CAPTURE_MODE_PANO_3D,              CAPTURE_MODE_PANO_3D,           eShotMode_Panorama3D);
    _ADD_STRING_VALUE_MAP2_("testshot",                                             0x10000000,                     0x10000000);
    }
_PARAM_MAP_END_(eMapShotMode)


/*******************************************************************************
*   exposure mode
*******************************************************************************/
_PARAM_MAP_BEG_(eMapMeterMode)
    _DEFAULT_STRING_VALUE_(   MtkCameraParameters::KEY_EXPOSURE_METER,            AE_METERING_MODE_CENTER_WEIGHT) // Default
    {
    _ADD_STRING_VALUE_MAP_(   MtkCameraParameters::EXPOSURE_METER_SPOT,             AE_METERING_MODE_SOPT);
    _ADD_STRING_VALUE_MAP_(   MtkCameraParameters::EXPOSURE_METER_CENTER,           AE_METERING_MODE_AVERAGE);
    _ADD_STRING_VALUE_MAP_(   MtkCameraParameters::EXPOSURE_METER_AVERAGE,          AE_METERING_MODE_CENTER_WEIGHT);
    }
_PARAM_MAP_END_(eMapMeterMode)


////////////////////////////////////////////////////////////////////////////////////////////////////
}; // namespace android
#endif  //_MTK_CAMERA_PARAMSMGR_MAP_H_

