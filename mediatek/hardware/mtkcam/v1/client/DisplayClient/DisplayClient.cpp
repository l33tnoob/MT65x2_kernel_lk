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

#define LOG_TAG "MtkCam/DisplayClient"
//
#include <cutils/properties.h>
#include "DisplayClient.h"
using namespace NSDisplayClient;
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
*******************************************************************************/
IDisplayClient*
IDisplayClient::
createInstance()
{
    return  new DisplayClient;
}


/******************************************************************************
*
*******************************************************************************/
DisplayClient::
DisplayClient()
    : IDisplayClient()
    //
    , mModuleMtx()
    , mpDisplayThread(NULL)
    , mIsDisplayEnabled(0)
    , mState(eState_Suspend)
    //
    , mpStreamImgInfo(NULL)
    , mpStreamOps(NULL)
    , mStreamBufList()
    , mi4MaxImgBufCount(0)
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mProfile_enquePrvOps("DisplayClient::enquePrvOps")
    , mProfile_dequeProcessor("DisplayClient::handleReturnBuffers")
    , mProfile_buffer_timestamp("DisplayClient::handleReturnBuffers")
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
    MY_LOGD("+");
    char cLogLevel[PROPERTY_VALUE_MAX] = {'\0'};
    ::property_get("debug.camera.display.loglevel", cLogLevel, "1");
    miLogLevel = ::atoi(cLogLevel);
    MY_LOGD("- debug.camera.display.loglevel=%s", cLogLevel);
}


