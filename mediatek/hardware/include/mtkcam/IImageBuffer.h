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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_IIMAGEBUFFER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_IIMAGEBUFFER_H_
//
#include <hardware/gralloc.h>
#include "common.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Image Buffer Usage.
 ******************************************************************************/
enum
{
    /* buffer is rarely read in software */
    eBUFFER_USAGE_SW_READ_RARELY        = GRALLOC_USAGE_SW_READ_RARELY, 
    /* buffer is often read in software */
    eBUFFER_USAGE_SW_READ_OFTEN         = GRALLOC_USAGE_SW_READ_OFTEN, 
    /* mask for the software read values */
    eBUFFER_USAGE_SW_READ_MASK          = GRALLOC_USAGE_SW_READ_MASK, 

    /* buffer is rarely written in software */
    eBUFFER_USAGE_SW_WRITE_RARELY       = GRALLOC_USAGE_SW_WRITE_RARELY, 
    /* buffer is often written in software */
    eBUFFER_USAGE_SW_WRITE_OFTEN        = GRALLOC_USAGE_SW_WRITE_OFTEN, 
    /* mask for the software write values */
    eBUFFER_USAGE_SW_WRITE_MASK         = GRALLOC_USAGE_SW_WRITE_MASK, 

    /* mask for the software access */
    eBUFFER_USAGE_SW_MASK               = eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_SW_WRITE_MASK, 

    /* buffer will be used as an OpenGL ES texture (read by GPU) */
    eBUFFER_USAGE_HW_TEXTURE            = GRALLOC_USAGE_HW_TEXTURE, 
    /* buffer will be used as an OpenGL ES render target (written by GPU) */
    eBUFFER_USAGE_HW_RENDER             = GRALLOC_USAGE_HW_RENDER, 

    /* buffer will be read by the HW camera pipeline */
    eBUFFER_USAGE_HW_CAMERA_READ        = GRALLOC_USAGE_HW_CAMERA_READ, 
    /* buffer will be written by the HW camera pipeline */
    eBUFFER_USAGE_HW_CAMERA_WRITE       = GRALLOC_USAGE_HW_CAMERA_WRITE, 
    /* buffer will be read and written by the HW camera pipeline */
    eBUFFER_USAGE_HW_CAMERA_READWRITE   = eBUFFER_USAGE_HW_CAMERA_WRITE | eBUFFER_USAGE_HW_CAMERA_READ, 

    /* mask for the hardware access */
    eBUFFER_USAGE_HW_MASK               = GRALLOC_USAGE_HW_MASK, 

};


/******************************************************************************
 *  Image Buffer Interface.
 ******************************************************************************/
class IImageBuffer
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Disallowed.
    /**
     * Copy constructor and Copy assignment are disallowed.
     */
                                    IImageBuffer(IImageBuffer const&);
    IImageBuffer&                   operator=(IImageBuffer const&);

