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

#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/imageio/ispio_pipe_identity.h>
#include <mtkcam/imageio/ispio_pipe_callbacks.h>

#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>

#include "cam_path.h"


namespace NSImageio 
{
    namespace NSIspio
    {
        /**
             * @brief  PipeImp class
             */
        class PipeImp
        {       
        protected:  // Data Members.
            char const*const    mszPipeName;
            EPipeID const       mePipeID;
            MINT32              mi4ErrorCode;

        public:

            /**
                   *@brief Get pipe name
                   *
                   *@return
                   *-char string with pipe name, otherwise indicates fail
                   */
            virtual char const* getPipeName() const { return mszPipeName; }

            /**
                   *@brief Get pipe ID
                   *
                   *@return
                   *-pipe ID, otherwise indicates fail
                   */
            virtual EPipeID getPipeId() const { return mePipeID; }

            /**
                   *@brief Get last error
                   *
                   *@return
                   *-error code, otherwise indicates fail
                   */
            virtual MINT32 getLastErrorCode() const { return mi4ErrorCode; }
      
        public:

            /**
                   * @brief  Constructor
                   *
                   *@param[in] szPipeName : pipe name
                   *@param[in] ePipeID : pipe ID
                   *@param[in] eScenarioID : HW scenario
                   *@param[in] eScenarioFmt : sensor format
                   */
            PipeImp( char const*const szPipeName,
                       EPipeID const ePipeID,
                       EScenarioID const eScenarioID,
                       EScenarioFmt const eScenarioFmt);
            
            /**
                   * @brief  Destructor
                   */
            virtual ~PipeImp()  {}
      
        protected:  // Data Members.
            
            MVOID*          mpCbUser;           //  Callback user.
            
            //  notify callback
            volatile MINT32       mi4NotifyMsgSet;  ///<  bitset of notify message types.
            PipeNotifyCallback_t  mNotifyCb;        ///<  notify callback function pointer.
            
            //  data callback
            volatile MINT32       mi4DataMsgSet;  ///<  bitset of data message types.
            PipeDataCallback_t    mDataCb;        ///<  data callback function pointer.

        protected:

            /**
                   *@brief Notify callback
                   *
                   *@param[in] msg : PipeNotifyInfo                   
                   */
            virtual MBOOL onNotifyCallback(PipeNotifyInfo const& msg) const;

            /**
                   *@brief Data callback
                   *
                   *@param[in] msg : PipeNotifyInfo                   
                   */
            virtual MBOOL onDataCallback(PipeDataInfo const& msg) const;

        public:

            /**
                   *@brief Set callback function
                   *
                   *@param[in] notify_cb : notift callback
                   *@param[in] data_cb : date callback
                   *@param[in] user : callback user                   
                   */
            virtual MVOID setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user);
            
            /**
                   *@brief Check if specific notify message is enabled or not
                   *
                   *@param[in] i4MsgTypes : notify message type
                   *
                   *@return
                   *-MTRUE indicates enable, otherwise indicates disable
                   */
            inline MBOOL isNotifyMsgEnabled(MINT32 const i4MsgTypes) const { return (i4MsgTypes & mi4NotifyMsgSet); }

            /**
                   *@brief Enable specific notify message
                   *
                   *@param[in] i4MsgTypes : notify message type
                   */
            inline MVOID enableNotifyMsg(MINT32 const i4MsgTypes)  { ::android_atomic_or(i4MsgTypes, &mi4NotifyMsgSet); }

            /**
                   *@brief Disable specific notify message
                   *
                   *@param[in] i4MsgTypes : notify message type
                   */
            inline MVOID disableNotifyMsg(MINT32 const i4MsgTypes)  { ::android_atomic_and(~i4MsgTypes, &mi4NotifyMsgSet); }
            
            /**
                   *@brief Check if specific date message is enabled or not
                   *
                   *@param[in] i4MsgTypes : date message type
                   *
                   *@return
                   *-MTRUE indicates enable, otherwise indicates disable
                   */
            inline MBOOL isDataMsgEnabled(MINT32 const i4MsgTypes) const { return (i4MsgTypes & mi4DataMsgSet); }

            /**
                   *@brief Enable specific data message
                   *
                   *@param[in] i4MsgTypes : data message type
                   */
            inline MVOID enableDataMsg(MINT32 const i4MsgTypes)  { ::android_atomic_or(i4MsgTypes, &mi4DataMsgSet); }

            /**
                   *@brief Disable specific data message
                   *
                   *@param[in] i4MsgTypes : data message type
                   */
            inline MVOID disableDataMsg(MINT32 const i4MsgTypes) { ::android_atomic_and(~i4MsgTypes, &mi4DataMsgSet); }

        public:

            /**
                   *@brief Configure CdpOutPort
                   *
                   *@param[in] oImgInfo : PortInfo
                   *@param[in] a_rotDma : CdpRotDMACfg
                   */
            MBOOL configCdpOutPort(PortInfo const* oImgInfo,CdpRotDMACfg &a_rotDma);

            /**
                   *@brief Configure dma port
                   *
                   *@param[in] portInfo : PortInfo
                   *@param[in] a_dma : IspDMACfg
                   *@param[in] pixel_Byte : byte per pixel
                   *@param[in] swap : swap or not
                   *@param[in] isBypassOffset : bypass offset or not
                   *@param[in] planeNum : plane number
                   */
            MBOOL configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum);

        protected:
            
            EScenarioID const   meScenarioID;
            EScenarioFmt const  meScenarioFmt;

        public:

            /**
                   *@brief Get HW scenario ID
                   */
            inline  MINT32  getScenarioID()     const { return meScenarioID; }

            /**
                   *@brief Get submod                   
                   */
            inline  MINT32  getScenarioSubmode()const { return mapScenarioFormatToSubmode(meScenarioID, meScenarioFmt); }

            /**
                   *@brief Map scenario and format to get submod
                   *
                   *@param[in] eScenarioID : HW scenario
                   *@param[in] eScenarioFmt : sensor format
                   */
            static  MINT32  mapScenarioFormatToSubmode(EScenarioID const eScenarioID, EScenarioFmt const eScenarioFmt);
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_IMP_H_

