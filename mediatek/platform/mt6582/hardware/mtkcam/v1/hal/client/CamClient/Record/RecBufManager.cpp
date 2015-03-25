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

#define LOG_TAG "MtkCam/CamClient/RecordClient"
//
#include <linux/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ion/ion.h>
#include <binder/MemoryHeapBase.h>
//
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include "RecBufManager.h"
//
using namespace NSCamClient;
using namespace NSRecordClient;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[RecBufManager::%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#if defined(MTK_ION_SUPPORT)
#define REC_BUF_ION     (1)
#else
#define REC_BUF_ION     (0)
#endif

/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf*
RecImgBuf::
alloc(
    camera_request_memory   requestMemory, 
    sp<RecImgInfo const>const& rpImgInfo
)
{
    bool ret = false;
    RecImgBuf* pRecImgBuf = NULL;
    camera_memory_t* camera_memory = NULL;
    int32_t IonDevFd = -1,IonBufFd = -1;
    struct ion_handle *pIonHandle = NULL;
    //
#if REC_BUF_ION
    //
    IonDevFd = ion_open();
    if(IonDevFd < 0)
    {
        CAM_LOGE("ion_open fail");
        goto lbExit;
    }
    //
    if(ion_alloc_mm(
        IonDevFd,
        rpImgInfo->mImgBufSize,
        32,
        0,
        &pIonHandle))
    {
        CAM_LOGE("ion_alloc_mm fail");
        goto lbExit;
    }
    //
    if(ion_share(
        IonDevFd,
        pIonHandle,
        &IonBufFd))
    {
        CAM_LOGE("ion_share fail");
        goto lbExit;
    }
    //
    camera_memory = requestMemory(IonBufFd, rpImgInfo->mImgBufSize, 1, NULL);
    if  ( ! camera_memory )
    {
        CAM_LOGE("[requestMemory] id:%d, size:%d", IonBufFd, rpImgInfo->mImgBufSize);
        goto lbExit;
    }
    //
#else
    //
    sp<MemoryHeapBase> pMemHeapBase = new MemoryHeapBase(rpImgInfo->mImgBufSize, 0, rpImgInfo->ms8ImgName);
    if  ( pMemHeapBase == 0 )
    {
        CAM_LOGE("[PrvCbImgBuf::alloc] cannot new MemoryHeapBase");
        goto lbExit;
    }
    //
    camera_memory = requestMemory(pMemHeapBase->getHeapID(), rpImgInfo->mImgBufSize, 1, NULL);
    if  ( ! camera_memory )
    {
        CAM_LOGE("[requestMemory] id:%d, size:%d", pMemHeapBase->getHeapID(), rpImgInfo->mImgBufSize);
        goto lbExit;
    }
    //
    pMemHeapBase = 0;
    //
#endif
    //
    pRecImgBuf = new RecImgBuf(
                            *camera_memory,
                            rpImgInfo,
                            IonDevFd,
                            IonBufFd,
                            pIonHandle,
                            rpImgInfo->mi4BufSecu,
                            rpImgInfo->mi4BufCohe);
lbExit:
    return  pRecImgBuf;
}

/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
RecImgBuf(
    camera_memory_t const&      rCamMem,
    sp<RecImgInfo const>const&  rpImgInfo, 
    int32_t                     IonDevFd,
    int32_t                     IonBufFd,
    struct ion_handle*          pIonHandle,
    int32_t                     bufSecu,
    int32_t                     bufCohe
)
    : ICameraImgBuf()
    , mpImgInfo(rpImgInfo)
    , mi8Timestamp(0)
    , mCamMem(rCamMem)
    , mIonDevFd(IonDevFd)
    , mIonBufFd(IonBufFd)
    , mpIonHandle(pIonHandle)
    , mi4BufSecu(bufSecu)
    , mi4BufCohe(bufCohe)
{
#if REC_BUF_ION
    CAM_LOGD("[RecImgBuf::RecImgBuf]ION");
#else
    CAM_LOGD("[RecImgBuf::RecImgBuf]M4U");
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
~RecImgBuf()
{
    struct ion_handle_data IonHandleData;
    //
    CAM_LOGD(
        "[RecImgBuf::~RecImgBuf]"
        "Name(%s),ION(%d),VA(0x%08X),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld)", 
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getBufSize(),
        getImgFormat().string(), 
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(), 
        getBitsPerPixel(),
        getTimestamp()
    );

    //
#if REC_BUF_ION
    //
    if(mIonDevFd >= 0)
    {
        if(mIonBufFd >= 0)
        {
            ion_share_close(
                mIonDevFd,
                mIonBufFd);
        }
        //
        ion_free(
            mIonDevFd,
            mpIonHandle);
        //
        ion_close(mIonDevFd);
    }
#endif
    if  ( mCamMem.release )
    {
        mCamMem.release(&mCamMem);
        mCamMem.release = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecImgBuf::
dump() const
{
    CAM_LOGD(
        "[RecImgBuf::dump]"
        "Name(%s),ION(%d),VA(0x%08X),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld),S/C(%d/%d)", 
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getBufSize(),
        getImgFormat().string(), 
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(), 
        getBitsPerPixel(),
        getTimestamp(),
        getBufSecu(),
        getBufCohe()
    );
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager*
RecBufManager::
alloc(
    char const*const        szImgFormat,
    uint32_t const          u4ImgWidth, 
    uint32_t const          u4ImgHeight, 
    uint32_t const          u4BufCount, 
    char const*const        szName, 
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe
)
{
    RecBufManager* pMgr = new RecBufManager(
                                szImgFormat,
                                u4ImgWidth,
                                u4ImgHeight, 
                                u4BufCount,
                                szName,
                                requestMemory,
                                i4BufSecu,
                                i4BufCohe);
    //
    if  ( pMgr && ! pMgr->init() )
    {
        // return NULL due to init failure.
        pMgr = NULL;
    }
    //
    return pMgr;
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager::
RecBufManager(
    char const*const        szImgFormat,
    uint32_t const          u4ImgWidth, 
    uint32_t const          u4ImgHeight, 
    uint32_t const          u4BufCount, 
    char const*const        szName, 
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe
)
    : RefBase()
    //
    , ms8Name(szName)
    , ms8ImgFormat(szImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4BufCount(u4BufCount)
    //
    , mvImgBuf()
    , mRequestMemory(requestMemory)
    //
    , mi4BufSecu(i4BufSecu)
    , mi4BufCohe(i4BufCohe)
    //
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager::
~RecBufManager()
{
    uninit();
    //
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecBufManager::
init()
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    mvImgBuf.clear();
    mvImgBufIonFd.clear();
    for (size_t i = 0; i < mu4BufCount; i++)
    {
        RecImgBuf* pRecImgBuf = RecImgBuf::alloc(
            mRequestMemory, 
            new RecImgInfo(
                mu4ImgWidth, 
                mu4ImgHeight, 
                ms8ImgFormat, 
                ms8Name,
                mi4BufSecu,
                mi4BufCohe
            )
        );
        if  ( pRecImgBuf == 0 )
        {
            MY_LOGE("cannot allocate pRecImgBuf [%d]", i);
            goto lbExit;
        }
        //
        pRecImgBuf->dump();
        mvImgBuf.push_back(pRecImgBuf);
        if  ( mvImgBuf[i] == 0 )
        {
            MY_LOGE("cannot allocate mvImgBuf[%d]", i);
            goto lbExit;
        }
        //
        mvImgBufIonFd.push_back(pRecImgBuf->getIonFd());
        if  ( mvImgBufIonFd[i] == 0 )
        {
            MY_LOGE("cannot allocate mvImgBufIonFd[%d]", i);
            goto lbExit;
        }
    }
    //
    //
    MY_LOGD("- ret(1)");
    return true;
lbExit:
    MY_LOGD("- ret(0)");
    uninit();
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecBufManager::
uninit()
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    for (size_t i = 0; i < mu4BufCount; i++)
    {   
        mvImgBuf.editItemAt(i) = NULL;
    }
    //
    mvImgBufIonFd.clear();
    //
    MY_LOGD("-");
}

