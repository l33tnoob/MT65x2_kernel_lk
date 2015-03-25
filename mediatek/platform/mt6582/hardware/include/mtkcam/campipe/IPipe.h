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
#ifndef _MTK_CAMERA_INC_CAMPIPE_IPIPE_H_
#define _MTK_CAMERA_INC_CAMPIPE_IPIPE_H_
//
#include <vector>
using namespace std;
//
//
#include <mtkcam/campipe/_scenario.h>
#include <mtkcam/campipe/_identity.h>
#include <mtkcam/campipe/_callbacks.h>
#include <mtkcam/campipe/_ports.h>
#include <mtkcam/campipe/_buffer.h>
//


/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
////////////////////////////////////////////////////////////////////////////////


/**  
 * @class IPipeCommand
 * @brief Interface of pipe command 
 * @details 
 * The is used for new send command style. 
 *
 */
class IPipeCommand
{
public:
    /**
     * @brief destroy function of IPipeCommand
     *
     * @details      
     *
     * @note 
     * 
     */   	
    virtual ~IPipeCommand() {}  
    /**
     * @brief execute the pipe command 
     *
     * @details      
     *
     * @note 
     * 
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */       
    virtual MBOOL   execute() = 0;
};


/**  
 * @class IPipe
 * @brief Interface of Pipe
 * @details 
 * Define the interface of the pipe for all pipe types 
 *
 */
class IPipe
{
public:     ////    Attributes.
    /**
     * @brief get the pipe name 
     *
     * @details      
     *
     * @note 
     *
     * @return 
     * - The name of the pipe 
     *
     */       
    virtual char const* getPipeName() const = 0;
    /**
     * @brief get the pipe id
     *
     * @details      
     *
     * @note 
     *
     * @return 
     * - The id of the pipe 
     *
     */       
    virtual EPipeID     getPipeId() const = 0;
    /**
     * @brief get the last error code 
     *
     * @details      
     *
     * @note 
     *
     * @return 
     * - The last error code 
     *
     */     
    virtual MINT32      getLastErrorCode() const = 0;

protected:  ////    Constructor/Destructor.
    virtual         ~IPipe() {}

public:     ////    Instantiation.
    /**
     * @brief destroy the pipe instance 
     *
     * @details      
     *
     * @note 
     *
     */
    virtual MVOID   destroyInstance() = 0;
    /**
     * @brief init the pipe 
     *
     * @details      
     *
     * @note 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */        
    virtual MBOOL   init() = 0;
    /**
     * @brief uninit the pipe 
     *
     * @details      
     *
     * @note 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */     
    virtual MBOOL   uninit() = 0;

public:     ////    Callbacks.
    /**
     * @brief set the notify/data callbacks 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] notify_cb: The notify callback function 
     *
     * @param[in] data_cb: The data callback fucntion
     *
     * @param[in] user: The caller
     *
     */
    virtual MVOID   setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user) = 0;
    //
    //  notify callback
    /**
     * @brief check if the notify message is enabled 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The notify message type that want to check. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */            
    virtual MBOOL   isNotifyMsgEnabled(MINT32 const i4MsgTypes) const   = 0;
    /**
     * @brief enable the notify message 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The notiyf message type that want to enable. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */       
    virtual MVOID   enableNotifyMsg(MINT32 const i4MsgTypes)            = 0;
    /**
     * @brief Disable the notify message 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The notify message type that want to disable. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */    
    virtual MVOID   disableNotifyMsg(MINT32 const i4MsgTypes)           = 0;
    //
    //  data callback
    /**
     * @brief check if the data message is enabled 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The data message type that want to check. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */         
    virtual MBOOL   isDataMsgEnabled(MINT32 const i4MsgTypes) const     = 0;
    /**
     * @brief enable the data message 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The data message type that want to enable. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */       
    virtual MVOID   enableDataMsg(MINT32 const i4MsgTypes)              = 0;
    /**
     * @brief Disable the data message 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] i4MsgTypes: The data message type that want to disable. 
     *
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */    
    virtual MVOID   disableDataMsg(MINT32 const i4MsgTypes)             = 0;

public:     ////    Operations.
    /**
     * @brief Start the pipe 
     *
     * @details      
     *
     * @note 
     * 
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */
    virtual MBOOL   start() = 0;    
    /**
     * @brief Stop the pipe 
     *
     * @details      
     *
     * @note 
     * 
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */    
    virtual MBOOL   stop()  = 0;

public:     ////    Buffer Quening.
    /**
     * @brief En-queue a buffer into the specified output port.
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] ePortID: The output port ID to enqueue.  \n
     *     The port ID must be one of which specified when configPipe().
     * @param[in] rQBufInfo: Reference to a QBufInfo structure containing a buffer \n 
     *     information to enqueue.
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */     
    virtual MBOOL   enqueBuf(PortID const ePortID, QBufInfo const& rQBufInfo)     = 0;
    /**
     * @brief De-queue a buffer information for a specified output port ID.
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] ePortID: The output port ID to dequeue.  \n
     *                             The port ID must be one of which specified when configPipe().
     * @param[out] rQBufInfo: Reference to a QTimeStampBufInfo structure to store the de-queued \n 
     *                                  buffer information
     * @param[in] u4TimeoutMs: timeout in milliseconds \n
     *                                      If u4TimeoutMS > 0, a timeout is specified, and this call will \n
     *                                      be blocked until any buffer is ready. \n
     *                                      If u4TimeoutMS = 0, this call must return immediately no matter \n
     *                                      whether any buffer is ready or not. \n
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */     
    virtual MBOOL   dequeBuf(
                        PortID const ePortID, 
                        QTimeStampBufInfo& rQBufInfo, 
                        MUINT32 const u4TimeoutMs = 0xFFFFFFFF
                    )  = 0;

public:     ////    Settings.
    /**
     * @brief Configure in/out ports of this pipe.
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] vInPorts: Reference to the vector of input ports.
     * @param[in] vOutPorts:  Reference to the vector of output ports.
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */      
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, MBOOL concurrency = MFALSE) = 0;

public:     //// info 
    /**
     * @brief query in/out ports property of this pipe.
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] vInPorts: Reference to the vector of input ports.
     * @param[in] vOutPorts:  Reference to the vector of output ports.
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */      
    virtual MBOOL   queryPipeProperty(vector<PortProperty> &vInPorts, vector<PortProperty> &vOutPorts) = 0; 
};
 

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe
#endif  //  _MTK_CAMERA_INC_CAMPIPE_IPIPE_H_

