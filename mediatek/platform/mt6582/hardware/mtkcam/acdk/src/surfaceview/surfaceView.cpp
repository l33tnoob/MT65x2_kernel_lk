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

//! \file  surfaceView.cpp

#define LOG_TAG "surfaceView"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cutils/properties.h>

extern "C" {
#include <linux/kd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <linux/mtkfb.h>
}

//====== ACDK Related Header Files ======

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkErrCode.h"

//====== Other Header Files ======
//#include "camera_custom_if.h"
#include "m4u_lib.h"

//====== ACDK SurfaceView Related Header Files ======

#include "AcdkSurfaceView.h"
#include "surfaceView.h"

//====== Namespace ======

using namespace NSACDK;

//====== Defince Value ======

#define FB_DEVICE      "/dev/graphics/fb0"
#define MEDIA_PATH     "//data"
#define BOOT_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"

#define VIDOE_LAYER_NO    5

//#undef MTK_M4U_SUPPORT

//====== Glabal Variable ======

static FILE *g_fp = NULL; 
static MINT32 g_overlayFlag = 0;
static MINT32 g_overlayDebug = 0;

/*******************************************************************************
*
********************************************************************************/
void surfaceView::destroyInstance() 
{
    delete this; 
}

/*******************************************************************************
*
********************************************************************************/
surfaceView::surfaceView()
    :AcdkSurfaceView()
    ,mTotalLayerNum(VIDOE_LAYER_NO)
    ,mBootMode(NORMAL_MODE)
    ,mFBfd(-1) 
    ,mSurfaceWidth(0)
    ,mSurfaceHeight(0)
    ,mSurfaceOriention(0)
    ,mFBNo(0)
    ,mFBBufferSize(0)
    ,mFBBufferNo(0)
    ,mFBBufferIndex(0)
{
    ACDK_LOGD("+"); 

    memset (&mVinfo, 0, sizeof(fb_var_screeninfo));
    m_pFrameBuf = NULL;
    m_pM4UDrv   = NULL;

    //====== Get Property =====

    char value1[PROPERTY_VALUE_MAX];
    property_get("camera.acdkovl.disable", value1, "0");
    g_overlayFlag = atoi(value1); 

    char value2[PROPERTY_VALUE_MAX];
    property_get("camera.acdk.debug", value2, "0");
    g_overlayDebug = atoi(value2); 

    ACDK_LOGD("total layer cnt(%d)", mTotalLayerNum); 
}

/*******************************************************************************
*
********************************************************************************/
surfaceView::~surfaceView()
{
    m_pFrameBuf = NULL;
    
    if(mFBfd > 0)
    {
        uninit(); 
    }
}    
    
