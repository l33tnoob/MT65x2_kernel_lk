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

//! \file  AcdkUtility.h

#ifndef _ACDKUTILITY_H_
#define _ACDKUTILITY_H_

#include <mtkcam/drv/imem_drv.h>

namespace NSACDK 
{    
    /**
         *@class AcdkUtility
         *@brief This class is used for getting infomation of image/sensor format
       */
    class AcdkUtility
    {
    public: 
            /**                       
                       *@brief AcdkUtility constructor                       
                     */
            AcdkUtility() {};

            /**                       
                       *@brief AcdkUtility destructor                       
                     */
            virtual ~AcdkUtility() {};

            /**                       
                       *@brief Create AcdkUtility object                       
                     */
            static AcdkUtility *createInstance();
            
            /**                       
                       *@brief Destroy AcdkUtility object                       
                     */
            MVOID destroyInstance();

            /**                       
                       *@brief Calculate preview size 
                       *@note Will align to 2x
                     */
            MVOID queryPrvSize(MUINT32 &oriW, MUINT32 &oriH);
            
            /**                       
                       *@brief Calculate capture size  
                       *@note Will align to 16x
                     */
            MVOID queryCapSize(MUINT32 &oriW, MUINT32 &oriH);

            /**                       
                       *@brief Get format infomation of RAW image
                       *@details It will calculate the RAW image stride and byte_per_pixle depends on RAW type
                       *@note the return value pixel_byte, need to be devided by 4.0 as float before using
                       *
                       *@param[in] imgFmt : RAW type : eImgFmt_BAYER8,eImgFmt_BAYER10,eImgFmt_BAYER12
                       *@param[in] u4ImgWidth : image width : unite is pixel
                       *@param[in,out] u4Stride : will be set to actual image stride
                       *@param[in,out] pixel_byte : will be set to actual byte_per_pixle
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            MINT32 queryRAWImgFormatInfo(MUINT32 const imgFmt, MUINT32 u4ImgWidth, MUINT32 &u4Stride, MUINT32 &pixel_byte);

            /**                       
                       *@brief Calculate image size
                       *@details It will calculate the image size depends image type
                       *@note RAW image is not support
                       *
                       *@param[in] imgFormat : image format
                       *@param[in] imgW : image width
                       *@param[in] imgH : image height
                       *@param[in,out] imgSize : will be set to size which is calcualted by function
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            MINT32 queryImageSize(MUINT32 imgFormat, MUINT32 imgW, MUINT32 imgH, MUINT32 &imgSize);

            /**                       
                       *@brief Calculate image stride
                       *@details It will calculate the image stride depends image type
                       *@note RAW image is not support
                       *
                       *@param[in] imgFormat : image format
                       *@param[in] imgW : image width
                       *@param[in] planeIndex : the 1 or 2 or 3 plane of image
                       *@param[in,out] imgStride : will be set to stride which is calcualted by function
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            MINT32 queryImageStride(MUINT32 imgFormat, MUINT32 imgW, MUINT32 planeIndex, MUINT32 *imgStride);

            /**                       
                       *@brief Image Process
                       *@details Rotate, image format transform, resize, etc
                       *
                       *@param[in] imgFormat : image format
                       *@param[in] srcImgW : width of input image
                       *@param[in] srcImgH : height of input image
                       *@param[in] orientaion : 0, 90, 180, 270
                       *@param[in] flip : 0-no flip, 1-flip
                       *@param[in] srcImem : IMEM of input image
                       *@param[in] dstImem : IMEM of output image
                       *@param[in] dstImgW : width of output image. not necessary, default is 0
                       *@param[in] dstImgH : height of output image. not necessary, default is 0
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            MINT32 imageProcess(MUINT32 imgOutFormat, 
                                      MUINT32 srcImgW, 
                                      MUINT32 srcImgH, 
                                      MUINT32 orientaion,
                                      MUINT32 flip,
                                      IMEM_BUF_INFO srcImem, 
                                      IMEM_BUF_INFO dstImem,
                                      MUINT32 dstImgW = 0,
                                      MUINT32 dstImgH = 0);
            
            /**                       
                       *@brief Unpack RAW image
                       *@details for MT6589 RAW type
                       *
                       *@param[in] srcImem : IMEM of input image
                       *@param[in] dstImem : IMEM of output image
                       *@param[in] a_imgW : width of input image
                       *@param[in] a_imgH  : height of input image
                       *@param[in] a_bitDepth  : bit depth of input image
                       *@param[in] a_Stride  : stride of input image
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            MINT32 rawImgUnpack(IMEM_BUF_INFO srcImem,
                                      IMEM_BUF_INFO dstImem,
                                      MUINT32 a_imgW,
                                      MUINT32 a_imgH,
                                      MUINT32 a_bitDepth,
                                      MUINT32 a_Stride);
    };
};
#endif //end AcdkUtility.h 

