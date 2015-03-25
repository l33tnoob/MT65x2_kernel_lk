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
#define LOG_TAG "isp_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>

#include "isp_mgr.h"
#include <mtkcam/featureio/tdri_mgr.h>

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Manager Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_BASE::
dumpRegInfo()
{
    RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

    for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
        MY_LOG("[addr] 0x%8x [value] 0x%8x\n", pRegInfo[i].addr, pRegInfo[i].val);
    }
}

MVOID
ISP_MGR_BASE::
dumpRegs()
{
    RegInfo_T RegInfo[m_u4RegInfoNum];

    readRegs(ISPDRV_MODE_ISP, RegInfo, m_u4RegInfoNum);

    for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
        MY_LOG("[addr] 0x%8x [value] 0x%8x\n", RegInfo[i].addr, RegInfo[i].val);
    }
}

MVOID
ISP_MGR_BASE::
addressErrorCheck(char const*const ptestCastName)
{
    RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

    if (m_u4StartAddr != pRegInfo[0].addr) {
        MY_ERR("[%s] Start address check error: (m_u4StartAddr, pRegInfo[0].addr) = (0x%8x, 0x%8x)", ptestCastName, m_u4StartAddr, pRegInfo[0].addr);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM_CTL_EN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CTL_EN_T&
ISP_MGR_CTL_EN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CTL_EN_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CTL_EN_T&
ISP_MGR_CTL_EN_T::
get(ISP_NVRAM_CTL_EN_T& rParam)
{
    GET_REG_INFO(CAM_CTL_EN1, en1);
    GET_REG_INFO(CAM_CTL_EN2, en2);

    return  (*this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_OBC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_OBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
put(ISP_NVRAM_OBC_T const& rParam)
{
    PUT_REG_INFO(CAM_OBC_OFFST0, offst0);
    PUT_REG_INFO(CAM_OBC_OFFST1, offst1);
    PUT_REG_INFO(CAM_OBC_OFFST2, offst2);
    PUT_REG_INFO(CAM_OBC_OFFST3, offst3);
    PUT_REG_INFO(CAM_OBC_GAIN0, gain0);
    PUT_REG_INFO(CAM_OBC_GAIN1, gain1);
    PUT_REG_INFO(CAM_OBC_GAIN2, gain2);
    PUT_REG_INFO(CAM_OBC_GAIN3, gain3);
    return  (*this);
}


template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
get(ISP_NVRAM_OBC_T& rParam)
{
    GET_REG_INFO(CAM_OBC_OFFST0, offst0);
    GET_REG_INFO(CAM_OBC_OFFST1, offst1);
    GET_REG_INFO(CAM_OBC_OFFST2, offst2);
    GET_REG_INFO(CAM_OBC_OFFST3, offst3);
    GET_REG_INFO(CAM_OBC_GAIN0, gain0);
    GET_REG_INFO(CAM_OBC_GAIN1, gain1);
    GET_REG_INFO(CAM_OBC_GAIN2, gain2);
    GET_REG_INFO(CAM_OBC_GAIN3, gain3);
    return  (*this);
}

MBOOL
ISP_MGR_OBC_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_OBC_T::apply()");

    // Merge AE gain
    MUINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 * m_u4IspAEGain + 256) >> 9;
    MUINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 * m_u4IspAEGain + 256) >> 9;
    MUINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 * m_u4IspAEGain + 256) >> 9;
    MUINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 * m_u4IspAEGain + 256) >> 9;

    reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBGAIN0 = CLAMP(OBGAIN0, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBGAIN1 = CLAMP(OBGAIN1, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBGAIN2 = CLAMP(OBGAIN2, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBGAIN3 = CLAMP(OBGAIN3, 0, 8191);

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_OBC, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (isEnable()) {
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, OB_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, OB_EN_SET, 1);
        } else {
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, OB_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, OB_EN_CLR, 1);
        }

    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_OBC);
        writeRegs(CAM_ISP_OBC, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (isEnable()){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, OB_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, OB_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, OB_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, OB_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_OBC);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_OBC);
        writeRegs(CAM_ISP_OBC, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (isEnable()){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, OB_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, OB_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, OB_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, OB_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_OBC);
    }

    addressErrorCheck("After ISP_MGR_OBC_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BPC_T const& rParam)
{
    PUT_REG_INFO(CAM_BPC_CON, con);
    PUT_REG_INFO(CAM_BPC_CD1_1, cd1_1);
    PUT_REG_INFO(CAM_BPC_CD1_2, cd1_2);
    PUT_REG_INFO(CAM_BPC_CD1_3, cd1_3);
    PUT_REG_INFO(CAM_BPC_CD1_4, cd1_4);
    PUT_REG_INFO(CAM_BPC_CD1_5, cd1_5);
    PUT_REG_INFO(CAM_BPC_CD1_6, cd1_6);
    PUT_REG_INFO(CAM_BPC_CD2_1, cd2_1);
    PUT_REG_INFO(CAM_BPC_CD2_2, cd2_2);
    PUT_REG_INFO(CAM_BPC_CD2_3, cd2_3);
    PUT_REG_INFO(CAM_BPC_CD0, cd0);
    PUT_REG_INFO(CAM_BPC_COR, cor);
    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BPC_T& rParam)
{
    GET_REG_INFO(CAM_BPC_CON, con);
    GET_REG_INFO(CAM_BPC_CD1_1, cd1_1);
    GET_REG_INFO(CAM_BPC_CD1_2, cd1_2);
    GET_REG_INFO(CAM_BPC_CD1_3, cd1_3);
    GET_REG_INFO(CAM_BPC_CD1_4, cd1_4);
    GET_REG_INFO(CAM_BPC_CD1_5, cd1_5);
    GET_REG_INFO(CAM_BPC_CD1_6, cd1_6);
    GET_REG_INFO(CAM_BPC_CD2_1, cd2_1);
    GET_REG_INFO(CAM_BPC_CD2_2, cd2_2);
    GET_REG_INFO(CAM_BPC_CD2_3, cd2_3);
    GET_REG_INFO(CAM_BPC_CD0, cd0);
    GET_REG_INFO(CAM_BPC_COR, cor);

    return  (*this);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_NR1_T const& rParam)
{
    PUT_REG_INFO(CAM_NR1_CON, con);
    PUT_REG_INFO(CAM_NR1_CT_CON, ct_con);

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_NR1_T& rParam)
{
    GET_REG_INFO(CAM_NR1_CON, con);
    GET_REG_INFO(CAM_NR1_CT_CON, ct_con);

    return  (*this);
}

MBOOL
ISP_MGR_BNR_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_BNR_T::apply()");

    MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE & isBPCEnable();
    MBOOL bCT_EN = reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN & isCTEnable();
    MBOOL bBNR_EN = bBPC_EN | bCT_EN;

    reinterpret_cast<ISP_CAM_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BPC_CON))->BPC_ENABLE = bBPC_EN;
    reinterpret_cast<ISP_CAM_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_NR1_CON))->NR1_CT_EN = bCT_EN;

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_BNR, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        // Set enable bit
        if (bBNR_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, BNR_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, BNR_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, BNR_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, BNR_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_BPC_CON, BPC_ENABLE, bBPC_EN);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_NR1_CON, NR1_CT_EN, bCT_EN);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setBnr(ISP_DRV_CQ01_SYNC, bBNR_EN, bBPC_EN);
        writeRegs(CAM_ISP_BNR, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (bBNR_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, BNR_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, BNR_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, BNR_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, BNR_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_BPC_CON, BPC_ENABLE, isBPCEnable());
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_NR1_CON, NR1_CT_EN, isCTEnable());
        //ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_DMA_EN_CLR, IMGCI_EN_CLR, 1);

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_BNR);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setBnr(ISP_DRV_CQ02_SYNC, bBNR_EN, bBPC_EN);
        writeRegs(CAM_ISP_BNR, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        // Set enable bit
        if (bBNR_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, BNR_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, BNR_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, BNR_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, BNR_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_BPC_CON, BPC_ENABLE, isBPCEnable());
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_NR1_CON, NR1_CT_EN, isCTEnable());
        //ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_DMA_EN_CLR, IMGCI_EN_CLR, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_BNR);
    }

    addressErrorCheck("After ISP_MGR_BNR_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_LSC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_LSC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_LSC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_LSC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
put(ISP_NVRAM_LSC_T const& rParam)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s\n", __FUNCTION__);
    PUT_REG_INFO(CAM_CTL_DMA_EN, lsci_en);
    PUT_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    PUT_REG_INFO(CAM_LSCI_XSIZE, xsize);
    PUT_REG_INFO(CAM_CTL_EN1, lsc_en);
    PUT_REG_INFO(CAM_LSC_CTL1, ctl1);
    PUT_REG_INFO(CAM_LSC_CTL2, ctl2);
    PUT_REG_INFO(CAM_LSC_CTL3, ctl3);
    PUT_REG_INFO(CAM_LSC_LBLOCK, lblock);
    PUT_REG_INFO(CAM_LSC_RATIO, ratio);
    //PUT_REG_INFO(CAM_LSC_GAIN_TH, gain_th);
    REG_INFO_VALUE(CAM_LSC_GAIN_TH) = 0x1FF00000;

    return  (*this);
}


