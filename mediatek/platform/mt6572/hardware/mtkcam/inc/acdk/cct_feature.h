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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCCTFeature.h $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCCTFeature.h
//! \brief

#ifndef _CCT_FEATURE_H_
#define _CCT_FEATURE_H_

#include "mtkcam/acdk/AcdkTypes.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "kd_imgsensor_define.h"
#include "camera_custom_nvram.h"

#define MAX_SUPPORT_CMD                     0x1000
#define ACDK_CCT_FEATURE_START              0x1000
#define CCT_CAMCTRL_FEATURE_START           (ACDK_CCT_FEATURE_START + 0x1000)
#define CCT_ISP_FEATURE_START               (ACDK_CCT_FEATURE_START + 0x2000)
#define CCT_SENSOR_FEATURE_START            (ACDK_CCT_FEATURE_START + 0x3000)
#define CCT_3A_FEATURE_START                (ACDK_CCT_FEATURE_START + 0x4000)
#define CCT_NVRAM_FEATURE_START             (ACDK_CCT_FEATURE_START + 0x5000)
#define CCT_CALI_FEATURE_START              (ACDK_CCT_FEATURE_START + 0x6000)
#define ACDK_CCT_FEATURE_END                (ACDK_CCT_FEATURE_START + 0x7000)


#define CCT_FL_ENG_SIZE 256  //y, rg, bg, sizeo of y is 256*2, total of 3 tab is 256*2*3
#define CCT_FL_ENG_UNIT_SIZE 2
#define CCT_FL_NVDATA_SIZE 4096


typedef enum
{
    ACDK_CCT_FEATURE_BEGIN = ACDK_CCT_FEATURE_START,

    //Camera Control
    ACDK_CCT_OP_DEV_GET_DSC_INFO  = CCT_CAMCTRL_FEATURE_START,
    ACDK_CCT_OP_RESUME_AE_AWB_PREVIEW_FROM_UNFINISHED_CAPTURE,
    ACDK_CCT_OP_DEV_MODE_SIZE,

    //NVRAM
    ACDK_CCT_OP_LOAD_FROM_NVRAM = CCT_NVRAM_FEATURE_START,               //load from nvram
    ACDK_CCT_OP_SAVE_TO_NVRAM,                   //save to nvram

    //ISP Control
    ACDK_CCT_OP_QUERY_ISP_ID = CCT_ISP_FEATURE_START,
    ACDK_CCT_OP_REG_READ,                                 //register read (Sensor / ISP)
    ACDK_CCT_OP_REG_WRITE,                              //register write (Sensor / ISP)
    ACDK_CCT_OP_ISP_READ_REG,
    ACDK_CCT_OP_ISP_WRITE_REG,
    ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX,
    ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX,
    ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS,
    ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS,
    ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF,
    ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
    ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA,
    ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
    ACDK_CCT_V2_ISP_DEFECT_TABLE_ON,
    ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF,
    ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,
    ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,
    ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,
    ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
    ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
    ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
    ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
    ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA,
    ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
    ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX,
    ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM,
    ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM,

    //CCM
    ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM,                         // MSDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM
    ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM,                       // MSDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM
    ACDK_CCT_V2_OP_AWB_GET_CCM_PARA,                                    // MSDK_CCT_V2_OP_AWB_GET_CCM_PARA
    ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS,                                // MSDK_CCT_V2_OP_AWB_GET_CCM_STATUS
    ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM,                              // MSDK_CCT_V2_OP_AWB_GET_CURRENT_CCM
    ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM,                                 // MSDK_CCT_V2_OP_AWB_GET_NVRAM_CCM
    ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM,                             // MSDK_CCT_V2_OP_AWB_SET_CURRENT
    ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM,                                //  MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM
    ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA,                            // MSDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA
    ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS,                        // MSDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS
    ACDK_CCT_OP_SET_CCM_MODE,
    ACDK_CCT_OP_GET_CCM_MODE,
    ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF,
    ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF,

    //Gamma
    ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS,                            //MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS
    ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG,                 //MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG
    //ACDK_CCT_V2_OP_AE_GET_GAMMA_PARAS,          //This seems for preview, it just for winmo AE         //MSDK_CCT_V2_OP_AE_GET_GAMMA_PARAS
    //ACDK_CCT_V2_OP_AE_UPDATE_GAMMA_PARAS,   //This seems for preview, it just for winmo AE         //MSDK_CCT_V2_OP_AE_UPDATE_GAMMA_PARAS
    //ACDK_CCT_V2_OP_AE_GET_GAMMA_PARA,           //phase out        //MSDK_CCT_V2_OP_AE_GET_GAMMA_PARA
    ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE,                            //MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE
    //ACDK_CCT_V2_OP_AE_UPDATE_GAMMA_PARA,     //This seems for preview, it just for winmo AE        //MSDK_CCT_V2_OP_AE_UPDATE_GAMMA_PARA
    ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE,
    //ISP Module Control
    ACDK_CCT_V2_OP_SET_OB_ON_OFF,
    ACDK_CCT_V2_OP_SAVE_OB_ON_OFF,
    ACDK_CCT_V2_OP_GET_OB_ON_OFF,
    ACDK_CCT_V2_OP_SET_NR_ON_OFF,
    ACDK_CCT_V2_OP_GET_NR_ON_OFF,
    ACDK_CCT_V2_OP_SET_EE_ON_OFF,
    ACDK_CCT_V2_OP_GET_EE_ON_OFF,
    ACDK_CCT_OP_SET_ISP_ON,
    ACDK_CCT_OP_SET_ISP_OFF,
    ACDK_CCT_OP_GET_ISP_ON_OFF,
    //ISP NVRAM
    ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM,
    ACDK_CCT_OP_ISP_SAVE_TO_NVRAM,
    //Shading NVRAM
    ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM,
    ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM,
    //PCA
    ACDK_CCT_OP_ISP_SET_PCA_TABLE,
    ACDK_CCT_OP_ISP_GET_PCA_TABLE,
    ACDK_CCT_OP_ISP_SET_PCA_PARA,
    ACDK_CCT_OP_ISP_GET_PCA_PARA,

    //Calibration
    ACDK_CCT_OP_DEFECT_TABLE_CAL = CCT_CALI_FEATURE_START,
    ACDK_CCT_V2_OP_SHADING_CAL,
    ACDK_CCT_V2_OP_SHADING_VERIFY,
    ACDK_CCT_V2_OP_DEFECT_VERIFY,
    ACDK_CCT_OP_SET_CALI_MODE,


    //Sensor
    ACDK_CCT_OP_QUERY_SENSOR = CCT_SENSOR_FEATURE_START,
    ACDK_CCT_OP_READ_SENSOR_REG,
    ACDK_CCT_OP_WRITE_SENSOR_REG,
    ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
    ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION,
    ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,
    ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
    ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
    ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
    ACDK_CCT_OP_GET_SENSOR_PREGAIN,
    ACDK_CCT_OP_SET_SENSOR_PREGAIN,
    ACDK_CCT_OP_GET_SENSOR_INFO,
    ACDK_CCT_OP_SET_SENSOR_INITIALIZE_AF,
    ACDK_CCT_OP_SET_SENSOR_CONSTANT_AF,
    ACDK_CCT_OP_SET_SENSOR_MOVE_FOCUS_LENS,
    //AE
    ACDK_CCT_OP_AE_ENABLE = CCT_3A_FEATURE_START,
    ACDK_CCT_OP_AE_DISABLE,
    ACDK_CCT_OP_AE_GET_ENABLE_INFO,
    ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE,
    ACDK_CCT_OP_DEV_AE_GET_INFO,
    ACDK_CCT_V2_OP_AE_GET_SCENE_MODE,
    ACDK_CCT_V2_OP_AE_SET_METERING_MODE,
    ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,
    ACDK_CCT_V2_OP_AE_SELECT_BAND,
    ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
    ACDK_CCT_V2_OP_AE_GET_BAND,
    ACDK_CCT_V2_OP_AE_GET_METERING_RESULT,
    ACDK_CCT_OP_DEV_AE_APPLY_INFO,
    ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,
    ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION,
    ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING,
    ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING,
    ACDK_CCT_OP_AE_GET_ISP_OB,
    ACDK_CCT_OP_AE_SET_ISP_OB,
    ACDK_CCT_OP_AE_GET_ISP_RAW_GAIN,
    ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN,
    ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME,
    ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE,
    ACDK_CCT_OP_AE_SET_SENSOR_GAIN,
    ACDK_CCT_OP_AE_CAPTURE_MODE,

    //AWB
    ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN,
    ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN,
    ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,
    ACDK_CCT_V2_OP_AWB_GET_GAIN,
    ACDK_CCT_V2_OP_AWB_SET_GAIN,
    ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,
    ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,
    ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA,
    ACDK_CCT_OP_AWB_SET_AWB_MODE,
    ACDK_CCT_OP_AWB_GET_AWB_MODE,
    ACDK_CCT_OP_AWB_GET_LIGHT_PROB,

    //AF
    ACDK_CCT_V2_OP_AF_OPERATION,
    ACDK_CCT_V2_OP_MF_OPERATION,
    ACDK_CCT_V2_OP_GET_AF_INFO,
    ACDK_CCT_V2_OP_AF_GET_BEST_POS,
    ACDK_CCT_V2_OP_AF_CALI_OPERATION,
    ACDK_CCT_V2_OP_AF_SET_RANGE,
    ACDK_CCT_V2_OP_AF_GET_RANGE,
    ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM,
    ACDK_CCT_V2_OP_AF_READ,
    ACDK_CCT_V2_OP_AF_APPLY,
    ACDK_CCT_V2_OP_AF_GET_FV,
    ACDK_CCT_OP_AF_ENABLE,
    ACDK_CCT_OP_AF_DISABLE,
    ACDK_CCT_OP_AF_GET_ENABLE_INFO,

    // Strobe
    ACDK_CCT_OP_FLASH_ENABLE ,	//0,
    ACDK_CCT_OP_FLASH_DISABLE,	//1,
    ACDK_CCT_OP_FLASH_GET_INFO,	//2, not used
    ACDK_CCT_OP_FLASH_CONTROL,	//3,
    ACDK_CCT_OP_STROBE_RATIO_TUNING,	//4, not used
    //for tuning





	//code ok
	ACDK_CCT_OP_STROBE_READ_NVRAM,	//5,
    ACDK_CCT_OP_STROBE_WRITE_NVRAM,	//6
    ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM,	//7
	ACDK_CCT_OP_STROBE_SET_PARAM,	//8
	ACDK_CCT_OP_STROBE_GET_PARAM,	//9

	ACDK_CCT_OP_STROBE_GET_NVDATA, //10,
	ACDK_CCT_OP_STROBE_SET_NVDATA, //11,

	ACDK_CCT_OP_STROBE_GET_ENG_Y,	//12,
	ACDK_CCT_OP_STROBE_SET_ENG_Y,	//13
	ACDK_CCT_OP_STROBE_GET_ENG_RG,	//14
	ACDK_CCT_OP_STROBE_SET_ENG_RG,	//15
	ACDK_CCT_OP_STROBE_GET_ENG_BG,	//16
	ACDK_CCT_OP_STROBE_SET_ENG_BG,	//17

	ACDK_CCT_OP_STROBE_NVDATA_TO_FILE,	//18,
	ACDK_CCT_OP_STROBE_FILE_TO_NVDATA,	//19,




}ACDK_CCT_FEATURE_ENUM;


