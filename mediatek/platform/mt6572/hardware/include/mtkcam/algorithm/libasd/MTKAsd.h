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

#ifndef _MTK_ASD_H
#define _MTK_ASD_H

#include "MTKAsdType.h"
#include "MTKAsdErrCode.h"

//#define	SIM_MAIN

typedef enum DRVAsdObject_s {
    DRV_ASD_OBJ_NONE = 0,
    DRV_ASD_OBJ_SW,
    DRV_ASD_OBJ_SW_NEON,    
    DRV_ASD_OBJ_UNKNOWN = 0xFF,
} DrvAsdObject_e;

/*****************************************************************************
	ASD Defines and State Machine / Proc Mode
******************************************************************************/
#define ASD_NUM_OF_SCENE  		(2)					// Will be fixed in final shipment
#define ASD_IM_WIDTH    		(160)				// default max image width
#define ASD_IM_HEIGHT   		(120)				// default max image height
#define ASD_IMAGE_SIZE 			(ASD_IM_WIDTH*ASD_IM_HEIGHT*2)	// Image buffer size
#define ASD_WORKING_BUFFER_SIZE (ASD_IMAGE_SIZE*9)	// working buffer size W*H*2*9
#define ASD_DECIDER_LOG_HEADER 512
#define ASD_DECIDER_LOG_BYTE_PER_CYCLE 256
#define ASD_DECIDER_LOG_MAX_CYCLE 600
#define ASD_DECIDER_BUFFER_SIZE (((ASD_DECIDER_LOG_BYTE_PER_CYCLE*ASD_DECIDER_LOG_MAX_CYCLE+ASD_DECIDER_LOG_HEADER+ASD_IMAGE_SIZE+31)>>5)<<5)
#define ASD_BUFFER_SIZE 	 (((ASD_IMAGE_SIZE+ASD_WORKING_BUFFER_SIZE+ASD_DECIDER_BUFFER_SIZE+31)>>5)<<5)

typedef enum ASD_PROC {
    ASD_PROC_DECIDER = 0,
    ASD_PROC_MAIN,
	ASD_PROC_UNKNOWN 
} ASD_PROC_ENUM;

typedef enum
{
	ASD_STATE_IDLE=0,		
	ASD_STATE_STANDBY,		// After Create Obj or Reset
	ASD_STATE_INIT,			// After Called AsdInit
	ASD_STATE_PROC,			// After Called AsdMain
} ASD_STATE_ENUM;

/*****************************************************************************
	ASD Main Module
******************************************************************************/
/*	ASD Main Process Structure */

typedef	struct
{
	MUINT8 	num_of_ort;						/* 0 - 4 */    
} ASD_SCD_TUNING_PARA_STRUCT, *P_ASD_SCD_TUNING_PARA_STRUCT;

typedef struct
{
	MUINT16 image_width;
	MUINT16 image_height;
	MUINT32 ext_mem_start_addr; //working buffer start address
	MUINT32 ext_mem_size;
	MUINT32 ImgFmtList;
	ASD_SCD_TUNING_PARA_STRUCT asd_tuning_data;
} ASD_SCD_SET_ENV_INFO_STRUCT, *P_ASD_SCD_SET_ENV_INFO_STRUCT;

/* union information data for decider */
typedef struct
{
	MUINT32 src_buffer_addr;	
} ASD_SCD_INFO_STRUCT;

typedef struct
{	/* ASD results */
	MUINT32 asd_start_time;
	MUINT32 asd_end_time;	
	MUINT8 asd_score[ASD_NUM_OF_SCENE]; // 0: landscape, 1: backlit
}ASD_SCD_RESULT_STRUCT,*P_ASD_SCD_RESULT_STRUCT;

/*****************************************************************************
	Decider Sub-Module
******************************************************************************/
/* input source type enum */
typedef enum
{
	ASD_DECIDER_INFO_SRC_YUV,    /* from YUV sensor */
	ASD_DECIDER_INFO_SRC_FP_RAW, /* from WCP1 FP */
	ASD_DECIDER_INFO_SRC_SP_RAW, /* from WCP2 SP */
	ASD_DECIDER_INFO_SRC_NUM
}	ASD_DECIDER_INFO_SRC_ENUM;

