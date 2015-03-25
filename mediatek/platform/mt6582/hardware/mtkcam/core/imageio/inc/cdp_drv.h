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
* @file cdp_drv.h
*
* cdp_drv Header File
*/

#ifndef _CDP_DRV_H_
#define _CDP_DRV_H_

#include "imageio_types.h"
#include <mtkcam/drv/isp_drv.h>    // for IspDrv class.
#include <mtkcam/drv/isp_reg.h>    // For isp_reg_t.

using namespace android;


/**
*@brief  CDP driver mode enum
*/
typedef enum
{
    CDP_DRV_MODE_FRAME,
    CDP_DRV_MODE_TPIPE,
    CDP_DRV_MODE_VRMRG
} CDP_DRV_MODE_ENUM;

/**
*@brief  CDP resize enum
*/
typedef enum
{
    CDP_DRV_RZ_CDRZ,
    CDP_DRV_RZ_CURZ,
    CDP_DRV_RZ_PRZ,
    CDP_DRV_RZ_AMOUNT
} CDP_DRV_RZ_ENUM;

/**
*@brief  CDP rotate dma enum
*/
typedef enum
{
//    CDP_DRV_ROTDMA_VRZO,  //js_test remove below later
    CDP_DRV_ROTDMA_VIDO,
    CDP_DRV_ROTDMA_DISPO,   // DISPO rotation is removed in 83.
    CDP_DRV_ROTDMA_AMOUNT
} CDP_DRV_ROTDMA_ENUM;

/**
*@brief  CDP resize algorithm enum
*/
typedef enum
{
//    CDP_DRV_ALGO_8_TAP,
    CDP_DRV_ALGO_4_TAP, // For 83, only 4-tap.
    CDP_DRV_ALGO_N_TAP,
    CDP_DRV_ALGO_4N_TAP,
    CDP_DRV_ALGO_AMOUNT
} CDP_DRV_ALGO_ENUM;

/**
*@brief  PRZ source select enum
*/
typedef enum    // PRZ_OPT_SEL. PRZ optional path selection. 0(from PRZ mux ouput)/1(from DIPI output)/2(from before CDRZ).
{
    CDP_DRV_PRZ_SRC_AFTER_CURZ,
    CDP_DRV_PRZ_SRC_BEFORE_CURZ,
    CDP_DRV_PRZ_SRC_BEFORE_CDRZ
} CDP_DRV_PRZ_SRC_ENUM;

/**
*@brief  DISPO source select enum
*/
typedef enum    // DISP_VID_SEL. Display and video dma input selection. 0(Video is from CRZ, display is from PRZ)/1(Video is from PRZ, display is from CRZ).
{
    CDP_DRV_DISPO_SRC_PRZ,  //DISPO source is PRZ, and VIDO source is RSP
    CDP_DRV_DISPO_SRC_RSP   //DISPO source is RSP, and VIDO source is PRZ
} CDP_DRV_DISPO_SRC_ENUM;

/**
*@brief  CDP image format enum
*/
typedef enum
{
    CDP_DRV_FORMAT_YUV422,
    CDP_DRV_FORMAT_YUV420,
    CDP_DRV_FORMAT_Y,
    CDP_DRV_FORMAT_RGB888,
    CDP_DRV_FORMAT_RGB565,
    CDP_DRV_FORMAT_XRGB8888
} CDP_DRV_FORMAT_ENUM;

/**
*@brief  CDP image plane number enum
*/
typedef enum
{
    CDP_DRV_PLANE_1,
    CDP_DRV_PLANE_2,
    CDP_DRV_PLANE_3
} CDP_DRV_PLANE_ENUM;

/**
*@brief  CDP sequence enum
*/
typedef enum
{
    // For YUV422 format 1-plane/3-plane.
    CDP_DRV_SEQUENCE_YVYU   = 0,
    CDP_DRV_SEQUENCE_YUYV   = 1,
    CDP_DRV_SEQUENCE_VYUY   = 2,
    CDP_DRV_SEQUENCE_UYVY   = 3,

    // For YUV422 format 2-plane, or YUV420 format.
    CDP_DRV_SEQUENCE_VUVU   = 0,
    CDP_DRV_SEQUENCE_UVUV   = 1,

    // For RGB888 format, or RGB565 format .
    CDP_DRV_SEQUENCE_RGB    = 0,
    CDP_DRV_SEQUENCE_BGR    = 1,

    // For XRGB8888 format.
    CDP_DRV_SEQUENCE_XRGB   = 0,
    CDP_DRV_SEQUENCE_XBGR   = 1,
    CDP_DRV_SEQUENCE_RGBX   = 2,
    CDP_DRV_SEQUENCE_BGRX   = 3,

    // For YOnly format.
    CDP_DRV_SEQUENCE_Y      = CDP_DRV_SEQUENCE_YVYU,

} CDP_DRV_SEQUENCE_ENUM;

