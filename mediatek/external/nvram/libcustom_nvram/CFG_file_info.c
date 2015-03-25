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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   CFG_file_info.c
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Configuration File List
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/
/*
#ifndef __CFG_FILE_INFO_H__
#define __CFG_FILE_INFO_H__

#ifdef __cplusplus
extern "C"
{
#endif
*/
#include "CFG_file_public.h"
#include "libnvram.h"
#include "inc/CFG_file_lid.h"
#include "inc/CFG_module_file.h"
#include "inc/CFG_module_default.h"
#include "inc/CFG_file_info.h"
#include "CFG_file_info_custom.h"
#include "CFG_Dfo_File.h"
#include "CFG_Dfo_Default.h"
#include <stdio.h>
#include <cutils/properties.h>

#define MAX_FILENAMELEN 128;
#define RESERVE_PATH	"Reserved"
#define RESERVE_VER	"000"

int BT_ConvertFunc(int, int, char*, char*);
const TCFG_FILE g_akCFG_File[]=
{
    //nvram version information
	{ "/data/nvram/APCFG/APRDCL/FILE_VER",		VER(AP_CFG_FILE_VER_INFO_LID), 		4,								
	CFG_FILE_VER_FILE_REC_TOTAL,			DEFAULT_ZERO,						0,  DataReset , NULL },

#ifdef MTK_COMBO_SUPPORT
	{ "/data/nvram/APCFG/APRDEB/BT_Addr",		VER(AP_CFG_RDEB_FILE_BT_ADDR_LID),	CFG_FILE_BT_ADDR_REC_SIZE,		
	CFG_FILE_BT_ADDR_REC_TOTAL,			DEFAULT_ZERO,				0,  DataConvert , BT_ConvertFunc},
#else
	{ "/data/nvram/APCFG/APRDEB/BT_Addr",		VER(AP_CFG_RDEB_FILE_BT_ADDR_LID),	CFG_FILE_BT_ADDR_REC_SIZE,		
	CFG_FILE_BT_ADDR_REC_TOTAL,			SIGNLE_DEFUALT_REC,			(char *)&stBtDefault,  DataReset , NULL},
#endif
#if 0
	{ "/data/nvram/APCFG/APRDEB/WIFI",	    	VER(AP_CFG_RDEB_FILE_WIFI_LID),		    CFG_FILE_WIFI_REC_SIZE,	
	CFG_FILE_WIFI_REC_TOTAL,		    	SIGNLE_DEFUALT_REC,				    (char *)&stWifiCfgDefault, DataReset , NULL},
#endif
	{ "/data/nvram/APCFG/APRDCL/AUXADC",			VER(AP_CFG_RDCL_FILE_AUXADC_LID),	CFG_FILE_AUXADC_REC_SIZE,		
	CFG_FILE_AUXADC_REC_TOTAL,				SIGNLE_DEFUALT_REC,					(char *)&stADCDefualt, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/CAMERA_Para",	VER(AP_CFG_RDCL_CAMERA_PARA_LID),   CFG_FILE_CAMERA_PARA_REC_SIZE,	
	CFG_FILE_CAMERA_PARA_REC_TOTAL,			DEFAULT_ZERO,						0, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/CAMERA_3A",	   	VER(AP_CFG_RDCL_CAMERA_3A_LID),     CFG_FILE_CAMERA_3A_REC_SIZE,	
	CFG_FILE_CAMERA_3A_REC_TOTAL,			    DEFAULT_ZERO,						0, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/CAMERA_SHADING",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,	
	CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/CAMERA_DEFECT",	VER(AP_CFG_RDCL_CAMERA_DEFECT_LID), CFG_FILE_CAMERA_DEFECT_REC_SIZE,	
	CFG_FILE_CAMERA_DEFECT_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/CAMERA_SENSOR",	VER(AP_CFG_RDCL_CAMERA_SENSOR_LID), CFG_FILE_CAMERA_SENSOR_REC_SIZE,	
	CFG_FILE_CAMERA_SENSOR_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL},
	
    { "/data/nvram/APCFG/APRDCL/CAMERA_LENS",	VER(AP_CFG_RDCL_CAMERA_LENS_LID),   CFG_FILE_CAMERA_LENS_REC_SIZE,	
	CFG_FILE_CAMERA_LENS_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL},
		
    { "/data/nvram/APCFG/APRDCL/UART",			VER(AP_CFG_RDCL_UART_LID), 			CFG_FILE_UART_CONFIG_SIZE,	
	CFG_FILE_UART_CONFIG_TOTAL,					SIGNLE_DEFUALT_REC,				(char *)&stUARTConfigDefault, DataReset , NULL},

    { "/data/nvram/APCFG/APRDCL/FACTORY",		VER(AP_CFG_RDCL_FACTORY_LID), 		CFG_FILE_FACTORY_REC_SIZE,	
	CFG_FILE_FACTORY_REC_TOTAL,				DEFAULT_ZERO,			    	    0, DataReset , NULL},    
	
    { "/data/nvram/APCFG/APRDCL/BWCS",			VER(AP_CFG_RDCL_BWCS_LID), 	        CFG_FILE_BWCS_CONFIG_SIZE,	
	CFG_FILE_BWCS_CONFIG_TOTAL,				SIGNLE_DEFUALT_REC,					(char *)&stBWCSConfigDefault, DataReset , NULL},	

    { "/data/nvram/APCFG/APRDCL/HWMON_ACC",		VER(AP_CFG_RDCL_HWMON_ACC_LID), 	CFG_FILE_HWMON_ACC_REC_SIZE,	
	CFG_FILE_HWMON_ACC_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset , NULL},	
	{ "/data/nvram/APCFG/APRDCL/HWMON_GYRO",	VER(AP_CFG_RDCL_HWMON_GYRO_LID), 	CFG_FILE_HWMON_GYRO_REC_SIZE,	
	CFG_FILE_HWMON_GYRO_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset, NULL},
#if 0
    { "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM",	VER(AP_CFG_RDEB_WIFI_CUSTOM_LID),	CFG_FILE_WIFI_CUSTOM_REC_SIZE,	
	CFG_FILE_WIFI_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stWifiCustomDefault, DataReset , NULL},
#endif
    { "/data/nvram/APCFG/APRDEB/OMADM_USB",		VER(AP_CFG_RDEB_OMADM_USB_LID),		CFG_FILE_OMADMUSB_REC_SIZE,	
	CFG_FILE_OMADMUSB_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stOMADMUSBDefualt,DataReset , NULL},
   // { "/nvram/APCFG/APRDCL/ADDED",	VER(AP_CFG_RDCL_ADDED_LID),	CFG_FILE_added_CONFIG_SIZE,	
   //	CFG_FILE_added_CONFIG_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stAddedDefault},
    { "/data/nvram/APCFG/APRDCL/Voice_Recognize_Param",   VER(AP_CFG_RDCL_FILE_VOICE_RECOGNIZE_PARAM_LID), CFG_FILE_VOICE_RECOGNIZE_PAR_SIZE,
	CFG_FILE_VOICE_RECOGNIZE_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Voice_Recognize_Par_default, DataReset , NULL},
//Reserved ten item
    { "/data/nvram/APCFG/APRDCL/Audio_AudEnh_Control_Opt",   VER(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID), CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_SIZE,
            CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&AUDENH_Control_Option_Par_default, DataReset , NULL},
    { "/data/nvram/APCFG/APRDEB/Dfo",   VER(AP_CFG_CUSTOM_FILE_DFO_LID), CFG_FILE_DFO_CONFIG_SIZE,
            CFG_FILE_DFO_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&stDfoConfigDefault, DataReset , NULL},
    { "/data/nvram/APCFG/APRDCL/Audio_VOIP_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID), CFG_FILE_AUDIO_VOIP_PAR_SIZE,
            CFG_FILE_AUDIO_VOIP_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Audio_VOIP_Par_default, DataReset , NULL},
    /*yucong add for PS calibration*/
	{ "/data/nvram/APCFG/APRDCL/HWMON_PS",	VER(AP_CFG_RDCL_HWMON_PS_LID), 	CFG_FILE_HWMON_PS_REC_SIZE,	
			CFG_FILE_HWMON_PS_REC_TOTAL,		    DEFAULT_ZERO,					    0, DataReset, NULL},
	{ "/data/nvram/APCFG/APRDCL/MD_Type",   VER(AP_CFG_FILE_MDTYPE_LID), CFG_FILE_MDTYPE_CONFIG_SIZE,
            CFG_FILE_MDTYPE_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&stMDTypeDefault, DataReset , NULL},
    { "/data/nvram/APCFG/APRDCL/EXT_MD_Type",   VER(AP_CFG_FILE_EXT_MDTYPE_LID), CFG_FILE_EXT_MDTYPE_CONFIG_SIZE,
            CFG_FILE_EXT_MDTYPE_CONFIG_TOTAL, SIGNLE_DEFUALT_REC,    (char *)&stEXTMDTypeDefault, DataReset , NULL},
    { "/data/nvram/APCFG/APRDCL/SDIO",   VER(AP_CFG_RDCL_FILE_SDIO_LID), CFG_FILE_SDIO_CONFIG_SIZE,
            CFG_FILE_SDIO_CONFIG_TOTAL, SIGNLE_DEFUALT_REC  , (char *)&stSDIOConfigDefault, DataReset , NULL},
    { RESERVE_PATH,	RESERVE_VER,	0,	0,	SIGNLE_DEFUALT_REC,	NULL ,	DataReset , NULL},
    { RESERVE_PATH,	RESERVE_VER,	0,	0,	SIGNLE_DEFUALT_REC,	NULL ,	DataReset , NULL},
    { RESERVE_PATH,	RESERVE_VER,	0,	0,	SIGNLE_DEFUALT_REC,	NULL ,	DataReset , NULL}, 
//Reserved   
};
int iCustomBeginLID=AP_CFG_CUSTOM_BEGIN_LID;
extern int iCustomBeginLID;
int iFileVerInfoLID=AP_CFG_FILE_VER_INFO_LID;
extern int iFileVerInfoLID;
int iFileBTAddrLID=AP_CFG_RDEB_FILE_BT_ADDR_LID;
extern int iFileBTAddrLID;
int iFileAuxADCLID=AP_CFG_RDCL_FILE_AUXADC_LID;
extern int iFileAuxADCLID;
int iFileOMADMUSBLID=AP_CFG_RDEB_OMADM_USB_LID;
extern int iFileOMADMUSBLID;


