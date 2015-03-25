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
#define LOG_TAG "aaa_hal_yuv"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
//#include <aaa_state.h>   //by jmac
//#include <camera_custom_nvram.h>
//#include <awb_param.h>
//#include <awb_mgr.h>
//#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
//#include <af_mgr.h>
#include <flash_param.h>
//#include <isp_tuning_mgr.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/hal/sensor_hal.h>
//#include <ae_param.h>
#include <mtkcam/common.h>
using namespace NSCam;
//#include <ae_mgr.h>
#include <kd_camera_feature.h>
#include <mtkcam/common/faces.h>
#include "aaa_hal_yuv.h"
#include "math.h"
#include "camera_custom_flashlight.h"
#include "camera_custom_if.h"
#include <cutils/properties.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>


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

#define GET_PROP(prop, dft, val)\
{\
   char value[PROPERTY_VALUE_MAX] = {'\0'};\
   property_get(prop, value, (dft));\
   (val) = atoi(value);\
}

typedef enum
{
    E_YUV_SAF_DONE = 0,
    E_YUV_SAF_FOCUSING = 1,
    E_YUV_SAF_INCAF = 2
} E_YUV_SAF; 

static MBOOL _bEnableMyLog = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv*
Hal3AYuv::
createInstance(MINT32 const i4SensorDevId)
{
    Hal3AYuv *pHal3AYuv  = Hal3AYuv::getInstance();

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN:
            pHal3AYuv->init(ESensorDev_Main);
        break;
        case SENSOR_DEV_SUB:
            pHal3AYuv->init(ESensorDev_Sub);
        break;
        case SENSOR_DEV_MAIN_2:
            pHal3AYuv->init(ESensorDev_MainSecond);
        break;
        case SENSOR_DEV_MAIN_3D:
            pHal3AYuv->init(ESensorDev_Main3D);
        break;
        case SENSOR_DEV_ATV:
            pHal3AYuv->init(ESensorDev_Atv);
        break;
        default:
            MY_ERR("Unsupport sensor device: %d\n", i4SensorDevId);
            return MNULL;
        break;
    }

    return pHal3AYuv;
}

Hal3AYuv*
Hal3AYuv::
getInstance()
{
    static Hal3AYuv singleton;
    return &singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AYuv::
destroyInstance()
{
    uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::Hal3AYuv()
    : Hal3ABase()
    , m_Users(0)
    , m_Lock()
    , m_LockAF()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , bAELockSupp(0)
    , bAWBLockSupp(0)
    , m_bAFThreadLoop(0)
    , m_pIspDrv(NULL)
    , m_fgAfTrig(MFALSE)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::~Hal3AYuv()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
init(MINT32 i4SensorDev)
{
    MY_LOG("[%s()] m_Users: %d \n", __FUNCTION__, m_Users);
    MINT32 i4CurrLensId = 0;
    MINT32 i4CurrSensorId = 0;    
    MRESULT ret = S_3A_OK;

   	Mutex::Autolock lock(m_Lock);
   
   	if (m_Users > 0){
      		MY_LOG("%d has created \n", m_Users);
      		android_atomic_inc(&m_Users);
      		return S_3A_OK;
   	}
    //SensorHal init
    if (!m_pSensorHal)   {
        m_pSensorHal = SensorHal::createInstance();
        MY_LOG("[m_pSensorHal]:0x%08x \n",m_pSensorHal);
        if (!m_pSensorHal) {
            MY_ERR("SensorHal::createInstance() fail \n");
            return ret;
        }
    }
    m_i4SensorDev = i4SensorDev;
    // lens init    
    //m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (MINT32)&m_i4SensorDev, 0, 0);
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev), SENSOR_CMD_GET_SENSOR_ID, (MINT32)&i4CurrSensorId, 0, 0);
    MCUDrv::lensSearch(m_i4SensorDev, i4CurrSensorId);
    i4CurrLensId = MCUDrv::getCurrLensID();    
    m_bIsdummylens = (i4CurrLensId == SENSOR_DRIVE_LENS_ID) ? FALSE : TRUE;
    MY_LOG("[currLensId] 0x%x,dummylens(%d)\n", i4CurrLensId,m_bIsdummylens);
    m_imageXS = 320;
   	m_imageYS = 240;

    // init strobe
    m_pStrobeDrvObj = StrobeDrv::createInstance();
    if (m_pStrobeDrvObj)
    {   
        m_pStrobeDrvObj->init(i4SensorDev);
        m_aeFlashlightType = m_pStrobeDrvObj->getFlashlightType();
        if (m_aeFlashlightType == StrobeDrv::FLASHLIGHT_NONE)
        {
            m_pStrobeDrvObj->uninit();
            m_pStrobeDrvObj->destroyInstance();
            m_pStrobeDrvObj = NULL;
        }
        MY_LOG("strobe type:%d\n",m_aeFlashlightType);
    }
    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->setState(0);
        if (m_pStrobeDrvObj->setStep(NSCamCustom::custom_GetYuvFlashlightStep()) == MHAL_NO_ERROR)
        {
            MY_LOG("setStep: %d\n", NSCamCustom::custom_GetYuvFlashlightStep());
        }
    }

    BV_THRESHOLD = NSCamCustom::custom_GetYuvFlashlightThreshold();
    m_preflashFrmCnt = NSCamCustom::custom_GetYuvFlashlightFrameCnt();
    m_strobeWidth = NSCamCustom::custom_GetYuvFlashlightDuty();
    m_i4AutoFocus = FALSE;
    m_i4AutoFocusTimeout = 0;
    m_bAeLimiter = 0;
    m_i4FDFrmCnt = 0;
    m_i4FDApplyCnt = 0;
    m_i4WinState = 0;
    
    // init ASD
    SENSOR_AE_AWB_REF_STRUCT ref;
    memset(&ref, 0, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev), SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN, (MINT32)&ref, 0, 0);
    memcpy(&m_AsdRef, &ref, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_AsdRef.SensorLV05LV13EVRef = 
        ASDLog2Func(ref.SensorAERef.AeRefLV05Shutter * ref.SensorAERef.AeRefLV05Gain,
                    ref.SensorAERef.AeRefLV13Shutter * ref.SensorAERef.AeRefLV13Gain);
    MY_LOG("[%s] ASD AE Ref: Lv05S(%d) Lv05G(%d) Lv13S(%d) Lv13G(%d) EVRef(%d)\n", __FUNCTION__,
        ref.SensorAERef.AeRefLV05Shutter, ref.SensorAERef.AeRefLV05Gain,
        ref.SensorAERef.AeRefLV13Shutter, ref.SensorAERef.AeRefLV13Gain,
        m_AsdRef.SensorLV05LV13EVRef);
	
    GET_PROP("debug.aaa_hal_yuv.log", "0", _bEnableMyLog);
	
    // init
    sendCommand(ECmd_Init, 0);
    m_fgAfTrig = MFALSE;

    EnableAFThread(1);

    android_atomic_inc(&m_Users);

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
uninit()
{
    MRESULT ret = S_3A_OK;

    MY_LOG("[%s()] m_Users: %d \n", __FUNCTION__, m_Users);
    
    Mutex::Autolock lock(m_Lock);
    
    // If no more users, return directly and do nothing.
    if (m_Users <= 0){
    	   return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    // There is no more User after decrease one User
    if (m_Users == 0) {
        //Reset Parameter
        Param_T npara;
        m_rParam = npara;
        EnableAFThread(0);
        sendCommand(ECmd_Uninit, 0);
        //SensorHal uninit
        if (m_pSensorHal){
            m_pSensorHal->destroyInstance();
            m_pSensorHal = NULL;
        }
    }
    // There are still some users
    else{
    	   MY_LOG("Still %d users \n", m_Users);
    }
    
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::sendCommand(ECmd_T const eCmd, MINT32 const i4Arg)
{
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT mflashInfo;
    const MINT32 mflashcnt = NSCamCustom::custom_GetYuvFlashlightFrameCnt();

    if (eCmd != ECmd_Update)
    {
        m_i4State = eCmd;
        m_i4AFSwitchCtrl = -1;
        MY_LOG("[%s()],%d\n", __FUNCTION__,eCmd);
    }

    if (eCmd == ECmd_Init)
    {        
        //EnableAFThread(1);
        m_i4AFSwitchCtrl = -1;
        return MTRUE;
    }
    else if (eCmd == ECmd_CameraPreviewStart || eCmd == ECmd_CamcorderPreviewStart)
    {        
        //Force reset Parameter
        Param_T old_para,rst_para;
        memset(&rst_para, 0, sizeof(Param_T));
        old_para = m_rParam;
        m_rParam = rst_para;
        m_bForceUpdatParam = TRUE;
        setParams(old_para);
        m_bForceUpdatParam = FALSE;
        
        m_bExifFlashOn = 0;
        
        return MTRUE;
    }
    else if  (eCmd == ECmd_Uninit)
    {
        //EnableAFThread(0);
        if (m_pStrobeDrvObj)
        {
            m_bFlashActive = FALSE;
            m_pStrobeDrvObj->setOnOff(0);
            m_pStrobeDrvObj->uninit();
            m_pStrobeDrvObj->destroyInstance();
            m_pStrobeDrvObj = NULL;
        }
        return MTRUE;
    }
    else if (eCmd == ECmd_PrecaptureStart)
    {
        if (m_pStrobeDrvObj)
        {
            MBOOL fgFlashOn = (eShotMode_ZsdShot == m_rParam.u4ShotMode) ? m_isFlashOnCapture : isAEFlashOn();
            if (fgFlashOn)
            {
                m_strobecurrent_BV = m_strobeTrigerBV;
                // updated in isAEFlashOn
                m_strobePreflashBV = m_strobeTrigerBV;                    
                //ON flashlight
                if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
                {
                    MY_LOG("setTimeOutTime: 0\n");
                }
                if (m_pStrobeDrvObj->setDuty(m_strobeWidth) == MHAL_NO_ERROR)
                {
                    MY_LOG("setLevel:%d\n", m_strobeWidth);
                }
                if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
                {
                    MY_LOG("setFire ON\n");
                    m_preflashFrmCnt = mflashcnt - 1;
                    m_preflashFrmCnt = m_preflashFrmCnt < mflashcnt ? m_preflashFrmCnt : 0;
                    m_bFlashActive = TRUE;
                    m_bExifFlashOn = 1;
                }

                if (NSCamCustom::custom_GetYuvPreflashAF())
                {
                    MINT32 i4AfState = isFocused();
                    MY_LOG("ECmd_PrecaptureStart: i4AfState(%d)\n", i4AfState);
                    
                    setAFMode(AF_MODE_AFS);
                    m_i4AutoFocus = TRUE;
                    m_i4AutoFocusTimeout = 30;
                    resetAFAEWindow();
                    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
                    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,0,0,0); 
                }
            }
            else
            {
                m_bReadyToCapture = 1;
            }
        }
        else
        {
            m_bReadyToCapture = 1;
        }

        return MTRUE;
    }
    else if (eCmd == ECmd_PrecaptureEnd)
    {
        resetReadyToCapture();
        m_isFlashOnCapture = 0;
        return MTRUE;
    }
    else if (eCmd == ECmd_CaptureStart)
    {
        MY_LOG("ECmd_CaptureStart: shotMode = %d\n", m_rParam.u4ShotMode);
        updateAeFlashCaptureParams();
        return MTRUE;
    }
    else if (eCmd == ECmd_CaptureEnd)
    {
        MY_LOG("ECmd_CaptureEnd: shotMode = %d\n", m_rParam.u4ShotMode);
        if (m_pStrobeDrvObj && m_bFlashActive == TRUE && m_rParam.u4ShotMode != CAPTURE_MODE_BURST_SHOT)
        {
            m_pStrobeDrvObj->setOnOff(0);
            MY_LOG("setFire OFF\n");
            
            m_bFlashActive = FALSE;
        }
        return MTRUE;
    }
    else if (eCmd == ECmd_RecordingStart)
    {
        MY_LOG("ECmd_RecordingStart:\n");

        if (m_pStrobeDrvObj && m_rParam.u4StrobeMode == FLASHLIGHT_AUTO)
        {
            MBOOL fgFlashOn = isAEFlashOn();
            if (fgFlashOn)
            {                
                //ON flashlight
                if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
                {
                    MY_LOG("setTimeOutTime: 0\n");
                }
                if (m_pStrobeDrvObj->setDuty(m_strobeWidth) == MHAL_NO_ERROR)
                {
                    MY_LOG("setLevel:%d\n", m_strobeWidth);
                }
                if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
                {
                    MY_LOG("setFire ON\n");
                    m_bFlashActive = TRUE;
                    m_bExifFlashOn = 1;
                }
            }
        }
        return MTRUE;
    }
    else if (eCmd == ECmd_RecordingEnd)
    {
        MY_LOG("ECmd_RecordingEnd:\n");
        if (m_pStrobeDrvObj && m_bFlashActive == TRUE)
        {
            m_pStrobeDrvObj->setOnOff(0);
            MY_LOG("setFire OFF\n");
            
            m_bFlashActive = FALSE;
        }
        return MTRUE;
    }
    else if (eCmd == ECmd_Update)
    {
        //MY_LOG("[%s()],ECmd_Update\n", __FUNCTION__);
        switch (m_i4State)
        {
        case ECmd_PrecaptureStart:
            if (m_pStrobeDrvObj)
            {
                if (m_bFlashActive == TRUE)
                {
                    if (NSCamCustom::custom_GetYuvPreflashAF())
                    {
                        if (m_i4AutoFocus)
                        {
                            MINT32 i4AfState = isFocused();
                            MY_LOG("ECmd_PrecaptureStart_ECmd_Update: SAF(%d)\n", i4AfState);
                            if (i4AfState == SENSOR_AF_FOCUSED)
                            {
                                m_i4AutoFocus = FALSE;
                                m_preflashFrmCnt += 2;
                            }
                            else if (m_i4AutoFocusTimeout == 0)
                            {
                                m_i4AutoFocus = FALSE;
                            }
                            m_i4AutoFocusTimeout = m_i4AutoFocusTimeout > 0 ? m_i4AutoFocusTimeout - 1 : 0;
                        }
                    }

                    m_strobePreflashBV = calcBV();
                    m_preflashFrmCnt = m_preflashFrmCnt > 0 ? m_preflashFrmCnt - 1 : 0;
                    
                    if (0 == m_preflashFrmCnt && FALSE == m_i4AutoFocus)
                    {
                        memset(&mflashInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));                    
                        //fixme
                        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_STROBE_INFO,(int)&mflashInfo,0,0); 

                        pre_shutter = mflashInfo.Exposuretime;
                        pre_gain = mflashInfo.Gain;
                        //OFF flashlight after preflash done.
                        if (m_bFlashActive == TRUE)
                        {
                            setAeLock(MTRUE);
                            setAwbLock(MTRUE);
                            if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                            {
                                MY_LOG("setFire OFF\n");
                            }
                        }
                        m_preflashFrmCnt = mflashcnt;
                        m_bReadyToCapture = 1;
                        MY_LOG("custom flash cnt:%d\n",mflashcnt);
                    }
                }
            }
            break;
        case ECmd_PrecaptureEnd:
        case ECmd_CaptureStart:
        case ECmd_CaptureEnd:
            break;
        default:
            //MY_LOG("ECmd_Update:\n");
            ::sem_post(&m_semAFThreadStart);
            break;
        } 
        return MTRUE;
    }
    else
    {
        MY_LOG("undefine \n");    
        return MTRUE;
    }
}