template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(ISP_NVRAM_LSC_T& rParam)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s\n", __FUNCTION__);
    GET_REG_INFO(CAM_CTL_DMA_EN, lsci_en);
    GET_REG_INFO(CAM_LSCI_BASE_ADDR, baseaddr);
    GET_REG_INFO(CAM_LSCI_XSIZE, xsize);
    GET_REG_INFO(CAM_CTL_EN1, lsc_en);
    GET_REG_INFO(CAM_LSC_CTL1, ctl1);
    GET_REG_INFO(CAM_LSC_CTL2, ctl2);
    GET_REG_INFO(CAM_LSC_CTL3, ctl3);
    GET_REG_INFO(CAM_LSC_LBLOCK, lblock);
    GET_REG_INFO(CAM_LSC_RATIO, ratio);
    GET_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}

template <>
ISP_MGR_LSC_T&
ISP_MGR_LSC_T::
get(ISP_NVRAM_LSC_CFG_T& rParam)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s\n", __FUNCTION__);
    GET_REG_INFO(CAM_LSC_CTL1, ctl1);
    GET_REG_INFO(CAM_LSC_CTL2, ctl2);
    GET_REG_INFO(CAM_LSC_CTL3, ctl3);
    GET_REG_INFO(CAM_LSC_LBLOCK, lblock);
    GET_REG_INFO(CAM_LSC_RATIO, ratio);
    GET_REG_INFO(CAM_LSC_GAIN_TH, gain_th);

    return  (*this);
}


#define LSC_DIRECT_ACCESS 0
#define EN_WRITE_REGS     0
MBOOL
ISP_MGR_LSC_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_LSC_T::apply()");

    ISPDRV_MODE_T drv_mode;
    MUINT32 XNum, YNum, Width, Height;
    MUINT32 LSCI_OFFSET, LSCI_XSIZE, LSCI_YSIZE, LSCI_STRIDE, LSC_RATIO, LSC_GAIN_TH, LSC_EN;
    
    MY_LOG_IF(ENABLE_MY_LOG,"[%s] create IspDrv \n", __FUNCTION__);
    IspDrv* m_pIspDrv = IspDrv::createInstance();

    XNum = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM;
    YNum = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM;
    Width = reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH;
    Height = reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT;
    MY_LOG_IF(ENABLE_MY_LOG,
            "[%s] IspProfile %d ---------\n"
            " XNum: %d \n"
            " YNum: %d \n"
            " Width: %d \n"
            " Height: %d \n"
            " Address: 0x%08x \n",
            __FUNCTION__,
            eIspProfile,
            XNum,
            YNum,
            Width,
            Height,
            reinterpret_cast<ISP_CAM_LSCI_BA*>(REG_INFO_VALUE_PTR(CAM_LSCI_BASE_ADDR))->BASE_ADDR);

    MY_LOG_IF(ENABLE_MY_LOG,"[%s] IspDrv:0x%08x \n", __FUNCTION__, m_pIspDrv);

    LSCI_OFFSET = 0;
    LSCI_XSIZE = (XNum+1)*4*128/8 - 1;
    LSCI_YSIZE = YNum;
    LSCI_STRIDE = (LSCI_XSIZE+1);
    LSC_RATIO = 0x20202020;
    LSC_GAIN_TH = 0x00000000;

    LSC_EN = reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->LSC_EN;

    MY_LOG_IF(ENABLE_MY_LOG,"%s\n", __FUNCTION__);
    //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_LSCI_ERR_STAT, ERR_STAT, 0xffff);

    if ((XNum == 0 && YNum == 0) || (Width == 0 || Height == 0)) {
        MY_LOG_IF(ENABLE_MY_LOG,"%s Invalid parameter \n", __FUNCTION__);
        return MTRUE;
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        drv_mode = ISPDRV_MODE_CQ0; //ISPDRV_MODE_ISP;//

        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_ISP_LSC);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_DMA_LSCI);

        // defaults
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_OFST, 0x0);
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_SIZE, 0x0);
#if 0
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08161620); // ultra-high
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00161600);
#else //ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08000010); // ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00000000);
#endif
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE, 0x0   );

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_OFST_ADDR, OFFSET_ADDR, LSCI_OFFSET);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE, LSCI_XSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE, LSCI_YSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE, LSCI_STRIDE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR, BASE_ADDR ,
                reinterpret_cast<ISP_CAM_LSCI_BA*>(REG_INFO_VALUE_PTR(CAM_LSCI_BASE_ADDR))->BASE_ADDR);

        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_CTL1, 0x10000000);

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_WIDTH,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_HEIGHT,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT);//sdblk_height,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_XNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM);//sdblk_xnum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_YNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM);//sdblk_ynum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lWIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lHEIGHT);


        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO11,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO11);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO10,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO10);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO01,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO01);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO00,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO00);

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH2,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH2);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH1,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH1);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH0,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH0);


        if (LSC_EN) {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 1);
        } else {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 1);
        }

        if (LSC_EN) {
            m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_LSCI);
            m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_ISP_LSC);
        } else {
            m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_ISP_LSC);
            m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_LSCI);
        }

        MY_LOG_IF(ENABLE_MY_LOG,"%s ISPDRV_MODE_CQ0 addr 0x%0x (LSC_EN, LSCI_EN) = \n"
                "(%d, %d)\n", __FUNCTION__,
                getIspReg(drv_mode),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET));
#if 0
        MY_LOG_IF(ENABLE_MY_LOG,"%s ISPDRV_MODE_CQ0 (XSize, YSize, Stride, ERR_STAT, LWidth, LHeight) = \n"
                "(%d, %d, %d, %d, %d, %d)\n", __FUNCTION__,
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE),
                ISP_READ_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_LSCI_ERR_STAT, ERR_STAT),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT));
#endif
        MY_LOG_IF(ENABLE_MY_LOG, "%s ISPDRV_MODE_CQ0 \n"
            "CAM_LSCI_BASE_ADDR = 0x%08x\n"
            "CAM_LSCI_XSIZE     = 0x%08x\n"
            "CAM_LSCI_YSIZE     = 0x%08x\n"
            "CAM_LSCI_STRIDE    = 0x%08x\n"
            "CAM_LSCI_CON       = 0x%08x\n"
            "CAM_LSCI_CON2      = 0x%08x\n"
            "CAM_LSC_CTL1       = 0x%08x\n"
            "CAM_LSC_CTL2       = 0x%08x\n"
            "CAM_LSC_CTL3       = 0x%08x\n"
            "CAM_LSC_LBLOCK     = 0x%08x\n"
            "CAM_LSC_RATIO      = 0x%08x\n"
            "CAM_LSC_GAIN_TH    = 0x%08x\n", __FUNCTION__,
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_XSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_YSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL1),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL3),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_LBLOCK),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_RATIO),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_GAIN_TH));
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        drv_mode = ISPDRV_MODE_CQ1_SYNC;

        TdriMgr::getInstance().setLsc(ISP_DRV_CQ01_SYNC,
                LSC_EN,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH, //sdblk_width,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM,//sdblk_xnum,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lWIDTH,//sdblk_last_width,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT,//sdblk_height,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM,//sdblk_ynum,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lHEIGHT,
                LSC_EN,
                ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE));//sdblk_last_height);


        // defaults
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_OFST, 0x0);
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_SIZE, 0x0);
#if 0
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08161620); // ultra-high
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00161600);
#else //ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08000010); // ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00000000);
#endif        
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE, 0x0   );

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_OFST_ADDR, OFFSET_ADDR, LSCI_OFFSET);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE, LSCI_XSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE, LSCI_YSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE, LSCI_STRIDE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR, BASE_ADDR ,
                reinterpret_cast<ISP_CAM_LSCI_BA*>(REG_INFO_VALUE_PTR(CAM_LSCI_BASE_ADDR))->BASE_ADDR);

        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_CTL1, 0x10000000);
        
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_WIDTH,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_HEIGHT,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT);//sdblk_height,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_XNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM);//sdblk_xnum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_YNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM);//sdblk_ynum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lWIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lHEIGHT);


        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO11,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO11);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO10,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO10);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO01,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO01);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO00,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO00);

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH2,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH2);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH1,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH1);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH0,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH0);


        if (LSC_EN) {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 1);
        } else {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_LSC);

        MY_LOG_IF(ENABLE_MY_LOG,"%s ISPDRV_MODE_CQ1 addr 0x%0x (LSC_EN, LSCI_EN) = \n"
                "(%d, %d)\n", __FUNCTION__,
                getIspReg(drv_mode),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET));
#if 0
        MY_LOG_IF(ENABLE_MY_LOG,"%s ISP_DRV_CQ1_SYNC (XSize, YSize, Stride, ERR_STAT, LWidth, LHeight) = \n"
                "(%d, %d, %d, %d, %d, %d)\n", __FUNCTION__,
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE),
                ISP_READ_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_LSCI_ERR_STAT, ERR_STAT),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT)
        );
