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

//! \file  AcdkMain.cpp

#define LOG_TAG "AcdkMain"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <linux/cache.h>
#include <cutils/properties.h>

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <semaphore.h>
#include <linux/mtkfb.h>
}
#include <cutils/pmem.h>

using namespace android;

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkErrCode.h"
#include "AcdkLog.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkBase.h"
#include "AcdkMhalBase.h"
#include "AcdkUtility.h"

using namespace NSACDK;
using namespace NSAcdkMhal;

#include <mtkcam/imageio/IPipe.h>

using namespace NSImageio;
using namespace NSIspio;

#include "mtkcam/hal/sensor_hal.h"

#include "kd_imgsensor_define.h"
#include <mtkcam/drv/imem_drv.h>

//#include "AcdkMain.h"

#include <mtkcam/common/hw/hwstddef.h>
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
#include "mtkcam/exif/IBaseCamExif.h"
#include "mtkcam/exif/CamExif.h"

using namespace NSCamHW;
using namespace NSCamShot;
#include "AcdkMain.h"

#define CCT_TUNING_SUPPORT 1
#include "mtkcam/hal/aaa_hal_base.h"
#include "camera_custom_nvram.h"
#include "isp_tuning.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_param.h"
#include "isp_tuning_mgr.h"
#include "ae_mgr.h"
using namespace NS3A;

/******************************************************************************
* Define Value
*******************************************************************************/
#define LOG_TAG "AcdkMain"

#define MEDIA_PATH "/data"

#define SWAP(a, b) {MUINT32 c = (a); (a) = (b); (b) = c; }

static MINT32 g_acdkMainDebug = 0;
static MINT32 g_dumpRAW = 0;

/******************************************************************************
* Global Variable
*******************************************************************************/
static AcdkMain *g_pAcdkMainObj = NULL;

/*******************************************************************************
*
********************************************************************************/
void AcdkMain::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*Constructor
********************************************************************************/
AcdkMain::AcdkMain ()
    :AcdkBase()
    ,m_eAcdkMainState(ACDK_MAIN_NONE)
    ,mOperaMode(ACDK_OPT_NONE_MODE)
    ,mFocusDone(0)
    ,mFrameCnt(0)
    ,mPrvWidth(320)
    ,mPrvHeight(240)
    ,mPrvStartX(0)
    ,mPrvStartY(0)
    ,mOrientation(0)
    ,mTestPatternOut(0)
    ,mCapWidth(0)
    ,mCapHeight(0)
    ,mCapType(0)
    ,mQVWidth(0)
    ,mQVHeight(0)
    ,mUnPack(MFALSE)
    ,mIsSOI(MFALSE)
    ,mLCMOrientation(0)
    ,mSurfaceIndex(0)
    ,mSensorInit(MFALSE)
    ,mSupportedSensorDev(0)
    ,mSensorDev(0)
    ,mSensorType(0)
    ,mSensorOrientation(0)
    ,mSensorVFlip(0)
    ,mSensorHFlip(0)
    ,mSetShutTime(0)
    ,mGetShutTime(0)
    ,mGetAFInfo(0)
    ,mIsFacotory(MFALSE)
{

    ACDK_LOGD("+");

    //====== SurfaceView ======

    m_pAcdkSurfaceViewObj = NULL;
    m_pAcdkSurfaceViewObj = AcdkSurfaceView::createInstance();
    if(m_pAcdkSurfaceViewObj == NULL)
    {
        ACDK_LOGE("Can not create surface view obj");
    }

    //====== AcdkMhal ======

    m_pAcdkMhalObj = NULL;
    m_pAcdkMhalObj = AcdkMhalBase::createInstance();
    if(m_pAcdkMhalObj == NULL)
    {
        ACDK_LOGE("Can not create AcdkMhal obj");
    }

    //====== AcdkUtility =====

    m_pAcdkUtilityObj = NULL;
    m_pAcdkUtilityObj = AcdkUtility::createInstance();
    if(m_pAcdkUtilityObj == NULL)
    {
        ACDK_LOGE("Can not create AcdkUtility obj");
    }

    //====== Sensor =======

    m_pSensorHalObj= NULL;
    m_pSensorHalObj = SensorHal::createInstance();    // create sensor hal object
    if(m_pSensorHalObj == NULL)
    {
        ACDK_LOGE("Can not create SensorHal obj");
    }

    //======  Capture Object ======
    m_pSingleShot = NULL;   // single shot object   

    //====== Memory ======

    //IMEM
    m_pIMemDrv = IMemDrv::createInstance();

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
       mPrvIMemInfo[i].size = mPrvIMemInfo[i].virtAddr = mPrvIMemInfo[i].phyAddr = 0;
       mPrvIMemInfo[i].memID = -5;

       mDispIMemInfo[i].size = mDispIMemInfo[i].virtAddr = mDispIMemInfo[i].phyAddr = 0;
       mDispIMemInfo[i].memID = -5;
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        mSurfaceIMemInfo[i].size = mSurfaceIMemInfo[i].virtAddr = mSurfaceIMemInfo[i].phyAddr = 0;
        mSurfaceIMemInfo[i].memID = -5;
    }

    mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
    mRawIMemInfo.memID = -5;

    mJpgIMemInfo.size = mJpgIMemInfo.virtAddr = mJpgIMemInfo.phyAddr = 0;
    mJpgIMemInfo.memID = -5;

    mQvIMemInfo.size = mQvIMemInfo.virtAddr = mQvIMemInfo.phyAddr = 0;
    mQvIMemInfo.memID = -5;
    
    //====== Global Variable ======

    g_pAcdkMainObj = this;

    ACDK_LOGD("-");
}