/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::getBootMode(MUINT32 &mode)
{
    ACDK_LOGD("+");
    
    //check if in Meta mode 
    MINT32 fd = open(BOOT_INFO_FILE, O_RDWR); 
    if(fd < 0) 
    {
        ACDK_LOGE("fail to open %s", BOOT_INFO_FILE);
        return ACDK_RETURN_API_FAIL; 
    }

    char bootMode[4]; 
    if(::read(fd, (MVOID*)bootMode, 4) == 0) 
    {
        ACDK_LOGE("fail to read %s", BOOT_INFO_FILE);
        close(fd); 
        return ACDK_RETURN_API_FAIL; 
    }
    
    if (bootMode[0] == 0x31) 
    {
        mode = META_MODE; 
    }
    else 
    {
        mode = NORMAL_MODE;
    }

    close(fd); 

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::init()
{
    ACDK_LOGD("+");
    
    mBootMode = NORMAL_MODE;

    //====== Open Frame Buffer Device ======
    
    mFBfd = ::open(FB_DEVICE, O_RDWR); 
    if (!mFBfd) 
    {
        ACDK_LOGE("Can not get frame buffer deivce %s ", FB_DEVICE); 
        return ACDK_RETURN_INVALID_DRIVER; 
    }

    //====== Get Fixed Screen Information ======
    
    if(::ioctl(mFBfd, FBIOGET_FSCREENINFO, &mFinfo) < 0)
    {
        ACDK_LOGE("Reading fixed information fail"); 
        return ACDK_RETURN_API_FAIL; 
    }

    //====== Get Variable Screen Information ======
    
    if(::ioctl(mFBfd, FBIOGET_VSCREENINFO, &mVinfo) < 0) 
    {
        ACDK_LOGE("Reading varaiable infromation fail");
        return ACDK_RETURN_API_FAIL;
    }    

    //====== Check the LCM Orientation ======

#ifdef USING_MTK_LDVT    
    mSurfaceOriention = 0;
#else
    mSurfaceOriention = atoi(MTK_LCM_PHYSICAL_ROTATION);
#endif    
   
    mSurfaceWidth  = mVinfo.xres; 
    mSurfaceHeight = mVinfo.yres;             

    //====== Dump The infromation ======
    
    ACDK_LOGD("smem_len     = %d", mFinfo.smem_len);
    ACDK_LOGD("Boot Mode    = %d", mBootMode); 
    ACDK_LOGD("mVinfo.xres  = %d", mVinfo.xres);        
    ACDK_LOGD("mVinfo.yres  = %d", mVinfo.yres);    
    ACDK_LOGD("mVinfo.xresv = %d", mVinfo.xres_virtual);  
    ACDK_LOGD("mVinfo.yresv = %d", mVinfo.yres_virtual);   
    ACDK_LOGD("mVinfo.xoff  = %d", mVinfo.xoffset);    
    ACDK_LOGD("mVinfo.yoff  = %d", mVinfo.yoffset);   
    ACDK_LOGD("mVinfo.bits_per_pixel = %d", mVinfo.bits_per_pixel);
    ACDK_LOGD("orientation = %d", mSurfaceOriention);

    if(g_overlayFlag == 0)
    {
        ACDK_LOGD("use overlay");

        //====== Init Layer Info ======
        
        for (MUINT32 i = 0; i < mTotalLayerNum; i++) 
        {
            memset(&mLayerInfo[i], 0, sizeof(struct fb_overlay_layer));             
        }
       
        //====== Force Disable Layer1 (Protection Step) ======
        
       /* mLayerInfo[1].layer_id = 1; //just for disable usage. mLayerInfo[0] disable layer0 and mLayerInfo[1] disable layer1

        if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo) < 0)
        {
            ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
            return ACDK_RETURN_API_FAIL;
        }
        mLayerInfo[1].layer_id = 0; //set to 0 is used to avoid affecting mLayerInfo[0], because we only use mLayerInfo[0]
        */

        mLayerInfo[0].layer_id = 0; //just for disable usage. mLayerInfo[0] disable layer0 and mLayerInfo[1] disable layer1
        
       if (ioctl(mFBfd, MTKFB_SET_OVERLAY_LAYER, &mLayerInfo[0]) < 0)
       {
           ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
           return ACDK_RETURN_API_FAIL;
       }
        
         mLayerInfo[0].layer_id = 1; //just for disable usage. mLayerInfo[0] disable layer0 and mLayerInfo[1] disable layer1
         
        if (ioctl(mFBfd, MTKFB_SET_OVERLAY_LAYER, &mLayerInfo[0]) < 0)
        {
            ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
            return ACDK_RETURN_API_FAIL;
        }

         mLayerInfo[0].layer_id = 3; //just for disable usage. mLayerInfo[0] disable layer0 and mLayerInfo[1] disable layer1
         
        if (ioctl(mFBfd, MTKFB_SET_OVERLAY_LAYER, &mLayerInfo[0]) < 0)
        {
            ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
            return ACDK_RETURN_API_FAIL;
        }




        //====== Create M4U Object ======

        m_pM4UDrv = new MTKM4UDrv();

        if(m_pM4UDrv == NULL)
        {
            ACDK_LOGE("m_pM4UDrv is NULL");
            return ACDK_RETURN_NULL_OBJ;
        }

#if 0   //cotta--temp for 82
        m_pM4UDrv->m4u_enable_m4u_func(DISP_OVL_0);        

        M4U_PORT_STRUCT port;
        port.Virtuality = 1;
        port.Security = 0;
        port.domain = 3;
        port.Distance = 1;
        port.Direction = 0; //M4U_DMA_READ_WRITE

        //port.ePortID = M4U_PORT_OVL_CH0;
        //ACDK_LOGD("m_pM4UDrv->m4u_config_port(M4U_PORT_OVL_CH0)");
        //if(ACDK_RETURN_NO_ERROR != m_pM4UDrv->m4u_config_port(&port))
        //{
        //    ACDK_LOGE("m_pM4UDrv->m4u_config_port(M4U_PORT_OVL_CH0) fail");
        //    return ACDK_RETURN_API_FAIL;
        //}
        
        port.ePortID = DISP_OVL_0;        
        if(0 != m_pM4UDrv->m4u_config_port(&port))
        {
            ACDK_LOGE("m_pM4UDrv->m4u_config_port(M4U_PORT_OVL_CH1) fail");
            return ACDK_RETURN_API_FAIL;
        }
#endif
    }
    else
    {
        ACDK_LOGD("use mmap");

        //====== Get Frame Buffer and Init======

        // per frame buffer size
        mFBBufferSize =  mVinfo.xres_virtual * mVinfo.yres * mVinfo.bits_per_pixel / 8;

        // number of frame in frame buffer
        //mFBBufferNo = mFinfo.smem_len / mFBBufferSize;
        mFBBufferNo = 2;  //zaikuo said : actually only use two 

        ACDK_LOGD("mFBBufferSize = %u",mFBBufferSize);
        ACDK_LOGD("mFBBufferNo   = %u",mFBBufferNo);
        ACDK_LOGD("mFBBufferIndex= %u",mFBBufferIndex);

        // memory map frame buffer
        m_pFrameBuf = (MUINT8 *)mmap(0, mFinfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, mFBfd, 0);

        if((MINT32)m_pFrameBuf == -1)
        {
            ACDK_LOGE("Failed to map framebuffer device to memory");
            return ACDK_RETURN_NULL_OBJ;     
        }
        ACDK_LOGD("The framebuffer device was mapped to memory successfully"); 
        
        // set the background color to black first    
        memset(m_pFrameBuf, 0, mFBBufferSize * 2);

        // set active buffer
        resetActiveFrameBuffer(mFBBufferIndex);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;     
}
    
/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::uninit()
{
    ACDK_LOGD("+"); 
    MINT32 err = ACDK_RETURN_NO_ERROR;
    
    if (mFBfd > 0 ) 
    {
        for (MUINT32 i = 0; i < mTotalLayerNum; i++) 
        {            
            mLayerInfo[i].layer_enable = 0;
        }
        mLayerInfo[0].layer_id= 0;
        mLayerInfo[1].layer_id= 1;
         
        if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo) < 0)  
        {
            ACDK_LOGE("ioctl(MTKFB_SET_VIDEO_LAYERS) failed");
            err = ACDK_RETURN_API_FAIL; 
        }
        
        if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) 
        {
            ACDK_LOGE("ioctl(MTKFB_TRIG_OVERLAY_OUT) failed");
            err = ACDK_RETURN_API_FAIL;     
        }        

        // in meta mode, we should restore the screen back 
        if (mBootMode == META_MODE) 
        {
            mVinfo.yoffset = 0;
            if (ioctl(mFBfd, MTKFB_META_RESTORE_SCREEN, &mVinfo) < 0) 
            { 
                ACDK_LOGE("Resore to Meta mode screen fail"); 
                err = ACDK_RETURN_API_FAIL;
            }
        }
        
        close(mFBfd); 
        mFBfd = -1; 
    }

    m_pFrameBuf = NULL;

    if(g_overlayFlag == 0)
    {
        if(m_pM4UDrv != NULL)
        {

#if 0   //cotta--temp for 82

            m_pM4UDrv->m4u_disable_m4u_func(DISP_OVL_0);
            
            M4U_PORT_STRUCT port;
            port.Virtuality = 0;  

            //port.ePortID = M4U_PORT_OVL_CH0;
            //if(ACDK_RETURN_NO_ERROR != m_pM4UDrv->m4u_config_port(&port))
            //{
            //    ACDK_LOGE("m_pM4UDrv->m4u_config_port(M4U_PORT_OVL_CH0) fail");
            //    return ACDK_RETURN_API_FAIL;
            //}
            
            port.ePortID = DISP_OVL_0;
            if(0 != m_pM4UDrv->m4u_config_port(&port))
            {
                ACDK_LOGE("m_pM4UDrv->m4u_config_port(M4U_PORT_OVL_CH1) fail");
                return ACDK_RETURN_API_FAIL;
            }
#endif
            delete m_pM4UDrv;
            m_pM4UDrv = NULL; 
        }
    }

    ACDK_LOGD("-");
    return err; 
}

    
/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::getSurfaceInfo(
            MUINT32 &width,
            MUINT32 &height, 
            MUINT32 &orientation)
{
    ACDK_LOGD("+");
    
    width  = mSurfaceWidth;
    height = mSurfaceHeight; 
    orientation = mSurfaceOriention; 

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR; 
}
         
