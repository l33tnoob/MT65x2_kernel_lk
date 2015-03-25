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
 

#ifndef _MTK_HDR_H
#define _MTK_HDR_H

#include "MTKHdrType.h"
#include "MTKHdrErrCode.h"

//#define HDR_PROFILING
#define HDR_DEBUG
//#define PC_SIM
#define HDR_MAX_CORE_NO     (4)

typedef enum
{
    HDR_STATE_PREPROCESS,
    HDR_STATE_SE,
    HDR_STATE_ALIGNMENT,
    HDR_STATE_BLEND,
}HDR_PROC_STATE_ENUM;

typedef enum
{

    HDR_PROC_ROUND1 = 1,
    HDR_PROC_ROUND2 = 2,
}HDR_PROC_ROUND_ENUM;


typedef enum
{
    HDR_STATE_STANDBY,
    HDR_STATE_INIT,
    HDR_STATE_PROC,
    HDR_STATE_READY,
    HDR_STATE_IDLE,
    HDR_STATE_MAX
}HDR_STATE_ENUM;

/* input source type enum */
typedef enum
{
	HDR_INFO_SRC_YUV,    /* from YUV sensor */
	HDR_INFO_SRC_RAW, /* from RAW sensor */
	HDR_INFO_SRC_NUM
}HDR_INFO_SRC_ENUM;

typedef enum
{
    HDR_PROCESS_FAST,
    HDR_PROCESS_NORMAL
}HDR_PROCESS_SPEED;

typedef struct
{
    MUINT32 BRatio;
    MUINT32 Gain[11]; //Gain for top N level

    double BottomFRatio;
    double TopFRatio;
    MUINT32 BottomFBound;
    MUINT32 TopFBound;
    MINT32 ThHigh;
    MINT32 ThLow;
    MUINT32 TargetLevelSub;

    // Multi-Core parameters
    MUINT32 CoreNumber;         // given cpu core number
    HDR_PROCESS_SPEED HdrSpeed; //Note!! do not set arbitrarily! set fast mode may cause system hang!!
}HDR_TUNING_PARA_STRUCT;


typedef enum
{
    HDR_FEATURE_BEGIN = 0,
    HDR_FEATURE_SET_PTHREAD_ATTR,
    HDR_FEATURE_SET_SE_INPUT_IMG,
    HDR_FEATURE_GET_SE_RESULT,
    HDR_FEATURE_SET_REC_PAIR_INFO,
    HDR_FEATURE_GET_BMAP,
    HDR_FEATURE_SET_BL_BMAP,
    HDR_FEATURE_GET_RESULT,
    HDR_FEATURE_GET_STATUS,
    HDR_FEATURE_SAVE_LOG,

    HDR_FEATURE_GET_PROC_INFO,
    HDR_FEATURE_SET_PROC_INFO,
    HDR_FEATURE_SET_WORK_BUF_INFO,
  

    HDR_FEATURE_SET_BMAP_BUFFER,
    HDR_FEATURE_SET_RESULT_BUFFER,
    
    HDR_FEATURE_MAX
}	HDR_FEATURE_ENUM;


typedef struct
{
    HDR_TUNING_PARA_STRUCT hdr_tuning_data;

    MUINT16 image_num;
    MUINT16 ev_gain1;
    MUINT16 ev_gain2; 
    MUINT16 ev_gain3; 
    MUINT16 image_width;
    MUINT16 image_height;
    MUINT32 target_tone;
    HDR_INFO_SRC_ENUM HdrSrcInfo;  /* information version control, 0,1  for YUV, RAW*/    

    //MUINT32 image_addr[3]; // input image address


} HDR_SET_ENV_INFO_STRUCT, *P_HDR_SET_ENV_INFO_STRUCT;


typedef struct
{
    MUINT16 small_image_width;
    MUINT16 small_image_height;
    MUINT32 ext_mem_size; //working buffer size
}HDR_GET_PROC_INFO_STRUCT, *P_HDR_GET_PROC_INFO_STRUCT;


typedef struct
{
    HDR_PROC_ROUND_ENUM ehdr_round;
    MUINT32 input_source_image_width;
    MUINT32 input_source_image_height;
    MUINT32 small_image_addr[3]; //To to MAV and weighting 
    MUINT32 input_source_image[3];  //The image to be done by HDR
}HDR_SET_PROC_INFO_STRUCT, *P_HDR_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 ext_mem_size;
    MUINT32 ext_mem_start_addr; //working buffer start address
}HDR_SET_WORK_BUF_INFO_STRUCT, *P_SET_WORK_BUF_INFO_STRUCT;


typedef struct
{
    MUINT16 se_image_width;
    MUINT16 se_image_height;
    MUINT32 se_image_addr;
}EIS_INPUT_IMG_INFO, *P_EIS_INPUT_IMG_INFO;


typedef struct
{
    MUINT32 weight_table_width;
    MUINT32 weight_table_height;
    MUINT8 *weight_table_data; 
}WEIGHT_TBL_INFO, *P_WEIGHT_TBL_INFO;

/*
typedef struct
{
    MUINT16 weight_map_width;
    MUINT16 weight_map_height;
    MUINT32 eis_image_addr;
}WEIGHTING_MAP_INFO, *P_WEIGHTING_MAP_INFO;
*/


typedef struct
{
    MUINT16 output_image_width;
    MUINT16 output_image_height;
    MUINT32 output_image_addr;
    MUINT32 result_buffer_size; // ION
}HDR_RESULT_STRUCT;


typedef struct
{
    MUINT32 bmap_width;
    MUINT32 bmap_height;
    MUINT32 bmap_image_addr[3];
    MUINT32 bmap_image_size;
}HDR_BMAP_BUFFER;


class MTKHdr {
public:
    static MTKHdr* createInstance();
    virtual void   destroyInstance() = 0;
       
    virtual ~MTKHdr(){};
    // Process Control
    virtual MRESULT HdrInit(void* InitInData, void* InitOutData);
    virtual MRESULT HdrMain(HDR_PROC_STATE_ENUM HdrState);	// START
    virtual MRESULT HdrReset();   //Reset
            
	// Feature Control        
	virtual MRESULT HdrFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
    
};


#endif
