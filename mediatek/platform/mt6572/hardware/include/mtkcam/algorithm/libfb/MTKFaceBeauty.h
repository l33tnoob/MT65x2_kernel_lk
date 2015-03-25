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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#ifndef _MTK_FACEBEAUTY_H
#define _MTK_FACEBEAUTY_H

#include "MTKFaceBeautyType.h"
#include "MTKFaceBeautyErrCode.h"

#define _FACEBEAUTY_MAX_IN_WIDTH             (4096)                  
#define _FACEBEAUTY_MAX_IN_HEIGHT            (3072)                
#define _FACEBEAUTY_MAX_STEP2_IN_WIDTH       (640)         
#define _FACEBEAUTY_MAX_STEP2_IN_HEIGHT      (480)         
#define _FACEBEAUTY_MAX_STEP1_IN_WIDTH       (2048)         
#define _FACEBEAUTY_MAX_STEP1_IN_HEIGHT      (1536)       

//20120903,Face Alignment working buffer size
//#define MAX_FD_NUM 16
#define _MAXFACENUM 16
#define _NPARTS  9
#define _IMGSIZE  80
#define _ALIW 120
#define _ALIH 220
#define _IMGSIZE_SQ  6400
#define _HI_BUFFER  (4*16*800)
#define _SIGN_BUFFER (4*16*800)
#define _II_BUFFER (4*(_IMGSIZE+1)*(_IMGSIZE+1))
#define _AC_BUFFER (4*_NPARTS*_IMGSIZE_SQ)
#define _PTS_BUFFER (4*2*_NPARTS)
#define _BMAP_BUFFER (4*_NPARTS*_IMGSIZE_SQ)
#define _ARGB_BUFFER (4*_NPARTS*_IMGSIZE_SQ)
#define _CONF_BUFFER (4*_IMGSIZE_SQ)
#define _DMAP_BUFFER (4*_IMGSIZE_SQ)
#define _LMAP_BUFFER (4*_IMGSIZE_SQ)
#define _ALIGN_IMG_BUFFER (_MAXFACENUM*_ALIW*_ALIH*3)
#define _FA_WORKING_BUFFER_SIZE _HI_BUFFER+_SIGN_BUFFER+_II_BUFFER+_AC_BUFFER+_PTS_BUFFER+_BMAP_BUFFER+_ARGB_BUFFER+_CONF_BUFFER+_DMAP_BUFFER+_LMAP_BUFFER+_ALIGN_IMG_BUFFER
//20120903, Face Beauty working buffer size
#define	_STEP2_BUFFER \
 _FACEBEAUTY_MAX_STEP2_IN_WIDTH*_FACEBEAUTY_MAX_STEP2_IN_HEIGHT\
+_FACEBEAUTY_MAX_STEP2_IN_WIDTH*_FACEBEAUTY_MAX_STEP2_IN_HEIGHT\
+4*_FACEBEAUTY_MAX_STEP2_IN_WIDTH*_FACEBEAUTY_MAX_STEP2_IN_HEIGHT \
+4*_FACEBEAUTY_MAX_STEP2_IN_WIDTH*_FACEBEAUTY_MAX_STEP2_IN_HEIGHT \
+4*_FACEBEAUTY_MAX_IN_WIDTH*_FACEBEAUTY_MAX_IN_HEIGHT  

#define _STEP1_BUFFER \
/*gYuvLaplasPyramid*/       _FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT*134/100*3/2 \
/*gLaplasCrntMatrix*/       +_FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT \
/*gLaplasDSMatrix*/         +_FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT \
/*gDownSampleMatrix*/       +_FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT \
/*gReconstuctAuxMatrix*/    +_FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT \
/*gAuxResult*/              +_FACEBEAUTY_MAX_STEP1_IN_WIDTH*_FACEBEAUTY_MAX_STEP1_IN_HEIGHT*3/2 \
/*4 bytes align*/           +16 \
/*Header Buffer Size*/      +3000
////*yuv_images*/           +FACEBEAUTY_MAX_NR_DS_IN_WIDTH*FACEBEAUTY_MAX_NR_DS_IN_HEIGHT*3/2 \

#define FACEBEAUTY_WORKING_BUFFER_SIZE  (  _STEP1_BUFFER>(_FA_WORKING_BUFFER_SIZE+_STEP2_BUFFER)?_STEP1_BUFFER:(_FA_WORKING_BUFFER_SIZE+_STEP2_BUFFER)  )


typedef enum DRVFaceBeautyObject_s {
    DRV_FACEBEAUTY_OBJ_NONE = 0,
    DRV_FACEBEAUTY_OBJ_SW,
    DRV_FACEBEAUTY_OBJ_SW_NEON,
    DRV_FACEBEAUTY_OBJ_SW2,
    DRV_FACEBEAUTY_OBJ_UNKNOWN = 0xFF,
} DrvFaceBeautyObject_e;



