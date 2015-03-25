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
* @file CamIOPipe.h
*
* CamIOPipe Header File
*/

#ifndef _ISPIO_CAMIO_PIPE_H_
#define _ISPIO_CAMIO_PIPE_H_

#include <vector>

using namespace std;

#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>

#include "camera_isp.h" 			//For Isp function ID
#include "isp_drv.h"
#include "mtkcam/hal/sensor_hal.h"
#include "kd_imgsensor_define.h"

//#include "imgsensor_drv_ldvt.h"


//
#if (PLATFORM_VERSION_MAJOR == 2)
#include <utils/threads.h>         	// For android::Mutex.
#else
#include <utils/Mutex.h>          	// For android::Mutex.
#endif

using namespace android;
/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class PipeImp;


/*******************************************************************************
*
********************************************************************************/
class CamIOPipe : public PipeImp
{
public:     ////    Constructor/Destructor.
                    CamIOPipe(
                        char const*const szPipeName,
                        EPipeID const ePipeID,
                        EScenarioID const eScenarioID,
                        EScenarioFmt const eScenarioFmt
                    );
                    virtual ~CamIOPipe();

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start();
    virtual MBOOL   startCQ0();                            // no-used
    virtual MBOOL   startCQ0B();                           // no-used
    virtual MBOOL   stop();

public:     ////    Buffer Quening.
    virtual MBOOL   enqueInBuf(                            // no-used
			PortID const portID, 
			QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeInBuf(	                           // no-used
			PortID const portID, 	
			QTimeStampBufInfo& rQBufInfo, 
			MUINT32 const u4TimeoutMs = 0xFFFFFFFF);
    virtual MBOOL   enqueOutBuf(
			PortID const portID, 
			QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeOutBuf(
			PortID const portID, 	
			QTimeStampBufInfo& rQBufInfo, 
			MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

public:     ////    Settings.
    virtual MBOOL   configPipe(
			vector<PortInfo const*>const& vInPorts, 
			vector<PortInfo const*>const& vOutPorts);
    virtual MBOOL   configPipeUpdate(
			vector<PortInfo const*>const& vInPorts, 
			vector<PortInfo const*>const& vOutPorts);
	
public:     ////    Commands.
    virtual MBOOL   onSet2Params(                          // no-used
			MUINT32 const u4Param1, 
			MUINT32 const u4Param2);
    virtual MBOOL   onGet1ParamBasedOn1Input(              // no-used
			MUINT32 const u4InParam, 
			MUINT32*const pu4OutParam);

public:     ////    Interrupt handling
	virtual MBOOL   irq(
			EPipePass pass, 
			EPipeIRQ irq_int);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(                           // no-used
			MINT32 cmd, 
			MINT32 arg1, 
			MINT32 arg2, 
			MINT32 arg3);

	
private:
    IspDrv*                 m_pIspDrv;
    vector<BufInfo>         m_vBufImgo;
    mutable Mutex       	m_queLock;
    //
    EConfigSettingStage     m_settingStage;

};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_CAMIO_PIPE_H_
