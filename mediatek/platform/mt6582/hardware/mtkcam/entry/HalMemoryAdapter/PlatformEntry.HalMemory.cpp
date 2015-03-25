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

#define LOG_TAG "MtkCam/PlatformEntry"
//
#include <utils/Mutex.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/hal/IHalMemory.h>
#include "../PlatformEntry.h"
using namespace NSCam;
//
#include <mtkcam/drv/imem_drv.h>
//
using namespace android;
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
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
namespace
{
class HalMemoryAdapter : public IHalMemory
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    mutable Mutex                   mOpenLock;
    MINT32                          miOpenCount;
    IMemDrv*                        mpMemDrv;

public:     ////                    Instantiation.
                                    HalMemoryAdapter();

    virtual MBOOL                   init(char const* szCallerName);
    virtual MBOOL                   uninit(char const* szCallerName);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHalMemory Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    static  IHalMemory*             createInstance(char const* szCallerName);
    virtual MVOID                   destroyInstance(char const* szCallerName);

    virtual MBOOL                   mapPA(
                                        char const* szCallerName, 
                                        Info* pInfo
                                    );
    virtual MBOOL                   unmapPA(
                                        char const* szCallerName, 
                                        Info const* pInfo
                                    );

    virtual MBOOL                   flushCache(
                                        Info const* pInfo, 
                                        MUINT const num
                                    );

    virtual MBOOL                   flushAllCache();

};


/******************************************************************************
 *
 ******************************************************************************/
HalMemoryAdapter    gHalMemoryAdapter;


};


/******************************************************************************
 *
 ******************************************************************************/
IHalMemory*
IHalMemory::
createInstance(char const* szCallerName)
{
    if  ( ! gHalMemoryAdapter.init(szCallerName) )
    {
        return  NULL;
    }
    return  &gHalMemoryAdapter;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalMemoryAdapter::
destroyInstance(char const* szCallerName)
{
    gHalMemoryAdapter.uninit(szCallerName);
}


/******************************************************************************
 *
 ******************************************************************************/
HalMemoryAdapter::
HalMemoryAdapter()
    : mOpenLock()
    , miOpenCount(0)
    , mpMemDrv(NULL)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
init(char const* szCallerName)
{
    Mutex::Autolock _l(mOpenLock);
    //
    if  ( 0 == miOpenCount )
    {
        mpMemDrv = IMemDrv::createInstance();
        if  ( ! mpMemDrv )
        {
            MY_LOGE("s@ IMemDrv::createInstance()", szCallerName);
            return  MFALSE;
        }
        //
        if  ( ! mpMemDrv->init() )
        {
            MY_LOGE("s@ IMemDrv::init()", szCallerName);
            return  MFALSE;
        }
    }
    //
    miOpenCount++;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
uninit(char const* szCallerName)
{
    Mutex::Autolock _l(mOpenLock);
    //
    if  ( 1 == miOpenCount )
    {
        if  ( ! mpMemDrv->uninit() )
        {
            MY_LOGE("%s@ IMemDrv::uninit()", szCallerName);
            return  MFALSE;
        }
        //
        mpMemDrv->destroyInstance();
        mpMemDrv = NULL;
    }
    //
    miOpenCount--;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
flushAllCache()
{
    Mutex::Autolock _l(mOpenLock);
    //
    if  ( ! mpMemDrv )
    {
        MY_LOGE("NULL mpMemDrv; OpenCount:%d", miOpenCount);
        return  MFALSE;
    }
    //
    return  0==mpMemDrv->cacheFlushAll();
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
flushCache(
    Info const* pInfo, 
    MUINT const num
)
{
#if 1
    return  flushAllCache();
#else
    for (MUINT i = 0; i < num; i++)
    {
        pInfo[i];
    }
    return  MTRUE;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
mapPA(
    char const* szCallerName, 
    Info* pInfo
)
{
    IMEM_BUF_INFO INFO;
    INFO.size       = pInfo->size;
    INFO.memID      = pInfo->ionFd;
    INFO.virtAddr   = pInfo->va;
    INFO.phyAddr    = pInfo->pa;
    INFO.bufSecu    = pInfo->security;
    INFO.bufCohe    = pInfo->coherence;
    INFO.useNoncache= 0;
    //
    //
    Mutex::Autolock _l(mOpenLock);
    //
    if  ( ! mpMemDrv )
    {
        MY_LOGE("%s@ NULL mpMemDrv; OpenCount:%d", szCallerName, miOpenCount);
        return  MFALSE;
    }
    //
    if  (  0 != mpMemDrv->mapPhyAddr(&INFO) )
    {
        MY_LOGE("%s@ mpMemDrv->mapPhyAddr()", szCallerName);
        return  MFALSE;
    }
    //
    pInfo->pa = INFO.phyAddr;
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalMemoryAdapter::
unmapPA(
    char const* szCallerName, 
    Info const* pInfo
)
{
    IMEM_BUF_INFO INFO;
    INFO.size       = pInfo->size;
    INFO.memID      = pInfo->ionFd;
    INFO.virtAddr   = pInfo->va;
    INFO.phyAddr    = pInfo->pa;
    INFO.bufSecu    = pInfo->security;
    INFO.bufCohe    = pInfo->coherence;
    INFO.useNoncache= 0;
    //
    //
    Mutex::Autolock _l(mOpenLock);
    //
    if  ( ! mpMemDrv )
    {
        MY_LOGE("%s@ NULL mpMemDrv; OpenCount:%d", szCallerName, miOpenCount);
        return  MFALSE;
    }
    //
    if  (  0 != mpMemDrv->unmapPhyAddr(&INFO) )
    {
        MY_LOGE("%s@ mpMemDrv->unmapPhyAddr()", szCallerName);
        return  MFALSE;
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
IHalMemory*
PlatformEntry::
createHalMemory(char const* szCallerName)
{
    return  IHalMemory::createInstance(szCallerName);
}

