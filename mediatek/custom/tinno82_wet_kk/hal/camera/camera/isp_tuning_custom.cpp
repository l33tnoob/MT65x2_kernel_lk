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

#define LOG_TAG "isp_tuning_custom"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
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
#include <isp_tuning_custom_instance.h>

#include "kd_imgsensor.h"//LINE <> <DATE20130917> <OV5648 tuning> wupingzhou

using namespace NSIspTuning;


/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eCamMode:
*               ECamMode_Video          = 0,
*               ECamMode_Online_Preview,
*               ECamMode_Online_Capture,
*               ECamMode_Online_Capture_ZSD,
*               ECamMode_Offline_Capture_Pass1,
*               ECamMode_Offline_Capture_Pass2,
*               ECamMode_HDR_Cap_Pass1_SF,  //  Pass1: Single Frame
*               ECamMode_HDR_Cap_Pass1_MF1, //  Pass1: Multi Frame Stage1
*               ECamMode_HDR_Cap_Pass1_MF2, //  Pass1: Multi Frame Stage2
*               ECamMode_HDR_Cap_Pass2,     //  Pass2
*
*           eIdx_Scene:
*               SCENE_MODE_OFF,             // Disable scene mode equal Auto mode
*               SCENE_MODE_NORMAL,          // Normal mode
*               SCENE_MODE_ACTION,          // Action mode
*               SCENE_MODE_PORTRAIT,        // Portrait mode
*               SCENE_MODE_LANDSCAPE,       // Landscape
*               SCENE_MODE_NIGHTSCENE,      // Night Scene
*               SCENE_MODE_NIGHTPORTRAIT,   // Night Portrait
*               SCENE_MODE_THEATRE,         // Theatre mode
*               SCENE_MODE_BEACH,           // Beach mode
*               SCENE_MODE_SNOW,            // Snow mode
*               SCENE_MODE_SUNSET,          // Sunset mode
*               SCENE_MODE_STEADYPHOTO,     // Steady photo mode
*               SCENE_MODE_FIREWORKS,       // Fireworks mode
*               SCENE_MODE_SPORTS,          // Sports mode
*               SCENE_MODE_PARTY,           // Party mode
*               SCENE_MODE_CANDLELIGHT,     // Candle light mode
*               SCENE_MODE_HDR,             // HDR mode
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1600
*
*           i4CCT:
*               Correlated color temperature
*
*           eCCTIndex_CCM:
*               Correlated color temperature index for CCM
*                   eIDX_CCM_TL84
*                   eIDX_CCM_CWF
*                   eIDX_CCM_D65
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*   rIdxMgr:
*       [in]    The default ISP tuning index manager.
*       [out]   The ISP tuning index manager after customizing.
*
*
*******************************************************************************/
MVOID
IspTuningCustom::
evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr)
{
//..............................................................................
    //  (1) Dump info. before customizing.
#if ENABLE_MY_LOG
    rCamInfo.dump();
#endif

#if 0
    LOGD("[+evaluate_nvram_index][before customizing]");
    rIdxMgr.dump();
#endif
//..............................................................................
    //  (2) Modify each index based on conditions.
    //
    //  setIdx_XXX() returns:
    //      MTURE: if successful
    //      MFALSE: if the input index is out of range.
    //
#if 0
    fgRet = rIdxMgr.setIdx_OBC(XXX);
    fgRet = rIdxMgr.setIdx_BPC(XXX);
    fgRet = rIdxMgr.setIdx_NR1(XXX);
    fgRet = rIdxMgr.setIdx_CFA(XXX);
    fgRet = rIdxMgr.setIdx_GGM(XXX);
    fgRet = rIdxMgr.setIdx_ANR(XXX);
    fgRet = rIdxMgr.setIdx_CCR(XXX);
    fgRet = rIdxMgr.setIdx_EE(XXX);
#endif
//..............................................................................
    //  (3) Finally, dump info. after modifying.
#if 0
    LOGD("[-evaluate_nvram_index][after customizing]");
    rIdxMgr.dump();
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_OBC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_OBC_T& rOBC)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...
    
    MY_LOG("rOBC.offst0 = 0x%8x", rOBC.offst0);
    MY_LOG("rOBC.offst1 = 0x%8x", rOBC.offst1);
    MY_LOG("rOBC.offst2 = 0x%8x", rOBC.offst2);
    MY_LOG("rOBC.offst3 = 0x%8x", rOBC.offst3);
    MY_LOG("rOBC.gain0 = 0x%8x", rOBC.gain0);
    MY_LOG("rOBC.gain1 = 0x%8x", rOBC.gain1);
    MY_LOG("rOBC.gain2 = 0x%8x", rOBC.gain2);
    MY_LOG("rOBC.gain3 = 0x%8x", rOBC.gain3);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_BPC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_BPC_T& rBPC)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rBPC.con = 0x%8x", rBPC.con);
    MY_LOG("rBPC.cd1_1 = 0x%8x", rBPC.cd1_1);
    MY_LOG("rBPC.cd1_2 = 0x%8x", rBPC.cd1_2);
    MY_LOG("rBPC.cd1_3 = 0x%8x", rBPC.cd1_3);
    MY_LOG("rBPC.cd1_4 = 0x%8x", rBPC.cd1_4);
    MY_LOG("rBPC.cd1_5 = 0x%8x", rBPC.cd1_5);
    MY_LOG("rBPC.cd1_6 = 0x%8x", rBPC.cd1_6);
    MY_LOG("rBPC.cd2_1 = 0x%8x", rBPC.cd2_1);
    MY_LOG("rBPC.cd2_2 = 0x%8x", rBPC.cd2_2);
    MY_LOG("rBPC.cd2_3 = 0x%8x", rBPC.cd2_3);
    MY_LOG("rBPC.cd0 = 0x%8x", rBPC.cd0);
    MY_LOG("rBPC.cor = 0x%8x", rBPC.cor);
    #endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_NR1(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_NR1_T& rNR1)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_SL2(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_SL2_T& rSL2)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rSL2.cen = 0x%8x", rSL2.cen);
    MY_LOG("rSL2.max0_rr = 0x%8x", rSL2.max0_rr);
    MY_LOG("rSL2.max1_rr = 0x%8x", rSL2.max1_rr);
    MY_LOG("rSL2.max2_rr = 0x%8x", rSL2.max2_rr);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_PGN(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_PGN_T& rPGN)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rPGN.satu01 = 0x%8x", rPGN.satu01);
    MY_LOG("rPGN.satu23 = 0x%8x", rPGN.satu23);
    MY_LOG("rPGN.gain01 = 0x%8x", rPGN.gain01);
    MY_LOG("rPGN.gain23 = 0x%8x", rPGN.gain23);
    MY_LOG("rPGN.offs01 = 0x%8x", rPGN.offs01);
    MY_LOG("rPGN.offs23 = 0x%8x", rPGN.offs23);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CFA(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CFA_T& rCFA)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCFA.bypass = 0x%8x", rCFA.bypass);
    MY_LOG("rCFA.ed_f = 0x%8x", rCFA.ed_f);
    MY_LOG("rCFA.ed_nyq = 0x%8x", rCFA.ed_nyq);
    MY_LOG("rCFA.ed_step = 0x%8x", rCFA.ed_step);
    MY_LOG("rCFA.rgb_hf = 0x%8x", rCFA.rgb_hf);
    MY_LOG("rCFA.bw = 0x%8x", rCFA.bw);
    MY_LOG("rCFA.f1_act = 0x%8x", rCFA.f1_act);
    MY_LOG("rCFA.f2_act = 0x%8x", rCFA.f2_act);
    MY_LOG("rCFA.f3_act = 0x%8x", rCFA.f3_act);
    MY_LOG("rCFA.f4_act = 0x%8x", rCFA.f4_act);
    MY_LOG("rCFA.f1_l = 0x%8x", rCFA.f1_l);
    MY_LOG("rCFA.f2_l = 0x%8x", rCFA.f2_l);
    MY_LOG("rCFA.f3_l = 0x%8x", rCFA.f3_l);
    MY_LOG("rCFA.f4_l = 0x%8x", rCFA.f4_l);
    MY_LOG("rCFA.hf_rb = 0x%8x", rCFA.hf_rb);
    MY_LOG("rCFA.hf_gain = 0x%8x", rCFA.hf_gain);
    MY_LOG("rCFA.hf_comp = 0x%8x", rCFA.hf_comp);
    MY_LOG("rCFA.hf_coring_th = 0x%8x", rCFA.hf_coring_th);
    MY_LOG("rCFA.act_lut = 0x%8x", rCFA.act_lut);
    MY_LOG("rCFA.spare = 0x%8x", rCFA.spare);
    MY_LOG("rCFA.bb = 0x%8x", rCFA.bb);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CCM_T& rCCM)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCM.conv0a = 0x%8x", rCCM.conv0a);
    MY_LOG("rCCM.conv0b = 0x%8x", rCCM.conv0b);
    MY_LOG("rCCM.conv1a = 0x%8x", rCCM.conv1a);
    MY_LOG("rCCM.conv1b = 0x%8x", rCCM.conv1b);
    MY_LOG("rCCM.conv2a = 0x%8x", rCCM.conv2a);
    MY_LOG("rCCM.conv2b = 0x%8x", rCCM.conv2b);
