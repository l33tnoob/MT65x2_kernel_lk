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
#define LOG_TAG "paramctrl_exif"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <camera_custom_nvram.h>
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
#include <dbg_isp_param.h>
#include <pca_mgr.h>
#include <lsc_mgr.h>
#include "paramctrl_if.h"
#include "paramctrl.h"

using namespace android;
using namespace NSIspTuning;
using namespace NSIspExifDebug;


namespace
{


//  Tag <- Isp Index.
inline
MVOID
setIspIdx(
    IspDebugTag (&rTags)[TagID_Total_Num],
    IspDebugTagID const eTagID,
    MUINT32 const u4Idx
)
{
    rTags[eTagID].u4ID  = getIspTag(eTagID);
    rTags[eTagID].u4Val = u4Idx;
}

//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    ESensorDev_T eSensorDev,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;
    getIspReg(eSensorDev, param);
    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //MY_LOG("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}

};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
saveDebugInfo()
{
#if 0
    IspDebugTag (&rTags)[TagID_Total_Num] = m_rIspExifDebugInfo.debugInfo.tags;

    switch  ( m_IspCamInfo.eIspProfile )
    {
    case EIspProfile_MFCapPass1: //  MFB pass 1
    case ECamMode_HDR_Cap_Pass1_SF:     //  HDR Pass1: Single Frame
    case ECamMode_HDR_Cap_Pass1_MF1:    //  HDR Pass1: Multi Frame Stage1
    case ECamMode_Online_Capture_ZSD:   //  ZSD Pass1
        //  Preprocessing Capture.
        break;
    default:
        goto lbExit;
    }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  Here, we are in preprocessing capture.
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //
    ////////////////////////////////////////////////////////////////////////////
    //  ISPRegs
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Preprocess Gain Control
    MY_LOG("Preprocess gain control:");
    setIspTags<ISP_NVRAM_GAIN_CTRL_T, Preproc__GainCtrl_Begin>(rTags);
    //  Preprocess Shading
    MY_LOG("Preprocess shading:");
    setIspIdx(rTags, Preproc__IDX_SHADING, m_IspNvramMgr.getIdx_Shading());
    setIspTags<ISP_NVRAM_SHADING_T, Preproc__SHADING_Begin>(rTags);
    //
    //  Preprocess OB
    MY_LOG("Preprocess ob:");
    setIspIdx(rTags, Preproc__IDX_OB, m_IspNvramMgr.getIdx_OB());
    setIspTags<ISP_NVRAM_OB_T, Preproc__OB_Begin>(rTags);
    //
    //  Preprocess DP
    MY_LOG("Preprocess dp:");
    setIspIdx(rTags, Preproc__IDX_DP, m_IspNvramMgr.getIdx_DP());
    setIspTags<ISP_NVRAM_DP_T, Preproc__DP_Begin>(rTags);
    //
    //  Preprocess NR1
    MY_LOG("Preprocess nr1:");
    setIspIdx(rTags, Preproc__IDX_NR1, m_IspNvramMgr.getIdx_NR1());
    setIspTags<ISP_NVRAM_NR1_T, Preproc__NR1_Begin>(rTags);
    //
    //  Preprocess NR2
    MY_LOG("Preprocess nr2:");
    setIspIdx(rTags, Preproc__IDX_NR2, m_IspNvramMgr.getIdx_NR2());
    setIspTags<ISP_NVRAM_NR2_T, Preproc__NR2_Begin>(rTags);

    if (m_IspCamInfo.eCamMode == ECamMode_Online_Capture_ZSD)
    {
        //
        //  Preprocess EE
        MY_LOG("Preprocess ee:");
        setIspIdx(rTags, Preproc__IDX_EE, m_IspNvramMgr.getIdx_EE());
        setIspTags<ISP_NVRAM_EE_T, Preproc__EE_Begin>(rTags);
        //
        //  saturation
        MY_LOG("Preprocess saturation:");
        setIspIdx(rTags, Preproc__IDX_SATURATION, m_IspNvramMgr.getIdx_Saturation());
        setIspTags<ISP_NVRAM_SATURATION_T, Preproc__SAT_Begin>(rTags);
        //
        //  contrast
        MY_LOG("Preprocess contrast:");
        setIspIdx(rTags, Preproc__IDX_CONTRAST, m_IspNvramMgr.getIdx_Contrast());
        setIspTags<ISP_NVRAM_CONTRAST_T, Preproc__CONTRAST_Begin>(rTags);
        //
        //  hue
        MY_LOG("Preprocess hue:");
        setIspIdx(rTags, Preproc__IDX_HUE, m_IspNvramMgr.getIdx_Hue());
        setIspTags<ISP_NVRAM_HUE_T, Preproc__HUE_Begin>(rTags);
        //
        //  PCA
        MY_LOG("Preprocess pca:");
        setIspIdx(rTags, Preproc__IDX_PCA, m_pIPcaMgr->getIdx());
        setIspTags<ISP_NVRAM_PCA_T, Preproc__PCA_Begin>(rTags);
    }
#endif
lbExit:
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo) const
{
    Mutex::Autolock lock(m_Lock);

    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rDebugInfo.hdr  = g_rIspExifDebugInfoHdr;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = m_rIspExifDebugInfo.debugInfo.tags;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);


    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // CAM_CTL_EN
    MY_LOG_IF(m_bDebugEnable,"CAM_CTL_EN:");
    setIspTags<ISP_NVRAM_CTL_EN_T, CAM_CTL_EN_Begin>(getSensorDev(), rTags);

    // OBC
    MY_LOG_IF(m_bDebugEnable,"OBC:");
    setIspIdx(rTags, IDX_OBC, m_IspNvramMgr.getIdx_OBC());
    setIspTags<ISP_NVRAM_OBC_T, CAM_OBC_Begin>(getSensorDev(), rTags);

    // BPC
    MY_LOG_IF(m_bDebugEnable,"BPC:");
    setIspIdx(rTags, IDX_BPC, m_IspNvramMgr.getIdx_BPC());
    setIspTags<ISP_NVRAM_BPC_T, CAM_BPC_Begin>(getSensorDev(), rTags);

    //  NR1
    MY_LOG_IF(m_bDebugEnable,"NR1:");
    setIspIdx(rTags, IDX_NR1, m_IspNvramMgr.getIdx_NR1());
    setIspTags<ISP_NVRAM_NR1_T, CAM_NR1_Begin>(getSensorDev(), rTags);

    // LSC
    MY_LOG_IF(m_bDebugEnable,"LSC:");
    setIspIdx(rTags, IDX_LSC, m_IspNvramMgr.getIdx_LSC());
    setIspTags<ISP_NVRAM_LSC_CFG_T, CAM_LSC_Begin>(getSensorDev(), rTags);

    // SL2
    MY_LOG_IF(m_bDebugEnable,"SL2:");
    setIspIdx(rTags, IDX_SL2, m_IspNvramMgr.getIdx_SL2());
    setIspTags<ISP_NVRAM_SL2_T, CAM_SL2_Begin>(getSensorDev(), rTags);

    // PGN
    MY_LOG_IF(m_bDebugEnable,"PGN:");
    setIspTags<ISP_NVRAM_PGN_T, CAM_PGN_Begin>(getSensorDev(), rTags);

    // CFA
    MY_LOG_IF(m_bDebugEnable,"CFA:");
    setIspIdx(rTags, IDX_CFA, m_IspNvramMgr.getIdx_CFA());
    setIspTags<ISP_NVRAM_CFA_T, CAM_CFA_Begin>(getSensorDev(), rTags);

    // G2G
    MY_LOG_IF(m_bDebugEnable,"G2G:");
    setIspTags<ISP_NVRAM_CCM_T, CAM_G2G_Begin>(getSensorDev(), rTags);

    // GGM
    setIspIdx(rTags, IDX_GGM, m_IspNvramMgr.getIdx_GGM());

    ISP_NVRAM_GGM_T& rGGM = *(reinterpret_cast<ISP_NVRAM_GGM_T*>(m_rIspExifDebugInfo.tableInfo.GGM));
    ISP_MGR_GGM_T::getInstance(getSensorDev()).get(rGGM);

    // G2C
    MY_LOG_IF(m_bDebugEnable,"G2C:");
    setIspTags<ISP_NVRAM_G2C_T, CAM_G2C_Begin>(getSensorDev(), rTags);

    // ANR
    MY_LOG_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, m_IspNvramMgr.getIdx_ANR());
    setIspTags<ISP_NVRAM_ANR_T, CAM_ANR_Begin>(getSensorDev(), rTags);

    // CCR
    MY_LOG_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, m_IspNvramMgr.getIdx_CCR());
    setIspTags<ISP_NVRAM_CCR_T, CAM_CCR_Begin>(getSensorDev(), rTags);

    // PCA
    MY_LOG_IF(m_bDebugEnable,"PCA:");
    setIspIdx(rTags, IDX_PCA, m_pPcaMgr->getIdx());
    setIspIdx(rTags, PCA_SLIDER, m_pPcaMgr->getSliderValue());
    setIspTags<ISP_NVRAM_PCA_T, CAM_PCA_Begin>(getSensorDev(), rTags);

    ISP_MGR_PCA_T::getInstance(getSensorDev()).getLut(m_rIspExifDebugInfo.tableInfo.PCA);

    //  EE
    MY_LOG_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, m_IspNvramMgr.getIdx_EE());
    setIspTags<ISP_NVRAM_EE_T, CAM_EE_Begin>(getSensorDev(), rTags);

    // SE
    MY_LOG_IF(m_bDebugEnable,"SE:");
    setIspTags<ISP_NVRAM_SE_T, CAM_SE_Begin>(getSensorDev(), rTags);

    ////////////////////////////////////////////////////////////////////////////
    //  (2.2) ISPComm
    ////////////////////////////////////////////////////////////////////////////
    for (MUINT32 i = 0; i < sizeof(ISP_NVRAM_COMMON_STRUCT)/sizeof(MUINT32); i++)
    {
        MUINT32 const u4TagID = COMM_Begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = m_rIspComm.CommReg[i];
    }

    //  (2.3) RAWIspCamInfo
    MY_LOG_IF(m_bDebugEnable,"RAWIspCamInfo:");
    setIspIdx(rTags, IspProfile, m_rIspCamInfo.eIspProfile);
    setIspIdx(rTags, SceneIdx, m_rIspCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, m_rIspCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, m_rIspCamInfo.eIdx_ISO);
    setIspIdx(rTags, ShadingIdx, m_rIspCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, LightValue_x10, m_rIspCamInfo.i4LightValue_x10);
    //
    //  (2.4) EffectMode
    MY_LOG_IF(m_bDebugEnable,"EffectMode:");
    setIspIdx(rTags, EffectMode, getEffect());
    //
    //  (2.5) UserSelectLevel
    MY_LOG_IF(m_bDebugEnable,"UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, getIspUsrSelectLevel().eIdx_Edge);
    setIspIdx(rTags, HueIdx, getIspUsrSelectLevel().eIdx_Hue);
    setIspIdx(rTags, SatIdx, getIspUsrSelectLevel().eIdx_Sat);
    setIspIdx(rTags, BrightIdx, getIspUsrSelectLevel().eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, getIspUsrSelectLevel().eIdx_Contrast);
    //
    //  (2.6) Output
    rDebugInfo.debugInfo = m_rIspExifDebugInfo.debugInfo;
    rDebugInfo.tableInfo = m_rIspExifDebugInfo.tableInfo;

    //  (3) Reset to the default.
    ::memset(&m_rIspExifDebugInfo, 0, sizeof(m_rIspExifDebugInfo));

    return  MERR_OK;
}

