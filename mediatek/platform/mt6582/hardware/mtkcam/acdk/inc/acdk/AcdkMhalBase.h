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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMhalBase.h


#ifndef _ACDKMHALBASE_H_
#define _ACDKMHALBASE_H_

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkCallback.h"

#include "mtkcam/common.h"
#include <mtkcam/imageio/IPipe.h>
#include "mtkcam/hal/sensor_hal.h"
#include <mtkcam/drv/imem_drv.h>

namespace NSAcdkMhal 
{   
    /**
         *@enum acdkMhalState_enum
         *@brief State enum of AcdkMhal
       */
    typedef enum acdkMhalState_enum 
    {
        ACDK_MHAL_NONE           = 0x0000,        
        ACDK_MHAL_INIT           = 0x0001,
        ACDK_MHAL_IDLE           = 0x0002,
        //
        ACDK_MHAL_PREVIEW_MASK   = 0x0100,
        ACDK_MHAL_PREVIEW        = 0x0100,
        //
        ACDK_MHAL_CAPTURE_MASK   = 0x0200,
        ACDK_MHAL_PRE_CAPTURE    = 0x0200,
        ACDK_MHAL_CAPTURE        = 0x0201,
        //
        ACDK_MHAL_PREVIEW_STOP   = 0x0400,
        //
        ACDK_MHAL_UNINIT         = 0x0800,
        //
        ACDK_MHAL_ERROR          = 0x8000
    } acdkMhalState_e;
    
    /**  
         *@struct acdkMhalCamFrame_s
         *@brief  Struct for paramter of prview frame
       */
    typedef struct acdkMhalCamFrame_s 
    {
        MUINT32 w;
        MUINT32 h;
        MUINT32 orientation;
        MBOOL flip;
        MUINT32 stride[3];
        MUINT32 frmFormat;
    } acdkMhalCamFrame_t;
   
    /**  
         *@struct acdkMhalPrvParam_s
         *@brief  Struct for preview HW scenario parameter
       */
    typedef struct acdkMhalPrvParam_s
    {       
        NSImageio::NSIspio::EScenarioID scenarioHW;
        halSensorDev_e sensorID;
        MUINT32 sensorWidth;
        MUINT32 sensorHeight;
        MUINT32 sensorStride[3];
        MUINT32 sensorType;
        MUINT32 sensorFormat;
        MUINT32 sensorColorOrder;      
        MUINT32 mu4SensorDelay;
        IMEM_BUF_INFO *imgImemBuf;
        IMEM_BUF_INFO *dispImemBuf;        
        acdkMhalCamFrame_t frmParam;       
        acdkObserver acdkMainObserver;
        MBOOL IsFactoryMode;
    }acdkMhalPrvParam_t;    
    
    /**
         *@class AcdkMhalBase
         *@brief This class handle opeation with ISP
       */
    class AcdkMhalBase
    {
    public:
            /**
                       *@brief AcdkMhalBase constructor
                     */
            AcdkMhalBase() {};

            /**
                       *@brief AcdkMhalBase destructor
                     */
            virtual ~AcdkMhalBase() {};

            /**                       
                       *@brief Create AcdkMhalBase Object
                       *@note Actually, it will create AcdkMhalPure or AcdkMhalEng object
                     */
            static AcdkMhalBase *createInstance();

            /**                       
                       *@brief Destory AcdkMhalBase Object
                     */
            virtual void destroyInstance() = 0;

            /**                       
                       *@brief Set current sate of AcdkMhalBase to newState
                       *
                       *@param[in] newState : new state
                     */
            virtual MVOID acdkMhalSetState(acdkMhalState_e newState) = 0;

            /**                       
                       *@brief Get current sate of AcdkMhalBase 
                     */
            virtual acdkMhalState_e acdkMhalGetState() = 0;

            /**                       
                       *@brief Indicates whether is ready for capture or not
                       *@return
                       *-MFALSE indicates not ready, MTRUE indicates ready
                     */
            virtual MBOOL acdkMhalReadyForCap() = 0;

            /**                       
                       *@brief Initialize function
                       *@note Must call this function right after createInstance and before other functions
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalInit() = 0;                                   

            /**                       
                       *@brief Uninitialize function
                       *@note Must call this function before destroyInstance
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalUninit() = 0;

            /**                       
                       *@brief Handle callback
                       *
                       *@param[in] a_type : callback type
                       *@param[in] a_addr1 : return data address
                       *@param[in] a_addr2 : return data address
                       *@param[in] a_dataSize : return data size
                     */
            virtual MVOID  acdkMhalCBHandle(MUINT32 a_type, MUINT32 a_addr1, MUINT32 a_addr2 = 0, MUINT32 const a_dataSize = 0) = 0;

            /**                       
                       *@brief Start preview
                       *@note Config and control ISP to start preview
                       *
                       *@param[in] a_pBuffIn : pointer to acdkMhalPrvParam_t data
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewStart(MVOID *a_pBuffIn) = 0;

            /**                       
                       *@brief Stop preview
                       *@note Config and control ISP to stop preview
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewStop() = 0;

            /**                       
                       *@brief Start capture                      
                       *
                       *@param[in] a_pBuffIn
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureStart(MVOID *a_pBuffIn) = 0;

            /**                       
                       *@brief Stop capture   
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureStop() = 0;

            /**                       
                       *@brief Execute preview process  
                       *@note Here is a preview loop
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewProc() = 0;

            /**                       
                       *@brief Change state to preCapture state and do related opertion
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreCapture() = 0;

            /**                       
                       *@brief  Execute capture process
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureProc() = 0;

            /**                       
                       *@brief  Get shutter time in us
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkMhalGetShutTime() = 0;

            /**                       
                       *@brief  Set shutter time
                       *
                       *@param[in] a_time : specific shutter time in us
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MVOID acdkMhalSetShutTime(MUINT32 a_time) = 0;
            /**                       
                       *@brief  Get AF information
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkMhalGetAFInfo() = 0;

    };
};
#endif //end AcdkMhalBase.h 

