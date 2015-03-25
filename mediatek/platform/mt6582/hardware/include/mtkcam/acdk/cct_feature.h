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
    ACDK_CCT_OP_3A_LOAD_FROM_NVRAM, //Yosen
    ACDK_CCT_OP_ISP_SAVE_TO_NVRAM,
    //Shading NVRAM
    ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM,
    ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM,
    //PCA
    ACDK_CCT_OP_ISP_SET_PCA_TABLE,
    ACDK_CCT_OP_ISP_GET_PCA_TABLE,
    ACDK_CCT_OP_ISP_SET_PCA_PARA,
    ACDK_CCT_OP_ISP_GET_PCA_PARA,
    ACDK_CCT_OP_ISP_SET_PCA_SLIDER,
    ACDK_CCT_OP_ISP_GET_PCA_SLIDER,

    // TSF
    ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE,

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
    ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
    ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
    ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION,
    ACDK_CCT_OP_DEV_AE_SET_TARGET,

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
    ACDK_CCT_AF_INIT, //for CctImp::init() use
    ACDK_CCT_AF_UNINIT, //for CctImp::uninit() use

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

	ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META, //20
	ACDK_CCT_OP_STROBE_SET_NVDATA_META,       //21   for cct tool, 0, 1, 6, 20, 21



}ACDK_CCT_FEATURE_ENUM;


typedef struct
{
	int exp;
	int afe_gain;
	int isp_gain;
	int distance;
	short yTab[256];  //x128
	short rgTab[256]; //1024 base
	short bgTab[256]; //1024 base
	short rsv[400];

}ACDK_FLASH_CCT_ENG_TABLE;

typedef struct
{
	int yTar;
	int antiIsoLevel;
	int antiExpLevel;
	int antiStrobeLevel;
	int antiUnderLevel;
	int antiOverLevel;
	int foregroundLevel;
	int isRefAfDistance;
    int accuracyLevel;

} ACDK_FLASH_TUNING_PARA;

typedef struct
{
	//torch, video
	int torchDuty;
	int torchStep;
	//AF
	int afDuty;
	int afStep;
	//pf, mf
	//normal bat setting
	int pfDuty;
	int mfDutyMax;
	int mfDutyMin;
	int pmfStep;
	//low bat setting
	int IChangeByVBatEn;
	int vBatL;	//mv
	int pfDutyL;
	int mfDutyMaxL;
	int mfDutyMinL;
	int pmfStepL;
	//burst setting
	int IChangeByBurstEn;
	int pfDutyB;
	int mfDutyMaxB;
	int mfDutyMinB;
	int pmfStepB;
}
ACDK_FLASH_ENG_LEVEL;

typedef struct
{
    UINT32 u4Version;

	ACDK_FLASH_CCT_ENG_TABLE engTab;
	ACDK_FLASH_TUNING_PARA tuningPara[6];

	int isTorchEngUpdate;
	int isAfEngUpdate;
	int isNormaEnglUpdate;
	int isLowBatEngUpdate;
	int isBurstEngUpdate;
	ACDK_FLASH_ENG_LEVEL engLevel;
	//NVRAM_FLASH_EXTENSION ext;
    //UINT8 reserved[MAXIMUM_NVRAM_CAMERA_DEFECT_FILE_SIZE-sizeof(UINT32)-sizeof(NVRAM_FLASH_CCT_ENG_TABLE)-sizeof(NVRAM_FLASH_TUNING_PARA)*6-sizeof(int)*5-sizeof(NVRAM_FLASH_ENG_LEVEL)-sizeof(NVRAM_FLASH_EXTENSION)];

} ACDK_STROBE_STRUCT, *PACDK_STROBE_STRUCT;


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
    ACDK_CCT_OP_AE_PLINE_TABLE_TEST,       // AE Pline table linearity test command.
    ACDK_CCT_CDVT_END
} ACDK_CCT_CDVT_ENUM;


typedef enum
{
    OUTPUT_PURE_RAW8 = 0,
    OUTPUT_PURE_RAW10,
    OUTPUT_PROCESSED_RAW8,
    OUTPUT_PROCESSED_RAW10,
    OUTPUT_YUV,
    OUTPUT_JPEG
} ACDK_CCT_CAP_OUTPUT_FORMAT;