typedef struct
{
	char	cFileName[128];
	unsigned int iLID;
}FileName;
typedef struct 
{
	char cCheckFileName[128];
	int  iCheckFileIndex;
	
}CheckFileName;

typedef struct 
{
	char cRecordCallerName[128];
	int  iRecordCallerIndex;
}RecordCallerName;


typedef struct
{
   int lid;
   unsigned char meta_mode_write;
   unsigned char security_enable;
} TCFG_FILE_PROTECT;
	


FileName aBackupToBinRegion[]=
{
	{"FILE_VER",AP_CFG_FILE_VER_INFO_LID},
	{"BT_Addr",AP_CFG_RDEB_FILE_BT_ADDR_LID},
	{"WIFI",AP_CFG_RDEB_FILE_WIFI_LID},
	{"AUXADC",AP_CFG_RDCL_FILE_AUXADC_LID},
	{"FACTORY",AP_CFG_RDCL_FACTORY_LID},
	{"HWMON_ACC",AP_CFG_RDCL_HWMON_ACC_LID},
	{"HWMON_GYRO",AP_CFG_RDCL_HWMON_GYRO_LID},
	{"HWMON_PS",AP_CFG_RDCL_HWMON_PS_LID},//yucong add for ALSPS calibration
	{"WIFI_CUSTOM",AP_CFG_RDEB_WIFI_CUSTOM_LID},
	{"GPS",AP_CFG_CUSTOM_FILE_GPS_LID},
#ifndef MTK_PRODUCT_INFO_SUPPORT
	{"PRODUCT_INFO",AP_CFG_REEB_PRODUCT_INFO_LID},
#endif
#ifdef	MTK_SDIORETRY_SUPPORT
	{"SDIO_RETRY",AP_CFG_RDEB_SDIO_RETRY_LID},
#endif
#if	defined(MTK_MT8193_HDMI_SUPPORT)||defined(MTK_INTERNAL_HDCP_SUPPORT)
    {"Hdcp_Key_Table", AP_CFG_RDCL_FILE_HDCP_KEY_LID},
#endif
    {"Dfo", AP_CFG_CUSTOM_FILE_DFO_LID},
    {"SDIO", AP_CFG_RDCL_FILE_SDIO_LID},
};
FileName aPerformance[]=
{
	{"CAMERA_Para",AP_CFG_RDCL_CAMERA_PARA_LID},
	{"CAMERA_3A",AP_CFG_RDCL_CAMERA_3A_LID},
	{"CAMERA_SHADING",AP_CFG_RDCL_CAMERA_SHADING_LID},
	{"CAMERA_DEFECT",AP_CFG_RDCL_CAMERA_DEFECT_LID},
	{"CAMERA_SENSOR",AP_CFG_RDCL_CAMERA_SENSOR_LID},
	{"CAMERA_LENS",AP_CFG_RDCL_CAMERA_LENS_LID}
};
const TCFG_FILE_PROTECT g_akCFG_File_Protect[]=
{
  // {AP_CFG_RDEB_FILE_BT_ADDR_LID,1,1},
};
CheckFileName aCheckExistList[]=
{
///	{"/data/nvram/APCFG/APRDCL/FILE_VER",		0},
	
	 
};

