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
#define LOG_TAG "af_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <kd_camera_feature.h>
#include <aaa_log.h>
#include <mtkcam/common/faces.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/featureio/flicker_hal_base.h>
#include "af_mgr.h"
#include <mtkcam/algorithm/lib3a/af_algo_if.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>

#include "camera_custom_cam_cal.h"  //seanlin 121022 for test
#include "cam_cal_drv.h" //seanlin 121022 for test

// AF v1.2
#include <math.h>
#include <android/sensor.h>             // for g/gyro sensor listener
#include <utils/SensorListener.h>    // for g/gyro sensor listener
#define SENSOR_ACCE_POLLING_MS  33     
#define SENSOR_GYRO_POLLING_MS  33

#define WIN_SIZE_MIN 8
#define WIN_SIZE_MAX 510
#define WIN_POS_MIN  16

#define FL_WIN_SIZE_MIN 8
#define FL_WIN_SIZE_MAX 4094
#define FL_WIN_POS_MIN  16
#define FLKPAUS

NVRAM_LENS_PARA_STRUCT* g_pNVRAM_LENS;
#ifdef FLKPAUS
FlickerHalBase* m_pFlickerHal;
#endif

using namespace NS3A;

// AF v1.2
// g/gyro sensor listener handler and data
static MINT32 gAcceInfo[3];
static MUINT64 gAcceTS;
static MBOOL gAcceValid = MFALSE;
static MINT32 gGyroInfo[3];
static MUINT64 gGyroTS;
static MBOOL gGyroValid = MFALSE;

