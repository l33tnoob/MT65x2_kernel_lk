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

#define LOG_TAG "MtkCam/CamClient/RecordClient"
//
#include <mtkcam/v1/PriorityDefs.h>
#include "RecordClient.h"
#include "RecBufManager.h"
using namespace NSCamClient;
using namespace NSRecordClient;
//
#include <sys/prctl.h>
//


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define CB_ADDR_WAIT_TIME           (33*1000) //us


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
RecordClient::
Command::
getName(EID const _eId)
{
#define CMD_NAME(x) case x: return #x
    switch  (_eId)
    {
    CMD_NAME(eID_EXIT);
    CMD_NAME(eID_WAKEUP);
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
RecordClient::
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
RecordClient::
readyToRun()
{
    ::prctl(PR_SET_NAME,"CamClient@Record", 0, 0, 0);
    //
    mi4ThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_CAMERA_RECORD_CLIENT;
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
    //
    mbThreadExit = false;
    //
    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
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
RecordClient::
getCommand(Command& rCmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+ que size(%d)", mCmdQue.size());
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
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
threadLoop()
{
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_WAKEUP:
            onClientThreadLoop(cmd);
            break;
        //
        case Command::eID_EXIT:
            mbThreadExit = true;
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
RecordClient::
onClientThreadLoop(Command const& rCmd)
{
    sp<RecBufManager> pBufMgr   = NULL;
    sp<IImgBufQueue>  pBufQueue = NULL;
    uint32_t i,size,releaseFrameCnt = 0;
    //
    if(!initBuffers())
    {
        MY_LOGE("initBuffers fail");
        return;
    }
    //  (1) Get references to pool/queue before starting, so that nothing will be free during operations.
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
        if  ( ! isEnabledState() )
        {
            MY_LOGI("Record client disabled");
            break;
        }

        //  (.3) re-prepare all TODO buffers, if possible, 
        //  since some DONE/CANCEL buffers return.
        prepareAllTodoBuffers(pBufQueue, pBufMgr);
    }
    //  (6) stop.
    MY_LOGD("pauseProcessor");
    pBufQueue->pauseProcessor();
    MY_LOGD("flushProcessor");
    pBufQueue->flushProcessor();
    MY_LOGD("stopProcessor");
    pBufQueue->stopProcessor();
    //
    //  (7) Cancel all un-returned buffers.
    cancelAllUnreturnBuffers();
    //
    while(1)
    {
        {
            Mutex::Autolock _lock(mModuleMtx);
            //
            size = mvRecBufInfo.size();
            //
            for(i=0; i<size; i++)
            {
                if(mvRecBufInfo[i].Sta != REC_BUF_STA_EMPTY)
                {
                    MY_LOGW("CNT(%d):Wait STA(%d)/VA(0x%08X) to be released",
                            releaseFrameCnt,
                            mvRecBufInfo[i].Sta,
                            (uint32_t)(mvRecBufInfo[i].VirAddr));
                    releaseFrameCnt++;
                    break;
                }
            }
        }
        //
        if(i == size)
        {
            MY_LOGD("All record frames have been released");
            break;
        }
        else
        if(releaseFrameCnt > eMAX_RECORD_BUFFER_NUM)
        {
            MY_LOGW("Timeout:Force to free buffer!");
            break;
        }
        else
        if(android_atomic_release_load(&mbForceReleaseBuf))
        {
            MY_LOGW("Re-record:Force to free buffer!");
            android_atomic_write(0, &mbForceReleaseBuf);
            break;
        }
        else
        {
            usleep(CB_ADDR_WAIT_TIME);
        }
    }
    //
    uninitBuffers();
}