/* all possible scene types enum, only 7 scenes are available for this MP */
typedef enum
{
    ASD_DECIDER_UI_AUTO=0,
    ASD_DECIDER_UI_N   =1,
    ASD_DECIDER_UI_B   =2,
    ASD_DECIDER_UI_P   =3,
    ASD_DECIDER_UI_L   =4,
    ASD_DECIDER_UI_NB  =5,
    ASD_DECIDER_UI_NP  =6,
    ASD_DECIDER_UI_NL  =7,
    ASD_DECIDER_UI_BP  =8,
    ASD_DECIDER_UI_BL  =9,
    ASD_DECIDER_UI_PL  =10,
    ASD_DECIDER_UI_NBP =11,
    ASD_DECIDER_UI_NBL =12,
    ASD_DECIDER_UI_NPL =13,
    ASD_DECIDER_UI_BPL =14,
    ASD_DECIDER_UI_NBPL=15,
    ASD_DECIDER_UI_SCENE_NUM
} ASD_DECIDER_UI_SCENE_TYPE_ENUM;

/* tuning param, time smooth weight vector customized options enum, time smooth range */
typedef enum
{
    ASD_TIME_WEIGHT_RANGE_1CYCLE=1,
	ASD_TIME_WEIGHT_RANGE_2CYCLE=2,
	ASD_TIME_WEIGHT_RANGE_3CYCLE=3,
	ASD_TIME_WEIGHT_RANGE_4CYCLE=4,
	ASD_TIME_WEIGHT_RANGE_5CYCLE=5,
	ASD_TIME_WEIGHT_RANGE_6CYCLE=6,
	ASD_TIME_WEIGHT_RANGE_7CYCLE=7,
	ASD_TIME_WEIGHT_RANGE_8CYCLE=8,
	ASD_TIME_WEIGHT_RANGE_9CYCLE=9,
	ASD_TIME_WEIGHT_RANGE_10CYCLE=10,
    ASD_TIME_WEIGHT_RANGE_CYCLE_MAX
} ASD_DECIDER_TIME_WEIGHT_RANGE_ENUM;

/* tuning param, time smooth weight vector customized options enum, time smooth type */
typedef enum
{
    ASD_TIME_WEIGHT_AVERAGE=0,     /* the same weight on each cycle results */
	ASD_TIME_WEIGHT_LATER_HIGHER,  /* higher weight on later cycle results */
    ASD_TIME_WEIGHT_IDX_WEIGHT_TYPE_NUM
} ASD_DECIDER_TIME_WEIGHT_TYPE_ENUM;

/* 3A information for decider usage */
typedef struct
{
    MINT16   AeEv;        /* AE Ev value */
    MINT16  AeFaceEnhanceEv;        /* Face AE enhance Ev value */    
    MBOOL   AeIsBacklit; /* AE Backlit Condition from AE algorithm */
    MBOOL   AeIsStable;  /* AE converge to stable situation */
    MUINT16 AwbCurRgain; /* Current AWB R channel gain */
    MUINT16 AwbCurBgain; /* Current AWB B channel gain */
    MBOOL   AwbIsStable; /* AWB converge to stable situation */
    MUINT16 AfPosition;  /* Current lens position */     
    MBOOL   AfIsStable;  /* Af converge to stable situation */
} ASD_DECIDER_INFO_3A_STRUCT;

/* FD information for decider usage */
typedef struct
{
    MUINT8  FdFaceNum;
    MUINT8  FdMainFacePose;
    MUINT16 FdMainFaceLuma;
    MUINT16 FdMainFaceX0;
    MUINT16 FdMainFaceX1;
    MUINT16 FdMainFaceY0;
    MUINT16 FdMainFaceY1;
    MUINT32 FdProcTimeStart;
    MUINT32 FdProcTimeEnd;
} ASD_DECIDER_INFO_FD_STRUCT;

/* Content scene detection information for decider usage */
typedef struct
{
    MUINT32 ScdProcTimeStart;
    MUINT32 ScdProcTimeEnd;
    MUINT8  ScdScoreLs;
    MUINT8  ScdScoreBl;    
} ASD_DECIDER_INFO_SCD_STRUCT;

/* union information data for decider */
typedef struct
{
    ASD_DECIDER_INFO_3A_STRUCT  InfoAaa; /* 3A information for ASD usage */
    ASD_DECIDER_INFO_FD_STRUCT  InfoFd;  /* FD information for ASD usage */
    ASD_DECIDER_INFO_SCD_STRUCT InfoScd; /* Scene Detection information for ASD usage */
} ASD_DECIDER_INFO_STRUCT;

