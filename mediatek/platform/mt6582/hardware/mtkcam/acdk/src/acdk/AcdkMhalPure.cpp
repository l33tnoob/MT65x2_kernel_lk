/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
 
//! \file  AcdkMhalPure.cpp

#define LOG_TAG "AcdkMhalPure"

#include <stdio.h>
#include <stdlib.h>
#include <linux/rtpm_prio.h>
#include <utils/threads.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <cutils/properties.h>

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkMhalBase.h"

using namespace NSAcdkMhal;

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ICdpPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>

using namespace NSImageio;
using namespace NSIspio;

#include "mtkcam/hal/sensor_hal.h"
#include "isp_function.h"
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/v1/config/PriorityDefs.h>

#include "AcdkMhalPure.h"

#define MEDIA_PATH "/data"
#define ACDK_MT6582_MDP_WO_IRQ 1 //SL test ACDK_6582_MDP without IRQ done
/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd;
static pthread_t g_threadMainHigh;
static AcdkMhalPure *g_pAcdkMHalPureObj = NULL;
static acdkObserver g_acdkMhalObserver;

static vector<PortInfo const*> g_vCamIOInPorts(1);
static vector<PortInfo const*> g_vCamIOIOutPorts(1);
//static vector<PortInfo const*> g_vCdpInPorts(3);
//static vector<PortInfo const*> g_vCdpOutPorts(3);
static vector<PortInfo const*> g_vPostProcInPorts(3);
static vector<PortInfo const*> g_vPostProcOutPorts(3);

static MINT32 g_acdkMhalPureDebug = 0;

/*******************************************************************************
* AcdkMhalPure
* brif : Constructor
*******************************************************************************/
AcdkMhalPure::AcdkMhalPure()
{
    mAcdkMhalState = ACDK_MHAL_NONE;

    m_pICamIOPipe = NULL;
    m_pCdpPipe = NULL;
    m_pPostProcPipe = NULL;

    mReadyForCap = MFALSE;

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));
    memset(&mAcdkMhalCapParam,0,sizeof(acdkMhalPrvParam_t));

    g_pAcdkMHalPureObj = this;
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalSetState(acdkMhalState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", mAcdkMhalState, newState);

    if(newState == ACDK_MHAL_ERROR)
    {
        goto ACDK_MHAL_SET_STATE_EXIT;
    }

    switch(mAcdkMhalState)
    {
    case ACDK_MHAL_NONE:
        switch(newState)
        {
        case ACDK_MHAL_INIT:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_NONE");
            ACDK_LOGE("State error ACDK_MHAL_NONE");
            break;
        }
        break;
    case ACDK_MHAL_INIT:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_INIT");
            ACDK_LOGE("State error ACDK_MHAL_INIT");
            break;
        }
        break;
    case ACDK_MHAL_IDLE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_CAPTURE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_IDLE");
            ACDK_LOGE("State error ACDK_MHAL_IDLE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_PRE_CAPTURE:
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PREVIEW");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW");
            break;
        }
        break;
    case ACDK_MHAL_PRE_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_PRE_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW_STOP:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_PREVIEW_STOP");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW_STOP");
            break;
        }
        break;
    case ACDK_MHAL_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_ERROR:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_ERROR");
            ACDK_LOGE("State error ACDK_MHAL_ERROR");
            break;
        }
        break;
    default:
        //ACDK_ASSERT(0, "Unknown state");
        ACDK_LOGE("Unknown state");
        break;
    }

ACDK_MHAL_SET_STATE_EXIT:

    mAcdkMhalState = newState;

    ACDK_LOGD("X, state(0x%04x)", mAcdkMhalState);
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
acdkMhalState_e AcdkMhalPure::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalPure::acdkMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkMhalProcLoop(MVOID *arg)
{
    ::prctl(PR_SET_NAME,"acdkMhalProcLoop",0,0,0);

#if MTKCAM_HAVE_RR_PRIORITY
    MINT32 const policy = SCHED_RR;
    MINT32 const priority = PRIO_RT_CAMERA_PREVIEW;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    ::sched_getparam(0, &sched_p);
    ACDK_LOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d),tid=%d", policy,
                                                                                             ::sched_getscheduler(0),
                                                                                             priority,
                                                                                             sched_p.sched_priority,
                                                                                             ::gettid());
#endif 

     //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    acdkMhalState_e eState;

    eState = g_pAcdkMHalPureObj->acdkMhalGetState();
    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalPureObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalPureObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);
    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalPure object
