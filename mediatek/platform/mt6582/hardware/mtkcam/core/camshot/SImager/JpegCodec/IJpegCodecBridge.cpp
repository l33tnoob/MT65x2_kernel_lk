/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "CamShot/JpegCodec"
// 
#include <utils/threads.h>
//
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>

//
#include "../../inc/IJpegCodec.h"
#include "./inc/JpegCodec.h"
//
using namespace android;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
class IJpegCodecBridge : public IJpegCodec
{
    friend  class   IJpegCodec;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    mutable android::Mutex      mLock;
    android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    JpegCodec*const            mpJpegCodecImp;
    inline  JpegCodec const*   getImp() const  { return mpJpegCodecImp; }
    inline  JpegCodec*         getImp()        { return mpJpegCodecImp; }

protected:  ////    Constructor/Destructor.
                    IJpegCodecBridge(JpegCodec*const pJpegCodec);
                    ~IJpegCodecBridge();

private:    ////    Disallowed.
                    IJpegCodecBridge(IJpegCodecBridge const& obj);
    IJpegCodecBridge&  operator=(IJpegCodecBridge const& obj);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    virtual MVOID   destroyInstance();

public:     ////    Attributes.
    virtual MINT32      getLastErrorCode() const;

   //
public:     ////    Operations.
    virtual MBOOL    encode(
                          ImgBufInfo const rSrcBufInfo, 
                          ImgBufInfo const rDstBufInfo, 
                          Rect const rROI, 
                          MUINT32 const u4Rotation, 
                          MUINT32 const u4Flip, 
                          MUINT32 const u4Quality, 
                          MUINT32 const u4IsSOI, 
                          MUINT32 &u4EncSize
                      ); 
};


/*******************************************************************************
* 
********************************************************************************/
IJpegCodec*
IJpegCodec::
createInstance()
{
    JpegCodec* pJpegCodecImp = new JpegCodec();
    if  ( ! pJpegCodecImp )
    {
        MY_LOGE("[IJpegCodec] fail to new JpegCodec");
        return  NULL;
    }
    //
    IJpegCodecBridge*  pIJpegCodec = new IJpegCodecBridge(pJpegCodecImp);
    if  ( ! pIJpegCodec )
    {
        MY_LOGE("[IJpegCodec] fail to new IJpegCodecBridge");
        delete  pJpegCodecImp;
        return  NULL;
    }
    //
    return  pIJpegCodec;
    return NULL;
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
IJpegCodecBridge::
destroyInstance()
{
    delete  mpJpegCodecImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
* 
********************************************************************************/
IJpegCodecBridge::
IJpegCodecBridge(JpegCodec*const pJpegCodec)
    : IJpegCodec()
    , mLock()
    , mu4InitRefCount(0)
    , mpJpegCodecImp(pJpegCodec)
{
}


/*******************************************************************************
* 
********************************************************************************/
IJpegCodecBridge::
~IJpegCodecBridge()
{
}

/*******************************************************************************
* 
********************************************************************************/
MINT32
IJpegCodecBridge::
getLastErrorCode() const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}



/*******************************************************************************
* 
********************************************************************************/
MBOOL
IJpegCodecBridge::
encode(
    ImgBufInfo const rSrcBufInfo, 
    ImgBufInfo const rDstBufInfo, 
    Rect const rROI, 
    MUINT32 const u4Rotation, 
    MUINT32 const u4Flip, 
    MUINT32 const u4Quality, 
    MUINT32 const u4IsSOI, 
    MUINT32 &u4EncSize
)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->encode(rSrcBufInfo, rDstBufInfo, rROI, u4Rotation, u4Flip, u4Quality, u4IsSOI, u4EncSize); 
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