enum ACDK_FL_CCT_ID
{
ACDK_FL_CCT_ID_yTar = 100,
ACDK_FL_CCT_ID_antiIsoLevel,
ACDK_FL_CCT_ID_antiExpLevel,
ACDK_FL_CCT_ID_antiStrobeLevel,
ACDK_FL_CCT_ID_antiUnderLevel,
ACDK_FL_CCT_ID_antiOverLevel,
ACDK_FL_CCT_ID_foregroundLevel,
ACDK_FL_CCT_ID_isRefAfDistance,
ACDK_FL_CCT_ID_accuracyLevel,
ACDK_FL_CCT_ID_isTorchEngUpdate,
ACDK_FL_CCT_ID_isAfEngUpdate,
ACDK_FL_CCT_ID_isNormaEnglUpdate,
ACDK_FL_CCT_ID_isLowBatEngUpdate,
ACDK_FL_CCT_ID_isBurstEngUpdate,
ACDK_FL_CCT_ID_torchEngMode,
ACDK_FL_CCT_ID_torchPeakI,
ACDK_FL_CCT_ID_torchAveI,
ACDK_FL_CCT_ID_torchDuty,
ACDK_FL_CCT_ID_torchStep,
ACDK_FL_CCT_ID_afEngMode,
ACDK_FL_CCT_ID_afPeakI,
ACDK_FL_CCT_ID_afAveI,
ACDK_FL_CCT_ID_afDuty,
ACDK_FL_CCT_ID_afStep,
ACDK_FL_CCT_ID_pmfEngMode,
ACDK_FL_CCT_ID_pfAveI,
ACDK_FL_CCT_ID_mfAveIMax,
ACDK_FL_CCT_ID_mfAveIMin,
ACDK_FL_CCT_ID_pmfPeakI,
ACDK_FL_CCT_ID_pfDuty,
ACDK_FL_CCT_ID_mfDutyMax,
ACDK_FL_CCT_ID_mfDutyMin,
ACDK_FL_CCT_ID_pmfStep,
ACDK_FL_CCT_ID_IChangeByVBatEn,
ACDK_FL_CCT_ID_vBatL,
ACDK_FL_CCT_ID_pfAveIL,
ACDK_FL_CCT_ID_mfAveIMaxL,
ACDK_FL_CCT_ID_mfAveIMinL,
ACDK_FL_CCT_ID_pmfPeakIL,
ACDK_FL_CCT_ID_pfDutyL,
ACDK_FL_CCT_ID_mfDutyMaxL,
ACDK_FL_CCT_ID_mfDutyMinL,
ACDK_FL_CCT_ID_pmfStepL,
ACDK_FL_CCT_ID_IChangeByBurstEn,
ACDK_FL_CCT_ID_pfAveIB,
ACDK_FL_CCT_ID_mfAveIMaxB,
ACDK_FL_CCT_ID_mfAveIMinB,
ACDK_FL_CCT_ID_pmfPeakIB,
ACDK_FL_CCT_ID_pfDutyB,
ACDK_FL_CCT_ID_mfDutyMaxB,
ACDK_FL_CCT_ID_mfDutyMinB,
ACDK_FL_CCT_ID_pmfStepB,
ACDK_FL_CCT_ID_distance
};

