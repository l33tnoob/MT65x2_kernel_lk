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
/**
* @file aaa_state.h
* @brief Declarations of 3A state machine
*/

#ifndef _AAA_STATE_H_
#define _AAA_STATE_H_

#include <utils/threads.h>
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
using namespace CPTool;
using namespace android;

namespace NS3A
{

typedef enum {
     eIntent_CameraPreviewStart = ECmd_CameraPreviewStart,
     eIntent_CameraPreviewEnd = ECmd_CameraPreviewEnd,
     eIntent_CamcorderPreviewStart = ECmd_CamcorderPreviewStart,
     eIntent_CamcorderPreviewEnd = ECmd_CamcorderPreviewEnd,
     eIntent_PrecaptureStart = ECmd_PrecaptureStart,
     eIntent_PrecaptureEnd = ECmd_PrecaptureEnd,
     eIntent_CaptureStart = ECmd_CaptureStart,
     eIntent_CaptureEnd = ECmd_CaptureEnd,
     eIntent_RecordingStart = ECmd_RecordingStart,
     eIntent_RecordingEnd = ECmd_RecordingEnd,
     eIntent_VsyncUpdate = ECmd_Update,
     eIntent_AFUpdate = ECmd_AFUpdate,
     eIntent_AFStart = ECmd_AFStart,
     eIntent_AFEnd = ECmd_AFEnd,
     eIntent_Init = ECmd_Init,
     eIntent_Uninit = ECmd_Uninit
} EIntent_T;

typedef enum
{
	eState_Invalid=-1,
    eState_Uninit,
    eState_Init,
    eState_CameraPreview,
    eState_CamcorderPreview,
    eState_Precapture,
    eState_Capture,
    eState_Recording,
    eState_AF
} EState_T;


#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_STATE(String) \
	   do { \
		   aee_system_exception( \
			   "3A State", \
			   NULL, \
			   DB_OPT_DEFAULT, \
			   String); \
	   } while(0)
#else
#define AEE_ASSERT_3A_STATE(String)
#endif

template<EIntent_T eIntent> struct intent2type { enum {v=eIntent}; };

class StateMgr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IState
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State Base Class
 */

class IState
{
public:
    IState(char const*const pcszName);
    virtual ~IState() {};


public:     //    Interfaces
	/**  
	  * @brief send intent eIntent_CameraPreviewStart to 3A state machine, prepare 3A for preview start
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CameraPreviewStart", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CameraPreviewStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_CameraPreviewEnd to 3A state machine, stop 3A preview process
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CameraPreviewEnd", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CameraPreviewEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_CamcorderPreviewStart to 3A state machine, prepare 3A for camcorder preview start
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CamcorderPreviewStart", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CamcorderPreviewStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_CamcorderPreviewEnd to 3A state machine, stop camcorder preview 3A process
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CamcorderPreviewEnd", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CamcorderPreviewEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_PrecaptureStart to 3A state machine, preprocess of capture 3A, enter precapture state
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_PrecaptureStart", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: PrecaptureStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_PrecaptureEnd to 3A state machine, leave from precapture state
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_PrecaptureEnd", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: PrecaptureEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_CaptureStart to 3A state machine, do 3A process before capture (EX: set AE sensor, shutter)
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CaptureStart", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CaptureStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_CaptureEnd to 3A state machine, do 3A process after capture (EX: set AWB, flare ISP HW)
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CaptureEnd", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CaptureEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_RecordingStart to 3A state machine, prepare 3A for recording process
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_RecordingStart", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: RecordingStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_RecordingEnd to 3A state machine, stop recording 3A process
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_RecordingEnd", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: RecordingEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_VsyncUpdate to 3A state machine. This intent is executed RIGHT AFTER wait VS irq. It deals with 3A process each frame, based on current 3A state
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_VsyncUpdate", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_AFUpdate to 3A state machine. This intent is executed by AF thread, RIGHT AFTER wait AF irq. It deals with AF process each frame, base on current 3A state
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFUpdate", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_AFStart to 3A state machine. Enter AF state for continuous AF process
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFStart", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_AFEnd to 3A state machine. leave from AF state then go to previous/next state (depend on the situation)
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFEnd", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_Init to 3A state machine. Init 3A hal
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_Init>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_Init", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: Init");
            return  E_3A_UNSUPPORT_COMMAND;
        }
	/**  
	  * @brief send intent eIntent_Uninit to 3A state machine. Uninit 3A hal
	  */
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_Uninit", m_pcszName);
			AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: Uninit");
            return  E_3A_UNSUPPORT_COMMAND;
        }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    virtual char const* getName() const { return m_pcszName; }

protected:  ////    Data Members.
    Hal3A*              m_pHal3A;
    char const*const    m_pcszName;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  States
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

protected:
	/**  
	  * @brief transit 3A state
	  * @param [in] eCurrentState current 3A state; please refer to Estate_T
	  * @param [in] eNewState new 3A state; please refer to Estate_T
	  */
    MRESULT transitState(EState_T const eCurrState, EState_T const eNewState);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF States
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    typedef enum
    {
        eAFState_None,
        eAFState_PreAF,
        eAFState_AF,
        eAFState_PostAF
    } EAFState_T;