#endif
        MY_LOG_IF(ENABLE_MY_LOG, "%s ISP_DRV_CQ1_SYNC \n"
            "CAM_LSCI_BASE_ADDR = 0x%08x\n"
            "CAM_LSCI_XSIZE     = 0x%08x\n"
            "CAM_LSCI_YSIZE     = 0x%08x\n"
            "CAM_LSCI_STRIDE    = 0x%08x\n"
            "CAM_LSCI_CON       = 0x%08x\n"
            "CAM_LSCI_CON2      = 0x%08x\n"
            "CAM_LSC_CTL1       = 0x%08x\n"
            "CAM_LSC_CTL2       = 0x%08x\n"
            "CAM_LSC_CTL3       = 0x%08x\n"
            "CAM_LSC_LBLOCK     = 0x%08x\n"
            "CAM_LSC_RATIO      = 0x%08x\n"
            "CAM_LSC_GAIN_TH    = 0x%08x\n", __FUNCTION__,
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_XSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_YSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL1),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL3),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_LBLOCK),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_RATIO),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_GAIN_TH));

    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        drv_mode = ISPDRV_MODE_CQ2_SYNC;

        TdriMgr::getInstance().setLsc(ISP_DRV_CQ02_SYNC,
                LSC_EN,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH, //sdblk_width,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM,//sdblk_xnum,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lWIDTH,//sdblk_last_width,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT,//sdblk_height,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM,//sdblk_ynum,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lHEIGHT,
                LSC_EN,
                ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE));//sdblk_last_height);

        // defaults
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_OFST, 0x0);
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_TILE_SIZE, 0x0);
#if 0
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08161620); // ultra-high
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00161600);
#else //ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON, 0x08000010); // ultra-highest
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_CON2, 0x00000000);
#endif
        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE, 0x0   );

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_OFST_ADDR, OFFSET_ADDR, LSCI_OFFSET);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE, LSCI_XSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE, LSCI_YSIZE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE, LSCI_STRIDE);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR, BASE_ADDR ,
                reinterpret_cast<ISP_CAM_LSCI_BA*>(REG_INFO_VALUE_PTR(CAM_LSCI_BASE_ADDR))->BASE_ADDR);

        ISP_WRITE_REG(getIspReg(drv_mode), CAM_LSC_CTL1, 0x10000000);
        
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_WIDTH,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_WIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_HEIGHT,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_HEIGHT);//sdblk_height,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL2, SDBLK_XNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL2_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL2))->SDBLK_XNUM);//sdblk_xnum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_CTL3, SDBLK_YNUM,
                reinterpret_cast<ISP_CAM_LSC_CTL3_T*>(REG_INFO_VALUE_PTR(CAM_LSC_CTL3))->SDBLK_YNUM);//sdblk_ynum,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lWIDTH); //sdblk_width,
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT,
                reinterpret_cast<ISP_CAM_LSC_LBLOCK_T*>(REG_INFO_VALUE_PTR(CAM_LSC_LBLOCK))->SDBLK_lHEIGHT);


        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO11,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO11);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO10,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO10);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO01,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO01);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_RATIO, RATIO00,
                reinterpret_cast<ISP_CAM_LSC_RATIO_T*>(REG_INFO_VALUE_PTR(CAM_LSC_RATIO))->RATIO00);

        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH2,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH2);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH1,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH1);
        ISP_WRITE_BITS(getIspReg(drv_mode), CAM_LSC_GAIN_TH, SDBLK_GAIN_TH0,
                reinterpret_cast<ISP_CAM_LSC_GAIN_TH_T*>(REG_INFO_VALUE_PTR(CAM_LSC_GAIN_TH))->SDBLK_GAIN_TH0);


        if (LSC_EN) {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 1);
        } else {
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_SET, LSC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_EN1_CLR, LSC_EN_CLR, 1);

            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_SET, LSCI_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(drv_mode), CAM_CTL_DMA_EN_CLR, LSCI_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_LSC);
#if 0
        MY_LOG_IF(ENABLE_MY_LOG,"%s ISP_DRV_CQ2_SYNC (XSize, YSize, Stride, ERR_STAT, LWidth, LHeight) = \n"
                "(%d, %d, %d, %d, %d, %d)\n", __FUNCTION__,
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_XSIZE, XSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_YSIZE, YSIZE),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSCI_STRIDE, STRIDE),
                ISP_READ_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_LSCI_ERR_STAT, ERR_STAT),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lWIDTH),
                ISP_READ_BITS(getIspReg(drv_mode), CAM_LSC_LBLOCK, SDBLK_lHEIGHT)
        );
#endif
        MY_LOG_IF(ENABLE_MY_LOG, "%s ISP_DRV_CQ2_SYNC \n"
            "CAM_LSCI_BASE_ADDR = 0x%08x\n"
            "CAM_LSCI_XSIZE     = 0x%08x\n"
            "CAM_LSCI_YSIZE     = 0x%08x\n"
            "CAM_LSCI_STRIDE    = 0x%08x\n"
            "CAM_LSCI_CON       = 0x%08x\n"
            "CAM_LSCI_CON2      = 0x%08x\n"
            "CAM_LSC_CTL1       = 0x%08x\n"
            "CAM_LSC_CTL2       = 0x%08x\n"
            "CAM_LSC_CTL3       = 0x%08x\n"
            "CAM_LSC_LBLOCK     = 0x%08x\n"
            "CAM_LSC_RATIO      = 0x%08x\n"
            "CAM_LSC_GAIN_TH    = 0x%08x\n", __FUNCTION__,
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_BASE_ADDR),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_XSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_YSIZE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_STRIDE),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSCI_CON2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL1),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL2),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_CTL3),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_LBLOCK),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_RATIO),
            ISP_READ_REG(getIspReg(drv_mode), CAM_LSC_GAIN_TH));
    }

    addressErrorCheck("After ISP_MGR_LSC_T::apply()");

    return  MTRUE;
}

MVOID
ISP_MGR_LSC_T::
enableLsc(MBOOL enable)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s %d\n", __FUNCTION__, enable);
    reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->LSC_EN = enable;
    reinterpret_cast<ISP_CAM_LSCI_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN))->LSCI_EN = enable;
}

MBOOL
ISP_MGR_LSC_T::
isEnable(void)
{
    MY_LOG_IF(ENABLE_MY_LOG,"%s %d\n", __FUNCTION__,
            reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->LSC_EN);
    return (reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->LSC_EN);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_PGN_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_PGN_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_PGN_T::
setIspPregain()
{
    // ISP pregain
    m_rIspPregain.i4R = (m_rIspAWBGain.i4R * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4G = (m_rIspAWBGain.i4G * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    m_rIspPregain.i4B = (m_rIspAWBGain.i4B * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG("m_i4FlareGain = %d\n", m_i4FlareGain);
    MY_LOG("m_rIspPregain.i4R = %d; m_rIspAWBGain.i4R = %d\n", m_rIspPregain.i4R , m_rIspAWBGain.i4R);
    MY_LOG("m_rIspPregain.i4G = %d; m_rIspAWBGain.i4G = %d\n", m_rIspPregain.i4G , m_rIspAWBGain.i4G);
    MY_LOG("m_rIspPregain.i4B = %d; m_rIspAWBGain.i4B = %d\n", m_rIspPregain.i4B , m_rIspAWBGain.i4B);

    // ISP flare offset
    m_i4IspFlareOffset = (m_i4FlareOffset * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MY_LOG("m_i4IspFlareOffset = %d; m_i4FlareOffset = %d\n", m_i4IspFlareOffset , m_i4FlareOffset);

    if ((m_rIspPregain.i4R == 0) ||
        (m_rIspPregain.i4G == 0) ||
        (m_rIspPregain.i4B == 0)) {
        MY_ERR("setIspPregain(): R = %d, G = %d, B = %d\n", m_rIspPregain.i4R, m_rIspPregain.i4G, m_rIspPregain.i4B);
        return MFALSE;
    }

    // CAM_PGN_GAIN01
    reinterpret_cast<ISP_CAM_PGN_GAIN01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN01))->PGN_CH0_GAIN = m_rIspPregain.i4B;
    reinterpret_cast<ISP_CAM_PGN_GAIN01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN01))->PGN_CH1_GAIN = m_rIspPregain.i4G;

    // CAM_PGN_GAIN23
    reinterpret_cast<ISP_CAM_PGN_GAIN23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN23))->PGN_CH2_GAIN = m_rIspPregain.i4G;
    reinterpret_cast<ISP_CAM_PGN_GAIN23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_GAIN23))->PGN_CH3_GAIN = m_rIspPregain.i4R;

    // CAM_PGN_OFFS01
    MUINT32 OFFS = (m_i4IspFlareOffset >= 0) ? static_cast<MUINT32>(m_i4IspFlareOffset) : static_cast<MUINT32>(4096 + m_i4IspFlareOffset);
    reinterpret_cast<ISP_CAM_PGN_OFFS01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS01))->PGN_CH0_OFFS = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFFS01_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS01))->PGN_CH1_OFFS = OFFS;

    // CAM_PGN_OFFS23
    reinterpret_cast<ISP_CAM_PGN_OFFS23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS23))->PGN_CH2_OFFS = OFFS;
    reinterpret_cast<ISP_CAM_PGN_OFFS23_T*>(REG_INFO_VALUE_PTR(CAM_PGN_OFFS23))->PGN_CH3_OFFS = OFFS;

    return MTRUE;
}

template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
put(ISP_NVRAM_PGN_T const& rParam)
{
    PUT_REG_INFO(CAM_PGN_SATU01, satu01);
    PUT_REG_INFO(CAM_PGN_SATU23, satu23);
    PUT_REG_INFO(CAM_PGN_GAIN01, gain01);
    PUT_REG_INFO(CAM_PGN_GAIN23, gain23);
    PUT_REG_INFO(CAM_PGN_OFFS01, offs01);
    PUT_REG_INFO(CAM_PGN_OFFS23, offs23);

    return  (*this);
}


