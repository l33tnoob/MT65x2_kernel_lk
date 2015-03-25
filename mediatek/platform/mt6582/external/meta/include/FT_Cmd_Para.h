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
 *   ft_private.h
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
 * 09 02 2010 sean.cheng
 * [ALPS00003477] [Need Patch] [Volunteer Patch] ALPS.W10.34 migrate camera function from 2.1 to 2.2
 * .Turn the CCAP / CCT Meta function
 *
 * 08 28 2010 qiuhuan.zhao
 * [ALPS00123522] [GPS] Android 2.2 porting
 * .
 *
 * 08 28 2010 qiuhuan.zhao
 * [ALPS00123522] [GPS] Android 2.2 porting
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for META mode check in.
 *
 * 08 14 2010 chipeng.chang
 * [ALPS00003297] [Need Patch] [Volunteer Patch] android 2.2 migration
 * modify meta android.mk include audioflinger.
 *
 * 07 16 2010 siyang.miao
 * [ALPS00122025]TST/FT for G-Sensor calibration
 * .
 *
 * 03 18 2010 lu.zhang
 * [ALPS00004362]CCAP APIs
 * for CCAP APIs
 *
 * 03 09 2010 ch.yeh
 * [ALPS00001276][BT]Migration to Android 2.1
 * [BT][meta]enable Bluetooth META function.
 *
 * 02 26 2010 lu.zhang
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


#ifndef __FT_PRIVATE_H__
#define __FT_PRIVATE_H__

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGI ALOGI
#define LOGW ALOGW


