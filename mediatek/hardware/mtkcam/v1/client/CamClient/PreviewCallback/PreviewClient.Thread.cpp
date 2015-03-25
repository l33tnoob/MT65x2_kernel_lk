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
#include <mtkcam/v1/PriorityDefs.h>
//
#include "PreviewClient.h"
#include "ImgBufManager.h"
using namespace NSCamClient;
using namespace NSPrvCbClient;
//
#include <sys/prctl.h>
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
char const*
PreviewClient::
Command::
getName(EID const _eId)
{
#define CMD_NAME(x) case x: return #x
    switch  (_eId)
    {
    CMD_NAME(eID_EXIT);
    CMD_NAME(eID_WAKEUP);
    CMD_NAME(eID_PREVIEW_FRAME);
    CMD_NAME(eID_POSTVIEW_FRAME);
    default:
        break;
    }
#undef  CMD_NAME
    return  "";
}


/******************************************************************************
 *
 ******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
PreviewClient::
requestExit()
{
    MY_LOGD("+");
    Thread::requestExit();
    //
    postCommand(Command(Command::eID_EXIT));
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
PreviewClient::
readyToRun()
{
    ::prctl(PR_SET_NAME,(unsigned long)"CamClient@Preview", 0, 0, 0);
    //
    mi4ThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_CAMERA_PREVIEW_CLIENT;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
#endif
    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
postCommand(Command const& rCmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    if  ( ! mCmdQue.empty() )
    {
        Command const& rBegCmd = *mCmdQue.begin();
        MY_LOGW("que size:%d > 0 with begin cmd::%s", mCmdQue.size(), rBegCmd.name());
    }
    //
    mCmdQue.push_back(rCmd);
    mCmdQueCond.broadcast();
    //
    MY_LOGD("- new command::%s", rCmd.name());
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
getCommand(Command& rCmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    MY_LOGD_IF((2<=miLogLevel), "+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        rCmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%s", rCmd.name());
    }
    //
    MY_LOGD_IF((2<=miLogLevel), "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
threadLoop()
{
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_WAKEUP:
        case Command::eID_PREVIEW_FRAME:
        case Command::eID_POSTVIEW_FRAME:
            onClientThreadLoop(cmd);
            break;
        //
        case Command::eID_EXIT:
        default:
            MY_LOGD("Command::%s", cmd.name());
            break;
        }
    }
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
onClientThreadLoop(Command const& rCmd)
{
    //  (1) Get references to pool/queue before starting, so that nothing will be free during operations.
    sp<ImgBufManager> pBufMgr   = NULL;
    sp<IImgBufQueue>  pBufQueue = NULL;
    {
        Mutex::Autolock _l(mModuleMtx);
        //
        pBufMgr     = mpImgBufMgr;
        pBufQueue   = mpImgBufQueue;
        if  ( pBufMgr == 0 || pBufQueue == 0 || ! isEnabledState() )
        {
            MY_LOGW("pBufMgr(%p), pBufQueue(%p), isEnabledState(%d)", pBufMgr.get(), pBufQueue.get(), isEnabledState());
            return;
        }
    }

    //  (2) stop & clear all buffers so that we won't deque any undefined buffer.
    pBufQueue->stopProcessor();

    //  (3) Prepare all TODO buffers.
    if  ( ! prepareAllTodoBuffers(pBufQueue, pBufMgr) )
    {
        return;
    }

    //  (4) Start
    if  ( ! pBufQueue->startProcessor() )
    {
        return;
    }

    //  (5)   Do until all wanted messages are disabled.
    while   (1)
    {
        //  (.1)
        waitAndHandleReturnBuffers(pBufQueue);

        //  (.2) break if disabled.
        //  add isProcessorRunning to make sure the former pauseProcessor 
        //  is sucessfully processed.
        if  ( ! isEnabledState() || ! pBufQueue->isProcessorRunning() )
        {
            MY_LOGI("Preview client disabled");
            break;
        }

        //  (.3) re-prepare all TODO buffers, if possible, 
        //  since some DONE/CANCEL buffers return.
        prepareAllTodoBuffers(pBufQueue, pBufMgr);
    }

    //  (6) stop.
    pBufQueue->pauseProcessor();
    pBufQueue->flushProcessor(); // clear "TODO"
    pBufQueue->stopProcessor();  // clear "DONE"
    //
    //  (7) Cancel all un-returned buffers.
    cancelAllUnreturnBuffers();
}

