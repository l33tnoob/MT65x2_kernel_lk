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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/IState.h>
#include "State.h"
using namespace NSMtkPhotoCamAdapter;
//


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

namespace android {
namespace NSMtkPhotoCamAdapter {
/*******************************************************************************
*   StateManager
*******************************************************************************/
class StateManager : public IStateManager
{
public:     ////            Attributes.
    virtual IState*         getCurrentState() const { return mpCurrState; }

    virtual bool            isState(ENState const eState);

public:     ////            Operations.
#if 0
    //
    //  eState:
    //      [in] the state to wait.
    //
    //  timeout:
    //      [in] the timeout to wait in nanoseconds. -1 indicates no timeout.
    //
    virtual status_t        waitState(ENState const eState, nsecs_t const timeout = -1);
#endif
    virtual status_t        transitState(ENState const eNewState);

    virtual bool            registerOneShotObserver(IObserver* pObserver);
    virtual void            unregisterObserver(IObserver* pObserver);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////            Instantiation.
    static IState*          getStateInst(ENState const eState);
    friend class            StateObserver;

public:     ////            Instantiation.
                            StateManager();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////            Data Members.
    Mutex                   mStateLock;
    Condition               mStateCond;
    IState* volatile        mpCurrState;        //  Pointer to the current state.
    //
    typedef List<IObserver*>ObserverList_t;
    Mutex                   mObserverLock;
    ObserverList_t          mObserverList;

};

}; // namespace NSMtkPhotoCamAdapter
}; // namespace android

/*******************************************************************************
 *  
 ******************************************************************************/
#define STATE_INST_DEFINITION(_name_) static State##_name_ gSingleton_State##_name_(IState::eState_##_name_)
STATE_INST_DEFINITION(Idle);
STATE_INST_DEFINITION(Preview);
STATE_INST_DEFINITION(PreCapture);
STATE_INST_DEFINITION(Capture);


IState*
StateManager::
getStateInst(ENState const eState)
{
    switch  (eState)
    {
#define STATE_ENUM_TO_INST(_name_)\
    case IState::eState_##_name_:\
        {\
            return  &gSingleton_State##_name_;\
        }

    STATE_ENUM_TO_INST(Idle);
    STATE_ENUM_TO_INST(Preview);
    STATE_ENUM_TO_INST(PreCapture);
    STATE_ENUM_TO_INST(Capture);
    default:
        MY_LOGW("bad eState(%d)", eState);
        break;
    };
    return  NULL;
}


/*******************************************************************************
 *  
 ******************************************************************************/
IStateManager*
IStateManager::
inst()
{
    static  StateManager singleton;
    return  &singleton;
}


/*******************************************************************************
 *  
 ******************************************************************************/
StateManager::
StateManager()
    : IStateManager()
    , mStateLock()
    , mStateCond()
    , mpCurrState(getStateInst(StateBase::eState_Idle))
    //
    , mObserverLock()
    , mObserverList()
    //
{
}


/*******************************************************************************
 *  
 ******************************************************************************/
bool
StateManager::
isState(ENState const eState)
{
    IState*const pWaitedState = getStateInst(eState);
    //
    Mutex::Autolock _lock(mStateLock);
//    MY_LOGD_IF(1, "(%d)[%s] current/waited=%s/%s", ::gettid(), __FUNCTION__, mpCurrState->getName(), pWaitedState->getName());
    if  ( pWaitedState != mpCurrState )
    {
        MY_LOGW("current/waited=%s/%s", mpCurrState->getName(), pWaitedState->getName());
        return  false;
    }
    //
    return  true;
}


/*******************************************************************************
 *  
 ******************************************************************************/
#if 0
status_t
StateManager::
waitState(ENState const eState, nsecs_t const timeout /*= -1*/)
{
    status_t status = OK;
    //
    IState*const pWaitedState = getStateInst(eState);
    //
    Mutex::Autolock _lock(mStateLock);
    if  ( pWaitedState != mpCurrState )
    {
        MY_LOGD_IF(1, "current/waited=%s/%s, timeout(%lld)", mpCurrState->getName(), pWaitedState->getName(), timeout);
        switch  (timeout)
        {
        case 0:     //  not wait.
            status = TIMED_OUT;
            break;
        case -1:    //  wait without timeout.
            status = mStateCond.wait(mStateLock);
            break;
        default:    //  wait with a given timeout.
            status = mStateCond.waitRelative(mStateLock, timeout);
            break;
        }
        //
        if  ( pWaitedState != mpCurrState || OK != status ) {
            MY_LOGW("Timeout: current/waited=%s/%s, status[%s(%d)]", mpCurrState->getName(), pWaitedState->getName(), ::strerror(-status), -status);
        }
    }
    //
    return  status;
}
#endif