RecordCallerName aRecordCallerList[]=
{
	{"Barcode",		1},
	{"IMEI",		2},
};



extern const TCFG_FILE g_akCFG_File[];
extern FileName aBackupToBinRegion[];
extern FileName aPerformance[];
extern CheckFileName aCheckExistList[];
extern const TCFG_FILE_PROTECT g_akCFG_File_Protect[];



const unsigned int g_i4CFG_File_Count = sizeof(g_akCFG_File)/sizeof(TCFG_FILE);
const unsigned int g_Backup_File_Count = sizeof(aBackupToBinRegion)/(sizeof(FileName));
const unsigned int g_Performance_File_Count = sizeof(aPerformance)/(sizeof(FileName));
const unsigned int g_Check_File_Count = sizeof(aCheckExistList)/(sizeof(CheckFileName));
const unsigned int g_i4CFG_File_Protect_Count = sizeof(g_akCFG_File_Protect)/(sizeof(TCFG_FILE_PROTECT));
const unsigned int g_RecordCaller_Count = sizeof(aRecordCallerList)/(sizeof(RecordCallerName));

extern const unsigned int g_i4CFG_File_Count;
extern const unsigned int g_Backup_File_Count;
extern const unsigned int g_Performance_File_Count;
extern const unsigned int g_Check_File_Count ;
extern const unsigned int g_i4CFG_File_Protect_Count;
extern const unsigned int g_RecordCaller_Count;
//misc log feature 
int nvram_misc_log_feature = 0;
int nvram_misc_log_block_offset = 2;