#include "meta_ccap_para.h"
#ifdef __cplusplus
extern "C"{
#endif

#include "meta_common.h"
#include "meta.h"
#include "FT_Public.h"
#include "Meta_APEditor_Para.h"
#include "meta_sdcard_para.h"
#include "Meta_GPIO_Para.h"
#include "meta_vibrator_para.h"
#include "meta_audio_para.h"
#include "meta_gsensor_para.h"
#include "meta_gyroscope_para.h"
//#ifdef FT_MATV_FEATURE
#include "meta_matv_para.h"
//#endif

#ifdef FT_WIFI_FEATURE
#include "meta_wifi_para.h"
#endif

#include "meta_msensor_para.h"
#include "meta_alsps_para.h"
#include "meta_touch_para.h"

#ifdef FT_HDCP_FEATURE
#include "meta_hdcp_para.h"
#endif


#define FTT_DBG 1

// Block stage definition for NVRam backup & restore
#define BLK_CREATE	0x01
#define BLK_WRITE	0x02
#define BLK_EOF		0x04

// the maximum size of peer buf for NVRAM backup & restore
#define NVRAM_PEER_MAX_LEN 2000

#define CHIP_RID_PATH	"/proc/rid"
#define CHIP_RID_LEN 	16 

typedef struct
{
    FT_H			header;						// ft module header, is used to differential the module
    unsigned int	dummy;						// extend alignment to 4 bytes
} FT_IS_ALIVE_REQ;

typedef struct
{
    FT_H			header;						// ft module header, is used to differential the module
    unsigned int	dummy;						// extend alignment to 4 bytes
} FT_IS_ALIVE_CNF;

//------------Primitive.sel----------------------------------------------------------------------

/********************************************
* Generic Primitives for FM driver
********************************************/

#ifdef FT_FM_FEATURE
#include "meta_fm_para.h"
#endif

/********************************************
* Generic Primitives for CPU Registor READ/WRITE
********************************************/
#include "meta_cpu_para.h"
//#include "meta_ccap_para.h"

/********************************************
* Generic Primitives for Peripheral Module
********************************************/

//#include "Meta_NLED_Para.h"
#include "meta_lcdbk_para.h"
#include "meta_keypadbk_para.h"
#include "meta_lcdft_para.h"

typedef enum
{
	 FT_UTILCMD_CHECK_IF_FUNC_EXIST = 0
	,FT_UTILCMD_CHECK_IF_ISP_SUPPORT
	,FT_UTILCMD_QUERY_BT_MODULE_ID
	,FT_UTILCMD_ENABLE_WATCHDOG_TIMER
	,FT_UTILCMD_CHECK_IF_ACOUSTIC16_SUPPORT
	,FT_UTILCMD_CHECK_IF_AUDIOPARAM45_SUPPORT
	,FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH
	,FT_UTILCMD_QUERY_PMIC_ID
	,FT_UTILCMD_BT_POWER_ON
	,FT_UTILCMD_KEYPAD_LED_ONOFF
	,FT_UTILCMD_VIBRATOR_ONOFF
	,FT_UTILCMD_QUERY_LOCAL_TIME
	,FT_UTILCMD_CHECK_IF_WIFI_ALC_SUPPORT
	,FT_UTILCMD_RF_ITC_PCL
	,FT_UTILCMD_CHECK_IF_DRC_SUPPORT
	,FT_UTILCMD_CHECK_IF_BT_POWERON
	,FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL
	,FT_UTILCMD_SIGNAL_INDICATOR_ONOFF
	,FT_UTILCMD_SET_CLEAN_BOOT_FLAG
	,FT_UTILCMD_LCD_COLOR_TEST
	,FT_UTILCMD_SAVE_MOBILE_LOG
	,FT_UTILCMD_OPEN_DUMP_LOG
	,FT_UTILCMD_END
} FtUtilCmdType;


/********************************************
* Generic Primitives for Peripheral Module to query the supported function
********************************************/

typedef struct
{
	unsigned int		query_ft_msg_id;
	unsigned int		query_op_code;
}FtUtilCheckIfFuncExist;

/********************************************
*Get Modem Information
********************************************/
typedef enum 
{
	FT_MODEM_OP_QUERY_INFO = 0,
	FT_MODEM_OP_CAPABILITY_LIST = 1,
	FT_MODEM_OP_SET_MODEMTYPE = 2,
	FT_MODEM_OP_GET_CURENTMODEMTYPE = 3,
	FT_MODEM_OP_QUERY_MDIMGTYPE = 4,
	FT_MODEM_END = 0x0fffffff
}FT_MODEM_OP;

typedef struct 
{
	unsigned char reserved;
}MODEM_QUERY_INFO_REQ;
	
typedef struct 
{
	unsigned int modem_number;
	unsigned int modem_id;
}MODEM_QUERY_INFO_CNF;

typedef struct 
{
    unsigned char reserved;
}MODEM_CAPABILITY_LIST_REQ;

typedef struct 
{
	MODEM_CAPABILITY modem_cap[8];
}MODEM_CAPABILITY_LIST_CNF; 


typedef struct 
{
	unsigned int modem_id;
	unsigned int modem_type;
}MODEM_SET_MODEMTYPE_REQ;

typedef struct 
{
	unsigned char reserved;	
}MODEM_SET_MODEMTYPE_CNF;

typedef struct 
{
	unsigned int modem_id;
}MODEM_GET_CURRENTMODEMTYPE_REQ;

typedef struct 
{
	unsigned int current_modem_type;
}MODEM_GET_CURENTMODEMTYPE_CNF;

typedef struct 
{
	unsigned int modem_id;	
}MODEM_QUERY_MDIMGTYPE_REQ;

typedef struct 
{
	unsigned int mdimg_type[16];
}MODEM_QUERY_MDIMGTYPE_CNF;

	
typedef union 
{
	MODEM_QUERY_INFO_REQ query_modem_info_req; 
	MODEM_CAPABILITY_LIST_REQ query_modem_cap_req;
	MODEM_SET_MODEMTYPE_REQ set_modem_type_req;
	MODEM_GET_CURRENTMODEMTYPE_REQ get_currentmodem_type_req; 
	MODEM_QUERY_MDIMGTYPE_REQ query_modem_imgtype_req;
}FT_MODEM_CMD;
	
typedef union 
{
	MODEM_QUERY_INFO_CNF query_modem_info_cnf;
	MODEM_CAPABILITY_LIST_CNF query_modem_cap_cnf;
	MODEM_SET_MODEMTYPE_CNF set_modem_type_cnf;
	MODEM_GET_CURENTMODEMTYPE_CNF get_currentmodem_type_cnf;
	MODEM_QUERY_MDIMGTYPE_CNF query_modem_imgtype_cnf;
}FT_MODEM_RESULT;
		
typedef struct 
{
	FT_H header;
	FT_MODEM_OP	type;
	FT_MODEM_CMD cmd;
}FT_MODEM_REQ;
		
typedef struct 
{
	FT_H header;
	FT_MODEM_OP type;
	unsigned char status;
	FT_MODEM_RESULT result;
}FT_MODEM_CNF;


/********************************************
* it is used to set clean boot flag
********************************************/

typedef struct
{
    int		Notused;
} SetCleanBootFlag_REQ;

typedef struct
{
    BOOL	drv_statsu;							//inidicate the result of setting clean boot
} SetCleanBootFlag_CNF;


/********************************************
* it is used to save mobile log
********************************************/

typedef struct
{
    int		reserved;
} SAVE_MOBILE_LOG_REQ;

typedef struct
{
    BOOL	drv_status;							
} SAVE_MOBILE_LOG_CNF;

typedef struct
{
    int		reserved;
} OPEN_DUMP_LOG_REQ;

typedef struct
{
    BOOL	drv_status;							
} OPEN_DUMP_LOG_CNF;

typedef union
{
    FtUtilCheckIfFuncExist	CheckIfFuncExist;
    WatchDog_REQ			m_WatchDogReq;
    KeypadBK_REQ			m_KeypadBKReq;
    LCDLevel_REQ			m_LCDReq;
    NLED_REQ				m_NLEDReq;
    SetCleanBootFlag_REQ	m_SetCleanBootFlagReq;
    LCDFt_REQ         m_LCDColorTestReq;
    SAVE_MOBILE_LOG_REQ     m_SaveMobileLogReq;
	OPEN_DUMP_LOG_REQ		m_OpenDumpLogReq;
    unsigned int			dummy;
} FtUtilCmdReq_U;

typedef union
{
    FtUtilCheckIfFuncExist	CheckIfFuncExist;
    WatchDog_CNF			m_WatchDogCnf;
    KeypadBK_CNF			m_KeypadBKCnf;
    LCDLevel_CNF			m_LCDCnf;
    NLED_CNF     			m_NLEDCnf;
    SetCleanBootFlag_CNF	m_SetCleanBootFlagCnf;
    LCDFt_CNF         m_LCDColorTestCNF;
    SAVE_MOBILE_LOG_CNF     m_SaveMobileLogCnf;
	OPEN_DUMP_LOG_CNF 		m_OpenDumpLogCnf;
    unsigned int			dummy;
} FtUtilCmdCnf_U;


typedef struct
{
    FT_H            header;	//ft header
    FtUtilCmdType   type;	//cmd type
    FtUtilCmdReq_U  cmd;	//cmd parameter
} FT_UTILITY_COMMAND_REQ;

typedef struct
{
    FT_H            header;	//ft header
    FtUtilCmdType   type;	//cmd type
    FtUtilCmdCnf_U  result;	//module cmd result
    unsigned char   status;	//ft status: 0 is success
} FT_UTILITY_COMMAND_CNF;

/********************************************
* it is used to support the version information of target side
********************************************/

typedef struct
{
    FT_H    header;		//ft header
} FT_VER_INFO_REQ;

typedef struct
{
    FT_H        header;			//ft header
    kal_uint8   bb_chip[64];
    kal_uint8   eco_ver[4];
    kal_uint8   sw_time[64];
    kal_uint8   dsp_fw[64];
    kal_uint8   dsp_patch[64];
    kal_uint8   sw_ver[64];
    kal_uint8   hw_ver[64];
    kal_uint8   melody_ver[64];
    unsigned char	 status;	//ft status: 0 is success
} FT_VER_INFO_CNF;


/********************************************
* NVRAM backup & restore
********************************************/

typedef struct
{
    unsigned int    file_size;
    unsigned char   file_ID;
    unsigned char   stage;
} FT_STREAM_BLOCK;

typedef struct
{
    FT_H    header;
    char    buffer[1024];
    int     count;
    int     mode;
} FT_NVRAM_BACKUP_REQ;

typedef struct
{
    FT_H            header;
    FT_STREAM_BLOCK block;
    unsigned char   status;
} FT_NVRAM_BACKUP_CNF;

typedef struct
{
    FT_H            header;
    FT_STREAM_BLOCK block;
} FT_NVRAM_RESTORE_REQ;

typedef struct
{
	FT_H		header;
	unsigned char	status;
} FT_NVRAM_RESTORE_CNF;


/********************************************
* it is used to check ft version information
********************************************/
/*
typedef struct
{
    FT_H			header;				//ft header
    kal_uint32 	 	meta_ver_from_pc;	//dll version information from PC
    kal_uint8		dummy[256];
} FT_CHECK_META_VER_REQ;

typedef struct
{
    FT_H			header;				//ft header
    kal_uint32 	 	meta_ver_required_by_target;	//dll version information from target
    kal_uint8		dummy[256];
    unsigned char	status;				//ft status: 0 is success
} FT_CHECK_META_VER_CNF;
*/

/********************************************
* Reboot
********************************************/
typedef struct
{
    FT_H            header;
    unsigned int    delay;
    unsigned int    dummy;
} FT_META_REBOOT_REQ;

/********************************************
* Custom API
********************************************/
typedef enum
{
    FT_CUSTOMER_OP_BASIC = 0
    ,FT_CUSTOMER_OP_END

} META_CUSTOMER_CMD_TYPE;

typedef union
{
    unsigned char	m_u1Dummy;
} META_CUSTOMER_CMD_U;

typedef union
{
    unsigned char  m_u1Dummy; 
} META_CUSTOMER_CNF_U;

typedef struct
{
    FT_H                    header;
    META_CUSTOMER_CMD_TYPE  type;
    META_CUSTOMER_CMD_U     cmd;	
} FT_CUSTOMER_REQ;

typedef struct
{
    FT_H                    header;
    META_CUSTOMER_CMD_TYPE  type;
    unsigned char           status;
    META_CUSTOMER_CNF_U     result;			
} FT_CUSTOMER_CNF;

/********************************************
 * Get chip ID
 ********************************************/
typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_GET_CHIPID_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   chipId[17];
    unsigned char   status;
} FT_GET_CHIPID_CNF;