template <>
ISP_MGR_PGN_T&
ISP_MGR_PGN_T::
get(ISP_NVRAM_PGN_T& rParam)
{
    GET_REG_INFO(CAM_PGN_SATU01, satu01);
    GET_REG_INFO(CAM_PGN_SATU23, satu23);
    GET_REG_INFO(CAM_PGN_GAIN01, gain01);
    GET_REG_INFO(CAM_PGN_GAIN23, gain23);
    GET_REG_INFO(CAM_PGN_OFFS01, offs01);
    GET_REG_INFO(CAM_PGN_OFFS23, offs23);

    return  (*this);
}


MBOOL
ISP_MGR_PGN_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_PGN_T::apply()");

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_PGN, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, PGN_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, PGN_EN_SET, 1);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PGN);
        writeRegs(CAM_ISP_PGN, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, PGN_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, PGN_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PGN);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PGN);
        writeRegs(CAM_ISP_PGN, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, PGN_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, PGN_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PGN);
    }

    addressErrorCheck("After ISP_MGR_PGN_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
put(ISP_NVRAM_SL2_T const& rParam)
{
    PUT_REG_INFO(CAM_SL2_CEN, cen);
    PUT_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    PUT_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    PUT_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);
    
    return  (*this);
}


template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2_T& rParam)
{
    GET_REG_INFO(CAM_SL2_CEN, cen);
    GET_REG_INFO(CAM_SL2_MAX0_RR, max0_rr);
    GET_REG_INFO(CAM_SL2_MAX1_RR, max1_rr);
    GET_REG_INFO(CAM_SL2_MAX2_RR, max2_rr);

    return  (*this);
}

MBOOL
ISP_MGR_SL2_T::
apply(EIspProfile_T eIspProfile)
{
#define ISP_SL2_LOG(cq) \
    MY_LOG_IF(ENABLE_MY_LOG,                                        \
        "%s "#cq" bSL2_EN = %d\n"                                   \
        "    SL2_EN_SET = %d\n"                                     \
        "    CAM_SL2_CEN     = 0x%08x\n"                            \
        "    CAM_SL2_MAX0_RR = 0x%08x\n"                            \
        "    CAM_SL2_MAX1_RR = 0x%08x\n"                            \
        "    CAM_SL2_MAX2_RR = 0x%08x\n",                           \
        __FUNCTION__, bSL2_EN,                                      \
        ISP_READ_BITS(getIspReg(cq), CAM_CTL_EN1_SET, SL2_EN_SET),  \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_CEN),                   \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX0_RR),               \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX1_RR),               \
        ISP_READ_REG(getIspReg(cq), CAM_SL2_MAX2_RR))

    addressErrorCheck("Before ISP_MGR_SL2_T::apply()");

    MBOOL bSL2_EN = isEnable();

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_CDP_SL2_FEATUREIO_1, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (bSL2_EN) {
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, SL2_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, SL2_EN_SET, 1);
        } else {
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, SL2_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, SL2_EN_CLR, 1);
        }

        ISP_SL2_LOG(ISPDRV_MODE_CQ0);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setSl2(ISP_DRV_CQ01_SYNC, bSL2_EN);
        writeRegs(CAM_CDP_SL2_FEATUREIO_1, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (bSL2_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, SL2_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, SL2_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, SL2_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, SL2_EN_CLR, 1);
        }
        
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_SL2);
        
        ISP_SL2_LOG(ISPDRV_MODE_CQ1_SYNC);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setSl2(ISP_DRV_CQ02_SYNC, bSL2_EN);
        writeRegs(CAM_CDP_SL2_FEATUREIO_1, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        if (bSL2_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, SL2_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, SL2_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, SL2_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, SL2_EN_CLR, 1);
        }
        
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_SL2);

        ISP_SL2_LOG(ISPDRV_MODE_CQ2_SYNC);
    }

    addressErrorCheck("After ISP_MGR_SL2_T::apply()");

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CFA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CFA_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CFA_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
put(ISP_NVRAM_CFA_T const& rParam)
{
    PUT_REG_INFO(CAM_CFA_BYPASS, bypass);
    PUT_REG_INFO(CAM_CFA_ED_F, ed_f);
    PUT_REG_INFO(CAM_CFA_ED_NYQ, ed_nyq);
    PUT_REG_INFO(CAM_CFA_ED_STEP, ed_step);
    PUT_REG_INFO(CAM_CFA_RGB_HF, rgb_hf);
    PUT_REG_INFO(CAM_CFA_BW, bw);
    PUT_REG_INFO(CAM_CFA_F1_ACT, f1_act);
    PUT_REG_INFO(CAM_CFA_F2_ACT, f2_act);
    PUT_REG_INFO(CAM_CFA_F3_ACT, f3_act);
    PUT_REG_INFO(CAM_CFA_F4_ACT, f4_act);
    PUT_REG_INFO(CAM_CFA_F1_L, f1_l);
    PUT_REG_INFO(CAM_CFA_F2_L, f2_l);
    PUT_REG_INFO(CAM_CFA_F3_L, f3_l);
    PUT_REG_INFO(CAM_CFA_F4_L, f4_l);
    PUT_REG_INFO(CAM_CFA_HF_RB, hf_rb);
    PUT_REG_INFO(CAM_CFA_HF_GAIN, hf_gain);
    PUT_REG_INFO(CAM_CFA_HF_COMP, hf_comp);
    PUT_REG_INFO(CAM_CFA_HF_CORING_TH, hf_coring_th);
    PUT_REG_INFO(CAM_CFA_ACT_LUT, act_lut);
    PUT_REG_INFO(CAM_CFA_SPARE, spare);
    PUT_REG_INFO(CAM_CFA_BB, bb);

    return  (*this);
}


template <>
ISP_MGR_CFA_T&
ISP_MGR_CFA_T::
get(ISP_NVRAM_CFA_T& rParam)
{
    GET_REG_INFO(CAM_CFA_BYPASS, bypass);
    GET_REG_INFO(CAM_CFA_ED_F, ed_f);
    GET_REG_INFO(CAM_CFA_ED_NYQ, ed_nyq);
    GET_REG_INFO(CAM_CFA_ED_STEP, ed_step);
    GET_REG_INFO(CAM_CFA_RGB_HF, rgb_hf);
    GET_REG_INFO(CAM_CFA_BW, bw);
    GET_REG_INFO(CAM_CFA_F1_ACT, f1_act);
    GET_REG_INFO(CAM_CFA_F2_ACT, f2_act);
    GET_REG_INFO(CAM_CFA_F3_ACT, f3_act);
    GET_REG_INFO(CAM_CFA_F4_ACT, f4_act);
    GET_REG_INFO(CAM_CFA_F1_L, f1_l);
    GET_REG_INFO(CAM_CFA_F2_L, f2_l);
    GET_REG_INFO(CAM_CFA_F3_L, f3_l);
    GET_REG_INFO(CAM_CFA_F4_L, f4_l);
    GET_REG_INFO(CAM_CFA_HF_RB, hf_rb);
    GET_REG_INFO(CAM_CFA_HF_GAIN, hf_gain);
    GET_REG_INFO(CAM_CFA_HF_COMP, hf_comp);
    GET_REG_INFO(CAM_CFA_HF_CORING_TH, hf_coring_th);
    GET_REG_INFO(CAM_CFA_ACT_LUT, act_lut);
    GET_REG_INFO(CAM_CFA_SPARE, spare);
    GET_REG_INFO(CAM_CFA_BB, bb);

    return  (*this);
}