/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
	MTKFACEBEAUTY_FEATURE_BEGIN,              
	MTKFACEBEAUTY_FEATURE_SET_ENV_INFO,  
    MTKFACEBEAUTY_FEATURE_GET_PROC_INFO,
    MTKFACEBEAUTY_FEATURE_SET_PROC_INFO,      
    MTKFACEBEAUTY_FEATURE_GET_ENV_INFO,       
    MTKFACEBEAUTY_FEATURE_GET_RESULT,         
    MTKFACEBEAUTY_FEATURE_GET_LOG,            
    MTKFACEBEAUTY_FEATURE_MAX                 
}	MTKFACEBEAUTY_FEATURE_ENUM;


typedef enum
{
    MTKFACEBEAUTY_CTRL_STEP1,     
	MTKFACEBEAUTY_CTRL_STEP2,     
    MTKFACEBEAUTY_CTRL_STEP3,     
    MTKFACEBEAUTY_CTRL_STEP4,     
    MTKFACEBEAUTY_CTRL_STEP5,     
    MTKFACEBEAUTY_CTRL_STEP6,     
    MTKFACEBEAUTY_CTRL_MAX        
} MTKFACEBEAUTY_CTRL_ENUM;    

typedef enum
{
    MTKFACEBEAUTY_IMAGE_YUV422,              
    MTKFACEBEAUTY_IMAGE_MAX
} MTKFACEBEAUTY_IMAGE_FORMAT_ENUM;

struct MTKFaceBeautyTuningPara
{
    MINT32 SmoothLevel;                             
    MINT32 ContrastLevel ;                         
    MINT32 BrightLevel ;                          
    MINT32 RuddyLevel ;                     
    MINT32 WarpLevel ;                   
    MINT32 WarpFaceNum;                     
	MINT32 MinFaceRatio;
    MINT32 AlignTH1;                     
	MINT32 AlignTH2;
};

struct MTKFaceBeautyEnvInfo
{
    MUINT16  Step2SrcImgWidth;                       
    MUINT16  Step2SrcImgHeight;                      
    MUINT16  Step1SrcImgWidth;                       
    MUINT16  Step1SrcImgHeight;                      
    MUINT16  SrcImgWidth;                            
    MUINT16  SrcImgHeight;                           
    MUINT16  FDWidth;                                
    MUINT16  FDHeight;                               
    MTKFACEBEAUTY_IMAGE_FORMAT_ENUM SrcImgFormat;    
    MBOOL    STEP1_ENABLE;                           
    MUINT32  WorkingBufAddr;                         
    MUINT32  WorkingBufSize;                         
    MTKFaceBeautyTuningPara *pTuningPara;            
};

struct MTKFaceBeautyProcInfo
{
    MUINT16  Step2SrcImgWidth;                       
    MUINT16  Step2SrcImgHeight;                      
    MUINT16  Step1SrcImgWidth;                       
    MUINT16  Step1SrcImgHeight;                      
    MUINT16  SrcImgWidth;                            
    MUINT16  SrcImgHeight;

    MTKFACEBEAUTY_CTRL_ENUM FaceBeautyCtrlEnum;    
    MUINT8* Step1SrcImgAddr;                       
    MUINT8* Step2SrcImgAddr;                       
    MUINT8* SrcImgAddr;                            
    MUINT8* Step4SrcImgAddr_1;                     
    MINT32 FDLeftTopPointX1[15];                   
    MINT32 FDLeftTopPointY1[15];                   
    MINT32 FDBoxSize[15];		                   
    MINT32 FDPose[15];                             
    MINT32  FaceCount;                             
    MUINT8*  Step4SrcImgAddr_2;                    
    MUINT8*  Step4ResultAddr;                      
    MUINT8*  Step5SrcImgAddr;                      
    MUINT8*  Step5SrcResultAddr;                   
    MUINT8*  Step6TempAddr;                        
    MUINT8*  Step6ResultAddr;                      
};

struct MTKFaceBeautyGetProcInfo
{
    MUINT32 WorkingBufferSize;
};

struct MTKFaceBeautyResultInfo
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

class MTKFaceBeauty {
public:
    static MTKFaceBeauty* createInstance(DrvFaceBeautyObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKFaceBeauty(){}
    // Process Control
    virtual MRESULT FaceBeautyInit(void *InitInData, void *InitOutData);	
    virtual MRESULT FaceBeautyMain(void);					
    virtual MRESULT FaceBeautyExit(void);					

	// Feature Control        
	virtual MRESULT FaceBeautyFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppFaceBeautyTmp : public MTKFaceBeauty {
public:
    //
    static MTKFaceBeauty* getInstance();
    virtual void destroyInstance();
    //
    AppFaceBeautyTmp() {}; 
    virtual ~AppFaceBeautyTmp() {};
};

#endif