typedef struct MULTI_SHOT_CAPTURE_STRUCT_S
{
    eACDK_CAMERA_MODE eCameraMode;
    eACDK_OPERA_MODE eOperaMode;
    ACDK_CCT_CAP_OUTPUT_FORMAT eOutputFormat;
    UINT16 u2JPEGEncWidth;
    UINT16 u2JPEGEncHeight;
    UINT32 u4CapCount;
    Func_CB fpCapCB;
    MINT32 i4IsSave; //0-don't save, 1-save
public :
    MULTI_SHOT_CAPTURE_STRUCT_S(eACDK_CAMERA_MODE a_camMode = CAPTURE_MODE,
                                                  eACDK_OPERA_MODE a_operaMode = ACDK_OPT_META_MODE,
                                                  ACDK_CCT_CAP_OUTPUT_FORMAT a_outFormat = OUTPUT_PURE_RAW10,
                                                  UINT16 a_jpegEncWdth = 0,
                                                  UINT16 a_jpegEncHeight = 0,
                                                  UINT32 a_capCnt = 1,
                                                  Func_CB a_funCB = NULL,
                                                  MINT32 a_isSave = 0)
                                                    : eCameraMode(a_camMode),
                                                      eOperaMode(a_operaMode),
                                                      eOutputFormat(a_outFormat),
                                                      u2JPEGEncWidth(a_jpegEncWdth),
                                                      u2JPEGEncHeight(a_jpegEncHeight),
                                                      u4CapCount(a_capCnt),
                                                      fpCapCB(a_funCB),
                                                      i4IsSave(a_isSave)

    {
    }
}ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT, *PACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT;


typedef struct STILL_CAPTURE_STRUCT_S
{
    eACDK_CAMERA_MODE eCameraMode;
    eACDK_OPERA_MODE eOperaMode;
    ACDK_CCT_CAP_OUTPUT_FORMAT eOutputFormat;
    UINT16 u2JPEGEncWidth;
    UINT16 u2JPEGEncHeight;
    Func_CB fpCapCB;
    MINT32 i4IsSave; //0-don't save, 1-save
public :
    STILL_CAPTURE_STRUCT_S(eACDK_CAMERA_MODE a_camMode = CAPTURE_MODE,
                                         eACDK_OPERA_MODE a_operaMode = ACDK_OPT_META_MODE,
                                         ACDK_CCT_CAP_OUTPUT_FORMAT a_outFormat = OUTPUT_PURE_RAW10,
                                         UINT16 a_jpegEncWdth = 0,
                                         UINT16 a_jpegEncHeight = 0,
                                         Func_CB a_funCB = NULL,
                                         MINT32 a_isSave = 0)
                                                    : eCameraMode(a_camMode),
                                                      eOperaMode(a_operaMode),
                                                      eOutputFormat(a_outFormat),
                                                      u2JPEGEncWidth(a_jpegEncWdth),
                                                      u2JPEGEncHeight(a_jpegEncHeight),
                                                      fpCapCB(a_funCB),
                                                      i4IsSave(a_isSave)

    {
    }
}ACDK_CCT_STILL_CAPTURE_STRUCT, *PACDK_CCT_STILL_CAPTURE_STRUCT;

typedef struct
{
    UINT16 u2PreviewWidth;
    UINT16 u2PreviewHeight;
    Func_CB fpPrvCB;
}ACDK_CCT_CAMERA_PREVIEW_STRUCT, *PACDK_CCT_CAMERA_PREVIEW_STRUCT;


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
    EIsp_Category_SL2,
    //EIsp_Category_NR3D,
    //EIsp_Category_MFB,
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
    ISP_NVRAM_SL2_T             SL2[NVRAM_SL2_TBL_NUM];
	//ISP_NVRAM_NR3D_T			NR3D[NVRAM_NR3D_TBL_NUM];
	//ISP_NVRAM_MFB_T 			MFB[NVRAM_MFB_TBL_NUM];
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

