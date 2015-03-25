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

#ifndef _MTK_CAMERA_INC_IDISPLAYCLIENT_H_
#define _MTK_CAMERA_INC_IDISPLAYCLIENT_H_
//
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
//
#include <hardware/camera.h>
#include <system/camera.h>
//


namespace android {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*
*******************************************************************************/
class IDisplayClient : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static  IDisplayClient*         createInstance();
    virtual                         ~IDisplayClient() {}

public:     ////

    /**
     * Initialize the display adapter to create any resources required.
     */
    virtual bool                    init()                          = 0;
    /**
     * Uninitialize the display adapter.
     */
    virtual bool                    uninit()                        = 0;

    /**
     * Set the preview_stream_ops to which frames are sent.
     *
     * Notes:
     *  (1) When calling setWindow(), all preview parameters have been decided.
     *      [CameraService]
     *          mHardware->setParameters() -> mHardware->setPreviewWindow() -> mHardware->startPreview()
     *          --> enableDisplay during startPreview()
     *          mHardware->setParameters() -> mHardware->startPreview() -> mHardware->setPreviewWindow()
     *          --> enableDisplay during setPreviewWindow()
     *  (2) During inactive preview, window may be changed by setWindow().
     */
    virtual bool                    setWindow(
                                        preview_stream_ops*const window, 
                                        int32_t const   wndWidth, 
                                        int32_t const   wndHeight, 
                                        int32_t const   i4MaxImgBufCount
                                    )                               = 0;

    /**
     *
     */
    virtual bool                    setImgBufProviderClient(
                                        sp<IImgBufProviderClient>const& rpClient
                                    )                               = 0;

    /**
     *
     */
    virtual bool                    disableDisplay()                = 0;
    virtual bool                    enableDisplay()                 = 0;
    virtual bool                    enableDisplay(
                                        int32_t const   i4Width, 
                                        int32_t const   i4Height, 
                                        int32_t const   i4BufCount, 
                                        sp<IImgBufProviderClient>const& rpClient
                                    )                               = 0;
#if 1
    virtual status_t                waitUntilDrained()              = 0;
#endif
    /**
     *
     */
    virtual status_t                dump(int fd, Vector<String8>const& args)    = 0;

};


}; // namespace android
#endif  //_MTK_CAMERA_INC_IDISPLAYCLIENT_H_

