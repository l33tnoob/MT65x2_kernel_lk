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

#ifndef _MTK_WARP_H
#define _MTK_WARP_H

#include "MTKWarpType.h"
#include "MTKWarpErrCode.h"
#include "MTKCoreType.h"

typedef enum DRVWarpObject_s {
    DRV_WARP_OBJ_NONE = 0,
    DRV_WARP_OBJ_MAV,
    DRV_WARP_OBJ_MAV_SW,
    DRV_WARP_OBJ_PANO,
    DRV_WARP_OBJ_PANO_SW,
    DRV_WARP_OBJ_UNKNOWN = 0xFF,
} DrvWarpObject_e;

//#define TIME_PROF

/*****************************************************************************
    WARP Define and State Machine 
******************************************************************************/
#define WARP_MAX_IMG_NUM            (25)        // maximum image number
#define WARP_BUFFER_UNSET           (0xFFFF)
//#define DEBUG

typedef enum
{
    WARP_STATE_STANDBY=0,   // After Create Obj or Reset
    WARP_STATE_INIT,        // After Called WarpInit
    WARP_STATE_PROC,        // After Called WarpMain
    WARP_STATE_PROC_READY,  // After Finish WarpMain    
} WARP_STATE_ENUM;

/*****************************************************************************
    Process Control 
******************************************************************************/
typedef enum MTK_WARP_IMAGE_ENUM
{
    WARP_IMAGE_RGB565=1,
    WARP_IMAGE_BGR565,
    WARP_IMAGE_RGB888,
    WARP_IMAGE_BGR888,
    WARP_IMAGE_ARGB888,
    WARP_IMAGE_ABGR888,
    WARP_IMAGE_BGRA8888,
    WARP_IMAGE_RGBA8888,
    WARP_IMAGE_YUV444,
    WARP_IMAGE_YUV422,
    WARP_IMAGE_YUV420,
    WARP_IMAGE_YUV411,
    WARP_IMAGE_YUV400,
    WARP_IMAGE_PACKET_UYVY422,
    WARP_IMAGE_PACKET_YUY2,
    WARP_IMAGE_PACKET_YVYU,
    WARP_IMAGE_NV21,
    WARP_IMAGE_YV12,

    WARP_IMAGE_RAW8=100,
    WARP_IMAGE_RAW10,
    WARP_IMAGE_EXT_RAW8,
    WARP_IMAGE_EXT_RAW10,
    WARP_IMAGE_JPEG=200
} MTK_WARP_IMAGE_ENUM;

typedef enum
{   
    MTK_WARP_DIR_RIGHT=0,        // panorama direction is right
    MTK_WARP_DIR_LEFT,           // panorama direction is left
    MTK_WARP_DIR_UP,             // panorama direction is up
    MTK_WARP_DIR_DOWN,           // panorama direction is down
    MTK_WARP_DIR_NO              // panorama direction is not decided
} MTK_WARP_DIRECTION_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    WARP_FEATURE_BEGIN,         // minimum of feature id
    WARP_FEATURE_ADD_IMAGE,     // feature id to add image information
    WARP_FEATURE_GET_RESULT,    // feature id to get result
    WARP_FEATURE_GET_LOG,       // feature id to get debugging information
    WARP_FEATURE_GET_WORKBUF_SIZE,// feature id to query buffer size
    WARP_FEATURE_SET_WORKBUF_ADDR,  // feature id to set working buffer address
    WARP_FEATURE_MAX            // maximum of feature id
}   WARP_FEATURE_ENUM;

// WARP_FEATURE_ADD_IMAGE, 
// Input    : WarpImageInfo
// Output   : NONE
struct WarpImageInfo
{
    MUINT32                 ImgAddr[WARP_MAX_IMG_NUM];  // input image address array
    MUINT32                 ImgNum;                     // input image number
    MTK_WARP_IMAGE_ENUM     ImgFmt;                     // input image format
    MUINT16                 Width;                      // input image width
    MUINT16                 Height;                     // input image height
    MFLOAT                  Hmtx[WARP_MAX_IMG_NUM][9];  // input homography matrix for MAV
    MUINT32                 Flength;                    // input focal length for Autorama
    MTK_WARP_DIRECTION_ENUM Direction;                  // input capture direction for Autorama
    MUINT32                 WorkingBuffAddr;            // Working buffer start address

    MUINT32                 ClipX[WARP_MAX_IMG_NUM];    // image offset X
    MUINT32                 ClipY[WARP_MAX_IMG_NUM];    // image offset Y
    MUINT32                 ClipWidth;                  // image result width
    MUINT32                 ClipHeight;                 // image result height
};

// WARP_FEATURE_GET_RESULT
// Input    : NONE
// Output   : WarpResultInfo
struct WarpResultInfo
{
    MUINT32                 Width;                      // output image width for Autorama
    MUINT32                 Height;                     // output image hieght for Autorama
    MINT32                  ElapseTime[3];              // record execution time
    CORE_ERRCODE_ENUM       RetCode;                    // returned status
};
/*******************************************************************************
*
********************************************************************************/
class MTKWarp {
public:
    static MTKWarp* createInstance(DrvWarpObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKWarp();
    // Process Control
    virtual MRESULT WarpInit(MUINT32 *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT WarpMain(void);                                         // START
    virtual MRESULT WarpReset(void);                                        // RESET
            
    // Feature Control        
    virtual MRESULT WarpFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppWarpTmp : public MTKWarp {
public:
    //
    static MTKWarp* getInstance();
    virtual void destroyInstance();
    //
    AppWarpTmp() {}; 
    virtual ~AppWarpTmp() {};
};

#endif