enum ACDK_FL_CCT_ENG_MODE
{
ACDK_FL_CCT_ENG_INDEX_MODE,
ACDK_FL_CCT_ENG_CURRENT_MODE
};

typedef enum {
    ACDK_CCT_CDVT_START = ACDK_CCT_FEATURE_END,
    // CDVT
    ACDK_CCT_OP_CDVT_SENSOR_TEST,           // [CDVT] Sensor Test
    ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION, // [CDVT] Sensor Calibration
    ACDK_CCT_OP_FLASH_CALIBRATION,
    ACDK_CCT_CDVT_END
} ACDK_CCT_CDVT_ENUM;

#if 0
typedef enum {
    ACDK_CCT_REG_ISP = 0,
    ACDK_CCT_REG_CMOS,
    ACDK_CCT_REG_CCD
} ACDK_CCT_REG_TYPE_ENUM;
#endif

/////////////////////////////////////////////////////////////////////////
//! Camera mode
/////////////////////////////////////////////////////////////////////////
typedef enum
{
    OUTPUT_PURE_RAW8 = 0,
    OUTPUT_PURE_RAW10,
    OUTPUT_PROCESSED_RAW8,
    OUTPUT_PROCESSED_RAW10,
    OUTPUT_YUV,
    OUTPUT_JPEG
} ACDK_CCT_CAP_OUTPUT_FORMAT;
typedef struct
{
    UINT32 u2PreviewWidth;
    UINT32 u2PreviewHeight;
    UINT16 u16PreviewTestPatEn;
    Func_CB fpPrvCB;
}ACDK_CCT_CAMERA_PREVIEW_STRUCT, *PACDK_CCT_CAMERA_PREVIEW_STRUCT;

typedef enum {
    ACDK_CCT_IS_SHADING_SUPPORTED = 0,
    ACDK_CCT_IS_AUTODEFECT_SUPPORTED,
    ACDK_CCT_IS_AUTODEFECT_COUNT_SUPPORTED,
    ACDK_CCT_IS_GAMMA_TABLE_SUPPORTED,
    ACDK_CCT_IS_CAPMODE_SUPPORTED,
    ACDK_CCT_IS_ISO_SUPPORTED,
    ACDK_CCT_IS_ISO_PRIORITY_SUPPORTED,
    ACDK_CCT_IS_AF_SUPPORTED,
} ACDK_CCT_IS_SUPPORTED_ENUM;

typedef enum
{
    ACDK_DEFECT_MODE_NONE = 0,
    ACDK_DEFECT_MODE_CAPTURE,
    ACDK_DEFECT_MODE_PREVIEW,
    ACDK_DEFECT_MODE_BOTH
}ACDK_DEFECT_MODE_ENUM;


/*******************************************************/
/************** Sensor Info CCT exposed struct **************/
/*******************************************************/
#if 0   //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	SENSOR_OUTPUT_FORMAT_RAW_B=0,
	SENSOR_OUTPUT_FORMAT_RAW_Gb,
	SENSOR_OUTPUT_FORMAT_RAW_Gr,
	SENSOR_OUTPUT_FORMAT_RAW_R,
	SENSOR_OUTPUT_FORMAT_UYVY,
	SENSOR_OUTPUT_FORMAT_VYUY,
	SENSOR_OUTPUT_FORMAT_YUYV,
	SENSOR_OUTPUT_FORMAT_YVYU,
	SENSOR_OUTPUT_FORMAT_CbYCrY,
	SENSOR_OUTPUT_FORMAT_CrYCbY,
	SENSOR_OUTPUT_FORMAT_YCbYCr,
	SENSOR_OUTPUT_FORMAT_YCrYCb,
} ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM;
#endif


#if 0   //@Sean Move to kd_imgsensor_define.h
typedef enum
{
    CMOS_SENSOR=0,
    CCD_SENSOR
} SENSOR_TYPE_ENUM;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	UINT16				SensorId;
	SENSOR_TYPE_ENUM	SensorType;
	ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM SensorOutputDataFormat;
} ACDK_SENSOR_ENG_INFO_STRUCT;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	UINT32 RegAddr;
	UINT32 RegData;
} ACDK_SENSOR_REG_INFO_STRUCT;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
    UINT32 	GroupIdx;
    UINT32 	ItemIdx;
    UINT8   ItemNamePtr[50];     	// item name
    UINT32 	ItemValue;                // item value
    MBOOL    IsTrueFalse;               // is this item for enable/disable functions
    MBOOL	IsReadOnly;             // is this item read only
    MBOOL	IsNeedRestart;          // after set this item need restart
    UINT32	Min;                    // min value of item value
    UINT32	Max;                    // max value of item value
} ACDK_SENSOR_ITEM_INFO_STRUCT;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	UINT32 GroupIdx;
	UINT32 ItemCount;
	UINT8 *GroupNamePtr;
} ACDK_SENSOR_GROUP_INFO_STRUCT;
#endif

/* ACDK_CCT_OP_QUERY_SENSOR */
#if 0 //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	ACDK_CCT_REG_TYPE_ENUM		Type;			// ISP, CMOS_SENSOR, CCD_SENSOR
	UINT32						DeviceId;
	ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM	StartPixelBayerPtn;
	UINT16						GrabXOffset;
	UINT16						GrabYOffset;
} ACDK_CCT_SENSOR_INFO_STRUCT, *PACDK_CCT_SENSOR_INFO_STRUCT;
#endif

//! for ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION */
typedef struct
{
	UINT16 SensorPreviewWidth;
	UINT16 SensorPreviewHeight;
	UINT16 SensorFullWidth;
	UINT16 SensorFullHeight;
	UINT16 SensorVideoWidth;
	UINT16 SensorVideoHeight;
} ACDK_CCT_SENSOR_RESOLUTION_STRUCT, *PACDK_CCT_SENSOR_RESOLUTION_STRUCT;







