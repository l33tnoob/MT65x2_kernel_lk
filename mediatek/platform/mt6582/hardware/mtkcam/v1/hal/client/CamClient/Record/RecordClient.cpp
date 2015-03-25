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
#include <binder/IMemory.h>
#include <camera/MtkCamera.h>
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
sp<IRecordClient>
IRecordClient::
createInstance(sp<IParamsManager> pParamsMgr)
{
    return  new RecordClient(pParamsMgr);
}


/******************************************************************************
 *
 ******************************************************************************/
RecordClient::
RecordClient(sp<IParamsManager> pParamsMgr)
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(pParamsMgr)
    , mIsMsgEnabled(0)
    , mIsRecStarted(0)
    , mi4RecWidth(0)
    , mi4RecHeight(0)
    //
    , mi4CallbackRefCount(0)
    , mi8CallbackTimeInMs(0)
    //
    , muImgBufIdx(0)
    , mpImgBufMgr(0)
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mDumpMtx()
    , mbForceReleaseBuf(0)
    , mi4DumpImgBufCount(0)
    , ms8DumpImgBufPath("")
    , mi4DumpImgBufIndex(0)
    , mvRecBufInfo()
    , mTimeStart(0)
    , mTimeEnd(0)
    , mFrameCount(0)
    , mLastTimeStamp(0)
    //
    , mi4BufSecu(0)
    , mi4BufCohe(0)
    //
    , mpExtImgProc(NULL)
    //
{
    MY_LOGD("+ this(%p)", this);
}


/******************************************************************************
 *
 ******************************************************************************/