#endif    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_GGM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_GGM_T& rGGM)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rGGM.rb_gmt.lut[0] = 0x%8x", rGGM.rb_gmt.lut[0]);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_ANR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_ANR_T& rANR)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rANR.con1 = 0x%8x", rANR.con1);
    MY_LOG("rANR.con2 = 0x%8x", rANR.con2);
    MY_LOG("rANR.con3 = 0x%8x", rANR.con3);
    MY_LOG("rANR.yad1 = 0x%8x", rANR.yad1);
    MY_LOG("rANR.yad2 = 0x%8x", rANR.yad2);
    MY_LOG("rANR.lut1 = 0x%8x", rANR.lut1);
    MY_LOG("rANR.lut2 = 0x%8x", rANR.lut2);
    MY_LOG("rANR.lut3 = 0x%8x", rANR.lut3);
    MY_LOG("rANR.pty = 0x%8x", rANR.pty);
    MY_LOG("rANR.cad = 0x%8x", rANR.cad);
    MY_LOG("rANR.ptc = 0x%8x", rANR.ptc);
    MY_LOG("rANR.lce1 = 0x%8x", rANR.lce1);
    MY_LOG("rANR.lce2 = 0x%8x", rANR.lce2);
    MY_LOG("rANR.hp1 = 0x%8x", rANR.hp1);
    MY_LOG("rANR.hp2 = 0x%8x", rANR.hp2);
    MY_LOG("rANR.hp3 = 0x%8x", rANR.hp3);
    MY_LOG("rANR.acty = 0x%8x", rANR.acty);
    MY_LOG("rANR.actc = 0x%8x", rANR.actc);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CCR_T& rCCR)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_EE(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_EE_T& rEE)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rEE.srk_ctrl = 0x%8x", rEE.srk_ctrl);
    MY_LOG("rEE.clip_ctrl = 0x%8x", rEE.clip_ctrl);
    MY_LOG("rEE.hp_ctrl1 = 0x%8x", rEE.hp_ctrl1);
    MY_LOG("rEE.hp_ctrl2 = 0x%8x", rEE.hp_ctrl2);
    MY_LOG("rEE.ed_ctrl1 = 0x%8x", rEE.ed_ctrl1);
    MY_LOG("rEE.ed_ctrl2 = 0x%8x", rEE.ed_ctrl2);
    MY_LOG("rEE.ed_ctrl3 = 0x%8x", rEE.ed_ctrl3);
    MY_LOG("rEE.ed_ctrl4 = 0x%8x", rEE.ed_ctrl4);
    MY_LOG("rEE.ed_ctrl5 = 0x%8x", rEE.ed_ctrl5);
    MY_LOG("rEE.ed_ctrl6 = 0x%8x", rEE.ed_ctrl6);
    MY_LOG("rEE.ed_ctrl7 = 0x%8x", rEE.ed_ctrl7);
    MY_LOG("rEE.ee_link1 = 0x%8x", rEE.ee_link1);
    MY_LOG("rEE.ee_link2 = 0x%8x", rEE.ee_link2);
    MY_LOG("rEE.ee_link3 = 0x%8x", rEE.ee_link3);
    MY_LOG("rEE.ee_link4 = 0x%8x", rEE.ee_link4);
    MY_LOG("rEE.ee_link5 = 0x%8x", rEE.ee_link5);
