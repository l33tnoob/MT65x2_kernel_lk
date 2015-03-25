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

#ifndef _MTK_HAL_CAMADAPTER_MTKPHOTO_INC_ISTATE_H_
#define _MTK_HAL_CAMADAPTER_MTKPHOTO_INC_ISTATE_H_
//


/*******************************************************************************
*
*******************************************************************************/
namespace android {
namespace NSMtkPhotoCamAdapter {


/*******************************************************************************
*   IStateHandler
*******************************************************************************/
class IStateHandler
{
public:     ////            Operations.
    virtual                 ~IStateHandler() {}

public:     ////            Interfaces
    virtual status_t        onHandleStartPreview()                          = 0;
    virtual status_t        onHandleStopPreview()                           = 0;
    //
    virtual status_t        onHandlePreCapture()                            = 0;
    virtual status_t        onHandleCapture()                               = 0;
    virtual status_t        onHandleCaptureDone()                           = 0;
    virtual status_t        onHandleCancelCapture()                         = 0;
};


/*******************************************************************************
*   IState
*******************************************************************************/
class IState
{
public:     ////            State Enum.
                            enum ENState
                            {
                                eState_Idle, 
                                eState_Preview, 
                                eState_PreCapture, 
                                eState_Capture, 
                            };

public:     ////            Operations.
    virtual                 ~IState() {}
    virtual char const*     getName() const                                 = 0;
    virtual ENState         getEnum() const                                 = 0;
    //
public:     ////            Interfaces
    virtual status_t        onStartPreview(IStateHandler* pHandler)         = 0;
    virtual status_t        onStopPreview(IStateHandler* pHandler)          = 0;
    //
    virtual status_t        onPreCapture(IStateHandler* pHandler)           = 0;
    virtual status_t        onCapture(IStateHandler* pHandler)              = 0;
    virtual status_t        onCaptureDone(IStateHandler* pHandler)          = 0;
    virtual status_t        onCancelCapture(IStateHandler* pHandler)        = 0;

};


/*******************************************************************************
*   IStateManager
*******************************************************************************/
class IStateManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            
    typedef IState::ENState ENState;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Observer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            
                            struct IObserver
                            {
                                virtual             ~IObserver() {}
                                virtual void        notify(ENState eNewState)   = 0;
                            };

                            class StateObserver : public IObserver
                            {
                            public:
                                virtual void        notify(ENState eNewState);
                                virtual status_t    waitState(ENState eState, nsecs_t const timeout = -1);
                                                    StateObserver(IStateManager*);
                                                    ~StateObserver();
                            protected:
                                IStateManager*const mpStateManager;
                                Mutex               mLock;
                                Condition           mCond;
                                ENState volatile    meCurrState;
                            };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            Attributes.
    virtual IState*         getCurrentState() const                         = 0;
    virtual bool            isState(ENState const eState)                   = 0;

public:     ////            Operations.
#if 0
    //
    //  eState:
    //      [in] the state to wait.
    //
    //  timeout:
    //      [in] the timeout to wait in nanoseconds. -1 indicates no timeout.
    //
    virtual status_t        waitState(
                                ENState const eState, 
                                nsecs_t const timeout = -1
                            )                                               = 0;
#endif
    virtual status_t        transitState(ENState const eNewState)           = 0;

    virtual bool            registerOneShotObserver(IObserver* pObserver)   = 0;
    virtual void            unregisterObserver(IObserver* pObserver)        = 0;

public:     ////            Instantiation.
    virtual                 ~IStateManager() {}

    static  IStateManager*  inst();

};


/*******************************************************************************
*
*******************************************************************************/
}; // namespace NSMtkPhotoCamAdapter
}; // namespace android
#endif // _MTK_HAL_CAMADAPTER_MTKPHOTO_INC_ISTATE_H_

