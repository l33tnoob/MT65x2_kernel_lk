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
#include <utils/String16.h>


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
ParamsManager::
ParamsManager(String8 const& rName, int32_t const i4OpenId)
    : IParamsManager()
    //
    , mRWLock()
    //
    , mpFeatureKeyedMap(NULL)
    //
    , mParameters()
    , mName(rName)
    , mi4OpenId(i4OpenId)
    //
    , ms8ClientAppMode(rName)
    , mi4HalAppMode(0)
    //
{
}


/******************************************************************************
*
*******************************************************************************/
ParamsManager::
~ParamsManager()
{
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
init()
{
    return  updateDefaultParams()
        &&  updateHalAppMode()
            ;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
uninit()
{
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getValuesKeyName(char const aKeyName[])
{
    return  getValuesKeyName(String8(aKeyName));
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getSupportedKeyName(String8 const& s8KeyName)
{
    return  s8KeyName + "-supported";
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getValuesKeyName(String8 const& s8KeyName)
{
    return  s8KeyName + "-values";
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
splitInt(String8 const& s8Input, Vector<int>& rOutput)
{
    rOutput.clear();
    //
    if  ( s8Input.isEmpty() )
    {
        MY_LOGW("empty string");
        return  false;
    }
    //
    char const*start = s8Input.string();
    char *end = NULL;
    do {
        int value = ::strtol(start, &end, 10);
        if  ( start == end ) {
            MY_LOGW_IF(0, "no digits in str:%s", s8Input.string());
            return  false;
        }
        rOutput.push_back(value);
        MY_LOGD_IF(0, "%d", value);
        start = end + 1;
    } while ( end && *end );
    //
    return  (rOutput.size() > 0);
}


/******************************************************************************
*   Splits a comma delimited string to a List of int Vector. 
*   Example string: "(10000,26623),(10000,30000)"
*******************************************************************************/
bool
ParamsManager::
splitRange(String8 const& s8Input, List< Vector<int> >& rOutput)
{
    rOutput.clear();
    //
    if  ( s8Input.isEmpty() )
    {
        MY_LOGW("empty string");
        return  false;
    }
    //
    int endIndex, fromIndex = 1;
    int endIndex_input = s8Input.length()-1;
    if  ( s8Input[0] != '(' || s8Input[endIndex_input] != ')' )
    {
        MY_LOGW("Invalid range list string=%s", s8Input.string());
        return  false;
    }
    //
    do  {
        endIndex = s8Input.find("),(", fromIndex);
        if (endIndex == -1) endIndex = endIndex_input;
        //
        Vector<int> vOut;
        String8 const s8SubString(s8Input.string()+fromIndex, endIndex-fromIndex);
        if  ( splitInt(s8SubString, vOut) ) {
            rOutput.push_back(vOut);
        }
        //
        fromIndex = endIndex + 3;
    } while (endIndex != endIndex_input);
    //
    return  (rOutput.size() > 0);
}


/******************************************************************************
*
*******************************************************************************/
void
IParamsManager::
showParameters(String8 const& rs8Param)
{
    static char bufParam[4096];

    const int count = 768;
    char *pbuf = bufParam;
    char ch;
    uint32_t len;
    uint32_t i, loop;

    pbuf[0] = 0;
    len = rs8Param.length();
    if ( len > 4096) {
        len = 4096;
    }

    ::strncpy(pbuf, rs8Param.string(), len);
    loop = (len + count - 1) / count;

    MY_LOGD(" %d, %d", loop, len);
    for (i = 0; i < loop; i++) {
        ch = pbuf[count];
        pbuf[count] = 0;
        CAM_LOGD("%s", pbuf);
        pbuf[count] = ch;
        pbuf += count;
    }
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
showParameters() const
{
    IParamsManager::showParameters(flatten());
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
setParameters(CameraParameters const& params)
{
    IParamsManager::showParameters(params.flatten());
    //
    status_t    status = OK;
    bool        bIsSceneChanged = false;
    Size        oldPrvSize, tmpPrvSize, newPrvSize;
    //
    //  (1) Check to see if new parameters are valid or not.
    if  ( OK != (status = checkParams(params)) )
    {
        MY_LOGE("checkParams() return %d", status);
        return  status;
    }

    //  (2) Check to see if Scene Changes or not.
    MY_LOGD("+");
    RWLock::AutoWLock _lock(mRWLock);
    MY_LOGD("AutoWLock");
    //
    char const*const pOldScene = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    char const*const pNewScene =      params.get(CameraParameters::KEY_SCENE_MODE);
    if  ( 0 != ::strcmp(pOldScene, pNewScene) )
    {
        //  scene mode has changed
        MY_LOGD("Scene change: %s -> %s", pOldScene, pNewScene);
        bIsSceneChanged = true;
    }
    //
    //  (3.1) Check to see if Preview Size Changes or not. Get old preview size
    mParameters.getPreviewSize(&oldPrvSize.width, &oldPrvSize.height);
         params.getPreviewSize(&tmpPrvSize.width, &tmpPrvSize.height);

    //  (4) Update Parameter
    mParameters = params;
    if  ( bIsSceneChanged ) {
        if  ( ! updateSceneAndParams() ) {
            status = BAD_VALUE;
        }
    }

    //  (5) Update Parameter: preview size
    updatePreviewSize();
    //
    //  (3.2) Check to see if Preview Size Changes or not. Get new preview size
    mParameters.getPreviewSize(&newPrvSize.width, &newPrvSize.height);
    if  ( oldPrvSize.width != newPrvSize.width || oldPrvSize.height != newPrvSize.height )
    {
        MY_LOGD(
            "Preview Size change: %dx%d/%dx%d -> (%dx%d)", 
            oldPrvSize.width, oldPrvSize.height, 
            tmpPrvSize.width, tmpPrvSize.height,
            newPrvSize.width, newPrvSize.height
        );
    }
    //
    MY_LOGD("-");
    return status;
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
flatten() const
{
    RWLock::AutoRLock _lock(mRWLock);
    return mParameters.flatten();
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getPreviewFormat() const
{
    return  getStr(CameraParameters::KEY_PREVIEW_FORMAT);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getPreviewSize(int *width, int *height) const
{
    RWLock::AutoRLock _lock(mRWLock);
    mParameters.getPreviewSize(width, height);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getVideoSize(int *width, int *height) const
{
    RWLock::AutoRLock _lock(mRWLock);
    mParameters.getVideoSize(width, height);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
getPictureSize(int *width, int *height) const
{
    RWLock::AutoRLock _lock(mRWLock);
    mParameters.getPictureSize(width, height);
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getZoomRatio() const
{
    RWLock::AutoRLock _lock(mRWLock);
    return  mvZoomRatios[mParameters.getInt(CameraParameters::KEY_ZOOM)];
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getZoomRatioByIndex(uint32_t index) const
{
    RWLock::AutoRLock _lock(mRWLock);
    return  mvZoomRatios[index];
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getRecordingHint() const
{
    RWLock::AutoRLock _lock(mRWLock);
    char const* p = mParameters.get(CameraParameters::KEY_RECORDING_HINT);
    //
    if ( ! p ) {
        MY_LOGD("No KEY_RECORDING_HINT");
        return false;
    }
    //
    return ( ::strcmp(p, CameraParameters::TRUE) != 0 ) ? false : true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
getVideoStabilization() const
{
    RWLock::AutoRLock _lock(mRWLock);
    char const* p = mParameters.get(CameraParameters::KEY_VIDEO_STABILIZATION);
    //
    if ( ! p ) {
        MY_LOGD("No KEY_VIDEO_STABILIZATION");
        return false;
    }
    //
    return ( ::strcmp(p, CameraParameters::TRUE) != 0 ) ? false : true;
}

/******************************************************************************
*
*******************************************************************************/
bool ParamsManager::getCShotIndicator() const
{
    RWLock::AutoRLock _lock(mRWLock);
    char const *p = mParameters.get(MtkCameraParameters::KEY_CSHOT_INDICATOR);
    
    if(!p)
    {
        MY_LOGD("No KEY_CSHOT_INDICATOR");
        return false;
    }
    
    return (::strcmp(p, CameraParameters::TRUE) != 0 ) ? false : true;
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getShotModeStr() const
{
    String8 s8ShotMode;
    RWLock::AutoRLock _lock(mRWLock);
    //
    char const*const pScene = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    if  ( pScene && 0 == ::strcmp(pScene, CameraParameters::SCENE_MODE_HDR) )
    {
        s8ShotMode = MtkCameraParameters::CAPTURE_MODE_HDR_SHOT;
        MY_LOGD_IF(1, "%s - scene(hdr)", s8ShotMode.string());
    }
    else
    if  ( char const* p = mParameters.get(MtkCameraParameters::KEY_CAPTURE_MODE) )
    {
        s8ShotMode = p;
        MY_LOGD_IF(0, "%s", s8ShotMode.string());
    }
    else
    {
        s8ShotMode = MtkCameraParameters::CAPTURE_MODE_NORMAL;
        MY_LOGD_IF(1, "%s by default - no shot mode", s8ShotMode.string());
    }
    //
    return  s8ShotMode;
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
ParamsManager::
getShotMode() const
{
    return  PARAMSMANAGER_MAP_INST(eMapShotMode)->valueFor(getShotModeStr());
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateBrightnessValue(int const iBV)
{
    set(MtkCameraParameters::KEY_BRIGHTNESS_VALUE, iBV);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewFPS(int const fps)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_FPS, fps);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewFrameInterval(int const interval)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_FRAME_INTERVAL_IN_US, interval);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updatePreviewAEIndexShutterGain(int const index, int const shutter, int const isp_gain, int const sensor_gain)
{
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SHUTTER_SPEED, shutter);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_ISP_GAIN, isp_gain);
    set(MtkCameraParameters::KEY_ENG_PREVIEW_SENSOR_GAIN, sensor_gain);    
    set(MtkCameraParameters::KEY_ENG_PREVIEW_AE_INDEX, index);    
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateCaptureShutterGain(int const shutter, int const isp_gain, int const sensor_gain)
{
    set(MtkCameraParameters::KEY_ENG_CAPTURE_SHUTTER_SPEED, shutter);
    set(MtkCameraParameters::KEY_ENG_CAPTURE_ISP_GAIN, isp_gain);
    set(MtkCameraParameters::KEY_ENG_CAPTURE_SENSOR_GAIN, sensor_gain);    
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
updateEngMsg(char const* msg)
{
    set(MtkCameraParameters::KEY_ENG_MSG, msg);
}

/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
set(char const* key, char const* value)
{
    RWLock::AutoWLock _lock(mRWLock);
    mParameters.set(key, value);
}


/******************************************************************************
*
*******************************************************************************/
void
ParamsManager::
set(char const* key, int value)
{
    RWLock::AutoWLock _lock(mRWLock);
    mParameters.set(key, value);
}


/******************************************************************************
*
*******************************************************************************/
String8
ParamsManager::
getStr(char const* key) const
{
    RWLock::AutoRLock _lock(mRWLock);
    char const* p = mParameters.get(key);
    return  (0 != p) ? String8(p) : String8::empty();
}


/******************************************************************************
*
*******************************************************************************/
int
ParamsManager::
getInt(char const*key) const
{
    RWLock::AutoRLock _lock(mRWLock);
    return mParameters.getInt(key);
}


/******************************************************************************
*
*******************************************************************************/
float
ParamsManager::
getFloat(char const* key) const
{
    RWLock::AutoRLock _lock(mRWLock);
    return mParameters.getFloat(key);
}


/******************************************************************************
*
*******************************************************************************/
bool
ParamsManager::
isEnabled(char const* key) const
{
    if  ( ! key )
    {
        MY_LOGW("Null key");
        return  false;
    }
    //
    RWLock::AutoRLock _lock(mRWLock);
    char const* p = mParameters.get(key);
    //
    if ( ! p ) {
        MY_LOGD_IF(1, "No %s", key);
        return false;
    }
    //
    return ( ::strcmp(p, CameraParameters::TRUE) != 0 ) ? false : true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
dump(int fd, Vector<String8>const& args)
{
    //
    if  ( args.empty() ) {
        mParameters.dump(fd, Vector<String16>());
        return  OK;
    }
    //
    MY_LOGD("args(%d)=%s", args.size(), (*args.begin()).string());
    //
    //
    //  Parse command: "s1=s2"
    if  ( args.size() == 1 ) {
        String8 s8OutputTemp;
        String8 const& s8Arg1 = args[0];
        //  find pattern in s8Arg1: "s1=s2"
        ssize_t const AssignIndex = s8Arg1.find("=");

        //  (1) No assignment. Just show the old value.
        if  ( -1 == AssignIndex ) {
            String8 const key(s8Arg1.string());
            char const *value_old = mParameters.get(key);
            //
            s8OutputTemp = String8::format("\t%s: ", key.string());
            if  ( value_old ) {
                s8OutputTemp += value_old;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
        }
        //  (2) Assignment. Show both old/new values and then set the new value.
        else {
            String8 const key(s8Arg1.string(), AssignIndex);
            char const *value_old = mParameters.get(key);
            char const *value_new = s8Arg1.string() + AssignIndex + 1;
            //
            //  show old key-value
            s8OutputTemp = String8::format("\t[old] %s: ", key.string());
            if  ( value_old ) {
                s8OutputTemp += value_old;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
            //
            //  show new key-value
            s8OutputTemp = String8::format("\t[new] %s: ", key.string());
            if  ( value_new ) {
                s8OutputTemp += value_new;
            }
            s8OutputTemp += "\n";
            ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
            //
            //  set new key-value
            mParameters.set(key, value_new);
        }
        return  OK;
    }
    //
    return  OK;
}