*******************************************************************************/
void AcdkMhalPure::destroyInstance()
{
    g_pAcdkMHalPureObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalInit()
{
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32  err = ACDK_RETURN_NO_ERROR;

    //====== Acdk Mhal State Setting ======

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    //====== Get Debug Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdk.debug", value, "0");
    g_acdkMhalPureDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalPureDebug(%d)",g_acdkMhalPureDebug);

    //====== Init Thread for Preview and Capture ======

    // Init semphore
    ::sem_init(&g_SemMainHigh, 0, 0);
    ::sem_init(&g_SemMainHighBack, 0, 0);
    ::sem_init(&g_SemMainHighEnd, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_CAMERA_PREVIEW};
#if MTKCAM_HAVE_RR_PRIORITY
    pthread_create(&g_threadMainHigh, &attr, acdkMhalProcLoop, NULL);
#else
    pthread_create(&g_threadMainHigh, NULL, acdkMhalProcLoop, NULL);
#endif

    //=== Set State to Idle State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalUninit
* brif : Uninitialize camera
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalUninit()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e eState;

    //====== Uninitialization ======

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = acdkMhalGetState();

    ACDK_LOGD("eState(0x%x)",eState);

    if(eState != ACDK_MHAL_NONE)
    {
        if((eState != ACDK_MHAL_IDLE) && (eState != ACDK_MHAL_ERROR))
        {
            ACDK_LOGD("Camera is not in the idle state");
            if(eState & ACDK_MHAL_PREVIEW_MASK)
            {
                err = acdkMhalPreviewStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalPreviewStop fail(0x%x)",err);
                }
            }
            else if(eState & ACDK_MHAL_CAPTURE_MASK)
            {
                err = acdkMhalCaptureStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalCaptureStop fail(0x%x)",err);
                }
            }

            // Polling until idle
            while(eState != ACDK_MHAL_IDLE)
            {
                // Wait 10 ms per time
                usleep(10000);
                eState = acdkMhalGetState();
            }
            ACDK_LOGD("Now camera is in the idle state");
        }

        //====== Set State to Uninit State ======

        acdkMhalSetState(ACDK_MHAL_UNINIT);

        //====== Semephore Process ======

        //post sem
        ACDK_LOGD("post g_SemMainHigh");
        ::sem_post(&g_SemMainHigh);

        //wait sem
        ACDK_LOGD("wait for g_SemMainHighEnd");
        ::sem_wait(&g_SemMainHighEnd);
        ACDK_LOGD("got g_SemMainHighEnd");
    }
    else
    {
        acdkMhalSetState(ACDK_MHAL_UNINIT);
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalCBHandle
* brif : callback handler
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalCBHandle(MUINT32 a_type, MUINT32 a_addr1, MUINT32 a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"+");
    //ACDK_ASSERT(!!g_acdkMhalObserver, "callback is NULL");
    if(!g_acdkMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "Camera State is not IDLE");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalPrvParam, a_pBuffIn, sizeof(acdkMhalPrvParam_t));
    QBufInfo rQBufInfo;

    //====== Member Variable ======

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        ACDK_LOGD("imgImemBuf  : virAddr[%d]=0x%x",i,mAcdkMhalPrvParam.imgImemBuf[i].virtAddr);
        ACDK_LOGD("dispImemBuf : virAddr[%d]=0x%x",i,mAcdkMhalPrvParam.dispImemBuf[i].virtAddr);
    }

    //====== Init CameraIO Pipe ======

    m_pICamIOPipe = ICamIOPipe::createInstance(mAcdkMhalPrvParam.scenarioHW, mAcdkMhalPrvParam.sensorType?eScenarioFmt_YUV:eScenarioFmt_RAW);

    if(MTRUE != m_pICamIOPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->init() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    //====== Config CamIO Pipe ======

    m_tgi.eImgFmt     = (EImageFormat)mAcdkMhalPrvParam.sensorFormat; //  Image Pixel Format
    m_tgi.eRawPxlID   = (ERawPxlID)mAcdkMhalPrvParam.sensorColorOrder;
    m_tgi.u4ImgWidth  = mAcdkMhalPrvParam.sensorWidth;  // Image Width
    m_tgi.u4ImgHeight = mAcdkMhalPrvParam.sensorHeight; // Image Height
    m_tgi.type        = EPortType_Sensor;     // EPortType
    m_tgi.index       = (mAcdkMhalPrvParam.sensorID == SENSOR_DEV_MAIN || mAcdkMhalPrvParam.sensorID == SENSOR_DEV_ATV) ? EPortIndex_TG1I : EPortIndex_TG1I;      //  port index
    m_tgi.inout       = EPortDirection_In;    // 0:in/1:out

    m_imgo.eImgFmt     = m_tgi.eImgFmt;     // Image Pixel Format
    m_imgo.u4ImgWidth  = m_tgi.u4ImgWidth;  // Image Width
    m_imgo.u4ImgHeight = m_tgi.u4ImgHeight; // Image Height
    m_imgo.crop.y      = 0;                     // TG crop
    m_imgo.crop.h      = m_imgo.u4ImgHeight;    //TG crop
    m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.sensorStride[0];    //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalPrvParam.sensorStride[1]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalPrvParam.sensorStride[2]; //  unit:PIXEL
    m_imgo.type  = EPortType_Memory;        // EPortType
    m_imgo.index = EPortIndex_IMGO;         // port index
    m_imgo.inout = EPortDirection_Out;      // 0:in/1:out

    //set CQ first before pipe config
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0,0,0);
    //RL modify
    //m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)CAM_CQ_SINGLE_IMMEDIATE_TRIGGER,(MINT32)CAM_CQ_TRIG_BY_START);
    //single trigger
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
         

    // config IO port
    g_vCamIOInPorts.at(0)   = &m_tgi;
    g_vCamIOIOutPorts.at(0) = &m_imgo;

    if(MTRUE != m_pICamIOPipe->configPipe(g_vCamIOInPorts, g_vCamIOIOutPorts))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->configPipe() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    //====== Enqueue Buffer ======

    //IMGO
    mPortID.index = EPortIndex_IMGO;
    rQBufInfo.vBufInfo.resize(1);
    rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.imgImemBuf[0].size;  //bytes

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        rQBufInfo.vBufInfo[0].u4BufVA = mAcdkMhalPrvParam.imgImemBuf[i].virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA = mAcdkMhalPrvParam.imgImemBuf[i].phyAddr;
        rQBufInfo.vBufInfo[0].memID   = mAcdkMhalPrvParam.imgImemBuf[i].memID;

        m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
        //RL modify
        ACDK_LOGD("m_pICamIOPipe->enqueOutBuf[%d].u4BufPA(%x)",i,rQBufInfo.vBufInfo[0].u4BufVA); 

    }

    //====== Init Postproc Pipe ======

    m_pPostProcPipe = IPostProcPipe::createInstance(mAcdkMhalPrvParam.scenarioHW, mAcdkMhalPrvParam.sensorType?eScenarioFmt_YUV:eScenarioFmt_RAW);

    if(MTRUE != m_pPostProcPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->init() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    //====== Config PostProcPipe ======

    m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
    m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
    m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
    m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    //  pixel
    m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];
    m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];
    m_imgi.crop.x = 0;
    m_imgi.crop.y = 0;
    m_imgi.crop.w = m_imgi.u4ImgWidth;
    m_imgi.crop.h = m_imgi.u4ImgHeight;
    m_imgi.type        = EPortType_Memory;      //  EPortType
    m_imgi.index       = EPortIndex_IMGI;       //  port index
    m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
    m_imgi.pipePass    = EPipePass_PASS2;       //  select pass