/*******************************************************************************
*
********************************************************************************/
enum ACDK_CCT_ISP_REG_CATEGORY
{
    EIsp_Category_OBC = 0,
    EIsp_Category_NR1,
    EIsp_Category_LSC,
    EIsp_Category_CFA,
    EIsp_Category_ANR,
    EIsp_Category_CCR,
    EIsp_Category_EE,
    EIsp_Category_NR3D,
    EIsp_Category_MFB,
    EIsp_Num_Of_Category
};


struct ACDK_CCT_ISP_NVRAM_REG
{
	ISP_NVRAM_OBC_T 			OBC[NVRAM_OBC_TBL_NUM];
	ISP_NVRAM_NR1_T 			NR1[NVRAM_NR1_TBL_NUM];
	ISP_NVRAM_LSC_T             LSC[NVRAM_LSC_TBL_NUM];
	ISP_NVRAM_CFA_T 			CFA[NVRAM_CFA_TBL_NUM];
	ISP_NVRAM_ANR_T 			ANR[NVRAM_ANR_TBL_NUM];
	ISP_NVRAM_CCR_T 			CCR[NVRAM_CCR_TBL_NUM];
	ISP_NVRAM_EE_T				EE[NVRAM_EE_TBL_NUM];
	ISP_NVRAM_NR3D_T			NR3D[NVRAM_NR3D_TBL_NUM];
	ISP_NVRAM_MFB_T 			MFB[NVRAM_MFB_TBL_NUM];
};

struct ACDK_ISP_NVRAM_REG
{
	ISP_NVRAM_REGISTER_STRUCT NVRAM_REG;
};

struct ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
{
    MUINT32                     u4Index;
    ACDK_CCT_ISP_REG_CATEGORY   eCategory;
};

struct ACDK_CCT_ISP_GET_TUNING_PARAS
{
    ACDK_CCT_ISP_NVRAM_REG  stIspNvramRegs;
};

struct ACDK_CCT_ISP_SET_TUNING_PARAS
{
    MUINT32                     u4Index;
    ACDK_CCT_ISP_REG_CATEGORY   eCategory;
    ACDK_CCT_ISP_NVRAM_REG      stIspNvramRegs;
};

struct ACDK_CCT_CCM_STRUCT
{
    MUINT32 M11;
    MUINT32 M12;
    MUINT32 M13;
    MUINT32 M21;
    MUINT32 M22;
    MUINT32 M23;
    MUINT32 M31;
    MUINT32 M32;
    MUINT32 M33;
};

struct ACDK_CCT_SET_NVRAM_CCM
{
    MUINT32                     u4Index;
    ACDK_CCT_CCM_STRUCT         ccm;
};

struct ACDK_CCT_NVRAM_CCM_PARA
{
    ACDK_CCT_CCM_STRUCT         ccm[NVRAM_CCM_TBL_NUM];
};

struct ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
{
    MUINT32             u4Offset;           //  in
    MUINT32             u4Count;            //  in
    MUINT8              u8ColorTemperature; //  in
    MUINT8              Reserved[3];        //
    ISP_NVRAM_PCA_BIN_T buffer[400];        //  in/out
};

