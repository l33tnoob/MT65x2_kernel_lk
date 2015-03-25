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
#define LOG_TAG "paramctrl_per_frame"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <isp_tuning.h>
#include <camera_feature.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <dynamic_ccm.h>
#include <ccm_mgr.h>
#include <lsc_mgr.h>
#include <dbg_isp_param.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include "CameraProfile.h"

using namespace android;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_All()
{
    if(isDynamicBypass() == MTRUE) {
        return ISP_MGR_OBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
               &&  ISP_MGR_PGN_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile);
    }
    else {

    return  MTRUE
        &&  ISP_MGR_OBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_BNR_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LSC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PGN_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_PCA_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_NR3D_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_MFB_T::getInstance(getSensorDev()).apply(m_rIspCamInfo.eIspProfile)
        ;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_All()
{
    MBOOL fgRet = MTRUE;

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_BNR_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_CFA_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_CCM_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_GGM_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_G2C_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_NBC_T::getInstance(getSensorDev()).reset()
        //&&  ISP_MGR_PCA_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_SEEE_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_NR3D_T::getInstance(getSensorDev()).reset()
        &&  ISP_MGR_MFB_T::getInstance(getSensorDev()).reset()
            ;

    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (2) default
    prepareHw_PerFrame_Default();

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_PGN()
        &&  prepareHw_PerFrame_CFA()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_GGM()
        &&  prepareHw_PerFrame_ANR()
        &&  prepareHw_PerFrame_CCR()
        &&  prepareHw_PerFrame_PCA()
        &&  prepareHw_PerFrame_EE()
        &&  prepareHw_PerFrame_NR3D()
        &&  prepareHw_PerFrame_MFB()
        &&  prepareHw_PerFrame_EFFECT()
            ;

    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
prepareHw_PerFrame_Default()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC()
{
    // Get default NVRAM parameter
    ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), obc );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC()
{
    // Get default NVRAM parameter
    ISP_NVRAM_BPC_T bpc = m_IspNvramMgr.getBPC();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_BPC(m_rIspCamInfo, m_IspNvramMgr, bpc);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), bpc );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1()
{
    // Get default NVRAM parameter
    ISP_NVRAM_NR1_T nr1 = m_IspNvramMgr.getNR1();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR1(m_rIspCamInfo, m_IspNvramMgr, nr1);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), nr1 );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC()
{
    MY_LOG_IF(m_bDebugEnable,"%s", __FUNCTION__);

    //////////////////////////////////////
    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;

    // Check to see if it is needed to load LUT.
    switch  (getOperMode())
    {
    case EOperMode_Normal:
    case EOperMode_PureRaw:
        MY_LOG("%s m_pLscMgr EOperMode_Normal", __FUNCTION__);
        //  (1) Check to see whether PCA is enabled?
        if  (! m_pLscMgr->isEnable())
        {
            MY_LOG("%s m_pLscMgr disable", __FUNCTION__);
            return  MTRUE;
        }

        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {
            // Dynamic Tuning: Enable
            new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        }

        m_pLscMgr->setCTIdx(new_cct_idx);
        m_pLscMgr->SetTBAToISP();
        m_pLscMgr->enableLsc(MTRUE);
        break;
    case EOperMode_EM:
    case EOperMode_Meta:
        MY_LOG("%s EOperMode_Meta", __FUNCTION__);
        break;
    default:
        break;
    }

    // debug message
    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    m_IspNvramMgr.setIdx_LSC(m_pLscMgr->getRegIdx());
    //////////////////////////////////////
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN()
{
    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    getIspHWBuf(getSensorDev(), pgn );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_PGN(m_rIspCamInfo, m_IspNvramMgr, pgn);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), pgn );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CFA()
{
    // Get default NVRAM parameter
    ISP_NVRAM_CFA_T cfa = m_IspNvramMgr.getCFA();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CFA(m_rIspCamInfo, m_IspNvramMgr, cfa);
    }

    if ((getOperMode() == EOperMode_Meta) && (!ISP_MGR_CFA_T::getInstance(getSensorDev()).isEnable())) // CCT usage: fix CFA index
    {
        cfa = m_IspNvramMgr.getCFA(NVRAM_CFA_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), cfa );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM()
{
    // Get default NVRAM parameter
    ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ccm );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM()
{
    // Get default NVRAM parameter
    ISP_NVRAM_GGM_T ggm = m_IspNvramMgr.getGGM();

    // Invoke callback for customers to modify.
    if ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_GGM(m_rIspCamInfo,  m_IspNvramMgr, ggm);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ggm );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR()
{
    // Get default NVRAM parameter
    ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_ANR(m_rIspCamInfo, m_IspNvramMgr, anr);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), anr );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR()
{
    // Get default NVRAM parameter
    ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCR(m_rIspCamInfo, m_IspNvramMgr, ccr);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ccr );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA()
{
    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    // (2) Invoke callback for customers to modify.
    if  (isDynamicTuning())
    {   // Dynamic Tuning: Enable
        m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));
    }

    // Check to see if it is needed to load LUT.
    switch  (getOperMode())
    {
    case EOperMode_Normal:
    case EOperMode_PureRaw:
        fgIsToLoadLut = m_pPcaMgr->isChanged();   // Load if changed.
        break;
    default:
        fgIsToLoadLut = MTRUE;                  // Force to load.
        break;
    }

    if (fgIsToLoadLut) {
        m_pPcaMgr->loadLut();
        m_pPcaMgr->loadConfig();
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE()
{
    // Get default NVRAM parameter
    ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_EE(m_rIspCamInfo, m_IspNvramMgr, ee);

        if (m_IspUsrSelectLevel.eIdx_Edge != ISP_EDGE_MIDDLE)
        {
            // User setting
            m_pIspTuningCustom->userSetting_EE(m_IspUsrSelectLevel.eIdx_Edge, ee);

        }
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), ee);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT()
{
    ISP_NVRAM_G2C_T g2c;
    ISP_NVRAM_SE_T se;

    // Get ISP HW buffer
    getIspHWBuf(getSensorDev(), g2c);
    getIspHWBuf(getSensorDev(), se);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_eIdx_Effect, m_IspUsrSelectLevel, g2c, se);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), g2c);
    putIspHWBuf(getSensorDev(), se);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D()
{
    if ((m_rIspCamInfo.eIspProfile == EIspProfile_NormalPreview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_ZsdPreview_CC)  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_ZsdPreview_NCC) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VideoPreview))
    {
        // Get default NVRAM parameter
        ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR3D(m_rIspCamInfo, m_IspNvramMgr, nr3d);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(getSensorDev(), nr3d );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB()
{
    if (m_rIspCamInfo.eIspProfile != EIspProfile_MFCapPass2)
    {
        ISP_MGR_MFB::getInstance(getSensorDev()).setEnable(MFALSE);
        return MTRUE;
    }

    ISP_MGR_MFB::getInstance(getSensorDev()).setEnable(MTRUE);

    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MFB(m_rIspCamInfo, m_IspNvramMgr, mfb);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(getSensorDev(), mfb );

    return  MTRUE;
}


