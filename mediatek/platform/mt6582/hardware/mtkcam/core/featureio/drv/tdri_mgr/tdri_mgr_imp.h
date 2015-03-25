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
#ifndef _TDRI_MGR_IMP_H_
#define _TDRI_MGR_IMP_H_

#include <cutils/xlog.h>    // for XLOG?().

#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/featureio/tdri_mgr.h>
#include <mtkcam/drv/tpipe_drv.h>
#include <mtkcam/drv/imem_drv.h>

//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""
#undef   DBG_LOG_LEVEL                      // Decide Debug Log level for current file. Can only choose from 2~8.
#define  DBG_LOG_LEVEL      4               // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).


#define DBG_LOG_LEVEL_SILENT	8
#define DBG_LOG_LEVEL_ASSERT	7
#define DBG_LOG_LEVEL_ERROR		6
#define DBG_LOG_LEVEL_WARN		5
#define DBG_LOG_LEVEL_INFO		4
#define DBG_LOG_LEVEL_DEBUG		3
#define DBG_LOG_LEVEL_VERBOSE	2
#undef	__func__
#define	__func__	__FUNCTION__


#if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_SILENT)		// 8
	#undef LOG_AST
	#undef LOG_ERR
	#undef LOG_WRN
	#undef LOG_INF
	#undef LOG_DBG
	#undef LOG_VRB
	#define LOG_AST(cond, fmt, arg...)
	#define LOG_ERR(fmt, arg...)
	#define LOG_WRN(fmt, arg...)
	#define LOG_INF(fmt, arg...)
	#define LOG_DBG(fmt, arg...)
	#define LOG_VRB(fmt, arg...)
#endif	// (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_SILENT)



#ifndef USING_MTK_LDVT   // Not using LDVT.
//    #include <cutils/xlog.h>
    #define NEW_LINE_CHAR           ""      // XLOG?() already includes a new line char at the end of line, so don't have to add one.

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)		// 7
        #undef LOG_AST
        #define LOG_AST(cond, fmt, arg...)      \
            do {        \
                if (!(cond))        \
                    XLOGE("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
            } while (0)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)		// 6
        #undef LOG_ERR
    	#define LOG_ERR(fmt, arg...)        XLOGE(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)		// 5
        #undef LOG_WRN
    	#define LOG_WRN(fmt, arg...)        XLOGW(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)		// 4
        #undef LOG_INF
    	#define LOG_INF(fmt, arg...)        XLOGI(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)		// 3
        #undef LOG_DBG
    	#define LOG_DBG(fmt, arg...)        XLOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)	// 2
        #undef LOG_VRB
    	#define LOG_VRB(fmt, arg...)        XLOGV(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #endif
#else   // Using LDVT.
    #include "uvvf.h"
    #define NEW_LINE_CHAR           "\n"

#if 1	// Enable LOG_*().
    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)		// 7
        #undef LOG_AST
        #define LOG_AST(expr, fmt, arg...)                                                                                                       \
            do {                                                                                                                                    \
                if (!(expr))                                                                                                                        \
                    VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);     \
            } while (0)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)		// 6
        #undef LOG_ERR
    	#define LOG_ERR(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)		// 5
        #undef LOG_WRN
    	#define LOG_WRN(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)		// 4
        #undef LOG_INF
    	#define LOG_INF(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)		// 3
        #undef LOG_DBG
    	#define LOG_DBG(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)	// 2
        #undef LOG_VRB
    	#define LOG_VRB(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #endif
#else	// Disable LOG_*().
    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ASSERT)		// 7
        #undef LOG_AST
        #define LOG_AST(expr, fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_ERROR)		// 6
        #undef LOG_ERR
    	#define LOG_ERR(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_WARN)		// 5
        #undef LOG_WRN
    	#define LOG_WRN(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_INFO)		// 4
        #undef LOG_INF
    	#define LOG_INF(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_DEBUG)		// 3
        #undef LOG_DBG
    	#define LOG_DBG(fmt, arg...)
    #endif

    #if (DBG_LOG_LEVEL <= DBG_LOG_LEVEL_VERBOSE)	// 2
        #undef LOG_VRB
    	#define LOG_VRB(fmt, arg...)
    #endif

#endif	// Enable/Disable LOG_*().

#endif  // USING_MTK_LDVT


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

typedef struct
{
   // int lce_en;
    int bnr_en;
    int lsc_en;
  //  int mfb_en;
    int cfa_en;
    int nbc_en;
    int seee_en;
   // int nr3d_en;
   // int imgci_en;
    int lsci_en;
   // int lcei_en;
    int sl2_en;
    //
    TdriBnrCfg bnr;
    TdriLscCfg lsc;
    //TdriLceCfg lce;
    TdriNbcCfg nbc;
    TdriSeeeCfg seee;
    //TdriMfbCfg mfb;
    TdriCfaCfg cfa;
    //int imgci_stride;
    int lsci_stride;
    //int lcei_stride;
    Tdrisl2Cfg sl2;
}TDRI_MGR_TPIPE_TABLE_TURNING;


