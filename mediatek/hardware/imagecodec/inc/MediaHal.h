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

/**
 * @file
 *   MediaHal.h
 *
 * @par Project:
 *   Yusu
 *
 * @par Description::
 *   MHAL definitions and function prototypes, this layer manager all multimedia module
 *   MHAL stands for Mediatek Hardware Abstract Layer
 *
 * @par Author:
 *   Wilson Lu (mtk01780)
 *   Tzu-Meng Chung (mtk02529)
 *
 * @par $Revision: #12 $
 * @par $Modtime:$
 * @par $Log:$
 *
 */

#ifndef MEDIA_HAL_H
#define MEDIA_HAL_H

#include "MediaTypes.h"

/*=============================================================================
 *                              Type definition
 *===========================================================================*/


/*******************************************************************************
* IO Control Mask
********************************************************************************/
#define MHAL_IOCTL_CAMERA_GROUP_MASK                0x1000  ///< Camera group mask
#define MHAL_IOCTL_JPEG_GROUP_MASK                  0x2000  ///< JPEG group mask
#define MHAL_IOCTL_VIDEO_GROUP_MASK                 0x4000  ///< VIDEO group mask

/*******************************************************************************
* IO Control Code
********************************************************************************/
#define MHAL_IOCTL_LOCK_RESOURCE                    0x0001  ///< To query and lock resource (IDP and DCT)
#define MHAL_IOCTL_UNLOCK_RESOURCE                  0x0002  ///< To unlock resource (IDP and DCT)
//
#define MHAL_IOCTL_JPEG_ENC                         0x2001  ///< To encode JPEG file
#define MHAL_IOCTL_JPEG_DEC_SET_READ_FUNC           0x2002  ///< To set the jpeg decode read functon
#define MHAL_IOCTL_JPEG_DEC_START                   0x2003  ///< To start jpeg decode
#define MHAL_IOCTL_JPEG_DEC_GET_INFO                0x2004  ///< To get the jpeg file information
#define MHAL_IOCTL_JPEG_DEC_PARSER                  0x2005  ///< To parser the jpeg file information
#define MHAL_IOCTL_JPEG_DEC_CANCEL                  0x2006  ///< To cancel the jpeg decoder
#define MHAL_IOCTL_WEBP_DEC_START                   0x2011  ///< To start webp decode
#define MHAL_IOCTL_WEBP_DEC_GET_INFO                0x2012  ///< To get the webp file information
#define MHAL_IOCTL_WEBP_DEC_PARSER                  0x2013  ///< To parser the webp file information
#define MHAL_IOCTL_WEBP_DEC_CANCEL                  0x2014  ///< To cancel the webp decoder
#define MHAL_IOCTL_MJPEG_DEC_PARSER                 0x2021  ///< To parser the mjpeg file information
#define MHAL_IOCTL_MJPEG_DEC_GET_INFO               0x2022  ///< To get the mjpeg file information
#define MHAL_IOCTL_MJPEG_DEC_START                  0x2023  ///< To start mjpeg decode
#define MHAL_IOCTL_MJPEG_DEC_CANCEL                 0x2024  ///< To cancel the mjpeg decoder


#define MHAL_IOCTL_MP4_PLAYBACK_OPEN                0x4001  ///< To open MP4 decoder driver
#define MHAL_IOCTL_MP4_PLAYBACK_INIT                0x4002  ///< To init MP4 decoder driver
#define MHAL_IOCTL_MP4_PLAYBACK_RST                 0x4003  ///< To reset MP4 decoder driver
#define MHAL_IOCTL_MP4_PLAYBACK_PARSE_H263_HEADER   0x4004  ///< To parse h263 header
#define MHAL_IOCTL_MP4_PLAYBACK_DECODE              0x4005  ///< To parse header and set HW configuration
#define MHAL_IOCTL_MP4_PLAYBACK_TRIGGER_HW_START    0x4006  ///< To trigger MP4 decoder driver start to decode
#define MHAL_IOCTL_MP4_PLAYBACK_CLOSE               0x4007  ///< To close MP4 decoder driver
#define MHAL_IOCTL_MP4_PLAYBACK_GET_PARAM           0x4008  ///< To get parameter
#define MHAL_IOCTL_MP4_PLAYBACK_SET_PARAM           0x4009  ///< To set parameter