/*******************************************************************************
*Destructor
********************************************************************************/
AcdkMain::~AcdkMain()
{
    ACDK_LOGD("+");

    g_pAcdkMainObj = NULL;

    if(m_pAcdkSurfaceViewObj != NULL)
    {
        m_pAcdkSurfaceViewObj->destroyInstance();
        m_pAcdkSurfaceViewObj = NULL;
    }

    if(m_pAcdkMhalObj != NULL)
    {
        m_pAcdkMhalObj->destroyInstance();
        m_pAcdkMhalObj = NULL;
    }

    if(m_pSensorHalObj != NULL)
    {
        m_pSensorHalObj->destroyInstance();
        m_pSensorHalObj = NULL;
    }

    if(m_pIMemDrv != NULL)
    {
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    g_acdkMainDebug = 0;
    g_dumpRAW = 0;
    
    ACDK_LOGD("-");
}

/*******************************************************************************
* acdkMainGetState
* brif : set state of AcdkMain
*******************************************************************************/
MVOID AcdkMain::acdkMainSetState(acdkMainState_e newState)
{    
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", m_eAcdkMainState, newState);
    
    if(newState == ACDK_MAIN_ERROR) 
    {
        goto ACDKMAIN_SET_STATE_EXIT;
    }
    
    switch(m_eAcdkMainState)
    {
    case ACDK_MAIN_NONE:
        switch(newState) 
        {
        case ACDK_MAIN_INIT:
        case ACDK_MAIN_UNINIT:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_NONE");
            break;
        }
        break;        
    case ACDK_MAIN_INIT:
        switch(newState) 
        {
        case ACDK_MAIN_IDLE:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_INIT");
            break;
        }
        break;    
    case ACDK_MAIN_IDLE:
        switch(newState)
        {
        case ACDK_MAIN_IDLE:
        case ACDK_MAIN_PREVIEW:
        case ACDK_MAIN_CAPTURE:
        case ACDK_MAIN_UNINIT:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_IDLE");
            break;
        }
        break;    
    case ACDK_MAIN_PREVIEW:
        switch(newState)
        {
        case ACDK_MAIN_IDLE:
        case ACDK_MAIN_PREVIEW:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_PREVIEW");
            break;
        }
        break;    
    case ACDK_MAIN_CAPTURE:
        switch(newState)
        {
        case ACDK_MAIN_IDLE:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_CAPTURE");
            break;
        }
        break;     
    case ACDK_MAIN_ERROR:
        switch(newState)
        {
        case ACDK_MAIN_IDLE:
        case ACDK_MAIN_UNINIT:
            break;
        default:            
            ACDK_LOGE("State error ACDK_MAIN_ERROR");
            break;
        }
        break;    
    default:
        ACDK_LOGE("Unknown state");
        break;
    }

ACDKMAIN_SET_STATE_EXIT:

    m_eAcdkMainState = newState;

    ACDK_LOGD("X, state(0x%04x)", m_eAcdkMainState);
}

/*******************************************************************************
* acdkMainGetState
* brif : get state of AcdkMain
*******************************************************************************/
acdkMainState_e AcdkMain::acdkMainGetState()
{   
    Mutex::Autolock _l(mLock);
    return m_eAcdkMainState;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::stateMgr(acdkMainState_e nextState)
{  
    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMainState_e curtState = acdkMainGetState();

    ACDK_LOGD("curtState(0x%x),nextState(0x%x)",curtState,nextState);

    if(curtState != ACDK_MAIN_IDLE)
    {
        if(curtState == ACDK_MAIN_PREVIEW)  // in preview state
        {
            if(nextState == ACDK_MAIN_PREVIEW)
            {
                ACDK_LOGD("Already preview");
                err = ACDK_RETURN_ERROR_STATE;
            }
            else if(nextState == ACDK_MAIN_CAPTURE)
            {               
                ACDK_LOGD("OK for Capture");
            }
            else
            {
                ACDK_LOGE("ACDK_MAIN_PREVIEW wrong state(0x%x)",nextState);
                err = ACDK_RETURN_INVALID_PARA;
            }
        }
        else if(curtState == ACDK_MAIN_CAPTURE) // in capture state
        {
            if(nextState == ACDK_MAIN_PREVIEW)
            {
                ACDK_LOGD("Capturing");
                while(ACDK_MAIN_CAPTURE == acdkMainGetState())
                {
                    usleep(200);
                }
                ACDK_LOGD("Capture done");
            }
            else if(nextState == ACDK_MAIN_CAPTURE)
            {
                ACDK_LOGD("Already capture");
                err = ACDK_RETURN_ERROR_STATE;
            }
            else
            {
                ACDK_LOGE("ACDK_MAIN_CAPTURE wrong state(0x%x)",nextState);
                err = ACDK_RETURN_INVALID_PARA;
            }
        }
        else
        {
            ACDK_LOGE("wrong state(0x%x)",nextState);
            err = ACDK_RETURN_INVALID_PARA;
        }
    } 

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::init()
{
    ACDK_LOGD("+");

    //====== Set State ======

    m_eAcdkMainState = ACDK_MAIN_INIT;

    //====== Local Variable =======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 width = 0, height = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //======  Object Checking =====

    if(m_pAcdkSurfaceViewObj == NULL)
    {
        ACDK_LOGE("Null SurfaceView Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pAcdkMhalObj == NULL)
    {
        ACDK_LOGE("Null AcdkMhal Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pAcdkUtilityObj == NULL)
    {
        ACDK_LOGE("Null AcdkUtilityObj Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    if(m_pIMemDrv == NULL)
    {
        ACDK_LOGE("Null IMemDrv Obj");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    //====== IMEM Init ======

    if(!m_pIMemDrv->init())
    {
        ACDK_LOGE("mpIMemDrv->init() error");
        err = ACDK_RETURN_NULL_OBJ;
        goto INIT_Exit;
    }

    //======  Surface View Setting ======

    err = m_pAcdkSurfaceViewObj->init();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("Faile to init surfaceview err(0x%x)", err);
        goto INIT_Exit;
    }

    m_pAcdkSurfaceViewObj->getSurfaceInfo(width, height, mLCMOrientation);

    ACDK_LOGD("width(%u),height(%u),mLCMOrientation(%u)",width,height,mLCMOrientation);

    //======  Sensor Init and Get Sensor Info======

    err = sensorInit();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("Sensor setting Fail. err(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto INIT_Exit;
    }

    err = getSensorInfo();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("getSensorInfo error(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto INIT_Exit;
    }

#if 0

    if(mLCMOrientation == 180)
    {
        mLCMOrientation = 0;
    }

    if(mSensorDev == SENSOR_DEV_SUB)
    {
        mOrientation = (mLCMOrientation - mSensorOrientation + 360) % 360;
    }
    else
    {
        mOrientation = (mLCMOrientation + mSensorOrientation) % 360;
    }
#else
    //For LCM 270 & subcam used(tablet)
    if((mSensorDev == SENSOR_DEV_SUB) && (mLCMOrientation == 270))
    {
        mOrientation = (mLCMOrientation + mSensorOrientation + 180) % 360;
    }
    else
    {
        mOrientation = (mLCMOrientation + mSensorOrientation) % 360;
    }
#endif
    calcPreviewWin(width, height, mPrvStartX, mPrvStartY, mPrvWidth, mPrvHeight);

    ACDK_LOGD("mOrientation(%u), mSensorOrientation(%u)", mOrientation, mSensorOrientation);
    ACDK_LOGD("prvStartX(%u), prvStartY(%u)", mPrvStartX, mPrvStartY);
    ACDK_LOGD("prvWidth(%u), prvHeight(%u)" ,mPrvWidth, mPrvHeight);

    //====== Initialize AcdkMhal ======

    err = m_pAcdkMhalObj->acdkMhalInit();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("mHalCamInit Fail(0x%x)", err);
        goto INIT_Exit;
    }

    //====== Get Debug Property ======

    property_get("camera.acdk.debug", value, "0");
    g_acdkMainDebug = atoi(value);

    ACDK_LOGD("g_acdkMainDebug(%d)",g_acdkMainDebug);

    //====== Set State ======

    acdkMainSetState(ACDK_MAIN_IDLE);
    
    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;

INIT_Exit:

    if(!m_pAcdkMhalObj)
    {
        m_pAcdkMhalObj->acdkMhalUninit();
    }

    if(!m_pAcdkSurfaceViewObj)
    {
        m_pAcdkSurfaceViewObj->uninit();
    }

    //====== Set State ======

    m_eAcdkMainState = ACDK_MAIN_NONE;

    ACDK_LOGD("Fail X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::uninit()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Set State ======

    m_eAcdkMainState = ACDK_MAIN_UNINIT;

    //====== AcdkMhal Uninit ======

    if(m_pAcdkMhalObj != NULL)
    {
        err = m_pAcdkMhalObj->acdkMhalUninit();
        if (err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit acdkMhalUninit(0x%x)", err);
        }
    }

    //====== AcdkSurface =====

    if(m_pAcdkSurfaceViewObj != NULL)
    {
        err = m_pAcdkSurfaceViewObj->uninit();
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit surfaceview(0x%x)", err);
        }
    }

    //====== AcdkUtility =====

    if(m_pAcdkUtilityObj != NULL)
    {
        m_pAcdkUtilityObj->destroyInstance();
        m_pAcdkUtilityObj = NULL;
    }

    //====== Sensor Hal ======

    if(m_pSensorHalObj != NULL)
    {
        err = m_pSensorHalObj->uninit();
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("Faile to uninit sensorHal(0x%x)", err);
        }
    }

    //====== Free Surface Memory ======

    if(mSurfaceIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(SURFACE_NUM, mSurfaceIMemInfo);
    }

    if(m_pIMemDrv != NULL)
    {
        if(!m_pIMemDrv->uninit())
        {
            ACDK_LOGE("Faile to uninit m_pIMemDrv");
        }
    }

    //====== Set State ======

    m_eAcdkMainState = ACDK_MAIN_NONE;
    
    ACDK_LOGD("-");

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::sensorInit()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 index = 0, subIndex = 0, sensorFound = 0;
    acdkMainSensorInfo_t camSensorInfo[8];

    //====== Check Object ======

    if(m_pSensorHalObj == NULL)
    {
        ACDK_LOGE("m_pSensorHalObj is NULL");
        err = ACDK_RETURN_NULL_OBJ;
        goto sensorInitExit;
    }

    //====== Sensor Setting =====

    //search sensor
    mSupportedSensorDev = m_pSensorHalObj->searchSensor();
    ACDK_LOGD("mSupportedSensorDev:0x%x", mSupportedSensorDev);

    // set each sensor type info
    if(mSupportedSensorDev & SENSOR_DEV_MAIN)
    {
        camSensorInfo[index].facing  = 0;    // back
        camSensorInfo[index].devType = SENSOR_DEV_MAIN;
        ++index;
    }

    if(mSupportedSensorDev & SENSOR_DEV_SUB)
    {
        camSensorInfo[index].facing  = 1;   // front
        camSensorInfo[index].devType = SENSOR_DEV_SUB;
        subIndex = index;
        ++index;
    }

    if(mSupportedSensorDev & SENSOR_DEV_ATV)
    {
        camSensorInfo[index].facing  = 0;
        camSensorInfo[index].devType = SENSOR_DEV_ATV;
        ++index;
    }

    if(mSupportedSensorDev & SENSOR_DEV_MAIN_2)
    {
        camSensorInfo[index].facing  = 0;
        camSensorInfo[index].devType = SENSOR_DEV_MAIN_2;
        ++index;
    }

    // set Main as default
    if(mSensorDev == SENSOR_DEV_NONE)
    {
        mSensorDev = SENSOR_DEV_MAIN;
        mSensorVFlip = 0;
        mSensorHFlip = 0;
    }

    // set current sensor type
    for(MINT32 i = 0; i < index; ++i)
    {
        if (camSensorInfo[i].devType == mSensorDev)
        {
            sensorFound = 1;
            break;
        }
    }

    if(sensorFound == 0)
    {
        ACDK_LOGE("sensor not found");
        err = ACDK_RETURN_INVALID_SENSOR;
        goto sensorInitExit;
    }
    else
    {
        ACDK_LOGD("mSensorDev   = %d",mSensorDev);
        ACDK_LOGD("mSensorVFlip = %d",mSensorVFlip);
        ACDK_LOGD("mSensorHFlip = %d",mSensorHFlip);
    }

    //====== Sensor Init======

    //set current sensor device
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                       SENSOR_CMD_SET_SENSOR_DEV,
                                       0,
                                       0,
                                       0);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_SET_SENSOR_DEV fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto sensorInitExit;
    }

    // init sensor
    err = m_pSensorHalObj->init();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pSensorHalObj->init() fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto sensorInitExit;
    }
    else
    {
        mSensorInit = MTRUE;
    }

sensorInitExit:

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::getSensorInfo()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 mode;

    //====== Check Object ======

    if(m_pSensorHalObj == NULL)
    {
        ACDK_LOGE("m_pSensorHalObj is NULL");
        err = ACDK_RETURN_NULL_OBJ;
        goto getSensorInfoExit;
    }

    //====== Get Sensor Info ======

    // get preview range
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                        (MINT32)&mSensorResolution.SensorPreviewWidth,
                                        (MINT32)&mSensorResolution.SensorPreviewHeight,
                                        0);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_SENSOR_PRV_RANGE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    // get sensor full range
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                        (MINT32)&mSensorResolution.SensorFullWidth,
                                        (MINT32)&mSensorResolution.SensorFullHeight,
                                        0);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_SENSOR_FULL_RANGE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    // get sensor video range
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                        (MINT32)&mSensorResolution.SensorVideoWidth,
                                        (MINT32)&mSensorResolution.SensorVideoHeight,
                                        0);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_SENSOR_FULL_RANGE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    // get RAW or YUV
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_TYPE,
                                        (MINT32)&mSensorType,
                                        0,
                                        0);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_SENSOR_TYPE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    //get sernsor orientation angle
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE,
                                        (MINT32)&mSensorOrientation,
                                        0,
                                        0);

    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    //get sensor format info
    memset(&mSensorFormatInfo, 0, sizeof(halSensorRawImageInfo_t));

    err =m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                       SENSOR_CMD_GET_RAW_INFO,
                                       (MINT32)&mSensorFormatInfo,
                                       1,
                                       0);

    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_RAW_INFO fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    //get sensor test pattern checksum value 
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,
                                        (MINT32)&mGetCheckSumValue,
                                        0,
                                        0);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;
    }

    //get sensro delay frame count

    mode = SENSOR_PREVIEW_DELAY;
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev, SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT, 
        (MINT32)&mu4SensorDelay, (MINT32)&mode,0);

    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto getSensorInfoExit;		
    }

    ACDK_LOGD("mSensorDev(%d)",mSensorDev);
    ACDK_LOGD("0-RAW,1-YUV(%d)",mSensorType);
    ACDK_LOGD("preview size : w(%u),h(%u)", mSensorResolution.SensorPreviewWidth, mSensorResolution.SensorPreviewHeight);
    ACDK_LOGD("full size    : w(%u),h(%u)", mSensorResolution.SensorFullWidth, mSensorResolution.SensorFullHeight);
    ACDK_LOGD("video size   : w(%u),h(%u)", mSensorResolution.SensorVideoWidth, mSensorResolution.SensorVideoHeight);
    ACDK_LOGD("bit depth(%u)",mSensorFormatInfo.u4BitDepth);
    ACDK_LOGD("isPacked(%u)",mSensorFormatInfo.u4IsPacked);
    ACDK_LOGD("color order(%u)",mSensorFormatInfo.u1Order);
    ACDK_LOGD("Checksum value(0x%x)",mGetCheckSumValue);
    ACDK_LOGD("Delay frame(0x%x)",mu4SensorDelay);