/*******************************************************************************
 *  
 ******************************************************************************/
status_t
StateManager::
transitState(ENState const eNewState)
{
    IState*const pNewState = getStateInst(eNewState);
    if  ( ! pNewState )
    {
        MY_LOGW("pNewState==NULL (eNewState:%d)", eNewState);
        return  INVALID_OPERATION;
    }
    //
    {
        Mutex::Autolock _lock(mStateLock);
        MY_LOGI("%s --> %s", mpCurrState->getName(), pNewState->getName());
        mpCurrState = pNewState;
        mStateCond.broadcast();
    }
    //
    {
        Mutex::Autolock _lock(mObserverLock);
        for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
        {
            (*it)->notify(eNewState);
        }
        mObserverList.clear();
    }
    return  OK;
}


/*******************************************************************************
 *  
 ******************************************************************************/
bool
StateManager::
registerOneShotObserver(IObserver* pObserver)
{
    if  ( pObserver == 0 ) {
        return  false;
    }
    //
    Mutex::Autolock _lock(mObserverLock);
    pObserver->notify(getCurrentState()->getEnum());
    mObserverList.push_back(pObserver);
    return  true;
}


/*******************************************************************************
 *  
 ******************************************************************************/
void
StateManager::
unregisterObserver(IObserver* pObserver)
{
    Mutex::Autolock _lock(mObserverLock);
    //
    for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
    {
        if  ( pObserver == (*it) )
        {
            MY_LOGD("(%p)", (*it));
            mObserverList.erase(it);
            break;
        }
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateObserver
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*******************************************************************************
 *  
 ******************************************************************************/
IStateManager::
StateObserver::
StateObserver(IStateManager* pStateManager)
    : mpStateManager(pStateManager)
    , mLock()
    , mCond()
    , meCurrState(pStateManager->getCurrentState()->getEnum())
{
}


/*******************************************************************************
 *  
 ******************************************************************************/
IStateManager::
StateObserver::
~StateObserver()
{
    mpStateManager->unregisterObserver(this);
}


/*******************************************************************************
 *  
 ******************************************************************************/
void
IStateManager::
StateObserver::
notify(ENState eNewState)
{
    Mutex::Autolock _lock(mLock);
    meCurrState = eNewState;
    mCond.broadcast();
}


/*******************************************************************************
 *  
 ******************************************************************************/
status_t
IStateManager::
StateObserver::
waitState(ENState eState, nsecs_t const timeout)
{
    status_t status = OK;
    //
    Mutex::Autolock _lock(mLock);
    //
    ENState eInitState = meCurrState;
    //
    if  ( eState != meCurrState )
    {
        MY_LOGD_IF(
            1, 
            "<StateObserver> + now/current/waited=%s/%s/%s, timeout(%lld)", 
            mpStateManager->getCurrentState()->getName(), 
            StateManager::getStateInst(meCurrState)->getName(), 
            StateManager::getStateInst(eState)->getName(), 
            timeout
        );
        switch  (timeout)
        {
        case 0:     //  not wait.
            status = TIMED_OUT;
            break;
        case -1:    //  wait without timeout.
            status = mCond.wait(mLock);
            break;
        default:    //  wait with a given timeout.
            status = mCond.waitRelative(mLock, timeout);
            break;
        }
        //
        if  ( eState != meCurrState )
        {
            status = FAILED_TRANSACTION;
        }
        //
        if  ( OK != status )
        {
            MY_LOGW(
                "<StateObserver> Timeout: now/current/waited/init=%s/%s/%s/%s, status[%s(%d)]", 
                mpStateManager->getCurrentState()->getName(), 
                StateManager::getStateInst(meCurrState)->getName(), 
                StateManager::getStateInst(eState)->getName(), 
                StateManager::getStateInst(eInitState)->getName(), 
                ::strerror(-status), -status
            );
        }
    }
    //
    return  status;
}

