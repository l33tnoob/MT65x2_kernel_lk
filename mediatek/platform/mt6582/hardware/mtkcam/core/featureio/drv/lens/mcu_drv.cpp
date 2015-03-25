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

#define LOG_TAG "LensMCU"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset 
#include <stdio.h> //sprintf
#include <cutils/log.h>

//
#include "MediaTypes.h"
#include "mcu_drv.h"
#include "lens_drv.h"
#include "lens_sensor_drv.h"
#include <mtkcam/hal/sensor_hal.h>

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define MCU_DRV_DBG(fmt, arg...) ALOGD(LOG_TAG fmt, ##arg)
#define MCU_DRV_ERR(fmt, arg...) ALOGE(LOG_TAG "Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define MCU_DRV_DBG(a,...)
#define MCU_DRV_ERR(a,...)
#endif

MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc[MAX_NUM_OF_SUPPORT_LENS] = {{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL}};
unsigned int  MCUDrv::m_u4CurrLensIdx = 0;

//extern function in camera_custom_msdk.h
//FIXME 
extern UINT32 LensCustomInit();
extern UINT32 LensCustomSetIndex(UINT32 a_u4CurrIdx); 
extern UINT32 LensCustomGetInitFunc(MSDK_LENS_INIT_FUNCTION_STRUCT *a_pLensInitFunc); 

/*******************************************************************************
*
********************************************************************************/
MCUDrv::~MCUDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
MCUDrv*   
MCUDrv::createInstance(unsigned int a_u4CurrLensId)
{
    if (a_u4CurrLensId == SENSOR_DRIVE_LENS_ID)
    {
        return LensSensorDrv::getInstance();
    }
    else
    {
        return LensDrv::getInstance();
    }
}


/*******************************************************************************
*
********************************************************************************/
int 
MCUDrv::lensSearch(
    unsigned int a_u4CurrSensorDev,
    unsigned int a_u4CurrSensorId
)
{
    INT32 i;

    MCU_DRV_DBG("lensSearch() - Entry \n"); 
    MCU_DRV_DBG("[CurrSensorDev]0x%04x [CurrSensorId]0x%04x\n", a_u4CurrSensorDev, a_u4CurrSensorId);

    LensCustomInit();
    LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc[0]);

    MCU_DRV_DBG("[LensInitTable-0][SensorId]0x%04x,[LensId]0x%04x\n", MCUDrv::m_LensInitFunc[0].SensorId, MCUDrv::m_LensInitFunc[0].LensId);
    MCU_DRV_DBG("[LensInitTable-1][SensorId]0x%04x,[LensId]0x%04x\n", MCUDrv::m_LensInitFunc[1].SensorId, MCUDrv::m_LensInitFunc[1].LensId);
    MCU_DRV_DBG("[LensInitTable-2][SensorId]0x%04x,[LensId]0x%04x\n", MCUDrv::m_LensInitFunc[2].SensorId, MCUDrv::m_LensInitFunc[2].LensId);    
    MCU_DRV_DBG("[LensInitTable-3][SensorId]0x%04x,[LensId]0x%04x\n", MCUDrv::m_LensInitFunc[3].SensorId, MCUDrv::m_LensInitFunc[3].LensId);

    MCUDrv::m_u4CurrLensIdx = 0;

    if (a_u4CurrSensorDev == SENSOR_DEV_MAIN)   // only search for main sensor, dummy for sub sensor always -> need modify for support 3D
    {
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++) {
            if ((MCUDrv::m_LensInitFunc[i].LensId == DUMMY_LENS_ID) ||
                (MCUDrv::m_LensInitFunc[i].LensId == SENSOR_DRIVE_LENS_ID) ||
                (MCUDrv::m_LensInitFunc[i].LensId == FM50AF_LENS_ID)
                )
            {
                MCUDrv::m_u4CurrLensIdx = i;
            }
        }

        // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++) {
            if ((MCUDrv::m_LensInitFunc[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                MCUDrv::m_u4CurrLensIdx = i;
                MCU_DRV_DBG("[idx]%d [CurrSensorId]0x%04x,[CurrLensIdx]0x%04x\n", i, a_u4CurrSensorId, MCUDrv::m_u4CurrLensIdx);                
                break;
            }
        }
    }
    else //sub or main2 case
    {
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++) {
            if ((MCUDrv::m_LensInitFunc[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                MCUDrv::m_u4CurrLensIdx = i;
                MCU_DRV_DBG("[idx]%d [CurrSensorId]0x%04x,[CurrLensIdx]0x%04x\n", i, a_u4CurrSensorId, MCUDrv::m_u4CurrLensIdx);                
                break;
            }
        }
    }

    LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx); 

    MCU_DRV_DBG("[CurrLensIdx]%d", MCUDrv::m_u4CurrLensIdx);

    return MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::getCurrLensID()
{
    return MCUDrv::m_LensInitFunc[MCUDrv::m_u4CurrLensIdx].LensId;
}

