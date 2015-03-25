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

#ifndef _MTK_CAMERA_INC_COMMON_CAMUTILS_IBUFFER_H_
#define _MTK_CAMERA_INC_COMMON_CAMUTILS_IBUFFER_H_
//
#include <sys/types.h>
//
#include <utils/RefBase.h>
#include <utils/String8.h>
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/RefBase.h>
#else
#include <utils/StrongPointer.h>
#endif


/******************************************************************************
 *
 ******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  Memory Buffer Interface.
 ******************************************************************************/
class IMemBuf : public virtual RefBase
{
public:     ////                Attributes.
    //
    virtual int64_t             getTimestamp() const                    = 0;
    virtual void                setTimestamp(int64_t const timestamp)   = 0;
    //
public:     ////                Attributes.
    virtual const char*         getBufName() const                      = 0;
    virtual size_t              getBufSize() const                      = 0;
    //
    virtual void*               getVirAddr() const                      = 0;
    virtual void*               getPhyAddr() const                      = 0;
    //
    virtual int                 getIonFd() const                        { return -1; }
    virtual int                 getBufSecu() const                      { return 0; }
    virtual int                 getBufCohe() const                      { return 0; }
    //
public:     ////
    //
    virtual                     ~IMemBuf() {};
};


/******************************************************************************
 *  Image Buffer Interface.
 ******************************************************************************/
class IImgBuf : public IMemBuf
{
public:     ////                Attributes.
    virtual String8 const&      getImgFormat()      const               = 0;
    virtual uint32_t            getImgWidth()       const               = 0;
    virtual uint32_t            getImgHeight()      const               = 0;

    //
    //  planeIndex
    //      [I] plane's index; 0, 1, and 2 refer to 1st-, 2nd-, and 3rd planes, 
    //          respectively.
    //
    //  return
    //      return its corresponding plane's stride, in pixel
    //
    virtual uint32_t            getImgWidthStride(
                                    uint_t const uPlaneIndex = 0
                                )   const                               = 0;

    virtual uint32_t            getBitsPerPixel()   const               = 0;
    //
public:     ////
    //
    virtual                     ~IImgBuf() {};
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_CAMERA_INC_COMMON_CAMUTILS_IBUFFER_H_