#define MHAL_IOCTL_MP4_RECORD_INIT                  0x4101  ///< To initial MPEG4 record
#define MHAL_IOCTL_MP4_RECORD_ENCODE_ONE_FRAME      0x4102  ///< To encode MPEG4 frame
#define MHAL_IOCTL_MP4_RECORD_DEINIT                0x4103  ///< To deinitial MPEG4 record

#define MHAL_IOCTL_H264_PLAYBACK_INIT               0x4201  ///< To initial H264 playback
#define MHAL_IOCTL_H264_PLAYBACK_DECODE_ONE_FRAME   0x4202  ///< To decode H264 frame
#define MHAL_IOCTL_H264_PLAYBACK_DEINIT             0x4203  ///< To deinitial H264 playback
#define MHAL_IOCTL_H264_PLAYBACK_GET_PARAM          0x4204  ///< To get parameter
#define MHAL_IOCTL_H264_PLAYBACK_SET_PARAM          0x4205  ///< To set parameter
#define MHAL_IOCTL_H264_PLAYBACK_INSERT_BUF         0x4206  ///< To insert buffer
#define MHAL_IOCTL_H264_PLAYBACK_RELEASE_BUF        0x4207  ///< To release buffer
#define MHAL_IOCTL_H264_PLAYBACK_CHECK_BUF          0x4208  ///< To check buffer
#define MHAL_IOCTL_H264_PLAYBACK_SHOW_BUF           0x4209  ///< To show buffer

#define MHAL_IOCTL_BITBLT                           0x8001  ///< To bitblt
#define MHAL_IOCTL_DIRECT_BITBLT_PREPARE            0x8002  ///< To direct link bitblt
#define MHAL_IOCTL_DIRECT_BITBLT_END                0x8003  ///< To release direct link resources

#define MHAL_IOCTL_FACTORY                          0x8100  ///< To run factory mode
#define MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE        0x8101  ///< To config fb immediate update

/*******************************************************************************
* Lock Resource Mode
********************************************************************************/
#define MHAL_MODE_JPEG_DECODE                       0x0001  ///< Record now scenario of jpeg decode
#define MHAL_MODE_JPEG_ENCODE                       0x0002  ///< Record now scenario of jpeg encode
#define MHAL_MODE_CAM_PREVIEW                       0x0004  ///< Record now scenario of camera preview
#define MHAL_MODE_CAM_CAPTURE                       0x0008  ///< Record now scenario of camera capture
#define MHAL_MODE_MP4_PLAYBACK                      0x0010  ///< Record now scenario of MPEG4 playback
#define MHAL_MODE_H264_PLAYBACK                     0x0010  ///< Record now scenario of H264 playback
#define MHAL_MODE_MP4_RECORD                        0x0020  ///< Record now scenario of MPEG4 record
#define MHAL_MODE_BITBLT                            0x0040  ///< Record now scenario of Bitblt

#define MHAL_MODE_ALL                               0x00FF

/*******************************************************************************
* Query Memory Mode
********************************************************************************/
#define MHAL_MEM_MODE_PREVIEW                       0x0001  ///< The query memory mode of camera preview
#define MHAL_MEM_MODE_CAPTURE                       0x0002  ///< The query memory mode of camera capture

/*******************************************************************************
* Lock Resource Interface
********************************************************************************/
typedef struct mHalLockParam_s
{
    MUINT32 mode;
    MUINT32 waitMilliSec;
    MUINT32 waitMode;
} MHalLockParam_t;

