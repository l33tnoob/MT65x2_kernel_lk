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
#ifndef _MTK_CAMERA_INC_CAMSHOT_ICAMSHOT_H_
#define _MTK_CAMERA_INC_CAMSHOT_ICAMSHOT_H_
//
//

#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camshot/_buffers.h>
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/**  
 * @class ICamShot
 * @brief Interface of CamShot
 */
class ICamShot
{
public:     ////    Attributes.
    /**
     * @brief Get the cam shot name of the caller
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     * -   string for the cam shot name of the caller 
     *
     */ 
    virtual char const* getCamShotName() const = 0;
    /**
     * @brief Get the shot mode of the caller 
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     * -   EShotMode: the shot mode of the caller 
     *
     */     
    virtual EShotMode     getShotMode() const = 0; 
    /**
     * @brief Get the last error code
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     * -   The error code 
     *
     */         
    virtual MINT32      getLastErrorCode() const = 0;

protected:  ////    Constructor/Destructor.
    virtual         ~ICamShot() {}

public:     ////    Instantiation.
    /**
     * @brief destroy the instance 
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     *
     */ 
    virtual MVOID   destroyInstance() = 0;
    /**
     * @brief Init the function 
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable      
     *
     */       
    virtual MBOOL   init() = 0;
    /**
     * @brief Uninit the function 
     *
     * @details 
     *
     * @note 
     * 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable      
     *
     */       
    virtual MBOOL   uninit() = 0;
public:     ////    Callbacks.
    /**
     * @brief Set the callbacks 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] notify_cb: The notify callback function
     * @param[in] data_cb: The data callback function
     * @param[in] user: The caller 
     *      
     * @return 
     *
     */   
    virtual MVOID   setCallbacks(CamShotNotifyCallback_t notify_cb, CamShotDataCallback_t data_cb, MVOID* user) = 0;
    /**
     * @brief Check if the notify callback message enable 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The notify callback message type
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */   
    virtual MBOOL   isNotifyMsgEnabled(MINT32 const i4MsgTypes) const   = 0;
    /**
     * @brief Enable notify callback message 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The notify callback message type
     *      
     * @return 
     *
     */    
    virtual MVOID   enableNotifyMsg(MINT32 const i4MsgTypes)            = 0;
    /**
     * @brief Disable notify message 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The notify callbackmessage type    
     *      
     * @return 
     *
     */      
    virtual MVOID   disableNotifyMsg(MINT32 const i4MsgTypes)           = 0;
    /**
     * @brief Check if the data callback message enable 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The data callback message type
     *      
     * @return 
     * -   MTRUE indicates the data message type is enable; 
     * -   MFALSE indicates the data message type is disable 
     *
     */     
    virtual MBOOL   isDataMsgEnabled(MINT32 const i4MsgTypes) const     = 0;
    /**
     * @brief Enable data callback message 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The data callback message type
     *      
     * @return 
     *
     */     
    virtual MVOID   enableDataMsg(MINT32 const i4MsgTypes)              = 0;
    /**
     * @brief Disable data message 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] i4MsgTypes: The data callbackmessage type    
     *      
     * @return 
     *
     */     
    virtual MVOID   disableDataMsg(MINT32 const i4MsgTypes)             = 0;

public:     ////    Operations.    
    /**
     * @brief Start to capture w/ sensor input. 
     *
     * @details 
     *
     * @note Sync function, the caller will be blocked until finish.  
     * 
     * @param[in] rSensorParam: The sensor parameter.      
     *      
     * @return 
     * -    MTRUE indicates success; 
     * -    MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */ 
    virtual MBOOL   start(SensorParam const & rSensorParam, MUINT32 u4ShotCount) = 0;
    /**
     * @brief Start to capture w/ sensor input
     *
     * @details
     *
     * @note Async function, it will return immediatly. 
     * 
     * @param[in] rSensorParam: The sensor parameter  
     *
     * @return
     * -   MTRUE indicates success; 
     * -   MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().      
     *
     *          
     */ 
    virtual MBOOL   startAsync(SensorParam const & rSensorParam) = 0;    
    /**
     * @brief Start to capture one image w/ sensor input   
     *
     * @details
     *
     * @note Sync function, the caller will be blocked until finish.  
     * 
     * @param[in] rSensorParam: The sensor parameter      
     *
     * @return
     * - MTRUE indicates success 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     *
     */ 
    virtual MBOOL   startOne(SensorParam const & rSensorParam) = 0;     
    /**
     * @brief Start to capture one w/ image input from memory 
     *
     * @details
     *
     * @note Sync function, the caller will be blocked until finish.  
     * 
     * @param[in] rImgBufInfo: The input image buffer info it can be 
     *  - BayerImgBufInfo: The bayer raw image buffer info 
     *  - ImgBufInfo: The yuv image buffer info 
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     */ 
    virtual MBOOL   startOne(ImgBufInfo const & rImgBufInfo) = 0;
    /**
     * @brief Start to capture one w/ image input from memory
     *
     * @details
     *
     * @note Sync function, the caller will be blocked until finish.
     *
     * @param[in] rSensorParam: The sensor parameter
     *
     * @param[in] rImgBufInfo: The input image buffer info it can be
     *  - BayerImgBufInfo: The bayer raw image buffer info
     *  - ImgBufInfo: The yuv image buffer info
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL   startOne(SensorParam const & rSensorParam, ImgBufInfo const & rImgBufInfo) = 0;
    /**
     * @brief Stop the capture operation 
     *
     * @details
     *
     * @note Sync function, the caller will be blocked until finish.  
     * 
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     */     
    virtual MBOOL   stop()  = 0;
   
public:     ////    Settings.
    /**
     * @brief Set the shot parameter 
     *
     * @details
     *
     * @note 
     * 
     * @param[in] rParam: the shot paraemter 
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     */    
    virtual MBOOL   setShotParam(ShotParam const & rParam) = 0; 
    /**
     * @brief Set the jpeg parameter 
     *
     * @details
     *
     * @note 
     * 
     * @param[in] rParam: the jpeg paraemter         
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     */        
    virtual MBOOL   setJpegParam(JpegParam const & rParam) = 0; 
    // 
public:     ////    buffer setting. 
    /**
     * @brief Register the image buffer info
     *
     * @details this function is used for caller to register their image buffer \n 
     * the memory will be allocated by caller. 
     *
     * @note 
     * 
     * @param[in] eBufType: The registered image buffer type
     * @param[in] rImgBuf: The input image buffer 
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().     
     */ 
    virtual MBOOL   registerImgBufInfo(ECamShotImgBufType const eBufType, ImgBufInfo const &rImgBuf) = 0; 

public:     
    /**
     * @brief Send command for extension 
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
     */ 
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3) = 0; 
public:     //// info 

};
 

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_INC_CAMSHOT_ICAMSHOT_H_

