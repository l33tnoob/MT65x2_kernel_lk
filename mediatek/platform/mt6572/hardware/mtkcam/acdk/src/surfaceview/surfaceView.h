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

//! \file  surfaceView.h 

#ifndef _SURFACEVIEW_H_
#define _SURFACEVIEW_H_

extern "C" {
#include <linux/kd.h>
#include <linux/fb.h>
#include "mtkfb.h"
}

namespace NSACDK 
{
class AcdkSurfaceView; 

/**  
*@enum eBOOT_MODE
*/
typedef enum {	 
    NORMAL_MODE = 0, 
    META_MODE = 1, 
}eBOOT_MODE; 

#if 0   // not use in MT6589
typedef enum
{
    ACDK_SURFACE_FORMAT_RGB_565 = 0,
    ACDK_SURFACE_FORMAT_BGR_565,
    ACDK_SURFACE_FORMAT_RGB_888,
    ACDK_SURFACE_FORMAT_BGR_888,
    ACDK_SURFACE_FORMAT_ARGB_8888,
    ACDK_SURFACE_FORMAT_ABGR_8888,
    ACDK_SURFACE_FORMAT_BGRA_8888,
    ACDK_SURFACE_FORMAT_RGBA_8888,
    ACDK_SURFACE_FORMAT_YUV_420,
    ACDK_SURFACE_FORMAT_YUV_420_SP,
    ACDK_SURFACE_FORMAT_MTK_YUV,
    ACDK_SURFACE_FORMAT_YUY2,
    ACDK_SURFACE_FORMAT_UYVY,
    ACDK_SURFACE_FORMAT_Y800,
    ACDK_SURFACE_FORMAT_YUV_422_PL,      //422 Planar,i.e YV16 Planar
    ACDK_SURFACE_FORMAT_ANDROID_YV12,   //Androdi YV12.YVU stride all 16 pixel align
    ACDK_SURFACE_FORMAT_IMG_YV12,       //Imagination YV12.YVU stride all 32 pixel align
    ACDK_SURFACE_FORMAT_ERROR,
    ACDK_SURFACE_FORMAT_ALL = 0xFFFFFFFF
} ACDK_SURFACE_BITBLT_FORMAT_ENUM;

typedef enum
{
    ACDK_SURFACE_BITBLT_ROT_0 = 0,
    ACDK_SURFACE_BITBLT_ROT_90 = 0x1,
    ACDK_SURFACE_BITBLT_ROT_180 = 0x2,
    ACDK_SURFACE_BITBLT_ROT_270 = 0x3,// (90 + 180)
    ACDK_SURFACE_BITBLT_FLIP_H = 0x4,
    ACDK_SURFACE_BITBLT_ALL = 0xFFFFFFFF
} ACDK_SURFACE_BITBLT_ROT_ENUM;

typedef struct acdkSurfaceBltParam_s
{
    MUINT32 srcX;
    MUINT32 srcY;
    MUINT32 srcW;
    MUINT32 srcWStride;
    MUINT32 srcH;
    MUINT32 srcHStride;
    MUINT32 srcAddr;
    MUINT32 srcFormat;

    MUINT32 dstW;
    MUINT32 dstH;
    MUINT32 dstAddr;
    MUINT32 dstFormat;
    MUINT32 pitch;

    MUINT32 orientation;
    MUINT32 doImageProcess;
    MUINT32 favor_flags;    /*Image Transform favor flags,this is a low level flags. default is 0*/    

    MUINT32 u4SrcOffsetXFloat;//0x100 stands for 1, 0x40 stands for 0.25 , etc...
    MUINT32 u4SrcOffsetYFloat;//0x100 stands for 1, 0x40 stands for 0.25 , etc...

    /*Resizer coeff if can apply*/
    MUINT32 resz_up_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    MUINT32 resz_dn_scale_coeff;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    MUINT32 resz_ee_h_str;          //down scale only/0~15
    MUINT32 resz_ee_v_str;          //down scale only/0~15
} acdkSurfaceBltParam_t;
#endif

/**
*@class surfaceView
*@brief This class is the implementation of AcdkSurfaceView
*/
class surfaceView : public AcdkSurfaceView
{	
public:
    /**
       *@brief Constructor
       */
    surfaceView(); 