#if 0
    m_dispo.eImgFmt     = (EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;   // Image Pixel Format
    m_dispo.eImgRot     = eImgRot_0;                    //dispo NOT support rotation
    m_dispo.eImgFlip    = eImgFlip_OFF;                 //dispo NOT support flip
    m_dispo.u4ImgWidth  = mAcdkMhalPrvParam.frmParam.w; // Dispaly Width
    m_dispo.u4ImgHeight = mAcdkMhalPrvParam.frmParam.h; // Display Height
    m_dispo.type        = EPortType_DISP_RDMA;          // EPortType
    m_dispo.index       = EPortIndex_DISPO;             // port index
    m_dispo.inout       = EPortDirection_Out;           // 0:in/1:out

#endif

    m_vido.eImgFmt     = (EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_vido.eImgRot     = (EImageRotation)mAcdkMhalPrvParam.frmParam.orientation;    // Rotation
    m_vido.eImgFlip    = mAcdkMhalPrvParam.frmParam.flip ? eImgFlip_ON : eImgFlip_OFF;
    m_vido.u4ImgWidth  = mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_vido.u4ImgHeight = mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.frmParam.stride[0]; //  pixel
    m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalPrvParam.frmParam.stride[1];
    m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalPrvParam.frmParam.stride[2];
    m_vido.type        = EPortType_VID_RDMA;                    // EPortType
    m_vido.index       = EPortIndex_VIDO;                       // port index
    m_vido.inout       = EPortDirection_Out;                    // 0:in/1:out

    g_vPostProcInPorts.resize(1);
    g_vPostProcInPorts.at(0) = &m_imgi;

    g_vPostProcOutPorts.resize(1);
    //g_vPostProcOutPorts.at(0) = &m_dispo;
    g_vPostProcOutPorts.at(0) = &m_vido;

    //set CQ first before pipe config
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ1,0,0);

    // tpipe
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init,0,0);

    if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    // tpipe
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_UpdateTrigger,0,0);

    //enqueue buffer
    //mPortID.index = EPortIndex_DISPO; //DISPO
    mPortID.index = EPortIndex_VIDO;    //VIDO
    rQBufInfo.vBufInfo.resize(1);   //vr dispo out is
    rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.dispImemBuf[0].size;    //bytes

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        rQBufInfo.vBufInfo[0].u4BufVA = mAcdkMhalPrvParam.dispImemBuf[i].virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA = mAcdkMhalPrvParam.dispImemBuf[i].phyAddr;
        rQBufInfo.vBufInfo[0].memID   = mAcdkMhalPrvParam.dispImemBuf[i].memID;

        m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
    }

    //====== Setting Callback ======

    g_acdkMhalObserver = mAcdkMhalPrvParam.acdkMainObserver;

    //====== Set State to Preview State ======

    acdkMhalSetState(ACDK_MHAL_PREVIEW);

    //====== Post Sem ======

    ::sem_post(&g_SemMainHigh);

acdkMhalPreviewStart_Exit:

    if(err != ACDK_RETURN_NO_ERROR)
    {
        if(m_pICamIOPipe != NULL)
        {
            ACDK_LOGE("release m_pICamIOPipe");
            m_pICamIOPipe->uninit();
            m_pICamIOPipe->destroyInstance();
            m_pICamIOPipe = NULL;
        }

        if(m_pPostProcPipe != NULL)
        {
            ACDK_LOGE("release m_pPostProcPipe");
            m_pPostProcPipe->uninit();
            m_pPostProcPipe->destroyInstance();
            m_pPostProcPipe = NULL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreviewStop
* brif : stop preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewStop()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    ACDK_LOGD("state(%d)", state);
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("is in IDLE state");
        return err;
    }
    else if(state != ACDK_MHAL_PREVIEW_STOP)
    {
        if(state & ACDK_MHAL_PREVIEW_MASK)
        {
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_PRE_CAPTURE)
        {
            // In preCapture, has to wait 3A ready flag
            ACDK_LOGD("it is ACDK_MHAL_PRE_CAPTURE state");
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_CAPTURE)
        {
            // It is in capture flow now, preview has been already stopped
            ACDK_LOGD("it is ACDK_MHAL_CAPTURE state");
            state = acdkMhalGetState();
            while(state == ACDK_MHAL_CAPTURE)
            {
                usleep(20);
                state = acdkMhalGetState();
            }

            acdkMhalSetState(ACDK_MHAL_IDLE);
        }
        else
        {
            // Unknown
            ACDK_LOGE("state(%d)", state);
        }
    }

    //====== Wait Semaphore ======

    ACDK_LOGD("Wait g_SemMainHighBack");
    ::sem_wait(&g_SemMainHighBack);
    ACDK_LOGD("Got g_SemMainHighBack");

    //====== Stop Preview ======

    if(MTRUE != m_pICamIOPipe->stop())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->stop() fail");
    }

    //====== Uninit CamIO & PostProcPipe ======

    if(MTRUE != m_pICamIOPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->uninit() fail");
    }

    m_pICamIOPipe->destroyInstance();
    m_pICamIOPipe = NULL;
    ACDK_LOGD("m_pICamIOPipe->destroyInstance()");

    if(MTRUE != m_pPostProcPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->uninit() fail");
    }

    m_pPostProcPipe->destroyInstance();
    m_pPostProcPipe = NULL;
    ACDK_LOGD("m_pPostProcPipe->destroyInstance()");

