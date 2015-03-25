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

/**
* @file PipeImp.h
*
* PipeImp Header File
*/

#ifndef _ISPIO_PIPE_IMP_H_
#define _ISPIO_PIPE_IMP_H_

#include <vector>

using namespace std;

#include <cutils/atomic.h>

#include <mtkcam/common.h>

#include <inc/imageio/ispio_pipe_scenario.h>
#include <inc/imageio/ispio_pipe_identity.h>
#include <inc/imageio/ispio_pipe_callbacks.h>
//
#include <inc/imageio/ispio_pipe_ports.h>
#include <inc/imageio/ispio_pipe_buffer.h>
//

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
class PipeImp
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    char const*const    mszPipeName;
    EPipeID const       mePipeID;
    MINT32              mi4ErrorCode;

public:     ////    Operations.
    virtual char const* getPipeName() const         { return mszPipeName; }
    virtual EPipeID     getPipeId() const           { return mePipeID; }
    virtual MINT32      getLastErrorCode() const    { return mi4ErrorCode; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Constructor/Destructor.
                    PipeImp(
                        char const*const szPipeName,
                        EPipeID const ePipeID,
                        EScenarioID const eScenarioID,
                        EScenarioFmt const eScenarioFmt
                    );
    virtual         ~PipeImp()  {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Callbacks.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    //
    MVOID*          mpCbUser;           //  Callback user.
    //
    //  notify callback
    volatile MINT32         mi4NotifyMsgSet;//  bitset of notify message types.
    PipeNotifyCallback_t    mNotifyCb;      //  notify callback function pointer.
    //
    //  data callback
    volatile MINT32         mi4DataMsgSet;  //  bitset of data message types.
    PipeDataCallback_t      mDataCb;        //  data callback function pointer.

protected:  ////    Helpers.
    virtual MBOOL   onNotifyCallback(PipeNotifyInfo const& msg) const;
    virtual MBOOL   onDataCallback(PipeDataInfo const& msg) const;

public:     ////    Operations.
    //
    virtual MVOID   setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user);
    //
    //  notify callback
    inline MBOOL    isNotifyMsgEnabled(MINT32 const i4MsgTypes) const   { return (i4MsgTypes & mi4NotifyMsgSet); }
    inline MVOID    enableNotifyMsg(MINT32 const i4MsgTypes)            { ::android_atomic_or(i4MsgTypes, &mi4NotifyMsgSet); }
    inline MVOID    disableNotifyMsg(MINT32 const i4MsgTypes)           { ::android_atomic_and(~i4MsgTypes, &mi4NotifyMsgSet); }
    //
    //  data callback
    inline MBOOL    isDataMsgEnabled(MINT32 const i4MsgTypes) const     { return (i4MsgTypes & mi4DataMsgSet); }
    inline MVOID    enableDataMsg(MINT32 const i4MsgTypes)              { ::android_atomic_or(i4MsgTypes, &mi4DataMsgSet); }
    inline MVOID    disableDataMsg(MINT32 const i4MsgTypes)             { ::android_atomic_and(~i4MsgTypes, &mi4DataMsgSet); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Scenario.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    //
    EScenarioID const   meScenarioID;
    EScenarioFmt const  meScenarioFmt;

public:     ////    Operations.
    inline  MINT32  getScenarioID()     const { return meScenarioID; }
    inline  MINT32  getScenarioSubmode()const { return mapScenarioFormatToSubmode(meScenarioID, meScenarioFmt); }
    static  MINT32  mapScenarioFormatToSubmode(EScenarioID const eScenarioID, EScenarioFmt const eScenarioFmt);
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_IMP_H_