#endif    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_CCM_T
IspTuningCustom::
evaluate_CCM_index(RAWIspCamInfo const& rCamInfo)
{
    MY_LOG("%s()\n", __FUNCTION__);
    
    MY_LOG(
        "[+evaluate_CCM_index]"
        "(eIdx_CCM, i4CCT, i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_CCM
        , rCamInfo.rAWBInfo.i4CCT
        , rCamInfo.rAWBInfo.i4FluorescentIndex);

    EIndex_CCM_T eIdx_CCM_new = rCamInfo.eIdx_CCM;

//    -----------------|---|---|--------------|---|---|------------------
//                                THA TH1 THB              THC TH2  THD

    MINT32 const THA = 3318;
    MINT32 const TH1 = 3484;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const TH2 = 5050;
    MINT32 const THD = 5316;
    MINT32 const F_IDX_TH1 = 25;
    MINT32 const F_IDX_TH2 = -25;

    switch  (rCamInfo.eIdx_CCM)
    {
    case eIDX_CCM_TL84:
        if  ( rCamInfo.rAWBInfo.i4CCT < THB )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex < F_IDX_TH2 )
                eIdx_CCM_new = eIDX_CCM_CWF;
            else 
                eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_CWF:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH1 )
                eIdx_CCM_new = eIDX_CCM_TL84;
            else 
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else 
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_D65:
        if  ( rCamInfo.rAWBInfo.i4CCT > THC )
        {
	        eIdx_CCM_new = eIDX_CCM_D65;
        } 
        else if ( rCamInfo.rAWBInfo.i4CCT > TH1 )
        {
            if(rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH2)
                eIdx_CCM_new = eIDX_CCM_TL84;
            else 
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else 
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        break;
    }

    if  ( rCamInfo.eIdx_CCM != eIdx_CCM_new )
    {
        MY_LOG(
            "[-evaluate_CCM_index] CCM Idx(old,new)=(%d,%d)"
            , rCamInfo.eIdx_CCM, eIdx_CCM_new
        );
    }

    return  eIdx_CCM_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_dynamic_ccm(RAWIspCamInfo const& rCamInfo)
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_PCA_LUT_T
IspTuningCustom::
evaluate_PCA_LUT_index(RAWIspCamInfo const& rCamInfo)
{
    MY_LOG("%s()\n", __FUNCTION__);

    // TODO: Add your code below...
    

    MY_LOG(
        "[+evaluate_PCA_LUT_index]"
        "(rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex
    );

    EIndex_PCA_LUT_T eIdx_PCA_LUT_new = rCamInfo.eIdx_PCA_LUT;

//    -----------------|-------|--------------|-------|------------------
//                    THA     THB            THC     THD

    MINT32 const THA = 3318;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const THD = 5316;

    switch  (rCamInfo.eIdx_PCA_LUT)
    {
    case eIDX_PCA_HIGH:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THC )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        break;
    case eIDX_PCA_MIDDLE:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        break;
    case eIDX_PCA_LOW:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
	        eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > THB )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        break;
    }

    if  ( rCamInfo.eIdx_PCA_LUT != eIdx_PCA_LUT_new )
    {
        MY_LOG(
            "[-evaluate_PCA_LUT_index] PCA_LUT_index(old,new)=(%d,%d)"
            , rCamInfo.eIdx_PCA_LUT, eIdx_PCA_LUT_new
        );
    }

    return eIdx_PCA_LUT_new;
}