MBOOL Hal3AYuv::isInVideo()
{
    MBOOL fgVdo = 
        (m_i4State == ECmd_CamcorderPreviewStart) || (m_i4State == ECmd_CamcorderPreviewEnd) ||
        (m_i4State == ECmd_RecordingStart) || (m_i4State == ECmd_RecordingEnd);
    return fgVdo;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setAeLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AE_LOCK : SENSOR_3A_AE_UNLOCK;
    
    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (bAWBLockSupp == 1)
    {
        MY_LOG("AE Lock supports, send CMD\n");
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_3A_CMD,(int)&iYuv3ACmd,0,0);
    }

    return MTRUE;

}

MBOOL Hal3AYuv::setAwbLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AWB_LOCK : SENSOR_3A_AWB_UNLOCK;
    
    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (bAWBLockSupp == 1)
    {
        MY_LOG("AWB Lock supports, send CMD\n");
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_3A_CMD,(int)&iYuv3ACmd,0,0);
    }

    return MTRUE;
}

MINT32 Hal3AYuv::enableAELimiterControl(MBOOL  bIsAELimiter)
{
    MY_LOG("[%s] bIsAELimiter = %d\n", __FUNCTION__, bIsAELimiter);

    m_bAeLimiter = bIsAELimiter;
        
    setAeLock(bIsAELimiter);
    setAwbLock(bIsAELimiter);
        
    return MTRUE;
}

