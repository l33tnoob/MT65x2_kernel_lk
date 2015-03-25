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
#include <cutils/properties.h>
//


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
bool
ParamsManager::
updateParams(int32_t const i4FKeyIndex)
{
    using namespace NSCameraFeature;
    //
    String8 const& rs8SceneMode = mpFeatureKeyedMap->getCurrentSceneMode();
    SceneKeyedMap const& rSceneKeyedMap = mpFeatureKeyedMap->valueAt(i4FKeyIndex);
    FeatureInfo const& rFInfo   = rSceneKeyedMap.valueFor(rs8SceneMode);
    //
    String8 const s8KeyName     = mpFeatureKeyedMap->keyAt(i4FKeyIndex);                //ex: "scene-mode"
    //
    if  ( rSceneKeyedMap.getType() == "default-values" )
    {
        String8 const s8ItemDefault     = rFInfo.getDefaultItem();                      //ex: "auto"
        mParameters.set(s8KeyName.string(), s8ItemDefault.string());                    //ex: "scene-mode=auto"
        //
        String8 const s8ItemList        = rFInfo.getSupportedList();                    //ex: "auto,normal,portrait"
        String8 const s8ItemListKeyName = ParamsManager::getValuesKeyName(s8KeyName);   //ex: "scene-mode-values"
        mParameters.set(s8ItemListKeyName.string(), s8ItemList.string());               //ex: "scene-mode-values=auto,normal,portrait"
        MY_LOGD_IF(1, "[%02d] %s=%s;%s=%s", i4FKeyIndex, s8KeyName.string(), s8ItemDefault.string(), s8ItemListKeyName.string(), s8ItemList.string());
        return  true;
    }
    //
    if  ( rSceneKeyedMap.getType() == "default-supported" )
    {
        String8 const s8ItemDefault     = rFInfo.getDefaultItem();                      //ex: "auto"
        mParameters.set(s8KeyName.string(), s8ItemDefault.string());                    //ex: "video-stabilization=false"
        //
        String8 const s8ItemList        = rFInfo.getSupportedList();                    //ex: "false"
        String8 const s8ItemListKeyName = ParamsManager::getSupportedKeyName(s8KeyName);//ex: "video-stabilization-supported"
        mParameters.set(s8ItemListKeyName.string(), s8ItemList.string());               //ex: "video-stabilization-supported=false"
        MY_LOGD_IF(1, "[%02d] %s=%s;%s=%s", i4FKeyIndex, s8KeyName.string(), s8ItemDefault.string(), s8ItemListKeyName.string(), s8ItemList.string());
        return  true;
    }
    //
    return  updateUserTypeParams(s8KeyName, rFInfo);
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateUserTypeParams(String8 const& s8KeyName, NSCameraFeature::FeatureInfo const& rFeatureInfo)
{
    if  ( s8KeyName == CameraParameters::KEY_ZOOM )
    {
        return  updateZoomParams(rFeatureInfo);
    }
    //
    if  ( s8KeyName == CameraParameters::KEY_PREVIEW_FPS_RANGE )
    {
        return  updatePreviewFpsParams(rFeatureInfo);
    }
    //
    if  ( s8KeyName == CameraParameters::KEY_EXPOSURE_COMPENSATION )
    {
        return  updateExposureCompensationParams(rFeatureInfo);
    }
    //
    if  ( s8KeyName == CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED )
    {
        return  updateVideoSnapshotParams(rFeatureInfo);
    }
    //
    return  false;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateZoomParams(NSCameraFeature::FeatureInfo const& rFeatureInfo)
{
    String8 const&  rs8ZoomIndex = rFeatureInfo.getDefaultItem();
    String8 const   s8ZoomRatios = rFeatureInfo.getSupportedList();
    int32_t const   i4MaxZoom = rFeatureInfo.getSupportedSize() - 1; // KEY_MAX_ZOOM can be zero 
    //
    mvZoomRatios.clear();
    for (size_t i = 0; i < rFeatureInfo.getSupportedSize(); i++)
    {
        mvZoomRatios.push_back(::atoi(rFeatureInfo.getSupportedItem(i).string()));
    }
    //
    MY_LOGD_IF(1, " %s=%s", CameraParameters::KEY_ZOOM_RATIOS, s8ZoomRatios.string());
    //
    mParameters.set(CameraParameters::KEY_ZOOM_RATIOS, s8ZoomRatios.string());
    mParameters.set(CameraParameters::KEY_MAX_ZOOM, i4MaxZoom);
    mParameters.set(CameraParameters::KEY_ZOOM, rs8ZoomIndex.string());
    mParameters.set(CameraParameters::KEY_ZOOM_SUPPORTED, CameraParameters::TRUE);
    mParameters.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, CameraParameters::TRUE);
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updatePreviewFpsParams(NSCameraFeature::FeatureInfo const& rFeatureInfo)
{
    String8 const s8FpsRangeDefault = rFeatureInfo.getDefaultItem();    //  "10000,26623"
    String8 const s8FpsRangeList    = rFeatureInfo.getSupportedList();  //  "(10000,26623),(10000,30000)"
    //
    MY_LOGD_IF(1, "%s=%s;%s=%s", 
        CameraParameters::KEY_PREVIEW_FPS_RANGE, s8FpsRangeDefault.string(), 
        CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, s8FpsRangeList.string()
    );
    //
    //  CameraParameters::KEY_PREVIEW_FRAME_RATE
    Vector<int> itemDefault;
    String8 s8FrameRateDefault;
    if  ( ! splitInt(s8FpsRangeDefault, itemDefault) )
    {
        MY_LOGE("splitInt:%s", s8FpsRangeDefault.string());
        return  false;
    }
    else
    {
        s8FrameRateDefault = String8::format("%d", itemDefault[1]/1000);
    }
    //
    //  CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES
    List< Vector<int> > itemList;
    String8 s8FrameRateList;
    if  ( ! splitRange(s8FpsRangeList, itemList) )
    {
        MY_LOGE("splitRange:%s", s8FpsRangeList.string());
        return  false;
    }
    else
    {
        List< Vector<int> >::iterator it = itemList.begin();
        s8FrameRateList = String8::format("%d", (*it)[1]/1000);
        for (it++; it != itemList.end(); it++)
        {
            s8FrameRateList += String8::format(",%d", (*it)[1]/1000);
        }
    }
    //
    //  Preview frame rate
    mParameters.set(CameraParameters::KEY_PREVIEW_FRAME_RATE,            s8FrameRateDefault.string());
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, s8FrameRateList.string());
    //
    //  Preview fps range
    //  TODO:
    mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,60000");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,60000)");
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateExposureCompensationParams(NSCameraFeature::FeatureInfo const& rFeatureInfo)
{
    String8 const& rs8ExpComp    = rFeatureInfo.getDefaultItem();
    String8 const& rs8MinExpComp = rFeatureInfo.getSupportedItem(0);
    String8 const& rs8MaxExpComp = rFeatureInfo.getSupportedItem(1);
    String8 const& rs8ExpCompStep= rFeatureInfo.getSupportedItem(2);
    //
    MY_LOGD_IF(1, "%s=%s;%s=%s;%s=%s;%s=%s", 
        CameraParameters::KEY_EXPOSURE_COMPENSATION,        rs8ExpComp.string(), 
        CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION,    rs8MinExpComp.string(), 
        CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION,    rs8MaxExpComp.string(), 
        CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP,   rs8ExpCompStep.string()
    );
    //
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION,        rs8ExpComp.string());
    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION,    rs8MinExpComp.string());
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION,    rs8MaxExpComp.string());
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP,   rs8ExpCompStep.string());
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateVideoSnapshotParams(NSCameraFeature::FeatureInfo const& rFeatureInfo)
{
    String8 const& rs8Supported = rFeatureInfo.getDefaultItem();
    //
    MY_LOGD_IF(1, "%s=%s", CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, rs8Supported.string());
    //
    mParameters.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, rs8Supported.string());
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams()
{
    bool ret = true;
    //
    MY_LOGD("+");
    //
    RWLock::AutoWLock _lock(mRWLock);
    //
    //  (1) cleanup mParameters.
    mParameters = MtkCameraParameters();
    //
    ret =   updateDefaultParams0()
        &&  updateDefaultParams1()
        &&  updateDefaultParams2()
            ;
    //
    //  show for debug.
    IParamsManager::showParameters(mParameters.flatten());
    //
    MY_LOGD("- ret(%d)", ret);
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams1()
{
    MY_LOGD("+");
    //
    if  (
            ! updateDefaultParams1_ByQuery()
        &&  ! updateDefaultParams1_ByDefault()  //  Fail to query => update by default
        )
    {
        //  both fail to update => return false.
        return  false;
    }
    //
    //  Prefferd Preview Size for Video
    if ( ! updatePreferredPreviewSize() )
    {
        return  false;
    }
    //
    updateDefaultFaceCapacity();
    updateDefaultVideoFormat();
    //
    //  For test.
    //mParameters.set("preview-size-values", "176x144,320x240,352x288,480x320,480x368,640x480,720x480,864x480,960x540"); 
    //mParameters.set("preview-size-values", "320x240,352x288,704x576,640x480,720x480,960x720,960x540");
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updatePreferredPreviewSize()
{
    char psize[32];
    Vector<Size> prvSizes, vdoSizes;
    //
    mParameters.getSupportedPreviewSizes(prvSizes);
    mParameters.getSupportedVideoSizes(vdoSizes);

    // KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO can be used only when
    // getSupportedVideoSizes() does not return an empty Vector of Size.
    if ( ! vdoSizes.isEmpty() )
    {
        for ( Vector<Size>::iterator vdoIt = vdoSizes.end()-1; vdoIt != vdoSizes.begin()-1; vdoIt-- )
        {
            for ( Vector<Size>::iterator prvIt = prvSizes.end()-1; prvIt != prvSizes.begin()-1; prvIt-- )
            {
                if ( vdoIt->width == prvIt->width && vdoIt->height == prvIt->height )
                {
                    snprintf(psize, sizeof(psize),"%dx%d", vdoIt->width, prvIt->height);
                    mParameters.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, psize);
                    MY_LOGD("KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO=%s", psize);
                    return true;
                }
            }
        }
        MY_LOGE("updatePreferredPreviewSize() fail");
        return false;
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams1_ByDefault()
{
    MY_LOGD("+");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Android parameters
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mParameters.set(CameraParameters::KEY_PREVIEW_SIZE, "320x240");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "320x240,640x480,1920x1080");
    //
    mParameters.set(CameraParameters::KEY_PREVIEW_FRAME_RATE, "30");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "30");
    mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,30000)");
    //
    mParameters.set(CameraParameters::KEY_PICTURE_SIZE, "2560x1920");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, "2560x1920");
    //
    mParameters.set(CameraParameters::KEY_VIDEO_SIZE, "640x480");
    mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, "640x480");
    //
    mParameters.set(CameraParameters::KEY_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);
    mParameters.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);
    //
    mParameters.set(CameraParameters::KEY_EFFECT, CameraParameters::EFFECT_NONE);
    mParameters.set(CameraParameters::KEY_SUPPORTED_EFFECTS, CameraParameters::EFFECT_NONE);
    //
    mParameters.set(CameraParameters::KEY_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
    mParameters.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
    //
    mParameters.set(CameraParameters::KEY_SCENE_MODE, CameraParameters::SCENE_MODE_AUTO);
    mParameters.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES, CameraParameters::SCENE_MODE_AUTO);
    //
    mParameters.set(CameraParameters::KEY_FLASH_MODE, CameraParameters::FLASH_MODE_OFF);
    mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, CameraParameters::FLASH_MODE_OFF);
    //
    mParameters.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_INFINITY);
    mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, CameraParameters::FOCUS_MODE_INFINITY);
    //
    mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, CameraParameters::FALSE);
    //
    mParameters.set(CameraParameters::KEY_ZOOM_RATIOS, "100,114,132,151,174,200,229,263,303,348,400");
    uint32_t const zoomRatios[] = {100, 114, 132, 151, 174, 200, 229, 263, 303, 348, 400};
    mvZoomRatios.clear();
    for (size_t i = 0; i < 11; i++) {
        mvZoomRatios.push_back(zoomRatios[i]);
    }
    mParameters.set(CameraParameters::KEY_MAX_ZOOM, mvZoomRatios.size()-1);
    mParameters.set(CameraParameters::KEY_ZOOM, 0);
    mParameters.set(CameraParameters::KEY_ZOOM_SUPPORTED, CameraParameters::TRUE);
    mParameters.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, CameraParameters::TRUE);
    //  Exp Comp
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, 0);
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, 0);
    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, 0);
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0.0");
    //
    mParameters.set(CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, CameraParameters::FALSE);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  MTK proprietary parameters
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  focus lamp
    mParameters.set(MtkCameraParameters::KEY_AF_LAMP_MODE, MtkCameraParameters::OFF);         
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_AF_LAMP_MODE), MtkCameraParameters::OFF);
    //  iso-speed 
    mParameters.set(MtkCameraParameters::KEY_ISO_SPEED, MtkCameraParameters::ISO_SPEED_AUTO);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_ISO_SPEED), MtkCameraParameters::ISO_SPEED_AUTO);
    //  ZSD-mode 
    mParameters.set(MtkCameraParameters::KEY_ZSD_MODE, MtkCameraParameters::OFF);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_ZSD_MODE), MtkCameraParameters::OFF);
    //  Capture Mode
    mParameters.set(MtkCameraParameters::KEY_CAPTURE_MODE, MtkCameraParameters::CAPTURE_MODE_NORMAL);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_CAPTURE_MODE), MtkCameraParameters::CAPTURE_MODE_NORMAL);
    //  Hue
    mParameters.set(MtkCameraParameters::KEY_HUE, MtkCameraParameters::MIDDLE);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_HUE), MtkCameraParameters::MIDDLE);
    //  Edge
    mParameters.set(MtkCameraParameters::KEY_EDGE, MtkCameraParameters::MIDDLE);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_EDGE), MtkCameraParameters::MIDDLE);
    //  Saturation
    mParameters.set(MtkCameraParameters::KEY_SATURATION, MtkCameraParameters::MIDDLE);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_SATURATION), MtkCameraParameters::MIDDLE);
    //  Brightness
    mParameters.set(MtkCameraParameters::KEY_BRIGHTNESS, MtkCameraParameters::MIDDLE);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_BRIGHTNESS), MtkCameraParameters::MIDDLE);
    //  Constrast
    mParameters.set(MtkCameraParameters::KEY_CONTRAST, MtkCameraParameters::MIDDLE);
    mParameters.set(getValuesKeyName(MtkCameraParameters::KEY_CONTRAST), MtkCameraParameters::MIDDLE);
    //
    
        
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams2()
{
    if  (
            ! updateDefaultParams2_ByQuery()
        &&  ! updateDefaultParams2_ByDefault()  //  Fail to query => update by default
        )
    {
        //  both fail to update => return false.
        return  false;
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams2_ByDefault()
{
    MY_LOGD("+");
    //
    //  AE/AWB Lock
    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, CameraParameters::FALSE);
    mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, CameraParameters::FALSE);
    //  AE/AF areas
    mParameters.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, 1);
    mParameters.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS, 1);
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultParams0()
{
    MY_LOGD("+");

    //  Previwe Format
    updateDefaultPreviewFormat();

    // Picture related
    mParameters.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS, CameraParameters::PIXEL_FORMAT_JPEG);

    //  JPEG related
    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, 100);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, 100);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, 160);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, 128);
    mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, "0x0,160x128,320x240");

    //  3A related
    mParameters.set(CameraParameters::KEY_FOCAL_LENGTH, "3.5"); // Should query from driver
    mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES, "0.95,1.9,Infinity");

    // Sensor related
    mParameters.set(CameraParameters::KEY_ROTATION, 0);
    mParameters.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, 60);
    mParameters.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, 60);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  MTK proprietary parameters
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mParameters.set(MtkCameraParameters::KEY_CAMERA_MODE, MtkCameraParameters::CAMERA_MODE_NORMAL);
//    mParameters.set(MtkCameraParameters::KEY_ISP_MODE, 0);     //0: normal
    //
    mParameters.set(MtkCameraParameters::KEY_CAPTURE_PATH, "/sdcard/DCIM/cap00");
    mParameters.set(MtkCameraParameters::KEY_BURST_SHOT_NUM, 1);

    mParameters.set(MtkCameraParameters::KEY_BRIGHTNESS_VALUE, 0);

    //  face beauty tuning (smooth level).
    mParameters.set(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL, 0);
    mParameters.set(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MIN, -4);
    mParameters.set(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MAX, 4);
    //  face beauty tuning (skin color).
    mParameters.set(MtkCameraParameters::KEY_FB_SKIN_COLOR, 0);
    mParameters.set(MtkCameraParameters::KEY_FB_SKIN_COLOR_MIN, -4);
    mParameters.set(MtkCameraParameters::KEY_FB_SKIN_COLOR_MAX, 4);
    //  face beauty tuning (sharp).
    mParameters.set(MtkCameraParameters::KEY_FB_SHARP, 0);
    mParameters.set(MtkCameraParameters::KEY_FB_SHARP_MIN, -4);
    mParameters.set(MtkCameraParameters::KEY_FB_SHARP_MAX, 4);

    //----------------------------------------------------------------------------------------------
    //  MTK proprietary parameters
    //----------------------------------------------------------------------------------------------

    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateDefaultPreviewFormat()
{
    String8 supportedPreviewFormats = String8(CameraParameters::PIXEL_FORMAT_YUV420SP)
                                    + ","
                                    + String8(CameraParameters::PIXEL_FORMAT_YUV420P)
                                    + ","
                                    + String8(MtkCameraParameters::PIXEL_FORMAT_YUV420I)
                                    ;
    //
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS, supportedPreviewFormats);
    //  Default preview format: NV21 (Yuv420sp)
    mParameters.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateSceneAndParams()
{
    return  
            //  (1) Set Scene Mode.
            updateScene()
            //  (2) Update Scene-Dependent Parameters.
        &&  updateSceneDependentParams()
            ;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateScene()
{
    //  update scene mode.
    if  ( mpFeatureKeyedMap ) {
        using namespace NSCameraFeature;
        const_cast<FeatureKeyedMap*>(mpFeatureKeyedMap)->setCurrentSceneMode(String8(mParameters.get(CameraParameters::KEY_SCENE_MODE)));
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updateSceneDependentParams()
{
    //  Update Scene-dependent parameters to mParameters.
    MY_LOGD("+");
    //
    //
    if  ( ! mpFeatureKeyedMap ) {
        return  true;
    }
    for (size_t fkey = 0; fkey < mpFeatureKeyedMap->size(); fkey++)
    {
        //  number of scene modes > 0 ==> scene-dependent.
        if  ( 0 < mpFeatureKeyedMap->valueAt(fkey).size() )
        {
            updateParams(fkey);
        }
    }
    //
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
updatePreviewSize()
{
    //  Update preview size to mParameters.
    MY_LOGD("+");
    //
    //
    Vector<Size> prvSizes;
    Size oriPrvSize, prvSize, candPrvSize;
    int prvSizeDiff = 0;
    int diffRatio = 0, diffSize = 0;
    int candRatio = 0, candSize = 0;
    //
    mParameters.getPreviewSize(&oriPrvSize.width, &oriPrvSize.height);
    mParameters.getSupportedPreviewSizes(prvSizes);
    //
    /**************************************************************************
    *  In normal case, preview width is bigger than preview height. (prvSizeDiff [w-h] is bigger than 0)
    *  If preview width <= preview height, (prvSizeDiff <= 0),   switch preview w/h.
    **************************************************************************/
    prvSizeDiff     = oriPrvSize.width - oriPrvSize.height;
    prvSize.width   = (prvSizeDiff > 0) ? oriPrvSize.width : oriPrvSize.height;
    prvSize.height  = (prvSizeDiff > 0) ? oriPrvSize.height : oriPrvSize.width;
    //
    /**************************************************************************
    *  src :  original preview size  (w, h)
    *  old :  candidate preview size  (wc, hc)
    *  new: for each preview size in supported list  (w', h')
    *  
    *  | w/h - wc/hc | - | w/h - w'/h' | => | w*hc*h' - wc*h*h' | - | w*hc*h' - w'*h*hc |
    *  
    **************************************************************************/
    candPrvSize = prvSizes[0];  // default candidate preview size
    candRatio   = abs(prvSize.width*candPrvSize.height*prvSizes[0].height - prvSize.height*prvSizes[0].height*candPrvSize.width); // default diff of ratio
    candSize    = abs(prvSize.width*prvSize.height - prvSizes[0].width*prvSizes[0].height); // default diff of size
    //
    for ( unsigned int idx = 0; idx < prvSizes.size(); ++idx )
    {
        diffRatio   = abs(prvSize.width*candPrvSize.height*prvSizes[idx].height - prvSize.height*candPrvSize.height*prvSizes[idx].width);
        candRatio   = abs(prvSize.width*candPrvSize.height*prvSizes[idx].height - prvSize.height*prvSizes[idx].height*candPrvSize.width);
        diffSize    = abs(prvSize.width*prvSize.height - prvSizes[idx].width*prvSizes[idx].height);
        //
        if ( 0 == diffRatio && 0 == diffSize)
        {
            // prvSize is in supported preview size list.
            goto lbExit;
        }
        //
        if (diffRatio < candRatio)
        {
            candSize    = diffSize;
            candPrvSize = prvSizes[idx];
        } 
        else if ( diffRatio == candRatio && diffSize < candSize)
        {
            candSize    = diffSize;
            candPrvSize = prvSizes[idx];
        }
    }
    /**************************************************************************
    *  If preview size does not in supported preview size list, choose the best preview size
    *  in supported preview size list. Check if original preview width is bigger than height,
    *   (prvSizeDiff > 0)  if not, switch preview w/h back.
    **************************************************************************/
    if ( prvSizeDiff > 0 )
    {
        MY_LOGW("new prvSize(%dx%d)", candPrvSize.width, candPrvSize.height);
        mParameters.setPreviewSize(candPrvSize.width, candPrvSize.height);
    }
    else
    {
        MY_LOGW("new prvSize(%dx%d)", candPrvSize.height, candPrvSize.width);
        mParameters.setPreviewSize(candPrvSize.height, candPrvSize.width);
    }
    //
lbExit:
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
int32_t
ParamsManager::
getHalAppMode() const
{
    RWLock::AutoRLock _lock(mRWLock);
    return mi4HalAppMode;
}


/******************************************************************************
*
*******************************************************************************/
#define PROPERTY_KEY_HAL_APPMODE    "hal.appmode"
bool
ParamsManager::
updateHalAppMode()
{
    bool ret = false;
    String8 s8HalAppMode;
    //
    RWLock::AutoWLock _lock(mRWLock);
    //
    int32_t const i4CamMode = mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    MY_LOGD("+ KEY_CAMERA_MODE:%d", i4CamMode);
    //
    //
    //  If Client App Mode != <Default> App, then.
    //
    if  ( ms8ClientAppMode != MtkCameraParameters::APP_MODE_NAME_DEFAULT )
    {
        s8HalAppMode = ms8ClientAppMode;
        goto lbExit;
    }
    //
    //  If Client App Mode == <Default> App, then:
    //
    if  ( i4CamMode == MtkCameraParameters::CAMERA_MODE_NORMAL )
    {
        s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_DEFAULT;
        goto lbExit;
    }
    //
    if  ( i4CamMode == MtkCameraParameters::CAMERA_MODE_MTK_PRV )
    {
        char const* pszZsdMode = mParameters.get(MtkCameraParameters::KEY_ZSD_MODE);
        if  ( ! pszZsdMode || 0 != ::strcmp(pszZsdMode, MtkCameraParameters::ON) )
        {
            s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO;
        }
        else
        {
            s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_ZSD;
        }
        // zsd: only for test
        {
            #if defined (MTK_ZSD_SUPPORT)
            #warning "[FIXME] MTK_ZSD_SUPPORT"
            MY_LOGD("MTK_ZSD_SUPPORT=yes This MACRO is for test ONLY");
            SensorHal* pSensorHal = NULL;
            halSensorType_e eSensorType;
            halSensorDev_e eSensorDev = (halSensorDev_e)DevMetaInfo::queryHalSensorDev(getOpenId());
            pSensorHal = SensorHal::createInstance();
            if (pSensorHal == NULL) {
                MY_LOGE("pSensorHal == NULL");
                goto lbExit;
            }
            pSensorHal->sendCommand(eSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (int32_t)&eSensorType);
            MY_LOGD("get sensor type, %d", eSensorType);
            if (eSensorType == SENSOR_TYPE_RAW) {
                s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_ZSD;
            }
            pSensorHal->destroyInstance();
            pSensorHal = NULL;
            #endif

            int32_t i4ZsdMode = 0;
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.zsdmode", value, "0");
            i4ZsdMode = atoi(value);
            MY_LOGD("zsd mode %d", i4ZsdMode);
            if ( 1 == i4ZsdMode || 2 == i4ZsdMode ) {
                s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_ZSD;
            } else if ( 3 == i4ZsdMode ) {
                s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO;
            }
        }
        goto lbExit;
    }
    if ( i4CamMode == MtkCameraParameters::CAMERA_MODE_MTK_VT )
    {
        s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_VT;
        goto lbExit;
    }
    //
    if  ( i4CamMode == MtkCameraParameters::CAMERA_MODE_MTK_VDO )
    {
        s8HalAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO;
        goto lbExit;
    }
    //
    //
    MY_LOGE("- NOT IMPLEMENT YET !");
    return false;
    //
lbExit:
    //  (3) update Hal's App Mode.
    //  (.1) update Hal's property.
    MtkCamUtils::Property::set(String8(PROPERTY_KEY_HAL_APPMODE), s8HalAppMode);
    //
    //  (.2) set Hal's App Mode.
    mi4HalAppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->valueFor(s8HalAppMode);
    //
    MY_LOGD("- property:%s=%s[%d]", PROPERTY_KEY_HAL_APPMODE, s8HalAppMode.string(), mi4HalAppMode);
    return true;
}

