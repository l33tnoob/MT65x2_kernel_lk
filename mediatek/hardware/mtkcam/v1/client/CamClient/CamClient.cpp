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

#define LOG_TAG "MtkCam/CamClient"
//
#include "MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include <mtkcam/v1/IParamsManager.h>
//
#include <mtkcam/v1/client/IPreviewClient.h>
#include <mtkcam/v1/client/IRecordClient.h>
#include <mtkcam/v1/client/IFDClient.h>
#if '1'==MTKCAM_HAVE_OT_CLIENT
#include <mtkcam/v1/client/IOTClient.h>
#endif
#include <mtkcam/v1/client/IPreviewFeatureClient.h>
//
#include <mtkcam/v1/ICamClient.h>
#include "CamClient.h"
#include <cutils/properties.h>
//
using namespace NSCamClient;
//


/******************************************************************************
 *
 ******************************************************************************/
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
sp<ICamClient>
ICamClient::
createInstance(sp<IParamsManager> pParamsMgr)
{
    return new CamClient(pParamsMgr);
}


/******************************************************************************
 *
 ******************************************************************************/
CamClient::
CamClient(sp<IParamsManager> pParamsMgr)
    : ICamClient()
    //
    , mModuleMtx()
    , mpCamMsgCbInfo(new CamMsgCbInfo)
    , mpParamsMgr(pParamsMgr)
    , mpImgBufPvdrClient(NULL)
    //
    , mpPreviewClient(NULL)
    , mpRecordClient(NULL)
    , mpFDClient(NULL)
#if '1'==MTKCAM_HAVE_OT_CLIENT    
    , mpOTClient(NULL)
#endif
    , mpPreviewFeatureClient(NULL)
    //
{
    MY_LOGD("this(%p)", this);
}


/******************************************************************************
 *
 ******************************************************************************/