/*****************************************************************************
* Video Encoder Interface
*****************************************************************************/
typedef enum
{
    VDO_ENC_FORMAT_H263 = 0,
    VDO_ENC_FORMAT_MPEG4,
    VDO_ENC_FORMAT_AVC,
    
    VDO_ENC_FORMAT_MAX = 0xFFFFFFFF
}VDO_ENC_FORMAT;


/*****************************************************************************
* JPEG Decoder Interface
*****************************************************************************/

typedef unsigned int (*pfReadStreamCallback)(MUINT8* srcStream, MUINT32 size);

typedef enum
{
	JPEG_ERROR_NONE = 0,
	JPEG_ERROR_INVALID_DRIVER,
	JPEG_ERROR_INVALID_CTRL_CODE,
	JPEG_ERROR_INVALID_PARA,
	JPEG_ERROR_INVALID_MEMORY,
	JPEG_ERROR_INVALID_FORMAT,
	
	JPEG_ERROR_ALL = 0xFFFFFFFF
}MHAL_JPEG_ERROR_ENUM;

typedef enum
{
	JPEG_RESULT_DONE = 0,
	JPEG_RESULT_PAUSE,
	JPEG_RESULT_OVERFLOW,
	JPEG_RESULT_HALT,
	
	RESULT_ALL = 0xFFFFFFFF
}MHAL_JPEG_DEC_RESULT_ENUM;

typedef enum {
    JPEG_OUT_FORMAT_RGB565 = 0,
    JPEG_OUT_FORMAT_RGB888,
    JPEG_OUT_FORMAT_ARGB8888,
    JPEG_OUT_FORMAT_YUY2,       // YUYV
    JPEG_OUT_FORMAT_UYVY,       // UYVY
    JPEG_OUT_FORMAT_YV12,       // YVU420 3 plane
    JPEG_OUT_FORMAT_ALL = 0xFFFFFFFF
}JPEG_OUT_FORMAT_ENUM;

typedef struct
{

    MINT32 timeout;     // time for wait IRQ, unit : millisecond
    MUINT8 *srcBuffer;  // avoid reset error, copy src file back
    MUINT32 srcBufSize;
    MUINT32 srcLength;
    MINT32 srcFD;
    
    // destination information
    MUINT8 *dstPhysAddr; // if set NULL, mhal will allocate memory 
    MUINT8 *dstVirAddr;
    MUINT32 dstWidth;
    MUINT32 dstHeight;
    MUINT32 doDithering; // 1: do HW dithering
    MUINT32 doPostProcessing; // 1: do HW PostProcessing
    MUINT32 PreferQualityOverSpeed; // 1: do SLOW_IDCT

    // Range Decode Infor
    MUINT32 doRangeDecode; // 1: do Range Decode, 0: disable
    MUINT32 rangeLeft;
    MUINT32 rangeTop;
    MUINT32 rangeRight;
    MUINT32 rangeBottom;
    
    JPEG_OUT_FORMAT_ENUM dstFormat;
    
    void* jpgDecHandle ;
    MUINT32 fCancelDecoderFlag;
       
}MHAL_JPEG_DEC_START_IN;

typedef struct
{
    MUINT8 *srcBuffer;
    MUINT32 srcLength;
    MINT32 srcFD;
    void* jpgDecHandle ;
    
}MHAL_JPEG_DEC_SRC_IN;

typedef struct
{   
    MUINT32 srcWidth;
    MUINT32 srcHeight;
    void* jpgDecHandle ;
    
}MHAL_JPEG_DEC_INFO_OUT;


/*****************************************************************************
* JPEG Encoder Interface
*****************************************************************************/
typedef enum {
    JPEG_IN_FORMAT_RGB565 = 0,
    JPEG_IN_FORMAT_RGB888,
    JPEG_IN_FORMAT_ARGB8888,
    
    JPEG_IN_FORMAT_ALL = 0xFFFFFFFF
}JPEG_IN_FORMAT_ENUM;

/**
 * @par Enumeration
 *   JPEG_SAMPLING_FORMAT
 * @par Description
 *   The JPEG sampling format you want to encode
 */
