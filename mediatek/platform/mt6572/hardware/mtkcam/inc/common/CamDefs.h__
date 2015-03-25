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

#ifndef _MTK_CAMERA_INC_COMMON_CAMDEFS_H_
#define _MTK_CAMERA_INC_COMMON_CAMDEFS_H_
/******************************************************************************
 *  Camera Definitions.
 ******************************************************************************/

 /**  
 * @enum EAppMode
 * @brief App Mode Enumeration.
 *
 */
enum EAppMode
{
    eAppMode_DefaultMode = 0,                     /*!< Default Mode */    	  
    eAppMode_EngMode,                             /*!< Engineer Mode */    	 
    eAppMode_AtvMode,                             /*!< ATV Mode */    	 
    eAppMode_S3DMode,                             /*!< S3D Mode */    	 
    eAppMode_VtMode,                              /*!< VT Mode */    	 
    eAppMode_PhotoMode,                           /*!< Photo Mode */    
    eAppMode_VideoMode,                           /*!< Video Mode */    
    eAppMode_ZsdMode,                             /*!< ZSD Mode */    	
    eAppMode_FactoryMode,			  /*!< Factory Mode */		
};

/******************************************************************************
 *  
 ******************************************************************************/
 
 /**  
 * @enum EShotMode
 * @brief Shot Mode Enumeration.
 *
 */
enum EShotMode
{
    eShotMode_NormalShot,                           /*!< Normal Shot */    	
    eShotMode_ContinuousShot,                       /*!< Continuous Shot Ncc*/ 
    eShotMode_ContinuousShotCc,                     /*!< Continuous Shot Cc*/ 
    eShotMode_BestShot,                             /*!< Best Select Shot */    
    eShotMode_EvShot,                               /*!< Ev-bracketshot Shot */ 
    eShotMode_SmileShot,                            /*!< Smile-detection Shot */ 
    eShotMode_HdrShot,                              /*!< High-dynamic-range Shot */
    eShotMode_AsdShot,                              /*!< Auto-scene-detection Shot */
    eShotMode_ZsdShot,                              /*!< Zero-shutter-delay Shot */    
    eShotMode_FaceBeautyShot,                       /*!<  Face-beautifier Shot */   
    eShotMode_Mav,                                  /*!< Multi-angle view Shot */    	
    eShotMode_Autorama,                             /*!< Auto-panorama Shot */  
    eShotMode_MultiMotionShot,                      /*!< Multi-motion Shot */    
    eShotMode_Panorama3D,                           /*!< Panorama 3D Shot */   
    eShotMode_Single3D,                             /*!< Single Camera 3D Shot */ 
    eShotMode_EngShot,								/*!< Engineer Mode Shot */
};

  /**  
 * @enum EImageFormat
 * @brief Image format Enumeration.
 *
 */
enum EImageFormat
{
    eImgFmt_UNKNOWN         = 0x000000,   /*!< unknow */    	
    eImgFmt_BAYER8          = 0x000001,   /*!< Bayer format, 8-bit */
    eImgFmt_BAYER10         = 0x000002,   /*!< Bayer format, 10-bit */ 
    eImgFmt_BAYER12         = 0x000004,   /*!< Bayer format, 12-bit */ 
    eImgFmt_YV12            = 0x000008,   /*!< 420 format, 3 plane (Y),(V),(U) */ 
    eImgFmt_NV21            = 0x000010,   /*!< 420 format, 2 plane (Y),(VU) */    
    eImgFmt_NV21_BLK        = 0x000020,   /*!< 420 format block mode, 2 plane (Y),(VU) */ 
    eImgFmt_NV12            = 0x000040,   /*!< 420 format, 2 plane (Y),(UV) */    	
    eImgFmt_NV12_BLK        = 0x000080,   /*!< 420 format block mode, 2 plane (Y),(UV) */ 
    eImgFmt_YUY2            = 0x000100,   /*!< 422 format, 1 plane (YUYV) */    
    eImgFmt_UYVY            = 0x000200,   /*!< 422 format, 1 plane (UYVY) */    
    eImgFmt_RGB565          = 0x000400,   /*!< RGB 565 (16-bit), 1 plane */    
    eImgFmt_RGB888          = 0x000800,   /*!< RGB 888 (24-bit), 1 plane (RGB) */
    eImgFmt_ARGB888         = 0x001000,   /*!< ARGB (32-bit), 1 plane */    	
    eImgFmt_JPEG            = 0x002000,   /*!< JPEG format */    	
    eImgFmt_YV16            = 0x004000,   /*!< 422 format, 3 plane (Y),(U),(V)*/    	
    eImgFmt_NV16            = 0x008000,   /*!< 422 format, 2 plane (Y),(UV)*/    	
    eImgFmt_NV61            = 0x010000,   /*!< 422 format, 2 plane (Y),(VU)*/    	
    eImgFmt_I420            = 0x020000,   /*!<420 format, 3 plane (Y),(U),(V) */    	
    eImgFmt_Y800            = 0x040000,   /*!< Y plane only  */   
    eImgFmt_YVYU            = 0x080000,   /*!< 422 format, 1 plane (YVYU)*/   
    eImgFmt_VYUY            = 0x100000,   /*!< 422 format, 1 plane (VYUY)   */
};


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_CAMERA_INC_COMMON_CAMDEFS_H_

