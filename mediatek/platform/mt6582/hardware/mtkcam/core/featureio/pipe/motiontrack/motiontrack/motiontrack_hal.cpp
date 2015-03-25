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
#define LOG_TAG "mHalMotionTrack"

#include <mtkcam/Log.h>
#include "AppMfbmm.h"
#include "camera_custom_motiontrack.h"
#include "motiontrack_hal.h"
#include <mtkcam/featureio/motiontrack_hal_base.h>

/*******************************************************************************
*
********************************************************************************/
#define MHAL_LOG(fmt, arg...)    CAM_LOGD(fmt,##arg) 

static halMOTIONTRACKBase *pHalMOTIONTRACK = NULL;
/*******************************************************************************
*
********************************************************************************/
halMOTIONTRACKBase*
halMOTIONTRACK::
getInstance()
{
    MHAL_LOG("[halMOTIONTRACK] getInstance \n");
    if (pHalMOTIONTRACK == NULL) {
        pHalMOTIONTRACK = new halMOTIONTRACK();
    }
    return pHalMOTIONTRACK;
}

/*******************************************************************************
*
********************************************************************************/
void   
halMOTIONTRACK::
destroyInstance() 
{
    if (pHalMOTIONTRACK) {
        delete pHalMOTIONTRACK;
    }
    pHalMOTIONTRACK = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halMOTIONTRACK::halMOTIONTRACK()
{
    mpMTKMotionTrackObj = MTKMfbmm::createInstance(DRV_MFBMM_OBJ_SW);
}

/*******************************************************************************
*                                            
********************************************************************************/
halMOTIONTRACK::~halMOTIONTRACK()
{    
    mpMTKMotionTrackObj->destroyInstance();
    mpMTKMotionTrackObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMOTIONTRACK::mHalMotionTrackInit(MTKPipeMotionTrackEnvInfo MotionTrackEnvInfo)
{
    MFBMM_INIT_PARAM_STRUCT MfbmmInitInfo;
    MHAL_LOG("[mHalMotionTrackInit] \n");
    
    if (!mpMTKMotionTrackObj) {
        MHAL_LOG("[mHalMotionTrackInit] Not instantiated yet? \n");
        return false;
    }    
    
    MfbmmInitInfo.mode = MFBMM_USEMODE_AUTO;
    MfbmmInitInfo.img_width = MotionTrackEnvInfo.SrcImgWidth;
    MfbmmInitInfo.img_height = MotionTrackEnvInfo.SrcImgHeight;
    MfbmmInitInfo.thread_num = MOTIONTRACK_DEFAULT_THREAD_NUM;
    MfbmmInitInfo.tuning_param.mode = MOTIONTRACK_DEFAULT_MODE;
    MfbmmInitInfo.tuning_param.maxMoveRange = MOTIONTRACK_DEFAULT_MAX_MOVE_RANGE;

    MHAL_LOG("[mHalMotionTrackInit] Width %d, Height %d\n", MfbmmInitInfo.img_width, MfbmmInitInfo.img_height);
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmInit((void*) &MfbmmInitInfo, NULL)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackInit] MfbmmInit failed (%d)\n", ret);
        return false;
    }
      
    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halMOTIONTRACK::mHalMotionTrackUninit()
{
    MHAL_LOG("[mHalMotionTrackUninit] \n");
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmReset()) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackUninit] MfbmmReset failed (%d)\n", ret);
        return false;
    }

    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halMOTIONTRACK::mHalMotionTrackGetWorkSize(MTKPipeMotionTrackWorkBufInfo *MotionTrackWorkBufInfo)
{    
    MHAL_LOG("[mHalMotionTrackGetWorkSize] \n");
    MFBMM_GET_PROC_INFO_STRUCT MfbmmProcInfo;
    
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmFeatureCtrl(MFBMM_FTCTRL_GET_PROC_INFO,
                                                     NULL,
                                                     (void*) &MfbmmProcInfo)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackGetWorkSize] MfbmmFeatureCtrl MFBMM_FTCTRL_GET_PROC_INFO failed (%d)\n", ret);
        return false;
    }

    MotionTrackWorkBufInfo->WorkBufSize = MfbmmProcInfo.workbuf_size;
    MHAL_LOG("[mHalMotionTrackGetWorkSize] Request buffer of %d bytes\n", MotionTrackWorkBufInfo->WorkBufSize);
    return true;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halMOTIONTRACK::mHalMotionTrackSetWorkBuf(MTKPipeMotionTrackWorkBufInfo MotionTrackWorkBufInfo)
{    
    MHAL_LOG("[mHalMotionTrackSetWorkBuf] \n");
    MFBMM_SET_WORKBUF_INFO_STRUCT MfbmmWorkBufInfo;
    
    MfbmmWorkBufInfo.workbuf_addr = MotionTrackWorkBufInfo.WorkBufAddr;
    MfbmmWorkBufInfo.workbuf_size = MotionTrackWorkBufInfo.WorkBufSize;
    MHAL_LOG("[mHalMotionTrackSetWorkBuf] Buffer addr %X, size %d\n", MfbmmWorkBufInfo.workbuf_addr, MfbmmWorkBufInfo.workbuf_size);
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmFeatureCtrl(MFBMM_FTCTRL_SET_WORKBUF_INFO,
                                                     (void*) &MfbmmWorkBufInfo,
                                                     NULL)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackSetWorkBuf] MfbmmFeatureCtrl MFBMM_FTCTRL_SET_WORKBUF_INFO failed (%d)\n", ret);
        return false;
    }

    return true;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halMOTIONTRACK::mHalMotionTrackAddImage(MTKPipeMotionTrackAddImageInfo MotionTrackAddImageInfo)
{    
    MHAL_LOG("[mHalMotionTrackAddImage] \n");
    MFBMM_ADD_IMAGE_STRUCT MfbmmAddImgInfo;
    
    MfbmmAddImgInfo.img_index = MotionTrackAddImageInfo.ImageIndex;
    MfbmmAddImgInfo.thbImageAddr = MotionTrackAddImageInfo.ThumbImageAddr;
    MfbmmAddImgInfo.width = MotionTrackAddImageInfo.ThumbImageWidth;
    MfbmmAddImgInfo.height = MotionTrackAddImageInfo.ThumbImageHeight;
    MfbmmAddImgInfo.stride_y = MotionTrackAddImageInfo.ThumbImageStrideY;
    MfbmmAddImgInfo.stride_uv = MotionTrackAddImageInfo.ThumbImageStrideUV;
    MfbmmAddImgInfo.mvx = MotionTrackAddImageInfo.MvX;
    MfbmmAddImgInfo.mvy = MotionTrackAddImageInfo.MvY;
    MHAL_LOG("[mHalMotionTrackAddImage] index %d, mvx %d, mvy %d\n", MfbmmAddImgInfo.img_index, MfbmmAddImgInfo.mvx, MfbmmAddImgInfo.mvy);
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmFeatureCtrl(MFBMM_FTCTRL_ADD_IMAGE,
                                                     (void*) &MfbmmAddImgInfo,
                                                     NULL)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackAddImage] MfbmmFeatureCtrl MFBMM_FTCTRL_ADD_IMAGE failed (%d)\n", ret);
        return false;
    }

    return true;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMOTIONTRACK::mHalMotionTrackGetIntermediateDataSize(MTKPipeMotionTrackIntermediateData *MotionTrackIntermediateData)
{
    MotionTrackIntermediateData->DataSize = sizeof(MFBMM_INTERMEDIATE_STRUCT);
    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMOTIONTRACK::mHalMotionTrackGetIntermediateData(MTKPipeMotionTrackIntermediateData MotionTrackIntermediateData)
{
    MHAL_LOG("[mHalMotionTrackGetIntermediateData] \n");

    if (MotionTrackIntermediateData.DataSize < sizeof(MFBMM_INTERMEDIATE_STRUCT))
    {
        MHAL_LOG("[mHalMotionTrackGetIntermediateData] buffer too small\n");
        return false;
    }
    
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmFeatureCtrl(MFBMM_FTCTRL_GET_INTERMEDIATE,
                                                     NULL,
                                                     (void*) MotionTrackIntermediateData.DataAddr)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackGetIntermediateData] MfbmmMain MFBMM_FTCTRL_GET_INTERMEDIATE failed (%d)\n", ret);
        return false;
    }

    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMOTIONTRACK::mHalMotionTrackSelectImage(MTKPipeMotionTrackSelectImageInfo *MotionTrackSelectImageInfo)
{
    MHAL_LOG("[mHalMotionTrackSelectImage] \n");
    
    /* PROC1 */
    MFBMM_PROC1_RESULT_STRUCT MfbmmSelectImgInfo;
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmMain(MFBMM_PROC1,
                                              NULL,
                                              (void*) &MfbmmSelectImgInfo)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackSelectImage] MfbmmMain MFBMM_PROC1 failed (%d)\n", ret);
        return false;
    }
    
    MHAL_LOG("[mHalMotionTrackSelectImage] Selected %d candicates: \n", MfbmmSelectImgInfo.can_num);
    MotionTrackSelectImageInfo->NumCandidateImages = MfbmmSelectImgInfo.can_num;
    for (int i = 0; i < MfbmmSelectImgInfo.can_num; i++)
    {
        MHAL_LOG("[mHalMotionTrackSelectImage]     index %d\n", MfbmmSelectImgInfo.can_img_idx[i]);
        MotionTrackSelectImageInfo->CandidateImageIndex[i] = MfbmmSelectImgInfo.can_img_idx[i];
    }
    return true;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halMOTIONTRACK::mHalMotionTrackBlendImage(MTKPipeMotionTrackBlendImageInfo MotionTrackBlendImageInfo, MTKPipeMotionTrackResultImageInfo *MotionTrackResultImageInfo)
{
    MHAL_LOG("[mHalMotionTrackBlendImage] \n");

    /* PROC2 */
    MFBMM_PROC2_INFO_STRUCT MfbmmBlendImgInfo;
    MfbmmBlendImgInfo.bld_num = MotionTrackBlendImageInfo.NumBlendImages;
    for (int i = 0; i < MfbmmBlendImgInfo.bld_num; i++)
    {
        MfbmmBlendImgInfo.img_idx[i] = MotionTrackBlendImageInfo.BlendImageIndex[i];
        MfbmmBlendImgInfo.srcImgYUV420[i] = MotionTrackBlendImageInfo.SrcImageAddr[i];
    }
    MHAL_LOG("[mHalMotionTrackBlendImage] Blend %d images: \n", MfbmmBlendImgInfo.bld_num);
    for (int i = 0; i < MfbmmBlendImgInfo.bld_num; i++)
    {
        MHAL_LOG("[mHalMotionTrackBlendImage]     index %d, addr %X\n", MfbmmBlendImgInfo.img_idx[i], MfbmmBlendImgInfo.srcImgYUV420[i]);
    }
    MRESULT ret;
    if ((ret = mpMTKMotionTrackObj->MfbmmMain(MFBMM_PROC2,
                                              (void*) &MfbmmBlendImgInfo,
                                              NULL)) != S_MFBMM_OK)
    {
        MHAL_LOG("[mHalMotionTrackBlendImage] MfbmmMain MFBMM_PROC2 failed (%d)\n", ret);
        return false;
    }

    /* PROC3 */
    MFBMM_PROC3_INFO_STRUCT MfbmmResultImgBuf;
    MFBMM_PROC3_RESULT_STRUCT MfbmmResultImgInfo;
    for (int i = 0; i < MotionTrackBlendImageInfo.NumBlendImages; i++)
    {
        MfbmmResultImgBuf.outImgYUV420 = MotionTrackBlendImageInfo.ResultImageAddr[i];
        MHAL_LOG("[mHalMotionTrackBlendImage] PROC3 loop %d\n", i);
        if ((ret = mpMTKMotionTrackObj->MfbmmMain(MFBMM_PROC3,
                                                  (void*) &MfbmmResultImgBuf,
                                                  (void*) &MfbmmResultImgInfo)) != S_MFBMM_OK)
        {
            MHAL_LOG("[mHalMotionTrackBlendImage] MfbmmMain MFBMM_PROC3 failed (%d)\n", ret);
            return false;
        }
    }
    MotionTrackResultImageInfo->OutputImgWidth = MfbmmResultImgInfo.out_img_width;
    MotionTrackResultImageInfo->OutputImgHeight = MfbmmResultImgInfo.out_img_height;

    return true;
}
