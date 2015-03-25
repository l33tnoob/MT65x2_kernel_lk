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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include <mtkcam/device/Cam1Device.h>
//
using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


////////////////////////////////////////////////////////////////////////////////
//  Implementation of hw_device_t
////////////////////////////////////////////////////////////////////////////////
static
int
camera_close_device(hw_device_t* device)
{
    if  ( ! device )
    {
        return  -EINVAL;
    }
    //
    return  Cam1Device::getDevice(device)->closeDevice();
}


static
hw_device_t const
gHwDevice = 
{
    /** tag must be initialized to HARDWARE_DEVICE_TAG */
    tag:        HARDWARE_DEVICE_TAG, 
    /** version number for hw_device_t */
    version:    CAMERA_DEVICE_API_VERSION_1_0, 
    /** reference to the module this device belongs to */
    module:     NULL, 
    /** padding reserved for future use */
    reserved:   {0}, 
    /** Close this device */
    close:      camera_close_device, 
};


////////////////////////////////////////////////////////////////////////////////
//  Implementation of camera_device_ops
////////////////////////////////////////////////////////////////////////////////
static int camera_set_preview_window(
    struct camera_device * device,
    struct preview_stream_ops *window
)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->setPreviewWindow(window);
    }
    //
    return  err;
}


static void camera_set_callbacks(
    struct camera_device * device,
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    camera_request_memory get_memory,
    void *user
)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->setCallbacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);
    }
}


static void camera_enable_msg_type(struct camera_device * device, int32_t msg_type)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->enableMsgType(msg_type);
    }
}


static void camera_disable_msg_type(struct camera_device * device, int32_t msg_type)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->disableMsgType(msg_type);
    }
}


static int camera_msg_type_enabled(struct camera_device * device, int32_t msg_type)
{
    int ret = 0;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        ret = pDev->msgTypeEnabled(msg_type);
    }
    //
    return  ret;
}


static int camera_start_preview(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->startPreview();
    }
    //
    return  err;
}


static void camera_stop_preview(struct camera_device * device)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->stopPreview();
    }
}


static int camera_preview_enabled(struct camera_device * device)
{
    int ret = 0;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        ret = pDev->previewEnabled();
    }
    //
    return  ret;
}


static int camera_store_meta_data_in_buffers(struct camera_device * device, int enable)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->storeMetaDataInBuffers(enable);
    }
    //
    return  err;
}


static int camera_start_recording(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->startRecording();
    }
    //
    return  err;
}


static void camera_stop_recording(struct camera_device * device)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->stopRecording();
    }
}


static int camera_recording_enabled(struct camera_device * device)
{
    int ret = 0;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        ret = pDev->recordingEnabled();
    }
    //
    return  ret;
}


static void camera_release_recording_frame(
    struct camera_device * device,
    const void *opaque
)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->releaseRecordingFrame(opaque);
    }
}


static int camera_auto_focus(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->autoFocus();
    }
    //
    return  err;
}


static int camera_cancel_auto_focus(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->cancelAutoFocus();
    }
    //
    return  err;
}


static int camera_take_picture(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->takePicture();
    }
    //
    return  err;
}


static int camera_cancel_picture(struct camera_device * device)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->cancelPicture();
    }
    //
    return  err;
}


static int camera_set_parameters(struct camera_device * device, const char *params)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->setParameters(params);
    }
    //
    return  err;
}


static char* camera_get_parameters(struct camera_device * device)
{
    char* param = NULL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        param = pDev->getParameters();
    }
    //
    return param;
}


static void camera_put_parameters(struct camera_device *device, char *parms)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->putParameters(parms);
    }
}


static int camera_send_command(
    struct camera_device * device,
    int32_t cmd, 
    int32_t arg1, 
    int32_t arg2
)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->sendCommand(cmd, arg1, arg2);
    }
    //
    return  err;
}


static void camera_release(struct camera_device * device)
{
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        pDev->uninitialize();
    }
}


static int camera_dump(struct camera_device * device, int fd)
{
    int err = -EINVAL;
    //
    Cam1Device*const pDev = Cam1Device::getDevice(device);
    if  ( pDev )
    {
        err = pDev->dump(fd);
    }
    //
    return  err;
}


static camera_device_ops const
gCameraDevOps = 
{
    #define OPS(name) name: camera_##name

    OPS(set_preview_window), 
    OPS(set_callbacks), 
    OPS(enable_msg_type), 
    OPS(disable_msg_type), 
    OPS(msg_type_enabled), 
    OPS(start_preview), 
    OPS(stop_preview), 
    OPS(preview_enabled), 
    OPS(store_meta_data_in_buffers), 
    OPS(start_recording), 
    OPS(stop_recording), 
    OPS(recording_enabled), 
    OPS(release_recording_frame), 
    OPS(auto_focus), 
    OPS(cancel_auto_focus), 
    OPS(take_picture), 
    OPS(cancel_picture), 
    OPS(set_parameters), 
    OPS(get_parameters), 
    OPS(put_parameters), 
    OPS(send_command), 
    OPS(release), 
    OPS(dump), 

    #undef  OPS
};


/******************************************************************************
 *
 ******************************************************************************/
Cam1Device::
Cam1Device()
    : ICamDevice()
    , mpModuleCallbacks(NULL)
    , mDevice()
    , mDeviceOps()
{
    MY_LOGD("ctor");
    ::memset(&mDevice, 0, sizeof(mDevice));
    mDevice.priv    = this;
    mDevice.common  = gHwDevice;
    mDevice.ops     = &mDeviceOps;
    mDeviceOps      = gCameraDevOps;
    //
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam1Device::
onLastStrongRef(const void* id)
{
    MY_LOGD("");
    uninitialize();
}

