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

#ifndef _MTK_HAL_CAMCLIENT_MotionTrackCLIENT_H_
#define _MTK_HAL_CAMCLIENT_MotionTrackCLIENT_H_
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
#include <mtkcam/featureio/motiontrack_hal_base.h>
#include "inc/IFeatureClient.h"
#include "mtkcam/hal/aaa_hal_base.h"
#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/featureio/eis_hal_base.h>
using namespace android;
using namespace MtkCamUtils;
using namespace NS3A;
//
namespace android {
namespace NSCamClient {

/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
class MotionTrackClient : public IFeatureClient
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    //
    MotionTrackClient(int ShotNum);
    MotionTrackClient(int ShotNum, int32_t previewFrameRate);
    virtual    ~MotionTrackClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual bool    init(int bufwidth,int bufheight);
    virtual bool    uninit();
    virtual MINT32  mHalCamFeatureProc(MVOID * bufadr, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot);
    virtual bool    stopFeature(int cancel);
    virtual MVOID   setImgCallback(ImgDataCallback_t data_cb);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MotionTrackClient.Scenario function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MINT32  CreateThumbImage(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MotionTrackClinet function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MINT32    mHalCamFeatureCompress();
    virtual MINT32    mHalCamFeatureBlend();
    virtual MBOOL     allocMem(IMEM_BUF_INFO &memBuf);
    virtual MBOOL     deallocMem(IMEM_BUF_INFO &memBuf);
    virtual MVOID     updateEISMethod(MUINT16 method);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:     
    static MVOID*     MotionTrackthreadFunc(void *arg); 
    pthread_t  MotionTrackFuncThread;
    sem_t      MotionTrackAddImgDone;
    sem_t      MotionTrackBlendDone;
    MBOOL      mCancel;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: 
    IMemDrv*          mpIMemDrv;  
    IMEM_BUF_INFO     mpFrameBuffer;
    IMEM_BUF_INFO     mpThumbBuffer;
    IMEM_BUF_INFO     mpMotionTrackWorkingBuf;      
    IMEM_BUF_INFO     mpBlendBuffer;      
	MUINT32           mNumBlendImages;
    int               mMotionTrackFrameWidth;
    int               mMotionTrackFrameHeight;
    int               mMotionTrackFrameSize;
    int               mMotionTrackThumbSize;
    MUINT16           mSensorRawWidth;
    MUINT16           mSensorRawHeight;
    MUINT16           mMotionTrackOutputWidth;
    MUINT16           mMotionTrackOutputHeight;
    ImgDataCallback_t mDataCb;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
private:	           
    int32_t 	        MotionTrackNum;
    halMOTIONTRACKBase* mpMotionTrackObj;
    int32_t 	      	mMotionTrackFrameIdx;
    int32_t			      mMotionTrackaddImgIdx; 
    int32_t 		      mJPGFrameAddr;
    uint8_t  	      	SaveFileName[64];
    int32_t             mPreviewFrameCount;
    SensorHal*          mpSensor;
    EisHalBase*         mpEisHal;
    mutable Mutex     mLock;
	  mutable Mutex 	  mLockUninit;
    MUINT32             mTimestamp;
    MUINT32             mTimelapse;
    MINT32              mMvX;
    MINT32              mMvY;
    MUINT16             mPreviewCropWidth;
    MUINT16             mPreviewCropHeight;
	  
	  
};
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_PREVIEW_PREVIEWCLIENT_H_

