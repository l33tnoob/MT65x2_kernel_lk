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

#define LOG_TAG "LensSensorDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset 
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include "mcu_drv.h"
#include "lens_sensor_drv.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"

#define DEBUG_LENS_SENSOR_DRV
#ifdef DEBUG_LENS_SENSOR_DRV
#define DRV_DBG(fmt, arg...) ALOGD(LOG_TAG fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE(LOG_TAG "Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif





/*******************************************************************************
*
********************************************************************************/
MCUDrv*
LensSensorDrv::getInstance()
{
    DRV_DBG("[ImgSensorDrv] getInstance \n");
    static LensSensorDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LensSensorDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LensSensorDrv::LensSensorDrv()
    : MCUDrv()
    , m_fdMCU(-1)
    , m_userCnt(0)
    , m_i4FocusPos(0)
{  
    DRV_DBG("[LensSensorDrv() construct]\n");
}

/*******************************************************************************
*
********************************************************************************/
LensSensorDrv::~LensSensorDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::init(
)
{
    DRV_DBG("[init()]\n"); 
    char  cBuf[64];
    int err = 0; 
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MUINT16 FeaturePara = 0;
    MUINT32 FeatureParaLen = 0;

    sprintf(cBuf, "/dev/%s", MCUDrv::m_LensInitFunc[MCUDrv::m_u4CurrLensIdx].LensDrvName);

    DRV_DBG("[Lens Driver]%s\n", cBuf);

    Mutex::Autolock lock(mLock);
    
    if (m_userCnt == 0) {
        if (m_fdMCU == -1) {
            m_fdMCU = open(cBuf, O_RDWR);
            if (m_fdMCU < 0) {
                DRV_ERR("error opening %s: %s", cBuf, strerror(errno));
                return MCUDrv::MCU_INVALID_DRIVER;
            }           

            featureCtrl.FeatureId = SENSOR_FEATURE_INITIALIZE_AF;
            featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
            featureCtrl.pFeatureParaLen = &FeatureParaLen;
            
            err = ioctl(m_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
            if (err < 0) {
                DRV_ERR("[initMCU] ioctl - SENSOR_FEATURE_INITIALIZE_AF, error %s",  strerror(errno));
            }            
        }
    }
    m_userCnt++;     

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::uninit(
)
{
    DRV_DBG("[uninit()]\n"); 
    
    Mutex::Autolock lock(mLock);
    
    if (m_userCnt == 1) {    	
        if (m_fdMCU > 0) {
            close(m_fdMCU);
        }        
        m_fdMCU = -1;
    }
    m_userCnt --;

    if (m_userCnt < 0)   {m_userCnt = 0;}

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::moveMCU(int a_i4FocusPos
)
{
    //DRV_DBG("moveMCU() - pos = %d \n", a_i4FocusPos); 	
    int err = 0; 
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;    
    MUINT16 FeaturePara = 0;
    MUINT32 FeatureParaLen = 0;
    
    if (m_fdMCU < 0) {
    	DRV_ERR("moveMCU() invalid m_fdMCU =%d\n", m_fdMCU); 
    	return MCUDrv::MCU_INVALID_DRIVER;
    }

    FeaturePara = (MUINT16)a_i4FocusPos;
    FeatureParaLen = sizeof(MUINT16);
    
    featureCtrl.FeatureId = SENSOR_FEATURE_MOVE_FOCUS_LENS;
    featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
    featureCtrl.pFeatureParaLen = &FeatureParaLen;
    
    err = ioctl(m_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    
    if (err < 0) {
        DRV_ERR("[moveMCU] ioctl - SENSOR_FEATURE_MOVE_FOCUS_LENS, error %s",  strerror(errno));
        return err; 
    }

    m_i4FocusPos = a_i4FocusPos;
    
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo
)
{
    int err = 0; 
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;    
    MUINT32 FeaturePara = 0;
    MUINT32 FeatureParaLen = 4;


    if (m_fdMCU < 0) {
        DRV_ERR("getMCUInfo() invalid m_fdMCU =%d\n", m_fdMCU);
        a_pMotorInfo->bIsMotorOpen = 0;    
        return MCUDrv::MCU_INVALID_DRIVER;
    }
        
    a_pMotorInfo->bIsMotorOpen = 1;
    a_pMotorInfo->u4CurrentPosition = m_i4FocusPos; 

    a_pMotorInfo->bIsSupportSR = 0;
    
    featureCtrl.pFeaturePara = (MUINT8*)&FeaturePara;
    featureCtrl.pFeatureParaLen = &FeatureParaLen;

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
    err = ioctl(m_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);        
    a_pMotorInfo->bIsMotorMoving = (bool)FeaturePara; 

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_INF;
    err = ioctl(m_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);        
    a_pMotorInfo->u4InfPosition = (MUINT32)FeaturePara; 

    featureCtrl.FeatureId = SENSOR_FEATURE_GET_AF_MACRO;
    err = ioctl(m_fdMCU, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);            
    a_pMotorInfo->u4MacroPosition = (MUINT32)FeaturePara; 

    //DRV_DBG("[ImgSensorDrv] [state]%d, [inf]%d, [macro]%d \n", a_pMotorInfo->bIsMotorMoving, a_pMotorInfo->u4InfPosition, a_pMotorInfo->u4MacroPosition);

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::setMCUInfPos(int a_i4FocusPos
)
{
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensSensorDrv::setMCUMacroPos(int a_i4FocusPos
)
{
    return MCUDrv::MCU_NO_ERROR;
}

