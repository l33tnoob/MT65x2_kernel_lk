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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   ft_public.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Lu.Zhang (MTK80251) 09/11/2009
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 03 12 2012 vend_am00076
 * [ALPS00251394] [Patch Request]
 * trunk ics
 *
 * 03 02 2012 vend_am00076
 * NULL
 * .
 *
 * 10 25 2011 nina.hsu
 * [ALPS00080644] [New_Feature][NFC] Merge NFC feature.
 * .
 *
 * 09 05 2010 siyang.miao
 * [ALPS00003981] Add reboot feature in meta
 * .
 *
 * 01 21 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 01 21 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 
 *
 * 
 *
 *******************************************************************************/
#ifndef _FT_PUBLIC_H
#define _FT_PUBLIC_H

#ifdef __cplusplus
extern "C" {

#endif

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGI ALOGI
#define LOGW ALOGW


typedef enum
{
	META_STATUS_FAILED = 0,
    META_STATUS_SUCCESS
} META_STATUS;


typedef enum
{
	META_SUCCESS = 0
	,META_FAILED
	,META_COMM_FAIL
	,META_NORESPONSE
	,META_BUFFER_LEN
	,META_FILE_BAD
	,META_LID_INVALID
	,META_INTERNAL_DB_ERR
	,META_NO_MEMORY
	,META_INVALID_ARGUMENTS
	,META_TIMEOUT
	,META_BUSY
	,META_INVALID_HANDLE
	,META_FAT_ERROR
	,META_FAT_DISK_FULL
	,META_FAT_ROOT_DIR_FULL
	,META_FAT_INVALID_FILENAME
	,META_FAT_INVALID_FILE_HANDLE
	,META_FAT_FILE_NOT_FOUND
	,META_FAT_DRIVE_NOT_FOUND
	,META_FAT_PATH_NOT_FOUND
	,META_FAT_ACCESS_DENIED
	,META_FAT_TOO_MANY_FILES
	,META_INCORRECT_TARGET_VER
	,META_COM_ERROR
	,META_BROM_CMD_ERROR
	,META_INCORRECT_BBCHIP_TYPE
	,META_BROM_ERROR
	,META_STOP_BOOTUP_PROCEDURE
	,META_CANCEL
	,META_CCT_NOT_IMPORT_PROFILE
	,META_CCT_INVALID_SENSOR_ID
	,META_CCT_TGT_NO_MEM_FOR_SINGLE_SHOT
	,META_CCT_TGT_NO_MEM_FOR_MULTI_SHOT
	,META_FUNC_NOT_IMPLEMENT_YET
	,META_CCT_NOT_IMPLEMENT_YET = META_FUNC_NOT_IMPLEMENT_YET
	,META_CCT_PREVIEW_ALREADY_STARTED
	,META_CCT_PREVIEW_ALREADY_STOPPED
	,META_CCT_READ_REG_NO_CNF
	,META_CCT_WRITE_REG_NO_CNF
	,META_CCT_TGT_ABORT_IMAGE_SENDING
	,META_CCT_READ_ONLY_REGISTER
	,META_CCT_LOAD_FROM_NVRAM_FAIL
	,META_CCT_SAVE_TO_NVRAM_FAIL
	,META_CCT_AE_INVALID_EC_LEVEL
	,META_CCT_AE_INVALID_EC_STEP
	,META_CCT_AE_ALREADY_ENABLED
	,META_CCT_AE_ALREADY_DISABLED
	,META_CCT_WB_INVALID_INDEX
	,META_CCT_NO_TGT_SENSOR_MATCH_IN_PROFILE
	,META_CCT_IMAGE_SENDING_FAME_NUM_ERROR
	,META_CCT_AE_IS_NOT_DISABLED
	,META_FAT_APP_QUOTA_FULL
	,META_IMEI_CD_ERROR
	,META_RFID_MISMATCH
	,META_NVRAM_DB_IS_NOT_LOADED_YET
	,META_CCT_ERR_CAPTURE_WIDTH_HEIGHT_TOO_SMALL
	,META_WAIT_FOR_TARGET_READY_TIMEOUT
	,META_CCT_ERR_SENSOR_ENG_SET_INVALID_VALUE
	,META_CCT_ERR_SENSOR_ENG_GROUP_NOT_EXIST
	,META_CCT_NO_TGT_ISP_MATCH_IN_PROFILE
	,META_CCT_TGT_ISP_SUPPORT_NOT_DEFINED
	,META_CCT_ERR_SENSOR_ENG_ITEM_NOT_EXIST
	,META_CCT_ERR_INVALID_COMPENSATION_MODE
	,META_CCT_ERR_USB_COM_NOT_READY
	,META_CCT_DEFECTPIXEL_CAL_UNDER_PROCESSING
	,META_CCT_ERR_DEFECTPIXEL_CAL_NO_MEM
	,META_CCT_ERR_TOO_MANY_DEFECT_PIXEL
	,META_CCT_ERR_CAPTURE_JPEG_FAIL
	,META_CCT_ERR_CAPTURE_JPEG_TIMEOUT
	,META_CCT_ERR_AF_FAIL
	,META_CCT_ERR_AF_TIMEOUT
	,META_CCT_ERR_AF_LENS_OFFSET_CAL_FAIL
	,META_CCT_ERR_PREVIEW_MUST_ENABLE
	,META_CCT_ERR_UNSUPPORT_CAPTURE_FORMAT
	,META_CCT_ERR_EXCEED_MAX_DEFECT_PIXEL
	,META_ERR_EXCEED_MAX_PEER_BUF_SIZE
	,META_CCT_ERR_INVALID_WIDTH_FACTOR
	,META_BROM_SECURITY_CHECK_FAIL
	,META_CCT_ERR_PREVIEW_MUST_DISABLE
	,META_MAUI_DB_INCONSISTENT
	,META_FAT_FILEPATH_TOO_LONG
	,META_FAT_RESTRICTED_FILEPATH
	,META_FAT_DIR_NOT_EXIST
	,META_FAT_DISK_SPACE_IS_NOT_ENOUGH
	,META_TDMB_ERR_BAND_NOT_EXIST
	,META_TDMB_ERR_FREQ_NOT_EXIST
	,META_TDMB_ERR_ENSM_NOT_EXIST
	,META_TDMB_ERR_SERV_NOT_EXIST
	,META_TDMB_ERR_SUB_CHAN_NOT_EXIST
	,META_TDMB_ERR_DEMOD_STATE
	,META_PERMISSION_DENIED
	,META_ENUMERATE_USB_FAIL
    ,META_STOP_ENUM_USB_PROCEDURE
    //----------------[TH] for CCT 6238--------------------------
    ,META_CCT_6238_AE_ALREADY_ENABLED
	,META_CCT_6238_AE_ALREADY_DISABLED
	,META_CCT_6238_AE_IS_NOT_DISABLED
    ,META_CCT_6238_ISP_FLASHLIGHT_LINEARITY_PRESTROBE_FAIL
    //-----------------------------------------------------------
    ,META_NOT_SUPPORT
	,META_LAST_RESULT
} META_RESULT;


//the ID define of ft req and cnf, it is used to ananlyze the different module.
 typedef enum
 {
   /* RF */
   FT_RF_TEST_REQ_ID = 0					   ,/*0*/
   FT_RF_TEST_CNF_ID						   ,
   /* BaseBand */
   FT_REG_READ_ID							   ,
   FT_REG_READ_CNF_ID						   ,
   FT_REG_WRITE_ID							   ,
   FT_REG_WRITE_CNF_ID						   ,/*5*/
   FT_ADC_GETMEADATA_ID 					   ,
   FT_ADC_GETMEADATA_CNF_ID 				   ,
   /* test alive */
   FT_IS_ALIVE_REQ_ID						   ,
   FT_IS_ALIVE_CNF_ID						   ,
   /* power off */
   FT_POWER_OFF_REQ_ID						   ,/*10*/
   /* unused */
   FT_RESERVED04_ID 						   ,
   /* required META_DLL version */
   FT_CHECK_META_VER_REQ_ID 				   ,
   FT_CHECK_META_VER_CNF_ID 				   ,
   /* utility command */
   FT_UTILITY_COMMAND_REQ_ID				   ,
   FT_UTILITY_COMMAND_CNF_ID				   ,/*15*/
   /* for NVRAM */
   FT_NVRAM_GET_DISK_INFO_REQ_ID			   ,
   FT_NVRAM_GET_DISK_INFO_CNF_ID			   ,
   FT_NVRAM_RESET_REQ_ID					   ,
   FT_NVRAM_RESET_CNF_ID					   ,
   FT_NVRAM_LOCK_CNF_ID 					   ,/*20*/
   FT_NVRAM_LOCK_REQ_ID 					   ,
   FT_NVRAM_READ_REQ_ID 					   ,
   FT_NVRAM_READ_CNF_ID 					   ,
   FT_NVRAM_WRITE_REQ_ID					   ,
   FT_NVRAM_WRITE_CNF_ID					   ,/*25*/
   /* FAT */
   FT_FAT_OPERATION_ID = 26 				   ,/* 26 ~ 40 */
   /* L4 Audio */
   FT_L4AUD_REQ_ID = 41 					   ,/* 41 ~ 50 */
   FT_L4AUD_CNF_ID							   ,
   /* Version Info */
   FT_VER_INFO_REQ_ID = 51					   ,/* 51 */
   FT_VER_INFO_CNF_ID						   ,
   /* CCT */
   FT_CCT_REQ_ID = 53						   ,/* 53 */
   FT_CCT_CNF_ID							   ,
   /* WiFi */
   FT_WIFI_WNDRV_SET_REQ_ID = 55			   ,/* 55 */
   FT_WIFI_WNDRV_SET_CNF_ID 				   ,
   FT_WIFI_WNDRV_QUERY_REQ_ID = 57			   ,/* 57 */
   FT_WIFI_WNDRV_QUERY_CNF_ID				   ,
   FT_WIFI_REQ_ID = 59						   ,/* 59 */
   FT_WIFI_CNF_ID							   ,  
   FT_BT_REQ_ID = 61						   ,
   FT_BT_CNF_ID 							   ,
   FT_PMIC_REG_READ_ID = 63 		   , 
   FT_PMIC_REG_READ_CNF_ID			   ,
   FT_PMIC_REG_WRITE_ID = 65		   , 
   FT_PMIC_REG_WRITE_CNF_ID 			   ,
   FT_URF_TEST_REQ_ID = 67					 ,	 /* 67 */
   FT_URF_TEST_CNF_ID				   ,
   FT_FM_REQ_ID = 69						  ,   /* 69 */
   FT_FM_CNF_ID = 70						  ,
   FT_TDMB_REQ_ID = 71				  , /* 71 */
   FT_TDMB_CNF_ID = 72				  , /* 72 */
   /* This is a special message defined to handle L1 report. */
   FT_DISPATCH_REPORT_ID					   ,
   FT_WM_METATEST_REQ_ID						,  	/* 74 */
   FT_WM_METATEST_CNF_ID						,
    // for battery dfi
   FT_WM_BAT_REQ_ID								,	/* 76 */
   FT_WM_BAT_CNF_ID								,
    //for dvbt test
   FT_WM_DVB_REQ_ID								,	/* 78 */
   FT_WM_DVB_CNF_ID								,
   FT_BATT_READ_INFO_REQ_ID=80    ,
   FT_BATT_READ_INFO_CNF_ID,
   FT_GPS_REQ_ID = 82							,
   FT_GPS_CNF_ID 							    ,
   FT_BAT_CHIPUPDATE_REQ_ID = 84	,
   FT_BAT_CHIPUPDATE_CNF_ID 			,
   FT_SDCARD_REQ_ID = 86 ,
   FT_SDCARD_CNF_ID 	 ,
   FT_LOW_POWER_REQ_ID = 88,
   FT_LOW_POWER_CNF_ID ,
   FT_GPIO_REQ_ID = 90,
   FT_GPIO_CNF_ID ,
   // For NVRAM backup & restore
   FT_NVRAM_BACKUP_REQ_ID = 94,
   FT_NVRAM_BACKUP_CNF_ID,
   FT_NVRAM_RESTORE_REQ_ID = 96,
   FT_NVRAM_RESTORE_CNF_ID,
   // For G-Sensor
   FT_GSENSOR_REQ_ID = 114,
   FT_GSENSOR_CNF_ID ,
   FT_META_MODE_LOCK_REQ_ID = 116,
   FT_META_MODE_LOCK_CNF_ID,
   // Reboot
   FT_REBOOT_REQ_ID = 118,
   // For MATV
   FT_MATV_CMD_REQ_ID = 119,
   FT_MATV_CMD_CNF_ID,
   // Customer API
   FT_CUSTOMER_REQ_ID = 121,
   FT_CUSTOMER_CNF_ID = 122,
   // Get chip ID
   FT_GET_CHIPID_REQ_ID = 123,
   FT_GET_CHIPID_CNF_ID = 124,
   // M-Sensor
   FT_MSENSOR_REQ_ID = 125,
   FT_MSENSOR_CNF_ID = 126,
   // Touch panel
   FT_CTP_REQ_ID = 127,
   FT_CTP_CNF_ID = 128,
   // ALS_PS
   FT_ALSPS_REQ_ID = 129,
   FT_ALSPS_CNF_ID = 130,
   //Gyroscope	
   FT_GYROSCOPE_REQ_ID = 131,
   FT_GYROSCOPE_CNF_ID = 132,
   // Get version info V2
   FT_VER_INFO_V2_REQ_ID = 133,
   FT_VER_INFO_V2_CNF_ID = 134,
      //CMMB
   FT_CMMB_REQ_ID = 135,
   FT_CMMB_CNF_ID = 136,

   FT_BUILD_PROP_REQ_ID = 137,
   FT_BUILD_PROP_CNF_ID = 138,

      // NFC
   FT_NFC_REQ_ID = 139,
   FT_NFC_CNF_ID = 140,

   FT_ADC_REQ_ID = 141,
   FT_ADC_CNF_ID = 142,

   FT_EMMC_REQ_ID = 143,
   FT_EMMC_CNF_ID = 144,

   FT_CRYPTFS_REQ_ID = 145,
   FT_CRYPTFS_CNF_ID = 146,

   FT_MODEM_REQ_ID = 147,
   FT_MODEM_CNF_ID = 148,

   FT_SIM_NUM_REQ_ID = 149,
   FT_SIM_NUM_CNF_ID = 150,
   
   // DFO
   FT_DFO_REQ_ID = 151,
   FT_DFO_CNF_ID = 152,

   //DRMKey
   FT_DRMKEY_REQ_ID = 153,
   FT_DRMKEY_CNF_ID = 154,

   FT_HDCP_REQ_ID = 155,
   FT_HDCP_CNF_ID = 156,

   FT_MSG_LAST_ID	
} FT_MESSAGE_ID;

 /* Header of every structure */
typedef struct
{
	unsigned short		  token;
	unsigned short		  id;
}FT_H;


//peer buff header definition, it is reserved from feature phone
typedef struct
{
	unsigned short	pdu_len;
	unsigned char	ref_count;
	unsigned char	pb_resvered;
	unsigned short	free_header_space;
	unsigned short	free_tail_space;
	
}peer_buff_struct;


typedef struct
{
	FT_H header;
	unsigned int dummy;	
}FT_POWER_OFF_REQ;

typedef enum 
{
	FT_MODEM_SRV_INVALID = 0,
	FT_MODEM_SRV_TST = 1,
	FT_MODEM_SRV_DHL = 2,
	FT_MODEM_SRV_END = 0x0fffffff
}FT_MODEM_SRV;
	
typedef enum 
{
	FT_MODEM_CH_NATIVE_INVALID = 0,
	FT_MODEM_CH_NATIVE_TST = 1,
	FT_MODEM_CH_TUNNELING = 2,
	FT_MODEM_CH_TUNNELING_IGNORE_CKSM = 3,
	FT_MODEM_CH_END = 0x0fffffff
}FT_MODEM_CH_TYPE;
	
typedef struct 
{
	FT_MODEM_SRV md_service;
	FT_MODEM_CH_TYPE ch_type;
	unsigned char reserved;
}MODEM_CAPABILITY; 

typedef struct 
{
	MODEM_CAPABILITY modem_cap[8];
}MODEM_CAPABILITY_LIST;


    /********************************************************************************
    //FUNCTION:
    //		WriteDataToPC
    //DESCRIPTION:
    //		this function is called to send cnf data to PC side. the local_len + Peer Len must less than 2031 bytes
    //		and peer len must less than 2000. so when it do not meet, module should divide the packet to
    //		many small packet to sent.
    //
    //PARAMETERS:
    //		Local_buf:	[IN]	local buf (cnf cmd)
    //		Local_len: 	[IN]	local buf size
    //		Peer_buf		[IN]	peer buff
    //		Peer_len:		[IN]	peer buff size
    //RETURN VALUE:
    //		TRUE is success, otherwise is fail
    //
    //DEPENDENCY:
    //		the FT module must have been loaded.
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/

int WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);

int getBootMode(void);

int getComportType(void);

#ifdef __cplusplus
};
#endif

#endif
