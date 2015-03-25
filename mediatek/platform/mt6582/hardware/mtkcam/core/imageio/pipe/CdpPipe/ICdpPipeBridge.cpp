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
#define LOG_TAG "iio/cdpb"
//
#include <utils/threads.h>
//
//#include <my_log.h>
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

//
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICdpPipe.h>
#include <cutils/properties.h>  // For property_get().

//
#include "PipeImp.h"
#include "CdpPipe.h"
//
using namespace android;



/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""

//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);




/*******************************************************************************
*
********************************************************************************/
class ICdpPipeBridge : public ICdpPipe
{
    friend  class   ICdpPipe;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    mutable android::Mutex      mLock;
    android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    CdpPipe*const            mpPipeImp;
    inline  CdpPipe const*   getImp() const  { return mpPipeImp; }
    inline  CdpPipe*         getImp()        { return mpPipeImp; }

protected:  ////    Constructor/Destructor.
                    ICdpPipeBridge(CdpPipe*const pCdpPipe);
                    ~ICdpPipeBridge();

private:    ////    Disallowed.
                    ICdpPipeBridge(ICdpPipeBridge const& obj);
    ICdpPipeBridge&  operator=(ICdpPipeBridge const& obj);

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
    virtual MBOOL   syncJpegPass2C();
    virtual MBOOL   startFmt();
    virtual MBOOL   stop();
public:     ////    Buffer Quening.
    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/);
    //
    virtual MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/);
public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
    virtual MBOOL   configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
public:     ////    Interrupt handling
	virtual MBOOL   irq(EPipePass pass, EPipeIRQ irq_int);
public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

};


/*******************************************************************************
*
********************************************************************************/
ICdpPipe*
ICdpPipe::
createInstance(EScenarioID const eScenarioID, EScenarioFmt const eScenarioFmt)
{
    CdpPipe* pPipeImp = new CdpPipe("CdpPipe", ICdpPipe::ePipeID, eScenarioID, eScenarioFmt);
    if  ( ! pPipeImp )
    {
        PIPE_ERR("[ICdpPipe] fail to new CdpPipe");
        return  NULL;
    }
    //
    ICdpPipeBridge*  pIPipe = new ICdpPipeBridge(pPipeImp);
    if  ( ! pIPipe )
    {
        PIPE_ERR("[ICdpPipe] fail to new ICdpPipeBridge");
        delete  pPipeImp;
        return  NULL;
    }
    DBG_LOG_CONFIG(imageio, pipe);

    //
    return  pIPipe;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
destroyInstance()
{
    delete  mpPipeImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
ICdpPipeBridge::
ICdpPipeBridge(CdpPipe*const pCdpPipe)
    : ICdpPipe()
    , mLock()
    , mu4InitRefCount(0)
    , mpPipeImp(pCdpPipe)
{
}


/*******************************************************************************
*
********************************************************************************/
ICdpPipeBridge::
~ICdpPipeBridge()
{
}


/*******************************************************************************
*
********************************************************************************/
char const*
ICdpPipeBridge::
getPipeName() const
{
    return  getImp()->getPipeName();
}


/*******************************************************************************
*
********************************************************************************/
EPipeID
ICdpPipeBridge::
getPipeId() const
{
    return  getImp()->getPipeId();
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ICdpPipeBridge::
getLastErrorCode() const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
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
    PIPE_DBG("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
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
    PIPE_DBG("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setCallbacks(notify_cb, data_cb, user);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
isNotifyMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isNotifyMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
enableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
disableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
isDataMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isDataMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
enableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICdpPipeBridge::
disableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
start()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->start();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
syncJpegPass2C()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->syncJpegPass2C();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
startFmt()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->startFmt();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
stop()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->stop();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->configPipe(vInPorts, vOutPorts);
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->configPipeUpdate(vInPorts, vOutPorts);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
irq(EPipePass pass, EPipeIRQ irq_int)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->irq(pass, irq_int);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enqueInBuf(portID, rQBufInfo);
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->dequeInBuf(portID, rQBufInfo, u4TimeoutMs);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enqueOutBuf(portID, rQBufInfo);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICdpPipeBridge::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->dequeOutBuf(portID, rQBufInfo, u4TimeoutMs);
}

/*******************************************************************************
* sendCommand
********************************************************************************/
MBOOL
ICdpPipeBridge::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->sendCommand(cmd, arg1, arg2, arg3);
}

/*******************************************************************************
* ICmd
********************************************************************************/
ICdpPipe::
ICmd::
ICmd(IPipe*const pIPipe)
    : mpIPipe(reinterpret_cast<ICdpPipe*>(pIPipe))
{
}


MBOOL
ICdpPipe::
ICmd::
verifySelf()
{
    ICdpPipeBridge*const pBridge = reinterpret_cast<ICdpPipeBridge*>(mpIPipe);
    //
    if  ( NULL == mpIPipe )
    {
        PIPE_ERR("[ICdpPipe::ICmd::verifySelf] NULL mpIPipe");
        return  MFALSE;
    }
    //
    if  ( pBridge->getPipeId() != ICdpPipe::ePipeID )
    {
        PIPE_ERR("[ICdpPipe::ICmd::verifySelf] ID(%d) != ICdpPipe::ePipeID(%d)", pBridge->getPipeId(), ICdpPipe::ePipeID);
        return  MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
* Cmd_Set2Params
* Command: Set 2 parameters.
********************************************************************************/
ICdpPipe::
Cmd_Set2Params::
Cmd_Set2Params(IPipe*const pIPipe, MUINT32 const u4Param1, MUINT32 const u4Param2)
    : ICmd(pIPipe)
    , mu4Param1(u4Param1)
    , mu4Param2(u4Param2)
{
}


MBOOL
ICdpPipe::
Cmd_Set2Params::
execute()
{
    if  ( verifySelf() )
    {
        ICdpPipeBridge*const pBridge = reinterpret_cast<ICdpPipeBridge*>(mpIPipe);
        //
        Mutex::Autolock _lock(pBridge->getLockRef());
        //
        CdpPipe*const pPipe = pBridge->getImp();
        return  pPipe->onSet2Params(mu4Param1, mu4Param2);
    }
    return  MFALSE;
}


/*******************************************************************************
* Cmd_Get1ParamBasedOn1Input
* Command: Get 1 parameter based on 1 input parameter.
********************************************************************************/
ICdpPipe::
Cmd_Get1ParamBasedOn1Input::
Cmd_Get1ParamBasedOn1Input(IPipe*const pIPipe, MUINT32 const u4InParam, MUINT32*const pu4OutParam)
    : ICmd(pIPipe)
    , mu4InParam(u4InParam)
    , mpu4OutParam(pu4OutParam)
{
}


MBOOL
ICdpPipe::
Cmd_Get1ParamBasedOn1Input::
execute()
{
    if  ( verifySelf() )
    {
        ICdpPipeBridge*const pBridge = reinterpret_cast<ICdpPipeBridge*>(mpIPipe);
        //
        Mutex::Autolock _lock(pBridge->getLockRef());
        //
        CdpPipe*const pPipe = pBridge->getImp();
        return  pPipe->onGet1ParamBasedOn1Input(mu4InParam, mpu4OutParam);
    }
    return  MFALSE;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