/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::setOverlayInfo(
        MUINT32 const layerNo, 
        MUINT32 const startx, 
        MUINT32 const starty, 
        MUINT32 const width, 
        MUINT32 const height, 
        MUINT32 const phyAddr, 
        MUINT32 const virtAddr,
        MUINT32 const orientation)
{
    ACDK_LOGD("+");
    
    if (layerNo > mTotalLayerNum) 
    {
        ACDK_LOGE("error layerNo(%d)", layerNo); 
        return ACDK_RETURN_INVALID_PARA; 
    }

    ACDK_LOGD_DYN(g_overlayDebug,"layerNo  = %u", layerNo); 
    ACDK_LOGD_DYN(g_overlayDebug,"startx   = %u", startx); 
    ACDK_LOGD_DYN(g_overlayDebug,"starty   = %u", starty); 
    ACDK_LOGD_DYN(g_overlayDebug,"width    = %u", width); 
    ACDK_LOGD_DYN(g_overlayDebug,"height   = %u", height);
    ACDK_LOGD_DYN(g_overlayDebug,"phyAddr  = 0x%x", phyAddr); 
    ACDK_LOGD_DYN(g_overlayDebug,"virtAddr = 0x%x", virtAddr); 
    ACDK_LOGD_DYN(g_overlayDebug,"orientation = %u", orientation); 

    if(orientation == 90 || orientation == 270)
    {
        mLayerInfo[layerNo].src_pitch  = height;        
        mLayerInfo[layerNo].src_width  = height;
        mLayerInfo[layerNo].src_height = width;        
        mLayerInfo[layerNo].tgt_width  = height;
        mLayerInfo[layerNo].tgt_height = width;
        
    }
    else
    {
        mLayerInfo[layerNo].src_pitch  = width;       
        mLayerInfo[layerNo].src_width  = width;
        mLayerInfo[layerNo].src_height = height;        
        mLayerInfo[layerNo].tgt_width  = width;
        mLayerInfo[layerNo].tgt_height = height;
    }

    mLayerInfo[layerNo].src_offset_x      = 0;
    mLayerInfo[layerNo].src_offset_y      = 0;
    mLayerInfo[layerNo].tgt_offset_x      = startx;
    mLayerInfo[layerNo].tgt_offset_y      = starty;
    mLayerInfo[layerNo].layer_id          = 1;
    mLayerInfo[layerNo].layer_enable      = 1;
    //IF Camear pass2 VIDO output RGB565 
    //mLayerInfo[layerNo].src_fmt           = MTK_FB_FORMAT_RGB565;
    //IF Camear pass2 VIDO output YUV422
    mLayerInfo[layerNo].src_fmt           = MTK_FB_FORMAT_YUV422;
    mLayerInfo[layerNo].src_use_color_key = 0;
    mLayerInfo[layerNo].src_color_key     = 0;
    mLayerInfo[layerNo].src_phy_addr      = (MUINT32*)phyAddr;
    mLayerInfo[layerNo].src_base_addr     = (MUINT32*)virtAddr;

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR; 
}     
                                    

