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
#define LOG_TAG "CamShot/SImager"
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
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/ISImager.h>
//
#include "../inc/SImager.h"
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
class ISImagerBridge : public ISImager
{
    friend  class   ISImager;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    mutable android::Mutex      mLock;
    android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    SImager*const            mpSImagerImp;
    inline  SImager const*   getImp() const  { return mpSImagerImp; }
    inline  SImager*         getImp()        { return mpSImagerImp; }

protected:  ////    Constructor/Destructor.
                    ISImagerBridge(SImager*const pSImager);
                    ~ISImagerBridge();

private:    ////    Disallowed.
                    ISImagerBridge(ISImagerBridge const& obj);
    ISImagerBridge&  operator=(ISImagerBridge const& obj);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    virtual MVOID   destroyInstance();

public:     ////    Attributes.
    virtual MINT32      getLastErrorCode() const;
    virtual MUINT32     getJpegSize() ; 

public:     ////    Callbacks.
    virtual MVOID   setCallback(SImagerNotifyCallback_t notify_cb, MVOID* user); 
    //
public:     ////    Operations.
    virtual MBOOL    execute(MUINT32 const u4TimeoutMs = 0xFFFFFFFF); 
    virtual MBOOL    executeAsync(); 
    virtual MBOOL    cancel(); 

public:     ////    Settings.
    virtual MBOOL   setTargetBufInfo(BufInfo const &rBufInfo); 
    virtual MBOOL   setFormat(EImageFormat const eFormat); 
    virtual MBOOL   setFlip(MUINT32 const u4Flip); 
    virtual MBOOL   setROI(Rect const rROI); 
    virtual MBOOL   setResize(MUINT32 const u4Width, MUINT32 const u4Height); 
    virtual MBOOL   setRotation(MUINT32 const u4Ratation); 
    virtual MBOOL   setEncodeParam(MUINT32 const &u4IsSOI, MUINT32 const &u4Quality); 
    virtual MBOOL   setStrideAlign(MUINT32 const u4StrideAlign[3]); 

};


/*******************************************************************************
* 
********************************************************************************/
ISImager*
ISImager::
createInstance(ImgBufInfo const & rImgBufInfo)
{
    SImager* pSImagerImp = new SImager(rImgBufInfo);
    if  ( ! pSImagerImp )
    {
        MY_LOGE("[ISImager] fail to new SImager");
        return  NULL;
    }
    //
    ISImagerBridge*  pISImager = new ISImagerBridge(pSImagerImp);
    if  ( ! pISImager )
    {
        MY_LOGE("[ISImager] fail to new ISImagerBridge");
        delete  pSImagerImp;
        return  NULL;
    }
    //
    return  pISImager;
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISImagerBridge::
destroyInstance()
{
    delete  mpSImagerImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
* 
********************************************************************************/
ISImagerBridge::
ISImagerBridge(SImager*const pSImager)
    : ISImager()
    , mLock()
    , mu4InitRefCount(0)
    , mpSImagerImp(pSImager)
{
}


/*******************************************************************************
* 
********************************************************************************/
ISImagerBridge::
~ISImagerBridge()
{
}

/*******************************************************************************
* 
********************************************************************************/
MINT32
ISImagerBridge::
getLastErrorCode() const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISImagerBridge::
setCallback(SImagerNotifyCallback_t notify_cb, MVOID* user)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setCallback(notify_cb, user);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
execute(MUINT32 const u4TimeoutMs)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->execute(u4TimeoutMs); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
executeAsync()
{
    Mutex::Autolock _lock(mLock);
    return getImp()->executeAsync(); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
cancel()
{
    Mutex::Autolock _lock(mLock);
    return getImp()->cancel(); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setTargetBufInfo(BufInfo const &rBufInfo)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setTargetBufInfo(rBufInfo); 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setFormat(EImageFormat const eFormat)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setFormat(eFormat); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setFlip(MUINT32 const u4Flip)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setFlip(u4Flip); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setROI(Rect const rROI)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setROI(rROI); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setResize(MUINT32 const u4Width, MUINT32 const u4Height)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setResize(u4Width, u4Height); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setRotation(MUINT32 const u4Ratation)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setRotation(u4Ratation); 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL   
ISImagerBridge::
setStrideAlign(MUINT32 const u4StrideAlign[3])
{
    Mutex::Autolock _lock(mLock); 
    return getImp()->setStrideAlign(u4StrideAlign); 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISImagerBridge::
setEncodeParam(MUINT32 const &u4IsSOI, MUINT32 const &u4Quality)
{
    Mutex::Autolock _lock(mLock);
    return getImp()->setEncodeParam(u4IsSOI, u4Quality); 
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32
ISImagerBridge::
getJpegSize()  
{
    Mutex::Autolock _lock(mLock); 
    return getImp()->getJpegSize(); 
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

