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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "aaa_hal"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include "aaa_hal.h"
#include <aaa_state.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <awb_mgr.h>
#include <kd_camera_feature.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/hal/sensor_hal.h>
#include <af_mgr.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>
#include <flash_tuning_custom.h>
#include <flash_mgr.h>
#include <lsc_mgr.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
using namespace CPTool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
MINT32        g_bAFThreadLoop = 0;
MINT32        g_semAFIRQWait = 0;

pthread_t     g_AFThread;
sem_t         g_semAFThreadstart;
IspDrv*       g_pIspDrv;

MINT32 Hal3A::sm_3ALogEnable = 0;

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}\

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
		  do { \
			  aee_system_exception( \
				  "Hal3A", \
				  NULL, \
				  DB_OPT_DEFAULT, \
				  String); \
		  } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A*
Hal3A::
createInstance(MINT32 const i4SensorDevId)
{
    Hal3A *pHal3A  = Hal3A::getInstance();

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            pHal3A->init(ESensorDev_Main);
        break;
        case SENSOR_DEV_SUB:
            pHal3A->init(ESensorDev_Sub);
        break;
        case SENSOR_DEV_MAIN_2:
            pHal3A->init(ESensorDev_MainSecond);
        break;
        case SENSOR_DEV_MAIN_3D:
            pHal3A->init(ESensorDev_Main3D);
        break;
        default:
            MY_ERR("Unsupport sensor device: %d\n", i4SensorDevId);
			AEE_ASSERT_3A_HAL("Unsupport sensor device.");
            return MNULL;
        break;
    }

    return pHal3A;
}

