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
#if defined(MTK_ION_SUPPORT)
#define LOG_TAG "MtkCam/IonImageBufferHeap"
//
#include "MyUtils.h"
#include <mtkcam/utils/IonImageBufferHeap.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
//
#include <ion/ion.h>
#include <sys/mman.h>
#include <asm/cache.h>
//


/******************************************************************************
 *
 ******************************************************************************/
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


/******************************************************************************
 *
 ******************************************************************************/
IonImageBufferHeap*
IonImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam, 
    AllocExtraParam const& rExtraParam
)
{
    MUINT const planeCount = Format::queryPlaneCount(rImgParam.imgFormat);
#if 1
    for (MUINT i = 0; i < planeCount; i++)
    {
        CAM_LOGW_IF(
            0!=(rImgParam.bufBoundaryInBytes[i]%L1_CACHE_BYTES), 
            "BoundaryInBytes[%d]=%d is not a multiple of %d", 
            i, rImgParam.bufBoundaryInBytes[i], L1_CACHE_BYTES
        );
    }
#endif
    //
    IonImageBufferHeap* pHeap = NULL;
    pHeap = new IonImageBufferHeap(szCallerName, rImgParam, rExtraParam);
    if  ( ! pHeap )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat) )
    {
        CAM_LOGE("onCreate");
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IonImageBufferHeap::
IonImageBufferHeap(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam, 
    AllocExtraParam const& rExtraParam
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mExtraParam(rExtraParam)
    //
    , mIonDevFD(-1)
    , mvHeapInfo()
    , mvBufInfo()
    //
{
    MY_LOGD("");
    ::memcpy(mBufStridesInPixelsToAlloc, rImgParam.bufStridesInPixels, sizeof(mBufStridesInPixelsToAlloc));
    ::memcpy(mBufBoundaryInBytesToAlloc, rImgParam.bufBoundaryInBytes, sizeof(mBufBoundaryInBytesToAlloc));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    MBOOL ret = MFALSE;
    //
    mIonDevFD = ::ion_open();
    if  ( 0 > mIonDevFD )
    {
        MY_LOGE("ion_open() return %d", mIonDevFD);
        goto lbExit;
    }
    //
    //  Allocate memory and setup mBufHeapInfo & rBufHeapInfo.
    //  Allocated memory of each plane is not contiguous.
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        if  ( ! helpCheckBufStrides(i, mBufStridesInPixelsToAlloc[i]) )
        {
            goto lbExit;
        }
        //
        {
            sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            //
            sp<MyBufInfo> pBufInfo = new MyBufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInPixels = mBufStridesInPixelsToAlloc[i];
            pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInPixelsToAlloc[i]);
            pBufInfo->iBoundaryInBytesToAlloc = mBufBoundaryInBytesToAlloc[i];
            //
            if  ( ! doAllocIon(*pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("doAllocIon");
                goto lbExit;
            }
            //
            //  setup return buffer information.
            rvBufInfo[i]->stridesInPixels = pBufInfo->stridesInPixels;
            rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
        {
            sp<MyHeapInfo> pHeapInfo = mvHeapInfo[i];
            sp<MyBufInfo> pBufInfo = mvBufInfo[i];
            //
            doDeallocIon(*pHeapInfo, *pBufInfo);
        }
        //
        if  ( 0 <= mIonDevFD )
        {
            ::ion_close(mIonDevFD);
            mIonDevFD = -1;
        }
    }
    MY_LOGD_IF(1, "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impUninit(BufInfoVect_t const& rvBufInfo)
{
    for (MUINT32 i = 0; i < mvBufInfo.size(); i++)
    {
        sp<MyHeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<MyBufInfo> pBufInfo = mvBufInfo[i];
        //
        doDeallocIon(*pHeapInfo, *pBufInfo);
    }
    //
    if  ( 0 <= mIonDevFD )
    {
        ::ion_close(mIonDevFD);
        mIonDevFD = -1;
    }
    //
    MY_LOGD_IF(1, "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doAllocIon(MyHeapInfo& rHeapInfo, MyBufInfo& rBufInfo)
{
    int err = 0;
    //
    //  ion_alloc_mm: buf handle
    err = ::ion_alloc_mm(mIonDevFD, rBufInfo.sizeInBytes, rBufInfo.iBoundaryInBytesToAlloc, 0, &rHeapInfo.pIonHandle);
    if  ( 0 != err )
    {
        MY_LOGE("ion_alloc_mm returns %d", err);
        goto lbExit;
    }
    //
    //  ion_share: buf handle -> buf fd
    err = ::ion_share(mIonDevFD, rHeapInfo.pIonHandle, &rHeapInfo.heapID);
    if  ( 0 != err || -1 == rHeapInfo.heapID )
    {
        MY_LOGE("ion_share returns %d, BufFD:%d", err, rHeapInfo.heapID);
        goto lbExit;
    }
    //
    //  ion_mmap: buf fd -> virtual address (NON-Cachable)
#if 0
    rBufInfo.va = (MUINT32)::ion_mmap(
        mIonDevFD, NULL, rBufInfo.sizeInBytes, 
        PROT_READ|PROT_WRITE|PROT_NOCACHE, 
        MAP_SHARED, rHeapInfo.heapID, 0
    );
    if  ( 0 == rBufInfo.va || -1 == rBufInfo.va )
    {
        MY_LOGE(
            "ion_mmap returns %d - DevFD:%d BufFD:%d BufSize:%#x", 
            rBufInfo.va, mIonDevFD, rHeapInfo.heapID, rBufInfo.sizeInBytes
        );
        goto lbExit;
    }
#endif
    //
    //
    return  MTRUE;
lbExit:
    return  MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
IonImageBufferHeap::
doDeallocIon(MyHeapInfo& rHeapInfo, MyBufInfo& rBufInfo)
{
    //  ion_munmap: virtual address
#if 0
    if  ( 0 != rBufInfo.va )
    {
        ::ion_munmap(mIonDevFD, (void *)rBufInfo.va, rBufInfo.sizeInBytes);
        rBufInfo.va = 0;
    }
#endif
    //
    //  ion_share_close: buf fd
    if  ( 0 <= rHeapInfo.heapID )
    {
        ::ion_share_close(mIonDevFD, rHeapInfo.heapID);
        rHeapInfo.heapID = -1;
    }
    //
    //  ion_free: buf handle
    if  ( NULL != rHeapInfo.pIonHandle )
    {
        ::ion_free(mIonDevFD, rHeapInfo.pIonHandle);
        rHeapInfo.pIonHandle = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impLockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    MBOOL ret = MFALSE;
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<MyHeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            int prot_flag = mExtraParam.nocache ? PROT_NOCACHE : 0;
            if  ( 0 != (usage & eBUFFER_USAGE_SW_READ_MASK) ) { prot_flag |= PROT_READ; }
            if  ( 0 != (usage & eBUFFER_USAGE_SW_WRITE_MASK) ){ prot_flag |= PROT_WRITE; }
            pBufInfo->va = (MUINT32)::ion_mmap(mIonDevFD, NULL, pBufInfo->sizeInBytes, prot_flag, MAP_SHARED, pHeapInfo->heapID, 0);
            if  ( 0 == pBufInfo->va || -1 == pBufInfo->va )
            {
                MY_LOGE(
                    "ion_mmap returns %d - DevFD:%d BufFD:%d BufSize:%#x", 
                    pBufInfo->va, mIonDevFD, pHeapInfo->heapID, pBufInfo->sizeInBytes
                );
                goto lbExit;
            }
        }
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( ! doMapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("%s@ doMapPhyAddr at %d-th plane", szCallerName, i);
                goto lbExit;
            }
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        impUnlockBuf(szCallerName, usage, rvBufInfo);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impUnlockBuf(
    char const* szCallerName, 
    MINT usage, 
    BufInfoVect_t const& rvBufInfo
)
{
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<MyHeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
                doUnmapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo);
                pBufInfo->pa = 0;
            }
            else {
                MY_LOGW("%s@ skip PA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  ( 0 != pBufInfo->va ) {
                ::ion_munmap(mIonDevFD, (void *)pBufInfo->va, pBufInfo->sizeInBytes);
                pBufInfo->va = 0;
            }
            else {
                MY_LOGW("%s@ skip VA=0 at %d-th plane", szCallerName, i);
            }
        }
    }
    //
#if 0
    //  SW Write + Cacheable Memory => Flush Cache.
    if  ( 0!=(usage & eBUFFER_USAGE_SW_WRITE_MASK) && 0==mExtraParam.nocache )
    {
        doFlushCache();
    }
#endif
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doMapPhyAddr(char const* szCallerName, MyHeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = 0;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpMapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpMapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = param.phyAddr;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doUnmapPhyAddr(char const* szCallerName, MyHeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = rBufInfo.pa;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpUnmapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpUnmapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = 0;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doFlushCache()
{
    Vector<HelperParamFlushCache> vParam;
    vParam.insertAt(0, mvHeapInfo.size());
    HelperParamFlushCache*const aParam = vParam.editArray();
    for (MUINT i = 0; i < vParam.size(); i++)
    {
        aParam[i].virAddr = mvBufInfo[i]->va;
        aParam[i].ionFd   = mvHeapInfo[i]->heapID;
        aParam[i].size    = mvBufInfo[i]->sizeInBytes;
    }
    if  ( ! helpFlushCache(aParam, vParam.size()) )
    {
        MY_LOGE("helpFlushCache");
        return  MFALSE;
    }
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
#endif  //MTK_ION_SUPPORT

