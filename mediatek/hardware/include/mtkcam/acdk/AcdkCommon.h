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

//! \file  AcdkCommon.h

#ifndef _ACDKCOMMON_H_
#define _ACDKCOMMON_H_

#include "mtkcam/acdk/AcdkTypes.h"

/**  
*@enum eACDK_COMMAND
*/
typedef enum
{
    ACDK_COMMAND_START = 0x80001000,
    ACDK_CMD_PREVIEW_START,
    ACDK_CMD_PREVIEW_STOP,
    ACDK_CMD_CAPTURE,
    ACDK_CMD_QV_IMAGE,
    ACDK_CMD_RESET_LAYER_BUFFER,
    ACDK_CMD_SET_SRC_DEV,
    ACDK_CMD_SET_OPERATION_MODE,
    ACDK_CMD_SET_SHUTTER_TIME,
    ACDK_CMD_GET_SHUTTER_TIME,
    ACDK_CMD_GET_CHECKSUM,
    ACDK_CMD_GET_AF_INFO,
    ACDK_COMMAND_END
}eACDK_COMMAND;

/**  
*@enum eACDK_CAP_MODE
*/
typedef enum  
{
    PREVIEW_MODE = 0,
    CAPTURE_MODE,
    VIDEO_MODE
}eACDK_CAMERA_MODE, eCAMERA_MODE;

/**  
*@enum eACDK_OPERA_MODE
*/
typedef enum  
{
    ACDK_OPT_NONE_MODE = 0,
    ACDK_OPT_META_MODE,
    ACDK_OPT_FACTORY_MODE
}eACDK_OPERA_MODE;

/**  
*@enum eIMAGE_TYPE
*/
typedef enum 
{
    RGB565_TYPE          = 0x00000001, 
    RGB888_TYPE          = 0x00000002, 
    PURE_RAW8_TYPE       = 0x00000004,
    PURE_RAW10_TYPE      = 0x00000008,
    PROCESSED_RAW8_TYPE  = 0x00000010,
    PROCESSED_RAW10_TYPE = 0x00000020,
    JPEG_TYPE            = 0x00000040,
    YUV_TYPE             = 0x00000080 
}eACDK_CAP_FORMAT;

/**  
*@struct ACDK_FEATURE_INFO_STRUCT
*/
typedef struct
{
    MUINT8 *puParaIn;
    MUINT8 *puParaOut;
    MUINT32 u4ParaInLen;
    MUINT32 u4ParaOutLen;
    MUINT32 *pu4RealParaOutLen;
}ACDK_FEATURE_INFO_STRUCT, *PACDK_FEATURE_INFO_STRUCT;

/**  
*@struct ACDK_PREVIEW_STRUCT
*/
typedef struct
{
    MUINT32 u4PrvW;
    MUINT32 u4PrvH;
    MUINT16 u16PreviewTestPatEn;
    Func_CB fpPrvCB;
    eACDK_OPERA_MODE eOperaMode;
}ACDK_PREVIEW_STRUCT, *PACDK_PREVIEW_STRUCT;

/**  
*@struct ACDK_SINGLE_CAP_STRUCT_S
*/
typedef struct ACDK_CAPTURE_STRUCT_S
{
    eACDK_CAMERA_MODE eCameraMode;
    eACDK_OPERA_MODE eOperaMode;
    eACDK_CAP_FORMAT eOutputFormat;
    MUINT16 u2JPEGEncWidth;
    MUINT16 u2JPEGEncHeight;
    Func_CB fpCapCB;
    MUINT32 u4CapCount;
    MINT32 i4IsSave; //0-don't save, 1-save
    MBOOL bUnPack;
public :
    ACDK_CAPTURE_STRUCT_S(eACDK_CAMERA_MODE a_camMode = CAPTURE_MODE,
                                         eACDK_OPERA_MODE a_operaMode = ACDK_OPT_META_MODE,
                                         eACDK_CAP_FORMAT a_outFormat = PURE_RAW10_TYPE,
                                         MUINT16 a_jpegEncWdth = 0,
                                         MUINT16 a_jpegEncHeight = 0,
                                         Func_CB a_funCB = NULL,
                                         MUINT32 a_capCnt = 1,
                                         MINT32 a_isSave = 0,
                                         MBOOL a_unPack = MFALSE)
                                                    : eCameraMode(a_camMode),
                                                      eOperaMode(a_operaMode),
                                                      eOutputFormat(a_outFormat),
                                                      u2JPEGEncWidth(a_jpegEncWdth),
                                                      u2JPEGEncHeight(a_jpegEncHeight),
                                                      fpCapCB(a_funCB),
                                                      u4CapCount(a_capCnt),
                                                      i4IsSave(a_isSave),
                                                      bUnPack(a_unPack)

    {
    }
}ACDK_CAPTURE_STRUCT, *PACDK_CAPTURE_STRUCT;

/**  
*@enum eRAW_ColorOrder
*@brief Color order of RAW image
*/
typedef enum
{
    RawPxlOrder_B = 0,  //! B Gb Gr R
    RawPxlOrder_Gb,     //! Gb B R Gr
    RawPxlOrderr_Gr,    //! Gr R B Gb
    RawPxlOrder_R       //! R Gr Gb B
}eRAW_ColorOrder;

/**  
*@enum eYUV_ColorOrder
*@brief YUV format
*/
typedef enum
{
    YUVFmt_Unknown = 0,
    YUVFmt_UYVY,  
    YUVFmt_VYUY,
    YUVFmt_YUY2,
    YUVFmt_YVYU
}eYUV_ColorOrder;

/**  
*@struct bufInfo
*@brief Infomation of image (except RAW)
*/
typedef struct
{
    MUINT8 *bufAddr; 
    MUINT32 imgWidth;
    MUINT32 imgHeight;
    MUINT32 imgSize;
    MINT32  imgFmt;
} bufInfo;

/**  
*@struct RAWBufInfo
*@brief Infomation of RAW image
*/
typedef struct  
{
    MUINT8 *bufAddr;
    MUINT32 bitDepth;
    MUINT32 imgWidth;
    MUINT32 imgHeight;
    MUINT32 imgSize;
    MBOOL   isPacked;
    eRAW_ColorOrder eColorOrder;
}RAWBufInfo; 

/**  
*@struct ImageBufInfo
*@brief Infomation of image buffer
*/
typedef struct 
{
    eACDK_CAP_FORMAT eImgType;   //! Image type 
    union 
    {
        bufInfo imgBufInfo; 
        RAWBufInfo RAWImgBufInfo; 
    }; 
}ImageBufInfo; 

/**  
*@struct ROIRect
*@brief CCT Application
*/
typedef struct 
{
    MUINT32 u4StartX;       //! Start X position for clip
    MUINT32 u4StartY;       //! Start Y position for clip
    MUINT32 u4ROIWidth;     //! Width of ROI 
    MUINT32 u4ROIHeight;    //! Height of ROI 
}ROIRect;

#endif //end AcdkCommon.h