MBOOL Hal3AYuv::setParams(Param_T const &rNewParam)
{
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0; 
    MINT32 i4SceneModeUpdate;
    MINT32 i4SceneModeChg;

    MY_LOG("[%s()] + \n", __FUNCTION__);

    i4SceneModeUpdate = 1;//rNewParam.u4SceneMode != SCENE_MODE_HDR;

    i4SceneModeChg = m_rParam.u4SceneMode != rNewParam.u4SceneMode || m_bForceUpdatParam;

    if (m_rParam.u4EffectMode != rNewParam.u4EffectMode || m_bForceUpdatParam){
        MY_LOG("[FID_COLOR_EFFECT],(%d)->(%d) \n",m_rParam.u4EffectMode,rNewParam.u4EffectMode);
        yuvCmd = FID_COLOR_EFFECT;
        yuvParam = rNewParam.u4EffectMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }

    // scene mode
    if (i4SceneModeUpdate)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE],(%d)->(%d) \n",m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            yuvCmd = FID_SCENE_MODE;
            yuvParam = rNewParam.u4SceneMode;  
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        
        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV],Idx:(%d)->(%d),Step:(%f)->(%f) \n",m_rParam.i4ExpIndex,rNewParam.i4ExpIndex,m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            yuvCmd = FID_AE_EV;
            yuvParam = mapAEToEnum(rNewParam.i4ExpIndex,rNewParam.fExpCompStep);        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }

        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE],(%d)->(%d) \n",m_rParam.u4AwbMode,rNewParam.u4AwbMode);
            yuvCmd = FID_AWB_MODE;
            yuvParam = rNewParam.u4AwbMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }

        if (m_rParam.u4BrightnessMode != rNewParam.u4BrightnessMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_BRIGHT],(%d)->(%d) \n",m_rParam.u4BrightnessMode,rNewParam.u4BrightnessMode);
            yuvCmd = FID_ISP_BRIGHT;
            yuvParam = rNewParam.u4BrightnessMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        if (m_rParam.u4HueMode != rNewParam.u4HueMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_HUE],(%d)->(%d) \n",m_rParam.u4HueMode,rNewParam.u4HueMode);
            yuvCmd = FID_ISP_HUE;
            yuvParam = rNewParam.u4HueMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        if (m_rParam.u4SaturationMode != rNewParam.u4SaturationMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_SAT],(%d)->(%d) \n",m_rParam.u4SaturationMode,rNewParam.u4SaturationMode);
            yuvCmd = FID_ISP_SAT;
            yuvParam = rNewParam.u4SaturationMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        if (m_rParam.u4ContrastMode != rNewParam.u4ContrastMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_CONTRAST],(%d)->(%d) \n",m_rParam.u4ContrastMode,rNewParam.u4ContrastMode);
            yuvCmd = FID_ISP_CONTRAST;
            yuvParam = rNewParam.u4ContrastMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        if (m_rParam.u4EdgeMode != rNewParam.u4EdgeMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_EDGE],(%d)->(%d) \n",m_rParam.u4EdgeMode,rNewParam.u4EdgeMode);
            yuvCmd = FID_ISP_EDGE;
            yuvParam = rNewParam.u4EdgeMode;        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
        if (m_rParam.u4IsoSpeedMode != rNewParam.u4IsoSpeedMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_ISO],(%d)->(%d) \n",m_rParam.u4IsoSpeedMode,rNewParam.u4IsoSpeedMode);
            yuvCmd = FID_AE_ISO;
            yuvParam = mapISOToEnum(rNewParam.u4IsoSpeedMode);
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
    }
    else if (rNewParam.u4SceneMode == SCENE_MODE_HDR)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE],(%d)->(SCENE_MODE_HDR) \n",m_rParam.u4SceneMode);
            // set scene mode off (backward compatible) first, then set scene mode hdr (for JB4.2)
            yuvCmd = FID_SCENE_MODE;
            yuvParam = SCENE_MODE_OFF;  
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
            yuvParam = SCENE_MODE_HDR;  
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }

        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV],Idx:(%d)->(%d),Step:(%f)->(%f) \n",m_rParam.i4ExpIndex,rNewParam.i4ExpIndex,m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            yuvCmd = FID_AE_EV;
            yuvParam = mapAEToEnum(rNewParam.i4ExpIndex,rNewParam.fExpCompStep);        
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
		
        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE],(%d)->(%d) \n",m_rParam.u4AwbMode,rNewParam.u4AwbMode);
            yuvCmd = FID_AWB_MODE;
            yuvParam = rNewParam.u4AwbMode; 	   
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
    }
    else
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE],(%d)->(%d) \n",m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            yuvCmd = FID_SCENE_MODE;
            yuvParam = rNewParam.u4SceneMode;  
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
        }
    }

    if (m_rParam.u4AntiBandingMode != rNewParam.u4AntiBandingMode || m_bForceUpdatParam){
        MY_LOG("[FID_AE_FLICKER],(%d)->(%d) \n",m_rParam.u4AntiBandingMode,rNewParam.u4AntiBandingMode);
        yuvCmd = FID_AE_FLICKER;
        yuvParam = rNewParam.u4AntiBandingMode;        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
    
    if ((m_bAeLimiter == 0 && m_rParam.bIsAELock != rNewParam.bIsAELock) || m_bForceUpdatParam)
    {
        setAeLock(rNewParam.bIsAELock);
    }

    if ((m_bAeLimiter == 0 && m_rParam.bIsAWBLock != rNewParam.bIsAWBLock) || m_bForceUpdatParam)
    {
        setAwbLock(rNewParam.bIsAWBLock);
    }
#if 0    
    //for cam-mode
    if (m_rParam.u4CamMode != rNewParam.u4CamMode || m_bForceUpdatParam){
        if (rNewParam.u4CamMode == eAppMode_VideoMode||rNewParam.u4CamMode == eAppMode_VtMode){
            MY_LOG("[FID_CAM_MODE],(%d)->(%d),fps(%d) \n",m_rParam.u4CamMode, rNewParam.u4CamMode,rNewParam.i4MaxFps);
            yuvParam=(rNewParam .i4MaxFps<=20000)?15:30;
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_VIDEO_FRAME_RATE,(int)&yuvParam,0,0);             
        }
    }
