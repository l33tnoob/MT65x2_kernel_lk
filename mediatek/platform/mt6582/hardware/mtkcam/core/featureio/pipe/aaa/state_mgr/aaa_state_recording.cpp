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
#define LOG_TAG "aaa_state_recording"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <aaa_hal.h>
#include "aaa_state.h"
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_awb_param.h>
#include <awb_mgr.h>
#include <buf_mgr.h>
#include <mtkcam/hal/sensor_hal.h>
#include <af_param.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <af_mgr.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_param.h>
#include <ae_mgr.h>
#include <flash_mgr.h>
#include <lsc_mgr.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include "aaa_state_flow_custom.h"

using namespace NS3A;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateRecording
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateRecording::
StateRecording()
    : IState("StateRecording")
{
}


MRESULT StateRecording::exitPreview()
{
	MY_LOG("StateRecording::exitPreview line=%d",__LINE__);
	MRESULT err;

    // AE uninit
    AeMgr::getInstance().uninit();

    // AWB uninit
    AwbMgr::getInstance().uninit();

    // AF uninit
    AfMgr::getInstance().uninit();

    // Flash uninit
    FlashMgr::getInstance()->uninit();

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


    transitState(eState_Recording, eState_Init);
    return  S_3A_OK;
}


MRESULT
StateRecording::
sendIntent(intent2type<eIntent_CamcorderPreviewEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_CamcorderPreviewEnd> line=%d", __LINE__);
	exitPreview();
	FlashMgr::getInstance()->setAFLampOnOff(0);
    return  S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingStart>");

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_RecordingEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_RecordingEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_RecordingEndd>");

    // State transition: eState_Recording --> eState_CamcorderPreview
    transitState(eState_Recording, eState_CamcorderPreview);

    //if(FlashMgr::getInstance()->getFlashMode()==LIB3A_FLASH_MODE_AUTO)
    //if(FlashMgr::getInstance()->getFlashMode()!=LIB3A_FLASH_MODE_FORCE_TORCH)
    //	FlashMgr::getInstance()->setAFLampOnOff(0);
	FlashMgr::getInstance()->videoRecordingEnd();

	FlickerHalBase::getInstance()->recordingEnd();


    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MRESULT err = S_3A_OK;
    BufInfo_T rBufInfo;

    MY_LOG("[StateRecording::sendIntent]<eIntent_VsyncUpdate>");

    // Update frame count
    updateFrameCount();

    // Dequeue AAO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AAO, rBufInfo);

    //MTK_SWP_PROJECT_START
    // F858
	AWB_OUTPUT_T rAWBOutput;
	AwbMgr::getInstance().getAWBOutput(rAWBOutput);
	TSF_REF_INFO_T rTSFRef;
    rTSFRef.awb_info.m_i4LV		= AeMgr::getInstance().getLVvalue(MTRUE);
    rTSFRef.awb_info.m_u4CCT	= AwbMgr::getInstance().getAWBCCT();
	rTSFRef.awb_info.m_RGAIN	= rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
	rTSFRef.awb_info.m_GGAIN	= rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
	rTSFRef.awb_info.m_BGAIN	= rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
	rTSFRef.awb_info.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
	rTSFRef.awb_info.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
    NSIspTuning::LscMgr::getInstance()->updateTSFinput(
            static_cast<NSIspTuning::LscMgr::LSCMGR_TSF_INPUT_SRC>(NSIspTuning::LscMgr::TSF_INPUT_VDO),
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

    //MTK_SWP_PROJECT_END

    // AWB
    MINT32 i4SceneLv = AeMgr::getInstance().getLVvalue(MTRUE);
    CPTLog(Event_Pipe_3A_AWB, CPTFlagStart);    // Profiling Start.
    AaaTimer localTimer("doVideoAWB", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
    AwbMgr::getInstance().doVideoAWB(getFrameCount(), AeMgr::getInstance().IsAEStable(), i4SceneLv, reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    localTimer.printTime();
    CPTLog(Event_Pipe_3A_AWB, CPTFlagEnd);     // Profiling End.

    // AE
    AWB_OUTPUT_T _a_rAWBOutput;
	AwbMgr::getInstance().getAWBOutput(_a_rAWBOutput);
    CPTLog(Event_Pipe_3A_AE, CPTFlagStart);    // Profiling Start.
    if (sm_bHasAEEverBeenStable == MFALSE)
    {
        if (AeMgr::getInstance().IsAEStable()) sm_bHasAEEverBeenStable = MTRUE;
    }
    if (isAELockedDuringCAF())
    {
        if (AfMgr::getInstance().isFocusFinish() || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
        (sm_bHasAEEverBeenStable == MFALSE)) //guarantee AE can doPvAE at beginning, until IsAEStable()=1 
        {      
            AaaTimer localTimer("doPvAE", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
            AeMgr::getInstance().doPvAE(getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr), MTRUE);
            localTimer.printTime();
        }
    }		
    else //always do AE, no matter whether lens are moving or not
    {
        AaaTimer localTimer("doPvAE", m_pHal3A->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
	AeMgr::getInstance().doPvAE(getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo.virtAddr), MTRUE);
        localTimer.printTime();
    }

    CPTLog(Event_Pipe_3A_AE, CPTFlagEnd);     // Profiling End.

    // Enqueue AAO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AAO, rBufInfo);

    // Update AAO DMA base address for next frame
    err = BufMgr::getInstance().updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), BufMgr::getInstance().getNextHwBuf(ECamDMA_AAO));

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFUpdate>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFUpdate>");

    BufInfo_T rBufInfo;

    // Dequeue AFO DMA buffer
    BufMgr::getInstance().dequeueHwBuf(ECamDMA_AFO, rBufInfo);

    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagStart);    // Profiling Start.
    AfMgr::getInstance().doAF(reinterpret_cast<MVOID *>(rBufInfo.virtAddr));
    CPTLog(Event_Pipe_3A_Continue_AF, CPTFlagEnd);     // Profiling End.

    // Enqueue AFO DMA buffer
    BufMgr::getInstance().enqueueHwBuf(ECamDMA_AFO, rBufInfo);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFStart>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFStart>");

    // Init
    if(AeMgr::getInstance().IsDoAEInPreAF() == MTRUE)   {
        MY_LOG("Enter PreAF state");
        transitAFState(eAFState_PreAF);
    }
    else   {
        MY_LOG("Enter AF state");
        AfMgr::getInstance().triggerAF();
        transitAFState(eAFState_AF);
    }

    // State transition: eState_CameraPreview --> eState_AF
    //transitState(eState_CameraPreview, eState_AF);

    transitState(eState_Recording, eState_AF);
    FlashMgr::getInstance()->notifyAfEnter();



    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateRecording::
sendIntent(intent2type<eIntent_AFEnd>)
{
    MY_LOG("[StateRecording::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}