struct ACDK_CCT_ACCESS_PCA_CONFIG
{
    MUINT32 EN              : 1; //[0]
    MUINT32 Reserved        : 31;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~





typedef enum
{
    CAMERA_TUNING_PREVIEW_SET = 0,
    CAMERA_TUNING_CAPTURE_SET,
    CAMERA_TUNING_VIDEO_SET,
    CAMERA_TUNING_BINNING_SET
} CAMERA_TUNING_SET_ENUM;

/* Enable/Disable Shading/Defect.. */
/* ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF */
typedef struct
{
    CAMERA_TUNING_SET_ENUM    Mode;
    UINT8    Enable;
}ACDK_CCT_MODULE_CTRL_STRUCT, *PACDK_CCT_MODULE_CTRL_STRUCT;

#if 1//remove if possible
typedef enum
{
	ISP_CATEGORY_NR1 = 0,
	ISP_CATEGORY_NR2,
	ISP_CATEGORY_EDGE,
	ISP_CATEGORY_SAT,
	ISP_CATEGORY_CONTRAST,
	ISP_CATEGORY_AUTO_DEFECT,
	ISP_CATEGORY_AUTO_SHADING
}ACDK_ISP_CATEGORY_ENUM;

typedef enum
{
	/* must to keep w/o compiler option for meta API */
	CAMERA_COMP_PREVIEW_NORMAL_SET=0,
	CAMERA_COMP_PREVIEW_LOWLIGHT_SET,
	CAMERA_COMP_CAPTURE_NORMAL_SET,
	CAMERA_COMP_CAPTURE_LOWLIGHT_SET,
	CAMERA_COMP_END
	/* new mode should insert before CAMERA_COMP_END */
}CAMERA_COMP_SET_ENUM;

typedef struct // 0x840B0550 / 0x840B0568 ~ 0x840B058C
{
    UINT8 ENNR;
    UINT8 GNF;
    UINT8 S1;
    UINT8 S2;
	UINT16 MBND;
	UINT8 GN1;
	UINT8	GN2;
	UINT8	GN3;
	UINT8	VLR1;
	UINT8 VLR2;
	UINT8 VLR3;
	UINT8 VLR4;
	UINT8	VLR5;
	UINT8 VLR6;
	UINT8 VLR7;
	UINT8 VLR8;
	UINT8	VLGR1;
	UINT8 VLGR2;
	UINT8 VLGR3;
	UINT8 VLGR4;
	UINT8	VLGR5;
	UINT8 VLGR6;
	UINT8 VLGR7;
	UINT8 VLGR8;
	UINT8	VLGB1;
	UINT8 VLGB2;
	UINT8 VLGB3;
	UINT8 VLGB4;
	UINT8	VLGB5;
	UINT8 VLGB6;
	UINT8 VLGB7;
	UINT8 VLGB8;
	UINT8	VLB1;
	UINT8 VLB2;
	UINT8 VLB3;
	UINT8 VLB4;
	UINT8	VLB5;
	UINT8 VLB6;
	UINT8 VLB7;
	UINT8 VLB8;
	//CT related
	UINT8 ENCT;
	UINT8 MDCT;
	UINT8 CT_DIV;
	UINT8 CT_THRD;
}winmo_cct_NR1_comp_struct;

typedef struct
{
	UINT8 ENC;
	UINT8 ENY;
	UINT8 UV_SMPL;
	UINT8 S2;
	UINT8 S3;
	UINT8	SY1;
	UINT8	SC1;
	UINT8	GNY;
	UINT8 GNC;
	UINT8 PTY1;
	UINT8 PTY2;
	UINT8 PTY3;
	UINT8 PTY4;
	UINT8 PTC1;
	UINT8 PTC2;
	UINT8 PTC3;
	UINT8 PTC4;
}winmo_cct_NR2_comp_struct;

typedef struct
{
	UINT8 ED_GAIN_TH;
	UINT8 CLIP_UNDER_TH;
	UINT8 CLIP_OVER_TH;
	UINT8 ED_BOUND_EN;
	UINT8 FILTER_SEL;
	UINT8	CLIP_UNDER_EN;
	UINT8	CLIP_OVER_EN;
	UINT8	YEDGE_EN;
	UINT8 RGBEDGE_EN;
	UINT8 ED_LUT_X1;
	UINT8 ED_LUT_X2;
	UINT8 ED_LUT_X3;
	UINT8 ED_LUT_Y1;
	UINT8 ED_LUT_Y2;
	UINT8 ED_LUT_Y3;
	UINT8 ED_LUT_Y4;
}winmo_cct_Edge_comp_struct;

typedef struct
{
	UINT8 ENC3;
	UINT8 Y1;
	UINT8 Y2;
	UINT8 Y3;
	UINT8 Y4;
	UINT8	G1;
	UINT8	G2;
	UINT8	G3;
	UINT8 G4;
	UINT8 G5;
}winmo_cct_Saturation_comp_struct;

typedef struct
{
	UINT8 ENY3;
	UINT8 OFSTY;
	UINT8 GAINY;
}winmo_cct_Contrast_comp_struct;

typedef struct
{
	UINT8  ENDP;
	UINT16 DP_THRD0;
	UINT16 DP_THRD1;
	UINT16 DP_THRD2;
	UINT16 DP_THRD3;
	UINT16 DP_THRD4;
	UINT16 DP_THRD5;
	UINT16 DP_THRD6;
	UINT16 DP_THRD7;
	UINT8 DP_CD1;
	UINT8 DP_CD2;
	UINT8 DP_CD3;
	UINT8 DP_CD4;
	UINT8 DP_CD5;
	UINT8 DP_CD6;
	UINT8 DP_CD7;
	UINT8 DP_SEL;
	UINT8 DP_NUM;
}winmo_cct_autodefect_comp_struct;


typedef struct {
    UINT8 m11;
    UINT8 m12;
    UINT8 m13;
    UINT8 m21;
    UINT8 m22;
    UINT8 m23;
    UINT8 m31;
    UINT8 m32;
    UINT8 m33;
}winmo_cct_ccm_matrix_struct, WINMO_CCT_CCM_STRUCT, *PWINMO_CCT_CCM_STRUCT;


typedef struct
{
    UINT8    LightSource[3][3][3];
    UINT8    CCMPreferEn;
}winmo_ccm_para_struct, WINMO_CCM_PARA_STRUCT, *PWINMO_CCM_PARA_STRUCT;

/* CCM light mode */
/* MSDK_CCT_V2_OP_AWB_GET_NVRAM_CCM; MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM */
typedef struct {
    WINMO_CCT_CCM_STRUCT    CCM;
    UINT8    CCMLightMode;
}ACDK_CCT_CCM_LIGHTMODE_STRUCT, *PACDK_CCT_CCM_LIGHTMODE_STRUCT;

typedef struct {
    MBOOL dynamicCCMEn;
}ACDK_CCT_CCM_STATUS_STRUCT, *PACDK_CCT_CCM_STATUS_STRUCT;
#endif

#define GAMMA_STEP_NO 144
#define TOTAL_GAMMA_NO 1

typedef struct
{
	UINT16 r_tbl[GAMMA_STEP_NO];
	UINT16 g_tbl[GAMMA_STEP_NO];
	UINT16 b_tbl[GAMMA_STEP_NO];
} ACDK_CCT_GAMMA_TABLE_STRUCT, *PACDK_CCT_GAMMA_TABLE_STRUCT;

#if 1//remove if possible
typedef struct {

	winmo_cct_NR1_comp_struct  isp_nr1_set[7];
       winmo_cct_NR2_comp_struct  isp_nr2_set[7];
       winmo_cct_Edge_comp_struct  isp_edge_set[7];
       winmo_cct_Saturation_comp_struct isp_sat_set[7];
       winmo_cct_Contrast_comp_struct isp_contrast_set[3];
	winmo_cct_autodefect_comp_struct isp_auto_defect_set[3];
	UINT8  current_index;
	UINT8  current_category;
} WINMO_CCT_ISP_TUNING_CMD;

typedef enum
{
	ACDK_CCT_COMP_PREVIEW_NORMAL_SET = 0
	,ACDK_CCT_COMP_PREVIEW_LOWLIGHT_SET
	,ACDK_CCT_COMP_CAPTURE_NORMAL_SET
	,ACDK_CCT_COMP_CAPTURE_LOWLIGHT_SET
} ACDK_CCT_COMP_SET_ENUM;

typedef struct {
	ACDK_CCT_COMP_SET_ENUM  comp_set;
	WINMO_CCT_ISP_TUNING_CMD  	*tuning_cmd;
} ACDK_CCT_ISP_TUNING_CMD,  *PACDK_CCT_ISP_TUNING_CMD;
#endif

/* ISP or 3A function enable */
/* MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF */
/* MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS; MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG */
typedef struct {
	MBOOL						Enable;
}ACDK_CCT_FUNCTION_ENABLE_STRUCT, *PACDK_CCT_FUNCTION_ENABLE_STRUCT;
#if 1//remove if possible
typedef struct {
	ACDK_ISP_CATEGORY_ENUM	index;
	UINT8					value;
}ACDK_ISP_TUNING_SET;
#endif
typedef struct
{
	UINT8 SDBLK_TRIG;
	UINT8 SHADING_EN;
	UINT8 SHADINGBLK_XOFFSET;
	UINT8 SHADINGBLK_YOFFSET;
	UINT8 SHADINGBLK_XNUM;
	UINT8 SHADINGBLK_YNUM;
	UINT16 SHADINGBLK_WIDTH;
	UINT16 SHADINGBLK_HEIGHT;
	UINT32 SHADING_RADDR;
	UINT16 SD_LWIDTH;
	UINT16 SD_LHEIGHT;
	UINT8 SDBLK_RATIO00;
	UINT8 SDBLK_RATIO01;
	UINT8 SDBLK_RATIO10;
	UINT8 SDBLK_RATIO11;
	UINT16 SD_TABLE_SIZE;
	//UINT32 SD_TABLE[MAX_SHADING_TABLE_SIZE];
}winmo_cct_shading_comp_struct;

//! Lens Info use
//! FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA, FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA
typedef struct
{
	UINT8 SHADING_MODE;
	winmo_cct_shading_comp_struct *pShadingComp;
}ACDK_CCT_SHADING_COMP_STRUCT, *PACDK_CCT_SHADING_COMP_STRUCT;


/* R/W ISP/Sensor Register */
#if 0
typedef struct
{
	ACDK_CCT_REG_TYPE_ENUM Type;
	UINT32 RegAddr;
	UINT32 RegData;
}	ACDK_CCT_REG_RW_STRUCT, *PACDK_CCT_REG_RW_STRUCT;
#endif




typedef struct
{
    UINT32 u4Eposuretime;   //!<: Exposure time in ms
    UINT32 u4GainMode; // 0: AfeGain; 1: ISO
    UINT32 u4AfeGain;       //!<: AFE digital gain
    UINT32 u4IspGain;      // !< Raw gain
    UINT32 u4ISO;
    UINT16  u2FrameRate;
    UINT16  u2FlareGain;   //128 base
    UINT16  u2FlareValue;
    UINT16  u2CaptureFlareGain;
    UINT16  u2CaptureFlareValue;
    MBOOL   bFlareAuto;
}ACDK_AE_MODE_CFG_T;

#if 0     //@Sean move to kd_imgsensor_define.h
typedef enum
{
	ACDK_SCENARIO_ID_CAMERA_PREVIEW=0,
	ACDK_SCENARIO_ID_VIDEO_PREVIEW,
	ACDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4,
	ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,
	ACDK_SCENARIO_ID_CAMERA_CAPTURE_MEM,
	ACDK_SCENARIO_ID_CAMERA_BURST_CAPTURE_JPEG,
	ACDK_SCENARIO_ID_VIDEO_DECODE_MPEG4,
	ACDK_SCENARIO_ID_VIDEO_DECODE_H263,
	ACDK_SCENARIO_ID_VIDEO_DECODE_H264,
	ACDK_SCENARIO_ID_VIDEO_DECODE_WMV78,
	ACDK_SCENARIO_ID_VIDEO_DECODE_WMV9,
	ACDK_SCENARIO_ID_VIDEO_DECODE_MPEG2,
	ACDK_SCENARIO_ID_IMAGE_YUV2RGB,
	ACDK_SCENARIO_ID_IMAGE_RESIZE,
	ACDK_SCENARIO_ID_IMAGE_ROTATE,
	ACDK_SCENARIO_ID_IMAGE_POST_PROCESS,
	ACDK_SCENARIO_ID_JPEG_RESIZE,
	ACDK_SCENARIO_ID_JPEG_DECODE,
	ACDK_SCENARIO_ID_JPEG_PARSE,
	ACDK_SCENARIO_ID_JPEG_ENCODE,
	ACDK_SCENARIO_ID_JPEG_ENCODE_THUMBNAIL,
	ACDK_SCENARIO_ID_DRIVER_IO_CONTROL,
	ACDK_SCENARIO_ID_DO_NOT_CARE,
	ACDK_SCENARIO_ID_IMAGE_DSPL_BUFFER_ALLOC,
	ACDK_SCENARIO_ID_TV_OUT,
	ACDK_SCENARIO_ID_MAX,
	ACDK_SCENARIO_ID_VIDOE_ENCODE_WITHOUT_PREVIEW, 		// for LTK test case
	ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG_BACK_PREVIEW,  // for LTK test case
	ACDK_SCENARIO_ID_VIDEO_DECODE_RV8,
	ACDK_SCENARIO_ID_VIDEO_DECODE_RV9,
}	ACDK_SCENARIO_ID_ENUM;
#endif


#if 0     //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	ACDK_SENSOR_IMAGE_NORMAL=0,
	ACDK_SENSOR_IMAGE_H_MIRROR,
	ACDK_SENSOR_IMAGE_V_MIRROR,
	ACDK_SENSOR_IMAGE_HV_MIRROR
}ACDK_SENSOR_IMAGE_MIRROR_ENUM;
#endif

