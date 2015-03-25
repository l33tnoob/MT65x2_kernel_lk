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
status_t
ParamsManager::
checkParams(CameraParameters const& params) const
{
    status_t status = OK;

#if 1
    bool ret = true
        &&  (OK == ( status = checkFlashMode(params) ))
        &&  (OK == ( status = checkFocusMode(params) ))
        &&  (OK == ( status = checkFocusAreas(params) ))
        &&  (OK == ( status = checkMeteringAreas(params) ))
        &&  (OK == ( status = checkPreviewSize(params) ))
        &&  (OK == ( status = checkPreviewFpsRange(params) ))
        &&  (OK == ( status = checkZoomValue(params) ))
            ;
#endif
    return  status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkFlashMode(CameraParameters const& params) const
{
    // workaround: for 3rd party set invalid flash mode.
    //             only test 'invalid' parameter.
    
    char const* pszMode = params.get(CameraParameters::KEY_FLASH_MODE);
    if ( pszMode != NULL)
    { 
        if  ( ::strcmp(pszMode, "invalid") == 0 )
        {
            MY_LOGE("Invalid FLASH mode = %s", pszMode);
            return BAD_VALUE;
        }
    }
    //
    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkFocusMode(CameraParameters const& params) const
{
    char const* pszMode          = params.get(CameraParameters::KEY_FOCUS_MODE);
    char const* pszModeSupported = params.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES);
    return isParameterValid(pszMode, pszModeSupported);
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
isParameterValid(char const* param, char const* supportedParams) const
{
    char *pos = NULL;

    if ( NULL == supportedParams )
    {
        MY_LOGE("Invalid supported parameters string");    
        return BAD_VALUE;
    }

    if ( NULL == param )
    {
        MY_LOGE("Invalid parameter string");
        return BAD_VALUE;
    }
    
    pos = strstr(supportedParams, param);
    if ( NULL == pos )
    {
        MY_LOGE("(%s) does not support (%s) mode", supportedParams, param);
        return BAD_VALUE;
    }

    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkFocusAreas(CameraParameters const& params) const
{
    char const* pszVal = params.get(CameraParameters::KEY_FOCUS_AREAS);
    if  ( pszVal )
    {
        int const numArea = params.getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
        if ( numArea > 0 )
        {
            return  isAreaValid(pszVal, numArea);
        }
    }
    //
    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkMeteringAreas(CameraParameters const& params) const
{
    char const* pszVal = params.get(CameraParameters::KEY_METERING_AREAS);
    if  ( pszVal )
    {
        int const numArea = params.getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS);
        if ( numArea > 0 )
        {
            return  isAreaValid(pszVal, numArea);
        }
    }
    //
    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
isAreaValid(char const* param, int const maxNumArea) const
{
    if ( NULL == param )
    {
        CAM_LOGD("Invalid supported parameters string");
        return BAD_VALUE;
    }
    List<camera_area_t> areas;
    return parseCamAreas(param, areas, maxNumArea);
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
parseCamAreas(char const* areaStr, List<camera_area_t> &areas, int const maxNumArea) const
{
    status_t status = BAD_VALUE;
    char *ctx;
    char *pArea = NULL;
    char *pStart = NULL;
    char *pEnd = NULL;
    const char *startToken = "(";
    const char endToken = ')';
    const char sep = ',';
    char *tmpBuffer = NULL;

    camera_area_t currentArea; 
    int numArea = 0;

    // 1.  parse areas
    if ( 0 == areaStr || 0 == *areaStr ) {
        return BAD_VALUE;
    }

    const int areaLength = ::strlen(areaStr) + 1;
    if ( 0 >= areaLength ) {
        return BAD_VALUE;
    }

    tmpBuffer = ( char * ) ::malloc(areaLength);
    if ( NULL == tmpBuffer ) {
        return NO_MEMORY;
    }

    ::memcpy(tmpBuffer, areaStr, areaLength);

    pArea = strtok_r(tmpBuffer, startToken, &ctx);

    do {
        pStart = pArea;
        // left 
        if ( NULL == pStart ) {
            MY_LOGE("Parsing of the left area coordinate failed!");
            goto lbExit;
        }
        currentArea.left = static_cast<int32_t>(strtol(pStart, &pEnd, 10));

        // top 
        if ( sep != *pEnd ) {
            MY_LOGE("Parsing of the top area coordinate failed!");
            goto lbExit;
        }
        currentArea.top = static_cast<int32_t>(strtol(pEnd+1, &pEnd, 10));

        // right 
        if ( sep != *pEnd ) {
            MY_LOGE("Parsing of the right area coordinate failed!");
            goto lbExit;
        }
        currentArea.right = static_cast<int32_t>(strtol(pEnd+1, &pEnd, 10));

        // bottom 
        if ( sep != *pEnd ) {
            MY_LOGE("Parsing of the bottom area coordinate failed!");
            goto lbExit;
        }
        currentArea.bottom = static_cast<int32_t>(strtol(pEnd+1, &pEnd, 10));

        // weight 
        if ( sep != *pEnd ) {
            MY_LOGE("Parsing of the weight area coordinate failed!");
            goto lbExit;
        }
        currentArea.weight = static_cast<int32_t>(strtol(pEnd+1, &pEnd, 10));

        if ( endToken != *pEnd ) {
            MY_LOGE("Malformed area!");
            goto lbExit;
        }

        // 1- 2.  check area
        if (!checkCamArea(currentArea) ) {
            MY_LOGE("Error area!");
            goto lbExit;
        }

        numArea++;
        if (numArea > maxNumArea) {
            MY_LOGE("Error count of area! (numArea, maxNumArea)=(%d, %d)", numArea, maxNumArea);
            goto lbExit;
        }
        areas.push_back(currentArea);
        pArea = strtok_r(NULL, startToken, &ctx);

    } while ( NULL != pArea );
    //
    //
    status = NO_ERROR;
lbExit:
    if ( NULL != tmpBuffer ) {
        ::free(tmpBuffer);
    }
    //
    return  status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkPreviewSize(CameraParameters const& params) const
{
    int prvWidth = 0, prvHeight = 0;
    params.getPreviewSize(&prvWidth, &prvHeight);
    
    if ( ! ( prvWidth > 0 && prvHeight > 0 )
        )
    {
        MY_LOGE("(prvWidth, prvHeight)=(%d, %d)", prvWidth, prvHeight);
        return BAD_VALUE;
    }
    //
    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
status_t
ParamsManager::
checkPreviewFpsRange(CameraParameters const& params) const
{
    int minFps = 0, maxFps = 0;
    params.getPreviewFpsRange(&minFps, &maxFps);
    if  (
            ( minFps < 0 || maxFps < 0 )
        ||  ( minFps > maxFps )
        )
    {
        MY_LOGE("(minFps, maxFps)=(%d, %d)", minFps, maxFps);
        return BAD_VALUE;
    }
    //
    return  NO_ERROR;
}


status_t
ParamsManager::
checkZoomValue(CameraParameters const& params) const
{
    int maxZoom     = params.getInt(CameraParameters::KEY_MAX_ZOOM);
    int zoomValue   = params.getInt(CameraParameters::KEY_ZOOM);
    if  ( zoomValue > maxZoom )
    {
        MY_LOGE("Invalid zoomValue(%d), bigger than maxZoom(%d)", zoomValue, maxZoom);
        return BAD_VALUE;
    }
    //
    return  NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
#define CAM_AREA_LEFT               (-1000)
#define CAM_AREA_TOP                (-1000)
#define CAM_AREA_RIGHT              (1000)
#define CAM_AREA_BOTTOM             (1000)
#define CAM_AREA_WEIGHT_MIN         (1)
#define CAM_AREA_WEIGHT_MAX         (1000)

bool
ParamsManager::
checkCamArea(camera_area_t const& camArea) const
{
    //Handles the invalid regin corner case.
    if  (
            ( 0 == camArea.top )
        &&  ( 0 == camArea.left )
        &&  ( 0 == camArea.bottom )
        &&  ( 0 == camArea.right )
        &&  ( 0 == camArea.weight )
        )
    {
        // A special case of single focus area (0,0,0,0,0) means driver to decide
        // the focus area. For example, the driver may use more signals to decide
        // focus areas and change them dynamically. Apps can set (0,0,0,0,0) if they
        // want the driver to decide focus areas.
        return true;
    }

    if ( ( CAM_AREA_WEIGHT_MIN > camArea.weight ) ||  ( CAM_AREA_WEIGHT_MAX < camArea.weight ) ) {
        MY_LOGE("Camera area weight is invalid %d", camArea.weight);
        return false;
    }

    if ( ( CAM_AREA_TOP > camArea.top ) || ( CAM_AREA_BOTTOM < camArea.top ) ) {
        MY_LOGE("Camera area top coordinate is invalid %d", camArea.top );
        return false;
    }

    if ( ( CAM_AREA_TOP > camArea.bottom ) || ( CAM_AREA_BOTTOM < camArea.bottom ) ) {
        MY_LOGE("Camera area bottom coordinate is invalid %d", camArea.bottom );
        return false;
    }

    if ( ( CAM_AREA_LEFT > camArea.left ) || ( CAM_AREA_RIGHT < camArea.left ) ) {
        MY_LOGE("Camera area left coordinate is invalid %d", camArea.left );
        return false;
    }

    if ( ( CAM_AREA_LEFT > camArea.right ) || ( CAM_AREA_RIGHT < camArea.right ) ) {
        MY_LOGE("Camera area right coordinate is invalid %d", camArea.right );
        return false;
    }

    if ( camArea.left >= camArea.right ) {
        MY_LOGE("Camera area left larger than right");
        return false;
    }

    if ( camArea.top >= camArea.bottom ) {
        MY_LOGE("Camera area top larger than bottom");
        return false;
    }

    return true;
}

