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

#ifndef _MTK_HAL_CAMADAPTER_MTKENG_STATE_STATE_H_
#define _MTK_HAL_CAMADAPTER_MTKENG_STATE_STATE_H_
//
#include <utils/threads.h>


namespace android {
namespace NSMtkEngCamAdapter {
/*******************************************************************************
*
*******************************************************************************/
class IState;
class IStateHandler;


/*******************************************************************************
*
*******************************************************************************/
class StateBase : public IState
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Interfaces
    virtual status_t    onStartPreview(IStateHandler* pHandler) { return op_UnSupport(__FUNCTION__); }
    virtual status_t    onStopPreview(IStateHandler* pHandler)  { return op_UnSupport(__FUNCTION__); }
    //
    virtual status_t    onPreCapture(IStateHandler* pHandler)   { return op_UnSupport(__FUNCTION__); }
    virtual status_t    onCapture(IStateHandler* pHandler)      { return op_UnSupport(__FUNCTION__); }
    virtual status_t    onCaptureDone(IStateHandler* pHandler)  { return op_UnSupport(__FUNCTION__); }
    virtual status_t    onCancelCapture(IStateHandler* pHandler){ return op_UnSupport(__FUNCTION__); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Operations.
                        StateBase(char const*const pcszName, ENState const eState);
    virtual char const* getName() const { return m_pcszName; }
    virtual ENState     getEnum() const { return m_eState; }
    IStateManager*      getStateManager() const { return m_pStateManager; }

protected:  ////        Data Members.
    char const*const    m_pcszName;
    ENState const       m_eState;
    IStateManager*const m_pStateManager;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////        Operations.
    status_t            waitState(ENState const eState, nsecs_t const timeout = -1);
    status_t            op_UnSupport(char const*const pcszDbgText = "");

};


/*******************************************************************************
*
*******************************************************************************/
struct StateIdle : public StateBase
{
                        StateIdle(ENState const eState);
    //
    virtual status_t    onStartPreview(IStateHandler* pHandler);
    virtual status_t    onCapture(IStateHandler* pHandler);
};


/*******************************************************************************
*
*******************************************************************************/
struct StatePreview : public StateBase
{
                        StatePreview(ENState const eState);
    //
    virtual status_t    onStopPreview(IStateHandler* pHandler);
    virtual status_t    onPreCapture(IStateHandler* pHandler);
};


/*******************************************************************************
*
*******************************************************************************/
struct StatePreCapture : public StateBase
{
                        StatePreCapture(ENState const eState);
    //
    virtual status_t    onStopPreview(IStateHandler* pHandler);
};


/*******************************************************************************
*
*******************************************************************************/
struct StateCapture : public StateBase
{
                        StateCapture(ENState const eState);
    //
    virtual status_t    onCaptureDone(IStateHandler* pHandler);
    virtual status_t    onCancelCapture(IStateHandler* pHandler);
};



/*******************************************************************************
*
*******************************************************************************/
};
}; // namespace android
#endif // _MTK_HAL_CAMADAPTER_MTKENG_STATE_STATE_H_

