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

#ifndef _MTK_HAL_CAMADAPTER_MTKPHOTO_INC_IMPSHOTCALLBACK_INL_
#define _MTK_HAL_CAMADAPTER_MTKPHOTO_INC_IMPSHOTCALLBACK_INL_
//
/******************************************************************************
 *
 *  This is an inline file, which declares the implementation of Shot Callback.
 *
 ******************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IShotCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

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
                                        int32_t ext1,
                                        int32_t ext2
                                    );

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
                                        bool const bPlayShutterSound, 
                                        uint32_t const u4CallbackIndex
                                    );

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
                                    );

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
                                    );

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
                                    );

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
                                        uint32_t const  u4CallbackIndex, 
                                        bool            fgIsFinalImage,
                                        uint32_t const  msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE
                                    );


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HAL_CAMADAPTER_MTKPHOTO_INC_IMPSHOTCALLBACK_INL_

