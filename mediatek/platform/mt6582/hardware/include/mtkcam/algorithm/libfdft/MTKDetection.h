/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
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

#ifndef _MTK_Detection_H
#define _MTK_Detection_H

//#include <system/camera.h>
//#include "MediaHal.h"

//#include "SWFD_Main.h"
//#include "SWSD_Main.h"

typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef signed int          MINT32;
typedef signed short        MINT16;
typedef signed char         MINT8;

typedef signed int        MBOOL;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif

#define SmileDetect                            (1)

#define MAX_FACE_NUM                  (15)

typedef enum DRVFDObject_s {
    DRV_FD_OBJ_NONE = 0,
    DRV_FD_OBJ_SW,
    DRV_FD_OBJ_HW,
    DRV_FD_OBJ_FDFT_SW,
    DRV_FD_OBJ_UNKNOWN = 0xFF,
} DrvFDObject_e;

typedef enum
{
	FDVT_IDLE_MODE =0,
	FDVT_GFD_MODE = 0x01,	
	FDVT_LFD_MODE = 0x02,
    FDVT_OT_MODE = 0x04,
	FDVT_SD_MODE = 0x08,
} FDVT_OPERATION_MODE_ENUM;

typedef enum
{
	FACEDETECT_TRACKING_REALPOS   = 0,
	FACEDETECT_TRACKING_DISPLAY,
}FACEDETECT_TRACKING_RESULT_TYPE_ENUM;

typedef enum
{
	FACEDETECT_GSENSOR_DIRECTION_0 = 0,
	FACEDETECT_GSENSOR_DIRECTION_90,
    FACEDETECT_GSENSOR_DIRECTION_270,
  FACEDETECT_GSENSOR_DIRECTION_180, 
    FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR,
}FACEDETECT_GSENSOR_DIRECTION;

struct result
{
    bool    af_face_indicator;// face detected flag
    int     face_index;       // priority of this face
    int     type;             // means this face is GFD, LFD, OT face
    int     x0;               // up-left x pos
    int     y0;               // up-left y pos
    int     x1;               // down-right x pos
    int     y1;               // down-right y pos        
    int     fcv;              // confidence value
    int     rip_dir;          // in plane rotate direction
    int     rop_dir;          // out plane rotate direction(0/1/2/3/4/5 = ROP00/ROP+50/ROP-50/ROP+90/ROP-90)
    int     size_index;       // face size index
    int     face_num;         // total face number
 };
 
typedef struct
{
    MINT16   wLeft;
    MINT16   wTop;
    MINT16   wWidth;
    MINT16   wHeight;
} FACEDETECT_RECT;


struct MTKFDFTInitInfo
{
    MUINT32  WorkingBufAddr;                   // working buffer
    MUINT32  WorkingBufSize;                   // working buffer size
    MUINT32  FDThreadNum;                      // default 1, suggest range: 1~2
    MUINT32  FDThreshold;                      // default 32, suggest range: 29~35 bigger is harder
    MUINT32  MajorFaceDecision;                // default 0: Size fist.  1: Center first.   2 Both
    MUINT32  OTRatio;                          // default 960, suggest range: 640~1200
    MUINT32  SmoothLevel;                      // default 1, suggest range: 1~15
    MUINT32  FDSkipStep;                       // default 4, suggest range: 2~6
    MUINT32  FDRectify;                        // default 10000000 means disable and 0 means disable as well. suggest range: 5~10
    MUINT32  FDRefresh;                        // default 70, suggest range: 30~120
    MUINT32  FDBufWidth;                       // preview width
    MUINT32  FDBufHeight;                      // preview height
    MUINT32  SDThreshold;                      // default 32, suggest range: 29~38 bigger is harder
    MUINT32  SDMainFaceMust;                   // default 1 , only capture when main face is smiling
    MUINT32  SDMaxSmileNum;                    // default 3, max faces applied smile detection
    MUINT32  GSensor;                          // default 1, means g-sensor is on the phone
};


/*******************************************************************************
*
********************************************************************************/
class MTKDetection {
public:
    static MTKDetection* createInstance(DrvFDObject_e eobject);
    virtual void      destroyInstance() = 0;
       
    virtual ~MTKDetection() {}
    virtual void FDVTInit(MUINT32 *fd_tuning_data);
    virtual void FDVTInit(MTKFDFTInitInfo *init_data);
    virtual void FDVTMain(MUINT32 ImageScaleBuffer, MUINT32 ImageBufferRGB565,  FDVT_OPERATION_MODE_ENUM fd_state, FACEDETECT_GSENSOR_DIRECTION direction, int gfd_fast_mode);
    virtual void FDVTReset(void);
    virtual MUINT32 FDVTGetResultSize(void);
    virtual MUINT8 FDVTGetResult(MUINT32 FD_result_Adr, FACEDETECT_TRACKING_RESULT_TYPE_ENUM result_type);
    virtual void FDVTGetICSResult(MUINT32 FD_ICS_Result, MUINT32 FD_Results, MUINT32 Width,MUINT32 Heigh, MUINT32 LCM, MUINT32 Sensor, MUINT32 Camera_TYPE, MUINT32 Draw_TYPE);
    virtual void FDVTGetFDInfo(MUINT32  FD_Info_Result);
    virtual void FDVTDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    #ifdef SmileDetect
    virtual void FDVTSDDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    virtual MUINT8 FDVTGetSDResult(MUINT32 FD_result_Adr);
    virtual void FDVTGetMode(FDVT_OPERATION_MODE_ENUM *mode);
    #endif
private:
    
};

class AppFDTmp : public MTKDetection {
public:
    //
    static MTKDetection* getInstance();
    virtual void destroyInstance();
    //
    AppFDTmp() {}; 
    virtual ~AppFDTmp() {};
};

#endif

