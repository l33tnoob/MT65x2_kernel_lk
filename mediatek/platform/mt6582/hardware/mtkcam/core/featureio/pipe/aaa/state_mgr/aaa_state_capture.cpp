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
#define LOG_TAG "aaa_state_capture"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <awb_mgr.h>
#include <buf_mgr.h>
#include "aaa_state.h"
#include <mtkcam/common.h>
using namespace NSCam;
#include <camera_custom_AEPlinetable.h>
#include <ae_param.h>
#include <ae_mgr.h>
#include <flash_mgr.h>
#include <mtkcam/hal/sensor_hal.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <af_mgr.h>
#include <isp_tuning.h>
#include <dbg_isp_param.h>
#include <isp_tuning_mgr.h>
#include <lsc_mgr.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.


using namespace NS3A;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCapture::
StateCapture()
    : IState("StateCapture")
{
	MY_LOG("IState(StateCapture)  line=%d", __LINE__);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CaptureStart>)
{
	MY_LOG("sendIntent(intent2type<eIntent_CaptureStart>)  line=%d", __LINE__);

/*
	FlashMgr::getInstance()->endPrecapture();
	if(FlashMgr::getInstance()->isBurstShotMode()==1)
	{
		FlashMgr::getInstance()->changeBurstEngLevel();
	}
	*/


    // AE: update capture parameter
    AeMgr::getInstance().doCapAE();

    AwbMgr::getInstance().cameraCaptureInit();

    /*if ((IspTuningMgr::getInstance().getOperMode() == EOperMode_Normal) ||
        (IspTuningMgr::getInstance().getSensorMode() == ESensorMode_Capture))*/ {

        // AAO DMA / state enable again
        MRESULT err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AAO>());
        if (FAILED(err)) {
            MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AAO) fail\n");
            return err;
        }

        err = BufMgr::getInstance().AAStatEnable(MTRUE);
        if (FAILED(err)) {
            MY_ERR("BufMgr::getInstance().AAStatEnable(MTRUE) fail\n");
            return err;
        }


        // AFO DMA / state enable again
        err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AFO>());
        if (FAILED(err)) {
            MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AFO) fail\n");
            return err;
        }

        err = BufMgr::getInstance().AFStatEnable(MTRUE);
        if (FAILED(err)) {
            MY_ERR("BufMgr::getInstance().AFStatEnable(MTRUE) fail\n");
            return err;
        }

    }

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CaptureEnd>)
{
	MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>)  line=%d", __LINE__);
    BufInfo_T rBufInfo;
    MRESULT err;

	//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "FlhChkFire_E");
    //


    /*if ((IspTuningMgr::getInstance().getOperMode() == EOperMode_Normal) ||
        (IspTuningMgr::getInstance().getSensorMode() == ESensorMode_Capture))*/ {

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrdQAA");
        // Dequeue AAO DMA buffer
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AWBDoCapAWB");
        // One-shot AWB
        MINT32 i4SceneLV = AeMgr::getInstance().getLVvalue(MFALSE);

        AwbMgr::getInstance().doCapAWB(i4SceneLV, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));

        MY_LOG("AwbMgr::getInstance().doCapAWB() END");
		FlashMgr::getInstance()->capCheckAndFireFlash_End();

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AEDoCapFlare");
       //Capture Flare compensate
	   //pass WB gain info
	   AWB_OUTPUT_T _a_rAWBOutput;
	   AwbMgr::getInstance().getAWBOutput(_a_rAWBOutput);
	   AeMgr::getInstance().doCapFlare(reinterpret_cast<MVOID *>(rBufInfo.virtAddr),FlashMgr::getInstance()->isFlashOnCapture() );

       MY_LOG("AeMgr::getInstance().doCapFlare() END");

        #if 1
        // F858
        TSF_REF_INFO_T rTSFRef;
        rTSFRef.awb_info.m_i4LV	    = i4SceneLV; //AeMgr::getInstance().getLVvalue();
        rTSFRef.awb_info.m_u4CCT    = AwbMgr::getInstance().getAWBCCT();
        rTSFRef.awb_info.m_RGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        rTSFRef.awb_info.m_GGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        rTSFRef.awb_info.m_BGAIN    = _a_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        rTSFRef.awb_info.m_FLUO_IDX = _a_rAWBOutput.rAWBInfo.i4FluorescentIndex;
        rTSFRef.awb_info.m_DAY_FLUO_IDX = _a_rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;	
        NSIspTuning::LscMgr::getInstance()->updateTSFinput(
                static_cast<NSIspTuning::LscMgr::LSCMGR_TSF_INPUT_SRC>(NSIspTuning::LscMgr::TSF_INPUT_CAP),
                &rTSFRef,
                reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    	
        MY_LOG("lv %d, cct %d, rgain %d, bgain %d, ggain %d, fluo idx %d, day flou idx %d\n", 
                rTSFRef.awb_info.m_i4LV,
                rTSFRef.awb_info.m_u4CCT,
                rTSFRef.awb_info.m_RGAIN,
                rTSFRef.awb_info.m_GGAIN,
                rTSFRef.awb_info.m_BGAIN,
                rTSFRef.awb_info.m_FLUO_IDX,
                rTSFRef.awb_info.m_DAY_FLUO_IDX
                );	
        //MTK_SWIP_PROJECT_END

        MY_LOG("LscMgr::getInstance()->updateTSFinput() END");
        #endif

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgreQAA"); 
        // Enqueue AAO DMA buffer
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrUpAAAddr"); 
        // Update AAO DMA address
        BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

		//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "AFCalBestShot"); 
        // --- best shot select ---
        BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);
        AfMgr::getInstance().calBestShotValue(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
        BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);
        // --- best shot select ---

        MY_LOG("AfMgr::getInstance().calBestShotValue() END");

    }
	//else
	//{
	//	FlashMgr::getInstance()->capCheckAndFireFlash_End();
	//}



	//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrAAUninit"); 
    // AAO DMA / state disable again
    err = BufMgr::getInstance().AAStatEnable(MFALSE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AAStatEnable(MFALSE) fail\n");
        return err;
    }

    err = BufMgr::getInstance().DMAUninit(camdma2type<ECamDMA_AAO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAunInit(ECamDMA_AAO) fail\n");
        return err;
    }

	//CPTLogStr(Event_SShot_createSensorRawImg, CPTFlagSeparator, "BufMgrAFUninit");
    // AFO DMA / state disable again
    err = BufMgr::getInstance().AFStatEnable(MFALSE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AFStatEnable(MFALSE) fail\n");
        return err;
    }

    err = BufMgr::getInstance().DMAUninit(camdma2type<ECamDMA_AFO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAunInit(ECamDMA_AFO) fail\n");
        return err;
    }
	
    AeMgr::getInstance().setStrobeMode(MFALSE);


    MY_LOG("sendIntent(intent2type<eIntent_CaptureEnd>) END");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
	MY_LOG("sendIntent(intent2type<eIntent_VsyncUpdate>)  line=%d", __LINE__);


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_AFUpdate>)
{
	MY_LOG("sendIntent(intent2type<eIntent_AFUpdate>)  line=%d", __LINE__);


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
	MY_LOG("sendIntent(intent2type<eIntent_CameraPreviewEnd>)  line=%d", __LINE__);

    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CameraPreviewStart>)
{
	MY_LOG("sendIntent(intent2type<eIntent_CameraPreviewStart>)  line=%d", __LINE__);
    MRESULT err;

    // Get parameters
    Param_T rParam;
    m_pHal3A->getParams(rParam);

    // AE init
    err = AeMgr::getInstance().cameraPreviewReinit();
    if (FAILED(err)) {
        MY_ERR("AwbMgr::getInstance().cameraPreviewReinit() fail\n");
        return err;
    }
	sm_bHasAEEverBeenStable = MFALSE;

    // AF init

    // AWB init
    err = AwbMgr::getInstance().cameraPreviewReinit(rParam);
    if (FAILED(err)) {
        MY_ERR("AwbMgr::getInstance().cameraPreviewReinit() fail\n");
        return err;
    }

    // AAO DMA / state enable again
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AAO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AAO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AAStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AAStatEnable(MTRUE) fail\n");
        return err;
    }

    AfMgr::getInstance().setAF_IN_HSIZE();
    AfMgr::getInstance().setFlkWinConfig();

    // AFO DMA / state enable again
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AFO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AFO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AFStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AFStatEnable(MTRUE) fail\n");
        return err;
    }

    // Reset frame count to -2
    resetFrameCount();

    IspTuningMgr::getInstance().validatePerFrame(MTRUE);


    FlashMgr::getInstance()->capturePreviewStart();
    FlickerHalBase::getInstance()->cameraPreviewStart();

    LscMgr::getInstance()->notifyPreflash(MFALSE);

    // State transition: eState_Capture --> eState_CameraPreview
    transitState(eState_Capture, eState_CameraPreview);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_AFEnd>)
{
	MY_LOG("sendIntent(intent2type<eIntent_AFEnd>)  line=%d", __LINE__);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_Uninit>)
{
	MY_LOG("sendIntent(intent2type<eIntent_Uninit>)  line=%d", __LINE__);
	FlashMgr::getInstance()->turnOffFlashDevice();

    // AAO DMA buffer uninit
    BufMgr::getInstance().uninit();

    // AE uninit
    AeMgr::getInstance().uninit();

    // AWB uninit
    AwbMgr::getInstance().uninit();

    // AF uninit
    AfMgr::getInstance().uninit();

    // Flash uninit
    FlashMgr::getInstance()->uninit();

    // State transition: eState_Capture --> eState_Uninit
    transitState(eState_Capture, eState_Uninit);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CamcorderPreviewStart: for CTS only
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCapture::
sendIntent(intent2type<eIntent_CamcorderPreviewStart>)
{
    MRESULT err;
    FlashMgr::getInstance()->turnOffFlashDevice();

    MY_LOG("[StateCapture::sendIntent]<eIntent_CamcorderPreviewStart>");

    // AE uninit
    AeMgr::getInstance().uninit();

    // AWB uninit
    AwbMgr::getInstance().uninit();

    // AF uninit
    AfMgr::getInstance().uninit();

    // Flash uninit
    FlashMgr::getInstance()->uninit();

    // Get parameters
    Param_T rParam;
    m_pHal3A->getParams(rParam);
    MINT32 i4SensorDev = m_pHal3A->getSensorDev();

    // AE init
    err = AeMgr::getInstance().camcorderPreviewInit(i4SensorDev, rParam);
    if (FAILED(err)) {
        MY_ERR("AebMgr::getInstance().camcorderPreviewInit() fail\n");
        return err;
    }
	sm_bHasAEEverBeenStable = MFALSE;

    // AF init
    err = AfMgr::getInstance().init();
    if (FAILED(err)) {
        MY_ERR("AfMgr::getInstance().init() fail\n");
        return err;
    }

    AfMgr::getInstance().setAF_IN_HSIZE();
    AfMgr::getInstance().setFlkWinConfig();

    // AWB init
    err = AwbMgr::getInstance().camcorderPreviewInit(i4SensorDev, rParam);
    if (FAILED(err)) {
        MY_ERR("AwbMgr::getInstance().camcorderPreviewInit() fail\n");
        return err;
    }

    // Flash init
    err = FlashMgr::getInstance()->init(i4SensorDev);
    if (FAILED(err)) {
        MY_ERR("FlashMgr::getInstance()->init(i4SensorDev) fail\n");
        return err;
    }

    // AAO DMA / state enable
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AAO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AAO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AAStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AAStatEnable(MTRUE) fail\n");
        return err;
    }

    // AFO DMA / state enable
    err = BufMgr::getInstance().DMAInit(camdma2type<ECamDMA_AFO>());
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().DMAInit(ECamDMA_AFO) fail\n");
        return err;
    }

    err = BufMgr::getInstance().AFStatEnable(MTRUE);
    if (FAILED(err)) {
        MY_ERR("BufMgr::getInstance().AFStatEnable(MTRUE) fail\n");
        return err;
    }

    // Reset frame count to -2
    resetFrameCount();

	FlashMgr::getInstance()->videoPreviewStart();

    // State transition: eState_Capture --> eState_CamcorderPreview
    transitState(eState_Capture, eState_CamcorderPreview);

    return  S_3A_OK;
}



