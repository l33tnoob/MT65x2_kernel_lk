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
#include "RecordClient.h"
#include "RecBufManager.h"
//
using namespace NSCamClient;
using namespace NSRecordClient;
//


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define FPS_CNT_TIME                (990*1000*1000) //ns
#define EMPTY_QUE_WAIT_TIME         (10*1000) //us
#define NO_ENQUE_WAIT_TIME     (10*1000) //us


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
RecordClient::
initBuffers()
{
    bool ret = false;
    //
    //  (1) Lock
    //Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Allocate buffers.
    if(mpImgBufMgr != 0)
    {
        MY_LOGD("RecBufManager::already alloc()");
        ret = true;
        goto lbExit;
    }
    //
    MY_LOGD("RecBufManager::first time alloc()");
    //
    mpParamsMgr->getVideoSize(&mi4RecWidth, &mi4RecHeight);
    //
    MY_LOGD("+ record: WxH=%dx%d, format(%s)", mi4RecWidth, mi4RecHeight, CameraParameters::PIXEL_FORMAT_YUV420P);
    //
    muImgBufIdx = 0;
    mpImgBufMgr = RecBufManager::alloc(
                    CameraParameters::PIXEL_FORMAT_YUV420P, 
                    mi4RecWidth,
                    mi4RecHeight, 
                    eMAX_RECORD_BUFFER_NUM, 
                    "RecordClientCb",
                    mpCamMsgCbInfo->mRequestMemory,
                    mi4BufSecu,
                    mi4BufCohe);
    if  ( mpImgBufMgr == 0 )
    {
        MY_LOGE("RecBufManager::alloc() fail");
        goto lbExit;
    }
    //
    for (int i = 0; i < eMAX_RECORD_BUFFER_NUM; i++)
    {
        REC_BUF_INFO_STRUCT RecBufInfo;
        RecBufInfo.Sta = REC_BUF_STA_EMPTY;
        RecBufInfo.VirAddr = (void*)(mpImgBufMgr->getBuf(i)->getVirAddr());
        mvRecBufInfo.push_back(RecBufInfo);
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
RecordClient::
uninitBuffers()
{
    //  (1) Lock
    Mutex::Autolock _l(mBufferMtx);
    //
    //  (2) Free buffers.
    if(mpImgBufMgr != 0)
    {
        mvRecBufInfo.clear();
        muImgBufIdx = 0;
        mpImgBufMgr = 0;
    }
    //
    //
    if(mpExtImgProc != NULL)
    {
        mpExtImgProc->uninit();
        mpExtImgProc->destroyInstance();
        mpExtImgProc = NULL;
    }
    //
    mBufferCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue, sp<RecBufManager>const& rpBufMgr)
{
    bool ret = false, findEmpty = false;
    uint32_t i, enqCount = 0, fillCount = 0, cbCount = 0;
    sp<ICameraImgBuf> pCameraImgBuf = NULL;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    for(i = 0; i < mvRecBufInfo.size(); i++)
    {
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_EMPTY)
        {
            pCameraImgBuf = rpBufMgr->getBuf(i);
            MY_LOGD("EQ:idx(%d),VA(0x%08X)",i,(uint32_t)(pCameraImgBuf->getVirAddr()));
            findEmpty = true;
            mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_ENQUE;
            ret = rpBufQueue->enqueProcessor(
                                ImgBufQueNode(pCameraImgBuf, ImgBufQueNode::eSTATUS_TODO));
            //
            if(!ret)
            {
                MY_LOGW("enqueProcessor() fails");
                goto lbExit;
            }
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
        {
            enqCount++;
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_FILL)
        {
            fillCount++;
        }
        else
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_CB)
        {
            cbCount++;
        }
    }
    //
    if(!findEmpty)
    {
        MY_LOGW("No EMPTY buf:Enq(%d),Fill(%d),CB(%d)",
                enqCount,
                fillCount,
                cbCount);
    }
    //
    ret = true;
lbExit:
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
cancelAllUnreturnBuffers()
{
    uint32_t i;
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    for(i = 0; i < mvRecBufInfo.size(); i++)
    {
        if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
        {
            mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_EMPTY; 
        }
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue)
{
    bool ret = false;
    uint32_t i,size;
    Vector<ImgBufQueNode> vQueNode;
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    {
        Mutex::Autolock _l(mModuleMtx);
        //
        size = mvRecBufInfo.size();
        for(i=0; i<size; i++)
        {
            if(mvRecBufInfo[i].Sta == REC_BUF_STA_ENQUE)
            {
                break;
            }
        }
    }
    //
    if(i == size)
    {
        MY_LOGW("No ENQUE buf");
        usleep(NO_ENQUE_WAIT_TIME);
        return ret;
    }
    //
    //  (1) deque buffers from processor.
    rpBufQueue->dequeProcessor(vQueNode);
    if  ( vQueNode.empty() ) {
        MY_LOGW("vQueNode.empty()");
        usleep(EMPTY_QUE_WAIT_TIME);
        goto lbExit;
    }
    //
    //  (2) handle buffers dequed from processor.
    ret = handleReturnBuffers(vQueNode);

lbExit:
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode)
{
    uint32_t i,j;
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");
    //
    //  (1) Lock
    //Mutex::Autolock _l(mModuleMtx);
    //
    //  (2) Remove from List and peform callback, one by one.
    int32_t const queSize = rvQueNode.size();
    for (i = 0; i < queSize; i++)
    {
        if(!(rvQueNode[i].isDONE()))
        {
            MY_LOGW("rvQueNode idx(%d) is not done",i);
            continue;
        }
        ImgBufQueNode const&    rQueNode    = rvQueNode[i];
        sp<IImgBuf>const&       rpQueImgBuf = rQueNode.getImgBuf();     //  ImgBuf in Queue.
        //
        if(rpQueImgBuf == 0)
        {
            MY_LOGW("i(%d):rpQueImgBuf is NULL",i);
            continue;
        }
        //
        {
            Mutex::Autolock _l(mModuleMtx);
            //
            for(j=0; j<mvRecBufInfo.size(); j++)
            {
                if(mvRecBufInfo[j].Sta == REC_BUF_STA_ENQUE)
                {
                    if(mvRecBufInfo[j].VirAddr == rpQueImgBuf->getVirAddr())
                    {
                        mvRecBufInfo.editItemAt(j).Sta = REC_BUF_STA_FILL;
                        break;
                    }
                }
            }
        }
        //
        if(j == mvRecBufInfo.size())
        {
            MY_LOGE("Can't find VA(0x%08X)",(uint32_t)(rpQueImgBuf->getVirAddr()));
            return  false;
        }
        //
        MY_LOGD_IF(
            ENABLE_LOG_PER_FRAME, 
            "CB:i(%d/%d),Idx(%d),Sta(%d),Info(0x%08X/%d.%06d)", 
            i,
            queSize-1,
            j,
            rQueNode.getStatus(),
            (uint32_t)(rpQueImgBuf->getVirAddr()),
            (uint32_t)((rpQueImgBuf->getTimestamp()/1000)/1000000), 
            (uint32_t)((rpQueImgBuf->getTimestamp()/1000)%1000000)
        );
        //
        if(mpExtImgProc != NULL)
        {
            if(mpExtImgProc->getImgMask() & ExtImgProc::BufType_Record)
            {
                IExtImgProc::ImgInfo img;
                //
                img.bufType     = ExtImgProc::BufType_Record;
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
        if(!performRecordCallback(j, mpImgBufMgr->getBuf(j), rQueNode.getCookieDE()))
        {
            Mutex::Autolock _l(mModuleMtx);
            mvRecBufInfo.editItemAt(j).Sta = REC_BUF_STA_EMPTY;
        }
    }
    //
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
performRecordCallback(int32_t bufIdx, sp<ICameraImgBuf>const& pCameraImgBuf, int32_t const msgType)
{
    nsecs_t timeDiff;
    //
    if  ( pCameraImgBuf != 0 )
    {
        //  [1] Dump image if wanted.
        if  ( 0 < mi4DumpImgBufCount )
        {
            MY_LOGD("<dump image> mi4DumpImgBufCount(%d) > 0", mi4DumpImgBufCount);
            Mutex::Autolock _lock(mDumpMtx);
            saveBufToFile(
                String8::format(
                    "%s_%s-(%d)%dx%d_%03d.yuv", 
                    ms8DumpImgBufPath.string(), 
                    pCameraImgBuf->getImgFormat().string(), 
                    pCameraImgBuf->getImgWidthStride(), 
                    pCameraImgBuf->getImgWidth(), 
                    pCameraImgBuf->getImgHeight(), 
                    mi4DumpImgBufIndex
                ), 
                (uint8_t*)pCameraImgBuf->getVirAddr(), 
                pCameraImgBuf->getBufSize()
            );
            ::android_atomic_dec(&mi4DumpImgBufCount);
            ::android_atomic_inc(&mi4DumpImgBufIndex);
        }

        if(!isMsgEnabled())
        {
            MY_LOGW_IF(ENABLE_LOG_PER_FRAME, "No REC CB: isEnabledState(%d), isMsgEnabled(%d)", isEnabledState(), isMsgEnabled());
            return false;
        }
        //
        if(mLastTimeStamp >= pCameraImgBuf->getTimestamp())
        {
            MY_LOGW("TimeStamp:Last(%d.%06d) >= Cur(%d.%06d)",
                    (uint32_t)((mLastTimeStamp/1000)/1000000), 
                    (uint32_t)((mLastTimeStamp/1000)%1000000),
                    (uint32_t)((pCameraImgBuf->getTimestamp()/1000)/1000000), 
                    (uint32_t)((pCameraImgBuf->getTimestamp()/1000)%1000000));
            return false;
        }
        mLastTimeStamp = pCameraImgBuf->getTimestamp();
        //  [2] Callback
        CamProfile profile(__FUNCTION__, "RecordClient");
        ::android_atomic_inc(&mi4CallbackRefCount);
        mi8CallbackTimeInMs = MtkCamUtils::getTimeInMs();
        //
        mFrameCount++;
        mTimeEnd = systemTime();
        timeDiff = mTimeEnd - mTimeStart; 
        if(timeDiff > FPS_CNT_TIME)
        {
            MY_LOGD("FPS(%d)",mFrameCount);
            mFrameCount = 0;
            mTimeStart = mTimeEnd;
        }
        //
        {
            Mutex::Autolock _l(mModuleMtx);
            //
            mvRecBufInfo.editItemAt(bufIdx).Sta = REC_BUF_STA_CB;
        }
        //
        mpCamMsgCbInfo->mDataCbTimestamp(
            (nsecs_t)pCameraImgBuf->getTimestamp(),
            (int32_t)CAMERA_MSG_VIDEO_FRAME,
            pCameraImgBuf->get_camera_memory(), 
            pCameraImgBuf->getBufIndex(), 
            mpCamMsgCbInfo->mCbCookie
        );
        MY_LOGD("CB:FC(%d),VA(0x%08X)",mFrameCount,pCameraImgBuf->getVirAddr());
        //
        ::android_atomic_dec(&mi4CallbackRefCount);
        //profile.print_overtime(10, "mDataCb(%x) - index(%d)", msgType, pCameraImgBuf->getBufIndex());
        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
releaseRecordingFrame(const void *opaque)
{
    uint32_t i,size;
    //
    Mutex::Autolock _lock(mModuleMtx);
    //
    size = mvRecBufInfo.size();
    //
    for(i=0; i<size; i++)
    {
        if(mvRecBufInfo[i].Sta  == REC_BUF_STA_CB)
        {
            if(mvRecBufInfo[i].VirAddr == opaque)
            {
                MY_LOGD("Idx(%d),VA(0x%08X)",
                        i,
                        (uint32_t)opaque);
                mvRecBufInfo.editItemAt(i).Sta = REC_BUF_STA_EMPTY;
                break;
            }
        }
    }
    //
    if(i == size)
    {
        MY_LOGE("Can't find VA(0x%08X)",(uint32_t)opaque);
    }
}


