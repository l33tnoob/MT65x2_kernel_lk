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
#ifndef _MTK_CAMERA_INC_CAMSHOT_ISIMAGER_H_
#define _MTK_CAMERA_INC_CAMSHOT_ISIMAGER_H_

//
#include <mtkcam/common/hw/hwstddef.h>
#include <mtkcam/camshot/_callbacks.h>


using namespace NSCamHW; 
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

/**  
 * @enum ESImagerNotifyMsg
 * @brief The SImager notify message 
 *
 */
enum ESImagerNotifyMsg {
    ESImager_NOTIFY_MSG_ERROR             = 0x0001,           // Notify for error 
    ESImager_NOTIFY_MSG_DONE              = 0x0002,           // Notify for start of frame 
    ESImager_NOTIFY_MSG_TIMEOUT           = 0x0004,           // Notify for end of frame   
};


/**  
 * @class ISImager
 * @brief Interface of SImager
 */
class ISImager
{
public:     ////    Attributes.
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
    virtual         ~ISImager() {}

public:     ////    Instantiation.
    /**
     * @brief create the ISImager instance 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] rImgBufInfo: the input image buffer info  
     *      
     * @return 
     * -   The instance of the ISImager
     *
     */   
    static ISImager* createInstance(ImgBufInfo const & rImgBufInfo);
    /**
     * @brief destroy the ISImager instance 
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
    

public:     ////    Operations.    
    /**
     * @brief Start to execute the image process w/ timeout 
     *
     * @details 
     *
     * @note Sync function, it will block until done
     * 
     * @param[in] u4TimeoutMs: the time out in ms. 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */      
    virtual MBOOL    execute(MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0; 
    /**
     * @brief Start to execute the image process 
     *
     * @details 
     *
     * @note ASync function, it will return immediate 
     * 
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */       
    virtual MBOOL    executeAsync() = 0; 
    /**
     * @brief Cancel the operation, use for executeAsync().
     *
     * @details 
     *
     * @note 
     * 
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */     
    virtual MBOOL    cancel() = 0; 

public:      //// attributes 
    /**
     * @brief Get the final jpeg bitstream buffer size
     * 
     *
     * @details if the ouptut buffer format is eImgFmt_JPEG 
     *
     * @note 
     *       
     * @return 
     * -   The final jpeg bitstream buffer size 
     * -   MFALSE indicates the notify message type is disable 
     *
     */ 
    virtual MUINT32    getJpegSize()  = 0; 

public:     ////    Settings 
    /**
     * @brief Set the target buffer info 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] rBufInfo: The buffer info of target 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */   
    virtual MBOOL   setTargetBufInfo(BufInfo const &rBufInfo) = 0; 
    /**
     * @brief Set the format of the target image 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] eFormat: The target image format 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setFormat(EImageFormat const eFormat) = 0; 
    /**
     * @brief Set the flip operation of the target image 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] u4Flip: The flip of target image 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setFlip(MUINT32 const u4Flip) = 0; 
    /**
     * @brief Set the roi of the source image 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] rROI: The region of interest of the source image  
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setROI(Rect const rROI) = 0; 
    /**
     * @brief Set the target size of the target image 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] u4Width: The width of the target image 
     * @param[in] u4Height: The height of the target image 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setResize(MUINT32 const u4Width, MUINT32 const u4Height) = 0; 
    /**
     * @brief Set the rotation operation of the source image 
     *
     * @details 
     *
     * @note 
     * 
     * @param[in] u4Rotation: The rotatoin angle 0, 90, 180, 270
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setRotation(MUINT32 const u4Rotation) = 0;  
    /**
     * @brief Set the encode param of the jpeg 
     *
     * @details 
     * This function only need to call if the target image is eImgFmt_JPEG
     * @note 
     * 
     * @param[in] u4IsSOI: Is embedded the start of image header into Jpeg 
     * @param[in] u4Quality: The jpeg encode quality factory 0 ~ 100 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setEncodeParam(MUINT32 const &u4IsSOI, MUINT32 const &u4Quality) = 0; 
    /**
     * @brief Set the stride align information 
     *
     * @details 
     * This function only need to call if the target image width need to 
     * have stride align, default is 1 byte align, 
     * @note 
     * 
     * @param[in] u4StrideAlign: The 1st, 2nd, 3rd plane stride  
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MBOOL   setStrideAlign(MUINT32 const u4StrideAlign[3]) = 0; 

public:     ////    Callbacks     
    /**
     * @brief regist the callback function for caller  
     *
     * @details 
     * This function only need to use if the call executeAsync()
     * @note 
     * 
     * @param[in] notify_cb: The notify callback function   
     * @param[in] user: The caller 
     *      
     * @return 
     * -   MTRUE indicates the notify message type is enable; 
     * -   MFALSE indicates the notify message type is disable 
     *
     */
    virtual MVOID   setCallback(SImagerNotifyCallback_t notify_cb, MVOID* user) = 0; 

 
public:     //// info 

};
 

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_INC_CAMSHOT_ISIMAGER_H_

