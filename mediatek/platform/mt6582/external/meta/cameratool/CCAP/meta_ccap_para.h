/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef __META_CCAP_PARA_H_
#define __META_CCAP_PARA_H_	



//#include "stdafx.h"
//#include "MSDK_CCT_Feature_exp.h"
//#include "MSDK_exp.h"
#include "meta.h"
//#include "MSDK_ISP_exp.h"
#include "FT_Public.h"
//#include "MSDK_Sensor_exp.h"

#include "cct_ErrCode.h"
#include <mtkcam/acdk/MdkIF.h>
#include <mtkcam/acdk/CctIF.h>

#include "camera_custom_nvram.h" // replace "msdk_nvram_camera_exp.h"
#include "meta_common.h"
#include <mtkcam/acdk/cct_feature.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIA_PATH "//sdcard"
#define PROJECT_NAME "Yusu"

#define FT_CNF_OK     0
#define FT_CNF_FAIL   1
//for adapet WinMo
#define cct_ccm_matrix_struct  WINMO_CCT_CCM_STRUCT
#define cct_awb_gain_struct AWB_GAIN_T

#define FT_CCT_ERR_TGT_NO_MEM_FOR_SINGLE_SHOT			0xFF
#define FT_CCT_ERR_TGT_NO_MEM_FOR_MULTI_SHOT			0xFE
#define FT_CCT_ERR_INVALID_SENSOR_ID					0xFD
#define FT_CCT_ERR_NOT_IMPLEMENT_YET					0xFC
#define FT_CCT_ERR_PREVIEW_ALREADY_STARTED				0xFB
#define FT_CCT_ERR_PREVIEW_ALREADY_STOPPED				0xFA
#define FT_CCT_ERR_CANCEL_IMAGE_SENDING					0xF9
#define FT_CCT_ERR_TST_QUEUE_FULL						0xF8
#define FT_CCT_ERR_READ_NVRAM_FAIL						0xF7
#define FT_CCT_ERR_WRITE_NVRAM_FAIL						0xF6
#define FT_CCT_ERR_AE_INVALID_EC_LEVEL					0xF5
#define FT_CCT_ERR_AE_INVALID_EC_STEP					0xF4
#define FT_CCT_ERR_AE_ALREADY_ENABLED					0xF3
#define FT_CCT_ERR_AE_ALREADY_DISABLED					0xF2
#define FT_CCT_ERR_WB_INVALID_INDEX						0xF1
#define FT_CCT_ERR_AE_IS_NOT_DISABLED					0xF0
#define FT_CCT_ERR_AFRESH_CAPTURE_FIRST					0xEF
#define FT_CCT_ERR_CAPTURE_WIDTH_HEIGHT_TOO_SMALL		0xEE
#define FT_CCT_ERR_SENSOR_ENG_SET_INVALID_VALUE			0xED
#define FT_CCT_ERR_SENSOR_ENG_GROUP_NOT_EXIST			0xEC
#define FT_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST			0xEB
#define FT_CCT_ERR_INVALID_COMPENSATION_MODE			0xEA
#define FT_CCT_ERR_USB_COM_NOT_READY					0xE9
#define FT_CCT_DEFECTPIXEL_CAL_UNDER_PROCESSING			0xE8
#define FT_CCT_ERR_DEFECTPIXEL_CAL_NO_MEM				0xE7
#define	FT_CCT_ERR_TOO_MANY_DEFECT_PIXEL				0xE6
#define FT_CCT_ERR_CAPTURE_JPEG_FAIL					0xE5
#define FT_CCT_ERR_CAPTURE_JPEG_TIMEOUT					0xE4
#define FT_CCT_ERR_AF_FAIL								0xE3
#define FT_CCT_ERR_AF_TIMEOUT							0xE2
#define FT_CCT_ERR_AF_LENS_OFFSET_CAL_FAIL				0xE1
#define FT_CCT_ERR_PREVIEW_MUST_ENABLE					0xE0
#define FT_CCT_ERR_UNSUPPORT_CAPTURE_FORMAT				0xDF
#define FT_CCT_ERR_EXCEED_MAX_DEFECT_PIXEL				0xDE
#define FT_CCT_ERR_INVALID_WIDTH_FACTOR					0xDD
#define FT_CCT_ERR_PREVIEW_MUST_DISABLE					0xDC
#define FT_CCT_6238_ERR_AE_ALREADY_ENABLED				0xDB
#define FT_CCT_6238_ERR_AE_ALREADY_DISABLED				0xDA
#define FT_CCT_6238_ERR_AE_IS_NOT_DISABLED				0xD9
#define FT_CCT_ERR_SHADING_TABLE_SIZE					0xD8


