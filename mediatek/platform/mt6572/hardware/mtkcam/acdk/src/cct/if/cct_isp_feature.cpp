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
#define LOG_TAG "isp_cctop"
//
#include <utils/Errors.h>
#include <cutils/xlog.h>

#include "cct_feature.h"
#include "isp_drv.h"
#include "cct_if.h"
#include "cct_imp.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "lsc_mgr.h"


/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    { XLOGD(fmt, ##arg);printf(fmt, ##arg);}
//#define MY_LOG(fmt, arg...)    XLOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)


/*******************************************************************************
*
********************************************************************************/

MBOOL  CctCtrl::updateIspRegs(MUINT32 const u4Category/*= 0xFFFFFFFF*/, MUINT32 const u4Index/*= 0xFFFFFFFF*/)
{
#if 0
    MBOOL fgRet = MFALSE;

    MBOOL fgIsDynamicISPEnabled = false;
    MBOOL fgIsDynamicCCMEnabled = false;
    MBOOL fgDisableDynamic = false; //  Disable Dynamic Data.

    //  (1) Save all index.
    ISP_NVRAM_REG_INDEX_T BackupIdx = m_rISPRegsIdx;

    //  (2) Modify a specific index.
#define MY_SET_ISP_REG_INDEX(_category)\
    case EIspReg_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  MFALSE;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch (u4Category)
    {
    MY_SET_ISP_REG_INDEX(LSC);
    MY_SET_ISP_REG_INDEX(OBC);
    MY_SET_ISP_REG_INDEX(CFA);
    MY_SET_ISP_REG_INDEX(BPC);
    MY_SET_ISP_REG_INDEX(NR1);
    MY_SET_ISP_REG_INDEX(ANR);
    MY_SET_ISP_REG_INDEX(EE);
    //MY_SET_ISP_REG_INDEX(Saturation);
    //MY_SET_ISP_REG_INDEX(Contrast);
    //MY_SET_ISP_REG_INDEX(Hue);
    MY_SET_ISP_REG_INDEX(CCM);
    MY_SET_ISP_REG_INDEX(GGM);
    default:
        break;
    }

    //  (3) Save the current dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicISP;
    cmd_GetDynamicISP.eCmd        = NSIspTuning::ECmd_GetDynamicTuning;
    cmd_GetDynamicISP.pOutBuf     = &fgIsDynamicISPEnabled;
    cmd_GetDynamicISP.u4OutBufSize= sizeof(MBOOL);
    //  (4) Save the current dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicCCM;
    cmd_GetDynamicCCM.eCmd        = NSIspTuning::ECmd_GetDynamicCCM;
    cmd_GetDynamicCCM.pOutBuf     = &fgIsDynamicCCMEnabled;
    cmd_GetDynamicCCM.u4OutBufSize= sizeof(MBOOL);
    //  (5) Disable the dynamic ISP.
    NSIspTuning::CmdArg_T cmd_DisableDynamicISP;
    cmd_DisableDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_DisableDynamicISP.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (6) Disable the dynamic CCM.
    NSIspTuning::CmdArg_T cmd_DisableDynamicCCM;
    cmd_DisableDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_DisableDynamicCCM.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicCCM.u4InBufSize= sizeof(MBOOL);
    //  (8) Restore the dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicISP;
    cmd_RestoreDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_RestoreDynamicISP.pInBuf     = &fgIsDynamicISPEnabled;
    cmd_RestoreDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (9) Restore the dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicCCM;
    cmd_RestoreDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_RestoreDynamicCCM.pInBuf     = &fgIsDynamicCCMEnabled;
    cmd_RestoreDynamicCCM.u4InBufSize= sizeof(MBOOL);


    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicISP))     //(3)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicCCM))     //(4)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicISP)) //(5)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicCCM)) //(6)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_VALIDATE_FRAME, true)                                  //(7) Validate
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicISP)) //(8)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicCCM)) //(9)
        )
    {
        goto lbExit;
    }

    MY_LOG("dynamic flags:(isp, ccm)=(%d, %d)", fgIsDynamicISPEnabled, fgIsDynamicCCMEnabled);

    fgRet = MTRUE;

lbExit:
    //  (10) Restore all index.
    m_rISPRegsIdx = BackupIdx;
#endif

    return  MTRUE;

}


