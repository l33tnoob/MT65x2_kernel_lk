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

/*------------------------------------------------------------------------------

    Table of contents

    1. Type definition for encoder instance
    2. Enumerations for API parameters
    3. Structures for API function parameters
    4. Encoder API function prototypes
    5. Encoder memory management callback functions

------------------------------------------------------------------------------*/

#ifndef __UVT_SWENCAPI_H__
#define __UVT_SWENCAPI_H__

//#include "UVTEncDefines.h"

#include "UVT_performance_measure.h"

#include "UVTEncBasetype.h"

#include "dec_enc_common.h"

#include "kal_non_specific_general_types.h"
#include "MP4SwEncDefs.h"
#ifdef __cplusplus
extern "C"
    {
#endif

/*------------------------------------------------------------------------------
    1. Type definition for encoder instance
------------------------------------------------------------------------------*/

typedef void *UVTSwEncInst;
#if defined(WIN32_PC_SIM) || defined(WIN32) || defined(ARMULATOR)
#include <stdio.h>
#define TARGET_PRINT printf
#define TARGET_PRINT_1 printf
#define TARGET_PRINT_2 printf
#define TARGET_PRINT_3 printf
#define TARGET_PRINT_4 printf
#define TARGET_PRINT_5 printf
#else
#define TARGET_PRINT(x)
#define TARGET_PRINT_1(x,y)
#define TARGET_PRINT_2(x,y,z)
#define TARGET_PRINT_3(x,y,z,a)
#define TARGET_PRINT_4(x,y,z,a,b)
#define TARGET_PRINT_5(x,y,z,a,b,c)
#endif

/*------------------------------------------------------------------------------
    2. Enumerations for API parameters
------------------------------------------------------------------------------*/

/* Function return values */
typedef enum
{
    UVTSWENC_OK = 0,
    UVTSWENC_VOP_READY = 1,
    UVTSWENC_GOV_READY = 2,
    UVTSWENC_VOP_READY_VBV_FAIL = 3,

    UVTSWENC_ERROR = -1,
    UVTSWENC_NULL_ARGUMENT = -2,
    UVTSWENC_INVALID_ARGUMENT = -3,
    UVTSWENC_MEMORY_ERROR = -4,
    UVTSWENC_EWL_ERROR = -5,
    UVTSWENC_EWL_MEMORY_ERROR = -6,
    UVTSWENC_INVALID_STATUS = -7,
    UVTSWENC_OUTPUT_BUFFER_OVERFLOW = -8,
    UVTSWENC_HW_ERROR = -9,
    UVTSWENC_HW_TIMEOUT = -10,
    UVTSWENC_SYSTEM_ERROR = -11,
    UVTSWENC_INSTANCE_ERROR = -12,
    UVTSWENC_EVALUATION_LIMIT_EXCEEDED = -13,
    UVTSWENC_OUTPUT_BUFFER_BLOCKED = -14,
    UVTSWENC_REJECT_INPUT = -15,
    UVTSWENC_OUTPUT_BUFFER_INVALID = -16,
    UVTSWENC_INPUT_FULL = -17,
    UVTSWENC_DEINIT_REJECT = -18,
    UVTSWENC_VT_DROPFRAME = -19
} UVTSwEncRet;

/* Stream type for initialization */
typedef enum
{
    UVTSWENC_MPEG4_PLAIN_STRM = 0,
    UVTSWENC_MPEG4_VP_STRM = 1,
    UVTSWENC_MPEG4_VP_DP_STRM = 2,
    UVTSWENC_MPEG4_VP_DP_RVLC_STRM = 3,
    UVTSWENC_MPEG4_SVH_STRM = 4,
    UVTSWENC_H263_STRM = 5
} UVTSwEncStrmType;

/* Profile and level for initialization */
typedef enum
{
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_0  = 8,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_0B = 9,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_1  = 1,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_2  = 2,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_3  = 3,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_4A = 4,
    UVTSWENC_MPEG4_SIMPLE_PROFILE_LEVEL_5  = 5,
    UVTSWENC_MPEG4_MAIN_PROFILE_LEVEL_2  = 50,
    UVTSWENC_MPEG4_MAIN_PROFILE_LEVEL_3  = 51,
    UVTSWENC_MPEG4_MAIN_PROFILE_LEVEL_4  = 52,
    UVTSWENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_3 = 243,
    UVTSWENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_4 = 244,
    UVTSWENC_MPEG4_ADV_SIMPLE_PROFILE_LEVEL_5 = 245,
    UVTSWENC_H263_PROFILE_0_LEVEL_10 = 1001,
    UVTSWENC_H263_PROFILE_0_LEVEL_20 = 1002,
    UVTSWENC_H263_PROFILE_0_LEVEL_30 = 1003,
    UVTSWENC_H263_PROFILE_0_LEVEL_40 = 1004,
    UVTSWENC_H263_PROFILE_0_LEVEL_50 = 1005,
    UVTSWENC_H263_PROFILE_0_LEVEL_60 = 1006,
    UVTSWENC_H263_PROFILE_0_LEVEL_70 = 1007
} UVTSwEncProfileAndLevel;

/* User data type */
typedef enum
{
    UVTSWENC_VOS_USER_DATA,
    UVTSWENC_VO_USER_DATA,
    UVTSWENC_VOL_USER_DATA,
    UVTSWENC_GOV_USER_DATA
} UVTSwEncUsrDataType;

/* VOP type for encoding */
typedef enum
{
    UVTSWENC_INTRA_VOP = 0,
    UVTSWENC_PREDICTED_VOP = 1,
    UVTSWENC_NOTCODED_VOP = 2              /* Used just as a return type */
} UVTSwEncVopType;

typedef enum
{
  UVTSWENC_SCENARIO_RECORDER = 0,
  UVTSWENC_SCENARIO_VT       = 1,
  UVTSWENC_SCENARIO_MAX
} UVTSwEncScenario;

typedef enum
{
  UVTSWENC_UVT                          = 3
} UVTSwEncAlgorithm;

typedef enum
{
  UVTSWENC_ROTATE_0   = 0,
  UVTSWENC_ROTATE_90  = 1,
  UVTSWENC_ROTATE_180 = 2,
  UVTSWENC_ROTATE_270 = 3
} UVTSwEncInputRotated;

/*------------------------------------------------------------------------------
    3. Structures for API function parameters
------------------------------------------------------------------------------*/

/* Configuration info for initialization */
typedef struct
{
    void  *callback_handle;
    MP4SwEncQualityLevel  qualityLevel;
    UVTSwEncProfileAndLevel profileAndLevel;
    UVTSwEncStrmType strmType;
    i32 width;
    i32 height;
    UVTSwEncInputRotated  inp_rotated;
    i32 frmRateNum;
    i32 frmRateDenom;
    i32 videoRange;
    i32 intraVopRate;
    i32 essStartAddr;
    i32 essSize;
    UVTSwEncAlgorithm algorithm;
    i32 range_reduction;
    i32 is_customer_set_table;
    i16 *dynamic_range_table;
} UVTSwEncCfg;

/* Coding control parameters */
typedef struct
{
    i32 insHEC;
    i32 insGOB;
    i32 insGOV;
    i32 vpSize;
} UVTSwEncCodingCtrl;

/* Rate control parameters */
typedef struct
{
    MP4SwEncQualityLevel  qualityLevel;
    i32 vopRc;	 
    i32 mbRc;
#ifdef VopSkip_Enable	 
    i32 vopSkip;
#endif
    i32 qpHdr;
    i32 qpHdrMin;
    i32 qpHdrMax;
    i32 bitPerSecond;
    i32 vbv;	 
    i32 cir;
    i32 rateHardLimitMultiple;
    i32 rateBalanceInterval;
} UVTSwEncRateCtrl;

typedef enum
{
  NULL_INPUT = 0,
  NORMAL_INPUT
} UVTSwEncInpDescriptor_e;

/* Encoder input structure */
typedef struct
{
    u64 TimeStamp;
    i32 ResetOnVTCall;
    u8  *pLuma;
    u8  *pChromaU;
    u8  *pChromaV;
    UVTSwEncInpDescriptor_e eDes;
    UVTSwEncVopType vopType;
    i32 timeIncr;
} UVTSwEncIn;

/* Time code structure for encoder output */
typedef struct
{
    i32 hours;
    i32 minutes;
    i32 seconds;
    i32 timeIncr;
    i32 timeRes;
} UVTSwEncTimeCode;

/* Encoder output structure */
typedef struct
{
    UVTSwEncTimeCode timeCode;
    UVTSwEncVopType vopType;
    i32 strmSize;
} UVTSwEncOut;

/* Input cropping and camera stabilization parameters */
typedef struct
{
    i32 origWidth;
    i32 origHeight;
    i32 xOffset;
    i32 yOffset;
} UVTSwEncCropCfg;

/* Version information */
typedef struct
{
    i32 major;    /* Encoder API major version */
    i32 minor;    /* Encoder API minor version */
} UVTSwEncApiVersion;

/* Capability information */
typedef struct 
{
    u32 profile;      /* Maximum support profile */
    u32 intMemSize;   /* Required internal memory size */
    u32 extMemSize;   /* Required internal memory size */
} UVTSwEncCapability;

/* Profile *///conflicting with Sw_types.h... so comment by mtk80691
/*
typedef enum
{
    MP4_BASELINE = 0
} UVTSwEncProfile;
*/

typedef enum
{
  ENC_SS_HEADER = 0,
  ENC_SS_LAST_SLICE_OF_PICTURE = 1,
  ENC_SS_PICTURE_BODY = 2,
  ENC_SS_EMPTY = 3,
  ENC_VOP_DROPED = 4,
} UVTSwEncStreamSliceType;

#define MP4_MAX_PACKET_COUNT   16

typedef struct
{
  u64 TimeStamp;
  u8* start;
  u32 length;
  u32 type;
  UVTSwEncTimeCode        timeCode;
  u32         vopType;	 
  u32                     pktNumber;
  vpInfo_t                vpInfo[MP4_MAX_PACKET_COUNT];
} UVTSwEncStreamSlice;

typedef struct
{
   u8* start;
   u8* end;
   u32 read_pointer;
   u32 write_pointer;
   u32 buffer_size;
   i32 is_wraparound_disabled;
} UVTSwEncStreamBufferDescriptor;

typedef enum
{
  YUV_FINISHED = 0
} MP4SwCbEvent;

typedef struct 
{
    u32 vop_time_increment_resolution;
    //u32 width;
    //u32 height;
} UVTSwEncVT_VOS_PAR;

typedef struct
{
  u32  frame_rate;
  u32  width;
  u32  height;
  u32  intra_vop_rate;
  u32  qp_init;
  u32  qp_max;
  u32  qp_min;
  u32  algorithm_select;
  u32  bit_rate;
  u32  dynamic_range_reduction;
  u32  is_customer_set_table;
  i16  *dynamic_range_table;
} UVTSwEncRecordSetting;

/*------------------------------------------------------------------------------
    4. Encoder API function prototypes
------------------------------------------------------------------------------*/

/* Version information */
UVTSwEncApiVersion UVTSwEncGetVersion(void);

/* Initialization & release */
UVTSwEncRet UVTSwEncInit(UVTSwEncCfg * pEncCfg, UVTSwEncInst * instAddr, u8 intraOnly);
UVTSwEncRet UVTSwEncRelease(void *handle, UVTSwEncInst inst);

/* Encoder configuration */
UVTSwEncRet UVTSwEncSetCodingCtrl(UVTSwEncInst inst, UVTSwEncCodingCtrl *
                                pCodeParams);
UVTSwEncRet UVTSwEncGetCodingCtrl(UVTSwEncInst inst, UVTSwEncCodingCtrl *
                                pCodeParams);
UVTSwEncRet UVTSwEncSetRateCtrl(UVTSwEncInst inst, UVTSwEncRateCtrl *pRateCtrl);
UVTSwEncRet UVTSwEncGetRateCtrl(UVTSwEncInst inst, UVTSwEncRateCtrl *pRateCtrl);
UVTSwEncRet UVTSwEncSetUsrData(UVTSwEncInst inst, const u8 * pBuf, i32 length,
                               UVTSwEncUsrDataType type);
/* Stream generation */
UVTSwEncRet UVTSwEncStrmStart(UVTSwEncInst inst, UVTSwEncIn * pEncIn);

UVTSwEncRet UVTSwEncStrmEnd(UVTSwEncInst inst, UVTSwEncIn * pEncIn);

UVTSwEncRet UVTSwEncStrmEncode(UVTSwEncInst inst, UVTSwEncIn * pEncIn);

UVTSwEncRet UVTSwEncGetBitstream(UVTSwEncInst  inst, UVTSwEncStreamSlice *pSlice);


UVTSwEncRet UVTSwEncQueryCapability(UVTSwEncCapability *pEncCap, 
                                    UVTSwEncScenario    scenario,
                                    i32 width,
                                    i32 height);

UVTSwEncRet UVTSwEncVOS (u8 *vos_buffer_ptr, u32 vos_buffer_size, 
                               UVTSwEncVT_VOS_PAR *VOSSetting, u32 *vos_size_encoded);

UVTSwEncRet UVTSwEncQueryRecordSetting(void *handle, UVTSwEncRecordSetting  *pRecSet,
                                       MP4SwEncQualityLevel         QualityLevel);
UVTSwEncRet MP4VTSwEncQueryInfo(void *handle, UVTSwEncRecordSetting *prRecSet);

/* Pre processing: camera stabilization */
#ifdef ClipSrcVideo_Enable
UVTSwEncRet UVTSwEncSetCrop(UVTSwEncInst inst, UVTSwEncCropCfg * pCropCfg);
UVTSwEncRet UVTSwEncGetCrop(UVTSwEncInst inst, UVTSwEncCropCfg * pCropCfg);
#endif
/*------------------------------------------------------------------------------
    5. Encoder callback functions
------------------------------------------------------------------------------*/

void *MP4VTSwEncCalloc(void * handle, u32 n, u32 s);

void MP4VTSwEncFree(void *handle, void *p);

void MP4VTSwEncMemcpy(void *d, const void *s, u32 n);

void UVTSwEncTrace(char *msg);

#ifdef V2_RING_STREAM
void MP4VTSwEncStreamBufferAllocate(void *handle, UVTSwEncStreamBufferDescriptor *sbuf_des);
void MP4VTSwEncStreamBufferUpdate(void *inst, u8 *write_ptr, i32 is_slice_contained);
void MP4VTSwEncPaused(void *handle, u8 *read_ptr);
#else
void MP4VTSwEncStreamBufferAllocate(void *handle, u8 **write_ptr, u32 *len, u32 expect_len);
void MP4VTSwEncStreamBufferUpdate(void *inst, u8 *write_ptr, i32 is_slice_contained);
#endif

void MP4VTSwEncYUVEncoded(void *handle, u8 *luma_ptr);


#if defined(WIN32_PC_SIM)
void *UVTSwEncCallocEss(void *cb_handle, u32 n, u32 s);
#else
#define UVTSwEncCallocEss MP4VTSwEncCalloc
#endif

/*------------------------------------------------------------------------------
    6. MTK added functions
------------------------------------------------------------------------------*/
//#include "UVTEncSim.h"
//true_e UVTSwEncChkSHV(UVTSwEncInst inst);

#ifdef __cplusplus
    }
#endif

#endif /*__UVTSWENCAPI_H__*/
