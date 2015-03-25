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
// Derived class must implement the below function. The thread starts its
// life here. There are two ways of using the Thread object:
// 1) loop: if this function returns true, it will be called again if 
//          requestExit() wasn't called.
// 2) once: if this function returns false, the thread will exit upon return.
bool
DisplayClient::
onThreadLoop(Command const& rCmd)
{
    //  (0) lock Processor.
    sp<IImgBufQueue> pImgBufQueue;
    {
        Mutex::Autolock _l(mModuleMtx);
        pImgBufQueue = mpImgBufQueue;
        if  ( pImgBufQueue == 0 || ! isDisplayEnabled() )
        {
            MY_LOGW("pImgBufQueue.get(%p), isDisplayEnabled(%d)", pImgBufQueue.get(), isDisplayEnabled());
            return  true;
        }
    }

    //  (1) Prepare all TODO buffers.
    if  ( ! prepareAllTodoBuffers(pImgBufQueue) )
    {
        return  true;
    }

    //  (2) Start
    if  ( ! pImgBufQueue->startProcessor() )
    {
        return  true;
    }
    //
    {
        Mutex::Autolock _l(mStateMutex);
        mState = eState_Loop;
        mStateCond.broadcast();
    }
    //
    //  (3) Do until disabled.
    while   ( 1 )
    {
        //  (.1)
        waitAndHandleReturnBuffers(pImgBufQueue);

        //  (.2) break if disabled.
        if  ( ! isDisplayEnabled() )
        {
            MY_LOGI("Display disabled");
            break;
        }

        //  (.3) re-prepare all TODO buffers, if possible, 
        //  since some DONE/CANCEL buffers return.
        prepareAllTodoBuffers(pImgBufQueue);
    }
    //
    //  (4) Stop
    pImgBufQueue->pauseProcessor();
    pImgBufQueue->flushProcessor();
    pImgBufQueue->stopProcessor();
    //
    //  (5) Cancel all un-returned buffers.
    cancelAllUnreturnBuffers();
    //
    {
        Mutex::Autolock _l(mStateMutex);
        mState = eState_Suspend;
        mStateCond.broadcast();
    }
    //
    return  true;
}


/******************************************************************************
*   dequePrvOps() -> enqueProcessor() & enque Buf List
*******************************************************************************/
bool
DisplayClient::
prepareOneTodoBuffer(sp<IImgBufQueue>const& rpBufQueue)
{
    bool ret = false;
    //
    MY_LOGD_IF((2<=miLogLevel), "+");
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) deque it from PrvOps
    sp<StreamImgBuf> pStreamImgBuf;
    if  ( ! dequePrvOps(pStreamImgBuf) )
    {
        goto lbExit;
    }
    //
    //  (3) enque it into Processor
    ret = rpBufQueue->enqueProcessor(
        ImgBufQueNode(pStreamImgBuf, ImgBufQueNode::eSTATUS_TODO)
    );
    if  ( ! ret ) {
        MY_LOGW("enqueProcessor() fails");
        MY_LOGW("[TODO] Should remove it from list and cancelPrvOps...");
        goto lbExit;
    }
    //
    //  (4) enque it into List & increment the list size.
    mStreamBufList.push_back(pStreamImgBuf);
    //
    ret = true;