typedef struct
{
    MUINT32*                        pDescriptorArray;   //CAM_MODULE_ENUM
    MUINT32*                        pDesriptorNum;      // the descriptor number that need to be updated
    MUINT32*                        pTopCtlEn1;         // top ctl en1 flag
    MUINT32*                        pTopCtlEn2;         // top ctl en2 flag
    MUINT32*                        pTopCtlDma;         // top ctl DMA flag
    TdriDrvCfg                      tdriMgrCfg;         // tpipe setting for tpipe driver
    TDRI_MGR_TPIPE_TABLE_TURNING    tdriTurningSetting;     // tpipe driver setting buffer for turning instantaneous
    IspDrv      *pVirIspDrv;
    isp_reg_t   *pVirtIspReg;
}TDRI_MGR_INFO;


typedef struct
{
    ISP_DRV_CQ_ENUM  ispCQ;
    TPIPE_DRV_CQ_ENUM tdriMgrCQ;
}TDRI_MGR_CQ_MAPPING;


typedef struct
{
    TDRI_MGR_FUNC_ENUM                  tdriMgrFunc;    // tpipe mgr function
    CAM_MODULE_ENUM                     cqFuncGrp1;     // update CQ function(group 1)
    CAM_MODULE_ENUM                     cqFuncGrp2;     // update CQ function(group 2)
    CAM_MODULE_ENUM                     cqFuncGrp3;     // update CQ function(group 3)
    MUINT32                             tpipeDrvFunc;   // update tpipe function
    MUINT32                             topCtlEn1;      // top En1
    MUINT32                             topCtlEn2;      // top En2
    MUINT32                             topCtlDma;      // top DMA
}TDRI_MGR_FUNC_MAPPING;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TdriMgrImp : public TdriMgr
{
    friend  TdriMgr& TdriMgr::getInstance();
    public:

    protected:
        TdriMgrImp();
        ~TdriMgrImp();
    //
    public:
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        //
        virtual MBOOL  flushSetting(ISP_DRV_CQ_ENUM ispCq);
        virtual MBOOL  applySetting(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM tmgFunc);
        //
        virtual MBOOL  setBnr(ISP_DRV_CQ_ENUM ispCq, MBOOL bnrEn, int bpcEn);//, int bpc_tbl_en, int bpc_tbl_size, int imgciEn, int imgciStride);
        virtual MBOOL  setLsc(ISP_DRV_CQ_ENUM ispCq, MBOOL lscEn, int sdblk_width, int sdblk_xnum, int sdblk_last_width,
                                    int sdblk_height, int sdblk_ynum, int sdblk_last_height, int lsciEn, int lsciStride);
      //  virtual MBOOL  setLce(ISP_DRV_CQ_ENUM ispCq, MBOOL lceEn, int lce_bc_mag_kubnx, int lce_offset_x,
      //                              int lce_bias_x, int lce_slm_width, int lce_bc_mag_kubny,
      //                              int lce_offset_y, int lce_bias_y, int lce_slm_height, int lceiEn, int lceiStride);
        virtual MBOOL  setNbc(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int anr_eny,
                                    int anr_enc, int anr_iir_mode, int anr_scale_mode);
        virtual MBOOL  setSeee(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int se_edge);//, int usm_over_shrink_en);
        virtual MBOOL  setSl2(ISP_DRV_CQ_ENUM ispCq, MBOOL en);
      //  virtual MBOOL  setMfb(ISP_DRV_CQ_ENUM ispCq, int bld_mode, int bld_deblock_en);
        virtual MBOOL  setCfa(ISP_DRV_CQ_ENUM ispCq, int bayer_bypass);
       // virtual MBOOL  setNr3dTop(ISP_DRV_CQ_ENUM ispCq, MBOOL en);
        virtual MBOOL  setOtherEngin(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM engin);
        //
    private:
        MBOOL  refreshTableSetting(TPIPE_DRV_CQ_ENUM tmgrCq, MINT32 tableUpdateFlag);
        static MINT32 mTdriMgrCB(MINT32 ispCqNum, MVOID *user);
        virtual MBOOL  handleTpipeTable(TPIPE_DRV_CQ_ENUM tmgrCq);
    //
    private:
        //
        TpipeDrv    *pTdriDri;
        IspDrv      *pIspDrv;
        //
        mutable Mutex       mLock;
        mutable Mutex       mUpdateLock;
        volatile MINT32     mInitCount;
        //
        TDRI_MGR_INFO tdriMgrInfo[TPIPE_DRV_CQ_NUM];
        //
};

//-----------------------------------------------------------------------------
#endif // _TDRI_MGR_IMP_H_

