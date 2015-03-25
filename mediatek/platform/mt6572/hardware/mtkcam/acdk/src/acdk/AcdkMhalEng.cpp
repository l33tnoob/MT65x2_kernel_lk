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

//! \file  AcdkMhalEng.cpp

#define LOG_TAG "AcdkMhalEng"

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

#include "imageio/IPipe.h"
#include "imageio/ICamIOPipe.h"
#include "imageio/ICdpPipe.h"
#include "imageio/IPostProcPipe.h"

using namespace NSImageio;
using namespace NSIspio;

#include "mtkcam/hal/sensor_hal.h"
//#include "isp_function.h"
#include "isp_drv.h"
#include "config/PriorityDefs.h"

#include "AcdkMhalEng.h"

#define MEDIA_PATH "/data"

/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd;
static pthread_t g_threadMainHigh;
static AcdkMhalEng *g_pAcdkMHalEngObj = NULL;
static acdkObserver g_acdkMhalObserver;

static vector<PortInfo const*> g_vCamIOInPorts(1);
static vector<PortInfo const*> g_vCamIOIOutPorts(1);
//static vector<PortInfo const*> g_vCdpInPorts(3);
//static vector<PortInfo const*> g_vCdpOutPorts(3);
static vector<PortInfo const*> g_vPostProcInPorts(3);
static vector<PortInfo const*> g_vPostProcOutPorts(3);

static MINT32 g_acdkMhalEngDebug = 0;