protected:  ////                    Destructor.
                                    IImageBuffer() {}

    //  Disallowed to directly delete a raw pointer.
    virtual                         ~IImageBuffer() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Reference Counting.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Reference Counting.
    virtual MVOID                   incStrong(MVOID const* id)          const   = 0;
    virtual MVOID                   decStrong(MVOID const* id)          const   = 0;
    virtual MINT32                  getStrongCount()                    const   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Image Attributes.
    virtual MINT                    getImgFormat()                      const   = 0;
    virtual MSize const&            getImgSize()                        const   = 0;
    virtual MBOOL                   setImgSize(MSize const imagesize)           = 0;
    virtual MINT                    getImgBitsPerPixel()                const   = 0;
    virtual MINT                    getPlaneBitsPerPixel(MUINT index)   const   = 0;
    virtual MUINT                   getPlaneCount()                     const   = 0;
    virtual MINT32                  getBitstreamSize()                   const   = 0;
    virtual MBOOL                   setBitstreamSize(MINT32 const bitstreamsize)  = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Buffer Attributes.

    /**
     * Return a pointer to a null-terminated string to indicate a magic name of 
     * buffer type.
     */
    virtual char const*             getMagicName()                      const   = 0;

    /**
     * Buffer offset in bytes of a given plane.
     * Buf VA(i) = Buf Offset(i) + Heap VA(i)
     */
    virtual MUINT32                 getBufOffsetInBytes(MUINT index)    const   = 0;

    /**
     * Buffer physical address of a given plane;
     * legal only after lockBuf() with a HW usage.
     */
    virtual MINT32                  getBufPA(MUINT index)               const   = 0;

    /**
     * Buffer virtual address of a given plane; 
     * legal only after lockBuf() with a SW usage.
     */
    virtual MINT32                  getBufVA(MUINT index)               const   = 0;

    /**
     * Buffer size in bytes of a given plane; always legal.
     *
     * buffer size in bytes = buffer size in pixels x getPlaneBitsPerPixel(index)
     *
     * buffer size in pixels = buffer width stride in pixels
     *                       x(buffer height stride in pixels - 1)
     *                       + image width in pixels
     *  
     * +---------+---------+---------+----------+
     * | Heap Pixel Array                       |
     * |                    O ROI Image         |
     * |                    = Buffer Size       |
     * |                                        |
     * |                    OOOOOOOOO===========|
     * |====================OOOOOOOOO===========|
     * |====================OOOOOOOOO===========|
     * |====================OOOOOOOOO           |
     * |                                        |
     * |                                        |
     * +---------+---------+---------+----------+
     *
     */
    virtual MUINT32                 getBufSizeInBytes(MUINT index)      const   = 0;

    /**
     * Buffer Strides in bytes of a given plane; always legal.
     */
    virtual MSize                   getBufStridesInBytes(MUINT index)   const   = 0;

    /**
     * Buffer Strides in pixels of a given plane; always legal.
     */
    virtual MSize                   getBufStridesInPixels(MUINT index)  const   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    /**
     * A buffer is allowed to access only between the interval of lockBuf() and 
     * unlockBuf(). Call lockBuf() with a usage flag before accessing a buffer, 
     * and call unlockBuf() after finishing accessing it.
     *
     * Physical address of a buffer, from getBufPA(), is legal only if a HW 
     * usage is specified when lockBuf().
     *
     * Virtual address of a buffer, from getBufVA(), is legal only if a SW usage
     * is specified when lockBuf().
     *
     * Physical and virtual addresses are legal if HW and SW usages are 
     * specified when lockBuf().
     *
     */
    virtual MBOOL                   lockBuf(
                                        char const* szCallerName, 
                                        MINT usage = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN
                                    )                                           = 0;
    virtual MBOOL                   unlockBuf(
                                        char const* szCallerName
                                    )                                           = 0;

public:     ////                    Cache.
    virtual MBOOL                   flushAllCache()                             = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  File Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    File Operations.
    virtual MBOOL                   saveToFile(char const* filepath)            = 0;
    virtual MBOOL                   loadFromFile(char const* filepath)          = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Timestamp Accesssors.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Timestamp Accesssors.
    virtual MINT64                  getTimestamp()                      const   = 0;
    virtual MVOID                   setTimestamp(MINT64 const timestamp)        = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Fence Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Fence Operations.
    /**
     * The acquire sync fence for this buffer. The HAL must wait on this fence
     * fd before attempting to read from or write to this buffer.
     *
     * The framework may be set to -1 to indicate that no waiting is necessary
     * for this buffer.
     *
     * When the HAL returns an output buffer to the framework with
     * process_capture_result(), the acquire_fence must be set to -1. If the HAL
     * never waits on the acquire_fence due to an error in filling a buffer,
     * when calling process_capture_result() the HAL must set the release_fence
     * of the buffer to be the acquire_fence passed to it by the framework. This
     * will allow the framework to wait on the fence before reusing the buffer.
     *
     * For input buffers, the HAL must not change the acquire_fence field during
     * the process_capture_request() call.
     */
    virtual MINT                    getAcquireFence()                   const   = 0;
    virtual MVOID                   setAcquireFence(MINT fence)                 = 0;

    /**
     * The release sync fence for this buffer. The HAL must set this fence when
     * returning buffers to the framework, or write -1 to indicate that no
     * waiting is required for this buffer.
     *
     * For the input buffer, the release fence must be set by the
     * process_capture_request() call. For the output buffers, the fences must
     * be set in the output_buffers array passed to process_capture_result().
     */
    virtual MINT                    getReleaseFence()                   const   = 0;
    virtual MVOID                   setReleaseFence(MINT fence)                 = 0;

};


