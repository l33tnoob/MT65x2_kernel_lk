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

#ifndef _MTK_CAMERA_INC_CAMPIPE_ICAMIO_PIPE_H_
#define _MTK_CAMERA_INC_CAMPIPE_ICAMIO_PIPE_H_

/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {

/**  
 * @enum ECamIOPipeCmd
 * @brief CamIO pipe command
 */
enum ECamIOPipeCmd {
    ECamIOPipeCmd_QUERY_BAYER_RAW_SRIDE  = 0x1001,         /*!<  for query bayer raw stride.  */
}; 
    

/**  
 * @class ICamIOPipe
 * @brief Interface of CamIOPipe 
 * @details 
 * ZSD Scenario is used to wraper the camIO pipe. \n
 * The data path will be TG --> ISP --> Mem, used to dump image from sensor \n
 *
 */
class ICamIOPipe : public IPipe
{
public:
    static EPipeID const ePipeID = ePipeID_1x2_Sensor_Tg_Isp_Mem;

public:     ////    Instantiation.
    /**
     * @brief Create the CamIOPipe instance 
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] eSWScenarioID: The SW scenario 
     * @param[in] eScenarioFmt: The SW sensor scenario format 
     *      
     * @return 
     * The ICamIOPipe instance. 
     *
     */   
    static ICamIOPipe* createInstance(ESWScenarioID const eSWScenarioID, EScenarioFmt const eScenarioFmt);

public:
    /**
     * @brief The old style sendcommand for extension
     *
     * @details      
     *
     * @note 
     * 
     * @param[in] cmd: the input command 
     * @param[in] arg1: input argument 1        
     * @param[in] arg2: input argument 2             
     * @param[in] arg3: input argument 3                
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */   	
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3) = 0; 
    /**
     * @brief Wait for HW signal. 
     *
     * @details      
     *
     * @note 
     * Only support the pipe that IN port is from sensor 
     * 
     * @param[in] ePipeSignal: The pipe signal 
     * @param[in] u4TimeoutMs: The time out in ms 
     *      
     * @return 
     * None 
     */        
    virtual MVOID   waitSignal(EPipeSignal ePipeSignal, MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0; 
    /**
     * @brief Start to do one shot. 
     *
     * @details      
     *
     * @note 
     * Only suuport the pipe that IN port is from sensor 
     *      
     * @return 
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */      
    virtual MBOOL   startOne(); 


}; 

}; //namespace NCCamPipe
#endif  //  _MTK_CAMERA_INC_CAMPIPE_ICAMIO_PIPE_H_
