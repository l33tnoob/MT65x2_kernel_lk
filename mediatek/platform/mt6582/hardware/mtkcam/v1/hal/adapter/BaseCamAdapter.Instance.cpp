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
#include "inc/CamUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include "inc/ImgBufProvidersManager.h"
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include "inc/BaseCamAdapter.h"
//


/******************************************************************************
*   Function Prototype.
*******************************************************************************/

sp<ICamAdapter> createMtkEngCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkAtvCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkPhotoCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkDefaultCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkVTCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
sp<ICamAdapter> createMtkZsdCamAdapter(String8 const& rName, int32_t const i4OpenId,  sp<IParamsManager> pParamsMgr);


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
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
sp<ICamAdapter>
ICamAdapter::
createInstance(
    String8 const&      rName, 
    int32_t const       i4OpenId, 
    sp<IParamsManager>  pParamsMgr
)
{
    String8 const s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(pParamsMgr->getHalAppMode());
    //
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_DEFAULT )
    {
        return  createMtkDefaultCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG )
    {
        return  createMtkEngCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ATV )
    {
        return  createMtkAtvCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_S3D )
    {
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_VT )
    {
        return  createMtkVTCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO )
    {
        return  createMtkPhotoCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO )
    {
        //Default and video use the same adapater for now,
        //and video may use different adapter in the future if there are so many differences.
        return  createMtkDefaultCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    if  ( s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD )
    {
        return  createMtkZsdCamAdapter(s8AppMode, i4OpenId, pParamsMgr);
    }
    //
    //
    //
    CAM_LOGE("[ICamAdapter::%s] NOT IMPLEMENT YET !", __FUNCTION__);
    return  NULL;
}


/******************************************************************************
*
*******************************************************************************/
bool
ICamAdapter::
isValidInstance(sp<ICamAdapter>const& rpCamAdapter)
{
    if  ( rpCamAdapter == 0 )
    {
        CAM_LOGW("[ICamAdapter::%s] NULL rpCamAdapter", __FUNCTION__);
        return false;
    }
    //
    String8 const s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(rpCamAdapter->getParamsManager()->getHalAppMode());
    //
    char const*const pszName    = rpCamAdapter->getName();
    int32_t   const i4OpenId    = rpCamAdapter->getOpenId();
    //
    //
    CAM_LOGD(
        "[ICamAdapter::%s] OpenId(%d) current(%s)/expect(%s)", 
        __FUNCTION__, i4OpenId, pszName, s8AppMode.string()
    );
    //
    return  (s8AppMode == pszName);
}