/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPReadIspReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{

    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_READ_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = 0xFFFFFFFF;

    err = m_pIspDrv->readRegs(&isp_reg, 1);

    if (err < 0) {
        MY_ERR("[CCTOPReadIspReg] readIspRegs() error");
        return err;
    }

    pIspRegInfoOut->RegData = isp_reg.Data;
    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);
    MY_LOG("[CCTOPReadIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    return err;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPWriteIspReg(MVOID *puParaIn)
{

    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    ISP_DRV_REG_IO_STRUCT isp_reg;

    MY_LOG("ACDK_CCT_OP_ISP_WRITE_REG\n");

    isp_reg.Addr = pIspRegInfoIn->RegAddr;
    isp_reg.Data = pIspRegInfoIn->RegData;

    err = m_pIspDrv->writeRegs(&isp_reg, 1);
    if (err < 0) {
        MY_ERR("[CCTOPWriteIspReg]writeRegs() error");
        return err;
    }

    MY_LOG("[CCTOPWriteIspReg] regAddr = %x, regData = %x\n", (MUINT32)isp_reg.Addr, (MUINT32)isp_reg.Data);

    return err;

}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID )
{
    if  ( sizeof(MUINT32) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<MUINT32 *>(puParaOut) = 0x65898A00;;
    *pu4RealParaOutLen = sizeof(MUINT32);

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG )
{
    return CCTOPReadIspReg((MVOID *)puParaIn, (MVOID *)puParaOut, pu4RealParaOutLen);
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG )
{
    return CCTOPWriteIspReg((MVOID *)puParaIn);
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;

#define MY_SET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch  (eCategory)
    {
        MY_SET_TUNING_INDEX(OBC);
        MY_SET_TUNING_INDEX(NR1);
        MY_SET_TUNING_INDEX(CFA);
        MY_SET_TUNING_INDEX(ANR);
        MY_SET_TUNING_INDEX(CCR);
        MY_SET_TUNING_INDEX(EE);
        MY_SET_TUNING_INDEX(NR3D);
        MY_SET_TUNING_INDEX(MFB);

        default:
            return  CCTIF_BAD_PARAM;
    }

    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY   i_type;
    typedef MUINT32                     o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    o_type&      rIndex    = *reinterpret_cast<o_type*>(puParaOut);


#define MY_GET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        rIndex = m_rISPRegsIdx._category;\
        break

    switch  (eCategory)
    {
        MY_GET_TUNING_INDEX(OBC);
        MY_GET_TUNING_INDEX(NR1);
        MY_GET_TUNING_INDEX(CFA);
        MY_GET_TUNING_INDEX(ANR);
        MY_GET_TUNING_INDEX(CCR);
        MY_GET_TUNING_INDEX(EE);
        MY_GET_TUNING_INDEX(NR3D);
        MY_GET_TUNING_INDEX(MFB);

        default:
            return  CCTIF_BAD_PARAM;
    }

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM )
{
    typedef ISP_NVRAM_MFB_MIXER_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MFB_MIXER_PARAM_T *rRegs = reinterpret_cast<o_type *>(puParaOut)->param;

    ::memcpy(rRegs, m_rBuf_ISP.ISPMfbMixer.param, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM )
{

    typedef ISP_NVRAM_MFB_MIXER_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_MFB_MIXER_PARAM_T *rRegs = reinterpret_cast<i_type *>(puParaIn)->param;
    ::memcpy(m_rBuf_ISP.ISPMfbMixer.param, rRegs, sizeof(i_type));

    return  CCTIF_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
pu4RealParaOutLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF )
{
    typedef ISP_NVRAM_CCM_POLY22_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<o_type *>(puParaOut);

    ::memcpy(rRegs, &m_rBuf_ISP.ISPCcmPoly22, sizeof(o_type));
    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ISP_NVRAM_MFB_MIXER_STRUCT
u4ParaInLen
    sizeof(ISP_NVRAM_MFB_MIXER_STRUCT);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF )
{

    typedef ISP_NVRAM_CCM_POLY22_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    ISP_NVRAM_CCM_POLY22_STRUCT *rRegs = reinterpret_cast<i_type *>(puParaIn);
    ::memcpy(&m_rBuf_ISP.ISPCcmPoly22, rRegs, sizeof(i_type));

    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ACDK_CCT_ISP_GET_TUNING_PARAS
u4ParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS )
{
    typedef ACDK_CCT_ISP_GET_TUNING_PARAS o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ACDK_CCT_ISP_NVRAM_REG& rRegs = reinterpret_cast<o_type*>(puParaOut)->stIspNvramRegs;

#define MY_GET_TUNING_PARAS(_category)\
    ::memcpy(rRegs._category, m_rISPRegs._category, sizeof(rRegs._category))

    MY_GET_TUNING_PARAS(OBC);
    MY_GET_TUNING_PARAS(NR1);
    MY_GET_TUNING_PARAS(LSC);
    MY_GET_TUNING_PARAS(CFA);
    MY_GET_TUNING_PARAS(ANR);
    MY_GET_TUNING_PARAS(CCR);
    MY_GET_TUNING_PARAS(EE);
    MY_GET_TUNING_PARAS(NR3D);
    MY_GET_TUNING_PARAS(MFB);

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_SET_TUNING_PARAS
u4ParaInLen
    sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS )
{
#if 0
    typedef ACDK_CCT_ISP_SET_TUNING_PARAS type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;
    ACDK_CCT_ISP_NVRAM_REG const&   rRegs       = reinterpret_cast<type const*>(puParaIn)->stIspNvramRegs;

    ISP_REG_CATEGORY eIspRegCategory;

#define MY_SET_TUNING_PARAS(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegs._category[u4Index] = rRegs._category[u4Index];\
        eIspRegCategory = EIspReg_##_category;\
        break

    switch  ( eCategory )
    {
    MY_SET_TUNING_PARAS(OBC);
    MY_SET_TUNING_PARAS(CFA);
    MY_SET_TUNING_PARAS(BPC);
    MY_SET_TUNING_PARAS(NR1);
    MY_SET_TUNING_PARAS(ANR);
    MY_SET_TUNING_PARAS(EE);
    //MY_SET_TUNING_PARAS(Saturation);
    //MY_SET_TUNING_PARAS(Contrast);
    //MY_SET_TUNING_PARAS(Hue);
    default:
        return  CCTIF_BAD_PARAM;
    }

    if  ( ! updateIspRegs(eIspRegCategory, u4Index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    typedef ACDK_CCT_ISP_SET_TUNING_PARAS type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;
    ACDK_CCT_ISP_NVRAM_REG const&   rRegs       = reinterpret_cast<type const*>(puParaIn)->stIspNvramRegs;

#define CHECK_INDEX_RANGE(_category)\
    if (IspNvramRegMgr::NUM_##_category <= u4Index)\
        return CCTIF_BAD_PARAM;\
    m_rISPRegs._category[u4Index] = rRegs._category[u4Index];\
    m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);

    switch(eCategory) {

        case EIsp_Category_OBC:
            CHECK_INDEX_RANGE(OBC);
            NSIspTuning::ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
            NSIspTuning::ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.OBC[u4Index]);
            NSIspTuning::ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_NR1:
            CHECK_INDEX_RANGE(NR1);

            NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCTEnable(MTRUE);
            NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.NR1[u4Index]);
            NSIspTuning::ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_LSC:
            CHECK_INDEX_RANGE(LSC);

            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).enableLsc(MTRUE);
            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.LSC[u4Index]);
            NSIspTuning::ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_CFA:
            CHECK_INDEX_RANGE(CFA);
            NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
            NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.CFA[u4Index]);
            NSIspTuning::ISP_MGR_CFA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_ANR:
            CHECK_INDEX_RANGE(ANR);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setANREnable(MTRUE);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.ANR[u4Index]);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_CCR:
            CHECK_INDEX_RANGE(CCR);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCREnable(MTRUE);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.CCR[u4Index]);
            NSIspTuning::ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_EE:
            CHECK_INDEX_RANGE(EE);
            NSIspTuning::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
            NSIspTuning::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.EE[u4Index]);
            NSIspTuning::ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_NR3D:
            CHECK_INDEX_RANGE(NR3D);
            NSIspTuning::ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.NR3D[u4Index]);
            NSIspTuning::ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        case EIsp_Category_MFB:
            CHECK_INDEX_RANGE(MFB);
            NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
            NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
            NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rRegs.MFB[u4Index]);
            NSIspTuning::ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
            break;

        default:
            return  CCTIF_BAD_PARAM;

    }

    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE )
{
#if 0
    MBOOL fgEnableDynamicTuning = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicTuning,
        pInBuf:             &fgEnableDynamicTuning,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    MY_LOG("Enable Dynamic Bypass!!\n");

    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass(MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE )
{
#if 0
    MBOOL fgEnableDynamicTuning = true;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicTuning,
        pInBuf:             &fgEnableDynamicTuning,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    MY_LOG("Disable Dynamic Bypass!!\n");

    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicBypass(MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF )
{
#if 0
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgIsDynamicTuningEnabled = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetDynamicTuning,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgIsDynamicTuningEnabled,
        u4OutBufSize:       sizeof(MBOOL),
        u4ActualOutSize:    0
    };

    if  (
        0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
    ||  sizeof(MBOOL) != cmd.u4ActualOutSize
        )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    reinterpret_cast<o_type*>(puParaOut)->Enable
        = fgIsDynamicTuningEnabled ? false : true;

    *pu4RealParaOutLen = sizeof(o_type);
#endif

    MINT32 bypass_en;

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    bypass_en = NSIspTuning::IspTuningMgr::getInstance().getDynamicBypass();
    if(bypass_en < 0)
    {
        return CCTIF_UNKNOWN_ERROR;
    }

    reinterpret_cast<o_type*>(puParaOut)->Enable = bypass_en;
    *pu4RealParaOutLen = sizeof(o_type);

    return CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE )
{
    typedef ACDK_CCT_GAMMA_TABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pGamma = reinterpret_cast<o_type*>(puParaOut);
    ISP_NVRAM_GGM_T ggm;

    NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ggm);

    for(int i=0;i<GAMMA_STEP_NO; i++) {
        pGamma->r_tbl[i] = ggm.rb_gmt.lut[i].R_GAMMA;
        pGamma->g_tbl[i] = ggm.g_gmt.lut[i].G_GAMMA;
        pGamma->b_tbl[i] = ggm.rb_gmt.lut[i].B_GAMMA;
    }

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE )
{
    typedef ACDK_CCT_GAMMA_TABLE_STRUCT i_type;
    int i;
    MUINT32 const index = m_rISPRegsIdx.GGM;

    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pGamma = reinterpret_cast<i_type*>(puParaIn);

    MY_LOG("[ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE]\n");

    for(i=0;i<GAMMA_STEP_NO; i++) {
        m_rISPRegs.GGM[index].rb_gmt.lut[i].R_GAMMA = pGamma->r_tbl[i];
        m_rISPRegs.GGM[index].g_gmt.lut[i].G_GAMMA = pGamma->g_tbl[i];
        m_rISPRegs.GGM[index].rb_gmt.lut[i].B_GAMMA = pGamma->b_tbl[i];
    }

    NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
    NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(m_rISPRegs.GGM[index]);
    NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS )
{
#if 0
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    MBOOL fgEnable = reinterpret_cast<type*>(puParaIn)->Enable ? false : true;

    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetForceCtrl_Gamma,
        pInBuf:             &fgEnable,
        u4InBufSize:        sizeof(fgEnable),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    if  ( ! updateIspRegs() )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnable = reinterpret_cast<type*>(puParaIn)->Enable ? MFALSE : MTRUE;

    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgEnable);
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG )
{
#if 0
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnable = MFALSE;

    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetForceCtrl_Gamma,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgEnable,
        u4OutBufSize:       sizeof(fgEnable),
        u4ActualOutSize:    0
    };

    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
        ||  sizeof(fgEnable) != cmd.u4ActualOutSize
        )
    {
        MY_ERR("[ERR] (u4OutBufSize, u4ActualOutSize)=(%d, %d)", sizeof(fgEnable), cmd.u4ActualOutSize);
        return  CCTIF_INVALID_DRIVER;
    }

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    reinterpret_cast<o_type*>(puParaOut)->Enable
        = fgEnable ? false : true;

    *pu4RealParaOutLen = sizeof(o_type);
