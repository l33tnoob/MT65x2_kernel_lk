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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include <mtkcam/device/Cam1DeviceBase.h>
//
using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[Cam1DeviceBase::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
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
Cam1DeviceBase::
Cam1DeviceBase(
    String8 const&          rDevName, 
    int32_t const           i4OpenId
)
    : Cam1Device()
    //
    , mpDeviceManager(NULL)
    , mDevName(rDevName)
    , mi4OpenId(i4OpenId)
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    //
    , mpParamsMgr(IParamsManager::createInstance(rDevName, i4OpenId))
    , mpCamAdapter(NULL)
    , mpCamClient()
    , mpDisplayClient()
    , mIsPreviewEnabled(false)
    //
    , mTodoCmdMap()
    , mTodoCmdMapLock()
    //
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
Cam1DeviceBase::
~Cam1DeviceBase()
{
    MY_LOGD("");
    mpDisplayClient.clear();
    mpCamClient.clear();
    mpCamAdapter.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
setDeviceManager(ICamDeviceManager* manager)
{
    mpDeviceManager = manager;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
onInit()
{
    MY_LOGD("+");
    //
    bool    ret = false;
    //
    //  (1) Initialize Parameters.
    if  ( ! mpParamsMgr->init() )
    {
        goto lbExit;
    }
    //
    //  (2) Create & Initialize ICamClient
    mpCamClient = ICamClient::createInstance(mpParamsMgr);
    if  ( mpCamClient == 0 || ! mpCamClient->init() )
    {
        MY_LOGE("mpCamClient(%p)->init() fail", mpCamClient.get());
        goto lbExit;
    }
    //
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
onUninit()
{
    MY_LOGD("+");
    //
    //
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->uninit();
        mpCamClient.clear();
    }
    //
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    //
    //
    mpParamsMgr->uninit();
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
closeDevice()
{
    return  mpDeviceManager->close(this);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
initialize()
{
    if  ( ! onInit() )
    {
        return  -ENODEV;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
uninitialize()
{
    onUninit();
}


/******************************************************************************
 *  Set the notification and data callbacks
 ******************************************************************************/
void
Cam1DeviceBase::
setCallbacks(
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    camera_request_memory get_memory, 
    void*user
)
{
    mpCamMsgCbInfo->mCbCookie       = user;
    mpCamMsgCbInfo->mNotifyCb       = notify_cb;
    mpCamMsgCbInfo->mDataCb         = data_cb;
    mpCamMsgCbInfo->mDataCbTimestamp= data_cb_timestamp;
    mpCamMsgCbInfo->mRequestMemory  = get_memory;
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->setCallbacks(mpCamMsgCbInfo);
    }
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
    }
}


/******************************************************************************
 *  Enable a message, or set of messages.
 ******************************************************************************/
void
Cam1DeviceBase::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->enableMsgType(msgType);
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->enableMsgType(msgType);
    }
}


/******************************************************************************
 *  Disable a message, or a set of messages.
 *
 *  Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
 *  should not rely on its client to call releaseRecordingFrame() to release
 *  video recording frames sent out by the cameral hal before and after the
 *  disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
 *  modify/access any video recording frame after calling
 *  disableMsgType(CAMERA_MSG_VIDEO_FRAME).
 ******************************************************************************/
void
Cam1DeviceBase::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mpCamMsgCbInfo->mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->disableMsgType(msgType);
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->disableMsgType(msgType);
    }
}


/******************************************************************************
 * Query whether a message, or a set of messages, is enabled.
 * Note that this is operates as an AND, if any of the messages
 * queried are off, this will return false.
 ******************************************************************************/
bool
Cam1DeviceBase::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
 *  Set the preview_stream_ops to which preview frames are sent.
 ******************************************************************************/
status_t
Cam1DeviceBase::
setPreviewWindow(preview_stream_ops* window)
{
    MY_LOGI("+ window(%p)", window);
    //
    status_t status = initDisplayClient(window);
    if  ( OK == status && previewEnabled() && mpDisplayClient != 0 )
    {
        status = enableDisplayClient();
    }
    //
    return  status;
}


/******************************************************************************
 *  Start preview mode.
 ******************************************************************************/
status_t
Cam1DeviceBase::
startPreview()
{
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter != 0 && mpCamAdapter->isTakingPicture() )
    {
        MY_LOGE("Capture is not done");
        status = INVALID_OPERATION;
        return  status;
    }
    //
    if  ( previewEnabled() )
    {
        MY_LOGD("Preview already running");
        status = ALREADY_EXISTS;
        return  status;
    }
    //
    if  ( ! onStartPreview() )
    {
        MY_LOGE("onStartPreviewLocked() fail");
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    if  ( mpDisplayClient == 0 )
    {
        MY_LOGD("DisplayClient is not ready.");
    }
    else if ( OK != (status = enableDisplayClient()) )
    {
        goto lbExit;
    }
    //
    if  ( mpCamClient != 0 )
    {
        if  ( ! mpCamClient->startPreview() )
        {
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    //
    //  startPreview in Camera Adapter.
    status = mpCamAdapter->startPreview();
    if  ( OK != status )
    {
        MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    mIsPreviewEnabled = true;
    //
    status = OK;
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup after error");
        //
        if  ( mpCamClient != 0 )
        {
            mpCamClient->stopPreview();
        }
        //
        disableDisplayClient();
    }
    //
    MY_LOGI("- status(%d)", status);
    return  status;
}


/******************************************************************************
 *  Stop a previously started preview.
 ******************************************************************************/
void
Cam1DeviceBase::
stopPreview()
{
    MY_LOGI("+");
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    //
    if  ( ! previewEnabled() )
    {
        MY_LOGD("Preview already stopped, perhaps!");
        MY_LOGD("We still force to clean up again.");
    }
    //
    if  ( mpCamAdapter != 0 )
    {
        if(recordingEnabled())
        {
            stopRecording();
        }
        mpCamAdapter->stopPreview();
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->stopPreview();
    }
    //
    disableDisplayClient();
    //
    //
    onStopPreview();
    //
    //
#if 1
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->waitUntilDrained();
    }
#endif
    //
    //
lbExit:
    //  Always set it to false.
    mIsPreviewEnabled = false;
    MY_LOGI("-");    
}


/******************************************************************************
 *  Returns true if preview is enabled.
 ******************************************************************************/
bool
Cam1DeviceBase::
previewEnabled()
{
    MY_LOGV("mIsPreviewEnabled:%d, mpCamAdapter:%p", mIsPreviewEnabled, mpCamAdapter.get());
    //
    if  ( ! mIsPreviewEnabled )
    {
        return  false;
    }
    //
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->previewEnabled()
        ;
}


/******************************************************************************
 * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
 * message is sent with the corresponding frame. Every record frame must be released
 * by a cameral hal client via releaseRecordingFrame() before the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
 * to manage the life-cycle of the video recording frames, and the client must
 * not modify/access any video recording frames.
 ******************************************************************************/
status_t
Cam1DeviceBase::
startRecording()
{
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //  startRecording in Camera Adapter.
    status = mpCamAdapter->startRecording();
    if  ( OK != status )
    {
        MY_LOGE("startRecording() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    if  ( mpCamClient != 0 )
    {
        //  Get recording format & size.
        //  Set.
        if  ( ! mpCamClient->startRecording() )
        {
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
lbExit:
    return  status;
}


/******************************************************************************
 *  Stop a previously started recording.
 ******************************************************************************/
void
Cam1DeviceBase::
stopRecording()
{
    MY_LOGI("+");
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->stopRecording();
    }
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->stopRecording();
    }
}


/******************************************************************************
 *  Returns true if recording is enabled.
 ******************************************************************************/
bool
Cam1DeviceBase::
recordingEnabled()
{
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->recordingEnabled()
        ;
}


/******************************************************************************
 *  Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
 *
 *  It is camera hal client's responsibility to release video recording
 *  frames sent out by the camera hal before the camera hal receives
 *  a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
 *  the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
 *  responsibility of managing the life-cycle of the video recording
 *  frames.
 ******************************************************************************/
void
Cam1DeviceBase::
releaseRecordingFrame(const void *opaque)
{
    if  ( mpCamClient != 0 )
    {
        mpCamClient->releaseRecordingFrame(opaque);
    }
}


/******************************************************************************
 *  Start auto focus, the notification callback routine is called
 *  with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
 *  will be called again if another auto focus is needed.
 ******************************************************************************/
status_t
Cam1DeviceBase::
autoFocus()
{
    MY_LOGD("+");
    //
    if  ( ! previewEnabled() )
    {
        MY_LOGE("preview is not enabled");
        return INVALID_OPERATION;
    }
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->autoFocus()
        ;
}


/******************************************************************************
 * Cancels auto-focus function. If the auto-focus is still in progress,
 * this function will cancel it. Whether the auto-focus is in progress
 * or not, this function will return the focus position to the default.
 * If the camera does not support auto-focus, this is a no-op.
 ******************************************************************************/
status_t
Cam1DeviceBase::
cancelAutoFocus()
{
    status_t status = ( mpCamAdapter == 0 )
        ?   (status_t)OK
        :   mpCamAdapter->cancelAutoFocus()
        ;
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    return  status;
}


/******************************************************************************
 *  Take a picture.
 ******************************************************************************/
status_t
Cam1DeviceBase::
takePicture()
{
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    if  ( mpCamClient != 0 )
    {
        mpCamClient->takePicture();
    }
    //
    //  takePicture in Camera Adapter.
    status = mpCamAdapter->takePicture();
    if  ( OK != status )
    {
        MY_LOGE("CamAdapter->takePicture() returns: [%s(%d)]", ::strerror(-status), -status);
        goto lbExit;
    }

lbExit:
    return  status;
}


/******************************************************************************
 *  Cancel a picture that was started with takePicture.  Calling this
 *  method when no picture is being taken is a no-op.
 ******************************************************************************/
status_t
Cam1DeviceBase::
cancelPicture()
{
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->cancelPicture()
        ;
}


/******************************************************************************
 * Set the camera parameters. This returns BAD_VALUE if any parameter is
 * invalid or not supported.
 ******************************************************************************/
status_t
Cam1DeviceBase::
setParameters(const char* params)
{
    status_t status = OK;
    //
    //  (1) Update params to mpParamsMgr.
    status = mpParamsMgr->setParameters(String8(params));
    if  ( OK != status ) {
        goto lbExit;
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    {
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
    }

lbExit:
    return  status;
}


/******************************************************************************
 *  Return the camera parameters.
 ******************************************************************************/
char*
Cam1DeviceBase::
getParameters()
{
    String8 params_str8 = mpParamsMgr->flatten();
    // camera service frees this string...
    uint32_t const params_len = sizeof(char) * (params_str8.length()+1);
    char*const params_string = (char*)::malloc(params_len);
    if  ( params_string )
    {
        ::strcpy(params_string, params_str8.string());
    }
    //
    MY_LOGV_IF(0, "- params(%p), len(%d)", params_string, params_len);
    return params_string;
}


/******************************************************************************
 * Put the camera parameters.
 ******************************************************************************/
void
Cam1DeviceBase::
putParameters(char *params)
{
    if  ( params )
    {
        ::free(params);
    }
    MY_LOGV_IF(0, "- params(%p)", params);
}


/******************************************************************************
 * Send command to camera driver.
 ******************************************************************************/
status_t
Cam1DeviceBase::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    status_t status = DEAD_OBJECT;
    //
    switch  (cmd)
    {
    case CAMERA_CMD_PLAY_RECORDING_SOUND:
        return  OK;
    default:
        break;
    }
    //
    //  (1) try to see if Camera Adapter can handle this command.
    sp<ICamAdapter> pCamAdapter = mpCamAdapter;
    if  ( pCamAdapter != 0 && INVALID_OPERATION != (status = pCamAdapter->sendCommand(cmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        return  status;
    }
    //
    //  (2) try to see if Camera Client can handle this command.
    sp<ICamClient> pCamClient = mpCamClient;
    if  ( pCamClient != 0 && INVALID_OPERATION != (status = pCamClient->sendCommand(cmd, arg1, arg2)) )
    {   //  we just return since this cammand has been handled.
        return  status;
    }
    //
    switch  (cmd)
    {
    case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            ssize_t index = mTodoCmdMap.indexOfKey(cmd);
            if  (index < 0)
            {
                MY_LOGD("queue cmd(%#d),args(%d,%d)", cmd, arg1, arg2);
                mTodoCmdMap.add(cmd, CommandInfo(cmd, arg1, arg2));
            }
            else
            {
                MY_LOGW("queue the same cmd(%#d),args(%d,%d) again", cmd, arg1, arg2);
                mTodoCmdMap.editValueAt(index) = CommandInfo(cmd, arg1, arg2);
            }
        }
        status = OK;
        break;
    default:
        MY_LOGW("not handle cmd(%#d),args(%d,%d)", cmd, arg1, arg2);
        break;
    }
    //
    //return  OK;
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height)
{
    mpParamsMgr->getPreviewSize(&ri4Width, &ri4Height);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
Cam1DeviceBase::
initCameraAdapter()
{
    bool ret = false;
    //
    //  (1) Check to see if CamAdapter has existed or not.
    if  ( mpCamAdapter != 0 )
    {
        if  ( ICamAdapter::isValidInstance(mpCamAdapter) )
        {   // do nothing & just return true if the same app.
            MY_LOGD("valid camera adapter: %s", mpCamAdapter->getName());
            ret = true;
            goto lbExit;
        }
        else
        {   // cleanup the original one if different app.
            MY_LOGW("invalid camera adapter: %s", mpCamAdapter->getName());
            mpCamAdapter->uninit();
            mpCamAdapter.clear();
        }
    }
    //
    //  (2) Create & init a new CamAdapter.
    mpCamAdapter = ICamAdapter::createInstance(mDevName, mi4OpenId, mpParamsMgr);
    if  ( mpCamAdapter != 0 && mpCamAdapter->init() )
    {
        //  (.1) init.
        mpCamAdapter->setCallbacks(mpCamMsgCbInfo);
        mpCamAdapter->enableMsgType(mpCamMsgCbInfo->mMsgEnabled);

        //  (.2) Invoke its setParameters
        if  ( OK != mpCamAdapter->setParameters() )
        {
            //  If fail, it should destroy instance before return.
            MY_LOGE("mpCamAdapter->setParameters() fail");
            goto lbExit;
        }

        //  (.3) Send to-do commands.
        {
            Mutex::Autolock _lock(mTodoCmdMapLock);
            for (size_t i = 0; i < mTodoCmdMap.size(); i++)
            {
                CommandInfo const& rCmdInfo = mTodoCmdMap.valueAt(i);
                MY_LOGD("send queued cmd(%#d),args(%d,%d)", rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
                mpCamAdapter->sendCommand(rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
            }
            mTodoCmdMap.clear();
        }

        //  (.4) [DisplayClient] set Image Buffer Provider Client if needed.
        if  ( mpDisplayClient != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpDisplayClient->setImgBufProviderClient() fail");
            goto lbExit;
        }

        //  (.5) [CamClient] set Image Buffer Provider Client if needed.
        if  ( mpCamClient != 0 && ! mpCamClient->setImgBufProviderClient(mpCamAdapter) )
        {
            MY_LOGE("mpCamClient->setImgBufProviderClient() fail");
            goto lbExit;
        }
    }
    else
    {
        MY_LOGE("mpCamAdapter(%p)->init() fail", mpCamAdapter.get());
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
initDisplayClient(preview_stream_ops* window)
{
#if '1'!=MTKCAM_HAVE_DISPLAY_CLIENT
    #warning "Not Build Display Client"
    MY_LOGD("Not Build Display Client");
    return  OK;
#else
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+ window(%p)", window);
    //
    //
    //  [1] Check to see whether the passed window is NULL or not.
    if  ( ! window )
    {
        MY_LOGW("NULL window is passed into...");
        //
        if  ( mpDisplayClient != 0 )
        {
            MY_LOGW("destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
        status = OK;
        goto lbExit;
    }
    //
    //
    //  [2] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    //
    //  [3] Initialize Display Client.
    if  ( mpDisplayClient != 0 )
    {
        if  ( previewEnabled() )
        {
            MY_LOGW("Do nothing since Display Client(%p) is already created after startPreview()", mpDisplayClient.get());
//          This method must be called before startPreview(). The one exception is that 
//          if the preview surface texture is not set (or set to null) before startPreview() is called, 
//          then this method may be called once with a non-null parameter to set the preview surface.
            status = OK;
            goto lbExit;
        }
        else
        {
            MY_LOGW("New window is set after stopPreview or takePicture. Destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
    }
    //  [3.1] create a Display Client.
    mpDisplayClient = IDisplayClient::createInstance();
    if  ( mpDisplayClient == 0 )
    {
        MY_LOGE("Cannot create mpDisplayClient");
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.2] initialize the newly-created Display Client.
    if  ( ! mpDisplayClient->init() )
    {
        MY_LOGE("mpDisplayClient init() failed");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.3] set preview_stream_ops & related window info.
    if  ( ! mpDisplayClient->setWindow(window, previewSize.width, previewSize.height, queryDisplayBufCount()) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //  [3.4] set Image Buffer Provider Client if it exist.
    if  ( mpCamAdapter != 0 && ! mpDisplayClient->setImgBufProviderClient(mpCamAdapter) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    //
    status = OK;
    //
lbExit:
    if  ( OK != status )
    {
        MY_LOGD("Cleanup...");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    MY_LOGD("- status(%d)", status);
    return  status;
#endif//MTKCAM_HAVE_DISPLAY_CLIENT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
Cam1DeviceBase::
enableDisplayClient()
{
    status_t status = OK;
    Size previewSize;
    //
    //  [1] Get preview size.
    if  ( ! queryPreviewSize(previewSize.width, previewSize.height) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    //  [2] Enable
    if  ( ! mpDisplayClient->enableDisplay(previewSize.width, previewSize.height, queryDisplayBufCount(), mpCamAdapter) )
    {
        MY_LOGE("mpDisplayClient(%p)->enableDisplay()", mpDisplayClient.get());
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    status = OK;
lbExit:
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1DeviceBase::
disableDisplayClient()
{
    if  ( mpDisplayClient != 0 )
    {
        mpDisplayClient->disableDisplay();
    }
}