typedef enum {
    JPEG_ENC_YUV444 = 444,
    JPEG_ENC_YUV422 = 422,
    JPEG_ENC_YUV411 = 411,
    JPEG_ENC_YUV420 = 420,
    JPEG_ENC_GRAYLEVEL = 400,
   
    JPEG_ENC_ALL = 0xFFFFFFFF

} JPEG_ENC_FORMAT_ENUM;

/**
 * @par Enumeration
 *   JPEG_ENCODE_QUALITY_ENUM
 * @par Description
 *   The JPEG quality you want to encode
 */

typedef struct
{
    MINT32 timeout;     // time for wait IRQ, unit : millisecond

    MUINT8 *srcAddr;
    MUINT8 *dstAddr;
    
    MUINT32 srcWidth;
    MUINT32 srcHeight;

    MUINT32 dstWidth;
    MUINT32 dstHeight;
    
    MUINT32 quality;
    MUINT32 dstSize;
    MUINT32 *encSize;
    
    JPEG_IN_FORMAT_ENUM inFormat;
    JPEG_ENC_FORMAT_ENUM outFormat;  
       
}MHAL_JPEG_ENC_START_IN;

/*******************************************************************************
* Bitblt Interface
********************************************************************************/
typedef enum
{
    MHAL_FORMAT_RGB_565 = 0,
    MHAL_FORMAT_BGR_565,
    MHAL_FORMAT_RGB_888,
    MHAL_FORMAT_BGR_888,
    MHAL_FORMAT_ARGB_8888,
    MHAL_FORMAT_ABGR_8888,
    MHAL_FORMAT_BGRA_8888,
    MHAL_FORMAT_RGBA_8888,
    MHAL_FORMAT_YUV_420,
    MHAL_FORMAT_YUV_420_SP,
    MHAL_FORMAT_MTK_YUV,
    MHAL_FORMAT_YUY2,
    MHAL_FORMAT_UYVY,
    MHAL_FORMAT_Y800,
    MHAL_FORMAT_YUV_422_PL,      //422 Planar,i.e YV16 Planar
    MHAL_FORMAT_ANDROID_YV12,   //Androdi YV12.YVU stride all 16 pixel align
    MHAL_FORMAT_IMG_YV12,       //Imagination YV12.YVU stride all 32 pixel align
    

    MHAL_FORMAT_ERROR,

    MHAL_FORMAT_ALL = 0xFFFFFFFF
} MHAL_BITBLT_FORMAT_ENUM;

typedef enum
{
    MHAL_BITBLT_ROT_0 = 0,
    MHAL_BITBLT_ROT_90 = 0x1,
    MHAL_BITBLT_ROT_180 = 0x2,
    MHAL_BITBLT_ROT_270 = 0x3,// (90 + 180)
    MHAL_BITBLT_FLIP_H = 0x4,

    MHAL_BITBLT_ALL = 0xFFFFFFFF
} MHAL_BITBLT_ROT_ENUM;

typedef struct mHalBltParam_s
{
    MUINT32 m4u_handle; //NULL imply VA exist in current process
    
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
    MUINT32 doImageProcess;// 0:disable, 0x1:color process+EE, 0x2:do color process only , 0x3:face beautify, bit[4]: 0-do bitblt if isp is N.A , 1-skip if isp is N.A.
    MUINT32 favor_flags;    /*Image Transform favor flags,this is a low level flags. default is 0*/
    

    MUINT32 u4SrcOffsetXFloat;//0x100 stands for 1, 0x40 stands for 0.25 , etc...
    MUINT32 u4SrcOffsetYFloat;//0x100 stands for 1, 0x40 stands for 0.25 , etc...


    /*Resizer coeff if can apply*/
    MUINT32 resz_up_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    MUINT32 resz_dn_scale_coeff;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    MUINT32 resz_ee_h_str;          //down scale only/0~15
    MUINT32 resz_ee_v_str;          //down scale only/0~15
} mHalBltParam_t;