#define FT_CCT_INTERNAL_BUF_SIZE		(0x2100)	// 2112 DWORDs 










	typedef enum 
	{
		FT_CCT_OP_REG_READ = 0
		,FT_CCT_OP_REG_WRITE
		,FT_CCT_OP_PREVIEW_LCD_START
		,FT_CCT_OP_PREVIEW_LCD_STOP
		,FT_CCT_OP_SINGLE_SHOT_CAPTURE
		,FT_CCT_OP_MULTI_SHOT_CAPTURE
		,FT_CCT_OP_QUERY_SENSOR
		,FT_CCT_OP_LOAD_FROM_NVRAM
		,FT_CCT_OP_SAVE_TO_NVRAM
		,FT_CCT_OP_AE_ENABLE
		,FT_CCT_OP_AE_DISABLE
		,FT_CCT_OP_AE_SET_EXPOSE_LEVEL
		,FT_CCT_OP_AE_GET_EXPOSE_LEVEL
		,FT_CCT_OP_AE_QUERY_EC_STEP_INFO
		,FT_CCT_OP_AE_SET_INIT_SHUTTER
		,FT_CCT_OP_AE_GET_INIT_SHUTTER
		,FT_CCT_OP_WB_ACTIVATE_BY_INDEX
		,FT_CCT_OP_WB_SET_BY_INDEX
		,FT_CCT_OP_WB_QUERY_ALL
		,FT_CCT_OP_AE_SET_MANUAL_SHUTTER
		,FT_CCT_OP_AE_GET_MANUAL_SHUTTER
		,FT_CCT_OP_GET_CAMERA_PARA_BUF
		,FT_CCT_OP_SINGLE_SHOT_CAPTURE_EX
		,FT_CCT_OP_MULTI_SHOT_CAPTURE_EX
		,FT_CCT_OP_GET_CAPTURE_BUF
		,FT_CCT_OP_RESUME_AE_AWB_PREVIEW_FROM_UNFINISHED_CAPTURE
		,FT_CCT_OP_GET_BANDING_FACTOR
		,FT_CCT_OP_GET_SENSOR_PREGAIN
		,FT_CCT_OP_SET_SENSOR_PREGAIN
		,FT_CCT_OP_MAIN_LCD_BACKLIGHT_SETTING
		,FT_CCT_OP_GET_GAMMA_TABLE
		,FT_CCT_OP_SET_GAMMA_TABLE
		,FT_CCT_OP_QUERY_ISP_ID
		,FT_CCT_OP_GET_CAL_STRUCT_BUF
		,FT_CCT_OP_GET_ENG_SENSOR_PARA
		,FT_CCT_OP_SET_ENG_SENSOR_PARA
		,FT_CCT_OP_GET_COMPENSATION_MODE
		,FT_CCT_OP_SET_COMPENSATION_MODE
		,FT_CCT_OP_GET_SHADING_PARA
		,FT_CCT_OP_SET_SHADING_PARA
		,FT_CCT_OP_GET_AUTODEFECT_PARA
		,FT_CCT_OP_SET_AUTODEFECT_PARA
		,FT_CCT_OP_GET_LAST_COMPENSATION_MODE
		,FT_CCT_OP_GET_AUTODEFECT_COUNT
		,FT_CCT_OP_SET_CAPTURE_DATA_TUNNEL
		,FT_CCT_OP_USB_TUNNEL_CANCEL
		,FT_CCT_OP_DEFECT_TABLE_VERIFY_BLOCK_FACTOR
		,FT_CCT_OP_DEFECT_TABLE_CAL					//47
		,FT_CCT_OP_DEV_AE_GET_INFO
		,FT_CCT_OP_DEV_AE_SET_TABLE_IDX
		,FT_CCT_OP_DEV_AE_SET_VALUE
		,FT_CCT_OP_DEV_AF_GET_TABLE
		,FT_CCT_OP_DEV_AF_CAL
		,FT_CCT_OP_DEV_AF_SET_TABLE_IDX
		,FT_CCT_OP_DEV_AF_SET_POS
		,FT_CCT_OP_DEV_STROBE_CAL_ENABLE
		,FT_CCT_OP_DEV_STROBE_CAL_SET_PARA
		,FT_CCT_OP_DEV_STROBE_CAL_DISABLE
		,FT_CCT_OP_ENABLE_USBCOM
		,FT_CCT_OP_AF_ON_OFF
		,FT_CCT_OP_DEFECT_TABLE_ON_OFF
		,FT_CCT_OP_AF_LENS_OFFSET_CAL
		,FT_CCT_OP_DEFECT_TABLE_BYPASS_BACKUP_SETTING
		,FT_CCT_OP_DEFECT_TABLE_RESTORE_SETTING
		,FT_CCT_OP_AE_GET_PERIOD_PARA
		,FT_CCT_OP_AE_SET_PERIOD_PARA
		,FT_CCT_OP_DEV_AE_BYPASS_FREERUN
		,FT_CCT_OP_DEV_AE_SET_SCENE_MODE
		,FT_CCT_OP_GET_GAMMA_TABLE_ON_OFF_FLAG
		,FT_CCT_OP_SET_GAMMA_TABLE_ON_OFF_FLAG
		,FT_CCT_OP_GET_GAMMA_ON_OFF_FLAG
		,FT_CCT_OP_SET_GAMMA_ON_OFF_FLAG
		,FT_CCT_OP_DEV_GET_ISO_GAIN
		,FT_CCT_OP_DEV_SET_ISO_GAIN
		,FT_CCT_OP_DEV_RECOVER_ISO_CAPTURE
		,FT_CCT_OP_DEV_SET_ISO_CAPTURE
		,FT_CCT_OP_DEV_GET_ISO_VALUE
		,FT_CCT_OP_DEV_SET_ISO_VALUE
		,FT_CCT_OP_DEV_FLASHLIGHT_CHARGE
		,FT_CCT_OP_DEV_FLASHLIGHT_STROBE
		,FT_CCT_OP_DEV_FLASHLIGHT_CONF
		,FT_CCT_OP_DEV_MODE_SIZE
		,FT_CCT_OP_DEV_FLASHLIGHT_TYPE
		,FT_CCT_OP_DEV_GET_ISO_GAIN_CCT
		,FT_CCT_OP_DEV_SET_ISO_GAIN_CCT
		,FT_CCT_OP_DEV_GET_ISO_VALUE_CCT
		,FT_CCT_OP_DEV_SET_ISO_VALUE_CCT
		,FT_CCT_OP_DEV_IF_SUPPORT_ISO
		,FT_CCT_OP_DEV_GET_FLASH_LEVEL
		,FT_CCT_OP_DEV_SET_FLASH_LEVEL
		,FT_CCT_OP_DEV_SET_FLASH_AE_INDEX
		,FT_CCT_OP_DEV_GET_FLASH_AE_INDEX	
		,FT_CCT_6238_OP_AE_ENABLE
		,FT_CCT_6238_OP_AE_DISABLE
		,FT_CCT_6238_OP_AE_SET_SCENE_MODE
		,FT_CCT_6238_OP_AE_SET_METERING_MODE
		,FT_CCT_6238_OP_AE_GET_CURRENT_EXPO_INFO
		,FT_CCT_6238_OP_AE_APPLY_EXPO_INFO
		,FT_CCT_6238_OP_AE_SELECT_BAND
		,FT_CCT_6238_OP_AE_GET_AUTO_EXPO_PARA
		,FT_CCT_6238_OP_AE_GET_ISO_VALUE_GAIN
		,FT_CCT_6238_OP_AE_GET_GAMMA_PARA
		,FT_CCT_6238_OP_AE_GET_GAMMA_TABLE
		,FT_CCT_6238_OP_AE_GET_FLARE_PARA
		,FT_CCT_6238_OP_AE_UPDATE_AUTO_EXPO_PARA
		,FT_CCT_6238_OP_AE_UPDATE_ISO_VALUE_GAIN
		,FT_CCT_6238_OP_AE_UPDATE_GAMMA_PARA
		,FT_CCT_6238_OP_AE_UPDATE_FLARE_PARA
		,FT_CCT_6238_OP_AE_GET_HISTOGRAM
		,FT_CCT_6238_OP_AE_GET_METERING_RESULT
		,FT_CCT_6238_OP_AE_GET_METERING_MODE_SETTING
		,FT_CCT_6238_OP_AE_UPDATE_METERING_MODE_SETTING
		,FT_CCT_6238_OP_AE_GET_WINDOW_HISTOGRAM
		,FT_CCT_6238_OP_AE_GET_SMOOTH_MODE_SETTING
		,FT_CCT_6238_OP_AE_UPDATE_SMOOTH_MODE_SETTING
		,FT_CCT_6238_OP_AE_ENABLE_PREVIEW_LOG
		,FT_CCT_6238_OP_AE_DISABLE_PREVIEW_LOG	
		,FT_CCT_6238_OP_AE_GET_PREVIEW_EXPO_INFO
		,FT_CCT_6238_OP_AWB_ENABLE_PREF_GAIN
		,FT_CCT_6238_OP_AWB_DISABLE_PREF_GAIN
		,FT_CCT_6238_OP_AWB_ENABLE_FAST_CONVERGE
		,FT_CCT_6238_OP_AWB_DISABLE_FAST_CONVERGE
		,FT_CCT_6238_OP_AWB_ENABLE_AUTO_RUN
		,FT_CCT_6238_OP_AWB_DISABLE_AUTO_RUN	
		,FT_CCT_6238_OP_AWB_ENABLE_SMALL_BOX
		,FT_CCT_6238_OP_AWB_DISABLE_SMALL_BOX
		,FT_CCT_6238_OP_AWB_GET_WINDOW_WHIT_POINT_COUNT_ARRAY
		,FT_CCT_6238_OP_AWB_GET_LIGHT_MODE
		,FT_CCT_6238_OP_AWB_GET_GAIN
		,FT_CCT_6238_OP_AWB_SET_GAIN
		,FT_CCT_6238_OP_AWB_SET_PREF_FACTOR
		,FT_CCT_6238_OP_AWB_GET_PREF_FACTOR
		,FT_CCT_6238_OP_AWB_ENABLE_DYNAMIC_CCM
		,FT_CCT_6238_OP_AWB_DISABLE_DYNAMIC_CCM
		,FT_CCT_6238_OP_AWB_GET_CURRENT_CCM
		,FT_CCT_6238_OP_AWB_ENABLE_FLASH_SYNC
		,FT_CCT_6238_OP_AWB_DISABLE_FLASH_SYNC
		,FT_CCT_6238_OP_AWB_APPLY_CAMERA_PARA2
		,FT_CCT_6238_OP_AWB_UPDATE_CAMERA_PARA2
		,FT_CCT_6238_OP_AWB_GET_NVRAM_CCM
		,FT_CCT_6238_OP_AWB_SET_NVRAM_CCM
		,FT_CCT_6238_OP_AWB_GET_AWB_PARA
		,FT_CCT_6238_OP_AWB_GET_CCM_PARA
		,FT_CCT_6238_OP_AWB_UPDATE_AWB_PARA
		,FT_CCT_6238_OP_AWB_UPDATE_CCM_PARA
		,FT_CCT_6238_OP_AWB_GET_AWB_STATUS
		,FT_CCT_6238_OP_AWB_GET_CCM_STATUS
		,FT_CCT_6238_OP_AWB_UPDATE_AWB_STATUS
		,FT_CCT_6238_OP_AWB_UPDATE_CCM_STATUS
		,FT_CCT_6238_OP_AWB_SET_CURRENT_CCM
		,FT_CCT_6238_OP_ISP_SET_SHADING_PARA
		,FT_CCT_6238_OP_ISP_GET_SHADING_PARA
		,FT_CCT_6238_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE
		,FT_CCT_6238_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE
		,FT_CCT_6238_OP_ISP_GET_TUNING_PARA
		,FT_CCT_6238_OP_ISP_SET_TUNING_PARA
		,FT_CCT_6238_OP_ISP_GET_PARTIAL_SHADING_TABLE
		,FT_CCT_6238_OP_ISP_SET_PARTIAL_SHADING_TABLE
		,FT_CCT_6238_OP_ISP_SET_SHADING_ON_OFF
		,FT_CCT_6238_OP_ISP_GET_SHADING_ON_OFF	
		,FT_CCT_6238_OP_AE_GET_SCENE_MODE
		,FT_CCT_6238_OP_AE_GET_METERING_MODE
		,FT_CCT_6238_OP_AE_GET_BAND
		,FT_CCT_6238_OP_AE_SET_GAMMA_BYPASS
		,FT_CCT_6238_OP_AE_GET_GAMMA_BYPASS_FLAG		
		,FT_CCT_6238_OP_ISP_SET_TUNING_INDEX
		,FT_CCT_6238_OP_ISP_GET_TUNING_INDEX
		,FT_CCT_6238_OP_ISP_GET_AE_TOTAL_GAIN
		,FT_CCT_6238_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF
		,FT_CCT_6238_OP_ISP_FLASHLIGHT_LINEARITY_PRESTROBE
		,FT_CCT_6238_OP_ISP_FLASHLIGHT_LINEARITY_RESULT
		,FT_CCT_6238_OP_ISP_ENABLE_BINNING_MODE
		,FT_CCT_6238_OP_ISP_DISABLE_BINNING_MODE     
		,FT_CCT_6238_OP_ISP_GET_BINNING_MODE
		,FT_CCT_6238_OP_ISP_FLASHLIGHT_SET_AE_PARA_2
		,FT_CCT_6238_OP_AF_GET_PARA
		,FT_CCT_6238_OP_AF_SET_PARA
		,FT_CCT_6238_OP_ISP_FLASHLIGHT_GET_AE_PARA_2
		,FT_CCT_6238_OP_ISP_GET_CAMCORDER_INFO
		,FT_CCT_6238_OP_ISP_BYPASS_SHADING_MODE_ENABLE
		,FT_CCT_6238_OP_ISP_BYPASS_SHADING_MODE_DISABLE
		,FT_CCT_6238_OP_FLASH_CONFIG
		,FT_CCT_6238_FLASH_DURATION_LUT
		,FT_CCT_6238_ISP_DEFECT_TABLE_ON
		,FT_CCT_6238_ISP_DEFECT_TABLE_OFF					// 184
		//----------[TH]M-shutter---------------------------------------------
		,FT_CCT_6238_OP_SET_SHUTTER_MODE					//185
		,FT_CCT_6238_OP_SET_SHUTTER_TARGET_TIME
		,FT_CCT_6238_OP_SET_SHUTTER_DELAY_TIME
		,FT_CCT_6238_OP_GET_SHUTTER_G_CHANNEL_MEAN			//188
		//,FT_CCT_6238_OP_GET_SHUTTER_TARGET_TIME
		//,FT_CCT_6238_OP_GET_SHUTTER_DELAY_TIME
		//,FT_CCT_6238_OP_GET_SHUTTER_MODE
		//,FT_CCT_6238_OP_SET_SHUTTER_CALIBRATION_ENABLE
		//,FT_CCT_6238_OP_SET_SHUTTER_CALIBRATION_DISABLE
		//,FT_CCT_6238_OP_GET_PREVIEW_TARGET_LINES
		//,FT_CCT_6238_OP_ENABLE_IMAGE_TRANSFER
		//,FT_CCT_6238_OP_DISABLE_IMAGE_TRANSFER
		//,FT_CCT_6238_OP_ISP_ENABLE_AUTO_DEFECT
		//,FT_CCT_6238_OP_ISP_DISABLE_AUTO_DEFECT
		//--------------Please add op code above this line for tool version compatibility--------------------
		,FT_CCT_6238_ISP_GET_DEFECT_TABLE					// 189
		,FT_CCT_6238_ISP_SET_DEFECT_TABLE
		,FT_CCT_6238_ISP_GET_SHADING_TABLE
		,FT_CCT_6238_ISP_SET_SHADING_TABLE	
		,FT_CCT_6238_OP_ISP_GET_TUNING_PARAS
		,FT_CCT_6238_OP_ISP_SET_TUNING_PARAS
		,FT_CCT_6238_OP_AE_GET_GAMMA_PARAS
		,FT_CCT_6238_OP_AE_UPDATE_GAMMA_PARAS
		,FT_CCT_6238_OP_AE_GET_WINDOW_HISTOGRAM_DATA
		,FT_CCT_6238_OP_AWB_GET_AWB_PARAS
		,FT_CCT_6238_OP_AWB_UPDATE_AWB_PARAS
		,FT_CCT_6238_OP_AF_GET_PARAS
		,FT_CCT_6238_OP_AF_SET_PARAS
		,FT_CCT_6238_OP_ISP_FLASHLIGHT_LINEARITY_RESULTS	// 202
//-----------Dual Camera Feature---------------------
		,FT_CCT_DUAL_OP_SET_CAMERA_MAIN_OR_SUB_TYPE // 203 dummy
		,FT_CCT_DUAL_OP_GET_CAMRA_PARA_TABLE		// 204 dummy
		,FT_CCT_DUAL_OP_GET_VALID_CAMERA_TYPE		// 205 dummy
		,FT_CCT_DUAL_OP_GET_SENSOR_PREFIX_NAME		// 206 dummy
		,FT_CCT_DUAL_OP_GET_LENS_PREFIX_NAME		// 207 dummy
    //---------------------------------------------
		,FT_CCT_6238_OP_ISP_GET_SHADING_TABLE_V3	// 208
		,FT_CCT_6238_OP_ISP_SET_SHADING_TABLE_V3	// 209
		,FT_CCT_6238_OP_CHECK_DYNAMIC_LSC_SUPPORT	// 210
		,FT_CCT_6238_OP_ENABLE_AWB_CT				// 211
		,FT_CCT_6238_OP_DISABLE_AWB_CT				// 212
		,FT_CCT_6238_OP_SET_LIGHT_SOURCE			// 213
    //for yusu cct
		,FT_CCT_V2_OP_SET_OB_ON_OFF				// 214
    ,FT_CCT_V2_OP_GET_OB_ON_OFF
    ,FT_CCT_V2_OP_SET_NR_ON_OFF
    ,FT_CCT_V2_OP_GET_NR_ON_OFF
    ,FT_CCT_V2_OP_SET_EE_ON_OFF
    ,FT_CCT_V2_OP_GET_EE_ON_OFF
    ,FT_CCT_V2_OP_AWB_GET_CCM_STATUS
    ,FT_CCT_OP_AE_GET_ENABLE_INFO
    ,FT_CCT_V2_OP_AE_APPLY_EXPO_INFO
    ,FT_CCT_OP_AWB_SET_AWB_MODE
    ,FT_CCT_OP_AWB_GET_AWB_MODE
    ,FT_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO
    ,FT_CCT_V2_OP_AE_SET_GAMMA_TABLE
    ,FT_CCT_V2_OP_AF_OPERATION
    ,FT_CCT_V2_OP_MF_OPERATION
    ,FT_CCT_V2_OP_AF_GET_BEST_POS
    ,FT_CCT_V2_OP_AF_GET_RANGE
    ,FT_CCT_OP_AE_GET_AUTO_EXPO_PARA
    ,FT_CCT_OP_FLASH_ENABLE
    ,FT_CCT_OP_FLASH_DISABLE
    ,FT_CCT_OP_FLASH_GET_INFO
    ,FT_CCT_V2_OP_GET_AF_INFO
    ,FT_CCT_V2_OP_AF_CALI_OPERATION
    ,FT_CCT_V2_OP_AF_SET_RANGE
    ,FT_CCT_V2_OP_AF_GET_FV
    ,FT_CCT_V2_OP_AF_READ
    ,FT_CCT_V2_OP_AF_APPLY
    ,FT_CCT_V2_OP_SHADING_CAL
    ,FT_CCT_V2_OP_DEV_AE_GET_INFO
    ,FT_CCT_OP_DEV_AE_APPLY_INFO
    ,FT_CCT_OP_DEV_AE_APPLY_MANUAL_INFO
    ,FT_CCT_OP_DEV_AE_GET_EV_CALIBRATION
    ,FT_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2
    ,FT_CCT_V2_OP_AWB_GET_AWB_PARA
    ,FT_CCT_V2_OP_ISP_SET_SHADING_INDEX
    ,FT_CCT_V2_OP_ISP_GET_SHADING_INDEX
		,FT_CCT_V2_OP_SAVE_OB_ON_OFF			// 250
		// CDVT ------------------------------
		,FT_CCT_OP_CDVT_SENSOR_TEST				// 251
		,FT_CCT_OP_CDVT_SENSOR_CALIBRATION		// 252
		// -----------------------------------
		,FT_CCT_OP_GET_DEFECT_TABLE				// 253
		,FT_CCT_OP_SET_DEFECT_TABLE
		,FT_CCT_OP_DEV_GET_DSC_INFO
		,FT_CCT_OP_DEV_GET_IRIS_INFO
		,FT_CCT_OP_DEV_SET_IRIS_INFO			// 257
		
		,FT_CCT_OP_GET_IMAGE_DIMENSION			// 258
		,FT_CCT_OP_SUBPREVIEW_LCD_START			//259
		,FT_CCT_OP_SUBPREVIEW_LCD_STOP			//260
		,FT_CCT_OP_PHOTOFLASH_CONTROL			//261
		// 6573 new feature
		,FT_CCT_OP_ISP_READ_REG					// 259
		,FT_CCT_OP_ISP_WRITE_REG				// 260
		,FT_CCT_OP_READ_SENSOR_REG				// 261
		,FT_CCT_OP_WRITE_SENSOR_REG				// 262
		,FT_CCT_OP_DEV_AE_SAVE_INFO_NVRAM		// 263
		,FT_CCT_V2_OP_AWB_SAVE_AWB_PARA			// 264
		,FT_CCT_V2_OP_AF_SAVE_TO_NVRAM			// 265
		,FT_CCT_OP_ISP_LOAD_FROM_NVRAM			// 266
		,FT_CCT_OP_ISP_SAVE_TO_NVRAM			// 267
		,FT_CCT_OP_ISP_SET_PCA_TABLE			// 268
		,FT_CCT_OP_ISP_GET_PCA_TABLE			// 269
		,FT_CCT_OP_ISP_SET_PCA_PARA				// 270
		,FT_CCT_OP_ISP_GET_PCA_PARA				// 271
		,FT_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF			//272
		,FT_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF			//273
		,FT_CCT_OP_SET_CCM_MODE					//274
		,FT_CCT_OP_GET_CCM_MODE					//275
		,FT_CCT_V2_OP_ISP_GET_NVRAM_DATA		//276
		,FT_CCT_V2_OP_ISP_GET_NVRAM_DATA_BUF	//277
		,FT_CCT_OP_SET_ISP_ON			//278
		,FT_CCT_OP_SET_ISP_OFF		//279
		,FT_CCT_OP_GET_ISP_ON_OFF	//280
		,FT_CCT_OP_GET_LSC_SENSOR_RESOLUTION	//281
		,FT_CCT_OP_SDTBL_LOAD_FROM_NVRAM
		,FT_CCT_OP_SDTBL_SAVE_TO_NVRAM
		,FT_CCT_OP_AWB_GET_LIGHT_PROB
		,FT_CCT_OP_STROBE_RATIO_TUNING	//285
		,FT_CCT_OP_SWITCH_CAMERA // 289
		,FT_CCT_OP_MAIN2PREVIEW_LCD_START
		,FT_CCT_OP_MAIN2PREVIEW_LCD_STOP
		,FT_CCT_OP_STROBE_CALIBRATION_START
		,FT_CCT_OP_STROBE_CALIBRATION_STOP
    ,FT_CCT_OP_STROBE_GET_CALIBRATION_STATUS
    ,FT_CCT_OP_STROBE_GET_CALIBRATION_DUMP
    ,FT_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF
    ,FT_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF
    ,FT_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM
    ,FT_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM
    ,FT_CCT_OP_ISP_GET_PCA_SLIDER
    ,FT_CCT_OP_ISP_SET_PCA_SLIDER
    ,FT_CCT_OP_AE_GET_CAPTURE_PARA
    ,FT_CCT_OP_AE_SET_CAPTURE_PARA
    //6582
    ,FT_CCT_OP_STROBE_READ_NVRAM_TO_PC
		,FT_CCT_OP_STROBE_SET_NVDATA
		,FT_CCT_OP_STROBE_WRITE_NVRAM
		,FT_CCT_OP_AE_GET_FALRE_CALIBRATION
		,FT_CCT_OP_AE_PLINE_TABLE_TEST
		,FT_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE
		,FT_CCT_OP_END
	} FT_CCT_OP;

	typedef struct 
	{
		unsigned int				value;
		unsigned char				value_len;
	} FT_CCT_REG_READ_CNF;
	typedef struct 
	{
		unsigned int				reg_addr;
		unsigned char				reg_len;
		unsigned char				value_len;
	} FT_CCT_REG_READ_REQ;
	typedef struct 
	{
		unsigned int				reg_addr;
		unsigned int				value;
		unsigned char				reg_len;
		unsigned char				value_len;
	} FT_CCT_REG_WRITE_REQ;
	typedef enum 
	{
		AP_SENSOR_OUTPUT_FORMAT_RAW_B=0,
		AP_SENSOR_OUTPUT_FORMAT_RAW_Gb,
		AP_SENSOR_OUTPUT_FORMAT_RAW_Gr,
		AP_SENSOR_OUTPUT_FORMAT_RAW_R,
		AP_SENSOR_OUTPUT_FORMAT_UYVY,
		AP_SENSOR_OUTPUT_FORMAT_VYUY,
		AP_SENSOR_OUTPUT_FORMAT_YUYV,
		AP_SENSOR_OUTPUT_FORMAT_YVYU,
		AP_SENSOR_OUTPUT_FORMAT_CbYCrY,
		AP_SENSOR_OUTPUT_FORMAT_CrYCbY,
		AP_SENSOR_OUTPUT_FORMAT_YCbYCr,
		AP_SENSOR_OUTPUT_FORMAT_YCrYCb

	} FT_CCT_BAYER_PATTERN;

	typedef struct 
	{
		ACDK_CCT_REG_TYPE_ENUM			type;
		unsigned int			device_id;
		unsigned short int		width;
		unsigned short int		height;
		FT_CCT_BAYER_PATTERN	start_pixel_bayer_ptn;
		unsigned short int		grab_x_offset;
		unsigned short int		grab_y_offset;
	} FT_CCT_SENSOR;
	typedef struct 
	{
		FT_CCT_SENSOR			sensor[4];
		unsigned char			sensor_count;
	} FT_CCT_ON_BOARD_SENSOR;
	typedef struct 
	{
		ACDK_CCT_REG_TYPE_ENUM			type;
		unsigned int			device_id;
		unsigned short int		width;
		unsigned short int		height;
		unsigned short int		preview_width;
		unsigned short int		preview_height;
		FT_CCT_BAYER_PATTERN	start_pixel_bayer_ptn;
		unsigned short int		grab_x_offset;
		unsigned short int		grab_y_offset;
	} FT_CCT_SENSOR_EX;
	typedef struct 
	{
		unsigned int			sensor_count;
		FT_CCT_SENSOR_EX		sensor[4];
	} FT_CCT_ON_BOARD_SENSOR_EX;
	typedef enum 
	{
		FT_CCT_WB_MANUAL_1 = 0,
		FT_CCT_WB_MANUAL_2,
		FT_CCT_WB_MANUAL_3,
		FT_CCT_WB_MANUAL_4,
		FT_CCT_WB_MANUAL_5,
		FT_CCT_WB_AUTO = 0xAA,
		FT_CCT_WB_RESET = 0xBB,
		FT_CCT_WB_OFF = 0xFF
	} FT_CCT_WB_SETTING;
	/*typedef enum 
	{
    CAMERA_TUNING_PREVIEW_SET = 0,
    CAMERA_TUNING_CAPTURE_SET,
    CAMERA_TUNING_BINNING_SET
		,FT_CCT_COMP_END
	} FT_CCT_COMP_SET_ENUM;*/
	typedef struct 
	{
		unsigned int		curr_lum;
		unsigned int		target_lum;
	} FT_CCT_DEV_STROBE_CAL_PARA;
	typedef enum {
		CAPTURE_JPEG_IDLE = 0
		,CAPTURE_JPEG_PROCESS
		,CAPTURE_JPEG_DONE
		,CAPTURE_JPEG_FAIL
	} FT_CCT_CAPTURE_JPEG_STATE_E;
	typedef enum {
		DATA_TUNNEL_RS232 = 0
		,DATA_TUNNEL_USB_COM
	} FT_CCT_DATA_TUNNEL_E;
	typedef enum {
		USB_TUNNEL_IDLE = 0
		,USB_TUNNEL_SENDING
		,USB_TUNNEL_CANCEL
		,USB_TUNNEL_FINISH
	} FT_CCT_USB_TUNNEL_STATE_E;
	typedef struct 
	{
		unsigned char			value;
		unsigned char			ec_step;
		signed char				ec_level;
	} FT_CCT_AE_SET_EXPOSE_LEVEL_REQ;
	#if 0           //unknown delete
	typedef struct 
	{
		unsigned char			mode;
		aeSmoothModeStruct		setting;         
	}aeUpdateSmoothModeStruct;
	#endif
	typedef struct 
	{
		signed int				value;
		signed int				min;
		signed int				max;
		bool					exist;
	} FT_CCT_SensorEngModeItem;
	typedef struct 
	{
		FT_CCT_SensorEngModeItem	pregain_r;
		FT_CCT_SensorEngModeItem	pregain_gr;
		FT_CCT_SensorEngModeItem	pregain_gb;
		FT_CCT_SensorEngModeItem	pregain_b;
	} FT_CCT_SENSOR_PREGAIN;
	typedef struct{
		unsigned char		mode;
		unsigned char		m_switch;
	}ispShadingStatusMsg;
	typedef enum 
	{
		FT_CCT_CAP_TO_INT_SRAM = 0,
		FT_CCT_CAP_TO_EXT_SRAM
	} FT_CCT_CAP_BUF_TYPE;
	typedef struct 
	{
		unsigned short int		left_pos;
		unsigned short int		top_pos;
		unsigned short int		width;
		unsigned short int		height;
		unsigned char			sub_sample;
		unsigned char			output_format;
		FT_CCT_CAP_BUF_TYPE		buf_type;
	} FT_CCT_CAPTURE_REQ;
	typedef struct 
	{
		unsigned char			status;
	}FT_CCT_NVRAM_CNF;
	typedef struct 
	{
		unsigned char			step;
	}FT_CCT_AE_QUERY_EC_STEP_INFO_REQ;
	typedef struct 
	{
		unsigned short int		left_pos;
		unsigned short int		top_pos;
		unsigned short int		width;
		unsigned short int		height;
		unsigned char			sub_sample;
		unsigned char			output_format;
		FT_CCT_BAYER_PATTERN	bayer_ptn;
		unsigned short int		output_width;
		unsigned short int		output_height;
		unsigned short int		frame_num;
		bool					last_frame;
	} FT_CCT_CAPTURE_CNF;
	typedef struct 
	{
		unsigned char			value;
		unsigned char			ec_step;
		signed char				ec_level;
	} FT_CCT_AE_GET_EXPOSE_LEVEL_CNF;
	typedef struct 
	{
		unsigned char			step;
		unsigned char			total_level;
	} FT_CCT_AE_QUERY_EC_STEP_INFO_CNF;
	typedef struct 
	{
		FT_CCT_WB_SETTING		activated_index;
	} FT_CCT_WB_QUERY_ALL_CNF;
	typedef struct 
	{
		unsigned short int		left_pos;
		unsigned short int		top_pos;
		unsigned short int		width;
		unsigned short int		height;
		unsigned char			sub_sample;
		unsigned char			output_format;
		FT_CCT_BAYER_PATTERN	bayer_ptn;
		unsigned short int		output_width;
		unsigned short int		output_height;
		unsigned int			capture_size;
		bool					continue_shot;
	} FT_CCT_CAPTURE_EX_CNF;
	typedef struct 
	{
		unsigned int			baseshutter_60hz;
		unsigned int			baseshutter_50hz;
	} FT_CCT_BANDING_FACTOR;
	typedef struct
	{
		bool					pregain_r_result;
		bool					pregain_gr_result;
		bool					pregain_gb_result;
		bool					pregain_b_result;
	} FT_CCT_SET_SENSOR_PREGAIN_CNF;
