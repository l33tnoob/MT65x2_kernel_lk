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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICE_H_
//
#include <utils/String8.h>
//
#include <hardware/camera.h>
#include "ICamDevice.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class Cam1Device : public ICamDevice
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    typedef android::status_t       status_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.

    camera_module_callbacks_t const*mpModuleCallbacks;
    camera_device                   mDevice;
    camera_device_ops               mDeviceOps;         //  which is pointed to by mDevice.ops.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    //
    static  inline Cam1Device*      getDevice(camera_device* device)
                                    {
                                        return (NULL == device)
                                                ? NULL
                                                : (Cam1Device*)((device)->priv)
                                                ;
                                    }

    static  inline Cam1Device*      getDevice(hw_device_t* device)
                                    {
                                        return  getDevice((camera_device*)device);
                                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Instantiation.
    virtual                         ~Cam1Device() {}
                                    Cam1Device();

    virtual void                    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual status_t                closeDevice()                           = 0;

    /**
     * Initialize the device resources owned by this object.
     *
     * Return values:
     *
     *  0:     On successful initialization
     *
     * -ENODEV: If initialization fails. Only uninitialize() can be called 
     *          successfully by the framework after this.
     */
    virtual status_t                initialize()                            = 0;

    /**
     * Uninitialize the device resources owned by this object. Note that this 
     * is *not* done in the destructor.
     */
    virtual void                    uninitialize()                          = 0;

    /**
     * Set the notification and data callbacks
     */
    virtual void                    setCallbacks(
                                        camera_notify_callback notify_cb,
                                        camera_data_callback data_cb,
                                        camera_data_timestamp_callback data_cb_timestamp,
                                        camera_request_memory get_memory,
                                        void *user
                                    )                                       = 0;

    /**
     * Enable a message, or set of messages.
     */
    virtual void                    enableMsgType(int32_t msgType)          = 0;

    /**
     * Disable a message, or a set of messages.
     *
     * Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
     * should not rely on its client to call releaseRecordingFrame() to release
     * video recording frames sent out by the cameral hal before and after the
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
     * modify/access any video recording frame after calling
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME).
     */
    virtual void                    disableMsgType(int32_t msgType)         = 0;

    /**
     * Query whether a message, or a set of messages, is enabled.
     * Note that this is operates as an AND, if any of the messages
     * queried are off, this will return false.
     */
    virtual bool                    msgTypeEnabled(int32_t msgType)         = 0;

    /**
     * Set the preview_stream_ops to which preview frames are sent.
     */
    virtual status_t                setPreviewWindow(preview_stream_ops* window) = 0;

    /**
     * Start preview mode.
     */
    virtual status_t                startPreview()                          = 0;

    /**
     * Stop a previously started preview.
     */
    virtual void                    stopPreview()                           = 0;

    /**
     * Returns true if preview is enabled.
     */
    virtual bool                    previewEnabled()                        = 0;

    /**
     * Request the camera hal to store meta data or real YUV data in
     * the video buffers send out via CAMERA_MSG_VIDEO_FRRAME for a
     * recording session. If it is not called, the default camera
     * hal behavior is to store real YUV data in the video buffers.
     *
     * This method should be called before startRecording() in order
     * to be effective.
     *
     * If meta data is stored in the video buffers, it is up to the
     * receiver of the video buffers to interpret the contents and
     * to find the actual frame data with the help of the meta data
     * in the buffer. How this is done is outside of the scope of
     * this method.
     *
     * Some camera hal may not support storing meta data in the video
     * buffers, but all camera hal should support storing real YUV data
     * in the video buffers. If the camera hal does not support storing
     * the meta data in the video buffers when it is requested to do
     * do, INVALID_OPERATION must be returned. It is very useful for
     * the camera hal to pass meta data rather than the actual frame
     * data directly to the video encoder, since the amount of the
     * uncompressed frame data can be very large if video size is large.
     *
     * @param enable if true to instruct the camera hal to store
     *      meta data in the video buffers; false to instruct
     *      the camera hal to store real YUV data in the video
     *      buffers.
     *
     * @return OK on success.
     */
    virtual status_t                storeMetaDataInBuffers(bool enable)     = 0;

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual status_t                startRecording()                        = 0;

    /**
     * Stop a previously started recording.
     */
    virtual void                    stopRecording()                         = 0;

    /**
     * Returns true if recording is enabled.
     */
    virtual bool                    recordingEnabled()                      = 0;

    /**
     * Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
     *
     * It is camera hal client's responsibility to release video recording
     * frames sent out by the camera hal before the camera hal receives
     * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
     * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
     * responsibility of managing the life-cycle of the video recording
     * frames.
     */
    virtual void                    releaseRecordingFrame(const void *opaque)   = 0;

    /**
     * Start auto focus, the notification callback routine is called
     * with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
     * will be called again if another auto focus is needed.
     */
    virtual status_t                autoFocus()                             = 0;

    /**
     * Cancels auto-focus function. If the auto-focus is still in progress,
     * this function will cancel it. Whether the auto-focus is in progress
     * or not, this function will return the focus position to the default.
     * If the camera does not support auto-focus, this is a no-op.
     */
    virtual status_t                cancelAutoFocus()                       = 0;

    /**
     * Take a picture.
     */
    virtual status_t                takePicture()                           = 0;

    /**
     * Cancel a picture that was started with takePicture.  Calling this
     * method when no picture is being taken is a no-op.
     */
    virtual status_t                cancelPicture()                         = 0;

    /**
     * Set the camera parameters. This returns BAD_VALUE if any parameter is
     * invalid or not supported.
     */
    virtual status_t                setParameters(const char* params)       = 0;

    /** 
     * Return the camera parameters.
     */
    virtual char*                   getParameters()                         = 0;

    /**
     * Put the camera parameters.
     */
    virtual void                    putParameters(char *params)             = 0;

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) = 0;

    /**
     * Dump state of the camera hardware
     */
    virtual status_t                dump(int fd)                            = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICamDevice Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual hw_device_t const*      get_hw_device() const { return &mDevice.common; }

    virtual void                    set_hw_module(hw_module_t const* module)
                                    {   
                                        mDevice.common.module = const_cast<hw_module_t*>(module);
                                    }

    virtual void                    set_module_callbacks(camera_module_callbacks_t const* callbacks)
                                    {
                                        mpModuleCallbacks = callbacks;
                                    }
};
};  //namespace NSCam


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NSCam::Cam1Device*
createCam1Device(
    android::String8 const  s8ClientAppMode, 
    int32_t const           i4OpenId
);


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM1DEVICE_H_