//check exist feature
int nvram_check_exist_feature = 0;
int nvram_check_exist_block_offset = 1;


//read back check feature
int nvram_read_back_feature = 0;

pfConvertFunc aNvRamConvertFuncTable[]=
{
  NULL,//AP_CFG_FILE_VER_INFO_LID
  NULL,//AP_CFG_RDEB_FILE_BT_ADDR_LID
  NULL,//AP_CFG_RDCL_FILE_AUXADC_LID
  NULL,//AP_CFG_RDCL_CAMERA_PARA_LID
  NULL,//AP_CFG_RDCL_CAMERA_3A_LID
  NULL,//AP_CFG_RDCL_CAMERA_SHADING_LID
  NULL,//AP_CFG_RDCL_CAMERA_DEFECT_LID
  NULL,//AP_CFG_RDCL_CAMERA_SENSOR_LID
  NULL,//AP_CFG_RDCL_CAMERA_LENS_LID
  NULL,//AP_CFG_RDCL_UART_LID
  NULL,//AP_CFG_RDCL_FACTORY_LID
  NULL,//AP_CFG_RDCL_BWCS_LID
  NULL,//AP_CFG_RDCL_HWMON_ACC_LID
  NULL,//AP_CFG_RDCL_HWMON_GYRO_LID
  NULL,//AP_CFG_RDCL_HWMON_PS_LID
  NULL,//AP_CFG_RDCL_FILE_AUDIO_LID
  NULL,//AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID
  NULL,//AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID
  NULL,//AP_CFG_CUSTOM_FILE_GPS_LID
  NULL,//AP_CFG_RDEB_FILE_WIFI_LID
  NULL,//AP_CFG_RDEB_WIFI_CUSTOM_LID
  NULL,//AP_CFG_RDEB_OMADM_USB_LID
  NULL,//AP_CFG_REEB_PRODUCT_INFO_LID
#ifdef	MTK_SDIORETRY_SUPPORT
  NULL,//AP_CFG_REEB_SDIO_RETRY_LID
#endif
  NULL,//AP_CFG_CUSTOM_FILE_DFO_LID
};
extern pfConvertFunc aNvRamConvertFuncTable[];