    static EAFState_T m_eAFState;
	/**  
	  * @brief transit AF state
	  * @param [in] eNewAFState new AF state; please refer to EAFState_T
	  */
    inline MVOID transitAFState(EAFState_T const eNewAFState)
    {
        m_eAFState = eNewAFState;
    }
	/**  
	  * @brief get current AF state
	  * @return current AF state; please refer to EAFState_T
	  */
    inline EAFState_T getAFState()
    {
        return m_eAFState;
    }

private:
    static IState*  getStateInstance(EState_T const eState);
    static IState*  sm_pCurrState;  //  Pointer to the current state.
    static EState_T sm_CurrStateEnum;
    static IState*  getCurrStateInstance();
    friend class StateMgr;
public:
	static EState_T getCurrStateEnum()
	{
		return sm_CurrStateEnum;
	}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame count
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static MINT32  m_i4FrameCount;

public:
	/**  
	  * @brief update 3A hal frame count
	  */
    inline MVOID  updateFrameCount()
    {
        if (++m_i4FrameCount >= 0x7FFFFFFF)
        {
            m_i4FrameCount = 0;
        }
    }
	/**  
	  * @brief reset 3A hal frame count (including delay frame count)
	  */
    inline MVOID  resetFrameCount()
    {
            m_i4FrameCount = -3; // delay 2 frames for 3A statistics ready
    }
	/**  
	  * @brief get current 3A hal frame count
	  * @return current frame count
	  */
    inline MINT32  getFrameCount()
    {
            return m_i4FrameCount;
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	3A log control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	static MINT32 sm_3APvLogEnable;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	3A flow control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    static MBOOL sm_bHasAEEverBeenStable;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateUninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Uninit. wait for init by camera process
 */
struct StateUninit : public IState
{
    StateUninit();
    virtual MRESULT  sendIntent(intent2type<eIntent_Init>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateInit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Init. wait for 3A preview start
 */
struct StateInit : public IState
{
    StateInit();
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: camera preview. it can update preview 3A per frame, wait for capture request, and do touch/continuous AF
 */
struct StateCameraPreview : public IState
{
    StateCameraPreview();
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>); //for zsd
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
	virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
	virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCamcorderPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Camcorder preview. it can update preview 3A per frame, wait for recording request, and do touch/continuous AF
 */
struct StateCamcorderPreview : public IState
{
    StateCamcorderPreview();
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>); // for CTS only
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePrecapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Precapture. preparation stage before 3A ready-to-capture
 */
struct StatePrecapture : public IState
{
    StatePrecapture();
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Capture. do capture 3A process for single/multi shot
 */
struct StateCapture : public IState
{
    StateCapture();
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>); // for ZSD capture
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>); // for CTS only
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateRecording
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: Recording. do recording 3A process per frame. support touch AF but not continuous AF
 */
struct StateRecording : public IState
{
    StateRecording();
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);

    MRESULT exitPreview();
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State: touch AF. Touch AF is executed in AF state, then go to previous/next state after AF done
 */
struct StateAF : public IState
{
    StateAF();
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);    
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);




    template<EAFState_T eAFState> struct state2type { enum {v=eAFState}; };

    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_None>);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_None>);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PreAF>);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PreAF>);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_AF>);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PostAF>);
    MRESULT exitPreview();

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  State Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**  
 * @brief 3A State Manager. It handles 3A intents to current state.
 */
class StateMgr
{
public:
    static StateMgr& getInstance()
    {
        static StateMgr singleton;
        return singleton;
    }

    StateMgr() : m_Lock() {}

    MRESULT sendCmd(ECmd_T eCmd)
    {
        Mutex::Autolock lock(m_Lock);

        EIntent_T eNewIntent = static_cast<EIntent_T>(eCmd);

        #define SEND_INTENT(_intent_)\
        case _intent_: return IState::getCurrStateInstance()->sendIntent(intent2type<_intent_>());\

        switch (eNewIntent)
        {
        SEND_INTENT(eIntent_CameraPreviewStart)
        SEND_INTENT(eIntent_CameraPreviewEnd)
        SEND_INTENT(eIntent_CamcorderPreviewStart)
        SEND_INTENT(eIntent_CamcorderPreviewEnd)
        SEND_INTENT(eIntent_PrecaptureStart)
        SEND_INTENT(eIntent_PrecaptureEnd)
        SEND_INTENT(eIntent_CaptureStart)
        SEND_INTENT(eIntent_CaptureEnd)
        SEND_INTENT(eIntent_RecordingStart)
        SEND_INTENT(eIntent_RecordingEnd)
        SEND_INTENT(eIntent_VsyncUpdate)
        SEND_INTENT(eIntent_AFUpdate)
        SEND_INTENT(eIntent_AFStart)
        SEND_INTENT(eIntent_AFEnd)
        SEND_INTENT(eIntent_Init)
        SEND_INTENT(eIntent_Uninit)
        }
        return  -1;
    }

private:
    mutable Mutex m_Lock;
};



};  //  namespace NS3A
#endif // _AAA_STATE_H_

