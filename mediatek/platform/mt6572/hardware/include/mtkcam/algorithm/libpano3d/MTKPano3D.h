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

#ifndef _MTK_PANO3D_H
#define _MTK_PANO3D_H

#include "MTKPano3DType.h"
#include "MTKPano3DErrCode.h"
typedef enum DRVPano3DObject_s {
    DRV_PANO3D_OBJ_NONE = 0,
    DRV_PANO3D_OBJ_SW,
    DRV_PANO3D_OBJ_GPU,    
    DRV_PANO3D_OBJ_UNKNOWN = 0xFF,
} DrvPano3DObject_e;

/*****************************************************************************
    PANO3D Define and State Machine 
******************************************************************************/
#define PANO3D_IM_WIDTH             (800)
#define PANO3D_IM_HEIGHT            (600)
#define PANO3D_CLIP_WIDTH           (1920)
#define PANO3D_CLIP_HEIGHT          (1080)
#define PANO3D_MAX_IMG_NUM          (25)
#define PHOTO_OPT_SWITCH            (1)
#define PANO3D_BUFFER_UNSET         (0xFFFF)

typedef enum
{
    PANO3D_STATE_STANDBY=0, // After Create Obj or Reset
    PANO3D_STATE_INIT,          // After Called Pano3DInit
    PANO3D_STATE_PROC,          // After Called Pano3DMain
    PANO3D_STATE_PROC_READY,    // After Finish Pano3DMain  
} PANO3D_STATE_ENUM;

/*****************************************************************************
    Process Control 
******************************************************************************/
// PANO3DInit, 
// Input    : Pano3DTuningInfo
// Output   : NONE
struct Pano3DTuningInfo
{
    MUINT16                 SmoothThre;
};

typedef enum
{
    PANO3D_IMAGE_YV12=0,          // input image format is YV12
    PANO3D_IMAGE_NV21,            // input image format is NV21
    PANO3D_IMAGE_RGB565,          // input image format is RGB565
    PANO3D_IMAGE_MAX              // maximum image format enum
}   PANO3D_IMAGE_FORMAT;

typedef enum
{
    PANO3D_SMALL_OVLP_WIDTH=0,    // error of small overlap width
    PANO3D_LARGE_SEAM_CHANGE,     // error of large seam change
    PANO3D_ERR_NO
} PANO3D_ERR_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    PANO3D_FEATURE_BEGIN,
    PANO3D_FEATURE_SET_ENV_INFO,
    PANO3D_FEATURE_GET_RESULT,
    PANO3D_FEATURE_GET_LOG,
    PANO3D_FEATURE_GET_WORKBUF_SIZE,
    PANO3D_FEATURE_SET_WORKBUF_ADDR,
    PANO3D_FEATURE_MAX
}   PANO3D_FEATURE_ENUM;

// PANO3D_FEATURE_SET_ENV_INFO, 
// Input    : Pano3DImageInfo
// Output   : NONE
struct Pano3DImageInfo
{
    MUINT32                 ImgAddr;
    MUINT16                 ImgWidth;
    MUINT16                 ImgHeight;
    MUINT32                 ImgNum;
    PANO3D_IMAGE_FORMAT     ImgFmt;
    MUINT32                 WorkingBuffAddr;    
    MFLOAT                  InvHmtx[PANO3D_MAX_IMG_NUM][9];

    // for 3D Panorama
    // Added by CM Cheng, 2011-06-09
    MINT32                  ClipY;                                  // image offset Y - the same for all images
    MUINT16                 ClipHeight;                             // image cropped height - the same for all images
    MINT16                  GridX[PANO3D_MAX_IMG_NUM];              // vertical offset in panorama space
    MINT16                  MinX[PANO3D_MAX_IMG_NUM];               // x_start in panorama space
    MINT16                  MaxX[PANO3D_MAX_IMG_NUM];               // x_end in panorama space

};

// PANO3D_FEATURE_GET_RESULT
// Input    : NONE
// Output   : Pano3DResultInfo
struct Pano3DResultInfo
{
    MUINT16                 PanoWidth;
    MUINT16                 PanoHeight;
    MUINT32                 LeftPanoImageAddr;
    MUINT32                 RightPanoImageAddr;
    MRESULT                 RetCode;
    MUINT8                  ErrPattern;         // Returned error/Warning bit pattern
                                                // bit 0: small overlap width (set if error)
                                                // bit 1: large seam change (set if error)

    // 3D picture cropping info
    MUINT32                 ClipX;              // horizontal offset of ROI
    MUINT32                 ClipY;              // vertical offset of ROI
    MUINT32                 ClipWidth;          // width of ROI
    MUINT32                 ClipHeight;         // height of ROI

    // Optimal Seam - debug purpose
    MINT32                 OptimalSeamLeft[PANO3D_MAX_IMG_NUM][PANO3D_IM_HEIGHT];
};

/*******************************************************************************
*
********************************************************************************/
class MTKPano3D {
public:
    static MTKPano3D* createInstance(DrvPano3DObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKPano3D();
    // Process Control
    virtual MRESULT Pano3DInit(void *InitInData, void *InitOutData);    // Env/Cb setting
    virtual MRESULT Pano3DMain(void);                   // START
    virtual MRESULT Pano3DReset(void);                  // RESET
            
    // Feature Control        
    virtual MRESULT Pano3DFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppPano3DTmp : public MTKPano3D {
public:
    //
    static MTKPano3D* getInstance();
    virtual void destroyInstance();
    //
    AppPano3DTmp() {}; 
    virtual ~AppPano3DTmp() {};
};

#endif