/********************************************
 * M-Sensor
 ********************************************/
typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_MSENSOR_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   status;
} FT_MSENSOR_CNF;

/********************************************
 * ALSPS
 ********************************************/
typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_ALSPS_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   status;
} FT_ALSPS_CNF;

/********************************************
* it is used to support the version information of target side
********************************************/
typedef struct
{
	FT_H header;
	unsigned char tag[64];
}FT_BUILD_PROP_REQ;

typedef struct
{
	FT_H header;
	unsigned char content[128];
	int		status;
}FT_BUILD_PROP_CNF;

typedef struct
{
    FT_H		 header;		//ft header
} FT_VER_INFO_V2_REQ;

typedef struct
{
    FT_H		header;			//ft header
    kal_uint8	bb_chip[64];
    kal_uint8	eco_ver[4];
    kal_uint8	sw_time[64];
    kal_uint8	dsp_fw[64];
    kal_uint8	dsp_patch[64];
    kal_uint8	sw_ver[64];
    kal_uint8	hw_ver[64];
    kal_uint8	melody_ver[64];
    kal_uint8    build_disp_id[64];
    unsigned char	 status;	//ft status: 0 is success
} FT_VER_INFO_V2_CNF;

typedef enum 
{
     FT_GET_SIM_NUM = 0,
     FT_MISC_WCN_END = 0x0fffffff
}FT_GET_SIM_OP;


