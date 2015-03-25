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
#ifndef _MTK_CAMERA_INC_COMMON_HW_HWSTDDEF_H_
#define _MTK_CAMERA_INC_COMMON_HW_HWSTDDEF_H_

#include <mtkcam/common.h>
using namespace NSCam;

/*******************************************************************************
*
*******************************************************************************/
namespace NSCamHW {
////////////////////////////////////////////////////////////////////////////////

/**  
 * @struct ImgInfo
 * @brief This structure is Image Info. 
 *
 */
struct ImgInfo
{
public:     //// fields.
    /**
      * @var eImgFmt 
      * Image Pixel Format
      */
    EImageFormat   eImgFmt;      
    /**
      * @var u4ImgWidth 
      * Image Width
      */    
    MUINT32        u4ImgWidth;     //  
    /**
      * @var u4ImgHeight
      * Image Height
      */    
    MUINT32        u4ImgHeight;    //  Image Height
    //
public:     //// constructors.
    ImgInfo(
        EImageFormat const _eImgFmt = eImgFmt_UNKNOWN, 
        MUINT32      const _u4ImgWidth = 0, 
        MUINT32      const _u4ImgHeight = 0
    )
        : eImgFmt(_eImgFmt)
        , u4ImgWidth(_u4ImgWidth)
        , u4ImgHeight(_u4ImgHeight)
    {}
};

/**  
 * @struct Rect 
 * @brief This structure is for rectangle info 
 *
 */
struct Rect{
public:
    /**
      * @var x 
      * x offset 
      */	
    MUINT32 x;
    /**
      * @var y 
      * y offset 
      */    
    MUINT32 y;
    /**
      * @var w
      * width 
      */    
    MUINT32 w;
    /**
      * @var h
      * height
      */    
    MUINT32 h;
     
public:     //// constructors. 
    // 
    Rect(MUINT32 const _x = 0 , 
         MUINT32 const _y = 0 , 
         MUINT32 const _w = 0, 
         MUINT32 const _h = 0)
        : x(_x)
        , y(_y)
        , w(_w)
        , h(_h)
    {
    }

};

/**  
 * @struct BufInfo 
 * @brief This structure is for buffer Info.
 *
 */
struct BufInfo
{
public:     //// fields.
    /**
      * @var u4BufSize
      * Per buffer size
      */
    MUINT32        u4BufSize;  
    /**
      * @var u4BufVA 
      * Virtual Address
      */    
    MUINT32        u4BufVA;    
    /**
      * @var u4BufPA 
      * Physical Address
      */    
    MUINT32        u4BufPA; 
    /**
      * @var i4MemID
      * Memory device ID 
      */    
    MINT32         i4MemID;       //  
    //
    MINT32         i4BufSecu;
    MINT32         i4BufCohe;
    //
public:     //// constructors.
    BufInfo(
        MUINT32 const _u4BufSize = 0, 
        MUINT32 const _u4BufVA = 0, 
        MUINT32 const _u4BufPA = 0,
        MINT32  const _i4MemID = -1,
        MINT32  const _i4BufSecu = 0,
        MINT32  const _i4BufCohe = 0
        //EMemoryType const _eMemType = eMemoryType_STD_M4U
    )
        : u4BufSize(_u4BufSize)
        , u4BufVA(_u4BufVA)
        , u4BufPA(_u4BufPA)
        , i4MemID(_i4MemID)
        , i4BufSecu(_i4BufSecu)
        , i4BufCohe(_i4BufCohe)
        //, eMemType(_eMemType)
    {
    }
    //
};

/**  
 * @struct ImgBufInfo 
 * @brief This structure is for image buffer info.
 *
 */
struct ImgBufInfo: public ImgInfo, public BufInfo
{
public:    //// constructors. 
    /**
      * @var u4Stride
      * The stride of the width for 3 planes 
      */   
    MUINT32 u4Stride[3]; 
    //
    ImgBufInfo( 
    )
        : ImgInfo()
        , BufInfo()
    {    
        u4Stride[0] = u4Stride[1] = u4Stride[2] = 0; 
    }
    //
    ImgBufInfo( 
        ImgInfo const &_ImgInfo, 
        BufInfo const &_BufInfo, 
        MUINT32 const _u4Stride[3]
    )
        : ImgInfo(_ImgInfo)
        , BufInfo(_BufInfo)
    {    
        u4Stride[0] = _u4Stride[0];  
        u4Stride[1] = _u4Stride[1];         
        u4Stride[2] = _u4Stride[2];  
    }
    // copy ctor
    ImgBufInfo(
        const ImgBufInfo &_ImgBufInfo
    )
        : ImgInfo( _ImgBufInfo )
        , BufInfo( _ImgBufInfo )
    {
        u4Stride[0] = _ImgBufInfo.u4Stride[0];  
        u4Stride[1] = _ImgBufInfo.u4Stride[1];         
        u4Stride[2] = _ImgBufInfo.u4Stride[2];  
    }
    // assignment operator
    ImgBufInfo& operator=( const ImgBufInfo &_ImgBufInfo )
    {
        ImgInfo::operator=(static_cast<const ImgInfo&>(_ImgBufInfo));
        BufInfo::operator=(static_cast<const BufInfo&>(_ImgBufInfo));
        u4Stride[0] = _ImgBufInfo.u4Stride[0];  
        u4Stride[1] = _ImgBufInfo.u4Stride[1];         
        u4Stride[2] = _ImgBufInfo.u4Stride[2];  
        return *this;
    }
}; 


////////////////////////////////////////////////////////////////////////////////

};  //namespace NSCamHW
#endif // _MTK_HAL_INC_COMMON_HW_HWSTDDEF_H_

