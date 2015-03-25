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

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>
#else
#include <utils/RWLock.h>
#endif

#include <utils/KeyedVector.h>
#include <mtkcam/Log.h>
#include <mtkcam/v1/camutils/CamInfo.h>


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[CamDevMetaInfoMap::%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


namespace android {
namespace MtkCamUtils {
namespace DevMetaInfo {
/******************************************************************************
*
*******************************************************************************/
namespace
{
    struct CamDevMetaInfo
    {
    public:     ////        fields.
        camera_info         m_cameraInfo;
        int32_t             m_i4DevSetupOrientation;    //  Device Setup Orientation.
        int32_t             m_i4DevId;                  //  EDeviceId
        int32_t             m_i4HalSensorDev;           //  halSensorDev_e in Sensor Hal

    public:     ////        operations.
                            CamDevMetaInfo()
                                : m_cameraInfo()
                                , m_i4DevSetupOrientation(0)
                                , m_i4DevId(eDevId_Unknown)
                                , m_i4HalSensorDev(0)
                            {
                                ::memset(&m_cameraInfo, 0, sizeof(m_cameraInfo));
                            }
    };

    DefaultKeyedVector<int32_t, CamDevMetaInfo> gvCamDevMetaInfo;
    RWLock                                      gRWLock;
};


/******************************************************************************
 *  Clears the database.
 *  
 *  return
 *      N/A
 *
 ******************************************************************************/
void
clear()
{
    RWLock::AutoWLock _l(gRWLock);
    gvCamDevMetaInfo.clear();
    MY_LOGD("pid/tid=%d/%d", ::getpid(), ::gettid());
}


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
)
{
    CamDevMetaInfo info;
    info.m_cameraInfo               = rCameraInfo;
    info.m_i4DevSetupOrientation    = i4DevSetupOrientation;
    info.m_i4DevId                  = i4DevId;
    info.m_i4HalSensorDev           = i4HalSensorDev;
    //
    {
    RWLock::AutoWLock _l(gRWLock);
    gvCamDevMetaInfo.add(i4OpenId, info);
    }
    MY_LOGD("pid/tid=%d/%d size:%d, OpenId:%d", ::getpid(), ::gettid(), gvCamDevMetaInfo.size(), i4OpenId);
}


/******************************************************************************
 *  Query the number of device.
 *
 *  return
 *      Returns the number of device.
 *
 ******************************************************************************/
int32_t
queryNumberOfDevice()
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.size();
}


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
queryCameraInfo(int32_t const i4OpenId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.valueFor(i4OpenId).m_cameraInfo;
}


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
queryDeviceWantedOrientation(int32_t const i4OpenId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.valueFor(i4OpenId).m_cameraInfo.orientation;
}


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
queryDeviceSetupOrientation(int32_t const i4OpenId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.valueFor(i4OpenId).m_i4DevSetupOrientation;
}


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
queryDeviceId(int32_t const i4OpenId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.valueFor(i4OpenId).m_i4DevId;
}


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
queryHalSensorDev(int32_t const i4OpenId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gvCamDevMetaInfo.valueFor(i4OpenId).m_i4HalSensorDev;
}


};  // namespace DevMetaInfo
};  // namespace MtkCamUtils
};  // namespace android