/*******************************************************************************
*
********************************************************************************/
MINT32 surfaceView::setOverlayBuf(
        MUINT32 const layerNo, 
        MUINT8  const *pSrcIn, 
        MUINT32 const srcFmt,  
        MUINT32 const srcW, 
        MUINT32 const srcH, 
        MUINT32 const orientation, 
        MUINT32 const hFlip, 
        MUINT32 const vFlip)
{
    //====== Local Variable ======

    MINT32 status = ACDK_RETURN_NO_ERROR;

#if 0

    ACDK_LOGD("[setOverlayBuf]E");

    ACDK_LOGD("[setOverlayBuf] layerNo = %u", layerNo); 
    ACDK_LOGD("[setOverlayBuf] pSrcIn  = 0x%x", (MUINT32)pSrcIn); 
    ACDK_LOGD("[setOverlayBuf] srcFmt  = %u", srcFmt); 
    ACDK_LOGD("[setOverlayBuf] srcW    = %u", srcW); 
    ACDK_LOGD("[setOverlayBuf] srcH    = %u", srcH);    
    ACDK_LOGD("[setOverlayBuf] hFlip   = %u", hFlip); 
    ACDK_LOGD("[setOverlayBuf] vFlip   = %u", vFlip);
    ACDK_LOGD("[setOverlayBuf] orientation = %u", orientation); 

    
    MINT32 bltOrientation;
    acdkSurfaceBltParam_t bltParam;

    //====== Value Setting ======
    
    // It's hw convert, the buffer address must be physical address

    memset(&bltParam, 0, sizeof(bltParam)); // initialize
    
    bltParam.srcAddr    = (MUINT32)(pSrcIn);
    bltParam.srcX       = 0;
    bltParam.srcY       = 0;
    bltParam.srcW       = srcW; // width;
    bltParam.srcWStride = srcW; //width;
    bltParam.srcH       = srcH; //height;
    bltParam.srcHStride = srcH; // height;
    bltParam.srcFormat  = (ACDK_SURFACE_BITBLT_FORMAT_ENUM) srcFmt; //MHAL_FORMAT_MTK_YUV;
    bltParam.dstAddr    =  (MUINT32)mLayerInfo[layerNo].src_phy_addr;
    
    bltOrientation = ACDK_SURFACE_BITBLT_ROT_0; 
    
    //Due to the android orientation should flip before rotation, 
    //Hence, if we rotate first, when rotate to 90 + h flip it will become 
    // rotate 270 + h flip. 
    switch (orientation) 
    {
        case 0:
            bltOrientation = hFlip ? ACDK_SURFACE_BITBLT_FLIP_H : ACDK_SURFACE_BITBLT_ROT_0; 
            break; 
        case 90:
            bltOrientation = hFlip ? ACDK_SURFACE_BITBLT_FLIP_H | ACDK_SURFACE_BITBLT_ROT_270 : ACDK_SURFACE_BITBLT_ROT_90; 
            break; 
        case 180:
            bltOrientation = hFlip ? ACDK_SURFACE_BITBLT_FLIP_H | ACDK_SURFACE_BITBLT_ROT_180 : ACDK_SURFACE_BITBLT_ROT_180; 
            break; 
        case 270:
            bltOrientation = hFlip ? ACDK_SURFACE_BITBLT_FLIP_H | ACDK_SURFACE_BITBLT_ROT_90 : ACDK_SURFACE_BITBLT_ROT_270; 
            break; 
    }
    
    if (orientation == 90 || orientation == 270)
    {
        bltParam.dstW  = srcH;
        bltParam.dstH  = srcW;
        bltParam.pitch = srcH;
    }
    else 
    {
        bltParam.dstW  = srcW;
        bltParam.dstH  = srcH;
        bltParam.pitch = srcW;
    }
    
    bltParam.dstFormat   = ACDK_SURFACE_FORMAT_RGB_565;      //display format     
    bltParam.orientation = bltOrientation; 

    //cotta--TODO : bitblt not sure
    
    //status = Mt6575_mHalBitblt(&bltParam);
    //if (ACDK_RETURN_NO_ERROR != status) 
    //{
    //    ACDK_LOGD("[setOverlayBuf] err: %d, can't do bitblt operation ", status);
    //    return status; 
    //}
    
    
    if (bltParam.srcFormat == ACDK_SURFACE_FORMAT_ABGR_8888)
    {
        g_fp = fopen ("//data//prv.raw", "wb"); 
        if (NULL == g_fp) 
        {
            ACDK_LOGE("[setOverlayBuf] Can not open file /data/prv.raw"); 
        }
        
        fwrite((void*)mLayerInfo[layerNo].src_base_addr , 1, bltParam.srcW * bltParam.srcH  * 2, g_fp); 
        fclose(g_fp); 
    }
#endif
    ACDK_LOGD("-");
    return status;
}     
 