typedef struct mHalBltExParam_s
{
    /*Source*/
    MHAL_BITBLT_FORMAT_ENUM  src_color_format;
    
    MUINT32 src_img_size_w;
    MUINT32 src_img_size_h;

    MUINT32 src_img_roi_x;
    MUINT32 src_img_roi_y;
    MUINT32 src_img_roi_w;
    MUINT32 src_img_roi_h;
    
    MUINT32 src_yuv_img_addr;
    

    /*For HDMI : RGB*/
    MINT32  en_hdmi_port;           //enable hdmi port
        MHAL_BITBLT_FORMAT_ENUM hdmi_color_format;

        MUINT32 hdmi_img_size_w;
        MUINT32 hdmi_img_size_h;

        
        MUINT32 hdmi_img_roi_x;
        MUINT32 hdmi_img_roi_y;
        MUINT32 hdmi_img_roi_w;
        MUINT32 hdmi_img_roi_h;
        
        MUINT32 hdmi_yuv_img_addr;
        MUINT32 hdmi_rotate_angle;

    /*For Display : 422 Pack*/
    MINT32             en_disp_port;           //enable disp port
        MHAL_BITBLT_FORMAT_ENUM disp_color_format;
        
        MUINT32 disp_img_size_w;
        MUINT32 disp_img_size_h;
        
        MUINT32 disp_img_roi_x;
        MUINT32 disp_img_roi_y;
        MUINT32 disp_img_roi_w;
        MUINT32 disp_img_roi_h;
        
        MUINT32 disp_yuv_img_addr;
        MUINT32 disp_rotate_angle;
    
    /*Resizer coeff if can apply*/
    MUINT32 resz_up_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    MUINT32 resz_dn_scale_coeff;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    MUINT32 resz_ee_h_str;          //down scale only/0~15
    MUINT32 resz_ee_v_str;          //down scale only/0~15
} mHalBltParamEx_t;



typedef enum
{
    MHAL_MLM_CLIENT_SFTEX,        //Surface flinger Texture
    MHAL_MLM_CLIENT_MTKOVERLAY,  
    MHAL_MLM_CLIENT_PV2ND,        //Preview 2nd path
    MHAL_MLM_CLIENT_ELEMENT,      //MDP Element
    MHAL_MLM_CLIENT_PVCPY,        //Camera preview path extra-copy
    MHAL_MLM_CLIENT_GRALLOC,      //Graphic Buffer Allocator

    MHAL_MLM_CLIENT_MAX = 19,    //MAX (for test)
    
} mHalMVALOOPMEM_CLIENT;

typedef enum
{
    MHAL_MEM_TYPE_INPUT,
    MHAL_MEM_TYPE_OUTPUT
    
} mHalREGLOOPMEM_TYPE;



#ifdef __cplusplus

class mHalMdpSize
{
public:
    unsigned long w;
    unsigned long h;

public:
    mHalMdpSize():
        w(0),h(0)
        {};

   
    mHalMdpSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };

    
    mHalMdpSize& operator=( const mHalMdpSize& rhs )
    {
        w = rhs.w; h = rhs.h;
        return *this;
    }

    bool operator==(const mHalMdpSize& rhs) const
    {
        return (  ( w == rhs.w ) && ( h == rhs.h ) );
    }

    bool operator!=(const mHalMdpSize& rhs) const
    {
        return !( *this == rhs );
    }

};

class mHalMdpRect
{
public:
    long            x;
    long            y;
    unsigned long   w;
    unsigned long   h;
    
public:
    mHalMdpRect():
        x(0),y(0),w(0),h(0)
        {};

    mHalMdpRect(long _x, long _y, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; w = _w; h = _h;
        };
    
    mHalMdpRect(const mHalMdpSize& size )
        {
            x = 0; y = 0; w = size.w; h = size.h;
        };

    mHalMdpRect& operator=( const mHalMdpRect& rhs )
    {
        x = rhs.x; y = rhs.y; w = rhs.w; h = rhs.h;
        return *this;
    }