/**
*@brief  CDP rotation enum
*/
typedef enum
{
    CDP_DRV_ROTATION_0 = 0,
    CDP_DRV_ROTATION_90,
    CDP_DRV_ROTATION_180,
    CDP_DRV_ROTATION_270
}CDP_DRV_ROTATION_ENUM;

/**
*@brief  image size
*/
typedef struct
{
    MUINT32     Width;
    MUINT32     Height;
} CDP_DRV_IMG_SIZE_STRUCT;

/**
*@brief  crop size
*/
typedef struct
{
    MFLOAT      Start;
    MUINT32     Size;
} CDP_DRV_CROP_STRUCT;

/**
*@brief  crop info
*/
typedef struct
{
    CDP_DRV_CROP_STRUCT     Width;
    CDP_DRV_CROP_STRUCT     Height;
} CDP_DRV_IMG_CROP_STRUCT;

//-----------------------------------------------------------------------------
/*

    CDRZ ---> 3DNR ---> CURZ ---> VIDO
                    |
                     ---> PRZ ---> DISPO

    Program flow as fllow:

    1.
        CreateInstance()
        Init()
    2.
        CDRZ_Config()               //If CDRZ is enabled...
    3.
        CURZ_Config()               //If CURZ is enabled...
                                                            //4.
                                                            //    VRZ_Config()                //If VRZ is enabled...
                                                            //5.
                                                            //    VRZO_Config()               //If VRZO is enabled...
                                                            //    VRZO_SetOutputAddr()        //If VRZO is enabled...
                                                            //6.
                                                            //    RSP_Enable()
    7.
        VIDO_Config()               //If VIDO is enabled...
        VIDO_SetOutputAddr()        //If VIDO is enabled...
    8.
        PRZ_Config()                //If PRZ is enabled...
    9.
        DISPO_Config()              //If DISPO is enabled...
        DISPO_SetOutputAddr()       //If DISPO is enabled...
    10.
        Do image process...
    11.
        CDRZ_Unconfig()         //If CDRZ was configed...
    12.
        CURZ_Unconfig()         //If CURZ was configed...
                                                            //13.
                                                            //    VRZ_Unconfig()          //If VRZ was configed...
                                                            //14.
                                                            //    VRZO_Unconfig()         //If VRZ was configed...
                                                            //15.
                                                            //    RSP_Enable()            //If PRZ was enabled...
    16
        VIDO_Unconfig()         //If VIDO was configed...
    17.
        PRZ_Unconfig()          //If PRZ was configed...
    18.
        DISPO_Unconfig()        //If DISPO was configed...
    19.
        Uninit()
        DestroyInstance()
*/
//-----------------------------------------------------------------------------

/**
*@brief CdpDrv class
*/
class CdpDrv
{
    protected:

        /**
             *@brief  Destructor      
             */
        virtual ~CdpDrv() {};
    
    public:

        /**
             *@brief  Create CdpDrv class object
             *@param[in] fgIsGdmaMode : is GDMA mode or not, only used in 6589
             *@return
             *-Pointer to CdpDrv object, otherwise indicates failure
             */
        static CdpDrv *CreateInstance(MBOOL fgIsGdmaMode = MFALSE);

        /**
             *@brief  Destroy CdpDrv class object
             */
        virtual MVOID DestroyInstance() = 0;