/*******************************************************************************
* AcdkMhalEng
* brif : Constructor
*******************************************************************************/
AcdkMhalEng::AcdkMhalEng()
{
    mAcdkMhalState = ACDK_MHAL_NONE;

    m_pICamIOPipe = NULL;
    m_pCdpPipe = NULL;
    m_pPostProcPipe = NULL;
    m_p3AHal = NULL;

    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

    mReadyForCap = MFALSE;

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    g_pAcdkMHalEngObj = this;
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkMhalEng::acdkMhalSetState(acdkMhalState_e newState)
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
acdkMhalState_e AcdkMhalEng::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalEng::acdkMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkMhalProcLoop(MVOID *arg)
{
    ::prctl(PR_SET_NAME,(unsigned long)"acdkMhalProcLoop",0,0,0);
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

    eState = g_pAcdkMHalEngObj->acdkMhalGetState();
    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalEngObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalEngObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalEngObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalEngObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);
    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalEng object
*******************************************************************************/
void AcdkMhalEng::destroyInstance()
{
    g_pAcdkMHalEngObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalInit()
{
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32  err = ACDK_RETURN_NO_ERROR;

    //====== Acdk Mhal State Setting ======

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    //====== Get Debug Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdk.debug", value, "0");
    g_acdkMhalEngDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalEngDebug(%d)",g_acdkMhalEngDebug);

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
MINT32 AcdkMhalEng::acdkMhalUninit()
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

        //====== 3A  Object Uninit ======

        if(m_p3AHal != NULL)
        {
            m_p3AHal->destroyInstance();
        }
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
MVOID AcdkMhalEng::acdkMhalCBHandle(MUINT32 a_type, MUINT32 a_addr1, MUINT32 a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"+");

    if(!g_acdkMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkMhal3ASetParm
* brif : set 3A parameter
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhal3ASetParam(MINT32 devID)
{
    ACDK_LOGD("devID(%d)",devID);

    Param_T cam3aParam;

    if(m_p3AHal == NULL)
    {
        m_p3AHal = Hal3ABase::createInstance(devID);
    }

    //====== 3AHal Set Parameter ======

    if(m_p3AHal == NULL)
    {
        ACDK_LOGE("m_p3AHal is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //get 3A parameter (default value)
    if(!m_p3AHal->getParams(cam3aParam))
    {
        ACDK_LOGE("m_p3AHal->getParams() fail");
        return ACDK_RETURN_API_FAIL;
    }

    //3A parameter setting
    cam3aParam.u4CamMode = eAppMode_PhotoMode;
    cam3aParam.u4AfMode         = 1;
    //Tuning parameter setting
    cam3aParam.u4BrightnessMode = 1;
    cam3aParam.u4HueMode        = 1;
    cam3aParam.u4SaturationMode = 1;
    cam3aParam.u4EdgeMode       = 1;
    cam3aParam.u4ContrastMode   = 1;

    //set zoom info
    m_p3AHal->setZoom(100, 0, 0, mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight);

    //set 3A parameter
    if(!m_p3AHal->setParams(cam3aParam))
    {
        ACDK_LOGE("m_p3AHal->setParams() fail");
        //return ACDK_RETURN_API_FAIL;
    }

    // set 3A callback
    if(!m_p3AHal->setCallbacks(g_pAcdkMHalEngObj))
    {
        ACDK_LOGE("m_p3AHal->setCallbacks() fail");
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalPreviewStart] Camera State is not IDLE");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalPrvParam, a_pBuffIn, sizeof(acdkMhalPrvParam_t));
    QBufInfo rQBufInfo;

    //====== Set 3A Parameter ======

    acdkMhal3ASetParam((MINT32)mAcdkMhalPrvParam.sensorID);

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
    m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.sensorStride[0]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalPrvParam.sensorStride[1]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalPrvParam.sensorStride[2]; //  unit:PIXEL
    m_imgo.type  = EPortType_Memory;        // EPortType
    m_imgo.index = EPortIndex_IMGO;         // port index
    m_imgo.inout = EPortDirection_Out;      // 0:in/1:out
    mu4SensorDelay = mAcdkMhalPrvParam.mu4SensorDelay; //Sensor Delay Frame

    //set CQ first before pipe config
    //m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0,0,0);
    //m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)CAM_CQ_SINGLE_IMMEDIATE_TRIGGER,(MINT32)CAM_CQ_TRIG_BY_START);

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

#if 0 /* unnecessary to enqu all buffer first */
    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        rQBufInfo.vBufInfo[0].u4BufVA = mAcdkMhalPrvParam.dispImemBuf[i].virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA = mAcdkMhalPrvParam.dispImemBuf[i].phyAddr;
        rQBufInfo.vBufInfo[0].memID   = mAcdkMhalPrvParam.dispImemBuf[i].memID;

        m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
    }
#else
    rQBufInfo.vBufInfo[0].u4BufVA = mAcdkMhalPrvParam.dispImemBuf[0].virtAddr;
    rQBufInfo.vBufInfo[0].u4BufPA = mAcdkMhalPrvParam.dispImemBuf[0].phyAddr;
    rQBufInfo.vBufInfo[0].memID   = mAcdkMhalPrvParam.dispImemBuf[0].memID;
    m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo); 
#endif
  

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
MINT32 AcdkMhalEng::acdkMhalPreviewStop()
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
            ACDK_LOGE("un know state(%d)", state);
        }
    }

    //====== Wait Semaphore ======

    ACDK_LOGD("Wait g_SemMainHighBack");
    ::sem_wait(&g_SemMainHighBack);
    ACDK_LOGD("Got g_SemMainHighBack");

    //====== 3A Scenario Control ======

    m_p3AHal->sendCommand(ECmd_CameraPreviewEnd);

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
MINT32 AcdkMhalEng::acdkMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Change State ======

    acdkMhalSetState(ACDK_MHAL_PRE_CAPTURE);

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureStart(MVOID *a_pBuffIn)
{

    ACDK_LOGD("+");
#if 0

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalCaptureStart] Camera State is not IDLE");

    //====== Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalCapParam, a_pBuffIn, sizeof(acdkMhalCapParam_t));

    //======  Capture Object ======

    ISingleShot *singleShot = ISingleShot::createInstance(eShotMode_NormalShot, "ACDK_NormalShot");
    if(singleShot == NULL)
    {
        ACDK_LOGE("singleShot create fail");
        return ACDK_RETURN_NULL_OBJ;
    }

    singleShot->init();

    //====== Setting Callback ======

    g_acdkMhalObserver = mAcdkMhalCapParam.acdkMainObserver;

    ACDK_LOGD("mAcdkMhalCapParam.capType = 0x%x", mAcdkMhalCapParam.capType);
    if(mAcdkMhalCapParam.capType & 0x3C)
    {
        singleShot->enableDataMsg(ECamShot_DATA_MSG_BAYER | ECamShot_DATA_MSG_POSTVIEW);
    }
    else if(mAcdkMhalCapParam.capType == JPEG_TYPE)
    {
        singleShot->enableDataMsg(ECamShot_DATA_MSG_JPEG | ECamShot_DATA_MSG_POSTVIEW);
    }

    //====== Set State to Capture State ======

    acdkMhalSetState(ACDK_MHAL_CAPTURE);

    //====== Capture Paramter Setting ======

    // shot param
    ShotParam rShotParam((EImageFormat)mAcdkMhalCapParam.capFormat, // yuv format
                          mAcdkMhalCapParam.capWidth,               // picutre width
                          mAcdkMhalCapParam.CapHeight,              // picture height
                          mAcdkMhalCapParam.capOrientation,         // picture rotation
                          mAcdkMhalCapParam.capFlip,                // picture flip => single shot not support
                          mAcdkMhalCapParam.qvFormat,               // postview format
                          mAcdkMhalCapParam.qvWidth,                // postview width
                          mAcdkMhalCapParam.qvHeight,               // postview height
                          mAcdkMhalCapParam.qvOrientation,          // postview rotation => no use. acdkMain should handle rotation by itself
                          mAcdkMhalCapParam.qvFlip,                 // postview flip => single shot not support
                          mAcdkMhalCapParam.zoom);

    ACDK_LOGD("rShotParam.ePictureFmt        = 0x%x", rShotParam.ePictureFmt);
    ACDK_LOGD("rShotParam.u4PictureWidth     = %u", rShotParam.u4PictureWidth);
    ACDK_LOGD("rShotParam.u4PictureHeight    = %u", rShotParam.u4PictureHeight);
    ACDK_LOGD("rShotParam.u4PictureRotation  = %u", rShotParam.u4PictureRotation);
    ACDK_LOGD("rShotParam.u4PictureFlip      = %u", rShotParam.u4PictureFlip);
    ACDK_LOGD("rShotParam.ePostViewFmt       = 0x%x", rShotParam.ePostViewFmt);
    ACDK_LOGD("rShotParam.u4PostViewWidth    = %u", rShotParam.u4PostViewWidth);
    ACDK_LOGD("rShotParam.u4PostViewHeight   = %u", rShotParam.u4PostViewHeight);
    ACDK_LOGD("rShotParam.u4PostViewRotation = %u", rShotParam.u4PostViewRotation);
    ACDK_LOGD("rShotParam.u4PostViewFlip     = %u", rShotParam.u4PostViewFlip);
    ACDK_LOGD("rShotParam.u4ZoomRatio        = %u", rShotParam.u4ZoomRatio);

    // jpeg param
    JpegParam rJpegParam(mAcdkMhalCapParam.jpegQuality, //Quality : 0 ~ 100
                         mAcdkMhalCapParam.jpegIsSOI);  //isSOI

    //ACDK_LOGD("rJpegParam.u4ThumbWidth   = %u", rJpegParam.u4ThumbWidth);
    //ACDK_LOGD("rJpegParam.u4ThumbHeight  = %u", rJpegParam.u4ThumbHeight);
    //ACDK_LOGD("rJpegParam.u4ThumbQuality = %u", rJpegParam.u4ThumbQuality);
    //ACDK_LOGD("rJpegParam.fgThumbIsSOI   = %u", rJpegParam.fgThumbIsSOI);
    ACDK_LOGD("rJpegParam.u4Quality = %u", rJpegParam.u4Quality);
    ACDK_LOGD("rJpegParam.fgIsSOI   = %u", rJpegParam.fgIsSOI);

    // sensor param
    SensorParam rSensorParam(mAcdkMhalCapParam.sensorDev,       // device ID
                             mAcdkMhalCapParam.sensorScenario,  // scenaio
                             mAcdkMhalCapParam.bitDepth,        // bit depth
                             mAcdkMhalCapParam.byPassDelay,     // bypass delay
                             mAcdkMhalCapParam.byPassScenario,  // bypass scenario
                             mAcdkMhalCapParam.rawTypr);        // RAW type : 0-pure raw, 1-processed raw

    ACDK_LOGD("rSensorParam.u4DeviceID      = %u", rSensorParam.u4DeviceID);
    ACDK_LOGD("rSensorParam.u4Scenario      = %u", rSensorParam.u4Scenario);
    ACDK_LOGD("rSensorParam.u4Bitdepth      = %u", rSensorParam.u4Bitdepth);
    ACDK_LOGD("rSensorParam.fgBypassDelay   = %u", rSensorParam.fgBypassDelay);
    ACDK_LOGD("rSensorParam.fgBypassScenaio = %u", rSensorParam.fgBypassScenaio);
    ACDK_LOGD("rSensorParam.u4RawType       = %u", rSensorParam.u4RawType);

    singleShot->setCallbacks(NULL, camShotDataCB, this);

    singleShot->setShotParam(rShotParam);

    singleShot->setJpegParam(rJpegParam);

    ACDK_LOGD("mAcdkMhalCapParam.capCnt(%u)",mAcdkMhalCapParam.capCnt);

    for(MUINT32 i = 0; i < mAcdkMhalCapParam.capCnt; ++i)
    {
        singleShot->startOne(rSensorParam);
    }

    err = singleShot->uninit();
    singleShot->destroyInstance();

    ACDK_LOGD("X");
    return err;
#else

    mFocusDone = MFALSE;

    m_p3AHal->autoFocus();

    ACDK_LOGD("wait focus done");
    while(mFocusDone == MFALSE)
    {
        usleep(200);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif
}


/*******************************************************************************
* acdkMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureStop()
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

    //====== Initialize Member Variable ======

    memset(&mAcdkMhalCapParam,0,sizeof(acdkMhalCapParam_t));

    //====== Set AcdkMhal State ======

    mReadyForCap = MFALSE;

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("X");
    return err;
#else
    mReadyForCap = MFALSE;
    mFocusDone = MFALSE;

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif
}


/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalPreviewProc()
{
    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"+");

    //====== Loca Variable Declaration ======

    MINT32  imgo_memID[OVERLAY_BUFFER_CNT] = {0};
    MUINT32 imgo_virAddr[OVERLAY_BUFFER_CNT] = {0}, imgo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINT32 imgi_memID, imgi_virAddr, imgi_phyAddr, dispo_memID, dispo_virAddr, dispo_phyAddr;
    MUINT32 imgo_num = 0;
    MBOOL isPreCap = MFALSE;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;
    int delayframecnt = 0;

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

    //3AHal control
    m_p3AHal->setIspProfile(EIspProfile_NormalPreview);

    //3A scenario control
    m_p3AHal->sendCommand(ECmd_CameraPreviewStart);

    // start CQ
    m_pICamIOPipe->startCQ0();

#if 0
    //cause CQ0 was configurred as SINGLE_IMMEDIATELY
    //m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ_NONE,0,0);
#else
    // because 3A need to dynamic update, so need to use continuous mode
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
#endif

    //sync VD
    if(m_tgi.index == EPortIndex_TG1I)
    {
        ACDK_LOGD("irq(EPipePass_PASS1_TG1)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_VSYNC);
    }
    else if(m_tgi.index == EPortIndex_TG2I)
    {
        ACDK_LOGD("irq(EPipePass_PASS1_TG2)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG2,EPIPEIRQ_VSYNC);
    }

    ACDK_LOGD("Preview pass1 start");
    m_pICamIOPipe->start();

    //pass1 continuous
    while((acdkMhalGetState() & ACDK_MHAL_PREVIEW_MASK) || (ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState()))
    {
        //====== In PreCapture State Or Not ======

        if(ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState() && isPreCap == MFALSE)
        {
            ACDK_LOGD("in precapture state");

            //====== 3A Scenario Control ======

            m_p3AHal->sendCommand(ECmd_PrecaptureStart);

            isPreCap = MTRUE;
        }

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
                ACDK_LOGD_DYN(g_acdkMhalEngDebug,"imgo_virAddr[%d](0x%x),imgo_phyAddr[%d](0x%x)",i,imgo_virAddr[i],i,imgo_phyAddr[i]);
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
                //fclose(pFp);
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

        //====== 3A Scenario Conotrol ======

        m_p3AHal->sendCommand(ECmd_Update);

        //====== Pass2 ======

        //enqueue pass2 IN buffer
        rQBufInfo.vBufInfo.resize(1);

        mPortID.index = EPortIndex_IMGI;
        rQBufInfo.vBufInfo[0].u4BufVA   = imgo_virAddr[0];   //always use one frame
        rQBufInfo.vBufInfo[0].u4BufPA   = imgo_phyAddr[0];   //always use one frame
        rQBufInfo.vBufInfo[0].memID     = imgo_memID[0];
        rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.imgImemBuf[0].size;
        m_pPostProcPipe->enqueInBuf(mPortID, rQBufInfo);

        ACDK_LOGD_DYN(g_acdkMhalEngDebug,"Update pass2 tuning cmdQ");

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

        ACDK_LOGD_DYN(g_acdkMhalEngDebug,"pass2 start");
        m_pPostProcPipe->start();

        ACDK_LOGD("waiting pass2 done");

        m_pPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);

        ACDK_LOGD("pass2 done");

        //dequeue pass2 in buffer
        mPortID.index = EPortIndex_IMGI;
        m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
        imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA;
        imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA;
        imgi_memID   = rQTSBufInfo.vBufInfo[0].memID;

        //dequeue pass2 out buffer
        //mPortID.index = EPortIndex_DISPO;
        mPortID.index = EPortIndex_VIDO;
        m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo);

        m_pPostProcPipe->stop();
        ACDK_LOGD_DYN(g_acdkMhalEngDebug,"pass2 stop");

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

            ACDK_LOGD_DYN(g_acdkMhalEngDebug,"dispo_virAddr(0x%x),dispo_phyAddr(0x%x)", dispo_virAddr,dispo_phyAddr);

            if (delayframecnt>=mu4SensorDelay)
            {
               acdkMhalCBHandle(ACDK_CB_PREVIEW, dispo_virAddr);
            }
            else
            {
               delayframecnt++;
            }

            //====== Enqueue Pass2 Out Buffer ======

            ACDK_LOGD_DYN(g_acdkMhalEngDebug,"prvCB back, enque dispo_virAddr(0x%x)",dispo_virAddr);

            // enqueue pass2 out buffer
            //mPortID.index = EPortIndex_DISPO;
            mPortID.index = EPortIndex_VIDO;
            rQBufInfo.vBufInfo[0].u4BufVA = dispo_virAddr;
            rQBufInfo.vBufInfo[0].u4BufPA = dispo_phyAddr;
            rQBufInfo.vBufInfo[0].memID   = dispo_memID;
            rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.dispImemBuf[0].size;
            m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
        }

        //====== Enqueue Pass1 Buffer ======

        for(MUINT32 i = 0; i < imgo_num; ++i)
        {
            ACDK_LOGD_DYN(g_acdkMhalEngDebug,"pass1 enque imgo_virAddr[%d](0x%x)",i,imgo_virAddr[i]);
            mPortID.index = EPortIndex_IMGO;
            rQBufInfo.vBufInfo[0].u4BufVA  = imgo_virAddr[i];
            rQBufInfo.vBufInfo[0].u4BufPA  = imgo_phyAddr[i];
            rQBufInfo.vBufInfo[0].memID    = imgo_memID[i];
            rQBufInfo.vBufInfo[0].u4BufSize = mAcdkMhalPrvParam.imgImemBuf[0].size;
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
        }

        ACDK_LOGD("pass1 enque done");
        imgo_num = 0;

        //====== Ready for Capture Or Not ======

        if(isPreCap == MTRUE)
        {
            if(MTRUE == m_p3AHal->isReadyToCapture())
            {
                ACDK_LOGD("ready for capture");

                acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);

                m_p3AHal->sendCommand(ECmd_PrecaptureEnd);

                mReadyForCap = MTRUE;
                isPreCap = MFALSE;
            }
        }
    }
    ACDK_LOGD_DYN(g_acdkMhalEngDebug,"-");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkMhalEng::acdkMhalCaptureProc()
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

    //3AHal control
    m_p3AHal->setIspProfile(EIspProfile_NormalCapture);

    m_pICamIOPipe->start(); //one time capture

    //3A scenario control
    m_p3AScenario->sendCommand(ECmd_CaptureStart);

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

/*******************************************************************************
* 
*******************************************************************************/
MUINT32 AcdkMhalEng::acdkMhalGetShutTime()
{
    m_p3AHal->getCaptureParams(0, 0, mCap3AParam);

    ACDK_LOGD("mCap3AParam.u4Eposuretime(%u)",mCap3AParam.u4Eposuretime);

    return mCap3AParam.u4Eposuretime;
    
}

/*******************************************************************************
* 
*******************************************************************************/
MVOID AcdkMhalEng::acdkMhalSetShutTime(MUINT32 a_time)
{
    // get and then set
    m_p3AHal->getCaptureParams(0, 0, mCap3AParam);

    ACDK_LOGD("ori(%u),new(%u)",mCap3AParam.u4Eposuretime,a_time);
    
    mCap3AParam.u4Eposuretime = a_time;
    m_p3AHal->updateCaptureParams(mCap3AParam);
}

/*******************************************************************************
* 
*******************************************************************************/
MUINT32 AcdkMhalEng::acdkMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkMhalGetAFInfo");
    u32AFInfo = g_pAcdkMHalEngObj->mFocusSucceed;

    return u32AFInfo;
    
}
/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkMhalEng::doNotifyCb(int32_t _msgType,
                                 int32_t _ext1,
                                 int32_t _ext2,
                                 int32_t _ext3)
{
    ACDK_LOGD("_msgType(%d),_ext1(%d)",_msgType,_ext1);

    if(_msgType == eID_NOTIFY_AF_FOCUSED)
    {
        if(_ext1 == 1)
        {
            ACDK_LOGD("Focus Succeed");
        }
        else if(_ext1 == 0)
        {
            ACDK_LOGD("Focus Fail");
        }

        g_pAcdkMHalEngObj->mFocusDone = MTRUE;
    }

    ACDK_LOGD("-");
}

/*******************************************************************************
* doDataCb
*******************************************************************************/
void AcdkMhalEng::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{

}



