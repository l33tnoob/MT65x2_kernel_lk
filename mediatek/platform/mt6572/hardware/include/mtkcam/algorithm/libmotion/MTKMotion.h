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

#ifndef _MTK_MOTION_H
#define _MTK_MOTION_H

#include "MTKMotionType.h"
#include "MTKMotionErrCode.h"
typedef enum DRVMotionObject_s {
    DRV_MOTION_OBJ_NONE = 0,
    DRV_MOTION_OBJ_PANO,                // Motion of Auto Panorama
    DRV_MOTION_OBJ_PANO_NEON,           // Motion of Auto Panorama (neon version)
    DRV_MOTION_OBJ_MAV,                 // Motion of Multi-angle View
    DRV_MOTION_OBJ_MAV_NEON,            // Motion of Multi-angle View (neon version)
    DRV_MOTION_OBJ_3D_PANO,                // Motion of Auto Panorama
    DRV_MOTION_OBJ_UNKNOWN = 0xFF,
} DrvMotionObject_e;

/*****************************************************************************
    MOTION Define and State Machine 
******************************************************************************/
#define MOTION_IM_WIDTH                 (320)                       // input image width
#define MOTION_IM_HEIGHT                (240)                       // input image height
#define MOTION_TOTAL_BN                 (16)                        // EIS bin number
#define MOTION_WORKING_BUFFER_SIZE      (MOTION_IM_WIDTH*MOTION_IM_HEIGHT*3)      // working buffer size (maximum of MAV and PANO)
#define MOTION_BUFFER_UNSET             (0xFFFF)
typedef enum
{
    MOTION_STATE_STANDBY=0,	    // After Create Obj or Exit
    MOTION_STATE_INIT,			// After Called MotionInit
    MOTION_STATE_PROC,			// After Called MotionMain
    MOTION_STATE_PROC_READY,	// After Finish MotionMain	
} MOTION_STATE_ENUM;

/*****************************************************************************
    Process Control 
******************************************************************************/
typedef enum
{   
    MTKMOTION_DIR_RIGHT=0,
    MTKMOTION_DIR_LEFT,
    MTKMOTION_DIR_UP,
    MTKMOTION_DIR_DOWN,
    MTKMOTION_DIR_NO
} MTKMOTION_DIRECTION_ENUM;

struct MTKMotionTuningPara
{
    MINT32  MaxMV;                      // maximum motion vertor        (default: 8)
    MINT32  StepLB;                     // upper bound of frame step    (default: 3)
    MINT32  StepUB;                     // lower bound of frame step    (default: 8)
    MINT32  ToleranceX;                 // horizontal tolerance of motion vector deviation
    MINT32  ToleranceY;                 // vertical tolerance of motion vector deviation
    MINT32  OverlapRatio;               // overlap ratio between captured images
};

typedef enum
{   
    MTKMOTION_ERR_LOW_TV=0,             // warning of low trust value
    MTKMOTION_ERR_LARGE_MV,             // warning of large motion vector
    MTKMOTION_ERR_VERT_SHAKE,           // warning of vertical shaking
    MTKMOTION_ERR_NO
} MTKMOTION_ERR_ENUM;

/* switch for simulation */
//#define SIM_MAIN

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    MTKMOTION_FEATURE_BEGIN,            // minimum of feature id
    MTKMOTION_FEATURE_SET_ENV_INFO,     // feature id to setup environmaent information
    MTKMOTION_FEATURE_SET_PROC_INFO,    // feature id to setup processing information
    MTKMOTION_FEATURE_GET_ENV_INFO,     // feature id to retrieve environment information
    MTKMOTION_FEATURE_GET_RESULT,       // feature id to get result
    MTKMOTION_FEATURE_GET_WORKBUF_SIZE, // feature id to get working buffer size
    MTKMOTION_FEATURE_SET_WORKBUF_ADDR, // feature id to set working buffer address
    MTKMOTION_FEATURE_GET_LOG,          // feature id to get debugging log
    MTKMOTION_FEATURE_MAX               // maximum of feature id

}MTKMOTION_FEATURE_ENUM;

// MTKMOTION_FEATURE_SET_ENV_INFO, 
// Input	: MTKMotionEnvInfo
// Output	: NONE
struct MTKMotionEnvInfo
{
    MUINT32 SrcImgWidth;                // source image width
    MUINT32 SrcImgHeight;               // source image height
    MUINT32 WorkingBuffAddr;            // working buffer address
    MUINT32 WorkingBuffSize;            // working buffer size
    MTKMotionTuningPara *pTuningPara;     // tuning parameters
};

// MTKMOTION_FEATURE_SET_PROC_INFO, 
// Input	: MTKMotionProcInfo
// Output	: NONE
struct MTKMotionProcInfo
{
    MUINT32 ImgAddr;                    // input image address
    MFLOAT	MotionValueXY[MOTION_TOTAL_BN*2];   //From EIS
    MFLOAT	TrustValueXY[MOTION_TOTAL_BN*2];    //From EIS
};


// MTKMOTION_FEATURE_GET_RESULT
// Input	: NONE
// Output	: MTKMotionResultInfo
struct MTKMotionResultInfo
{
    /* motion tracking results */
    MINT16      MV_X;                   // horizontal accumulated motion
    MINT16      MV_Y;                   // vertical accumulated motion
    MBOOL       ReadyToShot;            // ready to shot flag
    MRESULT     RetCode;                // returned code of state machine
    MUINT8      ErrPattern;             // returned error/warning bit pattern 
                                        // bit 0: low trust value (set if warning)
                                        // bit 1: large motion vector (set if warning)
                                        // bit 2: vertical shake (set if warning)
    MTKMOTION_DIRECTION_ENUM Direction; // panorama direction enum
};
/*******************************************************************************
*
********************************************************************************/
class MTKMotion {
public:
    static MTKMotion* createInstance(DrvMotionObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKMotion(){}
    // Process Control
    virtual MRESULT MotionInit(void* InitInData, void* InitOutData);	// Env/Cb setting
    virtual MRESULT MotionMain(void);					                    // START
    virtual MRESULT MotionExit(void);					                    // EXIT
            
    // Feature Control        
    virtual MRESULT MotionFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppMotionTmp : public MTKMotion {
public:
    //
    static MTKMotion* getInstance();
    virtual void destroyInstance();
    //
    AppMotionTmp() {}; 
    virtual ~AppMotionTmp() {};
};

#endif

