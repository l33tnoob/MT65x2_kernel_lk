/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 * AudioCmdHandler.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   The audio command handling interface API.
 *
 * Author:
 * -------
 *   Donglei Ji(mtk80823)
 *
 *******************************************************************************/

#ifndef _AUDIO_CMD_HANDLER_H_
#define _AUDIO_CMD_HANDLER_H_

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#include <pthread.h>
#include <media/mediaplayer.h>
#include <media/mediarecorder.h>

using namespace android;

//<--- for speech parameters calibration
#define MAX_VOL_CATE 3
#define CUSTOM_VOL_STEP 7

#define SPH_MODE_NUM 4
#define FIR_INDEX_NUM 6
#define FIR_NUM_NB 45
#define SPH_ENHANCE_PARAM_NUM 16
#define SPH_COMMON_NUM 12

#define FIR_NUM_WB 90
#define MAX_FILE_NAME_LEN 128
//--->

typedef struct
{
    int param1;
    int param2;
    int param3;
    int param4;
    int param5;
    unsigned int recvDataLen;
} AudioCmdParam;

typedef enum
{
    ACHFailed = -3,
    ACHLoadFileFailed,
    ACHParamError,
    ACHSucceeded
} ACHStatus;
typedef enum{
	AUDIO_ACF_FO = 0x212 
}AudioCommands;

typedef enum
{
    Analog_MIC1_Single = 1,
    Analog_MIC2_Single = 2,
    Analog_MIC_Dual = 9,
    Audio_I2S_IN = 98,
    Audio_FM_IN = 99,

    AUDIO_SOURCE_LIST_END
} AudioSourceType;

/*
typedef enum {
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
    SUPPORT_HANDSFREE_VOIP_DMNR = 0x1000,

    SUPPORT_INFO_LIST_END
}PhoneSupportInfo;
*/
//<--- for speech parameters calibration
typedef enum
{
    LOAD_VOLUME_POLICY =    0,
    SET_FM_SPEAKER_POLICY,

    AUDIO_POLICY_CNT
} SetPolicyParameters;

typedef enum
{
    DUAL_MIC_REC_PLAY_STOP = 0,
    DUAL_MIC_REC,
    DUAL_MIC_REC_PLAY,
    DUAL_MIC_REC_PLAY_HS,
    DUAL_MIC_REC_HF,
    DUAL_MIC_REC_PLAY_HF,
    DUAL_MIC_REC_PLAY_HS_HF,

    DMNR_TUNING_CMD_CNT
} DMNRTuningCmdType;

typedef enum
{
    DUAL_MIC_UL_GAIN = 0,
    DUAL_MIC_DL_GAIN,
    DUAL_MIC_HSDL_GAIN,
    DUAL_MIC_UL_GAIN_HF,

    DUAL_MIC_GAIN_CNT
} GainType;

// for speech calibration
typedef struct
{
    unsigned short sph_com_param[SPH_COMMON_NUM];
    unsigned short sph_mode_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM];
    short sph_in_fir[FIR_INDEX_NUM][FIR_NUM_NB];
    short sph_out_fir[FIR_INDEX_NUM][FIR_NUM_NB];
    short sph_output_FIR_coeffs[SPH_MODE_NUM][FIR_INDEX_NUM][FIR_NUM_NB];
    short selected_FIR_output_index[SPH_MODE_NUM];
} AUD_SPH_PARAM_STRUCT;

// for WB speech calibration
typedef struct
{
    unsigned short sph_mode_wb_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM]; //WB speech enhancement
    short sph_wb_in_fir[FIR_INDEX_NUM][FIR_NUM_WB]; // WB speech input FIR
    short sph_wb_out_fir[FIR_INDEX_NUM][FIR_NUM_WB];// WB speech output FIR
} AUD_SPH_WB_PARAM_STRUCT;

typedef struct
{
    unsigned short cmdType;
    unsigned short selected_fir_index;
    unsigned short dlDGGain;
    unsigned short dlPGA;
    unsigned short phone_mode;
    unsigned short wb_mode;
    char audio_file[MAX_FILE_NAME_LEN];
} AUD_TASTE_PARAM_STRUCT;

typedef struct
{
    char input_file[MAX_FILE_NAME_LEN];
    char output_file[MAX_FILE_NAME_LEN];
} DMNRTuningFileName;
//--->

/*=============================================================================
 *                              Class definition
 *===========================================================================*/

class AudioCmdHandler;

class PCMRecorderListener : public MediaRecorderListener
{
    public:
        PCMRecorderListener(AudioCmdHandler *pListener);
        ~PCMRecorderListener() {}

        void notify(int msg, int ext1, int ext2);

    private:
        AudioCmdHandler *m_pListener;
};