typedef struct 
{
    FT_H    header;
    FT_GET_SIM_OP    type;   
}FT_GET_SIM_REQ;

typedef struct 
{
    FT_H                  header;
    FT_GET_SIM_OP             type;	
   	unsigned char  status;
	unsigned int   number;
}FT_GET_SIM_CNF;

/********************************************
 * Generic Primitives for PMIC Module
 ********************************************/
#include "meta_pmic_para.h"


/********************************************
 * Generic Primitives for BT META
 ********************************************/
#ifdef FT_BT_FEATURE
#include "Meta_bt_Para.h"
#endif


/********************************************
 * Generic Primitives for DVBT META
 ********************************************/
//#include "Meta_DVB_T_Para.h"

/********************************************
 * Generic Primitives for BAT META
 ********************************************/
#include "meta_battery_para.h"

/********************************************
 * Generic Primitives for ADC META
 ********************************************/
#include "meta_adc_para.h"

/********************************************
 * Generic Primitives for GPS META
 ********************************************/
/* 2010-08-09 Marked for porting */
#ifdef FT_GPS_FEATURE
#include "meta_gps_para.h"
#endif

#ifdef FT_NFC_FEATURE
#include "meta_nfc_para.h"
#endif

#ifdef FT_EMMC_FEATURE
#include "meta_clr_emmc_para.h"
#include "meta_cryptfs_para.h"
#endif

