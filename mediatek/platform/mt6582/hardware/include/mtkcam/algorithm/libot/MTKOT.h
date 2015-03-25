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

#ifndef _MTK_OT_H
#define _MTK_OT_H

#include <mtkcam/algorithm/libot/MTKOTType.h>
#include <mtkcam/algorithm/libot/MTKOTErrCode.h>

#define MAXOTNUM 3

typedef enum DRVObjTrackObject_s {
    DRV_OT_OBJ_NONE = 0,
    DRV_OT_OBJ_SW_SINGLE,
    DRV_OT_OBJ_SW_MULTI,
    DRV_OT_OBJ_HW,
    DRV_OT_OBJ_UNKNOWN = 0xFF,
} DrvOTObject_e;



typedef enum
{
    MTKOT_FEATURE_BEGIN,              
    MTKOT_FEATURE_SET_ENV_INFO,  
    MTKOT_FEATURE_GET_PROC_INFO,
    MTKOT_FEATURE_SET_PROC_INFO,      
    MTKOT_FEATURE_GET_ENV_INFO,       
    MTKOT_FEATURE_GET_RESULT,
    MTKOT_FEATURE_GET_LOG,            
    MTKOT_FEATURE_MAX                 
}MTKOT_FEATURE_ENUM;

typedef enum
{
    MTKOT_IMAGE_YUV444,
    MTKOT_IMAGE_RGB565,
    MTKOT_IMAGE_MAX
} MTKOT_IMAGE_FORMAT_ENUM;


struct MTKOTTuningPara
{
    //Single OT
    MINT32 IniWinW;                    // default 15 , suggest range: 12~20  , init shape width.
    MINT32 IniWinH;                    // default 15 , suggest range: 12~20  , init shape height.
    MINT32 Incre;                      // default 3  , suggest range: 3~5    , each reshape size.
    MINT32 MaxObjHalfSize;             // default 45 , suggest range: 30~60  , max half w or h of object.
    MINT32 MinObjHalfSize;             // default 5  , suggest range: 5~10   , min half w or h of object.
    MINT32 OnlyReshapeAtBeginning;     // default 0  , suggest range: 0~1    , means only reshape at beginning.
    MINT32 FirstFrameLComputeShape;    // default 6  , suggest range: 6~10   , reshape time length at beginning
    MINT32 LComputePositionI;          // default 4  , suggest range: 4~5    , pos iteration time length at each frame.
    MINT32 LComputeShapeI;             // default 1  , suggest range: 1~2    , reshape time length
    MINT32 LComputeShapeF;             // default 5  , suggest range: 1~10   , reshape by frames
    MFLOAT LtOcObTH;                   // default 0.4, suggest range: 0.3~0.6, tracking threshold, bigger is harder
    MFLOAT ARFA;                       // default 0.0, suggest range: 0.0~1.0, update histogram ratio, bigger is stronger
    MINT32 DisplacementRange;          // default 18 , suggest range: 15~20  , Displacement Range
    MINT32 OBBoundaryMargin;           // default 21 , suggest range: 15~30  , check the object if out of image
    MINT32 OBLoseTrackingFrm;          // default 60 , suggest range: 1~90   , OB occurs, frames to determine lose tracking
    MINT32 OCLoseTrackingFrm;          // default 30 , suggest range: 1~90   , LT or OC occurs, frames to determine lose tracking
    MINT32 TsmthChangeStepSize;        // default 2  , suggest range: 1~4    , step size each reshape for smoothing
    MINT32 TsmthfrmNum;                // default 7  , suggest range: 3~10   , record the history of result for smoothing
    MINT32 LightResistance;            // default 1  , suggest range  0~2    , ability to resist AE change, trade off with error tracking
    //Multi OT
    MINT32 MultiOTThreshold;           // not used yet.
};

struct MTKOTEnvInfo
{
    MINT32   SrcImgWidth;                       
    MINT32   SrcImgHeight;   
    MTKOT_IMAGE_FORMAT_ENUM SrcImgFormat;
    MUINT32  WorkingBufAddr;                         
    MUINT32  WorkingBufSize;                         
    MTKOTTuningPara *pTuningPara;            
};


struct MTKOTProcInfo
{
    // For Working Memory Computation
    MINT32   SrcImgWidth;                
    MINT32   SrcImgHeight;

    //Single OT
    MINT32 InitargetFlag;
    MINT32 InitargetCenterX;
    MINT32 InitargetCenterY;
    MUINT8* SrcImgChannel1;
    MUINT8* SrcImgChannel2;
    MUINT8* SrcImgChannel3;

    //Multi OT
    MINT32 MultiTargetNum;
    MINT32 MultiInitargetX0[MAXOTNUM];
    MINT32 MultiInitargetY0[MAXOTNUM];
    MINT32 MultiInitargetX1[MAXOTNUM];
    MINT32 MultiInitargetY1[MAXOTNUM];
    MUINT16* SrcImgChannel4; 
};

struct MTKOTGetProcInfo
{
    MUINT32 WorkingBufferSize;
};


struct MTKOTResultInfo
{
    MINT32 X0;
    MINT32 Y0;
    MINT32 X1;
    MINT32 Y1;
    MINT32 SmoothX0;
    MINT32 SmoothY0;
    MINT32 SmoothX1;
    MINT32 SmoothY1;
    MINT32 OBCondition;
    MINT32 OCCondition;
    MINT32 LTCondition;
    MINT32 LoseTrackingFlag;

    MINT32 MultiResultNum;
    MINT32 MultiX0[MAXOTNUM];
    MINT32 MultiY0[MAXOTNUM];
    MINT32 MultiX1[MAXOTNUM];
    MINT32 MultiY1[MAXOTNUM];
};

class MTKOT {
public:
    static MTKOT* createInstance(DrvOTObject_e eobject);
    virtual void  destroyInstance() = 0;

    virtual ~MTKOT(){}
    // Process Control
    virtual MRESULT OTInit(void *InitInData, void *InitOutData);	
    virtual MRESULT OTMain(void);					
    virtual MRESULT OTExit(void);					

    // Feature Control        
    virtual MRESULT OTFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

#endif