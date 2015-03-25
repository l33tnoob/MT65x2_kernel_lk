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
#ifndef _AAA_SENSOR_MGR_H_
#define _AAA_SENSOR_MGR_H_

namespace NS3A
{

typedef struct
{
  MUINT16 u2SensorPreviewWidth;
  MUINT16 u2SensorPreviewHeight;
  MUINT16 u2SensorFullWidth;
  MUINT16 u2SensorFullHeight;
  MUINT16 u2SensorVideoWidth;
  MUINT16 u2SensorVideoHeight;
} SENSOR_RESOLUTION_INFO_T;

/*******************************************************************************
*
*******************************************************************************/
class AAASensorMgr
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AAASensorMgr(AAASensorMgr const&);
    //  Copy-assignment operator is disallowed.
    AAASensorMgr& operator=(AAASensorMgr const&);

public:  ////
    AAASensorMgr();
    ~AAASensorMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AAASensorMgr& getInstance();
    MRESULT init();
    MRESULT uninit();
    MRESULT setSensorDev(MINT32 i4SensorDev);
    MRESULT getSensorSyncinfo(MINT32 *i4SutterDelay, MINT32 *i4SensorGainDelay, MINT32 *i4IspGainDelay);
    MRESULT getSensorWidthHeight(MINT32 i4SensorDev, SENSOR_RESOLUTION_INFO_T* a_rSensorResolution);
    MRESULT setSensorExpTime(MUINT32 a_u4ExpTime);
    MUINT32 getSensorExpTime();
    MRESULT setSensorGain(MUINT32 a_u4SensorGain);
    MUINT32 getSensorGain();
    MRESULT setSensorFrameRate(MUINT32 a_u4SensorFrameRate);
    MUINT32 getSensorFrameRate();
    MRESULT setSensorExpLine(MUINT32 a_u4ExpLine);
    MRESULT setFlickerFrameRateActive(MBOOL a_bFlickerFPSAvtive);
    MUINT32 getFlickerFrameRateActive();
    MRESULT setSensorParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain, MUINT32 a_u4RawGain);
    MRESULT setPreviewParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);
    MRESULT setCaptureParams(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);
    MRESULT setPreviewLineBaseParams(MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain);
    MRESULT setCaptureLineBaseParams(MUINT32 a_u4ExpLine, MUINT32 a_u4SensorGain);

    MRESULT setPreviewMaxFrameRate(MUINT32 frameRate, MBOOL a_bIsZsd);
    MUINT32 getPreviewDefaultFrameRate(MBOOL a_bIsZsd);
private:
    MINT32 getSensorDelayFrame(halSensorDev_e eSensorDEv, MINT32 mode);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    SensorHal *m_pSensorHalObj;
    halSensorDev_e m_eSensorDevId;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL    m_bDebugEnable;
    MBOOL    m_bFlickerState;
    MUINT32 m_u4ExpTime;
    MUINT32 m_u4SensorGain;
    MUINT32 m_u4SensorFrameRate;
    MINT32 m_i4ShutterDelayFrame;
    MINT32 m_i4SensorGainDelayFrame;
    MINT32 m_i4IspGainDelayFrame;
};

};  //  namespace NS3A
#endif // _AAA_SENSOR_MGR_H_