#if 0
    //====== Uninit CDP ======

    if(MTRUE != m_pCdpPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pCdpPipe->uninit() fail");
        return err;
    }

    m_pCdpPipe->destroyInstance();
    m_pCdpPipe = NULL;
    ACDK_LOGD("m_pCdpPipe->destroyInstance()");
#endif

    //====== Initialize Member Variable =====

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    //====== Set Acdk Mhal State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreCapture
* brif : change ACDK mhal state to preCapture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    acdkMhalSetState(ACDK_MHAL_PRE_CAPTURE);

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");
    
    #if 0    

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "Camera State s not IDLE");

    //====== Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalCapParam, a_pBuffIn, sizeof(acdkMhalParam_t));
    QBufInfo rQBufInfo;

    //====== Init CameraIO Pipe ======

    m_pICamIOPipe = ICamIOPipe::createInstance(mAcdkMhalCapParam.scenarioHW, mAcdkMhalCapParam.sensorType?eScenarioFmt_YUV:eScenarioFmt_RAW);

    if(MTRUE != m_pICamIOPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->init() fail");
        goto acdkMhalCaptureStart_Exit;
    }

    //====== Config CamIO Pipe ======

    m_tgi.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.sensorFormat; //  Image Pixel Format
    m_tgi.eRawPxlID   = (ERawPxlID)mAcdkMhalCapParam.sensorColorOrder;
    m_tgi.u4ImgWidth  = mAcdkMhalCapParam.sensorWidth;  // Image Width
    m_tgi.u4ImgHeight = mAcdkMhalCapParam.sensorHeight; // Image Height
    m_tgi.type        = EPortType_Sensor;     // EPortType
    m_tgi.index       = (mAcdkMhalCapParam.sensorID == SENSOR_DEV_MAIN || mAcdkMhalCapParam.sensorID == SENSOR_DEV_ATV) ? EPortIndex_TG1I : EPortIndex_TG2I;      //  port index
    m_tgi.inout       = EPortDirection_In;    // 0:in, 1:out

    m_imgo.eImgFmt     = m_tgi.eImgFmt;     // Image Pixel Format
    m_imgo.u4ImgWidth  = m_tgi.u4ImgWidth;  // Image Width
    m_imgo.u4ImgHeight = m_tgi.u4ImgHeight; // Image Height
    m_imgo.crop.y      = 0;
    m_imgo.crop.h      = m_imgo.u4ImgHeight;
    m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.sensorStride[0]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.sensorStride[1]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.sensorStride[2]; //  unit:PIXEL
    m_imgo.type  = EPortType_Memory;        // EPortType
    m_imgo.index = EPortIndex_IMGO;         // port index
    m_imgo.inout = EPortDirection_Out;      // 0:in/1:out

    //set CQ first before pipe config
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0,0,0);
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)CAM_ISP_CQ0,(MINT32)CAM_CQ_SINGLE_IMMEDIATE_TRIGGER,(MINT32)CAM_CQ_TRIG_BY_START);
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init,0,0);

    // config IO port
    g_vCamIOInPorts.at(0)   = &m_tgi;
    g_vCamIOIOutPorts.at(0) = &m_imgo;

    if(MTRUE != m_pICamIOPipe->configPipe(g_vCamIOInPorts, g_vCamIOIOutPorts))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->configPipe() fail");
        goto acdkMhalCaptureStart_Exit;
    }

    //enqueue buffer
    ACDK_LOGD("capPass1ImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capPass1ImemBuf.virtAddr);

    mPortID.index = EPortIndex_IMGO;    //IMGO
    rQBufInfo.vBufInfo.resize(1);
    rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalCapParam.capPass1ImemBuf.size;   //bytes
    rQBufInfo.vBufInfo[0].u4BufVA   = mAcdkMhalCapParam.capPass1ImemBuf.virtAddr;
    rQBufInfo.vBufInfo[0].u4BufPA   = mAcdkMhalCapParam.capPass1ImemBuf.phyAddr;
    rQBufInfo.vBufInfo[0].memID     = mAcdkMhalCapParam.capPass1ImemBuf.memID;

    m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);


    //====== Init Postproc Pipe ======

    m_pPostProcPipe = IPostProcPipe::createInstance(eScenarioID_IP, mAcdkMhalCapParam.sensorType?eScenarioFmt_YUV:eScenarioFmt_RAW);

    if(MTRUE != m_pPostProcPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->init() fail");
        goto acdkMhalCaptureStart_Exit;
    }

    //====== Config PostProcPipe ======

    m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
    m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
    m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
    m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    // unit:PIXEL
    m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];    // unit:PIXEL
    m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];    // unit:PIXEL
    m_imgi.crop.x = 0;                          // unit:PIXEL
    m_imgi.crop.y = 0;                          // unit:PIXEL
    m_imgi.crop.w = m_imgi.u4ImgWidth;          // unit:PIXEL
    m_imgi.crop.h = m_imgi.u4ImgHeight;         // unit:PIXEL
    m_imgi.type        = EPortType_Memory;      //  EPortType
    m_imgi.index       = EPortIndex_IMGI;       //  port index
    m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
    m_imgi.pipePass    = EPipePass_PASS2;       //  select pass

    if(mAcdkMhalCapParam.captureType == RAW_TYPE)
    {
        m_vido.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.qvParam.frmFormat;   // Image Pixel Format
        m_vido.eImgRot     = (EImageRotation)mAcdkMhalCapParam.qvParam.orientation;    //dispo NOT support rotation
        m_vido.eImgFlip    = mAcdkMhalCapParam.qvParam.flip ? eImgFlip_ON : eImgFlip_OFF;  //dispo NOT support flip
        m_vido.u4ImgWidth  = mAcdkMhalCapParam.qvParam.w;           // Image Width
        m_vido.u4ImgHeight = mAcdkMhalCapParam.qvParam.h;           // Image Height
        m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.qvParam.stride[0];
        m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.qvParam.stride[1];
        m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.qvParam.stride[2];
        m_vido.type        = EPortType_DISP_RDMA;   // EPortType
        m_vido.index       = EPortIndex_VIDO;       // port index
        m_vido.inout       = EPortDirection_Out;    // 0:in/1:out
    }
    else
    {
        m_vido.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.frmParam.frmFormat;   // Image Pixel Format
        m_vido.eImgRot     = (EImageRotation)mAcdkMhalCapParam.frmParam.orientation;    //dispo NOT support rotation
        m_vido.eImgFlip    = mAcdkMhalCapParam.frmParam.flip ? eImgFlip_ON : eImgFlip_OFF;  //dispo NOT support flip
        m_vido.u4ImgWidth  = mAcdkMhalCapParam.frmParam.w;           // Image Width
        m_vido.u4ImgHeight = mAcdkMhalCapParam.frmParam.h;           // Image Height
        m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.frmParam.stride[0];
        m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.frmParam.stride[1];
        m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.frmParam.stride[2];
        m_vido.type        = EPortType_DISP_RDMA;   // EPortType
        m_vido.index       = EPortIndex_VIDO;       // port index
        m_vido.inout       = EPortDirection_Out;    // 0:in/1:out
    }

    //set CQ first before pipe config
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ1,0,0);

    // tpipe
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init,0,0);

    // config IO port
    g_vPostProcInPorts.resize(1);
    g_vPostProcInPorts.at(0) = &m_imgi;

    g_vPostProcOutPorts.resize(1);
    g_vPostProcOutPorts.at(0) = &m_vido;

    if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
        goto acdkMhalCaptureStart_Exit;
    }

    // tpipe
    m_pPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_UpdateTrigger,0,0);

    //enqueue buffer
    if(mAcdkMhalCapParam.captureType == RAW_TYPE)
    {
        ACDK_LOGD("capQVImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capQVImemBuf.virtAddr);

        mPortID.index = EPortIndex_VIDO;    //VIDO
        rQBufInfo.vBufInfo.resize(1);
        rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalCapParam.capQVImemBuf.size;    //bytes
        rQBufInfo.vBufInfo[0].u4BufVA   = mAcdkMhalCapParam.capQVImemBuf.virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA   = mAcdkMhalCapParam.capQVImemBuf.phyAddr;
        rQBufInfo.vBufInfo[0].memID     = mAcdkMhalCapParam.capQVImemBuf.memID;
    }
    else
    {
        ACDK_LOGD("capPass2ImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capPass2ImemBuf.virtAddr);

        mPortID.index = EPortIndex_VIDO;    //VIDO
        rQBufInfo.vBufInfo.resize(1);
        rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalCapParam.capPass2ImemBuf.size;    //bytes
        rQBufInfo.vBufInfo[0].u4BufVA   = mAcdkMhalCapParam.capPass2ImemBuf.virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA   = mAcdkMhalCapParam.capPass2ImemBuf.phyAddr;
        rQBufInfo.vBufInfo[0].memID     = mAcdkMhalCapParam.capPass2ImemBuf.memID;
    }

    m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);

    //====== Setting Callback ======

    g_acdkMhalObserver = mAcdkMhalCapParam.acdkMainObserver;

    //====== Set State to Preview State ======

    acdkMhalSetState(ACDK_MHAL_CAPTURE);

    //====== Post Sem ======

    ::sem_post(&g_SemMainHigh);

acdkMhalCaptureStart_Exit:

    if(err != ACDK_RETURN_NO_ERROR)
    {
        if(m_pICamIOPipe != NULL)
        {
            m_pICamIOPipe->uninit();
            m_pICamIOPipe->destroyInstance();
            m_pICamIOPipe = NULL;
        }

        if(m_pPostProcPipe != NULL)
        {
            m_pPostProcPipe->uninit();
            m_pPostProcPipe->destroyInstance();
            m_pPostProcPipe = NULL;
        }
    }

    ACDK_LOGD("-");
    return err;
#else
    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif

}