#if 0     //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW=0,
	ACDK_SENSOR_OPERATION_MODE_VIDEO,
	ACDK_SENSOR_OPERATION_MODE_STILL_CAPTURE,
	ACDK_SENSOR_OPERATION_MODE_WEB_CAPTURE,
	ACDK_SENSOR_OPERATION_MODE_MAX
} ACDK_SENSOR_OPERATION_MODE_ENUM;
#endif

#if 0     //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	ACDK_CAMERA_OPERATION_NORMAL_MODE=0,
	ACDK_CAMERA_OPERATION_META_MODE
} ACDK_CAMERA_OPERATION_MODE_ENUM;
#endif


/* ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3, ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3 */
typedef struct
{
	CAMERA_TUNING_SET_ENUM		Mode;
	UINT32 	Length;
	UINT32 	Offset;
	UINT8	ColorTemp;
	UINT32	*pBuffer;
}ACDK_CCT_TABLE_SET_STRUCT, *PACDK_CCT_TABLE_SET_STRUCT;

/* ACDK_CCT_V2_OP_SHADING_CAL */
typedef struct
{
    CAMERA_TUNING_SET_ENUM mode;
    UINT32 boundaryStartX;
    UINT32 boundaryStartY;
    UINT32 boundaryEndX;
    UINT32 boundaryEndY;
    UINT32 attnRatio;
    UINT8   colorTemp;
    UINT8   u1FixShadingIndex;
}ACDK_CCT_LSC_CAL_SET_STRUCT, *PACDK_CCT_LSC_CAL_SET_STRUCT;

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	ACDK_SENSOR_IMAGE_MIRROR_ENUM	SensorImageMirror;
	MBOOL	EnableShutterTansfer;			/* capture only */
	MBOOL	EnableFlashlightTansfer;		/* flash light capture only */
	ACDK_SENSOR_OPERATION_MODE_ENUM	SensorOperationMode;
	UINT16  ImageTargetWidth;		/* image captured width */
	UINT16  ImageTargetHeight;		/* image captuerd height */
	UINT16	CaptureShutter;			/* capture only */
	UINT16	FlashlightDuty;			/* flash light capture only */
	UINT16	FlashlightOffset;		/* flash light capture only */
	UINT16	FlashlightShutFactor;	/* flash light capture only */
	UINT16 	FlashlightMinShutter;
	ACDK_CAMERA_OPERATION_MODE_ENUM 	MetaMode; /* capture only */
	UINT32  DefaultPclk;       // Sensor pixel clock(Ex:24000000)
	UINT32  Pixels;             // Sensor active pixel number
	UINT32  Lines;              // Sensor active line number
	UINT32  Shutter;            // Sensor current shutter
	UINT32  FrameLines;      //valid+dummy lines for minimum shutter
}	ACDK_SENSOR_CONFIG_STRUCT;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	SENSOR_MIPI_1_LANE=0,
	SENSOR_MIPI_2_LANE,
	SENSOR_MIPI_4_LANE
} ACDK_SENSOR_MIPI_LANE_NUMBER_ENUM;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef enum
{
	SENSOR_INTERFACE_TYPE_PARALLEL=0,
	SENSOR_INTERFACE_TYPE_MIPI,
	SENSOR_INTERFACE_TYPE_MAX
} ACDK_SENSOR_INTERFACE_TYPE_ENUM;
#endif

