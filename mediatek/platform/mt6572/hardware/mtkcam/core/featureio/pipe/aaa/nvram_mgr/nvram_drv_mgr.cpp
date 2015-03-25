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
#define LOG_TAG "nvram_drv_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <nvram_drv.h>
#include <mtkcam/hal/sensor_hal.h>
#include <isp_tuning.h>
#include "nvram_drv_mgr.h"

using namespace NS3A;
using namespace NSIspTuning;

/*******************************************************************************
* NVRAM Driver Manager Context
*******************************************************************************/
class NvramDrvMgrCtx : public NvramDrvMgr
{
    friend class NvramDrvMgr;
protected:
    NvramDrvMgrCtx();
    ~NvramDrvMgrCtx();

protected:  ////    Data Members.
    NvramDrvBase*                   m_pNvramDrv;

    MUINT32                         m_u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM  m_eSensorEnum;

public:     ////    Interfaces.

    virtual MRESULT     init(
        MINT32 const i4SensorDev
    );

    virtual MRESULT     uninit();

private:    ////
    template <class Buf_T>
    Buf_T*  getRefBuf() const
    {
        NSNvram::BufIF<Buf_T>*const pBufIF = m_pNvramDrv->getBufIF<Buf_T>();
        if  ( ! pBufIF )
        {
            return  NULL;
        }
        return  pBufIF->getRefBuf(m_eSensorEnum, m_u4SensorID);
    }

public:     ////    Interfaces.

    virtual MVOID   getRefBuf(NVRAM_CAMERA_ISP_PARAM_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_CAMERA_SHADING_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_CAMERA_3A_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(AE_PLINETABLE_T*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_LENS_PARA_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_CAMERA_STROBE_STRUCT*& rpBuf) const;

};


NvramDrvMgr&
NvramDrvMgr::
getInstance()
{
    static NvramDrvMgrCtx singleton;
    return singleton;
}


NvramDrvMgrCtx::
NvramDrvMgrCtx()
    : NvramDrvMgr()
    , m_pNvramDrv(NULL)
    , m_u4SensorID(0)
    , m_eSensorEnum(DUAL_CAMERA_NONE_SENSOR)
{
}


NvramDrvMgrCtx::
~NvramDrvMgrCtx()
{
    uninit();
}


MRESULT
NvramDrvMgrCtx::
init(MINT32 const i4SensorDev)
{
    MRESULT err = S_NVRAM_OK;

    //  Sensor driver.
    SensorHal*const pSensorHal = SensorHal::createInstance();
    if  ( ! pSensorHal )
    {
        MY_ERR("Cannot create Sensor driver");
        err = E_NVRAM_BAD_SENSOR_DRV;
        goto lbExit;
    }

    //  Query sensor ID & sensor enum.
    switch  ( i4SensorDev )
    {
    case ESensorDev_Main:
        m_eSensorEnum = DUAL_CAMERA_MAIN_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&m_u4SensorID), 0, 0);
        break;
    case ESensorDev_Sub:
        m_eSensorEnum = DUAL_CAMERA_SUB_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&m_u4SensorID), 0, 0);
        break;
    case ESensorDev_MainSecond:
        m_eSensorEnum = DUAL_CAMERA_MAIN_SECOND_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&m_u4SensorID), 0, 0);
        break;
    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", i4SensorDev);
        err = E_NVRAM_BAD_PARAM;
        goto lbExit;
    }

    //  Nvram driver.
    if  ( ! m_pNvramDrv )
        m_pNvramDrv = NvramDrvBase::createInstance();
    if  ( ! m_pNvramDrv )
    {
        MY_ERR("Cannnot create NVRAM driver");
        err = E_NVRAM_BAD_NVRAM_DRV;
        goto lbExit;
    }

lbExit:
    if  ( pSensorHal )
        pSensorHal->destroyInstance();

    return  err;
}


MRESULT
NvramDrvMgrCtx::
uninit()
{
    if  ( m_pNvramDrv )
    {
        m_pNvramDrv->destroyInstance();
        m_pNvramDrv = NULL;
    }

    m_u4SensorID = 0;
    m_eSensorEnum = DUAL_CAMERA_NONE_SENSOR;

    return  S_NVRAM_OK;
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_ISP_PARAM_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_ISP_PARAM_STRUCT>();
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_SHADING_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_SHADING_STRUCT>();
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_3A_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_3A_STRUCT>();
}

MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_STROBE_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_STROBE_STRUCT>();
}

MVOID
NvramDrvMgrCtx::
getRefBuf(AE_PLINETABLE_T*& rpBuf) const
{
    rpBuf = getRefBuf<AE_PLINETABLE_T>();
}

MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_LENS_PARA_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_LENS_PARA_STRUCT>();
}

