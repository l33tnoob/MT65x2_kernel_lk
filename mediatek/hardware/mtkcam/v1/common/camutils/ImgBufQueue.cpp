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

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#include "Local.h"


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%s)[%s] "fmt, getQueName(), __FUNCTION__, ##arg)
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
namespace
{


char const*
ImgBufQueNodeStatus2Name(int32_t const iStatus)
{
    switch  (iStatus)
    {
    case ImgBufQueNode::eSTATUS_TODO:
        return  "eSTATUS_TODO";
    case ImgBufQueNode::eSTATUS_DONE:
        return  "eSTATUS_DONE";
    case ImgBufQueNode::eSTATUS_CANCEL:
        return  "eSTATUS_CANCEL";
    default:
        break;
    }
    return  "eSTATUS_UNKNOWN";
}


};


/******************************************************************************
*
*******************************************************************************/
ImgBufQueue::
ImgBufQueue(
    int32_t const i4QueueId, 
    char const*const pszQueueName
)
    : mi4QueueId(i4QueueId)
    , mpszQueueName(pszQueueName)
    //
    , mTodoImgBufQue()
    , mTodoImgBufQueMtx()
    //
    , mDoneImgBufQue()
    , mDoneImgBufQueMtx()
    , mDoneImgBufQueCond()
    //
    , mbIsProcessorRunning(false)
{
}


/******************************************************************************
*   REPEAT:[ dequeProvider() -> enqueProvider() ]
*   dequeProvider() returns false immediately if empty.
*******************************************************************************/
bool
ImgBufQueue::
dequeProvider(ImgBufQueNode& rNode)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mTodoImgBufQueMtx);
    //
    if  ( ! mTodoImgBufQue.empty() )
    {
        //  If the queue is not empty, take the first buffer from the queue.
        ret = true;
        rNode = *mTodoImgBufQue.begin();
        mTodoImgBufQue.erase(mTodoImgBufQue.begin());
    }
    else
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Empty Que");
    }
    //
    return ret;
}


/******************************************************************************
*   REPEAT:[ dequeProvider() -> enqueProvider() ]
*   dequeProvider() returns false immediately if empty.
*******************************************************************************/
bool
ImgBufQueue::
enqueProvider(ImgBufQueNode const& rNode)
{
    if  ( ! rNode ) {
        MY_LOGW("buffer is NULL");
        return  false;
    }
    //
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    //
    MY_LOGV_IF(
        ENABLE_LOG_PER_FRAME, 
        "+ Que.size(%d); (CookieED/CookieDE)=(%p,%p) %s; Buffer[%s@0x%08X@%d@%s@(%d)%dx%d@%d@Timestamp(%lld)]", 
        mDoneImgBufQue.size(), 
        rNode.getCookieED(), rNode.getCookieDE(), ImgBufQueNodeStatus2Name(rNode.getStatus()), 
        rNode->getBufName(), rNode->getVirAddr(), rNode->getBufSize(), rNode->getImgFormat().string(), 
        rNode->getImgWidthStride(), rNode->getImgWidth(), rNode->getImgHeight(), rNode->getBitsPerPixel(), rNode->getTimestamp()
    );
    //
    mDoneImgBufQue.push_back(rNode);
    mDoneImgBufQueCond.broadcast();
    //
    return  true;
}


