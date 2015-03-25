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

#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "FDClient.h"
#include "camera/MtkCamera.h"
//
using namespace NSCamClient;
using namespace NSFDClient;
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
sp<IFDClient>
IFDClient::
createInstance(sp<IParamsManager> pParamsMgr)
{
    return  new FDClient(pParamsMgr);
}


/******************************************************************************
 *
 ******************************************************************************/
FDClient::
FDClient(sp<IParamsManager> pParamsMgr)
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(pParamsMgr)
    , mIsFDStarted(0)
    //
    , mi4CallbackRefCount(0)
    , mi8CallbackTimeInMs(0)
    //
    , mpImgBufQueue(NULL)
    , mpImgBufPvdrClient(NULL)
    //
    , mpDetectedFaces(NULL) 
    , mIsDetected_FD(false)
    , mIsDetected_SD(false)
    , mIsSDenabled(false)
{
    MY_LOGD("+ this(%p)", this);
}


/******************************************************************************
 *
 ******************************************************************************/
FDClient::
~FDClient()
{
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
init()
{
    MY_LOGD("+");
    bool ret = true;
    //
    mpImgBufQueue = new ImgBufQueue(IImgBufProvider::eID_FD, "FDBuf@ImgBufQue");
    if  ( mpImgBufQueue == 0 )
    {
        MY_LOGE("Fail to new ImgBufQueue");
        ret = false;
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD("-");
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
uninit()
{
    MY_LOGD("+");

    {
        MY_LOGD("getThreadId(%d), getStrongCount(%d), this(%p)", getThreadId(), getStrongCount(), this);
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        ::android_atomic_write(0, &mIsFDStarted);        
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
    sp<IImgBufQueue> pImgBufQueue;
    {
        //Mutex::Autolock _l(mModuleMtx);
        pImgBufQueue = mpImgBufQueue;
    }
    //    
    if  ( pImgBufQueue != 0 )
    {
        pImgBufQueue->stopProcessor();
        pImgBufQueue = NULL;
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
FDClient::
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
FDClient::
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
void
FDClient::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
 *
 ******************************************************************************/
void
FDClient::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
isMsgEnabled()
{
    return  CAMERA_MSG_PREVIEW_METADATA == (CAMERA_MSG_PREVIEW_METADATA & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
onStateChanged()
{
    bool ret = true;
    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //
    if  ( isEnabledState() )
    {
        status_t status = run();
        if ( INVALID_OPERATION == status)
        {
            MY_LOGW("run(): FD is running");
        }
        else if ( OK != status )
        {
            MY_LOGE("Fail to run thread, status[%s(%d)]", ::strerror(-status), -status);
            ret = false;
            goto lbExit;
        }
        //    
        postCommand(Command::eID_WAKEUP);
    }
    else
    {
        if  ( mpImgBufQueue != 0 )
        {
            mpImgBufQueue->pauseProcessor();
        }
    }    
    //
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
FDClient::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    bool ret = false; 

    MY_LOGD("cmd(0x%x) +", cmd);
    
    switch  (cmd)
    {
    //
    case CAMERA_CMD_START_FACE_DETECTION:
        ret = startFaceDetection();
        break;
        
    case CAMERA_CMD_STOP_FACE_DETECTION:
        ret = stopFaceDetection();
        break;
        
    case CAMERA_CMD_START_SD_PREVIEW:
        mIsSDenabled = true;
        ret = true;
        break;
        
    case CAMERA_CMD_CANCEL_SD_PREVIEW:
        mIsSDenabled = false;
        ret = true;
        break;


    default:
        break;
    }

    MY_LOGD("-");

    return ret? OK : INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
startFaceDetection()
{ 
    MY_LOGD("+");
    bool ret = true;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //
    if  ( mpImgBufPvdrClient != 0 && ! mpImgBufPvdrClient->onImgBufProviderCreated(mpImgBufQueue) )
    {
        MY_LOGE("onImgBufProviderCreated failed");
        ret = false;
        goto lbExit;
    }
    //
    if ( !isEnabledState() )
    {
        ::android_atomic_write(1, &mIsFDStarted);
        ret = onStateChanged();
    }
    else 
    {
        MY_LOGW("FD is running");
        ret = false;
    }
    //
    MY_LOGD("-");    
    //
lbExit:    
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
stopFaceDetection()
{
    MY_LOGD("+");
    bool ret = true;
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    //
    if  ( mpImgBufPvdrClient != 0 )
    {
        mpImgBufPvdrClient->onImgBufProviderDestroyed(mpImgBufQueue->getProviderId());
    }
    //
    MY_LOGD("isEnabledState(%d) +", isEnabledState());
    //
    //
    if ( isEnabledState() )
    {
        ::android_atomic_write(0, &mIsFDStarted);
        ret = onStateChanged();
    }
    else 
    {
        MY_LOGW("FD was not running");
        ret = false;   
    }
    //
    MY_LOGD("-");
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
stopPreview()
{
    return stopFaceDetection();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::takePicture()
{
    return stopFaceDetection();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
isEnabledState()
{
    return  0 != ::android_atomic_release_load(&mIsFDStarted);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
performCallback(bool isDetected_FD, bool isDetected_SD)
{
    bool ret = true;

    
    // (1) FD callback:
    //     (-) always do whenever face is detected    
    //     (-) do when face is not detected, but last time was detected
    //     (-) otherwise, do no-op
    bool performFDcb = isDetected_FD ? true : mIsDetected_FD ? true : false;

    if (performFDcb && isMsgEnabled())
    {    
        camera_memory_t* dummyBuffer = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, NULL);        
        if  ( dummyBuffer )
        {
            camera_frame_metadata_t retFaces;
            retFaces.number_of_faces = mpDetectedFaces->number_of_faces;
            retFaces.faces = reinterpret_cast<camera_face_t*>(mpDetectedFaces->faces);
                
            mpCamMsgCbInfo->mDataCb(
                CAMERA_MSG_PREVIEW_METADATA, 
                dummyBuffer,
                0,
                &retFaces,
                mpCamMsgCbInfo->mCbCookie
            );

            dummyBuffer->release(dummyBuffer);

            mIsDetected_FD = isDetected_FD;
        }
    }
    else
    {
        ret = false;
        MY_LOGW_IF(ENABLE_LOG_PER_FRAME, "No FD CB: isDetected_FD(%d), mIsDetected_FD(%d), isMsgEnabled(%d)", isDetected_FD, mIsDetected_FD, isMsgEnabled());
    } 

    // (2) SD callback:
    
     mIsDetected_SD = isDetected_SD;
    
    bool performSDcb = mIsDetected_SD && mIsSDenabled;
    if (performSDcb && isMsgEnabled())
    {
        mpCamMsgCbInfo->mNotifyCb(
            MTK_CAMERA_MSG_EXT_NOTIFY, 
            MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT, 
            0,
            mpCamMsgCbInfo->mCbCookie
        );
    }
    else
    {
        ret = false;
        MY_LOGW_IF(ENABLE_LOG_PER_FRAME, "No SD CB: isDetected_SD(%d), mIsSDenabled(%d), isMsgEnabled(%d)", mIsDetected_SD, mIsSDenabled, isMsgEnabled());
    } 
    return ret;
}

