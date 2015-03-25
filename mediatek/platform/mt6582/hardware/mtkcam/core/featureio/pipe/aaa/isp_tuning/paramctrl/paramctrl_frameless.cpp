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
#define LOG_TAG "paramctrl_frameless"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <camera_feature.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <lsc_mgr.h>
#include <dbg_isp_param.h>
#include "paramctrl_if.h"
#include "paramctrl.h"

using namespace android;
using namespace NSIspTuning;


MBOOL
Paramctrl::
applyToHw_Frameless_All()
{
    return  MTRUE
        //&&  ISP_MGR_SHADING_T::getInstance().apply()
        ;
}


MBOOL
Paramctrl::
prepareHw_Frameless_All()
{
    MBOOL fgRet = MTRUE;

    //  (1) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepare_Frameless_Shading()
            ;
    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    return  fgRet;
}


MBOOL
Paramctrl::
prepare_Frameless_Shading()
{
    static MBOOL MetaModeInit = MFALSE;
    MBOOL fgRet = MTRUE;

    MY_LOG("[%s] mode %d, profile %d", __FUNCTION__, getOperMode(), m_rIspCamInfo.eIspProfile);
//    printf("[%s] OpMode %d, profile %d\n", __FUNCTION__,
//            getOperMode(),
//            m_rIspCamInfo.eIspProfile);

    switch  (getOperMode())
    {
    case EOperMode_Normal:
    case EOperMode_PureRaw:
        MY_LOG("%s m_pLscMgr EOperMode_Normal", __FUNCTION__);
        m_pLscMgr->setIspProfile(m_rIspCamInfo.eIspProfile);
        m_pLscMgr->SetTBAToISP();
        m_pLscMgr->enableLsc(MTRUE);
        break;
    case EOperMode_EM:
    case EOperMode_Meta:
        MY_LOG("%s EOperMode_Meta", __FUNCTION__);
            m_pLscMgr->setMetaIspProfile(m_rIspCamInfo.eIspProfile, getSensorMode());
            m_pLscMgr->ConfigUpdate();
            m_pLscMgr->SetTBAToISP();

                if (m_pLscMgr->isEnable() == MTRUE)
                    m_pLscMgr->enableLsc(MTRUE);
                else
                    m_pLscMgr->enableLsc(MFALSE);

        break;
    default:
        MY_LOG("[%s] Wrong OpMode %d", __FUNCTION__, getOperMode());
        return MTRUE;
    }

    // debug message
    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    m_IspNvramMgr.setIdx_LSC(m_pLscMgr->getRegIdx());
#if 0
/*
    UINT8 Shading_Parameter_IDX;

    if ((m_IspNvramMgr.getIdx_Shading()!=0)&&(m_IspNvramMgr.getIdx_Shading()!=1))
    {
        MY_LOG(
                    "[prepare_Frameless_Shading] "
                    " Shading Parameter Index 2, Disable Shading\n"
                );
        m_LscMgr.setMode(2); // Set binnig mode, should disable shading
        m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
        fgRet = m_LscMgr.enableLsc(0);// disable shading
        goto lbExit;
    }


    MY_LOG(
                "[prepare_Frameless_Shading] "
                " (CamMode,SensorMode,operation mode, CCT) = (%d,%d,%d,%d)\n"
                , m_IspCamInfo.eCamMode //   ECamMode_Video          = 0,     ECamMode_Online_Preview = ECamMode_Video,     ECamMode_Online_Capture,
                , getSensorMode()
                , getOperMode()
                , m_IspCamInfo.eIdx_Shading_CCT
            );

    // (1) free allocated SRAM when mode change
    //      The very 1st time when camera start-up, free EUsr_Shading_Capture will cause error.
    //      Ignore this error code from SRAM driver is ok.
    switch (m_IspCamInfo.eCamMode)
    {
         case ECamMode_Online_Preview:
         case ECamMode_Video:
             m_SysramMgr.free(NSIspSysram::EUsr_Shading_Capture);
             break;
         case ECamMode_Online_Capture:
         case ECamMode_Online_Capture_ZSD:
         case ECamMode_Offline_Capture_Pass1:
         case ECamMode_HDR_Cap_Pass1_SF:
         case ECamMode_HDR_Cap_Pass1_MF2:
             switch (getSensorMode())
             {
                 case ESensorMode_Preview : //engineer mode preivew raw capture
        	     m_SysramMgr.free(NSIspSysram::EUsr_Shading_Capture);
        	     break;
      		 case ESensorMode_Capture :
      		 default :
        	     m_SysramMgr.free(NSIspSysram::EUsr_Shading_Preview);
        	     break;
             }
             break;
         case ECamMode_Offline_Capture_Pass2:
         case ECamMode_HDR_Cap_Pass1_MF1:
         case ECamMode_HDR_Cap_Pass2:
         default:
              break;
    }

    //  (2) Here, make sure that sysram is available.
    //  re-allocate is acceptable, but only new allocate will reserve
    {
        MVOID* pPhyAddr = NULL;
        MVOID* pVirAddr = NULL;
        MERROR_ENUM err = MERR_OK;

        switch (m_IspCamInfo.eCamMode)
        {
            case ECamMode_Online_Preview:
            case ECamMode_Video:
        	m_LscMgr.setMode(0); // Set preview mode for  LscMgr get table size
        	err = m_SysramMgr.autoAlloc(
        	    NSIspSysram::EUsr_Shading_Preview, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
        	);
        	break;
            case ECamMode_Online_Capture:
            case ECamMode_Online_Capture_ZSD:
            case ECamMode_Offline_Capture_Pass1:
            case ECamMode_HDR_Cap_Pass1_SF:
            case ECamMode_HDR_Cap_Pass1_MF2:
        	switch (getSensorMode())
        	{
        	    case ESensorMode_Preview : //engineer mode preivew raw capture, allocate preview buffer
		        m_LscMgr.setMode(0); // Set preview mode for  LscMgr get table size
		        err = m_SysramMgr.autoAlloc(
		            NSIspSysram::EUsr_Shading_Preview, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
		        );
        		break;
      		    case ESensorMode_Capture :
      		    default :
		        m_LscMgr.setMode(1); // Set capture mode for  LscMgr get table size
		        err = m_SysramMgr.autoAlloc(
		            NSIspSysram::EUsr_Shading_Capture, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
		        );
        		break;
        	 }
        	 break;
            case ECamMode_Offline_Capture_Pass2:
            case ECamMode_HDR_Cap_Pass1_MF1:
            case ECamMode_HDR_Cap_Pass2:
        	switch (getSensorMode())
        	{
        	    case ESensorMode_Preview :
		        m_LscMgr.setMode(0); // Set preview mode for  configure shading parameters
        		break;
      		    case ESensorMode_Capture :
      		    default :
		        m_LscMgr.setMode(1); // Set capture  mode for  configure shading parameters
        		break;
        	 }
        	 break;
            default:
        	 break;
        }
        if  ( MERR_OK != err )
        {
            fgRet = MFALSE;
            goto lbExit;
        }

      MY_LOG(
                "[prepare_Frameless_Shading] "
                "Shading table address  Phy 0x%x Vir 0x%x\n"
                ,reinterpret_cast<MUINT32>(pPhyAddr)
                ,reinterpret_cast<MUINT32>(pVirAddr)
                );

        m_LscMgr.savePhyTBA(pPhyAddr);
        m_LscMgr.saveVirTBA(pVirAddr);
    }

    //  (3) prepare shading table and parameter for normal operation
    //       meta mode will be setted at "prepareHw_PerFrame_Shading"
    //       1. write shading parameter to register
    //       2. Load Shading table to SRAM
    if (getOperMode() == EOperMode_Meta)
    {
         // capture doesn't call "prepareHw_PerFrame_Shading", must set shading parameters here
         if (m_LscMgr.isEnable()!=0)
        {
            m_LscMgr.loadLut(); //load table
            fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
            if (fgRet != MTRUE) // update table address
            {
                MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "Set table address fail %d \n"
                                ,fgRet
                             );
            }
        }
	m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575

        //shouldn't chage any parameter unless user change it in meta mode.
        //but 2nd pass is an exception, must disable lsc.
        if(m_IspCamInfo.eCamMode == ECamMode_Offline_Capture_Pass2)
        {
	    fgRet = m_LscMgr.enableLsc(0);// disable shading
        }
        LOGD(
                      "[prepare_Frameless_Shading] "
                      "NVRAM data change"
                      " (CamMode,SensorMode, Operation_Mode) = (%d,%d,%d)\n"
                      "Lsc_mode, Lsc_idx (%d,%d) \n"
                      , m_IspCamInfo.eCamMode //   ECamMode_Video          = 0,     ECamMode_Online_Preview = ECamMode_Video,     ECamMode_Online_Capture,
                      , getSensorMode()
                      , getOperMode()
                      , m_LscMgr.getMode()
                      , m_LscMgr.getIdx()
                      );
    }
    else
    {
    if( (m_IspCamInfo.eCamMode == ECamMode_Offline_Capture_Pass2)
    ||  (m_IspCamInfo.eCamMode == ECamMode_HDR_Cap_Pass1_MF1)
    ||  (m_IspCamInfo.eCamMode == ECamMode_HDR_Cap_Pass2)
    ||	getOperMode()==EOperMode_PureRaw)
    {
	     m_LscMgr.setIdx(0);  //
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
	    if (fgRet != MTRUE) // update table address
	    {
		      MY_LOG(
		                "[prepare_Frameless_Shading] "
		                "Set table address fail %d \n"
		                ,fgRet
		            );
	    }
	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
	    fgRet = m_LscMgr.enableLsc(0);// disable shading
    }
    else
    {
           if (m_IspCamInfo.eIdx_Shading_CCT < eIDX_Shading_CCT_D65)
           {
	         m_LscMgr.setIdx(1);  // CWF and A Light both using 2nd talbe
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
           }
           else
           {
	         m_LscMgr.setIdx(2);  // D65  using 3rd table
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
           }
	    m_LscMgr.loadLut(); //load table
	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
	    if (fgRet != MTRUE) // update table address
	    {
		      MY_LOG(
		                "[prepare_Frameless_Shading] "
		                "Set table address fail %d \n"
		                ,fgRet
		            );
	    }
	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
	    fgRet = m_LscMgr.enableLsc(1);// enable shading
    }

ISP_NVRAM_SHADING_T debug;
ISP_MGR_SHADING_T::getInstance().get(debug);
      MY_LOG(
                    "[prepare_Frameless_Shading] "
                "Shading param 0x%x,0x%x ,0x%x ,0x%x ,0x%x \n"
                , debug.shading_ctrl1.val
                , debug.shading_ctrl2.val
                , debug.shading_read_addr.val
                , debug.shading_last_blk.val
                , debug.shading_ratio_cfg.val
            );
    }

lbExit:

      MY_LOG(
                "[prepare_Frameless_Shading] "
                " (lsc_idx, lsc_mode, fgret) = (%d, %d, %d)\n"
                ,m_LscMgr.getIdx()
                ,m_LscMgr.getMode()
                , fgRet
            );
*/
#endif
    return  fgRet;
}