/******************************************************************************
*
*******************************************************************************/
DisplayClient::
~DisplayClient()
{
    MY_LOGD("+");
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
init()
{
    bool ret = false;
    //
    MY_LOGD("+");
    //
    ret =   createDisplayThread()
        &&  createImgBufQueue()
            ;
    //
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
* Uninitialize the display client.
*******************************************************************************/
bool
DisplayClient::
uninit()
{
    status_t status = OK;
    MY_LOGI("+ getStrongCount(%d)", getStrongCount());
    //
    disableDisplay();
    destroyDisplayThread();
    destroyImgBufQueue();
    //
    MY_LOGI("- getStrongCount(%d)", getStrongCount());
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
createDisplayThread()
{
    bool    ret = false;
    status_t status = OK;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    mpDisplayThread = IDisplayThread::createInstance(this);
    if  (
            mpDisplayThread == 0
        ||  OK != (status = mpDisplayThread->run())
        )
    {
        MY_LOGE(
            "Fail to run DisplayThread - mpDisplayThread.get(%p), status[%s(%d)]", 
            mpDisplayThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
DisplayClient::
destroyDisplayThread()
{
    sp<IDisplayThread> pDisplayThread;
    {
        Mutex::Autolock _l(mModuleMtx);
        pDisplayThread = mpDisplayThread;
        mpDisplayThread = NULL;
    }
    //
    if  ( pDisplayThread != 0 )
    {
        MY_LOGD(
            "DisplayThread: (tid, getStrongCount, pDisplayThread)=(%d, %d, %p)", 
            pDisplayThread->getTid(), pDisplayThread->getStrongCount(), pDisplayThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        pDisplayThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = pDisplayThread->join()) )
        {
            MY_LOGW("Not to wait DisplayThread(tid:%d), status[%s(%d)]", pDisplayThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        pDisplayThread = NULL;
    }
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
createImgBufQueue()
{
    bool ret = false;
    //
    MY_LOGD("+");
    //
    {
        Mutex::Autolock _l(mModuleMtx);
        mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_DISPLAY, "CameraDisplay@ImgBufQue");
        if  ( mpImgBufQueue == 0 )
        {
            MY_LOGE("Fail to new ImgBufQueue");
            goto lbExit;
        }
    }
    //
    mpExtImgProc = ExtImgProc::createInstance();
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->init();
    }
    //
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
DisplayClient::
destroyImgBufQueue()
{
    MY_LOGD("+");
    //
    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
        mpImgBufPvdrClient = NULL;
    }
    //
    sp<IImgBufQueue> pImgBufQueue;
    {
        Mutex::Autolock _l(mModuleMtx);
        pImgBufQueue = mpImgBufQueue;
        mpImgBufQueue = NULL;
    }
    //
    if  ( pImgBufQueue != 0 )
    {
        pImgBufQueue->stopProcessor();
        pImgBufQueue = NULL;
    }
    //
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->uninit();
        mpExtImgProc->destroyInstance();
        mpExtImgProc = NULL;
    }
    //
    MY_LOGD("-");
}


/******************************************************************************
 * Set the preview_stream_ops to which frames are sent.
 *
 * Notes:
 *  (1) When calling setWindow(), all preview parameters have been decided.
 *      [CameraService]
 *          mHardware->setParameters() -> mHardware->setPreviewWindow() -> mHardware->startPreview()
 *          --> enableDisplay during startPreview()
 *          mHardware->setParameters() -> mHardware->startPreview() -> mHardware->setPreviewWindow()
 *          --> enableDisplay during setPreviewWindow()
 *  (2) During inactive preview, window may be changed by setWindow().
*******************************************************************************/
bool
DisplayClient::
setWindow(
    preview_stream_ops*const window, 
    int32_t const   wndWidth, 
    int32_t const   wndHeight, 
    int32_t const   i4MaxImgBufCount
)
{
    MY_LOGI("+ window(%p), WxH=%dx%d, count(%d)", window, wndWidth, wndHeight, i4MaxImgBufCount);
    //
    if  ( ! window )
    {
        MY_LOGE("NULL window passed into");
        return  false;
    }
    //
    if  ( 0 >= wndWidth || 0 >= wndHeight || 0 >= i4MaxImgBufCount )
    {
        MY_LOGE("bad arguments - WxH=%dx%d, count(%d)", wndWidth, wndHeight, i4MaxImgBufCount);
        return  false;
    }
    //
    //
    Mutex::Autolock _l(mModuleMtx);
    return  set_preview_stream_ops(window, wndWidth, wndHeight, i4MaxImgBufCount);
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
setImgBufProviderClient(sp<IImgBufProviderClient>const& rpClient)
{
    bool ret = false;
    //
    MY_LOGD("+ ImgBufProviderClient(%p), mpImgBufQueue.get(%p)", rpClient.get(), mpImgBufQueue.get());
    //
    if  ( rpClient == 0 )
    {
        MY_LOGE("NULL ImgBufProviderClient");
        mpImgBufPvdrClient = NULL;
        goto lbExit;
    }
    //
    if  ( mpImgBufQueue != 0 )
    {
        if  ( ! rpClient->onImgBufProviderCreated(mpImgBufQueue) )
        {
            goto lbExit;
        }
        mpImgBufPvdrClient = rpClient;
    }
    //
    ret = true;
lbExit:
    MY_LOGD("-");
    return  ret;
};


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
isDisplayEnabled() const
{
    return  0 != ::android_atomic_acquire_load(&mIsDisplayEnabled);
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
enableDisplay(
    int32_t const   i4Width, 
    int32_t const   i4Height, 
    int32_t const   i4BufCount, 
    sp<IImgBufProviderClient>const& rpClient
)
{
    bool ret = false;
    preview_stream_ops* pStreamOps = mpStreamOps;
    //
    //  [1] Re-configurate this instance if any setting changes.
    if  ( ! checkConfig(i4Width, i4Height, i4BufCount, rpClient) )
    {
        MY_LOGW("<Config Change> Uninit the current DisplayClient(%p) and re-config...", this);
        //
        //  [.1] uninitialize
        uninit();
        //
        //  [.2] initialize
        if  ( ! init() )
        {
            MY_LOGE("re-init() failed");
            goto lbExit;
        }
        //
        //  [.3] set related window info.
        if  ( ! setWindow(pStreamOps, i4Width, i4Height, i4BufCount) )
        {
            goto lbExit;
        }
        //
        //  [.4] set Image Buffer Provider Client.
        if  ( ! setImgBufProviderClient(rpClient) )
        {
            goto lbExit;
        }
    }
    //
    //  [2] Enable.
    if  ( ! enableDisplay() )
    {
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
enableDisplay()
{
    bool ret = false;
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    MY_LOGD("+ isDisplayEnabled(%d), mpDisplayThread.get(%p)", isDisplayEnabled(), mpDisplayThread.get());
    //
    //  (2) Check to see if it has been enabled.
    if  ( isDisplayEnabled() )
    {
        MY_LOGD("Display is already enabled");
        ret = true;
        goto lbExit;
    }
    //
    //  (3) Check to see if thread is alive.
    if  ( mpDisplayThread == 0 )
    {
        MY_LOGE("NULL mpDisplayThread");
        goto lbExit;
    }
    //
    //  (4) Enable the flag.
    ::android_atomic_write(1, &mIsDisplayEnabled);
    //
    //  (5) Post a command to wake up the thread.
    mpDisplayThread->postCommand(Command(Command::eID_WAKEUP));
    //
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
disableDisplay()
{
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    MY_LOGD("+ isDisplayEnabled(%d)", isDisplayEnabled());
    //
    //  (2) Check to see if it has been disabled.
    if  ( ! isDisplayEnabled() )
    {
        MY_LOGD("Display is already disabled");
        goto lbExit;
    }
    //
    //  (3) 
    ::android_atomic_write(0, &mIsDisplayEnabled);
    //
    //  (4) Pause
    if  ( mpImgBufQueue != 0 )
    {
        mpImgBufQueue->pauseProcessor();
    }
    //
lbExit:
    MY_LOGD("- isDisplayEnabled(%d)", isDisplayEnabled());
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DisplayClient::
checkConfig(
    int32_t const   i4Width, 
    int32_t const   i4Height, 
    int32_t const   i4BufCount, 
    sp<IImgBufProviderClient>const& rpClient
)
{
    Mutex::Autolock _l(mModuleMtx);
    int32_t const i4ImgWidth = mpStreamImgInfo->mu4ImgWidth;
    int32_t const i4ImgHeight= mpStreamImgInfo->mu4ImgHeight;
    //
    if  (
            i4BufCount  != mi4MaxImgBufCount
        ||  i4Width     != i4ImgWidth
        ||  i4Height    != i4ImgHeight
        ||  rpClient    != mpImgBufPvdrClient
        )
    {
        MY_LOGW(
            "Different [ImgBufProviderClient, BufCount, WxH]:(%p, %d, %dx%d) -> (%p, %d, %dx%d)", 
            mpImgBufPvdrClient.get(), mi4MaxImgBufCount, i4ImgWidth, i4ImgHeight, 
            rpClient.get(), i4BufCount, i4Width, i4Height
        );
        return  false;
    }
    //
    MY_LOGD_IF(
        1, 
        "The Same [ImgBufProviderClient, BufCount, WxH]:(%p, %d, %dx%d) -> (%p, %d, %dx%d)", 
        mpImgBufPvdrClient.get(), mi4MaxImgBufCount, i4ImgWidth, i4ImgHeight, 
        rpClient.get(), i4BufCount, i4Width, i4Height
    );
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
DisplayClient::
waitUntilDrained()
{
    nsecs_t timeout = 3000000000LL; // 3 sec
    //
    status_t status = OK;
    //
    Mutex::Autolock _l(mStateMutex);
    while   ( eState_Suspend != mState )
    {
        MY_LOGD("Wait %lld ns", timeout);
        nsecs_t startTime = ::systemTime();
        status = mStateCond.waitRelative(mStateMutex, timeout);
        if (TIMED_OUT == status)
        {
            MY_LOGW("state:%d - TIMED_OUT", mState);
            return status;
        }
        else if (OK != status)
        {
            MY_LOGW("state:%d - status[%s(%d)]", mState, ::strerror(-status), status);
            return status;
        }
        //
        nsecs_t deltaTime = ::systemTime() - startTime;
        if (timeout <= deltaTime) {
            timeout = 0;
        } else {
            timeout -= deltaTime;
        }
    }
    //
    return  status;
}