#endif
    //for frame rate
    if (m_rParam.i4MaxFps!=rNewParam.i4MaxFps||m_rParam.i4MinFps!=rNewParam.i4MinFps || m_bForceUpdatParam){
        if(rNewParam.i4MinFps==rNewParam.i4MaxFps&&rNewParam.i4MaxFps>0){
            MY_LOG("[FID_FIX_FRAMERATE],Max(%d)->(%d) \n",m_rParam.i4MaxFps,rNewParam.i4MaxFps);
            yuvParam = clamp(rNewParam.i4MaxFps/1000, 5, 30);//(rNewParam.i4MaxFps<=20000)?15:30;
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_VIDEO_FRAME_RATE,(int)&yuvParam,0,0);             
        }
    }
    
    if (m_rParam.u4StrobeMode != rNewParam.u4StrobeMode){
        MY_LOG("StrobeMode=%d\n", rNewParam.u4StrobeMode);
        setFlashMode(rNewParam.u4StrobeMode);
    }

    Mutex::Autolock lock(m_LockAF);

    if (((m_rParam.u4AfMode != rNewParam.u4AfMode) || m_bForceUpdatParam) && !m_bIsdummylens){
        MY_LOG("m_rParam.u4ShotMode=%d,rNewParam.u4ShotMode=%d", m_rParam.u4ShotMode, rNewParam.u4ShotMode);
        //if (rNewParam.u4ShotMode == eShotMode_Autorama &&
        //    rNewParam.u4AfMode == AF_MODE_AFC)
        //{
        //    MY_LOG("[FID_AF_MODE]eShotMode_Autorama(%d)->(%d),dummy(%d) \n",m_rParam.u4AfMode,rNewParam.u4AfMode,m_bIsdummylens);
        //}
        //else
        {
            MY_LOG("[FID_AF_MODE](%d)->(%d),dummy(%d) \n",m_rParam.u4AfMode,rNewParam.u4AfMode,m_bIsdummylens);
            setAFMode(rNewParam.u4AfMode);
            m_rParam.u4AfMode = rNewParam.u4AfMode;
        }
    }
    
    //update AF area
    if (m_max_af_areas > 0) 
    {
        UINT32 u4Diff = 0;
        AREA_T focusArea[MAX_FOCUS_AREAS]; 
        for (MUINT32 i = 0; i < rNewParam.rFocusAreas.u4Count; i++)
        {
            u4Diff += (UINT32)
                (m_rParam.rFocusAreas.rAreas[i].i4Left   != rNewParam.rFocusAreas.rAreas[i].i4Left) +
                (m_rParam.rFocusAreas.rAreas[i].i4Right  != rNewParam.rFocusAreas.rAreas[i].i4Right) +
                (m_rParam.rFocusAreas.rAreas[i].i4Top    != rNewParam.rFocusAreas.rAreas[i].i4Top) +
                (m_rParam.rFocusAreas.rAreas[i].i4Bottom != rNewParam.rFocusAreas.rAreas[i].i4Bottom);
            focusArea[i].i4Left   = rNewParam.rFocusAreas.rAreas[i].i4Left; 
            focusArea[i].i4Top    = rNewParam.rFocusAreas.rAreas[i].i4Top; 
            focusArea[i].i4Right  = rNewParam.rFocusAreas.rAreas[i].i4Right; 
            focusArea[i].i4Bottom = rNewParam.rFocusAreas.rAreas[i].i4Bottom; 
        }
        if (u4Diff != 0)
        {
            setFocusAreas(rNewParam.rFocusAreas.u4Count, focusArea);
        }
    }
    //update AE area 
    if (m_max_metering_areas > 0)
    {
        UINT32 u4Diff = 0;
        AREA_T meteringArea[MAX_METERING_AREAS]; 
        for (MUINT32 i = 0; i < rNewParam.rMeteringAreas.u4Count; i++)
        {
            u4Diff += (UINT32)
                (m_rParam.rMeteringAreas.rAreas[i].i4Left   != rNewParam.rMeteringAreas.rAreas[i].i4Left) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Right  != rNewParam.rMeteringAreas.rAreas[i].i4Right) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Top    != rNewParam.rMeteringAreas.rAreas[i].i4Top) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Bottom != rNewParam.rMeteringAreas.rAreas[i].i4Bottom);
            meteringArea[i].i4Left   = rNewParam.rMeteringAreas.rAreas[i].i4Left; 
            meteringArea[i].i4Top    = rNewParam.rMeteringAreas.rAreas[i].i4Top; 
            meteringArea[i].i4Right  = rNewParam.rMeteringAreas.rAreas[i].i4Right; 
            meteringArea[i].i4Bottom = rNewParam.rMeteringAreas.rAreas[i].i4Bottom; 
        }
        if (u4Diff != 0)
        {
            setMeteringAreas(rNewParam.rMeteringAreas.u4Count, meteringArea); 
        }
    }
    
    m_rParam = rNewParam;

    MY_LOG("[%s()] - \n", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::getSupportedParams(FeatureParam_T &rFeatureParam) 
{	
    MINT32 ae_lock=0,awb_lock=0;
    MINT32 max_focus=0,max_meter=0;
    
    MY_LOG("[%s()] \n", __FUNCTION__);
    
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AE_AWB_LOCK,(int)&ae_lock,(int)&awb_lock,0);
    bAELockSupp = ae_lock==1?1:0;
    bAWBLockSupp = awb_lock==1?1:0;
    rFeatureParam.bExposureLockSupported = bAELockSupp;
    rFeatureParam.bAutoWhiteBalanceLockSupported = bAWBLockSupp;
    MY_LOG("AE_sup(%d),AWB_sub(%d) \n",bAELockSupp,bAWBLockSupp);
    
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,(int)&max_focus,0,0);
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,(int)&max_meter,0,0);
    rFeatureParam.u4MaxMeterAreaNum = max_meter>=1?1:0;
    rFeatureParam.u4MaxFocusAreaNum = max_focus>=1?1:0;    
    m_max_metering_areas = max_meter;
    m_max_af_areas = max_focus;
    MY_LOG("FOCUS_max(%d),METER_max(%d) \n",max_focus,max_meter);
    
    //rFeatureParam.i4MaxLensPos = AfMgr::getInstance().getMaxLensPos();
    //rFeatureParam.i4MinLensPos = AfMgr::getInstance().getMinLensPos();
    rFeatureParam.i4AFBestPos = 0;
    rFeatureParam.i8BSSVlu = 0;
    rFeatureParam.u4FocusLength_100x = 350;
    
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::autoFocus()
{
    MY_LOG("[%s()]dummylens(%d) \n", __FUNCTION__, m_bIsdummylens);
   
    Mutex::Autolock lock(m_LockAF);
    
    if (m_bIsdummylens == 1 || m_max_af_areas == 0)
    {
        //add for cts
        m_i4AutoFocus = E_YUV_SAF_FOCUSING;
        MY_LOG("[%s] AF Not Support\n", __FUNCTION__);
        return MTRUE;
    }

    if ((m_rParam.u4AfMode != AF_MODE_AFC) && (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO))
    {   
        m_fgAfTrig = MTRUE;
        MY_LOG("[%s] Do SAF CMD\n", __FUNCTION__);
    }
    else
    {
        m_i4AutoFocus = E_YUV_SAF_INCAF;
        MY_LOG("[%s] called in AF mode(%d)", __FUNCTION__, m_rParam.u4AfMode);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::cancelAutoFocus()
{
    MY_LOG("[%s()] \n", __FUNCTION__);
    if (m_bIsdummylens == 1){return MTRUE;}

#if 0
    switch (m_rParam.u4AfMode)
    {
    case AF_MODE_INFINITY:
        //m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
        //break;
    case AF_MODE_AFC:
    case AF_MODE_AFC_VIDEO:
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
        break;
    default:
    case AF_MODE_AFS:
        MY_LOG("Do nothing, u4AfMode = %d\n", m_rParam.u4AfMode);
        break;
    }
#else
    if (m_rParam.u4AfMode != AF_MODE_AFS && m_rParam.u4ShotMode != eShotMode_Autorama)
    {
        m_rParam.u4AfMode = AF_MODE_INFINITY;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
    }
    m_i4AutoFocus = E_YUV_SAF_DONE;
#endif

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
//    ERROR_CHECK(AeMgr::getInstance().setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height))

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
    MY_LOG("[%s()] \n", __FUNCTION__);
    
    SENSOR_EXIF_INFO_STRUCT mSensorInfo;
    EXIF_INFO_T rEXIFInfo;
    memset(&rEXIFInfo, 0, sizeof(EXIF_INFO_T));
    memset(&mSensorInfo, 0, sizeof(SENSOR_EXIF_INFO_STRUCT));
    
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_EXIF_INFO,(int)&mSensorInfo,0,0);             
    
    MY_LOG("FNumber=%d, AEISOSpeed=%d, AWBMode=%d, CapExposureTime=%d, FlashLightTimeus=%d, RealISOValue=%d\n", 
           mSensorInfo.FNumber, m_rParam.u4IsoSpeedMode, m_rParam.u4AwbMode, 
           mSensorInfo.CapExposureTime, m_bExifFlashOn, mSensorInfo.RealISOValue);
    
    rEXIFInfo.u4FNumber = mSensorInfo.FNumber>0 ? mSensorInfo.FNumber : 28;
    rEXIFInfo.u4FocalLength = 350;
    rEXIFInfo.u4SceneMode = m_rParam.u4SceneMode;
    rEXIFInfo.u4AWBMode = m_rParam.u4AwbMode;
    rEXIFInfo.u4CapExposureTime = mSensorInfo.CapExposureTime>0? mSensorInfo.CapExposureTime : 0;
    rEXIFInfo.u4FlashLightTimeus = m_bExifFlashOn;
    rEXIFInfo.u4AEISOSpeed = m_rParam.u4IsoSpeedMode;
    rEXIFInfo.u4RealISOValue = mapEnumToISO(mSensorInfo.RealISOValue);
    rEXIFInfo.i4AEExpBias = m_rParam.i4ExpIndex;
    
    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);
    
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setDebugInfo(IBaseCamExif *pIBaseCamExif) const
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getDelayFrame(EQueryType_T const eQueryType) const
{
    MUINT32 ret = 0;
    SENSOR_DELAY_INFO_STRUCT pDelay;
	   
    MY_LOG("[%s()] \n", __FUNCTION__);

    memset(&pDelay,0x0,sizeof(SENSOR_DELAY_INFO_STRUCT));
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_DELAY_INFO,(int)&pDelay,0,0);
    MY_LOG("Init:%d,effect:%d,awb:%d,af:%d \n",pDelay.InitDelay,pDelay.EffectDelay,pDelay.AwbDelay,pDelay.AFSwitchDelayFrame);

    switch (eQueryType)
    {
        case EQueryType_Init:
        {
            ret = (pDelay.InitDelay>0 && pDelay.InitDelay<5)?pDelay.InitDelay:0;
            return ret;
        }
        case EQueryType_Effect:
        {
             ret = (pDelay.EffectDelay>0 && pDelay.EffectDelay<5)?pDelay.EffectDelay:0;
             return ret;
        }
        case EQueryType_AWB:
        {
            ret = (pDelay.AwbDelay>0 && pDelay.AwbDelay<5)?pDelay.AwbDelay:0;
            return ret;
        }
        case EQueryType_AF:
        {
            ret = pDelay.AFSwitchDelayFrame;
            ret = ret < 1200 ? ret : 0;
            return ret;
        }
        default:
            return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setIspProfile(EIspProfile_T const eIspProfile)
{
    //ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(eIspProfile))
    //ERROR_CHECK(IspTuningMgr::getInstance().validate())

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::doAFUpdate(void)
{
    MINT32 af_status;

    Mutex::Autolock lock(m_LockAF);

    MY_LOG_IF(_bEnableMyLog, "[%s] +\n", __FUNCTION__);

    if (0 == m_bIsdummylens && m_max_af_areas > 0)
    {
        if (m_fgAfTrig)
        {
            m_fgAfTrig = 0;
            MY_LOG("[%s] Trigger AF Start.", __FUNCTION__);
            setAFLampOnOff(MTRUE);
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AF_WINDOW,(int)m_AFzone,0,0);     
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,0,0,0); 
            m_i4AutoFocus = E_YUV_SAF_FOCUSING;
            m_i4AutoFocusTimeout = 30;
        }
    
        if (m_i4AutoFocus == E_YUV_SAF_FOCUSING)
        {
            af_status = isFocused();

            if (af_status == SENSOR_AF_FOCUSED)
            {
                MY_LOG("[%s] SAF(SENSOR_AF_FOCUSED)\n", __FUNCTION__);
                m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
                m_i4AutoFocus = E_YUV_SAF_DONE;
                setAFLampOnOff(MFALSE);
            }
            else if (m_i4AutoFocusTimeout == 0)
            {
                MY_LOG("[%s] SAF(TimeOut)\n", __FUNCTION__);
                m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
                m_i4AutoFocus = E_YUV_SAF_DONE;
                setAFLampOnOff(MFALSE);
            }

            m_i4AutoFocusTimeout = m_i4AutoFocusTimeout > 0 ? m_i4AutoFocusTimeout - 1 : 0;
        }
        else if (m_i4AutoFocus == E_YUV_SAF_INCAF)
        {
            MY_LOG("[%s] autofocus callback in conti mode", __FUNCTION__);
            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, 0, 0, 0);
            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
            m_i4AutoFocus = E_YUV_SAF_DONE;
        }
        else
        {
            if (m_rParam.u4AfMode == AF_MODE_AFC || m_rParam.u4AfMode == AF_MODE_AFC_VIDEO)
            {                    
                if (m_i4AFSwitchCtrl > 0)
                {
                    m_i4AFSwitchCtrl--;
                }
                else if (m_i4AFSwitchCtrl == 0)
                {
                    MY_LOG("[%s] Send CAF CMD\n", __FUNCTION__);
                    m_i4FDFrmCnt = 0;
                    resetAFAEWindow();
                    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CONSTANT_AF,0,0,0);
                    m_i4AFSwitchCtrl = -1;
                    m_i4PreAfStatus = SENSOR_AF_STATUS_MAX;
                }
                else //(m_i4AFSwitchCtrl == -1)
                {
                    af_status = isFocused();

                    if (m_i4PreAfStatus != af_status)
                    {
                        if (af_status == SENSOR_AF_FOCUSED)
                        {
                            MY_LOG("[%s] CAF(SENSOR_AF_FOCUSED)\n", __FUNCTION__);
                            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, 0, 0, 0);
                            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
                        }
                        else
                        {
                            MY_LOG("[%s] CAF(%d)\n", __FUNCTION__, af_status);
                            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, (af_status == SENSOR_AF_FOCUSING), 0, 0);
                            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
                        }
                    }
                    
                    m_i4PreAfStatus = af_status;

                    if (m_i4WinState == 2)
                    {
                        // reset window when FD off.
                        MY_LOG("[%s] Leave FD and reset window\n", __FUNCTION__);
                        m_i4WinState = 0;
                        resetAFAEWindow();
                    }
                }
            }
        }
    }
    else
    {
        if (m_i4AutoFocus)
        {
            MY_LOG("[%s] AF not support\n", __FUNCTION__);
            m_i4AutoFocus = E_YUV_SAF_DONE;
            m_pAFYuvCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
        }
    }
    
    MY_LOG_IF(_bEnableMyLog, "[%s] -\n", __FUNCTION__);

    return 0;
}