static const int AV_NO = 1;
	typedef struct 
	{
		bool							iris[1];    //iris[AV_NO]
	} FT_CCT_DEV_IRIS_INFO;
	typedef struct 
	{
		unsigned int		pos;
		unsigned int		atf_value;
	} FT_CCT_DEV_AF_CAL_RESULT;
	typedef struct 
	{
		unsigned short int				gain[3];
	} FT_CCT_DEV_ISO_GAIN;
	typedef struct 
	{
		unsigned short int				value[3];
	} FT_CCT_DEV_ISO_VALUE;
	typedef struct 
	{
		unsigned short int				width;
		unsigned short int				height;
		unsigned short int				preview_width;
		unsigned short int				preview_height;
	} FT_CCT_DEV_MODE_SIZE;
	/*
	static const int AE_MAX_FLARE_HIST_NO = 10;
	typedef struct{
		unsigned int count;
		unsigned int histogram[10];     //histogram[AE_MAX_FLARE_HIST_NO]
	}aeFlareHistogram;
	*/
	typedef struct{
		unsigned int windowWhitPoint[12];
	}awbWindowWhitPointArray;
	typedef struct{
		unsigned int value[EIsp_Num_Of_Category];   // OB, DM, DP, NR1, NR2, EE, SATURATION, CONTRAST, HUE
	}ispTuningSettingIndexValue;
	typedef struct{
		unsigned char  mode;
		unsigned int offset;
		unsigned int length;
	}ispDefectPara;
	typedef struct{
		unsigned char  mode;
		unsigned int offset;
		unsigned int length;
	}ispShadingPara;
	typedef struct{
	kal_uint8  mode;
	kal_uint8  color_temperature;
	kal_uint32 offset;
	kal_uint32 length;	
	}ispShadingParaV3;
	typedef struct 
	{
		FT_CCT_WB_SETTING		index;
	} FT_CCT_WB_ACTIVATE_BY_INDEX_REQ;
	typedef struct 
	{
		unsigned short int				color_temperature;
		unsigned short int				r_gain;
		unsigned short int				g_gain;
		unsigned short int				b_gain;
	} FT_CCT_WHITE_BALANCE;
	typedef struct 
	{
		FT_CCT_WB_SETTING		index;
		FT_CCT_WHITE_BALANCE	wb;
	} FT_CCT_WB_SET_BY_INDEX_REQ;
	typedef struct 
	{
		unsigned short int		left_pos;
		unsigned short int		top_pos;
		unsigned short int		width;
		unsigned short int		height;
		unsigned char			sub_sample;
		unsigned char			output_format;
		FT_CCT_CAP_BUF_TYPE		buf_type;
		bool					continue_shot;
	} FT_CCT_CAPTURE_EX_REQ;
	typedef struct
	{
		unsigned int			offset;
		unsigned int			length;
	} FT_CCT_GET_CAPTURE_BUF_REQ;
	typedef struct 
	{
		bool					no_confirm;
	} FT_CCT_RESUME_PREVIEW_REQ;
	typedef struct
	{
		bool					turn_on;
	} FT_CCT_MAIN_LCD_BACKLIGHT_REQ;
	typedef struct 
	{
		unsigned char		activate_af_table_index;
		unsigned int		set_pos;
	} FT_CCT_DEV_AF_PARA;
	typedef struct 
	{
		unsigned int					shutter;
		unsigned int					sensorgain;
		unsigned int					ispgain;
	} FT_CCT_DEV_AE_SET_VALUE_PARA;
	typedef struct 
	{
		unsigned char					select;
	} FT_CCT_DEV_ISO_CAPTURE;
	typedef struct 
	{
		unsigned char			sub_sample;
		unsigned char			output_format;
	} FT_CCT_GET_IMAGE_DIMENSION_REQ;
	typedef struct 
	{
		unsigned short int		width;
		unsigned short int		height;
		unsigned char			sub_sample;
		unsigned char			output_format;
		unsigned short int		output_width;
		unsigned short int		output_height;
	} FT_CCT_GET_IMAGE_DIMENSION_CNF;

	
	typedef struct{
		unsigned char lightSourceMode;
		cct_ccm_matrix_struct nvram_ccm;
	}awbNvramCCMReq;
	typedef struct{
		unsigned char enableFlag;
		//flashlight_cdt_ae_para_struct aePara;
	}ispFlashAePara;
	typedef struct {
		unsigned short int lut_trigger_time[20];
		unsigned short int lut_result_time[20];
	}xenon_duration_lut_struct;	


	typedef struct
	{
		unsigned int			AE_win[50];
	} FT_CCT_STROBE_CAL_STRUCT;

