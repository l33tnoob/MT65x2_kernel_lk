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

#define LOG_TAG "MtkCam/PrvCB"
//
#include <cutils/properties.h>
#include "PreviewClient.h"
#include "ImgBufManager.h"
//
using namespace NSCamClient;
using namespace NSPrvCbClient;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
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
sp<IPreviewClient>
IPreviewClient::
createInstance(sp<IParamsManager> pParamsMgr)
{
    return  new PreviewClient(pParamsMgr);
}


/******************************************************************************
 *
 ******************************************************************************/
PreviewClient::
PreviewClient(sp<IParamsManager> pParamsMgr)
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(pParamsMgr)
    , mIsMsgEnabled(0)
    , mIsPrvStarted(0)
    , ms8PrvTgtFmt("")
    , mi4PrvWidth(0)
    , mi4PrvHeight(0)
    //
    , mi4CallbackRefCount(0)
    , mi8CallbackTimeInMs(0)
    //
    , muImgBufIdx(0)
    , mpImgBufMgr(0)
    , mImgBufList()
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mProfile_callback("performPreviewCallback")
    , mProfile_dequeProcessor("handleReturnBuffers")
    , mProfile_buffer_timestamp("handleReturnBuffers")
    //
    , miLogLevel(1)
    //
    , mDumpMtx()
    , mi4DumpImgBufCount(0)
    , mi4DumpImgBufIndex(0)
    , ms8DumpImgBufPath("")
    //
    , mpExtImgProc(NULL)
{
    MY_LOGD("+ this(%p)", this);
    char cLogLevel[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("debug.camera.previewclient.loglevel", cLogLevel, "1");
    miLogLevel = ::atoi(cLogLevel);
    MY_LOGD("- debug.camera.previewclient.loglevel=%s", cLogLevel);
}


/******************************************************************************
 *
 ******************************************************************************/
PreviewClient::
~PreviewClient()
{
    MY_LOGD("+");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
init()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_PRV_CB, "PrvCB@ImgBufQue");
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("Fail to new ImgBufQueue");
        goto lbExit;
    }
    //
    //
    status = run();
    if  ( OK != status )
    {
        MY_LOGE("Fail to run thread, status[%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    //
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
uninit()
{
    MY_LOGD("+");
    //
    //
    if  ( 0 != mi4CallbackRefCount )
    {
        int64_t const i8CurrentTimeInMs = MtkCamUtils::getTimeInMs();
        MY_LOGW(
            "Preview Callback: ref count(%d)!=0, the last callback before %lld ms, timestamp:(the last, current)=(%lld ms, %lld ms)", 
            mi4CallbackRefCount, (i8CurrentTimeInMs-mi8CallbackTimeInMs), mi8CallbackTimeInMs, i8CurrentTimeInMs
        );
    }
    //
    //
    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
        mpImgBufPvdrClient = NULL;
    }
    //
    //
    if  ( mpImgBufQueue != 0 )
    {
        mpImgBufQueue->stopProcessor();
        mpImgBufQueue = NULL;
    }
    //
    //
    {
        MY_LOGD("getThreadId(%d), getStrongCount(%d), this(%p)", getThreadId(), getStrongCount(), this);
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        requestExit();
        status_t status = join();
        if  ( OK != status )
        {
            MY_LOGW("Not to wait thread(tid:%d), status[%s(%d)]", getThreadId(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
    }
    //
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
setImgBufProviderClient(sp<IImgBufProviderClient>const& rpClient)
{
    bool ret = false;
    //
    MY_LOGD("+ ImgBufProviderClient(%p)", rpClient.get());
    //
    //
    if  ( rpClient == 0 )
    {
        MY_LOGE("NULL ImgBufProviderClient");
        goto lbExit;
    }
    //
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("NULL ImgBufQueue");
        goto lbExit;
    }
    //
    if  ( ! rpClient->onImgBufProviderCreated(mpImgBufQueue) )
    {
        goto lbExit;
    }
    mpImgBufPvdrClient = rpClient;
    //
    //
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
PreviewClient::
setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    Mutex::Autolock _l(mModuleMtx);
    //
    //  value copy
    *mpCamMsgCbInfo = *rpCamMsgCbInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
startPreview()
{
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD("+ current mIsPrvStarted=%d", mIsPrvStarted);
        ::android_atomic_write(1, &mIsPrvStarted);
        //
        ms8PrvTgtFmt = mpParamsMgr->getPreviewFormat();
        mpParamsMgr->getPreviewSize(&mi4PrvWidth, &mi4PrvHeight);
        //
        MY_LOGD("+ preview: WxH=%dx%d, format(%s)", mi4PrvWidth, mi4PrvHeight, ms8PrvTgtFmt.string());
    }
    //
    initBuffers();
    //
    return  onStateChanged();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
stopPreview()
{
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD("+ current mIsPrvStarted=%d", mIsPrvStarted);
        ::android_atomic_write(0, &mIsPrvStarted);
    }
    //
    uninitBuffers();
    //
    return  onStateChanged();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
takePicture()
{
    return  stopPreview();
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
enableMsgType(int32_t msgType)
{
    int32_t const oldMsgType = mpCamMsgCbInfo->mMsgEnabled;
    int32_t const newMsgType = mpCamMsgCbInfo->mMsgEnabled | msgType;
    ::android_atomic_write(newMsgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    updateMsg(oldMsgType, newMsgType);
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
disableMsgType(int32_t msgType)
{
    int32_t const oldMsgType = mpCamMsgCbInfo->mMsgEnabled;
    int32_t const newMsgType = mpCamMsgCbInfo->mMsgEnabled & ~msgType;
    ::android_atomic_write(newMsgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    updateMsg(oldMsgType, newMsgType);
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
updateMsg(int32_t const oldMsgType, int32_t const newMsgType)
{
    int32_t const i4TargetMsgType = CAMERA_MSG_PREVIEW_FRAME;
    bool const isToggleOn   = 0 != (i4TargetMsgType & (~oldMsgType & newMsgType));
    bool const isToggleOff  = 0 != (i4TargetMsgType & (oldMsgType & ~newMsgType));
    //
    MY_LOGD_IF(
        (2<=miLogLevel), 
        "+ oldMsgType = 0x%08x, newMsgType = 0x%08x, (isToggleOn/isToggleOff)=(%d/%d)", 
        oldMsgType, newMsgType, isToggleOn, isToggleOff
    );
    //
    //
    if  (isToggleOn)
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD_IF((1<=miLogLevel), "+ current mIsMsgEnabled=%d", mIsMsgEnabled);
        ::android_atomic_write(1, &mIsMsgEnabled);
        onStateChanged();
    }
    //
    if  (isToggleOff)
    {
        Mutex::Autolock _l(mModuleMtx);
        MY_LOGD_IF((1<=miLogLevel), "+ current mIsMsgEnabled=%d", mIsMsgEnabled);
        ::android_atomic_write(0, &mIsMsgEnabled);
        onStateChanged();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
//  enable if both preview started && message enabled; otherwise disable.
bool
PreviewClient::
isEnabledState() const
{
    return  0 != ::android_atomic_release_load(&mIsMsgEnabled)
        &&  0 != ::android_atomic_release_load(&mIsPrvStarted)
            ;
}


/******************************************************************************
 *
 ******************************************************************************/
//  enable if both preview started && message enabled; otherwise disable.
bool
PreviewClient::
onStateChanged()
{
    bool ret = true;
    //
    if  ( isEnabledState() )
    {
//        if  ( initBuffers() )
        {
            postCommand(Command(Command::eID_WAKEUP));
        }
    }
    else
    {
        if  ( mpImgBufQueue != 0 )
        {
            mpImgBufQueue->pauseProcessor();
        }

//        uninitBuffers();
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
PreviewClient::
sendCommand(
    int32_t cmd, 
    int32_t arg1, 
    int32_t arg2
)
{
    return  INVALID_OPERATION;
}

