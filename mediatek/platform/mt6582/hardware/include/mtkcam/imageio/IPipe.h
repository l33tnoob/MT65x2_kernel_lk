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
* @file IPipe.h
*
* Pipe Header File
*/

#ifndef _ISPIO_I_PIPE_H_
#define _ISPIO_I_PIPE_H_

#include <vector>
using namespace std;

#include <mtkcam/common.h>

#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/imageio/ispio_pipe_identity.h>
#include <mtkcam/imageio/ispio_pipe_callbacks.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>


namespace NSImageio
{
    namespace NSIspio
    {        
        /**
             * @brief  Interface of pipe command
             */
        class IPipeCommand
        {
        public:

            /**
                   *@brief Destructor
                   */
            virtual ~IPipeCommand() {}

            /**
                   *@brief execute
                   */
            virtual MBOOL   execute() = 0;
        };
        
        /**
             * @brief  Interface of Pipe
             */
        class IPipe
        {
        public:

            /**
                   *@brief Get pipe name
                   *
                   *@return
                   *-char string with pipe name, otherwise indicates fail
                   */
            virtual char const* getPipeName() const = 0;

            /**
                   *@brief Get pipe ID
                   *
                   *@return
                   *-pipe ID, otherwise indicates fail
                   */
            virtual EPipeID getPipeId() const = 0;

            /**
                   *@brief Get last error
                   *
                   *@return
                   *-error code, otherwise indicates fail
                   */
            virtual MINT32 getLastErrorCode() const = 0;

        protected:

            /**
                   *@brief Destructor
                   */
            virtual ~IPipe() {}

        public:
            
            /**
                   *@brief Destory Pipe obejct                   
                   */
            virtual MVOID destroyInstance() = 0;

            /**
                   *@brief Initialize
                   */
            virtual MBOOL   init() = 0;

            /**
                   *@brief Uninitialize                   
                   */
            virtual MBOOL   uninit() = 0;

        public:

            /**
                   *@brief Set callback function
                   *
                   *@param[in] notify_cb : notift callback
                   *@param[in] data_cb : date callback
                   *@param[in] user : callback user                   
                   */
            virtual MVOID setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user) = 0;
            
            /**
                   *@brief Check if specific notify message is enabled or not
                   *
                   *@param[in] i4MsgTypes : notify message type
                   *
                   *@return
                   *-MTRUE indicates enable, otherwise indicates disable
                   */
            virtual MBOOL isNotifyMsgEnabled(MINT32 const i4MsgTypes) const = 0;

            /**
                   *@brief Enable specific notify message
                   *
                   *@param[in] i4MsgTypes : notify message type                  
                   */
            virtual MVOID enableNotifyMsg(MINT32 const i4MsgTypes) = 0;

            /**
                   *@brief Disable specific notify message
                   *
                   *@param[in] i4MsgTypes : notify message type                  
                   */
            virtual MVOID disableNotifyMsg(MINT32 const i4MsgTypes) = 0;
            
            /**
                   *@brief Check if specific date message is enabled or not
                   *
                   *@param[in] i4MsgTypes : date message type
                   *
                   *@return
                   *-MTRUE indicates enable, otherwise indicates disable
                   */
            virtual MBOOL isDataMsgEnabled(MINT32 const i4MsgTypes) const = 0;

            /**
                   *@brief Enable specific data message
                   *
                   *@param[in] i4MsgTypes : data message type
                   */
            virtual MVOID enableDataMsg(MINT32 const i4MsgTypes) = 0;

            /**
                   *@brief Disable specific data message
                   *
                   *@param[in] i4MsgTypes : data message type
                   */
            virtual MVOID disableDataMsg(MINT32 const i4MsgTypes) = 0;

        public:

            /**
                   *@brief Trigger pipe
                   */
            virtual MBOOL start() = 0;

            /**
                   *@brief Stop pipe
                   */
            virtual MBOOL   stop()  = 0;
        public:

            /**
                   *@brief Add a buffer into the specified non-sensor input port
                   *
                   *@param[in] portID : The input port ID to add. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to add
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo) = 0;

            /**
                   *@brief De-queue a buffer information for a specified input port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds. If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready. 
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL dequeInBuf( PortID const portID, 
                                         QTimeStampBufInfo& rQBufInfo, 
                                         MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0;
            
            /**
                   *@brief En-queue a buffer into the specified output port
                   *
                   *@param[in] PortID : The output port ID to enqueue. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to enqueue
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo) = 0;

            /**
                   *@brief De-queue a buffer information for a specified output port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready.
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL dequeOutBuf( PortID const portID, 
                                            QTimeStampBufInfo& rQBufInfo, 
                                            MUINT32 const u4TimeoutMs = 0xFFFFFFFF)  = 0;

        public:
  
            /**
                   *@brief Configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts) = 0;

            /**
                   *@brief Partial update configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure and an error code can be retrived by getLastErrorCode()
                   */
            virtual MBOOL configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts) = 0;

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
            virtual MBOOL irq(EPipePass pass, EPipeIRQ irq_int) = 0;            
            
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
            virtual MBOOL sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3) = 0;
        };    
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_PIPE_H_