static SensorListener* gpSensorListener;
void afListenHandler(ASensorEvent event)
{
    switch(event.type)
    {
        case ASENSOR_TYPE_ACCELEROMETER:
        {
            gAcceInfo[0] = event.acceleration.x * SENSOR_ACCE_SCALE;
            gAcceInfo[1] = event.acceleration.y * SENSOR_ACCE_SCALE;
            gAcceInfo[2] = event.acceleration.z * SENSOR_ACCE_SCALE;  
            gAcceTS = event.timestamp;
            /*MY_LOG("SensorEventAF Acc(%f,%f,%f,%lld)",
                event.acceleration.x,
                event.acceleration.y,
                event.acceleration.z,
                event.timestamp);*/
            break;
        }        
        case ASENSOR_TYPE_GYROSCOPE:
        {
            gGyroInfo[0] = event.vector.x * SENSOR_GYRO_SCALE;
            gGyroInfo[1] = event.vector.y * SENSOR_GYRO_SCALE;
            gGyroInfo[2] = event.vector.z * SENSOR_GYRO_SCALE;                        
            gGyroTS = event.timestamp;
            /*MY_LOG("SensorEventAF Gyro(%f,%f,%f,%lld)",
                event.vector.x,
                event.vector.y,
                event.vector.z,
                event.timestamp);*/
            break;
        }
        case ASENSOR_TYPE_MAGNETIC_FIELD:
        case ASENSOR_TYPE_LIGHT:
        case ASENSOR_TYPE_PROXIMITY:
        default:
        {
            MY_LOG("unknown type(%d)",event.type);
            break;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr& AfMgr::getInstance()
{
    static  AfMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr()
{
    m_Users   = 0;
    m_pSensorHal = NULL;           
    m_pMcuDrv = NULL;
    m_pIspDrv = NULL;
    m_pIspReg = NULL;
    m_pIAfAlgo = NULL;
    memset(&m_sAFInput,   0, sizeof(m_sAFInput));
    memset(&m_sAFOutput,   0, sizeof(m_sAFOutput));
    memset(&m_sAFParam,   0, sizeof(m_sAFParam));
    memset(&m_sAFConfig,   0, sizeof(m_sAFConfig));
    memset(&m_NVRAM_LENS,   0, sizeof(m_NVRAM_LENS));
    memset(&m_CameraFocusArea,   0, sizeof(m_CameraFocusArea));    
    memset(&m_FDArea,   0, sizeof(m_FDArea));
    memset(&m_sAFFullStat,   0, sizeof(m_sAFFullStat));
    memset(&m_sEZoom,   0, sizeof(m_sEZoom));
    
    m_i4AF_in_Hsize = 0;
    m_i4AF_in_Vsize = 0;

    m_i4CurrSensorDev = 0;
    m_i4CurrSensorId = 0x1;
    m_i4CurrLensId = 0;
    
    m_i4EnableAF = -1;
    
    m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
    
    m_sAFInput.i4FullScanStep = 1;

    m_pAFCallBack = NULL;
    m_i4AFPreStatus = AF_MARK_NONE;
    m_bDebugEnable = FALSE;
    m_i4AutoFocuscb = FALSE;

    m_i4AF_TH[0] = -1;
    m_i4AF_TH[2] = -1;    
    m_i4AF_THEX = -1;
    m_i4GMR[0] = -1;
    m_i4GMR[1] = -1;
    m_i4GMR[2] = -1;    
    m_tcaf_mode=0;
    m_preMCUpos=0;

    m_i4AutoFocus = FALSE;

    for (MINT32 i=0; i<36; i++)   {
        m_i8PreVStat[i] = 0;
    }
    
    m_i8BSSVlu   = 0;
	m_flkwin_syncflag  = 0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::init()
{
    MRESULT ret = S_3A_OK;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    //m_bDebugEnable = 1;

    Mutex::Autolock lock(m_Lock);
    m_tcaf_mode=0;
    m_preMCUpos=0;

    if (m_Users > 0)   {
        MY_LOG("[init] no init, %d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }
    else
    {              
        m_i4EnableAF = -1;
        MY_LOG("[init]EnableAF %d", m_i4EnableAF);
    }

    MY_LOG("[init] start, m_Users: %d", m_Users);

    // --- init MCU ---    
    if (m_i4EnableAF == -1)   {
        
        if (!m_pSensorHal)   {
            m_pSensorHal = SensorHal::createInstance();
        }
    
        if (!m_pSensorHal)   {
            MY_ERR("SensorHal::createInstance fail");
            return E_AF_NULL_POINTER;
        }
        m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (MINT32)&m_i4CurrSensorDev, 0, 0);
        m_pSensorHal->sendCommand((halSensorDev_e)m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_ID, (MINT32)&m_i4CurrSensorId, 0, 0);
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    
        MCUDrv::lensSearch(m_i4CurrSensorDev, m_i4CurrSensorId);
        m_i4CurrLensId = MCUDrv::getCurrLensID();

        MY_LOG("[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

        if (m_i4CurrLensId == 0xFFFF)   {m_i4EnableAF = 0;}
        else                            {m_i4EnableAF = 1;}
    }
    
    m_pMcuDrv = MCUDrv::createInstance(m_i4CurrLensId);   

    if (!m_pMcuDrv)   {
        MY_ERR("McuDrv::createInstance fail");
        return E_AF_NULL_POINTER;
    }

    if (m_pMcuDrv->init() < 0)   {
        MY_ERR("m_pMcuDrv->init() fail");
        return E_AF_MCU_FAIL;        
    }

    // --- init ISP Drv/Reg ---
    m_pIspDrv = IspDrv::createInstance();

    if (!m_pIspDrv) {
        MY_ERR("IspDrv::createInstance() fail \n");
        return E_AF_NULL_POINTER;
    }

    if (m_pIspDrv->init() < 0) {
        MY_ERR("pIspDrv->init() fail \n");
        return E_AF_ISP_DRV_FAIL;
    }

    //m_pVirtIspDrvCQ0 = m_pIspDrv->getCQInstance(ISP_DRV_CQ0);
    //m_pIspReg = (isp_reg_t*)m_pVirtIspDrvCQ0->getRegAddr();
    m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddr();

    if (!m_pIspReg) {
        MY_ERR("IspReg pointer NULL \n");
        return E_AF_NULL_POINTER;
    }

    setAF_IN_HSIZE();
    if ((m_sAFInput.sEZoom.i4W == 0) || (m_sAFInput.sEZoom.i4H == 0))   {

        m_sAFInput.sEZoom.i4W = m_i4AF_in_Hsize;
        m_sAFInput.sEZoom.i4H = m_i4AF_in_Vsize;        
    }

    // --- init af algo ---
    m_pIAfAlgo = IAfAlgo::createInstance();
    if (!m_pIAfAlgo) {
        MY_ERR("AfAlgo pointer NULL \n");
        return E_AF_NULL_POINTER;
    }
    m_i4AFPreStatus = AF_MARK_NONE;
    m_sAFOutput.sAFArea.sRect[0].i4Info=AF_MARK_NONE;
    m_i4AutoFocuscb = FALSE;
    m_sAFOutput.i4FDDetect=0;

    #ifdef FLKPAUS
    m_pFlickerHal = FlickerHalBase::getInstance();

    if (!m_pFlickerHal) {
        MY_ERR("m_pFlickerHal pointer NULL \n");
        return E_AF_NULL_POINTER;
    }
    #endif

    // --- NVRAM ---
    if (FAILED(NvramDrvMgr::getInstance().init(m_i4CurrSensorDev))) {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AWB_SENSOR_ERROR;
    }

    NvramDrvMgr::getInstance().getRefBuf(g_pNVRAM_LENS);
    NvramDrvMgr::getInstance().uninit();

    m_NVRAM_LENS.Version = g_pNVRAM_LENS->Version;
    m_NVRAM_LENS.rFocusRange = g_pNVRAM_LENS->rFocusRange;
    m_NVRAM_LENS.rAFNVRAM= g_pNVRAM_LENS->rAFNVRAM;

    #if 1
    //MY_LOG("[nvram][THRES_MAIN]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    //MY_LOG("[nvram][SUB_MAIN]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_SUB);
    MY_LOG("[nvram][Normal Num]%d [Macro Num]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum, m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);
    //MY_LOG("[nvram][VAFC Fail Cnt]%d", m_NVRAM_LENS.rAFNVRAM.i4VAFC_FAIL_CNT);
    //MY_LOG("[nvram][LV thres]%d", m_NVRAM_LENS.rAFNVRAM.i4LV_THRES);
    //MY_LOG("[nvram][PercentW]%d [PercentH]%d", m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W, m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H);
    //MY_LOG("[nvram][AFC step]%d", m_NVRAM_LENS.rAFNVRAM.i4AFC_STEP_SIZE);    
    //MY_LOG("[nvram][InfPos]%d", m_NVRAM_LENS.rFocusRange.i4InfPos);
    //MY_LOG("[nvram][MacroPos]%d", m_NVRAM_LENS.rFocusRange.i4MacroPos);
    #endif
    
    // AF v1.2    
    // --- init g/gyro sensor listener ---
    // i4Coef[5] == 1:enable SensorListener, else: disable    
    if((m_NVRAM_LENS.rAFNVRAM.i4Coef[5] > 0) && (m_AndroidServiceState==TRUE) ) 
    {                           
        gpSensorListener = SensorListener::createInstance();   
        gpSensorListener->setListener(afListenHandler);

        gAcceInfo[0] = gAcceInfo[1] = gAcceInfo[2] = 0;            
        gAcceValid = gpSensorListener->enableSensor(SensorListener::SensorType_Acc, SENSOR_ACCE_POLLING_MS);
        if(!gAcceValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Acc);
        }

        gGyroInfo[0] = gGyroInfo[1] = gGyroInfo[2] = 0; 
        gGyroValid = gpSensorListener->enableSensor(SensorListener::SensorType_Gyro, SENSOR_GYRO_POLLING_MS);
        if(!gGyroValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
        }
    }
        
    // --- Param ---
    m_sAFParam = getAFParam();

    MY_LOG("[Param][Normal min step]%d [Macro min step]%d", m_sAFParam.i4AFS_STEP_MIN_NORMAL, m_sAFParam.i4AFS_STEP_MIN_MACRO);

    // --- Config ---
    m_sAFConfig = getAFConfig();

    setAFArea(m_CameraFocusArea);
    
    // AF v1.2    
    // i4Coef[7] > 0: enable new scenechange , else: disable
    if((m_NVRAM_LENS.rAFNVRAM.i4Coef[7] > 0) && (m_AndroidServiceState==TRUE) )
    {   // update m_NVRAM_LENS.rAFNVRAM before setAFParam to AF alg
        updateSceneChangeParams();
    }
    
    m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    m_pIAfAlgo->initAF(m_sAFInput, m_sAFOutput);

    //MY_LOG("m_sAFInput[Cnt]%d [W]%d [H]%d [X]%d [Y]%d\n", m_sAFInput.sAFArea.i4Count, m_sAFInput.sAFArea.sRect[0].i4W, m_sAFInput.sAFArea.sRect[0].i4H, m_sAFInput.sAFArea.sRect[0].i4X, m_sAFInput.sAFArea.sRect[0].i4Y);
    //MY_LOG("m_sAFOutput[Cnt]%d [W]%d [H]%d [X]%d [Y]%d\n", m_sAFOutput.sAFArea.i4Count, m_sAFOutput.sAFArea.sRect[0].i4W, m_sAFOutput.sAFArea.sRect[0].i4H, m_sAFOutput.sAFArea.sRect[0].i4X, m_sAFOutput.sAFArea.sRect[0].i4Y);

    if (m_sAFParam.i4ReadOTP == TRUE)   {
        readOTP();
    }

    m_i4AF_TH[0] = -1;
    m_i4AF_TH[2] = -1;    
    m_i4AF_THEX = -1;

    m_PreAFArea.sRect[0].i4X =0;
    m_PreAFArea.sRect[0].i4W =0; 
    m_PreAFArea.sRect[0].i4Y =0;
    m_PreAFArea.sRect[0].i4H =0; 
    m_PreFDArea.sRect[0].i4X =0;
    m_PreFDArea.sRect[0].i4Y =0; 
    m_PreFDArea.sRect[0].i4W =0;
    m_PreFDArea.sRect[0].i4H =0; 

    setAFConfig(m_sAFOutput.sAFStatConfig);
    setAFWinTH(m_sAFOutput.sAFStatConfig);
    setGMR(m_sAFOutput.sAFStatConfig);    
    setAFWinConfig(m_sAFOutput.sAFArea);
    setFlkWinConfig();

    printAFConfigLog0();
    printAFConfigLog1();

    m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);
    
    // --- test code ---
    #if 0
    m_pMcuDrv->moveMCU(700);
    mcuMotorInfo MotorInfo;
    m_pMcuDrv->setMCUMacroPos(800);
    m_pMcuDrv->getMCUInfo(&MotorInfo);
    MY_LOG("[lens][inf]%d, [curr]%d, [macro]%d", MotorInfo.u4InfPosition, MotorInfo.u4CurrentPosition, MotorInfo.u4MacroPosition);
    #endif
    // ----------------

    android_atomic_inc(&m_Users);

    MY_LOG("[init] finish");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::uninit()
{

    MRESULT ret = S_3A_OK;
    m_tcaf_mode=0;

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)   {
        return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users != 0)  { // There are still some users.
        MY_LOG("[uninit] Still %d users \n", m_Users);
        return S_AF_OK;
    }
    MY_LOG("[uninit] m_Users: %d", m_Users);

    if (m_pIAfAlgo)   {
        m_pIAfAlgo->destroyInstance();
        m_pIAfAlgo = NULL;
    }
    
    // AF v1.2 
    // --- uninit g/gyro sensor listener ---    
    // i4Coef[5] == 1:enable SensorListener, else: disable
    if( (m_NVRAM_LENS.rAFNVRAM.i4Coef[5] > 0)  && (m_AndroidServiceState==TRUE) )
    {          
        if(gAcceValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Acc);
        }
        
        if(gGyroValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
        }  
        
        gpSensorListener->destroyInstance();
        gpSensorListener = NULL;
    }
    
    if (m_pMcuDrv)   {
        m_pMcuDrv->uninit();
        m_pMcuDrv->destroyInstance();
        m_pMcuDrv = NULL;
    }

    if (m_pIspDrv)   {
        m_pIspDrv->uninit();
        m_pIspReg = NULL;
        m_pVirtIspDrvCQ0 = NULL;
        m_pIspDrv = NULL;
    }

    #ifdef FLKPAUS
    if (m_pFlickerHal)   {
        m_pFlickerHal = NULL;
    }
    #endif

    m_i4EnableAF = -1;

    m_i4GMR[0] = -1;
    m_i4GMR[1] = -1;
    m_i4GMR[2] = -1;    
    
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFOpeartion()
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_OPERATION]\n");
    setAFMode(AF_MODE_AFS);
    triggerAF();
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPMFOpeartion(MINT32 a_i4MFpos)
{
    MINT32 i4TimeOutCnt = 0;

    MY_LOG("[ACDK_CCT_V2_OP_MF_OPERATION]\n");

    setAFMode(AF_MODE_MF);
    triggerAF();
    setMFPos(a_i4MFpos);

    while (!isFocusFinish())   {
        usleep(5000); // 5ms
        i4TimeOutCnt++;

        if (i4TimeOutCnt > 100)   {
            break;
        }
    }

    MY_LOG("[MF]pos:%d, value:%lld\n", a_i4MFpos, m_sAFInput.sAFStat.i8Stat24);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetAFInfo(MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;

    MY_LOG("[ACDK_CCT_V2_OP_GET_AF_INFO]\n");

    pAFInfo->i4AFMode = m_eLIB3A_AFMode;
    pAFInfo->i4AFMeter = LIB3A_AF_METER_SPOT;
    pAFInfo->i4CurrPos = m_sAFOutput.i4AFPos;

    *a_pOutLen = sizeof(ACDK_AF_INFO_T);

    MY_LOG("[AF Mode] = %d\n", pAFInfo->i4AFMode);
    MY_LOG("[AF Meter] = %d\n", pAFInfo->i4AFMeter);
    MY_LOG("[AF Current Pos] = %d\n", pAFInfo->i4CurrPos);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetBestPos(MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_BEST_POS]%d\n", m_sAFOutput.i4AFBestPos);
    *a_pAFBestPos = m_sAFOutput.i4AFBestPos;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFCaliOperation(MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AF_DEBUG_INFO_T rAFDebugInfo;
    MUINT32 aaaDebugSize;
    MINT32 i4TimeOutCnt = 0;

    MY_LOG("[ACDK_CCT_V2_OP_AF_CALI_OPERATION]\n");

    setAFMode(AF_MODE_AFS);
    usleep(500000);    // 500ms
    m_eLIB3A_AFMode = LIB3A_AF_MODE_CALIBRATION;
    m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);

    usleep(500000);    // 500ms

    while(!isFocusFinish())   {
        usleep(30000); // 30ms
        i4TimeOutCnt++;

        if (i4TimeOutCnt > 2000)   {
             break;
        }
    }

    getDebugInfo(rAFDebugInfo);

    pAFCaliData->i4Gap = (MINT32)rAFDebugInfo.Tag[3].u4FieldValue;

    for (MINT32 i = 0; i < 512; i++)   {
        if (rAFDebugInfo.Tag[i+4].u4FieldValue != 0)   {
            pAFCaliData->i8Vlu[i] = (MINT64)rAFDebugInfo.Tag[i+4].u4FieldValue;  // need fix it
            pAFCaliData->i4Num = i+1;
        }
        else   {
            break;
        }
    }

    pAFCaliData->i4BestPos = m_sAFOutput.i4AFBestPos;

    MY_LOG("[AFCaliData] Num = %d\n", pAFCaliData->i4Num);
    MY_LOG("[AFCaliData] Gap = %d\n", pAFCaliData->i4Gap);
    for (MINT32 i=0; i<pAFCaliData->i4Num; i++)   {
        MY_LOG("[AFCaliData] Vlu %d = %lld\n", i, pAFCaliData->i8Vlu[i]);
    }
    MY_LOG("[AFCaliData] Pos = %d\n", pAFCaliData->i4BestPos);

    setAFMode(AF_MODE_AFS);

    *a_pOutLen = sizeof(MINT32);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSetFocusRange(MVOID *a_pFocusRange)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_SET_RANGE]\n");
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    m_NVRAM_LENS.rFocusRange = *pFocusRange;
    
    if (m_pMcuDrv) {
        m_pMcuDrv->setMCUInfPos(m_NVRAM_LENS.rFocusRange.i4InfPos);
        m_pMcuDrv->setMCUMacroPos(m_NVRAM_LENS.rFocusRange.i4MacroPos);
    }

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    }

    MY_LOG("[Inf Pos] = %d\n", m_NVRAM_LENS.rFocusRange.i4InfPos);
    MY_LOG("[Marco Pos] = %d\n", m_NVRAM_LENS.rFocusRange.i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFocusRange(MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_RANGE]\n");
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    *pFocusRange = m_NVRAM_LENS.rFocusRange;
    *a_pOutLen = sizeof(FOCUS_RANGE_T);

    MY_LOG("[Inf Pos] = %d\n", pFocusRange->i4InfPos);
    MY_LOG("[Marco Pos] = %d\n", pFocusRange->i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_READ]\n");

    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = reinterpret_cast<NVRAM_LENS_PARA_STRUCT*>(a_pAFNVRAM);

    if (FAILED(NvramDrvMgr::getInstance().init(m_i4CurrSensorDev))) {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AWB_SENSOR_ERROR;
    }

    NvramDrvMgr::getInstance().getRefBuf(g_pNVRAM_LENS);
    NvramDrvMgr::getInstance().uninit();

    m_NVRAM_LENS.Version = g_pNVRAM_LENS->Version;
    m_NVRAM_LENS.rFocusRange = g_pNVRAM_LENS->rFocusRange;
    m_NVRAM_LENS.rAFNVRAM= g_pNVRAM_LENS->rAFNVRAM;
    
    *pAFNVRAM = m_NVRAM_LENS;
    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);
    
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFApplyNVRAMParam(MVOID *a_pAFNVRAM)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_APPLY]\n");
    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM;
    m_NVRAM_LENS = *pAFNVRAM;    

    MY_LOG("Apply to Phone[Thres Main]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("Apply to Phone[Thres Sub]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("Apply to Phone[HW_TH]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_TH.i4HW_TH[0][0]);
    MY_LOG("Apply to Phone[Statgain]%d\n", m_NVRAM_LENS.rAFNVRAM.i4StatGain);    

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    }

    if (m_pMcuDrv) {
        m_pMcuDrv->setMCUInfPos(m_NVRAM_LENS.rFocusRange.i4InfPos);
        m_pMcuDrv->setMCUMacroPos(m_NVRAM_LENS.rFocusRange.i4MacroPos);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSaveNVRAMParam()
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM]\n");

    MUINT32 u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum;    
    
    MRESULT err = S_AF_OK;
    
    NvramDrvBase* pNvramDrvObj = NvramDrvBase::createInstance();
    
    NSNvram::BufIF<NVRAM_LENS_PARA_STRUCT>*const pBufIF_Lens = pNvramDrvObj->getBufIF< NVRAM_LENS_PARA_STRUCT>();
    
    //  Sensor driver.
    SensorHal*const pSensorHal = SensorHal::createInstance();
    
    switch  ( m_i4CurrSensorDev )
    {
    case ESensorDev_Main:
        eSensorEnum = DUAL_CAMERA_MAIN_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case ESensorDev_Sub:
        eSensorEnum = DUAL_CAMERA_SUB_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case ESensorDev_MainSecond:
        eSensorEnum = DUAL_CAMERA_MAIN_SECOND_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", m_i4CurrSensorDev);
        err = E_NVRAM_BAD_PARAM;
        goto lbExit;
    }
    
    g_pNVRAM_LENS = pBufIF_Lens->getRefBuf(eSensorEnum, u4SensorID);
    
    MY_LOG("WriteNVRAM from Phone[Thres Main]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("WriteNVRAM from Phone[Thres Sub]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("WriteNVRAM from Phone[HW_TH]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_TH.i4HW_TH[0][0]);
    MY_LOG("WriteNVRAM from Phone[Statgain]%d\n", m_NVRAM_LENS.rAFNVRAM.i4StatGain);    
    
    g_pNVRAM_LENS->Version = m_NVRAM_LENS.Version;
    g_pNVRAM_LENS->rFocusRange = m_NVRAM_LENS.rFocusRange;
    g_pNVRAM_LENS->rAFNVRAM = m_NVRAM_LENS.rAFNVRAM;
    
    pBufIF_Lens->flush(eSensorEnum, u4SensorID);
    
    lbExit:
    if  ( pSensorHal )
        pSensorHal->destroyInstance();
    
    if ( pNvramDrvObj )
        pNvramDrvObj->destroyInstance();
    
    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFV(MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    ACDK_AF_POS_T *pAFPos = (ACDK_AF_POS_T *) a_pAFPosIn;
    ACDK_AF_VLU_T *pAFValue = (ACDK_AF_VLU_T *) a_pAFValueOut;

    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_FV]\n");

    pAFValue->i4Num = pAFPos->i4Num;

    setAFMode(AF_MODE_AFS);
    usleep(500000); // 500ms
    setAFMode(AF_MODE_MF);

    for (MINT32 i = 0; i < pAFValue->i4Num; i++) {

        setMFPos(pAFPos->i4Pos[i]);

        usleep(500000); // 500ms

        pAFValue->i8Vlu[i] = m_sAFInput.sAFStat.i8Stat24;

        MY_LOG("[FV]pos = %d, value = %lld\n", pAFPos->i4Pos[i], pAFValue->i8Vlu[i]);
    }

    setAFMode(AF_MODE_AFS);

    *a_pOutLen = sizeof(ACDK_AF_VLU_T);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFEnable()
{
    MY_LOG("[ACDK_CCT_OP_AF_ENABLE]\n");
    m_i4EnableAF = 1;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFDisable()
{
    MY_LOG("[ACDK_CCT_OP_AF_DISABLE]\n");
    m_i4EnableAF = 0;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetEnableInfo(MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_OP_AF_GET_ENABLE_INFO]%d\n", m_i4EnableAF);
    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;    
    *pEnableAF = m_i4EnableAF;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF()
{    
    if (m_pIAfAlgo)  {
        m_pIAfAlgo->triggerAF();        
    }    

    m_sAFOutput.i4IsAFDone = MFALSE;
    
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFMode(MINT32 a_eAFMode)
{
    AF_MODE_T eAFMode = static_cast<AF_MODE_T>(a_eAFMode);    
    static AF_MODE_T eCurrAFMode = AF_MODE_AFS;

    if (eCurrAFMode == eAFMode)   {
        return S_AF_OK;
    }

    eCurrAFMode = eAFMode;
    MY_LOG("[AFMode]%d\n", eCurrAFMode);
    
    switch (eCurrAFMode) {
        case AF_MODE_AFS: // AF-Single Shot Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
            break;
        case AF_MODE_AFC: // AF-Continuous Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC;
            break;
        case AF_MODE_AFC_VIDEO: // AF-Continuous Mode (Video)
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC_VIDEO;
            break;            
        case AF_MODE_MACRO: // AF Macro Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_MACRO;
            break;
        case AF_MODE_INFINITY: // Focus is set at infinity
            m_eLIB3A_AFMode = LIB3A_AF_MODE_INFINITY;
            break;
        case AF_MODE_MF: // Manual Focus Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_MF;
            break;
        case AF_MODE_FULLSCAN: // AF Full Scan Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_FULLSCAN;
            break;
        default:
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
            break;
    }


    
    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }
    m_PreAFArea.sRect[0].i4X =0;
    m_PreAFArea.sRect[0].i4W =0; 
    m_PreAFArea.sRect[0].i4Y =0;
    m_PreAFArea.sRect[0].i4H =0; 
    m_PreFDArea.sRect[0].i4X =0;
    m_PreFDArea.sRect[0].i4Y =0; 
    m_PreFDArea.sRect[0].i4W =0;
    m_PreFDArea.sRect[0].i4H =0; 

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFArea(CameraFocusArea_T a_sAFArea)
{
    MY_LOG("[setAFArea][Cnt]%d [L]%d [R]%d [U]%d [B]%d\n", a_sAFArea.u4Count, a_sAFArea.rAreas[0].i4Left, a_sAFArea.rAreas[0].i4Right, a_sAFArea.rAreas[0].i4Top, a_sAFArea.rAreas[0].i4Bottom);

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC)   {
        a_sAFArea.u4Count = 0;
    }
    if (m_tcaf_mode && a_sAFArea.u4Count==0)
        return S_AF_OK;

    if ((a_sAFArea.u4Count == 0) || (a_sAFArea.rAreas[0].i4Left == a_sAFArea.rAreas[0].i4Right) || (a_sAFArea.rAreas[0].i4Top == a_sAFArea.rAreas[0].i4Bottom))   {

        a_sAFArea.u4Count = 1;
        a_sAFArea.rAreas[0].i4Left   = -1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W / 100;
        a_sAFArea.rAreas[0].i4Right  =  1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W / 100;
        a_sAFArea.rAreas[0].i4Top    = -1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H / 100;
        a_sAFArea.rAreas[0].i4Bottom =  1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H / 100;
    }
    
    if ((m_Users>0) && (memcmp(&m_CameraFocusArea, &a_sAFArea, sizeof(m_CameraFocusArea)) == 0))   {
        return S_AF_OK;
    }
    
    m_CameraFocusArea = a_sAFArea;

    if (a_sAFArea.u4Count >= AF_WIN_NUM_SPOT)    {
        m_sAFInput.sAFArea.i4Count = AF_WIN_NUM_SPOT;
    }
    else   {
        m_sAFInput.sAFArea.i4Count = a_sAFArea.u4Count;
    }    

    if (m_sAFInput.sAFArea.i4Count != 0)   {
        for (MINT32 i=0; i<m_sAFInput.sAFArea.i4Count; i++)   {

            a_sAFArea.rAreas[i].i4Left   = (a_sAFArea.rAreas[i].i4Left   +1000) * m_sAFInput.sEZoom.i4W  / 2000 + m_sAFInput.sEZoom.i4X;
            a_sAFArea.rAreas[i].i4Right  = (a_sAFArea.rAreas[i].i4Right  +1000) * m_sAFInput.sEZoom.i4W  / 2000 + m_sAFInput.sEZoom.i4X;
            a_sAFArea.rAreas[i].i4Top    = (a_sAFArea.rAreas[i].i4Top    +1000) * m_sAFInput.sEZoom.i4H  / 2000 + m_sAFInput.sEZoom.i4Y;
            a_sAFArea.rAreas[i].i4Bottom = (a_sAFArea.rAreas[i].i4Bottom +1000) * m_sAFInput.sEZoom.i4H  / 2000 + m_sAFInput.sEZoom.i4Y;
            
            m_sAFInput.sAFArea.sRect[i].i4W = a_sAFArea.rAreas[i].i4Right - a_sAFArea.rAreas[i].i4Left;
            m_sAFInput.sAFArea.sRect[i].i4H = a_sAFArea.rAreas[i].i4Bottom - a_sAFArea.rAreas[i].i4Top;
            m_sAFInput.sAFArea.sRect[i].i4X = a_sAFArea.rAreas[i].i4Left;
            m_sAFInput.sAFArea.sRect[i].i4Y = a_sAFArea.rAreas[i].i4Top;
            m_sAFInput.sAFArea.sRect[i].i4Info   = a_sAFArea.rAreas[i].i4Weight;            
        }
    }

    MY_LOG("[setAFArea][Cnt]%d [L]%d [R]%d [U]%d [B]%d to [Cnt]%d [W]%d [H]%d [X]%d [Y]%d\n", a_sAFArea.u4Count, a_sAFArea.rAreas[0].i4Left, a_sAFArea.rAreas[0].i4Right, a_sAFArea.rAreas[0].i4Top, a_sAFArea.rAreas[0].i4Bottom, m_sAFInput.sAFArea.i4Count, m_sAFInput.sAFArea.sRect[0].i4W, m_sAFInput.sAFArea.sRect[0].i4H, m_sAFInput.sAFArea.sRect[0].i4X, m_sAFInput.sAFArea.sRect[0].i4Y);
    
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamMode(MINT32 a_eCamMode)
{
    MY_LOG("[setCamMode]%d\n", a_eCamMode);
	m_tcaf_mode=0;
    if (a_eCamMode == eAppMode_ZsdMode)   {m_sAFInput.i4IsZSD = TRUE;}
    else                                  {m_sAFInput.i4IsZSD = FALSE;}
    return S_AF_OK;    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if ((m_sEZoom.i4X == (MINT32)u4XOffset) &&
        (m_sEZoom.i4Y == (MINT32)u4YOffset) &&
        (m_sEZoom.i4W == (MINT32)u4Width) &&
        (m_sEZoom.i4H == (MINT32)u4Height))    
    {
        return S_AF_OK;
    }
    
    MY_LOG("[setZoomWinInfo][w]%d[h]%d[x]%d[y]%d\n", u4Width, u4Height, u4XOffset, u4YOffset);
    m_sEZoom.i4X = (MINT32)u4XOffset;
    m_sEZoom.i4Y = (MINT32)u4YOffset;
    m_sEZoom.i4W = (MINT32)u4Width;
    m_sEZoom.i4H = (MINT32)u4Height;
    m_tcaf_mode=0;

    MINT8 no_init_flag=0;
    if (!m_pIspReg)    
    {
        MY_LOG("no IspDrv, init");
        no_init_flag=1;
        m_pIspDrv = IspDrv::createInstance();
        if (!m_pIspDrv) 
            MY_ERR("IspDrv::createInstance() fail \n");
        else if (m_pIspDrv->init() < 0) 
            MY_ERR("pIspDrv->init() fail \n");
        else    
            m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddr();
    }
    if (m_sEZoom.i4X == 0)   
    {
        setAF_IN_HSIZE();

        if (m_i4AF_in_Hsize != m_sEZoom.i4W)   
        {
            m_i4Factor = m_i4AF_in_Hsize * 100 / m_sEZoom.i4W;
        }
        else   
        {
            m_i4Factor = 100;
    }
    }

    /* real digital factor, m_i4Factor is always 1.00 */
    MINT32 i4DzFactor = 100*(2*m_sEZoom.i4X + m_sEZoom.i4W)/m_sEZoom.i4W; /* 1X-> 100, 2X->200, ... */
    /* init window config from customized parameter */
    MINT32 i4WinCfg=(m_NVRAM_LENS.rAFNVRAM.i4Coef[4]);
    /* error check */
    if(i4WinCfg<1) i4WinCfg = 1; /*i4WinCfg=0 --> 1 & error check*/
    if(i4WinCfg>4) i4WinCfg = 4; /*error check*/
    if(i4WinCfg==4)
    {
        MY_LOG("[setZoomWinInfo] always DZ, DZ=%d, Bound=%d \n",i4DzFactor,i4WinCfg*100);
        /* i4WinCfg=4-> DigZoomFac>4, AF win no change */
    m_sAFInput.sEZoom.i4X = m_sEZoom.i4X * m_i4Factor / 100;
    m_sAFInput.sEZoom.i4Y = m_sEZoom.i4Y * m_i4Factor / 100;
    m_sAFInput.sEZoom.i4W = m_sEZoom.i4W * m_i4Factor / 100;
    m_sAFInput.sEZoom.i4H = m_sEZoom.i4H * m_i4Factor / 100;
    }
    else /* i4WinCfg = 1~3 */
    {
        /* i4WinCfg=1-> DigZoomFac>1, AF win no change */
        /* i4WinCfg=2-> DigZoomFac>2, AF win no change */
        /* i4WinCfg=3-> DigZoomFac>3, AF win no change */
        if(i4DzFactor >= i4WinCfg*100) /* fix to upper bound */
        {
            MY_LOG("[setZoomWinInfo] >bound2fix, DZ=%d, Bound=%d\n",i4DzFactor,i4WinCfg*100);
            m_sAFInput.sEZoom.i4W = (2*m_sEZoom.i4X + m_sEZoom.i4W)/i4WinCfg ;
            m_sAFInput.sEZoom.i4H = (2*m_sEZoom.i4Y + m_sEZoom.i4H)/i4WinCfg ;
            m_sAFInput.sEZoom.i4X = ((2*m_sEZoom.i4X + m_sEZoom.i4W)-m_sAFInput.sEZoom.i4W)/2;
            m_sAFInput.sEZoom.i4Y = ((2*m_sEZoom.i4Y + m_sEZoom.i4H)-m_sAFInput.sEZoom.i4H)/2;
        }
        else /* (i4DzFactor < i4WinCfg*100), AF win change aligning to digital zoom factor */
        {
            MY_LOG("[setZoomWinInfo] <bound2DZ, DZ=%d, Bound=%d \n",i4DzFactor,i4WinCfg*100);
            m_sAFInput.sEZoom.i4X = m_sEZoom.i4X ;
            m_sAFInput.sEZoom.i4Y = m_sEZoom.i4Y ;
            m_sAFInput.sEZoom.i4W = m_sEZoom.i4W ;
            m_sAFInput.sEZoom.i4H = m_sEZoom.i4H ;
        }
    }

    MY_LOG_IF(m_bDebugEnable, "[setZoomWinInfo][w]%d[h]%d[x]%d[y]%d to [w]%d[h]%d[x]%d[y]%d\n", u4Width, u4Height, u4XOffset, u4YOffset, m_sAFInput.sEZoom.i4W, m_sAFInput.sEZoom.i4H, m_sAFInput.sEZoom.i4X, m_sAFInput.sEZoom.i4Y);

    if (m_sAFOutput.i4IsMonitorFV == TRUE)   
    {
        setFlkWinConfig();
    }
    else   
    {
        CameraFocusArea_T sAFArea;
        sAFArea.u4Count = 0;
        setAFArea(sAFArea);
        setAFWinConfig(m_sAFInput.sAFArea);
    }

    if(no_init_flag)
    {
        if (m_pIspDrv)     
        {
            m_pIspDrv->uninit();
            m_pIspReg = NULL;
            m_pVirtIspDrvCQ0 = NULL;
            m_pIspDrv = NULL;
        }
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFDInfo(MVOID* a_sFaces)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    m_tcaf_mode=0;

    CameraFocusArea_T sFDArea;

    sFDArea.rAreas[0].i4Left   = (pFaces->faces->rect[0]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sFDArea.rAreas[0].i4Right  = (pFaces->faces->rect[2]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sFDArea.rAreas[0].i4Top    = (pFaces->faces->rect[1]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;
    sFDArea.rAreas[0].i4Bottom = (pFaces->faces->rect[3]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;

    m_FDArea.i4Count = (MINT32)pFaces->number_of_faces;
    m_FDArea.sRect[0].i4X = sFDArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4Y = sFDArea.rAreas[0].i4Top;
    m_FDArea.sRect[0].i4W = sFDArea.rAreas[0].i4Right - sFDArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4H = sFDArea.rAreas[0].i4Bottom - sFDArea.rAreas[0].i4Top;

    if (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent > 150)   {m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent = 150;}
    
    m_FDArea.sRect[0].i4X = m_FDArea.sRect[0].i4X + m_FDArea.sRect[0].i4W * ((100 - m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4Y = m_FDArea.sRect[0].i4Y + m_FDArea.sRect[0].i4H * ((100 - m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4W = m_FDArea.sRect[0].i4W * (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent) / 100;
    m_FDArea.sRect[0].i4H = m_FDArea.sRect[0].i4H * (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent) / 100;

    if ((m_FDArea.sRect[0].i4W == 0) || (m_FDArea.sRect[0].i4H == 0))   {
        m_FDArea.i4Count = 0;
    }

    MY_LOG_IF(m_bDebugEnable, "[setFDInfo]cnt:%d, X:%d Y:%d W:%d H:%d", m_FDArea.i4Count, m_FDArea.sRect[0].i4X, m_FDArea.sRect[0].i4Y, m_FDArea.sRect[0].i4W, m_FDArea.sRect[0].i4H);

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setFDWin(m_FDArea);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTInfo(MVOID* a_sOT)
{
    MtkCameraFaceMetadata *pObtinfo = (MtkCameraFaceMetadata *)a_sOT;
    m_tcaf_mode=0;
    m_FDArea.i4Score=pObtinfo->faces->score;

    CameraFocusArea_T sOBTArea;

    sOBTArea.rAreas[0].i4Left   = (pObtinfo->faces->rect[0]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sOBTArea.rAreas[0].i4Right  = (pObtinfo->faces->rect[2]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sOBTArea.rAreas[0].i4Top    = (pObtinfo->faces->rect[1]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;
    sOBTArea.rAreas[0].i4Bottom = (pObtinfo->faces->rect[3]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;

    m_FDArea.i4Count = (MINT32)pObtinfo->number_of_faces;
    m_FDArea.sRect[0].i4X = sOBTArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4Y = sOBTArea.rAreas[0].i4Top;
    m_FDArea.sRect[0].i4W = sOBTArea.rAreas[0].i4Right - sOBTArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4H = sOBTArea.rAreas[0].i4Bottom - sOBTArea.rAreas[0].i4Top;

    if (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent > 100)   {m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent = 100;}
    
    m_FDArea.sRect[0].i4X = m_FDArea.sRect[0].i4X + m_FDArea.sRect[0].i4W * ((100 - m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4Y = m_FDArea.sRect[0].i4Y + m_FDArea.sRect[0].i4H * ((100 - m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4W = m_FDArea.sRect[0].i4W * (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent) / 100;
    m_FDArea.sRect[0].i4H = m_FDArea.sRect[0].i4H * (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent) / 100;

    if ((m_FDArea.sRect[0].i4W == 0) || (m_FDArea.sRect[0].i4H == 0))   {
        m_FDArea.i4Count = 0;
    }

    MY_LOG_IF(m_bDebugEnable, "[setOTInfo]cnt:%d, X:%d Y:%d W:%d H:%d", m_FDArea.i4Count, m_FDArea.sRect[0].i4X, m_FDArea.sRect[0].i4Y, m_FDArea.sRect[0].i4W, m_FDArea.sRect[0].i4H);

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setFDWin(m_FDArea);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }

    return S_AF_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFMaxAreaNum()
{
    if (m_i4EnableAF == -1)   {
        if (!m_pSensorHal)   {
            m_pSensorHal = SensorHal::createInstance();
        }
    
        if (!m_pSensorHal)   {
            MY_LOG("SensorHal::createInstance fail");
            return 0;
        }
        m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (MINT32)&m_i4CurrSensorDev, 0, 0);
        m_pSensorHal->sendCommand((halSensorDev_e)m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_ID, (MINT32)&m_i4CurrSensorId, 0, 0);
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    
        MCUDrv::lensSearch(m_i4CurrSensorDev, m_i4CurrSensorId);
        m_i4CurrLensId = MCUDrv::getCurrLensID();

        MY_LOG("[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

        if (m_i4CurrLensId == 0xFFFF)   {m_i4EnableAF = 0;}
        else                            {m_i4EnableAF = 1;}
    }

    if (m_i4EnableAF)   {
        //MY_LOG_IF(m_bDebugEnable,"[getAFMaxAreaNum]%d\n", AF_WIN_NUM_SPOT);
        return AF_WIN_NUM_SPOT;
    }
    else   {
        MY_LOG("[getAFMaxAreaNum][AF disable]%d\n", 0);
        return 0;
    }       
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMaxLensPos()
{
    if (m_pMcuDrv)  {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo(&MotorInfo);
        //MY_LOG_IF(m_bDebugEnable,"[getMaxLensPos]%d\n", (MINT32)MotorInfo.u4MacroPosition);        
        return (MINT32)MotorInfo.u4MacroPosition;
    }
    else   {
        MY_LOG("[getMaxLensPos]m_pMcuDrv NULL\n");                
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMinLensPos()
{
    if (m_pMcuDrv)  {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo(&MotorInfo);
        //MY_LOG_IF(m_bDebugEnable,"[getMinLensPos]%d\n", (MINT32)MotorInfo.u4InfPosition);                
        return (MINT32)MotorInfo.u4InfPosition;
    }
    else   {
        MY_LOG("[getMinLensPos]m_pMcuDrv NULL\n");        
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFBestPos()
{
    return m_sAFOutput.i4AFBestPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFPos()
{
    return m_sAFOutput.i4AFPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFStable()
{
    return m_sAFOutput.i4IsAFDone;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableOffset()
{
    if (m_sAFInput.i4IsZSD)   {
        return m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4Offset;
    }
    else   {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   {            
            return m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset;
        }
        else   {
            return m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableMacroIdx()
{
    if (m_sAFInput.i4IsZSD)   {
        return m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4NormalNum;
    }
    else   {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   {            
            return m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
        }
        else   {
            return m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableIdxNum()
{
    return AF_TABLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* AfMgr::getAFTable()
{
    if (m_sAFInput.i4IsZSD)   {
        return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4Pos;
    }
    else   {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   {            
            return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos;
        }
        else   {
            return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setMFPos(MINT32 a_i4Pos)
{
    if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) && (m_sAFOutput.i4AFPos != a_i4Pos))   {

        MY_LOG("[setMFPos]%d\n", a_i4Pos);        
        m_i4MFPos = a_i4Pos;

        if (a_i4Pos==0xFFFFFFFF)  
        {
            MY_LOG("[setMFPos]skip set mf pos\n");   
            return S_AF_OK;
        }

        if (m_pIAfAlgo)  {
            m_pIAfAlgo->setMFPos(m_i4MFPos);
        }
        else   {
            MY_LOG("Null m_pIAfAlgo\n");
        }
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFullScanstep(MINT32 a_i4Step)
{
    if (m_sAFInput.i4FullScanStep != a_i4Step)   {
        MY_LOG("[setFullScanstep]%d\n", a_i4Step);
        m_sAFInput.i4FullScanStep = a_i4Step;
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T AfMgr::getAFFullStat()
{
    return m_sAFFullStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::doAF(MVOID *pAFStatBuf)
{
    if (m_i4EnableAF == 0)  {        
        m_sAFOutput.i4IsAFDone = 1;
        m_sAFOutput.i4IsFocused = 0;
        m_sAFOutput.i4AFPos = 0;
        MY_LOG("disableAF");        
        return S_AF_OK;
    }
    
    printAFConfigLog0();
    printAFConfigLog1();

    AE_MODE_CFG_T rPreviewInfo;
    AeMgr::getInstance().getPreviewParams(rPreviewInfo);

    MUINT8 iYvalue;
    AEMeterArea_T rWinSize;
    rWinSize.i4Left   = m_CameraFocusArea.rAreas[0].i4Left;
    rWinSize.i4Right  = m_CameraFocusArea.rAreas[0].i4Right;
    rWinSize.i4Top    = m_CameraFocusArea.rAreas[0].i4Top;
    rWinSize.i4Bottom = m_CameraFocusArea.rAreas[0].i4Bottom;
    
    AeMgr::getInstance().getAEMeteringYvalue(rWinSize, &iYvalue);

    m_sAFInput.i8GSum = (MINT64)iYvalue;
    m_sAFInput.i4ISO = (MINT32)rPreviewInfo.u4RealISO;
    m_sAFInput.i4IsAEStable = AeMgr::getInstance().IsAEStable();
    m_sAFInput.i4SceneLV = AeMgr::getInstance().getLVvalue(MTRUE);

    // AF v1.2
    // i4Coef[7] > 0: enable new scenechange , else: disable
    if(m_NVRAM_LENS.rAFNVRAM.i4Coef[7] > 0)
    {
        MUINT8 aeBlockV[25];
        AeMgr::getInstance().getAEBlockYvalues(aeBlockV, 25);
        
        m_pIAfAlgo->setAEBlockInfo(aeBlockV, 25);
    }

    // i4Coef[5] == 1:enable SensorListener, 0: disable
    if(m_NVRAM_LENS.rAFNVRAM.i4Coef[5] > 0)
    {         
        if(gAcceValid)
        {
            m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, SENSOR_ACCE_SCALE);
        }
        if(gGyroValid)
        {
            m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, SENSOR_GYRO_SCALE);    
        }
    }

    getLensInfo(m_sAFInput.sLensInfo);

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   
    {
        if (m_sAFOutput.i4IsMonitorFV == TRUE)   
        {
        m_sAFInput.sAFStat = Trans4WintoOneStat(pAFStatBuf);
        #ifdef FLKPAUS
        if(m_pFlickerHal)
            m_pFlickerHal->resume();
        #endif
    }
        else   
        {
        m_sAFInput.sAFStat = TransAFtoOneStat(pAFStatBuf);
        #ifdef FLKPAUS
        if(m_pFlickerHal)
           m_pFlickerHal->pause();
        #endif
    }
    }
    else   //AFS mode or else
    {
        m_FDArea.i4Count = 0;
        m_sAFOutput.i4FDDetect=0;
        if((m_sAFOutput.i4IsAFDone==0) && (	m_flkwin_syncflag==0)) //doing AF
        {
        m_sAFInput.sAFStat = TransAFtoOneStat(pAFStatBuf);
        #ifdef FLKPAUS
            //MY_LOG("FLKResume\n");
        if(m_pFlickerHal)
           m_pFlickerHal->pause();
        #endif
    }
        else
        {
            m_sAFInput.sAFStat = Trans4WintoOneStat(pAFStatBuf);
            #ifdef FLKPAUS
            //MY_LOG("FLKResume\n");
            if(m_pFlickerHal)
                m_pFlickerHal->resume();
            #endif
        }
    }

    m_sAFFullStat = TransToFullStat(pAFStatBuf);

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->handleAF(m_sAFInput, m_sAFOutput);
    }    
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }

    if (m_pMcuDrv)  {        
         MY_LOG("[TH]%d %d[ISO]%d %d[moveMCU]%d [H]%lld [V]%lld [FL]%lld ",                                                          
         ISP_READ_BITS(m_pIspReg , CAM_AF_TH, AF_TH0),
         ISP_READ_BITS(m_pIspReg , CAM_AF_TH, AF_TH2),
         m_sAFInput.i4ISO,
         (MINT32)m_sAFInput.i8GSum,
         m_sAFOutput.i4AFPos,
         m_sAFInput.sAFStat.i8Stat24, 
         m_sAFInput.sAFStat.i8StatV,
         m_sAFInput.sAFStat.i8StatFL);        
         
         if(m_preMCUpos!=m_sAFOutput.i4AFPos)
        m_pMcuDrv->moveMCU(m_sAFOutput.i4AFPos);
         m_preMCUpos=m_sAFOutput.i4AFPos;
    }
    else   {
        MY_LOG("Null m_pMcuDrv\n");
    }


    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   
    {
        if (m_sAFOutput.sAFArea.sRect[0].i4Info != m_i4AFPreStatus)   
        {
            if (m_pAFCallBack)   
            {                
                MY_LOG("Callback notify [pre]%d [now]%d", m_i4AFPreStatus, m_sAFOutput.sAFArea.sRect[0].i4Info);
                m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, !m_sAFOutput.i4IsAFDone, 0, 0);
                m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
                m_pAFCallBack->doDataCb(I3ACallBack::eID_DATA_AF_FOCUSED, &m_sAFOutput.sAFArea, sizeof(m_sAFOutput.sAFArea));
            }
            m_i4AFPreStatus = m_sAFOutput.sAFArea.sRect[0].i4Info;
        }
    }
    else  
    {
        m_i4AFPreStatus = AF_MARK_NONE;
    }
    setAFWinTH(m_sAFOutput.sAFStatConfig);
    setGMR(m_sAFOutput.sAFStatConfig);


    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)   
    {
        if (m_sAFOutput.i4IsMonitorFV == TRUE)   
        setFlkWinConfig();
        else   
        setAFWinConfig(m_sAFOutput.sAFArea);
    }
    else    //AFS mode or else
    {
        m_sAFOutput.i4FDDetect=0;
        if(m_sAFOutput.i4IsAFDone==0)  //Doing AF
        setAFWinConfig(m_sAFOutput.sAFArea);
        else    setFlkWinConfig();

        if(m_sAFParam.i4AFC_MODE==2)
            m_tcaf_mode=1;
    }
    
    // set AF info
    AF_INFO_T sAFInfo;
    sAFInfo.i4AFPos = m_sAFOutput.i4AFPos;

    IspTuningMgr::getInstance().setAFInfo(sAFInfo);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setFlkWinConfig()
{
	m_flkwin_syncflag  = 2;
    if (!m_pIspReg)   {
        MY_LOG("[setFlkWinConfig] m_pIspReg NULL");
        return;
    }
    MY_LOG_IF(m_bDebugEnable, "[setFlkWinConfig]\n");

    MINT32 i4WOri = ((((m_i4AF_in_Hsize-(WIN_POS_MIN*2))/6)>>1)<<1);
    MINT32 i4HOri = ((((m_i4AF_in_Vsize-(WIN_POS_MIN*2))/6)>>1)<<1);

    MINT32 i4W = Boundary(WIN_SIZE_MIN, i4WOri, WIN_SIZE_MAX);
    MINT32 i4H = Boundary(WIN_SIZE_MIN, i4HOri, WIN_SIZE_MAX);
    MINT32 i4X = WIN_POS_MIN;
    MINT32 i4Y = WIN_POS_MIN;
    MINT32 wintmp;


    if( (m_PreAFArea.sRect[0].i4X !=i4X) ||( m_PreAFArea.sRect[0].i4W !=i4W) 
     || (m_PreAFArea.sRect[0].i4Y !=i4Y) || (m_PreAFArea.sRect[0].i4H !=i4H))
    {
        m_PreAFArea.sRect[0].i4X =i4X;
        m_PreAFArea.sRect[0].i4W =i4W; 
        m_PreAFArea.sRect[0].i4Y =i4Y;
        m_PreAFArea.sRect[0].i4H =i4H; 

    // 13 bits (8192x8192) - double buffer, "must even position"
        wintmp=(Boundary(WIN_POS_MIN, i4X,            m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4WOri  , m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX01,  wintmp );
        wintmp=(Boundary(WIN_POS_MIN, i4X + i4WOri*2, m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4WOri*3, m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX23, wintmp );
        wintmp=(Boundary(WIN_POS_MIN, i4X + i4WOri*4, m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4WOri*5, m_i4AF_in_Hsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX45, wintmp );

        wintmp=(Boundary(WIN_POS_MIN, i4Y,            m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4HOri*2 - i4H, m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY01,wintmp);
        wintmp=(Boundary(WIN_POS_MIN, i4Y + i4HOri*2, m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4HOri*2 + i4H, m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY23,wintmp);
        wintmp=(Boundary(WIN_POS_MIN, i4Y + i4HOri*4, m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4HOri*5, m_i4AF_in_Vsize-WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY45,wintmp);

        // 10 bits (1022x1022) - double buffer
        ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE, (i4W & 0x3FF) + ((i4H&0x3FF)<<16) );
    }

    if (m_sAFOutput.i4FDDetect==0)
    {
        // 13 bits (8192x8192) - double buffer, "must even position"
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WIN_E, (Boundary(FL_WIN_POS_MIN, i4X + i4WOri*2, m_i4AF_in_Hsize - FL_WIN_POS_MIN) & 0x1FFE) 
        + ((Boundary(FL_WIN_POS_MIN, i4Y + i4HOri*2, m_i4AF_in_Vsize - FL_WIN_POS_MIN)&0x1FFE)<<16) );
        // 12 bits (4096x4096) - double buffer
        ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE_E, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE) 
        + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
    }
    else
    {
        
        MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, m_i4AF_in_Hsize - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
        MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, m_i4AF_in_Vsize - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
        
        // 13 bits (8192x8192) - double buffer, "must even position"
            ISP_WRITE_REG(m_pIspReg , CAM_AF_WIN_E, (i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );

        // 12 bits (4096x4096) - double buffer
            ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE_E, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, m_i4AF_in_Hsize - i4XE - FL_WIN_POS_MIN) & 0xFFE) 
            + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, m_i4AF_in_Vsize - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
    }
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFWinConfig(AF_AREA_T a_sAFArea)
{
    if(m_flkwin_syncflag)  m_flkwin_syncflag--;
    MY_LOG("[setAFWinConfig][cnt]%d[w]%d[h]%d[x]%d[y]%d\n", a_sAFArea.i4Count, a_sAFArea.sRect[0].i4W, a_sAFArea.sRect[0].i4H, a_sAFArea.sRect[0].i4X, a_sAFArea.sRect[0].i4Y);

    if ((a_sAFArea.i4Count != 1) || (a_sAFArea.sRect[0].i4W == 0) || (a_sAFArea.sRect[0].i4H == 0))   {return;}

    if (!m_pIspReg)   {
        MY_LOG("[setAFWinConfig] m_pIspReg NULL");
        return;
    }

    MINT32 i4W = Boundary(WIN_SIZE_MIN, (((a_sAFArea.sRect[0].i4W/6)>>1)<<1), WIN_SIZE_MAX);
    MINT32 i4H = Boundary(WIN_SIZE_MIN, (((a_sAFArea.sRect[0].i4H/6)>>1)<<1), WIN_SIZE_MAX);
    MINT32 i4X = Boundary(WIN_POS_MIN, (a_sAFArea.sRect[0].i4X >>1)<<1, m_i4AF_in_Hsize-WIN_POS_MIN-i4W*6);
    MINT32 i4Y = Boundary(WIN_POS_MIN, (a_sAFArea.sRect[0].i4Y >>1)<<1, m_i4AF_in_Vsize-WIN_POS_MIN-i4H*6);
    MINT32 wintmp;
    if(i4X + i4W*6 + WIN_POS_MIN >= m_i4AF_in_Hsize)
    {

        if((m_i4AF_in_Hsize - WIN_POS_MIN*2- i4X) >0)
            i4W = ((((m_i4AF_in_Hsize - WIN_POS_MIN*2- i4X)/6)>>1)<<1) ;
        else
        {    
            i4W = ((((m_i4AF_in_Hsize - WIN_POS_MIN*2)/6)>>1)<<1);
            i4X =WIN_POS_MIN;
        }
        
        MY_LOG("[setAFWinConfig] over Hsiz %d\n", i4X + i4W*6);
    }
    
    if(i4Y + i4H*6 + WIN_POS_MIN >= m_i4AF_in_Vsize)
    {
        if((m_i4AF_in_Vsize - WIN_POS_MIN*2- i4Y) >0)
            i4H = ((((m_i4AF_in_Vsize - WIN_POS_MIN*2- i4Y)/6)>>1)<<1) ;
        else
        {    
            i4H = ((((m_i4AF_in_Vsize - WIN_POS_MIN*2)/6)>>1)<<1);
            i4Y =WIN_POS_MIN;
        }

        MY_LOG("[setAFWinConfig]over Vsiz %d\n", i4Y + i4H*6);
    }

    if( (m_PreAFArea.sRect[0].i4X !=i4X) ||( m_PreAFArea.sRect[0].i4W !=i4W) 
     || (m_PreAFArea.sRect[0].i4Y !=i4Y) || (m_PreAFArea.sRect[0].i4H !=i4H))
    {
        m_PreAFArea.sRect[0].i4X =i4X;
        m_PreAFArea.sRect[0].i4W =i4W; 
        m_PreAFArea.sRect[0].i4Y =i4Y;
        m_PreAFArea.sRect[0].i4H =i4H; 

    // 13 bits (8192x8192) - double buffer, "must even position"
        wintmp= (i4X    &     0x1FFE) + (((i4X + i4W  )&0x1FFE)<<16) ;
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX01,  wintmp);
        wintmp=((i4X + i4W*2)&0x1FFE) + (((i4X + i4W*3)&0x1FFE)<<16) ;
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX23, wintmp);
        wintmp=((i4X + i4W*4)&0x1FFE) + (((i4X + i4W*5)&0x1FFE)<<16) ;
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINX45, wintmp );
        wintmp=  (i4Y    &     0x1FFE) + (((i4Y + i4H  )&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY01,  wintmp );
        wintmp= ((i4Y + i4H*2)&0x1FFE) + (((i4Y + i4H*3)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY23, wintmp );
        wintmp= ((i4Y + i4H*4)&0x1FFE) + (((i4Y + i4H*5)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WINY45, wintmp );

        // 10 bits (1022x1022) - double buffer
        wintmp=(i4W & 0x3FE) + ((i4H&0x3FE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE, wintmp );
        

    }
    MY_LOG("[AFWinFD]FDDet %d AFDone %d\n",m_sAFOutput.i4FDDetect,m_sAFOutput.i4IsAFDone );

    if (m_sAFOutput.i4FDDetect==0)
    {
        wintmp=(Boundary(FL_WIN_POS_MIN, i4X, m_i4AF_in_Hsize - FL_WIN_POS_MIN) & 0x1FFE) 
        + ((Boundary(FL_WIN_POS_MIN, i4Y, m_i4AF_in_Vsize - FL_WIN_POS_MIN)&0x1FFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_WIN_E,  wintmp);
        
        // 12 bits (4096x4096) - double buffer
        wintmp=(Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE) 
        + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE_E, wintmp );
    }
    else if (m_sAFOutput.i4IsAFDone==1)
    {
        MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, m_i4AF_in_Hsize - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
        MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, m_i4AF_in_Vsize - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
        
        // 13 bits (8192x8192) - double buffer, "must even position"
            ISP_WRITE_REG(m_pIspReg , CAM_AF_WIN_E, (i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );

        // 12 bits (4096x4096) - double buffer
            ISP_WRITE_REG(m_pIspReg , CAM_AF_SIZE_E, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, m_i4AF_in_Hsize - i4XE - FL_WIN_POS_MIN) & 0xFFE) 
            + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, m_i4AF_in_Vsize - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
    }
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAF_IN_HSIZE()
{
    if (!m_pIspReg)    {
        MY_LOG("[setAF_IN_HSIZE] m_pIspReg NULL");
        return;
    }

    MINT32 i4Pxl_S, i4Pxl_E, i4lin_S, i4lin_E;

    {
        i4Pxl_S = ISP_READ_BITS(m_pIspReg , CAM_TG_SEN_GRAB_PXL, PXL_S);
        i4Pxl_E = ISP_READ_BITS(m_pIspReg , CAM_TG_SEN_GRAB_PXL, PXL_E);
        i4lin_S = ISP_READ_BITS(m_pIspReg , CAM_TG_SEN_GRAB_LIN, LIN_S);
        i4lin_E = ISP_READ_BITS(m_pIspReg , CAM_TG_SEN_GRAB_LIN, LIN_E);
    }
    
    MINT32 i4Hrz_En = ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, HRZ_EN);
    //MINT32 i4Bin_En = ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, BIN_EN);
    //MINT32 i4Bin_Enable = ISP_READ_BITS(m_pIspReg , CAM_BIN_MODE, BIN_ENABLE);
    MINT32 i4SGG_Hrz_Sel = ISP_READ_BITS(m_pIspReg , CAM_CTL_SRAM_MUX_CFG, SGG_HRZ_SEL);   // 1: SGG after HRZ    

    m_i4AF_in_Vsize = i4lin_E - i4lin_S;
    
    if (i4SGG_Hrz_Sel && i4Hrz_En)  {
        m_i4AF_in_Hsize = ISP_READ_BITS(m_pIspReg , CAM_HRZ_OUT, HRZ_OUTSIZE);  // need confirm
    }
    else   {        
        m_i4AF_in_Hsize = i4Pxl_E - i4Pxl_S;    
    }

    ISP_WRITE_BITS(m_pIspReg , CAM_AF_IN_SIZE, AF_IN_HSIZE, Boundary(0, m_i4AF_in_Hsize, 8191));

    MY_LOG("[setAF_IN_HSIZE][SensorDev]%d [Pxl]%d [lin]%d [H]%d [V]%d\n", 
        m_i4CurrSensorDev,
        i4Pxl_E - i4Pxl_S,
        i4lin_E - i4lin_S,
        m_i4AF_in_Hsize,
        m_i4AF_in_Vsize);
    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFWinTH(AF_CONFIG_T a_sAFConfig)
{
    if (!m_pIspReg)    {
        MY_LOG("[setAFWinTH] m_pIspReg NULL");
        return;
    }

    // 8 bits
    if ((m_i4AF_TH[0] != a_sAFConfig.AF_TH[0]) || (m_i4AF_TH[2] != a_sAFConfig.AF_TH[2]))   {
        MY_LOG("[setAFWinTH0]%d [setAFWinTH2]%d\n", a_sAFConfig.AF_TH[0], a_sAFConfig.AF_TH[2]);
        ISP_WRITE_REG(m_pIspReg , CAM_AF_TH, 0x01000000+(Boundary(0, a_sAFConfig.AF_TH[2], 255)<<16)+ Boundary(0, a_sAFConfig.AF_TH[0], 255));
        m_i4AF_TH[0] = a_sAFConfig.AF_TH[0];
        m_i4AF_TH[2] = a_sAFConfig.AF_TH[2];        
    }

    // 8 bits
    if (m_i4AF_THEX != a_sAFConfig.AF_TH[0])   {
        MY_LOG("[setAFWinTHEX]%d \n", a_sAFConfig.AF_TH[0]+1);        
        ISP_WRITE_REG(m_pIspReg , CAM_AF_TH_E, Boundary(0, a_sAFConfig.AF_TH[0]+1, 255));
        m_i4AF_THEX = a_sAFConfig.AF_TH[0];
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setGMR(AF_CONFIG_T a_sAFConfig)
{
    if (!m_pIspReg)    {
        MY_LOG("[setGMR] m_pIspReg NULL");
        return;
    }

    // 8 bits
   // if ((m_i4GMR[0] != a_sAFConfig.i4SGG_GMR1) || (m_i4GMR[1] != a_sAFConfig.i4SGG_GMR2) || (m_i4GMR[2] != a_sAFConfig.i4SGG_GMR3))   {
        //MY_LOG("[setGMR]%d %d %d\n", a_sAFConfig.i4SGG_GMR1, a_sAFConfig.i4SGG_GMR2, a_sAFConfig.i4SGG_GMR3);
        //MY_LOG_IF(m_bDebugEnable,"[setGMR]%d %d %d [a_sAFConfig] %d %d %d", m_i4GMR[0], m_i4GMR[1], m_i4GMR[2], a_sAFConfig.i4SGG_GMR1, a_sAFConfig.i4SGG_GMR2, a_sAFConfig.i4SGG_GMR3);
        ISP_WRITE_REG(m_pIspReg , CAM_SGG_GMR, (Boundary(0, a_sAFConfig.i4SGG_GMR3, 255)<<16)+ (Boundary(0, a_sAFConfig.i4SGG_GMR2, 255)<<8) + Boundary(0, a_sAFConfig.i4SGG_GMR1, 255));

        m_i4GMR[0] = a_sAFConfig.i4SGG_GMR1;
        m_i4GMR[1] = a_sAFConfig.i4SGG_GMR2;
        m_i4GMR[2] = a_sAFConfig.i4SGG_GMR3;        
   // }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFConfig(AF_CONFIG_T a_sAFConfig)
{
    if (!m_pIspReg)    {
        MY_LOG("[setAFConfig] m_pIspReg NULL");
        return;
    }

    MY_LOG("[setAFConfig]\n");

    /*a_sAFConfig.AF_FILT1[0] = 12;
    a_sAFConfig.AF_FILT1[1] = 28;
    a_sAFConfig.AF_FILT1[2] = 33;
    a_sAFConfig.AF_FILT1[3] = 28;
    a_sAFConfig.AF_FILT1[4] = 17;
    a_sAFConfig.AF_FILT1[5] = 8;
    a_sAFConfig.AF_FILT1[6] = 2;*/


    MY_LOG_IF(m_bDebugEnable, "[SGG]%d [GMR]%d %d %d", a_sAFConfig.i4SGG_GAIN, a_sAFConfig.i4SGG_GMR1, a_sAFConfig.i4SGG_GMR2, a_sAFConfig.i4SGG_GMR3);

    MY_LOG_IF(m_bDebugEnable, "[f1]%d %d %d %d %d %d", a_sAFConfig.AF_FILT1[0],
        a_sAFConfig.AF_FILT1[1], a_sAFConfig.AF_FILT1[2], a_sAFConfig.AF_FILT1[3],
        a_sAFConfig.AF_FILT1[4], a_sAFConfig.AF_FILT1[5]);

    // SGG  
    MINT32 i4SGG_Sel_En = ISP_READ_BITS(m_pIspReg , CAM_CTL_MUX_SEL, SGG_SEL_EN);
    MINT32 i4SGG_Sel = ISP_READ_BITS(m_pIspReg , CAM_CTL_MUX_SEL, SGG_SEL);  // 0 : bin, 1 : before bin, 2 : lsc, 3 : imgi
    MINT32 i4SGG_Hrz_Sel = ISP_READ_BITS(m_pIspReg , CAM_CTL_SRAM_MUX_CFG, SGG_HRZ_SEL);   // 1: SGG after HRZ    
    MY_LOG_IF(m_bDebugEnable, "[SGG_Sel_En]%d [SGG_Sel]%d [SGG_Hrz_Sel]%d", i4SGG_Sel_En, i4SGG_Sel, i4SGG_Hrz_Sel);
    
    // Q0.7.4
    ISP_WRITE_BITS(m_pIspReg , CAM_SGG_PGN, SGG_GAIN, Boundary(0, a_sAFConfig.i4SGG_GAIN, 2047));
    // 8 bits
    //ISP_WRITE_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR1, Boundary(0, a_sAFConfig.i4SGG_GMR1, 255));    // 512
    //ISP_WRITE_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR2, Boundary(0, a_sAFConfig.i4SGG_GMR2, 255));    // 1024
    //ISP_WRITE_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR3, Boundary(0, a_sAFConfig.i4SGG_GMR3, 255));    // 2048

    // AF
    MINT32 i4TG_Pix_Id_En = ISP_READ_BITS(m_pIspReg , CAM_CTL_PIX_ID, TG_PIX_ID_EN);
    MINT32 i4TG_Pix_Id = ISP_READ_BITS(m_pIspReg , CAM_CTL_PIX_ID, TG_PIX_ID);   // 0:BGGR, 1:GBRG, 2GRBG, 3RGGB
    MINT32 i4Pix_Id = ISP_READ_BITS(m_pIspReg , CAM_CTL_PIX_ID, PIX_ID);   // 0:BGGR, 1:GBRG, 2GRBG, 3RGGB    
    MY_LOG_IF(m_bDebugEnable, "[TG_Pix_Id_En]%d [TG_Pix_Id]%d [Pix_Id]%d", i4TG_Pix_Id_En, i4TG_Pix_Id, i4Pix_Id);
    if (i4TG_Pix_Id_En == 1)    {
        i4Pix_Id = i4TG_Pix_Id;
    }
        
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_CON, AF_DECI_1, Boundary(0, a_sAFConfig.AF_DECI_1, 2));  // 1: 1/2 (double buffer)
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_CON, AF_ZIGZAG, Boundary(0, a_sAFConfig.AF_ZIGZAG, 1));  // 1: shift
    if ((i4Pix_Id == 1) || (i4Pix_Id == 2))   {
        ISP_WRITE_BITS(m_pIspReg , CAM_AF_CON, AF_ODD, 1);  // 1: odd
    }
    else   {
        ISP_WRITE_BITS(m_pIspReg , CAM_AF_CON, AF_ODD, 0);  //0: even
    }

  //  MY_LOG("[Filter-init]%d %d %d %d", a_sAFConfig.AF_FILT1[4],a_sAFConfig.AF_FILT1[5],a_sAFConfig.AF_FILT1[6],a_sAFConfig.AF_FILT1[7]);
  
    ISP_WRITE_REG(m_pIspReg , CAM_AF_FILT1_P14, (a_sAFConfig.AF_FILT1[3]<<24) + (a_sAFConfig.AF_FILT1[2]<<16) + (a_sAFConfig.AF_FILT1[1]<<8) + a_sAFConfig.AF_FILT1[0]);
    ISP_WRITE_REG(m_pIspReg , CAM_AF_FILT1_P58, (a_sAFConfig.AF_FILT1[7]<<24) + (a_sAFConfig.AF_FILT1[6]<<16) + (a_sAFConfig.AF_FILT1[5]<<8) + a_sAFConfig.AF_FILT1[4]);
    ISP_WRITE_REG(m_pIspReg , CAM_AF_FILT1_P912, (a_sAFConfig.AF_FILT1[11]<<24) + (a_sAFConfig.AF_FILT1[10]<<16) + (a_sAFConfig.AF_FILT1[9]<<8) + a_sAFConfig.AF_FILT1[8]);
    ISP_WRITE_REG(m_pIspReg , CAM_AF_VFILT_X01,(((a_sAFConfig.AF_FILT2[1]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[0]&0x7FF)));
    ISP_WRITE_REG(m_pIspReg , CAM_AF_VFILT_X23,(((a_sAFConfig.AF_FILT2[3]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[2]&0x7FF)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::printAFConfigLog0()
{
    if (!m_pIspReg)    {
        MY_LOG("[printAFConfigLog0] m_pIspReg NULL");
        return;
    }

    MINT32 i4EN = (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, SGG_EN)<<7)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, AF_EN)<<6)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, FLK_EN)<<5)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN2, EIS_EN)<<4)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, HRZ_EN)<<3)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_DMA_EN, ESFKO_EN)<<2);

    // monitor EIS / Flicker En
    MY_LOG("DoAF[sw]%x [XY]%d %d [Sz]%d %d  [G]%d %d %d [Hz]%d [WE|SE]%d %d %d %d [AFMode]%d [AE] %d, ", i4EN,
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX01, AF_WINX0),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY01, AF_WINY0),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE, AF_XSIZE),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE, AF_YSIZE),
                                                        ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR1),
                                                        ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR2),
                                                        ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR3),
                                                        ISP_READ_BITS(m_pIspReg , CAM_AF_IN_SIZE, AF_IN_HSIZE),
                                                        ISP_READ_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINXE),
                                                        ISP_READ_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINYE),
                                                        ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_XE),
                                                        ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_YE),
                                                        m_eLIB3A_AFMode, 
                                                        m_sAFInput.i4IsAEStable);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::printAFConfigLog1()
{
    if (!m_pIspReg)    {
        MY_LOG("[printAFConfigLog1] m_pIspReg NULL");
        return;
    }
    MY_LOG_IF(m_bDebugEnable,"[X1-5] %d %d %d %d %d [Y1-5] %d %d %d %d %d ",                                                                   
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX01, AF_WINX1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX23, AF_WINX2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX23, AF_WINX3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX45, AF_WINX4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX45, AF_WINX5),                                                     
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY01, AF_WINY1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY23, AF_WINY2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY23, AF_WINY3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY45, AF_WINY4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY45, AF_WINY5));


   /* MY_LOG_IF(m_bDebugEnable,"[F1]%d %d %d %d %d %d %d %d %d %d %d %d",
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P5),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P6),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P7),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P8),                                                     
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P9),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P10),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P11),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P12));*/

    MY_LOG_IF(m_bDebugEnable,"[THE]%d  [SGG_GN]%d [Deci]%d [Zig]%d [Odd]%d ",
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_TH_E, AF_TH0EX),
                                                         ISP_READ_BITS(m_pIspReg , CAM_SGG_PGN, SGG_GAIN),
                                                          ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_DECI_1),
                                                          ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_ZIGZAG),
                                                          ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_ODD));

    #if 0
    ISP_WRITE_BITS(m_pIspReg , CAM_CTL_DMA_INT, ESFKO_DONE_EN, 1);
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_DMA;
    WaitIrq.Status = ISP_DRV_IRQ_DMA_INT_ESFKO_DONE_ST;
    WaitIrq.Timeout = 1000; // 1 sec
    MINT32 ret = m_pIspDrv->waitIrq(WaitIrq);
    MY_LOG("[ESFKOdon IRQ]%d", ret);
    #endif

    #if 0
    ISP_WRITE_BITS(m_pIspReg , CAM_CTL_INT_EN, AF_DON_EN, 1);
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_INT;
    WaitIrq.Status = ISP_DRV_IRQ_INT_STATUS_AF_DON_ST;
    WaitIrq.Timeout = 1000; // 1 sec
    MINT32 ret = m_pIspDrv->waitIrq(WaitIrq);
    MY_LOG("[AFdon IRQ]%d", ret);
    #endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T AfMgr::TransToFullStat(MVOID *pAFStatBuf)
{    
    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);
    AF_FULL_STAT_T sAFStat;

    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)   {        
        if (m_sAFOutput.i4IsMonitorFV == TRUE)   {
            sAFStat.i8StatH[i] = ((((MINT64)pAFStat->sStat[i].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;        
            sAFStat.i8StatV[i] = 0;//pAFStat->sStat[i].u4StatV&0x3FFFFFF;
        }
        else   {
            sAFStat.i8StatH[i] = 0;
            sAFStat.i8StatV[i] = 0;
        }
    }
    return sAFStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_STAT_T AfMgr::Trans4WintoOneStat(MVOID *pAFStatBuf)
{
    AF_STAT_T sAFStat;
    MINT64 i8Stat24 = 0;
    MINT64 i8StatV = 0;
    MINT32 i4posx = 2;
    MINT32 i4posy = 2;

    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);

    sAFStat.i8Stat24 = 0;
    sAFStat.i8StatV  = 0;

    if(m_tcaf_mode && !m_sAFOutput.i4FDDetect)    {
        i4posx = m_sAFOutput.sAFArea.sRect[0].i4X -m_sAFInput.sEZoom.i4X+ (m_sAFOutput.sAFArea.sRect[0].i4W>>1);
        i4posy = m_sAFOutput.sAFArea.sRect[0].i4Y -m_sAFInput.sEZoom.i4Y+ (m_sAFOutput.sAFArea.sRect[0].i4H>>1);
        if (i4posx > m_sAFOutput.sAFArea.sRect[0].i4W/12+FL_WIN_POS_MIN) i4posx-= (m_sAFOutput.sAFArea.sRect[0].i4W/12+FL_WIN_POS_MIN);
        else i4posx=0;
        if (i4posy > m_sAFOutput.sAFArea.sRect[0].i4H/12+FL_WIN_POS_MIN) i4posy-= (m_sAFOutput.sAFArea.sRect[0].i4H/12+FL_WIN_POS_MIN);
        else i4posy=0;
        i4posx = i4posx/ (m_sAFInput.sEZoom.i4W/6);
        i4posy = i4posy/ (m_sAFInput.sEZoom.i4H/6);
        i4posx = Boundary(0,i4posx,4);
        i4posy = Boundary(0,i4posy,4);
    }
    //MY_LOG("posxy %d %d %d %d %d %d\n",(i4posx + i4posy*6),(i4posx+1+ i4posy*6),(i4posx+6+ i4posy*6),(i4posx+7+ i4posy*6), i4posx , i4posy);

    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)   {
        // --- H24 ---
        if (i==(i4posx + i4posy*6) || i==(i4posx+1+ i4posy*6) || i==(i4posx+6+ i4posy*6) || i==(i4posx+7+ i4posy*6)){
            i8Stat24 = ((((MINT64)pAFStat->sStat[i].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;
            sAFStat.i8Stat24 += i8Stat24;

            // --- V ---
            i8StatV = pAFStat->sStat[i].u4StatV&0x3FFFFFF;
            sAFStat.i8StatV += i8StatV;
            
            
            /*
            i8StatV = pAFStat->sStat[i].u4StatV&0x3FFFFFF;

            if (i8StatV >= m_i8PreVStat[i])   {sAFStat.i8StatV = sAFStat.i8StatV + i8StatV - m_i8PreVStat[i];}
            else  {
                sAFStat.i8StatV = sAFStat.i8StatV + i8StatV + 0x3FFFFFF - m_i8PreVStat[i];
            }
            m_i8PreVStat[i] = i8StatV;
            */
            //MY_LOG("%d [Ori24]%d [V]%d", i, pAFStat->sStat[i].u4Stat24, pAFStat->sStat[i].u4StatV);
            //MY_LOG("%d %lld[AFStat24]%lld, %lld[AFStatV]%lld", i, i8Stat24, sAFStat.i8Stat24, i8StatV, sAFStat.i8StatV);        
        }
    }

    //i8StatV = sAFStat.i8StatV;
    //if (sAFStat.i8StatV >= m_i8PreVStat)   {sAFStat.i8StatV -= m_i8PreVStat;}
    //else {
    //    sAFStat.i8StatV = sAFStat.i8StatV + 0x1FFFFFF - m_i8PreVStat;
    //}    
    //m_i8PreVStat = i8StatV;
    
    // --- Floating ---
    sAFStat.i8StatFL = (((MINT64)(pAFStat->sStat[36].u4StatV)&0xFF)<<32)+ (MINT64)pAFStat->sStat[36].u4Stat24;
    //MY_LOG("36 [Ori24]%d [V]%d", pAFStat->sStat[36].u4Stat24, pAFStat->sStat[36].u4StatV);

    if ((m_sAFOutput.i4FDDetect) && (m_FDArea.i4Count))    {
        sAFStat.i8Stat24 = sAFStat.i8StatFL;
        sAFStat.i8StatV = sAFStat.i8StatFL;
    }

    //MY_LOG("[AFStatH]%lld [AFStatFL]%lld [AFStatV]%lld", sAFStat.i8Stat24, sAFStat.i8StatFL, sAFStat.i8StatV);

    return sAFStat;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_STAT_T AfMgr::TransAFtoOneStat(MVOID *pAFStatBuf)
{
    AF_STAT_T sAFStat;
    MINT64 i8Stat24 = 0;
    MINT64 i8StatV = 0;

    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);

    sAFStat.i8Stat24 = 0;
    sAFStat.i8StatV  = 0;

    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)   {
        // --- H24 ---
        i8Stat24 = ((((MINT64)(pAFStat->sStat[i].u4StatV>>28))&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;
        sAFStat.i8Stat24 += i8Stat24;

        // --- V ---
        
        i8StatV = pAFStat->sStat[i].u4StatV&0x3FFFFFF;
        sAFStat.i8StatV += i8StatV;

        //MY_LOG("%d [Ori24]%d [V]%d", i, pAFStat->sStat[i].u4Stat24, pAFStat->sStat[i].u4StatV);
        //MY_LOG("%d %lld[AFStat24]%lld, %lld[AFStatV]%lld", i, i8Stat24, sAFStat.i8Stat24, i8StatV, sAFStat.i8StatV);        
    }

    /*MY_LOG("[V]%d %d %d %d %d %d %d %d %d %d %d %d",
                                   pAFStat->sStat[6].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[7].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[8].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[9].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[10].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[11].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[12].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[13].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[14].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[15].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[16].u4StatV&0x1FFFFFF
                                 , pAFStat->sStat[17].u4StatV&0x1FFFFFF);*/

    // --- Floating ---
    sAFStat.i8StatFL = (((MINT64)(pAFStat->sStat[36].u4StatV)&0xFF)<<32)+ (MINT64)pAFStat->sStat[36].u4Stat24;
    //MY_LOG("36 [Ori24]%d [V]%d", pAFStat->sStat[36].u4Stat24, pAFStat->sStat[36].u4StatV);

    if ((m_sAFOutput.i4FDDetect) && (m_FDArea.i4Count))   {
        sAFStat.i8Stat24 = sAFStat.i8StatFL;
        sAFStat.i8StatV = sAFStat.i8StatFL;
    }

    //MY_LOG("[AFStatH]%lld [AFStat32]%lld [AFStatFL]%lld [AFStatV]%lld", sAFStat.i8Stat24, i8Stat32, sAFStat.i8StatFL, sAFStat.i8StatV);

    return sAFStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max)
{
    if (a_i4Max < a_i4Min)  {a_i4Max = a_i4Min;}
    if (a_i4Vlu < a_i4Min)  {a_i4Vlu = a_i4Min;}
    if (a_i4Vlu > a_i4Max)  {a_i4Vlu = a_i4Max;}

    return a_i4Vlu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::enableAF(MINT32 a_i4En)
{
    MY_LOG("[enableAF]%d\n", a_i4En);
    m_i4EnableAF = a_i4En;
    return S_AF_OK;
}
MBOOL AfMgr::isAFavailable()
{
    //MY_LOG("[isAFavailable] %d\n", m_i4EnableAF);
    return m_i4EnableAF;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocusFinish()
{
    return m_sAFOutput.i4IsAFDone;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocused()
{
    return m_sAFOutput.i4IsFocused;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo)
{   
    if (m_pIAfAlgo)  {
        return m_pIAfAlgo->getDebugInfo(rAFDebugInfo);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");        
        return E_AF_NULL_POINTER;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getLensInfo(LENS_INFO_T &a_rLensInfo)
{
    MINT32 err = S_AF_OK;
    mcuMotorInfo rMotorInfo;

    if (m_pMcuDrv) {
        err = m_pMcuDrv->getMCUInfo(&rMotorInfo);

        a_rLensInfo.bIsMotorMoving = rMotorInfo.bIsMotorMoving;
        a_rLensInfo.bIsMotorOpen   = rMotorInfo.bIsMotorOpen;
        a_rLensInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        a_rLensInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        a_rLensInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        a_rLensInfo.bIsSupportSR   = rMotorInfo.bIsSupportSR;
    }

    return err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setCallbacks(I3ACallBack* cb)
{
    m_pAFCallBack = cb;
    return TRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SingleAF_CallbackNotify()
{
    m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
    //m_pAFCallBack->doDataCb(I3ACallBack::eID_DATA_AF_FOCUSED, &m_sAFOutput.sAFArea, sizeof(m_sAFOutput.sAFArea));        

    m_i4AutoFocus = FALSE;

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setBestShotConfig()
{
    MY_LOG("[setBestShotConfig]");

    setAF_IN_HSIZE();

    MINT32 i4WE = Boundary(FL_WIN_SIZE_MIN, (((m_i4AF_in_Hsize>>2)*3)>>1)<<1, FL_WIN_SIZE_MAX);
    MINT32 i4HE = Boundary(FL_WIN_SIZE_MIN, (((m_i4AF_in_Vsize>>2)*3)>>1)<<1, FL_WIN_SIZE_MAX);
    MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_i4AF_in_Hsize-i4WE)>>1, m_i4AF_in_Hsize - i4WE - FL_WIN_POS_MIN);
    MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_i4AF_in_Vsize-i4WE)>>1, m_i4AF_in_Vsize - i4HE - FL_WIN_POS_MIN);
    m_PreAFArea.sRect[0].i4X =0;
    m_PreAFArea.sRect[0].i4W =0; 
    m_PreAFArea.sRect[0].i4Y =0;
    m_PreAFArea.sRect[0].i4H =0; 
    m_PreFDArea.sRect[0].i4X =0;
    m_PreFDArea.sRect[0].i4Y =0; 
    m_PreFDArea.sRect[0].i4W =0;
    m_PreFDArea.sRect[0].i4H =0; 

    // setAFFloatingWinConfig
    // 13 bits (8192x8192) - double buffer, "must even position"
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINXE, i4XE);
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINYE, i4YE);
    // 12 bits (4096x4096) - double buffer
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_XE, i4WE);
    ISP_WRITE_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_YE, i4HE);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::calBestShotValue(MVOID *pAFStatBuf)
{
    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);

#if 0
    MINT32 i4EN = (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, SGG_EN)<<7)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, AF_EN)<<6)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, FLK_EN)<<5)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN2, EIS_EN)<<4)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_EN1, HRZ_EN)<<3)+
                  (ISP_READ_BITS(m_pIspReg , CAM_CTL_DMA_EN, ESFKO_EN)<<2);

    // monitor EIS / Flicker En
    MY_LOG("DoAF[SGG|AF|FK|EIS|HRZ|ESFKO]%x [X]%d %d %d %d %d %d [Y]%d %d %d %d %d %d [Size]%d %d [TH]%d %d", i4EN,
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX01, AF_WINX0),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX01, AF_WINX1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX23, AF_WINX2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX23, AF_WINX3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX45, AF_WINX4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINX45, AF_WINX5),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY01, AF_WINY0),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY01, AF_WINY1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY23, AF_WINY2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY23, AF_WINY3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY45, AF_WINY4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_WINY45, AF_WINY5),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE, AF_XSIZE),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE, AF_YSIZE),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_TH, AF_TH0),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_TH, AF_TH2));
    
    MY_LOG("[SGG_GN]%d [GMR1]%d [GMR2]%d [GMR3]%d [Deci]%d [Zig]%d [Odd]%d [Hsize]%d",      
                                                         ISP_READ_BITS(m_pIspReg , CAM_SGG_PGN, SGG_GAIN),
                                                         ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_SGG_GMR, SGG_GMR3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_DECI_1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_ZIGZAG),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_CON, AF_ODD),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_IN_SIZE, AF_IN_HSIZE));

    MY_LOG("[F1]%d %d %d %d %d %d %d %d %d %d %d %d",
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P1),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P2),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P3),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P14, AF_FILT1_P4),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P5),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P6),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P7),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P58, AF_FILT1_P8),                                                     
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P9),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P10),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P11),
                                                         ISP_READ_BITS(m_pIspReg , CAM_AF_FILT1_P912, AF_FILT1_P12));

    MY_LOG("[WinXE]%d [WinYE]%d [SizeXE]%d [SizeYE]%d [THE]%d",
                                                     ISP_READ_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINXE),
                                                     ISP_READ_BITS(m_pIspReg , CAM_AF_WIN_E, AF_WINYE),
                                                     ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_XE),
                                                     ISP_READ_BITS(m_pIspReg , CAM_AF_SIZE_E, AF_SIZE_YE),
                                                     ISP_READ_BITS(m_pIspReg , CAM_AF_TH_E, AF_TH0EX));

    MY_LOG("36 [Ori24]%d [V]%d", pAFStat->sStat[36].u4Stat24, pAFStat->sStat[36].u4StatV);
#endif

    m_i8BSSVlu = (((MINT64)(pAFStat->sStat[36].u4StatV)&0x3F)<<32) + (MINT64)pAFStat->sStat[36].u4Stat24;
    MY_LOG("[calBestShotValue] %lld", m_i8BSSVlu);
   
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::getBestShotValue()
{
    return m_i8BSSVlu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::readOTP()
{
    MUINT32 result=0; 
    CAM_CAL_DATA_STRUCT GetCamCalData; 
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance(); 
    MINT32 i4SensorDevID;

    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum = CAMERA_CAM_CAL_DATA_3A_GAIN;

    switch (m_i4CurrSensorDev) 
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;        
    case ESensorDev_MainSecond:  
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        return S_AWB_OK;
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }   

    result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);      
    MY_LOG("(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);
    
    if (result&CamCalReturnErr[enCamCalEnum])
    {
        MY_LOG("err (%s)", CamCalErrString[enCamCalEnum]); 
        return E_AF_NOSUPPORT;
    }

    MY_LOG("OTP data [S2aBitEn]%d [S2aAfBitflagEn]%d [S2aAf0]%d [S2aAf1]%d", GetCamCalData.Single2A.S2aBitEn
                                                                   , GetCamCalData.Single2A.S2aAfBitflagEn
                                                                   ,  GetCamCalData.Single2A.S2aAf[0]
                                                                   ,  GetCamCalData.Single2A.S2aAf[1]);

    MINT32 i4InfPos, i4MacroPos;
    if (GetCamCalData.Single2A.S2aBitEn & 0x1)   {

        i4InfPos = GetCamCalData.Single2A.S2aAf[0];

        if (GetCamCalData.Single2A.S2aBitEn & 0x2)   {

            i4MacroPos = GetCamCalData.Single2A.S2aAf[1];

            if (i4MacroPos < i4InfPos)   {
                MY_LOG("OTP abnormal return [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);
                return S_AF_OK;
            }
        }
        else   {
            i4MacroPos = 0;
        }

        MY_LOG("OTP [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);

        if (m_pIAfAlgo)   {
            m_pIAfAlgo->updateAFtableBoundary(i4InfPos, i4MacroPos);
        }
    }

    return S_AF_OK;
}

// AF v1.2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* This update function should be called before m_pIAfAlgo->setAFParam */
MRESULT AfMgr::updateSceneChangeParams()
{              
    UINT32 Level = m_NVRAM_LENS.rAFNVRAM.i4Coef[7];    
    if(0 == Level)
    {   // disable new part and use org only
        m_NVRAM_LENS.rAFNVRAM.i4Coef[8]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[9]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[10]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[11]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[12]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[13]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[14]=0;
        m_NVRAM_LENS.rAFNVRAM.i4Coef[15]=0;        
    }    
    else if((1<=Level)&&(Level<=3))
    {   // level 1~3
        UINT32 levelIdx = Level-1;
                
        //-- org part --        
        // normal
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4FV_CHANGE_THRES += SCENE_FV_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4FV_CHANGE_CNT   += SCENE_FV_CHG_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4FV_STABLE_THRES += SCENE_FV_STB_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4FV_STABLE_CNT   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4FV_STABLE_NUM   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_THRES += SCENE_GS_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_CNT   += SCENE_GS_CHG_CNT_LEVEL_DIFF[levelIdx];
        // zsd               
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4FV_CHANGE_THRES += SCENE_FV_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4FV_CHANGE_CNT   += SCENE_FV_CHG_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4FV_STABLE_THRES += SCENE_FV_STB_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4FV_STABLE_CNT   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4FV_STABLE_NUM   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4GS_CHANGE_THRES += SCENE_GS_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.i4GS_CHANGE_CNT   += SCENE_GS_CHG_CNT_LEVEL_DIFF[levelIdx];
        // video
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4FV_CHANGE_THRES += SCENE_FV_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4FV_CHANGE_CNT   += SCENE_FV_CHG_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4FV_STABLE_THRES += SCENE_FV_STB_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4FV_STABLE_CNT   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4FV_STABLE_NUM   += SCENE_FV_STB_CNT_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_THRES += SCENE_GS_CHG_THR_LEVEL_DIFF[levelIdx];
        m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_CNT   += SCENE_GS_CHG_CNT_LEVEL_DIFF[levelIdx];
       
        //-- new part --
        // encode param to i4Coef[8]/[9] format: chgT|chgN|stbT|stbN = ooxxooxx
        UINT32 GS_CHG_PARAM          = 1000000*(m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_THRES) \
                                       + 10000*(m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_CNT%100);
        UINT32 AEBLOCK_CHG_PARAM     = 1000000*(m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_THRES+SCENE_AEBLOCK_CHG_THR_DIFF) \
                                       + 10000*((m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4GS_CHANGE_CNT+SCENE_AEBLOCK_CHG_CNT_DIFF)%100);
        UINT32 GS_VID_CHG_PARAM      = 1000000*(m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_THRES) \
                                       + 10000*(m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_CNT%100);
        UINT32 AEBLOCK_VID_CHG_PARAM = 1000000*(m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_THRES+SCENE_AEBLOCK_CHG_THR_DIFF) \
                                       + 10000*((m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.i4GS_CHANGE_CNT+SCENE_AEBLOCK_CHG_CNT_DIFF)%100);
        // normal
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[8])
            m_NVRAM_LENS.rAFNVRAM.i4Coef[8]   = GS_CHG_PARAM + SCENE_GS_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[9])        
            m_NVRAM_LENS.rAFNVRAM.i4Coef[9]   = AEBLOCK_CHG_PARAM + SCENE_AEBLOCK_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[10])
            m_NVRAM_LENS.rAFNVRAM.i4Coef[10]  = SCENE_GYRO_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[11])        
            m_NVRAM_LENS.rAFNVRAM.i4Coef[11]  = SCENE_ACCE_LEVEL[levelIdx];
        // video
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[12])                
            m_NVRAM_LENS.rAFNVRAM.i4Coef[12]   = GS_VID_CHG_PARAM + SCENE_GS_VID_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[13])        
            m_NVRAM_LENS.rAFNVRAM.i4Coef[13]   = AEBLOCK_VID_CHG_PARAM + SCENE_AEBLOCK_VID_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[14])                
            m_NVRAM_LENS.rAFNVRAM.i4Coef[14]  = SCENE_GYRO_VID_LEVEL[levelIdx];
        if(0 == m_NVRAM_LENS.rAFNVRAM.i4Coef[15])        
            m_NVRAM_LENS.rAFNVRAM.i4Coef[15]  = SCENE_ACCE_VID_LEVEL[levelIdx];                     
    } 

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::autoFocus()
{    MY_LOG("autofocus %d", m_eLIB3A_AFMode);
    if ((m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC) && (m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC_VIDEO))   {
    m_i4AutoFocus = TRUE;
    }
    else  
        m_i4AutoFocuscb = TRUE;
}
MVOID AfMgr::DoCallback()
{
    if(m_pAFCallBack!=NULL && m_i4AutoFocuscb==TRUE)
    {
        m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, !m_sAFOutput.i4IsAFDone, 0, 0);
        m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
        MY_LOG("autofocus callback in conti mode %d", m_eLIB3A_AFMode);
        m_i4AutoFocuscb = FALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    if ((m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC) && (m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC_VIDEO))   {
    m_i4AutoFocus = FALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::TimeOutHandle()
{
    if (m_i4AutoFocus == TRUE)   {
        MY_LOG("timeout callback");        
        SingleAF_CallbackNotify();
        m_i4AutoFocus = FALSE;
    }
}
MVOID AfMgr::setAndroidServiceState(MBOOL a_state)
{
    m_AndroidServiceState=a_state;
}

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::pauseFocus()
{
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::resetFocus()
{
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setFocusPos(MINT32 a_i4Pos)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getFocusPos()
{
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::getFocusValue()
{
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFResultPos()
{
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFocusDistanceRange(MINT32 a_i4Distance_N, MINT32 a_i4Distance_M)
{
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getFocusDistance(MINT32 &a_i4Near, MINT32 &a_i4Curr, MINT32 &a_i4Far)
{
    return S_AF_OK;
}

#endif