#if 0         //  temp delete
typedef struct {
	DSC_INFO_STRUCT				dsc_info;	/* defined in MSDK_ISP_exp.h */
	SENSOR_INFO_STRUCT			sensor_info;
} FT_CCT_DEV_DSC_INFO;
#endif
#if 0        // temp delete
typedef struct {
	DEVICE_INFO_STRUCT				DeviceInfo;	/* defined in MSDK_ISP_exp.h */
	MSDK_CCT_DEV_AE_TABLE			AETable;
} FT_CCT_DEV_AE_INFO;
#endif
	typedef union 
	{
		FT_CCT_REG_READ_REQ					reg_read;
		FT_CCT_REG_WRITE_REQ				reg_write;
		//AE_LUT_INFO_STRUCT					dev_ae_set_scene_mode;
		FT_CCT_AE_SET_EXPOSE_LEVEL_REQ		ae_expose;
		//aeUpdateSmoothModeStruct			dev_6238_ae_smooth_mode_setting;       //temp delete
		ACDK_CCT_NVRAM_CCM_PARA				dev_6238_awb_cmm_para;
		unsigned int						dev_6238_ae_gamma_bypass;
		//aeIsoLutStruct						dev_6238_ae_iso_value_gain;
		//aeCctExpoSettingStruct				dev_6238_ae_auto_expo_para;
		ACDK_AE_MODE_CFG_T       yusu_dev_6238_ae_auto_expo_para;
		//aeFlareSettingStruct				dev_6238_ae_flare_para;
		//aeCctMeteringSettingStruct			dev_6238_ae_metering_mode_setting;
		unsigned char						dev_6238_ae_scene_mode;
		unsigned int						dev_6238_awb_small_box_light_source;
		cct_awb_gain_struct					dev_6238_awb_set_gain;           //temp delete
		ACDK_CCT_CCM_STRUCT					dev_6238_awb_current_ccm;
		unsigned char						dev_6238_ae_metering_mode;
		//aeCctApplyExpoInfoStruct			dev_6238_ae_expo_info_req;
		unsigned char						dev_6238_ae_select_band;
		unsigned short int					ae_manual_shutter;
		winmo_cct_shading_comp_struct				set_shading_para;          // temp delete
		ispShadingStatusMsg					dev_6238_isp_shading_status;
		FT_CCT_CAPTURE_REQ					capture;	
		FT_CCT_AE_QUERY_EC_STEP_INFO_REQ	ae_ec_step;
		FT_CCT_WB_ACTIVATE_BY_INDEX_REQ		wb_activate;
		FT_CCT_WB_SET_BY_INDEX_REQ			wb_set;
		unsigned short int					ae_init_shutter;
		FT_CCT_CAPTURE_EX_REQ				capture_ex;
		FT_CCT_GET_CAPTURE_BUF_REQ			get_capture_buf;
		FT_CCT_RESUME_PREVIEW_REQ			resume_preview;
		FT_CCT_SENSOR_PREGAIN				set_sensor_pregain;
		FT_CCT_MAIN_LCD_BACKLIGHT_REQ		main_lcd_backlight;
		unsigned short int					cal_nvram_lid;
		signed int							set_eng_sensor_para;
		//FT_CCT_COMP_SET_ENUM				set_compensation_mode;		
		CAMERA_TUNING_SET_ENUM				set_compensation_mode;		
		//cct_autodefect_comp_struct			set_autodefect_para;       //  unknown delete
		unsigned short int					set_capture_data_tunnel;
		//defectpixel_para_struct				defect_table_cal_para;   //  unknown delete
		FT_CCT_DEV_IRIS_INFO				dev_set_iris_info;
		FT_CCT_DEV_AF_PARA					dev_af_para;
		FT_CCT_DEV_STROBE_CAL_PARA			dev_strobe_cal_para;
		bool								on_off;
		//AF_OQC_cal_struct					lens_offset_cal_para;
		//camera_ae_period_para_struct		set_ae_period_para;
		FT_CCT_DEV_AE_SET_VALUE_PARA		dev_ae_set_value_para;
		unsigned char						dev_ae_set_table_index;	
		FT_CCT_DEV_ISO_GAIN					dev_set_iso_gain;
		FT_CCT_DEV_ISO_CAPTURE				dev_set_iso_capture;
		FT_CCT_DEV_ISO_VALUE				dev_set_iso_value;
		//flashlight_cdt_charge_struct		dev_flashlight_charge;
		//flashlight_cdt_strobe_struct		dev_flashlight_strobe;
		//flashlight_cdt_capture_struct       dev_flashlight_capture;
		unsigned int						flash_charge_level;		
		//cct_awb_preference_struct 			dev_6238_awb_set_pref_factor;         //temp delete
		unsigned char						dev_6238_awb_light_source_mode;
		//awbNvramCCMReq						dev_6238_awb_nvram_ccm;	     //temp delete
		ACDK_CCT_CCM_LIGHTMODE_STRUCT						dev_6238_awb_nvram_ccm;	     //temp delete
		//cct_awb_status_struct 				dev_6238_awb_awb_status;  //temp delete
		ACDK_CCT_CCM_STATUS_STRUCT 				dev_6238_awb_ccm_status;	//temp delete
		//CCT_LIGHT_SOURCE_ENUM				dev_6238_awb_cct_light_source;    //temp delete
		winmo_cct_shading_comp_struct				dev_6238_isp_set_shading_para;     //temp delete
			
		ispTuningSettingIndexValue			dev_6238_isp_tuning_setting_index_value;
		//flashlight_cdt_capture_struct		dev_6238_isp_flashlight_cdt_capture;     //unknown delete
		ispFlashAePara						dev_6238_isp_flashlight_set_ae_para;
		xenon_duration_lut_struct			dev_6238_flash_duration_lut;
		unsigned int						dev_6238_shutter_mode;
		unsigned int						dev_6238_shutter_target_time;
		unsigned int						dev_6238_shutter_delay_time;
		unsigned int						dev_6238_shutter_target_time_for_preview_target_lines;
		ispDefectPara						dev_6238_isp_defect_para;
		ispShadingPara						dev_6238_isp_shading_para;
		ispShadingParaV3					dev_6238_isp_shading_para_v3;
		ACDK_CCT_MODULE_CTRL_STRUCT          module_ctrl;
		ACDK_AE_MODE_CFG_T                    dev_ae_mode_cfg;
		unsigned int            dev_set_awb_mode;
		ACDK_CCT_GAMMA_TABLE_STRUCT          dev_ae_gamma_table;
		int                     dev_mf_operation;
		FOCUS_RANGE_T           dev_focus_range;
		ACDK_AF_POS_T           dev_af_pos;
		NVRAM_LENS_PARA_STRUCT  dev_af_lens_para;
		ACDK_CCT_LSC_CAL_SET_STRUCT          dev_lsc_cal_set;
		AE_NVRAM_T                           dev_ae_nvram_info;
		AWB_NVRAM_T                          dev_awb_nvram_info;
		unsigned char           dev_isp_shading_index;
		unsigned int            dev_save_ob_value;
		ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T    dev_cdvt_sensor_cal_input;
		unsigned int						dummy;
		FT_CCT_GET_IMAGE_DIMENSION_REQ		get_image_dimension;
		ACDK_CCT_FUNCTION_ENABLE_STRUCT		func_enable;
		unsigned int						awb_get_nvram_ccm_index;		// 6573
		ACDK_CCT_SET_NVRAM_CCM				awb_set_nvram_ccm_para;			// 6573
		ACDK_CCT_ACCESS_NVRAM_PCA_TABLE		isp_get_pca_table;				// 6573
		ACDK_CCT_ACCESS_NVRAM_PCA_TABLE		isp_set_pca_table;				// 6573
		ACDK_CCT_ACCESS_PCA_CONFIG			isp_set_pca_para;				// 6573
		unsigned int					dev_ccm_mode;
		CAMERA_NVRAM_STRUCTURE_ENUM 		dev_nvram_mode;
		FT_CCT_GET_CAPTURE_BUF_REQ			get_buf;
		ACDK_CCT_ISP_REG_CATEGORY			set_isp_on_off;
		ACDK_CCT_ISP_REG_CATEGORY			get_isp_on_off;
		unsigned int					dev_strobe_level;
		unsigned int					dev_src_device;
		ISP_NVRAM_CCM_POLY22_STRUCT		set_ccm_poly22;
		ISP_NVRAM_MFB_MIXER_STRUCT		set_MFB_mixer;
		ACDK_CCT_ACCESS_PCA_SLIDER		set_pca_slider;
		ACDK_STROBE_STRUCT						set_flash_nvram;
		unsigned int								set_Flare_Thres;
		ACDK_CDVT_AE_PLINE_TEST_INPUT_T		dev_pline_test_input;
	} FT_CCT_CMD;

	typedef union 
	{
		FT_CCT_REG_READ_CNF					reg_read;
		//aeSmoothModeStruct					get_6238_ae_smooth_mode_setting;      //unknown delete
		unsigned int						get_6238_awb_light_mode;
		cct_awb_gain_struct 				get_6238_awb_gain;              // temp delete
		ACDK_CCT_CCM_STRUCT					get_6238_awb_current_ccm;
		//aeCctCurrentExpoInfoStruct			get_6238_ae_current_expo_info_cnf;
		unsigned short int					ae_manual_shutter;
		unsigned int						isp_id;
		FT_CCT_SENSOR_PREGAIN				get_sensor_pregain;
		winmo_cct_shading_comp_struct				get_shading_para;      //temp delete 
		FT_CCT_CAPTURE_CNF					capture;
		FT_CCT_NVRAM_CNF					nvram;
		FT_CCT_AE_GET_EXPOSE_LEVEL_CNF		ae_expose;
		FT_CCT_AE_QUERY_EC_STEP_INFO_CNF	ae_ec_step;
		FT_CCT_WB_QUERY_ALL_CNF				wb_query_all;
		unsigned short int					ae_init_shutter;	
		FT_CCT_CAPTURE_EX_CNF				capture_ex;
		FT_CCT_BANDING_FACTOR				get_banding_factor;		
		FT_CCT_SET_SENSOR_PREGAIN_CNF		set_sensor_pregain;	
		FT_CCT_SensorEngModeItem			get_eng_sensor_para;
		//FT_CCT_COMP_SET_ENUM				get_compensation_mode;	
		CAMERA_TUNING_SET_ENUM				get_compensation_mode;	
		//cct_autodefect_comp_struct			get_autodefect_para;      //unknown delete
		unsigned short int					get_last_compensation_mode;
		unsigned short int					get_autodefect_count;
		//defectpixel_para_struct				defect_table_cal_para;
		FT_CCT_DEV_IRIS_INFO				dev_get_iris_info;
		FT_CCT_DEV_AF_CAL_RESULT			dev_af_cal_result;
		unsigned char						lens_offset_cal_return_code;
		//camera_ae_period_para_struct		get_ae_period_para;
		bool								on_off;
		FT_CCT_DEV_ISO_GAIN					dev_get_iso_gain;
		FT_CCT_DEV_ISO_VALUE				dev_get_iso_value;
		FT_CCT_DEV_MODE_SIZE				mode_size;
		unsigned char						flash_type;
		//exposure_lut_struct					flash_lut;
		//EXPOSURE_LUT_STRUCT					flash_lut;          //unknown delete
		unsigned int						flash_charge_level;
		unsigned int						flash_charge_timeout;
		unsigned char						ae_index;	
		//aeCctExpoSettingStruct				get_6238_ae_auto_expo_para;
		//aeIsoLutStruct						get_6238_ae_iso_value_gain;
		//aeFlareSettingStruct				get_6238_ae_flare_para;
		//aeFlareHistogram					get_6238_ae_flare_histogram;//remove
		//aeCctCurrentMeteringResultStruct 	get_6238_ae_metering_result;
		//aeCctMeteringSettingStruct			get_6238_ae_metering_mode_setting;	
		//aeOutputCurrentAeInfo				get_6238_ae_preview_expo_info;
		//aeGammaTableStruct					get_6238_ae_gamma_table;
		ACDK_CCT_GAMMA_TABLE_STRUCT   get_6238_ae_gamma_table;
		awbWindowWhitPointArray				get_6238_awb_window_whit_point;	
		//cct_awb_preference_struct			get_6238_awb_pref_factor;	 // temp dalete
		ACDK_CCT_CCM_STRUCT					get_6238_awb_nvram_ccm;    // temp dalete
		ACDK_CCT_NVRAM_CCM_PARA 			get_6238_awb_cmm_para;
		//cct_awb_status_struct 				get_6238_awb_awb_status;  // temp dalete
		//cct_ccm_status_struct 				get_6238_awb_cmm_status;   // temp dalete
		winmo_cct_shading_comp_struct				get_6238_isp_shading_para;      //temp delete
		ispShadingStatusMsg					get_6238_isp_shading_status;
		unsigned char						get_6238_ae_scene_mode;
		unsigned char						get_6238_ae_metering_mode;
		unsigned char						get_6238_ae_band;
		unsigned int						get_6283_ae_gamma_bypass_flag;
		ispTuningSettingIndexValue			get_6238_isp_tuning_setting_index_value;
		unsigned int						get_6238_isp_ae_total_gain;
		unsigned char						get_6238_isp_dynamic_bypass_mode_on_off;
		unsigned char						get_6238_isp_binning_mode;
		//flashlight_cdt_ae_para_struct		get_6238_isp_flashlight_cdt_capture;
		//camcorder_info_struct				get_6238_isp_dev_camcorder_info;
		unsigned int						get_6238_shutter_g_channel_mean;
		unsigned int						get_6238_shutter_delay_time;
		unsigned int						get_6238_shutter_target_time;		
		//CAMERA_CAPTURE_EXP_ENUM				get_6238_shutter_mode;
		unsigned int						get_6238_shutter_target_lines;
		ispDefectPara						get_6238_isp_defect_table_para;
		ispShadingPara						get_6238_isp_shading_table_para;
		ispShadingParaV3					get_6238_isp_shading_table_para_v3;
		kal_bool							get_6238_is_supported;
		ACDK_CCT_MODULE_CTRL_STRUCT        get_module_ctrl;
		ACDK_CCT_CCM_STATUS_STRUCT         get_ccm_status;
		unsigned int         get_ae_enable_info;
		unsigned int            get_awb_mode;
		int                     get_auto_run_info;
		int                     get_af_best_pos;
		FOCUS_RANGE_T                           get_focus_range;
		ACDK_AE_MODE_CFG_T                get_acd_ae_mode_cfg;
		int                     get_flash_enable;
		ACDK_AF_INFO_T          get_af_info;
		ACDK_AF_VLU_32T           get_af_pos;
		NVRAM_LENS_PARA_STRUCT  get_af_lens_para;
		AE_NVRAM_T              get_ae_nvram_info;
		int                     get_ae_current_ev_value;
		AWB_NVRAM_T             get_awb_nvram_info;
		unsigned char           get_isp_shading_index;
		ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T     get_cdvt_sensor_cal_output;
		unsigned int						dummy; // mark for 8 byte pack
		//unsigned char                       buff[1824];
		FT_CCT_GET_IMAGE_DIMENSION_CNF		get_image_dimension;
		ACDK_CCT_FUNCTION_ENABLE_STRUCT		get_func_enable;
		ACDK_CCT_ACCESS_NVRAM_PCA_TABLE		isp_get_pca_table;				// 6573
		ACDK_CCT_ACCESS_PCA_CONFIG			isp_get_pca_para;				// 6573
		unsigned int					get_ccm_mode;	
		ACDK_CCT_SENSOR_RESOLUTION_STRUCT	get_lsc_sensor_res;
		ACDK_AWB_LIGHT_PROBABILITY_T			get_awb_prob;
		FT_CCT_STROBE_CAL_STRUCT				get_AE_win;
		ISP_NVRAM_CCM_POLY22_STRUCT			get_ccm_poly22;
		ISP_NVRAM_MFB_MIXER_STRUCT			get_MFB_mixer;
		ACDK_CCT_ACCESS_PCA_SLIDER			get_pca_slider;
		ACDK_STROBE_STRUCT						get_flash_nvram;
		unsigned int								get_Flare_Offset;
		//ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T		get_pline_test_result;
	} FT_CCT_RESULT;

	typedef struct 
	{
		FT_H				header;
		FT_CCT_OP			op;
		ACDK_CCT_REG_TYPE_ENUM		type;
		unsigned int		device_id;
		FT_CCT_CMD			cmd;
	} FT_CCT_REQ;

       #pragma pack (4)
	typedef struct {
		FT_H				header;
		FT_CCT_OP			op;
		unsigned char		status;
		//int				dummy;
		FT_CCT_RESULT		result;			
	} FT_CCT_CNF;
	#pragma pack()
	
	typedef struct {

		kal_bool	is_init;
		kal_bool	is_fb_init;
		kal_uint8	cnf_return_code;

		// memory management 
		kal_uint32	mem_lcd_layer0_buf_addr;
		kal_uint32	mem_lcd_layer0_buf_size;
		kal_uint32	mem_isp_int_buf_addr;
		kal_uint32	mem_isp_int_buf_size;
		kal_uint32	mem_isp_ext_buf_addr;
		kal_uint32	mem_isp_ext_buf_size;
		kal_uint32	mem_capture_ext_buf_addr;
		kal_uint32	mem_capture_ext_buf_size;
		kal_uint32	mem_capture_int_buf_addr;
		kal_uint32	mem_capture_int_buf_size;
		kal_uint32	mem_capture_cur_buf_addr;
		kal_uint32	mem_capture_cur_buf_size;

		// sensor info 
		FT_CCT_ON_BOARD_SENSOR_EX	sensor_onboard_sensors;
		// sensor engineer mode state 
		kal_int16					sensor_eng_group_idx;
		kal_int16					sensor_eng_item_idx_pregain_R;
		kal_int16					sensor_eng_item_idx_pregain_Gr;
		kal_int16					sensor_eng_item_idx_pregain_Gb;
		kal_int16					sensor_eng_item_idx_pregain_B;

		// preview state 
		const FT_CCT_SENSOR_EX	*	p_preview_sensor;

		// AE state 
		kal_bool					ae_enable;

		// WB state 
		FT_CCT_WB_SETTING			wb_activated_idx;

		// AF state 
		kal_bool					af_on_off;
		kal_bool					af_done;
		kal_bool					af_lens_offset_cal;

		// compensation mode state 
		//FT_CCT_COMP_SET_ENUM		comp_mode;
		CAMERA_TUNING_SET_ENUM    comp_mode;

		// defect table calibration state 
		kal_bool					defect_table_on_off;
		kal_bool					defect_table_cal_in_progress;

		// strobe calibration state 
		kal_bool					strobe_cal_enable;
		FT_CCT_DEV_STROBE_CAL_PARA	strobe_cal_para;

		// capture jpeg state 
		FT_CCT_CAPTURE_JPEG_STATE_E	capture_jpeg_state;

		// USB data tunnel state 
		kal_bool					usb_ready;
		FT_CCT_DATA_TUNNEL_E		usb_capture_data_tunnel;
		FT_CCT_USB_TUNNEL_STATE_E	usb_tunnel_state;
		kal_uint32					usb_ready_to_write_cnt;

		//capture dormat
		unsigned char					output_format;

		//src device
		kal_uint32					src_device_mode;//main : 0x1¡Asub : 0x2¡Amain2 : 0x8 [89]

		// dev tool scene mode 
		//AE_LUT_INFO_STRUCT			dev_ae_scene_mode;	

		// debug 
		//FT_CCT_REQ		debug_last_req;  // temp delete
		//FT_CCT_CNF		debug_last_cnf;   // temp delete

	} FT_CCT_STATE_MACHINE;

	typedef struct 
	{
	FT_CCT_WHITE_BALANCE	wb[5];
	} FT_CCT_WHITE_BALANCE_ALL;

	typedef struct 
	{
	char					group_name[64];
	char					item_name[64];
	} FT_CCT_SENSOR_ENG_KEY;



