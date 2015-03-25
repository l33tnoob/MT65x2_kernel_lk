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
 *   CFG_file_info_custom.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   Configuration File List for Customer
 *
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 ****************************************************************************/

#ifndef __CFG_FILE_INFO_CUSTOM_H__
#define __CFG_FILE_INFO_CUSTOM_H__

#include "CFG_file_public.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#include "CFG_AUDIO_File.h"
#include "CFG_Audio_Default.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_Wifi_File.h"
#include "CFG_WIFI_Default.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "CFG_PRODUCT_INFO_Default.h"

//line<ps cali><add ps cali lid><20131216>yinhuiyog
#include "CFG_PS_CALI_File.h"
#include "CFG_PS_CALI_File_Default.h"

#include <stdio.h>
#include "CFG_STS_Default.h"//guchunhua,DATE20140306,modify for WIKOKK-53,LINE
#include "CFG_STS_File.h"//guchunhua,DATE20140306,modify for WIKOKK-53,LINE
#ifdef __cplusplus
extern "C"
{
#endif

    const TCFG_FILE g_akCFG_File_Custom[]=
    {
        {
            "/data/nvram/APCFG/APRDCL/Audio_Sph",       VER(AP_CFG_RDCL_FILE_AUDIO_LID),         CFG_FILE_SPEECH_REC_SIZE,
            CFG_FILE_SPEECH_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&speech_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/GPS",         VER(AP_CFG_CUSTOM_FILE_GPS_LID),	           CFG_FILE_GPS_CONFIG_SIZE,
            CFG_FILE_GPS_CONFIG_TOTAL,                  SIGNLE_DEFUALT_REC,                (char *)&stGPSConfigDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_CompFlt",       VER(AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID),         CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_AUDIO_COMPFLT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Effect",       VER(AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID),         CFG_FILE_AUDIO_EFFECT_REC_SIZE,
            CFG_FILE_AUDIO_EFFECT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_effect_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/WIFI",	    	VER(AP_CFG_RDEB_FILE_WIFI_LID),		    CFG_FILE_WIFI_REC_SIZE,
            CFG_FILE_WIFI_REC_TOTAL,		    	SIGNLE_DEFUALT_REC,				    (char *)&stWifiCfgDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM",	VER(AP_CFG_RDEB_WIFI_CUSTOM_LID),	CFG_FILE_WIFI_CUSTOM_REC_SIZE,
            CFG_FILE_WIFI_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stWifiCustomDefault, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Sph_Med",       VER(AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID),         CFG_FILE_AUDIO_PARAM_MED_REC_SIZE,
            CFG_FILE_AUDIO_PARAM_MED_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_param_med_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Vol_custom",       VER(AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID),         CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_VOLUME_CUSTOM_REC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&audio_volume_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Sph_Dual_Mic",       VER(AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID),         CFG_FILE_SPEECH_DUAL_MIC_SIZE,
            CFG_FILE_SPEECH_DUAL_MIC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&dual_mic_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_Wb_Sph",       VER(AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID),         CFG_FILE_WB_SPEECH_REC_SIZE,
            CFG_FILE_WB_SPEECH_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&wb_speech_custom_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDEB/PRODUCT_INFO",       VER(AP_CFG_REEB_PRODUCT_INFO_LID),         CFG_FILE_PRODUCT_INFO_SIZE,
            CFG_FILE_PRODUCT_INFO_TOTAL,                   SIGNLE_DEFUALT_REC,                                   (char *)&stPRODUCT_INFOConfigDefault,DataReset, NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Headphone_CompFlt",       VER(AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID),         CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_HEADPHONE_COMPFLT_REC_TOTAL,                   SIGNLE_DEFUALT_REC,                (char *)&audio_hcf_custom_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_gain_table",   VER(AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID), CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_GAIN_TABLE_CUSTOM_REC_TOTAL, SIGNLE_DEFUALT_REC  ,	 (char *)&Gain_control_table_default, DataReset , NULL
        },

        {
            "/data/nvram/APCFG/APRDCL/Audio_ver1_Vol_custom",       VER(AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID),         CFG_FILE_AUDIO_VER1_VOLUME_CUSTOM_REC_SIZE,
            CFG_FILE_AUDIO_VER1_VOLUME_CUSTOM_REC_TOTAL,           SIGNLE_DEFUALT_REC,                (char *)&audio_ver1_custom_default, DataReset , NULL
        },
        
        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID), CFG_FILE_AUDIO_HD_REC_PAR_SIZE,
            CFG_FILE_AUDIO_HD_REC_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Par_default, DataReset , NULL
        },
        
        { "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_Scene_Table",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID), CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_SIZE,
            CFG_FILE_AUDIO_HD_REC_SCENE_TABLE_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_Scene_Table_default, DataReset , NULL
        },

		{ "/data/nvram/APCFG/APRDCL/Audio_Hd_Record_48k_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_HD_REC_48K_PAR_LID), CFG_FILE_AUDIO_HD_REC_48K_PAR_SIZE,
            CFG_FILE_AUDIO_HD_REC_48K_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Hd_Recrod_48k_Par_default, DataReset , NULL
        },

        { "/data/nvram/APCFG/APRDCL/Audio_Buffer_DC_Calibration_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_LID), CFG_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_SIZE,
            CFG_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Audio_Buffer_DC_Calibration_Par_default, DataReset , NULL
        },
		//line<ps cali><add ps cali lid><20131216>yinhuiyong
		{
            "/data/nvram/APCFG/APRDEB/PS_CALI",       VER(AP_CFG_CUSTOM_FILE_PS_CALI_LID),         CFG_CUSTOM_FILE_PS_CALI_SIZE,
            CFG_CUSTOM_FILE_PS_CALI_TOTAL,                   SIGNLE_DEFUALT_REC,  (char *)&stPS_CALIConfigDefault,DataReset, NULL
        },

        //add by renbin.wei for sts
        {
            "/data/nvram/APCFG/APRDEB/STS_CUSTOM",	VER(AP_CFG_RDEB_STS_CUSTOM_LID),	CFG_FILE_STS_CUSTOM_REC_SIZE,
            CFG_FILE_STS_CUSTOM_REC_TOTAL,		    SIGNLE_DEFUALT_REC,				    (char *)&stStsCustomDefault, DataReset , NULL
        },
        { "/data/nvram/APCFG/APRDCL/VibSpk_CompFlt",   VER(AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID), CFG_FILE_AUDIO_COMPFLT_REC_SIZE,
            CFG_FILE_VIBSPK_COMPFLT_REC_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&audio_vibspk_custom_default, DataReset , NULL
        },
    };

    int iNvRamFileMaxLID=AP_CFG_CUSTOM_FILE_MAX_LID;
    extern int iNvRamFileMaxLID;
    const unsigned int g_i4CFG_File_Custom_Count = sizeof(g_akCFG_File_Custom)/sizeof(TCFG_FILE);

    extern const TCFG_FILE g_akCFG_File_Custom[];

    extern const unsigned int g_i4CFG_File_Custom_Count;

    int iFileWIFILID=AP_CFG_RDEB_FILE_WIFI_LID;
    extern int iFileWIFILID;
    int iFileCustomWIFILID=AP_CFG_RDEB_WIFI_CUSTOM_LID;
    extern int iFileCustomWIFILID;
    int iFilePRODUCT_INFOLID=AP_CFG_REEB_PRODUCT_INFO_LID;
    extern int iFilePRODUCT_INFOLID;

#ifdef __cplusplus
}
#endif

#endif
