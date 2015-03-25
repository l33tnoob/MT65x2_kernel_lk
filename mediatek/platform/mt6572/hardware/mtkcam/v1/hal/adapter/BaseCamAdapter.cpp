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
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


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
BaseCamAdapter::
BaseCamAdapter(
    String8 const&      rName, 
    int32_t const       i4OpenId, 
    sp<IParamsManager>  pParamsMgr
)
    : ICamAdapter()
    , mName(rName)
    , mi4OpenId(i4OpenId)
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    //
    , mpParamsMgr(pParamsMgr)
    , mpImgBufProvidersMgr(new ImgBufProvidersManager)
    //
{
    MY_LOGD(
        "sizeof=%d, this=%p, mpCamMsgCbInfo=%p, mpParamsMgr=%p, mImgBufProvidersMgr=%p", 
        sizeof(BaseCamAdapter), this, &mpCamMsgCbInfo, &mpParamsMgr, &mpImgBufProvidersMgr
    );
}


/******************************************************************************
*
*******************************************************************************/
BaseCamAdapter::
~BaseCamAdapter()
{
    MY_LOGD("tid(%d), OpenId(%d)", ::gettid(), getOpenId());
    MY_LOGD(
        "sizeof=%d, this=%p, mpCamMsgCbInfo=%p, mpParamsMgr=%p, mImgBufProvidersMgr=%p", 
        sizeof(BaseCamAdapter), this, &mpCamMsgCbInfo, &mpParamsMgr, &mpImgBufProvidersMgr
    );    
}


/******************************************************************************
*
*******************************************************************************/
sp<IParamsManager>const
BaseCamAdapter::
getParamsManager() const
{
    return mpParamsMgr;
}

sp<CamMsgCbInfo>const
BaseCamAdapter::
getCamMsgCbInfo() const
{
    return mpCamMsgCbInfo;
}



/******************************************************************************
* Set camera message-callback information.
*******************************************************************************/
void
BaseCamAdapter::
setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    //  value copy
    *mpCamMsgCbInfo = *rpCamMsgCbInfo;
    MY_LOGD("mpCamMsgCbInfo.get(%p), mpCamMsgCbInfo->getStrongCount(%d)", mpCamMsgCbInfo.get(), mpCamMsgCbInfo->getStrongCount());
}


/******************************************************************************
* Enable a message, or set of messages.
*******************************************************************************/
void
BaseCamAdapter::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
* Disable a message, or a set of messages.
*
* Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
* should not rely on its client to call releaseRecordingFrame() to release
* video recording frames sent out by the cameral hal before and after the
* disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
* modify/access any video recording frame after calling
* disableMsgType(CAMERA_MSG_VIDEO_FRAME).
*******************************************************************************/
void
BaseCamAdapter::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
* Query whether a message, or a set of messages, is enabled.
* Note that this is operates as an AND, if any of the messages
* queried are off, this will return false.
*******************************************************************************/
bool
BaseCamAdapter::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
* Send command to camera driver.
*******************************************************************************/
status_t
BaseCamAdapter::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    status_t status = OK;
    switch  (cmd)
    {
    default:
        MY_LOGW("tid(%d), bad command(%#x), (arg1, arg2)=(0x%x, 0x%x)", ::gettid(), cmd, arg1, arg2);
        status = INVALID_OPERATION;
        break;
    }
    return  status;
}


/******************************************************************************
*   Notify when IImgBufProvider is created.
*******************************************************************************/
bool
BaseCamAdapter::
onImgBufProviderCreated(sp<IImgBufProvider>const& rpProvider)
{
    if  ( rpProvider == 0 )
    {
        MY_LOGW("NULL provider");
        return  false;
    }
    //
    int32_t const i4ProviderId = rpProvider->getProviderId();
    if  ( (size_t)i4ProviderId >= mpImgBufProvidersMgr->getProvidersSize() )
    {
        MY_LOGE("bad ProviderId=%x >= %d", i4ProviderId, mpImgBufProvidersMgr->getProvidersSize());
        return  false;
    }
    //
    mpImgBufProvidersMgr->setProvider(i4ProviderId, rpProvider);
    //
    //
    MY_LOGI("- id=%d, ImgBufProvider=%p", i4ProviderId, rpProvider.get());
    return  true;
}


/******************************************************************************
*   Notify when IImgBufProvider is destroyed.
*******************************************************************************/
void
BaseCamAdapter::
onImgBufProviderDestroyed(int32_t const i4ProviderId)
{
    if  ( (size_t)i4ProviderId >= mpImgBufProvidersMgr->getProvidersSize() )
    {
        MY_LOGE("bad ProviderId=%x >= %d", i4ProviderId, mpImgBufProvidersMgr->getProvidersSize());
    }
    //
    MY_LOGI("id=%d, ImgBufProvider=%p", i4ProviderId, mpImgBufProvidersMgr->getProvider(i4ProviderId).get());
    //
    mpImgBufProvidersMgr->setProvider(i4ProviderId, NULL);
}