    bool operator==(const mHalMdpRect& rhs) const
    {
        return (  (x == rhs.x) && ( y == rhs.y) && ( w == rhs.w ) && ( h == rhs.h ) );
    }

    bool operator!=(const mHalMdpRect& rhs) const
    {
        return !( *this == rhs );
    }
    
    
};

class mHalMdpYuvAddr
{
public:
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;
    
    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by MDP driver calculation!----------------------------------------------*/
    
    unsigned long   y_buffer_size; 
    unsigned long   u_buffer_size; 
    unsigned long   v_buffer_size;

public:
    mHalMdpYuvAddr():
        y(0), u(0), v(0),
        y_buffer_size(0), u_buffer_size(0), v_buffer_size(0)
        {};
};



class mHalRegisterLoopMemory_t
{
public:
    mHalREGLOOPMEM_TYPE         mem_type;
    unsigned long               addr;
    unsigned long               buffer_size;
    MHAL_BITBLT_FORMAT_ENUM     mhal_color;
    mHalMdpSize                 img_size;
    mHalMdpRect                 img_roi;
    unsigned long               rotate; //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)

public:
    mHalRegisterLoopMemory_t():
        mem_type(MHAL_MEM_TYPE_INPUT),
        addr(0),
        buffer_size(0),
        mhal_color(MHAL_FORMAT_RGB_565),
        rotate(0) //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)
        {};
        
};

class mHalRegisterLoopMemoryObj_t
{
public:
    unsigned long       mdp_id;
    mHalMdpYuvAddr      calc_addr[1];
    mHalMdpYuvAddr      adapt_addr[1];
    unsigned long       adapt_m4u_flag_bit;
    unsigned long       alloc_mva_flag_bit;

public:
    mHalRegisterLoopMemoryObj_t():
        mdp_id(0),
        adapt_m4u_flag_bit(0),
        alloc_mva_flag_bit(0)
        {};
};

#else /*#ifdef __cplusplus*/

typedef struct mHalMdpSize_s
{
    unsigned long w;
    unsigned long h;
} mHalMdpSize;


typedef struct mHalMdpRect_s
{
    long            x;
    long            y;
    unsigned long   w;
    unsigned long   h;
} mHalMdpRect;

typedef struct mHalMdpYuvAddr_s
{
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;
    
    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by MDP driver calculation!----------------------------------------------*/
    
    unsigned long   y_buffer_size; 
    unsigned long   u_buffer_size; 
    unsigned long   v_buffer_size;

} mHalMdpYuvAddr;

typedef struct mHalRegisterLoopMemory_s
{
    mHalREGLOOPMEM_TYPE         mem_type;
    unsigned long               addr;
    unsigned long               buffer_size;
    MHAL_BITBLT_FORMAT_ENUM     mhal_color;
    mHalMdpSize                 img_size;
    mHalMdpRect                 img_roi;
    unsigned long               rotate; //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)
} mHalRegisterLoopMemory_t;

typedef struct mHalRegisterLoopMemoryObj_s
{
    unsigned long       mdp_id;
    mHalMdpYuvAddr      calc_addr[1];
    mHalMdpYuvAddr      adapt_addr[1];
    unsigned long       adapt_m4u_flag_bit;
    unsigned long       alloc_mva_flag_bit;
} mHalRegisterLoopMemoryObj_t;


#endif /*#ifdef __cplusplus*/



/*******************************************************************************
* Camera Interface
********************************************************************************/

/**
 * @par Structure
 *   mhalCamParam_t
 * @par Description
 *   This is a structure which store camera configuration
 */
typedef struct mhalCamVdoParam_s {
    MUINT32 u4YUVWidth; 
    MUINT32 u4YUVHeight; 
    MUINT32 u4VEncBitrate;      // Video encode bitrate
    VDO_ENC_FORMAT eVEncFormat;       // Video encode format
    MUINT32 u4PrvFps;
}mhalCamVdoParam_t; 