#if 0   //@Sean Move to kd_imgsensor_define.h
typedef struct
{
	UINT16 SensorPreviewWidth;
	UINT16 SensorPreviewHeight;
	UINT16 SensorFullWidth;
	UINT16 SensorFullHeight;
}	ACDK_SENSOR_RESOLUTION_INFO_STRUCT, *PACDK_SENSOR_RESOLUTION_INFO_STRUCT;
#endif

// defined the enum for enumerating the ISO/Binning information about each ISO mode.
#if 0    //@Sean move to kd_imgsensor_define.h
typedef enum
{
	ISO_100_MODE =0,
	ISO_200_MODE,
	ISO_400_MODE,
	ISO_800_MODE,
	ISO_1600_MODE,
	ISO_MAX_MODE
} ACDK_ISP_ISO_ENUM;
#endif

#if 0    //@Sean move to kd_imgsensor_define.h
typedef struct
{
	UINT32 				MaxWidth;
	UINT32				MaxHeight;
	MBOOL			    ISOSupported;
	MBOOL				BinningEnable;
} ACDK_ISP_BINNING_INFO_STRUCT, *PACDK_ISP_BINNING_INFO_STRUCT;
#endif

#if 0    //@Sean move to kd_imgsensor_define.h
typedef struct
{
	ACDK_ISP_BINNING_INFO_STRUCT	ISOBinningInfo[ISO_MAX_MODE];
} CAMERA_ISO_BINNING_INFO_STRUCT, *PCAMERA_ISO_BINNING_INFO_STRUCT;
#endif

#if 0    //@Sean move to kd_imgsensor_define.h
typedef struct
{
	UINT16 SensorPreviewResolutionX;
	UINT16 SensorPreviewResolutionY;
	UINT16 SensorFullResolutionX;
	UINT16 SensorFullResolutionY;
	UINT8 SensorClockFreq;				/* MHz */
	UINT8 SensorCameraPreviewFrameRate;
	UINT8 SensorVideoFrameRate;
	UINT8 SensorStillCaptureFrameRate;
	UINT8 SensorWebCamCaptureFrameRate;
	UINT8 SensorClockPolarity;			/* SENSOR_CLOCK_POLARITY_HIGH/SENSOR_CLOCK_POLARITY_Low */
	UINT8 SensorClockFallingPolarity;
	UINT8 SensorClockRisingCount;		/* 0..15 */
	UINT8 SensorClockFallingCount;		/* 0..15 */
	UINT8 SensorClockDividCount;		/* 0..15 */
	UINT8 SensorPixelClockCount;		/* 0..15 */
	UINT8 SensorDataLatchCount;			/* 0..15 */
	UINT8 SensorHsyncPolarity;
	UINT8 SensorVsyncPolarity;
	UINT8 SensorInterruptDelayLines;
	MBOOL  SensorResetActiveHigh;
	UINT32 SensorResetDelayCount;
	ACDK_SENSOR_INTERFACE_TYPE_ENUM SensroInterfaceType;
	ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM SensorOutputDataFormat;
	ACDK_SENSOR_MIPI_LANE_NUMBER_ENUM SensorMIPILandNumber;
	CAMERA_ISO_BINNING_INFO_STRUCT  SensorISOBinningInfo;
       UINT32 CaptureDelayFrame;
       UINT32 PreviewDelayFrame;
       UINT32 VideoDelayFrame;
       UINT16 SensorGrabStartX;
       UINT16 SensorGrabStartY;
       UINT16 SensorDrivingCurrent;
       UINT8   SensorMasterClockSwitch;
	UINT8   AEShutDelayFrame;		  /* The frame of setting shutter default 0 for TG int */
	UINT8   AESensorGainDelayFrame;	 /* The frame of setting sensor gain */
       UINT8   AEISPGainDelayFrame;
}	ACDK_SENSOR_INFO_STRUCT, *PACDK_SENSOR_INFO_STRUCT;
#endif

#if 0    //@Sean move to kd_imgsensor_define.h
typedef enum
{
	ISP_DRIVING_2MA=0,
	ISP_DRIVING_4MA,
	ISP_DRIVING_6MA,
	ISP_DRIVING_8MA
} ISP_DRIVING_CURRENT_ENUM;
#endif

#if 0    //@Sean move to kd_imgsensor_define.h
typedef struct{
    ACDK_SCENARIO_ID_ENUM ScenarioId;
    ACDK_SENSOR_INFO_STRUCT *pInfo;
    ACDK_SENSOR_CONFIG_STRUCT *pConfig;
}ACDK_SENSOR_GETINFO_STRUCT, *PACDK_SENSOR_GETINFO_STRUCT;
#endif
#if 0   //@Sean move to kd_imgsensor_define.h
typedef enum
typedef struct{
    ACDK_SENSOR_FEATURE_ENUM FeatureId;
    UINT8  *pFeaturePara;
    UINT32 *pFeatureParaLen;
}ACDK_SENSOR_FEATURECONTROL_STRUCT, *PACDK_SENSOR_FEATURECONTROL_STRUCT;
#endif

#if 0   //@Sean move to kd_imgsensor_define.h
typedef struct
{
	UINT16 GrabStartX;				/* The first grabed column data of the image sensor in pixel clock count */
	UINT16 GrabStartY;				/* The first grabed row data of the image sensor in pixel clock count */
	UINT16 ExposureWindowWidth;		/* Exposure window width of image sensor */
	UINT16 ExposureWindowHeight;	/* Exposure window height of image sensor */
	UINT16 ImageTargetWidth;		/* image captured width */
	UINT16 ImageTargetHeight;		/* image captuerd height */
	UINT16 ExposurePixel;			/* exposure window width of image sensor + dummy pixel */
	UINT16 CurrentExposurePixel;	/* exposure window width of image sensor + dummy pixel */
	UINT16 ExposureLine;			/* exposure window width of image sensor + dummy line */
	UINT16 ZoomFactor;				/* digital zoom factor */
} ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT;
#endif

#if 0   //@Sean move to kd_imgsensor_define.h
typedef struct{
    ACDK_SCENARIO_ID_ENUM ScenarioId;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow;
	ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData;
}ACDK_SENSOR_CONTROL_STRUCT;
#endif

typedef struct
{
	INT32 i4AFMode;
	INT32 i4AFMeter;
	INT32 i4CurrPos;

} ACDK_AF_INFO_T;

#define AF_TABLE_LENGTH 30

typedef struct
{
	INT32 i4Num;
	INT32 i4Pos[AF_TABLE_LENGTH];

} ACDK_AF_POS_T;

