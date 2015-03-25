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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IMAGEBUFFERHEAP_H_
//
#include <mtkcam/utils/BaseImageBufferHeap.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

struct PortBufInfo_v1
{
    MINT32      memID;
    MUINT32     virtAddr;
    MINT32      nocache;
    MINT32      security;
    MINT32      coherence;
    //
    PortBufInfo_v1(
        MINT32  _memID = -1,
        MUINT32 _virtAddr = 0,
        MINT32 _nocache = 0, 
        MINT32 _security = 0, 
        MINT32 _coherence = 0
    )
        : memID(_memID)
        , virtAddr(_virtAddr)
        , nocache(_nocache)
        , security(_security)
        , coherence(_coherence)
    {
    }
};

/******************************************************************************
 *  Image Buffer Heap (Camera1).
 ******************************************************************************/
class ImageBufferHeap : public NSImageBufferHeap::BaseImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for v1 Buffer
    typedef IImageBufferAllocator::ImgParam ImgParam_t;


public:     ////                    Creation.
    static  ImageBufferHeap*     create(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam, 
                                        PortBufInfo_v1 const& rPortBufInfo
                                    );

public:     ////                    Attributes.
    static  char const*             magicName() { return "ImageBufferHeap_v1"; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual char const*             impGetMagicName()                   const   { return magicName(); }

    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return mvHeapInfo; }

    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo);

public:     ////
    virtual MBOOL                   impLockBuf(
                                        char const* szCallerName, 
                                        MINT usage, 
                                        BufInfoVect_t const& rvBufInfo
                                    );
    virtual MBOOL                   impUnlockBuf(
                                        char const* szCallerName, 
                                        MINT usage, 
                                        BufInfoVect_t const& rvBufInfo
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual MBOOL                   doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);
    virtual MBOOL                   doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~ImageBufferHeap() {}
                                    ImageBufferHeap(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam,
                                        PortBufInfo_v1 const& rPortBufInfo
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Info to Allocate.
    MSize                           mBufStridesInPixelsToAlloc[3];  // buffer strides in pixels.

protected:  ////                    Info of Allocated Result.
    PortBufInfo_v1                  mPortBufInfo;   //
    HeapInfoVect_t                  mvHeapInfo;     //
    BufInfoVect_t                   mvBufInfo;      //

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IONIMAGEBUFFERHEAP_H_