#endif

    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    reinterpret_cast<o_type*>(puParaOut)->Enable
        = (NSIspTuning::ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable()) ? true : false;
    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM )
{
#if 1
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    pDst->M11 = rSrc.conv0a.bits.G2G_CNV_00;
    pDst->M12 = rSrc.conv0a.bits.G2G_CNV_01;
    pDst->M13 = rSrc.conv0b.bits.G2G_CNV_02;
    pDst->M21 = rSrc.conv1a.bits.G2G_CNV_10;
    pDst->M22 = rSrc.conv1a.bits.G2G_CNV_11;
    pDst->M23 = rSrc.conv1b.bits.G2G_CNV_12;
    pDst->M31 = rSrc.conv2a.bits.G2G_CNV_20;
    pDst->M32 = rSrc.conv2a.bits.G2G_CNV_21;
    pDst->M33 = rSrc.conv2b.bits.G2G_CNV_22;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("index 0x%03X", index);
    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);

#else

    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    NSIspTuning::ISP_MGR_CCM_T::getInstance(NSIspTuning::ESensorDev_Main).reset();
    NSIspTuning::ISP_MGR_CCM_T::getInstance(NSIspTuning::ESensorDev_Main).get(ccm);

    pDst->M11 = rSrc.conv0a.bits.G2G_CNV_00;
    pDst->M12 = rSrc.conv0a.bits.G2G_CNV_01;
    pDst->M13 = rSrc.conv0b.bits.G2G_CNV_02;
    pDst->M21 = rSrc.conv1a.bits.G2G_CNV_10;
    pDst->M22 = rSrc.conv1a.bits.G2G_CNV_11;
    pDst->M23 = rSrc.conv1b.bits.G2G_CNV_12;
    pDst->M31 = rSrc.conv2a.bits.G2G_CNV_20;
    pDst->M32 = rSrc.conv2a.bits.G2G_CNV_21;
    pDst->M33 = rSrc.conv2b.bits.G2G_CNV_22;

    *pu4RealParaOutLen = sizeof(o_type);


#endif

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_CCM_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_CCM_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

#if 0
    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];
    type*const       pSrc = reinterpret_cast<type*>(puParaIn);

    rDst.conv0a.bits.G2G_CNV_00 = pSrc->M11;
    rDst.conv0a.bits.G2G_CNV_01 = pSrc->M12;
    rDst.conv0b.bits.G2G_CNV_02 = pSrc->M13;
    rDst.conv1a.bits.G2G_CNV_10 = pSrc->M21;
    rDst.conv1a.bits.G2G_CNV_11 = pSrc->M22;
    rDst.conv1b.bits.G2G_CNV_12 = pSrc->M23;
    rDst.conv2a.bits.G2G_CNV_20 = pSrc->M31;
    rDst.conv2a.bits.G2G_CNV_21 = pSrc->M32;
    rDst.conv2b.bits.G2G_CNV_22 = pSrc->M33;

    //  write to register.
    if  ( ! updateIspRegs(EIspReg_CCM, index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
    return  CCTIF_NO_ERROR;
#endif

    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];
    type*const       pSrc = reinterpret_cast<type*>(puParaIn);

    rDst.conv0a.bits.G2G_CNV_00 = pSrc->M11;
    rDst.conv0a.bits.G2G_CNV_01 = pSrc->M12;
    rDst.conv0b.bits.G2G_CNV_02 = pSrc->M13;
    rDst.conv1a.bits.G2G_CNV_10 = pSrc->M21;
    rDst.conv1a.bits.G2G_CNV_11 = pSrc->M22;
    rDst.conv1b.bits.G2G_CNV_12 = pSrc->M23;
    rDst.conv2a.bits.G2G_CNV_20 = pSrc->M31;
    rDst.conv2a.bits.G2G_CNV_21 = pSrc->M32;
    rDst.conv2b.bits.G2G_CNV_22 = pSrc->M33;

    NSIspTuning::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    NSIspTuning::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
    NSIspTuning::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rDst);
    NSIspTuning::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    MUINT32 u4Index
u4ParaInLen
    sizeof(MUINT32)
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef MUINT32             i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const index = *reinterpret_cast<i_type*>(puParaIn);
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    pDst->M11 = rSrc.conv0a.bits.G2G_CNV_00;
    pDst->M12 = rSrc.conv0a.bits.G2G_CNV_01;
    pDst->M13 = rSrc.conv0b.bits.G2G_CNV_02;
    pDst->M21 = rSrc.conv1a.bits.G2G_CNV_10;
    pDst->M22 = rSrc.conv1a.bits.G2G_CNV_11;
    pDst->M23 = rSrc.conv1b.bits.G2G_CNV_12;
    pDst->M31 = rSrc.conv2a.bits.G2G_CNV_20;
    pDst->M32 = rSrc.conv2a.bits.G2G_CNV_21;
    pDst->M33 = rSrc.conv2b.bits.G2G_CNV_22;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_SET_NVRAM_CCM
u4ParaInLen
    sizeof(ACDK_CCT_SET_NVRAM_CCM)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM )
{
    typedef ACDK_CCT_SET_NVRAM_CCM type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type& rInParam = *reinterpret_cast<type*>(puParaIn);
    MUINT32 const index = rInParam.u4Index;
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];

    rDst.conv0a.bits.G2G_CNV_00 = rInParam.ccm.M11;
    rDst.conv0a.bits.G2G_CNV_01 = rInParam.ccm.M12;
    rDst.conv0b.bits.G2G_CNV_02 = rInParam.ccm.M13;
    rDst.conv1a.bits.G2G_CNV_10 = rInParam.ccm.M21;
    rDst.conv1a.bits.G2G_CNV_11 = rInParam.ccm.M22;
    rDst.conv1b.bits.G2G_CNV_12 = rInParam.ccm.M23;
    rDst.conv2a.bits.G2G_CNV_20 = rInParam.ccm.M31;
    rDst.conv2a.bits.G2G_CNV_21 = rInParam.ccm.M32;
    rDst.conv2b.bits.G2G_CNV_22 = rInParam.ccm.M33;
/*
    //  For the compatibility to old ways, needn't write to register/nvram.
    //  write to register.
    if  ( ! updateIspRegs(EIspReg_CCM, index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
*/
    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to set: %d", index);
    for (MUINT32 i = 0; i < ISP_NVRAM_CCM_T::COUNT; i++)
    {
        MY_LOG("CCM: [%d] 0x%06X", i, rDst.set[i]);
    }
    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
