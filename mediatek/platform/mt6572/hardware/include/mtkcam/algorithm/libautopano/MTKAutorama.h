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
 

#ifndef _MTK_AUTORAMA_H
#define _MTK_AUTORAMA_H

#include "MTKAutoramaType.h"
#include "MTKAutoramaErrCode.h"

typedef enum DRVPanoObject_s {
    DRV_AUTORAMA_OBJ_NONE = 0,
    DRV_AUTORAMA_OBJ_SW,
    DRV_AUTORAMA_OBJ_SW_NEON,    
    DRV_AUTORAMA_OBJ_UNKNOWN = 0xFF,
} DrvAutoramaObject_e;

typedef enum
{
	MTKAUTORAMA_STATE_STANDBY=0,	    // After Create Obj or Reset
	MTKAUTORAMA_STATE_INIT,			    // After Called PanoInit
	MTKAUTORAMA_STATE_ADD_IMG,			// After Called PanoMain with MTKAUTORAMA_CTRL_ENUM == MTKAUTORAMA_CTRL_ADD_IMAGE
	MTKAUTORAMA_STATE_ADD_IMG_READY,	// After Finish PanoMain with MTKAUTORAMA_CTRL_ENUM == MTKAUTORAMA_CTRL_ADD_IMAGE	
	MTKAUTORAMA_STATE_STITCH,		    // After Called PanoMain with MTKAUTORAMA_CTRL_ENUM == MTKAUTORAMA_CTRL_STITCH	
	MTKAUTORAMA_STATE_STITCH_READY      // After Finish PanoMain with MTKAUTORAMA_CTRL_ENUM == MTKAUTORAMA_CTRL_STITCH	
} MTKAUTORAMA_STATE_ENUM;


/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

typedef enum
{
	MTKAUTORAMA_FEATURE_BEGIN,              // minimum of feature id
	MTKAUTORAMA_FEATURE_SET_ENV_INFO,       // feature id to setup environment information
    MTKAUTORAMA_FEATURE_SET_PROC_INFO,      // feature id to setup processing information
    MTKAUTORAMA_FEATURE_GET_ENV_INFO,       // feature id to retrieve environment information
    MTKAUTORAMA_FEATURE_GET_RESULT,         // feature id to get result
    MTKAUTORAMA_FEATURE_GET_LOG,          // feature id to get debugging log
    MTKAUTORAMA_FEATURE_MAX                 // maximum of feature id
}	MTKAUTORAMA_FEATURE_ENUM;

typedef enum
{   
    MTKAUTORAMA_DIR_RIGHT=0,                // panorama direction is right
    MTKAUTORAMA_DIR_LEFT,                   // panorama direction is left
    MTKAUTORAMA_DIR_UP,                     // panorama direction is up
    MTKAUTORAMA_DIR_DOWN,                   // panorama direction is down
    MTKAUTORAMA_DIR_NO                      // panorama direction is not decided
} MTKAUTORAMA_DIRECTION_ENUM;

typedef enum
{
    MTKAUTORAMA_CTRL_ADD_IMAGE,             // processing add image
    MTKAUTORAMA_CTRL_STITCH,                // processing stitch images
    MTKAUTORAMA_CTRL_MAX
} MTKAUTORAMA_CTRL_ENUM;

typedef enum
{
    MTKAUTORAMA_IMAGE_YV12,             // processing add image
    MTKAUTORAMA_IMAGE_NV21,                // processing stitch images
    MTKAUTORAMA_IMAGE_MAX
} MTKAUTORAMA_IMAGE_FORMAT_ENUM;

// MTKAUTORAMA_FEATURE_SET_ENV_INFO, 
// Input	: MTKPanoEnvInfo
// Output	: NONE
struct MTKAutoramaEnvInfo
{
    MUINT16  SrcImgWidth;                      // input image width
    MUINT16  SrcImgHeight;                     // input image height
    MUINT16  MaxPanoImgWidth;                  // final output image max width
    MUINT8   MaxSnapshotNumber;                // max capture image num
    MUINT32  WorkingBufAddr;                   // working buffer
    MUINT32  WorkingBufSize;                   // working buffer size
    MBOOL    FixAE;                            // fixAe or not, if false, system should provide the ev information
    MUINT32  FocalLength;                      // lens focal length depends on lens, normally, this value can be set 750
    MBOOL    GPUWarp;                          // enable gpu cylindrical projection or not
    MTKAUTORAMA_IMAGE_FORMAT_ENUM SrcImgFormat;// source image format
};
// MTKPANO_FEATURE_SET_PROC_INFO, 
// Input	: MTKPanoProcInfo
// Output	: NONE
struct MTKAutoramaProcInfo
{
    MTKAUTORAMA_CTRL_ENUM AutoramaCtrlEnum;  // decide the current action is MTKPANO_CTRL_ADD_IMAGE or MTKPANO_CTRL_STITCH
    MUINT32  SrcImgAddr;                     // input image address. currently only support YUV420
    MINT32   EV;                             // ev of the source image, if Fix AE, this value can be set zero
    MTKAUTORAMA_DIRECTION_ENUM StitchDirection;// image stitch direction
};

// MTKPANO_FEATURE_GET_RESULT
// Input	: NONE
// Output	: MTKPanoResultInfo
struct MTKAutoramaResultInfo
{
    MUINT32  ImgBufferAddr;    // output image address, currently, only output YUV420
    MUINT16  ImgWidth;         // output image width
    MUINT16  ImgHeight;        // output image height
};

// MTKAUTORAMA_FEATURE_GET_ENV_INFO
// Input	: image number
// Output	: MTKAutoramaGetEnvInfo
struct MTKAutoramaGetEnvInfo
{
    MUINT8   MaxSnapshotNumber;     // max capture image num
    MUINT16  ImgWidth;				// output image width
    MUINT16  ImgHeight;				// output image height
    MUINT32  WorkingBuffSize;    // calculte by image number
};

class MTKAutorama {
public:
    static MTKAutorama* createInstance(DrvAutoramaObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKAutorama(){}
    // Process Control
    virtual MRESULT AutoramaInit(void *InitInData, void *InitOutData);	
    virtual MRESULT AutoramaMain(void);					
    virtual MRESULT AutoramaExit(void);					

	// Feature Control        
	virtual MRESULT AutoramaFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppAutoramaTmp : public MTKAutorama {
public:
    //
    static MTKAutorama* getInstance();
    virtual void destroyInstance();
    //
    AppAutoramaTmp() {}; 
    virtual ~AppAutoramaTmp() {};
};

#endif
