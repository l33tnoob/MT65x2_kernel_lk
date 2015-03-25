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
#ifndef _TDRI_MGR_H_
#define _TDRI_MGR_H_
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
typedef enum {
    //// need to re-calculate tpipe table
    TDRI_MGR_FUNC_BNR   = 0,
    TDRI_MGR_FUNC_LSC,
    TDRI_MGR_FUNC_MFB,
    TDRI_MGR_FUNC_CFA,
    TDRI_MGR_FUNC_NBC,
    TDRI_MGR_FUNC_SEEE,
    TDRI_MGR_FUNC_LCE_BASIC,
    TDRI_MGR_FUNC_NR3D_TOP,
    //
    TDRI_MGR_FUNC_NR3D,
    TDRI_MGR_FUNC_LCE_CUSTOM,
    TDRI_MGR_FUNC_OBC,
    TDRI_MGR_FUNC_PGN,
    TDRI_MGR_FUNC_CCL,
    TDRI_MGR_FUNC_G2G,
    TDRI_MGR_FUNC_G2C,
    TDRI_MGR_FUNC_DGM,
    TDRI_MGR_FUNC_GGMRB,
    TDRI_MGR_FUNC_GGMG,
    TDRI_MGR_FUNC_GGM_CTL,
    TDRI_MGR_FUNC_PCA,
    TDRI_MGR_FUNC_PCA_CON,
    TDRI_MGR_FUNC_NUM,
}TDRI_MGR_FUNC_ENUM;


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class TdriMgr
{
    public:

    protected:
        TdriMgr() {};
        virtual ~TdriMgr() {};
    //
    public:
        static TdriMgr& getInstance();
        virtual int     init(void) = 0;
        virtual int     uninit(void) = 0;
        //
        virtual MBOOL   flushSetting(ISP_DRV_CQ_ENUM ispCq) = 0;
        virtual MBOOL   applySetting(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM tmgFunc) = 0;
        //
        virtual MBOOL  setBnr(ISP_DRV_CQ_ENUM ispCq, MBOOL bnrEn, int bpcEn, int bpc_tbl_en, int bpc_tbl_size, int imgciEn, int imgciStride) = 0;
        virtual MBOOL  setLsc(ISP_DRV_CQ_ENUM ispCq, MBOOL lscEn, int sdblk_width, int sdblk_xnum, int sdblk_last_width,
                                    int sdblk_height, int sdblk_ynum, int sdblk_last_height, int lsciEn, int lsciStride) = 0;
        virtual MBOOL  setLce(ISP_DRV_CQ_ENUM ispCq, MBOOL lceEn, int lce_bc_mag_kubnx, int lce_offset_x,
                                    int lce_bias_x, int lce_slm_width, int lce_bc_mag_kubny,
                                    int lce_offset_y, int lce_bias_y, int lce_slm_height, int lceiEn, int lceiStride) = 0;
        virtual MBOOL  setNbc(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int anr_eny,
                            int anr_enc, int anr_iir_mode, int anr_scale_mode) = 0;
        virtual MBOOL  setSeee(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int se_edge, int usm_over_shrink_en) = 0;
        virtual MBOOL  setMfb(ISP_DRV_CQ_ENUM ispCq, int bld_mode, int bld_deblock_en) = 0;
        virtual MBOOL  setCfa(ISP_DRV_CQ_ENUM ispCq, int bayer_bypass) = 0;
        virtual MBOOL  setNr3dTop(ISP_DRV_CQ_ENUM ispCq, MBOOL en) = 0;
        virtual MBOOL  setOtherEngin(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM engin) = 0;
};


//-----------------------------------------------------------------------------
#endif  // _TDRI_MGR_H_