        /**
             *@brief  Initialize
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL Init() = 0;

        /**
             *@brief  Uninitialize
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL Uninit() = 0;

        /**
             *@brief  Set ISP register
             *@param[in] pIspReg : register data
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL SetIspReg(isp_reg_t *pIspReg) = 0;

        /**
             *@brief  Set physical ISP driver
             *@param[in] pPhyIspDrv : isp physical driver
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL SetPhyIspDrv(IspDrv *pPhyIspDrv) = 0;
        
//        virtual MBOOL   CalAlgoAndCStep(
//            CDP_DRV_RZ_ENUM         eRzName,
//            MUINT32                 SizeIn,
//            MUINT32                 u4CroppedSize,
//            MUINT32                 SizeOut,
//            CDP_DRV_ALGO_ENUM       *pAlgo,
//            MUINT32                 *pTable,
//            MUINT32                 *pCoeffStep) = 0;

        /**
             *@brief  CDP resize algorithm
             *@param[in] eFrameOrTpipeOrVrmrg : frame,tpipe or vrmrg
             *@param[in] eRzName : resizer name
             *@param[in] SizeIn_H : input width
             *@param[in] SizeIn_V : input height
             *@param[in] u4CroppedSize_H : cropped wdith
             *@param[in] u4CroppedSize_V : cropprd height
             *@param[in] SizeOut_H : output width
             *@param[in] SizeOut_V : output height
             *@param[in] pAlgo_H : resize algorithm for width
             *@param[in] pAlgo_V : resize algorithm for height
             *@param[in] pTable_H : resize table for width
             *@param[in] pTable_V : resize table for height
             *@param[in] pCoeffStep_H : coefficient step for width
             *@param[in] pCoeffStep_V : coefficient step for height
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL CalAlgoAndCStep(
            CDP_DRV_MODE_ENUM       eFrameOrTpipeOrVrmrg,
            CDP_DRV_RZ_ENUM         eRzName,
            MUINT32                 SizeIn_H,
            MUINT32                 SizeIn_V,
            MUINT32                 u4CroppedSize_H,
            MUINT32                 u4CroppedSize_V,
            MUINT32                 SizeOut_H,
            MUINT32                 SizeOut_V,
            CDP_DRV_ALGO_ENUM *pAlgo_H,
            CDP_DRV_ALGO_ENUM *pAlgo_V,
            MUINT32 *pTable_H,
            MUINT32 *pTable_V,
            MUINT32 *pCoeffStep_H,
            MUINT32 *pCoeffStep_V) = 0;

        /**
             *@brief  Rest CDP driver            
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   Reset() = 0;

        /**
             *@brief  Rest CDP driver to default          
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   ResetDefault() = 0;

        /**
             *@brief  Dump CDP register     
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   DumpReg() = 0;

        /**
             *@brief  Mapping rotate dma enum
             *@param[in] eInFormat : input format enum
             *@param[in] eInPlane : input plane enum
             *@param[in] pu4OutPlane : out put plane
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   RotDmaEnumRemapping(
            CDP_DRV_FORMAT_ENUM eInFormat,
            CDP_DRV_PLANE_ENUM eInPlane,
            MUINT32 *pu4OutPlane) = 0;

        /**
             *@brief  Check input format
             *@param[in] eInFormat : input format enum
             *@param[in] eInPlane : input plane enum
             *@param[in] eInSequence : input sequence enum
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   InputImgFormatCheck(
            CDP_DRV_FORMAT_ENUM     eInFormat,
            CDP_DRV_PLANE_ENUM      eInPlane,
            CDP_DRV_SEQUENCE_ENUM   eInSequence) = 0;

        
        /**
             *@brief  Congifure CDRZ
             *@param[in] eFrameOrTpipeOrVrmrg : input format enum
             *@param[in] SizeIn : input size
             *@param[in] SizeOut : output size
             *@param[in] Crop : crop info
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   CDRZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop) = 0;

        /**
             *@brief  Disable CDRZ            
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   CDRZ_Unconfig() = 0;
        
        /**
             *@brief  Congifure CURZ
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL CURZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop) = 0;

        /**
             *@brief  Disable CURZ
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL CURZ_Unconfig() = 0;

#if 0 //js_test remove below later
        //VRZ
        virtual MBOOL   VRZ_Config(
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop) = 0;
        virtual MBOOL   VRZ_Unconfig(void) = 0;
#endif //js_test remove below later

        /**
             *@brief  Congifure PRZ
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   PRZ_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
            CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
            CDP_DRV_IMG_CROP_STRUCT     Crop) = 0;

        /**
             *@brief  Disable PRZ
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   PRZ_Unconfig() = 0;

#if 0 //js_test remove below later
        //VRZO
        virtual MBOOL   VRZO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC) = 0;
        virtual MBOOL   VRZO_Config(
            CDP_DRV_MODE_ENUM           eFrameOrTpipeOrVrmrg,
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip) = 0;
        virtual MBOOL   VRZO_Unconfig(void) = 0;
#endif //js_test remove below later

        /**
             *@brief  Set VIDO output address
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   VIDO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC,
            MUINT32     PhyAddrV,
            MUINT32     OffsetV,
            MUINT32     StrideV) = 0;

        /**
             *@brief  Congifure VIDO
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   VIDO_Config(
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip,
            MUINT32                     u4TpipeWidth,
            MBOOL                       fgDitherEnable) = 0;

        /**
             *@brief  Disable VIDO
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   VIDO_Unconfig() = 0;

        /**
             *@brief  Set DISPO source
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   DISPO_SetSource(CDP_DRV_DISPO_SRC_ENUM Source) = 0;

        /**
             *@brief  Set DISPO output address
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   DISPO_SetOutputAddr(
            MUINT32     PhyAddr,
            MUINT32     Offset,
            MUINT32     Stride,
            MUINT32     PhyAddrC,
            MUINT32     OffsetC,
            MUINT32     StrideC,
            MUINT32     PhyAddrV,
            MUINT32     OffsetV,
            MUINT32     StrideV) = 0;

        /**
             *@brief  Congifure DISPO
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   DISPO_Config(
            CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
            CDP_DRV_IMG_CROP_STRUCT     Crop,
            CDP_DRV_FORMAT_ENUM         Format,
            CDP_DRV_PLANE_ENUM          Plane,
            CDP_DRV_SEQUENCE_ENUM       Sequence,
            CDP_DRV_ROTATION_ENUM       Rotation,
            MBOOL                       Flip,
            MBOOL                       fgDitherEnable) = 0;

        /**
             *@brief  Disable VIDO
             *@note no use in 6582             
             *@return
             *-MTRUE indicates success, otherwise indicates failure
             */
        virtual MBOOL   DISPO_Unconfig() = 0;
};

#endif  // _CDP_DRV_H_

