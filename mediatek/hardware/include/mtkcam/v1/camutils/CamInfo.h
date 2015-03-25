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

#ifndef _MTK_HAL_INC_COMMON_CAMUTILS_CAMINFO_H_
#define _MTK_HAL_INC_COMMON_CAMUTILS_CAMINFO_H_


/*******************************************************************************
*
*******************************************************************************/
#include <hardware/camera.h>
#include <utils/RefBase.h>


/*******************************************************************************
*   Message-Callback Info
*******************************************************************************/
struct CamMsgCbInfo : public android::LightRefBase<CamMsgCbInfo>
{
    int32_t                         mMsgEnabled;
    void*                           mCbCookie;
    camera_notify_callback          mNotifyCb;
    camera_data_callback            mDataCb;
    camera_data_timestamp_callback  mDataCbTimestamp;
    camera_request_memory           mRequestMemory;

                                    CamMsgCbInfo(
                                        int32_t                         msgEnabled = 0, 
                                        void*                           cbCookie = 0, 
                                        camera_notify_callback          notify_cb = 0,
                                        camera_data_callback            data_cb = 0, 
                                        camera_data_timestamp_callback  data_cb_timestamp= 0,
                                        camera_request_memory           get_memory = 0
                                    )
                                        : mMsgEnabled(msgEnabled)
                                        , mCbCookie(cbCookie)
                                        , mNotifyCb(notify_cb)
                                        , mDataCb(data_cb)
                                        , mDataCbTimestamp(data_cb_timestamp)
                                        , mRequestMemory(get_memory)
                                    {
                                    }

    CamMsgCbInfo&                   operator=(CamMsgCbInfo const& rhs)
                                    {
                                        mMsgEnabled = rhs.mMsgEnabled;
                                        mCbCookie = rhs.mCbCookie;
                                        mNotifyCb = rhs.mNotifyCb;
                                        mDataCb = rhs.mDataCb;
                                        mDataCbTimestamp = rhs.mDataCbTimestamp;
                                        mRequestMemory = rhs.mRequestMemory;
                                        return  (*this);
                                    }
};


/*******************************************************************************
*   Device ID
*******************************************************************************/
enum EDeviceId
{
    eDevId_ImgSensor,       //  image sensor
    eDevId_AtvSensor,       //  atv sensor
    eDevId_Unknown = 0xFFFFFFFF
};


/******************************************************************************
*
*******************************************************************************/
namespace android {
namespace MtkCamUtils {
namespace DevMetaInfo {


/******************************************************************************
 *  Clears the database.
 *  
 *  return
 *      N/A
 *
 ******************************************************************************/
void
clear();


/******************************************************************************
 *  add a device meta information.
 *
 *  i4OpenId
 *      [i] Open id
 *  rCameraInfo
 *      [i] camera info
 *  i4DevSetupOrientation
 *      [i] Device setup orientation
 *  i4DevId
 *      [i] Device id (EDeviceId)
 *  i4HalSensorDev
 *      [i] Sensor hal device id (halSensorDev_e)
 *  
 *  return
 *      N/A
 *
 ******************************************************************************/
void
add(
    int32_t const           i4OpenId, 
    camera_info const&      rCameraInfo, 
    int32_t const           i4DevSetupOrientation, 
    int32_t const           i4DevId, 
    int32_t const           i4HalSensorDev
);


/******************************************************************************
 *  Query the number of device.
 *
 *  return
 *      Returns the number of device.
 *
 ******************************************************************************/
int32_t
queryNumberOfDevice();


/******************************************************************************
 *  Given an open id, query the camera_info
 *
 *  i4OpenId
 *      [i] Open id
 *
 *  return
 *      Returns a value of type camera_info if found.
 *      Otherwise returns invalid value.
 *
 ******************************************************************************/
camera_info const&
queryCameraInfo(int32_t const i4OpenId);


/******************************************************************************
 *  Given an open id, query the device wanted orientation.
 *
 *  i4OpenId
 *      [i] Open id
 *
 *  return
 *      Returns a device wanted orientation if found.
 *      Otherwise returns invalid value.
 *
 ******************************************************************************/
int32_t
queryDeviceWantedOrientation(int32_t const i4OpenId);


/******************************************************************************
 *  Given an open id, query the device setup orientation.
 *
 *  i4OpenId
 *      [i] Open id
 *
 *  return
 *      Returns a device setup orientation if found.
 *      Otherwise returns invalid value.
 *
 ******************************************************************************/
int32_t
queryDeviceSetupOrientation(int32_t const i4OpenId);


/******************************************************************************
 *  Given an open id, query the device id
 *
 *  i4OpenId
 *      [i] Open id
 *
 *  return
 *      Returns a value of type EDeviceId if found.
 *      Otherwise returns invalid value.
 *
 ******************************************************************************/
int32_t
queryDeviceId(int32_t const i4OpenId);


/******************************************************************************
 *  Given an open id, query the sensor hal device id defined in sensor hal
 *
 *  i4OpenId
 *      [i] Open id
 *
 *  return
 *      Returns a value of type halSensorCmd_e if found.
 *      Otherwise returns invalid value.
 *
 ******************************************************************************/
int32_t
queryHalSensorDev(int32_t const i4OpenId);


};  // namespace DevMetaInfo
};  // namespace MtkCamUtils
};  // namespace android


/******************************************************************************
*
********************************************************************************/
#endif  //_MTK_HAL_INC_COMMON_CAMUTILS_CAMINFO_H_

