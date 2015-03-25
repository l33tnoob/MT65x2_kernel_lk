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
#include <inc/IState.h>
#include "State.h"
using namespace NSMtkEngCamAdapter;
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
#define ENABLE_OP_LOG               (1)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateBase
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*******************************************************************************
 *  
 ******************************************************************************/
StateBase::
StateBase(char const*const pcszName, ENState const eState)
    : IState()
    , m_pcszName(pcszName)
    , m_eState(eState)
    , m_pStateManager(IStateManager::inst())
{
}


/*******************************************************************************
 *  
 ******************************************************************************/
status_t
StateBase::
op_UnSupport(char const*const pcszDbgText)
{
    MY_LOGW("%s", pcszDbgText);
    //
    return  INVALID_OPERATION;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateIdle
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateIdle::
StateIdle(ENState const eState)
    : StateBase(__FUNCTION__, eState)
{
}


status_t
StateIdle::
onStartPreview(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleStartPreview();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
    status = stateWaiter.waitState(eState_Preview);
    if  ( OK != status ) {
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}



status_t
StateIdle::
onCapture(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleCapture();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
#if 1
    status = stateWaiter.waitState(eState_Capture);
    if  ( OK != status ) {
        goto lbExit;
    }
#endif
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatePreview::
StatePreview(ENState const eState)
    : StateBase(__FUNCTION__, eState)
{
}


status_t
StatePreview::
onStopPreview(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleStopPreview();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
    status = stateWaiter.waitState(eState_Idle);
    if  ( OK != status ) {
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}


status_t
StatePreview::
onPreCapture(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandlePreCapture();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
    status = stateWaiter.waitState(eState_PreCapture);
    if  ( OK != status ) {
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePreCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatePreCapture::
StatePreCapture(ENState const eState)
    : StateBase(__FUNCTION__, eState)
{
}


status_t
StatePreCapture::
onStopPreview(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleStopPreview();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
    status = stateWaiter.waitState(eState_Idle);
    if  ( OK != status ) {
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCapture::
StateCapture(ENState const eState)
    : StateBase(__FUNCTION__, eState)
{
}


status_t
StateCapture::
onCaptureDone(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleCaptureDone();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
    status = stateWaiter.waitState(eState_Idle);
    if  ( OK != status ) {
        goto lbExit;
    }
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}


status_t
StateCapture::
onCancelCapture(IStateHandler* pHandler)
{
    IStateManager::StateObserver stateWaiter(getStateManager());
    getStateManager()->registerOneShotObserver(&stateWaiter);
    //
    //
    MY_LOGD_IF(ENABLE_OP_LOG, "+");
    //
    status_t status = OK;
    //
    status = pHandler->onHandleCancelCapture();
    if  ( OK != status ) {
        goto lbExit;
    }
    //
#if 1
    status = stateWaiter.waitState(eState_Idle);
    if  ( OK != status ) {
        goto lbExit;
    }
#endif
    //
lbExit:
    MY_LOGD_IF(ENABLE_OP_LOG, "- status(%d)", status);
    return  status;
}

