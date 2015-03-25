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
/*
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FACEBEAUTIFY_HAL_BASE_H_
#define _FACEBEAUTIFY_HAL_BASE_H_

/*******************************************************************************
*
********************************************************************************/                 
#define FBFDWidth  320 //if image is vertical  FD width and Height must be exchange
#define FBFDHeight  240 //if image is vertical  FD width and Height must be exchange

#define gSmoothLevel    5;
#define gContrastLevel  5;
#define gBrightLevel    4;
#define gRuddyLevel     4;
#define gWarpLevel      4;
#define gWarpFaceNum    6;
#define gMinFaceRatio   10;

typedef enum HalFACEBEAUTIFYObject_s {
    HAL_FACEBEAUTY_OBJ_NONE = 0,
    HAL_FACEBEAUTY_OBJ_SW,
    HAL_FACEBEAUTY_OBJ_SW_NEON,    
    HAL_FACEBEAUTY_OBJ_UNKNOWN = 0xFF,
} HalFACEBEAUTIFYObject_e;

typedef enum
{
    MTKPIPEFACEBEAUTY_IMAGE_YUV422,                 // input image format
    MTKPIPEFACEBEAUTY_IMAGE_MAX
} MTKPIPEFACEBEAUTY_IMAGE_FORMAT_ENUM;

struct MTKPipeFaceBeautyTuningPara
{
    MINT32 SmoothLevel;                             //0:Extract skin mask + apply wrinkle removal, 1:Extract skin mask only    
    MINT32 ContrastLevel ;                           //0:close skin tone adjustment; 1:open skin tone adjustment
    MINT32 BrightLevel ;                            //0~4 (weak~strongest)
    MINT32 RuddyLevel ;                      //0~255 (non-smooth to strongest smooth)
    MINT32 WarpLevel ;                            //zoom ratio of down-sampled image
    MINT32 WarpFaceNum;                          //
	  MINT32 MinFaceRatio;
	  MINT32 AlignTH1;
	  MINT32 AlignTH2;
};

struct MTKPipeFaceBeautyEnvInfo
{
    MUINT16  Step2SrcImgWidth;                   
    MUINT16  Step2SrcImgHeight;                  
    MUINT16  Step1SrcImgWidth;                   
    MUINT16  Step1SrcImgHeight;                  
    MUINT16  SrcImgWidth;                        
    MUINT16  SrcImgHeight;                       
    MUINT16  FDWidth;                            
    MUINT16  FDHeight;                           
    MTKPIPEFACEBEAUTY_IMAGE_FORMAT_ENUM SrcImgFormat;
    MBOOL    STEP1_ENABLE;                       
    MUINT32  WorkingBufAddr;                     
    MUINT32  WorkingBufSize;                     
    MTKPipeFaceBeautyTuningPara *pTuningPara;  
};

struct MTKPipeFaceBeautyResultInfo
{
    MUINT8* Step1ResultAddr;
    MFLOAT* Score;							
    MFLOAT* TfmMtxI2A;						
    MFLOAT* Ali9pts;						
    MFLOAT* scaleA2I;
    MUINT8* ImgAliYUV;                      
    MUINT8* Step3ResultAddr_1;              
    MUINT8* Step3ResultAddr_2;              
    MUINT8* Step4ResultAddr;                
    MUINT8* Step5ResultAddr;                
    MUINT8* Step6ResultAddr;    
};

/*******************************************************************************
*
********************************************************************************/
class halFACEBEAUTIFYBase {
public:
    //
    MBOOL CANCEL;
    
    static halFACEBEAUTIFYBase* createInstance(HalFACEBEAUTIFYObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halFACEBEAUTIFYBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFacebeautifyInit () -
    //! \brief init facebeautify 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyInit(void* FaceBeautyEnvInfo) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFacebeautifyUninit () -
    //! \brief Facebeautify uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP2 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP2(void* ImgSrcAddr, void* FaceMetadata, void* FaceBeautyResultInfo) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP3 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP3(void* ImgSrcAddr, void* FaceBeautyResultInfo) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP1 () -
    //! 
    //
    ///////////////////////////////////////////////////////////////////////// 
    virtual MINT32 mHalSTEP1(void* ImgSrcAddr, void* FaceBeautyResultInfo){return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP4 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP4(void* ImgSrcAddr,void* BlurResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo){return 0;}     
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP5 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP5(void* ImgSrcAddr,void* AplhaMapColorBuffer,void* FaceBeautyResultInfo){return 0;}  
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSTEP6 () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalSTEP6(void* ImgSrcAddr,void* WarpWorkBufAdr,void* FaceBeautyResultInfo){return 0;} 
public:
    /////////////////////////////////////////////////////////////////////////
    //
    // Get Working buffer size () -
    //! 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 getWorkingBuffSize(int SrcImgWidth, int SrcImgHeight, int Step2SrcImgWidth, int Step2SrcImgHeight, int Step1SrcImgWidth, int Step1SrcImgHeight){return 0;}      
};

class halFACEBEAUTIFYTmp : public halFACEBEAUTIFYBase {
public:
    //
    static halFACEBEAUTIFYBase* getInstance();
    virtual void destroyInstance();
    //
    halFACEBEAUTIFYTmp() {}; 
    virtual ~halFACEBEAUTIFYTmp() {};
};

#endif

