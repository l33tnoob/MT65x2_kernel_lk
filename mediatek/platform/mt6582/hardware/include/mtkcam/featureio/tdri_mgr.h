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

 /**
 * @file tdri_mgr.h
 * @brief ISP tnuing function manager
 */
 
#ifndef _TDRI_MGR_H_
#define _TDRI_MGR_H_
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
 
/**  
 * @brief enum for ISP tuning function
 */


typedef enum {
    //// need to re-calculate tpipe table
    TDRI_MGR_FUNC_BNR   = 0, ///< update BNR tuning table to CQ
    TDRI_MGR_FUNC_LSC,       ///< update LSC tuning table to CQ
    //TDRI_MGR_FUNC_MFB,
    TDRI_MGR_FUNC_CFA,       ///< update CFA tuning table to CQ
    TDRI_MGR_FUNC_NBC,       ///< update NBC tuning table to CQ
    TDRI_MGR_FUNC_SEEE,      ///< update SEEE tuning table to CQ
    //TDRI_MGR_FUNC_LCE_BASIC, 
    TDRI_MGR_FUNC_SL2,       ///< update SL2 tuning table to CQ
    //TDRI_MGR_FUNC_NR3D_TOP,
    //
    //TDRI_MGR_FUNC_NR3D,
    //TDRI_MGR_FUNC_LCE_CUSTOM,
    TDRI_MGR_FUNC_OBC,       ///< update OBC tuning table to CQ
    TDRI_MGR_FUNC_PGN,       ///< update PGN tuning table to CQ
    TDRI_MGR_FUNC_CCL,       ///< update CCL tuning table to CQ
    TDRI_MGR_FUNC_G2G,       ///< update G2G tuning table to CQ
    TDRI_MGR_FUNC_G2C,       ///< update G2C tuning table to CQ
    //TDRI_MGR_FUNC_DGM,
    TDRI_MGR_FUNC_GGMRB,     ///< update GGMRB tuning table to CQ
    TDRI_MGR_FUNC_GGMG,      ///< update GGMG tuning table to CQ
    TDRI_MGR_FUNC_GGM_CTL,   ///< update GGM_CTL tuning table to CQ
    TDRI_MGR_FUNC_PCA,       ///< update PCA tuning table to CQ
    TDRI_MGR_FUNC_PCA_CON,   ///< update PCA_CON tuning table to CQ
    TDRI_MGR_FUNC_NUM,       ///< update isp tuning function number
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
 
/**  
 * @brief ISP tuning function and tdri parameter manager
 */

class TdriMgr
{
    public:

    protected:
        TdriMgr() {};
        virtual ~TdriMgr() {};
    //
    public:

	    /**  
            * @brief get instance
            */
        static TdriMgr& getInstance();
		/**  
            * @brief init function          
            */    
        virtual int     init(void) = 0;
		/**  
            * @brief uninit function          
            */    
        virtual int     uninit(void) = 0;
        /**  
            * @brief flush tuning settings
            * @param [in] ispCq CQ mode
            */
        virtual MBOOL   flushSetting(ISP_DRV_CQ_ENUM ispCq) = 0;
		/**  
            * @brief apply isp tuning settings to CQ
            * @param [in] ispCq CQ mode
            * @param [in] tmgFunc enum for ISP tuning function         
            */ 
        virtual MBOOL   applySetting(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM tmgFunc) = 0;
        /**  
            * @brief set BNR tuning function
            * @param [in] ispCq CQ mode
            * @param [in] bnrEn BR enable
            * @param [in] bpcEn BPC enable
            */ 
        virtual MBOOL  setBnr(ISP_DRV_CQ_ENUM ispCq, MBOOL bnrEn, int bpcEn)=0;//, int bpc_tbl_en, int bpc_tbl_size, int imgciEn, int imgciStride) = 0;
        /**  
            * @brief set LSC tuning function
            * @param [in] ispCq CQ mode
            * @param [in] lscEn LSC enable
            * @param [in] sdblk_width block width in x direction
            * @param [in] sdblk_xnum x directional number of block
            * @param [in] sdblk_last_width last block width
            * @param [in] sdblk_height block height in y direction
            * @param [in] sdblk_ynum block width in x direction
            * @param [in] sdblk_last_height last block height
            * @param [in] lsciEn LSCI enable
            * @param [in] lsciStride LSCI stride
            */ 
        virtual MBOOL  setLsc(ISP_DRV_CQ_ENUM ispCq, MBOOL lscEn, int sdblk_width, int sdblk_xnum, int sdblk_last_width,
                                    int sdblk_height, int sdblk_ynum, int sdblk_last_height, int lsciEn, int lsciStride) = 0;
        //virtual MBOOL  setLce(ISP_DRV_CQ_ENUM ispCq, MBOOL lceEn, int lce_bc_mag_kubnx, int lce_offset_x,
        //                            int lce_bias_x, int lce_slm_width, int lce_bc_mag_kubny,
        //                            int lce_offset_y, int lce_bias_y, int lce_slm_height, int lceiEn, int lceiStride) = 0;
        /**  
            * @brief set NBC tuning function
            * @param [in] ispCq CQ mode
            * @param [in] en NBC enable
            * @param [in] anr_eny Enable Y ANR
            * @param [in] anr_enc Enable C ANR
            * @param [in] anr_iir_mode In tdri mode, no IIR
            * @param [in] anr_scale_mode Different in tdri mode and frame mode
            */ 
        virtual MBOOL  setNbc(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int anr_eny,
                            int anr_enc, int anr_iir_mode, int anr_scale_mode) = 0;
		 /**  
            * @brief set SEEE tuning function
            * @param [in] ispCq CQ mode
            * @param [in] en EEE enable
            * @param [in] se_edge Select source of edge enhancement 
            */ 		
        virtual MBOOL  setSeee(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int se_edge)=0;//, int usm_over_shrink_en) = 0;

		/**  
            * @brief set SL2 tuning function
            * @param [in] ispCq CQ mode
            * @param [in] en SL2 enable           
            */ 
        virtual MBOOL  setSl2(ISP_DRV_CQ_ENUM ispCq, MBOOL en) = 0;
        //virtual MBOOL  setMfb(ISP_DRV_CQ_ENUM ispCq, int bld_mode, int bld_deblock_en) = 0;
        /**  
            * @brief set CFA tuning function
            * @param [in] ispCq CQ mode
            * @param [in] bayer_bypass Bypass bayer to rgb process           
            */ 
        virtual MBOOL  setCfa(ISP_DRV_CQ_ENUM ispCq, int bayer_bypass) = 0;
		 
       // virtual MBOOL  setNr3dTop(ISP_DRV_CQ_ENUM ispCq, MBOOL en) = 0;
        /**  
            * @brief set other engine
            * @param [in] ispCq CQ mode
            * @param [in] engin enum for ISP tuning function         
            */ 
        virtual MBOOL  setOtherEngin(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM engin) = 0;
};


//-----------------------------------------------------------------------------
#endif  // _TDRI_MGR_H_