/*******************************************************************************
*
********************************************************************************/   
MINT32 surfaceView::resetLayer(MUINT32 const layerNo)
{
    ACDK_LOGD("layerNo(%d)",layerNo);

    if(g_overlayFlag == 0)
    {
        ACDK_LOGD_DYN(g_overlayDebug,"use overlay");

        if (layerNo > mTotalLayerNum) 
        {
            ACDK_LOGE("error layerNo(%d)", layerNo); 
            return ACDK_RETURN_INVALID_PARA;         
        }

        mLayerInfo[0].layer_id = 1;
        mLayerInfo[0].layer_enable = 0;
        
        if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo[0]) < 0) 
        {
            ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
            return ACDK_RETURN_API_FAIL; 
        }    

        if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) 
        {
            ACDK_LOGE("MTKFB_TRIG_OVERLAY_OUT failed");
            return ACDK_RETURN_API_FAIL; 
        }
    }
    else
    {
        ACDK_LOGD_DYN(g_overlayDebug,"use mmap");
        memset(m_pFrameBuf, 0, mFBBufferSize * 2);
    }

    ACDK_LOGD_DYN(g_overlayDebug,"-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID surfaceView::resetActiveFrameBuffer(MUINT32 a_u4No)
{
    ACDK_LOGD_DYN(g_overlayDebug,"+");
    
    if(a_u4No > mFBBufferNo) 
    {
        ACDK_LOGE("Wrong Index");
        return;    
    }

    if(g_overlayFlag == 0)
    {
        ACDK_LOGD_DYN(g_overlayDebug,"use overlay");

        if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo) < 0) 
        {
            ACDK_LOGE("ioctl(MTKFB_SET_VIDEO_LAYERS) failed");
        }

        if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0)
        {
            ACDK_LOGE("ioctl(MTKFB_TRIG_OVERLAY_OUT) failed");    
        }
    }
    else
    {
        ACDK_LOGD_DYN(g_overlayDebug,"use mmap");

        mVinfo.yres_virtual = mVinfo.yres * 2;       
        mVinfo.yoffset = a_u4No * mVinfo.yres;  
        mVinfo.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;

        ACDK_LOGD_DYN(g_overlayDebug,"mVinfo.yres = %u",mVinfo.yres);
        ACDK_LOGD_DYN(g_overlayDebug,"mVinfo.yres_virtual = %u",mVinfo.yres_virtual);
        ACDK_LOGD_DYN(g_overlayDebug,"mVinfo.yoffset = %u",mVinfo.yoffset);

        if (ioctl(mFBfd, FBIOPUT_VSCREENINFO, &mVinfo) < 0)
        {
            ACDK_LOGE("ioctl(FBIOPUT_VSCREENINFO) failed");    
        }
    }

    ACDK_LOGD_DYN(g_overlayDebug,"-");
}

    
/*******************************************************************************
*
********************************************************************************/   
MINT32 surfaceView::refresh()
{
    ACDK_LOGD_DYN(g_overlayDebug,"+");    
 
    if(g_overlayFlag == 0)
    {
        ACDK_LOGD_DYN(g_overlayDebug,"use overlay");

        ACDK_LOGD_DYN(g_overlayDebug,"src VA(0x%x), PA(0x%x)",(MUINT32)mLayerInfo[0].src_base_addr,(MUINT32)mLayerInfo[0].src_phy_addr );

        if (ioctl(mFBfd, MTKFB_SET_OVERLAY_LAYER, &mLayerInfo[0]) < 0)
        {
            ACDK_LOGE("MTKFB_SET_VIDEO_LAYERS failed");
            return ACDK_RETURN_API_FAIL;
        }

        if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) 
        {
            ACDK_LOGE("MTKFB_TRIG_OVERLAY_OUT) failed");    
            return ACDK_RETURN_API_FAIL;
        } 
    } 
    else
    {
        MINT32 bytePrePixel = mVinfo.bits_per_pixel / 8;

        ACDK_LOGD_DYN(g_overlayDebug,"use mmap");
        ACDK_LOGD_DYN(g_overlayDebug,"src VA(0x%x), size(%u)",(MUINT32)mLayerInfo[0].src_base_addr,mLayerInfo[0].src_width * mLayerInfo[0].src_height * 2);
        ACDK_LOGD_DYN(g_overlayDebug,"Y offset(%d)",mVinfo.xres_virtual * mLayerInfo[0].tgt_offset_y * bytePrePixel);
        ACDK_LOGD_DYN(g_overlayDebug,"mFBNo(%u),mFBBufferIndex(%u)",mFBNo,mFBBufferIndex);
        
        //memcpy((m_pFrameBuf + (mFBBufferIndex * mFBBufferSize)) + (mVinfo.xres * mLayerInfo[0].tgt_offset_y * 2), mLayerInfo[0].src_base_addr, mLayerInfo[0].src_width * mLayerInfo[0].src_height * 2);

        memset(m_pFrameBuf, 0, mFBBufferSize * 2);
        for(MUINT32 i = 0; i < mLayerInfo[0].src_height; ++i)
        {
            memcpy((m_pFrameBuf + (mFBBufferIndex * mFBBufferSize))+ mLayerInfo[0].tgt_offset_x + (mVinfo.xres_virtual * mLayerInfo[0].tgt_offset_y * bytePrePixel) + (i * mVinfo.xres_virtual * bytePrePixel), 
                    mLayerInfo[0].src_base_addr + (i * mLayerInfo[0].src_width * 2), mLayerInfo[0].src_width * 2);
        }
        
        resetActiveFrameBuffer(mFBBufferIndex);   
            
        mFBBufferIndex = (mFBBufferIndex + 1) % mFBBufferNo;
    }
    
    mFBNo = (mFBNo + 1) % OVERLAY_BUFFER_CNT;

    ACDK_LOGD_DYN(g_overlayDebug,"-");
    return ACDK_RETURN_NO_ERROR; 
}
        