typedef struct
{
	INT32 i4Num;
	MINT64 i8Vlu[AF_TABLE_LENGTH];

} ACDK_AF_VLU_T;

// Data structure for CDVT

#define ACDK_CDVT_MAX_GAIN_TABLE_SIZE (1000)
#define ACDK_CDVT_MAX_TEST_COUNT (1000)

typedef enum
{
    ACDK_CDVT_TEST_EXPOSURE_LINEARITY = 0,
    ACDK_CDVT_TEST_GAIN_LINEARITY = 1,
    ACDK_CDVT_TEST_OB_STABILITY = 2
} ACDK_CDVT_TEST_ITEM_T;

typedef enum
{
    ACDK_CDVT_CALIBRATION_OB = 0,
    ACDK_CDVT_CALIBRATION_MIN_ISO = 1,
    ACDK_CDVT_CALIBRATION_MIN_SAT_GAIN = 2
} ACDK_CDVT_CALIBRATION_ITEM_T;

typedef enum
{
    ACDK_CDVT_SENSOR_MODE_PREVIEW = 0,
    ACDK_CDVT_SENSOR_MODE_CAPTURE = 1,
    ACDK_CDVT_SENSOR_MODE_VIDEO = 2
} ACDK_CDVT_SENSOR_MODE_T;

typedef enum
{
    ACDK_CDVT_EXP_MODE_LINE = 0,
    ACDK_CDVT_EXP_MODE_TIME = 1
} ACDK_CDVT_EXP_MODE_T;

typedef enum
{
    ACDK_CDVT_GAIN_CONFIG = 0,
    ACDK_CDVT_GAIN_TABLE = 1
} ACDK_CDVT_GAIN_CONTROL_MODE_T;

typedef enum
{
    ACDK_CDVT_FLICKER_50_HZ = 0,
    ACDK_CDVT_FLICKER_60_HZ = 1
} ACDK_CDVT_FLICKER_T;

typedef struct
{
    ACDK_CDVT_EXP_MODE_T eExpMode;
    INT32 i4Gain;
    INT32 i4ExpStart;
    INT32 i4ExpEnd;
    INT32 i4ExpInterval;
} ACDK_CDVT_EXP_LINEARITY_TEST_T;

typedef struct
{
    ACDK_CDVT_GAIN_CONTROL_MODE_T eGainControlMode;
    INT32 i4ExpTime;
    INT32 i4GainStart;
    INT32 i4GainEnd;
    INT32 i4GainInterval;
    INT32 i4GainTableSize;
    INT32 i4GainTable[ACDK_CDVT_MAX_GAIN_TABLE_SIZE];
} ACDK_CDVT_GAIN_LINEARITY_OB_STABILITY_TEST_T;

typedef struct
{
    INT32 i4ExpTime;
    INT32 i4Gain;
    INT32 i4RepeatTimes;
} ACDK_CDVT_OB_CALIBRATION_T;

typedef struct
{
    ACDK_CDVT_FLICKER_T eFlicker;
    INT32 i4LV;
    INT32 i4FNumber;
    INT32 i4OB;
} ACDK_CDVT_MIN_ISO_CALIBRATION_T;

typedef struct
{
    ACDK_CDVT_FLICKER_T eFlicker;
    INT32 i4TargetDeclineRate;
    INT32 i4GainBuffer;
    INT32 i4OB;
} ACDK_CDVT_MIN_SAT_GAIN_CALIBRATION_T;

typedef struct
{
    ACDK_CDVT_TEST_ITEM_T eTestItem;
    ACDK_CDVT_SENSOR_MODE_T eSensorMode;
    ACDK_CDVT_EXP_LINEARITY_TEST_T rExpLinearity;
    ACDK_CDVT_GAIN_LINEARITY_OB_STABILITY_TEST_T rGainLinearityOBStability;
} ACDK_CDVT_SENSOR_TEST_INPUT_T;

typedef struct
{
    // average
    FLOAT fRAvg;
    FLOAT fGrAvg;
    FLOAT fGbAvg;
    FLOAT fBAvg;

    // median
    UINT32 u4Median;
} ACDK_CDVT_RAW_ANALYSIS_RESULT_T;

typedef struct
{
    INT32 i4ErrorCode;
    INT32 i4TestCount;
    ACDK_CDVT_RAW_ANALYSIS_RESULT_T rRAWAnalysisResult[ACDK_CDVT_MAX_TEST_COUNT];
} ACDK_CDVT_SENSOR_TEST_OUTPUT_T;

typedef struct
{
    ACDK_CDVT_CALIBRATION_ITEM_T eCalibrationItem;
    ACDK_CDVT_SENSOR_MODE_T eSensorMode;
    ACDK_CDVT_OB_CALIBRATION_T rOB;
    ACDK_CDVT_MIN_ISO_CALIBRATION_T rMinISO;
    ACDK_CDVT_MIN_SAT_GAIN_CALIBRATION_T rMinSatGain;
} ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T;

typedef struct
{
    INT32 i4ErrorCode;
    INT32 i4OB;
    INT32 i4MinISO;
    INT32 i4MinSatGain;
} ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T;

typedef struct
{
    INT32 i4Num;
    INT32 i4Gap;
    INT32 i4BestPos;
    MINT64 i8Vlu[512];

} ACDK_AF_CALI_DATA_T;

//
typedef struct
{
    UINT32 level;
    UINT32 duration;
}ACDK_FLASH_CONTROL;

// Light source probability
#define ACDK_AWB_LIGHT_NUM (8)
typedef struct
{
	MUINT32 u4P0[ACDK_AWB_LIGHT_NUM]; // Probability 0
	MUINT32 u4P1[ACDK_AWB_LIGHT_NUM]; // Probability 1
	MUINT32 u4P2[ACDK_AWB_LIGHT_NUM]; // Probability 2
	MUINT32 u4P[ACDK_AWB_LIGHT_NUM];  // Probability
} ACDK_AWB_LIGHT_PROBABILITY_T;

typedef enum
{
    CAMERA_NVRAM_DEFECT_STRUCT = 0,          // NVRAM_CAMERA_DEFECT_STRUCT
    CAMERA_NVRAM_SHADING_STRUCT,              // NVRAM_CAMERA_SHADING_STRUCT
    CAMERA_NVRAM_3A_STRUCT,                         // NVRAM_CAMERA_3A_T
    CAMERA_NVRAM_ISP_PARAM_STRUCT,        // NVRAM_CAMERA_ISP_PARAM_STRUCT
} CAMERA_NVRAM_STRUCTURE_ENUM;

/* ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA */
typedef struct
{
	CAMERA_NVRAM_STRUCTURE_ENUM		Mode;
	UINT32	*pBuffer;
}ACDK_CCT_NVRAM_SET_STRUCT, *PACDK_CCT_NVRAM_SET_STRUCT;
#endif

