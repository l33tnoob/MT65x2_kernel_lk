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
#ifndef _MTK_CAMERA_CORE_CAMPIPE_INC_PIPEIMP_H_
#define _MTK_CAMERA_CORE_CAMPIPE_INC_PIPEIMP_H_

//
#include <vector>
//
using namespace std;

//
#include <cutils/atomic.h>
//
//
#include <mtkcam/campipe/_scenario.h>
#include <mtkcam/campipe/_identity.h>
#include <mtkcam/campipe/_callbacks.h>
#include <mtkcam/campipe/_params.h>
#include <mtkcam/campipe/_ports.h>
#include <mtkcam/campipe/_buffer.h>
//


/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
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
                        ESWScenarioID const eSWScenarioID, 
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
    MBOOL           handleNotifyCallback(MINT32 const i4Msg, MUINT32 const ext1, MUINT32 const ext2);  

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

protected:
    MVOID dumpPipeProperty(vector<PortProperty>const &vInPorts, vector<PortProperty>const &vOutPorts); 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Scenario.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    //
    ESWScenarioID const   meSWScenarioID;
    EScenarioFmt const  meScenarioFmt;

public:     ////    Operations.
    inline  MINT32  getScenarioID()     const { return meSWScenarioID; }
    inline  EScenarioFmt getScenarioFmt()    const { return meScenarioFmt; } 


};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe
#endif  //  _MTK_CAMERA_CORE_CAMPIPE_INC_PIPEIMP_H_