//
typedef struct priority_data {
    MHAL_CHAR name[20];
    MHAL_INT32 policy;
    MHAL_INT32 priority;
} mhalPriorityData_t;

typedef struct mHalMp4VEncParam_s
{
    void *pObj;
    void *pInputBuf;
}mHalMp4VEncParam_t;

/*=============================================================================
 *                             Function Declaration
 *===========================================================================*/

/**
 * @par Function
 *   mHalOpen
 * @par Description
 *   Open a mHal instance
 * @par Returns
 *   return a created instance.
 */
void*
mHalOpen();


/**
 * @par Function
 *   mHalClose
 * @par Description
 *   Close a mHal instance
 * @param
 *   fd     [I]     file descriptor returned from mHalOpen()

 * @par Returns
 *   none.
 */
void
mHalClose(void* fd);


/**
 * @par Function
 *   mHalIoctl
 * @par Description
 *   mHal ioctl function with fd
 * @param
 *   uCtrlCode          [I] The IO Control Code
 * @param
 *   pvInBuf            [I] The input parameter
 * @param
 *   uInBufSize         [I] The size of input parameter structure
 * @param
 *   pvOutBuf           [O] The output parameter
 * @param
 *   uOutBufSize        [I] The size of output parameter structure
  * @param
 *   puBytesReturned    [O] The number of bytes of return value
 * @par Returns
 *   error code
 */
int
mHalIoctl(
    void*           fd, 
    unsigned int    uCtrlCode, 
    void*           pvInBuf, 
    unsigned int    uInBufSize, 
    void*           pvOutBuf, 
    unsigned int    uOutBufSize, 
    unsigned int*   puBytesReturned
);


/**
 * @par Function
 *   mHalIoCtrl
 * @par Description
 *   The MHAL ioctrl function
 * @param
 *   a_u4CtrlCode       [IN]       The IO Control Code
 * @param
 *   a_pInBuffer        [IN]       The input parameter
 * @param
 *   a_u4InBufSize      [IN]       The size of input parameter structure
 * @param
 *   a_pOutBuffer       [OUT]      The output parameter
 * @param
 *   a_u4OutBufSize     [IN]       The size of output parameter structure
  * @param
 *   pBytesReturned     [OUT]      The number of bytes of return value
 * @par Returns
 *   MINT32
 */
#ifdef __cplusplus 
extern "C"{
#endif

MINT32
mHalIoCtrl(
    MUINT32 a_u4CtrlCode,
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize,
    MVOID *a_pOutBuffer,
    MUINT32 a_u4OutBufSize,
    MUINT32 *pBytesReturned
);

MINT32
mHalJpeg(
    MUINT32 a_u4CtrlCode,
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize,
    MVOID *a_pOutBuffer,
    MUINT32 a_u4OutBufSize,
    MUINT32 *pBytesReturned
);


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus 
extern "C"{
#endif
MINT32
mtk_AdjustPrio(
    MHAL_CHAR *name
);
#ifdef __cplusplus
}
#endif


/*******************************************************************************
* MDP mHal Interface
********************************************************************************/
#ifdef __cplusplus 
extern "C"{
#endif

signed int mHalMdp_BitBlt( void *a_pInBuffer );
signed int mHalMdpIpc_BitBlt( mHalBltParam_t* bltParam );
signed int mHalMdpLocal_BitBlt( mHalBltParam_t* bltParam );

signed int mHalMdp_BitBltEx( mHalBltParamEx_t* p_param );
signed int mHalMdp_BitbltSlice( void *a_pInBuffer );




int mHalMdp_RegisterLoopMemory(   mHalMVALOOPMEM_CLIENT client_id, mHalRegisterLoopMemory_t* p_param, mHalRegisterLoopMemoryObj_t* p_out_obj );
int mHalMdp_UnRegisterLoopMemory( mHalMVALOOPMEM_CLIENT client_id, mHalRegisterLoopMemoryObj_t* p_obj );


#ifdef __cplusplus
}
#endif


#endif // MEDIA_HAL_H