Hal3A*
Hal3A::
getInstance()
{
    static Hal3A singleton;

    return &singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3A::
destroyInstance()
{
    uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A::Hal3A()
    : Hal3ABase()
    , m_Users(0)
    , m_Lock()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    , mpFlickerHal(NULL)
    , m_bFaceDetectEnable(MFALSE)

{


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3A::~Hal3A()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3A::
init(MINT32 i4SensorDev)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa_hal.enable", value, "0");
    m_bDebugEnable = atoi(value);
	property_get("debug.aaa.pvlog.enable", value, "0");
	IState::sm_3APvLogEnable = sm_3ALogEnable = atoi(value);
	MY_LOG("IState::sm_3APvLogEnable = %d\n", IState::sm_3APvLogEnable);

    MY_LOG_IF(1,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
	MBOOL bRet = MTRUE;

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG_IF(1,"%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }

    m_i4SensorDev = i4SensorDev;

    MY_LOG_IF(1,"m_i4SensorDev: %d \n", m_i4SensorDev);

    // init
    bRet = postCommand(ECmd_Init);
	if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Init fail.");
    mpIspDrv = IspDrv::createInstance();
    bRet = mpIspDrv->init();
	if (!bRet) AEE_ASSERT_3A_HAL("IspDrv->init fail.");



    createThread();
    ret = EnableAFThread(1);
	if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(1) fail.");

    //flicker create buf
    FlickerHalBase *pFlk;
    pFlk = FlickerHalBase::getInstance();
    pFlk->createBuf();

    //
    bRet = IspTuningMgr::getInstance().init(getSensorDev());
	if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr init fail.");

    android_atomic_inc(&m_Users);

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3A::
uninit()
{
    MY_LOG_IF(1,"[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    MRESULT ret = S_3A_OK;
	MBOOL bRet = MTRUE;

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        destroyThread();
        ret = EnableAFThread(0);
		if (ret != S_3A_OK) AEE_ASSERT_3A_HAL("EnableAFThread(0) fail.");
        bRet = postCommand(ECmd_Uninit);
		if (!bRet) AEE_ASSERT_3A_HAL("ECmd_Uninit fail.");
        if ( mpIspDrv != NULL )
        {
            mpIspDrv->uninit();
            mpIspDrv=NULL;
        }
        bRet = IspTuningMgr::getInstance().uninit();
		if (!bRet) AEE_ASSERT_3A_HAL("IspTuningMgr uninit fail.");

        //flicker release buf
        FlickerHalBase::getInstance()->releaseBuf();
    }
    else    // There are still some users.
    {
        MY_LOG_IF(1,"Still %d users \n", m_Users);
    }

    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::postCommand(ECmd_T const eCmd, MINT32 const i4Arg)
{
	MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] enter, eCmd = %d\n", eCmd);

    if (eCmd == ECmd_CameraPreviewStart || eCmd == ECmd_CamcorderPreviewStart)  {
        //EnableAFThread(1);
        mpFlickerHal = FlickerHalBase::getInstance();
        if    ( mpFlickerHal->open(m_i4SensorDev)!=0 )
         {
             MY_ERR("[Hal3A::sendCommand-init] NULL mpFlickerHal");
         }

    }
    else if  (eCmd == ECmd_PrecaptureStart || eCmd == ECmd_CameraPreviewEnd || eCmd == ECmd_CamcorderPreviewEnd)
    {
        //EnableAFThread(0);

        if(mpFlickerHal!=NULL) //&&(eCmd != ECmd_PrecaptureStart) )
            {


                ERROR_CHECK(mpFlickerHal->close());
                mpFlickerHal=NULL;
            }
    }

    if ((eCmd == ECmd_CameraPreviewEnd) || (eCmd == ECmd_CamcorderPreviewEnd))
    {
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "3A PreviewEnd, start wait semAFProcEnd");
        //Once 3A receive PreviewEnd, 3A cannot return until AF thread also gets AFirq and finish this frame
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] start waiting semAFProcEnd semaphore\n");
        ::sem_wait(&semAFProcEnd); //break wait when it become 1, i.e., AF thread finish and post
        MY_LOG("3A PreviewEnd, wait semAFProcEnd done");
    }

	MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] before StateMgr::sendCmd(%d)\n", eCmd);
    ERROR_CHECK(StateMgr::getInstance().sendCmd(eCmd))
	MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] after StateMgr::sendCmd(%d)\n", eCmd);

    // FIXME: temp added before AF thread is ready
    if (eCmd == ECmd_Update)
    {
        //ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFUpdate))
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);    // Profiling Start.
        ERROR_CHECK(IspTuningMgr::getInstance().validatePerFrame(MFALSE))
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);     // Profiling End.
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] IspTuningMgr::validatePerFrame(MFALSE) done\n");


        {
            int Val;
            MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] start waiting mAFMtx lock\n");
            Mutex::Autolock autoLock(mAFMtx); //lock for critical section
            MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::postCommand] get mAFMtx lock\n");
            if (g_semAFIRQWait == 0)
            {
                //AF thread start wait AF irq
                ::sem_getvalue(&semAFProcEnd, &Val);
                if (Val == 1) ::sem_wait(&semAFProcEnd); //to be 0, it won't block
                ::sem_post(&g_semAFThreadstart);
                MY_LOG_IF(1, "[AFThread] sem_post g_semAFThreadstart, semAFProcEnd before wait = %d\n", Val);
            }
        }

        CPTLog(Event_Pipe_3A_Flicker, CPTFlagStart);    // Profiling Start.
        AaaTimer localTimer("Flicker", getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
        if((mpFlickerHal!=NULL) )
        {
            //MY_LOG("[3a HAL]: Flicker update \n");
            if(m_rParam.u4AntiBandingMode==AE_FLICKER_MODE_AUTO/*2*/)
            {
                ERROR_CHECK(mpFlickerHal->enable(1))
                ERROR_CHECK(mpFlickerHal->update())
				//MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] mpFlickerHal Enable & Update done\n");
            }
            else
            {
                ERROR_CHECK(mpFlickerHal->enable(0))
				//MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] mpFlickerHal disable done\n");
            }
        }
        localTimer.printTime();
        CPTLog(Event_Pipe_3A_Flicker, CPTFlagEnd);    // Profiling End.

    }


    
	MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG,"[Hal3A::postCommand] exit, eCmd = %d\n", eCmd);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setParams(Param_T const &rNewParam)
{
   // AE
   // AE frame rate range   // if the max frame rate equal min frame rate, the frame rate is fix
   ERROR_CHECK(AeMgr::getInstance().setAEMinMaxFrameRate(rNewParam.i4MinFps, rNewParam.i4MaxFps))

   // AE metering mode
   ERROR_CHECK(AeMgr::getInstance().setAEMeteringMode(rNewParam.u4AeMeterMode))
   
   // AE rotate weighting control
   ERROR_CHECK(AeMgr::getInstance().setAERotateDegree(rNewParam.i4RotateDegree))

   // AE ISO speed
   ERROR_CHECK(AeMgr::getInstance().setAEISOSpeed(rNewParam.u4IsoSpeedMode))

   // AE Meter Area
   ERROR_CHECK(AeMgr::getInstance().setAEMeteringArea(&rNewParam.rMeteringAreas))

   // AE Exp mode
   ERROR_CHECK(AeMgr::getInstance().setAEEVCompIndex(rNewParam.i4ExpIndex, rNewParam.fExpCompStep))

   // AE mode
   ERROR_CHECK(AeMgr::getInstance().setAEMode(rNewParam.u4AeMode))

   // AE anti banding
   ERROR_CHECK(AeMgr::getInstance().setAEFlickerMode(rNewParam.u4AntiBandingMode))

   // AE lock
   ERROR_CHECK(AeMgr::getInstance().setAELock(rNewParam.bIsAELock))

   // AE Cam mode
   ERROR_CHECK(AeMgr::getInstance().setAECamMode(rNewParam.u4CamMode))

   // AE Shot mode
   ERROR_CHECK(AeMgr::getInstance().setAEShotMode(rNewParam.u4ShotMode))


   // AF
   ERROR_CHECK(AfMgr::getInstance().setAFMode(rNewParam.u4AfMode))
   if(m_bFaceDetectEnable==MFALSE)
   ERROR_CHECK(AfMgr::getInstance().setAFArea(rNewParam.rFocusAreas))
   ERROR_CHECK(AfMgr::getInstance().setCamMode(rNewParam.u4CamMode))
   ERROR_CHECK(AfMgr::getInstance().setFullScanstep(rNewParam.i4FullScanStep))
   ERROR_CHECK(AfMgr::getInstance().setMFPos(rNewParam.i4MFPos))
   AfMgr::getInstance().setAndroidServiceState(rNewParam.bIsSupportAndroidService);

   // AWB mode
   ERROR_CHECK(AwbMgr::getInstance().setAWBMode(rNewParam.u4AwbMode))

   // AWB lock
   ERROR_CHECK(AwbMgr::getInstance().setAWBLock(rNewParam.bIsAWBLock))

   // ISP tuning
   ERROR_CHECK(IspTuningMgr::getInstance().setSceneMode(rNewParam.u4SceneMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setEffect(rNewParam.u4EffectMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Bright(rNewParam.u4BrightnessMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Hue(rNewParam.u4HueMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Sat(rNewParam.u4SaturationMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Edge(rNewParam.u4EdgeMode))
   ERROR_CHECK(IspTuningMgr::getInstance().setIspUserIdx_Contrast(rNewParam.u4ContrastMode))

   //flash
   ERROR_CHECK(FlashMgr::getInstance()->setSensorDev(m_i4SensorDev))
   ERROR_CHECK(FlashMgr::getInstance()->setFlashMode(rNewParam.u4StrobeMode))
   ERROR_CHECK(FlashMgr::getInstance()->setAfLampMode(rNewParam.u4AfLampMode))
   ERROR_CHECK(FlashMgr::getInstance()->setShotMode(rNewParam.u4ShotMode))
   ERROR_CHECK(FlashMgr::getInstance()->setCamMode(rNewParam.u4CamMode))
   ERROR_CHECK(FlashMgr::getInstance()->setEvComp(rNewParam.i4ExpIndex, rNewParam.fExpCompStep))


   ERROR_CHECK(FlickerHalBase::getInstance()->setFlickerMode(rNewParam.u4AntiBandingMode))



    m_rParam = rNewParam;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getSupportedParams(FeatureParam_T &rFeatureParam)
{
    // FIXME:
    rFeatureParam.bExposureLockSupported = AeMgr::getInstance().isAELockSupported();
    rFeatureParam.bAutoWhiteBalanceLockSupported = AwbMgr::getInstance().isAWBLockSupported();
    //rFeatureParam.u4MaxFocusAreaNum;
    rFeatureParam.u4MaxMeterAreaNum = AeMgr::getInstance().getAEMaxMeterAreaNum();
    AeMgr::getInstance().getNvramData(m_i4SensorDev);
	AE_DEVICES_INFO_T rDeviceInfo;
	AeMgr::getInstance().getSensorDeviceInfo(rDeviceInfo);
	rFeatureParam.u4FocusLength_100x = rDeviceInfo.u4FocusLength_100x;

    rFeatureParam.u4MaxFocusAreaNum = AfMgr::getInstance().getAFMaxAreaNum();
    rFeatureParam.i4MaxLensPos = AfMgr::getInstance().getMaxLensPos();
    rFeatureParam.i4MinLensPos = AfMgr::getInstance().getMinLensPos();
    rFeatureParam.i4AFBestPos = AfMgr::getInstance().getAFBestPos();
    rFeatureParam.i8BSSVlu = AfMgr::getInstance().getBestShotValue();

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::autoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

    if ((m_rParam.u4AfMode != AF_MODE_AFC) && (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO))   {
        ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFStart))
    }
    AfMgr::getInstance().autoFocus();
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::cancelAutoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);
    if ((m_rParam.u4AfMode != AF_MODE_AFC) && (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO))   {
        ERROR_CHECK(StateMgr::getInstance().sendCmd(ECmd_AFEnd))
    }
    AfMgr::getInstance().cancelAutoFocus();
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    ERROR_CHECK(AeMgr::getInstance().setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height))
    ERROR_CHECK(AfMgr::getInstance().setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height))
    ERROR_CHECK(IspTuningMgr::getInstance().setZoomRatio(u4ZoomRatio_x100))
    ERROR_CHECK(FlashMgr::getInstance()->setDigZoom(u4ZoomRatio_x100))

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
    EXIF_INFO_T rEXIFInfo;
    AE_DEVICES_INFO_T rDeviceInfo;
    AE_MODE_CFG_T rCaptureInfo;

    AeMgr::getInstance().getSensorDeviceInfo(rDeviceInfo);
    rEXIFInfo.u4FNumber = rDeviceInfo.u4LensFno; // Format: F2.8 = 28
    rEXIFInfo.u4FocalLength = rDeviceInfo.u4FocusLength_100x; // Format: FL 3.5 = 350
    rEXIFInfo.u4SceneMode = m_rParam.u4SceneMode; // Scene mode   (SCENE_MODE_XXX)
    rEXIFInfo.u4AEMeterMode = AeMgr::getInstance().getAEMeterMode(); // Exposure metering mode   (AE_METERING_MODE_XXX)
    rEXIFInfo.i4AEExpBias = AeMgr::getInstance().getEVCompensateIndex(); // Exposure index  (AE_EV_COMP_XX)
    rEXIFInfo.u4AEISOSpeed = AeMgr::getInstance().getAEISOSpeedMode();

    rEXIFInfo.u4AWBMode = m_rParam.u4AwbMode;

    AeMgr::getInstance().getCaptureParams(0, 0, rCaptureInfo);
    rEXIFInfo.u4CapExposureTime = rCaptureInfo.u4Eposuretime;
    if(FlashMgr::getInstance()->isFlashOnCapture())
        rEXIFInfo.u4FlashLightTimeus=30000;
    else
        rEXIFInfo.u4FlashLightTimeus=0;

    rEXIFInfo.u4RealISOValue = rCaptureInfo.u4RealISO;

    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setDebugInfo(IBaseCamExif *pIBaseCamExif) const
{
    // 3A debug info
    static AAA_DEBUG_INFO_T r3ADebugInfo;

    r3ADebugInfo.hdr.u4KeyID = AAA_DEBUG_KEYID;
    r3ADebugInfo.hdr.u4ModuleCount = MODULE_NUM(6,5);

    r3ADebugInfo.hdr.u4AEDebugInfoOffset        = sizeof(r3ADebugInfo.hdr);
    r3ADebugInfo.hdr.u4AFDebugInfoOffset        = r3ADebugInfo.hdr.u4AEDebugInfoOffset + sizeof(AE_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugInfoOffset       = r3ADebugInfo.hdr.u4AFDebugInfoOffset + sizeof(AF_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlashDebugInfoOffset     = r3ADebugInfo.hdr.u4AWBDebugInfoOffset + sizeof(AWB_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4FlickerDebugInfoOffset   = r3ADebugInfo.hdr.u4FlashDebugInfoOffset + sizeof(FLASH_DEBUG_INFO_T);
    r3ADebugInfo.hdr.u4AWBDebugDataOffset       = r3ADebugInfo.hdr.u4FlickerDebugInfoOffset + sizeof(FLICKER_DEBUG_INFO_T);

    // AE
    AeMgr::getInstance().getDebugInfo(r3ADebugInfo.rAEDebugInfo);//, r3ADebugInfo.rAEPlineDebugInfo);

    // AF
    AfMgr::getInstance().getDebugInfo(r3ADebugInfo.rAFDebugInfo);

    // AWB
    AwbMgr::getInstance().getDebugInfo(r3ADebugInfo.rAWBDebugInfo, r3ADebugInfo.rAWBDebugData);

    // Flash
    FlashMgr::getInstance()->getDebugInfo(&r3ADebugInfo.rFlashDebugInfo);

    // Flicker
//    Flicker::getInstance()->getDebugInfo(&r3ADebugInfo.rFlickerDebugInfo);

    MINT32 ID;
    pIBaseCamExif->sendCommand(CMD_REGISTER, AAA_DEBUG_KEYID, reinterpret_cast<MINT32>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&r3ADebugInfo), sizeof(AAA_DEBUG_INFO_T));

    // LSC
    DEBUG_SHAD_INFO_T rShadingDbgInfo;
    LscMgr::getInstance()->getDebugInfo(rShadingDbgInfo);
    pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_CAM_SHAD_MID, reinterpret_cast<MINT32>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rShadingDbgInfo), sizeof(rShadingDbgInfo));

    // LSC table
    DEBUG_SHAD_ARRAY_INFO_T rShadingDbgTbl;
    LscMgr::getInstance()->getDebugTbl(rShadingDbgTbl);
    pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_CAM_SHAD_ARRAY_MID, reinterpret_cast<MINT32>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rShadingDbgTbl), sizeof(rShadingDbgTbl));

    // ISP debug info
    NSIspExifDebug::IspExifDebugInfo_T rIspExifDebugInfo;
    IspTuningMgr::getInstance().getDebugInfo(rIspExifDebugInfo);

    pIBaseCamExif->sendCommand(CMD_REGISTER, static_cast<MINT32>(rIspExifDebugInfo.hdr.u4KeyID), reinterpret_cast<MINT32>(&ID));
    pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MINT32>(&rIspExifDebugInfo), sizeof(rIspExifDebugInfo));

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getDelayFrame(EQueryType_T const eQueryType) const
{
    switch (eQueryType)
    {
    case EQueryType_Init:
        return 3;
        break;
    case EQueryType_Effect:
    case EQueryType_AWB:
    default:
        return 0;
        break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setIspProfile(EIspProfile_T const eIspProfile)
{
    ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(eIspProfile))
    ERROR_CHECK(IspTuningMgr::getInstance().validate())

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo)
{
    AE_MODE_CFG_T rCaptureInfo;

    AeMgr::getInstance().getCaptureParams(index, i4EVidx, rCaptureInfo);

    a_rCaptureInfo.u4ExposureMode = rCaptureInfo.u4ExposureMode;
    a_rCaptureInfo.u4Eposuretime = rCaptureInfo.u4Eposuretime;
    a_rCaptureInfo.u4AfeGain = rCaptureInfo.u4AfeGain;
    a_rCaptureInfo.u4IspGain = rCaptureInfo.u4IspGain;
    a_rCaptureInfo.u4RealISO = rCaptureInfo.u4RealISO;
    a_rCaptureInfo.u4FlareGain = (MUINT32) rCaptureInfo.i2FlareGain;
    a_rCaptureInfo.u4FlareOffset = (MUINT32) rCaptureInfo.i2FlareOffset;
    a_rCaptureInfo.i4LightValue_x10 = AeMgr::getInstance().getCaptureLVvalue();
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    AE_MODE_CFG_T rCaptureInfo;

    rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
    rCaptureInfo.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    rCaptureInfo.u4AfeGain = a_rCaptureInfo.u4AfeGain;
    rCaptureInfo.u4IspGain = a_rCaptureInfo.u4IspGain;
    rCaptureInfo.u4RealISO = a_rCaptureInfo.u4RealISO;
    rCaptureInfo.i2FlareGain = (MINT16)a_rCaptureInfo.u4FlareGain;
    rCaptureInfo.i2FlareOffset = (MINT16)a_rCaptureInfo.u4FlareOffset;

    AeMgr::getInstance().updateCaptureParams(rCaptureInfo);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
    AeMgr::getInstance().getHDRCapInfo(a_strHDROutputInfo);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getRTParams(FrameOutputParam_T &a_strFrameOutputInfo)
{
    AeMgr::getInstance().getRTParams(a_strFrameOutputInfo);
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3A::setFDEnable(MBOOL a_sFacesEn)
{
    m_bFaceDetectEnable=a_sFacesEn;
}

MBOOL Hal3A::setFDInfo(MVOID* a_sFaces)
{
    AfMgr::getInstance().setFDInfo(a_sFaces);
    AeMgr::getInstance().setFDInfo(a_sFaces);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setOTInfo(MVOID* a_sOT)
{
    AfMgr::getInstance().setOTInfo(a_sOT);
	AeMgr::getInstance().setOTInfo(a_sOT);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::enableAELimiterControl(MBOOL  bIsAELimiter)
{
    AeMgr::getInstance().setAELimiterMode(bIsAELimiter);
    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3A::getFlashFrameNumBeforeCapFrame()
{
    //this flash API is valid after 3A StatePrecapture
    return FlashMgr::getInstance()->isFlashOnCapture() ? 3 : 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3A::onFireCapFlashIfNeeded()
{
    FlashMgr::getInstance()->capCheckAndFireFlash_Start(); //Originally, it's in 3A ECmd_CaptureStart
    //PS: in ECmd_CaptureEnd, we have FlashMgr::getInstance()->capCheckAndFireFlash_End();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getASDInfo(ASDInfo_T &a_rASDInfo)
{
    // AWB
    AWB_ASD_INFO_T rAWBASDInfo;
    AwbMgr::getInstance().getASDInfo(rAWBASDInfo);
    a_rASDInfo.i4AWBRgain_X128 = rAWBASDInfo.i4AWBRgain_X128;
    a_rASDInfo.i4AWBBgain_X128 = rAWBASDInfo.i4AWBBgain_X128;
    a_rASDInfo.i4AWBRgain_D65_X128 = rAWBASDInfo.i4AWBRgain_D65_X128;
    a_rASDInfo.i4AWBBgain_D65_X128 = rAWBASDInfo.i4AWBBgain_D65_X128;
    a_rASDInfo.i4AWBRgain_CWF_X128 = rAWBASDInfo.i4AWBRgain_CWF_X128;
    a_rASDInfo.i4AWBBgain_CWF_X128 = rAWBASDInfo.i4AWBBgain_CWF_X128;
    a_rASDInfo.bAWBStable = rAWBASDInfo.bAWBStable;

    a_rASDInfo.i4AFPos = AfMgr::getInstance().getAFPos();
    a_rASDInfo.pAFTable = AfMgr::getInstance().getAFTable();
    a_rASDInfo.i4AFTableOffset = AfMgr::getInstance().getAFTableOffset();
    a_rASDInfo.i4AFTableMacroIdx = AfMgr::getInstance().getAFTableMacroIdx();
    a_rASDInfo.i4AFTableIdxNum = AfMgr::getInstance().getAFTableIdxNum();
    a_rASDInfo.bAFStable = AfMgr::getInstance().getAFStable();

    a_rASDInfo.i4AELv_x10 = AeMgr::getInstance().getLVvalue(MTRUE);
    a_rASDInfo.bAEBacklit = AeMgr::getInstance().getAECondition(AE_CONDITION_BACKLIGHT);
    a_rASDInfo.bAEStable = AeMgr::getInstance().IsAEStable();
    a_rASDInfo.i2AEFaceDiffIndex = AeMgr::getInstance().getAEFaceDiffIndex();
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::getLCEInfo(LCEInfo_T &a_rLCEInfo)
{
    AeMgr::getInstance().getLCEPlineInfo(a_rLCEInfo);
    return MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT Hal3A::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;

    if (a_bEnable)  {

        if (g_bAFThreadLoop == 0)
        {
            /*ret = AfMgr::getInstance().init();
            if (FAILED(ret)) {
                MY_ERR("AfMgr::getInstance().init() fail\n");
                return ret;
            }*/

            g_pIspDrv = IspDrv::createInstance();

            if (!g_pIspDrv) {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (g_pIspDrv->init() < 0) {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }

            // create AF thread
            MY_LOG("[AFThread] Create");
            g_bAFThreadLoop = 1;
            sem_init(&g_semAFThreadstart, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_AF_THREAD};
            pthread_create(&g_AFThread, &attr, AFThreadFunc, this);
        }
    }
    else   {

        if (g_bAFThreadLoop == 1)
        {

            if (g_pIspDrv)   {
                g_pIspDrv->uninit();
                g_pIspDrv = NULL;
            }
            g_bAFThreadLoop = 0;
            ::sem_post(&g_semAFThreadstart);

            pthread_join(g_AFThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

    return ret;
}

MVOID * Hal3A::AFThreadFunc(void *arg)
{
    MY_LOG("[AFThread] tid: %d \n", gettid());
    ::prctl(PR_SET_NAME,"AFthread", 0, 0, 0);

    if (!g_pIspDrv) {
        MY_LOG("[AFThread] m_pIspDrv null\n");
        return NULL;
    }
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);

    // wait AFO done
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_AF_DON_ST;
    WaitIrq.Timeout = 500; // 200 msec

    while (g_bAFThreadLoop) {
        //
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (1)\n");
        {Mutex::Autolock autoLock(_this->mAFMtx); g_semAFIRQWait = 0;}
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (1)\n");
        
        ::sem_wait(&g_semAFThreadstart);
        MY_LOG_IF(1, "[Hal3A::AFThreadFunc] sem_wait g_semAFThreadstart done\n");

        {Mutex::Autolock autoLock(_this->mAFMtx); g_semAFIRQWait = 1;}
        MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (2)\n");

        if (g_bAFThreadLoop != 0)
        {
            AfMgr::getInstance().DoCallback();
			MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");
            if (g_pIspDrv->waitIrq(WaitIrq) > 0) // success
            {
                MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
                StateMgr::getInstance().sendCmd(ECmd_AFUpdate);
				MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");
            }
            else
            {
                MY_LOG("[AFThread] AF irq timeout\n");
                AfMgr::getInstance().TimeOutHandle();
                AfMgr::getInstance().printAFConfigLog0();
            }

            {
                int Val2;
                MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] start waiting mAFMtx lock (3)\n");
                Mutex::Autolock autoLock(_this->mAFMtx);
                MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AFThreadFunc] get mAFMtx lock (3)\n");
                
                ::sem_getvalue(&_this->semAFProcEnd, &Val2);
                MY_LOG_IF(1, "[AFThread] semAFProcEnd before post = %d\n", Val2);
                if (Val2 == 0) ::sem_post(&_this->semAFProcEnd); //to be 1
            }

        }
    }

    //::sem_post(&g_semAFThreadEnd);

    MY_LOG("[AFThread] End \n");

    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3A::setCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);

    return AfMgr::getInstance().setCallbacks(cb);
}




/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::waitVSirq()
{

    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Timeout = 3636; // 5000 msec
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    //no ISP_DRV_IRQ_INT_STATUS_VS2_ST;


    if (mpIspDrv==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    if (mpIspDrv->waitIrq(WaitIrq) <= 0)
    {
        MY_ERR("wait vsync timeout");
    }

}

/******************************************************************************
*
*******************************************************************************/
MINT32 Hal3A::isNeedFiringFlash()
{
    return FlashMgr::getInstance()->isNeedFiringFlash();
}
/******************************************************************************
*
*******************************************************************************/
MVOID Hal3A::endContinuousShotJobs()
{
    FlashMgr::getInstance()->turnOffFlashDevice();
}
