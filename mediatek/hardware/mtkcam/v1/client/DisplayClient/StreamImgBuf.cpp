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

#define LOG_TAG "MtkCam/DisplayClient"
//
#include "CamUtils.h"
using namespace android;
using namespace MtkCamUtils;
#include "StreamImgBuf.h"
using namespace NSDisplayClient;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
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
ImgInfo::
ImgInfo(
    uint32_t const u4ImgWidth, 
    uint32_t const u4ImgHeight, 
    char const*const ImgFormat, 
    int32_t const i4ImgFormat, 
    char const*const pImgName
)
    : ms8ImgName(pImgName)
    , ms8ImgFormat(ImgFormat)
    , mi4ImgFormat(i4ImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4BitsPerPixel( FmtUtils::queryBitsPerPixel(ms8ImgFormat) )
{
    CAM_LOGD(
        "[%s](%s@%dx%d@%d-bit)", 
        ms8ImgName.string(), ms8ImgFormat.string(), 
        mu4ImgWidth, mu4ImgHeight, mu4BitsPerPixel
    );
}


/******************************************************************************
 *
 ******************************************************************************/
StreamImgBuf::
StreamImgBuf(
    sp<ImgInfo const>const& rpImgInfo, 
    int32_t const   i4Stride, 
    void*const      pBufBase, 
    buffer_handle_t*pBufHndl, 
    int const       fdIon, 
    int64_t const   timestamp
)
    : IImgBuf()
    , mpImgInfo(rpImgInfo)
    , mi8Timestamp(timestamp)
    , mfdIon(fdIon)
    , mpBufBase(pBufBase)
    , mpBufHndl(pBufHndl)
    , mpANWBuffer(0)
    , mi4Stride(i4Stride)
    , mBufSize( FmtUtils::queryImgBufferSize(getImgFormat(), getImgWidth(), getImgHeight()) )
{
#ifndef container_of
#define container_of(ptr, type, member) \
    (type *)((char*)(ptr) - offsetof(type, member))
#endif

    mpANWBuffer = container_of(pBufHndl, ANativeWindowBuffer, handle);

    MY_LOGE_IF(mpANWBuffer->stride != (int)getImgWidthStride(0),"mismatch stride: %d %d", mpANWBuffer->stride, mi4Stride);
    MY_LOGE_IF(mpANWBuffer->width  != (int)getImgWidth(),       "mismatch width: %d %d", mpANWBuffer->width, getImgWidth());
    MY_LOGE_IF(mpANWBuffer->height != (int)getImgHeight(),      "mismatch height: %d %d", mpANWBuffer->height, getImgHeight());
#if 1
    size_t const y_size = getImgHeight() * (getImgWidthStride(0));
    size_t const vu_size= (getImgHeight()>>1) * (getImgWidthStride(1) + getImgWidthStride(2));
    size_t const bufSize = y_size + vu_size;
    MY_LOGE_IF(mBufSize != bufSize, "mismatch buffer size: %d %d", mBufSize, bufSize);
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
StreamImgBuf::
~StreamImgBuf()
{
#if 0
    MY_LOGD(
        "%dx%d ion:%d, pBuf/pBufHndl=%p/%p/%p", 
        getImgWidth(), getImgHeight(), 
        mfdIon, mpBufBase, mpBufHndl, *mpBufHndl
    );
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
StreamImgBuf::
getImgWidthStride(uint_t const uPlaneIndex) const
{
    return  (0 == uPlaneIndex)
        ?   mi4Stride
        :   ((~15) & (15 + (mi4Stride>>1)))
            ;
}


/******************************************************************************
 *
 ******************************************************************************/
void
StreamImgBuf::
dump() const
{
    MY_LOGD(
        "[%s](%s@%dx%d@%d-bit@%d), ion:%d, pBuf/pBufHndl=%p/%p/%p, Stride:%d, Timestamp:%lld", 
        getBufName(), getImgFormat().string(), getImgWidth(), getImgHeight(), 
        getBitsPerPixel(), getBufSize(), 
        mfdIon, 
        mpBufBase, mpBufHndl, *mpBufHndl, mi4Stride, mi8Timestamp
    );
}