/*
	typedef struct
	{
	unsigned char					table_size;
	//exposure_lut_struct				ae_table[256];
	EXPOSURE_LUT_STRUCT				ae_table[256];
	unsigned char					iris_table[256];
	} FT_CCT_DEV_AE_TABLE;
*/
	//typedef struct 
	//{
	//device_info_struct				device_info;
	//DEVICE_INFO_STRUCT				device_info
	//FT_CCT_DEV_AE_TABLE				ae_table;
	//} FT_CCT_DEV_AE_INFO;

	//MSDK_CCT_DEV_AE_TABLE
	
	/*
	typedef struct 
	{
	unsigned short int		start_idx;
	unsigned short int		end_idx;
	unsigned int			timer_ms;
	FT_CCT_DEV_AE_TABLE		freerun_table;
	} FT_CCT_DEV_AE_BYPASS_FREERUN_PARA;
*/
	typedef struct 
	{
	unsigned int				lum_info_size;
	unsigned int				lum_info[256];
	} FT_CCT_DEV_AE_BYPASS_FREERUN_RESULT;


	typedef struct{
	unsigned int data[4096];
	}ispTable;


	typedef struct{
	unsigned char		mode;
	unsigned int	offset;
	unsigned int	length;
	unsigned int	buffer[100];
	}ispPartialShadingTable;
	
	
	typedef struct
	{
    // average
    int fRAvg;
    int fGrAvg;
    int fGbAvg;
    int fBAvg;

    // median
    unsigned int u4Median;
	}	 AP_ACDK_CDVT_RAW_ANALYSIS_RESULT_T;

	typedef struct
	{
    int i4ErrorCode;
    int i4TestCount;
    AP_ACDK_CDVT_RAW_ANALYSIS_RESULT_T rRAWAnalysisResult[1000];
	} AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T;
	
	