/*******************************************************************************
* acdkMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureStop()
{
    ACDK_LOGD("+");
#if 0

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("in IDLE state");
        return err;
    }
    else if(state & ACDK_MHAL_PREVIEW_MASK)
    {
        ACDK_LOGD("in preview group state");
        return err;
    }
    else if(state & ACDK_MHAL_CAPTURE_MASK)
    {
        ACDK_LOGD("in capture group state");
        acdkMhalSetState(ACDK_MHAL_IDLE);
    }
    else
    {
        ACDK_LOGE("state:%d", state);
        ACDK_ASSERT(0, "[acdkMhalCaptureStop] state is unknow");
    }

    //====== Stop Capture ======

    if(MTRUE != m_pICamIOPipe->stop())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->stop() fail");
    }

    //====== Uninit CamIO & PostProcPipe ======

    if(MTRUE != m_pICamIOPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->uninit() fail");
    }

    m_pICamIOPipe->destroyInstance();
    m_pICamIOPipe = NULL;
    ACDK_LOGD("m_pICamIOPipe->destroyInstance()");

    if(mAcdkMhalCapParam.captureType != RAW_TYPE)
    {
        if(MTRUE != m_pPostProcPipe->uninit())
        {
            err = ACDK_RETURN_API_FAIL;
            ACDK_LOGE("m_pPostProcPipe->uninit() fail");
        }

        m_pPostProcPipe->destroyInstance();
        m_pPostProcPipe = NULL;
        ACDK_LOGD("m_pPostProcPipe->destroyInstance()");
    }

    //====== Initialize Member Variable ======

    memset(&mAcdkMhalCapParam,0,sizeof(acdkMhalParam_t));

    //====== Set AcdkMhal State ======

    mReadyForCap = MFALSE;

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
#else
    ACDK_LOGD("-");

    return ACDK_RETURN_NO_ERROR;
#endif
}

/*******************************************************************************
* 
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetShutTime()
{    
    return 0;    
}

/*******************************************************************************
* 
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalSetShutTime(MUINT32 a_time)
{    
}
/*******************************************************************************
* 
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetAFInfo()
{
    return 0;
}

/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewProc()
{
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"+");

    //====== Loca Variable Declaration ======

    MINT32  imgo_memID[OVERLAY_BUFFER_CNT] = {0};
    MUINT32 imgo_virAddr[OVERLAY_BUFFER_CNT] = {0}, imgo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINT32 imgi_memID, imgi_virAddr, imgi_phyAddr, dispo_memID, dispo_virAddr, dispo_phyAddr;
    MUINT32 imgo_num = 0;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;

    //====== Get Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //====== Variable Setting ======
    g_vPostProcInPorts.resize(1);
    g_vPostProcOutPorts.resize(1);

    //====== Preview Process ======

    //set pass1 out DMA register before pass1 start
    ACDK_LOGD("EPIPECmd_SET_CURRENT_BUFFER");
    m_pICamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGO,0,0);

    // start CQ
    m_pICamIOPipe->startCQ0();

    //cause CQ0 was configurred as SINGLE_IMMEDIATELY
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ_NONE,0,0);

    //sync VD
    if(m_tgi.index == EPortIndex_TG1I)
    {
        ACDK_LOGD("irq(EPipePass_PASS1_TG1)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_VSYNC);
    }
    //else if(m_tgi.index == EPortIndex_TG2I)
    //{
    //    ACDK_LOGD("irq(EPipePass_PASS1_TG2)(EPIPEIRQ_VSYNC)");
    //    m_pICamIOPipe->irq(EPipePass_PASS1_TG2,EPIPEIRQ_VSYNC);
    //}

    ACDK_LOGD("Preview pass1 start");
    m_pICamIOPipe->start();

    //pass1 continuous
    while(acdkMhalGetState() & ACDK_MHAL_PREVIEW_MASK)
    {
        //====== Pass1 ======

#if 0 // all these 3 steps are not required because FBC is implemented in CamIOPipe
        //SOF switch DMA address right after SOF
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_SOF);   //TG1

        //set pass1 out DMA register next buffer after SOF, will be applied to next SOF
        m_pICamIOPipe->sendCommand((MINT32)EPIPECmd_SET_NEXT_BUFFER, (MINT32)EPortIndex_IMGO,0,0);

        ACDK_LOGD("waiting pass1 done");

        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_PATH_DONE);
#endif

        ACDK_LOGD("waiting pass1 done");

        //dequeue pass1 OUT buffer
        mPortID.index = EPortIndex_IMGO;
        m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo);

        //check dequeue buffer number
        if(rQTSBufInfo.vBufInfo.size() == 0)
        {
            ACDK_LOGD("pass1 dequeue no buffer");
            continue;
        }
        else
        {
            imgo_num = (MUINT32)rQTSBufInfo.vBufInfo.size();

            for(MUINT32 i = 0; i < imgo_num; ++i)
            {
                imgo_virAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufVA;
                imgo_phyAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufPA;
                imgo_memID[i]   = rQTSBufInfo.vBufInfo[i].memID;
                ACDK_LOGD_DYN(g_acdkMhalPureDebug,"imgo_virAddr[%d](0x%x),imgo_phyAddr[%d](0x%x)",i,imgo_virAddr[i],i,imgo_phyAddr[i]);
            }
        }

        if(dumpEnable == 1 || dumpEnable == 3)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            ACDK_LOGD("prv VA(0x%x)",imgo_virAddr[0]);

            sprintf(szFileName, "%s/acdkPrv1.bin" , MEDIA_PATH);

            //====== Write RAW Data ======

            FILE *pFp = fopen(szFileName, "wb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
                fclose(pFp);
            }
            else
            {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr[0];
                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalPrvParam.imgImemBuf[0].size, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("Save image file name:%s, w(%u), h(%u)", szFileName, m_tgi.u4ImgWidth, m_tgi.u4ImgHeight);

                fclose(pFp);
            }
        }

        //====== Pass2 ======

        //enqueue pass2 IN buffer
        rQBufInfo.vBufInfo.resize(1);

        mPortID.index = EPortIndex_IMGI;
        rQBufInfo.vBufInfo[0].u4BufVA   = imgo_virAddr[0];   //always use one frame
        rQBufInfo.vBufInfo[0].u4BufPA   = imgo_phyAddr[0];   //always use one frame
        rQBufInfo.vBufInfo[0].memID     = imgo_memID[0];
        rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.imgImemBuf[0].size;
        m_pPostProcPipe->enqueInBuf(mPortID, rQBufInfo);

        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"Update pass2 tuning cmdQ");

        //config pass2 every time

        g_vPostProcInPorts.at(0) = &m_imgi;
        g_vPostProcOutPorts.at(0) = &m_vido;

        if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
        {
            ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
        }

        //set pass2 IN DMA register before pass2 start
        m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);

        //set pass2 OUT DMA register before pass2 start
        //m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
        m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);

        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass2 start");
        m_pPostProcPipe->start();

        ACDK_LOGD("waiting pass2 done");

        m_pPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);

        ACDK_LOGD("pass2 done");
#if ACDK_MT6582_MDP_WO_IRQ
#else
        //dequeue pass2 in buffer
        ACDK_LOGD("waiting dequeue pass2 in buffer");
        mPortID.index = EPortIndex_IMGI;
        m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
        imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
        imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
        imgi_memID   = rQTSBufInfo.vBufInfo[0].memID;
        ACDK_LOGD("dequeue pass2 in buffer done");
#endif
        //dequeue pass2 out buffer
        //mPortID.index = EPortIndex_DISPO;
        mPortID.index = EPortIndex_VIDO;
        m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo);

        m_pPostProcPipe->stop();
        ACDK_LOGD_DYN(g_acdkMhalPureDebug," pass2 stop");

        if(rQTSBufInfo.vBufInfo.size() == 0)
        {
            ACDK_LOGD("pass2 deque no buffer");
        }
        else
        {
            dispo_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
            dispo_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
            dispo_memID   = rQTSBufInfo.vBufInfo[0].memID;

            //====== Preview CallBack ======

            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"dispo_virAddr(0x%x),dispo_phyAddr(0x%x)", dispo_virAddr,dispo_phyAddr);

            acdkMhalCBHandle(ACDK_CB_PREVIEW, dispo_virAddr);

            //====== Enqueue Pass2 Out Buffer ======

            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"prvCB back, enque dispo_virAddr(0x%x)",dispo_virAddr);

            // enqueue pass2 out buffer
            //mPortID.index = EPortIndex_DISPO;
            mPortID.index = EPortIndex_VIDO;
            rQBufInfo.vBufInfo[0].u4BufVA = dispo_virAddr;
            rQBufInfo.vBufInfo[0].u4BufPA = dispo_phyAddr;
            rQBufInfo.vBufInfo[0].memID   = dispo_memID;
            rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.dispImemBuf[0].size;
            m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
        }
#if ACDK_MT6582_MDP_WO_IRQ
        //dequeue pass2 in buffer
        mPortID.index = EPortIndex_IMGI;
        m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
        imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
        imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
        imgi_memID   = rQTSBufInfo.vBufInfo[0].memID;
#endif

        //====== Enqueue Pass1 Buffer ======

        for(MUINT32 i = 0; i < imgo_num; ++i)
        {
            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass1 enque imgo_virAddr[%d](0x%x)",i,imgo_virAddr[i]);
            mPortID.index = EPortIndex_IMGO;
            rQBufInfo.vBufInfo[0].u4BufVA  = imgo_virAddr[i];
            rQBufInfo.vBufInfo[0].u4BufPA  = imgo_phyAddr[i];
            rQBufInfo.vBufInfo[0].memID    = imgo_memID[i];
            rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.imgImemBuf[0].size;
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
        }

        ACDK_LOGD("pass1 enque done");
        imgo_num = 0;
    }

    ACDK_LOGD("Exit preview while");

    //====== PreCapture Process ======

    MINT32 tempCnt = 0;
    if(ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState())
    {
        while(mReadyForCap == MFALSE)
        {
            //cotta--temp : just for test
            ++tempCnt;

            if(tempCnt > 50)
            {
                ACDK_LOGD("ready for capture");
                mReadyForCap = MTRUE;
            }
            else
            {
                usleep(1000);
            }

            if(mReadyForCap == MTRUE)
            {
                ACDK_LOGD("set state to PreviewStop and leave PreCapture");
                acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
            }
        }
    }

    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"-");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureProc()
{
#if 0
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32 imgo_memID,imgi_memID,vido_memID;
    MUINT32 imgi_virAddr,imgi_phyAddr,imgo_virAddr,imgo_phyAddr,vido_virAddr,vido_phyAddr;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;

    //====== Variable Setting ======

    g_vPostProcInPorts.resize(1);
    g_vPostProcOutPorts.resize(1);

    //====== Get Property ======

    char value[32] = {'\0'};

    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //====== Pass1 ======

    //set pass1 out DMA register before CQ0 start
    //use CQ0 to update DMA register
    ACDK_LOGD("EPIPECmd_SET_CURRENT_BUFFER");
    m_pICamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGO,0,0);

    ACDK_LOGD("CQ0 start");

    m_pICamIOPipe->startCQ0();

    ACDK_LOGD("pass1 start");

    m_pICamIOPipe->start(); //one time capture

    ACDK_LOGD("waiting pass1 done");

#if 0 // all these 3 steps are not required because FBC is implemented in CamIOPipe

    m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_PATH_DONE);

    ACDK_LOGD("zsd pass1 done");
#endif
    //dequeue pass1 OUT buffer
    mPortID.index = EPortIndex_IMGO;
    m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo);

    //MUST do
    ACDK_LOGD("clear VFDATA_EN bit");
    m_pICamIOPipe->stop();

    if(rQTSBufInfo.vBufInfo.size() == 0)
    {
        ACDK_LOGD("pass1 done deque no buffer");
    }
    else
    {
        imgo_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
        imgo_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
        imgo_memID = rQTSBufInfo.vBufInfo[0].memID;
        ACDK_LOGD("pass1 done,imgo_virAddr(0x%x)",imgo_virAddr);

        if(dumpEnable == 4)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            ACDK_LOGD("prv VA(0x%x)",imgo_virAddr);

            sprintf(szFileName, "%s/acdkCap1.bin" , MEDIA_PATH);

            //====== Write RAW Data ======

            FILE *pFp = fopen(szFileName, "wb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
                fclose(pFp);
            }
            else
            {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr;
                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalCapParam.capPass1ImemBuf.size, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("[.capPass1ImemBuf.size] fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("[.capPass1ImemBuf.size] Save image file name:%s, w(%u), h(%u)", szFileName, m_tgi.u4ImgWidth, m_tgi.u4ImgHeight);

                fclose(pFp);
            }
        }

        if(mAcdkMhalCapParam.captureType == RAW_TYPE)
        {
            //====== Raw Capture Callback ======

            acdkMhalCBHandle(ACDK_CB_RAW, imgo_virAddr);

            ACDK_LOGD("ACDK_CB_RAW back");
        }

        //====== Pass2 ======

        for(MUINT32 i = 0; i < 2; ++i)
        {
            //enqueue pass2 IN buffer
            rQBufInfo.vBufInfo.resize(1);

            mPortID.index = EPortIndex_IMGI;
            rQBufInfo.vBufInfo[0].u4BufVA = imgo_virAddr;
            rQBufInfo.vBufInfo[0].u4BufPA = imgo_phyAddr;
            rQBufInfo.vBufInfo[0].memID   = imgo_memID;
            rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalCapParam.capPass1ImemBuf.size;
            m_pPostProcPipe->enqueInBuf(mPortID, rQBufInfo);

            ACDK_LOGD("Update pass2 tuning cmdQ");

            //config pass2 every time

            g_vPostProcInPorts.at(0)  = &m_imgi;
            g_vPostProcOutPorts.at(0) = &m_vido;

            if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
            {
                ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
            }

            //set pass2 IN DMA register before pass2 start
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);

            //set pass2 OUT DMA register before pass2 start
            //m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);

            ACDK_LOGD("pass2 start");

            m_pPostProcPipe->start();

            ACDK_LOGD("waiting pass2 done");

            m_pPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);

            ACDK_LOGD("pass2 done");

            //====== Dequeue ======

            //dequeue pass2 out buffer
            mPortID.index = EPortIndex_VIDO;
            m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo);
            vido_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
            vido_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
            vido_memID   = rQTSBufInfo.vBufInfo[0].memID;

            //dequeue pass2 in buffer
            mPortID.index = EPortIndex_IMGI;
            m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
            imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
            imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
            imgi_memID   = rQTSBufInfo.vBufInfo[0].memID;

            m_pPostProcPipe->stop();
            ACDK_LOGD("pass2 stop");

            //====== Capture Callback ======

            ACDK_LOGD("vido_virAddr(0x%x)", vido_virAddr);

            if(mAcdkMhalCapParam.captureType == RAW_TYPE || i == 1)
            {
                acdkMhalCBHandle(ACDK_CB_QV, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_QV back");

                break;
            }
            else
            {
                acdkMhalCBHandle(ACDK_CB_JPEG, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_JPEG back");

                //====== Config PostProcPipe 2nd Run for QV after JPEG Capture ======

                m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
                m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
                m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
                m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];    // unit:PIXEL
                m_imgi.crop.x = 0;                          // unit:PIXEL
                m_imgi.crop.y = 0;                          // unit:PIXEL
                m_imgi.crop.w = m_imgi.u4ImgWidth;          // unit:PIXEL
                m_imgi.crop.h = m_imgi.u4ImgHeight;         // unit:PIXEL
                m_imgi.type        = EPortType_Memory;      //  EPortType
                m_imgi.index       = EPortIndex_IMGI;       //  port index
                m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
                m_imgi.pipePass    = EPipePass_PASS2;       //  select pass

                // config qv
                m_vido.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.qvParam.frmFormat;   // Image Pixel Format
                m_vido.eImgRot     = (EImageRotation)mAcdkMhalCapParam.qvParam.orientation;    //dispo NOT support rotation
                m_vido.eImgFlip    = mAcdkMhalCapParam.qvParam.flip ? eImgFlip_ON : eImgFlip_OFF;  //dispo NOT support flip
                m_vido.u4ImgWidth  = mAcdkMhalCapParam.qvParam.w;           // Image Width
                m_vido.u4ImgHeight = mAcdkMhalCapParam.qvParam.h;           // Image Height
                m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.qvParam.stride[0];
                m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.qvParam.stride[1];
                m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.qvParam.stride[2];
                m_vido.type        = EPortType_DISP_RDMA;   // EPortType
                m_vido.index       = EPortIndex_VIDO;       // port index
                m_vido.inout       = EPortDirection_Out;    // 0:in/1:out

                //set CQ first before pipe config
                m_pPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ1,0,0);

                // config IO port
                g_vPostProcInPorts.at(0) = &m_imgi;
                g_vPostProcOutPorts.at(0) = &m_vido;

                if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
                {
                    ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
                    break;
                }

                // enque buffer
                ACDK_LOGD("capQVImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capQVImemBuf.virtAddr);

                mPortID.index = EPortIndex_VIDO;    //VIDO
                rQBufInfo.vBufInfo.resize(1);
                rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalCapParam.capQVImemBuf.size;    //bytes
                rQBufInfo.vBufInfo[0].u4BufVA   = mAcdkMhalCapParam.capQVImemBuf.virtAddr;
                rQBufInfo.vBufInfo[0].u4BufPA   = mAcdkMhalCapParam.capQVImemBuf.phyAddr;
                rQBufInfo.vBufInfo[0].memID     = mAcdkMhalCapParam.capQVImemBuf.memID;

                m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
            }
        }
    }

    ACDK_LOGD("-");
#endif
    return ACDK_RETURN_NO_ERROR;
}