/*******************************************************************************
*
* eIdx_Shading_CCT_old:
*   [in] the previous color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
* i4CCT:
*   [in] the current color temperature from 3A.
*
*
* return:
*   [out] the current color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
*******************************************************************************/
EIndex_Shading_CCT_T
IspTuningCustom::
evaluate_Shading_CCT_index  (
        RAWIspCamInfo const& rCamInfo
)   const
{
    UINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

	MINT32 i4FIDX	= rCamInfo.rAWBInfo.i4FluorescentIndex;//LINE <> <DATE20130923> <camera shading tuning> wupingzhou
	MINT32 i4DFIDX	= rCamInfo.rAWBInfo.i4DaylightFluorescentIndex;//LINE <> <DATE20130923> <camera shading tuning> wupingzhou
	MINT32 i4SceneLV = rCamInfo.rAWBInfo.i4SceneLV;//LINE <> <DATE20130923> <camera shading tuning> wupingzhou

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

	UINT32 sensorID;//LINE <> <DATE20130917> <OV5648 tuning> wupingzhou
    sensorID = getSensorID();//LINE <> <DATE20130917> <OV5648 tuning> wupingzhou

	MINT32 THL1 = 3100;//3257;
	MINT32 THH1 = 3400;//3484;
    MINT32 THL2 = 4673;
    MINT32 THH2 = 4900;
    MINT32 TH3  = 50;
    MINT32 LV_H = 80;
	
	if (sensorID == OV5648MIPI_SENSOR_ID || sensorID == OV5649MIPI_SENSOR_ID)//LINE <> <DATE20130917> <OV5648 tuning> wupingzhou
	{
		THL1 = 3500;
		THH1 = 3800;//3484;
	}
	else if(sensorID == OV8825_SENSOR_ID)
	{
		LOGD("[-evaluate_Shading_CCT_index][OV8825MIPI] This is sunny MTM module.");

		THL1	= 3100; // 2850;
		THH1	= 3400; // 3484;
		THL2	= 4673; // 4500;
		THH2	= 4900; // 4700;
		TH3		= 40;  // 20;
	}
	else if(sensorID == OV8826_SENSOR_ID)
	{
		LOGD("[-evaluate_Shading_CCT_index][OV8825MIPI] This is sunny MTM module.");

		THL1	= 3100;
		THH1	= 3400;
		THL2	= 4673;
		THH2	= 5155;
		TH3		= -20;	//20130103 Jouny
	}
	else if (sensorID == S5K3H7Y_SENSOR_ID)
		{

		     THL1	= 3100;
		    THH1	= 3400;
		    THL2	= 4700;
 		    THH2	= 5200;
 		     TH3	= -20;	//20140127  <S5K3H7Y darling>xuezhen 
	}
	else
	{
		THL1 = 3100;//3257;
		THH1 = 3400;//3484;
	}
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

	LOGD("[-evaluate_Shading_CCT_index] i4CCT :%d, i4FIDX:%d,i4DFIDX:%d, i4SceneLV : %d\n"
            ,i4CCT, i4FIDX, i4DFIDX,i4SceneLV);

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
             if(sensorID ==S5K3H7Y_SENSOR_ID)
            	{ 
            	     if(i4SceneLV > 90)
            	     	{
            	     	     eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
            	     	}
			else
				{
				    eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
            	}
			else
				
                         {
                            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
			   }
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            if(sensorID ==S5K3H7Y_SENSOR_ID)
            	{ 
            	     if(i4SceneLV > 90)
            	     	{
            	     	     eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
            	     	}
			else
				{
				    eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
            	}
			else
				
                         {
                            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
			   }
        }
        else
        {
			if (sensorID == IMX111_SENSOR_ID)//LINE <> <DATE20130923> <camera shading tuning> wupingzhou
			{
				if(i4SceneLV < 73 && i4FIDX < 75)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV5648MIPI_SENSOR_ID || sensorID == OV5649MIPI_SENSOR_ID)//LINE <> <DATE20131104> <camera shading tuning> wupingzhou
			{
				if(i4SceneLV < 93 && i4FIDX < 53)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV8826_SENSOR_ID)//LINE <> <DATE20131104> <camera shading tuning> wupingzhou
			{
				if(i4DFIDX < TH3)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV8825_SENSOR_ID) // LINE <> <2013.1.23> <camera shading tuning> Jiangde
			{                               
				if(i4DFIDX < TH3)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}            
			else
			{
            	eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
            }
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
         eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
             if(sensorID ==S5K3H7Y_SENSOR_ID)
            	{ 
            	     if(i4SceneLV > 90)
            	     	{
            	     	     eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
            	     	}
			else
				{
				    eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
            	}
			else
				
                         {
                            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
			   }
        }
        else
        {
			if (sensorID == IMX111_SENSOR_ID)//LINE <> <DATE20130923> <camera shading tuning> wupingzhou
			{
				if(i4SceneLV < 71 && i4FIDX < 75)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV5648MIPI_SENSOR_ID || sensorID == OV5649MIPI_SENSOR_ID)//LINE <> <DATE20131104> <camera shading tuning> wupingzhou
			{
				if(i4SceneLV < 93 && i4FIDX < 53)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV8826_SENSOR_ID)//LINE <> <DATE20131104> <camera shading tuning> wupingzhou
			{
				if(i4DFIDX < TH3)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}
			else if (sensorID == OV8825_SENSOR_ID) // LINE <> <2013.1.23> <camera shading tuning> Jiangde
			{
				if(i4DFIDX < TH3)
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
				}
				else
				{
					eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
				}
			}            
			else
			{
            	eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
            }
        }
        break;
    }


// BEGIN <><2014.1.23><Sunny 8825> Jiangde
/*
 // case for Mixing light source
 	if(sensorID == OV8825_SENSOR_ID)
	{
      if (eIdx_Shading_CCT_new == eIDX_Shading_CCT_CWF){
        if ((i4CCT >= 4300) && (i4CCT < 4600)){
    	    if (i4SceneLV >= LV_H){
               LOGD(
                   " Switch to i4SceneLV(H) = %d\n"
                   ,i4SceneLV
    	       );
               eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65; 
            }
        }
      }
	}
*/ 
// END <><2014.1.23><Sunny 8825> Jiangde

//#if ENABLE_MY_LOG
    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        LOGD(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }
//#endif
		//2013/6/11 ¤U¤È 09:37:17eIdx_Shading_CCT_new	= eIDX_Shading_CCT_ALight;
		MY_LOG("Set Shading Index :%d\n", eIdx_Shading_CCT_new);
    return  eIdx_Shading_CCT_new;
}
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_index(MUINT32 const u4Iso) const
{
    MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso < 150 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 300 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 600 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 1200 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso < 2000 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso < 2800 )
    {
        return  eIDX_ISO_2400;
    }

    return  eIDX_ISO_3200;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refineLightSourceAWBGainforMultiCCM(AWB_GAIN_T& rD65, AWB_GAIN_T& rTL84, AWB_GAIN_T& rCWF, AWB_GAIN_T& rA)
{
    MY_LOG("%s()\n", __FUNCTION__);

    MY_LOG("D65 AWB Gain = (%d, %d, %d)\n", rD65.i4R, rD65.i4G, rD65.i4B);
    MY_LOG("TL84 AWB Gain = (%d, %d, %d)\n", rTL84.i4R, rTL84.i4G, rTL84.i4B);
    MY_LOG("CWF AWB Gain = (%d, %d, %d)\n", rCWF.i4R, rCWF.i4G, rCWF.i4B);
    MY_LOG("A AWB Gain = (%d, %d, %d)\n", rA.i4R, rA.i4G, rA.i4B);
}