MRESULT Hal3AYuv::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;

    if (a_bEnable)
    {
        if (m_bAFThreadLoop == 0)
        {
            m_pIspDrv = IspDrv::createInstance();

            if (!m_pIspDrv)
            {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (m_pIspDrv->init() < 0)
            {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }

            // create AF thread
            MY_LOG("[AFThread] Create");
            m_bAFThreadLoop = 1;
            ::sem_init(&m_semAFThreadStart, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_AF_THREAD};
//            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, 0};
            pthread_create(&m_AFThread, &attr, AFThreadFunc, this);
        }
    }
    else
    {
        if (m_bAFThreadLoop == 1)
        {
            if (m_pIspDrv)
            {
                m_pIspDrv->uninit();
                m_pIspDrv = NULL;
            }
            m_bAFThreadLoop = 0;
            ::sem_post(&m_semAFThreadStart);

            pthread_join(m_AFThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

    return ret;

}

MVOID * Hal3AYuv::AFThreadFunc(void *arg)
{
    MY_LOG("[%s] tid: %d \n", __FUNCTION__, gettid());
    ::prctl(PR_SET_NAME,"Cam@3A-AF", 0, 0, 0);

    Hal3AYuv* p3Ayuv = reinterpret_cast<Hal3AYuv*>(arg);

    if (!p3Ayuv->m_pIspDrv)
    {
        MY_LOG("[%s] m_pIspDrv null\n", __FUNCTION__);
        return NULL;
    }

    // wait AFO done
    //ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    //WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    //WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    //WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_AF_DON_ST;
    //WaitIrq.Timeout = 200; // 200 msec

    while (p3Ayuv->m_bAFThreadLoop)
    {
        ::sem_wait(&p3Ayuv->m_semAFThreadStart);   
        //if (p3Ayuv->m_pIspDrv->waitIrq(WaitIrq) >= 0) // success
        {  
            p3Ayuv->doAFUpdate();
        }
        //else
        //{
        //    MY_LOG("[%s] AF irq timeout\n", __FUNCTION__);
        //}
    }

    MY_LOG("[%s] End \n", __FUNCTION__);

    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setCallbacks(I3ACallBack* cb)
{
	MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);
    m_pAFYuvCallBack=cb;
	
    return MTRUE;
}

//******************************************************************************
// Map AE exposure to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapAEToEnum(MINT32 mval,MFLOAT mstep)
{
    MINT32 pEv,ret;  

    pEv = 100 * mval * mstep;
    
    if     (pEv <-250) { ret = AE_EV_COMP_n30;}  // EV compensate -3.0
    else if(pEv <-200) { ret = AE_EV_COMP_n25;}  // EV compensate -2.5
    else if(pEv <-170) { ret = AE_EV_COMP_n20;}  // EV compensate -2.0
    else if(pEv <-160) { ret = AE_EV_COMP_n17;}  // EV compensate -1.7
    else if(pEv <-140) { ret = AE_EV_COMP_n15;}  // EV compensate -1.5
    else if(pEv <-130) { ret = AE_EV_COMP_n13;}  // EV compensate -1.3    
    else if(pEv < -90) { ret = AE_EV_COMP_n10;}  // EV compensate -1.0
    else if(pEv < -60) { ret = AE_EV_COMP_n07;}  // EV compensate -0.7
    else if(pEv < -40) { ret = AE_EV_COMP_n05;}  // EV compensate -0.5
    else if(pEv < -10) { ret = AE_EV_COMP_n03;}  // EV compensate -0.3    
    else if(pEv ==  0) { ret = AE_EV_COMP_00; }  // EV compensate -2.5
    else if(pEv <  40) { ret = AE_EV_COMP_03; }  // EV compensate  0.3
    else if(pEv <  60) { ret = AE_EV_COMP_05; }  // EV compensate  0.5
    else if(pEv <  90) { ret = AE_EV_COMP_07; }  // EV compensate  0.7
    else if(pEv < 110) { ret = AE_EV_COMP_10; }  // EV compensate  1.0
    else if(pEv < 140) { ret = AE_EV_COMP_13; }  // EV compensate  1.3
    else if(pEv < 160) { ret = AE_EV_COMP_15; }  // EV compensate  1.5
    else if(pEv < 180) { ret = AE_EV_COMP_17; }  // EV compensate  1.7    
    else if(pEv < 210) { ret = AE_EV_COMP_20; }  // EV compensate  2.0
    else if(pEv < 260) { ret = AE_EV_COMP_25; }  // EV compensate  2.5
    else if(pEv < 310) { ret = AE_EV_COMP_30; }  // EV compensate  3.0
    else               { ret = AE_EV_COMP_00;}
    
    MY_LOG("[%s()]EV:(%d),Ret:(%d)\n", __FUNCTION__, pEv,ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapISOToEnum(MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret;  
    
    switch(u4NewAEISOSpeed){
        case 0:
            ret = AE_ISO_AUTO;
            break;
        case 100:
            ret = AE_ISO_100;
            break;
        case 200:
            ret = AE_ISO_200;
            break;
        case 400:
            ret = AE_ISO_400;
            break;
        case 800:
             ret = AE_ISO_800;
           break;
        case 1600:
            ret = AE_ISO_1600;
           break;
        default:
            MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);            
            ret = AE_ISO_AUTO;
            break;
    }
    MY_LOG("[%s()]ISOVal:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEISOSpeed, ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapEnumToISO(MUINT32 u4NewAEIsoEnum) const
{
    MINT32 ret;  
    
    switch(u4NewAEIsoEnum){
        case AE_ISO_AUTO:
            ret = 100;
            break;
        case AE_ISO_100:
            ret = 100;
            break;
        case AE_ISO_200:
            ret = 200;
            break;
        case AE_ISO_400:
            ret = 400;
            break;
        case AE_ISO_800:
             ret = 800;
           break;
        case AE_ISO_1600:
            ret = 1600;
           break;
        default:
            ret = 100;
            break;
    }
    MY_LOG("[%s()]ISOEnum:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEIsoEnum, ret);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo)
{
    CaptureParam_T rCaptureInfo;
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0; 
#if 0    
    if (m_rParam.u4CamMode == eAppMode_FactoryMode){
         yuvCmd = YUV_AUTOTEST_GET_SHUTTER_RANGE;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvShutterRange = yuvParam;
         
         yuvCmd = YUV_AUTOTEST_GET_SHADDING;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvShading = yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_GAMMA;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvGamma= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_SHUTTER;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvShutter= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_GAIN;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvGain= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_AE;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
         a_rCaptureInfo.u4YuvAE= yuvParam;

         MY_LOG("[Factory]getCaptureParams shading(%d),Gamma(%d),AE(%d),Shutter(%d),Gain(%d) \n",
               a_rCaptureInfo.u4YuvShading,a_rCaptureInfo.u4YuvGamma,a_rCaptureInfo.u4YuvAE,
               a_rCaptureInfo.u4YuvGain,a_rCaptureInfo.u4YuvShutter);
    }
    else
#endif
    {
        if (i4EVidx == 0 && index == 0)
        {
            a_rCaptureInfo.u4YuvAE = 0;
        }
        else if (i4EVidx == 0)
        {
            MUINT32 u4EVIndex[3] = {-2, 0, 2};
            index = ((index > 2) ? 2 : (index < 0 ? 0 : index));
            a_rCaptureInfo.u4YuvAE = u4EVIndex[index];
        }
        else
        {
            a_rCaptureInfo.u4YuvAE = i4EVidx;
        }
        MY_LOG("[%s] index(%d) i4EVidx(%d) u4YuvAE(%d)", __FUNCTION__, index, i4EVidx, a_rCaptureInfo.u4YuvAE);
    }
    
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    CaptureParam_T rCaptureInfo;
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0; 

    MY_LOG("[%s]\n", __FUNCTION__);

#if 0
    if (m_rParam.u4CamMode == eAppMode_FactoryMode)
    {
        MY_LOG("[Factory]updateCaptureParams shading(%d),Gamma(%d),AE(%d),Shutter(%d),Gain(%d) \n",
              a_rCaptureInfo.u4YuvShading,a_rCaptureInfo.u4YuvGamma,a_rCaptureInfo.u4YuvAE,
              a_rCaptureInfo.u4YuvGain,a_rCaptureInfo.u4YuvShutter);
        
        yuvCmd = YUV_AUTOTEST_SET_SHADDING;
        yuvParam = a_rCaptureInfo.u4YuvShading;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
        
        yuvCmd = YUV_AUTOTEST_SET_GAMMA;
        yuvParam=a_rCaptureInfo.u4YuvGamma;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
        
        yuvCmd = YUV_AUTOTEST_SET_AE;
        yuvParam=a_rCaptureInfo.u4YuvAE;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
        
        yuvCmd = YUV_AUTOTEST_SET_SHUTTER;
        yuvParam=a_rCaptureInfo.u4YuvShutter;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
        
        yuvCmd = YUV_AUTOTEST_SET_GAIN;
        yuvParam=a_rCaptureInfo.u4YuvGain;
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);             
    }
    else
#endif
    {
        MY_LOG("[%s FID_AE_EV], Idx:(%d)->(%d),Step:(%f)\n", __FUNCTION__, m_rParam.i4ExpIndex, a_rCaptureInfo.u4YuvAE, m_rParam.fExpCompStep);
        yuvCmd = FID_AE_EV;
        yuvParam = mapAEToEnum(a_rCaptureInfo.u4YuvAE, m_rParam.fExpCompStep);        
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_FEATURE_CMD,(int)&yuvCmd,(int)&yuvParam,0);
    }
	
    return S_3A_OK;
}

MINT32 Hal3AYuv::setFlashMode(MINT32 mode)
{
    MY_LOG("[%s] mode=%d\n", __FUNCTION__, mode);

    if (m_pStrobeDrvObj)
    {
        if (mode<LIB3A_FLASH_MODE_MIN || mode>LIB3A_FLASH_MODE_MAX)
        {
            //return FL_ERR_FlashModeNotSupport;
            MY_LOG("FL_ERR_FlashModeNotSupport\n");
        }
        else
        {
            if (m_rParam.u4StrobeMode == FLASHLIGHT_TORCH && mode != m_rParam.u4StrobeMode) //prviouw mode is torch. and change to another mode.
            {
                m_pStrobeDrvObj->setOnOff(0);
                MY_LOG("FLASHLIGHT_TORCH OFF\n");
            }

            if (mode == FLASHLIGHT_TORCH)
            {
                m_pStrobeDrvObj->setDuty(m_strobeWidth);
                m_pStrobeDrvObj->setStep(NSCamCustom::custom_GetYuvFlashlightStep());
                m_pStrobeDrvObj->setTimeOutTime(0);
                m_pStrobeDrvObj->setOnOff(0);
                m_pStrobeDrvObj->setOnOff(1);
                MY_LOG("FLASHLIGHT_TORCH ON\n");
            }
            else if(mode==FLASHLIGHT_FORCE_OFF)
            {
                m_pStrobeDrvObj->setTimeOutTime(1000);
                m_pStrobeDrvObj->setOnOff(0);
                MY_LOG("FLASHLIGHT_FORCE_OFF\n");
            }
        }
    }
    else
    {
        MY_LOG("No Strobe!\n");
    }

    return S_3A_OK;
}

MINT32 Hal3AYuv::getFlashFrameNumBeforeCapFrame()
{
    MINT32 i4FrmNum = 1;
    if (m_pStrobeDrvObj && m_bFlashActive == TRUE)
    {
        i4FrmNum = 3;
    }
    MY_LOG("[%s]: i4FrmNum(%d)\n", __FUNCTION__, i4FrmNum);
    return i4FrmNum;
}

MINT32 Hal3AYuv::updateAeFlashCaptureParams()
{
    MINT32 mflashEng=0;
    MINT32 mshutter=0,mcfg_gain=0,mgain=0;
    MINT32 i4StrobeWidth = 0;
    MINT32 i4HighcurrentTimeout = 0;
	
    if (m_pStrobeDrvObj && m_strobeWidth >0 && m_bFlashActive == TRUE)
    {
    	mflashEng = NSCamCustom::custom_GetFlashlightGain10X(); //20;
        MY_LOG("flashEng:%d,current_BV:%f,PreflashBV:%f,pre_shutter:%d,pre_gain:%d\n",mflashEng,m_strobecurrent_BV,m_strobePreflashBV,pre_shutter,pre_gain);
        convertFlashExpPara(
            mflashEng, m_AEFlashlightInfo.GAIN_BASE,
            m_strobecurrent_BV*1024, m_strobePreflashBV*1024,
            pre_shutter, pre_gain, 1024,
            mshutter, mgain, mcfg_gain);
            
        m_strobecurrent_BV = 0.0;
        i4StrobeWidth = m_strobeWidth;
        if (mflashEng > 10 && m_rParam.u4ShotMode != CAPTURE_MODE_BURST_SHOT)
        {
            MY_LOG("open high current mode\n");
            // strobe led driver should implement 0xff as 2x.
            i4StrobeWidth = NSCamCustom::custom_GetYuvFlashlightHighCurrentDuty();
            i4HighcurrentTimeout = NSCamCustom::custom_GetYuvFlashlightHighCurrentTimeout();
        
            MY_LOG("mshutter(%d), mgain(%d) \n",mshutter,mgain);
            // set to sensor
            setEShutterParam(mshutter,mgain);
        }       

        if (m_pStrobeDrvObj->setTimeOutTime(i4HighcurrentTimeout) == MHAL_NO_ERROR)
        {
            MY_LOG("setTimeOutTime: %d ms\n", i4HighcurrentTimeout);    
        }
        if (m_pStrobeDrvObj->setDuty(i4StrobeWidth) == MHAL_NO_ERROR)
        {
            MY_LOG("setLevel: %d\n",i4StrobeWidth);
        }

        if (m_rParam.u4ShotMode == CAPTURE_MODE_BURST_SHOT || mflashEng <= 10)
        {
            if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
            {
                MY_LOG("setFire ON\n");
            }
        }
        else
        {
            m_TrigFlashFire = MTRUE;
        }
    }
	
    return S_3A_OK;
}

MVOID Hal3AYuv::onFireCapFlashIfNeeded()
{
    if (m_TrigFlashFire == MTRUE)
    {
        MY_LOG("[%s]: shotMode(%d)\n", __FUNCTION__, m_rParam.u4ShotMode);
        m_TrigFlashFire = MFALSE;
        m_pStrobeDrvObj->setOnOff(1);            
    }
}

MINT32 Hal3AYuv::setAFMode(MINT32 AFMode)
{
    switch (AFMode) {
        case AF_MODE_AFS:
        case AF_MODE_INFINITY:
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0); 
            break;
        case AF_MODE_AFC:
        case AF_MODE_AFC_VIDEO:
            m_i4AFSwitchCtrl = getDelayFrame(EQueryType_AF);
            //m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CONSTANT_AF,0,0,0); 
            break;       
        default:     	
            break;
    }
    return S_3A_OK;	
}

MINT32 Hal3AYuv::isFocused()
{
    MINT32 err = MHAL_NO_ERROR;
    MINT32 focus_status = 0xffffffff;
    
    if(m_bIsdummylens)
    {
        return SENSOR_AF_FOCUSED;
    } 

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AF_STATUS,(int)&focus_status,0,0);     

    if (SENSOR_AF_SCENE_DETECTING == focus_status)
        focus_status = SENSOR_AF_FOCUSING;
#if 0
    switch (m_rParam.u4AfMode) {
        case AF_MODE_AFS:
        case AF_MODE_AFC:            
            m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_AF_STATUS,(int)&focus_status,0,0); 
            break;
        case AF_MODE_INFINITY: 
            break;
        default:
            break;
    }
#endif
    
//    if(SENSOR_AF_FOCUSED == focus_status) {return TRUE;}
//    MY_LOG("[AF]isFocused status=0x%x\n",focus_status);

    return focus_status;
}




MINT32 Hal3AYuv::clamp(MINT32 x, MINT32 min, MINT32 max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

MVOID Hal3AYuv::mapAeraToZone(
    AREA_T *p_area, MINT32 areaW, 
    MINT32 areaH, MINT32* p_zone, 
    MINT32 zoneW, MINT32 zoneH)
{

    MINT32 left, top, right, bottom;
    
    p_area->i4Left = clamp(p_area->i4Left, 0, areaW-1);
    p_area->i4Right = clamp(p_area->i4Right, 0, areaW-1);
    p_area->i4Top = clamp(p_area->i4Top, 0, areaH-1);
    p_area->i4Bottom = clamp(p_area->i4Bottom, 0, areaH-1);

    left     = p_area->i4Left * zoneW  / areaW;
    right    = p_area->i4Right * zoneW  / areaW;
    top      = p_area->i4Top * zoneH / areaH;
    bottom   = p_area->i4Bottom * zoneH / areaH;

    *p_zone = clamp(left, 0, zoneW-1);
    *(p_zone+1) = clamp(top, 0, zoneH-1);
    *(p_zone+2) = clamp(right, 0, zoneW-1);
    *(p_zone+3) = clamp(bottom, 0, zoneH-1);
    *(p_zone+4) = zoneW;
    *(p_zone+5) = zoneH;

    MY_LOG("[AF]maping area [L]%d,[U]%d,[R]%d,[B]%d [width]%d [height]%d\n to [L]%d,[U]%d,[R]%d,[B]%d [width]%d [height]%d\n",
        p_area->i4Left, p_area->i4Top, p_area->i4Right, p_area->i4Bottom, areaW, areaH, *p_zone, 
        *(p_zone+1), *(p_zone+2), *(p_zone+3), *(p_zone+4), *(p_zone+5));
}

MVOID Hal3AYuv::setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea)
{
	
   	MY_LOG("[AF][%s()] \n", __FUNCTION__);

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_af_areas))
    {
        return ;
    }
    else  // spot or matrix meter
    {
        m_sAFAREA[0] = *a_psFocusArea;

        m_sAFAREA[0].i4Left  = clamp(m_sAFAREA[0].i4Left + 1000, 0, 1999);  
        m_sAFAREA[0].i4Right  = clamp(m_sAFAREA[0].i4Right + 1000, 0, 1999); 
        m_sAFAREA[0].i4Top  = clamp(m_sAFAREA[0].i4Top + 1000, 0, 1999);
        m_sAFAREA[0].i4Bottom  = clamp(m_sAFAREA[0].i4Bottom + 1000, 0, 1999);
        MY_LOG("[AF] setFocusAreas\n");
        mapAeraToZone(&m_sAFAREA[0], 2000, 2000, &m_AFzone[0], m_imageXS, m_imageYS);       
    }
}

MVOID Hal3AYuv::getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psFocusArea = &m_sAFAREA[0];
}