#include "meta_dfo_para.h"

/********************************************
* Generic Primitives for common module(version, power off)
********************************************/
//int FT_Module_Init(void);
int FT_Module_Deinit(void);
void FT_TestAlive(FT_IS_ALIVE_REQ *req);
#ifdef FT_WIFI_FEATURE 
void FT_WIFI_OP(FT_WM_WIFI_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);
#endif
#ifdef FT_GPS_FEATURE 
void FT_GPS_OP(GPS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
#endif
#ifdef FT_NFC_FEATURE 
void FT_NFC_OP(NFC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
#endif
void FT_GPIO_OP(GPIO_REQ *req, char* peer_buf, kal_int16 peer_len);
void FT_GetVersionInfo(FT_VER_INFO_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len);
void FT_PowerOff(FT_POWER_OFF_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len );
//void FT_CheckMetaDllVersion(FT_CHECK_META_VER_REQ  *pFTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives forPeripheral META
 ********************************************/
void FT_CPURegW_OP(FT_REG_WRITE_REQ *req, char *pPeerBuf, kal_int16 peer_len);
void FT_CPURegR_OP(FT_REG_READ_REQ *req, char *pPeerBuf, kal_int16 peer_len);
void FT_Peripheral_OP(FT_UTILITY_COMMAND_REQ *req, char *pPeerBuf, kal_int16 peer_len);
void FT_PMICRegR_OP(FT_PMIC_REG_READ *FTReq, char *pPeerBuf, kal_int16 peer_len);
void FT_PMICRegW_OP(FT_PMIC_REG_WRITE *FTReq, char *pPeerBuf, kal_int16 peer_len);
void FT_SDcard_OP(SDCARD_REQ *req, char *peer_buff, kal_int16 peer_len);

/********************************************
 * Generic Primitives for nvram editor META
 ********************************************/
void FT_APEditorRead_OP(FT_AP_Editor_read_req *FTReq);
void FT_APEditorReset_OP(FT_AP_Editor_reset_req *FTReq);
void FT_APEditorWrite_OP(FT_AP_Editor_write_req *FTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives for bt META
 ********************************************/
#ifdef FT_BT_FEATURE   
void FT_BT_OP(BT_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);
#endif

/********************************************
 * Generic Primitives for FM META
 ********************************************/
#ifdef FT_FM_FEATURE     
void FT_FM_OP(FM_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);
#endif

/********************************************
 * Generic Primitives for dvbt META
 ********************************************/
//void FT_DVBT_OP(FT_DVB_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives for Battery META
 ********************************************/
void FT_BAT_OP(FT_BATT_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);

void FT_CCAP_OP(FT_CCT_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives for adc META
 ********************************************/
void FT_AUXADC_OP(AUXADC_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives for Bat META
 ********************************************/
void FT_BAT_ChipUpdate_OP(FT_BATT_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
void FT_BAT_FW_OP(FT_BATT_READ_INFO_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * Generic Primitives for AUDIO META
 ********************************************/
void FT_L4AUDIO_OP(FT_L4AUD_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len);


/********************************************
 * Generic Primitives for Low Power
 ********************************************/
void FT_LOW_POWER_OP(FT_LOW_POWER_REQ *req, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * NVRAM backup operation
 ********************************************/
void FT_NVRAM_Backup_OP(FT_NVRAM_BACKUP_REQ* req, char* pPeerBuf, kal_int16 peer_len);

/********************************************
 * NVRAM restore operation
 ********************************************/
void FT_NVRAM_Restore_OP(FT_NVRAM_RESTORE_REQ* req, char* pPeerBuf, kal_int16 peer_len);

/********************************************
 * G-Sensor operation
 ********************************************/
void FT_GSENSOR_OP(GS_REQ* req, char* pPeerBuf, kal_int16 peer_len);

/********************************************
 * Gyroscope operation
 ********************************************/
void FT_GYROSENSOR_OP(GYRO_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * META reboot
 ********************************************/
void FT_Reboot(FT_META_REBOOT_REQ *req);

/********************************************
 * META custom API
 ********************************************/
void FT_CUSTOMER_OP(FT_CUSTOMER_REQ* req, char* pPeerBuf, kal_int16 peer_len);

/********************************************
 * META mATV operation
 ********************************************/
#ifdef FT_MATV_FEATURE
void FT_MATV_OP(FT_MATV_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
#endif

/********************************************
 * Get chip ID
 ********************************************/
void FT_GET_CHIPID_OP(FT_GET_CHIPID_REQ* req, char* pPeerBuf, kal_int16 peer_len);

/********************************************
 * M-Sensor operation
 ********************************************/
void FT_MSENSOR_OP(FT_MSENSOR_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
 * ALS_PS operation
 ********************************************/
void FT_ALSPS_OP(FT_ALSPS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
   * Touch panel operation
   ********************************************/
void FT_CTP_OP(Touch_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);

/********************************************
   * Get Version Info V2
   ********************************************/
void FT_GetVersionInfoV2(FT_VER_INFO_V2_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len);

/********************************************
   * Get system/build.prop info
   ********************************************/
      
#ifdef FT_EMMC_FEATURE
void FT_CLR_EMMC_OP(FT_EMMC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
void FT_CRYPTFS_OP(FT_CRYPTFS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
#endif

void FT_BUILD_PROP_OP(FT_BUILD_PROP_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len);

void FT_MODEM_INFO_OP(FT_MODEM_REQ *pLocalBuf, char *pft_PeerBuf, kal_int16 ft_peer_len);


/********************************************
   * Get Sim CARD Number
 ********************************************/
void FT_SIM_NUM_OP(FT_GET_SIM_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len);


void FT_DFO_OP(FT_DFO_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);


/********************************************
   * ADC
   ********************************************/
void FT_ADC_OP(ADC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);

#ifdef FT_HDCP_FEATURE
void FT_HDCP_OP(HDCP_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len);
#endif


#ifdef __cplusplus
}
#endif

#endif

