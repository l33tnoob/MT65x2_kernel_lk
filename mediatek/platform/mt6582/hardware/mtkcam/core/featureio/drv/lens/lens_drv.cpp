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

#define LOG_TAG "LensDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset 
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include "mcu_drv.h"
#include "lens_drv.h"
#include "FM50AF.h"

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif





/*******************************************************************************
*
********************************************************************************/
MCUDrv*
LensDrv::getInstance()
{
    static LensDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LensDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::LensDrv()
    : MCUDrv()
    , m_fdMCU(-1)
    , m_userCnt(0)
{
    DRV_DBG("LensDrv()\n");
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::~LensDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::init(
)
{
    DRV_DBG("init() [m_userCnt]%d\n", m_userCnt); 
    char  cBuf[64];

    sprintf(cBuf, "/dev/%s", MCUDrv::m_LensInitFunc[MCUDrv::m_u4CurrLensIdx].LensDrvName);

    DRV_DBG("[Lens Driver]%s\n", cBuf);
    
    Mutex::Autolock lock(mLock);

    if (m_userCnt == 0) {        
        if (m_fdMCU == -1) {
            m_fdMCU = open(cBuf, O_RDWR);
            if (m_fdMCU < 0) {
                if (MCUDrv::m_u4CurrLensIdx == 0)  {  // no error log for dummy lens
                    return MCUDrv::MCU_NO_ERROR;
                }
                else  {
                    DRV_ERR("error opening %s: %s", cBuf, strerror(errno));                    
                    return MCUDrv::MCU_INVALID_DRIVER;
                }
            }
        }
    }
    m_userCnt++;     

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::uninit(
)
{
    DRV_DBG("uninit() [m_userCnt]%d + \n", m_userCnt); 
      
    Mutex::Autolock lock(mLock);
    
    if (m_userCnt == 1) {    	
        if (m_fdMCU > 0) {
        				
            close(m_fdMCU);
        }        
        m_fdMCU = -1;
    }
    m_userCnt --;

    if (m_userCnt < 0)   {m_userCnt = 0;}

    DRV_DBG("uninit() - \n"); 
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::moveMCU(int a_i4FocusPos
)
{
    //DRV_DBG("moveMCU() - pos = %d \n", a_i4FocusPos); 	
    int err; 
    if (m_fdMCU < 0) {
        
        if (MCUDrv::m_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
    	    DRV_ERR("[moveMCU] invalid m_fdMCU =%d\n", m_fdMCU); 
        	return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(m_fdMCU,FM50AFIOC_T_MOVETO,(unsigned long)a_i4FocusPos);
    if (err < 0) {
        DRV_ERR("[moveMCU] ioctl - FM50AFIOC_T_MOVETO, error %s",  strerror(errno));
        return err; 
    }
        
    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo
)
{
    //DRV_DBG("getMCUInfo() - E \n"); 	
    int err; 
    stFM50AF_MotorInfo motorInfo; 
    memset(&motorInfo, 0, sizeof(stFM50AF_MotorInfo)); 
    
    if (m_fdMCU < 0) {

        if (MCUDrv::m_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            a_pMotorInfo->bIsMotorOpen = 0;        
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {        
            DRV_ERR("[getMCUInfo] invalid m_fdMCU =%d\n", m_fdMCU);
            a_pMotorInfo->bIsMotorOpen = 0;    
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(m_fdMCU,FM50AFIOC_G_MOTORINFO, &motorInfo);
    if (err < 0) {
        DRV_ERR("[getMCUInfo] ioctl - FM50AFIOC_G_MOTORINFO, error %s",  strerror(errno));
        return err;         
    }
        
    a_pMotorInfo->bIsMotorOpen = 1;
    a_pMotorInfo->bIsMotorMoving = motorInfo.bIsMotorMoving; 
    a_pMotorInfo->u4CurrentPosition = motorInfo.u4CurrentPosition; 
    a_pMotorInfo->u4MacroPosition = motorInfo.u4MacroPosition; 
    a_pMotorInfo->u4InfPosition = motorInfo.u4InfPosition; 
    a_pMotorInfo->bIsSupportSR = motorInfo.bIsSupportSR;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUInfPos(int a_i4FocusPos
)
{
    DRV_DBG("setMCUInfPos() - pos = %d \n", a_i4FocusPos ); 	
    int err; 
    if (m_fdMCU < 0) {

        if (MCUDrv::m_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {        
        	DRV_ERR("[setMCUInfPos] invalid m_fdMCU =%d\n", m_fdMCU);
        	return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(m_fdMCU,FM50AFIOC_T_SETINFPOS,(unsigned long)a_i4FocusPos);
    if (err  < 0) {
        DRV_ERR("[setMCUInfPos] ioctl - FM50AFIOC_T_SETINFPOS, error %s",  strerror(errno));
        return err;             
    }
        

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUMacroPos(int a_i4FocusPos
)
{
    DRV_DBG("setMCUMacroPos() - pos = %d \n", a_i4FocusPos); 	
    int err; 
    if (m_fdMCU < 0) {
        if (MCUDrv::m_u4CurrLensIdx == 0)  {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else  {
            DRV_ERR("[setMCUMacroPos] invalid m_fdMCU =%d\n", m_fdMCU);
        	return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(m_fdMCU,FM50AFIOC_T_SETMACROPOS,(unsigned long)a_i4FocusPos);
    if (err < 0) {
        DRV_ERR("[setMCUMacroPos] ioctl - FM50AFIOC_T_SETMACROPOS, error %s",  strerror(errno));
        return err;          
    }

    return MCUDrv::MCU_NO_ERROR;
}