lbExit:
    MY_LOGD_IF((2<=miLogLevel), "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
*   dequePrvOps() -> enqueProcessor() & enque Buf List
*******************************************************************************/
bool
DisplayClient::
prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue)
{
    bool ret = false;
    //
    MY_LOGD_IF((2<=miLogLevel), "+ mStreamBufList.size(%d)", mStreamBufList.size());
    //
    while   ( mStreamBufList.size() < (size_t)mi4MaxImgBufCount )
    {
        if  ( ! prepareOneTodoBuffer(rpBufQueue) )
        {
            MY_LOGW("mStreamBufList.size(%d) < mi4MaxImgBufCount(%d)", mStreamBufList.size(), mi4MaxImgBufCount);
            break;
        }
    }
    //
    ret = (mStreamBufList.size() > 0);
    //
    MY_LOGD_IF((2<=miLogLevel), "- ret(%d) mStreamBufList.size(%d)", ret, mStreamBufList.size());
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
cancelAllUnreturnBuffers()
{
    MY_LOGD_IF((1<=miLogLevel), "+");
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    while   ( ! mStreamBufList.empty() )
    {
        sp<StreamImgBuf>const pStreamImgBuf = *mStreamBufList.begin();
        //  Remove the node from the list.
        mStreamBufList.erase(mStreamBufList.begin());
        //
        MY_LOGD(
            "Cancel buffer:[ion:%d %p/%d %lld %dx%d %s] StrongCount:%d", 
            pStreamImgBuf->getIonFd(), 
            pStreamImgBuf->getVirAddr(), pStreamImgBuf->getBufSize(), pStreamImgBuf->getTimestamp(), 
            pStreamImgBuf->getImgWidth(), pStreamImgBuf->getImgHeight(), pStreamImgBuf->getImgFormat().string(), 
            pStreamImgBuf->getStrongCount()
        );
        cancelPrvOps(pStreamImgBuf);
    }
    //
    MY_LOGD_IF((1<=miLogLevel), "-");
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue)
{
    bool ret = false;
    Vector<ImgBufQueNode> vQueNode;
    //
    MY_LOGD_IF((1<=miLogLevel), "+");
    //
    //  (1) deque buffers from processor.
    rpBufQueue->dequeProcessor(vQueNode);
    if  ( vQueNode.empty() ) {
        MY_LOGW("vQueNode.empty()");
        goto lbExit;
    }

    //  (2) handle buffers dequed from processor.
    ret = handleReturnBuffers(vQueNode);

lbExit:
    //
    MY_LOGD_IF((2<=miLogLevel), "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode)
{
    /*
     * Notes:
     *  For 30 fps, we just enque (display) the latest frame, 
     *  and cancel the others.
     *  For frame rate > 30 fps, we should judge the timestamp here or source.
     */
    //  (1) determine the latest DONE buffer index to display; otherwise CANCEL.
    int32_t idxToDisp = 0;
    for ( idxToDisp = rvQueNode.size()-1; idxToDisp >= 0; idxToDisp--)
    {
        if  ( rvQueNode[idxToDisp].isDONE() )
            break;
    }
    if  ( rvQueNode.size() > 1 )
    {
        MY_LOGW("(%d) display frame count > 1 --> select %d to display", rvQueNode.size(), idxToDisp);
    }
    //
    //  Show Time duration.
    if  ( 0 <= idxToDisp )
    {
        nsecs_t const _timestamp1 = rvQueNode[idxToDisp].getImgBuf()->getTimestamp();
        mProfile_buffer_timestamp.pulse(_timestamp1);
        nsecs_t const _msDuration_buffer_timestamp = ::ns2ms(mProfile_buffer_timestamp.getDuration());
        mProfile_buffer_timestamp.reset(_timestamp1);
        //
        mProfile_dequeProcessor.pulse();
        nsecs_t const _msDuration_dequeProcessor = ::ns2ms(mProfile_dequeProcessor.getDuration());
        mProfile_dequeProcessor.reset();
        //
        MY_LOGD_IF(
            (1<=miLogLevel), "+ %s(%lld) %s(%lld)", 
            (_msDuration_buffer_timestamp < 0 ) ? "time inversion!" : "", _msDuration_buffer_timestamp, 
            (_msDuration_dequeProcessor > 34) ? "34ms < Duration" : "", _msDuration_dequeProcessor
        );
    }
    //
    //  (2) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    //  (3) Remove from List and enquePrvOps/cancelPrvOps, one by one.
    int32_t const queSize = rvQueNode.size();
    for (int32_t i = 0; i < queSize; i++)
    {
        sp<IImgBuf>const&       rpQueImgBuf = rvQueNode[i].getImgBuf(); //  ImgBuf in Queue.
        sp<StreamImgBuf>const pStreamImgBuf = *mStreamBufList.begin();  //  ImgBuf in List.
        //  (.1)  Check valid pointers to image buffers in Queue & List
        if  ( rpQueImgBuf == 0 || pStreamImgBuf == 0 )
        {
            MY_LOGW("Bad ImgBuf:(Que[%d], List.begin)=(%p, %p)", i, rpQueImgBuf.get(), pStreamImgBuf.get());
            continue;
        }
        //  (.2)  Check the equality of image buffers between Queue & List.
        if  ( rpQueImgBuf->getVirAddr() != pStreamImgBuf->getVirAddr() )
        {
            MY_LOGW("Bad address in ImgBuf:(Que[%d], List.begin)=(%p, %p)", i, rpQueImgBuf->getVirAddr(), pStreamImgBuf->getVirAddr());
            continue;
        }
        //  (.3)  Every check is ok. Now remove the node from the list.
        mStreamBufList.erase(mStreamBufList.begin());
        //
        //  (.4)  enquePrvOps/cancelPrvOps
        if  ( i == idxToDisp ) {
            MY_LOGD_IF(
                (1<=miLogLevel), 
                "Show frame:%d %d [ion:%d %p/%d %lld]", 
                i, rvQueNode[i].getStatus(), pStreamImgBuf->getIonFd(), 
                pStreamImgBuf->getVirAddr(), pStreamImgBuf->getBufSize(), pStreamImgBuf->getTimestamp()
            );
            //
            if(mpExtImgProc != NULL)
            {
                if(mpExtImgProc->getImgMask() & ExtImgProc::BufType_Display)
                {
                    IExtImgProc::ImgInfo img;
                    //
                    img.bufType     = ExtImgProc::BufType_Display;
                    img.format      = pStreamImgBuf->getImgFormat();
                    img.width       = pStreamImgBuf->getImgWidth();
                    img.height      = pStreamImgBuf->getImgHeight();
                    img.stride[0]   = pStreamImgBuf->getImgWidthStride(0);
                    img.stride[1]   = pStreamImgBuf->getImgWidthStride(1);
                    img.stride[2]   = pStreamImgBuf->getImgWidthStride(2);
                    img.virtAddr    = (MUINT32)(pStreamImgBuf->getVirAddr());
                    img.bufSize     = pStreamImgBuf->getBufSize();
                    //
                    mpExtImgProc->doImgProc(img);
                }
            }
            //
            enquePrvOps(pStreamImgBuf);
        }
        else {
            MY_LOGW(
                "Drop frame:%d %d [ion:%d %p/%d %lld]", 
                i, rvQueNode[i].getStatus(), pStreamImgBuf->getIonFd(), 
                pStreamImgBuf->getVirAddr(), pStreamImgBuf->getBufSize(), pStreamImgBuf->getTimestamp()
            );
            cancelPrvOps(pStreamImgBuf);
        }
    }
    //
    MY_LOGD_IF((1<=miLogLevel), "-");
    return  true;
}