MBOOL
ISP_MGR_CFA_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_CFA_T::apply()");

    MINT32 i4BAYER_BYPASS = reinterpret_cast<ISP_CAM_CFA_BYPASS_T*>(REG_INFO_VALUE_PTR(CAM_CFA_BYPASS))->BAYER_BYPASS;

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_CFA, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setCfa(ISP_DRV_CQ01_SYNC, i4BAYER_BYPASS);
        writeRegs(CAM_ISP_CFA, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_CFA);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setCfa(ISP_DRV_CQ02_SYNC, i4BAYER_BYPASS);
        writeRegs(CAM_ISP_CFA, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_CFA);
    }

    addressErrorCheck("After ISP_MGR_CFA_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_T const& rParam)
{
    PUT_REG_INFO(CAM_G2G_CONV0A, conv0a);
    PUT_REG_INFO(CAM_G2G_CONV0B, conv0b);
    PUT_REG_INFO(CAM_G2G_CONV1A, conv1a);
    PUT_REG_INFO(CAM_G2G_CONV1B, conv1b);
    PUT_REG_INFO(CAM_G2G_CONV2A, conv2a);
    PUT_REG_INFO(CAM_G2G_CONV2B, conv2b);

    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam)
{
    GET_REG_INFO(CAM_G2G_CONV0A, conv0a);
    GET_REG_INFO(CAM_G2G_CONV0B, conv0b);
    GET_REG_INFO(CAM_G2G_CONV1A, conv1a);
    GET_REG_INFO(CAM_G2G_CONV1B, conv1b);
    GET_REG_INFO(CAM_G2G_CONV2A, conv2a);
    GET_REG_INFO(CAM_G2G_CONV2B, conv2b);

    return  (*this);
}

MBOOL
ISP_MGR_CCM_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_CCM_T::apply()");

    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2G_CONV0A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0A))->G2G_CNV_00 = 256;
        reinterpret_cast<ISP_CAM_G2G_CONV0A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0A))->G2G_CNV_01 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV0B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV0B))->G2G_CNV_02 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV1A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1A))->G2G_CNV_10 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV1A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1A))->G2G_CNV_11 = 256;
        reinterpret_cast<ISP_CAM_G2G_CONV1B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV1B))->G2G_CNV_12 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2A))->G2G_CNV_20 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2A_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2A))->G2G_CNV_21 = 0;
        reinterpret_cast<ISP_CAM_G2G_CONV2B_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CONV2B))->G2G_CNV_22 = 256;
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        // Set ACC to 8: Q1.2.8
        ISP_WRITE_REG(getIspReg(ISPDRV_MODE_CQ0), CAM_G2G_ACC, 0x00000008);
        writeRegs(CAM_ISP_G2G, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_CLR, G2G_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN1_SET, G2G_EN_SET, 1);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_G2G);
        // Set ACC to 8: Q1.2.8
        ISP_WRITE_REG(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_G2G_ACC, 0x00000008);
        writeRegs(CAM_ISP_G2G, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, G2G_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, G2G_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_G2G);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_G2G);
        // Set ACC to 8: Q1.2.8
        ISP_WRITE_REG(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_G2G_ACC, 0x00000008);
        writeRegs(CAM_ISP_G2G, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, G2G_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, G2G_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_G2G);
    }

    addressErrorCheck("After ISP_MGR_CCM_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_GGM_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_GGM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
put(ISP_NVRAM_GGM_T const& rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        m_rIspRegInfo_GGM_RB[i].val = rParam.rb_gmt.set[i];
        m_rIspRegInfo_GGM_G[i].val = rParam.g_gmt.set[i];
        //MY_LOG("m_rIspRegInfo_GGM_RB[%d].val = 0x%8x", i, m_rIspRegInfo_GGM_RB[i].val);
        //MY_LOG("m_rIspRegInfo_GGM_G[%d].val = 0x%8x", i, m_rIspRegInfo_GGM_G[i].val);
    }

    return  (*this);
}


template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
get(ISP_NVRAM_GGM_T& rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        rParam.rb_gmt.set[i] = m_rIspRegInfo_GGM_RB[i].val;
        rParam.g_gmt.set[i] = m_rIspRegInfo_GGM_G[i].val;
    }

    return  (*this);
}

MBOOL
ISP_MGR_GGM_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL bGAMMA_EN = isEnable();
    reinterpret_cast<ISP_CAM_GGM_CTRL_T*>(REG_INFO_VALUE_PTR(CAM_GGM_CTRL))->GAMMA_EN = bGAMMA_EN;

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        // GGM_CTL
        writeRegs(CAM_ISP_GGM_CTL, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_GGM_CTRL, GAMMA_EN, bGAMMA_EN);
        // GGM_RB
        writeRegs(CAM_ISP_GGMRB, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_RB), GGM_LUT_SIZE);
        // GGM_G
        writeRegs(CAM_ISP_GGMG, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_G), GGM_LUT_SIZE);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        // GGM_CTL
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGM_CTL);
        writeRegs(CAM_ISP_GGM_CTL, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_GGM_CTRL, GAMMA_EN, bGAMMA_EN);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGM_CTL);

        // GGM_RB
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGMRB);
        writeRegs(CAM_ISP_GGMRB, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_RB), GGM_LUT_SIZE);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGMRB);

        // GGM_G
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGMG);
        writeRegs(CAM_ISP_GGMG, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_G), GGM_LUT_SIZE);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_GGMG);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        // GGM_CTL
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGM_CTL);
        writeRegs(CAM_ISP_GGM_CTL, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_GGM_CTRL, GAMMA_EN, bGAMMA_EN);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGM_CTL);

        // GGM_RB
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGMRB);
        writeRegs(CAM_ISP_GGMRB, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_RB), GGM_LUT_SIZE);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGMRB);

        // GGM_G
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGMG);
        writeRegs(CAM_ISP_GGMG, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_GGM_G), GGM_LUT_SIZE);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_CLR, GGM_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN1_SET, GGM_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_GGMG);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_G2C_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_G2C_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
put(ISP_NVRAM_G2C_T const& rParam)
{
    PUT_REG_INFO(CAM_G2C_CONV_0A, conv_0a);
    PUT_REG_INFO(CAM_G2C_CONV_0B, conv_0b);
    PUT_REG_INFO(CAM_G2C_CONV_1A, conv_1a);
    PUT_REG_INFO(CAM_G2C_CONV_1B, conv_1b);
    PUT_REG_INFO(CAM_G2C_CONV_2A, conv_2a);
    PUT_REG_INFO(CAM_G2C_CONV_2B, conv_2b);

    return  (*this);
}


template <>
ISP_MGR_G2C_T&
ISP_MGR_G2C_T::
get(ISP_NVRAM_G2C_T& rParam)
{
    GET_REG_INFO(CAM_G2C_CONV_0A, conv_0a);
    GET_REG_INFO(CAM_G2C_CONV_0B, conv_0b);
    GET_REG_INFO(CAM_G2C_CONV_1A, conv_1a);
    GET_REG_INFO(CAM_G2C_CONV_1B, conv_1b);
    GET_REG_INFO(CAM_G2C_CONV_2A, conv_2a);
    GET_REG_INFO(CAM_G2C_CONV_2B, conv_2b);

    return  (*this);
}

