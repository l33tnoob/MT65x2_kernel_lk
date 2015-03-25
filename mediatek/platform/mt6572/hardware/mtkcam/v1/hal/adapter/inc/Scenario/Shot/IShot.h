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

#ifndef _MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_
#define _MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_
//
#include <utils/RefBase.h>
#include <utils/String8.h>

#include <camera/MtkCamera.h>

namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/
class ImpShot;


/******************************************************************************
 *  Interface of Shot Callback.
 ******************************************************************************/
class IShotCallback : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~IShotCallback() {}

public:     ////                    Operations.

    //
    //  Callback of Error (CAMERA_MSG_ERROR)
    //
    //  Arguments:
    //      ext1
    //          [I] extend argument 1.
    //
    //      ext2
    //          [I] extend argument 2.
    //
    virtual bool                    onCB_Error(
                                        int32_t ext1 = 0,
                                        int32_t ext2 = 0
                                    )   = 0;

    //
    //  Callback of Shutter (CAMERA_MSG_SHUTTER)
    //
    //      Invoking this callback may play a shutter sound.
    //
    //  Arguments:
    //      bPlayShutterSound
    //          [I] Play a shutter sound if ture; otherwise play no sound.
    //
    //      u4CallbackIndex
    //          [I] Callback index. 0 by default.
    //              If more than one shutter callback must be invoked during
    //              captures, for example burst shot & ev shot, this value is
    //              the callback index; and 0 indicates the first one callback.
    //
    virtual bool                    onCB_Shutter(
                                        bool const bPlayShutterSound = true, 
                                        uint32_t const u4CallbackIndex = 0
                                    )   = 0;

    //
    //  Callback of Postview for Display
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Postview timestamp
    //
    //      u4PostviewSize
    //          [I] Postview buffer size in bytes.
    //
    //      puPostviewBuf
    //          [I] Postview buffer with its size = u4PostviewSize
    //
    virtual bool                    onCB_PostviewDisplay(
                                        int64_t const   i8Timestamp, 
                                        uint32_t const  u4PostviewSize, 
                                        uint8_t const*  puPostviewBuf
                                    )   = 0;

    //
    //  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Postview timestamp
    //
    //      u4PostviewSize
    //          [I] Postview buffer size in bytes.
    //
    //      puPostviewBuf
    //          [I] Postview buffer with its size = u4PostviewSize
    //
    virtual bool                    onCB_PostviewClient(
                                        int64_t const   i8Timestamp, 
                                        uint32_t const  u4PostviewSize, 
                                        uint8_t const*  puPostviewBuf
                                    )   = 0;

    //
    //  Callback of Raw Image (CAMERA_MSG_RAW_IMAGE/CAMERA_MSG_RAW_IMAGE_NOTIFY)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Raw image timestamp
    //
    //      u4RawImgSize
    //          [I] Raw image buffer size in bytes.
    //
    //      puRawImgBuf
    //          [I] Raw image buffer with its size = u4RawImgSize
    //
    virtual bool                    onCB_RawImage(
                                        int64_t const   i8Timestamp, 
                                        uint32_t const  u4RawImgSize, 
                                        uint8_t const*  puRawImgBuf
                                    )   = 0;

    //
    //  Callback of Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
    //
    //      [Compressed Image] = [Header] + [Bitstream], 
    //      where 
    //          Header may be jpeg exif (including thumbnail)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Compressed image timestamp
    //
    //      u4BitstreamSize
    //          [I] Bitstream buffer size in bytes.
    //
    //      puBitstreamBuf
    //          [I] Bitstream buffer with its size = u4BitstreamSize
    //
    //      u4HeaderSize
    //          [I] Header size in bytes; header may be jpeg exif.
    //
    //      puHeaderBuf
    //          [I] Header buffer with its size = u4HeaderSize
    //
    //      u4CallbackIndex
    //          [I] Callback index. 0 by default.
    //              If more than one compressed callback must be invoked during
    //              captures, for example burst shot & ev shot, this value is
    //              the callback index; and 0 indicates the first one callback.
    //
    //      fgIsFinalImage
    //          [I] booliean value to indicate whether it is the final image.
    //              true if this is the final image callback; otherwise false.
    //              For single captures, this value must be true.
    //
    virtual bool                    onCB_CompressedImage(
                                        int64_t const   i8Timestamp,
                                        uint32_t const  u4BitstreamSize,
                                        uint8_t const*  puBitstreamBuf,
                                        uint32_t const  u4HeaderSize,
                                        uint8_t const*  puHeaderBuf,
                                        uint32_t const  u4CallbackIndex = 0,
                                        bool            fgIsFinalImage = true,
                                        uint32_t const  msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE
                                    )   = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
