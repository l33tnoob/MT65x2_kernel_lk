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

#ifndef _MTK_CAMERA_CLIENT_DISPLAYCLIENT_STREAMIMGBUF_H_
#define _MTK_CAMERA_CLIENT_DISPLAYCLIENT_STREAMIMGBUF_H_
//
#include <utils/RefBase.h>


namespace android {
namespace NSDisplayClient {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  Image Info
 ******************************************************************************/
struct ImgInfo : public LightRefBase<ImgInfo>
{
    String8                         ms8ImgName;
    String8                         ms8ImgFormat;
    int32_t                         mi4ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4BitsPerPixel;
    //
                                    ImgInfo(
                                        uint32_t const u4ImgWidth, 
                                        uint32_t const u4ImgHeight, 
                                        char const*const ImgFormat, 
                                        int32_t const i4ImgFormat, 
                                        char const*const pImgName = ""
                                    );
};


/******************************************************************************
 *  preview_stream_ops image buffer
 ******************************************************************************/
class StreamImgBuf : public IImgBuf
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IMemBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual int64_t                 getTimestamp() const                    { return mi8Timestamp; }
    virtual void                    setTimestamp(int64_t const timestamp)   { mi8Timestamp = timestamp; }
    //
public:     ////                    Attributes.
    virtual const char*             getBufName() const                      { return mpImgInfo->ms8ImgName.string(); }
    virtual size_t                  getBufSize() const                      { return mBufSize; }
    //
    virtual void*                   getVirAddr() const                      { return mpBufBase; }
    virtual void*                   getPhyAddr() const                      { return 0; }
    //
    virtual int                     getIonFd() const                        { return mfdIon; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual String8 const&          getImgFormat()      const               { return mpImgInfo->ms8ImgFormat; }
    virtual uint32_t                getImgWidth()       const               { return mpImgInfo->mu4ImgWidth; }
    virtual uint32_t                getImgHeight()      const               { return mpImgInfo->mu4ImgHeight; }
    virtual uint32_t                getImgWidthStride(
                                        uint_t const uPlaneIndex = 0
                                    )                   const;
    virtual uint32_t                getBitsPerPixel()   const               { return mpImgInfo->mu4BitsPerPixel; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  preview_stream_ops Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes.
    //
    inline  native_handle_t const*  getNativeHndlPtr() const                { return *mpBufHndl; }
    //
    inline  buffer_handle_t         getBufHndl()    const                   { return *mpBufHndl; }
    //
    inline  buffer_handle_t*        getBufHndlPtr() const                   { return mpBufHndl; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~StreamImgBuf();
                                    StreamImgBuf(
                                        sp<ImgInfo const>const& rpImgInfo, 
                                        int32_t const   i4Stride, 
                                        void*const      pBufBase, 
                                        buffer_handle_t*pBufHndl, 
                                        int const       fdIon    = -1, 
                                        int64_t const   timestamp= 0
                                    );

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Debug.
    void                            dump() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Memory.
    sp<ImgInfo const>               mpImgInfo;
    int64_t                         mi8Timestamp;
    int                             mfdIon;         //  ion shared file descriptor.
    void*                           mpBufBase;      //  Pointer to the locked buffer base address.
    buffer_handle_t*                mpBufHndl;      //  Pointer to the locked buffer handle.
    sp<ANativeWindowBuffer>         mpANWBuffer;    //  
    int32_t                         mi4Stride;      //
    size_t                          mBufSize;       //
};


}; // namespace NSDisplayClient
}; // namespace android
#endif  //_MTK_CAMERA_CLIENT_DISPLAYCLIENT_STREAMIMGBUF_H_

