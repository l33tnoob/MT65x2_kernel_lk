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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkVTCamAdapter.h"
using namespace NSMtkVTCamAdapter;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
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
sp<ICamAdapter>
createMtkVTCamAdapter(
    String8 const&      rName, 
    int32_t const       i4OpenId, 
    sp<IParamsManager>  pParamsMgr
)
{
    return new CamAdapter(
        rName, 
        i4OpenId, 
        pParamsMgr
    );
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
CamAdapter(
    String8 const&      rName, 
    int32_t const       i4OpenId, 
    sp<IParamsManager>  pParamsMgr
)
    : BaseCamAdapter(rName, i4OpenId, pParamsMgr)
    //
    , mpStateManager(IStateManager::inst())
    //
    , mpPreviewCmdQueThread(0)
    , mpPreviewBufMgr(NULL)
    //
    , mpResourceLock(NULL)
    //
{
    MY_LOGD(
        "sizeof=%d, this=%p, mpStateManager=%p, mpPreviewCmdQueThread=%p, mpPreviewBufMgr=%p, mpResourceLock=%p", 
        sizeof(CamAdapter), this, &mpStateManager, &mpPreviewCmdQueThread, &mpPreviewBufMgr, &mpResourceLock
    );
}


/******************************************************************************
*
*******************************************************************************/
CamAdapter::
~CamAdapter()
{
    MY_LOGD("tid(%d), OpenId(%d)", ::gettid(), getOpenId());
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
init()
{
    status_t status = NO_ERROR;
    //
    if  ( ! BaseCamAdapter::init() ) {
        goto lbExit;
    }
    //
    //   
    mpPreviewBufMgr = IPreviewBufMgr::createInstance(mpImgBufProvidersMgr); 
    mpPreviewCmdQueThread = IPreviewCmdQueThread::createInstance(mpPreviewBufMgr, getOpenId(), mpParamsMgr);
    if  ( mpPreviewCmdQueThread == 0 || OK != (status = mpPreviewCmdQueThread->run()) )
    {
        MY_LOGE(
            "Fail to run PreviewCmdQueThread - mpPreviewCmdQueThread.get(%p), status[%s(%d)]", 
            mpPreviewCmdQueThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    //
    //
    if ( OK != init3A() )
    {
        MY_LOGE("Fail to init 3A");
        goto lbExit;
    }
    //
    //
    mpResourceLock = ResourceLock::CreateInstance();
    if(mpResourceLock != NULL)
    {
        if(!(mpResourceLock->Init()))
        {
            MY_LOGE("mpResourceLock->Init fail");
            goto lbExit;
        }
    }
    //
    return  true;
lbExit:
    MY_LOGE("init() fail; now call uninit()");
    uninit();
    return  false;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
uninit()
{
    MY_LOGD("+");
    //
    //
    sp<IPreviewCmdQueThread> pPreviewCmdQueThread = mpPreviewCmdQueThread; 
    mpPreviewCmdQueThread = 0;       
    if ( pPreviewCmdQueThread != 0 ) {
        MY_LOGD(
            "PreviewCmdQ Thread: (tid, getStrongCount)=(%d, %d)", 
            pPreviewCmdQueThread->getTid(), pPreviewCmdQueThread->getStrongCount()
        );
        pPreviewCmdQueThread->requestExit();
        pPreviewCmdQueThread = 0;
    }
    //
    sp<IPreviewBufMgr> pPreviewBufMgr = mpPreviewBufMgr;
    mpPreviewBufMgr = 0;
    if ( pPreviewBufMgr != 0 )
    {
        pPreviewBufMgr->destroyInstance();
        pPreviewBufMgr = 0;
    }
    //
    //
    uninit3A();
    //
    //
    if(mpResourceLock != NULL)
    {
        if(!(mpResourceLock->Uninit()))
        {
            MY_LOGE("mpResourceLock->Uninit fail");
        }
        mpResourceLock->DestroyInstance();
        mpResourceLock = NULL;
    }
    //
    //
    BaseCamAdapter::uninit();
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    MY_LOGD("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)",cmd,arg1,arg2);
    //
    switch  (cmd)
    {
    case CAMERA_CMD_START_SMOOTH_ZOOM:
        MY_LOGD("START_SMOOTH_ZOOM");
        //
        if(arg1 < 0)
        {
            MY_LOGE("arg1(%d) < 0",arg1);
            return BAD_VALUE;
        }
        //
        if(mpPreviewCmdQueThread == NULL)
        {
            MY_LOGE("mpPreviewCmdQueThread is NULL");
            return INVALID_OPERATION;
        }
        //
        mpParamsMgr->set(CameraParameters::KEY_ZOOM, arg1);
        //
        if(!mpPreviewCmdQueThread->setZoom(mpParamsMgr->getZoomRatioByIndex((uint32_t)arg1)))
        {
            MY_LOGE("PreviewCmdQueThread setZoom failed");
        }
        return OK;
    case CAMERA_CMD_STOP_SMOOTH_ZOOM:
        MY_LOGD("STOP_SMOOTH_ZOOM");
        //do nothing for now.
        return OK;

    case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        CAM_LOGD("[sendCommand] CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG (%d)\n", arg1); 
        enableAFMove(arg1);
        return OK;
        break; 

        
    default:
        break;
    }
    return  BaseCamAdapter::sendCommand(cmd, arg1, arg2);
}