MVOID Hal3AYuv::getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psAEArea = &m_sAEAREA[0];
}

MVOID Hal3AYuv::setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* zone_addr = (MUINT32*)&m_AEzone[0];                  

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_metering_areas))
    {
        return;        
    }
    else  // spot or matrix meter
    {
        m_sAEAREA[0] = *a_psAEArea;

        m_sAEAREA[0].i4Left = clamp(m_sAEAREA[0].i4Left + 1000, 0, 1999);  
        m_sAEAREA[0].i4Right = clamp(m_sAEAREA[0].i4Right + 1000, 0, 1999); 
        m_sAEAREA[0].i4Top = clamp(m_sAEAREA[0].i4Top + 1000, 0, 1999);
        m_sAEAREA[0].i4Bottom = clamp(m_sAEAREA[0].i4Bottom + 1000, 0, 1999);
        MY_LOG("[AF]touch auto exposure setMeteringAreas\n");
        mapAeraToZone(&m_sAEAREA[0], 2000, 2000, &m_AEzone[0], m_imageXS, m_imageYS); 
        MY_LOG("[AF]touch auto exposure setMeteringAreas, zone_addr=0x%x\n", zone_addr);
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AE_WINDOW,(int)zone_addr,0,0); 
    }
}

MBOOL Hal3AYuv::setFDInfo(MVOID* a_sFaces)
{
    MINT32 i4Cnt;
    AREA_T rFDArea;
    MtkCameraFaceMetadata *pFaces;

    Mutex::Autolock lock(m_LockAF);

    pFaces = (MtkCameraFaceMetadata *)a_sFaces;

    if (pFaces == NULL)
    {
        MY_LOG("[%s] Leave\n", __FUNCTION__);
        m_i4WinState = 2;
        return TRUE;
    }

    i4Cnt = pFaces->number_of_faces;

    if (i4Cnt)
    {
        m_i4FDFrmCnt -= (m_i4FDFrmCnt > 0 ? 1 : 0);
        
        rFDArea.i4Left   = pFaces->faces->rect[0];
        rFDArea.i4Top    = pFaces->faces->rect[1];
        rFDArea.i4Right  = pFaces->faces->rect[2];
        rFDArea.i4Bottom = pFaces->faces->rect[3];

        if (rFDArea.i4Right == rFDArea.i4Left || rFDArea.i4Bottom == rFDArea.i4Top)
        {
            i4Cnt = 0;
        }
    }
    else
    {
        m_i4FDFrmCnt += (m_i4FDFrmCnt < 8 ? 1 : 0);
    }

    MY_LOG("[%s] m_i4FDFrmCnt=%d, score=%d\n", __FUNCTION__, m_i4FDFrmCnt, pFaces->faces->score);

    if (m_i4FDFrmCnt < 3)
    {
        m_i4FDApplyCnt -= (m_i4FDApplyCnt > 0 ? 1 : 0);
        if (m_i4FDApplyCnt == 0)
        {
            MY_LOG("[%s] number_of_faces = %d, (%d,%d,%d,%d)\n",
                __FUNCTION__, i4Cnt, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);

            m_i4WinState = 1;
            m_i4FDApplyCnt = 3;
            if (i4Cnt != 0)
            {
                if (m_max_af_areas > 0)
                {
                    setFocusAreas(i4Cnt, &rFDArea);
                    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AF_WINDOW,(int)m_AFzone,0,0); 
                }
                if (m_max_metering_areas > 0)
                {
                    setMeteringAreas(i4Cnt, &rFDArea);
                }
            }
        }
    }
    else
    {
        m_i4FDApplyCnt = 3;
        if (m_i4WinState == 1)
        {
            m_i4WinState = 0;
            resetAFAEWindow();
        }
    }
    
    return MTRUE;
}