MBOOL
ISP_MGR_G2C_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_G2C_T::apply()");

    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV00 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_0A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0A))->G2C_CNV01 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_CNV02 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_0B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_0B))->G2C_YOFFSET11 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV10 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1A))->G2C_CNV11 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_CNV12 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_1B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_1B))->G2C_UOFFSET10 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV20 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2A_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2A))->G2C_CNV21 = 0;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_CNV22 = 512;
        reinterpret_cast<ISP_CAM_G2C_CONV_2B_T*>(REG_INFO_VALUE_PTR(CAM_G2C_CONV_2B))->G2C_VOFFSET10 = 0;
    }


    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_G2C, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, G2C_EN_SET, 1);

    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_G2C);
        writeRegs(CAM_ISP_G2C, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, G2C_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_G2C);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_G2C);
        writeRegs(CAM_ISP_G2C, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, G2C_EN_SET, 1);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_G2C);
    }

    addressErrorCheck("After ISP_MGR_G2C_T::apply()");

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_T const& rParam)
{
    PUT_REG_INFO(CAM_ANR_CON1, con1);
    PUT_REG_INFO(CAM_ANR_CON2, con2);
    PUT_REG_INFO(CAM_ANR_CON3, con3);
    PUT_REG_INFO(CAM_ANR_YAD1, yad1);
    PUT_REG_INFO(CAM_ANR_YAD2, yad2);
    PUT_REG_INFO(CAM_ANR_4LUT1, lut1);
    PUT_REG_INFO(CAM_ANR_4LUT2, lut2);
    PUT_REG_INFO(CAM_ANR_4LUT3, lut3);
    PUT_REG_INFO(CAM_ANR_PTY, pty);
    PUT_REG_INFO(CAM_ANR_CAD, cad);
    PUT_REG_INFO(CAM_ANR_PTC, ptc);
    PUT_REG_INFO(CAM_ANR_LCE1, lce1);
    PUT_REG_INFO(CAM_ANR_LCE2, lce2);
    PUT_REG_INFO(CAM_ANR_HP1, hp1);
    PUT_REG_INFO(CAM_ANR_HP2, hp2);
    PUT_REG_INFO(CAM_ANR_HP3, hp3);
    PUT_REG_INFO(CAM_ANR_ACTY, acty);
    PUT_REG_INFO(CAM_ANR_ACTC, actc);

	m_bANRENCBackup = rParam.con1.val & 0x1;
	m_bANRENYBackup = (rParam.con1.val & 0x2)>>1;
	MY_LOG("ISP_MGR_NBC_T::put() ANR CON1 = %d\n", (rParam.con1.val));	

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_T& rParam)
{
    GET_REG_INFO(CAM_ANR_CON1, con1);
    GET_REG_INFO(CAM_ANR_CON2, con2);
    GET_REG_INFO(CAM_ANR_CON3, con3);
    GET_REG_INFO(CAM_ANR_YAD1, yad1);
    GET_REG_INFO(CAM_ANR_YAD2, yad2);
    GET_REG_INFO(CAM_ANR_4LUT1, lut1);
    GET_REG_INFO(CAM_ANR_4LUT2, lut2);
    GET_REG_INFO(CAM_ANR_4LUT3, lut3);
    GET_REG_INFO(CAM_ANR_PTY, pty);
    GET_REG_INFO(CAM_ANR_CAD, cad);
    GET_REG_INFO(CAM_ANR_PTC, ptc);
    GET_REG_INFO(CAM_ANR_LCE1, lce1);
    GET_REG_INFO(CAM_ANR_LCE2, lce2);
    GET_REG_INFO(CAM_ANR_HP1, hp1);
    GET_REG_INFO(CAM_ANR_HP2, hp2);
    GET_REG_INFO(CAM_ANR_HP3, hp3);
    GET_REG_INFO(CAM_ANR_ACTY, acty);
    GET_REG_INFO(CAM_ANR_ACTC, actc);

    return  (*this);
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_CCR_T const& rParam)
{
    PUT_REG_INFO(CAM_CCR_CON, con);
    PUT_REG_INFO(CAM_CCR_YLUT, ylut);
    PUT_REG_INFO(CAM_CCR_UVLUT, uvlut);
    PUT_REG_INFO(CAM_CCR_YLUT2, ylut2);

	m_bCCREnBackup = rParam.con.val & 0x1;
	MY_LOG("ISP_MGR_NBC_T::put() m_bCCREnBackup = %d\n", m_bCCREnBackup);

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
get(ISP_NVRAM_CCR_T& rParam)
{
    GET_REG_INFO(CAM_CCR_CON, con);
    GET_REG_INFO(CAM_CCR_YLUT, ylut);
    GET_REG_INFO(CAM_CCR_UVLUT, uvlut);
    GET_REG_INFO(CAM_CCR_YLUT2, ylut2);

    return  (*this);
}

MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_NBC_T::apply()");
	
	MY_LOG("ISP_MGR_NBC_T::apply() isCCREnable() = %d, m_bCCREnBackup = %d, CCR_EN = %d\n"
		, isCCREnable()
		, m_bCCREnBackup
		, reinterpret_cast<ISP_CAM_CCR_CON_T*>(REG_INFO_VALUE_PTR(CAM_CCR_CON))->CCR_EN);
	MY_LOG("ISP_MGR_NBC_T::apply() isANREnable()=%d, m_bANRENCBackup=%d, m_bANRENYBackup=%d, ANR_ENY=%d, ANR_ENC=%d\n"
		, isANREnable()
		, m_bANRENCBackup
		, m_bANRENYBackup
		, reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY
		, reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC);

    MBOOL bANR_ENY = /*reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY*/m_bANRENYBackup & isANREnable();
    MBOOL bANR_ENC = /*reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC*/m_bANRENCBackup & isANREnable();
    MBOOL bCCR_ENC = m_bCCREnBackup & isCCREnable();
    MBOOL bNBC_EN = bANR_ENY|bANR_ENC|bCCR_ENC;
    MINT32 i4ANR_FLT_MODE = reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_FLT_MODE;
    MINT32 i4ANR_SCALE_MODE = reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_SCALE_MODE;

    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY = bANR_ENY;
    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC = bANR_ENC;
    reinterpret_cast<ISP_CAM_CCR_CON_T*>(REG_INFO_VALUE_PTR(CAM_CCR_CON))->CCR_EN = bCCR_ENC;
	MY_LOG("before writing TdriMgr: (bNBC_EN, bCCR_ENC, bANR_ENC, bANR_ENY) = (%d, %d, %d, %d)\n",bNBC_EN, bCCR_ENC, bANR_ENC, bANR_ENY);
    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_NBC, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        // Set enable bit
        if (bNBC_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, NBC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, NBC_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, NBC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, NBC_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_ANR_CON1, ANR_ENY, bANR_ENY);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_ANR_CON1, ANR_ENC, bANR_ENC);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CCR_CON, CCR_EN, bCCR_ENC);
		
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setNbc(ISP_DRV_CQ01_SYNC, bNBC_EN, bANR_ENY, bANR_ENC, i4ANR_FLT_MODE, i4ANR_SCALE_MODE);
        writeRegs(CAM_ISP_NBC, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        // Set enable bit
        if (bNBC_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, NBC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, NBC_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, NBC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, NBC_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_ANR_CON1, ANR_ENY, bANR_ENY);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_ANR_CON1, ANR_ENC, bANR_ENC);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CCR_CON, CCR_EN, bCCR_ENC);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_NBC);


		
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setNbc(ISP_DRV_CQ02_SYNC, bNBC_EN, bANR_ENY, bANR_ENC, i4ANR_FLT_MODE, i4ANR_SCALE_MODE);
        writeRegs(CAM_ISP_NBC, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);

        // Set enable bit
        if (bNBC_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, NBC_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, NBC_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, NBC_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, NBC_EN_CLR, 1);
        }

        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_ANR_CON1, ANR_ENY, bANR_ENY);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_ANR_CON1, ANR_ENC, bANR_ENC);
        //ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CCR_CON, CCR_EN, bCCR_ENC);
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_NBC);
		
    }

    addressErrorCheck("After ISP_MGR_NBC_T::apply()");
	
	MY_LOG("after writing TdriMgr: (bNBC_EN, bCCR_ENC, bANR_ENC, bANR_ENY) = (%d, %d, %d, %d)\n",bNBC_EN, bCCR_ENC, bANR_ENC, bANR_ENY);
	MY_LOG("ISP_MGR_NBC_T::apply() isCCREnable() = %d, m_bCCREnBackup = %d, CCR_EN = %d\n"
		, isCCREnable()
		, m_bCCREnBackup
		, reinterpret_cast<ISP_CAM_CCR_CON_T*>(REG_INFO_VALUE_PTR(CAM_CCR_CON))->CCR_EN);
	MY_LOG("ISP_MGR_NBC_T::apply() isANREnable()=%d, m_bANRENCBackup=%d, m_bANRENYBackup=%d, ANR_ENY=%d, ANR_ENC=%d\n"
		, isANREnable()
		, m_bANRENCBackup
		, m_bANRENYBackup
		, reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENY
		, reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR_ENC);	


    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_PCA_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_PCA_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
put(ISP_NVRAM_PCA_T const& rParam)
{
    PUT_REG_INFO(CAM_PCA_CON1, con1);
    PUT_REG_INFO(CAM_PCA_CON2, con2);

    return  (*this);
}


template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
get(ISP_NVRAM_PCA_T& rParam)
{
    GET_REG_INFO(CAM_PCA_CON1, con1);
    GET_REG_INFO(CAM_PCA_CON2, con2);

    return  (*this);
}

MBOOL
ISP_MGR_PCA_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_PCA_T::apply()");

    MBOOL bPCA_EN = isEnable();

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        // PCA_TBL
        writeRegs(CAM_ISP_PCA, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_rIspRegInfo_PCA_LUT), PCA_BIN_NUM);
        // PCA_CON
        writeRegs(CAM_ISP_PCA_CON, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bPCA_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, PCA_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, PCA_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, PCA_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, PCA_EN_CLR, 1);
        }
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        // PCA_TBL
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA);
        writeRegs(CAM_ISP_PCA, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_PCA_LUT), PCA_BIN_NUM);
        // Set enable bit
        if (bPCA_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 1);
        }
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA);
        // PCA_CON
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA_CON);
        writeRegs(CAM_ISP_PCA_CON, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bPCA_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 1);
        }
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA_CON);

    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        // PCA_TBL
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PCA);
        writeRegs(CAM_ISP_PCA, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_rIspRegInfo_PCA_LUT), PCA_BIN_NUM);
        // Set enable bit
        if (bPCA_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PCA);
        // PCA_CON
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PCA_CON);
        writeRegs(CAM_ISP_PCA_CON, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bPCA_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, PCA_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, PCA_EN_CLR, 1);
        }
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_PCA_CON);
    }

    addressErrorCheck("After ISP_MGR_PCA_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SEEE (SE + EE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_SEEE_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_SEEE_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_EE_T const& rParam)
{
    PUT_REG_INFO(CAM_SEEE_SRK_CTRL, srk_ctrl);
    PUT_REG_INFO(CAM_SEEE_CLIP_CTRL, clip_ctrl);
    PUT_REG_INFO(CAM_SEEE_HP_CTRL1, hp_ctrl1);
    PUT_REG_INFO(CAM_SEEE_HP_CTRL2, hp_ctrl2);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL1, ed_ctrl1);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL2, ed_ctrl2);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL3, ed_ctrl3);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL4, ed_ctrl4);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL5, ed_ctrl5);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL6, ed_ctrl6);
    PUT_REG_INFO(CAM_SEEE_ED_CTRL7, ed_ctrl7);
    PUT_REG_INFO(CAM_SEEE_EE_LINK1, ee_link1);
    PUT_REG_INFO(CAM_SEEE_EE_LINK2, ee_link2);
    PUT_REG_INFO(CAM_SEEE_EE_LINK3, ee_link3);
    PUT_REG_INFO(CAM_SEEE_EE_LINK4, ee_link4);
    PUT_REG_INFO(CAM_SEEE_EE_LINK5, ee_link5);

    return  (*this);
}


template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_EE_T& rParam)
{
    GET_REG_INFO(CAM_SEEE_SRK_CTRL, srk_ctrl);
    GET_REG_INFO(CAM_SEEE_CLIP_CTRL, clip_ctrl);
    GET_REG_INFO(CAM_SEEE_HP_CTRL1, hp_ctrl1);
    GET_REG_INFO(CAM_SEEE_HP_CTRL2, hp_ctrl2);
    GET_REG_INFO(CAM_SEEE_ED_CTRL1, ed_ctrl1);
    GET_REG_INFO(CAM_SEEE_ED_CTRL2, ed_ctrl2);
    GET_REG_INFO(CAM_SEEE_ED_CTRL3, ed_ctrl3);
    GET_REG_INFO(CAM_SEEE_ED_CTRL4, ed_ctrl4);
    GET_REG_INFO(CAM_SEEE_ED_CTRL5, ed_ctrl5);
    GET_REG_INFO(CAM_SEEE_ED_CTRL6, ed_ctrl6);
    GET_REG_INFO(CAM_SEEE_ED_CTRL7, ed_ctrl7);
    GET_REG_INFO(CAM_SEEE_EE_LINK1, ee_link1);
    GET_REG_INFO(CAM_SEEE_EE_LINK2, ee_link2);
    GET_REG_INFO(CAM_SEEE_EE_LINK3, ee_link3);
    GET_REG_INFO(CAM_SEEE_EE_LINK4, ee_link4);
    GET_REG_INFO(CAM_SEEE_EE_LINK5, ee_link5);
    return  (*this);
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_SE_T const& rParam)
{
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL, edge_ctrl);
    PUT_REG_INFO(CAM_SEEE_Y_CTRL, y_ctrl);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL1, edge_ctrl1);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL2, edge_ctrl2);
    PUT_REG_INFO(CAM_SEEE_EDGE_CTRL3, edge_ctrl3);
    PUT_REG_INFO(CAM_SEEE_SPECIAL_CTRL, special_ctrl);
    PUT_REG_INFO(CAM_SEEE_CORE_CTRL1, core_ctrl1);
    PUT_REG_INFO(CAM_SEEE_CORE_CTRL2, core_ctrl2);

    return  (*this);
}