pu4RealParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        pDst->ccm[i].M11 = m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_00;
        pDst->ccm[i].M12 = m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_01;
        pDst->ccm[i].M13 = m_rISPRegs.CCM[i].conv0b.bits.G2G_CNV_02;
        pDst->ccm[i].M21 = m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_10;
        pDst->ccm[i].M22 = m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_11;
        pDst->ccm[i].M23 = m_rISPRegs.CCM[i].conv1b.bits.G2G_CNV_12;
        pDst->ccm[i].M31 = m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_20;
        pDst->ccm[i].M32 = m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_21;
        pDst->ccm[i].M33 = m_rISPRegs.CCM[i].conv2b.bits.G2G_CNV_22;
    }
    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;

}

/*
puParaIn
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaInLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_00 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].conv0a.bits.G2G_CNV_01 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].conv0b.bits.G2G_CNV_02 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_10 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].conv1a.bits.G2G_CNV_11 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].conv1b.bits.G2G_CNV_12 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_20 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].conv2a.bits.G2G_CNV_21 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].conv2b.bits.G2G_CNV_22 = pSrc->ccm[i].M33;
    }

    return  CCTIF_NO_ERROR;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM )
{
#if 0
    MBOOL fgEnableDynamicCCM = true;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif

    MY_LOG("Enable Dynamic CCM!!\n");

    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicCCM(MTRUE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM )
{
#if 0
    MBOOL fgEnableDynamicCCM = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif
    MY_LOG("Disable Dynamic CCM!!\n");

    if(NSIspTuning::IspTuningMgr::getInstance().setDynamicCCM(MFALSE) == MTRUE)
        return CCTIF_NO_ERROR;
    else
        return CCTIF_UNKNOWN_ERROR;

}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS )
{
#if 0
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnableDynamicCCM = reinterpret_cast<i_type*>(puParaIn)->Enable;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
#endif
    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS )
{
#if 0
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgIsDynamicCCMEnabled = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetDynamicCCM,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgIsDynamicCCMEnabled,
        u4OutBufSize:       sizeof(MBOOL),
        u4ActualOutSize:    0
    };

    if  (
        0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
    ||  sizeof(MBOOL) != cmd.u4ActualOutSize
        )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    reinterpret_cast<o_type*>(puParaOut)->Enable = fgIsDynamicCCMEnabled;

    *pu4RealParaOutLen = sizeof(o_type);
#endif


    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    MINT32 en;
    CCTIF_ERROR_ENUM err_status;

    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MY_LOG("[ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS]\n");

    en = NSIspTuning::IspTuningMgr::getInstance().getDynamicCCM();
    if(en < 0)
    {
        err_status = CCTIF_UNKNOWN_ERROR;
    }
    else {
        reinterpret_cast<o_type*>(puParaOut)->Enable = en;
        err_status = CCTIF_NO_ERROR;
    }

    *pu4RealParaOutLen = sizeof(o_type);

    return err_status;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(MUINT32);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE )
{
    typedef MUINT32 i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const u4Index = *reinterpret_cast<i_type const*>(puParaIn);

    MY_LOG("[ACDK_CCT_OP_SET_CCM_MODE] CCM Index: (old, new)=(%d, %d)", m_rISPRegsIdx.CCM, u4Index);

    if  ( IspNvramRegMgr::NUM_CCM <= u4Index )
    {
        return  CCTIF_BAD_PARAM;
    }

    m_rISPRegsIdx.CCM = static_cast<MUINT8>(u4Index);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
u4ParaInLen
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE )
{
    typedef MUINT32 o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<o_type*>(puParaOut) = m_rISPRegsIdx.CCM;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("[ACDK_CCT_OP_GET_CCM_MODE] Current CCM Index: %d", m_rISPRegsIdx.CCM);
    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CctCtrl::
dumpIspReg(MUINT32 const u4Addr) const
{
#if 1
    MY_LOG("[dumpIspReg] isp reg:%04X = 0x%08X", u4Addr, m_pIspDrv->readReg(u4Addr));
#endif
}


MVOID
CctCtrl::
setIspOnOff_OBC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.OBC;
    if  (fgOn)
    {
        if  ( ! m_fgEnabled_OB )
        {
            //  OB: from DISABLE to ENABLE
            //  restore the backup.
            m_rISPRegs.OBC[u4Index].set[0] = m_u4Backup_OB;
        }
        m_fgEnabled_OB = MTRUE;
    }
    else
    {
        if  ( m_fgEnabled_OB )
        {
            //  OB: from ENABLE to DISABLE.
            //  (1) backup the enabled OB.
            m_u4Backup_OB = m_rISPRegs.OBC[u4Index].set[0];
            //  (2) disable OB by assigning 0.
            m_rISPRegs.OBC[u4Index].set[0] = 0;
        }
        m_fgEnabled_OB = MFALSE;
    }

    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

}

MVOID
CctCtrl::
setIspOnOff_BPC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BPC;

    m_rISPRegs.BPC[u4Index].con.bits.BPC_ENABLE = fgOn;

	ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setBPCEnable(fgOn);
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}


MVOID
CctCtrl::
setIspOnOff_NR1(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR1;

    m_rISPRegs.NR1[u4Index].con.bits.NR1_CT_EN = fgOn;

	ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCTEnable(fgOn);
    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}

MVOID
CctCtrl::
setIspOnOff_CFA(MBOOL const fgOn)
{
    //N.A.
}


MVOID
CctCtrl::
setIspOnOff_CCM(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.CCM;

	ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}

MVOID
CctCtrl::
setIspOnOff_GGM(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.GGM;

	ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}



MVOID
CctCtrl::
setIspOnOff_ANR(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.ANR;
    ISP_CAM_ANR_CON1_T& ANRCtrl1 = m_rISPRegs.ANR[u4Index].con1.bits;

    ANRCtrl1.ANR_ENC = fgOn;
    ANRCtrl1.ANR_ENY = fgOn;

#if 0
    ISP_NR2_CTRL_T& rNR2Ctrl = m_rISPRegs.ANR[u4Index].ctrl.bits;
    rNR2Ctrl.ENC    = fgOn;
    rNR2Ctrl.ENY    = fgOn;
#endif

    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setANREnable(fgOn);
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

}

MVOID
CctCtrl::
setIspOnOff_CCR(MBOOL const fgOn)
{
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCREnable(fgOn);
    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

}


MVOID
CctCtrl::
setIspOnOff_EE(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;

    ISP_CAM_SEEE_SRK_CTRL_T &EECtrl_SRK = m_rISPRegs.EE[u4Index].srk_ctrl.bits;
    ISP_CAM_SEEE_CLIP_CTRL_T &EECtrl_Clip = m_rISPRegs.EE[u4Index].clip_ctrl.bits;

    EECtrl_SRK.USM_OVER_SHRINK_EN = fgOn;
    EECtrl_Clip.USM_OVER_CLIP_EN = fgOn;

#if 0
    ISP_EE_CTRL_T&  rEECtrl = m_rISPRegs.EE[u4Index].ee_ctrl.bits;
    rEECtrl.YEDGE_EN = fgOn;
    rEECtrl.RGBEDGE_EN = 0;//unused
#endif

    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}

MVOID
CctCtrl::
setIspOnOff_NR3D(MBOOL const fgOn)
{
    //N.A.
}

MVOID
CctCtrl::
setIspOnOff_MFB(MBOOL const fgOn)
{
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);
}

MBOOL
CctCtrl::
getIspOnOff_OBC() const
{
    return ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctCtrl::
getIspOnOff_BPC() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isBPCEnable();
}


MBOOL
CctCtrl::
getIspOnOff_NR1() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCTEnable();
}

MBOOL
CctCtrl::
getIspOnOff_CFA() const
{
    return MTRUE;
}

MBOOL
CctCtrl::
getIspOnOff_CCM() const
{
    return ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctCtrl::
getIspOnOff_GGM() const
{
    return  ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}


MBOOL
CctCtrl::
getIspOnOff_ANR() const
{
#if 0
    MUINT32 const u4Index = m_rISPRegsIdx.NR2;
    ISP_NR2_CTRL_T const& rCtrl = m_rISPRegs.NR2[u4Index].ctrl.bits;
    return  (0 != rCtrl.ENC)
        ||  (0 != rCtrl.ENY);
#endif
    return ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isANREnable();
}

MBOOL
CctCtrl::
getIspOnOff_CCR() const
{
    return ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCREnable();
}

MBOOL
CctCtrl::
getIspOnOff_EE() const
{
    return ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctCtrl::
getIspOnOff_NR3D() const
{
    return MTRUE;
}

MBOOL
CctCtrl::
getIspOnOff_MFB() const
{
    return ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}


MINT32
CctCtrl::
setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn)
{
#define SET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        setIspOnOff_##_category(fgOn);\
        MY_LOG("[setIspOnOff] < %s >", #_category);\
        break

    switch  ( u4Category )
    {
        SET_ISP_ON_OFF(OBC);
        SET_ISP_ON_OFF(NR1);
        SET_ISP_ON_OFF(CFA);
        SET_ISP_ON_OFF(ANR);
        SET_ISP_ON_OFF(CCR);
        SET_ISP_ON_OFF(EE);
        SET_ISP_ON_OFF(NR3D);
        SET_ISP_ON_OFF(MFB);

        default:
            MY_ERR("[setIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }

    return  CCTIF_NO_ERROR;
}


MINT32
CctCtrl::
getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const
{
#define GET_ISP_ON_OFF(_category)\
    case EIsp_Category_##_category:\
        MY_LOG("[getIspOnOff] < %s >", #_category);\
        rfgOn = getIspOnOff_##_category();\
        break

    switch  ( u4Category )
    {
        GET_ISP_ON_OFF(OBC);
        GET_ISP_ON_OFF(NR1);
        GET_ISP_ON_OFF(CFA);
        GET_ISP_ON_OFF(ANR);
        GET_ISP_ON_OFF(CCR);
        GET_ISP_ON_OFF(EE);
        GET_ISP_ON_OFF(NR3D);
        GET_ISP_ON_OFF(MFB);

        default:
            MY_ERR("[getIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 1);

    MY_LOG("[-ACDK_CCT_OP_SET_ISP_ON] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 0);

    MY_LOG("[ACDK_CCT_OP_SET_ISP_OFF] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY       i_type;
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    MBOOL&       rfgEnable = reinterpret_cast<o_type*>(puParaOut)->Enable;

    MINT32 const err = getIspOnOff(eCategory, rfgEnable);

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[-ACDK_CCT_OP_GET_ISP_ON_OFF] (eCategory, rfgEnable)=(%d, %d)", eCategory, rfgEnable);
    return  err;
}


/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )");

    err = m_rBufIf_ISP.refresh(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM] m_rBufIf_ISP.refresh() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )");

    err = m_rBufIf_ISP.flush(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SAVE_TO_NVRAM] m_rBufIf_ISP.flush() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE )
{
#if 0
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaIn);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_lo[u4Offset];
        break;
    case 1:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_md[u4Offset];
        break;
    case 2:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_hi[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_fw, pBuf_pc, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    return  CCTIF_NO_ERROR;
#else

    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaIn);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;

    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_lo[u4Offset];
        break;
    case 1:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_md[u4Offset];
        break;
    case 2:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_hi[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_fw, pBuf_pc, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).loadLut((MUINT32 *)pBuf_fw);
    NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(MTRUE);
    NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

    return  CCTIF_NO_ERROR;

#endif


}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE )
{
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaOut);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_lo[u4Offset];
        break;
    case 1:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_md[u4Offset];
        break;
    case 2:
        pBuf_fw = &m_rISPPca.PCA_LUTS.lut_hi[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_pc, pBuf_fw, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));
    *pu4RealParaOutLen = sizeof(type);

    return  CCTIF_NO_ERROR;


}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA )
{
#if 0
    typedef ACDK_CCT_ACCESS_PCA_CONFIG i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pAccess = reinterpret_cast<i_type*>(puParaIn);

    m_rISPPca.Config.ctrl.bits.EN = pAccess->EN;

    if  ( ! updateIspRegs() )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    dumpIspReg(0x0630);
    dumpIspReg(0x0634);
    dumpIspReg(0x0638);
    MY_LOG("-[ACDK_CCT_OP_ISP_SET_PCA_PARA] PCA_EN(%d)\n", pAccess->EN);
    return  CCTIF_NO_ERROR;
#else

    typedef ACDK_CCT_ACCESS_PCA_CONFIG i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pAccess = reinterpret_cast<i_type*>(puParaIn);

    NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable((MBOOL)pAccess->EN);
    NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalPreview);

    return  CCTIF_NO_ERROR;

#endif

}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA )
{
#if 0
    typedef ACDK_CCT_ACCESS_PCA_CONFIG o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pAccess = reinterpret_cast<o_type*>(puParaOut);

    pAccess->EN = m_rISPPca.Config.ctrl.bits.EN;
    *pu4RealParaOutLen = sizeof(o_type);

    dumpIspReg(0x0630);
    dumpIspReg(0x0634);
    dumpIspReg(0x0638);
    MY_LOG("-[ACDK_CCT_OP_ISP_GET_PCA_PARA] PCA_EN(%d)\n", pAccess->EN);
    return  CCTIF_NO_ERROR;
#else
    return  NSIspTuning::ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
#endif

}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/


typedef struct {
  MUINT32 TUNING_MODE;
  MUINT32 SHADING_MODE;
  MUINT32 ISPPROFILE;
} SHADING_MODE_MATRIX;

SHADING_MODE_MATRIX Mapping[] = {
  {CAMERA_TUNING_PREVIEW_SET,0, NSIspTuning::EIspProfile_NormalPreview}, //LSC_SCENARIO_01
  {CAMERA_TUNING_CAPTURE_SET,2, NSIspTuning::EIspProfile_NormalCapture}, //LSC_SCENARIO_04
  {CAMERA_TUNING_VIDEO_SET,  3, NSIspTuning::EIspProfile_VideoPreview}, //LSC_SCENARIO_09_17
  0,
};

static int GetShadingMode(int tuning_mode) {
    int i = 0;
    do {
      if (Mapping[i].TUNING_MODE == tuning_mode) {
            MY_LOG("-[%s] Shading Mode(%d)\n", __FUNCTION__,
                    Mapping[i].SHADING_MODE);
        return Mapping[i].SHADING_MODE;
      } else
        i++;
    } while(i < CAMERA_TUNING_BINNING_SET);
    return 0;
}

static int GetISPProfile(int tuning_mode) {
    int i = 0;
    do {
        if (Mapping[i].TUNING_MODE == tuning_mode) {
            MY_LOG("-[%s] ISP Profile(%d)\n", __FUNCTION__,
                    Mapping[i].ISPPROFILE);
            return Mapping[i].ISPPROFILE;
        } else
            i++;
    } while(i < CAMERA_TUNING_BINNING_SET);
    return 0;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF )
{

    MUINT32 u4Mode;
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);
    ISP_NVRAM_LSC_T tmp;
    u4Mode = GetShadingMode(pShadingPara->Mode);
    NSIspTuning::LscMgr::LSCParameter &LSC=  LscMgr::getInstance()->getLscNvram();
    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)u4Mode);


    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).get(tmp);
    LSC[u4Mode].lsc_en.bits.LSC_EN = tmp.lsc_en.bits.LSC_EN = pShadingPara->Enable;
    LSC[u4Mode].lsci_en.bits.LSCI_EN = tmp.lsci_en.bits.LSCI_EN = pShadingPara->Enable;
    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(tmp);
    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingPara->Mode));


    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF]"
        " (Shading mode pre/cap, on/off)=(%d,%d)\n"
        , u4Mode
        , m_rISPRegs.LSC[u4Mode].lsc_en.bits.LSC_EN);

    return  CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF )
{

    MUINT32 u4Mode = 0;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);

    ISP_NVRAM_LSC_T tmp;
    //NSIspTuning::LscMgr::LSCParameter &LSC=  LscMgr::getInstance()->getLscNvram();
    u4Mode = GetShadingMode(pShadingPara->Mode);
    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)u4Mode);
    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).get(tmp);

    //  enable/disable       : (1, 0)
    pShadingPara->Enable = tmp.lsc_en.bits.LSC_EN;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF]"
        " (Shading mode pre/cap, on/off)=(%d, %d)\n"
        , u4Mode
        , pShadingPara->Enable
        );

    return  CCTIF_NO_ERROR;

}
/*******************************************************************************
* Because CCT tool is working on preview mode (Lsc_mgr.m_u4Mode = 0)
* 1.
*    Capture parameters will not be update to isp register
*    (since Lsc_mgr.m_u4Mode doesn't changed) till capture command
* 2.
*     Preivew parameters will be updated to reigster immediately at "prepareHw_PerFrame_Shading()"
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA )
{
    MUINT32 u4Mode;
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_SHADING_COMP_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);

    NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MFALSE);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA] (Shading mode pre/cap)=(%d)\n", pShadingPara->SHADING_MODE);
//	    LscMgr::getInstance()->setIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingPara->SHADING_MODE));
//	//	    LscMgr::getInstance()->setCTIdx(0);
//	    LscMgr::getInstance()->SetTBAToISP();
    NSIspTuning::LscMgr::LSCParameter &LSC=  LscMgr::getInstance()->getLscNvram();

    u4Mode = (MUINT32)LscMgr::getInstance()->getLscScenarioByIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingPara->SHADING_MODE));
    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)u4Mode);
    LSC[u4Mode].lsc_en.bits.LSC_EN           = LSC[u4Mode].lsci_en.bits.LSCI_EN = pShadingPara->pShadingComp->SHADING_EN;
    LSC[u4Mode].ctl2.bits.SDBLK_XNUM         = pShadingPara->pShadingComp->SHADINGBLK_XNUM - 1;
    LSC[u4Mode].ctl3.bits.SDBLK_YNUM         = pShadingPara->pShadingComp->SHADINGBLK_YNUM - 1;
    LSC[u4Mode].ctl2.bits.SDBLK_WIDTH        = pShadingPara->pShadingComp->SHADINGBLK_WIDTH;
    LSC[u4Mode].ctl3.bits.SDBLK_HEIGHT       = pShadingPara->pShadingComp->SHADINGBLK_HEIGHT;
    LSC[u4Mode].baseaddr.bits.BASE_ADDR      =
        (UINT32)LscMgr::getInstance()->getPhyTBA((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingPara->SHADING_MODE));
    LSC[u4Mode].lblock.bits.SDBLK_lWIDTH     = pShadingPara->pShadingComp->SD_LWIDTH;
    LSC[u4Mode].lblock.bits.SDBLK_lHEIGHT    = pShadingPara->pShadingComp->SD_LHEIGHT;
    LSC[u4Mode].ratio.bits.RATIO00           = pShadingPara->pShadingComp->SDBLK_RATIO00;
    LSC[u4Mode].ratio.bits.RATIO01           = pShadingPara->pShadingComp->SDBLK_RATIO01;
    LSC[u4Mode].ratio.bits.RATIO10           = pShadingPara->pShadingComp->SDBLK_RATIO10;
    LSC[u4Mode].ratio.bits.RATIO11           = pShadingPara->pShadingComp->SDBLK_RATIO11;

    MY_LOG("[%s] LscMgr Config update \n", __FUNCTION__);
    LscMgr::getInstance()->ConfigUpdate();
//	    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).put(LSC[u4Mode]);
//	    ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingPara->SHADING_MODE));
    NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MTRUE);

       // log nvram data
    MY_LOG("ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA, mode = %d \n", u4Mode);
    MY_LOG("SHADING_EN:%d\n",           LSC[u4Mode].lsc_en.bits.LSC_EN      );
    MY_LOG("SHADINGBLK_XNUM:%d\n",      LSC[u4Mode].ctl2.bits.SDBLK_XNUM    );
    MY_LOG("SHADINGBLK_YNUM:%d\n",      LSC[u4Mode].ctl3.bits.SDBLK_YNUM    );
    MY_LOG("SHADINGBLK_WIDTH:%d\n",     LSC[u4Mode].ctl2.bits.SDBLK_WIDTH   );
    MY_LOG("SHADINGBLK_HEIGHT:%d\n",    LSC[u4Mode].ctl3.bits.SDBLK_HEIGHT  );
    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):0x%08x\n", LSC[u4Mode].baseaddr.bits.BASE_ADDR);
    MY_LOG("SD_LWIDTH:%d\n",            LSC[u4Mode].lblock.bits.SDBLK_lWIDTH  );
    MY_LOG("SD_LHEIGHT:%d\n",           LSC[u4Mode].lblock.bits.SDBLK_lHEIGHT );
    MY_LOG("SDBLK_RATIO00:%d\n",        LSC[u4Mode].ratio.bits.RATIO00        );
    MY_LOG("SDBLK_RATIO01:%d\n",        LSC[u4Mode].ratio.bits.RATIO01        );
    MY_LOG("SDBLK_RATIO10:%d\n",        LSC[u4Mode].ratio.bits.RATIO10        );
    MY_LOG("SDBLK_RATIO11:%d\n",        LSC[u4Mode].ratio.bits.RATIO11        );

    return  CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA )
{
    MUINT8 *pCompMode=reinterpret_cast<MUINT8*>(puParaIn);
    MUINT32 u4Mode;
    typedef ACDK_CCT_SHADING_COMP_STRUCT o_type;
    //if  (! puParaIn || sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
    if  (! puParaIn || sizeof(o_type) != u4ParaOutLen  || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);


//    LscMgr::getInstance()->setIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(*pCompMode));
//	    LscMgr::getInstance()->setCTIdx(0);
//    LscMgr::getInstance()->SetTBAToISP();
    NSIspTuning::LscMgr::LSCParameter &LSC=  LscMgr::getInstance()->getLscNvram();
    u4Mode = (MUINT32)LscMgr::getInstance()->getLscScenarioByIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(*pCompMode));
    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)u4Mode);
    //u4Mode = GetShadingMode(*pCompMode);
    //ISP_MGR_LSC_T::getInstance((ESensorDev_T)m_eSensorEnum).get(m_rISPRegs.LSC[u4Mode]);
    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA] (Shading mode pre/cap)=(%d) u4Mode %d\n", *pCompMode, u4Mode);

    pShadingPara->pShadingComp->SHADING_EN          = LSC[u4Mode].lsc_en.bits.LSC_EN;
    pShadingPara->pShadingComp->SHADINGBLK_XNUM     = LSC[u4Mode].ctl2.bits.SDBLK_XNUM+1;
    pShadingPara->pShadingComp->SHADINGBLK_YNUM     = LSC[u4Mode].ctl3.bits.SDBLK_YNUM+1;
    pShadingPara->pShadingComp->SHADINGBLK_WIDTH    = LSC[u4Mode].ctl2.bits.SDBLK_WIDTH;
    pShadingPara->pShadingComp->SHADINGBLK_HEIGHT   = LSC[u4Mode].ctl3.bits.SDBLK_HEIGHT;
    pShadingPara->pShadingComp->SHADING_RADDR       = LSC[u4Mode].baseaddr.bits.BASE_ADDR;//(UINT32)LscMgr::getInstance()->getPhyTBA();
    pShadingPara->pShadingComp->SD_LWIDTH           = LSC[u4Mode].lblock.bits.SDBLK_lWIDTH;
    pShadingPara->pShadingComp->SD_LHEIGHT          = LSC[u4Mode].lblock.bits.SDBLK_lHEIGHT;
    pShadingPara->pShadingComp->SDBLK_RATIO00       = LSC[u4Mode].ratio.bits.RATIO00;
    pShadingPara->pShadingComp->SDBLK_RATIO01       = LSC[u4Mode].ratio.bits.RATIO01;
    pShadingPara->pShadingComp->SDBLK_RATIO10       = LSC[u4Mode].ratio.bits.RATIO10;
    pShadingPara->pShadingComp->SDBLK_RATIO11       = LSC[u4Mode].ratio.bits.RATIO11;

    // log nvram data
    MY_LOG("ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA, mode = %d \n", (u4Mode));
    MY_LOG("SHADING_EN:%d\n", pShadingPara->pShadingComp->SHADING_EN );
    MY_LOG("SHADINGBLK_XNUM:%d\n", pShadingPara->pShadingComp->SHADINGBLK_XNUM);
    MY_LOG("SHADINGBLK_YNUM:%d\n", pShadingPara->pShadingComp->SHADINGBLK_YNUM);
    MY_LOG("SHADINGBLK_WIDTH:%d\n",  pShadingPara->pShadingComp->SHADINGBLK_WIDTH);
    MY_LOG("SHADINGBLK_HEIGHT:%d\n", pShadingPara->pShadingComp->SHADINGBLK_HEIGHT);
    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):0x%08x\n", pShadingPara->pShadingComp->SHADING_RADDR);
    MY_LOG("SD_LWIDTH:%d\n", pShadingPara->pShadingComp->SD_LWIDTH);
    MY_LOG("SD_LHEIGHT:%d\n", pShadingPara->pShadingComp->SD_LHEIGHT);
    MY_LOG("SDBLK_RATIO00:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO00);
    MY_LOG("SDBLK_RATIO01:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO01);
    MY_LOG("SDBLK_RATIO10:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO10);
    MY_LOG("SDBLK_RATIO11:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO11);

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX )
{
    MBOOL fgUpdateShadingNVRAMData = true;
    if  ( ! puParaIn )
        return  CCTIF_BAD_PARAM;
    MUINT8 *pShadingIndex  = reinterpret_cast<MUINT8*> (puParaIn);
    MUINT32 u4CCT = 0;

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX] "
        "(Set Shading table index to)=(%d)\n", *pShadingIndex);
    NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MFALSE);

    NSIspTuning::IspTuningMgr::getInstance().setIndex_Shading(*pShadingIndex);
    LscMgr::getInstance()->setCTIdx(*pShadingIndex);
    //LscMgr::getInstance()->SetTBAToISP();
    //LscMgr::getInstance()->enableLsc(MTRUE);
    //NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MTRUE);

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX )
{

    if  ( ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT8 *pShadingIndex = reinterpret_cast<MUINT8*>(puParaOut);
    NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MFALSE);

    NSIspTuning::IspTuningMgr::getInstance().getIndex_Shading(pShadingIndex);
    *pShadingIndex = LscMgr::getInstance()->getCTIdx();
    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX] "
                "(Get Shading table index to)=(%d)\n", *pShadingIndex);
    //NSIspTuning::IspTuningMgr::getInstance().validatePerFrame(MTRUE);

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3 )
{

    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );


    MUINT8* p_cct_input_address = (MUINT8 *)(pShadingtabledata->pBuffer);
    MUINT32 length;

    length = pShadingtabledata->Length;

    if(length==0 || length > MAX_SVD_SHADING_SIZE || p_cct_input_address == NULL)
    //if(length==0 || length > 4096 || p_cct_input_address == NULL)
    {
        MY_LOG("[Set Shading Table V3]"
            "length param is wrong or table buffer is null\n");
        return CCTIF_BAD_PARAM;
    }

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Set Shading Table V3]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)GetShadingMode(pShadingtabledata->Mode));
    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            memcpy(((MUINT8*)&m_rBuf_SD.Shading.PreviewSVDTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset,
                    p_cct_input_address, pShadingtabledata->Length);
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            memcpy(((MUINT8*)&m_rBuf_SD.Shading.CaptureSVDTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset,
                    p_cct_input_address, pShadingtabledata->Length);
            break;
        default:
            MY_LOG("[Set Shading Table V3]"
                "Camera mode not support shading table\n");
            return CCTIF_BAD_PARAM;
            break;
    }


    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3 )
{

    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );

    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
    NvramDrvMgr::getInstance().init((ESensorDev_T)m_eSensorEnum);
    NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);
   
    m_rBuf_SD = *pNvram_Shading;
    MUINT8* p_cct_output_address = (MUINT8 *)(pShadingtabledata->pBuffer);

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Get Shading Table V3]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)GetShadingMode(pShadingtabledata->Mode));
    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
        memcpy(p_cct_output_address,
                &(m_rBuf_SD.Shading.PreviewSVDTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset,
                pShadingtabledata->Length);
            break;
        case CAMERA_TUNING_CAPTURE_SET:
        memcpy(p_cct_output_address,
                &(m_rBuf_SD.Shading.CaptureSVDTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset,
                pShadingtabledata->Length);
            break;
        default:
            MY_LOG("[Get Shading Table V3]"
                "Camera mode not support shading table\n");
            NvramDrvMgr::getInstance().uninit();
            return CCTIF_BAD_PARAM;
    }

    NvramDrvMgr::getInstance().uninit();
    return  CCTIF_NO_ERROR;

}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF )
{

    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );


    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
    NvramDrvMgr::getInstance().init((ESensorDev_T)m_eSensorEnum);
    NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);

    m_rBuf_SD = *pNvram_Shading;

    MUINT32* ref_addr = NULL;
    MUINT8* p_cct_input_address = (MUINT8 *)(pShadingtabledata->pBuffer);
    MUINT32 length;

    length = pShadingtabledata->Length;

    if(p_cct_input_address == NULL)
    {
        MY_LOG("[Set Shading Table Poly Coef]"
            "length param is wrong or table buffer is null\n");
        return CCTIF_BAD_PARAM;
    }

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Set Shading Table Poly Coef]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    //LscMgr::getInstance()->setIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode));
    //LscMgr::getInstance()->setCTIdx(pShadingtabledata->ColorTemp);
    MUINT32 *virAddr =
        (MUINT32*)LscMgr::getInstance()->getVirTBA((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode),
        pShadingtabledata->ColorTemp);
    MUINT32 *phyAddr =
        (MUINT32*)LscMgr::getInstance()->getPhyTBA((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode),
        pShadingtabledata->ColorTemp);
    MY_LOG("[%s] Mode %d, Copy leng:%d, offset:%d to NVRAM buffer to virAddr 0x%08x, phy 0x%08x\n", __FUNCTION__,
            pShadingtabledata->Mode,
            length,
            pShadingtabledata->Offset,
            virAddr,
            phyAddr);

    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)GetShadingMode(pShadingtabledata->Mode));

    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            MY_LOG("[%s] CAMERA_TUNING_PREVIEW_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.PreviewFrmTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            MY_LOG("[%s] CAMERA_TUNING_CAPTURE_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.CaptureTilTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;
            break;
        case CAMERA_TUNING_VIDEO_SET:
            MY_LOG("[%s] CAMERA_TUNING_VIDEO_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.VideoFrmTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;
            break;
        default:
            MY_LOG("[Set Shading Table Poly Coef]"
                "Camera mode not support shading table\n");
            NvramDrvMgr::getInstance().uninit();
            return CCTIF_BAD_PARAM;
    }
    // write to NVRAM
    memcpy(ref_addr,
            p_cct_input_address,
            pShadingtabledata->Length*sizeof(MUINT32));
    // write to LSC mgr physical buffer
    memcpy((MVOID*)(virAddr+pShadingtabledata->Offset),
            p_cct_input_address,
            pShadingtabledata->Length*sizeof(MUINT32));

//    {
//        MY_LOG("[VirAddr PhyAddr Len] 0x%08x, 0x%08x 0x%08x\n", virAddr, phyAddr, length);
//        int i = 0;
//        for (i = 0; i < length; i+=4) {
//            MY_LOG("0x%08x, 0x%08x, 0x%08x, 0x%08x || 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
//                    *(virAddr+pShadingtabledata->Offset+i+0),
//                    *(virAddr+pShadingtabledata->Offset+i+1),
//                    *(virAddr+pShadingtabledata->Offset+i+2),
//                    *(virAddr+pShadingtabledata->Offset+i+3),
//                    *(ref_addr+i+0),
//                    *(ref_addr+i+1),
//                    *(ref_addr+i+2),
//                    *(ref_addr+i+3));
//        }
//    }

    NvramDrvMgr::getInstance().uninit();
    return  CCTIF_NO_ERROR;
}

#include <nvram_drv_mgr.h>
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF )
{

    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );

    NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
    NvramDrvMgr::getInstance().init((ESensorDev_T)m_eSensorEnum);
    NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);

    m_rBuf_SD = *pNvram_Shading;

    MUINT32 *ref_addr = NULL;
    //LscMgr::getInstance()->setIspProfile((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode));
    //LscMgr::getInstance()->setCTIdx(pShadingtabledata->ColorTemp);
    MUINT32 *virAddr =
        (MUINT32 *)LscMgr::getInstance()->getVirTBA((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode),
        pShadingtabledata->ColorTemp);
    MUINT32 *phyAddr =
        (MUINT32 *)LscMgr::getInstance()->getPhyTBA((NSIspTuning::EIspProfile_T)GetISPProfile(pShadingtabledata->Mode),
        pShadingtabledata->ColorTemp);
    MUINT32 length = pShadingtabledata->Length;
    MUINT8* p_cct_output_address = (MUINT8 *)(pShadingtabledata->pBuffer);

    MY_LOG("[%s] Mode %d, Get leng:%d, offset:%d from NVRAM buffer virAddr 0x%08x, phy 0x%08x\n", __FUNCTION__,
                pShadingtabledata->Mode,
                length,
                pShadingtabledata->Offset,
                virAddr,
                phyAddr);

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Get Shading Table Poly Coef]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }


    LscMgr::getInstance()->setMetaLscScenario((NSIspTuning::LscMgr::ELscScenario_T)GetShadingMode(pShadingtabledata->Mode));
    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            MY_LOG("[%s] CAMERA_TUNING_PREVIEW_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.PreviewFrmTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;

            break;
        case CAMERA_TUNING_CAPTURE_SET:
            MY_LOG("[%s] CAMERA_TUNING_CAPTURE_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.CaptureTilTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;

            break;
        case CAMERA_TUNING_VIDEO_SET:
            MY_LOG("[%s] CAMERA_TUNING_VIDEO_SET", __FUNCTION__);
            ref_addr = ((MUINT32*)&m_rBuf_SD.Shading.VideoFrmTable[pShadingtabledata->ColorTemp][0])+pShadingtabledata->Offset;

            break;
        default:
            MY_LOG("[Get Shading Table Poly Coef]"
                    "Camera mode not support shading table\n");

            NvramDrvMgr::getInstance().uninit();
            return CCTIF_BAD_PARAM;
    }

    memcpy(p_cct_output_address,
            ref_addr,
            pShadingtabledata->Length*sizeof(MUINT32));

//    {
//        MY_LOG("[VirAddr Phy Content] 0x%08x, 0x%08x 0x%08x\n", virAddr, phyAddr, length);
//        int i = 0;
//        for (i = 0; i < length; i+=4) {
//            MY_LOG("0x%08x, 0x%08x, 0x%08x, 0x%08x || 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
//                    *(virAddr+pShadingtabledata->Offset+i+0),
//                    *(virAddr+pShadingtabledata->Offset+i+1),
//                    *(virAddr+pShadingtabledata->Offset+i+2),
//                    *(virAddr+pShadingtabledata->Offset+i+3),
//                    *(ref_addr+i+0),
//                    *(ref_addr+i+1),
//                    *(ref_addr+i+2),
//                    *(ref_addr+i+3));
//        }
//    }

    NvramDrvMgr::getInstance().uninit();
    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA )
{
#if 1
    typedef ACDK_CCT_NVRAM_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pCAMERA_NVRAM_DATA  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA]"
        "Mode is %d"
        , pCAMERA_NVRAM_DATA->Mode
        );


    MUINT8* p_cct_output_address = (MUINT8 *)(pCAMERA_NVRAM_DATA->pBuffer);

    switch (pCAMERA_NVRAM_DATA->Mode)
    {
        case CAMERA_NVRAM_DEFECT_STRUCT:
            break;
        case CAMERA_NVRAM_SHADING_STRUCT:
        {
            NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
            NvramDrvMgr::getInstance().init((ESensorDev_T)m_eSensorEnum);
            NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);

            m_rBuf_SD = *pNvram_Shading;
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading
                    , sizeof(ISP_SHADING_STRUCT));
            *pu4RealParaOutLen = sizeof(ISP_SHADING_STRUCT);
            MY_LOG("PreviewSize :%d\n",  m_rBuf_SD.Shading.LSCSize[0]);
            MY_LOG("CaptureSize :%d\n", m_rBuf_SD.Shading.LSCSize[3]);
            MY_LOG("Pre SVD Size :%d\n", m_rBuf_SD.Shading.PreviewSVDSize);
            MY_LOG("Cap SVD Size :%d\n", m_rBuf_SD.Shading.CaptureSVDSize);
            MY_LOG("NVRAM Data :%d\n", m_rBuf_SD.Shading.PreviewFrmTable[0][0]);

            NvramDrvMgr::getInstance().uninit();
        }
            break;
        case CAMERA_NVRAM_3A_STRUCT:
            break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT: {
            ::memcpy(p_cct_output_address, &m_rISPRegs, sizeof(ISP_NVRAM_REGISTER_STRUCT));
            *pu4RealParaOutLen = sizeof(ISP_NVRAM_REGISTER_STRUCT);
            }
            break;
        default:
            MY_LOG("[Get Camera NVRAM data]"
                "Not support NVRAM structure\n");
            return CCTIF_BAD_PARAM;
            break;
    }


    return  CCTIF_NO_ERROR;
#else
    return  CCTIF_NO_ERROR;
#endif

}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )
{
#if 1
    MY_LOG("[%s] ", __FUNCTION__);
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )");


    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const pBufIF = m_pNvramDrv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();

    err = pBufIF->refresh(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM] m_rBufIf_SD.refresh() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
#else
    return  CCTIF_NO_ERROR;
#endif
}


IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )
{
#if 1
    MY_LOG("[%s] ", __FUNCTION__);
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )");

    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const pBufIF = m_pNvramDrv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();

    err = pBufIF->flush(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM] m_rBufIf_SD.flush() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
#else
    return  CCTIF_NO_ERROR;
#endif
}

