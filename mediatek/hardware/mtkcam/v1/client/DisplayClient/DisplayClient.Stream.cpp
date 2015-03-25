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
#include <camera/MtkCameraParameters.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
//
#include "DisplayClient.h"
using namespace NSDisplayClient;
//
#if ('1'==MTKCAM_HAVE_GRALLOC_EXTRA)
#include <ui/gralloc_extra.h>
#endif


/******************************************************************************
*
*******************************************************************************/
#if defined(MTKCAM_DISPLAY_USE_YV12_GPU)
    #define CAMERA_DISPLAY_FORMAT       (MtkCameraParameters::PIXEL_FORMAT_YV12_GPU)
    #define CAMERA_DISPLAY_FORMAT_HAL   (HAL_PIXEL_FORMAT_YV12)
#else
    #warning "[FIXME] I420 used in DisplayClient"
    #define CAMERA_DISPLAY_FORMAT       (MtkCameraParameters::PIXEL_FORMAT_YUV420I)
    #define CAMERA_DISPLAY_FORMAT_HAL   (HAL_PIXEL_FORMAT_I420)
#endif

#define CAMERA_GRALLOC_USAGE        ( GRALLOC_USAGE_SW_READ_RARELY  \
                                    | GRALLOC_USAGE_SW_WRITE_NEVER  \
                                    | GRALLOC_USAGE_HW_TEXTURE )


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
*******************************************************************************/
void
DisplayClient::
dumpDebug(StreamBufList_t const& rQue, char const*const pszDbgText /*= ""*/)
{
    MY_LOGD("[%s] + StreamBufList_t.size(%d)", pszDbgText, rQue.size());
    if  ( ! rQue.empty() )
    {
        MY_LOGD("NOT EMPTY StreamBufList_t.size(%d)", rQue.size());
        for (StreamBufList_t::const_iterator it = rQue.begin(); it != rQue.end(); it++)
        {
            if  ( (*it) != 0 ) {
                (*it)->dump();
            }
        }
    }
    MY_LOGD("[%s] -", pszDbgText);
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
set_preview_stream_ops(
    preview_stream_ops*const window, 
    int32_t const   wndWidth, 
    int32_t const   wndHeight, 
    int32_t const   i4MaxImgBufCount
)
{
    CamProfile profile(__FUNCTION__, "DisplayClient");
    //
    bool        ret = false;
    status_t    err = 0;
    int32_t     min_undequeued_buf_count = 0;
    //
    //  (2) Check
    if  ( ! mStreamBufList.empty() )
    {
        MY_LOGE(
            "locked buffer count(%d)!=0, "
            "callers must return all dequeued buffers, "
//            "and then call cleanupQueue()"
            , mStreamBufList.size()
        );
        dumpDebug(mStreamBufList, __FUNCTION__);
        goto lbExit;
    }
    //
    //  (3) Sava info.
    mpStreamImgInfo.clear();
    mpStreamImgInfo     = new ImgInfo(wndWidth, wndHeight, CAMERA_DISPLAY_FORMAT, CAMERA_DISPLAY_FORMAT_HAL, "Camera@Display");
    mpStreamOps         = window;
    mi4MaxImgBufCount   = i4MaxImgBufCount;
    //
    //
    //  (4.1) Set gralloc usage bits for window. 
    err = mpStreamOps->set_usage(mpStreamOps, CAMERA_GRALLOC_USAGE);
    if  ( err )
    {
        MY_LOGE("set_usage failed: status[%s(%d)]", ::strerror(-err), -err);
        if  ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned");
            mpStreamOps = NULL;
        }
        goto lbExit;
    }
    //
    //  (4.2) Get minimum undequeue buffer count
    err = mpStreamOps->get_min_undequeued_buffer_count(mpStreamOps, &min_undequeued_buf_count);
    if  ( err )
    {
        MY_LOGE("get_min_undequeued_buffer_count failed: status[%s(%d)]", ::strerror(-err), -err);
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mpStreamOps = NULL;
        }
        goto lbExit;
    }
    //
    //  (4.3) Set the number of buffers needed for display.
    MY_LOGI(
        "set_buffer_count(%d) = wanted_buf_count(%d) + min_undequeued_buf_count(%d)", 
        mi4MaxImgBufCount+min_undequeued_buf_count, mi4MaxImgBufCount, min_undequeued_buf_count
    );
    err = mpStreamOps->set_buffer_count(mpStreamOps, mi4MaxImgBufCount+min_undequeued_buf_count);
    if  ( err )
    {
        MY_LOGE("set_buffer_count failed: status[%s(%d)]", ::strerror(-err), -err);
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mpStreamOps = NULL;
        }
        goto lbExit;
    }
    //
    //  (4.4) Set window geometry
    err = mpStreamOps->set_buffers_geometry(
            mpStreamOps, 
            mpStreamImgInfo->mu4ImgWidth, 
            mpStreamImgInfo->mu4ImgHeight, 
            mpStreamImgInfo->mi4ImgFormat
        );
    if  ( err )
    {
        MY_LOGE(
            "set_buffers_geometry(%dx%d@%s/%x) failed: status[%s(%d)]", 
            mpStreamImgInfo->mu4ImgWidth, mpStreamImgInfo->mu4ImgHeight, 
            mpStreamImgInfo->ms8ImgFormat.string(), mpStreamImgInfo->mi4ImgFormat, 
            ::strerror(-err), -err
        );
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mpStreamOps = NULL;
        }
        goto lbExit;
    }
    //
    //
    ret = true;