template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_SE_T& rParam)
{
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL, edge_ctrl);
    GET_REG_INFO(CAM_SEEE_Y_CTRL, y_ctrl);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL1, edge_ctrl1);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL2, edge_ctrl2);
    GET_REG_INFO(CAM_SEEE_EDGE_CTRL3, edge_ctrl3);
    GET_REG_INFO(CAM_SEEE_SPECIAL_CTRL, special_ctrl);
    GET_REG_INFO(CAM_SEEE_CORE_CTRL1, core_ctrl1);
    GET_REG_INFO(CAM_SEEE_CORE_CTRL2, core_ctrl2);

    return  (*this);
}


MBOOL
ISP_MGR_SEEE_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_SEEE_T::apply()");

    MBOOL bSEEE_EN = isEnable();
    MINT32 i4SE_EDGE = reinterpret_cast<ISP_CAM_SEEE_EDGE_CTRL_T*>(REG_INFO_VALUE_PTR(CAM_SEEE_EDGE_CTRL))->SE_EDGE;

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ0) { // PASS 1
        writeRegs(CAM_ISP_SEEE, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bSEEE_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, SEEE_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_SET, SEEE_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ0), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 1);
        }
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ1_SYNC) { // PASS 2 (CQ1)
        TdriMgr::getInstance().setSeee(ISP_DRV_CQ01_SYNC, bSEEE_EN, i4SE_EDGE);
        writeRegs(CAM_ISP_SEEE, ISPDRV_MODE_CQ1_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bSEEE_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, SEEE_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_SET, SEEE_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ1_SYNC), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_SEEE);
    }

    if (m_rIspDrvMode[eIspProfile] & ISPDRV_MODE_CQ2_SYNC) { // PASS 2 (CQ2)
        TdriMgr::getInstance().setSeee(ISP_DRV_CQ02_SYNC, bSEEE_EN, i4SE_EDGE);
        writeRegs(CAM_ISP_SEEE, ISPDRV_MODE_CQ2_SYNC, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        // Set enable bit
        if (bSEEE_EN){
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, SEEE_EN_SET, 1);
        }else{
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_SET, SEEE_EN_SET, 0);
            ISP_WRITE_ENABLE_BITS(getIspReg(ISPDRV_MODE_CQ2_SYNC), CAM_CTL_EN2_CLR, SEEE_EN_CLR, 1);
        }

        TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC, TDRI_MGR_FUNC_SEEE);
    }

    addressErrorCheck("After ISP_MGR_SEEE_T::apply()");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AWB_STAT_CONFIG_T&
ISP_MGR_AWB_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AWB_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
config(AWB_STAT_CONFIG_T& rAWBStatConfig)
{
    addressErrorCheck("Before ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    // CAM_AWB_WIN_ORG
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_WIN_ORG_X = rAWBStatConfig.i4WindowOriginX;
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_WIN_ORG_Y = rAWBStatConfig.i4WindowOriginY;
    // CAM_AWB_WIN_SIZE
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_WIN_SIZE_X = rAWBStatConfig.i4WindowSizeX;
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_WIN_SIZE_Y = rAWBStatConfig.i4WindowSizeY;
    // CAM_AWB_WIN_PITCH
    reinterpret_cast<ISP_CAM_AWB_WIN_PITCH_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PITCH))->AWB_WIN_PIT_X = rAWBStatConfig.i4WindowPitchX;
    reinterpret_cast<ISP_CAM_AWB_WIN_PITCH_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PITCH))->AWB_WIN_PIT_Y = rAWBStatConfig.i4WindowPitchY;
    // CAM_AWB_WIN_NUM
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_WIN_NUM_X = rAWBStatConfig.i4WindowNumX;
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_WIN_NUM_Y = rAWBStatConfig.i4WindowNumY;
    // CAM_AWB_RAWPREGAIN1_0
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_0))->RAWPREGAIN1_R = rAWBStatConfig.i4PreGainR;
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_0))->RAWPREGAIN1_G = rAWBStatConfig.i4PreGainG;
    // CAM_AWB_RAWPREGAIN1_1
    reinterpret_cast<ISP_CAM_AWB_RAWPREGAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWPREGAIN1_1))->RAWPREGAIN1_B = rAWBStatConfig.i4PreGainB;
    // CAM_AWB_RAWLIMIT1_0
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_0))->RAWLIMIT1_R = rAWBStatConfig.i4PreGainLimitR;
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_0))->RAWLIMIT1_G = rAWBStatConfig.i4PreGainLimitG;
    // CAM_AWB_RAWLIMIT1_1
    reinterpret_cast<ISP_CAM_AWB_RAWLIMIT1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_RAWLIMIT1_1))->RAWLIMIT1_B = rAWBStatConfig.i4PreGainLimitB;
    // CAM_AWB_LOW_THR
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;
    // CAM_AWB_HI_THR
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;
    // CAM_AWB_PIXEL_CNT0
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT0))->PIXEL_CNT0 = rAWBStatConfig.i4PixelCountR;
    // CAM_AWB_PIXEL_CNT1
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT1))->PIXEL_CNT1 = rAWBStatConfig.i4PixelCountG;
    // CAM_AWB_PIXEL_CNT2
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT2_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT2))->PIXEL_CNT2 = rAWBStatConfig.i4PixelCountB;
    // CAM_AWB_ERR_THR
    reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;
    // CAM_AWB_ROT
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_C = rAWBStatConfig.i4Cos;
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_S = rAWBStatConfig.i4Sin;

    #define AWB_LIGHT_AREA_CFG(TYPE, REG, FIELD, BOUND)\
    if (BOUND >= 0)\
        reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = BOUND;\
    else\
        reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = (1 << 14) + BOUND;\


    // CAM_AWB_L0
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_LOW, rAWBStatConfig.i4AWBXY_WINL[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_UP, rAWBStatConfig.i4AWBXY_WINR[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_UP, rAWBStatConfig.i4AWBXY_WINU[0])

    // CAM_AWB_L1
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_LOW, rAWBStatConfig.i4AWBXY_WINL[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_UP, rAWBStatConfig.i4AWBXY_WINR[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_UP, rAWBStatConfig.i4AWBXY_WINU[1])

    // CAM_AWB_L2
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_LOW, rAWBStatConfig.i4AWBXY_WINL[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_UP, rAWBStatConfig.i4AWBXY_WINR[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_UP, rAWBStatConfig.i4AWBXY_WINU[2])

    // CAM_AWB_L3
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_LOW, rAWBStatConfig.i4AWBXY_WINL[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_UP, rAWBStatConfig.i4AWBXY_WINR[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_UP, rAWBStatConfig.i4AWBXY_WINU[3])

    // CAM_AWB_L4
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_LOW, rAWBStatConfig.i4AWBXY_WINL[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_UP, rAWBStatConfig.i4AWBXY_WINR[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_UP, rAWBStatConfig.i4AWBXY_WINU[4])

    // CAM_AWB_L5
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_LOW, rAWBStatConfig.i4AWBXY_WINL[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_UP, rAWBStatConfig.i4AWBXY_WINR[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_UP, rAWBStatConfig.i4AWBXY_WINU[5])

    // CAM_AWB_L6
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_LOW, rAWBStatConfig.i4AWBXY_WINL[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_UP, rAWBStatConfig.i4AWBXY_WINR[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_UP, rAWBStatConfig.i4AWBXY_WINU[6])

    // CAM_AWB_L7
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_LOW, rAWBStatConfig.i4AWBXY_WINL[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_UP, rAWBStatConfig.i4AWBXY_WINR[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_UP, rAWBStatConfig.i4AWBXY_WINU[7])

    // CAM_AWB_L8
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_LOW, rAWBStatConfig.i4AWBXY_WINL[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_UP, rAWBStatConfig.i4AWBXY_WINR[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_UP, rAWBStatConfig.i4AWBXY_WINU[8])

    // CAM_AWB_L9
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_LOW, rAWBStatConfig.i4AWBXY_WINL[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_UP, rAWBStatConfig.i4AWBXY_WINR[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_UP, rAWBStatConfig.i4AWBXY_WINU[9])

    apply();

    addressErrorCheck("After ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply()
{
    writeRegs(CAM_ISP_AWB, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_RAWPREGAIN2_T&
ISP_MGR_AE_RAWPREGAIN2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AE_RAWPREGAIN2_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_RAWPREGAIN2_T::
setRAWPregain2(AWB_GAIN_T& rAWBRAWPregain2)
{
    addressErrorCheck("Before ISP_MGR_AE_RAWPREGAIN2_T::apply()");

    // CAM_AE_RAWPREGAIN2_0
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_0))->RAWPREGAIN2_R = rAWBRAWPregain2.i4R;
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_0))->RAWPREGAIN2_G = rAWBRAWPregain2.i4G;

    // CAM_AE_RAWPREGAIN2_1
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWPREGAIN2_1))->RAWPREGAIN2_B = rAWBRAWPregain2.i4B;

    apply();

    addressErrorCheck("After ISP_MGR_AE_RAWPREGAIN2_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_RAWPREGAIN2_T::
apply()
{
    return  writeRegs(CAM_ISP_AE, ISPDRV_MODE_ISP, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
    // FIXME: return  writeRegs(CAM_ISP_AE, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics and Histogram Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_STAT_HIST_CONFIG_T&
ISP_MGR_AE_STAT_HIST_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AE_STAT_HIST_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_STAT_HIST_CONFIG_T::
config(AE_STAT_PARAM_T& rAEStatConfig)
{
    addressErrorCheck("Before ISP_MGR_AE_STAT_HIST_CONFIG_T::apply()");

    MY_LOG("setAEHistConfig Hist0:%d %d %d %d %d %d %d, Hist1:%d %d %d %d %d %d %d, Hist2:%d %d %d %d %d %d %d, Hist3:%d %d %d %d %d %d %d\n",
    rAEStatConfig.rAEHistWinCFG[0].bAEHistEn, rAEStatConfig.rAEHistWinCFG[0].uAEHistBin, rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow,
    rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[1].bAEHistEn,
    rAEStatConfig.rAEHistWinCFG[1].uAEHistBin, rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi,
    rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[2].bAEHistEn, rAEStatConfig.rAEHistWinCFG[2].uAEHistBin,
    rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt, rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow,
    rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi, rAEStatConfig.rAEHistWinCFG[3].bAEHistEn, rAEStatConfig.rAEHistWinCFG[3].uAEHistBin, rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt,
    rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow, rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi, rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow, rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi);


    // CAM_AE_RAWLIMIT2_0
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_0))->AE_LIMIT2_R = 0xFFF;
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_0))->AE_LIMIT2_G = 0xFFF;
    // CAM_AE_RAWLIMIT2_1
    reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RAWLIMIT2_1))->AE_LIMIT2_B = 0xFFF;
    // CAM_AE_MATRIX_COEF0
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF0))->RC_CNV00 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF0))->RC_CNV01 = 0x000;
    // CAM_AE_MATRIX_COEF1
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF1))->RC_CNV02 = 0x000;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF1))->RC_CNV10 = 0x000;
    // CAM_AE_MATRIX_COEF2
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF2))->RC_CNV11 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF2))->RC_CNV12 = 0x000;
    // CAM_AE_MATRIX_COEF3
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF3))->RC_CNV20 = 0x000;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF3))->RC_CNV21 = 0x000;
    // CAM_AE_MATRIX_COEF4
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF4))->RC_CNV22 = 0x200;
    reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_MATRIX_COEF4))->AE_RC_ACC = 0x09;
    // CAM_AE_HST_SET
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_BIN = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_BIN = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_BIN = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_BIN = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_COLOR = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_COLOR = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_COLOR = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
    reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_COLOR = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;
    // CAM_AE_HST0_RNG
    reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow;
    reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
    reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
    reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;
    // CAM_AE_HST1_RNG
    reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow;
    reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
    reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow;
    reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;
    // CAM_AE_HST2_RNG
    reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow;
    reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
    reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow;
    reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;
    // CAM_AE_HST3_RNG
    reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow;
    reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
    reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
    reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

    apply();

    // CAM_AE_HST_CTL
    ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_AE_HST_CTL, AE_HST0_EN, rAEStatConfig.rAEHistWinCFG[0].bAEHistEn);
    ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_AE_HST_CTL, AE_HST1_EN, rAEStatConfig.rAEHistWinCFG[1].bAEHistEn);
    ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_AE_HST_CTL, AE_HST2_EN, rAEStatConfig.rAEHistWinCFG[2].bAEHistEn);
    ISP_WRITE_BITS(getIspReg(ISPDRV_MODE_ISP), CAM_AE_HST_CTL, AE_HST3_EN, rAEStatConfig.rAEHistWinCFG[3].bAEHistEn);

    addressErrorCheck("After ISP_MGR_AE_STAT_HIST_CONFIG_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_HIST_CONFIG_T::
apply()
{
    return  writeRegs(CAM_ISP_AE, ISPDRV_MODE_ISP, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Flicker
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_FLK_CONFIG_T&
ISP_MGR_FLK_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_FLK_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_FLK_DEV<ESensorDev_Main>::getInstance();
    }
}


MBOOL
ISP_MGR_FLK_CONFIG_T::
apply()
{
#if 0
    ISPDRV_MODE_T drv_mode;
    IspDrv* m_pIspDrv = IspDrv::createInstance();

	drv_mode=ISPDRV_MODE_CQ0;

        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_DMA_ESFKO);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_DMA_FLKI);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_ISP_FLK);
        m_pIspDrv->cqDelModule(ISP_DRV_CQ0, CAM_TOP_CTL_01);