getSensorInfoExit:

    ACDK_LOGD("-");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::calcPreviewWin(
                   MUINT32 const surfaceWidth,
                   MUINT32 const surfaceHeight,
                   MUINT32 &x,
                   MUINT32 &y,
                   MUINT32 &width,
                   MUINT32 &height)
{
    ACDK_LOGD("+");

    //====== Local Variable ======

    MUINT32 offset = 0;
    MUINT32 tempW = surfaceWidth, tempH = surfaceHeight;
    MUINT32 degree = mOrientation;

    //====== Calculation ======

    // decide preview size && offset
    // the screen scan direction has a angle's shift to camera sensor

    if(degree == 90 || degree== 270) //Sensor need to rotate
    {
        tempW = surfaceHeight;
        tempH = surfaceWidth;
    }

    if(tempW > tempH)
    {
        width  = (tempH / 3 * 4); 
        height = (width / 4 * 3);         
    }
    else
    {
        height = (tempW / 4 * 3); 
        width  = (height / 3 * 4);         
    }

    m_pAcdkUtilityObj->queryPrvSize(width,height);

    x = (tempW - width)  / 2;
    y = (tempH - height) / 2;

    if(degree == 90 || degree== 270)    //Sensor need to rotate
    {
        SWAP(x, y);
        SWAP(width, height);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    ACDK_LOGD("bufCnt(%d)", bufCnt);

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    ACDK_LOGD("memSize(%u),alingSize(%u)", memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->allocVirtBuf() error");
            err = ACDK_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->mapPhyAddr() error");
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkMain::destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    ACDK_LOGD("bufCnt(%d)", bufCnt);

    MINT32 err = ACDK_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                ACDK_LOGD("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                ACDK_LOGE("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = ACDK_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            ACDK_LOGD("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->unmapPhyAddr() error");
            err = ACDK_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            ACDK_LOGE("m_pIMemDrv->freeVirtBuf() error");
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkMain::sensorFormatSetting(MUINT32 mode, MUINT32 &imgFormat, MUINT32 &imgSize, MUINT32 *imgStride)
{
    ACDK_LOGD("+");

    MUINT32 tempRAWPixelByte = 0;
    MUINT32 tempWidth, tempHeight;
    MUINT32 tempSize,tempStride,tempFormat;

    //====== Mode Setting ======

    if(mode == PREVIEW_MODE)
    {
        tempWidth  = mSensorResolution.SensorPreviewWidth;
        tempHeight = mSensorResolution.SensorPreviewHeight;
        tempStride = mSensorResolution.SensorPreviewWidth;
    }
    else if(mode == VIDEO_MODE)
    {
        tempWidth  = mSensorResolution.SensorVideoWidth;
        tempHeight = mSensorResolution.SensorVideoHeight;
        tempStride = mSensorResolution.SensorVideoWidth;
    }
    else
    {
        tempWidth  = mSensorResolution.SensorFullWidth;
        tempHeight = mSensorResolution.SensorFullHeight;
        tempStride = mSensorResolution.SensorFullWidth;
    }

    //====== Format Setting ======

    if(mSensorType == SENSOR_TYPE_RAW)  // RAW
    {
        //get RAW image bit depth
        switch(mSensorFormatInfo.u4BitDepth)
        {
            case 8 : tempFormat  = eImgFmt_BAYER8;
                break;
            case 10 : tempFormat = eImgFmt_BAYER10;
                break;
            case 12 : tempFormat = eImgFmt_BAYER12;
                break;
            default : tempFormat = eImgFmt_UNKNOWN;
                      ACDK_LOGE("unknown raw image bit depth(%u)",mSensorFormatInfo.u4BitDepth);
                      return ACDK_RETURN_INVALID_PARA;
        }

        // calculate real stride and get byte per pixel. for RAW sensor only
        if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryRAWImgFormatInfo(tempFormat,tempWidth,tempStride,tempRAWPixelByte))
        {
            ACDK_LOGE("queryRAWImgFormatInfo fail");
            return ACDK_RETURN_API_FAIL;
        }
        else
        {
            tempSize = ceil(tempStride * tempHeight * (tempRAWPixelByte / 4.0));
        }

        // set value
        imgSize   = tempSize;
        imgFormat = tempFormat;
        ACDK_LOGD("RAW : imgSize(%u),imgFormat(0x%x)",imgSize,imgFormat);

        if(imgStride != NULL)
        {
            imgStride[0] = tempStride;
            imgStride[1] = 0;
            imgStride[2] = 0;
            ACDK_LOGD("RAW : imgStride[0](%u)",imgStride[0]);
        }
    }
    else if(mSensorType == SENSOR_TYPE_YUV) //YUV
    {
        //mapping YUV format between Sensor_Hal and CamIOPipe
        //Cb = U, Cr = V
        switch(mSensorFormatInfo.u1Order)
        {
            case SENSOR_OUTPUT_FORMAT_UYVY :
            case SENSOR_OUTPUT_FORMAT_CbYCrY :
                    tempFormat = eImgFmt_UYVY;
                break;
            case SENSOR_OUTPUT_FORMAT_VYUY :
            case SENSOR_OUTPUT_FORMAT_CrYCbY :
                    tempFormat = eImgFmt_VYUY;
                break;
            case SENSOR_OUTPUT_FORMAT_YUYV :
            case SENSOR_OUTPUT_FORMAT_YCbYCr :
                    tempFormat = eImgFmt_YUY2;
                break;
            case SENSOR_OUTPUT_FORMAT_YVYU :
            case SENSOR_OUTPUT_FORMAT_YCrYCb :
                    tempFormat = eImgFmt_YVYU;
                break;
            default : tempFormat = eImgFmt_UNKNOWN;
                      ACDK_LOGE("unknown YUV type(0x%x)",mSensorFormatInfo.u1Order);
                      return ACDK_RETURN_INVALID_PARA;
        }

        // calculate image size
        if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageSize(tempFormat,tempWidth,tempHeight,tempSize))
        {
            ACDK_LOGE("YUV - queryImageSize fail");
            return ACDK_RETURN_API_FAIL;
        }

        // calculate image stride

        if(imgStride != NULL)
        {
            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,0,&imgStride[0]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(0) : %d",imgStride[0]);
                return ACDK_RETURN_API_FAIL;
            }

            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,1,&imgStride[1]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(1) : %d",imgStride[1]);
                return ACDK_RETURN_API_FAIL;
            }

            if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryImageStride(tempFormat,tempWidth,2,&imgStride[2]))
            {
                ACDK_LOGE("YUV - queryImageStride fail(2) : %d",imgStride[2]);
                return ACDK_RETURN_API_FAIL;
            }

            ACDK_LOGD("YUV - imgStride[0](%u),imgStride[1](%u),imgStride[2](%u)",imgStride[0],
                                                                                 imgStride[1],
                                                                                 imgStride[2]);
        }
        // set value
        imgSize = tempSize;
        imgFormat = tempFormat;
        ACDK_LOGD("YUV - imgSize(%u),imgFormat(0x%x)",imgSize,imgFormat);

    }
    else
    {
        ACDK_LOGD("sensor type not yet");
    }

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::startPreview(Func_CB prvCb)
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MINT32 continuous = 1;
    MUINT32 sensorImgSize = 0;
    MUINT32 displaySize,sensorFormat;
    //OVL support YUY2/RGB565 
    //MUINT32 dispalyFormat = eImgFmt_RGB565;
    MUINT32 dispalyFormat = eImgFmt_YUY2;
    MUINT32 sensorStride[3] = {0}, dispalyStride[3] = {0};

    //====== Check and Set State ======

    err = stateMgr(ACDK_MAIN_PREVIEW);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }

    acdkMainSetState(ACDK_MAIN_PREVIEW);

    //====== Calculate Memory Size ======

    // calculate pass1 frame buffer size and stride
    err = sensorFormatSetting(PREVIEW_MODE,sensorFormat,sensorImgSize,&sensorStride[0]);
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("sensorFormatSetting fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }

    //calculate display frame buffer size and stride
    m_pAcdkUtilityObj->queryImageSize(dispalyFormat,mPrvWidth,mPrvHeight,displaySize);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,0,&dispalyStride[0]);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,1,&dispalyStride[1]);
    m_pAcdkUtilityObj->queryImageStride(dispalyFormat,mPrvWidth,2,&dispalyStride[2]);

    ACDK_LOGD("sensorImgSize(%u),displaySize(%u)",sensorImgSize,displaySize);

    //====== Allocate Memory ======

    createMemBuf(sensorImgSize, OVERLAY_BUFFER_CNT, mPrvIMemInfo);      // pass1 - sensor in
    createMemBuf(displaySize,   OVERLAY_BUFFER_CNT, mDispIMemInfo);     // pass2 - video out

    if(mSurfaceIMemInfo[0].virtAddr == 0)
    {
        createMemBuf(displaySize, SURFACE_NUM, mSurfaceIMemInfo);  // surface - frame buffer
    }

    ACDK_LOGD("mPrvIMemInfo  : size(sensorImgSize) = %u",mPrvIMemInfo[0].size);
    ACDK_LOGD("mDispIMemInfo : size(displaySize)   = %u",mDispIMemInfo[0].size);
    ACDK_LOGD("mSurfaceIMemInfo : size(displaySize) = %u",mSurfaceIMemInfo[0].size);

    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        ACDK_LOGD("prvVA[%d]  = 0x%x, prvPA[%d]  = 0x%x",i,mPrvIMemInfo[i].virtAddr,i,mPrvIMemInfo[i].phyAddr);
        ACDK_LOGD("dispVA[%d] = 0x%x, dispPA[%d] = 0x%x",i,mDispIMemInfo[i].virtAddr,i,mDispIMemInfo[i].phyAddr);
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {        
        ACDK_LOGD("surfaceVA[%d] = 0x%x, surfacePA[%d] = 0x%x",i,mSurfaceIMemInfo[i].virtAddr,i,mSurfaceIMemInfo[i].phyAddr);
    }

    // init value
    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        if(0 == mPrvIMemInfo[i].virtAddr && 0 == mPrvIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mPrvIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            memset((MVOID *)mPrvIMemInfo[i].virtAddr, 0, mPrvIMemInfo[i].size);
        }

        if(0== mDispIMemInfo[i].virtAddr && 0 == mDispIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mDispIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            memset((MVOID *)mDispIMemInfo[i].virtAddr, 0, mDispIMemInfo[i].size);
        }
    }

    for(MINT32 i = 0; i < SURFACE_NUM; ++i)
    {
        if(0 == mSurfaceIMemInfo[i].virtAddr && 0 == mSurfaceIMemInfo[i].phyAddr)
        {
            ACDK_LOGE("mSurfaceIMemInfo[%d] Get the memory fail",i);
            err = ACDK_RETURN_MEMORY_ERROR;
            goto startPreviewExit;
        }
        else
        {
            //for OVL output using rgb565 = 0x0000 -->black color
            //for OVL output using yuv422 = 0x0000 --> Green color , 0x0080 --> Black color
            //only first frame reset 
            if(mFrameCnt == 0)
            {
                memset((MVOID *)mSurfaceIMemInfo[i].virtAddr, 0, mSurfaceIMemInfo[i].size);
            }
         }
    }

#if 0   //cotta--Zaikuo: ION no need to register buffer

    err = m_pAcdkSurfaceViewObj->registerBuffer(mSurfaceIMemInfo.virtAddr, mSurfaceIMemInfo.size);
    if (err != 0)
    {
        ACDK_LOGE("m_pAcdkSurfaceViewObj->registerBuffer() fail err = 0x%x", err);
        goto startPreviewExit;
    }