/*******************************************************************************
* 
*******************************************************************************/
MINT32 surfaceView::getNumLayer(MUINT32 &numLayer)
{
    numLayer = mTotalLayerNum; 
    return ACDK_RETURN_NO_ERROR; 
}

/*******************************************************************************
* 
*******************************************************************************/
MINT32 surfaceView::registerBuffer(MUINT32 virtAddr, MUINT32 size)
{
    int ret = 0;
#if 0
    ACDK_LOGD("+");
    
    if (mFBfd < 0) 
    {
        ACDK_LOGE("error fd(%d)", mFBfd); 
        return ACDK_RETURN_INVALID_DRIVER; 
    } 

    //====== Local Variable ======

    int ret = 0;
    struct fb_overlay_buffer_info b;

    //====== Variable Setting ======
    
    b.src_vir_addr = (MUINT32)virtAddr;
    b.size         = (MUINT32)size;

    //====== IOCTL ======
    
    ret = ::ioctl(mFBfd, MTKFB_REGISTER_OVERLAYBUFFER, &b);
    if (ret < 0) 
    {
        ACDK_LOGE("MTKFB_REGISTER_OVERLAYBUFFER failed, err(%s), %d", strerror(ret), ret);
    }

    ACDK_LOGD("-");
#endif
    return ret; 
}

/*******************************************************************************
* 
*******************************************************************************/
MINT32 surfaceView::unRegisterBuffer(MUINT32 virtAddr) 
{
    int ret = 0;
#if 0
    ACDK_LOGD("+");
    
    if (mFBfd < 0) 
    {
        ACDK_LOGE("error fd(%d)", mFBfd); 
        return ACDK_RETURN_INVALID_DRIVER; 
    } 
 
    MUINT32 va = virtAddr; 
    int ret = ::ioctl(mFBfd, MTKFB_UNREGISTER_OVERLAYBUFFER, &va);
    if (ret < 0) 
    {
        ACDK_LOGE("MTKFB_UNREGISTER_OVERLAYBUFFER failed, err(%s), %d", strerror(ret), ret);
    }

    ACDK_LOGD("-");
#endif
    return ret;    
}