RecordClient::
~RecordClient()
{
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
init()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_REC_CB, "RecCB@ImgBufQue");
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
RecordClient::
uninit()
{
    MY_LOGD("+");
    //
    //
    if  ( 0 != mi4CallbackRefCount )
    {
        int64_t const i8CurrentTimeInMs = MtkCamUtils::getTimeInMs();
        MY_LOGW(
            "Record Callback: ref count(%d)!=0, the last callback before %lld ms, timestamp:(the last, current)=(%lld ms, %lld ms)", 
            mi4CallbackRefCount, (i8CurrentTimeInMs-mi8CallbackTimeInMs), mi8CallbackTimeInMs, i8CurrentTimeInMs
        );
    }
    //
    //
    if(isEnabledState())
    {
        stopRecording();
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
        #if 0
        status_t status = join();
        if  ( OK != status )
        {
            MY_LOGW("Not to wait thread(tid:%d), status[%s(%d)]", getThreadId(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        #else
        {
            #define WAIT_THREAD_EXIT_TIME   (10*1000)
            #define WAIT_THREAD_EXIT_CNT    (10)
            //
            uint32_t waitThreadExitCnt = 0;
            //
            while(waitThreadExitCnt < WAIT_THREAD_EXIT_CNT)
            {
                MY_LOGD("mbThreadExit(%d),waitThreadExitCnt(%d)",mbThreadExit,waitThreadExitCnt);
                if(mbThreadExit)
                {
                    break;
                }
                //
                waitThreadExitCnt++;
                usleep(WAIT_THREAD_EXIT_TIME);
            }
        }
        #endif
    }
    //To prevent that user CAMERA_CMD_GET_REC_BUF_INFO but not start and stop recording.
    uninitBuffers();
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
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
RecordClient::
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
RecordClient::
startRecording()
{
    bool ret = false; 
    //
    MY_LOGD("+");
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    if(isEnabledState())
    {
        MY_LOGE("Recording has been started");
        goto lbExit;
    }
    //
    MY_LOGD("+ current mIsRecStarted=%d", mIsRecStarted);
    ::android_atomic_write(1, &mIsRecStarted);
    //
    mpParamsMgr->getVideoSize(&mi4RecWidth, &mi4RecHeight);
    MY_LOGD("+ record: WxH=%dx%d, format(%s)", mi4RecWidth, mi4RecHeight, CameraParameters::PIXEL_FORMAT_YUV420P);
    //
    mTimeStart = systemTime();
    mTimeEnd = mTimeStart;
    mFrameCount = 0;
    mLastTimeStamp = 0;
    //
    ret = onStateChanged();
    //
lbExit:
    //
    MY_LOGD("-");
    //
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
stopRecording()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGD("+");
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    if(!isEnabledState())
    {
        MY_LOGE("Recording has been stopped");
        goto lbExit;
    }
    //
    MY_LOGD("getThreadId(%d), getStrongCount(%d), this(%p)", getThreadId(), getStrongCount(), this);
    //
    MY_LOGD("+ current mIsRecStarted=%d", mIsRecStarted);
    ::android_atomic_write(0, &mIsRecStarted);
    //
    ret = onStateChanged();
    //
    mpImgBufQueue->pauseProcessor();
    //
lbExit:
    //
    MY_LOGD("-");
    //
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecordClient::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
isMsgEnabled()
{
    return  CAMERA_MSG_VIDEO_FRAME == (CAMERA_MSG_VIDEO_FRAME & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RecordClient::
isEnabledState() const
{
    return  0 != ::android_atomic_release_load(&mIsRecStarted);
}


/******************************************************************************
 *
 ******************************************************************************/
//  enable if both record started && message enabled; otherwise disable.
bool
RecordClient::
onStateChanged()
{
    bool ret = true;
    //
    if  ( isEnabledState() )
    {
        postCommand(Command(Command::eID_WAKEUP));
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
RecordClient::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    bool ret = false;
    //
    MY_LOGD("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)",cmd,arg1,arg2);
    //
    switch(cmd)
    {
        case CAMERA_CMD_GET_REC_BUF_INFO:
        {
            uint32_t i;
            sp<ICameraImgBuf> pImgBuf = NULL;
            CameraRecBufInfo IonBufInfo;
            Vector<CameraRecBufInfo>* pVecIonBufInfo = (Vector<CameraRecBufInfo>*)arg1;
            CameraRecSetting* pRecSetting = (CameraRecSetting*)arg2;
            //
            if(pVecIonBufInfo == NULL)
            {
                MY_LOGE("pVecIonBufInfo is NULL");
                break;
            }
            //
            if(pRecSetting == 0)
            {
                mi4BufSecu = 0;
                mi4BufCohe = 0;
            }
            else
            {
                mi4BufSecu = pRecSetting->mi4BufSecu;
                mi4BufCohe = pRecSetting->mi4BufCohe;
            }
            //
            {
                Mutex::Autolock _l(mBufferMtx);
                MY_LOGD("Buffer size(%d)",mvRecBufInfo.size());
                if(mvRecBufInfo.size() > 0)
                {
                    MY_LOGW("Buffer is not released, wait to release buffer!");
                    android_atomic_write(1, &mbForceReleaseBuf);
                    mBufferCond.wait(mBufferMtx);
                    MY_LOGW("Finish to release buffer!");
                }
            }
            //
            Mutex::Autolock _l(mModuleMtx);
            if(initBuffers())
            {
                if(mpImgBufMgr == NULL)
                {
                    MY_LOGE("mpImgBufMgr is NULL");
                    break;
                }
                //
                for(i=0; i<eMAX_RECORD_BUFFER_NUM; i++)
                {
                    pImgBuf = mpImgBufMgr->getBuf(i);
                    IonBufInfo.i4MemId = mpImgBufMgr->getBufIonFd(i);
                    IonBufInfo.u4VirAddr = (uint32_t)(pImgBuf->getVirAddr());
                    IonBufInfo.u4Size = pImgBuf->getBufSize();
                    pVecIonBufInfo->push_back(IonBufInfo);
                }
            }
            MY_LOGD("CAMERA_CMD_GET_REC_BUF_INFO:size(%d),S/C(%d/%d)",
                    pVecIonBufInfo->size(),
                    mi4BufSecu,
                    mi4BufCohe);
            ret = true;
            break;
        }
        default:
        {
            MY_LOGD("No implement cmd(0x%08X)",cmd);
            break;
        }
    }
    //
    return ret? OK : INVALID_OPERATION;
}