#endif

    //====== Config Sensor ======

    halSensorIFParam_t sensorHalParam[2];

    memset(sensorHalParam, 0, sizeof(halSensorIFParam_t) * 2);

    sensorHalParam[0].u4SrcW = mSensorResolution.SensorPreviewWidth;
    sensorHalParam[0].u4SrcH = mSensorResolution.SensorPreviewHeight;
    sensorHalParam[0].u4CropW = mSensorResolution.SensorPreviewWidth;
    sensorHalParam[0].u4CropH = mSensorResolution.SensorPreviewHeight;
    sensorHalParam[0].u4IsContinous = continuous;
    sensorHalParam[0].u4IsBypassSensorScenario = 0;
    sensorHalParam[0].u4IsBypassSensorDelay = continuous ? 1 : 0;
    sensorHalParam[0].scenarioId= ACDK_SCENARIO_ID_CAMERA_PREVIEW;

    sensorHalParam[1].u4SrcW = mSensorResolution.SensorPreviewWidth;
    sensorHalParam[1].u4SrcH = mSensorResolution.SensorPreviewHeight;
    sensorHalParam[1].u4CropW = mSensorResolution.SensorPreviewWidth;
    sensorHalParam[1].u4CropH = mSensorResolution.SensorPreviewHeight;
    sensorHalParam[1].u4IsContinous = continuous;
    sensorHalParam[1].u4IsBypassSensorScenario = 0;
    sensorHalParam[1].u4IsBypassSensorDelay = continuous ? 1 : 0;
    sensorHalParam[1].scenarioId= ACDK_SCENARIO_ID_CAMERA_PREVIEW;

    if(mSensorDev == SENSOR_DEV_MAIN || mSensorDev == SENSOR_DEV_ATV)   //main sensor & atv use sensorHalaram[0]
    {
        ACDK_LOGD("main/atv-Continous=%u",sensorHalParam[0].u4IsContinous);
        ACDK_LOGD("main/atv-BypassSensorScenario=%u",sensorHalParam[0].u4IsBypassSensorScenario);
        ACDK_LOGD("main/atv-BypassSensorDelay=%u",sensorHalParam[0].u4IsBypassSensorDelay);
    }
    else if(mSensorDev == SENSOR_DEV_SUB || mSensorDev == SENSOR_DEV_MAIN_2)     // main2 & sub sensor
    {
        ACDK_LOGD("main2/sub-Continous=%u",sensorHalParam[1].u4IsContinous);
        ACDK_LOGD("main2/sub-BypassSensorScenario=%u",sensorHalParam[1].u4IsBypassSensorScenario);
        ACDK_LOGD("main2/sub-BypassSensorDelay=%u",sensorHalParam[1].u4IsBypassSensorDelay);
    }

    err = m_pSensorHalObj->setConf(sensorHalParam);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pSensorHalObj->setConf() fail(0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
        goto startPreviewExit;
    }

    if(mTestPatternOut)
    {
        MINT32 u32Enable = 1;
        err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MINT32)&u32Enable,
                                           0,
                                           0);
    }


    //====== Preview Parameter Setting ======

    memset(&mAcdkMhalPrvParam, 0, sizeof(acdkMhalPrvParam_t));        
    
    //preview param setting
    mAcdkMhalPrvParam.scenarioHW         = eScenarioID_VSS;
    mAcdkMhalPrvParam.sensorID           = (halSensorDev_e)mSensorDev;
    mAcdkMhalPrvParam.sensorWidth        = mSensorResolution.SensorPreviewWidth;
    mAcdkMhalPrvParam.sensorHeight       = mSensorResolution.SensorPreviewHeight;
    mAcdkMhalPrvParam.sensorStride[0]    = sensorStride[0];
    mAcdkMhalPrvParam.sensorStride[1]    = sensorStride[1];
    mAcdkMhalPrvParam.sensorStride[2]    = sensorStride[2];
    mAcdkMhalPrvParam.sensorType         = mSensorType;
    mAcdkMhalPrvParam.sensorFormat       = sensorFormat;
    mAcdkMhalPrvParam.sensorColorOrder   = mSensorFormatInfo.u1Order;
    mAcdkMhalPrvParam.mu4SensorDelay     = mu4SensorDelay;
    mAcdkMhalPrvParam.imgImemBuf         = mPrvIMemInfo;
    mAcdkMhalPrvParam.dispImemBuf        = mDispIMemInfo;
    mAcdkMhalPrvParam.frmParam.w         = mPrvWidth;
    mAcdkMhalPrvParam.frmParam.h         = mPrvHeight;
    mAcdkMhalPrvParam.frmParam.flip      = mSensorHFlip ? MTRUE : MFALSE;
    mAcdkMhalPrvParam.frmParam.stride[0] = dispalyStride[0];
    mAcdkMhalPrvParam.frmParam.stride[1] = dispalyStride[1];
    mAcdkMhalPrvParam.frmParam.stride[2] = dispalyStride[2];
    mAcdkMhalPrvParam.frmParam.frmFormat = dispalyFormat;
    mAcdkMhalPrvParam.acdkMainObserver   = acdkObserver(cameraCallback, this);
    if(mIsFacotory)
    {
        mAcdkMhalPrvParam.IsFactoryMode = 1;
    }
    switch(mOrientation)
    {
        case 0 : mAcdkMhalPrvParam.frmParam.orientation = eImgRot_0;
            break;
        case 90 : mAcdkMhalPrvParam.frmParam.orientation = eImgRot_90;
            break;
        case 180 : mAcdkMhalPrvParam.frmParam.orientation = eImgRot_180;
            break;
        case 270 : mAcdkMhalPrvParam.frmParam.orientation = eImgRot_270;
            break;
    }

    ACDK_LOGD("scenarioHW      = %d", (MINT32)mAcdkMhalPrvParam.scenarioHW);
    ACDK_LOGD("sensorID        = %d", (MINT32)mAcdkMhalPrvParam.sensorID);
    ACDK_LOGD("sensorWidth     = %u", mAcdkMhalPrvParam.sensorWidth);
    ACDK_LOGD("sensorHeight    = %u", mAcdkMhalPrvParam.sensorHeight);
    ACDK_LOGD("sensorStride[0] = %u", mAcdkMhalPrvParam.sensorStride[0]);
    ACDK_LOGD("sensorStride[1] = %u", mAcdkMhalPrvParam.sensorStride[1]);
    ACDK_LOGD("sensorStride[2] = %u", mAcdkMhalPrvParam.sensorStride[2]);
    ACDK_LOGD("sensorType      = %u", mAcdkMhalPrvParam.sensorType);
    ACDK_LOGD("sensorFormat    = 0x%x", mAcdkMhalPrvParam.sensorFormat);
    ACDK_LOGD("colorOrder      = %u", mAcdkMhalPrvParam.sensorColorOrder);
    ACDK_LOGD("frmParam.w      = %u", mAcdkMhalPrvParam.frmParam.w);
    ACDK_LOGD("frmParam.h      = %u", mAcdkMhalPrvParam.frmParam.h);
    ACDK_LOGD("frmParam.orientation = %d", mAcdkMhalPrvParam.frmParam.orientation);
    ACDK_LOGD("frmParam.flip   = %d", mAcdkMhalPrvParam.frmParam.flip);
    ACDK_LOGD("frmParam.stride[0] = %u", mAcdkMhalPrvParam.frmParam.stride[0]);
    ACDK_LOGD("frmParam.stride[1] = %u", mAcdkMhalPrvParam.frmParam.stride[1]);
    ACDK_LOGD("frmParam.stride[2] = %u", mAcdkMhalPrvParam.frmParam.stride[2]);
    ACDK_LOGD("frmParam.frmFormat = 0x%x", mAcdkMhalPrvParam.frmParam.frmFormat);

    //====== AcdkMhal PreviewStart ======

    err = m_pAcdkMhalObj->acdkMhalPreviewStart(&mAcdkMhalPrvParam);
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("preview start fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

startPreviewExit:

    ACDK_LOGD("-");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::stopPreview()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== AcdkMhal PreviewStop ======

    err = m_pAcdkMhalObj->acdkMhalPreviewStop();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview Fail(err=0x%x)",err);
        err = ACDK_RETURN_API_FAIL;
    }

    //====== Surface Uninit ======

#if 0   //cotta--Zaikuo : ION no need to do this

    ACDK_LOGD("unregisterBuffer addr = 0x%x", mSurfaceIMemInfo.virtAddr);
    err = m_pAcdkSurfaceViewObj->unRegisterBuffer( mSurfaceIMemInfo.virtAddr);
    if (err != 0)
    {
        ACDK_LOGE("m_pAcdkSurfaceViewObj->registerBuffer() fail err = 0x%x ", err);
    }

#endif

    //====== Free Memory ======

    if(mPrvIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mPrvIMemInfo);
    }

    if(mDispIMemInfo[0].virtAddr != 0)
    {
        destroyMemBuf(OVERLAY_BUFFER_CNT, mDispIMemInfo);
    }

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
       mPrvIMemInfo[i].size = mPrvIMemInfo[i].virtAddr = mPrvIMemInfo[i].phyAddr = 0;
       mPrvIMemInfo[i].memID = -5;

       mDispIMemInfo[i].size = mDispIMemInfo[i].virtAddr = mDispIMemInfo[i].phyAddr = 0;
       mDispIMemInfo[i].memID = -5;
    }

    //====== Set State ======

    acdkMainSetState(ACDK_MAIN_IDLE);
    
    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::takePicture(
                MUINT32 const mode,
                MUINT32 const imgType,
                Func_CB const capCb,
                MUINT32 const width,
                MUINT32 const height,
                MUINT32 const captureCnt,
                MINT32  const isSaveImg)
{
    ACDK_LOGD("+");

    //====== Local Variable ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 sensorScenarioId;
    //VIDO output format depend on OVL used  (eImgFmt_RGB565 /eImgFmt_YUY2)
    MUINT32 qvFormat = eImgFmt_YUY2, capFormat = eImgFmt_YUY2;
    MUINT32 rawBit = mSensorFormatInfo.u4BitDepth, rawType = 0;

    //====== Check State ======

    err = stateMgr(ACDK_MAIN_CAPTURE);
    if(err == ACDK_RETURN_ERROR_STATE)
    {
        ACDK_LOGD("warning: redundent command. protect!!");
        return ACDK_RETURN_NO_ERROR;
    }
    else if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stateMgr fail(0x%x)",err);
        return err;
    }
    
    //====== Create Object of Single Shot ======

    m_pSingleShot = ISingleShot::createInstance(eShotMode_NormalShot, "ACDK_NormalShot");
    if (m_pSingleShot == NULL)
    {
        ACDK_LOGE("m_pSingleShot create fail");
        return ACDK_RETURN_NULL_OBJ;
    }

    // special raw dump
    char rawVal[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdkdump.raw", rawVal, "0");  // 4 : 8bit-pure, 8 : 10bit-pure, 16 : 8bit-processed, 32 : 10bit-processed
    g_dumpRAW = atoi(rawVal);

    if(g_dumpRAW != 0 && g_dumpRAW != 4 && g_dumpRAW != 8 && g_dumpRAW != 16 && g_dumpRAW != 32)
    {
        ACDK_LOGE("wrong g_dumpRAW(%d), set to 0",g_dumpRAW);
        g_dumpRAW = 0;
    }

    //====== Parameter Setting ======

    // width and height
    if (width != 0 && height != 0)
    {
        mCapWidth  = width;
        mCapHeight = height;

        if((mCapWidth % 16) != 0)
        {
            mCapWidth = mCapWidth & (~0xF);
        }

        if((mCapHeight % 16) != 0)
        {
            mCapHeight = mCapHeight & (~0xF);
        }
    }
    else
    {
        if(mode == PREVIEW_MODE)
        {
            mCapWidth = mSensorResolution.SensorPreviewWidth;
            mCapHeight = mSensorResolution.SensorPreviewHeight;
        }
        else if(mode == VIDEO_MODE)
        {
            mCapWidth  = mSensorResolution.SensorVideoWidth;
            mCapHeight = mSensorResolution.SensorVideoHeight;
        }
        else
        {
            mCapWidth  = mSensorResolution.SensorFullWidth;
            mCapHeight = mSensorResolution.SensorFullHeight;
        }

        m_pAcdkUtilityObj->queryCapSize(mCapWidth,mCapHeight);
    } 

    ACDK_LOGD("mCapWidth(%u),mCapHeight(%u)",mCapWidth,mCapHeight);

    // sensor scenario
    if(mode == PREVIEW_MODE)
    {
        sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
    else if(mode == VIDEO_MODE)
    {
        sensorScenarioId = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
    }
    else
    {
        sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
    }

    ACDK_LOGD("mode(%u),sensorScenarioId(%u)",mode,sensorScenarioId);

    //====== AF ======
    err = m_pAcdkMhalObj->acdkMhalCaptureStart(NULL);   // do AF
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkMhalObj->acdkMhalCaptureStart(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }

    //====== Precapture Process ======

    // Do pre capture before stop preview, for 3A
    ACDK_LOGD("change to precapture state");

    err = m_pAcdkMhalObj->acdkMhalPreCapture();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkMhalObj->acdkMhalPreCapture fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("wait readyForCap");

    while(m_pAcdkMhalObj->acdkMhalReadyForCap() == MFALSE)
    {
        usleep(200);
    }

    //====== Stop Preview ======

    ACDK_LOGD("stop preview");

    err = stopPreview();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("stopPreview fail, err(0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }

    //====== Set to Capture State =====

    // AcdkMhal
    m_pAcdkMhalObj->acdkMhalSetState(ACDK_MHAL_CAPTURE);

    // AcdkMain
    acdkMainSetState(ACDK_MAIN_CAPTURE);

    //====== Check Orientation ======

    if(mOrientation == 90 || mOrientation == 270)
    {  
        mQVWidth  = mPrvHeight;
        mQVHeight = mPrvWidth;        
    }
    else
    {
        mQVWidth  = mPrvWidth;
        mQVHeight = mPrvHeight;
    }    
    
    //===== Set ISP Tuning Parameter and AE Capture Mode ======

    ACDK_LOGD("mOperaMode(%u)", mOperaMode);

#if CCT_TUNING_SUPPORT

    MINT32 u4AEEnable, u4AEEnableLen;

    // operation mode and sensor mode for ISP tuning
    if(mOperaMode == ACDK_OPT_META_MODE)
    {
        NSIspTuning::IspTuningMgr::getInstance().setOperMode(NSIspTuning::EOperMode_Meta);
    }
    else if(mOperaMode == ACDK_OPT_FACTORY_MODE)
    {
        NSIspTuning::IspTuningMgr::getInstance().setOperMode(NSIspTuning::EOperMode_Normal);
    }

    if(mode == PREVIEW_MODE)
    {
        NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Preview);        
    }
    else if(mode == VIDEO_MODE)
    {
        NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Video);       
    }
    else
    {
        NSIspTuning::IspTuningMgr::getInstance().setSensorMode(NSIspTuning::ESensorMode_Capture);        
    }

    // sensor mode for AE
    NS3A::AeMgr::getInstance().CCTOPAEGetEnableInfo(&u4AEEnable, (MUINT32 *)&u4AEEnableLen);

    ACDK_LOGD("AE Eanble(%d)", u4AEEnable);

    if(u4AEEnable)  // only effect when AE enable
    {
        // sensor mode
        if(mode == PREVIEW_MODE)
        {            
            err = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(0);
        }
        else if(mode == VIDEO_MODE)
        {            
            err = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(2);
        }
        else
        {            
            err = NS3A::AeMgr::getInstance().CCTOPAESetCaptureMode(1);
        }
        
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("CCTOPAESetCaptureMode fail(err=0x%x)",err);
            return ACDK_RETURN_API_FAIL;
        }
    }