    /**
       *@brief Destroy surfaceView instance       
       */
    virtual void  destroyInstance();

    /**
       *@brief Initialize funtion
       *@note Must call this function right after createInstance and before other functions
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 init(); 

    /**
       *@brief Initialization funtion
       *@note Must call this function before destroyInstance
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 uninit();     

     /**
       *@brief Not use in MT6589
       */
    virtual MINT32 setOverlayBuf(
        MUINT32 const layerNo, 
        MUINT8 const *pSrcIn, 
        MUINT32 const srcFmt,  
        MUINT32 const srcW, 
        MUINT32 const srcH, 
        MUINT32 const orientation,
        MUINT32 const hFlip, 
        MUINT32 const vFlip) ;

    /**
       *@brief Get width, height and orientation of LCM
       *
       *@param[in,out] width : will be set to LCM width
       *@param[in,out] height : will be set to LCM height
       *@param[in,out] orientation : will be set to LCM orientation
       *
       *@return
       *-0 indicates success, otherwise indicates fail
       */    
    virtual MINT32 getSurfaceInfo(
            MUINT32 &width,
            MUINT32 &height, 
            MUINT32 &orientation);
         
    /**
       *@brief Set necessary info for frame buffer overlay 
       *
       *@param[in] layerNo : only use 0
       *@param[in] startx : frame start coordinate of x-direction
       *@param[in] starty : frame start coordinate of y-direction
       *@param[in] width : width of frame
       *@param[in] height : height of frame
       *@param[in] phyAddr : physical address of frame
       *@param[in] virtAddr : virtual address of frame
       *@param[in] orientation : orientation of frame
       *
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 setOverlayInfo(
            MUINT32 const layerNo, 
            MUINT32 const startx, 
            MUINT32 const starty, 
            MUINT32 const width, 
            MUINT32 const height, 
            MUINT32 const phyAddr, 
            MUINT32 const virtAddr,
            MUINT32 const orientation);     

    /**
       *@brief Not use in MT6589
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 registerBuffer(MUINT32 virtAddr, MUINT32 size);

    /**
       *@brief Not use in MT6589
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 unRegisterBuffer(MUINT32 virtAddr); 

    /**
       *@brief Reset to original layery status when not use overlay anymore
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 resetLayer(MUINT32 const layerNo); 

    /**
       *@brief Set to active layer       
       */
    virtual MVOID resetActiveFrameBuffer(MUINT32 a_u4No);

    /**
       *@brief Refresh to show preview
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 refresh();         

    /**
       *@brief Get how many layers
       *
       *@param[in,out] numLayer : will be set to total layers number
       *
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 getNumLayer(MUINT32 &numLayer);  

    /**
       *@brief Get current frame buffer index
       *@return
       *-frame buffer index
       */
    virtual MINT32 getFBNo() {return mFBNo;} 
   

protected:

    /**
       *@brief Destructor
       */
    virtual ~surfaceView();     


private: 

    /**
       *@brief Get boot mode from frame buffer
       *@return
       *-0 indicates success, otherwise indicates fail
       */
    MINT32 getBootMode(MUINT32 &mode); 

    /***************************************************************************/

    MUINT8 *m_pFrameBuf;
    
    MUINT32 mTotalLayerNum; 
    MUINT32 mBootMode; 
    MINT32  mFBfd; 
    MUINT32 mSurfaceWidth;
    MUINT32 mSurfaceHeight; 
    MUINT32 mSurfaceOriention;

    struct fb_overlay_layer mLayerInfo[5];
    struct fb_var_screeninfo mVinfo; 
    struct fb_fix_screeninfo mFinfo;
    
    MUINT32 mFBNo;
    MUINT32 mFBBufferSize;
    MUINT32 mFBBufferNo;
    MUINT32 mFBBufferIndex;

    //M4U Object
    MTKM4UDrv *m_pM4UDrv;    
};
};
#endif //end surfaceView.h 