CamClient::
~CamClient()
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamClient::
init()
{
    bool    ret = false;
    //
    MY_LOGD("+");
    //
    {
        Mutex::Autolock _l(mModuleMtx);
        //
#if '1'==MTKCAM_HAVE_PREVIEWCALLBACK_CLIENT
        mpPreviewClient = IPreviewClient::createInstance(mpParamsMgr);
        if  ( mpPreviewClient == 0 || ! mpPreviewClient->init() )
        {
            ret = false;
            goto lbExit;
        }
#endif
        //
#if '1'==MTKCAM_HAVE_RECORD_CLIENT
        mpRecordClient = IRecordClient::createInstance(mpParamsMgr);
        if  ( mpRecordClient == 0 || ! mpRecordClient->init() )
        {
            ret = false;
            goto lbExit;
        }
#endif
        //
#if '1'==MTKCAM_HAVE_FD_CLIENT
        //***********Binchang 20121219 Add FD on/off Opition****************//
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        ::property_get("debug.camera.fd.switch", value, "0");
        int FDSwitchOPT = ::atoi(value);
        if  ( FDSwitchOPT == 0 )
        {
            mpFDClient = IFDClient::createInstance(mpParamsMgr);
            if  ( mpFDClient == 0 || ! mpFDClient->init() )
            {
                ret = false;
                goto lbExit;
            }
        }
#endif
#if '1'==MTKCAM_HAVE_OT_CLIENT
        mpOTClient = IOTClient::createInstance(mpParamsMgr);
        if  ( mpOTClient == 0 || ! mpOTClient->init() )
        {
            ret = false;
            goto lbExit;
        }
#endif

        //
#if '1'==MTKCAM_HAVE_PREVIEWFEATURE_CLIENT
        mpPreviewFeatureClient = IPREFEATUREClient::createInstance(mpParamsMgr);
        if  ( mpPreviewFeatureClient == 0 || ! mpPreviewFeatureClient->init() )
        {
            ret = false;
            goto lbExit;
        }
#endif
    }
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
CamClient::
uninit()
{
    MY_LOGI("+ getStrongCount(%d)", getStrongCount());
    //
    //
    Mutex::Autolock _l(mModuleMtx);
    //
    if  ( mpPreviewClient != 0 )
    {
        mpPreviewClient->uninit();
        mpPreviewClient = NULL;
    }
    //
    if  ( mpRecordClient != 0 )
    {
        mpRecordClient->uninit();
        mpRecordClient = NULL;
    }
    //
    if  ( mpFDClient != 0 )
    {
        mpFDClient->uninit();
        mpFDClient = NULL;
    }
#if '1'==MTKCAM_HAVE_OT_CLIENT    
    if  ( mpOTClient != 0 )
    {
        mpOTClient->uninit();
        mpOTClient = NULL;
    }    
#endif    
    //
    if  ( mpPreviewFeatureClient != 0 )
    {
        mpPreviewFeatureClient->uninit();
        mpPreviewFeatureClient = NULL;
    }

    mpImgBufPvdrClient = NULL;
    //
    MY_LOGI("- getStrongCount(%d)", getStrongCount());
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamClient::
setImgBufProviderClient(sp<IImgBufProviderClient>const& rpClient)
{
    bool ret = false;
    //
    //
    if  ( mpPreviewClient != 0 && ! mpPreviewClient->setImgBufProviderClient(rpClient) )
    {
        goto lbExit;
    }
    //
    if  ( mpRecordClient != 0 && ! mpRecordClient->setImgBufProviderClient(rpClient) )
    {
        goto lbExit;
    }
    //
    if  ( mpFDClient != 0 && ! mpFDClient->setImgBufProviderClient(rpClient) )
    {
        goto lbExit;
    }
    //
#if '1'==MTKCAM_HAVE_OT_CLIENT     
    if  ( mpOTClient != 0 && ! mpOTClient->setImgBufProviderClient(rpClient) )
    {
        goto lbExit;
    }
#endif    
    //
    if  ( mpPreviewFeatureClient != 0 && ! mpPreviewFeatureClient->setImgBufProviderClient(rpClient) )
    {
        goto lbExit;
    }
    //
    mpImgBufPvdrClient = rpClient;
    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
CamClient::
setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    //  value copy
    *mpCamMsgCbInfo = *rpCamMsgCbInfo;
    //
    if  ( mpPreviewClient != 0 ) {
        mpPreviewClient->setCallbacks(mpCamMsgCbInfo);
    }
    if  ( mpRecordClient != 0 ) {
        mpRecordClient->setCallbacks(mpCamMsgCbInfo);
    }
    if  ( mpFDClient != 0) {
        mpFDClient->setCallbacks(mpCamMsgCbInfo);
    }
#if '1'==MTKCAM_HAVE_OT_CLIENT     
    if  ( mpOTClient != 0) {
        mpOTClient->setCallbacks(mpCamMsgCbInfo);
    }    
#endif    
    if  ( mpPreviewFeatureClient != 0) {
        mpPreviewFeatureClient->setCallbacks(mpCamMsgCbInfo);
    }
}


/******************************************************************************
 * Enable a message, or set of messages.
 ******************************************************************************/
void
CamClient::
enableMsgType(int32_t msgType)
{
    MY_LOGD("msgType: %d", msgType);
    //
    if  ( mpPreviewClient != 0 )
    {
        mpPreviewClient->enableMsgType(msgType);
    }
    //
    if  ( mpRecordClient != 0 )
    {
        mpRecordClient->enableMsgType(msgType);
    }
    //
    if  ( mpFDClient != 0 )
    {
        mpFDClient->enableMsgType(msgType);
    }
    //
#if '1'==MTKCAM_HAVE_OT_CLIENT     
    if  ( mpOTClient != 0 )
    {
        mpOTClient->enableMsgType(msgType);
    }
#endif    
    //    
    if  ( mpPreviewFeatureClient != 0 )
    {
        mpPreviewFeatureClient->enableMsgType(msgType);
    }
}


/******************************************************************************
 * Disable a message, or a set of messages.
 *
 * Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
 * should not rely on its client to call releaseRecordingFrame() to release
 * video recording frames sent out by the cameral hal before and after the
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
 * modify/access any video recording frame after calling
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME).
 ******************************************************************************/
void
CamClient::
disableMsgType(int32_t msgType)
{
    if  ( mpPreviewClient != 0 )
    {
        mpPreviewClient->disableMsgType(msgType);
    }
    //
    if  ( mpRecordClient != 0 )
    {
        mpRecordClient->disableMsgType(msgType);
    }
    //
    if ( mpFDClient != 0 )
    {
        mpFDClient->disableMsgType(msgType);
    }
    //
#if '1'==MTKCAM_HAVE_OT_CLIENT     
    if ( mpOTClient != 0 )
    {
        mpOTClient->disableMsgType(msgType);
    }
#endif    
    //    
    if ( mpPreviewFeatureClient != 0 )
    {
        mpPreviewFeatureClient->disableMsgType(msgType);
    }
    
}


/******************************************************************************
 * Query whether a message, or a set of messages, is enabled.
 * Note that this is operates as an AND, if any of the messages
 * queried are off, this will return false.
 ******************************************************************************/
bool
CamClient::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == (msgType & ::android_atomic_release_load(&mpCamMsgCbInfo->mMsgEnabled));
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamClient::
startPreview()
{
    bool ret = false;
    //
    MY_LOGD("+");
    //  (1) Lock
    Mutex::Autolock _l(mModuleMtx);
    //
    if  ( mpPreviewClient != 0 && ! mpPreviewClient->startPreview() )
    {
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CamClient::
stopPreview()
{
    MY_LOGD("+");
    //
    if  ( mpPreviewClient != 0 )
    {
        mpPreviewClient->stopPreview();
    }
    //
    if  ( mpFDClient != 0 )
    {
        mpFDClient->stopPreview();
    }
    //
#if '1'==MTKCAM_HAVE_OT_CLIENT     
    if  ( mpOTClient != 0 )
    {
        mpOTClient->stopPreview();
    }    
#endif
    if  ( mpPreviewFeatureClient != 0 )
    {
        mpPreviewFeatureClient->stopPreview();
    }
    MY_LOGD("-");    
}


/******************************************************************************
 *
 ******************************************************************************/
void
CamClient::
takePicture()
{
    if ( mpPreviewClient != 0 )
    {
        mpPreviewClient->takePicture();
    }

    if ( mpFDClient != 0 )
    {
        mpFDClient->takePicture();
    }
    
#if '1'==MTKCAM_HAVE_OT_CLIENT 
    if ( mpOTClient != 0 )
    {
        mpOTClient->takePicture();
    }
#endif    
}


/******************************************************************************
 *
 ******************************************************************************/
/**
 * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
 * message is sent with the corresponding frame. Every record frame must be released
 * by a cameral hal client via releaseRecordingFrame() before the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
 * to manage the life-cycle of the video recording frames, and the client must
 * not modify/access any video recording frames.
 */
bool
CamClient::
startRecording()
{
    return  ( mpRecordClient != 0 )
        ?   mpRecordClient->startRecording()
        :   false
            ;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CamClient::
stopRecording()
{
    if  ( mpRecordClient != 0 )
    {
        mpRecordClient->stopRecording();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
/**
 * Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
 *
 * It is camera hal client's responsibility to release video recording
 * frames sent out by the camera hal before the camera hal receives
 * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
 * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
 * responsibility of managing the life-cycle of the video recording
 * frames.
 */
void
CamClient::
releaseRecordingFrame(const void *opaque)
{
    if  ( mpRecordClient != 0 )
    {
        mpRecordClient->releaseRecordingFrame(opaque);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamClient::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    MY_LOGD("+");
    if  ( mpPreviewClient != 0 && OK == mpPreviewClient->sendCommand(cmd, arg1, arg2) )
    {   //  Return OK since command has been processed.
        MY_LOGD("PreviewClient takes it - ");
        return  OK;
    }
    //
    if  ( mpRecordClient != 0 && OK == mpRecordClient->sendCommand(cmd, arg1, arg2) )
    {   //  Return OK since command has been processed.
        MY_LOGD("RecordClient takes it - ");
        return  OK;
    }

    if  ( mpFDClient !=0 && OK == mpFDClient->sendCommand(cmd, arg1, arg2))
    {
        MY_LOGD("FDClient takes it - ");
        return OK;
    }
    
#if '1'==MTKCAM_HAVE_OT_CLIENT 
    if  ( mpOTClient !=0 && OK == mpOTClient->sendCommand(cmd, arg1, arg2))
    {
        MY_LOGD("OTClient takes it - ");
        return OK;
    }
#endif
    if  ( mpPreviewFeatureClient !=0 && OK == mpPreviewFeatureClient->sendCommand(cmd, arg1, arg2))
    {
        MY_LOGD("mpPreviewFeatureClient takes it - ");
        return OK;
    }
    //
    return  INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamClient::
dump(int fd, Vector<String8>& args)
{
    //
    if  ( args.empty() ) {
        return  OK;
    }
    //
    MY_LOGD("args(%d)=%s", args.size(), (*args.begin()).string());
    //
    //
    ////////////////////////////////////////////////////////////////////////////
    //  Parse Command:
    ////////////////////////////////////////////////////////////////////////////
    //
    //  <Preview>
    if  ( *args.begin() == "Preview" )
    {
        args.erase(args.begin());
        mpPreviewClient->dump(fd, args);
        return  OK;
    }
    //
    //  <Record>
    if  ( *args.begin() == "Record" )
    {
        args.erase(args.begin());
        mpRecordClient->dump(fd, args);
        return  OK;
    }
    //
    //
    return OK;
}