/* Decider Process Structure */
/* decider final result scene */
typedef struct
{
    MBOOL DataScdValid; /* TRUE if content scene detection's data are updated in this cycle */
    MBOOL DataFdValid;  /* TRUE if fd data are updated in this cycle */
    ASD_DECIDER_UI_SCENE_TYPE_ENUM DeciderUiScene;
} ASD_DECIDER_RESULT_STRUCT;

/* reference parameters for decider usage, it's varied by projects */
typedef struct
{
    ASD_DECIDER_INFO_SRC_ENUM DeciderInfoVer;  /* information version control, 0,1,2 for YUV,WCP1-RAW, WCP2-RAW*/    
    MUINT16 RefAwbD65Rgain;  /* D65 R channel gain */
    MUINT16 RefAwbD65Bgain;  /* D65 B channel gain */
    MUINT16 RefAwbCwfRgain;  /* CWF R channel gain */
    MUINT16 RefAwbCwfBgain;  /* CWF B channel gain */
    void       *RefAfTbl;       /* the whole AF table including macro index & total number of the table */ 	
} ASD_DECIDER_REF_STRUCT;

/* tuning parameters */
typedef struct
{
    ASD_DECIDER_TIME_WEIGHT_TYPE_ENUM  TimeWeightType;
	ASD_DECIDER_TIME_WEIGHT_RANGE_ENUM TimeWeightRange;
	MINT16 IdxWeightBlAe;
	MINT16 IdxWeightBlScd;    
	MINT16 IdxWeightLsAe;        
  	MINT16 IdxWeightLsAwb;
  	MINT16 IdxWeightLsAf;    
    MINT16 IdxWeightLsScd;
    MINT16 EvLoThrNight;
    MINT16 EvHiThrNight;
    MINT16 EvLoThrOutdoor;
    MINT16 EvHiThrOutdoor;
    MINT16 BacklitLockEvDiff;  
    MUINT8 ScoreThrNight;
    MUINT8 ScoreThrBacklit;
    MUINT8 ScoreThrPortrait;
    MUINT8 ScoreThrLandscape;
    MBOOL BacklitLockEnable;
    ASD_DECIDER_UI_SCENE_TYPE_ENUM UiSceneLut[ASD_DECIDER_UI_SCENE_NUM];
    MBOOL  bReserved;
    void   *pReserved;
} ASD_DECIDER_TUNING_PARA_STRUCT;

typedef struct 
{
    ASD_DECIDER_REF_STRUCT			DeciderRefData;
    ASD_DECIDER_TUNING_PARA_STRUCT	DeciderTuneData;    // tuning parameters    
} ASD_DECIDER_INIT_STRUCT;

/*****************************************************************************
	A.S.D.
******************************************************************************/
/* feature control enum */
typedef enum
{
	ASD_FEATURE_BEGIN,
	ASD_FEATURE_GET_RESULT,
	ASD_FEATURE_GET_STATUS,
	ASD_FEATURE_GET_LOG,
	
	DECIDER_FEATURE_GET_RESULT,
	DECIDER_FEATURE_SET_LOG_INFO, /* Decider feature control to set settings for debug log */
	DECIDER_FEATURE_GET_LOG_INFO,

	ASD_FEATURE_SAVE_DECIDER_LOG_INFO,
		
	ASD_FEATURE_MAX
}	ASD_FEATURE_ENUM;

struct ASD_INIT_INFO
{
    ASD_SCD_SET_ENV_INFO_STRUCT			*pInfo;					
    ASD_DECIDER_REF_STRUCT				*pDeciderInfo;
    ASD_DECIDER_TUNING_PARA_STRUCT		*pDeciderTuningInfo;    // tuning parameters    
};

/*******************************************************************************
*
********************************************************************************/
class MTKAsd {
public:
    static MTKAsd* createInstance(DrvAsdObject_e eobject);
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKAsd(){}
    // Process Control
    virtual MRESULT AsdInit(void* InitInData, void* InitOutData);	// Env/Cb setting
    virtual MRESULT AsdMain(ASD_PROC_ENUM ProcId, void* pParaIn);	// START
            
	// Feature Control        
	virtual MRESULT AsdFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};

class AppAsdTmp : public MTKAsd {
public:
    //
    static MTKAsd* getInstance();
    virtual void destroyInstance();
    //
    AppAsdTmp() {}; 
    virtual ~AppAsdTmp() {};
};

#endif

