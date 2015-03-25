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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   Custom_NvRam_lid.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *    header file of Custom_NvRam_lid
 *
 * Author:
 * -------
 *   Ning.F (MTK08139) 03/09/2009
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 11 12 2010 renbang.jiang
 * [ALPS00134025] [Wi-Fi] move Wi-Fi NVRAM definition source file to project folder from common folder
 * .
 *
 * 11 05 2010 renbang.jiang
 * [ALPS00134025] [Wi-Fi] move Wi-Fi NVRAM definition source file to project folder from common folder
 * .
 *
 * 06 24 2010 yunchang.chang
 * [ALPS00002677][Need Patch] [Volunteer Patch] ALPS.10X.W10.26 Volunteer patch for GPS customization use NVRam 
 * .
 *
 * Jun 22 2009 mtk01352
 * [DUMA00007771] Moving modem side customization to AP
 *
 *
 * Apr 29 2009 mtk80306
 * [DUMA00116080] revise the customization of nvram
 * revise nvram customization
 *
 * Mar 21 2009 mtk80306
 * [DUMA00112158] fix the code convention.
 * modify code convention
 *
 * Mar 9 2009 mtk80306
 * [DUMA00111088] nvram customization
 * nvram customization
 *
 *
 *
 *
 *
 *******************************************************************************/


#ifndef CUSTOM_CFG_FILE_LID_H
#define CUSTOM_CFG_FILE_LID_H


//#include "../../../../../common/inc/cfg_file_lid.h"
//#include "../../../common/cgen/inc/CFG_File_LID.h"
#include "CFG_file_lid.h"

/* the definition of file LID */
typedef enum
{
    AP_CFG_RDCL_FILE_AUDIO_LID=AP_CFG_CUSTOM_BEGIN_LID,	//AP_CFG_CUSTOM_BEGIN_LID: this lid must not be changed, it is reserved for system.
    AP_CFG_CUSTOM_FILE_GPS_LID,
    AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID,
    AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID,
    AP_CFG_RDEB_FILE_WIFI_LID,
    AP_CFG_RDEB_WIFI_CUSTOM_LID,   //30
    AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID,
    AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID,
    AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID,
    AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID,
    AP_CFG_REEB_PRODUCT_INFO_LID,
//	  AP_CFG_RDCL_FILE_META_LID,
//    AP_CFG_CUSTOM_FILE_CUSTOM1_LID,
//    AP_CFG_CUSTOM_FILE_CUSTOM2_LID,
    AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID,
    AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID,
    AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID,
    AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID,
    AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID,  //40
    AP_CFG_RDCL_FILE_AUDIO_HD_REC_48K_PAR_LID,
    AP_CFG_RDCL_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_LID,
    AP_CFG_CUSTOM_FILE_PS_CALI_LID,//line<FTM><add ps cali lid><20131216>yinhuiyong
	AP_CFG_RDEB_STS_CUSTOM_LID,    //DONT MOVE IT //SaleTrackerNew 44
    AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID,
    AP_CFG_CUSTOM_FILE_MAX_LID,
} CUSTOM_CFG_FILE_LID;


/* verno of data items */
/* audio file version */
#define AP_CFG_RDCL_FILE_AUDIO_LID_VERNO			"001"

/* GPS file version */
#define AP_CFG_CUSTOM_FILE_GPS_LID_VERNO			"000"

/* audio acf file version */
#define AP_CFG_RDCL_FILE_AUDIO_COMPFLT_LID_VERNO	"001"

/* audio hcf file version */
#define AP_CFG_RDCL_FILE_HEADPHONE_COMPFLT_LID_VERNO	"001"
/* audio vibspk hcf file version */
#define AP_CFG_RDCL_FILE_VIBSPK_COMPFLT_LID_VERNO	"001"

/* audio effect file version */
#define AP_CFG_RDCL_FILE_AUDIO_EFFECT_LID_VERNO	"001"

/* audio med file version */
#define AP_CFG_RDCL_FILE_AUDIO_PARAM_MED_LID_VERNO  "001"

/* audio volume custom file version */
#define AP_CFG_RDCL_FILE_AUDIO_VOLUME_CUSTOM_LID_VERNO  "001"
#define AP_CFG_RDCL_FILE_AUDIO_VER1_VOLUME_CUSTOM_LID_VERNO  "001"

/* dual mic custom file version */
#define AP_CFG_RDCL_FILE_DUAL_MIC_CUSTOM_LID_VERNO  "002"

/* audio wb specch param custom file version */
#define AP_CFG_RDCL_FILE_AUDIO_WB_PARAM_LID_VERNO "001"

/* audio gain table custom file version */
#define AP_CFG_RDCL_FILE_AUDIO_GAIN_TABLE_LID_VERNO "001"

/* audio hd record par custom file version*/
#define AP_CFG_RDCL_FILE_AUDIO_HD_REC_PAR_LID_VERNO "001"
#define AP_CFG_RDCL_FILE_AUDIO_HD_REC_SCENE_LID_VERNO "001"
#define AP_CFG_RDCL_FILE_AUDIO_HD_REC_48K_PAR_LID_VERNO "001"

/* audio buffer dc calibration custom file version*/
#define AP_CFG_RDCL_FILE_AUDIO_BUFFER_DC_CALIBRATION_PAR_LID_VERNO "000"

/* META log and com port config file version */
#define AP_CFG_RDCL_FILE_META_LID_VERNO			    "000"

/* custom2 file version */
#define AP_CFG_CUSTOM_FILE_CUSTOM1_LID_VERNO			"000"
/* custom2 file version */
#define AP_CFG_CUSTOM_FILE_CUSTOM2_LID_VERNO			"000"

/* WIFI file version */
#define AP_CFG_RDEB_FILE_WIFI_LID_VERNO				"000"
/* WIFI MAC addr file version */
#define AP_CFG_RDCL_FILE_WIFI_ADDR_LID_VERNO		"000"
#define AP_CFG_RDEB_WIFI_CUSTOM_LID_VERNO				"000"
#define AP_CFG_REEB_PRODUCT_INFO_LID_VERNO      "000"

//guchunhua,DATE20140306,modify for WIKOKK-53,LINE
#define AP_CFG_RDEB_STS_CUSTOM_LID_VERNO "000"

#define AP_CFG_CUSTOM_FILE_PS_CALI_LID_VERNO	"000"//line<ps cali><add ps cali lid><20131216>yinhuiyong

#endif /* CFG_FILE_LID_H */