MBOOL Hal3AYuv::resetAFAEWindow()
{
    MINT32 ai4Zone[6];

    MY_LOG("[%s]\n", __FUNCTION__);
    
    // reset to center point
    ai4Zone[0] = ai4Zone[2] = m_imageXS/2;
    ai4Zone[1] = ai4Zone[3] = m_imageYS/2;
    ai4Zone[4] = m_imageXS;
    ai4Zone[5] = m_imageYS;

    if (m_max_af_areas > 0)
    {
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AF_WINDOW,(int)ai4Zone,0,0); 
    }

    if (m_max_metering_areas > 0)
    {
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AE_WINDOW,(int)ai4Zone,0,0); 
    }

    return MTRUE;
}

MINT32 Hal3AYuv::isNeedFiringFlash()
{
    MY_LOG("[%s]\n", __FUNCTION__);

    MBOOL bFlashOn;

    if (m_rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_OFF)
    {
        MY_LOG("[%s] FLASHLIGHT_FORCE_OFF\n", __FUNCTION__);
        bFlashOn = 0;
    }
    else if (m_rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_ON)
    {
        MY_LOG("[%s] FLASHLIGHT_FORCE_ON\n", __FUNCTION__);
        bFlashOn = 1;
    }
    else //auto
    {
        if (isAEFlashOn())
        {
            MY_LOG("[%s] isAEFlashOn ON\n", __FUNCTION__);
            bFlashOn = 1;
        }
        else
        {
            MY_LOG("[%s] isAEFlashOn OFF\n", __FUNCTION__);
            bFlashOn = 0;
        }
    }
    
#if defined(DUMMY_FLASHLIGHT)
    MY_LOG("[%s] DUMMY_FLASHLIGHT\n", __FUNCTION__);
    bFlashOn = 0;
#endif

    m_isFlashOnCapture = bFlashOn;

    return bFlashOn;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::isAEFlashOn()
{
    MINT32 rtn = 0;
    MUINT32 u4TrigFlashOn;

    m_strobeTrigerBV = calcBV();

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO,(int)&u4TrigFlashOn,0,0); 

    MY_LOG("m_aeFlashlightType=0x%x, m_aeStrobeMode=0x%x;BV_THRESHOLD=%f,m_strobeTrigerBV=%f,u4TrigFlashOn=%d\n",
        m_aeFlashlightType,
        m_rParam.u4StrobeMode,
        BV_THRESHOLD,
        m_strobeTrigerBV,
        u4TrigFlashOn);

    if (FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType ||
        FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType)
    {
        if ( (LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_FORCE_ON == m_rParam.u4StrobeMode )
        {
            rtn = 1;
        }
        else if ((LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_AUTO == m_rParam.u4StrobeMode && u4TrigFlashOn
                 /*(BV_THRESHOLD > m_strobeTrigerBV ) */)
        {
            rtn = 1;
        }
    }    
    return rtn;
}

MINT32 Hal3AYuv::setAFLampOnOff(MBOOL bOnOff)
{
    MINT32 i4Ret = S_3A_OK;
    MINT32 i4AfLampSupport = NSCamCustom::custom_GetYuvAfLampSupport();

    if (m_pStrobeDrvObj && i4AfLampSupport)
    {
        if (m_rParam.u4StrobeMode != FLASHLIGHT_TORCH && !isInVideo() /*m_rParam.u4CamMode != eAppMode_VideoMode*/)
        {
            MY_LOG("[%s] bOnOff(%d), StrobeMode(%d)\n", __FUNCTION__, bOnOff, m_rParam.u4StrobeMode);
            if (bOnOff)
            {
                MBOOL fgFlashOn = isAEFlashOn();
                if (fgFlashOn)
                {
                    //ON flashlight
                    if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setTimeOutTime: 0\n");
                    }
                    if (m_pStrobeDrvObj->setDuty(m_strobeWidth) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setLevel:%d\n", m_strobeWidth);
                    }
                    if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
                    {
                        MY_LOG("[%s] setFire ON\n", __FUNCTION__);
                    }
                }
                else
                {
                    MY_LOG("[%s] No need to turn on AF lamp.\n", __FUNCTION__);
                }
            }
            else
            {
                if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                {
                    MY_LOG("[%s] setFire OFF\n", __FUNCTION__);
                }
            }
        }
        else
        {
            MY_LOG("[%s] StrobeMode(%d), CamMode(%d), skip\n", __FUNCTION__,
                m_rParam.u4StrobeMode, m_rParam.u4CamMode);
        }
    }
    else
    {
        MY_LOG("[%s] strobe object(0x%08x), AfLampSupport(%d)\n", __FUNCTION__, m_pStrobeDrvObj, i4AfLampSupport);
        i4Ret = E_3A_NULL_OBJECT;
    }

    return i4Ret;
}

/*******************************************************************************
*
********************************************************************************/
double Hal3AYuv::calcBV()
{
    DOUBLE  AV=0, TV=0, SV=0, BV=0;
    MINT32 ISO =0;
    MINT32 u4MiniISOGain = 50;
    queryAEFlashlightInfoFromSensor();

    AV=AEFlashlightLog2((double)m_AEFlashlightInfo.u4Fno/10)*2; 
    TV=AEFlashlightLog2(1000000/((double)m_AEFlashlightInfo.Exposuretime)); 
    ISO=m_AEFlashlightInfo.Gain * u4MiniISOGain/ m_AEFlashlightInfo.GAIN_BASE;
    SV=AEFlashlightLog2(((double)ISO)/3.125);
   
    BV = AV + TV - SV ;

    MY_LOG("AV=%f, TV=%f,ISO=%d,SV=%f,BV=%f\n", AV, TV, ISO, SV, BV);

    return (BV);
}

/*******************************************************************************
*
********************************************************************************/
double Hal3AYuv::AEFlashlightLog2(double x)
{
     return log(x)/log((double)2);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::queryAEFlashlightInfoFromSensor()
{
    MINT32 err = MHAL_NO_ERROR;
    memset(&m_AEFlashlightInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));
	//fixme
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_GET_YUV_STROBE_INFO,(int)&m_AEFlashlightInfo,0,0); 
	
    if (m_AEFlashlightInfo.u4Fno == 0)
    {
        MY_ERR("query Fnumber fail, val: %d, set to 28\n", m_AEFlashlightInfo.u4Fno);
        m_AEFlashlightInfo.u4Fno = 28;
    }
    if (m_AEFlashlightInfo.Exposuretime > 1000000 || m_AEFlashlightInfo.Exposuretime == 0)
    {
        MY_ERR("query exp fail,val:%d,set to 1000\n",m_AEFlashlightInfo.Exposuretime);
        m_AEFlashlightInfo.Exposuretime=1000;
    }
    if (m_AEFlashlightInfo.GAIN_BASE < 50)
    {
        MY_ERR("query gain_base fail, val: %d, set to 50\n", m_AEFlashlightInfo.GAIN_BASE);
        m_AEFlashlightInfo.GAIN_BASE = 50;
    }
    if (/*m_AEFlashlightInfo.Gain > 2000 ||*/ m_AEFlashlightInfo.Gain == 0)
    {
        MY_ERR("query gain fail, val: %d, set to %d\n", m_AEFlashlightInfo.Gain, m_AEFlashlightInfo.GAIN_BASE);
        m_AEFlashlightInfo.Gain = m_AEFlashlightInfo.GAIN_BASE;
    }
    
    MY_LOG("u4Fno=%d, Exposuretime=%d, Gain=%d, GAIN_BASE=%d\n", 
        m_AEFlashlightInfo.u4Fno,
        m_AEFlashlightInfo.Exposuretime,
        m_AEFlashlightInfo.Gain,
        m_AEFlashlightInfo.GAIN_BASE);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::setEShutterParam(
       MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 err;

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0)) {
        MY_LOG("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    MY_LOG("[%s] ExpTime(%d us), SensorGain(%d)\n", __FUNCTION__, a_u4ExpTime, a_u4SensorGain);

    // exposure time in terms of 32us
    a_u4ExpTime = a_u4ExpTime >> 5;
    a_u4SensorGain = a_u4SensorGain << 4;

    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_SENSOR_EXP_LINE,(int)&a_u4ExpTime,0,0); 
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_SENSOR_GAIN,(int)&a_u4SensorGain,0,0); 

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID Hal3AYuv::endContinuousShotJobs()
{
    if (m_pStrobeDrvObj && m_bFlashActive == TRUE)
    {
        // force off for burst shot mode
        m_pStrobeDrvObj->setOnOff(0);
        m_bFlashActive = FALSE;
        MY_LOG("[%s]: setFire OFF\n", __FUNCTION__);
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
    Hal3A_HDROutputParam_T HDROutputParam;

    HDROutputParam.u4OutputFrameNum = 3;
    HDROutputParam.u4FinalGainDiff[0] = 4096;
    HDROutputParam.u4FinalGainDiff[1] = 256;
    HDROutputParam.u4TargetTone = 150;

    a_strHDROutputInfo = HDROutputParam;

    return S_3A_OK;
}

void Hal3AYuv::convertFlashExpPara(MINT32 flashEngRatio_x10, MINT32 minAfeGain_x1024, 
			 MINT32 bv0_x1024, MINT32 bv1_x1024,
			 MINT32  exp1, MINT32  afeGain1_x1024, MINT32  ispGain1_x1024, 
			 MINT32& exp2, MINT32& afeGain2_x1024, MINT32& ispGain2_x1024) const
{
	MY_LOG("convertFlashExpParaa ratio=%d minG=%d bv0=%d bv1=%d\n",flashEngRatio_x10, minAfeGain_x1024, bv0_x1024, bv1_x1024);
	MY_LOG("convertFlashExpParaa exp=%d afe=%d isp=%d\n",exp1, afeGain1_x1024, ispGain1_x1024);
	if(minAfeGain_x1024==0)
		minAfeGain_x1024=2048;
	double bv0;
	double bv1;	
	double engRatio;
	double delEv;
	double rat;
	bv0 = bv0_x1024/1024.0;
	bv1 = bv1_x1024/1024.0;
	engRatio = flashEngRatio_x10/10.0;	


	/*
	double m0;
	double m1;
	double rat2;
	m0 = pow(2, bv0);
	m1 = pow(2, bv1);
	double rr;
	rat2 = ( (m0+(m1-m0)*engRatio)/m1);
	*/

	if (bv1 < bv0)
	{
	    exp2 = exp1 / engRatio;
	    afeGain2_x1024 = afeGain1_x1024;
	    ispGain2_x1024 = 1024;
	    MY_LOG("[%s] bv1 < bv0!\n", __FUNCTION__);
            return;
	}

	rat = ((pow(2, bv1-bv0)-1)*engRatio +1)*pow(2, bv0-bv1);		

	double maxGainRatio=1;
	if(afeGain1_x1024>minAfeGain_x1024)
		maxGainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
	maxGainRatio *= ispGain1_x1024/1024.0; 

	MY_LOG("[%s] rat(%3.6f), maxGainRatio(%3.6f)\n", __FUNCTION__, rat, maxGainRatio);

	double gainRatio;
	double expRatio;
	if(rat>maxGainRatio)
	{		
		exp2 = exp1*(maxGainRatio/rat);
		afeGain2_x1024 = minAfeGain_x1024;
		ispGain2_x1024 = 1024;		
	}
	else
	{	
		gainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
		MY_LOG("[%s] rat(%3.6f), gainRatio(%3.6f)\n", __FUNCTION__, rat, gainRatio);
		if(rat > gainRatio)
		{
			exp2 = exp1;
			afeGain2_x1024 = minAfeGain_x1024;	
			ispGain2_x1024 = ispGain1_x1024/(rat/gainRatio);
		}
		else
		{
			exp2 = exp1;
			afeGain2_x1024 = afeGain1_x1024/rat;	
			ispGain2_x1024 = ispGain1_x1024;
		}		
	}
}

/* LUT for gain & dEv */
#define ASD_LOG2_LUT_RATIO_BASE 256
#define ASD_LOG2_LUT_NO 101
#define ASD_LOG2_LUT_CENTER 0
#define YUV_EVDELTA_THRESHOLD  10


const MINT32 ASD_LOG2_LUT_RATIO[ASD_LOG2_LUT_NO]={
256,/* 0 */
274, 294, 315, 338, 362, 388, 416, 446, 478, 512,/* 0.1~1.0 */
549, 588, 630, 676, 724, 776, 832, 891, 955, 1024,/* 1.1~2.0 */
1097, 1176, 1261, 1351, 1448, 1552, 1663, 1783, 1911, 2048,/* 2.1~3.0 */
2195, 2353, 2521, 2702, 2896, 3104, 3327, 3566, 3822, 4096,/* 3.1~4.0 */
4390, 4705, 5043, 5405, 5793, 6208, 6654, 7132, 7643, 8192,/* 4.1~5.0 */
8780, 9410, 10086, 10809, 11585, 12417, 13308, 14263, 15287, 16384,/* 5.1~6.0 */
17560, 18820, 20171, 21619, 23170, 24834, 26616, 28526, 30574, 32768,/* 6.1~7.0 */
35120, 37640, 40342, 43238, 46341, 49667, 53232, 57052, 61147, 65536,/* 7.1~8.0 */
70240, 75281, 80864, 86475, 92682, 99334, 106464, 114105, 122295, 131072,/* 8.1~9.0 */
140479, 150562, 161369, 172951, 185364, 198668, 212927, 228210, 244589, 262144/* 9.1~10.0 */
};

MINT32 Hal3AYuv::ASDLog2Func(MUINT32 numerator, MUINT32 denominator) const
{
    MUINT32 temp_p;
    MINT32 x;
    MUINT32 *p_LOG2_LUT_RATIO = (MUINT32*)(&ASD_LOG2_LUT_RATIO[0]);
    
    temp_p = numerator*p_LOG2_LUT_RATIO[ASD_LOG2_LUT_CENTER];

    if (temp_p>denominator*ASD_LOG2_LUT_RATIO_BASE)
    {
        for (x=ASD_LOG2_LUT_CENTER; x<ASD_LOG2_LUT_NO; x++)
        {
            temp_p = denominator*p_LOG2_LUT_RATIO[x];
            
            if (temp_p>=numerator*ASD_LOG2_LUT_RATIO_BASE)
            {
                if ((temp_p -numerator*ASD_LOG2_LUT_RATIO_BASE) 
                    > (numerator*ASD_LOG2_LUT_RATIO_BASE-denominator*p_LOG2_LUT_RATIO[x-1]))
                {
                    return x-1;
                }
                else
                {
                    return x;           
                }
            }
            else if (x==ASD_LOG2_LUT_NO-1)
            {
                return (ASD_LOG2_LUT_NO-1);
            }
        }
    }
    return ASD_LOG2_LUT_CENTER;
}


#define ASD_ABS(val) (((val) < 0) ? -(val) : (val))

void Hal3AYuv::calcASDEv(const SENSOR_AE_AWB_CUR_STRUCT& cur)
{
    MINT32 AeEv;
    MY_LOG("[%s] shutter=%d,gain=%d,", __FUNCTION__,
        cur.SensorAECur.AeCurShutter,cur.SensorAECur.AeCurGain);

    //m_i4AELv_x10
    if ((m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain)
        <= (cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain))
    {
        AeEv = 50;//0*80/IspSensorAeAwbRef.SensorLV05LV13EVRef+50;        
    }
    else
    {
        AeEv = ASDLog2Func(m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain,
                           cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain);
        if (AeEv == 0)
        {
            AeEv = 50;
        }
        else
        {
            if (m_AsdRef.SensorLV05LV13EVRef)
            {
                AeEv = AeEv * 80 / m_AsdRef.SensorLV05LV13EVRef + 50;
            }
            else
            {
                AeEv = 150;
            }
        }
    }
    
    if (AeEv > 150) // EV range from 50 ~150
    {
        AeEv = 150;
    }
    
    if (ASD_ABS(m_i4AELv_x10-AeEv) <= YUV_EVDELTA_THRESHOLD) 
    {
        m_bAEStable = TRUE;    
    }
    else
    {
        m_bAEStable = FALSE;        
    }
    
    m_i4AELv_x10 = AeEv;         
	
    MY_LOG("[%s] m_i4AELv_x10=%d", __FUNCTION__, m_i4AELv_x10);
} 

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::getASDInfo(ASDInfo_T &a_ASDInfo)
{
    MINT32 err = MHAL_NO_ERROR;

    memset(&a_ASDInfo, 0, sizeof(a_ASDInfo));

    SENSOR_AE_AWB_CUR_STRUCT cur;
       
    m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev), SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN, (MINT32)&cur, 0, 0);  
        
    calcASDEv(cur);

    a_ASDInfo.i4AELv_x10 = m_i4AELv_x10;
    a_ASDInfo.bAEStable = m_bAEStable;
    a_ASDInfo.i4AWBRgain_X128 = cur.SensorAwbGainCur.AwbCurRgain;
    a_ASDInfo.i4AWBBgain_X128 = cur.SensorAwbGainCur.AwbCurBgain;
    a_ASDInfo.i4AWBRgain_D65_X128 = m_AsdRef.SensorAwbGainRef.AwbRefD65Rgain;
    a_ASDInfo.i4AWBBgain_D65_X128 = m_AsdRef.SensorAwbGainRef.AwbRefD65Bgain;
    a_ASDInfo.i4AWBRgain_CWF_X128 = m_AsdRef.SensorAwbGainRef.AwbRefCWFRgain;
    a_ASDInfo.i4AWBBgain_CWF_X128 = m_AsdRef.SensorAwbGainRef.AwbRefCWFBgain;
        
    MY_LOG("[%s][i4AELv_x10] %d\n", __FUNCTION__, a_ASDInfo.i4AELv_x10);
    MY_LOG("[%s][bAEStable] %d\n", __FUNCTION__, a_ASDInfo.bAEStable);
    MY_LOG("[%s][i4AWBRgain_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_X128);
    MY_LOG("[%s][i4AWBBgain_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_X128);
    MY_LOG("[%s][i4AWBRgain_D65_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_D65_X128);
    MY_LOG("[%s][i4AWBBgain_D65_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_D65_X128);
    MY_LOG("[%s][i4AWBRgain_CWF_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_CWF_X128);
    MY_LOG("[%s][i4AWBBgain_CWF_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_CWF_X128);
    
    return S_3A_OK;
}