struct ACDK_CCT_ACCESS_PCA_SLIDER
{
    ISP_NVRAM_PCA_SLIDER_STRUCT slider;
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

/* CCM light mode */
/* MSDK_CCT_V2_OP_AWB_GET_NVRAM_CCM; MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM */
typedef struct {
    WINMO_CCT_CCM_STRUCT    CCM;
    UINT8    CCMLightMode;
}ACDK_CCT_CCM_LIGHTMODE_STRUCT, *PACDK_CCT_CCM_LIGHTMODE_STRUCT;

typedef struct {
    MBOOL dynamicCCMEn;
}ACDK_CCT_CCM_STATUS_STRUCT, *PACDK_CCT_CCM_STATUS_STRUCT;

#define GAMMA_STEP_NO 144
#define TOTAL_GAMMA_NO 1

typedef struct
{
	UINT16 r_tbl[GAMMA_STEP_NO];
	UINT16 g_tbl[GAMMA_STEP_NO];
	UINT16 b_tbl[GAMMA_STEP_NO];
} ACDK_CCT_GAMMA_TABLE_STRUCT, *PACDK_CCT_GAMMA_TABLE_STRUCT;


/* ISP or 3A function enable */
/* MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF */
/* MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS; MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG */
typedef struct {
	MBOOL						Enable;
}ACDK_CCT_FUNCTION_ENABLE_STRUCT, *PACDK_CCT_FUNCTION_ENABLE_STRUCT;

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

typedef struct
{
	UINT32 u4ExposureMode;
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

typedef struct
{
	INT32 i4Num;
	INT32 i4Vlu[AF_TABLE_LENGTH*2];

} ACDK_AF_VLU_32T;

// Data structure for CDVT

#define ACDK_CDVT_MAX_GAIN_TABLE_SIZE (1000)
#define ACDK_CDVT_MAX_TEST_COUNT (1000)
#define ACDK_CDVT_AEPLINE_MAX_TEST_COUNT (800)

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
    ACDK_CDVT_EXP_MODE_TIME = 0,
    ACDK_CDVT_EXP_MODE_LINE = 1
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
    INT32 i4Index;
    INT32 i4ShutterTime;
    INT32 i4SensorGain;
    INT32 i4ISPGain;
} ACDK_CDVT_AE_TABLE_T;

typedef struct
{
    INT32 i4ShutterDelayFrame;
    INT32 i4SensorGainDelayFrame;
    INT32 i4ISPGainDelayFrame;
    INT32 i4TestSteps;
    INT32 i4RepeatTime;
    INT32 i4IndexScanStart;
    INT32 i4IndexScanEnd;    
    ACDK_CDVT_AE_TABLE_T i4PlineTable[ACDK_CDVT_AEPLINE_MAX_TEST_COUNT];
} ACDK_CDVT_AE_PLINE_TABLE_LINEARITY_TEST_T;

typedef struct
{
    ACDK_CDVT_SENSOR_MODE_T eSensorMode;
    ACDK_CDVT_AE_PLINE_TABLE_LINEARITY_TEST_T rAEPlinetableInfo;
} ACDK_CDVT_AE_PLINE_TEST_INPUT_T;

typedef struct
{
    INT32 i4Index;
    INT32 i4ShutterTime;
    INT32 i4SensorGain;
    INT32 i4ISPGain;
    INT32 i4Yvalue;
} ACDK_CDVT_Y_ANALYSIS_RESULT_T;

typedef struct
{
    INT32 i4ErrorCode;
    INT32 i4TestCount;
    ACDK_CDVT_Y_ANALYSIS_RESULT_T rYAnalysisResult[ACDK_CDVT_AEPLINE_MAX_TEST_COUNT];
} ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T;

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
    CAMERA_NVRAM_LENS_STRUCT,
} CAMERA_NVRAM_STRUCTURE_ENUM;

/* ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA */
typedef struct
{
	CAMERA_NVRAM_STRUCTURE_ENUM		Mode;
	UINT32	*pBuffer;
}ACDK_CCT_NVRAM_SET_STRUCT, *PACDK_CCT_NVRAM_SET_STRUCT;
#endif