/******************************************************************************
 *  Arguments:
 *      rNode
 *          [I] If this function returns true, rNode is a copy of the first
 *          node in the queue. Unlike dequeProvider(), the first node is not
 *          removed from the queue.
 *  Return:
 *      false if the queue is empty.
 *      true if the queue is not empty.
*******************************************************************************/
bool
ImgBufQueue::
queryProvider(ImgBufQueNode& rNode)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mTodoImgBufQueMtx);
    //
    if  ( ! mTodoImgBufQue.empty() )
    {
        //  If the queue is not empty, return a copy of the first buffer.
        ret = true;
        rNode = *mTodoImgBufQue.begin();
    }
    else
    {
        MY_LOGD_IF(0, "Empty Que");
    }
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufQueue::
startProcessor()
{
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    //
    mbIsProcessorRunning = true;
    mDoneImgBufQueCond.broadcast();
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufQueue::
pauseProcessor()
{
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    //
    mbIsProcessorRunning = false;
    mDoneImgBufQueCond.broadcast();
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufQueue::
stopProcessor()
{
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    //
    mbIsProcessorRunning = false;
    mDoneImgBufQueCond.broadcast();
    //
    if  ( ! mDoneImgBufQue.empty() ) {
        MY_LOGW("intent to clear Done Que: size(%d)!=0", mDoneImgBufQue.size());
        mDoneImgBufQue.clear();
    }
    //
    return  true;
}


/******************************************************************************
*   It returns false if Processor is Running; call pauseProcessor() firstly
*   before flushProcessor().
*******************************************************************************/
bool
ImgBufQueue::
flushProcessor()
{
    Mutex::Autolock _lock_DONE(mDoneImgBufQueMtx);   //  + lock DONE QUE
    //
    if  ( mbIsProcessorRunning )
    {
        MY_LOGW("IsProcessorRunning=1; please pause it before calling this function.");
        return  false;
    }
    //
    //
    Mutex::Autolock _lock_TODO(mTodoImgBufQueMtx);   //  + lock TODO QUE
    //
    MY_LOGD("TODO Que.size(%d)", mTodoImgBufQue.size());
    for ( List<ImgBufQueNode>::iterator it = mTodoImgBufQue.begin(); it != mTodoImgBufQue.end(); it++ )
    {
        (*it).setStatus(ImgBufQueNode::eSTATUS_CANCEL);
        MY_LOGD_IF(
            ENABLE_LOG_PER_FRAME, 
            "%s: (CookieED/mi4CookieDE)=(%p/%p); Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]", 
            ImgBufQueNodeStatus2Name((*it).getStatus()), (*it).getCookieED(), (*it).getCookieDE(),  
            (*it)->getBufName(), (*it)->getVirAddr(), (*it)->getBufSize(), (*it)->getImgFormat().string(), 
            (*it)->getImgWidthStride(), (*it)->getImgWidth(), (*it)->getImgHeight(), 
            (*it)->getBitsPerPixel(), (*it)->getTimestamp()
        );
        mDoneImgBufQue.push_back(*it);
    }
    //
    mTodoImgBufQue.clear();
    //
    //
    return  true;
}

bool
ImgBufQueue::
isProcessorRunning()
{
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    return mbIsProcessorRunning;
}
/******************************************************************************
*
*******************************************************************************/
bool
ImgBufQueue::
enqueProcessor(ImgBufQueNode const& rNode)
{
    if  ( ! rNode ) {
        MY_LOGW("buffer is NULL");
        return  false;
    }
    //
    Mutex::Autolock _lock(mTodoImgBufQueMtx);
    //
    MY_LOGV_IF(
        ENABLE_LOG_PER_FRAME, 
        "+ Que.size(%d); %s: (CookieED/CookieDE)=(%p/%p); Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]", 
        mTodoImgBufQue.size(), ImgBufQueNodeStatus2Name(rNode.getStatus()), rNode.getCookieED(), rNode.getCookieDE(), 
        rNode->getBufName(), rNode->getVirAddr(), rNode->getBufSize(), rNode->getImgFormat().string(), 
        rNode->getImgWidthStride(), rNode->getImgWidth(), rNode->getImgHeight(), 
        rNode->getBitsPerPixel(), rNode->getTimestamp()
    );
    //
    mTodoImgBufQue.push_back(rNode);
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufQueue::
dequeProcessor(Vector<ImgBufQueNode>& rvNode)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mDoneImgBufQueMtx);
    //
    while   ( mDoneImgBufQue.empty() && mbIsProcessorRunning )
    {
        status_t status = mDoneImgBufQueCond.wait(mDoneImgBufQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), Que.size(%d), IsProcessorRunning(%d)", status, mDoneImgBufQue.size(), mbIsProcessorRunning);
        }
    }
    //
    if  ( ! mDoneImgBufQue.empty() )
    {
        //  If the queue is not empty, deque all buffers from the queue.
        ret = true;
        rvNode = mDoneImgBufQue;
        mDoneImgBufQue.clear();
    }
    else
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Empty Que");
        rvNode.clear();
    }
    //
    return ret;
}

