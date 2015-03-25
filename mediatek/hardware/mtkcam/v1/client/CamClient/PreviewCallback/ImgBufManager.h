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

#ifndef _MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_
#define _MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_
//


namespace android {
namespace NSCamClient {
namespace NSPrvCbClient {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Image Info
*******************************************************************************/
struct ImgInfo : public LightRefBase<ImgInfo>
{
    String8                         ms8ImgName;
    String8                         ms8ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4BitsPerPixel;
    size_t                          mImgBufSize;
    //
                                    ImgInfo(
                                        uint32_t const u4ImgWidth, 
                                        uint32_t const u4ImgHeight, 
                                        char const*const ImgFormat, 
                                        char const*const pImgName = ""
                                    )
                                        : ms8ImgName(pImgName)
                                        , ms8ImgFormat(ImgFormat)
                                        , mu4ImgWidth(u4ImgWidth)
                                        , mu4ImgHeight(u4ImgHeight)
                                        , mu4BitsPerPixel( FmtUtils::queryBitsPerPixel(ms8ImgFormat) )
                                        , mImgBufSize( FmtUtils::queryImgBufferSize(ms8ImgFormat, mu4ImgWidth, mu4ImgHeight) )
                                    {
                                        CAM_LOGD(
                                            "[ImgInfo::ImgInfo] [%s](%s@%dx%d@%d-bit@%d)", 
                                            ms8ImgName.string(), ms8ImgFormat.string(), 
                                            mu4ImgWidth, mu4ImgHeight, mu4BitsPerPixel, mImgBufSize
                                        );
                                    }
};


/******************************************************************************
*   image buffer for preview callback
*******************************************************************************/
class PrvCbImgBuf : public ICameraImgBuf
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
    virtual size_t                  getBufSize() const                      { return mpImgInfo->mImgBufSize; }
    //
    virtual void*                   getVirAddr() const                      { return mCamMem.data; }
    virtual void*                   getPhyAddr() const                      { return mCamMem.data; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual String8 const&          getImgFormat()      const               { return mpImgInfo->ms8ImgFormat; }
    virtual uint32_t                getImgWidth()       const               { return mpImgInfo->mu4ImgWidth;  }
    virtual uint32_t                getImgHeight()      const               { return mpImgInfo->mu4ImgHeight; }
    virtual uint32_t                getImgWidthStride(
                                        uint_t const uPlaneIndex = 0
                                    )  const
                                    {
                                        return  FmtUtils::queryImgWidthStride(getImgFormat(), getImgWidth(), uPlaneIndex);
                                    }
    virtual uint32_t                getBitsPerPixel()   const               { return mpImgInfo->mu4BitsPerPixel; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraBuf Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual uint_t                  getBufIndex() const                     { return 0; }
    virtual camera_memory_t*        get_camera_memory()                     { return &mCamMem; }
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static PrvCbImgBuf*             alloc(
                                        camera_request_memory   requestMemory, 
                                        sp<ImgInfo const>const& rpImgInfo
                                    );

public:     ////                    Instantiation.
                                    PrvCbImgBuf(
                                        camera_memory_t const&  rCamMem, 
                                        sp<ImgInfo const>const& rpImgInfo, 
                                        int64_t const timestamp = 0
                                    );
    virtual                         ~PrvCbImgBuf();

public:     ////                    Debug.
    void                            dump() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Memory.
    sp<ImgInfo const>               mpImgInfo;
    int64_t                         mi8Timestamp;
    camera_memory_t                 mCamMem;
};


/******************************************************************************
*   
*******************************************************************************/
class ImgBufManager : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static ImgBufManager*           alloc(
                                        char const*const        szImgFormat, 
                                        uint32_t const          u4ImgWidth, 
                                        uint32_t const          u4ImgHeight, 
                                        uint32_t const          u4BufCount, 
                                        char const*const        szName, 
                                        camera_request_memory   requestMemory
                                    );

public:     ////                    Attributes.
    //
    virtual char const*             getName() const             { return ms8Name.string(); }
    sp<ICameraImgBuf>const&         getBuf(size_t index) const  { return mvImgBuf[index]; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Instantiation.
    virtual                         ~ImgBufManager();

protected:  ////                    Instantiation.
                                    ImgBufManager(
                                        char const*const        szImgFormat, 
                                        uint32_t const          u4ImgWidth, 
                                        uint32_t const          u4ImgHeight, 
                                        uint32_t const          u4BufCount, 
                                        char const*const        szName, 
                                        camera_request_memory   requestMemory
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    //
    bool                            init();
    void                            uninit();

protected:  ////                    Data Members.
    //
    String8                         ms8Name;
    String8                         ms8ImgFormat;
    uint32_t                        mu4ImgWidth;
    uint32_t                        mu4ImgHeight;
    uint32_t                        mu4BufCount;
    //
    Vector< sp<ICameraImgBuf> >     mvImgBuf;
    camera_request_memory           mRequestMemory;

};


}; // namespace NSPrvCbClient
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_CAMERA_CLIENT_CAMCLIENT_PREVIEWCALLBACK_IMGBUFMANAGER_H_

