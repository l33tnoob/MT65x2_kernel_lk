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
bool
PreviewClient::
initBuffers()
{
    bool ret = false;
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Allocate buffers.
    muImgBufIdx = 0;
    mpImgBufMgr = ImgBufManager::alloc(
        ms8PrvTgtFmt, 
        mi4PrvWidth, mi4PrvHeight, 
        eMAX_PREVIEW_BUFFER_NUM, 
        "PreviewClientCb", mpCamMsgCbInfo->mRequestMemory
    );
    if  ( mpImgBufMgr == 0 )
    {
        MY_LOGE("ImgBufManager::alloc() fail");
        goto lbExit;
    }
    //
    //
    mpExtImgProc = ExtImgProc::createInstance();
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->init();
    }
    //
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
uninitBuffers()
{
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Free buffers.
    muImgBufIdx = 0;
    mpImgBufMgr = 0;
    //
    //
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->uninit();
        mpExtImgProc->destroyInstance();
        mpExtImgProc = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue, sp<ImgBufManager>const& rpBufMgr)
{
    //  For better performance, we should enque all TODO buffers before starting.
    //  In that case, we could deque more than one buffers, and should enque
    //  them in the loop.
    bool ret = false;
    //
    MY_LOGD_IF((2<=miLogLevel), "+");
    //
    //  (1) Determine how many buffers to enque.
    int iEnqCount = 0;
    {
        //  all buffers = callback buffers + enq buffers (in list) + not-enq buffers (others)
        //  Assumption:
        //  (1) callback is returned in order.
        //  (2) all callback are returned within this function, 
        //      ==> not-enq buffers (others) = eMAX_PREVIEW_BUFFER_NUM - mImgBufList.size()
        Mutex::Autolock _l(mModuleMtx);

        iEnqCount = eMAX_PREVIEW_BUFFER_NUM - mImgBufList.size();
    }
    //
    //
    sp<ICameraImgBuf> pCameraImgBuf = NULL;
    for (int i = 0; i < iEnqCount; i++)
    {
        //  (.1) Determine which buffer to enque.
        {
            Mutex::Autolock _l(mModuleMtx);
            pCameraImgBuf = rpBufMgr->getBuf(muImgBufIdx);
            muImgBufIdx = (muImgBufIdx+1) % eMAX_PREVIEW_BUFFER_NUM;
        }
        //
        //  (.2) enque it into Processor
        ret = rpBufQueue->enqueProcessor(
            ImgBufQueNode(pCameraImgBuf, ImgBufQueNode::eSTATUS_TODO)
        );
        if  ( ! ret ) {
            MY_LOGW("enqueProcessor() fails");
            goto lbExit;
        }
        //
        //  (.3) enque it into List & increment the list size.
        {
            Mutex::Autolock _l(mModuleMtx);
            mImgBufList.push_back(pCameraImgBuf);
        }
    }
    //
    ret = true;
lbExit:
    MY_LOGD_IF((2<=miLogLevel), "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
cancelAllUnreturnBuffers()
{
    MY_LOGD_IF((1<=miLogLevel), "+");
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    while   ( ! mImgBufList.empty() )
    {
        MY_LOGD("mImgBufList.size(%d)", mImgBufList.size());
        mImgBufList.clear();
    }
    //
    MY_LOGD_IF((1<=miLogLevel), "-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
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
    //
    //  (2) handle buffers dequed from processor.
    ret = handleReturnBuffers(vQueNode);

lbExit:
    //
    MY_LOGD_IF((2<=miLogLevel), "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreviewClient::
handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode)
{
    //
    //  (1) determine the index of the latest DONE buffer for callback.
    int32_t idxToCallback = 0;
    for ( idxToCallback = rvQueNode.size()-1; idxToCallback >= 0; idxToCallback-- )
    {
        if  ( rvQueNode[idxToCallback].isDONE() )
            break;
    }
    if  ( rvQueNode.size() > 1 )
    {
        MY_LOGW("(%d) preview callback frame count > 1 --> select %d to callback", rvQueNode.size(), idxToCallback);
    }
    //
    //  Show Time duration.
    if  ( 0 <= idxToCallback )
    {
        nsecs_t const _timestamp1 = rvQueNode[idxToCallback].getImgBuf()->getTimestamp();
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
            (_msDuration_buffer_timestamp > 34 ) ? "34ms<Duration" : "", _msDuration_buffer_timestamp, 
            (_msDuration_dequeProcessor > 34) ? "34ms<Duration" : "", _msDuration_dequeProcessor
        );
    }
    //
    //  (2) Remove from List and peform callback, one by one.
    int32_t const queSize = rvQueNode.size();
    for (int32_t i = 0; i < queSize; i++)
    {
        ImgBufQueNode const&    rQueNode    = rvQueNode[i];
        sp<IImgBuf>const&       rpQueImgBuf = rQueNode.getImgBuf();     //  ImgBuf in Queue.
        sp<ICameraImgBuf>       pListImgBuf = NULL;
        {
            Mutex::Autolock _l(mModuleMtx);
            //
            ImgBufNode const    ListNode    = *mImgBufList.begin();     //  Node in List.
            pListImgBuf = ListNode.getImgBuf();     //  ImgBuf in List.
            //  (.1)  Check valid pointers to image buffers in Queue & List
            if  ( rpQueImgBuf == 0 || pListImgBuf == 0 )
            {
                MY_LOGW("Bad ImgBuf:(Que[%d], List.begin)=(%p, %p)", i, rpQueImgBuf.get(), pListImgBuf.get());
                continue;
            }
            //  (.2)  Check the equality of image buffers between Queue & List.
            if  ( rpQueImgBuf->getVirAddr() != pListImgBuf->getVirAddr() )
            {
                MY_LOGW("Bad address in ImgBuf:(Que[%d], List.begin)=(%p, %p)", i, rpQueImgBuf->getVirAddr(), pListImgBuf->getVirAddr());
                continue;
            }
            //  (.3)  Every check is ok. Now remove the node from the list.
            mImgBufList.erase(mImgBufList.begin());
        }
        //
        //  (.4)  Perform callback.
        if  ( i == idxToCallback ) {
            //
            if(mpExtImgProc != NULL)
            {
                if(mpExtImgProc->getImgMask() & ExtImgProc::BufType_PreviewCB)
                {
                    IExtImgProc::ImgInfo img;
                    //
                    img.bufType     = ExtImgProc::BufType_PreviewCB;
                    img.format      = rpQueImgBuf->getImgFormat();
                    img.width       = rpQueImgBuf->getImgWidth();
                    img.height      = rpQueImgBuf->getImgHeight();
                    img.stride[0]   = rpQueImgBuf->getImgWidthStride(0);
                    img.stride[1]   = rpQueImgBuf->getImgWidthStride(1);
                    img.stride[2]   = rpQueImgBuf->getImgWidthStride(2);
                    img.virtAddr    = (MUINT32)(rpQueImgBuf->getVirAddr());
                    img.bufSize     = rpQueImgBuf->getBufSize();
                    //
                    mpExtImgProc->doImgProc(img);
                }
            }
            //
            MY_LOGD_IF(
                (1<=miLogLevel), 
                "callback:%d(%d) [%p/%d %#x] CookieDE:%#x", 
                i, rQueNode.getStatus(), pListImgBuf->getVirAddr(), 
                pListImgBuf->getBufSize(), pListImgBuf->getTimestamp(), 
                rQueNode.getCookieDE()
            );
            performPreviewCallback(pListImgBuf, rQueNode.getCookieDE());
        }
    }
    //
    MY_LOGD_IF((1<=miLogLevel), "-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreviewClient::
performPreviewCallback(sp<ICameraImgBuf>const& pCameraImgBuf, int32_t const msgType)
{
    if  ( pCameraImgBuf != 0 )
    {
        mProfile_callback.pulse();
        if  ( mProfile_callback.getDuration() >= ::s2ns(2) ) {
            mProfile_callback.updateFps();
            mProfile_callback.showFps();
            mProfile_callback.reset();
        }

        //  [1] Dump image if wanted.
        dumpImgBuf_If(pCameraImgBuf);

        //
        //  [2] Callback
        CamProfile profile(__FUNCTION__, "PreviewClient");
        //
        sp<CamMsgCbInfo> pCamMsgCbInfo;
        {
            Mutex::Autolock _l(mModuleMtx);
            pCamMsgCbInfo = mpCamMsgCbInfo;
            mi8CallbackTimeInMs = MtkCamUtils::getTimeInMs();
        }
        //
        ::android_atomic_inc(&mi4CallbackRefCount);
        pCamMsgCbInfo->mDataCb(
            0 != msgType ? msgType : (int32_t)CAMERA_MSG_PREVIEW_FRAME, 
            pCameraImgBuf->get_camera_memory(), 
            pCameraImgBuf->getBufIndex(), 
            NULL, 
            pCamMsgCbInfo->mCbCookie
        );
        ::android_atomic_dec(&mi4CallbackRefCount);

        profile.print_overtime(10, "mDataCb(%x) - index(%d)", msgType, pCameraImgBuf->getBufIndex());
    }
}

