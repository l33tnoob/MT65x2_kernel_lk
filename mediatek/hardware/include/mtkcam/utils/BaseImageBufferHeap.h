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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFERHEAP_H_
//
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <utils/String8.h>
#include <mtkcam/IImageBuffer.h>


/******************************************************************************
 *
 ******************************************************************************/
class IHalMemory;


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSImageBufferHeap {
using namespace android;


/******************************************************************************
 *  Image Buffer Heap (Base).
 ******************************************************************************/
class BaseImageBufferHeap : public IImageBufferHeap, protected virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Reference Counting.
    virtual MVOID                   incStrong(MVOID const* id)          const   { RefBase::incStrong(id); }
    virtual MVOID                   decStrong(MVOID const* id)          const   { RefBase::decStrong(id); }
    virtual MINT32                  getStrongCount()                    const   { return RefBase::getStrongCount(); }

public:     ////                    Image Attributes.
    virtual MINT                    getImgFormat()                      const   { return mImgFormat; }
    virtual MSize const&            getImgSize()                        const   { return mImgSize; }
    virtual MINT                    getImgBitsPerPixel()                const;
    virtual MINT                    getPlaneBitsPerPixel(MUINT index)   const;
    virtual MUINT                   getPlaneCount()                     const   { return mPlaneCount; }
    virtual MINT32                  getBitstreamSize()                   const   { return mBitstreamSize; }

public:     ////                    Buffer Attributes.
    virtual char const*             getMagicName()                      const   { return impGetMagicName(); }
    virtual MINT32                  getHeapID(MUINT index)              const;
    virtual MINT32                  getHeapIDCount()                    const;
    virtual MINT32                  getBufPA(MUINT index)               const;
    virtual MINT32                  getBufVA(MUINT index)               const;
    virtual MUINT32                 getBufSizeInBytes(MUINT index)      const;
    virtual MSize                   getBufStridesInBytes(MUINT index)   const;
    virtual MSize                   getBufStridesInPixels(MUINT index)  const;

public:     ////                    Buffer Operations.
    virtual MBOOL                   lockBuf(
                                        char const* szCallerName, 
                                        MINT usage
                                    );
    virtual MBOOL                   unlockBuf(
                                        char const* szCallerName
                                    );
    virtual MBOOL                   flushAllCache();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBuffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    /**
     * Create an IImageBuffer instance with its ROI equal to the image full 
     * resolution of this heap.
     */
    virtual IImageBuffer*           createImageBuffer();

    /**
     * This call is legal only if the heap format is blob.
     * 
     * From the given blob heap, create an IImageBuffer instance with a specified
     * offset and size, and its format equal to blob.
     */
    virtual IImageBuffer*           createImageBuffer_FromBlobHeap(
                                        MUINT32     offsetInBytes, 
                                        MINT32      sizeInBytes
                                    );

    /**
     * This call is legal only if the heap format is blob.
     * 
     * From the given blob heap, create an IImageBuffer instance with a specified
     * offset, image format, image size in pixels, and buffer strides in pixels.
     */
    virtual IImageBuffer*           createImageBuffer_FromBlobHeap(
                                        MUINT32     offsetInBytes, 
                                        MINT32      imgFormat, 
                                        MSize const&imgSize, 
                                        MSize const bufStridesInPixels[3]
                                    );

    /**
     * Create an IImageBuffer instance indicating the left-side or right-side 
     * buffer within a side-by-side image.
     * 
     * Left side if isRightSide = 0; otherwise right side.
     */
    virtual IImageBuffer*           createImageBuffer_SideBySide(
                                        MBOOL       isRightSide
                                    );

    /**
     * Create an IImageBuffer instance with a specified ROI in pixels.
     * Image ROI is defined as a rectangle (x, y, w, h), with (x, y) describing 
     * the top-left pixel of the image.  Both w and h cannot be larger than the 
     * image resolution of this heap.
     */
    virtual IImageBuffer*           createImageBuffer(
                                        MRect const&imgROI
                                    );



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Heap Info.
                                    struct HeapInfo : public LightRefBase<HeapInfo>
                                    {
                                    MINT32      heapID;             // heap ID.
                                                //
                                                HeapInfo()
                                                    : heapID(-1)
                                                {
                                                }
                                    };
    typedef Vector<sp<HeapInfo> >   HeapInfoVect_t;

public:  ////                    Buffer Info.
                                    struct BufInfo : public LightRefBase<BufInfo>
                                    {
                                    MINT32      pa;                 // (plane) physical address
                                    MINT32      va;                 // (plane) virtual address
                                    MSize       stridesInPixels;    // (plane) strides in pixels
                                    MUINT32     sizeInBytes;        // (plane) size in bytes
                                                //
                                                BufInfo(
                                                    MINT32          _pa = 0, 
                                                    MINT32          _va = 0, 
                                                    MSize const&    _stridesInPixels = MSize(), 
                                                    MUINT32         _sizeInBytes = 0
                                                )
                                                    : pa(_pa)
                                                    , va(_va)
                                                    , stridesInPixels(_stridesInPixels)
                                                    , sizeInBytes(_sizeInBytes)
                                                {
                                                }
                                    };
    typedef Vector<sp<BufInfo> >    BufInfoVect_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Template-Method Pattern. Subclass must implement them.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    /**
     * Return a pointer to a null-terminated string to indicate a magic name of 
     * buffer type.
     */
    virtual char const*             impGetMagicName()                   const   = 0;

    /**
     * This call is valid after calling impLockBuf(); 
     * invalid after impUnlockBuf().
     */
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   = 0;

    /**
     * onCreate() must be invoked by a subclass when its instance is created to
     * inform this base class of a creating event.
     * The call impInit(), implemented by a subclass, will be invoked by this 
     * base class during onCreate() for initialization.
     * As to buffer information (i.e. BufInfoVect_t), buffer strides in pixels 
     * and buffer size in bytes of each plane as well as the vector size MUST be
     * legal, at least, after impInit() return success.
     *
     * onLastStrongRef() will be invoked to indicate the last one reference to
     * this instance before it is freed.
     * The call impUninit(), implemented by a subclass, will be invoked by this
     * base class during onLastStrongRef() for uninitialization.
     */
    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo)     = 0;
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo)   = 0;