/******************************************************************************
 *  Image Buffer Heap Interface.
 ******************************************************************************/
class IImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Disallowed.
    /**
     * Copy assignment are disallowed.
     */
    IImageBufferHeap&               operator=(IImageBufferHeap const&);

protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IImageBufferHeap() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Reference Counting.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Reference Counting.
    virtual MVOID                   incStrong(MVOID const* id)          const   = 0;
    virtual MVOID                   decStrong(MVOID const* id)          const   = 0;
    virtual MINT32                  getStrongCount()                    const   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Image Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Image Attributes.
    virtual MINT                    getImgFormat()                      const   = 0;
    virtual MSize const&            getImgSize()                        const   = 0;
    virtual MINT                    getImgBitsPerPixel()                const   = 0;
    virtual MINT                    getPlaneBitsPerPixel(MUINT index)   const   = 0;
    virtual MUINT                   getPlaneCount()                     const   = 0;
    virtual MINT32                  getBitstreamSize()                   const   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Buffer Attributes.

    /**
     * Return a pointer to a null-terminated string to indicate a magic name of 
     * buffer type.
     */
    virtual char const*             getMagicName()                      const   = 0;

    /**
     * Heap ID could be ION fd, PMEM fd, and so on.
     * Legal only after lockBuf().
     */
    virtual MINT32                  getHeapID(MUINT index = 0)          const   = 0;

    /**
     * 0 <= Heap ID count <= plane count.
     * Legal only after lockBuf().
     */
    virtual MINT32                  getHeapIDCount()                    const   = 0;

    /**
     * Buffer physical address of a given plane;
     * legal only after lockBuf() with a HW usage.
     */
    virtual MINT32                  getBufPA(MUINT index)               const   = 0;

    /**
     * Buffer virtual address of a given plane; 
     * legal only after lockBuf() with a SW usage.
     */
    virtual MINT32                  getBufVA(MUINT index)               const   = 0;

    /**
     * Buffer size in bytes of a given plane; always legal.
     */
    virtual MUINT32                 getBufSizeInBytes(MUINT index)      const   = 0;

    /**
     * Buffer Strides in bytes of a given plane; always legal.
     */
    virtual MSize                   getBufStridesInBytes(MUINT index)   const   = 0;

    /**
     * Buffer Strides in pixels of a given plane; always legal.
     */
    virtual MSize                   getBufStridesInPixels(MUINT index)  const   = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    /**
     * A buffer is allowed to access only between the interval of lockBuf() and 
     * unlockBuf(). Call lockBuf() with a usage flag before accessing a buffer, 
     * and call unlockBuf() after finishing accessing it.
     *
     * Physical address of a buffer, from getBufPA(), is legal only if a HW 
     * usage is specified when lockBuf().
     *
     * Virtual address of a buffer, from getBufVA(), is legal only if a SW usage
     * is specified when lockBuf().
     *
     * Physical and virtual addresses are legal if HW and SW usages are 
     * specified when lockBuf().
     *
     */
    virtual MBOOL                   lockBuf(
                                        char const* szCallerName, 
                                        MINT usage = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN
                                    )                                           = 0;
    virtual MBOOL                   unlockBuf(
                                        char const* szCallerName
                                    )                                           = 0;

