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

#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>

#include "cam_path.h"
#include "mtkcam/hal/sensor_hal.h"

//TODO:remove later
#include "isp_function.h"
#include "kd_imgsensor_define.h"

//#include "imgsensor_drv_ldvt.h"


namespace NSImageio 
{
    namespace NSIspio
    {
        class PipeImp;

        /**
             * @brief  CamIOPipe class
             */
        class CamIOPipe : public PipeImp
        {
        public:

            /**
                   * @brief  Constructor
                   *
                   *@param[in] szPipeName : pipe name
                   *@param[in] ePipeID : pipe ID
                   *@param[in] eScenarioID : HW scenario
                   *@param[in] eScenarioFmt : sensor format
                   */
            CamIOPipe( char const*const szPipeName,
                           EPipeID const ePipeID,
                           EScenarioID const eScenarioID,
                           EScenarioFmt const eScenarioFmt);

            /**
                   * @brief  Destructor
                   */
            virtual ~CamIOPipe();

        public:

            /**
                   * @brief  Initialize
                   */
            virtual MBOOL init();

            /**
                   * @brief  Uninitialize
                   */
            virtual MBOOL uninit();

        public:

            /**
                   * @brief  Trigger CamIOPipe
                   */
            virtual MBOOL start();

            /**
                   * @brief  Trigger command queue 0
                   */
            virtual MBOOL startCQ0();

            /**
                   * @brief  Trigger command queue 0B
                   */
            virtual MBOOL startCQ0B();

            /**
                   * @brief  Stop CamIOPipe
                   */
            virtual MBOOL stop();

        public:

            /**
                   *@brief Add a buffer into the specified non-sensor input port
                   *
                   *@param[in] portID : The input port ID to add. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to add
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo);

            /**
                   *@brief De-queue a buffer information for a specified input port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds. If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready. 
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

            /**
                   *@brief En-queue a buffer into the specified output port
                   *
                   *@param[in] PortID : The output port ID to enqueue. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to enqueue
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo);

            /**
                   *@brief De-queue a buffer information for a specified output port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready.
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

        public:

            /**
                   *@brief Configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);

            /**
                   *@brief Partial update configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
        public:

            /**
                   *@brief Set 2 parameters
                   *                   
                   *@param[in] u4Param1 : command 1
                   *@param[in] u4Param2 : command 2
                   */
            virtual MBOOL onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2);

            /**
                   *@brief Get 1 parameter based on 1 input parameter
                   *                   
                   *@param[in] u4InParam : input parameter
                   *@param[in] pu4OutParam : output parameter
                   */
            virtual MBOOL onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam);

        //extend
        public:

            /**
                   *@brief Interrupt handling
                   *
                   *@param[in] pass : Specific pass
                   *@param[in] irq_int : IRQ type
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
        	virtual MBOOL irq(EPipePass pass, EPipeIRQ irq_int);

        public:

            /**
                   *@brief Original style sendCommand method
                   *
                   *@param[in] cmd : command
                   *@param[in] arg1 : argument 1
                   *@param[in] arg2 : argument 2
                   *@param[in] arg3 : argument 3
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);
        
        private:
            IspDrvShell*            m_pIspDrvShell;
            CamPathPass1            m_CamPathPass1;
            CamPathPass1Parameter   m_camPass1Param;
            vector<BufInfo>         m_vBufImgo;
            vector<BufInfo>         m_vBufImg2o;
            ////image sensor
            MINT32                  m_pass1_CQ;
            EConfigSettingStage     m_settingStage;
            MINT32                  m_CQ0TrigMode;
            MINT32                  m_CQ0BTrigMode;
            MINT32                  m_CQ0CTrigMode;
            //The raw type, 0: pure raw, 1: pre-process raw     
            MUINT32                 m_RawType;
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_CAMIO_PIPE_H_