//Add for new nvram partition feature
pfCallbackForDaemon callback_for_nvram_daemon = NULL;
extern pfCallbackForDaemon callback_for_nvram_daemon;

#ifdef MTK_PRODUCT_INFO_SUPPORT
extern bool nvram_new_partition_support()
{
	return true;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = 
{
	{ AP_CFG_REEB_PRODUCT_INFO_LID, 0, 1024 * 1024 },
};
const unsigned int g_new_nvram_lid_count = sizeof(g_new_nvram_lid)/sizeof(TABLE_FOR_SPECIAL_LID);
const char *nvram_new_partition_name = "/dev/pro_info";
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#else
extern bool nvram_new_partition_support()
{
	return false;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = {0 , 0, 0}; 
const unsigned int g_new_nvram_lid_count = 0;
const char *nvram_new_partition_name = NULL;
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#endif
//end new nvram partition feature

#ifdef MTK_COMBO_SUPPORT
int nvram_bt_default_value(unsigned char *ucNvRamData)
{
    unsigned long chipId;
    int  chipId_ready_retry = 0;
    char chipId_val[PROPERTY_VALUE_MAX];
    int  rec_size = CFG_FILE_BT_ADDR_REC_SIZE;
    
    do {
        if(property_get("persist.mtk.wcn.combo.chipid", chipId_val, NULL) &&
            0 != strcmp(chipId_val, "-1")){
            chipId = strtoul(chipId_val, NULL, 16);
            break;
        }
        else {
            chipId_ready_retry ++;
            usleep(500000);
        }
    } while(chipId_ready_retry < 10);
    
    NVRAM_LOG("Get combo chip id retry %d\n", chipId_ready_retry);
    if (chipId_ready_retry >= 10){
        NVRAM_LOG("Get combo chip id fails!\n");
        return false;
    }
    else{
        NVRAM_LOG("Combo chip id %x\n", chipId);
        
        switch(chipId)
        {
          case 0x6620:
            /* NVRAM is MT6620 default */
            memcpy(ucNvRamData, &stBtDefault_6620, rec_size);
            break;
          case 0x6628:
            /* NVRAM is MT6628 default */
            memcpy(ucNvRamData, &stBtDefault_6628, rec_size);
            break;
          case 0x6572:
            /* NVRAM is MT6572 default */
            memcpy(ucNvRamData, &stBtDefault_6572, rec_size);
            break;
          case 0x6582:
            /* NVRAM is MT6582 default */
            memcpy(ucNvRamData, &stBtDefault_6582, rec_size);
            break;
          case 0x6592:
            /* NVRAM is MT6592 default */
            memcpy(ucNvRamData, &stBtDefault_6592, rec_size);
            break;
          case 0x6571:
            /* NVRAM is MT6571 default */
            memcpy(ucNvRamData, &stBtDefault_6571, rec_size);
            break;
          case 0x6630:
            /* NVRAM is MT6630 default */
            memcpy(ucNvRamData, &stBtDefault_6630, rec_size);
            break;
          default:
            NVRAM_LOG("Unknown combo chip id\n");
            return false;
        }
        
        return true;
    }
}

int BT_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem, char *pDstMem)
{
    int rec_size = CFG_FILE_BT_ADDR_REC_SIZE;
    int rec_num = CFG_FILE_BT_ADDR_REC_TOTAL;
    
    if(NULL == pSrcMem || NULL == pDstMem){
        return false;
    }
    else{
        if(0 == CurrentVerID && 1 == NewVerID){
            memcpy(pDstMem, pSrcMem, 30); // Keep the customization data
            memset(pDstMem + 30, 0, rec_size*rec_num - 30);
            return true;
        }
        return false;
    }   
}
#else
int nvram_bt_default_value(unsigned char *ucNvRamData)
{
    return false;
}

int BT_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem, char *pDstMem)
{
    return false;
}
#endif

#ifdef MTK_EMMC_SUPPORT
extern bool nvram_emmc_support()
{
	return true;
}
#else
extern bool nvram_emmc_support()
{
	return false;
}
#endif
/*
#ifdef __cplusplus
}
#endif

#endif
*/