static bool ft_cct_init();
static bool ft_fb_init();
	
META_BOOL META_CCAP_init();

void META_CCAP_deinit();

void META_CCAP_OP(const FT_CCT_REQ* req, char* peer_buff_in);
BOOL Set_srcDev(UINT32 srcDev);
	

	
	
	
#ifdef __cplusplus
};
#endif
	//MSDK_CCT_CAL_STRUCT

	
	/*typedef enum 
	{
	OUTPUT_RAW_8BITS = 0
	,OUTPUT_YUV
	,OUTPUT_JPEG
	,OUTPUT_RGB888
	,OUTPUT_RAW_10BITS
	,OUTPUT_EXT_RAW_8BITS
	,OUTPUT_EXT_RAW_10BITS
	,OUTPUT_EXT_YUV
	} FT_CCT_CAP_OUTPUT_FORMAT;







	typedef struct 
	{
	unsigned char			gamma[1024];
	} FT_CCT_GAMMA_TABLE;




	typedef struct
	{
	bool							is_cal_result;
	defectpixel_result_struct		defectpixel;
	} FT_CCT_DEFECT_TABLE;

	typedef struct 
	{
	unsigned short int				pregain_max;
	unsigned short int				pregain_min;
	} sensor_info_struct;

	typedef struct 
	{
	dsc_info_struct					dsc_info;
	sensor_info_struct				sensor_info;
	} FT_CCT_DEV_DSC_INFO;

	

	
	

	



	


	typedef struct{
	unsigned int window_pixel;
	unsigned int window[AE_MAX_WIND_NO];
	unsigned int histogram[AE_MAX_CCM_HIST_NO];
	}aeWindowHistogram;			//[TH] add

	

	

	
	
	
	

	//---------TEST-----------------
	

	


	*/
	//------------------------------



#endif