#endif

    //====== Single Shot Process ======  

    mCapType = imgType;
    ACDK_LOGD("mCapType(0x%x)", mCapType);

    m_pSingleShot->init();

    if(mCapType & 0x3C) //RAW type
    {
        switch(mCapType)
        {
            case PURE_RAW8_TYPE : rawBit= 8;
                                  rawType = 0;   //pure raw
                break;
            case PURE_RAW10_TYPE : rawBit= 10;
                                   rawType = 0;   //pure raw
                break;
            case PROCESSED_RAW8_TYPE : rawBit= 8;
                                       rawType = 1;   //processed raw
                break;
            case PROCESSED_RAW10_TYPE : rawBit= 10;
                                        rawType = 1;   //processed raw
                break;
        }
        
        ACDK_LOGD("rawBit(%u),rawType(%u)",rawBit,rawType);
        
        m_pSingleShot->enableDataMsg(ECamShot_DATA_MSG_BAYER | ECamShot_DATA_MSG_POSTVIEW);
    }
    else if(mCapType == JPEG_TYPE)
    { 
        ACDK_LOGD("g_dumpRAW(%d)",g_dumpRAW);
        
        if(g_dumpRAW == 0)
        {
            m_pSingleShot->enableDataMsg(ECamShot_DATA_MSG_JPEG | ECamShot_DATA_MSG_POSTVIEW);
        }
        else
        {
            switch(g_dumpRAW)
            {
                case 4 : rawBit= 8;
                         rawType = 0;   //pure raw
                    break;
                case 8 : rawBit= 10;
                         rawType = 0;   //pure raw
                    break;
                case 16 : rawBit= 8;
                          rawType = 1;   //processed raw
                    break;                
                case 32 : rawBit= 10;
                          rawType = 1;   //processed raw
                    break;
            }
            
            m_pSingleShot->enableDataMsg(ECamShot_DATA_MSG_BAYER | ECamShot_DATA_MSG_JPEG | ECamShot_DATA_MSG_POSTVIEW);            
        }
    }

    // shot param
    ShotParam rShotParam((EImageFormat)capFormat,   // yuv format 
                          mCapWidth,                // picutre width 
                          mCapHeight,               // picture height
                          0,                        // picture rotation (mOrientation)
                          0,                        // picture flip => single shot not support
                          eImgFmt_YUY2,             // postview format 
                          mQVWidth,                 // postview width 
                          mQVHeight,                // postview height 
                          0,                        // postview rotation => no use. acdkMain should handle rotation by itself
                          0,                        // postview flip => single shot not support
                          100);                     //u4ZoomRatio

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

    JpegParam rJpegParam(100,       //Quality
                         mIsSOI);   //isSOI , True : create jpeg data include JFIF info 
                                    // False : bit stream data

    //ACDK_LOGD("rJpegParam.u4ThumbWidth   = %u", rJpegParam.u4ThumbWidth);
    //ACDK_LOGD("rJpegParam.u4ThumbHeight  = %u", rJpegParam.u4ThumbHeight);
    //ACDK_LOGD("rJpegParam.u4ThumbQuality = %u", rJpegParam.u4ThumbQuality);
    //ACDK_LOGD("rJpegParam.fgThumbIsSOI   = %u", rJpegParam.fgThumbIsSOI);
    ACDK_LOGD("rJpegParam.u4Quality = %u", rJpegParam.u4Quality);
    ACDK_LOGD("rJpegParam.fgIsSOI   = %u", rJpegParam.fgIsSOI);

    // sensor param    
    SensorParam rSensorParam(mSensorDev,        // device ID 
                             sensorScenarioId,  // scenaio 
                             rawBit,            // bit depth 
                             MFALSE,             // bypass delay 
                             MFALSE,            // bypass scenario 
                             rawType);          // RAW type : 0-pure raw, 1-processed raw
    
    ACDK_LOGD("rSensorParam.u4DeviceID      = %u", rSensorParam.u4DeviceID);
    ACDK_LOGD("rSensorParam.u4Scenario      = %u", rSensorParam.u4Scenario);
    ACDK_LOGD("rSensorParam.u4Bitdepth      = %u", rSensorParam.u4Bitdepth);
    ACDK_LOGD("rSensorParam.fgBypassDelay   = %u", rSensorParam.fgBypassDelay);
    ACDK_LOGD("rSensorParam.fgBypassScenaio = %u", rSensorParam.fgBypassScenaio);
    ACDK_LOGD("rSensorParam.u4RawType       = %u", rSensorParam.u4RawType);

    //config sensor test pattern 
    if(mTestPatternOut)
    {
        MINT32 u32Enable = 1;
        err = m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MINT32)&u32Enable,
                                           0,
                                           0);
    }

    m_pSingleShot->setCallbacks(NULL, camShotDataCB, this);

    m_pSingleShot->setShotParam(rShotParam);

    m_pSingleShot->setJpegParam(rJpegParam);

    MUINT32 captureLoop;
    if(captureCnt == 0)
    {
        captureLoop = 1;
    }
    else
    {
        captureLoop = captureCnt;
    }

    ACDK_LOGD("captureCnt(%u),captureLoop(%u)",captureCnt,captureLoop);
    
    for(MUINT32 i = 0; i < captureLoop; ++i)
    {
        //====== Factory-Camera Auto-Testing Get Current Shutter Time ======

        mGetShutTime = m_pAcdkMhalObj->acdkMhalGetShutTime();
        
        ACDK_LOGD("mGetShutTime(%u)",mGetShutTime);
        
        //====== Factory-Camera Auto-Testing Set Shutter Time Forcedly ======

        if(mOperaMode == ACDK_OPT_FACTORY_MODE && mSetShutTime != 0)
        {
            ACDK_LOGD("set shutter time forcedly(%d)",mSetShutTime);

            m_pAcdkMhalObj->acdkMhalSetShutTime(mSetShutTime);

            mSetShutTime = 0;
        }
        
        m_pSingleShot->startOne(rSensorParam);  

        //====== Show QV Image ======

        quickViewImg(qvFormat);

        //====== Save Image ======

        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.acdkdump.enable", value, "0");
        MINT32 dumpEnable = atoi(value);

        if(isSaveImg == 1 || dumpEnable == 4)
        {
            if(mCapType & 0x3C || g_dumpRAW != 0)
            {
                saveRAWImg(mode);
            }

            if(mCapType == JPEG_TYPE)
            {
                saveJPEGImg();
            }
        }
        
        //====== Capture Callback ======

        if(capCb != NULL)
        {
            ImageBufInfo acdkCapInfo;
            acdkCapInfo.eImgType = (eACDK_CAP_FORMAT)mCapType;
            // PURE_RAW8_TYPE =0x04, PURE_RAW10_TYPE = 0x08 
            // PURE_RAW10_TYPE = 0x10, PROCESSED_RAW10_TYPE = 0x20
            if(mCapType & 0x3C)
            {   
                if(mSensorType == SENSOR_TYPE_YUV) //YUV
                {
                    acdkCapInfo.eImgType             = YUV_TYPE;
                    acdkCapInfo.imgBufInfo.bufAddr   = (MUINT8 *)mRawIMemInfo.virtAddr;                    
                    acdkCapInfo.imgBufInfo.imgSize   = mRawIMemInfo.size;

                    if(mode == PREVIEW_MODE)
                    {
                        acdkCapInfo.imgBufInfo.imgWidth  = mSensorResolution.SensorPreviewWidth;
                        acdkCapInfo.imgBufInfo.imgHeight = mSensorResolution.SensorPreviewHeight;
                    }
                    else if(mode == VIDEO_MODE)
                    {
                        acdkCapInfo.imgBufInfo.imgWidth  = mSensorResolution.SensorVideoWidth;
                        acdkCapInfo.imgBufInfo.imgHeight = mSensorResolution.SensorVideoHeight;
                    }
                    else
                    {
                        acdkCapInfo.imgBufInfo.imgWidth  = mSensorResolution.SensorFullWidth;
                        acdkCapInfo.imgBufInfo.imgHeight = mSensorResolution.SensorFullHeight;
                    }

                    switch(mSensorFormatInfo.u1Order)
                    {
                        case SENSOR_OUTPUT_FORMAT_UYVY :
                        case SENSOR_OUTPUT_FORMAT_CbYCrY :
                                acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_UYVY;
                            break;
                        case SENSOR_OUTPUT_FORMAT_VYUY :
                        case SENSOR_OUTPUT_FORMAT_CrYCbY :
                                acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_VYUY;
                            break;
                        case SENSOR_OUTPUT_FORMAT_YUYV :
                        case SENSOR_OUTPUT_FORMAT_YCbYCr :
                                acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_YUY2;
                            break;
                        case SENSOR_OUTPUT_FORMAT_YVYU :
                        case SENSOR_OUTPUT_FORMAT_YCrYCb :
                                acdkCapInfo.imgBufInfo.imgFmt= YUVFmt_YVYU;
                            break;
                        default : acdkCapInfo.imgBufInfo.imgFmt = YUVFmt_Unknown;
                                  ACDK_LOGE("unknown YUV type(0x%x)",mSensorFormatInfo.u1Order);                                  
                    }

                    ACDK_LOGD("YUVImg.bufAddr     = 0x%x", (MUINT32)acdkCapInfo.imgBufInfo.bufAddr);                    
                    ACDK_LOGD("YUVImg.imgWidth    = %u", acdkCapInfo.imgBufInfo.imgWidth);
                    ACDK_LOGD("YUVImg.imgHeight   = %u", acdkCapInfo.imgBufInfo.imgHeight);
                    ACDK_LOGD("YUVImg.imgSize     = %u", acdkCapInfo.imgBufInfo.imgSize);
                    ACDK_LOGD("YUVImg.imgFmt      = %d", acdkCapInfo.imgBufInfo.imgFmt);
                }
                else
                {
                    if(mode == PREVIEW_MODE)
                    {
                        acdkCapInfo.RAWImgBufInfo.imgWidth  = mSensorResolution.SensorPreviewWidth;
                        acdkCapInfo.RAWImgBufInfo.imgHeight = mSensorResolution.SensorPreviewHeight;
                    }
                    else if(mode == VIDEO_MODE)
                    {
                        acdkCapInfo.RAWImgBufInfo.imgWidth  = mSensorResolution.SensorVideoWidth;
                        acdkCapInfo.RAWImgBufInfo.imgHeight = mSensorResolution.SensorVideoHeight;
                    }
                    else
                    {
                        acdkCapInfo.RAWImgBufInfo.imgWidth  = mSensorResolution.SensorFullWidth;
                        acdkCapInfo.RAWImgBufInfo.imgHeight = mSensorResolution.SensorFullHeight;
                    }
                    
                    if(mUnPack == MTRUE) //for specail mode use : unpack raw image to normal package 
                    {
                        MUINT32 imgStride = mSensorResolution.SensorFullWidth, unPackSize = 0;
                        MUINT32 tempFmt, tempRAWPixelByte;
                        IMEM_BUF_INFO unPackIMem;

                        //calculate stride                        
                        switch(mSensorFormatInfo.u4BitDepth)
                        {
                            case 8 : tempFmt  = eImgFmt_BAYER8;
                                break;
                            case 10 : tempFmt = eImgFmt_BAYER10;
                                break;
                            case 12 : tempFmt = eImgFmt_BAYER12;
                                break;
                            default : tempFmt = eImgFmt_UNKNOWN;
                                      ACDK_LOGE("unknown raw image bit depth(%u)",mSensorFormatInfo.u4BitDepth);
                                      break;
                        }

                        // calculate real stride and get byte per pixel. for RAW sensor only
                        if(ACDK_RETURN_NO_ERROR != m_pAcdkUtilityObj->queryRAWImgFormatInfo(tempFmt,acdkCapInfo.RAWImgBufInfo.imgWidth,imgStride,tempRAWPixelByte))
                        {
                            ACDK_LOGE("queryRAWImgFormatInfo fail");                            
                        }                        
                        
                        //prepare memory
                        unPackSize = acdkCapInfo.RAWImgBufInfo.imgWidth * acdkCapInfo.RAWImgBufInfo.imgHeight * 2;
                        ACDK_LOGD("unPackSize(%u),imgStride(%u)",unPackSize,imgStride);
                        
                        createMemBuf(unPackSize, 1, &unPackIMem);

                        if(unPackIMem.virtAddr == 0)
                        {
                            memset((MVOID *)unPackIMem.virtAddr, 0, unPackIMem.size);
                            ACDK_LOGE("unPackIMem is NULL");
                        }
                        else
                        { 
                            m_pAcdkUtilityObj->rawImgUnpack(mRawIMemInfo,
                                                            unPackIMem,
                                                            acdkCapInfo.RAWImgBufInfo.imgWidth,
                                                            acdkCapInfo.RAWImgBufInfo.imgHeight,
                                                            mSensorFormatInfo.u4BitDepth,
                                                            imgStride);

                            acdkCapInfo.RAWImgBufInfo.bufAddr   = (MUINT8 *)unPackIMem.virtAddr;                        
                            acdkCapInfo.RAWImgBufInfo.imgSize   = unPackIMem.size;
                            acdkCapInfo.RAWImgBufInfo.isPacked  = MFALSE;
                        }

                        
                    }
                    else
                    {
                        acdkCapInfo.RAWImgBufInfo.bufAddr   = (MUINT8 *)mRawIMemInfo.virtAddr;                        
                        acdkCapInfo.RAWImgBufInfo.imgSize   = mRawIMemInfo.size;
                        acdkCapInfo.RAWImgBufInfo.isPacked  = MTRUE;                        
                    }

                    acdkCapInfo.RAWImgBufInfo.bitDepth  = mSensorFormatInfo.u4BitDepth;
                    acdkCapInfo.RAWImgBufInfo.eColorOrder = (eRAW_ColorOrder)mSensorFormatInfo.u1Order;

                    ACDK_LOGD("RAWImg.bufAddr     = 0x%x", (MUINT32)acdkCapInfo.RAWImgBufInfo.bufAddr);
                    ACDK_LOGD("RAWImg.bitDepth    = %u", acdkCapInfo.RAWImgBufInfo.bitDepth);
                    ACDK_LOGD("RAWImg.imgWidth    = %u", acdkCapInfo.RAWImgBufInfo.imgWidth);
                    ACDK_LOGD("RAWImg.imgHeight   = %u", acdkCapInfo.RAWImgBufInfo.imgHeight);
                    ACDK_LOGD("RAWImg.imgSize     = %u", acdkCapInfo.RAWImgBufInfo.imgSize);
                    ACDK_LOGD("RAWImg.isPacked    = %u", acdkCapInfo.RAWImgBufInfo.isPacked);
                    ACDK_LOGD("RAWImg.eColorOrder = %u", acdkCapInfo.RAWImgBufInfo.eColorOrder);
                }               
            }
            else if(mCapType == JPEG_TYPE)
            {
                acdkCapInfo.imgBufInfo.bufAddr   = (MUINT8 *)mJpgIMemInfo.virtAddr;
                acdkCapInfo.imgBufInfo.imgWidth  = mCapWidth;
                acdkCapInfo.imgBufInfo.imgHeight = mCapHeight;
                acdkCapInfo.imgBufInfo.imgSize   = mJpgIMemInfo.size;

                ACDK_LOGD("imgBufInfo.bufAddr   = 0x%x", (MUINT32)acdkCapInfo.imgBufInfo.bufAddr);
                ACDK_LOGD("imgBufInfo.imgWidth  = %u", acdkCapInfo.imgBufInfo.imgWidth);
                ACDK_LOGD("imgBufInfo.imgHeight = %u", acdkCapInfo.imgBufInfo.imgHeight);
                ACDK_LOGD("imgBufInfo.imgSize   = %u", acdkCapInfo.imgBufInfo.imgSize);
            }

            capCb(&acdkCapInfo);
        }
    }
    
    //====== Uninit and Release ======

    // destory single shot object
    if(m_pSingleShot != NULL)
    {
        m_pSingleShot->uninit();
        m_pSingleShot->destroyInstance();
    }

    // free memory
    if(mRawIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mRawIMemInfo);
        mRawIMemInfo.size = mRawIMemInfo.virtAddr = mRawIMemInfo.phyAddr = 0;
        mRawIMemInfo.memID = -5;
    }

    if(mJpgIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mJpgIMemInfo);
        
        mJpgIMemInfo.size = mJpgIMemInfo.virtAddr = mJpgIMemInfo.phyAddr = 0;
        mJpgIMemInfo.memID = -5;        
    }

    if(mQvIMemInfo.virtAddr != 0)
    {
        destroyMemBuf(1, &mQvIMemInfo);
      
        mQvIMemInfo.size = mQvIMemInfo.virtAddr = mQvIMemInfo.phyAddr = 0;
        mQvIMemInfo.memID = -5;
    }

    //====== Stop Capture ======

    err = m_pAcdkMhalObj->acdkMhalCaptureStop();
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("m_pAcdkMhalObj->acdkMhalCaptureStop(err=0x%x)",err);
        return ACDK_RETURN_API_FAIL;
    }
    
    //====== Set to IDLE State ======

    // AcdkMhak
    m_pAcdkMhalObj->acdkMhalSetState(ACDK_MHAL_IDLE);

    // AcdkMain
    acdkMainSetState(ACDK_MAIN_IDLE);
    
    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::getFrameCnt(MUINT32 &frameCnt)
{
    frameCnt = mFrameCnt;
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::quickViewImg(MUINT32 qvFormat)
{ 
    //====== Check Input Argument ======

    if(mQvIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mQvIMemInfo is empty"); 
        return ACDK_RETURN_NULL_OBJ; 
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;    
    MUINT8 *pQVBufIn = (MUINT8 *)mQvIMemInfo.virtAddr;

    //====== Dump QV Image (Debug) ======    
    
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    if(dumpEnable == 5)    
    {
        MINT32 i4WriteCnt = 0;
        char szFileName[256];
        sprintf(szFileName, "%s/acdkQV1.yuv" , MEDIA_PATH);

        FILE *pFp = fopen(szFileName, "wb");
 
        if(NULL == pFp)
        {
            ACDK_LOGE("Can't open file to save image");            
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pQVBufIn, 1, mQvIMemInfo.size, pFp);
        
        fflush(pFp);
            
        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail"); 
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mQVWidth, mQVHeight); 

        fclose(pFp);
    }  

    //====== Process QV Image ======

    IMEM_BUF_INFO procQvMemInfo;
    createMemBuf(mQvIMemInfo.size, 1, &procQvMemInfo);

    ACDK_LOGD("procQvMemInfo : vir(0x%x),phy(0x%x)",procQvMemInfo.virtAddr,procQvMemInfo.phyAddr);
   
    m_pAcdkUtilityObj->imageProcess(qvFormat,
                                    mQVWidth,
                                    mQVHeight,
                                    mOrientation,
                                    mSensorHFlip,
                                    mQvIMemInfo,
                                    procQvMemInfo);
    
    //====== Set Surface Parameter ======

    MUINT8 *pQVBufOut = (MUINT8 *)procQvMemInfo.virtAddr;

    if(dumpEnable == 5)
    {
        MINT32 i4WriteCnt = 0;
        char szFileName[256];
        sprintf(szFileName, "%s/acdkQV2.rgb" , MEDIA_PATH);

        FILE *pFp = fopen(szFileName, "wb");
 
        if(NULL == pFp)
        {
            ACDK_LOGE("Can't open file to save image");           
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pQVBufOut, 1, procQvMemInfo.size, pFp);
        
        fflush(pFp);
            
        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail"); 
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mQVWidth, mQVHeight); 

        fclose(pFp);
    }  
    
    ACDK_LOGD("QV VA(0x%x)",(MUINT32)pQVBufOut);
    ACDK_LOGD("surfaceIndex[%u],VA(0x%x),PA(0x%x)",mSurfaceIndex,mSurfaceIMemInfo[mSurfaceIndex].virtAddr,mSurfaceIMemInfo[mSurfaceIndex].phyAddr);
   
    // show frame by overlay
    err = m_pAcdkSurfaceViewObj->setOverlayInfo(0, 
                                                mPrvStartX, 
                                                mPrvStartY,
                                                mQVWidth, 
                                                mQVHeight,
                                                mSurfaceIMemInfo[mSurfaceIndex].phyAddr, 
                                                mSurfaceIMemInfo[mSurfaceIndex].virtAddr, 
                                                mOrientation);
                    
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("setOverlayInfo fail err(0x%x)",err); 
        err = ACDK_RETURN_API_FAIL;
        goto quickViewImgExit;
    }

    ACDK_LOGD("qv size(%u)", procQvMemInfo.size); 
    memcpy((MVOID *)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, pQVBufOut, procQvMemInfo.size);
    m_pIMemDrv->cacheFlushAll();
    
    err = m_pAcdkSurfaceViewObj->refresh(); 
    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("refresh fail err(0x%x)", err); 
        err = ACDK_RETURN_API_FAIL; 
    }

    //update surface index
    mSurfaceIndex = (mSurfaceIndex + 1) % SURFACE_NUM;
    
