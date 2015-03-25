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

#define LOG_TAG "MtkCam/BaseImageBufferHeap"
//
#include "MyUtils.h"
#include <mtkcam/IPlatform.h>
#include <mtkcam/hal/IHalMemory.h>
#include <mtkcam/utils/BaseImageBufferHeap.h>
#include "BaseImageBuffer.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::NSImageBufferHeap;
//
#include <dlfcn.h>


/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT_FOR_IMGBUF
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
    //
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
    
#else
    
#include "uvvf.h"
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s::%s] "fmt, getMagicName(), __FUNCTION__, ##arg)
    //
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#endif

/******************************************************************************
 *
 ******************************************************************************/
BaseImageBufferHeap::
~BaseImageBufferHeap()
{
}


/******************************************************************************
 *
 ******************************************************************************/
BaseImageBufferHeap::
BaseImageBufferHeap(char const* szCallerName)
    : IImageBufferHeap()
    //
    , mpHalMemory(NULL)
    , mInitMtx()
    , mLockMtx()
    , mLockCount(0)
    , mLockUsage(0)
    , mvBufInfo()
    //
    , mCallerName(szCallerName)
    , mImgSize(0)
    , mImgFormat(0)
    , mPlaneCount(0)
    , mBitstreamSize(0)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
BaseImageBufferHeap::
onLastStrongRef(const void* id)
{
    MY_LOGD_IF(1, "[%s] this:%p %dx%d format:%#x", mCallerName.string(), this, mImgSize.w, mImgSize.h, mImgFormat);
    //
    Mutex::Autolock _l(mInitMtx);
    uninitLocked();

    if  ( 0 != mLockCount )
    {
        MY_LOGE("Not unlock before release heap - LockCount:%d", mLockCount);
        dumpCallStack(__FUNCTION__);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
onCreate(
    MSize const& imgSize, 
    MINT const imgFormat,
    MINT32 const bitstreamSize
)
{
    MY_LOGD_IF(1, "[%s] this:%p %dx%d format:%#x", mCallerName.string(), this, imgSize.w, imgSize.h, imgFormat);
    //
    Mutex::Autolock _l(mInitMtx);
    //
    mImgSize = imgSize;
    mImgFormat = imgFormat;
    mBitstreamSize = bitstreamSize;
    mPlaneCount = Format::queryPlaneCount(imgFormat);
    //
    return  initLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
BaseImageBufferHeap::
getPlaneBitsPerPixel(MUINT index) const
{
    return  Format::queryPlaneBitsPerPixel(getImgFormat(), index);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
BaseImageBufferHeap::
getImgBitsPerPixel() const
{
    return  Format::queryImageBitsPerPixel(getImgFormat());
}


/******************************************************************************
 * Heap ID could be ION fd, PMEM fd, and so on.
 * Legal only after lock().
 ******************************************************************************/
MINT32
BaseImageBufferHeap::
getHeapID(MUINT index) const
{
    Mutex::Autolock _l(mLockMtx);
    //
    if  ( 0 >= mLockCount )
    {
        MY_LOGE("This call is legal only after lock()");
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    HeapInfoVect_t const& rvHeapInfo = impGetHeapInfo();
    if  ( index >= rvHeapInfo.size() )
    {
        MY_LOGE("Invalid index:%d >= %d", index, rvHeapInfo.size());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  rvHeapInfo[index]->heapID;
}


/******************************************************************************
 * 0 <= Heap ID count <= plane count.
 * Legal only after lock().
 ******************************************************************************/
MINT32
BaseImageBufferHeap::
getHeapIDCount() const
{
    Mutex::Autolock _l(mLockMtx);
    //
    if  ( 0 >= mLockCount )
    {
        MY_LOGE("This call is legal only after lock()");
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  impGetHeapInfo().size();
}


/******************************************************************************
 * Buffer physical address; legal only after lock() with HW usage.
 ******************************************************************************/
MINT32
BaseImageBufferHeap::
getBufPA(MUINT index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%d) >= PlaneCount(%d)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    if  (
        0 == mLockCount
    ||  0 == (mLockUsage & eBUFFER_USAGE_HW_MASK)
    )
    {
        MY_LOGE("This call is legal only after lockBuf() with HW usage - LockCount:%d Usage:%#x", mLockCount, mLockUsage);
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  mvBufInfo[index]->pa;
}


/******************************************************************************
 * Buffer virtual address; legal only after lock() with SW usage.
 ******************************************************************************/
MINT32
BaseImageBufferHeap::
getBufVA(MUINT index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%d) >= PlaneCount(%d)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    if  (
        0 == mLockCount
    ||  0 == (mLockUsage & eBUFFER_USAGE_SW_MASK)
    )
    {
        MY_LOGE("This call is legal only after lockBuf() with SW usage - LockCount:%d Usage:%#x", mLockCount, mLockUsage);
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  mvBufInfo[index]->va;
}


/******************************************************************************
 * Buffer size in bytes; always legal.
 ******************************************************************************/
MUINT32
BaseImageBufferHeap::
getBufSizeInBytes(MUINT index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%d) >= PlaneCount(%d)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return  mvBufInfo[index]->sizeInBytes;
}


/******************************************************************************
 * Buffer Strides in bytes; always legal.
 ******************************************************************************/
MSize
BaseImageBufferHeap::
getBufStridesInBytes(MUINT index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%d) >= PlaneCount(%d)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    MINT const planeBitsPerPixel = getPlaneBitsPerPixel(index);
    MSize const stridesInPixels = getBufStridesInPixels(index);
    return  MSize(
        (stridesInPixels.w * planeBitsPerPixel)>>3, 
        (stridesInPixels.h * planeBitsPerPixel)>>3
    );
}


/******************************************************************************
 * Buffer Strides in pixels; always legal.
 ******************************************************************************/
MSize
BaseImageBufferHeap::
getBufStridesInPixels(MUINT index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGE("Bad index(%d) >= PlaneCount(%d)", index, getPlaneCount());
        dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    //
    Mutex::Autolock _l(mLockMtx);
    //
    return  mvBufInfo[index]->stridesInPixels;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
lockBuf(char const* szCallerName, MINT usage)
{
    Mutex::Autolock _l(mLockMtx);
    return  lockBufLocked(szCallerName, usage);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
unlockBuf(char const* szCallerName)
{
    Mutex::Autolock _l(mLockMtx);
    return  unlockBufLocked(szCallerName);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
lockBufLocked(char const* szCallerName, MINT usage)
{
    MY_LOGF_IF(0<mLockCount, "%s@ Has locked - LockCount:%d", szCallerName, mLockCount);
    //
    if  ( ! impLockBuf(szCallerName, usage, mvBufInfo) )
    {
        MY_LOGE("%s@ impLockBuf() usage:#x", szCallerName, usage);
        return  MFALSE;
    }
    //
    //  Check Buffer Info.
    if  ( getPlaneCount() != mvBufInfo.size() )
    {
        MY_LOGE("%s@ BufInfo.size(%d) != PlaneCount(%d)", szCallerName, mvBufInfo.size(), getPlaneCount());
        return  MFALSE;
    }
    //
    for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
    {
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) && 0 == mvBufInfo[i]->va )
        {
            MY_LOGE("%s@ Bad result at %d-th plane: va=0 with SW usage:#x", szCallerName, i, usage);
            return  MFALSE;
        }
        //
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) && 0 == mvBufInfo[i]->pa )
        {
            MY_LOGE("%s@ Bad result at %d-th plane: pa=0 with HW usage:#x", szCallerName, i, usage);
            return  MFALSE;
        }
    }
    //
    mLockUsage = usage;
    mLockCount++;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
unlockBufLocked(char const* szCallerName)
{
    if  ( 0 == mLockCount )
    {
        MY_LOGW("%s@ Never lock", szCallerName);
        return  MFALSE;
    }
    //
    if  ( ! impUnlockBuf(szCallerName, mLockUsage, mvBufInfo) )
    {
        MY_LOGE("%s@ impUnlockBuf() usage:#x", szCallerName, mLockUsage);
        return  MFALSE;
    }
    //
    mLockUsage = 0;
    mLockCount--;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace
{
static Mutex        gMutex;
static MVOID*       gLibPlatform = NULL;
static IPlatform*   gIPlatform = NULL;

IPlatform*
getPlatform()
{
    Mutex::Autolock _l(gMutex);
    if  ( gIPlatform )
    {
        return  gIPlatform;
    }
    //
    char const szModulePath[] = "/system/lib/libcam_platform.so";
    char const szEntrySymbol[] = "getHandleToPlatform";
    void* pfnEntry = NULL;
    IPlatform* pIPlatform = NULL;
    //
    if  ( ! gLibPlatform )
    {
        gLibPlatform = ::dlopen(szModulePath, RTLD_NOW);
        if  ( ! gLibPlatform )
        {
            char const *err_str = ::dlerror();
            CAM_LOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
            goto lbExit;
        }
    }
    //
    pfnEntry = ::dlsym(gLibPlatform, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        CAM_LOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pIPlatform = reinterpret_cast<IPlatform*(*)()>(pfnEntry)();
    if  ( ! pIPlatform )
    {
        CAM_LOGE("No hardware instance");
        goto lbExit;
    }
    //
    gIPlatform = pIPlatform;
    //
lbExit:
    //
    CAM_LOGD("%s():%p return %p in %s", szEntrySymbol, pfnEntry, gIPlatform, szModulePath);
    return gIPlatform;
}
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
initLocked()
{
    mvBufInfo.clear();
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        mvBufInfo.push_back(new BufInfo);
    }
    //
    if  ( ! impInit(mvBufInfo) )
    {
        MY_LOGE("%s@ impInit()", getMagicName());
        return  MFALSE;
    }
    //
    for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
    {
        if  ( ! mvBufInfo[i]->stridesInPixels )
        {
            MY_LOGE(
                "%s@ Bad result at %d-th plane: strides:%dx%d", 
                getMagicName(), i, mvBufInfo[i]->stridesInPixels.w, mvBufInfo[i]->stridesInPixels.h
            );
            return  MFALSE;
        }
    }
    //
    mpHalMemory = getPlatform()->createHalMemory(getMagicName());
    if  ( ! mpHalMemory )
    {
        MY_LOGE("%s@ createHalMemory()", getMagicName());
        uninitLocked();
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
uninitLocked()
{
    if  ( ! impUninit(mvBufInfo) )
    {
        MY_LOGE("%s@ impUninit()", getMagicName());
    }
    mvBufInfo.clear();
    //
    if  ( mpHalMemory )
    {
        mpHalMemory->destroyInstance(getMagicName());
        mpHalMemory = NULL;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
flushAllCache()
{
    Mutex::Autolock _l(mInitMtx);
    //
    if  ( ! mpHalMemory )
    {
        MY_LOGE("NULL MemHal");
        return  MFALSE;
    }
    //
    return  mpHalMemory->flushAllCache();
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* 
BaseImageBufferHeap::
createImageBuffer() 
{
    MSize bufStridesInPixels[3];
    for (MUINT32 i = 0; i < getPlaneCount(); ++i)
    {
        bufStridesInPixels[i] = getBufStridesInPixels(i);
    }
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, getImgSize(), getImgFormat(), getBitstreamSize(), bufStridesInPixels);
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* 
BaseImageBufferHeap::
createImageBuffer_FromBlobHeap(
    MUINT32     offsetInBytes, 
    MINT32      sizeInBytes
) 
{
    if ( getImgFormat() != eImgFmt_BLOB )
    {
        CAM_LOGE("Heap format(0x%x) is illegal.", getImgFormat());
        return NULL;
    }
    //
    MSize imgSize(sizeInBytes, getImgSize().h);
    MSize bufStridesInPixels[] = { MSize(sizeInBytes, 1), MSize(0, 0), MSize(0, 0) };
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, imgSize, getImgFormat(), getBitstreamSize(), bufStridesInPixels, offsetInBytes);
    //
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* 
BaseImageBufferHeap::
createImageBuffer_FromBlobHeap(
    MUINT32     offsetInBytes, 
    MINT32      imgFormat, 
    MSize const&imgSize, 
    MSize const bufStridesInPixels[3]
) 
{
    if ( getImgFormat() != eImgFmt_BLOB )
    {
        CAM_LOGE("Heap format(0x%x) is illegal.", getImgFormat());
        return NULL;
    }
    //
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, imgSize, imgFormat, getBitstreamSize(), bufStridesInPixels, offsetInBytes);
    //
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* 
BaseImageBufferHeap::
createImageBuffer_SideBySide(MBOOL isRightSide) 
{
    MSize SBSImgSize(getImgSize().w>>1, getImgSize().h);
    MINT32 offset = (isRightSide) ? getImgSize().w>>1 : 0;
    MSize bufStridesInPixels[3];
    for (MUINT32 i = 0; i < getPlaneCount(); ++i)
    {
        bufStridesInPixels[i].w = ( eImgFmt_BLOB == getImgFormat() ) ? getBufStridesInPixels(i).w>>1 : getBufStridesInPixels(i).w;
        bufStridesInPixels[i].h = getBufStridesInPixels(i).h;
    }
    //
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, SBSImgSize, getImgFormat(), getBitstreamSize(), bufStridesInPixels, offset);
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* 
BaseImageBufferHeap::
createImageBuffer(MRect const&imgROI)
{
    MSize roiImgSize(imgROI.width(), imgROI.height());
    MSize bufStridesInPixels[3];
    MINT32 roiBufSize = imgROI.size().size();   // roi buf size = roi.w * roi.h
    for (MUINT32 i = 0; i < getPlaneCount(); ++i)
    {
        bufStridesInPixels[i].w = ( eImgFmt_BLOB == getImgFormat() ) ? roiBufSize : getBufStridesInPixels(i).w;
        bufStridesInPixels[i].h = getBufStridesInPixels(i).h;
    }
    MINT32 offset = imgROI.leftTop().y*getBufStridesInPixels(0).w + imgROI.leftTop().x;
    //
    NSCam::NSImageBuffer::BaseImageBuffer* pImgBuffer = NULL;
    pImgBuffer = new NSCam::NSImageBuffer::BaseImageBuffer(this, roiImgSize, getImgFormat(), getBitstreamSize(), bufStridesInPixels, offset);
    if  ( ! pImgBuffer )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pImgBuffer->onCreate() )
    {
        CAM_LOGE("onCreate");
        delete pImgBuffer;
        return NULL;
    }
    //
    return pImgBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpMapPhyAddr(char const* szCallerName, HelperParamMapPA& rParam)
{
    MY_LOGA_IF(NULL==mpHalMemory, "NULL HalMemory");
    //
    IHalMemory::Info info;
    info.pa         = 0;
    info.va         = rParam.virAddr;
    info.ionFd      = rParam.ionFd;
    info.size       = rParam.size;
    info.security   = rParam.security;
    info.coherence  = rParam.coherence;
    //
    if  ( ! mpHalMemory->mapPA(szCallerName, &info) )
    {
        MY_LOGE("%s@ IHalMemory::mapPA", szCallerName);
        return  MFALSE;
    }
    //
    rParam.phyAddr = info.pa;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpUnmapPhyAddr(char const* szCallerName, HelperParamMapPA const& rParam)
{
    MY_LOGA_IF(NULL==mpHalMemory, "NULL HalMemory");
    //
    IHalMemory::Info info;
    info.pa         = rParam.phyAddr;
    info.va         = rParam.virAddr;
    info.ionFd      = rParam.ionFd;
    info.size       = rParam.size;
    info.security   = rParam.security;
    info.coherence  = rParam.coherence;
    //
    if  ( ! mpHalMemory->unmapPA(szCallerName, &info) )
    {
        MY_LOGE("%s@ IHalMemory::unmapPA", szCallerName);
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpFlushCache(
    HelperParamFlushCache const* paParam, 
    MUINT const num
)
{
    MY_LOGA_IF(NULL==paParam||0==num, "Bad arguments: %p %d", paParam, num);
    MY_LOGA_IF(NULL==mpHalMemory, "NULL HalMemory");
    //
    Vector<IHalMemory::Info> vInfo;
    vInfo.insertAt(0, num);
    IHalMemory::Info*const aInfo = vInfo.editArray();
    for (MUINT i = 0; i < num; i++)
    {
        aInfo[i].pa         = 0;
        aInfo[i].va         = paParam[i].virAddr;
        aInfo[i].ionFd      = paParam[i].ionFd;
        aInfo[i].size       = paParam[i].size;
        aInfo[i].security   = 0;
        aInfo[i].coherence  = 0;
    }
    //
    if  ( ! mpHalMemory->flushCache(aInfo, num) )
    {
        MY_LOGE("IHalMemory::flushCache");
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BaseImageBufferHeap::
helpCheckBufStrides(
    MUINT const planeIndex, 
    MSize const& planeBufStridesInPixels
) const
{
    MINT32 const planeImgWidthStrideInPixels = Format::queryPlaneWidthInPixels(getImgFormat(), planeIndex, getImgSize().w);
    MINT32 const planeImgHeightStrideInPixels= Format::queryPlaneHeightInPixels(getImgFormat(), planeIndex, getImgSize().h);
    //
    if  ( planeBufStridesInPixels.w < planeImgWidthStrideInPixels )
    {
        MY_LOGE(
            "[%dx%d image @ %d-th plane] Bad width stride in pixels: given buffer stride:%d < image stride:%d", 
            getImgSize().w, getImgSize().h, planeIndex, 
            planeBufStridesInPixels.w, planeImgWidthStrideInPixels
        );
        return  MFALSE;
    }
    //
    if  ( planeBufStridesInPixels.h < planeImgHeightStrideInPixels )
    {
        MY_LOGE(
            "[%dx%d image @ %d-th plane] Bad height stride in pixels: given buffer stride:%d < image stride:%d", 
            getImgSize().w, getImgSize().h, planeIndex, 
            planeBufStridesInPixels.h, planeImgHeightStrideInPixels
        );
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
BaseImageBufferHeap::
helpQueryBufSizeInBytes(
    MUINT const planeIndex, 
    MSize const& planeStridesInPixels
) const
{
    MY_LOGF_IF(planeIndex >= getPlaneCount(), "Bad index:%d >= PlaneCount:%d", planeIndex, getPlaneCount());
    //
    MUINT32 const sizeInPixels  = (planeStridesInPixels.w * planeStridesInPixels.h);
    MUINT32 const sizeInBits    = (Format::queryPlaneBitsPerPixel(getImgFormat(), planeIndex) * sizeInPixels);
    return  (sizeInBits>>3);    // size in bytes.
}