enum ECommand
{
    //  This command is to set shot-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ShotParam (i.e. ShotParam const*)
    //      arg2
    //          [I] sizeof(ShotParam)
    eCmd_setShotParam, 

    //  This command is to set jpeg-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to JpegParam (i.e. JpegParam const*)
    //      arg2
    //          [I] sizeof(JpegParam)
    eCmd_setJpegParam, 

    //  This command is to set capture buffer handler.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ICaptureBufMgrHandler
    //      arg2
    //          [I] sizeof(ICaptureBufMgrHandler)
    eCmd_setCaptureBufHandler,
    //  This command is to reset this class. After captures and then reset, 
    //  performing a new capture should work well, no matter whether previous 
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    eCmd_reset, 

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    eCmd_capture, 

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    eCmd_cancel, 
    //  This command is to perform set continuous shot speed.
    //
    //  Arguments:
    //          N/A
    eCmd_setCShotSpeed,

};


/******************************************************************************
 *
 ******************************************************************************/
struct ShotParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Picture.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for captured pictures.
    String8                         ms8PictureFormat;

    // The dimensions for captured pictures in pixels (width x height). 
    int32_t                         mi4PictureWidth;
    int32_t                         mi4PictureHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Postview Image.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for postview display.
    String8                         ms8PostviewDisplayFormat;

    // The image format for postview client.
    String8                         ms8PostviewClientFormat;

    // The dimensions for postview in pixels (width x height).
    int32_t                         mi4PostviewWidth;
    int32_t                         mi4PostviewHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Shot File Name.
    //      This is an auxiliary path/file name to save extra files if needed.
    //      ms8ShotFileName.string() returns a pointer to a C-style 
    //      null-terminated string (const char*)
    String8                         ms8ShotFileName;

    // The zoom ratio is in 1/100 increments. Ex: 320 refers to a zoom of 3.2x.
    uint32_t                        mu4ZoomRatio;

    // Shot count in total.
    //      0: request to take no picture.
    //      1: request to take only 1 picture.
    //      N: request to take N pictures.
    uint32_t                        mu4ShotCount;

    // The rotation angle in degrees relative to the orientation of the camera.
    //
    // For example, suppose the natural orientation of the device is portrait.
    // The device is rotated 270 degrees clockwise, so the device orientation is
    // 270. Suppose a back-facing camera sensor is mounted in landscape and the
    // top side of the camera sensor is aligned with the right edge of the
    // display in natural orientation. So the camera orientation is 90. The
    // rotation should be set to 0 (270 + 90).
    //
    // Example value: "0" or "90" or "180" or "270".
    int32_t                         mi4Rotation;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    ShotParam(
                                        String8 const&          s8PictureFormat         = String8::empty(), 
                                        int32_t const           i4PictureWidth          = 2560, 
                                        int32_t const           i4PictureHeight         = 1920, 
                                        //
                                        String8 const&          s8PostviewDisplayFormat = String8::empty(), 
                                        String8 const&          s8PostviewClientFormat  = String8::empty(), 
                                        int32_t const           i4PostviewWidth         = 640, 
                                        int32_t const           i4PostviewHeight        = 480, 
                                        //
                                        String8 const&          rs8ShotFileName         = String8::empty(), 
                                        uint32_t const          u4ZoomRatio             = 100, 
                                        uint32_t const          u4ShotCount             = 1, 
                                        int32_t const           i4Rotation              = 0
                                    )
                                        : ms8PictureFormat(s8PictureFormat)
                                        , mi4PictureWidth(i4PictureWidth)
                                        , mi4PictureHeight(i4PictureHeight)
                                        //
                                        , ms8PostviewDisplayFormat(s8PostviewDisplayFormat)
                                        , ms8PostviewClientFormat(s8PostviewClientFormat)
                                        , mi4PostviewWidth(i4PostviewWidth)
                                        , mi4PostviewHeight(i4PostviewHeight)
                                        //
                                        , ms8ShotFileName(rs8ShotFileName)
                                        , mu4ZoomRatio(u4ZoomRatio)
                                        , mu4ShotCount(u4ShotCount)
                                        , mi4Rotation(i4Rotation)
                                        //
                                    {
                                    }
                                    //
    ShotParam&                      operator=(ShotParam const& rhs)
                                    {
                                        ms8PictureFormat    = rhs.ms8PictureFormat;
                                        mi4PictureWidth     = rhs.mi4PictureWidth;
                                        mi4PictureHeight    = rhs.mi4PictureHeight;
                                        //
                                        ms8PostviewDisplayFormat= rhs.ms8PostviewDisplayFormat;
                                        ms8PostviewClientFormat = rhs.ms8PostviewClientFormat;
                                        mi4PostviewWidth    = rhs.mi4PostviewWidth;
                                        mi4PostviewHeight   = rhs.mi4PostviewHeight;
                                        //
                                        ms8ShotFileName     = rhs.ms8ShotFileName;
                                        mu4ZoomRatio        = rhs.mu4ZoomRatio;
                                        mu4ShotCount        = rhs.mu4ShotCount;
                                        mi4Rotation         = rhs.mi4Rotation;
                                        //
                                        return  (*this);
                                    }
};


