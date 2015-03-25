/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "aaa_hal/3Athread"
//
#include <utils/threads.h>
#include <utils/List.h>
using namespace android;
//
#include <mtkcam/common.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <aaa_state.h>
using namespace NS3A;
//
//
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

/******************************************************************************
*
*******************************************************************************/
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
		  do { \
			  aee_system_exception( \
				  "Hal3A", \
				  NULL, \
				  DB_OPT_DEFAULT, \
				  String); \
		  } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::createThread()
{
    sem_init(&mSem, 0, 0);
    sem_init(&semAFProcEnd, 0, 0);
    pthread_create(&mThread, NULL, onThreadLoop, this);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::destroyThread()
{
    MY_LOG("+");

    // post exit
    addCommandQ(ECmd_Uninit);

    //
    pthread_join(mThread, NULL);

    //
    MY_LOG("-");    
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::changeThreadSetting()
{
    // (1) set name 
    ::prctl(PR_SET_NAME,"3ATHREAD", 0, 0, 0);

    // (2) set policy/priority
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_3A_THREAD;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOG(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3A::sendCommand(ECmd_T const r3ACmd, MINT32 const i4Arg)
{
	if (IState::getCurrStateEnum() == eState_AF)
		if ((r3ACmd == ECmd_RecordingStart) || (r3ACmd == ECmd_RecordingEnd))
		{
			MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::sendCommand] eState_AF: get r3ACmd(%d)", r3ACmd);
			cancelAutoFocus();
		}
		
    switch (r3ACmd){
        //type 1: run command by 3A thread
        case ECmd_CameraPreviewStart:
        case ECmd_CamcorderPreviewStart:
        case ECmd_PrecaptureEnd:
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
        break;
        
        //type 2: without wait
        case ECmd_Update:   
            addCommandQ(r3ACmd);            
        break;
        
        //type 3: clear previous commands and run current command by 3A thread
        case ECmd_PrecaptureStart: //from type1 to type3, avoid capture delay
		case ECmd_RecordingStart: //from type1 to type3, avoid recording delay
		case ECmd_RecordingEnd: //from type1 to type3, avoid recording stop delay
        case ECmd_CameraPreviewEnd:
        case ECmd_CamcorderPreviewEnd:
        case ECmd_Uninit:               
            clearCommandQ();
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
        break;

        //type 4: run current command directly by caller thread
        default:
            MBOOL bRet = postCommand(r3ACmd, i4Arg);
			if (!bRet) AEE_ASSERT_3A_HAL("type4 postCommand fail.");
        break;
    }
    
    return true;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::clearCommandQ()
{
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mCmdQ.size();

    for (List<ECmd_T>::iterator it = mCmdQ.begin(); it != mCmdQ.end();)
    {
        if ((*it == ECmd_Update) && (mCmdQ.size() >= 2)) //add size limitation to avoid mCmdQ deadlock
    	{
            MY_LOG("cmd size(%d), clear(%d), ", mCmdQ.size(), *it);            
            it = mCmdQ.erase(it);
        }
        else //include 2 cases; 1). keep all cmds except for ECmd_Update, 2). keep at least 1 ECmd_Update in mCmdQ
        {
			MY_LOG("Command not killed: cmd size(%d), beginning cmd(%d), ", mCmdQ.size(), *it);
			it++;
        }
    }
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::addCommandQ(ECmd_T const &r3ACmd)
{
    Mutex::Autolock autoLock(mModuleMtx);

    MY_LOG_IF(LOG_PERFRAME_ENABLE, "mCmdQ size(%d) + cmd(%d)", mCmdQ.size(), r3ACmd);

    mCmdQ.push_back(r3ACmd);
    mCmdQCond.broadcast();
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3A::getCommand(ECmd_T &rCmd)
{
    Mutex::Autolock autoLock(mModuleMtx);

    if (mCmdQ.size() == 0)
    {
        mCmdQCond.wait(mModuleMtx);
    }
	if (mCmdQ.size() == 0) 
		MY_ERR("mCmdQ.size() = %d after mCmdQCond.wait(), failed(0x%x)", mCmdQ.size(), E_3A_ERR);
	
    MY_LOG("CmdQ-size(%d), todo(%d)", mCmdQ.size(), *mCmdQ.begin());

    rCmd = *mCmdQ.begin();
    mCmdQ.erase(mCmdQ.begin());

    if (rCmd == ECmd_Uninit){
        return MFALSE;
    }
    else {
        return MTRUE;
    }
}


/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3A::onThreadLoop(MVOID *arg)
{
    MY_LOG("+");

    ECmd_T rCmd;
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);

    // (1) change thread setting
    _this->changeThreadSetting();

    // (2) thread-in-loop
    while (_this->getCommand(rCmd))
    {
        MY_LOG_IF(LOG_PERFRAME_ENABLE, "rCmd(%d)+", rCmd);
        //
        switch (rCmd)
        {
            case ECmd_Update:
                //
                //::sem_post(&_this->mSem);
                //
                if ( ! _this->isReadyToCapture())
                {   
					MY_LOG_IF(sm_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::onThreadLoop] start waitVSirq.");
                    _this->waitVSirq();
					MY_LOG_IF(1, "[Hal3A::onThreadLoop] waitVSirq done.");
                    AaaTimer localTimer("VsyncUpdate", _this->getSensorDev(), (Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
                    if ( ! _this->postCommand(rCmd))
                    {
                        MY_ERR("Cmd(%d) failed(0x%x)", rCmd, _this->getErrorCode());  
						AEE_ASSERT_3A_HAL("onThreadLoop ECmd_Update fail.");
                    }
                    localTimer.printTime();
                }
                else 
                {
                    MY_LOG("already ready to capture. save one VS");
                }
            break;
            
            default:
                //
                if ( ! _this->postCommand(rCmd))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd, _this->getErrorCode());  
					AEE_ASSERT_3A_HAL("onThreadLoop postCommand fail.");
                }
                //
                ::sem_post(&_this->mSem);
            break;
        }
        //         
        //
        MY_LOG_IF(LOG_PERFRAME_ENABLE, "rCmd(%d)-", rCmd);        
    }

    MY_LOG("-");

    return NULL;
}

