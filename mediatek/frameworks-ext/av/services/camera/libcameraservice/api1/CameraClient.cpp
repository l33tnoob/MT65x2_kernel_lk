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

/*
**
** Copyright (C) 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "CameraService"
//
#include <binder/MemoryBase.h>
//
#include <camera/CameraParameters.h>
#include <camera/MtkCamera.h>
//
#include "device1/CameraHardwareInterface.h"
#include "api1/CameraClient.h"


namespace android {


/******************************************************************************
 *
 ******************************************************************************/
void
CameraClient::
playRecordingSound()
{
    CameraParameters params(mHardware->getParameters());
    int value = 0;
    value = params.getInt("rec-mute-ogg");
    if (value != 1) {
        mCameraService->playSound(CameraService::SOUND_RECORDING);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
CameraClient::
handleMtkExtNotify(int32_t ext1, int32_t ext2)
{
    int32_t const extMsgType = ext1;
    switch  (extMsgType)
    {
    case MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE:
        handleMtkExtCaptureDone(ext1, ext2);
        break;
    //
    case MTK_CAMERA_MSG_EXT_NOTIFY_SHUTTER:
        handleMtkExtShutter(ext1, ext2);
        break;
    //
    case MTK_CAMERA_MSG_EXT_NOTIFY_BURST_SHUTTER:
        handleMtkExtBurstShutter(ext1, ext2);
        break;
    case MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_SHUTTER:
        handleMtkExtContinuousShutter(ext1, ext2);
        break;
    case MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END:
        handleMtkExtContinuousEnd(ext1, ext2);
        break;
    //
    default:
        handleGenericNotify(MTK_CAMERA_MSG_EXT_NOTIFY, ext1, ext2);
        break;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
CameraClient::
handleMtkExtData(const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    MtkCamMsgExtDataHelper MtkExtDataHelper;

    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ALOGE("[handleMtkExtData] MtkCamMsgExtDataHelper::init fail - dataPtr(%p), this(%p)", dataPtr.get(), this);
        return;
    }

    void*   const pvExtParam   = MtkExtDataHelper.getExtParamBase();
    size_t  const ExtParamSize = MtkExtDataHelper.getExtParamSize();
    switch  (MtkExtDataHelper.getExtMsgType())
    {
    case MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE:
        handleMtkExtDataCompressedImage(dataPtr, metadata);
        break;
    //
    case MTK_CAMERA_MSG_EXT_DATA_BURST_SHOT:
        handleMtkExtDataBurstShot(dataPtr, metadata);
        break;
    //
    case MTK_CAMERA_MSG_EXT_DATA_CONTINUOUS_SHOT:
        handleMtkExtDataContinuousShot(dataPtr, metadata);
        break;
    //
    default:
        handleGenericData(MTK_CAMERA_MSG_EXT_DATA, dataPtr, metadata);
        break;
    }
    MtkExtDataHelper.uninit();
}


/******************************************************************************
 *  Shutter Callback (not disable CAMERA_MSG_SHUTTER)
 *      ext2: 1: CameraService will play shutter sound.
 ******************************************************************************/
void
CameraClient::
handleMtkExtShutter(int32_t ext1, int32_t ext2)
{
    ALOGD("[%s] (ext2, mPlayShutterSound)=(%d, %d) \r\n", __FUNCTION__, ext2, mPlayShutterSound);

    if  ( 1 == ext2 ) {
        if (mPlayShutterSound) {
            mCameraService->playSound(CameraService::SOUND_SHUTTER);
        }
    }

    sp<ICameraClient> c = mRemoteCallback;
    if (c != 0) {
//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
        mLock.unlock();
#endif
//!--
        c->notifyCallback(CAMERA_MSG_SHUTTER, 0, 0);
        if (!lockIfMessageWanted(CAMERA_MSG_SHUTTER)) return;
    }
//    disableMsgType(CAMERA_MSG_SHUTTER);

//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
    mLock.unlock();
#endif
//!--
}


/******************************************************************************
 *  Burst Shutter Callback Handler
 *       ext2: count-down shutter number; 0: the last one shutter.
 ******************************************************************************/
void
CameraClient::
handleMtkExtBurstShutter(int32_t ext1, int32_t ext2)
{
    handleShutter();
    if  (0 < ext2) {
        //  not the last one burst shutter.
        enableMsgType(CAMERA_MSG_SHUTTER);
        ALOGD("[handleMtkExtBurstShutter] count-down burst shutter number:%d - enableMsgType(CAMERA_MSG_SHUTTER) \r\n", ext2);
    }
    else {
        ALOGD("[handleMtkExtBurstShutter] the last one burst shutter \r\n");
    }
}


/******************************************************************************
 *  Burst Shot (EV Shot)
 *      int[0]: the total shut count.
 *      int[1]: count-down shut number; 0: the last one shut.
 ******************************************************************************/
void
CameraClient::
handleMtkExtDataBurstShot(const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    MtkCamMsgExtDataHelper MtkExtDataHelper;
    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ALOGE("[%s] MtkCamMsgExtDataHelper::init fail - dataPtr(%p), this(%p) \r\n", __FUNCTION__, dataPtr.get(), this);
        return;
    }
    //
    uint_t const*const pExtParam = (uint_t const*)MtkExtDataHelper.getExtParamBase();
    uint_t const uTotalShutCount = pExtParam[0];
    uint_t const uCountdownIndex = pExtParam[1];
    //
    size_t const    imageSize   = MtkExtDataHelper.getExtParamSize()    - sizeof(uint_t) * 2;
    ssize_t const   imageOffset = MtkExtDataHelper.getExtParamOffset()  + sizeof(uint_t) * 2;
    sp<MemoryBase> image = new MemoryBase(MtkExtDataHelper.getHeap(), imageOffset, imageSize);
    //
    MtkExtDataHelper.uninit();
    //
    //
    if  (0 < uCountdownIndex) {
        //  not the last one burst shut.
        ALOGD("[%s] count-down burst shut number:%d/%d - (size, offset)=(%d, %d) \r\n", __FUNCTION__, uCountdownIndex, uTotalShutCount, imageSize, imageOffset);
    }
    else {
        disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
        ALOGD("[%s] the last one burst shut - (size, offset)=(%d, %d) \r\n", __FUNCTION__, imageSize, imageOffset);
    }
    //
    if (image == 0) {
        ALOGE("[%s] fail to new MemoryBase \r\n", __FUNCTION__);
        return;
    }
    //
    sp<ICameraClient> c = mRemoteCallback;
//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
    mLock.unlock();
#endif
//!--
    if (c != 0) {
        c->dataCallback(CAMERA_MSG_COMPRESSED_IMAGE, image, NULL);
    }
}


/******************************************************************************
*   Continuous Shutter Callback Handler
*       ext2: current continuous shutter number.
*******************************************************************************/
void
CameraClient::
handleMtkExtContinuousShutter(int32_t ext1, int32_t ext2)
{
    //if (mPlayShutterSound) {
    //    mCameraService->playSound(CameraService::SOUND_SHUTTER);
    //}

    sp<ICameraClient> c = mRemoteCallback;
    if (c != 0) {
//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
        mLock.unlock();
#endif
//!--
        c->notifyCallback(CAMERA_MSG_SHUTTER, 0, 0);
        if (!lockIfMessageWanted(CAMERA_MSG_SHUTTER)) return;
    }
    //disableMsgType(CAMERA_MSG_SHUTTER);

//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
    mLock.unlock();
#endif
//!--

    //enableMsgType(CAMERA_MSG_SHUTTER);
    ALOGD("[handleMtkExtContinuousShutter] current continuous shutter number:%d \n", ext2);
}


/******************************************************************************
 *  Continuous Shot
 *      int[0]: current continuous shut number.
 ******************************************************************************/
void
CameraClient::
handleMtkExtDataContinuousShot(const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    MtkCamMsgExtDataHelper MtkExtDataHelper;
    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ALOGE("[%s] MtkCamMsgExtDataHelper::init fail - dataPtr(%p), this(%p) \r\n", __FUNCTION__, dataPtr.get(), this);
        return;
    }
    //
    uint_t const*const pExtParam = (uint_t const*)MtkExtDataHelper.getExtParamBase();
    uint_t const uCurShutCount = pExtParam[0];
    //
    size_t const    imageSize   = MtkExtDataHelper.getExtParamSize()    - sizeof(uint_t) * 1;
    ssize_t const   imageOffset = MtkExtDataHelper.getExtParamOffset()  + sizeof(uint_t) * 1;
    sp<MemoryBase> image = new MemoryBase(MtkExtDataHelper.getHeap(), imageOffset, imageSize);
    //
    MtkExtDataHelper.uninit();

    ALOGD("[%s] current continuous shut number:%d - (size, offset)=(%d, %d) \r\n", __FUNCTION__,  uCurShutCount, imageSize, imageOffset);

    //
    if (image == 0) {
        ALOGE("[%s] fail to new MemoryBase \r\n", __FUNCTION__);
        return;
    }
    //
    sp<ICameraClient> c = mRemoteCallback;
//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
    mLock.unlock();
#endif
//!--
    if (c != 0) {
        c->dataCallback(CAMERA_MSG_COMPRESSED_IMAGE, image, NULL);
    }
}


/******************************************************************************
 *  Continuous EndCallback Handler
 ******************************************************************************/
void
CameraClient::
handleMtkExtContinuousEnd(int32_t ext1, int32_t ext2)
{
    disableMsgType(CAMERA_MSG_SHUTTER);
    disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
    handleGenericNotify(MTK_CAMERA_MSG_EXT_NOTIFY, ext1, ext2);
    ALOGD("[handleMtkExtContinuousEnd] total continuous shut number is %d \n", ext2);
}


/******************************************************************************
 *  Capture done (disable CAMERA_MSG_SHUTTER / CAMERA_MSG_COMPRESSED_IMAGE)
 ******************************************************************************/
void
CameraClient::
handleMtkExtCaptureDone(int32_t ext1, int32_t ext2)
{
    ALOGD("[%s] disable CAMERA_MSG_SHUTTER / CAMERA_MSG_COMPRESSED_IMAGE \r\n", __FUNCTION__);
    disableMsgType(CAMERA_MSG_SHUTTER);
    disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
}


/******************************************************************************
 *  Compressed Image (not disable CAMERA_MSG_COMPRESSED_IMAGE)
 *      int[0]: current shut index; 0: the first one shut.
 ******************************************************************************/
void
CameraClient::
handleMtkExtDataCompressedImage(const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    MtkCamMsgExtDataHelper MtkExtDataHelper;
    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ALOGE("[%s] MtkCamMsgExtDataHelper::init fail - dataPtr(%p), this(%p) \r\n", __FUNCTION__, dataPtr.get(), this);
        return;
    }
    //
    uint_t const*const pExtParam = (uint_t const*)MtkExtDataHelper.getExtParamBase();
    uint_t const      uShutIndex = pExtParam[0];
    //
    size_t const    imageSize   = MtkExtDataHelper.getExtParamSize()    - sizeof(uint_t) * 1;
    ssize_t const   imageOffset = MtkExtDataHelper.getExtParamOffset()  + sizeof(uint_t) * 1;
    sp<MemoryBase> image = new MemoryBase(MtkExtDataHelper.getHeap(), imageOffset, imageSize);
    //
    MtkExtDataHelper.uninit();

    ALOGD("[%s] current shut index:%d - (size, offset)=(%d, %d) \r\n", __FUNCTION__, uShutIndex, imageSize, imageOffset);
    //
    if (image == 0) {
        ALOGE("[%s] fail to new MemoryBase \r\n", __FUNCTION__);
        return;
    }
    //
    sp<ICameraClient> c = mRemoteCallback;
//!++
#if 1   // defined(MTK_CAMERA_BSP_SUPPORT)
#else
    mLock.unlock();
#endif
//!--

    if (c != 0) {
        c->dataCallback(CAMERA_MSG_COMPRESSED_IMAGE, image, NULL);
    }
}


};