#if 1
		ISP_BITS(getIspReg(drv_mode),CAM_FLK_CON,FLK_MODE)=reinterpret_cast<ISP_CAM_FLK_CON*>(REG_INFO_VALUE_PTR(CAM_FLK_CON))->FLK_MODE;
		ISP_BITS(getIspReg(drv_mode), CAM_CTL_EN1, FLK_EN) =reinterpret_cast<ISP_CAM_FLK_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->FLK_EN;
		ISP_BITS(getIspReg(drv_mode),CAM_CTL_EN1_SET,FLK_EN_SET)=reinterpret_cast<ISP_CAM_CTL_EN1_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1_SET))->FLK_EN_SET;
		ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_EN,ESFKO_EN)=reinterpret_cast<ISP_CAM_CTL_DMA_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN))->ESFKO_EN;
		ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_EN_SET,ESFKO_EN_SET)=reinterpret_cast<ISP_CAM_CTL_DMA_EN_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN_SET))->ESFKO_EN_SET;
		ISP_BITS(getIspReg(drv_mode),CAM_CTL_DMA_INT,ESFKO_DONE_EN)=reinterpret_cast<ISP_CAM_CTL_DMA_INT*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN ;
		ISP_BITS(getIspReg(drv_mode),CAM_CTL_INT_EN,FLK_DON_EN)=reinterpret_cast<ISP_CAM_CTL_INT_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_INT_EN))->FLK_DON_EN ;
#endif

	m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_ESFKO);
	m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_FLKI);
	m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_ISP_FLK);
	m_pIspDrv->cqAddModule(ISP_DRV_CQ0, CAM_TOP_CTL_01);

    //FLICKER_LOG("[apply 22]: ,FLK_MODE:%d ,FLK_EN:%d ,FLK_EN_SET:%d,ESFKO_DMA_EN:%d\n",  ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_FLK_CON,FLK_MODE),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1,FLK_EN),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1_SET,FLK_EN_SET),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_DMA_EN,ESFKO_EN));
	//MY_LOG_IF(ENABLE_MY_LOG,"[apply 23]: ,FLK_MODE:%d ,FLK_EN:%d ,FLK_EN_SET:%d,ESFKO_DMA_EN:%d\n",  ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_FLK_CON,FLK_MODE), (int) ISP_REG(getIspReg(ISPDRV_MODE_ISP), CAM_FLK_CON),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1,FLK_EN),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_EN1_SET,FLK_EN_SET),ISP_BITS(getIspReg(ISPDRV_MODE_ISP),CAM_CTL_DMA_EN,ESFKO_EN));

#endif

    return  MTRUE;//writeRegs(CAM_ISP_FLK, ISPDRV_MODE_CQ0, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
}


MVOID
ISP_MGR_FLK_CONFIG_T::
enableFlk(MBOOL enable)
{
#if 0
		reinterpret_cast<ISP_CAM_FLK_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1))->FLK_EN = enable;
		reinterpret_cast<ISP_CAM_CTL_EN1_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_EN1_SET))->FLK_EN_SET = enable;
		reinterpret_cast<ISP_CAM_CTL_DMA_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN))->ESFKO_EN = enable;
		reinterpret_cast<ISP_CAM_CTL_DMA_EN_SET*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_EN_SET))->ESFKO_EN_SET = enable;
		reinterpret_cast<ISP_CAM_FLK_CON*>(REG_INFO_VALUE_PTR(CAM_FLK_CON))->FLK_MODE = 0;
		reinterpret_cast<ISP_CAM_CTL_DMA_INT*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN = enable;
		reinterpret_cast<ISP_CAM_CTL_INT_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_INT_EN))->FLK_DON_EN = enable;
		//reinterpret_cast<ISP_CAM_LSC_EN*>(REG_INFO_VALUE_PTR(CAM_CTL_DMA_INT))->ESFKO_DONE_EN = enable;
		apply();
#endif
}
MVOID
ISP_MGR_FLK_CONFIG_T::
SetFLKWin(MINT32 offsetX,MINT32 offsetY , MINT32 sizeX ,MINT32 sizeY)
{

}
MVOID
ISP_MGR_FLK_CONFIG_T::
SetFKO_DMA_Addr(MINT32 address,MINT32 size)
{

}
