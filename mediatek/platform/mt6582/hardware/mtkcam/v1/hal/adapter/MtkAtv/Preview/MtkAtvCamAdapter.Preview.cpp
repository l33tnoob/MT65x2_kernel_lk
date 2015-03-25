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

#define LOG_TAG "MtkATV/Preview"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkAtvCamAdapter.h>
using namespace NSMtkAtvCamAdapter;
//
#include <cutils/properties.h>
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[%s] "fmt,  getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
previewEnabled() const
{
    return  mpStateManager->isState(IState::eState_Preview);
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
startPreview()
{
    return  mpStateManager->getCurrentState()->onStartPreview(this);
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
stopPreview()
{
    mpStateManager->getCurrentState()->onStopPreview(this);
}


/******************************************************************************
*   CamAdapter::startPreview() -> IState::onStartPreview() -> 
*   IStateHandler::onHandleStartPreview() -> CamAdapter::onHandleStartPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStartPreview()
{
    MY_LOGD("+");

    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start, CPTFlagStart);
    
    if ( ! mpResourceLock->SetMode(ResourceLock::eMTKPHOTO_PRV) )
    {
        CAM_LOGE("Resource SetMode fail");
        return INVALID_OPERATION;
    }

    //
    if ( ! mpResourceLock->Lock(ResourceLock::eMTKPHOTO_PRV) )
    {
        CAM_LOGE("Resource Lock fail");
        return INVALID_OPERATION;
    }
    
    //
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagStart);       
    if ( ! mpPreviewCmdQueThread->postCommand(PrvCmdCookie::eStart, PrvCmdCookie::eSemAfter) )
    {
        MY_LOGE("StartPreview stage 1 (start): fail");
        goto lbExit;
    }
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagEnd);    
    
    //
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start_stable, CPTFlagStart);    
    if ( ! mpPreviewCmdQueThread->postCommand(PrvCmdCookie::eDelay, PrvCmdCookie::eSemAfter) )
    {
         MY_LOGE("StartPreview stage 2 (delay): fail");
         goto lbExit;
    }
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start_stable, CPTFlagEnd);    
    
    //
    if ( ! mpPreviewCmdQueThread->postCommand(PrvCmdCookie::eUpdate, PrvCmdCookie::eSemBefore) )
    {
         MY_LOGE("StartPreview stage 3 (udpate): fail");
         goto lbExit;
    }
    //
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_start, CPTFlagEnd);
    
    MY_LOGD("-");
    return OK;
   //
lbExit:
    return INVALID_OPERATION;
}


/******************************************************************************
*   CamAdapter::stopPreview() -> IState::onStopPreview() -> 
*   IStateHandler::onHandleStopPreview() -> CamAdapter::onHandleStopPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStopPreview()
{
    MY_LOGD("+");

    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagStart);

    if ( ! mpPreviewCmdQueThread->postCommand(PrvCmdCookie::eStop, PrvCmdCookie::eSemAfter) )
    {
        MY_LOGE("StopPreview fail");
        goto lbExit;
    }
    //
    if ( ! mpResourceLock->Unlock(ResourceLock::eMTKPHOTO_PRV) )
    {
        CAM_LOGE("Resource Lock fail");
        return INVALID_OPERATION;
    }
    //
    if ( ! mpResourceLock->SetMode(ResourceLock::eMTKCAM_IDLE) )
    {
        CAM_LOGE("Resource SetMode fail");
        return INVALID_OPERATION;
    }

    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagEnd);

    MY_LOGD("-");

    return OK;
    
lbExit:
    return INVALID_OPERATION;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onPreCapture() -> 
*   IStateHandler::onHandlePreCapture() -> CamAdapter::onHandlePreCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreCapture()
{
    MY_LOGD("+");

    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_precap, CPTFlagStart);

    if (!mpPreviewCmdQueThread->getCurPrvBuf(mpCurPrvBuf))
    {
        MY_LOGE("PreCapture fail");
        goto lbExit;        
    }

    IStateManager::inst()->transitState(IState::eState_PreCapture);

#if 0
    // [ATV]+
    if ( ! mpPreviewCmdQueThread->postCommand(PrvCmdCookie::ePrecap, PrvCmdCookie::eSemAfter) )
    {
        MY_LOGE("PreCapture fail");
        goto lbExit;        
    }
#endif    
    //[ATV]-
    CPTLog(Event_Hal_Adapter_MtkPhotoPreview_precap, CPTFlagEnd);

    MY_LOGD("-");
    return OK;
    
lbExit:    
    return INVALID_OPERATION;
}