lbExit:
    profile.print_overtime(10, "ret(%d)", ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
DisplayClient::
dequePrvOps(sp<StreamImgBuf>& rpImgBuf)
{
    bool                    ret = false;
    status_t                err = 0;
    //
    int                     fdIon   = -1;
    buffer_handle_t*        phBuffer= NULL;
    void*                   address = NULL;
    int                     stride  = 0;    // dummy variable to get stride
    Rect const              bounds(mpStreamImgInfo->mu4ImgWidth, mpStreamImgInfo->mu4ImgHeight);
    //
    //
    MY_LOGD_IF((2<=miLogLevel), "+");
    //
    if  ( ! mpStreamOps || mpStreamImgInfo == 0 )
    {
        MY_LOGW("mpStreamOps(%p), mpStreamImgInfo.get(%p)", mpStreamOps, mpStreamImgInfo.get());
        goto lbExit;
    }
    //
    //  [1] dequeue_buffer
    err = mpStreamOps->dequeue_buffer(mpStreamOps, &phBuffer, &stride);
    if  ( err || NULL == phBuffer )
    {
        MY_LOGW(
            "dequeue_buffer failed with phBuffer=%p: status[%s(%d)], mStreamBufList.size()(%d)", 
            phBuffer, ::strerror(-err), -err, mStreamBufList.size()
        );
        goto lbExit;
    }
    //
    //  [2] lock buffers
    err = mpStreamOps->lock_buffer(mpStreamOps, phBuffer);
    if  ( err )
    {
        MY_LOGE("lock_buffer failed: status[%s(%d)]", ::strerror(-err), -err);
        mpStreamOps->cancel_buffer(mpStreamOps, phBuffer);
        goto lbExit;
    }
    //
    //  [3] Now let the graphics framework to lock the buffer, and provide
    //  us with the framebuffer data address.
    err = GraphicBufferMapper::get().lock(*phBuffer, CAMERA_GRALLOC_USAGE, bounds, &address);
    if  ( err )
    {
        MY_LOGE("GraphicBufferMapper.lock failed: status[%s(%d)]", ::strerror(-err), -err);
        mpStreamOps->cancel_buffer(mpStreamOps, phBuffer);
        goto lbExit;
    }
    //
    //  [4] Get the ion fd of gralloc buffer.
#if ('1'==MTKCAM_HAVE_GRALLOC_EXTRA)
    {
        int idx = 0, num = 0;
        err = ::gralloc_extra_getIonFd(*phBuffer, &idx, &num);
        if  ( num > 0 ) // current num should be 1
        {
            fdIon = (*phBuffer)->data[idx];
            MY_LOGD_IF((2<=miLogLevel), "getIonFd(): fdIon=%d, idx/num=%d/%d, status[%s(%d)]", fdIon, idx, num, ::strerror(-err), -err);
        }
        else
        {
            fdIon = -1;
            MY_LOGW("getIonFd(): fdIon=-1, idx/num=%d/%d, status[%s(%d)]", idx, num, ::strerror(-err), -err);
        }
    }
#endif
    //
    //  [5] Setup the output to return.
    rpImgBuf = new StreamImgBuf(mpStreamImgInfo, stride, address, phBuffer, fdIon);
    //
    ret = true;
lbExit:
    MY_LOGD_IF(
        (2<=miLogLevel), 
        "- ret:%d, (ion, address, stride)=(%d, %p, %d), phBuffer=%p/%p", 
        ret, fdIon, address, stride, phBuffer, (phBuffer ? *phBuffer : 0)
    );
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
DisplayClient::
enquePrvOps(sp<StreamImgBuf>const& rpImgBuf)
{
    mProfile_enquePrvOps.pulse();
    if  ( mProfile_enquePrvOps.getDuration() >= ::s2ns(2) ) {
        mProfile_enquePrvOps.updateFps();
        mProfile_enquePrvOps.showFps();
        mProfile_enquePrvOps.reset();
    }
    //
    status_t    err = 0;
    //
    CamProfile profile(__FUNCTION__, "DisplayClient");
    profile.print_overtime(
        ((1<=miLogLevel) ? 0 : 1000), 
        "+ locked buffer count(%d), rpImgBuf(%p,%p), Timestamp(%lld)", 
        mStreamBufList.size(), rpImgBuf.get(), rpImgBuf->getVirAddr(), rpImgBuf->getTimestamp()
    );
    //
    //  [1] unlock buffer before sending to display
    GraphicBufferMapper::get().unlock(rpImgBuf->getBufHndl());
    profile.print_overtime(1, "GraphicBufferMapper::unlock");
    //
    //  [2] Dump image if wanted.
    dumpImgBuf_If(rpImgBuf);
    //
    //  [3] set timestamp.
    err = mpStreamOps->set_timestamp(mpStreamOps, rpImgBuf->getTimestamp());
    profile.print_overtime(2, "mpStreamOps->set_timestamp, Timestamp(%lld)", rpImgBuf->getTimestamp());
    if  ( err )
    {
        MY_LOGE(
            "mpStreamOps->set_timestamp failed: status[%s(%d)], rpImgBuf(%p), Timestamp(%lld)", 
            ::strerror(-err), -err, rpImgBuf.get(), rpImgBuf->getTimestamp()
        );
    }
    //
    //  [4] set gralloc buffer type & dirty
#if ('1'==MTKCAM_HAVE_GRALLOC_EXTRA)
    ::gralloc_extra_setBufParameter(
        rpImgBuf->getBufHndl(), 
        GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, 
        GRALLOC_EXTRA_BIT_TYPE_CAMERA | GRALLOC_EXTRA_BIT_DIRTY
    );
#endif
    //
    //  [5] unlocks and post the buffer to display.
    err = mpStreamOps->enqueue_buffer(mpStreamOps, rpImgBuf->getBufHndlPtr());
    profile.print_overtime(10, "mpStreamOps->enqueue_buffer, Timestamp(%lld)", rpImgBuf->getTimestamp());
    if  ( err )
    {
        MY_LOGE(
            "mpStreamOps->enqueue_buffer failed: status[%s(%d)], rpImgBuf(%p,%p)", 
            ::strerror(-err), -err, rpImgBuf.get(), rpImgBuf->getVirAddr()
        );
    }
}


/******************************************************************************
*
*******************************************************************************/
void
DisplayClient::
cancelPrvOps(sp<StreamImgBuf>const& rpImgBuf)
{
    //
    status_t    err = 0;
    //
    CamProfile profile(__FUNCTION__, "DisplayClient");
    profile.print_overtime(
        ((1<=miLogLevel) ? 0 :1000), 
        "+ locked buffer count(%d), rpImgBuf(%p,%p)", 
        mStreamBufList.size(), rpImgBuf.get(), rpImgBuf->getVirAddr()
    );
    //
    //  [1] unlock buffer before canceling
    GraphicBufferMapper::get().unlock(rpImgBuf->getBufHndl());
    //
    profile.print_overtime(1, "GraphicBufferMapper::unlock");
    //
    //  [2] unlocks and cancel buffer.
    err = mpStreamOps->cancel_buffer(mpStreamOps, rpImgBuf->getBufHndlPtr());
    if  ( err )
    {
        MY_LOGE(
            "mpStreamOps->cancel_buffer failed: status[%s(%d)], rpImgBuf(%p,%p)", 
            ::strerror(-err), -err, rpImgBuf.get(), rpImgBuf->getVirAddr()
        );
    }
    //
    //
    profile.print_overtime(
        ((1<=miLogLevel) ? 0 :10), 
        "mpStreamOps->cancel_buffer, rpImgBuf(%p,%p)", 
        rpImgBuf.get(), rpImgBuf->getVirAddr()
    );
}