/******************************************************************************
 *
 ******************************************************************************/
struct JpegParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Jpeg quality of captured picture. The range is 1 to 100, with 100 being
    // the best.
    uint32_t                        mu4JpegQuality;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg Thumb.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The quality of the EXIF thumbnail in Jpeg picture. The range is 1 to 100,
    // with 100 being the best.
    uint32_t                        mu4JpegThumbQuality;

    // The width (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbWidth;

    // The height (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GPS.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // GPS latitude coordinate.
    // Example value: "25.032146" or "-33.462809".
    String8                         ms8GpsLatitude;

    // GPS longitude coordinate.
    // Example value: "121.564448" or "-70.660286".
    String8                         ms8GpsLongitude;

    // GPS altitude.
    // header.
    // Example value: "21.0" or "-5".
    String8                         ms8GpsAltitude;

    // GPS timestamp (UTC in seconds since January 1, 1970).
    // Example value: "1251192757".
    String8                         ms8GpsTimestamp;

    // GPS Processing Method
    // Example value: "GPS" or "NETWORK".
    String8                         ms8GpsMethod;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    JpegParam(
                                        uint32_t const          u4JpegQuality       = 100, 
                                        uint32_t const          u4JpegThumbQuality  = 100, 
                                        int32_t const           i4JpegThumbWidth    = 160, 
                                        int32_t const           i4JpegThumbHeight   = 120, 
                                        String8 const&          s8GpsLatitude       = String8::empty(), 
                                        String8 const&          s8GpsLongitude      = String8::empty(), 
                                        String8 const&          s8GpsAltitude       = String8::empty(), 
                                        String8 const&          s8GpsTimestamp      = String8::empty(), 
                                        String8 const&          s8GpsMethod         = String8::empty()
                                    )
                                        : mu4JpegQuality(u4JpegQuality)
                                        //
                                        , mu4JpegThumbQuality(u4JpegThumbQuality)
                                        , mi4JpegThumbWidth(i4JpegThumbWidth)
                                        , mi4JpegThumbHeight(i4JpegThumbHeight)
                                        //
                                        , ms8GpsLatitude(s8GpsLatitude)
                                        , ms8GpsLongitude(s8GpsLongitude)
                                        , ms8GpsAltitude(s8GpsAltitude)
                                        , ms8GpsTimestamp(s8GpsTimestamp)
                                        , ms8GpsMethod(s8GpsMethod)
                                        //
                                    {
                                    }
                                    //
    JpegParam&                      operator=(JpegParam const& rhs)
                                    {
                                        mu4JpegQuality  = rhs.mu4JpegQuality;
                                        //
                                        mu4JpegThumbQuality = rhs.mu4JpegThumbQuality;
                                        mi4JpegThumbWidth   = rhs.mi4JpegThumbWidth;
                                        mi4JpegThumbHeight  = rhs.mi4JpegThumbHeight;
                                        //
                                        ms8GpsLatitude      = rhs.ms8GpsLatitude;
                                        ms8GpsLongitude     = rhs.ms8GpsLongitude;
                                        ms8GpsAltitude      = rhs.ms8GpsAltitude;
                                        ms8GpsTimestamp     = rhs.ms8GpsTimestamp;
                                        ms8GpsMethod        = rhs.ms8GpsMethod;
                                        //
                                        return  (*this);
                                    }
};


/******************************************************************************
 *  Interface of Shot Class.
 *******************************************************************************/
class IShot : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual char const*             getShotName() const;
    virtual uint32_t                getShotMode() const;
    virtual int32_t                 getOpenId() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    IShot(sp<ImpShot> pImpShot);
    virtual                         ~IShot();

    virtual bool                    setCallback(sp<IShotCallback> pShotCallback);

public:     ////                    Operations.
    virtual bool                    sendCommand(
                                        ECommand const  cmd, 
                                        uint32_t const  arg1 = 0, 
                                        uint32_t const  arg2 = 0
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    sp<ImpShot>                     mpImpShot;

};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //_MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_

