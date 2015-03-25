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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_ICAMDEVICEMANAGER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_ICAMDEVICEMANAGER_H_
//
#include <utils/Errors.h>
#include <utils/StrongPointer.h>
//
#include <hardware/camera_common.h>
#include <mtkcam/common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class ICamDevice;


/******************************************************************************
 *
 ******************************************************************************/
class ICamDeviceManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Destructor.

    //  Disallowed to directly delete a raw pointer.
    virtual                             ~ICamDeviceManager() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        

    virtual android::status_t           open(
                                            hw_module_t const* module, 
                                            char const* name, 
                                            hw_device_t** device
                                        )                                       = 0;

    virtual android::status_t           close(ICamDevice* pDevice)              = 0;

    /**
     * getNumberOfDevices:
     *
     * Returns the number of camera devices accessible through the camera
     * module.  The camera devices are numbered 0 through N-1, where N is the
     * value returned by this call. The name of the camera device for open() is
     * simply the number converted to a string. That is, "0" for camera ID 0,
     * "1" for camera ID 1.
     *
     * The value here must be static, and cannot change after the first call to
     * this method
     */
    virtual int32_t                     getNumberOfDevices()                    = 0;

    /**
     * getDeviceInfo:
     *
     * Return the static information for a given camera device. This information
     * may not change for a camera device.
     *
     */
    virtual android::status_t           getDeviceInfo(
                                            int const deviceId, 
                                            camera_info& rInfo
                                        )                                       = 0;

    /**
     * set_callbacks:
     *
     * Provide callback function pointers to the HAL module to inform framework
     * of asynchronous camera module events. The framework will call this
     * function once after initial camera HAL module load, after the
     * get_number_of_cameras() method is called for the first time, and before
     * any other calls to the module.
     *
     * Version information (based on camera_module_t.common.module_api_version):
     *
     *  CAMERA_MODULE_API_VERSION_1_0, CAMERA_MODULE_API_VERSION_2_0:
     *
     *    Not provided by HAL module. Framework may not call this function.
     *
     *  CAMERA_MODULE_API_VERSION_2_1:
     *
     *    Valid to be called by the framework.
     *
     */
    virtual android::status_t           setCallbacks(
                                            camera_module_callbacks_t const* callbacks
                                        )                                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
ICamDeviceManager*  getCamDeviceManager();


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_ICAMDEVICEMANAGER_H_