public:     ////                    Cache.
    virtual MBOOL                   flushAllCache()                             = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBuffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    /**
     * Create an IImageBuffer instance with its ROI equal to the image full 
     * resolution of this heap.
     */
    virtual IImageBuffer*           createImageBuffer()                    = 0;

    /**
     * This call is legal only if the heap format is blob.
     * 
     * From the given blob heap, create an IImageBuffer instance with a specified
     * offset and size, and its format equal to blob.
     */
    virtual IImageBuffer*           createImageBuffer_FromBlobHeap(
                                        MUINT32     offsetInBytes, 
                                        MINT32      sizeInBytes
                                    )                                      = 0;

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
                                    )                                      = 0;

    /**
     * Create an IImageBuffer instance indicating the left-side or right-side 
     * buffer within a side-by-side image.
     * 
     * Left side if isRightSide = 0; otherwise right side.
     */
    virtual IImageBuffer*           createImageBuffer_SideBySide(
                                        MBOOL       isRightSide
                                    )                                      = 0;

    /**
     * Create an IImageBuffer instance with a specified ROI in pixels.
     * Image ROI is defined as a rectangle (x, y, w, h), with (x, y) describing 
     * the top-left pixel of the image.  Both w and h cannot be larger than the 
     * image resolution of this heap.
     */
    virtual IImageBuffer*           createImageBuffer(
                                        MRect const&imgROI
                                    )                                      = 0;

};


/******************************************************************************
 *  Image Buffer Allocator Interface.
 ******************************************************************************/
class IImageBufferAllocator
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for Allocations.

                                    struct ImgParam
                                    {
                                    MINT32  imgFormat;              // image format.
                                    MSize   imgSize;                // image resolution in pixels.
                                    MSize   jpgSize;                // jpeg resolution in pixels. for JPEG memory.
                                    MINT32  bufSize;                // buffer size in bytes. for JPEG memory.
                                    MSize   bufStridesInPixels[3];  // buffer strides in pixels.
                                    MINT32  bufBoundaryInBytes[3];  // the address will be a multiple of boundary in bytes, which must be a power of two.
                                            //
                                            //  Image.
                                            ImgParam(
                                                MINT32 const    _imgFormat, 
                                                MSize const&    _imgSize, 
                                                MSize const     _bufStridesInPixels[], 
                                                MINT32 const    _bufBoundaryInBytes[], 
                                                MUINT const     _planeCount
                                            )
                                                : imgFormat(_imgFormat)
                                                , imgSize(_imgSize)
                                                , jpgSize(_imgSize)
                                                , bufSize(0)
                                            {
                                                for (MUINT i = 0; i < _planeCount; i++)
                                                {
                                                    bufStridesInPixels[i] = _bufStridesInPixels[i];
                                                    bufBoundaryInBytes[i] = _bufBoundaryInBytes[i];
                                                }
                                            }
                                            //
                                            //  BLOB memory.
                                            ImgParam(
                                                MINT32 const    _bufSize,               // buffer size in bytes. 
                                                MINT32 const    _bufBoundaryInBytes     // the address will be a multiple of boundary in bytes, which must be a power of two.s
                                            )
                                                : imgFormat(eImgFmt_BLOB)
                                                , imgSize(MSize(_bufSize, 1))
                                                , jpgSize(MSize(_bufSize, 1))
                                                , bufSize(_bufSize)
                                            {
                                                bufStridesInPixels[0] = MSize(_bufSize, 1);
                                                bufBoundaryInBytes[0] = _bufBoundaryInBytes;
                                            }
                                            //
                                            //  JPEG memory.
                                            ImgParam(
                                                MSize const&    _imgSize,               // image resolution in pixels.
                                                MINT32 const    _bufSize,               // buffer size in bytes.
                                                MINT32 const    _bufBoundaryInBytes     // the address will be a multiple of boundary in bytes, which must be a power of two.
                                            )
                                                : imgFormat(eImgFmt_JPEG)
                                                , imgSize(MSize(_bufSize, 1))
                                                , jpgSize(_imgSize)
                                                , bufSize(_bufSize)
                                            {
                                                bufStridesInPixels[0] = MSize(_bufSize, 1);
                                                bufBoundaryInBytes[0] = _bufBoundaryInBytes;
                                            }
                                    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    static IImageBufferAllocator*   getInstance();

    IImageBuffer*                   alloc(char const* szCallerName, ImgParam const& rImgParam);

    IImageBuffer*                   alloc_ion(char const* szCallerName, ImgParam const& rImgParam);

    IImageBuffer*                   alloc_pmem(char const* szCallerName, ImgParam const& rImgParam);

    MVOID                           free(IImageBuffer* pImageBuffer);

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_IIMAGEBUFFER_H_