class AudioCmdHandler : public MediaPlayerListener
{
    public:
        AudioCmdHandler();
        ~AudioCmdHandler();
        AudioCmdHandler(const AudioCmdHandler &);
        AudioCmdHandler &operator=(const AudioCmdHandler &);

        // The  functions of processing audio cmds
        ACHStatus setRecorderParam(AudioCmdParam &audioCmdParams);
        ACHStatus startRecorderFrMIC(AudioCmdParam &audioCmdParams);
        ACHStatus setMICGain(AudioCmdParam &audioCmdParams);
        ACHStatus startRecorderFrFM(AudioCmdParam &audioCmdParams);
        ACHStatus playFM();
        ACHStatus stopPlayingFM();
        ACHStatus setVDPG(AudioCmdParam &audioCmdParams);
        ACHStatus setAUDLINEPG(AudioCmdParam &audioCmdParams);
        ACHStatus setFMorMICVUPG(AudioCmdParam &audioCmdParams, bool bFMGain);
        ACHStatus startAudioPlayer(AudioCmdParam &audioCmdParams);
        ACHStatus stopAudioPlayer();
        ACHStatus startRecorderFrI2S(AudioCmdParam &audioCmdParams);
        ACHStatus writeRegister(AudioCmdParam &audioCmdParams);
        String8 readRegister(AudioCmdParam &audioCmdParams);
        ACHStatus stopRecorder();
        void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);

        // add for mt6575 HQA
        void setParameters(const String8 &keyValuePaires);

        // add for mt6577 HQA
        ACHStatus SetAudioData(int cmdType, void *ptr, size_t len);

        //<--- for speech parameters calibration
        ACHStatus ULCustSPHParamFromNV(void *pParam, int *len, int block);
        ACHStatus DLCustSPHParamToNV(void *pParam, int block);

        ACHStatus ULCustSPHVolumeParamFromNV(void *pParam, int *len);
        ACHStatus DLCustSPHVolumeParamToNV(void *pParam);

        ACHStatus ULCustSPHWBParamFromNV(void *pParam, int *len, int block);
        ACHStatus DLCustSPHWBParamToNV(void *pParam, int block);

        ACHStatus ULCustACFParamFromNV(void *pParam, int *len);
        ACHStatus DLCustACFParamToNV(void *pParam);
	ACHStatus findACFFO(unsigned short cmdType, void *pParam, int *len);

        ACHStatus ULCustHCFParamFromNV(void *pParam, int *len);
        ACHStatus DLCustHCFParamToNV(void *pParam);

        ACHStatus AudioTasteRunning(unsigned short cmdType, void *pParam = NULL);

        ACHStatus ULCustDualMicParamFromNV(void *pParam, int *len, int block);
        ACHStatus DLCustDualMicParamToNV(void *pParam, int block);

        ACHStatus getDMNRGain(unsigned short cmdType, void *pParam, int *len);
        ACHStatus setDMNRGain(unsigned short cmdType, unsigned short gain);

        ACHStatus AudioDMNRTuning(unsigned short cmdType, bool bWB, void *pParam);

        ACHStatus ULCustHDRecParamFromNV(void *pParam, int *len, int block);
        ACHStatus DLCustHDRecParamToNV(void *pParam, int block);

        ACHStatus ULCustHDRecSceTableFromNV(void *pParam, int *len);
        ACHStatus DLCustHDRecSceTableToNV(void *pParam);

        ACHStatus ULCustHDRecHSParamFromNV(void *pParam, int *len);
        ACHStatus DLCustHDRecHSParamToNV(void *pParam);
        ACHStatus HDRecording(int enable, void *pParam);

        ACHStatus getPhoneSupportInfo(unsigned int *supportInfo);
        void enableHALHDRecTunning(bool enbale);

        ACHStatus ULCustVOIPParamFromNV(void *pParam, int *len);
        ACHStatus DLCustVOIPParamToNV(void *pParam);
        //--->

    private:
        int m_RecordMaxDur;
        int m_RecordChns;
        int m_RecordSampleRate;
        int m_RecordBitsPerSample;
        int m_fd;
        bool m_bRecording;
        bool m_bHDRecTunning;

        sp <MediaPlayer> m_MediaPlayerClient;
        sp <MediaPlayer> m_FMMediaPlayerClient;
        sp <MediaRecorder> m_MediaRecorderClient;
        sp <PCMRecorderListener> m_pMediaRecorderListenner;
        AudioSourceType m_RecordAudioSource;

        ACHStatus startRecorder(const char *filePath, int recordAudioSource);
        ACHStatus startFMAudioPlayer();
        ACHStatus stopFMAudioPlayer();
        bool isRecording() {return m_bRecording; }
};

#endif  //_AUDIO_COMP_FLT_CUST_PARAM_H_




