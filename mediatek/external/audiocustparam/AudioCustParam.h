/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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
 * AudioCustParam.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements custom parameter setting.
 *
 * Author:
 * -------
 *   HP Cheng (mtk01752)
 *
 *******************************************************************************/

#ifndef _AUDIO_CUST_PARAM_H_
#define _AUDIO_CUST_PARAM_H_


/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include "CFG_AUDIO_File.h"
#include "CFG_file_lid.h"
#include <utils/Log.h>
#include <utils/String8.h>


namespace android
{

typedef enum
{
    SUPPORT_WB_SPEECH = 0x1,
    SUPPORT_DUAL_MIC = 0x2,
    SUPPORT_HD_RECORD = 0x4,
    SUPPORT_HD_48K_REC = 0x8,
    SUPPORT_DMNR_3_0 = 0x10,
    SUPPORT_DMNR_AT_MODEM = 0x20,
    SUPPORT_VOIP_ENHANCE = 0x40,
    SUPPORT_WIFI_ONLY = 0x80,
    SUPPORT_3G_DATA = 0x100,
    SUPPORT_NO_RECEIVER = 0x200,
    SUPPORT_ASR = 0x400,
    SUPPORT_VOIP_NORMAL_DMNR = 0x800,
    SUPPORT_VOIP_HANDSFREE_DMNR = 0x1000,
    SUPPORT_INCALL_NORMAL_DMNR = 0x2000,
    SUPPORT_INCALL_HANDSFREE_DMNR = 0x4000,
    SUPPORT_VOICE_UNLOCK = 0x8000,
    SUPPORT_DMNR_COMPLEX_ARCH = 0x10000,
    SUPPORT_GET_FO_VALUE = 0x20000,
    FEATURE_SUPPORT_INFO_LIST_END
} FeatureSupportInfo;


/*=============================================================================
 *                              Class definition
 *===========================================================================*/

bool checkNvramReady(void);

uint32_t QueryFeatureSupportInfo(void);

// NB speech parameters
int GetNBSpeechParamFromNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
int SetNBSpeechParamToNVRam(AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
int GetCustParamFromNV(AUDIO_CUSTOM_PARAM_STRUCT *pPara);

int SetCustParamToNV(AUDIO_CUSTOM_PARAM_STRUCT *pPara);
// Dual mic speech parameters
int GetDualMicSpeechParamFromNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
int SetDualMicSpeechParamToNVRam(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);

// WB speech parameters
int GetWBSpeechParamFromNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
int SetWBSpeechParamToNVRam(AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
// WB Speech functions
int GetCustWBParamFromNV(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara);

int SetCustWBParamToNV(AUDIO_CUSTOM_WB_PARAM_STRUCT *pPara);

// get med param parameter from NVRAM
int GetMedParamFromNV(AUDIO_PARAM_MED_STRUCT *pPara);
int SetMedParamToNV(AUDIO_PARAM_MED_STRUCT *pPara);

// functions
int GetVolumeVer1ParamFromNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);
int SetVolumeVer1ParamToNV(AUDIO_VER1_CUSTOM_VOLUME_STRUCT *pPara);

// get audio custom parameter from NVRAM
int GetAudioCustomParamFromNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);
int SetAudioCustomParamToNV(AUDIO_VOLUME_CUSTOM_STRUCT *pPara);

// get audio custom parameter from NVRAM
int GetAudioGainTableParamFromNV(AUDIO_GAIN_TABLE_STRUCT *pPara);
int SetAudioGainTableParamToNV(AUDIO_GAIN_TABLE_STRUCT *pPara);


int Read_DualMic_CustomParam_From_NVRAM(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara);
int Write_DualMic_CustomParam_To_NVRAM(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pPara);

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
// Get/Set Audio HD record parameters from/to NVRAM
int GetHdRecordParamFromNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);
int SetHdRecordParamToNV(AUDIO_HD_RECORD_PARAM_STRUCT *pPara);

// Get/Set Audio HD record scene table from/to NVRAM
int GetHdRecordSceneTableFromNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);
int SetHdRecordSceneTableToNV(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *pPara);

// Get/Set Audio HD record parameters from/to NVRAM
int GetHdRecord48kParamFromNV(AUDIO_HD_RECORD_48K_PARAM_STRUCT *pPara);
int SetHdRecord48kParamToNV(AUDIO_HD_RECORD_48K_PARAM_STRUCT *pPara);

#endif

// Get/Set voice revognition customization parameters
int GetVoiceRecogCustParamFromNV(VOICE_RECOGNITION_PARAM_STRUCT *pPara);
int SetVoiceRecogCustParamToNV(VOICE_RECOGNITION_PARAM_STRUCT *pPara);

int GetAudEnhControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);
int SetAudEnhControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);

int GetHiFiDACControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);
int SetHiFiDACControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);

// Get/Set Audio Buffer DC Calibration data
int GetDcCalibrationParamFromNV(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *pPara);
int SetDcCalibrationParamToNV(AUDIO_BUFFER_DC_CALIBRATION_STRUCT *pPara);

int GetAudioVoIPParamFromNV(AUDIO_VOIP_PARAM_STRUCT *pPara);
int SetAudioVoIPParamToNV(AUDIO_VOIP_PARAM_STRUCT *pPara);

int GetBesLoudnessControlOptionParamFromNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);
int SetBesLoudnessControlOptionParamToNV(AUDIO_AUDENH_CONTROL_OPTION_STRUCT *pPara);



}; // namespace android

#endif  //_AUDIO_YUSU_CUST_PARAM_H_