quickViewImgExit:

    if(procQvMemInfo.virtAddr != 0 )
    {
        destroyMemBuf(1, &procQvMemInfo);        
    }
    
    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}
    
/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::setSrcDev(MINT32 srcDev)
{
    ACDK_LOGD("srcDev(%d)",srcDev);

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Switch sensor ======

    switch(srcDev)
    {
        case SENSOR_DEV_MAIN : //main
            mSensorDev = SENSOR_DEV_MAIN;
            mSensorVFlip = 0;
            mSensorHFlip = 0;
            break;
        case SENSOR_DEV_SUB : //sub
            mSensorDev = SENSOR_DEV_SUB;
            mSensorVFlip = 0;
            mSensorHFlip = 1;
            break;
        case SENSOR_DEV_ATV : //atv
            mSensorDev = SENSOR_DEV_ATV;
            mSensorVFlip = 0;
            mSensorHFlip = 0;
            break;
        case SENSOR_DEV_MAIN_2 : //main2
            mSensorDev = SENSOR_DEV_MAIN_2;
            mSensorVFlip = 0;
            mSensorHFlip = 0;
            break;
        default:
            ACDK_LOGE("wrong mSensorDev = %d", srcDev);
            err = ACDK_RETURN_INVALID_SENSOR;
    }

    //====== Get Sensro Info ======

    if(mSensorInit == MTRUE)
    {
        err = getSensorInfo();
        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("getSensorInfo error(0x%x)",err);
            err = ACDK_RETURN_API_FAIL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::setShutterTime(MUINT32 a_Time)
{
    ACDK_LOGD("time(%u)",a_Time);

    mSetShutTime = a_Time;
    
    return ACDK_RETURN_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/

MBOOL AcdkMain::makeExifHeader(MUINT32 const u4CamMode, 
                                    MUINT8* const puThumbBuf, 
                                    MUINT32 const u4ThumbSize, 
                                    MUINT8* puExifBuf, 
                                    MUINT32 &u4FinalExifSize, 
                                    MUINT32 u4ImgIndex, 
                                    MUINT32 u4GroupId)

{

   ACDK_LOGD("+ (u4CamMode, puThumbBuf, u4ThumbSize, puExifBuf) = (%d, %p, %d, %p)", 
                           u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf); 

   if (u4ThumbSize > 63 * 1024) 
   {
       ACDK_LOGD("The thumbnail size is large than 63K, the exif header will be broken"); 
   }
   bool ret = true;
   uint32_t u4App1HeaderSize = 0; 
   uint32_t u4AppnHeaderSize = 0; 

   uint32_t exifHeaderSize = 0;
   CamExif rCamExif;
   CamExifParam rExifParam;
   CamDbgParam rDbgParam;
   MUINT32 capFormat = eImgFmt_YUY2;

   // shot param
   ShotParam rShotParam((EImageFormat)capFormat,   // yuv format 
                         mCapWidth,                // picutre width 
                         mCapHeight,               // picture height
                         0,                        // picture rotation (mOrientation)
                         0,                        // picture flip => single shot not support
                         eImgFmt_YUY2,             // postview format 
                         mQVWidth,                 // postview width 
                         mQVHeight,                // postview height 
                         0,                        // postview rotation => no use. acdkMain should handle rotation by itself
                         0,                        // postview flip => single shot not support
                         100);                     // u4ZoomRatio
   
   // the bitstream already rotated. rotation should be 0 
   rExifParam.u4Orientation = 0; 
   rExifParam.u4ZoomRatio = 100; 
   //
   //camera_info rCameraInfo = MtkCamUtils::DevMetaInfo::queryCameraInfo(getOpenId()); 
   rExifParam.u4Facing = mSensorDev;//Main/Sub Camera
   //
   rExifParam.u4ImgIndex = u4ImgIndex;
   rExifParam.u4GroupId = u4GroupId;
   //
   rExifParam.u4FocusH = 0;
   rExifParam.u4FocusL = 0;
   // 
   //! CamDbgParam (for camMode, shotMode)
   rDbgParam.u4CamMode = u4CamMode; 
   rDbgParam.u4ShotMode = 0;//getShotMode();    
   //
   rCamExif.init(rExifParam,  rDbgParam);

   Hal3ABase* p3AHal = Hal3ABase::createInstance((halSensorDev_e)mSensorDev); 
   SensorHal* pSensorHal = SensorHal::createInstance(); 

   p3AHal->set3AEXIFInfo(&rCamExif); 
   // the bitstream already rotated. it need to swap the width/height
   if (90 == rShotParam.u4PictureRotation || 270 == rShotParam.u4PictureRotation) 
   {
       rCamExif.makeExifApp1(rShotParam.u4PictureHeight,  rShotParam.u4PictureWidth, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
   }
   else 
   {
       rCamExif.makeExifApp1(rShotParam.u4PictureWidth, rShotParam.u4PictureHeight, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
   }
   // copy thumbnail image after APP1 
   MUINT8 *pdest = puExifBuf + u4App1HeaderSize; 
   ::memcpy(pdest, puThumbBuf, u4ThumbSize) ; 
   // 
   // 3A Debug Info 
   p3AHal->setDebugInfo(&rCamExif); 
   //
   // Sensor Debug Info 
   pSensorHal->setDebugInfo(&rCamExif); 
   pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize; 
   //
   rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
   rCamExif.uninit();

   u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize; 
   p3AHal->destroyInstance(); 
   pSensorHal->destroyInstance(); 

   ACDK_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)", 
                         u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize); 
   return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL AcdkMain::camShotDataCB(MVOID* user, CamShotDataInfo const msg)
{
    ACDK_LOGD("msg.msgType(%u)",msg.msgType);

    AcdkMain * const _This = reinterpret_cast<AcdkMain *>(user);
    acdkCallbackParam_t acdkCbParam;

    if(NULL != _This)
    {
        if(ECamShot_DATA_MSG_BAYER == msg.msgType)
        {
            acdkCbParam.type  = ACDK_CB_RAW;
            acdkCbParam.addr1 = (MUINT32)msg.puData;    // pointer to RAW data buffer
            acdkCbParam.addr2 = 0;
            acdkCbParam.imgSize  = msg.u4Size;          // pointer to RAW data buffer
            acdkCbParam.thubSize = 0;
        }
        else if(ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            acdkCbParam.type  = ACDK_CB_QV;
            acdkCbParam.addr1 = (MUINT32)msg.puData;    // pointer to RAW data buffer
            acdkCbParam.addr2 = 0;
            acdkCbParam.imgSize  = msg.u4Size;          // pointer to RAW data buffer
            acdkCbParam.thubSize = 0;
        }
        else if(ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            acdkCbParam.type  = ACDK_CB_JPEG;
            acdkCbParam.addr1 = (MUINT32)msg.puData;    // pointer to JPEG data buffer
            acdkCbParam.addr2 = msg.ext1;               // pointer to thumbnail data buffer
            acdkCbParam.imgSize  = msg.u4Size;          // size of JPEG data buffer
            acdkCbParam.thubSize = msg.ext2;            // size of thumbnail buffer
        }

        _This->dispatchCallback(&acdkCbParam);
    }
    else
    {
        ACDK_LOGE("NULL AcdkMain pointer");
        return MFALSE;
    }

    ACDK_LOGD("-");
    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
void AcdkMain::cameraCallback(void* param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    acdkCBInfo * const pCBInfo = reinterpret_cast<acdkCBInfo*>(param);
    if(!pCBInfo)
    {
        ACDK_LOGE("NULL pCBInfo");
        return;
    }

    AcdkMain * const _This = reinterpret_cast<AcdkMain*>(pCBInfo->mCookie);
    if(!_This )
    {
        ACDK_LOGE("NULL AcdkMain");
        return;
    }

    acdkCallbackParam_t acdkCbParam;
    acdkCbParam.type  = pCBInfo->mType;
    acdkCbParam.addr1 = pCBInfo->mAddr1;
    acdkCbParam.addr2 = pCBInfo->mAddr2;

    _This->dispatchCallback(&acdkCbParam);

     ACDK_LOGD_DYN(g_acdkMainDebug,"-");
}


/*******************************************************************************
*
********************************************************************************/
void AcdkMain::dispatchCallback(void *param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkCallbackParam_t *pcbParam = (acdkCallbackParam_t *)param;

    switch(pcbParam->type)
    {
        case ACDK_CB_PREVIEW :  //0x1
            ACDK_LOGD_DYN(g_acdkMainDebug,"Preview Callback");
            err = handlePreviewCB(pcbParam);
            break;
        case ACDK_CB_RAW :  //0x3
            ACDK_LOGD_DYN(g_acdkMainDebug,"Raw Callback");
            err = handleRAWCB(pcbParam);
            break;
        case ACDK_CB_JPEG :  //0x4
            ACDK_LOGD_DYN(g_acdkMainDebug,"JPEG Callback");
            err =  handleJPEGCB(pcbParam);
            break;
        case ACDK_CB_QV :    //0x5
            ACDK_LOGD_DYN(g_acdkMainDebug,"QV Callback");
            err = handleQVCB(pcbParam);
            break;        
        default :
            err =  ACDK_RETURN_INVALID_PARA;
            break;
    }

    if(err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("CB(%d),err(0x%x)",(MINT32)pcbParam->type, err);
    }

    ACDK_LOGD_DYN(g_acdkMainDebug,"-");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::handlePreviewCB(MVOID *param)
{
    ACDK_LOGD_DYN(g_acdkMainDebug,"+");

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *) param;

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pVirBufin  = NULL;
    MUINT32 displayAddr = pCBParam->addr1;

    mFrameCnt++;
    pVirBufin = (MUINT8 *)displayAddr;

    ACDK_LOGD_DYN(g_acdkMainDebug,"mPrvStartX(%u),mPrvStartY(%u)",mPrvStartX,mPrvStartY);
    ACDK_LOGD_DYN(g_acdkMainDebug,"mPrvWidth(%u),mPrvHeight(%u)",mPrvWidth,mPrvHeight);
    ACDK_LOGD_DYN(g_acdkMainDebug,"index(%u),VA(0x%x),PA(0x%x)",mSurfaceIndex,mSurfaceIMemInfo[mSurfaceIndex].virtAddr,mSurfaceIMemInfo[mSurfaceIndex].phyAddr);
    ACDK_LOGD_DYN(g_acdkMainDebug,"mOrientation(%u)",mOrientation);

    // show frame by overlay
    if(mOrientation == 90 || mOrientation == 270)
    {
        err = m_pAcdkSurfaceViewObj->setOverlayInfo(0, 
                                                    mPrvStartX, 
                                                    mPrvStartY,
                                                    mPrvHeight, 
                                                    mPrvWidth,
                                                    mSurfaceIMemInfo[mSurfaceIndex].phyAddr, 
                                                    mSurfaceIMemInfo[mSurfaceIndex].virtAddr, 
                                                    mOrientation);
    }
    else
    {
        err = m_pAcdkSurfaceViewObj->setOverlayInfo(0, 
                                                    mPrvStartX, 
                                                    mPrvStartY,
                                                    mPrvWidth, 
                                                    mPrvHeight,
                                                    mSurfaceIMemInfo[mSurfaceIndex].phyAddr, 
                                                    mSurfaceIMemInfo[mSurfaceIndex].virtAddr, 
                                                    mOrientation);
    }
    
    if(err != ACDK_RETURN_NO_ERROR) 
    {
        ACDK_LOGE("setOverlayInfo fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

#if 0

    ACDK_LOGD("m_pAcdkSurfaceViewObj->setOverlayBuf");
    ACDK_LOGD("pPhybufin  = 0x%x",(MUINT32)pVirBufin);
    ACDK_LOGD("srcFormat  = %u",(MUINT32)mAcdkMhalPrvParam.frmParam.frmFormat);
    ACDK_LOGD("srcWidth   = %u",mAcdkMhalPrvParam.frmParam.w);
    ACDK_LOGD("srcHeight  = %u",mAcdkMhalPrvParam.frmParam.h);
    ACDK_LOGD("srcOrientation = %u",mOrientation);
    ACDK_LOGD("mSensorHFlip = %u",mSensorHFlip);
    ACDK_LOGD("mSensorVFlip = %u",mSensorVFlip);

    //! FIXME, the mOrientation should depend on screen and sensor

    err = m_pAcdkSurfaceViewObj->setOverlayBuf(
                               0,
                               pPhybufin,
                               11,   //MHAL_FORMAT_YUY2
                               mAcdkMhalPrvParam.frmParam.w,
                               mAcdkMhalPrvParam.frmParam.h,
                               mOrientation,
                               mSensorHFlip,
                               mSensorVFlip
                               );

    if (err != 0)
    {
        ACDK_LOGE("setOverlayBuf fail err = 0x%x", err);
        return ACDK_RETURN_API_FAIL;
    }
#endif

    //====== Copy Image to Surface Memory =======

    char value[PROPERTY_VALUE_MAX];
    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    if(dumpEnable == 2 || dumpEnable == 3)
    {
        ACDK_LOGD("dump");

        char szFileName[256];
        MINT32 i4WriteCnt = 0;
        MINT32 cnt = 0;

        ACDK_LOGD("prv VA(0x%x)",displayAddr);

        sprintf(szFileName, "%s/acdkPrv2.rgb",MEDIA_PATH);

        //====== Write RAW Data ======

        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp )
        {
            ACDK_LOGE("Can't open file to save image");
            fclose(pFp);
            return ACDK_RETURN_NULL_OBJ;
        }

        i4WriteCnt = fwrite(pVirBufin, 1, mSurfaceIMemInfo[mSurfaceIndex].size, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            ACDK_LOGE("fync fail");
            fclose(pFp);
            return ACDK_RETURN_API_FAIL;
        }

        ACDK_LOGD("Save image file name:%s, w(%d), h(%d)", szFileName, mPrvWidth, mPrvHeight);

        fclose(pFp);
    }

    memcpy((MVOID *)mSurfaceIMemInfo[mSurfaceIndex].virtAddr, pVirBufin, mSurfaceIMemInfo[mSurfaceIndex].size);
    m_pIMemDrv->cacheFlushAll();

    //====== Refresh Screen =======

    err = m_pAcdkSurfaceViewObj->refresh();
    if (err != ACDK_RETURN_NO_ERROR)
    {
        ACDK_LOGE("refresh fail err(0x%x)", err);
        return ACDK_RETURN_API_FAIL;
    }

    //update surface index
    mSurfaceIndex = (mSurfaceIndex + 1) % SURFACE_NUM;
    
    ACDK_LOGD_DYN(g_acdkMainDebug,"-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/

MINT32 AcdkMain::handleRAWCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if (param == NULL)
    {
        ACDK_LOGE("param is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *)param;
    MUINT8 *pCapBufIn = (MUINT8 *)pCBParam->addr1;

    ACDK_LOGD("RAW src VA(0x%x)",(MUINT32)pCapBufIn);

    //====== Copy RAW Data ======
    if(mRawIMemInfo.virtAddr == 0)
    {
        createMemBuf(pCBParam->imgSize, 1, &mRawIMemInfo);
    }

    if(mRawIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mRawIMemInfo is NULL"); 
        return ACDK_RETURN_NULL_OBJ;
    }
    else
    {   
        memcpy((MVOID *)mRawIMemInfo.virtAddr, pCapBufIn, mRawIMemInfo.size);
        m_pIMemDrv->cacheFlushAll();
        ACDK_LOGD("mRawIMemInfo : vir(0x%x),phy(0x%x),memID(%d)",mRawIMemInfo.virtAddr,mRawIMemInfo.phyAddr,mRawIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;        
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::handleJPEGCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(param == NULL)
    {
        ACDK_LOGE("param is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *) param;
    MUINT8 *pJpgBufIn = (MUINT8 *)pCBParam->addr1;
    MUINT32 imgSize;
    ACDK_LOGD("JPEG src VA(0x%x)",(MUINT32)pJpgBufIn);
    
    if(mIsSOI) // JFIF
        imgSize = pCBParam->imgSize;
    else //EXIF
        imgSize = pCBParam->imgSize + 64*1024;
 
    //====== Copy RAW Data ======

    if(mJpgIMemInfo.virtAddr == 0)
    {
        createMemBuf(imgSize, 1, &mJpgIMemInfo);
    }

    if(mJpgIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mJpgIMemInfo is NULL"); 
        return ACDK_RETURN_NULL_OBJ;
    }
    else
    {
        if(mIsSOI)
        {       
            memcpy((MVOID *)mJpgIMemInfo.virtAddr, pJpgBufIn, mJpgIMemInfo.size);
            m_pIMemDrv->cacheFlushAll();
        }
        else
        {
            MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024]; 
            MUINT32 u4ExifHeaderSize = 0; 

            makeExifHeader(eAppMode_FactoryMode, NULL, 0, puExifHeaderBuf, u4ExifHeaderSize,0,0); 
            ACDK_LOGD("(exifHeaderBuf, size) = (%p, %d)",puExifHeaderBuf, u4ExifHeaderSize); 
            memcpy((MVOID *)mJpgIMemInfo.virtAddr, puExifHeaderBuf, u4ExifHeaderSize);
            memcpy((MVOID *)((MUINT8 *)mJpgIMemInfo.virtAddr + u4ExifHeaderSize), pJpgBufIn, pCBParam->imgSize);
            m_pIMemDrv->cacheFlushAll();
            mJpgIMemInfo.size = u4ExifHeaderSize + pCBParam->imgSize;

            delete [] puExifHeaderBuf;  
        }
        ACDK_LOGD("mJpgIMemInfo : vir(0x%x),phy(0x%x),memID(%d)",mJpgIMemInfo.virtAddr,mJpgIMemInfo.phyAddr,mJpgIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::handleQVCB(MVOID *param)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if (param == NULL)
    {
        ACDK_LOGE("param is NULL"); 
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======
   
    acdkCallbackParam_t *pCBParam = (acdkCallbackParam_t *)param;
    MUINT8 *pQVBufIn  = (MUINT8 *)pCBParam->addr1;

    ACDK_LOGD("QV src VA(0x%x)",(MUINT32)pQVBufIn);

    //====== Copy RAW Data ======

    if(mQvIMemInfo.virtAddr == 0)
    {
        createMemBuf(pCBParam->imgSize, 1, &mQvIMemInfo);
    }

    if(mQvIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mQvIMemInfo is NULL"); 
        return ACDK_RETURN_NULL_OBJ; 
    }
    else
    {
        memcpy((MVOID *)mQvIMemInfo.virtAddr, pQVBufIn, mQvIMemInfo.size);
        m_pIMemDrv->cacheFlushAll();
        ACDK_LOGD(" mQvIMemInfo : vir(0x%x),phy(0x%x),memID(%d)",mQvIMemInfo.virtAddr,mQvIMemInfo.phyAddr,mQvIMemInfo.memID);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;       
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::saveJPEGImg()
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(mJpgIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mJpgIMemInfo is empty");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pImgBufIn = (MUINT8 *)mJpgIMemInfo.virtAddr;
    MINT32 i4WriteCnt = 0;
    char szFileName[256];

    //====== Write RAW Data ======

    ACDK_LOGD("JPEG src VA(0x%x)",(MUINT32)pImgBufIn);

    sprintf(szFileName, "%s/acdkCap.jpg",MEDIA_PATH);

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");       
        err = ACDK_RETURN_NULL_OBJ;
        goto saveJPEGImgExit;
    }

    i4WriteCnt = fwrite(pImgBufIn, 1, mJpgIMemInfo.size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        ACDK_LOGE("fync fail");
        fclose(pFp);
        err = ACDK_RETURN_API_FAIL;
        goto saveJPEGImgExit;
    }

    ACDK_LOGD("Save image file name:%s,w(%u),h(%u)", szFileName, mCapWidth, mCapHeight);

    fclose(pFp);

saveJPEGImgExit:

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkMain::saveRAWImg(MINT32 mode)
{
    ACDK_LOGD("+");

    //====== Check Input Argument ======

    if(mRawIMemInfo.virtAddr == 0)
    {
        ACDK_LOGE("mRawIMemInfo is empty");
        return ACDK_RETURN_NULL_OBJ;
    }

    //====== Local Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT8 *pImgBufIn = (MUINT8 *)mRawIMemInfo.virtAddr;
    MINT32 i4WriteCnt = 0;
    char szFileName[256];

    //====== Width & Height Setting ======

    MUINT32 rawWidth = 0, rawHeight = 0;
    
    if(mode == PREVIEW_MODE)
    {
        rawWidth  = mSensorResolution.SensorPreviewWidth;
        rawHeight = mSensorResolution.SensorPreviewHeight;
    }
    else if(mode == VIDEO_MODE)
    {
        rawWidth  = mSensorResolution.SensorVideoWidth;
        rawHeight = mSensorResolution.SensorVideoHeight;
    }
    else
    {
        rawWidth  = mSensorResolution.SensorFullWidth;
        rawHeight = mSensorResolution.SensorFullHeight;
    }

    //====== Write RAW Data ======
    ACDK_LOGD("RAW src VA(0x%x)",(MUINT32)pImgBufIn);
    if(mSensorType == SENSOR_TYPE_YUV) //YUV
    {
        sprintf(szFileName, "%s/YUVImg_YUY2.yuv" , MEDIA_PATH);
    }
    else
    {
        if(mCapType == PURE_RAW8_TYPE || g_dumpRAW == PURE_RAW8_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapPureRaw8.raw" , MEDIA_PATH);
        }
        else if(mCapType == PURE_RAW10_TYPE || g_dumpRAW == PURE_RAW10_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapPureRaw10.raw" , MEDIA_PATH);
        }
        else if(mCapType == PROCESSED_RAW8_TYPE || g_dumpRAW == PROCESSED_RAW8_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapProcRaw8.raw" , MEDIA_PATH);
        }
        else if(mCapType == PROCESSED_RAW10_TYPE || g_dumpRAW == PROCESSED_RAW10_TYPE)
        {
            sprintf(szFileName, "%s/acdkCapProcRaw10.raw" , MEDIA_PATH);
        }
    }
    
    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
        err = ACDK_RETURN_NULL_OBJ;
        goto saveRAWImgExit;
    }

    i4WriteCnt = fwrite(pImgBufIn, 1, mRawIMemInfo.size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        ACDK_LOGE("fync fail");
        fclose(pFp);
        err = ACDK_RETURN_API_FAIL;
        goto saveRAWImgExit;
    }

    ACDK_LOGD("Save image file name:%s,w(%u),h(%u)", szFileName, rawWidth, rawHeight);

    fclose(pFp);

saveRAWImgExit :

    ACDK_LOGD("-");
    return err;
}


/*******************************************************************************
*Functionality : take care of command and used as API for upper layer
********************************************************************************/
MINT32 AcdkMain::sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8        *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8        *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32       *pu4RealParaOutLen
)
{
    MINT32 err = ACDK_RETURN_NO_ERROR;

    ACDK_LOGD("cmd(0x%x)", a_u4Ioctl);

    if (a_u4Ioctl == ACDK_CMD_QV_IMAGE)
    {
        //ACDK_LOGD("ACDK_CMD_QV_IMAGE");
        //err = quickViewImg((MUINT8*)puParaIn, (Func_CB)puParaOut);

        ACDK_LOGD("no need to call QV command right now");
    }
    else if (a_u4Ioctl == ACDK_CMD_RESET_LAYER_BUFFER)
    {
        ACDK_LOGD("ACDK_CMD_RESET_LAYER_BUFFER");
        
        err = m_pAcdkSurfaceViewObj->resetLayer(0);
    }
    else if (a_u4Ioctl == ACDK_CMD_SET_SRC_DEV)
    {
        MINT32 *pSrcDev = (MINT32 *)puParaIn;
        err = setSrcDev(*pSrcDev);

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_SET_SRC_DEV fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_SET_OPERATION_MODE)
    {
        ACDK_LOGD("ACDK_CMD_SET_OPERATION_MODE not ready");
    }
    else if(a_u4Ioctl == ACDK_CMD_SET_SHUTTER_TIME)
    {
        MINT32 *pShutterTime = (MINT32 *)puParaIn;
        err = setShutterTime(*pShutterTime);

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_SET_SHUTTER_TIME fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_SHUTTER_TIME)
    {        
        *((MUINT32 *)puParaOut) = mGetShutTime;
        ACDK_LOGD("ACDK_CMD_GET_SHUTTER_TIME(%u)",*((MUINT32 *)puParaOut));
    }
    else if(a_u4Ioctl == ACDK_CMD_GET_CHECKSUM)
    {        
        *((MUINT32 *)puParaOut) = mGetCheckSumValue;
        ACDK_LOGD("ACDK_CMD_GET_CHECKSUM(%u)",*((MUINT32 *)puParaOut));
    }   
    else if(a_u4Ioctl == ACDK_CMD_GET_AF_INFO)
    {        
        mGetAFInfo = m_pAcdkMhalObj->acdkMhalGetAFInfo();

        *((MUINT32 *)puParaOut) = mGetAFInfo;;
        ACDK_LOGD("ACDK_CMD_GET_AF_INFO(%u)",*((MUINT32 *)puParaOut));
    }   
    else if(a_u4Ioctl == ACDK_CMD_PREVIEW_START)
    {
        PACDK_PREVIEW_STRUCT pCamPreview = (ACDK_PREVIEW_STRUCT *)puParaIn;

        ACDK_LOGD("ACDK_CMD_PREVIEW_START");
        ACDK_LOGD("Preview Width:%d", pCamPreview->u4PrvW);
        ACDK_LOGD("Preview Height:%d", pCamPreview->u4PrvH);
        
        mTestPatternOut = pCamPreview->u16PreviewTestPatEn;
        if(pCamPreview->eOperaMode == ACDK_OPT_FACTORY_MODE)
        {
            mIsFacotory = 1;
        }

        err = startPreview(pCamPreview->fpPrvCB);

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_PREVIEW_START fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_PREVIEW_STOP)
    {
        ACDK_LOGD("ACDK_CMD_PREVIEW_STOP");
        err = stopPreview();

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_PREVIEW_STOP fail(0x%x)",err);
        }
    }
    else if(a_u4Ioctl == ACDK_CMD_CAPTURE)
    {
        PACDK_CAPTURE_STRUCT pCapConfig = (ACDK_CAPTURE_STRUCT *)puParaIn;
        ACDK_LOGD("ACDK_CMD_CAPTURE");
        ACDK_LOGD("Capture Mode:%d", pCapConfig->eCameraMode);
        ACDK_LOGD("Operation Mode:%d", pCapConfig->eOperaMode);
        ACDK_LOGD("Format:%d", pCapConfig->eOutputFormat);
        ACDK_LOGD("Width:%d", pCapConfig->u2JPEGEncWidth);
        ACDK_LOGD("Height:%d", pCapConfig->u2JPEGEncHeight);
        ACDK_LOGD("CB:0x%x", pCapConfig->fpCapCB);
        ACDK_LOGD("cap cnt:%d", pCapConfig->u4CapCount);
        ACDK_LOGD("isSave:%d", pCapConfig->i4IsSave);

        // get operation mode
        mOperaMode = pCapConfig->eOperaMode;
        mIsSOI     = MFALSE;  //  True : JFPIF header , False : EXIF  header

        switch(pCapConfig->eOutputFormat)
        {
            case JPEG_TYPE :
            case PURE_RAW8_TYPE :
            case PURE_RAW10_TYPE :
            case PROCESSED_RAW8_TYPE :
            case PROCESSED_RAW10_TYPE :
                err = takePicture(pCapConfig->eCameraMode,
                              pCapConfig->eOutputFormat,
                              pCapConfig->fpCapCB,
                              pCapConfig->u2JPEGEncWidth,
                              pCapConfig->u2JPEGEncHeight,
                              pCapConfig->u4CapCount,
                              pCapConfig->i4IsSave);
                break;
            default : ACDK_LOGE("No Support Format");
                      err = ACDK_RETURN_API_FAIL;  
        }        

        if(err != ACDK_RETURN_NO_ERROR)
        {
            ACDK_LOGE("ACDK_CMD_CAPTURE fail(0x%x)",err);
        }
    }
    
   return err;
}

