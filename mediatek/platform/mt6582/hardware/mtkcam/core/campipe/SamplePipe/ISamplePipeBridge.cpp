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
#define LOG_TAG "campipe/sample"
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
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/ISamplePipe.h>
//
#include "../inc/PipeImp.h"
#include "../inc/SamplePipe.h"
//
using namespace android;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
class ISamplePipeBridge : public ISamplePipe
{
    friend  class   ISamplePipe;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    mutable android::Mutex      mLock;
    android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    SamplePipe*const            mpPipeImp;
    inline  SamplePipe const*   getImp() const  { return mpPipeImp; }
    inline  SamplePipe*         getImp()        { return mpPipeImp; }

protected:  ////    Constructor/Destructor.
                    ISamplePipeBridge(SamplePipe*const pSamplePipe);
                    ~ISamplePipeBridge();

private:    ////    Disallowed.
                    ISamplePipeBridge(ISamplePipeBridge const& obj);
    ISamplePipeBridge&  operator=(ISamplePipeBridge const& obj);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    virtual MVOID   destroyInstance();
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Attributes.
    virtual char const* getPipeName() const;
    virtual EPipeID     getPipeId() const;
    virtual MINT32      getLastErrorCode() const;

public:     ////    Callbacks.
    virtual MVOID   setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user);
    //
    //  notify callback
    virtual MBOOL   isNotifyMsgEnabled(MINT32 const i4MsgTypes) const;
    virtual MVOID   enableNotifyMsg(MINT32 const i4MsgTypes);
    virtual MVOID   disableNotifyMsg(MINT32 const i4MsgTypes);
    //
    //  data callback
    virtual MBOOL   isDataMsgEnabled(MINT32 const i4MsgTypes) const;
    virtual MVOID   enableDataMsg(MINT32 const i4MsgTypes);
    virtual MVOID   disableDataMsg(MINT32 const i4MsgTypes);

public:     ////    Operations.
    virtual MBOOL   start();
    virtual MBOOL   stop();

public:     ////    Buffer Quening.
    virtual MBOOL   enqueBuf(PortID const ePortID, QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeBuf(PortID const ePortID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);    

public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, MBOOL concurrency = MFALSE);
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3); 
public:     ////    info 
    virtual MBOOL   queryPipeProperty(vector<PortProperty> &vInPorts, vector<PortProperty> &vOutPorts); 
    

};


/*******************************************************************************
* 
********************************************************************************/
ISamplePipe*
ISamplePipe::
createInstance(ESWScenarioID const eSWScenarioID, EScenarioFmt const eScenarioFmt)
{
    SamplePipe* pPipeImp = new SamplePipe("SamplePipe", ISamplePipe::ePipeID, eSWScenarioID, eScenarioFmt);
    if  ( ! pPipeImp )
    {
        MY_LOGE("[ISamplePipe] fail to new SamplePipe");
        return  NULL;
    }
    //
    ISamplePipeBridge*  pIPipe = new ISamplePipeBridge(pPipeImp);
    if  ( ! pIPipe )
    {
        MY_LOGE("[ISamplePipe] fail to new ISamplePipeBridge");
        delete  pPipeImp;
        return  NULL;
    }
    //
    return  pIPipe;
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
destroyInstance()
{
    delete  mpPipeImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
* 
********************************************************************************/
ISamplePipeBridge::
ISamplePipeBridge(SamplePipe*const pSamplePipe)
    : ISamplePipe()
    , mLock()
    , mu4InitRefCount(0)
    , mpPipeImp(pSamplePipe)
{
}


/*******************************************************************************
* 
********************************************************************************/
ISamplePipeBridge::
~ISamplePipeBridge()
{
}


/*******************************************************************************
* 
********************************************************************************/
char const*
ISamplePipeBridge::
getPipeName() const
{
    return  getImp()->getPipeName();
}


/*******************************************************************************
* 
********************************************************************************/
EPipeID
ISamplePipeBridge::
getPipeId() const
{
    return  getImp()->getPipeId();
}

/*******************************************************************************
* 
********************************************************************************/
MINT32
ISamplePipeBridge::
getLastErrorCode() const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
init()
{
    MBOOL   ret = MTRUE;
    Mutex::Autolock _lock(mLock);

    if  ( 0 != mu4InitRefCount )
    {
        mu4InitRefCount++;
    }
    else if ( (ret = getImp()->init()) )
    {
        mu4InitRefCount = 1;
    }
    MY_LOGD("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
uninit()
{
    MBOOL   ret = MTRUE;
    Mutex::Autolock _lock(mLock);

    if  ( 0 < mu4InitRefCount )
    {
        mu4InitRefCount--;
        if  ( 0 == mu4InitRefCount )
        {
            ret = getImp()->uninit();
        }
    }
    MY_LOGD("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setCallbacks(notify_cb, data_cb, user);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
isNotifyMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isNotifyMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
enableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
disableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
isDataMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isDataMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
enableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableDataMsg(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MVOID
ISamplePipeBridge::
disableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableDataMsg(i4MsgTypes);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
start()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->start();
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
stop()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->stop();
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, MBOOL concurrency)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->configPipe(vInPorts, vOutPorts);
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL   
ISamplePipeBridge::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->sendCommand(cmd, arg1, arg2, arg3);
} 

/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
queryPipeProperty(vector<PortProperty> &vInPorts, vector<PortProperty> &vOutPorts)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->queryPipeProperty(vInPorts, vOutPorts);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
enqueBuf(PortID const ePortID, QBufInfo const& rQBufInfo)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enqueBuf(ePortID, rQBufInfo);
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
ISamplePipeBridge::
dequeBuf(PortID const ePortID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->dequeBuf(ePortID, rQBufInfo, u4TimeoutMs);
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe

