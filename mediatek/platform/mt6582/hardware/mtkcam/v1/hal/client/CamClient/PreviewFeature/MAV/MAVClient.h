/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/**
 * @file MAVClient.h
 * @brief Header file for MAVClient.cpp
 * @details N/A.
 */
#ifndef _MTK_HAL_CAMCLIENT_MAVCLIENT_H_
#define _MTK_HAL_CAMCLIENT_MAVCLIENT_H_
//
#include <CamUtils.h>
#include <system/camera.h>
#include <mtkcam/drv/imem_drv.h>
#include <pthread.h>
#include <semaphore.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include "mtkcam/common.h"
#include <mtkcam/featureio/3DF_hal_base.h>
#include "inc/IFeatureClient.h"

using namespace android;
using namespace MtkCamUtils;

//
namespace android {
namespace NSCamClient {

/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
 /**
 * @brief Preview Client Handler for MAV.
 */
class MAVClient : public IFeatureClient
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    //
    MAVClient(int ShotNum);
    virtual    ~MAVClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief Init function for MAVClient.
     *
     * @details
     *      1. Create frame buffer.
     *      2. Create working buffer.
     *      3. initial algorithm.
     *      4. reset member parameter.
     *      5. thread create.
     *
     * @note N/A.
     *
     * @param [in] bufwidth Width of buffer.
     * @param [in] bufheight Height of buffer.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual bool      init(int bufwidth,int bufheight);

    /**
     * @brief Un-init function for MAVClient.
     *
     * @details
     *      1. Release memory.
     *      2. Un-init algorithm.
     *
     * @note N/A
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual bool      uninit();

    /**
     * @brief Get source image and trigger algorithm when shot.
     *
     * @note N/A.
     *
     * @param [in] bufadr Source buffer
     * @param [out] mvX MAV result in horizontal direction.
     * @param [out] mvY MAV result in vertical direction.
     * @param [out] dir Output image directory. (Not used in MAV).
     * @param [in] isShot A flag to judge whether to shot or not.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MINT32    mHalCamFeatureProc(MVOID * bufadr, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot);

    /**
     * @brief Stop MAV.
     *
     * @details Stop MAV and merge captured frames.
     *
     * @note N/A.
     *
     * @param [in] cancel A flag to indicate cancel or not.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual bool      stopFeature(int cancel);

    /**
     * @brief Set image call back function pointer.
     *
     * @note N/A.
     *
     * @param [in] data_cb image call back function pointer,
     *
     * @return N/A.
     */
    virtual MVOID     setImgCallback(ImgDataCallback_t data_cb);

    /**
     * @brief Wait until MAV merge done, then use callback function to start compression.
     *
     * @note N/A.
     *
     * @return
     * - NO_ERROR indicates success.
     */
    virtual MINT32    mHalCamFeatureCompress();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MAVClient.Scenario function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    /**
     * @brief Call do motion algorithm.
     *
     * @note N/A.
     *
     * @param [in] bufadr Input buffer address.
     * @param [in] arg1 Algorithm Input parameters.
     * @param [out] shot A flag to indicate ready to shot.
     *
     * @return
     * - NO_ERROR indicates success.
     */
    virtual MINT32    ISShot(MVOID * bufadr, MVOID *arg1, MBOOL &shot);

    /**
     * @brief Config source/destination Motionstream, then trigger HW.
     *
     * @note N/A.
     *
     * @param [in] srcbufadr Source image buffer address.
     * @param [in] ImgWidth image width.
     * @param [in] ImgHeight image height.
     * @param [in] dstbufadr destination image buffer address.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MINT32    CreateMotionSrc(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr);   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MAVClinet function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:    
    /**
     * @brief Wait until add image done, then merge and warp images.
     *
     * @note N/A.
     *
     * @return
     * - NO_ERROR indicates success.
     */
    virtual MINT32    mHalCamFeatureMerge();

    /**
     * @brief Get input image.
     *
     * @note N/A.
     *
     * @return
     * - NO_ERROR indicates success.
     */
    virtual MINT32    mHalCamFeatureAddImg();    

    /**
     * @brief Allocate memory.
     *
     * @note N/A.
     *
     * @param [out] memBuf buffer address.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL     allocMem(IMEM_BUF_INFO &memBuf);

    /**
     * @brief Release memory.
     *
     * @note N/A.
     *
     * @param [in] memBuf buffer address.
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL     deallocMem(IMEM_BUF_INFO &memBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:     
    /**
     * @brief MAV main thread.
     *
     * @details Config MAV thread and loop forever to execute MAV.
     *
     * @note N/A.
     *
     * @param [in] arg Not used.
     *
     * @return NULL.
     */
    static MVOID*     MAVthreadFunc(void *arg); 

    pthread_t         MAVFuncThread;
    sem_t             MAVSemThread;
    sem_t             MAVmergeDone;
    MBOOL             mCancel;
    MBOOL             mStop;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: 
    IMemDrv*          mpIMemDrv;  
    IMEM_BUF_INFO     mpframeBuffer[MAV_PIPE_MAX_IMAGE_NUM];
    IMEM_BUF_INFO     mpMotionBuffer;
    IMEM_BUF_INFO     mpMAVMotionBuffer;
    IMEM_BUF_INFO     mpWarpBuffer;
    IMEM_BUF_INFO     mpMAVWorkingBuf;
    int               mMAVFrameWidth;
    int               mMAVFrameHeight;
    int               mMAVFrameSize;
    ImgDataCallback_t mDataCb;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
private:	        
    int32_t 	        MAVnum;
    hal3DFBase* 	    mpMAVObj;
    int32_t 	        mMAVFrameIdx;
    int32_t			    mMAVaddImgIdx; 
    int32_t 		    mJPGFrameAddr;
    MavPipeResultInfo   mpMAVResult;
    uint8_t  	      	SaveFileName[64];
    mutable Mutex       mLock;
    mutable Mutex 	    mLockUninit;
		  
};
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_PREVIEW_PREVIEWCLIENT_H_