public:     ////
    /**
     * As to buffer information (i.e. BufInfoVect_t), buffer strides in pixels 
     * and buffer size in bytes of each plane as well as the vector size MUST be
     * always legal.
     *
     * After calling impLockBuf() successfully, the heap information from 
     * impGetHeapInfo() must be legal; virtual address and physical address of 
     * each plane must be legal if any SW usage and any HW usage are specified, 
     * respectively.
     */
    virtual MBOOL                   impLockBuf(
                                        char const* szCallerName, 
                                        MINT usage, 
                                        BufInfoVect_t const& rvBufInfo
                                    )                                           = 0;
    virtual MBOOL                   impUnlockBuf(
                                        char const* szCallerName, 
                                        MINT usage, 
                                        BufInfoVect_t const& rvBufInfo
                                    )                                           = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Helper Functions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Helper Params.
                                    struct HelperParamMapPA
                                    {
                                    MINT32      phyAddr;        // physicall address; in/out
                                    MINT32      virAddr;        // virtual address
                                    MINT32      ionFd;          // ION file descriptor
                                    MINT32      size;
                                    MINT32      security;
                                    MINT32      coherence;
                                    };

                                    struct HelperParamFlushCache
                                    {
                                    MINT32      virAddr;        // virtual address
                                    MINT32      ionFd;          // ION file descriptor
                                    MINT32      size;
                                    };

protected:  ////                    Helper Functions.
    virtual MBOOL                   helpMapPhyAddr(
                                        char const* szCallerName, 
                                        HelperParamMapPA& rParam
                                    );

    virtual MBOOL                   helpUnmapPhyAddr(
                                        char const* szCallerName, 
                                        HelperParamMapPA const& rParam
                                    );

    virtual MBOOL                   helpFlushCache(
                                        HelperParamFlushCache const* paParam, 
                                        MUINT const num
                                    );

    virtual MBOOL                   helpCheckBufStrides(
                                        MUINT const planeIndex, 
                                        MSize const& planeBufStridesInPixels
                                    ) const;

    virtual MUINT32                 helpQueryBufSizeInBytes(
                                        MUINT const planeIndex, 
                                        MSize const& planeStridesInPixels
                                    ) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Called inside lock.
    virtual MBOOL                   initLocked();
    virtual MBOOL                   uninitLocked();
    virtual MBOOL                   lockBufLocked(char const* szCallerName, MINT usage);
    virtual MBOOL                   unlockBufLocked(char const* szCallerName);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~BaseImageBufferHeap();
                                    BaseImageBufferHeap(char const* szCallerName);

protected:  ////                    Callback (LastStrongRef@RefBase)
    virtual void                    onLastStrongRef(const void* id);

protected:  ////                    Callback (Create)
    virtual MBOOL                   onCreate(
                                        MSize const& imgSize, 
                                        MINT const imgFormat = eImgFmt_BLOB,
                                        MINT32 const bitstreamSize = 0
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Heap Info.
    IHalMemory*                     mpHalMemory;
    mutable Mutex                   mInitMtx;
    mutable Mutex                   mLockMtx;
    MINT32 volatile                 mLockCount;
    MINT32                          mLockUsage;
    BufInfoVect_t                   mvBufInfo;

private:    ////                    Image Attributes.
    String8                         mCallerName;
    MSize                           mImgSize;
    MINT                            mImgFormat;
    MUINT                           mPlaneCount;
    MINT32                          mBitstreamSize; // in bytes

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSImageBufferHeap
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_BASEIMAGEBUFFERHEAP_H_

