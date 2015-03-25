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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V1_CAMUTILS_CAMFORMAT_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V1_CAMUTILS_CAMFORMAT_H_
//


namespace android {
namespace MtkCamUtils {
namespace FmtUtils {


/*****************************************************************************
 * @brief Query the imageio format.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * imageio format.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding imageio format (i.e. EImageFormat)
 *
 ******************************************************************************/
uint_t
queryImageioFormat(
    char const* szPixelFormat
);


/*****************************************************************************
 * @brief Query the bits per pixel.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * bits per pixel.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding bits per pixel.
 *
 ******************************************************************************/
int
queryBitsPerPixel(
    char const* szPixelFormat
);


/*****************************************************************************
 * @brief Query the plane count.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * plane count.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding plane count.
 *
 ******************************************************************************/
size_t
queryPlaneCount(
    char const* szPixelFormat
);


/*****************************************************************************
 * @brief Query a specific plane's width stride.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image width and plane's 
 * index, return its corresponding plane's stride.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgWidth: image width, in pixel.
 *
 * @param[in] planeIndex: plane's index; 0, 1, and 2 refer to 1st-, 2nd-, and 
 * 3rd plane, respectively.
 *
 * @return its corresponding plane's stride, in pixel
 *
 ******************************************************************************/
size_t
queryImgWidthStride(
    char const* szPixelFormat, 
    size_t imgWidth, 
    size_t planeIndex
);


/*****************************************************************************
 * @brief Query a specific plane's height stride.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image height and plane's 
 * index, return its corresponding plane's stride.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgHeight: image height, in pixel.
 *
 * @param[in] planeIndex: plane's index; 0, 1, and 2 refer to 1st-, 2nd-, and 
 * 3rd plane, respectively.
 *
 * @return its corresponding plane's stride, in pixel
 *
 ******************************************************************************/
size_t
queryImgHeightStride(
    char const* szPixelFormat, 
    size_t imgHeight, 
    size_t planeIndex
);


/*****************************************************************************
 * @brief Query a striding buffer size, in bytes.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image width/height, 
 * return its corresponding striding buffer size, in bytes.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgWidth: image width, in pixel.
 *
 * @param[in] imgHeight: image height, in pixel.
 *
 * @return its corresponding striding buffer size, in bytes
 *
 ******************************************************************************/
size_t
queryImgBufferSize(
    char const* szPixelFormat, 
    size_t imgWidth, 
    size_t imgHeight
);


};  // namespace FmtUtils
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V1_CAMUTILS_CAMFORMAT_H_

